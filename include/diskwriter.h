/** 
 * @file  diskwriter.h
 * @brief DiskWriter classes
 * @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
 * @version $Id: diskwriter.h 292 2007-08-27 05:36:39Z sugiura $
 */
#ifndef __SSTORAGE_DISKWRITER_H__
#define __SSTORAGE_DISKWRITER_H__

#include <queue>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include "common.h"
#include "filehandle.h"
#include "volume.h"
#include "counter.h"

using namespace std;

class WriteCache;

/** DiskWriter class */
class DiskWriter {
	typedef boost::mutex::scoped_lock lock;
public:
	/** Constructor
	    @param _reactor : owner of this
	    @param _wc : the Write Cache
	 */
	DiskWriter(boost::shared_ptr<WriteCache> _wc);
	/** Destructor */
	virtual ~DiskWriter();

	/** add Write filehandle
	    @param handle : FileWriteHandle + start + length
	    @param offset : offset[B] from front of segment's LBA 
	    @return success or failure
	 */
	bool addHandle(const LowHandle &handle, const uint64_t offset);
	/** main loop of disk writer */
	virtual void operator()() throw(std::runtime_error);
	/** event notifiler, used by iSCSI reactor thread */
	void notifyEvent() { event.notify_one(); }
	/** may disk writer thread stop */
	void mayStop();
	/** do the work */
	virtual bool doWork();

protected:
	/** need notify for finished event? */
	bool needNotify();
	/** thread may stop flag */
	bool stop;
	/** file desctriptor to write out */
	typedef	map<uint64_t,LowHandle> write_handle_map_t;
	write_handle_map_t handles;
	/** the write cache */
	boost::shared_ptr<WriteCache> wc;
	/** write event guard mutex */
	boost::mutex ev_guard;
	/** event condition wait */
	boost::condition_variable event;

	filehandle_t getHandle(uint64_t start, size_t length,
			       size_t &covered, uint64_t &real_offset);
	bool doWrite(const filehandle_t fd, void *buf, size_t count,
		     uint64_t real_offset);
};

typedef boost::shared_ptr<DiskWriter> DiskWriterPtr;

template <typename T>
class SingletonQueue {
	typedef boost::mutex::scoped_lock lock;
public:
	void push(const T data)
	{
		lock lk(container_guard);
		container.push(data);
	}
	bool empty()
	{
		lock lk(container_guard);
		return container.empty();
	}
	size_t size()
	{
		lock lk(container_guard);
		return container.size();
	}
	T & front()
	{
		lock lk(container_guard);
		return container.front();
	}
	void pop()
	{
		lock lk(container_guard);
		container.pop();
	} 
private:
	queue<T> container;
	boost::mutex container_guard;
};

struct io_context;
class WriteSegment;
class AioDiskWriter : public DiskWriter {
public:
	/** Constructor */
	AioDiskWriter(boost::shared_ptr<WriteCache> _wc);
	/** Destructor */
	~AioDiskWriter();
	/** main loop */
	void operator()() throw(std::runtime_error);
	/** do the work */
	bool doWork();
	/** Event Listener work */
	bool doListenerWork();
private:
	const static size_t IOCB_MAX = 64;
	class EventListener {
	public:
		EventListener(AioDiskWriter *_parent) : parent(_parent) {}
		void operator()() throw(std::runtime_error);
	private:
		mutable AioDiskWriter *parent;
	} listener;
	io_context *ctx;
	SingletonQueue<size_t> iocbs_sizes;
	SingletonCounter<IOCB_MAX*2> iocbs_total;
	boost::shared_ptr<WriteSegment> current_ws;
};

#endif /* __SSTORAGE_DISKWRITER_H__ */
