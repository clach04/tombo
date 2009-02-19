#include <windows.h>
#include <tchar.h>

#include "Tombo.h"
#include "message.h"
#include "TString.h"
#include "Uniconv.h"
#include "TomboURI.h"
#include "Repository.h"
#include "Property.h"

#include "AutoPtr.h"

#include "RepositoryImpl.h"

// At this time, Repository is only proxy to RepositoryImpl.
// This class will choose repository implementations in future.

/////////////////////////////////////////
// Global definition
/////////////////////////////////////////

Repository g_Repository;

/////////////////////////////////////////
// Repository ctor & dtor, initializer
/////////////////////////////////////////

Repository::Repository() : pDefaultImpl(NULL)
{
}

Repository::~Repository()
{
}

BOOL Repository::Init()
{
	if (!vSubRepository.Init(10, 10)) return FALSE;
	return ClearSubRepository();
}

BOOL Repository::ClearSubRepository()
{
	for (DWORD i = 0; i < vSubRepository.NumItems(); i++) {
		RepositoryImpl *p = *vSubRepository.GetUnit(i);
		delete p;
	}
	vSubRepository.Clear(FALSE);

	pDefaultImpl = NULL;
	return TRUE;
}

BOOL Repository::AddSubRepository(RepositoryImpl *pImpl)
{
	if (!vSubRepository.Add(&pImpl)) return FALSE;

	pDefaultImpl = *vSubRepository.GetUnit(0);
	return TRUE;
}

BOOL Repository::GetAttachURI(const TomboURI *pBase, TomboURI *pAttached)
{
	if (pBase->IsLeaf()) {
		if (!pBase->GetParent(pAttached)) return FALSE;
	} else {
		if (!pAttached->Init(pBase->GetFullURI())) return FALSE;
	}
	return TRUE;
}

RepositoryImpl *Repository::GetAssocSubRepository(const TomboURI *pURI)
{
	DWORD n = vSubRepository.NumItems();
	for (DWORD i = 0; i < n; i++) {
		RepositoryImpl *pImpl = *vSubRepository.GetUnit(i);
		if (_tcsncmp(pImpl->GetRootURI()->GetFullURI() + 8, pURI->GetFullURI() + 8, pImpl->GetRepNameLen() + 1) == 0) {
			return pImpl;
		}
	}
	return NULL;
}

/////////////////////////////////////////
// delegated methods
/////////////////////////////////////////

BOOL Repository::Update(const TomboURI *pCurrentURI, 
						LPCTSTR pData, 
						TomboURI *pNewURI, TString *pNewHeadLine)
{
	// pNewURI is in same sub repository
	RepositoryImpl *pImpl = GetAssocSubRepository(pCurrentURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->Update(pCurrentURI, pData, pNewURI, pNewHeadLine);
}

BOOL Repository::GetHeadLine(const TomboURI *pURI, TString *pHeadLine)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->GetHeadLine(pURI, pHeadLine);
}

BOOL Repository::GetOption(const TomboURI *pURI, URIOption *pOption)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->GetOption(pURI, pOption);
}

BOOL Repository::SetOption(const TomboURI *pCurrentURI, URIOption *pOption)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pCurrentURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->SetOption(pCurrentURI, pOption);
}

BOOL Repository::IsEncrypted(const TomboURI *pURI)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->IsEncrypted(pURI);
}

BOOL Repository::Delete(const TomboURI *pURI, URIOption *pOption)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->Delete(pURI, pOption);
}

BOOL Repository::Copy(const TomboURI *pCopyFrom, const TomboURI *pCopyTo, URIOption *pOption)
{
	// XXXX over subrepository is not supported yet.
	RepositoryImpl *pImpl = GetAssocSubRepository(pCopyFrom);
	if (pImpl == NULL) return FALSE;
	return pImpl->Copy(pCopyFrom, pCopyTo, pOption);
}

BOOL Repository::Move(const TomboURI *pMoveFrom, const TomboURI *pMoveTo, URIOption *pOption)
{
	// XXXX over subrepository is not supported yet.
	RepositoryImpl *pImpl = GetAssocSubRepository(pMoveFrom);
	if (pImpl == NULL) return FALSE;
	return pImpl->Move(pMoveFrom, pMoveTo, pOption);
}

BOOL Repository::GetPhysicalPath(const TomboURI *pURI, TString *pFullPath)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->GetPhysicalPath(pURI, pFullPath);
}

BOOL Repository::ChangeHeadLine(const TomboURI *pURI, LPCTSTR pReqNewHeadLine, URIOption *pOption)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->ChangeHeadLine(pURI, pReqNewHeadLine, pOption);
}


URIList *Repository::GetChild(const TomboURI *pFolder, BOOL bSkipEncrypt, BOOL bLooseDecrypt, BOOL *pLoose)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pFolder);
	if (pImpl == NULL) return FALSE;
	return pImpl->GetChild(pFolder, bSkipEncrypt, bLooseDecrypt, pLoose);
}

BOOL Repository::RequestAllocateURI(const TomboURI *pBaseURI, LPCTSTR pText, TString *pHeadLine, TomboURI *pURI, const TomboURI *pTemplateURI)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pBaseURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->RequestAllocateURI(pBaseURI, pText, pHeadLine, pURI, pTemplateURI);
}

BOOL Repository::GetAttribute(const TomboURI *pURI, NoteAttribute *pAttribute)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->GetAttribute(pURI, pAttribute);
}

BOOL Repository::GetNoteAttribute(const TomboURI *pURI, UINT64 *pLastUpdate, UINT64 *pCreateDate, UINT64 *pFileSize)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->GetNoteAttribute(pURI, pLastUpdate, pCreateDate, pFileSize);
}

BOOL Repository::SetAttribute(const TomboURI *pURI, const NoteAttribute *pAttribute)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->SetAttribute(pURI, pAttribute);
}

LPTSTR Repository::GetNoteData(const TomboURI *pURI)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->GetNoteData(pURI);
}

LPBYTE Repository::GetNoteDataNative(const TomboURI *pURI, LPDWORD pSize)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->GetNoteDataNative(pURI, pSize);
}

BOOL Repository::ExecuteAssoc(const TomboURI *pURI, ExeAppType nType)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pURI);
	if (pImpl == NULL) return FALSE;
	return pImpl->ExecuteAssoc(pURI, nType);
}

BOOL Repository::MakeFolder(const TomboURI *pParent, LPCTSTR pFolderName)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pParent);
	if (pImpl == NULL) return FALSE;
	return pImpl->MakeFolder(pParent, pFolderName);
}

BOOL Repository::GetFileName(const TomboURI *pURI, TString *pName)
{
	RepositoryImpl *pImpl = GetAssocSubRepository(pURI);
	if (pImpl == NULL) return FALSE;
	return pURI->GetBaseName(pName);
}

/////////////////////////////////////////
// sub repository IF
/////////////////////////////////////////

DWORD Repository::GetNumOfSubRepository()
{
	return vSubRepository.NumItems();
}

DWORD Repository::GetSubRepositoryType(DWORD nIndex)
{
	RepositoryImpl *pImpl = *vSubRepository.GetUnit(nIndex);
	if (pImpl == NULL) return NULL;

	return pImpl->GetRepositoryType();
}

LPCTSTR Repository::GetSubRepositoryName(DWORD nIndex)
{
	RepositoryImpl *pImpl = *vSubRepository.GetUnit(nIndex);
	if (pImpl == NULL) return NULL;

	return pImpl->GetDisplayName();
}

const TomboURI *Repository::GetSubRepositoryRootURI(DWORD nIndex)
{
	RepositoryImpl *pImpl = *vSubRepository.GetUnit(nIndex);
	if (pImpl == NULL) return NULL;
	return pImpl->GetRootURI();
}

LPTSTR Repository::GetSubRepoXMLSaveString(DWORD nIndex)
{
	RepositoryImpl *pImpl = *vSubRepository.GetUnit(nIndex);
	if (pImpl == NULL) return NULL;
	return pImpl->GetXMLSaveString();
}

// factory methods for RepositoryImpl
RepositoryImpl *Repository::CreateSubRepo(LPCWSTR pName, const WCHAR **atts)
{
	if (wcscmp(pName, L"localfile") == 0) {
		DWORD i = 0;
		LPTSTR pDispName = NULL;
		LPTSTR pRepName = NULL;
		LPTSTR pTopDir = NULL;
		BOOL bKeepTitle = FALSE;
		BOOL bKeepCaret = FALSE;
		BOOL bSafeFileName = FALSE;

		while (atts[i] != L'\0') {
			if (wcscmp(atts[i], L"name") == 0) {
				pRepName = ConvWCharToTChar(atts[i + 1]);
			} else if (wcscmp(atts[i], L"dispname") == 0) {
				pDispName = ConvWCharToTChar(atts[i + 1]);
			} else if (wcscmp(atts[i], L"path") == 0) {
				pTopDir = ConvWCharToTChar(atts[i + 1]);
			} else if (wcscmp(atts[i], L"keeptitle") == 0) {
				bKeepTitle = (wcscmp(atts[i + 1], L"1") == 0);
			} else if (wcscmp(atts[i], L"keepcaret") == 0) {
				bKeepCaret = (wcscmp(atts[i + 1], L"1") == 0);
			} else if (wcscmp(atts[i], L"safefilename") == 0) {
				bSafeFileName = (wcscmp(atts[i + 1], L"1") == 0);
			}
			i+= 2;
		}
		ArrayAutoPointer<TCHAR> ap1(pDispName), ap2(pRepName), ap3(pTopDir);

		if (pRepName == NULL || pDispName == NULL || pTopDir == NULL) {
			SetLastError(ERROR_INVALID_PARAMETER);
			return NULL;
		}

		LocalFileRepository *pImpl = new LocalFileRepository();
		if (pImpl == NULL) {
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return NULL;
		}
		if (!pImpl->Init(pRepName, pDispName, pTopDir, bKeepTitle, bKeepCaret, bSafeFileName)) {
			delete[] pImpl;
			return NULL;
		}
		return pImpl;
	} else if (wcscmp(pName, L"vfolder") == 0) {
		LPTSTR pDispName = NULL;
		LPTSTR pRepName = NULL;
		DWORD i = 0;
		while (atts[i] != L'\0') {
			if (wcscmp(atts[i], L"name") == 0) {
				pRepName = ConvWCharToTChar(atts[i + 1]);
			} else if (wcscmp(atts[i], L"dispname") == 0) {
				pDispName = ConvWCharToTChar(atts[i + 1]);
			}
			i += 2;
		}
		ArrayAutoPointer<TCHAR> ap1(pDispName), ap2(pRepName);

		VFolderRepository *pImpl = new VFolderRepository();
		if (pImpl == NULL) {
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return NULL;
		}
		if (!pImpl->Init(pRepName, pDispName)) {
			delete[] pImpl;
			return NULL;
		}
		return pImpl;
	} else {
		return NULL;
	}
}
