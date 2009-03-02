#include <windows.h>
#include "../stdafx.h"

#include "Uniconv.h"

static TestRunner *runner;

static void Base64EncodeTest1();
static void Base64EncodeTest2();
static void Base64EncodeTest3();
static void Base64EncodeTest4_1();
static void Base64EncodeTest4_2();
static void Base64EncodeTest4_3();
static void Base64EncodeTest4_4();
static void Base64EncodeTest4_5();
static void Base64EncodeTest4_6();
static void Base64EncodeTest4_7();
static void Base64DecodeTest1();
static void Base64DecodeTest2();
static void ConvUTF8ToUCS2Test1();
static void ConvUTF8ToUCS2Test2();
static void ConvUTF8ToUCS2Test3();
static void ConvUCS2ToUTF8Test1();
static void ConvUCS2ToUTF8Test2();
static void ConvUCS2ToUTF8Test3();

void UniconvTest(TestRunner *r) {
	runner = r;
	runner->WriteMsg("UniconvTest\r\n");

	Base64EncodeTest1();
	Base64EncodeTest2();
	Base64EncodeTest3();
	Base64EncodeTest4_1();
	Base64EncodeTest4_2();
	Base64EncodeTest4_3();
	Base64EncodeTest4_4();
	Base64EncodeTest4_5();
	Base64EncodeTest4_6();
	Base64EncodeTest4_7();
	Base64DecodeTest1();
	Base64DecodeTest2();
	ConvUTF8ToUCS2Test1();
	ConvUTF8ToUCS2Test2();
	ConvUTF8ToUCS2Test3();
	ConvUCS2ToUTF8Test1();
	ConvUCS2ToUTF8Test2();
	ConvUCS2ToUTF8Test3();
}

// test probe
extern DWORD g_Base64EncodeAllocSize;

////////////////////////////////////////////////

void Base64EncodeTest1() {
	char *pOut = Base64Encode((LPBYTE)"Hello", 5);

	runner->assert(g_Base64EncodeAllocSize == 9);
	runner->assert(strcmp("SGVsbG8=", pOut) == 0);
	delete [] pOut;
}

void Base64EncodeTest2() {
	char *pOut = Base64Encode(NULL, 5);
	runner->assert(pOut == NULL);
}

void Base64EncodeTest3() {
	char *pOut = Base64Encode((LPBYTE)"Hello", 0);
	runner->assert(pOut == NULL);
}

void Base64EncodeTest4_1()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 1);

	runner->assert(g_Base64EncodeAllocSize == 5);
	runner->assert(strcmp("SA==", pOut) == 0);
	delete [] pOut;
}

void Base64EncodeTest4_2()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 2);

	runner->assert(g_Base64EncodeAllocSize == 5);
	runner->assert(strcmp("SGU=", pOut) == 0);
	delete [] pOut;
}

void Base64EncodeTest4_3()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 3);

	runner->assert(g_Base64EncodeAllocSize == 5);
	runner->assert(strcmp("SGVs", pOut) == 0);
	delete [] pOut;
}

void Base64EncodeTest4_4()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 4);

	runner->assert(g_Base64EncodeAllocSize == 9);
	runner->assert(strcmp("SGVsbA==", pOut) == 0);
	delete [] pOut;
}

void Base64EncodeTest4_5()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 5);

	runner->assert(g_Base64EncodeAllocSize == 9);
	runner->assert(strcmp("SGVsbG8=", pOut) == 0);
	delete [] pOut;
}

void Base64EncodeTest4_6()
{
	char *pOut = Base64Encode((LPBYTE)"Hello ", 6);

	runner->assert(g_Base64EncodeAllocSize == 9);
	runner->assert(strcmp("SGVsbG8g", pOut) == 0);
	delete [] pOut;
}

void Base64EncodeTest4_7()
{
	char *pOut = Base64Encode((LPBYTE)"Hello w", 7);

	runner->assert(g_Base64EncodeAllocSize == 13);
	runner->assert(strcmp("SGVsbG8gdw==", pOut) == 0);
	delete [] pOut;
}

void Base64DecodeTest1()
{
	DWORD n;
	LPBYTE p = Base64Decode("SGVsbG8=", &n);
	runner->assert(n == 5);
	runner->assert(strncmp((char*)p, "Hello", 5) == 0);
	delete [] p;
}

void Base64DecodeTest2()
{
	DWORD n;
	LPBYTE p = Base64Decode("SGVsbG8", &n);
	runner->assert(p == NULL);
}


// UTF-8 1 byte conversion
void ConvUTF8ToUCS2Test1()
{
	char aInput[] = { 0x54, 0x45, 0x53, 0x54, 0x00};	// "TEST"
	WCHAR aExpect[] = {0x0054, 0x0045, 0x0053, 0x0054, 0x0000};

	LPWSTR pResult = ConvUTF8ToUCS2(aInput);
	runner->assert(wcscmp(aExpect, pResult) == 0);
}

// UTF-8 2 byte conversion
void ConvUTF8ToUCS2Test2()
{
	char aInput[] = { (char)0xce, (char)0xb8, (char)0xcf, (char)0x80, 0x00};	//  #GREEK SMALL LETTER THETA, #GREEK SMALL LETTER PI
	WCHAR aExpect[] = {0x03b8, 0x03c0, 0x0000};

	LPWSTR pResult = ConvUTF8ToUCS2(aInput);
	runner->assert(wcscmp(aExpect, pResult) == 0);
}

// UTF-8 3 byte conversion
void ConvUTF8ToUCS2Test3()
{
	char aInput[] = {	(char)0xe3, (char)0x81, (char)0xa8, (char)0xe3,	// TOMBO by Hira-gana and Kanji
						(char)0x82, (char)0x93, (char)0xe3, (char)0x81,
						(char)0xbc, (char)0xe8, (char)0x9c, (char)0xbb,
						(char)0xe8, (char)0x9b, (char)0x89, (char)0x00};
	WCHAR aExpect[] = {0x3068, 0x3093, 0x307c, 0x873b, 0x86c9, 0x0000};

	LPWSTR pResult = ConvUTF8ToUCS2(aInput);
	runner->assert(wcscmp(aExpect, pResult) == 0);
}

void ConvUCS2ToUTF8Test1()
{
	WCHAR aInput[] = {0x0054, 0x0045, 0x0053, 0x0054, 0x0000};	// "TEST"
	char aExpect[] = { 0x54, 0x45, 0x53, 0x54, 0x00};

	char *pResult = ConvUCS2ToUTF8(aInput);
	runner->assert(strcmp(aExpect, pResult) == 0);
}

void ConvUCS2ToUTF8Test2()
{
	WCHAR aInput[] = {0x03b8, 0x03c0, 0x0000};
	char aExpect[] = { (char)0xce, (char)0xb8, (char)0xcf, (char)0x80, 0x00};	//  #GREEK SMALL LETTER THETA, #GREEK SMALL LETTER PI

	char *pResult = ConvUCS2ToUTF8(aInput);
	runner->assert(strcmp(aExpect, pResult) == 0);
}

void ConvUCS2ToUTF8Test3()
{
	WCHAR aInput[] = {0x3068, 0x3093, 0x307c, 0x873b, 0x86c9, 0x0000};
	char aExpect[] = {	(char)0xe3, (char)0x81, (char)0xa8, (char)0xe3,	// TOMBO by Hira-gana and Kanji
						(char)0x82, (char)0x93, (char)0xe3, (char)0x81,
						(char)0xbc, (char)0xe8, (char)0x9c, (char)0xbb,
						(char)0xe8, (char)0x9b, (char)0x89, (char)0x00};

	char *pResult = ConvUCS2ToUTF8(aInput);
	runner->assert(strcmp(aExpect, pResult) == 0);
}
