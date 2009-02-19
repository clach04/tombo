#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
#include <aygshell.h>
#endif
#include "Tombo.h"
#include "resource.h"
#include "Property.h"
#include "MemoSelectView.h"

#define KEY_CTRL_C 3
#define KEY_CTRL_V 22
#define KEY_CTRL_X 24

extern "C" {
//typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
}

static SUPER_WND_PROC gSuperProc;
static HINSTANCE hInst;
static HWND hParentWnd;
static MemoSelectView *pView;

#include "Logger.h"

void SelectViewSetWndProc(SUPER_WND_PROC wp, HWND hParent, HINSTANCE h, MemoSelectView *p)
{
	gSuperProc = wp;
	hParentWnd = hParent;
	hInst = h;
	pView = p;
}

LRESULT CALLBACK NewSelectViewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
#ifdef SEPVIEW_DEBUG
	case WM_LBUTTONDOWN:
		g_Logger.WriteLog("WM_LBUTTONDOWN\r\n");
		break;
	case WM_LBUTTONUP:
		g_Logger.WriteLog("WM_LBUTTONUP\r\n");
		break;
#endif
#if defined(PLATFORM_HPC)
	case WM_LBUTTONDOWN:
		{
			BOOL bAltButton = (GetKeyState(VK_MENU) & 0x8000) != 0;
			if (bAltButton) {
				POINT pt;
				pt.x = LOWORD(lParam); pt.y = HIWORD(lParam);
				TV_HITTESTINFO hti;

				hti.pt = pt;
				HTREEITEM hX = TreeView_HitTest(hwnd, &hti);
				if (hX) {
					TreeView_SelectItem(hwnd, hX);
				}
				return 0;
			}
			break;
		}
	case WM_LBUTTONUP:
		{
			BOOL bAltButton = (GetKeyState(VK_MENU) & 0x8000) != 0;
			if (bAltButton) {
				POINT pt;
				pt.x = LOWORD(lParam); pt.y = HIWORD(lParam);
				pView->OnNotify_RClick(pt);
				return 0;
			}
			break;
		}
#endif
	case WM_CHAR:
		{
			// ignore events for disabling click beeps when focusing.
			if (!pView) return 0;
			HTREEITEM hItem;
			TreeViewItem *pItem = pView->GetCurrentItem(&hItem);
			switch(wParam) {
			case KEY_CTRL_C:
				pView->OnCopy(pItem);
				break;
			case KEY_CTRL_X:
				pView->OnCut(pItem);
				break;
			case KEY_CTRL_V:
				pView->OnPaste();
				break;
			}
		return 0;
		}
	case WM_KEYDOWN:
		{
			BOOL bShiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;

			if (wParam == VK_DELETE) {
				SendMessage(hParentWnd, WM_COMMAND, IDM_DELETEITEM, 0);
				return 0;
			}
			if (wParam == VK_LEFT && bShiftDown) {
				pView->SelUpFolderWithoutOpen();
				return 0;
			}
			if (wParam == VK_UP && bShiftDown) {
				pView->SelPrevBrother();
				return 0;
			}
			if (wParam == VK_DOWN && bShiftDown) {
				pView->SelNextBrother();
				return 0;
			}
		}
		break;
	case WM_SETFOCUS:
		{
			if (g_Property.GetUseTwoPane() && pView) {
				pView->OnGetFocus();
			}
			break;
		}
	}
    return CallWindowProc(gSuperProc, hwnd, msg, wParam, lParam);
}
