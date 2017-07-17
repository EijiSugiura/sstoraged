/**
   @file maintenancein.h
   @brief SCSI MaintenanceIn handler
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: maintenancein.h 228 2007-07-23 04:42:42Z sugiura $
 */

#ifndef __SCSI_MAINTENANCEIN_H__
#define __SCSI_MAINTENANCEIN_H__

#include <boost/shared_ptr.hpp>
#include "common.h"
#include "scsi/generic.h"

using namespace std;

class iSCSISession;
class MaintenanceInHandler : public SCSIHandler {
public:
	MaintenanceInHandler();
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
private:
};

/** MaintenanceIn Command */
struct MaintenanceInCommand {
	/** Operation code */
	uint8_t opcode;
	/** Service Action*/
	uint8_t service_action;
	/** RCTD|....|Reporting Options */
	uint8_t  flags;
	/** Requested Operation Code */
	uint8_t  requested_operation_code;
	/** Requested Service Action */
	uint16_t requested_service_action;
	/** Allocation Length */
	uint32_t allocation_length;
	uint8_t  rsvd;
	/** Control */
	uint8_t control;
} __attribute__ ((packed));

class MaintenanceInParser : public SCSIParser<MaintenanceInCommand>{
public:
	/** Constructor */
 	MaintenanceInParser(MaintenanceInCommand *ptr)
		: SCSIParser<MaintenanceInCommand>(ptr) {}
	/** the validator */
	bool valid() const;

	/** check Service Action In 16 */
	bool isReportSupportedOpCodes() const { return (cdb->service_action == REPORT_SUPPORTED_OPCODES); }
private:
	static const uint8_t REPORT_SUPPORTED_OPCODES = 0x0C;
};

struct SenseErrorHeader {
	/** Response Code */
	uint8_t response_code;
	uint8_t rsvd;
	/** FileMark|EOM|ILI|.|Sense Key */
	uint8_t flags;
	/** Sense Info */
	uint32_t sense_info;
	/** Additional Length */
	uint8_t additional_length;
	/** Command Specific Information */
	uint32_t command_specific_information;
	/** Additional Sense + Qualifier */
	uint16_t additional_sense;
	/** Field Replacable Unit */
	uint8_t  field_replacable_unit;
	/** SKSV|Sense Key Specific */
	uint8_t  flags2[3];
	SenseErrorHeader()
	: response_code(0x70), 
	  /** Illegal Request */
		flags(0x05), 
		additional_length(10), 
	  /** Invalid field in CDB */
		additional_sense(htons(0x2400)) {}
} __attribute__ ((packed));

class SenseErrorData : public DataSegment<SenseErrorHeader> {
public:
	SenseErrorData(SenseErrorHeader* ptr)
		: DataSegment<SenseErrorHeader>(ptr,this->SIZE)
	{}
	~SenseErrorData() {}
	static const size_t SIZE = sizeof(SenseErrorHeader);
private:
};

#endif /* __SCSI_MAINTENANCEIN_H__ */
