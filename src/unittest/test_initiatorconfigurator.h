#include <cxxtest/TestSuite.h>
#include <sstream>
#include <iostream>

using namespace std;

#include "configbinder.h"
#include "initiatorconfigurator.h"

class InitiatorConfiguratorTestSuite : public CxxTest::TestSuite 
{
 public:
	void setUp()
	{
		ConfigBinder *binder = new ConfigBinder();
		binder->load("testInitiatorConfigOK.conf");
		delete binder;
	}
	void tearDown()
	{
		Configurator &config = Configurator::getInstance();
		config.clear();
	}

	/** iterator */
  	void testIterator_Success() {
 		InitiatorConfigurator *initiator = new InitiatorConfigurator;
  		size_t counter = 0;
  		for(InitiatorConfigurator::iterator itr = initiator->begin();
   		    itr != initiator->end(); ++itr, ++counter){
  			ostringstream os;
  			os << "No." << (counter+1);
  			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
   		}
  		TS_ASSERT_EQUALS(counter, 3U);
  		TS_ASSERT_EQUALS(initiator->size(), 3U);
  		TS_ASSERT_THROWS_NOTHING(delete initiator);
  	}
 	/** add */
  	void testAdd_Success() {
 		InitiatorConfigurator *initiator = new InitiatorConfigurator;
 		size_t counter = 0;
 		for(InitiatorConfigurator::iterator itr = initiator->begin();
  		    itr != initiator->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 3U);
 		TS_ASSERT_EQUALS(initiator->size(), 3U);
 		InitiatorInfo info((string)"No.4", (string)"[localhost]:3260");
 		TS_ASSERT_EQUALS(initiator->add(info), true);
 		counter = 0;
 		for(InitiatorConfigurator::iterator itr = initiator->begin();
  		    itr != initiator->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 4U);
 		TS_ASSERT_EQUALS(initiator->size(), 4U);
 		InitiatorInfo info2((string)"No.5", (string)"[localhost]:3261", (string)"5");
 		TS_ASSERT_EQUALS(initiator->add(info2), true);
 		counter = 0;
 		for(InitiatorConfigurator::iterator itr = initiator->begin();
  		    itr != initiator->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 5U);
 		TS_ASSERT_EQUALS(initiator->size(), 5U);
 		struct sockaddr_storage ss;
 		ss.ss_family = AF_INET;
 		struct sockaddr_in *sin = (struct sockaddr_in*)&ss;
 		sin->sin_addr.s_addr = htonl(0xAC170001);
 		sin->sin_port = htons(3263);
 		InitiatorInfo info3((string)"No.6", ss, (string)"6");
 		TS_ASSERT_EQUALS(initiator->add(info3), true);
 		counter = 0;
 		for(InitiatorConfigurator::iterator itr = initiator->begin();
  		    itr != initiator->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 6U);
 		TS_ASSERT_EQUALS(initiator->size(), 6U);

 		TS_ASSERT_THROWS_NOTHING(delete initiator);

// 		Configurator &config = Configurator::getInstance();
//  		cout << config.dump() << endl;
  	}

 	/** del */
  	void testDel_Success() {
 		InitiatorConfigurator *initiator = new InitiatorConfigurator;
 		size_t counter = 0;
 		for(InitiatorConfigurator::iterator itr = initiator->begin();
  		    itr != initiator->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 3U);
 		TS_ASSERT_EQUALS(initiator->size(), 3U);
 		InitiatorInfo info((string)"No.4", (string)"[localhost]:3260");
 		TS_ASSERT_EQUALS(initiator->add(info), true);
 		counter = 0;
 		for(InitiatorConfigurator::iterator itr = initiator->begin();
  		    itr != initiator->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 4U);
 		TS_ASSERT_EQUALS(initiator->size(), 4U);
 		TS_ASSERT_EQUALS(initiator->del("No.4"), true);
 		counter = 0;
 		for(InitiatorConfigurator::iterator itr = initiator->begin();
  		    itr != initiator->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 3U);
 		TS_ASSERT_EQUALS(initiator->size(), 3U);

 		TS_ASSERT_THROWS_NOTHING(delete initiator);
  	}
  	void testDel_Failure() {
 		InitiatorConfigurator *initiator = new InitiatorConfigurator;
 		size_t counter = 0;
 		for(InitiatorConfigurator::iterator itr = initiator->begin();
  		    itr != initiator->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 3U);
 		TS_ASSERT_EQUALS(initiator->size(), 3U);
 		TS_ASSERT_EQUALS(initiator->del("No.3"), true);
 		counter = 0;
 		for(InitiatorConfigurator::iterator itr = initiator->begin();
  		    itr != initiator->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 2U);
 		TS_ASSERT_EQUALS(initiator->size(), 2U);
 		TS_ASSERT_EQUALS(initiator->del("No.3"), false);
 		counter = 0;
 		for(InitiatorConfigurator::iterator itr = initiator->begin();
  		    itr != initiator->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 2U);
 		TS_ASSERT_EQUALS(initiator->size(), 2U);
 		TS_ASSERT_EQUALS(initiator->del("No.2"), true);
 		counter = 0;
 		for(InitiatorConfigurator::iterator itr = initiator->begin();
  		    itr != initiator->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 1U);
 		TS_ASSERT_EQUALS(initiator->size(), 1U);
 		TS_ASSERT_EQUALS(initiator->del("No.2"), false);
 		counter = 0;
 		for(InitiatorConfigurator::iterator itr = initiator->begin();
  		    itr != initiator->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 1U);
 		TS_ASSERT_EQUALS(initiator->size(), 1U);
 		TS_ASSERT_EQUALS(initiator->del("No.1"), true);
 		counter = 0;
 		for(InitiatorConfigurator::iterator itr = initiator->begin();
  		    itr != initiator->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 0U);
 		TS_ASSERT_EQUALS(initiator->size(), 0U);
 		TS_ASSERT_EQUALS(initiator->del("No.1"), false);
 		counter = 0;
 		for(InitiatorConfigurator::iterator itr = initiator->begin();
  		    itr != initiator->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 0U);
 		TS_ASSERT_EQUALS(initiator->size(), 0U);

 		TS_ASSERT_THROWS_NOTHING(delete initiator);

// 		Configurator &config = Configurator::getInstance();
//  		cout << config.dump() << endl;
  	}
	/** validPeer */
	void testValidPeer_None_OK(){
		InitiatorConfigurator *initiator = new InitiatorConfigurator;
		InitiatorConfigurator::iterator itr = initiator->find("No.1");
		AuthInfoiSCSI none1("None","Discovery");
		TS_ASSERT_EQUALS(itr->validPeer(none1), true);
		AuthInfoiSCSI none2("None","Session");
		TS_ASSERT_EQUALS(itr->validPeer(none2), true);

		itr = initiator->find("No.3");
		TS_ASSERT_EQUALS(itr->validPeer(none1), true);
		TS_ASSERT_EQUALS(itr->validPeer(none2), true);

 		TS_ASSERT_THROWS_NOTHING(delete initiator);
	}
 	void testValidPeer_Chap_OK(){
 		InitiatorConfigurator *initiator = new InitiatorConfigurator;
 		AuthInfoiSCSI chap1("CHAP","Discovery");
 		chap1.setName("user1");
 		chap1.setPassword("pass1");
		InitiatorConfigurator::iterator itr = initiator->find("No.2");
 		TS_ASSERT_EQUALS(itr->validPeer(chap1), true);

 		AuthInfoiSCSI chap3("CHAP","Session");
 		chap3.setName("user3");
 		chap3.setPassword("pass3");
 		TS_ASSERT_EQUALS(itr->validPeer(chap3), true);

		itr = initiator->find("No.3");
 		AuthInfoiSCSI chap5("CHAP","Discovery");
		chap5.setName("user5");
		chap5.setPassword("pass5");
		TS_ASSERT_EQUALS(itr->validPeer(chap5), true);

 		AuthInfoiSCSI chap7("CHAP","Session");
		chap7.setName("user7");
		chap7.setPassword("pass7");
		TS_ASSERT_EQUALS(itr->validPeer(chap7), true);

// 		Configurator &config = Configurator::getInstance();
//  		cout << config.dump() << endl;

 		TS_ASSERT_THROWS_NOTHING(delete initiator);
 	}
 	void testValidPeer_Chap_NG(){
 		InitiatorConfigurator *initiator = new InitiatorConfigurator;
 		AuthInfoiSCSI chap1("CHAP","Discovery");
 		chap1.setName("user0");
 		chap1.setPassword("pass1");
		InitiatorConfigurator::iterator itr = initiator->find("No.2");
 		TS_ASSERT_EQUALS(itr->validPeer(chap1), false);

 		AuthInfoiSCSI chap2("CHAP","Session");
 		chap2.setName("user2");
 		chap2.setPassword("pass0");
 		TS_ASSERT_EQUALS(itr->validPeer(chap2), false);

 		AuthInfoiSCSI chap3("CHAP","Session");
 		chap3.setName("user2");
 		chap3.setPassword("");
 		TS_ASSERT_EQUALS(itr->validPeer(chap3), false);

		itr = initiator->find("No.3");
 		AuthInfoiSCSI chap5("CHAP","Discovery");
		chap5.setName("user");
		chap5.setPassword("pass5");
		TS_ASSERT_EQUALS(itr->validPeer(chap5), false);

 		AuthInfoiSCSI chap7("CHAP","Session");
		chap7.setName("user7");
		chap7.setPassword("pass7 ");
		TS_ASSERT_EQUALS(itr->validPeer(chap7), false);

 		TS_ASSERT_THROWS_NOTHING(delete initiator);
 	}
	/** getMyAuth */
	void testGetMyAuth_OK(){
		InitiatorConfigurator *initiator = new InitiatorConfigurator;
		InitiatorConfigurator::iterator itr = initiator->find("No.1");
		vector<AuthInfoiSCSI> infos;
		TS_ASSERT_EQUALS(itr->getMyAuth("Discovery", infos), true);
		for(vector<AuthInfoiSCSI>::iterator itr2 = infos.begin();
		    itr2 != infos.end(); ++itr2){
			TS_ASSERT_EQUALS(itr2->getMethod(), "None");
		}
		TS_ASSERT_EQUALS(infos.size(), 1U);
		TS_ASSERT_EQUALS(itr->getMyAuth("Session", infos), true);
		for(vector<AuthInfoiSCSI>::iterator itr2 = infos.begin();
		    itr2 != infos.end(); ++itr2){
			TS_ASSERT_EQUALS(itr2->getMethod(), "None");
		}
		TS_ASSERT_EQUALS(infos.size(), 1U);
 		TS_ASSERT_THROWS_NOTHING(delete initiator);
	}
	/** addLUN */
	void testAddLUN_OK(){
 		InitiatorConfigurator *initiator = new InitiatorConfigurator;
		InitiatorConfigurator::iterator itr = initiator->find("No.1");
		TS_ASSERT_EQUALS(itr->lun_size(), 2U);
		size_t counter = 0;
 		for(InitiatorInfo::lun_iterator itr2 = itr->lun_begin();
 		    itr2 != itr->lun_end(); ++itr2, ++counter){
 			ostringstream os;
 			os << "LV" << counter;
 			TS_ASSERT_EQUALS(os.str(), itr2->getKey());
 		}
		TS_ASSERT_EQUALS(counter, 2U);

		TS_ASSERT_EQUALS(itr->addLUN((string)"LV2"),true);
		TS_ASSERT_EQUALS(itr->lun_size(), 3U);
		counter = 0;
 		for(InitiatorInfo::lun_iterator itr2 = itr->lun_begin();
 		    itr2 != itr->lun_end(); ++itr2, ++counter){
 			ostringstream os;
 			os << "LV" << counter;
 			TS_ASSERT_EQUALS(os.str(), itr2->getKey());
 		}
		TS_ASSERT_EQUALS(counter, 3U);

 		TS_ASSERT_THROWS_NOTHING(delete initiator);
	}
	void testAddLUN_Duplicate(){
 		InitiatorConfigurator *initiator = new InitiatorConfigurator;
		InitiatorConfigurator::iterator itr = initiator->find("No.1");
		TS_ASSERT_EQUALS(itr->lun_size(), 2U);
		size_t counter = 0;
 		for(InitiatorInfo::lun_iterator itr2 = itr->lun_begin();
 		    itr2 != itr->lun_end(); ++itr2, ++counter){
 			ostringstream os;
 			os << "LV" << counter;
 			TS_ASSERT_EQUALS(os.str(), itr2->getKey());
 		}
		TS_ASSERT_EQUALS(counter, 2U);

		TS_ASSERT_EQUALS(itr->addLUN((string)"LV1"),false);
		TS_ASSERT_EQUALS(itr->lun_size(), 2U);
		counter = 0;
 		for(InitiatorInfo::lun_iterator itr2 = itr->lun_begin();
 		    itr2 != itr->lun_end(); ++itr2, ++counter){
 			ostringstream os;
 			os << "LV" << counter;
 			TS_ASSERT_EQUALS(os.str(), itr2->getKey());
 		}
		TS_ASSERT_EQUALS(counter, 2U);

 		TS_ASSERT_THROWS_NOTHING(delete initiator);
	}

	/** delLUN */
	void testDelLUN_OK(){
 		InitiatorConfigurator *initiator = new InitiatorConfigurator;
		InitiatorConfigurator::iterator itr = initiator->find("No.1");
		TS_ASSERT_EQUALS(itr->lun_size(), 2U);
 		size_t counter = 0;
 		for(InitiatorInfo::lun_iterator itr2 = itr->lun_begin();
 		    itr2 != itr->lun_end(); ++itr2, ++counter){
 			ostringstream os;
 			os << "LV" << counter;
 			TS_ASSERT_EQUALS(os.str(), itr2->getKey());
 		}
		TS_ASSERT_EQUALS(counter, 2U);

// 		Configurator &config = Configurator::getInstance();
//  		cout << config.dump() << endl;

		TS_ASSERT_EQUALS(itr->delLUN("LV1"),true);
 		TS_ASSERT_EQUALS(itr->lun_size(), 1U);
 		counter = 0;
 		for(InitiatorInfo::lun_iterator itr2 = itr->lun_begin();
 		    itr2 != itr->lun_end(); ++itr2, ++counter){
 			ostringstream os;
 			os << "LV" << counter;
 			TS_ASSERT_EQUALS(os.str(), itr2->getKey());
 		}
		TS_ASSERT_EQUALS(counter, 1U);

//  		cout << config.dump() << endl;

		TS_ASSERT_EQUALS(itr->delLUN("LV0"),true);
 		TS_ASSERT_EQUALS(itr->lun_size(), 0U);
 		counter = 0;
 		for(InitiatorInfo::lun_iterator itr2 = itr->lun_begin();
 		    itr2 != itr->lun_end(); ++itr2, ++counter){
 			ostringstream os;
 			os << "LV" << counter;
 			TS_ASSERT_EQUALS(os.str(), itr2->getKey());
 		}
		TS_ASSERT_EQUALS(counter, 0U);

//  		cout << config.dump() << endl;

 		TS_ASSERT_THROWS_NOTHING(delete initiator);
	}
	/** NullData */
	void testNullData(){
		Configurator &config = Configurator::getInstance();
		config.clear();
 		InitiatorConfigurator *initiator = new InitiatorConfigurator;
 		InitiatorInfo info((string)"No.1", (string)"[localhost]:3260");
 		TS_ASSERT_EQUALS(initiator->add(info), true);
		InitiatorConfigurator::iterator itr = initiator->find("No.1");
		TS_ASSERT_EQUALS(itr->addLUN((string)"LV1"), true);
 		TS_ASSERT_EQUALS(initiator->del("No.1"), true);

 		TS_ASSERT_THROWS_NOTHING(delete initiator);
	}
};
