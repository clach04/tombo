#ifndef HPCPLATFORM_H
#define HPCPLATFORM_H
#if defined(PLATFORM_HPC)

#define PLATFORM_TYPE HPCPlatform

class StatusBar;

class HPCPlatform : public PlatformLayer {
protected:
	void ControlMenu(BOOL bTreeActive);
	void ControlToolbar(BOOL bTreeActive);

	StatusBar *pStatusBar;
public:
	HWND hMSCmdBar;

	HPCPlatform();
	virtual ~HPCPlatform();

	void Create(HWND hWnd, HINSTANCE hInst);

	HWND GetMainToolBar();

	HMENU GetMainMenu();
	HMENU GetMDToolMenu();
	HMENU GetMSEditMenu();
	HMENU GetMSBookMarkMenu();

	void EnableMenu(UINT uid, BOOL bEnable);
	void EnableSearchNext();
	void CheckMenu(UINT uid, BOOL bCheck);

	void OpenDetailsView();
	void CloseDetailsView();

	void AdjustUserRect(RECT *r);

	void ShowStatusBar(BOOL bShow);
	void SetStatusIndicator(DWORD nPos, LPCTSTR pText, BOOL bDisp);
	WORD GetStatusBarHeight();
	void ResizeStatusBar(WPARAM wParam, LPARAM lParam);
	void GetStatusWindowRect(RECT *pRect);

	static HPCPlatform *PlatformFactory() { return new HPCPlatform(); }

	static HMENU LoadMainMenu();
//	static HMENU LoadContextMenu(DWORD nFlg);
};

#endif // PLATFORM_HPC
#endif