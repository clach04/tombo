#include <windows.h>
#include "../stdafx.h"

#include "TString.h"
#include "TomboURI.h"


static TestRunner *runner;

static void InitTest1();
static void InitTest2();
static void InitTest3();
static void InitTest4();
static void InitTest5();
static void InitTest6();
static void InitTest7();
static void GetRepoTest1();
static void IteratorTest1();
static void IteratorTest2();
static void IteratorTest3();
static void GetParentTest1();
static void GetParentTest2();
static void GetParentTest3();
static void GetParentTest4();
static void GetParentTest5();
static void GetParentTest6();
static void IsLeafTest1();
static void IsLeafTest2();
static void IsLeafTest3();
static void GetFilePathTest1();
static void GetFilePathTest2();
static void GetFilePathTest3();
static void GetBaseNameTest1();
static void GetBaseNameTest2();
static void GetBaseNameTest3();
static void IsRootTest1();
static void IsRootTest2();
static void IsRootTest3();

void TomboURITest(TestRunner *r) {
	runner = r;
	runner->WriteMsg("TomboURITest\r\n");

	InitTest1();
	InitTest2();
	InitTest3();
	InitTest4();
	InitTest5();
	InitTest6();
	InitTest7();
	GetRepoTest1();
	IteratorTest1();
	IteratorTest2();
	IteratorTest3();
	GetParentTest1();
	GetParentTest2();
	GetParentTest3();
	GetParentTest4();
	GetParentTest5();
	GetParentTest6();
	IsLeafTest1();
	IsLeafTest2();
	IsLeafTest3();
	GetFilePathTest1();
	GetFilePathTest2();
	GetFilePathTest3();
	GetBaseNameTest1();
	GetBaseNameTest2();
	GetBaseNameTest3();
	IsRootTest1();
	IsRootTest2();
	IsRootTest3();
}



void InitTest1() {
	// no header information. fail.

	TomboURI uri;
	BOOL b = uri.Init("test");
	runner->assert(!b);
	runner->assert(GetLastError() == ERROR_INVALID_DATA);
}

void InitTest2() {
	// incomplete repository definition. fail.

	TomboURI uri;
	BOOL b = uri.Init("tombo://default");
	runner->assert(!b);
	runner->assert(GetLastError() == ERROR_INVALID_DATA);
}

void InitTest3() {
	// directs root of default repository.

	TomboURI uri;
	BOOL b = uri.Init("tombo://default/");
	runner->assert(b);
	runner->assert(uri.nMaxPathItem == 7);
}

void InitTest4() {
	// exists first item (no path sep)

	TomboURI uri;
	BOOL b = uri.Init("tombo://default/hello");
	runner->assert(b);
	runner->assert(uri.nMaxPathItem == 7);
}

void InitTest5() {
	// exists first item (with path sep)

	TomboURI uri;
	BOOL b = uri.Init("tombo://default/hello/");
	runner->assert(b);
	runner->assert(uri.nMaxPathItem == 7);
}

void InitTest6() {

	TomboURI uri;
	BOOL b = uri.Init("tombo://default/hello world");
	runner->assert(b);
	runner->assert(uri.nMaxPathItem == 11);
}

void InitTest7() {
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/test.txt"));

	TomboURI uri2(uri);
	runner->assert(uri.GetFullURI() == uri2.GetFullURI());

	TomboURI uri3;
	uri3.Init(uri);
	runner->assert(uri.GetFullURI() == uri2.GetFullURI());
}

void GetRepoTest1() {
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/"));
	
	TString repo;
	BOOL b = uri.GetRepositoryName(&repo);
	runner->assert(b);
	runner->assert(strcmp(repo.Get(), "default") == 0);
}

void IteratorTest1() {
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/"));

	TomboURIItemIterator itr(&uri);
	runner->assert(itr.Init());
	LPCTSTR p;
	itr.First();
	p = itr.Current();
	runner->assert(p == NULL);
	runner->assert(itr.IsLeaf() == FALSE);
}

void IteratorTest2() {
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/aaa/bbb/ccc.txt"));

	TomboURIItemIterator itr(&uri);
	runner->assert(itr.Init());
	LPCTSTR p;
	itr.First();

	p = itr.Current();
	runner->assert(strcmp(p, "aaa") == 0);
	runner->assert(itr.IsLeaf() == FALSE);
	itr.Next();

	p = itr.Current();
	runner->assert(strcmp(p, "bbb") == 0);
	runner->assert(itr.IsLeaf() == FALSE);
	itr.Next();

	p = itr.Current();
	runner->assert(strcmp(p, "ccc.txt") == 0);
	runner->assert(itr.IsLeaf() == TRUE);
	itr.Next();

	p = itr.Current();
	runner->assert(p == NULL);

	itr.Next();
	runner->assert(p == NULL);

}

void IteratorTest3() {
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/aaa/bbb/"));

	TomboURIItemIterator itr(&uri);
	runner->assert(itr.Init());
	LPCTSTR p;
	itr.First();

	p = itr.Current();
	runner->assert(strcmp(p, "aaa") == 0);
	runner->assert(itr.IsLeaf() == FALSE);
	itr.Next();

	p = itr.Current();
	runner->assert(strcmp(p, "bbb") == 0);
	runner->assert(itr.IsLeaf() == FALSE);
	itr.Next();

	p = itr.Current();
	runner->assert(p == NULL);

}

void GetParentTest1()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/aaa/bbb/ccc/ddd.txt"));
	TomboURI sParent;
	runner->assert(uri.GetParent(&sParent));

	runner->assert(strcmp(sParent.GetFullURI(), "tombo://default/aaa/bbb/ccc/") == 0);

}

void GetParentTest2()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/aaa/bbb/ccc/"));
	TomboURI sParent;
	runner->assert(uri.GetParent(&sParent));

	runner->assert(strcmp(sParent.GetFullURI(), "tombo://default/aaa/bbb/") == 0);
}

void GetParentTest3()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/aaa/bbb/ccc"));
	TomboURI sParent;
	runner->assert(uri.GetParent(&sParent));

	runner->assert(strcmp(sParent.GetFullURI(), "tombo://default/aaa/bbb/") == 0);
}

void GetParentTest4()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/"));
	TomboURI sParent;
	runner->assert(uri.GetParent(&sParent));
	runner->assert(strcmp(sParent.GetFullURI(), "tombo://default/") == 0);
}

void GetParentTest5()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/aaa.txt"));
	TomboURI sParent;
	runner->assert(uri.GetParent(&sParent));
	runner->assert(strcmp(sParent.GetFullURI(), "tombo://default/") == 0);
}

void GetParentTest6()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/aaa/ddd.txt"));
	TomboURI sParent;
	runner->assert(uri.GetParent(&sParent));

	runner->assert(strcmp(sParent.GetFullURI(), "tombo://default/aaa/") == 0);

}

void IsLeafTest1()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/"));
	runner->assert(uri.IsLeaf() == FALSE);
}

void IsLeafTest2()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/aaa/"));
	runner->assert(uri.IsLeaf() == FALSE);
}

void IsLeafTest3()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/aaa"));
	runner->assert(uri.IsLeaf() == TRUE);
}

void GetFilePathTest1()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/"));

	TString sPath;
	runner->assert(uri.GetFilePath(&sPath));
	runner->assert(_tcscmp(sPath.Get(), TEXT("")) == 0);
}

void GetFilePathTest2()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/aaa"));

	TString sPath;
	runner->assert(uri.GetFilePath(&sPath));
	runner->assert(_tcscmp(sPath.Get(), TEXT("aaa")) == 0);

}

void GetFilePathTest3()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/aaa/bbb/ccc/"));

	TString sPath;
	runner->assert(uri.GetFilePath(&sPath));
	runner->assert(_tcscmp(sPath.Get(), TEXT("aaa\\bbb\\ccc\\")) == 0);
}

void GetBaseNameTest1()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/aaa/bbb/ccc/dd.txt"));

	TString sBase;
	runner->assert(uri.GetBaseName(&sBase));
	runner->assert(_tcscmp(sBase.Get(), TEXT("dd.txt")) == 0);
}

void GetBaseNameTest2()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/aaa/bbb/ccc/"));

	TString sBase;
	runner->assert(uri.GetBaseName(&sBase));
	runner->assert(_tcscmp(sBase.Get(), TEXT("ccc")) == 0);
}

void GetBaseNameTest3()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/"));

	TString sBase;
	runner->assert(uri.GetBaseName(&sBase));
	runner->assert(_tcscmp(sBase.Get(), TEXT("")) == 0);
}

void IsRootTest1()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/"));
	runner->assert(uri.IsRoot() == TRUE);
}

void IsRootTest2()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://default/aaa.txt"));
	runner->assert(uri.IsRoot() == FALSE);
}

void IsRootTest3()
{
	TomboURI uri;
	runner->assert(uri.Init("tombo://repo/"));
	runner->assert(uri.IsRoot() == TRUE);
}
