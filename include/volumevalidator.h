/**
   @file volumevalidator.h
   @brief Volume Validator
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: volumevalidator.h 40 2007-04-16 05:59:01Z sugiura $
 */
#ifndef __VOLUME_VALIDATOR_H__
#define __VOLUME_VALIDATOR_H__

#include <map>
#include <boost/variant.hpp>
#include "json/value.h"
#include "rootvalidator.h"

using namespace std;

class VolumeValidator : public RootValidator {
public:
	VolumeValidator();
	virtual ~VolumeValidator() {}
	bool attrsSatisfied(const Json::Value &root) const
		throw (std::runtime_error);
};

class RealVolumeValidator : public RootValidator {
public:
	RealVolumeValidator();
	virtual ~RealVolumeValidator() {}
	bool attrsSatisfied(const Json::Value &root) const
		throw (std::runtime_error);
};

class VolumesValidator {
public:
	VolumesValidator() {}
	virtual ~VolumesValidator() {}
	/** validator
	    @return true     : valid
	    @return false    : invalid
	 */
	bool valid() const;
};

#endif /* __VOLUME_VALIDATOR_H__ */
