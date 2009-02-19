#include <windows.h>
#include <tchar.h>
#include "UniConv.h"
#include "TString.h"

////////////////////////////////////////////////////
// TString implimentation
////////////////////////////////////////////////////

BOOL TString::Alloc(DWORD nSize)
{
	if (pString) delete [] pString;
	pString = new TCHAR[nSize];
	if (pString == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}
	return TRUE;
}

BOOL TString::Set(LPCTSTR p)
{
	if (!Alloc(_tcslen(p) + 1)) return FALSE;
	_tcscpy(pString, p);
	return TRUE;
}

BOOL TString::StrCat(LPCTSTR pCat)
{
	if (pString == NULL) return Set(pCat);

	LPTSTR p = pString;
	pString = new TCHAR[_tcslen(p) + _tcslen(pCat) + 1];
	if (pString == NULL) {
		pString = p;
		return FALSE;
	}
	wsprintf(pString, TEXT("%s%s"), p, pCat);
	delete p;
	return TRUE;
}

BOOL TString::Join(LPCTSTR p1, LPCTSTR p2)
{
	DWORD n = _tcslen(p1) + _tcslen(p2) + 1;
	if (!Alloc(n)) return FALSE;
	wsprintf(pString, TEXT("%s%s"), p1, p2);
	return TRUE;
}

BOOL TString::Join(LPCTSTR p1, LPCTSTR p2, LPCTSTR p3)
{
	DWORD n = _tcslen(p1) + _tcslen(p2) + _tcslen(p3) + 1;
	if (!Alloc(n)) return FALSE;
	wsprintf(pString, TEXT("%s%s%s"), p1, p2, p3);
	return TRUE;
}

BOOL TString::Join(LPCTSTR p1, LPCTSTR p2, LPCTSTR p3, LPCTSTR p4)
{
	DWORD n = _tcslen(p1) + _tcslen(p2) + _tcslen(p3) + _tcslen(p4) + 1;
	if (!Alloc(n)) return FALSE;
	wsprintf(pString, TEXT("%s%s%s%s"), p1, p2, p3, p4);
	return TRUE;
}

BOOL TString::GetDirectoryPath(LPCTSTR pFullPath)
{
	//最後の'\'の位置の取得
	LPCTSTR p = pFullPath;
	LPCTSTR q = NULL;
#ifdef _WIN32_WCE
	while (*p) {
		if (*p == TEXT('\\')) q = p;
		p++;
	}
#else
	while (*p) {
		if (*p == TEXT('\\')) q = p;
		if (IsDBCSLeadByte(*p)) {
			p++;
		}
		p++;
	}
#endif

	if (q == NULL) {
		// '\'が存在しなかった = ディレクトリ部は存在しなかった
		if (!Alloc(1)) return FALSE;
		pString[0] = TEXT('\0');
	} else {
		DWORD nDirStringLen = q - pFullPath + 1;
		if (!Alloc(nDirStringLen + 1)) return FALSE;
		_tcsncpy(pString, pFullPath, nDirStringLen);
		pString[nDirStringLen] = TEXT('\0');
	}
	return TRUE;
}

void TString::ChopExtension()
{
	if (!pString) return;

	DWORD l = _tcslen(pString);
	if (l > 4) {
		pString[l - 4] = TEXT('\0');
	}
}

void TString::ChopFileNumber()
{
	if (pString == NULL || *pString == TEXT('\0')) return;

	DWORD n = _tcslen(pString);
	LPTSTR p = pString + n - 1;
	if (*p != TEXT(')')) return;

	p--;
	while(p >= pString) {
		if (*p == TEXT('(')) {
			*p = TEXT('\0');
			return;
		}
		if (*p < TEXT('0') || *p > TEXT('9')) break;
		p--;
	}
	return;
}

static LPTSTR GetTail(LPTSTR pBuf)
{
#ifdef _WIN32_WCE
	return pBuf + _tcslen(pBuf) - 1;
#else
	LPTSTR p = pBuf;
	LPTSTR pTail = pBuf;
	while(*p) {
		if (IsDBCSLeadByte(*p)) {
			pTail = p++;
			if (*p) p++;
		} else {
			pTail = p++;
		}
	}
	return pTail;
#endif
}

void TString::ChopFileSeparator()
{
	LPTSTR p;
	while(TRUE) {
		p = GetTail(pString);
		if (*p == TEXT('\\')) {
			*p = TEXT('\0');
		} else {
			break;
		}
	}

}

BOOL TString::GetPathTail(LPCTSTR pFullPath)
{
	//最後の'\'の位置の取得
	LPCTSTR p = pFullPath;
	LPCTSTR q = NULL;
#ifdef _WIN32_WCE
	while (*p) {
		if (*p == TEXT('\\')) q = p;
		p++;
	}
#else
	while (*p) {
		if (*p == TEXT('\\')) q = p;
		if (IsDBCSLeadByte(*p)) {
			p++;
		}
		p++;
	}
#endif

	if (q == NULL) {
		// '\'が存在しなかった = ディレクトリ部は存在しなかった
		return Set(pFullPath);
	} else {
		return Set(q + 1);
	}
}

////////////////////////////////////////////////////
// WString implimentation
////////////////////////////////////////////////////

BOOL WString::Alloc(DWORD nLetters)
{
	if (pString) delete [] pString;
	pString = new WCHAR[nLetters];
	if (pString == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}
	return TRUE;
}

BOOL WString::Set(TString *pSrc)
{
	if (pSrc->Get() == NULL) return FALSE;
#ifdef _WIN32_WCE
	// TString has WCHAR strings. So copy it.
	if (!Alloc(wcslen(pSrc->Get()) + 1)) return FALSE;
	wcscpy(pString, pSrc->Get());
#else
	// TString has char strings. Convert WBCS to Unicode.
	DWORD len = strlen(pSrc->Get());
	if (!Alloc(len + 1)) return FALSE;
		// Strictly say, this allocation is overallocation. 
		// But for performance, allocation does without counting.
	MultiByteToWideChar(CP_ACP, 0, pSrc->Get(), -1, pString, len + 1);
#endif

	return TRUE;
}

/////////////////////////////////////////////
// SharedString implimentation
/////////////////////////////////////////////

SharedString::SharedString() : pBuf(NULL)
{
}

SharedString::SharedString(const SharedString& s) : pBuf(NULL)
{
	Ref(s);
}

SharedString::~SharedString()
{
	ReleaseBuf();
}

BOOL SharedString::Init(LPCTSTR p)
{
	return Set(p);
}

BOOL SharedString::Init(const SharedString& s)
{
	Ref(s);
	return TRUE;
}

void SharedString::ReleaseBuf()
{
	if (pBuf) {
		if (pBuf->nRefCount == 1) {
			delete [] pBuf->pStr;
			delete pBuf;
		} else {
			pBuf->nRefCount--;
		}
		pBuf = NULL;
	}
}

BOOL SharedString::Set(LPCTSTR p)
{
	ReleaseBuf();

	pBuf = new SharedBuf();
	if (pBuf == NULL) return FALSE;

	pBuf->pStr = StringDup(p);
	if (pBuf->pStr == NULL) return FALSE;
	pBuf->nRefCount = 1;
	return TRUE;
}

BOOL SharedString::Set(const SharedString &s)
{
	Ref(s);
	return TRUE;
}

void SharedString::Ref(const SharedString& s)
{
	ReleaseBuf();
	if (s.pBuf) {
		pBuf = s.pBuf;
		pBuf->nRefCount++;
	}
}

LPCTSTR SharedString::Get() const
{
	if (pBuf == NULL) return NULL;
	return pBuf->pStr;
}