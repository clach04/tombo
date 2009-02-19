#ifndef PROPERTYPAGE_H
#define PROPERTYPAGE_H

class PropertyTab;
#define PROPERTY_TITLE_MAX_SIZE 256

/////////////////////////////////////////////////
// Windows property page control(?) wrapper
/////////////////////////////////////////////////

typedef PropertyTab *PPropertyTab;

class PropertyPage {
public:
	DWORD Popup(HINSTANCE hInst, HWND hWnd, PPropertyTab *ppPage, DWORD nPage, LPCTSTR pTitle, LPTSTR pIcon, DWORD nStart = 0);
};

/////////////////////////////////////////////////
// Property tab
/////////////////////////////////////////////////

class PropertyTab {
protected:
	DWORD nResourceID;
	DLGPROC pDlgProc;
	TCHAR aTitle[PROPERTY_TITLE_MAX_SIZE];
	
	/////////////////////////////
	// default dialog procedure
	
	// If you don't need something special, use this function for 
	// DLGPROC in ctor.
	// This function calls Init() at creation time, 
	// OnCommand() as WM_COMMAND handler, and 
	// Apply() when "OK" button pressed.
	static BOOL APIENTRY DefaultPageProc(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam);

protected:
	//////////////////////////
	// Tab page callback

	virtual void Init(HWND hDlg) {}
	virtual BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam) { return FALSE; }
	virtual BOOL OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam) { return FALSE; }

	// if return FALSE, block exiting dialog.
	virtual BOOL Apply(HWND hDlg) { return TRUE; } 
	virtual void Cancel(HWND hDlg, WPARAM wParam, LPARAM lParam) {}

public:
	PropertyTab(DWORD id, DWORD nTitleResID, DLGPROC proc);
	PropertyTab(DWORD id, LPCTSTR pTitle, DLGPROC proc);

	virtual ~PropertyTab() {}

	DWORD ResourceID() { return nResourceID; }
	DLGPROC DialogProc() { return pDlgProc; }
	LPCTSTR Title() { return aTitle; }
};


#endif
