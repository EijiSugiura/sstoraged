/**
   @file initiatorconfigurator.cpp
   @brief Initiator info Configurator
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: initiatorconfigurator.cpp 145 2007-06-20 03:30:40Z sugiura $
 */

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "common.h"
#include "initiatorconfigurator.h"
#include "initiatorvalidator.h"

InitiatorConfigurator::InitiatorConfigurator() : pimpl(NULL)
{
	Configurator &config = Configurator::getInstance();
	/** @todo: make this more smart way... */
	pimpl = new TaggedConfigurator<InitiatorInfo>(config.pimpl->root,
						      "Initiator");
}

InitiatorConfigurator::~InitiatorConfigurator()
{
	if(pimpl){
 		delete pimpl;
 		pimpl = NULL;
	}
}

TaggedConfigurator<InitiatorInfo>::iterator InitiatorConfigurator::begin()
{
	return pimpl->begin();
}

TaggedConfigurator<InitiatorInfo>::iterator InitiatorConfigurator::end()
{
	return pimpl->end();
}

bool InitiatorConfigurator::empty() const
{
	return (pimpl->size() == 0);
}

bool InitiatorConfigurator::valid()
{
	if(this->empty())
		return false;
	size_t counter = 0;
	for(InitiatorConfigurator::iterator itr = this->begin();
	    itr != this->end(); ++itr,++counter){
		try {
			if(!itr->valid())
				return false;
		} catch (const exception &e) {
			throw runtime_error("Initiator[" +
					    boost::lexical_cast<string>(counter) +
					    "] : " + e.what());
		}
	}
	return true;
}

bool InitiatorConfigurator::add(const InitiatorInfo &info)
{
	return pimpl->add(info);
}

bool InitiatorConfigurator::del(const string &name)
{
	return pimpl->del(name);
}

TaggedConfigurator<InitiatorInfo>::iterator InitiatorConfigurator::find(const string &name)
{
	return pimpl->find(name);
}

size_t InitiatorConfigurator::size()
{
	return pimpl->size();
}

InitiatorInfo::InitiatorInfo(Json::Value &value, const size_t &key)
	: TargetInfo("",""), luns(NULL)
{
	if(value.isMember("InitiatorName"))
		name = value["InitiatorName"].asString();
	else
		throw std::runtime_error("InitiatorName is not specified\n");
	if(value.isMember("InitiatorAlias"))
		alias = value["InitiatorAlias"].asString();
	else
		alias = name;
	if(value.isMember("InitiatorAddress"))
		setAddr(value["InitiatorAddress"].asString());
// 	else
// 		throw std::runtime_error("InitiatorAddress is not specified\n");

	luns = new TaggedConfigurator<LUNinfo>(value, "LUN");

	/** @todo: validate for required attribute */
	if(value.isMember("DiscoveryAuthMethod")){
		string method = value["DiscoveryAuthMethod"].asString();
		if(method.find("None") != method.npos){
			AuthInfoiSCSI tmp("None", "Discovery");
			myowns.push_back(tmp);
			peers.push_back(tmp);
		}
		if(method.find("CHAP") != method.npos &&
		   value.isMember("DiscoveryInitiatorUsername") &&
		   value.isMember("DiscoveryInitiatorPassword")){
			AuthInfoiSCSI tmp("CHAP", "Discovery");
			tmp.setName(value["DiscoveryInitiatorUsername"].asString());
			tmp.setPassword(value["DiscoveryInitiatorPassword"].asString());
			peers.push_back(tmp);
		}
		if(method.find("CHAP") != method.npos &&
		   value.isMember("DiscoveryTargetUsername") &&
		   value.isMember("DiscoveryTargetPassword")){
			AuthInfoiSCSI tmp("CHAP", "Discovery");
			tmp.setName(value["DiscoveryTargetUsername"].asString());
			tmp.setPassword(value["DiscoveryTargetPassword"].asString());
			myowns.push_back(tmp);
		}
	}
	if(value.isMember("SessionAuthMethod")){
		string method = value["SessionAuthMethod"].asString();
		if(method.find("None") != method.npos){
			AuthInfoiSCSI tmp("None", "Session");
			myowns.push_back(tmp);
			peers.push_back(tmp);
		}
		if(method.find("CHAP") != method.npos &&
		   value.isMember("SessionInitiatorUsername") &&
		   value.isMember("SessionInitiatorPassword")){
			AuthInfoiSCSI tmp("CHAP", "Session");
			tmp.setName(value["SessionInitiatorUsername"].asString());
			tmp.setPassword(value["SessionInitiatorPassword"].asString());
			peers.push_back(tmp);
		}
		if(method.find("CHAP") != method.npos &&
		   value.isMember("SessionTargetUsername") &&
		   value.isMember("SessionTargetPassword")){
			AuthInfoiSCSI tmp("CHAP", "Session");
			tmp.setName(value["SessionTargetUsername"].asString());
			tmp.setPassword(value["SessionTargetPassword"].asString());
			myowns.push_back(tmp);
		}
	}
}

Json::Value InitiatorInfo::serialize() const
{
	Json::Value tmp;
	tmp["InitiatorName"] = getName();
	tmp["InitiatorAlias"] = getAlias();
	try {
		tmp["InitiatorAddress"] = getAddrPortStr();
	} catch(...) {
		/** @todo : nothing to do? */
		// tmp["InitiatorAddress"] = "";
	}
	/** @todo: add other members */
	for(vector<AuthInfoiSCSI>::const_iterator itr = peers.begin();
	    itr != peers.end(); ++itr){
		string method = itr->getMethod();
		if(itr->getState() == "Discovery"){
			if(tmp["DiscoveryAuthMethod"].asString().length() > 0)
				tmp["DiscoveryAuthMethod"].append("|"+ method);
			else
				tmp["DiscoveryAuthMethod"] = method;
			tmp["DiscoveryInitiatorUsername"] = itr->getName();
			tmp["DiscoveryInitiatorPassword"] = itr->getPassword();
		}else if(itr->getState() == "Session"){
			if(tmp["SessionAuthMethod"].asString().length() > 0)
				tmp["SessionAuthMethod"].append("|"+ method);
			else
				tmp["SessionAuthMethod"] = method;
			tmp["SessionInitiatorUsername"] = itr->getName();
			tmp["SessionInitiatorPassword"] = itr->getPassword();
		}else{
			// error
		}
	}
	for(vector<AuthInfoiSCSI>::const_iterator itr = myowns.begin();
	    itr != myowns.end(); ++itr){
		string method = itr->getMethod();
		if(itr->getState() == "Discovery"){
			if(tmp["DiscoveryAuthMethod"].asString().length() > 0)
				tmp["DiscoveryAuthMethod"].append("|"+ method);
			else
				tmp["DiscoveryAuthMethod"] = method;
			tmp["DiscoveryAuthMethod"] = method;
			tmp["DiscoveryTargetUsername"] = itr->getName();
			tmp["DiscoveryTargetPassword"] = itr->getPassword();
		}else if(itr->getState() == "Session"){
			if(tmp["SessionAuthMethod"].asString().length() > 0)
				tmp["SessionAuthMethod"].append("|"+ method);
			else
				tmp["SessionAuthMethod"] = method;
			tmp["SessionAuthMethod"] = method;
			tmp["SessionTargetUsername"] = itr->getName();
			tmp["SessionTargetPassword"] = itr->getPassword();
		}else{
			// error
		}
	}
	return tmp;
}

bool InitiatorInfo::validPeer(const AuthInfoiSCSI &info) const
{
	string method = info.getMethod();
	if(method.find("None") != method.npos){
		for(vector<AuthInfoiSCSI>::const_iterator itr = peers.begin();
		    itr != peers.end(); ++itr){
			if(info.getState() == itr->getState() &&
			   info.getMethod() == "None" )
				return true;
		}
	} else if(method.find("CHAP") != method.npos){
		for(vector<AuthInfoiSCSI>::const_iterator itr = peers.begin();
		    itr != peers.end(); ++itr){
			if(info.getState() == itr->getState() &&
			   info.getMethod() == "CHAP" &&
			   info.getName() == itr->getName() &&
			   info.getPassword() == itr->getPassword())
				return true;
		}
	}
	return false;
}

bool InitiatorInfo::getMyAuth(const string &state, vector<AuthInfoiSCSI> &infos) const
{
	infos.clear();
	for(vector<AuthInfoiSCSI>::const_iterator itr = myowns.begin();
	    itr != myowns.end(); ++itr){
		if(itr->getState() == state){
			infos.push_back(*itr);
		}
	}
	if(infos.size() > 0)
		return true;
	return false;
}

bool InitiatorInfo::valid() const
{
	/** @todo : duplicate entry check */
	Json::Value tmp = serialize();
	InitiatorValidator validator;
	return (validator.attrsSatisfied(tmp) &&
		validator.validAttrs(tmp));
}
