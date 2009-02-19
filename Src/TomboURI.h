#ifndef TOMBOURI_H
#define TOMBOURI_H

/////////////////////////////////////////////
// Notes path information
/////////////////////////////////////////////

class TomboURI {
	int nMaxPathItem;

	SharedString uri;

	friend class TomboURITest;
public:
	///////////////////////////////
	// ctor, dtor and initializer
	TomboURI();
	TomboURI(const TomboURI&);
	~TomboURI();

	BOOL Init(LPCTSTR pURI);
	BOOL Init(const TomboURI&);
	BOOL InitByNotePath(LPCTSTR pRepoName, LPCTSTR pNotePath);

	///////////////////////////////
	// accessor

	BOOL GetRepositoryName(TString *pRepo) const;

	// get full path of URI.
	LPCTSTR GetFullURI() const { return uri.Get(); }

	// get path part of URI.
	// ex. tombo://default/aa/bb/cc.txt -> /aa/bb/cc.txt
	LPCTSTR GetPath() const;

	// get parent path of URI.
	// ex. tombo://default/aa/bb/cc.txt -> tombo://default/aa/bb/
	//     tombo://default/aa/bb/       -> tombo://default/aa/
	//     tombo://default/				-> tombo://default/
	BOOL GetParent(TomboURI *pParent) const;

	BOOL GetAttachFolder(TomboURI *pAttach) const;

	DWORD GetMaxPathItem() const { return nMaxPathItem; }

	// get last 1 item from URI.
	// ex. tombo://default/aa/bb/cc.txt -> cc.txt
	//     tombo://default/aa/bb/cc/    -> cc
	BOOL GetBaseName(TString *pBase) const;

	// Is the URI point to leaf node?
	// Checking does only to URI string. Not confirm to repository.
	BOOL IsLeaf() const;

	// Is the URI point to root node?
	BOOL IsRoot() const;

	// Get path string
	// This method will be obsoleted in future version.
	// ex. tombo://default/aaa/bbb/ccc.txt -> aaa\bbb\ccc.txt
	BOOL GetFilePath(TString *pPath) const;


	///////////////////////////////
	// helper functions
	static LPCTSTR GetNextSep(LPCTSTR p);

	TomboURI &operator=(const TomboURI &uri);
};

/////////////////////////////////////////////
// path item iterator
/////////////////////////////////////////////

class TomboURIItemIterator {
	const TomboURI *pURI;
	LPTSTR pBuf;
	DWORD nPos;
public:

	///////////////////////////////
	// ctor, dtor and initializer
	TomboURIItemIterator(const TomboURI *p) : pURI(p), pBuf(NULL) {}
	~TomboURIItemIterator() { if (pBuf) delete[] pBuf; }
	BOOL Init();

	///////////////////////////////
	// iteration methods
	void First();
	LPCTSTR Current();
	void Next();

	///////////////////////////////
	// May current item have child?
	BOOL IsLeaf();
};

#endif