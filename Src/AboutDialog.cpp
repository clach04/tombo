#include <windows.h>
#include "Tombo.h"

#ifdef _WIN32_WCE
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
#include <aygshell.h>
#endif
#endif

#include "resource.h"
#include "AboutDialog.h"

// version info

//#define TOMBO_VERSION_INFO TEXT("Tombo version 1.17")
#define TOMBO_VERSION_INFO TEXT("Tombo 2.0 beta 5")

// copyrights and credits

LPCTSTR pCopyright = 
TOMBO_VERSION_INFO
TEXT("\r\n")
TEXT("  Copyright(C) 2000-2003,2007-2012 Tomohisa Hirami\r\n")
TEXT("  Copyright(C) 2004-2006 TOMBO maintainers\r\n")
TEXT("\r\n")
TEXT("MD5 library:\r\n")
TEXT("  Copyright (C) 1995,1996,1998,1999 Free Software Foundation, Inc.\r\n")
TEXT("\r\n")
TEXT("BLOWFISH encryption library:\r\n")
TEXT("  Copyright (C) 1998 Free Software Foundation, Inc.\r\n")
TEXT("\r\n")
TEXT("Oniguruma(regular expression matching and search library):\r\n")
TEXT("  Copyright (c) 2002-2005  K.Kosako  <sndgk393 AT ybb DOT ne DOT jp>\r\n")
TEXT("  All rights reserved.\r\n")
TEXT("  ** Oniguruma is distributed under BSD License. ** \r\n")
TEXT("\r\n")
TEXT("The Expat XML Parser:\r\n")
TEXT("  Copyright (c) 1998, 1999, 2000 Thai Open Source Software Center Ltd\r\n")
TEXT("                               and Clark Cooper\r\n")
TEXT("  Copyright (c) 2001, 2002 Expat maintainers.\r\n")
TEXT("  ** Expat library is distributed under MIT License. ** \r\n")
TEXT("--------\r\n")
TEXT("TOMBO is free software; you can redistribute it and/or modify\r\n")
TEXT("it under the terms of the GNU General Public License as published by\r\n")
TEXT("the Free Software Foundation; either version 2 of the License, or\r\n")
TEXT(" any later version.\r\n")
TEXT("\r\n")
TEXT("TOMBO is distributed in the hope that it will be useful,\r\n")
TEXT("but WITHOUT ANY WARRANTY; without even the implied warranty of\r\n")
TEXT("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\r\n")
TEXT("GNU General Public License for more details.\r\n")
TEXT("\r\n")
TEXT("You should have received a copy of the GNU General Public License\r\n")
TEXT("along with this program; if not, write to the Free Software\r\n")
TEXT("Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA\r\n")
TEXT("\r\n")
TEXT("--------\r\n")
TEXT("\r\n")
TEXT("TOMBO\t: http://homepage2.nifty.com/thirami\r\n")
TEXT("GnuPG\t: http://www.gnupg.org/\r\n")
TEXT("Oniguruma: http://www.geocities.jp/kosako3/oniguruma/\r\n")
TEXT("Expat\t: http://www.libexpat.org/\r\n")
TEXT("\r\n")
TEXT("--------\r\n")
TEXT("\r\n")
TEXT("CREDITS:\r\n")
TEXT("  Program\r\n")
TEXT("      Tomohisa Hirami\r\n")
TEXT("  Improve document and convert to DocBook:\r\n")
TEXT("      Kevin Grover\r\n")
TEXT("\r\n")
TEXT("Thanks To:\r\n")
TEXT("  Gerry Busch\r\n")
TEXT("  Michael Efimov\r\n")
TEXT("  Shigeyuki Matsuki\r\n");


static LRESULT CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
#if defined(PLATFORM_PKTPC) || defined(PLATFORM_WM5)
		SHINITDLGINFO shidi;
		shidi.dwMask = SHIDIM_FLAGS;
		shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
		shidi.hDlg = hDlg;
		SHInitDialog(&shidi);
#endif
		{
			HWND hCopyright = GetDlgItem(hDlg, IDC_COPYRIGHT);
			SetWindowText(hCopyright, pCopyright);
			HWND hVersion = GetDlgItem(hDlg, IDC_VERSIONINFO);
			SetWindowText(hVersion, TOMBO_VERSION_INFO);
			return TRUE;
		}
	case WM_COMMAND:
		if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL)) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void AboutDialog::Popup(HINSTANCE hInst, HWND hParent)
{
	DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hParent, (DLGPROC)AboutDlgProc);
}