/**
   @file initiator.h
   @brief iSCSI Initiator classes
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: initiator.h 91 2007-05-22 06:54:44Z sugiura $
 */
#ifndef __ISCSI_INITIATOR_H__
#define __ISCSI_INITIATOR_H__

#include <boost/shared_ptr.hpp>
#include "initiatorconfigurator.h"
#include "iscsi/protocol.h"

using namespace std;

class LogicalVolume;

/** Initiator cache class */
class iSCSIInitiator {
public:
	/** Constructor */
	iSCSIInitiator(const string  &iname);
	/** Destructor */
	~iSCSIInitiator() {}

	/** InitiatorName getter */
	string getName() const { return name; }
	/** InitiatorAlias getter */
	string getAlias() const { return alias; }
	/** InitiatorAlias setter */
	void setAlias(const string &ialias) { alias = ialias; }
	/** add Authentication info */
	void addAuthInfo(const AuthInfoiSCSI &info) { authens.push_back(info); }
	/** MaxRecvDataSegmentLength setter
	    @param mrds_length : the value
	*/
	void setMaxRecvDataSegmentLength(const uint32_t mrds_length)
	{ max_recv_data_segment_length = mrds_length; }
	/* MaxRecvDataSegmentLength getter */
	uint32_t getMaxRecvDataSegmentLength() const
	{ return max_recv_data_segment_length; }
	/** LUN list getter */
	vector<string> getLUNnames() const;
	/** LUN getter */
	boost::shared_ptr<LogicalVolume> getLUN(uint32_t lun) const;
private:
	/** Name */
	string name;
	/** Alias */
	string alias;
	/** Authenticate infos */
	vector<AuthInfoiSCSI> authens;
	/** MaxRecvDataSegmentLength */
	uint32_t max_recv_data_segment_length;
	static const size_t DEFAULT_MAX_RECV_DATA_SEGMENT_LENGTH = 8192;
	/** LogicalVolume list */
	vector<boost::shared_ptr<LogicalVolume> > lvs;
};

typedef boost::shared_ptr<iSCSIInitiator> iSCSIInitiatorPtr;

#endif /* __ISCSI_INITIATOR_H__ */
