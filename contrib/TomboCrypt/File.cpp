#include "wintypes.h"
#include "File.h"

//////////////////////////////////////////////////////////////////////
// dtor
//////////////////////////////////////////////////////////////////////

File::~File()
{
	Close();
}

//////////////////////////////////////////////////////////////////////
// ファイルオープン
//////////////////////////////////////////////////////////////////////

BOOL File::Open(LPCTSTR pFileName, bool bWrite)
{
	hFile = fopen(pFileName, (bWrite ? "wb+" : "rb"));

	if (hFile == NULL) return FALSE;

	if (fseek(hFile, 0, SEEK_END) != 0) {
		Close();
		return FALSE;
	}
	nSize = ftell(hFile);

	if (fseek(hFile, 0, SEEK_SET) != 0) {
		Close();
		return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// ファイルクローズ
//////////////////////////////////////////////////////////////////////

void File::Close()
{
	if (hFile != NULL) {
		fclose(hFile);
		hFile = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// ファイルポインタのシーク
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// 現在のファイルポインタの取得
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// ファイル読み込み
//////////////////////////////////////////////////////////////////////

BOOL File::Read(LPBYTE pBuf, LPDWORD pSize)
{
	DWORD n = *pSize;
	*pSize = (DWORD)fread(pBuf, 1, n, hFile);
	if (n != *pSize) return FALSE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// ファイル書き込み
//////////////////////////////////////////////////////////////////////

BOOL File::Write(const LPBYTE pBuf, DWORD nSize)
{
	DWORD n = nSize;
	nSize = (DWORD)fwrite(pBuf, 1, n, hFile);
	if (n != nSize) return FALSE;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////
// Set EOF
//////////////////////////////////////////////////////////////////////

BOOL File::SetEOF()
{
	//return SetEndOfFile(hFile);
	return TRUE;
}
