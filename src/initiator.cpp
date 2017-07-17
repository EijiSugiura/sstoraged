/**
   @file initiator.cpp
   @brief iSCSI Initiator classes
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: initiator.cpp 91 2007-05-22 06:54:44Z sugiura $
 */

#include "common.h"
#include "volume.h"
#include "initiator.h"

iSCSIInitiator::iSCSIInitiator(const string  &iname)
	: name(iname), alias(), authens(),
	  max_recv_data_segment_length(DEFAULT_MAX_RECV_DATA_SEGMENT_LENGTH)
{
	/** Load LUN list */
	InitiatorConfigurator initiator;
	InitiatorConfigurator::iterator itr = initiator.find(iname);
	for(InitiatorInfo::lun_iterator lun = itr->lun_begin();
	    lun != itr->lun_end(); ++lun){
		/** initialize Logical/Physical volume */
		VolumeConfigurator volumes;
		try {
			VolumeConfigurator::iterator volume = volumes.find(lun->getKey());
			lvs.push_back(LogicalVolumePtr(new LogicalVolume(*volume)));
		} catch (const exception &e) {
			throw runtime_error(e.what());
		}
	}
}

vector<string> iSCSIInitiator::getLUNnames() const
{
	vector<string> names;
	for(vector<boost::shared_ptr<LogicalVolume> >::const_iterator itr = lvs.begin();
	    itr != lvs.end(); ++itr){
		names.push_back((*itr)->getName());
	}
	return names;
}

LogicalVolumePtr iSCSIInitiator::getLUN(uint32_t lun) const
{
	/** check existence */
	if(lun >= lvs.size()){
		LogicalVolumePtr null;
		return null;
	}
	return lvs[lun];
}
