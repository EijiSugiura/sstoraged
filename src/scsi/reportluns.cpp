/**
   @file reportluns.cpp
   @brief SCSI ReportLUNS handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: reportluns.cpp 312 2007-09-28 00:56:17Z sugiura $
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
#include "scsi/reportluns.h"


ReportLUNSHandler::ReportLUNSHandler()
{}

bool ReportLUNSParser::valid() const
{
	/** Check OpCode */
	if(getOpCode() != REPORT_LUNS){
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

bool ReportLUNSHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	LOG4CXX_DEBUG(logger, "REPORT LUNS");
	CommandParser req(reinterpret_cast<CommandReqHeader*>(parent->frontToken()));
	ReportLUNSParser cdb(reinterpret_cast<ReportLUNSHeader*>(req.getSCB()));

	if(!(req.getFlags() & COMMAND_FLAG_READ) ||
	   req.getFlags() & COMMAND_FLAG_WRITE ||
	   !cdb.valid()){
		LOG4CXX_ERROR(logger, "Invalid REPORT LUNS request");
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

	/** create LUN lists */
	ReportLUNSData lun_list(reinterpret_cast<ReportLUNSheader*>
				(reply.reserveDataSegment(cdb.getAllocationLength())),
				cdb.getAllocationLength());

	iSCSIInitiatorPtr initiator = parent->initiator.get();
	vector<string> luns = initiator->getLUNnames();
	if(luns.empty()){
		LOG4CXX_ERROR(logger, "\"" + initiator->getName() + "\" has NO LUN");
		return false;
	}
	uint32_t id = 0;
	for(vector<string>::iterator lun = luns.begin();
	    lun != luns.end(); ++lun, ++id){
		if(!lun_list.add(id)){
			LOG4CXX_WARN(logger, "Too small data segment for REPORT LUNS");
		}
	}

	/** convert reply -> send task */
	parent->sendtask.push(SendTaskPtr(new SendTask(parent, fd, reply.serialize())));
	return true;
}

bool ReportLUNSData::add(uint32_t id)
{
	/** update LUN list length */
	uint32_t length = ntohl(data->lun_list_length);
	length += LUN::SIZE;

	/** anyway update length */
	data->lun_list_length = htonl(length);

	/** header + LUN list */
	if(this->getLimit() < sizeof(ReportLUNSheader) + length)
		return false;
	/** add LUN */
	LUN lun(id);
	memcpy(reinterpret_cast<uint8_t*>(data) + length,
	       lun.lun, lun.SIZE);
	return true;
}
