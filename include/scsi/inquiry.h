/**
   @file inquiry.h
   @brief SCSI Inquiry handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: inquiry.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __SCSI_INQUIRY_H__
#define __SCSI_INQUIRY_H__

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <boost/shared_ptr.hpp>
#include "common.h"
#include "scsi/generic.h"

using namespace std;

class iSCSISession;
class InquiryHandler : public SCSIHandler {
public:
	InquiryHandler();
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
private:
};

/** Inquiry Command */
struct InquiryCommand {
	/** Operation code */
	uint8_t opcode;
	/** .......|EVPD */
	uint8_t flags;
	/** Page Code */
	uint8_t pagecode;
	/** Allocation Length */
	uint16_t allocation_length;
	/** Control */
	uint8_t control;
} __attribute__ ((packed));

class InquiryParser : public SCSIParser<InquiryCommand>{
public:
	/** Constructor */
 	InquiryParser(InquiryCommand *ptr)
		: SCSIParser<InquiryCommand>(ptr){}
	/** the validator */
	bool valid() const;

	/** check EVPD flag */
	bool isEVPD() const { return (cdb->flags & FLAG_EVPD); }
	/** Page Code getter */
	uint8_t getPageCode() const { return cdb->pagecode; }
	/** Allocation Length getter */
	uint16_t getAllocationLength() const
	{ return ntohs(cdb->allocation_length); }
private:
	static const uint8_t FLAG_EVPD = 0x01;
};

#define SPC4		(0x06)
#define TRML_OP		(0x40)
#define FORMAT		(0x02)
#define FLAG_CMD_QUEUE	(0x02)
#define VENDOR_ID	"PRIV    "
#define PRODUCT_ID	"sStorage        "
#define PRODUCT_REVISION_LEVEL	"0.1"

struct InquiryDataHeader {
	/** Peripheral Qualifier / Peripheral Device Type */
	uint8_t device_type;
	/** RMB */
	uint8_t rmb;
	/** SCSI Version */
	uint8_t version;
	/** AERC/NormACA/HiSup/Response Data Format*/
	uint8_t response_data_format;
	/** Additional Length */
	uint8_t additional_length;
	/** sccs */
	uint8_t spc_flags2;
	/** BQue/EncServ/VS/MultiP/MChngr///Addr16 */
	uint8_t spc_flags1;
	/** RelAdr//WBus16/Sync/Linked//CmdQue/VS */
	uint8_t flags;
	/** Vendor Identification */
	uint8_t vendor_id[8];
	/** Product Identification */
	uint8_t product_id[16];
	/** Product Revision Level */
	uint8_t product_revision_level[4];
	InquiryDataHeader()
		: device_type(0), rmb(0), version(SPC4), 
		  response_data_format(TRML_OP|FORMAT),
		  additional_length(0),
		  spc_flags2(0), spc_flags1(0),
		  flags(FLAG_CMD_QUEUE)
	{
		memcpy(vendor_id, VENDOR_ID, sizeof(vendor_id));
		memcpy(product_id, PRODUCT_ID, sizeof(product_id));
		memcpy(product_revision_level, PRODUCT_REVISION_LEVEL,
		       sizeof(product_revision_level));
	}
};
/** @todo : treat full size of InquiryData? */

class InquiryData : public DataSegment<InquiryDataHeader> {
public:
	InquiryData(InquiryDataHeader* ptr)
		: DataSegment<InquiryDataHeader>(ptr, this->SIZE)
	{
		data->additional_length = SIZE - 4;
	}
	~InquiryData() {}
	static const size_t SIZE = sizeof(InquiryDataHeader);
};

struct SupportedVPDPage {
	uint8_t peripheral;
	uint8_t pagecode;
	uint8_t rsvd;
	uint8_t pagelength;
	uint8_t supportedvpd[3];
	SupportedVPDPage()
		: peripheral(0), pagecode(0x00), rsvd(0),
		  pagelength(0)
	{
		supportedvpd[0] = 0x00;
		supportedvpd[1] = 0x80;
		supportedvpd[2] = 0x83;
	}
} __attribute__ ((packed));

class SupportedVPDData : public DataSegment<SupportedVPDPage> {
public:
	SupportedVPDData(SupportedVPDPage *ptr)
		: DataSegment<SupportedVPDPage>(ptr, this->SIZE)
	{
		data->pagelength = this->SIZE - 4;
	}
	~SupportedVPDData() {}
	static const size_t SIZE = sizeof(SupportedVPDPage);
};

struct ProductSerialPage {
#define PRODUCT_SERIAL "0.1"
	uint8_t peripheral;
	uint8_t pagecode;
	uint8_t rsvd;
	uint8_t pagelength;
	uint8_t serial[4];
	ProductSerialPage()
		: peripheral(0), pagecode(0x80), rsvd(0),
		  pagelength(0)
	{
		memcpy(serial, PRODUCT_SERIAL, 3);
	}
} __attribute__ ((packed));

class ProductSerialData : public DataSegment<ProductSerialPage> {
public:
	ProductSerialData(ProductSerialPage *ptr)
		: DataSegment<ProductSerialPage>(ptr, this->SIZE)
	{
		data->pagelength = this->SIZE - 4;
	}
	~ProductSerialData() {}
	static const size_t SIZE = sizeof(ProductSerialPage);
};

struct Designator {
#define DESIGNATOR_ID "ISP"
	uint8_t codeset;
	uint8_t type;
	uint8_t rsvd;
	uint8_t length;
	uint8_t id[24];
	Designator()
		: codeset(0x01), type(0x01), rsvd(0),
		  length(0)
	{
		memcpy(id, DESIGNATOR_ID, strlen(DESIGNATOR_ID));
	}
} __attribute__ ((packed));

struct DeviceIDPage {
	uint8_t peripheral;
	uint8_t pagecode;
	uint16_t pagelength;
	Designator designator;
	DeviceIDPage()
		: peripheral(0), pagecode(0x83),
		  pagelength(0), designator()
	{
		designator.length = sizeof(Designator) - 4;
	}
} __attribute__ ((packed));

class DeviceIDData : public DataSegment<DeviceIDPage> {
public:
	DeviceIDData(DeviceIDPage *ptr)
		: DataSegment<DeviceIDPage>(ptr, this->SIZE)
	{}
	~DeviceIDData() {}
	static const size_t SIZE = sizeof(DeviceIDPage);
};

#endif /* __SCSI_INQUIRY_H__ */
