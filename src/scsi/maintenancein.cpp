/**
   @file maintenancein.cpp
   @brief SCSI MaintenanceIn handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: maintenancein.cpp 312 2007-09-28 00:56:17Z sugiura $
 */

#include "common.h"

#ifdef HAVE_SCSI_SCSI_H
#include <scsi/scsi.h>
#else
#include "scsi.h"
#endif

#include "configurator.h"
#include "logger.h"
#include "session.h"
#include "volume.h"
#include "iscsi/protocol.h"
#include "scsi/generic.h"
#include "scsi/maintenancein.h"

MaintenanceInHandler::MaintenanceInHandler()
{}

bool MaintenanceInParser::valid() const
{
	/** Check OpCode */
	if(getOpCode() != MAINTENANCE_IN){
		LOG4CXX_ERROR(logger, "Invalid CDB OpCode");
		return false;
	}
	if(!isReportSupportedOpCodes()){
		LOG4CXX_ERROR(logger, "Invalid Service Action");
	}
	/** @todo : Check SN */

	return true;
}

bool MaintenanceInHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	LOG4CXX_DEBUG(logger, "READ CAPACITY(10)");
	CommandParser req(reinterpret_cast<CommandReqHeader*>(parent->frontToken()));
	MaintenanceInParser cdb(reinterpret_cast<MaintenanceInCommand*>(req.getSCB()));

	if(!(req.getFlags() & COMMAND_FLAG_READ) ||
	   req.getFlags() & COMMAND_FLAG_WRITE ||
	   !cdb.valid()){
		LOG4CXX_ERROR(logger, "Invalid READ CAPACITY request");
		return false;
	}
	/** Check Final flag */
	if(!(req.isFinal())){
		LOG4CXX_WARN(logger, "Initiator MUST set final bit");
 		return false;
	}

	/** generate DataIn */
 	DataInGenerator reply;
	/** always sense error */
	reply.setCmdStatus(CHECK_CONDITION);
	reply.setITT(req.getITT());
	/** Target Transfer Tag is not supplied 
	    @todo : support DataACK & SNACK */
	reply.setTTT(ISCSI_RSVD_TASK_TAG);
	/** update STATSN */
	iSCSIConnectionPtr conn = parent->connection.find(fd);
	reply.setSTATSN(conn->getSTATSN());
	conn->advanceSTATSN();
	reply.setEXPCMDSN(parent->getEXPCMDSN());
	reply.setMAXCMDSN(parent->getMAXCMDSN());

	SenseErrorData data(reinterpret_cast<SenseErrorHeader*>
			    (reply.reserveDataSegment(SenseErrorData::SIZE)));

	/** set Residual Count */
	reply.setResidualCount(req.getEXPDATALEN(),reply.getDlength());

	/** convert reply -> send task */
	parent->sendtask.push(SendTaskPtr(new SendTask(parent, fd, reply.serialize())));
	return true;
}

