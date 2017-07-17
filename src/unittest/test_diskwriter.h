#include <cxxtest/TestSuite.h>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

#include "writecache.h"
#include "diskwriter.h"

vector<string> strings;
class DiskWriterTestSuite : public CxxTest::TestSuite 
{
	WriteCachePtr wc;
	DiskWriter *dw;
	uint8_t getByte01(const uint64_t offset)
	{
		uint8_t buf = 0xff;
		FileReadHandle fh("./test_diskwriter01.dat");
		pread64(fh.get(), reinterpret_cast<void*>(&buf), sizeof(buf), offset);
		return buf;
	}
	uint8_t getByte02(const uint64_t offset)
	{
		uint8_t buf = 0xff;
		FileReadHandle fh("./test_diskwriter02.dat");
		pread64(fh.get(), reinterpret_cast<void*>(&buf), sizeof(buf), offset);
		return buf;
	}
 public:
	void setUp()
	{
		char buf[10240] = "";
		memset(buf, 0x00, sizeof(buf));
		FileWriteHandle fh1("./test_diskwriter01.dat");
		write(fh1.get(), buf, sizeof(buf));
		FileWriteHandle fh2("./test_diskwriter02.dat");
		write(fh2.get(), buf, sizeof(buf));


		wc = WriteCachePtr(new WriteCache);
		dw = new DiskWriter(wc);
	}
	void tearDown()
	{
		WriteCachePtr null;
		wc = null;
		delete dw;
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();
	}
	/**
	   0. [0:10240) = 0x00
	   1. overwrite by lba[0+2) cbuf[0:1024) = 0x01
	 */
	void testSingleWrite01()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 0;
		size_t transfer_length = 2;
		uint64_t offset = SCSI::LBA2OFFSET(lba);
		size_t size = SCSI::LBA2OFFSET(transfer_length);

		LowHandle lh(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			     offset, size);
		TS_ASSERT_EQUALS(dw->addHandle(lh, 0ULL), true);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		cbuf->stepForwardTail(size);
		memset(cbuf->ref(), 0x01, size);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,size,offset), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

		void *ptr = cbuf->head();
		TS_ASSERT_EQUALS(strings[2], (string)"Write Segment : [3] offset 0 (0)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/1024");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[3], (string)"Write Segment : [3] offset 500 (0)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/524");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 1000 (1)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 24/24");
		TS_ASSERT_EQUALS(getByte01(0), 0x01);
		TS_ASSERT_EQUALS(getByte01(1023), 0x01);
		TS_ASSERT_EQUALS(getByte01(1024), 0x00);
	}
	/**
	   0. [0:10240) = 0x00
	   1. overwrite by lba[0+2) cbuf[100:1124) = 0x01
	 */
	void testSingleWrite02()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 0;
		size_t transfer_length = 2;
		size_t ds_offset = 100;
		uint64_t lh_offset = SCSI::LBA2OFFSET(lba);
		size_t size = SCSI::LBA2OFFSET(transfer_length);

		LowHandle lh(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			     lh_offset, size);
		TS_ASSERT_EQUALS(dw->addHandle(lh, lh_offset), true);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		cbuf->stepForwardTail(size+ds_offset);
		cbuf->stepForwardOffset(ds_offset-48);
		memset(cbuf->ref(), 0x01, size+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,size,ds_offset), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

		void *ptr = reinterpret_cast<uint8_t*>(cbuf->head()) + ds_offset;
		TS_ASSERT_EQUALS(strings[2], (string)"Write Segment : [3] offset 0 (0)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/1024");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[3], (string)"Write Segment : [3] offset 500 (0)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/524");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 1000 (1)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 24/24");
		TS_ASSERT_EQUALS(getByte01(0), 0x01);
		TS_ASSERT_EQUALS(getByte01(1023), 0x01);
		TS_ASSERT_EQUALS(getByte01(1024), 0x00);
	}
	/**
	   0. [0:10240) = 0x00
	   1. overwrite by lba[1+2) cbuf[100:1124) = 0x01
	 */
	void testSingleWrite03()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 1;
		size_t transfer_length = 2;
		size_t ds_size = SCSI::LBA2OFFSET(transfer_length);
		size_t ds_offset = 100;
		uint64_t lh_offset = 0;
		uint64_t lh_size = 10240;

		LowHandle lh(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			     lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh, 0UL), true);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		cbuf->stepForwardTail(ds_size+ds_offset);
		cbuf->stepForwardOffset(ds_offset-48);
		memset(cbuf->ref(), 0x01, ds_size+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,ds_size,ds_offset), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

		void *ptr = reinterpret_cast<uint8_t*>(cbuf->head()) + ds_offset;
		TS_ASSERT_EQUALS(strings[2], (string)"Write Segment : [3] offset 512 (1)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/1024");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[3], (string)"Write Segment : [3] offset 1012 (1)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/524");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 1512 (2)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 24/24");
		TS_ASSERT_EQUALS(getByte01(511), 0x00);
		TS_ASSERT_EQUALS(getByte01(512), 0x01);
		TS_ASSERT_EQUALS(getByte01(1535), 0x01);
		TS_ASSERT_EQUALS(getByte01(1536), 0x00);
	}
	/**
	   0. [0:10240) = 0x00 with LowHandle[512:(10240-512))
	   1. overwrite by lba[0+2) cbuf[0:1024) = 0x01
	 */
	void testSingleWrite04()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 0;
		size_t transfer_length = 2;
		size_t ds_size = SCSI::LBA2OFFSET(transfer_length);
		size_t ds_offset = 0;
		uint64_t lh_offset = 512;
		uint64_t lh_size = 10240-lh_offset;

		LowHandle lh(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			     lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh, 0UL), true);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		cbuf->stepForwardTail(ds_size+ds_offset);
		cbuf->stepForwardOffset(ds_offset-48);
		memset(cbuf->ref(), 0x01, ds_size+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,ds_size,ds_offset), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

		void *ptr = reinterpret_cast<uint8_t*>(cbuf->head()) + ds_offset;
		TS_ASSERT_EQUALS(strings[2], (string)"Write Segment : [3] offset 512 (1)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/1024");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[3], (string)"Write Segment : [3] offset 1012 (1)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/524");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 1512 (2)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 24/24");
		TS_ASSERT_EQUALS(getByte01(511), 0x00);
		TS_ASSERT_EQUALS(getByte01(512), 0x01);
		TS_ASSERT_EQUALS(getByte01(1535), 0x01);
		TS_ASSERT_EQUALS(getByte01(1536), 0x00);
	}
	/**
	   0. [0:10240) = 0x00 with LowHandle[512:(10240-512))
	   1. overwrite by lba[0+2) cbuf[100:1124) = 0x01
	 */
	void testSingleWrite05()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 0;
		size_t transfer_length = 2;
		size_t ds_size = SCSI::LBA2OFFSET(transfer_length);
		size_t ds_offset = 100;
		uint64_t lh_offset = 512;
		uint64_t lh_size = 10240-lh_offset;

		LowHandle lh(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			     lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh, 0UL), true);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		cbuf->stepForwardTail(ds_size+ds_offset);
		cbuf->stepForwardOffset(ds_offset-48);
		memset(cbuf->ref(), 0x01, ds_size+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,ds_size,ds_offset), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

		void *ptr = reinterpret_cast<uint8_t*>(cbuf->head()) + ds_offset;
		TS_ASSERT_EQUALS(strings[2], (string)"Write Segment : [3] offset 512 (1)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/1024");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[3], (string)"Write Segment : [3] offset 1012 (1)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/524");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 1512 (2)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 24/24");
		TS_ASSERT_EQUALS(getByte01(511), 0x00);
		TS_ASSERT_EQUALS(getByte01(512), 0x01);
		TS_ASSERT_EQUALS(getByte01(1535), 0x01);
		TS_ASSERT_EQUALS(getByte01(1536), 0x00);
	}
	/**
	   0. [0:10240) = 0x00 with LowHandle[512:(10240-512))
	   1. overwrite by lba[1+2) cbuf[100:1124) = 0x01
	 */
	void testSingleWrite06()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 1;
		size_t transfer_length = 2;
		size_t ds_size = SCSI::LBA2OFFSET(transfer_length);
		size_t ds_offset = 100;
		uint64_t lh_offset = 512;
		uint64_t lh_size = 10240-lh_offset;

		LowHandle lh(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			     lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh, 0UL), true);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		cbuf->stepForwardTail(ds_size+ds_offset);
		cbuf->stepForwardOffset(ds_offset-48);
		memset(cbuf->ref(), 0x01, ds_size+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,ds_size,ds_offset), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

		void *ptr = reinterpret_cast<uint8_t*>(cbuf->head()) + ds_offset;
		TS_ASSERT_EQUALS(strings[2], (string)"Write Segment : [3] offset 1024 (2)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/1024");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[3], (string)"Write Segment : [3] offset 1524 (2)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/524");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 2024 (3)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 24/24");
		TS_ASSERT_EQUALS(getByte01(1023), 0x00);
		TS_ASSERT_EQUALS(getByte01(1024), 0x01);
		TS_ASSERT_EQUALS(getByte01(2047), 0x01);
		TS_ASSERT_EQUALS(getByte01(2048), 0x00);
	}
	/**
	   0. [0:10240) = 0x00 with LowHandle[0:10240)
	   1. overwrite by lba[18+2) cbuf[0:1024) = 0x01
	 */
	void testSingleWrite07()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 18;
		size_t transfer_length = 2;
		size_t ds_size = SCSI::LBA2OFFSET(transfer_length);
		size_t ds_offset = 0;
		uint64_t lh_offset = 0;
		uint64_t lh_size = 10240-lh_offset;

		LowHandle lh(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			     lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh, 0UL), true);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		cbuf->stepForwardTail(ds_size+ds_offset);
		cbuf->stepForwardOffset(ds_offset-48);
		memset(cbuf->ref(), 0x01, ds_size+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,ds_size,ds_offset), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

		void *ptr = reinterpret_cast<uint8_t*>(cbuf->head()) + ds_offset;
		TS_ASSERT_EQUALS(strings[2], (string)"Write Segment : [3] offset 9216 (18)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/1024");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[3], (string)"Write Segment : [3] offset 9716 (18)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/524");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 10216 (19)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 24/24");
		TS_ASSERT_EQUALS(getByte01(9215), 0x00);
		TS_ASSERT_EQUALS(getByte01(9216), 0x01);
		TS_ASSERT_EQUALS(getByte01(10239), 0x01);
	}
	/**
	   0. [0:10240) = 0x00 with LowHandle[0:10240)
	   1. overwrite by lba[18+2) cbuf[100:1124) = 0x01
	 */
	void testSingleWrite08()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 18;
		size_t transfer_length = 2;
		size_t ds_size = SCSI::LBA2OFFSET(transfer_length);
		size_t ds_offset = 100;
		uint64_t lh_offset = 0;
		uint64_t lh_size = 10240-lh_offset;

		LowHandle lh(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			     lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh, 0UL), true);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		cbuf->stepForwardTail(ds_size+ds_offset);
		cbuf->stepForwardOffset(ds_offset-48);
		memset(cbuf->ref(), 0x01, ds_size+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,ds_size,ds_offset), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

		void *ptr = reinterpret_cast<uint8_t*>(cbuf->head()) + ds_offset;
		TS_ASSERT_EQUALS(strings[2], (string)"Write Segment : [3] offset 9216 (18)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/1024");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[3], (string)"Write Segment : [3] offset 9716 (18)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/524");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 10216 (19)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 24/24");
		TS_ASSERT_EQUALS(getByte01(9215), 0x00);
		TS_ASSERT_EQUALS(getByte01(9216), 0x01);
		TS_ASSERT_EQUALS(getByte01(10239), 0x01);
	}
	/**
	   0. [0:10240) = 0x00 with LowHandle[512:(10240-512))
	   1. overwrite by lba[17+2) cbuf[100:1124) = 0x01
	 */
	void testSingleWrite09()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 17;
		size_t transfer_length = 2;
		size_t ds_size = SCSI::LBA2OFFSET(transfer_length);
		size_t ds_offset = 100;
		uint64_t lh_offset = 512;
		uint64_t lh_size = 10240-lh_offset;

		LowHandle lh(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			     lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh, 0UL), true);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		cbuf->stepForwardTail(ds_size+ds_offset);
		cbuf->stepForwardOffset(ds_offset-48);
		memset(cbuf->ref(), 0x01, ds_size+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,ds_size,ds_offset), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

		void *ptr = reinterpret_cast<uint8_t*>(cbuf->head()) + ds_offset;
		TS_ASSERT_EQUALS(strings[2], (string)"Write Segment : [3] offset 9216 (18)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/1024");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[3], (string)"Write Segment : [3] offset 9716 (18)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 500/524");
		ptr = (uint8_t*)ptr + 500;
		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 10216 (19)  " +
				 boost::lexical_cast<string>(ptr) +
				 "  -> 24/24");
		TS_ASSERT_EQUALS(getByte01(9215), 0x00);
		TS_ASSERT_EQUALS(getByte01(9216), 0x01);
		TS_ASSERT_EQUALS(getByte01(10239), 0x01);
	}
	/**
	   0.1.[0:10240) = 0x00 with LowHandle[0:10240)
	   0.2.[20:10240) = 0x00 with LowHandle[0:10240)

	   1. overwrite by lba[19+2) cbuf[0:1024) = 0x01
	 */
	void testDoubleSegmentWrite01()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 19;
		size_t transfer_length = 2;
		size_t ds_size = SCSI::LBA2OFFSET(transfer_length);
		size_t ds_offset = 0;
		uint64_t lh_offset = 0;
		uint64_t lh_size = 10240-lh_offset;

		LowHandle lh1(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			      lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh1, 0UL), true);
		LowHandle lh2(FileHandlePtr(new FileWriteHandle("./test_diskwriter02.dat")),
			      0, 10240);
		TS_ASSERT_EQUALS(dw->addHandle(lh2, SCSI::LBA2OFFSET(20)), true);

		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		cbuf->stepForwardTail(ds_size+ds_offset);
		cbuf->stepForwardOffset(ds_offset-48);
		memset(cbuf->ref(), 0x01, ds_size+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,ds_size,ds_offset), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

		void *ptr = reinterpret_cast<uint8_t*>(cbuf->head()) + ds_offset;
 		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 9728 (19)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 500/512");
 		ptr = (uint8_t*)ptr + 500;
 		TS_ASSERT_EQUALS(strings[5], (string)"Write Segment : [3] offset 10228 (19)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 12/12");
 		ptr = (uint8_t*)ptr + 12;
 		TS_ASSERT_EQUALS(strings[6], (string)"Write Segment : [4] offset 0 (0)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 500/512");
 		ptr = (uint8_t*)ptr + 500;
 		TS_ASSERT_EQUALS(strings[7], (string)"Write Segment : [4] offset 500 (0)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 12/12");
 		TS_ASSERT_EQUALS(getByte01(9727), 0x00);
 		TS_ASSERT_EQUALS(getByte01(9728), 0x01);
 		TS_ASSERT_EQUALS(getByte01(10239), 0x01);
 		TS_ASSERT_EQUALS(getByte02(0), 0x01);
 		TS_ASSERT_EQUALS(getByte02(511), 0x01);
 		TS_ASSERT_EQUALS(getByte02(512), 0x00);
	}
	/**
	   0.1.[0:10240) = 0x00 with LowHandle[0:10240)
	   0.2.[20:10240) = 0x00 with LowHandle[0:10240)

	   1. overwrite by lba[19+2) cbuf[100:1124) = 0x01
	 */
	void testDoubleSegmentWrite02()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 19;
		size_t transfer_length = 2;
		size_t ds_size = SCSI::LBA2OFFSET(transfer_length);
		size_t ds_offset = 100;
		uint64_t lh_offset = 0;
		uint64_t lh_size = 10240-lh_offset;

		LowHandle lh1(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			      lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh1, 0UL), true);
		LowHandle lh2(FileHandlePtr(new FileWriteHandle("./test_diskwriter02.dat")),
			      0, 10240);
		TS_ASSERT_EQUALS(dw->addHandle(lh2, SCSI::LBA2OFFSET(20)), true);

		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		cbuf->stepForwardTail(ds_size+ds_offset);
		cbuf->stepForwardOffset(ds_offset-48);
		memset(cbuf->ref(), 0x01, ds_size+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,ds_size,ds_offset), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

		void *ptr = reinterpret_cast<uint8_t*>(cbuf->head()) + ds_offset;
 		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 9728 (19)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 500/512");
 		ptr = (uint8_t*)ptr + 500;
 		TS_ASSERT_EQUALS(strings[5], (string)"Write Segment : [3] offset 10228 (19)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 12/12");
 		ptr = (uint8_t*)ptr + 12;
 		TS_ASSERT_EQUALS(strings[6], (string)"Write Segment : [4] offset 0 (0)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 500/512");
 		ptr = (uint8_t*)ptr + 500;
 		TS_ASSERT_EQUALS(strings[7], (string)"Write Segment : [4] offset 500 (0)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 12/12");
 		TS_ASSERT_EQUALS(getByte01(9727), 0x00);
 		TS_ASSERT_EQUALS(getByte01(9728), 0x01);
 		TS_ASSERT_EQUALS(getByte01(10239), 0x01);
 		TS_ASSERT_EQUALS(getByte02(0), 0x01);
 		TS_ASSERT_EQUALS(getByte02(511), 0x01);
 		TS_ASSERT_EQUALS(getByte02(512), 0x00);
	}
	/**
	   0.1.[0:10240) = 0x00 with LowHandle[512:(10240-512))
	   0.2.[20:10240) = 0x00 with LowHandle[512:(10240-512))

	   1. overwrite by lba[18+2) cbuf[100:1124) = 0x01
	 */
	void testDoubleSegmentWrite03()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 18;
		size_t transfer_length = 2;
		size_t ds_size = SCSI::LBA2OFFSET(transfer_length);
		size_t ds_offset = 100;
		uint64_t lh_offset = 512;
		uint64_t lh_size = 10240-lh_offset;

		LowHandle lh1(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			      lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh1, 0UL), true);
		LowHandle lh2(FileHandlePtr(new FileWriteHandle("./test_diskwriter02.dat")),
			      lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh2, SCSI::LBA2OFFSET(19)), true);

		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf = CommonBufPtr(new CommonBuf);
		cbuf->stepForwardTail(ds_size+ds_offset);
		cbuf->stepForwardOffset(ds_offset-48);
		memset(cbuf->ref(), 0x01, ds_size+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf,ds_size,ds_offset), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

		void *ptr = reinterpret_cast<uint8_t*>(cbuf->head()) + ds_offset;
 		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 9728 (19)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 500/512");
 		ptr = (uint8_t*)ptr + 500;
 		TS_ASSERT_EQUALS(strings[5], (string)"Write Segment : [3] offset 10228 (19)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 12/12");
 		ptr = (uint8_t*)ptr + 12;
 		TS_ASSERT_EQUALS(strings[6], (string)"Write Segment : [4] offset 512 (1)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 500/512");
 		ptr = (uint8_t*)ptr + 500;
 		TS_ASSERT_EQUALS(strings[7], (string)"Write Segment : [4] offset 1012 (1)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 12/12");
 		TS_ASSERT_EQUALS(getByte01(9727), 0x00);
 		TS_ASSERT_EQUALS(getByte01(9728), 0x01);
 		TS_ASSERT_EQUALS(getByte01(10239), 0x01);
 		TS_ASSERT_EQUALS(getByte02(0), 0x00);
 		TS_ASSERT_EQUALS(getByte02(511), 0x00);
 		TS_ASSERT_EQUALS(getByte02(512), 0x01);
 		TS_ASSERT_EQUALS(getByte02(1023), 0x01);
 		TS_ASSERT_EQUALS(getByte02(1024), 0x00);
	}
	/**
	   0. [0:10240) = 0x00 with LowHandle[0:10240)
	   1. overwrite by lba[0+2) cbuf1[size-512:512) = 0x01 & cbuf2[0:512) = 0x02
	 */
	void testDoubleCbufWrite01()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 0;
		size_t transfer_length = 2;
		size_t ds_size = SCSI::LBA2OFFSET(transfer_length);
		size_t ds_offset = COMMON_BUF_SIZE - ds_size/2;
		uint64_t lh_offset = 0;
		uint64_t lh_size = 10240-lh_offset;

		LowHandle lh(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			     lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh, 0UL), true);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf1 = CommonBufPtr(new CommonBuf);
		cbuf1->stepForwardTail(cbuf1->getSize());
		cbuf1->stepForwardOffset(cbuf1->getSize() - ds_size/2 - 48);
		memset(cbuf1->ref(), 0x01, ds_size/2+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf1,ds_size/2,ds_offset), true);
		CommonBufPtr cbuf2 = CommonBufPtr(new CommonBuf);
		cbuf2->stepForwardTail(ds_size/2);
		memset(cbuf2->head(), 0x02, ds_size/2);
		TS_ASSERT_EQUALS(ws->addDataSegment(cbuf2,ds_size/2), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

 		void *ptr = reinterpret_cast<uint8_t*>(cbuf1->head()) + ds_offset;
 		TS_ASSERT_EQUALS(strings[2], (string)"Write Segment : [3] offset 0 (0)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 500/512");
 		ptr = (uint8_t*)ptr + 500;
 		TS_ASSERT_EQUALS(strings[3], (string)"Write Segment : [3] offset 500 (0)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 12/12");
 		ptr = reinterpret_cast<uint8_t*>(cbuf2->head());
 		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 512 (1)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 500/512");
 		ptr = (uint8_t*)ptr + 500;
 		TS_ASSERT_EQUALS(strings[5], (string)"Write Segment : [3] offset 1012 (1)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 12/12");
 		TS_ASSERT_EQUALS(getByte01(0), 0x01);
 		TS_ASSERT_EQUALS(getByte01(511), 0x01);
 		TS_ASSERT_EQUALS(getByte01(512), 0x02);
 		TS_ASSERT_EQUALS(getByte01(1023), 0x02);
 		TS_ASSERT_EQUALS(getByte01(1024), 0x00);
	}
	/**
	   0. [0:10240) = 0x00 with LowHandle[512:(10240-512))
	   1. overwrite by lba[0+2) cbuf1[size-512:512) = 0x01 & cbuf2[0:512) = 0x02
	 */
	void testDoubleCbufWrite02()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 0;
		size_t transfer_length = 2;
		size_t ds_size = SCSI::LBA2OFFSET(transfer_length);
		size_t ds_offset = COMMON_BUF_SIZE - ds_size/2;
		uint64_t lh_offset = 512;
		uint64_t lh_size = 10240-lh_offset;

		LowHandle lh(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			     lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh, 0UL), true);
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf1 = CommonBufPtr(new CommonBuf);
		cbuf1->stepForwardTail(cbuf1->getSize());
		cbuf1->stepForwardOffset(cbuf1->getSize() - ds_size/2 - 48);
		memset(cbuf1->ref(), 0x01, ds_size/2+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf1,ds_size/2,ds_offset), true);
		CommonBufPtr cbuf2 = CommonBufPtr(new CommonBuf);
		cbuf2->stepForwardTail(ds_size/2);
		memset(cbuf2->head(), 0x02, ds_size/2);
		TS_ASSERT_EQUALS(ws->addDataSegment(cbuf2,ds_size/2), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

 		void *ptr = reinterpret_cast<uint8_t*>(cbuf1->head()) + ds_offset;
 		TS_ASSERT_EQUALS(strings[2], (string)"Write Segment : [3] offset 512 (1)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 500/512");
 		ptr = (uint8_t*)ptr + 500;
 		TS_ASSERT_EQUALS(strings[3], (string)"Write Segment : [3] offset 1012 (1)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 12/12");
 		ptr = reinterpret_cast<uint8_t*>(cbuf2->head());
 		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 1024 (2)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 500/512");
 		ptr = (uint8_t*)ptr + 500;
 		TS_ASSERT_EQUALS(strings[5], (string)"Write Segment : [3] offset 1524 (2)  " +
 				 boost::lexical_cast<string>(ptr) +
 				 "  -> 12/12");
 		TS_ASSERT_EQUALS(getByte01(0), 0x00);
 		TS_ASSERT_EQUALS(getByte01(511), 0x00);
 		TS_ASSERT_EQUALS(getByte01(512), 0x01);
 		TS_ASSERT_EQUALS(getByte01(1023), 0x01);
 		TS_ASSERT_EQUALS(getByte01(1024), 0x02);
 		TS_ASSERT_EQUALS(getByte01(1535), 0x02);
 		TS_ASSERT_EQUALS(getByte01(1536), 0x00);
	}

	/**
	   0.1.[0:10240) = 0x00 with LowHandle[0:10240)
	   0.2.[20:10240) = 0x00 with LowHandle[0:10240)
	   1. overwrite by lba[19+2) cbuf1[size-512:512) = 0x01 & cbuf2[0:512) = 0x02
	 */
	void testDoubleSegmentCbufWrite01()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 19;
		size_t transfer_length = 2;
		size_t ds_size = SCSI::LBA2OFFSET(transfer_length);
		size_t ds_offset = COMMON_BUF_SIZE - ds_size/2;
		uint64_t lh_offset = 0;
		uint64_t lh_size = 10240-lh_offset;

		LowHandle lh(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			     lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh, 0UL), true);
		LowHandle lh2(FileHandlePtr(new FileWriteHandle("./test_diskwriter02.dat")),
			      lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh2, SCSI::LBA2OFFSET(20)), true);

		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf1 = CommonBufPtr(new CommonBuf);
		cbuf1->stepForwardTail(cbuf1->getSize());
		cbuf1->stepForwardOffset(cbuf1->getSize() - ds_size/2 - 48);
		memset(cbuf1->ref(), 0x01, ds_size/2+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf1,ds_size/2,ds_offset), true);
		CommonBufPtr cbuf2 = CommonBufPtr(new CommonBuf);
		cbuf2->stepForwardTail(ds_size/2);
		memset(cbuf2->head(), 0x02, ds_size/2);
		TS_ASSERT_EQUALS(ws->addDataSegment(cbuf2,ds_size/2), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

  		void *ptr = reinterpret_cast<uint8_t*>(cbuf1->head()) + ds_offset;
  		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 9728 (19)  " +
  				 boost::lexical_cast<string>(ptr) +
  				 "  -> 500/512");
  		ptr = (uint8_t*)ptr + 500;
  		TS_ASSERT_EQUALS(strings[5], (string)"Write Segment : [3] offset 10228 (19)  " +
  				 boost::lexical_cast<string>(ptr) +
  				 "  -> 12/12");
  		ptr = reinterpret_cast<uint8_t*>(cbuf2->head());
  		TS_ASSERT_EQUALS(strings[6], (string)"Write Segment : [4] offset 0 (0)  " +
  				 boost::lexical_cast<string>(ptr) +
  				 "  -> 500/512");
  		ptr = (uint8_t*)ptr + 500;
  		TS_ASSERT_EQUALS(strings[7], (string)"Write Segment : [4] offset 500 (0)  " +
  				 boost::lexical_cast<string>(ptr) +
  				 "  -> 12/12");
  		TS_ASSERT_EQUALS(getByte01(9727), 0x00);
  		TS_ASSERT_EQUALS(getByte01(9728), 0x01);
  		TS_ASSERT_EQUALS(getByte01(10239), 0x01);
  		TS_ASSERT_EQUALS(getByte02(0), 0x02);
  		TS_ASSERT_EQUALS(getByte02(511), 0x02);
  		TS_ASSERT_EQUALS(getByte02(512), 0x00);
	}
	/**
	   0.1.[0:10240) = 0x00 with LowHandle[512:(10240-512))
	   0.2.[20:10240) = 0x00 with LowHandle[512:(10240-512))
	   1. overwrite by lba[18+2) cbuf1[size-512:512) = 0x01 & cbuf2[0:512) = 0x02
	 */
	void testDoubleSegmentCbufWrite02()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 18;
		size_t transfer_length = 2;
		size_t ds_size = SCSI::LBA2OFFSET(transfer_length);
		size_t ds_offset = COMMON_BUF_SIZE - ds_size/2;
		uint64_t lh_offset = 512;
		uint64_t lh_size = 10240-lh_offset;

		LowHandle lh(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			     lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh, 0UL), true);
		LowHandle lh2(FileHandlePtr(new FileWriteHandle("./test_diskwriter02.dat")),
			      lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh2, SCSI::LBA2OFFSET(19)), true);

		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf1 = CommonBufPtr(new CommonBuf);
		cbuf1->stepForwardTail(cbuf1->getSize());
		cbuf1->stepForwardOffset(cbuf1->getSize() - ds_size/2 - 48);
		memset(cbuf1->ref(), 0x01, ds_size/2+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf1,ds_size/2,ds_offset), true);
		CommonBufPtr cbuf2 = CommonBufPtr(new CommonBuf);
		cbuf2->stepForwardTail(ds_size/2);
		memset(cbuf2->head(), 0x02, ds_size/2);
		TS_ASSERT_EQUALS(ws->addDataSegment(cbuf2,ds_size/2), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

  		void *ptr = reinterpret_cast<uint8_t*>(cbuf1->head()) + ds_offset;
  		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 9728 (19)  " +
  				 boost::lexical_cast<string>(ptr) +
  				 "  -> 500/512");
  		ptr = (uint8_t*)ptr + 500;
  		TS_ASSERT_EQUALS(strings[5], (string)"Write Segment : [3] offset 10228 (19)  " +
  				 boost::lexical_cast<string>(ptr) +
  				 "  -> 12/12");
  		ptr = reinterpret_cast<uint8_t*>(cbuf2->head());
  		TS_ASSERT_EQUALS(strings[6], (string)"Write Segment : [4] offset 512 (1)  " +
  				 boost::lexical_cast<string>(ptr) +
  				 "  -> 500/512");
  		ptr = (uint8_t*)ptr + 500;
  		TS_ASSERT_EQUALS(strings[7], (string)"Write Segment : [4] offset 1012 (1)  " +
  				 boost::lexical_cast<string>(ptr) +
  				 "  -> 12/12");
  		TS_ASSERT_EQUALS(getByte01(9727), 0x00);
  		TS_ASSERT_EQUALS(getByte01(9728), 0x01);
  		TS_ASSERT_EQUALS(getByte01(10239), 0x01);
  		TS_ASSERT_EQUALS(getByte02(0), 0x00);
  		TS_ASSERT_EQUALS(getByte02(511), 0x00);
  		TS_ASSERT_EQUALS(getByte02(512), 0x02);
  		TS_ASSERT_EQUALS(getByte02(1023), 0x02);
  		TS_ASSERT_EQUALS(getByte02(1024), 0x00);
	}
	/**
	   0.1.[0:10240) = 0x00 with LowHandle[512:(10240-512))
	   0.2.[20:10240) = 0x00 with LowHandle[512:(10240-512))
	   1. overwrite by lba[18+2) cbuf1[size-512:512) = 0x01 & cbuf2[0:512) = 0x02
	 */
	void testDoubleSegmentCbufWrite03()
	{
		copy(strings.begin(),strings.end(), ostream_iterator<string>(cout, "\n"));
		strings.clear();

		uint64_t lba = 18;
		size_t transfer_length = 2;
		size_t ds_size1 = SCSI::LBA2OFFSET(transfer_length)/2 - 100;
		size_t ds_size2 = SCSI::LBA2OFFSET(transfer_length)/2 + 100;
		size_t ds_offset = COMMON_BUF_SIZE - ds_size1;
		uint64_t lh_offset = 512;
		uint64_t lh_size = 10240-lh_offset;

		LowHandle lh(FileHandlePtr(new FileWriteHandle("./test_diskwriter01.dat")),
			     lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh, 0UL), true);
		LowHandle lh2(FileHandlePtr(new FileWriteHandle("./test_diskwriter02.dat")),
			      lh_offset, lh_size);
		TS_ASSERT_EQUALS(dw->addHandle(lh2, SCSI::LBA2OFFSET(19)), true);

		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(lba,transfer_length));
		CommonBufPtr cbuf1 = CommonBufPtr(new CommonBuf);
		cbuf1->stepForwardTail(cbuf1->getSize());
		cbuf1->stepForwardOffset(cbuf1->getSize() - ds_size1 - 48);
		memset(cbuf1->ref(), 0x01, ds_size1+48);
		TS_ASSERT_EQUALS(ws->set1stDataSegment(cbuf1,ds_size1,ds_offset), true);
		CommonBufPtr cbuf2 = CommonBufPtr(new CommonBuf);
		cbuf2->stepForwardTail(ds_size2);
		memset(cbuf2->head(), 0x02, ds_size2);
		TS_ASSERT_EQUALS(ws->addDataSegment(cbuf2,ds_size2), true);
		wc->push(ws);
		TS_ASSERT_EQUALS(dw->doWork(), true);

  		void *ptr = reinterpret_cast<uint8_t*>(cbuf1->head()) + ds_offset;
  		TS_ASSERT_EQUALS(strings[4], (string)"Write Segment : [3] offset 9728 (19)  " +
  				 boost::lexical_cast<string>(ptr) +
  				 "  -> 412/412");
  		ptr = reinterpret_cast<uint8_t*>(cbuf2->head());
  		TS_ASSERT_EQUALS(strings[5], (string)"Write Segment : [3] offset 10140 (19)  " +
  				 boost::lexical_cast<string>(ptr) +
  				 "  -> 100/100");
  		ptr = (uint8_t*)ptr + 100;
  		TS_ASSERT_EQUALS(strings[6], (string)"Write Segment : [4] offset 512 (1)  " +
  				 boost::lexical_cast<string>(ptr) +
  				 "  -> 500/512");
  		ptr = (uint8_t*)ptr + 500;
  		TS_ASSERT_EQUALS(strings[7], (string)"Write Segment : [4] offset 1012 (1)  " +
  				 boost::lexical_cast<string>(ptr) +
  				 "  -> 12/12");
  		TS_ASSERT_EQUALS(getByte01(9727), 0x00);
  		TS_ASSERT_EQUALS(getByte01(9728), 0x01);
  		TS_ASSERT_EQUALS(getByte01(10139), 0x01);
  		TS_ASSERT_EQUALS(getByte01(10140), 0x02);
  		TS_ASSERT_EQUALS(getByte01(10239), 0x02);
  		TS_ASSERT_EQUALS(getByte02(0), 0x00);
  		TS_ASSERT_EQUALS(getByte02(511), 0x00);
  		TS_ASSERT_EQUALS(getByte02(512), 0x02);
  		TS_ASSERT_EQUALS(getByte02(1023), 0x02);
  		TS_ASSERT_EQUALS(getByte02(1024), 0x00);
	}
};

void* CommonBuf::operator new(size_t size)
{
	return ::operator new(size);
}

void CommonBuf::operator delete(void *obj, size_t size)
{
	return ::operator delete(obj);
}

bool CommonBuf::releasePool()
{
	return true;
}

pool_state_t CommonBuf::poolstate = POOL_STATE_NORMAL;

pool_state_t CommonBuf::getPoolState()
{ return poolstate; }
