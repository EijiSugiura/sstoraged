/**
   @file login.h
   @brief iSCSI Login handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: login.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __ISCSI_LOGIN_H__
#define __ISCSI_LOGIN_H__

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <map>
#include <list>
#include <boost/shared_ptr.hpp>
#include "common.h"
#include "ioveccontainer.h"
#include "iscsi/iscsi.h"

using namespace std;

/** Login Request Header */
struct LoginReqHeader {
	/** Operation code */
	uint8_t opcode;
	/** Transit|Continue bits */
	uint8_t flags;
	/** Max. version supported */
	uint8_t max_version;
	/** Min. version supported */
	uint8_t min_version;
	/** AHS's total length */
	uint8_t hlength;
	/** Data Length */
	uint8_t dlength[3];
	/** Initiator Session ID */
	uint8_t isid[6];
	/** Target Session Identifying Handle */
	uint16_t tsih;
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
class LoginParser : public iSCSIParser<LoginReqHeader>{
public:
	/** Constructor */
 	LoginParser(LoginReqHeader *ptr);
	/** the validator */
	bool valid() const;

	uint32_t getEXPSTATSN() const { return ntohl(bhs->expstatsn); }
	SessionID getSessionID() const { return SessionID(bhs->isid); }
	uint16_t getTSIH() const { return ntohs(bhs->tsih); }
	uint16_t getCID() const { return ntohs(bhs->cid); }
	uint8_t currentStage() const
	{ return ((bhs->flags & LOGIN_MASK_CURRENT_STAGE) >> 2); }
	uint8_t nextStage() const
	{ return (bhs->flags & LOGIN_MASK_NEXT_STAGE); }

	/** parse key/value pair & store it pair map */
	bool parsePair(pair<string,string> apair);
	string getPairValue(const string &attr);
	vector<pair<string,string> > getPairAttrs()
	{ return pairs;	}
private:
	/** Login PDU Contine flag */
	const static uint8_t LOGIN_FLAG_CONTINUE	= 0x40;
	/** Continue flag checker */
	bool isContinue() const { return (bhs->flags & LOGIN_FLAG_CONTINUE); }
	/** Login PDU Transit flag */
	const static uint8_t LOGIN_FLAG_TRANSIT		= 0x80;
	/** Transit flag checker */
	bool isTransit() const { return (bhs->flags & LOGIN_FLAG_TRANSIT); }
	/** Login PDU Current Stage mask */
	const static uint8_t LOGIN_MASK_CURRENT_STAGE	= 0x0C;	/* 2 bits */
	/** Login PDU Next Stage mask */
	const static uint8_t LOGIN_MASK_NEXT_STAGE	= 0x03;	/* 2 bits */

	vector<pair<string,string> > pairs;
	typedef boost::shared_ptr<GenericValidator<string> > PairValidatorPtr;
	typedef map<string,PairValidatorPtr> pair_validators_t;
	pair_validators_t pair_validators;
};

/** Login Response Header */
struct LoginRspHeader {
	/** Operation code */
	uint8_t opcode;
	uint8_t flags;
	/** Max. version supported */
	uint8_t max_version;
	/** Active version */
	uint8_t active_version;
	/** AHS's total length */
	uint8_t hlength;
	/** Data Length */
	uint8_t dlength[3];
	/** Initiator Session ID */
	uint8_t isid[6];
	/** Target Session Identifying Handle */
	uint16_t tsih;
	/** Initiator Task Tag */
	uint32_t itt;
	uint32_t rsvd3;
	/** STAT SN */
	uint32_t statsn;
	/** EXP Command SN */
	uint32_t expcmdsn;
	/** MAX. Command SN */
	uint32_t maxcmdsn;
	/** see Login RSP ststus classes below */
	uint8_t status_class;
	/** see Login RSP Status details below */
	uint8_t status_detail;
	uint8_t rsvd4[10];
	LoginRspHeader() : opcode(iSCSI_OP_LOGIN_RSP),
			   flags(0),
			   max_version(ISCSI_DRAFT20_VERSION),
			   active_version(ISCSI_DRAFT20_VERSION),
			   hlength(0), dlength(), isid(), tsih(),
			   itt(0), rsvd3(0), statsn(0), expcmdsn(0), maxcmdsn(0),
			   status_class(0), status_detail(0), rsvd4()
	{}
};

class LoginGenerator : public iSCSIGenerator<LoginRspHeader>{
public:
	/** Default Constructor */
 	LoginGenerator();
	/** Flags setter 
	    always set TRANSIT bit
	 */
	void setFlags(const uint8_t flags) { bhs->flags = (LOGIN_FLAG_TRANSIT|flags); }
	/** Status Sequence Number setter */
	void setSTATSN(const uint32_t statsn) { bhs->statsn = htonl(statsn); }
	/** Expected Command Sequence Number setter */
	void setEXPCMDSN(const uint32_t expcmdsn) { bhs->expcmdsn = htonl(expcmdsn); }
	/** MAX. Command Sequence Number setter */
	void setMAXCMDSN(const uint32_t maxcmdsn) { bhs->maxcmdsn = htonl(maxcmdsn); }
	void setSessionID(const SessionID &sid) { memcpy(bhs->isid, sid.sid, sid.SIZE); }
	void setStatusClass(const uint8_t status_class)
	{ bhs->status_class = status_class; }
	void setStatusDetail(const uint8_t status_detail)
	{ bhs->status_detail = status_detail; }
	void addKeyValue(const string& key, const string &value);
	CommonBufPtr serialize();
private:
	/** Login PDU Transit flag */
	const static uint8_t LOGIN_FLAG_TRANSIT		= 0x80;
	CommonBufPtr cbuf;
	uint32_t total_dlength;
};

/** Login stage (phase) codes for CSG, NSG */
#define SECURITY_NEGOTIATION_STAGE	(0)
#define OP_PARMS_NEGOTIATION_STAGE	(1)
#define FULL_FEATURE_PHASE		(3)
#define UNKNOWN_PHASE			(0xFF)

/** Login Status response classes */
#define STATUS_CLS_SUCCESS		(0x00)
#define STATUS_CLS_REDIRECT		(0x01)
#define STATUS_CLS_INITIATOR_ERR	(0x02)
#define STATUS_CLS_TARGET_ERR		(0x03)

/** Login Status response detail codes */
/** Class-0 (Success) */
#define LOGIN_STATUS_ACCEPT		(0x00)

/** Class-1 (Redirection) */
#define LOGIN_STATUS_TGT_MOVED_TEMP	(0x01)
#define LOGIN_STATUS_TGT_MOVED_PERM	(0x02)

/** Class-2 (Initiator Error) */
#define LOGIN_STATUS_INIT_ERR		(0x00)
#define LOGIN_STATUS_AUTH_FAILED	(0x01)
#define LOGIN_STATUS_TGT_FORBIDDEN	(0x02)
#define LOGIN_STATUS_TGT_NOT_FOUND	(0x03)
#define LOGIN_STATUS_TGT_REMOVED	(0x04)
#define LOGIN_STATUS_NO_VERSION		(0x05)
#define LOGIN_STATUS_ISID_ERROR		(0x06)
#define LOGIN_STATUS_MISSING_FIELDS	(0x07)
#define LOGIN_STATUS_CONN_ADD_FAILED	(0x08)
#define LOGIN_STATUS_NO_SESSION_TYPE	(0x09)
#define LOGIN_STATUS_NO_SESSION		(0x0a)
#define LOGIN_STATUS_INVALID_REQUEST	(0x0b)

/** Class-3 (Target Error) */
#define LOGIN_STATUS_TARGET_ERROR 	(0x00)
#define LOGIN_STATUS_SVC_UNAVAILABLE	(0x01)
#define LOGIN_STATUS_NO_RESOURCES 	(0x02)

/** Min. and Max. length of a PDU we can support */
#define MIN_PDU_LENGTH		(8 << 9)	/* 4KB */
#define MAX_PDU_LENGTH		(0xffffffff)	/* Huge */

class iSCSIManager;
class iSCSISession;
class GenericPairHandler;
class Task;
class LoginHandler : public iSCSIHandler {
public:
	/** Default Constructor */
	LoginHandler();
	/** The handler */
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
private:
	bool handle1stRequest(boost::shared_ptr<iSCSISession> parent,
			      const socket_t fd,
			      LoginParser &req,
			      list<boost::shared_ptr<Task> > &tasks);
	bool handle1stDiscoveryRequest(boost::shared_ptr<iSCSISession> parent,
				       const socket_t fd,
				       LoginParser &req,
				       list<boost::shared_ptr<Task> > &tasks);
	bool handle1stNormalRequest(boost::shared_ptr<iSCSISession> parent,
				    const socket_t fd,
				    LoginParser &req,
				    list<boost::shared_ptr<Task> > &tasks);
	iSCSIManager &manager;
	typedef boost::shared_ptr<GenericPairHandler> PairHandlerPtr;
	typedef map<string,PairHandlerPtr> pair_handlers_t;
	pair_handlers_t pair_handlers;
};

#endif /* __ISCSI_LOGIN_H__ */
