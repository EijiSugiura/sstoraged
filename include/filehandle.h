/**
   @file filehandle.h
   @brief File handling classes
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: filehandle.h 312 2007-09-28 00:56:17Z sugiura $
 */
#ifndef __FILEHANDLE_H__
#define __FILEHANDLE_H__

#include <unistd.h>
#include <fcntl.h>
#include <boost/shared_ptr.hpp>
#include "common.h"

#ifdef HAVE_WINSOCK2_H
#include <winbase.h>
#endif

class FileHandle {
public:
	FileHandle(const string &_path)
		: handle(INVALID_HANDLE_VALUE), path(_path) {}
	virtual ~FileHandle();
	filehandle_t get() const { return handle; }
	string & getPath() const { return path; }
#ifndef HAVE_WINSOCK2_H
	static const int OPENFLAG = O_LARGEFILE|O_CREAT;
	static const mode_t OPENMODE = S_IRUSR|S_IWUSR;
#else
	static const int OPENFLAG = OPEN_ALWAYS;
#endif
protected:
	filehandle_t handle;
	mutable string path;
};

typedef boost::shared_ptr<FileHandle> FileHandlePtr;

class FileReadHandle : public FileHandle {
public:
	FileReadHandle(const string &_path);
};

typedef boost::shared_ptr<FileReadHandle> FileReadHandlePtr;

class FileWriteHandle : public FileHandle {
public:
	FileWriteHandle(const string &_path);
};

typedef boost::shared_ptr<FileWriteHandle> FileWriteHandlePtr;

/** Low disk image handle class */
class LowHandle {
public:
	LowHandle() : offset(0), length(0){}
	/** Constructor
	    @param _descriptor : file descriptor
	    @param off : start offset
	    @param len : length
	*/
	LowHandle(FileHandlePtr _descriptor,
		  const off64_t off, const uint64_t len);
	/** destructor */
	~LowHandle() {}
	/** descriptor getter */
	filehandle_t getDescriptor() const { return descriptor->get(); }
	/** byte unit start offset getter */
	off64_t getOffset() const { return offset; }
	/** byte unit length getter */
	uint64_t getLength() const { return length; }
	/** path getter */
	string & getPath() const { return descriptor->getPath(); }
private:
	FileHandlePtr descriptor;
	off64_t offset;
	uint64_t length;
};

#endif /* __FILEHANDLE_H__ */
