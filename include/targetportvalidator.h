/**
   @file targetportvalidator.h
   @brief TargetPort validator
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: targetportvalidator.h 40 2007-04-16 05:59:01Z sugiura $
 */
#ifndef __TARGETPORTVALIDATOR_H__
#define __TARGETPORTVALIDATOR_H__

#include <stdexcept>

using namespace std;

/** TargetPort Validator class */
class TargetPortValidator {
public:
	TargetPortValidator() {}
	virtual ~TargetPortValidator() {}
	/** validator
	    @return true     : valid
	    @return false    : invalid
	 */
 	bool valid() const;
};
#endif /* __TARGETPORTVALIDATOR_H__ */
