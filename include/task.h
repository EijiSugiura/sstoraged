/** 
 * @file  task.h
 * @brief Task classes
 * @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
 * @version $Id: task.h 312 2007-09-28 00:56:17Z sugiura $
 */
#ifndef __SSTORAGE_TASK_H__
#define __SSTORAGE_TASK_H__

#include "common.h"

#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

#include <queue>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

/**
   Task interface class
 */
class Task {
public:
	/** Default constructor */
	Task()
	{
#if 0
		LOG4CXX_DEBUG(logger, "Task Constructed : " +
			      boost::lexical_cast<string>(this));
#endif
	}

	/** Destructor */
	virtual ~Task()
	{
#if 0
		LOG4CXX_DEBUG(logger, "Task Destructed : " +
			      boost::lexical_cast<string>(this));
#endif
	}

	/** execute task */
	virtual bool run() = 0;
};

typedef boost::shared_ptr<Task> TaskPtr;

class iSCSISession;
class CommonBuf;
/**
   Send data task
 */
class SendTask : public Task {
public:
	SendTask(boost::shared_ptr<iSCSISession> parent,
		 socket_t afd,
		 boost::shared_ptr<CommonBuf> acbuf);
	bool run();

	void push(boost::shared_ptr<CommonBuf> acbuf);
	list<boost::shared_ptr<CommonBuf> >::iterator begin()
	{ return cbufs.begin(); }
	size_t size() { return cbufs.size(); }
	size_t getRemain() const { return total_remain; }
	bool stepForward(const size_t size);
protected:
	list<boost::shared_ptr<CommonBuf> > cbufs;
	boost::shared_ptr<iSCSISession> session;
	socket_t fd;
	size_t total_remain;
};

typedef boost::shared_ptr<SendTask> SendTaskPtr;

/**
   Set TCP_CORK
 */
class TCPCorkTask : public Task {
public:
	TCPCorkTask(socket_t afd)
		: fd(afd) {}
	bool run();
private:
	socket_t fd;
};

typedef boost::shared_ptr<TCPCorkTask> TCPCorkTaskPtr;

/**
   Unset TCP_CORK
 */
class TCPUncorkTask : public Task {
public:
	TCPUncorkTask(socket_t afd)
		: fd(afd) {}
	bool run();
private:
	socket_t fd;
};

typedef boost::shared_ptr<TCPUncorkTask> TCPUncorkTaskPtr;

class LogicalVolume;
/**
   sendfile() task
 */
class SendfileTask : public Task {
public:
	SendfileTask(boost::shared_ptr<iSCSISession> parent,
		     const socket_t afd,
		     boost::shared_ptr<LogicalVolume> alv,
		     const uint64_t lba, const size_t length);
	bool run();
protected:
	boost::shared_ptr<iSCSISession> session;
	socket_t fd;
	boost::shared_ptr<LogicalVolume> lv;
	uint64_t offset;
	size_t remain;
};

typedef boost::shared_ptr<SendfileTask> SendfileTaskPtr;

class WriteSegment;
/**
   Send Cache task
 */
class SendCacheTask : public Task {
public:
	SendCacheTask(boost::shared_ptr<iSCSISession> parent, socket_t _fd,
		      boost::shared_ptr<WriteSegment> _ws,
		      uint64_t lba, uint32_t length);
	bool run();

	size_t size() const;
	vector<boost::shared_ptr<CommonBuf> >::iterator begin(size_t &offset);
	size_t getRemain() const { return total_remain; }
	bool stepForward(const size_t size);
protected:
	bool _stepForward(const size_t size);
	boost::shared_ptr<iSCSISession> session;
	socket_t fd;
	boost::shared_ptr<WriteSegment> ws;
	/** LBA to send */
	uint64_t clba;
	/** total remain to send */
	size_t total_remain;
	/** current cbuf index */
	size_t cur_index;
	/** offset from cbuf's head */
	size_t offset;
};

typedef boost::shared_ptr<SendCacheTask> SendCacheTaskPtr;

/**
   Close session task
 */
class CloseSessionTask : public Task {
public:
	CloseSessionTask(boost::shared_ptr<iSCSISession> parent)
		: session(parent) {}
	bool run();
private:
	boost::shared_ptr<iSCSISession> session;
};

typedef boost::shared_ptr<CloseSessionTask> CloseSessionTaskPtr;

/**
   Close connection task
 */
class CloseConnectionTask : public Task {
public:
	CloseConnectionTask(boost::shared_ptr<iSCSISession> parent,
			    socket_t afd)
		: session(parent), fd(afd) {}
	bool run();
private:
	boost::shared_ptr<iSCSISession> session;
	socket_t fd;
};

typedef boost::shared_ptr<CloseConnectionTask> CloseConnectionTaskPtr;
/**
   Create reactor thread task
 */
class CreateReactorTask : public Task {
public:
	CreateReactorTask(boost::shared_ptr<iSCSISession> parent,
			  socket_t afd)
		: session(parent), fd(afd) {}
	bool run();
private:
	boost::shared_ptr<iSCSISession> session;
	socket_t fd;
};

typedef boost::shared_ptr<CreateReactorTask> CreateReactorTaskPtr;

/**
   Remove reactor thread task
 */
class RemoveReactorTask : public Task {
public:
	RemoveReactorTask(boost::shared_ptr<iSCSISession> parent)
		: session(parent) {}
	bool run();
private:
	boost::shared_ptr<iSCSISession> session;
};

typedef boost::shared_ptr<RemoveReactorTask> RemoveReactorTaskPtr;

#endif /* __SSTORAGE_TASK_H__ */
