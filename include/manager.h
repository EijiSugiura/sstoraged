/**
   @file manager.h
   @brief iSCSI Manager class
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: manager.h 312 2007-09-28 00:56:17Z sugiura $
 */
#ifndef __MANAGER_H__
#define __MANAGER_H__

#include "common.h"

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include <stdexcept>
#include <list>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include "port.h"
#include "session.h"
#include "reactor.h"
#include "iscsi/protocol.h"
#include "counter.h"

using namespace std;

class PortListener;

/** iSCSI Manager class */
class iSCSIManager {
public:
	/** singleton instance getter */
	static iSCSIManager &getInstance() throw(std::out_of_range);
	/** destructor */
	virtual ~iSCSIManager() throw()	{}

	/** port listener setter
	    @param aListener : the iSCSI port listener
	 */
	void setListener(boost::shared_ptr<PortListener> aListener)
	{ listener = aListener; }
	/** port listener getter */
	boost::shared_ptr<PortListener> getListener() const { return listener; }
	/** port listener clearer */
	void clearListener()
	{
		boost::shared_ptr<PortListener> null;
		listener = null;
	}

	/** add new session
	    @param session : the new session to add
	 */
	bool addSession(iSCSISessionPtr &session) throw();
	/** delete exists session
	    @param session : an address of exists session
	*/
	bool delSession(const iSCSISession *session) throw();
	/** find exists session
	    @param session : an address of exists session
	 */
	iSCSISessionPtr findSession(const iSCSISession *session) throw();
	/** find exists session
	    @param sid : Session ID
	 */
	iSCSISessionPtr findSession(const SessionID &sid) throw();

	/** add new reactor 
	    @param reactor : new iSCSI reactor
	 */
	bool addReactor(iSCSIReactorPtr reactor) throw();
	/** delete exists reactor
	    @param reactor : exists iSCSI reactor
	 */
	bool delReactor(iSCSIReactorPtr reactor) throw();
	/** find exists reactor
	    @param SessionID : exists Session ID
	 */ 
	iSCSIReactorPtr findReactor(const SessionID &sid) throw();

	/** get next TSIH */
	uint32_t getNextTSIH() { return tsih_gen(); }
private:
	/** default constructor */
	iSCSIManager() throw();
	/** initializer */
	static void init() throw(std::out_of_range);
	/** instanse cache */
	static iSCSIManager *cache;

	/** port listener instance */
        boost::shared_ptr<PortListener> listener;

	/** initial/discovery session pool */
	typedef map<socket_t,iSCSISessionPtr> session_map_t;
 	session_map_t sessions;

	/** reactors = normal session pool */
	typedef list<iSCSIReactorPtr> reactor_list_t;
 	reactor_list_t reactors;
	/** reactor thread pool */
	boost::thread_group threads;

	/** TSIH generator */
	IncrementalCounter<uint32_t,1,ULONG_MAX> tsih_gen;

};

#endif /* __MANAGER_H__ */
