/**
   @file targetportconfigurator.h
   @brief Target info Configurator
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: targetportconfigurator.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __TARGETPORTCONFIGURATOR_H__
#define __TARGETPORTCONFIGURATOR_H__

#include "common.h"

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include "taggedconfigurator.h"
#include "validator.h"

using namespace std;

/** TargetPort information class */
class TargetPortInfo : public ConfigInfo<string> {
public:
	TargetPortInfo() : port() {}
	TargetPortInfo(const string &aPort) : port(aPort) {}
	TargetPortInfo(const Json::Value &value, const size_t &key)
	{
		port = value.asString();
	}
	/** key getter */
	virtual string getKey() const { return port; }
	/** TargetPort getter */
	string get() const { return port; }
	/** member serializer */
	virtual Json::Value serialize() const
	{
		Json::Value tmp(port);
		return tmp;
	}
	/** validator */
	bool valid() const
	{
		try {
			AddrStrValidator addrstr;
			return addrstr.valid(port);
		} catch (const std::exception &e) {
			throw std::runtime_error("Illegal TargetPort\"" + port + "\""
						 + e.what() + "\n");
		}
	}
private:
	/** TargetPort [address]:port */
	string port;
};

/** TargetPort Configurator class */
class TargetPortConfigurator {
public:
	/** TargetPortInfo iterator */
	typedef TaggedConfigurator<TargetPortInfo>::iterator iterator;
	/** default constructor */
	TargetPortConfigurator();
	/** destructor */
	~TargetPortConfigurator();
	/** TargetPortInfo iterator : begin */
	iterator begin();
	/** TargetPortInfo iterator : end */
	iterator end();
	/** Check if it is empty */
	bool empty() const;
	/** validator */
	bool valid();
	/** Add new TargetPortInfo */
	bool add(const TargetPortInfo &info);
	/** Delete exist TargetPortInfo by key */
	bool del(const string &name);
	/** Get number of ports */
	size_t size() const;
private:
	/** TargetPortConfigurator implementation */
	TaggedConfigurator<TargetPortInfo> *pimpl;
};

#endif /* __TARGETPORTCONFIGURATOR_H__ */
