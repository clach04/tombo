#ifndef PLATFORMLAYER_H
#define PLATFORMLAYER_H

///////////////////////////////////////////////////
// constants
///////////////////////////////////////////////////

#define BOOKMARK_MENU_POS 2
#define NUM_BOOKMARK_SUBMENU_DEFAULT 3

#define CTXMENU_DIR          1
#define CTXMENU_FILE         2
#define CTXMENU_USEASSOC     4
#define CTXMENU_ENABLEEXTAPP 8

///////////////////////////////////////////////////
// Platform abstruction
///////////////////////////////////////////////////

class PlatformLayer {
protected:
	HWND hMainWnd;
public:
	PlatformLayer();
	virtual ~PlatformLayer();

	BOOL Init(HWND h);

	virtual void Create(HWND hWnd, HINSTANCE hInst) = 0;

	static HMENU LoadContextMenu(DWORD nFlg);
};

///////////////////////////////////////////////////
///////////////////////////////////////////////////
// helper function/classes
///////////////////////////////////////////////////
///////////////////////////////////////////////////

void SetAppIcon(HINSTANCE hInst, HWND hWnd);

///////////////////////////////////////////////////
// menu related
///////////////////////////////////////////////////

struct MenuMsgRes {
	int iPos;
	int iMenuID;
	int iExtOpt;
	int iMsgID;
	MenuMsgRes *pSubMenu;
};

void AddMenuItemByMsgRes(HMENU hMenu, MenuMsgRes *pRes);
void OverrideMenuTitle(HMENU hMenu, MenuMsgRes *pRes, int nNumRes);

#if defined(PLATFORM_WIN32)
#include "Win32Platform.h"
#endif
#if defined(PLATFORM_PKTPC)
#include "PocketPCPlatform.h"
#endif
#if defined(PLATFORM_WM5)
#include "WM5Platform.h"
#endif
#if defined(PLATFORM_PSPC)
#include "PsPCPlatform.h"
#endif
#if defined(PLATFORM_HPC)
#include "HPCPlatform.h"
#endif
#if defined(PLATFORM_BE500)
#include "LagendaPlatform.h"
#endif

#endif