/**
   @file acceptor.h
   @brief Port Acceptor class
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: acceptor.h 42 2007-04-17 03:28:02Z sugiura $
 */
#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include <stdexcept>
#include "common.h"

using namespace std;

/** abstruct Acceptor class */
class Acceptor {
public:
	/** default construcor */
	Acceptor() throw() {}
	/** destructor */
	virtual ~Acceptor() {}
	/** accept handler */
	virtual status_t accept(const socket_t fd) = 0;
protected:
	/** call accept() 
	    @param fd : listen file descriptor
	    @param addr : peer address
	 */
	socket_t do_accept(const socket_t fd, sockaddr_storage &addr);
};

/** iSCSI Acceptor class */
class iSCSIAcceptor : public Acceptor {
public:
	/** accept handler */
	status_t accept(const socket_t fd);
protected:
	using Acceptor::do_accept;
};

#endif /* __ACCEPTOR_H__ */
