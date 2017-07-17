/**
   @file common.cpp
   @brief Common implementations
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: common.cpp 312 2007-09-28 00:56:17Z sugiura $
 */

#include <sstream>
#include <iomanip>
#include <boost/lexical_cast.hpp>
#include "common.h"

using namespace std;

#ifndef HAVE_WINSOCK2_H
const socket_t INVALID_SOCKET = -1;
const socket_t INVALID_HANDLE_VALUE = -1;
#endif

string SCSI::DUMPOCTETS(const uint8_t *ptr)
{
	ostringstream os;
	os << boost::lexical_cast<string>((void*)ptr) << " ";
	os << hex << setfill('0');
	size_t counter = 8;
	while(counter--){
		os << setw(2) << (0x00FF&(unsigned int)(*ptr));
		++ptr;
	}
	return os.str();
}

string SCSI::DUMPOCTETS(const uint8_t *ptr, size_t size)
{
	ostringstream os;
	os << boost::lexical_cast<string>((void*)ptr) << " ";
	os << hex << setfill('0');
	while(size){
		size_t counter = min((size_t)32,size);
		while(counter--){
			os << setw(2) << (0x00FF&(unsigned int)(*ptr));
			++ptr;
			--size;
		}
		os << endl;
	}
	return os.str();
}

string SCSI::DUMPBHS(const uint8_t *ptr)
{
	ostringstream os;
	os << endl;
	size_t counter = 6;
	while(counter--){
		os << DUMPOCTETS(ptr) << endl;
		ptr += 8;
	}
	return os.str();
}
