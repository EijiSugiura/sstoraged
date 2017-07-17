/**
   @file readcapacity16.cpp
   @brief SCSI ReadCapacity16 handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: readcapacity16.cpp 312 2007-09-28 00:56:17Z sugiura $
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
#include "scsi/readcapacity16.h"

ReadCapacity16Handler::ReadCapacity16Handler()
{}

bool ReadCapacity16Parser::valid() const
{
	/** Check OpCode */
	if(getOpCode() != SERVICE_ACTION_IN_16){
		LOG4CXX_ERROR(logger, "Invalid CDB OpCode");
		return false;
	}
	if(!isReadCapacity16()){
		LOG4CXX_ERROR(logger, "Invalid Service Action");
	}
	/** @todo : support PMI */
	if(isPMI()){
		LOG4CXX_ERROR(logger, "PMI not supported yet");
		return false;		
	}
	/** @todo : Check SN */

	return true;
}

bool ReadCapacity16Handler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	LOG4CXX_DEBUG(logger, "READ CAPACITY(16)");
	CommandParser req(reinterpret_cast<CommandReqHeader*>(parent->frontToken()));
	ReadCapacity16Parser cdb(reinterpret_cast<ReadCapacity16Command*>(req.getSCB()));

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
	reply.setCmdStatus(GOOD);
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

	/** assume PMI == 0 */
	if(cdb.getLBA() == 0){
		ReadCapacity16Data data(reinterpret_cast<ReadCapacity16Header*>
				      (reply.reserveDataSegment(ReadCapacity16Data::SIZE)));
		/** get Volume info. and set last LBA */
		iSCSIInitiatorPtr initiator = parent->initiator.get();
		LogicalVolumePtr lv = initiator->getLUN(req.getLUN().get());
		if(lv.get() == NULL){
			LOG4CXX_ERROR(logger, "Invalid LUN : " + boost::lexical_cast<string>(req.getLUN().get()));
			return false;
		}
		data.setLBA(lv->getTotalSize() - 1);
	} else {
		LOG4CXX_WARN(logger, "Too small data segment for READ CAPACITY");
		reply.setCmdStatus(CHECK_CONDITION);
		FixedSenseData sense_error(reinterpret_cast<FixedSenseParam*>
					   (reply.reserveDataSegment(FixedSenseData::SIZE)));
	}

	/** set Residual Count */
	reply.setResidualCount(req.getEXPDATALEN(),reply.getDlength());

	/** convert reply -> send task */
	parent->sendtask.push(SendTaskPtr(new SendTask(parent, fd, reply.serialize())));
	return true;
}

