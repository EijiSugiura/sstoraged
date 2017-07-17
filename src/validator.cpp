/**
   @file validator.cpp
   @brief Validator implementations
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: validator.cpp 40 2007-04-16 05:59:01Z sugiura $
 */
#include <iostream>
#include <sstream>
#include <algorithm>
#include "validator.h"
#include "inetutils.h"

bool AddrStrValidator::valid(const string &addrstr) const
{
	sockaddr_storage tmp;
	return InetUtil::getSockAddr(addrstr, tmp);
}

AttrValidator::AttrValidator(const string required[]) throw()
{
	/** Initialize required attributes */
	for(size_t counter=0; required[counter].length() > 0; ++counter){
		required_attrs[required[counter]] = false;
	}
}

AttrValidator::AttrValidator(const string required[], const string optional[])
	throw(std::runtime_error)
{
	/** Initialize required attributes */
	for(size_t counter=0; required[counter].length() > 0; ++counter){
		required_attrs[required[counter]] = false;
	}
	/** Initialize required attributes */
	for(size_t counter=0; optional[counter].length() > 0; ++counter){
		if(required_attrs.find(optional[counter]) != required_attrs.end())
			throw std::runtime_error(optional[counter] + " is duplicated");
		optional_attrs[optional[counter]] = false;
	}
}

bool AttrValidator::satisfied(const vector<string>& members) throw (std::runtime_error)
{
	for(map<string,bool>::iterator key = required_attrs.begin();
	    key != required_attrs.end(); ++key){
		vector<string>::const_iterator itr = find(members.begin(),
							  members.end(),
							  key->first);
		if(itr == members.end())
			throw std::runtime_error(key->first + " is not specified\n");
	}
	return true;
}
