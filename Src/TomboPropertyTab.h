#ifndef TOMBOPROPERTYTAB_H
#define TOMBOPROPERTYTAB_H

//////////////////////////////////////////
// TOMBO general property tab
//////////////////////////////////////////

class TomboPropertyTab : public PropertyTab {
protected:
	Property *pProperty;
public:
	TomboPropertyTab(Property *prop, DWORD id, DLGPROC proc, LPCTSTR pTitleName) : PropertyTab(id, pTitleName, proc), pProperty(prop) {}
};

//////////////////////////////////////////
// TomboRoot setting tab
//////////////////////////////////////////

class FolderTab : public TomboPropertyTab {
public:
	FolderTab(Property *p) : 
	  TomboPropertyTab(p, IDD_PROPTAB_FOLDER,(DLGPROC)DefaultPageProc, MSG_PROPTAB_FOLDER) {}
	~FolderTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
};

//////////////////////////////////////////
// Password timeout setting tab
//////////////////////////////////////////

class PassTimeoutTab : public TomboPropertyTab {
public:
	PassTimeoutTab(Property *p) : 
	  TomboPropertyTab(p, IDD_PROPTAB_PASS_TIMEOUT,(DLGPROC)DefaultPageProc, MSG_PROPTAB_PASS_TIMEOUT) {}
	~PassTimeoutTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
};

//////////////////////////////////////////
// Font setting tab
//////////////////////////////////////////

class FontTab : public TomboPropertyTab {
public:
	FontTab(Property *p) :
	  TomboPropertyTab(p, IDD_PROPTAB_FONT, (DLGPROC)DefaultPageProc, MSG_PROPTAB_FONT) {}
	~FontTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
};

//////////////////////////////////////////
// Date format
//////////////////////////////////////////

class DateFormatTab : public TomboPropertyTab {
public:
	DateFormatTab(Property *p) :
	  TomboPropertyTab(p, IDD_PROPTAB_INSDATE, (DLGPROC)DefaultPageProc, MSG_PROPTAB_DATE) {}
	~DateFormatTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
};

//////////////////////////////////////////
// Caret setting tab
//////////////////////////////////////////

class KeepCaretTab : public TomboPropertyTab {
public:
	KeepCaretTab(Property *p) :
	  TomboPropertyTab(p, IDD_PROPTAB_KEEPCARET, (DLGPROC)DefaultPageProc, MSG_PROPTAB_KEEPCARET) {}
	~KeepCaretTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
};

//////////////////////////////////////////
// Action buttons
//////////////////////////////////////////

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
class AppButtonTab : public TomboPropertyTab {
public:
	AppButtonTab(Property *p) :
	  TomboPropertyTab(p, IDD_PROPTAB_APPBUTTON, (DLGPROC)DefaultPageProc, MSG_PROPTAB_APPBUTTON) {}
	~AppButtonTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
};
#endif

//////////////////////////////////////////
// SIP tab
//////////////////////////////////////////

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)

class SipTab : public TomboPropertyTab {
public:
	SipTab(Property *p) :
	  TomboPropertyTab(p, IDD_PROPTAB_SIP, (DLGPROC)DefaultPageProc, MSG_PROPTAB_SIP) {}
  ~SipTab() {}

  void Init(HWND hDlg);
  BOOL Apply(HWND hDlg);
};
#endif

//////////////////////////////////////////
// Codepage tab
//////////////////////////////////////////

class CodepageTab : public TomboPropertyTab {
public:
	CodepageTab(Property *p) :
	  TomboPropertyTab(p, IDD_PROPTAB_CODEPAGE, (DLGPROC)DefaultPageProc, MSG_PROPTAB_CODEPAGE) {}
	~CodepageTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
};

//////////////////////////////////////////
// DefaultNote tab
//////////////////////////////////////////

class DefaultNoteTab : public TomboPropertyTab {
	LPCTSTR pCurrentPath;
public:
	DefaultNoteTab(Property *p, LPCTSTR pPath) :
	  TomboPropertyTab(p, IDD_PROPTAB_DEFNOTE, (DLGPROC)DefaultPageProc, MSG_PROPTAB_DEFNOTE), pCurrentPath(pPath) {}
	~DefaultNoteTab() {}
	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);

protected:
	void SetBlank(HWND hDlg);
	void SetCurrent(HWND hDlg);
};

//////////////////////////////////////////
// External application tab
//////////////////////////////////////////

#if !defined(PLATFORM_PSPC)
class ExtAppTab : public TomboPropertyTab {
	DWORD nUseAssoc;
	TCHAR aExtApp1[MAX_PATH];
	TCHAR aExtApp2[MAX_PATH];
protected:
	void Choose1(HWND hDlg);
	void Choose2(HWND hDlg);

public:
	ExtAppTab(Property *p) :
	  TomboPropertyTab(p, IDD_PROPTAB_EXTAPP, (DLGPROC)DefaultPageProc, MSG_PROPTAB_EXTAPP) {}
	~ExtAppTab() {}

	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
};
#endif

// ChooseColor dialog
class ColorTab : public TomboPropertyTab {
protected:
	static LRESULT APIENTRY CustomPageProc(HWND hDlg, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void Choose(HWND hDlg, DWORD nCtlId, HBRUSH* pBrush, COLORREF *pColor);

public:
	COLORREF cFg, cBg, cEol, cLEol, cTab, cEof;
	HBRUSH hFgBrush, hBgBrush, hEolBrush, hLEolBrush, hTabBrush, hEofBrush;

	ColorTab(Property *p) :
	  TomboPropertyTab(p, IDD_PROPTAB_COLOR, (DLGPROC)CustomPageProc, MSG_PROPTAB_COLOR) {}
	~ColorTab();

	void Init(HWND hDlg);
	BOOL Apply(HWND hDlg);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
};

#endif