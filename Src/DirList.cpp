#include <windows.h>
#include <tchar.h>
#include "Tombo.h"
#include "DirList.h"
#include "MemoNote.h"
#include "TString.h"
#include "TomboURI.h"
#include "Repository.h"

static StringBufferT *pSortSB;
extern "C" int SortItems(const void *e1, const void *e2)
{
	const DirListItem *p1 = (const DirListItem*)e1;
	const DirListItem *p2 = (const DirListItem*)e2;
	LPCTSTR q1 = pSortSB->Get(p1->nHeadLinePos);
	LPCTSTR q2 = pSortSB->Get(p2->nHeadLinePos);
	if (p1->bFolder == p2->bFolder) {
		return _tcsicmp(q1, q2);
	} else {
		if (p1->bFolder) {
			return -1;
		} else {
			return 1;
		}
	}
}

BOOL DirList::Init(LPCTSTR pUB)
{
	pURIBase = pUB;

	if (!vDirList.Init(50, 10)) return FALSE;
	if (!sbDirList.Init(400, 20)) return FALSE;
	return TRUE;
}

DirList::~DirList()
{
}

DWORD DirList::GetList(LPCTSTR pMatchPath, BOOL bSkipEncrypt, BOOL bLooseDecrypt)
{
	BOOL bPartial = FALSE;

	// make folder/file list
	WIN32_FIND_DATA wfd;
	HANDLE hHandle = FindFirstFile(pMatchPath, &wfd);
	if (hHandle != INVALID_HANDLE_VALUE) {
		struct DirListItem di;

		do {
			if (_tcscmp(wfd.cFileName, TEXT(".")) == 0 || _tcscmp(wfd.cFileName, TEXT("..")) == 0) continue;

			DWORD l = _tcslen(wfd.cFileName);
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// folder 
				di.bFolder = TRUE;
			} else {
				// file
				DWORD n = MemoNote::IsNote(wfd.cFileName);
				if (n == NOTE_TYPE_NO || n == NOTE_TYPE_TDT) continue;
				
				di.bFolder = FALSE;
			}

			if (!sbDirList.Add(pURIBase, _tcslen(pURIBase), &(di.nURIPos))) return DIRLIST_GETLIST_RESULT_FAIL;
			DWORD d;
			if (!sbDirList.Add(wfd.cFileName, l, &d)) return DIRLIST_GETLIST_RESULT_FAIL;
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (!sbDirList.Add(TEXT("/"), 1, &d)) return DIRLIST_GETLIST_RESULT_FAIL;
			}
			if (!sbDirList.Add(TEXT(""), 1, &d)) return DIRLIST_GETLIST_RESULT_FAIL;

			TomboURI sURI;
			TString sHeadLine;
			if (!sURI.Init(GetFileName(di.nURIPos))) return DIRLIST_GETLIST_RESULT_FAIL;

			URIOption opt(NOTE_OPTIONMASK_ENCRYPTED);
			g_Repository.GetOption(&sURI, &opt);

			if (!opt.bEncrypt || !bSkipEncrypt) {
				if (!g_Repository.GetHeadLine(&sURI, &sHeadLine)) {
					if (GetLastError() == ERROR_INVALID_PASSWORD && bLooseDecrypt) {
						bPartial = TRUE;
						sHeadLine.Set(TEXT("????????"));
					} else {
						return DIRLIST_GETLIST_RESULT_FAIL;
					}
				}
				if (!sbDirList.Add(sHeadLine.Get(), _tcslen(sHeadLine.Get()) + 1, &(di.nHeadLinePos))) return DIRLIST_GETLIST_RESULT_FAIL;
	
				// Add file name to buffer
				if (!sbDirList.Add(wfd.cFileName, l + 1, &(di.nFileNamePos))) return DIRLIST_GETLIST_RESULT_FAIL;
				if (!vDirList.Add(&di)) return DIRLIST_GETLIST_RESULT_FAIL;
			}
		} while(FindNextFile(hHandle, &wfd));
		FindClose(hHandle);
	}

	DWORD n = vDirList.NumItems();

	// sort 
	pSortSB = &sbDirList;
	qsort((LPBYTE)vDirList.GetBuf(), n, sizeof(struct DirListItem), SortItems);

	if (bPartial) {
		return DIRLIST_GETLIST_RESULT_PARTIAL;
	} else {
		return DIRLIST_GETLIST_RESULT_SUCCESS;
	}
}
