#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "StatusBar.h"

#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)

BOOL StatusBar::Create(HWND hWnd, BOOL bNew)
{
#if defined(PLATFORM_HPC)
	hStatusBar = CreateStatusWindow(WS_CHILD , TEXT(""), 
									hWnd, IDC_STATUS);
#endif
#if defined(PLATFORM_WIN32)
	hStatusBar = CreateStatusWindow(WS_CHILD | SBARS_SIZEGRIP, "", 
									hWnd, IDC_STATUS);
#endif

	ResizeStatusBar();
	SendMessage(hStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS , (LPARAM)"");
	return TRUE;
}

void StatusBar::ResizeStatusBar()
{
	RECT r;
	GetClientRect(hStatusBar, &r);
	DWORD nHeight = r.bottom - r.top;
	DWORD nWidth = r.right - r.left;

	int nSep[4];

	DWORD nWndSize = nHeight * 2;
	nSep[0] = nWidth - nWndSize*3 - nHeight;
	nSep[1] = nSep[0] + nWndSize;
	nSep[2] = nSep[1] + nWndSize;
	nSep[3] = nSep[2] + nWndSize;
	SendMessage(hStatusBar, SB_SETPARTS, (WPARAM)4, (LPARAM)nSep);
}

void StatusBar::Show(BOOL bShow)
{
	if (bShow) {
		ShowWindow(hStatusBar, SW_SHOW);
	} else {
		ShowWindow(hStatusBar, SW_HIDE);
	}
}

void StatusBar::SetStatusIndicator(DWORD nPos, LPCTSTR pText, BOOL bDisp)
{
	LPCTSTR p;
	if (bDisp) {
		p = pText;
	} else {
		p = TEXT("");
	}
	SendMessage(hStatusBar, SB_SETTEXT, nPos, (LPARAM)p);
}

WORD StatusBar::GetHeight()
{
	RECT rStatus;
	GetWindowRect(&rStatus);
	return (WORD)(rStatus.bottom - rStatus.top);
}

void StatusBar::SendSize(WPARAM wParam, LPARAM lParam)
{
	SendMessage(hStatusBar, WM_SIZE, wParam, lParam);
}

void StatusBar::GetWindowRect(RECT *p)
{
	::GetWindowRect(hStatusBar, p);
}

#else
///////////////////////////////////////////////////
// non support platform
///////////////////////////////////////////////////

BOOL StatusBar::Create(HWND hWnd, BOOL bNew) { return TRUE; }
void StatusBar::ResizeStatusBar() {}
void StatusBar::Show(BOOL bShow) {}
void StatusBar::SetStatusIndicator(DWORD nPos, LPCTSTR pText, BOOL bDisp) {}
WORD StatusBar::GetHeight() { return 0; }
void StatusBar::SendSize(WPARAM wParam, LPARAM lParam) {}
void StatusBar::GetWindowRect(RECT *p) {}

#endif