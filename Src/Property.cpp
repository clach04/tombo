#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#if defined(PLATFORM_WIN32)
#include <stdio.h>
#endif
#if defined(PLATFORM_BE500)
#include <GetDisk.h>
#endif
#include <expat.h>

#include "Tombo.h"
#include "Property.h"
#include "resource.h"
#include "FileSelector.h"
#include "UniConv.h"
#include "PasswordManager.h"
#include "Message.h"
#include "DialogTemplate.h"
#include "TString.h"
#include "PropertyPage.h"
#include "TomboPropertyTab.h"
#include "File.h"
#include "AutoPtr.h"
#include "List.h"
#include "TomboURI.h"
#include "Repository.h"
#include "VarBuffer.h"
#include "RepositoryImpl.h"

//////////////////////////////////////////
// Attribute definitions

#define TOMBO_PROP_VERSION "1.16"
#define PROP_FILE_NAME TEXT("tomboprops.xml")
#define PROP_TMP_FILE_NAME TEXT("tomboprops.xml~")

#define TOMBO_MAIN_KEY TEXT("Software\\flatfish\\Tombo")
#define TOPDIR_ATTR_NAME TEXT("TopDir")
#define USEYAE_ATTR_NAME TEXT("UseYAEdit")
#define BOOKMARK_ATTR_NAME TEXT("BookMark")
#define SEARCHHIST_ATTR_NAME TEXT("SearchHistory")
#define TOPDIRHIST_ATTR_NAME TEXT("TopDirHistory")
#define REBARHIST_ATTR_NAME TEXT("RebarPos")

//////////////////////////////////////////
// property pages

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
#define PROPTAB_PAGES 10
#else
#define PROPTAB_PAGES 8
#endif

//////////////////////////////////////////
// default values

#if defined(PLATFORM_WIN32)
#define MEMO_TOP_DIR TEXT("c:\\My Documents\\Pocket_PC My Documents\\TomboRoot")
#else
#define MEMO_TOP_DIR TEXT("\\My Documents\\TomboRoot")
#endif

#if defined(PLATFORM_BE500)
#define TOMBO_ROOT_SUFFIX TEXT("\\TomboRoot")
#endif

#define DEFAULTDATEFORMAT1 TEXT("%y/%M/%d")
#define DEFAULTDATEFORMAT2 TEXT("%h:%m:%s")

//////////////////////////////////////////
class RepositoryImpl;

static HKEY GetTomboRootKey();

static BOOL SetSZToReg(HKEY hKey, LPCTSTR pAttr, LPCTSTR pValue);
static BOOL SetDWORDToReg(HKEY hKey, LPCTSTR pAttr, DWORD nValue);
static BOOL SetMultiSZToReg(HKEY hKey, LPCTSTR pAttr, LPCTSTR pValue, DWORD nSize);

static DWORD GetDWORDFromReg(HKEY hKey, LPCTSTR pAttr, DWORD nDefault);
static LPTSTR GetMultiSZFromReg(HKEY hKey, LPCTSTR pAttr);
static BOOL GetSZFromReg(HKEY hKey, LPCTSTR pAttr, LPTSTR pBuf, DWORD nBuf, LPCTSTR pDefault);
static LPTSTR GetAllocSZFromReg(HKEY hKey, LPCTSTR pAttr);

static DWORD CountMultiSZLen(LPCTSTR pData);

static BOOL MakeFont(HFONT *phFont, LPCTSTR pName, DWORD nSize, BYTE bQuality);

//////////////////////////////////////////
// property definitions
//////////////////////////////////////////

struct PropListNum {
	DWORD nPropId;
	LPCTSTR pAttrName;
	DWORD nDefault;
} propListNum[] = {
	{ PROP_N_PASSTIMEOUT,				TEXT("PassTimeOut"),				5 },
	{ PROP_N_DETAILSVIEW_KEEPCARET,		TEXT("KeepCaret"),					FALSE },
	{ PROP_NDETAILSVIEW_TABSTOP,		TEXT("Tabstop"),					8 },
	{ PROP_N_SELECTVIEW_FONTSIZE,		TEXT("SelectViewFontSize"),			0xFFFFFFFF },
	{ PROP_N_SELECTVIEW_FONTQUALITY,	TEXT("SelectViewFontQuality"),		DEFAULT_QUALITY },
	{ PROP_N_DETAILSVIEW_FONTSIZE,		TEXT("DetailsViewFontSize"),		0xFFFFFFFF },
	{ PROP_N_DETAILSVIEW_FONTQUALITY,	TEXT("DetailsViewFontQuality"),		DEFAULT_QUALITY },
	{ PROP_N_AUTOSELECT_MODE,			TEXT("AutoSelectMode"),				TRUE },
	{ PROP_N_SINGLECLICK_MODE,			TEXT("SingleClickMode"),			TRUE },
	{ PROP_N_USE_TWO_PANE,				TEXT("UseTwoPane"),					TRUE },
	{ PROP_N_KEEP_TITLE,				TEXT("KeepTitle"),					FALSE },
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WIN32) || defined(PLATFORM_WM5)
	{ PROP_N_SWITCH_WINDOW_TITLE,		TEXT("SwitchWindowTitle"),			TRUE },
#endif
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	{ PROP_N_APP_BUTTON1,				TEXT("AppButton1"),					APPBUTTON_ACTION_DISABLE },
	{ PROP_N_APP_BUTTON2,				TEXT("AppButton2"),					APPBUTTON_ACTION_DISABLE },
	{ PROP_N_APP_BUTTON3,				TEXT("AppButton3"),					APPBUTTON_ACTION_DISABLE },
	{ PROP_N_APP_BUTTON4,				TEXT("AppButton4"),					APPBUTTON_ACTION_DISABLE },
	{ PROP_N_APP_BUTTON5,				TEXT("AppButton5"),					APPBUTTON_ACTION_DISABLE },
	{ PROP_N_SIPSIZE_DELTA,				TEXT("SipSizeDelta"),				0},
#endif
	{ PROP_N_CODEPAGE,					TEXT("CodePage"),					0 },
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	{ PROP_N_DISABLEEXTRAACTIONBUTTON,	TEXT("DisableExtraActionButton"),	0},
#endif
#if defined(PLATFORM_HPC) || defined(PLATFORM_WIN32)
	{ PROP_N_HIDESTATUSBAR,				TEXT("HideStatusBar"),				0 },
#endif
#if defined(PLATFORM_WIN32)
	{ PROP_N_STAYTOPMOST,				TEXT("StayTopMost"),				0 },
	{ PROP_N_HIDEREBAR,					TEXT("HideRebar"),					0 },
#endif
	{ PROP_N_WRAPTEXT,					TEXT("WrapText"),					1 },
	{ PROP_N_OPENREADONLY,				TEXT("OpenReadOnly"),				FALSE },
	{ PROP_N_DISABLESAVEDLG,			TEXT("DisableSaveDlg"),				FALSE },
	{ PROP_N_USEASSOC,					TEXT("UseSoftwareAssoc"),			FALSE },
	{ PROP_N_SAFEFILENAME,				TEXT("UseSafeFileName"),			FALSE },
#if (defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)) && defined(FOR_VGA)
	{ PROP_N_TOMBO_WINSIZE3,			TEXT("WinSize3"),					0xFFFF },
#endif
	{ PROP_N_KEEP_LAST_OPEN,			TEXT("UseLastOpenNote"),			FALSE },
	{ PROP_N_USE_YAE,					TEXT("UseYAE"),						FALSE },
	{ PROP_N_DISABLE_YAE,				TEXT("DisableYAE"),					FALSE },
	{ 0xFFFFFFFF,						NULL,								NULL},
};

struct PropListStr {
	DWORD nPropId;
	LPCTSTR pAttrName;
	LPCTSTR pDefault;
} propListStr[] = {
	{ PROP_S_TOPDIR,					TOPDIR_ATTR_NAME,					NULL },	
	{ PROP_S_SELECTVIEW_FONTNAME,		TEXT("SelectViewFontName"),			TEXT("") },
	{ PROP_S_DETAILSVIEW_FONTNAME,		TEXT("DetailsViewFontName"),		TEXT("") },
	{ PROP_S_DETAILSVIEW_DATEFORMAT1,	TEXT("DateFormat1"),				DEFAULTDATEFORMAT1 },
	{ PROP_S_DETAILSVIEW_DATEFORMAT2,	TEXT("DateFormat2"),				DEFAULTDATEFORMAT2 },
	{ PROP_S_DEFAULTNOTE,				TEXT("DefaultNote"),				TEXT("") },
	{ PROP_S_EXTAPP1,					TEXT("ExtApp1"),					TEXT("") },
	{ PROP_S_EXTAPP2,					TEXT("ExtApp2"),					TEXT("") },
	{ PROP_S_WINSIZE,					TEXT("WinSize2"),					NULL }, 
	{ PROP_S_LAST_OPEN_URI,				TEXT("LastOpenURI"),				TEXT("") },
	{ 0xFFFFFFFF,						NULL,								NULL },
};

//////////////////////////////////////////
// get message string
//////////////////////////////////////////

LPCTSTR GetString(UINT nID)
{
	static TCHAR buf[MESSAGE_MAX_SIZE];
	LoadString(g_hInstance, nID, buf, MESSAGE_MAX_SIZE);
	return buf;
}

//////////////////////////////////////////
// ctor
//////////////////////////////////////////

Property::Property() : pCmdlineAssignedTopDir(NULL), pBookMark(NULL), pSearchHistory(NULL), pTopDirHistory(NULL), 
	pDefaultRep(NULL), pRepos(NULL), nNumRepos(0)
#if defined(PLATFORM_HPC)
	,pCmdBarInfo(NULL)
#endif
{
	bLoad = FALSE;
	bNeedAsk = TRUE;

	for (DWORD i = 0; i < NUM_PROPS_STR; i++) {
		pPropsStr[i] = NULL;
	}

	SetTopDir(TEXT(""));
	SetDefaultNote(TEXT(""));
}

Property::~Property()
{
	DWORD i;
	for (i = 0; i < NUM_PROPS_STR; i++) {
		delete [] pPropsStr[i];
	}

	for (i = 0; i < nNumRepos; i++) {
		delete pRepos[i];
	}
	delete [] pRepos;

	delete [] pCmdlineAssignedTopDir;
	delete [] pBookMark;
	delete [] pSearchHistory;
	delete [] pTopDirHistory;
#if defined(PLATFORM_HPC)
	delete [] pCmdBarInfo;
#endif
}

//////////////////////////////////////////
// 
//////////////////////////////////////////

BOOL Property::SetStringProperty(DWORD nPropId, LPCTSTR pValue)
{
	delete[] pPropsStr[nPropId];
	if (pValue == NULL) {
		pPropsStr[nPropId] = NULL;
		return TRUE;
	}
	if ((pPropsStr[nPropId] = StringDup(pValue)) == NULL) return FALSE;
	return TRUE;
}

//////////////////////////////////////////
// topdir
//////////////////////////////////////////

LPCTSTR Property::GetTomboRoot()
{
	if (pCmdlineAssignedTopDir != NULL && _tcslen(pCmdlineAssignedTopDir) > 0) {
		return pCmdlineAssignedTopDir;
	}
	return GetTopDir();
}

//////////////////////////////////////////
// font
//////////////////////////////////////////

HFONT Property::SelectViewFont()
{
	HFONT hFont = NULL;
	MakeFont(&hFont, GetSelectViewFontName(), GetSelectViewFontSize(), (BYTE)GetSelectViewFontQuality());
	return hFont;
}

HFONT Property::DetailsViewFont()
{
	HFONT hFont = NULL;
	MakeFont(&hFont, GetDetailsViewFontName(), GetDetailsViewFontSize(), (BYTE)GetDetailsViewFontQuality());
	return hFont;
}

//////////////////////////////////////////
// Popup property dialog
//////////////////////////////////////////

DWORD Property::Popup(HINSTANCE hInst, HWND hWnd, const TomboURI *pCurrentSelectedURI)
{
	TString sSelPath;
	if (pCurrentSelectedURI == NULL) {
		sSelPath.Set(TEXT(""));
	} else {
		sSelPath.Set(pCurrentSelectedURI->GetFullURI());
	}

	PropertyTab *pages[PROPTAB_PAGES];
	FolderTab pgFolder(this);
	DefaultNoteTab pgDefNote(this, sSelPath.Get());
	PassTimeoutTab pgTimeout(this);
	FontTab pgFont(this);
	DateFormatTab pgDate(this);
	KeepCaretTab pgKeepCaret(this);
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	AppButtonTab pgAppButton(this);
	SipTab pgSip(this);
#endif
	CodepageTab pgCodepage(this);
#if !defined(PLATFORM_PSPC)
	ExtAppTab pgExtApp(this);
#endif

	DWORD n = 0;

	pages[n++] = &pgFolder;
	pages[n++] = &pgDefNote;
	pages[n++] = &pgTimeout;
	pages[n++] = &pgFont;
	pages[n++] = &pgDate;
	pages[n++] = &pgKeepCaret;
#if !defined(PLATFORM_PSPC)
	pages[n++] = &pgExtApp;
#endif
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	pages[n++] = &pgAppButton;
	pages[n++] = &pgSip;
#endif
	pages[n++] = &pgCodepage;

	PropertyPage pp;
	if (pp.Popup(hInst, hWnd, pages, n, MSG_PROPTAB_TITLE, MAKEINTRESOURCE(IDI_TOMBO)) == IDOK) {
		////////////////////////////////////
		// sync Topdir <-> rep array settings
		// XXXX: this code are temporary till changing property dialog
		LPCTSTR pTopDir = GetTopDir();
		for (DWORD i = 0; i < GetNumSubRepository(); i++) {
			RepositoryImpl *pImpl = pRepos[i];
			if (pImpl->GetRepositoryType() == TOMBO_REPO_SUBREPO_TYPE_LOCALFILE && _tcscmp(pImpl->GetRepositoryName(), TEXT("default")) == 0) {
				LocalFileRepository *pDefaultRep = (LocalFileRepository*)pImpl;
				pDefaultRep->SetTopDir(pTopDir);
			}
		}
		////////////////////////////////////

		if (!Save()) {
			MessageBox(NULL, MSG_SAVE_DATA_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
		}
		return IDOK;
	}
	return IDCANCEL;
}

//////////////////////////////////////////
// load properties
//////////////////////////////////////////

void GetNameAndValue(const XML_Char **atts, LPTSTR *ppKey, LPTSTR *ppValue)
{
	*ppKey = NULL;
	*ppValue = NULL;

	int i = 0;
	while (atts[i]) {
		if (wcscmp((LPCWSTR)atts[i], L"name") == 0) {
			*ppKey = ConvWCharToTChar((LPCWSTR)atts[i + 1]);
		} else if (wcscmp((LPCWSTR)atts[i], L"value") == 0) {
			*ppValue = ConvWCharToTChar((LPCWSTR)atts[i + 1]);
		}
		i+= 2;
	}
}

PropListNum *SerachNumAttrName(LPTSTR pAttr)
{
	PropListNum *p = propListNum;
	while (p->nPropId != 0xFFFFFFFF) {
		if (_tcscmp(pAttr, p->pAttrName) == 0) {
			return p;
		}
		p++;
	}
	return NULL;
}

PropListStr *SerachStrAttrName(LPTSTR pAttr)
{
	PropListStr *p = propListStr;
	while (p->nPropId != 0xFFFFFFFF) {
		if (_tcscmp(pAttr, p->pAttrName) == 0) {
			return p;
		}
		p++;
	}
	return NULL;
}

struct PropFileParseInfo {
	Property *pProperty;

	LPTSTR pMultiName;
	List lMultiItem;

	BOOL bInTomboRoot;

	TVector<RepositoryImpl*> vSubRepos;
	LocalFileRepository *pDefaultRep;

	PropFileParseInfo() : pMultiName(NULL), bInTomboRoot(FALSE), pDefaultRep(NULL) {}
	~PropFileParseInfo() { delete[] pMultiName; }

	void SetMultiName(LPTSTR p) { delete[] pMultiName; pMultiName = p; }
};

static void StartElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
	PropFileParseInfo *pParseInfo = (PropFileParseInfo*)userData;
	Property *pProperty = pParseInfo->pProperty;

	LPCWSTR pName = (LPCWSTR)name;

	LPTSTR pKey;
	LPTSTR pValue;

	if (wcscmp(pName, L"tomboroot") == 0) {
		pParseInfo->bInTomboRoot = TRUE;
	} else if (pParseInfo->bInTomboRoot) {
		RepositoryImpl *pImpl = Repository::CreateSubRepo((LPCWSTR)name, (const WCHAR **)atts);
		if (pImpl != NULL) {
			pParseInfo->vSubRepos.Add(&pImpl);
		}
		// XXX: in current version, default repository assumed as LocalFileRepository.
		if (_tcscmp(TEXT("default"), pImpl->GetRepositoryName()) == 0) {
			pParseInfo->pDefaultRep = (LocalFileRepository*)pImpl;
		}
	} else if (wcscmp(pName, L"tomboprop") == 0) {
		// XXXX: NOP in current version
		// for future version, version check will be added.
	} else if (wcscmp(pName, L"num") == 0) {
		GetNameAndValue(atts, &pKey, &pValue);
		PropListNum *p = SerachNumAttrName(pKey);
		if (p != NULL) {
			DWORD nValue = _ttol(pValue);
			pProperty->SetNumberPropertyById(p->nPropId, nValue);
		}
		delete [] pKey;
		delete [] pValue;
	} else if (wcscmp(pName, L"str") == 0) {
		GetNameAndValue(atts, &pKey, &pValue);
		PropListStr *p = SerachStrAttrName(pKey);
		if (p != NULL) {
			pProperty->SetStringPropertyWithBuffer(p->nPropId, pValue);
			// buffer is managed under Property, so do not delete[] pValue;
		} else {
			delete [] pValue;
		}
		delete [] pKey;
	} else if (wcscmp(pName, L"multistr") == 0) {
		if (wcscmp((LPCWSTR)atts[0], L"name") == 0) {
			pParseInfo->SetMultiName(ConvWCharToTChar((LPCWSTR)atts[1]));
		}
	} else if (wcscmp(pName, L"item") == 0) {
		if (wcscmp((LPCWSTR)atts[0], L"value") == 0) {
			pParseInfo->lMultiItem.Add(ConvWCharToTChar((LPCWSTR)atts[1]));
		}
	}
#if defined(PLATFORM_HPC)
	else if (wcscmp(pName, L"rebar") == 0) {
		GetNameAndValue(atts, &pKey, &pValue);
		LPCOMMANDBANDSRESTOREINFO pCmdBarInfo = new COMMANDBANDSRESTOREINFO[NUM_COMMANDBAR];
		memset(pCmdBarInfo, 0, sizeof(COMMANDBANDSRESTOREINFO) * NUM_COMMANDBAR);
		DWORD wID1, fStyle1, cxRestored1, fMaximized1;
		DWORD wID2, fStyle2, cxRestored2, fMaximized2;

		if (swscanf(pValue, TEXT("%d,%d,%d,%d,%d,%d,%d,%d"),
			&wID1, &fStyle1, &cxRestored1, &fMaximized1,
			&wID2, &fStyle2, &cxRestored2, &fMaximized2) != 8) {
			// TODO: set default
			MessageBox(NULL, TEXT("NOT YET"), TEXT("DEBUG"), MB_OK);
		} else {
			pCmdBarInfo[0].cbSize = pCmdBarInfo[1].cbSize = sizeof(COMMANDBANDSRESTOREINFO);
			pCmdBarInfo[0].wID = (UINT)wID1;
			pCmdBarInfo[1].wID = (UINT)wID2;
			pCmdBarInfo[0].fStyle = (UINT)fStyle1;
			pCmdBarInfo[1].fStyle = (UINT)fStyle2;
			pCmdBarInfo[0].cxRestored = (UINT)cxRestored1;
			pCmdBarInfo[1].cxRestored = (UINT)cxRestored2;
			pCmdBarInfo[0].fMaximized = (BOOL)fMaximized1;
			pCmdBarInfo[1].fMaximized = (BOOL)fMaximized2;
			pProperty->SetCommandbarInfoWithBuffer(pCmdBarInfo);
		}
	}
#endif
}

static void EndElement(void *userData, const XML_Char *name)
{
	PropFileParseInfo *pParseInfo = (PropFileParseInfo*)userData;

	if (wcscmp((LPCWSTR)name, L"tomboroot") == 0) {
		pParseInfo->bInTomboRoot = FALSE;
	} else if (wcscmp((LPCWSTR)name, L"multistr") == 0) {
		List *pList = &(pParseInfo->lMultiItem);

		DWORD n = 0;
		DWORD i = 0;
		HANDLE h = pList->First();
		while (h) {
			LPTSTR p = (LPTSTR)pList->Value(h);
			n += _tcslen(p) + 1;
			i++;
			h = pList->Next(h);
		}
		if (i == 0) return;

		LPTSTR pValue = new TCHAR[n + 1];
		LPTSTR p = pValue;
		h = pList->First();
		while (h) {
			LPTSTR item = (LPTSTR)pList->Value(h);
			_tcscpy(p, item);
			p += _tcslen(item) + 1;

			delete[] item;
			h = pList->Next(h);
		}
		*p = TEXT('\0');

		pList->Clear();

		if (_tcscmp(pParseInfo->pMultiName, BOOKMARK_ATTR_NAME) == 0) {
			pParseInfo->pProperty->SetBookMark(pValue);
			delete[] pValue;
		} else if (_tcscmp(pParseInfo->pMultiName, TOPDIRHIST_ATTR_NAME) == 0) {
			pParseInfo->pProperty->SetTopDirHist(pValue);
		} else if (_tcscmp(pParseInfo->pMultiName, SEARCHHIST_ATTR_NAME) == 0) {
			pParseInfo->pProperty->SetSearchHist(pValue);
		} else {
			delete [] pValue;
		}
	}
}

BOOL Property::LoadDefaultProperties()
{
	PropListNum *pNum = propListNum;
	while (pNum->nPropId != 0xFFFFFFFF) {
		nPropsNum[pNum->nPropId] = pNum->nDefault;
		pNum++;
	}

	PropListStr *pPLS = propListStr;
	while (pPLS->nPropId != 0xFFFFFFFF) {
		if (pPLS->pDefault != NULL) {
			pPropsStr[pPLS->nPropId] = StringDup(pPLS->pDefault);
			if (pPropsStr[pPLS->nPropId] == NULL) return FALSE;
		} else {
			pPropsStr[pPLS->nPropId] = NULL;
		}
		pPLS++;
	}
	return TRUE;
}

BOOL Property::Load()
{
	BOOL bResult = LoadProperties();

	// Convert topdir value to repository value
	LPCTSTR pTopDir = GetTopDir();
	if (pTopDir != NULL && GetNumSubRepository() == 0) {
		pRepos = new RepositoryImpl*[2];
		nNumRepos = 2;

		LocalFileRepository *pLocalImpl = new LocalFileRepository();
//		if (!pLocalImpl->Init(TEXT("default"), , pTopDir, bKeepTitle, bKeepCaret, bSafeFileName)) {
		// XXXX : 
		if (!pLocalImpl->Init(TEXT("default"), MSG_MEMO, pTopDir, FALSE, FALSE, FALSE)) {
			return FALSE;
		}
		pRepos[0] = pLocalImpl;
		pDefaultRep = pLocalImpl;

		VFolderRepository *pVImpl = new VFolderRepository();
		if (!pVImpl->Init(TEXT("@vfolder"), MSG_VIRTUAL_FOLDER)) {
			return FALSE;
		}
		pRepos[1] = pVImpl;
	}

	if (!bResult) {
		// set default value
		bNeedAsk = TRUE;
		return LoadDefaultProperties();
	}
	return TRUE;
}

BOOL Property::LoadProperties()
{
	TCHAR pathbuf[MAX_PATH + 1];
	TCHAR pathbuf2[MAX_PATH + 1];
	GetModuleFileName(NULL, pathbuf, MAX_PATH);
	GetFilePath(pathbuf2, pathbuf);
	TString sPropFile;
	if (!sPropFile.Join(pathbuf2, PROP_FILE_NAME)) return FALSE;

	File fFile;
	DWORD i;
	if (!fFile.Open(sPropFile.Get(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING)) {
		BOOL bStrict;
		BOOL bResult = LoadFromReg(&bStrict);
		if (bResult) {
			if (bStrict) {
				bNeedAsk = FALSE;
			} else {
				bNeedAsk = TRUE;
			}
			return TRUE;
		} else {
			return FALSE;
		}
	}
	DWORD nFileSize = fFile.FileSize();
	XML_Parser pParser = XML_ParserCreate(NULL);
	if (pParser == NULL) return FALSE;

	PropFileParseInfo ppi;
	ppi.pProperty = this;
	if (!ppi.vSubRepos.Init(5, 5)) return FALSE;

	XML_SetElementHandler(pParser, StartElement, EndElement);
	XML_SetUserData(pParser, &ppi);

	void *pBuf = XML_GetBuffer(pParser, nFileSize);
	if (pBuf == NULL) return FALSE;

	if (!fFile.Read((LPBYTE)pBuf, &nFileSize)) return FALSE;

	for (i = 0; i < NUM_PROPS_STR; i++) {
		delete [] pPropsStr[i];
		pPropsStr[i] = NULL;
	}

	if (!XML_ParseBuffer(pParser, nFileSize, TRUE)) {
		return FALSE;
	}
	XML_ParserFree(pParser);

	for (i = 0; i < nNumRepos; i++) {
		delete pRepos[i];
	}
	delete [] pRepos;

	nNumRepos = ppi.vSubRepos.NumItems();
	pRepos = new RepositoryImpl*[nNumRepos];
	for (i = 0; i < ppi.vSubRepos.NumItems(); i++) {
		RepositoryImpl *pImpl = *ppi.vSubRepos.GetUnit(i);
		pRepos[i] = pImpl;
	}
	pDefaultRep = ppi.pDefaultRep;

	bNeedAsk = FALSE;
	bLoad = TRUE;

	return TRUE;
}

BOOL Property::LoadFromReg(BOOL *pStrict)
{
	DWORD res, typ, siz;
	HKEY hTomboRoot;

	hTomboRoot = GetTomboRootKey();
	if (hTomboRoot == NULL) return FALSE;

	*pStrict = TRUE;

	if (pCmdlineAssignedTopDir) {
		SetTopDir(pCmdlineAssignedTopDir);
	} else {
		TCHAR buf[MAX_PATH];
		siz = sizeof(buf);
		res = RegQueryValueEx(hTomboRoot, TOPDIR_ATTR_NAME, NULL, &typ, (LPBYTE)buf, &siz);
		if (res == ERROR_SUCCESS) {
			if (!SetTopDir(buf)) return FALSE;
		} else {
#if defined(PLATFORM_BE500)
			GetUserDiskName(g_hInstance, buf, MAX_PATH);
			if (_tcslen(buf) + _tcslen(TOMBO_ROOT_SUFFIX) < MAX_PATH - 1) {
				_tcscat(buf, TOMBO_ROOT_SUFFIX);
			}
			if (!SetTopDir(buf)) return FALSE;
#else
			if (!SetTopDir(MEMO_TOP_DIR)) return FALSE;
#endif
			*pStrict = FALSE;
		}
	}

	// load number props.
	PropListNum *pNum = propListNum;
	while (pNum->nPropId != 0xFFFFFFFF) {
		DWORD nVal = GetDWORDFromReg(hTomboRoot, pNum->pAttrName, pNum->nDefault);
		nPropsNum[pNum->nPropId] = nVal;
		pNum++;
	}

#if !(defined(PLATFORM_WIN32) || defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5))
	SetSwitchWindowTitle(FALSE);
#endif

	LPTSTR pStr;

	PropListStr *pPLS = propListStr;
	while (pPLS->nPropId != 0xFFFFFFFF) {
		if (pPLS->nPropId == PROP_S_TOPDIR) { pPLS++; continue; }

		pStr = GetAllocSZFromReg(hTomboRoot, pPLS->pAttrName);
		if (pStr == NULL) {
			if (pPLS->pDefault) {
				pStr = StringDup(pPLS->pDefault);
			} else {
				pStr = NULL;
			}
		}
		delete [] pPropsStr[pPLS->nPropId];
		pPropsStr[pPLS->nPropId] = pStr;

		pPLS++;
	}

	delete[] pBookMark;
	pBookMark = GetMultiSZFromReg(hTomboRoot, BOOKMARK_ATTR_NAME);

	delete [] pSearchHistory;
	pSearchHistory = GetMultiSZFromReg(hTomboRoot, SEARCHHIST_ATTR_NAME); 

	delete [] pTopDirHistory;
	pTopDirHistory = GetMultiSZFromReg(hTomboRoot, TOPDIRHIST_ATTR_NAME);


#if defined(PLATFORM_HPC)
	LPCOMMANDBANDSRESTOREINFO pcbi = new COMMANDBANDSRESTOREINFO[NUM_COMMANDBAR];
	siz = sizeof(COMMANDBANDSRESTOREINFO) * NUM_COMMANDBAR;
	res = RegQueryValueEx(hTomboRoot, REBARHIST_ATTR_NAME, 0, 
						&typ, (LPBYTE)pcbi, &siz);

	if (siz == NUM_COMMANDBAR * sizeof(COMMANDBANDSRESTOREINFO)) {
		delete []pCmdBarInfo;
		pCmdBarInfo = pcbi;
	}
#endif
	RegCloseKey(hTomboRoot);
	return TRUE;
}

//////////////////////////////////////////
// save properties
//////////////////////////////////////////

BOOL SaveMultiSZToFile(File *pFile, LPCTSTR pAttr, LPCTSTR pMValue)
{
	if (pMValue == NULL) return TRUE;

	const char *pMultiPropHdr = "  <multistr name=\"";
	const char *pMultiPropClose = "\">\n";
	const char *pMultiPropCloseTag = "  </multistr>\n";
	const char *pItemHdr = "    <item value=\"";
	const char *pItemClose = "\"/>\n";

	DWORD nMultiPropHdr = strlen(pMultiPropHdr);
	DWORD nMultiPropClose = strlen(pMultiPropClose);
	DWORD nMultiPropCloseTag = strlen(pMultiPropCloseTag);
	DWORD nItemHdr = strlen(pItemHdr);
	DWORD nItemClose = strlen(pItemClose);

	if (!pFile->Write((LPBYTE)pMultiPropHdr, nMultiPropHdr)) return FALSE;

	char *pName = ConvUnicode2SJIS(pAttr);
	if (pName == NULL) return FALSE;
	if (!pFile->Write((LPBYTE)pName, strlen(pName))) { delete[] pName; return FALSE; }
	delete[] pName;

	if (!pFile->Write((LPBYTE)pMultiPropClose, nMultiPropClose)) return FALSE;

	LPCTSTR p = pMValue;
	while(*p) {
		LPCTSTR pTVal = p;
		if (!pFile->Write((LPBYTE)pItemHdr, nItemHdr)) return FALSE;

		char *pUTFVal = EscapeXMLStr(pTVal);
		if (!pFile->Write((LPBYTE)pUTFVal, strlen(pUTFVal))) { delete[] pUTFVal; return FALSE;}
		delete[] pUTFVal;

		if (!pFile->Write((LPBYTE)pItemClose, nItemClose)) return FALSE;
		p+= _tcslen(p) + 1;
	}

	if (!pFile->Write((LPBYTE)pMultiPropCloseTag, nMultiPropCloseTag)) return FALSE;
	return TRUE;
}

BOOL Property::SaveToFile(File *pFile)
{
	const char *pHeader = 
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		// TODO: DTD
		"<tomboprop version=\"" TOMBO_PROP_VERSION "\">\n";

	const char *pFooter = "</tomboprop>\n";

	if (!pFile->Write((LPBYTE)pHeader, strlen(pHeader))) return FALSE;

	const char *pNumPropHdr = "  <num name=\"";
	const char *pStrPropHdr = "  <str name=\"";
	const char *pProp2nd = "\" value=\"";
	const char *pPropLst = "\"/>\n";

	DWORD nNumPropHdr = strlen(pNumPropHdr);
	DWORD nStrPropHdr = strlen(pStrPropHdr);
	DWORD nProp2nd = strlen(pProp2nd);
	DWORD nPropLst = strlen(pPropLst);
	
	char buf[128];

	const char *pTomboRootStart = "  <tomboroot>\n";
	const char *pTomboRootEnd   = "  </tomboroot>\n";

	if (!pFile->Write((LPBYTE)pTomboRootStart, strlen(pTomboRootStart))) return FALSE;
	for (DWORD i = 0; i < g_Repository.GetNumOfSubRepository(); i++) {
		LPTSTR pSaveStr = g_Repository.GetSubRepoXMLSaveString(i);
		AutoPointer<TCHAR> ap(pSaveStr);
		if (pSaveStr == NULL) return FALSE;
		char *pSaveStrA = ConvTCharToUTF8(pSaveStr);
		AutoPointer<char> ap2(pSaveStrA);

		if (!pFile->Write((LPBYTE)pSaveStrA, strlen(pSaveStrA))) return FALSE;
	}
	if (!pFile->Write((LPBYTE)pTomboRootEnd, strlen(pTomboRootEnd))) return FALSE;


	// save number props.
	PropListNum *pNum = propListNum;
	while (pNum->nPropId != 0xFFFFFFFF) {

		if (!pFile->Write((LPBYTE)pNumPropHdr, nNumPropHdr)) return FALSE;

		char *pName = ConvUnicode2SJIS(pNum->pAttrName);
		if (pName == NULL) return FALSE;
		if (!pFile->Write((LPBYTE)pName, strlen(pName))) { delete[] pName; return FALSE; }
		delete[] pName;

		if (!pFile->Write((LPBYTE)pProp2nd, nProp2nd)) return FALSE;

		DWORD nValue = nPropsNum[pNum->nPropId];
		sprintf(buf, "%d", nValue);
		if (!pFile->Write((LPBYTE)buf, strlen(buf))) return FALSE;

		if (!pFile->Write((LPBYTE)pPropLst, nPropLst)) return FALSE;

		pNum++;
	}

	// save string props.
	PropListStr *pStr = propListStr;
	while (pStr->nPropId != 0xFFFFFFFF) {
		if (pPropsStr[pStr->nPropId] == NULL) {
			pStr++;
			continue;
		}

		if (!pFile->Write((LPBYTE)pStrPropHdr, nStrPropHdr)) return FALSE;

		char *pName = ConvUnicode2SJIS(pStr->pAttrName);
		if (pName == NULL) return FALSE;
		if (!pFile->Write((LPBYTE)pName, strlen(pName))) { delete[] pName; return FALSE; }
		delete[] pName;

		if (!pFile->Write((LPBYTE)pProp2nd, nProp2nd)) return FALSE;

		char *pVal = EscapeXMLStr(pPropsStr[pStr->nPropId]);
		if (pVal == NULL) return FALSE;
		if (!pFile->Write((LPBYTE)pVal, strlen(pVal))) { delete[] pVal; return FALSE; }
		delete[] pVal;

		if (!pFile->Write((LPBYTE)pPropLst, nPropLst)) return FALSE;
		pStr++;
	}

	// save multi value
	if (!SaveMultiSZToFile(pFile, BOOKMARK_ATTR_NAME, pBookMark)) return FALSE;
	if (!SaveMultiSZToFile(pFile, SEARCHHIST_ATTR_NAME, pSearchHistory)) return FALSE;
	if (!SaveMultiSZToFile(pFile, TOPDIRHIST_ATTR_NAME, pTopDirHistory)) return FALSE;

#if defined(PLATFORM_HPC)
	if (pCmdBarInfo != NULL) {
		LPCOMMANDBANDSRESTOREINFO p1 = pCmdBarInfo;
		LPCOMMANDBANDSRESTOREINFO p2 = pCmdBarInfo + 1;

		char aCmdBarBuf[1024];
		sprintf(aCmdBarBuf, "%d,%d,%d,%d,%d,%d,%d,%d",
			p1->wID, p1->fStyle, p1->cxRestored, p1->fMaximized, 
			p2->wID, p2->fStyle, p2->cxRestored, p2->fMaximized);

		const char *pRebarPropHdr = "  <rebar value=\"";
		if (!pFile->Write((LPBYTE)pRebarPropHdr, strlen(pRebarPropHdr))) return FALSE;
		if (!pFile->Write((LPBYTE)aCmdBarBuf, strlen(aCmdBarBuf))) return FALSE;
		if (!pFile->Write((LPBYTE)pPropLst, nPropLst)) return FALSE;
	}
#endif

	// footer
	if (!pFile->Write((LPBYTE)pFooter, strlen(pFooter))) return FALSE;
	return TRUE;
}

BOOL Property::Save()
{

	TCHAR pathbuf[MAX_PATH + 1];
	TCHAR pathbuf2[MAX_PATH + 1];
	GetModuleFileName(NULL, pathbuf, MAX_PATH);
	GetFilePath(pathbuf2, pathbuf);
	TString sPropFile, sPropFileTmp;
	if (!sPropFile.Join(pathbuf2, PROP_FILE_NAME)) return FALSE;
	if (!sPropFileTmp.Join(pathbuf2, PROP_TMP_FILE_NAME)) return FALSE;

	File propFile;
	if (!propFile.Open(sPropFileTmp.Get(), GENERIC_WRITE, FILE_SHARE_WRITE, CREATE_ALWAYS)) return FALSE;

	if (!SaveToFile(&propFile)) {
		return FALSE;
	} else {
		propFile.Close();
		DeleteFile(sPropFile.Get());
		MoveFile(sPropFileTmp.Get(), sPropFile.Get());
	}
	return TRUE;
}

//////////////////////////////////////////
// Registry operations
//////////////////////////////////////////

static DWORD GetDWORDFromReg(HKEY hKey, LPCTSTR pAttr, DWORD nDefault)
{
	DWORD siz = sizeof(DWORD);
	DWORD typ;
	DWORD nValue;
	DWORD res = RegQueryValueEx(hKey, pAttr, NULL, &typ, (LPBYTE)&nValue, &siz);
	if (res != ERROR_SUCCESS) {
		return nDefault;
	}
	return nValue;
}

static BOOL GetSZFromReg(HKEY hKey, LPCTSTR pAttr, LPTSTR pBuf, DWORD nBuf, LPCTSTR pDefault)
{
	DWORD res, siz, typ;
	siz = nBuf;
	res = RegQueryValueEx(hKey, pAttr, NULL, &typ, (LPBYTE)pBuf, &siz);
	if (res != ERROR_SUCCESS) {
		if (res == ERROR_FILE_NOT_FOUND) {
			_tcscpy(pBuf, pDefault);
			return TRUE;
		} else {
			SetLastError(res);
			return FALSE;
		}
	}
	return TRUE;
}

static LPTSTR GetAllocSZFromReg(HKEY hKey, LPCTSTR pAttr)
{
	DWORD res, siz, typ;
	siz = 0;
	res = RegQueryValueEx(hKey, pAttr, NULL, &typ, NULL, &siz);
	if (res != ERROR_SUCCESS) {
		SetLastError(res);
		return FALSE;
	}
	LPTSTR p = new TCHAR[siz / sizeof(TCHAR) + 1];
	res = RegQueryValueEx(hKey, pAttr, NULL, &typ, (LPBYTE)p, &siz);
	if (res != ERROR_SUCCESS) {
		SetLastError(res);
		return FALSE;
	}
	return p;
}

static LPTSTR GetMultiSZFromReg(HKEY hKey, LPCTSTR pAttr)
{
	DWORD res, siz, typ;
	res = RegQueryValueEx(hKey, pAttr, NULL, &typ, NULL, &siz);
	if (res != ERROR_SUCCESS) {
		SetLastError(res);
		return NULL;
	}

	LPTSTR pBuf;
	DWORD n = siz / sizeof(TCHAR) + 1;
	pBuf = new TCHAR[n];
	if (pBuf == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	memset(pBuf, 0, n * sizeof(TCHAR));
	res = RegQueryValueEx(hKey, pAttr, NULL, &typ, (LPBYTE)pBuf, &siz);
	if (res != ERROR_SUCCESS) {
		delete[] pBuf;
		SetLastError(res);
		return NULL;
	}
	return pBuf;
}

///////////////////////////////////////////////////
// get font
///////////////////////////////////////////////////

static BOOL MakeFont(HFONT *phFont, LPCTSTR pName, DWORD nSize, BYTE bQuality)
{
	if (nSize == 0xFFFFFFFF) {
		*phFont = NULL;
		return TRUE;
	}
	LOGFONT lf;
	lf.lfHeight = nSize;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = FW_NORMAL;
	lf.lfItalic = FALSE;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = bQuality;
	lf.lfPitchAndFamily = DEFAULT_PITCH;
	_tcscpy(lf.lfFaceName, pName);
	*phFont = CreateFontIndirect(&lf);
	return *phFont != NULL;
}

///////////////////////////////////////////////////
// save window size
///////////////////////////////////////////////////

BOOL Property::SaveWinSize(UINT flags, UINT showCmd, LPRECT pWinRect, WORD nSelectViewWidth)
{
	TCHAR buf[1024];

	wsprintf(buf, TEXT("%d,%d,%d,%d,%d,%d,%d"), 
		flags, showCmd,
		pWinRect->left, pWinRect->top,
		pWinRect->right, pWinRect->bottom,
		nSelectViewWidth);

	delete [] pPropsStr[PROP_S_WINSIZE];
	pPropsStr[PROP_S_WINSIZE] = StringDup(buf);
	return TRUE;
}

///////////////////////////////////////////////////
// get window size
///////////////////////////////////////////////////

BOOL Property::GetWinSize(UINT *pFlags, UINT *pShowCmd, LPRECT pWinRect, LPWORD pSelectViewWidth)
{
	if (pPropsStr[PROP_S_WINSIZE] == NULL) {
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}

	DWORD n;
	if (_stscanf(pPropsStr[PROP_S_WINSIZE], TEXT("%d,%d,%d,%d,%d,%d,%d"),
		pFlags, pShowCmd,
		&(pWinRect->left), &(pWinRect->top),
		&(pWinRect->right), &(pWinRect->bottom),
		&n) != 7) {
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}
	*pSelectViewWidth = (WORD)n;
	// check and modify window position
	if (pWinRect->left < 0) pWinRect->left = 0;
	if (pWinRect->top < 0) pWinRect->top = 0;

	return TRUE;
}

///////////////////////////////////////////////////
// helper funcs
///////////////////////////////////////////////////

static HKEY GetTomboRootKey()
{
	DWORD res;
	HKEY hTomboRoot;

	res = RegOpenKeyEx(HKEY_CURRENT_USER, TOMBO_MAIN_KEY, 0, KEY_READ, &hTomboRoot);
	if (res != ERROR_SUCCESS) {
		SetLastError(res);
		return NULL;
	}
	return hTomboRoot;
}

////////////////////////////////////////////////////////////////
// Get/Set History from/to ComboBox
////////////////////////////////////////////////////////////////

BOOL SetHistoryToComboBox(HWND hCombo, LPCTSTR pHistoryStr)
{
	if (pHistoryStr == NULL) return TRUE;

	LPCTSTR p = pHistoryStr;
	LPCTSTR q = p;
	while(*q) {
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)q);
		q += _tcslen(q) + 1;
	}
	SendMessage(hCombo, CB_SETCURSEL, 0, 0);
	return TRUE;
}

LPTSTR GetHistoryFromComboBox(HWND hCombo, LPCTSTR pSelValue, DWORD nSave)
{
	DWORD nItems = SendMessage(hCombo, CB_GETCOUNT, 0, 0);
	if (nItems > nSave) nItems = nSave;

	DWORD i;
	DWORD nMatch = SendMessage(hCombo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pSelValue);

	DWORD nBufLen = _tcslen(pSelValue) + 1;
	DWORD nLen;
	for (i = 0; i < nItems; i++) {
		if (i == nMatch) continue;

		nLen = SendMessage(hCombo, CB_GETLBTEXTLEN, i, 0);
		if (nLen == CB_ERR) return FALSE;
		nBufLen += nLen + 1;
	}
	nBufLen++;

	LPTSTR p = new TCHAR[nBufLen];
	if (!p) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	LPTSTR q = p;
	_tcscpy(q, pSelValue);
	q += _tcslen(pSelValue) + 1;
	for (i = 0; i < nItems; i++) {
		if (i == nMatch) continue;
		SendMessage(hCombo, CB_GETLBTEXT, i, (LPARAM)q);
		q += _tcslen(q) + 1;
	}
	*q = TEXT('\0');
	return p;
}

#if defined(PLATFORM_HPC)
BOOL Property::SetCommandbarInfo(LPCOMMANDBANDSRESTOREINFO p)
{
	delete[] pCmdBarInfo;
	pCmdBarInfo = new COMMANDBANDSRESTOREINFO[NUM_COMMANDBAR];
	memcpy(pCmdBarInfo, p, sizeof(COMMANDBANDSRESTOREINFO) * NUM_COMMANDBAR);
	return TRUE;
}

BOOL Property::GetCommandbarInfo(LPCOMMANDBANDSRESTOREINFO p, DWORD n)
{
	if (pCmdBarInfo == NULL) return FALSE;
	memcpy(p, pCmdBarInfo, sizeof(COMMANDBANDSRESTOREINFO) * NUM_COMMANDBAR);
	return TRUE;
}

#endif

BOOL Property::SetCmdLineAssignedTomboRoot(LPCTSTR p, DWORD nLen)
{
	pCmdlineAssignedTopDir = new TCHAR[nLen + 1];
	if (!pCmdlineAssignedTopDir) return FALSE;
	_tcsncpy(pCmdlineAssignedTopDir, p, nLen);
	pCmdlineAssignedTopDir[nLen] = TEXT('\0');
	ChopFileSeparator(pCmdlineAssignedTopDir);
	return TRUE;
}

static DWORD CountMultiSZLen(LPCTSTR pData)
{
	if (pData == NULL) return 0;
	LPCTSTR p = pData;
	DWORD n = 0;
	while(*p) {
		DWORD i = _tcslen(p) + 1;
		p += i;
		n += i;
	}
	n++;
	return n;
}

///////////////////////////////////////////////////
// BookMark
///////////////////////////////////////////////////

BOOL Property::SetBookMark(LPCTSTR pBM)
{
	DWORD nSize = CountMultiSZLen(pBM);

	LPTSTR pBuf = new TCHAR[nSize];
	if (pBuf == NULL) return FALSE;
	for (DWORD i = 0; i < nSize; i++) {
		pBuf[i] = pBM[i];
	}
	delete[] pBookMark;

	pBookMark = pBuf;
	return TRUE;
}

///////////////////////////////////////////////////
// Code conversion related
///////////////////////////////////////////////////

LPBYTE ConvTCharToFileEncoding(LPCTSTR p, LPDWORD pSize)
{
	LPBYTE pData;

	switch (g_Property.GetCodePage()) {
	case TOMBO_CP_UTF16LE:
		pData = (LPBYTE)ConvTCharToWChar(p);
		if (pData == NULL) return FALSE;
		*pSize = wcslen((LPCWSTR)pData) * sizeof(WCHAR);
		break;
	case TOMBO_CP_UTF8:
		pData = (LPBYTE)ConvTCharToUTF8(p);
		if (pData == NULL) return FALSE;
		*pSize = strlen((const char*)pData);
		break;
	default:
		pData = (LPBYTE)ConvUnicode2SJIS(p);
		if (pData == NULL) return FALSE;
		*pSize = strlen((const char*)pData);
	}
	return pData;
}

LPTSTR ConvFileEncodingToTChar(LPBYTE p)
{
	switch (g_Property.GetCodePage()) {
	case TOMBO_CP_UTF16LE:
		return ConvWCharToTChar((LPCWSTR)p);
	case TOMBO_CP_UTF8:
		return ConvUTF8ToTChar((const char*)p);
	default:
		return ConvSJIS2Unicode((const char*)p);	
	}
}

RepositoryImpl *Property::GetSubRepository(DWORD nIndex) 
{ 
	RepositoryImpl *pImpl = pRepos[nIndex];
	if (pImpl == NULL) return NULL;
	return pImpl->Clone(); 
}

DWORD Property::GetKeepCaret()
{
	if (pDefaultRep) return pDefaultRep->GetKeepCaret();
	else return nPropsNum[PROP_N_DETAILSVIEW_KEEPCARET];
}

void Property::SetKeepCaret(DWORD n)
{
	if (pDefaultRep) pDefaultRep->SetKeepCaret(n);
	nPropsNum[PROP_N_DETAILSVIEW_KEEPCARET] = n;
}

DWORD Property::GetUseSafeFileName()
{
	if (pDefaultRep) return pDefaultRep->GetSafeFileName();
	else return nPropsNum[PROP_N_SAFEFILENAME];
}

void Property::SetUseSafeFileName(DWORD n)
{
	if (pDefaultRep) pDefaultRep->SetSafeFileName(n);
	nPropsNum[PROP_N_SAFEFILENAME] = n;
}

DWORD Property::GetKeepTitle()
{
	if (pDefaultRep) return pDefaultRep->GetKeepTitle();
	else return nPropsNum[PROP_N_KEEP_TITLE];
}

void Property::SetKeepTitle(DWORD n)
{
	if (pDefaultRep) pDefaultRep->SetKeepTitle(n);
	nPropsNum[PROP_N_KEEP_TITLE] = n;
}

