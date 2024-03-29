#pragma once

#define IDM_APP_TEST_FINISHED 300
#define IDM_APP_WRITE_RESULT  301

class TestRunner {
protected:
	HANDLE hThread;
	HWND hWnd;

	DWORD nSuccess;
	DWORD nFail;

public:
	TestRunner(void);
	~TestRunner(void);

	void run(HWND hWnd);
	HWND GetWnd() { return hWnd; }

	void WriteMsg(LPCTSTR pMsg);
	void FinishNotify();

	void TestSuccess();
	void TestFail(LPCTSTR pTestCaseName);

	void assert(BOOL b);
	void assert(BOOL b, LPCTSTR pFile, long lLine);
};

#define ASSERT(b) runner->assert(b, __FILE__, __LINE__)
#define TESTCASE runner->WriteMsg(__FUNCTION__)