#ifndef TOMBO_STRING_H
#define TOMBO_STRING_H

////////////////////////////////////
// TCHAR string
////////////////////////////////////

class TString {
	LPTSTR pString;
public:
	TString() : pString(NULL) {}
	~TString() { if (pString) delete [] pString; }

	LPTSTR Get() const { return pString; }

	BOOL Alloc(DWORD nSize);
	BOOL Set(LPCTSTR p);
	BOOL StrCat(LPCTSTR p);

	BOOL Join(LPCTSTR p1, LPCTSTR p2);
	BOOL Join(LPCTSTR p1, LPCTSTR p2, LPCTSTR p3);
	BOOL Join(LPCTSTR p1, LPCTSTR p2, LPCTSTR p3, LPCTSTR p4);

	// pFullPathからディレクトリ部のみを取得
	// XX\YY\ZZ => XX\YY
	BOOL GetDirectoryPath(LPCTSTR pFullPath);

	// pFullPathから末尾を取得
	// XX\YY\ZZ => ZZ
	BOOL GetPathTail(LPCTSTR pFullPath);

	void ChopExtension();
	void ChopFileNumber();
	void ChopFileSeparator();
};

////////////////////////////////////
// String class with reference count
////////////////////////////////////

class SharedString {
	struct SharedBuf {
		LPTSTR pStr;
		WORD nRefCount;
	};

	SharedBuf *pBuf;

	friend class SharedStringTest;
protected:
	void ReleaseBuf();
	void Ref(const SharedString& s);

	BOOL Alloc(DWORD nSize);
public:
	SharedString();
	SharedString(const SharedString& s);

	~SharedString();

	BOOL Init(LPCTSTR p);
	BOOL Init(const SharedString& s);

	BOOL Set(LPCTSTR p);
	BOOL Set(const SharedString& s);
	LPCTSTR Get() const;

#ifdef UNIT_TEST
	SharedBuf *GetBuf() { return pBuf; }
#endif
};

////////////////////////////////////
// WCHAR string
////////////////////////////////////

class WString {
	LPWSTR pString;
public:
	WString() : pString(NULL) {}
	~WString() { if (pString) delete [] pString; }

	LPWSTR Get() { return pString; }
	BOOL Alloc(DWORD nLetters);

	BOOL Set(TString *pSrc);
};

#endif