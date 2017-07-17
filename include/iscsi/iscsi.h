/** 
 * @file  iscsi/iscsi.h
 * @brief iSCSI header definitions
 * @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
 * @version $Id: iscsi.h 312 2007-09-28 00:56:17Z sugiura $
 */
#ifndef __ISCSI_ISCSI_H__
#define __ISCSI_ISCSI_H__

#include <string.h>

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

using namespace std;

struct SessionID {
	static const size_t SIZE=6;
	uint8_t	sid[SIZE];
	SessionID()
	{ memset(this->sid, 0, SIZE); }
	SessionID(uint8_t *ptr)
	{ set(ptr); }
	void set(uint8_t *ptr)
	{ memcpy(this->sid, ptr, SIZE); }
	bool operator == (const SessionID &right)
	{ return !memcmp(this->sid, right.sid, SIZE); }
};

struct LUN {
	static const size_t SIZE=8;
	uint8_t	lun[SIZE];
	LUN()
	{ memset(this->lun, 0, SIZE); }
	LUN(uint8_t *ptr)
	{ set(ptr); }
	LUN(const uint32_t id) : lun()
//	{ *(reinterpret_cast<uint32_t*>(&lun[4])) = htonl(id); }
	{
		uint32_t lun32 = htonl(id);
		memset(lun, 0x00, sizeof(lun32));
		memcpy(&lun[4], &lun32, sizeof(lun32));
	}
	uint32_t get()
//	{ return *(reinterpret_cast<uint32_t*>(&lun[4])); }
	{ 
		uint32_t lun32;
		memcpy(&lun32, &lun[4], sizeof(lun32));
		return lun32;
	}
	void set(uint8_t *ptr)
	{ memcpy(this->lun, ptr, SIZE); }
	bool operator == (const LUN &right)
	{return !memcmp(this->lun, right.lun, SIZE); }
};

/** iSCSI Generic Header */
struct iSCSIHeader {
	uint8_t opcode;
	/** Final bit only */
	uint8_t flags;
	uint8_t rsvd2[2];
	/** AHSs total length */
	uint8_t hlength;
	/** Data length */
	uint8_t dlength[3];
	/** Logical Unit */
	uint8_t lun[8];
	/** Initiator Task Tag */
	uint32_t itt;
	uint8_t other[28];
};

/** iSCSI Version Number */
#define ISCSI_DRAFT20_VERSION	(0x00)

/** Reserved value for initiator/target task tag */
#define ISCSI_RSVD_TASK_TAG	(0xffffffff)

/** network byte-order to host one converter, 24bit version
    assumes a pointer to a 3-byte array */
#define ntoh24(p) (((p)[0] << 16) | ((p)[1] << 8) | ((p)[2]))

/** host byte-order to network one converter, 24bit version
    assumes a pointer to a 3 byte array, and an integer value */
#define hton24(p, v) {\
        p[0] = (((v) >> 16) & 0xFF); \
        p[1] = (((v) >> 8) & 0xFF); \
        p[2] = ((v) & 0xFF); \
}

/** iSCSI protocol parser template class */
template<typename T>
class iSCSIParser {
public:
	/** Constructor with BHS pointer */
	iSCSIParser(T *ptr) : bhs(ptr) {}
	virtual ~iSCSIParser() {}
	/** Flags getter */
	uint8_t getFlags() const { return bhs->flags; }
	/** Final flag checker */
	bool isFinal() const { return (bhs->flags&ISCSI_FLAG_FINAL); }
	/** Immedeate Command Flag checker */
	bool isImmediateCmd() const { return (bhs->opcode & ISCSI_OP_IMMEDIATE); }
	/** Operation Code getter */
	uint8_t getOpCode() const { return (bhs->opcode & ISCSI_OPCODE_MASK); }
	/** AHS's total length getter */
	uint8_t getHlength() const { return bhs->hlength; }
	/** Data length getter */
	uint32_t getDlength() const
	{ return static_cast<uint32_t>(ntoh24(bhs->dlength)); }
	/** Whole PDU length getter */
	uint32_t getPDUlength() const
	{
		uint32_t pdu_length = (sizeof(iSCSIHeader) + getHlength() + getDlength());
		if(pdu_length % PAD_WORD_LEN)
			pdu_length += PAD_WORD_LEN - pdu_length%PAD_WORD_LEN;
		return pdu_length;
	}
	/** Initiator Task Tag Getter */
	uint32_t getITT() const { return bhs->itt; }
	/** Get offset from BHS head to DataSegment
	    @return 0 : there's no data segment
	    @return offset : offset to data segment
	 */
	size_t getDataSegmentOffset() const
	{
		if(getDlength() == 0)
			return 0;
		return sizeof(iSCSIHeader) + getHlength();
	}
	/** @todo: define new/delete operators? */

protected:
	/** BHS header */
	mutable T *bhs;
	/** last PDU has a final bit */
	const static uint8_t ISCSI_FLAG_FINAL	= 0x80;
	/** Opcode encoding bits */
	const static uint8_t ISCSI_OP_RETRY	= 0x80;
	const static uint8_t ISCSI_OP_IMMEDIATE	= 0x40;
	const static uint8_t ISCSI_OPCODE_MASK	= 0x3F;
	/** Padding word length */
	const static size_t PAD_WORD_LEN	= 4;
};

/* Client to Server Message Opcode values */
#define iSCSI_OP_NOOP_OUT		(0x00)
#define iSCSI_OP_SCSI_CMD		(0x01)
#define iSCSI_OP_TASK_MGT_REQ		(0x02)
#define iSCSI_OP_LOGIN_CMD		(0x03)
#define iSCSI_OP_TEXT_CMD		(0x04)
#define iSCSI_OP_SCSI_DATA		(0x05)
#define iSCSI_OP_LOGOUT_CMD		(0x06)
#define iSCSI_OP_SNACK_CMD		(0x10)

/* Server to Client Message Opcode values */
#define iSCSI_OP_NOOP_IN		(0x20)
#define iSCSI_OP_SCSI_RSP		(0x21)
#define iSCSI_OP_TASK_MGT_RSP		(0x22)
#define iSCSI_OP_LOGIN_RSP		(0x23)
#define iSCSI_OP_TEXT_RSP		(0x24)
#define iSCSI_OP_SCSI_DATA_RSP		(0x25)
#define iSCSI_OP_LOGOUT_RSP		(0x26)
#define iSCSI_OP_R2T			(0x31)
#define iSCSI_OP_ASYNC_MSG		(0x32)
#define iSCSI_OP_REJECT			(0x3f)

#define iSCSI_LOGIN_SESSION		(0)
#define iSCSI_DISCOVERY_SESSION		(1)
#define iSCSI_NORMAL_SESSION		(2)

/** iSCSI protocol generator template class */
template<typename T>
class iSCSIGenerator {
public:
	/** Default Constructor */
	iSCSIGenerator() : bhs(NULL) {}
	/** Destructor */
	virtual ~iSCSIGenerator() {}
	/** flags setter
	    @param flags : flags to set
	 */
	void setFlags(const uint8_t flags) { bhs->flags = flags; }
	/** Initiator Task Tag setter
	    @param itt : Initiator Task Tag to set
	 */
	void setITT(const uint32_t itt) { bhs->itt = itt; }
	/** Dlength setter
	    @param length : Data segment length to set
	 */
	void setDlength(const uint32_t length) { hton24(bhs->dlength, length); }
	/** @todo: define new/delete operaters? */

	/** last PDU has a final bit */
	const static uint8_t ISCSI_FLAG_FINAL	= 0x80;
protected:
	/** BHS header */
	T *bhs;
	/** Opcode Immediate flag */
	const static uint8_t ISCSI_OP_IMMEDIATE	= 0x40;
	/** Padding word length */
	const static size_t PAD_WORD_LEN	= 4;
};

class iSCSISession;
class iSCSIHandler {
public:
	iSCSIHandler() {}
	virtual ~iSCSIHandler() {}
	virtual bool handle(boost::shared_ptr<iSCSISession> session, 
			    const socket_t fd) = 0;
};

typedef boost::shared_ptr<iSCSIHandler> iSCSIHandlerPtr;

#endif /* __ISCSI_ISCSI_H__ */
