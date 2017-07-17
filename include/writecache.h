/** 
 * @file  writecache.h
 * @brief Write Cache class
 * @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
 * @version $Id: writecache.h 288 2007-08-21 03:44:04Z sugiura $
 */
#ifndef __WRITE_CACHE_H__
#define __WRITE_CACHE_H__

#include <vector>
#include <iostream>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/thread.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include "ioveccontainer.h"

using namespace std;
using namespace boost::multi_index;

struct WriteSegment {
	/** Normal constructor */
	WriteSegment(const uint64_t _lba, const size_t _length)
		: lba(_lba), transfer_length(_length),
		  rlba(_lba), rlength(_length), copy(false),
		  cbufs(), ds_offset(0), stored(0) {}
	/** Copy constructor */
	WriteSegment(boost::shared_ptr<WriteSegment> ptr)
		: lba(ptr->lba), transfer_length(ptr->transfer_length),
		  rlba(ptr->rlba), rlength(ptr->rlength), copy(true),
		  cbufs(ptr->cbufs), ds_offset(0), stored(ptr->stored) {}

	/** Pooled allocator */
 	static void* operator new(size_t size);
 	/** Pooled deallocator */
 	static void operator delete(void* obj, size_t size);

	/** Dump out WriteSegments */
	string dump();
	/** Set 1st WriteSegment
	    @param cbuf : 1st DataSegment
	    @param size : size of DataSegment
	    @param offset : offset from cbuf's buf to DataSegment
	    @return success or failure
	 */
	bool set1stDataSegment(const CommonBufPtr cbuf, const size_t size,
			       const size_t offset)
	{
		if(!cbufs.empty())
			return false;
		stored += size;
		if(stored > SCSI::LBA2OFFSET(transfer_length))
			return false;
		cbufs.push_back(cbuf);
		ds_offset = offset;
		return true;
	}
	/** Add next WriteSegment
	    @param cbuf : next DataSegment
	    @param size : size of DataSegment
	    @return success or failure
	 */
	bool addDataSegment(const CommonBufPtr cbuf, size_t size)
	{
		if(cbufs.empty())
			return false;
		stored += size;
		if(stored > SCSI::LBA2OFFSET(transfer_length))
			return false;
		cbufs.push_back(cbuf);
		return true;
	}
	/** The LBA for WRITE */
	uint64_t lba;
	/** Transfer Length for WRITE */
	size_t transfer_length;
	/** The LBA for READ */
	uint64_t rlba;
	/** Transfer Length for READ */
	size_t rlength;
	/** Copy flag */
	bool copy;
	/** Common Buffer cache array */
	vector<CommonBufPtr> cbufs;
	/** offset from 1st cbuf's buf to 1st DataSegment */
	size_t ds_offset;
	/** stored data segment length in bytes */
	size_t stored;
};

typedef boost::shared_ptr<WriteSegment> WriteSegmentPtr;
struct WriteSegmentTag {};
typedef boost::singleton_pool<WriteSegmentTag, sizeof(WriteSegment)> WriteSegmentPool;

/** LBA tag struct */
struct by_lba {};
/** FIFO tag struct */
struct by_fifo {};
/** WriteSegment's pointer tag struct */
struct by_ptr {};

class DiskWriter;

/** Write Cache class */
class WriteCache {
public:
	/** Default constructor */
	WriteCache();
	/** Destructor */
	~WriteCache();
	/** dump out WriteCache info. */
	string dump();
	/**
	   search with LBA & LBA+TransferLength range,
	   used by iSCSIReactor thread for READ
	   @param lba : the LBA to search
	   @param translfer_length : search range in LBA unit
	   @return Cache HIT : WriteSegment array with size > 0
	   @return Cache MISS : WriteSegment array with size == 0
	 */
	vector<WriteSegmentPtr> search(const uint64_t lba,
				       const size_t transfer_length)
	{
		lock lk(wq_guard);
		return _search(lba,transfer_length);
	}

	/**
	   put new Write Segment to WRITE
	   used by iSCSIReactor thread
	   @param ws : Write Segment to write
	 */
	void push(WriteSegmentPtr ws);
	/**
	   get the front of Write Segment queue
	   used by DiskWriter
	   @return uncopied WriteSegment in front of Write Segment queue
	 */
	WriteSegmentPtr front()
	{
		lock lk(wq_guard);
		return _front();
	}
	/**
	   get next element of Write Segment queue
	   used by AioDiskWriter
	   @param _ws : set current ws, then return next uncopied
	                WriteSegment in Write Segment queue
	   @return success or failure
	 */
	bool getNext(WriteSegmentPtr &_ws);
	/**
	   remove the front of Write Segment queue
	   call front() before use this.
	 */
	void pop()
	{
		lock lk(wq_guard);
		if(wq.empty())
			return;
		/** always remove front of FIFO queue */
		wq.pop_front();
	}
	/** empty checker */
	bool empty()
	{
		lock lk(wq_guard);
		return wq.empty();
	}
	/** DiskWriter getter */
	boost::shared_ptr<DiskWriter> getWriter() const
	{  return writer; }
	/** DiskWriter setter
	    @param _writer : The DiskWriter
	 */
	void setWriter(boost::shared_ptr<DiskWriter> _writer);
	size_t size()
	{
		lock lk(wq_guard);
		return wq.size();
	}

	void waitTillDone()
	{
		lock lk(ev_guard);
		boost::xtime xt;
		boost::xtime_get(&xt, boost::TIME_UTC);
		xt.sec += 1;
		ev_finished.timed_wait(lk, xt);
	}
	void notifyDone()
	{
		lock lk(ev_guard);
		ev_finished.notify_one();
	}
private:
	/**
	   DO search with LBA & LBA+TransferLength range,
	   used by iSCSIReactor thread for READ.
	   MUST BE LOCKED
	   @param lba : the LBA to search
	   @param translfer_length : search range in LBA unit
	   @return Cache HIT : WriteSegment array with size > 0
	   @return Cache MISS : WriteSegment array with size == 0
	 */
	vector<WriteSegmentPtr> _search(const uint64_t lba,
					const size_t transfer_length);
	/**
	   DO get the front of Write Segment queue
	   MUST BE LOCKED
	   @return get front uncopied=original element of FIFO queue
	 */
	WriteSegmentPtr _front()
	{
		WriteSegmentPtr ws;
		if(wq.empty())
			return ws;
		/** skip copy segments */
		for(ws = wq.front();
		    !wq.empty() && (ws->copy == true);
		    ws = wq.front())
			wq.pop_front();
		/** get front of FIFO queue,
		    last WriteSegment is NOT copied one, always.
		    @see WriteCache::push(WriteSegmentPtr ws) */
		return ws;
	}
	/** WroteCacje lock definition */
	typedef boost::mutex::scoped_lock lock;
	/** WriteCache mutex */
	boost::mutex wq_guard;
	struct ws_ptr {
		typedef void* result_type;
		result_type operator()(const WriteSegmentPtr &ws) const
		{ return ws.get(); }
	};
	/** Write Segment cache type */
	typedef boost::multi_index_container<
		WriteSegmentPtr,
		indexed_by<sequenced<tag<by_fifo> >,
		ordered_non_unique<tag<by_lba>, member<WriteSegment,uint64_t,&WriteSegment::rlba> >,
		ordered_unique<tag<by_ptr>, ws_ptr>
	        >
	> writecache_set_t;
	/** The Write Segment cache */
	writecache_set_t wq;
	typedef writecache_set_t::index<by_fifo>::type writecache_fifo_index;
	typedef writecache_set_t::index<by_lba>::type writecache_lba_index;
	typedef writecache_set_t::index<by_ptr>::type writecache_ptr_index;

 	/** The disk writer */
 	boost::shared_ptr<DiskWriter> writer;

	/** Event mutex */
	boost::mutex ev_guard;
	/** Event finished condition */
	boost::condition_variable ev_finished;
};

typedef boost::shared_ptr<WriteCache> WriteCachePtr;

#endif /* __WRITE_CACHE_H__ */
