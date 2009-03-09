#include <windows.h>
#include <tchar.h>
#if !defined(PLATFORM_PSPC)
#include <imm.h>
#endif
#if defined(PLATFORM_WIN32)
#include <zmouse.h>
#endif
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
#include <aygshell.h>
#endif
#include "resource.h"
#include "Region.h"
#include "YAEdit.h"
#include "YAEditView.h"
#include "YAEditDoc.h"
#include "VarBuffer.h"
#include "LineManager.h"
#include "LineWrapper.h"
#include "LineWrapperImpl.h"
#include "Logger.h"
#include "FontWidthCache.h"
#include "TomboLib/UniConv.h"
#include "TomboLib/Clipboard.h"
#include "TString.h"

/////////////////////////////////////////////////////////////////////////////
// constatnt definitions
/////////////////////////////////////////////////////////////////////////////

#define YAEDIT_CLASS_NAME TEXT("YAEditCtl")

#define CHARA_CTRL_C 3
#define CHARA_CTRL_S 19
#define CHARA_CTRL_V 22
#define CHARA_CTRL_X 24
#define CHARA_BS 8
#define CHARA_ENTER 13
#define CHARA_ESC 27
#define CHARA_TAB 9
#define CHARA_SPACE 32

/////////////////////////////////////////////////////////////////////////////
// static funcs/vars declarations
/////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK YAEditWndProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////
// Regist window class
/////////////////////////////////////////////////////////////////////////////

BOOL YAEdit::RegisterClass(HINSTANCE hInst)
{
	WNDCLASS wc;

	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = (WNDPROC)YAEditWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(LONG);
	wc.hInstance = hInst;
	wc.hIcon = NULL;
#ifdef _WIN32_WCE
	wc.hCursor = NULL;
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
#else
	wc.hCursor = LoadCursor(NULL, IDC_IBEAM);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
#endif
	wc.lpszMenuName = NULL;
	wc.lpszClassName = YAEDIT_CLASS_NAME;
	::RegisterClass(&wc);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// factory method
/////////////////////////////////////////////////////////////////////////////

YAEdit *YAEdit::GetInstance(YAEditCallback *pCallback)
{
	return new YAEditImpl(pCallback);
}

YAEditDoc *YAEditImpl::CreateDocument(const char *pStr, YAEditCallback* pCb)
{
	YAEditDoc *pDoc = new YAEditDoc();
	if (pDoc == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	if (pDoc->Init(pStr, this, pCb)) return pDoc;
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////////////////////////////////////////

YAEditImpl::YAEditImpl(YAEditCallback *pCB) : 
	pWrapper(NULL), bScrollTimerOn(FALSE), pView(NULL), 
	bMouseDown(FALSE), pLineMgr(NULL), pCallback(pCB), 
	bInsertMode(FALSE)
{
}

YAEditImpl::~YAEditImpl()
{
	if (pView) {
		if (pView->hViewWnd) DestroyWindow(pView->hViewWnd);
	}
	if (pLineMgr) delete pLineMgr;
}

/////////////////////////////////////////////////////////////////////////////
// Event handler
/////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK YAEditWndProc(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	if (nMessage == WM_CREATE) {
		LPCREATESTRUCT pCS = (LPCREATESTRUCT)lParam;
		YAEditImpl *frm = (YAEditImpl*)pCS->lpCreateParams;
		SetWindowLong(hWnd, 0, (LONG)frm);
		frm->OnCreate(hWnd, wParam, lParam);
		return 0;
	}

	YAEditImpl *frm = (YAEditImpl*)GetWindowLong(hWnd, 0);
	if (frm == NULL) {
		return DefWindowProc(hWnd, nMessage, wParam, lParam);
	}

	switch(nMessage) {
	case WM_KEYDOWN:
		if (frm->OnKeyDown(hWnd, wParam, lParam)) {
			return 0;
		}
		break;
	case WM_CHAR:
		frm->OnChar(hWnd, wParam, lParam);
		return 0;
	case WM_DESTROY:
		return 0;
	case WM_PAINT:
		frm->OnPaint(hWnd, wParam, lParam);
		break;
	case WM_VSCROLL:
		frm->OnVScroll(hWnd, wParam, lParam);
		return 0;
	case WM_HSCROLL:
		frm->OnHScroll(hWnd, wParam, lParam);
		return 0;
	case WM_LBUTTONDOWN:
		frm->OnLButtonDown(hWnd, wParam, lParam);
		break;
	case WM_LBUTTONDBLCLK:
		frm->OnLButtonDblClick(hWnd, wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		frm->OnMouseMove(hWnd, wParam, lParam);
		break;
	case WM_LBUTTONUP:
		frm->OnLButtonUp(hWnd, wParam, lParam);
		break;
	case WM_SETFOCUS:
		frm->OnSetFocus();
		return 0;
	case WM_KILLFOCUS:
		frm->OnKillFocus();
		return 0;
	case WM_TIMER:
		frm->OnTimer(hWnd, wParam, lParam);
		return 0;
	case WM_SIZE:
		frm->OnResize(hWnd, wParam, lParam);
		break;
	case WM_RBUTTONDOWN:
		frm->OnRbuttonDown(hWnd, wParam, lParam);
		return 0;
#if defined(PLATFORM_WIN32)
	case WM_MOUSEWHEEL:
		frm->OnMouseWheel(hWnd, wParam, lParam);
		return 0;
#endif
//	case WM_IME_COMPOSITION:
//		frm->OnIMEComposition(hWnd, wParam, lParam);
//		return 0;
	}
	return DefWindowProc(hWnd, nMessage, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// Create window
/////////////////////////////////////////////////////////////////////////////

BOOL YAEditImpl::Create(HINSTANCE hInst, HWND hParent, DWORD nId, RECT &r, BOOL bWrap)
{
	hInstance = hInst;
	pDoc = NULL;

	pLineMgr = new LineManager();
	if (!pLineMgr->Init(this)) return FALSE;

	pView = new YAEditView(this);

	bWrapLine = bWrap;
	if (!SetWrapper()) return FALSE;

	pDoc = new YAEditDoc(); 
	if (!pDoc->Init("", this, pCallback)) return FALSE;
	
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	pView->hViewWnd = CreateWindowEx(WS_EX_CLIENTEDGE, YAEDIT_CLASS_NAME, TEXT(""),
						WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,
						r.left,
						r.top,
						r.right,
						r.bottom,
						hParent,
						(HMENU)nId,
						hInst,
						this);
	DWORD nx = GetLastError();
#else
	pView->hViewWnd = CreateWindow(YAEDIT_CLASS_NAME, TEXT(""),
						WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,
						r.left,
						r.top,
						r.right,
						r.bottom,
						hParent,
						(HMENU)nId,
						hInst,
						this);
#endif

	if (pView->hViewWnd == NULL) return FALSE;

#if defined(PLATFORM_WIN32)
	// clear buffer
	aKeyBuffer[0] = aKeyBuffer[1] = '\0';
#endif

	rSelRegion.posStart.Set(0, 0);
	rSelRegion.posEnd.Set(0, 0);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// set wrapper
/////////////////////////////////////////////////////////////////////////////

BOOL YAEditImpl::SetWrapper()
{
	if (bWrapLine) {
		// allocate wrapper
		FixedPixelLineWrapper *pWw = new FixedPixelLineWrapper();
		if (pWw == NULL || !pWw->Init(this)) return FALSE;
		delete pWrapper;
		pWrapper = pWw;
	} else {
		SimpleLineWrapper *pWw = new SimpleLineWrapper();
		if (pWw == NULL) return FALSE;
		delete pWrapper;
		pWrapper = pWw;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// WM_CREATE handler
/////////////////////////////////////////////////////////////////////////////
// In this function, hViewWnd are not initialized yet.

void YAEditImpl::OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	pView->OnCreate(hWnd, wParam, lParam);

	pView->ResetPosition();
	pWrapper->SetViewWidth(pView->GetViewClientRect().right - pView->GetViewClientRect().left - pView->nMaxCharWidth);

	// associate with default(empty) document.
	// Since memory allocation check, object is created in Create(), and assoicated here.
	SetDoc(pDoc);
}

/////////////////////////////////////////////////////////////////////////////
// WM_VSCROLL handler
/////////////////////////////////////////////////////////////////////////////

void YAEditImpl::OnVScroll(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int nScrollCode = LOWORD(wParam);
	switch(nScrollCode) {
	case SB_LINEDOWN:
		pView->Next1L();
		break;
	case SB_LINEUP:
		pView->Prev1L();
		break;
	case SB_PAGEDOWN:
		pView->NextPage();
		break;
	case SB_PAGEUP:
		pView->PrevPage();
		break;
	case SB_THUMBTRACK:
		pView->SetScrollVertPos(HIWORD(wParam), TRUE);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// WM_HSCROLL handler
/////////////////////////////////////////////////////////////////////////////

void YAEditImpl::OnHScroll(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int nScrollCode = LOWORD(wParam);
	switch(nScrollCode) {
	case SB_LINELEFT:
		pView->ScrollLeft1L();
		break;
	case SB_LINERIGHT:
		pView->ScrollRight1L();
		break;
	case SB_PAGELEFT:
		pView->ScrollLeft1P();
		break;
	case SB_PAGERIGHT:
		pView->ScrollRight1P();
		break;
	case SB_THUMBTRACK:
		{
			int nPos = HIWORD(wParam); 
			pView->SetScrollHorizPos(nPos);
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// WM_MOUSEWHEEL
/////////////////////////////////////////////////////////////////////////////
#if defined(PLATFORM_WIN32)
void YAEditImpl::OnMouseWheel(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	short delta = ((short) HIWORD(wParam))/WHEEL_DELTA;
	pView->SetScrollVertByOffset(-delta * 2);
}
#endif

/////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
// Line drawing related members
//////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void YAEditImpl::OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);
	if (pDoc == NULL) { 
		// Usually, pDoc has some value. this check is for safety.
		EndPaint(hWnd, &ps);
		return;
	}
	pView->PaintRect(hDC, ps.rcPaint);
	EndPaint(hWnd, &ps);
}

void YAEditImpl::RequestRedraw(DWORD nLineNo, WORD nLeftPos, BOOL bToBottom) { pView->RequestRedraw(nLineNo, nLeftPos, bToBottom); }
void YAEditImpl::RequestRedrawRegion(const Region *pRegion) 
{ 
	pView->RequestRedrawRegion(pRegion); 
}

/////////////////////////////////////////////////////////////////////////////
// FOCUS
/////////////////////////////////////////////////////////////////////////////

void YAEditImpl::OnSetFocus()  { 
	if (pView) pView->OnSetFocus();
	if (pCallback) pCallback->OnGetFocus();
}

void YAEditImpl::OnKillFocus() { if (pView) pView->OnKillFocus(); }

/////////////////////////////////////////////////////////////////////////////
// Key handler
/////////////////////////////////////////////////////////////////////////////
BOOL YAEditImpl::OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int nVertKey = (int)wParam;
	BOOL bShiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	BOOL bCtrlDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	BOOL bAltDown = (GetKeyState(VK_MENU) & 0x8000) != 0;

	if (bShiftDown) {
		switch (nVertKey) {
		case VK_RIGHT:
			CmdSelRight();
			break;
		case VK_LEFT:
			CmdSelLeft();
			break;
		case VK_UP:
			CmdSelUp();
			break;
		case VK_DOWN:
			CmdSelDown();
			break;
		case VK_DELETE:
			CmdCut();
			break;
		case VK_INSERT:
			CmdPaste();
			break;
		case VK_HOME:
			if (bCtrlDown) {
				CmdSelTopOfDoc();
			} else {
				CmdSelTopOfLogicalLine();
			}
			break;
		case VK_END:
			if (bCtrlDown) {
				CmdSelEndOfDoc();
			} else {
				CmdSelEndOfLogicalLine();
			}
			break;
		}
	} else if (bCtrlDown) {
		switch (nVertKey) {
		case 'C':
			CmdCopy();
			break;
		case 'V':
			CmdPaste();
			break;
		case 'X':
			CmdCut();
			break;
		case 'A':
			CmdSelAll();
			break;
		case 'Z':
			CmdUndo();
			break;
		case 'B':
			CmdToggleReadOnly();
			break;
		case VK_INSERT:
			CmdCopy();
			break;
		default:
			CmdNOP();
		}
		return TRUE;
	} else {
		switch (nVertKey) {
		case VK_DELETE:
			CmdDeleteChar();
			break;
		case VK_RIGHT:
			CmdMoveRight();
			return TRUE;
		case VK_LEFT:
			CmdMoveLeft();
			return TRUE;
		case VK_UP:
			CmdMoveUp();
			return TRUE;
		case VK_DOWN:
			CmdMoveDown();
			return TRUE;
		case VK_PRIOR:
			CmdScrollUp();
			return TRUE;
		case VK_NEXT:
			CmdScrollDown();
			return TRUE;
		case VK_HOME:
			CmdMoveTOL();
			break;
		case VK_END:
			CmdMoveEOL();
			break;
		case VK_INSERT:
			CmdToggleInsertMode();
			break;
		default:
			return FALSE;
		}
	}

	pView->ScrollCaret();
	return TRUE;
}

void YAEditImpl::OnChar(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	TCHAR ch = (TCHAR)wParam;

	if (pDoc->IsReadOnly()) {
		if (ch == CHARA_BS) { CmdScrollUp(); return; }
		if (ch == CHARA_SPACE) { CmdScrollDown(); return; }
		return;
	}

	if (ch == CHARA_BS) { CmdBackSpace(); return; }
	if (ch == CHARA_ENTER) { CmdReplaceString(TEXT("\n")); return; }
	if (ch == CHARA_ESC) { /* nop */; return; }

	// if control char, nothing to do
	if (
#if defined(PLATFORM_WIN32)
		aKeyBuffer[0] == '\0' && 
#endif
		0 <= ch && ch <= 29 && ch != CHARA_TAB) return;

#if defined(PLATFORM_WIN32)
	if (aKeyBuffer[0] != '\0') {
		aKeyBuffer[1] = ch;
		aKeyBuffer[2] = TEXT('\0');
		if (bInsertMode && SelectedRegion().IsEmptyRegion()) {
			CmdSelRight();
		}
		ReplaceText(SelectedRegion(), aKeyBuffer);
		aKeyBuffer[0] = aKeyBuffer[1] = '\0';
		return;
	} else {
		// if char is DBCS lead byte, buffering.
		if (IsDBCSLeadByte(ch)) {
			aKeyBuffer[0] = ch;
			return;
		}
	}
#endif
	TCHAR kbuf[2];
	kbuf[0] = ch; kbuf[1] = TEXT('\0');
	if (bInsertMode && SelectedRegion().IsEmptyRegion()) {
		CmdSelRight();
	}

	ReplaceText(SelectedRegion(), kbuf);
}

/////////////////////////////////////////////////////////////////////////////
// update selected region
/////////////////////////////////////////////////////////////////////////////

void YAEditImpl::UpdateSelRegion()
{
	Coordinate nSelNow = pView->GetCaretPosition();
	Coordinate nSelOld;

	Region r;
	ExtendSelectRegion(nSelNow, &nSelOld);
	if (nSelNow < nSelOld) {
		r.posStart = nSelNow; r.posEnd = nSelOld;
	} else {
		r.posStart = nSelOld; r.posEnd = nSelNow;
	}
	RequestRedrawRegion(&r);
}

/////////////////////////////////////////////////////////////////////////////
// commands
/////////////////////////////////////////////////////////////////////////////

void YAEditImpl::CmdNOP() { /* NOP */ }
void YAEditImpl::CmdMoveRight() {
	pView->ScrollCaret();
	pView->MoveRight();
	ClearRegion();
	pView->ScrollCaret();
	pDoc->CloseUndoRegion();
}

void YAEditImpl::CmdMoveLeft()  {
	pView->ScrollCaret();
	pView->MoveLeft();
	ClearRegion();
	pView->ScrollCaret();
	pDoc->CloseUndoRegion();
}

void YAEditImpl::CmdMoveUp()
{
	pView->ScrollCaret(); 
	pView->MoveUp();
	ClearRegion();
	pView->ScrollCaret(); 
	pDoc->CloseUndoRegion();
}

void YAEditImpl::CmdMoveEOL()   {
	pView->ScrollCaret();
	pView->MoveEOL();
	ClearRegion();
	pView->ScrollCaret();
	pDoc->CloseUndoRegion();
}
void YAEditImpl::CmdMoveTOL()   { 
	pView->ScrollCaret();
	pView->MoveTOL();
	ClearRegion();
	pDoc->CloseUndoRegion();
}

void YAEditImpl::CmdMoveDown()  { 
	pView->ScrollCaret();
	pView->MoveDown();
	ClearRegion(); 
	pDoc->CloseUndoRegion();
}

void YAEditImpl::CmdSelRight()	{ pView->MoveRight(); UpdateSelRegion(); }
void YAEditImpl::CmdSelLeft()	{ pView->MoveLeft(); UpdateSelRegion(); }
void YAEditImpl::CmdSelUp()		{ pView->MoveUp(); UpdateSelRegion(); }
void YAEditImpl::CmdSelDown()	{ pView->MoveDown(); UpdateSelRegion(); }

void YAEditImpl::CmdScrollUp()	{ pView->PrevPage(); }
void YAEditImpl::CmdScrollDown(){ pView->NextPage(); }

void YAEditImpl::CmdSelTopOfLogicalLine() 
{
	Region rNewRegion;

	rNewRegion.posEnd = rSelRegion.posStart;
	rNewRegion.posStart.Set(0, rSelRegion.posStart.row);

	ClearRegion();
	rSelRegion = rNewRegion;
	RequestRedrawRegion(&rSelRegion);
	pView->SetCaretPosition(rSelRegion.posStart);
	pDoc->CloseUndoRegion();
}

void YAEditImpl::CmdSelEndOfLogicalLine()
{
	Region rNewRegion;
	rNewRegion.posStart = rSelRegion.posEnd;

	LineChunk lc;
	pLineMgr->GetLineChunk(rSelRegion.posEnd.row, &lc);
	rNewRegion.posEnd.Set(lc.LineLen(), rSelRegion.posEnd.row);

	ClearRegion();
	rSelRegion = rNewRegion;
	RequestRedrawRegion(&rSelRegion);
	pView->SetCaretPosition(rSelRegion.posEnd);
	pDoc->CloseUndoRegion();

}

void YAEditImpl::CmdSelTopOfDoc()
{
	Region rNewRegion;
	rNewRegion.posEnd = rSelRegion.posStart;
	rNewRegion.posStart.Set(0, 0);

	ClearRegion();
	rSelRegion = rNewRegion;
	RequestRedrawRegion(&rSelRegion);
	pView->SetCaretPosition(rSelRegion.posStart);
	pDoc->CloseUndoRegion();
}

void YAEditImpl::CmdSelEndOfDoc()
{
	Region rNewRegion;
	rNewRegion.posStart = rSelRegion.posEnd;

	LineChunk lc;
	pLineMgr->GetLineChunk(pLineMgr->MaxLine() - 1, &lc);
	rNewRegion.posEnd.Set(lc.LineLen(), pLineMgr->MaxLine() - 1);

	ClearRegion();
	rSelRegion = rNewRegion;
	RequestRedrawRegion(&rSelRegion);
	pView->SetCaretPosition(rSelRegion.posEnd);
	pDoc->CloseUndoRegion();
}

void YAEditImpl::CmdReplaceString(LPCTSTR p)
{
	ReplaceText(SelectedRegion(), p);
	pView->ResetScrollbar();
}

void YAEditImpl::CmdCut()
{
	if (pDoc->IsReadOnly()) return;

	pDoc->CloseUndoRegion();
	if (IsRegionSelected()) {
		if (!CopyToClipboard()) {
			MessageBox(pView->hViewWnd, TEXT("Copy to clipboard failed."), TEXT("ERROR"), MB_ICONWARNING | MB_OK);
		}
		ReplaceText(SelectedRegion(), TEXT(""));
		pView->ResetScrollbar();
	}
}

void YAEditImpl::CmdCopy()
{
	if (!CopyToClipboard()) {
		MessageBox(pView->hViewWnd, TEXT("Copy to clipboard failed."), TEXT("ERROR"), MB_ICONWARNING | MB_OK);
	}
}

void YAEditImpl::CmdPaste()
{
	if (pDoc->IsReadOnly()) return;

	pDoc->CloseUndoRegion();
	if (!InsertFromClipboard()) {
		MessageBox(pView->hViewWnd, TEXT("Paste from clipboard failed."), TEXT("ERROR"), MB_ICONWARNING | MB_OK);
	} else {
		pView->ResetScrollbar();
	}
}

void YAEditImpl::CmdBackSpace()
{
	if (pDoc->IsReadOnly()) return;

	if (IsRegionSelected()) {
		ReplaceText(SelectedRegion(), TEXT(""));
	} else {
		Region r;
		r.posEnd = pView->GetCaretPosition();
		pView->MoveLeft();
		r.posStart = pView->GetCaretPosition();
		if (!r.IsEmptyRegion()) ReplaceText(r, TEXT(""));
	}
	pDoc->CloseUndoRegion();
}

void YAEditImpl::CmdDeleteChar()
{
	if (pDoc->IsReadOnly()) return;

	pDoc->CloseUndoRegion();
	if (IsRegionSelected()) {
		ReplaceText(SelectedRegion(), TEXT(""));
	} else {
		Region r;
		r.posStart = pView->GetCaretPosition();
		pView->MoveRight();
		r.posEnd = pView->GetCaretPosition();
		if (!r.IsEmptyRegion()) ReplaceText(r, TEXT(""));
	}
}

void YAEditImpl::CmdSelAll()
{
	rSelRegion.posStart.Set(0, 0);

	LineChunk lc;
	pLineMgr->GetLineChunk(pLineMgr->MaxLine() - 1, &lc);
	rSelRegion.posEnd.Set(lc.LineLen(), pLineMgr->MaxLine() - 1);
	RequestRedrawRegion(&rSelRegion);
}

void YAEditImpl::CmdUndo()
{
	if (pDoc->IsReadOnly()) return;

	pDoc->Undo();
}

void YAEditImpl::CmdToggleReadOnly()
{
	pDoc->SetReadOnly(!pDoc->IsReadOnly());

	if (pCallback) {
		pCallback->ChangeReadOnlyStatusNotify(pDoc->IsReadOnly());
	}
}

void YAEditImpl::CmdToggleWrapMode(BOOL bWrap)
{
	bWrapLine = bWrap;
	SetWrapper();

	RECT r;
	GetClientRect(pView->hViewWnd, &r);
	// rewrap logical lines
	pWrapper->SetViewWidth(r.right - r.left - pView->nMaxCharWidth);
	pLineMgr->RecalcWrap(pWrapper);
	// reset view
	pView->UpdateMaxLineWidth();	// UpdateMaxLineWidth depends on LineManager so call after updating LineManager.

	// reset caret/region position
	pView->ResetPosition();
	pView->SetCaretPosition(Coordinate(0, 0));
	ClearSelectedRegion();	

	// update scrollbar
	pView->ResetScrollbar();

	// redraw screen
	pView->RedrawAllScreen();
}

void YAEditImpl::CmdToggleInsertMode()
{
	bInsertMode = !bInsertMode;
}

/////////////////////////////////////////////////////////////////////////////
//  RButton
/////////////////////////////////////////////////////////////////////////////

void YAEditImpl::OnRbuttonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	RECT rWinRect;
	GetWindowRect(hWnd, &rWinRect);

	WORD x = (WORD)(rWinRect.left + LOWORD(lParam));
	WORD y = (WORD)(rWinRect.top + HIWORD(lParam));

	OnLButtonDown(hWnd, wParam, lParam);
	OnLButtonUp(hWnd, wParam, lParam);
	pCallback->OnContextMenu(pView->hViewWnd, x, y);
}

/////////////////////////////////////////////////////////////////////////////
// WM_LBUTTONDOWN
/////////////////////////////////////////////////////////////////////////////

void YAEditImpl::OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{

	SetFocus();

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	// Tap&hold
	SHRGINFO rgi;
	rgi.cbSize = sizeof(SHRGINFO);
	rgi.hwndClient = hWnd;
	rgi.ptDown.x = LOWORD(lParam);
	rgi.ptDown.y = HIWORD(lParam);
	rgi.dwFlags = SHRG_RETURNCMD;

	if (SHRecognizeGesture(&rgi) == GN_CONTEXTMENU) {
		RECT r;
		GetWindowRect(hWnd, &r);

		pCallback->OnContextMenu(pView->hViewWnd, r.left + rgi.ptDown.x, r.top + rgi.ptDown.y);
		return;
	}

#endif

	WORD nMouseDrgStartX, nMouseDrgStartY;	// LButton down point by WM_LBUTTONDOWN
	nMouseDrgStartX = LOWORD(lParam);
	nMouseDrgStartY = HIWORD(lParam);

	pDoc->CloseUndoRegion();

	// move caret
	DWORD nNewRow = pView->DpLinePixelToLgLineNo(nMouseDrgStartY);
	if (nNewRow < pLineMgr->MaxLine()) {
		if (nNewRow > 0 && nNewRow - pView->GetBaseLineNo() >= pView->GetPageHeight()) {
			nNewRow--;
		}

		pView->SetNearCursorPos(nMouseDrgStartX, nNewRow);

		Region rOldRgn = SelectedRegion();
		rSelRegion.posStart = rSelRegion.posEnd = pView->GetCaretPosition();

		// crear previously selected region
		RequestRedrawRegion(&rOldRgn);
	}


	// get mouse capture
	SetCapture(hWnd);
	bMouseDown = TRUE;
}

// Hiragana = 3040 - 309F
// Hankaku-Katakana = FF71-FF9F

// 0 = control code
// 1 = SPC
// 2 = mark
// 3 = number
// 4 = alpha
// 5 = DBCS
static const BYTE codeType[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2,
  2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2,
  2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#if defined(PLATFORM_WIN32)
static BYTE getCharType(BYTE b)
{
	if (IsDBCSLeadByte(b)) {
		return 5;
	} else {
		return codeType[b];
	}
}

inline LPCTSTR NextLetter(LPCTSTR p) {
	return CharNext(p);
}

#else
static BYTE getCharType(WCHAR c)
{
	if (c < 0x7F) {
		return codeType[c];
	}
	return 5;
}

inline LPCTSTR NextLetter(LPCTSTR p) {
	return p+1;
}
#endif

void YAEditImpl::OnLButtonDblClick(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LineChunk lc;
	pLineMgr->GetLineChunk(rSelRegion.posStart.row, &lc);

	LPCTSTR pLine = lc.GetLineData();
	LPCTSTR pCur = pLine + rSelRegion.posStart.col;
	DWORD nCol = rSelRegion.posStart.col;
	BYTE nCurType;

	// get start position
	nCurType = getCharType(*pCur);

	LPCTSTR p = pLine;
	LPCTSTR q = p;
	while (p < pCur) {
		if (getCharType(*p) != nCurType) {
			q = p;
		}
		p = NextLetter(p);
	}
	if (q < pCur && getCharType(*q) != nCurType) q = NextLetter(q);
	DWORD nStartPos = q - pLine;

	// get end position
	LPCTSTR pEnd = pLine + lc.LineLen();
	p = pCur;
	q = pEnd;

	while (p < pEnd) {
		if (getCharType(*p) != nCurType) {
			q = p;
			break;
		}
		p = NextLetter(p);
	}
	DWORD nEndPos = q - pLine;

	// update select region
	rSelRegion.posEnd.row = rSelRegion.posStart.row;
	rSelRegion.posStart.col = nStartPos;
	rSelRegion.posEnd.col = nEndPos;
	RequestRedrawRegion(&rSelRegion);

	// update caret position
	pView->SetCaretPosition(rSelRegion.posEnd);
	pView->ScrollCaret();

}

/////////////////////////////////////////////////////////////////////////////
//  Mouse move
/////////////////////////////////////////////////////////////////////////////

#ifndef MAKEPOINTS
#define MAKEPOINTS(l)   (*((POINTS FAR *) & (l))) 
#endif

void YAEditImpl::SetSelectionFromPoint(int xPos, int yPos)
{
	// when SetCaptured, cursor pos may be negative.
	if (xPos < 0) xPos = 0;
	if (yPos < 0) {
		pView->Prev1L();
		yPos = 0;
	}
	if (xPos > pView->GetViewClientRect().right) xPos = pView->GetViewClientRect().right - 1;
	if (yPos > pView->GetViewClientRect().bottom) {
		yPos = pView->GetViewClientRect().bottom - 1;
		pView->Next1L();
	}

	// move cursor
	DWORD nNewRow = pView->DpLinePixelToLgLineNo(yPos);
	if (nNewRow < pLineMgr->MaxLine()) {
		pView->SetNearCursorPos(xPos, nNewRow);
		UpdateSelRegion();
	}
}

void YAEditImpl::OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (!bMouseDown) return;

	POINTS ptPos;
	ptPos = MAKEPOINTS(lParam);
	int xPos = ptPos.x;
	int yPos = ptPos.y;

	if (xPos < 0 || yPos < 0 || xPos > pView->GetViewClientRect().right || yPos > pView->GetViewClientRect().bottom) {
		if (!bScrollTimerOn) {
			SetTimer(hWnd, IDT_SELSCROLL, 50, NULL);
			bScrollTimerOn = TRUE;
			ptMousePos.x = ptPos.x;
			ptMousePos.y = ptPos.y;
		}
	} else {
		if (bScrollTimerOn) {
			KillTimer(hWnd, IDT_SELSCROLL);
			bScrollTimerOn = FALSE;
		}
	}
	SetSelectionFromPoint(xPos, yPos);
}

/////////////////////////////////////////////////////////////////////////////
// WM_LBUTTONUP
/////////////////////////////////////////////////////////////////////////////

void YAEditImpl::OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (!bMouseDown) return;

	// kill scroll timer
	if (bScrollTimerOn) {
		KillTimer(hWnd, IDT_SELSCROLL);
		bScrollTimerOn = FALSE;
	}

	// release mouse capture
	if (GetCapture() == hWnd) ReleaseCapture();
	bMouseDown = FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// WM_TIMER
/////////////////////////////////////////////////////////////////////////////

void YAEditImpl::OnTimer(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam) {
	case IDT_SELSCROLL:
		SetSelectionFromPoint(ptMousePos.x, ptMousePos.y);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Set new document
/////////////////////////////////////////////////////////////////////////////

YAEditDoc *YAEditImpl::SetDoc(YAEditDoc *pNewDoc)
{
	// replace document and LineManager
	YAEditDoc *pOldDoc = pDoc;
	pDoc = pNewDoc;
	pLineMgr->ReleaseBuffer();

	RECT r;
	GetClientRect(pView->hViewWnd, &r);

	// rewrap logical lines
	pWrapper->SetViewWidth(r.right - r.left - pView->nMaxCharWidth);
	pLineMgr->RecalcWrap(pWrapper);
	// reset view
	pView->UpdateMaxLineWidth();	// UpdateMaxLineWidth depends on LineManager so call after updating LineManager.

	// reset caret/region position
	pView->ResetPosition();
	pView->SetCaretPosition(Coordinate(0, 0));
	ClearSelectedRegion();	

	// update scrollbar
	pView->ResetScrollbar();

	// redraw screen
	pView->RedrawAllScreen();
	return pOldDoc;
}

/////////////////////////////////////////////////////////////////////////////
// get max line width for decide hscroll range
/////////////////////////////////////////////////////////////////////////////

DWORD YAEditImpl::GetLineWidth(DWORD nOffset, LPCTSTR pStr, DWORD nLen) { return pView->GetLineWidth(nOffset, pStr, nLen); }

/////////////////////////////////////////////////////////////////////////////
// Resize window
/////////////////////////////////////////////////////////////////////////////

void YAEditImpl::OnResize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	DWORD width = (DWORD)LOWORD(lParam);
	DWORD height = (DWORD)HIWORD(lParam);

	Region rPhRgn;

	// Preserve absolute cursor position before rewrapping.
	Coordinate cPhCursorPos;
	pLineMgr->LogicalPosToPhysicalPos(&(pView->GetCaretPosition()), &cPhCursorPos);
	pLineMgr->LogicalPosToPhysicalPos(&(rSelRegion.posStart), &(rPhRgn.posStart));
	pLineMgr->LogicalPosToPhysicalPos(&(rSelRegion.posEnd), &(rPhRgn.posEnd));

	pView->ResizeNotify();

	RECT r;
	GetClientRect(pView->hViewWnd, &r);

	// rewrap logical lines
	pWrapper->SetViewWidth(r.right - r.left - pView->nMaxCharWidth);
	pLineMgr->RecalcWrap(pWrapper);

	pView->ResetScrollbar();

	// get cursor position after rewrapping.
	Coordinate cLgCursorPos;
	pLineMgr->PhysicalPosToLogicalPos(&cPhCursorPos, &cLgCursorPos);
	pView->SetCaretPosition(cLgCursorPos);
	pLineMgr->PhysicalPosToLogicalPos(&(rPhRgn.posStart), &(rSelRegion.posStart));
	pLineMgr->PhysicalPosToLogicalPos(&(rPhRgn.posEnd), &(rSelRegion.posEnd));

	// redraw screen
	pView->RedrawAllScreen();
}

void YAEditImpl::ResizeWindow(int x, int y, int width, int height) 
{
	// resizing and re-configure logical lines
	MoveWindow(pView->hViewWnd, x, y, width, height, TRUE);
}

BOOL YAEditImpl::Show(int nCmdShow)
{
	ShowWindow(pView->hViewWnd, nCmdShow);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// Region
/////////////////////////////////////////////////////////////////////////////
void YAEditImpl::SetFocus()
{
	::SetFocus(pView->hViewWnd);
}

/////////////////////////////////////////////////////////////////////////////
// Region
/////////////////////////////////////////////////////////////////////////////

void YAEditImpl::ClearRegion()
{
	RequestRedrawRegion(&SelectedRegion());
	ClearSelectedRegion();
}

void YAEditImpl::ClearSelectedRegion()
{
	rSelRegion.posEnd = pView->GetCaretPosition();
	rSelRegion.posStart = rSelRegion.posEnd;
}

/////////////////////////////////////////////////////////////////////////////
// Insert String considering cursor move
/////////////////////////////////////////////////////////////////////////////

BOOL YAEditImpl::ReplaceText(const Region &rLg, LPCTSTR pText)
{
	Region r;
	pLineMgr->LogicalPosToPhysicalPos(&(rLg.posStart), &(r.posStart));
	pLineMgr->LogicalPosToPhysicalPos(&(rLg.posEnd), &(r.posEnd));
	return pDoc->ReplaceString(&r, pText);
}

/////////////////////////////////////////////////////////////////////////////
// Clipboard
/////////////////////////////////////////////////////////////////////////////

BOOL YAEditImpl::CopyToClipboard()
{
	Tombo_Lib::Clipboard cb;

	// Get region data
	DWORD nRgnSize = GetRegionSize();
	if (nRgnSize == 0) return TRUE;
	TString sRgnStr;
	if (!sRgnStr.Alloc(nRgnSize + 1)) return FALSE;
	if (!GetRegionString(sRgnStr.Get())) return FALSE;

	// Set data to clipboard
	if (!cb.Open(pView->hViewWnd)) return FALSE;
	if (!cb.SetText(sRgnStr.Get())) return FALSE;
	cb.Close();

	return TRUE;
}

BOOL YAEditImpl::InsertFromClipboard()
{
	// Get data from clipboard
	Tombo_Lib::Clipboard cb;

	if (!cb.Open(pView->hViewWnd)) return FALSE;
	LPTSTR pText = cb.GetText();
	cb.Close();
	if (pText == NULL) return TRUE;

	// Insert to buffer
	// allocation check has finished, so this case is clipboard is empty or unknown format, so return TRUE.
	if (!ReplaceText(SelectedRegion(), pText)) return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// region
/////////////////////////////////////////////////////////////////////////////

DWORD YAEditImpl::GetRegionSize()
{
	if (IsSelRegionOneLine()) {
		return rSelRegion.posEnd.col - rSelRegion.posStart.col;
	} else {
		DWORD nSize = 0;
		DWORD i;
		for (i = rSelRegion.posStart.row; i <= rSelRegion.posEnd.row; i++) {
			LineChunk lc;
			if (!GetLgLineChunk(i, &lc)) return 0;

			if (i == rSelRegion.posStart.row) {
				nSize += lc.LineLen() - rSelRegion.posStart.col;
				if (!lc.IsContLine()) nSize += 2;	// \r\n
			} else if (i == rSelRegion.posEnd.row) {
				nSize += rSelRegion.posEnd.col;
			} else {
				nSize += lc.LineLen();
				if (!lc.IsContLine()) nSize += 2;	// \r\n
			}
		}
		return nSize;
	}
}

BOOL YAEditImpl::GetRegionString(LPTSTR pBuf)
{
	if (IsSelRegionOneLine()) {
		LineChunk lc;
		if (!GetLgLineChunk(rSelRegion.posStart.row, &lc)) return FALSE;
		DWORD n = rSelRegion.posEnd.col - rSelRegion.posStart.col;
		_tcsncpy(pBuf, lc.GetLineData() + rSelRegion.posStart.col, n);
		*(pBuf + n) = TEXT('\0');
		return TRUE;
	} else {
		LPTSTR p = pBuf;
		DWORD n;

		DWORD i;
		for (i = rSelRegion.posStart.row; i <= rSelRegion.posEnd.row; i++) {
			LineChunk lc;
			if (!GetLgLineChunk(i, &lc)) return FALSE;

			if (i == rSelRegion.posStart.row) {
				n = lc.LineLen() - rSelRegion.posStart.col;
				_tcsncpy(p, lc.GetLineData() + rSelRegion.posStart.col, n);
				p += n;

				if (!lc.IsContLine()) {
					// add eol mark
					*p++ = TEXT('\r');
					*p++ = TEXT('\n');
				}
			} else if (i == rSelRegion.posEnd.row) {
				n = rSelRegion.posEnd.col;
				_tcsncpy(p, lc.GetLineData(), n);
				p += n;
			} else {
				n = lc.LineLen();
				_tcsncpy(p, lc.GetLineData(), n);
				p += n;

				if (!lc.IsContLine()) {
					// add eol mark
					*p++ = TEXT('\r');
					*p++ = TEXT('\n');
				}
			}

		}
		*p = TEXT('\0');
		return TRUE;
	}
}

void YAEditImpl::ExtendSelectRegion(const Coordinate &nCurrent, Coordinate *pPrev)
{
	if (!IsRegionSelected()) {
		*pPrev = rSelRegion.posStart;

		if (rSelRegion.posStart < nCurrent) {
			bForwardDrag = TRUE;
			rSelRegion.posEnd = nCurrent;
		} else {
			bForwardDrag = FALSE;
			rSelRegion.posStart = nCurrent;
		}
	} else {
		if (bForwardDrag) {
			if (nCurrent < rSelRegion.posStart) {
				// turn to backward drag
				bForwardDrag = FALSE;
				*pPrev = rSelRegion.posEnd;
				rSelRegion.posEnd = rSelRegion.posStart;
				rSelRegion.posStart = nCurrent;
			} else {
				*pPrev = rSelRegion.posEnd;
				rSelRegion.posEnd = nCurrent;
			}
		} else {
			if (nCurrent > rSelRegion.posEnd) {
				// turn to forward drag
				bForwardDrag = TRUE;
				*pPrev = rSelRegion.posStart;
				rSelRegion.posStart = rSelRegion.posEnd;
				rSelRegion.posEnd = nCurrent;
			} else {
				*pPrev = rSelRegion.posStart;
				rSelRegion.posStart = nCurrent;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Update notify from YAEditDoc
/////////////////////////////////////////////////////////////////////////////

BOOL YAEditImpl::UpdateNotify(PhysicalLineManager *pPhMgr, const Region *pOldRegion, const Region *pNewRegion, DWORD nBefPhLines, DWORD nAftPhLines, DWORD nAffeLines)
{
	DWORD nBefLgLines = pLineMgr->MaxLine();

	Coordinate cLgAfStart;
	DWORD nAffLgLines;
	if (!pLineMgr->AdjustLgLines(pPhMgr, pWrapper, *pOldRegion, nBefPhLines, nAftPhLines, nAffeLines, &cLgAfStart, &nAffLgLines)) return FALSE;

	DWORD nAftLgLines = pLineMgr->MaxLine();

	// adjust caret position
	Coordinate cCaretPos;
	pLineMgr->PhysicalPosToLogicalPos(&(pNewRegion->posEnd), &cCaretPos);
	pView->SetCaretPosition(cCaretPos);
	ClearSelectedRegion();

	// update view
	// what a inefficient logic!!
	RequestRedraw(cLgAfStart.row, 0, TRUE);

	pView->ScrollCaret();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Set font
/////////////////////////////////////////////////////////////////////////////

void YAEditImpl::SetFont(HFONT hFont)
{
	if (hFont == NULL) {
		hFont = (HFONT)GetStockObject(SYSTEM_FONT);
	}
	pView->SetFont(hFont);
	pWrapper->SetViewWidth(pView->GetViewClientRect().right - pView->GetViewClientRect().left - pView->nMaxCharWidth);

	if (pLineMgr) delete pLineMgr;
	pLineMgr = new LineManager();
	if (!pLineMgr->Init(this)) return;
	SetDoc(pDoc);
}

/////////////////////////////////////////////////////////////////////////////
// get caret position
/////////////////////////////////////////////////////////////////////////////

DWORD YAEditImpl::GetCaretPos()
{
	Coordinate lgCur = pView->GetCaretPosition();
	Coordinate phCur;
	pLineMgr->LogicalPosToPhysicalPos(&lgCur, &phCur);

	Region r;

	r.posStart.Set(0, 0);
	r.posEnd.Set(phCur.col, phCur.row);
	
	return pDoc->GetDataBytes(&r);
}

void YAEditImpl::SetCaretPos(DWORD n)
{
	Coordinate phCur, lgCur;
	pDoc->ConvertBytesToCoordinate(n, &phCur);
	pLineMgr->PhysicalPosToLogicalPos(&phCur, &lgCur);
	pView->SetCaretPosition(lgCur);
	ClearRegion();
	pView->ScrollCaret();
}

BOOL YAEditImpl::GetLgLineChunk(DWORD nLineNo, LineChunk *pChunk)
{
	// retrieve line data info.
	if (!GetLineMgr()->GetLineChunk(nLineNo, pChunk)) return FALSE;
	pChunk->SetSelRegion(&SelectedRegion());
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// get previous position
/////////////////////////////////////////////////////////////////////////////
// In DBCS char set, it is difficult to detmine previous char. It may be -1, or may be -2.
// In Unicode(UCS-2), this is simply -1 letter(2 bytes).

DWORD YAEditImpl::GetPrevOffset(DWORD n, DWORD nPos)
{
#if defined(PLATFORM_WIN32)
	LineChunk lc;
	if (!GetLineMgr()->GetLineChunk(n, &lc)) return FALSE;

	const char *p = lc.GetLineData();
	if (!p) return 0;

	const char *r = p + nPos;
	const char *q = p;
	const char *pPrevChar = NULL;
	while(*q && r > q) {
		pPrevChar = q;
		if (IsDBCSLeadByte(*q)) {
			q++;
		}
		q++;
	}
	return pPrevChar ? nPos - (pPrevChar - p) : 0;
#else
	return 1;
#endif
}

/////////////////////////////////////////////////////////////////////////////
// select region
/////////////////////////////////////////////////////////////////////////////

void YAEditImpl::SetSelectRegion(DWORD nStartPos, DWORD nEndPos)
{
	Coordinate phCur;

	Coordinate lgStartPos;
	pDoc->ConvertBytesToCoordinate(nStartPos, &phCur);
	pLineMgr->PhysicalPosToLogicalPos(&phCur, &lgStartPos);

	Coordinate lgEndPos;
	pDoc->ConvertBytesToCoordinate(nEndPos, &phCur);
	pLineMgr->PhysicalPosToLogicalPos(&phCur, &lgEndPos);

	ClearRegion();
	// set selected region
	rSelRegion.posStart = lgStartPos;
	rSelRegion.posEnd = lgEndPos;

	RequestRedrawRegion(&rSelRegion);
	pView->SetCaretPosition(lgEndPos);
	pView->ScrollCaret();

}
