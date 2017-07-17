/**
   @file sendfile.cpp
   @brief Sendfile Task classes
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: sendfile.cpp 212 2007-07-18 06:54:55Z sugiura $
 */

#include <sys/sendfile.h>
#include "logger.h"
#include "ioveccontainer.h"
#include "session.h"
#include "connection.h"
#include "volume.h"
#include "task.h"

#ifdef LOG4CXX_COUT
#undef LOG4CXX_DEBUG
#define LOG4CXX_DEBUG(logger,message)	strings.push_back(message)
#undef LOG4CXX_ERROR
#define LOG4CXX_ERROR(logger,message)	strings.push_back(message)
extern vector<string> strings;
#endif

SendfileTask::SendfileTask(boost::shared_ptr<iSCSISession> parent,
			   const socket_t afd,
			   boost::shared_ptr<LogicalVolume> alv,
			   const uint64_t lba, const size_t length)
	: session(parent), fd(afd), lv(alv),
	  offset(SCSI::LBA2OFFSET(lba)), remain(SCSI::LBA2OFFSET(length))
{
	LOG4CXX_DEBUG(logger, "Sendfile Constructed : lba "
		      + boost::lexical_cast<string>(lba) + " ("
		      + boost::lexical_cast<string>(offset) +
		      + ") remain " + boost::lexical_cast<string>(remain));
}

bool SendfileTask::run()
{
	iSCSIConnectionPtr conn = session->connection.find(fd);
	while(remain > 0){
		vector<LowHandle> lhs= lv->getLowHandles(offset, remain, true);
		if(lhs.empty())
			LOG4CXX_ERROR(logger, "Failed to sendfile : LowHandle is empty");
		for(vector<LowHandle>::iterator itr = lhs.begin();
		    itr != lhs.end(); ++itr){
			off64_t real_offset = itr->getOffset();
			size_t real_length = itr->getLength();
			ssize_t size = sendfile64(fd, itr->getDescriptor(),
						  &real_offset, real_length);
			if(size < 0)
				goto try_again;
			LOG4CXX_DEBUG(logger, "Sendfile : "
				      + boost::lexical_cast<string>(size)
				      + " offset " + boost::lexical_cast<string>(real_offset) +
				      + " length " + boost::lexical_cast<string>(real_length));
			offset += size;
			remain -= size;
			if(real_length != static_cast<size_t>(size))
				goto try_again;
		}
		conn->unwaitWritable();
		return true;
	}
 try_again:
	conn->waitWritable();
	return false;
}
