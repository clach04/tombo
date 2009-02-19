#ifndef PHYSICALLINEMANAGER_H
#define PHYSICALLINEMANAGER_H

class MemBlock;
class MemManager;
class Region;
class StringSplitter;

/////////////////////////////////////////////////////////////////////////////
// Physical line element info
/////////////////////////////////////////////////////////////////////////////

struct LineInfo {
	MemBlock *pLine;
};

/////////////////////////////////////////////////////////////////////////////
// Line Management System(Physical line)
/////////////////////////////////////////////////////////////////////////////

class PhysicalLineManager
{
protected:
	TVector<LineInfo> aliLine; // line index
	MemManager *pMemMgr;

	BOOL bModify;

	BOOL AppendLine(MemBlock *pBlock);
	BOOL UpdateBlock(DWORD nLineNo, MemBlock *pNewBlk);

public:

	PhysicalLineManager();
	~PhysicalLineManager();
	BOOL Init();

	BOOL LoadDoc(LPCTSTR pStr);
	char* GetDocumentData(LPDWORD pLen);

	LPTSTR GetLine(DWORD n);
	LineInfo *GetLineInfo(DWORD nLine);
	DWORD LineChars(DWORD n);

	DWORD MaxLine() { return aliLine.NumItems(); }

	BOOL InsertLine(DWORD nLineNo, LPCTSTR pData, DWORD nLen);
	BOOL DeleteLine(DWORD nLineNo, DWORD nLines);

	BOOL DeleteRegion(const Region *pPhRegion);

	BOOL InsertString(DWORD nLineNo, DWORD nStart, LPCTSTR pData, DWORD nLen);
	BOOL DeleteString(DWORD nLineNo, DWORD nStart, DWORD nLen);

	BOOL ReplaceRegion(const Region *pRegion, LPCTSTR pStr, LPDWORD pAffectedLines, Region *pNewRegion);

	LPTSTR GetRegionString(const Region *pRegion);

	friend class PhysicalLineManagerTest;
};


#endif