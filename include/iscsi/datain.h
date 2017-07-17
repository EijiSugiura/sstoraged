/**
   @file datain.h
   @brief iSCSI DataIn handler
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: datain.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __ISCSI_DATAIN_H__
#define __ISCSI_DATAIN_H__

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

struct DataInHeader {
	/** Operation code */
	uint8_t opcode;
	/** F|000|O|U|S bit */
	uint8_t flags;
	uint8_t rsvd2;
	/** SCSI Command Status */
	uint8_t cmd_status;
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
	/** Data SN */
	uint32_t datasn;
	/** Buffer Offset */
	uint32_t offset;
	/** Residual Count */
	uint32_t residual_count;
	DataInHeader() : opcode(iSCSI_OP_SCSI_DATA_RSP),
			 flags(0), rsvd2(0), cmd_status(GOOD),
			 hlength(0), dlength(), lun(),
			 itt(0), ttt(0),
			 statsn(0), expcmdsn(0), maxcmdsn(0), datasn(0), 
			 offset(0), residual_count(0)
	{}
};

/** SCSI DataIn generator class */
class DataInGenerator : public iSCSIGenerator<DataInHeader>{
public:
	/** Default Constructor */
 	DataInGenerator();
	/** Copy Constructor */
	DataInGenerator(DataInGenerator &old);
	/** Flags getter */
	uint8_t getFlags() const { return bhs->flags; }
	/** Data length getter */
	uint32_t getDlength() const
	{ return static_cast<uint32_t>(ntoh24(bhs->dlength)); }
	/** SCSI Command Status code setter */
	void setCmdStatus(const uint8_t code)
	{
		bhs->flags |= (ISCSI_FLAG_FINAL|DATAIN_FLAG_STATUS);
		bhs->cmd_status = code;
	}
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
	/** Data Sequence Number setter */
	void advanceDataSN() { bhs->datasn = htonl(datasn_gen()); }
	/** Data Sequence Number getter */
	uint32_t getDataSN() { return datasn_gen.cur(); }
	/** Buffer Offset setter */
	void addOffset(const uint32_t offset) { bhs->offset = htonl(ntohl(bhs->offset) + offset); }
	/** ResidualCount setter
	    @param exp : Expected data length in request
	    @param cur : Current data length for reply
	 */
	void setResidualCount(const uint32_t exp,
			      const uint32_t cur)
	{
		if(exp > cur){
			bhs->flags |= DATAIN_FLAG_UNDERFLOW;
			bhs->residual_count = htonl(exp - cur);
		} else if(exp < cur) {
			bhs->flags |= DATAIN_FLAG_OVERFLOW;
			bhs->residual_count = htonl(cur - exp);
			/** shrink to EXPDATALEN */
			setDlength(exp);
		}
	}
	/** Reserve/map tail space for data segment
	    @param size : size to reserve
	    @return pointer to data segment
	 */
	uint8_t* reserveDataSegment(const size_t size);
	/** change data segment length
	    @param size : new size
	    @return success or failure
	 */
	bool resetDlength(const size_t size);
	/** PDU serializer */
	CommonBufPtr serialize();

private:
	/** DataIn ACK flag
	    @todo : handle this flag for recovery level > 0
	 */
	const static uint8_t DATAIN_FLAG_ACK		= 0x40;
	/** DataIn Status flag */
	const static uint8_t DATAIN_FLAG_STATUS		= 0x01;
	/** DataIn Overflow flag */
	const static uint8_t DATAIN_FLAG_OVERFLOW	= 0x04;
	/** DataIn Underflow flag */
	const static uint8_t DATAIN_FLAG_UNDERFLOW	= 0x02;
	/** send buffer */
	CommonBufPtr cbuf;
	/** DataSN generator */
	SequenceCounter datasn_gen;
};

#endif /* __ISCSI_DATAIN_H__ */
