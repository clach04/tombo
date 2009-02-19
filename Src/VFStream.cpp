#include <windows.h>
#include <tchar.h>
#if defined(PLATFORM_WIN32)
#include <wchar.h>
#endif
#include <commctrl.h>
#include "Tombo.h"
#include "Message.h"
#include "UniConv.h"
#include "TString.h"
#include "TomboURI.h"
#include "Repository.h"
#include "File.h"
#include "Property.h"
#include "MemoSelectView.h"
#include "DirectoryScanner.h"
#include "VFStream.h"
#include "SearchEngine.h"
#include "VarBuffer.h"
#include "AutoPtr.h"
#include "URIScanner.h"

#include "resource.h"
#include "DialogTemplate.h"
#include "FilterAddDlg.h"

#define STORE_INIT_SIZE 100
#define STORE_EXTEND_DELTA 50

////////////////////////////////////
// VFNote
////////////////////////////////////

VFNote::~VFNote()
{
	delete [] pTitle;
	delete pURI;
}

BOOL VFNote::Init(const TomboURI *pu, LPCTSTR title)
{
	pURI = new TomboURI(*pu);
	if (pURI == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }

	pTitle = StringDup(title);
	if (pTitle == NULL) return FALSE;

	if (!g_Repository.GetNoteAttribute(pURI, &uLastUpdate, &uCreateDate, &uFileSize)) return FALSE;
	return TRUE;
}

////////////////////////////////////
////////////////////////////////////

VFStream::VFStream() : pNext(NULL)
{
}

VFStream::~VFStream()
{
}

BOOL VFStream::SetNext(VFStream *p)
{
	pNext = p;
	return TRUE;
}

BOOL VFStream::Prepare()
{
	if (pNext) return pNext->Prepare();
	return TRUE;
}

void VFStream::FreeObject()
{
	if (pNext) pNext->FreeObject();
	delete pNext;
	pNext = NULL;
}

BOOL VFStream::PostActivate()
{
	if (pNext) return pNext->PostActivate();
	return TRUE;
}

BOOL VFStream::NeedEncryptedNote()
{
	return FALSE;
}

////////////////////////////////////
// Traverse directory
////////////////////////////////////
// VFDirectoryGenerator's helper class

class VFolderScanner : public URIScanner {
	VFStream *pNext;
	BOOL bCheckEncrypt;

	DWORD nError;

	void InitialScan() { nError = ERROR_SUCCESS; }
	void Node();

public:

	BOOL Init(const TomboURI *pURI, VFStream *pNext, BOOL bCheckEncrypt);
	DWORD GetError() { return nError; }
};

BOOL VFolderScanner::Init(const TomboURI *pURI, VFStream *pN, BOOL bce)
{
	bCheckEncrypt = bce;
	pNext = pN;
	return URIScanner::Init(&g_Repository, pURI, !bCheckEncrypt);
}

void VFolderScanner::Node() 
{
	const TomboURI *pCur = CurrentURI();
	LPCTSTR pTitle = GetTitle();
	
	if (!bCheckEncrypt && g_Repository.IsEncrypted(pCur)) return;

	VFNote *pVF = new VFNote();
	if (pVF == NULL) {
		nError = ERROR_NOT_ENOUGH_MEMORY;
		StopScan();
		return;
	}
	if (!pVF->Init(pCur, pTitle)) {
		nError = GetLastError();
		StopScan();
		delete pVF;
		return;
	}

	// pass to the filter.
	if (!pNext->Store(pVF)) {
		nError = GetLastError();
		StopScan();
		delete pVF;
	}
}

////////////////////////////////////
// VFDirectoryGenerator
////////////////////////////////////

VFDirectoryGenerator::VFDirectoryGenerator() : pURI(NULL)
{
}

VFDirectoryGenerator::~VFDirectoryGenerator()
{
	delete pURI;
}

BOOL VFDirectoryGenerator::Init(LPCTSTR pDirPath, BOOL bCe)
{
	bCheckEncrypt = bCe;
	SetDirPath(pDirPath);
	return TRUE;
}

BOOL VFDirectoryGenerator::Init(const TomboURI *pURI, BOOL bCe)
{
	bCheckEncrypt = bCe;
	return SetURI(pURI);
}

BOOL VFDirectoryGenerator::SetDirPath(LPCTSTR pPath)
{
	delete pURI;
	pURI = new TomboURI();
	if (pURI == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }
	// XXXX get repo name and set it
	if (!pURI->InitByNotePath(TEXT("default"), pPath)) return FALSE;
	return TRUE;
}

BOOL VFDirectoryGenerator::SetURI(const TomboURI *p)
{
	delete pURI;
	pURI = new TomboURI(*p);
	if (pURI == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }
	return TRUE;
}

BOOL VFDirectoryGenerator::Activate()
{
	if (!pNext) return FALSE;

	BOOL bCE;
	if (bCheckEncrypt) {
		bCE = TRUE;
	} else {
		bCE = FALSE;
		VFStream *p = pNext;
		while(p) {
			if (p->NeedEncryptedNote()) {
				bCE = TRUE;
			}
			p = p->GetNext();
		}
	}
	VFolderScanner vfs;
	if (!vfs.Init(pURI, pNext, bCE)) return FALSE;
	return vfs.FullScan();
}

BOOL VFDirectoryGenerator::Store(VFNote *p)
{
	// usually, this member is not called.
	return FALSE;
}

VFStream *VFDirectoryGenerator::Clone(VFStore **ppTail)
{
	VFDirectoryGenerator *p = new VFDirectoryGenerator();
	if (p == NULL) return NULL;
	
	p->bCheckEncrypt = bCheckEncrypt;
	p->pNext = pNext->Clone(ppTail);
	if (!p || p->pNext == NULL) {
		delete p;
		return NULL;
	}
	p->pURI = new TomboURI(*pURI);

	return p;
}

BOOL VFDirectoryGenerator::GenerateXMLOpenTag(File *pFile)
{
	pNext->GenerateXMLOpenTag(pFile);

	TString sDirPath;
	if (!pURI->GetFilePath(&sDirPath)) return FALSE;

	if (!pFile->WriteUnicodeString(L"<src folder=\"")) return FALSE;
	LPWSTR pDirW = ConvTCharToWChar(sDirPath.Get());
	if (!pDirW) return FALSE;
	if (!pFile->WriteUnicodeString(pDirW)) {
		delete [] pDirW;
		return FALSE;
	}
	delete [] pDirW;
	if (!pFile->WriteUnicodeString(L"\"")) return FALSE;
	if (bCheckEncrypt) {
		if (!pFile->WriteUnicodeString(L" checkencrypt='True'")) return FALSE;
	}
	if (!pFile->WriteUnicodeString(L"/>\n")) return FALSE;

	return TRUE;
}

BOOL VFDirectoryGenerator::GenerateXMLCloseTag(File *pFile)
{
	return pNext->GenerateXMLCloseTag(pFile);
}


LPCTSTR VFDirectoryGenerator::GetFilterType()
{
	return NULL;
}

BOOL VFDirectoryGenerator::ToString(TString *p)
{
	return FALSE;
}


BOOL VFDirectoryGenerator::UpdateParamWithDialog(HINSTANCE hInst, HWND hParent)
{
	return FALSE;
}

////////////////////////////////////
// VFStore implimentation
////////////////////////////////////

VFStore::VFStore()
{
}

VFStore::~VFStore()
{
}

BOOL VFStore::Init()
{
	return vNotes.Init(STORE_INIT_SIZE, STORE_EXTEND_DELTA);
}

void VFStore::FreeObject()
{
	/* NOP */
}

BOOL VFStore::Prepare()
{
	if (!vNotes.Clear(TRUE)) return FALSE;
	return TRUE;
}

BOOL VFStore::Store(VFNote *p)
{
	return vNotes.Add(&p);
}

BOOL VFStore::PostActivate()
{
	// TODO: sorting.
	return TRUE;
}

void VFStore::FreeArray()
{
	DWORD n = vNotes.NumItems();
	for (DWORD i = 0; i < n; i++) {
		delete (*vNotes.GetUnit(i));
	}
	vNotes.Clear(FALSE);
}

VFStream *VFStore::Clone(VFStore **ppTail)
{
	VFStore *p = new VFStore();
	if (!p || !p->Init()) return NULL;
	*ppTail = p;
	return p;
}

BOOL VFStore::GenerateXMLOpenTag(File *pFile)
{
	return TRUE;
}

BOOL VFStore::GenerateXMLCloseTag(File *pFile)
{
	return TRUE;
}

LPCTSTR VFStore::GetFilterType()
{
	return NULL;
}

BOOL VFStore::ToString(TString *p)
{
	return FALSE;
}

BOOL VFStore::UpdateParamWithDialog(HINSTANCE hInst, HWND hParent)
{
	return FALSE;
}

////////////////////////////////////
//  VFRegexFilter
////////////////////////////////////

VFRegexFilter::VFRegexFilter() : pRegex(NULL), pPassMgr(NULL)
{
}

VFRegexFilter::~VFRegexFilter()
{
	delete pRegex;
}

BOOL VFRegexFilter::Init(LPCTSTR pPat, BOOL bCase, BOOL bEnc, BOOL bFileName, BOOL bNeg, PasswordManager *pPMgr)
{
	pPattern = new TString();
	if (!pPattern) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}
	pPassMgr = pPMgr;
	return Reset(pPat, bCase, bEnc, bFileName, bNeg);
}

BOOL VFRegexFilter::Reset(LPCTSTR pPat, BOOL bCase, BOOL bEnc, BOOL bFileName, BOOL bNeg)
{
	if (!pPattern->Set(pPat)) return FALSE;

	bCaseSensitive = bCase;
	bFileNameOnly= bFileName;
	bNegate = bNeg;

	pRegex = new SearchEngineA();
	const char *pReason;
	if (!pRegex || !pRegex->Init(g_Property.GetCodePage(), bEnc, bFileName, pPassMgr)) return FALSE;
	if (!pRegex->Prepare(pPat, bCase, &pReason)) return FALSE;
	return TRUE;
}

BOOL VFRegexFilter::Store(VFNote *p)
{
	switch(pRegex->SearchFromURI(p->GetURI())) {
	case SR_NOTFOUND:
		if (bNegate) {
			return pNext->Store(p);
		} else {
			delete p; // when discarding, Store() must delete object p.
			return TRUE;
		}
	case SR_FOUND:
		if (bNegate) {
			delete p; // when discarding, Store() must delete object p.
			return TRUE;
		} else {
			return pNext->Store(p);
		}
	case SR_CANCELED:
	case SR_FAILED:
	default:
		return FALSE;
	}
}

VFStream *VFRegexFilter::Clone(VFStore **ppTail)
{
	VFRegexFilter *p = new VFRegexFilter();
	if (p == NULL) return NULL;

	p->pPattern = new TString();
	p->pPattern->Set(pPattern->Get());

	p->pRegex = pRegex->Clone();
	p->pNext = pNext->Clone(ppTail);

	p->bCaseSensitive = bCaseSensitive;
	p->bFileNameOnly = bFileNameOnly;
	p->bNegate = bNegate;

	if (!p || p->pRegex == NULL || 
		p->pNext == NULL) {
		delete p;
		return NULL;
	}
	return p;
}

BOOL VFRegexFilter::GenerateXMLOpenTag(File *pFile)
{
	if (!pNext->GenerateXMLOpenTag(pFile)) return FALSE;

	if (!pFile->WriteUnicodeString(L"<grep pattern=\"")) return FALSE;
	WString sPatW;
	if (!sPatW.Set(pPattern)) return FALSE;
	if (!pFile->WriteUnicodeString(sPatW.Get())) return FALSE;
	if (!pFile->WriteUnicodeString(L"\"")) return FALSE;

	if (bCaseSensitive) {
		if (!pFile->WriteUnicodeString(L" casesensitive='True'")) return FALSE;
	}
	if (bFileNameOnly) {
		if (!pFile->WriteUnicodeString(L" filenameonly='True'")) return FALSE;
	}
	if (bNegate) {
		if (!pFile->WriteUnicodeString(L" not='True'")) return FALSE;
	}

	if (pRegex->IsSearchEncryptMemo()) {
		if (!pFile->WriteUnicodeString(L" checkencrypt='True'")) return FALSE;
	}

	if (!pFile->WriteUnicodeString(L">\n")) return FALSE;

	return TRUE;
}

BOOL VFRegexFilter::GenerateXMLCloseTag(File *pFile)
{
	if (!pFile->WriteUnicodeString(L"</grep>\n")) return FALSE;
	if (!pNext->GenerateXMLCloseTag(pFile)) return FALSE;
	return TRUE;
}

LPCTSTR VFRegexFilter::GetFilterType()
{
	return MSG_STREAM_NAME_REGEXP;
}

BOOL VFRegexFilter::ToString(TString *p)
{
	return p->Set(pPattern->Get());
}

BOOL VFRegexFilter::UpdateParamWithDialog(HINSTANCE hInst, HWND hParent)
{
	RegexFilterAddDlg ad;
	if (!ad.Init(pPattern->Get(), bCaseSensitive, pRegex->IsSearchEncryptMemo(), bFileNameOnly, bNegate)) return FALSE;
	if (ad.Popup(hInst, hParent) == IDOK) {
		Reset(ad.GetMatchString()->Get(), 
				ad.IsCaseSensitive(), ad.IsCheckEncrypt(),
				ad.IsCheckFileName(), ad.IsNegate());
		return TRUE;
	}
	return FALSE;
}

BOOL VFRegexFilter::NeedEncryptedNote()
{
	return pRegex->IsSearchEncryptMemo();
}

////////////////////////////////////
//  VFLimitFilter
////////////////////////////////////

VFLimitFilter::VFLimitFilter() : nLimit(0)
{
}

VFLimitFilter::~VFLimitFilter()
{
}

BOOL VFLimitFilter::Init(DWORD n)
{
	nLimit = n;
	return TRUE;
}

BOOL VFLimitFilter::Prepare()
{
	nCount = 0;
	return VFStream::Prepare();
}

BOOL VFLimitFilter::Store(VFNote *p)
{
	if (nCount >= nLimit) {
		delete p;
		return TRUE;
	}
	nCount++;
	return pNext->Store(p);
}

VFStream *VFLimitFilter::Clone(VFStore **ppTail)
{
	VFLimitFilter *p = new VFLimitFilter();
	if (!p || !p->Init(nLimit)) {
		delete p;
		return NULL;
	}
	p->pNext = pNext->Clone(ppTail);
	if (!p || p->pNext == NULL) {
		delete p;
		return NULL;
	}
	return p;
}


BOOL VFLimitFilter::GenerateXMLOpenTag(File *pFile)
{
	if (!pNext->GenerateXMLOpenTag(pFile)) return FALSE;

	if (!pFile->WriteUnicodeString(L"<limit number=\"")) return FALSE;
	WCHAR buf[32];
	swprintf(buf, L"%d", nLimit);
	if (!pFile->WriteUnicodeString(buf)) return FALSE;

	if (!pFile->WriteUnicodeString(L"\">\n")) return FALSE;
	return TRUE;
}

BOOL VFLimitFilter::GenerateXMLCloseTag(File *pFile)
{
	if (!pFile->WriteUnicodeString(L"</limit>\n")) return FALSE;
	if (!pNext->GenerateXMLCloseTag(pFile)) return FALSE;
	return TRUE;
}

LPCTSTR VFLimitFilter::GetFilterType()
{
	return MSG_STREAM_NAME_LIMIT;
}

BOOL VFLimitFilter::ToString(TString *p)
{
	if (!p->Alloc(_tcslen(MSG_STREAM_VALUE_LIMIT) + 10)) return FALSE;
	wsprintf(p->Get(), MSG_STREAM_VALUE_LIMIT, nLimit);
	return TRUE;
}

BOOL VFLimitFilter::UpdateParamWithDialog(HINSTANCE hInst, HWND hParent)
{
	LimitFilterAddDlg ad;
	if (!ad.Init(nLimit)) return FALSE;
	if (ad.Popup(hInst, hParent) == IDOK) {
		nLimit = ad.GetLimit();
		return TRUE;
	}
	return FALSE;
}

////////////////////////////////////
//  VFTimestampFilter
////////////////////////////////////

VFTimestampFilter::VFTimestampFilter()
{
}

VFTimestampFilter::~VFTimestampFilter()
{
}

BOOL VFTimestampFilter::Reset(DWORD nDelta, BOOL bNew)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);

	nDeltaDays = nDelta;

	uBase = ((UINT64)ft.dwHighDateTime << 32) | (UINT64)ft.dwLowDateTime;
	UINT64 d = 0xc92a69c000;	// 1 day
	uBase -= d * nDelta;
	bNewer = bNew;

	return TRUE;
}

BOOL VFTimestampFilter::Store(VFNote *pNote)
{
	if (bNewer && pNote->GetLastUpdate() > uBase || (!bNewer && pNote->GetLastUpdate() < uBase)) {
		return pNext->Store(pNote);
	} else {
		delete pNote;
		return TRUE;
	}
}

VFStream *VFTimestampFilter::Clone(VFStore **ppTail)
{
	VFTimestampFilter *p = new VFTimestampFilter();
	if (!p) return NULL;

	p->bNewer = bNewer;
	p->uBase = uBase;
	p->nDeltaDays = nDeltaDays;
	p->pNext = pNext->Clone(ppTail);
	if (!p || p->pNext == NULL) {
		delete p;
		return NULL;
	}
	return p;
}

BOOL VFTimestampFilter::GenerateXMLOpenTag(File *pFile)
{
	if (!pNext->GenerateXMLOpenTag(pFile)) return FALSE;

	if (!pFile->WriteUnicodeString(L"<timestamp days=\"")) return FALSE;
	WCHAR buf[32];
	swprintf(buf, L"%d", nDeltaDays);
	if (!pFile->WriteUnicodeString(buf)) return FALSE;
	if (!pFile->WriteUnicodeString(L"\"")) return FALSE;

	if (bNewer) {
		if (!pFile->WriteUnicodeString(L" newer='True'")) return FALSE;
	} else {
		if (!pFile->WriteUnicodeString(L" older='True'")) return FALSE;
	}

	if (!pFile->WriteUnicodeString(L">\n")) return FALSE;
	return TRUE;
}

BOOL VFTimestampFilter::GenerateXMLCloseTag(File *pFile)
{
	if (!pFile->WriteUnicodeString(L"</timestamp>\n")) return FALSE;
	if (!pNext->GenerateXMLCloseTag(pFile)) return FALSE;
	return TRUE;
}

LPCTSTR VFTimestampFilter::GetFilterType()
{
	return MSG_STREAM_NAME_TIMESTAMP;
}

BOOL VFTimestampFilter::ToString(TString *p)
{
	LPCTSTR pTmpl;
	if (bNewer) {
		pTmpl = MSG_STREAM_VALUE_TIMESTAMP_NEWER;
	} else {
		pTmpl = MSG_STREAM_VALUE_TIMESTAMP_OLDER;
	}
	if (!p->Alloc(_tcslen(pTmpl) + 10)) return FALSE;
	wsprintf(p->Get(), pTmpl, nDeltaDays);
	return TRUE;
}

BOOL VFTimestampFilter::UpdateParamWithDialog(HINSTANCE hInst, HWND hParent)
{
	TimestampFilterAddDlg ad;
	if (!ad.Init(nDeltaDays, bNewer)) return FALSE;
	if (ad.Popup(hInst, hParent) == IDOK) {
		return Reset(ad.GetDeltaDay(), ad.IsNewer());
	}
	return FALSE;
}

////////////////////////////////////
//  VFSortFilter
////////////////////////////////////

VFSortFilter::VFSortFilter()
{
}

VFSortFilter::~VFSortFilter()
{
}

BOOL VFSortFilter::Init(SortFuncType sf)
{
	sfType = sf;
	return vNotes.Init(STORE_INIT_SIZE, STORE_EXTEND_DELTA);
}

BOOL VFSortFilter::Prepare()
{
	vNotes.Clear(TRUE);
	return VFStream::Prepare();
}

BOOL VFSortFilter::Store(VFNote *p)
{
	if (!vNotes.Add(&p)) return FALSE;
	return TRUE;
}

extern "C" {
typedef int SortFunc(const void *e1, const void *e2);
};

extern "C" static int SortNotes_FileNameAsc(const void *e1, const void *e2)
{
	VFNote *p1 = *(VFNote**)e1;
	VFNote *p2 = *(VFNote**)e2;
	return _tcsicmp(p1->GetTitle(), p2->GetTitle());
}

extern "C" static int SortNotes_FileNameDesc(const void *e1, const void *e2)
{
	VFNote *p1 = *(VFNote**)e1;
	VFNote *p2 = *(VFNote**)e2;
	return _tcsicmp(p2->GetTitle(), p1->GetTitle());
}

extern "C" static int SortNotes_LastUpdateOlder(const void *e1, const void *e2)
{
	VFNote *p1 = *(VFNote**)e1;
	VFNote *p2 = *(VFNote**)e2;
	if (p1->GetLastUpdate() == p2->GetLastUpdate()) return 0;
	if (p1->GetLastUpdate() > p2->GetLastUpdate()) return 1;
	else return -1;
}

extern "C" static int SortNotes_LastUpdateNewer(const void *e1, const void *e2)
{
	VFNote *p1 = *(VFNote**)e1;
	VFNote *p2 = *(VFNote**)e2;
	if (p2->GetLastUpdate() == p1->GetLastUpdate()) return 0;
	if (p2->GetLastUpdate() > p1->GetLastUpdate()) return 1;
	else return -1;
}

extern "C" static int SortNotes_CreateDateOlder(const void *e1, const void *e2)
{
	VFNote *p1 = *(VFNote**)e1;
	VFNote *p2 = *(VFNote**)e2;
	if (p1->GetCreateDate() == p2->GetCreateDate()) return 0;
	if (p1->GetCreateDate() > p2->GetCreateDate()) return 1;
	else return -1;
}

extern "C" static int SortNotes_CreateDateNewer(const void *e1, const void *e2)
{
	VFNote *p1 = *(VFNote**)e1;
	VFNote *p2 = *(VFNote**)e2;
	if (p2->GetCreateDate() == p1->GetCreateDate()) return 0;
	if (p2->GetCreateDate() > p1->GetCreateDate()) return 1;
	else return -1;
}

extern "C" static int SortNotes_FileSizeAsc(const void *e1, const void *e2)
{
	VFNote *p1 = *(VFNote**)e1;
	VFNote *p2 = *(VFNote**)e2;

	if (p1->GetFileSize() == p2->GetFileSize()) return 0;
	if (p1->GetFileSize() > p2->GetFileSize()) return 1;
	else return -1;
}

extern "C" static int SortNotes_FileSizeDsc(const void *e1, const void *e2)
{
	VFNote *p1 = *(VFNote**)e1;
	VFNote *p2 = *(VFNote**)e2;

	if (p2->GetFileSize() == p1->GetFileSize()) return 0;
	if (p2->GetFileSize() > p1->GetFileSize()) return 1;
	else return -1;
}

BOOL VFSortFilter::PostActivate()
{
	VFNote *p;
	DWORD n = vNotes.NumItems();

	SortFunc *pFunc;
	switch(sfType) {
	case SortFunc_FileNameAsc:
		pFunc = SortNotes_FileNameAsc;
		break;
	case SortFunc_FileNameDsc:
		pFunc = SortNotes_FileNameDesc;
		break;
	case SortFunc_LastUpdateAsc:
		pFunc = SortNotes_LastUpdateOlder;
		break;
	case SortFunc_LastUpdateDsc:
		pFunc = SortNotes_LastUpdateNewer;
		break;
	case SortFunc_CreateDateAsc:
		pFunc = SortNotes_CreateDateOlder;
		break;
	case SortFunc_CreateDateDsc:
		pFunc = SortNotes_CreateDateNewer;
		break;
	case SortFunc_FileSizeAsc:
		pFunc = SortNotes_FileSizeAsc;
		break;
	case SortFunc_FileSizeDsc:
		pFunc = SortNotes_FileSizeDsc;
		break;
	default:
		return FALSE;
	}
	qsort((LPBYTE)vNotes.GetBuf(), n, sizeof(VFNote*), pFunc);
	for (DWORD i = 0; i < n; i++) {
		p = *vNotes.GetUnit(i);
		if (!pNext->Store(p)) return FALSE;
	}
	return VFStream::PostActivate();
}

VFStream *VFSortFilter::Clone(VFStore **ppTail)
{
	VFSortFilter *p = new VFSortFilter();
	if (!p || !p->Init(sfType)) return NULL;

	p->pNext = pNext->Clone(ppTail);
	if (!p || p->pNext == NULL) {
		delete p;
		return NULL;
	}
	return p;
}

BOOL VFSortFilter::GenerateXMLOpenTag(File *pFile)
{
	if (!pNext->GenerateXMLOpenTag(pFile)) return FALSE;

	if (!pFile->WriteUnicodeString(L"<sort func=\"")) return FALSE;
	LPCWSTR pType;
	switch(sfType) {
	case SortFunc_FileNameAsc:
		pType = L"filename_asc";
		break;
	case SortFunc_FileNameDsc:
		pType = L"filename_dsc";
		break;
	case SortFunc_LastUpdateAsc:
		pType = L"lastupdate_asc";
		break;
	case SortFunc_LastUpdateDsc:
		pType = L"lastupdate_dsc";
		break;
	case SortFunc_CreateDateAsc:
		pType = L"createdate_asc";
		break;
	case SortFunc_CreateDateDsc:
		pType = L"createdate_dsc";
		break;
	case SortFunc_FileSizeAsc:
		pType = L"filesize_asc";
		break;
	case SortFunc_FileSizeDsc:
		pType = L"filesize_dsc";
		break;
	default:
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}
	if (!pFile->WriteUnicodeString(pType)) return FALSE;
	if (!pFile->WriteUnicodeString(L"\">\n")) return FALSE;
	return TRUE;
}

BOOL VFSortFilter::GenerateXMLCloseTag(File *pFile)
{
	if (!pFile->WriteUnicodeString(L"</sort>\n")) return FALSE;
	if (!pNext->GenerateXMLCloseTag(pFile)) return FALSE;
	return TRUE;
}

LPCTSTR VFSortFilter::GetFilterType()
{
	return MSG_STREAM_NAME_SORT;
}

BOOL VFSortFilter::ToString(TString *p)
{
	LPCTSTR pTmpl;
	switch(sfType) {
	case SortFunc_FileNameAsc:
		pTmpl = MSG_STREAM_VALUE_SORT_FNAME_ASC;
		break;
	case SortFunc_FileNameDsc:
		pTmpl = MSG_STREAM_VALUE_SORT_FNAME_DSC;
		break;
	case SortFunc_LastUpdateAsc:
		pTmpl = MSG_STREAM_VALUE_SORT_LASTUPD_ASC;
		break;
	case SortFunc_LastUpdateDsc:
		pTmpl = MSG_STREAM_VALUE_SORT_LASTUPD_DSC;
		break;
	case SortFunc_CreateDateAsc:
		pTmpl = MSG_STREAM_VALUE_SORT_CREATE_ASC;
		break;
	case SortFunc_CreateDateDsc:
		pTmpl = MSG_STREAM_VALUE_SORT_CREATE_DSC;
		break;
	case SortFunc_FileSizeAsc:
		pTmpl = MSG_STREAM_VALUE_SORT_FILESIZE_ASC;
		break;
	case SortFunc_FileSizeDsc:
		pTmpl = MSG_STREAM_VALUE_SORT_FILESIZE_DSC;
		break;
	default:
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}
	return p->Set(pTmpl);
}

BOOL VFSortFilter::UpdateParamWithDialog(HINSTANCE hInst, HWND hParent)
{
	SortFilterAddDlg ad;
	if (!ad.Init(sfType)) return FALSE;
	if (ad.Popup(hInst, hParent) == IDOK) {
		sfType = ad.GetType();
		return TRUE;
	}
	return FALSE;
}
