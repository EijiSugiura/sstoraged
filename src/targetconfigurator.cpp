/**
   @file targetconfigurator.cpp
   @brief Target info Configurator
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: targetconfigurator.cpp 91 2007-05-22 06:54:44Z sugiura $
 */

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "common.h"
#include "targetconfigurator.h"
#include "inetutils.h"
#include "targetvalidator.h"

TargetInfo::TargetInfo(const Json::Value &value, const size_t &key)
{
	/** @todo: check existence */
	if(value.isMember("TargetName"))
		name = value["TargetName"].asString();
	else
		throw std::runtime_error("TargetName is not specified\n");
	if(value.isMember("TargetAlias"))
		alias = value["TargetAlias"].asString();
	else
		alias = name;
	if(value.isMember("TargetAddress"))
		setAddr(value["TargetAddress"].asString());
	else
		throw std::runtime_error("TargetAddress is not specified\n");
}

string TargetInfo::getAddrPortStr() const
{
	return InetUtil::getAddrPortStr(address);
}

bool TargetInfo::setAddr(const string &addr_str)
{
	return InetUtil::getSockAddr(addr_str, address);
}

Json::Value TargetInfo::serialize() const
{
	Json::Value tmp;
	tmp["TargetName"] = getName();
	tmp["TargetAlias"] = getAlias();
	try {
		tmp["TargetAddress"] = getAddrPortStr();
	} catch (...) {
		tmp["TargetAddress"] = "";
	}
	return tmp;
}

bool TargetInfo::valid() const
{
	Json::Value tmp = serialize();
	TargetValidator validator;
	return (validator.attrsSatisfied(tmp) &&
		validator.validAttrs(tmp));
}

TargetConfigurator::TargetConfigurator() : pimpl(NULL)
{
	Configurator &config = Configurator::getInstance();
	/** @todo: make this more smart way... */
	pimpl = new TaggedConfigurator<TargetInfo>(config.pimpl->root,
						   "Target");
}

TargetConfigurator::~TargetConfigurator()
{
	if(pimpl){
		delete pimpl;
		pimpl = NULL;
	}
}

TaggedConfigurator<TargetInfo>::iterator TargetConfigurator::begin()
{
	return pimpl->begin();
}

TaggedConfigurator<TargetInfo>::iterator TargetConfigurator::end()
{
	return pimpl->end();
}

bool TargetConfigurator::empty() const
{
	return (pimpl->size() == 0);
}

bool TargetConfigurator::valid()
{
	if(this->empty())
		return false;
	size_t counter = 0;
	for(TargetConfigurator::iterator itr = this->begin();
	    itr != this->end(); ++itr,++counter){
		try {
			if(!itr->valid())
				return false;
		} catch(const exception &e) {
			throw runtime_error("Target[" +
					    boost::lexical_cast<string>(counter) +
					    "] : " + e.what());
		}
	}
	/** @todo : duplicate entry check */
	return true;
}

bool TargetConfigurator::add(const TargetInfo &info)
{
	return pimpl->add(info);
}

bool TargetConfigurator::del(const string &name)
{
	return pimpl->del(name);
}

TaggedConfigurator<TargetInfo>::iterator TargetConfigurator::find(const string &name)
{
	return pimpl->find(name);
}
