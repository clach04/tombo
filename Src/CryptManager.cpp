#include <windows.h>
#include <stdlib.h>
#include "CryptManager.h"
#include "File.h"

void WipeOutAndDelete(char *p, DWORD len);
#ifdef _WIN32_WCE
void WipeOutAndDelete(LPTSTR p, DWORD len);
#endif

BOOL CryptManager::Init(const char *pKey)
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	srand(ft.dwLowDateTime);

	getMD5Sum(md5key, (byte*)pKey, strlen(pKey));
	return TRUE;
}

//////////////////////////////////////////////////
// デストラクタ
//////////////////////////////////////////////////
// セキュリティ上、保持していたmd5keyを消去する。

CryptManager::~CryptManager()
{
	for (DWORD i = 0; i < 16; i++) {
		md5key[i] = 0;
	}
}

//////////////////////////////////////////////////
// データの暗号化
//////////////////////////////////////////////////

BOOL CryptManager::Encrypt(LPBYTE pBuf, int len)
{
	if (len == 0) return FALSE;

	if (!crypt.ResetStream(md5key, 16)) return FALSE;
	BYTE buf[8];
	LPBYTE p = pBuf;
	int n = len;
	int i;
	while (n > 8) {
		for (i = 0; i < 8; i++) {
			buf[i] = p[i];
		}
		crypt.Encrypt(p, buf, 8);
		p += 8;
		n -= 8;
	}
	if (n > 0) {
		for (i = 0; i < n; i++) {
			buf[i] = p[i];
		}
		crypt.Encrypt(p, buf, n);
	}

	for (i = 0; i < 8; i++) buf[i] = 0;
	return TRUE;
}

//////////////////////////////////////////////////
// データの復号
//////////////////////////////////////////////////

BOOL CryptManager::Decrypt(LPBYTE pBuf, int len)
{
	if (len == 0) return FALSE;

	if (!crypt.ResetStream(md5key, 16)) return FALSE;

	BYTE buf[8];
	LPBYTE p = pBuf;
	int n = len;
	int i;
	while (n >= 8) {
		for (i = 0; i < 8; i++) {
			buf[i] = p[i];
		}
		crypt.Decrypt(p, buf);
		p += 8;
		n -= 8;
	}

	for (i = 0; i < 8; i++) buf[i] = 0;
	return TRUE;
}

//////////////////////////////////////////////////
// データの暗号化とファイルへの保存
//////////////////////////////////////////////////

BOOL CryptManager::EncryptAndStore(const LPBYTE pData, int nSize, LPCTSTR pFileName)
{
	int len;
	LPBYTE pBuf = EncryptBuffer(pData, nSize, &len);
	// ファイルへの保存
	File outf;

	if (!outf.Open(pFileName, GENERIC_WRITE, 0, OPEN_ALWAYS)) {
		WipeOutAndDelete((char*)pBuf, len);
		return FALSE;
	}
	if (!outf.Write(pBuf, len)) {
		TCHAR buf[1024];
		wsprintf(buf, TEXT("CryptManager::EncryptAndStore write failed %d"), GetLastError());
		MessageBox(NULL, buf, TEXT("DEBUG"), MB_OK);
	}
	if (!outf.SetEOF()) return FALSE;
	outf.Close();

	WipeOutAndDelete((char*)pBuf, len);
	return TRUE;
}

//////////////////////////////////////////////////
// Encrypt data and add header
//////////////////////////////////////////////////
// CryptManagerによる暗号化ファイルのフォーマット
// The format of the container is:
// 0-3  : BF01(4 bytes)
// 4-7  : data length (include randum area + md5sum)(4 bytes)
// 8-15 :* random data(8 bytes)
//16-31 :* md5sum of plain text(16 bytes)
//32-   :* data

// '*' is encrypted.
// 

LPBYTE CryptManager::EncryptBuffer(const LPBYTE pData, int nSize, int *pLen)
{
	int i=0;
	int len = ((nSize >> 3) + 1) * 8;
	len += 24;

	len += 8;

	LPBYTE pBufF = new BYTE[len];
	if (pBufF == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	LPBYTE pBuf = pBufF + 8;

	// set random number
	for (i = 0; i < 8; i++) {
		pBuf[i] = (BYTE)(rand() & 0xFF);
	}

	strncpy((char*)pBufF, "BF01", 4);
	*(int*)(pBufF + 4) = nSize;

	// get md5sum of plain data
	getMD5Sum(pBuf + 8, pData, nSize);

	// copy plain data
	LPBYTE p = pBuf + 24;
	const BYTE *q = pData;
	for (i = 0; i < nSize; i++) {
		*p++ = *q++;
	}

	// encryption
	if (!Encrypt(pBuf, nSize + 24)) {
		WipeOutAndDelete((char*)pBufF, len);
		return NULL;
	}
	*pLen = len;
	return pBufF;
}

//////////////////////////////////////////////////
// Load from file and decrypt data
//////////////////////////////////////////////////

LPBYTE CryptManager::LoadAndDecrypt(LPDWORD pSize, LPCTSTR pFileName)
{
	File inf;
	if (!inf.Open(pFileName, GENERIC_READ, 0, OPEN_EXISTING)) {
		return NULL;
	}

	DWORD nFileSize = inf.FileSize();
	char version[5];
	DWORD n;
	DWORD nDataSize;

	// バージョンヘッダ
	n = 4;
	inf.Read((LPBYTE)version, &n);
	version[4] = '\0';
	if (strcmp(version, "BF01") != 0) {
		SetLastError(ERROR_INVALID_DATA);
		return NULL;
	}

	// データ長
	n = sizeof(nDataSize);
	inf.Read((LPBYTE)&nDataSize, &n);

	LPBYTE pBuf = new BYTE[nFileSize + 1];
	n = nFileSize - 4 - sizeof(nDataSize);
	inf.Read(pBuf, &n);

	if (!Decrypt(pBuf, n)) {
		WipeOutAndDelete((char*)pBuf, nFileSize + 1);
		return NULL;
	}

	// 復号化文MD5SUMの取得
	BYTE decriptsum[16];
	getMD5Sum(decriptsum, pBuf + 24, nDataSize);

	// 正しく復号化できたかのチェック
	for (int i = 0; i < 16; i++) {
		if (pBuf[8 + i] != decriptsum[i]) {
			WipeOutAndDelete((char*)pBuf, nFileSize + 1);
			SetLastError(ERROR_INVALID_PASSWORD);
			return NULL;
		}
	}
	pBuf[nDataSize + 24] = '\0';
	*pSize = nDataSize;

	// 領域再確保
	// 乱数データとMD5SUMをまとめて復号化するために1つのバッファで確保したが、
	// deleteを正しく行えるように領域を再確保、コピーして返す
	LPBYTE pData = new BYTE[nDataSize + 2];
	if (pData == NULL) {
		WipeOutAndDelete((char*)pBuf, nFileSize + 2);
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	memcpy(pData, pBuf + 24, nDataSize);
	pData[nDataSize] = '\0';
	pData[nDataSize + 1] = '\0';	// sentinel for the file is UTF16
	WipeOutAndDelete((char*)pBuf, nFileSize + 1);
	return pData;
}

LPBYTE CryptManager::DecryptBuffer(const LPBYTE pCrypted, int nSize)
{
	if (nSize % 8 != 0) {
		SetLastError(ERROR_INVALID_DATA);
		return NULL;
	}

	LPBYTE pBuf = new BYTE[nSize];
	if (pBuf == NULL) return NULL;

	memcpy(pBuf, pCrypted, nSize);

	if (!Decrypt(pBuf + 8, nSize - 8)) {
		WipeOutAndDelete((char*)pBuf, nSize);
		return NULL;
	}

	DWORD n = *(LPDWORD)(pBuf + 4);

	BYTE decriptsum[16];
	getMD5Sum(decriptsum, pBuf + 32, n);

	for (int i = 0; i < 16; i++) {
		if (pBuf[16 + i] != decriptsum[i]) {
			WipeOutAndDelete((char*)pBuf, nSize);
			SetLastError(ERROR_INVALID_PASSWORD);
			return NULL;
		}
	}

	LPBYTE pData = new BYTE[n + 1];
	if (pData == NULL) {
		WipeOutAndDelete((char*)pBuf, nSize);
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	memcpy(pData, pBuf + 32, n);
	pData[n] = 0;

	WipeOutAndDelete((char*)pBuf, nSize);
	return pData;
}

void WipeOutAndDelete(LPTSTR p, DWORD len)
{
	for (DWORD i = 0; i < len; i++) p[i] = TEXT('\0');
	delete [] p;
}


#ifdef _WIN32_WCE
void WipeOutAndDelete(char *p, DWORD len)
{
	for (DWORD i = 0; i < len; i++) p[i] = TEXT('\0');
	delete [] p;
}

#endif

