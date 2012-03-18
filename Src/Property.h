#ifndef PROPERTY_H
#define PROPERTY_H

#include <commctrl.h>

#define MAX_DATEFORMAT_LEN 256

////////////////////////////////////
// property index
////////////////////////////////////

#define PROP_N_PASSTIMEOUT				 0
#define PROP_N_DETAILSVIEW_KEEPCARET	 1
#define PROP_NDETAILSVIEW_TABSTOP		 2
#define PROP_N_SELECTVIEW_FONTSIZE		 3
#define PROP_N_SELECTVIEW_FONTQUALITY	 4
#define PROP_N_DETAILSVIEW_FONTSIZE		 5
#define PROP_N_DETAILSVIEW_FONTQUALITY	 6
#define PROP_N_AUTOSELECT_MODE			 7
#define PROP_N_SINGLECLICK_MODE			 8
#define PROP_N_USE_TWO_PANE				 9
#define PROP_N_SWITCH_WINDOW_TITLE		10
#define PROP_N_KEEP_TITLE				11
#define PROP_N_APP_BUTTON1				12
#define PROP_N_APP_BUTTON2				13
#define PROP_N_APP_BUTTON3				14
#define PROP_N_APP_BUTTON4				15
#define PROP_N_APP_BUTTON5				16
#define PROP_N_CODEPAGE					17
#define PROP_N_DISABLEEXTRAACTIONBUTTON	18	// PKTPC
#define PROP_N_SIPSIZE_DELTA			19	// PKTPC
#define PROP_N_HIDESTATUSBAR			20	// HPC, WIN32
#define PROP_N_STAYTOPMOST				21	// WIN32
#define PROP_N_HIDEREBAR				22	// WIN32
#define PROP_N_WRAPTEXT					23
#define PROP_N_OPENREADONLY				24
#define PROP_N_DISABLESAVEDLG			25
#define PROP_N_USEASSOC					26
#define PROP_N_SAFEFILENAME				27
#define PROP_N_TOMBO_WINSIZE3			28	// PKTPCVGA
#define PROP_N_KEEP_LAST_OPEN			29
#define PROP_N_USE_YAE					30
#define PROP_N_DISABLE_YAE              31
#define PROP_N_FGCOLOR					32
#define PROP_N_BGCOLOR					33
#define PROP_N_EOL_COLOR				34
#define PROP_N_LEOL_COLOR				35
#define PROP_N_TAB_COLOR				36
#define PROP_N_EOF_COLOR				37

#define NUM_PROPS_NUM 38

#define PROP_S_TOPDIR					 0
#define PROP_S_SELECTVIEW_FONTNAME		 1
#define PROP_S_DETAILSVIEW_FONTNAME		 2
#define PROP_S_DETAILSVIEW_DATEFORMAT1	 3
#define PROP_S_DETAILSVIEW_DATEFORMAT2	 4
#define PROP_S_DEFAULTNOTE				 5
#define PROP_S_EXTAPP1					 6
#define PROP_S_EXTAPP2					 7
#define PROP_S_WINSIZE					 8
#define PROP_S_LAST_OPEN_URI			 9

#define NUM_PROPS_STR 10

// file encoding related defs
#define TOMBO_CP_DEFAULT	    0
#define TOMBO_CP_UTF16LE	 1200	// UTF-16LE
#define TOMBO_CP_UTF8		65001	// UTF-8
#define TOMBO_CP_GREEK		 1253	// Greek

////////////////////////////////////
// accessor generation macros
////////////////////////////////////

#define STR_ACCESSOR(NAME, ATTR) \
	LPCTSTR Get##NAME##() { return pPropsStr[ATTR]; }\
	BOOL Set##NAME##(LPCTSTR pDir) { return SetStringProperty(ATTR, pDir); }

#define NUM_ACCESSOR(NAME, ATTR) \
	DWORD Get##NAME##() { return nPropsNum[ATTR]; }\
	void Set##NAME##(DWORD n) { nPropsNum[ATTR] = n; }

#define NUM_ACCESSOR_DECL(NAME) \
	DWORD Get##NAME##();\
	void Set##NAME##(DWORD n);

class File;
class TomboURI;
class RepositoryImpl;
class LocalFileRepository;

////////////////////////////////////
// Property data
////////////////////////////////////

class Property {

	// internal status
	BOOL bLoad;
	BOOL bNeedAsk;

	// persistent props
	DWORD nPropsNum[NUM_PROPS_NUM];
	LPTSTR pPropsStr[NUM_PROPS_STR];

	LPTSTR pBookMark;
	LPTSTR pSearchHistory;
	LPTSTR pTopDirHistory;

#if defined(PLATFORM_HPC)
	LPCOMMANDBANDSRESTOREINFO pCmdBarInfo;
#endif

	// not persistent props
	LPTSTR pCmdlineAssignedTopDir;

	// internal helper funcs 
	BOOL SetStringProperty(DWORD nPropId, LPCTSTR pValue);

	BOOL SaveToFile(File *pFile);
	BOOL LoadFromReg(BOOL *pStrict);
	BOOL LoadProperties();
	BOOL LoadDefaultProperties();

	RepositoryImpl **pRepos;
	LocalFileRepository *pDefaultRep;
	DWORD nNumRepos;

public:
	Property();
	~Property();

	/////////////////////////////////
	// set default props

	BOOL SetDefaultROMode(BOOL) {return TRUE; }
	BOOL SetCmdLineAssignedTomboRoot(LPCTSTR p, DWORD nLen);

	/////////////////////////////////
	// accessor

	// Repository related
	DWORD GetNumSubRepository() { return nNumRepos; }
	// Clone RepsitoryImpl and return it
	RepositoryImpl *GetSubRepository(DWORD nIndex);

	// startup related

	STR_ACCESSOR(TopDir, PROP_S_TOPDIR)		// TOMBO root directory
	// GetTomboRoot 
	LPCTSTR GetTomboRoot();

	NUM_ACCESSOR_DECL(KeepCaret) // whether keep caret position or not
	NUM_ACCESSOR_DECL(UseSafeFileName)	// change crypted file name random
	NUM_ACCESSOR_DECL(KeepTitle) // keep file name even if headline has changed

	STR_ACCESSOR(LastOpenURI, PROP_S_LAST_OPEN_URI) // notes store folder
	NUM_ACCESSOR(KeepLastOpen, PROP_N_KEEP_LAST_OPEN)	// when starting, the note last open is opened.
	STR_ACCESSOR(DefaultNote, PROP_S_DEFAULTNOTE)	// use this uri when starting tombo

	// font related
	STR_ACCESSOR(SelectViewFontName, PROP_S_SELECTVIEW_FONTNAME)
	NUM_ACCESSOR(SelectViewFontSize, PROP_N_SELECTVIEW_FONTSIZE)	// selectview font size
	NUM_ACCESSOR(SelectViewFontQuality, PROP_N_SELECTVIEW_FONTQUALITY) // do use selectview ClearType font?
	STR_ACCESSOR(DetailsViewFontName, PROP_S_DETAILSVIEW_FONTNAME)
	NUM_ACCESSOR(DetailsViewFontSize, PROP_N_DETAILSVIEW_FONTSIZE)	// editview font size
	NUM_ACCESSOR(DetailsViewFontQuality, PROP_N_DETAILSVIEW_FONTQUALITY) // do use editview ClearType font? 
	HFONT SelectViewFont();
	HFONT DetailsViewFont();

	// editview related
	STR_ACCESSOR(DateFormat1, PROP_S_DETAILSVIEW_DATEFORMAT1)
	STR_ACCESSOR(DateFormat2, PROP_S_DETAILSVIEW_DATEFORMAT2)
	NUM_ACCESSOR(Tabstop, PROP_NDETAILSVIEW_TABSTOP)	// tab stop
	NUM_ACCESSOR(WrapText, PROP_N_WRAPTEXT)	// text wrapping on editview
	NUM_ACCESSOR(OpenReadOnly, PROP_N_OPENREADONLY)	// always read only mode when open the nots.
	NUM_ACCESSOR(DisableSaveDlg, PROP_N_DISABLESAVEDLG)	// disable asking save when closing notes
	NUM_ACCESSOR(CodePage, PROP_N_CODEPAGE) 	// Codepage selection

	// crypt related
	NUM_ACCESSOR(PassTimeout, PROP_N_PASSTIMEOUT)	// password timeout	

	// MainFrame related
	NUM_ACCESSOR(AutoSelectMemo, PROP_N_AUTOSELECT_MODE)	// display note when treeview selection is moving
	NUM_ACCESSOR(SingleClick, PROP_N_SINGLECLICK_MODE)	// display note when clicking the tree item 
	NUM_ACCESSOR(SwitchWindowTitle, PROP_N_SWITCH_WINDOW_TITLE) // is sync window text to note's name?
	NUM_ACCESSOR(UseTwoPane, PROP_N_USE_TWO_PANE) // is two pane mode?

	// extapp related
	NUM_ACCESSOR(UseAssociation, PROP_N_USEASSOC)	// use default file assosiation
	STR_ACCESSOR(ExtApp1, PROP_S_EXTAPP1)
	STR_ACCESSOR(ExtApp2, PROP_S_EXTAPP2)

	// Use YAEDIT component. On 1.x, always FALSE.
	NUM_ACCESSOR(UseYAEdit, PROP_N_USE_YAE)
	NUM_ACCESSOR(DisableYAEdit, PROP_N_DISABLE_YAE)


	NUM_ACCESSOR(AppButton1, PROP_N_APP_BUTTON1)	// application buttons
	NUM_ACCESSOR(AppButton2, PROP_N_APP_BUTTON2)
	NUM_ACCESSOR(AppButton3, PROP_N_APP_BUTTON3)
	NUM_ACCESSOR(AppButton4, PROP_N_APP_BUTTON4)
	NUM_ACCESSOR(AppButton5, PROP_N_APP_BUTTON5)

#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
	NUM_ACCESSOR(SipSizeDelta, PROP_N_SIPSIZE_DELTA)

	// Disable open/close notes when action button pushed
	NUM_ACCESSOR(DisableExtraActionButton, PROP_N_DISABLEEXTRAACTIONBUTTON)

#endif

#if defined(PLATFORM_HPC) || defined(PLATFORM_WIN32)
	NUM_ACCESSOR(HideStatusBar, PROP_N_HIDESTATUSBAR)	// Hide status bar
	void ToggleShowStatusBar() { nPropsNum[PROP_N_HIDESTATUSBAR] = !nPropsNum[PROP_N_HIDESTATUSBAR]; }
#else
	DWORD GetHideStatusBar() { return TRUE; }
	void ToggleShowStatusBar() { /* nop */ }
#endif
#if defined(PLATFORM_WIN32)
	NUM_ACCESSOR(StayTopMost, PROP_N_STAYTOPMOST)	// keep tombo topmost of the window
	void ToggleStayTopMost() { nPropsNum[PROP_N_STAYTOPMOST] = !nPropsNum[PROP_N_STAYTOPMOST]; }

	NUM_ACCESSOR(HideRebar, PROP_N_HIDEREBAR)	// hide rebar(toolbar)
	void ToggleShowRebar() { nPropsNum[PROP_N_HIDEREBAR] = !nPropsNum[PROP_N_HIDEREBAR]; }
#endif

	// Color related props
	NUM_ACCESSOR(FgColor, PROP_N_FGCOLOR)
	NUM_ACCESSOR(BgColor, PROP_N_BGCOLOR)
	NUM_ACCESSOR(EolColor, PROP_N_EOL_COLOR)
	NUM_ACCESSOR(LEolColor, PROP_N_LEOL_COLOR)
	NUM_ACCESSOR(TabColor, PROP_N_TAB_COLOR)
	NUM_ACCESSOR(EofColor, PROP_N_EOF_COLOR)

	// save restore main window size
	BOOL SaveWinSize(UINT flags, UINT showCmd, LPRECT pWinRect, WORD nSelectViewWidth);
	BOOL GetWinSize(UINT *pFlags, UINT *pShowCmd, LPRECT pWinRect, LPWORD pSelectViewWidth);

#if (defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)) && defined(FOR_VGA)
	NUM_ACCESSOR(WinSize2, PROP_N_TOMBO_WINSIZE3)	// horizontal pane size
#endif

	LPCTSTR GetBookMark() { return pBookMark; }
	BOOL SetBookMark(LPCTSTR pBookMark);

	LPCTSTR GetSearchHist() { return pSearchHistory; } 
	void SetSearchHist(LPTSTR pHist) { delete [] pSearchHistory; pSearchHistory = pHist; }

	LPCTSTR GetTopDirHist() { return pTopDirHistory; }
	void SetTopDirHist(LPTSTR pHist) { delete [] pTopDirHistory; pTopDirHistory = pHist; }

#if defined(PLATFORM_HPC)
	// save commandbar position
	BOOL SetCommandbarInfo(LPCOMMANDBANDSRESTOREINFO p);
	void SetCommandbarInfoWithBuffer(LPCOMMANDBANDSRESTOREINFO p) { delete [] pCmdBarInfo; pCmdBarInfo = p; }
	BOOL GetCommandbarInfo(LPCOMMANDBANDSRESTOREINFO p, DWORD n);
#endif

	void SetNumberPropertyById(DWORD nId, DWORD nValue) { nPropsNum[nId] = nValue; }
	void SetStringPropertyWithBuffer(DWORD nId, LPTSTR pValue) { pPropsStr[nId] = pValue; }

	/////////////////////////////////
	// main op

	// popup property dialog
	DWORD Popup(HINSTANCE hInst, HWND hWnd, const TomboURI *pCurrentSelectedURI);

	// load properties
	BOOL Load();

	// save properties
	BOOL Save();

	// check object status
	BOOL IsLoaded() { return bLoad; }
	BOOL IsNeedAskUser() { return bNeedAsk; }
};

////////////////////////////////////
// Search history
////////////////////////////////////

BOOL SetHistoryToComboBox(HWND hCombo, LPCTSTR pHistoryStr);
LPTSTR GetHistoryFromComboBox(HWND hCombo, LPCTSTR pSelValue, DWORD nSave);

////////////////////////////////////
// global var declaration
////////////////////////////////////
extern Property g_Property;

/////////////////////////////////////////////
// attribute name definitions
/////////////////////////////////////////////

// some codes assumes this value is just '2'. check if this value is changed.
#define NUM_COMMANDBAR 2

/////////////////////////////////////////////
// Code conversion related
/////////////////////////////////////////////

LPBYTE ConvTCharToFileEncoding(LPCTSTR p, LPDWORD pSize);

// p is assumed terminated by '\0' if encoding is MBCS/UTF-8 and L'\0' if encoding is UTF-16
LPTSTR ConvFileEncodingToTChar(LPBYTE p);

#endif
