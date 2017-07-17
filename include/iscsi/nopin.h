/** 
 * @file  nopin.h
 * @brief Nop-In header definitions
 * @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
 * @version $Id: nopin.h 156 2007-06-28 07:18:19Z sugiura $
 */
#ifndef __ISCSI_NOPIN_H__
#define __ISCSI_NOPIN_H__

#include <boost/shared_ptr.hpp>
#include "ioveccontainer.h"
#include "iscsi/iscsi.h"

using namespace std;

class iSCSISession;

/** Nop-In handler class */
class NopInHandler : public iSCSIHandler {
public:
	/** Default Constructor */
	NopInHandler() {}
	/** The handler */
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
};

/** Nop-In Header */
struct NopInHeader {
	/** Operation code */
	uint8_t opcode;
	/** F....... */
	uint8_t flags;
	uint16_t rsvd2;
	/** AHS's total length */
	uint8_t hlength;
	/** Data Length */
	uint8_t dlength[3];
	/** Logical Unit Number */
	uint8_t lun[8];
	/** Initiator Task Tag */
	uint32_t itt;
	/** Target Transfer Tag */
	uint32_t ttt;
	/** Status SN */
	uint32_t statsn;
	/** Expected Status SN */
	uint32_t expcmdsn;
	/** Max Status SN */
	uint32_t maxcmdsn;
	uint8_t rsvd4[12];
};

/** Nop-In parser class */
class NopInParser : public iSCSIParser<NopInHeader> {
public:
	/** Constructor */
 	NopInParser(NopInHeader *ptr) : iSCSIParser<NopInHeader>(ptr) {}
	/** the validator */
	bool valid() const;

	/** LUN getter */
	LUN getLUN() const { return LUN(bhs->lun); }
	/** Target Transfer Tag Getter */
	uint32_t getTTT() const { return bhs->ttt; }
	/** Status SN getter */
	uint32_t getSTATSN() const { return ntohl(bhs->statsn); }
	/** Expected Command SN getter */
	uint32_t getEXPCMDSN() const { return ntohl(bhs->expcmdsn); }
	/** Max Command SN getter */
	uint32_t getMAXCMDSN() const { return ntohl(bhs->maxcmdsn); }
};

/** Nop-In generator class */
class NopInGenerator : public iSCSIGenerator<NopInHeader> {
public:
	/** Default Constructor */
	NopInGenerator();

	/** Target Transfer Tag Setter */
	void setTTT(const uint32_t ttt) { bhs->ttt = ttt; }
	/** Status Sequence Number setter */
	void setSTATSN(const uint32_t statsn) { bhs->statsn = htonl(statsn); }
	/** Expected Command Sequence Number setter */
	void setEXPCMDSN(const uint32_t expcmdsn) { bhs->expcmdsn = htonl(expcmdsn); }
	/** Command Sequence Number setter */
	void setMAXCMDSN(const uint32_t maxcmdsn) { bhs->maxcmdsn = htonl(maxcmdsn); }
	/** Serializer No padding, cause no AHS & data */
	CommonBufPtr serialize() { return cbuf; }
private:
	CommonBufPtr cbuf;
};

#endif /* __ISCSI_NOPIN_H__ */
