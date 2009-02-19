#include <windows.h>
#include <tchar.h>
#include "MemManager.h"

#define INITIAL_HEAP_SIZE 2048

/////////////////////////////////////////////////////////////////////////////
// ctor & dtor, initializer
/////////////////////////////////////////////////////////////////////////////

MemManager::MemManager() : hHeap(NULL)
{
}

MemManager::~MemManager()
{
	if (hHeap) {
		HeapDestroy(hHeap);
	}
}

BOOL MemManager::Init()
{
	hHeap = HeapCreate(0, INITIAL_HEAP_SIZE, 0);
	if (hHeap == NULL) return FALSE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Allocate memory
/////////////////////////////////////////////////////////////////////////////

MemBlock *MemManager::Alloc(DWORD nLetters)
{
	WORD nlen = sizeof(MemBlock) + nLetters * sizeof(TCHAR);
	MemBlock *p = (MemBlock*)HeapAlloc(hHeap, 0, nlen);
	if (!p) return NULL;
	p->nBlockType = MEMBLOCK_TYPE_NORMAL;
	p->nBlockLen = (WORD)nLetters;
	p->nUsed = 0;
	return p;
}

/////////////////////////////////////////////////////////////////////////////
// Allocate memory with memory copy
/////////////////////////////////////////////////////////////////////////////

MemBlock *MemManager::Alloc(LPCTSTR pStr, DWORD nLetters)
{
	MemBlock *p = Alloc(nLetters);
	if (p == NULL) return NULL;

	p->nUsed = (WORD)nLetters;
	LPTSTR pData = p->GetDataArea();
	memcpy(pData, pStr, nLetters * sizeof(TCHAR));
	return p;
}

/////////////////////////////////////////////////////////////////////////////
// free memory
/////////////////////////////////////////////////////////////////////////////
void MemManager::Free(MemBlock *p)
{
	HeapFree(hHeap, 0, p);
}

/////////////////////////////////////////////////////////////////////////////
// Extend memory
/////////////////////////////////////////////////////////////////////////////
MemBlock *MemManager::Extend(MemBlock *pBlock, DWORD nPos, DWORD nSize)
{
	if (pBlock->nUsed + nSize >= pBlock->nBlockLen) {
		// need realloc
		DWORD nNeedBytes = nSize * sizeof(TCHAR);
		DWORD nDeltaBytes = (nNeedBytes / sizeof(MemBlock) + 1) * sizeof(MemBlock);

		pBlock = (MemBlock*)HeapReAlloc(hHeap, 0, pBlock, sizeof(MemBlock) + (pBlock->nUsed)*sizeof(TCHAR) + nDeltaBytes);
		if (pBlock == NULL) return NULL;
		pBlock->nBlockLen += nDeltaBytes / sizeof(TCHAR);
	}
	LPTSTR p = pBlock->GetDataArea();
	memmove(p + (nPos + nSize), p + nPos, (pBlock->nUsed - nPos)* sizeof(TCHAR));
	pBlock->nUsed += nSize;
	return pBlock;
}

/////////////////////////////////////////////////////////////////////////////
// concat string
/////////////////////////////////////////////////////////////////////////////

MemBlock *MemManager::ConCat(MemBlock *pBlock, LPCTSTR pStr, DWORD nLetters)
{
	DWORD nUsed = pBlock->nUsed;
	MemBlock *p = Extend(pBlock, pBlock->nUsed, nLetters);
	if (p == NULL) return NULL;

	_tcsncpy(p->GetDataArea() + nUsed, pStr, nLetters);
	return p;
}