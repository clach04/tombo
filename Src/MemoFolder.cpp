#include <windows.h>
#include <commctrl.h>
#include <tchar.h>

#include "Tombo.h"
#include "UniConv.h"
#include "TString.h"
#include "TomboURI.h"
#include "DirectoryScanner.h"
#include "MemoFolder.h"
#include "PasswordManager.h"
#include "MemoNote.h"
#include "Message.h"
#include "Repository.h"

///////////////////////////////////////////////
// ctor & dtor
///////////////////////////////////////////////

MemoFolder::MemoFolder() : pFullPath(NULL)
{
}

MemoFolder::~MemoFolder()
{
	if (pFullPath) delete [] pFullPath;
}

BOOL MemoFolder::Init(LPCTSTR pTop, LPCTSTR p)
{
	pTopDir = pTop;
	if ((pFullPath = StringDup(p)) == NULL) return FALSE;
	return TRUE;
}

///////////////////////////////////////////////
// ディレクトリの移動
///////////////////////////////////////////////

BOOL MemoFolder::Move(LPCTSTR pDst)
{
	// 末尾のディレクトリを取得
	TCHAR aDst[MAX_PATH];
	_tcscpy(aDst, pFullPath);
	ChopFileSeparator(aDst);
	TString sDir;
	sDir.GetDirectoryPath(aDst);
	LPTSTR p = aDst + _tcslen(sDir.Get());

	TCHAR aSrc[MAX_PATH];
	_tcscpy(aSrc, pFullPath);
	ChopFileSeparator(aSrc);

	// 移動先が移動元のフォルダ名と同じになるようにパラメータを調整
	TString sDst;
	if (!sDst.Join(pDst, p)) return FALSE;

	// 移動処理
	if (!MoveFile(aSrc, sDst.Get())) return FALSE;

	return TRUE;
}

///////////////////////////////////////////////
// Delete directory
///////////////////////////////////////////////

DSFileDelete::DSFileDelete(){ /* NOP */}

BOOL DSFileDelete::Init(LPCTSTR pPath)
{
	return DirectoryScanner::Init(pPath, 0);
}

void DSFileDelete::InitialScan() {/* NOP */}
void DSFileDelete::PreDirectory(LPCTSTR) {/* NOP */}

void DSFileDelete::PostDirectory(LPCTSTR)
{
	if (!IsStopScan()) {
		TCHAR buf[MAX_PATH*2];
		_tcscpy(buf, CurrentPath());
		ChopFileSeparator(buf);
		if (!RemoveDirectory(buf)) {
			_tcscpy(aFailPath, buf);
//			_tcscpy(aErrorMsg, MSG_RMDIR_FAILED);
			StopScan();
			SetLastError(ERROR_TOMBO_E_RMDIR_FAILED);
		}
	}
}

void DSFileDelete::AfterScan() 
{
	PostDirectory(NULL);
}

void DSFileDelete::File(LPCTSTR p)
{
	// if the file that TOMBO is not treat exists, stop deleting. 
	if (MemoNote::IsNote(CurrentPath()) == NOTE_TYPE_NO) {
		_tcscpy(aFailPath, CurrentPath());
		StopScan();
		SetLastError(ERROR_TOMBO_W_OTHERFILE_EXISTS);
		return;
	}

	// Delete
	if (!DeleteFile(CurrentPath())) {
		_tcscpy(aFailPath, CurrentPath());
		StopScan();
		SetLastError(ERROR_TOMBO_E_RMFILE_FAILED);
	}
}

///////////////////////////////////////////////
// ディレクトリコピー
///////////////////////////////////////////////

class DSFileCopy: public DirectoryScanner {
	TCHAR aDstPath[MAX_PATH * 2];
public:
	TCHAR aFailPath[MAX_PATH * 2];
	TCHAR aErrorMsg[1024];

	BOOL Init(LPCTSTR pSrc, LPCTSTR pDst);
	void InitialScan();
	void AfterScan();
	void PreDirectory(LPCTSTR);
	void PostDirectory(LPCTSTR);
	void File(LPCTSTR);
};

BOOL DSFileCopy::Init(LPCTSTR pSrc, LPCTSTR pDst)
{
	_tcscpy(aDstPath, pDst);
	return DirectoryScanner::Init(pSrc, 0);
}

void DSFileCopy::InitialScan()
{
	TCHAR buf[MAX_PATH * 2];
	_tcscpy(buf, aDstPath);
	ChopFileSeparator(buf);
	if (!CreateDirectory(buf, NULL)) {
		_tcscpy(aFailPath, buf);
		_tcscpy(aErrorMsg, MSG_MKDIR_FAILED);
		StopScan();
	}
}

void DSFileCopy::AfterScan() {/* NOP */}

void DSFileCopy::File(LPCTSTR p)
{
	_tcscat(aDstPath, p);

	if (!CopyFile(CurrentPath(), aDstPath, FALSE)) {
		_tcscpy(aFailPath, aDstPath);
		_tcscpy(aErrorMsg, MSG_FILECOPY_FAILED);
		StopScan();
	}

	*(aDstPath + (_tcslen(aDstPath) - _tcslen(p))) = TEXT('\0');
}

void DSFileCopy::PreDirectory(LPCTSTR p)
{
	_tcscat(aDstPath, p);
	if (!CreateDirectory(aDstPath, NULL)) {
		_tcscpy(aFailPath, aDstPath);
		_tcscpy(aErrorMsg, MSG_MKDIR_FAILED);
		StopScan();
	}
	_tcscat(aDstPath, TEXT("\\"));
}

void DSFileCopy::PostDirectory(LPCTSTR p)
{
	*(aDstPath + (_tcslen(aDstPath) - _tcslen(p) - 1)) = TEXT('\0');
}

BOOL MemoFolder::Copy(LPCTSTR pDst)
{
	DSFileCopy fc;
	fc.Init(pFullPath, pDst);

	if (!fc.Scan()) {
		sErrorReason.Join(fc.aErrorMsg, TEXT(" : "), fc.aFailPath);
		return FALSE;
	} else {
		return TRUE;
	}
}

///////////////////////////////////////////////
// ディレクトリ名の変更
///////////////////////////////////////////////

BOOL MemoFolder::Rename(LPCTSTR pNewName)
{
	if (_tcslen(pNewName) == 0) {
		SetLastError(ERROR_NO_DATA);
		return FALSE;
	}

	TString sCurrent;
	if (!sCurrent.Set(pFullPath)) return FALSE;
	sCurrent.ChopFileSeparator();
	
	TString sNew;
	sNew.GetDirectoryPath(sCurrent.Get());
	sNew.StrCat(pNewName);

	return MoveFile(sCurrent.Get(), sNew.Get());
}

///////////////////////////////////////////////
//  Encrypt/decrypt to folderr
///////////////////////////////////////////////

DSEncrypt::~DSEncrypt()
{
	delete pURI;
}

BOOL DSEncrypt::Init(LPCTSTR pTopDir, LPCTSTR pPath, LPCTSTR pBaseURI, BOOL bEnc) {
	nBaseLen = _tcslen(pTopDir);
	nNotEncrypted = 0;
	bEncrypt = bEnc;
	pURI = new TString();
	if (pURI == NULL || !pURI->Alloc(MAX_PATH)) return FALSE;
	nURIBufSize = MAX_PATH;

	_tcscpy(pURI->Get(), pBaseURI);
	nCurrentPos = _tcslen(pBaseURI);

	return DirectoryScanner::Init(pPath, 0);
}

BOOL DSEncrypt::CheckURIBuffer(LPCTSTR p)
{
	// check buffer size
	if (nCurrentPos + _tcslen(p) > nURIBufSize) {
		TString *pNewBuf = new TString();
		if (pNewBuf == NULL || !pNewBuf->Alloc(nURIBufSize + MAX_PATH)) {
			StopScan();
			return FALSE;
		}
		_tcscpy(pNewBuf->Get(), pURI->Get());
		delete pURI;
		pURI = pNewBuf;
	}
	return TRUE;
}

void DSEncrypt::PreDirectory(LPCTSTR pDir)
{
	if (!CheckURIBuffer(pDir)) return;

	DWORD n = _tcslen(pDir);
	_tcscpy(pURI->Get() + nCurrentPos, pDir);
	nCurrentPos += n;
	_tcscat(pURI->Get() + nCurrentPos, TEXT("/"));
	nCurrentPos++;
}

void DSEncrypt::PostDirectory(LPCTSTR pDir)
{
	DWORD n = _tcslen(pDir);
	nCurrentPos -= n + 1;
	*(pURI->Get() + nCurrentPos) = TEXT('\0');
}

void DSEncrypt::File(LPCTSTR pFile)
{
	if (!CheckURIBuffer(pFile)) return;

	_tcscpy(pURI->Get() + nCurrentPos, pFile);

	TomboURI sURI;
	if (!sURI.Init(pURI->Get())) {
		StopScan();
		return;
	}

	URIOption gopt(NOTE_OPTIONMASK_VALID | NOTE_OPTIONMASK_ENCRYPTED);
	if (!g_Repository.GetOption(&sURI, &gopt)) {
		nNotEncrypted++;
		return;
	}
	if (gopt.bValid == FALSE || gopt.bFolder == TRUE) return;

	if (gopt.bEncrypt == bEncrypt) return;

	URIOption opt(NOTE_OPTIONMASK_ENCRYPTED);
	opt.bEncrypt = bEncrypt;
	if (!g_Repository.SetOption(&sURI, &opt)) {
		nNotEncrypted++;
		return;
	}
}
