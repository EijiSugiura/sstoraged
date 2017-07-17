#include <cxxtest/TestSuite.h>

using namespace std;

#include "configurator.h"

class ConfiguratorTestSuite : public CxxTest::TestSuite 
{
 public:
	void setUp()	{}
	void tearDown() {
 		Configurator &config = Configurator::getInstance();
 		config.clear();
	}

	/** getter/setter */
 	void testBool_Success() {
 		Configurator &config = Configurator::getInstance();
 		TS_ASSERT_EQUALS(config.setAttr("InitialR2T", true), true);
 		bool value;
 		TS_ASSERT_EQUALS(config.getAttr("InitialR2T", value), true);
 		TS_ASSERT_EQUALS(value, true);
 	}
 	void testBool_Default() {
 		Configurator &config = Configurator::getInstance();
 		bool value;
 		TS_ASSERT_EQUALS(config.getAttr("InitialR2T", value), true);
 		TS_ASSERT_EQUALS(value, false);
 	}
//  	void testBool_Optional() {
//  		Configurator &config = Configurator::getInstance();
//  		bool value;
//  		TS_ASSERT_EQUALS(config.getAttr("OptionalBool", value), false);
//  		TS_ASSERT_EQUALS(config.setAttr("OptionalBool", true), true);
//  		TS_ASSERT_EQUALS(config.getAttr("OptionalBool", value), true);
//  		TS_ASSERT_EQUALS(value, true);
//  	}
 	void testBool_InvalidAttr() {
 		Configurator &config = Configurator::getInstance();
 		TS_ASSERT_EQUALS(config.setAttr("IllegalBool", true), false);
 	}
	void testNumber_Success() {
		Configurator &config = Configurator::getInstance();
		TS_ASSERT_EQUALS(config.setAttr("MaxConnections", (uint64_t)1), true);
		uint64_t value = 0;
		TS_ASSERT_EQUALS(config.getAttr("MaxConnections", value), true);
		TS_ASSERT_EQUALS(value, (uint64_t)1);
	}
	void testNumber_Default() {
		Configurator &config = Configurator::getInstance();
		uint64_t value = 0;
		TS_ASSERT_EQUALS(config.getAttr("MaxConnections", value), true);
		TS_ASSERT_EQUALS(value, (uint64_t)1);
	}
// 	void testNumber_Optional() {
// 		Configurator &config = Configurator::getInstance();
// 		uint64_t value = 0;
// 		TS_ASSERT_EQUALS(config.getAttr("OptionalNumber", value), false);
// 		value = 1;
// 		TS_ASSERT_EQUALS(config.setAttr("OptionalNumber", value), true);
// 		TS_ASSERT_EQUALS(config.getAttr("OptionalNumber", value), true);
// 		TS_ASSERT_EQUALS(value, (uint64_t)1);
// 	}
	void testNumber_InvalidAttr() {
		Configurator &config = Configurator::getInstance();
		TS_ASSERT_EQUALS(config.setAttr("IllegalNumber", (uint64_t)1), false);
	}
 	void testString_Success() {
 		Configurator &config = Configurator::getInstance();
 		TS_ASSERT_EQUALS(config.setAttr("LogFile", string("/var/log/sstoraged/system.log")), true);
 		string value;
 		TS_ASSERT_EQUALS(config.getAttr("LogFile", value), true);
 		TS_ASSERT_EQUALS(value, (string)"/var/log/sstoraged/system.log");
 	}
 	void testString_Default() {
 		Configurator &config = Configurator::getInstance();
 		string value;
 		TS_ASSERT_EQUALS(config.getAttr("LogFile", value), true);
 		TS_ASSERT_EQUALS(value, (string)"/var/log/sstoraged/system.log");
 	}
//  	void testString_Optional() {
//  		Configurator &config = Configurator::getInstance();
//  		string value;
//  		TS_ASSERT_EQUALS(config.getAttr("OptionalString", value), false);
// 		value = "test";
//  		TS_ASSERT_EQUALS(config.setAttr("OptionalString", value), true);
//  		TS_ASSERT_EQUALS(config.getAttr("OptionalString", value), true);
// 		TS_ASSERT_EQUALS(value, (string)"test");
//  	}
 	void testString_InvalidAttr() {
 		Configurator &config = Configurator::getInstance();
 		TS_ASSERT_EQUALS(config.setAttr("IllegalString", (string)"test"), false);
 	}

	/** clear */
	void testClear() {
 		Configurator &config = Configurator::getInstance();
 		config.setAttr("LogFile", (string)"test");
 		string value;
 		TS_ASSERT_EQUALS(config.getAttr("LogFile", value), true);
 		TS_ASSERT_EQUALS(value, (string)"test");
		TS_ASSERT_THROWS_NOTHING(config.clear());
		// defaulting to...
 		TS_ASSERT_EQUALS(config.getAttr("LogFile", value), true);
 		TS_ASSERT_EQUALS(value, (string)"/var/log/sstoraged/system.log");
	}

	/** parse */
	void testParse_Success() {
		string test = "{\n\t\"Bool\" : true,\n\t\"Number\" : 1,\n\t\"String\" : \"test\"\n}\n";
		Configurator &config = Configurator::getInstance();
		bool ret = false;
		TS_ASSERT_THROWS_NOTHING(ret = config.parse(test));
		TS_ASSERT_EQUALS(ret, true);
	}
	void testParse_Fail_InvalidFormat() {
		string test = "{\n\tBool : true,\n\t\"Number\" : 1,\n\t\"String\" : \"test\"\n}\n";
		Configurator &config = Configurator::getInstance();
		TS_ASSERT_THROWS_EQUALS(config.parse(test), const std::exception &e,
					string(e.what()), "Failed to parse configuration : * Line 2, Column 2\n  Missing \'}\' or object member name\n");
	}

	/** @todo : validate */

	/** dump */
	void testDump_EmptyConfig() {
		string lines = "";
		Configurator &config = Configurator::getInstance();
		TS_ASSERT_THROWS_NOTHING(lines = config.dump());
		TS_ASSERT_EQUALS(lines, "{}\n");
	}
	void testDump_Success() {
		string test = "{\n   \"Bool\" : true,\n   \"Number\" : 1,\n   \"String\" : \"test\"\n}\n";
		Configurator &config = Configurator::getInstance();
		config.parse(test);
		string lines = "";
		TS_ASSERT_THROWS_NOTHING(lines = config.dump());
		TS_ASSERT_EQUALS(lines, test);
	}

	/** @todo : setDefaults */

	/** @todo : default config test */
};
