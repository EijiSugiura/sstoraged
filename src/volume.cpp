/**
   @file volume.cpp
   @brief iSCSI Volume classes
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: volume.cpp 312 2007-09-28 00:56:17Z sugiura $
 */

#include "common.h"
#include "logger.h"
#include "volume.h"

bool PhysicalVolume::valid() const
{
	/** check existense of physical volume */
	filehandle_t fd = open64(path.c_str(), O_RDWR|FileHandle::OPENFLAG,
				 FileHandle::OPENMODE);
	if(fd == INVALID_HANDLE_VALUE){
		LOG4CXX_ERROR(logger, "Failed to open Physical Volume : " + path +
			      " (" + boost::lexical_cast<string>(errno) + ")");
		return false;
	}
	if(lseek64(fd, SCSI::LBA2OFFSET(count-1), SEEK_SET) < 0){
		close(fd);
		LOG4CXX_ERROR(logger, "Failed to seek Physical Volume : " + path);
		return false;
	}
	char *buf = NULL;
	if(posix_memalign((void**)&buf, SCSI::LBAUNIT, SCSI::LBAUNIT) < 0) {
		LOG4CXX_ERROR(logger, "Can't allocate aligned buffer");
		return false;
	}
	/** 1st read check */
	if(read(fd, buf, SCSI::LBAUNIT) <= 0){
		/** if failed, extend Physical Volume */
		lseek64(fd, SCSI::LBA2OFFSET(count-1), SEEK_SET);
		memset(buf, 0, SCSI::LBAUNIT);
		if(write(fd, buf, SCSI::LBAUNIT) != static_cast<ssize_t>(SCSI::LBAUNIT)){
			free(buf);
			close(fd);
			LOG4CXX_ERROR(logger, "Failed to validate Physical Volume");
			return false;
		}
		LOG4CXX_INFO(logger, "Physical Volume : " + path 
			     + " extended " + boost::lexical_cast<string>(start) +
			     "+" + boost::lexical_cast<string>(count));
	}
	LOG4CXX_INFO(logger, "Physical Volume : " + path 
		     + " validated");
	free(buf);
	close(fd);
	return true;
}

uint64_t PhysicalVolume::getCoverage(const uint64_t offset, const uint64_t length) const
{
//   	LOG4CXX_DEBUG(logger, "Coverage : sector " +
//   		      boost::lexical_cast<string>(sector) + " offset " +
//   		      boost::lexical_cast<string>(offset) + " length " +
//   		      boost::lexical_cast<string>(length));
	if(offset > sector)
		return 0;
	else if(offset + length < sector)
		return length;
	return (sector - offset);
}

vector<LowHandle> VolumeArray::getLowHandles(const off64_t offset,
					     const uint64_t length,
					     const bool read_only) const
{
	vector<LowHandle> ret;
	off64_t cur = offset;
	uint64_t remain = length;
	for(vector<PhysicalVolumePtr>::const_iterator itr = volumes.begin();
	    itr != volumes.end(); ++itr){
		uint64_t covered = (*itr)->getCoverage(cur, remain);
		LOG4CXX_DEBUG(logger, "getLowHandles : " +
			      boost::lexical_cast<string>(cur) + "+" +
			      boost::lexical_cast<string>(remain) + " " +
			      boost::lexical_cast<string>(covered));
		if(covered > 0){
			FileHandlePtr handle = (read_only)?
				(*itr)->getReadDescriptor():
				FileHandlePtr(new FileWriteHandle((*itr)->getPath()));
			ret.push_back(LowHandle(handle,
						((*itr)->getStart()+cur), covered));
			remain -= covered;
			if(remain == 0)
				break;
			cur = 0;
		} else {
			cur -= (*itr)->getSector();
		}
	}
	return ret;
}

LogicalVolume::LogicalVolume(VolumeInfo  &info)
	: name(info.getName())
{
	/** build MAIN volume */
	original = VolumeArrayPtr(new VolumeArray);
	for(VolumeInfo::part_iterator part = info.beginOrg();
	    part != info.endOrg(); ++part){
		PhysicalVolumePtr pv =
			PhysicalVolumePtr(new PhysicalVolume(part->getHost(),
							     part->getPath(),
							     part->getStart(),
							     part->getSectorSize()));
		if(pv->valid())
			original->addVolume(pv);
		else {
			ostringstream os;
			os << "Ignored Physical Volume : \"" << pv->getPath()
			   << "\" in Logical Volume(Main) : " << name;
			LOG4CXX_ERROR(logger, os.str());
			/** @todo : throw exception */
			throw runtime_error(os.str());
		}
	}

	/** set total volume size */
	this->count = original->getTotalSize();

	/** build COPY volume */
	size_t counter = 0;
	for(VolumeInfo::volume_iterator volume = info.beginRep();
	    volume != info.endRep(); ++volume){
		VolumeArrayPtr vols = VolumeArrayPtr(new VolumeArray);
		for(RealVolumeArray::part_iterator part = volume->begin();
		    part != volume->end(); ++part){
			PhysicalVolumePtr pv =
				PhysicalVolumePtr(new PhysicalVolume(part->getHost(),
								     part->getPath(),
								     part->getStart(),
								     part->getSectorSize()));
			if(pv->valid())
				vols->addVolume(pv);
			else {
				LOG4CXX_ERROR(logger, "Ignored Physical Volume : \"" + pv->getPath()
					      + "\" in Logical Volume(Copy) : " + name);
				/** @todo : throw exception? */
				return;
			}
		}
		counter++;
		if(this->count > vols->getTotalSize()){
			LOG4CXX_ERROR(logger, "Copy Logical Volume[" + 
				      boost::lexical_cast<string>(counter)
				      + "] is ignored, too small for \"" + name + "\"");
		} else {
			copies.push_back(vols);
		}
	}
	/** @todo : check sector sizes */
}
