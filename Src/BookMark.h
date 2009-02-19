#ifndef BOOKMARK_H
#define BOOKMARK_H

////////////////////////////////////
// book mark items
////////////////////////////////////

struct BookMarkItem {
	WORD nID;
	LPTSTR pPath;
	LPTSTR pName;
};

////////////////////////////////////
// book mark
////////////////////////////////////

class BookMark {
	WORD nBookMarkIDBase;
	WORD nMaxBookMarkID;
	TVector<BookMarkItem> vBookMark;
public:
	////////////////////////////////////////
	// ctor & dtor, initialize
	BookMark();
	~BookMark();
	BOOL Init(WORD nBaseID);

	void Clear();

	////////////////////////////////////////
	// get/check ID members

	BOOL IsBookMarkID(WORD nID);
	WORD GetBaseID() { return nBookMarkIDBase; }
	WORD GetAssignedID() { return nMaxBookMarkID; }

	////////////////////////////////////////
	// add/find bookmark

	const BookMarkItem *Assign(LPCTSTR pPath);
	const BookMarkItem *Find(WORD nID);

	////////////////////////////////////////
	// enumeration methods
	DWORD NumItems();
	const BookMarkItem *GetUnit(DWORD n);

	////////////////////////////////////////
	// for load/store bookmarks
	BOOL ImportFromMultiSZ(LPCTSTR pBookMarks);
	LPTSTR ExportToMultiSZ();

};

#endif