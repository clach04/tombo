#include <windows.h>
#include "VarBuffer.h"
#include "LineWrapper.h"
#include "LineWrapperImpl.h"
#include "Region.h"
#include "YAEdit.h"

/////////////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////////////

LineWrapper::WrapResult FixedPixelLineWrapper::Wrap(DWORD nCurrentPos, LPCTSTR pBase, DWORD nLineLimit, LPDWORD pSepPos)
{
	LPCTSTR pStart = pBase + nCurrentPos;
	LPCTSTR pLimit = pBase + nLineLimit;
	LPCTSTR p = pStart;
	DWORD nWidth;
	DWORD nCurrentWidth = 0;
	DWORD k;
	while(p < pLimit) {
		k = 1;
#if defined(PLATFORM_WIN32)
		if (IsDBCSLeadByte(*p)) {
			k++;
		}
#endif
		nWidth = pView->GetLineWidth(nCurrentWidth, p, k);
		nCurrentWidth += nWidth;
		if (nCurrentWidth > nViewWidth) break;
		p += k;
	}

	if (p == pLimit) {
		return WRAPRESULT_FINISH;
	} else {
		*pSepPos = p - pBase;
		return WRAPRESULT_CONT;
	}
}
