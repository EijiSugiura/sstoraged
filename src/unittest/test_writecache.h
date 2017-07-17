#include <cxxtest/TestSuite.h>
#include <sstream>
#include <iostream>

using namespace std;

#include "diskwriter.h"
#include "writecache.h"

vector<string> strings;
class WriteCacheTestSuite : public CxxTest::TestSuite 
{
 public:
	void setUp()
	{
	}
	void tearDown()
	{
		strings.clear();
	}
	/**
	   only one segment
	   [5:10)
	 */
	void testSearch01()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(5,5));
		wc.push(ws);
		vector<WriteSegmentPtr> wss = wc.search(0,5);
		TS_ASSERT_EQUALS(wss.size(), 0U);
		wss = wc.search(5,5);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(5,6);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(4,6);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(4,7);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(5,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(9,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(9,5);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(10,1);
		TS_ASSERT_EQUALS(wss.size(), 0U);
		wss = wc.search(10,5);
		TS_ASSERT_EQUALS(wss.size(), 0U);
	}
	/**
	   Sparse 2 segments
	   [5:10) [15:20)
	 */
	void testSearch02()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(5,5));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(15,5));
		wc.push(ws);

		vector<WriteSegmentPtr> wss = wc.search(0,5);
		TS_ASSERT_EQUALS(wss.size(), 0U);
		wss = wc.search(4,6);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(4,7);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(5,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(5,5);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(5,6);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(9,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(9,5);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(10,1);
		TS_ASSERT_EQUALS(wss.size(), 0U);
		wss = wc.search(10,5);
		TS_ASSERT_EQUALS(wss.size(), 0U);
		wss = wc.search(10,6);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(11,4);
		TS_ASSERT_EQUALS(wss.size(), 0U);
		wss = wc.search(11,5);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(14,1);
		TS_ASSERT_EQUALS(wss.size(), 0U);
		wss = wc.search(14,2);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(15,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(15,5);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(15,6);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(19,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(19,2);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(20,1);
		TS_ASSERT_EQUALS(wss.size(), 0U);
		wss = wc.search(20,5);
		TS_ASSERT_EQUALS(wss.size(), 0U);
	}
	/**
	   splitted segments

	   [2:6) [6:10)
              [5:6)
	   
	 */
	void testSearch03()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(5,5));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(2,4));
		wc.push(ws);

		vector<WriteSegmentPtr> wss = wc.search(0,2);
		TS_ASSERT_EQUALS(wss.size(), 0U);
		wss = wc.search(0,3);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(1,1);
		TS_ASSERT_EQUALS(wss.size(), 0U);
		wss = wc.search(1,2);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(2,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(2,3);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(2,4);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(2,5);
		TS_ASSERT_EQUALS(wss.size(), 2U);
		wss = wc.search(2,9);
		TS_ASSERT_EQUALS(wss.size(), 2U);
		wss = wc.search(4,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(4,2);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(4,3);
		TS_ASSERT_EQUALS(wss.size(), 2U);
		wss = wc.search(4,6);
		TS_ASSERT_EQUALS(wss.size(), 2U);
		wss = wc.search(4,7);
		TS_ASSERT_EQUALS(wss.size(), 2U);
		wss = wc.search(5,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(5,2);
		TS_ASSERT_EQUALS(wss.size(), 2U);
		wss = wc.search(5,5);
		TS_ASSERT_EQUALS(wss.size(), 2U);
		wss = wc.search(5,6);
		TS_ASSERT_EQUALS(wss.size(), 2U);
		wss = wc.search(6,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(6,4);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(6,5);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(9,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(9,2);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		wss = wc.search(10,1);
		TS_ASSERT_EQUALS(wss.size(), 0U);
		wss = wc.search(10,2);
		TS_ASSERT_EQUALS(wss.size(), 0U);
	}
	/**
	   same I/O segment
	   WriteSegment[3] = 
	   10+5
	   5+5
	   0+5
	   search(0,5)  = 1 segment
	   search(5,5)  = 1 segment
	   search(10,5) = 1 segment
	 */
	void testSameSegments01()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(0,5));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(5,5));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(10,5));
		wc.push(ws);

		vector<WriteSegmentPtr> wss = wc.search(0,5);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 0ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 5UL);
		}
		wss = wc.search(5,5);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 5ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 5UL);
		}
		wss = wc.search(10,5);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 10ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 5UL);
		}
	}
	/**
	   same I/O segment
	   WriteSegment[3] = 
	   10+1
	   5+1
	   0+1
	   search(0,1)  = 1 segment
	   search(5,1)  = 1 segment
	   search(10,1) = 1 segment
	 */
	void testSameSegments02()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(0,1));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(5,1));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(10,1));
		wc.push(ws);

		vector<WriteSegmentPtr> wss = wc.search(0,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 0ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 1UL);
		}
		wss = wc.search(5,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 5ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 1UL);
		}
		wss = wc.search(10,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 10ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 1UL);
		}
	}
	/**
	   Input segment <= Output segment
	   WriteSegment[3] = 
	   10+5
	   5+5
	   0+5
	   search(0,6)   = 2 segment
	   search(1,10)  = 3 segment
	   search(5,6)   = 2 segment
	   search(4,2)   = 2 segment
	   search(6,5)   = 2 segment
	 */
	void testOverwrapSegments01()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(0,5));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(5,5));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(10,5));
		wc.push(ws);

		vector<WriteSegmentPtr> wss = wc.search(0,6);
		TS_ASSERT_EQUALS(wss.size(), 2U);
 		wss = wc.search(1,10);
 		TS_ASSERT_EQUALS(wss.size(), 3U);
 		wss = wc.search(5,6);
 		TS_ASSERT_EQUALS(wss.size(), 2U);
 		wss = wc.search(4,2);
 		TS_ASSERT_EQUALS(wss.size(), 2U);
 		wss = wc.search(6,5);
 		TS_ASSERT_EQUALS(wss.size(), 2U);
	}
	/**
	   Input segment <= Output segment
	   WriteSegment[3] = 
	   10+1
	   5+1
	   0+1
	   search(0,6)   = 2 segment
	   search(1,10)  = 2 segment
	   search(5,6)   = 2 segment
	   search(4,2)   = 1 segment
	   search(6,5)   = 1 segment
	 */
	void testOverwrapSegments02()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(0,1));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(5,1));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(10,1));
		wc.push(ws);

		vector<WriteSegmentPtr> wss = wc.search(0,6);
		TS_ASSERT_EQUALS(wss.size(), 2U);
 		wss = wc.search(1,10);
 		TS_ASSERT_EQUALS(wss.size(), 2U);
 		wss = wc.search(5,6);
 		TS_ASSERT_EQUALS(wss.size(), 2U);
 		wss = wc.search(4,2);
 		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 5ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 1UL);
		}
 		wss = wc.search(6,5);
 		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 10ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 1UL);
		}
	}
	/**
	   Nested Input segment <= Output segment
	   WriteSegment[3] = 
	   10+1
	   5+1
	   0+11
	   search(0,6)   = 2 segment
	   search(1,10)  = 3 segment
	   search(5,6)   = 2 segment
	   search(4,2)   = 2 segment
	   search(6,5)   = 1 segment
	 */
	void testOverwrapSegments03()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(0,11));
		wc.push(ws);
//		cout << wc.dump();
		ws = WriteSegmentPtr(new WriteSegment(5,1));
		wc.push(ws);
//		cout << wc.dump();
		ws = WriteSegmentPtr(new WriteSegment(10,1));
		wc.push(ws);
//		cout << wc.dump();

		vector<WriteSegmentPtr> wss = wc.search(0,6);
		TS_ASSERT_EQUALS(wss.size(), 2U);
 		wss = wc.search(1,10);
 		TS_ASSERT_EQUALS(wss.size(), 4U);
 		wss = wc.search(5,6);
 		TS_ASSERT_EQUALS(wss.size(), 3U);
 		wss = wc.search(4,2);
 		TS_ASSERT_EQUALS(wss.size(), 2U);
 		wss = wc.search(6,5);
 		TS_ASSERT_EQUALS(wss.size(), 2U);
	}
	/**
	   Input segment > Output segment
	   WriteSegment[3] = 
	   10+5
	   5+5
	   0+5
	   search(0,1)   = 1 segment
	   search(1,1)   = 1 segment
	   search(4,1)   = 1 segment
	   search(5,1)   = 1 segment
	 */
	void testIncludeSegments01()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(0,5));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(5,5));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(10,5));
		wc.push(ws);

		vector<WriteSegmentPtr> wss = wc.search(0,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 0ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 5UL);
		}
 		wss = wc.search(1,1);
 		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 0ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 5UL);
		}
 		wss = wc.search(4,1);
 		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 0ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 5UL);
		}
 		wss = wc.search(5,1);
 		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 5ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 5UL);
		}
	}
	/**
	   Nested Input segment > Output segment
	   WriteSegment[3] = 
	   10+1
	   5+1
	   0+11
	   search(0,1)   = 1 segment
	   search(1,1)   = 1 segment
	   search(4,1)   = 1 segment
	   search(5,1)   = 1 segment
	   search(11,1)   = 1 segment
	 */
	void testIncludeSegments03()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(0,11));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(5,1));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(10,1));
		wc.push(ws);

		vector<WriteSegmentPtr> wss = wc.search(0,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 0ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 11UL);
		}
 		wss = wc.search(1,1);
 		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 0ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 11UL);
		}
 		wss = wc.search(4,1);
 		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 0ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 11UL);
		}
 		wss = wc.search(5,1);
 		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 5ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 1UL);
		}
 		wss = wc.search(10,1);
 		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 10ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 1UL);
		}
	}
	/**
	   Collision Input segment > Output segment
	   WriteSegment[3] = 
	   9+3
	   10+1
	   1+12
	   search(1,1)   = 1 segment
	   search(8,1)   = 1 segment
	   search(9,1)   = 1 segment
	   search(10,1)   = 1 segment
	   search(11,1)   = 1 segment
	   search(12,1)   = 1 segment
	 */
	void testIncludeSegments04()
	{
		WriteCache wc;
//		cout << wc.dump();
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(1,11));
		wc.push(ws);
//		cout << wc.dump();
		// merge segment
		ws = WriteSegmentPtr(new WriteSegment(10,1));
		wc.push(ws);
//		cout << wc.dump();
		// merge segment
		ws = WriteSegmentPtr(new WriteSegment(9,3));
		wc.push(ws);
//		cout << wc.dump();

		vector<WriteSegmentPtr> wss = wc.search(1,1);
		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 1ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 11UL);
		}
 		wss = wc.search(8,1);
 		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 1ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 11UL);
		}
 		wss = wc.search(9,1);
 		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 9ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 3UL);
		}
 		wss = wc.search(10,1);
 		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 9ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 3UL);
		}
		// copied leaf
 		wss = wc.search(11,1);
 		TS_ASSERT_EQUALS(wss.size(), 1U);
		for(vector<WriteSegmentPtr>::iterator itr = wss.begin();
		    itr != wss.end(); ++itr){
			TS_ASSERT_EQUALS((*itr)->lba, 9ULL);
			TS_ASSERT_EQUALS((*itr)->transfer_length, 3UL);
		}
	}
	/**
	   Input segment != Output segment
	   WriteSegment[3] = 
	   10+1
	   5+1
	   0+1
	   search(1,1)   = 0 segment
	   search(4,1)   = 0 segment
	   search(6,4)   = 0 segment
	   search(12,1)  = 0 segment
	   search(13,5)  = 0 segment
	 */
	void testEmptySegments01()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(0,1));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(5,1));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(10,1));
		wc.push(ws);

		vector<WriteSegmentPtr> wss = wc.search(1,1);
		TS_ASSERT_EQUALS(wss.size(), 0U);
 		wss = wc.search(4,1);
 		TS_ASSERT_EQUALS(wss.size(), 0U);
 		wss = wc.search(6,4);
 		TS_ASSERT_EQUALS(wss.size(), 0U);
 		wss = wc.search(12,1);
 		TS_ASSERT_EQUALS(wss.size(), 0U);
 		wss = wc.search(13,5);
 		TS_ASSERT_EQUALS(wss.size(), 0U);
	}
	/**
	   Nested Input segment != Output segment
	   WriteSegment[3] = 
	   10+1
	   5+1
	   1+11
	   search(0,1)   = 0 segment
	   search(12,1)   = 0 segment
	 */
	void testEmptySegments02()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(1,11));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(5,1));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(10,1));
		wc.push(ws);

		vector<WriteSegmentPtr> wss = wc.search(0,1);
		TS_ASSERT_EQUALS(wss.size(), 0U);
 		wss = wc.search(11,1);
 		TS_ASSERT_EQUALS(wss.size(), 0U);
	}
	/**
	   Collision Input segment != Output segment
	   WriteSegment[3] = 
	   10+1
	   10+2
	   1+11
	   search(0,1)   = 0 segment
	   search(12,1)   = 0 segment
	 */
	void testEmptySegments03()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(1,11));
		wc.push(ws);
//		cout << wc.dump();
		// merge segment
		ws = WriteSegmentPtr(new WriteSegment(10,1));
		wc.push(ws);
//		cout << wc.dump();
		// merge segment
		ws = WriteSegmentPtr(new WriteSegment(10,2));
		wc.push(ws);
//		cout << wc.dump();

		vector<WriteSegmentPtr> wss = wc.search(0,1);
		TS_ASSERT_EQUALS(wss.size(), 0U);
 		wss = wc.search(12,1);
 		TS_ASSERT_EQUALS(wss.size(), 0U);
	}

	/**
	   FIFO queue
	   WriteSegment[3] = 
	   10+1
	   5+1
	   1+11
	 */
	void testFIFO()
	{
		WriteCache wc;
		WriteSegmentPtr ws = WriteSegmentPtr(new WriteSegment(1,11));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(5,1));
		wc.push(ws);
		ws = WriteSegmentPtr(new WriteSegment(10,1));
		wc.push(ws);

//		cout << wc.dump();

		ws = wc.front();
		TS_ASSERT_EQUALS(ws->lba, 1ULL);
		TS_ASSERT_EQUALS(ws->transfer_length, 11UL);
		wc.pop();
		ws = wc.front();
		TS_ASSERT_EQUALS(ws->lba, 5ULL);
		TS_ASSERT_EQUALS(ws->transfer_length, 1UL);
		wc.pop();
		ws = wc.front();
		TS_ASSERT_EQUALS(ws->lba, 10ULL);
		TS_ASSERT_EQUALS(ws->transfer_length, 1UL);
		wc.pop();
	}
};

DiskWriter::DiskWriter(boost::shared_ptr<WriteCache> _wc){}
DiskWriter::~DiskWriter(){}
void DiskWriter::operator()() throw (runtime_error) {}
bool DiskWriter::doWork() { return true; }

const socket_t INVALID_SOCKET = -1;
