#include <windows.h>
#include <tchar.h>
#include "AutoPtr.h"
#include "Region.h"
#include "VarBuffer.h"
#include "LineWrapper.h"
#include "LineWrapperImpl.h"
#include "YAEdit.h"
#include "MemManager.h"
#include "TomboLib/UniConv.h"
#include "LineManager.h"
#include "PhysicalLineManager.h"
#include "StringSplitter.h"
#include "TString.h"
#include "YAEditDoc.h"

/////////////////////////////////////////////////////////////////////////////
// undo info
/////////////////////////////////////////////////////////////////////////////

UndoInfo::UndoInfo() : bUndoApplied(FALSE), bOpenRegion(TRUE)
{
}


UndoInfo::~UndoInfo()
{
}

BOOL UndoInfo::CmdUndo(YAEditDoc *pDoc) {
	if (bUndoApplied) {
		bUndoApplied = FALSE;
		return pDoc->ReplaceString(&rPrevRegion, sNewStr.Get(), TRUE);
	} else {
		bUndoApplied = TRUE;
		return pDoc->ReplaceString(&rNewRegion, sPrevStr.Get(), TRUE);
	}
}

BOOL UndoInfo::UpdateUndoRegion(const Region *pPrevRegion, LPCTSTR pPrev, const Region *pNewRegion, LPCTSTR pNew) {
	rPrevRegion = *pPrevRegion;
	if (!sPrevStr.Set(pPrev)) return FALSE;
//	pPrevStr = pPrev;

	rNewRegion = *pNewRegion;
	return sNewStr.Set(pNew);
}

/////////////////////////////////////////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////////////////////////////////////////

YAEditDoc::YAEditDoc() : 
	pPhLineMgr(NULL), pCallback(NULL), pListener(NULL), 
	pUndo(NULL), bReadOnly(FALSE)
{
}

YAEditDoc::~YAEditDoc()
{
	if (pPhLineMgr) delete pPhLineMgr;
	if (pUndo) delete pUndo;
}

BOOL YAEditDoc::Init(const char *pStr, YAEditListener *pL, YAEditCallback*pCb)
{
	pCallback = pCb;
	pListener = pL;
	if (!ReleaseDoc()) return FALSE;
	return LoadDoc(pStr);
}

/////////////////////////////////////////////////////////////////////////////
// load document
/////////////////////////////////////////////////////////////////////////////

BOOL YAEditDoc::LoadDoc(const char *pStr)
{
	ReleaseDoc();
	LPTSTR pStrT;
#ifdef _WIN32_WCE
	pStrT = ConvSJIS2Unicode(pStr);
#else
	pStrT = (LPTSTR)pStr;
#endif

	if (!pPhLineMgr->LoadDoc(pStrT)) return FALSE;
#ifdef _WIN32_WCE
	delete [] pStrT;
#endif

	SetModify(FALSE);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// save document
/////////////////////////////////////////////////////////////////////////////

char *YAEditDoc::GetDocumentData(LPDWORD pLen)
{
	return pPhLineMgr->GetDocumentData(pLen);
}

/////////////////////////////////////////////////////////////////////////////
// release document memory
/////////////////////////////////////////////////////////////////////////////

BOOL YAEditDoc::ReleaseDoc()
{
	// LineManager depends on MarkerAllocator, delete/new order is important.
	if (pPhLineMgr) delete pPhLineMgr;

	pPhLineMgr = new PhysicalLineManager();
	if (pPhLineMgr == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}
	if (!pPhLineMgr->Init()) return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Replace string
/////////////////////////////////////////////////////////////////////////////

BOOL YAEditDoc::ReplaceString(const Region *pDelRegion, LPCTSTR pString, BOOL bKeepUndo)
{
	// if read only mode, ignore editing.
	if (bReadOnly) return TRUE;

	if (pUndo == NULL) {
		pUndo = new UndoInfo();
		if (pUndo == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }
	}

	DWORD nPhLinesBefore = pPhLineMgr->MaxLine();

	LPTSTR pPrevText = pPhLineMgr->GetRegionString(pDelRegion);

	// delete region and insert string
	Region rNewRegion;
	DWORD nAffLines;
	if (!pPhLineMgr->ReplaceRegion(pDelRegion, pString, &nAffLines, &rNewRegion)) {
		return FALSE;
	}

	// preserve string and region after replaced.
	if (!bKeepUndo) {
		if (!pUndo->UpdateUndoRegion(pDelRegion, pPrevText, &rNewRegion, pString)) {
			return FALSE;
		}
	} else {
		delete [] pPrevText;
	}

	// notify to view
	DWORD nPhLinesAfter = pPhLineMgr->MaxLine();
	if (pListener && !pListener->UpdateNotify(pPhLineMgr, pDelRegion, &rNewRegion, nPhLinesBefore, nPhLinesAfter, nAffLines)) {
		return FALSE;
	}
	SetModify(TRUE);

	return TRUE;
}

BOOL YAEditDoc::Undo()
{
	if (pUndo == NULL) return TRUE;

	BOOL bResult = pUndo->CmdUndo(this);
	return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// update status
/////////////////////////////////////////////////////////////////////////////

void YAEditDoc::SetModify(BOOL b)
{
	if (bModified == b) return;
	BOOL bOld = bModified;
	bModified = b;
	if (pCallback) pCallback->ChangeModifyStatusNotify(bModified);
}

void YAEditDoc::SetReadOnly(BOOL b)
{
	bReadOnly = b;
}

/////////////////////////////////////////////////////////////////////////////
// update modify status
/////////////////////////////////////////////////////////////////////////////

DWORD YAEditDoc::GetDataBytes(const Region *pRegion)
{
	if (pRegion->posStart.row == pRegion->posEnd.row) {
		return pRegion->posEnd.col - pRegion->posStart.col;
	} else {
		DWORD nBytes = 0;
		LineInfo *pInfo;
		pInfo= pPhLineMgr->GetLineInfo(pRegion->posStart.row);

		// first line
		nBytes = pInfo->pLine->nUsed - pRegion->posStart.col + 2; // +2 means CRLF

		for (DWORD i = pRegion->posStart.row + 1; i < pRegion->posEnd.row; i++) {
			pInfo = pPhLineMgr->GetLineInfo(i);
			nBytes += pInfo->pLine->nUsed + 2;
		}

		// last line
		nBytes += pRegion->posEnd.col;

		return nBytes;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Convert data bytes to physical position
/////////////////////////////////////////////////////////////////////////////

void YAEditDoc::ConvertBytesToCoordinate(DWORD nPos, Coordinate *pPos)
{
	DWORD nBytes = 0;
	LineInfo *p = NULL;

	DWORD n = pPhLineMgr->MaxLine();
	DWORD i = 0;

	do {
		p = pPhLineMgr->GetLineInfo(i);

		if (nBytes + p->pLine->nUsed >= nPos) {
			// in the line
			pPos->row = i;
			pPos->col = nPos - nBytes;
			return;
		}
		nBytes += p->pLine->nUsed + 2;
		i++;
		if (i >= n) break;

		if (nBytes >= nPos) {
			pPos->row = i;
			pPos->col = 0;
			return;
		}

	} while (TRUE);

	// if pos is grater than docment size, set EOL
	pPos->row = n - 1;
	pPos->col = p->pLine->nUsed;
}
