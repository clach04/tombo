#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#define TEST_CLASS_NAME XXXTest

class TEST_CLASS_NAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TEST_CLASS_NAME);
	CPPUNIT_TEST(dummyTest);
	CPPUNIT_TEST_SUITE_END();

public:
	TEST_CLASS_NAME() {}
	~TEST_CLASS_NAME() {}

	virtual void setUp() {}
	virtual void tearDown() {}

	void dummyTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TEST_CLASS_NAME);

////////////////////////////////////////////////

void TEST_CLASS_NAME::dummyTest() {
	CPPUNIT_ASSERT(1);
}
