#include <windows.h>
#include <tchar.h>
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
#include <aygshell.h>
#endif
#include "Tombo.h"
#include "resource.h"
#include "MemoDetailsView.h"

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
#include "PlatformLayer.h"
#endif

#include "Property.h"

static SUPER_WND_PROC gSuperProc;

#define KEY_ESC 0x1B
#define KEY_CTRL_A 1
#define KEY_CTRL_B 2
#define KEY_CTRL_C 3

#define KEY_COLON       0xBB
#define KEY_SEMICOLON   0xBA

void SetWndProc(SUPER_WND_PROC wp)
{
	gSuperProc = wp;
}

/////////////////////////////////////////
// Window procedure for sub classing editview
/////////////////////////////////////////
SUPER_WND_PROC gDefaultProc;
DWORD gDelta;

LRESULT CALLBACK DetailsViewSuperProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CREATE) {
		LPCREATESTRUCT pCS = (LPCREATESTRUCT)lParam;
		SimpleEditor *frm = (SimpleEditor*)pCS->lpCreateParams;
		SetWindowLong(hwnd, gDelta, (LONG)frm);
	}
	return CallWindowProc(gDefaultProc, hwnd, msg, wParam, lParam);
}

#if defined(PLATFORM_SIG3)
INT nSelBase = -1;
#endif

LRESULT CALLBACK NewDetailsViewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SimpleEditor *pView = (SimpleEditor*)GetWindowLong(hwnd, gDelta);
	if (pView == NULL) {
		return CallWindowProc(gDefaultProc, hwnd, msg, wParam, lParam);
	}

	switch(msg) {
	case WM_CLEAR:
		if (pView->IsReadOnly()) return 0;
		break;
	case WM_CUT:
		if (pView->IsReadOnly()) return 0;
		break;
	case WM_PASTE:
		if (pView->IsReadOnly()) return 0;
		break;
	case WM_KEYDOWN:
		if (pView->OnKeyDown(hwnd, wParam, lParam) == 0) return 0;
		break;
	case WM_CHAR:
		pView->SetMDSearchFlg(TRUE); // clear search status flag

		// if read only mode, ignore key events
		if (pView->IsReadOnly()) {
			if (wParam == KEY_CTRL_C) break;	// Ctrl-C should not disable
			return 0;
		}
		// disable Ctrl-B(BELL)
		if (wParam == KEY_CTRL_A || wParam == KEY_CTRL_B) return 0;
		break;

	case WM_COMMAND:
		pView->OnCommand(hwnd, wParam, lParam);
		break;

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	case WM_LBUTTONDOWN:
		{
			// clear search status
			pView->SetMDSearchFlg(TRUE);

			// Tap&hold
			SHRGINFO rgi;
			rgi.cbSize = sizeof(SHRGINFO);
			rgi.hwndClient = hwnd;
			rgi.ptDown.x = LOWORD(lParam);
			rgi.ptDown.y = HIWORD(lParam);
			rgi.dwFlags = SHRG_RETURNCMD;

			if (SHRecognizeGesture(&rgi) == GN_CONTEXTMENU) {
				HMENU hPopup = PLATFORM_TYPE::LoadDetailsViewPopupMenu();

				int x, y;
				UINT nFlg = TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN;
				RECT r;
				GetWindowRect(hwnd, &r);

				x = rgi.ptDown.x;
				if (x < (r.right - r.left) / 2) {
					nFlg |= TPM_LEFTALIGN;
					x++;
				} else {
					nFlg |= TPM_RIGHTALIGN;
					x--;
				}

				y = rgi.ptDown.y + r.top;

				if (rgi.ptDown.y + r.top < r.bottom / 2) {
					nFlg |= TPM_TOPALIGN;
					y++;
				} else {
					nFlg |= TPM_BOTTOMALIGN;
					y--;
				}

				DWORD nID = TrackPopupMenuEx(hPopup, nFlg, x, y, hwnd, NULL);
				DestroyMenu(hPopup);
				if (nID != 0) {

					if (nID == IDM_PASTE) {
						CallWindowProc(gSuperProc, hwnd, msg, wParam, lParam);
					}
					pView->OnCommand(hwnd, MAKEWPARAM(nID, 0), 0);
				}
				return 0;
			}
			break;
		}
#else
	case WM_LBUTTONDOWN:
		// clear search status
		pView->SetMDSearchFlg(TRUE);
#if defined(PLATFORM_SIG3)
		{
			INT xPos, yPos;
			xPos = (INT)LOWORD(lParam);
			yPos = (INT)HIWORD(lParam);
			LPARAM l = MAKELPARAM(xPos, yPos);
			nSelBase = SendMessage(hwnd, EM_CHARFROMPOS, 0, l) & 0xFFFF;
		}
#endif
		break;
#endif
	case WM_SETFOCUS:
		{
			if (pView) pView->OnGetFocus();
			break;
		}
	}

	LRESULT lResult = CallWindowProc(gSuperProc, hwnd, msg, wParam, lParam);
	if (pView && msg != EM_GETMODIFY) {
		pView->SetModifyStatus();
	}
    return lResult;
}
