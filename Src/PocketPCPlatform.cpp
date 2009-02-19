#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)

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

///////////////////////////////////////////////////////
// Menu label definitions
///////////////////////////////////////////////////////

static MenuMsgRes aMSMemoMenu[] = {
	{  0, IDM_NEWFOLDER,   0, MSG_ID_MENUITEM_MAIN_NEWFOLDER },
	{  1, IDM_CUT,         0, MSG_ID_MENUITEM_MAIN_CUT },
	{  2, IDM_COPY,        0, MSG_ID_MENUITEM_MAIN_COPY },
	{  3, IDM_PASTE,       0, MSG_ID_MENUITEM_MAIN_PASTE },
	{  4, -1,              0, 0 },
	{  5, IDM_RENAME,      0, MSG_ID_MENUITEM_MAIN_RENAME },
	{  6, IDM_DELETEITEM,  0, MSG_ID_MENUITEM_MAIN_DELETE },
	{  7, -1,              0, 0 },
	{  8, IDM_SEARCH,      0, MSG_ID_MENUITEM_MAIN_FIND },
	{  9, IDM_SEARCH_NEXT, 0, MSG_ID_MENUITEM_MAIN_FIND_NEXT },
	{ 10, IDM_SEARCH_PREV, 0, MSG_ID_MENUITEM_MAIN_FIND_PREV },
	{ 11, -1,              0, 0 },
	{ 12, IDM_GREP,        0, MSG_ID_MENUITEM_MAIN_QUICKFILTER },
	// 13 is used by default separator
	{ 14, IDM_ENCRYPT,     0, MSG_ID_MENUITEM_MAIN_ENCRYPT },
	{ 15, IDM_DECRYPT,     0, MSG_ID_MENUITEM_MAIN_DECRYPT },
};

static MenuMsgRes aMSToolMenu[] = {
	{  0, IDM_EXIT,        0, MSG_ID_MENUITEM_TOOL_EXIT },
	// 1 is not defined
	{  2, IDM_ABOUT,       0, MSG_ID_MENUITEM_TOOL_ABOUT },
	{  3, IDM_PROPERTY,    0, MSG_ID_MENUITEM_TOOL_PROPERTY},
	{  4, IDM_VFOLDER_DEF, 0, MSG_ID_MENUITEM_TOOL_VFOLDER_DEF},
	{  5, IDM_FORGETPASS,  0, MSG_ID_MENUITEM_TOOL_FORGETPASS},
};

static MenuMsgRes aMSBookmarkMenu[] = {
	{  0, IDM_BOOKMARK_ADD,    0, MSG_ID_MENUITEM_B_ADDBM },
	{  1, IDM_BOOKMARK_CONFIG, 0, MSG_ID_MENUITEM_B_EDITBM },
	// 2 is not defined
};


static MenuMsgRes aMDEditMenu[] = {
	{  0, IDM_CUT,         0, MSG_ID_MENUITEM_MAIN_CUT },
	{  1, IDM_COPY,        0, MSG_ID_MENUITEM_MAIN_COPY },
	{  2, IDM_PASTE,       0, MSG_ID_MENUITEM_MAIN_PASTE },
	{  3, -1,              0, 0 },
	{  4, IDM_SELALL,      0, MSG_ID_MENUITEM_DETAILS_SELALL },
	// 5 is not defined
	{  6, IDM_SEARCH,      0, MSG_ID_MENUITEM_MAIN_FIND },
	{  7, IDM_SEARCH_NEXT, 0, MSG_ID_MENUITEM_MAIN_FIND_NEXT },
	{  8, IDM_SEARCH_PREV, 0, MSG_ID_MENUITEM_MAIN_FIND_PREV },
};

static MenuMsgRes aMDToolMenu[] = {
	{  0, IDM_DETAILS_HSCROLL, MF_CHECKED, MSG_ID_MENUITEM_TOOL_WRAP },
	// 1 is not defined
	{  2, IDM_ABOUT,           0,          MSG_ID_MENUITEM_TOOL_ABOUT },
	{  3, IDM_EXIT,            0,          MSG_ID_MENUITEM_TOOL_EXIT },
};

///////////////////////////////////////////////////////
// Toolbar definitions
///////////////////////////////////////////////////////

#define NUM_TOOLBAR_BMP 12

#define NUM_MS_TOOLTIP 1
LPTSTR pMSToolTip[] = {
	NULL, // MSG_TOOLTIPS_NEWMEMO,
};

#define NUM_MD_TOOLTIP 6
LPTSTR pMDToolTip[] = {
	NULL, // MSG_TOOLTIPS_RETURNLIST,
	NULL, //MSG_TOOLTIPS_SAVE,
	TEXT(""),
	TEXT(""),
	NULL, // MSG_TOOLTIPS_INSDATE1,
	NULL, // MSG_TOOLTIPS_INSDATE2,
};

///////////////////////////////////////////////////////
// methods
///////////////////////////////////////////////////////

void PocketPCPlatform::Create(HWND hWnd, HINSTANCE hInst)
{
	// menubar for Tree view
	SHMENUBARINFO mbi;
	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize = sizeof(SHMENUBARINFO);
	mbi.hwndParent = hWnd;
	mbi.nToolBarId = IDM_MAIN_MENU;
	mbi.hInstRes =hInst;
#if defined(FOR_VGA)
	mbi.nBmpId = IDB_TOOLBAR_LARGE;
#else
	mbi.nBmpId = IDB_TOOLBAR;
#endif
	mbi.cBmpImages = NUM_TOOLBAR_BMP;
	if (!SHCreateMenuBar(&mbi)) {
		MessageBox(hWnd, TEXT("SHCreateMenuBar failed."), TEXT("DEBUG"), MB_OK);
	}
	hMSCmdBar = mbi.hwndMB;

	TBBUTTONINFO tbi;
	memset(&tbi, 0, sizeof(&tbi));
	tbi.cbSize = sizeof(tbi);
	tbi.dwMask = TBIF_TEXT;

	tbi.pszText = (LPTSTR)MSG_MENU_NOTE;
	tbi.cchText = _tcslen(MSG_MENU_NOTE);
	SendMessage(hMSCmdBar, TB_SETBUTTONINFO, (WPARAM)(INT)IDM_EDIT_MEMO, (LPARAM)&tbi);

	tbi.pszText = (LPTSTR)MSG_MENU_TOOL;
	tbi.cchText = _tcslen(MSG_MENU_TOOL);
	SendMessage(hMSCmdBar, TB_SETBUTTONINFO, (WPARAM)(INT)IDM_MENUITEM3, (LPARAM)&tbi);

	pMSToolTip[0] = (LPTSTR)MSG_TOOLTIPS_NEWMEMO;
	SendMessage(hMSCmdBar, TB_SETTOOLTIPS, (WPARAM)NUM_MS_TOOLTIP, (LPARAM)pMSToolTip);

	//////////////////////////
	// create menu items

	// hMenu is not created when default menu item is empty, leave only one separator.
	OverrideMenuTitle(SHGetSubMenu(hMSCmdBar, IDM_EDIT_MEMO), aMSMemoMenu, sizeof(aMSMemoMenu) / sizeof(MenuMsgRes));
	OverrideMenuTitle(SHGetSubMenu(hMSCmdBar, IDM_MENUITEM3), aMSToolMenu, sizeof(aMSToolMenu) / sizeof(MenuMsgRes));
	OverrideMenuTitle(SHGetSubMenu(hMSCmdBar, IDM_MS_BOOKMARK), aMSBookmarkMenu, sizeof(aMSBookmarkMenu) / sizeof(MenuMsgRes));

	////////////////////--

	// menubar for edit view
	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize = sizeof(SHMENUBARINFO);
	mbi.hwndParent = hWnd;
	mbi.nToolBarId = IDM_DETAILS_MENU;
	mbi.hInstRes =hInst;
#if defined(FOR_VGA)
	mbi.nBmpId = IDB_TOOLBAR_LARGE;
#else
	mbi.nBmpId = IDB_TOOLBAR;
#endif
	mbi.cBmpImages = NUM_TOOLBAR_BMP;
	if (!SHCreateMenuBar(&mbi)) {
		MessageBox(hWnd, TEXT("SHCreateMenuBar failed."), TEXT("DEBUG"), MB_OK);
	}
	hMDCmdBar = mbi.hwndMB;

	pMDToolTip[0] = (LPTSTR)MSG_TOOLTIPS_RETURNLIST;
	pMDToolTip[1] = (LPTSTR)MSG_TOOLTIPS_SAVE;
	pMDToolTip[4] = (LPTSTR)MSG_TOOLTIPS_INSDATE1;
	pMDToolTip[5] = (LPTSTR)MSG_TOOLTIPS_INSDATE2;
	SendMessage(hMDCmdBar, TB_SETTOOLTIPS, (WPARAM)NUM_MD_TOOLTIP, (LPARAM)pMDToolTip);

	OverrideMenuTitle(SHGetSubMenu(hMDCmdBar, IDM_EDIT_MEMO), aMDEditMenu, sizeof(aMDEditMenu) / sizeof(MenuMsgRes));
	OverrideMenuTitle(SHGetSubMenu(hMDCmdBar, IDM_DETAILS_TOOL), aMDToolMenu, sizeof(aMDToolMenu) / sizeof(MenuMsgRes));

	ShowWindow(hMDCmdBar, SW_HIDE);
}

HMENU PocketPCPlatform::GetMDToolMenu() { return SHGetSubMenu(hMDCmdBar, IDM_DETAILS_TOOL); }
HMENU PocketPCPlatform::GetMSEditMenu() { return SHGetSubMenu(hMSCmdBar, IDM_EDIT_MEMO); }
HMENU PocketPCPlatform::GetMSBookMarkMenu() { return SHGetSubMenu(hMSCmdBar, IDM_MS_BOOKMARK); }

void PocketPCPlatform::EnableMenu(UINT uid, BOOL bEnable)
{
	BOOL bSBS = FALSE;
	BOOL bMenu = TRUE;
	HWND hTB;

	HMENU hMenu;
	switch (uid) {
	case IDM_DECRYPT:
	case IDM_ENCRYPT:
	case IDM_DELETEITEM:
	case IDM_RENAME:
	case IDM_CUT:
	case IDM_COPY:
	case IDM_PASTE:
	case IDM_NEWFOLDER:
	case IDM_GREP:
		hMenu = GetMSEditMenu();
		break;
	case IDM_NEWMEMO:
		bSBS = TRUE;
		hTB = hMSCmdBar;
		hMenu = GetMSEditMenu();
		break;
	case IDM_SAVE:
		bMenu = FALSE;
		bSBS = TRUE;
		hTB = hMDCmdBar;
		break;
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

	if (bSBS) {
		SendMessage(hTB, TB_ENABLEBUTTON, uid, MAKELONG(bEnable, 0));
	}
}

void PocketPCPlatform::CheckMenu(UINT uid, BOOL bCheck)
{
	HWND hTB;
	HMENU hMenu = NULL;
	BOOL bCBS = FALSE;
	switch(uid) {
	case IDM_TOGGLEPANE:
		bCBS = TRUE;
		hTB = hMSCmdBar;
		bCheck = !bCheck;
		break;
	case IDM_DETAILS_HSCROLL:
		bCBS = FALSE;
		hMenu = GetMDToolMenu();
		break;
	default:
		return;
	}

	if (bCBS) {
		SendMessage(hTB, TB_CHECKBUTTON, uid, MAKELONG(bCheck, 0));
	}
	if (hMenu) {
		CheckMenuItem(hMenu, uid, MF_BYCOMMAND | (bCheck ? MF_CHECKED : MF_UNCHECKED));
	}
}

void PocketPCPlatform::OpenDetailsView()
{
	ShowWindow(hMDCmdBar, SW_SHOW);
	ShowWindow(hMSCmdBar, SW_HIDE);

	TBBUTTONINFO tbi;
	memset(&tbi, 0, sizeof(&tbi));
	tbi.cbSize = sizeof(tbi);
	tbi.dwMask = TBIF_TEXT;

	tbi.pszText = (LPTSTR)MSG_MENU_TOOL;
	tbi.cchText = _tcslen(MSG_MENU_TOOL);
	SendMessage(hMDCmdBar, TB_SETBUTTONINFO, (WPARAM)(INT)IDM_DETAILS_TOOL, (LPARAM)&tbi);

	tbi.pszText = (LPTSTR)MSG_MENU_EDIT;
	tbi.cchText = _tcslen(MSG_MENU_EDIT);
	SendMessage(hMDCmdBar, TB_SETBUTTONINFO, (WPARAM)(INT)IDM_EDIT_MEMO, (LPARAM)&tbi);

}

void PocketPCPlatform::CloseDetailsView()
{
	ShowWindow(hMDCmdBar, SW_HIDE);
	ShowWindow(hMSCmdBar, SW_SHOW);
}

void PocketPCPlatform::EnableSearchNext()
{
	SendMessage(hMSCmdBar, TB_SETSTATE, IDM_SEARCH_PREV, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(hMSCmdBar, TB_SETSTATE, IDM_SEARCH_NEXT, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(hMDCmdBar, TB_SETSTATE, IDM_SEARCH_PREV, MAKELONG(TBSTATE_ENABLED, 0)); 
	SendMessage(hMDCmdBar, TB_SETSTATE, IDM_SEARCH_NEXT, MAKELONG(TBSTATE_ENABLED, 0)); 
}

void PocketPCPlatform::AdjustUserRect(RECT *r)
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

HMENU PocketPCPlatform::LoadSelectViewPopupMenu()
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

HMENU PocketPCPlatform::LoadDetailsViewPopupMenu()
{
	HMENU hMenu = CreatePopupMenu();
	OverrideMenuTitle(hMenu, aMDPopupMenu, sizeof(aMDPopupMenu) / sizeof(MenuMsgRes));
	return hMenu;
}

#endif // PLATFORM_PKTPC