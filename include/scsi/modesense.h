/**
   @file modesense.h
   @brief SCSI ModeSense handler
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: modesense.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __SCSI_MODESENSE_H__
#define __SCSI_MODESENSE_H__

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <boost/shared_ptr.hpp>
#include <map>
#include "common.h"
#include "volume.h"
#include "scsi/generic.h"

using namespace std;

class iSCSISession;
class ModeSenseHandler : public SCSIHandler {
public:
	ModeSenseHandler();
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
private:
};

/** ModeSense Request Header */
struct ModeSenseHeader {
	/** Operation code */
	uint8_t opcode;
	/** Flags ....|DBD|... */
	uint8_t flags;
	/** PC|Page Code */
	uint8_t pagecode;
	/** subpagecode */
	uint8_t subpagecode;
	/** Allocation Length */
	uint8_t allocation_length;
	/** Control */
	uint8_t control;
} __attribute__ ((packed));

class ModeSenseParser : public SCSIParser<ModeSenseHeader>{
public:
	/** Constructor */
 	ModeSenseParser(ModeSenseHeader *ptr) : SCSIParser<ModeSenseHeader>(ptr) {}
	/** the validator */
	bool valid() const;

	/** DBD getter */
	bool isDBD() const { return (cdb->flags & FLAG_DBD); }
	/** Page Code getter */
	uint8_t getPageCode() const { return (cdb->pagecode & PAGECODE_MASK); }
	/** Sub Page Code getter */
	uint8_t getSubPageCode() const { return cdb->subpagecode; }
	/** Allocation Length getter */
	uint8_t getAllocationLength() const { return cdb->allocation_length; }
private:
	static const uint8_t FLAG_DBD = 0x08;
	static const uint8_t PAGECODE_MASK = 0x3F;
};

struct ModeParamHeader {
	uint8_t mode_data_length;
	uint8_t medium_type;
	uint8_t rsvd;
	uint8_t block_descriptor_length;
	ModeParamHeader()
		: mode_data_length(0),
		  medium_type(0), rsvd(0),
		  block_descriptor_length(0) {}
} __attribute__ ((packed));

struct ShortLBABlockDescriptor {
	uint32_t number_of_lba;
	uint32_t lba_unit;
	ShortLBABlockDescriptor()
		: number_of_lba(0),
		  lba_unit(htonl(SCSI::LBAUNIT)) {}
	void setNumberOfLBA(const uint32_t num)
	{ number_of_lba = htonl(num); }
};

class ModeSenseData : public DataSegment<ModeParamHeader> {
public:
	ModeSenseData(ModeParamHeader *ptr, const size_t size)
		: DataSegment<ModeParamHeader>(ptr, size), total(this->HEADER_SIZE)
	{}
	~ModeSenseData() {}

	bool appendBlockDescriptor(const LogicalVolumePtr lv);
	bool appendDisconnectReconnect();
	bool appendFormatDevice();
	bool appendRigidDiskGeometry();
	bool appendCaching();
	bool appendControl();
	bool appendInformationalExceptionsCtrl();
	size_t getLength() const { return total; }
	void setDesiredLength(const uint8_t size)
	{ data->mode_data_length = size; }
	bool setBlockDescriptorLength(const size_t size)
	{
		/** anyway update length, first */
		data->block_descriptor_length = static_cast<uint8_t>(size);
		if(this->getLimit() < size ||
		   0xFF < size) // for only 1byte length field
			return false;
		return true;
	}
	static const size_t HEADER_SIZE	= sizeof(ModeParamHeader);
protected:
	size_t total;
	bool setLength(const size_t size)
	{
		/** subtract 1 byte for offset of length field,
		    anyway update length */
		data->mode_data_length = static_cast<uint8_t>(size - 1);
		if(this->getLimit() < size ||
		   0x100 < size) // for only 1byte length field
			return false;
		return true;
	}
};

struct LongLBABlockDescriptor {
	uint64_t number_of_lba;
	uint32_t rsvd;
	uint32_t logical_block_length;
	LongLBABlockDescriptor()
		: number_of_lba(0),
		  rsvd(0),
		  logical_block_length(0) {}
};


/** @todo : support ModeSense(16) */

#endif /* __SCSI_MODESENSE_H__ */
