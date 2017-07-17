/**
   @file read.cpp
   @brief SCSI Read handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: read.cpp 312 2007-09-28 00:56:17Z sugiura $
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
#include "writecache.h"
#include "iscsi/protocol.h"
#include "scsi/generic.h"
#include "scsi/read.h"

#ifdef LOG4CXX_COUT
#undef LOG4CXX_DEBUG
#define LOG4CXX_DEBUG(logger,message)	cout << (message) << endl
#undef LOG4CXX_ERROR
#define LOG4CXX_ERROR(logger,message)	cerr << (message) << endl
#undef LOG4CXX_WARN
#define LOG4CXX_WARN(logger,message)	cerr << (message) << endl
#endif

ReadHandler::ReadHandler(iSCSISession *parent)
	: byte_mrds(0), transfer_mrds(0)
{
	if(parent){
		initiator = parent->initiator.get();
		byte_mrds = initiator->getMaxRecvDataSegmentLength();
		transfer_mrds = SCSI::OFFSET2LBA(byte_mrds);
	}
}

bool Read10Parser::valid() const
{
	if(getOpCode() != READ_10){
		LOG4CXX_ERROR(logger, "Invalid CDB OpCode");
		return false;
	}
	/** Read Protect */
	if(getRDPROTECT()){
		LOG4CXX_ERROR(logger, "RDPROTECT not supported yet");
		return false;		
	}
	/** @todo : support DPO */
	if(isDPO()){
		LOG4CXX_ERROR(logger, "DPO not supported yet");
		return false;		
	}
	/** FUA */
	if(isFUA()){
		LOG4CXX_WARN(logger, "FUA : always Force Unit Access");
	}
	/** FUANV */
	if(isFUANV()){
		LOG4CXX_WARN(logger, "FUA_NV : always Force Unit Access");
	}
	/** @todo : Check SN */

	return true;
}

bool ReadHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	CommandParser req(reinterpret_cast<CommandReqHeader*>(parent->frontToken()));
	Read10Parser cdb(reinterpret_cast<Read10Command*>(req.getSCB()));

	LOG4CXX_DEBUG(logger, "READ(10) " + boost::lexical_cast<string>(cdb.getLBA()) +
		      " + " + boost::lexical_cast<string>(cdb.getTransferLength()));

	if(!(req.getFlags() & COMMAND_FLAG_READ) ||
	   req.getFlags() & COMMAND_FLAG_WRITE ||
	   !cdb.valid()){
		LOG4CXX_ERROR(logger, "Invalid READ(10) request");
		return false;
	}
	/** Check Final flag */
	if(!(req.isFinal())){
		LOG4CXX_WARN(logger, "Initiator MUST set final bit");
 		return false;
	}

	/** get Volume info. and check LBA range */
	uint32_t lun_id = req.getLUN().get();
	LogicalVolumePtr lv = initiator->getLUN(lun_id);
	if(lv.get() == NULL){
		LOG4CXX_ERROR(logger, "Invalid LUN : " +
			      boost::lexical_cast<string>(lun_id));
		return false;
	}
	uint64_t lba = cdb.getLBA();
	uint32_t transfer_length = cdb.getTransferLength();
	if(lba >= lv->getTotalSize() || transfer_length == 0 ||
	   lba + transfer_length > lv->getTotalSize()){
		LOG4CXX_ERROR(logger, "Too large LBA : " +
			      boost::lexical_cast<string>(lba) + "+" +
			      boost::lexical_cast<string>(transfer_length) + " : " +
			      boost::lexical_cast<string>(lv->getTotalSize()));
		return false;
	}

	/** generate DataIn */
 	DataInGenerator reply;
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

	/** no need to set Residual Count, assume GOOD DataIn response */

	/** search Write Cache */
	vector<WriteSegmentPtr> cache = parent->wcaches[lun_id]->search(lba,transfer_length);
	parent->sendtask.push(TCPCorkTaskPtr(new TCPCorkTask(fd)));
	if(cache.empty()){
		LOG4CXX_DEBUG(logger, "CacheMiss : " +
			      boost::lexical_cast<string>(lba) + "+" +
			      boost::lexical_cast<string>(transfer_length));
		/** case :  CacheMiss ->
		    Header(Send) + DataIn(Sendfile) */
		while(1){
			uint32_t length = transfer_length;
			if(length > transfer_mrds){
				length = transfer_mrds;
				reply.setDlength(byte_mrds);
			} else {
				reply.setCmdStatus(GOOD);
				reply.setDlength(SCSI::LBA2OFFSET(transfer_length));
				transfer_length = 0;
			}
			LOG4CXX_DEBUG(logger, "DataIn : " + boost::lexical_cast<string>(reply.getDlength()));
			/** DataIn header */
			parent->sendtask.push(SendTaskPtr(new SendTask(parent, fd,
								       reply.serialize())));
			/** Data Segment */
			parent->sendtask.push(SendfileTaskPtr(new SendfileTask(parent, fd,
									       lv, lba,
									       length)));
			if(transfer_length == 0)
				break;
			// Update DataIn header
			DataInGenerator replica(reply);
			reply = replica;
			reply.advanceDataSN();
			reply.addOffset(SCSI::LBA2OFFSET(length));
			transfer_length -= length;
			/** update LBA */
			lba += length;
		}
	} else {
		LOG4CXX_DEBUG(logger, "CacheHit : " +
			      boost::lexical_cast<string>(lba) + "+" +
			      boost::lexical_cast<string>(transfer_length));
		/** case : CacheHit
		    Header(Send) + DataIn(Sendfile or SendCache) */
		while(1){
			uint32_t length = transfer_length;
			if(length > transfer_mrds){
				length = transfer_mrds;
				reply.setDlength(byte_mrds);
			} else {
				reply.setCmdStatus(GOOD);
				reply.setDlength(SCSI::LBA2OFFSET(transfer_length));
				transfer_length = 0;
			}
			LOG4CXX_DEBUG(logger, "DataIn : " + boost::lexical_cast<string>(reply.getDlength()));

			/** DataIn header */
			parent->sendtask.push(SendTaskPtr(new SendTask(parent, fd,
								       reply.serialize())));
			vector<WriteSegmentPtr> hits = getHitSegments(cache, lba, length);
			if(hits.empty()){
				LOG4CXX_ERROR(logger, "CacheHit but no hits!");
				return false;
			}
			/** DataSegment -- update LBA */
			sendCache(parent, fd, lv, hits, lba, length);
			if(transfer_length == 0)
				break;
			// Update DataIn header
			DataInGenerator replica(reply);
			reply = replica;
			reply.advanceDataSN();
			reply.addOffset(SCSI::LBA2OFFSET(length));
			transfer_length -= length;
		}
	}
	parent->sendtask.push(TCPUncorkTaskPtr(new TCPUncorkTask(fd)));
	return true;
}

vector<WriteSegmentPtr> ReadHandler::getHitSegments(const vector<WriteSegmentPtr> &cache,
					       const uint64_t lba,
					       const uint32_t transfer_length)
{
	vector<WriteSegmentPtr> hits;
	vector<WriteSegmentPtr>::const_iterator end = cache.end();
	for(vector<WriteSegmentPtr>::const_iterator itr = cache.begin();
	    itr != end; ++itr){
		WriteSegmentPtr ws = *itr;
		if((lba <= ws->rlba && ws->rlba <= lba + transfer_length) ||
		   (ws->rlba < lba && lba + transfer_length < ws->rlba + ws->rlength) ||
		   (ws->rlba < lba && lba < ws->rlba + ws->rlength)){
			hits.push_back(ws);
		}
	}
	return hits;
}

bool ReadHandler::sendCache(const iSCSISessionPtr parent, const socket_t fd,
			    const LogicalVolumePtr lv, const vector<WriteSegmentPtr> &hits,
			    uint64_t &rlba, uint32_t transfer_length)
{
	for(vector<WriteSegmentPtr>::const_iterator itr = hits.begin();
	    itr != hits.end() && transfer_length > 0; ++itr){
		uint64_t clba = (*itr)->rlba;
		uint32_t clength = (*itr)->rlength;
		if(clba <= rlba){
			LOG4CXX_DEBUG(logger, "case1 : "
				      + boost::lexical_cast<string>(clba) + "<="
				      + boost::lexical_cast<string>(rlba));
			uint32_t hit_length = clength - (rlba - clba);
			if(rlba + transfer_length < clba + clength)
				hit_length -= clba + clength - rlba - transfer_length;
			/** The 1st segment */
			parent->sendtask.push(SendCacheTaskPtr(new SendCacheTask(parent, fd,
										 *itr, rlba,
										 hit_length)));
			transfer_length -= hit_length;
			rlba += hit_length;
		} else if(rlba + transfer_length <= clba + clength) {
			uint32_t miss_length = clba - rlba;
			LOG4CXX_DEBUG(logger, "case2 : "
				      + boost::lexical_cast<string>(rlba) + "+"
				      + boost::lexical_cast<string>(transfer_length) + "<="
				      + boost::lexical_cast<string>(clba) + "+"
				      + boost::lexical_cast<string>(clength));
			parent->sendtask.push(SendfileTaskPtr(new SendfileTask(parent, fd,
									       lv, rlba,
									       miss_length)));
			/** The last hit segment */
			parent->sendtask.push(SendCacheTaskPtr(new SendCacheTask(parent, fd,
										 *itr, clba,
										 transfer_length - miss_length)));
			transfer_length = 0;
			rlba = clba + clength - miss_length;
			break;
		} else {
			LOG4CXX_DEBUG(logger, "case3 : ");
			uint32_t miss_length = clba-rlba;
			parent->sendtask.push(SendfileTaskPtr(new SendfileTask(parent, fd,
									       lv, rlba,
									       miss_length)));
			parent->sendtask.push(SendCacheTaskPtr(new SendCacheTask(parent, fd,
										 *itr, clba,
										 clength)));
			transfer_length -= miss_length + clength;
			rlba = clba + clength;
		}
	}
	if(transfer_length > 0){
		/** The last miss segment */
		parent->sendtask.push(SendfileTaskPtr(new SendfileTask(parent, fd,
								       lv, rlba,
								       transfer_length)));
		rlba += transfer_length;
	}
	return true;
}
