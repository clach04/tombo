#ifndef WIN32PLATFORM_H
#define WIN32PLATFORM_H
#if defined(PLATFORM_WIN32)

#define PLATFORM_TYPE Win32Platform


class StatusBar;

class Win32Platform : public PlatformLayer {
protected:
	void ControlMenu(BOOL bTreeActive);
	void ControlToolbar(BOOL bTreeActive);

	StatusBar *pStatusBar;
public:
	HWND hRebar;
	HWND hToolBar;

	Win32Platform();
	virtual ~Win32Platform();

	void Create(HWND hWnd, HINSTANCE hInst);

	HWND GetMainToolBar() { return hToolBar; }

	HMENU GetMainMenu() { return GetMenu(hMainWnd); }
	HMENU GetMDToolMenu() { return GetMainMenu(); }
	HMENU GetMSEditMenu() { return GetMainMenu(); }
	HMENU GetMSBookMarkMenu() { return GetSubMenu(GetMainMenu(), BOOKMARK_MENU_POS); }

	void EnableMenu(UINT uid, BOOL bEnable);
	void EnableSearchNext();
	void CheckMenu(UINT uid, BOOL bCheck);

	void OpenDetailsView();
	void CloseDetailsView();

	void AdjustUserRect(RECT *r);

	void ShowStatusBar(BOOL bShow);
	void ShowRebar(BOOL bShow);

	void SetStatusIndicator(DWORD nPos, LPCTSTR pText, BOOL bDisp);
	WORD GetStatusBarHeight();
	void ResizeStatusBar(WPARAM wParam, LPARAM lParam);
	void GetStatusWindowRect(RECT *pRect);

	static Win32Platform *PlatformFactory() { return new Win32Platform(); }

	static HMENU LoadMainMenu();
	static HMENU LoadContextMenu(DWORD nFlg);
};

#endif // PLATFORM_WIN32
#endif