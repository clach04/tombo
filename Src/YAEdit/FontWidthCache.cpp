#include <windows.h>
#include "FontWidthCache.h"

////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////

FontWidthCache::FontWidthCache() : hRefDC(NULL), hWndRef(NULL)
{
}

FontWidthCache::~FontWidthCache()
{
	if (hRefDC) {
		ReleaseDC(hWndRef, hRefDC);
	}
	Clear();
}


BOOL FontWidthCache::Init(HWND h, HFONT hFont)
{
	hWndRef = h;
	hRefDC = GetDC(hWndRef);
	SelectObject(hRefDC, hFont);
	Clear();
	return TRUE;
}

void FontWidthCache::Clear()
{
	memset(bWidthTable, 0, FONT_TABLE_SIZE);
}

#if defined(PLATFORM_WIN32)

BYTE FontWidthCache::GetOnebyteCharWidth(const char c)
{
	DWORD nIdx = c;

	SIZE size;
	GetTextExtentPoint32(hRefDC, &c, 1, &size);
	return (BYTE)size.cx;
}

BYTE FontWidthCache::GetTwobyteCharWidth(const char *p)
{
	if (*p == '\0') return 0;
	SIZE size;
	GetTextExtentPoint32(hRefDC, p, 2, &size);
	return (BYTE)size.cx;
}

#else

BYTE FontWidthCache::GetWideCharWidth(const WCHAR c)
{
	if (bWidthTable[c]) return bWidthTable[c];

	SIZE size;
	GetTextExtentPoint32(hRefDC, &c, 1, &size);
	bWidthTable[c] = (BYTE)size.cx;
	return (BYTE)size.cx;
}
#endif

