/**
   @file rootvalidator.cpp
   @brief Root Validator
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: rootvalidator.cpp 225 2007-07-20 03:07:33Z sugiura $
 */
#include <iostream>
#include "rootvalidator.h"
#include "targetportvalidator.h"
#include "targetvalidator.h"
#include "initiatorvalidator.h"
#include "volumevalidator.h"

static const string required[] = {
	"GlobalTargetName",
	"LocalTargetName",
	""};
static const string optional[] = {
	"ConfigFile",
	"PIDfile",
	"Debug",
	"TargetPort",
	"ConsolePort",
	"LogFile",
	"LogLevel",
	"MaxConnections",
	"InitialR2T",
	"ImmediateData",
	"MaxRecvDataSegmentLength",
	"MaxBurstLength",
	"FirstBurstLength",
	"DefaultTime2Wait",
	"DefaultTime2Retain",
	"MaxOutstandingR2T",
	"DataPDUInOrder",
	"DataSequenceInOrder",
	"ErrorRecoveryLevel",
	"HeaderDigest",
	"DataDigest",
	"NopOutInterval",
	"NopOutTimeout",
	"IFMarker",
	"OFMarker",
	"Target",
	"Initiator",
	"Volume",
	"PoolHardLimit",
	"PoolSoftLimit",
	""};

RootValidator::RootValidator()
{
 	validators["ConfigFile"] = new LengthValidator<string,size_t,1U,PATH_MAX>();
 	validators["PIDfile"] = new LengthValidator<string,size_t,1U,PATH_MAX>();
 	validators["Debug"] = new NullValidator<bool>();

	/** @todo: support iSCSI name validator */
 	validators["GlobalTargetName"] = new LengthValidator<string,size_t,1U,PATH_MAX>();
 	validators["LocalTargetName"] = new LengthValidator<string,size_t,1U,PATH_MAX>();

 	validators["ConsolePort"] = new AddrStrValidator();
 	validators["LogFile"] = new LengthValidator<string,size_t,1U,PATH_MAX>();

 	/** @todo: support exact string match */
 	validators["LogLevel"] = new LengthValidator<string,size_t,1U,6U>();
 	validators["PoolSoftLimit"] = new RangeValidator<uint64_t,size_t,1048576U,LONG_MAX>();
 	validators["PoolHardLimit"] = new RangeValidator<uint64_t,size_t,2097152U,ULONG_MAX>();

 	validators["MaxConnections"] = new RangeValidator<uint64_t,size_t,1U,8U>();
 	validators["InitialR2T"] = new NullValidator<bool>();
 	validators["ImmediateData"] = new NullValidator<bool>();
 	validators["MaxRecvDataSegmentLength"] = new RangeValidator<uint64_t,size_t,512U,((2U<<24) - 1)>();
 	validators["MaxBurstLength"] = new  RangeValidator<uint64_t,size_t,512U,((2U<<24) - 1)>();
 	validators["FirstBurstLength"] = new  RangeValidator<uint64_t,size_t,512U,((2U<<24) - 1)>();
 	validators["DefaultTime2Wait"] = new  RangeValidator<uint64_t,size_t,0U,3600U>();
 	validators["DefaultTime2Retain"] = new  RangeValidator<uint64_t,size_t,0U,3600U>();
 	validators["MaxOutstandingR2T"] = new RangeValidator<uint64_t,size_t,0U,USHRT_MAX>();
 	validators["DataPDUInOrder"] = new FixedValidator<bool,true>();
 	validators["DataSequenceInOrder"] = new FixedValidator<bool,true>();
 	validators["ErrorRecoveryLevel"] = new RangeValidator<uint64_t,size_t,0U,2U>();

 	/** @todo: support exact string match */
 	validators["HeaderDigest"] = new LengthValidator<string,size_t,4U,12U>();
 	validators["DataDigest"] = new LengthValidator<string,size_t,4U,12U>();

 	validators["NopOutInterval"] = new RangeValidator<uint64_t,size_t,0U,3600U>();
 	validators["NopOutTimeout"] = new RangeValidator<uint64_t,size_t,0U,60U>();

 	validators["IFMarker"] = new NullValidator<bool>();
 	validators["OFMarker"] = new NullValidator<bool>();
}

RootValidator::~RootValidator()
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
}

bool RootValidator::attrsSatisfied(const Json::Value &root) const throw (std::runtime_error)
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

bool RootValidator::validAttrs(const Json::Value &root) const
{
	string errors = "";
	bool ret = true;
	Json::Value::Members members = root.getMemberNames();
	for(Json::Value::Members::const_iterator itr = members.begin();
	    itr != members.end(); ++itr){
		try {
			if(*itr == "TargetPort"){
				TargetPortValidator targetport_validator;
				if(!targetport_validator.valid()){
					ret = false;
					errors.append(*itr + " is invalid\n");
				}
				continue;
			} else if(*itr == "Target"){
				TargetsValidator targets_validator;
				if(!targets_validator.valid()){
					ret = false;
					errors.append(*itr + " is invalid\n");
				}
				continue;
			} else if(*itr == "Initiator"){
				InitiatorsValidator initiators_validator;
				if(!initiators_validator.valid()){
					ret = false;
					errors.append(*itr + " is invalid\n");
				}
				continue;
			} else if(*itr == "Volume"){
				VolumesValidator volumes_validator;
				if(!volumes_validator.valid()){
					ret = false;
					errors.append(*itr + " is invalid\n");
				}
				continue;
			}
			switch(root[*itr].type()){
			case Json::uintValue:
			case Json::intValue:
				if(!validAttr(*itr, root[*itr].asUInt())){
					ret = false;
					errors.append(*itr + " is invalid\n");
				}
				break;
			case Json::stringValue:
				if(!validAttr(*itr, root[*itr].asString())){
					ret = false;
					errors.append(*itr + " is invalid\n");
				}
				break;
			case Json::booleanValue:
				if(!validAttr(*itr, root[*itr].asBool())){
					ret = false;
					errors.append(*itr + " is invalid\n");
				}
				break;
			default:
				ret = false;
				throw std::runtime_error("Unsupported type : " + *itr);
				break;
			}
		} catch (const std::exception &e) {
			ret = false;
			errors.append(e.what());
		}
	}
	if(ret == false)
		throw std::runtime_error(errors);
	return ret;
}
