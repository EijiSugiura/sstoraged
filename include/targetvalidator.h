/**
   @file targetvalidator.h
   @brief Target Validator
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: targetvalidator.h 40 2007-04-16 05:59:01Z sugiura $
 */
#ifndef __TARGET_VALIDATOR_H__
#define __TARGET_VALIDATOR_H__

#include <map>
#include <boost/variant.hpp>
#include "json/value.h"
#include "rootvalidator.h"

using namespace std;

class TargetValidator : public RootValidator {
public:
	TargetValidator();
	virtual ~TargetValidator() {}
	bool attrsSatisfied(const Json::Value &root) const
		throw (std::runtime_error);
};

class TargetsValidator {
public:
	TargetsValidator() {}
	virtual ~TargetsValidator() {}
	/** validator
	    @return true     : valid
	    @return false    : invalid
	 */
	bool valid() const;
};

#endif /* __TARGET_VALIDATOR_H__ */
