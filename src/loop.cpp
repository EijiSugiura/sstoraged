/**
   @file loop.cpp
   @brief Event Multiplexor loop implementations
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: loop.cpp 312 2007-09-28 00:56:17Z sugiura $
 */

#include <errno.h>
#include "logger.h"
#include "signallistener.h"
#include "loop.h"

SelectLoop::SelectLoop() : nfds(INVALID_SOCKET)
{
	FD_ZERO(&readfdcache);
	FD_ZERO(&writefdcache);
}

SelectLoop::SelectLoop(PortPtr aport) throw(std::runtime_error)
	: nfds(INVALID_SOCKET)
{
	FD_ZERO(&readfdcache);
	FD_ZERO(&writefdcache);
	addPort(aport);
}

SelectLoop::~SelectLoop() {}

void SelectLoop::operator()() throw (runtime_error)
{
	SignalListener &signal = SignalListener::getInstance();
	LOG4CXX_INFO(logger, "entering loop.");
	while(!stop){
		if(signal.checkFin())
			break;
		if(signal.checkReinit()){
			/** @todo : add re-initialize routine */
			signal.clearReinit();
		}
		fd_set readfds = readfdcache;
		fd_set writefds = writefdcache;
		timeval timeout = {tv_sec : 1, tv_usec : 0};
		int events = select(nfds+1, &readfds, &writefds, NULL, &timeout);
		if(events < 0){
			if(errno == EINTR)
				continue;
			else
				break;
		}
		vector<PortPtr>::iterator itr = ports.begin();
		vector<PortPtr>::iterator enditr = ports.end();
		for(;itr != enditr; ++itr){
			socket_t fd = (*itr)->getSocket();
			if(FD_ISSET(fd, &readfds)){
				events--;
				status_t ret = (*itr)->onReceive(fd);
				if(ret <= 0)
					break;
			}
			if(FD_ISSET(fd, &writefds)){
				events--;
				if((*itr)->onSend(fd) < 0){
					LOG4CXX_ERROR(logger, "failed to send");
					break;
				}
			}
		}
		if(events != 0)
			LOG4CXX_WARN(logger, "events remain");
	}
	LOG4CXX_INFO(logger, "exiting loop.");
	return;
}
