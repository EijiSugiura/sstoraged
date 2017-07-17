/**
   @file signallistener.cpp
   @brief Signal Listener implementation
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: signallistener.cpp 145 2007-06-20 03:30:40Z sugiura $
 */

#include <signal.h>
#include <errno.h>
#include <iostream>
#include "signallistener.h"
#include "logger.h"

/** SignalListener instance cache */
SignalListener *cache = NULL;

SignalListener &SignalListener::getInstance() throw(std::out_of_range)
{
	if(cache == NULL){
		try {
			init();
		} catch(...) {
			throw std::out_of_range("Can't get SignalListener instance");
		}
	}
	return *cache;
}

void SignalListener::clearInstance()
{
	if(cache != NULL){
		delete cache;
		cache = NULL;
	}
}

SignalListener::SignalListener() throw(runtime_error)
	: reinit(false), fin(false), may_exit(false)
{
	sigemptyset(&ss);
	if(sigaddset(&ss, SIGHUP) < 0) {
		cerr << "Can't set HUP mask" << endl;
		throw runtime_error("Can't set HUP mask");
	}
	if(sigaddset(&ss, SIGINT) < 0) {
		cerr << "Can't set INT mask" << endl;
		throw runtime_error("Can't set INT mask");
	}
	if(sigaddset(&ss, SIGTERM) < 0) {
		cerr << "Can't set TERM mask" << endl;
		throw runtime_error("Can't set TERM mask");
	}
	if(pthread_sigmask(SIG_BLOCK, &ss, NULL)) {
		cerr << "Can't block signals" << endl;
		throw runtime_error("Can't block signals");
	}
}

void SignalListener::init() throw(std::out_of_range)
{
	try{
		cache = new SignalListener();
	}catch(...){
		throw std::out_of_range("Failed to construct SignalListener");
	}
}

void SignalListener::operator()() throw(runtime_error)
{
	int signo;
	NDC::push("SignalListener");
	LOG4CXX_INFO(logger, "waked up.");
	while(1){
		siginfo_t si = {};
		timespec timeout = {tv_sec : 1, tv_nsec : 0};
		signo = sigtimedwait(&ss, &si, &timeout);
		switch(signo){
		case SIGHUP:
			{
				lock lk(signal_guard);
				reinit = true;
			}
			LOG4CXX_INFO(logger, "HUP received.");
			break;

		case SIGINT:
		case SIGTERM:
			{
				lock lk(signal_guard);
				fin = true;
			}
			LOG4CXX_INFO(logger, "killed.");
			goto end;
		default:
			{
				lock lk(signal_guard);
				if(may_exit){
					LOG4CXX_INFO(logger, "exiting.");
					goto end;
				}
			}
			break;
		}
	}
 end:
	NDC::remove();
}

bool SignalListener::checkReinit() throw()
{
	lock lk(signal_guard);
	return reinit;
}

void SignalListener::clearReinit() throw()
{
	lock lk(signal_guard);
	reinit = false;
	return;
}

bool SignalListener::checkFin() throw()
{
	lock lk(signal_guard);
	return fin;
}

void SignalListener::exit() throw()
{
	lock lk(signal_guard);
	may_exit = true;
	return;
}
