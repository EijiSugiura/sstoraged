/**
   @file dataout.h
   @brief iSCSI DataOut handler
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: dataout.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __ISCSI_DATAOUT_H__
#define __ISCSI_DATAOUT_H__

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

class iSCSISession;
class iSCSIInitiator;
/** DataOut handler class */
class DataOutHandler : public iSCSIHandler {
public:
	/** Default Constructor */
	DataOutHandler(iSCSISession *parent);
	/** The handler */
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
private:
	boost::shared_ptr<iSCSIInitiator> initiator;
	uint32_t mrds;
};

struct DataOutHeader {
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
	uint32_t rsvd2;
	/** Expecteced Status SN */
	uint32_t expstatsn;
	uint32_t rsvd3;
	/** Data SN */
	uint32_t datasn;
	/** Buffer Offset */
	uint32_t offset;
	uint32_t rsvd4;
};

/** SCSI DataOut parser class */
class DataOutParser : public iSCSIParser<DataOutHeader>{
public:
	/** Default Constructor */
 	DataOutParser(DataOutHeader *ptr) : iSCSIParser<DataOutHeader>(ptr) {}
	/** the validator */
	bool valid() const;

	/** LUN getter */
	LUN getLUN() const { return LUN(bhs->lun); }
	/** Target Transfer Tag Getter */
	uint32_t getTTT() const { return bhs->ttt; }
	/** Expected Status SN getter */
	uint32_t getEXPSTATSN() const { return ntohl(bhs->expstatsn); }
	/** Data SN getter */
	uint32_t getDataSN() const { return ntohl(bhs->datasn); }

private:
	/** send buffer */
	CommonBufPtr cbuf;
};

#endif /* __ISCSI_DATAIN_H__ */
