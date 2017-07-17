/**
   @file writecache.cpp
   @brief Write Cache class
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: writecache.cpp 292 2007-08-27 05:36:39Z sugiura $
 */

#include "logger.h"
#include "writecache.h"
#include "diskwriter.h"

#ifdef LOG4CXX_COUT
#undef LOG4CXX_DEBUG
#define LOG4CXX_DEBUG(logger,message)	strings.push_back(message)
#undef LOG4CXX_WARN
#define LOG4CXX_WARN(logger,message)	strings.push_back(message)
#undef LOG4CXX_ERROR
#define LOG4CXX_ERROR(logger,message)	strings.push_back(message)
extern vector<string> strings;
#endif

string WriteCache::dump()
{
	lock lk(wq_guard);
	ostringstream os;
	os << "dump : ";
	writecache_lba_index &lba_index = wq.get<by_lba>();
	for(writecache_lba_index::iterator itr = lba_index.begin();
	    itr != lba_index.end(); ++itr){
		os << (*itr)->dump() << " ";
	}
	os << endl;
	return os.str();
}

string WriteSegment::dump()
{
	ostringstream os;
	os << "["<< boost::lexical_cast<string>(lba) <<  "+"
	   << boost::lexical_cast<string>(transfer_length) << " "
	   << boost::lexical_cast<string>(rlba) << "+" 
	   << boost::lexical_cast<string>(rlength) << "]";
	return os.str();
}

vector<WriteSegmentPtr> WriteCache::_search(const uint64_t lba,
					    const size_t transfer_length)
{
	/** must be locked */
	vector<WriteSegmentPtr> hits;
	if(wq.empty())
		return hits;
	writecache_lba_index &lba_index = wq.get<by_lba>();
	writecache_lba_index::const_iterator itr = lba_index.lower_bound(lba);
	writecache_lba_index::const_iterator begin = lba_index.begin();
	writecache_lba_index::const_iterator end = lba_index.end();
	// search front of range
	for(writecache_lba_index::const_iterator bitr = itr;
	    (bitr == end ||
	     lba < (*bitr)->rlba ||
	     (*bitr)->rlength == 0) && bitr != begin;){
		--bitr;
		if((*bitr)->rlength == 0)
			continue;
		if(lba < ((*bitr)->rlba + (*bitr)->rlength))
			itr = bitr;
	}

	// fill hits
	for(;
	    itr != end; ++itr){
		if(lba+transfer_length <= (*itr)->rlba)
			break;
		if((*itr)->rlength > 0)
			hits.push_back(*itr);
	}
	return hits;
}

bool WriteCache::getNext(WriteSegmentPtr &_ws)
{
	lock lk(wq_guard);
	if(wq.empty())
		return false;

	if(_ws.get() == NULL){
		/** front case */
		_ws = _front();
		LOG4CXX_DEBUG(logger, "getNext[0] : "
			      + boost::lexical_cast<string>(_ws.get()));
		return true;
	}

	/** intermediate case
	    search current element */
	writecache_ptr_index &ptr_index = wq.get<by_ptr>();
	writecache_ptr_index::const_iterator ptr = ptr_index.find(_ws.get());
	if(ptr == ptr_index.end()){
		_ws = _front();
		LOG4CXX_DEBUG(logger, "getNext[1] : "
			      + boost::lexical_cast<string>(_ws.get()));
		return true;
	}

	/** get projection of iterator */
	writecache_fifo_index::const_iterator itr = wq.project<0>(ptr);
	writecache_fifo_index::const_iterator end = wq.get<by_fifo>().end();

	/** skip copied element */
	for(++itr; itr != end; ++itr){
		if(!((*itr)->copy))
			break;
	}
	if(itr == end){
		// LOG4CXX_DEBUG(logger, "getNext : reach at the end of queue");
		return false;
	}
			
	/** get next writable one in FIFO queue */
	_ws = *itr;
	LOG4CXX_DEBUG(logger, "getNext[2] : "
		      + boost::lexical_cast<string>(_ws.get()));
	return true;
}

WriteCache::WriteCache()
{
	LOG4CXX_DEBUG(logger, "WriteCache Constructed : " +
		      boost::lexical_cast<string>(this));
}

void* WriteSegment::operator new(size_t size)
{
	if(size != sizeof(WriteSegment)){
		LOG4CXX_ERROR(logger, "WriteSegment allocated with invalid size");
		return ::operator new(size);
	}
	return WriteSegmentPool::malloc();
}

void WriteSegment::operator delete(void *obj, size_t size)
{
	if(!obj)
		return;
	if(size != sizeof(WriteSegment) ||
	   !WriteSegmentPool::is_from(obj)){
		LOG4CXX_ERROR(logger, "Unknown WriteSegment destructed");
		return ::operator delete(obj);
	}
	WriteSegmentPool::free(obj);
}

void WriteCache::push(WriteSegmentPtr ws)
{
	lock lk(wq_guard);
	vector<WriteSegmentPtr> hits = _search(ws->lba,ws->transfer_length);

	/** merge READ segments */
	vector<WriteSegmentPtr>::iterator end = hits.end();
	for(vector<WriteSegmentPtr>::iterator itr = hits.begin();
	    itr != end; ++itr){
		uint64_t hhead = (*itr)->rlba;
		uint64_t htail = hhead + (*itr)->rlength;
		uint64_t whead = ws->lba;
		uint64_t wtail = ws->lba + ws->transfer_length;
		LOG4CXX_DEBUG(logger, "old " + (*itr)->dump() + " new " + ws->dump());
		if(whead <= hhead && htail <= wtail){
			/** overwrite case */
			(*itr)->rlength = 0;
			LOG4CXX_DEBUG(logger, "overwrite : " + (*itr)->dump());
		} else if(hhead < whead && wtail < htail){
			/** split case */
			// shrink copy head
			WriteSegmentPtr cs = WriteSegmentPtr(new WriteSegment(*itr));
			cs->rlba = cs->lba + wtail - hhead;
			cs->rlength = htail - wtail;
			wq.push_back(cs);
			// shrink tail
			(*itr)->rlength -= std::min(static_cast<size_t>(htail - whead),
						    (*itr)->rlength);
			LOG4CXX_DEBUG(logger, "split : " + cs->dump() +
				      " " + (*itr)->dump());
		} else if(hhead < whead && whead < htail &&
			  htail <= wtail) {
			/** shrink tail case */
			(*itr)->rlength -= std::min(static_cast<size_t>(htail - whead),
						    (*itr)->rlength);
			LOG4CXX_DEBUG(logger, "shrink tail : " + (*itr)->dump());
		} else if(whead <= hhead &&
			  hhead < wtail && wtail < htail) {
			/** shrink head case */
			size_t step = std::min(static_cast<size_t>(wtail - hhead),
					       (*itr)->rlength);
			(*itr)->rlba += step;
			(*itr)->rlength -= step;
			LOG4CXX_DEBUG(logger, "shrink head : " + (*itr)->dump());
		} else {
			/** never reach here */
			LOG4CXX_WARN(logger, "MISS!! ["
				     + boost::lexical_cast<string>(hits.size())
				     + "] cache : " + (*itr)->dump()
				     + " new : " + ws->dump());
		}
	}

	/** always add to end of FIFO queue */
//	LOG4CXX_DEBUG(logger, "Push : " + boost::lexical_cast<string>(ws.get()));
	wq.push_back(ws);
}

void WriteCache::setWriter(DiskWriterPtr _writer)
{
	LOG4CXX_DEBUG(logger, "Set writer : " + boost::lexical_cast<string>(this) +
		      " " + boost::lexical_cast<string>(_writer.get()));
	writer = _writer;
}

WriteCache::~WriteCache()
{
	LOG4CXX_DEBUG(logger, "WriteCache Destructed : " +
		      boost::lexical_cast<string>(this));
}
