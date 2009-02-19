#ifndef MEMMANAGER_H
#define MEMMANAGER_H

////////////////////////////////////////////////////////////////////////////
// Data Management System
/////////////////////////////////////////////////////////////////////////////

#define MEMBLOCK_TYPE_SENTINEL 0
#define MEMBLOCK_TYPE_NORMAL 1

/////////////////////////////////////////////////////////////////////////////
// Data block header
/////////////////////////////////////////////////////////////////////////////

class MemBlock {
public:
	BYTE nBlockType;
	DWORD nBlockLen;
	DWORD nUsed;
	// in MBCS coding system, nBlockLen and nUsed's unit is bytes.
	// in UCS-2(Unicode) coding system, nBlockLen and nUsed's unit is letters.
	// so, sizeof(TCHAR) times are real memory bytes in UCS-2 coding system.

	LPTSTR GetDataArea() { return (LPTSTR)(((LPBYTE)this) + sizeof(MemBlock)); }
};

/////////////////////////////////////////////////////////////////////////////
// Controller of DMS
/////////////////////////////////////////////////////////////////////////////

class MemManager {
protected:
	HANDLE hHeap;	// Local heap

public:
	MemManager();
	~MemManager();
	BOOL Init();

	MemBlock *Alloc(DWORD nLetters);
	MemBlock *Alloc(LPCTSTR pStr, DWORD nLetters);
	void Free(MemBlock *p);
	MemBlock *Extend(MemBlock *pBlock, DWORD nPos, DWORD nSize);

	MemBlock *ConCat(MemBlock *pBlock, LPCTSTR pStr, DWORD nLetters);
};

#endif
