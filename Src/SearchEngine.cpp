#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <string.h>

#include "Tombo.h"
#include "SearchEngine.h"
#include "TString.h"
#include "UniConv.h"
#include "Property.h"
#include "TomboURI.h"
#include "Repository.h"

#include "AutoPtr.h"

#include "RegexUtil.h"

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
// regex search (MBCS version)
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// ctor & dtor
////////////////////////////////////////////////////////

SearchEngineA::SearchEngineA() : pCompiledPattern(NULL), pPattern(NULL)
{
}

SearchEngineA::~SearchEngineA()
{
	delete [] pPattern;
	if (pCompiledPattern) {
		Regex_Free(pCompiledPattern);
	}
}

BOOL SearchEngineA::Init(DWORD nCP, BOOL bSE, BOOL bFo, PasswordManager *pPMgr)
{
	nCodePage = nCP;
	bSearchEncrypt = bSE;
	bFileNameOnly = bFo;
	pPassMgr = pPMgr;
	return TRUE;
}

////////////////////////////////////////////////////////
// prepare regex pattern
////////////////////////////////////////////////////////

BOOL SearchEngineA::Prepare(LPCTSTR pPat, BOOL bCS, const char **ppReason)
{
	delete[] pPattern;	// release previous Prepare data
	pPattern = ConvTCharToFileEncoding(pPat, &nPatLen);

	if (pPattern == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		*ppReason = "Not enough memory";
		return FALSE;
	}

	if (pCompiledPattern) {
		Regex_Free(pCompiledPattern);
	}

	pCompiledPattern = Regex_Compile(pPattern, !bCS, ppReason, nCodePage);
	if (!pCompiledPattern) {
		return FALSE;
	}

	bCaseSensitive = bCS;
	return TRUE;
}

////////////////////////////////////////////////////////
// search from text
////////////////////////////////////////////////////////

BOOL SearchEngineA::SearchTextA(const LPBYTE pText, DWORD nStartPos, BOOL bForward, BOOL bShift)
{
	LPBYTE p = pText + nStartPos;

	if (!pCompiledPattern) return TRUE;

	// if bShift is TRUE, shift start position 1 letter
	if (bShift) {
		if (bForward) {
			p = ShiftRight(pText, p, nCodePage);
		} else {
			p = ShiftLeft(pText, p, nCodePage);
		}
	}

	// execute searching
	int s, e;
	int res = Regex_Search(pCompiledPattern, (p - pText), pText, bForward, &s, &e, nCodePage);
	if (res >= 0) {
		nMatchStart = s;
		nMatchEnd = e;
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL SearchEngineA::SearchTextT(LPCTSTR pText, DWORD nStartPos, BOOL bForward, BOOL bShift)
{
	DWORD nTextLen;
	LPBYTE pData = ConvTCharToFileEncoding(pText, &nTextLen);
	SecureBufferAutoPointerByte ap(pData, nTextLen);

#if defined(PLATFORM_WIN32)
	DWORD nSystemCodePage = 0;
#else
	DWORD nSystemCodePage = TOMBO_CP_UTF16LE;
#endif

	// convert unicode pos to MBCS pos
	DWORD nStartPosA = ConvertPos((LPBYTE)pText, nStartPos * sizeof(TCHAR), nSystemCodePage, pData, nCodePage);

	// exec searching
	BOOL bResult = SearchTextA(pData, nStartPosA, bForward, bShift);

	// if matched, convert MBCS pos to unicode pos
	if (bResult) {
		DWORD nMatchStartA = nMatchStart;
		DWORD nMatchEndA = nMatchEnd;

		nMatchStart = ConvertPos(pData, nMatchStartA, nCodePage, (LPBYTE)pText, nSystemCodePage) / sizeof(TCHAR);
		nMatchEnd = ConvertPos(pData, nMatchEndA, nCodePage, (LPBYTE)pText, nSystemCodePage) / sizeof(TCHAR);
	}
	return bResult;
}

////////////////////////////////////////////////////////
// Matching
////////////////////////////////////////////////////////

SearchResult SearchEngineA::SearchFromURI(const TomboURI *pURI)
{
	if (bFileNameOnly) {
		TString sPartName;
		if (!g_Repository.GetFileName(pURI, &sPartName)) return SR_FAILED;

		BOOL bMatch;

		DWORD nSize;
		LPBYTE pText = ConvTCharToFileEncoding(sPartName.Get(), &nSize);
		SecureBufferAutoPointerByte ap(pText, nSize);

		bMatch = (Regex_Search(pCompiledPattern, 0, pText, TRUE, NULL, NULL, nCodePage) >= 0);

		return bMatch ? SR_FOUND : SR_NOTFOUND;
	} else {
		URIOption opt(NOTE_OPTIONMASK_ENCRYPTED);
		if (!g_Repository.GetOption(pURI, &opt)) return SR_FAILED;

		// skip crypted note if it is not search target.
		if (!IsSearchEncryptMemo() && opt.bEncrypt) return SR_NOTFOUND;

		DWORD nSize;
		LPBYTE pMemo = g_Repository.GetNoteDataNative(pURI, &nSize);
		if (pMemo == NULL) return SR_FAILED;
		SecureBufferAutoPointerByte ap(pMemo, nSize);

		BOOL bMatch = (Regex_Search(pCompiledPattern, 0, pMemo, TRUE, NULL, NULL, nCodePage) >= 0);
		return bMatch ? SR_FOUND : SR_NOTFOUND;
	}
}

////////////////////////////////////////////////////////
// Duplicate object
////////////////////////////////////////////////////////

SearchEngineA *SearchEngineA::Clone()
{
	const char *pReason;
	SearchEngineA *p = new SearchEngineA();
	if (!p) return NULL;

	LPTSTR pPat = ConvFileEncodingToTChar(pPattern);
	ArrayAutoPointer<TCHAR> ap(pPat);

	if (!p->Init(nCodePage, bSearchEncrypt, bFileNameOnly, pPassMgr) ||
		!p->Prepare(pPat, bCaseSensitive, &pReason)) {
		delete p;
		return NULL;
	}
	return p;
}


