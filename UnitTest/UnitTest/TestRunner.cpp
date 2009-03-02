#include "StdAfx.h"
#include "TestRunner.h"

#include <stdio.h>

DWORD WINAPI TestRunProc(LPVOID pParam);

TestRunner::TestRunner(void) {
	hThread = NULL;
}

TestRunner::~TestRunner(void) {
}

void TestRunner::run(HWND hWnd) {
	nSuccess = nFail = 0;
	this->hWnd= hWnd;
	hThread = CreateThread(NULL, 0, TestRunProc, this, 0, NULL);

}

void TestRunner::WriteMsg(LPCTSTR pMsg) {
	LPTSTR p = new TCHAR[_tcslen(pMsg) + 1];
	_tcscpy(p, pMsg);
	SendMessage(GetWnd(), WM_COMMAND, MAKEWPARAM(IDM_APP_WRITE_RESULT, 0), (LPARAM)p);
}

void TestRunner::FinishNotify() {
	WriteMsg(TEXT("\r\n\r\n"));
	WriteMsg(TEXT("---------------\r\n"));
	TCHAR buf[1024];
	sprintf(buf, "Success=%d, Fail=%d\r\n", nSuccess, nFail);
	WriteMsg(buf);
	WriteMsg(TEXT("Test finished."));

	SendMessage(GetWnd(), WM_COMMAND, MAKEWPARAM(IDM_APP_TEST_FINISHED, 0), NULL);
}

void TestRunner::TestSuccess() {
	WriteMsg(TEXT("."));
	nSuccess++;
}

void TestRunner::TestFail(LPCTSTR pTestCase) {
	WriteMsg(TEXT("\r\n"));
	WriteMsg(TEXT("FAIL"));
	WriteMsg(pTestCase);
	WriteMsg(TEXT("\r\n"));
	nFail++;
}

void TestRunner::assert(BOOL b) {
	if (b) {
		TestSuccess();
	} else {
		TestFail(TEXT(""));
	}
}

void VarBufferTest(TestRunner* pRunner);
void SharedStringTest(TestRunner *r);
void UniconvTest(TestRunner *r);
void TomboURITest(TestRunner *r);
void URIScannerTest(TestRunner *r);
void MemoInfoTest(TestRunner *r);
void RegexTest(TestRunner *r);
void YAEditDocTest(TestRunner *r);

DWORD WINAPI TestRunProc(LPVOID pParam) {
	TestRunner *runner = (TestRunner*)pParam;

	//////////////// TEST CASES /////////////////////
	VarBufferTest(runner);
	runner->WriteMsg(TEXT("\r\n"));
	SharedStringTest(runner);
	runner->WriteMsg(TEXT("\r\n"));
	UniconvTest(runner);
	runner->WriteMsg(TEXT("\r\n"));
	TomboURITest(runner);
	runner->WriteMsg(TEXT("\r\n"));
	URIScannerTest(runner);
	runner->WriteMsg(TEXT("\r\n"));
	MemoInfoTest(runner);
	runner->WriteMsg(TEXT("\r\n"));
	RegexTest(runner);
	runner->WriteMsg(TEXT("\r\n"));
	YAEditDocTest(runner);

	//////////////// TEST CASES END /////////////////////

	runner->FinishNotify();
	return 0;
}