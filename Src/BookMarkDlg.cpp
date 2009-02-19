#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "DialogTemplate.h"
#include "TString.h"
#include "resource.h"
#include "Message.h"
#include "VarBuffer.h"
#include "BookMarkDlg.h"
#include "BookMark.h"

static BOOL GetLVItem(HWND hList, int iItem, int iSubItem, TString *pBuf);
static void LVSetSel(HWND hList, int iItem, BOOL bSet);

////////////////////////////////////////////////////////////////
// ctor & dtor
////////////////////////////////////////////////////////////////

BOOL BookMarkDlg::Init(BookMark *p)
{
	pBookMark = p;
	return TRUE;
}

static DlgMsgRes aMsgRes[] = {
	{ IDC_BMEDIT_UP,     MSG_ID_DLG_BMEDIT_UP },
	{ IDC_BMEDIT_DOWN,   MSG_ID_DLG_BMEDIT_DOWN },
	{ IDC_BMEDIT_DELETE, MSG_ID_DLG_BMEDIT_DELETE },
	{ IDOK,              MSG_ID_DLG_CMN_OK },
	{ IDCANCEL,          MSG_ID_DLG_CMN_CANCEL },
};

////////////////////////////////////////////////////////////////
// Initialize dialog
////////////////////////////////////////////////////////////////

void BookMarkDlg::InitDialog(HWND hDlg)
{
	OverrideDlgMsg(hDlg, MSG_ID_DLG_BMEDIT_TITLE, aMsgRes, sizeof(aMsgRes) / sizeof(DlgMsgRes));

	HWND hList = GetDlgItem(hDlg, IDC_BMEDIT_LIST);

	// init listview
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = LVCFMT_LEFT;

	lvc.cx = 100;
	lvc.pszText = (LPTSTR)MSG_BMDLG_TITLE_NAME;
	ListView_InsertColumn(hList, 0, &lvc);

	lvc.cx = 150;
	lvc.pszText = (LPTSTR)MSG_BMDLG_TITLE_PATH;
	ListView_InsertColumn(hList, 1, &lvc);
	
	// Insert item
	DWORD n = pBookMark->NumItems();
	for (DWORD i = 0; i < n; i++) {
		const BookMarkItem *pItem = pBookMark->GetUnit(i);
		InsertItem(hList, i, pItem);
	}
}

BOOL BookMarkDlg::InsertItem(HWND hList, DWORD iPos, const BookMarkItem *pItem)
{
	LV_ITEM li;
	li.iItem = iPos;
	li.mask = LVIF_TEXT;
	li.pszText = pItem->pName;
	li.cchTextMax = _tcslen(pItem->pName);
	li.iSubItem = 0;
	ListView_InsertItem(hList, &li);

	li.mask = LVIF_TEXT;
	li.pszText = pItem->pPath;
	li.cchTextMax = _tcslen(pItem->pPath);
	li.iSubItem = 1;
	ListView_SetItem(hList, &li);
	return TRUE;
}

////////////////////////////////////////////////////////////////
// OK button
////////////////////////////////////////////////////////////////

BOOL BookMarkDlg::OnOK()
{
	pBookMark->Clear();

	HWND hList = GetDlgItem(hDialog, IDC_BMEDIT_LIST);
	int n = ListView_GetItemCount(hList);

	TString sPath;
	for (int i = 0; i < n; i++) {
		if (!GetLVItem(hList, i, 1, &sPath)) return TRUE;
		pBookMark->Assign(sPath.Get());
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////
// Command handler
////////////////////////////////////////////////////////////////

BOOL BookMarkDlg::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam)) {
	case IDC_BMEDIT_UP:
		return Command_UpDown(hDlg, -1);
	case IDC_BMEDIT_DOWN:
		return Command_UpDown(hDlg, 1);
	case IDC_BMEDIT_DELETE:
		return Command_Delete(hDlg);
	}
	return TRUE;
}


////////////////////////////////////////////////////////////////
// swap position
////////////////////////////////////////////////////////////////

BOOL BookMarkDlg::Command_UpDown(HWND hDlg, int iDelta)
{
	HWND hList = GetDlgItem(hDlg, IDC_BMEDIT_LIST);
	int iSel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

	if (iSel + iDelta < 0 || iSel + iDelta >= (int)pBookMark->NumItems()) return TRUE;

	TString sName1, sPath1;
	TString sName2, sPath2;

	// get data
	if (!GetLVItem(hList, iSel, 0, &sName1) ||
		!GetLVItem(hList, iSel, 1, &sPath1) ||
		!GetLVItem(hList, iSel + iDelta, 0, &sName2) ||
		!GetLVItem(hList, iSel + iDelta, 1, &sPath2)) return TRUE;

	// swap and set data
	ListView_SetItemText(hList, iSel, 0, sName2.Get());
	ListView_SetItemText(hList, iSel, 1, sPath2.Get());
	ListView_SetItemText(hList, iSel + iDelta, 0, sName1.Get());
	ListView_SetItemText(hList, iSel + iDelta, 1, sPath1.Get());

	// set selection info
	LVSetSel(hList, iSel, FALSE);
	LVSetSel(hList, iSel + iDelta, TRUE);

	SetFocus(hList);
	return TRUE;
}

////////////////////////////////////////////////////////////////
// delete item
////////////////////////////////////////////////////////////////

BOOL BookMarkDlg::Command_Delete(HWND hDlg)
{
	// confirm delete
	if (MessageBox(hDlg, MSG_CONFIRM, MSG_DEL_BOOKMARK, MB_YESNO | MB_ICONQUESTION) != IDYES) return TRUE;

	HWND hList = GetDlgItem(hDlg, IDC_BMEDIT_LIST);
	int iSel;

	while(1) {
		iSel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
		if (iSel == -1) break;
		ListView_DeleteItem(hList, iSel);
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////
// helper functions
////////////////////////////////////////////////////////////////

static BOOL GetLVItem(HWND hList, int iItem, int iSubItem, TString *pBuf)
{
	TCHAR buf[MAX_PATH * 2];

	LVITEM li;
	li.mask = LVIF_TEXT;
	li.iItem = iItem;
	li.iSubItem = iSubItem;
	li.pszText = buf;
	li.cchTextMax = MAX_PATH*2;
	ListView_GetItem(hList, &li);

	return pBuf->Set(buf);
}

static void LVSetSel(HWND hList, int iItem, BOOL bSet)
{
	LVITEM li;
	li.mask = LVIF_STATE;
	li.iItem = iItem;
	li.iSubItem = 0;
	li.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
	if (bSet) {
		li.state = LVIS_SELECTED | LVIS_FOCUSED;
	} else {
		li.state = 0;
	}
	ListView_SetItem(hList, &li);
}
