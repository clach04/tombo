#include <windows.h>
#include <tchar.h>
#include "Tombo.h"
#include "Message.h"
#include "Uniconv.h"

#include "TString.h"

//////////////////////////////////////////////////
// prototype
//////////////////////////////////////////////////
static void ReplaceSpecialChar(LPTSTR pSrc);

#include "MsgRes/DefaultMsg.cpp"

 ////////////////////////////////////////////////
/////////////////////////////////////////////////
// TomboMessage implimentation
/////////////////////////////////////////////////
////////////////////////////////////////////////

//////////////////////////////////////////////////
// ctor & initializer
//////////////////////////////////////////////////

TomboMessage::TomboMessage() : pMsgBuf(NULL)
{
}

TomboMessage::~TomboMessage()
{
	delete [] pMsgBuf;
}

BOOL TomboMessage::Init()
{
	// load default message
	for (DWORD i = 0; i < NUM_MESSAGES; i++) {
		pMsg[i] = defaultMsg[i];
	}

	TCHAR buf[MAX_PATH * 2];
	TCHAR buf2[MAX_PATH * 2];

	GetModuleFileName(NULL, buf, MAX_PATH);
	GetFilePath(buf2, buf);

	_tcscat(buf2, TOMBO_MSG_DEF_FILE);

	HANDLE h = CreateFile(buf2, GENERIC_READ, FILE_SHARE_READ, 
							NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h != INVALID_HANDLE_VALUE) {
		LoadMsg(h);
	}
	CloseHandle(h);
	return TRUE;
}

//////////////////////////////////////////////////
// Get data from file and convert to native code.
//////////////////////////////////////////////////
// In windows platform, code is for each MBCS codeset.
// In CE platform, code is UCS2.

LPTSTR TomboMessage::GetNatvieData(HANDLE hFile)
{
	// read UTF-8 file
	DWORD nBytes = GetFileSize(hFile, NULL);
	char *pTmp = new char[nBytes + 1];
	if (!ReadFile(hFile, (LPBYTE)pTmp, nBytes, &nBytes, NULL)) {
		delete [] pTmp;
		return NULL;
	}
	*(pTmp + nBytes) = '\0';

#if defined(PLATFORM_WIN32)
	// convert to UCS2 data
	LPWSTR pW = ConvUTF8ToUCS2(pTmp);
	delete [] pTmp;

	// convert to MBCS data
	int nSize = WideCharToMultiByte(CP_ACP, 0, pW, -1, NULL, 0, NULL, NULL);
	char *pData = new char[nSize + 1];
	WideCharToMultiByte(CP_ACP, 0, pW, -1, pData, nSize, NULL, NULL);
	delete [] pW;
#else
	LPWSTR pData = ConvUTF8ToUCS2(pTmp);
	delete[] pTmp;
#endif
	return pData;
}

//////////////////////////////////////////////////
// Load message
//////////////////////////////////////////////////

BOOL TomboMessage::LoadMsg(HANDLE h)
{
	LPTSTR p = pMsgBuf = GetNatvieData(h);
	if (p == NULL) return FALSE;

	DWORD i = 0;

	pMsg[i++] = p;
	while (*p) {
		if (*p == TEXT('\r') && *(p+1) == TEXT('\n')) {
			*p = *(p+1) = TEXT('\0');
			p += 2;
			if (i >= NUM_MESSAGES) break;
			if (_tcslen(p) == 0) break;
			pMsg[i++] = p;
		}
#if defined(PLATFORM_WIN32)
		p = CharNext(p);
#else
		p++;
#endif
	}

	DWORD nValid = i;

	for (i= 0; i < nValid; i++) {
		ReplaceSpecialChar((LPTSTR)pMsg[i]);
	}
	return TRUE;
}

//////////////////////////////////////////////////
// Retrieve message
//////////////////////////////////////////////////

LPCTSTR TomboMessage::GetMsg(DWORD nMsgID)
{
	if (nMsgID == 0 || nMsgID > NUM_MESSAGES) return NULL;
	return pMsg[nMsgID - 1];
}

//////////////////////////////////////////////////
// Replace escape char to real char
//////////////////////////////////////////////////

static void ReplaceSpecialChar(LPTSTR pSrc)
{
	LPTSTR p;
	LPTSTR q;
	p = q = pSrc;
	while(*p) {
		if (*p == TEXT('\\')) {
			switch (*(p+1)) {
			case TEXT('r'):
				*q++ = TEXT('\r');
				p+= 2;
				break;
			case TEXT('n'):
				*q++ = TEXT('\n');
				p+= 2;
				break;
			case TEXT('\\'):
				*q++ = TEXT('\\');
				p+= 2;
				break;
			case TEXT('t'):
				*q++ = TEXT('\t');
				p+= 2;
				break;
			case TEXT('0'):
				*q++ = TEXT('\0');
				p+= 2;
				break;
			default:
				*q++ = *p++;
			}
		} else {
#if defined(PLATFORM_WIN32)
			if (IsDBCSLeadByte(*p)) {
				*q++ = *p++;
			}
#endif
			*q++ = *p++;
		}
	}
	*q = TEXT('\0');
}
