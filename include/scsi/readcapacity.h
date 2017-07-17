/**
   @file readcapacity.h
   @brief SCSI ReadCapacity handler
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: readcapacity.h 228 2007-07-23 04:42:42Z sugiura $
 */

#ifndef __SCSI_READCAPACITY_H__
#define __SCSI_READCAPACITY_H__

#include <boost/shared_ptr.hpp>
#include "common.h"
#include "scsi/generic.h"

using namespace std;

class iSCSISession;
class ReadCapacityHandler : public SCSIHandler {
public:
	ReadCapacityHandler();
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
private:
};

/** ReadCapacity Command */
struct ReadCapacityCommand {
	/** Operation code */
	uint8_t opcode;
	uint8_t rsvd1;
	/** LBA */
	uint32_t lba;
	uint16_t rsvd2;
	/** .......|PMI */
	uint8_t  flags;
	/** Control */
	uint8_t control;
} __attribute__ ((packed));

class ReadCapacityParser : public SCSIParser<ReadCapacityCommand>{
public:
	/** Constructor */
 	ReadCapacityParser(ReadCapacityCommand *ptr)
		: SCSIParser<ReadCapacityCommand>(ptr) {}
	/** the validator */
	bool valid() const;

	/** check EVPD flag */
	bool isPMI() const { return (cdb->flags & FLAG_PMI); }
	/** LBA getter */
	uint32_t getLBA() const { return ntohl(cdb->lba); }
private:
	static const uint8_t FLAG_PMI = 0x01;
};

struct ReadCapacityHeader {
	/** Logical Block Address */
	uint32_t lba;
	/** Logical Block Length in Bytes */
	uint32_t lba_unit;
	ReadCapacityHeader()
		: lba(0), lba_unit(htonl(SCSI::LBAUNIT)) {}
};

class ReadCapacityData : public DataSegment<ReadCapacityHeader> {
public:
	ReadCapacityData(ReadCapacityHeader* ptr)
		: DataSegment<ReadCapacityHeader>(ptr,this->SIZE)
	{}
	~ReadCapacityData() {}
	void setLBA(const uint32_t lba) { data->lba = htonl(lba); }
	static const size_t SIZE = sizeof(ReadCapacityHeader);
private:
};

#endif /* __SCSI_READCAPACITY_H__ */
