#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

class PasswordDialog {
	char *pPassword;

	HWND hDialog;
	HINSTANCE hInstance;
	BOOL bVerify;
public:
	void ClearPassword(HWND hDlg);

	PasswordDialog() : pPassword(NULL), hDialog(NULL), hInstance(NULL), bVerify(FALSE) {}
	~PasswordDialog();

	void InitDialog(HWND hWnd);
	BOOL OnOK(HWND hWnd);

	DWORD Popup(HINSTANCE hInst, HWND hParent, BOOL bVerify);
	const char *Password() { return pPassword; }
};


#endif