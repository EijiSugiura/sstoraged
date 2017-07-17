/**
   @file acceptor.cpp
   @brief Port Acceptor class
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: acceptor.cpp 212 2007-07-18 06:54:55Z sugiura $
 */

#include <sys/socket.h>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include "logger.h"
#include "inetutils.h"
#include "port.h"
#include "portlistener.h"
#include "acceptor.h"
#include "manager.h"
#include "session.h"
#include "connection.h"

socket_t Acceptor::do_accept(const socket_t fd,
			     sockaddr_storage &addr)
{
	socklen_t socklen = sizeof(addr);
	return ::accept(fd, reinterpret_cast<sockaddr*>(&addr),
				 &socklen);
}

status_t iSCSIAcceptor::accept(const socket_t fd)
{
	sockaddr_storage addr = {};
	socket_t client = do_accept(fd, addr);
	if(client < 0)
		return -errno;
	LOG4CXX_INFO(logger, "port accepted ["
		     + boost::lexical_cast<string>(client) + "] from " 
		     + InetUtil::getAddrPortStr(addr));

	/** add client socket to iSCSI manager */
 	PortPtr client_port = PortPtr(new NonBlockingPort(client));
	iSCSISession *session = new iSCSISession();
	/** set receiver/sender */
	client_port->setReceiver(boost::bind(&iSCSISession::recv,session,_1));
	client_port->setSender(boost::bind(&iSCSISession::send,session,_1));

	iSCSIManager &manager = iSCSIManager::getInstance();
	iSCSIConnectionPtr conn = iSCSIConnectionPtr(new iSCSIConnection(client_port,
									 manager.getListener()));
	session->connection.add(conn);

	iSCSISessionPtr session_ptr = iSCSISessionPtr(session);
	/** @todo : fix return as status_t
	    and Loop::ports is changed,
	    so iterator must be updated...
	 */
	if(!manager.addSession(session_ptr))
		return -EIO;
	return 0;
}
