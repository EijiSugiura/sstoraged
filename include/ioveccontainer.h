/** 
 * @file  ioveccontainer.h
 * @brief IOvec Container class
 * @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
 * @version $Id: ioveccontainer.h 312 2007-09-28 00:56:17Z sugiura $
 */
#ifndef __IOVECCONTAINER_H__
#define __IOVECCONTAINER_H__

#include "common.h"

#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
typedef iovec IOVEC;
#endif

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
typedef WSABUF IOVEC;
#endif

#include <list>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/pool/singleton_pool.hpp>
#include "logger.h"
#include "task.h"
#include "counter.h"

using namespace std;

typedef enum {
	POOL_STATE_NORMAL = 0,
	POOL_STATE_SOFT,
	POOL_STATE_HARD,
	POOL_STATE_MAX,
} pool_state_t;

/** Common Buffer fixed size -- MaxRecvDataSegmentLength
    @todo add 48bytes for header space?
 */
#define COMMON_BUF_SIZE	(32*4096)
#define COMMON_REAL_SIZE	(COMMON_BUF_SIZE+(512))

class Limitter;

/** Common Buffer class*/
class CommonBuf {
	struct CommonTag {};
	typedef boost::singleton_pool<CommonTag, COMMON_REAL_SIZE> CommonPool;
public:
	/** Default constructor */
	CommonBuf() : buf(CommonPool::malloc()), size(COMMON_BUF_SIZE),
		offset(0), tail(0)
	{
		if(!buf)
			throw std::out_of_range("Failed to allocate common buf");
#ifdef CBUF_DEBUG
		LOG4CXX_DEBUG(logger, "CBuf Constructed : " +
			      boost::lexical_cast<string>(this) +
			      " " + boost::lexical_cast<string>(buf));
		memset(buf, 0, size);
#endif
	}
	/** Destructor */
	~CommonBuf()
	{
#ifdef CBUF_DEBUG
		LOG4CXX_DEBUG(logger, "CBuf Destructed : " +
			      boost::lexical_cast<string>(this) +
			      " " + boost::lexical_cast<string>(buf));
#endif
		CommonPool::free(buf);
		buf = NULL;
	}
	/** Pooled allocator */
 	static void* operator new(size_t size);
 	/** Pooled deallocator */
 	static void operator delete(void* obj, size_t size);
	/** Pool size getter */
	static size_t getPoolSize();
	static bool releasePool();

	/** head pointer getter */
	void *head() { return buf; }
	/** edge pointer getter */
	void *edge() { return (reinterpret_cast<unsigned char*>(buf) + size); }
	/** reference/(to read) pointer getter */
	void *ref() { return (reinterpret_cast<unsigned char*>(buf) + offset); }
	/** current/(to write) pointer getter */
	void *cur() { return (reinterpret_cast<unsigned char*>(buf) + tail); }
	/** size getter */
	size_t getSize() const { return size; }
	/** remain to read */
	size_t getRemain() const { return (tail - offset); }
	/** free space to write */
	size_t getCapacity() const { return (size - tail); }
	/** offset from front of buf to ptr
	    @param ptr : pointer to end point
	 */
	size_t getOffset(uint8_t *ptr) const
	{ return (ptr - reinterpret_cast<uint8_t*>(buf)); }
	/** on edge checker
	    @return edge or not
	 */
	bool isEdge(uint8_t *ptr, const size_t challenge) const
	{ return ((size - getOffset(ptr)) < challenge);}
	bool inRange(uint8_t *ptr) const
	{ return (buf <= ptr && ptr-size <= buf); }
	/** begin checker */
	bool begin() const { return (offset == 0); }
	/** end checker */
 	bool end() const { return (offset == size); }
	/** step forward tail offset */
	bool stepForwardTail(const size_t step)
	{
		if(size < tail + step)
			return false;
		tail += step;
		return true;
	}
	/** shrink space */
	bool stepBackwardTail(const size_t step)
	{
		if(step > getRemain())
			return false;
		tail -= step;
		return true;
	}
	/** step forward start offset */
	bool stepForwardOffset(const size_t step)
	{
		if(getRemain() < step)
			return false;
		offset += step;
		return true;
	}

	/** Limitter getter */
	static boost::shared_ptr<Limitter> getLimitter();
	/** PoolState getter */
	static pool_state_t getPoolState();
	/** PoolState setter */
	static void setPoolState(const pool_state_t _state);
private:
	/** the common buffer */
	void *buf;
	/** size of common buffer */
	size_t size;
	/** start offset to read/write */
	size_t offset;
	/** tail offset to read/write */
	size_t tail;

	/** counter for # of buffers */
	static SingletonCounter<USHRT_MAX> counter;
	/** Pool limitter */
	static boost::shared_ptr<Limitter> limitter;
	/** PoolState string getter */
	static string & getPoolStateStr(const pool_state_t _state);
	/** Pool state : "Normal"|"Soft"|"Hard" */
	static pool_state_t poolstate;
	/** Pool state guard */
	static boost::mutex state_guard;
};

typedef boost::shared_ptr<CommonBuf> CommonBufPtr;
struct CommonBufTag {};
typedef boost::singleton_pool<CommonBufTag, sizeof(CommonBuf)> CommonBufPool;

class SendTask;
class SendCacheTask;
/** iovec container class */
class IovecContainer {
public:
	/** Default constructor */
	IovecContainer() : vec(NULL), count(0), remain(0), overflow(0) {}
	/** Constructor to send data */
	IovecContainer(SendTask *task);
	/** Constructor to send cache */
	IovecContainer(SendCacheTask *task);
	/** Destructor */
	~IovecContainer();

	/** reserve readable space */
	bool reserve(const size_t total);

	/** step forward start offset */
	CommonBufPtr stepForwardOffset(size_t &size);
	/** step forward tail */
	bool stepForwardTail(size_t size);
	/** update vec[] */
	bool update();
	/** get iovec reference */
	IOVEC *refVec() const { return vec; }
	/** iovec counter */
	size_t getCount() const { return count; }
	/** get remain size */
	size_t getRemain() const { return remain; }
	/** get front reference of common buf array */
	CommonBufPtr front() const { return cbufs.front(); }
	/** get second reference of common buf array */
	CommonBufPtr second() const { return *(++cbufs.begin()); }
	/** @todo: define new/delete operaters */
private:
	/** rotate iovec array */
	bool rotate(const size_t index);
	/** the iovec */
	IOVEC *vec;
	/** size of iovec array */
	size_t count;
	/** cbuf cache */
	list<CommonBufPtr> cbufs;
	/** total remain size */
	size_t remain;
	/** cbufs.size() - count */
	size_t overflow;
};

#endif /* __IOVECCONTAINER_H__ */
