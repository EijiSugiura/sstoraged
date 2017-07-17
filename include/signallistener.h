/**
   @file signallistener.h
   @brief Signal Listener
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: signallistener.h 312 2007-09-28 00:56:17Z sugiura $
 */
#ifndef __SIGNALLISTENER_H__
#define __SIGNALLISTENER_H__

#include "common.h"

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include <stdexcept>
#include <boost/utility.hpp>
#include <boost/thread.hpp>
#include "signal.h"

using namespace std;

/** SignalListener class */
class SignalListener : private boost::noncopyable {
	typedef boost::mutex::scoped_lock lock;
public:
	/** SignalListener instanse accessor */
	static SignalListener &getInstance() throw(std::out_of_range);
	/** SignalListener instanse clearer */
	static void clearInstance();

	/** main loop functor */
	void operator()() throw(std::runtime_error);
	/** reinit checker */
	bool checkReinit() throw();
	/** clear reinit state */
	void clearReinit() throw();
	/** fin chcker */
	bool checkFin() throw();
	/** request to exit main loop */
	void exit() throw();
private:
	/** default constructor */
	SignalListener() throw(std::runtime_error);
	/** destructor */
	~SignalListener() {}
	/** initializer */
	static void init() throw(std::out_of_range);
	/** signal listener mutex */
	boost::mutex signal_guard;
	/** signal set cache */
	sigset_t ss;
	/** reinit flag */
	bool reinit;
	/** fin flag */
	bool fin;
	/** may exit main loop flag */
	bool may_exit;
};

#endif /* __SIGNALLISTENER_H__ */
