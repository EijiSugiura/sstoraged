/**
   @file nopin.cpp
   @brief iSCSI Nop-In handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: nopin.cpp 229 2007-07-23 04:44:49Z sugiura $
 */

#include "common.h"
#include "session.h"
#include "logger.h"
#include "iscsi/nopin.h"

bool NopInParser::valid() const
{
	/** Check OpCode */
	if(getOpCode() != iSCSI_OP_NOOP_IN){
		LOG4CXX_ERROR(logger, "Invalid OpCode");
		return false;
	}
	/** Check Final flag */
	if(!(isFinal())){
		LOG4CXX_ERROR(logger, "Initiator MUST set final bit");
		return false;
	}
	if(getTTT() != ISCSI_RSVD_TASK_TAG){
		/** @todo :  Only Ping Nop-In request is supported */
		LOG4CXX_ERROR(logger, "Update StatSN Nop-In request not supported");
		return false;
	}

	/** @todo : Check SN */

	return true;
}

bool NopInHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	LOG4CXX_DEBUG(logger, "NopIn");
	NopInHeader *bhs = reinterpret_cast<NopInHeader*>(parent->frontToken());
	NopInParser req(bhs);
	if(!req.valid()){
		LOG4CXX_ERROR(logger, "Invalid Nop-In request");
		return false;
	}

	LOG4CXX_WARN(logger, "Nothing to do, yet");
	return true;
}

NopInGenerator::NopInGenerator()
{
	cbuf = CommonBufPtr(new CommonBuf());
	bhs = reinterpret_cast<NopInHeader*>(cbuf->cur());
	cbuf->stepForwardTail(sizeof(*bhs));
#ifdef BHS_DEBUG
	LOG4CXX_DEBUG(logger, "Reply BHS at " +
		      boost::lexical_cast<string>(bhs));
#endif
	memset(bhs, 0, sizeof(*bhs));
	bhs->opcode = iSCSI_OP_NOOP_IN;
	bhs->flags = ISCSI_FLAG_FINAL;
	/** @todo : support AHS */
	bhs->hlength = 0;
}
