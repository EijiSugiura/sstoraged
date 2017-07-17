/**
   @file keyvalue.h
   @brief iSCSI Key/Value pair validators
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: keyvalue.h 146 2007-06-20 13:54:08Z sugiura $
 */

#ifndef __ISCSI_KEYVALUE_H__
#define __ISCSI_KEYVALUE_H__

#include <boost/shared_ptr.hpp>
#include "validator.h"
#include "iscsi/login.h"

using namespace std;

class GenericPairValidator : public GenericValidator<string> {
public:
	GenericPairValidator() {}
	virtual ~GenericPairValidator() {}
	bool valid(const string &value) const = 0;
	bool invalid(const string &value) const = 0;
protected:
//	static const size_t KEY_MAXLEN = 64;
	static const size_t VALUE_MAXLEN = 255;
};

class iSCSISession;
class GenericPairHandler {
public:
	GenericPairHandler() {}
	virtual ~GenericPairHandler() {}
	virtual bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
			    LoginGenerator &generator) = 0;
};

class AlwaysTrueValidator : public GenericPairValidator {
public:
	bool valid(const string &value) const;
	bool invalid(const string &value) const { return !valid(value); }
};

class NothingTodoHandler : public GenericPairHandler {
public:
	bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
		    LoginGenerator &generator) { return true; }
};


class InitiatorNameValidator : public GenericPairValidator {
public:
	bool valid(const string &value) const;
	bool invalid(const string &value) const { return !valid(value); }
};

class InitiatorNameHandler : public GenericPairHandler {
public:
	bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
		    LoginGenerator &generator);
};

class InitiatorAliasHandler : public GenericPairHandler {
public:
	bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
		    LoginGenerator &generator);
};

class AuthMethodValidator : public GenericPairValidator {
public:
	bool valid(const string &value) const;
	bool invalid(const string &value) const { return !valid(value); }
};

class AuthMethodHandler : public GenericPairHandler {
public:
	bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
		    LoginGenerator &generator);
};

class SessionTypeValidator : public GenericPairValidator {
public:
	bool valid(const string &value) const;
	bool invalid(const string &value) const { return !valid(value); }
};

class TargetNameValidator : public GenericPairValidator {
public:
	bool valid(const string &value) const;
	bool invalid(const string &value) const { return !valid(value); }
};

class TargetAliasHandler : public GenericPairHandler {
public:
	bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
		    LoginGenerator &generator);
};

class DigestValidator : public GenericPairValidator {
public:
	bool valid(const string &value) const;
	bool invalid(const string &value) const { return !valid(value); }
};

class DigestHandler : public GenericPairHandler {
public:
	DigestHandler(const string iattr) : attr(iattr) {}
	bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
		    LoginGenerator &generator);
private:
	string attr;
};

class MaxRecvDataSegmentLengthHandler : public GenericPairHandler {
public:
	bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
		    LoginGenerator &generator);
};

class BoolStringValidator : public GenericPairValidator {
	bool valid(const string &value) const;
	bool invalid(const string &value) const { return !valid(value); }	
};

template<typename T>
class NegotiateMinHandler : public GenericPairHandler {
public:
	NegotiateMinHandler(const string iattr) : attr(iattr) {}
	bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
		    LoginGenerator &generator)
	{
		T negotiated;
		if(!parent->getAttr(attr,negotiated))
			LOG4CXX_WARN(logger, attr + " not found");
		negotiated = min(negotiated, boost::lexical_cast<T>(value));
		parent->setAttr(attr,negotiated);
		generator.addKeyValue(attr, boost::lexical_cast<string>(negotiated));
		
		LOG4CXX_DEBUG(logger, attr + " : " + value);
		return true;
	}
private:
	string attr;
};

template<typename T>
class NegotiateMaxHandler : public GenericPairHandler {
public:
	NegotiateMaxHandler(const string iattr) : attr(iattr) {}
	bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
		    LoginGenerator &generator)
	{
		T negotiated = 0;
		if(!parent->getAttr(attr,negotiated))
			LOG4CXX_WARN(logger, attr + " not found");
		negotiated = max(negotiated, boost::lexical_cast<T>(value));
		parent->setAttr(attr,negotiated);
		generator.addKeyValue(attr, boost::lexical_cast<string>(negotiated));
		
		LOG4CXX_DEBUG(logger, attr + " : " + value);
		return true;
	}
private:
	mutable string attr;
};

template<typename T>
class NormalMinHandler : public GenericPairHandler {
public:
	NormalMinHandler(const string iattr) : attr(iattr) {}
	bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
		    LoginGenerator &generator)
	{
		if(parent->type.get() != iSCSI_NORMAL_SESSION){
			LOG4CXX_ERROR(logger, attr + " is irrelevant when: ( SessionType=Discovery )");
			return false;
		}
		T negotiated = 0;
		if(!parent->getAttr(attr,negotiated))
			LOG4CXX_WARN(logger, attr + " not found");
		negotiated = min(negotiated, boost::lexical_cast<T>(value));
		parent->setAttr(attr,negotiated);
		generator.addKeyValue(attr, boost::lexical_cast<string>(negotiated));
		
		LOG4CXX_DEBUG(logger, attr + " : " + value);
		return true;
	}
private:
	mutable string attr;
};

class NormalOrHandler : public GenericPairHandler {
public:
	NormalOrHandler(const string iattr) : attr(iattr) {}
	bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
		    LoginGenerator &generator);
private:
	mutable string attr;
};

class NormalAndHandler : public GenericPairHandler {
public:
	NormalAndHandler(const string iattr) : attr(iattr) {}
	bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
		    LoginGenerator &generator);
private:
	mutable string attr;
};

class NegotiateAndHandler : public GenericPairHandler {
public:
	NegotiateAndHandler(const string iattr) : attr(iattr) {}
	bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
		    LoginGenerator &generator);
private:
	mutable string attr;
};


class SendTargetsValidator : public GenericPairValidator {
public:
	bool valid(const string &value) const;
	bool invalid(const string &value) const { return !valid(value); }
};

class SendTargetsHandler {
public:
	bool handle(boost::shared_ptr<iSCSISession> parent, const string &value,
		    TextGenerator &generator);
};

#endif /* __ISCSI_KEYVALUE_H__ */
