/**
   @file send.cpp
   @brief Send Data Task classes
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: send.cpp 160 2007-06-30 09:39:50Z sugiura $
 */

#include "logger.h"
#include "ioveccontainer.h"
#include "manager.h"
#include "session.h"
#include "connection.h"
#include "task.h"

#ifdef LOG4CXX_COUT
#undef LOG4CXX_DEBUG
#define LOG4CXX_DEBUG(logger,message)	strings.push_back(message)
#undef LOG4CXX_ERROR
#define LOG4CXX_ERROR(logger,message)	strings.push_back(message)
extern vector<string> strings;
#endif

SendTask::SendTask(iSCSISessionPtr parent,
		   socket_t afd,
		   CommonBufPtr acbuf)
	: cbufs(), session(parent), fd(afd), total_remain(0)
{
	this->push(acbuf);
}

void SendTask::push(boost::shared_ptr<CommonBuf> acbuf)
{
	cbufs.push_back(acbuf);
	total_remain += acbuf->getRemain();
}

bool SendTask::stepForward(const size_t size)
{
	if(cbufs.empty()){
		LOG4CXX_ERROR(logger, "SendTask::Empty buffer!");
		return false;
	}
	size_t tmp = size;
	/** step forward till sent size */
	while(tmp > 0 && !cbufs.empty()){
		CommonBufPtr cbuf = cbufs.front();
		size_t remain = cbuf->getRemain();
		if(remain <= tmp){
			cbufs.pop_front();
			tmp -= remain;
			total_remain -= remain;
		} else {
			cbuf->stepForwardOffset(tmp);
			total_remain -= tmp;
			return true;
		}
	}
	return true;
}

bool SendTask::run()
{
	iSCSIConnectionPtr conn = session->connection.find(fd);
	while(this->size() != 0){
		IovecContainer buf(this);
#ifndef LOG4CXX_COUT
		ssize_t size = writev(fd, buf.refVec(), buf.getCount());
#else
		ssize_t size = min(500UL, (unsigned long)buf.getRemain());
#endif
		if(size <= 0)
			break;
		LOG4CXX_DEBUG(logger, "Sent : " +
			      boost::lexical_cast<string>(size) +
			      " remain " +
			      boost::lexical_cast<string>(this->getRemain())
#if 0
			      + " " + SCSI::DUMPBHS(reinterpret_cast<uint8_t*>(buf.refVec()->iov_base))
#endif
			      );
		if(buf.getRemain() == static_cast<size_t>(size)){
			conn->unwaitWritable();
			return true;
		} else
			this->stepForward(size);
	}
	conn->waitWritable();
	return false;
}
