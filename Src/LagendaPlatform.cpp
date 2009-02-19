#if defined(PLATFORM_BE500)
#include <windows.h>
#include <commctrl.h>
#include <CSO.h>

#include "resource.h"
#include "Message.h"
#include "PlatformLayer.h"
#include "LagendaPlatform.h"
#include "SipControl.h"

#define NUM_IMG_BUTTONS 0

#define NUM_SV_CMDBAR_BUTTONS 10
CSOBAR_BUTTONINFO	aSVCSOBarButtons[NUM_SV_CMDBAR_BUTTONS] = 
{
	IDM_SV_MENU_1,  CSOBAR_BUTTON_SUBMENU_DOWN,  CSO_BUTTON_DISP, (-1),         NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_SV_MENU_2,  CSOBAR_BUTTON_SUBMENU_DOWN,  CSO_BUTTON_DISP, IDB_TOOL,     NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	0,              CSOBAR_SEP,                  CSO_BUTTON_DISP, (-1),         NULL, NULL,     NULL,   0,                     1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_SV_MENU_3,  CSOBAR_BUTTON_SUBMENU_DOWN,  CSO_BUTTON_DISP, IDB_BOOKMARK, NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	0,              CSOBAR_SEP,                  CSO_BUTTON_DISP, (-1),         NULL, NULL,     NULL,   0,                     1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_NEWMEMO,    CSOBAR_BUTTON_NORM,          CSO_BUTTON_DISP, IDB_NEWMEMO,  NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	0,              CSOBAR_SEP,                  CSO_BUTTON_DISP, (-1),         NULL, NULL,     NULL,   0,                     1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
//	IDM_DELETEITEM, CSOBAR_BUTTON_NORM,          CSO_BUTTON_DISP, IDB_DELETE,   NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
//	0,              CSOBAR_SEP,                  CSO_BUTTON_DISP, (-1),         NULL, NULL,     NULL,   0,                     1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_SEARCH,     CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_FIND,      NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_SEARCH_PREV,CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_FINDPREV,  NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_SEARCH_NEXT,CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_FINDNEXT,  NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
};

#define NUM_DV_CMDBAR_BUTTONS 12

// Set hInstance on Oncreate if put bitmaps.
CSOBAR_BUTTONINFO	aDVCSOBarButtons[NUM_DV_CMDBAR_BUTTONS] = 
{
	IDM_RETURNLIST, CSOBAR_COMMON_BUTTON,       CSO_BUTTON_DISP, (-1),            NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), CSO_ID_BACK, CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	0,              CSOBAR_SEP,                 CSO_BUTTON_DISP, (-1),            NULL, NULL,     NULL,   0,                     1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_DV_MENU_1,  CSOBAR_BUTTON_SUBMENU_DOWN, CSO_BUTTON_DISP, (-1),            NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	0,              CSOBAR_SEP,                 CSO_BUTTON_DISP, (-1),            NULL, NULL,     NULL,   0,                     1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_SAVE,       CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_SAVE,        NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	0,              CSOBAR_SEP,                 CSO_BUTTON_DISP, (-1),            NULL, NULL,     NULL,   0,                     1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_CUT,        CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_CUT,         NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_COPY,       CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_COPY,        NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_PASTE,      CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_PASTE,       NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	0,              CSOBAR_SEP,                 CSO_BUTTON_DISP, (-1),            NULL, NULL,     NULL,   0,                     1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_INSDATE1,   CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_INSDATE1,    NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
	IDM_INSDATE2,   CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_INSDATE2,    NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,

//	IDM_SEARCH_PREV,CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_FINDPREV,    NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 2, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
//	IDM_SEARCH_NEXT,CSOBAR_BUTTON_NORM,         CSO_BUTTON_DISP, IDB_FINDNEXT,    NULL, NULL,     NULL,   CSOBAR_CODEPOS_CENTER, 2, (-1), (-1), (-1), (-1), 0,           CLR_INVALID, CLR_INVALID, CLR_INVALID, FALSE, FALSE,
};

static HWND MakeCSOBar(HINSTANCE hInst, HWND hWnd, 
						   DWORD nCtlID)
{
	CSOBAR_BASEINFO cb;
	cb.x = cb.y = cb.width = cb.height = -1;
	cb.line = 1;
	cb.backColor = CSOBAR_DEFAULT_BACKCOLOR;
	cb.titleColor = CSOBAR_DEFAULT_CODECOLOR;
	cb.titleText = NULL;
	cb.titleBmpResId = NULL;
	cb.titleBmpResIns = NULL;

	HWND h;
	h = CSOBar_Create(hInst, hWnd, nCtlID, cb);
	CSOBar_AddAdornments(h, hInst, 1, CSOBAR_ADORNMENT_CLOSE, 0);
	return h;
}

void LagendaPlatform::Create(HWND hWnd, HINSTANCE hInst)
{
	// Tree view 
	hMSCmdBar = MakeCSOBar(hInst, hWnd, ID_CMDBAR_MAIN);
//	HMENU hMSMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU_MAIN));
//	hMSMemoMenu     = aSVCSOBarButtons[0].SubMenu = GetSubMenu(hMSMenu, 0);
//	hMSToolMenu     = aSVCSOBarButtons[1].SubMenu = GetSubMenu(hMSMenu, 1);
//	hMSBookMarkMenu = aSVCSOBarButtons[3].SubMenu = GetSubMenu(hMSMenu, 2);

	hMSMemoMenu     = aSVCSOBarButtons[0].SubMenu = LagendaPlatform::LoadMSMemoMenu();
	hMSToolMenu     = aSVCSOBarButtons[1].SubMenu = LagendaPlatform::LoadMSToolMenu();
	hMSBookMarkMenu = aSVCSOBarButtons[3].SubMenu = LagendaPlatform::LoadMSBookmarkMenu();

	aSVCSOBarButtons[0].FaceText = (LPTSTR)MSG_MEMO;

	for (int i = 0; i < NUM_SV_CMDBAR_BUTTONS; i++) {
		aSVCSOBarButtons[i].reshInst = hInst;
	}
	CSOBar_AddButtons(hMSCmdBar, hInst, NUM_SV_CMDBAR_BUTTONS, &aSVCSOBarButtons[0]);

	// Edit view
	aDVCSOBarButtons[2].FaceText = (LPTSTR)MSG_EDIT;

	hMDCmdBar = MakeCSOBar(hInst, hWnd, ID_CMDBAR_DETAILS);
	HMENU hMDMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU_DETAILS));
//	hMDEditMenu = aDVCSOBarButtons[2].SubMenu	= GetSubMenu(hMDMenu, 0);
	hMDEditMenu = aDVCSOBarButtons[2].SubMenu	= LagendaPlatform::LoadMDEditMenu();
	for (i = 0; i < NUM_DV_CMDBAR_BUTTONS; i++) {
		aDVCSOBarButtons[i].reshInst = hInst;
	}
	CSOBar_AddButtons(hMDCmdBar, hInst, NUM_DV_CMDBAR_BUTTONS, &aDVCSOBarButtons[0]);

    CSOBar_Show(hMSCmdBar, TRUE);

}

void LagendaPlatform::EnableMenu(UINT uid, BOOL bEnable)
{
	BOOL bMenu = TRUE; HMENU hMenu;
	BOOL bSBS = FALSE; HWND hSBSWnd;

	switch (uid) {
	case IDM_DECRYPT:
	case IDM_ENCRYPT:
	case IDM_RENAME:
	case IDM_NEWMEMO:
	case IDM_CUT:
	case IDM_COPY:
	case IDM_PASTE:
	case IDM_GREP:
		hMenu = GetMSEditMenu();
		break;
	case IDM_NEWFOLDER:
		hMenu = GetMSToolMenu();
		break;
	case IDM_DELETEITEM:
		hMenu = GetMSEditMenu();
		hSBSWnd = hMSCmdBar;
		bSBS = TRUE;
		break;
	case IDM_SAVE:
		bMenu = FALSE;
		bSBS = TRUE; hSBSWnd = hMDCmdBar;
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
		if (bEnable) {
			CSOBar_SetButtonState(hSBSWnd, TRUE, uid, 1, CSO_BUTTON_DISP);
		} else {
			CSOBar_SetButtonState(hSBSWnd, TRUE, uid, 1, CSO_BUTTON_GRAYED);
		}
	}
}

void LagendaPlatform::CheckMenu(UINT uid, BOOL bCheck)
{
	HMENU hMenu;
	BOOL bCBS = FALSE;
	switch(uid) {
	case IDM_TOGGLEPANE:
		bCBS = TRUE;
		hMenu = hMSToolMenu;
		break;
//	case IDM_DETAILS_HSCROLL:
//		hMenu = hMDToolMenu;
//		bCBS = TRUE;
//		break;
	default:
		return;
	}

	if (bCBS) {
		CheckMenuItem(hMenu, uid, MF_BYCOMMAND | (bCheck ? MF_CHECKED : MF_UNCHECKED));
	}
}

void LagendaPlatform::OpenDetailsView()
{
	CSOBar_Show(hMSCmdBar, SW_HIDE);
	CSOBar_Show(hMDCmdBar, SW_SHOW);
}

void LagendaPlatform::CloseDetailsView()
{
	CSOBar_Show(hMDCmdBar, SW_HIDE);
	CSOBar_Show(hMSCmdBar, SW_SHOW);
}

void LagendaPlatform::AdjustUserRect(RECT *r)
{
	DWORD nHOffset = CSOBar_Height(hMSCmdBar);
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

////////////////////////////////////
// load menu
////////////////////////////////////

static MenuMsgRes aMSMemoMenu[] = {
	{  0, IDM_CUT,         0, MSG_ID_MENUITEM_MAIN_CUT },
	{  1, IDM_COPY,        0, MSG_ID_MENUITEM_MAIN_COPY },
	{  2, IDM_PASTE,       0, MSG_ID_MENUITEM_MAIN_PASTE },
	{  3, -1,              0, 0 },
	{  4, IDM_NEWMEMO,     0, MSG_ID_TOOLTIPS_NEWMEMO },
	{  5, IDM_DELETEITEM,  0, MSG_ID_MENUITEM_MAIN_DELETE },
	{  6, IDM_RENAME,      0, MSG_ID_MENUITEM_MAIN_RENAME },
	{  7, -1,              0, 0 },
	{  8, IDM_ENCRYPT,     0, MSG_ID_MENUITEM_MAIN_ENCRYPT },
	{  9, IDM_DECRYPT,     0, MSG_ID_MENUITEM_MAIN_DECRYPT },
	{ 10, -1,              0, 0 },
	{ 11, IDM_EXTAPP1,     0, MSG_ID_DLG_EXTAPP_LBL_APP1 },
	{ 12, IDM_EXTAPP2,     0, MSG_ID_DLG_EXTAPP_LBL_APP2 },
};

HMENU LagendaPlatform::LoadMSMemoMenu()
{
	HMENU hMenu = CreatePopupMenu();
	OverrideMenuTitle(hMenu, aMSMemoMenu, sizeof(aMSMemoMenu)/sizeof(MenuMsgRes));
	return hMenu;
}

static MenuMsgRes aMSToolMenu[] = {
	{  0, IDM_NEWFOLDER,   0, MSG_ID_MENUITEM_MAIN_NEWFOLDER },
	{  1, -1,              0, 0 },
	{  2, IDM_TOGGLEPANE,  0, MSG_ID_MENUITEM_BE_T_TWOPANE },
	{  3, -1,              0, 0 },
	{  4, IDM_PROPERTY,    0, MSG_ID_MENUITEM_TOOL_PROPERTY },
	{  5, IDM_VFOLDER_DEF, 0, MSG_ID_MENUITEM_TOOL_VFOLDER_DEF },
	{  6, IDM_FORGETPASS,  0, MSG_ID_MENUITEM_TOOL_FORGETPASS },
	{  7, IDM_ABOUT,       0, MSG_ID_MENUITEM_TOOL_ABOUT },
	{  8, -1,              0, 0 },
	{  9, IDM_SEARCH,      0, MSG_ID_MENUITEM_MAIN_FIND },
	{ 10, IDM_SEARCH_NEXT, 0, MSG_ID_MENUITEM_MAIN_FIND_NEXT },
	{ 11, IDM_SEARCH_PREV, 0, MSG_ID_MENUITEM_MAIN_FIND_PREV },
	{ 12, -1,              0, 0 },
	{ 13, IDM_GREP,        0, MSG_ID_MENUITEM_MAIN_QUICKFILTER },
};

HMENU LagendaPlatform::LoadMSToolMenu()
{
	HMENU hMenu = CreatePopupMenu();
	OverrideMenuTitle(hMenu, aMSToolMenu, sizeof(aMSToolMenu)/sizeof(MenuMsgRes));
	return hMenu;
}

static MenuMsgRes aMSBookmarkMenu[] = {
	{ 0, IDM_BOOKMARK_ADD,    0, MSG_ID_MENUITEM_W32_B_ADDBM },
	{ 1, IDM_BOOKMARK_CONFIG, 0, MSG_ID_MENUITEM_W32_B_EDITBM },
	{ 2, -1,                  0, 0 },
};

HMENU LagendaPlatform::LoadMSBookmarkMenu()
{
	HMENU hMenu = CreatePopupMenu();
	OverrideMenuTitle(hMenu, aMSBookmarkMenu, sizeof(aMSBookmarkMenu)/sizeof(MenuMsgRes));
	return hMenu;
}

static MenuMsgRes aMDEditMenu[] = {
	{  0, IDM_CUT,             0,          MSG_ID_MENUITEM_MAIN_CUT },
	{  1, IDM_COPY,            0,          MSG_ID_MENUITEM_MAIN_COPY },
	{  2, IDM_PASTE,           0,          MSG_ID_MENUITEM_MAIN_PASTE },
	{  3, -1,                  0,          0 },
	{  4, IDM_SELALL,          0,          MSG_ID_MENUITEM_DETAILS_SELALL },
	{  5, -1,                  0,          0 },
	{  6, IDM_SEARCH,          0,          MSG_ID_MENUITEM_MAIN_FIND }, 
	{  7, IDM_SEARCH_NEXT,     0,          MSG_ID_MENUITEM_MAIN_FIND_NEXT },
	{  8, IDM_SEARCH_PREV,     0,          MSG_ID_MENUITEM_MAIN_FIND_PREV },
	{  9, -1,                  0,          0 },
	{ 10, IDM_DETAILS_HSCROLL, MF_CHECKED, MSG_ID_MENUITEM_TOOL_WRAP },
	{ 11, -1,                  0,          0 },
	{ 12, IDM_INSDATE1,        0,          MSG_ID_TOOLTIPS_INSDATE1 },
	{ 13, IDM_INSDATE2,        0,          MSG_ID_TOOLTIPS_INSDATE2 },
};

HMENU LagendaPlatform::LoadMDEditMenu()
{
	HMENU hMenu = CreatePopupMenu();
	OverrideMenuTitle(hMenu, aMDEditMenu, sizeof(aMDEditMenu)/sizeof(MenuMsgRes));
	return hMenu;
}


#endif // PLATFORM_BE500