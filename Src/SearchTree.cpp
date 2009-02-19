#include <windows.h>
#include <tchar.h>
#include "Tombo.h"
#include "resource.h"
#include "UniConv.h"
#include "SearchEngine.h"
#include "SearchTree.h"
#include "TString.h"
#include "TomboURI.h"
#include "DialogTemplate.h"
#include "Message.h"
#include "Property.h"
#include "Repository.h"
#include "VarBuffer.h"
#include "URIScanner.h"

////////////////////////////////
////////////////////////////////

class SearchTreeScanner : public URIScanner {
public:
	BOOL bFound;
	TomboURI *pMatchedURI;

	SearchEngineA *pRegex;
	BOOL bSkipOne;

	BOOL Init(SearchEngineA *p, const TomboURI *pBase, BOOL bSkipOne, BOOL bSkipEncrypt);
	SearchTreeScanner();

	void Node();

};

SearchTreeScanner::SearchTreeScanner() : bFound(FALSE), pMatchedURI(NULL)
{
}

BOOL SearchTreeScanner::Init(SearchEngineA *p, const TomboURI *pBase, BOOL bSOne, BOOL bSEncrypt)
{
	pRegex = p;
	bSkipOne = bSOne;
	return URIScanner::Init(&g_Repository, pBase, bSEncrypt);
}

void SearchTreeScanner::Node()
{
	if (bSkipOne) {
		bSkipOne = FALSE;
		return;
	}

	SearchResult result = pRegex->SearchFromURI(CurrentURI());
	switch(result) {
	case SR_FOUND:
		bFound = TRUE;
		pMatchedURI = new TomboURI(*CurrentURI());
		StopScan();
		break;
	case SR_NOTFOUND:
		break;
	case SR_FAILED:
		StopScan();
		break;
	}
}

////////////////////////////////
// ctor & dtor
////////////////////////////////

BOOL SearchTree::Init(SearchEngineA *p, const TomboURI *pURI,  BOOL bDForward, BOOL bSOne, BOOL bSkipEncrypt)
{
	pStartURI = new TomboURI(*pURI);
	if (pStartURI == NULL) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); return FALSE; }
	pRegex = p;
	bSearchDirectionForward = bDForward;
	bSkipOne = bSOne;
	bSearchEncryptedMemo = !bSkipEncrypt;
	return TRUE;
}


SearchTree::~SearchTree()
{
	delete pScanner;
	delete pStartURI;
	delete pMatchedURI;
}

////////////////////////////////
// Dialog procedure
////////////////////////////////

static LRESULT CALLBACK SearchTreeDlgProc(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	SearchTree *pDlg;
	if (nMessage == WM_INITDIALOG) {
		SetWindowLong(hDlg, DWL_USER, lParam);
		pDlg = (SearchTree*)lParam;

		pDlg->InitDialog(hDlg);
		return TRUE;
	}

	pDlg = (SearchTree*)GetWindowLong(hDlg, DWL_USER);
	if (pDlg == NULL) return FALSE;

	switch(nMessage) {
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			pDlg->OnClose(hDlg, LOWORD(wParam));
			return TRUE;
		} else if (LOWORD(wParam) == IDCANCEL) {
			pDlg->CancelRequest();
			pDlg->OnClose(hDlg, LOWORD(wParam));
			return TRUE;
		}
	}
	return FALSE;
}

////////////////////////////////
// Thread function
////////////////////////////////

extern "C" static DWORD WINAPI SearchThreadFunc(LPVOID p)
{
	SearchTree *pSt = (SearchTree*)p;

	// Do search work
	pSt->SetResult(pSt->Search());

	PostMessage(pSt->GetWnd(), WM_COMMAND, MAKEWPARAM(IDOK, 0), NULL);
	return 0;
}

/////////////////////////////////////////
// Popup dialog and start searching
/////////////////////////////////////////

void SearchTree::Popup(HINSTANCE hInst, HWND hParent)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SEARCHING), hParent, (DLGPROC)SearchTreeDlgProc, (LONG)this);
}

/////////////////////////////////////////
// Initialize
/////////////////////////////////////////

static DlgMsgRes aDlgMsgRes[] = {
	{ IDCANCEL, MSG_ID_DLG_CMN_CANCEL}, 
};

void SearchTree::InitDialog(HWND hDlg)
{
	OverrideDlgMsg(hDlg, -1, aDlgMsgRes, sizeof(aDlgMsgRes)/sizeof(DlgMsgRes));

	DWORD nThreadId;

	hDlgWnd = hDlg;
	srResult = SR_NOTFOUND;
	hSearchThread = CreateThread(NULL, 0, SearchThreadFunc, (LPVOID)this, 0, &nThreadId);
}

/////////////////////////////////////////
// Close dialog
/////////////////////////////////////////

void SearchTree::OnClose(HWND hDlg, WORD nId)
{
	if (hSearchThread) {
		WaitForSingleObject(hSearchThread, INFINITE);
		CloseHandle(hSearchThread);
		hSearchThread = NULL;
	}
	EndDialog(hDlg, nId);
}

/////////////////////////////////////////
// request cancel
/////////////////////////////////////////

void SearchTree::CancelRequest()
{
	HWND hWnd = GetDlgItem(hDlgWnd, IDC_SEARCHMSG);
	SetWindowText(hWnd, MSG_SEARCH_CANCELING);
	if (pScanner) {
		pScanner->StopScan();
	}
}

/////////////////////////////////////////
// get current URI
/////////////////////////////////////////

const TomboURI* SearchTree::CurrentURI()
{
	if (pScanner) {
		return pScanner->CurrentURI();
	} else {
		return NULL;
	}
}

/////////////////////////////////////////
// search main
/////////////////////////////////////////

SearchResult SearchTree::Search()
{
	TomboURI sRoot;
	sRoot.Init(TEXT("tombo://default/"));

	pScanner = new SearchTreeScanner();
	if (!pScanner->Init(pRegex, &sRoot, bSkipOne, !bSearchEncryptedMemo)) return SR_FAILED;
	if (!pScanner->Scan(pStartURI, !bSearchDirectionForward)) {
		return SR_FAILED;
	}

	if (pScanner->bFound) {
		pMatchedURI = new TomboURI(*(pScanner->pMatchedURI));
		return SR_FOUND;
	}
	if (pScanner->IsStopScan()) {
		return SR_CANCELED;
	}
	return SR_NOTFOUND;
}