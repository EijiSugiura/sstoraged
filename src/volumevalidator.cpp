/**
   @file volumevalidator.cpp
   @brief Volume Validator
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: volumevalidator.cpp 91 2007-05-22 06:54:44Z sugiura $
 */
#include <iostream>
#include "volumevalidator.h"
#include "volumeconfigurator.h"

static const string volume_required[] = {
 	"VolumeName",
//	"Main",
	""};
static const string volume_optional[] = {
//	"Copy",
	""};

VolumeValidator::VolumeValidator()
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
 	validators["VolumeName"] = new LengthValidator<string,size_t,1U,PATH_MAX>();
// 	validators["Main"] = new NullValidator<string>();
// 	validators["Copy"] = new NullValidator<string>();
}

bool VolumeValidator::attrsSatisfied(const Json::Value &root) const throw (std::runtime_error)
{
 	AttrValidator validator(volume_required,volume_optional);
	Json::Value::Members members = root.getMemberNames();
	Json::Value::Members::const_iterator itr =
		find_if(members.begin(), members.end(), validator);
	/** Check if invalid attrs are not exists */
	if(itr != members.end())
		throw std::runtime_error("Invalid attribute " + *itr + " found");
	/** Check if required attrs are exists */
	return validator.satisfied(members);
}

static const string real_required[] = {
 	"Host",
 	"Path",
	"Start",
	"Count",
	""};
static const string real_optional[] = {
	""};

RealVolumeValidator::RealVolumeValidator()
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
	/** @todo: support iSCSI name validator */
 	validators["Host"] = new LengthValidator<string,size_t,1U,PATH_MAX>();

	validators["Path"] = new LengthValidator<string,size_t,1U,PATH_MAX>();
	validators["Start"] = new RangeValidator<uint64_t,size_t,0U,ULLONG_MAX>();
	validators["Count"] = new RangeValidator<uint64_t,size_t,1U,ULLONG_MAX>();
}

bool RealVolumeValidator::attrsSatisfied(const Json::Value &root) const throw (std::runtime_error)
{
 	AttrValidator validator(real_required,real_optional);
	Json::Value::Members members = root.getMemberNames();
	Json::Value::Members::const_iterator itr =
		find_if(members.begin(), members.end(), validator);
	/** Check if invalid attrs are not exists */
	if(itr != members.end())
		throw std::runtime_error("Invalid attribute " + *itr + " found");
	/** Check if required attrs are exists */
	return validator.satisfied(members);
}

bool VolumesValidator::valid() const
{
	VolumeConfigurator conf;
	return conf.valid();
}
