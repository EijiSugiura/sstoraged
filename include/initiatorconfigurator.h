/**
   @file initiatorconfigurator.h
   @brief Initiator info Configurator
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: initiatorconfigurator.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __INITIATORCONFIGURATOR_H__
#define __INITIATORCONFIGURATOR_H__

#include "common.h"

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#include "targetconfigurator.h"

using namespace std;

/** iSCSI authenticate information class*/
class AuthInfoiSCSI {
public:
	/**
	   @param aMethod : auth method
	   @param aState : state
	   @todo: validate
	*/
	AuthInfoiSCSI(const string &aMethod, const string &aState)
		: method(aMethod), state(aState) {}
	~AuthInfoiSCSI() {}
	/** Method getter */
	string getMethod() const { return method; }
	/** State getter */
	string getState() const { return state; }
	/** Name getter */
	string getName() const { return name; }
	/** Name setter
	    @param aName : name to set
	 */
	void setName(const string &aName) { name = aName; }
	/** Password getter */
	string getPassword() const { return password; }
	/** Password setter
	    @param aPassword : password to set
	 */
	void setPassword(const string &aPassword) { password = aPassword; }
	/** equal operator */
	inline bool operator == (const string & method)
	{ return (this->method == method); }

private:
	/** auth method */
	string method;
	/** state */
	string state;
	/** name */
	string name;
	/** password */
	string password;
};

/** LUN information class */
class LUNinfo : public ConfigInfo<string> {
public:
	LUNinfo(const string &aLun) : lun(aLun) {}
	LUNinfo(const Json::Value &value, const size_t &key)
	{
		lun = value.asString();
		if(lun.length() == 0)
			throw runtime_error("empty LUN is specified\n");
	}
	/** key getter */
	virtual string getKey() const { return lun; }
	/** LUN getter */
	string get() const { return lun; }
	/** member serializer */
	virtual Json::Value serialize() const
	{
		Json::Value tmp(lun);
		return tmp;
	}
	/** validator */
	bool valid() const { return true; }
private:
	/** LUN name */
	string lun;
};

/** Initiator information class */
class InitiatorInfo : public TargetInfo {
public:
 	InitiatorInfo(const string& aName, const string &aAddress)
		: TargetInfo(aName, aAddress), luns (NULL) {}
 	InitiatorInfo(const string& aName, const sockaddr_storage &aAddress,
		      const string& aAlias)
		: TargetInfo(aName, aAddress, aAlias), luns (NULL) {}
 	InitiatorInfo(const string& aName, const string &aAddress,
 		   const string& aAlias)
		: TargetInfo(aName, aAddress, aAlias), luns (NULL) {}
 	InitiatorInfo(Json::Value &value, const size_t &key);
	virtual ~InitiatorInfo()
	{
 		if(luns){
 			delete luns;
 			luns = NULL;
 		}
	}
	/** member serializer */
 	virtual Json::Value serialize() const;

	/** Peer validator/authenticator */
	bool validPeer(const AuthInfoiSCSI &info) const;
	/** My Authentication info. getter */
	bool getMyAuth(const string &state, vector<AuthInfoiSCSI> &infos) const;
	/** @todo : support AuthInfoiSCSI handling add/del... */

	/** LUN iterator */
	typedef TaggedConfigurator<LUNinfo>::iterator lun_iterator;
	/** LUN iterator : begin */
	lun_iterator lun_begin() { return luns->begin(); }
	/** LUN iterator : end */
	lun_iterator lun_end() { return luns->end(); }
	/** LUN iterator : size */
	size_t lun_size() { return luns->size(); }
	/** Add LUN */
	bool addLUN(const LUNinfo &info)
	{
		/** @todo: check uniqueness of LUN */
		return luns->add(info);
	}
	/** Delete LUN */
	bool delLUN(const string &lun)
	{ return luns->del(lun); }
	/** validator */
	bool valid() const;

protected:
	using TargetInfo::name;
	using TargetInfo::alias;
	using TargetInfo::address;

private:
	/** LUN info */
	TaggedConfigurator<LUNinfo> *luns;
	/** Peer Auth infos */
	vector<AuthInfoiSCSI> peers;
	/** My Auth infos */
	vector<AuthInfoiSCSI> myowns;
};

/** Initiator Configurator class */
class InitiatorConfigurator {
public:
	/** InitiatorInfo iterator */
	typedef TaggedConfigurator<InitiatorInfo>::iterator iterator;
	/** default constructor */
	InitiatorConfigurator();
	/** destructor */
	~InitiatorConfigurator();
	/** InitiatorInfo iterator : begin */
	iterator begin();
	/** InitiatorInfo iterator : end */
	iterator end();
	/** Check if it is empty */
	bool empty() const;
	/** validator */
	bool valid();
	/** Add new InitiatorInfo */
	bool add(const InitiatorInfo &info);
	/** Delete exist InitiatorInfo */
	bool del(const string &name);
	/** Search exist InitiatorInfo by name */
	iterator find(const string &name);
	/** InitiatorInfo iterator : size */
	size_t size();
private:
	/** InitiatorConfigurator implementation */
	TaggedConfigurator<InitiatorInfo> *pimpl;
};

#endif /* __INITIATORCONFIGURATOR_H__ */
