/**
   @file inquiry.cpp
   @brief SCSI Inquiry handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: inquiry.cpp 312 2007-09-28 00:56:17Z sugiura $
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
#include "iscsi/protocol.h"
#include "scsi/generic.h"
#include "scsi/inquiry.h"

InquiryHandler::InquiryHandler()
{}

bool InquiryParser::valid() const
{
	/** Check OpCode */
	if(getOpCode() != INQUIRY){
		LOG4CXX_ERROR(logger, "Invalid CDB OpCode");
		return false;
	}
	/** Check Allocation Length */
	if(getAllocationLength() < 16){
		LOG4CXX_ERROR(logger, "Invalid CDB Allocation Length");
		return false;		
	}
	/** @todo : Check SN */

	return true;
}

bool InquiryHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	LOG4CXX_DEBUG(logger, "INQUIRY");
	CommandParser req(reinterpret_cast<CommandReqHeader*>(parent->frontToken()));
	InquiryParser cdb(reinterpret_cast<InquiryCommand*>(req.getSCB()));

	if(!(req.getFlags() & COMMAND_FLAG_READ) ||
	   req.getFlags() & COMMAND_FLAG_WRITE ||
	   !cdb.valid()){
		LOG4CXX_ERROR(logger, "Invalid INQUIRY request");
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

	if(!cdb.isEVPD()){
		LOG4CXX_DEBUG(logger, "Standard Data");
		if(cdb.getAllocationLength() < InquiryData::SIZE){
			/** @todo : suport partial reply */
			LOG4CXX_ERROR(logger, "Illegal Allocation Length for Inquiry Standard Data");
			return false;
		}
		InquiryData data(reinterpret_cast<InquiryDataHeader*>
				 (reply.reserveDataSegment(InquiryData::SIZE)));
	} else {
		switch(cdb.getPageCode()){
		case 0x00: {
			LOG4CXX_DEBUG(logger, "SupportedVPD");
			if(cdb.getAllocationLength() < SupportedVPDData::SIZE){
				LOG4CXX_ERROR(logger, "Illegal Allocation Length for SupportedVPD");
				return false;
			}
			SupportedVPDData data(reinterpret_cast<SupportedVPDPage*>
					      (reply.reserveDataSegment(SupportedVPDData::SIZE)));
			break;
		}
		case 0x80: {
			LOG4CXX_DEBUG(logger, "ProductSerial");
			if(cdb.getAllocationLength() < ProductSerialData::SIZE){
				LOG4CXX_ERROR(logger, "Illegal Allocation Length for ProductSerial");
				return false;
			}
			ProductSerialData data(reinterpret_cast<ProductSerialPage*>
					       (reply.reserveDataSegment(ProductSerialData::SIZE)));
			break;
		}
		case 0x83: {
			LOG4CXX_DEBUG(logger, "DeviceID");
			if(cdb.getAllocationLength() < DeviceIDData::SIZE){
				LOG4CXX_ERROR(logger, "Illegal Allocation Length for DeviceID");
				return false;
			}
			DeviceIDData data(reinterpret_cast<DeviceIDPage*>
					 (reply.reserveDataSegment(DeviceIDData::SIZE)));
			break;
		}
		default: {
			ostringstream os;
			os << hex << cdb.getPageCode();
			LOG4CXX_ERROR(logger, "Not supported VPD : " + os.str());
			return false;
		}
		}
	}

	/** set Residual Count */
	reply.setResidualCount(req.getEXPDATALEN(),reply.getDlength());

	/** convert reply -> send task */
	parent->sendtask.push(SendTaskPtr(new SendTask(parent, fd, reply.serialize())));
	return true;
}

