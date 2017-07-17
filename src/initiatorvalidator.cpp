/**
   @file initiatorvalidator.cpp
   @brief Initiator Validator
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: initiatorvalidator.cpp 91 2007-05-22 06:54:44Z sugiura $
 */
#include <iostream>
#include "initiatorvalidator.h"
#include "initiatorconfigurator.h"

static const string required[] = {
 	"InitiatorAlias",
 	"InitiatorName",
	""};
static const string optional[] = {
 	"InitiatorAddress",
	"LUN",
 	"DiscoveryAuthMethod",
 	"DiscoveryInitiatorUsername",
 	"DiscoveryInitiatorPassword",
 	"DiscoveryTargetUsername",
 	"DiscoveryTargetPassword",
 	"SessionAuthMethod",
 	"SessionInitiatorUsername",
 	"SessionInitiatorPassword",
 	"SessionTargetUsername",
 	"SessionTargetPassword",
	""};

InitiatorValidator::InitiatorValidator()

{
 	for(validator_map_t::iterator itr = validators.begin();
 	    itr != validators.end(); ++itr){
		switch(itr->second.which()){
 		case 0:
 			delete boost::get<GenericValidator<uint64_t>* >(itr->second);
 			break;
 		case 1:
 			delete boost::get<GenericValidator<bool>* >(itr->second);
 			break;
 		case 2:
 			delete boost::get<GenericValidator<string>* >(itr->second);
 			break;
		default:
			break;
		}
 	}
	validators.clear();
 	validators["InitiatorAlias"] = new LengthValidator<string,size_t,1U,PATH_MAX>();
 	validators["InitiatorName"] = new LengthValidator<string,size_t,1U,PATH_MAX>();
 	validators["InitiatorAddress"] = new AddrStrValidator();
	validators["LUN"] = new NullValidator<string>();
 	validators["DiscoveryAuthMethod"] = new NullValidator<string>();
 	validators["DiscoveryInitiatorUsername"] = new LengthValidator<string,size_t,1U,128U>();
 	validators["DiscoveryInitiatorPassword"] = new LengthValidator<string,size_t,1U,128U>();
 	validators["DiscoveryTargetUsername"] = new LengthValidator<string,size_t,1U,128U>();
 	validators["DiscoveryTargetPassword"] = new LengthValidator<string,size_t,1U,128U>();
 	validators["SessionAuthMethod"] = new NullValidator<string>();
 	validators["SessionInitiatorUsername"] = new LengthValidator<string,size_t,1U,128U>();
 	validators["SessionInitiatorPassword"] = new LengthValidator<string,size_t,1U,128U>();
 	validators["SessionTargetUsername"] = new LengthValidator<string,size_t,1U,128U>();
 	validators["SessionTargetPassword"] = new LengthValidator<string,size_t,1U,128U>();
}

bool InitiatorValidator::attrsSatisfied(const Json::Value &root) const throw (std::runtime_error)
{
 	AttrValidator validator(required,optional);
	Json::Value::Members members = root.getMemberNames();
	Json::Value::Members::const_iterator itr =
		find_if(members.begin(), members.end(), validator);
	/** Check if invalid attrs are not exists */
	if(itr != members.end())
		throw std::runtime_error("Invalid attribute " + *itr + " found");
	/** Check if required attrs are exists */
	return validator.satisfied(members);
}

bool InitiatorsValidator::valid() const
{
	InitiatorConfigurator conf;
	return conf.valid();
}
