#include <windows.h>
#include "../stdafx.h"

#include "UniConv.h"
#include "RegexUtil.h"

static TestRunner *runner;

static void RegexTest1();
static void CaseIgnoreTest1();
static void CaseIgnoreTest2();
static void NoMatchTest1();
static void KanjiTest1();
static void KanjiTest2();
static void KanjiTest3();
static void ConvUTF8PosToUCSPosTest1();
static void ConvUTF8PosToUCSPosTest2();
static void ConvUTF8PosToUCSPosTest3();
static void ConvUTF8PosToUCSPosTest4();
static void ConvUCSPosToUTF8PosTest1();
static void ConvUCSPosToUTF8PosTest2();
static void ConvUCSPosToUTF8PosTest3();
static void ConvUCSPosToUTF8PosTest4();
static void ShiftRightTest1();
static void ShiftRightTest2();
static void ShiftRightTest3();
static void ShiftLeftTest1();
static void ShiftLeftTest2();
static void ShiftLeftTest3();
static void UTF8Test1();
static void UTF8Test2();
static void ConvertPosTest1();
static void ConvertPosTest2();
static void ConvertPosTest3();
static void ConvertPosTest4();
static void ConvertPosTest5();
static void ConvertPosTest6();

void RegexTest(TestRunner *r) {
	runner = r;
	runner->WriteMsg("RegexTest\r\n");

	RegexTest1();
	CaseIgnoreTest1();
	CaseIgnoreTest2();
	NoMatchTest1();
	KanjiTest1();
	KanjiTest2();
	KanjiTest3();
	ConvUTF8PosToUCSPosTest1();
	ConvUTF8PosToUCSPosTest2();
	ConvUTF8PosToUCSPosTest3();
	ConvUTF8PosToUCSPosTest4();
	ConvUCSPosToUTF8PosTest1();
	ConvUCSPosToUTF8PosTest2();
	ConvUCSPosToUTF8PosTest3();
	ConvUCSPosToUTF8PosTest4();
	ShiftRightTest1();
	ShiftRightTest2();
	ShiftRightTest3();
	ShiftLeftTest1();
	ShiftLeftTest2();
	ShiftLeftTest3();
	UTF8Test1();
	UTF8Test2();
	ConvertPosTest1();
	ConvertPosTest2();
	ConvertPosTest3();
	ConvertPosTest4();
	ConvertPosTest5();
	ConvertPosTest6();

}

////////////////////////////////////////////////


void RegexTest1() {
	const char *pReason;
	void *pPat = Regex_Compile((LPBYTE)"pat", FALSE, &pReason, 0);
	runner->assert(pPat != NULL);

	const char *pString = "aaa bbb pap pat pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, (LPBYTE)pString, TRUE, &nStart, &nEnd, 0);

	runner->assert(n == 12);
	runner->assert(nStart == 12);
	runner->assert(nEnd == 15);
}

void CaseIgnoreTest1() {
	const char *pReason;
	void *pPat = Regex_Compile((LPBYTE)"PAT", FALSE, &pReason, 0);
	runner->assert(pPat != NULL);

	const char *pString = "aaa bbb pat PAT pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, (LPBYTE)pString, TRUE, &nStart, &nEnd, 0);

	runner->assert(n == 12);
	runner->assert(nStart == 12);
	runner->assert(nEnd == 15);
}


void CaseIgnoreTest2() {
	const char *pReason;
	void *pPat = Regex_Compile((LPBYTE)"PAT", TRUE, &pReason, 0);
	runner->assert(pPat != NULL);

	const char *pString = "aaa bbb pat PAT pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, (LPBYTE)pString, TRUE, &nStart, &nEnd, 0);

	runner->assert(n == 8);
	runner->assert(nStart == 8);
	runner->assert(nEnd == 11);
}

void NoMatchTest1() {
	const char *pReason;
	void *pPat = Regex_Compile((LPBYTE)"pat", FALSE, &pReason, 0);
	runner->assert(pPat != NULL);

	const char *pString = "aaa bbb pap pad pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, (LPBYTE)pString, TRUE, &nStart, &nEnd, 0);

	runner->assert(n == -1);
}

void KanjiTest1() {

	const char *pReason;
	void *pPat = Regex_Compile((LPBYTE)"Š¿š", FALSE, &pReason, 0);
	runner->assert(pPat != NULL);

	char *pString = "aaa Š¿š pap pad pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, (LPBYTE)pString, TRUE, &nStart, &nEnd, 0);

	runner->assert(n == 4);
	runner->assert(nStart == 4);
	runner->assert(nEnd == 8);
}

void KanjiTest2() {

	const char *pReason;
	void *pPat = Regex_Compile((LPBYTE)"Š¿+š", FALSE, &pReason, 0);
	runner->assert(pPat != NULL);

	char *pString = "aŠ¿Š¿š pap pad pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, (LPBYTE)pString, TRUE, &nStart, &nEnd, 0);

	runner->assert(n == 1);
	runner->assert(nStart == 1);
	runner->assert(nEnd==7);
}

void KanjiTest3() {
	const char *pReason;
	void *pPat = Regex_Compile((LPBYTE)"[Š¿š]+", FALSE, &pReason, 0);
	runner->assert(pPat != NULL);

	char *pString = "ƒeƒXƒgaŠ¿Š¿ššŠ¿š pap pad pas";

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, (LPBYTE)pString, TRUE, &nStart, &nEnd, 0);

	runner->assert(n == 7);
	runner->assert(nStart == 7);
	runner->assert(nEnd==19);
}

void ConvUTF8PosToUCSPosTest1() {
	char *pUTF = ConvUCS2ToUTF8(L"abcdefg");
	DWORD n = ConvUTF8PosToUCSPos(pUTF, 3);
	runner->assert(n == 3);
}

void ConvUTF8PosToUCSPosTest2() {
	char *pUTF = ConvUCS2ToUTF8(L"abƒÎƒÓƒÆcdefg");
	DWORD n = ConvUTF8PosToUCSPos(pUTF, 6);
	runner->assert(n == 4);
}

void ConvUTF8PosToUCSPosTest3() {
	char *pUTF = ConvUCS2ToUTF8(L"abƒÎƒÆŠ¿ašcdefg");
	DWORD n = ConvUTF8PosToUCSPos(pUTF, 10);
	runner->assert(n == 6);
}

void ConvUTF8PosToUCSPosTest4() {
	char *pUTF = ConvUCS2ToUTF8(L"abƒÎƒÆŠ¿ašcdefg");
	DWORD n = ConvUTF8PosToUCSPos(pUTF, 18);
	runner->assert(n == 12);
}

void ConvUCSPosToUTF8PosTest1() {
	char *pUTF = ConvUCS2ToUTF8(L"abcdefg");
	DWORD n = ConvUCSPosToUTF8Pos(pUTF, 3);
	runner->assert(n == 3);
}

void ConvUCSPosToUTF8PosTest2() {
	char *pUTF = ConvUCS2ToUTF8(L"abƒÎƒÓƒÆcdefg");
	DWORD n = ConvUCSPosToUTF8Pos(pUTF, 4);
	runner->assert(n == 6);
}

void ConvUCSPosToUTF8PosTest3() {
	char *pUTF = ConvUCS2ToUTF8(L"abƒÎƒÆŠ¿ašcdefg");
	DWORD n = ConvUCSPosToUTF8Pos(pUTF, 6);
	runner->assert(n == 10);
}

void ConvUCSPosToUTF8PosTest4() {
	char *pUTF = ConvUCS2ToUTF8(L"abƒÎƒÆŠ¿ašcdefg");
	DWORD n = ConvUCSPosToUTF8Pos(pUTF, 12);
	runner->assert(n == 18);
}

// Shift right SJIS
void ShiftRightTest1() {
	LPBYTE p0 = (LPBYTE)"abcŠ¿šdef¶ghi";
	LPBYTE p1 = (LPBYTE)ShiftRight(p0, p0 + 3, 0);
	runner->assert(p1 == p0 + 5);
	LPBYTE p2 = (LPBYTE)ShiftRight(p0, p1, 0);
	runner->assert(p2 == p0 + 7);
	LPBYTE p3 = (LPBYTE)ShiftRight(p0, p2, 0);
	runner->assert(p3 == p0 + 8);

}

// Shift right UTF-8
void ShiftRightTest2() {
	LPBYTE p0 = (LPBYTE)ConvUCS2ToUTF8(L"abƒÎƒÆŠ¿ašcdefg");

	LPBYTE p1 = (LPBYTE)ShiftRight(p0, p0 + 1, 65001);
	runner->assert(p1 == p0 + 2);

	LPBYTE p2 = (LPBYTE)ShiftRight(p0, p0 + 2, 65001);
	runner->assert(p2 == p0 + 4);

	LPBYTE p3 = (LPBYTE)ShiftRight(p0, p0 + 6, 65001);
	runner->assert(p3 == p0 + 9);
}

// Shift right UTF-16
void ShiftRightTest3() {
	LPBYTE p0 = (LPBYTE)L"abƒÎƒÆŠ¿ašcdefg";

	LPBYTE p1 = (LPBYTE)ShiftRight(p0, p0 + 1, 1200);
	runner->assert(p1 == p0 + 2);

	LPBYTE p2 = (LPBYTE)ShiftRight(p0, p0 + 2, 1200);
	runner->assert(p2 == p0 + 4);

	LPBYTE p3 = (LPBYTE)ShiftRight(p0, p0 + 6, 1200);
	runner->assert(p3 == p0 + 8);
}

// Shift left SJIS
void ShiftLeftTest1() {
	LPBYTE p0 = (LPBYTE)"abcŠ¿šdef¶ghi";

	LPBYTE p1 = (LPBYTE)ShiftLeft(p0, p0 + 5, 0);
	runner->assert(p1 == p0 + 3);

	LPBYTE p2 = (LPBYTE)ShiftLeft(p0, p0 + 3, 0);
	runner->assert(p2 == p0 + 2);
}

// Shift left UTF-8
void ShiftLeftTest2() {
	LPBYTE p0 = (LPBYTE)ConvUCS2ToUTF8(L"abƒÎƒÆŠ¿ašcdefg");

	LPBYTE p1 = (LPBYTE)ShiftLeft(p0, p0 + 9, 65001);
	runner->assert(p1 == p0 + 6);

	LPBYTE p2 = (LPBYTE)ShiftLeft(p0, p0 + 6, 65001);
	runner->assert(p2 == p0 + 4);

	LPBYTE p3 = (LPBYTE)ShiftLeft(p0, p0 + 2, 65001);
	runner->assert(p3 == p0 + 1);

}

// Shift left UTF-8
void ShiftLeftTest3() {
	LPBYTE p0 = (LPBYTE)L"abƒÎƒÆŠ¿ašcdefg";

	LPBYTE p1 = (LPBYTE)ShiftLeft(p0, p0 + 9, 1200);
	runner->assert(p1 == p0 + 8);

	LPBYTE p2 = (LPBYTE)ShiftLeft(p0, p0 + 6, 1200);
	runner->assert(p2 == p0 + 4);

	LPBYTE p3 = (LPBYTE)ShiftLeft(p0, p0 + 2, 1200);
	runner->assert(p3 == p0 + 0);
}

void UTF8Test1() {

	LPBYTE pPatUTF8 = (LPBYTE)ConvUCS2ToUTF8(L"Š¿š");

	const char *pReason;
	void *pPat = Regex_Compile(pPatUTF8, FALSE, &pReason, 65001);
	runner->assert(pPat != NULL);

	LPBYTE pString = (LPBYTE)ConvUCS2ToUTF8(L"abƒÎƒÆŠ¿šcdefg");

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, pString, TRUE, &nStart, &nEnd, 0);

	runner->assert(n == 6);
	runner->assert(nStart == 6);
	runner->assert(nEnd == 12);
}

void UTF8Test2() {

	LPBYTE pPatUTF8 = (LPBYTE)ConvUCS2ToUTF8(L"bƒÎ");

	const char *pReason;
	void *pPat = Regex_Compile(pPatUTF8, FALSE, &pReason, 65001);
	runner->assert(pPat != NULL);

	LPBYTE pString = (LPBYTE)ConvUCS2ToUTF8(L"abƒÎƒÆŠ¿šcdefg");

	int nStart, nEnd;

	int n = Regex_Search(pPat, 0, pString, TRUE, &nStart, &nEnd, 0);

	runner->assert(n == 1);
	runner->assert(nStart == 1);
	runner->assert(nEnd == 4);
}


// UTF-8 -> Native(zero position)
void ConvertPosTest1() {
	LPBYTE pSrc = (LPBYTE)ConvUCS2ToUTF8(L"Š¿abƒÓš");
	LPBYTE pDst = (LPBYTE)"Š¿abƒÓš";

	DWORD n = ConvertPos(pSrc, 0, 65001, pDst, 0);
	
	runner->assert(n == 0);

}

// UTF-8 -> Native
void ConvertPosTest2() {
	LPBYTE pSrc = (LPBYTE)ConvUCS2ToUTF8(L"Š¿abƒÓš");
	LPBYTE pDst = (LPBYTE)"Š¿abƒÓš";

	DWORD n = ConvertPos(pSrc, 7, 65001, pDst, 0);
	runner->assert(n == 6);

}

// UTF-8 -> UCS2
void ConvertPosTest3() {
	LPBYTE pSrc = (LPBYTE)ConvUCS2ToUTF8(L"Š¿abƒÓš");
	LPBYTE pDst = (LPBYTE)L"Š¿abƒÓš";

	DWORD n = ConvertPos(pSrc, 7, 65001, pDst, 1200);
	// notice result is byte position, so not 4
	runner->assert(n == 8);
}

// Native -> UTF-8
void ConvertPosTest4() {
	LPBYTE pSrc = (LPBYTE)"Š¿abƒÓš";
	LPBYTE pDst = (LPBYTE)ConvUCS2ToUTF8(L"Š¿abƒÓš");

	DWORD n = ConvertPos(pSrc, 6, 0, pDst, 65001);
	runner->assert(n == 7);
}

// Native -> UCS2
void ConvertPosTest5() {
	LPBYTE pSrc = (LPBYTE)"Š¿abƒÓš";
	LPBYTE pDst = (LPBYTE)L"Š¿abƒÓš";

	DWORD n = ConvertPos(pSrc, 6, 0, pDst, 1200);
	runner->assert(n == 8);
}

// UTF-8 -> Native
void ConvertPosTest6() {
	LPBYTE pSrc = (LPBYTE)ConvUCS2ToUTF8(L"Š¿abƒÓš");
	LPBYTE pDst = (LPBYTE)"Š¿abƒÓš";

	DWORD n = ConvertPos(pSrc, 10, 65001, pDst, 0);
	runner->assert(n == 8);

}
