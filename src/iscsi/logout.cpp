/**
   @file logout.cpp
   @brief iSCSI Logout handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: logout.cpp 162 2007-07-02 11:55:24Z sugiura $
 */

#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include "configurator.h"
#include "logger.h"
#include "session.h"
#include "writecache.h"
#include "iscsi/logout.h"
#include "iscsi/keyvalue.h"

bool LogoutParser::valid() const
{
	/** Check OpCode */
	if(getOpCode() != iSCSI_OP_LOGOUT_CMD){
		LOG4CXX_ERROR(logger, "Invalid OpCode");
		return false;
	}
	/** Check Final flag */
	if(!(isFinal())){
		LOG4CXX_ERROR(logger, "Initiator MUST set final bit");
		return false;
	}

	/** @todo : Check SN */

	return true;
}

bool LogoutHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	LOG4CXX_DEBUG(logger, "LOGOUT");
	LogoutReqHeader *bhs = reinterpret_cast<LogoutReqHeader*>(parent->frontToken());
	LogoutParser req(bhs);
	if(!req.valid()){
		LOG4CXX_ERROR(logger, "Invalid LOGOUT request");
		return false;
	}

	/** local tasks list */
	list<TaskPtr> tasks;

	/** generate Logout reply */
 	LogoutGenerator reply;
	reply.setResponse(LOGOUT_SUCCESS);
	/** @todo : terminate pending commands, when reason code is
	    "close the connection/session". */
	switch(req.getReason()){
	case LOGOUT_REASON_CLOSE_SESSION:
		if(parent->type.get() == iSCSI_DISCOVERY_SESSION)
			tasks.push_back(CloseSessionTaskPtr(new CloseSessionTask(parent)));
		else
			tasks.push_back(RemoveReactorTaskPtr(new RemoveReactorTask(parent)));
		break;
	case LOGOUT_REASON_CLOSE_CONNECTION:
		/** CID may be performed on a different transport connection
		    when the TCP connection for the CID has already been
		    terminated. */
		if(parent->connection.find(req.getCID()).get() == NULL){
			reply.setResponse(LOGOUT_CID_NOT_FOUND);
			if(parent->type.get() == iSCSI_DISCOVERY_SESSION)
				tasks.push_back(CloseSessionTaskPtr(new CloseSessionTask(parent)));
			else
				tasks.push_back(RemoveReactorTaskPtr(new RemoveReactorTask(parent)));
		/** @todo : request stop reactor thread to main thread, for Normal session*/
		} else
			tasks.push_back(CloseConnectionTaskPtr(new CloseConnectionTask(parent,
										       fd)));
		break;
	case LOGOUT_REASON_RECOVERY:
		LOG4CXX_ERROR(logger, "Not support RECOVERY");
		reply.setResponse(LOGOUT_RECOVERY_UNSUPPORTED);
		break;
	default:
		reply.setResponse(LOGOUT_CLEANUP_FAILED);
		break;
	}

	reply.setITT(req.getITT());
	uint32_t val = 0;
	parent->getAttr("DefaultTime2Wait", val);
	reply.setTime2Wait(static_cast<uint16_t>(val));
	parent->getAttr("DefaultTime2Retain", val);
	reply.setTime2Retain(static_cast<uint16_t>(val));
	
	/** update STATSN */
	iSCSIConnectionPtr conn = parent->connection.find(fd);
	reply.setSTATSN(conn->getSTATSN());
	conn->advanceSTATSN();

	reply.setEXPCMDSN(parent->getEXPCMDSN());
	reply.setMAXCMDSN(parent->getMAXCMDSN());

	/** convert reply -> send task */
	tasks.push_front(SendTaskPtr(new SendTask(parent, fd, reply.serialize())));
	parent->sendtask.push(tasks);
	return true;
}

LogoutGenerator::LogoutGenerator()
{
	cbuf = CommonBufPtr(new CommonBuf());
	bhs = reinterpret_cast<LogoutRspHeader*>(cbuf->cur());
	LogoutRspHeader tmp;
	*bhs = tmp;
	cbuf->stepForwardTail(sizeof(*bhs));
#ifdef BHS_DEBUG
	LOG4CXX_DEBUG(logger, "Reply BHS at " +
		      boost::lexical_cast<string>(bhs));
#endif
	bhs->flags = ISCSI_FLAG_FINAL;
}
