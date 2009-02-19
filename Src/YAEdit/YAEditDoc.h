#ifndef YAEDITDOC_H
#define YAEDITDOC_H

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

	BOOL ReplaceString(const Region *pRegion, LPCTSTR pString);
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
	// set current document status as undo point
	BOOL InsertUndoPoint();

	////////////////////////////////////////////////////
	// only for testing

	LPCTSTR GetUndoStr();
	const Region GetUndoRegion();
};

#endif