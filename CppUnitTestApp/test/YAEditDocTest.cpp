#include <windows.h>
#include <tchar.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include "VarBuffer.h"
#include "Region.h"
#include "YAEditDoc.h"
#include "PhysicalLineManager.h"
#include "MemManager.h"

#define TEST_CLASS_NAME YAEditDocTest


class TEST_CLASS_NAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TEST_CLASS_NAME);
	CPPUNIT_TEST(LoadTest1);
	CPPUNIT_TEST(LoadTest2);
	CPPUNIT_TEST(LoadTest3);
	CPPUNIT_TEST(LoadTest4);
	CPPUNIT_TEST(LoadTest5);
	CPPUNIT_TEST(LoadTest6);
	CPPUNIT_TEST(LoadTest7);
	CPPUNIT_TEST(LoadTest8);

	CPPUNIT_TEST(ConvertBytesToCoordinateTest1);
	CPPUNIT_TEST(ConvertBytesToCoordinateTest2);

	CPPUNIT_TEST(UndoTest1);
	CPPUNIT_TEST(UndoTest2);
	CPPUNIT_TEST(UndoTest3);

	CPPUNIT_TEST_SUITE_END();

public:
	TEST_CLASS_NAME() {}
	~TEST_CLASS_NAME() {}

	virtual void setUp() {}
	virtual void tearDown() {}

	void LoadTest1();
	void LoadTest2();
	void LoadTest3();
	void LoadTest4();
	void LoadTest5();
	void LoadTest6();
	void LoadTest7();
	void LoadTest8();

	void ConvertBytesToCoordinateTest1();
	void ConvertBytesToCoordinateTest2();

	void UndoTest1();
	void UndoTest2();
	void UndoTest3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TEST_CLASS_NAME);

////////////////////////////////////////////////

// empty string
void TEST_CLASS_NAME::LoadTest1() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT(""), NULL, NULL);
	CPPUNIT_ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	CPPUNIT_ASSERT(pLM->MaxLine() == 1);
	LineInfo *pLi = pLM->GetLineInfo(0);

	CPPUNIT_ASSERT(pLi->pLine->nUsed == 0);
}

// 1 line string without CRLF
void TEST_CLASS_NAME::LoadTest2() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("Hello world"), NULL, NULL);
	CPPUNIT_ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	CPPUNIT_ASSERT(pLM->MaxLine() == 1);
	LineInfo *pLi = pLM->GetLineInfo(0);

	CPPUNIT_ASSERT(pLi->pLine->nUsed == 11);
}

// 2 line string
void TEST_CLASS_NAME::LoadTest3() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("Hellow\r\nworld"), NULL, NULL);
	CPPUNIT_ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	CPPUNIT_ASSERT(pLM->MaxLine() == 2);
	LineInfo *pLi0 = pLM->GetLineInfo(0);
	CPPUNIT_ASSERT(pLi0->pLine->nUsed == 6);

	LPCTSTR p0 = pLM->GetLine(0);
	CPPUNIT_ASSERT(_tcsncmp(p0, TEXT("Hellow"), 6) == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	CPPUNIT_ASSERT(pLi1->pLine->nUsed == 5);
	LPCTSTR p1 = pLM->GetLine(1);
	CPPUNIT_ASSERT(_tcsncmp(p1, TEXT("world"), 5) == 0);	
}

// 2 line string end with CRLF
void TEST_CLASS_NAME::LoadTest4() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("Hello\r\n"), NULL, NULL);
	CPPUNIT_ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	CPPUNIT_ASSERT(pLM->MaxLine() == 2);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	CPPUNIT_ASSERT(pLi0->pLine->nUsed == 5);

	LPCTSTR p0 = pLM->GetLine(0);
	CPPUNIT_ASSERT(_tcsncmp(p0, TEXT("Hello"), 5) == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	CPPUNIT_ASSERT(pLi1->pLine->nUsed == 0);
}

// 2 line string start with CRLF
void TEST_CLASS_NAME::LoadTest5() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("\r\nHello"), NULL, NULL);

	CPPUNIT_ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	CPPUNIT_ASSERT(pLM->MaxLine() == 2);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	CPPUNIT_ASSERT(pLi0->pLine->nUsed == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	CPPUNIT_ASSERT(pLi1->pLine->nUsed == 5);

	LPCTSTR p1 = pLM->GetLine(1);
	CPPUNIT_ASSERT(_tcsncmp(p1, TEXT("Hello"), 5) == 0);

}

// CRLF
// CRLF
// CRLF
// EOF
void TEST_CLASS_NAME::LoadTest6() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("\r\n\r\n\r\n"), NULL, NULL);

	CPPUNIT_ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	CPPUNIT_ASSERT(pLM->MaxLine() == 4);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	CPPUNIT_ASSERT(pLi0->pLine->nUsed == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	CPPUNIT_ASSERT(pLi1->pLine->nUsed == 0);

	LineInfo *pLi2 = pLM->GetLineInfo(2);
	CPPUNIT_ASSERT(pLi2->pLine->nUsed == 0);

	LineInfo *pLi3 = pLM->GetLineInfo(3);
	CPPUNIT_ASSERT(pLi3->pLine->nUsed == 0);

}

// CRLF
// CRLF
// abcCRLF
// CRLF
// CRLF
// EOF
void TEST_CLASS_NAME::LoadTest7() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("\r\n\r\nabc\r\n\r\n\r\n"), NULL, NULL);

	CPPUNIT_ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	CPPUNIT_ASSERT(pLM->MaxLine() == 6);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	CPPUNIT_ASSERT(pLi0->pLine->nUsed == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	CPPUNIT_ASSERT(pLi1->pLine->nUsed == 0);

	LineInfo *pLi2 = pLM->GetLineInfo(2);
	CPPUNIT_ASSERT(pLi2->pLine->nUsed == 3);
	LPCTSTR p2 = pLM->GetLine(2);
	CPPUNIT_ASSERT(_tcsncmp(p2, TEXT("abc"), 3) == 0);


	LineInfo *pLi3 = pLM->GetLineInfo(3);
	CPPUNIT_ASSERT(pLi3->pLine->nUsed == 0);

	LineInfo *pLi4 = pLM->GetLineInfo(4);
	CPPUNIT_ASSERT(pLi4->pLine->nUsed == 0);

	LineInfo *pLi5 = pLM->GetLineInfo(5);
	CPPUNIT_ASSERT(pLi5->pLine->nUsed == 0);

}

// abcCRLF
// defCRLF
// ghi[EOF]
void TEST_CLASS_NAME::LoadTest8() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("abc\r\ndef\r\nghi"), NULL, NULL);

	CPPUNIT_ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	CPPUNIT_ASSERT(pLM->MaxLine() == 3);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	CPPUNIT_ASSERT(pLi0->pLine->nUsed == 3);
	LPCTSTR p0 = pLM->GetLine(0);
	CPPUNIT_ASSERT(_tcsncmp(p0, TEXT("abc"), 3) == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	CPPUNIT_ASSERT(pLi1->pLine->nUsed == 3);
	LPCTSTR p1 = pLM->GetLine(1);
	CPPUNIT_ASSERT(_tcsncmp(p1, TEXT("def"), 3) == 0);

	LineInfo *pLi2 = pLM->GetLineInfo(2);
	CPPUNIT_ASSERT(pLi2->pLine->nUsed == 3);
	LPCTSTR p2 = pLM->GetLine(2);
	CPPUNIT_ASSERT(_tcsncmp(p2, TEXT("ghi"), 3) == 0);
}

// abcCRLF
// defgCRLF
// hijklEOF
void TEST_CLASS_NAME::ConvertBytesToCoordinateTest1()
{
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("abc\r\ndefg\r\nhijkl"), NULL, NULL);

	Coordinate pos;
	pDoc->ConvertBytesToCoordinate(0, &pos);
	CPPUNIT_ASSERT(pos.row == 0 && pos.col == 0);

	pDoc->ConvertBytesToCoordinate(2, &pos);
	CPPUNIT_ASSERT(pos.row == 0 && pos.col == 2);

	pDoc->ConvertBytesToCoordinate(5, &pos);
	CPPUNIT_ASSERT(pos.row == 1 && pos.col == 0);

	pDoc->ConvertBytesToCoordinate(6, &pos);
	CPPUNIT_ASSERT(pos.row == 1 && pos.col == 1);

	pDoc->ConvertBytesToCoordinate(9, &pos);
	CPPUNIT_ASSERT(pos.row == 1 && pos.col == 4);

	pDoc->ConvertBytesToCoordinate(11, &pos);
	CPPUNIT_ASSERT(pos.row == 2 && pos.col == 0);

	pDoc->ConvertBytesToCoordinate(16, &pos);
	CPPUNIT_ASSERT(pos.row == 2 && pos.col == 5);

	pDoc->ConvertBytesToCoordinate(100, &pos);
	CPPUNIT_ASSERT(pos.row == 2 && pos.col == 5);
}

void TEST_CLASS_NAME::ConvertBytesToCoordinateTest2()
{
	YAEditDoc *pDoc = new YAEditDoc();

	Coordinate pos;

	BOOL bResult = pDoc->Init(TEXT("TOMBO 1.16\r\n"), NULL, NULL);
	pDoc->ConvertBytesToCoordinate(11, &pos);

	CPPUNIT_ASSERT(pos.row == 1 && pos.col == 0);
}

// initial state
void TEST_CLASS_NAME::UndoTest1()
{
	YAEditDoc *pDoc = new YAEditDoc();

	BOOL bResult = pDoc->Init(TEXT("a"), NULL, NULL);
	CPPUNIT_ASSERT(bResult);

	Region r0(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

	Region r = pDoc->GetUndoRegion();
	LPCTSTR p = pDoc->GetUndoStr();

	CPPUNIT_ASSERT(r0 == r && p == NULL);
}

void TEST_CLASS_NAME::UndoTest2()
{
	YAEditDoc *pDoc = new YAEditDoc();

	BOOL bResult = pDoc->Init(TEXT("a"), NULL, NULL);
	CPPUNIT_ASSERT(bResult);

	Region rReplace(1, 0, 1, 0);
	bResult = pDoc->ReplaceString(&rReplace, TEXT("bcd"));
	CPPUNIT_ASSERT(bResult);
	// abcd I

	Region r = pDoc->GetUndoRegion();
	LPCTSTR p = pDoc->GetUndoStr();

	Region rExpected(1, 0, 4, 0);
	LPCTSTR pExpected = TEXT("");
	CPPUNIT_ASSERT(r == rExpected);
	CPPUNIT_ASSERT(_tcscmp(p, pExpected) == 0);

	// Undo
	bResult = pDoc->Undo();
	CPPUNIT_ASSERT(bResult);

	DWORD nLen;
	LPTSTR pUndo1 = pDoc->GetDocumentData(&nLen);
	CPPUNIT_ASSERT(_tcsncmp(pUndo1, TEXT("a"), nLen) == 0);

}

void TEST_CLASS_NAME::UndoTest3()
{
	YAEditDoc *pDoc = new YAEditDoc();

	BOOL bResult = pDoc->Init(TEXT("abcde"), NULL, NULL);
	CPPUNIT_ASSERT(bResult);

	Region rReplace(2, 0, 4, 0);
	bResult = pDoc->ReplaceString(&rReplace, TEXT("fgh"));
	CPPUNIT_ASSERT(bResult);
	// abcd I

	Region r = pDoc->GetUndoRegion();
	LPCTSTR p = pDoc->GetUndoStr();

	Region rExpected(2, 0, 5, 0);
	LPCTSTR pExpected = TEXT("cd");
	CPPUNIT_ASSERT(r == rExpected);
	CPPUNIT_ASSERT(_tcscmp(p, pExpected) == 0);


	// Undo
	bResult = pDoc->Undo();
	CPPUNIT_ASSERT(bResult);

	DWORD nLen;
	LPTSTR pUndo1 = pDoc->GetDocumentData(&nLen);
	CPPUNIT_ASSERT(_tcsncmp(pUndo1, TEXT("abcde"), nLen) == 0);

}

