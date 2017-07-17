/**
   @file taskmng.cpp
   @brief iSCSI TaskMng handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: taskmng.cpp 225 2007-07-20 03:07:33Z sugiura $
 */

#include <boost/lexical_cast.hpp>
#include "configurator.h"
#include "logger.h"
#include "session.h"
#include "iscsi/taskmng.h"

bool TaskMngParser::valid() const
{
	/** Check OpCode */
	if(getOpCode() != iSCSI_OP_TASK_MGT_REQ){
		LOG4CXX_ERROR(logger, "Invalid OpCode");
		return false;
	}
	/** Check Final flag */
	if(!(isFinal())){
		LOG4CXX_ERROR(logger, "Initiator MUST set final bit");
		return false;
	}
	/** Check AHS length */
	if(getHlength() != 0){
		LOG4CXX_ERROR(logger, "Invalid AHS length");
		return false;
	}
	/** Check Data length */
	if(getDlength() != 0){
		LOG4CXX_ERROR(logger, "Invalid Data length");
		return false;
	}
	/** Check RTT */
	if(getFunction() != TASK_FUNC_ABORT_TASK &&
	   getFunction() != TASK_FUNC_TASK_REASSIGN &&
	   getRTT() != ISCSI_RSVD_TASK_TAG){
		LOG4CXX_ERROR(logger, "Invalid RTT");
		return false;
	}

	/** @todo : Check SN */

	return true;
}

bool TaskMngHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	TaskMngReqHeader *bhs = reinterpret_cast<TaskMngReqHeader*>(parent->frontToken());
	TaskMngParser req(bhs);
	LOG4CXX_DEBUG(logger, "TASKMNG [" +
		      boost::lexical_cast<string>(req.getFunction()) + "]");
	if(!req.valid()){
		LOG4CXX_ERROR(logger, "Invalid TASKMNG request");
		return false;
	}

	/** update CMDSN */
	if(!req.isImmediateCmd())
		parent->advanceCMDSNs();

	/** generate TaskMng reply */
 	TaskMngGenerator reply;
	switch(req.getFunction()){
	default:
		reply.setResponse(TASKMNG_FUNC_COMPLETE);
		break;
	}

	reply.setITT(req.getITT());
	reply.setLUN(req.getLUN());
	/** update STATSN */
	iSCSIConnectionPtr conn = parent->connection.find(fd);
	reply.setSTATSN(conn->getSTATSN());
	conn->advanceSTATSN();
	reply.setEXPCMDSN(parent->getEXPCMDSN());
	reply.setMAXCMDSN(parent->getMAXCMDSN());

	/** convert reply -> send task */
	parent->sendtask.push(SendTaskPtr(new SendTask(parent, fd, reply.serialize())));
	return true;
}

TaskMngGenerator::TaskMngGenerator()
{
	cbuf = CommonBufPtr(new CommonBuf());
	bhs = reinterpret_cast<TaskMngRspHeader*>(cbuf->cur());
	TaskMngRspHeader tmp;
	*bhs = tmp;
	cbuf->stepForwardTail(sizeof(*bhs));
#ifdef BHS_DEBUG
	LOG4CXX_DEBUG(logger, "Reply BHS at " +
		      boost::lexical_cast<string>(bhs));
#endif
	bhs->flags = ISCSI_FLAG_FINAL;
}
