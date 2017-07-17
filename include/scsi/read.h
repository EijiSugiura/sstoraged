/**
   @file read.h
   @brief SCSI Read handler
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: read.h 160 2007-06-30 09:39:50Z sugiura $
 */

#ifndef __SCSI_READ_H__
#define __SCSI_READ_H__

#include <boost/shared_ptr.hpp>
#include "common.h"
#include "volume.h"
#include "scsi/generic.h"

using namespace std;

class iSCSISession;
class iSCSIInitiator;
class WriteSegment;
class ReadHandler : public SCSIHandler {
public:
	ReadHandler(iSCSISession *parent);
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
	vector<boost::shared_ptr<WriteSegment> >
		getHitSegments(const vector<boost::shared_ptr<WriteSegment> > &cache,
			       const uint64_t lba,
			       const uint32_t transfer_length);
	bool sendCache(const iSCSISessionPtr parent, const socket_t fd,
		       const LogicalVolumePtr lv,
		       const vector<boost::shared_ptr<WriteSegment> > &hits,
		       uint64_t &rlba, uint32_t length);
private:
	boost::shared_ptr<iSCSIInitiator> initiator;
	uint32_t byte_mrds;
	uint32_t transfer_mrds;
};

/** Read(10) Command */
struct Read10Command {
	/** Operation code */
	uint8_t opcode;
	/** RDPROTECT[3]|DPO|FUA|.|FUA_NV|. */
	uint8_t flags;
	/** LBA */
	uint32_t lba;
	/** Group Number */
	uint8_t group_number;
	/** Transfer Length */
	uint16_t transfer_length;
	/** Control */
	uint8_t control;
} __attribute__ ((packed));

class Read10Parser : public SCSIParser<Read10Command>{
public:
	/** Constructor */
 	Read10Parser(Read10Command *ptr)
		: SCSIParser<Read10Command>(ptr) {}
	/** the validator */
	bool valid() const;

	/** RDPROTECT getter */
	uint8_t getRDPROTECT() const { return (cdb->flags & MASK_RDPROTECT); }
	/** DPO checker */
	bool isDPO() const { return (cdb->flags & FLAG_DPO); }
	/** FUA checker */
	bool isFUA() const { return (cdb->flags & FLAG_FUA); }
	/** FUA_NV checker */
	bool isFUANV() const { return (cdb->flags & FLAG_FUA_NV); }
	/** LBA getter */
	uint32_t getLBA() const { return ntohl(cdb->lba); }
	/** Group Number getter */
	uint8_t getGroupNumber() const { return cdb->group_number; }
	/** Transfer Length getter */
	uint16_t getTransferLength() const { return ntohs(cdb->transfer_length); }
private:
	static const uint8_t MASK_RDPROTECT = 0xE0;
	static const uint8_t FLAG_DPO = 0x10;
	static const uint8_t FLAG_FUA = 0x08;
	static const uint8_t FLAG_FUA_NV = 0x02;
};

#endif /* __SCSI_READ_H__ */
