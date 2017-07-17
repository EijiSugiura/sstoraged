/**
   @file port.cpp
   @brief Port management class
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: port.cpp 314 2007-09-28 04:07:21Z sugiura $
 */

#include "common.h"

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include "inetutils.h"
#include "port.h"

#ifdef LOG4CXX_COUT
#include <iostream>
#undef LOG4CXX_DEBUG
#define LOG4CXX_DEBUG(logger,message)	cout << (message) << endl
#undef LOG4CXX_INFO
#define LOG4CXX_INFO(logger,message)	cout << (message) << endl
#undef LOG4CXX_ERROR
#define LOG4CXX_ERROR(logger,message)	cerr << (message) << endl
#endif

static inline string GetErrorMessage(const int number){
#ifdef HAVE_STRERROR_R
	char buf[MAX_CHAR_PER_LINE] = {};
	strerror_r(number, buf, sizeof(buf)-1);
	return static_cast<string>(buf);
#else
	return static_cast<string>(strerror(number));
#endif
}

Port::Port(const socket_t &accepted) : fd(accepted), address(), receiver(NULL), sender(NULL)
{
	socklen_t len = sizeof(address);
	getpeername(fd, reinterpret_cast<sockaddr*>(&address), &len);
}
Port::Port(const sockaddr_storage &addr) throw (runtime_error)
	: fd(INVALID_SOCKET), address(addr), receiver(NULL), sender(NULL)
{
	try {
		fd = socket(addr.ss_family, SOCK_STREAM, IPPROTO_TCP);
		if(!valid())
			throw runtime_error("Failed to open socket : " + GetErrorMessage(errno));
	} catch(const std::exception &e) {
		throw runtime_error(e.what());
	}
}
Port::~Port()
{
	if(valid()){
		close(fd);
		LOG4CXX_INFO(logger, "port closed [" + boost::lexical_cast<string>(fd) 
			     + "] for "
			     + InetUtil::getAddrPortStr(address));
		fd = INVALID_SOCKET;
	}
}

bool Port::setReceiver(boost::function1<status_t, socket_t> handler)
{
	if(receiver)
		return false;
	receiver = handler;
	return true;
}

bool Port::setSender(boost::function1<status_t, socket_t> handler)
{
	if(sender)
		return false;
	sender = handler;
	return true;
}

status_t Port::onReceive(const socket_t fd)
{
	if(!receiver)
		throw(std::runtime_error("Port receiver not specified for ["
					 + boost::lexical_cast<string>(fd) + "]"));
	return receiver(fd);
}

status_t Port::onSend(const socket_t fd)
{
	if(!sender)
		throw(std::runtime_error("Port sender not specified for ["
					 + boost::lexical_cast<string>(fd) + "]"));
	return sender(fd);
}

NonBlockingPort::NonBlockingPort(const socket_t &accepted) throw (runtime_error)
{
	try {
		fd = accepted;
		socklen_t len = sizeof(address);
		getpeername(fd, reinterpret_cast<sockaddr*>(&address), &len);
#ifndef HAVE_WINSOCK2_H
		int opts = fcntl(fd, F_GETFL);
		if(opts < 0)
			throw runtime_error("Failed to get socket options : " + GetErrorMessage(errno));
		if(fcntl(fd, F_SETFL, (opts|O_NONBLOCK)) < 0)
			throw runtime_error("Failed to set nonblocking mode : " + GetErrorMessage(errno));
#else	// HAVE_WINSOCK2_H
		u_long opts = 1;
		if(ioctlsocket(fd, FIONBIO, &opts) == SOCKET_ERROR)
			throw runtime_error("Failed to set nonblocking mode : " + GetErrorMessage(errno));
#endif
	} catch(const std::exception &e) {
		throw runtime_error(e.what());
	}
}
NonBlockingPort::NonBlockingPort(const sockaddr_storage &addr) throw (runtime_error)
{
	address = addr;
	try {
		fd = socket(addr.ss_family, SOCK_STREAM, IPPROTO_TCP);
		if(!valid())
			throw runtime_error("Failed to open nonblocking socket : " + GetErrorMessage(errno));
#ifndef HAVE_WINSOCK2_H
		int opts = fcntl(fd, F_GETFL);
		if(opts < 0)
			throw runtime_error("Failed to get socket options : " + GetErrorMessage(errno));
		if(fcntl(fd, F_SETFL, (opts|O_NONBLOCK)) < 0)
			throw runtime_error("Failed to set nonblocking mode : " + GetErrorMessage(errno));
#else	// HAVE_WINSOCK2_H
		u_long opts = 1;
		if(ioctlsocket(fd, FIONBIO, &opts) == SOCKET_ERROR)
			throw runtime_error("Failed to set nonblocking mode : " + GetErrorMessage(errno));
#endif
	} catch(const std::exception &e) {
		throw runtime_error(e.what());
	}
}

ListenPort::ListenPort(const sockaddr_storage &addr) throw (runtime_error)
{
	address = addr;
	try {
		fd = socket(addr.ss_family, SOCK_STREAM, IPPROTO_TCP);
		if(!valid()) {
			throw runtime_error("Failed to open socket : " + GetErrorMessage(errno));
		}
		if(bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(sockaddr_storage)) < 0)
			throw runtime_error("Failed to bind socket : " + GetErrorMessage(errno));
		if(::listen(fd, SOMAXCONN) < 0)
			throw runtime_error("Failed to listen socket : " + GetErrorMessage(errno));
	} catch(const std::exception &e) {
		throw runtime_error(e.what());
	}
}
