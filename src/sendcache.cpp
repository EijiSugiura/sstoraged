/**
   @file sendcache.cpp
   @brief Send Cache data Task classes
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: sendcache.cpp 225 2007-07-20 03:07:33Z sugiura $
 */

#include "logger.h"
#include "ioveccontainer.h"
#include "session.h"
#include "connection.h"
#include "writecache.h"
#include "task.h"

#ifdef LOG4CXX_COUT
#undef LOG4CXX_DEBUG
#define LOG4CXX_DEBUG(logger,message)	strings.push_back(message)
#undef LOG4CXX_ERROR
#define LOG4CXX_ERROR(logger,message)	strings.push_back(message)
extern vector<string> strings;
#endif

SendCacheTask::SendCacheTask(boost::shared_ptr<iSCSISession> parent, socket_t _fd,
			     boost::shared_ptr<WriteSegment> _ws,
			     uint64_t lba, uint32_t length)
	: session(parent), fd(_fd), ws(_ws),
	  clba(lba), total_remain(SCSI::LBA2OFFSET(length)),
	  cur_index(0), offset(0)
{
	/** @todo : fix (ws->lba + offset => clba) and cur_index */
 	if(ws->lba < clba){
		total_remain += SCSI::LBA2OFFSET(clba - ws->lba);
 		_stepForward(SCSI::LBA2OFFSET(clba - ws->lba));
		total_remain -= SCSI::LBA2OFFSET(clba - ws->lba);
	}
}

bool SendCacheTask::run()
{
	iSCSIConnectionPtr conn = session->connection.find(fd);
	while(this->size() != 0){
		IovecContainer buf(this);
#ifndef LOG4CXX_COUT
		ssize_t size = writev(fd, buf.refVec(), buf.getCount());
#else
		ssize_t size = min(500UL, (unsigned long)this->getRemain());
#endif
		if(size <= 0)
			break;
		LOG4CXX_DEBUG(logger, "SendCache : " +
			      boost::lexical_cast<string>(size) +
			      " lba " + boost::lexical_cast<string>(this->clba) +
			      "/" + boost::lexical_cast<string>(ws->lba) +
			      " remain " + boost::lexical_cast<string>(this->getRemain()) +
			      " offset " + boost::lexical_cast<string>(this->offset)
#ifdef CHECK_DATA_INTEGRITY
			      + " " +
			      SCSI::DUMPOCTETS(reinterpret_cast<uint8_t*>(buf.refVec()->iov_base))
#endif
			      );
		if(this->getRemain() == static_cast<size_t>(size)){
			conn->unwaitWritable();
			return true;
		} else if(!this->stepForward(size)) {
			LOG4CXX_ERROR(logger, "Internal error in send cache");
			return true;
		}
	}
	conn->waitWritable();
	return false;
}

size_t SendCacheTask::size() const
{
	return ws->cbufs.size()-cur_index;
}

vector<CommonBufPtr>::iterator SendCacheTask::begin(size_t &_offset)
{
	if(cur_index == 0)
		_offset = offset + ws->ds_offset;
	else
		_offset = offset;
	return ws->cbufs.begin()+cur_index;
}

bool SendCacheTask::_stepForward(const size_t size)
{
	size_t tmp = size;
	/** step forward till sent size */
	while(tmp > 0){
		if(cur_index >= ws->cbufs.size())
			return false;
		CommonBufPtr cbuf = ws->cbufs[cur_index];
		size_t remain = min(static_cast<size_t>(total_remain),
				    static_cast<size_t>(cbuf->getSize() - this->offset -
							((cur_index == 0)?(ws->ds_offset):0)));
		if(remain <= tmp){
			++cur_index;
			this->offset = 0;
			tmp -= remain;
		} else {
			this->offset += tmp;
			tmp = 0;
			break;
		}
	}
	if(tmp != 0)
		return false;
	return true;
}

bool SendCacheTask::stepForward(const size_t size)
{
	if(this->size() == 0){
		LOG4CXX_ERROR(logger, "SendCacheTask: Empty buffer!");
		return false;
	}
	if(size > total_remain || !_stepForward(size)){
		LOG4CXX_ERROR(logger, "SendCacheTask: Too large offset");
		return false;
	}
	total_remain -= size;
	return true;
}

