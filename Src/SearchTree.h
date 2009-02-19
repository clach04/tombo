#ifndef SEARCHTREE_H
#define SEARCHTREE_H

enum SearchResult;
class SearchEngineA;
class SearchTreeScanner;

///////////////////////////////////////////////////////////
// Popup "searching" dialog and do search another thread
///////////////////////////////////////////////////////////

class SearchTree {
protected:
	///////////////////////////////
	// dialog

	HWND hDlgWnd;

	///////////////////////////////
	// Thread and thread control vars.

	HANDLE hSearchThread;

	///////////////////////////////
	// for searching variables

	SearchEngineA *pRegex;
	BOOL bSearchDirectionForward;
	BOOL bSearchEncryptedMemo;
	BOOL bSkipOne;

	TomboURI *pStartURI;

	SearchTreeScanner *pScanner;
	TomboURI *pMatchedURI;

	SearchResult srResult;

public:
	////////////////////////
	// ctor & dtor

	SearchTree() : pRegex(NULL), hSearchThread(NULL), hDlgWnd(NULL), pStartURI(NULL), pScanner(NULL), pMatchedURI(NULL) {}
	~SearchTree();
	BOOL Init(SearchEngineA *p, const TomboURI *pStartURI, BOOL bDirectionForward, BOOL bSkipOne, BOOL bSkipEncrypt);

	////////////////////////
	// Dialog callback

	void InitDialog(HWND hDlg);
	void OnClose(HWND hDlg, WORD nId);

	HWND GetWnd() { return hDlgWnd; }

	////////////////////////
	// other funcs

	void Popup(HINSTANCE hInst, HWND hParent);
	SearchResult Search();

	void CancelRequest();

	void SetResult(SearchResult sr) { srResult = sr; }
	SearchResult GetResult() { return srResult; }

	const TomboURI *GetMatchedURI() { return pMatchedURI; }
	const TomboURI *CurrentURI();
};

#endif