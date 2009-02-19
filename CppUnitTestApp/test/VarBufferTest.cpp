#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include <windows.h>
#include "VarBuffer.h"

class VarBufferTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(VarBufferTest);
	CPPUNIT_TEST(VarBufferImplTest1);
	CPPUNIT_TEST(VarBufferImplTest2);
	CPPUNIT_TEST(VarBufferImplTest3);
	CPPUNIT_TEST(VarBufferImplTest4);
	CPPUNIT_TEST(VarBufferImplTest5);
	CPPUNIT_TEST_SUITE_END();

public:
	VarBufferTest() {}
	~VarBufferTest() {}

	virtual void setUp() {}
	virtual void tearDown() {}

	void VarBufferImplTest1();
	void VarBufferImplTest2();
	void VarBufferImplTest3();
	void VarBufferImplTest4();
	void VarBufferImplTest5();
};

CPPUNIT_TEST_SUITE_REGISTRATION(VarBufferTest);

void VarBufferTest::VarBufferImplTest1()
{
	VarBufferImpl vb;
	DWORD nc;

	CPPUNIT_ASSERT(vb.Init(10, 10));
	CPPUNIT_ASSERT(LocalSize(vb.pBuf) == 10);

	// initial data. not extended
	char buf[20];
	strcpy(buf, "0123456789abcdef");
	CPPUNIT_ASSERT(vb.Add((LPBYTE)buf, 5, NULL));
	CPPUNIT_ASSERT(LocalSize(vb.pBuf) == 10);
	CPPUNIT_ASSERT(vb.nCurrentUse == 5);
	CPPUNIT_ASSERT(strncmp((const char*)vb.GetBuffer(), "01234", 5) == 0);

	// append data
	CPPUNIT_ASSERT(vb.Add((LPBYTE)(buf + 5), 4, &nc));
	CPPUNIT_ASSERT(LocalSize(vb.pBuf) == 10);
	CPPUNIT_ASSERT(vb.nCurrentUse == 9);
	CPPUNIT_ASSERT(strncmp((const char*)vb.GetBuffer(), "012345678", 9) == 0);
	CPPUNIT_ASSERT(nc == 5);

	// append data exntend memory
	CPPUNIT_ASSERT(vb.Add((LPBYTE)(buf + 9), 1, &nc));
	CPPUNIT_ASSERT(LocalSize(vb.pBuf) == 20);
	CPPUNIT_ASSERT(vb.nCurrentUse == 10);
	CPPUNIT_ASSERT(strncmp((const char*)vb.GetBuffer(), "0123456789", 10) == 0);
	CPPUNIT_ASSERT(nc == 9);

}

void VarBufferTest::VarBufferImplTest2()
{
	VarBufferImpl vb;

	CPPUNIT_ASSERT(vb.Init(5, 5));
	CPPUNIT_ASSERT(LocalSize(vb.pBuf) == 5);

	// extend multi blocks
	char buf[30];
	CPPUNIT_ASSERT(vb.Add((LPBYTE)buf, 19, NULL));
	CPPUNIT_ASSERT(LocalSize(vb.pBuf) == 20);
	CPPUNIT_ASSERT(vb.nCurrentUse = 19);
}

void VarBufferTest::VarBufferImplTest3()
{
	////////
	// clear test

	char buf[30];

	VarBufferImpl vb1;
	CPPUNIT_ASSERT(vb1.Init(5, 5));
	CPPUNIT_ASSERT(LocalSize(vb1.pBuf) == 5);

	CPPUNIT_ASSERT(vb1.Add((LPBYTE)buf, 20, NULL));
	CPPUNIT_ASSERT(LocalSize(vb1.pBuf) == 25);
	CPPUNIT_ASSERT(vb1.nCurrentUse = 20);

	// clear but keep buffer
	CPPUNIT_ASSERT(vb1.Clear(FALSE));
	CPPUNIT_ASSERT(vb1.nCurrentUse == 0);
	CPPUNIT_ASSERT(LocalSize(vb1.pBuf) == 25);

	// clear and realloc buffer
	CPPUNIT_ASSERT(vb1.Clear(TRUE));
	CPPUNIT_ASSERT(vb1.nCurrentUse == 0);
	CPPUNIT_ASSERT(LocalSize(vb1.pBuf) == 5);

}

void VarBufferTest::VarBufferImplTest4()
{
	// extend test
	const char *pSample = "0123456789abcdef";

	VarBufferImpl vb;
	CPPUNIT_ASSERT(vb.Init(10, 5));
	CPPUNIT_ASSERT(LocalSize(vb.pBuf) == 10);
	CPPUNIT_ASSERT(vb.Add((LPBYTE)pSample, 5, NULL));

	CPPUNIT_ASSERT(vb.Extend(0, 2));
	CPPUNIT_ASSERT(strncmp((const char*)vb.GetBuffer() + 2, "012345678", 5) == 0);

	CPPUNIT_ASSERT(vb.Extend(7, 2));
	CPPUNIT_ASSERT(vb.nCurrentUse == 9);

	CPPUNIT_ASSERT(vb.Extend(4, 2));
	CPPUNIT_ASSERT(vb.nCurrentUse == 11);
	CPPUNIT_ASSERT(strncmp((const char*)vb.GetBuffer() + 2, "012", 3) == 0);
	CPPUNIT_ASSERT(strncmp((const char*)vb.GetBuffer() + 7, "345", 2) == 0);
}

void VarBufferTest::VarBufferImplTest5()
{
	// shorten test
	const char *pSample = "0123456789abcdef";
	VarBufferImpl vb;

	CPPUNIT_ASSERT(vb.Init(10, 5));
	CPPUNIT_ASSERT(LocalSize(vb.pBuf) == 10);
	CPPUNIT_ASSERT(vb.Add((LPBYTE)pSample, 9, NULL));
	CPPUNIT_ASSERT(strncmp((const char*)vb.GetBuffer(), pSample, 9) == 0);

	CPPUNIT_ASSERT(vb.Shorten(0, 2));
	CPPUNIT_ASSERT(vb.nCurrentUse == 7);
	CPPUNIT_ASSERT(strncmp((const char*)vb.GetBuffer(), pSample + 2, 7) == 0);

	CPPUNIT_ASSERT(vb.Shorten(5, 2));
	CPPUNIT_ASSERT(vb.nCurrentUse == 5);
	CPPUNIT_ASSERT(strncmp((const char*)vb.GetBuffer(), pSample + 2, 5) == 0);

}