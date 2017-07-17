/**
   @file reactor.cpp
   @brief iSCSI Reactor class
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: reactor.cpp 312 2007-09-28 00:56:17Z sugiura $
 */

#include <sstream>
#include <iomanip>
#include "logger.h"
#include "manager.h"
#include "session.h"
#include "reactor.h"
#include "volume.h"
#include "writecache.h"
#include "diskwriter.h"

iSCSIReactor::iSCSIReactor(const iSCSISessionPtr aSession) : session(aSession)
{
	LOG4CXX_DEBUG(logger, "Reactor Constructed : " +
		      boost::lexical_cast<string>(this) + " " +
		      boost::lexical_cast<string>(session.get()));
}

iSCSIReactor::~iSCSIReactor()
{
	LOG4CXX_DEBUG(logger, "Reactor Destructed : " +
		      boost::lexical_cast<string>(this) + " " +
		      boost::lexical_cast<string>(session.get()));
}

static string stringify(const SessionID val)
{
	ostringstream os;
	os << hex << setfill('0');
	for(size_t counter = 0; counter < val.SIZE; ++counter){
		os << setw(2) << (unsigned int)val.sid[counter];
	}
	return os.str();
}

void iSCSIReactor::operator()() throw(runtime_error)
{
	NDC::push("reactor[" + stringify(session->sid.get()) + "]");

	iSCSIManager &manager = iSCSIManager::getInstance();
	iSCSIReactorPtr reactor = manager.findReactor(session->sid.get());
	if(reactor.get() == NULL){
		LOG4CXX_ERROR(logger, "Reactor loop : Internal error");
		goto end;
	}

	/** setup diskwriters */
	if(!setupWriters())
		goto end;
	pthread_detach(pthread_self());

 	(*loop)();

	/** clear diskwriters */
	clearWriters();

 end:
	session->connection.clearListener();
	if(!manager.delReactor(reactor))
		LOG4CXX_ERROR(logger, "Failed to remove reactor");
	NDC::remove();
}

status_t iSCSIReactor::recv(const socket_t fd)
{
	return session->recv(session, fd);
}

void iSCSIReactor::mayStop()
{
	LOG4CXX_DEBUG(logger, "Reactor may stop : " +
		      boost::lexical_cast<string>(this) + " " +
		      boost::lexical_cast<string>(session.get()));	
	loop->mayStop();
}

bool iSCSIReactor::setupWriters()
{
	iSCSIInitiatorPtr initiator = session->initiator.get();
	if(initiator.get() == NULL){
		LOG4CXX_ERROR(logger, "Initiator is not initialized!");
		return false;
	}

	/** initialize write caches */
	size_t size = initiator->getLUNnames().size();
	LOG4CXX_DEBUG(logger, "Setup "
		      + boost::lexical_cast<string>(size) + " write caches");
	while(size--)
		session->wcaches.push_back(WriteCachePtr(new WriteCache));

	for(uint32_t lun_id = 0; lun_id < session->wcaches.size(); ++lun_id){
		DiskWriterPtr writer =
			DiskWriterPtr(new AioDiskWriter(session->wcaches[lun_id]));
		LogicalVolumePtr lv = initiator->getLUN(lun_id);
		vector<LowHandle> handles =
			lv->getLowHandles(0, SCSI::LBA2OFFSET(lv->getTotalSize()), false);
		/** @todo : support distributed segment */
		uint64_t offset = 0;
		for(vector<LowHandle>::iterator itr = handles.begin();
		    itr != handles.end(); ++itr) {
			writer->addHandle(*itr, offset);
			offset += (*itr).getLength();
		}
		if(offset != SCSI::LBA2OFFSET(lv->getTotalSize())){
			LOG4CXX_ERROR(logger, "Internal error with volume coverage");
			return false;
		}
		/** create writer thread */
		wthreads.create_thread(boost::ref(*writer));
		session->wcaches[lun_id]->setWriter(writer);
	}
	return true;
}

void iSCSIReactor::clearWriters()
{
	for(uint32_t lun_id = 0; lun_id < session->wcaches.size(); ++lun_id){
		session->wcaches[lun_id]->getWriter()->mayStop();
	}
	wthreads.join_all();
}
