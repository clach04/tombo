#include <windows.h>
#include <tchar.h>
#include "Tombo.h"
#include "UniConv.h"
#include "TString.h"
#include "TomboURI.h"
#include "VarBuffer.h"
#include "URIScanner.h"
#include "Repository.h"

#define URILIST_INITSIZE 20
#define URILIST_DELTASIZE 10

/////////////////////////////////////////////////
//////////////////////////////////////////////////
// URIList implementation
//////////////////////////////////////////////////
//////////////////////////////////////////////////

URIList::URIList()
{
}

URIList::~URIList()
{
	for (DWORD i = 0; i < liList.NumItems(); i++) {
		URIListItem *pItem = liList.GetUnit(i);
		delete pItem->pURI;
		delete [] pItem->pTitle;
	}
}

BOOL URIList::Init()
{
	if (!liList.Init(URILIST_INITSIZE, URILIST_DELTASIZE)) return FALSE;
	return TRUE;
}

BOOL URIList::Add(const TomboURI *pURI, LPCTSTR pTitle)
{
	URIListItem item;
	if (pURI) {
		item.pURI = new TomboURI(*pURI);
		if (item.pURI == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }
	}

	if (pTitle) {
		item.pTitle = StringDup(pTitle); 
		if (item.pTitle == NULL) return FALSE;
	}

	if (!liList.Add(&item)) return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////
//////////////////////////////////////////////////
// URIScanner implementation
//////////////////////////////////////////////////
//////////////////////////////////////////////////

//////////////////////////////////////////////////
// ctor & Initialize
//////////////////////////////////////////////////

URIScanner::URIScanner() : pBaseURI(NULL), pCurrentURI(NULL), pTop(NULL), pBaseTitle(NULL)
{
}

BOOL URIScanner::Init(IEnumRepository *pRepo, const TomboURI *pURI, BOOL bSKE)
{
	// initialize value
	pRepository = pRepo;
	bSkipEncrypt = bSKE;
	iDirection = 1;

	// check URI
	URIOption opt(NOTE_OPTIONMASK_VALID);
	if (!pRepository->GetOption(pURI, &opt)) return FALSE;

	if (!opt.bFolder) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	// set URI
	pBaseURI = new TomboURI(*pURI);

	return TRUE;
}

//////////////////////////////////////////////////
// dtor
//////////////////////////////////////////////////

URIScanner::~URIScanner()
{
	ClearStack();
	delete pBaseURI;
	delete pBaseTitle;
}

//////////////////////////////////////////////////
// treat stack frame
//////////////////////////////////////////////////

void URIScanner::ClearStack()
{
	while(pTop) {
		LeaveFrame();
	}
}

void URIScanner::LeaveFrame()
{
	StackFrame *pSF = pTop;
	pTop = pSF->pNext;

	delete pSF->pList;
	delete pSF;
}

BOOL URIScanner::PushFrame(const TomboURI *pURI)
{
	StackFrame *pSF = new StackFrame();
	if (pSF == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }

	BOOL bLoose;
	pSF->pList = pRepository->GetChild(pCurrentURI, bSkipEncrypt, FALSE, &bLoose);
	if (pSF->pList == NULL) {
		DWORD n = GetLastError();
		if (GetLastError() == ERROR_CANCELLED) {
			StopScan();
			delete pSF;
			return TRUE;
		}
		return FALSE;
	}
	if (iDirection == 1) {
		pSF->nPos = 0;
	} else {
		pSF->nPos = pSF->pList->GetSize() - 1;
	}

	pSF->pNext = pTop;
	pTop = pSF;

	return TRUE;
}

//////////////////////////////////////////////////
// cutomisable methods
//////////////////////////////////////////////////

void URIScanner::InitialScan() {}
void URIScanner::AfterScan() {}
void URIScanner::PreFolder() {}
void URIScanner::PostFolder() {}
void URIScanner::Node() {}

//////////////////////////////////////////////////
// Scan repository
//////////////////////////////////////////////////

BOOL URIScanner::FullScan()
{
	return Scan(NULL, FALSE);
}

//////////////////////////////////////////////////
// Scan repository
//////////////////////////////////////////////////

BOOL URIScanner::Scan(const TomboURI *pStartURI, BOOL bReverse)
{
	// Initialize 
	if (bReverse) {
		iDirection = -1;
	} else {
		iDirection = 1;
	}
	bStopScan = FALSE;
	pBaseTitle = new TString();
	if (!pRepository->GetHeadLine(pBaseURI, pBaseTitle)) return FALSE;

	// set marker and call initialize method
	pCurrentURI = pBaseURI;
	pTitle = pBaseTitle->Get();
	InitialScan();


	// create stack frame
	if (pStartURI == NULL || _tcscmp(pStartURI->GetFullURI(), pBaseURI->GetFullURI()) == 0) {
		if (!PushFrame(pCurrentURI)) return FALSE;
		if (bStopScan) {
			AfterScan();
			return TRUE;
		}
		PreFolder();
	} else {
		// if pStartURI is passed, traverse the tree to create stack frame.
		if (!MakeFrame(pStartURI)) {
			return FALSE;
		}
	}

	while(pTop) {
		// enum current frame
		while (pTop->nPos >= 0 && pTop->nPos < pTop->pList->GetSize()) {

			if (bStopScan) break;

			// update marker
			pCurrentURI = pTop->pList->GetURI(pTop->nPos);
			pTitle = pTop->pList->GetTitle(pTop->nPos);

			// check folder or not
			URIOption opt(NOTE_OPTIONMASK_VALID);
			if (!pRepository->GetOption(pCurrentURI, &opt)) return FALSE;

			if (opt.bFolder) {
				// create new frame
				if (!PushFrame(pCurrentURI)) return FALSE;
				if (bStopScan) break;
				PreFolder();
				continue;
			} else {
				Node();
			}
			pTop->nPos += iDirection;
		}

		// when enumeration finished, 
		LeaveFrame();
		if (pTop) {
			pCurrentURI = pTop->pList->GetURI(pTop->nPos);
			pTitle = pTop->pList->GetTitle(pTop->nPos);
		} else {
			pCurrentURI = pBaseURI;
			pTitle = pBaseTitle->Get();
		}
		PostFolder();
		if (pTop) {
			pTop->nPos += iDirection;
		}
	}

	// call finalize method
	AfterScan();

	return TRUE;
}

//////////////////////////////////////////////////
//
//////////////////////////////////////////////////

BOOL URIScanner::MakeFrame(const TomboURI *pStartURI)
{
	// check base uri and start uri
	TString repB, repS;
	if (!pBaseURI->GetRepositoryName(&repB) || !pStartURI->GetRepositoryName(&repS) ||
		_tcscmp(repB.Get(), repS.Get()) != 0) return FALSE;

	TomboURIItemIterator itrB(pBaseURI);
	TomboURIItemIterator itrS(pStartURI);
	if (!itrB.Init() || !itrS.Init()) return FALSE;
	itrB.First(); itrS.First();
	while(TRUE) {
		LPCTSTR pB = itrB.Current();
		LPCTSTR pS = itrS.Current();

		if (pB == NULL || pS == NULL) {
			if (pS == NULL && pB != NULL) return FALSE;
			break;
		}

		if (_tcscmp(pB, pS) != 0) return FALSE;

		itrB.Next();
		itrS.Next();
	}

	// OK, stack frames.

	const TomboURI *pURI = pCurrentURI;

	pTitle = pBaseTitle->Get();

	// stack first frame
	if (!PushFrame(pURI)) return FALSE;
	PreFolder();

	TString shl;

	LPCTSTR pElement = itrS.Current();
	LPCTSTR pHeadLine;

	do {
		// get each element
		if (itrS.IsLeaf()) {
			if (!pRepository->GetHeadLine(pStartURI, &shl)) return FALSE;
			pHeadLine = shl.Get();

		} else {
			pHeadLine = pElement;

		}

		while (pTop->nPos >= 0 && pTop->nPos < pTop->pList->GetSize()) {
			LPCTSTR pElem = pTop->pList->GetTitle(pTop->nPos);

			if (_tcscmp(pHeadLine, pElem) == 0) {
				// path is matched.

				pCurrentURI = pTop->pList->GetURI(pTop->nPos);
				pTitle = pElem;

				URIOption opt(NOTE_OPTIONMASK_VALID);
				if (!pRepository->GetOption(pCurrentURI, &opt)) return FALSE;
				if (opt.bFolder) {
					if (!PushFrame(pCurrentURI)) return FALSE;
					if (bStopScan) break;
					PreFolder();
					break;
				} else {
					break;
				}
			}

			pTop->nPos += iDirection;
		}

		itrS.Next();
	} while((pElement = itrS.Current()) != NULL);


	return TRUE;
}
