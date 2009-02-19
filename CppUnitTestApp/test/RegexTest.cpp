#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>
#include <windows.h>

// These test cases assume default encoding as CP932(SJIS).

#include "UniConv.h"

#include "RegexUtil.h"
#ifdef COMMENT
extern "C" {
void* Regex_Compile(const LPBYTE pPattern, BOOL bIgnoreCase, const char **ppReason, DWORD nCodePage);
void Regex_Free(void *p);
int Regex_Search(void *p, int iStart, const LPBYTE pTarget, BOOL bForward, int *pStart, int *pEnd, DWORD nCodePage);

const LPBYTE ShiftLeft(const LPBYTE pString, const LPBYTE pPos, DWORD nCodePage);
const LPBYTE ShiftRight(const LPBYTE pString, const LPBYTE pPos, DWORD nCodePage);

DWORD UnicodePosToMBCSPos(const char *pStr, DWORD n, DWORD nCodePage);
DWORD FileEncPosToUnicodePos(const char *pStr, DWORD n, DWORD nCodePage);
};
#endif

#define TEST_CLASS_NAME RegexTest

class TEST_CLASS_NAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TEST_CLASS_NAME);

	CPPUNIT_TEST(RegexTest1);

	CPPUNIT_TEST(CaseIgnoreTest1);
	CPPUNIT_TEST(CaseIgnoreTest2);

	CPPUNIT_TEST(NoMatchTest1);
	
	CPPUNIT_TEST(KanjiTest1);
	CPPUNIT_TEST(KanjiTest2);
	CPPUNIT_TEST(KanjiTest3);
	
	CPPUNIT_TEST(ConvUTF8PosToUCSPosTest1);
	CPPUNIT_TEST(ConvUTF8PosToUCSPosTest2);
	CPPUNIT_TEST(ConvUTF8PosToUCSPosTest3);
	CPPUNIT_TEST(ConvUTF8PosToUCSPosTest4);

	CPPUNIT_TEST(ConvUCSPosToUTF8PosTest1);
	CPPUNIT_TEST(ConvUCSPosToUTF8PosTest2);
	CPPUNIT_TEST(ConvUCSPosToUTF8PosTest3);
	CPPUNIT_TEST(ConvUCSPosToUTF8PosTest4);

	CPPUNIT_TEST(ShiftRightTest1);
	CPPUNIT_TEST(ShiftRightTest2);
	CPPUNIT_TEST(ShiftRightTest3);

	CPPUNIT_TEST(ShiftLeftTest1);
	CPPUNIT_TEST(ShiftLeftTest2);
	CPPUNIT_TEST(ShiftLeftTest3);

	CPPUNIT_TEST(UTF8Test1);
	CPPUNIT_TEST(UTF8Test2);

	CPPUNIT_TEST(ConvertPosTest1);
	CPPUNIT_TEST(ConvertPosTest2);
	CPPUNIT_TEST(ConvertPosTest3);
	CPPUNIT_TEST(ConvertPosTest4);
	CPPUNIT_TEST(ConvertPosTest5);
	CPPUNIT_TEST(ConvertPosTest6);

	CPPUNIT_TEST_SUITE_END();


public:
	TEST_CLASS_NAME() {}
	~TEST_CLASS_NAME() {}

	virtual void setUp() {}
	virtual void tearDown() {}

	void RegexTest1();

	void CaseIgnoreTest1();
	void CaseIgnoreTest2();

	void NoMatchTest1();

	void KanjiTest1();
	void KanjiTest2();
	void KanjiTest3();

	void ConvUTF8PosToUCSPosTest1();
	void ConvUTF8PosToUCSPosTest2();
	void ConvUTF8PosToUCSPosTest3();
	void ConvUTF8PosToUCSPosTest4();

	void ConvUCSPosToUTF8PosTest1();
	void ConvUCSPosToUTF8PosTest2();
	void ConvUCSPosToUTF8PosTest3();
	void ConvUCSPosToUTF8PosTest4();

	void ShiftRightTest1();
	void ShiftRightTest2();
	void ShiftRightTest3();

	void ShiftLeftTest1();
	void ShiftLeftTest2();
	void ShiftLeftTest3();

	void UTF8Test1();
	void UTF8Test2();

	void ConvertPosTest1();
	void ConvertPosTest2();
	void ConvertPosTest3();
	void ConvertPosTest4();
	void ConvertPosTest5();
	void ConvertPosTest6();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TEST_CLASS_NAME);

////////////////////////////////////////////////


void TEST_CLASS_NAME::RegexTest1() {
	const char *pReason;
	void *pPat = Regex_Compile((LPBYTE)"pat", FALSE, &pReason, 0);
	CPPUNIT_ASSERT(pPat != NULL);

	const char *pString = "aaa bbb pap pat pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, (LPBYTE)pString, TRUE, &nStart, &nEnd, 0);

	CPPUNIT_ASSERT(n == 12);
	CPPUNIT_ASSERT(nStart == 12);
	CPPUNIT_ASSERT(nEnd == 15);
}

void TEST_CLASS_NAME::CaseIgnoreTest1() {
	const char *pReason;
	void *pPat = Regex_Compile((LPBYTE)"PAT", FALSE, &pReason, 0);
	CPPUNIT_ASSERT(pPat != NULL);

	const char *pString = "aaa bbb pat PAT pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, (LPBYTE)pString, TRUE, &nStart, &nEnd, 0);

	CPPUNIT_ASSERT(n == 12);
	CPPUNIT_ASSERT(nStart == 12);
	CPPUNIT_ASSERT(nEnd == 15);
}


void TEST_CLASS_NAME::CaseIgnoreTest2() {
	const char *pReason;
	void *pPat = Regex_Compile((LPBYTE)"PAT", TRUE, &pReason, 0);
	CPPUNIT_ASSERT(pPat != NULL);

	const char *pString = "aaa bbb pat PAT pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, (LPBYTE)pString, TRUE, &nStart, &nEnd, 0);

	CPPUNIT_ASSERT(n == 8);
	CPPUNIT_ASSERT(nStart == 8);
	CPPUNIT_ASSERT(nEnd == 11);
}

void TEST_CLASS_NAME::NoMatchTest1() {
	const char *pReason;
	void *pPat = Regex_Compile((LPBYTE)"pat", FALSE, &pReason, 0);
	CPPUNIT_ASSERT(pPat != NULL);

	const char *pString = "aaa bbb pap pad pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, (LPBYTE)pString, TRUE, &nStart, &nEnd, 0);

	CPPUNIT_ASSERT(n == -1);
}

void TEST_CLASS_NAME::KanjiTest1() {

	const char *pReason;
	void *pPat = Regex_Compile((LPBYTE)"Š¿Žš", FALSE, &pReason, 0);
	CPPUNIT_ASSERT(pPat != NULL);

	char *pString = "aaa Š¿Žš pap pad pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, (LPBYTE)pString, TRUE, &nStart, &nEnd, 0);

	CPPUNIT_ASSERT(n == 4);
	CPPUNIT_ASSERT(nStart == 4);
	CPPUNIT_ASSERT(nEnd == 8);
}

void TEST_CLASS_NAME::KanjiTest2() {

	const char *pReason;
	void *pPat = Regex_Compile((LPBYTE)"Š¿+Žš", FALSE, &pReason, 0);
	CPPUNIT_ASSERT(pPat != NULL);

	char *pString = "aŠ¿Š¿Žš pap pad pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, (LPBYTE)pString, TRUE, &nStart, &nEnd, 0);

	CPPUNIT_ASSERT(n == 1);
	CPPUNIT_ASSERT(nStart == 1);
	CPPUNIT_ASSERT(nEnd==7);
}

void TEST_CLASS_NAME::KanjiTest3() {
	const char *pReason;
	void *pPat = Regex_Compile((LPBYTE)"[Š¿Žš]+", FALSE, &pReason, 0);
	CPPUNIT_ASSERT(pPat != NULL);

	char *pString = "ƒeƒXƒgaŠ¿Š¿ŽšŽšŠ¿Žš pap pad pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, (LPBYTE)pString, TRUE, &nStart, &nEnd, 0);

	CPPUNIT_ASSERT(n == 7);
	CPPUNIT_ASSERT(nStart == 7);
	CPPUNIT_ASSERT(nEnd==19);
}

void TEST_CLASS_NAME::ConvUTF8PosToUCSPosTest1() {
	char *pUTF = ConvUCS2ToUTF8(_L("abcdefg"));
	DWORD n = ConvUTF8PosToUCSPos(pUTF, 3);
	CPPUNIT_ASSERT(n == 3);
}

void TEST_CLASS_NAME::ConvUTF8PosToUCSPosTest2() {
	char *pUTF = ConvUCS2ToUTF8(_L("abƒÎƒÓƒÆcdefg"));
	DWORD n = ConvUTF8PosToUCSPos(pUTF, 6);
	CPPUNIT_ASSERT(n == 4);
}

void TEST_CLASS_NAME::ConvUTF8PosToUCSPosTest3() {
	char *pUTF = ConvUCS2ToUTF8(_L("abƒÎƒÆŠ¿aŽšcdefg"));
	DWORD n = ConvUTF8PosToUCSPos(pUTF, 10);
	CPPUNIT_ASSERT(n == 6);
}

void TEST_CLASS_NAME::ConvUTF8PosToUCSPosTest4() {
	char *pUTF = ConvUCS2ToUTF8(_L("abƒÎƒÆŠ¿aŽšcdefg"));
	DWORD n = ConvUTF8PosToUCSPos(pUTF, 18);
	CPPUNIT_ASSERT(n == 12);
}

void TEST_CLASS_NAME::ConvUCSPosToUTF8PosTest1() {
	char *pUTF = ConvUCS2ToUTF8(_L("abcdefg"));
	DWORD n = ConvUCSPosToUTF8Pos(pUTF, 3);
	CPPUNIT_ASSERT(n == 3);
}

void TEST_CLASS_NAME::ConvUCSPosToUTF8PosTest2() {
	char *pUTF = ConvUCS2ToUTF8(_L("abƒÎƒÓƒÆcdefg"));
	DWORD n = ConvUCSPosToUTF8Pos(pUTF, 4);
	CPPUNIT_ASSERT(n == 6);
}

void TEST_CLASS_NAME::ConvUCSPosToUTF8PosTest3() {
	char *pUTF = ConvUCS2ToUTF8(_L("abƒÎƒÆŠ¿aŽšcdefg"));
	DWORD n = ConvUCSPosToUTF8Pos(pUTF, 6);
	CPPUNIT_ASSERT(n == 10);
}

void TEST_CLASS_NAME::ConvUCSPosToUTF8PosTest4() {
	char *pUTF = ConvUCS2ToUTF8(_L("abƒÎƒÆŠ¿aŽšcdefg"));
	DWORD n = ConvUCSPosToUTF8Pos(pUTF, 12);
	CPPUNIT_ASSERT(n == 18);
}

// Shift right SJIS
void TEST_CLASS_NAME::ShiftRightTest1() {
	LPBYTE p0 = (LPBYTE)"abcŠ¿Žšdef¶ghi";
	LPBYTE p1 = (LPBYTE)ShiftRight(p0, p0 + 3, 0);
	CPPUNIT_ASSERT(p1 == p0 + 5);
	LPBYTE p2 = (LPBYTE)ShiftRight(p0, p1, 0);
	CPPUNIT_ASSERT(p2 == p0 + 7);
	LPBYTE p3 = (LPBYTE)ShiftRight(p0, p2, 0);
	CPPUNIT_ASSERT(p3 == p0 + 8);

}

// Shift right UTF-8
void TEST_CLASS_NAME::ShiftRightTest2() {
	LPBYTE p0 = (LPBYTE)ConvUCS2ToUTF8(_L("abƒÎƒÆŠ¿aŽšcdefg"));

	LPBYTE p1 = (LPBYTE)ShiftRight(p0, p0 + 1, 65001);
	CPPUNIT_ASSERT(p1 == p0 + 2);

	LPBYTE p2 = (LPBYTE)ShiftRight(p0, p0 + 2, 65001);
	CPPUNIT_ASSERT(p2 == p0 + 4);

	LPBYTE p3 = (LPBYTE)ShiftRight(p0, p0 + 6, 65001);
	CPPUNIT_ASSERT(p3 == p0 + 9);
}

// Shift right UTF-16
void TEST_CLASS_NAME::ShiftRightTest3() {
	LPBYTE p0 = (LPBYTE)_L("abƒÎƒÆŠ¿aŽšcdefg");

	LPBYTE p1 = (LPBYTE)ShiftRight(p0, p0 + 1, 1200);
	CPPUNIT_ASSERT(p1 == p0 + 2);

	LPBYTE p2 = (LPBYTE)ShiftRight(p0, p0 + 2, 1200);
	CPPUNIT_ASSERT(p2 == p0 + 4);

	LPBYTE p3 = (LPBYTE)ShiftRight(p0, p0 + 6, 1200);
	CPPUNIT_ASSERT(p3 == p0 + 8);
}

// Shift left SJIS
void TEST_CLASS_NAME::ShiftLeftTest1() {
	LPBYTE p0 = (LPBYTE)"abcŠ¿Žšdef¶ghi";

	LPBYTE p1 = (LPBYTE)ShiftLeft(p0, p0 + 5, 0);
	CPPUNIT_ASSERT(p1 == p0 + 3);

	LPBYTE p2 = (LPBYTE)ShiftLeft(p0, p0 + 3, 0);
	CPPUNIT_ASSERT(p2 == p0 + 2);
}

// Shift left UTF-8
void TEST_CLASS_NAME::ShiftLeftTest2() {
	LPBYTE p0 = (LPBYTE)ConvUCS2ToUTF8(_L("abƒÎƒÆŠ¿aŽšcdefg"));

	LPBYTE p1 = (LPBYTE)ShiftLeft(p0, p0 + 9, 65001);
	CPPUNIT_ASSERT(p1 == p0 + 6);

	LPBYTE p2 = (LPBYTE)ShiftLeft(p0, p0 + 6, 65001);
	CPPUNIT_ASSERT(p2 == p0 + 4);

	LPBYTE p3 = (LPBYTE)ShiftLeft(p0, p0 + 2, 65001);
	CPPUNIT_ASSERT(p3 == p0 + 1);

}

// Shift left UTF-8
void TEST_CLASS_NAME::ShiftLeftTest3() {
	LPBYTE p0 = (LPBYTE)_L("abƒÎƒÆŠ¿aŽšcdefg");

	LPBYTE p1 = (LPBYTE)ShiftLeft(p0, p0 + 9, 1200);
	CPPUNIT_ASSERT(p1 == p0 + 8);

	LPBYTE p2 = (LPBYTE)ShiftLeft(p0, p0 + 6, 1200);
	CPPUNIT_ASSERT(p2 == p0 + 4);

	LPBYTE p3 = (LPBYTE)ShiftLeft(p0, p0 + 2, 1200);
	CPPUNIT_ASSERT(p3 == p0 + 0);
}

void TEST_CLASS_NAME::UTF8Test1() {

	LPBYTE pPatUTF8 = (LPBYTE)ConvUCS2ToUTF8(_L("Š¿Žš"));

	const char *pReason;
	void *pPat = Regex_Compile(pPatUTF8, FALSE, &pReason, 65001);
	CPPUNIT_ASSERT(pPat != NULL);

	LPBYTE pString = (LPBYTE)ConvUCS2ToUTF8(_L("abƒÎƒÆŠ¿Žšcdefg"));

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, pString, TRUE, &nStart, &nEnd, 0);

	CPPUNIT_ASSERT(n == 6);
	CPPUNIT_ASSERT(nStart == 6);
	CPPUNIT_ASSERT(nEnd == 12);
}

void TEST_CLASS_NAME::UTF8Test2() {

	LPBYTE pPatUTF8 = (LPBYTE)ConvUCS2ToUTF8(_L("bƒÎ"));

	const char *pReason;
	void *pPat = Regex_Compile(pPatUTF8, FALSE, &pReason, 65001);
	CPPUNIT_ASSERT(pPat != NULL);

	LPBYTE pString = (LPBYTE)ConvUCS2ToUTF8(_L("abƒÎƒÆŠ¿Žšcdefg"));

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, pString, TRUE, &nStart, &nEnd, 0);

	CPPUNIT_ASSERT(n == 1);
	CPPUNIT_ASSERT(nStart == 1);
	CPPUNIT_ASSERT(nEnd == 4);
}


// UTF-8 -> Native(zero position)
void TEST_CLASS_NAME::ConvertPosTest1() {
	LPBYTE pSrc = (LPBYTE)ConvUCS2ToUTF8(_L("Š¿abƒÓŽš"));
	LPBYTE pDst = (LPBYTE)"Š¿abƒÓŽš";

	DWORD n = ConvertPos(pSrc, 0, 65001, pDst, 0);
	
	CPPUNIT_ASSERT(n == 0);

}

// UTF-8 -> Native
void TEST_CLASS_NAME::ConvertPosTest2() {
	LPBYTE pSrc = (LPBYTE)ConvUCS2ToUTF8(_L("Š¿abƒÓŽš"));
	LPBYTE pDst = (LPBYTE)"Š¿abƒÓŽš";

	DWORD n = ConvertPos(pSrc, 7, 65001, pDst, 0);
	CPPUNIT_ASSERT(n == 6);

}

// UTF-8 -> UCS2
void TEST_CLASS_NAME::ConvertPosTest3() {
	LPBYTE pSrc = (LPBYTE)ConvUCS2ToUTF8(_L("Š¿abƒÓŽš"));
	LPBYTE pDst = (LPBYTE)L"Š¿abƒÓŽš";

	DWORD n = ConvertPos(pSrc, 7, 65001, pDst, 1200);
	// notice result is byte position, so not 4
	CPPUNIT_ASSERT(n == 8);
}

// Native -> UTF-8
void TEST_CLASS_NAME::ConvertPosTest4() {
	LPBYTE pSrc = (LPBYTE)"Š¿abƒÓŽš";
	LPBYTE pDst = (LPBYTE)ConvUCS2ToUTF8(_L("Š¿abƒÓŽš"));

	DWORD n = ConvertPos(pSrc, 6, 0, pDst, 65001);
	CPPUNIT_ASSERT(n == 7);
}

// Native -> UCS2
void TEST_CLASS_NAME::ConvertPosTest5() {
	LPBYTE pSrc = (LPBYTE)"Š¿abƒÓŽš";
	LPBYTE pDst = (LPBYTE)L"Š¿abƒÓŽš";

	DWORD n = ConvertPos(pSrc, 6, 0, pDst, 1200);
	CPPUNIT_ASSERT(n == 8);
}

// UTF-8 -> Native
void TEST_CLASS_NAME::ConvertPosTest6() {
	LPBYTE pSrc = (LPBYTE)ConvUCS2ToUTF8(_L("Š¿abƒÓŽš"));
	LPBYTE pDst = (LPBYTE)"Š¿abƒÓŽš";

	DWORD n = ConvertPos(pSrc, 10, 65001, pDst, 0);
	CPPUNIT_ASSERT(n == 8);

}
