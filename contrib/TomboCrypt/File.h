#ifndef FF_FILE_H
#define FF_FILE_H

#include <stdio.h>

// WIN32 APIでは64bitのファイルサイズをサポートしているが、
// 通常のアプリケーションでは32bitで十分なのでプログラムの簡略化のため、
// 内部では32bitで処理する。

class File {
protected:
	FILE* hFile;
	DWORD nSize;
public:

	// コンストラクタ&デストラクタ
	File() : hFile(NULL) {}
	~File();

	//BOOL Open(LPCTSTR pFileName, DWORD nMode, DWORD nShareMode, DWORD nOpenMode);
	BOOL Open(LPCTSTR pFileName, bool bWrite);

	void Close();

	// ファイルサイズの取得
	DWORD FileSize() { return nSize; }

	// ファイルポインタのシーク
	//BOOL Seek(DWORD nPos);
	// 現在のファイルポインタの取得
	//DWORD CurrentPos();

	BOOL Read(LPBYTE pBuf, LPDWORD pSize);
	BOOL Write(const LPBYTE pBuf, DWORD nSize);

	//BOOL WriteUnicodeString(LPCWSTR p);

	BOOL SetEOF();
};

#endif
