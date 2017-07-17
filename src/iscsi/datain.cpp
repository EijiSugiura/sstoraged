/**
   @file datain.cpp
   @brief iSCSI DataIn handler
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: datain.cpp 229 2007-07-23 04:44:49Z sugiura $
 */

#include <boost/lexical_cast.hpp>
#include "logger.h"
#include "iscsi/datain.h"

DataInGenerator::DataInGenerator()
	: cbuf(CommonBufPtr(new CommonBuf())), datasn_gen()
{
	bhs = reinterpret_cast<DataInHeader*>(cbuf->cur());
	DataInHeader tmp;
	*bhs = tmp;
	cbuf->stepForwardTail(sizeof(*bhs));
	datasn_gen();
#ifdef BHS_DEBUG
	LOG4CXX_DEBUG(logger, "Reply BHS at " +
		      boost::lexical_cast<string>(bhs));
#endif
}

DataInGenerator::DataInGenerator(DataInGenerator &old)
	: cbuf(CommonBufPtr(new CommonBuf())), datasn_gen()
{
	/** replicate common buffer */
	CommonBufPtr oldbuf = old.serialize();
	cbuf->stepForwardTail(oldbuf->getRemain());
	memcpy(cbuf->ref(), oldbuf->ref(), oldbuf->getRemain());
#ifdef BHS_DEBUG
	LOG4CXX_DEBUG(logger, "Reply BHS at "+ 
		      boost::lexical_cast<string>(cbuf->ref()) + "/" +
		      boost::lexical_cast<string>(oldbuf->ref()) + " " +
		      boost::lexical_cast<string>(oldbuf->getRemain()));
#endif
	bhs = reinterpret_cast<DataInHeader*>(cbuf->ref());
	/** copy DataSN */
	datasn_gen.set(old.getDataSN());
}

uint8_t* DataInGenerator::reserveDataSegment(const size_t size)
{
	if(size == 0){
		LOG4CXX_ERROR(logger, "Empty data segment specified");
		return NULL;
	}
	if(size > cbuf->getCapacity()){
		LOG4CXX_WARN(logger, "Too large data segment : " + boost::lexical_cast<string>(size) +
			     " > " + boost::lexical_cast<string>(cbuf->getCapacity()));
		return NULL;
	}
	/** reserve data segment */
	this->setDlength(size);
	cbuf->stepForwardTail(size);
	return reinterpret_cast<uint8_t*>(bhs + 1);
}

bool DataInGenerator::resetDlength(const size_t size)
{
	if(size == 0){
		LOG4CXX_ERROR(logger, "Empty data segment specified");
		return false;
	}
	if(size > cbuf->getCapacity()){
		LOG4CXX_WARN(logger, "Too large data segment : " + boost::lexical_cast<string>(size) +
			     " > " + boost::lexical_cast<string>(cbuf->getCapacity()));
		return false;
	}
	size_t old_size = this->getDlength();
	if(size > old_size){
		LOG4CXX_WARN(logger, "Can't extend data segment : " + boost::lexical_cast<string>(size) +
			     " > " + boost::lexical_cast<string>(old_size));
		return false;
	}
	/** reserve data segment */
	this->setDlength(size);
	/** @todo : shrink common buffer */
	if(!cbuf->stepBackwardTail(old_size - size)){
		LOG4CXX_WARN(logger, "Too small to shrink data segment : " + boost::lexical_cast<string>(size) +
			     " > " + boost::lexical_cast<string>(old_size) +
			     "/" + boost::lexical_cast<string>(cbuf->getRemain()));
		return false;
	}
	return true;
}


CommonBufPtr DataInGenerator::serialize()
{
	/** padding */
	if(this->getDlength()%PAD_WORD_LEN)
		cbuf->stepForwardTail(PAD_WORD_LEN - this->getDlength()%PAD_WORD_LEN);
	/** @todo : support bi-directional Read/Write with AHS */
	return cbuf;
}
