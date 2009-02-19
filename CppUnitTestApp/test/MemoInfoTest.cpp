#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>

#include <windows.h>
#include <tchar.h>
#include "TString.h"
#include "UniConv.h"
#include "MemoInfo.h"
#include "File.h"

#define TEST_CLASS_NAME MemoInfoTest

class TEST_CLASS_NAME : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(TEST_CLASS_NAME);
	CPPUNIT_TEST(WriteInfoTest1);
	CPPUNIT_TEST(ReadInfoTest1);
	CPPUNIT_TEST(RenameInfoTest1);
	CPPUNIT_TEST(DeleteInfoTest1);

	CPPUNIT_TEST(WriteInfoTest2);
	CPPUNIT_TEST(ReadInfoTest2);
	CPPUNIT_TEST(RenameInfoTest2);
	CPPUNIT_TEST(DeleteInfoTest2);

	CPPUNIT_TEST_SUITE_END();

	TCHAR path[MAX_PATH];
public:
	TEST_CLASS_NAME() {}
	~TEST_CLASS_NAME() {}

	virtual void setUp();
	virtual void tearDown() {}

	void WriteInfoTest1();
	void WriteInfoTest2();
	void ReadInfoTest1();
	void ReadInfoTest2();
	void RenameInfoTest1();
	void RenameInfoTest2();
	void DeleteInfoTest1();
	void DeleteInfoTest2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TEST_CLASS_NAME);

////////////////////////////////////////////////
void TEST_CLASS_NAME::setUp()
{
	// get current dir path
	TCHAR buf[MAX_PATH];
	GetModuleFileName(NULL, buf, MAX_PATH);
	GetFilePath(path, buf);

	wsprintf(buf, TEXT("%s%s"), path, TEXT("sub1"));
	CreateDirectory(buf, NULL);

	_tcscat(buf, TEXT("\\sub2"));
	CreateDirectory(buf, NULL);
}

////////////////////////////////////////////////

// Case: new Info
void TEST_CLASS_NAME::WriteInfoTest1() {

	TCHAR buf[MAX_PATH];
	wsprintf(buf, "%s%s", path, "aaa.tdt");

	DeleteFile(buf);

	MemoInfo mi(path);
	CPPUNIT_ASSERT(mi.WriteInfo("aaa", 0x12345678));


	File f;
	CPPUNIT_ASSERT(f.Open(buf, GENERIC_READ, 0, OPEN_EXISTING));
	CPPUNIT_ASSERT(f.FileSize() == 5);

	BYTE data[20];
	DWORD nSiz = 5;
	CPPUNIT_ASSERT(f.Read(data, &nSiz));
	CPPUNIT_ASSERT(nSiz == 5);

	CPPUNIT_ASSERT(data[0] == 1);
	CPPUNIT_ASSERT(data[1] == 0x78);
	CPPUNIT_ASSERT(data[2] == 0x56);
	CPPUNIT_ASSERT(data[3] == 0x34);
	CPPUNIT_ASSERT(data[4] == 0x12);
}

// Case: new Info
void TEST_CLASS_NAME::WriteInfoTest2() {

	TCHAR buf[MAX_PATH];
	wsprintf(buf, "%s%s", path, "sub1\\sub2\\bbb.tdt");

	DeleteFile(buf);

	MemoInfo mi(path);
	CPPUNIT_ASSERT(mi.WriteInfo("sub1\\sub2\\bbb", 0x12345678));


	File f;
	CPPUNIT_ASSERT(f.Open(buf, GENERIC_READ, 0, OPEN_EXISTING));
	CPPUNIT_ASSERT(f.FileSize() == 5);

	BYTE data[20];
	DWORD nSiz = 5;
	CPPUNIT_ASSERT(f.Read(data, &nSiz));
	CPPUNIT_ASSERT(nSiz == 5);

	CPPUNIT_ASSERT(data[0] == 1);
	CPPUNIT_ASSERT(data[1] == 0x78);
	CPPUNIT_ASSERT(data[2] == 0x56);
	CPPUNIT_ASSERT(data[3] == 0x34);
	CPPUNIT_ASSERT(data[4] == 0x12);
}

void TEST_CLASS_NAME::ReadInfoTest1()
{
	MemoInfo mi(path);
	DWORD n;

	CPPUNIT_ASSERT(mi.ReadInfo("aaa", &n));
	CPPUNIT_ASSERT(n == 0x12345678);
}

void TEST_CLASS_NAME::ReadInfoTest2()
{
	MemoInfo mi(path);
	DWORD n;

	CPPUNIT_ASSERT(mi.ReadInfo("sub1\\sub2\\bbb", &n));
	CPPUNIT_ASSERT(n == 0x12345678);
}

void TEST_CLASS_NAME::RenameInfoTest1()
{
	TCHAR n0[MAX_PATH];
	TCHAR n1[MAX_PATH];

	wsprintf(n0, "%s\\aaa", path);
	wsprintf(n1, "%s\\xxx", path);

	MemoInfo mi(path);
	CPPUNIT_ASSERT(mi.RenameInfo(n0, n1));

	TCHAR buf[MAX_PATH];
	wsprintf(buf, "%s%s", path, "aaa.tdt");

	File f;
	CPPUNIT_ASSERT(f.Open(buf, GENERIC_READ, 0, OPEN_EXISTING) == FALSE);
	CPPUNIT_ASSERT(GetLastError() == ERROR_FILE_NOT_FOUND);

	wsprintf(buf, "%s%s", path, "xxx.tdt");
	File f2;
	CPPUNIT_ASSERT(f2.Open(buf, GENERIC_READ, 0, OPEN_EXISTING));
	f2.Close();

	MemoInfo mi2(path);
	DWORD n;
	CPPUNIT_ASSERT(mi2.ReadInfo("xxx", &n));
	CPPUNIT_ASSERT(n == 0x12345678);

}

void TEST_CLASS_NAME::RenameInfoTest2()
{
	TCHAR n0[MAX_PATH];
	TCHAR n1[MAX_PATH];

	wsprintf(n0, "%s\\%s", path, "sub1\\sub2\\bbb");
	wsprintf(n1, "%s\\%s", path, "sub1\\sub2\\yyy");

	MemoInfo mi(path);
	CPPUNIT_ASSERT(mi.RenameInfo(n0, n1));

	TCHAR buf[MAX_PATH];
	wsprintf(buf, "%s%s", path, "sub1\\sub2\\bbb.tdt");

	File f;
	CPPUNIT_ASSERT(f.Open(buf, GENERIC_READ, 0, OPEN_EXISTING) == FALSE);
	CPPUNIT_ASSERT(GetLastError() == ERROR_FILE_NOT_FOUND);

	wsprintf(buf, "%s%s", path, "sub1\\sub2\\yyy.tdt");
	File f2;
	CPPUNIT_ASSERT(f2.Open(buf, GENERIC_READ, 0, OPEN_EXISTING));
	f2.Close();

	MemoInfo mi2(path);
	DWORD n;
	CPPUNIT_ASSERT(mi2.ReadInfo("sub1\\sub2\\yyy", &n));
	CPPUNIT_ASSERT(n == 0x12345678);

}

void TEST_CLASS_NAME::DeleteInfoTest1()
{
	MemoInfo mi(path);
	CPPUNIT_ASSERT(mi.DeleteInfo("xxx"));

	TCHAR buf[MAX_PATH];
	wsprintf(buf, "%s%s", path, "xxx.tdt");

	File f;
	CPPUNIT_ASSERT(f.Open(buf, GENERIC_READ, 0, OPEN_EXISTING) == FALSE);
	CPPUNIT_ASSERT(GetLastError() == ERROR_FILE_NOT_FOUND);
}

void TEST_CLASS_NAME::DeleteInfoTest2()
{
	MemoInfo mi(path);
	CPPUNIT_ASSERT(mi.DeleteInfo("sub1\\sub2\\yyy"));

	TCHAR buf[MAX_PATH];
	wsprintf(buf, "%s%s", path, "sub1\\sub2\\yyy.tdt");

	File f;
	CPPUNIT_ASSERT(f.Open(buf, GENERIC_READ, 0, OPEN_EXISTING) == FALSE);
	CPPUNIT_ASSERT(GetLastError() == ERROR_FILE_NOT_FOUND);
}