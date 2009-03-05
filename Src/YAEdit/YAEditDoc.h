#ifndef YAEDITDOC_H
#define YAEDITDOC_H

#include "TString.h"

class YAEditImpl;
class Region;
class PhysicalLineManager;
class YAEditCallback;
class YAEditListener;

class UndoInfo;


////////////////////////////////////////////////////
// Document container for YAE
////////////////////////////////////////////////////

class YAEditDoc {
protected:
	PhysicalLineManager *pPhLineMgr;
	YAEditListener *pListener;

	YAEditCallback *pCallback;

	// undo related
	UndoInfo *pUndo;

	// this member should not to edit directry. use SetModify().
	BOOL bModified;
	BOOL bReadOnly;

	BOOL ReleaseDoc();
public:
	////////////////////////////////////////////////////
	// ctor, dtor & initialize

	YAEditDoc();
	~YAEditDoc();

	BOOL Init(const char *pStr, YAEditListener *pListener, YAEditCallback*pCb);

	PhysicalLineManager *GetPhMgr() { return pPhLineMgr; }

	////////////////////////////////////////////////////
	// set/get document

	BOOL LoadDoc(const char *pStr);
	char *GetDocumentData(LPDWORD pLen);

	////////////////////////////////////////////////////
	// retrieve & set line 

	// bKeepUndo is usualy set FALSE. Only undo operation set true.
	// when the value is true, ReplaceString do not change UndoInfo value.
	BOOL ReplaceString(const Region *pRegion, LPCTSTR pString, BOOL bKeepUndo = FALSE);

	BOOL Undo();

	BOOL IsModify() { return bModified; }
	void SetModify(BOOL b);

	BOOL IsReadOnly() { return bReadOnly; }
	void SetReadOnly(BOOL b);

	////////////////////////////////////////////////////
	// Data size related functions
	DWORD GetDataBytes(const Region *pRegion);
	void ConvertBytesToCoordinate(DWORD nPos, Coordinate *pPos);

	////////////////////////////////////////////////////
	//
	void CloseUndoRegion();

	////////////////////////////////////////////////////
	// only for testing
#ifdef UNIT_TEST
	UndoInfo *GetUndoInfo() { return pUndo; }
#endif
};

/////////////////////////////////////////////////////////////////////////////
// undo info
/////////////////////////////////////////////////////////////////////////////

class UndoInfo {
#ifdef UNIT_TEST
public:
#endif
	TString sPrevStr;
	Region rPrevRegion;

	TString sNewStr;
	Region rNewRegion;

	// true if this undo is applied. At creation time, this value is false and set true
	// when YAEditDoc::Undo is called.
	BOOL bUndoApplied;

	// Assume type 'a' and 'b', we expect remove 'ab' when undoing. If this value is ture, 
	// UndoInfo merge current changes and previous changes.
	BOOL bOpenRegion;
public:
	UndoInfo();
	~UndoInfo();

	BOOL UpdateUndoRegion(const Region *pPrevRegion, LPCTSTR pPrevStr, 
							const Region *pNewRegion, LPCTSTR pNewStr);

	BOOL CmdUndo(YAEditDoc *pDoc);

	void CloseUndoRegion();
	BOOL IsOpened() { return bOpenRegion; }
};

#endif