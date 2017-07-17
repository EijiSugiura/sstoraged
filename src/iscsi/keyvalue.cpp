/**
   @file keyvalue.cpp
   @brief iSCSI Key/Value pair validators
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: keyvalue.cpp 262 2007-08-12 08:14:11Z sugiura $
*/

#include <boost/lexical_cast.hpp>
#include <boost/assign/std/vector.hpp>
#include "logger.h"
#include "inetutils.h"
#include "initiatorconfigurator.h"
#include "initiator.h"
#include "targetportconfigurator.h"
#include "session.h"
#include "iscsi/keyvalue.h"

bool AlwaysTrueValidator::valid(const string &value) const
{
	if(value.length() > VALUE_MAXLEN)
		return false;
	return true;
}

bool InitiatorNameValidator::valid(const string &value) const
{
	if(value.length() > VALUE_MAXLEN)
		return false;
	/** Check existence of InitiatorName */
	InitiatorConfigurator initiator;
	InitiatorConfigurator::iterator itr = initiator.find(value);
	if(itr == initiator.end()){
		LOG4CXX_ERROR(logger, "No such InitiatorName : " + value);
		return false;
	}

	return true;
}

bool InitiatorNameHandler::handle(iSCSISessionPtr parent, const string &value,
				  LoginGenerator &generator)
{
	LOG4CXX_DEBUG(logger, "InitiatorName : " + value);
	try {
		/** initialize initiator & logical volumes */
		iSCSIInitiatorPtr initiator = iSCSIInitiatorPtr(new iSCSIInitiator(value));
		parent->initiator.set(initiator);
	} catch (const exception &e) {
		return false;
	}

	/** this is the 1st LOGIN request for connection */
	if(parent->type.get() == iSCSI_DISCOVERY_SESSION){
		/** alway set TargetPortalGroupTag=1 
		    @todo : support multiple TargetPortalGroupTag
		 */
		parent->tag.set(1UL);
	}
	generator.addKeyValue("TargetPortalGroupTag", boost::lexical_cast<string>(parent->tag.get()));

	return true;
}

bool InitiatorAliasHandler::handle(iSCSISessionPtr parent, const string &value,
				   LoginGenerator &generator)
{
	iSCSIInitiatorPtr initiator = parent->initiator.get();
	initiator->setAlias(value);

	LOG4CXX_DEBUG(logger, "InitiatorAlias : " + value);
	return true;
}

bool AuthMethodValidator::valid(const string &value) const
{
	if(value.length() > VALUE_MAXLEN)
		return false;
	if(value == "None" ||
	   value == "CHAP,None")
		return true;
	LOG4CXX_ERROR(logger, "Invalid AuthMethod : " + value);
	return false;
}

bool AuthMethodHandler::handle(iSCSISessionPtr parent, const string &value,
			       LoginGenerator &generator)
{
	LOG4CXX_DEBUG(logger, "AuthMethod : " + value);
	iSCSIInitiatorPtr initiator = parent->initiator.get();
	string method = "";
	if(value.find("CHAP") != value.npos){
		AuthInfoiSCSI info(static_cast<string>("CHAP"), parent->type.getStr());
		initiator->addAuthInfo(info);
		method = "CHAP";
	}
	if(value.find("None") != value.npos){
		AuthInfoiSCSI info(static_cast<string>("None"), parent->type.getStr());
		initiator->addAuthInfo(info);
		if(method.length() > 0)
			method += ",None";
		else
			method = "None";
	}
	/** @todo : authenticate with InitiatorInfo::validPeer()*/

	generator.addKeyValue("AuthMethod", method);

	return true;
}

bool SessionTypeValidator::valid(const string &value) const
{
	if(value.length() > VALUE_MAXLEN)
		return false;
	if(value == "Discovery" ||
	   value == "Normal")
		return true;
	LOG4CXX_ERROR(logger, "Invalid SessionType : " + value);
	return false;
}

bool TargetNameValidator::valid(const string &value) const
{
	if(value.length() > VALUE_MAXLEN)
		return false;
	string name;
	Configurator &config = Configurator::getInstance();
	if(!config.getAttr("GlobalTargetName", name))
		LOG4CXX_WARN(logger, "TargetName not found");
	if(strncasecmp(value.c_str(),name.c_str(),value.length())){
		LOG4CXX_ERROR(logger, "Invalid TargetName : " + value + " != " + name);
		return false;
	}
	return true;
}

bool TargetAliasHandler::handle(iSCSISessionPtr parent, const string &value,
				LoginGenerator &generator)
{
	LOG4CXX_DEBUG(logger, "TargetAlias : " + value);
	return true;
}

bool DigestValidator::valid(const string &value) const
{
	if(value.length() > VALUE_MAXLEN)
		return false;
	if(value == "None" ||
	   value == "None,CRC32C" ||
	   value == "CRC32C,None")
		return true;
	return false;
}

bool DigestHandler::handle(iSCSISessionPtr parent, const string &value,
			   LoginGenerator &generator)
{
	string tconfig;
	Configurator &config = Configurator::getInstance();
	if(!config.getAttr(attr, tconfig))
		LOG4CXX_WARN(logger, attr + " not found");

	LOG4CXX_DEBUG(logger, attr + " : " + value);
	string negotiated = "";
	if(tconfig.find("CRC32C") != tconfig.npos &&
	   value.find("CRC32C") != value.npos){
		negotiated = "CRC32C";
	}
	if(tconfig.find("None") != tconfig.npos &&
	   value.find("None") != value.npos){
		if(negotiated.length() > 0){
			negotiated += ",None";
		} else {
			negotiated = "None";
		}
	}
	generator.addKeyValue(attr, negotiated);

	/** @todo : store Digest value? */

	return true;
}

bool MaxRecvDataSegmentLengthHandler::handle(iSCSISessionPtr parent, const string &value,
					     LoginGenerator &generator)
{
	iSCSIInitiatorPtr initiator = parent->initiator.get();
	uint32_t mrds_length = boost::lexical_cast<uint32_t>(value);
	initiator->setMaxRecvDataSegmentLength(mrds_length);

	LOG4CXX_DEBUG(logger, "MaxRecvDataSegmentLength : " + value);

	if(!parent->getAttr("MaxRecvDataSegmentLength", mrds_length)){
		LOG4CXX_WARN(logger, "MaxRecvDataSegmentLength not found");
	} else {
		generator.addKeyValue("MaxRecvDataSegmentLength",
				      boost::lexical_cast<string>(mrds_length));
	}
	return true;
}

bool BoolStringValidator::valid(const string &value) const
{
	if(value.length() > VALUE_MAXLEN)
		return false;
	if(value == "No" ||
	   value == "Yes")
		return true;
	return false;
}

bool NormalOrHandler::handle(iSCSISessionPtr parent, const string &value,
			     LoginGenerator &generator)
{
	if(parent->type.get() != iSCSI_NORMAL_SESSION){
		LOG4CXX_ERROR(logger, attr + " is irrelevant when: ( SessionType=Discovery )");
		return false;
	}

	LOG4CXX_DEBUG(logger, attr + " : " + value);
	bool negotiated = false;
	if(!parent->getAttr(attr,negotiated))
		LOG4CXX_WARN(logger, attr + " not found");
	negotiated |= (value == "Yes")?true:false;
	parent->setAttr(attr,negotiated);
	if(negotiated == true)
		generator.addKeyValue(attr, "Yes");
	else
		generator.addKeyValue(attr, "No");
		
	return true;
}


bool NormalAndHandler::handle(iSCSISessionPtr parent, const string &value,
			      LoginGenerator &generator)
{
	if(parent->type.get() != iSCSI_NORMAL_SESSION){
		LOG4CXX_ERROR(logger, attr + " is irrelevant when: ( SessionType=Discovery )");
		return false;
	}

	LOG4CXX_DEBUG(logger, attr + " : " + value);
	bool negotiated = false;
	if(!parent->getAttr(attr,negotiated))
		LOG4CXX_WARN(logger, attr + " not found");
	negotiated &= (value == "Yes")?true:false;
	parent->setAttr(attr,negotiated);
	if(negotiated == true)
		generator.addKeyValue(attr, "Yes");
	else
		generator.addKeyValue(attr, "No");
		
	return true;
}

bool NegotiateAndHandler::handle(iSCSISessionPtr parent, const string &value,
				 LoginGenerator &generator)
{
	LOG4CXX_DEBUG(logger, attr + " : " + value);
	bool negotiated = false;
	if(!parent->getAttr(attr,negotiated))
		LOG4CXX_WARN(logger, attr + " not found");
	negotiated &= (value == "Yes")?true:false;
	parent->setAttr(attr,negotiated);
	if(negotiated == true)
		generator.addKeyValue(attr, "Yes");
	else
		generator.addKeyValue(attr, "No");
		
	return true;
}

bool SendTargetsValidator::valid(const string &value) const
{
	if(value.length() > VALUE_MAXLEN)
		return false;
	if(value != "All")
		return false;
	/** @todo : support <iSCSI-target-name> and <nothing>
	    @see : RFC3720 Appendix D.  SendTargets Operation */
	return true;
}

bool SendTargetsHandler::handle(iSCSISessionPtr parent, const string &value,
				TextGenerator &generator)
{
	LOG4CXX_DEBUG(logger, "SendTargets : " + value);

	/** @see : RFC3720 Appendix D.  SendTargets Operation */

	Configurator &config = Configurator::getInstance();
	string tname;
	if(!config.getAttr("GlobalTargetName", tname))
		LOG4CXX_WARN(logger, "GlobalTargetName not found");
	generator.addKeyValue("TargetName", tname);

	string taddress = "";
	TargetPortConfigurator conf;
	if(conf.size() == 1 && InetUtil::isAnyAddr(conf.begin()->get())){
		/** convert AnyAddress -> address:port */
		uint16_t port = InetUtil::getPort(conf.begin()->get());
		sockaddr_storage tmp = {};
		InetUtil::getSockAddr(conf.begin()->get(), tmp);
		vector<sockaddr_storage> addrs;
		if(!InetUtil::getNICaddrs(tmp.ss_family, addrs))
			return false;
		try {
			for(vector<sockaddr_storage>::iterator addr = addrs.begin();
			    addr != addrs.end(); ++addr){
				if(InetUtil::isLoopbackAddr(*addr))
					continue;
				InetUtil::setPort(*addr, port);
				generator.addKeyValue("TargetAddress", 
						      InetUtil::getAddrPortStr2(*addr) + "," +
						      boost::lexical_cast<string>(parent->tag.get()));
			}
		} catch(...) {
			return false;
		}
	} else {
		for(TargetPortConfigurator::iterator itr = conf.begin();
		    itr != conf.end(); ++itr){
			generator.addKeyValue("TargetAddress", itr->get() + "," +
					      boost::lexical_cast<string>(parent->tag.get()));
		}
	}
	return true;
}
