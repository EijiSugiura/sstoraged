#include <cxxtest/TestSuite.h>
#include <sstream>
#include <iostream>
#include "manager.h"
#include "session.h"
#include "volume.h"
#include "scsi/read.h"

using namespace std;

#include "diskwriter.h"
#include "writecache.h"

ostream& operator<<(ostream& os, const WriteSegmentPtr& v) {
	return os << v->dump();
}

vector<string> strings;

iSCSIInitiator::iSCSIInitiator(const string  &iname)
	: name(iname), alias(), authens(),
	  max_recv_data_segment_length(DEFAULT_MAX_RECV_DATA_SEGMENT_LENGTH)
{}

class ReadCacheTestSuite : public CxxTest::TestSuite 
{
	iSCSISessionPtr parent;
	iSCSIInitiatorPtr initiator;
	LogicalVolumePtr lv;
	ReadHandler *reader;
	socket_t fd;
public:
	void setUp()
	{
		if(parent.get() == NULL) {
			parent = iSCSISessionPtr(new iSCSISession);
			initiator = iSCSIInitiatorPtr(new iSCSIInitiator("dummy"));
			parent->initiator.set(initiator);
		}
		if(!reader)
			reader = new ReadHandler(parent.get());
	}
	void tearDown()
	{
		strings.clear();
	}
	/**
	   segment : [5:5)
	   search  : [0:15)	   
	 */
	void testCacheIncludedBySearchRange01()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(5,5));
		wc.push(ws);

		uint64_t lba = 0;
		uint32_t transfer_length = 15;
		vector<WriteSegmentPtr> wss = wc.search(lba,transfer_length);
		cout << "Found : ";
		copy(wss.begin(),wss.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		vector<WriteSegmentPtr> hits = reader->getHitSegments(wss,lba,transfer_length);
		cout << "Hit : ";
		copy(hits.begin(),hits.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		cout << "SendCache : " << (unsigned long long)lba <<
			"+" << (unsigned long)transfer_length << endl;
		reader->sendCache(parent, fd, lv, hits, lba, transfer_length);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		TS_ASSERT_EQUALS(strings.size(), 4);
 		TS_ASSERT_EQUALS(strings[1],(string)"\tSendfile  : 0+5(2560)");
		TS_ASSERT_EQUALS(strings[2],(string)"\tSendCache : 5+5(2560)");
		TS_ASSERT_EQUALS(strings[3],(string)"\tSendfile  : 10+5(2560)");
	}
	/**
	   segment : [5:5)
	   search  : [1:15)
	 */
	void testCacheIncludedBySearchRange02()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(5,5));
		wc.push(ws);

		uint64_t lba = 1;
		uint32_t transfer_length = 15;
		vector<WriteSegmentPtr> wss = wc.search(lba,transfer_length);
		cout << "Found : ";
		copy(wss.begin(),wss.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		vector<WriteSegmentPtr> hits = reader->getHitSegments(wss,lba,transfer_length);
		cout << "Hit : ";
		copy(hits.begin(),hits.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		cout << "SendCache : " << (unsigned long long)lba <<
			"+" << (unsigned long)transfer_length << endl;
		reader->sendCache(parent, fd, lv, hits, lba, transfer_length);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		TS_ASSERT_EQUALS(strings.size(), 4);
 		TS_ASSERT_EQUALS(strings[1],(string)"\tSendfile  : 1+4(2048)");
		TS_ASSERT_EQUALS(strings[2],(string)"\tSendCache : 5+5(2560)");
		TS_ASSERT_EQUALS(strings[3],(string)"\tSendfile  : 10+6(3072)");
	}
	/**
	   segment : [5:5) [10:5)
	   search  : [1:20)
	 */
	void testMultiCacheIncludedBySearchRange01()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(5,5));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(10,5));
		wc.push(ws);

		uint64_t lba = 1;
		uint32_t transfer_length = 20;
		vector<WriteSegmentPtr> wss = wc.search(lba,transfer_length);
		cout << "Found : ";
		copy(wss.begin(),wss.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		vector<WriteSegmentPtr> hits = reader->getHitSegments(wss,lba,transfer_length);
		cout << "Hit : ";
		copy(hits.begin(),hits.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		cout << "SendCache : " << (unsigned long long)lba <<
			"+" << (unsigned long)transfer_length << endl;
		reader->sendCache(parent, fd, lv, hits, lba, transfer_length);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		TS_ASSERT_EQUALS(strings.size(), 5);
 		TS_ASSERT_EQUALS(strings[1],(string)"\tSendfile  : 1+4(2048)");
		TS_ASSERT_EQUALS(strings[2],(string)"\tSendCache : 5+5(2560)");
		TS_ASSERT_EQUALS(strings[3],(string)"\tSendCache : 10+5(2560)");
		TS_ASSERT_EQUALS(strings[4],(string)"\tSendfile  : 15+6(3072)");
	}
	/**
	   segment : [5:5) [11:5)
	   search  : [1:20)
	 */
	void testMultiCacheIncludedBySearchRange02()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(5,5));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(11,5));
		wc.push(ws);

		uint64_t lba = 1;
		uint32_t transfer_length = 20;
		vector<WriteSegmentPtr> wss = wc.search(lba,transfer_length);
		cout << "Found : ";
		copy(wss.begin(),wss.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		vector<WriteSegmentPtr> hits = reader->getHitSegments(wss,lba,transfer_length);
		cout << "Hit : ";
		copy(hits.begin(),hits.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		cout << "SendCache : " << (unsigned long long)lba <<
			"+" << (unsigned long)transfer_length << endl;
		reader->sendCache(parent, fd, lv, hits, lba, transfer_length);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		TS_ASSERT_EQUALS(strings.size(), 6);
 		TS_ASSERT_EQUALS(strings[1],(string)"\tSendfile  : 1+4(2048)");
		TS_ASSERT_EQUALS(strings[2],(string)"\tSendCache : 5+5(2560)");
		TS_ASSERT_EQUALS(strings[3],(string)"\tSendfile  : 10+1(512)");
		TS_ASSERT_EQUALS(strings[4],(string)"\tSendCache : 11+5(2560)");
		TS_ASSERT_EQUALS(strings[5],(string)"\tSendfile  : 16+5(2560)");
	}
	/**
	   segment : [0:15)
	   search  : [5:10)
	 */
	void testSearchRangeIncludedByCache01()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(0,15));
		wc.push(ws);

		uint64_t lba = 5;
		uint32_t transfer_length = 5;
		vector<WriteSegmentPtr> wss = wc.search(lba,transfer_length);
		cout << "Found : ";
		copy(wss.begin(),wss.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		vector<WriteSegmentPtr> hits = reader->getHitSegments(wss,lba,transfer_length);
		cout << "Hit : ";
		copy(hits.begin(),hits.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		cout << "SendCache : " << (unsigned long long)lba <<
			"+" << (unsigned long)transfer_length << endl;
		reader->sendCache(parent, fd, lv, hits, lba, transfer_length);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		TS_ASSERT_EQUALS(strings.size(), 2);
		TS_ASSERT_EQUALS(strings[1],(string)"\tSendCache : 5+5(2560)");
	}
	/**
	   segment : [1:15)
	   search  : [5:10)
	 */
	void testSearchRangeIncludedByCache02()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(1,15));
		wc.push(ws);

		uint64_t lba = 5;
		uint32_t transfer_length = 5;
		vector<WriteSegmentPtr> wss = wc.search(lba,transfer_length);
		cout << "Found : ";
		copy(wss.begin(),wss.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		vector<WriteSegmentPtr> hits = reader->getHitSegments(wss,lba,transfer_length);
		cout << "Hit : ";
		copy(hits.begin(),hits.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		cout << "SendCache : " << (unsigned long long)lba <<
			"+" << (unsigned long)transfer_length << endl;
		reader->sendCache(parent, fd, lv, hits, lba, transfer_length);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		TS_ASSERT_EQUALS(strings.size(), 2);
		TS_ASSERT_EQUALS(strings[1],(string)"\tSendCache : 5+5(2560)");
	}
	/**
	   segment : [0:8)[8:12)
	   search  : [5:10)
	 */
	void testSearchRangeIncludedByMultiCache01()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(0,8));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(8,12));
		wc.push(ws);

		uint64_t lba = 5;
		uint32_t transfer_length = 5;
		vector<WriteSegmentPtr> wss = wc.search(lba,transfer_length);
		cout << "Found : ";
		copy(wss.begin(),wss.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		vector<WriteSegmentPtr> hits = reader->getHitSegments(wss,lba,transfer_length);
		cout << "Hit : ";
		copy(hits.begin(),hits.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		cout << "SendCache : " << (unsigned long long)lba <<
			"+" << (unsigned long)transfer_length << endl;
		reader->sendCache(parent, fd, lv, hits, lba, transfer_length);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		TS_ASSERT_EQUALS(strings.size(), 3);
		TS_ASSERT_EQUALS(strings[1],(string)"\tSendCache : 5+3(1536)");
		TS_ASSERT_EQUALS(strings[2],(string)"\tSendCache : 8+2(1024)");
	}
	/**
	   segment : [0:8)[9:12)
	   search  : [5:10)
	 */
	void testSearchRangeIncludedByMultiCache02()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(0,8));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(9,12));
		wc.push(ws);

		uint64_t lba = 5;
		uint32_t transfer_length = 5;
		vector<WriteSegmentPtr> wss = wc.search(lba,transfer_length);
		cout << "Found : ";
		copy(wss.begin(),wss.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		vector<WriteSegmentPtr> hits = reader->getHitSegments(wss,lba,transfer_length);
		cout << "Hit : ";
		copy(hits.begin(),hits.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		cout << "SendCache : " << (unsigned long long)lba <<
			"+" << (unsigned long)transfer_length << endl;
		reader->sendCache(parent, fd, lv, hits, lba, transfer_length);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		TS_ASSERT_EQUALS(strings.size(), 4);
		TS_ASSERT_EQUALS(strings[1],(string)"\tSendCache : 5+3(1536)");
		TS_ASSERT_EQUALS(strings[2],(string)"\tSendfile  : 8+1(512)");
		TS_ASSERT_EQUALS(strings[3],(string)"\tSendCache : 9+1(512)");
	}
	/**
	   segment : [5:5)
	   search  : [0:6)
	 */
	void testPartialOverwrap01()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(5,5));
		wc.push(ws);

		uint64_t lba = 0;
		uint32_t transfer_length = 6;
		vector<WriteSegmentPtr> wss = wc.search(lba,transfer_length);
		cout << "Found : ";
		copy(wss.begin(),wss.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		vector<WriteSegmentPtr> hits = reader->getHitSegments(wss,lba,transfer_length);
		cout << "Hit : ";
		copy(hits.begin(),hits.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		cout << "SendCache : " << (unsigned long long)lba <<
			"+" << (unsigned long)transfer_length << endl;
		reader->sendCache(parent, fd, lv, hits, lba, transfer_length);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		TS_ASSERT_EQUALS(strings.size(), 3);
 		TS_ASSERT_EQUALS(strings[1],(string)"\tSendfile  : 0+5(2560)");
		TS_ASSERT_EQUALS(strings[2],(string)"\tSendCache : 5+1(512)");
	}
	/**
	   segment : [5:5)
	   search  : [9:5)
	 */
	void testPartialOverwrap02()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(5,5));
		wc.push(ws);

		uint64_t lba = 9;
		uint32_t transfer_length = 5;
		vector<WriteSegmentPtr> wss = wc.search(lba,transfer_length);
		cout << "Found : ";
		copy(wss.begin(),wss.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		vector<WriteSegmentPtr> hits = reader->getHitSegments(wss,lba,transfer_length);
		cout << "Hit : ";
		copy(hits.begin(),hits.end(), ostream_iterator<WriteSegmentPtr>(cout," "));
		cout << endl;
		cout << "SendCache : " << (unsigned long long)lba <<
			"+" << (unsigned long)transfer_length << endl;
		reader->sendCache(parent, fd, lv, hits, lba, transfer_length);
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		TS_ASSERT_EQUALS(strings.size(), 3);
 		TS_ASSERT_EQUALS(strings[1],(string)"\tSendCache : 9+1(512)");
		TS_ASSERT_EQUALS(strings[2],(string)"\tSendfile  : 10+4(2048)");
	}
};

DataInGenerator::DataInGenerator() {}
DataInGenerator::DataInGenerator(DataInGenerator&) {}
CommonBufPtr DataInGenerator::serialize()
{
	CommonBufPtr null;
	return null;
}
LogicalVolumePtr iSCSIInitiator::getLUN(unsigned int) const
{
	LogicalVolumePtr null;
	return null;
}
iSCSIManager::iSCSIManager() throw() {}
iSCSIManager & iSCSIManager::getInstance() throw(std::out_of_range)
{
	static iSCSIManager dummy;
	return dummy;
}
iSCSISession::iSCSISession() : manager(iSCSIManager::getInstance()){}
iSCSISession::~iSCSISession() {}
iSCSIConnectionPtr iSCSISession::Connection::find(int) const
{
	iSCSIConnectionPtr null;
	return null;
}
SendCacheTask::SendCacheTask(boost::shared_ptr<iSCSISession> parent, socket_t _fd,
			     boost::shared_ptr<WriteSegment> _ws,
			     uint64_t lba, uint32_t length)
	: session(parent), fd(_fd), ws(_ws),
	  clba(lba), total_remain(SCSI::LBA2OFFSET(length)), cur_index(0)
{
	ostringstream os;
	os << "\tSendCache : " << (unsigned long)clba << 
		"+" << (unsigned long)SCSI::OFFSET2LBA(total_remain) <<
		"(" <<(unsigned long)total_remain << ")";
	strings.push_back(os.str());
}
bool SendCacheTask::run() {return true;}
SendTask::SendTask(boost::shared_ptr<iSCSISession>, int, boost::shared_ptr<CommonBuf>)
{}
bool SendTask::run() {return true;}
SendfileTask::SendfileTask(boost::shared_ptr<iSCSISession> parent,
			   const socket_t afd,
			   boost::shared_ptr<LogicalVolume> alv,
			   const uint64_t lba, const size_t length)
	: session(parent), fd(afd), lv(alv),
	  offset(SCSI::LBA2OFFSET(lba)), remain(SCSI::LBA2OFFSET(length))
{
	ostringstream os;
	os << "\tSendfile  : " << (unsigned long)lba << 
		"+" << (unsigned long)SCSI::OFFSET2LBA(remain) <<
		"(" <<(unsigned long)remain << ")";
	strings.push_back(os.str());

}
bool SendfileTask::run() {return true;}
bool TCPCorkTask::run() {return true;}
bool TCPUncorkTask::run() {return true;}

DiskWriter::DiskWriter(boost::shared_ptr<WriteCache> _wc){}
DiskWriter::~DiskWriter(){}
void DiskWriter::operator()() throw (runtime_error) {}
bool DiskWriter::doWork() { return true; }

const socket_t INVALID_SOCKET = -1;
