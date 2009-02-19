#include <windows.h>
#include <tchar.h>
#include "Tombo.h"
#include "VarBuffer.h"
#include "TString.h"
#include "TomboURI.h"
#include "UniConv.h"
#include "BookMark.h"

#include "Repository.h"

////////////////////////////////////
// ctor & dtor
////////////////////////////////////

BookMark::BookMark()
{
}

BookMark::~BookMark()
{
	Clear();
}

BOOL BookMark::Init(WORD nBaseID)
{
	nBookMarkIDBase = nMaxBookMarkID = nBaseID;
	return vBookMark.Init(10, 10);
}

/////////////////////////////////////////
// Clear current bookmarks
/////////////////////////////////////////

void BookMark::Clear()
{
	DWORD n = vBookMark.NumItems();
	for (DWORD i = 0; i < n; i++) {
		BookMarkItem *p = vBookMark.GetUnit(i);

		delete [] p->pPath;
		delete [] p->pName;
	}
	vBookMark.Clear(TRUE);
	nMaxBookMarkID = nBookMarkIDBase;
}


/////////////////////////////////////////
// check the ID is assigned to bookmark?
/////////////////////////////////////////

BOOL BookMark::IsBookMarkID(WORD nID)
{
	return nBookMarkIDBase <= nID  && nID < nMaxBookMarkID;
}

/////////////////////////////////////////
// Allocate book mark
/////////////////////////////////////////

const BookMarkItem *BookMark::Assign(LPCTSTR pPath)
{
	// get headline 
	TString sTitle;

	TomboURI tURI;
	if (!tURI.Init(pPath)) return NULL;
	if (!g_Repository.GetHeadLine(&tURI, &sTitle)) return NULL;

	BookMarkItem bi;

	bi.nID = nMaxBookMarkID;
	bi.pName = StringDup(sTitle.Get());
	bi.pPath = StringDup(pPath);

	if (!vBookMark.Add(&bi)) return NULL;

	nMaxBookMarkID++;
	return vBookMark.GetUnit(vBookMark.NumItems() - 1);
}

/////////////////////////////////////////
// Find bookmark
/////////////////////////////////////////

const BookMarkItem *BookMark::Find(WORD nID)
{
	BookMarkItem *p = NULL;
	DWORD n = vBookMark.NumItems();
	for (DWORD i = 0; i < n; i++) {
		if (vBookMark.GetUnit(i)->nID == nID) {
			p = vBookMark.GetUnit(i);
			break;
		}
	}
	return p;
}

/////////////////////////////////////////
// enum bookmarks
/////////////////////////////////////////

DWORD BookMark::NumItems()
{
	return vBookMark.NumItems();
}

const BookMarkItem *BookMark::GetUnit(DWORD n)
{
	return vBookMark.GetUnit(n);
}


/////////////////////////////////////////
// import
/////////////////////////////////////////
// for read from registry

BOOL BookMark::ImportFromMultiSZ(LPCTSTR pBookMarks)
{
	// release current bookmarks
	Clear();

	// load bookmarks
	LPCTSTR p = pBookMarks;
	while(*p) {
		if (*p == TEXT('\\') && *(p+1) == TEXT('\0')) {
			if (!Assign(TEXT(""))) return FALSE;
		} else {
			if (!Assign(p)) return FALSE;
		}
		p += _tcslen(p) + 1;
	}

	return TRUE;
}

/////////////////////////////////////////
// export
/////////////////////////////////////////
// for write to registry

LPTSTR BookMark::ExportToMultiSZ()
{
	DWORD nSize = 0;

	DWORD i;

	BookMarkItem *p = NULL;
	DWORD n = vBookMark.NumItems();
	for (i = 0; i < n; i++) {
		p = vBookMark.GetUnit(i);

		if (_tcslen(p->pPath) == 0) {
			// root node
			nSize++;
		}

		nSize += _tcslen(p->pPath) + 1;
	}
	nSize++;

	LPTSTR pMarks = new TCHAR[nSize];
	if (pMarks == NULL) return NULL;

	LPTSTR pCur = pMarks;
	for (i = 0; i < n; i++) {
		p = vBookMark.GetUnit(i);

		if (_tcslen(p->pPath) == 0) {
			*pCur++ = TEXT('\\');
			*pCur++ = TEXT('\0');
		} else {
			_tcscpy(pCur, p->pPath);
			pCur += _tcslen(p->pPath) + 1;
		}
	}
	*pCur = TEXT('\0');

	return pMarks;
}
