#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include "PropertyPage.h"

#define RESMSG(x) (GetString(x))

LPCTSTR GetString(UINT nID);

// from http://sourceforge.jp/forum/message.php?msg_id=11376
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
#include <aygshell.h>
static
int CALLBACK PropSheetProc(HWND hDlg, UINT uMsg, LPARAM lParam)
{
	if(uMsg == PSCB_INITIALIZED) {
		// empty menubar
		SHMENUBARINFO mbi;
		memset(&mbi, 0, sizeof(SHMENUBARINFO));
		mbi.cbSize = sizeof(SHMENUBARINFO);
		mbi.hwndParent = hDlg;
		mbi.dwFlags	= SHCMBF_EMPTYBAR;
		SHCreateMenuBar(&mbi);

		// tab-control move to bottom 
		HWND hTab = GetDlgItem( hDlg, 0x3020);	//TabControl
		DWORD style = GetWindowLong( hTab, GWL_STYLE ) | TCS_BOTTOM; 
		SetWindowLong( hTab, GWL_STYLE, style );
		return 0;
	}
	if(uMsg == PSCB_GETVERSION) {
		return COMCTL32_VERSION;
	}
	return 0;
}
#endif

/////////////////////////////////////////////////////
// PropertyPage popup
/////////////////////////////////////////////////////

DWORD PropertyPage::Popup(HINSTANCE hInst, HWND hWnd, PPropertyTab *ppPage, DWORD nPage, LPCTSTR pTitle, LPTSTR pIcon, DWORD nStartPage)
{
	PROPSHEETPAGE *pPsp;
    PROPSHEETHEADER psh;
	DWORD i;
	pPsp = new PROPSHEETPAGE[nPage];
	if (pPsp == NULL) return IDCANCEL;

	for (i = 0; i < nPage; i++) {
	    pPsp[i].dwSize = sizeof(PROPSHEETPAGE);
	    pPsp[i].dwFlags = PSP_USETITLE;
	    pPsp[i].hInstance = hInst;
	    pPsp[i].pszTemplate = MAKEINTRESOURCE(ppPage[i]->ResourceID());
	    pPsp[i].pfnDlgProc = ppPage[i]->DialogProc();
	    pPsp[i].pszTitle = ppPage[i]->Title();
		pPsp[i].lParam = (LONG)ppPage[i];
	}

    psh.dwSize = sizeof(PROPSHEETHEADER);

	// from http://sourceforge.jp/forum/message.php?msg_id=11376
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_MAXIMIZE | PSH_USECALLBACK;
    psh.pfnCallback = PropSheetProc;
#else
    psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
#endif
    psh.hwndParent = hWnd;
    psh.hInstance = hInst;
    psh.pszIcon = pIcon;
    psh.pszCaption = pTitle;
    psh.nPages = nPage;
    psh.nStartPage = nStartPage;
    psh.ppsp = (LPCPROPSHEETPAGE) pPsp;
    psh.pfnCallback = NULL;

	DWORD nResult = PropertySheet(&psh);
	delete pPsp;
	return nResult;
}

/////////////////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////////////////

PropertyTab::PropertyTab(DWORD id, DWORD nTitleResID, DLGPROC proc)
 : nResourceID(id), pDlgProc(proc) 
{
	_tcscpy(aTitle, RESMSG(nTitleResID));
}

PropertyTab::PropertyTab(DWORD id, LPCTSTR pTitle, DLGPROC proc)
 : nResourceID(id), pDlgProc(proc) 
{
	_tcscpy(aTitle, pTitle);
}

/////////////////////////////////////////////////////
// Default dialog procedure
/////////////////////////////////////////////////////

BOOL APIENTRY PropertyTab::DefaultPageProc(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	PropertyTab *pPage;

	if (nMessage == WM_INITDIALOG) {
		PROPSHEETPAGE *ps = (PROPSHEETPAGE*)lParam;
		pPage = (PropertyTab*)ps->lParam;
		SetWindowLong(hDlg, DWL_USER, ps->lParam);

		pPage->Init(hDlg);
		return TRUE;
	}

	pPage = (PropertyTab*)GetWindowLong(hDlg, DWL_USER);
	if (pPage == NULL) return FALSE;

	switch(nMessage) {
	case WM_COMMAND:
		return pPage->OnCommand(hDlg, wParam, lParam);

	case WM_NOTIFY:
		switch (((NMHDR FAR *) lParam)->code) {
        case PSN_APPLY:
			if (pPage->Apply(hDlg)) {
				SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
			} else {
				SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
			}
			return TRUE;
		case PSN_KILLACTIVE:
			if (pPage->Apply(hDlg)) {
				SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
			} else {
				SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
			}
			return TRUE;
		case PSN_RESET:
			pPage->Cancel(hDlg, wParam, lParam);
			return TRUE;
		default:
			pPage->OnNotify(hDlg, wParam, lParam);
		}
	}
	return FALSE;
}
