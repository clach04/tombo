#include <windows.h>
#include "VarBuffer.h"

/////////////////////////////////////////////////////////////////////////////
// ctor, dtor & Initializer
/////////////////////////////////////////////////////////////////////////////

VarBufferImpl::~VarBufferImpl()
{
	if (pBuf) {
		LocalFree(pBuf);
	}
}

BOOL VarBufferImpl::Init(DWORD ni, DWORD delta)
{
	pBuf = (LPBYTE)LocalAlloc(LMEM_FIXED, ni);
	if (pBuf == NULL) return FALSE;

	nMax = nInitBytes = ni;
	nDeltaBytes = delta;

	nCurrentUse = 0;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Append area
/////////////////////////////////////////////////////////////////////////////

BOOL VarBufferImpl::Add(LPBYTE pData, DWORD nBytes, LPDWORD pOffset)
{
	if (nCurrentUse + nBytes >= nMax) {
		// ReAllocate extra memory
		DWORD nNumAdd = (((nBytes - (nMax - nCurrentUse)) / nDeltaBytes)  + 1) * nDeltaBytes;
		pBuf = (LPBYTE)LocalReAlloc(pBuf, nMax + nNumAdd, LMEM_MOVEABLE);
		if (pBuf == NULL) return FALSE;

		nMax += nNumAdd;
	}

	memcpy(pBuf + nCurrentUse, pData, nBytes);
	if (pOffset) {
		*pOffset = nCurrentUse;
	}
	nCurrentUse += nBytes;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Extend area
/////////////////////////////////////////////////////////////////////////////

BOOL VarBufferImpl::Extend(DWORD nPos, DWORD nExtendBytes)
{
	if (nCurrentUse + nExtendBytes > nMax) {
		// realloc 
		DWORD nDelta = (nExtendBytes / nDeltaBytes + 1) * nDeltaBytes;
		LPBYTE pNewBuf;
		pNewBuf = (LPBYTE)LocalReAlloc(pBuf, nMax + nDelta, LMEM_MOVEABLE);
		if (pNewBuf == NULL) return FALSE;
		pBuf = pNewBuf;
		nMax += nDelta;
	}
	memmove(pBuf + nPos + nExtendBytes, pBuf + nPos, nCurrentUse - nPos);
	nCurrentUse += nExtendBytes;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Shorten area
/////////////////////////////////////////////////////////////////////////////

BOOL VarBufferImpl::Shorten(DWORD nPos, DWORD nShortenBytes)
{
	memmove(pBuf + nPos, pBuf + nPos + nShortenBytes, nCurrentUse - (nPos + nShortenBytes));
	nCurrentUse -= nShortenBytes;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Clear buffer
/////////////////////////////////////////////////////////////////////////////
// if bReAlloc is TRUE, memory area are re-allocated and size is initialized.
// if FALSE, keep current area.

BOOL VarBufferImpl::Clear(BOOL bReAlloc)
{
	if (bReAlloc) {
		LocalFree(pBuf);
		return Init(nInitBytes, nDeltaBytes);
	} else {
		nCurrentUse = 0;
		return TRUE;
	}
}