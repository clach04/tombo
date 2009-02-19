#ifndef URISCANNER_H
#define URISCANNER_H

class TomboURI;
class IEnumRepository;

//////////////////////////////////////////////////
// List of URI
//////////////////////////////////////////////////
//
// item order is display order.

class URIList {
protected:
	struct URIListItem {
		TomboURI *pURI;
		LPTSTR pTitle;

		URIListItem() : pURI(NULL), pTitle(NULL) {}
	};

	TVector<URIListItem> liList;
public:
	URIList();
	virtual ~URIList();

	BOOL Init();

	DWORD GetSize() { return liList.NumItems(); }
	const TomboURI *GetURI(DWORD n) { return liList.GetUnit(n)->pURI; }
	LPCTSTR GetTitle(DWORD n) { return liList.GetUnit(n)->pTitle; }

	BOOL Add(const TomboURI *pURI, LPCTSTR pTitle);
};

//////////////////////////////////////////////////
// URI Scanner class
//////////////////////////////////////////////////

class URIScanner {
private:
	struct StackFrame {
		URIList *pList;
		DWORD nPos;

		StackFrame *pNext;
	};
	StackFrame *pTop;
	IEnumRepository *pRepository;
	TomboURI *pBaseURI;
	TString *pBaseTitle;
	BOOL bSkipEncrypt;

	int iDirection;	// 1 or -1

	const TomboURI *pCurrentURI;
	LPCTSTR pTitle;

	BOOL bStopScan;

	BOOL PushFrame(const TomboURI *pURI);
	void ClearStack();
	void LeaveFrame();

	BOOL MakeFrame(const TomboURI *pStartPos);

protected:
	LPCTSTR GetTitle() { return pTitle; }

	// Request to stop scanning. 
	// even if this flag is set, 5 methods are not interrupted and done at finish the method.
	// Notice PostFolder() and AfterScan() is called even if flag is set.

	virtual void InitialScan();
	virtual void AfterScan();

	virtual void PreFolder();
	virtual void PostFolder();

	virtual void Node();

public:
	URIScanner();
	~URIScanner();

	BOOL Init(IEnumRepository *pEnumIF, const TomboURI *pTopURI, BOOL bSkipEncrypt);

	void StopScan() { bStopScan = TRUE; }
	BOOL IsStopScan() { return bStopScan; }

	// The URI now scanning. You can use this value in 5 customizable methods.
	const TomboURI* CurrentURI() { return pCurrentURI; }

	BOOL FullScan();
	BOOL Scan(const TomboURI *pStartURL, BOOL bReverse);
};

#endif