#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
#include <aygshell.h>
#endif

#include "Tombo.h"
#include "TString.h"
#include "TomboURI.h"
#include "MemoSelectView.h"
#include "resource.h"

#include "UniConv.h"
#include "MemoManager.h"
#include "VarBuffer.h"
#include "MainFrame.h"
#include "Property.h"
#include "TreeViewItem.h"
#include "Message.h"

#include "NewFolderDialog.h"

#include "AutoPtr.h"
#include "Repository.h"

/////////////////////////////////////////
//  defs
/////////////////////////////////////////

// Size of each image in IDB_MEMOSELECT_IMAGES
#if defined(FOR_VGA)
#define IMAGE_CX 24
#define IMAGE_CY 24
#else
#define IMAGE_CX 16
#define IMAGE_CY 16
#endif

// Number of items in IDB_MEMOSELECT_IMAGES
#define NUM_MEMOSELECT_BITMAPS 10

/////////////////////////////////////////
//  root info
/////////////////////////////////////////

struct MSViewRootInfo {
	TomboURI uri;
	HTREEITEM hItem;
	DWORD nType;
};

/////////////////////////////////////////
//  static functions
/////////////////////////////////////////

static void InsertDummyNode(HWND hTree, HTREEITEM hItem);
static HTREEITEM FindItem2(HWND hWnd, HTREEITEM hParent, LPCTSTR pHeadLine, DWORD nLen);
void SelectViewSetWndProc(SUPER_WND_PROC wp, HWND hParent, HINSTANCE h, MemoSelectView *p);
LRESULT CALLBACK NewSelectViewProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
static HIMAGELIST CreateSelectViewImageList(HINSTANCE hInst);

/////////////////////////////////////////
// init
/////////////////////////////////////////

MemoSelectView::~MemoSelectView()
{
	delete [] pRoots;
}

BOOL MemoSelectView::Create(LPCTSTR pName, RECT &r, HWND hParent, DWORD nID, HINSTANCE hInst, HFONT hFont)
{
	hSelectViewImgList = CreateSelectViewImageList(hInst);

	DWORD nWndStyle;
	nWndStyle = WS_CHILD | WS_VSCROLL | WS_HSCROLL | TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;
#if !defined(PLATFORM_WM5)
	nWndStyle |= TVS_EDITLABELS;
#endif

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	hViewWnd = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, pName, nWndStyle,
								r.left, r.top, r.right, r.bottom,
								hParent, (HMENU)nID, hInst, this);
#else
	hViewWnd = CreateWindow(WC_TREEVIEW, pName, nWndStyle | WS_BORDER,
							r.left, r.top, r.right, r.bottom, 
							hParent, (HMENU)nID, hInst, this);
#endif
	if (hViewWnd == NULL) return FALSE;

	SUPER_WND_PROC wp = (SUPER_WND_PROC)GetWindowLong(hViewWnd, GWL_WNDPROC);
	SelectViewSetWndProc(wp, hParent, g_hInstance, this);
	SetWindowLong(hViewWnd, GWL_WNDPROC, (LONG)NewSelectViewProc);

	TreeView_SetImageList(hViewWnd, hSelectViewImgList, TVSIL_NORMAL);

	if (hFont != NULL) {
		SendMessage(hViewWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	}
	return TRUE;
}

/////////////////////////////////////////
// Insert node
/////////////////////////////////////////
// Insert node with sorting.

static HTREEITEM InsertNode(HWND hTree, TV_INSERTSTRUCT *ti)
{
	LPCTSTR pInsertStr = ti->item.pszText;
	DWORD n = _tcslen(pInsertStr);
	TString compBuf;

	TreeViewItem *pInsItem = (TreeViewItem*)ti->item.lParam;
	DWORD nInsOrder = pInsItem->ItemOrder();

	TreeViewItem *pCompareItem;
	DWORD nCompOrder;

	if (!compBuf.Alloc(n + 1)) {
		return NULL;
	}

	if (ti->hParent != TVI_ROOT) {
		HTREEITEM h;
		HTREEITEM hIns;

		TV_ITEM t;
		t.mask = TVIF_TEXT | TVIF_PARAM;
		t.cchTextMax = n;
		t.pszText = compBuf.Get();

		hIns = TVI_FIRST;

		h = TreeView_GetChild(hTree, ti->hParent);
		while(h) {
			t.hItem = h;
			TreeView_GetItem(hTree, &t);
			pCompareItem = (TreeViewItem*)t.lParam;

			nCompOrder = 0;
			if (pCompareItem) {
				nCompOrder = pCompareItem->ItemOrder();
			} else {
				nCompOrder = -1;
			}

			if (nInsOrder < nCompOrder) break;
			if (nInsOrder == nCompOrder && _tcsicmp(t.pszText, pInsertStr) > 0) break;

			hIns = h;
			h = TreeView_GetNextSibling(hTree, h);
		}
		ti->hInsertAfter = hIns;
	} else {
		ti->hInsertAfter = TVI_LAST;
	}
	HTREEITEM hInserted = TreeView_InsertItem(hTree, ti);
	return hInserted;
}

/////////////////////////////////////////
// Insert file node
/////////////////////////////////////////

HTREEITEM MemoSelectView::InsertFile(HTREEITEM hParent, const TomboURI *pURI, 
								LPCTSTR pTitle, BOOL bInsertToLast, BOOL bLink)
{
	URIOption opt(NOTE_OPTIONMASK_ICON);
	if (!g_Repository.GetOption(pURI, &opt)) return NULL;

	TV_INSERTSTRUCT ti;
	ti.hParent = hParent;
	ti.hInsertAfter = TVI_LAST;
	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	ti.item.iImage = ti.item.iSelectedImage = opt.iIcon;

	TreeViewFileItem *tvi;
	if (bLink) {
		tvi = new TreeViewFileLink();
	} else {
		tvi = new TreeViewFileItem();
	}

	if (!tvi) return NULL;

	tvi->SetURI(pURI);
	ti.item.lParam = (LPARAM)tvi;

	ti.item.pszText = (LPTSTR)pTitle;
	HTREEITEM hItem;
	if (bInsertToLast) {
		hItem = TreeView_InsertItem(hViewWnd, &ti);
	} else {
		hItem = InsertNode(hViewWnd, &ti);
	}
	tvi->SetViewItem(hItem);
	return hItem;
}

/////////////////////////////////////////
// Insert folder
/////////////////////////////////////////

HTREEITEM MemoSelectView::InsertFolder(HTREEITEM hParent, LPCTSTR pName, TreeViewItem *tvi, BOOL bInsertLast)
{
	HWND hTree = hViewWnd;

	TV_INSERTSTRUCT ti;
	ti.hParent = hParent;
	ti.hInsertAfter = TVI_LAST;
	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	ti.item.pszText = (LPTSTR)pName;
	ti.item.iImage = ti.item.iSelectedImage = tvi->GetIcon(this, MEMO_VIEW_STATE_INIT);
	ti.item.lParam = (LPARAM)tvi;

	HTREEITEM hItem;
	if (bInsertLast) {
		hItem = TreeView_InsertItem(hViewWnd, &ti);
	} else {
		hItem = InsertNode(hTree, &ti);
	}
	tvi->SetViewItem(hItem);

	InsertDummyNode(hTree, hItem);
	return hItem;
}

/////////////////////////////////////////
// Initialize tree
/////////////////////////////////////////

BOOL MemoSelectView::InitTree(VFManager *pManager)
{
	DeleteAllItem();

	nNumRoots = g_Repository.GetNumOfSubRepository();
	pRoots = new MSViewRootInfo[nNumRoots];

	for (DWORD i = 0; i < nNumRoots; i++) {
		const TomboURI *pSubRootURI = g_Repository.GetSubRepositoryRootURI(i);
		LPCTSTR pSubRootName = g_Repository.GetSubRepositoryName(i);
		DWORD nSubRootType = g_Repository.GetSubRepositoryType(i);

		pRoots[i].uri = *g_Repository.GetSubRepositoryRootURI(i);
		pRoots[i].nType = nSubRootType;

		switch(nSubRootType) {
		case TOMBO_REPO_SUBREPO_TYPE_LOCALFILE:
			{
				TreeViewFolderItem *pItem = new TreeViewFolderItem();
				pItem->SetURI(pSubRootURI);
				pRoots[i].hItem = InsertFolder(TVI_ROOT, pSubRootName, pItem, TRUE);
			}
			break;
		case TOMBO_REPO_SUBREPO_TYPE_VFOLDER:
			{
				TreeViewVirtualFolderRoot *pVFRoot = new TreeViewVirtualFolderRoot();
				if (!pVFRoot || !pVFRoot->Init(pSubRootURI, pManager)) return FALSE;
				pRoots[i].hItem = InsertFolder(TVI_ROOT, pSubRootName, pVFRoot, TRUE);
			}
			break;
		default:
			return FALSE;
		}
		TreeView_Expand(hViewWnd, pRoots[i].hItem, TVE_EXPAND);
	}
	return TRUE;
}

/////////////////////////////////////////
// Delete one item
/////////////////////////////////////////
//
// Delete item from the treeview and release its memory.

void MemoSelectView::DeleteOneItem(HTREEITEM hItem)
{
	TV_ITEM ti;
	ti.mask = TVIF_PARAM;

	// get lParam(is TreeViewItem*);
	ti.hItem = hItem;
	TreeView_GetItem(hViewWnd, &ti);
	TreeViewItem *tvi;
	tvi = (TreeViewItem *)ti.lParam;

	// if deleting item is clipped, release clipping.
	if (pClipItem == tvi) pClipItem = NULL;

	delete tvi;
	TreeView_DeleteItem(hViewWnd, hItem);
}

/////////////////////////////////////////
// Delete items recursive.
/////////////////////////////////////////
//
// Usually, hFirst is first child of a node named "A".
// DeleteItemsRec() are delete follow sibling and these children, 
// so parent node "A" is not deleted.
// It deletes TreeViewItem that is pointed by HTREEITEM, too.

void MemoSelectView::DeleteItemsRec(HTREEITEM hFirst)
{
	HTREEITEM hItem = hFirst;
	HTREEITEM hNext, hChild;

	while(hItem) {
		// Delete children.
		hChild = TreeView_GetChild(hViewWnd, hItem);
		if (hChild) {
			DeleteItemsRec(hChild);
		}

		// Get next item before hItem is deleted.
		hNext = TreeView_GetNextSibling(hViewWnd, hItem);

		// Delete hItem
		DeleteOneItem(hItem);

		hItem = hNext;
	}
}

BOOL MemoSelectView::DeleteAllItem()
{
	DeleteItemsRec(TreeView_GetRoot(hViewWnd));
	pClipItem = NULL;
	return TRUE;
}

/////////////////////////////////////////
// Is the item expending?
/////////////////////////////////////////

BOOL MemoSelectView::IsExpand(HTREEITEM hItem)
{
	TV_ITEM ti;
	ti.hItem = hItem;
	ti.mask = TVIF_STATE;
	ti.state = TVIS_EXPANDED;
	ti.stateMask = TVIS_EXPANDED;
	TreeView_GetItem(hViewWnd, &ti);
	return ti.state & TVIS_EXPANDED;
}

/////////////////////////////////////////
// get item current selected
/////////////////////////////////////////

const TomboURI* MemoSelectView::GetCurrentSelectedURI()
{
	TreeViewItem *pItem = GetCurrentItem();
	if (pItem == NULL) return NULL;

	return pItem->GetRealURI();
}

TreeViewItem* MemoSelectView::GetCurrentItem(HTREEITEM *pItem)
{
	HTREEITEM hItem = TreeView_GetSelection(hViewWnd);
	if (hItem == NULL) return NULL;

	TV_ITEM it;
	it.mask = TVIF_PARAM;
	it.hItem = hItem;
	if (TreeView_GetItem(hViewWnd, &it)) {
		if (pItem) *pItem = hItem;
		TreeViewItem *tvi = (TreeViewItem*)it.lParam;
		return tvi;
	}
	return NULL;
}

BOOL MemoSelectView::Show(int nCmdShow)
{
	ShowWindow(hViewWnd, nCmdShow);
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	// close menu if switch to other view
	if (nCmdShow == SW_HIDE) {
		ReleaseCapture();
	}
#endif
	return UpdateWindow(hViewWnd);
}

void MemoSelectView::SetFocus()
{
	::SetFocus(hViewWnd);
}

///////////////////////////////////////////
// menu control
///////////////////////////////////////////

static void ControlSubMenu(HMENU hMenu, UINT nID, BOOL bEnable)
{
	if (bEnable) {
		EnableMenuItem(hMenu, nID, MF_BYCOMMAND | MF_ENABLED);
	} else {
		EnableMenuItem(hMenu, nID, MF_BYCOMMAND | MF_GRAYED);
	}
}

///////////////////////////////////////////
// OnNotify Handler
///////////////////////////////////////////
// if you want to return TRUE/FALSE, return TRUE/FALSE.
// if you want to do default action, return 0xFFFFFFFF.

LRESULT MemoSelectView::OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LPNM_TREEVIEW pHdr = (LPNM_TREEVIEW)lParam;

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	NMRGINFO *pnmrginfo = (PNMRGINFO)lParam;
	if (pnmrginfo->hdr.code == GN_CONTEXTMENU) {
		// tap & hold
		OnNotify_RClick(pnmrginfo->ptAction);
		return TRUE;
	}
#endif

	switch(pHdr->hdr.code) {
//	case NM_CLICK:
//		g_Logger.WriteLog("NM_CLICK\r\n");
//		return 0;
	case NM_DBLCLK:
		{
//			g_Logger.WriteLog("NM_DBLCLK\r\n");
			HTREEITEM hItem = TreeView_GetSelection(hViewWnd);
			TV_ITEM it;
			it.mask = TVIF_PARAM;
			it.hItem = hItem;

			TreeView_GetItem(hViewWnd, &it);
			TreeViewItem *tvi = (TreeViewItem*)it.lParam;
			if (!tvi) break;

			if (!tvi->HasMultiItem()) {
				const TomboURI *pURI = ((TreeViewFileItem*)tvi)->GetRealURI();
				GetManager()->GetMainFrame()->LoadMemo(pURI, TRUE);
				pMemoMgr->GetMainFrame()->PostSwitchView();
				return 0;
			}
			// 暗黙でExpand/Collapseが発生
		}
		break;
	case NM_RETURN:
		{
			// DBLCLKと違い、擬似的にツリーの開閉処理を発生させる
			HTREEITEM hItem = TreeView_GetSelection(hViewWnd);
			TV_ITEM it;
			it.mask = TVIF_PARAM | TVIF_STATE;
			it.hItem = hItem;
			it.stateMask = TVIS_EXPANDED;
			TreeView_GetItem(hViewWnd, &it);

			TreeViewItem *tvi = (TreeViewItem*)it.lParam;
			if (!tvi) break;

			if (tvi->HasMultiItem()) {
				ToggleExpandFolder(hItem, it.state & TVIS_EXPANDED);
			} else {
				// switch to edit view
				tvi->LoadMemo(this, TRUE);
				pMemoMgr->GetMainFrame()->ActivateView(MainFrame::VT_DetailsView);
			}
		}
		break;
	case TVN_SELCHANGING:
		{
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
			// 2-Pane Viewの場合には メモの切り替えが発生し、かつ
			// 保存確認でキャンセルを押した場合、アイテムの切り替えを
			// 認めてはならない
			if (!g_Property.GetUseTwoPane() || !pMemoMgr) return FALSE;

			DWORD nYNC;
			if (!pMemoMgr->SaveIfModify(&nYNC, FALSE)) {
				TCHAR buf[1024];
				wsprintf(buf, MSG_SAVE_FAILED, GetLastError());
				pMemoMgr->GetMainFrame()->MessageBox(buf, TEXT("ERROR"), MB_ICONSTOP | MB_OK | MB_APPLMODAL);

				// stop the change
				return TRUE;
			}
			switch(nYNC) {
			case IDCANCEL:
				// cancel
				return TRUE;
			case IDOK:
				/* fall through */
			case IDYES:
				return FALSE;
			case IDNO:
				// celar note
				pMemoMgr->GetDetailsView()->DiscardMemo();
				return FALSE;
			}
#endif
			return FALSE;
		}

	case TVN_SELCHANGED:
		{
			NMTREEVIEW *p = (LPNMTREEVIEW)lParam;
			TreeViewItem *tvi = (TreeViewItem*)((p->itemNew).lParam);
			if (tvi == NULL) {
				break;
			}
			// Control menu item
			ControlMenu();

			if (g_Property.GetUseTwoPane() && (p->action == TVC_BYMOUSE || p->action == TVC_BYKEYBOARD)) {
				// change notes if operated by user. Otherwise no switching occured.
				pMemoMgr->SetMSSearchFlg(TRUE);
				tvi->LoadMemo(this, FALSE);
			}
		}
		break;
	case TVN_ITEMEXPANDING:
		if (pHdr->action == TVE_EXPAND) {
			TreeExpand(pHdr->itemNew.hItem);
		} else {
			TreeCollapse(pHdr->itemNew.hItem);
		}
		return FALSE;
#if !defined(PLATFORM_WM5)
	case TVN_BEGINLABELEDIT:
		{
			TreeViewItem *pItem = (TreeViewItem*)(((LPNMTVDISPINFO)lParam)->item.lParam);
			if (pItem && !pItem->IsOperationEnabled(this, TreeViewItem::OpRename)) return TRUE;
			return FALSE;
		}
	case TVN_ENDLABELEDIT:
		{
			LPNMTVDISPINFO p = (LPNMTVDISPINFO)lParam;
			return EditLabel(&(p->item));
		}
#endif
#if defined(PLATFORM_WIN32)
	case NM_RCLICK:
		{
			POINT pt, pth;
			GetCursorPos(&pt);
			pth = pt;
			OnNotify_RClick(pt);
			break;
		}
#endif

	}
	// return for each handler if it need return values.
	return 0xFFFFFFFF;
}

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC) || defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
void MemoSelectView::OnNotify_RClick(POINT pt)
{
	TV_HITTESTINFO hti;
	POINT cpt = pt;
#if defined(PLATFORM_WIN32) || defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	ScreenToClient(hViewWnd, &cpt);
#endif
	hti.pt = cpt;
	HTREEITEM hX = TreeView_HitTest(hViewWnd, &hti);

	if (hti.hItem == NULL) return;
	TreeViewItem *pItem = GetTVItem(hti.hItem);

	TreeView_SelectItem(hViewWnd, hti.hItem);

	DWORD nFlg;
	if (pItem->HasMultiItem()) {
		nFlg = CTXMENU_DIR;
	} else {
		TreeViewFileItem *pFileItem = (TreeViewFileItem*)pItem;
		const TomboURI *pURI = pFileItem->GetRealURI();
		URIOption opt(NOTE_OPTIONMASK_ENCRYPTED);
		g_Repository.GetOption(pURI, &opt);

		nFlg = CTXMENU_FILE | (g_Property.GetUseAssociation() ? CTXMENU_USEASSOC : 0);
    /*
    // commented out so chi files can be launched by external apps
		if (!opt.bEncrypt) {
        */
			nFlg |= CTXMENU_ENABLEEXTAPP;
            /*
		}
    */
	}

	HMENU hMenu = PlatformLayer::LoadContextMenu(nFlg);

	ControlSubMenu(hMenu, IDM_ENCRYPT, pItem->IsOperationEnabled(this, TreeViewItem::OpEncrypt));
	ControlSubMenu(hMenu, IDM_DECRYPT, pItem->IsOperationEnabled(this, TreeViewItem::OpDecrypt));
	ControlSubMenu(hMenu, IDM_CUT, pItem->IsOperationEnabled(this, TreeViewItem::OpCut));
	ControlSubMenu(hMenu, IDM_COPY, pItem->IsOperationEnabled(this, TreeViewItem::OpCopy));
	ControlSubMenu(hMenu, IDM_PASTE, pItem->IsOperationEnabled(this, TreeViewItem::OpPaste));
	ControlSubMenu(hMenu, IDM_RENAME, pItem->IsOperationEnabled(this, TreeViewItem::OpRename));
	ControlSubMenu(hMenu, IDM_DELETEITEM, pItem->IsOperationEnabled(this, TreeViewItem::OpDelete));
	ControlSubMenu(hMenu, IDM_NEWFOLDER, pItem->IsOperationEnabled(this, TreeViewItem::OpNewFolder));
	ControlSubMenu(hMenu, IDM_SEARCH, pItem->IsOperationEnabled(this, TreeViewItem::OpGrep));
	ControlSubMenu(hMenu, IDM_TRACELINK, pItem->IsOperationEnabled(this, TreeViewItem::OpLink));

	DWORD id = TrackPopupMenuEx(hMenu, TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN, pt.x, pt.y, hViewWnd, NULL);
	DestroyMenu(hMenu);

	switch(id) {
	case IDM_CUT:
		OnCut(pItem);
		break;
	case IDM_COPY:
		OnCopy(pItem);
		break;
	case IDM_PASTE:
		OnPaste();
		break;
	case IDM_ENCRYPT:
		OnEncrypt(pItem);
		break;
	case IDM_DECRYPT:
		OnDecrypt(pItem);
		break;
	case IDM_RENAME:
		OnEditLabel(hti.hItem);
		break;
	case IDM_DELETEITEM:
		OnDelete(hti.hItem, pItem);
		break;
	case IDM_NEWFOLDER:
		pMemoMgr->GetMainFrame()->NewFolder(pItem);
		break;
	case IDM_SEARCH:
		TreeView_SelectItem(hViewWnd, hti.hItem);
		pMemoMgr->GetMainFrame()->OnSearch();
		break;
	case IDM_TRACELINK:
		{
			TreeViewFileLink *p = (TreeViewFileLink*)pItem;
			ShowItemByURI(p->GetRealURI(), TRUE, FALSE);
		}
		break;
	case IDM_ASSOC:
		TreeView_SelectItem(hViewWnd, hti.hItem);
		pItem->ExecApp(pMemoMgr, this, ExecType_Assoc);
		break;
	case IDM_EXTAPP1:
		TreeView_SelectItem(hViewWnd, hti.hItem);
		pItem->ExecApp(pMemoMgr, this, ExecType_ExtApp1);
		break;
	case IDM_EXTAPP2:
		TreeView_SelectItem(hViewWnd, hti.hItem);
		pItem->ExecApp(pMemoMgr, this, ExecType_ExtApp2);
		break;
	}
}
#endif
///////////////////////////////////////////
// OnCommand handler
///////////////////////////////////////////

BOOL MemoSelectView::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HTREEITEM  hItem;
	TreeViewItem *pItem = GetCurrentItem(&hItem);

	switch(LOWORD(wParam)) {
	case IDM_ENCRYPT:
		OnEncrypt(pItem);
		return TRUE;
	case IDM_DECRYPT:
		OnDecrypt(pItem);
		return TRUE;
	case IDM_DELETEITEM:
		// DEL key is handled by SelectView findows procedure, but leave this chunk for sending WM_COMMAND
		OnDelete(hItem, pItem);
		return TRUE;
	case IDM_ACTIONBUTTON:
		OnActionButton(hWnd);
		return TRUE;
	case IDM_CUT:
		// Cut/Copy/Paste key is handled by SelectView findows procedure, but leave this chunk for sending WM_COMMAND
		OnCut(pItem);
		return TRUE;
	case IDM_COPY:
		OnCopy(pItem);
		return TRUE;
	case IDM_PASTE:
		OnPaste();
		return TRUE;
	case IDM_RENAME:
		OnEditLabel(hItem);
		return TRUE;
#if defined(PLATFORM_BE500)
	case IDM_EXTAPP1:
		pItem->ExecApp(pMemoMgr, this, ExecType_ExtApp1);
		return TRUE;
	case IDM_EXTAPP2:
		pItem->ExecApp(pMemoMgr, this, ExecType_ExtApp2);
		return TRUE;
#endif
	}
	return FALSE;
}

///////////////////////////////////////////
// HotKey
///////////////////////////////////////////

BOOL MemoSelectView::OnHotKey(HWND hWnd, WPARAM wParam)
{
	switch(wParam) {
	case APP_BUTTON1:
		/* fall through */
	case APP_BUTTON2:
		/* fall through */
	case APP_BUTTON3:
		/* fall through */
	case APP_BUTTON4:
		/* fall through */
	case APP_BUTTON5:
		OnActionButton(hWnd);
		return TRUE;
	default:
		return FALSE;
	}
}

///////////////////////////////////////////
// Encrypt
///////////////////////////////////////////

void MemoSelectView::OnEncrypt(TreeViewItem *pItem)
{
	if (!pItem->Encrypt(pMemoMgr, this)) {
		TCHAR buf[1024];
		wsprintf(buf, MSG_ENCRYPTION_FAILED, GetLastError());
		TomboMessageBox(hViewWnd, buf, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
	}
}

///////////////////////////////////////////
// Decrypt
///////////////////////////////////////////

void MemoSelectView::OnDecrypt(TreeViewItem *pItem)
{
	if (!pItem->Decrypt(pMemoMgr, this)) {
		TCHAR buf[1024];
		wsprintf(buf, MSG_DECRYPTION_FAILED, GetLastError());
		TomboMessageBox(hViewWnd, buf, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
	}
}

/////////////////////////////////////////////
// Rename
/////////////////////////////////////////////

void MemoSelectView::OnEditLabel(HTREEITEM hItem)
{
#if defined(PLATFORM_WM5)
	TCHAR buf[MAX_PATH];
	TV_ITEM item;
	item.mask = TVIF_TEXT | TVIF_PARAM;
	item.hItem = hItem;
	item.cchTextMax = MAX_PATH;
	item.pszText = buf;
	TreeView_GetItem(hViewWnd, &item);

	NewFolderDialog dlg;
	dlg.SetOption(MSG_ID_MENUITEM_MAIN_RENAME, buf);
	if (dlg.Popup(g_hInstance, hViewWnd) == IDOK) {
		TreeViewItem *pti = (TreeViewItem*)(item.lParam);
		if (pti == NULL) return;
		pti->Rename(pMemoMgr, this, dlg.FolderName());
		
		item.mask = TVIF_TEXT;
		item.cchTextMax = _tcslen(dlg.FolderName());
		item.pszText = (LPTSTR)dlg.FolderName();
		TreeView_SetItem(hViewWnd, &item);
	}
#else
	if (hItem == NULL) return;
	TreeView_EditLabel(hViewWnd, hItem);
	return;
#endif
}

///////////////////////////////////////////
// delete
///////////////////////////////////////////

void MemoSelectView::OnDelete(HTREEITEM hItem, TreeViewItem *pItem)
{
	if (hItem == NULL) return;
	if (pItem->Delete(pMemoMgr, this)) {
		DeleteOneItem(hItem);
	}
}

///////////////////////////////////////////
// get window size
///////////////////////////////////////////

void MemoSelectView::GetSize(LPWORD pWidth, LPWORD pHeight)
{
	RECT r;
	GetWindowRect(hViewWnd, &r);
	*pWidth = (WORD)(r.right - r.left);
	*pHeight = (WORD)(r.bottom - r.top);
}

void MemoSelectView::GetSize(LPRECT pRect)
{
	GetWindowRect(hViewWnd, pRect);
}

void MemoSelectView::GetClientRect(LPRECT pRect)
{
	::GetClientRect(hViewWnd, pRect);
}

///////////////////////////////////////////
// get path
///////////////////////////////////////////

LPTSTR MemoSelectView::GeneratePath(HTREEITEM hItem, LPTSTR pBuf, DWORD nSiz)
{
	HWND hTree = hViewWnd;

	LPTSTR p = pBuf + nSiz - 2;
	*(p+1) = TEXT('\0');

	LPTSTR pPrev;

	TV_ITEM it;
	TCHAR buf[MAX_PATH];

	HTREEITEM h = hItem;
	it.mask = TVIF_HANDLE | TVIF_TEXT;
	it.pszText = buf;

	while(h) {
		it.hItem = h;
		it.cchTextMax = MAX_PATH;
		TreeView_GetItem(hTree, &it);

		DWORD l = _tcslen(buf);
		pPrev = p + 1;
		*p-- = TEXT('\\');
		p -= l - 1;
		_tcsncpy(p, buf, l);
		p--;
		h = TreeView_GetParent(hTree, h);
	}
	return pPrev;
}

void MemoSelectView::TreeExpand(HTREEITEM hItem)
{
	// delete dummy child nodes.
	HTREEITEM di = TreeView_GetChild(hViewWnd, hItem);
	DeleteItemsRec(di);

	// change icon to "EXPANDING".
	TreeViewItem *pItem = GetTVItem(hItem);
	SetIcon(pItem, MEMO_VIEW_STATE_OPEN_SET);

	if (!pItem->HasMultiItem()) {
		MessageBox(NULL, TEXT("This node does'nt have multi item"), TEXT("DEBUG"), MB_OK);
		return;
	}

	TreeViewFolderItem *pFolder = (TreeViewFolderItem*)pItem;

	// expand nodes.
	pFolder->Expand(this);

	// Close node if lower node is not exist.
	if (TreeView_GetChild(hViewWnd, hItem) == NULL) {
		TreeCollapse(hItem);
	}
}

void MemoSelectView::TreeCollapse(HTREEITEM hItem)
{
	HTREEITEM h = TreeView_GetChild(hViewWnd, hItem);

	DeleteItemsRec(h); // delete nodes
	InsertDummyNode(hViewWnd, hItem); // insert dummy child

	// set icon
	TreeViewItem *pItem = GetTVItem(hItem);
	SetIcon(pItem, MEMO_VIEW_STATE_OPEN_CLEAR); 

	TreeView_Expand(hViewWnd, hItem, TVE_COLLAPSE | TVE_COLLAPSERESET);
}


///////////////////////////////////////////
// move window or resize window
///////////////////////////////////////////

void MemoSelectView::MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight)
{
	::MoveWindow(hViewWnd, x, y, nWidth, nHeight, TRUE);
}

/////////////////////////////////////////////////////////////
// TreeViewItem status change notify
/////////////////////////////////////////////////////////////
// Called from TreeViewItem when icon status/headline string changed.
// Update treeview status and link assosiation of HTREEITEM <-> TreeViewItem 
// and TreeViewItem <-> MemoManager.

BOOL MemoSelectView::UpdateItemStatusNotify(TreeViewItem *pItem, LPCTSTR pNewHeadLine)
{
	// Get old HTREEITEM
	HTREEITEM hOrigNode = pItem->GetViewItem();

	HTREEITEM hParent = TreeView_GetParent(hViewWnd, hOrigNode);
	if (hParent == NULL) {
		hParent = TVI_ROOT;
	}

	// Delete from Tree. (TreeViewItem is not deleted because it will deleted at caller.)
	TreeView_DeleteItem(hViewWnd, hOrigNode);

	// Insert to tree.
	TV_INSERTSTRUCT ti;
	ti.hParent = hParent;
	ti.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	ti.item.iImage = ti.item.iSelectedImage = pItem->GetIcon(this, 0); // Get current icon
	ti.item.pszText = (LPTSTR)pNewHeadLine;
	ti.item.lParam = (LPARAM)pItem;
	HTREEITEM hItem = ::InsertNode(hViewWnd, &ti);

	// link TreeViewItem <-> HTREEITEM assosiation
	pItem->SetViewItem(hItem);
	TreeView_SelectItem(hViewWnd, hItem);
	return TRUE;
}

///////////////////////////////////////////
// Action(Hardware) buttons
///////////////////////////////////////////

void MemoSelectView::OnActionButton(HWND hWnd)
{
	HTREEITEM hItem;
	TreeViewItem *p = GetCurrentItem(&hItem);
	if (p && !p->HasMultiItem()) {
		p->OpenMemo(this, OPEN_REQUEST_MSVIEW_ACTIVE);
		return;
	}
	
	RECT r;
	TreeView_GetItemRect(hViewWnd, hItem, &r, TRUE);
	PostMessage(hViewWnd, WM_LBUTTONDBLCLK, MK_LBUTTON, MAKELPARAM(r.left, r.top));
}

///////////////////////////////////////////
// get item path for insert
///////////////////////////////////////////
// if the item is node, return itself.
// if the item is leaf, return the parent node of the leaf

HTREEITEM MemoSelectView::GetPathForNewItem(TString *pPath, TreeViewItem *pItem)
{
	TCHAR buf[MAX_PATH];
	HTREEITEM hItem = NULL;

	if (pItem == NULL) {
		pItem = GetCurrentItem(&hItem);
	} else {
		hItem = pItem->GetViewItem();
	}
	if (hItem == NULL) {
		if (!pPath->Alloc(1)) return NULL;
		_tcscpy(pPath->Get(), TEXT(""));

		return TreeView_GetRoot(hViewWnd);
	}

	if (pItem && !pItem->HasMultiItem()) {
		hItem = TreeView_GetParent(hViewWnd, hItem);
	}

	LPTSTR pPathTop = GeneratePath(hItem, buf, MAX_PATH);
	if (!pPath->Set(pPathTop)) return NULL;
	return hItem;
}

///////////////////////////////////////////
// carete new folder
///////////////////////////////////////////

BOOL MemoSelectView::CreateNewFolder(HTREEITEM hItem, LPCTSTR pFolder)
{
	// expand the tree if it collapsed
	if (!IsExpand(hItem) && (TreeView_GetChild(hViewWnd, hItem) != NULL)) {
		// the folder inserted is automatically reloaded by expanding the tree
		TreeView_Expand(hViewWnd, hItem, TVE_EXPAND);
	} else {
		// if node is already expanded, insert it

		TreeViewItem *pParent = GetTVItem(hItem);
		const TomboURI *pParentURI = pParent->GetRealURI();
		TString s;
		if (!s.Join(pParentURI->GetFullURI(), pFolder, TEXT("/"))) return FALSE;
		TomboURI sNewURI;
		if (!sNewURI.Init(s.Get())) return FALSE;

		TreeViewFolderItem *pItem = new TreeViewFolderItem();
		pItem->SetURI(&sNewURI);
		InsertFolder(hItem, pFolder, pItem, FALSE);
		if (TreeView_GetChild(hViewWnd, hItem) != NULL) {
			TreeView_Expand(hViewWnd, hItem, TVE_EXPAND);
		}
	}
	return TRUE;
}

BOOL MemoSelectView::MakeNewFolder(HWND hWnd, TreeViewItem *pItem)
{
	NewFolderDialog dlg;
	BOOL bPrev = bDisableHotKey;
	bDisableHotKey = TRUE;
	DWORD nResult = dlg.Popup(g_hInstance, hWnd);
	bDisableHotKey = bPrev;
	if (nResult == IDOK) {
		LPCTSTR pFolder = dlg.FolderName();

		TomboURI sBaseURI, sURI;
		if (pItem) {
			sBaseURI = *(pItem->GetRealURI());
		} else {
			const TomboURI *pCurSel = GetCurrentSelectedURI();
			if (pCurSel == NULL) return FALSE;
			sBaseURI = *pCurSel;
		}			

		if (!g_Repository.GetAttachURI(&sBaseURI, &sURI)) return FALSE;
		HTREEITEM hItem = GetItemFromURI(sURI.GetFullURI());
		if (!g_Repository.MakeFolder(&sURI, pFolder)) return FALSE;
		CreateNewFolder(hItem, pFolder);

	}
	return TRUE;
}

/////////////////////////////////////////
// Change headline string
/////////////////////////////////////////
// For re-ordering, delete once and re-insert again.
// New URI should locate same as old URI
// URI should point to file

BOOL MemoSelectView::UpdateHeadLine(LPCTSTR pOldURI, TomboURI *pNewURI, LPCTSTR pNewHeadLine)
{
	// get HTREEITEM from old URI
	HTREEITEM hOld = GetItemFromURI(pOldURI);
	if (hOld == NULL) return TRUE; // if node is collapsed, nothing to do

	// if URI is not changed, only focusing
	if (_tcscmp(pOldURI, pNewURI->GetFullURI()) == 0) {
		TreeView_SelectItem(hViewWnd, hOld);
		return TRUE;
	}


	// get parent
	HTREEITEM hParent = TreeView_GetParent(hViewWnd, hOld);

	TreeView_SelectItem(hViewWnd, NULL);

	// remove current node
	DeleteOneItem(hOld);

	// insert node
	HTREEITEM hNew = InsertFile(hParent, pNewURI, pNewHeadLine, FALSE, FALSE);
	if (hNew == NULL) return FALSE;

	TreeView_SelectItem(hViewWnd, hNew);
	return TRUE;
}

HTREEITEM MemoSelectView::GetItemFromURI(LPCTSTR pURI)
{
	TomboURI uri;
	if (!uri.Init(pURI)) return NULL;

	TString sRep;
	if (!uri.GetRepositoryName(&sRep)) return NULL;

	HTREEITEM hCurrent = GetRootItem(sRep.Get());
	if (hCurrent == NULL) return NULL;

	TomboURIItemIterator itr(&uri);
	if (!itr.Init()) return NULL;

	LPCTSTR p;
	for (itr.First(); p = itr.Current(); itr.Next()) {
		if (itr.IsLeaf()) {
			DWORD n = _tcslen(p);
			if (n >= 4) {
				hCurrent = FindItem2(hViewWnd, hCurrent, p, n - 4);
			} else {
				hCurrent = FindItem2(hViewWnd, hCurrent, p, n);
			}
		} else {
			hCurrent = FindItem2(hViewWnd, hCurrent, p, _tcslen(p));
		}
	}
	return hCurrent;
}

HTREEITEM MemoSelectView::GetRootItem(LPCTSTR pRep)
{
	for (DWORD i = 0; i < nNumRoots; i++) {
		TString repName;
		if (!pRoots[i].uri.GetRepositoryName(&repName)) return NULL;
		if (_tcsicmp(repName.Get(), pRep) == 0) return pRoots[i].hItem;
	}
	return NULL;
}

/////////////////////////////////////////
// set icon
/////////////////////////////////////////

void MemoSelectView::SetIcon(TreeViewItem *ptv, DWORD nStatus)
{
	DWORD nImage;

	nImage = ptv->GetIcon(this, nStatus);

	TV_ITEM ti;
	ti.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	ti.hItem = ptv->GetViewItem();
	ti.iImage = ti.iSelectedImage = nImage;
	TreeView_SetItem(hViewWnd, &ti);
}

/////////////////////////////////////////
// set item to psudo clipboard 
/////////////////////////////////////////

void MemoSelectView::SetShareArea(TreeViewItem *pItem)
{
	if (pClipItem) {
		SetIcon(pClipItem, MEMO_VIEW_STATE_CLIPED_CLEAR);
	}
	pClipItem = pItem;
	if (pClipItem) {
		SetIcon(pClipItem, MEMO_VIEW_STATE_CLIPED_SET);
	}
}

/////////////////////////////////////////
// prepare move the item
/////////////////////////////////////////

void MemoSelectView::OnCut(TreeViewItem *pItem)
{
	SetShareArea(pItem);
	bCut = TRUE;
}

/////////////////////////////////////////
// prepare copy the item
/////////////////////////////////////////

void MemoSelectView::OnCopy(TreeViewItem *pItem)
{
	SetShareArea(pItem);
	bCut = FALSE;
}

/////////////////////////////////////////
// move/copy notes
/////////////////////////////////////////

void MemoSelectView::OnPaste()
{
	if (pClipItem == NULL) return;

	if (bCut) {
		// move notes
		HTREEITEM hItem = pClipItem->GetViewItem();
		LPCTSTR pErr = NULL;
		if (!pClipItem->Move(pMemoMgr, this, &pErr)) {
			if (pErr) {
				TomboMessageBox(hViewWnd, pErr, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
			} else {
				TomboMessageBox(hViewWnd, MSG_MOVE_MEMO_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
			}
			return;
		}
		DeleteOneItem(hItem);
	} else {
		// copy notes
		LPCTSTR pErr = NULL;
		if (!pClipItem->Copy(pMemoMgr, this, &pErr)) {
			if (pErr) {
				TomboMessageBox(hViewWnd, pErr, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
			} else {
				TomboMessageBox(hViewWnd, MSG_COPY_MEMO_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
			}
		}
	}
}

/////////////////////////////////////////
// Control menu
/////////////////////////////////////////

void MemoSelectView::ControlMenu()
{
	MainFrame *pMf = pMemoMgr->GetMainFrame();
	if (pMf == NULL) return;

	TreeViewItem *pItem = GetCurrentItem();
	if (pItem) {
		pMf->EnableDelete(pItem->IsOperationEnabled(this, TreeViewItem::OpDelete));
		pMf->EnableRename(pItem->IsOperationEnabled(this, TreeViewItem::OpRename));

		pMf->EnableEncrypt(pItem->IsOperationEnabled(this, TreeViewItem::OpEncrypt));
		pMf->EnableDecrypt(pItem->IsOperationEnabled(this, TreeViewItem::OpDecrypt));

		pMf->EnableNew(pItem->IsOperationEnabled(this, TreeViewItem::OpNewMemo));
		pMf->EnableNewFolder(pItem->IsOperationEnabled(this, TreeViewItem::OpNewFolder));

		pMf->EnableCut(pItem->IsOperationEnabled(this, TreeViewItem::OpCut));
		pMf->EnableCopy(pItem->IsOperationEnabled(this, TreeViewItem::OpCopy));
		pMf->EnablePaste(pItem->IsOperationEnabled(this, TreeViewItem::OpPaste));

		pMf->EnableGrep(pItem->IsOperationEnabled(this, TreeViewItem::OpGrep));

	} else {
		pMf->EnableDelete(FALSE);
		pMf->EnableRename(FALSE);
		pMf->EnableEncrypt(FALSE);
		pMf->EnableDecrypt(FALSE);

		pMf->EnableCut(FALSE);
		pMf->EnableCopy(FALSE);
		pMf->EnablePaste(FALSE);
	}
}

/////////////////////////////////////////
// acquire forcusing
/////////////////////////////////////////

void MemoSelectView::OnGetFocus()
{
	MainFrame *pMf = pMemoMgr->GetMainFrame();
	if (pMf) {
		pMf->ActivateView(MainFrame::VT_SelectView);
	}
	ControlMenu();
}

/////////////////////////////////////////
// set font
/////////////////////////////////////////

void MemoSelectView::SetFont(HFONT hFont)
{
	SendMessage(hViewWnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
}

///////////////////////////////////////////
// expand/collapse tree
///////////////////////////////////////////

void MemoSelectView::ToggleExpandFolder(HTREEITEM hItem, UINT status)
{
	if (status & TVIS_EXPANDED) {
		TreeCollapse(hItem);
	} else {
		TreeView_Expand(hViewWnd, hItem, TVE_EXPAND);
		TreeExpand(hItem);
	}
}

/////////////////////////////////////////////
//  get TreeViewItem from HTREEITEM
/////////////////////////////////////////////

TreeViewItem *MemoSelectView::GetTVItem(HTREEITEM h)
{
	TV_ITEM ti;
	ti.mask = TVIF_PARAM;
	ti.hItem = h;
	TreeView_GetItem(hViewWnd, &ti);
	return (TreeViewItem*)ti.lParam;
}

/////////////////////////////////////////////
// Insert dummy node
/////////////////////////////////////////////
// to add "+" icon, insert dummy node if a folder which don't have no chil.

static void InsertDummyNode(HWND hTree, HTREEITEM hItem)
{
	TV_INSERTSTRUCT tisub;
	tisub.hParent = hItem;
	tisub.hInsertAfter = TVI_LAST;
	tisub.item.mask = TVIF_PARAM;
	tisub.item.lParam = NULL;
	TreeView_InsertItem(hTree, &tisub);
}

/////////////////////////////////////////////
// Rename
/////////////////////////////////////////////
//
// Result:  TRUE = accept label change
//			FALSE = refuse label change

LRESULT MemoSelectView::EditLabel(TVITEM *pItem)
{
	if (pItem->pszText == NULL) return FALSE;

	TreeViewItem *pti = (TreeViewItem*)(pItem->lParam);
	if (pti == NULL) return FALSE;
	return pti->Rename(pMemoMgr, this, pItem->pszText);
}

/////////////////////////////////////////////
// Expand tree and show note
/////////////////////////////////////////////

static HTREEITEM FindItem2(HWND hWnd, HTREEITEM hParent, LPCTSTR pStr, DWORD nLen)
{
	HTREEITEM hItem = TreeView_GetChild(hWnd, hParent);
	TV_ITEM ti;

	TCHAR buf[MAX_PATH + 1];
	ti.mask = TVIF_TEXT | TVIF_PARAM;
	ti.cchTextMax = MAX_PATH;
	ti.pszText = buf;

	while(hItem) {
		ti.hItem = hItem;
		TreeView_GetItem(hWnd, &ti);

		if (_tcsnicmp(buf, pStr, nLen) == 0 && *(buf + nLen) == TEXT('\0')) {
			return hItem;
		}

		hItem = TreeView_GetNextSibling(hWnd, hItem);
	}
	return NULL;
}

HTREEITEM MemoSelectView::ShowItemByURI(const TomboURI *pURI, BOOL bSelChange, BOOL bOpenNotes)
{
	HTREEITEM hCurrent;

	// get root node
	TString sRepo;
	if (!pURI->GetRepositoryName(&sRepo)) return FALSE;
	hCurrent = GetRootItem(sRepo.Get());
	if (hCurrent == NULL) return NULL;

	// expand root node
	if (!IsExpand(hCurrent)) {
		TreeView_Expand(hViewWnd, hCurrent, TVE_EXPAND);
	}

	if (bSelChange) TreeView_SelectItem(hViewWnd, hCurrent);

	TomboURIItemIterator itr(pURI);
	if (!itr.Init()) return NULL;

	TreeViewItem *pItem = NULL;
	LPCTSTR p;
	for (itr.First(); p = itr.Current(); itr.Next()) {
		HTREEITEM h;
		if (itr.IsLeaf()) {
			TString sHeadLine;
			if (!g_Repository.GetHeadLine(pURI, &sHeadLine)) return NULL;
			h = FindItem2(hViewWnd, hCurrent, sHeadLine.Get(), _tcslen(sHeadLine.Get()));
		} else {
			h = FindItem2(hViewWnd, hCurrent, p, _tcslen(p));
		}

		if (h) {
			hCurrent = h;

			pItem = GetTVItem(hCurrent);

			if (pItem->HasMultiItem() && !IsExpand(hCurrent)) {
				TreeView_Expand(hViewWnd, hCurrent, TVE_EXPAND);
			}
		}
	}

	if (hCurrent && bSelChange) {
		TreeView_SelectItem(hViewWnd, hCurrent);
	}

	if (pItem) {
		DWORD nOption = bOpenNotes ? OPEN_REQUEST_MSVIEW_ACTIVE : OPEN_REQUEST_MDVIEW_ACTIVE;
		pItem->OpenMemo(this, nOption);
	}
	return hCurrent;
}

/////////////////////////////////////////////
// Insert grep result
/////////////////////////////////////////////

BOOL MemoSelectView::InsertVirtualFolder(const VFInfo *pInfo)
{
	HTREEITEM hSearchRoot = GetRootItem(VFOLDER_REPO_NAME);
	TreeViewVirtualFolderRoot *pVFRoot = (TreeViewVirtualFolderRoot*)GetTVItem(hSearchRoot);
	return pVFRoot->AddSearchResult(this, pInfo);
}

/////////////////////////////////////////////
// Get virtual folder root instance
/////////////////////////////////////////////
void MemoSelectView::CloseVFRoot()
{
	HTREEITEM hSearchRoot = GetRootItem(VFOLDER_REPO_NAME);
	if (IsExpand(hSearchRoot)) {
		TreeCollapse(hSearchRoot);
	}
}

/////////////////////////////////////////////
// select node
/////////////////////////////////////////////

void MemoSelectView::SelUpFolderWithoutOpen()
{
	HTREEITEM hItem;
	TreeViewItem *pItem = GetCurrentItem(&hItem);
	HTREEITEM hParent = TreeView_GetParent(hViewWnd, hItem);
	if (hParent) {
		TreeView_SelectItem(hViewWnd, hParent);
	}
}

void MemoSelectView::SelNextBrother()
{
	HTREEITEM hItem;
	TreeViewItem *pItem = GetCurrentItem(&hItem);

	HTREEITEM h = TreeView_GetNextSibling(hViewWnd, hItem);
	if (h) {
		TreeView_SelectItem(hViewWnd, h);
	}
}

void MemoSelectView::SelPrevBrother()
{
	HTREEITEM hItem;
	TreeViewItem *pItem = GetCurrentItem(&hItem);

	HTREEITEM h = TreeView_GetPrevSibling(hViewWnd, hItem);
	if (h) {
		TreeView_SelectItem(hViewWnd, h);
	}
}

static HIMAGELIST CreateSelectViewImageList(HINSTANCE hInst)
{
	HIMAGELIST hImageList;
	// Create Imagelist.
	if ((hImageList = ImageList_Create(IMAGE_CX, IMAGE_CY, ILC_MASK, NUM_MEMOSELECT_BITMAPS, 0)) == NULL) return NULL;
#if defined(FOR_VGA)
	HBITMAP hBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MEMOSELECT_IMAGES24));
#else
	HBITMAP hBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_MEMOSELECT_IMAGES));
#endif
	// Transparent color is GREEN
	COLORREF rgbTransparent = RGB(0,255,0);
	ImageList_AddMasked(hImageList, hBmp, rgbTransparent);
	DeleteObject(hBmp);

	return hImageList;
}
