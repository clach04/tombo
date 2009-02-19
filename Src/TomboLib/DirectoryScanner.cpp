#include <windows.h>
#include <tchar.h>

#include "Uniconv.h"
#include "DirectoryScanner.h"

//////////////////////////////////////////////////
// ディレクトリ階層を走査する
//////////////////////////////////////////////////

BOOL DirectoryScanner::Init(LPCTSTR p, DWORD nFlg)
{
	_tcscpy(aScanPath, p);

	// 末尾が\で終るように調整
	ChopFileSeparator(aScanPath);
	_tcscat(aScanPath, TEXT("\\"));

	nScanFlag = nFlg;
	return TRUE;
}


BOOL DirectoryScanner::Scan()
{
	bContinue = TRUE;

	InitialScan();
	if (bContinue) {
		ScanDirectory();
	}
	AfterScan();
	return bContinue;
}

//////////////////////////////////////////////////
// スキャン処理実体
//////////////////////////////////////////////////

void DirectoryScanner::ScanDirectory()
{
	LPTSTR pTail = aScanPath + _tcslen(aScanPath);

	_tcscat(pTail, TEXT("*.*"));

	WIN32_FIND_DATA wfd;
	HANDLE hHandle = FindFirstFile(aScanPath, &wfd);
	if (hHandle != INVALID_HANDLE_VALUE) {
		do {
			if (_tcscmp(wfd.cFileName, TEXT(".")) == 0 || _tcscmp(wfd.cFileName, TEXT("..")) == 0) continue;
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// フォルダ
				if (!(nScanFlag & DIRECTORY_SCAN_NOREC)) {
					_tcscpy(pTail, wfd.cFileName);
					_tcscat(pTail, TEXT("\\"));

					PreDirectory(wfd.cFileName);
					ScanDirectory();
					PostDirectory(wfd.cFileName);
					if (!bContinue) break;
				}
			} else {
				// ファイル
				if (!(nScanFlag & DIRECTORY_SCAN_NOFILE)) {
					_tcscpy(pTail, wfd.cFileName);
					File(wfd.cFileName);
				}
				if (!bContinue) break;
			}
		} while(FindNextFile(hHandle, &wfd));
		FindClose(hHandle);
	}
	*pTail = TEXT('\0');
}
