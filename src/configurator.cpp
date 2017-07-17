/**
   @file configurator.cpp
   @brief Configurator
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: configurator.cpp 230 2007-07-23 04:47:06Z sugiura $
 */

#include <iostream>
#include <fstream>
#include <boost/thread/once.hpp>
#include "common.h"
#include "json/reader.h"
#include "json/writer.h"
#include "configurator.h"
#include "rootvalidator.h"

Configurator *Configurator::cache = NULL;

Configurator::Configurator() throw() : pimpl(NULL)
{
	pimpl = new ConfiguratorImpl();
}

Configurator &Configurator::getInstance() throw(std::out_of_range)
{
	try {
		static boost::once_flag flg = BOOST_ONCE_INIT;
		boost::call_once(init, flg);
	} catch(...) {
		throw std::out_of_range("Can't get Configurator instance");
	}
	return *cache;
}

boost::recursive_mutex &Configurator::getGuard()
{
	return Configurator::getInstance().pimpl->config_guard;
}

void Configurator::init() throw(std::out_of_range)
{
	try{
		cache = new Configurator();
	}catch(...){
		throw std::out_of_range("Failed to construct Configurator");
	}
}

Configurator::ConfiguratorImpl::~ConfiguratorImpl()
{
	lock lk(config_guard);
	root.clear();
	defaultValue.clear();
}

bool Configurator::setDefaults() throw(std::runtime_error)
{
	bool ret = false;
	try {
		pimpl->clear();
		ret = pimpl->setDefaults();
	} catch(const std::exception &e) {
		throw std::runtime_error(e.what());
	} catch(...) {
		throw std::runtime_error("Unknown error : " + static_cast<string>(__PRETTY_FUNCTION__));
	}
	return ret;
}

bool Configurator::parse(const string& lines) throw (std::runtime_error)
{
	bool ret = false;
	try {
		ret = pimpl->parse(lines);
		/** @todo : validate */
	} catch(const std::exception &e) {
		throw std::runtime_error(e.what());
	} catch(...) {
		throw std::runtime_error("Unknown error : " + static_cast<string>(__PRETTY_FUNCTION__));
	}
	return ret;
}

string Configurator::dump() throw (std::runtime_error)
{
	string lines("");
	bool ret = false;
	try {
		ret = pimpl->dump(lines);
	} catch(const std::exception &e) {
		throw std::runtime_error(e.what());
	} catch(...) {
		throw std::runtime_error("Unknown error : " + static_cast<string>(__PRETTY_FUNCTION__));
	}
	return lines;
}

void Configurator::clear() throw()
{
	try {
		pimpl->clear();
//  		delete pimpl;
//  		pimpl = NULL;
//   		delete cache;
//   		cache = NULL;
	} catch (...) {
		// noting to do...
	}
}


Configurator::ConfiguratorImpl::ConfiguratorImpl()
{
	try {
		lock lk(config_guard);
		clear();
		setDefaults();
	} catch (...) {
		throw std::runtime_error("Can't construct ConfiguratorImpl\n");
	}
}

void Configurator::ConfiguratorImpl::clear()
{
	try {
		lock lk(config_guard);
		root.clear();
	} catch (...) {
		throw std::runtime_error("Failed to clear configurations\n");
	}
}

bool Configurator::ConfiguratorImpl::setDefaults()
{
	defaultValue["Debug"] = 	false;
	defaultValue["ConfigFile"] = 	static_cast<string>(DEFAULT_CONFIGFILE);
	defaultValue["PIDfile"] =	static_cast<string>(DEFAULT_PIDFILE);
#include "configdefault.hpp"
	return true;
}

bool Configurator::ConfiguratorImpl::parse(const string &lines)
{
	lock lk(config_guard);
	Json::Reader reader;
	saveCommandArgs();
	try {
		if(!reader.parse(lines, root)){
			throw std::runtime_error("Failed to parse configuration : " + reader.getFormatedErrorMessages());
			return false;
		}
	} catch (...) {
		throw std::runtime_error("Failed to parse configuration : " + reader.getFormatedErrorMessages());
	}
	restoreCommandArgs();
	return true;
}

void Configurator::ConfiguratorImpl::saveCommandArgs()
{
	/** Save whole config root */
	cmdargs = root;
}

void Configurator::ConfiguratorImpl::restoreCommandArgs()
{
	/** Restore command args */
	if(cmdargs.isMember("Debug"))
		root["Debug"] = 	cmdargs["Debug"];
	if(cmdargs.isMember("ConfigFile"))
		root["ConfigFile"] = 	cmdargs["ConfigFile"];
	if(cmdargs.isMember("PIDfile"))
		root["PIDfile"] = 	cmdargs["PIDfile"];
}

bool Configurator::ConfiguratorImpl::dump(string& lines)
{
	try {
		lock lk(config_guard);
		Json::StyledWriter writer;
		lines = writer.write(root);
	} catch (...) {
		throw std::runtime_error("Unknown error : " + static_cast<string>(__PRETTY_FUNCTION__) );
	}
	return true;
}

bool Configurator::ConfiguratorImpl::getAttr(const string& attr, bool& value)
{
	lock lk(config_guard);
	config_map_t::const_iterator itr = defaultValue.find(attr);
	if(!root.isMember(attr) && itr == defaultValue.end())
		return false;
	try {
		if(root.isMember(attr) && itr == defaultValue.end())
			value = root[attr].asBool();
		else
			value = root.get(attr, boost::get<bool>(itr->second)).asBool();
	} catch(const std::exception &e) {
		throw e;
	} catch(...) {
		throw std::runtime_error("Unknown error : " + static_cast<string>(__PRETTY_FUNCTION__) );
	}
	return true;
}

bool Configurator::ConfiguratorImpl::getAttr(const string& attr, string& value)
{
	lock lk(config_guard);
	config_map_t::const_iterator itr = defaultValue.find(attr);
	if(!root.isMember(attr) && itr == defaultValue.end())
		return false;
	try {
		if(root.isMember(attr) && itr == defaultValue.end())
			value = root[attr].asString();
		else
			value = root.get(attr, boost::get<string>(itr->second)).asString();
	} catch(const std::exception &e) {
		throw e;
	} catch(...) {
		throw std::runtime_error("Unknown error : " + static_cast<string>(__PRETTY_FUNCTION__) );
	}
	return true;
}

bool Configurator::ConfiguratorImpl::getAttr(const string& attr, int& value)
{
	lock lk(config_guard);
	config_map_t::const_iterator itr = defaultValue.find(attr);
	if(!root.isMember(attr) && itr == defaultValue.end())
		return false;
	try {
		if(root.isMember(attr) && itr == defaultValue.end())
			value = root[attr].asInt();
		else
			value = root.get(attr, boost::get<int>(itr->second)).asInt();
	} catch(const std::exception &e) {
		throw e;
	} catch(...) {
		throw std::runtime_error("Unknown error : " + static_cast<string>(__PRETTY_FUNCTION__) );
	}
	return true;
}

bool Configurator::ConfiguratorImpl::getAttr(const string& attr, uint64_t& value)
{
	lock lk(config_guard);
	config_map_t::const_iterator itr = defaultValue.find(attr);
	if(!root.isMember(attr) && itr == defaultValue.end())
		return false;
	try {
		if(root.isMember(attr) && itr == defaultValue.end())
			value = root[attr].asUInt();
		else
			value = root.get(attr, boost::get<uint64_t>(itr->second)).asUInt();
	} catch(const exception &e) {
		throw e;
	} catch(...) {
		throw std::runtime_error("Unknown error : " + static_cast<string>(__PRETTY_FUNCTION__) );
	}
	return true;
}

bool Configurator::validate() throw (std::runtime_error)
{
	bool ret = false;
	try {
		RootValidator root_validator;
		lock lk(pimpl->config_guard);
		ret = (root_validator.attrsSatisfied(pimpl->root) &&
		       root_validator.validAttrs(pimpl->root));
	} catch (const std::exception &e) {
		throw std::runtime_error(e.what());
	}
	return ret;
}
