/**
   @file r2t.h
   @brief iSCSI R2T handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: r2t.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __ISCSI_R2T_H__
#define __ISCSI_R2T_H__

#ifdef HAVE_SCSI_SCSI_H
#include <scsi/scsi.h>
#else
#include "scsi.h"
#endif

#include <boost/shared_ptr.hpp>
#include "common.h"
#include "counter.h"
#include "ioveccontainer.h"
#include "iscsi/iscsi.h"

using namespace std;

struct R2THeader {
	/** Operation code */
	uint8_t opcode;
	/** F|....... bit */
	uint8_t flags;
	uint8_t rsvd[2];
	/** AHS's total length */
	uint8_t hlength;
	/** Data length */
	uint8_t dlength[3];
	/** LUN */
	uint8_t lun[8];
	/** Initiator Task Tag */
	uint32_t itt;
	/** Target Transfer Tag */
	uint32_t ttt;
	/** Status SN */
	uint32_t statsn;
	/** Expecteced Command SN */
	uint32_t expcmdsn;
	/** MAX Command SN */
	uint32_t maxcmdsn;
	/** R2T SN */
	uint32_t r2tsn;
	/** Buffer Offset */
	uint32_t offset;
	/** Desired Data Transfer Length */
	uint32_t transfer_length;
	R2THeader() : opcode(iSCSI_OP_R2T),
		      flags(0), rsvd(),
		      hlength(0), dlength(), lun(), itt(0), ttt(0),
		      statsn(0), expcmdsn(0), maxcmdsn(0), r2tsn(0), 
		      offset(0), transfer_length(0)
	{}
};

/** SCSI R2T generator class */
class R2TGenerator : public iSCSIGenerator<R2THeader>{
public:
	/** Default Constructor */
 	R2TGenerator();
	/** LUN setter */
	void setLUN(const LUN &lun) { memcpy(bhs->lun, lun.lun, lun.SIZE); }
	/** TargetTransferTag setter */
	void setTTT(const uint32_t ttt) { bhs->ttt = ttt; }
	/** Status Sequence Number setter */
	void setSTATSN(const uint32_t statsn) { bhs->statsn = htonl(statsn); }
	/** Expected Command Sequence Number setter */
	void setEXPCMDSN(const uint32_t expcmdsn) { bhs->expcmdsn = htonl(expcmdsn); }
	/** MAX. Command Sequence Number setter */
	void setMAXCMDSN(const uint32_t maxcmdsn) { bhs->maxcmdsn = htonl(maxcmdsn); }
	/** R2T setter */
	void setR2TSN(const uint32_t r2tsn) { bhs->r2tsn = htonl(r2tsn); }
	/** Buffer Offset setter */
	void setOffset(const uint32_t offset) { bhs->offset = htonl(offset); }
	/** DesiredDataTransferLength setter */
	void setDesiredDataTransferLength(const uint32_t transfer_length)
	{ bhs->transfer_length = htonl(transfer_length); }
	/** PDU serializer */
	CommonBufPtr serialize();
private:
	CommonBufPtr cbuf;
	SequenceCounter datasn_gen;
};

#endif /* __ISCSI_R2T_H__ */
