#include <cxxtest/TestSuite.h>
#include <sstream>
#include <iostream>

using namespace std;

#include "configbinder.h"
#include "volumeconfigurator.h"

class VolumeConfiguratorTestSuite : public CxxTest::TestSuite 
{
 public:
	void setUp()
	{
		ConfigBinder *binder = new ConfigBinder();
		binder->load("testVolumeConfigOK.conf");
		delete binder;
	}
	void tearDown()
	{
		Configurator &config = Configurator::getInstance();
		config.clear();
	}

	/** iterator */
  	void testIterator_Success() {
 		VolumeConfigurator *volume = new VolumeConfigurator;
 		size_t counter = 0;
 		for(VolumeConfigurator::iterator itr = volume->begin();
  		    itr != volume->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 3U);
 		TS_ASSERT_EQUALS(volume->size(), 3U);
 		TS_ASSERT_THROWS_NOTHING(delete volume);
  	}
 	/** add */
  	void testAdd_Success() {
 		VolumeConfigurator *volume = new VolumeConfigurator;
 		size_t counter = 0;
 		for(VolumeConfigurator::iterator itr = volume->begin();
  		    itr != volume->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 3U);
 		TS_ASSERT_EQUALS(volume->size(), 3U);
 		VolumeInfo info((string)"No.4");
		RealVolume part4("iqn.2006-12.com.example:sStorage0", "/tmp/dummy4",
				 0, 1024);
		TS_ASSERT_EQUALS(info.appendOrgPart(part4), true);
// 		TS_ASSERT_EQUALS(volume->add(info), true);
 		counter = 0;
 		for(VolumeConfigurator::iterator itr = volume->begin();
  		    itr != volume->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 4U);
 		TS_ASSERT_EQUALS(volume->size(), 4U);
 		VolumeInfo info2((string)"No.5");
		RealVolume part5("iqn.2006-12.com.example:sStorage0", "/tmp/dummy5",
				 0, 1024);
		TS_ASSERT_EQUALS(info2.appendOrgPart(part5), true);
// 		TS_ASSERT_EQUALS(volume->add(info2), true);
 		counter = 0;
 		for(VolumeConfigurator::iterator itr = volume->begin();
  		    itr != volume->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 5U);
 		TS_ASSERT_EQUALS(volume->size(), 5U);
 		VolumeInfo info3((string)"No.6");
		RealVolume part6("iqn.2006-12.com.example:sStorage0", "/tmp/dummy6",
				 0, 1024);
		TS_ASSERT_EQUALS(info3.appendOrgPart(part6), true);
// 		TS_ASSERT_EQUALS(volume->add(info3), true);
 		counter = 0;
 		for(VolumeConfigurator::iterator itr = volume->begin();
  		    itr != volume->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 6U);
 		TS_ASSERT_EQUALS(volume->size(), 6U);

 		TS_ASSERT_THROWS_NOTHING(delete volume);

// 		Configurator &config = Configurator::getInstance();
//  		cout << config.dump() << endl;
  	}

 	/** del */
   	void testDel_Success() {
  		VolumeConfigurator *volume = new VolumeConfigurator;
  		size_t counter = 0;
  		for(VolumeConfigurator::iterator itr = volume->begin();
   		    itr != volume->end(); ++itr, ++counter){
  			ostringstream os;
  			os << "No." << (counter+1);
  			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
   		}
  		TS_ASSERT_EQUALS(counter, 3U);
  		TS_ASSERT_EQUALS(volume->size(), 3U);
  		VolumeInfo info((string)"No.4");
 		RealVolume part4("iqn.2006-12.com.example:sStorage0", "/tmp/dummy4",
 				 0, 1024);
 		TS_ASSERT_EQUALS(info.appendOrgPart(part4), true);
//  		TS_ASSERT_EQUALS(volume->add(info), true);
  		counter = 0;
  		for(VolumeConfigurator::iterator itr = volume->begin();
   		    itr != volume->end(); ++itr, ++counter){
  			ostringstream os;
  			os << "No." << (counter+1);
  			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
   		}
  		TS_ASSERT_EQUALS(counter, 4U);
  		TS_ASSERT_EQUALS(volume->size(), 4U);
  		TS_ASSERT_EQUALS(volume->del("No.4"), true);
  		counter = 0;
  		for(VolumeConfigurator::iterator itr = volume->begin();
   		    itr != volume->end(); ++itr, ++counter){
  			ostringstream os;
  			os << "No." << (counter+1);
  			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
   		}
  		TS_ASSERT_EQUALS(counter, 3U);
  		TS_ASSERT_EQUALS(volume->size(), 3U);

  		TS_ASSERT_THROWS_NOTHING(delete volume);
   	}
  	void testDel_Failure() {
 		VolumeConfigurator *volume = new VolumeConfigurator;
 		size_t counter = 0;
 		for(VolumeConfigurator::iterator itr = volume->begin();
  		    itr != volume->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 3U);
 		TS_ASSERT_EQUALS(volume->size(), 3U);
 		TS_ASSERT_EQUALS(volume->del("No.3"), true);
 		counter = 0;
 		for(VolumeConfigurator::iterator itr = volume->begin();
  		    itr != volume->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 2U);
 		TS_ASSERT_EQUALS(volume->size(), 2U);
 		TS_ASSERT_EQUALS(volume->del("No.3"), false);
 		counter = 0;
 		for(VolumeConfigurator::iterator itr = volume->begin();
  		    itr != volume->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 2U);
 		TS_ASSERT_EQUALS(volume->size(), 2U);
 		TS_ASSERT_EQUALS(volume->del("No.2"), true);
 		counter = 0;
 		for(VolumeConfigurator::iterator itr = volume->begin();
  		    itr != volume->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 1U);
 		TS_ASSERT_EQUALS(volume->size(), 1U);
 		TS_ASSERT_EQUALS(volume->del("No.2"), false);
 		counter = 0;
 		for(VolumeConfigurator::iterator itr = volume->begin();
  		    itr != volume->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 1U);
 		TS_ASSERT_EQUALS(volume->size(), 1U);
 		TS_ASSERT_EQUALS(volume->del("No.1"), true);
 		counter = 0;
 		for(VolumeConfigurator::iterator itr = volume->begin();
  		    itr != volume->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 0U);
 		TS_ASSERT_EQUALS(volume->size(), 0U);
 		TS_ASSERT_EQUALS(volume->del("No.1"), false);
 		counter = 0;
 		for(VolumeConfigurator::iterator itr = volume->begin();
  		    itr != volume->end(); ++itr, ++counter){
 			ostringstream os;
 			os << "No." << (counter+1);
 			TS_ASSERT_EQUALS(os.str(), (*itr).getName());
  		}
 		TS_ASSERT_EQUALS(counter, 0U);
 		TS_ASSERT_EQUALS(volume->size(), 0U);

 		TS_ASSERT_THROWS_NOTHING(delete volume);

// 		Configurator &config = Configurator::getInstance();
//  		cout << config.dump() << endl;
  	}
	/** appendOrgPart */
 	void testAppendOrg_OK(){
  		VolumeConfigurator *volume = new VolumeConfigurator;
 		VolumeConfigurator::iterator itr = volume->find("No.1");
 		TS_ASSERT_EQUALS(itr->partsOrg(), 1ULL);
		TS_ASSERT_EQUALS(itr->sectorSizeOrg(), 1024ULL);
 		TS_ASSERT_EQUALS(itr->volumesRep(), 1ULL);

		RealVolume part1("[localhost]:3260", "/tmp/dummy1",
				 0, 1024);
		TS_ASSERT_EQUALS(itr->appendOrgPart(part1), true);
 		TS_ASSERT_EQUALS(itr->partsOrg(), 2ULL);
		TS_ASSERT_EQUALS(itr->sectorSizeOrg(), 2048ULL);
 		TS_ASSERT_EQUALS(itr->volumesRep(), 1ULL);

		RealVolume part2("[FC::1]:3260", "/tmp/dummy2",
				 1024, 1024);
		TS_ASSERT_EQUALS(itr->appendOrgPart(part2), true);
 		TS_ASSERT_EQUALS(itr->partsOrg(), 3ULL);
		TS_ASSERT_EQUALS(itr->sectorSizeOrg(), 3072ULL);
 		TS_ASSERT_EQUALS(itr->volumesRep(), 1ULL);
// 		Configurator &config = Configurator::getInstance();
//   		cout << config.dump() << endl;
 		TS_ASSERT_THROWS_NOTHING(delete volume);
 	}
// 	void testAppendOrg_Duplicate(){
// 	}
	/** createRep */
 	/** appendRepPart */
 	void testCreateAppendRep_OK(){
  		VolumeConfigurator *volume = new VolumeConfigurator;
 		VolumeConfigurator::iterator itr = volume->find("No.1");
 		TS_ASSERT_EQUALS(itr->partsOrg(), 1ULL);
		TS_ASSERT_EQUALS(itr->sectorSizeOrg(), 1024ULL);
 		TS_ASSERT_EQUALS(itr->volumesRep(), 1ULL);

//   		Configurator &config = Configurator::getInstance();
//     		cout << config.dump() << endl;

		RealVolume part1("[localhost]:3260", "/tmp/dummy1",
				 0, 1024);
		size_t index = ULONG_MAX;
		TS_ASSERT_EQUALS(itr->createRep(part1, index), true);

//     		cout << config.dump() << endl;

 		TS_ASSERT_EQUALS(index, 1ULL);
  		TS_ASSERT_EQUALS(itr->partsOrg(), 1ULL);
 		TS_ASSERT_EQUALS(itr->sectorSizeOrg(), 1024ULL);
  		TS_ASSERT_EQUALS(itr->volumesRep(), 2ULL);
   		TS_ASSERT_EQUALS(itr->partsRep(1), 1ULL);
  		TS_ASSERT_EQUALS(itr->sectorSizeRep(1), 1024ULL);

 		RealVolume part2("[FC::1]:3260", "/tmp/dummy2",
 				 1024, 1024);
 		TS_ASSERT_EQUALS(itr->appendRepPart(1, part2), true);

//     		cout << config.dump() << endl;

  		TS_ASSERT_EQUALS(itr->partsOrg(), 1ULL);
 		TS_ASSERT_EQUALS(itr->sectorSizeOrg(), 1024ULL);
  		TS_ASSERT_EQUALS(itr->volumesRep(), 2ULL);
  		TS_ASSERT_EQUALS(itr->partsRep(1), 2ULL);
  		TS_ASSERT_EQUALS(itr->sectorSizeRep(1), 2048ULL);
 		TS_ASSERT_THROWS_NOTHING(delete volume);
 	}
// 	void testCreateRep_NG(){
// 	}
	/** removeRep*/
 	void testRemoveRep_OK(){
//  		Configurator &config = Configurator::getInstance();
//   		cout << config.dump() << endl;

  		VolumeConfigurator *volume = new VolumeConfigurator;
 		VolumeConfigurator::iterator itr = volume->find("No.1");
 		TS_ASSERT_EQUALS(itr->partsOrg(), 1ULL);
		TS_ASSERT_EQUALS(itr->sectorSizeOrg(), 1024ULL);
 		TS_ASSERT_EQUALS(itr->volumesRep(), 1ULL);

		RealVolume part1("[localhost]:3260", "/tmp/dummy1",
				 0, 1024);
		size_t index;
		TS_ASSERT_EQUALS(itr->createRep(part1, index), true);

//   		cout << "Create part1 : " << config.dump() << endl;

		TS_ASSERT_EQUALS(index, 1ULL);
 		TS_ASSERT_EQUALS(itr->partsOrg(), 1ULL);
		TS_ASSERT_EQUALS(itr->sectorSizeOrg(), 1024ULL);
 		TS_ASSERT_EQUALS(itr->volumesRep(), 2ULL);
 		TS_ASSERT_EQUALS(itr->partsRep(1), 1ULL);
 		TS_ASSERT_EQUALS(itr->sectorSizeRep(1), 1024ULL);

		RealVolume part2("[FC::1]:3260", "/tmp/dummy2",
				 1024, 1024);
		TS_ASSERT_EQUALS(itr->appendRepPart(1, part2), true);

//   		cout << "Append part2 : " << config.dump() << endl;

 		TS_ASSERT_EQUALS(itr->partsOrg(), 1ULL);
		TS_ASSERT_EQUALS(itr->sectorSizeOrg(), 1024ULL);
 		TS_ASSERT_EQUALS(itr->volumesRep(), 2ULL);
 		TS_ASSERT_EQUALS(itr->partsRep(1), 2ULL);
 		TS_ASSERT_EQUALS(itr->sectorSizeRep(1), 2048ULL);

		RealVolume part3("[FC::3]:3260", "/tmp/dummy3",
				 0, 512);
		TS_ASSERT_EQUALS(itr->createRep(part3, index), true);

//   		cout  << "Create part3 : "<< config.dump() << endl;

		TS_ASSERT_EQUALS(index, 2U);
 		TS_ASSERT_EQUALS(itr->partsOrg(), 1ULL);
		TS_ASSERT_EQUALS(itr->sectorSizeOrg(), 1024ULL);
 		TS_ASSERT_EQUALS(itr->volumesRep(), 3ULL);
 		TS_ASSERT_EQUALS(itr->partsRep(1), 2ULL);
 		TS_ASSERT_EQUALS(itr->sectorSizeRep(1), 2048ULL);
 		TS_ASSERT_EQUALS(itr->partsRep(2), 1ULL);
 		TS_ASSERT_EQUALS(itr->sectorSizeRep(2), 512ULL);

		TS_ASSERT_EQUALS(itr->removeRep(1), true);

//   		cout  << "Remove 0 : "<< config.dump() << endl;

 		TS_ASSERT_EQUALS(itr->partsOrg(), 1ULL);
		TS_ASSERT_EQUALS(itr->sectorSizeOrg(), 1024ULL);
 		TS_ASSERT_EQUALS(itr->volumesRep(), 2ULL);
 		TS_ASSERT_EQUALS(itr->partsRep(1), 1ULL);
 		TS_ASSERT_EQUALS(itr->sectorSizeRep(1), 512ULL);

 		RealVolume part4("[FC::4]:3260", "/tmp/dummy4",
  				 0, 768);

  		TS_ASSERT_EQUALS(itr->createRep(part4, index), true);

//    		cout << "Create part4 : " << config.dump() << endl;

		TS_ASSERT_EQUALS(index, 2U);
  		TS_ASSERT_EQUALS(itr->partsOrg(), 1ULL);
 		TS_ASSERT_EQUALS(itr->sectorSizeOrg(), 1024ULL);
  		TS_ASSERT_EQUALS(itr->volumesRep(), 3ULL);
  		TS_ASSERT_EQUALS(itr->partsRep(1), 1ULL);
  		TS_ASSERT_EQUALS(itr->sectorSizeRep(1), 512ULL);
  		TS_ASSERT_EQUALS(itr->partsRep(2), 1ULL);
  		TS_ASSERT_EQUALS(itr->sectorSizeRep(2), 768ULL);

 		TS_ASSERT_EQUALS(itr->removeRep(0), true);
		TS_ASSERT_EQUALS(itr->removeRep(0), true);
		TS_ASSERT_EQUALS(itr->removeRep(0), true);

//   		cout << config.dump() << endl;

  		TS_ASSERT_EQUALS(itr->partsOrg(), 1ULL);
 		TS_ASSERT_EQUALS(itr->sectorSizeOrg(), 1024ULL);
  		TS_ASSERT_EQUALS(itr->volumesRep(), 0ULL);
 		TS_ASSERT_THROWS_NOTHING(delete volume);
	}
//  	void testRemoveRep_NG(){
// 	}

	/** NullData */
 	void testNullData(){
 		Configurator &config = Configurator::getInstance();
 		config.clear();
 		ConfigBinder *binder = new ConfigBinder();
 		binder->load("testVolumeConfigNULL.conf");
 		delete binder;
  		VolumeConfigurator *volume = new VolumeConfigurator;
  		VolumeInfo info((string)"No.1");
 		RealVolume part4("iqn.2006-12.com.example:sStorage0", "/tmp/dummy4",
 				 0, 1024);
 		TS_ASSERT_EQUALS(info.appendOrgPart(part4), true);
//  		TS_ASSERT_EQUALS(volume->add(info), true);
 		VolumeConfigurator::iterator itr = volume->find("No.1");
  		TS_ASSERT_EQUALS(volume->del("No.1"), true);

  		TS_ASSERT_THROWS_NOTHING(delete volume);
 	}
};
