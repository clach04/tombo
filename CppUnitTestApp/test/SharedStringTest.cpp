#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include <windows.h>
#include <tchar.h>
#include "TString.h"

#define TEST_CLASS_NAME SharedStringTest

class TEST_CLASS_NAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TEST_CLASS_NAME);
	CPPUNIT_TEST(InitTest1);
	CPPUNIT_TEST(InitTest2);
	CPPUNIT_TEST(InitTest3);
	CPPUNIT_TEST_SUITE_END();

public:
	TEST_CLASS_NAME() {}
	~TEST_CLASS_NAME() {}

	virtual void setUp() {}
	virtual void tearDown() {}

	void InitTest1();
	void InitTest2();
	void InitTest3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TEST_CLASS_NAME);

////////////////////////////////////////////////

void TEST_CLASS_NAME::InitTest1() {
	SharedString ss;

	CPPUNIT_ASSERT(ss.Init("TEST"));
	CPPUNIT_ASSERT(strcmp(ss.Get(), "TEST") == 0);
}

void TEST_CLASS_NAME::InitTest2() {
	SharedString ss;

	CPPUNIT_ASSERT(ss.Init("TEST"));
	CPPUNIT_ASSERT(strcmp(ss.Get(), "TEST") == 0);

	SharedString ss2(ss);
	CPPUNIT_ASSERT(strcmp(ss2.Get(), "TEST") == 0);
	CPPUNIT_ASSERT(ss.Get() == ss2.Get());
	CPPUNIT_ASSERT(ss2.pBuf->nRefCount == 2);

	SharedString ss3;
	CPPUNIT_ASSERT(ss3.Init(ss2));
	CPPUNIT_ASSERT(strcmp(ss3.Get(), "TEST") == 0);
	CPPUNIT_ASSERT(ss3.Get() == ss.Get());
	CPPUNIT_ASSERT(ss3.pBuf->nRefCount == 3);
}

void TEST_CLASS_NAME::InitTest3() {
	SharedString ss;
	CPPUNIT_ASSERT(ss.Init("TEST"));
	CPPUNIT_ASSERT(strcmp(ss.Get(), "TEST") == 0);

	{
		SharedString ss2(ss);
		CPPUNIT_ASSERT(strcmp(ss2.Get(), "TEST") == 0);
		CPPUNIT_ASSERT(ss.Get() == ss2.Get());
		CPPUNIT_ASSERT(ss2.pBuf->nRefCount == 2);
	}

	CPPUNIT_ASSERT(strcmp(ss.Get(), "TEST") == 0);
	CPPUNIT_ASSERT(ss.pBuf->nRefCount == 1);
}
