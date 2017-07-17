#include <cxxtest/TestSuite.h>
#include <sstream>
#include <iostream>

using namespace std;

#include "targetconfigurator.h"

class TargetConfiguratorTestSuite : public CxxTest::TestSuite 
{
 public:
	void setUp()
	{
		Configurator &config = Configurator::getInstance();
		config.parse("{ \"Target\" : [ {\"TargetName\" : \"No.1\", \"TargetAlias\" : \"1\", \"TargetAddress\" : \"[192.168.0.1]:3260\"}, {\"TargetName\" : \"No.2\", \"TargetAlias\" : \"2\", \"TargetAddress\" : \"[192.168.0.2]:3260\"} ] }\n");
// 		cout << config.dump() << endl;
	}
	void tearDown()
	{
		Configurator &config = Configurator::getInstance();
		config.clear();
	}

	/** iterator */
 	void testIterator_Success() {
		TargetConfigurator *target = new TargetConfigurator;
		size_t counter = 0;
		for(TargetConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
			ostringstream os;
			os << "No." << (counter+1);
			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
 		}
		TS_ASSERT_EQUALS(counter, 2U);
		TS_ASSERT_THROWS_NOTHING(delete target);
 	}
	/** add */
 	void testAdd_Success() {
		TargetConfigurator *target = new TargetConfigurator;
		size_t counter = 0;
		for(TargetConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
			ostringstream os;
			os << "No." << (counter+1);
			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
 		}
		TS_ASSERT_EQUALS(counter, 2U);
		TargetInfo info((string)"No.3", (string)"[localhost]:3260");
		TS_ASSERT_EQUALS(target->add(info), true);
		counter = 0;
		for(TargetConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
			ostringstream os;
			os << "No." << (counter+1);
			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
 		}
		TS_ASSERT_EQUALS(counter, 3U);
		TargetInfo info2((string)"No.4", (string)"[localhost]:3261", (string)"4");
		TS_ASSERT_EQUALS(target->add(info2), true);
		counter = 0;
		for(TargetConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
			ostringstream os;
			os << "No." << (counter+1);
			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
 		}
		TS_ASSERT_EQUALS(counter, 4U);
		struct sockaddr_storage ss;
		ss.ss_family = AF_INET;
		struct sockaddr_in *sin = (struct sockaddr_in*)&ss;
		sin->sin_addr.s_addr = htonl(0xAC170001);
		sin->sin_port = htons(3263);
		TargetInfo info3((string)"No.5", ss, (string)"5");
		TS_ASSERT_EQUALS(target->add(info3), true);
		counter = 0;
		for(TargetConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
			ostringstream os;
			os << "No." << (counter+1);
			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
 		}
		TS_ASSERT_EQUALS(counter, 5U);

		TS_ASSERT_THROWS_NOTHING(delete target);

// 		Configurator &config = Configurator::getInstance();
//  		cout << config.dump() << endl;
 	}

	/** del */
 	void testDel_Success() {
		TargetConfigurator *target = new TargetConfigurator;
		size_t counter = 0;
		for(TargetConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
			ostringstream os;
			os << "No." << (counter+1);
			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
 		}
		TS_ASSERT_EQUALS(counter, 2U);
		TargetInfo info((string)"No.3", (string)"[localhost]:3260");
		TS_ASSERT_EQUALS(target->add(info), true);
		counter = 0;
		for(TargetConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
			ostringstream os;
			os << "No." << (counter+1);
			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
 		}
		TS_ASSERT_EQUALS(counter, 3U);
		TS_ASSERT_EQUALS(target->del("No.3"), true);
		counter = 0;
		for(TargetConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
			ostringstream os;
			os << "No." << (counter+1);
			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
 		}
		TS_ASSERT_EQUALS(counter, 2U);

		TS_ASSERT_THROWS_NOTHING(delete target);
 	}
 	void testDel_Failure() {
		TargetConfigurator *target = new TargetConfigurator;
		size_t counter = 0;
		for(TargetConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
			ostringstream os;
			os << "No." << (counter+1);
			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
 		}
		TS_ASSERT_EQUALS(counter, 2U);
		TS_ASSERT_EQUALS(target->del("No.2"), true);
		counter = 0;
		for(TargetConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
			ostringstream os;
			os << "No." << (counter+1);
			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
 		}
		TS_ASSERT_EQUALS(counter, 1U);
		TS_ASSERT_EQUALS(target->del("No.2"), false);
		counter = 0;
		for(TargetConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
			ostringstream os;
			os << "No." << (counter+1);
			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
 		}
		TS_ASSERT_EQUALS(counter, 1U);
		TS_ASSERT_EQUALS(target->del("No.1"), true);
		counter = 0;
		for(TargetConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
			ostringstream os;
			os << "No." << (counter+1);
			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
 		}
		TS_ASSERT_EQUALS(counter, 0U);
		TS_ASSERT_EQUALS(target->del("No.1"), false);
		counter = 0;
		for(TargetConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
			ostringstream os;
			os << "No." << (counter+1);
			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
 		}
		TS_ASSERT_EQUALS(counter, 0U);

		TS_ASSERT_THROWS_NOTHING(delete target);
 	}

	/** NullData */
	void testNullData(){
		Configurator &config = Configurator::getInstance();
		config.clear();
		TargetConfigurator *target = new TargetConfigurator;
		TargetInfo info((string)"No.1", (string)"[localhost]:3260");
		TS_ASSERT_EQUALS(target->add(info), true);
		TS_ASSERT_EQUALS(target->del("No.1"), true);
		TS_ASSERT_THROWS_NOTHING(delete target);
	}
};
