#include <windows.h>
#include <tchar.h>
#include "Tombo.h"
#include "TString.h"
#include "File.h"
#include "MemoInfo.h"

#define VERSION_INFO 1
#define TDT_FILE_EXT TEXT(".tdt")

BOOL MemoInfo::WriteInfo(LPCTSTR pMemoPath, DWORD nPos)
{
	BYTE cVer = VERSION_INFO;

	TString sFileName;
	if (!sFileName.Join(pTopDir, TEXT("\\"), pMemoPath, TDT_FILE_EXT)) return FALSE;

	File fInfo;
	if (!fInfo.Open(sFileName.Get(), GENERIC_WRITE, 0, CREATE_ALWAYS)) {
		return FALSE;
	}
	if (!fInfo.Write((LPBYTE)&cVer, sizeof(cVer)) ||
		!fInfo.Write((LPBYTE)&nPos, sizeof(nPos))) {
		return FALSE;
	}
	fInfo.Close();
	return TRUE;
}

BOOL MemoInfo::ReadInfo(LPCTSTR pMemoPath, LPDWORD pPos)
{
	BYTE cVer;

	TString sFileName;
	if (!sFileName.Join(pTopDir, TEXT("\\"), pMemoPath, TDT_FILE_EXT)) return FALSE;

	File fInfo;
	if (!fInfo.Open(sFileName.Get(), GENERIC_READ, 0, OPEN_EXISTING)) {
		return FALSE;
	}
	DWORD r1, r2;
	r1 = sizeof(cVer);
	r2 = sizeof(DWORD);
	if (!fInfo.Read((LPBYTE)&cVer, &r1) ||
		!fInfo.Read((LPBYTE)pPos, &r2)) {
		return FALSE;
	}
	if (cVer != VERSION_INFO) {
		*pPos = 0;
	}
	fInfo.Close();
	return TRUE;
}

BOOL MemoInfo::DeleteInfo(LPCTSTR pMemoPath)
{
	TString sFileName;
	if (!sFileName.Join(pTopDir, TEXT("\\"), pMemoPath, TDT_FILE_EXT)) return FALSE;
	return DeleteFile(sFileName.Get());
}

BOOL MemoInfo::RenameInfo(LPCTSTR pOld, LPCTSTR pNew)
{
	TString sOldFileName;
	TString sNewFileName;
	if (!sOldFileName.Join(pOld, TDT_FILE_EXT) || !sNewFileName.Join(pNew, TDT_FILE_EXT)) return FALSE;
	DeleteFile(sNewFileName.Get());
	return MoveFile(sOldFileName.Get(), sNewFileName.Get());
}