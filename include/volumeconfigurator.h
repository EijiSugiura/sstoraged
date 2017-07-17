/**
   @file volumeconfigurator.h
   @brief Volume info Configurator
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: volumeconfigurator.h 312 2007-09-28 00:56:17Z sugiura $
 */

#ifndef __VOLUMECONFIGURATOR_H__
#define __VOLUMECONFIGURATOR_H__

#include "common.h"

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#include <iostream>
#include "taggedconfigurator.h"

using namespace std;

/** Real Volume info class */
class RealVolume : public ConfigInfo<string> {
public:
	RealVolume(const string &aHost, const string &aPath,
		   const uint64_t &offset, const uint64_t &sectors)
		: host(), path(aPath), start(offset), count(sectors)
	{
		setHost(aHost);
	}
	RealVolume(const Json::Value &value, const size_t &key);
	virtual ~RealVolume() {}
	/** key getter */
	virtual string getKey() const
	{
		ostringstream os;
		os << getHostStr() << ":" << path <<
			":" << (unsigned long)start <<
			":" << (unsigned long)count;
		return os.str();
	}
	/** member serializer */
	virtual Json::Value serialize() const
	{
		Json::Value tmp;
		tmp["Host"] = getHostStr();
		tmp["Path"] = path;
		tmp["Start"] = start;
		tmp["Count"] = count;
		return tmp;
	}
	/** host getter */
	sockaddr_storage getHost() const { return host; }
	/** host string getter */
	string getHostStr() const;
	/** host setter
	    @param addr_str : host string to set */
	bool setHost(const string &addr_str);
	/** PATH getter */
	string getPath() const { return path; }
	/** start offset getter */
	uint64_t getStart() const { return start; }
	/** sector size getter */
	uint64_t getSectorSize() const { return count; }
	/** sector size setter
	    @param size : the sector size */
	bool setSectorSize(uint64_t size);
	/** validator */
	bool valid() const;
private:
	/** host address */
	sockaddr_storage host;
	/** path to real volume */
	string path;
	/** start offset */
	uint64_t start;
	/** offset from start to end */
	uint64_t count;
};

/** Array of Real Volumes class */
class RealVolumeArray : public ConfigInfo<size_t> {
public:
	RealVolumeArray(const RealVolume &volume, const size_t &key)
		: index(key), volumes(), total_sectors(0)
	{
		volumes.push_back(volume);
		total_sectors += volume.getSectorSize();
	}
	RealVolumeArray(const Json::Value &value, const size_t &key);
	virtual ~RealVolumeArray() {}
	/** key getter */
	virtual size_t getKey() const
	{
		return index;
	}
	/** member serializer */
	virtual Json::Value serialize() const;
	/** Add new volume */
	bool add(const RealVolume &volume)
	{
		volumes.push_back(volume);
		total_sectors += volume.getSectorSize();
		return true;
	}
	/** Delete exist volume by index */
	bool del(const size_t &aindex)
	{
		if(aindex > volumes.size())
			return false;
		vector<RealVolume>::iterator itr = volumes.begin() + aindex;
		total_sectors -= itr->getSectorSize();
		volumes.erase(itr);
		return true;
	}
	/** Number of volumes getter */
	uint64_t size(){ return volumes.size(); }
	/** Total sector size getter */
	uint64_t getSectorSize() { return total_sectors; }
	/** validator */
	bool valid() const;

	typedef vector<RealVolume>::iterator part_iterator;
	part_iterator begin() { return volumes.begin(); }
	part_iterator end() { return volumes.end(); }

private:
	/** Index of Volumes */
	size_t index;
	/** Volume array */
	vector<RealVolume> volumes;
	/** Total sectors */
	uint64_t total_sectors;
};

/** Volume info class */
class VolumeInfo : public ConfigInfo<string> {
public:
	VolumeInfo(const string &aName);
	VolumeInfo(Json::Value &value, const size_t &key);
	virtual ~VolumeInfo()
	{
		if(orgs){
			delete orgs;
			orgs = NULL;
		}
		if(reps){
			delete reps;
			reps = NULL;
		}
	}
	/** key getter */
	virtual string getKey() const { return name; }
	/** member serializer */
	virtual Json::Value serialize() const
	{
		Json::Value tmp;
		tmp["VolumeName"] = getName();
		return tmp;
	}
	/** name getter */
	string getName() const { return name; }
	/** name setter */
	void setName(const string &input) { name = input; }

	typedef TaggedConfigurator<RealVolume>::iterator part_iterator;

	/** Append volume to original volume */
	bool appendOrgPart(const RealVolume &volume)
	{
		org_total += volume.getSectorSize();
		return orgs->add(volume);
	}
	/** Number of partitions in original volume */
	uint64_t partsOrg() const { return orgs->size(); }
	/** Total sector size of original volume */
	uint64_t sectorSizeOrg() const { return org_total; }
	part_iterator beginOrg() { return orgs->begin(); }
	part_iterator endOrg() { return orgs->end(); }

	typedef TaggedConfigurator<RealVolumeArray,size_t>::iterator volume_iterator;

	/** Create new replica volume */
	bool createRep(const RealVolume &volume, size_t &index)
	{
		index = reps->size();
		RealVolumeArray rep(volume, index);
		return reps->add(rep);
	}
	/** Append volume to index'th replica volume */
 	bool appendRepPart(const size_t index, const RealVolume &volume)
 	{
 		/** @todo: make status shift, if (rep size >= org size) satisfied */
 		volume_iterator itr = reps->find(index);
		if(itr == reps->end())
			return false;
 		itr->add(volume);
		reps->update(*itr);
 		return true;
 	}
 	/** Remove index'th whole replica volume */
 	bool removeRep(const size_t index)
 	{
 		reps->del(index);
 		return true;
 	}

 	/** Number of replica volumes */
 	uint64_t volumesRep() const 
 	{
 		return reps->size();
 	}
 	/** Number of partitions in index'th replica volume */
 	uint64_t partsRep(const size_t index) const
 	{
 		volume_iterator rep = reps->find(index);
		if(rep == reps->end())
			return 0;
 		return rep->size();
 	}
 	/** Total sector size of index'th replica volume */
 	uint64_t sectorSizeRep(const size_t index) const
 	{
 		volume_iterator rep = reps->find(index);
		if(rep == reps->end())
			return 0;
 		return rep->getSectorSize();
 	}
	volume_iterator beginRep() { return reps->begin(); }
	volume_iterator endRep() { return reps->end(); }

	/** @todo: support RealVolume handlings */

	/** Swap 1st replica volume and original one */
	bool swap();
	/** Swap index'th replica volume and original volume */
	bool swap(const size_t index);
	/** VolumeInfo validator */
	bool valid() const;

protected:
	/** Volume name */
	string name;
	/** MAIN volumes */
	TaggedConfigurator<RealVolume> *orgs;
	/** Number of MAIN volumes */
	uint64_t org_total;
	/** COPY volume arrays */
	TaggedConfigurator<RealVolumeArray, size_t > *reps;
};

/** Volume configurator class */
class VolumeConfigurator {
public:
	/** VolumeInfo iterator */
	typedef TaggedConfigurator<VolumeInfo>::iterator iterator;
	/** default constructor */
	VolumeConfigurator();
	/** destructor */
	~VolumeConfigurator();
	/** Volume info iterator : begin */
	iterator begin();
	/** Volume info iterator : end */
	iterator end();
	/** Check if it is empty */
	bool empty() const;
	/** validator */
	bool valid();
	/** Search Volume info by name */
	iterator find(const string &name);
	/** Add new volume -- NEVER USED */
	bool add(const VolumeInfo &info);
	/** Delete exist volume by name */
	bool del(const string &name);
	/** Number of volumes getter */
	uint64_t size() const;
private:
	/** Volume configurator implementation */
	TaggedConfigurator<VolumeInfo> *pimpl;
};

#endif /* __VOLUMECONFIGURATOR_H__ */
