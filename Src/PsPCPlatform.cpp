#if defined(PLATFORM_PSPC)
#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "PlatformLayer.h"
#include "PsPCPlatform.h"
#include "SipControl.h"

#define NUM_IMG_BUTTONS 12

#define BOOKMARK_MENU_POS 2

#define NUM_CMDBAR_BUTTONS 4
static TBBUTTON aCmdBarButtons[NUM_CMDBAR_BUTTONS] = {
	{0,  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{17, IDM_NEWMEMO   , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
//	{0,  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{22, IDM_SEARCH_PREV,              0, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{23, IDM_SEARCH_NEXT,              0, TBSTYLE_BUTTON, 0, 0, 0, -1},
//	{0,  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
};

#define NUM_MD_CMDBAR_BUTTONS 7
static TBBUTTON aMDCmdBarButtons[NUM_MD_CMDBAR_BUTTONS] = {
	{0,            0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{15,           IDM_RETURNLIST, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{STD_FILESAVE, IDM_SAVE,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,            0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{22,           IDM_SEARCH_PREV,              0, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{23,           IDM_SEARCH_NEXT,              0, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,            0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},

};

static HWND MakeCommandBar(HINSTANCE hInst, HWND hWnd, 
						   DWORD nCtlID, WORD nMenuID, 
						   TBBUTTON *pButtons,
						   DWORD nButton, DWORD nImages, DWORD nBitmap)
{
	HWND h;
	h = CommandBar_Create(hInst, hWnd, nCtlID);
	int boffset;
	CommandBar_AddBitmap(h, HINST_COMMCTRL,IDB_STD_SMALL_COLOR, 15, 0, 0);
	boffset = CommandBar_AddBitmap(h, hInst, nBitmap, nImages, 0, 0);

	CommandBar_InsertMenubar(h, hInst, nMenuID, 0);
	CommandBar_AddButtons(h, nButton, pButtons);
	CommandBar_AddAdornments(h, 0, 0);
	return h;
}

void PsPCPlatform::Create(HWND hWnd, HINSTANCE hInst)
{
	hMSCmdBar = MakeCommandBar(hInst, hWnd, ID_CMDBAR_MAIN,
								IDR_MENU_MAIN, aCmdBarButtons, 
								NUM_CMDBAR_BUTTONS, NUM_IMG_BUTTONS, 
								IDB_TOOLBAR);
	hMDCmdBar = MakeCommandBar(hInst, hWnd, ID_CMDBAR_DETAILS,
								IDR_MENU_DETAILS, aMDCmdBarButtons,
								NUM_MD_CMDBAR_BUTTONS, NUM_IMG_BUTTONS,
								IDB_TOOLBAR);
    CommandBar_Show(hMSCmdBar, TRUE);
}

void PsPCPlatform::EnableMenu(UINT uid, BOOL bEnable)
{
	HMENU hMenu;
	BOOL bMenu = TRUE;
	BOOL bTB = FALSE; HWND hTB = NULL;

	switch (uid) {
	case IDM_DECRYPT:
	case IDM_ENCRYPT:
	case IDM_DELETEITEM:
	case IDM_RENAME:
	case IDM_NEWMEMO:
	case IDM_CUT:
	case IDM_COPY:
	case IDM_PASTE:
	case IDM_NEWFOLDER:
	case IDM_GREP:
		hMenu = GetMSEditMenu();
		break;
	case IDM_SAVE:
		bMenu = FALSE;
		bTB = TRUE;
		hTB = hMDCmdBar;
	default:
		return;
	}

	if (bMenu) {
		if (bEnable) {
			EnableMenuItem(hMenu, uid, MF_BYCOMMAND | MF_ENABLED);
		} else {
			EnableMenuItem(hMenu, uid, MF_BYCOMMAND | MF_GRAYED);
		}
	}

	if (bTB) {
		SendMessage(hTB, TB_ENABLEBUTTON, uid, MAKELONG(bEnable, 0)); 
	}
}

void PsPCPlatform::OpenDetailsView()
{
	CommandBar_Show(hMSCmdBar, SW_HIDE);
	CommandBar_Show(hMDCmdBar, SW_SHOW);
}

void PsPCPlatform::CloseDetailsView()
{
	CommandBar_Show(hMDCmdBar, SW_HIDE);
	CommandBar_Show(hMSCmdBar, SW_SHOW);
}

void PsPCPlatform::EnableSearchNext()
{
	SendMessage(hMSCmdBar, TB_SETSTATE, IDM_SEARCH_PREV, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(hMSCmdBar, TB_SETSTATE, IDM_SEARCH_NEXT, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(hMDCmdBar, TB_SETSTATE, IDM_SEARCH_PREV, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(hMDCmdBar, TB_SETSTATE, IDM_SEARCH_NEXT, MAKELONG(TBSTATE_ENABLED, 0)); 
}

void PsPCPlatform::AdjustUserRect(RECT *r)
{
	DWORD nHOffset = CommandBar_Height(hMSCmdBar);
	r->top += nHOffset;
	r->bottom -= nHOffset;

	// SIP
	BOOL bStat;
	SipControl sc;
	if (!sc.Init()) return;
	if (!sc.GetSipStat(&bStat)) return;

	if (bStat) {
		RECT rSip = sc.GetRect();	
		r->bottom -= (rSip.bottom - rSip.top);
	}
}

#endif // PLATFORM_PSPC
