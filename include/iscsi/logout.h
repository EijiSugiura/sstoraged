/**
   @file logout.h
   @brief iSCSI Logout handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: logout.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __ISCSI_LOGOUT_H__
#define __ISCSI_LOGOUT_H__

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <map>
#include "common.h"
#include "ioveccontainer.h"
#include "iscsi/iscsi.h"

using namespace std;

class iSCSISession;
class SendTask;
class LogoutHandler : public iSCSIHandler {
public:
	/** Default Constructor */
	LogoutHandler() {}
	/** The handler */
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
};

/** logout reason_code values */
typedef enum logout_reason {
	LOGOUT_REASON_CLOSE_SESSION	= 0,
	LOGOUT_REASON_CLOSE_CONNECTION,
	LOGOUT_REASON_RECOVERY,
} logout_reason_t;

/** Logout Request Header */
struct LogoutReqHeader {
	/** Operation code */
	uint8_t opcode;
	/** Final bits|Reason code */
	uint8_t flags;
	uint8_t rsvd1[2];
	/** AHS's total length */
	uint8_t hlength;
	/** Data Length */
	uint8_t dlength[3];
	uint8_t rsvd2[8];
	/** Initiator Task Tag */
	uint32_t itt;
	/** Connection ID */
	uint16_t cid;
	uint16_t rsvd3;
	/** Command SN */
	uint32_t cmdsn;
	/** EXP STAT SN */
	uint32_t expstatsn;
	uint8_t rsvd4[16];
};

#include "validator.h"
class LogoutParser : public iSCSIParser<LogoutReqHeader>{
public:
	/** Constructor */
 	LogoutParser(LogoutReqHeader *ptr) : iSCSIParser<LogoutReqHeader>(ptr) {}
	/** the validator */
	bool valid() const;

	logout_reason_t getReason() const
	{ return static_cast<logout_reason_t>(LOGOUT_FLAG_REASON_MASK & bhs->flags); }
	uint32_t getEXPSTATSN() const { return ntohl(bhs->expstatsn); }
	uint16_t getCID() const { return ntohs(bhs->cid); }
private:
	/** Logout PDU REASON mask */
	const static uint8_t LOGOUT_FLAG_REASON_MASK	= 0x7F;

};

/** logout response status values */
typedef enum logout_response {
	LOGOUT_SUCCESS			= 0,
	LOGOUT_CID_NOT_FOUND,
	LOGOUT_RECOVERY_UNSUPPORTED,
	LOGOUT_CLEANUP_FAILED,
} logout_response_t;

/** Logout Response Header */
struct LogoutRspHeader {
	/** Operation code */
	uint8_t opcode;
	/** F|....... */
	uint8_t flags;
	/** Logout response values */
	uint8_t response;
	uint8_t rsvd2;
	/** AHS's total length */
	uint8_t hlength;
	/** Data Length */
	uint8_t dlength[3];
	uint8_t rsvd3[8];
	/** Initiator Task Tag */
	uint32_t itt;
	uint32_t rsvd4;
	/** STAT SN */
	uint32_t statsn;
	/** EXP Command SN */
	uint32_t expcmdsn;
	/** MAX. Command SN */
	uint32_t maxcmdsn;
	uint32_t rsvd5;
	/** Time2Wait */
	uint16_t t2wait;
	/** Time2Retain */
	uint16_t t2retain;
	uint32_t rsvd6;
	LogoutRspHeader() : opcode(iSCSI_OP_LOGOUT_RSP),
			    flags(0), response(LOGOUT_SUCCESS), rsvd2(0),
			    hlength(0), dlength(),
			    rsvd3(), itt(0), rsvd4(0),
			    statsn(0), expcmdsn(0), maxcmdsn(0),
			    rsvd5(0), t2wait(0), t2retain(0), rsvd6(0)
	{}
};

class LogoutGenerator : public iSCSIGenerator<LogoutRspHeader>{
public:
	/** Default Constructor */
 	LogoutGenerator();
	/** Flags setter
	    Final bit is always on
	    @param flags : flags to set
	 */
	void setFlags(const uint8_t flags) { bhs->flags = (ISCSI_FLAG_FINAL|flags); }
	/** Response code setter */
	void setResponse(const logout_response_t code) { bhs->response = code; }
	/** Status Sequence Number setter */
	void setSTATSN(const uint32_t statsn) { bhs->statsn = htonl(statsn); }
	/** Expected Command Sequence Number setter */
	void setEXPCMDSN(const uint32_t expcmdsn) { bhs->expcmdsn = htonl(expcmdsn); }
	/** MAX. Command Sequence Number setter */
	void setMAXCMDSN(const uint32_t maxcmdsn) { bhs->maxcmdsn = htonl(maxcmdsn); }
	/** Time2Wait setter */
	void setTime2Wait(const uint16_t val) { bhs->t2wait = htons(val); }
	/** Time2Retain setter */
	void setTime2Retain(const uint16_t val) { bhs->t2retain = htons(val); }
	/** Serializer No padding, cause no AHS & data */
	CommonBufPtr serialize() { return cbuf; }
private:
	CommonBufPtr cbuf;
};

#endif /* __ISCSI_LOGOUT_H__ */
