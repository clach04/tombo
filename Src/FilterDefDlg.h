#ifndef FILTERDEFDLG_H
#define FILTERDEFDLG_H

class VFInfo;

class FilterDefDlg {
public:
	VFInfo *pInfo;

	///////////////////////////
	// ctor & dtor

	FilterDefDlg() : pInfo(NULL) {}
	~FilterDefDlg() {}
	
	BOOL Init();

	///////////////////////////
	// Popup function (entry point for other class&funcs)

	DWORD Popup(HINSTANCE hInst, HWND hWnd, VFInfo *pInfo, BOOL bNew);
};

#endif
