/**
   @file connection.cpp
   @brief Connection handling classes
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: connection.cpp 45 2007-04-20 09:49:07Z sugiura $
 */

#include <boost/bind.hpp>
#include "portlistener.h"
#include "reactor.h"
#include "connection.h"

iSCSIConnection::iSCSIConnection(const PortPtr accepted, PortListenerPtr actor)
	: cid(0), listener(actor)
{
	port = accepted;
	LOG4CXX_DEBUG(logger, "Connection Constructed : " +
		      boost::lexical_cast<string>(this));
	listener->addPort(port);
}

iSCSIConnection::~iSCSIConnection()
{
	LOG4CXX_DEBUG(logger, "Connection Destructed : " +
		      boost::lexical_cast<string>(this));
	if(listener.get() != NULL)
		listener->delPort(port);
}

void iSCSIConnection::waitWritable()
{
	listener->waitWritable(port);
}
void iSCSIConnection::unwaitWritable()
{
	listener->unwaitWritable(port);
}

void iSCSIConnection::changeListener(iSCSIReactorPtr actor)
{
	if(listener.get() != NULL)
		listener->delPort(port);
	listener = actor;
	listener->addPort(port);
	port->clearReceiver();
	port->setReceiver(boost::bind(&iSCSIReactor::recv,actor,_1));
	LOG4CXX_DEBUG(logger, "Change listener to reactor");
}

void iSCSIConnection::clearListener()
{
	listener->delPort(port);
	port->clearReceiver();

	iSCSIReactorPtr null;
	listener = null;
}
