/**
   @file configbinder.cpp
   @brief Configuration File Binder
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: configbinder.cpp 25 2007-02-20 09:45:49Z sugiura $
 */

#include <iostream>
#include <fstream>
#include "common.h"
#include "configurator.h"
#include "configbinder.h"

bool ConfigBinder::ConfigBinderImpl::append(const string& file)
{
	try {
		ifstream fin(file.c_str());
		if(fin.fail())
			throw std::runtime_error("Failed to open : " + file);
		while(!fin.eof()){
			char buf[MAX_CHAR_PER_LINE]={};
			fin.getline(buf, sizeof(buf));
			lines += buf;
			lines += '\n';
		}
		fin.close();
	} catch (const std::exception &e) {
		throw std::runtime_error(e.what());
	}
	return true;
}

bool ConfigBinder::ConfigBinderImpl::parse()
{
	Configurator &config = Configurator::getInstance();
	return config.parse(lines);
}

bool ConfigBinder::ConfigBinderImpl::dump(const string& file)
{
	Configurator &config = Configurator::getInstance();
	try {
		lines = config.dump();
	} catch (const std::exception &e){
		throw std::runtime_error(e.what());
	}

	try {
		ofstream fout(file.c_str());
		if(fout.fail())
			throw std::runtime_error("Failed to open : " + file);
		fout.write(lines.c_str(), lines.size());
		fout.close();
	} catch (const std::exception &e) {
		throw std::runtime_error(e.what());
	}
	return true;
}

bool ConfigBinder::ConfigBinderImpl::validate() const
{
	Configurator &config = Configurator::getInstance();
	bool ret = false;
	try {
		ret = config.validate();
	} catch (const std::exception &e) {
		throw std::runtime_error(e.what());
	}
	return ret;
}
