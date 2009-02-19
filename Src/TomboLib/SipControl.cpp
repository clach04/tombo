#include <windows.h>
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_WM5)
#include <aygshell.h>
#endif
#if defined(PLATFORM_BE500)
#include <sip.h>
#endif
#include "Tombo.h"
#include "SipControl.h"

BOOL SipControl::GetSipStat(BOOL *pStatus)
{
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500) || defined(PLATFORM_WM5)
	SIPINFO sip;
	memset(&sip, 0, sizeof(sip));
	sip.cbSize = sizeof(sip);
	sip.dwImDataSize = 0;

#if defined(PLATFORM_BE500)
	if (!SipGetInfo(&sip)) return FALSE;
#else
	if (!SHSipInfo(SPI_GETSIPINFO, 0, &sip, 0)) return FALSE;
#endif
	*pStatus = (sip.fdwFlags & SIPF_ON) != 0;
	rSipRect = sip.rcSipRect;
	rVisibleDesktop = sip.rcVisibleDesktop;
#endif
	return TRUE;
}

BOOL SipControl::SetSipStat(BOOL bActive)
{
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_PSPC) || defined(PLATFORM_BE500) || defined(PLATFORM_WM5)
	SIPINFO sip;
	memset(&sip, 0, sizeof(sip));
	sip.cbSize = sizeof(sip);

#if defined(PLATFORM_BE500)
	if (!SipGetInfo(&sip)) return FALSE;
#else
	if (!SHSipInfo(SPI_GETSIPINFO, 0, &sip, 0)) return FALSE;
#endif
	if ((bActive && (sip.fdwFlags & SIPF_ON)) ||
		(!bActive && !(sip.fdwFlags & SIPF_ON))) {
		return TRUE;
	}

	if (bActive) {
		sip.fdwFlags |= SIPF_ON;
	} else {
		sip.fdwFlags &= ~SIPF_ON;
	}
#if defined(PLATFORM_BE500)
	if (!SipSetInfo(&sip)) return FALSE;
#else
	if (!SHSipInfo(SPI_SETSIPINFO, 0, &sip, 0)) return FALSE;
#endif
#endif
	return TRUE;
}