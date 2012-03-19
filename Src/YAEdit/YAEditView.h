#ifndef YAEDITVIEW_H
#define YAEDITVIEW_H

class YAEditImpl;
class YAEditDoc;

class YAEditView {
	YAEditImpl *pCtrl;

	///////////////////////////////////////
	// cursor(caret) related members

	BOOL bShowCaret;	// is caret shown?

protected:
	///////////////////////////////////////
	// proxy to YAEdit

	DWORD GetMaxLine();

	LONG nAveCharWidth;// average width of 1 letter
	DWORD nLineH;		// 1 line height
	DWORD nPageHeight;	// number of lines in the window
	DWORD nBaseLineNo;	// top page position

	DWORD nHorizPageScrollDelta;
	DWORD nColOffset;	// offset of horiz scroll

	DWORD nTabWidth;	// tab width(pixels)

	FontWidthCache *pFontCache;

	///////////////////////////////////////
	// cursor(caret) related members

	DWORD nCursorColPos;// cursor col position (by pixel)
	DWORD nCursorRow;	// cursor row position(line no of the document)
	DWORD nCursorCol;	// cursor col position (by char)
						// in DBCS coding, nCusorCol points bytes.
						// in UCS-2 coding, nCursorCol points number of the letters.

	///////////////////////////////////////
	// Drawing

	BOOL PaintLine(HDC hDC, LineChunk *pChunk, const LPRECT pRect, DWORD nSelStart, DWORD nSelEnd);
	void DrawEndLineMark(HDC hDC, DWORD w, DWORD nMaxLine, RECT *pRect, LineChunk *pChunk);
	BOOL DirectPaintLine(DWORD nLineNo);
	void CalcInvalidateArea(DWORD nLine, DWORD nStart, DWORD nEnd);

	RECT rClientRect;

	///////////////////////////////////////
	// Color
	YAEditViewColorDef colorDef;

public:
	HWND hViewWnd;

	LONG nMaxCharWidth;
	DWORD nMaxWidthPixel; // max line width(pixels) in the document

	HFONT hFont;
	LOGFONT lfLogFont;

	///////////////////////////////////////
	// initializing

	YAEditView(YAEditImpl *p, const YAEditViewColorDef& colorDef);
	~YAEditView();

	BOOL ResetPosition();

	void ResizeNotify();
	BOOL ResetScrollbar();

	///////////////////////////////////////
	// checking funcs.
	BOOL IsCursorInPageBottom();
	BOOL IsCursorInPageTop();
	BOOL IsCursorInDocTop();
	BOOL IsCursorInDocBottom();

	BOOL IsCursorInDisplay();
	BOOL IsLineDisplay(DWORD nLgLineNo);

	///////////////////////////////////////
	// cursor control 

	void SetCaretPos();
	void ScrollCaret();
	void SetNearCursorPos(WORD xPos, DWORD nYLines);

	void SetCaretPosition(const Coordinate& pos);
	Coordinate GetCaretPosition() { return Coordinate(nCursorCol, nCursorRow); }

	/////////////////////////////////
	// Redrawing

	void PaintRect(HDC hDC, const RECT &r);
	void RequestRedraw(DWORD nLineNo, WORD nLeftPos, BOOL bToBottom);
	void RequestRedrawWithLine(DWORD nLineNo, DWORD nNumLine);
	void RequestRedrawRegion(const Region *pRegion);
	void RedrawAllScreen();

	///////////////////////////////////////
	// Coordinate conversion
	// logical line No <-> display line no / pixel position

	DWORD LgLineNoToDpLineNo(DWORD nLgLineNo);
	DWORD DpLineNoToDpLinePixel(DWORD nDpLineNo);
	DWORD DpLineNoToLgLineNo(DWORD nDpLineNo);
	DWORD DpLinePixelToLgLineNo(DWORD nDpLinePixel);

	///////////////////////////////////////
	// message handler helper
	void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);

	///////////////////////////////////////
	// Focus
	void OnSetFocus();
	void OnKillFocus();

	///////////////////////////////////////
	// Scroll

	void SetScrollVertPos(DWORD nPos, BOOL bFullRewrite);
	void SetScrollVertByOffset(int nLineOffset);
	void SetScrollHorizPos(int nPos);

	void Next1L();
	void Prev1L();
	void NextPage();
	void PrevPage();

	void ScrollRight1L();
	void ScrollLeft1L();
	void ScrollRight1P();
	void ScrollLeft1P();

	///////////////////////////////////////
	// cursor moving funcs.

	void MoveRight();
	void MoveLeft();
	void MoveUp();
	void MoveDown();
	void MoveEOL();
	void MoveTOL();

	///////////////////////////////////////
	// line width related funcs.

	DWORD GetLineWidth(DWORD nOffset, LPCTSTR pStr, DWORD nLen);

	// Check all LineManager data and get the value of max width.
	void UpdateMaxLineWidth();

	///////////////////////////////////////
	// font related funcs

	void ResetFontInfo();
	void SetFont(HFONT hFont);

	void SetColorDef(const YAEditViewColorDef& cdef);

	///////////////////////////////////////
	// scroll bar status
	BOOL IsVertScrollbarDisplayed();

	///////////////////////////////////////
	// accessor
	DWORD GetPageHeight() { return nPageHeight; }
	DWORD GetBaseLineNo() { return nBaseLineNo; }
	const RECT& GetViewClientRect() { return rClientRect; }

	// just debugging
	FontWidthCache *GetFontCache() { return pFontCache; }
};

#endif