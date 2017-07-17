/** 
 * @file  nopout.h
 * @brief Nop-Out header definitions
 * @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
 * @version $Id: nopout.h 156 2007-06-28 07:18:19Z sugiura $
 */
#ifndef __ISCSI_NOPOUT_H__
#define __ISCSI_NOPOUT_H__

#include <boost/shared_ptr.hpp>
#include "ioveccontainer.h"
#include "iscsi/iscsi.h"

using namespace std;

class iSCSISession;

/** Nop-Out handler class */
class NopOutHandler : public iSCSIHandler {
public:
	/** Default Constructor */
	NopOutHandler() {}
	/** The handler */
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
};

/** Nop-Out Header */
struct NopOutHeader {
	/** Operation code */
	uint8_t opcode;
	/** F....... */
	uint8_t flags;
	uint16_t rsvd2;
	uint8_t rsvd3;
	/** Data Length */
	uint8_t dlength[3];
	/** Logical Unit Number */
	uint8_t lun[8];
	/** Initiator Task Tag */
	uint32_t itt;
	/** Target Transfer Tag */
	uint32_t ttt;
	/** Command SN */
	uint32_t cmdsn;
	/** Expected Status SN */
	uint32_t expstatsn;
	uint8_t rsvd4[16];
};

/** Nop-Out parser class */
class NopOutParser : public iSCSIParser<NopOutHeader> {
public:
	/** Constructor */
 	NopOutParser(NopOutHeader *ptr) : iSCSIParser<NopOutHeader>(ptr) {}
	/** the validator */
	bool valid() const;

	/** LUN getter */
	LUN getLUN() const { return LUN(bhs->lun); }
	/** Target Transfer Tag Getter */
	uint32_t getTTT() const { return bhs->ttt; }
	/** Command SN getter */
	uint32_t getCMDSN() const { return ntohl(bhs->cmdsn); }
	/** Expected Status SN getter */
	uint32_t getEXPSTATSN() const { return ntohl(bhs->expstatsn); }
};

/** Nop-Out generator class */
class NopOutGenerator : public iSCSIGenerator<NopOutHeader> {
public:
	/** Default Constructor */
	NopOutGenerator();

	/** Target Transfer Tag Setter */
	void setTTT(const uint32_t ttt) { bhs->ttt =  ttt; }
	/** Command Sequence Number setter */
	void setCMDSN(const uint32_t cmdsn) { bhs->cmdsn = htonl(cmdsn); }
	/** Expected Status Sequence Number setter */
	void setEXPSTATSN(const uint32_t expstatsn) { bhs->expstatsn = htonl(expstatsn); }
	/** Serializer No padding, cause no AHS & data */
	CommonBufPtr serialize() { return cbuf; }
private:
	CommonBufPtr cbuf;
};

#endif /* __ISCSI_NOPOUT_H__ */
