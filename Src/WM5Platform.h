#ifndef WM5PLATFORM_H
#define WM5PLATFORM_H
#if defined(PLATFORM_WM5)

#define PLATFORM_TYPE WM5Platform

#define SHGetMenu(hWndMB)  (HMENU)SendMessage((hWndMB), SHCMBM_GETMENU, (WPARAM)0, (LPARAM)0)
#define SHGetSubMenu(hWndMB,ID_MENU) (HMENU)SendMessage((hWndMB), SHCMBM_GETSUBMENU, (WPARAM)0, (LPARAM)ID_MENU)
#define SHSetSubMenu(hWndMB,ID_MENU) (HMENU)SendMessage((hWndMB), SHCMBM_SETSUBMENU, (WPARAM)0, (LPARAM)ID_MENU)

class WM5Platform : public PlatformLayer {
	HWND hMainWnd;
	HWND hMSCmdBar;
	HWND hMDCmdBar;

public:

	void Create(HWND hWnd, HINSTANCE hInst);

	HMENU GetMDToolMenu();
	HMENU GetMSBookMarkMenu();

	void EnableMenu(UINT uid, BOOL bEnable);
	void EnableSearchNext();
	void CheckMenu(UINT uid, BOOL bCheck);

	void OpenDetailsView();
	void CloseDetailsView();

	void AdjustUserRect(RECT *r);

	void ShowStatusBar(BOOL bShow) {/* nop */}
	void SetStatusIndicator(DWORD nPos, LPCTSTR pText, BOOL bDisp) { /* nop */ }
	WORD GetStatusBarHeight() { return 0; }
	void ResizeStatusBar(WPARAM wParam, LPARAM lParam) { /* nop */ }
	void GetStatusWindowRect(RECT *pRect);

	static WM5Platform *PlatformFactory() { return new WM5Platform(); }

	static HMENU LoadSelectViewPopupMenu();
	static HMENU LoadDetailsViewPopupMenu();
};

#endif

#endif
