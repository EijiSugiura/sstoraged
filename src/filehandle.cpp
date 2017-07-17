/**
   @file filehandle.cpp
   @brief File handling classes
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: filehandle.cpp 312 2007-09-28 00:56:17Z sugiura $
 */

#include <errno.h>
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include "filehandle.h"

#ifdef LOG4CXX_COUT
#undef LOG4CXX_DEBUG
#define LOG4CXX_DEBUG(logger,message)	strings.push_back(message)
#undef LOG4CXX_ERROR
#define LOG4CXX_ERROR(logger,message)	strings.push_back(message)
#undef LOG4CXX_INFO
#define LOG4CXX_INFO(logger,message)	strings.push_back(message)
extern vector<string> strings;
#endif

FileHandle::~FileHandle()
{
	if(handle != INVALID_HANDLE_VALUE){
#ifndef HAVE_WINSOCK2_H
		close(handle);
#else
		CloseHandle(handle);
#endif
		LOG4CXX_INFO(logger, "Close Physical Volume : " + path + " [" +
			     boost::lexical_cast<string>(handle) + "]");
		handle = INVALID_HANDLE_VALUE;
	}
}

FileReadHandle::FileReadHandle(const string &_path)
	: FileHandle(_path)
{
#ifndef HAVE_WINSOCK2_H
	handle = open64(path.c_str(), O_RDONLY|OPENFLAG, OPENMODE);
#else
	handle = CreateFile(path.c_str(), GENERIC_READ, 0, NULL, OPENFLAG, 0, NULL);
#endif
	if(handle == INVALID_HANDLE_VALUE){
		LOG4CXX_ERROR(logger, "Failed to open Physical Volume : " + path +
			      " (" + boost::lexical_cast<string>(errno) + ") for read ");
	}
	LOG4CXX_INFO(logger, "Open Physical Volume : " + path + " for read[" +
		     boost::lexical_cast<string>(handle) + "]");
}

FileWriteHandle::FileWriteHandle(const string &_path)
	: FileHandle(_path)
{
#ifndef HAVE_WINSOCK2_H
	handle = open64(path.c_str(), O_WRONLY|OPENFLAG, OPENMODE);
#else
	handle = CreateFile(path.c_str(), GENERIC_WRITE, 0, NULL, OPENFLAG, 0, NULL);
#endif
	if(handle == INVALID_HANDLE_VALUE){
		LOG4CXX_ERROR(logger, "Failed to open Physical Volume : " + path +
			      " (" + boost::lexical_cast<string>(errno) + ") for write ");
	}
	LOG4CXX_INFO(logger, "Open Physical Volume : " + path + " for write[" +
		     boost::lexical_cast<string>(handle) + "]");
}

LowHandle::LowHandle(FileHandlePtr _descriptor,
		     const off64_t off, const uint64_t len)
	: descriptor(_descriptor), offset(off), length(len)
{
	LOG4CXX_DEBUG(logger ,"LowHandle : [" +
		      boost::lexical_cast<string>(descriptor->get())+
		      + "] offset " +
		      boost::lexical_cast<string>(offset)+
		      " length " + 
		      boost::lexical_cast<string>(length));
}
