/**
   @file dummy.cpp
   @brief SCSI Dummy handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: dummy.cpp 312 2007-09-28 00:56:17Z sugiura $
 */

#include "common.h"

#ifdef HAVE_SCSI_SCSI_H
#include <scsi/scsi.h>
#else
#include "scsi.h"
#endif

#include <iomanip>
#include "configurator.h"
#include "logger.h"
#include "session.h"
#include "iscsi/protocol.h"
#include "scsi/generic.h"
#include "scsi/dummy.h"

DummyHandler::DummyHandler()
{
	opstr[0x00] = "TEST UNIT READY";
	opstr[0x01] = "REZERO UNIT";
	opstr[0x03] = "REQUEST SENSE";
	opstr[0x04] = "FORMAT UNIT";
	opstr[0x05] = "READ BLOCK LIMITS";
	opstr[0x07] = "REASSIGN BLOCKS";
	opstr[0x08] = "READ 6";
	opstr[0x0a] = "WRITE 6";
	opstr[0x0b] = "SEEK 6";
	opstr[0x0f] = "READ REVERSE";
	opstr[0x10] = "WRITE FILEMARKS";
	opstr[0x11] = "SPACE";
	opstr[0x12] = "INQUIRY";
	opstr[0x14] = "RECOVER BUFFERED DATA";
	opstr[0x15] = "MODE SELECT";
	opstr[0x16] = "RESERVE";
	opstr[0x17] = "RELEASE";
	opstr[0x18] = "COPY";
	opstr[0x19] = "ERASE";
	opstr[0x1a] = "MODE SENSE";
	opstr[0x1b] = "START STOP";
	opstr[0x1c] = "RECEIVE DIAGNOSTIC";
	opstr[0x1d] = "SEND DIAGNOSTIC";
	opstr[0x1e] = "ALLOW MEDIUM REMOVAL";
	opstr[0x24] = "SET WINDOW";
	opstr[0x25] = "READ CAPACITY";
	opstr[0x28] = "READ 10";
	opstr[0x2a] = "WRITE 10";
	opstr[0x2b] = "SEEK 10";
	opstr[0x2e] = "WRITE VERIFY";
	opstr[0x2f] = "VERIFY";
	opstr[0x30] = "SEARCH HIGH";
	opstr[0x31] = "SEARCH EQUAL";
	opstr[0x32] = "SEARCH LOW";
	opstr[0x33] = "SET LIMITS";
	opstr[0x34] = "PRE FETCH";
	opstr[0x34] = "READ POSITION";
	opstr[0x35] = "SYNCHRONIZE CACHE";
	opstr[0x36] = "LOCK UNLOCK CACHE";
	opstr[0x37] = "READ DEFECT DATA";
	opstr[0x38] = "MEDIUM SCAN";
	opstr[0x39] = "COMPARE";
	opstr[0x3a] = "COPY VERIFY";
	opstr[0x3b] = "WRITE BUFFER";
	opstr[0x3c] = "READ BUFFER";
	opstr[0x3d] = "UPDATE BLOCK";
	opstr[0x3e] = "READ LONG";
	opstr[0x3f] = "WRITE LONG";
	opstr[0x40] = "CHANGE DEFINITION";
	opstr[0x41] = "WRITE SAME";
	opstr[0x43] = "READ TOC";
	opstr[0x4c] = "LOG SELECT";
	opstr[0x4d] = "LOG SENSE";
	opstr[0x55] = "MODE SELECT 10";
	opstr[0x56] = "RESERVE 10";
	opstr[0x57] = "RELEASE 10";
	opstr[0x5a] = "MODE SENSE 10";
	opstr[0x5e] = "PERSISTENT RESERVE IN";
	opstr[0x5f] = "PERSISTENT RESERVE OUT";
	opstr[0x9e] = "SERVICE ACTION IN 16";
	opstr[0xa3] = "MAINTENANCE IN";
	opstr[0xa5] = "MOVE MEDIUM";
	opstr[0xa8] = "READ 12";
	opstr[0xaa] = "WRITE 12";
	opstr[0xae] = "WRITE VERIFY 12";
	opstr[0xb0] = "SEARCH HIGH 12";
	opstr[0xb1] = "SEARCH EQUAL 12";
	opstr[0xb2] = "SEARCH LOW 12";
	opstr[0xb8] = "READ ELEMENT STATUS";
	opstr[0xb6] = "SEND VOLUME TAG";
	opstr[0xea] = "WRITE LONG 2";
}

bool DummyHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	CommandParser req(reinterpret_cast<CommandReqHeader*>(parent->frontToken()));
	map<uint8_t,string>::const_iterator itr = opstr.find(req.getCDBOpCode());
	if(itr == opstr.end()){
		ostringstream os;
		os << hex << showbase << setfill('0')  << setw(2) << req.getCDBOpCode();
		LOG4CXX_DEBUG(logger, "DUMMY : unknown opcode : " + os.str());
	} else {
		LOG4CXX_DEBUG(logger, "DUMMY : " + itr->second);
	}
	/** Check Final flag */
	if(!(req.isFinal())){
		LOG4CXX_WARN(logger, "Initiator MUST set final bit");
 		return false;
	}

	/** generate DataIn */
 	SCSIRspGenerator reply;
	reply.setFlags(reply.ISCSI_FLAG_FINAL);
	reply.setCmdStatus(GOOD);
	reply.setITT(req.getITT());
	/** update STATSN */
	iSCSIConnectionPtr conn = parent->connection.find(fd);
	reply.setSTATSN(conn->getSTATSN());
	conn->advanceSTATSN();
	reply.setEXPCMDSN(parent->getEXPCMDSN());
	reply.setMAXCMDSN(parent->getMAXCMDSN());

	/** set Residual Count */
	reply.setResidualCount(req.getEXPDATALEN(),reply.getDlength());

	/** convert reply -> send task */
	parent->sendtask.push(SendTaskPtr(new SendTask(parent, fd, reply.serialize())));
	return true;
}
