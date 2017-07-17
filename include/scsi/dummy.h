/**
   @file dummy.h
   @brief SCSI Dummy handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: dummy.h 140 2007-06-14 06:15:02Z sugiura $
 */

#ifndef __SCSI_DUMMY_H__
#define __SCSI_DUMMY_H__

#include <boost/shared_ptr.hpp>
#include "common.h"
#include "volume.h"
#include "scsi/generic.h"

using namespace std;

class iSCSISession;
class DummyHandler : public SCSIHandler {
public:
	DummyHandler();
	bool handle(boost::shared_ptr<iSCSISession> session,
		    const socket_t fd);
private:
	map<uint8_t,string> opstr;
};

#endif /* __SCSI_DUMMY_H__ */
