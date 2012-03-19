#ifndef YAEDIT_H
#define YAEDIT_H

class YAEditDoc;
class LineWrapper;
class LineChunk;
class FontWidthCache;
class YAEditView;
class LineManager;
class PhysicalLineManager;
struct YAEContextMenu;
struct YAEditViewColorDef;

struct YAEditViewColorDef {
	COLORREF rgbEol;
	COLORREF rgbLEol;
	COLORREF rgbTab;
	COLORREF rgbEof;
	COLORREF rgbForeground;
	COLORREF rgbBackground;
};

//////////////////////////////////////////////////
// callback class
//////////////////////////////////////////////////

class YAEditCallback {
public:
	// called when get screen forcus
	virtual void OnGetFocus() = 0;

	// called from YAE when the document is modified.
	virtual void ChangeModifyStatusNotify(BOOL bStatus) = 0;

	// called from YAE when the document read only flag is changed
	virtual void ChangeReadOnlyStatusNotify(BOOL bStatus) = 0;

	virtual void OnContextMenu(HWND hWnd, WORD x, WORD y) = 0;
};

//////////////////////////////////////////////////
// Controller class for YAE
//////////////////////////////////////////////////
// YAEdit is abstract class for hiding YAE internal methods.

class YAEdit {
public:

	virtual BOOL Create(HINSTANCE hInst, HWND hWnd, DWORD nId, RECT &r, BOOL bWrap, YAEditViewColorDef c) = 0;
	virtual void SetFocus() = 0;
	virtual void SetFont(HFONT hFont) = 0;

	virtual void ResizeWindow(int x, int y, int width, int height) = 0;
	virtual BOOL Show(int nCmdShow) = 0;

	///////////////////////////////////////
	// document related funcs

	virtual YAEditDoc *GetDoc() = 0;
	virtual YAEditDoc *SetDoc(YAEditDoc *pNewDoc) = 0;


	///////////////////////////////////////
	// caret position/selection related funcs

	virtual DWORD GetCaretPos() = 0;
	virtual void SetCaretPos(DWORD n) = 0;

	virtual void SetSelectRegion(DWORD nStart, DWORD nEnd) = 0;


	///////////////////////////////////////
	// exported commands

	virtual void CmdReplaceString(LPCTSTR p) = 0;
	virtual void CmdBackSpace() = 0;

	virtual void CmdUndo() = 0;

	virtual void CmdCut() = 0;
	virtual void CmdCopy() = 0;
	virtual void CmdPaste() = 0;

	virtual void CmdSelAll() = 0;
	virtual void CmdToggleWrapMode(BOOL bFold) = 0;


	///////////////////////////////////////
	// color
	virtual void SetColorDef(const YAEditViewColorDef& cdef) = 0;

	///////////////////////////////////////
	// register window class
	static BOOL RegisterClass(HINSTANCE hInst);

	///////////////////////////////////////
	// factory method

	// get YAEdit instance
	static YAEdit *GetInstance(YAEditCallback *pCallback, COLORREF cBk);

	// get YAEditDoc instance
	virtual YAEditDoc *CreateDocument(const char *pStr, YAEditCallback*pCb) = 0;
};


//////////////////////////////////////////////////
// Controller class for YAE implementation
//////////////////////////////////////////////////

class YAEditListener {
public:
	virtual BOOL UpdateNotify(PhysicalLineManager *pPhMgr, const Region *pOldRegion, const Region *pNewRegion, DWORD nBefPhLines, DWORD nAftPhLines, DWORD nAffeLines) = 0;
};

//////////////////////////////////////////////////
// YAE implementation
//////////////////////////////////////////////////

class YAEditImpl : public YAEdit, YAEditListener {
protected:
	///////////////////////////////////////
	// callback handler
	YAEditCallback *pCallback;

	///////////////////////////////////////
	// window related members
	HDC hCommonDC;
	HINSTANCE hInstance;
	HBRUSH hBkBrush;

	///////////////////////////////////////
	// VMC related members
	YAEditDoc *pDoc;
	YAEditView *pView;

	///////////////////////////////////////
	// key related members
#if defined(PLATFORM_WIN32)
	char aKeyBuffer[3];
#endif

	///////////////////////////////////////
	// line management members

	LineManager *pLineMgr;
	LineWrapper *pWrapper;

	BOOL bScrollTimerOn;
	POINT ptMousePos;
	BOOL bMouseDown;

	BOOL bWrapLine;
	BOOL bInsertMode;

	// value is by logical coordinate, 
	// (nSelStartCol, nSelStartRow) < (nSelEndCol, nSelEndRow) is always TRUE.
	Region rSelRegion;
	BOOL bForwardDrag;

protected:
	///////////////////////////////////////
	// select region
	void SetSelectionFromPoint(int xPos, int yPos);
	void UpdateSelRegion();

	////////////////////////////////////////////////////
	// line operation helper
	BOOL ReplaceText(const Region &r, LPCTSTR pText);

	////////////////////////////////////////////////////
	// Region related members
	BOOL GetRegionString(LPTSTR pBuf);
	DWORD GetRegionSize();

	void ClearRegion();
	void ClearSelectedRegion();

	BOOL SetWrapper();

public:

	///////////////////////////////////////
	// ctor & initialize
	YAEditImpl(YAEditCallback *pCb, COLORREF bkColor);
	virtual ~YAEditImpl();

	BOOL Create(HINSTANCE hInst, HWND hWnd, DWORD nId, RECT &r, BOOL bWrap, YAEditViewColorDef c);
	void SetFocus();

	/////////////////////////////////
	// Event handler

	void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);

	BOOL OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam); // if FALSE, call default proc
	void OnChar(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnLButtonDblClick(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnRbuttonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void OnVScroll(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnHScroll(HWND hWnd, WPARAM wParam, LPARAM lParam);

#if defined(PLATFORM_WIN32)
	void OnMouseWheel(HWND hWnd, WPARAM wParam, LPARAM lParam);
#endif

	void OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnResize(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void OnEraseBkgnd(HWND hWnd, WPARAM wParam, LPARAM lParam);

	/////////////////////////////////
	// forcus window
	void OnSetFocus();
	void OnKillFocus();
	void OnGetFocus();

	/////////////////////////////////
	// application funcs.
	YAEditDoc *SetDoc(YAEditDoc *pNewDoc);
	YAEditDoc *GetDoc() { return pDoc; }
	LineManager *GetLineMgr() { return pLineMgr; }

	///////////////////////////////////////
	// cursor moving funcs.

	DWORD GetCaretPos();
	void SetCaretPos(DWORD n);

	/////////////////////////////////
	// Commands

	void CmdNOP();

	void CmdBackSpace();
	void CmdDeleteChar();
	void CmdMoveRight();
	void CmdMoveLeft();
	void CmdMoveUp();
	void CmdMoveDown();
	void CmdMoveEOL();
	void CmdMoveTOL();
	
	void CmdSelRight();
	void CmdSelLeft();
	void CmdSelUp();
	void CmdSelDown();

	void CmdSelEndOfLogicalLine();
	void CmdSelTopOfLogicalLine();
	void CmdSelTopOfDoc();
	void CmdSelEndOfDoc();

	void CmdScrollUp();
	void CmdScrollDown();

	void CmdReplaceString(LPCTSTR p);
	void CmdCut();
	void CmdCopy();
	void CmdPaste();

	void CmdSelAll();
	void CmdUndo();

	void CmdToggleReadOnly();
	void CmdToggleWrapMode(BOOL bWrap);
	void CmdToggleInsertMode();

	/////////////////////////////////
	// Move/Resize window
	void ResizeWindow(int x, int y, int width, int height);
	BOOL Show(int nCmdShow);

	/////////////////////////////////
	// YAEditDoc callback

	void RequestRedraw(DWORD nLineNo, WORD nLeftPos, BOOL bToBottom);
	void RequestRedrawRegion(const Region *pRegion);

	/////////////////////////////////
	// Line wrapping 
	LineWrapper *GetWrapper() { return pWrapper; }
	DWORD GetLineWidth(DWORD nOffset, LPCTSTR pStr, DWORD nLen);

	/////////////////////////////////
	// Clipboard
	BOOL CopyToClipboard();
	BOOL InsertFromClipboard();

	////////////////////////////////////////////////////
	// Region related members

	BOOL IsRegionSelected() { return rSelRegion.posStart != rSelRegion.posEnd; }
	BOOL IsSelRegionOneLine() { return rSelRegion.posStart.row == rSelRegion.posEnd.row; }
	const Region& SelectedRegion() { return rSelRegion; }

	// Select [selected region] + nCurrent
	void ExtendSelectRegion(const Coordinate &nCurrent, Coordinate *pPrev);

	void SetSelectRegion(DWORD nStart, DWORD nEnd);

	////////////////////////////////////////////////////
	// callback from Document
	BOOL UpdateNotify(PhysicalLineManager *pPhMgr, const Region *pOldRegion, const Region *pNewRegion, DWORD nBefPhLines, DWORD nAftPhLines, DWORD nAffeLines);

	////////////////////////////////////////////////////
	// font
	void SetFont(HFONT hFont);

	////////////////////////////////////////////////////
	// color
	void SetBackgroundColor(COLORREF cBk);
	void SetColorDef(const YAEditViewColorDef& cdef);

	////////////////////////////////////////////////////
	// data access from YAEditView
	BOOL GetLgLineChunk(DWORD nLineNo, LineChunk *pChunk);
	DWORD GetPrevOffset(DWORD nLineNo, DWORD nCurrentPos);

	////////////////////////////////////////////////////
	// 
	YAEditDoc *CreateDocument(const char *pStr, YAEditCallback*pCb);
};
#endif