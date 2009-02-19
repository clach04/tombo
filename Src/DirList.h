#ifndef DIRLIST_H
#define DIRLIST_H

#include "VarBuffer.h"

#define DIRLIST_GETLIST_RESULT_FAIL 0
#define DIRLIST_GETLIST_RESULT_SUCCESS 1
#define DIRLIST_GETLIST_RESULT_PARTIAL 2

//////////////////////////////////////////////////////
// Directory item information
//////////////////////////////////////////////////////

struct DirListItem {
	DWORD nFileNamePos;
	DWORD nURIPos;
	DWORD nHeadLinePos;
	BOOL bFolder;
};

//////////////////////////////////////////////////////
// Directory list scanner
//////////////////////////////////////////////////////

class DirList {
	LPCTSTR pURIBase;
public:
	TVector<DirListItem> vDirList;
	StringBufferT sbDirList;

	BOOL Init(LPCTSTR pURIBase);
	~DirList();

	DWORD GetList(LPCTSTR pMatchPath, BOOL bSkipEncrypt, BOOL bLooseDecrypt);

	DWORD NumItems() { return vDirList.NumItems(); }
	DirListItem *GetItem(DWORD n) { return vDirList.GetUnit(n); }
	LPCTSTR GetFileName(DWORD n) { return sbDirList.Get(n); }
};

#endif
