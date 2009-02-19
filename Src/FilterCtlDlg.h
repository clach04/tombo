#ifndef FILTERCTLDLG_H
#define FILTERCTLDLG_H

class MemoSelectView;
class VFManager;

class FilterCtlDlg {
	HIMAGELIST hImageList;
	MemoSelectView *pView;
	VFManager *pManager;

	DWORD nNotKeepPos;

protected:
	void DeleteSelectedItem(HWND hDlg);

public:
	BOOL Init(MemoSelectView *pView, VFManager *pManager);

	DWORD Popup(HINSTANCE hInst, HWND hParent, HIMAGELIST hImgList); 

	void InitDialog(HWND hDlg);
	void DestroyDialog(HWND hDlg, int iResult);

	BOOL OnOK(HWND hDlg);
	BOOL OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);

	BOOL Command_UpDown(HWND hDlg, int delta);
	void Command_ToggleKeep(HWND hDlg);
	void Command_New(HWND hDlg);
	void Command_Delete(HWND hDlg);
	void Command_Rename(HWND hDlg);

	BOOL Notify_ItemChanged(HWND hDlg, LPARAM lParam);
	BOOL Notify_EndLabelEdit(HWND hDlg, LPARAM lParam);
	BOOL Notify_Keydown(HWND hDlg, LPARAM lParam);
	BOOL Notify_DblClick(HWND hDlg, LPARAM lParam);
};

#endif