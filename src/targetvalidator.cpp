/**
   @file targetvalidator.cpp
   @brief Target Validator
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: targetvalidator.cpp 91 2007-05-22 06:54:44Z sugiura $
 */
#include <iostream>
#include "targetvalidator.h"
#include "targetconfigurator.h"

static const string required[] = {
 	"TargetAlias",
 	"TargetName",
 	"TargetAddress",
	""};
static const string optional[] = {
	""};

TargetValidator::TargetValidator()
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
 	validators["TargetAlias"] = new LengthValidator<string,size_t,1U,PATH_MAX>();
 	validators["TargetName"] = new LengthValidator<string,size_t,1U,PATH_MAX>();
 	validators["TargetAddress"] = new AddrStrValidator();
}

bool TargetValidator::attrsSatisfied(const Json::Value &root) const throw (std::runtime_error)
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

bool TargetsValidator::valid() const
{
	TargetConfigurator conf;
	return conf.valid();
}
