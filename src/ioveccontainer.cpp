/** 
 * @file  ioveccontainer.cpp
 * @brief IOvec Container class
 * @author Eiji Sugiura <eiji.sugiura@gmail.com>
 * @version $Id: ioveccontainer.cpp 284 2007-08-15 08:36:45Z sugiura $
 */

#include <boost/lexical_cast.hpp>
#include "logger.h"
#include "ioveccontainer.h"
#include "configurator.h"
#include "limitter.h"

#ifdef LOG4CXX_COUT
#undef LOG4CXX_DEBUG
#define LOG4CXX_DEBUG(logger,message)	strings.push_back(message)
#undef LOG4CXX_WARN
#define LOG4CXX_WARN(logger,message)	strings.push_back(message)
#undef LOG4CXX_ERROR
#define LOG4CXX_ERROR(logger,message)	strings.push_back(message)
#undef LOG4CXX_INFO
#define LOG4CXX_INFO(logger,message)	strings.push_back(message)
extern vector<string> strings;
#endif

SingletonCounter<USHRT_MAX> CommonBuf::counter;
size_t CommonBuf::getPoolSize()
{
	return counter.cur();
}

bool CommonBuf::releasePool()
{
	return (CommonPool::release_memory() &&
		CommonBufPool::release_memory());
}

LimitterPtr CommonBuf::limitter;
#ifndef LOG4CXX_COUT
LimitterPtr CommonBuf::getLimitter()
{
	if(limitter.get() == NULL){
		Configurator &cur_conf = Configurator::getInstance();
		uint64_t poolsoftlimit;
		cur_conf.getAttr("PoolSoftLimit", poolsoftlimit);
		poolsoftlimit /= COMMON_REAL_SIZE + sizeof(CommonBuf);
		uint64_t poolhardlimit;
		cur_conf.getAttr("PoolHardLimit", poolhardlimit);
		poolhardlimit /= COMMON_REAL_SIZE + sizeof(CommonBuf);
		limitter = boost::shared_ptr<Limitter>(new LinearLimitter(poolsoftlimit, poolhardlimit));
	}
	return limitter;
}
#endif

pool_state_t CommonBuf::poolstate = POOL_STATE_NORMAL;
boost::mutex CommonBuf::state_guard;
pool_state_t CommonBuf::getPoolState()
{
	boost::mutex::scoped_lock lk(state_guard);
	return poolstate;
}
string & CommonBuf::getPoolStateStr(const pool_state_t _state)
{
	static string poolstr[POOL_STATE_MAX] = { "Normal", "Soft", "Hard" };
	return poolstr[_state];
}
void CommonBuf::setPoolState(const pool_state_t _state)
{
	boost::mutex::scoped_lock lk(state_guard);
	if(poolstate == _state)
		return;
	LOG4CXX_INFO(logger, "PoolState : " + getPoolStateStr(poolstate) +
		     "->" + getPoolStateStr(_state));
	poolstate = _state;
}

void* CommonBuf::operator new(size_t size)
{
	if(size != sizeof(CommonBuf)){
		LOG4CXX_ERROR(logger, "CommonBuf allocated with invalid size");
		return ::operator new(size);
	}
	void *ret = CommonBufPool::malloc();
	if(ret)
		++counter;
	return ret;
}

void CommonBuf::operator delete(void *obj, size_t size)
{
	if(!obj)
		return;
	if(size != sizeof(CommonBuf) ||
	   !CommonBufPool::is_from(obj)){
		LOG4CXX_ERROR(logger, "Unknown CommonBuf destructed");
		return ::operator delete(obj);
	}
	CommonBufPool::free(obj);
	--counter;
}

IovecContainer::IovecContainer(SendTask *task)
	: remain(0), overflow(0)
{
	/** convert send task -> iovec */
	count = task->size();
	vec = new iovec[count];
	list<CommonBufPtr>::iterator itr = task->begin();
	for(size_t i = 0; i < count; ++i,++itr){
		cbufs.push_back(*itr);
		vec[i].iov_base = cbufs.back()->ref();
		vec[i].iov_len = cbufs.back()->getRemain();
		remain += vec[i].iov_len;
#ifdef IOVEC_DEBUG
		LOG4CXX_DEBUG(logger, "vec[" + boost::lexical_cast<string>(i) + "] " +
			      boost::lexical_cast<string>(cbufs.back()->head()) + "/" +
			      boost::lexical_cast<string>(vec[i].iov_base) +  "+" +
			      boost::lexical_cast<string>(vec[i].iov_len));
#endif
	}
}

IovecContainer::IovecContainer(SendCacheTask *task)
	: remain(0), overflow(0)
{
	/** convert send task -> iovec */
	count = task->size();
	vec = new iovec[count];
	size_t offset;
	vector<CommonBufPtr>::iterator itr = task->begin(offset);
	for(size_t i = 0; i < count; ++i,++itr){
		cbufs.push_back(*itr);
		vec[i].iov_base = reinterpret_cast<uint8_t*>(cbufs.back()->head())
			+ offset;
		vec[i].iov_len = min(task->getRemain() - remain,
				     cbufs.back()->getSize()-offset);
		remain += vec[i].iov_len;
		offset = 0;
#ifdef IOVEC_DEBUG
		LOG4CXX_DEBUG(logger, "vec[" + boost::lexical_cast<string>(i) + "] " +
			      boost::lexical_cast<string>(cbufs.back()->head()) + "/" +
			      boost::lexical_cast<string>(vec[i].iov_base) +  "+" +
			      boost::lexical_cast<string>(vec[i].iov_len));
#endif
	}
}

IovecContainer::~IovecContainer()
{
	delete [] vec;
	vec = NULL;
}

bool IovecContainer::stepForwardTail(size_t size)
{
	for(list<CommonBufPtr>::iterator itr = cbufs.begin();
	    size > 0 && itr != cbufs.end(); ++itr) {
		size_t capacity = (*itr)->getCapacity();
		if(capacity < size) {
			(*itr)->stepForwardTail(capacity);
			size -= capacity;
			continue;
		}
		(*itr)->stepForwardTail(size);
		size = 0;
	}
	return (size == 0);
}

static bool NotEndPredicate(CommonBufPtr cbuf)
{
	return !cbuf->end();
}

CommonBufPtr IovecContainer::stepForwardOffset(size_t &size)
{
	list<CommonBufPtr>::iterator itr = find_if(cbufs.begin(),cbufs.end(),
						   NotEndPredicate);
	if(itr == cbufs.end()){
		CommonBufPtr null;
		return null;
	}
	size_t capacity = (*itr)->getRemain();
	if(capacity < size) {
		(*itr)->stepForwardOffset(capacity);
		size -= capacity;
		remain -= capacity;
	} else {
		(*itr)->stepForwardOffset(size);
		size = 0;
		remain -= size;
	}
	return (*itr);
}

#ifdef IOVEC_DEBUG
static void dumpCbufs(const list<CommonBufPtr> &cbufs)
{
	size_t counter = 0;
	for(list<CommonBufPtr>::const_iterator itr = cbufs.begin();
	    itr != cbufs.end(); ++itr,++counter){
		LOG4CXX_DEBUG(logger, "cbuf[" +
			      boost::lexical_cast<string>(counter) + "] " +
			      boost::lexical_cast<string>((*itr)->head()) + " read " +
			      boost::lexical_cast<string>((*itr)->getRemain()) + " capacity " +
			      boost::lexical_cast<string>((*itr)->getCapacity()));
	}
}

static void dumpIovecs(const iovec *vec, size_t size)
{
	for(size_t index = 0; index < size; ++index){
		LOG4CXX_DEBUG(logger, "vec[" +
			      boost::lexical_cast<string>(index) + "] " +
			      boost::lexical_cast<string>(vec[index].iov_base) +  "+" +
			      boost::lexical_cast<string>(vec[index].iov_len));
	}
}
#endif

/** @todo : omit index, cause needless */
bool IovecContainer::rotate(const size_t index)
{
	memmove(&vec[0], &vec[1], sizeof(iovec)*(count-index-1));
	cbufs.push_back(CommonBufPtr(new CommonBuf));
	remain += cbufs.back()->getSize();
	vec[count-1].iov_base = cbufs.back()->cur();
	vec[count-1].iov_len = cbufs.back()->getCapacity();
	return true;
}

bool IovecContainer::update()
{
#ifdef IOVEC_DEBUG
	dumpCbufs(cbufs);
#endif
	list<CommonBufPtr>::iterator itr = cbufs.begin();
	for(size_t index = 0;
	    itr != cbufs.end() && (*itr)->end();
	    itr = cbufs.begin(), ++index){
		cbufs.pop_front();
		if(overflow > 0){
			LOG4CXX_DEBUG(logger, "Rotate Iovec cleanup");
			--overflow;
		} else {
			LOG4CXX_DEBUG(logger, "Rotate Iovec underflow");
			rotate(index);
		}
	}
	for(ssize_t skip = overflow; skip > 0; --skip)
		++itr;
	/** @todo : check if this if-clause need */
	if((*itr)->inRange(reinterpret_cast<uint8_t*>(vec[0].iov_base))){
		/** rotate vec */		
		for(size_t index = 0;
		    itr != cbufs.end() && (*itr)->getCapacity() == 0;
		    ++itr, ++index){
			LOG4CXX_DEBUG(logger, "Rotate Iovec overflow");
			rotate(index);
			++overflow;
		}
	}
	/** update 1st iovec */
	vec[0].iov_base = (*itr)->cur();
	vec[0].iov_len = (*itr)->getCapacity();
	LOG4CXX_DEBUG(logger, "Update Iovec : " + 
		      boost::lexical_cast<string>(overflow)+ "/" +
		      boost::lexical_cast<string>(cbufs.size()));
#ifdef IOVEC_DEBUG
	dumpIovecs(vec, count);
	dumpCbufs(cbufs);
#endif
	return true;
}

bool IovecContainer::reserve(const size_t total)
{
	if(count == 0){
		count = max((total/COMMON_BUF_SIZE + 1), 2LU);
		vec = new iovec[count];
		for(size_t i = 0; i < count; ++i){
			cbufs.push_back(CommonBufPtr(new CommonBuf));
			vec[i].iov_base = cbufs.back()->ref();
			vec[i].iov_len = cbufs.back()->getSize();
			remain += vec[i].iov_len;
			LOG4CXX_DEBUG(logger, "vec[" + boost::lexical_cast<string>(i) + "] " +
				      boost::lexical_cast<string>(cbufs.back()->head()) + "/" +
				      boost::lexical_cast<string>(vec[i].iov_base) +  "+" +
				      boost::lexical_cast<string>(vec[i].iov_len));
		}
	} else if(remain < total){
		LOG4CXX_WARN(logger,"Not enough buffers!!! : remain " +
			     boost::lexical_cast<string>(remain) + " < total " +
			     boost::lexical_cast<string>(total));
		return false;
	}
	return true;
}
