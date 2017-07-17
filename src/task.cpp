/**
   @file task.cpp
   @brief Task classes
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: task.cpp 314 2007-09-28 04:07:21Z sugiura $
 */

#include "common.h"

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#include "logger.h"
#include "ioveccontainer.h"
#include "manager.h"
#include "session.h"
#include "connection.h"
#include "volume.h"
#include "task.h"

bool TCPCorkTask::run()
{
	int on = 1;
	if(setsockopt(fd, SOL_TCP, TCP_CORK, &on, sizeof(on)) < 0){
		LOG4CXX_WARN(logger, "Can't set TCP_CORK");
	}
	return true;
}

bool TCPUncorkTask::run()
{
	int off = 0;
	if(setsockopt(fd, SOL_TCP, TCP_CORK, &off, sizeof(off)) < 0){
		LOG4CXX_WARN(logger, "Can't unset TCP_CORK");
	}
	return true;
}

bool CloseSessionTask::run()
{
	iSCSIManager &manager = iSCSIManager::getInstance();
	if(!manager.delSession(session.get()))
		LOG4CXX_ERROR(logger, "Failed to close session");
	return true;
}

bool CloseConnectionTask::run()
{
	if(!session->connection.del(fd))
		LOG4CXX_ERROR(logger, "Failed to close conection [" +
			      boost::lexical_cast<string>(fd) + "]");
	return true;
}

bool CreateReactorTask::run()
{
	iSCSIManager &manager = iSCSIManager::getInstance();

	/** move session&conn -> reactor thread */
	manager.delSession(session.get());
	iSCSIReactorPtr reactor = iSCSIReactorPtr(new iSCSIReactor(session));
	iSCSIConnectionPtr conn = session->connection.find(fd);
	conn->changeListener(reactor);
	/** @todo : wait till old session removed */
	return manager.addReactor(reactor);
}

bool RemoveReactorTask::run()
{
	iSCSIManager &manager = iSCSIManager::getInstance();
	iSCSIReactorPtr reactor = manager.findReactor(session->sid.get());
	reactor->mayStop();
	return true;
}
