#ifndef FF_FILE_H
#define FF_FILE_H


// Though WIN32 API supports 64bit file size, 
// File object treats file size 32bit for simplify

////////////////////////////////////////
// File I/O
////////////////////////////////////////

class File {
protected:
	HANDLE hFile;
	DWORD nSize;
public:

	// ctor & dtor
	File() : hFile(INVALID_HANDLE_VALUE) {}
	~File();

	BOOL Open(LPCTSTR pFileName, DWORD nMode, DWORD nShareMode, DWORD nOpenMode);
	void Close();

	// Get file size
	DWORD FileSize() { return nSize; }

	// Seek file pointer
	BOOL Seek(DWORD nPos);

	// Get current file pointer
	DWORD CurrentPos();

	BOOL Read(LPBYTE pBuf, LPDWORD pSize);
	BOOL Write(const LPBYTE pBuf, DWORD nSize);

	BOOL WriteUnicodeString(LPCWSTR p);

	BOOL SetEOF();
};

#endif