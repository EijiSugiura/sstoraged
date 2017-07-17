#include <cxxtest/TestSuite.h>
#include <sstream>
#include <iostream>
#include "manager.h"
#include "session.h"
#include "task.h"
#include "writecache.h"

using namespace std;

#include "diskwriter.h"

vector<string> strings;
static iSCSIConnectionPtr conn;
class SendCacheTestSuite : public CxxTest::TestSuite 
{
	iSCSISessionPtr parent;
	static const socket_t fd = 10;
public:
	void setUp()
	{
		if(parent.get() == NULL){
			try {
				parent = iSCSISessionPtr(new iSCSISession);
				PortPtr client_port = PortPtr(new Port(10));
				PortListenerPtr listner;
				conn = iSCSIConnectionPtr(new iSCSIConnection(client_port,
										  listner));
				parent->connection.add(conn);
			} catch(...){
			}
		}
	}
	void tearDown()
	{
		strings.clear();
	}
	/**
	   [5:2) = 1024[B]
	   cbuf offset=0
	 */
	void testSingleCbuf01()
	{
		uint64_t lba = 5;
		uint32_t transfer_length = 2;
		uint32_t offset = 0;
		size_t size = SCSI::LBA2OFFSET(transfer_length);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba, transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,size,offset), true);
		SendCacheTaskPtr task = SendCacheTaskPtr(new SendCacheTask(parent, fd, ws,
									   lba, transfer_length));
		TS_ASSERT_EQUALS(task->run(), true);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		void *head = cbuf->head();
		void *ptr = (uint8_t*)head + offset;
		TS_ASSERT_EQUALS(strings[0], (string)"vec[0] " +
				 boost::lexical_cast<string>(head) + "/" +
				 boost::lexical_cast<string>(ptr) + "+1024");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[2], (string)"vec[0] " +
				 boost::lexical_cast<string>(head) + "/" +
				 boost::lexical_cast<string>(ptr) + "+524");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[4], (string)"vec[0] " +
				 boost::lexical_cast<string>(head) + "/" +
				 boost::lexical_cast<string>(ptr) + "+24");
		TS_ASSERT_EQUALS(strings.size(), 6UL);
	}
	/**
	   [5:2) = 1024[B]
	   cbuf offset=128
	 */
	void testSingleCbuf02()
	{
		uint64_t lba = 5;
		uint32_t transfer_length = 2;
		uint32_t offset = 128;
		size_t size = SCSI::LBA2OFFSET(transfer_length);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba, transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,size,offset), true);
		SendCacheTaskPtr task = SendCacheTaskPtr(new SendCacheTask(parent, fd, ws,
									   lba, transfer_length));
		TS_ASSERT_EQUALS(task->run(), true);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		void *head = cbuf->head();
		void *ptr = (uint8_t*)head + offset;
		TS_ASSERT_EQUALS(strings[0], (string)"vec[0] " +
				 boost::lexical_cast<string>(head) + "/" +
				 boost::lexical_cast<string>(ptr) + "+1024");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[2], (string)"vec[0] " +
				 boost::lexical_cast<string>(head) + "/" +
				 boost::lexical_cast<string>(ptr) + "+524");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[4], (string)"vec[0] " +
				 boost::lexical_cast<string>(head) + "/" +
				 boost::lexical_cast<string>(ptr) + "+24");
		TS_ASSERT_EQUALS(strings.size(), 6UL);
	}
	/**
	   [5:2) = 1024[B]
	   cbuf offset=COMMON_BUF_SIZE-1024
	 */
	void testSingleCbuf03()
	{
		uint64_t lba = 5;
		uint32_t transfer_length = 2;
		uint32_t offset = COMMON_BUF_SIZE-1024;
		size_t size = SCSI::LBA2OFFSET(transfer_length);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba, transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,size,offset) , true);
		SendCacheTaskPtr task = SendCacheTaskPtr(new SendCacheTask(parent, fd, ws,
									   lba, transfer_length));
		TS_ASSERT_EQUALS(task->run(), true);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		void *head = cbuf->head();
		void *ptr = (uint8_t*)head + offset;
		TS_ASSERT_EQUALS(strings[0], (string)"vec[0] " +
				 boost::lexical_cast<string>(head) + "/" +
				 boost::lexical_cast<string>(ptr) + "+1024");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[2], (string)"vec[0] " +
				 boost::lexical_cast<string>(head) + "/" +
				 boost::lexical_cast<string>(ptr) + "+524");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[4], (string)"vec[0] " +
				 boost::lexical_cast<string>(head) + "/" +
				 boost::lexical_cast<string>(ptr) + "+24");
		TS_ASSERT_EQUALS(strings.size(), 6UL);
	}
	/**
	   [5:2) = 1024[B]
	   1st cbuf offset=COMMON_BUF_SIZE-512
	 */
	void testDoubleCbuf01()
	{
		uint64_t lba = 5;
		uint32_t transfer_length = 2;
		uint32_t offset = COMMON_BUF_SIZE-512;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba, transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,512,offset), true);
		CommonBufPtr cbuf2 = CommonBufPtr(new CommonBuf);
		TS_ASSERT_EQUALS(ws->addDataSegment(cbuf2,512), true);
		SendCacheTaskPtr task = SendCacheTaskPtr(new SendCacheTask(parent, fd, ws,
									   lba, transfer_length));
		TS_ASSERT_EQUALS(task->run(), true);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
 		void *head = cbuf->head();
 		void *ptr = (uint8_t*)head + offset;
 		TS_ASSERT_EQUALS(strings[0], (string)"vec[0] " +
 				 boost::lexical_cast<string>(head) + "/" +
 				 boost::lexical_cast<string>(ptr) + "+512");
 		void *head2 = cbuf2->head();
 		void *ptr2 = (uint8_t*)head2;
 		TS_ASSERT_EQUALS(strings[1], (string)"vec[1] " +
 				 boost::lexical_cast<string>(head2) + "/" +
 				 boost::lexical_cast<string>(ptr2) + "+512");
 		ptr = (uint8_t*)ptr + 500;
 		TS_ASSERT_EQUALS(strings[3], (string)"vec[0] " +
 				 boost::lexical_cast<string>(head) + "/" +
 				 boost::lexical_cast<string>(ptr) + "+12");
 		TS_ASSERT_EQUALS(strings[4], (string)"vec[1] " +
 				 boost::lexical_cast<string>(head2) + "/" +
 				 boost::lexical_cast<string>(ptr2) + "+512");
 		ptr2 = (uint8_t*)ptr2 + 488;
 		TS_ASSERT_EQUALS(strings[6], (string)"vec[0] " +
 				 boost::lexical_cast<string>(head2) + "/" +
 				 boost::lexical_cast<string>(ptr2) + "+24");
		TS_ASSERT_EQUALS(strings.size(), 8UL);
	}
	/**
	   [5:2) = 1024[B]
	   1st cbuf offset=COMMON_BUF_SIZE-500
	 */
	void testDoubleCbuf02()
	{
		uint64_t lba = 5;
		uint32_t transfer_length = 2;
		uint32_t offset = COMMON_BUF_SIZE-500;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba, transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,512,offset), true);
		CommonBufPtr cbuf2 = CommonBufPtr(new CommonBuf);
		TS_ASSERT_EQUALS(ws->addDataSegment(cbuf2,512), true);
		SendCacheTaskPtr task = SendCacheTaskPtr(new SendCacheTask(parent, fd, ws,
									   lba, transfer_length));
		TS_ASSERT_EQUALS(task->run(), true);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
 		void *head = cbuf->head();
 		void *ptr = (uint8_t*)head + offset;
 		TS_ASSERT_EQUALS(strings[0], (string)"vec[0] " +
 				 boost::lexical_cast<string>(head) + "/" +
 				 boost::lexical_cast<string>(ptr) + "+500");
 		void *head2 = cbuf2->head();
 		void *ptr2 = (uint8_t*)head2;
 		TS_ASSERT_EQUALS(strings[1], (string)"vec[1] " +
 				 boost::lexical_cast<string>(head2) + "/" +
 				 boost::lexical_cast<string>(ptr2) + "+524");
 		TS_ASSERT_EQUALS(strings[3], (string)"vec[0] " +
 				 boost::lexical_cast<string>(head2) + "/" +
 				 boost::lexical_cast<string>(ptr2) + "+524");
 		ptr2 = (uint8_t*)ptr2 + 500;
 		TS_ASSERT_EQUALS(strings[5], (string)"vec[0] " +
 				 boost::lexical_cast<string>(head2) + "/" +
 				 boost::lexical_cast<string>(ptr2) + "+24");
		TS_ASSERT_EQUALS(strings.size(), 7UL);
	}
	/**
	   [5:2) = 1024[B]
	   1st cbuf offset=COMMON_BUF_SIZE-128
	 */
	void testDoubleCbuf03()
	{
		uint64_t lba = 5;
		uint32_t transfer_length = 2;
		uint32_t offset = COMMON_BUF_SIZE-128;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba, transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,128,offset), true);
		CommonBufPtr cbuf2 = CommonBufPtr(new CommonBuf);
		TS_ASSERT_EQUALS(ws->addDataSegment(cbuf2,896), true);
		SendCacheTaskPtr task = SendCacheTaskPtr(new SendCacheTask(parent, fd, ws,
									   lba, transfer_length));
		TS_ASSERT_EQUALS(task->run(), true);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
 		void *head = cbuf->head();
 		void *ptr = (uint8_t*)head + offset;
 		TS_ASSERT_EQUALS(strings[0], (string)"vec[0] " +
 				 boost::lexical_cast<string>(head) + "/" +
 				 boost::lexical_cast<string>(ptr) + "+128");
 		void *head2 = cbuf2->head();
 		void *ptr2 = (uint8_t*)head2;
 		TS_ASSERT_EQUALS(strings[1], (string)"vec[1] " +
 				 boost::lexical_cast<string>(head2) + "/" +
 				 boost::lexical_cast<string>(ptr2) + "+896");
 		ptr2 = (uint8_t*)ptr2 + 372;
 		TS_ASSERT_EQUALS(strings[3], (string)"vec[0] " +
 				 boost::lexical_cast<string>(head2) + "/" +
 				 boost::lexical_cast<string>(ptr2) + "+524");
 		ptr2 = (uint8_t*)ptr2 + 500;
 		TS_ASSERT_EQUALS(strings[5], (string)"vec[0] " +
 				 boost::lexical_cast<string>(head2) + "/" +
 				 boost::lexical_cast<string>(ptr2) + "+24");
		TS_ASSERT_EQUALS(strings.size(), 7UL);
	}
	/**
	   cache[5:2) = 1024[B] = 0x00*512 + 0x01*512
	   send [6:1)
	   cbuf offset=0
	 */
	void testSplittedCbuf01()
	{
		uint64_t lba = 5;
		uint32_t transfer_length = 2;
		uint64_t clba = 6;
		uint32_t ctransfer_length = 1;
		uint32_t offset = 0;
		size_t size = SCSI::LBA2OFFSET(transfer_length);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba, transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);

		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,size,offset), true);
		SendCacheTaskPtr task = SendCacheTaskPtr(new SendCacheTask(parent, fd, ws,
									   clba, ctransfer_length));
		TS_ASSERT_EQUALS(task->run(), true);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		void *head = cbuf->head();
		void *ptr = (uint8_t*)head + offset + SCSI::LBA2OFFSET(clba-lba);
		TS_ASSERT_EQUALS(strings[0], (string)"vec[0] " +
				 boost::lexical_cast<string>(head) + "/" +
				 boost::lexical_cast<string>(ptr) + "+512");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[2], (string)"vec[0] " +
				 boost::lexical_cast<string>(head) + "/" +
				 boost::lexical_cast<string>(ptr) + "+12");
		TS_ASSERT_EQUALS(strings.size(), 4UL);
	}
	/**
	   cache[5:2) = 1024[B] = 0x00*512 + 0x01*512
	   send [6:1)
	   cbuf offset=COMMON_BUF_SIZE-512
	 */
	void testSplittedCbuf02()
	{
		uint64_t lba = 5;
		uint32_t transfer_length = 2;
		uint64_t clba = 6;
		uint32_t ctransfer_length = 1;
		uint32_t offset = COMMON_BUF_SIZE-512;
		size_t size = SCSI::LBA2OFFSET(transfer_length);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba, transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,512,offset), true);
		CommonBufPtr cbuf2 = CommonBufPtr(new CommonBuf);
		TS_ASSERT_EQUALS(ws->addDataSegment(cbuf2,512), true);

		SendCacheTaskPtr task = SendCacheTaskPtr(new SendCacheTask(parent, fd, ws,
									   clba, ctransfer_length));
		TS_ASSERT_EQUALS(task->run(), true);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
 		void *head2 = cbuf2->head();
 		void *ptr2 = (uint8_t*)head2;
 		TS_ASSERT_EQUALS(strings[0], (string)"vec[0] " +
 				 boost::lexical_cast<string>(head2) + "/" +
 				 boost::lexical_cast<string>(ptr2) + "+512");
		ptr2 = (uint8_t*)ptr2 + 500;
		TS_ASSERT_EQUALS(strings[2], (string)"vec[0] " +
				 boost::lexical_cast<string>(head2) + "/" +
				 boost::lexical_cast<string>(ptr2) + "+12");
		TS_ASSERT_EQUALS(strings.size(), 4UL);
	}
	/**
	   cache[5:2) = 1024[B] = 0x00*512 + 0x01*512
	   send [6:1)
	   cbuf offset=COMMON_BUF_SIZE-128
	 */
	void testSplittedCbuf03()
	{
		uint64_t lba = 5;
		uint32_t transfer_length = 2;
		uint64_t clba = 6;
		uint32_t ctransfer_length = 1;
		uint32_t offset = COMMON_BUF_SIZE-128;
		size_t size = SCSI::LBA2OFFSET(transfer_length);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba, transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,128,offset), true);
		CommonBufPtr cbuf2 = CommonBufPtr(new CommonBuf);
		TS_ASSERT_EQUALS(ws->addDataSegment(cbuf2,896), true);

		SendCacheTaskPtr task = SendCacheTaskPtr(new SendCacheTask(parent, fd, ws,
									   clba, ctransfer_length));
		TS_ASSERT_EQUALS(task->run(), true);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\nzz"));
 		void *head2 = cbuf2->head();
 		void *ptr2 = (uint8_t*)head2 + 384;
 		TS_ASSERT_EQUALS(strings[0], (string)"vec[0] " +
 				 boost::lexical_cast<string>(head2) + "/" +
 				 boost::lexical_cast<string>(ptr2) + "+512");
		ptr2 = (uint8_t*)ptr2 + 500;
		TS_ASSERT_EQUALS(strings[2], (string)"vec[0] " +
				 boost::lexical_cast<string>(head2) + "/" +
				 boost::lexical_cast<string>(ptr2) + "+12");
		TS_ASSERT_EQUALS(strings.size(), 4UL);
	}
};

iSCSIManager::iSCSIManager() throw() {}
iSCSIManager & iSCSIManager::getInstance() throw(std::out_of_range)
{
	static iSCSIManager dummy;
	return dummy;
}
iSCSISession::iSCSISession() : manager(iSCSIManager::getInstance()){}
iSCSIConnectionPtr iSCSISession::Connection::find(int) const
{
	return conn;
}
bool iSCSISession::Connection::add(boost::shared_ptr<iSCSIConnection> const& conn) throw()
{ return true; }
iSCSISession::~iSCSISession() {}

iSCSIConnection::iSCSIConnection(boost::shared_ptr<Port>, boost::shared_ptr<PortListener>)
{}
void iSCSIConnection::unwaitWritable() {}
void iSCSIConnection::waitWritable() {}
iSCSIConnection::~iSCSIConnection() {}

DiskWriter::DiskWriter(boost::shared_ptr<WriteCache> _wc){}
DiskWriter::~DiskWriter(){}
void DiskWriter::operator()() throw (runtime_error) {}
bool DiskWriter::doWork() { return true; }

