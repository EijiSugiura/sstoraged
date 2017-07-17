/**
   @file reactor.h
   @brief Reactor handling classes
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: reactor.h 312 2007-09-28 00:56:17Z sugiura $
 */
#ifndef __REACTOR_H__
#define __REACTOR_H__

#include "common.h"

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include <stdexcept>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/thread/thread.hpp>
#include "portlistener.h"
#include "session.h"
#include "iscsi/protocol.h"

using namespace std;

class DiskWriter;

/** iSCSI Reactor class */
class iSCSIReactor : public PortListener {
public:
	/** constructor
	    @param _session : the iSCSI session
	 */
	iSCSIReactor(const iSCSISessionPtr _session);

	/** destructor */
	~iSCSIReactor();

	/** main loop of reactor */
	void operator()() throw(std::runtime_error);

	/** reactor's iSCSI stream receiver
	    @param fd : socket descriptor to receive
	 */
	status_t recv(const socket_t fd);

	/** Session ID getter */
	SessionID getSessionID() const { return session->sid.get(); }
	/** session getter */
	iSCSISessionPtr getSession() const { return session; }

	/** May the reactor thread stop */
	void mayStop();

	/** thread info. setter
	    @param thr : the thread info
	 */
	void setThread(boost::thread *thr) { thread = thr; }

	/** thread info. getter
	    @return thread info
	*/
	boost::thread *getThread() { return thread; }

private:
	/** setup disk writer */
	bool setupWriters();
  	/** clear exists disk writer */
  	void clearWriters();

	/** the iSCSI session */
	iSCSISessionPtr session;
	/** myown thread info */
	boost::thread *thread;

	/** disk writer thread pool */
	boost::thread_group wthreads;
};

typedef boost::shared_ptr<iSCSIReactor> iSCSIReactorPtr;

#endif /* __REACTOR_H__ */
