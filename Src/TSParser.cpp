#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#define XMLPARSEAPI(type) type __cdecl	// for expat
#define XML_UNICODE_WCHAR_T
#include <expat.h>
#include "Tombo.h"
#include "UniConv.h"
#include "TSParser.h"
#include "VFStream.h"
#include "VFManager.h"
#include "AutoPtr.h"
#include "TString.h"
#include "TomboURI.h"

///////////////////////////////////////
// UCS2 -> MBCS conversion libs.
///////////////////////////////////////

class ConvertWideToMultiByte {
	char *p;
public:
	ConvertWideToMultiByte() : p(NULL) {}
	~ConvertWideToMultiByte() { delete []p; }

	char *Convert(WCHAR *p);
	char *Get() { return p; }
};

char *ConvertWideToMultiByte::Convert(WCHAR *pSrc)
{
	if (pSrc == NULL) return NULL;

	DWORD n = (wcslen(pSrc) + 1) * 2;
	p = new char[n];
	if (p == NULL) return NULL;
	WideCharToMultiByte(CP_ACP, 0, pSrc, -1, p, n, NULL, NULL);
	return p;
}

///////////////////////////////////////
// TSParser ctor&dtor
///////////////////////////////////////

TSParser::TSParser()
{
}

TSParser::~TSParser()
{
}

///////////////////////////////////////
// XML tag info 
///////////////////////////////////////

#define TAGID_UNKNOWN	0
#define TAGID_INITIAL	1
#define TAGID_FOLDERS	2
#define TAGID_VFOLDER	3
#define TAGID_GREP		4
#define TAGID_SRC		5
#define TAGID_TIMESTAMP	6
#define TAGID_LIMIT		7
#define TAGID_ORDER		8

static DWORD nAllowParent[] = {
	0,						// TAGID_UNKONWN
	0,						// TAGID_INITIAL
	(1 << TAGID_INITIAL),	// TAGID_FOLDERS
	(1 << TAGID_FOLDERS),	// TAGID_VFOLDER
	(1 << TAGID_VFOLDER) | (1 << TAGID_GREP) | (1 << TAGID_TIMESTAMP) | (1 << TAGID_LIMIT) | (1 << TAGID_ORDER),
							// TAGID_GREP
	(1 << TAGID_VFOLDER) | (1 << TAGID_GREP) | (1 << TAGID_TIMESTAMP) | (1 << TAGID_LIMIT) | (1 << TAGID_ORDER),
							// TAGID_SRC
	(1 << TAGID_VFOLDER) | (1 << TAGID_GREP) | (1 << TAGID_TIMESTAMP) | (1 << TAGID_LIMIT) | (1 << TAGID_ORDER),
							// TAGID_TIMESTAMP
	(1 << TAGID_VFOLDER) | (1 << TAGID_GREP) | (1 << TAGID_TIMESTAMP) | (1 << TAGID_LIMIT) | (1 << TAGID_ORDER),
							// TAGID_LIMIT
	(1 << TAGID_VFOLDER) | (1 << TAGID_GREP) | (1 << TAGID_TIMESTAMP) | (1 << TAGID_LIMIT) | (1 << TAGID_ORDER),
							// TAGID_ORDER
};

///////////////////////////////////////
// TAG data
///////////////////////////////////////

TSParseTagItem::~TSParseTagItem()
{
}

BOOL TSParseTagItem::StartElement(ParseInfo *p, const XML_Char **atts)
{
	return TRUE;
}

BOOL TSParseTagItem::EndElement(ParseInfo *p)
{
	return TRUE;
}

///////////////////////////////////////
//  "src" tag implimentation
///////////////////////////////////////

class TSSrcTag : public TSParseTagItem {
	WCHAR *pSrc;
	WCHAR *pURI;
	BOOL bCheckEncrypt;
public:
	TSSrcTag() : TSParseTagItem(TAGID_SRC), pSrc(NULL), bCheckEncrypt(FALSE), pURI(NULL) {}
	~TSSrcTag();

	BOOL StartElement(ParseInfo *p, const XML_Char **atts);
	BOOL EndElement(ParseInfo *p);
};

TSSrcTag::~TSSrcTag()
{
	delete [] pSrc;
	delete [] pURI;
}

BOOL TSSrcTag::StartElement(ParseInfo *p, const XML_Char **atts)
{
	bCheckEncrypt = FALSE;

	DWORD i = 0;
	while(atts[i] != NULL) {
		if (wcsicmp(atts[i], L"folder") == 0) {
			delete[] pSrc;
			pSrc = StringDupW(atts[i + 1]);
			if (pSrc == NULL) return FALSE;
		} else if (wcsicmp(atts[i], L"uri") == 0) {
			delete[] pURI;
			pURI = StringDupW(atts[i + 1]);
			if (pURI == NULL) return FALSE;
		} else if (wcsicmp(atts[i], L"checkencrypt") == 0) {
			bCheckEncrypt = TRUE;
		}

		i += 2;
	}

	if (!pSrc && !pURI) {
		// necessary attribute is not found.
		return FALSE;
	}
	return TRUE;
}

BOOL TSSrcTag::EndElement(ParseInfo *p)
{
	if (pHead != NULL) {
		// "src" tag can't have sub items.
		return FALSE;
	}

	VFDirectoryGenerator *pGen = new VFDirectoryGenerator();
	if (pGen == NULL) return FALSE;

	LPTSTR pSrcPath = ConvWCharToTChar(pSrc);
	ArrayAutoPointer<TCHAR> ap1(pSrcPath);

	LPTSTR pSrcURI = ConvWCharToTChar(pURI);
	ArrayAutoPointer<TCHAR> ap2(pSrcURI);

	if (pSrcPath && !pGen->Init(pSrcPath, bCheckEncrypt)) return FALSE;
	else if (pSrcURI) {
		TomboURI uri;
		if (!uri.Init(pSrcURI)) return FALSE;
		if (!pGen->Init(&uri, bCheckEncrypt)) return FALSE;
	}
	// Pass create object to parent item
	TSParseTagItem *pParent = pNext;
	pParent->pHead = pParent->pTail = pGen;
	return TRUE;
}

///////////////////////////////////////
// "grep" tag implimentation
///////////////////////////////////////

class TSGrepTag : public TSParseTagItem {
	WCHAR *pPattern;
	BOOL bCaseSensitive;
	BOOL bFileNameOnly;
	BOOL bNegate;
	BOOL bCheckEncrypt;
public:
	TSGrepTag() : TSParseTagItem(TAGID_GREP), pPattern(NULL) {}
	~TSGrepTag();

	BOOL StartElement(ParseInfo *p, const XML_Char **atts);
	BOOL EndElement(ParseInfo *p);
};

TSGrepTag::~TSGrepTag()
{
	if (pPattern) delete [] pPattern;
}

BOOL TSGrepTag::StartElement(ParseInfo *p, const XML_Char **atts)
{
	bCaseSensitive = bFileNameOnly = bNegate = bCheckEncrypt = FALSE;
	DWORD i = 0;
	while(atts[i] != NULL) {
		if (wcsicmp(atts[i], L"pattern") == 0) {
			pPattern = new WCHAR[wcslen(atts[i + 1]) + 1];
			if (pPattern == NULL) return FALSE;
			wcscpy(pPattern, atts[i + 1]);
		} else if (wcsicmp(atts[i], L"casesensitive") == 0) {
			bCaseSensitive = TRUE;
		} else if (wcsicmp(atts[i], L"filenameonly") == 0) {
			bFileNameOnly = TRUE;
		} else if (wcsicmp(atts[i], L"not") == 0) {
			bNegate = TRUE;
		} else if (wcsicmp(atts[i], L"checkencrypt") == 0) {
			bCheckEncrypt = TRUE;
		}
		i += 2;
	}
	if (!pPattern) {
		return FALSE;
	}
	return TRUE;
}

BOOL TSGrepTag::EndElement(ParseInfo *p)
{
	if (pHead == NULL) {
		// Grep tag should have child tag.
		return FALSE;
	}
	VFRegexFilter *pFilter = new VFRegexFilter();

#ifdef _WIN32_WCE
	LPTSTR pConved = pPattern;
#else
	ConvertWideToMultiByte conv;
	if (!conv.Convert(pPattern)) return FALSE;
	LPTSTR pConved = conv.Get();
#endif
	if (!pFilter || !pFilter->Init(pConved, bCaseSensitive, bCheckEncrypt, bFileNameOnly, bNegate, g_pPasswordManager)) return FALSE;

	// Pass create object to parent item
	TSParseTagItem *pParent = pNext;
	pTail->SetNext(pFilter);
	pParent->pHead = pHead;
	pParent->pTail = pFilter;
	return TRUE;
}

///////////////////////////////////////
// "vfolder" tag implimentation
///////////////////////////////////////

class TSVFolderTag : public TSParseTagItem {
	WCHAR *pName;
public:
	TSVFolderTag() : TSParseTagItem(TAGID_VFOLDER), pName(NULL){}
	~TSVFolderTag();

	BOOL StartElement(ParseInfo *p, const XML_Char **atts);
	BOOL EndElement(ParseInfo *p);
};

TSVFolderTag::~TSVFolderTag()
{
	if (pName) delete[] pName;
}

BOOL TSVFolderTag::StartElement(ParseInfo *p, const XML_Char **atts)
{
	DWORD i = 0;
	while(atts[i] != NULL) {
		if (wcsicmp(atts[i], L"name") == 0) {
			pName = new WCHAR[wcslen(atts[i + 1]) + 1];
			if (pName == NULL) return FALSE;
			wcscpy(pName, atts[i + 1]);
		}
		i += 2;
	}
	if (!pName) {
		return FALSE;
	}
	return TRUE;
}

BOOL TSVFolderTag::EndElement(ParseInfo *p)
{
	if (pHead == NULL || pTail == NULL) return FALSE;

	// add VFStore
	VFStore *pStore = new VFStore();
	if (!pStore || !pStore->Init()) {
		delete pStore;
		return FALSE;
	}
	pTail->SetNext(pStore);

	// convert Node name
	// TOMBO uses expat UNICODE version, so convert MBCS if platform is win32.

#ifdef _WIN32_WCE
	LPTSTR pConved = pName;
#else
	ConvertWideToMultiByte conv;
	if (!conv.Convert(pName)) return FALSE;
	LPTSTR pConved = conv.Get();
#endif

	p->pListener->ProcessStream(pConved, TRUE, (VFDirectoryGenerator*)pHead, pStore);

	return TRUE;
}

///////////////////////////////////////
// "timestamp" tag implimentation
///////////////////////////////////////

class TSTimestampTag : public TSParseTagItem {
	DWORD nDelta;
	DWORD nRecent;
public:
	TSTimestampTag() : TSParseTagItem(TAGID_TIMESTAMP) {}
	~TSTimestampTag() {}

	BOOL StartElement(ParseInfo *p, const XML_Char **atts);
	BOOL EndElement(ParseInfo *p);
	
};

BOOL TSTimestampTag::StartElement(ParseInfo *p, const XML_Char **atts)
{
	DWORD i = 0;
	nRecent = TRUE;
	nDelta = 0xFFFFFFFF;
	while(atts[i] != NULL) {
		if (wcsicmp(atts[i], L"days") == 0) {
			// atts[i + 1];
			nDelta = _wtol(atts[i + 1]);
		}
		if (wcsicmp(atts[i], L"older") == 0) {
			nRecent = FALSE;
		}
		if (wcsicmp(atts[i], L"newer") == 0) {
			nRecent = TRUE;
		}
		i += 2;
	}
	if (nDelta == 0xFFFFFFFF) return FALSE;
	return TRUE;
}

BOOL TSTimestampTag::EndElement(ParseInfo *p)
{
	if (pHead == NULL) return FALSE;

	VFTimestampFilter *pFilter = new VFTimestampFilter();
	if (pFilter == NULL || !pFilter->Init(nDelta, nRecent)) return FALSE;
	TSParseTagItem *pParent = pNext;
	pTail->SetNext(pFilter);
	pParent->pHead = pHead;
	pParent->pTail = pFilter;
	return TRUE;
}

///////////////////////////////////////
// "limit" tag implimentation
///////////////////////////////////////

class TSLimitTag : public TSParseTagItem {
	DWORD nLimit;
public:
	TSLimitTag() : TSParseTagItem(TAGID_LIMIT) {}
	~TSLimitTag() {}

	BOOL StartElement(ParseInfo *p, const XML_Char **atts);
	BOOL EndElement(ParseInfo *p);

	DWORD GetLimit() { return nLimit; }
};

BOOL TSLimitTag::StartElement(ParseInfo *p, const XML_Char **atts)
{
	nLimit = 0xFFFFFFFF;
	DWORD i = 0;
	while(atts[i] != NULL) {
		if (wcsicmp(atts[i], L"number") == 0) {
			// atts[i + 1];
			nLimit = _wtol(atts[i + 1]);
		}
		i += 2;
	}
	if (nLimit == 0xFFFFFFFF) return FALSE;
	return TRUE;
}

BOOL TSLimitTag::EndElement(ParseInfo *p)
{
	if (pHead == NULL) return FALSE;

	VFLimitFilter *pFilter = new VFLimitFilter();
	if (pFilter == NULL || !pFilter->Init(nLimit)) return FALSE;
	TSParseTagItem *pParent = pNext;
	pTail->SetNext(pFilter);
	pParent->pHead = pHead;
	pParent->pTail = pFilter;
	return TRUE;
}

///////////////////////////////////////
// "order" tag implimentation
///////////////////////////////////////

class TSOrderTag : public TSParseTagItem {
	VFSortFilter::SortFuncType sfType;
public:
	TSOrderTag() : TSParseTagItem(TAGID_ORDER){}
	~TSOrderTag() {}

	BOOL StartElement(ParseInfo *p, const XML_Char **atts);
	BOOL EndElement(ParseInfo *p);
};

BOOL TSOrderTag::StartElement(ParseInfo *p, const XML_Char **atts)
{
	DWORD i = 0;
	while(atts[i] != NULL) {
		if (wcsicmp(atts[i], L"func") == 0) {
			if (wcsicmp(atts[i + 1], L"filename_asc") == 0) {
				sfType = VFSortFilter::SortFunc_FileNameAsc;
			} else if (wcsicmp(atts[i + 1], L"filename_dsc") == 0) {
				sfType = VFSortFilter::SortFunc_FileNameDsc;
			} else if (wcsicmp(atts[i + 1], L"lastupdate_asc") == 0) {
				sfType = VFSortFilter::SortFunc_LastUpdateAsc;
			} else if (wcsicmp(atts[i + 1], L"lastupdate_dsc") == 0) {
				sfType = VFSortFilter::SortFunc_LastUpdateDsc;
			} else if (wcsicmp(atts[i + 1], L"createdate_asc") == 0) {
				sfType = VFSortFilter::SortFunc_CreateDateAsc;
			} else if (wcsicmp(atts[i + 1], L"createdate_dsc") == 0) {
				sfType = VFSortFilter::SortFunc_CreateDateDsc;
			} else if (wcsicmp(atts[i + 1], L"filesize_asc") == 0) {
				sfType = VFSortFilter::SortFunc_FileSizeAsc;
			} else if (wcsicmp(atts[i + 1], L"filesize_dsc") == 0) {
				sfType = VFSortFilter::SortFunc_FileSizeDsc;
			} else {
				return FALSE;
			}
		}
		i += 2;
	}
	return TRUE;
}

BOOL TSOrderTag::EndElement(ParseInfo *p)
{
	if (pHead == NULL) return FALSE;

	VFSortFilter *pFilter = new VFSortFilter();
	if (pFilter == NULL || !pFilter->Init(sfType)) return FALSE;
	TSParseTagItem *pParent = pNext;
	pTail->SetNext(pFilter);
	pParent->pHead = pHead;
	pParent->pTail = pFilter;
	return TRUE;
}

///////////////////////////////////////
// ParseInfo implimentation
///////////////////////////////////////

ParseInfo::~ParseInfo()
{
	TSParseTagItem *p = pTop;
	TSParseTagItem *q;
	while(p) {
		q = p;
		p = p->GetNext();
		delete q;
	}
}

BOOL ParseInfo::Init(VirtualFolderEnumListener *pLsnr)
{
	pListener = pLsnr;

	TSParseTagItem *pTag = new TSParseTagItem(TAGID_INITIAL);
	if (pTag == NULL) return FALSE;
	Push(pTag);
	return TRUE;
}

DWORD ParseInfo::GetTagID(const WCHAR *pTagName)
{
	if (wcsicmp(pTagName, L"folders") == 0) {
		return TAGID_FOLDERS;
	} else if (wcsicmp(pTagName, L"vfolder") == 0) {
		return TAGID_VFOLDER;
	} else if (wcsicmp(pTagName, L"grep") == 0) {
		return TAGID_GREP;
	} else if (wcsicmp(pTagName, L"src") == 0) {
		return TAGID_SRC;
	} else if (wcsicmp(pTagName, L"timestamp") == 0) {
		return TAGID_TIMESTAMP;
	} else if (wcsicmp(pTagName, L"limit") == 0) {
		return TAGID_LIMIT;
	} else if (wcsicmp(pTagName, L"sort") == 0) {
		return TAGID_ORDER;
	} else {
		return TAGID_UNKNOWN;
	}
}

TSParseTagItem *ParseInfo::GetTagObjectFactory(DWORD nTagID)
{
	switch (nTagID) {
	case TAGID_FOLDERS:
		return new TSParseTagItem(TAGID_FOLDERS);
	case TAGID_VFOLDER:
		return new TSVFolderTag();
	case TAGID_GREP:
		return new TSGrepTag();
	case TAGID_SRC:
		return new TSSrcTag();
	case TAGID_TIMESTAMP:
		return new TSTimestampTag();
	case TAGID_LIMIT:
		return new TSLimitTag();
	case TAGID_ORDER:
		return new TSOrderTag();
	default:
		return NULL;
	}
}

void ParseInfo::Push(TSParseTagItem *p)
{
	p->SetNext(pTop); 
	pTop = p;
}

void ParseInfo::Pop()
{
	TSParseTagItem *p = pTop;
	pTop = p->GetNext();
	delete p;
}

BOOL ParseInfo::IsValidParent(DWORD nTag)
{
	return ((nAllowParent[nTag] & (1 << pTop->GetTagID())) != 0);
}

///////////////////////////////////////
// expat callback funcs.
///////////////////////////////////////

static void StartElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
	ParseInfo *pInfo = (ParseInfo*)userData;
	if (pInfo->IsError()) return;

	// Check tag
	DWORD nCurTag = pInfo->GetTagID(name);
	if (nCurTag == TAGID_UNKNOWN) {
		pInfo->SetError();
		return;
	}
	if (!pInfo->IsValidParent(nCurTag)) {
		pInfo->SetError();
		return;
	}
	TSParseTagItem *pTag = pInfo->GetTagObjectFactory(nCurTag);
	pInfo->Push(pTag);
	if (!pTag->StartElement(pInfo, atts)) {
		pInfo->SetError();
	}
}

static void EndElement(void *userData, const XML_Char *name)
{
	ParseInfo *pInfo = (ParseInfo*)userData;
	if (pInfo->IsError()) {
		pInfo->Pop();
		return;
	}
	pInfo->Top()->EndElement(pInfo);
	pInfo->Pop();
}

///////////////////////////////////////
// parser main
///////////////////////////////////////

BOOL TSParser::Parse(LPCTSTR pFileName, VirtualFolderEnumListener *pLsnr)
{
	XML_Parser pParser;
	ParseInfo info;

	if (!info.Init(pLsnr)) return FALSE;

	HANDLE hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return FALSE;

	DWORD nFileSize = GetFileSize(hFile, NULL);

	pParser = XML_ParserCreate(NULL);
	if (pParser == NULL) {
		CloseHandle(hFile);
		return FALSE;
	}

	XML_SetElementHandler(pParser, StartElement, EndElement);
	XML_SetUserData(pParser, &info);

	void *pBuf = XML_GetBuffer(pParser, nFileSize);
	if (pBuf == NULL) {
		CloseHandle(hFile);
		return FALSE;
	}
	DWORD nRead;
	if (!ReadFile(hFile, pBuf, nFileSize, &nRead, NULL)) {
		CloseHandle(hFile);
		return FALSE;
	}

	CloseHandle(hFile);

	if (!XML_ParseBuffer(pParser, nFileSize, TRUE)) {
		const WCHAR *p = XML_ErrorString(XML_GetErrorCode(pParser));
		int ln = XML_GetCurrentLineNumber(pParser);
		int col = XML_GetCurrentColumnNumber(pParser);
		return FALSE;
	}
	XML_ParserFree(pParser);
	return TRUE;
}
