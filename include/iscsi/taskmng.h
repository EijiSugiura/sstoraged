/**
   @file taskmng.h
   @brief iSCSI TaskMng handler
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: taskmng.h 164 2007-07-03 05:36:07Z sugiura $
 */

#ifndef __ISCSI_TASKMNG_H__
#define __ISCSI_TASKMNG_H__

#include "common.h"
#include "ioveccontainer.h"
#include "iscsi/iscsi.h"

using namespace std;

class iSCSISession;
class TaskMngHandler : public iSCSIHandler {
public:
	/** Default Constructor */
	TaskMngHandler() {}
	/** The handler */
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
};

/** Task Function code */
typedef enum task_function {
	TASK_FUNC_ABORT_TASK = 1,
	TASK_FUNC_ABORT_TASK_SET,
	TASK_FUNC_CLEAR_ACA,
	TASK_FUNC_CLEAR_TASK_SET,
	TASK_FUNC_LOGICAL_UNIT_RESET,
	TASK_FUNC_TARGET_WARM_RESET,
	TASK_FUNC_TARGET_COLD_RESET,
	TASK_FUNC_TASK_REASSIGN,
} task_function_t;

/** TaskMng Request Header */
struct TaskMngReqHeader {
	/** Operation code */
	uint8_t opcode;
	/** Final bits|Function code */
	uint8_t flags;
	uint8_t rsvd1[2];
	/** AHS's total length */
	uint8_t hlength;
	/** Data Length */
	uint8_t dlength[3];
	/** LUN */
	uint8_t lun[8];
	/** Initiator Task Tag */
	uint32_t itt;
	/** Referenced Task Tag */
	uint32_t rtt;
	/** Command SN */
	uint32_t cmdsn;
	/** Expected Status SN */
	uint32_t expstatsn;
	/** Referenced Command SN for ABORT TASK */
	uint32_t refcmdsn;
	/** Expected Data SN for */
	uint32_t expdatasn;
	uint8_t rsvd2[8];
};

#include "validator.h"
class TaskMngParser : public iSCSIParser<TaskMngReqHeader>{
public:
	/** Constructor */
 	TaskMngParser(TaskMngReqHeader *ptr) : iSCSIParser<TaskMngReqHeader>(ptr) {}
	/** the validator */
	bool valid() const;

	task_function_t getFunction() const
	{ return static_cast<task_function_t>(TASKMNG_FLAG_FUNCTION_MASK & bhs->flags); }
	/** LUN getter */
	LUN getLUN() const { return LUN(bhs->lun); }
	uint32_t getRTT() const { return ntohl(bhs->rtt); }
	uint32_t getEXPSTATSN() const { return ntohl(bhs->expstatsn); }
	uint32_t getREFCMDSN() const { return ntohl(bhs->refcmdsn); }
	uint32_t getEXPDATASN() const { return ntohl(bhs->expdatasn); }
private:
	/** TaskMng PDU Function mask */
	const static uint8_t TASKMNG_FLAG_FUNCTION_MASK	= 0x7F;
};

/** logout response status values */
typedef enum taskmng_response {
	TASKMNG_FUNC_COMPLETE = 0,
	TASKMNG_TASK_NOT_EXISTS,
	TASKMNG_LUN_NOT_EXISTS,
	TASKMNG_TASK_STILL_ALLEGIANT,
	TASKMNG_TASK_REASIGN_NOT_SUPPORT,
	TASKMNG_NOT_SUPPORT,
	TASKMNG_AUTH_FAILED,
	TASKMNG_REJECTED = 0xFF,
} taskmng_response_t;

/** TaskMng Response Header */
struct TaskMngRspHeader {
	/** Operation code */
	uint8_t opcode;
	/** F|....... */
	uint8_t flags;
	/** TaskMng response values */
	uint8_t response;
	uint8_t rsvd2;
	/** AHS's total length */
	uint8_t hlength;
	/** Data Length */
	uint8_t dlength[3];
	uint8_t lun[8];
	/** Initiator Task Tag */
	uint32_t itt;
	uint32_t rsvd4;
	/** STAT SN */
	uint32_t statsn;
	/** EXP Command SN */
	uint32_t expcmdsn;
	/** MAX. Command SN */
	uint32_t maxcmdsn;
	uint8_t rsvd5[12];
	TaskMngRspHeader() : opcode(iSCSI_OP_TASK_MGT_RSP),
			    flags(0), response(TASKMNG_FUNC_COMPLETE), rsvd2(0),
			    hlength(0), dlength(),
			    lun(), itt(0), rsvd4(0),
			    statsn(0), expcmdsn(0), maxcmdsn(0),
			    rsvd5()
	{}
};

class TaskMngGenerator : public iSCSIGenerator<TaskMngRspHeader>{
public:
	/** Default Constructor */
 	TaskMngGenerator();
	/** Response code setter */
	void setResponse(const taskmng_response_t code) { bhs->response = code; }
	/** LUN setter */
	void setLUN(const LUN &lun) { memcpy(bhs->lun, lun.lun, lun.SIZE); }
	/** Status Sequence Number setter */
	void setSTATSN(const uint32_t statsn) { bhs->statsn = htonl(statsn); }
	/** Expected Command Sequence Number setter */
	void setEXPCMDSN(const uint32_t expcmdsn) { bhs->expcmdsn = htonl(expcmdsn); }
	/** MAX. Command Sequence Number setter */
	void setMAXCMDSN(const uint32_t maxcmdsn) { bhs->maxcmdsn = htonl(maxcmdsn); }
	/** Serializer No padding, cause no AHS & data */
	CommonBufPtr serialize() { return cbuf; }
private:
	CommonBufPtr cbuf;
};

#endif /* __ISCSI_TASKMNG_H__ */
