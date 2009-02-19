#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include "NewFolderDialog.h"
#include "resource.h"
#include "SipControl.h"
#include "Message.h"
#include "DialogTemplate.h"

NewFolderDialog::NewFolderDialog() : 
	hDialog(NULL), hInstance(NULL), pBaseText(NULL), 
	nTitleID(MSG_ID_DLG_NEWFOLDER_TITLE)
{
	aFolder[0] = TEXT('\0'); 
}

//////////////////////////////////////////////////////////
// Dialog procedure
//////////////////////////////////////////////////////////

static BOOL APIENTRY DlgProc(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	NewFolderDialog *pDlg;
	if (nMessage == WM_INITDIALOG) {
		SetWindowLong(hDlg, DWL_USER, lParam);
		pDlg = (NewFolderDialog*)lParam;

		pDlg->InitDialog(hDlg);
		return TRUE;
	}

	pDlg = (NewFolderDialog*)GetWindowLong(hDlg, DWL_USER);
	if (pDlg == NULL) return FALSE;

	switch (nMessage) {
	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			if (pDlg->OnOK(hDlg)) {
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

//////////////////////////////////////////////////////////
// popup dialog
//////////////////////////////////////////////////////////

DWORD NewFolderDialog::Popup(HINSTANCE hInst, HWND hParent)
{
	hInstance = hInst;

	SipControl sc;
	BOOL bSipStat, bResult;

	bResult = sc.Init() && sc.GetSipStat(&bSipStat);
	if (bResult) sc.SetSipStat(TRUE);
	
	DWORD result;
	result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_NEWFOLDER), 
								hParent, (DLGPROC)DlgProc, (LONG)this);

	if (bResult) sc.SetSipStat(bSipStat);

	return result;
}

//////////////////////////////////////////////////////////
// Initialize
//////////////////////////////////////////////////////////

static DlgMsgRes aDlgRes[] = {
	{ IDOK,     MSG_ID_DLG_CMN_OK },
	{ IDCANCEL, MSG_ID_DLG_CMN_CANCEL },
};

void NewFolderDialog::InitDialog(HWND hDlg)
{
	OverrideDlgMsg(hDlg, nTitleID, aDlgRes, sizeof(aDlgRes)/sizeof(DlgMsgRes));

	HWND hEdit = GetDlgItem(hDlg, IDC_NEWFOLDER_NAME);

	if (pBaseText == NULL) {
		pBaseText = TEXT("");
	}
	SetWindowText(hEdit, pBaseText);
}

//////////////////////////////////////////////////////////
// OK
//////////////////////////////////////////////////////////

BOOL NewFolderDialog::OnOK(HWND hDlg)
{
	HWND hEdit = GetDlgItem(hDlg, IDC_NEWFOLDER_NAME);
	DWORD n = GetWindowTextLength(hEdit);
	if (n == 0) return FALSE;

	GetWindowText(hEdit, aFolder, n + 1);
	aFolder[n] = TEXT('\0');
	return TRUE;
}

void NewFolderDialog::SetOption(DWORD nID, LPCTSTR pText)
{
	nTitleID = nID;
	pBaseText = pText;
}