#include <windows.h>
#include <tchar.h>
#include <commctrl.h>

#include "Tombo.h"
#include "Property.h"
#include "resource.h"
#include "SearchDlg.h"
#include "UniConv.h"
#include "SipControl.h"

#include "DialogTemplate.h"
#include "Message.h"

////////////////////////////////////////////////////////////////
// ctor & dtor
////////////////////////////////////////////////////////////////

SearchDialog::~SearchDialog()
{
	if (pSearchStr) delete [] pSearchStr;
}


////////////////////////////////////////////////////////////////
// Dialog proc
////////////////////////////////////////////////////////////////

static BOOL APIENTRY DlgProc(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	SearchDialog *pDlg;
	if (nMessage == WM_INITDIALOG) {
		SetWindowLong(hDlg, DWL_USER, lParam);
		pDlg = (SearchDialog*)lParam;

		pDlg->InitDialog(hDlg);
		return TRUE;
	}

	pDlg = (SearchDialog*)GetWindowLong(hDlg, DWL_USER);
	if (pDlg == NULL) return FALSE;

	switch (nMessage) {
	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			if (pDlg->OnOK()) {
				EndDialog(hDlg, IDOK);
			}
			break;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			break;
		}
		return TRUE;
	}
	return FALSE;

}

////////////////////////////////////////////////////////////////
// init
////////////////////////////////////////////////////////////////

static DlgMsgRes aMsgRes[] = {
	{ IDC_SEARCH_FIND_LABEL,     MSG_ID_DLG_SEARCH_FIND_LABEL },
	{ IDC_SEARCH_DIRECT_LABEL,   MSG_ID_DLG_SEARCH_DIRECTION_LABEL },
	{ IDC_SEARCH_DIRECTION_UP,   MSG_ID_DLG_SEARCH_DIRECTION_UP },
	{ IDC_SEARCH_DIRECTION_DOWN, MSG_ID_DLG_SEARCH_DIRECTION_DOWN },
	{ IDC_SEARCH_CASESENSITIVE,  MSG_ID_DLG_FILTERDEF_ADD_REGEX_CASESENSITIVE },
	{ IDC_SEARCH_ENCRYPTMEMO,    MSG_ID_DLG_FILTERDEF_ADD_REGEX_INCLUDECRYPTED },
	{ IDC_FILENAMEONLY,          MSG_ID_DLG_FILTERDEF_ADD_REGEX_FORFILENAME },
	{ IDOK,                      MSG_ID_DLG_CMN_OK },
	{ IDCANCEL,                  MSG_ID_DLG_CMN_CANCEL },
};

void SearchDialog::InitDialog(HWND hDlg)
{
	OverrideDlgMsg(hDlg, MSG_ID_DLG_SEARCH_TITLE, aMsgRes, sizeof(aMsgRes)/sizeof(DlgMsgRes));
	hDialog = hDlg;

	HWND hCombo = GetDlgItem(hDlg, IDC_SEARCH_STRING);
	LPCTSTR pHist = g_Property.GetSearchHist();
	SetHistoryToComboBox(hCombo, pHist);

	HWND hCheckEncrypt = GetDlgItem(hDlg, IDC_SEARCH_ENCRYPTMEMO);
	EnableWindow(hCheckEncrypt, bCheckEncrypt);
	HWND hFileNameOnly = GetDlgItem(hDlg, IDC_FILENAMEONLY);
	EnableWindow(hFileNameOnly, bFileNameOnly);
	
	HWND hSearchDirectionUp = GetDlgItem(hDlg, IDC_SEARCH_DIRECTION_UP);
	HWND hSearchDirectionDown = GetDlgItem(hDlg, IDC_SEARCH_DIRECTION_DOWN);
	EnableWindow(hSearchDirectionUp, bEnableDirection);
	EnableWindow(hSearchDirectionDown, bEnableDirection);

	CheckRadioButton(hDlg, IDC_SEARCH_DIRECTION_UP, IDC_SEARCH_DIRECTION_DOWN, IDC_SEARCH_DIRECTION_DOWN);
}

////////////////////////////////////////////////////////////////
// popup
////////////////////////////////////////////////////////////////

DWORD SearchDialog::Popup(HINSTANCE hInst, HWND hParent, BOOL bCE)
{
	hInstance = hInst;

	bCheckEncrypt = bFileNameOnly = bCE;
	bEnableDirection = bCE;

	SipControl sc;
	BOOL bSipStat, bResult;
		bResult = sc.Init() && sc.GetSipStat(&bSipStat);
	if (bResult) sc.SetSipStat(TRUE);

	DWORD result;
	result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SEARCH), 
								hParent, (DLGPROC)DlgProc, (LONG)this);

	if (bResult) sc.SetSipStat(bSipStat);
	return result;
}

////////////////////////////////////////////////////////////////
// OK
////////////////////////////////////////////////////////////////

BOOL SearchDialog::OnOK()
{
	// find encrypted notes
	HWND hCheckEncrypt = GetDlgItem(hDialog, IDC_SEARCH_ENCRYPTMEMO);
	bCheckEncrypt = (SendMessage(hCheckEncrypt, BM_GETCHECK, 0, 0) == BST_CHECKED);
	
	// case sensitive
	HWND hCaseSensitive = GetDlgItem(hDialog, IDC_SEARCH_CASESENSITIVE);
	bCaseSensitive = (SendMessage(hCaseSensitive, BM_GETCHECK, 0, 0) == BST_CHECKED);

	HWND hFileNameOnly = GetDlgItem(hDialog, IDC_FILENAMEONLY);
	bFileNameOnly = (SendMessage(hFileNameOnly, BM_GETCHECK, 0, 0) == BST_CHECKED);

	// Search direction
	HWND hSearchDirectionUp = GetDlgItem(hDialog, IDC_SEARCH_DIRECTION_UP);
	bSearchDirectionUp = (SendMessage(hSearchDirectionUp, BM_GETCHECK, 0, 0) == BST_CHECKED);

	// find string
	HWND hSearchWord = GetDlgItem(hDialog, IDC_SEARCH_STRING);
	int n = GetWindowTextLength(hSearchWord);

	// ‰½‚à“ü‚ê‚Ä‚¢‚È‚¯‚ê‚Î–³Ž‹
	if (n == 0) return FALSE;

	LPTSTR pSearchTextW = new TCHAR[n + 2];
	if (pSearchTextW == NULL) return FALSE;
	GetWindowText(hSearchWord, pSearchTextW, n + 1);
	
	g_Property.SetSearchHist(GetHistoryFromComboBox(hSearchWord, pSearchTextW, NUM_SEARCH_HISTORY));
	pSearchStr = pSearchTextW;
	return TRUE;
}