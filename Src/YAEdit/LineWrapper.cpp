#include <windows.h>
#include "VarBuffer.h"
#include "LineWrapper.h"
#include "Region.h"
#include "YAEdit.h"

/////////////////////////////////////////////////////////////////////////////
// LinWrapper methods
/////////////////////////////////////////////////////////////////////////////

BOOL LineWrapper::GetNewMarkerList(TVector<DWORD> *pLines, DWORD nCalcStartPos, LPCTSTR pData, DWORD nLen)
{
	LineWrapper::WrapResult wr;

	DWORD nSepPoint;
	BOOL bBreak = FALSE;
	DWORD nPos = nCalcStartPos;

	while(!bBreak) {
		wr = Wrap(nPos, pData, nLen, &nSepPoint);

		switch(wr) {
		case LineWrapper::WRAPRESULT_CONT:
			if (!pLines->Add(&nSepPoint)) return FALSE;
			break;

		case LineWrapper::WRAPRESULT_FINISH:
			if (!pLines->Add(&nLen)) return FALSE;
			bBreak = TRUE;
			break;

		case LineWrapper::WRAPRESULT_FAIL:
			return FALSE;
		}

		nPos = nSepPoint;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// SimpleLineWrapper implimentation
/////////////////////////////////////////////////////////////////////////////

SimpleLineWrapper::SimpleLineWrapper()
{
}

SimpleLineWrapper::~SimpleLineWrapper()
{
}

LineWrapper::WrapResult SimpleLineWrapper::Wrap(DWORD nCurrentPos, LPCTSTR pBase, DWORD nLineLimit, LPDWORD pSepPos)
{
	LPCTSTR pStart = pBase + nCurrentPos;
	LPCTSTR pLimit = pBase + nLineLimit;

	*pSepPos = nLineLimit - nCurrentPos;
	return WRAPRESULT_FINISH;
}

/////////////////////////////////////////////////////////////////////////////
// FixedLetterWrapper implimentation
/////////////////////////////////////////////////////////////////////////////

FixedLetterWrapper::FixedLetterWrapper(DWORD n) : nViewWidth(n) {}
FixedLetterWrapper::~FixedLetterWrapper() {}

LineWrapper::WrapResult FixedLetterWrapper::Wrap(DWORD nCurrentPos, LPCTSTR pBase, DWORD nLineLimit, LPDWORD pSepPos)
{
	LPCTSTR pStart = pBase + nCurrentPos;
	DWORD nLen = nLineLimit - nCurrentPos;

	if (nLen > nViewWidth) {
		*pSepPos = nCurrentPos + nViewWidth;		
		return WRAPRESULT_CONT;
	} else {
		*pSepPos = nLineLimit;
		return WRAPRESULT_FINISH;
	}
}
