/**
   @file portlistener.cpp
   @brief PortListener class
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: portlistener.cpp 314 2007-09-28 04:07:21Z sugiura $
 */

#include "common.h"

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include "portlistener.h"
#include "loop.h"

void PortListener::operator()() throw(runtime_error)
{
 	(*loop)();
}

