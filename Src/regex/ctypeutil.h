#ifndef _CTYPEUTIL_H
#define _CTYPEUTIL_H

// MBCSî≈ctypeå›ä∑ÉãÅ[É`Éì
// CE2.11Ç…ÇÕë∂ç›ÇµÇƒÇ¢Ç»Ç¢ÅBç¢Ç¡ÇΩÇ‡ÇÃÇæ...

#define CTUTIL_PRINT  1
#define CTUTIL_DIGIT  (CTUTIL_PRINT << 1)
#define CTUTIL_CNTRL  (CTUTIL_PRINT << 2)
#define CTUTIL_LOWER  (CTUTIL_PRINT << 3)
#define CTUTIL_PUNCT  (CTUTIL_PRINT << 4)
#define CTUTIL_SPACE  (CTUTIL_PRINT << 5)
#define CTUTIL_UPPER  (CTUTIL_PRINT << 6)
#define CTUTIL_XDIGIT (CTUTIL_PRINT << 7)

int isctype(int c, int t);
#define isalnum(c)	(isctype(c, CTUTIL_UPPER | CTUTIL_LOWER | CTUTIL_DIGIT))
#define isspace(c)	(isctype(c, CTUTIL_SPACE))
#define isdigit(c)	(isctype(c, CTUTIL_DIGIT))
#define isalpha(c)	(isctype(c, CTUTIL_UPPER | CTUTIL_LOWER))
#define iscntrl(c)	(isctype(c, CTUTIL_CNTRL))
#define isprint(c)	(isctype(c, CTUTIL_PRINT))
#define islower(c)	(isctype(c, CTUTIL_LOWER))
#define ispunct(c)	(isctype(c, CTUTIL_PUNCT))
#define isupper(c)	(isctype(c, CTUTIL_UPPER))
#define isxdigit(c)	(isctype(c, CTUTIL_XDIGIT))

#endif