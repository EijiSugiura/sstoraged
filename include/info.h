/** 
 * @file  info.h
 * @brief Info classes
 * @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
 * @version $Id: info.h 150 2007-06-27 04:49:46Z sugiura $
 */
#ifndef __SSTORAGE_INFO_H__
#define __SSTORAGE_INFO_H__

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include "common.h"
#include "counter.h"

using namespace std;

/**
   Info interface class
 */
class Info {
public:
	/** Default constructor */
	Info() {}
	/** Destructor */
	virtual ~Info() {}
};

typedef boost::shared_ptr<Info> InfoPtr;

/** DataOut receive info. class */
class DataOutInfo : public Info {
public:
	/** Constructor
	    @param _lba : the LBA
	    @param _total : total length in byte unit
	    @param _remain : remain length in byte unit
	 */
	DataOutInfo(const uint64_t _lba, const uint32_t _total, const uint32_t _remain)
		: lba(_lba), total(_total), remain(_remain), r2tsn_gen() {}
	/** LBA getter */
	uint64_t getLBA() const { return lba; }
	/** Total length getter */
	uint32_t getTotal() const { return total; }
	/** Remain length getter */
	uint32_t getRemain() const { return remain; }
	/** Remain length setter */
	void setRemain(const uint32_t _remain) { remain = _remain; }
	/** R2T SN updater */
	uint32_t advanceR2TSN() { return r2tsn_gen(); }
private:
	/** Logical Block Address */
	mutable uint64_t lba;
	/** Total Length in byte unit */
	mutable uint32_t total;
	/** Remain Length in byte unit */
	uint32_t remain;
	/** R2T sequence number */
	SequenceCounter r2tsn_gen;
};

typedef boost::shared_ptr<DataOutInfo> DataOutInfoPtr;

#endif /* __SSTORAGE_INFO_H__ */
