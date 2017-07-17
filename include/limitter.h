/**
   @file limitter.h
   @brief Limitter classes
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: session.h 225 2007-07-20 03:07:33Z sugiura $
 */

#ifndef __ISCSI_LIMITTER__
#define __ISCSI_LIMITTER__

#include "session.h"

/** Limitter interface class */
class Limitter {
public:
	/**
	   Constructor
	   @param _x1 : soft limit point
	   @param _x2 : hard limit point
	*/
	Limitter(const size_t _x1, const size_t _x2)
		: x1(_x1), x2(_x2)
	{}
	/** Destructor */
	virtual ~Limitter() {}
	/** Soft limit point getter */
	size_t getX1() const { return x1; }
	/** Hard limit point getter */
	size_t getX2() const { return x2; }
	/**
	   limitted value getter interface
	   @param x : current point
	*/
	virtual size_t getY(const size_t x) const = 0;
protected:
	/** soft limit point */
	size_t x1;
	/** hard limit point */
	size_t x2;
};

typedef boost::shared_ptr<Limitter> LimitterPtr;

/**
   Linear Limitter
   f(x) = MAX     : where x <  x1
   f(x) = ax + b
   f(x) = 1       : where x >= x2
*/
class LinearLimitter : public Limitter{
public:
	/**
	   Constructor
	   @param _x1 : soft limit point
	   @param _x2 : hard limit point
	*/
	LinearLimitter(const size_t _x1, const size_t _x2)
		: Limitter(_x1, _x2), ainv((x2 - x1)/(iSCSISession::CMD_QUEUE_SIZE - 1))
	{}
	/**
	   limitted value getter
	   @param x : current point
	*/
	size_t getY(const size_t x) const
	{
		if(x < x1)
			return iSCSISession::CMD_QUEUE_SIZE;
		else if(x >= x2)
			return 1;
		return iSCSISession::CMD_QUEUE_SIZE - (x - x1)/ainv;
	}
private:
	/** the inverse of "a" in "f(x) = ax + b" */
	size_t ainv;
};

#endif // __ISCSI_LIMITTER__
