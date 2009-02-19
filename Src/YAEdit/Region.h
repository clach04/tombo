#ifndef REGION_H
#define REGION_H

class Coordinate {
public:
	DWORD col;
	DWORD row;

	Coordinate() : col(0), row(0) {}
	Coordinate(DWORD c, DWORD r) : col(c), row(r) {}
	void Set(DWORD c, DWORD r) { col = c; row = r; }
};

inline int operator==(const Coordinate &pos1, const Coordinate &pos2) { return pos1.col == pos2.col && pos1.row == pos2.row; }
inline int operator!=(const Coordinate &pos1, const Coordinate &pos2) { return !(pos1 == pos2); }
inline int operator<(const Coordinate &pos1, const Coordinate &pos2) { return pos1.row < pos2.row || (pos1.row == pos2.row && pos1.col < pos2.col); }
inline int operator>(const Coordinate &pos1, const Coordinate &pos2) { return pos2 < pos1; }

class Region {
public:
	enum { COL_EOL = 0xFFFFFFFF, ROW_MAX = 0xFFFFFFFF };

	Coordinate posStart;
	Coordinate posEnd;

	Region() { posStart.Set(0, 0); posEnd.Set(0, 0); }
	Region(const Region& r) { posStart = r.posStart; posEnd = r.posEnd; }
	Region(DWORD cs, DWORD rs, DWORD ce, DWORD re) { posStart.Set(cs, rs); posEnd.Set(ce, re); }

	BOOL IsEmptyRegion() const { return posStart == posEnd; }

};

inline int operator==(const Region &r1, const Region &r2) { return (r1.posStart == r2.posStart) && (r1.posEnd == r2.posEnd); }

#endif