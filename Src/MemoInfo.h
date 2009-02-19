#ifndef MEMOINFO_H
#define MEMOINFO_H

////////////////////////////////////////
// Notes properties
////////////////////////////////////////

class MemoInfo {
	LPCTSTR pTopDir;
public:
	MemoInfo(LPCTSTR pTop) : pTopDir(pTop) {}
	~MemoInfo() {}

	// path should partpath :-<
	BOOL WriteInfo(LPCTSTR pMemoPath, DWORD nPos);
	BOOL ReadInfo(LPCTSTR pMemoPath, LPDWORD pPos);

	BOOL DeleteInfo(LPCTSTR pMemoPath);

	// pOldFilePath and pNewFilePath should full path of the note.
	BOOL RenameInfo(LPCTSTR pOldFilePath, LPCTSTR pNewFilePath);
};

#endif