#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include "YAEditor.h"

#include "resource.h"

#include "Tombo.h"
#include "UniConv.h"
#include "Property.h"
#include "Region.h"
#include "YAEdit.h"
#include "YAEditDoc.h"
#include "YAEditView.h"
#include "MemoManager.h"
#include "MainFrame.h"

/////////////////////////////////////////////////////////////////////////////
// TomboDoc callback
/////////////////////////////////////////////////////////////////////////////

class YAEDetailsViewCallback : public YAEditCallback {
	YAEditor *pEditor;
	MemoManager *pManager;
public:
	YAEDetailsViewCallback(YAEditor *pSelf, MemoManager *pmm) : pEditor(pSelf), pManager(pmm) {}

	void OnGetFocus();
	void ChangeModifyStatusNotify(BOOL bStatus);
	void ChangeReadOnlyStatusNotify(BOOL bStatus);

	void OnContextMenu(HWND hWnd, WORD x, WORD y);
};

void YAEDetailsViewCallback::OnGetFocus()
{
	pEditor->OnGetFocus();
}

void YAEDetailsViewCallback::ChangeModifyStatusNotify(BOOL bStatus)
{
	pEditor->ChangeModifyStatusNotify(bStatus);
}

void YAEDetailsViewCallback::ChangeReadOnlyStatusNotify(BOOL bStatus)
{
	pManager->GetMainFrame()->SetReadOnlyStatus(bStatus);
}


typedef void (YAEditor::*YAEditCommandFunc)();

struct YAEContextMenu {
	LPCTSTR pItemName;
	YAEditCommandFunc pFunc;
};

void YAEDetailsViewCallback::OnContextMenu(HWND hWnd, WORD x, WORD y)
{
	// MSG_xx needs initializ after initialized message resources. 
	static YAEContextMenu contextMenu[] = {
		{ MSG_MENUITEM_UNDO,			&YAEditor::CmdUndo },
		{ TEXT(""),						NULL },
		{ MSG_MENUITEM_MAIN_CUT,		&YAEditor::CmdCut },
		{ MSG_MENUITEM_MAIN_COPY,		&YAEditor::CmdCopy },
		{ MSG_MENUITEM_MAIN_PASTE,		&YAEditor::CmdPaste },
		{ MSG_MENUITEM_MAIN_DELETE,		&YAEditor::CmdBackSpace },
		{ TEXT(""),						NULL },
		{ MSG_MENUITEM_DETAILS_SELALL,	&YAEditor::CmdSelAll },
		{ TEXT(""),						NULL },
		{ MSG_TOOLTIPS_INSDATE1,		&YAEditor::InsertDate1 },
		{ MSG_TOOLTIPS_INSDATE2,		&YAEditor::InsertDate2 },
		{ NULL,							NULL },
	};

	// create context menu
	HMENU hMenu = CreatePopupMenu();
	DWORD nItems = 1;

	const YAEContextMenu *p = contextMenu;
	while (p->pItemName != NULL) {
		if (*(p->pItemName) == TEXT('\0')) {
			InsertMenu(hMenu, nItems - 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
		} else {
			InsertMenu(hMenu, nItems - 1, MF_BYPOSITION | MF_STRING, nItems, p->pItemName);
		}
		p++;
		nItems++;
	}

	DWORD id = TrackPopupMenuEx(hMenu, TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN, x, y, hWnd, NULL);
	if (id > 0 && id < nItems) {
		YAEditCommandFunc f = contextMenu[id - 1].pFunc;
		(pEditor->*f)();
	}
	DestroyMenu(hMenu);
}

/////////////////////////////////////////////////////////////////////////////
// YAEditor implimentation 
/////////////////////////////////////////////////////////////////////////////


YAEditor::YAEditor(MemoManager *pMgr) : MemoDetailsView(pMgr), pEdit(NULL), pYAECallback(NULL)
{
}

YAEditor::~YAEditor()
{
	delete pEdit;
	delete pYAECallback;
}

BOOL YAEditor::Create(LPCTSTR pName, RECT &r, HWND hParent, HINSTANCE hInst, HFONT hFont)
{
	pYAECallback = new YAEDetailsViewCallback(this, pManager);
	pEdit = YAEdit::GetInstance(pYAECallback);
	pEdit->Create(hInst, hParent, nID, r, g_Property.GetWrapText());
	pEdit->SetFont(hFont);
	return TRUE;
}

BOOL YAEditor::Init(DWORD n)
{
	nID = n;
	return TRUE;
}

void YAEditor::SetFocus()
{
	if (pEdit) pEdit->SetFocus();
}

void YAEditor::OnGetFocus()
{
	pManager->GetMainFrame()->NotifyDetailsViewFocused();
}


LPTSTR YAEditor::GetMemo()
{
	DWORD nLen;
	char *pData = pEdit->GetDoc()->GetDocumentData(&nLen);
#if defined(PLATFORM_WIN32)
	return pData;
#else
	LPTSTR pDataW = ConvSJIS2Unicode(pData);
	delete [] pData;
	return pDataW;
#endif
}

void YAEditor::MoveWindow(DWORD x, DWORD y, DWORD nWidth, DWORD nHeight)
{
	if (pEdit) pEdit->ResizeWindow(x, y, nWidth, nHeight);
}

BOOL YAEditor::SetMemo(LPCTSTR pMemoW, DWORD nPos, BOOL bReadOnly)
{
#if defined(PLATFORM_WIN32)
	const char *pMemo = pMemoW;
#else
	char *pMemo = ConvUnicode2SJIS(pMemoW);
#endif
	YAEditDoc *pDoc = pEdit->CreateDocument(pMemo, pYAECallback);
	if (pDoc == NULL) return FALSE;

	pDoc->SetReadOnly(bReadOnly);
	pYAECallback->ChangeReadOnlyStatusNotify(bReadOnly);

#if !defined(PLATFORM_WIN32)
	delete [] pMemo;
#endif

	YAEditDoc *pOldDoc = pEdit->SetDoc(pDoc);
	delete pOldDoc;

	if (g_Property.GetKeepCaret()) {
		pEdit->SetCaretPos(nPos);
	}

	return TRUE;
}

BOOL YAEditor::IsModify()
{
	return pEdit->GetDoc()->IsModify();
}

void YAEditor::ResetModify()
{
	pEdit->GetDoc()->SetModify(FALSE);
}

BOOL YAEditor::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(LOWORD(wParam)) {
	case IDM_CUT:
		pEdit->CmdCut();
		return TRUE;
	case IDM_COPY:
		pEdit->CmdCopy();
		return TRUE;
	case IDM_PASTE:
		pEdit->CmdPaste();
		return TRUE;
	case IDM_INSDATE1:
		InsertDate1();
		return TRUE;
	case IDM_INSDATE2:
		InsertDate2();
		return TRUE;
	case IDM_UNDO:
		pEdit->CmdUndo();
		return TRUE;
	}
	return FALSE;
}

void YAEditor::SetMDSearchFlg(BOOL bFlg)
{
	pManager->SetMDSearchFlg(bFlg);
}

void YAEditor::SetFont(HFONT hFont)
{
	pEdit->SetFont(hFont);
}

DWORD YAEditor::GetCursorPos()
{
	return pEdit->GetCaretPos();
}

void YAEditor::ChangeModifyStatusNotify(BOOL bStatus)
{
	pManager->GetMainFrame()->SetModifyStatus(bStatus);
}

BOOL YAEditor::ReplaceText(LPCTSTR p)
{
	pEdit->CmdReplaceString(p);
	return TRUE;
}

void YAEditor::SetSelectRegion(DWORD nStart, DWORD nEnd)
{
	pEdit->SetSelectRegion(nStart, nEnd);
}

void YAEditor::SetReadOnly(BOOL bReadOnly)
{
	pEdit->GetDoc()->SetReadOnly(bReadOnly);
}

BOOL YAEditor::IsReadOnly()
{
	return pEdit->GetDoc()->IsReadOnly();
}

BOOL YAEditor::SetFolding(BOOL bFold)
{
	pEdit->CmdToggleWrapMode(bFold);
	return TRUE;
}

void YAEditor::CmdUndo()
{
	pEdit->CmdUndo();
}

void YAEditor::CmdCut()
{
	pEdit->CmdCut();
}

void YAEditor::CmdCopy()
{
	pEdit->CmdCopy();
}

void YAEditor::CmdPaste()
{
	pEdit->CmdPaste();
}

void YAEditor::CmdBackSpace()
{
	pEdit->CmdBackSpace();
}

void YAEditor::CmdSelAll()
{
	pEdit->CmdSelAll();
}

BOOL YAEditor::Show(int nCmdShow)
{
	return pEdit->Show(nCmdShow);
}