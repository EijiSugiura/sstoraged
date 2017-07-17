/**
   @file port.h
   @brief Port handling classes
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: port.h 312 2007-09-28 00:56:17Z sugiura $
 */
#ifndef __PORT_H__
#define __PORT_H__

#include "common.h"

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

using namespace std;

/** Port class */
class Port {
public:
 	Port() throw() : fd(INVALID_SOCKET), address(), receiver(NULL), sender(NULL) {}
 	Port(const socket_t &accepted);
	Port(const sockaddr_storage &addr) throw (std::runtime_error);
	virtual ~Port();
 	/** socket getter */
 	socket_t getSocket() const throw() { return fd; }
 	/** my own address getter */
 	sockaddr_storage *getAddress() const throw() { return &address; }
 	/** regist receive handler
 	    @param handler : the receive handler
 	    @return true : succeeded
 	    @return false : already registered handler exists
 	 */
 	bool setReceiver(boost::function1<status_t, socket_t> handler);
 	void clearReceiver(){ receiver = NULL; }
 	/** regist send handler
 	    @param handler : the send handler
 	    @return true : succeeded
 	    @return false : already registered handler exists
 	 */
 	bool setSender(boost::function1<status_t, socket_t> handler);

 	/** receiver caller
 	    @param fd : the file descriptor
 	    @param waitwrite : reserve writable flag
 	    @return true : receive succeeded
 	    @return false : receive failed
 	 */
 	status_t onReceive(const socket_t fd);
 	/** sender caller
 	    @param fd : the file descriptor
 	    @param waitwrite : reserve writable flag
 	    @return true : send succeeded
 	    @return false : send failed
 	 */
 	status_t onSend(const socket_t fd);
protected:
 	/** socket validator
 	    @return true : valid socket
 	    @return false : invalid socket
 	*/
 	bool valid() { return (fd != INVALID_SOCKET); }
	/** socket */
	socket_t fd;
	/** my own address */
	mutable struct sockaddr_storage address;
 	/** receive handler */
 	boost::function1<status_t, socket_t> receiver;
 	/** send handler */
 	boost::function1<status_t, socket_t> sender;
};

typedef boost::shared_ptr<Port> PortPtr;

/** NonBlocking port class */
class NonBlockingPort : public Port {
public:
	NonBlockingPort(const socket_t &accepted) throw (std::runtime_error);
	NonBlockingPort(const sockaddr_storage &addr) throw (std::runtime_error);
protected:
	using Port::fd;
	using Port::address;
	using Port::receiver;
	using Port::sender;
};

typedef boost::shared_ptr<NonBlockingPort> NonBlockingPortPtr;

/** Listen port class */
class ListenPort : public Port {
public:
	ListenPort(const sockaddr_storage &addr) throw (std::runtime_error);
protected:
	using Port::fd;
	using Port::address;
	using Port::receiver;
	using Port::sender;
};

typedef boost::shared_ptr<ListenPort> ListenPortPtr;

#endif /* __PORT_H__ */
