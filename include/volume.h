/**
   @file volume.h
   @brief Volume classes
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: volume.h 212 2007-07-18 06:54:55Z sugiura $
 */
#ifndef __ISCSI_VOLUME_H__
#define __ISCSI_VOLUME_H__

#include <fcntl.h>
#include <boost/shared_ptr.hpp>
#include "volumeconfigurator.h"
#include "filehandle.h"

using namespace std;

/** Physical Volume cache class */
class PhysicalVolume {
public:
	/** Constructor */
	PhysicalVolume(const sockaddr_storage &_host,
		       const string &_path, 
		       const uint64_t _start, const uint64_t _count)
		: host(_host), path(_path), start(_start), count(_count),
		  sector(SCSI::LBA2OFFSET(count)),
		  descriptor(FileReadHandlePtr(new FileReadHandle(path))) {}
	/** Destructor */
	virtual ~PhysicalVolume() {}
	/** Validator
	    @return validity
	 */
	bool valid() const;

	/** Host address getter
	    @return host address
	*/
	sockaddr_storage & getHost() const { return host; }
	/** PATH getter
	    @return file/device PATH
	*/
	string & getPath() const { return path; }
	/** Start offset getter
	    @return start offset
	*/
	uint64_t getStart() const { return start; }
	/** Count in LBA unit getter
	    @return sector size
	*/
	uint32_t getCount() const { return count; }
	/** Sector in Byte size getter
	    @return sector size
	*/
	uint64_t getSector() const { return sector; }

	/** get coverage size
	    @param offset : start offset
	    @param length : length to cover
	    @return coverage size
	 */
	uint64_t getCoverage(const uint64_t offset, const uint64_t length) const;

	/** read-only file descriptor getter */
	FileHandlePtr getReadDescriptor() const { return descriptor; }

protected:
	/** host address */
	mutable sockaddr_storage host;
	/** path to physical volume */
	mutable string path;
	/** start offset in LBA unit */
	mutable uint64_t start;
	/** offset from start to end in LBA unit */
	mutable uint32_t count;
	/** offset from start to end in Byte unit */
	mutable uint64_t sector;
	/** read only file descriptor */
	FileReadHandlePtr descriptor;
private:
};

typedef boost::shared_ptr<PhysicalVolume> PhysicalVolumePtr;

/** Volume Array cache class */
class VolumeArray {
public:
	/** Constructor */
	VolumeArray() : count(0) {}
	/** add new physical volume
	    @param volume : physical volume to add
	    @return success or failure
	 */
	bool addVolume(PhysicalVolumePtr volume)
	{
		volumes.push_back(volume);
		count += volume->getCount();
		return true;
	}
	/** replace exist volume to new one
	   @param offset : offset to swap point
	   @param volume : new volume
	   @return success or failure
	 */
	bool replaceVolume(const uint64_t offset,
			PhysicalVolumePtr volume);

	/** Total size getter 
	    @return volume total size */
	uint64_t getTotalSize() const { return count; }


	/** Low disk image accessor getter
	    @param offset : start offset
	    @param length : tranfer length
	    @param read_only : read or write handle flag
	    @return LowHandle vector
	    @todo : handle copy volumes for load balancing
	 */
	vector<LowHandle> getLowHandles(const off64_t offset,
					const uint64_t length,
					const bool read_only) const;
protected:
	vector<PhysicalVolumePtr> volumes;
	/** volume total size */
	uint64_t count;
};

typedef boost::shared_ptr<VolumeArray> VolumeArrayPtr;

/** Logical Volume cache class */
class LogicalVolume {
public:
	/** Constructor */
	LogicalVolume(VolumeInfo  &info);
	/** Destructor */
	~LogicalVolume() {}

	/** Name getter
	    @return volume name
	 */
	string getName() const { return name; }

	/** swap 1st valid Copy Volume & Main Volume
	    @return success or failure
	 */
	bool swapCopy2Main();

	/** Total size getter 
	    @return volume total size */
	uint64_t getTotalSize() const { return count; }

	/** Low disk image accessor getter
	    @param offset : start offset
	    @param length : tranfer length
	    @param read_only : read or write handle flag
	    @return LowHandle vector
	    @todo : handle copy volumes for load balancing
	 */
	vector<LowHandle> getLowHandles(const off64_t offset,
					const uint64_t length,
					const bool read_only) const
	{ return original->getLowHandles(offset, length, read_only); }

private:
	/** Name */
	string name;
	/** the Main Volume */
	VolumeArrayPtr original;
	/** Copy Volumes */
	vector<VolumeArrayPtr> copies;

	/** total number of sector */
	uint64_t count;
};

typedef boost::shared_ptr<LogicalVolume> LogicalVolumePtr;

#endif /* __ISCSI_VOLUME_H__ */
