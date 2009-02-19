#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include "Tombo.h"
#include "PasswordManager.h"
#include "PasswordDialog.h"
#include "CryptManager.h"
#include "Property.h"
#include "resource.h"
#include "Message.h"

#define FINGERPRINT_VERSION 1
#define FINGERPRINT_NUM_ENCRYPT 64

//////////////////////////////////////////
// global vars
//////////////////////////////////////////

PasswordManager *g_pPassManager = NULL;

//////////////////////////////////////////
// デストラクタ
//////////////////////////////////////////

PasswordManager::~PasswordManager()
{
	ForgetPassword();
}

//////////////////////////////////////////
// 初期化
//////////////////////////////////////////

BOOL PasswordManager::Init(HWND hWnd, HINSTANCE hInst)
{
	hParent = hWnd;
	hInstance = hInst;
	UpdateAccess();
	return TRUE;
}


//////////////////////////////////////////
// パスワードの取得
//////////////////////////////////////////

const char *PasswordManager::Password(BOOL *pCancel, BOOL bEncrypt)
{
	if (pPassword) return pPassword;

	PasswordDialog dlg;

	BOOL bPrev = bDisableHotKey;
	bDisableHotKey = TRUE;
	DWORD nResult = dlg.Popup(hInstance, hParent, bEncrypt);
	bDisableHotKey = bPrev;
	if ( nResult == IDOK) {
		if (dlg.Password() == NULL) {
			TomboMessageBox(hParent, MSG_CANT_GET_PASS,
						TEXT("ERROR"), MB_ICONSTOP | MB_OK);
			return NULL;
		} else {
			// タイマーのセット

			if (!SetTimer(hParent, ID_PASSWORDTIMER, 60*1000, NULL)) {
				TomboMessageBox(hParent, MSG_TIMER_SET_FAILED, TEXT("ERROR"), MB_ICONSTOP | MB_OK);
				return NULL;
			}

			// パスワードの保持
			pPassword = new char[strlen(dlg.Password()) + 1];
			strcpy(pPassword, dlg.Password());
			return pPassword;
		}
	} else {
		*pCancel = TRUE;
		return NULL;
	}
}


//////////////////////////////////////////
// パスワードの無効化
//////////////////////////////////////////

void PasswordManager::ForgetPassword()
{
	if (pPassword) {
		char *p = pPassword;
		while (*p) {
			*p++ = '\0';
		}
		delete [] pPassword;
		pPassword = NULL;
		KillTimer(hParent, ID_PASSWORDTIMER);
	}
}

//////////////////////////////////////////
// FingerPrintの計算
//////////////////////////////////////////
// GetFingerPrint, SetFingerPrintの下請け


static BOOL GetFingerPrintFromSeed(LPBYTE pResult, const LPBYTE pSeed, const char *pPass)
{
	CryptManager cMgr;
	if (!cMgr.Init(pPass)) return FALSE;

	BYTE workbuf1[16], workbuf2[16];
	DWORD i, count;

	for (i = 0; i < 16; i++) workbuf1[i] = pSeed[i];

	for (count = 0; count < FINGERPRINT_NUM_ENCRYPT; count++) {
		// 暗号化
		if (!cMgr.Encrypt(workbuf1, 16)) return FALSE;

		// MD5によるハッシュ化
		getMD5Sum(workbuf2, workbuf1, 16);

		for (i = 0; i < 16; i++) workbuf1[i] = workbuf2[i];
	}
	for (i = 0; i < 16; i++) pResult[i] = workbuf1[i];

	return TRUE;
}

//////////////////////////////////////////
// フィンガープリントの取得
//////////////////////////////////////////

BOOL GetFingerPrint(LPBYTE pFp, const char *pPassword)
{
	DWORD i;

	// FingerPrintのseed取得のための乱数系列初期化
	SYSTEMTIME st;
	GetSystemTime(&st);
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	srand(ft.dwLowDateTime);

	// FingerPrintのseedの取得
	for (i = 0; i < 16; i++) {
		pFp[i] = (BYTE)(rand() & 0xFF);
	}

	// FingerPrintの取得
	pFp[0] = FINGERPRINT_VERSION;
	BOOL bResult = GetFingerPrintFromSeed(pFp + 17, pFp + 1, pPassword);

	return bResult;
}

//////////////////////////////////////////
// フィンガープリントのチェック
//////////////////////////////////////////

BOOL CheckFingerPrint(LPBYTE pFp, const char *pPassword)
{
	BYTE result[16];

	if (pFp[0] != FINGERPRINT_VERSION) {
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}

	if (!GetFingerPrintFromSeed(result, pFp + 1, pPassword)) return FALSE;
	for (DWORD i = 0; i < 16; i++) {
		if (result[i] != pFp[17 + i]) return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////
// アクセスの続行
//////////////////////////////////////////
void PasswordManager::UpdateAccess()
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ftLastAccess);
}

void PasswordManager::ForgetPasswordIfNotAccessed()
{
	if (pPassword == NULL) return;

	SYSTEMTIME st;
	FILETIME ftNow;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ftNow);

	UINT64 utLastAccess = ((UINT64)ftLastAccess.dwHighDateTime << 32) | (UINT64)ftLastAccess.dwLowDateTime;
	UINT64 utNow = ((UINT64)ftNow.dwHighDateTime << 32) | (UINT64)ftNow.dwLowDateTime;

	UINT64 utDiff = (utNow - utLastAccess) / 10000000;
	utDiff /= 60;

	if (utDiff > g_Property.GetPassTimeout()) {
		SendMessage(hParent, WM_TIMER, 0, 0);
	}
}
