/**
   @file readcapacity16.h
   @brief SCSI ReadCapacity16 handler
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: readcapacity16.h 228 2007-07-23 04:42:42Z sugiura $
 */

#ifndef __SCSI_READCAPACITY16_H__
#define __SCSI_READCAPACITY16_H__

#include <boost/shared_ptr.hpp>
#include "common.h"
#include "scsi/generic.h"

using namespace std;

class iSCSISession;
class ReadCapacity16Handler : public SCSIHandler {
public:
	ReadCapacity16Handler();
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
private:
};

/** ReadCapacity16 Command */
struct ReadCapacity16Command {
	/** Operation code */
	uint8_t opcode;
	/** ...|Service Action In 16 */
	uint8_t service_action;
	/** LBA64 */
	uint64_t lba;
	/** Allocation Length */
	uint64_t allocation_length;
	/** .......|PMI */
	uint8_t  flags;
	/** Control */
	uint8_t control;
} __attribute__ ((packed));

class ReadCapacity16Parser : public SCSIParser<ReadCapacity16Command>{
public:
	/** Constructor */
 	ReadCapacity16Parser(ReadCapacity16Command *ptr)
		: SCSIParser<ReadCapacity16Command>(ptr) {}
	/** the validator */
	bool valid() const;

	/** check Service Action In 16 */
	bool isReadCapacity16() const { return (cdb->service_action == READCAPACITY16); }
	/** check EVPD flag */
	bool isPMI() const { return (cdb->flags & FLAG_PMI); }
	/** LBA getter */
	uint64_t getLBA() const { return be64toh(cdb->lba); }
private:
	static const uint8_t FLAG_PMI = 0x01;
	static const uint8_t READCAPACITY16 = 0x10;
	static const uint8_t ALLOCATION_LENGTH = 32;
};

struct ReadCapacity16Header {
	/** Logical Block Address */
	uint64_t lba;
	/** Logical Block Length in Bytes */
	uint32_t lba_unit;
	/** ......|RTO_EN|PROT_EN */
	uint8_t flags;
	uint8_t rsvd1,rsvd2,rsvd3;
	uint64_t rsvd4,rsvd5;
	ReadCapacity16Header()
		: lba(0), lba_unit(htonl(SCSI::LBAUNIT)) {}
};

class ReadCapacity16Data : public DataSegment<ReadCapacity16Header> {
public:
	ReadCapacity16Data(ReadCapacity16Header* ptr)
		: DataSegment<ReadCapacity16Header>(ptr,this->SIZE)
	{}
	~ReadCapacity16Data() {}
	void setLBA(const uint64_t lba) { data->lba = htobe64(lba); }
	static const size_t SIZE = sizeof(ReadCapacity16Header);
private:
	static const uint8_t FLAG_RTO_EN = 0x02;
	static const uint8_t FLAG_PROT_EN = 0x01;
};

#endif /* __SCSI_READCAPACITY16_H__ */
