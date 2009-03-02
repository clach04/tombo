#include <windows.h>
#include "../stdafx.h"


#include "VarBuffer.h"
#include "Region.h"
#include "YAEditDoc.h"
#include "PhysicalLineManager.h"
#include "MemManager.h"

static TestRunner *runner;

static void LoadTest1();
static void LoadTest2();
static void LoadTest3();
static void LoadTest4();
static void LoadTest5();
static void LoadTest6();
static void LoadTest7();
static void LoadTest8();
static void ConvertBytesToCoordinateTest1();
static void ConvertBytesToCoordinateTest2();
static void UndoTest1();
static void UndoTest2();
static void UndoTest3();

void YAEditDocTest(TestRunner *r) {
	runner = r;
	runner->WriteMsg("YAEditDocTest\r\n");

	LoadTest1();
	LoadTest2();
	LoadTest3();
	LoadTest4();
	LoadTest5();
	LoadTest6();
	LoadTest7();
	LoadTest8();
	ConvertBytesToCoordinateTest1();
	ConvertBytesToCoordinateTest2();
	UndoTest1();
	UndoTest2();
	UndoTest3();

}

////////////////////////////////////////////////

// empty string
void LoadTest1() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT(""), NULL, NULL);
	runner->assert(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	runner->assert(pLM->MaxLine() == 1);
	LineInfo *pLi = pLM->GetLineInfo(0);

	runner->assert(pLi->pLine->nUsed == 0);
}

// 1 line string without CRLF
void LoadTest2() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("Hello world"), NULL, NULL);
	runner->assert(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	runner->assert(pLM->MaxLine() == 1);
	LineInfo *pLi = pLM->GetLineInfo(0);

	runner->assert(pLi->pLine->nUsed == 11);
}

// 2 line string
void LoadTest3() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("Hellow\r\nworld"), NULL, NULL);
	runner->assert(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	runner->assert(pLM->MaxLine() == 2);
	LineInfo *pLi0 = pLM->GetLineInfo(0);
	runner->assert(pLi0->pLine->nUsed == 6);

	LPCTSTR p0 = pLM->GetLine(0);
	runner->assert(_tcsncmp(p0, TEXT("Hellow"), 6) == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	runner->assert(pLi1->pLine->nUsed == 5);
	LPCTSTR p1 = pLM->GetLine(1);
	runner->assert(_tcsncmp(p1, TEXT("world"), 5) == 0);	
}

// 2 line string end with CRLF
void LoadTest4() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("Hello\r\n"), NULL, NULL);
	runner->assert(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	runner->assert(pLM->MaxLine() == 2);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	runner->assert(pLi0->pLine->nUsed == 5);

	LPCTSTR p0 = pLM->GetLine(0);
	runner->assert(_tcsncmp(p0, TEXT("Hello"), 5) == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	runner->assert(pLi1->pLine->nUsed == 0);
}

// 2 line string start with CRLF
void LoadTest5() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("\r\nHello"), NULL, NULL);

	runner->assert(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	runner->assert(pLM->MaxLine() == 2);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	runner->assert(pLi0->pLine->nUsed == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	runner->assert(pLi1->pLine->nUsed == 5);

	LPCTSTR p1 = pLM->GetLine(1);
	runner->assert(_tcsncmp(p1, TEXT("Hello"), 5) == 0);

}

// CRLF
// CRLF
// CRLF
// EOF
void LoadTest6() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("\r\n\r\n\r\n"), NULL, NULL);

	runner->assert(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	runner->assert(pLM->MaxLine() == 4);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	runner->assert(pLi0->pLine->nUsed == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	runner->assert(pLi1->pLine->nUsed == 0);

	LineInfo *pLi2 = pLM->GetLineInfo(2);
	runner->assert(pLi2->pLine->nUsed == 0);

	LineInfo *pLi3 = pLM->GetLineInfo(3);
	runner->assert(pLi3->pLine->nUsed == 0);

}

// CRLF
// CRLF
// abcCRLF
// CRLF
// CRLF
// EOF
void LoadTest7() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("\r\n\r\nabc\r\n\r\n\r\n"), NULL, NULL);

	runner->assert(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	runner->assert(pLM->MaxLine() == 6);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	runner->assert(pLi0->pLine->nUsed == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	runner->assert(pLi1->pLine->nUsed == 0);

	LineInfo *pLi2 = pLM->GetLineInfo(2);
	runner->assert(pLi2->pLine->nUsed == 3);
	LPCTSTR p2 = pLM->GetLine(2);
	runner->assert(_tcsncmp(p2, TEXT("abc"), 3) == 0);


	LineInfo *pLi3 = pLM->GetLineInfo(3);
	runner->assert(pLi3->pLine->nUsed == 0);

	LineInfo *pLi4 = pLM->GetLineInfo(4);
	runner->assert(pLi4->pLine->nUsed == 0);

	LineInfo *pLi5 = pLM->GetLineInfo(5);
	runner->assert(pLi5->pLine->nUsed == 0);

}

// abcCRLF
// defCRLF
// ghi[EOF]
void LoadTest8() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("abc\r\ndef\r\nghi"), NULL, NULL);

	runner->assert(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	runner->assert(pLM->MaxLine() == 3);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	runner->assert(pLi0->pLine->nUsed == 3);
	LPCTSTR p0 = pLM->GetLine(0);
	runner->assert(_tcsncmp(p0, TEXT("abc"), 3) == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	runner->assert(pLi1->pLine->nUsed == 3);
	LPCTSTR p1 = pLM->GetLine(1);
	runner->assert(_tcsncmp(p1, TEXT("def"), 3) == 0);

	LineInfo *pLi2 = pLM->GetLineInfo(2);
	runner->assert(pLi2->pLine->nUsed == 3);
	LPCTSTR p2 = pLM->GetLine(2);
	runner->assert(_tcsncmp(p2, TEXT("ghi"), 3) == 0);
}

// abcCRLF
// defgCRLF
// hijklEOF
void ConvertBytesToCoordinateTest1()
{
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("abc\r\ndefg\r\nhijkl"), NULL, NULL);

	Coordinate pos;
	pDoc->ConvertBytesToCoordinate(0, &pos);
	runner->assert(pos.row == 0 && pos.col == 0);

	pDoc->ConvertBytesToCoordinate(2, &pos);
	runner->assert(pos.row == 0 && pos.col == 2);

	pDoc->ConvertBytesToCoordinate(5, &pos);
	runner->assert(pos.row == 1 && pos.col == 0);

	pDoc->ConvertBytesToCoordinate(6, &pos);
	runner->assert(pos.row == 1 && pos.col == 1);

	pDoc->ConvertBytesToCoordinate(9, &pos);
	runner->assert(pos.row == 1 && pos.col == 4);

	pDoc->ConvertBytesToCoordinate(11, &pos);
	runner->assert(pos.row == 2 && pos.col == 0);

	pDoc->ConvertBytesToCoordinate(16, &pos);
	runner->assert(pos.row == 2 && pos.col == 5);

	pDoc->ConvertBytesToCoordinate(100, &pos);
	runner->assert(pos.row == 2 && pos.col == 5);
}

void ConvertBytesToCoordinateTest2()
{
	YAEditDoc *pDoc = new YAEditDoc();

	Coordinate pos;

	BOOL bResult = pDoc->Init(TEXT("TOMBO 1.16\r\n"), NULL, NULL);
	pDoc->ConvertBytesToCoordinate(11, &pos);

	runner->assert(pos.row == 1 && pos.col == 0);
}

// initial state
void UndoTest1()
{
	YAEditDoc *pDoc = new YAEditDoc();

	BOOL bResult = pDoc->Init(TEXT("a"), NULL, NULL);
	runner->assert(bResult);

	Region r0(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

	Region r = pDoc->GetUndoRegion();
	LPCTSTR p = pDoc->GetUndoStr();

	runner->assert(r0 == r && p == NULL);
}

void UndoTest2()
{
	YAEditDoc *pDoc = new YAEditDoc();

	BOOL bResult = pDoc->Init(TEXT("a"), NULL, NULL);
	runner->assert(bResult);

	Region rReplace(1, 0, 1, 0);
	bResult = pDoc->ReplaceString(&rReplace, TEXT("bcd"));
	runner->assert(bResult);
	// abcd I

	Region r = pDoc->GetUndoRegion();
	LPCTSTR p = pDoc->GetUndoStr();

	Region rExpected(1, 0, 4, 0);
	LPCTSTR pExpected = TEXT("");
	runner->assert(r == rExpected);
	runner->assert(_tcscmp(p, pExpected) == 0);

	// Undo
	bResult = pDoc->Undo();
	runner->assert(bResult);

	DWORD nLen;
	LPTSTR pUndo1 = pDoc->GetDocumentData(&nLen);
	runner->assert(_tcsncmp(pUndo1, TEXT("a"), nLen) == 0);

}

void UndoTest3()
{
	YAEditDoc *pDoc = new YAEditDoc();

	BOOL bResult = pDoc->Init(TEXT("abcde"), NULL, NULL);
	runner->assert(bResult);

	Region rReplace(2, 0, 4, 0);
	bResult = pDoc->ReplaceString(&rReplace, TEXT("fgh"));
	runner->assert(bResult);
	// abcd I

	Region r = pDoc->GetUndoRegion();
	LPCTSTR p = pDoc->GetUndoStr();

	Region rExpected(2, 0, 5, 0);
	LPCTSTR pExpected = TEXT("cd");
	runner->assert(r == rExpected);
	runner->assert(_tcscmp(p, pExpected) == 0);


	// Undo
	bResult = pDoc->Undo();
	runner->assert(bResult);

	DWORD nLen;
	LPTSTR pUndo1 = pDoc->GetDocumentData(&nLen);
	runner->assert(_tcsncmp(pUndo1, TEXT("abcde"), nLen) == 0);

}

