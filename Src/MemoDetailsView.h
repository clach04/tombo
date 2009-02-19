#ifndef MEMODETAILSVIEW_H
#define MEMODETAILSVIEW_H

class MemoDetailsView;
class SearchEngineA;
class TString;
class TomboURI;
class MemoManager;

///////////////////////////////////////
// Edit view abstraction
///////////////////////////////////////

class MemoDetailsView {
protected:
	TomboURI *pCurrentURI;
	MemoManager *pManager;
public:

	MemoDetailsView(MemoManager *pMgr);
	virtual ~MemoDetailsView();

	virtual BOOL Create(LPCTSTR pName, RECT &r, HWND hParent, HINSTANCE hInst, HFONT hFont) = 0;

	virtual void SetTabstop() = 0;
	virtual BOOL SetFolding(BOOL bFold) = 0;
	virtual void SetReadOnly(BOOL bReadOnly) = 0;
	virtual BOOL IsReadOnly() = 0;

	virtual void SetModifyStatus() = 0;

	virtual void SetMDSearchFlg(BOOL bFlg) = 0;

	virtual BOOL Show(int nCmdShow) = 0;
	virtual void SetFocus() = 0;
	virtual void SetFont(HFONT hFont) = 0;
	virtual void MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight) = 0;

	virtual BOOL OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) = 0;
	virtual BOOL OnHotKey(HWND hWnd, WPARAM wParam) = 0;
	virtual void OnGetFocus() = 0;

	virtual BOOL SetMemo(LPCTSTR pMemo, DWORD nPos, BOOL bReadOnly) = 0;
	virtual LPTSTR GetMemo() = 0;
	virtual BOOL IsModify() = 0;
	virtual void ResetModify() = 0;

	virtual DWORD GetCursorPos() = 0;
	virtual DWORD GetInitialPos() = 0;

	virtual void SelectAll() = 0;

	BOOL Search(BOOL bFirstSearch, BOOL bForward, BOOL bNFMsg, BOOL bSearchFromTop);

	BOOL Save(const TomboURI *pCurrentURI, TomboURI *pNewURI, TString *pNewHeadLine, LPCTSTR pText);

	const TomboURI *GetCurrentURI();
	void SetCurrentNote(const TomboURI *pURI);

	// Is the note displayed in details view?
	BOOL IsNoteDisplayed(const TomboURI *pURI);

	BOOL StoreCursorPos();
	BOOL DiscardMemo();

	BOOL LoadNote(const TomboURI *pURI);

	virtual BOOL ReplaceText(LPCTSTR p) = 0;
	virtual void SetSelectRegion(DWORD nStart, DWORD nEnd) = 0;

	void InsertDate1();
	void InsertDate2();
};

//////////////////////////////////////////
// Edit view
//////////////////////////////////////////

class SimpleEditor : public MemoDetailsView {
	HWND hViewWnd;		// The window handle used now(hViewWnd_fd or hViewWnd_nf)
	HWND hViewWnd_fd;	// The window created by wrapping options
	HWND hViewWnd_nf;	// The window created by no wrapping options

	DWORD nID, nID_nf;

	DWORD nLeftOffset;

	BOOL bShowStatus;	// Is view displayed?

	BOOL bReadOnly;		// is read only mode?

	DWORD nInitialPos;	// cursor position when open this note.

public:

	///////////////////////
	// Initialize

	SimpleEditor(MemoManager *pMgr);
	BOOL Init(DWORD nID, DWORD nID_nf);
	BOOL Create(LPCTSTR pName, RECT &r, HWND hParent, HINSTANCE hInst, HFONT hFont);

	static BOOL RegisterClass(HINSTANCE hInst);

	///////////////////////
	// Properties

	void SetTabstop();				// Tab stop
	BOOL SetFolding(BOOL bFold);	// Change wrapping

	void SetReadOnly(BOOL bReadOnly);
	BOOL IsReadOnly() { return bReadOnly; }

	void SetModifyStatus();

	////////////////////////
	// Message handler

	BOOL Show(int nCmdShow);
	void SetFocus() { ::SetFocus(hViewWnd); }
	void SetFont(HFONT hFont);
	void MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight);

	BOOL OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL OnHotKey(HWND hWnd, WPARAM wParam);
	UINT OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void OnGetFocus();

	////////////////////////////
	// Data access

	BOOL SetMemo(LPCTSTR pMemo, DWORD nPos, BOOL bReadOnly);
	LPTSTR GetMemo();
	BOOL IsModify() { if (hViewWnd) return SendMessage(hViewWnd, EM_GETMODIFY, 0, 0); else return FALSE; }
	void ResetModify() { SendMessage(hViewWnd, EM_SETMODIFY, (WPARAM)(UINT)FALSE, 0); }

	void SetMDSearchFlg(BOOL bFlg);


	DWORD GetCursorPos();
	DWORD GetInitialPos() { return nInitialPos; }

	void SelectAll();

	BOOL ReplaceText(LPCTSTR p);
	void SetSelectRegion(DWORD nStart, DWORD nEnd);
};

#endif
