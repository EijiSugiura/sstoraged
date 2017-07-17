#include <cxxtest/TestSuite.h>
#include <sstream>
#include <iostream>

using namespace std;

#include "inetutils.h"

class InetUtilTestSuite : public CxxTest::TestSuite 
{
 public:
	void setUp()
	{
	}
	void tearDown()
	{
	}

	/** getAddrPortStr */
	void testGetAddrStr_OK()
	{
		sockaddr_storage addr = {};
		sockaddr_in *sin = reinterpret_cast<sockaddr_in*>(&addr); 
		sin->sin_family = AF_INET;
		sin->sin_port = htons(80);
		sin->sin_addr.s_addr = htonl(0xAC100001U);
		TS_ASSERT_EQUALS(InetUtil::getAddrPortStr(addr), static_cast<string>("[172.16.0.1]:80"));

		sockaddr_in6 *sin6 = reinterpret_cast<sockaddr_in6*>(&addr);
		sin6->sin6_family = AF_INET6;
		sin6->sin6_port = htons(80);
		sin6->sin6_addr = in6addr_loopback;
		TS_ASSERT_EQUALS(InetUtil::getAddrPortStr(addr), static_cast<string>("[::1]:80"));
	}
	void testGetAddrStr_NG()
	{
		sockaddr_storage addr = {};
		TS_ASSERT_THROWS_EQUALS(InetUtil::getAddrPortStr(addr), const std::exception &e,
					string(e.what()), "ai_family not supported");

		sockaddr_in *sin = reinterpret_cast<sockaddr_in*>(&addr); 
		sin->sin_family = AF_INET;
		TS_ASSERT_THROWS_EQUALS(InetUtil::getAddrPortStr(addr), const std::exception &e,
					string(e.what()), "Invalid port specified");

		sockaddr_in6 *sin6 = reinterpret_cast<sockaddr_in6*>(&addr);
		sin6->sin6_family = AF_INET6;
		TS_ASSERT_THROWS_EQUALS(InetUtil::getAddrPortStr(addr), const std::exception &e,
					string(e.what()), "Invalid port specified");
	}
	/** getSockAddr */
	void testGetSockAddr_OK()
	{
		sockaddr_storage repl = {};
		TS_ASSERT_EQUALS(InetUtil::getSockAddr(static_cast<string>("[172.16.0.1]:80"),repl), true);
		sockaddr_in *sin = reinterpret_cast<sockaddr_in*>(&repl); 
		TS_ASSERT_EQUALS(sin->sin_family, AF_INET);
		TS_ASSERT_EQUALS(sin->sin_port, htons(80));
		TS_ASSERT_EQUALS(sin->sin_addr.s_addr, htonl(0xAC100001U));

//		TS_ASSERT_EQUALS(InetUtil::getSockAddr(static_cast<string>("[localhost]:80"),repl), true);
//		TS_ASSERT_EQUALS(sin->sin_family, AF_INET);
//		TS_ASSERT_EQUALS(sin->sin_port, htons(80));
//		TS_ASSERT_EQUALS(sin->sin_addr.s_addr, htonl(0x7F000001U));

		TS_ASSERT_EQUALS(InetUtil::getSockAddr(static_cast<string>("[FC01::1]:80"),repl), true);
 		sockaddr_in6 *sin6 = reinterpret_cast<sockaddr_in6*>(&repl);
		TS_ASSERT_EQUALS(sin6->sin6_family, AF_INET6);
		TS_ASSERT_EQUALS(sin6->sin6_port, htons(80));
		in6_addr tmp = {};
		tmp.s6_addr[0] = 0xFC;
		tmp.s6_addr[1] = 0x01;
		tmp.s6_addr[15] = 0x01;
		TS_ASSERT_EQUALS(IN6_ARE_ADDR_EQUAL(&sin6->sin6_addr, &tmp), true);

		TS_ASSERT_EQUALS(InetUtil::getSockAddr(static_cast<string>("[::1]:80"),repl), true);
		TS_ASSERT_EQUALS(sin6->sin6_family, AF_INET6);
		TS_ASSERT_EQUALS(sin6->sin6_port, htons(80));
		TS_ASSERT_EQUALS(IN6_ARE_ADDR_EQUAL(&sin6->sin6_addr, &in6addr_loopback), true);
	}
	void testGetSockAddr_NG()
	{
		sockaddr_storage repl = {};
		TS_ASSERT_EQUALS(InetUtil::getSockAddr(static_cast<string>(""),repl), false);
		TS_ASSERT_EQUALS(InetUtil::getSockAddr(static_cast<string>("[]:"),repl), false);
	}
	/** isAnyAddr */
	void testIsAnyAddr()
	{
		string str = "[0.0.0.0]:3260";
		TS_ASSERT_EQUALS(InetUtil::isAnyAddr(str), true);
		str = "[127.0.0.1]:3260";
		TS_ASSERT_EQUALS(InetUtil::isAnyAddr(str), false);
		str = "[::]:3260";
		TS_ASSERT_EQUALS(InetUtil::isAnyAddr(str), true);
		str = "[::1]:3260";
		TS_ASSERT_EQUALS(InetUtil::isAnyAddr(str), false);
	}
	/** isLoopbackAddr */
	void testIsLoopbackAddr()
	{
		sockaddr_storage addr;
		string str = "[0.0.0.0]:3260";
		InetUtil::getSockAddr(str, addr);
		TS_ASSERT_EQUALS(InetUtil::isLoopbackAddr(addr), false);
		str = "[127.0.0.1]:3260";
		InetUtil::getSockAddr(str, addr);
		TS_ASSERT_EQUALS(InetUtil::isLoopbackAddr(addr), true);
		str = "[::]:3260";
		InetUtil::getSockAddr(str, addr);
		TS_ASSERT_EQUALS(InetUtil::isLoopbackAddr(addr), false);
		str = "[::1]:3260";
		InetUtil::getSockAddr(str, addr);
		TS_ASSERT_EQUALS(InetUtil::isLoopbackAddr(addr), true);
	}
	/** getNICs */
	void testGetNICs()
	{
		vector<string> nics;
		TS_ASSERT_THROWS_NOTHING(nics = InetUtil::getNICs());
		cout << endl << "NIC list :";
		for(vector<string>::iterator nic = nics.begin();
		    nic != nics.end(); ++nic)
			cout << " " << *nic;
		cout << endl;
	}
	/** getNICaddrs */
	void testGetNICaddrs()
	{
		vector<sockaddr_storage> addrs;
		bool ret;
		TS_ASSERT_THROWS_NOTHING(ret = InetUtil::getNICaddrs(0, addrs));
		TS_ASSERT_EQUALS(ret, false);

		TS_ASSERT_THROWS_NOTHING(ret = InetUtil::getNICaddrs(AF_INET, addrs));
		TS_ASSERT_EQUALS(ret, true);
		cout << endl << "ADDR(IPv4) list :";
		for(vector<sockaddr_storage>::iterator addr = addrs.begin();
		    addr != addrs.end(); ++addr)
			cout << " " << InetUtil::getAddrStr(*addr);
		cout << endl;

		addrs.clear();
		TS_ASSERT_THROWS_NOTHING(ret = InetUtil::getNICaddrs(AF_INET6, addrs));
		TS_ASSERT_EQUALS(ret, true);
		cout << endl << "ADDR(IPv6) list :";
		for(vector<sockaddr_storage>::iterator addr = addrs.begin();
		    addr != addrs.end(); ++addr)
			cout << " " << InetUtil::getAddrStr(*addr);
		cout << endl;
	}
};
