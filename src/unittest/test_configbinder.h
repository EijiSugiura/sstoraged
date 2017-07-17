#include <cxxtest/TestSuite.h>

using namespace std;

#include "configurator.h"
#include "configbinder.h"

class ConfigBinderTestSuite : public CxxTest::TestSuite 
{
 public:
	void setUp()	{}
	void tearDown()
	{
 		Configurator &config = Configurator::getInstance();
 		config.clear();
	}

	/** load */
 	void testLoad_Fail_NoSuchFile() {
		ConfigBinder *binder = new ConfigBinder();
		TS_ASSERT_THROWS_EQUALS(binder->load(""), const std::exception &e,
					string(e.what()), "Failed to open : ");
		delete binder;
 	}
 	void testLoad_Fail_InvalidFormat() {
		ConfigBinder *binder = new ConfigBinder();
		TS_ASSERT_THROWS_EQUALS(binder->load("testConfigParseNG.conf"), const std::exception &e,
					string(e.what()), "Failed to parse configuration : * Line 2, Column 2\n  Missing \'}\' or object member name\n");
		delete binder;
 	}
 	void testLoad_Success() {
		ConfigBinder *binder = new ConfigBinder();
		bool ret = false;
		TS_ASSERT_THROWS_NOTHING(ret = binder->load("testConfigParseOK.conf"));
		TS_ASSERT_EQUALS(ret, true);

		delete binder;
 	}

	/** append */
 	void testAppend_Fail_NoSuchFile() {
		ConfigBinder *binder = new ConfigBinder();
		TS_ASSERT_THROWS_EQUALS(binder->append(""), const std::exception &e,
					string(e.what()), "Failed to open : ");
		delete binder;
 	}
 	void testAppend_Fail_InvalidFormat() {
		ConfigBinder *binder = new ConfigBinder();
		TS_ASSERT_THROWS_EQUALS(binder->append("testConfigParseNG.conf"), const std::exception &e,
					string(e.what()), "Failed to parse configuration : * Line 2, Column 2\n  Missing \'}\' or object member name\n");
		delete binder;
 	}
 	void testAppend_Success() {
		ConfigBinder *binder = new ConfigBinder();
		bool ret = false;
		TS_ASSERT_THROWS_NOTHING(ret = binder->append("testConfigParseOK.conf"));
		TS_ASSERT_EQUALS(ret, true);
		/** @todo: check attributes */
		delete binder;
 	}
 	void testAppend_Success_Twice() {
		ConfigBinder *binder = new ConfigBinder();
		bool ret = false;
		TS_ASSERT_THROWS_NOTHING(ret = binder->append("testConfigParseOK.conf"));
		TS_ASSERT_EQUALS(ret, true);
		TS_ASSERT_THROWS_NOTHING(ret = binder->append("testConfigAppendOK.conf"));
		TS_ASSERT_EQUALS(ret, true);
		/** @todo: check attributes */
		delete binder;
 	}
	/** dump */
	void testDump_Fail_NoSuchFile() {
		ConfigBinder *binder = new ConfigBinder();
		TS_ASSERT_THROWS_EQUALS(binder->dump(""), const std::exception &e,
					string(e.what()), "Failed to open : ");
		delete binder;
	}
	void testDump_Success_Load() {
		ConfigBinder *binder = new ConfigBinder();
		binder->load("testConfigParseOK.conf");
		bool ret = false;
		TS_ASSERT_THROWS_NOTHING(ret = binder->dump("testConfigDump.conf"));
		TS_ASSERT_EQUALS(ret, true);
		delete binder;
	}
	void testDump_Success_Append() {
		ConfigBinder *binder = new ConfigBinder();
		binder->append("testConfigParseOK.conf");
		bool ret = false;
		TS_ASSERT_THROWS_NOTHING(ret = binder->dump("testConfigDumpAppend.conf"));
		TS_ASSERT_EQUALS(ret, true);
		delete binder;
	}
	void testDump_Success_Twice() {
		ConfigBinder *binder = new ConfigBinder();
		binder->append("testConfigParseOK.conf");
		binder->append("testConfigAppendOK.conf");
		bool ret = false;
		TS_ASSERT_THROWS_NOTHING(ret = binder->dump("testConfigDumpTwice.conf"));
		TS_ASSERT_EQUALS(ret, true);
		delete binder;
	}
	/** clear */


	/** validate */
 	void testValidate_Success() {
		ConfigBinder *binder = new ConfigBinder();
		bool ret = false;
		TS_ASSERT_THROWS_NOTHING(ret = binder->load("testConfigValidateOK.conf"));
		TS_ASSERT_EQUALS(ret, true);
		TS_ASSERT_EQUALS(binder->validate(), true);
		delete binder;
 	}
 	void testValidate_NG() {
		ConfigBinder *binder = new ConfigBinder();
		bool ret = false;
		TS_ASSERT_THROWS_NOTHING(ret = binder->load("testConfigValidateNG.conf"));
		TS_ASSERT_EQUALS(ret, true);
		TS_ASSERT_THROWS_EQUALS(ret = binder->validate(), const std::exception &e,
					string(e.what()), "GlobalTargetName is invalid\nLocalTargetName : Type mismatch? boost::bad_get: failed value get using boost::get");
// 		TS_ASSERT_EQUALS(ret, false);
		delete binder;
 	}
};
