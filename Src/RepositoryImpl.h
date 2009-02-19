#ifndef REPOSITORYIMPL_H
#define REPOSITORYIMPL_H

class TomboURI;
class TString;
class Repository;
class URIOption;
class DirList;
class URIList;

class MemoNote;

/////////////////////////////////////////
// Repository implimentation 
/////////////////////////////////////////
// The base class of implimentation

class RepositoryImpl {
private:
	LPTSTR pRepName;
	LPTSTR pDispName;
	TomboURI *pRootURI;
	DWORD nRepType;
	DWORD nRepNameLen;

public:
	RepositoryImpl();
	virtual ~RepositoryImpl();

	BOOL Init(LPCTSTR pRepName, LPCTSTR pDispName, DWORD nRepType);

	LPCTSTR GetRepositoryName() { return pRepName; }
	LPCTSTR GetDisplayName() { return pDispName; }
	const TomboURI *GetRootURI() { return pRootURI; }
	DWORD GetRepNameLen() { return nRepNameLen; }
	DWORD GetRepositoryType() { return nRepType; }

	////////////////////////////
	// Interface definitions

	// clone this instance
	virtual RepositoryImpl *Clone() = 0;

	// get XML string to restore this subrepository
	virtual LPTSTR GetXMLSaveString() = 0;

	virtual BOOL Update(const TomboURI *pCurrentURI, LPCTSTR pData, TomboURI *pNewURI, TString *pNewHeadLine) = 0;
	virtual BOOL Delete(const TomboURI *pURI, URIOption *pOption) = 0;
	virtual BOOL Copy(const TomboURI *pCopyFrom, const TomboURI *pCopyTo, URIOption *pOption) = 0;
	virtual BOOL Move(const TomboURI *pMoveFrom, const TomboURI *pMoveTo, URIOption *pOption) = 0;

	virtual BOOL ChangeHeadLine(const TomboURI *pURI, LPCTSTR pReqNewHeadLine, URIOption *pOption) = 0;

	virtual BOOL GetHeadLine(const TomboURI *pURI, TString *pHeadLine) = 0;

	virtual BOOL GetOption(const TomboURI *pURI, URIOption *pOption) const = 0;
	virtual BOOL SetOption(const TomboURI *pCurrentURI, URIOption *pOption) = 0;

	virtual BOOL GetPhysicalPath(const TomboURI *pURI, TString *pFullPath) = 0;

	virtual URIList *GetChild(const TomboURI *pFolder, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose) = 0;

	virtual BOOL RequestAllocateURI(const TomboURI *pBaseURI, LPCTSTR pText, TString *pHeadLine, TomboURI *pURI, const TomboURI *pTemplateURI) = 0;

	virtual BOOL GetAttribute(const TomboURI *pURI, NoteAttribute *pAttribute) = 0;
	virtual BOOL SetAttribute(const TomboURI *pURI, const NoteAttribute *pAttribute) = 0;
	virtual BOOL GetNoteAttribute(const TomboURI *pURI, UINT64 *pLastUpdate, UINT64 *pCreateDate, UINT64 *pFileSize) = 0;

	virtual LPTSTR GetNoteData(const TomboURI *pURI) = 0;
	virtual LPBYTE GetNoteDataNative(const TomboURI *pURI, LPDWORD pSize) = 0;

	virtual BOOL ExecuteAssoc(const TomboURI *pURI, ExeAppType nType) = 0;
	virtual BOOL MakeFolder(const TomboURI *pURI, LPCTSTR pFolderName) = 0;

	////////////////////////////
	// helper function

	BOOL IsEncrypted(const TomboURI *pURI);
};

///////////////////////////////////////////////////////////////////
// Repository implimentation which stores notes to local file
///////////////////////////////////////////////////////////////////

class LocalFileRepository : public RepositoryImpl {
	LPTSTR pTopDir;

	BOOL bKeepTitle;
	BOOL bKeepCaret;
	BOOL bSafeFileName;

protected:
	//////////////////////////////////////////
	// Save notes and its subroutines

	BOOL Save(const TomboURI *pCurrentURI, LPCTSTR pMemo, TomboURI *pNewURI, TString *pHeadLine);

	BOOL SaveIfHeadLineIsNotChanged(MemoNote *pNote, LPCTSTR pMemo, LPCTSTR pOrigFile);
	BOOL SaveIfHeadLineIsChanged(MemoNote *pNote, LPCTSTR pMemo,
								 LPCTSTR pOrigFile, LPCTSTR pHeadLine, 
								 TString *pNewHeadLine);

	////////////////////////////////////////
	// Encryption/Decryption subroutines
	BOOL EncryptLeaf(const TomboURI *pCurrentURI, URIOption *pOption);
	BOOL DecryptLeaf(const TomboURI *pCurrentURI, URIOption *pOption);
	BOOL EnDecryptFolder(const TomboURI *pCurrentURI, URIOption *pOption);

	TomboURI *DoEncryptFile(const TomboURI *pOldURI, MemoNote *pNote, TString *pHeadLine);
	BOOL NegotiateNewName(LPCTSTR pMemoPath, LPCTSTR pText, 
							 LPCTSTR pMemoDir,
							 TString *pFullPath, LPCTSTR *ppNotePath, TString *pHeadLine);

	////////////////////////////////////////
	// others
	BOOL GetSafeFileName(const TString *pBasePath, TString *pNewName);
	BOOL GetTDTFullPath(const TomboURI *pURI, TString *pTdtName);

public:
	LocalFileRepository();
	virtual ~LocalFileRepository();

	BOOL Init(LPCTSTR pRepName, LPCTSTR pDispName, LPCTSTR pTopDir, 
				BOOL bKeepTitle, BOOL bKeepCaret, BOOL bSafeFileName);

	// temporary method for refactoring
	BOOL SetTopDir(LPCTSTR pDir);

	///////////////////////////////
	// impliment RepositoryImpl methods

	RepositoryImpl *Clone();
	LPTSTR GetXMLSaveString();

	BOOL Update(const TomboURI *pCurrentURI, LPCTSTR pData, TomboURI *pNewURI, TString *pNewHeadLine);
	BOOL Delete(const TomboURI *pURI, URIOption *pOption);
	BOOL Copy(const TomboURI *pCopyFrom, const TomboURI *pCopyTo, URIOption *pOption);
	BOOL Move(const TomboURI *pMoveFrom, const TomboURI *pMoveTo, URIOption *pOption);

	BOOL ChangeHeadLine(const TomboURI *pURI, LPCTSTR pReqNewHeadLine, URIOption *pOption);

	BOOL GetHeadLine(const TomboURI *pURI, TString *pHeadLine);
	BOOL GetPhysicalPath(const TomboURI *pURI, TString *pFullPath);

	BOOL GetOption(const TomboURI *pURI, URIOption *pOption) const;

	// change note status
	BOOL SetOption(const TomboURI *pCurrentURI, URIOption *pOption);

	DWORD GetList(const TomboURI *pFolder, DirList *pList, BOOL bSkipEncrypt, BOOL bLooseDecrypt);
	URIList *GetChild(const TomboURI *pFolder, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose);

	BOOL RequestAllocateURI(const TomboURI *pBaseURI, LPCTSTR pText, TString *pHeadLine, TomboURI *pURI, const TomboURI *pTemplateURI);

	BOOL GetAttribute(const TomboURI *pURI, NoteAttribute *pAttribute);
	BOOL SetAttribute(const TomboURI *pURI, const NoteAttribute *pAttribute);
	BOOL GetNoteAttribute(const TomboURI *pURI, UINT64 *pLastUpdate, UINT64 *pCreateDate, UINT64 *pFileSize);

	LPTSTR GetNoteData(const TomboURI *pURI);
	LPBYTE GetNoteDataNative(const TomboURI *pURI, LPDWORD pSize);

	BOOL ExecuteAssoc(const TomboURI *pURI, ExeAppType nType);
	BOOL MakeFolder(const TomboURI *pURI, LPCTSTR pFolderName);

	BOOL GetKeepTitle() { return bKeepTitle; }
	void SetKeepTitle(BOOL b) { bKeepTitle = b; }
	BOOL GetKeepCaret() { return bKeepCaret; }
	void SetKeepCaret(BOOL b) { bKeepCaret = b; }
	BOOL GetSafeFileName() { return bSafeFileName; }
	void SetSafeFileName(BOOL b) { bSafeFileName = b; }
};

///////////////////////////////////////////////////////////////////
// virtual folder repository implimentation
///////////////////////////////////////////////////////////////////
// in this time, VFolderRepository is psudo implementation.

class VFolderRepository : public RepositoryImpl {
public:
	VFolderRepository();
	virtual ~VFolderRepository();
	BOOL Init(LPCTSTR pRepName, LPCTSTR pDispName);

	RepositoryImpl *Clone();
	LPTSTR GetXMLSaveString();

	BOOL Update(const TomboURI *pCurrentURI, LPCTSTR pData, TomboURI *pNewURI, TString *pNewHeadLine);
	BOOL Delete(const TomboURI *pURI, URIOption *pOption);
	BOOL Copy(const TomboURI *pCopyFrom, const TomboURI *pCopyTo, URIOption *pOption);
	BOOL Move(const TomboURI *pMoveFrom, const TomboURI *pMoveTo, URIOption *pOption);

	BOOL ChangeHeadLine(const TomboURI *pURI, LPCTSTR pReqNewHeadLine, URIOption *pOption);

	BOOL GetHeadLine(const TomboURI *pURI, TString *pHeadLine);

	BOOL GetOption(const TomboURI *pURI, URIOption *pOption) const;
	BOOL SetOption(const TomboURI *pCurrentURI, URIOption *pOption);

	BOOL GetPhysicalPath(const TomboURI *pURI, TString *pFullPath);

	URIList *GetChild(const TomboURI *pFolder, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose);

	BOOL RequestAllocateURI(const TomboURI *pBaseURI, LPCTSTR pText, TString *pHeadLine, TomboURI *pURI, const TomboURI *pTemplateURI);

	BOOL GetAttribute(const TomboURI *pURI, NoteAttribute *pAttribute);
	BOOL SetAttribute(const TomboURI *pURI, const NoteAttribute *pAttribute);
	BOOL GetNoteAttribute(const TomboURI *pURI, UINT64 *pLastUpdate, UINT64 *pCreateDate, UINT64 *pFileSize);

	LPTSTR GetNoteData(const TomboURI *pURI);
	LPBYTE GetNoteDataNative(const TomboURI *pURI, LPDWORD pSize);

	BOOL ExecuteAssoc(const TomboURI *pURI, ExeAppType nType);
	BOOL MakeFolder(const TomboURI *pURI, LPCTSTR pFolderName);
};


#endif