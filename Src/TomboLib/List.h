#ifndef LIST_H
#define LIST_H

////////////////////////////////
// LinkedList
////////////////////////////////

class List {
	struct Item {
		Item *pNext;
		Item *pPrev;
		LPVOID pVal;
	};

	Item *pHead, *pTail;
public:
	
	List() : pHead(NULL), pTail(NULL){}
	~List() { Clear(); }

	void Clear();

	HANDLE Add(LPVOID val);

	LPVOID Remove(HANDLE h);

	HANDLE First();
	HANDLE Next(HANDLE h);
	HANDLE Prev(HANDLE h);

	LPVOID Value(HANDLE h);
};

HANDLE SearchList(List *l, const char *str);

#endif
