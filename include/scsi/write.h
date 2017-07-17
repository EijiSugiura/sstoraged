/**
   @file write.h
   @brief SCSI Write handler
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: write.h 164 2007-07-03 05:36:07Z sugiura $
 */

#ifndef __SCSI_WRITE_H__
#define __SCSI_WRITE_H__

#include <boost/shared_ptr.hpp>
#include "common.h"
#include "volume.h"
#include "scsi/generic.h"

using namespace std;

class iSCSISession;
class iSCSIInitiator;
class WriteCache;
class WriteHandler : public SCSIHandler {
public:
	WriteHandler(iSCSISession *parent);
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
private:
	boost::shared_ptr<iSCSIInitiator> initiator;
	uint32_t mrds;
	uint32_t firstBurstLength;
	uint32_t maxBurstLength;
	bool immediateEnabled;
	bool initialR2T;
	vector<boost::shared_ptr<WriteCache> > wcaches;
};

/** Write(10) Command */
struct Write10Command {
	/** Operation code */
	uint8_t opcode;
	/** WRPROTECT[3]|DPO|FUA|.|FUA_NV|. */
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

class Write10Parser : public SCSIParser<Write10Command>{
public:
	/** Constructor */
 	Write10Parser(Write10Command *ptr)
		: SCSIParser<Write10Command>(ptr) {}
	/** the validator */
	bool valid() const;

	/** RDPROTECT getter */
	uint8_t getWRPROTECT() const { return (cdb->flags & MASK_WRPROTECT); }
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
	/** Transfer Length getter in bytes */
	uint32_t getTransferLength() const { return ntohs(cdb->transfer_length); }
private:
	static const uint8_t MASK_WRPROTECT = 0xE0;
	static const uint8_t FLAG_DPO = 0x10;
	static const uint8_t FLAG_FUA = 0x08;
	static const uint8_t FLAG_FUA_NV = 0x02;
};

#endif /* __SCSI_WRITE_H__ */
