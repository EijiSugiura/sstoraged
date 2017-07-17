/**
   @file rootvalidator.h
   @brief Root Validator
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: rootvalidator.h 91 2007-05-22 06:54:44Z sugiura $
 */
#ifndef __ROOT_VALIDATOR_H__
#define __ROOT_VALIDATOR_H__

#include <map>
#include <boost/variant.hpp>
#include "json/value.h"
#include "validator.h"

using namespace std;

/** Root configuration validator */
class RootValidator {
public:
	RootValidator();
	virtual ~RootValidator();

	/** Check if required attrs are satisfied */
	bool attrsSatisfied(const Json::Value &root) const throw (std::runtime_error);

	/** Multiple Attribute & Value validator */
	bool validAttrs(const Json::Value &root) const ;

	/** Attribute & Value validator */
	template <typename T>
	bool validAttr(const string &attr, const T& value) const
	{
		bool ret = false;
		validator_map_t::const_iterator itr = validators.find(attr);
		if(itr ==validators.end())
			return ret;
		try {
			GenericValidator<T> *validator = boost::get< GenericValidator<T>* >(itr->second);
			ret = validator->valid(value);
		} catch  (const std::exception &e) {
			throw std::runtime_error(attr + " : Type mismatch? " + e.what());
		}
		return ret;
	}
protected:
	typedef map<string, boost::variant<GenericValidator<uint64_t>*,
					   GenericValidator<bool>*,
					   GenericValidator<string>* > > validator_map_t;
	/** Validator map */
	validator_map_t validators;
};

#endif /* __ROOT_VALIDATOR_H__ */
