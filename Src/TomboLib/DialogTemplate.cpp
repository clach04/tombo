#include <windows.h>
#include <tchar.h>
#include "DialogTemplate.h"
#include "Message.h"

namespace Tombo_Lib {

////////////////////////////////////////////////////////////////
// Dialog Procedure(Template)
////////////////////////////////////////////////////////////////

static BOOL APIENTRY DlgProcTemplate(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	DialogTemplate *pDlg;
	if (nMessage == WM_INITDIALOG) {
		SetWindowLong(hDlg, DWL_USER, lParam);
		pDlg = (DialogTemplate*)lParam;

		pDlg->SetDialog(hDlg);
		pDlg->InitDialog(hDlg);
		return TRUE;
	}

	pDlg = (DialogTemplate*)GetWindowLong(hDlg, DWL_USER);
	if (pDlg == NULL) return FALSE;

	switch (nMessage) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			pDlg->SetResult(IDOK);
			if (pDlg->OnOK()) {
				EndDialog(hDlg, pDlg->GetResult());
			}
			break;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			break;
		default:
			return pDlg->OnCommand(hDlg, wParam, lParam);
		}
		return TRUE;
	case WM_NOTIFY:
		return pDlg->OnNotify(hDlg, wParam, lParam);
	}
	return FALSE;

}

////////////////////////////////////////////////////////////////
// Default handler
////////////////////////////////////////////////////////////////

void DialogTemplate::InitDialog(HWND hDlg)
{
}

BOOL DialogTemplate::OnOK()
{
	return TRUE;
}

BOOL DialogTemplate::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

BOOL DialogTemplate::OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

void DialogTemplate::SetDialog(HWND hDlg)
{
	hDialog = hDlg;
}

////////////////////////////////////////////////////////////////
// Popup entry
////////////////////////////////////////////////////////////////

DWORD DialogTemplate::Popup(HINSTANCE hInst, WORD nID, HWND hParent)
{
	hInstance = hInst;
	
	DWORD result;
	result = DialogBoxParam(hInst, MAKEINTRESOURCE(nID), 
								hParent, (DLGPROC)DlgProcTemplate, (LONG)this);

	return result;
}

} // end namespace

////////////////////////////////////////////////////////////////
// i18n
////////////////////////////////////////////////////////////////

void OverrideDlgMsg(HWND hDlg, int nTitleMsg, DlgMsgRes aMsgRes[], int nMsgRes)
{
#if !defined(PLATFORM_PSPC)
#if defined(PLATFORM_WIN32)
	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT); // not need release
#endif
	if (nTitleMsg != -1) {
		SetWindowText(hDlg, g_mMsgRes.GetMsg(nTitleMsg));
	}

	HWND h;
	for (int i = 0; i < nMsgRes; i++) {
		h = GetDlgItem(hDlg, aMsgRes[i].iCtlID);
#if defined(PLATFORM_WIN32)
		SendMessage(h, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
#endif
		SetWindowText(h, g_mMsgRes.GetMsg(aMsgRes[i].iMsgID));
	}
#endif
}
