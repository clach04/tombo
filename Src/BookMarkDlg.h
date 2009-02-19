#ifndef BOOKMARKDLG_H
#define BOOKMARKDLG_H

class BookMark;
struct BookMarkItem;

class BookMarkDlg : public Tombo_Lib::DialogTemplate {
	BookMark *pBookMark;
protected:
	BOOL InsertItem(HWND hList, DWORD iPos, const BookMarkItem *pItem);
public:
	BOOL Init(BookMark *p);

	void InitDialog(HWND hDlg);
	BOOL OnOK();

	DWORD Popup(HINSTANCE hInst, HWND hParent) {
		return DialogTemplate::Popup(hInst, IDD_BOOKMARK_EDIT, hParent);
	}

	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
	BOOL Command_UpDown(HWND hDlg, int iDelta);
	BOOL Command_Delete(HWND hDlg);

};


#endif