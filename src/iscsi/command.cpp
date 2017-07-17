/**
   @file command.cpp
   @brief iSCSI Command handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: command.cpp 225 2007-07-20 03:07:33Z sugiura $
 */

#include <iomanip>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include "configurator.h"
#include "logger.h"
#include "session.h"
#include "scsi/protocol.h"
#include "iscsi/command.h"

CommandHandler::CommandHandler(iSCSISession *parent)
{
	/** supported SCSI command */
	commands[REPORT_LUNS]		= SCSIHandlerPtr(new ReportLUNSHandler);
	commands[INQUIRY]		= SCSIHandlerPtr(new InquiryHandler);
	commands[READ_CAPACITY]		= SCSIHandlerPtr(new ReadCapacityHandler);
	commands[SERVICE_ACTION_IN_16]	= SCSIHandlerPtr(new ReadCapacity16Handler);
	commands[MODE_SENSE]		= SCSIHandlerPtr(new ModeSenseHandler);
	commands[READ_10]		= SCSIHandlerPtr(new ReadHandler(parent));
	commands[WRITE_10]		= SCSIHandlerPtr(new WriteHandler(parent));

	commands[MAINTENANCE_IN]	= SCSIHandlerPtr(new MaintenanceInHandler);

	/** belows are dummy */
	SCSIHandlerPtr dummy = SCSIHandlerPtr(new DummyHandler);
	commands[TEST_UNIT_READY]	= dummy;
	commands[VERIFY]		= dummy;
	commands[SYNCHRONIZE_CACHE]	= dummy;
	commands[START_STOP]		= dummy;
	commands[REASSIGN_BLOCKS]	= dummy;
	commands[RESERVE]		= dummy;
	commands[RELEASE]		= dummy;
}

bool CommandParser::valid() const
{
	/** Check OpCode */
	if(getOpCode() != iSCSI_OP_SCSI_CMD){
		LOG4CXX_ERROR(logger, "Invalid OpCode");
		return false;
	}
	/** Check Read|Write flag */
	if(bhs->flags & COMMAND_FLAG_READ &&
	   bhs->flags & COMMAND_FLAG_WRITE){
		/** @todo : support bi-directional read/write,
		    which uses Bidirectional Read Expected Data Transfer Length AHS */
		LOG4CXX_ERROR(logger, "Bidirectional Read/Write not supported");
		return false;
	}
	/** @todo : Check SCSI Command Attribute */
	/** @todo : Check SN */

	return true;
}

bool CommandHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	LOG4CXX_DEBUG(logger, "COMMAND");
	CommandReqHeader *bhs = reinterpret_cast<CommandReqHeader*>(parent->frontToken());
	CommandParser req(bhs);
	if(!req.valid()){
		LOG4CXX_ERROR(logger, "Invalid COMMAND request");
		return false;
	}

	/** update CMDSN
	    @todo move this section to when SCSI STATUS bit enabled reponse are sent
	 */
	if(!req.isImmediateCmd())
		parent->advanceCMDSNs();

	/** Call SCSI command handler */
	command_map_t::iterator command = commands.find(req.getCDBOpCode());
	if(command == commands.end()){
		ostringstream os;
		os << std::hex << std::setfill('0') << std::setw(2)
		   << (unsigned int)req.getCDBOpCode();
		LOG4CXX_ERROR(logger, "Not supported SCSI Command = " + os.str());
		return false;
	}
	if(!command->second->handle(parent, fd)){
		LOG4CXX_ERROR(logger, "Failed to handle SCSI Command");
		return false;
	}

	return true;
}

SCSIRspGenerator::SCSIRspGenerator()
{
	cbuf = CommonBufPtr(new CommonBuf());
	bhs = reinterpret_cast<SCSIRspHeader*>(cbuf->cur());
	SCSIRspHeader tmp;
	*bhs = tmp;
	cbuf->stepForwardTail(sizeof(*bhs));
#ifdef BHS_DEBUG
	LOG4CXX_DEBUG(logger, "Reply BHS at " +
		      boost::lexical_cast<string>(bhs));
#endif
}

CommonBufPtr SCSIRspGenerator::serialize()
{
	/** No padding, cause no AHS & data */
	LOG4CXX_DEBUG(logger, "RSP : itt " +
		      boost::lexical_cast<string>(ntohl(bhs->itt)));
	return cbuf;
}
