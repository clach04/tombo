#include <windows.h>
#include "../stdafx.h"

#include "VarBuffer.h"

static TestRunner *runner;

static void test1();
static void test2();
static void test3();
static void test4();
static void test5();

void VarBufferTest(TestRunner *r) {
	runner = r;

	runner->WriteMsg("VarBufferTest\r\n");
	test1();
	test2();
	test3();
	test4();
	test5();
}

void test1() {
	VarBufferImpl vb;
	DWORD nc;

	runner->assert(vb.Init(10, 10));
	runner->assert(LocalSize(vb.GetBuffer()) == 10);

	// initial data. not extended
	char buf[20];
	strcpy(buf, "0123456789abcdef");
	runner->assert(vb.Add((LPBYTE)buf, 5, NULL));
	runner->assert(LocalSize(vb.GetBuffer()) == 10);
	runner->assert(vb.CurrentUse() == 5);
	runner->assert(strncmp((const char*)vb.GetBuffer(), "01234", 5) == 0);

	// append data
	runner->assert(vb.Add((LPBYTE)(buf + 5), 4, &nc));
	runner->assert(LocalSize(vb.GetBuffer()) == 10);
	runner->assert(vb.CurrentUse() == 9);
	runner->assert(strncmp((const char*)vb.GetBuffer(), "012345678", 9) == 0);
	runner->assert(nc == 5);

	// append data exntend memory
	runner->assert(vb.Add((LPBYTE)(buf + 9), 1, &nc));
	runner->assert(LocalSize(vb.GetBuffer()) == 20);
	runner->assert(vb.CurrentUse() == 10);
	runner->assert(strncmp((const char*)vb.GetBuffer(), "0123456789", 10) == 0);
	runner->assert(nc == 9);
}

void test2() {
	VarBufferImpl vb;

	runner->assert(vb.Init(5, 5));
	runner->assert(LocalSize(vb.GetBuffer()) == 5);

	// extend multi blocks
	char buf[30];
	runner->assert(vb.Add((LPBYTE)buf, 19, NULL));
	runner->assert(LocalSize(vb.GetBuffer()) == 20);
	runner->assert(vb.CurrentUse() == 19);
}

void test3() {
	////////
	// clear test

	char buf[30];

	VarBufferImpl vb1;
	runner->assert(vb1.Init(5, 5));
	runner->assert(LocalSize(vb1.GetBuffer()) == 5);

	runner->assert(vb1.Add((LPBYTE)buf, 20, NULL));
	runner->assert(LocalSize(vb1.GetBuffer()) == 25);
	runner->assert(vb1.CurrentUse() == 20);

	// clear but keep buffer
	runner->assert(vb1.Clear(FALSE));
	runner->assert(vb1.CurrentUse() == 0);
	runner->assert(LocalSize(vb1.GetBuffer()) == 25);

	// clear and realloc buffer
	runner->assert(vb1.Clear(TRUE));
	runner->assert(vb1.CurrentUse() == 0);
	runner->assert(LocalSize(vb1.GetBuffer()) == 5);

}

void test4()
{
	// extend test
	const char *pSample = "0123456789abcdef";

	VarBufferImpl vb;
	runner->assert(vb.Init(10, 5));
	runner->assert(LocalSize(vb.GetBuffer()) == 10);
	runner->assert(vb.Add((LPBYTE)pSample, 5, NULL));

	runner->assert(vb.Extend(0, 2));
	runner->assert(strncmp((const char*)vb.GetBuffer() + 2, "012345678", 5) == 0);

	runner->assert(vb.Extend(7, 2));
	runner->assert(vb.CurrentUse() == 9);

	runner->assert(vb.Extend(4, 2));
	runner->assert(vb.CurrentUse() == 11);
	runner->assert(strncmp((const char*)vb.GetBuffer() + 2, "012", 3) == 0);
	runner->assert(strncmp((const char*)vb.GetBuffer() + 7, "345", 2) == 0);
}

void test5()
{
	// shorten test
	const char *pSample = "0123456789abcdef";
	VarBufferImpl vb;

	runner->assert(vb.Init(10, 5));
	runner->assert(LocalSize(vb.GetBuffer()) == 10);
	runner->assert(vb.Add((LPBYTE)pSample, 9, NULL));
	runner->assert(strncmp((const char*)vb.GetBuffer(), pSample, 9) == 0);

	runner->assert(vb.Shorten(0, 2));
	runner->assert(vb.CurrentUse() == 7);
	runner->assert(strncmp((const char*)vb.GetBuffer(), pSample + 2, 7) == 0);

	runner->assert(vb.Shorten(5, 2));
	runner->assert(vb.CurrentUse() == 5);
	runner->assert(strncmp((const char*)vb.GetBuffer(), pSample + 2, 5) == 0);

}
