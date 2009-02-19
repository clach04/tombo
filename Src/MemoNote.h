#ifndef MEMONOTE_H
#define MEMONOTE_H

class PasswordManager;
class TString;
class TomboURI;
class Repository;

////////////////////////////////////////
// Note type definition
////////////////////////////////////////

#define NOTE_TYPE_NO      0
#define NOTE_TYPE_PLAIN   1
#define NOTE_TYPE_CRYPTED 2
#define NOTE_TYPE_TDT     3

////////////////////////////////////////
// represents notes
////////////////////////////////////////

class MemoNote {
private:
	virtual BOOL SaveData(PasswordManager *pMgr, const LPBYTE pData, DWORD nLen, LPCTSTR pWriteFile) = 0;
	static BOOL MemoNoteFactory(LPCTSTR pFile, MemoNote **ppNote);
protected:
	LPTSTR pPath;
public:
	///////////////////////////////////////////
	// initialize

	MemoNote();
	virtual ~MemoNote();

	BOOL Init(LPCTSTR p);

	///////////////////////////////////////////

	virtual MemoNote *GetNewInstance() const = 0;
	virtual LPCTSTR GetExtension() = 0;

	//////////////////////////////////
	// memo data operation members

	// get memo data
	virtual LPBYTE GetMemoBodyNative(LPCTSTR pTopDir, PasswordManager *pMgr, LPDWORD pSize) const = 0;
	LPTSTR GetMemoBody(LPCTSTR pTopDir, PasswordManager *pMgr) const;

	// save memo
	BOOL SaveDataT(PasswordManager *pMgr, LPCTSTR pMemo, LPCTSTR pWriteFile);

	//////////////////////////////////
	// Encryption/Decryption : 
	//     returns MemoNote instance after encryption/decryption.
	//     set buffer to pHeadLine
	//     if headline string is changed, *pIsModified is set to TURE and
	//     new headline string is set to buffer.

	virtual MemoNote *Decrypt(LPCTSTR pTopDir, PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified) const;

	//////////////////////////////////
	// File operation

	virtual BOOL DeleteMemoData(LPCTSTR pTopDir) const ;
	BOOL Rename(LPCTSTR pTopDir, LPCTSTR pNewName);
	static MemoNote *CopyMemo(LPCTSTR pTopDir, const MemoNote *pOrig, LPCTSTR pMemoPath, TString *pHeadLine);

	//////////////////////////////////
	// path related functions

	LPCTSTR MemoPath() const { return pPath; }
	BOOL SetMemoPath(LPCTSTR p);
	BOOL GetURI(LPCTSTR pRepoName, TomboURI *pURI) const;

	//////////////////////////////////
	// notes attributes

	static DWORD IsNote(LPCTSTR pFile);

	//////////////////////////////////
	// Factory method for MemoNote.
	// create and initialize MemoNote and return by ppNote.
	//
	// if creation failed, return FALSE.
	// if pFile is not memo, return TRUE and *ppNote sets to NULL.
	static MemoNote *MemoNoteFactory(const TomboURI *pURI);

	//////////////////////////////////
	// Headline related funcs

	static BOOL GetHeadLinePath(LPCTSTR pTopDir, LPCTSTR pMemoPath, LPCTSTR pHeadLine, LPCTSTR pExt, 
								TString *pFullPath, LPCTSTR *ppNotePath, TString *pNewHeadLine);
	static BOOL GetHeadLineFromMemoText(LPCTSTR pMemo, TString *pHeadLine);
	static BOOL GetHeadLineFromFilePath(LPCTSTR pFilePath, TString *pHeadLine);

};

////////////////////////////////////////
// Plain notes
////////////////////////////////////////

class PlainMemoNote : public MemoNote {
	BOOL SaveData(PasswordManager *pMgr, const LPBYTE pData, DWORD nLen, LPCTSTR pWriteFile);

public:
	MemoNote *GetNewInstance() const ;
	LPCTSTR GetExtension();

	LPBYTE GetMemoBodyNative(LPCTSTR pTopDir, PasswordManager *pMgr, LPDWORD pSize) const;

};

////////////////////////////////////////
// Encrypted notes
////////////////////////////////////////

class CryptedMemoNote : public MemoNote {
	BOOL SaveData(PasswordManager *pMgr, const LPBYTE pData, DWORD nLen, LPCTSTR pWriteFile);

protected:
	LPBYTE GetMemoBodySub(LPCTSTR pTopDir, PasswordManager *pMgr, LPDWORD pSize) const;
public:
	MemoNote *GetNewInstance() const ;
	LPCTSTR GetExtension();

	LPBYTE GetMemoBodyNative(LPCTSTR pTopDir, PasswordManager *pMgr, LPDWORD pSize) const;

	MemoNote *Decrypt(LPCTSTR pTopDir, PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified) const;
};

#endif