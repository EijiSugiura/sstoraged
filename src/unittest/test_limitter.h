#include <cxxtest/TestSuite.h>
#include <sstream>
#include <iostream>

using namespace std;

#include "limitter.h"

class LimitterTestSuite : public CxxTest::TestSuite 
{
 public:
	void testLinearLimitter()
	{
		size_t softlimit = 134217728/(COMMON_REAL_SIZE + sizeof(CommonBuf));
		size_t hardlimit = 268435456/(COMMON_REAL_SIZE + sizeof(CommonBuf));
		cout << "soft : hard = " << softlimit<< " : " << hardlimit << endl;
		LinearLimitter limitter(softlimit, hardlimit);
		TS_ASSERT_EQUALS(limitter.getY(1), iSCSISession::CMD_QUEUE_SIZE);
		TS_ASSERT_EQUALS(limitter.getY(softlimit), iSCSISession::CMD_QUEUE_SIZE);
// 		for(size_t size = softlimit+1; size < hardlimit; ++size){
// 			cout << size << " : " << limitter.getY(size) << endl;
// 		}
		TS_ASSERT_EQUALS(limitter.getY(softlimit+(hardlimit-softlimit)/2),
				 (iSCSISession::CMD_QUEUE_SIZE)/2+1);
		TS_ASSERT_EQUALS(limitter.getY(hardlimit), 1U);
		TS_ASSERT_EQUALS(limitter.getY(hardlimit+1), 1U);
		TS_ASSERT_EQUALS(limitter.getY(USHRT_MAX), 1U);
	}
};
