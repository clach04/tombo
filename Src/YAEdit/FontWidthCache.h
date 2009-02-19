#ifndef FONTWIDTHCACHE_H
#define FONTWIDTHCACHE_H

#define FONT_TABLE_SIZE (256*256)

class FontWidthCache {
	BYTE bWidthTable[FONT_TABLE_SIZE];
	HWND hWndRef;
	HDC hRefDC;

protected:

public:
	FontWidthCache();
	~FontWidthCache();

	BOOL Init(HWND hWnd, HFONT hFont);

	void Clear();

#if defined(PLATFORM_WIN32)
	BYTE GetOnebyteCharWidth(const char c);
	BYTE GetTwobyteCharWidth(const char *p);
#else
	BYTE GetWideCharWidth(const WCHAR c);
#endif
};

#endif

