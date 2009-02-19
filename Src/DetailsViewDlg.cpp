#include <windows.h>
#include <tchar.h>

#include <aygshell.h>

#include "Tombo.h"
#include "resource.h"
#include "MemoManager.h"
#include "DialogTemplate.h"
#include "DetailsViewDlg.h"
#include "MemoDetailsView.h"
#include "Property.h"
#include "MemoNote.h"
#include "Message.h"

#include "Tombo.h"
#define NUM_TOOLBAR_BMP 12

///////////////////////////////////////////////////
// view callback
///////////////////////////////////////////////////

class DetailsViewDlgCallback : public MemoDetailsViewCallback {
	MemoManager *pMgr;
	DetailsViewDlg *pDlg;
public:
	DetailsViewDlgCallback(MemoManager *pm, DetailsViewDlg *p) : pMgr(pm), pDlg(p) {}
	void GetFocusCallback(MemoDetailsView *pView);
	void SetModifyStatusCallback(MemoDetailsView *pView);
	void SetReadOnlyStatusCallback(MemoDetailsView *pView);

	void SetSearchFlg(BOOL bFlg);

	SearchEngineA *GetSearchEngine(MemoDetailsView *pView);
	void GetCurrentSelectedPath(MemoDetailsView *pView, TString *pPath);
};

void DetailsViewDlgCallback::GetFocusCallback(MemoDetailsView *pView) {}
void DetailsViewDlgCallback::SetModifyStatusCallback(MemoDetailsView *pView)
{
	pDlg->SetModify();
}

void DetailsViewDlgCallback::SetReadOnlyStatusCallback(MemoDetailsView *pView) {}
void DetailsViewDlgCallback::SetSearchFlg(BOOL bFlg) {}

SearchEngineA *DetailsViewDlgCallback::GetSearchEngine(MemoDetailsView *pView)
{
	return pMgr->GetSearchEngine();
}
void DetailsViewDlgCallback::GetCurrentSelectedPath(MemoDetailsView *pView, TString *pPath)
{
	pMgr->GetCurrentSelectedPath(pPath);
}

///////////////////////////////////////////////////
// 
///////////////////////////////////////////////////

DetailsViewDlg::DetailsViewDlg() : pText(NULL)
{
}

DetailsViewDlg::~DetailsViewDlg()
{
	delete [] pText;
}

void DetailsViewDlg::InitDialog(HWND hDlg)
{
	hDialog = hDlg;

	// set PocketPC style dialog
	SHINITDLGINFO shidi;
	shidi.dwMask = SHIDIM_FLAGS;
	shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
	shidi.hDlg = hDlg;
	SHInitDialog(&shidi);

	// create menu bar
	SHMENUBARINFO mbi;
	memset(&mbi, 0, sizeof(SHMENUBARINFO));
	mbi.cbSize = sizeof(SHMENUBARINFO);
	mbi.hwndParent = hDlg;
	mbi.nToolBarId = IDM_DETAILSDLG_MENU;
	mbi.hInstRes =g_hInstance;
	mbi.nBmpId = IDB_TOOLBAR;
	mbi.cBmpImages = NUM_TOOLBAR_BMP;
	SHCreateMenuBar(&mbi);

	hMenubar = mbi.hwndMB;

	RECT r;
	GetClientRect(hDlg, &r);

	DetailsViewDlgCallback *pCB = new DetailsViewDlgCallback(pMgr, this);
	SimpleEditor *p = new SimpleEditor(pCB);
	p->Init(pMgr, 2000, 2001);
	p->Create(TEXT("MemoDetailsDlg"), r, hDlg, g_hInstance, g_Property.DetailsViewFont());
	pView = p;

	pView->Show(SW_SHOW);
	pView->SetMemo(pText, 0, FALSE);
	MemoNote::WipeOutAndDelete(pText);
	pText = NULL;

	pView->SetModifyStatus();
	pView->SetFocus();
}

BOOL DetailsViewDlg::OnOK()
{
	if (pView->IsModify()) {
		int iResult = MessageBox(hDialog, MSG_MEMO_EDITED, MSG_CONFIRM_SAVE, MB_ICONQUESTION | MB_YESNOCANCEL | MB_APPLMODAL);
		if (iResult == IDCANCEL) return FALSE;
		if (iResult == IDYES) {
			delete [] pText;
			pText = NULL;
		}
		// override popup results
		nResult = iResult;
	}
	return TRUE;
}

DWORD DetailsViewDlg::Popup(HINSTANCE hInst, HWND hParent, MemoManager *p, LPTSTR pt)
{
	pText = pt;
	pMgr = p;
	return DialogTemplate::Popup(hInst, IDD_DETAILSVIEW, hParent);	
}

void DetailsViewDlg::SetModify()
{
	BOOL bEnable = pView->IsModify();
	SendMessage(hMenubar, TB_ENABLEBUTTON, IDM_SAVE, MAKELONG(bEnable, 0));
}