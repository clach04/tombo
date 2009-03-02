#include <windows.h>
#include <tchar.h>
#include "VarBuffer.h"
#include "MemManager.h"
#include "PhysicalLineManager.h"
#include "TString.h"
#include "Region.h"
#include "TomboLib/Uniconv.h"
#include "StringSplitter.h"

#define INITIAL_ARRAY_SIZE 1000
#define ARRAY_DELTA 100

/////////////////////////////////////////////////////////////////////////////
// ctor & dtor, initializer
/////////////////////////////////////////////////////////////////////////////

PhysicalLineManager::PhysicalLineManager() : pMemMgr(NULL)
{
}

PhysicalLineManager::~PhysicalLineManager()
{
	if (pMemMgr) delete pMemMgr;
}


BOOL PhysicalLineManager::Init()
{
	if (!aliLine.Init(INITIAL_ARRAY_SIZE, ARRAY_DELTA)) return FALSE;
	if ((pMemMgr = new MemManager()) == NULL || !pMemMgr->Init()) return FALSE;
	bModify = FALSE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// retrieve line info
/////////////////////////////////////////////////////////////////////////////

LPTSTR PhysicalLineManager::GetLine(DWORD n) 
{
	if (n >= MaxLine()) return NULL;
	LineInfo *p = GetLineInfo(n);
	return p->pLine->GetDataArea();
}

LineInfo *PhysicalLineManager::GetLineInfo(DWORD nLine)
{
	if (nLine >= MaxLine()) return NULL;
	return aliLine.GetUnit(nLine);
}

DWORD PhysicalLineManager::LineChars(DWORD n)
{
	if (n>= MaxLine()) return 0;
	return aliLine.GetUnit(n)->pLine->nUsed;
}

/////////////////////////////////////////////////////////////////////////////
// append line to end of the document
/////////////////////////////////////////////////////////////////////////////

BOOL PhysicalLineManager::AppendLine(MemBlock *pBlock)
{
	LineInfo li;
	li.pLine = pBlock;
	return aliLine.Add(&li);
}

/////////////////////////////////////////////////////////////////////////////
// replace line
/////////////////////////////////////////////////////////////////////////////

BOOL PhysicalLineManager::UpdateBlock(DWORD nLineNo, MemBlock *pNewBlk)
{
	if (nLineNo >= MaxLine()) return FALSE;
	LineInfo *p = aliLine.GetUnit(nLineNo);
	p->pLine = pNewBlk;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// insert line
/////////////////////////////////////////////////////////////////////////////

BOOL PhysicalLineManager::InsertLine(DWORD nLineNo, LPCTSTR pData, DWORD nLen)
{
	MemBlock *pBlock = pMemMgr->Alloc(pData, nLen);
	if (pBlock == NULL) return FALSE;

	if (!aliLine.Extend(nLineNo, 1)) return FALSE;
	LineInfo *p = aliLine.GetUnit(nLineNo);
	p->pLine = pBlock;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// delete line
/////////////////////////////////////////////////////////////////////////////
// MemBlock are not released automatically. Release manually if you want.

BOOL PhysicalLineManager::DeleteLine(DWORD nLineNo, DWORD nLines)
{
	if (!aliLine.Shorten(nLineNo, nLines)) return FALSE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// delete region
/////////////////////////////////////////////////////////////////////////////
BOOL PhysicalLineManager::DeleteRegion(const Region *pRegion)
{
	DWORD nNewEnd;
	return ReplaceRegion(pRegion, TEXT(""), &nNewEnd, NULL);
}

/////////////////////////////////////////////////////////////////////////////
// Insert string
/////////////////////////////////////////////////////////////////////////////

BOOL PhysicalLineManager::InsertString(DWORD nLineNo, DWORD nStart, LPCTSTR pStr, DWORD nLen)
{
	LineInfo *pInfo = aliLine.GetUnit(nLineNo);
	if (!pInfo) return FALSE;

	MemBlock *pBlk = pInfo->pLine;
	MemBlock *pNewBlk = pMemMgr->Extend(pBlk, (WORD)nStart, (WORD)nLen);
	if (pNewBlk == NULL) return FALSE;

	if (pBlk != pNewBlk) {
		UpdateBlock(nLineNo, pNewBlk);
		pInfo->pLine = pNewBlk;
	}
	LPTSTR pData = pNewBlk->GetDataArea();
	memcpy(pData + nStart, pStr, nLen * sizeof(TCHAR));
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// delete string
/////////////////////////////////////////////////////////////////////////////

BOOL PhysicalLineManager::DeleteString(DWORD nLineNo, DWORD nStart, DWORD nLen)
{
	LineInfo *pInfo = aliLine.GetUnit(nLineNo);
	if (!pInfo) return FALSE;

	MemBlock *pBlk = pInfo->pLine;

	LPTSTR pData = pBlk->GetDataArea();
	memmove(pData + nStart, pData + nStart + nLen, pBlk->nUsed - nStart - nLen);
	pBlk->nUsed -= (WORD)nLen;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Load document
/////////////////////////////////////////////////////////////////////////////

BOOL PhysicalLineManager::LoadDoc(LPCTSTR pStr)
{
	// release document index and data area
	if (!aliLine.Clear(TRUE)) return FALSE;
	delete pMemMgr;
	if ((pMemMgr = new MemManager()) == NULL || !pMemMgr->Init()) return FALSE;

	MemBlock *pBlock;
	// check empty document

	if (*pStr == TEXT('\0')) {
		pBlock = pMemMgr->Alloc(TEXT(""), 0);
		if (pBlock == NULL || !AppendLine(pBlock)) return FALSE;
		return TRUE;
	}

	// chop each line
	LPCTSTR q = pStr;
	LPCTSTR pPrevTop = pStr;

	while(*q) {
		if (*q == TEXT('\r') || *q == TEXT('\n')) {
			// insert line
			pBlock = pMemMgr->Alloc(pPrevTop, q - pPrevTop);
			if (pBlock == NULL || !AppendLine(pBlock)) return FALSE;

			if (*q == TEXT('\r') && *(q+1) == TEXT('\n')) {
				q = CharNext(q);
			}
			q = CharNext(q);
			pPrevTop = q;

			if (*q == TEXT('\0')) {
				pBlock = pMemMgr->Alloc(pPrevTop, 0);
				if (pBlock == NULL || !AppendLine(pBlock)) return FALSE;				
				return TRUE;
			}

			continue;
		}
		q = CharNext(q);
	}

	if (pPrevTop != q) {
		pBlock = pMemMgr->Alloc(pPrevTop, q - pPrevTop);
		if (pBlock == NULL || !AppendLine(pBlock)) return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Load document
/////////////////////////////////////////////////////////////////////////////

char *PhysicalLineManager::GetDocumentData(LPDWORD pLen)
{
	DWORD nSize = 0;

	// Count buffer size
	DWORD nLine = aliLine.NumItems();
	DWORD i;
	for (i = 0; i < nLine; i++) {
		MemBlock *p = aliLine.GetUnit(i)->pLine;

		nSize += p->nUsed;
		nSize += 2;
	}
	nSize -= 2;

	LPTSTR pData = new TCHAR[nSize + 1];
	if (pData == NULL) return NULL;
	LPTSTR q = pData;
	for (i = 0; i < nLine; i++) {
		MemBlock *p = aliLine.GetUnit(i)->pLine;

		_tcsncpy(q, p->GetDataArea(), p->nUsed);
		q += p->nUsed;
		if (i < nLine - 1) {
			_tcscpy(q, TEXT("\r\n"));
			q += 2;
		}
	}
	*q = TEXT('\0');

	char *pResultData;
#ifdef _WIN32_WCE
	pResultData = ConvUnicode2SJIS(pData);
	*pLen = strlen(pResultData);
	delete [] pData;
#else
	pResultData = pData;
	*pLen = nSize;
#endif
	return pResultData;
}

/////////////////////////////////////////////////////////////////////////////
// Get string in the region
/////////////////////////////////////////////////////////////////////////////

LPTSTR PhysicalLineManager::GetRegionString(const Region *pRegion)
{
	if (pRegion->posStart.row == pRegion->posEnd.row) {
		MemBlock *pBlock = aliLine.GetUnit(pRegion->posStart.row)->pLine;

		DWORD nLen = pBlock->nUsed;
		LPTSTR pLine = pBlock->GetDataArea();

		DWORD nRegionSize = pRegion->posEnd.col - pRegion->posStart.col;
		LPTSTR p = new TCHAR[nRegionSize + 1];
		if (p == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return NULL; }

		_tcsncpy(p, pLine + pRegion->posStart.col, nRegionSize);
		*(p + nRegionSize) = TEXT('\0');
		return p;

	} else {
		// get data size
		DWORD nRegionSize = 0;

		MemBlock *pFirstBlock = aliLine.GetUnit(pRegion->posStart.row)->pLine;
		DWORD nFirstLen = pFirstBlock->nUsed - pRegion->posStart.col;
		nRegionSize += nFirstLen;

		nRegionSize += 2;	// CRLF
		MemBlock *pLastBlock = aliLine.GetUnit(pRegion->posEnd.row)->pLine;
		DWORD nLastLen = pRegion->posEnd.col;
		nRegionSize += nLastLen;
		DWORD i;
		for (i = pRegion->posStart.row + 1; i < pRegion->posEnd.row; i++) {
			MemBlock *pBlock = aliLine.GetUnit(i)->pLine;
			DWORD nLen = pBlock->nUsed;
			nRegionSize += nLen;
			nRegionSize += 2; // CRLF;
		}

		// allocate buffer
		LPTSTR p = new TCHAR[nRegionSize + 1];
		if (p == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return NULL; }

		// padding data
		LPTSTR q = p;
		_tcsncpy(q, pFirstBlock->GetDataArea() + pRegion->posStart.col, nFirstLen);
		q += nFirstLen;

		_tcsncpy(q, TEXT("\r\n"), 2); q += 2;

		for (i = pRegion->posStart.row + 1; i < pRegion->posEnd.row; i++) {
			MemBlock *pBlock = aliLine.GetUnit(i)->pLine;
			_tcsncpy(q, pBlock->GetDataArea(), pBlock->nUsed);
			q += pBlock->nUsed;
			_tcsncpy(q, TEXT("\r\n"), 2); q += 2;			
		}

		_tcsncpy(q, pLastBlock->GetDataArea(), nLastLen);
		q += nLastLen;
		*q = TEXT('\0');
		return p;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Replace string
/////////////////////////////////////////////////////////////////////////////

BOOL PhysicalLineManager::ReplaceRegion(const Region *pRegion, LPCTSTR pString, LPDWORD pAffectedLines, Region *pNewRegion)
{
	// parse and split replase string
	StringSplitter ss;
	if (!ss.Init() || !ss.Parse(pString)) return FALSE;
	*pAffectedLines = ss.MaxLine();

	// keep first and last line of curent memblock;
	MemBlock *pFirstLine;
	MemBlock *pLastLine;

	pFirstLine = aliLine.GetUnit(pRegion->posStart.row)->pLine;
	if (pRegion->posStart.row != pRegion->posEnd.row) {
		pLastLine = aliLine.GetUnit(pRegion->posEnd.row)->pLine;
	} else {
		pLastLine = NULL;
	}

	DWORD i;
	// free unused block;
	if (pRegion->posEnd.row - pRegion->posStart.row >= 1) {
		for (i = pRegion->posStart.row + 1; i <= pRegion->posEnd.row - 1; i++) {
			pMemMgr->Free(aliLine.GetUnit(i)->pLine);
		}
	}

	// modify physical line vector
	DWORD nPrevLines = pRegion->posEnd.row - pRegion->posStart.row + 1;
	DWORD nAfterLines = ss.MaxLine();
	if (nPrevLines > nAfterLines) {
		aliLine.Shorten(pRegion->posStart.row, nPrevLines - nAfterLines);
	} else {
		aliLine.Extend(pRegion->posStart.row, nAfterLines - nPrevLines);
	}

	// link appended line except first and last line.
	MemBlock *pAppend;
	DWORD nMidStart = pRegion->posStart.row;
	for (i = 1; i < ss.MaxLine() - 1; i++) {
		pAppend = pMemMgr->Alloc(pString + ss.GetBegin(i), ss.GetEnd(i) - ss.GetBegin(i));
		if (pAppend == NULL) {
			// TODO: error handling
			return FALSE;
		}
		aliLine.GetUnit(nMidStart + i)->pLine = pAppend;
	}

	pAppend = pMemMgr->Alloc(pFirstLine->GetDataArea(), pRegion->posStart.col);
	if (pAppend == NULL) return FALSE;
	pAppend = pMemMgr->ConCat(pAppend, pString + ss.GetBegin(0), ss.GetEnd(0) - ss.GetBegin(0));
	if (pAppend == NULL) return FALSE;

	DWORD nLastAppend;
	if (ss.MaxLine() == 1) {
		nLastAppend = 0;
	} else {
		// top line
		aliLine.GetUnit(pRegion->posStart.row)->pLine = pAppend;

		// last line
		nLastAppend = ss.MaxLine() - 1;
		pAppend = pMemMgr->Alloc(pString + ss.GetBegin(nLastAppend), ss.GetEnd(nLastAppend) - ss.GetBegin(nLastAppend));
		if (pAppend == NULL) return FALSE;
	}

	if (pLastLine) {
		pAppend = pMemMgr->ConCat(pAppend, pLastLine->GetDataArea() + pRegion->posEnd.col, pLastLine->nUsed - pRegion->posEnd.col);
	} else {
		// replace in one line
		pAppend = pMemMgr->ConCat(pAppend, pFirstLine->GetDataArea() + pRegion->posEnd.col, pFirstLine->nUsed - pRegion->posEnd.col);
	}
	if (pAppend == NULL) return FALSE;
	aliLine.GetUnit(pRegion->posStart.row + nLastAppend)->pLine = pAppend;

	// free old memblocks
	pMemMgr->Free(pFirstLine);
	if (pLastLine) pMemMgr->Free(pLastLine);

	// set pNewRegion
	if (pNewRegion) {
		pNewRegion->posStart = pRegion->posStart;
		pNewRegion->posEnd.row = pRegion->posStart.row + ss.MaxLine() - 1;

		if (ss.MaxLine() == 1) {
			pNewRegion->posEnd.col = pRegion->posStart.col + ss.GetEnd(ss.MaxLine() - 1);
		} else {
			pNewRegion->posEnd.col = ss.GetEnd(ss.MaxLine() - 1) - ss.GetBegin(ss.MaxLine() - 1);
		}
	}

	return TRUE;
}

