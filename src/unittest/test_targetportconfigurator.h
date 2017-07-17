#include <cxxtest/TestSuite.h>
#include <sstream>
#include <iostream>

using namespace std;

#include "targetportconfigurator.h"

class TargetPortConfiguratorTestSuite : public CxxTest::TestSuite 
{
 public:
	void setUp()
	{
		Configurator &config = Configurator::getInstance();
		config.parse("{ \"TargetPort\" : [ \"[192.168.0.1]:3260\", \"[192.168.0.2]:3260\" ] }\n");
// 		cout << config.dump() << endl;
	}
	void tearDown()
	{
		Configurator &config = Configurator::getInstance();
		config.clear();
	}

	/** iterator */
 	void testIterator_Success() {
		TargetPortConfigurator *target = new TargetPortConfigurator;
		size_t counter = 0;
		for(TargetPortConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
// 			ostringstream os;
// 			os << "No." << (counter+1);
// 			TS_ASSERT_EQUALS(os.str(), (*itr).getKey());
 		}
		TS_ASSERT_EQUALS(counter, 2U);
		TS_ASSERT_THROWS_NOTHING(delete target);
 	}
	/** add */
 	void testAdd_Success() {
		TargetPortConfigurator *target = new TargetPortConfigurator;
		size_t counter = 0;
		for(TargetPortConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
// 			ostringstream os;
// 			os << "No." << (counter+1);
// 			TS_ASSERT_EQUALS(os.str(), (*itr).getKey());
 		}
		TS_ASSERT_EQUALS(counter, 2U);
		TargetPortInfo info((string)"[localhost]:3260");
		TS_ASSERT_EQUALS(target->add(info), true);
		counter = 0;
		for(TargetPortConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
// 			ostringstream os;
// 			os << "No." << (counter+1);
// 			TS_ASSERT_EQUALS(os.str(), (*itr).getKey());
 		}
		TS_ASSERT_EQUALS(counter, 3U);
		TargetPortInfo info2((string)"[localhost]:3261");
		TS_ASSERT_EQUALS(target->add(info2), true);
		counter = 0;
		for(TargetPortConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
// 			ostringstream os;
// 			os << "No." << (counter+1);
// 			TS_ASSERT_EQUALS(os.str(), (*itr).getKey());
 		}
		TS_ASSERT_EQUALS(counter, 4U);

		TS_ASSERT_THROWS_NOTHING(delete target);

// 		Configurator &config = Configurator::getInstance();
//  		cout << config.dump() << endl;
 	}

	/** del */
 	void testDel_Success() {
		TargetPortConfigurator *target = new TargetPortConfigurator;
		size_t counter = 0;
		for(TargetPortConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
// 			ostringstream os;
// 			os << "No." << (counter+1);
// 			TS_ASSERT_EQUALS(os.str(), (*itr).getKey());
 		}
		TS_ASSERT_EQUALS(counter, 2U);
		TargetPortInfo info((string)"[localhost]:3260");
		TS_ASSERT_EQUALS(target->add(info), true);
		counter = 0;
		for(TargetPortConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
// 			ostringstream os;
// 			os << "No." << (counter+1);
// 			TS_ASSERT_EQUALS(os.str(), (*itr).getKey());
 		}
		TS_ASSERT_EQUALS(counter, 3U);
		TS_ASSERT_EQUALS(target->del("[localhost]:3260"), true);
		counter = 0;
		for(TargetPortConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
// 			ostringstream os;
// 			os << "No." << (counter+1);
// 			TS_ASSERT_EQUALS(os.str(), (*itr).getKey());
 		}
		TS_ASSERT_EQUALS(counter, 2U);

		TS_ASSERT_THROWS_NOTHING(delete target);
 	}
 	void testDel_Failure() {
		TargetPortConfigurator *target = new TargetPortConfigurator;
		size_t counter = 0;
		for(TargetPortConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
// 			ostringstream os;
// 			os << "No." << (counter+1);
// 			TS_ASSERT_EQUALS(os.str(), (*itr).getKey());
 		}
		TS_ASSERT_EQUALS(counter, 2U);
		TS_ASSERT_EQUALS(target->del("[192.168.0.1]:3260"), true);
		counter = 0;
		for(TargetPortConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
// 			ostringstream os;
// 			os << "No." << (counter+1);
// 			TS_ASSERT_EQUALS(os.str(), (*itr).getKey());
 		}
		TS_ASSERT_EQUALS(counter, 1U);
		TS_ASSERT_EQUALS(target->del("[192.168.0.1]:3260"), false);
		counter = 0;
		for(TargetPortConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
// 			ostringstream os;
// 			os << "No." << (counter+1);
// 			TS_ASSERT_EQUALS(os.str(), (*itr).getKey());
 		}
		TS_ASSERT_EQUALS(counter, 1U);
		TS_ASSERT_EQUALS(target->del("[192.168.0.2]:3260"), true);
		counter = 0;
		for(TargetPortConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
// 			ostringstream os;
// 			os << "No." << (counter+1);
// 			TS_ASSERT_EQUALS(os.str(), (*itr).getKey());
 		}
		TS_ASSERT_EQUALS(counter, 0U);
		TS_ASSERT_EQUALS(target->del("[192.168.0.2]:3260"), false);
		counter = 0;
		for(TargetPortConfigurator::iterator itr = target->begin();
 		    itr != target->end(); ++itr, ++counter){
// 			ostringstream os;
// 			os << "No." << (counter+1);
// 			TS_ASSERT_EQUALS(os.str(), (*itr).getKey());
 		}
		TS_ASSERT_EQUALS(counter, 0U);

		TS_ASSERT_THROWS_NOTHING(delete target);
 	}

	/** NullData */
	void testNullData(){
		Configurator &config = Configurator::getInstance();
		config.clear();
		TargetPortConfigurator *target = new TargetPortConfigurator;
		TargetPortInfo info((string)"[localhost]:3260");
		TS_ASSERT_EQUALS(target->add(info), true);
		TS_ASSERT_EQUALS(target->del("[localhost]:3260"), true);
		TS_ASSERT_THROWS_NOTHING(delete target);
	}
};
