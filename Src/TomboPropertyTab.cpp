#include <windows.h>
#include <tchar.h>

#include "Tombo.h"
#include "resource.h"
#include "Message.h"
#include "Property.h"
#include "UniConv.h"
#include "TString.h"
#include "DialogTemplate.h"
#include "PropertyPage.h"
#include "TomboPropertyTab.h"
#include "FileSelector.h"

//////////////////////////////////////////
// defs
//////////////////////////////////////////

#define MEMO_TOP_DIR_NUM_HISTORY 8
#define FONTSIZE_MIN 6
#define FONTSIZE_MAX 32
#define DEFAULT_FONTNAME TEXT("Tahoma")
#define DEFAULT_FONTSIZE 9
#define CLEARTYPE_QUALITY 5

#define SIP_DELTA 16

static BOOL CreateDirectories(LPCTSTR pDir);

static void SetCheckBox(HWND hWnd, BOOL bChecked)
{
	if (bChecked) {
		SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}
}

static BOOL GetCheckBox(HWND hWnd)
{
	if (SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		return TRUE;
	} else {
		return FALSE;
	}
}

//////////////////////////////////////////
// TomboRoot setting tab
//////////////////////////////////////////

static DlgMsgRes aFolderRes[] = {
	{ IDC_PROPTAB_FOLDER_LABEL, MSG_ID_DLG_PROPTAB_FOLDER_LABEL },
	{ IDC_PROPTAB_FOLDER_HELP1, MSG_ID_DLG_PROPTAB_FOLDER_HELP1 },
	{ IDC_KEEPTITLE,            MSG_ID_DLG_PROPTAB_FOLDER_SYNCTITLE },
	{ IDC_PROPTAB_FOLDER_HELP2, MSG_ID_DLG_PROPTAB_FOLDER_HELP2 },
	{ IDC_PROP_READONLY,        MSG_ID_DLG_PROPTAB_FOLDER_READONLY },
};

void FolderTab::Init(HWND hDlg)
{
	OverrideDlgMsg(hDlg, -1, aFolderRes, sizeof(aFolderRes)/sizeof(DlgMsgRes));

	HWND hFolder = GetDlgItem(hDlg, IDC_TOPFOLDER);
	LPCTSTR pHist = g_Property.GetTopDirHist();
	SetHistoryToComboBox(hFolder, pHist);
	if (GetWindowTextLength(hFolder) == 0) {
		if (pProperty->GetTopDir()) {
			SetWindowText(hFolder, pProperty->GetTopDir());
		}
	}

	HWND hKeepTitle = GetDlgItem(hDlg, IDC_KEEPTITLE);
	if (pProperty->GetKeepTitle()) {
		SendMessage(hKeepTitle, BM_SETCHECK, BST_UNCHECKED, 0);
	} else {
		SendMessage(hKeepTitle, BM_SETCHECK, BST_CHECKED, 0);
	}

	HWND hOpenReadOnly = GetDlgItem(hDlg, IDC_PROP_READONLY);
	if (pProperty->GetOpenReadOnly()) {
		SendMessage(hOpenReadOnly, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hOpenReadOnly, BM_SETCHECK, BST_UNCHECKED, 0);
	}
}

BOOL FolderTab::Apply(HWND hDlg)
{
	HWND hKeepTitle = GetDlgItem(hDlg, IDC_KEEPTITLE);
	if (SendMessage(hKeepTitle, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->SetKeepTitle(FALSE);
	} else {
		pProperty->SetKeepTitle(TRUE);
	}

	TCHAR aTopPath[MAX_PATH];
	HWND hTopPath = GetDlgItem(hDlg, IDC_TOPFOLDER);
	GetWindowText(hTopPath, aTopPath, MAX_PATH);

	// 空白の除去
	LPTSTR p = aTopPath;
	while(*p == TEXT(' ') || *p == TEXT('\t')) p++;
	if (_tcslen(p) == 0) {
		SetFocus(hTopPath);
		SendMessage(hTopPath, EM_SETSEL, 0, -1);
		return FALSE;
	}

#if defined(PLATFORM_WIN32)
	if (_tcslen(p) == 3 && _istalpha(p[0]) && p[1] == TEXT(':') && p[2] == TEXT('\\')) {
		return pProperty->SetTopDir(p);
	} else if (_tcslen(p) == 2 && _istalpha(p[0]) && p[1] == TEXT(':')) {
		p[2] = TEXT('\\');
		p[3] = TEXT('\0');
		return pProperty->SetTopDir(p);
	} else {
		ChopFileSeparator(p);
	}
#endif
#if defined(_WIN32_WCE)
	if (_tcscmp(p, TEXT("\\")) == 0) {
		MessageBox(hDlg, MSG_ROOT_NOT_ALLOWED, TOMBO_APP_NAME, MB_ICONERROR | MB_OK);
		SetFocus(hTopPath);
		return FALSE;
	}
	ChopFileSeparator(p);
#endif
	// フォルダの存在チェック
	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile(p, &fd);
	if (h == INVALID_HANDLE_VALUE) {
		// 存在せず
		if (MessageBox(hDlg, MSG_IS_CREATE_DIR, MSG_CREATE_DIR_TTL, MB_ICONQUESTION | MB_OKCANCEL) == IDOK) {
			if (!CreateDirectories(p)) {
				MessageBox(hDlg, MSG_MKDIR_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
				SetFocus(hTopPath);
				SendMessage(hTopPath, EM_SETSEL, 0, -1);
				return FALSE;
			}
		} else {
			SetFocus(hTopPath);
			SendMessage(hTopPath, EM_SETSEL, 0, -1);
			return FALSE;
		}
	} else {
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			// ディレクトリではない
			SetFocus(hTopPath);
			SendMessage(hTopPath, EM_SETSEL, 0, -1);
			return FALSE;
		}
		FindClose(h);
	}

	// Save history
	g_Property.SetTopDirHist(GetHistoryFromComboBox(hTopPath, p, MEMO_TOP_DIR_NUM_HISTORY));

	HWND hReadOnly = GetDlgItem(hDlg, IDC_PROP_READONLY);
	if (SendMessage(hReadOnly, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->SetOpenReadOnly(TRUE);
	} else {
		pProperty->SetOpenReadOnly(FALSE);
	}

	return pProperty->SetTopDir(p);
}

BOOL FolderTab::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam)) {
	case IDC_FOLDERSEL:
		{
			FileSelector sel;
			if (sel.Popup(g_hInstance, hDlg, MSG_CHOOSE_FOLDER, NULL) == IDOK) {
				HWND hEdit = GetDlgItem(hDlg, IDC_TOPFOLDER);
				SetWindowText(hEdit, sel.SelectedPath());
			}
		}
		break;
	}
	return TRUE;
}

//////////////////////////////////////////
// Password timeout setting tab
//////////////////////////////////////////

static DlgMsgRes aPassTimeout[] = {
	{ IDC_PROPTAB_PASS_TIMEOUT_LABEL,MSG_ID_DLG_PROPTAB_PASSTO_LABEL },
	{ IDC_PROPTAB_PASS_TIMEOUT_HELP, MSG_ID_DLG_PROPTAB_PASSTO_HELP },
	{ IDC_PASS_TIMEOUT_USE_SAFEFILE, MSG_ID_DLG_PROPTAB_TIMEOUT_USE_SAFENAME},
	{ IDC_PROPTAB_PASS_TIMEOUT_SAFEFILE_HELP, MSG_ID_DLG_PROPTAB_TIMEOUT_USE_SAFENAME_HELP},
};

void PassTimeoutTab::Init(HWND hDlg)
{
	OverrideDlgMsg(hDlg, -1, aPassTimeout, sizeof(aPassTimeout)/sizeof(DlgMsgRes));
	HWND hTimeout = GetDlgItem(hDlg, IDC_PASS_TIMEOUT);
	TCHAR buf[64];
	wsprintf(buf, TEXT("%d"), pProperty->GetPassTimeout());
	SetWindowText(hTimeout, buf);

	HWND hSafeFileName = GetDlgItem(hDlg, IDC_PASS_TIMEOUT_USE_SAFEFILE);
	if (pProperty->GetUseSafeFileName()) {
		SendMessage(hSafeFileName, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hSafeFileName, BM_SETCHECK, BST_UNCHECKED, 0);
	}
}

BOOL PassTimeoutTab::Apply(HWND hDlg)
{
	HWND hTimeout = GetDlgItem(hDlg, IDC_PASS_TIMEOUT);
	TCHAR buf[64];
	DWORD n;

	GetWindowText(hTimeout, buf, 64);

	n = _ttol(buf);
	if (n < 1 || n > 60) {
		SetFocus(hTimeout);
		return FALSE;
	}
	pProperty->SetPassTimeout(n);

	HWND hSafeFileName = GetDlgItem(hDlg, IDC_PASS_TIMEOUT_USE_SAFEFILE);
	if (SendMessage(hSafeFileName, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->SetUseSafeFileName(TRUE);
	} else {
		pProperty->SetUseSafeFileName(FALSE);
	}
	return TRUE;
}

//////////////////////////////////////////
// Font setting tab
//////////////////////////////////////////

extern "C" static int CALLBACK PropEnumFonts(ENUMLOGFONT FAR *pFont, NEWTEXTMETRIC FAR *pMetric, int iFontType, LPARAM lParam);

static void SetFontName(HDC hDC, HWND hFontName, LPCTSTR pDefaultFontName)
{
	EnumFontFamilies(hDC, NULL, (FONTENUMPROC)PropEnumFonts, (LPARAM)hFontName);
	DWORD n = SendMessage(hFontName, CB_FINDSTRINGEXACT, -1, (LPARAM)pDefaultFontName);
	if (n != CB_ERR) {
		SendMessage(hFontName, CB_SETCURSEL, n, 0);
	} else {
		SendMessage(hFontName, CB_SETCURSEL, 0, 0);
	}
}

static void SetFontSize(HWND hFontSize, DWORD nDefaultSize)
{
	TCHAR buf[32];
	BOOL bSet = FALSE;

	for (DWORD i = FONTSIZE_MIN; i <= FONTSIZE_MAX; i++) {
		wsprintf(buf, TEXT("%d"), i);
		SendMessage(hFontSize, CB_ADDSTRING, 0, (LPARAM)buf);
		if (i == nDefaultSize) {
			bSet = TRUE;
			SendMessage(hFontSize, CB_SETCURSEL, i - FONTSIZE_MIN, 0);
		}
	}
	if (!bSet) {
		wsprintf(buf, TEXT("%d"), nDefaultSize);
		SendMessage(hFontSize, CB_INSERTSTRING, 0, (LPARAM)buf);
		SendMessage(hFontSize, CB_SETCURSEL, 0, 0);
	}
}

static void InitFontControls(HDC hDC, HWND hName, HWND hSize, HWND hDefault, HWND hCT, LPCTSTR pPropName, DWORD nPropSize, BYTE bQuality)
{
	LPCTSTR pFont;
	DWORD nSize;

	if (nPropSize == 0xFFFFFFFF) {
		pFont = DEFAULT_FONTNAME;
		nSize = DEFAULT_FONTSIZE;
		SendMessage(hDefault, BM_SETCHECK, BST_CHECKED, 0);
		EnableWindow(hName, FALSE);
		EnableWindow(hSize, FALSE);
		EnableWindow(hCT, FALSE);
	} else {
		SendMessage(hDefault, BM_SETCHECK, BST_UNCHECKED, 0);
		EnableWindow(hName, TRUE);
		EnableWindow(hSize, TRUE);
		EnableWindow(hCT, TRUE);
		pFont = pPropName;
		nSize = nPropSize;
	}
	SetFontName(hDC, hName, pFont);
	SetFontSize(hSize, nSize);

	if (bQuality == DEFAULT_QUALITY) {
		SendMessage(hCT, BM_SETCHECK, BST_UNCHECKED, 0);
	} else {
		SendMessage(hCT, BM_SETCHECK, BST_CHECKED, 0);
	}
}

static DlgMsgRes aFontRes[] = {
	{ IDC_PROPTAB_FONT_TREE_LABEL, MSG_ID_DLG_PROPTAB_FONT_TREE_LABEL },
	{ IDC_FONT_SELECT_USEDEFAULT,  MSG_ID_DLG_PROPTAB_FONT_USE_DEFAULT },
#if defined(PLATFORM_WIN32)
	{ IDC_FONT_SELECT_CLEARTYPE,   MSG_ID_DLG_PROPTAB_USE_CLEARTYPE_FONT },
	{ IDC_FONT_DETAILS_CLEARTYPE,  MSG_ID_DLG_PROPTAB_USE_CLEARTYPE_FONT },
#else
	{ IDC_FONT_SELECT_CLEARTYPE,   MSG_ID_DLG_PROPTAB_USE_CLEARTYPE_FONT_CE },
	{ IDC_FONT_DETAILS_CLEARTYPE,  MSG_ID_DLG_PROPTAB_USE_CLEARTYPE_FONT_CE },
#endif
	{ IDC_PROPTAB_FONT_EDIT_LABEL, MSG_ID_DLG_PROPTAB_FONT_EDIT_LABEL },
	{ IDC_FONT_DETAILS_USEDEFAULT, MSG_ID_DLG_PROPTAB_FONT_USE_DEFAULT },
};

void FontTab::Init(HWND hDlg)
{
	OverrideDlgMsg(hDlg, -1, aFontRes, sizeof(aFontRes)/sizeof(DlgMsgRes));

	HWND hSelectUseDefault = GetDlgItem(hDlg, IDC_FONT_SELECT_USEDEFAULT);
	HWND hSelectName = GetDlgItem(hDlg, IDC_FONT_SELECT_NAME);
	HWND hSelectSize = GetDlgItem(hDlg, IDC_FONT_SELECT_SIZE);
	HWND hSelectCT   = GetDlgItem(hDlg, IDC_FONT_SELECT_CLEARTYPE);

	HWND hDetailsUseDefault = GetDlgItem(hDlg, IDC_FONT_DETAILS_USEDEFAULT);
	HWND hDetailsName = GetDlgItem(hDlg, IDC_FONT_DETAILS_NAME);
	HWND hDetailsSize = GetDlgItem(hDlg, IDC_FONT_DETAILS_SIZE);
	HWND hDetailsCT   = GetDlgItem(hDlg, IDC_FONT_DETAILS_CLEARTYPE);

	HDC hDC = GetDC(hDlg);

	InitFontControls(hDC, hSelectName, hSelectSize, hSelectUseDefault, hSelectCT, pProperty->GetSelectViewFontName(), pProperty->GetSelectViewFontSize(), (BYTE)pProperty->GetSelectViewFontQuality());
	InitFontControls(hDC, hDetailsName, hDetailsSize, hDetailsUseDefault, hDetailsCT, pProperty->GetDetailsViewFontName(), pProperty->GetDetailsViewFontSize(), (BYTE)pProperty->GetDetailsViewFontQuality());

	ReleaseDC(hDlg, hDC);
}

static BOOL CheckNumberFormat(LPCTSTR pStr)
{
	LPCTSTR p = pStr;
	while(*p) {
		if (!_istdigit(*p)) return FALSE;
		p++;
	}
	return TRUE;
}

static BOOL GetFontStat(HWND hName, HWND hSize, HWND hDefault, HWND hCT, LPTSTR pName, LPDWORD pSize, LPBYTE pClearType)
{
	DWORD nStat = SendMessage(hDefault, BM_GETCHECK, 0, 0);
	if (nStat & BST_CHECKED) {
		_tcscpy(pName, TEXT(""));
		*pSize = 0xFFFFFFFF;
	} else {
		DWORD n;
		n = SendMessage(hName, CB_GETCURSEL, 0, 0);
		SendMessage(hName, CB_GETLBTEXT, n, (LPARAM)pName);

		// get font size
		TCHAR buf[256];
		GetWindowText(hSize, buf, 256);
		int is = _ttoi(buf);
		if (!CheckNumberFormat(buf) || is <= 0) {
			MessageBox(hSize, MSG_INVALID_FONT_SIZE, TOMBO_APP_NAME, MB_ICONWARNING|MB_OK);
			return FALSE;
		}
		*pSize = (DWORD)is;
	}

	DWORD nCT = SendMessage(hCT, BM_GETCHECK, 0, 0);
	if (nCT & BST_CHECKED) {
		*pClearType = CLEARTYPE_QUALITY;
	} else {
		*pClearType = DEFAULT_QUALITY;
	}
	return TRUE;
}

BOOL FontTab::Apply(HWND hDlg)
{
	HWND hSelectUseDefault = GetDlgItem(hDlg, IDC_FONT_SELECT_USEDEFAULT);
	HWND hSelectName = GetDlgItem(hDlg, IDC_FONT_SELECT_NAME);
	HWND hSelectSize = GetDlgItem(hDlg, IDC_FONT_SELECT_SIZE);
	HWND hSelectCT   = GetDlgItem(hDlg, IDC_FONT_SELECT_CLEARTYPE);

	HWND hDetailsUseDefault = GetDlgItem(hDlg, IDC_FONT_DETAILS_USEDEFAULT);
	HWND hDetailsName = GetDlgItem(hDlg, IDC_FONT_DETAILS_NAME);
	HWND hDetailsSize = GetDlgItem(hDlg, IDC_FONT_DETAILS_SIZE);
	HWND hDetailsCT   = GetDlgItem(hDlg, IDC_FONT_DETAILS_CLEARTYPE);

	TCHAR fontName[LF_FACESIZE];

	DWORD nFS;
	BYTE nFQ;
	if(!GetFontStat(hSelectName, hSelectSize, hSelectUseDefault, hSelectCT, fontName, &nFS, &nFQ)) {
		return FALSE;
	}
	if (!pProperty->SetSelectViewFontName(fontName)) return FALSE;
	pProperty->SetSelectViewFontSize(nFS);
	pProperty->SetSelectViewFontQuality(nFQ);

	if (!GetFontStat(hDetailsName, hDetailsSize, hDetailsUseDefault, hDetailsCT, fontName, &nFS, &nFQ)) {
		return FALSE;
	}
	if (!pProperty->SetDetailsViewFontName(fontName)) return FALSE;
	pProperty->SetDetailsViewFontSize(nFS);
	pProperty->SetDetailsViewFontQuality(nFQ);
	return TRUE;
}

static void ToggleDefault(HWND hName, HWND hSize, HWND hDefault, HWND hCT)
{
	if (SendMessage(hDefault, BM_GETCHECK, 0, 0) & BST_CHECKED) {
		// デフォルトフォント
		EnableWindow(hName, FALSE);
		EnableWindow(hSize, FALSE);
		EnableWindow(hCT, FALSE);
	} else {
		// ユーザ定義フォント
		EnableWindow(hName, TRUE);
		EnableWindow(hSize, TRUE);
		EnableWindow(hCT, TRUE);
	}
}

BOOL FontTab::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	HWND hSelectUseDefault = GetDlgItem(hDlg, IDC_FONT_SELECT_USEDEFAULT);
	HWND hSelectName = GetDlgItem(hDlg, IDC_FONT_SELECT_NAME);
	HWND hSelectSize = GetDlgItem(hDlg, IDC_FONT_SELECT_SIZE);
	HWND hSelectCT   = GetDlgItem(hDlg, IDC_FONT_SELECT_CLEARTYPE);
	HWND hDetailsUseDefault = GetDlgItem(hDlg, IDC_FONT_DETAILS_USEDEFAULT);
	HWND hDetailsName = GetDlgItem(hDlg, IDC_FONT_DETAILS_NAME);
	HWND hDetailsSize = GetDlgItem(hDlg, IDC_FONT_DETAILS_SIZE);
	HWND hDetailsCT   = GetDlgItem(hDlg, IDC_FONT_DETAILS_CLEARTYPE);

	switch (wParam) {
	case IDC_FONT_SELECT_USEDEFAULT:
		ToggleDefault(hSelectName, hSelectSize, hSelectUseDefault, hSelectCT);
		break;
	case IDC_FONT_DETAILS_USEDEFAULT:
		ToggleDefault(hDetailsName, hDetailsSize, hDetailsUseDefault, hDetailsCT);
		break;
	}
	return TRUE;
}

//////////////////////////////////////////
// Date format
//////////////////////////////////////////

static DlgMsgRes aDateRes[] = {
	{ IDC_PROPTAB_INSDATE_DATE1, MSG_ID_DLG_PROPTAB_DATE_INSDATE1 },
	{ IDC_PROPTAB_INSDATE_DATE2, MSG_ID_DLG_PROPTAB_DATE_INSDATE2 },
	{ IDC_DATEFORMAT_DESC,       MSG_ID_DATEFORMAT_DESC },
};

void DateFormatTab::Init(HWND hDlg)
{
	OverrideDlgMsg(hDlg, -1, aDateRes, sizeof(aDateRes)/sizeof(DlgMsgRes));

	HWND hFormat1 = GetDlgItem(hDlg, IDC_PROP_DATEFORMAT1);
	HWND hFormat2 = GetDlgItem(hDlg, IDC_PROP_DATEFORMAT2);
	HWND hDesc = GetDlgItem(hDlg, IDC_DATEFORMAT_DESC);
	SetWindowText(hFormat1, pProperty->GetDateFormat1());
	SetWindowText(hFormat2, pProperty->GetDateFormat2());
	DWORD nTS = 4*4;
	SendMessage(hDesc, EM_SETTABSTOPS, 1, (LPARAM)&nTS);
//	SetWindowText(hDesc, MSG_DATEFORMAT_DESC);
}

BOOL DateFormatTab::Apply(HWND hDlg)
{
	HWND hFormat1 = GetDlgItem(hDlg, IDC_PROP_DATEFORMAT1);
	HWND hFormat2 = GetDlgItem(hDlg, IDC_PROP_DATEFORMAT2);

	TCHAR buf[MAX_DATEFORMAT_LEN];
	GetWindowText(hFormat1, buf, MAX_DATEFORMAT_LEN - 1);
	if (!pProperty->SetDateFormat1(buf)) return FALSE;
	GetWindowText(hFormat2, buf, MAX_DATEFORMAT_LEN - 1);
	if (!pProperty->SetDateFormat2(buf)) return FALSE;
	return TRUE;
}

//////////////////////////////////////////
// Caret setting tab
//////////////////////////////////////////

static DlgMsgRes aKeepCaretRes[] = {
	{ IDC_PROP_KEEPCARET,          MSG_ID_DLG_PROPTAB_KEEPCARET_KEEPCARET },
#if !defined(PLATFORM_BE500)
	{ IDC_PROPTAB_KEEPCARET_HELP1, MSG_ID_DLG_PROPTAB_KEEPCARET_HELP1 },
	{ IDC_PROPTAB_KEEPCARET_LABEL, MSG_ID_DLG_PROPTAB_KEEPCARET_LABEL },
	{ IDC_PROPTAB_KEEPCARET_HELP2, MSG_ID_DLG_PROPTAB_KEEPCARET_HELP2 },
#if !defined(PLATFORM_PSPC) && !defined(PLATFORM_HPC)
	{ IDC_PROP_SWITCHTITLE,        MSG_ID_DLG_PROPTAB_KEEPCARET_SYNCTITLE },
	{ IDC_PROPTAB_KEEPCARET_HELP3, MSG_ID_DLG_PROPTAB_KEEPCARET_HELP3 },
#endif
	{ IDC_PROPTAB_DISABLESAVEDLG,  MSG_ID_DLG_PROPTAB_DISABLESAVEDLG },
#endif
};

void KeepCaretTab::Init(HWND hDlg)
{
	OverrideDlgMsg(hDlg, -1, aKeepCaretRes, sizeof(aKeepCaretRes)/sizeof(DlgMsgRes));

	HWND hWnd = GetDlgItem(hDlg, IDC_PROP_KEEPCARET);
	if (pProperty->GetKeepCaret()) {
		SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	// tab stop
	HWND hTabWnd = GetDlgItem(hDlg, IDC_PROP_TABSTOP);
	TCHAR buf[30];
	wsprintf(buf, TEXT("%d"), pProperty->GetTabstop());
	SetWindowText(hTabWnd, buf);

#if defined(PLATFORM_WIN32) || defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	HWND hSwitchTitleWnd = GetDlgItem(hDlg, IDC_PROP_SWITCHTITLE);
	if (pProperty->GetSwitchWindowTitle()) {
		SendMessage(hSwitchTitleWnd, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hSwitchTitleWnd, BM_SETCHECK, BST_UNCHECKED, 0);
	}
#endif

	HWND hDisableSaveDlg = GetDlgItem(hDlg, IDC_PROPTAB_DISABLESAVEDLG);
	if (pProperty->GetDisableSaveDlg()) {
		SendMessage(hDisableSaveDlg, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hDisableSaveDlg, BM_SETCHECK, BST_UNCHECKED, 0);
	}

}

BOOL KeepCaretTab::Apply(HWND hDlg)
{
	HWND hWnd = GetDlgItem(hDlg, IDC_PROP_KEEPCARET);
	if (SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->SetKeepCaret(TRUE);
	} else {
		pProperty->SetKeepCaret(FALSE);
	}

	// tab stop
	HWND hTabWnd = GetDlgItem(hDlg, IDC_PROP_TABSTOP);
	TCHAR buf[30];
	int n;
	GetWindowText(hTabWnd, buf, 30);

	n = _ttol(buf);
	if (n < 1) {
		SetFocus(hTabWnd);
		return FALSE;
	}
	pProperty->SetTabstop(n);

#if defined(PLATFORM_WIN32) || defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	HWND hSwitchWnd = GetDlgItem(hDlg, IDC_PROP_SWITCHTITLE);
	if (SendMessage(hSwitchWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->SetSwitchWindowTitle(TRUE);
	} else {
		pProperty->SetSwitchWindowTitle(FALSE);
	}
#endif

	HWND hDisableSaveDlg = GetDlgItem(hDlg, IDC_PROPTAB_DISABLESAVEDLG);
	if (SendMessage(hDisableSaveDlg, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->SetDisableSaveDlg(TRUE);
	} else {
		pProperty->SetDisableSaveDlg(FALSE);
	}

	return TRUE;
}

//////////////////////////////////////////
// Action buttons
//////////////////////////////////////////

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
static DlgMsgRes aAppButtonRes[] = {
	{ IDC_PROP_APPBUTTON1,       MSG_ID_DLG_PROPTAB_APPBTN_BUTTON1 }, 
	{ IDC_PROP_APPBUTTON2,       MSG_ID_DLG_PROPTAB_APPBTN_BUTTON2 },
	{ IDC_PROP_APPBUTTON3,       MSG_ID_DLG_PROPTAB_APPBTN_BUTTON3 },
	{ IDC_PROP_APPBUTTON4,       MSG_ID_DLG_PROPTAB_APPBTN_BUTTON4 },
	{ IDC_PROP_APPBUTTON5,       MSG_ID_DLG_PROPTAB_APPBTN_BUTTON5 },
	{ IDC_PROPTAB_APPBTN_HELP,   MSG_ID_DLG_PROPTAB_APPBTN_HELP },
	{ IDC_PROPTAB_DISABLEACTION, MSG_ID_DLG_PROPTAB_APPBTN_DISABLE },
};

void AppButtonTab::Init(HWND hDlg)
{
	OverrideDlgMsg(hDlg, -1, aAppButtonRes, sizeof(aAppButtonRes)/sizeof(DlgMsgRes));

	HWND hAppButton1 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON1);
	HWND hAppButton2 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON2);
	HWND hAppButton3 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON3);
	HWND hAppButton4 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON4);
	HWND hAppButton5 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON5);

	HWND hDisableAppButton = GetDlgItem(hDlg, IDC_PROPTAB_DISABLEACTION);

	if (pProperty->GetAppButton1()) {
		SendMessage(hAppButton1, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hAppButton1, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	if (pProperty->GetAppButton2()) {
		SendMessage(hAppButton2, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hAppButton2, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	if (pProperty->GetAppButton3()) {
		SendMessage(hAppButton3, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hAppButton3, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	if (pProperty->GetAppButton4()) {
		SendMessage(hAppButton4, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hAppButton4, BM_SETCHECK, BST_UNCHECKED, 0);
	}
	if (pProperty->GetAppButton5()) {
		SendMessage(hAppButton5, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hAppButton5, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	if (pProperty->GetDisableExtraActionButton()) {
		SendMessage(hDisableAppButton, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hDisableAppButton, BM_SETCHECK, BST_UNCHECKED, 0);
	}
}

BOOL AppButtonTab::Apply(HWND hDlg)
{
	HWND hAppButton1 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON1);
	HWND hAppButton2 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON2);
	HWND hAppButton3 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON3);
	HWND hAppButton4 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON4);
	HWND hAppButton5 = GetDlgItem(hDlg, IDC_PROP_APPBUTTON5);
	HWND hDisableAppButton = GetDlgItem(hDlg, IDC_PROPTAB_DISABLEACTION);

	if (SendMessage(hAppButton1, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->SetAppButton1(APPBUTTON_ACTION_ENABLE);
	} else {
		pProperty->SetAppButton1(APPBUTTON_ACTION_DISABLE);
	}
	if (SendMessage(hAppButton2, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->SetAppButton2(APPBUTTON_ACTION_ENABLE);
	} else {
		pProperty->SetAppButton2(APPBUTTON_ACTION_DISABLE);
	}
	if (SendMessage(hAppButton3, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->SetAppButton3(APPBUTTON_ACTION_ENABLE);
	} else {
		pProperty->SetAppButton3(APPBUTTON_ACTION_DISABLE);
	}
	if (SendMessage(hAppButton4, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->SetAppButton4(APPBUTTON_ACTION_ENABLE);
	} else {
		pProperty->SetAppButton4(APPBUTTON_ACTION_DISABLE);
	}
	if (SendMessage(hAppButton5, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->SetAppButton5(APPBUTTON_ACTION_ENABLE);
	} else {
		pProperty->SetAppButton5(APPBUTTON_ACTION_DISABLE);
	}

	if (SendMessage(hDisableAppButton, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->SetDisableExtraActionButton(TRUE);
	} else {
		pProperty->SetDisableExtraActionButton(FALSE);
	}

	return TRUE;
}

#endif

//////////////////////////////////////////
// SIP tab
//////////////////////////////////////////

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
static DlgMsgRes aSIPRes[] = {
	{ IDC_PROP_SIPDELTA, MSG_ID_DLG_PROPTAB_SIP_SHIFT },
};

void SipTab::Init(HWND hDlg)
{
	OverrideDlgMsg(hDlg, -1, aSIPRes, sizeof(aSIPRes)/sizeof(DlgMsgRes));

	HWND hKeepDelta = GetDlgItem(hDlg, IDC_PROP_SIPDELTA);
	if (pProperty->GetSipSizeDelta() != 0) {
		SendMessage(hKeepDelta, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hKeepDelta, BM_SETCHECK, BST_UNCHECKED, 0);
	}
}

BOOL SipTab::Apply(HWND hDlg)
{
	HWND hKeepDelta = GetDlgItem(hDlg, IDC_PROP_SIPDELTA);
	if (SendMessage(hKeepDelta, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->SetSipSizeDelta(SIP_DELTA);
	} else {
		pProperty->SetSipSizeDelta(0);
	}
	return TRUE;
}

#endif

//////////////////////////////////////////
// Codepage tab
//////////////////////////////////////////

static DlgMsgRes aCPRes[] = {
	{ IDC_PROPTAB_CODEPAGE_LABEL, MSG_ID_DLG_PROPTAB_CODEPAGE_LABEL },
	{ IDC_PROPTAB_CODEPAGE_NOTICE, MSG_ID_DLG_PROPTAB_CODEPAGE_NOTICE },
};

void CodepageTab::Init(HWND hDlg)
{
	OverrideDlgMsg(hDlg, -1, aCPRes, sizeof(aCPRes)/sizeof(DlgMsgRes));

	HWND hWnd = GetDlgItem(hDlg, IDC_PROPTAB_CODEPAGE_CODEPAGE);
	SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)MSG_CODEPAGE_DEFAULT);
	SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)MSG_CODEPAGE_UTF16);
	SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)MSG_CODEPAGE_UTF8);
	SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)MSG_CODEPAGE_GREEK);
	switch (pProperty->GetCodePage()) {
	case TOMBO_CP_UTF16LE:
		SendMessage(hWnd, CB_SETCURSEL, 1, 0);
		break;
	case TOMBO_CP_UTF8:
		SendMessage(hWnd, CB_SETCURSEL, 2, 0);
		break;
	case TOMBO_CP_GREEK:
		SendMessage(hWnd, CB_SETCURSEL, 3, 0);
		break;
	default:
		SendMessage(hWnd, CB_SETCURSEL, 0, 0);
	}
}

BOOL CodepageTab::Apply(HWND hDlg)
{
	HWND hWnd = GetDlgItem(hDlg, IDC_PROPTAB_CODEPAGE_CODEPAGE);
	switch (SendMessage(hWnd, CB_GETCURSEL, 0, 0)) {
	case 0:		// default
		pProperty->SetCodePage(TOMBO_CP_DEFAULT);
		break;
	case 1:		// UTF16
		pProperty->SetCodePage(TOMBO_CP_UTF16LE);
		break;
	case 2:		// UTF8
		pProperty->SetCodePage(TOMBO_CP_UTF8);
		break;
	case 3:		// Greek
		pProperty->SetCodePage(TOMBO_CP_GREEK);
		break;
	}
	return TRUE;
}

//////////////////////////////////////////
// DefaultNote tab
//////////////////////////////////////////

static DlgMsgRes aDefNote[] = {
	{ IDC_PROPTAB_DEFNOTE_LABEL,      MSG_ID_DLG_PROPTAB_DEFNOTE_LABEL },
	{ IDC_PROPTAB_DEFNOTE_SETCURRENT, MSG_ID_DLG_PROPTAB_DEFNOTE_CURRENT },
	{ IDC_PROPTAB_DEFNOTE_SETBLANK,   MSG_ID_DLG_PROPTAB_BLANK },
	{ IDC_PROPTAB_DEFNOTE_USELAST,	  MSG_ID_DLG_PROPTAB_DEFNOTE_LAST_USE },
};

void DefaultNoteTab::Init(HWND hDlg)
{
	OverrideDlgMsg(hDlg, -1, aDefNote, sizeof(aDefNote)/sizeof(DlgMsgRes));
	HWND hLastOpen = GetDlgItem(hDlg, IDC_PROPTAB_DEFNOTE_USELAST);
	SetCheckBox(hLastOpen, g_Property.GetKeepLastOpen());

	HWND hPath = GetDlgItem(hDlg, IDC_PROPTAB_DEFNOTE_PATH);
	SetWindowText(hPath, g_Property.GetDefaultNote());

	HWND hButton1 = GetDlgItem(hDlg, IDC_PROPTAB_DEFNOTE_SETCURRENT);
	HWND hButton2 = GetDlgItem(hDlg, IDC_PROPTAB_DEFNOTE_SETBLANK);
	BOOL bEnable = !g_Property.GetKeepLastOpen();
	EnableWindow(hPath, bEnable);
	EnableWindow(hButton1, bEnable);
	EnableWindow(hButton2, bEnable);
}

BOOL DefaultNoteTab::Apply(HWND hDlg)
{
	HWND hPath = GetDlgItem(hDlg, IDC_PROPTAB_DEFNOTE_PATH);
	TCHAR buf[MAX_PATH];
	GetWindowText(hPath, buf, MAX_PATH);
	g_Property.SetKeepLastOpen(GetCheckBox(GetDlgItem(hDlg, IDC_PROPTAB_DEFNOTE_USELAST)));
	return g_Property.SetDefaultNote(buf);
}

BOOL DefaultNoteTab::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam)) {
	case IDC_PROPTAB_DEFNOTE_SETCURRENT:
		SetCurrent(hDlg);
		break;
	case IDC_PROPTAB_DEFNOTE_SETBLANK:
		SetBlank(hDlg);
		break;
	case IDC_PROPTAB_DEFNOTE_USELAST:
		{
			HWND hPath = GetDlgItem(hDlg, IDC_PROPTAB_DEFNOTE_PATH);
			HWND hButton1 = GetDlgItem(hDlg, IDC_PROPTAB_DEFNOTE_SETCURRENT);
			HWND hButton2 = GetDlgItem(hDlg, IDC_PROPTAB_DEFNOTE_SETBLANK);
			DWORD bEnable = !GetCheckBox(GetDlgItem(hDlg, IDC_PROPTAB_DEFNOTE_USELAST));
			EnableWindow(hPath, bEnable);
			EnableWindow(hButton1, bEnable);
			EnableWindow(hButton2, bEnable);
		}
		break;
	}
	return TRUE;
}

void DefaultNoteTab::SetBlank(HWND hDlg)
{
	HWND hPath = GetDlgItem(hDlg, IDC_PROPTAB_DEFNOTE_PATH);
	SetWindowText(hPath, TEXT(""));
}

void DefaultNoteTab::SetCurrent(HWND hDlg)
{
	HWND hPath = GetDlgItem(hDlg, IDC_PROPTAB_DEFNOTE_PATH);
	if (pCurrentPath) {
		SetWindowText(hPath, pCurrentPath);
	} else {
		SetWindowText(hPath, TEXT(""));
	}
}

//////////////////////////////////////////
// External application tab
//////////////////////////////////////////

#if !defined(PLATFORM_PSPC)
static DlgMsgRes aExtApp[] = {
	{ IDC_PROP_EXTAPP_USEASSOC,    MSG_ID_DLG_EXTAPP_USEASSOC },
	{ IDC_PROP_EXTAPP_TTL_EXTAPP1, MSG_ID_DLG_EXTAPP_LBL_APP1 },
	{ IDC_PROP_EXTAPP_TTL_EXTAPP2, MSG_ID_DLG_EXTAPP_LBL_APP2 },
	{ IDC_PROP_EXTAPP_CAUTION,     MSG_ID_DLG_EXTAPP_CAUTION },
};

void ExtAppTab::Init(HWND hDlg)
{
	OverrideDlgMsg(hDlg, -1, aExtApp, sizeof(aExtApp)/sizeof(DlgMsgRes));

	HWND hUseAssoc = GetDlgItem(hDlg, IDC_PROP_EXTAPP_USEASSOC);
	if (pProperty->GetUseAssociation()) {
		SendMessage(hUseAssoc, BM_SETCHECK, BST_CHECKED, 0);
	} else {
		SendMessage(hUseAssoc, BM_SETCHECK, BST_UNCHECKED, 0);
	}

	SetWindowText(GetDlgItem(hDlg, IDC_PROP_EXTAPP_PATH_EXTAPP1), pProperty->GetExtApp1());
	SetWindowText(GetDlgItem(hDlg, IDC_PROP_EXTAPP_PATH_EXTAPP2), pProperty->GetExtApp2());
}

BOOL ExtAppTab::Apply(HWND hDlg)
{
	HWND hWnd = GetDlgItem(hDlg, IDC_PROP_EXTAPP_USEASSOC);
	if (SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pProperty->SetUseAssociation(TRUE);
	} else {
		pProperty->SetUseAssociation(FALSE);
	}
	HWND hAp1 = GetDlgItem(hDlg, IDC_PROP_EXTAPP_PATH_EXTAPP1);
	HWND hAp2 = GetDlgItem(hDlg, IDC_PROP_EXTAPP_PATH_EXTAPP2);
	DWORD n1 = GetWindowTextLength(hAp1);
	DWORD n2 = GetWindowTextLength(hAp2);
	DWORD n = (n1 > n2 ? n1 : n2) + 1;
	TString s;
	if (!s.Alloc(n)) return FALSE;
	GetWindowText(hAp1, s.Get(), n);
	if (!pProperty->SetExtApp1(s.Get())) return FALSE;
	GetWindowText(hAp2, s.Get(), n);
	if (!pProperty->SetExtApp2(s.Get())) return FALSE;
	return TRUE;
}

BOOL ExtAppTab::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam)) {
	case IDC_PROPTAB_EXTAPP_CHOOSE1:
		Choose1(hDlg);
		break;
	case IDC_PROPTAB_EXTAPP_CHOOSE2:
		Choose2(hDlg);
		break;
	}
	return TRUE;
}

void ExtAppTab::Choose1(HWND hDlg)
{
	FileSelector sel;
#if defined(PLATFORM_WIN32)
	LPCTSTR pExt = MSG_DLG_EXTAPP_CHOOSE_EXT;
#else
	LPCTSTR pExt = TEXT("*.exe");
#endif
	if (sel.Popup(g_hInstance, hDlg, MSG_DLG_EXTAPP_CHOOSE_TTL, pExt) == IDOK) {
		HWND hWnd = GetDlgItem(hDlg, IDC_PROP_EXTAPP_PATH_EXTAPP1);
		SetWindowText(hWnd, sel.SelectedPath());
	}
}

void ExtAppTab::Choose2(HWND hDlg)
{
	FileSelector sel;
#if defined(PLATFORM_WIN32)
	LPCTSTR pExt = MSG_DLG_EXTAPP_CHOOSE_EXT;
#else
	LPCTSTR pExt = TEXT("*.exe");
#endif

	if (sel.Popup(g_hInstance, hDlg, MSG_DLG_EXTAPP_CHOOSE_TTL, pExt) == IDOK) {
		HWND hWnd = GetDlgItem(hDlg, IDC_PROP_EXTAPP_PATH_EXTAPP2);
		SetWindowText(hWnd, sel.SelectedPath());
	}
}
#endif

//////////////////////////////////////////
// helper functions
//////////////////////////////////////////

// set font name to combobox
//
// callback handler called from EnumFontFamilies
// set window handle to LPARAM

int CALLBACK PropEnumFonts(ENUMLOGFONT FAR *pFont, NEWTEXTMETRIC FAR *pMetric, int iFontType, LPARAM lParam)
{
	HWND hWnd = (HWND)lParam;

	LPTSTR pFace = pFont->elfLogFont.lfFaceName;
	if (*pFace == TEXT('@')) return TRUE; // disable font for vertical
	// if (pMetric->tmPitchAndFamily & 0x1) return TRUE;

	SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)pFace);
	return TRUE;
}

// create directory only if it not exists
static BOOL CheckCreateDir(LPCTSTR pDir)
{
	WIN32_FIND_DATA wfd;
	HANDLE h = FindFirstFile(pDir, &wfd);
	if (h == INVALID_HANDLE_VALUE) {
		return CreateDirectory(pDir, NULL);
	}
	FindClose(h);
	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		return TRUE;
	} else {
		SetLastError(ERROR_FILE_EXISTS);
		return FALSE;
	}
}

// create directory recursively
static BOOL CreateDirectories(LPCTSTR pDir)
{
	LPCTSTR p = pDir;
	TCHAR aPath[MAX_PATH];
	LPTSTR q = aPath;

#if defined(PLATFORM_WIN32)
	if (isalpha(*p) && *(p+1) == ':' && *(p+2) == '\\') {
		// D:\Path
		*q++ = *p++;
		*q++ = *p++;
		*q++ = *p++;
	} else if (*p == '\\' && *(p+1) == '\\') {
		// \\server\share\Path
		*q++ = *p++;
		*q++ = *p++;

		// server\ 
		while(*p) {
			if (*p == '\\') {
				*q++ = *p++;
				break;
			}
			if (IsDBCSLeadByte(*p)) {
				*q++ = *p++;
			}
			*q++ = *p++;
		}
		// share\ 
		while(*p) {
			if (*p == '\\') {
				*q++ = *p++;
				break;
			}
			if (IsDBCSLeadByte(*p)) {
				*q++ = *p++;
			}
			*q++ = *p++;
		}
	} else {
		return FALSE;
	}
#else
	// \\ 
	if (*p != TEXT('\\')) {
		return FALSE;
	}
	*q++ = *p++;
#endif

	while(*p) {
		if (*p == TEXT('\\')) {
			*q = TEXT('\0');
			if (!CheckCreateDir(aPath)) return FALSE;
		}
		*q++ = *p++;
	}
	*q = TEXT('\0');
	return CheckCreateDir(aPath);
}
