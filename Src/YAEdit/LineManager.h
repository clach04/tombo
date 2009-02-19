#ifndef LINEMANAGER_H
#define LINEMANAGER_H

class LineWrapper;
class PhysicalLineManager;
struct LineInfo;
class MemBlock;
class Coordinate;
class Region;
class YAEditDoc;
class YAEditImpl;

/////////////////////////////////////////////////////////////////////////////
// Line chunk
/////////////////////////////////////////////////////////////////////////////
// Line chunk is wrapper for logical line data.
//
// In future plan, impliment GC and LineChunk controls locking contorl, so 
// You should not allocate LineChunk to heap area.
// For same resons, LineChunk result especially GetLineData() may updated
// by other thread. don't keep its results pointer after LineChunk object
// has deleted.

class LineChunk {
	LPCTSTR pLine;
	DWORD nLen;
	BOOL bIsContLine;

	DWORD nLineNo;
	const Region *pSelRegion;
public:
	LineChunk() : pLine(NULL), nLen(0), bIsContLine(FALSE), nLineNo(0xFFFFFFFF), pSelRegion(NULL) {}
	~LineChunk() {}

	///////////////////////////////////////
	// logical line data

	LPCTSTR GetLineData() { return pLine; }
	DWORD LineLen() { return nLen; }

	///////////////////////////////////////
	// attribute accessor

	BOOL IsContLine() { return bIsContLine; }

	///////////////////////////////////////
	// line no information
	DWORD GetLineNo() { return nLineNo; }

	///////////////////////////////////////
	// selected region
	void SetSelRegion(const Region *p) { pSelRegion = p; }
	void GetSelRange(LPDWORD pStart, LPDWORD pEnd);

	friend class LineManager;
};

/////////////////////////////////////////////////////////////////////////////
// Logical line element 
/////////////////////////////////////////////////////////////////////////////

class LgLineInfo {
	DWORD nPhyLineNo;
	DWORD nLinePos;

	friend class LineManager;
	friend class LineManagerTest;
};

/////////////////////////////////////////////////////////////////////////////
// Line Management System
/////////////////////////////////////////////////////////////////////////////

class LineManager {
	TVector<LgLineInfo> aliLine; // logical line index

	YAEditImpl *pEdit;

protected:

	BOOL ReMapLine(LineWrapper *pWrapper, DWORD nCalcStartPos, DWORD nPhLineNo, DWORD nLgLineNo, 
					LPCTSTR pData, DWORD nLen, 
					DWORD nPrevLines, LPDWORD pPostLines);

	DWORD LineChars(DWORD n);
	BOOL IsContLine(DWORD n);
	
	DWORD ConvertToPhysicalLine(DWORD nLgLine);
		// Convert Logical line no to Physical line no

	DWORD ConvertPhLineToLgLineTop(DWORD nPhLine);

	BOOL AdjustLines(DWORD nLgLineNo, DWORD nOld, DWORD nNew);
	void GetDeletedLgLines(const Region &rPhDelete, Coordinate *pLgStartPos, LPDWORD pLgAfEndLine);
public:

	////////////////////////////////////////////////////
	// ctor, dtor & initialize

	LineManager();
	~LineManager();
	BOOL Init(YAEditImpl *pEdit);
	BOOL ReleaseBuffer();

	////////////////////////////////////////////////////
	// request rewrapping

	BOOL RecalcWrap(LineWrapper *pWrapper);
	BOOL Assoc(PhysicalLineManager *pMgr, LineWrapper *pWrapper);

	////////////////////////////////////////////////////
	// refer logical line.

	DWORD MaxLine() { return aliLine.NumItems(); }
	BOOL GetLineChunk(DWORD n, LineChunk *pChunk);

	////////////////////////////////////////////////////
	// logical position <-> physical position conversion

	void LogicalPosToPhysicalPos(const Coordinate *pLgPos, Coordinate *pPhPos);
	void PhysicalPosToLogicalPos(const Coordinate *pPhPos, Coordinate *pLgPos);
		// In current implimentation, Ph->Lg conversion is more heavy than Lg->Ph conversions.

	////////////////////////////////////////////////////
	// corresponds to physical line changes

	BOOL AdjustLgLines(PhysicalLineManager *pMgr, LineWrapper *pWrapper, const Region &rPhDelete, DWORD nPhBeforeLines, DWORD nPhAfterLines, DWORD nPhAffectedLine, Coordinate *pLgAfStart, LPDWORD pAffLines);

	friend class LineManagerTest;
};

#endif
