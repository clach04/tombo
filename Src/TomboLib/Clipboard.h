#ifndef TOMBOUTL_CLIPBOARD_H
#define TOMBOUTL_CLIPBOARD_H

namespace Tombo_Lib {

class Clipboard {
	BOOL bOpen;
public:
	Clipboard();
	~Clipboard();

	BOOL Open(HWND hWnd);
	void Close();

	//////////////////////
	// Get text data
		// return value should delete[] if you don't need.
		// if clipboard is empty or not text, return NULL and *pEmpty == FALSE;
	LPTSTR GetText();

	//////////////////////
	// Set text data
		// Data type is CF_TEXT if win32 and CF_UNICODETEXT if WinCE.
	BOOL SetText(LPCTSTR pText);

};


}; // namespace Tombo_Lib

#endif
