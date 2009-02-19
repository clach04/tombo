#ifndef REPOSITORY_H
#define REPOSITORY_H

class TString;
class TomboURI;
class RepositoryImpl;
class URIOption;
class URIList;

class NoteAttribute;

#include "VarBuffer.h"

/////////////////////////////////////////
// Option flag definitions
/////////////////////////////////////////

#define NOTE_OPTIONMASK_ENCRYPTED  1
#define NOTE_OPTIONMASK_SAFEFILE   2
#define NOTE_OPTIONMASK_VALID      4
#define NOTE_OPTIONMASK_ICON       8

/////////////////////////////////////////
// Custom error code
/////////////////////////////////////////

#define ERROR_TOMBO_REP_CODE_BASE_ERROR (0x24000000)
#define ERROR_TOMBO_REP_CODE_BASE_WARN  (0x22000000)
#define ERROR_TOMBO_REP_CODE_BASE_INFO  (0x21000000)

#define ERROR_TOMBO_E_INVALIDURI         (ERROR_TOMBO_REP_CODE_BASE_ERROR + 1)
#define ERROR_TOMBO_E_SOME_ERROR_OCCURED (ERROR_TOMBO_REP_CODE_BASE_ERROR + 2)
#define ERROR_TOMBO_E_RMFILE_FAILED      (ERROR_TOMBO_REP_CODE_BASE_ERROR + 3)
#define ERROR_TOMBO_E_RMDIR_FAILED       (ERROR_TOMBO_REP_CODE_BASE_ERROR + 4)

#define ERROR_TOMBO_W_DELETEOLD_FAILED          (ERROR_TOMBO_REP_CODE_BASE_WARN + 1)
#define ERROR_TOMBO_W_OTHERFILE_EXISTS          (ERROR_TOMBO_REP_CODE_BASE_ERROR + 2)
#define ERROR_TOMBO_W_OPERATION_NOT_PERMITTED   (ERROR_TOMBO_REP_CODE_BASE_ERROR + 3)

#define ERROR_TOMBO_I_OPERATION_NOT_PERFORMED (ERROR_TOMBO_REP_CODE_BASE_INFO + 1)
#define ERROR_TOMBO_I_GET_PASSWORD_CANCELED   (ERROR_TOMBO_REP_CODE_BASE_INFO + 2)

/////////////////////////////////////////
// GetList results
/////////////////////////////////////////

#define TOMBO_REPO_GETLIST_FAIL 0
#define TOMBO_REPO_GETLIST_SUCCESS 1
#define TOMBO_REPO_GETLIST_PARTIAL 2

/////////////////////////////////////////
// Sub repository type 
/////////////////////////////////////////

#define TOMBO_REPO_SUBREPO_TYPE_INVALID		0
#define TOMBO_REPO_SUBREPO_TYPE_LOCALFILE	1
#define TOMBO_REPO_SUBREPO_TYPE_VFOLDER		2

//////////////////////////////////////////////////////////////
// Repository enumeration interface
//////////////////////////////////////////////////////////////

class IEnumRepository {
public:
	virtual URIList *GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose) = 0;
	virtual BOOL GetOption(const TomboURI *pURI, URIOption *pOption) = 0;
	virtual BOOL GetHeadLine(const TomboURI *pURI, TString *pHeadLine) = 0;
};

//////////////////////////////////////////////////////////////
// Repository
//////////////////////////////////////////////////////////////
// Repository is an abstraction of TOMBO's notes/folder tree.

class Repository : public IEnumRepository {

protected:
	RepositoryImpl *pDefaultImpl;
	TVector<RepositoryImpl*> vSubRepository;

	// Get real physical path from URI.
	//
	// This method may be not supported by some RepositoryImpl type.
	BOOL GetPhysicalPath(const TomboURI *pURI, TString *pFullPath);

	// get sub repository reference assciated with the URI
	RepositoryImpl *GetAssocSubRepository(const TomboURI *pURI);
public:

	Repository();
	~Repository();

	BOOL Init();
	BOOL ClearSubRepository();

	// Add sub repository. pImpl instance is controled under Repository so do not delete pImpl by caller.
	BOOL AddSubRepository(RepositoryImpl *pImpl);

	////////////////////////////
	// Note/folder operations

	BOOL Update(const TomboURI *pCurrentURI, LPCTSTR pData, TomboURI *pNewURI, TString *pNewHeadLine);
	BOOL GetHeadLine(const TomboURI *pURI, TString *pHeadLine);

	BOOL Delete(const TomboURI *pURI, URIOption *pOption);
	BOOL Copy(const TomboURI *pCopyFrom, const TomboURI *pCopyTo, URIOption *pOption);
	BOOL Move(const TomboURI *pMoveFrom, const TomboURI *pMoveTo, URIOption *pOption);

	BOOL ExecuteAssoc(const TomboURI *pURI, ExeAppType nType);
	BOOL MakeFolder(const TomboURI *pParent, LPCTSTR pFolderName);

	// Rename headline
	// Repository data is updated if necessary.
	BOOL ChangeHeadLine(const TomboURI *pURI, LPCTSTR pReqNewHeadLine, URIOption *pOption);

	LPTSTR GetNoteData(const TomboURI *pURI);
	LPBYTE GetNoteDataNative(const TomboURI *pURI, LPDWORD pSize);

	// Get URI the a item attached to.
	// ex. :  tombo://repo/a/b/c/ -> tombo://repo/a/b/c/
	//     :  tombo://repo/a/b/c/d.txt -> tombo://repo/a/b/c/
	BOOL GetAttachURI(const TomboURI *pBase, TomboURI *pAttached);

	// for use search only.
	BOOL GetFileName(const TomboURI *pURI, TString *pName);

	// Get notes under the url.
	URIList *GetChild(const TomboURI *pFolderURI, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose);

	// Decide URI
	BOOL RequestAllocateURI(const TomboURI *pBaseURI, LPCTSTR pText, TString *pHeadLine, TomboURI *pURI, const TomboURI *pTemplateURI);

	////////////////////////////
	// File attribute functions

	// get option from URI
	//
	// if NOTE_OPTIONMASK_VALID is set by pOption, bValid and bFolder is effective.
	// if NOTE_OPTIONMASK_ENCRYPTED is set, bEncrypt is effective.
	// if NOTE_OPTIONMASK_SAFEFILE is set, bSafeFile is effective.
	BOOL GetOption(const TomboURI *pURI, URIOption *pOption);

	// set option from URI
	//
	// if NOTE_OPTIONMASK_ENCRYPTED is set and bEncrypt is TRUE and bSafeFile is FALSE, encrypt to the URI by normal mode.
	// if NOTE_OPTIONMASK_ENCRYPTED is set and bEncrypt is TRUE and bSafeFile is TRUE, encrypt to the URI by safefile mode.
	// if NOTE_OPTIONMASK_ENCRYPTED is set and bEncrypt is FALSE, decrypt to the URI.
	//
	// In current implimentation, change normal mode <-> safe file mode is not supported.
	BOOL SetOption(const TomboURI *pCurrentURI, URIOption *pOption);

	BOOL GetAttribute(const TomboURI *pURI, NoteAttribute *pAttribute);
	BOOL SetAttribute(const TomboURI *pURI, const NoteAttribute *pAttribute);
	// get note attributes.
	// TODO:  merged to GetAttribute.
	BOOL GetNoteAttribute(const TomboURI *pURI, UINT64 *pLastUpdate, UINT64 *pCreateDate, UINT64 *pFileSize);

	/////////////////////////////
	// helper function

	// Check the URI data is encrypted
	BOOL IsEncrypted(const TomboURI *pURI);

	/////////////////////////////
	// sub repository IF
	DWORD GetNumOfSubRepository();
	DWORD GetSubRepositoryType(DWORD nIndex);
	LPCTSTR GetSubRepositoryName(DWORD nIndex);
	const TomboURI *GetSubRepositoryRootURI(DWORD nIndex);
	LPTSTR GetSubRepoXMLSaveString(DWORD nIndex);

	static RepositoryImpl *CreateSubRepo(LPCWSTR pName, const WCHAR **atts);
};

//////////////////////////////////////////////////////////////
// URI option
//////////////////////////////////////////////////////////////

class URIOption {
public:
	URIOption(DWORD flg = 0) : nFlg(flg), pNewURI(NULL), pNewHeadLine(NULL) {}
	~URIOption() { delete pNewURI; delete pNewHeadLine; }

	// request section
	DWORD nFlg;
	BOOL bEncrypt;
	BOOL bSafeFileName;
	BOOL bValid;
	BOOL bFolder;

	int iIcon;

	// result info section
	// if these value is not set by NULL, delete when NoteOption is deleted.
	TString *pNewHeadLine;
	TomboURI *pNewURI;
};

//////////////////////////////////////////////////////////////
// File Attribute
//////////////////////////////////////////////////////////////

class NoteAttribute {
public:
	DWORD nCursorPos;
	BOOL bReadOnly;
};

//////////////////////////////////////////////////////////////
// Global definitions
//////////////////////////////////////////////////////////////

extern Repository g_Repository;

#endif