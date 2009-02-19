#include <windows.h>
#include "VarBuffer.h"
#include "StringSplitter.h"

/////////////////////////////////////////////////////////////////////////////
// Initialize
/////////////////////////////////////////////////////////////////////////////

BOOL StringSplitter::Init()
{
	return lines.Init(10, 10);
}

/////////////////////////////////////////////////////////////////////////////
// parse string and set to vector
/////////////////////////////////////////////////////////////////////////////

// Usually, EOL code is only one and it is designated by options but
// in current version, EOL code is not deterministic.
// all pattern is accepted below even though some code exists in other lines.
// CR, LF, CR+LF, CR+CR, LF+LF, LF+CR

// ex.  abc\rdef\nghi is delimited by 3 lines : [abc][def][ghi]

BOOL StringSplitter::Parse(LPCTSTR pStr)
{
	lines.Clear(FALSE);
	if (*pStr == TEXT('\0')) {
		Marker m;
		m.nBegin = m.nEnd = 0;
		return lines.Add(&m);
	}

	LPCTSTR p;
	LPCTSTR pFirstLine, pLastLine;
	pFirstLine = pLastLine = p = pStr;
	Marker m;

	while(*p) {

		if (*p == TEXT('\r') || *p == TEXT('\n')) {

			// add line info
			m.nBegin = pFirstLine - pStr;
			m.nEnd = pLastLine - pStr;
			if (!lines.Add(&m)) return FALSE;

			// skip if CRLF
			p++;
			if (*p == TEXT('\r') || *p == TEXT('\n')) {
				p++;
			}

			// prepare for next line
			pFirstLine = pLastLine = p;
			continue;
		}
		pLastLine = p = CharNext(p);
	}

	m.nBegin = pFirstLine - pStr;
	m.nEnd = pLastLine - pStr;
	if (!lines.Add(&m)) return FALSE;

	return TRUE;
}

