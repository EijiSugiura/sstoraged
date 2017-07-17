/**
   @file targetportvalidator.cpp
   @brief Targetport Validator
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: targetportvalidator.cpp 40 2007-04-16 05:59:01Z sugiura $
 */
#include "targetportconfigurator.h"
#include "targetportvalidator.h"

bool TargetPortValidator::valid() const
{
	TargetPortConfigurator conf;
	return conf.valid();
}
