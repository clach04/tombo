#ifndef CRYPTMANAGER_H
#define CRYPTMANAGER_H

#include "Crypt.h"

class CryptManager {
	Crypt crypt;
	BYTE md5key[16];

public:
	~CryptManager();

	// Set password
	BOOL Init(const char *pKey);

	// pBufに平文を設定して呼ぶことでpBufに暗号文を返す。
	BOOL Encrypt(LPBYTE pBuf, int len);

	// pBufに暗号文を設定して呼ぶことでpBufに平文を返す。
	// lenは8の倍数でなければならない。
	BOOL Decrypt(LPBYTE pBuf, int len);

	// テキストを受け取り、暗号化してファイルに保存する
	BOOL EncryptAndStore(const LPBYTE pData, int nSize, LPCTSTR pFileName);

	// 暗号化されたファイルを復号化する。*pSizeには本当のバイト数が入る。
	// また、戻り値のデータ列の最後にはNULLが付加されている。
	// 戻り値のバッファはnewされているため、いらなくなった段階で呼び出し元がdelete[]する必要がある。
	LPBYTE LoadAndDecrypt(LPDWORD pSize, LPCTSTR pFileName);

	LPBYTE EncryptBuffer(const LPBYTE pData, int nSize, int *pLen);
	LPBYTE DecryptBuffer(const LPBYTE pCrypted, int nSize);
};

extern "C" {
	void getMD5Sum(byte *md5sum, byte *in, int len);
};
#endif

