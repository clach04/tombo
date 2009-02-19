#include <windows.h>
#include <tchar.h>

#include "Tombo.h"
#include "Message.h"
#include "UniConv.h"
#include "TString.h"
#include "TomboURI.h"
#include "Property.h"
#include "Repository.h"
#include "RepositoryImpl.h"

#include "MemoNote.h"
#include "AutoPtr.h"
#include "PasswordManager.h"
#include "MemoInfo.h"

#include "DirList.h"
#include "URIScanner.h"
#include "DirectoryScanner.h"
#include "MemoFolder.h"

#if defined(PLATFORM_BE500)
#include <CoShellapi.h>
#endif


/////////////////////////////////////////
// static funcs
/////////////////////////////////////////

#define DEFAULT_HEADLINE MSG_DEFAULT_HEADLINE

static int ChopFileNumberLen(LPTSTR pHeadLine);
static BOOL IsFileExist(LPCTSTR pFileName);

/////////////////////////////////////////
// Repository ctor & dtor, initializer
/////////////////////////////////////////

RepositoryImpl::RepositoryImpl() : pRepName(NULL), pDispName(NULL), pRootURI(NULL) {}
RepositoryImpl::~RepositoryImpl() 
{
	delete [] pRepName;
	delete [] pDispName;
	delete pRootURI;
}

BOOL RepositoryImpl::Init(LPCTSTR rep, LPCTSTR disp, DWORD ntype)
{
	pRepName = StringDup(rep);
	pDispName = StringDup(disp);
	nRepNameLen = _tcslen(pRepName);
	
	nRepType = ntype;

	TString sStrURI;
	if (!sStrURI.Join(TEXT("tombo://"), rep, TEXT("/"))) return FALSE;
	pRootURI = new TomboURI();
	if (!pRootURI->Init(sStrURI.Get())) return FALSE;

	return (pRepName != NULL) && (pDispName != NULL);
}

/////////////////////////////////////////
// Is the note encrypted?
/////////////////////////////////////////

BOOL RepositoryImpl::IsEncrypted(const TomboURI *pURI)
{
	URIOption opt(NOTE_OPTIONMASK_ENCRYPTED);
	GetOption(pURI, &opt);
	return opt.bEncrypt;
}

/////////////////////////////////////////
// ctor & dtor, initializer
/////////////////////////////////////////

LocalFileRepository::LocalFileRepository() : pTopDir(NULL)
{
}

LocalFileRepository::~LocalFileRepository()
{
	delete[] pTopDir;
}

BOOL LocalFileRepository::Init(LPCTSTR rep, LPCTSTR disp, LPCTSTR dir, 
							   BOOL title, BOOL caret, BOOL safefile) 
{
	if (!RepositoryImpl::Init(rep, disp, TOMBO_REPO_SUBREPO_TYPE_LOCALFILE)) return FALSE;

	pTopDir = StringDup(dir);
	bKeepTitle = title;
	bKeepCaret = caret;
	bSafeFileName = safefile;
	return TRUE;
}

RepositoryImpl *LocalFileRepository::Clone()
{
	LocalFileRepository *pImpl = new LocalFileRepository();
	if (pImpl == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	if (!pImpl->Init(GetRepositoryName(), GetDisplayName(), pTopDir, bKeepTitle, bKeepCaret, bSafeFileName)) {
		delete[] pImpl;
		return NULL;
	}
	return pImpl;
}

LPTSTR LocalFileRepository::GetXMLSaveString()
{
	TString sXMLStr;
	if (!sXMLStr.Join(TEXT("      <localfile name=\""), GetRepositoryName(), TEXT("\" dispname=\""), GetDisplayName())) return NULL;
	if (!sXMLStr.StrCat(TEXT("\" path=\""))) return NULL;
	if (!sXMLStr.StrCat(pTopDir)) return NULL;
	if (!sXMLStr.StrCat(TEXT("\" keeptitle=\""))) return NULL;
	if (!sXMLStr.StrCat(bKeepTitle ? TEXT("1") : TEXT("0"))) return NULL;
	if (!sXMLStr.StrCat(TEXT("\" keepcaret=\""))) return NULL;
	if (!sXMLStr.StrCat(bKeepCaret ? TEXT("1") : TEXT("0"))) return NULL;
	if (!sXMLStr.StrCat(TEXT("\" safefilename=\""))) return NULL;
	if (!sXMLStr.StrCat(bSafeFileName ? TEXT("1") : TEXT("0"))) return NULL;
	if (!sXMLStr.StrCat(TEXT("\"/>\n"))) return NULL;
	return StringDup(sXMLStr.Get());
}

BOOL LocalFileRepository::SetTopDir(LPCTSTR pDir)
{
	delete [] pTopDir;
	pTopDir = StringDup(pDir);
	return pTopDir != NULL;
}

/////////////////////////////////////////
// get attribute file path from notes URI 
/////////////////////////////////////////

BOOL LocalFileRepository::GetTDTFullPath(const TomboURI *pURI, TString *pTdtName)
{
	TString sPath;
	if (!pURI->GetFilePath(&sPath)) return FALSE;
	return pTdtName->Join(pTopDir, TEXT("\\"), sPath.Get(), TEXT(".tdt"));
}

/////////////////////////////////////////
// Update note
/////////////////////////////////////////

BOOL LocalFileRepository::Update(const TomboURI *pCurrentURI, LPCTSTR pData,
								 TomboURI *pNewURI, TString *pNewHeadLine)
{
	// Save note data
	if (!Save(pCurrentURI, pData, pNewURI, pNewHeadLine)) {
		return FALSE;
	}
	
	// if tdt exists, move one
	TString sCurrentTdtPath, sNewTdtPath;
	if (!GetTDTFullPath(pCurrentURI, &sCurrentTdtPath)) return FALSE;
	if (!GetTDTFullPath(pNewURI, &sNewTdtPath)) return FALSE;

	MemoInfo mi(pTopDir);
	mi.RenameInfo(sCurrentTdtPath.Get(), sNewTdtPath.Get());
	return TRUE;
}

/////////////////////////////////////////
// Update's subfunction
/////////////////////////////////////////
BOOL LocalFileRepository::Save(const TomboURI *pCurrentURI, LPCTSTR pMemo, 
							   TomboURI *pNewURI, TString *pNewHeadLine)
{
	MemoNote *pNote = MemoNote::MemoNoteFactory(pCurrentURI);
	if (pNote == NULL) return FALSE;
	AutoPointer<MemoNote> apNote(pNote);

	TString sOrigFile;
	if (!GetPhysicalPath(pCurrentURI, &sOrigFile)) return FALSE;

	// get current headline from path
	if (!GetHeadLine(pCurrentURI, pNewHeadLine)) return FALSE;

	URIOption opt(NOTE_OPTIONMASK_SAFEFILE);
	if (!GetOption(pCurrentURI, &opt)) return FALSE;

	if (bKeepTitle || opt.bSafeFileName) {
		if (!SaveIfHeadLineIsNotChanged(pNote, pMemo, sOrigFile.Get())) return FALSE;

		// URI is not changed.
		return pNewURI->Init(*pCurrentURI);

	} else {
		// Get new headline from memo text
		TString sHeadLine;
		if (!MemoNote::GetHeadLineFromMemoText(pMemo, &sHeadLine)) return FALSE;

		DWORD nH = ChopFileNumberLen(pNewHeadLine->Get());
		DWORD nH2 = ChopFileNumberLen(sHeadLine.Get());

		BOOL bResult;
		// check headline has changed.
		if (nH == nH2 && _tcsncmp(pNewHeadLine->Get(), sHeadLine.Get(), nH) == 0) {
			bResult = SaveIfHeadLineIsNotChanged(pNote, pMemo, sOrigFile.Get());
		} else {
			bResult = SaveIfHeadLineIsChanged(pNote, pMemo, sOrigFile.Get(), 
										 sHeadLine.Get(), pNewHeadLine);
		}
		if (bResult) {
			bResult = pNote->GetURI(GetRepositoryName(), pNewURI);
		}
		return bResult;
	}
}

BOOL LocalFileRepository::SaveIfHeadLineIsChanged(
	MemoNote *pNote, LPCTSTR pMemo, LPCTSTR pOrigFile, LPCTSTR pHeadLine, 
	TString *pNewHeadLine)
{
		LPCTSTR pNotePath = pNote->MemoPath();

		// changed.
		TString sMemoDir;
		TString sNewFile;

		if (!sMemoDir.GetDirectoryPath(pNotePath)) return FALSE;
		if (!MemoNote::GetHeadLinePath(pTopDir, sMemoDir.Get(), pHeadLine, pNote->GetExtension(),
							 &sNewFile, &pNotePath, pNewHeadLine)) return FALSE;

		// write data		
		BOOL bResult = pNote->SaveDataT(g_pPassManager, pMemo, sNewFile.Get());
		if (bResult) {
			// delete original file
			DeleteFile(pOrigFile);

			// Additionally, rename memo info(*.tdt) file.
			if (bKeepCaret) {
				MemoInfo mi(pTopDir);
				mi.RenameInfo(pOrigFile, sNewFile.Get());
			}

			// Update note's file path information. 
			if (!pNote->SetMemoPath(pNotePath)) return FALSE;

			return TRUE;

		} else {
			// rollback (delete new writing file)
			DeleteFile(sNewFile.Get());
			return FALSE;
		}
}

BOOL LocalFileRepository::SaveIfHeadLineIsNotChanged(MemoNote *pNote, LPCTSTR pMemo, LPCTSTR pOrigFile)
{
	// Generate backup file name
	TString sBackupFile;
	if (!sBackupFile.Join(pOrigFile, TEXT(".tmp"))) return FALSE;
	// Backup(copy) original file
	//
	// Because ActiveSync can't treat Move&Write, backup operation uses not move but copy
	if (!CopyFile(pOrigFile, sBackupFile.Get(), FALSE)) {
		// if new file, copy are failed but it is OK.
		if (GetLastError() != ERROR_FILE_NOT_FOUND) return FALSE;
	}

	// Save to file
	if (!pNote->SaveDataT(g_pPassManager, pMemo, pOrigFile)) {
		// When save failed, try to rollback original file.
		DeleteFile(pOrigFile);
		MoveFile(sBackupFile.Get(), pOrigFile);
		return FALSE;
	}
	// remove backup file
	DeleteFile(sBackupFile.Get());
	return TRUE;
}

////////////////////////////////////////////////////////
// remove "(n)" from headline
////////////////////////////////////////////////////////

static int ChopFileNumberLen(LPTSTR pHeadLine)
{
	if (*pHeadLine == TEXT('\0')) return 0;

	DWORD n = _tcslen(pHeadLine);
	LPTSTR p = pHeadLine + n - 1;
	if (*p != TEXT(')')) return n;
	p--;
	while(p >= pHeadLine) {
		if (*p == TEXT('(')) {
			return p - pHeadLine;
		}
		if (*p < TEXT('0') || *p > TEXT('9')) break;
		p--;
	}
	return n;
}

/////////////////////////////////////////////
// file existance check
/////////////////////////////////////////////

static BOOL IsFileExist(LPCTSTR pFileName)
{
	HANDLE hFile = CreateFile(pFileName, GENERIC_READ, FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	} else {
		CloseHandle(hFile);
		return TRUE;
	}
}

/////////////////////////////////////////
// Get Headline string
/////////////////////////////////////////
// This method ask password if it need.

BOOL LocalFileRepository::GetHeadLine(const TomboURI *pURI, TString *pHeadLine)
{
	BOOL bIsLeaf = pURI->IsLeaf();

	// check if the uri is safename
	if (bIsLeaf) {
		LPCTSTR pURIstr = pURI->GetFullURI();
		DWORD n = _tcslen(pURIstr);
		if (n > 4 && _tcscmp(pURIstr + n - 4, TEXT(".chs")) == 0) {
			TString sPath;
			if (!pURI->GetFilePath(&sPath)) return FALSE;
			CryptedMemoNote cn;
			cn.Init(sPath.Get());
			LPTSTR p = cn.GetMemoBody(pTopDir, g_pPassManager);
			if (p == NULL) return FALSE;
			SecureBufferAutoPointerT ap(p);
			if (!MemoNote::GetHeadLineFromMemoText(p, pHeadLine)) return FALSE;
			return TRUE;
		}
	}

	if (!pURI->GetBaseName(pHeadLine)) return FALSE;

	if (_tcslen(pHeadLine->Get()) == 0) {
		// root
		return pHeadLine->Set(TEXT("[root]"));
	}

	if (bIsLeaf) {
		LPTSTR p = pHeadLine->Get();
		DWORD n = _tcslen(p);
		if (n > 4) {
			*(p + n - 4) = TEXT('\0');
		}
	}
	return TRUE;
}

/////////////////////////////////////////
// Get option information
/////////////////////////////////////////

BOOL LocalFileRepository::GetOption(const TomboURI *pURI, URIOption *pOption) const
{
	if ((pOption->nFlg & NOTE_OPTIONMASK_VALID) || (pOption->nFlg & NOTE_OPTIONMASK_ICON)) {
		LPCTSTR p = pURI->GetFullURI();
		DWORD len = _tcslen(p);
		if (_tcscmp(p + len - 1, TEXT("/")) == 0) {
			// folder
			pOption->bValid = TRUE;
			pOption->bFolder = TRUE;
		} else {
			// file
			p = p + len - 4;
			if (_tcsicmp(p, TEXT(".txt")) == 0 ||
				_tcsicmp(p, TEXT(".chi")) == 0 || 
				_tcsicmp(p, TEXT(".chs")) == 0) {
				pOption->bValid = TRUE;
				pOption->bFolder = FALSE;
			} else {
				pOption->bValid = FALSE;
				return TRUE;
			}
		}
	}
	if ((pOption->nFlg & NOTE_OPTIONMASK_ENCRYPTED) || (pOption->nFlg & NOTE_OPTIONMASK_ICON)) {
		LPCTSTR p = pURI->GetFullURI();
		DWORD n = _tcslen(p);
		if (n > 4) {
			if (_tcsicmp(p + n - 4, TEXT(".chi")) == 0 ||
				_tcsicmp(p + n - 4, TEXT(".chs")) == 0) {
				pOption->bEncrypt = TRUE;
			} else {
				pOption->bEncrypt = FALSE;
			}
		}
	}

	if (pOption->nFlg & NOTE_OPTIONMASK_SAFEFILE) {
		LPCTSTR p = pURI->GetFullURI();
		if (_tcslen(p) > 4 && _tcscmp(p + _tcslen(p) - 4, TEXT(".chs")) == 0) {
			pOption->bSafeFileName = TRUE;
		} else {
			pOption->bSafeFileName = FALSE;
		}
	}
	if (pOption->nFlg & NOTE_OPTIONMASK_ICON) {
		if (pOption->bFolder) {
			pOption->iIcon = IMG_FOLDER;
		} else {
			if (pOption->bEncrypt) {
				pOption->iIcon = IMG_ARTICLE_ENCRYPTED;
			} else {
				pOption->iIcon = IMG_ARTICLE;
			}
		}
	}
	return TRUE;
}

/////////////////////////////////////////
// Set option information
/////////////////////////////////////////

BOOL LocalFileRepository::SetOption(const TomboURI *pCurrentURI, URIOption *pOption)
{
	if (pOption->nFlg & NOTE_OPTIONMASK_ENCRYPTED) {
		if (pCurrentURI->IsLeaf()) {
			// set option to file
			if (pOption->bEncrypt) {
				// encrypt
				return EncryptLeaf(pCurrentURI, pOption);
			} else {
				// decrypt
				return DecryptLeaf(pCurrentURI, pOption);
			}
		} else {
			// set option to folder
			return EnDecryptFolder(pCurrentURI, pOption);
		}
	}
	return TRUE;
}

/////////////////////////////////////////
// Get safe file name
/////////////////////////////////////////

BOOL LocalFileRepository::GetSafeFileName(const TString *pBasePath, TString *pNewName)
{
	TString s;

	DWORD l = _tcslen(pBasePath->Get()) + 20 + 1;
	if (!s.Alloc(l)) return FALSE;
	_tcscpy(s.Get(), pBasePath->Get());
	_tcscat(s.Get(), TEXT("0000000000000000.chs"));

	LPTSTR pFileNamePart = s.Get() + _tcslen(pBasePath->Get());
	int nw;
	do {
		// generate 10digit random number
		nw = rand() % 10000;
		wsprintf(pFileNamePart, TEXT("%04d"), nw);
		nw = rand() % 10000;
		wsprintf(pFileNamePart + 4, TEXT("%04d"), nw);
		nw = rand() % 10000;
		wsprintf(pFileNamePart + 8, TEXT("%04d"), nw);
		nw = rand() % 10000;
		wsprintf(pFileNamePart + 12, TEXT("%04d"), nw);
		_tcscpy(pFileNamePart + 16, TEXT(".chs"));
	} while(IsFileExist(s.Get())); // if same name exists, retry it

	if (!pNewName->Set(pFileNamePart)) return FALSE;

	return TRUE;
}

/////////////////////////////////////////
// Decide new allocated filename
/////////////////////////////////////////

BOOL LocalFileRepository::NegotiateNewName(LPCTSTR pMemoPath, LPCTSTR pText, LPCTSTR pMemoDir,
							 TString *pFullPath, LPCTSTR *ppNotePath, TString *pNewHeadLine)
{
	TString sHeadLine;

	if (bSafeFileName) {
		TString sBase;
		TString sNewName;
		if (!sBase.Join(pTopDir, TEXT("\\"), pMemoDir)) return FALSE;
		if (!GetSafeFileName(&sBase, &sNewName)) return FALSE;

		if (!pFullPath->Join(sBase.Get(), sNewName.Get())) return FALSE;

		*ppNotePath = pFullPath->Get() + _tcslen(pTopDir) + 1;
		if (!MemoNote::GetHeadLineFromMemoText(pText, pNewHeadLine)) return FALSE;
	} else {
		if (bKeepTitle) {
			if (!MemoNote::GetHeadLineFromFilePath(pMemoPath, &sHeadLine)) return FALSE;
		} else {
			if (!MemoNote::GetHeadLineFromMemoText(pText, &sHeadLine)) return FALSE;
		}
	
		if (!MemoNote::GetHeadLinePath(pTopDir, pMemoDir, sHeadLine.Get(), TEXT(".chi"), 
								pFullPath, ppNotePath, pNewHeadLine)) {
			return FALSE;
		}
	}
	return TRUE;
}

//////////////////////////////
// Encrypt file
//
// assume pCurURI is Leaf.

TomboURI *LocalFileRepository::DoEncryptFile(const TomboURI *pOldURI, MemoNote *pNote, TString *pHeadLine)
{
	TString sMemoDir;
	if (!sMemoDir.GetDirectoryPath(pNote->MemoPath())) return NULL;

	// Get plain memo data from file
	LPTSTR pText = pNote->GetMemoBody(pTopDir, g_pPassManager);
	if (pText == NULL) return NULL;
	SecureBufferAutoPointerT ap1(pText);

	TString sFullPath;
	LPCTSTR pNotePath;

	// Decide new name
	if (!NegotiateNewName(pNote->MemoPath(), pText, sMemoDir.Get(), 
						&sFullPath, &pNotePath, pHeadLine)) return NULL;

	// Create new CyrptedMemoNote instance
	CryptedMemoNote *p = new CryptedMemoNote();

	if (!p->Init(pNotePath)) return NULL;
	AutoPointer<CryptedMemoNote> ap2(p);

	// Save memo
	if (!p->SaveDataT(g_pPassManager, pText, sFullPath.Get())) return NULL;

	// rename TDT
	TString sOrigTDT;
	TString sNewTDT;
	if (GetTDTFullPath(pOldURI, &sOrigTDT) &&
		sNewTDT.Join(sFullPath.Get(), TEXT(".tdt"))) {
		DeleteFile(sNewTDT.Get());
		MoveFile(sOrigTDT.Get(), sNewTDT.Get());
	}

	// generate new URI
	TomboURI *pURI = new TomboURI();
	if (pURI == NULL || !p->GetURI(GetRepositoryName(), pURI)) return NULL;

	return pURI;
}

BOOL LocalFileRepository::EncryptLeaf(const TomboURI *pPlainURI, URIOption *pOption)
{
	MemoNote *pPlain = MemoNote::MemoNoteFactory(pPlainURI);
	AutoPointer<MemoNote> ap(pPlain);

	pOption->pNewHeadLine = new TString();
	if (pOption->pNewHeadLine == NULL) return FALSE;

	pOption->pNewURI = DoEncryptFile(pPlainURI, pPlain, pOption->pNewHeadLine);
	if (pOption->pNewURI == NULL) return FALSE;

	if (!pPlain->DeleteMemoData(pTopDir)) {
		SetLastError(ERROR_TOMBO_W_DELETEOLD_FAILED);
		return FALSE;
	}

	return TRUE;
}

BOOL LocalFileRepository::DecryptLeaf(const TomboURI *pCurrentURI, URIOption *pOption)
{
	BOOL b;
	MemoNote *pCur = MemoNote::MemoNoteFactory(pCurrentURI);
	AutoPointer<MemoNote> ap(pCur);

	if ((pOption->pNewHeadLine = new TString()) == NULL) return FALSE;

	MemoNote *p = pCur->Decrypt(pTopDir, g_pPassManager, pOption->pNewHeadLine, &b);
	if (p == NULL) return FALSE;
	AutoPointer<MemoNote> ap2(p);

	pOption->pNewURI = new TomboURI();
	if (pOption->pNewURI == NULL) return FALSE;
	if (!p->GetURI(GetRepositoryName(), pOption->pNewURI)) return FALSE;

	// rename TDT
	TString sOrigTDT;
	TString sNewTDT;
	if (GetTDTFullPath(pCurrentURI, &sOrigTDT) &&
		GetTDTFullPath(pOption->pNewURI, &sNewTDT)) {
		DeleteFile(sNewTDT.Get());
		MoveFile(sOrigTDT.Get(), sNewTDT.Get());
	}

	if (!pCur->DeleteMemoData(pTopDir)) {
		SetLastError(ERROR_TOMBO_W_DELETEOLD_FAILED);
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////
// Encrypt/Decrypt folder
/////////////////////////////////////////

BOOL LocalFileRepository::EnDecryptFolder(const TomboURI *pCurrentURI, URIOption *pOption)
{
	TString sPath;
	if (!GetPhysicalPath(pCurrentURI, &sPath)) return FALSE;

	DSEncrypt fc;
	if (!fc.Init(pTopDir, sPath.Get(), pCurrentURI->GetFullURI(), pOption->bEncrypt)) return FALSE;

	// ask password
	BOOL bCancel;
	const char *pPass = g_pPassManager->Password(&bCancel, pOption->bEncrypt);
	if (pPass == NULL) {
		SetLastError(ERROR_TOMBO_I_GET_PASSWORD_CANCELED);
		return FALSE;
	}

	// scan and encrypt/decrypt
	if (!fc.Scan() || fc.nNotEncrypted != 0) {
		SetLastError(ERROR_TOMBO_E_SOME_ERROR_OCCURED);
		return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////
// Delete Note/folder
/////////////////////////////////////////

BOOL LocalFileRepository::Delete(const TomboURI *pURI, URIOption *pOption)
{
	if (pURI->IsLeaf()) {
		MemoNote *pNote = MemoNote::MemoNoteFactory(pURI);
		if (pNote == NULL) return FALSE;
		AutoPointer<MemoNote> ap(pNote);

		return pNote->DeleteMemoData(pTopDir);
	} else {
		TString sFullPath;
		if (!GetPhysicalPath(pURI, &sFullPath)) return FALSE;
		
		DSFileDelete fd;
		fd.Init(sFullPath.Get());
		if (!fd.Scan()) {
			return FALSE;
		}
	}
	return TRUE;
}

/////////////////////////////////////////
// Get Physical file/folder path
/////////////////////////////////////////
BOOL LocalFileRepository::GetPhysicalPath(const TomboURI *pURI, TString *pFullPath)
{
	if (!pFullPath->Alloc(_tcslen(pTopDir) + _tcslen(pURI->GetPath()) + 1)) return FALSE;
	LPCTSTR p = pURI->GetPath();
	_tcscpy(pFullPath->Get(), pTopDir);
	LPTSTR q = pFullPath->Get() + _tcslen(pFullPath->Get());

	while (*p) {
#if defined(PLATFORM_WIN32)
		if (IsDBCSLeadByte(*p)) {
			*q++ = *p++;
		}
#endif
		if (*p == TEXT('/')) {
			*q++ = TEXT('\\');
			p++;
			continue;
		}
		*q++ = *p++;
	}
	*q = TEXT('\0');
	return TRUE;
}

/////////////////////////////////////////

static BOOL IsSubFolder(LPCTSTR pSrc, LPCTSTR pDst)
{
	DWORD n = _tcslen(pSrc);
	if (_tcsncmp(pSrc, pDst, n) == 0) return TRUE;
	return FALSE;
}

/////////////////////////////////////////
// Copy note/folder
/////////////////////////////////////////

BOOL LocalFileRepository::Copy(const TomboURI *pCopyFrom, const TomboURI *pCopyTo, URIOption *pOption)
{
	URIOption opt1(NOTE_OPTIONMASK_ENCRYPTED | NOTE_OPTIONMASK_SAFEFILE | NOTE_OPTIONMASK_VALID);
	if (!GetOption(pCopyFrom, &opt1)) return FALSE;

	URIOption opt2(NOTE_OPTIONMASK_VALID);
	if (!GetOption(pCopyTo, &opt2)) return FALSE;

	if (!opt1.bValid || !opt2.bValid || !opt2.bFolder) {
		SetLastError(ERROR_TOMBO_E_INVALIDURI);
		return FALSE;
	}

	if (opt1.bFolder) {
		TString sSrcFull, sDstFull;
		if (!GetPhysicalPath(pCopyFrom, &sSrcFull)) return FALSE;
		if (!GetPhysicalPath(pCopyTo, &sDstFull)) return FALSE;

		if (IsSubFolder(sSrcFull.Get(), sDstFull.Get())) {
			SetLastError(ERROR_TOMBO_W_OPERATION_NOT_PERMITTED);
			return FALSE;
		}
		
		// Adjust Path
		TString sHL;
		if (!GetHeadLine(pCopyFrom, &sHL)) return FALSE;
		if (!sDstFull.StrCat(sHL.Get()) || !sDstFull.StrCat(TEXT("\\"))) return FALSE;

		MemoFolder mf;
		if (!mf.Init(pTopDir, sSrcFull.Get())) return FALSE;
		return mf.Copy(sDstFull.Get());
	} else {
		if ((pOption->pNewHeadLine = new TString()) == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }
		if ((pOption->pNewURI = new TomboURI()) == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }

		TString sToPath;
		if (!GetPhysicalPath(pCopyTo, &sToPath)) return FALSE;

		if (opt1.bSafeFileName) {
			TString sOrigFull;
			if (!GetPhysicalPath(pCopyFrom, &sOrigFull)) return FALSE;

			TString sBase;
			if (!pCopyFrom->GetBaseName(&sBase)) return FALSE;
			TString sNewPath;
			if (!sNewPath.Join(sToPath.Get(), sBase.Get())) return FALSE;
			if (IsFileExist(sNewPath.Get())) {
				TString sNewBase;
				if (!GetSafeFileName(&sToPath, &sNewBase)) return FALSE;
				if (!sNewPath.Join(sToPath.Get(), sNewBase.Get())) return FALSE;
				if (!sBase.Set(sNewBase.Get())) return FALSE;
			}
			if (!CopyFile(sOrigFull.Get(), sNewPath.Get(), TRUE)) return FALSE;
			TString sOrigTDT, sNewTDT;
			if (sOrigTDT.Join(sOrigFull.Get(), TEXT(".tdt")) &&
				sNewTDT.Join(sNewPath.Get(), TEXT(".tdt"))) {
				CopyFile(sOrigTDT.Get(), sNewTDT.Get(), TRUE);
			}
			if (!GetHeadLine(pCopyFrom, pOption->pNewHeadLine)) return FALSE;

			TString sNewURI;
			if (!sNewURI.Join(pCopyTo->GetFullURI(), sBase.Get())) return FALSE;
			if (!pOption->pNewURI->Init(sNewURI.Get())) return FALSE;

			return TRUE;
		} else {
			MemoNote *pNote = MemoNote::MemoNoteFactory(pCopyFrom);
			if (pNote == NULL) return FALSE;
			AutoPointer<MemoNote> ap(pNote);

			LPCTSTR pMemoPath = sToPath.Get() + _tcslen(pTopDir) + 1;

			MemoNote *pNewNote = MemoNote::CopyMemo(pTopDir, pNote, pMemoPath, pOption->pNewHeadLine);
			if (pNewNote == NULL) return FALSE;
			AutoPointer<MemoNote> ap2(pNewNote);

			pNewNote->GetURI(GetRepositoryName(), pOption->pNewURI);
			return TRUE;
		}
	}
}

/////////////////////////////////////////
// Move
/////////////////////////////////////////

BOOL LocalFileRepository::Move(const TomboURI *pMoveFrom, const TomboURI *pMoveTo, URIOption *pOption)
{
	if (!Copy(pMoveFrom, pMoveTo, pOption)) return FALSE;
	URIOption opt;
	if (!Delete(pMoveFrom, &opt)) return FALSE;
	return TRUE;
}

/////////////////////////////////////////
// Rename headline
/////////////////////////////////////////

BOOL LocalFileRepository::ChangeHeadLine(const TomboURI *pURI, LPCTSTR pReqNewHeadLine, URIOption *pOption)
{
	URIOption opt(NOTE_OPTIONMASK_ENCRYPTED | NOTE_OPTIONMASK_SAFEFILE | NOTE_OPTIONMASK_VALID);
	if (!GetOption(pURI, &opt)) return FALSE;
	if (opt.bValid == FALSE) {
		SetLastError(ERROR_TOMBO_E_INVALIDURI);
		return FALSE;
	}

	if (opt.bFolder) {
		TString sFullPath;
		if (!GetPhysicalPath(pURI, &sFullPath)) return FALSE;

		MemoFolder mf;
		if (!mf.Init(pTopDir, sFullPath.Get())) return FALSE;

		return mf.Rename(pReqNewHeadLine);
	} else {
		if (opt.bEncrypt && opt.bSafeFileName) {
			SetLastError(ERROR_TOMBO_I_OPERATION_NOT_PERFORMED);
			return FALSE;
		}

		MemoNote *pNote = MemoNote::MemoNoteFactory(pURI);
		if (pNote == NULL) return FALSE;
		AutoPointer<MemoNote> ap(pNote);

		if (!pNote->Rename(pTopDir, pReqNewHeadLine)) return FALSE;

		TomboURI *p = new TomboURI();
		if (p == NULL || !pNote->GetURI(GetRepositoryName(), p)) {
			delete p;
			return FALSE;
		}
		pOption->pNewURI = p;

		return TRUE;
	}
}

/////////////////////////////////////////
// GetList
/////////////////////////////////////////

DWORD LocalFileRepository::GetList(const TomboURI *pFolder, DirList *pList, BOOL bSkipEncrypt, BOOL bLooseDecrypt)
{
	TString sPartPath;
	if (!pFolder->GetFilePath(&sPartPath)) return TOMBO_REPO_GETLIST_FAIL;

	TString sFullPath;
	if (_tcslen(sPartPath.Get()) > 0) {
		if (!sFullPath.Join(pTopDir, TEXT("\\"), sPartPath.Get(), TEXT("*.*"))) return TOMBO_REPO_GETLIST_FAIL;
	} else {
		if (!sFullPath.Join(pTopDir, TEXT("\\*.*"))) return TOMBO_REPO_GETLIST_FAIL;
	}

	if (!pList->Init(pFolder->GetFullURI())) return TOMBO_REPO_GETLIST_FAIL;
	switch (pList->GetList(sFullPath.Get(), bSkipEncrypt, bLooseDecrypt)) {
	case DIRLIST_GETLIST_RESULT_SUCCESS:
		return TOMBO_REPO_GETLIST_SUCCESS;
	case DIRLIST_GETLIST_RESULT_PARTIAL:
		return TOMBO_REPO_GETLIST_PARTIAL;
	default:
		return TOMBO_REPO_GETLIST_FAIL;
	}
}

URIList *LocalFileRepository::GetChild(const TomboURI *pFolder, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose)
{
	URIList *pList = new URIList();
	if (pList == NULL || !pList->Init()) { return FALSE; }

	DirList dlist;
	switch (GetList(pFolder, &dlist, bSkipEncrypt, bLooseDecrypt)) {
	case DIRLIST_GETLIST_RESULT_FAIL:
		return NULL;
	case DIRLIST_GETLIST_RESULT_SUCCESS:
		*pLoose = FALSE;
		break;
	case DIRLIST_GETLIST_RESULT_PARTIAL:
		*pLoose = TRUE;
		break;
	}

	for (DWORD i = 0; i < dlist.NumItems(); i++) {
		DirListItem *pItem = dlist.GetItem(i);
		LPCTSTR pFN = dlist.GetFileName(pItem->nFileNamePos);
		LPCTSTR pHL = dlist.GetFileName(pItem->nHeadLinePos);
		LPCTSTR pURI = dlist.GetFileName(pItem->nURIPos);

		TomboURI sURI;
		if (!sURI.Init(pURI)) return NULL;

		URIOption opt(NOTE_OPTIONMASK_VALID);
		if (!GetOption(&sURI, &opt)) return NULL;

		if (!opt.bFolder) {
			DWORD dirType = MemoNote::IsNote(pFN);
			if (dirType != NOTE_TYPE_PLAIN && dirType != NOTE_TYPE_CRYPTED) continue;
		}

		if (!pList->Add(&sURI, dlist.GetFileName(pItem->nHeadLinePos))) return NULL;
	}
	return pList;
}


/////////////////////////////////////////
// 
/////////////////////////////////////////
BOOL LocalFileRepository::RequestAllocateURI(const TomboURI *pBaseURI, LPCTSTR pText, TString *pHeadLine, TomboURI *pURI, const TomboURI *pTemplateURI)
{	
	LPCTSTR pMemoPath;
	TString sMemoPath;
	if (!pBaseURI->GetFilePath(&sMemoPath)) return FALSE;
	pMemoPath = sMemoPath.Get();

	MemoNote *pNote;
	if (pTemplateURI) {
		MemoNote *pCurrent = MemoNote::MemoNoteFactory(pTemplateURI);
		if (pCurrent == NULL) return FALSE;
		AutoPointer<MemoNote> apNote(pCurrent);

		pNote = pCurrent->GetNewInstance();
	} else {
		pNote = new PlainMemoNote();
	}

	if (pNote == NULL) return FALSE;

	AutoPointer<MemoNote> ap(pNote);
	
	TString sFullPath;
	TString sHeadLine;
	LPCTSTR pNotePath;

	if (!MemoNote::GetHeadLineFromMemoText(pText, &sHeadLine)) return FALSE;
	if (!MemoNote::GetHeadLinePath(pTopDir, pMemoPath, sHeadLine.Get(), 
									pNote->GetExtension(), &sFullPath, &pNotePath, pHeadLine)) return FALSE;
	if (!pNote->Init(pNotePath)) return FALSE;

	if (!pNote->GetURI(GetRepositoryName(), pURI)) return FALSE;

	return TRUE;
}

BOOL LocalFileRepository::GetAttribute(const TomboURI *pURI, NoteAttribute *pAttribute)
{
	MemoNote *pNote = MemoNote::MemoNoteFactory(pURI);
	if (pNote == NULL) return FALSE;
	AutoPointer<MemoNote> ap(pNote);

	MemoInfo mi(pTopDir);
	DWORD nPos = 0;
	if (pNote->MemoPath()) {
		if (!mi.ReadInfo(pNote->MemoPath(), &nPos)) nPos = 0;
	}
	pAttribute->nCursorPos = nPos;

	BOOL bReadOnly;
	if (!g_Property.GetOpenReadOnly()) {
//		if (!pNote->IsReadOnly(&bReadOnly)) {
//			return FALSE;
//		}
		TString sFullPath;
		if (!sFullPath.Join(pTopDir, TEXT("\\"), pNote->MemoPath())) return FALSE;

		WIN32_FIND_DATA wfd;
		HANDLE h = FindFirstFile(sFullPath.Get(), &wfd);
		if (h == INVALID_HANDLE_VALUE) return FALSE;
		FindClose(h);

		bReadOnly = (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY;

	} else {
		bReadOnly = TRUE;
	}
	pAttribute->bReadOnly = bReadOnly;

	return TRUE;
}

BOOL LocalFileRepository::GetNoteAttribute(const TomboURI *pURI, UINT64 *pLastUpdate, UINT64 *pCreateDate, UINT64 *pFileSize)
{
	URIOption opt(NOTE_OPTIONMASK_VALID);
	if (!GetOption(pURI, &opt)) return FALSE;
	if (!opt.bValid) { SetLastError(ERROR_TOMBO_E_INVALIDURI); return FALSE; }
	if (opt.bFolder) { SetLastError(ERROR_NOT_SUPPORTED); return FALSE; }

	TString sFullPath;
	if (!GetPhysicalPath(pURI, &sFullPath)) return FALSE;

	WIN32_FIND_DATA wfd;
	HANDLE h = FindFirstFile(sFullPath.Get(), &wfd);
	if (h != INVALID_HANDLE_VALUE) {
		*pLastUpdate = ((UINT64)wfd.ftLastWriteTime.dwHighDateTime << 32) | (UINT64)wfd.ftLastWriteTime.dwLowDateTime ;
		*pCreateDate = ((UINT64)wfd.ftCreationTime.dwHighDateTime << 32) | (UINT64)wfd.ftCreationTime.dwLowDateTime;
		*pFileSize = ((UINT64)wfd.nFileSizeHigh << 32 ) | (UINT64)wfd.nFileSizeLow;
		FindClose(h);
		return TRUE;
	} else {
		return FALSE;
	}
}

BOOL LocalFileRepository::SetAttribute(const TomboURI *pURI, const NoteAttribute *pAttribute)
{
	MemoNote *pNote = MemoNote::MemoNoteFactory(pURI);
	if (pNote == NULL) return FALSE;
	AutoPointer<MemoNote> ap(pNote);

	MemoInfo mi(pTopDir);

	if (pNote == NULL) return FALSE;
	mi.WriteInfo(pNote->MemoPath(), pAttribute->nCursorPos);

	return TRUE;
}

LPTSTR LocalFileRepository::GetNoteData(const TomboURI *pURI)
{
	MemoNote *pNote = MemoNote::MemoNoteFactory(pURI);
	if (pNote == NULL) return FALSE;
	AutoPointer<MemoNote> ap(pNote);

	BOOL bLoop = FALSE;
	LPTSTR p;

	do {
		bLoop = FALSE;
		p = pNote->GetMemoBody(pTopDir, g_pPassManager);
		if (p == NULL) {
			DWORD nError = GetLastError();
			if (nError == ERROR_INVALID_PASSWORD) {
				bLoop = TRUE;
			} else {
				return NULL;
			}
		}
	} while (bLoop);
	return p;
}

LPBYTE LocalFileRepository::GetNoteDataNative(const TomboURI *pURI, LPDWORD pSize)
{
	MemoNote *pNote = MemoNote::MemoNoteFactory(pURI);
	if (pNote == NULL) return FALSE;
	AutoPointer<MemoNote> ap(pNote);

	BOOL bLoop = FALSE;
	LPBYTE p;
	DWORD nSize;
	do {
		bLoop = FALSE;
		p = pNote->GetMemoBodyNative(pTopDir, g_pPassManager, &nSize);
		if (p == NULL) {
			DWORD nError = GetLastError();
			if (nError == ERROR_INVALID_PASSWORD) {
				bLoop = TRUE;
			} else {
				return NULL;
			}
		}
	} while (bLoop);

	if (p) {
		*pSize = nSize;
	}

	return p;
}

BOOL LocalFileRepository::ExecuteAssoc(const TomboURI *pURI, ExeAppType nType)
{
	URIOption opt(NOTE_OPTIONMASK_VALID);
	if (!GetOption(pURI, &opt)) return FALSE;
	if (opt.bValid == FALSE) {
		SetLastError(ERROR_TOMBO_E_INVALIDURI);
		return FALSE;
	}

	if (opt.bFolder) {
		if (nType != ExecType_Assoc) {
			SetLastError(ERROR_NOT_SUPPORTED);
			return FALSE;
		}

		TString sCurrentPath;
		if (!GetPhysicalPath(pURI, &sCurrentPath)) return FALSE;

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		memset(&si, 0, sizeof(si));
		memset(&pi, 0, sizeof(pi));
		si.cb = sizeof(si);

		if (!CreateProcess(TEXT("\\windows\\iexplore.exe"), sCurrentPath.Get(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return FALSE;
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return TRUE;
#else
		SHELLEXECUTEINFO se;
		memset(&se, 0, sizeof(se));
		se.cbSize = sizeof(se);
		se.hwnd = NULL;
		se.lpVerb = TEXT("explore");
		se.lpFile = sCurrentPath.Get();
		se.lpParameters = NULL;
		se.lpDirectory = NULL;
		se.nShow = SW_SHOWNORMAL;
		ShellExecuteEx(&se);
		return TRUE;
#endif
	} else {
		TString sFullPath;
		if (!GetPhysicalPath(pURI, &sFullPath)) return FALSE;

		if (nType == ExecType_Assoc) {
			SHELLEXECUTEINFO se;
			memset(&se, 0, sizeof(se));
			se.cbSize = sizeof(se);
			se.hwnd = NULL;
			se.lpVerb = TEXT("open");
			se.lpFile = sFullPath.Get();
			se.lpParameters = NULL;
			se.lpDirectory = NULL;
			se.nShow = SW_SHOWNORMAL;
			ShellExecuteEx(&se);
			if ((int)se.hInstApp < 32) return FALSE;
			return TRUE;
		} else if (nType == ExecType_ExtApp1 || nType == ExecType_ExtApp2) {
			LPCTSTR pExeFile = nType == ExecType_ExtApp1 ? g_Property.GetExtApp1() : g_Property.GetExtApp2();
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			memset(&si, 0, sizeof(si));
			memset(&pi, 0, sizeof(pi));
			si.cb = sizeof(si);

			TString sExe;
			TString sCmdLine;
#if defined(PLATFORM_WIN32)
			if (!sCmdLine.Join(TEXT("\""), pExeFile, TEXT("\" "))) return FALSE;
			if (!sCmdLine.StrCat(TEXT("\""))) return FALSE;
			if (!sCmdLine.StrCat(sFullPath.Get())) return FALSE;
			if (!sCmdLine.StrCat(TEXT("\""))) return FALSE;
			if (!CreateProcess(NULL, sCmdLine.Get(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return FALSE;
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			return TRUE;

#endif
#if defined(PLATFORM_HPC)
			if (!sExe.Set(pExeFile)) return FALSE;
			if (!sCmdLine.Join(TEXT("\""), sFullPath.Get(), TEXT("\""))) return FALSE;
			if (!CreateProcess(sExe.Get(), sCmdLine.Get(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return FALSE;
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			return TRUE;

#endif
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
			if (!sExe.Set(pExeFile)) return FALSE;
			if (!sCmdLine.Set(sFullPath.Get())) return FALSE;
			if (!CreateProcess(sExe.Get(), sCmdLine.Get(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return FALSE;
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			return TRUE;
#endif
#if defined(PLATFORM_BE500)
			return CoshExecute(NULL, pExeFile, sFullPath.Get());
#endif
		} else {
			SetLastError(ERROR_NOT_SUPPORTED);
			return FALSE;
		}
	}
}

BOOL LocalFileRepository::MakeFolder(const TomboURI *pURI, LPCTSTR pFolderName)
{
	URIOption opt(NOTE_OPTIONMASK_VALID);
	if (!GetOption(pURI, &opt)) return FALSE;
	if (!opt.bValid) { SetLastError(ERROR_TOMBO_E_INVALIDURI); return FALSE; }
	if (!opt.bFolder) { SetLastError(ERROR_NOT_SUPPORTED); return FALSE; }

	TString sPath;
	if (!GetPhysicalPath(pURI, &sPath)) return FALSE;
	if (!sPath.StrCat(pFolderName)) return FALSE;
	TrimRight(sPath.Get());
	ChopFileSeparator(sPath.Get());
	return CreateDirectory(sPath.Get(), NULL);
}

/////////////////////////////////////////
// Virtual folder
/////////////////////////////////////////

VFolderRepository::VFolderRepository()
{
}

VFolderRepository::~VFolderRepository()
{
}

BOOL VFolderRepository::Init(LPCTSTR pRepName, LPCTSTR pDispName)
{
	return RepositoryImpl::Init(pRepName, pDispName, TOMBO_REPO_SUBREPO_TYPE_VFOLDER);
}

RepositoryImpl *VFolderRepository::Clone()
{
	VFolderRepository *pImpl = new VFolderRepository();
	if (pImpl == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	if (!pImpl->Init(GetRepositoryName(), GetDisplayName())) {
		return NULL;
	}
	return pImpl;
}

LPTSTR VFolderRepository::GetXMLSaveString()
{
	TString sXMLStr;
	if (!sXMLStr.Join(TEXT("      <vfolder name=\""), GetRepositoryName(), TEXT("\" dispname=\""), GetDisplayName())) return NULL;
	if (!sXMLStr.StrCat(TEXT("\"/>\n"))) return NULL;
	return StringDup(sXMLStr.Get());
}

BOOL VFolderRepository::Update(const TomboURI *pCurrentURI, LPCTSTR pData, TomboURI *pNewURI, TString *pNewHeadLine){ return FALSE; }
BOOL VFolderRepository::Delete(const TomboURI *pURI, URIOption *pOption){ return FALSE; }
BOOL VFolderRepository::Copy(const TomboURI *pCopyFrom, const TomboURI *pCopyTo, URIOption *pOption){ return FALSE; }
BOOL VFolderRepository::Move(const TomboURI *pMoveFrom, const TomboURI *pMoveTo, URIOption *pOption){ return FALSE; }

BOOL VFolderRepository::ChangeHeadLine(const TomboURI *pURI, LPCTSTR pReqNewHeadLine, URIOption *pOption){ return FALSE; }

BOOL VFolderRepository::GetHeadLine(const TomboURI *pURI, TString *pHeadLine){ return FALSE; }

BOOL VFolderRepository::GetOption(const TomboURI *pURI, URIOption *pOption) const{ return FALSE; }
BOOL VFolderRepository::SetOption(const TomboURI *pCurrentURI, URIOption *pOption){ return FALSE; }

BOOL VFolderRepository::GetPhysicalPath(const TomboURI *pURI, TString *pFullPath){ return FALSE; }

URIList *VFolderRepository::GetChild(const TomboURI *pFolder, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose) { return NULL; }

BOOL VFolderRepository::RequestAllocateURI(const TomboURI *pBaseURI, LPCTSTR pText, TString *pHeadLine, TomboURI *pURI, const TomboURI *pTemplateURI){ return FALSE; }

BOOL VFolderRepository::GetAttribute(const TomboURI *pURI, NoteAttribute *pAttribute){ return FALSE; }
BOOL VFolderRepository::SetAttribute(const TomboURI *pURI, const NoteAttribute *pAttribute){ return FALSE; }
BOOL VFolderRepository::GetNoteAttribute(const TomboURI *pURI, UINT64 *pLastUpdate, UINT64 *pCreateDate, UINT64 *pFileSize){ return FALSE; }

LPTSTR VFolderRepository::GetNoteData(const TomboURI *pURI) { return NULL; }
LPBYTE VFolderRepository::GetNoteDataNative(const TomboURI *pURI, LPDWORD pSize) { return NULL; }

BOOL VFolderRepository::ExecuteAssoc(const TomboURI *pURI, ExeAppType nType){ return FALSE; }
BOOL VFolderRepository::MakeFolder(const TomboURI *pURI, LPCTSTR pFolderName){ return FALSE; }




