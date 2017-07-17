/**
   @file targetportconfigurator.cpp
   @brief Target info Configurator
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: targetportconfigurator.cpp 40 2007-04-16 05:59:01Z sugiura $
 */

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "common.h"
#include "targetportconfigurator.h"

TargetPortConfigurator::TargetPortConfigurator() : pimpl(NULL)
{
	Configurator &config = Configurator::getInstance();
	/** @todo: make this more smart way... */
	pimpl = new TaggedConfigurator<TargetPortInfo>(config.pimpl->root,
						       "TargetPort");
	if(this->empty())
		add(TargetPortInfo(DEFAULT_TARGETPORT));
}

TargetPortConfigurator::~TargetPortConfigurator()
{
	if(pimpl){
		delete pimpl;
		pimpl = NULL;
	}
}

TaggedConfigurator<TargetPortInfo>::iterator TargetPortConfigurator::begin()
{
	return pimpl->begin();
}

TaggedConfigurator<TargetPortInfo>::iterator TargetPortConfigurator::end()
{
	return pimpl->end();
}

bool TargetPortConfigurator::empty() const
{
	return (pimpl->size() == 0);
}

bool TargetPortConfigurator::valid()
{
	if(this->empty())
		return false;
	for(TargetPortConfigurator::iterator itr = this->begin();
	    itr != this->end(); ++itr){
		if(!itr->valid())
			return false;
	}
	return true;
}

bool TargetPortConfigurator::add(const TargetPortInfo &info)
{
	return pimpl->add(info);
}

bool TargetPortConfigurator::del(const string &name)
{
	return pimpl->del(name);
}

size_t TargetPortConfigurator::size() const
{
	return pimpl->size();
}
