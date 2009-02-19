#ifndef TREEVIEWITEM_H
#define TREEVIEWITEM_H

#include <commctrl.h>

class MemoSelectView;
class MemoManager;
class VFDirectoryGenerator;
class VFStore;
class VFStream;
class MemoLoctor;
class TreeViewVirtualFolder;
class TString;
class VFManager;
class VFInfo;
class TomboURI;

#include "MemoManager.h"
#include "VarBuffer.h"

/////////////////////////////////////////////
//  TreeView Node
/////////////////////////////////////////////

class TreeViewItem {
	BOOL bHasMultiItem;
	HTREEITEM hItem;

protected:
	class Locator {
		TomboURI *pURI;
	public:
		Locator() : pURI(NULL) {}
		~Locator();
		const TomboURI *getURI() const { return pURI; }
		void set(const TomboURI *pURI);
	};

	Locator loc;

public:
	///////////////////////////////////////////////////////
	// constants

	enum OpType {
		OpDelete	= (1 << 0),
		OpRename	= (1 << 1),
		OpEncrypt	= (1 << 2),
		OpDecrypt	= (1 << 3),
		OpNewMemo	= (1 << 4),
		OpNewFolder = (1 << 5),
		OpCut		= (1 << 6),
		OpCopy		= (1 << 7),
		OpPaste		= (1 << 8),
		OpGrep		= (1 << 9),
		OpLink		= (1 << 10)
	};

	///////////////////////////////////////////////////////
	// TreeViewItem specific methods

	TreeViewItem(BOOL bMultiItem);
	virtual ~TreeViewItem();

	HTREEITEM GetViewItem();
	void SetViewItem(HTREEITEM h);

	BOOL HasMultiItem() { return bHasMultiItem; }

	////////////////////////////////
	// location related members

	void SetURI(const TomboURI *p);
	virtual const TomboURI *GetRealURI() const { return loc.getURI(); }

	///////////////////////////////////////////////////////

	// is operation enabled?
	virtual BOOL IsOperationEnabled(MemoSelectView *pView, OpType op);

	//  node operators
	virtual BOOL Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName) = 0;
	virtual BOOL Delete(MemoManager *pMgr, MemoSelectView *pView) = 0;
		// "this" object is not deleted, so delete manually if return value is TRUE.
	virtual BOOL Encrypt(MemoManager *pMgr, MemoSelectView *pView) = 0;
	virtual BOOL Decrypt(MemoManager *pMgr, MemoSelectView *pView) = 0;
	virtual BOOL ExecApp(MemoManager *pMgr, MemoSelectView *pView, ExeAppType nType) = 0;

	// Move/Copy this item to current selected item
	virtual BOOL Move(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr) = 0;
	virtual BOOL Copy(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr) = 0;

	// Compare item order
	virtual DWORD ItemOrder() = 0;
	virtual DWORD GetIcon(MemoSelectView *pView, DWORD nStatus) = 0;

	// Get path information

	virtual BOOL OpenMemo(MemoSelectView *pView, DWORD nOption);
	virtual BOOL LoadMemo(MemoSelectView *pView, BOOL bAskPass);
	virtual BOOL IsUseDetailsView();
};

/////////////////////////////////////////////
//  File
/////////////////////////////////////////////
class TreeViewFileItem : public TreeViewItem {
protected:

	BOOL CopyMove(BOOL bCopy, MemoManager *pMgr, MemoSelectView *pView);
public:
	TreeViewFileItem();
	~TreeViewFileItem();

	////////////////////////////////
	// inherited methods
	BOOL IsOperationEnabled(MemoSelectView *pView, OpType op);

	BOOL Delete(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Encrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Decrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName);
	BOOL ExecApp(MemoManager *pMgr, MemoSelectView *pView, ExeAppType nType);

	BOOL Move(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr);
	BOOL Copy(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr);

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);
	DWORD ItemOrder();

	BOOL OpenMemo(MemoSelectView *pView, DWORD nOption);
	BOOL LoadMemo(MemoSelectView *pView, BOOL bAskPass);

};

/////////////////////////////////////////////
//  Real folder
/////////////////////////////////////////////

class TreeViewFolderItem : public TreeViewItem {
	BOOL CopyMove(BOOL bCopy, MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr);
public:
	TreeViewFolderItem();

	////////////////////////////////
	// multi node methods

	virtual BOOL Expand(MemoSelectView *pView);

	////////////////////////////////
	// inherited methods

	BOOL IsOperationEnabled(MemoSelectView *pView, OpType op);

	BOOL Rename(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR pNewName);
	BOOL Delete(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Encrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL Decrypt(MemoManager *pMgr, MemoSelectView *pView);
	BOOL ExecApp(MemoManager *pMgr, MemoSelectView *pView, ExeAppType nType);

	BOOL Move(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr);
	BOOL Copy(MemoManager *pMgr, MemoSelectView *pView, LPCTSTR *ppErr);

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);
	DWORD ItemOrder();
};

/////////////////////////////////////////////
//  File link
/////////////////////////////////////////////
class TreeViewFileLink : public TreeViewFileItem {
public:

	////////////////////////////////
	// inherited methods

	BOOL IsOperationEnabled(MemoSelectView *pView, OpType op);
};

/////////////////////////////////////////////
//  Virtual Folder (Root)
/////////////////////////////////////////////

class TreeViewVirtualFolderRoot : public TreeViewFolderItem {
	VFManager *pManager;
	TomboURI *pDefaultURI;
public:
	TreeViewVirtualFolderRoot();
	~TreeViewVirtualFolderRoot();
	BOOL Init(const TomboURI *pURI, VFManager *pManager);

	////////////////////////////
	// class specific methods

	BOOL AddSearchResult(MemoSelectView *pView, const VFInfo *p);
	BOOL InsertVirtualFolder(MemoSelectView *pView, LPCTSTR pName, VFDirectoryGenerator *pGen, VFStore *pStore);

	////////////////////////////////
	// multi node methods

	BOOL Expand(MemoSelectView *pView);

	////////////////////////////////
	// inherited methods

	BOOL IsOperationEnabled(MemoSelectView *pView, OpType op);

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);

	const TomboURI *GetRealURI() const;
};


/////////////////////////////////////////////
//  Virtual Folder (Non-root)
/////////////////////////////////////////////

class TreeViewVirtualFolder : public TreeViewFolderItem {
protected:
	VFDirectoryGenerator *pGenerator;
	VFStore *pStore;
	TomboURI *pDefaultURI;
public:
	TreeViewVirtualFolder();
	~TreeViewVirtualFolder();

	////////////////////////////
	// class specific methods

	// pGen's life scope is control under TreeViewVirtualFolder.
	// don't delete pGen after calling SetGenerator.
	BOOL SetGenerator(VFDirectoryGenerator *pGen);

	// pStore's  life scope is control under TreeViewVirtualFolder.
	// don't delete pStore after calling SetStore.
	BOOL SetStore(VFStore *pStore);

	////////////////////////////////
	// multi node methods

	BOOL Expand(MemoSelectView *pView);

	////////////////////////////////
	// inherited methods

	BOOL IsOperationEnabled(MemoSelectView *pView, OpType op);

	DWORD GetIcon(MemoSelectView *pView, DWORD nStatus);

	const TomboURI *GetRealURI() const;
};


#endif
