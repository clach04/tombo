#include <windows.h>
#include "../stdafx.h"


#include "VarBuffer.h"
#include "Region.h"
#include "TString.h"
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
static void UndoTest4();
static void UndoTest5();
static void UndoTest6();
static void UndoTest7();


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
	UndoTest4();
	UndoTest5();
	UndoTest6();
	UndoTest7();
}

////////////////////////////////////////////////

// empty string
void LoadTest1() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT(""), NULL, NULL);
	ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	ASSERT(pLM->MaxLine() == 1);
	LineInfo *pLi = pLM->GetLineInfo(0);

	ASSERT(pLi->pLine->nUsed == 0);
}

// 1 line string without CRLF
void LoadTest2() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("Hello world"), NULL, NULL);
	ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	ASSERT(pLM->MaxLine() == 1);
	LineInfo *pLi = pLM->GetLineInfo(0);

	ASSERT(pLi->pLine->nUsed == 11);
}

// 2 line string
void LoadTest3() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("Hellow\r\nworld"), NULL, NULL);
	ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	ASSERT(pLM->MaxLine() == 2);
	LineInfo *pLi0 = pLM->GetLineInfo(0);
	ASSERT(pLi0->pLine->nUsed == 6);

	LPCTSTR p0 = pLM->GetLine(0);
	ASSERT(_tcsncmp(p0, TEXT("Hellow"), 6) == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	ASSERT(pLi1->pLine->nUsed == 5);
	LPCTSTR p1 = pLM->GetLine(1);
	ASSERT(_tcsncmp(p1, TEXT("world"), 5) == 0);	
}

// 2 line string end with CRLF
void LoadTest4() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("Hello\r\n"), NULL, NULL);
	ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	ASSERT(pLM->MaxLine() == 2);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	ASSERT(pLi0->pLine->nUsed == 5);

	LPCTSTR p0 = pLM->GetLine(0);
	ASSERT(_tcsncmp(p0, TEXT("Hello"), 5) == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	ASSERT(pLi1->pLine->nUsed == 0);
}

// 2 line string start with CRLF
void LoadTest5() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("\r\nHello"), NULL, NULL);

	ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	ASSERT(pLM->MaxLine() == 2);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	ASSERT(pLi0->pLine->nUsed == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	ASSERT(pLi1->pLine->nUsed == 5);

	LPCTSTR p1 = pLM->GetLine(1);
	ASSERT(_tcsncmp(p1, TEXT("Hello"), 5) == 0);

}

// CRLF
// CRLF
// CRLF
// EOF
void LoadTest6() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("\r\n\r\n\r\n"), NULL, NULL);

	ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	ASSERT(pLM->MaxLine() == 4);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	ASSERT(pLi0->pLine->nUsed == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	ASSERT(pLi1->pLine->nUsed == 0);

	LineInfo *pLi2 = pLM->GetLineInfo(2);
	ASSERT(pLi2->pLine->nUsed == 0);

	LineInfo *pLi3 = pLM->GetLineInfo(3);
	ASSERT(pLi3->pLine->nUsed == 0);

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

	ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	ASSERT(pLM->MaxLine() == 6);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	ASSERT(pLi0->pLine->nUsed == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	ASSERT(pLi1->pLine->nUsed == 0);

	LineInfo *pLi2 = pLM->GetLineInfo(2);
	ASSERT(pLi2->pLine->nUsed == 3);
	LPCTSTR p2 = pLM->GetLine(2);
	ASSERT(_tcsncmp(p2, TEXT("abc"), 3) == 0);


	LineInfo *pLi3 = pLM->GetLineInfo(3);
	ASSERT(pLi3->pLine->nUsed == 0);

	LineInfo *pLi4 = pLM->GetLineInfo(4);
	ASSERT(pLi4->pLine->nUsed == 0);

	LineInfo *pLi5 = pLM->GetLineInfo(5);
	ASSERT(pLi5->pLine->nUsed == 0);

}

// abcCRLF
// defCRLF
// ghi[EOF]
void LoadTest8() {
	YAEditDoc *pDoc = new YAEditDoc();
	BOOL bResult = pDoc->Init(TEXT("abc\r\ndef\r\nghi"), NULL, NULL);

	ASSERT(bResult);
	PhysicalLineManager *pLM = pDoc->GetPhMgr();

	ASSERT(pLM->MaxLine() == 3);

	LineInfo *pLi0 = pLM->GetLineInfo(0);
	ASSERT(pLi0->pLine->nUsed == 3);
	LPCTSTR p0 = pLM->GetLine(0);
	ASSERT(_tcsncmp(p0, TEXT("abc"), 3) == 0);

	LineInfo *pLi1 = pLM->GetLineInfo(1);
	ASSERT(pLi1->pLine->nUsed == 3);
	LPCTSTR p1 = pLM->GetLine(1);
	ASSERT(_tcsncmp(p1, TEXT("def"), 3) == 0);

	LineInfo *pLi2 = pLM->GetLineInfo(2);
	ASSERT(pLi2->pLine->nUsed == 3);
	LPCTSTR p2 = pLM->GetLine(2);
	ASSERT(_tcsncmp(p2, TEXT("ghi"), 3) == 0);
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
	ASSERT(pos.row == 0 && pos.col == 0);

	pDoc->ConvertBytesToCoordinate(2, &pos);
	ASSERT(pos.row == 0 && pos.col == 2);

	pDoc->ConvertBytesToCoordinate(5, &pos);
	ASSERT(pos.row == 1 && pos.col == 0);

	pDoc->ConvertBytesToCoordinate(6, &pos);
	ASSERT(pos.row == 1 && pos.col == 1);

	pDoc->ConvertBytesToCoordinate(9, &pos);
	ASSERT(pos.row == 1 && pos.col == 4);

	pDoc->ConvertBytesToCoordinate(11, &pos);
	ASSERT(pos.row == 2 && pos.col == 0);

	pDoc->ConvertBytesToCoordinate(16, &pos);
	ASSERT(pos.row == 2 && pos.col == 5);

	pDoc->ConvertBytesToCoordinate(100, &pos);
	ASSERT(pos.row == 2 && pos.col == 5);
}

void ConvertBytesToCoordinateTest2()
{
	YAEditDoc *pDoc = new YAEditDoc();

	Coordinate pos;

	BOOL bResult = pDoc->Init(TEXT("TOMBO 1.16\r\n"), NULL, NULL);
	pDoc->ConvertBytesToCoordinate(11, &pos);

	ASSERT(pos.row == 1 && pos.col == 0);
}

// initial state
void UndoTest1()
{
	YAEditDoc *pDoc = new YAEditDoc();

	BOOL bResult = pDoc->Init(TEXT("a"), NULL, NULL);
	ASSERT(bResult);

	Region r0(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);

	UndoInfo *pUndo = pDoc->GetUndoInfo();
	ASSERT(pUndo == NULL);

}

void UndoTest2()
{
	YAEditDoc *pDoc = new YAEditDoc();

	BOOL bResult = pDoc->Init(TEXT("a"), NULL, NULL);
	ASSERT(bResult);
	// a I

	Region rReplace(1, 0, 1, 0);
	bResult = pDoc->ReplaceString(&rReplace, TEXT("bcd"));
	ASSERT(bResult);
	// abcd I

	UndoInfo *pUndo = pDoc->GetUndoInfo();
	Region rExpectedPrev(1, 0, 1, 0);
	Region rExpectedNew(1, 0, 4, 0);
	ASSERT(pUndo->rPrevRegion == rExpectedPrev);
	ASSERT(pUndo->rNewRegion == rExpectedNew);
	ASSERT(_tcscmp(pUndo->sPrevStr.Get(), TEXT("")) == 0);
	ASSERT(_tcscmp(pUndo->sNewStr.Get(), TEXT("bcd")) == 0);

	// Undo
	bResult = pDoc->Undo();
	ASSERT(bResult);

	DWORD nLen;
	LPTSTR pUndo1 = pDoc->GetDocumentData(&nLen);
	ASSERT(_tcsncmp(pUndo1, TEXT("a"), nLen) == 0);

}

void UndoTest3()
{
	YAEditDoc *pDoc = new YAEditDoc();

	BOOL bResult = pDoc->Init(TEXT("abcde"), NULL, NULL);
	ASSERT(bResult);

	Region rReplace(2, 0, 4, 0);
	bResult = pDoc->ReplaceString(&rReplace, TEXT("fgh"));
	ASSERT(bResult);
	// abcd I

	UndoInfo *pUndo = pDoc->GetUndoInfo();

	Region rExpectedPrev(2, 0, 4, 0);
	Region rExpectedNew(2, 0, 5, 0);
	ASSERT(pUndo->rPrevRegion == rExpectedPrev);
	ASSERT(pUndo->rNewRegion == rExpectedNew);

	ASSERT(_tcscmp(pUndo->sPrevStr.Get(), TEXT("cd")) == 0);
	ASSERT(_tcscmp(pUndo->sNewStr.Get(), TEXT("fgh")) == 0);


	// Undo
	bResult = pDoc->Undo();
	ASSERT(bResult);

	DWORD nLen;
	LPTSTR pUndo1 = pDoc->GetDocumentData(&nLen);
	ASSERT(_tcsncmp(pUndo1, TEXT("abcde"), nLen) == 0);

}

void UndoTest4() {
	YAEditDoc *pDoc = new YAEditDoc();
	ASSERT(pDoc->Init(TEXT("-----"), NULL, NULL));

	// -----
	Region rReplace(2, 0, 2, 0);
	ASSERT(pDoc->ReplaceString(&rReplace, TEXT("a")));
	// --a---

	// Undo!
	ASSERT(pDoc->Undo());

	// expect is -----
	DWORD nLen;
	LPTSTR pResult;
	pResult = pDoc->GetDocumentData(&nLen);
	ASSERT(pResult != NULL);
	ASSERT(_tcsncmp(pResult, TEXT("-----"), nLen) == 0);
	ASSERT(nLen == 5);

	// Undo(exactly say, this is Redo)
	ASSERT(pDoc->Undo());

	// expect is --a---
	pResult = pDoc->GetDocumentData(&nLen);
	ASSERT(pResult != NULL);
	ASSERT(_tcsncmp(pResult, TEXT("--a---"), nLen) == 0);
	ASSERT(nLen == 6);

	// Undo
	ASSERT(pDoc->Undo());
	pResult = pDoc->GetDocumentData(&nLen);
	ASSERT(pResult != NULL);
	ASSERT(_tcsncmp(pResult, TEXT("-----"), nLen) == 0);
	ASSERT(nLen == 5);

	// Undo(exactly say, this is Redo)
	ASSERT(pDoc->Undo());

	// expect is --a---
	pResult = pDoc->GetDocumentData(&nLen);
	ASSERT(pResult != NULL);
	ASSERT(_tcsncmp(pResult, TEXT("--a---"), nLen) == 0);
	ASSERT(nLen == 6);
}

void UndoTest5() {

	YAEditDoc *pDoc = new YAEditDoc();
	ASSERT(pDoc->Init(TEXT("-----"), NULL, NULL));

	// -----
	Region rReplace(2, 0, 2, 0);
	ASSERT(pDoc->ReplaceString(&rReplace, TEXT("a")));
	// --a---
	Region rReplace2(3, 0, 3, 0);
	ASSERT(pDoc->ReplaceString(&rReplace2, TEXT("b")));
	// --ab---

	DWORD nLen;
	LPTSTR pResult;

	pResult = pDoc->GetDocumentData(&nLen);
	ASSERT(pResult != NULL);
	ASSERT(_tcsncmp(pResult, TEXT("--ab---"), nLen) == 0);
	ASSERT(nLen == 7);

	// Undo!
	ASSERT(pDoc->Undo());

	// expect is -----
	pResult = pDoc->GetDocumentData(&nLen);
	ASSERT(pResult != NULL);
	ASSERT(_tcsncmp(pResult, TEXT("-----"), nLen) == 0);
	ASSERT(nLen == 5);
	// internal detail
	UndoInfo *pui = pDoc->GetUndoInfo();
	ASSERT(_tcscmp(pui->sPrevStr.Get(), TEXT("")) == 0);
	ASSERT(_tcscmp(pui->sNewStr.Get(), TEXT("ab")) == 0);
	Region rPrevExpect(2, 0, 2, 0);
	ASSERT(pui->rPrevRegion == rPrevExpect);
	Region rNewExpect(2, 0, 4, 0);
	ASSERT(pui->rNewRegion == rNewExpect);

	ASSERT(pDoc->Undo());

	// Undo(Redo)
	pResult = pDoc->GetDocumentData(&nLen);
	ASSERT(pResult != NULL);
	ASSERT(_tcsncmp(pResult, TEXT("--ab---"), nLen) == 0);
	ASSERT(nLen == 7);
	// expect is --ab---

}

// CloseUndoRegion test
void UndoTest6() {

	YAEditDoc *pDoc = new YAEditDoc();
	ASSERT(pDoc->Init(TEXT("-----"), NULL, NULL));

	// -----
	Region rReplace(2, 0, 2, 0);
	ASSERT(pDoc->ReplaceString(&rReplace, TEXT("a")));
	// --a---

	// Cursor moved so undo is applyed only to 'b'
	pDoc->CloseUndoRegion();

	Region rReplace2(3, 0, 3, 0);
	ASSERT(pDoc->ReplaceString(&rReplace2, TEXT("b")));
	// --ab---

	DWORD nLen;
	LPTSTR pResult;

	pResult = pDoc->GetDocumentData(&nLen);
	ASSERT(pResult != NULL);
	ASSERT(_tcsncmp(pResult, TEXT("--ab---"), nLen) == 0);
	ASSERT(nLen == 7);

	// Undo!
	ASSERT(pDoc->Undo());

	// expect is -----
	pResult = pDoc->GetDocumentData(&nLen);
	ASSERT(pResult != NULL);
	ASSERT(_tcsncmp(pResult, TEXT("--a---"), nLen) == 0);
	ASSERT(nLen == 6);

	ASSERT(pDoc->Undo());

	// Undo(Redo)
	pResult = pDoc->GetDocumentData(&nLen);
	ASSERT(pResult != NULL);
	ASSERT(_tcsncmp(pResult, TEXT("--ab---"), nLen) == 0);
	ASSERT(nLen == 7);
	// expect is --ab---

}

// 'a', 'b', 'c', BS, 'D', Ctrl-Z, Ctl-Z, Ctl-Z
void UndoTest7() {
	YAEditDoc *pDoc = new YAEditDoc();
	ASSERT(pDoc->Init(TEXT("-----"), NULL, NULL));
	// -----
	Region r1(2, 0, 2, 0);
	ASSERT(pDoc->ReplaceString(&r1, TEXT("a")));

	Region r2(3, 0, 3, 0);
	ASSERT(pDoc->ReplaceString(&r2, TEXT("b")));

	Region r3(4, 0, 4, 0);
	ASSERT(pDoc->ReplaceString(&r3, TEXT("c")));
//	--abc---

	Region r4(4, 0, 5, 0);
	ASSERT(pDoc->ReplaceString(&r4, TEXT("")));
	pDoc->CloseUndoRegion();

	Region r5(4, 0, 4, 0);
	ASSERT(pDoc->ReplaceString(&r5, TEXT("d")));

	ASSERT(pDoc->Undo());
	ASSERT(pDoc->Undo());

	DWORD nLen;
	LPTSTR pResult;

	pResult = pDoc->GetDocumentData(&nLen);
	ASSERT(pResult != NULL);
	ASSERT(_tcsncmp(pResult, TEXT("--abd---"), nLen) == 0);
	ASSERT(nLen == 8);

	ASSERT(pDoc->Undo());
	ASSERT(0);
}