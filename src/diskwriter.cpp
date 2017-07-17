/** 
 * @file  diskwriter.cpp
 * @brief DiskWriter classes
 * @author Eiji Sugiura <eiji.sugiura@gmail.com>
 * @version $Id: diskwriter.cpp 312 2007-09-28 00:56:17Z sugiura $
 */

#include <unistd.h>
#include <fcntl.h>
#include <libaio.h>
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include "writecache.h"
#include "diskwriter.h"

#ifdef LOG4CXX_COUT
#undef LOG4CXX_DEBUG
#define LOG4CXX_DEBUG(logger,message)	strings.push_back(message)
#undef LOG4CXX_ERROR
#define LOG4CXX_ERROR(logger,message)	strings.push_back(message)
#undef LOG4CXX_WARN
#define LOG4CXX_WARN(logger,message)	strings.push_back(message)
#undef LOG4CXX_INFO
#define LOG4CXX_INFO(logger,message)	strings.push_back(message)
extern vector<string> strings;
#endif

bool DiskWriter::addHandle(const LowHandle &handle, const uint64_t offset)
{
	handles[offset] = handle;
	return true;
}

void DiskWriter::operator()() throw(std::runtime_error)
{
	NDC::push("writer[" + boost::lexical_cast<string>(this) +  "]");

	LOG4CXX_INFO(logger, "entering loop.");
	/** disk writer work loop */
	while(!stop){
		lock lk(ev_guard);
		boost::xtime xt;
		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.sec += 1;
		event.timed_wait(lk, xt);
		if(!doWork())
			break;
	}
	LOG4CXX_INFO(logger, "exiting loop.");

	NDC::remove();
	return;
}

bool DiskWriter::needNotify()
{
	static SequenceCounter counter;
	pool_state_t state = CommonBuf::getPoolState();
	if(state == POOL_STATE_NORMAL)
		return false;
	else if(state == POOL_STATE_HARD && counter()%2 != 0)
		return false;
	return true;
}

bool DiskWriter::doWork()
{
	if(wc->empty())
		return true;
	bool notified = false;
	do{
		WriteSegmentPtr ws = wc->front();
		/** resolve "LBA, TransferLength" + LowHandle
		    -> handles & write it out */
		uint64_t start = SCSI::LBA2OFFSET(ws->lba);
		size_t length = SCSI::LBA2OFFSET(ws->transfer_length);
		for(vector<CommonBufPtr>::iterator itr = ws->cbufs.begin();
		    itr != ws->cbufs.end(); ++itr){
			size_t ds_offset = (itr == ws->cbufs.begin())?(ws->ds_offset):0;
			size_t segment = min(length,
					     ((*itr)->getSize() - ds_offset));
			uint64_t offset = 0;
			length -= segment;
			while(segment > 0){
				void *buf = reinterpret_cast<uint8_t*>((*itr)->head())
					+ ds_offset + offset;
				size_t covered = 0;
				uint64_t real_offset = 0;
				/** LBA + segment = covered from buf with offset*/
				filehandle_t descriptor = getHandle(start, segment, covered, real_offset);
				if(descriptor == INVALID_HANDLE_VALUE){
					LOG4CXX_ERROR(logger, "Failed to resolve real segment");
					return false;
				}
				if(!doWrite(descriptor,
					    buf,covered,real_offset)){
					LOG4CXX_ERROR(logger, "Failed to write out segment");
					break;
				}
				start += covered;
				segment -= covered;
				offset += covered;
			}
			if(segment != 0){
				LOG4CXX_ERROR(logger, "Segment mismatch");
				return false;
			}
		}
		if(length != 0){
			LOG4CXX_ERROR(logger, "Length mismatch");
			return false;
		}
		wc->pop();
		if(needNotify()){
			wc->notifyDone();
			notified = true;
		}
	}while(!wc->empty());
	if(notified){
		LOG4CXX_DEBUG(logger,"Release CommonPool");
		CommonBuf::releasePool();
	}
	return true;
}

filehandle_t DiskWriter::getHandle(uint64_t start, size_t length,
				   size_t &covered, uint64_t &real_offset)
{
	if(handles.empty()){
		LOG4CXX_ERROR(logger, "Internal error in file handling");
		return INVALID_HANDLE_VALUE;
	}
	write_handle_map_t::const_iterator itr = handles.lower_bound(start);
	if(itr != handles.begin() && (itr == handles.end() || itr->first > start)){
		--itr;
	}
	uint64_t offset = itr->first;
	LowHandle handle = itr->second;
	if(offset > start){
		LOG4CXX_ERROR(logger, "Internal error in offset handling : " +
			      boost::lexical_cast<string>(start) + "+" +
			      boost::lexical_cast<string>(length) + " " +
			      boost::lexical_cast<string>(offset) + "/" +
			      boost::lexical_cast<string>(real_offset));
		return INVALID_HANDLE_VALUE;
	}
	real_offset = start - offset + handle.getOffset();
	covered = min(static_cast<uint64_t>(length),
		      handle.getLength() + handle.getOffset() - real_offset);
	if(covered == 0){
		LOG4CXX_ERROR(logger, "Internal error in coverage : covered = 0 " +
			      boost::lexical_cast<string>(start) + "+" +
			      boost::lexical_cast<string>(length) + "|" +
			      boost::lexical_cast<string>(handle.getOffset()) + "+" +
			      boost::lexical_cast<string>(handle.getLength()) + "-" +
			      boost::lexical_cast<string>(real_offset) + " " +
			      boost::lexical_cast<string>(offset) + "/" +
			      boost::lexical_cast<string>(real_offset));
		return INVALID_HANDLE_VALUE;
	}

	return handle.getDescriptor();
}

bool DiskWriter::doWrite(const filehandle_t fd, void *buf, size_t count, uint64_t offset)
{
	uint8_t *ptr = reinterpret_cast<uint8_t*>(buf);
 try_again:
#ifndef LOG4CXX_COUT
	ssize_t size = pwrite64(fd, ptr, count, offset);
#else
	ssize_t size = pwrite64(fd, ptr, min(500UL,(unsigned long)count), offset);
#endif
	LOG4CXX_DEBUG(logger, "Write Segment : [" + boost::lexical_cast<string>(fd) + "] offset "
		      + boost::lexical_cast<string>(offset) + " ("
		      + boost::lexical_cast<string>(SCSI::OFFSET2LBA(offset)) + ") "
#ifdef CHECK_DATA_INTEGRITY
//		      + " " + SCSI::DUMPOCTETS(ptr,size)
		      + " " + SCSI::DUMPOCTETS(ptr)
#else
		      + " " + SCSI::DUMPOCTETS(ptr,0)
#endif
		      + " -> " + boost::lexical_cast<string>(size)
		      + "/"  + boost::lexical_cast<string>(count));

	if(size < 0) {
		if(errno == EINTR)
			goto try_again;
		else {
			LOG4CXX_ERROR(logger, "Failed to do write (" +
				      boost::lexical_cast<string>(errno) + ")");
			return false;
		}
	} else if(static_cast<size_t>(size) != count){
		ptr += size;
		count -= size;
		offset += size;
		goto try_again;
	}
	return true;
}

void DiskWriter::mayStop()
{
	stop = true;
	LOG4CXX_DEBUG(logger, "writer may stop");
}

DiskWriter::DiskWriter(boost::shared_ptr<WriteCache> _wc)
	: stop(false), wc(_wc)
{
	LOG4CXX_DEBUG(logger, "DiskWriter Constructed : " +
		      boost::lexical_cast<string>(this));
}

DiskWriter::~DiskWriter()
{
	if(!wc->empty()){
		LOG4CXX_WARN(logger, "Dirty data is remain! : " +
			     boost::lexical_cast<string>(wc->size()));
	}
	/** Clear WriteCache */
	WriteCachePtr null;
	wc = null;

	LOG4CXX_DEBUG(logger, "DiskWriter Destructed : " +
		      boost::lexical_cast<string>(this));
}

AioDiskWriter::AioDiskWriter(boost::shared_ptr<WriteCache> _wc)
	: DiskWriter(_wc), listener(this), ctx(NULL),
	  iocbs_sizes(), iocbs_total()
{
	LOG4CXX_DEBUG(logger, "AioDiskWriter Constructed : " +
		      boost::lexical_cast<string>(this));
	if(io_setup(IOCB_MAX, &ctx)){
		LOG4CXX_ERROR(logger, "Failed to setup io_context");
	}
}

AioDiskWriter::~AioDiskWriter()
{
	if(ctx && io_destroy(ctx)){
		LOG4CXX_ERROR(logger, "Failed to destroy io_context");
	}
}

bool AioDiskWriter::doWork()
{
	if(wc->empty())
		return true;

	while(iocbs_total.cur() < IOCB_MAX && wc->getNext(current_ws)){
		/** resolve "LBA, TransferLength" + LowHandle
		    -> handles & write it out */
		uint64_t start = SCSI::LBA2OFFSET(current_ws->lba);
		size_t length = SCSI::LBA2OFFSET(current_ws->transfer_length);
		iocb *iocbs[IOCB_MAX] = {};
		size_t iocbs_size = 0;
		for(vector<CommonBufPtr>::iterator itr = current_ws->cbufs.begin();
		    itr != current_ws->cbufs.end() && iocbs_size < IOCB_MAX; ++itr){
			size_t ds_offset = (itr == current_ws->cbufs.begin())?(current_ws->ds_offset):0;
			size_t segment = min(length,
					     ((*itr)->getSize() - ds_offset));
			uint64_t offset = 0;
			length -= segment;
			while(segment > 0){
				void *buf = reinterpret_cast<uint8_t*>((*itr)->head())
					+ ds_offset + offset;
				size_t covered = 0;
				uint64_t real_offset = 0;
				/** LBA + segment = covered from buf with offset*/
				filehandle_t descriptor = getHandle(start, segment, covered, real_offset);
				if(descriptor == INVALID_HANDLE_VALUE){
					LOG4CXX_ERROR(logger, "Failed to resolve real segment");
					return false;
				}

				iocb *iocbuf = new iocb;
				io_prep_pwrite(iocbuf, descriptor, buf, covered, real_offset);
				io_set_callback(iocbuf, NULL);
				iocbs[iocbs_size] = iocbuf;
				++iocbs_size;
				LOG4CXX_DEBUG(logger, "Submit Segment : [" + boost::lexical_cast<string>(descriptor) + "] offset "
					      + boost::lexical_cast<string>(real_offset) + " ("
					      + boost::lexical_cast<string>(SCSI::OFFSET2LBA(real_offset)) + ") "
					      + " " + SCSI::DUMPOCTETS(reinterpret_cast<uint8_t*>(buf),0)
					      + " -> " + boost::lexical_cast<string>(covered));

				start += covered;
				segment -= covered;
				offset += covered;
			}
			if(segment != 0){
				LOG4CXX_ERROR(logger, "Segment mismatch");
				return false;
			}
		}
		if(length != 0){
			LOG4CXX_ERROR(logger, "Length mismatch");
			return false;
		}
		if(iocbs_size >= IOCB_MAX){
			LOG4CXX_ERROR(logger, "Too many cbufs in ws");
			return false;
		}

		io_submit(ctx, iocbs_size, iocbs);
		iocbs_sizes.push(iocbs_size);
		iocbs_total += iocbs_size;
		LOG4CXX_DEBUG(logger, "AIO submit : " + boost::lexical_cast<string>(iocbs_size) +
			      "/" + boost::lexical_cast<string>(iocbs_total.cur()) +
			      " ws " + boost::lexical_cast<string>(current_ws.get()) +
			      "/" + boost::lexical_cast<string>(wc->size()));
	}

	return true;
}

void AioDiskWriter::EventListener::operator()() throw(std::runtime_error)
{
	NDC::push("wlistener[" + boost::lexical_cast<string>(this) +  "]");

	LOG4CXX_INFO(logger, "entering loop.");
	/** disk writer work loop */
	while(!parent->stop){
		boost::mutex::scoped_lock lk(parent->ev_guard);
		boost::xtime xt;
		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.sec += 1;
		parent->event.timed_wait(lk, xt);
		if(!parent->doListenerWork())
			break;
	}
	LOG4CXX_INFO(logger, "exiting loop.");

	NDC::remove();
	return;
}

bool AioDiskWriter::doListenerWork()
{
	bool notified = false;
	while(!iocbs_sizes.empty()){
		io_event io_events[IOCB_MAX] = {};
		timespec timeout = {tv_sec : 1, tv_nsec : 0};
		timeout.tv_sec = 1;
		int events = io_getevents(ctx, 1, IOCB_MAX, io_events, &timeout);
		if(events == 0)
			break;
		LOG4CXX_DEBUG(logger, "AIO getevents : " + boost::lexical_cast<string>(events)
			      + " iocbs[" + boost::lexical_cast<string>(iocbs_sizes.size()) + "]");
		for(int counter = 0; counter < events; ++counter){
			size_t & iocbs_size = iocbs_sizes.front();
			iocb *iocbuf = io_events[counter].obj;
			if(iocbuf->u.c.nbytes != io_events[counter].res){
				LOG4CXX_ERROR(logger, "AIO write length mismatch");
				return false;
			}
			LOG4CXX_DEBUG(logger, "Event Segment : [" + boost::lexical_cast<string>(iocbuf->aio_fildes) + "] offset "
				      + boost::lexical_cast<string>(iocbuf->u.c.offset) + " ("
				      + boost::lexical_cast<string>(SCSI::OFFSET2LBA(iocbuf->u.c.offset)) + ") "
				      + " " + SCSI::DUMPOCTETS(reinterpret_cast<uint8_t*>(iocbuf->u.c.buf),0)
				      + " -> " + boost::lexical_cast<string>(iocbuf->u.c.nbytes));
			delete iocbuf;
			iocbuf = NULL;
			if(--iocbs_size == 0){
				/** clean up front of FIFO queue */
				WriteSegmentPtr ws = wc->front();
				LOG4CXX_DEBUG(logger, "Pop ws : " + boost::lexical_cast<string>(ws.get())
					      + "/" + boost::lexical_cast<string>(wc->size()));
				wc->pop();
				iocbs_sizes.pop();
				if(needNotify()){
					wc->notifyDone();
					notified = true;
				}
			}
			--iocbs_total;
		}
		LOG4CXX_DEBUG(logger, "Done getevents : " + boost::lexical_cast<string>(iocbs_sizes.size()) +
			      "/" + boost::lexical_cast<string>(iocbs_total.cur()));
	}
	if(notified){
		LOG4CXX_DEBUG(logger,"Release CommonPool");
		CommonBuf::releasePool();
	}

	return true;
}

void AioDiskWriter::operator()() throw(std::runtime_error)
{
	boost::thread wlistener(boost::ref(listener));
	DiskWriter::operator()();
	wlistener.join();
	return;
}
