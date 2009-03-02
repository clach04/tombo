#include <windows.h>
#include "../stdafx.h"
#include "TString.h"
#include "UniConv.h"
#include "MemoInfo.h"
#include "File.h"

static TestRunner *runner;
static TCHAR path[MAX_PATH];


static void setUp();

static void WriteInfoTest1();
static void WriteInfoTest2();
static void ReadInfoTest1();
static void ReadInfoTest2();
static void RenameInfoTest1();
static void RenameInfoTest2();
static void DeleteInfoTest1();
static void DeleteInfoTest2();

void MemoInfoTest(TestRunner *r) {
	runner = r;
	runner->WriteMsg("MemoInfoTest\r\n");

	setUp();

	WriteInfoTest1();
	WriteInfoTest2();
	ReadInfoTest1();
	ReadInfoTest2();
	RenameInfoTest1();
	RenameInfoTest2();
	DeleteInfoTest1();
	DeleteInfoTest2();
}


void setUp()
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
void WriteInfoTest1() {

	TCHAR buf[MAX_PATH];
	wsprintf(buf, "%s%s", path, "aaa.tdt");

	DeleteFile(buf);

	MemoInfo mi(path);
	runner->assert(mi.WriteInfo("aaa", 0x12345678));


	File f;
	runner->assert(f.Open(buf, GENERIC_READ, 0, OPEN_EXISTING));
	runner->assert(f.FileSize() == 5);

	BYTE data[20];
	DWORD nSiz = 5;
	runner->assert(f.Read(data, &nSiz));
	runner->assert(nSiz == 5);

	runner->assert(data[0] == 1);
	runner->assert(data[1] == 0x78);
	runner->assert(data[2] == 0x56);
	runner->assert(data[3] == 0x34);
	runner->assert(data[4] == 0x12);
}

// Case: new Info
void WriteInfoTest2() {

	TCHAR buf[MAX_PATH];
	wsprintf(buf, "%s%s", path, "sub1\\sub2\\bbb.tdt");

	DeleteFile(buf);

	MemoInfo mi(path);
	runner->assert(mi.WriteInfo("sub1\\sub2\\bbb", 0x12345678));


	File f;
	runner->assert(f.Open(buf, GENERIC_READ, 0, OPEN_EXISTING));
	runner->assert(f.FileSize() == 5);

	BYTE data[20];
	DWORD nSiz = 5;
	runner->assert(f.Read(data, &nSiz));
	runner->assert(nSiz == 5);

	runner->assert(data[0] == 1);
	runner->assert(data[1] == 0x78);
	runner->assert(data[2] == 0x56);
	runner->assert(data[3] == 0x34);
	runner->assert(data[4] == 0x12);
}

void ReadInfoTest1()
{
	MemoInfo mi(path);
	DWORD n;

	runner->assert(mi.ReadInfo("aaa", &n));
	runner->assert(n == 0x12345678);
}

void ReadInfoTest2()
{
	MemoInfo mi(path);
	DWORD n;

	runner->assert(mi.ReadInfo("sub1\\sub2\\bbb", &n));
	runner->assert(n == 0x12345678);
}

void RenameInfoTest1()
{
	TCHAR n0[MAX_PATH];
	TCHAR n1[MAX_PATH];

	wsprintf(n0, "%s\\aaa", path);
	wsprintf(n1, "%s\\xxx", path);

	MemoInfo mi(path);
	runner->assert(mi.RenameInfo(n0, n1));

	TCHAR buf[MAX_PATH];
	wsprintf(buf, "%s%s", path, "aaa.tdt");

	File f;
	runner->assert(f.Open(buf, GENERIC_READ, 0, OPEN_EXISTING) == FALSE);
	runner->assert(GetLastError() == ERROR_FILE_NOT_FOUND);

	wsprintf(buf, "%s%s", path, "xxx.tdt");
	File f2;
	runner->assert(f2.Open(buf, GENERIC_READ, 0, OPEN_EXISTING));
	f2.Close();

	MemoInfo mi2(path);
	DWORD n;
	runner->assert(mi2.ReadInfo("xxx", &n));
	runner->assert(n == 0x12345678);

}

void RenameInfoTest2()
{
	TCHAR n0[MAX_PATH];
	TCHAR n1[MAX_PATH];

	wsprintf(n0, "%s\\%s", path, "sub1\\sub2\\bbb");
	wsprintf(n1, "%s\\%s", path, "sub1\\sub2\\yyy");

	MemoInfo mi(path);
	runner->assert(mi.RenameInfo(n0, n1));

	TCHAR buf[MAX_PATH];
	wsprintf(buf, "%s%s", path, "sub1\\sub2\\bbb.tdt");

	File f;
	runner->assert(f.Open(buf, GENERIC_READ, 0, OPEN_EXISTING) == FALSE);
	runner->assert(GetLastError() == ERROR_FILE_NOT_FOUND);

	wsprintf(buf, "%s%s", path, "sub1\\sub2\\yyy.tdt");
	File f2;
	runner->assert(f2.Open(buf, GENERIC_READ, 0, OPEN_EXISTING));
	f2.Close();

	MemoInfo mi2(path);
	DWORD n;
	runner->assert(mi2.ReadInfo("sub1\\sub2\\yyy", &n));
	runner->assert(n == 0x12345678);

}

void DeleteInfoTest1()
{
	MemoInfo mi(path);
	runner->assert(mi.DeleteInfo("xxx"));

	TCHAR buf[MAX_PATH];
	wsprintf(buf, "%s%s", path, "xxx.tdt");

	File f;
	runner->assert(f.Open(buf, GENERIC_READ, 0, OPEN_EXISTING) == FALSE);
	runner->assert(GetLastError() == ERROR_FILE_NOT_FOUND);
}

void DeleteInfoTest2()
{
	MemoInfo mi(path);
	runner->assert(mi.DeleteInfo("sub1\\sub2\\yyy"));

	TCHAR buf[MAX_PATH];
	wsprintf(buf, "%s%s", path, "sub1\\sub2\\yyy.tdt");

	File f;
	runner->assert(f.Open(buf, GENERIC_READ, 0, OPEN_EXISTING) == FALSE);
	runner->assert(GetLastError() == ERROR_FILE_NOT_FOUND);
}