#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
#include <aygshell.h>
#endif
#include "Tombo.h"
#include "AutoPtr.h"
#include "VarBuffer.h"
#include "MainFrame.h"
#include "MemoDetailsView.h"
#include "resource.h"
#include "TString.h"
#include "UniConv.h"
#include "Property.h"
#include "SearchEngine.h"
#include "Message.h"
#include "TomboURI.h"
#include "Repository.h"
#include "MemoManager.h"

#define KEY_COLON       0xBB
#define KEY_SEMICOLON   0xBA

static BOOL GetDateText(TString *pInsStr, LPCTSTR pFormat, TString *pPath);

void SetWndProc(SUPER_WND_PROC wp);
LRESULT CALLBACK NewDetailsViewProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

LPCTSTR pMonth[12] = {
	TEXT("Jan"), TEXT("Feb"), TEXT("Mar"), TEXT("Apr"),
	TEXT("May"), TEXT("Jun"), TEXT("Jul"), TEXT("Aug"),
	TEXT("Sep"), TEXT("Oct"), TEXT("Nov"), TEXT("Dec")
};

LPCTSTR pWeekJ[7] = {
	TEXT("日"), TEXT("月"), TEXT("火"), TEXT("水"), TEXT("木"), TEXT("金"), TEXT("土")
};

LPCTSTR pWeekE[7] = {
	TEXT("Sun"), TEXT("Mon"), TEXT("Tue"), TEXT("Wed"), TEXT("Thr"), TEXT("Fri"), TEXT("Sat")
};

///////////////////////////////////////////
// 
///////////////////////////////////////////

MemoDetailsView::MemoDetailsView(MemoManager *pMgr) : pCurrentURI(NULL), pManager(pMgr)
{
}

MemoDetailsView::~MemoDetailsView()
{
	delete pCurrentURI;
}

const TomboURI* MemoDetailsView::GetCurrentURI()
{
	return pCurrentURI;
}

void MemoDetailsView::SetCurrentNote(const TomboURI *pURI)
{
	if (pCurrentURI == pURI) return;
	delete pCurrentURI;
	pCurrentURI = NULL;

	if (pURI) {
		pCurrentURI = new TomboURI(*pURI);
	}
}

///////////////////////////////////////////
// Discard current note
///////////////////////////////////////////
//
// note is discarded even if it is modifying.

BOOL MemoDetailsView::DiscardMemo()
{
	SetMemo(TEXT(""), 0, FALSE);
	SetCurrentNote(NULL);
	SetReadOnly(FALSE);
	pManager->GetMainFrame()->SetNewMemoStatus(TRUE);
	pManager->GetMainFrame()->SetWindowTitle(NULL);
	return TRUE;
}


BOOL MemoDetailsView::StoreCursorPos()
{
	if (g_Property.GetKeepCaret()) {

		DWORD nPos = GetCursorPos();
		DWORD nInitPos = GetInitialPos();

		if (pCurrentURI && nPos != nInitPos) {
			NoteAttribute attr;
			attr.nCursorPos = nPos;
			if (!g_Repository.SetAttribute(pCurrentURI, &attr)) return FALSE;
		}
	}
	return TRUE;
}

BOOL MemoDetailsView::Save(const TomboURI *pCurrentURI, TomboURI *pNewURI, TString *pNewHeadLine, LPCTSTR pText)
{
	if (!g_Repository.Update(pCurrentURI, pText, pNewURI, pNewHeadLine)) return FALSE;
	ResetModify();
	pManager->GetMainFrame()->SetModifyStatus(FALSE);
	SetCurrentNote(pNewURI);
	return TRUE;
}

BOOL MemoDetailsView::LoadNote(const TomboURI *pURI)
{
	// load note data and attribute
	LPTSTR p = g_Repository.GetNoteData(pURI);
	if (p == NULL) {
		SetMemo(MSG_CANT_OPEN_MEMO, 0, TRUE);
		return TRUE;
	}
	SecureBufferAutoPointerT sp(p);

	NoteAttribute attr;
	if (!g_Repository.GetAttribute(pURI, &attr)) return FALSE;

	// set memo to view
	SetMemo(p, attr.nCursorPos, attr.bReadOnly);
	SetCurrentNote(pURI);

	return TRUE;
}

////////////////////////////////////////////////////////
// Is this note are displayed in detailsview?
////////////////////////////////////////////////////////

BOOL MemoDetailsView::IsNoteDisplayed(const TomboURI *pURI)
{
	if (GetCurrentURI() == NULL) return FALSE;
	return _tcsicmp(pURI->GetFullURI(), GetCurrentURI()->GetFullURI()) == 0;
}

////////////////////////////////////////////////////////
// insert date
////////////////////////////////////////////////////////

void MemoDetailsView::InsertDate1()
{
	TString sDate;

	TString sPathStr;
	pManager->GetCurrentSelectedPath(&sPathStr);

	if (!GetDateText(&sDate, g_Property.GetDateFormat1(), &sPathStr)) {
		TomboMessageBox(NULL, MSG_GET_DATE_FAILED, TEXT("ERROR"), MB_ICONERROR | MB_OK);
		return;
	}
	ReplaceText(sDate.Get());
}

void MemoDetailsView::InsertDate2()
{
	TString sDate;

	TString sPathStr;
	pManager->GetCurrentSelectedPath(&sPathStr);

	if (!GetDateText(&sDate, g_Property.GetDateFormat2(), &sPathStr)) {
		TomboMessageBox(NULL, MSG_GET_DATE_FAILED, TEXT("ERROR"), MB_ICONERROR | MB_OK);
		return;
	}
	ReplaceText(sDate.Get());
}

/////////////////////////////////////////
// 検索
/////////////////////////////////////////
//
// RESULT: return TRUE if matched.
//
// bFirstSearch : 検索開始位置: TRUE = メモ先頭 FALSE = 現在のカーソル位置 + 1
// bForward : 検索の向き: TRUE = 順方向 FALSE = 逆方向
// bNFMsg: 見つからなかった場合にメッセージを出すか
// pFound: if string is found, set TRUE otherwise set FALSE.


BOOL MemoDetailsView::Search(BOOL bFirstSearch, BOOL bForward, BOOL bNFMsg, BOOL bSearchFromTop)
{
	SearchEngineA *pSE;
	pSE = pManager->GetSearchEngine();
	if (pSE == NULL) return FALSE;

	LPTSTR pT = GetMemo();
	SecureBufferAutoPointerT sb(pT);

	DWORD nSearchStart;
	BOOL bShift = FALSE;

	if (bFirstSearch) {
		nSearchStart = 0;
	} else {
		nSearchStart = GetCursorPos();
		bShift = TRUE;
	}

	BOOL bMatch;
	bMatch = pSE->SearchTextT(pT, nSearchStart, bForward, bShift);

	if (bMatch) {
		DWORD nStart = pSE->MatchStart();
		DWORD nEnd = pSE->MatchEnd();

		SetSelectRegion(nStart, nEnd);
	} else {
		if (bNFMsg) MessageBox(NULL, MSG_STRING_NOT_FOUND, TOMBO_APP_NAME, MB_OK | MB_ICONINFORMATION);
	}

	return bMatch;
}

///////////////////////////////////////////
// initializer
///////////////////////////////////////////

SimpleEditor::SimpleEditor(MemoManager *pMgr) : MemoDetailsView(pMgr), hViewWnd(NULL)
{
}


BOOL SimpleEditor::Init(DWORD id, DWORD id_nf)
{
	nID = id;
	nID_nf = id_nf;
	return TRUE;
}

extern SUPER_WND_PROC gDefaultProc;
extern DWORD gDelta;
LRESULT CALLBACK DetailsViewSuperProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

BOOL SimpleEditor::RegisterClass(HINSTANCE hInst)
{
	// superclassing
	WNDCLASS wc;
	GetClassInfo(hInst, TEXT("EDIT"), &wc);

	wc.hInstance = hInst;
	wc.lpszClassName = TEXT("TomboSimpleEditor");
	gDelta = wc.cbWndExtra;
	wc.cbWndExtra = ((wc.cbWndExtra + sizeof(MemoDetailsView*)) / 4 + 1)* 4;

	gDefaultProc = (SUPER_WND_PROC)wc.lpfnWndProc;

	wc.lpfnWndProc = DetailsViewSuperProc;

	return  ::RegisterClass(&wc) != 0;
}
///////////////////////////////////////////
// Create window
///////////////////////////////////////////
extern HINSTANCE g_hInstance;

BOOL SimpleEditor::Create(LPCTSTR pName, RECT &r, HWND hParent, HINSTANCE hInst, HFONT hFont)
{
	DWORD nWndStyle;

	nWndStyle = WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN;

#if defined(PLATFORM_PSPC) || defined(PLATFORM_BE500)
//	nLeftOffset = 3;
	// for draw border, left offset has disabled.
	nLeftOffset = 0;
#else
	nLeftOffset = 0;
#endif
	
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	hViewWnd_fd = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("TomboSimpleEditor"), pName, nWndStyle, 
							r.left + nLeftOffset, r.top, r.right - nLeftOffset, r.bottom, 
							hParent, (HMENU)nID, hInst, this);
	hViewWnd_nf = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("TomboSimpleEditor"), pName, nWndStyle | ES_AUTOHSCROLL | WS_HSCROLL, 
							r.left + nLeftOffset, r.top, r.right - nLeftOffset, r.bottom, 
							hParent, (HMENU)nID_nf, hInst, this);
#else
	hViewWnd_fd = CreateWindow(TEXT("TomboSimpleEditor"), pName, nWndStyle | WS_BORDER, 
							r.left + nLeftOffset, r.top, r.right - nLeftOffset, r.bottom, 
							hParent, (HMENU)nID, hInst, this);
	hViewWnd_nf = CreateWindow(TEXT("TomboSimpleEditor"), pName, nWndStyle | ES_AUTOHSCROLL | WS_HSCROLL | WS_BORDER, 
							r.left + nLeftOffset, r.top, r.right - nLeftOffset, r.bottom, 
							hParent, (HMENU)nID_nf, hInst, this);
#endif
	if (hViewWnd_fd == NULL || hViewWnd_nf == NULL) return FALSE;
	hViewWnd = hViewWnd_fd;

	// sub classing of edit control
	SUPER_WND_PROC wp = (SUPER_WND_PROC)GetWindowLong(hViewWnd, GWL_WNDPROC);
	SetWndProc(wp);
	SetWindowLong(hViewWnd_nf, GWL_WNDPROC, (LONG)NewDetailsViewProc);
	SetWindowLong(hViewWnd_fd, GWL_WNDPROC, (LONG)NewDetailsViewProc);

	if (hFont != NULL) {
		SetFont(hFont);
	}
	SetTabstop();
	return TRUE;
}

///////////////////////////////////////////
// hide/show window
///////////////////////////////////////////

BOOL SimpleEditor::Show(int nCmdShow)
{
	ShowWindow(hViewWnd, nCmdShow);
	if (nCmdShow == SW_SHOW) {
		bShowStatus = TRUE;
	} else if (nCmdShow == SW_HIDE) {
		bShowStatus = FALSE;
	}

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	// タップ&ホールドメニューが出たまま画面が切り替わった際にメニューを閉じさせる
	if (nCmdShow == SW_HIDE) {
		ReleaseCapture();
	}
#endif
	return UpdateWindow(hViewWnd);
}

///////////////////////////////////////////
// ウィンドウサイズの移動
///////////////////////////////////////////

void SimpleEditor::MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight)
{
	::MoveWindow(hViewWnd_nf, x + nLeftOffset, y, nWidth - nLeftOffset, nHeight, TRUE);
	::MoveWindow(hViewWnd_fd, x + nLeftOffset, y, nWidth - nLeftOffset, nHeight, TRUE);
}

///////////////////////////////////////////
// OnCommandの処理
///////////////////////////////////////////

BOOL SimpleEditor::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam)) {
	case IDM_CUT:
		SendMessage(hViewWnd, WM_CUT, 0, 0);
		return TRUE;
	case IDM_COPY:
		SendMessage(hViewWnd, WM_COPY, 0, 0);
		return TRUE;
	case IDM_PASTE:
		SendMessage(hViewWnd, WM_PASTE, 0, 0);
		return TRUE;
	case IDM_UNDO:
		SendMessage(hViewWnd, WM_UNDO, 0, 0);
		return TRUE;
	case IDM_ACTIONBUTTON:
		PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_RETURNLIST, 0), 0);
		return TRUE;
	case IDM_INSDATE1:
		{
			InsertDate1();
			return TRUE;
		}
	case IDM_INSDATE2:
		{
			InsertDate2();
			return TRUE;
		}
	case IDM_TOGGLEREADONLY:
		{
			SetReadOnly(!IsReadOnly());
			return FALSE;
		}
	}

	return FALSE;
}

///////////////////////////////////////////
// ホットキーの処理
///////////////////////////////////////////

BOOL SimpleEditor::OnHotKey(HWND hWnd, WPARAM wParam)
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
		PostMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDM_RETURNLIST, 0), 0);
		return TRUE;
	default:
		return FALSE;
	}
}

///////////////////////////////////////////
// Key handler
///////////////////////////////////////////
UINT SimpleEditor::OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	BOOL bShiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	BOOL bCtrlKeyDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

#if defined(PLATFORM_SIG3)
	if (bShiftDown && wParam == VK_UP) {
		INT nPrevStart, nPrevEnd;
		INT nAftStart, nAftEnd;
		SendMessage(hWnd, EM_GETSEL, (WPARAM)&nPrevStart, (LPARAM)&nPrevEnd);
		LRESULT lResult = CallWindowProc(gSuperProc, hWnd, msg, wParam, lParam);
		SendMessage(hWnd, EM_GETSEL, (WPARAM)&nAftStart, (LPARAM)&nAftEnd);

		if (nAftStart < nSelBase) {
			SendMessage(hwnd, EM_SETSEL, (WPARAM)nSelBase, (LPARAM)nAftStart);
			return lResult;
		} else {
			return lResult;
		}
	}
	if (bShiftDown && wParam == VK_DOWN) {
		INT nPrevStart, nPrevEnd;
		INT nAftStart, nAftEnd;
		SendMessage(hwnd, EM_GETSEL, (WPARAM)&nPrevStart, (LPARAM)&nPrevEnd);
		LRESULT lResult = CallWindowProc(gSuperProc, hwnd, msg, wParam, lParam);
		SendMessage(hwnd, EM_GETSEL, (WPARAM)&nAftStart, (LPARAM)&nAftEnd);

		if (nAftStart < nSelBase) {
			SendMessage(hwnd, EM_SETSEL, (WPARAM)nSelBase, (LPARAM)nAftEnd);
			return lResult;
		} else {
			return lResult;
		}
	}

	if (!(bShiftDown && wParam == VK_LEFT) && 
		!(bShiftDown && wParam == VK_RIGHT)) {
		POINT pt;
		GetCaretPos(&pt);
		LPARAM l = MAKELPARAM(pt.x, pt.y);
		nSelBase = SendMessage(hwnd, EM_CHARFROMPOS, 0, l) & 0xFFFF;
	}
#endif

	if (bCtrlKeyDown && wParam == TEXT('A')) {
		SelectAll();
		return 0;
	} else if (bCtrlKeyDown && wParam == TEXT('B')) {
		SetReadOnly(!IsReadOnly());
		return 0;
	}

	if (IsReadOnly()) {
		if (wParam == VK_DELETE) return 0;
		if (wParam == VK_BACK || wParam == VK_CONVERT || wParam == VK_LEFT) {
			SendMessage(hWnd, WM_KEYDOWN, VK_PRIOR, lParam);
			return 0;
		}
		if (wParam == VK_SPACE || wParam == VK_RIGHT) {
			SendMessage(hWnd, WM_KEYDOWN, VK_NEXT, lParam); 
			return 0;
		}
	} else {
		if (wParam == KEY_COLON && bCtrlKeyDown) {	// :
			InsertDate1();
		}
		if (wParam == KEY_SEMICOLON && bCtrlKeyDown) {	// ;
			InsertDate2();
		}
	}
	return 1;
}

///////////////////////////////////////////
// フォーカスの取得
///////////////////////////////////////////

void SimpleEditor::OnGetFocus()
{
	pManager->GetMainFrame()->NotifyDetailsViewFocused();
}

///////////////////////////////////////////
// メモの設定
///////////////////////////////////////////

BOOL SimpleEditor::SetMemo(LPCTSTR pMemo, DWORD nPos, BOOL bReadOnly)
{
	SetReadOnly(bReadOnly);

	nInitialPos = nPos;

	SetWindowText(hViewWnd, pMemo);
	if (g_Property.GetKeepCaret()) {
		SendMessage(hViewWnd, EM_SETSEL, nPos, nPos);
		PostMessage(hViewWnd, EM_SCROLLCARET, 0, 0);
	}
	SetModifyStatus();
	return TRUE;
}	

///////////////////////////////////////////
// メモの取得
///////////////////////////////////////////

LPTSTR SimpleEditor::GetMemo()
{
	DWORD n = GetWindowTextLength(hViewWnd);

	LPTSTR p = new TCHAR[n + 1];
	if (p == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	GetWindowText(hViewWnd, p,n+1);
	return p;
}

/////////////////////////////////////////
// フォントの設定
/////////////////////////////////////////

void SimpleEditor::SetFont(HFONT hFont)
{
	SendMessage(hViewWnd_fd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	SendMessage(hViewWnd_nf, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
}

/////////////////////////////////////////
// get cursor position
/////////////////////////////////////////

DWORD SimpleEditor::GetCursorPos()
{
	DWORD nPos;
	SendMessage(hViewWnd, EM_GETSEL, (WPARAM)&nPos, (LPARAM)NULL);
	return nPos;
}

/////////////////////////////////////////
// get partial path
/////////////////////////////////////////

static BOOL GetPartialPathFW(TString *pChoped, TString *pOrig, DWORD nLevel)
{
	if (!pChoped->Join(TEXT("\\"), pOrig->Get())) return FALSE;
	if (nLevel == 0) return TRUE;

	LPTSTR p = pChoped->Get() + 1;
	DWORD i = 0;
	while (*p) {
		if (*p == TEXT('\\')) {
			i++;
			if (i >= nLevel) {
				*p = TEXT('\0');
				return TRUE;
			}
		}

		p = CharNext(p);
	}
	return TRUE;
}

static LPCTSTR GetPartialPathBW(TString *pChoped, TString *pOrig, DWORD nLevel)
{
	if (!pChoped->Join(TEXT("\\"), pOrig->Get())) return NULL;
	if (nLevel == 0) return pChoped->Get();

	LPTSTR p = pChoped->Get() + 1;
	DWORD n = 0;
	while(*p) {
		if (*p == TEXT('\\')) {
			n++;
		}
		p = CharNext(p);
	}

	if (nLevel > n) {
		nLevel = n;
	}
	DWORD k = n - nLevel;

	p = pChoped->Get() + 1;
	DWORD i = 0;
	while(*p) {
		if (*p == TEXT('\\')) {
			i++;
			if (i > k) break;
		}
		p = CharNext(p);
	}
	return p;
}
/////////////////////////////////////////
// format date string
/////////////////////////////////////////

static BOOL GetDateText(TString *pInsStr, LPCTSTR pFormat, TString *pPath)
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	TString sPartPath;
	LPCTSTR pTop;
	DWORD nLv;

	// 文字バッファサイズのカウント
	DWORD nLen = 0;
	LPCTSTR p = pFormat;
	while(*p) {
#if defined(PLATFORM_WIN32)
		if (IsDBCSLeadByte(*p)) {
			p += 2;
			nLen += 2;
			continue;
		}
#endif
		if (*p == TEXT('%')) {
			p++;
			switch(*p) {
			case TEXT('y'):
				nLen += 4;
				break;
			case TEXT('b'):
				nLen += 3;
				break;
			case TEXT('w'):
			case TEXT('W'):
				nLen += 3;
				break;
			case TEXT('f'):
				if (_istdigit(*(p+1))) {
					nLv = *(p+1) - TEXT('0');
					p++;
				} else {
					nLv = 0;
				}
				if (!GetPartialPathFW(&sPartPath, pPath, nLv)) return FALSE;
				nLen += _tcslen(sPartPath.Get());
				break;
			case TEXT('F'):
				if (_istdigit(*(p+1))) {
					nLv = *(p+1) - TEXT('0');
					p++;
				} else {
					nLv = 0;
				}
				pTop = GetPartialPathBW(&sPartPath, pPath, nLv);
				if (!pTop) return FALSE;
				nLen += _tcslen(pTop);
				break;
			default:
				nLen += 2;
				break;
			}
			if (*p) p++;
		} else {
			p++;
			nLen++;
		}
	}
	nLen++;
	if (!pInsStr->Alloc(nLen)) return FALSE;

	LPTSTR q = pInsStr->Get();
	p = pFormat;
	while(*p) {
#if defined(PLATFORM_WIN32)
		if (IsDBCSLeadByte(*p)) {
			*q++ = *p++;
			*q++ = *p++;
			continue;
		}
#endif
		if (*p == TEXT('%')) {
			p++;
			switch(*p) {
			case TEXT('y'):
				wsprintf(q, TEXT("%4d"), st.wYear);
				q += 4;
				break;
			case TEXT('Y'):
				wsprintf(q, TEXT("%02d"), st.wYear % 100);
				q += 2;
				break;
			case TEXT('M'):
				wsprintf(q, TEXT("%02d"), st.wMonth);
				q += 2;
				break;
			case TEXT('b'):
				_tcscpy(q, pMonth[st.wMonth - 1]);
				q += 3;
				break;
			case TEXT('d'):
				wsprintf(q, TEXT("%02d"), st.wDay);
				q += 2;
				break;
			case TEXT('D'):
				wsprintf(q, TEXT("%d"), st.wDay);
				q++;
				if (st.wDay >= 10) q++;
				break;
			case TEXT('w'):
				_tcscpy(q, pWeekJ[st.wDayOfWeek]);
				q += _tcslen(pWeekJ[st.wDayOfWeek]);
				break;
			case TEXT('W'):
				_tcscpy(q, pWeekE[st.wDayOfWeek]);
				q += _tcslen(pWeekE[st.wDayOfWeek]);
				break;
			case TEXT('h'):
				wsprintf(q, TEXT("%02d"), st.wHour);
				q += 2;
				break;
			case TEXT('H'):
				wsprintf(q, TEXT("%02d"), st.wHour % 12);
				q += 2;
				break;
			case TEXT('I'):
				{
					int n = st.wHour % 12;
					if (n == 0) n = 12;
					wsprintf(q, TEXT("%02d"), n);
					q+= 2;
				}
				break;
			case TEXT('a'):
				if (st.wHour >= 12) {
					_tcscpy(q, TEXT("PM"));
				} else {
					_tcscpy(q, TEXT("AM"));
				}
				q += 2;
				break;
			case TEXT('m'):
				wsprintf(q, TEXT("%02d"), st.wMinute);
				q += 2;
				break;
			case TEXT('s'):
				wsprintf(q, TEXT("%02d"), st.wSecond);
				q += 2;
				break;
			case TEXT('n'):
				wsprintf(q, TEXT("\r\n"));
				q += 2;
				break;
			case TEXT('f'):
				if (_istdigit(*(p+1))) {
					nLv = *(p+1) - TEXT('0');
					p++;
				} else {
					nLv = 0;
				}
				if (!GetPartialPathFW(&sPartPath, pPath, nLv)) return FALSE;
				_tcscpy(q, sPartPath.Get());
				q += _tcslen(sPartPath.Get());
				break;
			case TEXT('F'):
				if (_istdigit(*(p+1))) {
					nLv = *(p+1) - TEXT('0');
					p++;
				} else {
					nLv = 0;
				}
				pTop = GetPartialPathBW(&sPartPath, pPath, nLv);
				if (!pTop) return FALSE;
				_tcscpy(q, pTop);
				q += _tcslen(pTop);
				break;
			default:
				*q++ = TEXT('%');
				if (*p) *q++ = *p++;
			}
			p++;
		} else {
			*q++ = *p++;
		}
	}
	*q = TEXT('\0');
	return TRUE;
}

/////////////////////////////////////////
// ステータス表示
/////////////////////////////////////////

void SimpleEditor::SetModifyStatus()
{
	pManager->GetMainFrame()->SetModifyStatus(IsModify());
}

/////////////////////////////////////////
// ステータス表示
/////////////////////////////////////////

void SimpleEditor::SelectAll() 
{	
	SetFocus();
	SendMessage(hViewWnd, EM_SETSEL, 0, -1); 
}

/////////////////////////////////////////
// 折り返し表示の切り替え
/////////////////////////////////////////

BOOL SimpleEditor::SetFolding(BOOL bFold)
{
	HWND hPrev;
	HWND hAfter;

	// 現在使用されているウィンドウの切り替え
	if (hViewWnd == hViewWnd_fd) {
		hPrev = hViewWnd_fd;
		hAfter = hViewWnd_nf;
	} else {
		hPrev = hViewWnd_nf;
		hAfter = hViewWnd_fd;
	}
	hViewWnd = hAfter;

	// テキスト本文の引継ぎ
	DWORD nLen = GetWindowTextLength(hPrev) + 1;
	LPTSTR p = new TCHAR[nLen];
	if (p == NULL) {
		hViewWnd = hPrev;
		return FALSE;
	}
	p[0] = TEXT('\0');
	GetWindowText(hPrev, p, nLen);
	SetWindowText(hAfter, p);
	delete [] p; p = NULL;

	// 更新状態の引継ぎ
	SendMessage(hAfter, EM_SETMODIFY, (WPARAM)SendMessage(hPrev, EM_GETMODIFY, 0, 0), 0);

	// カーソル位置の引継ぎ

	// 現在表示されている場合、表示状態を切り替え
	if (!bShowStatus) return TRUE;
	::ShowWindow(hPrev, SW_HIDE);
	::ShowWindow(hAfter, SW_SHOW);

	return TRUE;
}

/////////////////////////////////////////
// タブストップの設定
/////////////////////////////////////////

void SimpleEditor::SetTabstop() {
	DWORD n = g_Property.GetTabstop() * 4;
	SendMessage(hViewWnd_fd, EM_SETTABSTOPS, 1, (LPARAM)&n);
	SendMessage(hViewWnd_nf, EM_SETTABSTOPS, 1, (LPARAM)&n);
}

/////////////////////////////////////////
// change read only mode
/////////////////////////////////////////

void SimpleEditor::SetReadOnly(BOOL bro)
{
	bReadOnly = bro;
	pManager->GetMainFrame()->SetReadOnlyStatus(IsReadOnly());
}

/////////////////////////////////////////
//
/////////////////////////////////////////

void SimpleEditor::SetMDSearchFlg(BOOL bFlg)
{
	pManager->SetMDSearchFlg(bFlg);
}

/////////////////////////////////////////
// Insert date
/////////////////////////////////////////

BOOL SimpleEditor::ReplaceText(LPCTSTR p)
{
	SendMessage(hViewWnd, EM_REPLACESEL, 0, (LPARAM)p);
	return TRUE;
}

void SimpleEditor::SetSelectRegion(DWORD nStart, DWORD nEnd)
{
	SendMessage(hViewWnd, EM_SETSEL, (WPARAM)nStart, (LPARAM)nEnd);
	SendMessage(hViewWnd, EM_SCROLLCARET, 0, 0);
}
