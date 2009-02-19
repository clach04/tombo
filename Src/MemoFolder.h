#ifndef MEMOFOLDER_H
#define MEMOFOLDER_H

class PasswordManager;
class TString;

/////////////////////////////////////////
// Folder operation classes
/////////////////////////////////////////
//
// Thease classes should be used by only LocalRepository.
// If you want to use thease features, request to it.


class MemoFolder {
	LPCTSTR pTopDir;
	LPTSTR pFullPath;
	TString sErrorReason;
public:

	///////////////////////////////////////////////
	// ctor & dtor
	MemoFolder();
	~MemoFolder();
	BOOL Init(LPCTSTR pTopDir, LPCTSTR pPath);

	BOOL Copy(LPCTSTR pDst);
	BOOL Move(LPCTSTR pDst);
	BOOL Rename(LPCTSTR pNewName);

	LPCTSTR GetErrorReason() { return sErrorReason.Get(); }
};

/////////////////////////////////////////
// Encrypt/decrypt folder
/////////////////////////////////////////

class DSEncrypt: public DirectoryScanner {
	DWORD nBaseLen;
	PasswordManager *pPassMgr;
	BOOL bEncrypt;

	TString *pURI;
	DWORD nURIBufSize;
	DWORD nCurrentPos;

protected:
	BOOL CheckURIBuffer(LPCTSTR p);

public:
	DWORD nNotEncrypted;

	~DSEncrypt();
	// If bEncrypt is FALSE, decrypt files.
	BOOL Init(LPCTSTR pTopDir, LPCTSTR pPath, LPCTSTR pBaseURI, BOOL bEncrypt);

	void InitialScan() {}
	void AfterScan() {}
	void PreDirectory(LPCTSTR);
	void PostDirectory(LPCTSTR);
	void File(LPCTSTR);
};

///////////////////////////////////////////////
// Delete directory
///////////////////////////////////////////////

class DSFileDelete: public DirectoryScanner {
public:
	TCHAR aFailPath[MAX_PATH * 2];
	TCHAR aErrorMsg[1024];

	DSFileDelete();

	BOOL Init(LPCTSTR pPath); 
	void InitialScan();
	void AfterScan();
	void PreDirectory(LPCTSTR);
	void PostDirectory(LPCTSTR);
	void File(LPCTSTR);
};

#endif
