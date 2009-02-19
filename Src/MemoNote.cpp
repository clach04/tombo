#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "Tombo.h"
#include "MemoNote.h"
#include "File.h"
#include "CryptManager.h"
#include "UniConv.h"
#include "PasswordManager.h"
#include "MemoManager.h"
#include "Property.h"
#include "TString.h"
#include "MemoInfo.h"
#include "Message.h"
#include "TomboURI.h"
#include "AutoPtr.h"

#include "Repository.h"

#define DEFAULT_HEADLINE MSG_DEFAULT_HEADLINE

/////////////////////////////////////////////
//
/////////////////////////////////////////////

MemoNote::MemoNote() : pPath(NULL)
{
}

MemoNote::~MemoNote()
{
	delete [] pPath;
}

BOOL MemoNote::Init(LPCTSTR p)
{
	if (pPath) delete [] pPath;
	pPath = StringDup(p);
	if (pPath == NULL) return FALSE;
	return TRUE;
}

/////////////////////////////////////////////
// get note's URI
/////////////////////////////////////////////
BOOL MemoNote::GetURI(LPCTSTR pRepoName, TomboURI *pURI) const
{
	return pURI->InitByNotePath(pRepoName, pPath);
}

/////////////////////////////////////////////
// Get memo data from file and decrypt if nesessary
/////////////////////////////////////////////

LPTSTR MemoNote::GetMemoBody(LPCTSTR pTopDir, PasswordManager *pMgr) const
{
	DWORD nSize;
	LPBYTE pData = GetMemoBodyNative(pTopDir, pMgr, &nSize);
	if (!pData) return NULL;
	SecureBufferAutoPointerByte ap(pData, nSize);
	return ConvFileEncodingToTChar(pData);
}

LPBYTE PlainMemoNote::GetMemoBodyNative(LPCTSTR pTopDir, PasswordManager *pMgr, LPDWORD pSize) const
{
	TString sFileName;
	if (!sFileName.Join(pTopDir, TEXT("\\"), pPath)) return NULL;

	File inf;
	if (!inf.Open(sFileName.Get(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING)) return NULL;

	LPBYTE pData = new BYTE[inf.FileSize() + 2];
	if (pData == NULL) return NULL;

	DWORD nSize = inf.FileSize();
	if (!inf.Read(pData, &nSize)) return NULL;
	pData[nSize] = TEXT('\0');
	pData[nSize + 1] = TEXT('\0');	// sentinel for the UTF16 encoding file

	*pSize = nSize;
	return pData;
}

LPBYTE CryptedMemoNote::GetMemoBodySub(LPCTSTR pTopDir, PasswordManager *pMgr, LPDWORD pSize) const
{
	BOOL bRegistedPassword = TRUE;

	TString sFileName;
	if (!sFileName.Join(pTopDir, TEXT("\\"), pPath)) return NULL;

	LPBYTE pPlain;
	for (DWORD i = 0; i < NUM_RETRY_INVALID_PASSWORD; i++) {
		BOOL bCancel;
		const char *pPassword = pMgr->Password(&bCancel, FALSE);
		if (pPassword == NULL) {
			if (bCancel) SetLastError(ERROR_CANCELLED);
			pMgr->ForgetPassword();
			return NULL;
		}
		CryptManager cMgr;
		if (!cMgr.Init(pPassword)) return NULL;

		pPlain = cMgr.LoadAndDecrypt(pSize, sFileName.Get());
		if (pPlain != NULL) {
			bRegistedPassword = TRUE;
			break;
		} else {
			bRegistedPassword = FALSE;
			pMgr->ForgetPassword();
		}
	}
	return pPlain;
}

LPBYTE CryptedMemoNote::GetMemoBodyNative(LPCTSTR pTopDir, PasswordManager *pMgr, LPDWORD pSize) const
{
	return GetMemoBodySub(pTopDir, pMgr, pSize);
}

/////////////////////////////////////////////
// Save note data
/////////////////////////////////////////////

BOOL MemoNote::SaveDataT(PasswordManager *pMgr, LPCTSTR pMemo, LPCTSTR pWriteFile)
{
	LPBYTE pData;
	DWORD nLen;

	pData = ConvTCharToFileEncoding(pMemo, &nLen);
	if (pData == NULL) return FALSE;
	SecureBufferAutoPointerByte ap(pData, nLen);
	return SaveData(pMgr, pData, nLen, pWriteFile);
}

BOOL PlainMemoNote::SaveData(PasswordManager *pMgr, const LPBYTE pData, DWORD nLen, LPCTSTR pWriteFile)
{
	File outf;
	if (!outf.Open(pWriteFile, GENERIC_WRITE, 0, OPEN_ALWAYS)) return FALSE;
	if (!outf.Write((LPBYTE)pData, nLen)) return FALSE;
	if (!outf.SetEOF()) return FALSE;
	outf.Close();
	return TRUE;
}

BOOL CryptedMemoNote::SaveData(PasswordManager *pMgr, const LPBYTE pData, DWORD nLen, LPCTSTR pWriteFile)
{
	CryptManager cMgr;
	BOOL bCancel;
	const char *pPassword = pMgr->Password(&bCancel, TRUE);
	if (pPassword == NULL) return FALSE;

	if (!cMgr.Init(pPassword)) {
		MessageBox(NULL, TEXT("In CryptedMemoNote::SaveData,CryptManager::Init failed"), TEXT("DEBUG"), MB_OK);
		return FALSE;
	}
	return cMgr.EncryptAndStore(pData, nLen, pWriteFile);
}

/////////////////////////////////////////////
// 復号化
/////////////////////////////////////////////

MemoNote *MemoNote::Decrypt(LPCTSTR pTopDir, PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified) const
{
	return NULL;
}

MemoNote *CryptedMemoNote::Decrypt(LPCTSTR pTopDir, PasswordManager *pMgr, TString *pHeadLine, BOOL *pIsModified) const
{
	// メモ本文取得
	LPTSTR pText = GetMemoBody(pTopDir, pMgr);
	if (pText == NULL) return FALSE;
	SecureBufferAutoPointerT ap(pText);

	// ヘッドライン取得
	TString sMemoDir;
	if (!sMemoDir.GetDirectoryPath(pPath)) return FALSE;

	TString sFullPath;
	LPCTSTR pNotePath;
	TString sHeadLine;
	if (g_Property.GetKeepTitle()) {
		if (!GetHeadLineFromFilePath(pPath, &sHeadLine)) return FALSE;
	} else {
		if (!GetHeadLineFromMemoText(pText, &sHeadLine)) return FALSE;
	}

	if (!GetHeadLinePath(pTopDir, sMemoDir.Get(), sHeadLine.Get(), TEXT(".txt"), 
							&sFullPath, &pNotePath, pHeadLine)) {
		return FALSE;
	}

	// 新しいMemoNoteインスタンスを生成
	PlainMemoNote *p = new PlainMemoNote();
	if (!p->Init(pNotePath)) {
		WipeOutAndDeleteFile(sFullPath.Get());
		return NULL;
	}

	// メモ保存
	if (!p->SaveDataT(pMgr, pText, sFullPath.Get())) {
		delete p;
		return NULL;
	}
	return p;
}

/////////////////////////////////////////////
// データの削除
/////////////////////////////////////////////

BOOL MemoNote::DeleteMemoData(LPCTSTR pTopDir) const 
{
	TString sFileName;
	if (!sFileName.Join(pTopDir, TEXT("\\"), pPath)) return FALSE;

	// 付加情報を保持していた場合にはその情報も削除
	if (MemoPath()) {
		MemoInfo mi(pTopDir);
		mi.DeleteInfo(MemoPath());
	}

	return WipeOutAndDeleteFile(sFileName.Get());
}

/////////////////////////////////////////////
// インスタンスの生成
/////////////////////////////////////////////

MemoNote *PlainMemoNote::GetNewInstance() const 
{
	return new PlainMemoNote();
}

MemoNote *CryptedMemoNote::GetNewInstance() const
{
	return new CryptedMemoNote();
}

/////////////////////////////////////////////
// 拡張子の取得
/////////////////////////////////////////////

LPCTSTR PlainMemoNote::GetExtension()
{
	return TEXT(".txt");
}

LPCTSTR CryptedMemoNote::GetExtension()
{
	return TEXT(".chi");
}

/////////////////////////////////////////////
// ヘッドラインの取得
/////////////////////////////////////////////
// メモ本文からヘッドラインとなる文字列を取得する。
//
// ヘッドラインは
// ・メモの1行目である
// ・1行目が一定以上の長さの場合、その先頭部分
// とする。

BOOL MemoNote::GetHeadLineFromMemoText(LPCTSTR pMemo, TString *pHeadLine)
{
	// ヘッドライン長のカウント
	LPCTSTR p = pMemo;
	DWORD n = 0;
	while(*p) {
		if ((*p == TEXT('\r')) || (*p == TEXT('\n'))) break;
#ifndef _WIN32_WCE
		if (IsDBCSLeadByte(*p)) {
			p += 2;
			n += 2;
			continue;
		}
#endif
		n++;
		p++;
	}

	TString sHeadLineCand;
	if (!sHeadLineCand.Alloc(n + 1)) return FALSE;
	_tcsncpy(sHeadLineCand.Get(), pMemo, n);
	*(sHeadLineCand.Get() + n) = TEXT('\0');

	// 領域確保・コピー
	if (!pHeadLine->Alloc(n + 1)) return FALSE;
	DropInvalidFileChar(pHeadLine->Get(), sHeadLineCand.Get());
	TrimRight(pHeadLine->Get());

	if (_tcslen(pHeadLine->Get()) == 0) {
		if (!pHeadLine->Set(DEFAULT_HEADLINE)) return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////
// ファイルの存在チェック
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

// 与えられた文字列からファイル名を生成する

// IN:	pMemoPath	: メモのパス(TOPDIRからの相対パス,ファイル名は含まず)
//		pHeadLine	: ヘッドライン文字列
//		pExt		: 付与する拡張子
// OUT:	pFullPath	: メモのフルパス
//		ppNotePath	: メモのパス(TOPDIRからの相対パス,ファイル名を含み、
//					  必要なら"(n)"でディレクトリで一意となるように調整されている
//		pNewHeadLine: 一覧表示用新ヘッドライン(必要に応じて"(n)"が付与されている)

BOOL MemoNote::GetHeadLinePath(LPCTSTR pTopDir, LPCTSTR pMemoPath, LPCTSTR pHeadLine, LPCTSTR pExt, 
							TString *pFullPath, LPCTSTR *ppNotePath, TString *pNewHeadLine)
{
	DWORD n = _tcslen(pHeadLine);
	if (n < _tcslen(DEFAULT_HEADLINE)) n = _tcslen(DEFAULT_HEADLINE);

	DWORD nHeadLineLen = n + 20;
	DWORD nFullPathLen = _tcslen(pTopDir) + 1 + 
						 _tcslen(pMemoPath) + nHeadLineLen + _tcslen(pExt);
	if (!pNewHeadLine->Alloc(nHeadLineLen + 1)) return FALSE;
	if (!pFullPath->Alloc(nFullPathLen + 1)) return FALSE;

	DropInvalidFileChar(pNewHeadLine->Get(), pHeadLine);
	if (_tcslen(pNewHeadLine->Get()) == 0) _tcscpy(pNewHeadLine->Get(), DEFAULT_HEADLINE);
	wsprintf(pFullPath->Get(), TEXT("%s\\%s%s"), pTopDir, pMemoPath, pNewHeadLine->Get());

	LPTSTR p = pFullPath->Get();
	LPTSTR q = p + _tcslen(p);
	LPTSTR r = pNewHeadLine->Get() + _tcslen(pNewHeadLine->Get());

	*ppNotePath = pFullPath->Get() + _tcslen(pTopDir) + 1;

	// ファイル名の確定
	// 同名のファイルが存在した場合には"(n)"を付加する
	_tcscpy(q, pExt);
	if (!IsFileExist(p)) return TRUE;

	DWORD i = 1;
	do {
		wsprintf(q, TEXT("(%d)%s"), i, pExt);
		wsprintf(r, TEXT("(%d)"), i);
		i++;
	} while(IsFileExist(p));
	return TRUE;
}

////////////////////////////////////////////////////////
// ファイルパスからヘッドラインを取得
////////////////////////////////////////////////////////
// 本文は参照しないため、実際のヘッドライン文字列と必ずしも一致しないことに注意。

BOOL MemoNote::GetHeadLineFromFilePath(LPCTSTR pFilePath, TString *pHeadLine)
{
	LPCTSTR p = pFilePath;
	LPCTSTR q = NULL;
#ifdef _WIN32_WCE
	while (*p) {
		if (*p == TEXT('\\')) q = p;
		p++;
	}
#else
	while (*p) {
		if (*p == TEXT('\\')) q = p;
		if (IsDBCSLeadByte(*p)) {
			p++;
		}
		p++;
	}
#endif
	if (q == NULL) {
		if (!pHeadLine->Set(pFilePath)) return FALSE;
	} else {
		if (!pHeadLine->Set(q + 1)) return FALSE;
	}

	pHeadLine->ChopExtension();
	pHeadLine->ChopFileNumber();
	return TRUE;
}

////////////////////////////////////////////////////////
// メモファイルをコピーしてインスタンスを生成
////////////////////////////////////////////////////////

MemoNote *MemoNote::CopyMemo(LPCTSTR pTopDir, const MemoNote *pOrig, LPCTSTR pMemoPath, TString *pHeadLine)
{
	MemoNote *pNote;
	pNote = pOrig->GetNewInstance();
	if (pNote == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}

	TString sNewFullPath;
	LPCTSTR pNotePath;
	TString sHeadLine;

	if (!GetHeadLineFromFilePath(pOrig->MemoPath(), &sHeadLine)) {
		delete pNote;
		return NULL;
	}
	if (!GetHeadLinePath(pTopDir, pMemoPath, sHeadLine.Get(), pNote->GetExtension(), &sNewFullPath, &pNotePath, pHeadLine)) {
		delete pNote;
		return NULL;
	}

	TString sOrigPath;
	if (!sOrigPath.Join(pTopDir, TEXT("\\"), pOrig->MemoPath())) {
		delete pNote;
		return NULL;
	}

	if (!CopyFile(sOrigPath.Get(), sNewFullPath.Get(), TRUE) || !pNote->Init(pNotePath)) {
		delete pNote;
		return NULL;
	}
	// the result of copying tdt is not checked.
	TString sOrigTDT, sNewTDT;
	if (sOrigTDT.Join(sOrigPath.Get(), TEXT(".tdt")) && 
		sNewTDT.Join(sNewFullPath.Get(), TEXT(".tdt"))) {
		CopyFile(sOrigTDT.Get(), sNewTDT.Get(), TRUE);
	}
	return pNote;
}

/////////////////////////////////////////////
// ファイル名変更
/////////////////////////////////////////////

BOOL MemoNote::Rename(LPCTSTR pTopDir, LPCTSTR pNewName)
{
	TString sPath;
	if (!sPath.GetDirectoryPath(pPath)) return FALSE;

	if (_tcslen(pNewName) == 0) {
		SetLastError(ERROR_NO_DATA);
		return FALSE;
	}

	// 新しいpPathの領域確保
	DWORD nBaseLen = _tcslen(sPath.Get());
	LPTSTR pNewPath = new TCHAR[nBaseLen + _tcslen(pNewName) + _tcslen(GetExtension()) + 6 + 1];
	if (pNewPath == NULL) return FALSE;

	// 新pPath生成
	_tcscpy(pNewPath, sPath.Get());
	DropInvalidFileChar(pNewPath + nBaseLen, pNewName);
	_tcscat(pNewPath + nBaseLen, GetExtension());

	// ファイル名リネーム用パス生成
	TString sOldFullPath;
	TString sNewFullPath;
	if (!sOldFullPath.Join(pTopDir, TEXT("\\"), pPath) ||
		!sNewFullPath.Join(pTopDir, TEXT("\\"), pNewPath)) {
		delete [] pNewPath;
		return FALSE;
	}

	// ファイル名リネーム実行
	if (!MoveFile(sOldFullPath.Get(), sNewFullPath.Get())) {
		delete [] pNewPath;
		return FALSE;
	}

	// *.tdtのリネームの実行
	MemoInfo mi(pTopDir);
	mi.RenameInfo(sOldFullPath.Get(), sNewFullPath.Get());

	delete [] pPath;
	pPath = pNewPath;

	return TRUE;
}

/////////////////////////////////////////////
// Check is this file memo?
/////////////////////////////////////////////

DWORD MemoNote::IsNote(LPCTSTR pFile)
{
	DWORD len = _tcslen(pFile);
	if (len <= 4) return NOTE_TYPE_NO;

	LPCTSTR p = pFile + len - 4;

	DWORD nType;
	if (_tcsicmp(p, TEXT(".txt")) == 0) {
		nType = NOTE_TYPE_PLAIN;
	} else if (_tcsicmp(p, TEXT(".chi")) == 0) {
		nType = NOTE_TYPE_CRYPTED;
	} else if (_tcsicmp(p, TEXT(".chs")) == 0) {
		nType = NOTE_TYPE_CRYPTED;
	} else if (_tcsicmp(p, TEXT(".tdt")) == 0) {
		nType = NOTE_TYPE_TDT;
	} else {
		nType = NOTE_TYPE_NO;
	}
	return nType;
}

/////////////////////////////////////////////
// set the note path
/////////////////////////////////////////////

BOOL MemoNote::SetMemoPath(LPCTSTR p)
{
	LPTSTR pNewPath = StringDup(p);
	if (pNewPath == NULL) return FALSE;
	delete [] pPath;
	pPath = pNewPath;
	return TRUE;
}

/////////////////////////////////////////////
// MemoNote object factory
/////////////////////////////////////////////

BOOL MemoNote::MemoNoteFactory(LPCTSTR pFile, MemoNote **ppNote)
{
	*ppNote = NULL;

	DWORD nType = IsNote(pFile);
	if (nType == NOTE_TYPE_NO || nType == NOTE_TYPE_TDT) return TRUE;
	if (nType == NOTE_TYPE_PLAIN) {
		*ppNote = new PlainMemoNote();
	} else {
		*ppNote = new CryptedMemoNote();
	}

	if (*ppNote == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}

	if (!(*ppNote)->Init(pFile)) {
		delete (*ppNote);
		*ppNote = NULL;
		return FALSE;
	}
	return TRUE;
}

MemoNote *MemoNote::MemoNoteFactory(const TomboURI *pURI)
{
	LPCTSTR pURIPath = pURI->GetPath() + 1;
	LPTSTR pBuf = StringDup(pURIPath);
	if (pBuf == NULL) return NULL;

	// replace '/' to '\'
	LPTSTR p = pBuf;
	while(p) {
		p = _tcschr(p, TEXT('/'));
		if (p) {
			*p = TEXT('\\');
		}
	}
	MemoNote *pNote = NULL;
	MemoNote::MemoNoteFactory(pBuf, &pNote);
	delete [] pBuf;
	return pNote;
}
