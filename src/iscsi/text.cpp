/**
   @file text.cpp
   @brief iSCSI Text handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: text.cpp 229 2007-07-23 04:44:49Z sugiura $
 */

#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include "configurator.h"
#include "logger.h"
#include "session.h"
#include "writecache.h"
#include "iscsi/text.h"
#include "iscsi/keyvalue.h"

static pair<string,string> GetKeyValuePair(const string &str)
{
	boost::regex regex("(.*)=(.*)");
	boost::smatch results;
	pair<string,string> ret;
	try {
		if(!boost::regex_search(str, results, regex))
			return ret;
		ret.first = results.str(1);
		ret.second = results.str(2);
	} catch(...) {
		return ret;
	}
	return ret;
}

TextParser::TextParser(TextReqHeader *ptr)
	: iSCSIParser<TextReqHeader>(ptr)
{
	pair_validators["SendTargets"] = PairValidatorPtr(new SendTargetsValidator);
}

TextHandler::TextHandler()
{}

bool TextParser::valid() const
{
	/** Check OpCode */
	if(getOpCode() != iSCSI_OP_TEXT_CMD){
		LOG4CXX_ERROR(logger, "Invalid OpCode");
		return false;
	}
	/** @todo : support continued TEXT request */
	if(isContinue() || !(isFinal())){
		LOG4CXX_ERROR(logger, "Continued TEXT request not supported");
		LOG4CXX_ERROR(logger, "Initiator MUST set final bit");
		return false;
	} else if(getTTT() != ISCSI_RSVD_TASK_TAG){
		LOG4CXX_ERROR(logger, "Initiator MUST set TTT to 0xffffffff with final bit on");
		return false;
	}

	/** @todo : Check SN */

	return true;
}

bool TextParser::parsePair(pair<string,string> apair)
{
	pair_validators_t::const_iterator itr = pair_validators.find(apair.first);
	if(itr == pair_validators.end()){
		LOG4CXX_ERROR(logger, "Invalid key : " + apair.first);
		return false;
	}
	if(!itr->second->valid(apair.second)){
		LOG4CXX_ERROR(logger, "Invalid " + apair.first + " : " + apair.second);
		return false;
	}
	/** Store key/value pair */
	pairs.push_back(apair);
	return true;
}

string TextParser::getPairValue(const string &attr)
{
	for(vector<pair<string,string> >::iterator itr = pairs.begin();
	    itr != pairs.end(); ++itr){
		if(itr->first == attr){
			return itr->second;
		}
	}
	string null = "";
	return null;
}

bool TextHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	LOG4CXX_DEBUG(logger, "TEXT");
	TextReqHeader *bhs = reinterpret_cast<TextReqHeader*>(parent->frontToken());
	TextParser req(bhs);
	if(!req.valid()){
		LOG4CXX_ERROR(logger, "Invalid TEXT request");
		return false;
	}

	/** @todo : handle pairs on edge! */
	char * ptr = reinterpret_cast<char *>(bhs + 1);
	int32_t dlength = req.getDlength();
	while(dlength > 0){
		string str(ptr);
		/** Parse key/value pair */
		pair<string,string> apair = GetKeyValuePair(str);
		LOG4CXX_DEBUG(logger, "Parse " + apair.first + "=" + apair.second);
		if(!req.parsePair(apair))
			return false;
		int parsed = str.length() + 1;
		dlength -= parsed;
		ptr += parsed;
	}

	/** update CMDSN */
	if(!req.isImmediateCmd())
		parent->advanceCMDSNs();

	/** generate Text reply */
 	TextGenerator reply;
	/** alway set final bit */
	reply.setFlags(reply.ISCSI_FLAG_FINAL);
	reply.setITT(req.getITT());
	/** ...then, set Reserved Task Tag, too */
	reply.setTTT(ISCSI_RSVD_TASK_TAG);
	
	/** update STATSN */
	iSCSIConnectionPtr conn = parent->connection.find(fd);
	reply.setSTATSN(conn->getSTATSN());
	conn->advanceSTATSN();

	reply.setEXPCMDSN(parent->getEXPCMDSN());
	reply.setMAXCMDSN(parent->getMAXCMDSN());

	/** handle other key/value */
	vector<pair<string,string> > attrs = req.getPairAttrs();
	if(attrs[0].first != "SendTargets")
		return false;
	SendTargetsHandler targets;
	targets.handle(parent, req.getPairValue("SendTargets"), reply);

	/** convert reply -> send task */
	parent->sendtask.push(SendTaskPtr(new SendTask(parent, fd, reply.serialize())));
	return true;
}

TextGenerator::TextGenerator()
	: total_dlength(0)
{
	cbuf = CommonBufPtr(new CommonBuf());
	bhs = reinterpret_cast<TextRspHeader*>(cbuf->cur());
	TextRspHeader tmp;
	*bhs = tmp;
	cbuf->stepForwardTail(sizeof(*bhs));
#ifdef BHS_DEBUG
	LOG4CXX_DEBUG(logger, "Reply BHS at " +
		      boost::lexical_cast<string>(bhs));
#endif
}

void TextGenerator::addKeyValue(const string& key, const string &value)
{
	string pair = key + "=" + value;
	char *ptr = reinterpret_cast<char*>(cbuf->cur());
	memcpy(ptr, pair.c_str(), pair.length());
	memset(ptr+pair.length(), 0, 1);
	cbuf->stepForwardTail(pair.length() + 1);
	total_dlength += pair.length() + 1;
	LOG4CXX_DEBUG(logger, "Added " + pair);
}

CommonBufPtr TextGenerator::serialize()
{
	/** set dlength */
	setDlength(total_dlength);
	/** padding */
	if(total_dlength%PAD_WORD_LEN)
		cbuf->stepForwardTail(PAD_WORD_LEN - total_dlength%PAD_WORD_LEN);
	return cbuf;
}
