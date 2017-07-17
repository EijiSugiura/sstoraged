/**
   @file tokenizer.h
   @brief iSCSI protocol tokenizer
   @author Eiji Sugiura <sugiura@isp.co.jp> Research Institute of Systems Planning, Inc
   @version $Id: tokenizer.h 140 2007-06-14 06:15:02Z sugiura $
 */

#ifndef __ISCSI_TOKENIZER_H__
#define __ISCSI_TOKENIZER_H__

#include <stdint.h>
#include <queue>
#include <list>
#include "ioveccontainer.h"
#include "iscsi/iscsi.h"

using namespace std;

class IovecContainer;
class WriteSegment;

struct iSCSIToken {
	iSCSIToken(iSCSIHeader *_bhs) : bhs(_bhs) {}
	iSCSIHeader *bhs;
	list<CommonBufPtr> cbufs;	
};

typedef boost::shared_ptr<iSCSIToken> iSCSITokenPtr;

class iSCSITokenizer {
public:
	iSCSITokenizer() : status(TOKEN_HEAD), offset(0) {}
	~iSCSITokenizer() {}

	bool push(IovecContainer *buf, size_t size);
	iSCSIHeader *frontHeader() const { return bhss.front()->bhs; }
	boost::shared_ptr<WriteSegment>
		frontDataSegment(const uint64_t lba,
				 const uint32_t transfer_length) const;
	bool pop();
	bool empty() const { return bhss.empty(); }
private:
	queue<iSCSITokenPtr> bhss;
	typedef enum {
		TOKEN_HEAD,
		TOKEN_PARTIAL,
		TOKEN_COMPLETE,
	} token_status_t;
	token_status_t status;
	size_t offset;
};

#endif /* __ISCSI_TOKENIZER_H__ */
