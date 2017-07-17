/**
   @file initiatorvalidator.h
   @brief Initiator Validator
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: initiatorvalidator.h 40 2007-04-16 05:59:01Z sugiura $
 */
#ifndef __INITIATOR_VALIDATOR_H__
#define __INITIATOR_VALIDATOR_H__

#include <map>
#include <boost/variant.hpp>
#include "json/value.h"
#include "rootvalidator.h"

using namespace std;

class InitiatorValidator : public RootValidator {
public:
	InitiatorValidator();
	virtual ~InitiatorValidator() {}
	bool attrsSatisfied(const Json::Value &root) const
		throw (std::runtime_error);
};

class InitiatorsValidator {
public:
	InitiatorsValidator() {}
	virtual ~InitiatorsValidator() {}
	/** validator
	    @return true     : valid
	    @return false    : invalid
	 */
	bool valid() const;
};

#endif /* __INITIATOR_VALIDATOR_H__ */
