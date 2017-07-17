/**
   @file targetconfigurator.h
   @brief Target info Configurator
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: targetconfigurator.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __TARGETCONFIGURATOR_H__
#define __TARGETCONFIGURATOR_H__

#include "common.h"

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include "taggedconfigurator.h"

using namespace std;

/** Target info class */
class TargetInfo : public ConfigInfo<string> {
public:
	TargetInfo(const string& aName, const string &aAddress)
		: name(aName), alias(aName), address()
	{ setAddr(aAddress); }
	TargetInfo(const string& aName, const sockaddr_storage &aAddress,
		   const string& aAlias) : name(aName), alias(aAlias), address(aAddress)
	{}
	TargetInfo(const string& aName, const string &aAddress,
		   const string& aAlias) : name(aName), alias(aAlias), address()
	{ setAddr(aAddress); }
	TargetInfo(const Json::Value &value, const size_t &key);
	virtual ~TargetInfo() {}
	/** key getter */
	virtual string getKey() const { return name; }
	/** member serialize */
	virtual Json::Value serialize() const;
	/** name getter */
	string getName() const { return name; }
	/** name setter */
	void setName(const string &input) { name = input; }
	/** alias getter */
	string getAlias() const { return alias; }
	/** alias setter */
	void setAlias(const string &input) { alias = input; }
	/** address getter */
	sockaddr_storage getAddr() const { return address; }
	/** address string getter */
	string getAddrPortStr() const;
	/** address setter
	    @param input : sockaddr address to set
	 */
	void setAddr(const sockaddr_storage &input) { address = input; }
	/** address setter
	    @param addr_str : address string to set
	*/
	bool setAddr(const string &addr_str);
	/** validator */
	virtual bool valid() const;

protected:
	/** target name */
	string name;
	/** target alias */
	string alias;
	/** target address */
	sockaddr_storage address;
};

/** Target Configurator class */
class TargetConfigurator {
public:
	/** TargetInfo iterator */
	typedef TaggedConfigurator<TargetInfo>::iterator iterator;
	/** default constructor */
	TargetConfigurator();
	/** destructor */
	~TargetConfigurator();
	/** TargetInfo iterator : begin */
	iterator begin();
	/** TargetInfo iterator : end */
	iterator end();
	/** Check if it is empty */
	bool empty() const;
	/** validator */
	bool valid();
	/** Add new TargetInfo */
	bool add(const TargetInfo &info);
	/** Delete exist TargetInfo by key */
	bool del(const string &name);
	/** Search exist TargetInfo by name */
	iterator find(const string &name);
private:
	/** TargetConfigurator implementation */
	TaggedConfigurator<TargetInfo> *pimpl;
};

#endif /* __TARGETCONFIGURATOR_H__ */
