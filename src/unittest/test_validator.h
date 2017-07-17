#include <cxxtest/TestSuite.h>
#include <cstdint>

using namespace std;

#include "validator.h"

class ValidatorTestSuite : public CxxTest::TestSuite 
{
 public:
	void setUp()	{}
	void tearDown() {}

	/** RangeValidator */
 	void testRangeValidator_uint32_t() {
		vector<uint32_t> nums;
		nums.push_back(0U);
		nums.push_back(ULONG_MAX);

		vector<uint32_t>::iterator itr =
			find_if(nums.begin(), nums.end(), RangeValidator<uint32_t,uint32_t,0,ULONG_MAX>());
 		TS_ASSERT_EQUALS(itr, nums.end());

		RangeValidator<uint32_t,uint32_t,1U,ULONG_MAX-1> validator;
		TS_ASSERT_EQUALS(validator.invalid(0), true);
		TS_ASSERT_EQUALS(validator.invalid(ULONG_MAX), true);
	}

 	void testRangeValidator_int32_t() {
		vector<int32_t> nums;
		nums.push_back(LONG_MIN);
		nums.push_back(0L);
		nums.push_back(LONG_MAX);

		vector<int32_t>::iterator itr =
			find_if(nums.begin(), nums.end(), RangeValidator<int32_t,int32_t,LONG_MIN,LONG_MAX>());
 		TS_ASSERT_EQUALS(itr, nums.end());

		RangeValidator<int32_t,int32_t,0U,LONG_MAX-1> validator;
		TS_ASSERT_EQUALS(validator.invalid(-1), true);
		TS_ASSERT_EQUALS(validator.invalid(LONG_MAX), true);
	}

	/** LengthValidator */
  	void testLengthValidator_string() {
  		vector<string> strs;
  		strs.push_back("A");
  		strs.push_back("1234567890");

  		vector<string>::iterator itr =
  			find_if(strs.begin(), strs.end(), LengthValidator<string,size_t,1U,PATH_MAX>());
   		TS_ASSERT_EQUALS(itr, strs.end());

		LengthValidator<string,size_t,1U,2U> validator;
   		TS_ASSERT_EQUALS(validator.invalid(static_cast<string>("")), true);
  		TS_ASSERT_EQUALS(validator.invalid(static_cast<string>("123")), true);
 	}

	/** AttrValidator */
	void testAttrValidator_required() {
		const string required[] = {"Attr1","Attr2", ""};
		AttrValidator validator(required);
		TS_ASSERT_EQUALS(validator.valid("Attr1"), true);
		TS_ASSERT_EQUALS(validator.valid("Attr2"), true);
		TS_ASSERT_EQUALS(validator.valid("Attr3"), false);
		TS_ASSERT_EQUALS(validator.valid("Attr"), false);
		TS_ASSERT_EQUALS(validator.valid(""), false);
	}
	void testAttrValidator_optional() {
		const string required[] = {"Attr1","Attr2", ""};
		const string optional[] = {"Attr3","Attr4", ""};
		AttrValidator validator(required, optional);
		TS_ASSERT_EQUALS(validator.valid("Attr1"), true);
		TS_ASSERT_EQUALS(validator.valid("Attr2"), true);
		TS_ASSERT_EQUALS(validator.valid("Attr3"), true);
		TS_ASSERT_EQUALS(validator.valid("Attr4"), true);
		TS_ASSERT_EQUALS(validator.valid("Attr5"), false);
		TS_ASSERT_EQUALS(validator.valid("Attr"), false);
		TS_ASSERT_EQUALS(validator.valid(""), false);
	}
	void testAttrValidator_duplicate() {
		const string required[] = {"Attr1","Attr2", ""};
		const string optional[] = {"Attr3","Attr4", "Attr", "Attr2", "Attr1", ""};
		TS_ASSERT_THROWS_EQUALS(AttrValidator validator(required, optional),
					const std::exception &e,
					e.what(), string("Attr2 is duplicated"));
	}

	/** AddrStrValidator */
	void testAddrStrValidator_OK()
	{
		AddrStrValidator validator;
		TS_ASSERT_EQUALS(validator.valid(static_cast<string>("[172.16.0.1]:80")), true);
		TS_ASSERT_EQUALS(validator.valid(static_cast<string>("[FC01::1]:80")), true);
		TS_ASSERT_EQUALS(validator.valid(static_cast<string>("[::1]:80")), true);
	}
	void testAddrStrValidator_NG()
	{
		AddrStrValidator validator;;
		TS_ASSERT_EQUALS(validator.valid(static_cast<string>("")), false);
		TS_ASSERT_EQUALS(validator.valid(static_cast<string>("[]:")), false);
	}

};

