/**
   @file reportluns.h
   @brief SCSI ReportLUNS handler
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: reportluns.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __SCSI_REPORTLUNS_H__
#define __SCSI_REPORTLUNS_H__

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <boost/shared_ptr.hpp>
#include <map>
#include "common.h"
#include "scsi/generic.h"

using namespace std;

class iSCSISession;
class ReportLUNSHandler : public SCSIHandler {
public:
	ReportLUNSHandler();
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
private:
};

/** ReportLUNS Request Header */
struct ReportLUNSHeader {
	/** Operation code */
	uint8_t opcode;
	uint8_t rsvd1;
	/** Select Port */
	uint8_t select_report;
	uint8_t rsvd2[3];
	/** Allocation Length */
	uint32_t allocation_length;
	uint8_t rsvd3;
	/** Control */
	uint8_t control;
} __attribute__ ((packed));

class ReportLUNSParser : public SCSIParser<ReportLUNSHeader>{
public:
	/** Constructor */
 	ReportLUNSParser(ReportLUNSHeader *ptr) : SCSIParser<ReportLUNSHeader>(ptr) {}
	/** the validator */
	bool valid() const;

	/** Select Report getter */
	uint8_t getSelectReport() const { return cdb->select_report; }
	/** Allocation Length getter */
	uint32_t getAllocationLength() const { return ntohl(cdb->allocation_length); }
};

struct ReportLUNSheader {
	uint32_t lun_list_length;
	uint32_t rsvd;
	ReportLUNSheader() : lun_list_length(0), rsvd(0) {}
};

class ReportLUNSData : public DataSegment<ReportLUNSheader> {
public:
	ReportLUNSData(ReportLUNSheader *ptr, const size_t size)
		: DataSegment<ReportLUNSheader>(ptr, size)
	{}
	~ReportLUNSData() {}
	/** Add new LUN to data
	    @param id : LUN ID
	    @return success or failure
	 */
	bool add(uint32_t id);
};

#endif /* __SCSI_REPORTLUNS_H__ */
