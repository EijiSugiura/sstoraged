/**
   @file inetutils.h
   @brief INET utilities class
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: inetutils.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __INETUTILS_H__
#define __INETUTILS_H__

#include "common.h"

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_WINSOCK2_H
#include <ws2tcpip.h>
#endif

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <boost/regex.hpp>

using namespace std;

/** Inet Utility class */
class InetUtil {
public:
	InetUtil() {}
	virtual ~InetUtil() {}
	/** sockaddr to "[address]:port" string converter
	    @param addr : sockaddr address to convert
	    @return converted [ip]:port formatted string
	*/
	static string getAddrPortStr(const sockaddr_storage &addr) throw(std::runtime_error)
	{
		ostringstream os;
		char host[INET6_ADDRSTRLEN] = { 0 }, port[8] = { 0 };
		if(status_t ret = getnameinfo((struct sockaddr*)&addr, sizeof(addr),
					      host, sizeof(host), port, sizeof(port),
					      NI_NUMERICHOST|NI_NUMERICSERV)){
			throw std::runtime_error(gai_strerror(ret));
		}
		try {
			os << "[" << (char*)host << "]:" << (char*)port;
		} catch(...) {
			throw std::runtime_error("Unknown error");
		}
		if(strlen(port) < 1 || strncmp(port, "0", 1) == 0)
			throw std::runtime_error("Invalid port specified");
		return os.str();
	}
	/** sockaddr to "address:port" string converter
	    @param addr : sockaddr address to convert
	    @return converted [ip]:port formatted string
	*/
	static string getAddrPortStr2(const sockaddr_storage &addr) throw(std::runtime_error)
	{
		ostringstream os;
		char host[INET6_ADDRSTRLEN] = { 0 }, port[8] = { 0 };
		if(status_t ret = getnameinfo((struct sockaddr*)&addr, sizeof(addr),
					      host, sizeof(host), port, sizeof(port),
					      NI_NUMERICHOST|NI_NUMERICSERV)){
			throw std::runtime_error(gai_strerror(ret));
		}
		try {
			os << (char*)host << ":" << (char*)port;
		} catch(...) {
			throw std::runtime_error("Unknown error");
		}
		if(strlen(port) < 1 || strncmp(port, "0", 1) == 0)
			throw std::runtime_error("Invalid port specified");
		return os.str();
	}
	/** sockaddr's address to string converter
	    @param addr : sockaddr address to convert
	    @return converted [ip]:port formatted string
	*/
	static string getAddrStr(const sockaddr_storage &addr) throw(std::runtime_error)
	{
		char host[INET6_ADDRSTRLEN] = { 0 };
		if(status_t ret = getnameinfo((struct sockaddr*)&addr, sizeof(addr),
					      host, sizeof(host), NULL, 0,
					      NI_NUMERICHOST)){
			throw std::runtime_error(gai_strerror(ret));
		}
		return static_cast<string>(host);
	}
	/** string to sockaddr converter
	    @param str : [ip]:port formatted string
	    @param addr : converted sockaddr address
	    @return true : success
	    @return false : fail
	 */
	static bool getSockAddr(const string & str, sockaddr_storage &addr) throw()
	{
		boost::regex regex("\\[(.*)\\].*:([0-9]+)");
		boost::smatch results;
		try {
			if(!boost::regex_search(str, results, regex))
				return false;
			struct addrinfo *info = NULL;
			status_t ret = getaddrinfo(results.str(1).c_str(),
						   results.str(2).c_str(), NULL, &info);
			if(ret)
				return false;
			/** store 1st addrinfo */
			memcpy(&addr, info->ai_addr, info->ai_addrlen);
			if(info)
				freeaddrinfo(info);
		} catch(...) {
			return false;
		}
		return true;
	}
	/** Any Address checker
	    @param str : address string to check
	    @return true : address is Any Address
	    @return false : address is Not Any Address
	*/
	static bool isAnyAddr(const string &str) throw()
	{
		union {
			sockaddr_in 		s4;
			sockaddr_in6 		s6;
			sockaddr_storage 	ss;
		} addr = {};
		try {
			if(!getSockAddr(str, addr.ss))
				return false;
		} catch(...) {
			return false;
		}
		switch(addr.ss.ss_family){
		case AF_INET:
			if(addr.s4.sin_addr.s_addr == INADDR_ANY)
				return true;
			break;
		case AF_INET6:
			if(IN6_IS_ADDR_UNSPECIFIED(&addr.s6.sin6_addr))
				return true;
			break;
		default:
			break;
		}
		return false;
	}
	/** Loopback Address checker
	    @param str : address string to check
	    @return true : address is Loopback Address
	    @return false : address is Not Loopback Address
	*/
	static bool isLoopbackAddr(sockaddr_storage &addr) throw()
	{
		switch(addr.ss_family){
		case AF_INET:
			if((reinterpret_cast<sockaddr_in*>(&addr))->sin_addr.s_addr == htonl(INADDR_LOOPBACK))
				return true;
			break;
		case AF_INET6:
			if(IN6_IS_ADDR_LOOPBACK(&(reinterpret_cast<sockaddr_in6*>(&addr))->sin6_addr))
				return true;
			break;
		default:
			break;
		}
		return false;
	}
	/** port getter
	    @param str : [address]:port string
	    @return port number in host byte order
	 */
	static uint16_t getPort(const string &str) throw()
	{
		uint16_t port = 0;
		union {
			sockaddr_in 		s4;
			sockaddr_in6 		s6;
			sockaddr_storage 	ss;
		} addr = {};
		try {
			if(!getSockAddr(str, addr.ss))
				return port;
		} catch(...) {
			return port;
		}
		switch(addr.ss.ss_family){
		case AF_INET:
			port = addr.s4.sin_port;
			break;
		case AF_INET6:
			port = addr.s6.sin6_port;
			break;
		default:
			return port;
		}
		return ntohs(port);
		
	}
	/** port setter
	    @param addr : address struct
	    @param port : host byte ordered port number to set 
	 */
	static void setPort(sockaddr_storage &addr, const uint16_t port) throw()
	{
		uint16_t nport = htons(port);
		switch(addr.ss_family){
		case AF_INET:
			reinterpret_cast<sockaddr_in*>(&addr)->sin_port = nport;
			break;
		case AF_INET6:
			reinterpret_cast<sockaddr_in6*>(&addr)->sin6_port = nport;
			break;
		default:
			break;
		}
		return;
	}
	/** NIC list getter
	    @return NIC's name list
	 */
	static vector<string> getNICs()
	{
		vector<string> nics;
#ifndef HAVE_WINSOCK2_H
		const static size_t MAX_NICS = 16;
		ifreq ifr[MAX_NICS] = {};
		ifconf ifc = {};
		socket_t fd = socket(AF_INET, SOCK_DGRAM, 0);
		ifc.ifc_len = sizeof(ifr);
		ifc.ifc_ifcu.ifcu_buf = reinterpret_cast<char*>(ifr);
		ioctl(fd, SIOCGIFCONF, &ifc);
		for(size_t counter = 0;
		    counter < (ifc.ifc_len / sizeof(ifreq)); ++counter){
			nics.push_back(ifr[counter].ifr_name);
		}
		close(fd);
#else
#warning dummy getNICs()!
#endif
		return nics;
	}
	/** Address list getter
	    @param addrs : Address list
	    @return true : success
	    @return false : failure
	 */
	static bool getNICaddrs(const uint16_t family, vector<sockaddr_storage> &addrs)
	{
		switch(family){
		case AF_INET:
			return InetUtil::getNICv4Addrs(addrs);
		case AF_INET6:
			return InetUtil::getNICv6Addrs(addrs);
		default:
			break;
		}
		return false;
	}
private:
	static bool getNICv4Addrs(vector<sockaddr_storage> &addrs)
	{
		vector<string> nics = InetUtil::getNICs();
		if(nics.empty())
			return false;
#ifndef HAVE_WINSOCK2_H
		socket_t fd = socket(AF_INET, SOCK_DGRAM, 0);
		for(vector<string>::iterator itr = nics.begin();
		    itr != nics.end(); ++itr){
			ifreq ifr = {};
			ifr.ifr_addr.sa_family = AF_INET;
			strncpy(ifr.ifr_name, (*itr).c_str(), IFNAMSIZ-1);
			ioctl(fd, SIOCGIFADDR, &ifr);

			sockaddr_storage addr = {};
			memcpy(&addr, &ifr.ifr_addr, sizeof(sockaddr));
			addrs.push_back(addr);
		}
		close(fd);
#else
#warning dummy getNICv4Addrs()!
#endif
		return true;
	}
	static bool getNICv6Addrs(vector<sockaddr_storage> &addrs)
	{
#ifndef HAVE_WINSOCK2_H
		ifstream ifs("/proc/net/if_inet6");
		char buf[MAX_CHAR_PER_LINE];
 		while(!ifs.eof()){
			sockaddr_in6 addr = {};
			addr.sin6_family = AF_INET6;
			ifs.getline(buf, sizeof(buf));
  			sscanf(buf, "%08x%08x%08x%08x",
  			       &addr.sin6_addr.s6_addr32[0],
  			       &addr.sin6_addr.s6_addr32[1],
  			       &addr.sin6_addr.s6_addr32[2],
  			       &addr.sin6_addr.s6_addr32[3]);
			addr.sin6_addr.s6_addr32[0] = htonl(addr.sin6_addr.s6_addr32[0]);
			addr.sin6_addr.s6_addr32[1] = htonl(addr.sin6_addr.s6_addr32[1]);
			addr.sin6_addr.s6_addr32[2] = htonl(addr.sin6_addr.s6_addr32[2]);
			addr.sin6_addr.s6_addr32[3] = htonl(addr.sin6_addr.s6_addr32[3]);
			if(IN6_IS_ADDR_UNSPECIFIED(&(addr.sin6_addr)))
				continue;
			addrs.push_back(*reinterpret_cast<sockaddr_storage*>(&addr));
		}
#else
#warning dummy getNICv6Addrs()!
#endif
		return true;
	}
};

#endif /* __INETUTILS_H__ */
