#include <cxxtest/TestSuite.h>

using namespace std;

#include "rootvalidator.h"

class RootValidatorTestSuite : public CxxTest::TestSuite 
{
 public:
	void setUp()	{}
	void tearDown() {}

	void testConfigFile(){
		RootValidator validator;
 		TS_ASSERT_EQUALS(validator.validAttr("ConfigFile", (string)""), false);
 		TS_ASSERT_EQUALS(validator.validAttr("ConfigFile", (string)"test.conf"), true);
 		TS_ASSERT_EQUALS(validator.validAttr("ConfigFile", (string)"/etc/sstorage/sstoraged.conf"), true);
	}
	void testPIDfile(){
		RootValidator validator;
 		TS_ASSERT_EQUALS(validator.validAttr("PIDfile", (string)""), false);
 		TS_ASSERT_EQUALS(validator.validAttr("PIDfile", (string)"test.pid"), true);
 		TS_ASSERT_EQUALS(validator.validAttr("PIDfile", (string)"/var/run/sstoraged.pid"), true);
	}
	void testDebug(){
		RootValidator validator;
 		TS_ASSERT_EQUALS(validator.validAttr("Debug", true), true);
 		TS_ASSERT_EQUALS(validator.validAttr("Debug", false), true);
	}
	void testGlobalTargetName(){
		RootValidator validator;
 		TS_ASSERT_EQUALS(validator.validAttr("GlobalTargetName", (string)""), false);
 		TS_ASSERT_EQUALS(validator.validAttr("GlobalTargetName", (string)"A"), true);
 		TS_ASSERT_EQUALS(validator.validAttr("GlobalTargetName", (string)"iqn.2006-12.com.example:sStorage"), true);
	}
 	void testLocalTargetName(){
		RootValidator validator;
 		TS_ASSERT_EQUALS(validator.validAttr("LocalTargetName", (string)""), false);
 		TS_ASSERT_EQUALS(validator.validAttr("LocalTargetName", (string)"A"), true);
 		TS_ASSERT_EQUALS(validator.validAttr("LocalTargetName", (string)"iqn.2006-12.com.example:sStorage0"), true);
	}
// 	void testTargetPort(){
// 		RootValidator validator;
// 		TS_ASSERT_EQUALS(validator.validAttr("TargetPort", (string)""), false);
// 		TS_ASSERT_EQUALS(validator.validAttr("TargetPort", (string)"0.0.0.0:3260"), false);
// 		TS_ASSERT_EQUALS(validator.validAttr("TargetPort", (string)"[0.0.0.0]:3260"), true);
// 		TS_ASSERT_EQUALS(validator.validAttr("TargetPort", (string)"[fe01::1]:3260"), true);
// 	}
	void testConsolePort(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("ConsolePort", (string)""), false);
		TS_ASSERT_EQUALS(validator.validAttr("ConsolePort", (string)"0.0.0.0:3260"), false);
		TS_ASSERT_EQUALS(validator.validAttr("ConsolePort", (string)"[0.0.0.0]:3260"), true);
		TS_ASSERT_EQUALS(validator.validAttr("ConsolePort", (string)"[fe01::1]:3260"), true);
	}
	void testLogFile(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("LogFile", (string)""), false);
		TS_ASSERT_EQUALS(validator.validAttr("LogFile", (string)"/var/log/sstoraged/system.log"), true);
	}
	void testLogLevel(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("LogLevel", (string)""), false);
		TS_ASSERT_EQUALS(validator.validAttr("LogLevel", (string)"info"), true);
		TS_ASSERT_EQUALS(validator.validAttr("LogLevel", (string)"debug"), true);
	}
	void testMaxConnections(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("MaxConnections", 0LLU), false);
		TS_ASSERT_EQUALS(validator.validAttr("MaxConnections", 1LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("MaxConnections", 8LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("MaxConnections", 9LLU), false);
	}
	void testInitialR2T(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("InitialR2T", true), true);
		TS_ASSERT_EQUALS(validator.validAttr("InitialR2T", false), true);
	}
	void testImmediateData(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("ImmediateData", true), true);
		TS_ASSERT_EQUALS(validator.validAttr("ImmediateData", false), true);
	}
	void testMaxRecvDataSegmentLength(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("MaxRecvDataSegmentLength", 511LLU), false);
		TS_ASSERT_EQUALS(validator.validAttr("MaxRecvDataSegmentLength", 512LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("MaxRecvDataSegmentLength", 2LLU<<24-1), true);
		TS_ASSERT_EQUALS(validator.validAttr("MaxRecvDataSegmentLength", 2LLU<<24), false);
	}
	void testMaxBurstLength(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("MaxBurstLength", 511LLU), false);
		TS_ASSERT_EQUALS(validator.validAttr("MaxBurstLength", 512LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("MaxBurstLength", 2LLU<<24-1), true);
		TS_ASSERT_EQUALS(validator.validAttr("MaxBurstLength", 2LLU<<24), false);
	}
	void testFirstBurstLength(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("FirstBurstLength", 511LLU), false);
		TS_ASSERT_EQUALS(validator.validAttr("FirstBurstLength", 512LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("FirstBurstLength", 2LLU<<24-1), true);
		TS_ASSERT_EQUALS(validator.validAttr("FirstBurstLength", 2LLU<<24), false);
	}
	void testDefaultTime2Wait(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("DefaultTime2Wait", 0LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("DefaultTime2Wait", 3600LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("DefaultTime2Wait", 3601LLU), false);
	}
	void testDefaultTime2Retain(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("DefaultTime2Retain", 0LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("DefaultTime2Retain", 3600LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("DefaultTime2Retain", 3601LLU), false);
	}
	void testMaxOutstandingR2T(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("MaxOutstandingR2T", 0LLU), true);
 		TS_ASSERT_EQUALS(validator.validAttr("MaxOutstandingR2T", (uint64_t)USHRT_MAX), true);
 		TS_ASSERT_EQUALS(validator.validAttr("MaxOutstandingR2T", USHRT_MAX+1LLU), false);
	}
	void testDataPDUInOrder(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("DataPDUInOrder", false), false);
		TS_ASSERT_EQUALS(validator.validAttr("DataPDUInOrder", true), true);
	}
	void testDataSequenceInOrder(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("DataSequenceInOrder", false), false);
		TS_ASSERT_EQUALS(validator.validAttr("DataSequenceInOrder", true), true);
	}
	void testErrorRecoveryLevel(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("ErrorRecoveryLevel", 0LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("ErrorRecoveryLevel", 2LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("ErrorRecoveryLevel", 3LLU), false);
	}
	void testHeaderDigest(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("HeaderDigest", (string)""), false);
		TS_ASSERT_EQUALS(validator.validAttr("HeaderDigest", (string)"None"), true);
		TS_ASSERT_EQUALS(validator.validAttr("HeaderDigest", (string)"CHAP"), true);
		TS_ASSERT_EQUALS(validator.validAttr("HeaderDigest", (string)"None|CHAP"), true);
	}
	void testDataDigest(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("DataDigest", (string)""), false);
		TS_ASSERT_EQUALS(validator.validAttr("DataDigest", (string)"None"), true);
		TS_ASSERT_EQUALS(validator.validAttr("DataDigest", (string)"CHAP"), true);
		TS_ASSERT_EQUALS(validator.validAttr("DataDigest", (string)"None|CHAP"), true);
	}
	void testNopOutInterval(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("NopOutInterval", 0LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("NopOutInterval", 3600LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("NopOutInterval", 3601LLU), false);
	}
	void testNopOutTimeout(){
		RootValidator validator;
		TS_ASSERT_EQUALS(validator.validAttr("NopOutTimeout", 0LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("NopOutTimeout", 60LLU), true);
		TS_ASSERT_EQUALS(validator.validAttr("NopOutTimeout", 61LLU), false);
	}
// 	void testTarget(){
// 		RootValidator validator;
// 		TS_ASSERT_EQUALS(validator.validAttr("Target", (string)""), false);
// 		TS_ASSERT_EQUALS(validator.validAttr("Target", (string)""), true);
// 	}
// 	void testInitiator(){
// 		RootValidator validator;
// 		TS_ASSERT_EQUALS(validator.validAttr("Initiator", (string)""), false);
// 		TS_ASSERT_EQUALS(validator.validAttr("Initiator", (string)""), true);
// 	}
// 	void testVolume(){
// 		RootValidator validator;
// 		TS_ASSERT_EQUALS(validator.validAttr("Volume", (string)""), false);
// 		TS_ASSERT_EQUALS(validator.validAttr("Volume", (string)""), true);
// 	}
};
