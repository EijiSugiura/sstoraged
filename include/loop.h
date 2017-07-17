/**
   @file loop.h
   @brief Event Multiplexor Loop declarations
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: loop.h 312 2007-09-28 00:56:17Z sugiura $
 */
#ifndef __LOOP_H__
#define __LOOP_H__

#include "common.h"

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include <stdexcept>
#include <vector>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include "port.h"

using namespace std;

/** Loop class */
class Loop {
public:
	/** defaulst constructor */
	Loop() throw() : ports(), stop(false){}
	/**
	   constructor with port
	   @param aport : default port to wait event
	*/
	Loop(PortPtr aport) throw(std::runtime_error)
		: stop(false)
	{ addPort(aport);  }
	/** destructor */
	virtual ~Loop() { ports.clear(); }
	void mayStop() { stop = true; }
	/** loop functor */
	virtual void operator()() throw(std::runtime_error) = 0;
	/** add new port */
	virtual bool addPort(PortPtr aport)
	{
		lock lk(ports_guard);
		ports.push_back(aport);
		return true;
	}
	/** delete exist port */
	virtual bool delPort(PortPtr aport)
	{
		lock lk(ports_guard);
		ports.erase(remove(ports.begin(), ports.end(), aport), ports.end());
		return true;
	}
	virtual bool delPort(socket_t fd)
	{
		lock lk(ports_guard);
		for(vector<PortPtr>::iterator itr = ports.begin();
		    itr != ports.end(); ++itr){
			if((*itr)->getSocket() == fd){
				ports.erase(remove(ports.begin(), ports.end(), *itr),
					    ports.end());
				return true;
			}
		}
		return false;
	}
	virtual void waitWritable(socket_t fd) = 0;
	virtual void unwaitWritable(socket_t fd) = 0;
protected:
	/** port array to wait event */
	vector<PortPtr> ports;
	/** port array mutex */
	boost::mutex ports_guard;
	typedef boost::mutex::scoped_lock lock;
	/** stop flag */
	bool stop;
};

/** Select Loop implementation */
class SelectLoop : public Loop {
public:
	SelectLoop();
	/** constructor with default port
	    @param aport : default port to wait events 
	*/
	SelectLoop(PortPtr aport) throw(std::runtime_error);
	/** destructor */
	~SelectLoop();
	/** select loop functor */
	void operator()() throw(std::runtime_error);
	/** add new port */
	bool addPort(PortPtr aport)
	{
		socket_t fd = aport->getSocket();
		if(fd > static_cast<socket_t>(nfds))
			nfds = fd;
		FD_SET(fd, &readfdcache);
		LOG4CXX_DEBUG(logger, "Add socket [" +
			      boost::lexical_cast<string>(fd) +"]");
		return Loop::addPort(aport);
	}
	/** delete exist port */
	bool delPort(PortPtr aport)
	{
		socket_t fd = aport->getSocket();
		FD_CLR(fd, &readfdcache);
		FD_CLR(fd, &writefdcache);
		LOG4CXX_DEBUG(logger, "Delete socket [" +
			      boost::lexical_cast<string>(fd) +"]");
		return Loop::delPort(aport);
	}
	bool delPort(socket_t fd)
	{
		FD_CLR(fd, &readfdcache);
		FD_CLR(fd, &writefdcache);
		LOG4CXX_DEBUG(logger, "Delete socket [" +
			      boost::lexical_cast<string>(fd) +"]");
		return Loop::delPort(fd);
	}
	void waitWritable(socket_t fd)
	{
		LOG4CXX_DEBUG(logger, "Wait writable socket [" +
			      boost::lexical_cast<string>(fd) +"]");		
		FD_SET(fd, &writefdcache);
	}

	void unwaitWritable(socket_t fd)
	{
// 		LOG4CXX_DEBUG(logger, "Unwait writable socket [" +
// 			      boost::lexical_cast<string>(fd) +"]");		
		FD_CLR(fd, &writefdcache);
	}
protected:
	using Loop::ports;
	using Loop::ports_guard;
private:
	/** read file descriptor set */
	fd_set readfdcache;
	/** write file descriptor set */
	fd_set writefdcache;
	/** 1st argument of select() */
	int nfds;
};

/** Poll Loop implementation */
class PollLoop : public Loop {
public:
	/** constructor with default port
	    @param aport : default port to wait events 
	*/
	PollLoop(PortPtr aport) throw(std::runtime_error)
	{ addPort(aport);  }
	/** poll loop functor */
	void operator()() throw(std::runtime_error);
protected:
	using Loop::ports;
	using Loop::ports_guard;
};

#endif /* __LOOP_H__ */
