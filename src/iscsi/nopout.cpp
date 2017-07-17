/**
   @file nopout.cpp
   @brief iSCSI Nop-Out handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: nopout.cpp 277 2007-08-14 14:44:47Z sugiura $
 */

#include "common.h"
#include "session.h"
#include "logger.h"
#include "iscsi/nopout.h"
#include "iscsi/nopin.h"

bool NopOutParser::valid() const
{
	/** Check OpCode */
	if(getOpCode() != iSCSI_OP_NOOP_OUT){
		LOG4CXX_ERROR(logger, "Invalid OpCode");
		return false;
	}
	/** Check Final flag */
	if(!(isFinal())){
		LOG4CXX_ERROR(logger, "Initiator MUST set final bit");
		return false;
	}
	/** Check ITT */
	if(getITT() == ISCSI_RSVD_TASK_TAG){
		LOG4CXX_ERROR(logger, "NopOut : Invalid ITT");
		return false;
	}
	/** Check TTT */
	if(getTTT() != ISCSI_RSVD_TASK_TAG){
		/** @todo :  Only Ping Nop-Out request is supported */
		LOG4CXX_ERROR(logger, "NopOut as ping echo is not supported");
		return false;
	}
	/** Check ping data length */
	if(getDlength() > 0){
		/** @todo : Support ping data */
		LOG4CXX_ERROR(logger, "Ping data not supported, yet");
		return false;
	}

	/** @todo : Check SN */

	return true;
}

bool NopOutHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	LOG4CXX_DEBUG(logger, "NopOut");
	NopOutHeader *bhs = reinterpret_cast<NopOutHeader*>(parent->frontToken());
	NopOutParser req(bhs);
	if(!req.valid()){
		LOG4CXX_ERROR(logger, "Invalid Nop-Out request");
		return false;
	}

	/** update CMDSN */
	if(!req.isImmediateCmd())
		parent->advanceCMDSNs();

	NopInGenerator reply;
	reply.setITT(req.getITT());
	/** ...then, set Reserved Task Tag, too */
	reply.setTTT(ISCSI_RSVD_TASK_TAG);

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

NopOutGenerator::NopOutGenerator()
{
	cbuf = CommonBufPtr(new CommonBuf());
	bhs = reinterpret_cast<NopOutHeader*>(cbuf->cur());
	cbuf->stepForwardTail(sizeof(*bhs));
#ifdef BHS_DEBUG
	LOG4CXX_DEBUG(logger, "Reply BHS at " +
		      boost::lexical_cast<string>(bhs));
#endif
	bhs->opcode = iSCSI_OP_NOOP_OUT;
	bhs->flags = ISCSI_FLAG_FINAL;
}
