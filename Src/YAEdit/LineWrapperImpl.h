#ifndef LINEWRAPPERIMPL_H
#define LINEWRAPPERIMPL_H

/////////////////////////////////////////////////////////////////////////////
// wrapped by window width
/////////////////////////////////////////////////////////////////////////////

class YAEditImpl;

class FixedPixelLineWrapper : public LineWrapper {
protected:
	YAEditImpl *pView;
public:

	FixedPixelLineWrapper() {}
	virtual ~FixedPixelLineWrapper() {}

	BOOL Init(YAEditImpl *p) { pView = p; return TRUE; }

	WrapResult Wrap(DWORD nCurrentPos, LPCTSTR pBase, DWORD nLineLimit, LPDWORD pSepPos);

};


#endif