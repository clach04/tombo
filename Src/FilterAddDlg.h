#ifndef FILTERADDDLG_H
#define FILTERADDDLG_H

class TString;

////////////////////////////////////////////////////////////////
// Regex filter
////////////////////////////////////////////////////////////////

class RegexFilterAddDlg : public Tombo_Lib::DialogTemplate {
	TString *pMatchStr;
	BOOL bCaseSensitive;
	BOOL bCheckEncrypt;
	BOOL bMatchFileName;
	BOOL bNegate;

public:
	~RegexFilterAddDlg();
	BOOL Init(LPCTSTR pMatch = NULL, 
				BOOL bCS = FALSE, BOOL bCE = FALSE, 
				BOOL bFile = FALSE, BOOL bNeg = FALSE);

	void InitDialog(HWND hDlg);
	BOOL OnOK();

	DWORD Popup(HINSTANCE hInst, HWND hParent) {
		return DialogTemplate::Popup(hInst, IDD_FILTERDEF_ADD_REGEX, hParent);
	}

	////////////////////////////
	// Accessor
	TString *GetMatchString() { return pMatchStr; }
	BOOL IsCaseSensitive() { return bCaseSensitive; }
	BOOL IsCheckEncrypt() { return bCheckEncrypt; }
	BOOL IsCheckFileName() { return bMatchFileName; }
	BOOL IsNegate() { return bNegate; }
};

////////////////////////////////////////////////////////////////
// Regex filter
////////////////////////////////////////////////////////////////

class LimitFilterAddDlg : public Tombo_Lib::DialogTemplate {
	DWORD nLimit;
public:
	LimitFilterAddDlg() : nLimit(0) {}
	BOOL Init(DWORD n = 0) { nLimit = n; return TRUE; }

	void InitDialog(HWND hDlg);
	BOOL OnOK();

	DWORD Popup(HINSTANCE hInst, HWND hParent) {
		return DialogTemplate::Popup(hInst, IDD_FILTERDEF_ADD_LIMIT, hParent);
	}

	////////////////////////////
	// Accessor
	DWORD GetLimit() { return nLimit; }
};

////////////////////////////////////////////////////////////////
// Timestamp filter
////////////////////////////////////////////////////////////////

class TimestampFilterAddDlg : public Tombo_Lib::DialogTemplate {
	DWORD nDeltaDays;
	BOOL bNewer;
public:
	TimestampFilterAddDlg() : nDeltaDays(0), bNewer(FALSE) {}
	BOOL Init(DWORD nDelta = 0, BOOL bNew = FALSE);

	void InitDialog(HWND hDlg);
	BOOL OnOK();

	DWORD Popup(HINSTANCE hInst, HWND hParent) {
		return DialogTemplate::Popup(hInst, IDD_FILTERDEF_ADD_TIMESTAMP, hParent);
	}

	////////////////////////////
	// Accessor
	DWORD GetDeltaDay() { return nDeltaDays; }
	BOOL IsNewer() { return bNewer; }

};

////////////////////////////////////////////////////////////////
// Sort filter
////////////////////////////////////////////////////////////////

class SortFilterAddDlg : public Tombo_Lib::DialogTemplate {
	VFSortFilter::SortFuncType sfType;
public:
	BOOL Init(VFSortFilter::SortFuncType sf = VFSortFilter::SortFunc_Unknown) { sfType = sf; return TRUE; }

	void InitDialog(HWND hDlg);
	BOOL OnOK();

	DWORD Popup(HINSTANCE hInst, HWND hParent) {
		return DialogTemplate::Popup(hInst, IDD_FILTERDEF_ADD_SORT, hParent);
	}

	VFSortFilter::SortFuncType GetType() { return sfType; }

};

#endif