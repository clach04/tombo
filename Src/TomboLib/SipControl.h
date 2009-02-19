#ifndef SIPCONTROL_H
#define SIPCONTROL_H

class SipControl {
	RECT rSipRect;
	RECT rVisibleDesktop;
public:
	BOOL Init() { return TRUE; }

	BOOL GetSipStat(BOOL *pStatus);
	BOOL SetSipStat(BOOL bActive);
	RECT GetRect() { return rSipRect; }
	RECT GetVisibleDesktop() { return rVisibleDesktop; }
};


#endif