#include <windows.h>
#include <tchar.h>
#include "Tombo.h"
#include "resource.h"
#include "Message.h"
#include "GrepDialog.h"
#include "Property.h"
#include "PropertyPage.h"
#include "DialogTemplate.h"

#define NUM_GREP_PROP_PAGES 1

/////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////

BOOL GrepDialog::Init(LPCTSTR pPath)
{
	if (!sPath.Set(pPath)) return FALSE;
	return TRUE;
}

/////////////////////////////////////////
// Main Tab
/////////////////////////////////////////

class GrepMainTab : public PropertyTab {
	GrepDialog *pDialog;
public:
	GrepMainTab(GrepDialog *pDlg) : pDialog(pDlg), PropertyTab(IDD_GREPTAB_MAIN, MSG_GREPTAB_MAIN, (DLGPROC)DefaultPageProc) {}
	~GrepMainTab() {}

	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
};

static DlgMsgRes aGrepMain[] = {
	{ IDC_GREPTAB_MAIN_POSLABEL,      MSG_ID_DLG_GREPTAB_MAIN_POSLABEL },
	{ IDC_GREPTAB_MAIN_STRLABEL,      MSG_ID_DLG_GREPTAB_MAIN_STRLABEL },
	{ IDC_GREPTAB_MAIN_CASESENSITIVE, MSG_ID_DLG_FILTERDEF_ADD_REGEX_CASESENSITIVE },
	{ IDC_GREPTAB_MAIN_ENCRYPTNOTE,   MSG_ID_DLG_FILTERDEF_ADD_REGEX_INCLUDECRYPTED },
	{ IDC_GREPTAB_MAIN_FILENAME,      MSG_ID_DLG_FILTERDEF_ADD_REGEX_FORFILENAME },
};

void GrepMainTab::Init(HWND hDlg)
{
	OverrideDlgMsg(hDlg, -1, aGrepMain, sizeof(aGrepMain)/sizeof(DlgMsgRes));

	HWND hCombo = GetDlgItem(hDlg, IDC_GREPTAB_MAIN_SEARCHSTRING);
	HWND hPath = GetDlgItem(hDlg, IDC_GREPTAB_MAIN_STARTDIR);

	TString sDispPath;
	if (!sDispPath.Join(TEXT("folder:\\"), pDialog->GetPath())) return;
	SetWindowText(hPath, sDispPath.Get());
	LPCTSTR pHist = g_Property.GetSearchHist();
	SetHistoryToComboBox(hCombo, pHist);
}

BOOL GrepMainTab::Apply(HWND hDlg)
{
	HWND hCaseSensitive = GetDlgItem(hDlg, IDC_GREPTAB_MAIN_CASESENSITIVE);
	HWND hCheckCrypted = GetDlgItem(hDlg, IDC_GREPTAB_MAIN_ENCRYPTNOTE);
	HWND hCheckFileName = GetDlgItem(hDlg, IDC_GREPTAB_MAIN_FILENAME);
	HWND hMatchString = GetDlgItem(hDlg, IDC_GREPTAB_MAIN_SEARCHSTRING);

	TString *pMatchString = pDialog->GetMatchStringPtr();
	DWORD nLen = GetWindowTextLength(hMatchString);
	if (!pMatchString->Alloc(nLen + 1)) return FALSE;
	GetWindowText(hMatchString, pMatchString->Get(), nLen + 1);
	g_Property.SetSearchHist(GetHistoryFromComboBox(hMatchString, pMatchString->Get(), NUM_SEARCH_HISTORY));

	pDialog->SetCaseSensitive(SendMessage(hCaseSensitive, BM_GETCHECK, 0, 0) == BST_CHECKED);
	pDialog->SetCheckCryptedMemo(SendMessage(hCheckCrypted, BM_GETCHECK, 0, 0) == BST_CHECKED);
	pDialog->SetCheckFileName(SendMessage(hCheckFileName, BM_GETCHECK, 0, 0) == BST_CHECKED);

	return TRUE;
}

BOOL GrepMainTab::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

/////////////////////////////////////////
// Persistent Tab
/////////////////////////////////////////

class GrepPersistTab : public PropertyTab {
public:
	GrepPersistTab() : PropertyTab(IDD_GREPTAB_PERSIST, MSG_GREPTAB_PERSIST, (DLGPROC)DefaultPageProc) {}
	~GrepPersistTab() {}

	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
};

//static DlgMsgRes aPersist[] = {
//	{ IDC_PROPTAB_PERSIST_SELECTPERSIST, MSG_ID_DLG_GREPTAB_PERSIST_KEEP },
//	{ IDC_GREPTAB_PERSIST_LABEL, MSG_ID_DLG_GREPTAB_PERSIST_LABEL },
//};

void GrepPersistTab::Init(HWND hDlg)
{
//	OverrideDlgMsg(hDlg, -1, aPersist, sizeof(aPersist)/sizeof(DlgMsgRes));
}

BOOL GrepPersistTab::Apply(HWND hDlg)
{
	return TRUE;
}

BOOL GrepPersistTab::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

/////////////////////////////////////////
// Dialog popup
/////////////////////////////////////////

DWORD GrepDialog::Popup(HINSTANCE hInst, HWND hParent)
{
	PropertyTab *pages[NUM_GREP_PROP_PAGES];
	GrepMainTab pgMain(this);
	GrepPersistTab pgPersist;

	pages[0] = &pgMain;
//	pages[1] = &pgPersist;

	PropertyPage pp;

	return pp.Popup(hInst, hParent, pages, NUM_GREP_PROP_PAGES, MSG_GREPTAB_MAIN_TTL, MAKEINTRESOURCE(IDI_TOMBO));
}

