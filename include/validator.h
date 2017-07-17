/**
   @file validator.h
   @brief Validator
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: validator.h 40 2007-04-16 05:59:01Z sugiura $
 */
#ifndef __VALIDATOR_H__
#define __VALIDATOR_H__

#include <stdexcept>
#include <vector>
#include <map>
#include <algorithm>
#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>
#include <boost/lexical_cast.hpp>
using namespace std;

/** Abstruct of validator */
template <typename T>
class GenericValidator {
public:
	GenericValidator() {}
	virtual ~GenericValidator() {}
	/** validator
	    @param challenge : value to validate
	    @return true     : valid
	    @return false    : invalid
	 */
 	virtual bool valid(const T& challenge) const = 0;
	/** invalidator
	    @param challenge : value to invalidate
	    @return true     : invalid
	    @return false    : valid
	 */ 
 	virtual bool invalid(const T& challenge) const = 0;
};

/** Value range validator */
template <typename T, typename SIZE, SIZE MIN, SIZE MAX=MIN>
class RangeValidator : public GenericValidator<T>{
public:
	RangeValidator() : min(MIN), max(MAX) {}
	virtual ~RangeValidator() {}
	/** 
	    Range validate functor
	    @param challenge : a value to validate
	    @return true  : invalid value
	    @return false : valid value
	 */
 	bool operator()(const T& challenge) const
 	{ return !valid(challenge); }
	/**
	   Check if value is in valid range
	   @param challenge : a value to validate
	   @return true  : valid value
	   @return false : invalid value
	*/
 	bool valid(const T& challenge) const
 	{ return (min <= challenge && challenge <= max); }
	/**
	   Check if value is out of range
	   @param challenge : a value to invalidate
	   @return true  : invalid value
	   @return false : valid value
	*/
 	bool invalid(const T& challenge) const
 	{ return !valid(challenge); }
protected:
	/** minimum value of range */
	mutable SIZE min;
	/** maximum value of range */
	mutable SIZE max;
};

/** Value Length Validator */
template<typename T, typename SIZE, SIZE MIN, SIZE MAX=MIN>
class LengthValidator 
	: public GenericValidator<T>{
public:
	LengthValidator() : min(MIN), max(MAX) {}
	~LengthValidator() {}
	/** 
	    Length validate functor
	    @param challenge : a value to validate
	    @return true  : invalid value
	    @return false : valid value
	 */
   	bool operator()(const T& challenge) const
   	{ return !valid(challenge); }
	/**
	   Check if length is in valid range
	   @param challenge : a value to validate
	   @return true  : valid value
	   @return false : invalid value
	*/
  	bool valid(const T& challenge) const
  	{ return (min <= challenge.length() &&
		  challenge.length() <= max); }
	/**
	   Check if length is out of range
	   @param challenge : a value to invalidate
	   @return true  : invalid value
	   @return false : valid value
	*/
  	bool invalid(const T& challenge) const
  	{ return !valid(challenge); }
protected:
	/** minimum length */
	mutable SIZE min;
	/** maximum length */
	mutable SIZE max;
};

/** Range Cast Validator */
template<typename T, typename SIZE, SIZE MIN, SIZE MAX=MIN>
class RangeCastValidator 
	: public GenericValidator<T>{
public:
	RangeCastValidator() : min(MIN), max(MAX) {}
	~RangeCastValidator() {}
	/**
	   Check if length is in valid range
	   @param challenge : a value to validate
	   @return true  : valid value
	   @return false : invalid value
	*/
  	bool valid(const T& challenge) const
  	{
		SIZE length = boost::lexical_cast<SIZE>(challenge);
		return (min <= length && length <= max);
	}
	/**
	   Check if length is out of range
	   @param challenge : a value to invalidate
	   @return true  : invalid value
	   @return false : valid value
	*/
  	bool invalid(const T& challenge) const
  	{ return !valid(challenge); }
protected:
	/** minimum length */
	mutable SIZE min;
	/** maximum length */
	mutable SIZE max;
};

/** Address String Validator */
class AddrStrValidator : public GenericValidator<string>{
public:
	AddrStrValidator() {}
	virtual ~AddrStrValidator() {}
	/**
	   Check if address string is valid format
	   @param addrstr : a value to validate
	   @return true   : valid value
	   @return false  : invalid value
	 */
	bool valid(const string &addrstr) const;
	/**
	   Check if address string is invalid format
	   @param addrstr : a value to invalidate
	   @return true   : invalid value
	   @return false  : valid value
	 */
	bool invalid(const string &addrstr) const { return !valid(addrstr); }
private:
};

/** FixedValidator class */
template <typename T, T FIXED>
class FixedValidator : public GenericValidator<T>{
public:
	FixedValidator() : fixed(FIXED) {}
	virtual ~FixedValidator() {}
	/**
	   Check if challenge is valid fixed string
	   @param challenge : a value to validate
	   @return true   : valid value
	   @return false  : invalid value
	 */
	bool valid(const T &challenge) const
	{ return (challenge == fixed); }
	/**
	   Check if challenge is invalid fixed string
	   @param challenge : a value to invalidate
	   @return true   : invalid value
	   @return false  : valid value
	 */
	bool invalid(const T &challenge) const
	{ return (challenge != fixed); }
private:
	/** fixed string */
	mutable T fixed;
};

/** NullValidator class */
template <typename T>
class NullValidator : public GenericValidator<T>{
public:
	NullValidator() {}
	virtual ~NullValidator() {}
	/**
	   Null validator, always return true, as if valid value
	   @param challenge : a value to validate
	   @return true : valid value
	 */
	bool valid(const T &challenge) const
	{ return true; }
	/**
	   Null invaildator, always return false, as if valid value
	   @param challenge : a value to invalidate
	   @return false : valid value
	 */
	bool invalid(const T &challenge) const
	{ return false;	}
};

/** Attribute Validator */
class AttrValidator : public GenericValidator<string>{
public:
	/**
	   Constructor for required values only
	   @param required : required attributes array
	*/
	AttrValidator(const string required[]) throw();
	/**
	   Constructor for required & optional values
	   @param required : required attributes array
	   @param optional : optional attributes array
	*/
	AttrValidator(const string required[], const string optional[])
		throw(std::runtime_error);
	virtual ~AttrValidator() {}
	/**
	   Attribute validate functor
	   mark attrs to check if sattisfied
	   @param attr : value to validate
	 */
	bool operator()(const string &attr)
	{ return invalid(attr); }
	/** Check if all required attributes are satisfied
	    @param members : attribute names to check
	    @return true : valid
	    @return false : invalid
	 */
	bool satisfied(const vector<string> &members) throw (std::runtime_error);
	/** Check if attribute can be specified
	    @param attr : attribute name to check
	    @return true : valid attribute name
	    @return false : invalid attribute name
	 */
	bool valid(const string &attr) const
	{ return !invalid(attr); }
	/** Check if attribute can not be specified
	    @param attr : attribute name to check
	    @return true : invalid attribute name
	    @return false : valid attribute name
	 */
	bool invalid(const string &attr) const
	{ return ( required_attrs.find(attr) == required_attrs.end() &&
		   optional_attrs.find(attr) == optional_attrs.end() ); }
protected:
	/** Required attributes map */
	map<string,bool> required_attrs;
	/** Optional attributes map */
	map<string,bool> optional_attrs;
};


#endif /* __VALIDATOR_H__ */
