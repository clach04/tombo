#include <windows.h>
#include "FileInputStream.h"

// ストリームから1文字読み込む。
// 読み込めない場合には0を返す。この場合、GetLastError()にエラーコード。
// EOFの場合にはGetLastError()はERROR_HANDLE_EOFを返す

char FileInputStream::getNext()
{

	DWORD nr;

	while(1) {
		// バッファに残っていればそれを返す
		if (pCur < pData) {
			return *pCur++;
		}

		// すでにEOFならEOFを返す。
		if (bEof == TRUE) {
			SetLastError(ERROR_HANDLE_EOF);
			return 0;
		}

		// 残っていなければファイルから読み込む
		if (!ReadFile(hHandle, buf, sizeof(buf), &nr, NULL)) {
			return 0;
		}
		if (sizeof(buf) != nr) {
			bEof = TRUE;
		}
		pCur = buf;
		pData = buf + nr;
	}
	return 0; // dummy
}

BOOL FileInputStream::GetLine(char *buf, DWORD s, BOOL bNoCR)
{
	char *p = buf;
	char *max = buf + s;
	BOOL bCR = FALSE;

	while(TRUE) {
		if (p >= max) {
			SetLastError(ERROR_MORE_DATA);
			return FALSE;
		}
		
		*p = getNext();
		if (*p == 0) {
			if (GetLastError() == ERROR_HANDLE_EOF && p != buf) {
				*p++ = '\0';
				return TRUE;
			}
			return FALSE;
		} else if (*p == '\r') {
			bCR = TRUE;
			p++;
			continue;
		} else if (*p == '\n' && bCR) {
			if (bNoCR) {
				*(p-1) = '\0';
			} else {
				p++;
			}
			break;
		}

		p++;
	}
	*p++ = '\0';
	return TRUE;

}

