#ifndef REGEXUTIL_H
#define REGEXUTIL_H

#ifdef __cplusplus
extern "C" {
#endif

void* Regex_Compile(const LPBYTE pPattern, BOOL bIgnoreCase, const char **ppReason, DWORD nCodePage);
void Regex_Free(void *p);
int Regex_Search(void *p, int iStart, const LPBYTE pTarget, BOOL bForward, int *pStart, int *pEnd, DWORD nCodePage);

const LPBYTE ShiftLeft(const LPBYTE pString, const LPBYTE pPos, DWORD nCodePage);
const LPBYTE ShiftRight(const LPBYTE pString, const LPBYTE pPos, DWORD nCodePage);

DWORD ConvertPos(const LPBYTE pSrcStr, DWORD nSrcPos, DWORD nSrcEnc, const LPBYTE pDstStr, DWORD nDstEnc);

#ifdef __cplusplus
};
#endif

#endif

