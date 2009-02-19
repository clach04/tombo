#ifndef TSCOMPILER_H
#define TSCOMPILER_H

#include <expat.h>
class MemoSelectView;
class ParseInfo;
class VFStream;
class TreeViewVirtualFolder;
class VirtualFolderEnumListener;

////////////////////////////////////
// TOMBO script parser
////////////////////////////////////

class TSParser {
public:
	TSParser();
	~TSParser();

	BOOL Parse(LPCTSTR pFileName, VirtualFolderEnumListener *pListener);
};

///////////////////////////////////////
// XML TAG abstruction
///////////////////////////////////////

class TSParseTagItem {
protected:
	DWORD nTagID;
	TSParseTagItem *pNext;

public:
	VFStream *pHead;
	VFStream *pTail;

	////////////////////////
	// ctor & dtor

	TSParseTagItem(DWORD nID) : nTagID(nID), pHead(NULL), pTail(NULL) {}
	virtual ~TSParseTagItem(); 

	////////////////////////
	// accessor & mutator

	DWORD GetTagID() { return nTagID; }

	////////////////////////
	// tag chain operation

	void SetNext(TSParseTagItem *p) { pNext = p; }
	TSParseTagItem *GetNext() { return pNext; }

	////////////////////////
	// Start element
	virtual BOOL StartElement(ParseInfo *p, const XML_Char **atts);

	////////////////////////
	// End element
	virtual BOOL EndElement(ParseInfo *p);
};

///////////////////////////////////////
// XML Parser helper
///////////////////////////////////////

class ParseInfo {
	BOOL bError;
	TSParseTagItem *pTop;
public:
	////////////////////////
	// ctor & dtor

	ParseInfo() : bError(FALSE), pTop(NULL), pListener(NULL) {}
	~ParseInfo();
	BOOL Init(VirtualFolderEnumListener *pLsnr);

	////////////////////////
	// public vars
	VirtualFolderEnumListener *pListener;

	////////////////////////
	// Error info

	BOOL IsError() { return bError; }
	void SetError() { bError = TRUE; }

	////////////////////////
	// check & get tag Object

	DWORD GetTagID(const WCHAR *pTagName);
	TSParseTagItem *GetTagObjectFactory(DWORD nTagID);

	////////////////////////
	// Tag info operation

	void Push(TSParseTagItem *p);
	TSParseTagItem *Top() { return pTop; }
	void Pop();

	////////////////////////
	// Tag validation
	BOOL IsValidParent(DWORD nTag);
};


#endif
