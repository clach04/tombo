#ifndef TOMBO_AUTOPTR_H
#define TOMBO_AUTOPTR_H

template <class T>
class AutoPointer {
	T* pPtr;
public:
	AutoPointer(T* p = NULL) : pPtr(p) {}
	~AutoPointer() { delete pPtr; }

	void set(T* p) { pPtr = p; }
	void releaseAndSet(T* p) { delete pPtr; pPtr = p; }
};

template <class T>
class ArrayAutoPointer {
	T* pPtr;
public:
	ArrayAutoPointer(T* p = NULL) : pPtr(p) {}
	~ArrayAutoPointer() { delete [] pPtr; }

	void set(T* p) { pPtr = p; }
	void releaseAndSet(T* p) { delete [] pPtr; pPtr = p; }
};

class SecureBufferAutoPointerT {
	LPTSTR pPtr;
public:
	SecureBufferAutoPointerT(LPTSTR p) : pPtr(p) {}
	~SecureBufferAutoPointerT();
};

class SecureBufferAutoPointerA {
	char *pPtr;
public:
	SecureBufferAutoPointerA(char *p) : pPtr(p) {}
	~SecureBufferAutoPointerA();
};

class SecureBufferAutoPointerW {
	LPWSTR pPtr;
public:
	SecureBufferAutoPointerW(LPWSTR p) : pPtr(p) {}
	~SecureBufferAutoPointerW();
};

class SecureBufferAutoPointerByte {
	LPBYTE pPtr;
	DWORD nLen;
public:
	SecureBufferAutoPointerByte(LPBYTE p = NULL, DWORD n = 0) : pPtr(p), nLen(n) {}
	~SecureBufferAutoPointerByte();

	void Set(LPBYTE p, DWORD n) { Clear(); pPtr = p; nLen = n; }
	void Clear();
};

#endif
