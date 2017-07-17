/**
   @file text.h
   @brief iSCSI Text handler
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: text.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __ISCSI_TEXT_H__
#define __ISCSI_TEXT_H__

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <map>
#include "common.h"
#include "ioveccontainer.h"
#include "iscsi/iscsi.h"

using namespace std;

class iSCSISession;
class GenericPairHandler;
class SendTask;
class TextHandler : public iSCSIHandler {
public:
	/** Default Constructor */
	TextHandler();
	/** The handler */
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
private:
	typedef boost::shared_ptr<GenericPairHandler> PairHandlerPtr;
	typedef map<string,PairHandlerPtr> pair_handlers_t;
	pair_handlers_t pair_handlers;
};

/** Text Request Header */
struct TextReqHeader {
	/** Operation code */
	uint8_t opcode;
	/** Final|Continue bits */
	uint8_t flags;
	uint8_t rsvd2[2];
	/** AHS's total length */
	uint8_t hlength;
	/** Data Length */
	uint8_t dlength[3];
	/** Reserved or LUN */
	uint8_t lun[8];
	/** Initiator Task Tag */
	uint32_t itt;
	/** Target Transfer Tag */
	uint32_t ttt;
	/** Command SN */
	uint32_t cmdsn;
	/** EXP STAT SN */
	uint32_t expstatsn;
	uint8_t rsvd5[16];
};

#include "validator.h"
class TextParser : public iSCSIParser<TextReqHeader>{
public:
	/** Constructor */
 	TextParser(TextReqHeader *ptr);
	/** the validator */
	bool valid() const;

	uint32_t getTTT() const { return bhs->ttt; }
	uint32_t getCMDSN() const { return ntohl(bhs->cmdsn); }
	uint32_t getEXPSTATSN() const { return ntohl(bhs->expstatsn); }

	/** parse key/value pair & store it pair map */
	bool parsePair(pair<string,string> apair);
	/** key/value pair getter */
	string getPairValue(const string &attr);
	vector<pair<string,string> > getPairAttrs()
	{ return pairs;	}
private:
	/** Text PDU Contine flags */
	const static uint8_t TEXT_FLAG_CONTINUE		= 0x40;
	/** Continue flag checker */
	bool isContinue() const { return (bhs->flags & TEXT_FLAG_CONTINUE); }

	vector<pair<string,string> > pairs;
	typedef boost::shared_ptr<GenericValidator<string> > PairValidatorPtr;
	typedef map<string,PairValidatorPtr> pair_validators_t;
	pair_validators_t pair_validators;
};

/** Text Response Header */
struct TextRspHeader {
	/** Operation code */
	uint8_t opcode;
	uint8_t flags;
	uint8_t rsvd2[2];
	/** AHS's total length */
	uint8_t hlength;
	/** Data Length */
	uint8_t dlength[3];
	uint8_t rsvd4[8];
	/** Initiator Task Tag */
	uint32_t itt;
	/** Target Transfer Tag */
	uint32_t ttt;
	/** Status SN */
	uint32_t statsn;
	/** Expected CMD SN */
	uint32_t expcmdsn;
	/** MAX CMD SN */
	uint32_t maxcmdsn;
	uint8_t rsvd5[12];
	TextRspHeader() : opcode(iSCSI_OP_TEXT_RSP),
			  flags(0),
			  rsvd2(),
			  hlength(0), dlength(), rsvd4(),
			  itt(0), ttt(0), statsn(0), expcmdsn(0), maxcmdsn(0),
			  rsvd5()
	{}
};

class TextGenerator : public iSCSIGenerator<TextRspHeader>{
public:
	/** Default Constructor */
 	TextGenerator();
	/** TargetTransferTag setter */
	void setTTT(const uint32_t ttt) { bhs->ttt = ttt; }
	/** Status Sequence Number setter */
	void setSTATSN(const uint32_t statsn) { bhs->statsn = htonl(statsn); }
	/** Expected Command Sequence Number setter */
	void setEXPCMDSN(const uint32_t expcmdsn) { bhs->expcmdsn = htonl(expcmdsn); }
	/** MAX. Command Sequence Number setter */
	void setMAXCMDSN(const uint32_t maxcmdsn) { bhs->maxcmdsn = htonl(maxcmdsn); }
	/** add Key&Value pair */
	void addKeyValue(const string& key, const string &value);
	/** PDU serializer */
	CommonBufPtr serialize();
private:
	CommonBufPtr cbuf;
	uint32_t total_dlength;
};

#endif /* __ISCSI_TEXT_H__ */
