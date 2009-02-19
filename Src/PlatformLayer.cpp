#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#if defined(PLATFORM_BE500)
#include <CSO.h>
#endif

#include "Tombo.h"
#include "Message.h"
#include "resource.h"

#include "PlatformLayer.h"
#include "Property.h"

///////////////////////////////////////////////////
// ctor & dtor
///////////////////////////////////////////////////

PlatformLayer::PlatformLayer() : hMainWnd(NULL)
{
}

PlatformLayer::~PlatformLayer()
{
}

BOOL PlatformLayer::Init(HWND h)
{
	hMainWnd = h;
	return TRUE;
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
// helper functions
///////////////////////////////////////////////////
///////////////////////////////////////////////////

void SetAppIcon(HINSTANCE hInst, HWND hWnd)
{
#if defined(PLATFORM_WIN32)
	// set app icon
	HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TOMBO));
	SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_BIG, (LPARAM)hIcon);
#endif
#if defined(PLATFORM_HPC)
	HICON hIcon =  (HICON)LoadImage(hInst,
                                MAKEINTRESOURCE(IDI_TOMBO),
                                IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	SendMessage(hWnd, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hIcon);
#endif
}

///////////////////////////////////////////////////
// menu helper
///////////////////////////////////////////////////

void AddMenuItemByMsgRes(HMENU hMenu, MenuMsgRes *pRes)
{
	int i = 0;
	while(1) {
		if (pRes[i].iPos == -1) break;

		if (pRes[i].iMenuID == -1) {
			InsertMenu(hMenu,  pRes[i].iPos, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
		} else if (pRes[i].pSubMenu != NULL) {
			HMENU hSubMenu = CreatePopupMenu();
			AddMenuItemByMsgRes(hSubMenu, pRes[i].pSubMenu);

			InsertMenu(hMenu, pRes[i].iPos, MF_BYPOSITION | MF_STRING | MF_POPUP | pRes[i].iExtOpt, (UINT)hSubMenu, g_mMsgRes.GetMsg(pRes[i].iMsgID));
		} else {
			InsertMenu(hMenu,  pRes[i].iPos, MF_BYPOSITION | MF_STRING | pRes[i].iExtOpt, pRes[i].iMenuID, g_mMsgRes.GetMsg(pRes[i].iMsgID));
		}

		i++;
	}
}

void OverrideMenuTitle(HMENU hMenu, MenuMsgRes *pRes, int nNumRes)
{
	for (int i = 0; i < nNumRes; i++) {
		if (pRes[i].iMenuID == -1) {
			InsertMenu(hMenu,  pRes[i].iPos, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
		} else {
			InsertMenu(hMenu,  pRes[i].iPos, MF_BYPOSITION | MF_STRING | pRes[i].iExtOpt, pRes[i].iMenuID, g_mMsgRes.GetMsg(pRes[i].iMsgID));
		}
	}
}

///////////////////////////////////////////////////
// 
///////////////////////////////////////////////////
#if defined(PLATFORM_HPC) || defined(PLATFORM_WIN32) || defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)

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
};

static MenuMsgRes aDirectoryContextMenu[] = {
	{  4, IDM_ASSOC,      0, MSG_ID_MENUITEM_EXPLORER },
	{  5, -1,             0, 0 },
};

HMENU PlatformLayer::LoadContextMenu(DWORD nFlg)
{
	HMENU hMenu = CreatePopupMenu();
	OverrideMenuTitle(hMenu, aContextMenu, sizeof(aContextMenu)/sizeof(MenuMsgRes));
	if (nFlg & CTXMENU_DIR) {
		OverrideMenuTitle(hMenu, aDirectoryContextMenu, sizeof(aDirectoryContextMenu)/sizeof(MenuMsgRes));
	} else if ((nFlg & CTXMENU_FILE) && (nFlg & CTXMENU_ENABLEEXTAPP)) {
		DWORD nPos = 4;
		if (nFlg & CTXMENU_USEASSOC) {
			MenuMsgRes aAssocMenu[] = {
				{  nPos, IDM_ASSOC,      0, MSG_ID_MENUITEM_ASSOCIATION },
			};
			OverrideMenuTitle(hMenu, aAssocMenu, sizeof(aAssocMenu)/sizeof(MenuMsgRes));
			nPos++;
		}
		if (g_Property.GetExtApp1() && _tcslen(g_Property.GetExtApp1()) > 0) {
			MenuMsgRes aExtMenu1[] = {
				{  nPos, IDM_EXTAPP1,    0, MSG_ID_DLG_EXTAPP_LBL_APP1 },
			};
			OverrideMenuTitle(hMenu, aExtMenu1, sizeof(aExtMenu1)/sizeof(MenuMsgRes));
			nPos++;
		}
		if (g_Property.GetExtApp2() && _tcslen(g_Property.GetExtApp2()) > 0) {
			MenuMsgRes aExtMenu2[] = {
				{  nPos, IDM_EXTAPP2,    0, MSG_ID_DLG_EXTAPP_LBL_APP2 },
			};
			OverrideMenuTitle(hMenu, aExtMenu2, sizeof(aExtMenu2)/sizeof(MenuMsgRes));
			nPos++;
		}
		if (nPos > 4) {
			MenuMsgRes aSep[] = {
				{  nPos, -1,    0, 0 },
			};
			OverrideMenuTitle(hMenu, aSep, sizeof(aSep)/sizeof(MenuMsgRes));
		}
	}
	return hMenu;
}

#endif
