#ifndef STATUSBAR_H
#define STATUSBAR_H

///////////////////////////////////////////////////
// Status bar
///////////////////////////////////////////////////

class StatusBar {
public:
#if defined(PLATFORM_WIN32) || defined(PLATFORM_HPC)
	HWND hStatusBar;
#endif

	BOOL Create(HWND hParent, BOOL bNew);
	void ResizeStatusBar();

	void SetStatusIndicator(DWORD nPos, LPCTSTR pText, BOOL bDisp);
	void Show(BOOL bShow);

	void SendSize(WPARAM wParam, LPARAM lParam);

	void GetWindowRect(RECT *pRect);
	WORD GetHeight();
};

#endif