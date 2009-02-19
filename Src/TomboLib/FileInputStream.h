#ifndef FILEINPUTSTREAM_H
#define FILEINPUTSTREAM_H

class FileInputStream {
	HANDLE hHandle;
	char buf[4096];
	char *pCur, *pData;
	BOOL bEof;

public:
	FileInputStream(HANDLE h = INVALID_HANDLE_VALUE) : hHandle(h), pCur(buf), pData(buf), bEof(FALSE) {}

	void Assoc(HANDLE h) { hHandle = h; pCur = pData = buf; bEof = FALSE; }
	char getNext();

	// bNoCR = TRUEの場合、pBufに設定される値の末尾の\r\nは落として設定する。
	// bNoCR = FALSEの場合、末尾には\r\nが残る
	BOOL GetLine(char *pBuf, DWORD size, BOOL bNoCR = FALSE);
};

#endif
