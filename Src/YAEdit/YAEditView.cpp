#include <windows.h>
#include <tchar.h>
#if !defined(PLATFORM_PSPC)
#include <imm.h>
#endif
#if defined(PLATFORM_WIN32)
#include <zmouse.h>
#endif
#include "resource.h"
#include "Region.h"
#include "YAEdit.h"
#include "YAEditView.h"
#include "YAEditDoc.h"
#include "VarBuffer.h"
#include "LineManager.h"
#include "LineWrapper.h"
#include "Logger.h"
#include "FontWidthCache.h"
#include "TomboLib/UniConv.h"
#include "TomboLib/Clipboard.h"
#include "TString.h"

#define CHAR_TYPE_NORMAL 0
#define CHAR_TYPE_SPACE 1
#define CHAR_TYPE_SPACE_ZENKAKU 2
#define CHAR_TYPE_TAB 3

/////////////////////////////////////////////////////////////////////////////
// initialize
/////////////////////////////////////////////////////////////////////////////

YAEditView::YAEditView(YAEditImpl *p, const YAEditViewColorDef& cdef) : pCtrl(p), pFontCache(NULL), hFont(NULL)  {
	colorDef = cdef;
}

YAEditView::~YAEditView()
{
	if (pFontCache) delete pFontCache;
}

BOOL YAEditView::ResetPosition()
{
	nCursorColPos = nCursorCol = 0;
	nBaseLineNo = nCursorRow = 0;
	nColOffset = 0;

	bShowCaret = FALSE;
	return TRUE;
}

void YAEditView::ResizeNotify()
{
	GetClientRect(hViewWnd, &rClientRect);
	nPageHeight = (rClientRect.bottom - rClientRect.top) / nLineH;
}

BOOL YAEditView::ResetScrollbar()
{
	nHorizPageScrollDelta = (rClientRect.right - rClientRect.left) / 4;

	// set scroll info
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;

	// vert scroll bar
	si.nMin = 0;
	si.nPos = nBaseLineNo;
	si.nMax = GetMaxLine() - 1;
	si.nPage = nPageHeight;
	::SetScrollInfo(hViewWnd, SB_VERT, &si, TRUE);

	// horiz scroll bar
	si.nMin = 0;
	si.nMax = nMaxWidthPixel - nMaxCharWidth;
	si.nPage = (rClientRect.right - rClientRect.left - nMaxCharWidth);
	si.nPos = nColOffset;
	::SetScrollInfo(hViewWnd, SB_HORZ, &si, TRUE);
	
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// Draw line element helper functions
/////////////////////////////////////////////////////////////////////////////

static void DrawZenkakuSpace(HDC hDC, RECT *pRect, COLORREF color)
{
	HPEN hPen = CreatePen(PS_SOLID, 0, color);
	HGDIOBJ hOld = SelectObject(hDC, hPen);

	POINT pt[5];
	pt[0].x = pRect->left + 3;	pt[0].y = pRect->top + 3;
	pt[1].x = pt[0].x;			pt[1].y = pRect->bottom - 3;
	pt[2].x = pRect->right - 3;	pt[2].y = pt[1].y;
	pt[3].x = pt[2].x;			pt[3].y = pRect->top + 3;
	pt[4].x = pt[0].x;			pt[4].y = pt[0].y;
	Polyline(hDC, pt, 5);

	SelectObject(hDC, hOld);
	DeleteObject(hPen);
}

static void DrawTab(HDC hDC, RECT *pRect, COLORREF color)
{
	COLORREF crDefault = SetTextColor(hDC, color);
	DrawText(hDC, TEXT(">"), 1, pRect, DT_TOP | DT_SINGLELINE);
	SetTextColor(hDC, crDefault);
}

static DWORD CheckCharType(LPCTSTR pLine, LPDWORD pNumChar)
{
#if defined(PLATFORM_WIN32)
	if (*pLine == (char)0x81 &&
		*(pLine + 1) == (char)0x40) {
		*pNumChar = 2;
		return CHAR_TYPE_SPACE_ZENKAKU;
	}
#endif
	if (*pLine == TEXT(' ')) {
		*pNumChar = 1;
		return CHAR_TYPE_SPACE;
	}
	if (*pLine == TEXT('\t')) {
		*pNumChar = 1;
		return CHAR_TYPE_TAB;
	}

#if defined(PLATFORM_WIN32)
	*pNumChar = (IsDBCSLeadByte(*pLine) ? 2 : 1);
#else
	*pNumChar = 1;
#endif
	return CHAR_TYPE_NORMAL;
}

static void DrawEOL(HDC hDC, LPRECT pRect, const YAEditViewColorDef& cdef)
{
	WORD nHalfX = (WORD)((pRect->right - pRect->left)/ 2);
	WORD nHalfY = (WORD)((pRect->bottom - pRect->top) / 2);

	int nW = pRect->right - pRect->left;
	int nH = pRect->bottom - pRect->top;
	WORD w = ((nW < nH ? nW : nH) * 8) / 10;
	WORD rX = (nW - w) / 2;
	WORD rY = (nH - w) / 2;

	HPEN hPen = CreatePen(PS_SOLID, 0, cdef.rgbEol);
	HGDIOBJ hOld = SelectObject(hDC, hPen);
	SetBkColor(hDC, cdef.rgbBackground);

	POINT pt[3];
	pt[0].x = pRect->right - rX;	pt[0].y = pRect->top + rY;
	pt[1].x = pt[0].x;				pt[1].y = pRect->bottom - rY;
	pt[2].x = pRect->left + rX;		pt[2].y = pt[1].y;
	Polyline(hDC, pt, 3);

	for (int x = 0; x <= 2; x++) {
		for (int y = -x; y <= x; y++) {
			SetPixel(hDC, pt[2].x + x, pt[2].y + y, cdef.rgbEol);
		}
	}
	SelectObject(hDC, hOld);
	DeleteObject(hPen);
}

static void DrawLEOL(HDC hDC, LPRECT pRect, const YAEditViewColorDef cdef)
{
	WORD nHalfX = (WORD)((pRect->right - pRect->left)/ 2);

	int nH = pRect->bottom - pRect->top;
	WORD w = (nH * 7) / 10;
	WORD rY = (nH - w) / 2;

	HPEN hPen = CreatePen(PS_SOLID, 0, cdef.rgbLEol);
	HGDIOBJ hOld = SelectObject(hDC, hPen);

	POINT pt[2];
	pt[0].x = pRect->right + nHalfX; pt[0].y = pRect->top + rY;
	pt[1].x = pt[0].x; pt[1].y = pRect->bottom - rY;
	Polyline(hDC, pt, 2);

	for (int y = -2; y <= 0; y++) {
		for (int x = y; x <= -y; x++) {
			SetPixel(hDC, pt[1].x + x, pt[1].y + y, cdef.rgbLEol);
		}
	}
	SelectObject(hDC, hOld);
	DeleteObject(hPen);
}

void YAEditView::DrawEndLineMark(HDC hDC, DWORD wStartPos, DWORD nMaxLine, RECT *pRect, LineChunk *pChunk)
{
	DWORD nLineNo = pChunk->GetLineNo();

	RECT r2;
	r2.left = (WORD)wStartPos;
	r2.top = pRect->top;
	r2.bottom = r2.top + nLineH;

	if (nLineNo + 1 == nMaxLine) {
		// EOF
		r2.right = r2.left + 100;
		SetTextColor(hDC, colorDef.rgbEof);
		SetBkColor(hDC, colorDef.rgbBackground);
		DrawText(hDC, TEXT("[EOF]"), 5, &r2, DT_TOP | DT_SINGLELINE);
	} else {
		// EOL
		if (pChunk->IsContLine()) {
			// Logical EOL
			r2.right = r2.left + 5;
			DrawLEOL(hDC, &r2, colorDef);
		} else {
			// Physical EOL
			r2.right = r2.left + nAveCharWidth;
			DrawEOL(hDC, &r2, colorDef);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Draw one line 
/////////////////////////////////////////////////////////////////////////////

BOOL YAEditView::PaintLine(HDC hDC, LineChunk *pChunk, const LPRECT pRect, DWORD nSelStart, DWORD nSelEnd)
{
	HFONT hOldFont = NULL;
	if (hFont) {
		hOldFont = (HFONT)SelectObject(hDC, hFont);
	}

	DWORD nLineNo = pChunk->GetLineNo();

	BOOL bInvertText = FALSE;

	LPCTSTR pLine = pChunk->GetLineData();
	DWORD nLineLen = pChunk->LineLen();

	// Draw line
	DWORD nBaseX = pRect->left;

	DWORD nCurrentCharPos;	// which char is drawing
	DWORD nOffset;			// where the char should draw

	DWORD nNumChar;		// number(length) of char
	DWORD nCharWidth;	// number of pixels drawn

	DWORD nSpecialChar;	// character type

	nOffset = 0;
	nCurrentCharPos = 0; 

	RECT r = *pRect;
	r.bottom = r.top + nLineH;

	while(nCurrentCharPos < nLineLen) {
		r.left = nBaseX + nOffset;

		if (nSelStart <= nCurrentCharPos && nCurrentCharPos < nSelEnd) {
			bInvertText = TRUE;
		} else {
			bInvertText = FALSE;
		}

		LPCTSTR pChunkData = pLine + nCurrentCharPos;
		nSpecialChar = CheckCharType(pChunkData, &nNumChar);

		switch(nSpecialChar) {
		case CHAR_TYPE_SPACE:
			if (bInvertText) {
				HBRUSH hBrush = CreateSolidBrush(colorDef.rgbForeground);
				HGDIOBJ hDefaultBrush = SelectObject(hDC, hBrush);
				Rectangle(hDC, r.left, r.top, r.left + GetLineWidth(0, TEXT(" "), 1), r.bottom);
				SelectObject(hDC, hDefaultBrush);
				DeleteObject(hBrush);
			}
			nCharWidth = GetLineWidth(nOffset, pChunkData, nNumChar);
			break;
		case CHAR_TYPE_TAB:
			nCharWidth = GetLineWidth(nOffset, TEXT("\t"), 1);
			if (bInvertText) {
				HBRUSH hBrush = CreateSolidBrush(colorDef.rgbForeground);
				HGDIOBJ hDefaultBrush = SelectObject(hDC, hBrush);
				Rectangle(hDC, r.left, r.top, r.left + nCharWidth, r.bottom);
				SelectObject(hDC, hDefaultBrush);
				DeleteObject(hBrush);
			} else {
				r.right = r.left + nCharWidth;
				DrawTab(hDC, &r, colorDef.rgbTab);
			}
			break;

#if defined(PLATFORM_WIN32)
		case CHAR_TYPE_SPACE_ZENKAKU:
			nCharWidth = GetLineWidth(0, TEXT("@"), 2);
			r.right = r.left + nCharWidth;
			DrawZenkakuSpace(hDC, &r, RGB(64, 128, 128));
			break;
#endif
		default:	// normal char
			{
				if (bInvertText) {
					SetTextColor(hDC, colorDef.rgbBackground);
					SetBkColor(hDC, colorDef.rgbForeground);
				} else {
					SetTextColor(hDC, colorDef.rgbForeground);
					SetBkColor(hDC, colorDef.rgbBackground);
				}
				ExtTextOut(hDC, r.left, pRect->top, 0, NULL, pChunkData, nNumChar, NULL);
				nCharWidth = GetLineWidth(nOffset, pChunkData, nNumChar);
			}
		}

		nCurrentCharPos += nNumChar;
		nOffset += nCharWidth;
	}

	// Draw End of line mark
	DrawEndLineMark(hDC, nOffset, GetMaxLine(), pRect, pChunk);
	if (hFont) {
		SelectObject(hDC, hOldFont);
	}
	return TRUE;
}

void YAEditView::PaintRect(HDC hDC, const RECT &rPaintRect)
{
	RECT r = rClientRect;

	DWORD nStartRow = r.top / nLineH + nBaseLineNo;
	DWORD nEndRow = r.bottom / nLineH + nBaseLineNo;

	DWORD nMaxLine = GetMaxLine();
	if (nMaxLine < nEndRow) nEndRow = nMaxLine;

	r.right = rClientRect.right;

	SetViewportOrgEx(hDC, -(int)nColOffset, 0, NULL);

	DWORD nSelStart = 0;
	DWORD nSelEnd = 0;

	DWORD i;
	for (i = nStartRow; i < nEndRow; i++) {
		// get line data
		LineChunk lc;
		if (!pCtrl->GetLgLineChunk(i, &lc)) break;

		lc.GetSelRange(&nSelStart, &nSelEnd);

		if (!PaintLine(hDC, &lc, &r, nSelStart, nSelEnd)) break;
		r.top += nLineH;
	}

}

BOOL YAEditView::DirectPaintLine(DWORD nLineNo)
{
	if (!IsLineDisplay(nLineNo)) return TRUE; // the line is not in window

	LineChunk lc;
	if (!pCtrl->GetLgLineChunk(nLineNo, &lc)) return FALSE;

	DWORD nDpLineNo = LgLineNoToDpLineNo(nLineNo);

	RECT r;
	r.left = 0;
	r.top = DpLineNoToDpLinePixel(nDpLineNo);
	r.right = rClientRect.right;
	if (nDpLineNo == nPageHeight) {
		r.bottom = rClientRect.bottom;
	} else {
		r.bottom = DpLineNoToDpLinePixel(nDpLineNo + 1);
	}

	HDC hDC = GetDC(hViewWnd);

	SelectObject(hDC, GetSysColorBrush(COLOR_WINDOW));
	SelectObject(hDC, GetStockObject(NULL_PEN));
	Rectangle(hDC, r.left, r.top, r.right, r.bottom + 1);
	PaintLine(hDC, &lc, &r, 0, 0);

	ReleaseDC(hViewWnd, hDC);
	return TRUE;
}

void YAEditView::RequestRedraw(DWORD nLineNo, WORD nLeftPos, BOOL bToBottom)
{
	if (!IsLineDisplay(nLineNo)) return;

	RECT r;
	r.left = nLeftPos;
	r.right = rClientRect.right;
	
	r.top = (nLineNo - nBaseLineNo) * nLineH;
	if (bToBottom) {
		r.bottom = rClientRect.bottom;
	} else {
		r.bottom = r.top + nLineH;
	}

	InvalidateRect(hViewWnd, &r, TRUE);
}

// maybe this method is not used now
void YAEditView::RequestRedrawWithLine(DWORD nLineNo, DWORD nNumLine)
{
	LineChunk lc;
	pCtrl->GetLgLineChunk(nLineNo + nNumLine, &lc);

	// redraw updated region
	Region r;
	r.posStart.Set(0, nLineNo);
	r.posEnd.Set(lc.LineLen(), nLineNo + nNumLine);
	RequestRedrawRegion(&r);
}

void YAEditView::RequestRedrawRegion(const Region *pRegion)
{
	Region rgn = *pRegion;

	// replace Region::COL_EOL
	if (rgn.posEnd.col == Region::COL_EOL) {
		LineChunk lc2;
		pCtrl->GetLgLineChunk(rgn.posEnd.row, &lc2);
		rgn.posEnd.col = lc2.LineLen();
	}

	if (rgn.posEnd.row < nBaseLineNo ||
		rgn.posStart.row > nBaseLineNo + nPageHeight) {
		// selected area is out of screen
		return;
	}

	if (rgn.posStart.row == rgn.posEnd.row) {
		// region is in one line
		CalcInvalidateArea(rgn.posStart.row, rgn.posStart.col, rgn.posEnd.col);
	} else {
		// top of line
		if (rgn.posStart.row >= nBaseLineNo && rgn.posStart.row <= nBaseLineNo + nPageHeight) {
			LineChunk lc;
			pCtrl->GetLgLineChunk(rgn.posStart.row, &lc);
			CalcInvalidateArea(rgn.posStart.row, 0, lc.LineLen());
		}

		// end of line
		if (rgn.posEnd.row >= nBaseLineNo && rgn.posEnd.row <= nBaseLineNo + nPageHeight) {
			CalcInvalidateArea(rgn.posEnd.row, 0, rgn.posEnd.col);
		}

		// rest area

		// select lines that in the view
		DWORD nStartRow = rgn.posStart.row + 1;
		DWORD nEndRow = rgn.posEnd.row - 1;
		if (nStartRow > nEndRow) return;
		if (nStartRow < nBaseLineNo) nStartRow = nBaseLineNo;
		if (nEndRow > nBaseLineNo + nPageHeight) nEndRow = nBaseLineNo + nPageHeight;

		// request update
		RECT r;
		r.left = rClientRect.left; r.right = rClientRect.right;
		r.top = (nStartRow - nBaseLineNo) * nLineH;
		r.bottom = r.top + (nEndRow - nStartRow + 1) * nLineH;
		InvalidateRect(hViewWnd, &r, TRUE);
	}

	//
	if (rgn.posEnd.row == GetMaxLine() - 1) {
		RECT r;
		r.left = rClientRect.left; r.right = rClientRect.right;
		r.top = (rgn.posEnd.row - nBaseLineNo) * nLineH;
		r.bottom = rClientRect.bottom;
		InvalidateRect(hViewWnd, &r, TRUE);

	}
}

void YAEditView::CalcInvalidateArea(DWORD nLine, DWORD nStart, DWORD nEnd)
{
	RECT r;

	LineChunk lc;
	if (!pCtrl->GetLgLineChunk(nLine, &lc)) return;

	DWORD nStartPos = GetLineWidth(0, lc.GetLineData(), nStart);
	DWORD nEndPos = GetLineWidth(nStartPos, lc.GetLineData() + nStart, 
								nEnd - nStart);

	r.left = nStartPos - nColOffset;
	r.top = (nLine - nBaseLineNo) * nLineH;
	r.right = r.left + nEndPos;
	r.bottom = r.top + nLineH;

	if (nEnd == lc.LineLen()) {
		r.right = rClientRect.right;
	}

	InvalidateRect(hViewWnd, &r, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// get max line no
/////////////////////////////////////////////////////////////////////////////
// This method is the proxy of YAEdit.

DWORD YAEditView::GetMaxLine()
{
	if (pCtrl && pCtrl->GetDoc()) {
		return pCtrl->GetLineMgr()->MaxLine();
	} else {
		return 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
// set caret position
/////////////////////////////////////////////////////////////////////////////

void YAEditView::SetCaretPos()
{
	if (IsCursorInDisplay()) {
		if (!bShowCaret) {
			ShowCaret(hViewWnd);
			bShowCaret = TRUE;
		}

#if !defined(PLATFORM_PSPC)
		HIMC hIMC = ImmGetContext(hViewWnd);
		COMPOSITIONFORM cf;
		cf.dwStyle = CFS_POINT;
		cf.ptCurrentPos.x = nCursorColPos - nColOffset;
		cf.ptCurrentPos.y = (nCursorRow - nBaseLineNo) * nLineH;
		ImmSetCompositionWindow(hIMC, &cf);
		ImmSetCompositionFont(hIMC, &lfLogFont);
		ImmReleaseContext(hViewWnd, hIMC);
#endif

		::SetCaretPos(nCursorColPos - nColOffset, (nCursorRow - nBaseLineNo)*nLineH);
	} else {
		if (bShowCaret) {
			HideCaret(hViewWnd);
			bShowCaret = FALSE;
		}
	}
}

void YAEditView::SetFont(HFONT h)
{
	if (h == NULL) return;
	hFont = h;

	GetObject(h, sizeof(LOGFONT), &lfLogFont);

	ResetFontInfo();
}

/////////////////////////////////////////////////////////////////////////////
// ScrollCaret
/////////////////////////////////////////////////////////////////////////////
// Scroll view that cursor is in.
//
// ScrollCaret try to adjust to view cursor cetner of the view if it can.
void YAEditView::ScrollCaret()
{
	if (IsCursorInDisplay()) return;

	// Adjust vert position
	DWORD nNewBase;
	if (nCursorRow > nPageHeight / 2) {
		nNewBase = nCursorRow - nPageHeight / 2;
	} else {
		nNewBase = 0;
	}
	SetScrollVertPos(nNewBase, TRUE);

	// Adjust horiz position. 
	int nPos = nCursorColPos - (rClientRect.right - rClientRect.left)/2;
	SetScrollHorizPos(nPos);
}

/////////////////////////////////////////////////////////////////////////////
// Find text position
/////////////////////////////////////////////////////////////////////////////

void YAEditView::SetNearCursorPos(WORD xPos, DWORD nYLines)
{
	xPos += (WORD)nColOffset;
	nCursorRow = nYLines;

	LineChunk lc;
	if (!pCtrl->GetLgLineChunk(nYLines, &lc)) {
		SetCaretPos();
		return;
	}
	LPCTSTR pLine = lc.GetLineData();
	DWORD nLineLen = lc.LineLen();
	
	LPCTSTR pLimit = pLine + nLineLen;
	LPCTSTR p = pLine;
	LPCTSTR q;
	DWORD w = 0;
	DWORD nCurWidth = 0;

	while (p < pLimit) {
		q = CharNext(p);

		w = GetLineWidth(nCurWidth, p, q - p);
		if (nCurWidth + w> xPos) break;
		nCurWidth += w;
		p = q;
	}
	nCursorCol = p - pLine;
	nCursorColPos = nCurWidth;

	SetCaretPos();
}

void YAEditView::SetCaretPosition(const Coordinate &pos)
{
	nCursorRow = pos.row;
	nCursorCol = pos.col;

	LineChunk lc;
	if (!pCtrl->GetLgLineChunk(nCursorRow, &lc)) return;

	if (nCursorCol >= lc.LineLen()) {
		nCursorCol = lc.LineLen();
	}

	nCursorColPos = GetLineWidth(0, lc.GetLineData(), nCursorCol);
	SetCaretPos();
}

/////////////////////////////////////////////////////////////////////////////
// scroll view
/////////////////////////////////////////////////////////////////////////////

void YAEditView::SetScrollVertPos(DWORD nPos, BOOL bFullRewrite)
{
	if (nPos + nPageHeight > GetMaxLine()) {
		nPos = GetMaxLine() - nPageHeight;
	}

	nBaseLineNo = nPos;
	if (bFullRewrite) {
		InvalidateRect(hViewWnd, &rClientRect, TRUE);
	}
	SetScrollPos(hViewWnd, SB_VERT, nBaseLineNo, TRUE);
	SetCaretPos();
}

void YAEditView::SetScrollVertByOffset(int nLineOffset)
{
	if (nPageHeight > GetMaxLine()) return;

	DWORD nPos = nBaseLineNo + nLineOffset;

	if (nLineOffset < 0 && nBaseLineNo < (DWORD)-nLineOffset) {
		nPos = 0;
	} else if (nPos + nPageHeight > GetMaxLine()) {
		nPos = GetMaxLine() - nPageHeight;
	}
	if (nPos == nBaseLineNo) return;

	nBaseLineNo = nPos;

	// set invalidate range
	RECT r = rClientRect;
	if (nLineOffset == 1) {
		r.bottom = r.top + (nLineH + 1) * nPageHeight;
		HideCaret(hViewWnd);
		ScrollWindowEx(hViewWnd, 0, -(int)nLineH, &r, &r, NULL, NULL, 0);
		DirectPaintLine(DpLineNoToLgLineNo(nPageHeight) - 1);
		ShowCaret(hViewWnd);
		SetScrollVertPos(nPos, FALSE);
		return;
	} else if (nLineOffset == -1) {
		r.bottom = r.top + nLineH * nPageHeight;
		HideCaret(hViewWnd);
		ScrollWindowEx(hViewWnd, 0, nLineH, &r, &r, NULL, NULL, 0);
		DirectPaintLine(nBaseLineNo);
		ShowCaret(hViewWnd);
		SetScrollVertPos(nPos, FALSE);
		return;
	}

	SetScrollVertPos(nPos, TRUE);
}

void YAEditView::SetScrollHorizPos(int nPos)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	GetScrollInfo(hViewWnd, SB_HORZ, &si);

	if (nPos < si.nMin) {
		nPos = 0;
	} else if (si.nMax - (int)si.nPage <= nPos) {
		nPos = si.nMax - si.nPage;
	}
	if (nPos == si.nPos) return;

	nColOffset = nPos;
	SetScrollPos(hViewWnd, SB_HORZ, nPos, TRUE);
	InvalidateRect(hViewWnd, &rClientRect, TRUE);
	SetCaretPos();
}

void YAEditView::NextPage() { SetScrollVertByOffset((int)(nPageHeight - 2)); }
void YAEditView::PrevPage() { SetScrollVertByOffset(-(int)(nPageHeight - 2)); }
void YAEditView::Next1L() { SetScrollVertByOffset(1); }
void YAEditView::Prev1L() { SetScrollVertByOffset(-1); }
void YAEditView::ScrollRight1L() { SetScrollHorizPos(nColOffset + nAveCharWidth); }
void YAEditView::ScrollLeft1L() { SetScrollHorizPos(nColOffset - nAveCharWidth); }
void YAEditView::ScrollRight1P() { SetScrollHorizPos(nColOffset + nHorizPageScrollDelta); }
void YAEditView::ScrollLeft1P() { SetScrollHorizPos(nColOffset - nHorizPageScrollDelta); }

/////////////////////////////////////////////////////////////////////////////
// cursor position check funcs
/////////////////////////////////////////////////////////////////////////////

BOOL YAEditView::IsCursorInDocBottom() { return nCursorRow + 1 == GetMaxLine(); }
BOOL YAEditView::IsCursorInPageBottom() { return nCursorRow - nBaseLineNo == nPageHeight - 1; }
BOOL YAEditView::IsCursorInPageTop() { return nCursorRow - nBaseLineNo == 0; }
BOOL YAEditView::IsCursorInDocTop() { return nCursorRow == 0 && nCursorCol == 0; }

BOOL YAEditView::IsCursorInDisplay()
{
	if (nCursorRow < nBaseLineNo ||
		nCursorRow >= nBaseLineNo + nPageHeight ||
		nCursorColPos < nColOffset || 
		nColOffset + (rClientRect.right - rClientRect.left) < nCursorColPos
	) return FALSE;
	return TRUE;
}

BOOL YAEditView::IsLineDisplay(DWORD nLineNo)
{
	return (nBaseLineNo <= nLineNo) && (nLineNo < nBaseLineNo + nPageHeight);
}
/////////////////////////////////////////////////////////////////////////////
//  Coordinate conversion
/////////////////////////////////////////////////////////////////////////////

DWORD YAEditView::LgLineNoToDpLineNo(DWORD nLgLineNo) { return nLgLineNo - nBaseLineNo; }
DWORD YAEditView::DpLineNoToLgLineNo(DWORD nDpLineNo) { return nDpLineNo + nBaseLineNo; }
DWORD YAEditView::DpLineNoToDpLinePixel(DWORD nDpLineNo) { return rClientRect.top + nDpLineNo * nLineH; }
DWORD YAEditView::DpLinePixelToLgLineNo(DWORD nDpLinePixel) { return nDpLinePixel / nLineH + nBaseLineNo; }

/////////////////////////////////////////////////////////////////////////////
// WM_CREATE
/////////////////////////////////////////////////////////////////////////////
void YAEditView::OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	GetClientRect(hWnd, &rClientRect);
	hViewWnd = hWnd;

	ResetFontInfo();

}

void YAEditView::ResetFontInfo()
{
	HDC hDC = GetDC(hViewWnd);
	if (hDC) {
		HFONT hOldFont = NULL;
		if (hFont) {
			hOldFont = (HFONT)SelectObject(hDC, hFont);
		}
		TEXTMETRIC tm;
		GetTextMetrics(hDC, &tm);
		nLineH = tm.tmHeight;
		nAveCharWidth = tm.tmAveCharWidth;
		nMaxCharWidth = tm.tmMaxCharWidth;

		nTabWidth = nAveCharWidth * 4;

		if (hFont) {
			SelectObject(hDC, hOldFont);
		}
		ReleaseDC(hViewWnd, hDC);
	} else {
		return;
	}

	nPageHeight = (rClientRect.bottom - rClientRect.top) / nLineH;

	if (pFontCache) delete pFontCache;
	pFontCache = new FontWidthCache();
	if (pFontCache == NULL || !pFontCache->Init(hViewWnd, hFont)) return;

}
/////////////////////////////////////////////////////////////////////////////
// Forcus window
/////////////////////////////////////////////////////////////////////////////

void YAEditView::OnSetFocus()
{
	DWORD nCaretWidth = GetSystemMetrics(SM_CXBORDER);
	if (nCaretWidth < 2) nCaretWidth = 2;

	CreateCaret(hViewWnd, NULL, nCaretWidth, nLineH);
	bShowCaret = TRUE;
	ShowCaret(NULL);
	SetCaretPos();
}

void YAEditView::OnKillFocus()
{
	HideCaret(hViewWnd);
	bShowCaret = FALSE;
	DestroyCaret();
}

/////////////////////////////////////////////////////////////////////////////
// cursor move
/////////////////////////////////////////////////////////////////////////////

void YAEditView::MoveRight()
{
	if (!pCtrl->GetDoc()) return;

	LineChunk lc;
	if (!pCtrl->GetLgLineChunk(nCursorRow, &lc)) return;

	if (nCursorCol >= lc.LineLen()) {
		// at the right of the line.
		if (IsCursorInDocBottom()) return;
		MoveDown();
		MoveTOL();
		return; 
	}

	LPCTSTR pLine = lc.GetLineData();
	LPCTSTR pCurrentChar = pLine + nCursorCol;
	DWORD nLen = 1;

#if defined(PLATFORM_WIN32)
	if (IsDBCSLeadByte(*pCurrentChar)) {
		nLen = 2;
	}
#endif

	nCursorColPos += GetLineWidth(nCursorColPos, pCurrentChar, nLen);
	nCursorCol += nLen;

	if (!IsCursorInDisplay()) {
		ScrollRight1P();
	}
	SetCaretPos();
}

void YAEditView::MoveLeft()
{
	if (!pCtrl->GetDoc()) return;

	if (nCursorCol == 0) {
		// at the top of the line.
		if (nCursorRow == 0) return;
		MoveUp();
		MoveEOL();
		return; 
	}

	LineChunk lc;
	if (!pCtrl->GetLgLineChunk(nCursorRow, &lc)) return;
	LPCTSTR pLine = lc.GetLineData();
	DWORD nLen = pCtrl->GetPrevOffset(nCursorRow, nCursorCol);
	LPCTSTR pNewChar = pLine + nCursorCol - nLen;
	nCursorColPos = GetLineWidth(0, pLine, nCursorCol - nLen);
	nCursorCol -= nLen;

	if (!IsCursorInDisplay()) {
		ScrollLeft1P();
	}
	SetCaretPos();
}

void YAEditView::MoveUp()
{
	if (IsCursorInPageTop()) {
		if (nBaseLineNo == 0) return;
		// scroll up 1 line
		Prev1L();
	}
	SetNearCursorPos((WORD)nCursorColPos, nCursorRow - 1);
}

void YAEditView::MoveEOL()
{
	LineChunk lc;
	if (!pCtrl->GetLgLineChunk(nCursorRow, &lc)) return;

	LPCTSTR p = lc.GetLineData();
	nCursorCol = lc.LineLen();
	nCursorColPos = GetLineWidth(0, p, nCursorCol);

	if (!IsCursorInDisplay()) {
		if (nCursorColPos < nColOffset) {
			SetScrollHorizPos(((nColOffset - nCursorColPos) / nHorizPageScrollDelta + 1) * nHorizPageScrollDelta);
		} else {
			DWORD n = nCursorColPos / nHorizPageScrollDelta - 1; // most right position that you can view EOL.
			n -= (rClientRect.right - rClientRect.left) / nHorizPageScrollDelta -1; // Adjust that the line is displayed as much as possible.
			SetScrollHorizPos(n * nHorizPageScrollDelta);
		}
	}

	SetCaretPos();
}

void YAEditView::MoveTOL()
{
	nCursorCol = 0;
	nCursorColPos = 0;
	if (!IsCursorInDisplay()) {
		SetScrollHorizPos(0);
	}
	SetCaretPos();
}

void YAEditView::MoveDown()
{
	if (IsCursorInDocBottom()) return;
	if (IsCursorInPageBottom()) Next1L();

	SetNearCursorPos((WORD)nCursorColPos, nCursorRow + 1);
}

DWORD YAEditView::GetLineWidth(DWORD nOffset, LPCTSTR pStr, DWORD nLen)
{
	LPCTSTR pLimit = pStr + nLen;
	DWORD w = 0;
	LPCTSTR p = pStr;
	LPCTSTR q;
	while (p < pLimit) {
		q = CharNext(p);

		if (*p == TEXT('\t')) {
			DWORD wt = ((nOffset + w) / nTabWidth + 1) * nTabWidth;
			w = wt - nOffset;
		} else {
			DWORD wc;
#if defined(PLATFORM_WIN32)
			if (q - p == 1) {
				wc = pFontCache->GetOnebyteCharWidth(*p);
			} else {
				wc = pFontCache->GetTwobyteCharWidth(p);
			}
#else
			wc = pFontCache->GetWideCharWidth(*p);
#endif
			w += wc;
		}
		p = q;
	}
	return w;
}

void YAEditView::UpdateMaxLineWidth()
{
	DWORD i, w;
	DWORD n = pCtrl->GetLineMgr()->MaxLine();

	nMaxWidthPixel = 0;
	LineChunk lc;
	for (i = 0; i < n; i++) {
		if (!pCtrl->GetLgLineChunk(i, &lc)) return;

		LPCTSTR p = lc.GetLineData();
		w = GetLineWidth(0, p, lc.LineLen());
		if (w > nMaxWidthPixel) nMaxWidthPixel = w;
	}
}

void YAEditView::RedrawAllScreen()
{
	InvalidateRect(hViewWnd, NULL, TRUE);
}

BOOL YAEditView::IsVertScrollbarDisplayed()
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;

	GetScrollInfo(hViewWnd, SB_VERT, &si);
	return (si.nMin != si.nMax) && ((UINT)si.nMin <= si.nPage) && (si.nPage <= (UINT)si.nMax);
}

void YAEditView::SetColorDef(const YAEditViewColorDef& cdef) {
	colorDef = cdef;
}