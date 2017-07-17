/**
   @file dataout.cpp
   @brief iSCSI DataOut handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: dataout.cpp 164 2007-07-03 05:36:07Z sugiura $
 */

#include <iomanip>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include "configurator.h"
#include "logger.h"
#include "session.h"
#include "writecache.h"
#include "diskwriter.h"
#include "iscsi/dataout.h"

DataOutHandler::DataOutHandler(iSCSISession *parent)
{
	initiator = parent->initiator.get();
	mrds = initiator->getMaxRecvDataSegmentLength();
}

bool DataOutParser::valid() const
{
	/** Check OpCode */
	if(getOpCode() != iSCSI_OP_SCSI_DATA){
		LOG4CXX_ERROR(logger, "Invalid OpCode");
		return false;
	}
	/** @todo : Check SN */

	return true;
}

bool DataOutHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	DataOutHeader *bhs = reinterpret_cast<DataOutHeader*>(parent->frontToken());
	DataOutParser req(bhs);
	if(!req.valid()){
		LOG4CXX_ERROR(logger, "Invalid DataOut request");
		return false;
	}

	/** Get DataOut receive task */
	uint32_t itt = req.getITT();
	DataOutInfoPtr dinfo = parent->recvinfo.get(itt);
	if(dinfo.get() == NULL){
		LOG4CXX_ERROR(logger, "No such DataOut info : " +
			      boost::lexical_cast<string>(itt));
		return false;
	}
	uint64_t lba = dinfo->getLBA();
	uint32_t remain = dinfo->getRemain();
	uint32_t length = req.getDlength();
	LOG4CXX_DEBUG(logger, "DataOut : itt " +
		      boost::lexical_cast<string>(ntohl(itt)) + " length " +
		      boost::lexical_cast<string>(length) + " remain " +
		      boost::lexical_cast<string>(remain));
	WriteSegmentPtr ws = parent->frontDataSegment(lba+SCSI::OFFSET2LBA(dinfo->getTotal()-remain),
						      SCSI::OFFSET2LBA(length));
	if(ws.get() != NULL){
		uint32_t lun_id = req.getLUN().get();
		if(parent->wcaches.size() <= lun_id){
			LOG4CXX_ERROR(logger, "Invalid LUN");
			return false;
		}
		/** store into WriteCache */
		parent->wcaches[lun_id]->push(ws);
		/** notify to DiskWriter */
		parent->wcaches[lun_id]->getWriter()->notifyEvent();
	}

	/** Data Transfer complete? */
	if(remain < length){
		LOG4CXX_ERROR(logger, "Too large DataSegment length : " +
			      boost::lexical_cast<string>(remain) + "<" +
			      boost::lexical_cast<string>(length));
		return false;
	}
	remain -= length;
	/** Update receive info */
	dinfo->setRemain(remain);
	/** DataOut continue? */
	if(!req.isFinal())
		return true;

	/** Data Transfer finished? */
	if(remain == 0){
		/** generate SCSI Rsp */
		SCSIRspGenerator reply;
		reply.setFlags(reply.ISCSI_FLAG_FINAL);
		reply.setCmdStatus(GOOD);
		reply.setITT(itt);
		/** update STATSN */
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
		/** Release receive info */
		parent->recvinfo.pop(itt);
		return true;
	}
	/** generate next R2T */
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
	r2t.setOffset(dinfo->getTotal()-remain);
	r2t.setDesiredDataTransferLength(min(remain,mrds));
	r2t.setR2TSN(dinfo->advanceR2TSN());
	parent->sendtask.push(SendTaskPtr(new SendTask(parent, fd,
						       r2t.serialize())));
	return true;
}
