#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include "Tombo.h"
#include "TString.h"
#include "MemoManager.h"
#include "MemoDetailsView.h"
#include "MemoSelectView.h"
#include "VarBuffer.h"
#include "MainFrame.h"
#include "UniConv.h"
#include "Property.h"
#include "TreeViewItem.h"
#include "SearchEngine.h"
#include "Message.h"
#include "TomboURI.h"
#include "AutoPtr.h"

#include "Repository.h"

/////////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////////

MemoManager::MemoManager() : pSearchEngineA(NULL), bMSSearchFlg(FALSE), bMDSearchFlg(FALSE)
{
}

MemoManager::~MemoManager()
{
	if(pSearchEngineA) {
		delete pSearchEngineA;
	}
}

/////////////////////////////////////////////
// initialize
/////////////////////////////////////////////

BOOL MemoManager::Init(MainFrame *mf, MemoDetailsView *md, MemoSelectView *ms) 
{
	pMainFrame = mf;
	pMemoDetailsView = md;
	pMemoSelectView = ms;
	return TRUE; 
}

/////////////////////////////////////////////
// get current selected path
/////////////////////////////////////////////

BOOL MemoManager::GetCurrentSelectedPath(TString *pPath)
{
	TString sURIstr;
	TomboURI sURI;

	if (pMemoDetailsView->GetCurrentURI()) {
		sURI = (*pMemoDetailsView->GetCurrentURI());
	} else {
		if (pMemoSelectView->GetCurrentSelectedURI()) {
			sURI = (*pMemoSelectView->GetCurrentSelectedURI());
		} else {
			return FALSE;
		}
	}

	if (sURI.IsLeaf()) {
		TomboURI sParent;
		if (!sURI.GetParent(&sParent)) return FALSE;
		if (!sParent.GetFilePath(pPath)) return FALSE;
	} else {
		if (!sURI.GetFilePath(pPath)) return FALSE;
	}
	ChopFileSeparator(pPath->Get());

	return TRUE;
}

////////////////////////////////////////////////////////
// Save notes
////////////////////////////////////////////////////////
// Notes is not save if not modified or read only.
// Otherwise save notes after confirm.

BOOL MemoManager::SaveIfModify(LPDWORD pYNC, BOOL bDupMode)
{
	if (pYNC) {
		*pYNC = IDOK;
	}

	// skip saving if not modified or read only
	if (!pMemoDetailsView->IsModify() || pMemoDetailsView->IsReadOnly()) {
		pMemoDetailsView->StoreCursorPos();
		return TRUE;
	}

	if (pYNC) {
		if (!g_Property.GetDisableSaveDlg()) {
			*pYNC = pMainFrame->MessageBox(MSG_MEMO_EDITED, MSG_CONFIRM_SAVE, MB_ICONQUESTION | MB_YESNOCANCEL | MB_APPLMODAL);
			if (*pYNC == IDNO || *pYNC == IDCANCEL) return TRUE;
		} else {
			*pYNC = IDOK;
		}
	}

	////////////////////////////////////////////

	// get memo data
	LPTSTR pText = pMemoDetailsView->GetMemo();
	SecureBufferAutoPointerT ap(pText);
	if (pText == NULL) return FALSE;

	////////////////////////////////////////////

	if (bDupMode) {
		if (!AllocNewMemo(pText, TRUE)) return FALSE;
		// duplicate mode notes are treated as update because pCurrentURI has set.
	} else {
		// Create node if the note is new
		if (pMemoDetailsView->GetCurrentURI() == NULL) {
			if (!AllocNewMemo(pText, FALSE)) return FALSE;
			// change status because the note is not new note at this point.
			pMainFrame->SetNewMemoStatus(FALSE);
		}
	}

	///////////////////////////////////////
	// save notes and update treeview

	TomboURI sCurrentURI(*(pMemoDetailsView->GetCurrentURI()));	// to preserve it because it changed by method Save.

	TomboURI sNewURI;
	TString sNewHeadLine;

	// save note contents
	if (!pMemoDetailsView->Save(&sCurrentURI, &sNewURI, &sNewHeadLine, pText)) return FALSE;

	// UpdateHeadLine causes TVN_SELCHANGING and call SaveIfModify.
	// So if not ResetModify is called, infinite calling causes GPF.
	// update treeview headline string
	pMemoSelectView->UpdateHeadLine(sCurrentURI.GetFullURI(), &sNewURI, sNewHeadLine.Get());

	// update window title
	pMainFrame->SetWindowTitle(&sNewURI);

	return TRUE;
}

////////////////////////////////////////////////////////
// allocate new memo
////////////////////////////////////////////////////////

BOOL MemoManager::AllocNewMemo(LPCTSTR pText, BOOL bCopy)
{
	const TomboURI *pTemplateURI = NULL;
	if (bCopy) {
		pTemplateURI = pMemoDetailsView->GetCurrentURI();
	}

	// get note path
	TomboURI sAttachFolder;
	TomboURI sSelected;

	const TomboURI *pCurSelectedURI = pMemoSelectView->GetCurrentSelectedURI();
	if (pCurSelectedURI == NULL) return FALSE;
	sSelected = *pCurSelectedURI;

	if (!sSelected.GetAttachFolder(&sAttachFolder)) return FALSE;

	// allocate new instance and associate to tree view
	TString sHeadLine;
	TomboURI sNewURI;

	// get URI
	if (!g_Repository.RequestAllocateURI(&sAttachFolder, pText, &sHeadLine, &sNewURI, pTemplateURI)) return FALSE;

	// Insert new node to select view
	HTREEITEM hParent;
	hParent = pMemoSelectView->ShowItemByURI(&sAttachFolder, FALSE);
	if (hParent == NULL) return FALSE;
	HTREEITEM hNewItem = pMemoSelectView->InsertFile(hParent, &sNewURI, sHeadLine.Get(), FALSE, FALSE);

	pMemoDetailsView->SetCurrentNote(&sNewURI);

	return TRUE;
}

////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////

void MemoManager::InactiveDetailsView()
{
	pMainFrame->LeaveDetailsView(TRUE);
}

////////////////////////////////////////////////////////
// 
////////////////////////////////////////////////////////

void MemoManager::SetSearchEngine(SearchEngineA *p)
{
	if (pSearchEngineA) {
		delete pSearchEngineA;
	}
	pSearchEngineA = p;
}

void MemoManager::ChangeURINotify(const TomboURI *pNewURI)
{
	pMemoDetailsView->SetCurrentNote(pNewURI);
}
