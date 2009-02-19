#include <windows.h>
#include "List.h"

///////////////// データのクリア
//
// 保持しているデータについては開放しないので必要なら
// Clear()される前に明示的にクリアする必要がある。

void List::Clear()
{
	Item *p = pHead;
	Item *q;

	while(p) {
		q = p;
		p = p->pNext;
		LocalFree(q);
	}

	pHead = pTail = NULL;
}

////////////////// リストへの追加
//
// pをリストの末尾へ追加

HANDLE List::Add(LPVOID val)
{
	Item *q = (Item*)LocalAlloc(LMEM_FIXED, sizeof(Item));
	if (q == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}

	if (pTail) {
		pTail->pNext = q;
		q->pPrev = pTail;
		pTail = q;
	} else {
		pHead = pTail = q;
		q->pPrev = NULL;
	}
	q->pVal = val;
	q->pNext = NULL;
	return (HANDLE)q;
}

HANDLE List::First()
{
	return (HANDLE)pHead;
}

HANDLE List::Next(HANDLE h)
{
	return (HANDLE)(((Item*)h)->pNext);
}

HANDLE List::Prev(HANDLE h)
{
	return (HANDLE)(((Item*)h)->pPrev);
}

LPVOID List::Value(HANDLE h)
{
	if (h == NULL) return NULL;
	Item *p = (Item*)h;
	return p->pVal;
}

LPVOID List::Remove(HANDLE h)
{
	Item *p = (Item*)h;
	LPVOID val = p->pVal;

	if (p->pPrev) {
		p->pPrev->pNext = p->pNext;
	} else {
		pHead = p->pNext;
	}
	if (p->pNext) {
		p->pNext->pPrev = p->pPrev;
	} else {
		pTail = p->pPrev;
	}

	LocalFree(p);
	return val;
}

//////////////////////////////////
// 文字列リストに対するヘルパ関数

HANDLE SearchList(List *l, const char *str)
{
	HANDLE h = l->First();
	while(h) {
		if (strcmp((char*)l->Value(h), str) == 0) {
			return h;
		}
		h = l->Next(h);
	}
	return NULL;
}