#ifndef VARBUFFER_H
#define VARBUFFER_H

////////////////////////////////////////////////////
// Variable Buffer Implimentation
////////////////////////////////////////////////////
// This class does not consider word alignment,
// It is due to caller.

class VarBufferImpl {
	LPBYTE pBuf;
	DWORD nInitBytes;
	DWORD nDeltaBytes;
	DWORD nMax;

	DWORD nCurrentUse;

public:
	VarBufferImpl() : pBuf(NULL), nInitBytes(0), nDeltaBytes(0), nCurrentUse(0), nMax(0) {}
	~VarBufferImpl();

	BOOL Init(DWORD nInitBytes, DWORD nDeltaBytes);

	BOOL Add(LPBYTE pData, DWORD nBytes, LPDWORD pOffset);
	BOOL Extend(DWORD nPos, DWORD nExtendBytes);
	BOOL Shorten(DWORD nPos, DWORD nShortenBytes);

	LPBYTE GetBuffer() { return pBuf; }
	LPBYTE Get(DWORD nOffset) { return pBuf + nOffset; }

	BOOL Clear(BOOL bReAlloc);

	DWORD CurrentUse() { return nCurrentUse; }
};

////////////////////////////////////////////////////
// String buffer(for SBCS)
////////////////////////////////////////////////////

class StringBufferA : public VarBufferImpl {
public:
	// BOOL Init(DWORD nInitBytes, DWORD nDeltaBytes); //inherit from VarBufferImpl
	BOOL Add(const char *pData, DWORD nBytes, LPDWORD pOffset) { return VarBufferImpl::Add((LPBYTE)pData, nBytes, pOffset); }
	const char *GetBuffer() { return (const char*)VarBufferImpl::GetBuffer(); }
	const char *Get(DWORD nOffset) { return (const char*)VarBufferImpl::Get(nOffset); }
};

////////////////////////////////////////////////////
// String buffer(for MBCS)
////////////////////////////////////////////////////

class StringBufferT : public VarBufferImpl {
public:
	// BOOL Init(DWORD nInitBytes, DWORD nDeltaBytes); //inherit from VarBufferImpl
	BOOL Add(LPCTSTR pData, DWORD nLetters, LPDWORD pOffset) { return VarBufferImpl::Add((LPBYTE)pData, nLetters * sizeof(TCHAR), pOffset); }
	LPCTSTR GetBuffer() { return (LPCTSTR)VarBufferImpl::GetBuffer(); }
	LPCTSTR Get(DWORD nOffset) { return (LPCTSTR)VarBufferImpl::Get(nOffset); }
};

////////////////////////////////////////////////////
// Vector
////////////////////////////////////////////////////

template <class T>
class TVector : VarBufferImpl {
public:
	TVector() : VarBufferImpl() {}
	~TVector() {}
	BOOL Init(DWORD nNumInit, DWORD nNumDelta) { return VarBufferImpl::Init(nNumInit * sizeof(T), nNumDelta * sizeof(T)); }
	BOOL Add(T* pUnit) { return VarBufferImpl::Add((LPBYTE)pUnit, sizeof(T), NULL); }
	T* GetBuf() { return (T*)VarBufferImpl::GetBuffer(); }
	T* GetUnit(DWORD n) { return (T*)VarBufferImpl::Get(n * sizeof(T)); }
	DWORD NumItems() { return CurrentUse() / sizeof(T); }

	BOOL Extend(DWORD nIndex, DWORD nExtendItems) { return VarBufferImpl::Extend(nIndex * sizeof(T), nExtendItems * sizeof(T)); }
	BOOL Shorten(DWORD nIndex, DWORD nShortenItems) { return VarBufferImpl::Shorten(nIndex * sizeof(T), nShortenItems * sizeof(T)); }

	BOOL Clear(BOOL bReAlloc) { return VarBufferImpl::Clear(bReAlloc); }

};


#endif
