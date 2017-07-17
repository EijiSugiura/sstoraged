/**
   @file manager.cpp
   @brief iSCSI Manager class
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: manager.cpp 312 2007-09-28 00:56:17Z sugiura $
 */

#include <iostream>
#include <fstream>
#include <boost/thread/once.hpp>
#include "common.h"
#include "portlistener.h"
#include "manager.h"

iSCSIManager *iSCSIManager::cache = NULL;

iSCSIManager::iSCSIManager() throw() : listener()
{}

iSCSIManager &iSCSIManager::getInstance() throw(std::out_of_range)
{
	try {
		static boost::once_flag flg = BOOST_ONCE_INIT;
		boost::call_once(init, flg);
	} catch(...) {
		throw std::out_of_range("Can't get iSCSIManager instance");
	}
	return *cache;
}

void iSCSIManager::init() throw(std::out_of_range)
{
	try{
		cache = new iSCSIManager();
	}catch(...){
		throw std::out_of_range("Failed to construct iSCSIManager");
	}
}

bool iSCSIManager::addSession(iSCSISessionPtr &session) throw()
{
	iSCSIConnectionPtr conn = session->connection.getInitial();
	if(conn.get() == NULL)
		return false;
	socket_t fd = conn->getSocket();
	/** store session */
	session_map_t::iterator itr = sessions.find(fd);
	if(itr != sessions.end()){
		LOG4CXX_ERROR(logger, "Session already exists : Type = " +
			      session->type.getStr());
		return false;
	}
	sessions[fd] = session;
	LOG4CXX_DEBUG(logger, "Add session : " +
		      boost::lexical_cast<string>(itr->second.get()) +
		      " Type = " + session->type.getStr());
	return true;
}
bool iSCSIManager::delSession(const iSCSISession *session) throw()
{
	/** search session */
	session_map_t::iterator itr = sessions.begin();
	for(;
	    itr != sessions.end(); ++itr){
		if(itr->second.get() == session)
			break;
	}
	if(itr == sessions.end())
		return false;

	LOG4CXX_DEBUG(logger, "Delete session : " +
		      boost::lexical_cast<string>(itr->second.get()) +
		      " Type = " + itr->second->type.getStr());

	sessions.erase(itr);
	return true;
}
iSCSISessionPtr iSCSIManager::findSession(const iSCSISession *session) throw()
{
	/** search session */
	session_map_t::iterator itr = sessions.begin();
	for(;
	    itr != sessions.end(); ++itr){
		if(itr->second.get() == session)
			break;
	}
	if(itr == sessions.end()){
		iSCSISessionPtr null_ptr;
		return null_ptr;
	}

	LOG4CXX_DEBUG(logger, "Find session : " +
		      boost::lexical_cast<string>(itr->second.get()) +
		      " Type = " + itr->second->type.getStr());

	return itr->second;
}
iSCSISessionPtr iSCSIManager::findSession(const SessionID &sid) throw()
{
	/** search session */
	session_map_t::iterator itr = sessions.begin();
	for(;
	    itr != sessions.end(); ++itr){
		if(itr->second->sid.get() == sid)
			break;
	}
	if(itr == sessions.end()){
		iSCSISessionPtr null_ptr;
		return null_ptr;
	}

	LOG4CXX_DEBUG(logger, "Find session : Type = " +
		      itr->second->type.getStr());

	return itr->second;
}

bool iSCSIManager::addReactor(iSCSIReactorPtr reactor) throw()
{
	SessionID sid = reactor->getSessionID();
	for(reactor_list_t::iterator itr = reactors.begin();
	    itr != reactors.end(); ++itr){
		iSCSIReactorPtr tmp = *itr;
		if(tmp->getSessionID() == sid){
			LOG4CXX_ERROR(logger, "Same session already exists");
			return false;
		}
	}
	reactors.push_front(reactor);
	LOG4CXX_DEBUG(logger, "Add reactor : " +
		      boost::lexical_cast<string>(reactor.get()));

	/**   create new thread */
	boost::thread *thr = threads.create_thread(boost::ref(*reactor));
	reactor->setThread(thr);
	return true;
}

bool iSCSIManager::delReactor(iSCSIReactorPtr key) throw()
{
	bool ret = true;
	for(reactor_list_t::iterator itr = reactors.begin();
	    itr != reactors.end(); ++itr){
		if(key.get() == itr->get()){
			LOG4CXX_DEBUG(logger, "Delete reactor : " +
				      boost::lexical_cast<string>(key.get()));
			goto found;
		}
	}
	LOG4CXX_ERROR(logger, "Delete reactor : Internal error");
	ret = false;
 found:
 	boost::thread *thr = key->getThread();
	threads.remove_thread(thr);
	delete thr;
	reactors.erase(remove(reactors.begin(),reactors.end(), key), reactors.end());
	return ret;
}

iSCSIReactorPtr iSCSIManager::findReactor(const SessionID &sid) throw()
{
	for(reactor_list_t::iterator itr = reactors.begin();
	    itr != reactors.end(); ++itr){
		iSCSIReactorPtr tmp = *itr;
		if(tmp->getSessionID() == sid){
			LOG4CXX_DEBUG(logger, "Find reactor : " +
				      boost::lexical_cast<string>(tmp.get()));
			return tmp;
		}
	}
	iSCSIReactorPtr null;
	return null;
}

