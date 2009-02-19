#ifndef NEWFOLDERDIALOG_H
#define NEWFOLDERDIALOG_H

class NewFolderDialog {
	HWND hDialog;
	HINSTANCE hInstance;
	TCHAR aFolder[MAX_PATH];
	LPCTSTR pBaseText;
	DWORD nTitleID;
public:
	
	NewFolderDialog();
	void InitDialog(HWND hWnd);
	BOOL OnOK(HWND hWnd);

	DWORD Popup(HINSTANCE hInst, HWND hParent);
	LPCTSTR FolderName() { return aFolder; }

	void SetOption(DWORD nTitleID, LPCTSTR pText);
};

#endif