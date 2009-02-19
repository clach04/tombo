#include <windows.h>

#include "VarBuffer.h"
#include "Region.h"
#include "LineManager.h"
#include "MemManager.h"
#include "LineWrapper.h"
#include "PhysicalLineManager.h"
#include "YAEdit.h"
#include "YAEditDoc.h"

#define INITIAL_ARRAY_SIZE 1000
#define ARRAY_DELTA 100

/////////////////////////////////////////////////////////////////////////////
// ctor & dtor, initializer
/////////////////////////////////////////////////////////////////////////////

LineManager::LineManager() : pEdit(NULL)
{
}

LineManager::~LineManager()
{
}

BOOL LineManager::Init(YAEditImpl *pEd)
{
	pEdit = pEd;
	if (!aliLine.Init(INITIAL_ARRAY_SIZE, ARRAY_DELTA)) return FALSE;
	return TRUE;
}

BOOL LineManager::ReleaseBuffer()
{
	if (MaxLine() != 0 && !aliLine.Clear(TRUE)) return FALSE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Reference logical line.
/////////////////////////////////////////////////////////////////////////////

DWORD LineManager::LineChars(DWORD nLgLine)
{
	LgLineInfo *pInfo = aliLine.GetUnit(nLgLine);
	LgLineInfo *pNext = aliLine.GetUnit(nLgLine + 1);

	if (nLgLine + 1 >= MaxLine() || pNext->nPhyLineNo != pInfo->nPhyLineNo) {
		// Get the end position of physical line
		DWORD nPhLine = pInfo->nPhyLineNo;
		MemBlock *pBlock = pEdit->GetDoc()->GetPhMgr()->GetLineInfo(nPhLine)->pLine;
		return pBlock->nUsed - pInfo->nLinePos;
	} else {
		return pNext->nLinePos - pInfo->nLinePos; 
	}
}

BOOL LineManager::IsContLine(DWORD nLgLine)
{
	LgLineInfo *pInfo = aliLine.GetUnit(nLgLine);
	LgLineInfo *pNext = aliLine.GetUnit(nLgLine + 1);
	if (nLgLine + 1 >= MaxLine() || pNext->nPhyLineNo != pInfo->nPhyLineNo) {
		return FALSE;
	} else {
		return TRUE;
	}

}

BOOL LineManager::AdjustLines(DWORD nLgLineNo, DWORD nOld, DWORD nNew)
{
	if (nOld == nNew) return TRUE;
	if (nOld > nNew) {
		// dec line
		return aliLine.Shorten(nLgLineNo + nNew, nOld - nNew);
	} else {
		// inc line
		return aliLine.Extend(nLgLineNo, nNew - nOld);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Convert physical line to logical line that is top of same physical line.
/////////////////////////////////////////////////////////////////////////////

DWORD LineManager::ConvertPhLineToLgLineTop(DWORD nPhLine)
{
	DWORD nStart = 0;
	DWORD nEnd = MaxLine();
	DWORD nMid, nMidPhLine;

	while(nStart <= nEnd) {
		nMid = (nStart + nEnd) / 2;
		nMidPhLine = aliLine.GetUnit(nMid)->nPhyLineNo;
		if (nPhLine == nMidPhLine) break;
		if (nPhLine > nMidPhLine) {
			nStart = nMid + 1;
		} else {
			nEnd = nMid - 1;
		}
	}
	if (nPhLine == nMidPhLine) {
		while(nMid > 0 && aliLine.GetUnit(nMid - 1)->nPhyLineNo == nPhLine) {
			nMid--;
		}
		return nMid;
	} else {
		return 0xFFFFFFFF;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Convert logical line to physical line
/////////////////////////////////////////////////////////////////////////////

DWORD LineManager::ConvertToPhysicalLine(DWORD nLgLine)
{
	LgLineInfo *pLgInfo = aliLine.GetUnit(nLgLine);
	if (pLgInfo == NULL) return NULL;
	return pLgInfo->nPhyLineNo;
}

/////////////////////////////////////////////////////////////////////////////
// line position conversion
/////////////////////////////////////////////////////////////////////////////

// (LgLineNo, LgPos) -> (PhLineNo, PhPos)

void LineManager::LogicalPosToPhysicalPos(const Coordinate *pLgPos, Coordinate *pPhPos)
{
	DWORD nLgLineNo = pLgPos->row;
	pPhPos->row = ConvertToPhysicalLine(nLgLineNo);
	LgLineInfo *pLi = aliLine.GetUnit(nLgLineNo);
	pPhPos->col = pLi->nLinePos + pLgPos->col;
}

// (PhLineNo, PhPos) -> (LgLineNo, LgPos)
void LineManager::PhysicalPosToLogicalPos(const Coordinate *pPhPos, Coordinate *pLgPos)
{
	DWORD nPhLineNo = pPhPos->row;
	DWORD nLgLineTop = ConvertPhLineToLgLineTop(nPhLineNo);

	DWORD nPos = pPhPos->col;
	DWORD i = nLgLineTop;
	while(i + 1 < aliLine.NumItems()) {
		LgLineInfo *pLi = aliLine.GetUnit(i + 1);
		if (pLi->nPhyLineNo != nPhLineNo || pLi->nLinePos > nPos) break;
		i++;
	}
	pLgPos->row = i;
	pLgPos->col = pPhPos->col - aliLine.GetUnit(i)->nLinePos;
}

/////////////////////////////////////////////////////////////////////////////
// Re-wrapping
/////////////////////////////////////////////////////////////////////////////

BOOL LineManager::RecalcWrap(LineWrapper *pWrapper)
{
	BOOL b = Assoc(pEdit->GetDoc()->GetPhMgr(), pWrapper);
	return b;
}

/////////////////////////////////////////////////////////////////////////////
// ReMap line
/////////////////////////////////////////////////////////////////////////////

BOOL LineManager::ReMapLine(LineWrapper *pWrapper, DWORD nCalcStartPos, DWORD nPhLineNo, DWORD nLgLineNo, 
							LPCTSTR pData, DWORD nLen, 
							DWORD nPrevLines, LPDWORD pPostLines)
{
	TVector<DWORD> vSep;

	if (!vSep.Init(10, 10)) return FALSE;
	if (!vSep.Add(&nCalcStartPos)) return FALSE;

	if (!pWrapper->GetNewMarkerList(&vSep, nCalcStartPos, pData, nLen)) return FALSE;
	*pPostLines = vSep.NumItems() - 1;

	if (!AdjustLines(nLgLineNo, nPrevLines, *pPostLines)) return FALSE;

	LgLineInfo *pInfo;
	for (DWORD i = 0; i < *pPostLines; i++) {
		pInfo = aliLine.GetUnit(nLgLineNo + i);
		pInfo->nPhyLineNo = nPhLineNo;
		pInfo->nLinePos = *(vSep.GetUnit(i));
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Get line chunk
/////////////////////////////////////////////////////////////////////////////

BOOL LineManager::GetLineChunk(DWORD nLgLine, LineChunk *pChunk)
{
	LgLineInfo *p = aliLine.GetUnit(nLgLine);
	LPTSTR pStr = pEdit->GetDoc()->GetPhMgr()->GetLine(p->nPhyLineNo);
	pChunk->pLine = pStr + p->nLinePos; 

	pChunk->nLen = LineChars(nLgLine);
	pChunk->bIsContLine = IsContLine(nLgLine);
	pChunk->nLineNo = nLgLine;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////////////

void LineChunk::GetSelRange(LPDWORD pStart, LPDWORD pEnd)
{
	if (pSelRegion->posStart.row <= nLineNo && nLineNo <= pSelRegion->posEnd.row) {
		if (nLineNo == pSelRegion->posStart.row) {
			*pStart = pSelRegion->posStart.col;
		} else {
			*pStart = 0;
		}
		if (nLineNo == pSelRegion->posEnd.row) {
			*pEnd = pSelRegion->posEnd.col;
		} else {
			*pEnd = LineLen();
		}
	} else {
		*pStart = *pEnd = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Associate with given PhsicalLineManager
/////////////////////////////////////////////////////////////////////////////

BOOL LineManager::Assoc(PhysicalLineManager *pMgr, LineWrapper *pWrapper)
{
	DWORD k;
	DWORD n = pMgr->MaxLine();
	LPTSTR pLine;
	DWORD nLen, nLgLines;

	if (!aliLine.Clear(FALSE)) return FALSE;
	for (k = 0; k < n; k++) {
		DWORD nLgLineNo = MaxLine();

		pLine = pMgr->GetLine(k);
		nLen = pMgr->LineChars(k);

		if (!ReMapLine(pWrapper, 0, k, nLgLineNo, pLine, nLen, 0, &nLgLines)) return FALSE;
	}
	return TRUE;
}

void LineManager::GetDeletedLgLines(const Region &rPhDelete, Coordinate *pLgStartPos, LPDWORD pLgAfEndLine)
{
	// convert Physical region to Logical region
	PhysicalPosToLogicalPos(&(rPhDelete.posStart), pLgStartPos);

	Coordinate cLgAfEnd;
	PhysicalPosToLogicalPos(&(rPhDelete.posEnd), &cLgAfEnd);

	// get max logical lineno whose PhLineNo is same.
	DWORD nRow = cLgAfEnd.row;
	DWORD n = MaxLine();
	while (nRow + 1 < n) {
		if (aliLine.GetUnit(nRow + 1)->nPhyLineNo == rPhDelete.posEnd.row) {
			nRow++;
		} else {
			break;
		}
	}
	*pLgAfEndLine = nRow;
}

BOOL LineManager::AdjustLgLines(PhysicalLineManager *pMgr, LineWrapper *pWrapper, const Region &rPhDelete, DWORD nPhBeforeLines, DWORD nPhAfterLines, DWORD nPhAffectedLine, Coordinate *pLgAfStart, LPDWORD pAffLines)
{
	// TODO1: merge vWork and vSep
	// TODO2: wrap part of line especially when rPhDelete.posStart.col is not first logical line in the physical line, 

	// get logical line range that LineManager should remove.
	DWORD nLgAfStartLine, nLgAfEndLine;
	Region r = rPhDelete;
	r.posStart.col = 0;
	GetDeletedLgLines(r, pLgAfStart, &nLgAfEndLine);
	nLgAfStartLine = pLgAfStart->row;

	///////////////////////////
	// wrap affected physical lines and save to work area.

	TVector<LgLineInfo> vWork; // work area
	if (!vWork.Init(nPhAffectedLine, nPhAffectedLine / 2 > 0 ? nPhAffectedLine : 1)) return FALSE;
		// The number of logical line is same as physical lines if no wrapping, so allocate first.

	MemBlock *pBlock;
	TVector<DWORD> vSep;
	if (!vSep.Init(10, 10)) return FALSE;
	DWORD i, j, l;
	LgLineInfo li;
	DWORD nZero = 0;

	l = r.posStart.row;
	for (i = 0; i < nPhAffectedLine; i++) {	// for each line
		if (!vSep.Clear(FALSE)) return FALSE;
		if (!vSep.Add(&nZero)) return FALSE;

		pBlock = pMgr->GetLineInfo(l)->pLine;

		if (!pWrapper->GetNewMarkerList(&vSep, 0, pBlock->GetDataArea(), pBlock->nUsed)) return FALSE;
		DWORD n = vSep.NumItems() - 1;

		li.nPhyLineNo = l;
		for (j = 0; j < n; j++) {
			li.nLinePos = *(vSep.GetUnit(j));
			if (!vWork.Add(&li)) return FALSE;
		}

		l++;
	}

	///////////////////////
	// if number of physical line changes, update index
	if (nPhBeforeLines < nPhAfterLines) {
		DWORD nDelta = nPhAfterLines - nPhBeforeLines;
		for (i = nLgAfEndLine + 1; i< aliLine.NumItems(); i++) {
			aliLine.GetUnit(i)->nPhyLineNo += nDelta;
		}
	} else {
		if (nPhBeforeLines > nPhAfterLines) {
			DWORD nDelta = nPhBeforeLines - nPhAfterLines;
			for (i = nLgAfEndLine + 1; i < aliLine.NumItems(); i++) {
				aliLine.GetUnit(i)->nPhyLineNo -= nDelta;
			}
		}
	}

	///////////////////////
	// Adjust lgmgr size

	DWORD nExist = nLgAfEndLine - nLgAfStartLine + 1;
	DWORD nNeed = vWork.NumItems();
	if (nExist < nNeed) {
		if (!aliLine.Extend(nLgAfStartLine, nNeed - nExist)) return FALSE;
	} else {
		if (!aliLine.Shorten(nLgAfStartLine, nExist - nNeed)) return FALSE;
	}

	///////////////////////
	// update lmgr

	LgLineInfo *pSrc, *pDst;
	for (i = 0; i < vWork.NumItems(); i++) {
		pSrc = vWork.GetUnit(i);
		pDst = aliLine.GetUnit(nLgAfStartLine + i);
		pDst->nLinePos = pSrc->nLinePos;
		pDst->nPhyLineNo = pSrc->nPhyLineNo;
	}

	return TRUE;
}
