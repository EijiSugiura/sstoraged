/**
   @file command.h
   @brief iSCSI Command handler
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: command.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __ISCSI_COMMAND_H__
#define __ISCSI_COMMAND_H__

#ifdef HAVE_SCSI_SCSI_H
#include <scsi/scsi.h>
#else
#include "scsi.h"
#endif

#include "common.h"
#include "ioveccontainer.h"
#include "iscsi/iscsi.h"

using namespace std;

class iSCSISession;
class SCSIHandler;
/** SCSI Command handler class */
class CommandHandler : public iSCSIHandler {
public:
	/** Default Constructor */
	CommandHandler(iSCSISession *parent);
	/** The handler */
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
private:
	typedef map<uint8_t, boost::shared_ptr<SCSIHandler> > command_map_t;
	command_map_t commands;
};

/** SCSI Command Header */
struct CommandReqHeader {
	/** Operation code */
	uint8_t opcode;
	/** Final|Continue|Read|Write bits */
	uint8_t flags;
	uint8_t rsvd2[2];
	/** AHS's total length */
	uint8_t hlength;
	/** Data length */
	uint8_t dlength[3];
	/** Logical Unit Number */
	uint8_t lun[8];
	/** Initiator Task Tag */
	uint32_t itt;
	/** Data Length */
	uint32_t expdatalen;
	/** Command SN */
	uint32_t cmdsn;
	/** Expected Status SN */
	uint32_t expstatsn;
	/** SCSI Command Block */
	uint8_t scb[16];
	/* Additional Data (Command Dependent) */
};

/** Command PDU READ flag */
#define COMMAND_FLAG_READ		(0x40)
/** Command PDU WRITE flag */
#define COMMAND_FLAG_WRITE		(0x20)
/** Command attribute mask (3 bits) */
#define COMMAND_FLAG_ATTR_MASK		(0x07)

/** SCSI Command Attribute values */
typedef enum command_attr {
	COMMAND_ATTR_UNTAGGED =		0,
	COMMAND_ATTR_SIMPLE,
	COMMAND_ATTR_ORDERED,
	COMMAND_ATTR_HEAD_OF_QUEUE,
	COMMAND_ATTR_ACA,
} command_attr_t;

class WriteSegment;
/** SCSI Command parser class */
class CommandParser : public iSCSIParser<CommandReqHeader>{
public:
	/** Constructor */
 	CommandParser(CommandReqHeader *ptr) : iSCSIParser<CommandReqHeader>(ptr) {}
	/** the validator */
	bool valid() const;

	/** LUN getter */
	LUN getLUN() const { return LUN(bhs->lun); }
	/** EXPDATALEN getter */
	uint32_t getEXPDATALEN() const { return ntohl(bhs->expdatalen); }
	/** Command SN getter */
	uint32_t getCMDSN() const { return ntohl(bhs->cmdsn); }
	/** Expected Status SN getter */
	uint32_t getEXPSTATSN() const { return ntohl(bhs->expstatsn); }
	void * getSCB() const { return bhs->scb; }
	uint8_t getCDBOpCode() const { return bhs->scb[0]; }
	boost::shared_ptr<WriteSegment> getWriteSegment() const;
private:
};

/** iSCSI Status values. Valid if Rsp Selector bit is not set */
typedef enum iscsi_response {
	RESPONSE_CMD_COMPLETED = 0,
	RESPONSE_TARGET_FAILURE,
	RESPONSE_SUBSYS_FAILURE,
} iscsi_response_t;

/** SCSI Response Header */
struct SCSIRspHeader {
	/** Operation code */
	uint8_t opcode;
	/** F..o|u|O|U. */
	uint8_t flags;
	/** SCSI Response */
	uint8_t response;
	/** Command Status */
	uint8_t cmd_status;
	/** AHS's total length */
	uint8_t hlength;
	/** Data length */
	uint8_t dlength[3];
	uint8_t rsvd[8];
	/** Initiator Task Tag */
	uint32_t itt;
	/** SNACK Tag */
	uint32_t snack_tag;
	/** Status SN */
	uint32_t statsn;
	/** Expecteced Command SN */
	uint32_t expcmdsn;
	/** MAX Command SN */
	uint32_t maxcmdsn;
	/** Expected Data SN */
	uint32_t expdatasn;
	/** Bidirectional Read Residual Count */
	uint32_t bidirectional_read_residual_count;
	/** Residual Count */
	uint32_t residual_count;
	/* Response or Sense Data (optional) */
	SCSIRspHeader() : opcode(iSCSI_OP_SCSI_RSP),
			  flags(0),
			  response(RESPONSE_CMD_COMPLETED), cmd_status(),
			  hlength(0), dlength(), rsvd(),
			  itt(0), snack_tag(0),
			  statsn(0), expcmdsn(0), maxcmdsn(0), expdatasn(0), 
			  bidirectional_read_residual_count(0), residual_count(0)
	{}
};

/** SCSI SCSIRsp generator class */
class SCSIRspGenerator : public iSCSIGenerator<SCSIRspHeader>{
public:
	/** Default Constructor */
 	SCSIRspGenerator();
	/** Flags setter
	    Final bit is always on
	    @param flags : flags to set
	 */
	void setFlags(const uint8_t flags) { bhs->flags = (ISCSI_FLAG_FINAL|flags); }
	/** Flags getter */
	uint8_t getFlags() const { return bhs->flags; }
	/** Data length getter */
	uint32_t getDlength() const
	{ return static_cast<uint32_t>(ntoh24(bhs->dlength)); }
	/** SCSI Resonse setter */
	void setResponse(const iscsi_response_t code) { bhs->response = code; }
	/** SCSI Command Status code setter */
	void setCmdStatus(const uint8_t code) { bhs->cmd_status = code; }
	/** Status Sequence Number setter */
	void setSTATSN(const uint32_t statsn) { bhs->statsn = htonl(statsn); }
	/** Expected Command Sequence Number setter */
	void setEXPCMDSN(const uint32_t expcmdsn) { bhs->expcmdsn = htonl(expcmdsn); }
	/** MAX. Command Sequence Number setter */
	void setMAXCMDSN(const uint32_t maxcmdsn) { bhs->maxcmdsn = htonl(maxcmdsn); }
	/** Data Sequence Number setter */
	void setEXPDataSN(const uint32_t expdatasn) { bhs->expdatasn = htonl(expdatasn); }
	/** ResidualCount setter
	    @param exp : Expected data length in request
	    @param cur : Current data length for reply
	 */
	void setResidualCount(const uint32_t exp,
			      const uint32_t cur)
	{
		if(exp > cur){
			bhs->flags |= RESPONSE_FLAG_UNDERFLOW;
			bhs->residual_count = htonl(exp - cur);
		} else if(exp < cur) {
			bhs->flags |= RESPONSE_FLAG_OVERFLOW;
			bhs->residual_count = htonl(cur - exp);
			/** shrink to EXPDATALEN */
			setDlength(exp);
		}
	}
	/** PDU serializer */
	CommonBufPtr serialize();
private:
	/** send buffer */
	CommonBufPtr cbuf;
	/** SCSI Response Overflow flag */
	const static uint8_t RESPONSE_FLAG_OVERFLOW	= 0x04;
	/** SCSI Response Underflow flag */
	const static uint8_t RESPONSE_FLAG_UNDERFLOW	= 0x02;
	/** SCSI Response Bidirectional Overflow flag */
	const static uint8_t RESPONSE_FLAG_BIDI_OVERFLOW	= 0x10;
	/** SCSI Response Bidirectional Underflow flag */
	const static uint8_t RESPONSE_FLAG_BIDI_UNDERFLOW	= 0x08;
};

#endif /* __ISCSI_COMMAND_H__ */
