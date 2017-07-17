#include <cxxtest/TestSuite.h>
#include <sstream>
#include <iostream>
#include "manager.h"
#include "session.h"
#include "task.h"

using namespace std;

vector<string> strings;
static iSCSIConnectionPtr conn;
class SendTestSuite : public CxxTest::TestSuite 
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
	   cbuf offset=0 length=1024[B]
	 */
	void testSingleCbuf01()
	{
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		uint32_t length = 1024;
		uint32_t offset = 0;		
		cbuf->stepForwardTail(length);
		SendTaskPtr task = SendTaskPtr(new SendTask(parent, fd, cbuf));
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
	   cbuf offset=128 length=1024[B]
	 */
	void testSingleCbuf02()
	{
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		uint32_t length = 1024;
		uint32_t offset = 128;
		cbuf->stepForwardTail(offset + length);
		cbuf->stepForwardOffset(offset);
		SendTaskPtr task = SendTaskPtr(new SendTask(parent, fd, cbuf));
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
	   cbuf offset=COMMON_BUF_SIZE-1024 length=1024[B]
	 */
	void testSingleCbuf03()
	{
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		uint32_t length = 1024;
		uint32_t offset = COMMON_BUF_SIZE-1024;
		cbuf->stepForwardTail(offset + length);
		cbuf->stepForwardOffset(offset);
		SendTaskPtr task = SendTaskPtr(new SendTask(parent, fd, cbuf));
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
	   cbuf1 offset=COMMON_BUF_SIZE-512 length=512[B]
	   cbuf2 offset=0         length=512[B]
	 */
	void testDoubleCbuf01()
	{
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		uint32_t length = 512;
		uint32_t offset = COMMON_BUF_SIZE-length;
		cbuf->stepForwardTail(offset + length);
		cbuf->stepForwardOffset(offset);
		SendTaskPtr task = SendTaskPtr(new SendTask(parent, fd, cbuf));
		CommonBufPtr cbuf2 = CommonBufPtr(new CommonBuf);
		cbuf2->stepForwardTail(512);
		task->push(cbuf2);
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
	   cbuf1 offset=COMMON_BUF_SIZE-500 length=500[B]
	   cbuf2 offset=0         length=524[B]
	 */
	void testDoubleCbuf02()
	{
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		uint32_t length = 500;
		uint32_t offset = COMMON_BUF_SIZE-length;
		cbuf->stepForwardTail(offset + length);
		cbuf->stepForwardOffset(offset);
		SendTaskPtr task = SendTaskPtr(new SendTask(parent, fd, cbuf));
		CommonBufPtr cbuf2 = CommonBufPtr(new CommonBuf);
		cbuf2->stepForwardTail(524);
		task->push(cbuf2);
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
	}
	/**
	   cbuf1 offset=COMMON_BUF_SIZE-128 length=128[B]
	   cbuf2 offset=0         length=896[B]
	 */
	void testDoubleCbuf03()
	{
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		uint32_t length = 128;
		uint32_t offset = COMMON_BUF_SIZE-length;
		cbuf->stepForwardTail(offset + length);
		cbuf->stepForwardOffset(offset);
		SendTaskPtr task = SendTaskPtr(new SendTask(parent, fd, cbuf));
		CommonBufPtr cbuf2 = CommonBufPtr(new CommonBuf);
		cbuf2->stepForwardTail(896);
		task->push(cbuf2);
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

size_t SendCacheTask::size() const {return 0; }
vector<CommonBufPtr>::iterator SendCacheTask::begin(size_t& offset)
{
	vector<CommonBufPtr>::iterator null;
	return null;
}
string SCSI::DUMPBHS(const uint8_t *ptr)
{
	return "";
}
const socket_t INVALID_SOCKET = -1;
