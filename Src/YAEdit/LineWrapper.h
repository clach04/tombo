#ifndef LINEWRAPPER_H
#define LINEWRAPPER_H

////////////////////////////////////////////////////////////////////////////
// Wrap line text
/////////////////////////////////////////////////////////////////////////////

class LineWrapper {
protected:
	DWORD nViewWidth;
public:
	enum WrapResult {
		WRAPRESULT_FAIL,
		WRAPRESULT_CONT,
		WRAPRESULT_FINISH
	};

	LineWrapper() {}
	virtual ~LineWrapper() {}

	/////////////////////////////////////////////////////////////////
	// wrapping implimentaiton
	// IN:
	//		nCurrentPos
	//		pBase
	//		nLineLimit
	// OUT:
	//		pSepPos	
	// RESULT:
	//		WRAPRESULT_FINISH	- returns when wrapping has finished.
	//		WRAPRESULT_CONT		- returns when remain data exists.

	virtual WrapResult Wrap(DWORD nCurrentPos, LPCTSTR pBase, DWORD nLineLimit, LPDWORD pSepPos) = 0;

	/////////////////////////////////////////////////////////////////
	// get wrapping info to line.
	// Seek lines and calls Wrap(), summarise results to pLines.
	//
	// IN:
	//		pData	- physical line data
	//		nLen	- data length
	//		nCalcStartPos - wrap start position
	// OUT:
	//		pLines	- array of wrapping points

	BOOL GetNewMarkerList(TVector<DWORD> *pLines, DWORD nCalcStartPos, LPCTSTR pData, DWORD nLen);

	/////////////////////////////////////////////////////////////////
	// data accessors 

	// screen width
	void SetViewWidth(DWORD nWidth) { nViewWidth = nWidth; }
	DWORD GetViewWidth() { return nViewWidth; }

};

////////////////////////////////////////////////////////////////////////////
// Simple line wrapper
/////////////////////////////////////////////////////////////////////////////
// SimpleLineWrapper associates 1 physical line to 1 logical line(no wrapping).

class SimpleLineWrapper : public LineWrapper {
public:
	SimpleLineWrapper();
	virtual ~SimpleLineWrapper();

	WrapResult Wrap(DWORD nCurrentPos, LPCTSTR pBase, DWORD nLineLimit, LPDWORD pSepPos);
};

////////////////////////////////////////////////////////////////////////////
// wrap by specified letters 
/////////////////////////////////////////////////////////////////////////////

class FixedLetterWrapper : public LineWrapper {
	DWORD nViewWidth;
public:
	FixedLetterWrapper(DWORD n);
	~FixedLetterWrapper();

	WrapResult Wrap(DWORD nCurrentPos, LPCTSTR pBase, DWORD nLineLimit, LPDWORD pSepPos);
};
#endif
