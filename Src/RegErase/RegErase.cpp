#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "Message.h"
#define TOMBO_MAIN_KEY TEXT("Software\\flatfish\\Tombo")
#define FLATFISH_MAIN_KEY TEXT("Software\\flatfish")

TomboMessage g_mMsgRes;

void MessageError(DWORD n)
{
	LPCTSTR pMsg = MSG_REGERASE_ERROR;
	LPTSTR p = new TCHAR[_tcslen(MSG_REGERASE_ERROR) + 128];
	_stprintf(p, MSG_REGERASE_ERROR, n);
	MessageBox(NULL, p, MSG_REGERASE_TITLE, MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR pCmdLine, int nCmdShow)
{
	g_mMsgRes.Init();
	if (MessageBox(NULL, MSG_REGERASE_CONFIRM, MSG_REGERASE_TITLE, MB_ICONQUESTION | MB_YESNO) != IDYES) {
		MessageBox(NULL, MSG_REGERASE_CANCELED, MSG_REGERASE_TITLE, MB_OK);
		return 1;
	}

	BOOL bDeleted = FALSE;

	DWORD nResult = RegDeleteKey(HKEY_CURRENT_USER, TOMBO_MAIN_KEY);
	if (nResult != ERROR_SUCCESS) {
		if (nResult == ERROR_FILE_NOT_FOUND || nResult == ERROR_INVALID_PARAMETER) {
			bDeleted = TRUE;
		} else {
			MessageError(nResult);
			return 0;
		}
	}

	HKEY hFF;
#if defined(PLATFORM_WIN32)
	nResult = RegOpenKeyEx(HKEY_CURRENT_USER, FLATFISH_MAIN_KEY, 0, KEY_ALL_ACCESS, &hFF);
#else
	nResult = RegOpenKeyEx(HKEY_CURRENT_USER, FLATFISH_MAIN_KEY, 0, 0, &hFF);
#endif
	if (nResult == ERROR_SUCCESS) {

		// delete only if subkey under HKCU\Software\flatfish is not exist
		DWORD nSubKeys;
		if (RegQueryInfoKey(hFF, 
				NULL, NULL, NULL, &nSubKeys, NULL, NULL, NULL, NULL,
				NULL, NULL, NULL) == ERROR_SUCCESS) {

			RegCloseKey(hFF);
			if (nSubKeys == 0) {
				if (RegDeleteKey(HKEY_CURRENT_USER, FLATFISH_MAIN_KEY) == ERROR_SUCCESS) {
					bDeleted = FALSE;
				}
			}
		} else {
			RegCloseKey(hFF);
		}
	} else {
		if (nResult != ERROR_FILE_NOT_FOUND && nResult != ERROR_INVALID_PARAMETER) {
			MessageError(nResult);
			return 0;
		}
	}

	if (bDeleted) {
		MessageBox(NULL, MSG_REGERASE_NOT_EXIST, MSG_REGERASE_TITLE, MB_OK | MB_ICONINFORMATION | MB_APPLMODAL | MB_SETFOREGROUND);
	} else {
		MessageBox(NULL, MSG_REGERASE_DELETED, MSG_REGERASE_TITLE, MB_OK | MB_ICONINFORMATION | MB_APPLMODAL | MB_SETFOREGROUND);
	}

	return 0;
}
