#ifndef MEMOSELECTVIEW_H
#define MEMOSELECTVIEW_H

class MemoManager;
class TString;
class TreeViewItem;
class MemoLocator;
class TreeViewVirtualFolderRoot;
class VFManager;
class VFInfo;
class TomboURI;
struct MSViewRootInfo;

class MemoSelectView {

	/////////////////////////////
	// Window related members

	HWND hViewWnd;

	// root node
	MSViewRootInfo *pRoots;
	DWORD nNumRoots;

	HIMAGELIST hSelectViewImgList;

	MemoManager *pMemoMgr;

	/////////////////////////////
	// for Cut/Copy/Paste
	TreeViewItem *pClipItem;
	BOOL bCut;

	// mode flags
	BOOL bAutoLoadMode;
	BOOL bSingleClickMode;

	void SetShareArea(TreeViewItem *pItem);

	void DeleteOneItem(HTREEITEM hItem);
	void DeleteItemsRec(HTREEITEM hFirst);

	///////////////////////////////////

	void SetIcon(TreeViewItem* p, DWORD nStatus);

	///////////////////////////////////
	LRESULT EditLabel(TVITEM *pItem);

protected:
	HTREEITEM GetRootItem(LPCTSTR pRep);

public:
	/////////////////////////////
	// Initialize functions

	MemoSelectView() : hViewWnd(NULL), pMemoMgr(NULL), bAutoLoadMode(FALSE), bSingleClickMode(FALSE), pRoots(NULL), nNumRoots(0) {}
	~MemoSelectView();
	BOOL Init(MemoManager *p) { pMemoMgr = p; bCut = FALSE; pClipItem = NULL; return TRUE; }
	BOOL Create(LPCTSTR pName, RECT &r, HWND hParent, DWORD nID, HINSTANCE hInst, HFONT hFont);

	/////////////////////////////
	// display related funcs

	BOOL Show(int nCmdShow);
	void SetFocus();
	void SetFont(HFONT hFont);

	LRESULT OnNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC) || defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	void OnNotify_RClick(POINT pt);
#endif
	BOOL OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void OnActionButton(HWND hWnd);
	BOOL OnHotKey(HWND hWnd, WPARAM wParam);
	void OnGetFocus();
	void OnDelete(HTREEITEM hItem, TreeViewItem *pItem);
	void OnEncrypt(TreeViewItem *pItem);
	void OnDecrypt(TreeViewItem *pItem);
	void OnEditLabel(HTREEITEM hItem);

	// psudo clipboard
	void OnCut(TreeViewItem *pItem);
	void OnCopy(TreeViewItem *pItem);
	void OnPaste();

	void GetSize(LPWORD pWidth, LPWORD pHeight);
	void GetSize(LPRECT pRect);
	void GetClientRect(LPRECT pRect);

	void MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight);

	/////////////////////////////
	// accessor/mutator for flags

	MemoManager *GetManager() { return pMemoMgr; }

	void SetAutoLoadMode(BOOL bMode) { bAutoLoadMode = bMode; }
	BOOL IsAutoLoadMode() { return bAutoLoadMode; }

	void SetSingleClickMode(BOOL bMode) { bSingleClickMode = bMode; }
	BOOL IsSingleClickMode() { return bSingleClickMode; }

	/////////////////////////////
	// Key
	void SelUpFolderWithoutOpen();
	void SelNextBrother();
	void SelPrevBrother();

	/////////////////////////////
	// item ops

	BOOL InitTree(VFManager *pManager);
	BOOL DeleteAllItem();

	/////////////////////////////
	// Expand/collapse tree
	void ToggleExpandFolder(HTREEITEM hItem, UINT stat);
	void TreeExpand(HTREEITEM hItem);
	void TreeCollapse(HTREEITEM hItem);
	BOOL IsExpand(HTREEITEM hItem);

	HIMAGELIST GetImageList() { return hSelectViewImgList; }

	// Insert file node
	HTREEITEM InsertFile(HTREEITEM hParent, const TomboURI *pURI, LPCTSTR pTitle, BOOL bInsertLast, BOOL bLink);

	// Insert folder node
	// if bInsertLast is TRUE, Insert folder without sorting.
	HTREEITEM InsertFolder(HTREEITEM hParent, LPCTSTR pName, TreeViewItem *tvi, BOOL bInsertLast);

	// update headline string
	BOOL UpdateHeadLine(LPCTSTR pOldURI, TomboURI *pNewURI, LPCTSTR pNewHeadLine);

	// TreeViewItemの状態が変わったことによるビューへの変更依頼
	BOOL UpdateItemStatusNotify(TreeViewItem *pItem, LPCTSTR pNewHeadLine);

	// 現在選択しているアイテムのパスを取得する。
	// 選択しているアイテムがフォルダの場合、自身までのパスを含む
	HTREEITEM GetPathForNewItem(TString *pPath, TreeViewItem *pItem = NULL);

	HTREEITEM GetParentItem(HTREEITEM hItem) { return TreeView_GetParent(hViewWnd, hItem); }

	BOOL CreateNewFolder(HTREEITEM hItem, LPCTSTR pFolder);
	BOOL MakeNewFolder(HWND hWnd, TreeViewItem *pItem);

	// Search tree and get HTREEITEM
	HTREEITEM GetItemFromURI(LPCTSTR pURI);

	// returns TreeViewItem associated with current selected node.
	// if pItem is not null set HTREEITEM current selected.
	// if no node is selected, return NULL.
	TreeViewItem *GetCurrentItem(HTREEITEM *pItem = NULL);

	// returns current selected node's URI.
	// if no node is selected, return NULL.
	// Be careful the return value may be released by MemoSelectView when closing tree
	// so if you want to keep it, copy the instance in the situation.
	const TomboURI *GetCurrentSelectedURI();

	TreeViewItem *GetTVItem(HTREEITEM h);

	BOOL IsCliped(TreeViewItem* p) { return p == pClipItem; }
	LONG GetItem(TV_ITEM *p) { return TreeView_GetItem(hViewWnd, p); }

	LPTSTR GeneratePath(HTREEITEM hItem, LPTSTR pBuf, DWORD nSiz);

	// Choose specified notes. if tree is collapsed, expand one.
	HTREEITEM ShowItemByURI(const TomboURI *pURI, BOOL bSelChange = TRUE, BOOL bOpenNotes = TRUE);

	/////////////////////////////
	// Virtual folder
	BOOL InsertVirtualFolder(const VFInfo *pInfo);

	void CloseVFRoot();

	/////////////////////////////
	// Control menu
	void ControlMenu();
};

/////////////////////////////////////////
// Node status definitions
/////////////////////////////////////////

// initialized
#define MEMO_VIEW_STATE_INIT			1

// in clipboard
#define MEMO_VIEW_STATE_CLIPED_SET		(1 << 1)
#define MEMO_VIEW_STATE_CLIPED_CLEAR	(1 << 2)

// opened
#define MEMO_VIEW_STATE_OPEN_SET		(1 << 3)
#define MEMO_VIEW_STATE_OPEN_CLEAR		(1 << 4)


#endif