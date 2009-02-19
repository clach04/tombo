#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include <windows.h>
#include "UniConv.h"

#define TEST_CLASS_NAME UniConvTest

class TEST_CLASS_NAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TEST_CLASS_NAME);
	CPPUNIT_TEST(Base64EncodeTest1);
	CPPUNIT_TEST(Base64EncodeTest2);
	CPPUNIT_TEST(Base64EncodeTest3);
	CPPUNIT_TEST(Base64EncodeTest4_1);
	CPPUNIT_TEST(Base64EncodeTest4_2);
	CPPUNIT_TEST(Base64EncodeTest4_3);
	CPPUNIT_TEST(Base64EncodeTest4_4);
	CPPUNIT_TEST(Base64EncodeTest4_5);
	CPPUNIT_TEST(Base64EncodeTest4_6);
	CPPUNIT_TEST(Base64EncodeTest4_7);

	CPPUNIT_TEST(Base64DecodeTest1);
	CPPUNIT_TEST(Base64DecodeTest2);

	CPPUNIT_TEST(ConvUTF8ToUCS2Test1);
	CPPUNIT_TEST(ConvUTF8ToUCS2Test2);
	CPPUNIT_TEST(ConvUTF8ToUCS2Test3);

	CPPUNIT_TEST(ConvUCS2ToUTF8Test1);
	CPPUNIT_TEST(ConvUCS2ToUTF8Test2);
	CPPUNIT_TEST(ConvUCS2ToUTF8Test3);

	CPPUNIT_TEST_SUITE_END();

public:
	TEST_CLASS_NAME() {}
	~TEST_CLASS_NAME() {}

	virtual void setUp() {}
	virtual void tearDown() {}

	void Base64EncodeTest1();
	void Base64EncodeTest2();
	void Base64EncodeTest3();
	void Base64EncodeTest4_1();
	void Base64EncodeTest4_2();
	void Base64EncodeTest4_3();
	void Base64EncodeTest4_4();
	void Base64EncodeTest4_5();
	void Base64EncodeTest4_6();
	void Base64EncodeTest4_7();

	void Base64DecodeTest1();
	void Base64DecodeTest2();

	void ConvUTF8ToUCS2Test1();
	void ConvUTF8ToUCS2Test2();
	void ConvUTF8ToUCS2Test3();

	void ConvUCS2ToUTF8Test1();
	void ConvUCS2ToUTF8Test2();
	void ConvUCS2ToUTF8Test3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TEST_CLASS_NAME);

// test probe
extern DWORD g_Base64EncodeAllocSize;

////////////////////////////////////////////////

void TEST_CLASS_NAME::Base64EncodeTest1() {
	char *pOut = Base64Encode((LPBYTE)"Hello", 5);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 9);
	CPPUNIT_ASSERT(strcmp("SGVsbG8=", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64EncodeTest2() {
	char *pOut = Base64Encode(NULL, 5);
	CPPUNIT_ASSERT(pOut == NULL);
}

void TEST_CLASS_NAME::Base64EncodeTest3() {
	char *pOut = Base64Encode((LPBYTE)"Hello", 0);
	CPPUNIT_ASSERT(pOut == NULL);
}

void TEST_CLASS_NAME::Base64EncodeTest4_1()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 1);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 5);
	CPPUNIT_ASSERT(strcmp("SA==", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64EncodeTest4_2()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 2);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 5);
	CPPUNIT_ASSERT(strcmp("SGU=", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64EncodeTest4_3()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 3);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 5);
	CPPUNIT_ASSERT(strcmp("SGVs", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64EncodeTest4_4()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 4);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 9);
	CPPUNIT_ASSERT(strcmp("SGVsbA==", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64EncodeTest4_5()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 5);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 9);
	CPPUNIT_ASSERT(strcmp("SGVsbG8=", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64EncodeTest4_6()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 6);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 9);
	CPPUNIT_ASSERT(strcmp("SGVsbG8g", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64EncodeTest4_7()
{
	char *pOut = Base64Encode((LPBYTE)"Hello w", 7);

	CPPUNIT_ASSERT(g_Base64EncodeAllocSize == 13);
	CPPUNIT_ASSERT(strcmp("SGVsbG8gdw==", pOut) == 0);
	delete [] pOut;
}

void TEST_CLASS_NAME::Base64DecodeTest1()
{
	DWORD n;
	LPBYTE p = Base64Decode("SGVsbG8=", &n);
	CPPUNIT_ASSERT(n == 5);
	CPPUNIT_ASSERT(strncmp((char*)p, "Hello", 5) == 0);
	delete [] p;
}

void TEST_CLASS_NAME::Base64DecodeTest2()
{
	DWORD n;
	LPBYTE p = Base64Decode("SGVsbG8", &n);
	CPPUNIT_ASSERT(p == NULL);
}

// UTF-8 1 byte conversion
void TEST_CLASS_NAME::ConvUTF8ToUCS2Test1()
{
	char aInput[] = { 0x54, 0x45, 0x53, 0x54, 0x00};	// "TEST"
	WCHAR aExpect[] = {0x0054, 0x0045, 0x0053, 0x0054, 0x0000};

	LPWSTR pResult = ConvUTF8ToUCS2(aInput);
	CPPUNIT_ASSERT(wcscmp(aExpect, pResult) == 0);
}

// UTF-8 2 byte conversion
void TEST_CLASS_NAME::ConvUTF8ToUCS2Test2()
{
	char aInput[] = { (char)0xce, (char)0xb8, (char)0xcf, (char)0x80, 0x00};	//  #GREEK SMALL LETTER THETA, #GREEK SMALL LETTER PI
	WCHAR aExpect[] = {0x03b8, 0x03c0, 0x0000};

	LPWSTR pResult = ConvUTF8ToUCS2(aInput);
	CPPUNIT_ASSERT(wcscmp(aExpect, pResult) == 0);
}

// UTF-8 3 byte conversion
void TEST_CLASS_NAME::ConvUTF8ToUCS2Test3()
{
	char aInput[] = {	(char)0xe3, (char)0x81, (char)0xa8, (char)0xe3,	// TOMBO by Hira-gana and Kanji
						(char)0x82, (char)0x93, (char)0xe3, (char)0x81,
						(char)0xbc, (char)0xe8, (char)0x9c, (char)0xbb,
						(char)0xe8, (char)0x9b, (char)0x89, (char)0x00};
	WCHAR aExpect[] = {0x3068, 0x3093, 0x307c, 0x873b, 0x86c9, 0x0000};

	LPWSTR pResult = ConvUTF8ToUCS2(aInput);
	CPPUNIT_ASSERT(wcscmp(aExpect, pResult) == 0);
}

void TEST_CLASS_NAME::ConvUCS2ToUTF8Test1()
{
	WCHAR aInput[] = {0x0054, 0x0045, 0x0053, 0x0054, 0x0000};	// "TEST"
	char aExpect[] = { 0x54, 0x45, 0x53, 0x54, 0x00};

	char *pResult = ConvUCS2ToUTF8(aInput);
	CPPUNIT_ASSERT(strcmp(aExpect, pResult) == 0);
}

void TEST_CLASS_NAME::ConvUCS2ToUTF8Test2()
{
	WCHAR aInput[] = {0x03b8, 0x03c0, 0x0000};
	char aExpect[] = { (char)0xce, (char)0xb8, (char)0xcf, (char)0x80, 0x00};	//  #GREEK SMALL LETTER THETA, #GREEK SMALL LETTER PI

	char *pResult = ConvUCS2ToUTF8(aInput);
	CPPUNIT_ASSERT(strcmp(aExpect, pResult) == 0);
}

void TEST_CLASS_NAME::ConvUCS2ToUTF8Test3()
{
	WCHAR aInput[] = {0x3068, 0x3093, 0x307c, 0x873b, 0x86c9, 0x0000};
	char aExpect[] = {	(char)0xe3, (char)0x81, (char)0xa8, (char)0xe3,	// TOMBO by Hira-gana and Kanji
						(char)0x82, (char)0x93, (char)0xe3, (char)0x81,
						(char)0xbc, (char)0xe8, (char)0x9c, (char)0xbb,
						(char)0xe8, (char)0x9b, (char)0x89, (char)0x00};

	char *pResult = ConvUCS2ToUTF8(aInput);
	CPPUNIT_ASSERT(strcmp(aExpect, pResult) == 0);
}