#include <windows.h>
#include <tchar.h>
#include "Message.h"
#include "VarBuffer.h"
#include "VFManager.h"
#include "VFStream.h"
#include "PasswordManager.h"
#include "Tombo.h"
#include "UniConv.h"
#include "TString.h"
#include "TSParser.h"
#include "File.h"

/////////////////////////////////////////////
//  ctor & dtor
/////////////////////////////////////////////

VFManager::VFManager() : nGrepCount(0)
{
}

VFManager::~VFManager()
{
	ClearInfo();
}

BOOL VFManager::Init()
{
	return vbInfo.Init(5,5);
}

void VFManager::ClearInfo()
{
	// delete vbInfo's items memory
	DWORD n = vbInfo.NumItems();
	VFInfo *p;
	for (DWORD i = 0; i < n; i++) {
		p = vbInfo.GetUnit(i);
		p->Release();
	}
	vbInfo.Clear(FALSE);
}

/////////////////////////////////////////////
//  Create stream from VFInfo
/////////////////////////////////////////////

BOOL VFManager::StreamObjectsFactory(const VFInfo *pInfo, VFDirectoryGenerator **ppGen, VFStore **ppStore)
{
	*ppGen = (VFDirectoryGenerator*)pInfo->pGenerator->Clone(ppStore);
	return *ppGen != NULL;
}

/////////////////////////////////////////////
//  Get VFInfo
/////////////////////////////////////////////

const VFInfo *VFManager::GetGrepVFInfo(LPCTSTR pPath, LPCTSTR pRegex,
						BOOL bIsCaseSensitive, BOOL bCheckCrypt, BOOL bCheckFileName, BOOL bNegate)
{
	if (pPath == NULL || pRegex == NULL) {
		SetLastError(ERROR_INVALID_DATA);
		return NULL;
	}
	VFInfo vInfo;

	// Create template stream
	VFDirectoryGenerator *pGen = new VFDirectoryGenerator();
	VFStore *pStore = new VFStore();
	VFRegexFilter *pFilter = new VFRegexFilter();
	if (!pGen || !pStore || !pFilter) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		delete pGen;
		delete pStore;
		delete pFilter;
		return NULL;
	}
	if (!pGen->Init(pPath, bCheckCrypt) ||
		!pFilter->Init(pRegex, bIsCaseSensitive, bCheckCrypt, bCheckFileName, bNegate, g_pPasswordManager) ||
		!pStore->Init()) {
		delete pGen;
		delete pStore;
		delete pFilter;
		return NULL;
	}

	pGen->SetNext(pFilter); pFilter->SetNext(pStore); //link objects
	vInfo.bPersist = FALSE;
	vInfo.pGenerator = pGen;
	vInfo.pStore = pStore;

	vInfo.pName = GetNodeName();
	if (vInfo.pName == NULL) return NULL;

	// Regist data
	if (!vbInfo.Add(&vInfo)) return FALSE;
	DWORD n = vbInfo.NumItems();
	VFInfo *q = vbInfo.GetUnit(n - 1);

	return q;
}

/////////////////////////////////////////////
//  Enum vfolders
/////////////////////////////////////////////

BOOL VFManager::Enum(VirtualFolderEnumListener *pListener)
{
	VFInfo *pInfo;
	VFDirectoryGenerator *pGen;
	VFStore *pStore;

	/// Add virtual folders.
	TCHAR buf[MAX_PATH + 1];
	TCHAR buf2[MAX_PATH + 1];
	GetModuleFileName(NULL, buf, MAX_PATH);
	GetFilePath(buf2, buf);

	TString sVFpath;
	if (!sVFpath.Join(buf2, TOMBO_VFOLDER_DEF_FILE)) return FALSE;

	TSParser tp;
	tp.Parse(sVFpath.Get(), pListener);

	// Enum temporary vfolders
	DWORD n = vbInfo.NumItems();
	for (DWORD i = 0; i < n; i++) {
		pInfo = vbInfo.GetUnit(i);
		if (!StreamObjectsFactory(pInfo, &pGen, &pStore)) return FALSE;
		if (!pListener->ProcessStream(pInfo->pName, FALSE, pGen, pStore)) break;
	}
	return TRUE;
}

BOOL VFManager::RetrieveInfo(const VFInfo *pInfo, VirtualFolderEnumListener *pListener)
{
	VFDirectoryGenerator *pGen;
	VFStore *pStore;
	if (!StreamObjectsFactory(pInfo, &pGen, &pStore)) return FALSE;
	pListener->ProcessStream(pInfo->pName, pInfo->bPersist, pGen, pStore);
	return TRUE;
}

/////////////////////////////////////////////
//  Update vfolders
/////////////////////////////////////////////

BOOL VFManager::UpdateVirtualFolders(VFInfo **ppInfo, DWORD nNumFolders)
{
	// Create tmp file
	TCHAR buf[MAX_PATH + 1];
	TCHAR buf2[MAX_PATH + 1];
	GetModuleFileName(NULL, buf, MAX_PATH);
	GetFilePath(buf2, buf);

	TString sVFpathTmp, sVFpath;
	if (!sVFpath.Join(buf2, TOMBO_VFOLDER_DEF_FILE)) return FALSE;
	if (!sVFpathTmp.Join(buf2, TOMBO_VFOLDER_DEF_FILE, TEXT(".tmp"))) return FALSE;
	File fNewVFDef;
	if (!fNewVFDef.Open(sVFpathTmp.Get(), GENERIC_WRITE, FILE_SHARE_WRITE, CREATE_ALWAYS)) return FALSE;

	// prepare headers
	LPCWSTR pHeader = 
		L"<?xml version='1.0' encoding='UTF-16'?>\n"
		L"<!DOCTYPE folders SYSTEM \"TOMBO_vfolder.dtd\">\n"
		L"<folders>\n";

	if (!fNewVFDef.WriteUnicodeString(pHeader)) return FALSE;

	ClearInfo();

	// save virtual folder defs
	DWORD i;
	VFInfo *p;
	for (i = 0; i < nNumFolders; i++) {
		p = ppInfo[i];
		if (p->bPersist) {
			if (!p->WriteXML(&fNewVFDef)) return FALSE;
		} else {
			vbInfo.Add(p);
			p->pGenerator = NULL;
			p->pName = NULL;
		}
	}

	// output footer
	if (!fNewVFDef.WriteUnicodeString(L"</folders>\n")) return FALSE;
	fNewVFDef.Close();

	// replace def file
	DeleteFile(sVFpath.Get());
	MoveFile(sVFpathTmp.Get(), sVFpath.Get());
	return TRUE;
}

/////////////////////////////////////////////
//  Assign name to node
/////////////////////////////////////////////

LPTSTR VFManager::GetNodeName()
{
	LPTSTR pName;
	pName = new TCHAR[_tcslen(MSG_GREP_NONAME_LABEL) + 10];
	if (pName == NULL) {
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	wsprintf(pName, TEXT("%s%03d"), MSG_GREP_NONAME_LABEL, ++nGrepCount);
	return pName;
}

/////////////////////////////////////////////
//  VirtualFolderEnumListener implimentation
/////////////////////////////////////////////

VirtualFolderEnumListener::~VirtualFolderEnumListener()
{
}

/////////////////////////////////////////////
//  VFInfo implimentation
/////////////////////////////////////////////

void VFInfo::Release()
{
	 delete[] pName;
	 if(pGenerator) pGenerator->FreeObject();
}

/////////////////////////////////////////////
//  Output XML
/////////////////////////////////////////////

BOOL VFInfo::WriteXML(File *p)
{
	if (pName == NULL) {
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}

	LPWSTR pNameW = ConvTCharToWChar(pName);

	if (!p->WriteUnicodeString(L"<vfolder name=\"") || 
		!p->Write((LPBYTE)pNameW, wcslen(pNameW) * sizeof(WCHAR)) ||
		!p->WriteUnicodeString(L"\">\n")) {
		delete [] pNameW;
		return FALSE;
	}
	delete [] pNameW;

	if (!pGenerator->GenerateXMLOpenTag(p)) return FALSE;
	if (!pGenerator->GenerateXMLCloseTag(p)) return FALSE;

	if (!p->WriteUnicodeString(L"</vfolder>\n")) return FALSE;
	return TRUE;
}

/////////////////////////////////////////////
//  Clone VFInfo
/////////////////////////////////////////////

VFInfo *VFInfo::Clone()
{
	VFInfo *pNew = new VFInfo();
	if (pNew == NULL) return NULL;

	pNew->pGenerator = (VFDirectoryGenerator*)pGenerator->Clone(&(pNew->pStore));
	if (pNew->pGenerator == NULL) return NULL;

	pNew->bPersist = bPersist;
	pNew->pName = StringDup(pName);
	if (pNew->pName == NULL) {
		pNew->Release();
		delete pNew;
		return NULL;
	}
	return pNew;
}

