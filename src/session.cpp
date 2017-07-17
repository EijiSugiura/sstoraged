/**
   @file session.cpp
   @brief iSCSI Session class
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: session.cpp 284 2007-08-15 08:36:45Z sugiura $
 */

#include <errno.h>
#include <sys/uio.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include "common.h"
#include "logger.h"
#include "configurator.h"
#include "manager.h"
#include "session.h"
#include "writecache.h"
#include "ioveccontainer.h"
#include "limitter.h"
#include "iscsi/protocol.h"


/** @todo : FIX : Target Portal Group Tag is always 1 */
iSCSISession::iSCSISession()
	: type(), tag(), sid(), tsih(), sendtask(), recvinfo(),
	  connection(), manager(iSCSIManager::getInstance()),
	  expcmdsn_gen(), maxcmdsn_gen(),
	  tokens()
{
	LOG4CXX_DEBUG(logger, "Session Constructed : " +
		      boost::lexical_cast<string>(this));
	Configurator &cur_conf = Configurator::getInstance();
	/** load configurations */
	uint64_t num;
	cur_conf.getAttr("DefaultTime2Wait",num);
	setAttr("DefaultTime2Wait", static_cast<uint32_t>(num));

	cur_conf.getAttr("DefaultTime2Retain",num);
	setAttr("DefaultTime2Retain", static_cast<uint32_t>(num));

	cur_conf.getAttr("MaxBurstLength",num);
	setAttr("MaxBurstLength", static_cast<uint32_t>(num));

	cur_conf.getAttr("FirstBurstLength",num);
	setAttr("FirstBurstLength", static_cast<uint32_t>(num));

	cur_conf.getAttr("MaxConnections",num);
	setAttr("MaxConnections", static_cast<uint32_t>(num));

	cur_conf.getAttr("MaxOutStandingR2T",num);
	setAttr("MaxOutstandingR2T", static_cast<uint32_t>(num));

	cur_conf.getAttr("ErrorRecoveryLevel",num);
	setAttr("ErrorRecoveryLevel", static_cast<uint32_t>(num));

	cur_conf.getAttr("MaxRecvDataSegmentLength", num);
	setAttr("MaxRecvDataSegmentLength", static_cast<uint32_t>(num));
	mrds_length = num;

	bool flag;
	cur_conf.getAttr("InitialR2T", flag);
	setAttr("InitialR2T", flag);

	cur_conf.getAttr("ImmediateData", flag);	
	setAttr("ImmediateData", flag);

	cur_conf.getAttr("IFMarker", flag);	
	setAttr("IFMarker", flag);

	cur_conf.getAttr("OFMarker", flag);	
	setAttr("OFMarker", flag);

	cur_conf.getAttr("DataPDUInOrder", flag);
	setAttr("DataPDUInOrder", flag);

	cur_conf.getAttr("DataSequenceInOrder", flag);
	setAttr("DataSequenceInOrder", flag);

	/** set LOGIN handler only */
	protos[iSCSI_OP_LOGIN_CMD] = iSCSIHandlerPtr(new LoginHandler);

	/** init SN */
	expcmdsn_gen.set(1);
	maxcmdsn_gen.set(2);
}

iSCSISession::~iSCSISession()
{
	LOG4CXX_DEBUG(logger, "Session Destructed : " +
		      boost::lexical_cast<string>(this));
}


bool iSCSISession::shift2Discovery()
{
	if(type.get() != iSCSI_DISCOVERY_SESSION){
		LOG4CXX_ERROR(logger, "Session type mismatch");
		return false;
	}
	protos[iSCSI_OP_TEXT_CMD] = iSCSIHandlerPtr(new TextHandler);
	protos[iSCSI_OP_LOGOUT_CMD] = iSCSIHandlerPtr(new LogoutHandler);
	LOG4CXX_DEBUG(logger, "Shift to Discovery session");
	return true;
}

bool iSCSISession::shift2Normal()
{
	if(type.get() != iSCSI_NORMAL_SESSION){
		LOG4CXX_ERROR(logger, "Session type mismatch");
		return false;
	}
	protos[iSCSI_OP_SCSI_CMD] = iSCSIHandlerPtr(new CommandHandler(this));
	protos[iSCSI_OP_SCSI_DATA] = iSCSIHandlerPtr(new DataOutHandler(this));
	protos[iSCSI_OP_NOOP_OUT] = iSCSIHandlerPtr(new NopOutHandler);
	protos[iSCSI_OP_NOOP_IN] = iSCSIHandlerPtr(new NopInHandler);
	protos[iSCSI_OP_TASK_MGT_REQ] = iSCSIHandlerPtr(new TaskMngHandler);
	protos[iSCSI_OP_LOGOUT_CMD] = iSCSIHandlerPtr(new LogoutHandler);
	
	/** change SN buffer size */
	uint32_t current_maxcmdsn = maxcmdsn_gen.cur();
	maxcmdsn_gen.set(current_maxcmdsn + CMD_QUEUE_SIZE);

	LOG4CXX_DEBUG(logger, "Shift to Normal session");
	return true;
}

status_t iSCSISession::doRecv(iSCSISessionPtr session, const socket_t fd)
{
	IovecContainer & recvbuf = connection.find(fd)->getRecvBuf();
	recvbuf.reserve(mrds_length);
	size_t total = 0;
	while(1){
		ssize_t size = readv(fd, recvbuf.refVec(), recvbuf.getCount());
		if(size > 0){
#ifdef TOKENIZER_DEBUG
			LOG4CXX_DEBUG(logger, "Recv : " 
				      + boost::lexical_cast<string>(size));
#endif
			/** tokenize */
			tokens.push(&recvbuf, size);
			total += size;

			status_t ret = this->doWork(session, fd);
			if(ret < 0)
				return ret;
			continue;
		} else if(size == 0) {
			LOG4CXX_DEBUG(logger, "Peer closed [" 
				      + boost::lexical_cast<string>(fd) + "]");
			return 0;
		}
		if(errno == EAGAIN)
			break;
		LOG4CXX_ERROR(logger, "Failed to receive : " +
			      static_cast<string>(strerror(errno)));
		return  -errno;
	}
	return total;
}

status_t iSCSISession::cleanUp(const socket_t fd)
{
	if(connection.count() <= 1){
		/** the last connection was lost, then remove session/reactor */
		if(!manager.delSession(this)){
			iSCSIReactorPtr reactor = manager.findReactor(this->sid.get());
			if(reactor.get() == NULL){
				LOG4CXX_ERROR(logger, "Not found session as iSCSIReactor " +
					      boost::lexical_cast<string>(this));
				return 0;
			}
			reactor->mayStop();
		}
		return 0;
	}
	/** Close a connection */
	connection.del(fd);
	return 0;
}

status_t iSCSISession::recv(const socket_t fd)
{
	iSCSISessionPtr session = manager.findSession(this);

	if(this->doRecv(session, fd) <= 0)
		return this->cleanUp(fd);

	/** anyway, send it out */
	if(!sendtask.invoke())
		return -EIO;
	return 1;
}

status_t iSCSISession::doWork(iSCSISessionPtr session, const socket_t fd)
{
	while(!tokens.empty()){
		/** handle iSCSI protocols */
		iSCSIParser<iSCSIHeader> header(reinterpret_cast<iSCSIHeader*>(tokens.frontHeader()));
		handler_map_t::iterator proto = protos.find(header.getOpCode());
		if(proto == protos.end()){
			ostringstream os;
			os << std::hex << std::setfill('0') << std::setw(2)
			   << (unsigned int)header.getOpCode();
			LOG4CXX_ERROR(logger, "Not suppoted OpCode = " + os.str());
			this->cleanUp(fd);
			return -ESRCH;
		}
		if(!proto->second->handle(session, fd)){
			LOG4CXX_ERROR(logger, "Failed to handle request");
			this->cleanUp(fd);
			return -EIO;
		}
		tokens.pop();
	}
	return 0;
}

status_t iSCSISession::recv(iSCSISessionPtr session, const socket_t fd)
{
	if(this->doRecv(session, fd) <= 0)
		return this->cleanUp(fd);

	/** anyway, send it out */
	if(!sendtask.invoke())
		return -EIO;
	return 1;
}

TaskPtr iSCSISession::SessionSendTask::pop()
{
	TaskPtr task = tasks.front();
	tasks.pop_front();
#if 0
	LOG4CXX_DEBUG(logger, "Task pop : " +
		      boost::lexical_cast<string>(task.get()));
#endif
	return task;
}

bool iSCSISession::SessionSendTask::invoke()
{
	try {
		while(!tasks.empty()){
			TaskPtr task = tasks.front();
			if(!task->run())
				break; // try again later...
			this->pop(); // well done...
		}
	} catch (const exception &e){
		/** clear all remain tasks */
		LOG4CXX_ERROR(logger, e.what());
		tasks.clear();
		return false;
	}
	return true;
}


status_t iSCSISession::send(const socket_t fd)
{
	/** @todo : check if 1st task is SendTask */

	if(!sendtask.invoke())
		return -EIO;
	return 0;
}

string iSCSISession::SessionType::getStr() const
{
	switch(type){
	case iSCSI_DISCOVERY_SESSION:
		return "Discovery";
	case iSCSI_NORMAL_SESSION:
		return "Normal";
	default:
		break;
	}
	return "Login";
}
void iSCSISession::SessionType::set(const string& str)
{
	if(str == static_cast<string>("Discovery"))
		type = iSCSI_DISCOVERY_SESSION;
	else if(str == static_cast<string>("Normal"))
		type = iSCSI_NORMAL_SESSION;
	else
		type = iSCSI_LOGIN_SESSION;
}

bool iSCSISession::Connection::add(const iSCSIConnectionPtr &conn) throw()
{
	iSCSIConnectionPtr old = find(conn->getSocket());
	if(old.get() != NULL)
		return false;
	ports[conn->getSocket()] = conn;
	LOG4CXX_DEBUG(logger, "Connection added : FD = " 
		      + boost::lexical_cast<string>(conn->getSocket()));
	return true;
}

bool iSCSISession::Connection::del(const socket_t fd) throw()
{
	conn_map_t::iterator itr = ports.find(fd);
	if(itr == ports.end()){
		LOG4CXX_DEBUG(logger, "Connection not found : FD = " 
			      + boost::lexical_cast<string>(fd));
		return false;
	}
	LOG4CXX_DEBUG(logger, "Connection deleted :  FD = " 
		      + boost::lexical_cast<string>(itr->second->getSocket()));
	ports.erase(itr);
	return true;
}

bool iSCSISession::Connection::del(const iSCSIConnectionPtr &conn) throw()
{
	return del(conn->getSocket());
}

iSCSIConnectionPtr iSCSISession::Connection::find(const socket_t fd) const
{
	conn_map_t::const_iterator itr = ports.find(fd);
	if(itr == ports.end()){
		LOG4CXX_DEBUG(logger, "Connection not found : FD = " 
			      + boost::lexical_cast<string>(fd));
		iSCSIConnectionPtr null;
		return null;
	}
	return itr->second;
}

iSCSIConnectionPtr iSCSISession::Connection::getInitial() const
{
	if(ports.size() != 1){
		LOG4CXX_ERROR(logger, "Internal error with initial connection"); 
		iSCSIConnectionPtr null;
		return null;
	}
	return ports.begin()->second;
}

void iSCSISession::Connection::clearListener()
{
	for(conn_map_t::iterator itr = ports.begin();
	    itr != ports.end(); ++itr){
		itr->second->clearListener();
	}
}

WriteCachePtr iSCSISession::findLargestWriteCache()
{
	WriteCachePtr ret;
	size_t maxsize = 0;
	vector<WriteCachePtr>::iterator end = wcaches.end();
	for(vector<WriteCachePtr>::iterator itr = wcaches.begin();
	    itr != end; ++itr){
		size_t size = (*itr)->size();
		if(maxsize < size){
			ret = *itr;
			maxsize = size;
		}
	}
	return ret;
}

void iSCSISession::advanceCMDSNs()
{
	expcmdsn_gen();
	size_t poolnum = CommonBuf::getPoolSize();
	LimitterPtr limitter = CommonBuf::getLimitter();
	size_t queuesize_wil = (type.get()==iSCSI_DISCOVERY_SESSION)?
		1:(limitter->getY(poolnum));
	while(getCMDQueue() < queuesize_wil){
		maxcmdsn_gen();
	}
	/** Pool status transition */
	if(poolnum > limitter->getX2()){
		/** @todo : wait writer's work */
		WriteCachePtr wc = this->findLargestWriteCache();
		if(wc.get()){
			LOG4CXX_DEBUG(logger, "Write remain : "
				      + boost::lexical_cast<string>(wc->size()));
			wc->waitTillDone();
		}
		CommonBuf::setPoolState(POOL_STATE_HARD);
	} else if(poolnum > limitter->getX1()){
		CommonBuf::setPoolState(POOL_STATE_SOFT);
	} else {
		CommonBuf::setPoolState(POOL_STATE_NORMAL);
	}
	LOG4CXX_DEBUG(logger, "CMDSN QueueSize[" +
		      boost::lexical_cast<string>(getCMDQueue()) +
		      "] : PoolSize " +
		      boost::lexical_cast<string>(poolnum) + "/" +
		      boost::lexical_cast<string>(limitter->getX1()) + "/" +
		      boost::lexical_cast<string>(limitter->getX2()));
}
