#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#include "resource.h"
#include "Tombo.h"
#include "Message.h"
#include "TString.h"
#include "UniConv.h"
#include "TreeViewItem.h"
#include "MemoSelectView.h"
#include "FilterCtlDlg.h"
#include "VFManager.h"
#include "VFStream.h"
#include "FilterDefDlg.h"
#include "DialogTemplate.h"

extern HINSTANCE g_hInstance;

static LRESULT CALLBACK FilterCtlDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////
// initialize
/////////////////////////////////////////


BOOL FilterCtlDlg::Init(MemoSelectView *p, VFManager *pMgr)
{
	pManager = pMgr;
	pView = p;
	return TRUE;
}

/////////////////////////////////////////
// entry point
/////////////////////////////////////////

DWORD FilterCtlDlg::Popup(HINSTANCE hInst, HWND hParent, HIMAGELIST hImgList)
{
	hImageList = hImgList;
	return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FILTERCTL), hParent, (DLGPROC)FilterCtlDlgProc, (LONG)this);
}

/////////////////////////////////////////
// Initialize dialog
/////////////////////////////////////////

class FilterAddListener : public VirtualFolderEnumListener {
public:
	DWORD nCount;
	LV_ITEM *pItem;
	HWND hListView;

	DWORD nNKPos;

	BOOL ProcessStream(LPCTSTR pName, BOOL bPersit, VFDirectoryGenerator *pGen, VFStore *pStore);
};

BOOL FilterAddListener::ProcessStream(LPCTSTR pName, BOOL bPersist, VFDirectoryGenerator *pGen, VFStore *pStore)
{
	// create lParam;
	VFInfo *pInfo = new VFInfo();
	if (!pInfo) return FALSE;
	
	pInfo->pName = StringDup(pName);
	if (pInfo->pName == NULL) return FALSE;
	pInfo->bPersist = bPersist;
	pInfo->pGenerator = pGen;
	pInfo->pStore = pStore;

	pItem->iItem = nCount++;
	pItem->cchTextMax = _tcslen(pName);
	pItem->pszText = (LPTSTR)pName;
	pItem->lParam = (LPARAM)pInfo;
	if (bPersist) {
		pItem->iImage = VFOLDER_IMAGE_PERSIST;
	} else {
		if (nNKPos == 0xFFFFFFFF) {
			nNKPos = nCount - 1;
		}
		pItem->iImage = VFOLDER_IMAGE_TEMP;
	}
	ListView_InsertItem(hListView, pItem);
	return TRUE;
}

static DlgMsgRes aDlgMsgRes[] = {
	{ IDC_FILTERCTL_UP,     MSG_ID_DLG_FILTERCTL_UP },
	{ IDC_FILTERCTL_DOWN,   MSG_ID_DLG_FILTERCTL_DOWN },
	{ IDC_FILTERCTL_KEEP,   MSG_ID_DLG_FILTERCTL_KEEP },
	{ IDC_FILTERCTL_NEW,    MSG_ID_DLG_FILTERCTL_NEW },
	{ IDC_FILTERCTL_RENAME, MSG_ID_DLG_FILTERCTL_RENAME },
	{ IDC_FILTERCTL_DELETE, MSG_ID_DLG_FILTERCTL_DELETE },
	{ IDOK,                 MSG_ID_DLG_CMN_OK },
	{ IDCANCEL,             MSG_ID_DLG_CMN_CANCEL },
};

void FilterCtlDlg::InitDialog(HWND hDlg)
{
	OverrideDlgMsg(hDlg, MSG_ID_DLG_FILTERCTL_TITLE, aDlgMsgRes, sizeof(aDlgMsgRes)/sizeof(DlgMsgRes));

	HWND hList = GetDlgItem(hDlg, IDC_FILTERCTL_LIST);

	ListView_SetImageList(hList, hImageList, LVSIL_SMALL);

	/////////////////////////////////////////
	// Insert column headers

	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 200;
	lvc.pszText = (LPTSTR)MSG_VFOLDER_NAME_TTL;
	ListView_InsertColumn(hList, 0, &lvc);

	/////////////////////////////////////////
	// Insert current items

	LVITEM li;
	li.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	li.iSubItem = 0;
	DWORD i = 0;

	FilterAddListener fal;
	fal.nCount = 0;
	fal.hListView = hList;
	fal.pItem = &li;
	fal.nNKPos = 0xFFFFFFFF;

	pManager->Enum(&fal);
	if (fal.nNKPos == 0xFFFFFFFF) {
		nNotKeepPos = fal.nCount;
	} else {
		nNotKeepPos = fal.nNKPos;
	}

	HWND hSave = GetDlgItem(hDlg, IDC_FILTERCTL_KEEP);
	HWND hUp = GetDlgItem(hDlg, IDC_FILTERCTL_UP);
	HWND hDown = GetDlgItem(hDlg, IDC_FILTERCTL_DOWN);

	EnableWindow(hSave, FALSE);
	EnableWindow(hUp, FALSE);
	EnableWindow(hDown, FALSE);
}

/////////////////////////////////////////
// destroy dialog
/////////////////////////////////////////

void FilterCtlDlg::DestroyDialog(HWND hDlg, int iResult)
{
	// prevent removing imagelist
	HWND hList = GetDlgItem(hDlg, IDC_FILTERCTL_LIST);
	ListView_SetImageList(hList, NULL, LVSIL_SMALL);

	// release items
	int n = ListView_GetItemCount(hList);
	int i;
	LV_ITEM li;
	VFInfo *pInfo;
	li.mask = LVIF_PARAM;
	for (i = 0; i < n; i++) {
		li.iItem = i;
		ListView_GetItem(hList, &li);
		pInfo = (VFInfo*)li.lParam;
		pInfo->Release();
		delete pInfo;
	}

	EndDialog(hDlg, iResult);
}

/////////////////////////////////////////
// Update virtual folder definitions
/////////////////////////////////////////

BOOL FilterCtlDlg::OnOK(HWND hDlg)
{
	HWND hList = GetDlgItem(hDlg, IDC_FILTERCTL_LIST);

	int n = ListView_GetItemCount(hList);
	VFInfo **ppInfoArray = new VFInfo*[n];
	if (!ppInfoArray) return TRUE;

	int i;
	LV_ITEM li;
	VFInfo *pInfo;
	li.mask = LVIF_PARAM;
	for (i = 0; i < n; i++) {
		li.iItem = i;
		ListView_GetItem(hList, &li);
		pInfo = (VFInfo*)li.lParam;
		ppInfoArray[i] = pInfo;
	}
	pManager->UpdateVirtualFolders(ppInfoArray, n);
	return TRUE;
}

/////////////////////////////////////////
// Delete node from listview
/////////////////////////////////////////

void FilterCtlDlg::DeleteSelectedItem(HWND hDlg)
{
	HWND hList = GetDlgItem(hDlg, IDC_FILTERCTL_LIST);

	int iSel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	if (iSel < 0 ) return;

	if (MessageBox(hDlg, MSG_CONFIRM_DEL_VFOLDER, MSG_DEL_VFOLDER_TTL, MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	LVITEM li;
	li.mask = LVIF_PARAM;
	li.iItem = iSel;
	li.iSubItem = 0;
	ListView_GetItem(hList, &li);
	VFInfo *pInfo = (VFInfo*)li.lParam;

	pInfo->Release();
	delete pInfo;
	ListView_DeleteItem(hList, iSel);
}

/////////////////////////////////////////
// "Save" checkbox
/////////////////////////////////////////

void FilterCtlDlg::Command_ToggleKeep(HWND hDlg)
{
	HWND hList = GetDlgItem(hDlg, IDC_FILTERCTL_LIST);
	HWND hSave = GetDlgItem(hDlg, IDC_FILTERCTL_KEEP);


	int iSel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	LVITEM li;
	li.mask = LVIF_PARAM;
	li.iItem = iSel;
	li.iSubItem = 0;
	ListView_GetItem(hList, &li);
	VFInfo *pInfo = (VFInfo*)li.lParam;

	li.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
	li.pszText = pInfo->pName;
	li.cchTextMax = _tcslen(pInfo->pName);
	li.lParam = (LPARAM)pInfo;
	li.state = 	LVIS_FOCUSED | LVIS_SELECTED;

	if (SendMessage(hSave, BM_GETCHECK, 0, 0) == BST_CHECKED) {
		pInfo->bPersist = TRUE;
	
		ListView_DeleteItem(hList, iSel);
		li.iImage = 8;
		li.iItem = nNotKeepPos++;
	} else {
		pInfo->bPersist = FALSE;

		ListView_DeleteItem(hList, iSel);
		li.iImage = 6;
		li.iItem = --nNotKeepPos;
	}
	ListView_InsertItem(hList, &li);
}

/////////////////////////////////////////
// "Up"/"Down" button
/////////////////////////////////////////

BOOL FilterCtlDlg::Command_UpDown(HWND hDlg, int delta)
{
	HWND hList = GetDlgItem(hDlg, IDC_FILTERCTL_LIST);
	int iSel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

	if (iSel < 0) return TRUE;

	LVITEM li;
	li.mask = LVIF_STATE | LVIF_IMAGE | LVIF_PARAM;
	li.iItem = iSel;
	li.iSubItem = 0;
	ListView_GetItem(hList, &li);

	VFInfo *pInfo = (VFInfo*)li.lParam;

	ListView_DeleteItem(hList, iSel);

	li.iItem = iSel + delta;
	li.mask |= LVIF_TEXT;
	li.pszText = pInfo->pName;
	li.cchTextMax = _tcslen(pInfo->pName);
	li.state = 	LVIS_FOCUSED | LVIS_SELECTED;
	ListView_InsertItem(hList, &li);

	SetFocus(hList);
	return TRUE;
}

/////////////////////////////////////////
// "New" button
/////////////////////////////////////////

void FilterCtlDlg::Command_New(HWND hDlg)
{
	// Create empty VFInfo
	VFInfo *pInfo = new VFInfo();
	if (!pInfo ||
		!(pInfo->pGenerator = new VFDirectoryGenerator()) ||
		!pInfo->pGenerator->Init(TEXT(""), FALSE) ||
		!(pInfo->pStore = new VFStore()) ||
		!pInfo->pStore->Init()) {
		MessageBox(hDlg, MSG_NOT_ENOUGH_MEMORY, TOMBO_APP_NAME, MB_OK | MB_ICONERROR);
		if (pInfo) {
			delete pInfo->pGenerator;
			delete pInfo->pStore;

		}
		delete pInfo;
		return;
	}
	pInfo->pGenerator->SetNext(pInfo->pStore);

	// Popup dialog
	FilterDefDlg fd;
	fd.Init();
	if (fd.Popup(g_hInstance, hDlg, pInfo, TRUE) == IDOK) {
		// Assign name to virtual folder
		pInfo->pName = pManager->GetNodeName();
		if (pInfo->pName == NULL) {
			MessageBox(hDlg, MSG_NOT_ENOUGH_MEMORY, TOMBO_APP_NAME, MB_OK | MB_ICONERROR);
			pInfo->Release();
			delete pInfo;
			return;
		}
		pInfo->bPersist = TRUE;

		// Insert to list
		HWND hList = GetDlgItem(hDlg, IDC_FILTERCTL_LIST);
		LVITEM li;
		li.mask = LVIF_STATE | LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
		li.iItem = ListView_GetItemCount(hList);
		li.iSubItem = 0;
		li.state = 	LVIS_FOCUSED | LVIS_SELECTED;
		li.pszText = pInfo->pName;
		li.cchTextMax = _tcslen(pInfo->pName);
		li.lParam = (LPARAM)pInfo;
		li.iImage = VFOLDER_IMAGE_PERSIST;

		int idx = ListView_InsertItem(hList, &li);
		BOOL bX = ListView_EnsureVisible(hList, idx, TRUE);

	} else {
		pInfo->Release();
		delete pInfo;
	}

}

/////////////////////////////////////////
// "Delete" button
/////////////////////////////////////////

void FilterCtlDlg::Command_Delete(HWND hDlg)
{
	DeleteSelectedItem(hDlg);
}

/////////////////////////////////////////
// "Rename" button
/////////////////////////////////////////

void FilterCtlDlg::Command_Rename(HWND hDlg)
{
	HWND hList = GetDlgItem(hDlg, IDC_FILTERCTL_LIST);
	int iSel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

	if (iSel < 0) return;

	SetFocus(hList);
	ListView_EditLabel(hList, iSel);
}

/////////////////////////////////////////
// WM_NOTIFY handler
/////////////////////////////////////////

BOOL FilterCtlDlg::OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	NMHDR *pHdr = (NMHDR*)lParam;
	if (pHdr->code == LVN_ITEMCHANGED) {
		return Notify_ItemChanged(hDlg, lParam);
	}
	if (pHdr->code == LVN_ENDLABELEDIT) {
		return Notify_EndLabelEdit(hDlg, lParam);
	}
	if (pHdr->code == LVN_KEYDOWN) {
		return Notify_Keydown(hDlg, lParam);
	}
	if (pHdr->code == NM_DBLCLK) {
		return Notify_DblClick(hDlg, lParam);
	}
	return TRUE;
}

/////////////////////////////////////////
// LVN_ITEMCHANGED
/////////////////////////////////////////
BOOL FilterCtlDlg::Notify_ItemChanged(HWND hDlg, LPARAM lParam)
{
	NMHDR *pHdr = (NMHDR*)lParam;
	NMLISTVIEW *pLv = (NMLISTVIEW*)lParam;
	if (pLv->uNewState & LVIS_FOCUSED) {
		VFInfo *pInfo = (VFInfo*)pLv->lParam;

		HWND hSave = GetDlgItem(hDlg, IDC_FILTERCTL_KEEP);
		HWND hUp = GetDlgItem(hDlg, IDC_FILTERCTL_UP);
		HWND hDown = GetDlgItem(hDlg, IDC_FILTERCTL_DOWN);

		EnableWindow(hSave, TRUE);
		SendMessage(hSave, BM_SETCHECK, pInfo->bPersist ? BST_CHECKED : BST_UNCHECKED, 0);

		EnableWindow(hUp, pLv->iItem > 0);
		EnableWindow(hDown, pLv->iItem < ListView_GetItemCount(pHdr->hwndFrom) - 1);
	}
	return TRUE;
}

/////////////////////////////////////////
// LVN_ENDLABELEDIT
/////////////////////////////////////////

BOOL FilterCtlDlg::Notify_EndLabelEdit(HWND hDlg, LPARAM lParam)
{
	NMLVDISPINFO *pDi = (NMLVDISPINFO*)lParam;
	NMHDR *pHdr = (NMHDR*)lParam;
	VFInfo *pInfo = (VFInfo*)(pDi->item.lParam);

	if (pDi->item.pszText == NULL) return FALSE;

	delete [] pInfo->pName;
	pInfo->pName = StringDup(pDi->item.pszText);
	ListView_SetItem(pHdr->hwndFrom, &(pDi->item));
	return TRUE;
}

/////////////////////////////////////////
// NMKEYDOWN
/////////////////////////////////////////

BOOL FilterCtlDlg::Notify_Keydown(HWND hDlg, LPARAM lParam)
{
	NMHDR *pHdr = (NMHDR*)lParam;
	NMLVKEYDOWN *pKd = (NMLVKEYDOWN*)lParam;
	if (pKd->wVKey != VK_DELETE) return TRUE;

	DeleteSelectedItem(hDlg);
	return TRUE;
}

/////////////////////////////////////////
// NMDBLCLK
/////////////////////////////////////////

BOOL FilterCtlDlg::Notify_DblClick(HWND hDlg, LPARAM lParam)
{
	NMLISTVIEW* pLv = (NMLISTVIEW*)lParam;
	HWND hList = GetDlgItem(hDlg, IDC_FILTERCTL_LIST);

	// get base info
	int iSel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	if (iSel < 0 ) return TRUE;
	LVITEM li;
	li.mask = LVIF_PARAM;
	li.iItem = iSel;
	li.iSubItem = 0;
	ListView_GetItem(hList, &li);
	VFInfo *pInfoMaster = (VFInfo*)li.lParam;

	// copy VFInfo for update
	VFInfo *pInfoUpd = pInfoMaster->Clone();	
	if (pInfoUpd == NULL) return FALSE;
	// popup dialog
	FilterDefDlg fd;
	fd.Init();
	if (fd.Popup(g_hInstance, hDlg, pInfoUpd, FALSE) == IDOK) {
		li.lParam = (LPARAM)pInfoUpd;
		ListView_SetItem(hList, &li);

		pInfoMaster->Release();
		delete pInfoMaster;

	} else {
		pInfoUpd->Release();
		delete pInfoUpd;
	}
	return TRUE;
}

/////////////////////////////////////////
// Dialog procedure
/////////////////////////////////////////

static LRESULT CALLBACK FilterCtlDlgProc(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	FilterCtlDlg *pDlg;
	if (nMessage == WM_INITDIALOG) {
		SetWindowLong(hDlg, DWL_USER, lParam);
		pDlg = (FilterCtlDlg*)lParam;

		pDlg->InitDialog(hDlg);
		return TRUE;
	}
	pDlg = (FilterCtlDlg*)GetWindowLong(hDlg, DWL_USER);
	if (pDlg == NULL) return FALSE;

	switch(nMessage) {
	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			if (pDlg->OnOK(hDlg)) {
				pDlg->DestroyDialog(hDlg, IDOK);
			}
			break;
		case IDCANCEL:
			pDlg->DestroyDialog(hDlg, IDCANCEL);
			break;
		case IDC_FILTERCTL_KEEP:
			pDlg->Command_ToggleKeep(hDlg);
			break;
		case IDC_FILTERCTL_UP:
			pDlg->Command_UpDown(hDlg, -1);
			break;
		case IDC_FILTERCTL_DOWN:
			pDlg->Command_UpDown(hDlg, 1);
			break;
		case IDC_FILTERCTL_NEW:
			pDlg->Command_New(hDlg);
			break;
		case IDC_FILTERCTL_DELETE:
			pDlg->Command_Delete(hDlg);
			break;
		case IDC_FILTERCTL_RENAME:
			pDlg->Command_Rename(hDlg);
			break;
		}
		return TRUE;
	case WM_NOTIFY:
		return pDlg->OnNotify(hDlg, wParam, lParam);
	}
	return FALSE;
}
