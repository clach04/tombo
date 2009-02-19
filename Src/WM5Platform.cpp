#if defined(PLATFORM_WM5)

#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <aygshell.h>

#include "Tombo.h"
#include "resource.h"
#include "Message.h"
#include "PlatformLayer.h"
#include "PocketPCPlatform.h"
#include "SipControl.h"
#include "Property.h"

#define MENUBAR_MENU_LEFT 0
#define MENUBAR_MENU_RIGHT 1

///////////////////////////////////////////////////////
// Menu definitions
///////////////////////////////////////////////////////
// To retrieve submenu depends on the item's position.
// Be careful if order changes.

// Treeview

static MenuMsgRes aMSLeftMenu[] = {
	{  0, IDM_NEWMEMO,         0, MSG_ID_TOOLTIPS_NEWMEMO,  NULL },
	{  1, -1,                  0, 0,                        NULL },
	{  2, IDM_BOOKMARK_ADD,    0, MSG_ID_MENUITEM_B_ADDBM,	NULL },
	{  3, IDM_BOOKMARK_CONFIG, 0, MSG_ID_MENUITEM_B_EDITBM, NULL },
	{  4, -1,                  0, 0,                        NULL },
	{ -1, 0,                   0, 0,                        NULL },
};

static MenuMsgRes aMSRightEditSubMenu[] = {
	{  0, IDM_CUT,      0, MSG_ID_MENUITEM_MAIN_CUT,    NULL },
	{  1, IDM_COPY,     0, MSG_ID_MENUITEM_MAIN_COPY,   NULL },
	{  2, IDM_PASTE,    0, MSG_ID_MENUITEM_MAIN_PASTE,  NULL },
	{ -1, 0,            0, 0,                           NULL },
};

static MenuMsgRes aMSRightFindSubMenu[] = {
	{  0, IDM_SEARCH,      0,         MSG_ID_MENUITEM_MAIN_FIND,         NULL },
	{  1, IDM_SEARCH_NEXT, MF_GRAYED, MSG_ID_MENUITEM_MAIN_FIND_NEXT,    NULL },
	{  2, IDM_SEARCH_PREV, MF_GRAYED, MSG_ID_MENUITEM_MAIN_FIND_PREV,    NULL },
	{  3, -1,              0,         0,                                 NULL },
	{  4, IDM_GREP,        0,         MSG_ID_MENUITEM_MAIN_QUICKFILTER,  NULL },
	{ -1, 0,               0,         0,                                 NULL },
};

static MenuMsgRes aMSRightSecuritySubMenu[] = {
	{  0, IDM_ENCRYPT,    0, MSG_ID_MENUITEM_MAIN_ENCRYPT,      NULL },
	{  1, IDM_DECRYPT,    0, MSG_ID_MENUITEM_MAIN_DECRYPT,      NULL },
	{  2, -1,             0, 0,                                 NULL },
	{  3, IDM_FORGETPASS, 0, MSG_ID_MENUITEM_TOOL_FORGETPASS,   NULL },
	{ -1, 0,              0, 0,                                 NULL },
};

static MenuMsgRes aMSRightFileSubMenu[] = {
	{  0, IDM_RENAME,     0, MSG_ID_MENUITEM_MAIN_RENAME,    NULL },
	{  1, IDM_NEWFOLDER,  0, MSG_ID_MENUITEM_MAIN_NEWFOLDER, NULL },
	{  2, -1,             0, 0,                              NULL },
	{  3, IDM_DELETEITEM, 0, MSG_ID_MENUITEM_MAIN_DELETE,    NULL },
	{ -1, 0,              0, 0,                              NULL },
};

static MenuMsgRes aMSRightPropertySubMenu[] = {
	{  0, IDM_VFOLDER_DEF, 0, MSG_ID_MENUITEM_TOOL_VFOLDER_DEF,  NULL },
	{  1, IDM_PROPERTY,    0, MSG_ID_MENUITEM_TOOL_PROPERTY,     NULL },
	{ -1, 0,              0, 0,                              NULL },
};

#define MS_R_EDIT_SUBMENU_POS 0
#define MS_R_FIND_SUBMENU_POS 1
#define MS_R_SECURITY_SUBMENU_POS 2
#define MS_R_FILE_SUBMENU_POS 3
#define MS_R_PROPERTY_SUBMENU_POS 4

static MenuMsgRes aMSRightMenu[] = {
	{  0,  0,             0, MSG_ID_EDIT,                  aMSRightEditSubMenu },
	{  1,  0,             0, MSG_ID_MENUITEM_MAIN_FIND,    aMSRightFindSubMenu },
	{  2,  0,             0, MSG_ID_PROPTAB_PASS_TIMEOUT,  aMSRightSecuritySubMenu },
	{  3,  0,             0, MSG_ID_MENUITEM_FILE,         aMSRightFileSubMenu },
	{  4,  0,             0, MSG_ID_PROPTAB_TITLE,         aMSRightPropertySubMenu },
	{  5, -1,             0, 0,                            NULL },
	{  6, IDM_TOGGLEPANE, 0, MSG_ID_MENUITEM_BE_T_TWOPANE, NULL }, 
	{  7, -1,             0, 0,                            NULL },
	{  8, IDM_ABOUT,      0, MSG_ID_MENUITEM_TOOL_ABOUT,   NULL },
	{  9, IDM_EXIT,       0, MSG_ID_MENUITEM_TOOL_EXIT,    NULL },
	{ -1, 0,              0, 0,                            NULL },
};

// Editview

static MenuMsgRes aMDRightEditSubMenu[] = {
	{  0, IDM_CUT,      0, MSG_ID_MENUITEM_MAIN_CUT,   NULL },
	{  1, IDM_COPY,     0, MSG_ID_MENUITEM_MAIN_COPY,  NULL },
	{  2, IDM_PASTE,    0, MSG_ID_MENUITEM_MAIN_PASTE, NULL },
	{  3, -1,           0, 0,                          NULL },
	{  4, IDM_INSDATE1, 0, MSG_ID_TOOLTIPS_INSDATE1,   NULL },
	{  5, IDM_INSDATE2, 0, MSG_ID_TOOLTIPS_INSDATE2,   NULL },
	{ -1, 0,            0, 0,                          NULL },
};

static MenuMsgRes aMDRightFindSubMenu[] = {
	{  0, IDM_SEARCH,      0,         MSG_ID_MENUITEM_MAIN_FIND,         NULL },
	{  1, IDM_SEARCH_NEXT, MF_GRAYED, MSG_ID_MENUITEM_MAIN_FIND_NEXT,    NULL },
	{  2, IDM_SEARCH_PREV, MF_GRAYED, MSG_ID_MENUITEM_MAIN_FIND_PREV,    NULL },
	{ -1, 0,               0,         0,                                 NULL },
};

#define MD_R_EDIT_SUBMENU_POS 0
#define MD_R_FIND_SUBMENU_POS 1

static MenuMsgRes aMDRightMenu[] = {
	{  0,  0,                  0,          MSG_ID_EDIT,               aMDRightEditSubMenu },
	{  1,  0,                  0,          MSG_ID_MENUITEM_MAIN_FIND, aMDRightFindSubMenu },
	{  2, -1,                  0,          0,                         NULL },
	{  3, IDM_DETAILS_HSCROLL, MF_CHECKED, MSG_ID_MENUITEM_TOOL_WRAP, NULL },
	{  4, -1,                  0,          0,                         NULL },
	{  5, IDM_EXIT,            0,          MSG_ID_MENUITEM_TOOL_EXIT, NULL },
	{ -1, 0,                   0,          0,                         NULL },
};

///////////////////////////////////////////////////////
// util
///////////////////////////////////////////////////////

static HMENU GetMenuFromMenubar(HWND hWnd, DWORD nIndex) {
	TBBUTTONINFO tbbi = {0};
	tbbi.cbSize = sizeof(tbbi);
	tbbi.dwMask = TBIF_LPARAM | TBIF_BYINDEX;                
	SendMessage(hWnd, TB_GETBUTTONINFO, nIndex, (LPARAM)&tbbi);
	return (HMENU)tbbi.lParam;
}

static HWND CreateMenubar(HWND hWnd, HINSTANCE hInst, UINT nToolBarId, MenuMsgRes *pLeft, MenuMsgRes *pRight)
{
	SHMENUBARINFO mbi;
	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize = sizeof(SHMENUBARINFO);
	mbi.hwndParent = hWnd;
	mbi.nToolBarId = nToolBarId;
	mbi.hInstRes =hInst;

	if (!SHCreateMenuBar(&mbi)) {
		TCHAR buf[1024];
		wsprintf(buf, TEXT("SHCreateMenuBar failed(%d)."), GetLastError());
		MessageBox(hWnd, buf, TEXT("DEBUG"), MB_OK);
		return NULL;
	}

	if (pLeft != NULL) {
		HMENU hMenu = GetMenuFromMenubar(mbi.hwndMB, MENUBAR_MENU_LEFT);
		RemoveMenu(hMenu, 0, MF_BYPOSITION);
		AddMenuItemByMsgRes(hMenu, pLeft);
	}
	if (pRight != NULL) {
		HMENU hMenu = GetMenuFromMenubar(mbi.hwndMB, MENUBAR_MENU_RIGHT);
		RemoveMenu(hMenu, 0, MF_BYPOSITION);
		AddMenuItemByMsgRes(hMenu, pRight);
	}

	return mbi.hwndMB;
}

static HMENU GetSubMenu(HMENU hMenu, DWORD nIndex)
{
	MENUITEMINFO mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_SUBMENU;
	if (!GetMenuItemInfo(hMenu, nIndex, TRUE, &mi)) {
		return NULL;
	}
	return mi.hSubMenu;
}

///////////////////////////////////////////////////////
// methods
///////////////////////////////////////////////////////

void WM5Platform::Create(HWND hWnd, HINSTANCE hInst)
{
	hMainWnd = hWnd;

	// create menubar
	hMSCmdBar = CreateMenubar(hWnd, hInst, IDM_MAIN_MENU, aMSLeftMenu, aMSRightMenu);
	hMDCmdBar = CreateMenubar(hWnd, hInst, IDM_DETAILS_MENU, NULL, aMDRightMenu);

	TBBUTTONINFO tbi;
	ZeroMemory(&tbi, sizeof(tbi));
	tbi.cbSize = sizeof(tbi);
	tbi.dwMask = TBIF_BYINDEX | TBIF_TEXT;

	tbi.pszText = (LPTSTR)MSG_MEMO;
	tbi.cchText = _tcslen(MSG_MEMO);
	SendMessage(hMSCmdBar, TB_SETBUTTONINFO, 0, (LPARAM)&tbi);
	tbi.pszText = (LPTSTR)MSG_TOOL;
	tbi.cchText = _tcslen(MSG_TOOL);
	SendMessage(hMSCmdBar, TB_SETBUTTONINFO, 1, (LPARAM)&tbi);
	SendMessage(hMDCmdBar, TB_SETBUTTONINFO, 1, (LPARAM)&tbi);

	ShowWindow(hMDCmdBar, SW_HIDE);
}

HMENU WM5Platform::GetMDToolMenu() { return SHGetSubMenu(hMDCmdBar, IDM_DETAILS_TOOL); }
HMENU WM5Platform::GetMSBookMarkMenu()
{
	return GetMenuFromMenubar(hMSCmdBar, MENUBAR_MENU_LEFT);
}

void WM5Platform::EnableMenu(UINT uid, BOOL bEnable)
{
	BOOL bSBS = FALSE;
	BOOL bMenu = TRUE;
	HWND hTB;

	HMENU hMenu = NULL;
	switch (uid) {
	case IDM_CUT:
	case IDM_COPY:
	case IDM_PASTE:
		hMenu = GetSubMenu(GetMenuFromMenubar(hMSCmdBar, MENUBAR_MENU_RIGHT), MS_R_EDIT_SUBMENU_POS);
		break;
	case IDM_GREP:
		hMenu = GetSubMenu(GetMenuFromMenubar(hMSCmdBar, MENUBAR_MENU_RIGHT), MS_R_FIND_SUBMENU_POS);
		break;
	case IDM_DECRYPT:
	case IDM_ENCRYPT:
		hMenu = GetSubMenu(GetMenuFromMenubar(hMSCmdBar, MENUBAR_MENU_RIGHT), MS_R_SECURITY_SUBMENU_POS);
		break;
	case IDM_DELETEITEM:
	case IDM_RENAME:
	case IDM_NEWFOLDER:
		hMenu = GetSubMenu(GetMenuFromMenubar(hMSCmdBar, MENUBAR_MENU_RIGHT), MS_R_FILE_SUBMENU_POS);
		break;
	case IDM_NEWMEMO:
		hMenu = GetMenuFromMenubar(hMSCmdBar, MENUBAR_MENU_LEFT);
		break;
	case IDM_SAVE:
		bSBS = TRUE;
		hTB = hMDCmdBar;
		break;
	default:
		return;
	}

	if (hMenu != NULL) {
		if (bEnable) {
			EnableMenuItem(hMenu, uid, MF_BYCOMMAND | MF_ENABLED);
		} else {
			EnableMenuItem(hMenu, uid, MF_BYCOMMAND | MF_GRAYED);
		}
	}

	if (bSBS) {
		SendMessage(hTB, TB_ENABLEBUTTON, uid, MAKELONG(bEnable, 0));
	}
}

void WM5Platform::CheckMenu(UINT uid, BOOL bCheck)
{
	HMENU hMenu = NULL;
	switch (uid) {
	case IDM_TOGGLEPANE:
		hMenu = GetMenuFromMenubar(hMSCmdBar, MENUBAR_MENU_RIGHT);
		break;
	case IDM_DETAILS_HSCROLL:
		hMenu = GetMenuFromMenubar(hMDCmdBar, MENUBAR_MENU_RIGHT);
		break;
	default:
		return;
	}

	if (hMenu) {
		CheckMenuItem(hMenu, uid, MF_BYCOMMAND | (bCheck ? MF_CHECKED : MF_UNCHECKED));
	}
}

void WM5Platform::OpenDetailsView()
{
	ShowWindow(hMDCmdBar, SW_SHOW);
	ShowWindow(hMSCmdBar, SW_HIDE);

	SHDoneButton(hMainWnd, SHDB_SHOW);
}

void WM5Platform::CloseDetailsView()
{
	ShowWindow(hMDCmdBar, SW_HIDE);
	ShowWindow(hMSCmdBar, SW_SHOW);

	SHDoneButton(hMainWnd, SHDB_HIDE);
}

void WM5Platform::EnableSearchNext()
{
	HMENU hMSMenu = GetSubMenu(GetMenuFromMenubar(hMSCmdBar, MENUBAR_MENU_RIGHT), MS_R_FIND_SUBMENU_POS);
	EnableMenuItem(hMSMenu, IDM_SEARCH_PREV, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMSMenu, IDM_SEARCH_NEXT, MF_BYCOMMAND | MF_ENABLED);

	HMENU hMDMenu = GetSubMenu(GetMenuFromMenubar(hMDCmdBar, MENUBAR_MENU_RIGHT), MD_R_FIND_SUBMENU_POS);
	EnableMenuItem(hMDMenu, IDM_SEARCH_PREV, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(hMDMenu, IDM_SEARCH_NEXT, MF_BYCOMMAND | MF_ENABLED);
}

void WM5Platform::AdjustUserRect(RECT *r)
{
	// menubar
	RECT rMenuRect;
	GetWindowRect(hMSCmdBar, &rMenuRect);
	DWORD nHOffset = rMenuRect.bottom - rMenuRect.top;

	r->bottom -= nHOffset - 1;

	// SIP
	BOOL bStat;
	SipControl sc;
	if (!sc.Init()) return;
	if (!sc.GetSipStat(&bStat)) return;

	if (bStat) {
		RECT rSip = sc.GetRect();
		DWORD nDelta = g_Property.GetSipSizeDelta();
	
		r->bottom -= (rSip.bottom - rSip.top + nDelta);
	}
}

///////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////
static MenuMsgRes aMSPopupMenu[] = {
	{  0, IDM_CUT,        0, MSG_ID_MENUITEM_MAIN_CUT },
	{  1, IDM_COPY,       0, MSG_ID_MENUITEM_MAIN_COPY },
	{  2, IDM_PASTE,      0, MSG_ID_MENUITEM_MAIN_PASTE },
	{  3, -1, 0, 0},
	{  4, IDM_DELETEITEM, 0, MSG_ID_MENUITEM_MAIN_DELETE },
	{  5, IDM_RENAME,     0, MSG_ID_MENUITEM_MAIN_RENAME },
	{  6, -1, 0, 0},
	{  7, IDM_ENCRYPT,    0, MSG_ID_MENUITEM_MAIN_ENCRYPT },
	{  8, IDM_DECRYPT,    0, MSG_ID_MENUITEM_MAIN_DECRYPT },

};

HMENU WM5Platform::LoadSelectViewPopupMenu()
{
	HMENU hMenu = CreatePopupMenu();
	OverrideMenuTitle(hMenu, aMSPopupMenu, sizeof(aMSPopupMenu) / sizeof(MenuMsgRes));
	return hMenu;
}

static MenuMsgRes aMDPopupMenu[] = {
	{ 0, IDM_CUT,      0, MSG_ID_MENUITEM_MAIN_CUT },
	{ 1, IDM_COPY,     0, MSG_ID_MENUITEM_MAIN_COPY },
	{ 2, IDM_PASTE,    0, MSG_ID_MENUITEM_MAIN_PASTE },
	{ 3, -1, 0, 0},
	{ 4, IDM_INSDATE1, 0, MSG_ID_TOOLTIPS_INSDATE1 },
	{ 5, IDM_INSDATE2, 0, MSG_ID_TOOLTIPS_INSDATE2 },
};

HMENU WM5Platform::LoadDetailsViewPopupMenu()
{
	HMENU hMenu = CreatePopupMenu();
	OverrideMenuTitle(hMenu, aMDPopupMenu, sizeof(aMDPopupMenu) / sizeof(MenuMsgRes));
	return hMenu;
}

#endif // PLATFORM_WM5