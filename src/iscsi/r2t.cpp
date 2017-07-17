/**
   @file rt2.cpp
   @brief iSCSI R2T handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: r2t.cpp 162 2007-07-02 11:55:24Z sugiura $
 */

#include "logger.h"
#include "iscsi/r2t.h"

R2TGenerator::R2TGenerator()
	: cbuf(CommonBufPtr(new CommonBuf()))
{
	bhs = reinterpret_cast<R2THeader*>(cbuf->cur());
	R2THeader tmp;
	*bhs = tmp;
	cbuf->stepForwardTail(sizeof(*bhs));
#ifdef BHS_DEBUG
	LOG4CXX_DEBUG(logger, "Reply BHS at " +
		      boost::lexical_cast<string>(bhs));
#endif
	bhs->flags = ISCSI_FLAG_FINAL;
}

CommonBufPtr R2TGenerator::serialize()
{
	/** no need to padding, cause bhs only */
	/** @todo : support bi-directional Read/Write with AHS */
	LOG4CXX_DEBUG(logger, "R2T : itt " +
		      boost::lexical_cast<string>(ntohl(bhs->itt)) + " offset " +
		      boost::lexical_cast<string>(ntohl(bhs->offset)) + " r2tsn " +
		      boost::lexical_cast<string>(ntohl(bhs->r2tsn)));
	return cbuf;
}
