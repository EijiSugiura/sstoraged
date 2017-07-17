/**
   @file login.cpp
   @brief iSCSI Login handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: login.cpp 312 2007-09-28 00:56:17Z sugiura $
 */

#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include "configurator.h"
#include "logger.h"
#include "manager.h"
#include "session.h"
#include "task.h"
#include "writecache.h"
#include "iscsi/login.h"
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

LoginParser::LoginParser(LoginReqHeader *ptr)
	: iSCSIParser<LoginReqHeader>(ptr)
{
	pair_validators["InitiatorName"] = PairValidatorPtr(new InitiatorNameValidator);
	pair_validators["InitiatorAlias"] = PairValidatorPtr(new AlwaysTrueValidator);
	pair_validators["AuthMethod"] = PairValidatorPtr(new AuthMethodValidator);
	pair_validators["SessionType"] = PairValidatorPtr(new SessionTypeValidator);
	pair_validators["TargetName"] = PairValidatorPtr(new TargetNameValidator);
	pair_validators["TargetAlias"] = PairValidatorPtr(new AlwaysTrueValidator);
	pair_validators["HeaderDigest"] = PairValidatorPtr(new DigestValidator);
	pair_validators["DataDigest"] = PairValidatorPtr(new DigestValidator);

	pair_validators["IFMarker"] = PairValidatorPtr(new BoolStringValidator);
	pair_validators["OFMarker"] = PairValidatorPtr(new BoolStringValidator);

	/** Segment length min = 512*/
	static const size_t SEGMENT_LEN_MIN = 512;
	/** Segment length max = 2**24-1*/
	static const size_t  SEGMENT_LEN_MAX = (1024*1024*16-1);
	pair_validators["MaxRecvDataSegmentLength"] =
		PairValidatorPtr(new RangeCastValidator<string,size_t,SEGMENT_LEN_MIN,SEGMENT_LEN_MAX>);
	pair_validators["DefaultTime2Wait"] =
		PairValidatorPtr(new RangeCastValidator<string,size_t,0,3600>);
	pair_validators["DefaultTime2Retain"] =
		PairValidatorPtr(new RangeCastValidator<string,size_t,0,3600>);
	pair_validators["ErrorRecoveryLevel"] =
		PairValidatorPtr(new RangeCastValidator<string,size_t,0,2>);
	pair_validators["MaxConnections"] =
		PairValidatorPtr(new RangeCastValidator<string,size_t,1,USHRT_MAX>);
	pair_validators["MaxBurstLength"] =
		PairValidatorPtr(new RangeCastValidator<string,size_t,SEGMENT_LEN_MIN,SEGMENT_LEN_MAX>);
	pair_validators["FirstBurstLength"] =
		PairValidatorPtr(new RangeCastValidator<string,size_t,SEGMENT_LEN_MIN,SEGMENT_LEN_MAX>);
	pair_validators["MaxOutstandingR2T"] =
		PairValidatorPtr(new RangeCastValidator<string,size_t,1,USHRT_MAX>);
	pair_validators["InitialR2T"] = PairValidatorPtr(new BoolStringValidator);
	pair_validators["ImmediateData"] = PairValidatorPtr(new BoolStringValidator);
	pair_validators["DataPDUInOrder"] = PairValidatorPtr(new BoolStringValidator);
	pair_validators["DataSequenceInOrder"] = PairValidatorPtr(new BoolStringValidator);

	pair_validators["X-com.cisco.PingTimeout"] = PairValidatorPtr(new AlwaysTrueValidator);
	pair_validators["X-com.cisco.sendAsyncText"] = PairValidatorPtr(new AlwaysTrueValidator);
	pair_validators["X-com.cisco.protocol"] = PairValidatorPtr(new AlwaysTrueValidator);
}

LoginHandler::LoginHandler()
	: manager(iSCSIManager::getInstance())
{
	pair_handlers["InitiatorName"] = PairHandlerPtr(new InitiatorNameHandler);
	pair_handlers["InitiatorAlias"] = PairHandlerPtr(new InitiatorAliasHandler);
	pair_handlers["AuthMethod"] = PairHandlerPtr(new AuthMethodHandler);
	pair_handlers["SessionType"] = PairHandlerPtr(new NothingTodoHandler);
	pair_handlers["TargetName"] = PairHandlerPtr(new NothingTodoHandler);
	pair_handlers["TargetAlias"] = PairHandlerPtr(new TargetAliasHandler);
	pair_handlers["HeaderDigest"] = PairHandlerPtr(new DigestHandler(static_cast<string>("HeaderDigest")));
	pair_handlers["DataDigest"] = PairHandlerPtr(new DigestHandler(static_cast<string>("DataDigest")));
	pair_handlers["MaxRecvDataSegmentLength"] = PairHandlerPtr(new MaxRecvDataSegmentLengthHandler);
	pair_handlers["DefaultTime2Wait"] =
		PairHandlerPtr(new NegotiateMaxHandler<uint32_t>(static_cast<string>("DefaultTime2Wait")));
	pair_handlers["DefaultTime2Retain"] =
		PairHandlerPtr(new NegotiateMinHandler<uint32_t>(static_cast<string>("DefaultTime2Retain")));
 	pair_handlers["ErrorRecoveryLevel"] =
		PairHandlerPtr(new NegotiateMinHandler<uint32_t>(static_cast<string>("ErrorRecoveryLevel")));
 	pair_handlers["MaxConnections"] =
		PairHandlerPtr(new NormalMinHandler<uint32_t>(static_cast<string>("MaxConnections")));
 	pair_handlers["MaxBurstLength"] =
 		PairHandlerPtr(new NormalMinHandler<uint32_t>(static_cast<string>("MaxBurstLength")));
 	/** @todo : check "Irrelevant when: ( InitialR2T=Yes and ImmediateData=No )" */
 	pair_handlers["FirstBurstLength"] =
 		PairHandlerPtr(new NormalMinHandler<uint32_t>(static_cast<string>("FirstBurstLength")));
 	pair_handlers["MaxOutstandingR2T"] =
 		PairHandlerPtr(new NormalMinHandler<uint32_t>(static_cast<string>("MaxOutstandingR2T")));
 	pair_handlers["InitialR2T"] = PairHandlerPtr(new NormalOrHandler(static_cast<string>("InitialR2T")));
 	pair_handlers["ImmediateData"] = PairHandlerPtr(new NormalAndHandler(static_cast<string>("ImmediateData")));;
 	pair_handlers["DataPDUInOrder"] = PairHandlerPtr(new NormalOrHandler(static_cast<string>("DataPDUInOrder")));
 	pair_handlers["DataSequenceInOrder"] = PairHandlerPtr(new NormalOrHandler(static_cast<string>("DataSequenceInOrder")));
 	pair_handlers["IFMarker"] = PairHandlerPtr(new NegotiateAndHandler(static_cast<string>("IFMarker")));;
 	pair_handlers["OFMarker"] = PairHandlerPtr(new NegotiateAndHandler(static_cast<string>("OFMarker")));;

	pair_handlers["X-com.cisco.PingTimeout"] = PairHandlerPtr(new NothingTodoHandler);
	pair_handlers["X-com.cisco.sendAsyncText"] = PairHandlerPtr(new NothingTodoHandler);
	pair_handlers["X-com.cisco.protocol"] = PairHandlerPtr(new NothingTodoHandler);
}

bool LoginParser::valid() const
{
	/** Check OpCode */
	if(getOpCode() != iSCSI_OP_LOGIN_CMD){
		LOG4CXX_ERROR(logger, "Invalid OpCode");
		return false;
	}
	if(!(isTransit())){
		LOG4CXX_ERROR(logger, "Initiator MUST set transit bit");
		return false;
	}
	if(isContinue()){
		LOG4CXX_ERROR(logger, "Continued LOGIN request not supported");
		return false;
	}

	/** @todo : Check SN */

	return true;
}

bool LoginParser::parsePair(pair<string,string> apair)
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

string LoginParser::getPairValue(const string &attr)
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

static uint8_t shiftLoginStage(const uint8_t csg, const uint8_t nsg)
{
	switch(csg){
	case SECURITY_NEGOTIATION_STAGE:
		if(nsg != FULL_FEATURE_PHASE && nsg != OP_PARMS_NEGOTIATION_STAGE)
			return UNKNOWN_PHASE;
		return (nsg<FULL_FEATURE_PHASE)?nsg:FULL_FEATURE_PHASE;
	case OP_PARMS_NEGOTIATION_STAGE:
		if(nsg != FULL_FEATURE_PHASE)
			return UNKNOWN_PHASE;
		return FULL_FEATURE_PHASE;
	case FULL_FEATURE_PHASE:
	default:
		return UNKNOWN_PHASE;
	}
}


bool LoginHandler::handle1stRequest(boost::shared_ptr<iSCSISession> parent,
				    const socket_t fd,
				    LoginParser &req,
				    list<boost::shared_ptr<Task> > &tasks)
{
	switch(parent->type.get()){
	case iSCSI_DISCOVERY_SESSION:
		return handle1stDiscoveryRequest(parent, fd, req, tasks);
	case iSCSI_NORMAL_SESSION:
		return handle1stNormalRequest(parent, fd, req, tasks);
	default:
		break;
	}
	return false;
}

bool LoginHandler::handle1stDiscoveryRequest(boost::shared_ptr<iSCSISession> parent,
					     const socket_t fd,
					     LoginParser &req,
					     list<boost::shared_ptr<Task> > &tasks)
{
	/** Get current connection */
	iSCSIConnectionPtr cur_conn = parent->connection.find(fd);

	/** Search Same SessionID session */
	if(manager.findSession(req.getSessionID()).get() != NULL){
		LOG4CXX_ERROR(logger, "Multiple Discovery session!");
		return false;
	}
	/** instantiate new session */
	LOG4CXX_INFO(logger, "Instantiate new discovery session");
	parent->sid.set(req.getSessionID());
	cur_conn->setCID(req.getCID());
	return true;
}

bool LoginHandler::handle1stNormalRequest(boost::shared_ptr<iSCSISession> parent,
					  const socket_t fd,
					  LoginParser &req,
					  list<boost::shared_ptr<Task> > &tasks)
{
	/** Get current connection */
	iSCSIConnectionPtr cur_conn = parent->connection.find(fd);

	/** Search Same SessionID session */
	if(manager.findSession(req.getSessionID()).get() != NULL)
		LOG4CXX_WARN(logger, "Discovery session is remain?");

	iSCSIReactorPtr reactor = manager.findReactor(req.getSessionID());

	if(reactor.get() == NULL){
		if(req.getTSIH() != 0){
			/** @todo : error return
			    session does not exist */
			LOG4CXX_ERROR(logger, "Session does not exist");
			return false;
		}
		/** instantiate new session */
		LOG4CXX_INFO(logger, "Instantiate new normal session");
		parent->sid.set(req.getSessionID());
		cur_conn->setCID(req.getCID());

		/** create new reactor thread task */
		tasks.push_back(CreateReactorTaskPtr(new CreateReactorTask(parent,fd)));
		return true;
	}

	iSCSISessionPtr old_session = reactor->getSession();
	if(req.getTSIH() == 0){
		/** session reinstatement */
		LOG4CXX_INFO(logger, "Session reinstatement");

		/** stop old reactor task */
		tasks.push_back(RemoveReactorTaskPtr(new RemoveReactorTask(old_session)));
		parent->sid.set(req.getSessionID());
		cur_conn->setCID(req.getCID());

		/** create new reactor thread task */
		tasks.push_back(CreateReactorTaskPtr(new CreateReactorTask(parent,fd)));
		return true;
	}

	if(req.getTSIH() != old_session->tsih.get()){
		/** @todo : error return
		    session dose not exist */
		LOG4CXX_ERROR(logger, "Session does not exist");
		return false;
	}

	/** reuse old session/reactor */
	iSCSIConnectionPtr old_conn = old_session->connection.find(req.getCID());
	if(old_conn.get() == NULL){
		/** add a new connection to the session */
		cur_conn->changeListener(reactor);
		old_session->connection.add(cur_conn);
		LOG4CXX_INFO(logger, "Add a new connection to the session");
	} else {
		/** RFC3720 10.12.9
		    ExpStatSN is valid only Login Requests restart connection */
		if(req.getEXPSTATSN() != old_conn->getSTATSN()){
			LOG4CXX_ERROR(logger, "Invalid ExpStatSN");
			return false;
		}
		// copy STATSN
		cur_conn->setSTATSN(old_conn->getSTATSN());
		/** connection reinstatement */
		old_session->connection.del(old_conn);
		cur_conn->changeListener(reactor);
		old_session->connection.add(cur_conn);
		LOG4CXX_INFO(logger, "Connection reinstatement");
	}
	/** append task that closes current session */
	tasks.push_back(CloseSessionTaskPtr(new CloseSessionTask(parent)));

	parent->sid.set(req.getSessionID());
	cur_conn->setCID(req.getCID());
	return true;
}

bool LoginHandler::handle(iSCSISessionPtr parent, const socket_t fd)
{
	LOG4CXX_DEBUG(logger, "LOGIN");
	LoginReqHeader *bhs = reinterpret_cast<LoginReqHeader*>(parent->frontToken());
	LoginParser req(bhs);
	if(!req.valid()){
		LOG4CXX_ERROR(logger, "Invalid LOGIN request");
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

	/** 1st Request? */
	list<TaskPtr> tasks;
	if(parent->type.get() == iSCSI_LOGIN_SESSION){
		/** set SessionType first... */
		parent->type.set(req.getPairValue("SessionType"));
		LOG4CXX_DEBUG(logger, "SessionType : " + req.getPairValue("SessionType"));
		if(!this->handle1stRequest(parent, fd, req, tasks))
			return false;
	}

	/** get next stage */
	uint8_t nsg = shiftLoginStage(req.currentStage(), req.nextStage());

	/** update CMDSN */
	if(!req.isImmediateCmd())
		parent->advanceCMDSNs();

	/** generate Login reply */
 	LoginGenerator reply;
	/** CAUTION!!! Always set TRANSIT bit
	    @todo : support other authentications
	 */
	reply.setFlags((req.currentStage()<<2)|nsg);
	reply.setITT(req.getITT());
	
	/** update STATSN */
	iSCSIConnectionPtr conn = parent->connection.find(fd);
	reply.setSTATSN(conn->getSTATSN());
	conn->advanceSTATSN();

	reply.setEXPCMDSN(parent->getEXPCMDSN());
	reply.setMAXCMDSN(parent->getMAXCMDSN());
	reply.setSessionID(parent->sid.get());
	reply.setStatusClass(STATUS_CLS_SUCCESS);
	reply.setStatusDetail(LOGIN_STATUS_ACCEPT);

	/** handle other key/value */
	vector<pair<string,string> > attrs = req.getPairAttrs();
	for(vector<pair<string,string> >::iterator itr = attrs.begin();
	    itr != attrs.end(); ++itr){
		/** ... and add data segment */
		if(!pair_handlers[itr->first]->handle(parent, req.getPairValue(itr->first), reply))
			return false;
	}

	/** status shift
	    @todo : move this section to status shift module
	 */
	switch(nsg){
	case SECURITY_NEGOTIATION_STAGE:
	case OP_PARMS_NEGOTIATION_STAGE:
		break;
	case FULL_FEATURE_PHASE:
		LOG4CXX_DEBUG(logger, "Transit to full feature phase");
		if(parent->tsih.get() == 0)
			parent->tsih.set(manager.getNextTSIH());
		switch(parent->type.get()){
		case iSCSI_DISCOVERY_SESSION:
			/** change session handler to full feature's one */
			parent->shift2Discovery();
			break;
		case iSCSI_NORMAL_SESSION:
			/** change session handler to full feature's one */
			parent->shift2Normal();
			break;
		default:
			LOG4CXX_ERROR(logger, "Invalid SessionType");
			return false;
		}
		break;
	default:
		LOG4CXX_ERROR(logger, "Unknown LOGIN stage shift");
		return false;
	}

	/** convert reply -> send task */
	tasks.push_front(SendTaskPtr(new SendTask(parent, fd, reply.serialize())));
	parent->sendtask.push(tasks);
	return true;
}

LoginGenerator::LoginGenerator()
	: total_dlength(0)
{
	cbuf = CommonBufPtr(new CommonBuf());
	bhs = reinterpret_cast<LoginRspHeader*>(cbuf->cur());
	LoginRspHeader tmp;
	*bhs = tmp;
	cbuf->stepForwardTail(sizeof(*bhs));
#ifdef BHS_DEBUG
	LOG4CXX_DEBUG(logger, "Reply BHS at " +
		      boost::lexical_cast<string>(bhs));
#endif
}

void LoginGenerator::addKeyValue(const string& key, const string &value)
{
	string pair = key + "=" + value;
	char *ptr = reinterpret_cast<char*>(cbuf->cur());
	memcpy(ptr, pair.c_str(), pair.length());
	memset(ptr+pair.length(), 0, 1);
	cbuf->stepForwardTail(pair.length() + 1);
	total_dlength += pair.length() + 1;
	LOG4CXX_DEBUG(logger, "Added " + pair);
}

CommonBufPtr LoginGenerator::serialize()
{
	/** set dlength */
	setDlength(total_dlength);
	/** padding */
	if(total_dlength%PAD_WORD_LEN)
		cbuf->stepForwardTail(PAD_WORD_LEN - total_dlength%PAD_WORD_LEN);
	return cbuf;
}
