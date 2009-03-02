#include <windows.h>
#include <tchar.h>
#if defined(TOMBO)
#include "Tombo.h"
#include "Property.h"
#endif
#include "Uniconv.h"
#include "TString.h"
#include "AutoPtr.h"
#include "File.h"

#ifndef ESC
#define ESC 0x1B
#endif

static void shift(BYTE *ph, BYTE *pl);

#if defined(PLATFORM_BE500) && defined(TOMBO_LANG_ENGLISH)
static WCHAR cp_1253[] = {
	0x0000, // 0x00 #NULL
	0x0001, // 0x01 #START OF HEADING
	0x0002, // 0x02 #START OF TEXT
	0x0003, // 0x03 #END OF TEXT
	0x0004, // 0x04 #END OF TRANSMISSION
	0x0005, // 0x05 #ENQUIRY
	0x0006, // 0x06 #ACKNOWLEDGE
	0x0007, // 0x07 #BELL
	0x0008, // 0x08 #BACKSPACE
	0x0009, // 0x09 #HORIZONTAL TABULATION
	0x000A, // 0x0A #LINE FEED
	0x000B, // 0x0B #VERTICAL TABULATION
	0x000C, // 0x0C #FORM FEED
	0x000D, // 0x0D #CARRIAGE RETURN
	0x000E, // 0x0E #SHIFT OUT
	0x000F, // 0x0F #SHIFT IN
	0x0010, // 0x10 #DATA LINK ESCAPE
	0x0011, // 0x11 #DEVICE CONTROL ONE
	0x0012, // 0x12 #DEVICE CONTROL TWO
	0x0013, // 0x13 #DEVICE CONTROL THREE
	0x0014, // 0x14 #DEVICE CONTROL FOUR
	0x0015, // 0x15 #NEGATIVE ACKNOWLEDGE
	0x0016, // 0x16 #SYNCHRONOUS IDLE
	0x0017, // 0x17 #END OF TRANSMISSION BLOCK
	0x0018, // 0x18 #CANCEL
	0x0019, // 0x19 #END OF MEDIUM
	0x001A, // 0x1A #SUBSTITUTE
	0x001B, // 0x1B #ESCAPE
	0x001C, // 0x1C #FILE SEPARATOR
	0x001D, // 0x1D #GROUP SEPARATOR
	0x001E, // 0x1E #RECORD SEPARATOR
	0x001F, // 0x1F #UNIT SEPARATOR
	0x0020, // 0x20 #SPACE
	0x0021, // 0x21 #EXCLAMATION MARK
	0x0022, // 0x22 #QUOTATION MARK
	0x0023, // 0x23 #NUMBER SIGN
	0x0024, // 0x24 #DOLLAR SIGN
	0x0025, // 0x25 #PERCENT SIGN
	0x0026, // 0x26 #AMPERSAND
	0x0027, // 0x27 #APOSTROPHE
	0x0028, // 0x28 #LEFT PARENTHESIS
	0x0029, // 0x29 #RIGHT PARENTHESIS
	0x002A, // 0x2A #ASTERISK
	0x002B, // 0x2B #PLUS SIGN
	0x002C, // 0x2C #COMMA
	0x002D, // 0x2D #HYPHEN-MINUS
	0x002E, // 0x2E #FULL STOP
	0x002F, // 0x2F #SOLIDUS
	0x0030, // 0x30 #DIGIT ZERO
	0x0031, // 0x31 #DIGIT ONE
	0x0032, // 0x32 #DIGIT TWO
	0x0033, // 0x33 #DIGIT THREE
	0x0034, // 0x34 #DIGIT FOUR
	0x0035, // 0x35 #DIGIT FIVE
	0x0036, // 0x36 #DIGIT SIX
	0x0037, // 0x37 #DIGIT SEVEN
	0x0038, // 0x38 #DIGIT EIGHT
	0x0039, // 0x39 #DIGIT NINE
	0x003A, // 0x3A #COLON
	0x003B, // 0x3B #SEMICOLON
	0x003C, // 0x3C #LESS-THAN SIGN
	0x003D, // 0x3D #EQUALS SIGN
	0x003E, // 0x3E #GREATER-THAN SIGN
	0x003F, // 0x3F #QUESTION MARK
	0x0040, // 0x40 #COMMERCIAL AT
	0x0041, // 0x41 #LATIN CAPITAL LETTER A
	0x0042, // 0x42 #LATIN CAPITAL LETTER B
	0x0043, // 0x43 #LATIN CAPITAL LETTER C
	0x0044, // 0x44 #LATIN CAPITAL LETTER D
	0x0045, // 0x45 #LATIN CAPITAL LETTER E
	0x0046, // 0x46 #LATIN CAPITAL LETTER F
	0x0047, // 0x47 #LATIN CAPITAL LETTER G
	0x0048, // 0x48 #LATIN CAPITAL LETTER H
	0x0049, // 0x49 #LATIN CAPITAL LETTER I
	0x004A, // 0x4A #LATIN CAPITAL LETTER J
	0x004B, // 0x4B #LATIN CAPITAL LETTER K
	0x004C, // 0x4C #LATIN CAPITAL LETTER L
	0x004D, // 0x4D #LATIN CAPITAL LETTER M
	0x004E, // 0x4E #LATIN CAPITAL LETTER N
	0x004F, // 0x4F #LATIN CAPITAL LETTER O
	0x0050, // 0x50 #LATIN CAPITAL LETTER P
	0x0051, // 0x51 #LATIN CAPITAL LETTER Q
	0x0052, // 0x52 #LATIN CAPITAL LETTER R
	0x0053, // 0x53 #LATIN CAPITAL LETTER S
	0x0054, // 0x54 #LATIN CAPITAL LETTER T
	0x0055, // 0x55 #LATIN CAPITAL LETTER U
	0x0056, // 0x56 #LATIN CAPITAL LETTER V
	0x0057, // 0x57 #LATIN CAPITAL LETTER W
	0x0058, // 0x58 #LATIN CAPITAL LETTER X
	0x0059, // 0x59 #LATIN CAPITAL LETTER Y
	0x005A, // 0x5A #LATIN CAPITAL LETTER Z
	0x005B, // 0x5B #LEFT SQUARE BRACKET
	0x005C, // 0x5C #REVERSE SOLIDUS
	0x005D, // 0x5D #RIGHT SQUARE BRACKET
	0x005E, // 0x5E #CIRCUMFLEX ACCENT
	0x005F, // 0x5F #LOW LINE
	0x0060, // 0x60 #GRAVE ACCENT
	0x0061, // 0x61 #LATIN SMALL LETTER A
	0x0062, // 0x62 #LATIN SMALL LETTER B
	0x0063, // 0x63 #LATIN SMALL LETTER C
	0x0064, // 0x64 #LATIN SMALL LETTER D
	0x0065, // 0x65 #LATIN SMALL LETTER E
	0x0066, // 0x66 #LATIN SMALL LETTER F
	0x0067, // 0x67 #LATIN SMALL LETTER G
	0x0068, // 0x68 #LATIN SMALL LETTER H
	0x0069, // 0x69 #LATIN SMALL LETTER I
	0x006A, // 0x6A #LATIN SMALL LETTER J
	0x006B, // 0x6B #LATIN SMALL LETTER K
	0x006C, // 0x6C #LATIN SMALL LETTER L
	0x006D, // 0x6D #LATIN SMALL LETTER M
	0x006E, // 0x6E #LATIN SMALL LETTER N
	0x006F, // 0x6F #LATIN SMALL LETTER O
	0x0070, // 0x70 #LATIN SMALL LETTER P
	0x0071, // 0x71 #LATIN SMALL LETTER Q
	0x0072, // 0x72 #LATIN SMALL LETTER R
	0x0073, // 0x73 #LATIN SMALL LETTER S
	0x0074, // 0x74 #LATIN SMALL LETTER T
	0x0075, // 0x75 #LATIN SMALL LETTER U
	0x0076, // 0x76 #LATIN SMALL LETTER V
	0x0077, // 0x77 #LATIN SMALL LETTER W
	0x0078, // 0x78 #LATIN SMALL LETTER X
	0x0079, // 0x79 #LATIN SMALL LETTER Y
	0x007A, // 0x7A #LATIN SMALL LETTER Z
	0x007B, // 0x7B #LEFT CURLY BRACKET
	0x007C, // 0x7C #VERTICAL LINE
	0x007D, // 0x7D #RIGHT CURLY BRACKET
	0x007E, // 0x7E #TILDE
	0x007F, // 0x7F #DELETE
	0x20AC, // 0x80 #EURO SIGN
	0x0020, // 0x81 #UNDEFINED
	0x201A, // 0x82 #SINGLE LOW-9 QUOTATION MARK
	0x0192, // 0x83 #LATIN SMALL LETTER F WITH HOOK
	0x201E, // 0x84 #DOUBLE LOW-9 QUOTATION MARK
	0x2026, // 0x85 #HORIZONTAL ELLIPSIS
	0x2020, // 0x86 #DAGGER
	0x2021, // 0x87 #DOUBLE DAGGER
	0x0020, // 0x88 #UNDEFINED
	0x2030, // 0x89 #PER MILLE SIGN
	0x0020, // 0x8A #UNDEFINED
	0x2039, // 0x8B #SINGLE LEFT-POINTING ANGLE QUOTATION MARK
	0x0020, // 0x8C #UNDEFINED
	0x0020, // 0x8D #UNDEFINED
	0x0020, // 0x8E #UNDEFINED
	0x0020, // 0x8F #UNDEFINED
	0x0020, // 0x90 #UNDEFINED
	0x2018, // 0x91 #LEFT SINGLE QUOTATION MARK
	0x2019, // 0x92 #RIGHT SINGLE QUOTATION MARK
	0x201C, // 0x93 #LEFT DOUBLE QUOTATION MARK
	0x201D, // 0x94 #RIGHT DOUBLE QUOTATION MARK
	0x2022, // 0x95 #BULLET
	0x2013, // 0x96 #EN DASH
	0x2014, // 0x97 #EM DASH
	0x0020, // 0x98 #UNDEFINED
	0x2122, // 0x99 #TRADE MARK SIGN
	0x0020, // 0x9A #UNDEFINED
	0x203A, // 0x9B #SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
	0x0020, // 0x9C #UNDEFINED
	0x0020, // 0x9D #UNDEFINED
	0x0020, // 0x9E #UNDEFINED
	0x0020, // 0x9F #UNDEFINED
	0x00A0, // 0xA0 #NO-BREAK SPACE
	0x0385, // 0xA1 #GREEK DIALYTIKA TONOS
	0x0386, // 0xA2 #GREEK CAPITAL LETTER ALPHA WITH TONOS
	0x00A3, // 0xA3 #POUND SIGN
	0x00A4, // 0xA4 #CURRENCY SIGN
	0x00A5, // 0xA5 #YEN SIGN
	0x00A6, // 0xA6 #BROKEN BAR
	0x00A7, // 0xA7 #SECTION SIGN
	0x00A8, // 0xA8 #DIAERESIS
	0x00A9, // 0xA9 #COPYRIGHT SIGN
	0x0020, // 0xAA #UNDEFINED
	0x00AB, // 0xAB #LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
	0x00AC, // 0xAC #NOT SIGN
	0x00AD, // 0xAD #SOFT HYPHEN
	0x00AE, // 0xAE #REGISTERED SIGN
	0x2015, // 0xAF #HORIZONTAL BAR
	0x00B0, // 0xB0 #DEGREE SIGN
	0x00B1, // 0xB1 #PLUS-MINUS SIGN
	0x00B2, // 0xB2 #SUPERSCRIPT TWO
	0x00B3, // 0xB3 #SUPERSCRIPT THREE
	0x0384, // 0xB4 #GREEK TONOS
	0x00B5, // 0xB5 #MICRO SIGN
	0x00B6, // 0xB6 #PILCROW SIGN
	0x00B7, // 0xB7 #MIDDLE DOT
	0x0388, // 0xB8 #GREEK CAPITAL LETTER EPSILON WITH TONOS
	0x0389, // 0xB9 #GREEK CAPITAL LETTER ETA WITH TONOS
	0x038A, // 0xBA #GREEK CAPITAL LETTER IOTA WITH TONOS
	0x00BB, // 0xBB #RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
	0x038C, // 0xBC #GREEK CAPITAL LETTER OMICRON WITH TONOS
	0x00BD, // 0xBD #VULGAR FRACTION ONE HALF
	0x038E, // 0xBE #GREEK CAPITAL LETTER UPSILON WITH TONOS
	0x038F, // 0xBF #GREEK CAPITAL LETTER OMEGA WITH TONOS
	0x0390, // 0xC0 #GREEK SMALL LETTER IOTA WITH DIALYTIKA AND TONOS
	0x0391, // 0xC1 #GREEK CAPITAL LETTER ALPHA
	0x0392, // 0xC2 #GREEK CAPITAL LETTER BETA
	0x0393, // 0xC3 #GREEK CAPITAL LETTER GAMMA
	0x0394, // 0xC4 #GREEK CAPITAL LETTER DELTA
	0x0395, // 0xC5 #GREEK CAPITAL LETTER EPSILON
	0x0396, // 0xC6 #GREEK CAPITAL LETTER ZETA
	0x0397, // 0xC7 #GREEK CAPITAL LETTER ETA
	0x0398, // 0xC8 #GREEK CAPITAL LETTER THETA
	0x0399, // 0xC9 #GREEK CAPITAL LETTER IOTA
	0x039A, // 0xCA #GREEK CAPITAL LETTER KAPPA
	0x039B, // 0xCB #GREEK CAPITAL LETTER LAMDA
	0x039C, // 0xCC #GREEK CAPITAL LETTER MU
	0x039D, // 0xCD #GREEK CAPITAL LETTER NU
	0x039E, // 0xCE #GREEK CAPITAL LETTER XI
	0x039F, // 0xCF #GREEK CAPITAL LETTER OMICRON
	0x03A0, // 0xD0 #GREEK CAPITAL LETTER PI
	0x03A1, // 0xD1 #GREEK CAPITAL LETTER RHO
	0x0020, // 0xD2 #UNDEFINED
	0x03A3, // 0xD3 #GREEK CAPITAL LETTER SIGMA
	0x03A4, // 0xD4 #GREEK CAPITAL LETTER TAU
	0x03A5, // 0xD5 #GREEK CAPITAL LETTER UPSILON
	0x03A6, // 0xD6 #GREEK CAPITAL LETTER PHI
	0x03A7, // 0xD7 #GREEK CAPITAL LETTER CHI
	0x03A8, // 0xD8 #GREEK CAPITAL LETTER PSI
	0x03A9, // 0xD9 #GREEK CAPITAL LETTER OMEGA
	0x03AA, // 0xDA #GREEK CAPITAL LETTER IOTA WITH DIALYTIKA
	0x03AB, // 0xDB #GREEK CAPITAL LETTER UPSILON WITH DIALYTIKA
	0x03AC, // 0xDC #GREEK SMALL LETTER ALPHA WITH TONOS
	0x03AD, // 0xDD #GREEK SMALL LETTER EPSILON WITH TONOS
	0x03AE, // 0xDE #GREEK SMALL LETTER ETA WITH TONOS
	0x03AF, // 0xDF #GREEK SMALL LETTER IOTA WITH TONOS
	0x03B0, // 0xE0 #GREEK SMALL LETTER UPSILON WITH DIALYTIKA AND TONOS
	0x03B1, // 0xE1 #GREEK SMALL LETTER ALPHA
	0x03B2, // 0xE2 #GREEK SMALL LETTER BETA
	0x03B3, // 0xE3 #GREEK SMALL LETTER GAMMA
	0x03B4, // 0xE4 #GREEK SMALL LETTER DELTA
	0x03B5, // 0xE5 #GREEK SMALL LETTER EPSILON
	0x03B6, // 0xE6 #GREEK SMALL LETTER ZETA
	0x03B7, // 0xE7 #GREEK SMALL LETTER ETA
	0x03B8, // 0xE8 #GREEK SMALL LETTER THETA
	0x03B9, // 0xE9 #GREEK SMALL LETTER IOTA
	0x03BA, // 0xEA #GREEK SMALL LETTER KAPPA
	0x03BB, // 0xEB #GREEK SMALL LETTER LAMDA
	0x03BC, // 0xEC #GREEK SMALL LETTER MU
	0x03BD, // 0xED #GREEK SMALL LETTER NU
	0x03BE, // 0xEE #GREEK SMALL LETTER XI
	0x03BF, // 0xEF #GREEK SMALL LETTER OMICRON
	0x03C0, // 0xF0 #GREEK SMALL LETTER PI
	0x03C1, // 0xF1 #GREEK SMALL LETTER RHO
	0x03C2, // 0xF2 #GREEK SMALL LETTER FINAL SIGMA
	0x03C3, // 0xF3 #GREEK SMALL LETTER SIGMA
	0x03C4, // 0xF4 #GREEK SMALL LETTER TAU
	0x03C5, // 0xF5 #GREEK SMALL LETTER UPSILON
	0x03C6, // 0xF6 #GREEK SMALL LETTER PHI
	0x03C7, // 0xF7 #GREEK SMALL LETTER CHI
	0x03C8, // 0xF8 #GREEK SMALL LETTER PSI
	0x03C9, // 0xF9 #GREEK SMALL LETTER OMEGA
	0x03CA, // 0xFA #GREEK SMALL LETTER IOTA WITH DIALYTIKA
	0x03CB, // 0xFB #GREEK SMALL LETTER UPSILON WITH DIALYTIKA
	0x03CC, // 0xFC #GREEK SMALL LETTER OMICRON WITH TONOS
	0x03CD, // 0xFD #GREEK SMALL LETTER UPSILON WITH TONOS
	0x03CE, // 0xFE #GREEK SMALL LETTER OMEGA WITH TONOS
	0x0020, // 0xFF #UNDEFINED
};

// Unicode range 0x2013 - 0x2022
static unsigned char CP_1253_20XX[16] = {
	0x96, 0x97, 0xAF, 0, 0, 0x91, 0x92, 0x82, 0, 0x93, 0x94, 0x84, 0, 0x86, 0x87, 0x95
};

unsigned char UnicodeToCP1253(WORD tc)
{
	if ((tc >= 0 && tc <= 0x7F) ||
		(tc >= 0xA0 && tc <= 0xBD) &&
		(tc != 0xA1 || tc != 0xA2 || tc != 0xAA || tc != 0xAF ||
		tc != 0xB4 || tc != 0xB8 || tc != 0xB9 || tc != 0xBA || tc != 0xBC)) {
		return (unsigned char)(tc & 0xFF);
	}
	if (tc >= 0x388 && tc <= 0x3CE && (tc != 0x38B && tc != 0x38D && tc != 0x3A2)) {
		return (unsigned char)(tc - 0x388 + 0xB8);
	}
	
	if (tc >= 0x2013 && tc <= 0x2022) {
		unsigned char c = CP_1253_20XX[tc - 0x2013];
		if (c != 0) return c;
	}

	unsigned char c;
	switch (tc) {
	case 0x0192: c = 0x83; break;
	case 0x0384: c = 0xB4; break;
	case 0x0385: c = 0xA1; break;
	case 0x0386: c = 0xA2; break;
	case 0x2026: c = 0x85; break;
	case 0x2030: c = 0x89; break;
	case 0x2039: c = 0x8B; break;
	case 0x203A: c = 0x9B; break;
	case 0x20AC: c = 0x80; break;
	case 0x2122: c = 0x99; break;
	default:
		c = (unsigned char)tc & 0xFF; // can't convert it.
	}
	return c;
}
void MultiByteToWideChar_CP1253(LPCSTR pMb, LPWSTR pUni, DWORD n)
{
	const unsigned char *p = (const unsigned char *)pMb;
	LPWSTR q = pUni;

	DWORD i = 0; 
	while(*p && (n == 0xFFFFFFFF || i < n)) {
		*q++ = cp_1253[*p];
		p++;
		i++;
	}
	*q++ = TEXT('\0');
}

void WideCharToMultiByte_CP1253(LPCWSTR pUni, LPSTR pMb, DWORD n)
{
	LPCWSTR p = pUni;
	unsigned char *q = (unsigned char*)pMb;

	DWORD i = 0;
	while(*p && (n = 0xFFFFFFFF || i < n)) {
		*q++ = UnicodeToCP1253(*p);
		p++;
		i++;
	}
	*q++ = '\0';
}
#endif //defined(PLATFORM_BE500) && defined(TOMBO_LANG_ENGLISH)

////////////////////////////////////////////////////////////
// SJIS -> Unicode変換
////////////////////////////////////////////////////////////

LPTSTR ConvSJIS2Unicode(const char *p)
{
	// 領域確保
	DWORD l = strlen(p) + 1;
	LPTSTR pUni = new TCHAR[l];
	if (pUni == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}

#ifdef _WIN32_WCE
#if defined(PLATFORM_BE500) && defined(TOMBO_LANG_ENGLISH)
	if (g_Property.GetCodePage() == 1253) { // Greek codepage
		MultiByteToWideChar_CP1253((LPCSTR)p, pUni, -1);
	} else {
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)p, -1, pUni, l);
	}
#else
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)p, -1, pUni, l);
#endif

#else	// _WIN32_WCE
	_tcscpy(pUni, p);
#endif
	return pUni;
}

#ifdef _WIN32_WCE
DWORD CountMBStrings(const char *pStr, DWORD nBytes)
{
#if defined(PLATFORM_BE500) && defined(TOMBO_LANG_ENGLISH)
	if (g_Property.GetCodePage() == 1253) {
		DWORD n = strlen(pStr);
		return (n < nBytes) ? n : nBytes;
	} else {
		return MultiByteToWideChar(CP_ACP, 0, pStr, nBytes, NULL, 0);
	}
#else
	return MultiByteToWideChar(CP_ACP, 0, pStr, nBytes, NULL, 0);
#endif
}
#endif

#ifdef _WIN32_WCE
DWORD CountWCBytes(LPCTSTR pStr, DWORD nChar)
{
#if defined(PLATFORM_BE500) && defined(TOMBO_LANG_ENGLISH)
	if (g_Property.GetCodePage() == 1253) {
		DWORD n = _tcslen(pStr);
		return (n < nChar) ? n : nChar;
	} else{
		return WideCharToMultiByte(CP_ACP, 0, pStr, nChar, NULL, 0, NULL, NULL);
	}
#else
	return WideCharToMultiByte(CP_ACP, 0, pStr, nChar, NULL, 0, NULL, NULL);
#endif
}
#endif

DWORD ConvUTF8PosToUCSPos(const char *pUTF, DWORD nBytes)
{
	DWORD n = 0;
	const char *p = pUTF;
	while(*p) {
		if ((DWORD)(p - pUTF) == nBytes) break;
		if ((*p & 0x80) == 0x00) {
			p++; n++;
		} else if ((*p & 0xE0) == 0xC0) {
			p += 2; n++;
		} else if ((*p & 0xF0) == 0xE0) {
			p += 3; n++;
		} else return -1;
	}
	return n;
}

DWORD ConvUCSPosToUTF8Pos(const char *pUTF, DWORD nUCSPos)
{
	DWORD n = 0;
	const char *p = pUTF;
	while(*p) {
		if (n >= nUCSPos) break;
		if ((*p & 0x80) == 0x00) {
			p++; n++;
		} else if ((*p & 0xE0) == 0xC0) {
			p += 2; n++;
		} else if ((*p & 0xF0) == 0xE0) {
			p += 3; n++;
		} else return -1;
	}
	return (DWORD)(p - pUTF);
}

///////////////////////////////////////////////////////////
// Unicode -> SJIS変換
////////////////////////////////////////////////////////////

char *ConvUnicode2SJIS(LPCTSTR p)
{
	DWORD l = (_tcslen(p) + 1)*sizeof(TCHAR);
	char *pS = new char[l];
	if (pS == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
#ifdef _WIN32_WCE
#if defined(PLATFORM_BE500) && defined(TOMBO_LANG_ENGLISH)
	if (g_Property.GetCodePage() == 1253) {
		WideCharToMultiByte_CP1253(p, pS, -1);
	} else {
		WideCharToMultiByte(CP_ACP, 0, p, -1, pS, l, NULL, NULL);
	}
#else
	WideCharToMultiByte(CP_ACP, 0, p, -1, pS, l, NULL, NULL);
#endif
#else // _WIN32_WCE
	strcpy(pS, p);
#endif
	return pS;
}

///////////////////////////////////////////////////////////
// Convert TCHAR to WCHAR
////////////////////////////////////////////////////////////

LPWSTR ConvTCharToWChar(LPCTSTR p)
{
	if (p == NULL) return NULL;
#if defined(PLATFORM_WIN32)
	LPWSTR pW;
	DWORD nLen = strlen(p);
	pW = new WCHAR[nLen +1];
	if (pW == NULL) return NULL;
	MultiByteToWideChar(CP_ACP, 0, p, -1, pW, nLen + 1);
	pW[nLen] = L'\0';
	return pW;
#else
	return StringDup(p);
#endif
}

LPTSTR ConvWCharToTChar(LPCWSTR p)
{
	if (p == NULL) return NULL;
#if defined(PLATFORM_WIN32)
	DWORD nLen = WideCharToMultiByte(CP_ACP, 0, p, -1, NULL, 0, NULL, NULL);
	LPTSTR pT = new TCHAR[nLen + 1];
	if (pT == NULL) return NULL;
	WideCharToMultiByte(CP_ACP, 0, p, -1, pT, nLen + 1, NULL, NULL);
	pT[nLen] = TEXT('\0');
	return pT;
#else
	return StringDup(p);
#endif
}

//////////////////////////////////
// TCHAR <-> UTF-8
//////////////////////////////////

char *ConvTCharToUTF8(LPCTSTR p)
{
	if (p == NULL) return NULL;
#if defined(PLATFORM_WIN32)
	LPWSTR pW = ConvTCharToWChar(p);
	SecureBufferAutoPointerW ap(pW);
	if (pW == NULL) return NULL;
	
	return ConvUCS2ToUTF8(pW);
#else
	return ConvUCS2ToUTF8(p);
#endif
}

LPTSTR ConvUTF8ToTChar(const char *p)
{
	LPWSTR pW = ConvUTF8ToUCS2(p);
	if (pW == NULL) return NULL;
#if defined(PLATFORM_WIN32)
	SecureBufferAutoPointerW ap(pW);
	return ConvWCharToTChar(pW);
#else
	return pW;
#endif
}

///////////////////////////////////////////////////
// JIS -> SJIS変換ルーチン
///////////////////////////////////////////////////
// 変換アルゴリズムは "C言語による最新アルゴリズム辞典" 奥村晴彦, 技術評論社 がベース。
#define CODEPAGE_ANSI    0
#define CODEPAGE_JIS     1
#define CODEPAGE_HANKANA 2

void ConvJIS2SJIS(const char *pIn, char *pOut)
{
	const BYTE *p = (const BYTE*)pIn;
	BYTE *q = (BYTE*)pOut;

	BYTE c, d, n;
	DWORD codepage = CODEPAGE_ANSI;

	while(*p) {
		// Is escape-sequence?

		if (*p == ESC) {
			if (*(p+1) == '$') {
				if (*(p+2) == 'B' || *(p+2) == '@') {
					p+= 3;
					codepage = CODEPAGE_JIS;
					continue;
				}
			}
			if (*(p+1) == '(') {
				n = *(p+2);
				if (n == 'H' || n == 'J' || n == 'B') {
					p+= 3;
					codepage = CODEPAGE_ANSI;
					continue;
				}
				if (n == 'I') {
					p+= 3;
					codepage = CODEPAGE_HANKANA;
					continue;
				}
			}
			if (*(p+1) == 'K') {
				codepage = CODEPAGE_JIS;
				p+= 2;
				continue;
			}
			if (*(p+1) == 'H') {
				codepage = CODEPAGE_ANSI;
				p += 2;
				continue;
			}
		}

		// for processing by codepage
		switch(codepage) {
		case CODEPAGE_ANSI:
			*q++ = *p++;
			break;
		case CODEPAGE_JIS:
			c = *p++;
			if (c >= 0x21 && c <= 0x7E) {
				if ((d = *p++) >= 0x21 && d <= 0x7E) {
					shift(&c, &d);
				}
				*q++ = c; if (d != '\0') *q++ = (char)d;
			} else if (c >= 0xA1 && c <= 0xFE) {
				if ((d = *p++) >= 0xA1 && d <= 0xFE) {
					d &= 0x7F; c &= 0x7F; shift(&c, &d);
				}
				*q++ = c; if (d != '\0') *q++ = (char)d;
			} else *q++ = c;
			break;
		case CODEPAGE_HANKANA:
			*q++ = (*p++) | 0x80;
			break;
		}
	}
	*q++ = '\0';
}

///////////////////////////
// 変換下請け

static void shift(BYTE *ph, BYTE *pl)
{
	if (*ph & 1) {
		if (*pl < 0x60) *pl += 0x1F;
		else *pl += 0x20;
	} else *pl += 0x7E;
	if (*ph < 0x5F) *ph = (*ph + 0xE1) >> 1;
	else *ph = (*ph + 0x161) >> 1;
}

///////////////////////////////////////////////////
// MimeDec下請け

// BASE64文字を0-63の値に変換

inline char dec64(char c)
{
	if ('A' <= c && c <= 'Z') {
		return c - 'A';
	}
	if ('a' <= c && c <= 'z') {
		return c - 'a' + 26;
	}
	if ('0' <= c && c <= '9') {
		return c - '0' + 52;
	}
	if (c == '+') return 62;
	if (c == '/') return 63;
	else return 64;
}

///////////////////////////////////////////////////
// MIME BASE64(ISO-2022-JP) デコード
///////////////////////////////////////////////////
// BASE64ヘッダ(ISO-2022-JPのみ)をデコードする

BOOL MimeDec(char *pDst, const char *pSrc)
{
	const char *p = pSrc;
	char *q = pDst;
	char b1, b2, b3;
	char c1, c2, c3, c4;

S1:
	while(*p) {
		if (*p == '=' && *(p+1) == '?') break;
		*q++ = *p++;
	}
	if (*p == '\0') {
		*q++ = '\0';
		return FALSE;
	}
	// ASSERT(*p == '=' && *(p+1) == '?')

	if (strncmp(p + 2, "iso-2022-jp?B?", 14) != 0 &&
		strncmp(p + 2, "ISO-2022-JP?B?", 14) != 0) {
		*q++ = *p++;
		*q++ = *p++;
		goto S1;
	}
	p += 16;

	while(*p) {
		if (*p == '?') break;
		c1 = dec64(*p++);
		c2 = dec64(*p++);
		c3 = dec64(*p++);
		c4 = dec64(*p++);

		b1 = (c1 << 2) | ((c2 & 0x30) >> 4);
		*q++ = b1;

		if (c3 != 64) {
			b2 = ((c2 & 0xF) << 4) | ((c3 & 0x3c) >> 2);
			*q++ = b2;
		}

		if (c3 != 64 && c4 != 64) {
			b3 = ((c3 & 0x3) << 6) | c4;
			*q++ = b3;
		}


	}
	if (*p == '\0') {
		*q++ = '\0';
		return FALSE;
	}
	if (*p == '?' && *(p+1) == '=') {
		p += 2;
		goto S1;
	}
	*q = '\0';
	return FALSE;
}


////////////////////////////////////////
// ConvSJIS2JIS下請け

// SJISの2バイトペアをJISの2バイトペアに変換

static void jis(BYTE *ph, BYTE *pl)
{
	if (*ph <= 0x9F) {
		if (*pl < 0x9F) *ph = (*ph << 1) - 0xE1;
		else			*ph = (*ph << 1) - 0xE0;
	} else {
		if (*pl < 0x9F) *ph = (*ph << 1) - 0x161;
		else			*ph = (*ph << 1) - 0x160;
	}
	if		(*pl < 0x7F) *pl -= 0x1F;
	else if (*pl < 0x9F) *pl -= 0x20;
	else				 *pl -= 0x7E;
}


//////////////////////////////////////////////
// top～*curの内容がコピーされたバッファを確保する。
// topの指す先は開放される。
// 新しいバッファはbufsizよりCONVSJIS2_JIS_EXTEND_SIZE拡張されている

#define CONVSJIS2JIS_INIT_SIZE 256
#define CONVSJIS2JIS_EXTEND_SIZE 64

static char *Resize(char *top, char **cur, int *bufsiz)
{
	char *p = new char[*bufsiz + CONVSJIS2JIS_EXTEND_SIZE];
	if (p == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	memcpy(p, top, *cur - top);
	*cur = (*cur - top) + p;
	*bufsiz += CONVSJIS2JIS_EXTEND_SIZE;
	delete [] top;
	return p;
}

//////////////////////////////////////////////////
// Shift JISから JISへの変換
//////////////////////////////////////////////////
// 変換アルゴリズムは "C言語による最新アルゴリズム辞典" 奥村晴彦, 技術評論社 がベース。
//
// 領域を確保しそこへのポインタが返される。

char *ConvSJIS2JIS(char *str)
{
	char *outbuf = new char[CONVSJIS2JIS_INIT_SIZE];
	int bufsiz = CONVSJIS2JIS_INIT_SIZE;

	if (outbuf == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}

	char *p = str;
	char *q = outbuf;

	BYTE c, d;
	BOOL bKanji = FALSE;
	while ((c = *p++) != '\0') {
		if (iskanji(c)) {
			if (bKanji == FALSE) {
				if (q - outbuf + 3 >= bufsiz) {
					outbuf = Resize(outbuf, &q, &bufsiz);
					if (q == NULL) return FALSE;
				}
				*q++ = ESC;
				*q++ = '$';
				*q++ = 'B';
				bKanji = TRUE;
			}
			d = *p++;
			if (iskanji2(d)) {
				jis(&c, &d);
				if (q - outbuf + 2 >= bufsiz) {
					outbuf = Resize(outbuf, &q, &bufsiz);
					if (q == NULL) return FALSE;
				}
				*q++ = c;
				*q++ = d;
			} else {
				if (q - outbuf >= bufsiz) {
					outbuf = Resize(outbuf, &q, &bufsiz);
					if (q == NULL) return FALSE;
				}
				*q++ = c;
				if (d != '\0') {
					if (q - outbuf >= bufsiz) {
						outbuf = Resize(outbuf, &q, &bufsiz);
						if (q == NULL) return FALSE;
					}
					*q++ = d;
				}
			}
		} else {
			if (bKanji == TRUE) {
				if (q - outbuf + 3 >= bufsiz) {
					outbuf = Resize(outbuf, &q, &bufsiz);
					if (q == NULL) return FALSE;
				}
				*q++ = ESC;
				*q++ = '(';
				*q++ = 'B';
				bKanji = FALSE;
			}
			if (q - outbuf >= bufsiz) {
				outbuf = Resize(outbuf, &q, &bufsiz);
				if (q == NULL) return FALSE;
			}
			*q++ = c;
		}
	}
	if (bKanji = TRUE) {
			if (bKanji == TRUE) {
				if (q - outbuf + 3 >= bufsiz) {
					outbuf = Resize(outbuf, &q, &bufsiz);
					if (q == NULL) return FALSE;
				}
				*q++ = ESC;
				*q++ = '(';
				*q++ = 'B';
				bKanji = FALSE;
			}
	}
	if (q - outbuf >= bufsiz) {
		outbuf = Resize(outbuf, &q, &bufsiz);
		if (q == NULL) return FALSE;
	}
	*q++ = '\0';
	return outbuf;
}

#ifdef COMMENT
////////////////////////////////////////////////////
// Base64Encoder実装
////////////////////////////////////////////////////

// 変更するとうまく動かなくなるかも。
// BASE64_LINE_WIDTHはENCODE_BUF_SIZEより 最低でも4/3 + 18以上大きいこと。
#define BASE64_LINE_WIDTH 80
#define ENCODE_BUF_SIZ 26

#define ENC_HEADER_LEN 16
char *pEncHeader = "=?iso-2022-jp?B?";

#define ENC_FOOTER_LEN 2
char *pEncFooter = "?=";


Base64Encoder::~Base64Encoder()
{
	Item *p = head;
	Item *q;
	while(p) {
		delete [] (p->pLine);
		q = p;
		p = p->pNext;
		delete q;
	}
}

// 指定バイト以内に収める
// 漢字が含まれていたらASCIIもまとめてエンコードする
// あふれる場合には改行する
// 改行の際に文字コードがJISだったらASCIIに戻すためESC $ (を含めてエンコード
// 漢字の1byteめと2byteめの間では改行しない。


static void EncBuf(char *pIn, char *pOut)
{
	DWORD v;
	static char enctable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	
	DWORD n = strlen(pIn);

	unsigned char *p = (unsigned char *)pIn;
	unsigned char *q = (unsigned char *)pOut;

	while(n >= 3) {
		v = *p >> 2;
		*q++ = enctable[v];
		v = ((*p & 3) << 4) + (*(p+1) >> 4);
		*q++ = enctable[v];
		v = ((*(p+1) & 0xF) << 2) + (*(p+2) >> 6);
		*q++ = enctable[v];
		v = *(p+2) & 0x3F;
		*q++ = enctable[v];

		p += 3;
		n -= 3;
	}

	if (n == 2) {
		v = *p >> 2;
		*q++ = enctable[v];
		v = ((*p & 3) << 4) + (*(p+1) >> 4);
		*q++ = enctable[v];
		v = ((*(p+1) & 0x0F) << 2);
		*q++ = enctable[v];
		*q++ = '=';
	} else if (n == 1) {
		v = *p >> 2;
		*q++ = enctable[v];
		v = (*p & 3) << 4;
		*q++ = enctable[v];
		*q++ = '=';
		*q++ = '=';
	}
	*q = '\0';
}

#define TOKEN_EOF   0
#define TOKEN_OTHER 1
#define TOKEN_KANJI 2
#define TOKEN_TO_JIS   3
#define TOKEN_TO_ASCII 4

BOOL Base64Encoder::Encode(char *str)
{
	BOOL bKanji = FALSE;
	char *pLine = AllocLine();
	if (pLine == NULL) return FALSE;
	
	char *p = str;

	while(TRUE) {
		p = EncodeLine(p, pLine, &bKanji);
		if (*p == '\0') break; 
		pLine = AllocLine();
		if (pLine == NULL) return FALSE;
	}

	return TRUE;
}

char *Base64Encoder::EncodeLine(char *pIn, char *pOut, BOOL *pKanji)
{
	char *p = pIn;

	char kbuf[ENCODE_BUF_SIZ];
	char *q = kbuf;

	BOOL bKanji = *pKanji;
	DWORD n, t;

	// 行頭の調整
	// KKKK..	=>	E$BKKKK		bKanji = T
	// aaaa..	=>  aaaa
	// E$BK..	=>  E$BK
	// E(Ba..	=>  a			bKanji = T
	if (bKanji) {
		BOOL bk = bKanji;
		n = GetToken(p, &bk, &t);
		if (t == TOKEN_KANJI) {
			*q++ = ESC;
			*q++ = '$';
			*q++ = 'B';
		} else {
			p += n;
			bKanji = FALSE;
		}
	}

	// kbufに文字をつめてゆく
	t = TOKEN_EOF;

	while(*p) {
		BOOL bk = bKanji;
		DWORD pt;
		n = GetToken(p, &bk, &pt);
		if (q - kbuf + 3 + n >= ENCODE_BUF_SIZ) break;

		bKanji = bk;
		t = pt;
		memcpy(q, p, n);
		q += n;
		p += n;
	}

	// 行末の調整
	// KKKKK	->  KKE(B	next: kanji
	// aaE$B	->	aa		next: kanji
	// aaE(B	->	aaE(B	next: ascii
	// aaaaa	->	aaaaa	next: ascii

	if (t == TOKEN_KANJI) {
		*q++ = ESC;
		*q++ = '(';
		*q++ = 'B';
	} else if (t == TOKEN_TO_JIS) {
		q -= 3;
	}
	*q++ = '\0';

	// コードの変換
	if (strlen(kbuf) > 0) {
		strcpy(pOut, pEncHeader);
		EncBuf(kbuf, pOut + ENC_HEADER_LEN);
		strcat(pOut, pEncFooter);
	} else {
		*pOut = '\0';
	}
	*pKanji = bKanji;
	return p;
}

DWORD Base64Encoder::GetToken(char *pCurrent, BOOL *pKanji, DWORD *pType)
{
	DWORD nBytes;

	if (*pCurrent == '\0') {
		*pType = TOKEN_EOF;
		nBytes = 1;
	} else if (*pCurrent == ESC && *(pCurrent+1) && *(pCurrent + 2) == 'B') {
		if (*(pCurrent + 1) == '$') {
			*pType = TOKEN_TO_JIS;
			*pKanji = TRUE;
		} else if (*(pCurrent + 1) == '(') {
			*pType = TOKEN_TO_ASCII;
			*pKanji = FALSE;
		}
		nBytes = 3;
		pCurrent += 3;
	} else if (*pKanji) {
		*pType = TOKEN_KANJI;
		nBytes = 2;
		pCurrent += 2;
	} else {
		*pType = TOKEN_OTHER;
		nBytes = 1;
	}
	return nBytes;
}

char *Base64Encoder::AllocLine()
{
	char *p = new char[BASE64_LINE_WIDTH + 1];
	if (p == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
		
	Item *pItem = new Item;
	if (pItem == NULL) {
		delete [] p;
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}

	pItem->pLine = p;
	pItem->pNext = NULL;

	if (tail) {
		tail->pNext = pItem;
		tail = pItem;
	} else {
		head = tail = pItem;
	}
	return p;
}
#endif

////////////////////////////////////////////////////
// misc functions
////////////////////////////////////////////////////

static LPTSTR GetTail(LPTSTR pBuf)
{
#ifdef _WIN32_WCE
	return pBuf + _tcslen(pBuf) - 1;
#else
	LPTSTR p = pBuf;
	LPTSTR pTail = pBuf;
	while(*p) {
		if (iskanji(*p)) {
			pTail = p++;
			if (*p) p++;
		} else {
			pTail = p++;
		}
	}
	return pTail;
#endif
}

void ChopFileSeparator(LPTSTR pBuf)
{
	LPTSTR p;
	while(TRUE) {
		p = GetTail(pBuf);
		if (*p == TEXT('\\')) {
			*p = TEXT('\0');
		} else {
			break;
		}
	}
}

void TrimRight(LPTSTR pStr)
{
	LPTSTR p = pStr;
	LPTSTR pLastSpc = NULL;
	while (*p) {
		if (*p == TEXT(' ') && pLastSpc==NULL) {
			pLastSpc = p;
		}
		if (*p != TEXT(' ')) {
			pLastSpc = NULL;
		}
		p = CharNext(p);
	}
	if (pLastSpc) {
		*pLastSpc = TEXT('\0');
	}
}

////////////////////////////////////////////////////
// strdup clone
////////////////////////////////////////////////////

LPTSTR StringDup(LPCTSTR pStr)
{
	DWORD l = _tcslen(pStr);
	LPTSTR p = new TCHAR[l + 1];
	if (p == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	_tcscpy(p, pStr);
	return p;
}

LPWSTR StringDupW(LPCWSTR pStr)
{
	DWORD l = wcslen(pStr);
	LPWSTR p = new WCHAR[l + 1];
	if (p == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	wcscpy(p, pStr);
	return p;
}

char *StringDupA(const char *pStr)
{
	DWORD l = strlen(pStr);
	char *p = new char[l + 1];
	if (p == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	strcpy(p, pStr);
	return p;
}


////////////////////////////////////////////////////
// 文字列を漢字としてコピー
////////////////////////////////////////////////////

void CopyKanjiString(LPTSTR pDst, LPCTSTR pSrc, DWORD nLen)
{
#ifdef WIN32_WCE
	LPCTSTR p = pSrc;
	LPTSTR q = pDst;
	DWORD n = 0;
	while(*p) {
		if (iskanji(*p)) {
			if (n < nLen - 1) {
				*q++ = *p++;
				*q++ = *p++;
				n+= 2;
				continue;
			} else {
				break;
			}
		} else {
			*q++ = *p++; 
			n++;
		}
	}
	*q = TEXT('\0');
#else
	_tcsncpy(pDst, pSrc, nLen);
#endif
}

////////////////////////////////////////////////////
// Convert UTF-8 to UCS2
////////////////////////////////////////////////////
// Windows CE's WideCharToMultiByte is not support UTF-8... sigh..

// Allocate and convert to UCS2 code. Caller should delete[] the buffer returned.
// if invalid UTF-8 data, return NULL and set GetLastError() to ERROR_INVALID_DATA.

LPWSTR ConvUTF8ToUCS2(const char *pUTFData)
{
	LPWSTR pData = new WCHAR[strlen(pUTFData) + 1];

	const char *p = pUTFData;
	LPWSTR q = pData;

	WORD w1, w2, w3;
	WCHAR c;

	while(*p) {
		if ((*p & 0xF0) == 0xE0 && (*(p+1) & 0xC0) == 0x80 && (*(p+2) & 0xC0) == 0x80) {
			// 3byte code
			w1 = *p & 0x0F;
			w2 = *(p+1) & 0x3F;
			w3 = *(p+2) & 0x3F;
			c = (w1 << 12) | (w2 << 6) | w3;
			*q++ = c;
			p+= 3;
		} else if ((*p & 0xE0) == 0xC0 && (*(p+1) & 0x80) == 0x80) {
			// 2byte code
			w1 = *p & 0x1F;
			w2 = *(p+1) & 0x3F;
			c = (w1 << 6) | w2;
			*q++ = c;
			p+= 2;
		} else if ((*p & 0x80) == 0x00) {
			// 1byte code
			w1 = *p;
			c = w1;
			*q++ = c;
			p++;
		} else {
			// illegal code
			delete [] pData;
			SetLastError(ERROR_INVALID_DATA);
			return NULL;
		}

	}
	*q = TEXT('\0');

	return pData;
}

////////////////////////////////////////////////////
// UCS2 -> UTF-8
////////////////////////////////////////////////////
char *ConvUCS2ToUTF8(LPCWSTR pStr)
{
	DWORD len = wcslen(pStr);
	char *pBuf = new char[(len + 1) * 3];
	if (pBuf == NULL) { 
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}
	
	LPCWSTR p = pStr;
	char *q = pBuf;
	while(*p) {
		if (*p < 0x80) {
			*q++ = (char)(*p & 0x007F);
		} else if (*p < 0x800) {
			*q++ = (char)(((*p & 0x07C0) >> 6) | 0xC0);
			*q++ = (char)(*p & 0x003F) | 0x80;
		} else {
			*q++ = (char)(((*p >> 12) & 0x0F) | 0xE0);
			*q++ = (char)(((*p & 0x0FC0) >> 6) | 0x80);
			*q++ = (char)((*p & 0x3F) | 0x80);
		}
		p++;
	}

	*q = '\0';
	return pBuf;
}

////////////////////////////////////////////////////
// Escape XML special string
////////////////////////////////////////////////////

char *EscapeXMLStr(LPCTSTR pStr)
{
	LPWSTR pWStr;
#if defined(PLATFORM_WIN32)
	pWStr =  ConvTCharToWChar(pStr);
	ArrayAutoPointer<WCHAR> ap1(pWStr);
	if (pWStr == NULL) return NULL;
#else
	pWStr = (LPWSTR)pStr;
#endif

	// check escape string is exist
	DWORD nExt = 0;
	LPCWSTR p = pWStr;
	while(*p) {
		if (*p == TEXT('<') || *p == TEXT('>') ||
			*p == TEXT('&') || 
			*p == TEXT('\'') || *p == TEXT('"')) {
			nExt += 6;
		}
		p++;
	}

	LPWSTR pUCS;
	ArrayAutoPointer<WCHAR> ap2;

	if (nExt == 0) {
		pUCS = pWStr;
	} else {
		// Need escape
		LPWSTR pEscaped = new WCHAR[wcslen(pWStr) + nExt + 1];
		ap2.set(pEscaped);
		
		LPWSTR q = pEscaped;
		p = pWStr;
		while(*p) {
			switch (*p) {
			case L'<':
				wcscpy(q, L"&lt;"); q+= 4;
				break;
			case L'>':
				wcscpy(q, L"&gt;"); q+= 4;
				break;
			case L'&':
				wcscpy(q, L"&amp;"); q+= 5;
				break;
			case L'\'':
				wcscpy(q, L"&apos;"); q+= 6;
				break;
			case L'"':
				wcscpy(q, L"&quot;"); q+= 6;
				break;
			default:
				*q++ = *p;
			}
			p++;
		}
		*q = L'\0';
		pUCS = pEscaped;
	}
	
	return ConvUCS2ToUTF8(pUCS);
}

////////////////////////////////////////////////////
// Base64 encode/decode
////////////////////////////////////////////////////

#ifdef UNIT_TEST
DWORD g_Base64EncodeAllocSize;
#endif

char *Base64Encode(const LPBYTE pBinary, DWORD nSrcLen)
{
	if (pBinary == NULL || nSrcLen == 0) return NULL;

	static char enctable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	DWORD nBufSiz = nSrcLen / 3 * 4;
	if (nSrcLen % 3 != 0) { nBufSiz += 4; }
	nBufSiz++; // for \0

#ifdef UNIT_TEST
	g_Base64EncodeAllocSize = nBufSiz;
#endif

	// alloc
	unsigned char *pOutBuf = new unsigned char[nBufSiz];
	if (pOutBuf == NULL) return NULL;

	LPBYTE p = pBinary;
	unsigned char *q = pOutBuf;

	DWORD v;
	DWORD n = nSrcLen;

	while(n >= 3) {
		v = *p >> 2;
		*q++ = enctable[v];
		v = ((*p & 3) << 4) + (*(p+1) >> 4);
		*q++ = enctable[v];
		v = ((*(p+1) & 0xF) << 2) + (*(p+2) >> 6);
		*q++ = enctable[v];
		v = *(p+2) & 0x3F;
		*q++ = enctable[v];

		p += 3;
		n -= 3;
	}

	if (n == 2) {
		v = *p >> 2;
		*q++ = enctable[v];
		v = ((*p & 3) << 4) + (*(p+1) >> 4);
		*q++ = enctable[v];
		v = ((*(p+1) & 0x0F) << 2);
		*q++ = enctable[v];
		*q++ = '=';
	} else if (n == 1) {
		v = *p >> 2;
		*q++ = enctable[v];
		v = (*p & 3) << 4;
		*q++ = enctable[v];
		*q++ = '=';
		*q++ = '=';
	}
	*q = '\0';
	return (char*)pOutBuf;
}


LPBYTE Base64Decode(const char *pM64str, LPDWORD pDataSize)
{
	DWORD nPrevSize = strlen(pM64str);
	LPBYTE pDecData = new BYTE[nPrevSize];

	if (pDecData == NULL || nPrevSize % 4 != 0) return NULL;

	const char *p = pM64str;
	LPBYTE q = pDecData;
	BYTE b1, b2, b3;

	unsigned char c1, c2, c3, c4;

	while(*p) {
		c1 = dec64(*p++);
		c2 = dec64(*p++);
		c3 = dec64(*p++);
		c4 = dec64(*p++);

		b1 = (c1 << 2) | ((c2 & 0x30) >> 4);
		*q++ = b1;

		if (c3 != 64) {
			b2 = ((c2 & 0xF) << 4) | ((c3 & 0x3c) >> 2);
			*q++ = b2;
		}

		if (c3 != 64 && c4 != 64) {
			b3 = ((c3 & 0x3) << 6) | c4;
			*q++ = b3;
		}
	}
	*pDataSize = q - pDecData;
	return pDecData;
}

////////////////////////////////////////////////////////////////////
// ファイル名として使用できない文字を抜いた形で文字列をコピー
////////////////////////////////////////////////////////////////////

// ヘッドライン除外文字列
#define SKIPCHAR TEXT("\\/:,;*?<>\"\t")

void DropInvalidFileChar(LPTSTR pDst, LPCTSTR pSrc)
{
	LPTSTR q = pDst;
	LPCTSTR p = pSrc;

	// ファイル名として使用できない文字をスキップしてヘッドラインをコピー
	while(*p) {
#ifndef _WIN32_WCE
		if (IsDBCSLeadByte(*p)) {
			*q++ = *p++;
			*q++ = *p++;
			continue;
		}
#endif
		if (_tcschr(SKIPCHAR, *p) != NULL) {
			p++;
			continue;
		}
		*q++ = *p++;
	}
	*q = TEXT('\0');
}

////////////////////////////////////////////////////////////////////
// find next '\\'
////////////////////////////////////////////////////////////////////

LPCTSTR GetNextDirSeparator(LPCTSTR pStart)
{
	LPCTSTR p = pStart;
	while(*p) {
#if defined(PLATFORM_WIN32)
		if (IsDBCSLeadByte((BYTE)*p)) {
			p++;
			if (*p) p++;
			continue;
		}
#endif
		if (*p == TEXT('\\')) return p;
		p++;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////
// Get file path
////////////////////////////////////////////////////////////////////

void GetFilePath(LPTSTR pFilePath, LPCTSTR pFileName)
{
	LPCTSTR p = pFileName;
	LPCTSTR q = NULL;

	// get last position of '\'
	while(*p) {
#ifdef PLATFORM_WIN32
		if (IsDBCSLeadByte((BYTE)*p)) {
			p+= 2;
			continue;
		}
#endif
		if (*p == TEXT('\\')) {
			q = p;
		}
		p++;
	}
	if (q == NULL) {
		*pFilePath = TEXT('\0');
		return;
	}
	_tcsncpy(pFilePath, pFileName, q - pFileName + 1);
	*(pFilePath + (q - pFileName + 1)) = TEXT('\0');
}

////////////////////////////////////////////////////////////////////
// Get file path
////////////////////////////////////////////////////////////////////

void WipeOutAndDelete(LPTSTR p)
{
	if (p == NULL) return;

	LPTSTR q = p;
	while (*q) {
		*q++ = TEXT('\0');
	}
	delete [] p;
}

#ifdef _WIN32_WCE
void WipeOutAndDelete(char *p)
{
	if (p == NULL) return;

	char *q = p;
	while (*q) {
		*q++ = TEXT('\0');
	}
	delete [] p;
}
#endif

/////////////////////////////////////////////
// Clear file contents and delete it
/////////////////////////////////////////////

BOOL WipeOutAndDeleteFile(LPCTSTR pFile)
{
	File delf;
	if (!delf.Open(pFile, GENERIC_WRITE, 0, OPEN_ALWAYS)) return FALSE;

	DWORD i;
	DWORD nSize = delf.FileSize() / 64 + 1;
	BYTE buf[64];
	for (i = 0; i < 64; i++) buf[i] = 0;

	for (i = 0; i < nSize; i++) {
		delf.Write(buf, 64);
	}
	delf.Close();
	return DeleteFile(pFile);
}
