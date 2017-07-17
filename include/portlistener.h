/**
   @file portlistener.h
   @brief Port Listener
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: portlistener.h 312 2007-09-28 00:56:17Z sugiura $
 */
#ifndef __PORTLISTENER_H__
#define __PORTLISTENER_H__

#include "common.h"

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include "port.h"
#include "loop.h"

using namespace std;

/** PortListener class */
class PortListener {
public:
	PortListener() throw(std::runtime_error)
		: loop(new SelectLoop) {}
	virtual ~PortListener() { delete loop; }
	/** main loop of PortListener */
	void operator()() throw(std::runtime_error);
	status_t recv(const socket_t fd) { return 0; };

	bool addPort(const PortPtr port) throw()
	{ return loop->addPort(port); }
	bool delPort(const PortPtr port) throw()
	{ return loop->delPort(port); }
	void waitWritable(const PortPtr port) throw()
	{ loop->waitWritable(port->getSocket()); }
	void unwaitWritable(const PortPtr port) throw()
	{ loop->unwaitWritable(port->getSocket()); }
protected:
	/** loop implementation */
	Loop *loop;	
};

typedef boost::shared_ptr<PortListener> PortListenerPtr;

#endif /* __PORTLISTENER_H__ */
