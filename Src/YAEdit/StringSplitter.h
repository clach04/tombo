#ifndef LINESPLITTER_H
#define LINESPLITTER_H

class StringSplitter {
	struct Marker {
		DWORD nBegin;
		DWORD nEnd;
	};

	TVector<Marker> lines;
public:
	StringSplitter() {}
	~StringSplitter() {}

	BOOL Init();

	BOOL Parse(LPCTSTR pStr);

	DWORD MaxLine() { return lines.NumItems(); }
	DWORD GetBegin(DWORD i) { return lines.GetUnit(i)->nBegin; }
	DWORD GetEnd(DWORD i) { return lines.GetUnit(i)->nEnd; }

	friend class StringSplitterTest;
};

#endif
