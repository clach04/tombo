#include <windows.h>
#include "../stdafx.h"

#include <windows.h>
#include <tchar.h>
#include "Tombo.h"
#include "TString.h"
#include "UniConv.h"
#include "TomboURI.h"
#include "Repository.h"
#include "VarBuffer.h"
#include "URIScanner.h"

static TestRunner *runner;

static void URIListTest1();
static void URIScanTest1();
static void URIScanTest2();
static void URIScanTest3();
static void URIScanTest4();
static void URIScanTest5();
static void URIScanTest6();
static void URIScanTest7();
static void URIScanTest8();
static void PartialScanTest1();
static void PartialScanTest2();
static void InterruptTest1();
static void InterruptTest2();
static void InterruptTest3();
static void InterruptTest4();

void URIScannerTest(TestRunner *r) {
	runner = r;
	runner->WriteMsg("URIScannerTest\r\n");

	URIListTest1();
	URIScanTest1();
	URIScanTest2();
	URIScanTest3();
	URIScanTest4();
	URIScanTest5();
	URIScanTest6();
	URIScanTest7();
	URIScanTest8();
	PartialScanTest1();
	PartialScanTest2();
	InterruptTest1();
	InterruptTest2();
	InterruptTest3();
	InterruptTest4();
}

void URIListTest1() {
	URIList list;
	runner->assert(list.Init());

	runner->assert(list.Add(NULL, NULL));

	TomboURI uri;
	uri.Init("tombo://default/test");
	runner->assert(list.Add(&uri, NULL));

	{	// the case of when uri2 released first.
		TomboURI uri2;
		uri2.Init("tombo://default/test2");
		runner->assert(list.Add(&uri2, NULL));
	}

	runner->assert(list.Add(NULL, "test"));

	runner->assert(list.GetSize() == 4);

	runner->assert(list.GetURI(0) == NULL);
	runner->assert(list.GetTitle(0) == NULL);
	runner->assert(strcmp(list.GetURI(1)->GetFullURI(), "tombo://default/test") == 0);
	runner->assert(list.GetTitle(1) == NULL);
	runner->assert(strcmp(list.GetURI(2)->GetFullURI(), "tombo://default/test2") == 0);
	runner->assert(list.GetTitle(2) == NULL);
	runner->assert(list.GetURI(3) == NULL);
	runner->assert(strcmp(list.GetTitle(3), "test") == 0);
}

////////////////////////////////////////////////
// dummy repository
class DummyRepoBase : public IEnumRepository {
public:
	BOOL GetOption(const TomboURI *pURI, URIOption *pOption);
	BOOL GetHeadLine(const TomboURI *pURI, TString *pHeadLine);
};

BOOL DummyRepoBase::GetOption(const TomboURI *pURI, URIOption *pOption)
{
	LPCTSTR p = pURI->GetFullURI();
	if (*(p + strlen(p) - 1) == '/') {
		pOption->bFolder = TRUE;
	} else {
		pOption->bFolder = FALSE;
	}
	return TRUE;
}

BOOL DummyRepoBase::GetHeadLine(const TomboURI *pURI, TString *pHeadLine)
{
	LPTSTR p = StringDup(pURI->GetFullURI());
	if (*(p + strlen(p) -1) == '/') {
		*(p + strlen(p) - 1) = '\0';	// folder only
		LPCTSTR pTop = strrchr(p, '/') + 1;
		pHeadLine->Set(pTop);
		return TRUE;
	} else {
		LPTSTR q = p;
		LPTSTR r;
		while(*q) {
			if (*q == '/') r = q;
			q++;
		}
		*(r + strlen(r) - 4) = '\0';
		pHeadLine->Set(r + 1);
		return TRUE;
	}
}

////////////////////////////////////////////////
// dummy repo sample

class RepoX : public DummyRepoBase {
	URIList *GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose);
};

URIList *RepoX::GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose)
{
	if (strcmp(pFolderURI->GetFullURI(), "tombo://default/aaa/") == 0) {
		URIList *pList = new URIList();
		pList->Init();
		TomboURI uri1; uri1.Init("tombo://default/aaa/bbb/"); pList->Add(&uri1, "bbb");
		TomboURI uri2; uri2.Init("tombo://default/aaa/test/"); pList->Add(&uri2, "test");
		TomboURI uri3; uri3.Init("tombo://default/aaa/hello.txt"); pList->Add(&uri3, "hello");
		TomboURI uri4; uri4.Init("tombo://default/aaa/zzz.txt"); pList->Add(&uri4, "zzz");
		return pList;
	} 
	if (strcmp(pFolderURI->GetFullURI(), "tombo://default/aaa/bbb/") == 0) {
		URIList *pList = new URIList();
		pList->Init();
		return pList;
	}
	if (strcmp(pFolderURI->GetFullURI(), "tombo://default/aaa/test/") == 0) {
		URIList *pList = new URIList();
		pList->Init();
		TomboURI uri3; uri3.Init("tombo://default/aaa/test/sub.txt"); pList->Add(&uri3, "sub");
		return pList;
	}
	return NULL;
}


////////////////////////////////////////////////
// Generic test scanner

// at result, calling hisory is stored to sbuf.

class TestScanner1 : public URIScanner {
public:
	StringBufferA sbuf;

	int nInitialScan;
	int nAfterScan;
	int nPreFolder;
	int nPostFolder;
	int nNote;

	TestScanner1();

	void InitialScan();
	void AfterScan();

	void PreFolder();
	void PostFolder();

	void Node();

	BOOL Check(LPCTSTR pMsg, LPCTSTR pCorrect);
	static void FullScanTest(LPCTSTR pTestName, LPCTSTR pCorrect, DummyRepoBase *pRepo);
	static void ScanTest(LPCTSTR pTestName, LPCTSTR pCorrect, DummyRepoBase *pRepo, const TomboURI *pURI, BOOL bReverse);
};

TestScanner1::TestScanner1() 
{
	nInitialScan = nAfterScan = nPreFolder = nPostFolder = nNote = 0;
	sbuf.Init(100, 20);
}

void TestScanner1::InitialScan() 
{
	DWORD n;
	sbuf.Add("IS", 2, &n);
}

void TestScanner1::AfterScan() 
{
	DWORD n;
	sbuf.Add("AS", 2+1, &n);
}

void TestScanner1::PreFolder() 
{
	DWORD n;

	sbuf.Add("BF", 2, &n);

	LPCTSTR pFull = CurrentURI()->GetFullURI();
	sbuf.Add(pFull, strlen(pFull), &n);
	sbuf.Add(GetTitle(), strlen(GetTitle()), &n);
}

void TestScanner1::PostFolder() 
{
	DWORD n;
	sbuf.Add("AF", 2, &n);

	LPCTSTR pFull = CurrentURI()->GetFullURI();
	sbuf.Add(pFull, strlen(pFull), &n);
	sbuf.Add(GetTitle(), strlen(GetTitle()), &n);
}

void TestScanner1::Node()
{
	DWORD n;
	sbuf.Add("ND", 2, &n);

	LPCTSTR pFull = CurrentURI()->GetFullURI();
	sbuf.Add(pFull, strlen(pFull), &n);
	sbuf.Add(GetTitle(), strlen(GetTitle()), &n);
}

void TestScanner1::FullScanTest(LPCTSTR pTestName, LPCTSTR pCorrect, DummyRepoBase *pRepo)
{
	TomboURI base;
	base.Init("tombo://default/aaa/");

	TestScanner1 s1;

	runner->assert(s1.Init(pRepo, &base, FALSE));

	BOOL bResult = s1.FullScan();
	runner->assert(bResult == TRUE);
	LPCTSTR pResultStr = s1.sbuf.Get(0);
	runner->assert(s1.Check(pTestName, pCorrect));
}

void TestScanner1::ScanTest(LPCTSTR pTestName, LPCTSTR pCorrect, DummyRepoBase *pRepo, const TomboURI *pURI, BOOL bReverse)
{
	TomboURI base;
	base.Init("tombo://default/aaa/");

	TestScanner1 s1;

	runner->assert(s1.Init(pRepo, &base, FALSE));

	BOOL bResult = s1.Scan(pURI, bReverse);
	runner->assert(bResult == TRUE);
	LPCTSTR pResultStr = s1.sbuf.Get(0);
	runner->assert(s1.Check(pTestName, pCorrect));
}

BOOL TestScanner1::Check(LPCTSTR pMsg, LPCTSTR pCorrect)
{
	LPCTSTR pResult = sbuf.Get(0);

	DWORD n1 = strlen(pResult);
	DWORD n2 = strlen(pCorrect);

	for (DWORD i = 0; i < n1; i++) {
		if (pResult[i] != pCorrect[i]) {
			Sleep(1);
		}
	}
	Sleep(1);

	return strcmp(sbuf.Get(0), pCorrect) == 0;
}

////////////////////////////////////////////////
// TEST
//
// Test the root is empty

// test data
class Repo1 : public DummyRepoBase {
	URIList *GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose);
};

URIList *Repo1::GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose)
{
	URIList *pList = new URIList();
	pList->Init();
	return pList;
}

// test runner
void URIScanTest1() 
{
	LPCTSTR pTest = "TEST1";
	LPCTSTR pCorrect = 
		"IS"
		"BF" "tombo://default/aaa/" "aaa"
		"AF" "tombo://default/aaa/" "aaa"
		"AS";
	Repo1 rep;

	TestScanner1::FullScanTest(pTest, pCorrect, &rep);
}

////////////////////////////////////////////////
// TEST
//
// 2 nodes under the root

// test data
class Repo2 : public DummyRepoBase {
	URIList *GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose);
};

URIList *Repo2::GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose)
{
	URIList *pList = new URIList();
	pList->Init();

	TomboURI uri1; uri1.Init("tombo://default/aaa/bbb.txt"); pList->Add(&uri1, "bbb");
	TomboURI uri2; uri2.Init("tombo://default/aaa/ccc.txt"); pList->Add(&uri2, "ccc");

	return pList;
}

void URIScanTest2() 
{
	LPCTSTR pTest = "TEST3";
	LPCTSTR pCorrect = 
		"IS"
		"BF" "tombo://default/aaa/" "aaa"
		"ND" "tombo://default/aaa/bbb.txt" "bbb"
		"ND" "tombo://default/aaa/ccc.txt" "ccc"
		"AF" "tombo://default/aaa/" "aaa"
		"AS";
	Repo2 rep;

	TestScanner1::FullScanTest(pTest, pCorrect, &rep);
}

////////////////////////////////////////////////
// TEST
//
// there are a folder and a node under the root.

// test data
class Repo3 : public DummyRepoBase {
	URIList *GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose);
};

URIList *Repo3::GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose)
{
	URIList *pList = new URIList();
	pList->Init();

	if (strcmp(pFolderURI->GetFullURI(), "tombo://default/aaa/") == 0) {
		TomboURI uri1; uri1.Init("tombo://default/aaa/bbb/"); pList->Add(&uri1, "bbb");
		TomboURI uri2; uri2.Init("tombo://default/aaa/ccc.txt"); pList->Add(&uri2, "ccc");
	}
	if (strcmp(pFolderURI->GetFullURI(), "tombo://default/aaa/bbb/") == 0) {
		URIList *pList = new URIList();
		pList->Init();
		return pList;
	}
	return pList;
}

void URIScanTest3() 
{
	LPCTSTR pTest = "TEST3";
	LPCTSTR pCorrect = 
		"IS"
		"BF" "tombo://default/aaa/" "aaa"
		"BF" "tombo://default/aaa/bbb/" "bbb"
		"AF" "tombo://default/aaa/bbb/" "bbb"
		"ND" "tombo://default/aaa/ccc.txt" "ccc"
		"AF" "tombo://default/aaa/" "aaa"
		"AS";
	Repo3 rep;

	TestScanner1::FullScanTest(pTest, pCorrect, &rep);
}

////////////////////////////////////////////////
// TEST
//
// nested folder

// test data
class Repo4 : public DummyRepoBase {
	const TomboURI *pStopURI;
protected:
	URIList *GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose);
public:
	Repo4() : DummyRepoBase(), pStopURI(NULL) {}
	Repo4(const TomboURI *pURI);
};

Repo4::Repo4(const TomboURI *pURI)
{
	pStopURI = new TomboURI(*pURI);
}

URIList *Repo4::GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose)
{
	URIList *pList = new URIList();
	pList->Init();

	if (pStopURI != NULL && strcmp(pStopURI->GetFullURI(), pFolderURI->GetFullURI()) == 0) {
		SetLastError(ERROR_CANCELLED);
		return NULL;
	}

	if (strcmp(pFolderURI->GetFullURI(), "tombo://default/aaa/") == 0) {
		TomboURI uri1; uri1.Init("tombo://default/aaa/bbb/"); pList->Add(&uri1, "bbb");
		TomboURI uri2; uri2.Init("tombo://default/aaa/ddd.txt"); pList->Add(&uri2, "ddd");
	}
	if (strcmp(pFolderURI->GetFullURI(), "tombo://default/aaa/bbb/") == 0) {
		URIList *pList = new URIList();
		pList->Init();
		TomboURI uri1; uri1.Init("tombo://default/aaa/bbb/ccc/"); pList->Add(&uri1, "ccc");
		TomboURI uri2; uri2.Init("tombo://default/aaa/bbb/eee.txt"); pList->Add(&uri2, "eee");
		return pList;
	}
	if (strcmp(pFolderURI->GetFullURI(), "tombo://default/aaa/bbb/ccc/") == 0) {
		URIList *pList = new URIList();
		pList->Init();
		TomboURI uri1; uri1.Init("tombo://default/aaa/bbb/ccc/fff.txt"); pList->Add(&uri1, "fff");
		TomboURI uri2; uri2.Init("tombo://default/aaa/bbb/ccc/ggg.txt"); pList->Add(&uri2, "ggg");
		return pList;
	}

	return pList;
}

void URIScanTest4() 
{
	LPCTSTR pTest = "TEST4";
	LPCTSTR pCorrect = 
		"IS"
		"BF" "tombo://default/aaa/" "aaa"
		"BF" "tombo://default/aaa/bbb/" "bbb"
		"BF" "tombo://default/aaa/bbb/ccc/" "ccc"
		"ND" "tombo://default/aaa/bbb/ccc/fff.txt" "fff"
		"ND" "tombo://default/aaa/bbb/ccc/ggg.txt" "ggg"
		"AF" "tombo://default/aaa/bbb/ccc/" "ccc"
		"ND" "tombo://default/aaa/bbb/eee.txt" "eee"
		"AF" "tombo://default/aaa/bbb/" "bbb"
		"ND" "tombo://default/aaa/ddd.txt" "ddd"
		"AF" "tombo://default/aaa/" "aaa"
		"AS";
	Repo4 rep;

	TestScanner1::FullScanTest(pTest, pCorrect, &rep);
}

////////////////////////////////////////////////
// TEST
//
// stop scan test
// Data is same as URIScanTest4

class TestScanner2 : public TestScanner1 {
public:
	void Node();
};

void TestScanner2::Node() 
{
	TestScanner1::Node();
	LPCTSTR pFull = CurrentURI()->GetFullURI();
	if (strcmp(pFull, "tombo://default/aaa/bbb/ccc/fff.txt") == 0) {
		StopScan();
	}
}

void URIScanTest5() 
{
	LPCTSTR pTest = "TEST5";
	LPCTSTR pCorrect = 
		"IS"
		"BF" "tombo://default/aaa/" "aaa"
		"BF" "tombo://default/aaa/bbb/" "bbb"
		"BF" "tombo://default/aaa/bbb/ccc/" "ccc"
		"ND" "tombo://default/aaa/bbb/ccc/fff.txt" "fff"
		"AF" "tombo://default/aaa/bbb/ccc/" "ccc"
		"AF" "tombo://default/aaa/bbb/" "bbb"
		"AF" "tombo://default/aaa/" "aaa"
		"AS";
	Repo4 rep;

	TomboURI base;
	base.Init("tombo://default/aaa/");

	TestScanner2 s1;

	runner->assert(s1.Init(&rep, &base, FALSE));

	BOOL bResult = s1.FullScan();
	runner->assert(bResult == TRUE);
	LPCTSTR pResultStr = s1.sbuf.Get(0);
	runner->assert(s1.Check(pTest, pCorrect));
}

////////////////////////////////////////////////
// TEST
//
// cancel test
// assume when user cancelled password dialog

class Repo6 : public Repo4 {
	URIList *GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose);
};

URIList *Repo6::GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose)
{
	if (strcmp(pFolderURI->GetFullURI(), "tombo://default/aaa/bbb/ccc/") == 0) {
		SetLastError(ERROR_CANCELLED);
		return NULL;
	} else {
		return Repo4::GetChild(pFolderURI, bSkipEncrypt, FALSE, FALSE);
	}
}

void URIScanTest6() 
{
	LPCTSTR pTest = "TEST6";
	LPCTSTR pCorrect = 
		"IS"
		"BF" "tombo://default/aaa/" "aaa"
		"BF" "tombo://default/aaa/bbb/" "bbb"
		"AF" "tombo://default/aaa/bbb/" "bbb"
		"AF" "tombo://default/aaa/" "aaa"
		"AS";
	Repo6 rep;

	TestScanner1::FullScanTest(pTest, pCorrect, &rep);
}

////////////////////////////////////////////////
// TEST
//
// cancel at root
// assume when user cancelled password dialog

class Repo7 : public Repo4 {
	URIList *GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose);
};

URIList *Repo7::GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose)
{
	SetLastError(ERROR_CANCELLED);
	return NULL;
}

void URIScanTest7() 
{
	LPCTSTR pTest = "TEST7";
	LPCTSTR pCorrect = 
		"IS"
		"AS";
	Repo7 rep;

	TestScanner1::FullScanTest(pTest, pCorrect, &rep);
}

////////////////////////////////////////////////
// TEST
//
// reverse order of URIScanTest4()

void URIScanTest8() 
{
	LPCTSTR pTest = "TEST8";
	LPCTSTR pCorrect = 
		"IS"
		"BF" "tombo://default/aaa/" "aaa"
		"ND" "tombo://default/aaa/ddd.txt" "ddd"
		"BF" "tombo://default/aaa/bbb/" "bbb"		
		"ND" "tombo://default/aaa/bbb/eee.txt" "eee"
		"BF" "tombo://default/aaa/bbb/ccc/" "ccc"
		"ND" "tombo://default/aaa/bbb/ccc/ggg.txt" "ggg"
		"ND" "tombo://default/aaa/bbb/ccc/fff.txt" "fff"
		"AF" "tombo://default/aaa/bbb/ccc/" "ccc"		
		"AF" "tombo://default/aaa/bbb/" "bbb"
		"AF" "tombo://default/aaa/" "aaa"
		"AS";
	Repo4 rep;	// Repo4 is correct.

	TestScanner1::ScanTest(pTest, pCorrect, &rep, NULL, TRUE);
}

////////////////////////////////////////////////
// TEST
//
// partial scan test

void PartialScanTest1() 
{
	LPCTSTR pTest = "TEST-P1";
	LPCTSTR pCorrect = 
		"IS"
		"BF" "tombo://default/aaa/" "aaa"
		"BF" "tombo://default/aaa/bbb/" "bbb"
		"BF" "tombo://default/aaa/bbb/ccc/" "ccc"
		"ND" "tombo://default/aaa/bbb/ccc/ggg.txt" "ggg"
		"AF" "tombo://default/aaa/bbb/ccc/" "ccc"
		"ND" "tombo://default/aaa/bbb/eee.txt" "eee"
		"AF" "tombo://default/aaa/bbb/" "bbb"
		"ND" "tombo://default/aaa/ddd.txt" "ddd"
		"AF" "tombo://default/aaa/" "aaa"
		"AS";
	Repo4 rep;

	TomboURI part;
	part.Init("tombo://default/aaa/bbb/ccc/ggg.txt");

	TestScanner1::ScanTest(pTest, pCorrect, &rep, &part, FALSE);

}

////////////////////////////////////////////////
// TEST
//
// baseURI and startURI is same

void PartialScanTest2() 
{
	LPCTSTR pTest = "PSTEST2";
	LPCTSTR pCorrect = 
		"IS"
		"BF" "tombo://default/aaa/" "aaa"
		"BF" "tombo://default/aaa/bbb/" "bbb"
		"BF" "tombo://default/aaa/bbb/ccc/" "ccc"
		"ND" "tombo://default/aaa/bbb/ccc/fff.txt" "fff"
		"ND" "tombo://default/aaa/bbb/ccc/ggg.txt" "ggg"
		"AF" "tombo://default/aaa/bbb/ccc/" "ccc"
		"ND" "tombo://default/aaa/bbb/eee.txt" "eee"
		"AF" "tombo://default/aaa/bbb/" "bbb"
		"ND" "tombo://default/aaa/ddd.txt" "ddd"
		"AF" "tombo://default/aaa/" "aaa"
		"AS";
	Repo4 rep;

	TomboURI part;
	part.Init("tombo://default/aaa/");

	TestScanner1::ScanTest(pTest, pCorrect, &rep, &part, FALSE);
}

////////////////////////////////////////////////
// TEST
//
// cancelled at root

void InterruptTest1() 
{
	LPCTSTR pTest = "IRTEST1";
	LPCTSTR pCorrect = 
		"IS"
		"AS";
	TomboURI sStopURI;
	sStopURI.Init("tombo://default/aaa/");
	Repo4 rep(&sStopURI);

	TestScanner1::ScanTest(pTest, pCorrect, &rep, NULL, FALSE);
}

////////////////////////////////////////////////
// TEST
//
// cancelled at sub folder

void InterruptTest2() 
{
	LPCTSTR pTest = "IRTEST1";
	LPCTSTR pCorrect = 
		"IS"
		"BF" "tombo://default/aaa/" "aaa"
		"BF" "tombo://default/aaa/bbb/" "bbb"
		"AF" "tombo://default/aaa/bbb/" "bbb"
		"AF" "tombo://default/aaa/" "aaa"
		"AS";

	TomboURI sStopURI;
	sStopURI.Init("tombo://default/aaa/bbb/ccc/");
	Repo4 rep(&sStopURI);

	TestScanner1::ScanTest(pTest, pCorrect, &rep, NULL, FALSE);
}

////////////////////////////////////////////////
// TEST
//
// cancelled at sub folder

void InterruptTest3() 
{
	LPCTSTR pTest = "IRTEST1";
	LPCTSTR pCorrect = 
		"IS"
		"BF" "tombo://default/aaa/" "aaa"
		"BF" "tombo://default/aaa/bbb/" "bbb"
		"AF" "tombo://default/aaa/bbb/" "bbb"
		"AF" "tombo://default/aaa/" "aaa"
		"AS";

	TomboURI sStopURI;
	sStopURI.Init("tombo://default/aaa/bbb/ccc/");
	Repo4 rep(&sStopURI);

	TomboURI part;
	part.Init("tombo://default/aaa/bbb/ccc/ggg.txt");

	TestScanner1::ScanTest(pTest, pCorrect, &rep, &part, FALSE);
}

////////////////////////////////////////////////
// TEST
//
// cancelled at sub folder

void InterruptTest4() 
{
	LPCTSTR pTest = "IRTEST1";
	LPCTSTR pCorrect = 
		"IS"
		"BF" "tombo://default/aaa/" "aaa"
		"AF" "tombo://default/aaa/" "aaa"
		"AS";

	TomboURI sStopURI;
	sStopURI.Init("tombo://default/aaa/bbb/");
	Repo4 rep(&sStopURI);

	TomboURI part;
	part.Init("tombo://default/aaa/bbb/ccc/ggg.txt");

	TestScanner1::ScanTest(pTest, pCorrect, &rep, &part, FALSE);
}

