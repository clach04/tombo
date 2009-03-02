#include <windows.h>
#include "../stdafx.h"

#include "TString.h"

static TestRunner *runner;

static void test1();
static void test2();
static void test3();

void SharedStringTest(TestRunner *r) {
	runner = r;
	runner->WriteMsg("SharedStringTest\r\n");

	test1();
	test2();
	test3();
}

void test1() {
	SharedString ss;

	runner->assert(ss.Init("TEST"));
	runner->assert(strcmp(ss.Get(), "TEST") == 0);
}

void test2() {
	SharedString ss;

	runner->assert(ss.Init("TEST"));
	runner->assert(strcmp(ss.Get(), "TEST") == 0);

	SharedString ss2(ss);
	runner->assert(strcmp(ss2.Get(), "TEST") == 0);
	runner->assert(ss.Get() == ss2.Get());
	runner->assert(ss2.GetBuf()->nRefCount == 2);

	SharedString ss3;
	runner->assert(ss3.Init(ss2));
	runner->assert(strcmp(ss3.Get(), "TEST") == 0);
	runner->assert(ss3.Get() == ss.Get());
	runner->assert(ss3.GetBuf()->nRefCount == 3);
}

void test3() {
	SharedString ss;
	runner->assert(ss.Init("TEST"));
	runner->assert(strcmp(ss.Get(), "TEST") == 0);

	{
		SharedString ss2(ss);
		runner->assert(strcmp(ss2.Get(), "TEST") == 0);
		runner->assert(ss.Get() == ss2.Get());
		runner->assert(ss2.GetBuf()->nRefCount == 2);
	}

	runner->assert(strcmp(ss.Get(), "TEST") == 0);
	runner->assert(ss.GetBuf()->nRefCount == 1);
}
