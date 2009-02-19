#if defined(PLATFORM_HPC)
#include <windows.h>
#include <commctrl.h>

#include "resource.h"
#include "Property.h"
#include "Tombo.h"
#include "Message.h"

#include "StatusBar.h"
#include "PlatformLayer.h"
#include "HPCPlatform.h"
#include "Property.h"

#define NUM_MY_TOOLBAR_BMPS 0
#define NUM_IMG_BUTTONS 10

#define NUM_CMDBAR_BUTTONS 19
static TBBUTTON aCmdBarButtons[NUM_CMDBAR_BUTTONS] = {
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{STD_FILENEW + NUM_MY_TOOLBAR_BMPS,  IDM_NEWMEMO,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{STD_FILESAVE + NUM_MY_TOOLBAR_BMPS, IDM_SAVE,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{STD_CUT + NUM_MY_TOOLBAR_BMPS,      IDM_CUT,        TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{STD_COPY + NUM_MY_TOOLBAR_BMPS,     IDM_COPY,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{STD_PASTE + NUM_MY_TOOLBAR_BMPS,    IDM_PASTE,      TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{STD_DELETE + NUM_MY_TOOLBAR_BMPS,   IDM_DELETEITEM, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{18,                                 IDM_INSDATE1,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{19,                                 IDM_INSDATE2,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{21,                                 IDM_TOGGLEPANE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
	{24,                                 IDM_SEARCH,     TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{22,                                 IDM_SEARCH_PREV,              0, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{23,                                 IDM_SEARCH_NEXT,              0, TBSTYLE_BUTTON, 0, 0, 0, -1},
	{0,                                  0,              TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
};

///////////////////////////////////////
// methods implimentation
///////////////////////////////////////

HPCPlatform::HPCPlatform() : pStatusBar(NULL)
{
}

HPCPlatform::~HPCPlatform()
{
	delete pStatusBar;
}

static HWND GetCommandBar(HWND hBand, UINT uBandID)
{
	UINT idx = SendMessage(hBand, RB_IDTOINDEX, uBandID, 0);
	if (idx == -1) return NULL;
	HWND hwnd = CommandBands_GetCommandBar(hBand, idx);
	return hwnd;
}

void HPCPlatform::Create(HWND hWnd, HINSTANCE hInst)
{
	HWND hBand, hwnd;
	REBARBANDINFO arbbi[2];

	pStatusBar = new StatusBar();
	pStatusBar->Create(hWnd, g_Property.GetUseTwoPane());

	// CommandBand¶¬
	HIMAGELIST himl = ImageList_Create(16,16,ILC_COLOR, 0, 1);
	HBITMAP hBmp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_REBAR));
	ImageList_Add(himl, hBmp, (HBITMAP)NULL);
	DeleteObject(hBmp);

	hBand = CommandBands_Create(hInst, hWnd, IDC_CMDBAND,
								RBS_AUTOSIZE | RBS_VARHEIGHT | RBS_BANDBORDERS | RBS_SMARTLABELS, 
								himl);
	arbbi[0].cbSize = sizeof(REBARBANDINFO);
	arbbi[0].fMask = RBBIM_ID | RBBIM_STYLE | RBBIM_SIZE | RBBIM_IMAGE;
	arbbi[0].fStyle = RBBS_NOGRIPPER;
	arbbi[0].wID = ID_CMDBAR_MAIN;
	arbbi[0].cx = 280;
	arbbi[0].iImage = 0;

	arbbi[1].cbSize = sizeof(REBARBANDINFO);
	arbbi[1].fMask = RBBIM_ID | RBBIM_STYLE | RBBIM_SIZE;
	arbbi[1].fStyle = 0;
	arbbi[1].wID = ID_BUTTONBAND;
	arbbi[1].cx = 360;

	// restore commandbar info
	COMMANDBANDSRESTOREINFO cbri[NUM_COMMANDBAR];
	BOOL bRestoreFlg = g_Property.GetCommandbarInfo(cbri, NUM_COMMANDBAR);
	if (bRestoreFlg) {
		arbbi[0].fStyle = cbri[0].fStyle;
		arbbi[0].cx = cbri[0].cxRestored;

		arbbi[1].fMask |= RBBIM_STYLE;
		arbbi[1].fStyle = cbri[1].fStyle;
		arbbi[1].cx = cbri[1].cxRestored;
	}

	CommandBands_AddBands(hBand, hInst, 2, arbbi);
	// set first band(menu)

	HMENU h = LoadMainMenu();
	hwnd = GetCommandBar(hBand, ID_CMDBAR_MAIN);
	CommandBar_InsertMenubarEx(hwnd, NULL, (LPTSTR)h, 0);

	// set next band(buttons)
	hwnd = GetCommandBar(hBand, ID_BUTTONBAND);

	CommandBar_AddBitmap(hwnd, HINST_COMMCTRL, IDB_STD_SMALL_COLOR, 15, 0, 0);
	CommandBar_AddBitmap(hwnd, hInst, IDB_TOOLBAR, NUM_IMG_BUTTONS, 0, 0);

	CommandBar_AddButtons(hwnd, sizeof(aCmdBarButtons)/sizeof(TBBUTTON), aCmdBarButtons);

	hMSCmdBar = hBand;
	CommandBands_AddAdornments(hBand, hInst, 0, NULL);

	if (bRestoreFlg) {
		if (cbri[0].fMaximized) {
			SendMessage(hBand, RB_MAXIMIZEBAND, 0, (LPARAM)0);
		}
		if (cbri[1].fMaximized) {
			SendMessage(hBand, RB_MAXIMIZEBAND, 1, (LPARAM)0);
		}
	}

}

HWND HPCPlatform::GetMainToolBar()
{
	return GetCommandBar(hMSCmdBar, ID_BUTTONBAND);
}

HMENU HPCPlatform::GetMainMenu() 
{ 
	return CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0); 
}

HMENU HPCPlatform::GetMDToolMenu()
{
	return CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0);

}

HMENU HPCPlatform::GetMSEditMenu()
{
	return CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0);
}

HMENU HPCPlatform::GetMSBookMarkMenu()
{
	HMENU hMenu = GetMainMenu();
	return GetSubMenu(hMenu, BOOKMARK_MENU_POS);
}

void HPCPlatform::EnableMenu(UINT uId, BOOL bEnable)
{
	BOOL bTb = FALSE;
	HWND hTbWin;
	switch(uId) {
	case IDM_DELETEITEM:
	case IDM_NEWMEMO:
	case IDM_CUT:
	case IDM_COPY:
	case IDM_PASTE:
	case IDM_SAVE:
		bTb = TRUE;
		hTbWin = GetMainToolBar();
	default:
		break;
	}


	HMENU hMenu = GetMainMenu();
	UINT uFlg1;
	if (bEnable) {
		uFlg1 = MF_BYCOMMAND | MF_ENABLED;
	} else {
		uFlg1 = MF_BYCOMMAND | MF_GRAYED;
	}
	EnableMenuItem(hMenu, uId, uFlg1);

	if (bTb) {
		SendMessage(hTbWin, TB_ENABLEBUTTON, uId, MAKELONG(bEnable, 0));
	}
}

void HPCPlatform::ControlMenu(BOOL bTreeActive)
{
	HMENU hMenu = CommandBar_GetMenu(GetCommandBar(hMSCmdBar, ID_CMDBAR_MAIN), 0);

	UINT uFlg1, uFlg2;
	if (bTreeActive) {
		uFlg1 = MF_BYCOMMAND | MF_ENABLED;
		uFlg2 = MF_BYCOMMAND | MF_GRAYED;
	} else {
		uFlg1 = MF_BYCOMMAND | MF_GRAYED;
		uFlg2 = MF_BYCOMMAND | MF_ENABLED;
	}

	EnableMenuItem(hMenu, IDM_FORGETPASS, uFlg1);
	EnableMenuItem(hMenu, IDM_PROPERTY, uFlg1);

	EnableMenuItem(hMenu, IDM_INSDATE1, uFlg2);
	EnableMenuItem(hMenu, IDM_INSDATE2, uFlg2);
	EnableMenuItem(hMenu, IDM_DETAILS_HSCROLL, uFlg2);
}

void HPCPlatform::ControlToolbar(BOOL bTreeActive)
{
	HWND hToolBar = GetCommandBar(hMSCmdBar, ID_BUTTONBAND);
	SendMessage(hToolBar, TB_ENABLEBUTTON, IDM_INSDATE1, MAKELONG(!bTreeActive, 0));
	SendMessage(hToolBar, TB_ENABLEBUTTON, IDM_INSDATE2, MAKELONG(!bTreeActive, 0));
}

void HPCPlatform::OpenDetailsView()
{
	ControlMenu(FALSE);
	ControlToolbar(FALSE);
}

void HPCPlatform::CloseDetailsView()
{
	ControlMenu(TRUE);
	ControlToolbar(TRUE);
}

void HPCPlatform::EnableSearchNext()
{
	SendMessage(GetCommandBar(hMSCmdBar, ID_BUTTONBAND), TB_SETSTATE, IDM_SEARCH_PREV, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(GetCommandBar(hMSCmdBar, ID_BUTTONBAND), TB_SETSTATE, IDM_SEARCH_NEXT, MAKELONG(TBSTATE_ENABLED, 0)); 
}

void HPCPlatform::AdjustUserRect(RECT *r)
{
	DWORD nHOffset = CommandBands_Height(hMSCmdBar);

	WORD nStatusHeight;
	if (g_Property.GetHideStatusBar()) {
		nStatusHeight = 0;
	} else {
		nStatusHeight = GetStatusBarHeight();
	}

	r->top += nHOffset;
	r->bottom -= (nHOffset + nStatusHeight);

}

void HPCPlatform::CheckMenu(UINT uid, BOOL bCheck)
{
	HMENU hMenu;
	BOOL bNegButton = FALSE;
	switch(uid) {
	case IDM_TOGGLEPANE:
		bNegButton = TRUE;
		hMenu = GetMainMenu();
		break;
	case IDM_DETAILS_HSCROLL:
		hMenu = GetMDToolMenu();
		break;
	default:
		break;
	}

	BOOL bButton = bNegButton ? !bCheck : bCheck;
	// CheckMenuItem is superseeded funcs, but in CE, SetMenuItemInfo can't set values, so use it.
	CheckMenuItem(hMenu, uid, MF_BYCOMMAND | (bCheck ? MF_CHECKED : MF_UNCHECKED));
	SendMessage(GetCommandBar(hMSCmdBar, ID_BUTTONBAND), TB_PRESSBUTTON, uid, MAKELONG(bButton, 0));
}

void HPCPlatform::ShowStatusBar(BOOL bShow)
{
	pStatusBar->Show(bShow);
}

void HPCPlatform::SetStatusIndicator(DWORD nPos, LPCTSTR pText, BOOL bDisp)
{
	pStatusBar->SetStatusIndicator(nPos, pText, bDisp);
}

WORD HPCPlatform::GetStatusBarHeight()
{
	return pStatusBar->GetHeight();
}

void HPCPlatform::ResizeStatusBar(WPARAM wParam, LPARAM lParam)
{
	pStatusBar->SendSize(wParam, lParam);
	pStatusBar->ResizeStatusBar();
}

void HPCPlatform::GetStatusWindowRect(RECT *pRect)
{
	pStatusBar->GetWindowRect(pRect);
}

///////////////////////////////////////////////////
// Load i18nized menu
///////////////////////////////////////////////////

static void InsertBaseMenu(HMENU hMain, int pos, LPCTSTR pText, HMENU hSub)
{
	InsertMenu(hMain, pos, MF_BYPOSITION | MF_POPUP | MF_STRING , (UINT)hSub, pText);
}

static MenuMsgRes aFileMenu[] = {
	{  0, IDM_NEWMEMO,    0, MSG_ID_MENUITEM_W32_F_NEWMEMO },
	{  1, IDM_NEWFOLDER,  0, MSG_ID_MENUITEM_W32_F_NEWFOLDER },
	{  2, IDM_RENAME,     0, MSG_ID_MENUITEM_W32_F_RENAME },
	{  3, IDM_DELETEITEM, 0, MSG_ID_MENUITEM_W32_F_DEL },
	{  4, -1,             0, 0 },
	{  5, IDM_SAVE,       0, MSG_ID_MENUITEM_W32_F_SAVE },
	{  6, -1,             0, 0 },
	{  7, IDM_ABOUT,      0, MSG_ID_MENUITEM_W32_H_ABOUT },
	{  8, IDM_EXIT,       0, MSG_ID_MENUITEM_W32_F_EXIT },
};

static MenuMsgRes aEditMenu[] = {
	{  0, IDM_CUT,      0, MSG_ID_MENUITEM_W32_E_CUT },
	{  1, IDM_COPY,     0, MSG_ID_MENUITEM_W32_E_COPY },
	{  2, IDM_PASTE,    0, MSG_ID_MENUITEM_W32_E_PASTE },
	{  3, -1,           0, 0 },
	{  4, IDM_SELALL,   0, MSG_ID_MENUITEM_W32_E_SELALL },
	{  5, -1,           0, 0 },
	{  6, IDM_INSDATE1, 0, MSG_ID_MENUITEM_W32_E_DATE1 },
	{  7, IDM_INSDATE2, 0, MSG_ID_MENUITEM_W32_E_DATE2 },
};

static MenuMsgRes aBookMarkMenu[] = {
	{  0, IDM_BOOKMARK_ADD,    0, MSG_ID_MENUITEM_W32_B_ADDBM },
	{  1, IDM_BOOKMARK_CONFIG, 0, MSG_ID_MENUITEM_W32_B_EDITBM },
	{  2, -1,                  0, 0 },
};

static MenuMsgRes aFindMenu[] = {
	{  0, IDM_SEARCH,      0, MSG_ID_MENUITEM_W32_E_FIND },
	{  1, IDM_SEARCH_NEXT, 0, MSG_ID_MENUITEM_W32_E_FINDNEXT },
	{  2, IDM_SEARCH_PREV, 0, MSG_ID_MENUITEM_W32_E_FINDPREV },
	{  3, -1,              0, 0 },
	{  4, IDM_GREP,        0, MSG_ID_MENUITEM_W32_E_QFILTER },
	{  5, IDM_VFOLDER_DEF, 0, MSG_ID_MENUITEM_W32_T_VIRTUALFOLDER },
};

static MenuMsgRes aToolMenu[] = {
	{  0, IDM_DETAILS_HSCROLL, MF_CHECKED, MSG_ID_MENUITEM_W32_T_WRAPTEXT },
	{  1, IDM_TOGGLEPANE,      MF_CHECKED, MSG_ID_MENUITEM_W32_T_TWOPANE },
	{  2, IDM_SHOWSTATUSBAR,   MF_CHECKED, MSG_ID_MENUITEM_W32_T_STATUSBAR },
	{  3, -1,                  0,          0 },
	{  4, IDM_ENCRYPT,         0,          MSG_ID_MENUITEM_W32_T_ENCRYPT },
	{  5, IDM_DECRYPT,         0,          MSG_ID_MENUITEM_W32_T_DECRYPT },
	{  6, -1,                  0,          0 },
	{  7, IDM_FORGETPASS,      0,          MSG_ID_MENUITEM_W32_T_ERASEPASS },
	{  8, -1,                  0,          0 },
	{  9, IDM_PROPERTY,        0,          MSG_ID_MENUITEM_W32_T_OPTIONS },
};


HMENU HPCPlatform::LoadMainMenu()
{
	HMENU hMain = CreateMenu();
	HMENU hSub;
	OverrideMenuTitle(hSub = CreatePopupMenu(), aFileMenu, sizeof(aFileMenu)/sizeof(MenuMsgRes));
	InsertBaseMenu(hMain, 0, MSG_MENUITEM_W32B_FILE, hSub);
	OverrideMenuTitle(hSub = CreatePopupMenu(), aEditMenu, sizeof(aEditMenu)/sizeof(MenuMsgRes));
	InsertBaseMenu(hMain, 1, MSG_MENUITEM_W32B_EDIT, hSub);
	OverrideMenuTitle(hSub = CreatePopupMenu(), aBookMarkMenu, sizeof(aBookMarkMenu)/sizeof(MenuMsgRes));
	InsertBaseMenu(hMain, 2, MSG_MENUITEM_W32B_BOOKMARK, hSub);
	OverrideMenuTitle(hSub = CreatePopupMenu(), aFindMenu, sizeof(aFindMenu)/sizeof(MenuMsgRes));
	InsertBaseMenu(hMain, 3, MSG_MENUITEM_FIND, hSub);
	OverrideMenuTitle(hSub = CreatePopupMenu(), aToolMenu, sizeof(aToolMenu)/sizeof(MenuMsgRes));
	InsertBaseMenu(hMain, 4, MSG_MENUITEM_W32B_TOOL, hSub);

	return hMain;
}

static MenuMsgRes aContextMenu[] = {
	{  0, IDM_CUT,        0, MSG_ID_MENUITEM_MAIN_CUT },
	{  1, IDM_COPY,       0, MSG_ID_MENUITEM_MAIN_COPY },
	{  2, IDM_PASTE,      0, MSG_ID_MENUITEM_MAIN_PASTE },
	{  3, -1,             0, 0 },
	{  4, IDM_ENCRYPT,    0, MSG_ID_MENUITEM_MAIN_ENCRYPT },
	{  5, IDM_DECRYPT,    0, MSG_ID_MENUITEM_MAIN_DECRYPT },
	{  6, -1,             0, 0 },
	{  7, IDM_SEARCH,     0, MSG_ID_MENUITEM_MAIN_FIND },
	{  8, -1,             0, 0 },
	{  9, IDM_NEWFOLDER,  0, MSG_ID_MENUITEM_MAIN_NEWFOLDER },
	{ 10, -1,             0, 0 },
	{ 11, IDM_DELETEITEM, 0, MSG_ID_MENUITEM_MAIN_DELETE },
	{ 12, IDM_RENAME,     0, MSG_ID_MENUITEM_MAIN_RENAME },
	{ 13, -1,             0, 0 },
	{ 14, IDM_TRACELINK,  0, MSG_ID_MENUITEM_CTX_TRACELINK},
	{ 15, -1,             0, 0 },
	{ 16, IDM_EXTAPP1,    0, MSG_ID_MENUITEM_EXTAPP1 },
};

#ifdef COMMENT
HMENU HPCPlatform::LoadContextMenu(DWORD nFlg)
{
	HMENU hMenu = CreatePopupMenu();
	OverrideMenuTitle(hMenu, aContextMenu, sizeof(aContextMenu)/sizeof(MenuMsgRes));
	return hMenu;
}
#endif
#endif // PLATFORM_HPC
