/** 
 * @file  scsi/generic.h
 * @brief SCSI generic header definitions
 * @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
 * @version $Id: generic.h 228 2007-07-23 04:42:42Z sugiura $
 */
#ifndef __SCSI_GENERIC_H__
#define __SCSI_GENERIC_H__

#include <boost/shared_ptr.hpp>

using namespace std;

#ifndef REPORT_LUNS
#  define REPORT_LUNS (0xa0)
#endif
#ifndef MAINTENANCE_IN
#  define MAINTENANCE_IN (0xa3)
#endif
#ifndef SERVICE_ACTION_IN_16
#  define SERVICE_ACTION_IN_16 (0x9e)
#endif

/** scsi Generic Header */
struct SCSICDBHeader {
	uint8_t cdb[16];
};

template<typename T>
class SCSIParser {
public:
	/** Constructor with CDB pointer */
	SCSIParser(T *ptr) : cdb(ptr) {}
	virtual ~SCSIParser() {}
	uint8_t getOpCode() const { return cdb->opcode; }
protected:
	/** CDB header */
	mutable T *cdb;
};

template<typename T>
class SCSIGenerator {
public:
	/** Default Constructor */
	SCSIGenerator() : cdb(NULL) {}
	/** Destructor */
	virtual ~SCSIGenerator() {}
protected:
	/** CDB header */
	T *cdb;
};

class iSCSISession;
class SCSIHandler {
public:
	SCSIHandler() {}
	virtual ~SCSIHandler() {}
	virtual bool handle(boost::shared_ptr<iSCSISession> session, 
			    const socket_t fd) = 0;
};

typedef boost::shared_ptr<SCSIHandler> SCSIHandlerPtr;

template<typename T>
class DataSegment {
public:
	DataSegment(T *ptr, const size_t size)
		: data(ptr), max_size(size)
	{
		if(max_size >= sizeof(T)){
			// call default constructor
			T tmp;
			// and copy it
			*data = tmp;
		}
	}
	virtual ~DataSegment() {}
	size_t getLimit() const { return max_size; }
protected:
	T *data;
	size_t max_size;
};

struct FixedSenseParam {
	uint8_t response_code;
	uint8_t obsolute;
	uint8_t sense_key;
	uint8_t information[4];
	uint8_t additional_sense_length;
	uint8_t command_specific_information[4];
	uint8_t additional_sense_code;
	uint8_t additional_sense_code_qualifier;
	FixedSenseParam()
		: response_code(0x70), // CurrentError & FixedFormat
		  obsolute(0), sense_key(ILLEGAL_REQUEST),
		  information(), additional_sense_length(6),
		  command_specific_information(),
		  additional_sense_code(0x24), // INVALID FIELD IN CDB
		  additional_sense_code_qualifier(0x00)
	{}
};

class FixedSenseData : public DataSegment<FixedSenseParam> {
public:
	FixedSenseData(FixedSenseParam *ptr)
		: DataSegment<FixedSenseParam>(ptr,this->SIZE) {}
	~FixedSenseData() {}
	void setResponseCode(const uint8_t code) { data->response_code = code; }
	void setSenseKey(const uint8_t key) { data->sense_key = key; }
	void setAdditionalSenseLength(const uint8_t length)
	{ data->additional_sense_length = length; }
	void setAdditionalSenseCode(const uint8_t code)
	{ data->additional_sense_code = code; }
	void setAdditionalSenseCodeQualifier(const uint8_t qualifier)
	{ data->additional_sense_code_qualifier = qualifier; }
	const static size_t SIZE =  sizeof(FixedSenseParam);
};

#endif /* __SCSI_GENERIC_H__ */
