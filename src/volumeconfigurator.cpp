/**
   @file volumeconfigurator.cpp
   @brief Volume info Configurator
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: volumeconfigurator.cpp 102 2007-05-29 01:35:12Z sugiura $
 */

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "common.h"
#include "volumeconfigurator.h"
#include "inetutils.h"
#include "volumevalidator.h"
#include "targetconfigurator.h"

RealVolume::RealVolume(const Json::Value &value, const size_t &key)
	: host(), path(), start(), count()
{
	if(!value.isMember("Host")) 
		throw runtime_error("[" + boost::lexical_cast<string>(key) +
				    "] : Host is not specified");
	if(!value.isMember("Path"))
		throw runtime_error("[" + boost::lexical_cast<string>(key) +
				    "] : Path is not specified");
	if(!value.isMember("Start"))
		throw runtime_error("[" + boost::lexical_cast<string>(key) +
				    "] : Start is not specified");
	if(!value.isMember("Count"))
		throw runtime_error("[" + boost::lexical_cast<string>(key) +
				    "] : Count is not specified");
	if(!setHost(value["Host"].asString()))
		throw runtime_error("[" + boost::lexical_cast<string>(key) +
				    "] : Invalid Host : \"" +
				    value["Host"].asString() + "\" specified");
	path = value["Path"].asString();
	start = value["Start"].asUInt();
	count = value["Count"].asUInt();
}

string RealVolume::getHostStr() const
{
	return InetUtil::getAddrPortStr(host);
}

bool RealVolume::setHost(const string &ltarget_str)
{
	TargetConfigurator targets;
	TargetConfigurator::iterator itr = targets.find(ltarget_str);
	if(itr == targets.end())
		return InetUtil::getSockAddr(ltarget_str, host);
	return InetUtil::getSockAddr(itr->getAddrPortStr(), host);
}

bool RealVolume::valid() const
{
	Json::Value tmp = serialize();
	RealVolumeValidator validator;
	return (validator.attrsSatisfied(tmp) &&
		validator.validAttrs(tmp));
}

RealVolumeArray::RealVolumeArray(const Json::Value &value, const size_t &key)
	: index(key), volumes(), total_sectors(0)
{
	if(!value.isArray())
		throw runtime_error(" must be array format\n");
	/** @todo: check existence */
	for(size_t counter = 0; counter < value.size(); ++counter){
		RealVolume volume(value[counter], counter);
		volumes.push_back(volume);
		total_sectors += volume.getSectorSize();
	}
}

Json::Value RealVolumeArray::serialize() const
{
	Json::Value tmp;
	vector<RealVolume>::const_iterator itr = volumes.begin();
	for(;
	    itr != volumes.end(); ++itr){
		tmp.append(itr->serialize());
	}
	return tmp;
}


bool RealVolumeArray::valid() const
{
	try {
		for(vector<RealVolume>::const_iterator itr = volumes.begin();
		    itr != volumes.end(); ++itr){
			if(!itr->valid())
				return false;
		}
	} catch(const exception &e) {
		throw runtime_error("Volume array[" +
				    boost::lexical_cast<string>(getKey()) +
				    "] : " + e.what() + "\n");
	}
	return true;
}

VolumeConfigurator::VolumeConfigurator() : pimpl(NULL)
{
	Configurator &config = Configurator::getInstance();
	/** @todo: make this more smart way... */
	pimpl = new TaggedConfigurator<VolumeInfo>(config.pimpl->root,
						   "Volume");
}

VolumeConfigurator::~VolumeConfigurator()
{
	if(pimpl){
  		delete pimpl;
 		pimpl = NULL;
	}
}

TaggedConfigurator<VolumeInfo>::iterator VolumeConfigurator::begin()
{
	return pimpl->begin();
}

TaggedConfigurator<VolumeInfo>::iterator VolumeConfigurator::end()
{
	return pimpl->end();
}

bool VolumeConfigurator::empty() const
{
	return (pimpl->size() == 0);
}

bool VolumeConfigurator::valid()
{
	if(this->empty())
		return false;
	size_t counter = 0;
	for(VolumeConfigurator::iterator itr = this->begin();
	    itr != this->end(); ++itr,++counter){
		try {
			if(!itr->valid())
				return false;
		} catch(const exception &e) {
			throw runtime_error("Volume[" +
					    boost::lexical_cast<string>(counter) +
					    "] : " + e.what());
		}
	}
	return true;
}

bool VolumeConfigurator::add(const VolumeInfo &info)
{
	return pimpl->add(info);
}

bool VolumeConfigurator::del(const string &name)
{
	return pimpl->del(name);
}

uint64_t VolumeConfigurator::size() const
{
	return pimpl->size();
}

TaggedConfigurator<VolumeInfo>::iterator VolumeConfigurator::find(const string &name)
{
	return pimpl->find(name);
}

VolumeInfo::VolumeInfo(const string &aName)
	: name(aName), orgs(NULL), org_total(0), reps(NULL)
{
	VolumeConfigurator volumes;
	VolumeConfigurator::iterator itr = volumes.find(name);
	if(itr == volumes.end()){
		Configurator &config = Configurator::getInstance();
		/** @todo: make this more smart way... */
		Json::Value tmp;
		tmp["VolumeName"] = name;
		tmp["Main"].resize(0);
		tmp["Copy"].resize(0);
		Json::Value &value = config.pimpl->root["Volume"];
		value.append(tmp);
		orgs = new TaggedConfigurator<RealVolume>(value[value.size()-1], "Main");
		if(!value[value.size()-1].isMember("Main")){			
			throw runtime_error("Main is not resized\n");
		}
		reps = new TaggedConfigurator<RealVolumeArray,size_t>(value[value.size()-1], "Copy");
	}
}

VolumeInfo::VolumeInfo(Json::Value &value, const size_t &key)
	: name(""), orgs(NULL), org_total(0), reps(NULL)
{
	if(value.isMember("VolumeName"))
		name = value["VolumeName"].asString();
	else
		throw runtime_error("VolumeName is not specified\n");
	if(value.isMember("Main")) {
		orgs = new TaggedConfigurator<RealVolume>(value, "Main");
		for(TaggedConfigurator<RealVolume>::iterator itr = orgs->begin();
		    itr != orgs->end(); ++itr)
			org_total += itr->getSectorSize();
	} else
		throw runtime_error("Main is not specified\n");
	if(!value.isMember("Copy")){
		/** dummy Copy entries */
		value["Copy"].resize(0);
	}
	reps = new TaggedConfigurator<RealVolumeArray,size_t>(value, "Copy");
}

bool VolumeInfo::valid() const
{
	/** @todo : duplicate/overwrap entry check */
	try {
		bool ret = true;
		Json::Value tmp = serialize();
		VolumeValidator validator;
		if(!validator.attrsSatisfied(tmp) || !validator.validAttrs(tmp))
			ret = false;
		if(orgs == NULL)
			return ret;
		/** check real volumes */
		for(VolumeInfo::part_iterator part
			    = const_cast<TaggedConfigurator<RealVolume>*>(orgs)->begin();
		    part != const_cast<TaggedConfigurator<RealVolume>*>(orgs)->end();
		    ++part){
			if(!part->valid())
				ret = false;
		}
		if(reps == NULL)
			return ret;
		for(VolumeInfo::volume_iterator volume
			    = const_cast<TaggedConfigurator<RealVolumeArray, size_t>*>(reps)->begin();
		    volume !=  const_cast<TaggedConfigurator<RealVolumeArray, size_t>*>(reps)->end();
		    ++volume){
			if(!volume->valid())
				ret = false;
		}

		return ret;

	} catch(const exception &e) {
		throw runtime_error("Illegal Volume \"" + getKey() + "\" : "
				    + e.what() + "\n");
	}
}
