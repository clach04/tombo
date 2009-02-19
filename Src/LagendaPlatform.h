#ifndef LAGENDAPLATFORM_H
#define LAGENDAPLATFORM_H
#if defined(PLATFORM_BE500)

#define PLATFORM_TYPE LagendaPlatform

class LagendaPlatform : public PlatformLayer {
public:
	HWND hMSCmdBar;
	HWND hMDCmdBar;

	HMENU hMSMemoMenu;
	HMENU hMSToolMenu;
	HMENU hMSBookMarkMenu;
	HMENU hMDEditMenu;

	void Create(HWND hWnd, HINSTANCE hInst);

	HMENU GetMDToolMenu() { return hMDEditMenu; }
	HMENU GetMSEditMenu() { return hMSMemoMenu; }
	HMENU GetMSBookMarkMenu() { return hMSBookMarkMenu; }
	HMENU GetMSToolMenu() { return hMSToolMenu; }

	void EnableMenu(UINT uid, BOOL bEnable);
	void EnableSearchNext() { /* nop */ }
	void CheckMenu(UINT uid, BOOL bCheck);

	void OpenDetailsView();
	void CloseDetailsView();

	void AdjustUserRect(RECT *r);

	void ShowStatusBar(BOOL bShow) { /* nop */ }
	void SetStatusIndicator(DWORD nPos, LPCTSTR pText, BOOL bDisp) { /* nop */ }
	WORD GetStatusBarHeight() { return 0; }
	void ResizeStatusBar(WPARAM wParam, LPARAM lParam) { /* nop */ }
	void GetStatusWindowRect(RECT *pRect);

	static LagendaPlatform *PlatformFactory() { return new LagendaPlatform(); }

	static HMENU LoadMSMemoMenu();
	static HMENU LoadMSToolMenu();
	static HMENU LoadMSBookmarkMenu();
	static HMENU LoadMDEditMenu();

};

#endif // PLATFORM_BE500
#endif