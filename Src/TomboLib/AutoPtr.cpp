#include <windows.h>
#include "AutoPtr.h"

SecureBufferAutoPointerT::~SecureBufferAutoPointerT() {
	if (pPtr == NULL) return;
	LPTSTR q = pPtr;
	while(*q) *q++ = TEXT('\0');
	delete [] pPtr;
}

SecureBufferAutoPointerA::~SecureBufferAutoPointerA() {
	if (pPtr == NULL) return;
	char *q = pPtr;
	while(*q) *q++ = TEXT('\0');
	delete [] pPtr;
}

SecureBufferAutoPointerW::~SecureBufferAutoPointerW() {
	if (pPtr == NULL) return;
	LPWSTR q = pPtr;
	while(*q) *q++ = TEXT('\0');
	delete [] pPtr;
}

SecureBufferAutoPointerByte::~SecureBufferAutoPointerByte() {
	Clear();
}

void SecureBufferAutoPointerByte::Clear()
{
	if (pPtr == NULL) return;
	LPBYTE p = pPtr;
	for (DWORD i = 0; i < nLen; i++) {
		*p++ = 0;
	}
	delete [] pPtr;
	pPtr = NULL;
}