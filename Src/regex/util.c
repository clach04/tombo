#include <windows.h>
#include <string.h>

#include "oniguruma.h"
#include "RegexUtil.h"

static char err_buf[ONIG_MAX_ERROR_MESSAGE_LEN];

// regex library wrapper

static OnigEncoding GetNativeEncoding(DWORD nCodePage)
{
	OnigEncoding enc;
	UINT acp = GetACP();

	switch (nCodePage) {
	case 1200:	// UTF-16LE
		return ONIG_ENCODING_UTF16_LE;
	case 65001:	// UTF-8
		return ONIG_ENCODING_UTF8;
	}

	// if not match, use system default

	switch(acp) {
	case 932:	// Japan
		enc = ONIG_ENCODING_SJIS;
		break;
//	case 936:	// Chinese (PRC, Singapore)
//		break;
//	case 949:	// Korean
//		break;
//	case 950:	// Chinese (Taiwan; Hong Kong SAR, PRC) 
//		break;
//	case 1250:	// Eastern European 
//		break;
	case 1251:	// Cyrillic
		enc = ONIG_ENCODING_ISO_8859_5;
		break;
	case 1252:	// Latin 1
		enc = ONIG_ENCODING_ISO_8859_1;
		break;
	case 1253:	// Greek
		enc = ONIG_ENCODING_ISO_8859_7;
		break;
	default:	// default = ASCII
		enc = ONIG_ENCODING_ASCII;
	}
	return enc;
}

//////////////////////////////////////////////////////////
// Prepare and compile regex pattern
//////////////////////////////////////////////////////////
// pPattern		: expression pattern
//                  In CE version, pPattern's encoding is UCS2. 
// bIgnoreCase	: is case insensitive?
// ppReason 	: In current, ignored.
// 
// result 		: if success, returns pointer to pattern which is compiled.
//				  if fail, return NULL

void* Regex_Compile(const LPBYTE pPattern, BOOL bIgnoreCase, const char **ppReason, DWORD nCodePage)
{
	regex_t *reg;
	OnigErrorInfo einfo;

	UChar *pattern = (UChar*)pPattern;
	OnigEncoding enc;
	OnigOptionType option;
	int len;
	int r;

	if (bIgnoreCase) {
		option = ONIG_OPTION_IGNORECASE;
	} else {
		option = ONIG_OPTION_NONE;
	}

	enc = GetNativeEncoding(nCodePage);

	len = onigenc_str_bytelen_null(enc, pattern);

	r = onig_new(&reg, pattern, 
					 pattern + len,
					 option, enc, ONIG_SYNTAX_DEFAULT, &einfo);
	if (r != ONIG_NORMAL) {
		onig_error_code_to_str(err_buf, r, einfo);
		*ppReason = err_buf;
		return NULL;
	}
	return reg;
}

//////////////////////////////////////////////////////////
// free regex pattern
//////////////////////////////////////////////////////////
//
// p  : the pointer that Regex_Compile returned.

void Regex_Free(void *p)
{
	if (p == NULL) return;
	onig_free((regex_t*)p);
}

//////////////////////////////////////////////////////////
// execute matching
//////////////////////////////////////////////////////////
// [IN]
// p		: the pointer that Regex_Compile returned.
// iStart	: start position(bytes) on pTarget
// pTarget	: target string
// bForward : TRUE if search to forward
//            FALSE if search to backword
//
// [OUT] 
// pStart	:  ptr to start char of region that is matched.
// pEnd		:  ptr to end char of region that is matched.
//
// [return]
//	>=0 : position to start char of region that is matched.
//  -1  : not matched.
//  -2  : error

int Regex_Search(void *p, int iStart, const LPBYTE pTarget, BOOL bForward, int *pStart, int *pEnd, DWORD nCodePage)
{
	regex_t *reg = (regex_t*)p;
	unsigned char *str;
	unsigned char *end;
	unsigned char *start, *range;
	int r;
	int result;
	OnigEncoding enc;

	OnigRegion *region;

	enc = GetNativeEncoding(nCodePage);
	str = (unsigned char*)pTarget;
	end = str + onigenc_str_bytelen_null(enc, str);

	if (bForward) {
		start = str + iStart;
		range = end;
	} else {
		start = str + iStart;
		range = str;
	}
	region = onig_region_new();
	
	r = onig_search(reg, str, end, start, range, region, ONIG_OPTION_NONE);
	if (r >= 0) {
		// match
		if (region->num_regs == 0) return -2;
		if (pStart) *pStart = region->beg[0];
		if (pEnd) *pEnd = region->end[0];
		result = r;
	} else if (r == ONIG_MISMATCH) {
		// mismatch
		result = -1;
	} else {
		// error
		result = -2;
	}
	onig_region_free(region, 1);
	return result;
}

//////////////////////////////////////////////////////////
// convert an encoding position to another position
//////////////////////////////////////////////////////////

DWORD ConvertPos(const LPBYTE pSrcStr, DWORD nSrcPos, DWORD nSrcEnc, const LPBYTE pDstStr, DWORD nDstEnc)
{
	OnigEncoding srcEnc = GetNativeEncoding(nSrcEnc);
	OnigEncoding dstEnc = GetNativeEncoding(nDstEnc);

	DWORD i = 0;
	const UChar* p = pSrcStr;
	const UChar* q = pDstStr;

	DWORD nSrc, nDst;

	nSrc = nDst = 0;
	while (*p) {
		nSrc = p - pSrcStr;
		nDst = q - pDstStr; 

		if (nSrc >= nSrcPos) break;

		p = onigenc_get_right_adjust_char_head(srcEnc, (LPBYTE)pSrcStr, p + 1);
		q = onigenc_get_right_adjust_char_head(dstEnc, (LPBYTE)pDstStr, q + 1);
	}
	if (*p == 0) {
		nDst = q - pDstStr;
	}
	return nDst;
}

//////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////

const LPBYTE ShiftLeft(const LPBYTE pString, const LPBYTE pPos, DWORD nCodePage)
{
	OnigEncoding enc = GetNativeEncoding(nCodePage);

	LPBYTE p = pPos - 1;
	LPBYTE q;
	while (p >= pString) {
		q = onigenc_get_right_adjust_char_head(enc, pString, p);
		if (q < pPos) break;
		p--;
	}
	return q;
}

const LPBYTE ShiftRight(const LPBYTE pString, const LPBYTE pPos, DWORD nCodePage)
{
	OnigEncoding enc = GetNativeEncoding(nCodePage);
	return (LPBYTE)onigenc_get_right_adjust_char_head(enc, pString, pPos + 1);
}

