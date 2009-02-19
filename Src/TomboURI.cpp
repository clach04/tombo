#include <windows.h>
#include <tchar.h>
#include "TString.h"
#include "TomboURI.h"

static LPCTSTR pURIPrefix = TEXT("tombo://");

/////////////////////////////////////////////
/////////////////////////////////////////////
// URI implimentation
/////////////////////////////////////////////
/////////////////////////////////////////////

/////////////////////////////////////////////
// ctor & dtor
/////////////////////////////////////////////

TomboURI::TomboURI()
{
}

TomboURI::TomboURI(const TomboURI &u)
{
	uri.Set(u.uri);
	nMaxPathItem = u.nMaxPathItem;
}

TomboURI::~TomboURI()
{
}

TomboURI &TomboURI::operator =(const TomboURI &val)
{
	uri.Set(val.uri);
	nMaxPathItem = val.nMaxPathItem;
	return *this;
}

/////////////////////////////////////////////
// initialize
/////////////////////////////////////////////

BOOL TomboURI::Init(LPCTSTR pURI)
{
	// check header string
	if (_tcsnicmp(pURI, pURIPrefix, 8) != 0) {
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}

	nMaxPathItem = 0;

	// count repository string length
	LPCTSTR p = pURI + 8;
	LPCTSTR q;
	DWORD nLv = 0;
	while (1) {
		q = GetNextSep(p);
		if (q == NULL) {
			int n = _tcslen(p);
			if (nMaxPathItem < n) nMaxPathItem = n;
			break;
		}
		if (nMaxPathItem < (q - p)) nMaxPathItem = q - p;
		p = q + 1;
		nLv++;
	}

	// check like  "tombo:///"
	if (nLv == 0) {
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
	}

	return uri.Set(pURI);
}

BOOL TomboURI::Init(const TomboURI &u)
{
	uri.Set(u.uri);
	nMaxPathItem = u.nMaxPathItem;
	return TRUE;
}
/////////////////////////////////////////////
// Yet another initializer
/////////////////////////////////////////////
// This function is obsoleted and will be removed in the future.

BOOL TomboURI::InitByNotePath(LPCTSTR pRepoName, LPCTSTR pNotePath)
{
	TString s;
	if (!s.Alloc(9 + _tcslen(pRepoName) + _tcslen(pNotePath) + 1)) return FALSE;
	wsprintf(s.Get(), TEXT("tombo://%s/"), pRepoName);
	LPTSTR p = s.Get() + 9 + _tcslen(pRepoName);

	LPCTSTR q = pNotePath;
	if (*q == TEXT('\\')) q++;

	while(*q) {
		if (*q == TEXT('\\')) {
			*p++ = TEXT('/');
			q++;
			continue;
		}
#if defined(PLATFORM_WIN32)
		if (IsDBCSLeadByte(*q)) {
			*p++ = *q++;
		}
#endif
		*p++ = *q++;
	}
	*p = TEXT('\0');
	return Init(s.Get());
}

/////////////////////////////////////////////
// skip to next separator
/////////////////////////////////////////////

LPCTSTR TomboURI::GetNextSep(LPCTSTR pPartPath)
{
	LPCTSTR p = pPartPath;
	while (*p) {
		if (*p == TEXT('/')) return p;
		p = CharNext(p);
	}
	return NULL;
}

/////////////////////////////////////////////
// get repository name
/////////////////////////////////////////////

BOOL TomboURI::GetRepositoryName(TString *pRepo) const
{
	LPCTSTR p = uri.Get() + 8;
	LPCTSTR q = GetNextSep(p);

	if (!pRepo->Alloc(q - p + 1)) return FALSE;
	_tcsncpy(pRepo->Get(), p, q - p);
	*(pRepo->Get() + (q - p)) = TEXT('\0');

	return TRUE;
}

/////////////////////////////////////////////
// get path part
/////////////////////////////////////////////

LPCTSTR TomboURI::GetPath() const
{
	LPCTSTR p = uri.Get() + _tcslen(pURIPrefix);

	// skip repository part
	p = _tcschr(p, TEXT('/'));
	return p;
}

/////////////////////////////////////////////
// Is the URI point to leaf node?
/////////////////////////////////////////////

BOOL TomboURI::IsLeaf() const
{
	if (_tcslen(GetPath() + 1) == 0) return FALSE; // root
	LPCTSTR p = uri.Get();
	return *(p + _tcslen(p) - 1) != TEXT('/');
}

/////////////////////////////////////////////
// Root node?
/////////////////////////////////////////////

BOOL TomboURI::IsRoot() const
{
	LPCTSTR p = _tcschr(uri.Get() + _tcslen(TEXT("tombo://")), TEXT('/'));
	if (p == NULL) return FALSE; // invalid URI
	if (*(p+1) == TEXT('\0')) return TRUE;
	return FALSE;
}

/////////////////////////////////////////////
// get parent path
/////////////////////////////////////////////

BOOL TomboURI::GetParent(TomboURI *pParent) const
{
	LPCTSTR p = GetPath();
	LPCTSTR pBase = p;

	if (*p) p++;

	LPCTSTR q = NULL;
	while (*p) {
		if (*p == TEXT('/') && *(p+1) != TEXT('\0')) {
			q = p;
		}
		p = CharNext(p);
	}

	TString s;
	if (q == NULL) {
		// result is root node.
		DWORD n = GetPath() - uri.Get() + 1;
		if (!s.Alloc(n + 1)) return FALSE;
		_tcsncpy(s.Get(), uri.Get(), n);
		*(s.Get() + n) = TEXT('\0');

	} else {
		if (!s.Alloc(q - uri.Get() + 2)) return FALSE;
		_tcsncpy(s.Get(), uri.Get(), q - uri.Get());
		*(s.Get() + (q - uri.Get())) = TEXT('\0');

		_tcscat(s.Get(), TEXT("/"));
	}
	if (!pParent->Init(s.Get())) return FALSE;

	return TRUE;
}

/////////////////////////////////////////////
// get folder to attach new note
/////////////////////////////////////////////

BOOL TomboURI::GetAttachFolder(TomboURI *pAttach) const
{
	if (IsLeaf()) {
		if (!GetParent(pAttach)) return FALSE;
	} else {
		*pAttach = *this;
	}
	return TRUE;
}

/////////////////////////////////////////////
// get parent path
/////////////////////////////////////////////

BOOL TomboURI::GetFilePath(TString *pPath) const
{
	LPCTSTR p = GetPath();
	if (*p) *p++;

	if (!pPath->Alloc(_tcslen(p) + 1)) return FALSE;
	_tcscpy(pPath->Get(), p);

	LPTSTR q = pPath->Get();
	while(q = _tcschr(q, TEXT('/'))) {
		*q = TEXT('\\');
	}
	return TRUE;
}

/////////////////////////////////////////////
// get base name
/////////////////////////////////////////////

BOOL TomboURI::GetBaseName(TString *pBase) const
{
	LPCTSTR p = GetPath();
	LPCTSTR q = NULL;
	while(*p) {
		if (*p == TEXT('/') && *(p+1) != TEXT('\0')) {
			q = p;
			p++;
			continue;
		}
#if defined(PLATFORM_WIN32)
		p = CharNext(p);
#else
		p++;
#endif
	}

	if (q == NULL) {
		// if url point to root
		if (!pBase->Set(TEXT(""))) return FALSE;
		return TRUE;
	}
	if (!pBase->Set(q + 1)) return FALSE;
	if (*(q + _tcslen(q) - 1) == TEXT('/')) {
		*(pBase->Get() + _tcslen(pBase->Get()) - 1) = TEXT('\0');
	}
	return TRUE;
}

/////////////////////////////////////////////
/////////////////////////////////////////////
// iterator implimentation
/////////////////////////////////////////////
/////////////////////////////////////////////

BOOL TomboURIItemIterator::Init()
{
	// reallocate buffer
	if (pBuf) delete[] pBuf;
	pBuf = new TCHAR[pURI->GetMaxPathItem() + 1];
	if (pBuf == NULL) return FALSE;

	return TRUE;
}

void TomboURIItemIterator::First()
{
	// seek head of path
	LPCTSTR p = TomboURI::GetNextSep(pURI->GetFullURI() + 8);
	p++;
	nPos = p - pURI->GetFullURI();

	Next();
}

LPCTSTR TomboURIItemIterator::Current()
{
	if (*pBuf == TEXT('\0')) return NULL;
	return pBuf;
}

void TomboURIItemIterator::Next()
{
	LPCTSTR p = pURI->GetFullURI() + nPos;

	LPTSTR q = pBuf;
	while (*p && *p != TEXT('/')) {
#if defined(PLATFORM_WIN32)
		if (IsDBCSLeadByte(*p)) {
			*q++ = *p++;
		}
#endif
		*q++ = *p++;
	}
	*q = TEXT('\0');
	if (*p == TEXT('/')) p++;

	nPos = p - pURI->GetFullURI();
}

BOOL TomboURIItemIterator::IsLeaf()
{
	LPCTSTR p = pURI->GetFullURI() + nPos;
	return (*p == TEXT('\0')) && (*(p-1) != TEXT('/'));
}