/**
   @file tokenizer.cpp
   @brief iSCSI protocol tokenizer
   @author Eiji Sugiura <eiji.sugiura@gmail.com>
   @version $Id: tokenizer.cpp 277 2007-08-14 14:44:47Z sugiura $
 */

#include <boost/lexical_cast.hpp>
#include "logger.h"
#include "session.h"
#include "ioveccontainer.h"
#include "writecache.h"
#include "iscsi/iscsi.h"
#include "iscsi/tokenizer.h"

bool iSCSITokenizer::push(IovecContainer *buf, size_t size)
{
	iSCSIHeader *bhs = NULL;
	iSCSITokenPtr token;
	size_t pdu_length = 0;
	if(!buf->stepForwardTail(size)){
		LOG4CXX_ERROR(logger, "Failed to step forward tail of recvbuf");
		return false;
	}
	offset += size;
 multi_tokens:
#ifdef TOKENIZER_DEBUG
	LOG4CXX_DEBUG(logger, "REF : " + boost::lexical_cast<string>(buf->front()->head())
		      + "/" + boost::lexical_cast<string>(buf->front()->ref()) +
		      " remain " + boost::lexical_cast<string>(offset));
#endif
	switch(status){
	case TOKEN_HEAD:
		if(offset < sizeof(iSCSIHeader)){
#ifdef TOKENIZER_DEBUG
			LOG4CXX_DEBUG(logger, "Partial Head " + 
				      boost::lexical_cast<string>(sizeof(iSCSIHeader)) +
				      " > " +
				      boost::lexical_cast<string>(offset));
#endif
			break;
		}
		// fall through
	case TOKEN_PARTIAL:
		bhs = reinterpret_cast<iSCSIHeader*>(buf->front()->ref());
		if(!bhs) {
			LOG4CXX_ERROR(logger, "Internal buffer error : " +
				      static_cast<string>(__PRETTY_FUNCTION__));
			return false;
		}
		if(buf->front()->isEdge(reinterpret_cast<uint8_t*>(bhs),
					sizeof(iSCSIHeader))){
			/** @todo: handle header on edge!  */
			size_t size = sizeof(iSCSIHeader) - 
				(buf->front()->getSize() -
				 buf->front()->getOffset(reinterpret_cast<uint8_t*>(bhs)));
			memcpy(buf->front()->edge(), buf->second()->head(), size);
			LOG4CXX_DEBUG(logger, "Header on edge : "
				      + SCSI::DUMPBHS(reinterpret_cast<uint8_t*>(bhs)));
		}
		{
			iSCSIParser<iSCSIHeader> parser(bhs);
			pdu_length = parser.getPDUlength();
#if TOKENIZER_DEBUG || BHS_DEBUG
			LOG4CXX_DEBUG(logger, "BHS : " + boost::lexical_cast<string>(bhs) + " offset " +
				      boost::lexical_cast<string>(buf->front()->getOffset(reinterpret_cast<uint8_t*>(bhs))) +
				      " pdu " + boost::lexical_cast<string>(pdu_length) +
				      " itt " + boost::lexical_cast<string>(parser.getITT()));
#endif
		}
		if(pdu_length > offset){
#ifdef TOKENIZER_DEBUG
			LOG4CXX_DEBUG(logger, "Partial PDU " + 
				      boost::lexical_cast<string>(pdu_length) +
				      " > " +
				      boost::lexical_cast<string>(offset));
#endif
			status = TOKEN_PARTIAL;
			break;
		}
		// fall through
	case TOKEN_COMPLETE:
		token = iSCSITokenPtr(new iSCSIToken(bhs));
		/** step forward IovecContainer */
		offset -= pdu_length;
		while(pdu_length > 0){
			CommonBufPtr cbuf = buf->stepForwardOffset(pdu_length);
			if(cbuf.get() == NULL){
				LOG4CXX_WARN(logger, "NOT Push cbuf to tokenizer");
				continue;
			}
			token->cbufs.push_back(cbuf);
#ifdef TOKENIZER_DEBUG
			LOG4CXX_DEBUG(logger, "Push cbuf " +
				      boost::lexical_cast<string>(cbuf.get()) + " " +
				      boost::lexical_cast<string>(cbuf->head()) + " read " +
				      boost::lexical_cast<string>(cbuf->getRemain()) + " capacity " +
				      boost::lexical_cast<string>(cbuf->getCapacity()) +
				      " to tokenizer");
#endif
		}
		bhss.push(token);
		status = TOKEN_HEAD;
		if(offset > 0){
			LOG4CXX_DEBUG(logger, "Multiple tokens");
			buf->update();
			goto multi_tokens;
		}
		break;
	default:
		LOG4CXX_ERROR(logger, "Internal status error : " +
			      static_cast<string>(__PRETTY_FUNCTION__));
		return false;
	}
	buf->update();
	return true;
}

bool iSCSITokenizer::pop()
{
	if(bhss.empty())
		return false;
#ifdef TOKENIZER_DEBUG
 	iSCSITokenPtr token = bhss.front();
	for(list<CommonBufPtr>::const_iterator itr = token->cbufs.begin();
	    itr != token->cbufs.end(); ++itr){
		LOG4CXX_DEBUG(logger, "Pop cbuf " +
			      boost::lexical_cast<string>((*itr).get()) + " " +
			      boost::lexical_cast<string>((*itr)->head()) +
			      " from tokenizer");
	}
#endif
	bhss.pop();
	return true;
}

WriteSegmentPtr iSCSITokenizer::frontDataSegment(const uint64_t lba,
						 const uint32_t transfer_length) const
{
	WriteSegmentPtr ret, null;
	iSCSITokenPtr token = bhss.front();
	iSCSIHeader *bhs = token->bhs;
	iSCSIParser<iSCSIHeader> parser(bhs);
#if TOKENIZER_DEBUG || BHS_DEBUG
	LOG4CXX_DEBUG(logger, "BHS : " + boost::lexical_cast<string>(bhs) + " offset " +
		      boost::lexical_cast<string>(token->cbufs.front()->getOffset(reinterpret_cast<uint8_t*>(bhs))));
#endif
	size_t dlength = parser.getDlength();
	size_t ds_offset = parser.getDataSegmentOffset();
	if(ds_offset == 0){
		LOG4CXX_ERROR(logger, "There's no data segment!");
		return ret;
	}
	list<CommonBufPtr>::const_iterator itr = token->cbufs.begin();
	if((*itr)->isEdge(reinterpret_cast<uint8_t*>(bhs), sizeof(iSCSIHeader))){
		/** Data segment is begining in 2nd cbuf */
		size_t offset = (*itr)->getOffset(reinterpret_cast<uint8_t*>(bhs));
		size_t size = (*itr)->getSize();
		if(offset > size){
			LOG4CXX_ERROR(logger, "Too larget offset for CommonBuf size");
			return null;
		}
		/** subtract partial header length in 1st cbuf */
		ds_offset = sizeof(iSCSIHeader) - (size - offset);
		/** step forward to 2nd cbuf */
		if(++itr == token->cbufs.end()){
			LOG4CXX_ERROR(logger, "1st Data segment has vanished!");
			return null;
		}
	} else {
		ds_offset += (*itr)->getOffset(reinterpret_cast<uint8_t*>(bhs));
	}
	/** create WriteSegment */
	ret = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
	size_t stored = min(dlength, ((*itr)->getSize() - ds_offset));
	if(!ret->set1stDataSegment(*itr, stored, ds_offset)){
		return null;
	}
#ifdef TOKENIZER_DEBUG
	LOG4CXX_DEBUG(logger, "WriteSegment : " +
		      boost::lexical_cast<string>(ret->lba) + "+" +
		      boost::lexical_cast<string>(ret->transfer_length));
	LOG4CXX_DEBUG(logger, "1st Data Segment : "+
		      boost::lexical_cast<string>((*itr)->head()) + "+" +
		      boost::lexical_cast<string>(ds_offset) + "=" +
		      SCSI::DUMPOCTETS(reinterpret_cast<uint8_t*>((*itr)->head())
				       + ds_offset) + " -> " +
		      boost::lexical_cast<string>(stored) );
#endif
	/** handle multiple DataSegment */
	while(dlength > stored){
		if(++itr == token->cbufs.end()){
			LOG4CXX_ERROR(logger, "2nd Data segment has vanished!");
			return null;
		}
		size_t size = min(dlength-stored, (*itr)->getSize());
		if(!ret->addDataSegment(*itr, size)){
			LOG4CXX_ERROR(logger, "Too match Data segment : " +
				      boost::lexical_cast<string>(size));
			return null;
		}
#ifdef TOKENIZER_DEBUG
		LOG4CXX_DEBUG(logger, "Add multiple Data Segment : " +
			      SCSI::DUMPOCTETS(reinterpret_cast<uint8_t*>((*itr)->head())) + " -> " +
			      boost::lexical_cast<string>(size));
#endif
		stored += size;
	}
	return ret;
}
