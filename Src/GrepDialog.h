#ifndef GREPDIALOG_H
#define GREPDIALOG_H

#include "TString.h"

/////////////////////////////////////////
// Advanced grep setting dialog
/////////////////////////////////////////

class GrepDialog {
	TString sPath;
	TString sMatchString;

	BOOL bCaseSensitive;
	BOOL bCheckCryptedMemo;
	BOOL bCheckFileName;
public:

	///////////////////////////
	// ctor & dtor

	GrepDialog() {}
	~GrepDialog() {}

	BOOL Init(LPCTSTR pPath);

	///////////////////////////
	// Popup function (entry point for other class&funcs)

	DWORD Popup(HINSTANCE hInst, HWND hWnd);

	///////////////////////////
	// Data accessor

	LPCTSTR GetPath() { return sPath.Get(); }
	LPCTSTR GetMatchString() { return sMatchString.Get(); }
	LPCTSTR GetName() { return NULL; }

	BOOL IsCaseSensitive() { return bCaseSensitive; }
	BOOL IsCheckCryptedMemo() { return bCheckCryptedMemo; }
	BOOL IsCheckFileName() { return bCheckFileName; }
	BOOL IsNegate() { return 0; }

	///////////////////////////
	// for property tabs
	void SetCaseSensitive(BOOL b) { bCaseSensitive = b; }
	void SetCheckCryptedMemo(BOOL b) { bCheckCryptedMemo = b; }
	void SetCheckFileName(BOOL b) { bCheckFileName = b; }
	TString *GetMatchStringPtr() { return &sMatchString; }
};

#endif
