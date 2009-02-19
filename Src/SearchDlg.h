#ifndef SEARCHDLG_H
#define SEARCHDLG_H

#include "Property.h"

////////////////////////////////////////////////////////////////
// 検索ダイアログ
////////////////////////////////////////////////////////////////

class SearchDialog {
	HWND hDialog;
	HINSTANCE hInstance;

	LPTSTR pSearchStr;
	BOOL bCaseSensitive;
	BOOL bCheckEncrypt;
	BOOL bFileNameOnly;
	BOOL bSearchDirectionUp;
	BOOL bEnableDirection;
public:

	////////////////////////////////
	// ctor & dtor

	SearchDialog() : hDialog(NULL), hInstance(NULL), pSearchStr(NULL), bCaseSensitive(FALSE), bFileNameOnly(FALSE), bSearchDirectionUp(FALSE), bEnableDirection(FALSE) {}
	~SearchDialog();

	////////////////////////////////
	// イベントハンドラ

	void InitDialog(HWND hDlg);

	BOOL OnOK();

	////////////////////////////////
	// ダイアログ起動

	DWORD Popup(HINSTANCE hInst, HWND hParent, BOOL bCheckEncrypt);

	////////////////////////////////
	// 結果取得

	LPCTSTR SearchString() { return pSearchStr; }
	BOOL IsCaseSensitive() { return bCaseSensitive; }
	BOOL IsSearchEncryptMemo() { return bCheckEncrypt; }
	BOOL IsFileNameOnly() { return bFileNameOnly; }
	BOOL IsSearchDirectionUp() { return bSearchDirectionUp; }
};


#endif
