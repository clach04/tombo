#include <windows.h>
#include <tchar.h>
#include "Clipboard.h"
#include "UniConv.h"

namespace Tombo_Lib {

/////////////////////////////////////////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////////////////////////////////////////

Clipboard::Clipboard() : bOpen(FALSE) {}

Clipboard::~Clipboard() 
{
	Close();
}

/////////////////////////////////////////////////////////////////////////////
// Open/close clipboard
/////////////////////////////////////////////////////////////////////////////

BOOL Clipboard::Open(HWND hWnd)
{
	if (bOpen) return TRUE;

	bOpen = OpenClipboard(hWnd);
	return bOpen;
}

void Clipboard::Close()
{
	if (bOpen) {
		CloseClipboard();
	}
}

/////////////////////////////////////////////////////////////////////////////
// Get text data
/////////////////////////////////////////////////////////////////////////////

LPTSTR Clipboard::GetText()
{
	if (!bOpen) return NULL;

	LPTSTR pText = NULL;
#if defined(PLATFORM_WIN32)
	HANDLE hText = GetClipboardData(CF_TEXT);
	if (hText != NULL) {
		pText = StringDup((char*)LocalLock(hText));
		LocalUnlock(hText);
		if (pText == NULL) return NULL;
	}
#else
	HANDLE hText = GetClipboardData(CF_TEXT);
	if (hText != NULL) {
		pText = ConvSJIS2Unicode((char*)LocalLock(hText));
		LocalUnlock(hText);
		if (pText == NULL) return NULL;
	} else {
		hText = GetClipboardData(CF_UNICODETEXT);
		if (hText != NULL) {
			pText= StringDup((LPCTSTR)LocalLock(hText));
			LocalUnlock(hText);
			if (pText == NULL) return FALSE;
		}
	}
#endif
	return pText;
}

/////////////////////////////////////////////////////////////////////////////
// Set text data
/////////////////////////////////////////////////////////////////////////////

BOOL Clipboard::SetText(LPCTSTR pText)
{
	if (!bOpen) {
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}
	if (!EmptyClipboard()) return FALSE;

	DWORD nLen = (_tcslen(pText) + 1) * sizeof(TCHAR);
	HANDLE hText = LocalAlloc(LMEM_MOVEABLE , nLen);
	if (hText == NULL) return FALSE;

	LPTSTR p = (LPTSTR)LocalLock(hText);
	_tcscpy(p, pText);

	LocalUnlock(hText);

	UINT uType;
#if defined(PLATFORM_WIN32)
	uType = CF_TEXT;
#else
	uType = CF_UNICODETEXT;
#endif
	if (SetClipboardData(uType, hText) == NULL) return FALSE;
	return TRUE;
}


}; // namespace Tombo_Lib


