#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include <windows.h>
#include <tchar.h>
#include "TString.h"
#include "TomboURI.h"

#define TEST_CLASS_NAME TomboURITest

class TEST_CLASS_NAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TEST_CLASS_NAME);
	CPPUNIT_TEST(InitTest1);
	CPPUNIT_TEST(InitTest2);
	CPPUNIT_TEST(InitTest3);
	CPPUNIT_TEST(InitTest4);
	CPPUNIT_TEST(InitTest5);
	CPPUNIT_TEST(InitTest6);
	CPPUNIT_TEST(InitTest7);
	CPPUNIT_TEST(GetRepoTest1);
	CPPUNIT_TEST(IteratorTest1);
	CPPUNIT_TEST(IteratorTest2);
	CPPUNIT_TEST(IteratorTest3);
	CPPUNIT_TEST(GetParentTest1);
	CPPUNIT_TEST(GetParentTest2);
	CPPUNIT_TEST(GetParentTest3);
	CPPUNIT_TEST(GetParentTest4);
	CPPUNIT_TEST(GetParentTest5);
	CPPUNIT_TEST(GetParentTest6);
	CPPUNIT_TEST(IsLeafTest1);
	CPPUNIT_TEST(IsLeafTest2);
	CPPUNIT_TEST(IsLeafTest3);
	CPPUNIT_TEST(GetFilePathTest1);
	CPPUNIT_TEST(GetFilePathTest2);
	CPPUNIT_TEST(GetFilePathTest3);
	CPPUNIT_TEST(GetBaseNameTest1);
	CPPUNIT_TEST(GetBaseNameTest2);
	CPPUNIT_TEST(GetBaseNameTest3);
	CPPUNIT_TEST(IsRootTest1);
	CPPUNIT_TEST(IsRootTest2);
	CPPUNIT_TEST(IsRootTest3);
	CPPUNIT_TEST_SUITE_END();

public:
	TEST_CLASS_NAME() {}
	~TEST_CLASS_NAME() {}

	virtual void setUp() {}
	virtual void tearDown() {}

	void InitTest1();
	void InitTest2();
	void InitTest3();
	void InitTest4();
	void InitTest5();
	void InitTest6();
	void InitTest7();
	void GetRepoTest1();
	void IteratorTest1();
	void IteratorTest2();
	void IteratorTest3();
	void GetParentTest1();
	void GetParentTest2();
	void GetParentTest3();
	void GetParentTest4();
	void GetParentTest5();
	void GetParentTest6();
	void IsLeafTest1();
	void IsLeafTest2();
	void IsLeafTest3();
	void GetFilePathTest1();
	void GetFilePathTest2();
	void GetFilePathTest3();
	void GetBaseNameTest1();
	void GetBaseNameTest2();
	void GetBaseNameTest3();
	void IsRootTest1();
	void IsRootTest2();
	void IsRootTest3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TEST_CLASS_NAME);

////////////////////////////////////////////////

void TEST_CLASS_NAME::InitTest1() {
	// no header information. fail.

	TomboURI uri;
	BOOL b = uri.Init("test");
	CPPUNIT_ASSERT(!b);
	CPPUNIT_ASSERT(GetLastError() == ERROR_INVALID_DATA);
}

void TEST_CLASS_NAME::InitTest2() {
	// incomplete repository definition. fail.

	TomboURI uri;
	BOOL b = uri.Init("tombo://default");
	CPPUNIT_ASSERT(!b);
	CPPUNIT_ASSERT(GetLastError() == ERROR_INVALID_DATA);
}

void TEST_CLASS_NAME::InitTest3() {
	// directs root of default repository.

	TomboURI uri;
	BOOL b = uri.Init("tombo://default/");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(uri.nMaxPathItem == 7);
}

void TEST_CLASS_NAME::InitTest4() {
	// exists first item (no path sep)

	TomboURI uri;
	BOOL b = uri.Init("tombo://default/hello");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(uri.nMaxPathItem == 7);
}

void TEST_CLASS_NAME::InitTest5() {
	// exists first item (with path sep)

	TomboURI uri;
	BOOL b = uri.Init("tombo://default/hello/");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(uri.nMaxPathItem == 7);
}

void TEST_CLASS_NAME::InitTest6() {

	TomboURI uri;
	BOOL b = uri.Init("tombo://default/hello world");
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(uri.nMaxPathItem == 11);
}

void TEST_CLASS_NAME::InitTest7() {
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/test.txt"));

	TomboURI uri2(uri);
	CPPUNIT_ASSERT(uri.GetFullURI() == uri2.GetFullURI());

	TomboURI uri3;
	uri3.Init(uri);
	CPPUNIT_ASSERT(uri.GetFullURI() == uri2.GetFullURI());
}

void TEST_CLASS_NAME::GetRepoTest1() {
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/"));
	
	TString repo;
	BOOL b = uri.GetRepositoryName(&repo);
	CPPUNIT_ASSERT(b);
	CPPUNIT_ASSERT(strcmp(repo.Get(), "default") == 0);
}

void TEST_CLASS_NAME::IteratorTest1() {
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/"));

	TomboURIItemIterator itr(&uri);
	CPPUNIT_ASSERT(itr.Init());
	LPCTSTR p;
	itr.First();
	p = itr.Current();
	CPPUNIT_ASSERT(p == NULL);
	CPPUNIT_ASSERT(itr.IsLeaf() == FALSE);
}

void TEST_CLASS_NAME::IteratorTest2() {
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa/bbb/ccc.txt"));

	TomboURIItemIterator itr(&uri);
	CPPUNIT_ASSERT(itr.Init());
	LPCTSTR p;
	itr.First();

	p = itr.Current();
	CPPUNIT_ASSERT(strcmp(p, "aaa") == 0);
	CPPUNIT_ASSERT(itr.IsLeaf() == FALSE);
	itr.Next();

	p = itr.Current();
	CPPUNIT_ASSERT(strcmp(p, "bbb") == 0);
	CPPUNIT_ASSERT(itr.IsLeaf() == FALSE);
	itr.Next();

	p = itr.Current();
	CPPUNIT_ASSERT(strcmp(p, "ccc.txt") == 0);
	CPPUNIT_ASSERT(itr.IsLeaf() == TRUE);
	itr.Next();

	p = itr.Current();
	CPPUNIT_ASSERT(p == NULL);

	itr.Next();
	CPPUNIT_ASSERT(p == NULL);

}

void TEST_CLASS_NAME::IteratorTest3() {
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa/bbb/"));

	TomboURIItemIterator itr(&uri);
	CPPUNIT_ASSERT(itr.Init());
	LPCTSTR p;
	itr.First();

	p = itr.Current();
	CPPUNIT_ASSERT(strcmp(p, "aaa") == 0);
	CPPUNIT_ASSERT(itr.IsLeaf() == FALSE);
	itr.Next();

	p = itr.Current();
	CPPUNIT_ASSERT(strcmp(p, "bbb") == 0);
	CPPUNIT_ASSERT(itr.IsLeaf() == FALSE);
	itr.Next();

	p = itr.Current();
	CPPUNIT_ASSERT(p == NULL);

}

void TEST_CLASS_NAME::GetParentTest1()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa/bbb/ccc/ddd.txt"));
	TomboURI sParent;
	CPPUNIT_ASSERT(uri.GetParent(&sParent));

	CPPUNIT_ASSERT(strcmp(sParent.GetFullURI(), "tombo://default/aaa/bbb/ccc/") == 0);

}

void TEST_CLASS_NAME::GetParentTest2()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa/bbb/ccc/"));
	TomboURI sParent;
	CPPUNIT_ASSERT(uri.GetParent(&sParent));

	CPPUNIT_ASSERT(strcmp(sParent.GetFullURI(), "tombo://default/aaa/bbb/") == 0);
}

void TEST_CLASS_NAME::GetParentTest3()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa/bbb/ccc"));
	TomboURI sParent;
	CPPUNIT_ASSERT(uri.GetParent(&sParent));

	CPPUNIT_ASSERT(strcmp(sParent.GetFullURI(), "tombo://default/aaa/bbb/") == 0);
}

void TEST_CLASS_NAME::GetParentTest4()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/"));
	TomboURI sParent;
	CPPUNIT_ASSERT(uri.GetParent(&sParent));
	CPPUNIT_ASSERT(strcmp(sParent.GetFullURI(), "tombo://default/") == 0);
}

void TEST_CLASS_NAME::GetParentTest5()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa.txt"));
	TomboURI sParent;
	CPPUNIT_ASSERT(uri.GetParent(&sParent));
	CPPUNIT_ASSERT(strcmp(sParent.GetFullURI(), "tombo://default/") == 0);
}

void TEST_CLASS_NAME::GetParentTest6()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa/ddd.txt"));
	TomboURI sParent;
	CPPUNIT_ASSERT(uri.GetParent(&sParent));

	CPPUNIT_ASSERT(strcmp(sParent.GetFullURI(), "tombo://default/aaa/") == 0);

}

void TEST_CLASS_NAME::IsLeafTest1()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/"));
	CPPUNIT_ASSERT(uri.IsLeaf() == FALSE);
}

void TEST_CLASS_NAME::IsLeafTest2()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa/"));
	CPPUNIT_ASSERT(uri.IsLeaf() == FALSE);
}

void TEST_CLASS_NAME::IsLeafTest3()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa"));
	CPPUNIT_ASSERT(uri.IsLeaf() == TRUE);
}

void TEST_CLASS_NAME::GetFilePathTest1()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/"));

	TString sPath;
	CPPUNIT_ASSERT(uri.GetFilePath(&sPath));
	CPPUNIT_ASSERT(_tcscpy(sPath.Get(), TEXT("")));
}

void TEST_CLASS_NAME::GetFilePathTest2()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa"));

	TString sPath;
	CPPUNIT_ASSERT(uri.GetFilePath(&sPath));
	CPPUNIT_ASSERT(_tcscpy(sPath.Get(), TEXT("aaa")));

}

void TEST_CLASS_NAME::GetFilePathTest3()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa/bbb/ccc/"));

	TString sPath;
	CPPUNIT_ASSERT(uri.GetFilePath(&sPath));
	CPPUNIT_ASSERT(_tcscpy(sPath.Get(), TEXT("aaa\\bbb\\ccc\\")));
}

void TEST_CLASS_NAME::GetBaseNameTest1()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa/bbb/ccc/dd.txt"));

	TString sBase;
	CPPUNIT_ASSERT(uri.GetBaseName(&sBase));
	CPPUNIT_ASSERT(_tcscmp(sBase.Get(), TEXT("dd.txt")) == 0);
}

void TEST_CLASS_NAME::GetBaseNameTest2()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa/bbb/ccc/"));

	TString sBase;
	CPPUNIT_ASSERT(uri.GetBaseName(&sBase));
	CPPUNIT_ASSERT(_tcscmp(sBase.Get(), TEXT("ccc")) == 0);
}

void TEST_CLASS_NAME::GetBaseNameTest3()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/"));

	TString sBase;
	CPPUNIT_ASSERT(uri.GetBaseName(&sBase));
	CPPUNIT_ASSERT(_tcscmp(sBase.Get(), TEXT("")) == 0);
}

void TEST_CLASS_NAME::IsRootTest1()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/"));
	CPPUNIT_ASSERT(uri.IsRoot() == TRUE);
}

void TEST_CLASS_NAME::IsRootTest2()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://default/aaa.txt"));
	CPPUNIT_ASSERT(uri.IsRoot() == FALSE);
}

void TEST_CLASS_NAME::IsRootTest3()
{
	TomboURI uri;
	CPPUNIT_ASSERT(uri.Init("tombo://repo/"));
	CPPUNIT_ASSERT(uri.IsRoot() == TRUE);
}