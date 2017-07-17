/**
   @file write.cpp
   @brief SCSI Write handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: write.cpp 312 2007-09-28 00:56:17Z sugiura $
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
#include "scsi/write.h"
#include "diskwriter.h"
#include "writecache.h"

WriteHandler::WriteHandler(iSCSISession *parent)
{
	initiator = parent->initiator.get();
	if(initiator.get() == NULL){
		LOG4CXX_ERROR(logger,"Internal error : failed to initialize WriteHandler");
		throw runtime_error("Internal error : failed to initialize WriteHandler");
	}
	mrds = initiator->getMaxRecvDataSegmentLength();
	parent->getAttr("FirstBurstLength", firstBurstLength);
	parent->getAttr("MaxBurstLength", maxBurstLength);
	parent->getAttr("ImmediateData", immediateEnabled);
	parent->getAttr("InitialR2T", initialR2T);
}

bool Write10Parser::valid() const
{
	if(getOpCode() != WRITE_10){
		LOG4CXX_ERROR(logger, "Invalid CDB OpCode");
		return false;
	}
	if(getWRPROTECT()){
		LOG4CXX_ERROR(logger, "WRPROTECT not supported yet");
		return false;		
	}
	if(isDPO()){
		LOG4CXX_ERROR(logger, "DPO not supported yet");
		return false;		
	}
	/** @todo :  support write back mode */
	if(isFUA()){
		LOG4CXX_WARN(logger, "FUA : always Force Unit Access");
	}
	if(isFUANV()){
		LOG4CXX_WARN(logger, "FUA_NV : always Force Unit Access");
	}
	/** @todo : Check SN */

	return true;
}

bool WriteHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	CommandParser req(reinterpret_cast<CommandReqHeader*>(parent->frontToken()));
	Write10Parser cdb(reinterpret_cast<Write10Command*>(req.getSCB()));
	uint32_t itt = req.getITT();
	LOG4CXX_DEBUG(logger, "WRITE(10) : itt " + boost::lexical_cast<string>(ntohl(itt)) +
		      " " + boost::lexical_cast<string>(cdb.getLBA()) +
		      " + " + boost::lexical_cast<string>(cdb.getTransferLength()));

	if(!(req.getFlags() & COMMAND_FLAG_WRITE) ||
	   req.getFlags() & COMMAND_FLAG_READ ||
	   !cdb.valid()){
		LOG4CXX_ERROR(logger, "Invalid WRITE(10) request");
		return false;
	}
	/** Check MaxBurstLength */
	if(SCSI::LBA2OFFSET(cdb.getTransferLength()) > maxBurstLength){
		LOG4CXX_ERROR(logger, "Exceed MaxBurstLength");
		return false;
	}

	/** get Volume info. and check LBA range */
	LogicalVolumePtr lv = initiator->getLUN(req.getLUN().get());
	if(lv.get() == NULL){
		LOG4CXX_ERROR(logger, "Invalid LUN : " +
			      boost::lexical_cast<string>(req.getLUN().get()));
		return false;
	}
	uint64_t lba = cdb.getLBA();
	uint32_t transfer_length = cdb.getTransferLength();
	uint32_t remain = SCSI::LBA2OFFSET(transfer_length);
	if(lba >= lv->getTotalSize() || transfer_length == 0 ||
	   lba + transfer_length > lv->getTotalSize()){
		LOG4CXX_ERROR(logger, "Too large LBA : " +
			      boost::lexical_cast<string>(lba) + "+" +
			      boost::lexical_cast<string>(transfer_length) + " : " +
			      boost::lexical_cast<string>(lv->getTotalSize()));
		return false;
	}

	/** Immediate data */
	if(immediateEnabled){
		WriteSegmentPtr ws = parent->frontDataSegment(lba,SCSI::OFFSET2LBA(req.getDlength()));
		if(ws.get() == NULL){
			LOG4CXX_ERROR(logger, "Immediate Write segment vanished");
			return false;
		}
		uint32_t lun_id = req.getLUN().get();
		if(parent->wcaches.size() <= lun_id){
			LOG4CXX_ERROR(logger, "Invalid LUN");
			return false;
		}
		/** store into WriteCache */
		parent->wcaches[lun_id]->push(ws);
		/** notify to DiskWriter */
		parent->wcaches[lun_id]->getWriter()->notifyEvent();
		remain -= req.getDlength();
		LOG4CXX_DEBUG(logger, "Immediate Write : " +
			      boost::lexical_cast<string>(lba) + " length " +
			      boost::lexical_cast<string>(req.getDlength()) + " remain " +
			      boost::lexical_cast<string>(remain));
		/** Data Transfer complete? */
		if(remain == 0){
			if(!(req.isFinal())){
				LOG4CXX_ERROR(logger, "Absense of FINAL bit");
				return false;
			}
			/** generate SCSI Rsp */
			SCSIRspGenerator reply;
			reply.setFlags(reply.ISCSI_FLAG_FINAL);
			reply.setCmdStatus(GOOD);
			reply.setITT(itt);
			// update STATSN
			iSCSIConnectionPtr conn = parent->connection.find(fd);
			reply.setSTATSN(conn->getSTATSN());
			conn->advanceSTATSN();
			reply.setEXPCMDSN(parent->getEXPCMDSN());
			reply.setMAXCMDSN(parent->getMAXCMDSN());
			/** set Residual Count
			    @todo : check omit this
			 */
			// reply.setResidualCount(req.getEXPDATALEN(),reply.getDlength());

			parent->sendtask.push(SendTaskPtr(new SendTask(parent, fd,
								       reply.serialize())));
			return true;
		}
	}

	/** R2T response */
	DataOutInfoPtr dinfo = DataOutInfoPtr(new DataOutInfo(lba,
							      SCSI::LBA2OFFSET(transfer_length),
							      remain));
	if(initialR2T || req.getDlength() >= firstBurstLength){
		/** generate R2T */
		R2TGenerator r2t;
		r2t.setITT(itt);
		/** We use same value for convenience, itt == ttt */
		r2t.setTTT(itt);
		r2t.setLUN(req.getLUN());
		/** NOT update STATSN */
		iSCSIConnectionPtr conn = parent->connection.find(fd);
		r2t.setSTATSN(conn->getSTATSN());
		r2t.setEXPCMDSN(parent->getEXPCMDSN());
		r2t.setMAXCMDSN(parent->getMAXCMDSN());
		r2t.setOffset(SCSI::LBA2OFFSET(transfer_length) - remain);
		r2t.setDesiredDataTransferLength(min(remain,mrds));
		r2t.setR2TSN(dinfo->advanceR2TSN());
		parent->sendtask.push(SendTaskPtr(new SendTask(parent, fd,
							       r2t.serialize())));
	}
	/** Store DataOut receive task */
	parent->recvinfo.put(itt, dinfo);
	return true;
}

