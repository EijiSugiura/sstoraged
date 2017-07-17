/**
   @file counter.h
   @brief Counter utility classes
   @author Eiji Sugiura <eiji.sugiura@gmail.com> Research Institute of Systems Planning, Inc
   @version $Id: counter.h 289 2007-08-21 16:36:29Z sugiura $
 */
#ifndef __COUNTER_H__
#define __COUNTER_H__

#include <stdexcept>
#include <climits>

using namespace std;

template <typename T, T MIN, T MAX>
class IncrementalCounter {
public:
	IncrementalCounter() : counter(MIN) {}
	T operator()()
	{
		T tmp = counter++;
		if(tmp >= MAX)
			counter = MIN;
		return tmp;
	}
protected:
	T counter;
};

class SequenceCounter : public IncrementalCounter<uint32_t, 0, ULONG_MAX> {
public:
	SequenceCounter() {}
	void set(const uint32_t isn) { counter = isn; }
	uint32_t cur() const { return counter; }
};

template <typename T, T MIN, T MAX>
class NumberCounter {
public:
	NumberCounter() : counter(MIN) {}
	virtual ~NumberCounter() {}
	T cur() const { return counter; }
	virtual T operator++()
	{
		T tmp = counter++;
		if(tmp >= MAX){
			counter = MAX;
			throw std::runtime_error("Too large to increment");
		}
		return counter;
	}
	virtual T operator+=(const T diff)
	{
		T tmp = counter;
		counter += diff;
		if(tmp >= MAX){
			counter = MAX;
			throw std::runtime_error("Too large to increment");
		}
		return counter;
	}
	virtual T operator--()
	{
		T tmp = counter--;
		if(tmp <= MIN){
			counter = MIN;
			throw std::runtime_error("Too small to decrement");
		}
		return counter;
	}
protected:
	T counter;
};

#include <boost/thread.hpp>

template <size_t MAX>
class SingletonCounter : public NumberCounter<size_t, 0, MAX> {
	typedef boost::mutex::scoped_lock lock;
public:
	size_t cur()
	{
		lock lk(counter_guard);
		return NumberCounter<size_t, 0, MAX>::cur();
	}
	size_t operator++()
	{
		lock lk(counter_guard);
		return NumberCounter<size_t, 0, MAX>::operator++();
	}
	size_t operator+=(const size_t diff)
	{
		lock lk(counter_guard);
		return NumberCounter<size_t, 0, MAX>::operator+=(diff);
	}
	size_t operator--()
	{
		lock lk(counter_guard);
		return NumberCounter<size_t, 0, MAX>::operator--();
	}
private:
	boost::mutex counter_guard;
};


#endif /* __COUNTER_H__ */
