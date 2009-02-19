#ifndef DETAILSVIEWDLG_H
#define DETAILSVIEWDLG_H

class MemoDetailsView;
class MemoManager;

class DetailsViewDlg : public Tombo_Lib::DialogTemplate
{
	HWND hDialog;
	HWND hMenubar;

	MemoManager *pMgr;
	MemoDetailsView *pView;

	LPTSTR pText;
public:
	DetailsViewDlg();
	~DetailsViewDlg();

	void InitDialog(HWND hDlg);
	BOOL OnOK();

	DWORD Popup(HINSTANCE hInst, HWND hParent, MemoManager *pMgr, LPTSTR pText);

	void SetModify();

	LPCTSTR GetText() { return pText; }

};

#endif
