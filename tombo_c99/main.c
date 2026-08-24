#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <richedit.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "encoding.h"
#include "bf01_file.h"

#define IDM_NEW      1001
#define IDM_OPEN     1002
#define IDM_SAVE     1003
#define IDM_SAVEAS   1004
#define IDM_EXIT     1005
#define IDM_UNDO     1010
#define IDM_CUT      1011
#define IDM_COPY     1012
#define IDM_PASTE    1013
#define IDM_FIND     1014
#define IDM_FINDNEXT 1015
#define IDM_FINDPREV 1016
#define IDM_WORDWRAP 1020
#define IDM_FORGETPASSWORD 1040
#define IDM_ABOUT    1030

#define IDT_PASSWORD 1

#define ID_TREE      2001
#define ID_EDITOR    2002
#define ID_STATUS    2003
#define ID_SPLITTER  2004

#define IDC_PASS_EDIT    3001
#define IDC_PASS_CONFIRM 3002
#define IDC_PASS_OK      3003
#define IDC_PASS_CANCEL  3004
#define IDC_PASS_EDIT2   3005
#define IDC_PASS_SHOW    3006

#define IDC_FIND_EDIT  3101
#define IDC_FIND_NEXT  3102
#define IDC_FIND_PREV  3103
#define IDC_FIND_CLOSE 3104

#define IDM_OPEN_DIR     2001
#define IDM_OPEN_ASSOC   2002
#define IDM_NEW_FOLDER   2003
#define IDM_ENCRYPT_FILE 2004
#define IDM_DECRYPT_FILE 2005
#define IDM_RENAME       2006
#define WM_START_LABEL_EDIT (WM_APP + 1)

static HINSTANCE g_hInst;
static HWND g_hWnd;
static HWND g_hTree, g_hEditor, g_hStatus, g_hSplitter;
static HWND g_hFindDlg;
static int g_passOk;
static int g_passEncrypt;
static int g_treeW = 200;
static int g_splitDrag;
static AppConfig g_cfg;
static char g_curFile[MAX_PATH];
static char g_curDir[MAX_PATH];
static BOOL g_dirty;
static HFONT g_hFont;
static wchar_t g_findText[256];

static char g_cached_pass[256];
static int g_pass_cached;
static DWORD g_pass_expire_tick;

static wchar_t *g_editor_wtext;
static int g_editor_wlen;
static UINT g_file_cp;
static char g_rightClickPath[MAX_PATH];
static HTREEITEM g_rightClickItem;

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK PassWndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK FindWndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK SplitterWndProc(HWND, UINT, WPARAM, LPARAM);
static void PopulateTree(HWND hTree, const char *dir, HTREEITEM hParent);
static void RefreshTree(void);
static void TomboOpenFile(const char *path);
static void SaveCurrentFile(void);
static void SaveFileAs(void);
static int PromptSave(void);
static void UpdateTitle(void);
static void UpdateStatus(void);
static void SetEditorFont(HWND hEd);
static int AskPassword(char *passBuf, int bufsize, int encrypt);
static void SetEditorTextW(const wchar_t *wtext);
static int GetEditorTextW(wchar_t **out_w, int *out_wlen);
static void NewFolderAt(HWND hTree, HTREEITEM hParent, const char *parentPath);
static void EncryptFileToDisk(const char *path);
static void DecryptFileToDisk(const char *path);
static int is_chi_file(const char *path);

static void PasswordCache_Set(const char *pass) {
  strncpy(g_cached_pass, pass, sizeof(g_cached_pass) - 1);
  g_cached_pass[sizeof(g_cached_pass) - 1] = '\0';
  g_pass_cached = 1;
  if (g_cfg.password_timeout > 0)
    g_pass_expire_tick = GetTickCount() + (DWORD)g_cfg.password_timeout * 1000;
}

static int PasswordCache_Get(char *passBuf, int bufsize) {
  if (g_pass_cached && g_cfg.password_timeout > 0 && GetTickCount() < g_pass_expire_tick) {
    strncpy(passBuf, g_cached_pass, bufsize - 1);
    passBuf[bufsize - 1] = '\0';
    return 1;
  }
  return 0;
}

static void PasswordCache_Clear(void) {
  SecureZeroMemory(g_cached_pass, sizeof(g_cached_pass));
  g_pass_cached = 0;
}

static void PasswordCache_ResetTimer(void) {
  if (g_pass_cached && g_cfg.password_timeout > 0)
    g_pass_expire_tick = GetTickCount() + (DWORD)g_cfg.password_timeout * 1000;
}

static int is_tombo_ext(const char *name) {
  const char *dot = strrchr(name, '.');
  if (!dot) return 0;
  return !_stricmp(dot, ".txt") || !_stricmp(dot, ".chi") || !_stricmp(dot, ".chs") || !_stricmp(dot, ".md");
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmdLine, int nShow) {
  WNDCLASSEX wc;
  MSG msg;
  INITCOMMONCONTROLSEX icc;

  (void)hPrev; (void)cmdLine;
  g_hInst = hInst;

  config_load(&g_cfg, CFG_PATH);

  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_TREEVIEW_CLASSES;
  InitCommonControlsEx(&icc);

  ZeroMemory(&wc, sizeof(wc));
  wc.cbSize = sizeof(wc);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInst;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszClassName = "TomboClass";
  RegisterClassEx(&wc);

  wc.lpfnWndProc = PassWndProc;
  wc.lpszClassName = "PassDialog";
  RegisterClassEx(&wc);

  wc.lpfnWndProc = FindWndProc;
  wc.lpszClassName = "FindDialog";
  RegisterClassEx(&wc);

  wc.style = CS_HREDRAW;
  wc.lpfnWndProc = SplitterWndProc;
  wc.lpszClassName = "Splitter";
  RegisterClassEx(&wc);

  {
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    if (g_cfg.win_w < 100) g_cfg.win_w = 800;
    if (g_cfg.win_h < 100) g_cfg.win_h = 600;
    if (g_cfg.win_w > sw) g_cfg.win_w = sw;
    if (g_cfg.win_h > sh) g_cfg.win_h = sh;
    if (g_cfg.win_x < 0) g_cfg.win_x = 0;
    if (g_cfg.win_y < 0) g_cfg.win_y = 0;
    if (g_cfg.win_x > sw - g_cfg.win_w) g_cfg.win_x = sw - g_cfg.win_w;
    if (g_cfg.win_y > sh - g_cfg.win_h) g_cfg.win_y = sh - g_cfg.win_h;
  }

  g_hWnd = CreateWindowEx(0, "TomboClass", "Tombo",
    WS_OVERLAPPEDWINDOW, g_cfg.win_x, g_cfg.win_y,
    g_cfg.win_w, g_cfg.win_h,
    NULL, NULL, hInst, NULL);

  {
    RECT wrc;
    GetWindowRect(g_hWnd, &wrc);
    g_cfg.win_x = wrc.left; g_cfg.win_y = wrc.top;
    g_cfg.win_w = wrc.right - wrc.left; g_cfg.win_h = wrc.bottom - wrc.top;
  }

  ShowWindow(g_hWnd, nShow);
  UpdateWindow(g_hWnd);

  while (GetMessage(&msg, NULL, 0, 0)) {
    if (g_hFindDlg && IsDialogMessage(g_hFindDlg, &msg)) continue;
    if (msg.message == WM_KEYDOWN && (msg.wParam == VK_OEM_PLUS || msg.wParam == VK_OEM_MINUS)
        && GetFocus() == g_hTree) {
      HTREEITEM hSel = TreeView_GetSelection(g_hTree);
      if (hSel) {
        TreeView_Expand(g_hTree, hSel,
          msg.wParam == VK_OEM_PLUS ? TVE_EXPAND : TVE_COLLAPSE);
      }
      continue;
    }
    if (msg.message == WM_KEYDOWN && msg.wParam == VK_TAB) {
      HWND hFocus = GetFocus();
      int shift = GetKeyState(VK_SHIFT) & 0x8000;
      if (hFocus == g_hTree && !shift) {
        SetFocus(g_hEditor);
        continue;
      }
      if (hFocus == g_hEditor && shift) {
        SetFocus(g_hTree);
        continue;
      }
      if (hFocus == g_hEditor && !shift) {
        SendMessage(g_hEditor, EM_REPLACESEL, TRUE, (LPARAM)"\t");
        continue;
      }
    }
    if (msg.message == WM_KEYDOWN && (GetKeyState(VK_CONTROL) & 0x8000)) {
      int id = 0;
      switch (msg.wParam) {
      case 'N': id = IDM_NEW; break;
      case 'O': id = IDM_OPEN; break;
      case 'S': id = g_curFile[0] ? IDM_SAVE : IDM_SAVEAS; break;
      case 'F': id = IDM_FIND; break;
      case 'Z': id = IDM_UNDO; break;
      }
      if (id) { SendMessage(g_hWnd, WM_COMMAND, id, 0); continue; }
    }
    if (msg.message == WM_KEYDOWN && msg.wParam == VK_F2 && GetFocus() == g_hTree) {
      HTREEITEM hSel = TreeView_GetSelection(g_hTree);
      if (hSel && hSel != TreeView_GetRoot(g_hTree))
        SendMessage(g_hTree, TVM_EDITLABEL, 0, (LPARAM)hSel);
      continue;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return (int)msg.wParam;
}

static void SetEditorFont(HWND hEd) {
  if (g_hFont) DeleteObject(g_hFont);
  g_hFont = CreateFont(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
  SendMessage(hEd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
}

static void InitMenu(HWND hWnd) {
  HMENU hMenu = GetMenu(hWnd);
  EnableMenuItem(hMenu, IDM_SAVE, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_SAVEAS, MF_GRAYED);
}

static void UpdateMenuSaveState(HWND hWnd) {
  HMENU hMenu = GetMenu(hWnd);
  EnableMenuItem(hMenu, IDM_SAVE, g_curFile[0] ? MF_ENABLED : MF_GRAYED);
  EnableMenuItem(hMenu, IDM_SAVEAS, MF_ENABLED);
}

static LRESULT CALLBACK SplitterWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_SETCURSOR:
    SetCursor(LoadCursor(NULL, IDC_SIZEWE));
    return TRUE;
  case WM_LBUTTONDOWN:
    SetCapture(hWnd);
    g_splitDrag = 1;
    return 0;
  case WM_LBUTTONUP:
    if (g_splitDrag) { ReleaseCapture(); g_splitDrag = 0; }
    return 0;
  case WM_MOUSEMOVE:
    if (g_splitDrag) {
      POINT pt;
      RECT crc;
      pt.x = (short)LOWORD(lParam);
      pt.y = (short)HIWORD(lParam);
      ClientToScreen(hWnd, &pt);
      ScreenToClient(g_hWnd, &pt);
      if (pt.x < 50) pt.x = 50;
      g_treeW = pt.x;
      GetClientRect(g_hWnd, &crc);
      SendMessage(g_hWnd, WM_SIZE, 0, MAKELPARAM(crc.right, crc.bottom));
    }
    return 0;
  }
  return DefWindowProc(hWnd, msg, wParam, lParam);
}

static HWND CreateEditor(HWND hParent, int wrap) {
  DWORD style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL;
  if (!wrap) style |= WS_HSCROLL | ES_AUTOHSCROLL;
  return CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
    style, 0, 0, 100, 100, hParent, (HMENU)ID_EDITOR, g_hInst, NULL);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_CREATE: {
    HMENU hMenu = CreateMenu();
    HMENU hFile = CreatePopupMenu();
    HMENU hEdit = CreatePopupMenu();
    HMENU hView = CreatePopupMenu();
    HMENU hTools = CreatePopupMenu();
    HMENU hHelp = CreatePopupMenu();

    AppendMenu(hFile, MF_STRING, IDM_NEW, "&New\tCtrl+N");
    AppendMenu(hFile, MF_STRING, IDM_OPEN, "&Open...\tCtrl+O");
    AppendMenu(hFile, MF_STRING, IDM_SAVE, "&Save\tCtrl+S");
    AppendMenu(hFile, MF_STRING, IDM_SAVEAS, "Save &As...");
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, IDM_EXIT, "E&xit");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFile, "&File");

    AppendMenu(hEdit, MF_STRING, IDM_UNDO, "&Undo\tCtrl+Z");
    AppendMenu(hEdit, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEdit, MF_STRING, IDM_CUT, "Cu&t\tCtrl+X");
    AppendMenu(hEdit, MF_STRING, IDM_COPY, "&Copy\tCtrl+C");
    AppendMenu(hEdit, MF_STRING, IDM_PASTE, "&Paste\tCtrl+V");
    AppendMenu(hEdit, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEdit, MF_STRING, IDM_FIND, "&Find...\tCtrl+F");
    AppendMenu(hEdit, MF_STRING, IDM_FINDNEXT, "Find &Next\tF3");
    AppendMenu(hEdit, MF_STRING, IDM_FINDPREV, "Find &Previous\tShift+F3");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEdit, "&Edit");

    AppendMenu(hView, MF_STRING, IDM_WORDWRAP, "&Word Wrap");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hView, "&View");

    AppendMenu(hTools, MF_STRING, IDM_FORGETPASSWORD, "&Forget Password");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hTools, "&Tools");

    AppendMenu(hHelp, MF_STRING, IDM_ABOUT, "&About");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelp, "&Help");
    SetMenu(hWnd, hMenu);
    InitMenu(hWnd);

    g_hTree = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, "",
      WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_EDITLABELS,
      0, 0, 200, 400, hWnd, (HMENU)ID_TREE, g_hInst, NULL);

    g_hSplitter = CreateWindow("Splitter", "",
      WS_CHILD | WS_VISIBLE, 200, 0, 4, 400, hWnd, (HMENU)ID_SPLITTER, g_hInst, NULL);

    g_hEditor = CreateEditor(hWnd, 0);

    g_hStatus = CreateWindowEx(0, STATUSCLASSNAME, "",
      WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
      0, 0, 0, 0, hWnd, (HMENU)ID_STATUS, g_hInst, NULL);

    SetEditorFont(g_hEditor);
    SendMessage(g_hEditor, EM_SETLIMITTEXT, 0, 0);

    if (g_cfg.last_dir[0]) strncpy(g_curDir, g_cfg.last_dir, MAX_PATH - 1);
    else GetCurrentDirectory(MAX_PATH, g_curDir);
    g_treeW = g_cfg.tree_w;
    RefreshTree();
    SetFocus(g_hTree);
    if (g_cfg.password_timeout > 0)
      SetTimer(hWnd, IDT_PASSWORD, 1000, NULL);
    return 0;
  }

  case WM_SIZE: {
    RECT rc;
    int w = LOWORD(lParam), h = HIWORD(lParam);
    int statusH = 0;
    HDWP hdwp;

    SendMessage(g_hStatus, WM_SIZE, 0, 0);
    GetWindowRect(g_hStatus, &rc);
    statusH = rc.bottom - rc.top;

    hdwp = BeginDeferWindowPos(4);
    DeferWindowPos(hdwp, g_hTree, NULL, 0, 0, g_treeW, h - statusH, SWP_NOZORDER);
    DeferWindowPos(hdwp, g_hSplitter, NULL, g_treeW, 0, 4, h - statusH, SWP_NOZORDER);
    DeferWindowPos(hdwp, g_hEditor, NULL, g_treeW + 4, 0, w - g_treeW - 4, h - statusH, SWP_NOZORDER);
    DeferWindowPos(hdwp, g_hStatus, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
    EndDeferWindowPos(hdwp);
    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
    return 0;
  }

  case WM_COMMAND:
    if (LOWORD(wParam) == ID_EDITOR && HIWORD(wParam) == EN_CHANGE) {
      if (!g_dirty) {
        g_dirty = TRUE;
        UpdateMenuSaveState(hWnd);
        UpdateTitle();
      }
      return 0;
    }
    switch (LOWORD(wParam)) {
    case IDM_NEW:
      if (PromptSave() != IDCANCEL) {
        g_curFile[0] = '\0';
        SetWindowTextW(g_hEditor, L"");
        if (g_editor_wtext) { free(g_editor_wtext); g_editor_wtext = NULL; }
        g_editor_wlen = 0;
        g_file_cp = 0;
        g_dirty = FALSE;
        UpdateMenuSaveState(hWnd);
        UpdateTitle();
        UpdateStatus();
        SetFocus(g_hEditor);
      }
      break;
    case IDM_OPEN: {
      OPENFILENAME ofn;
      char file[MAX_PATH] = "";
      ZeroMemory(&ofn, sizeof(ofn));
      ofn.lStructSize = sizeof(ofn);
      ofn.hwndOwner = hWnd;
      ofn.lpstrFilter = "Text Files (*.txt;*.md)\0*.txt;*.md\0Encrypted (*.chi;*.chs)\0*.chi;*.chs\0All Files (*.*)\0*.*\0";
      ofn.lpstrFile = file;
      ofn.nMaxFile = MAX_PATH;
      ofn.lpstrInitialDir = g_curDir[0] ? g_curDir : NULL;
      ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
      if (GetOpenFileName(&ofn) && PromptSave() != IDCANCEL) TomboOpenFile(file);
      break;
    }
    case IDM_SAVE:
      if (g_curFile[0]) SaveCurrentFile();
      else SaveFileAs();
      break;
    case IDM_SAVEAS:
      SaveFileAs();
      break;
    case IDM_EXIT:
      SendMessage(hWnd, WM_CLOSE, 0, 0);
      break;
    case IDM_UNDO:
      SendMessage(g_hEditor, EM_UNDO, 0, 0);
      break;
    case IDM_CUT:
      SendMessage(g_hEditor, WM_CUT, 0, 0);
      break;
    case IDM_COPY:
      SendMessage(g_hEditor, WM_COPY, 0, 0);
      break;
    case IDM_PASTE:
      SendMessage(g_hEditor, WM_PASTE, 0, 0);
      break;
    case IDM_FIND:
      if (!g_hFindDlg) {
        g_hFindDlg = CreateWindowEx(WS_EX_TOOLWINDOW, "FindDialog", "Find",
          WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
          CW_USEDEFAULT, CW_USEDEFAULT, 300, 100,
          hWnd, NULL, g_hInst, NULL);
      }
      ShowWindow(g_hFindDlg, SW_SHOW);
      SetFocus(g_hFindDlg);
      break;
    case IDM_FINDNEXT: {
      FINDTEXTW ft;
      LONG selEnd;
      LONG pos;
      SendMessage(g_hEditor, EM_GETSEL, 0, (WPARAM)&selEnd);
      ft.chrg.cpMin = selEnd;
      ft.chrg.cpMax = -1;
      ft.lpstrText = g_findText;
      if (g_findText[0] && (pos = (LONG)SendMessageW(g_hEditor, EM_FINDTEXTW, 0, (LPARAM)&ft)) >= 0)
        SendMessage(g_hEditor, EM_SETSEL, pos, pos + (int)wcslen(g_findText));
      break;
    }
    case IDM_FINDPREV: {
      FINDTEXTW ft;
      LONG selStart;
      LONG pos;
      SendMessage(g_hEditor, EM_GETSEL, (WPARAM)&selStart, 0);
      ft.chrg.cpMin = selStart;
      ft.chrg.cpMax = 0;
      ft.lpstrText = g_findText;
      if (g_findText[0] && (pos = (LONG)SendMessageW(g_hEditor, EM_FINDTEXTW, FR_DOWN, (LPARAM)&ft)) >= 0)
        SendMessage(g_hEditor, EM_SETSEL, pos, pos + (int)wcslen(g_findText));
      break;
    }
    case IDM_WORDWRAP: {
      int len = GetWindowTextLengthW(g_hEditor);
      wchar_t *wtext = NULL;
      RECT edRc;
      HWND hOld = g_hEditor;
      if (len > 0) {
        wtext = (wchar_t *)malloc((len + 1) * sizeof(wchar_t));
        if (wtext) GetWindowTextW(g_hEditor, wtext, len + 1);
      }
      GetWindowRect(g_hEditor, &edRc);
      MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&edRc, 2);
      g_cfg.word_wrap = !g_cfg.word_wrap;
      g_hEditor = CreateEditor(hWnd, g_cfg.word_wrap);
      SetWindowPos(g_hEditor, NULL, edRc.left, edRc.top,
        edRc.right - edRc.left, edRc.bottom - edRc.top, SWP_NOZORDER);
      SetEditorFont(g_hEditor);
      SendMessage(g_hEditor, EM_SETLIMITTEXT, 0, 0);
      if (wtext) {
        SetWindowTextW(g_hEditor, wtext);
        free(wtext);
      }
      DestroyWindow(hOld);
      SetFocus(g_hEditor);
      CheckMenuItem(GetMenu(hWnd), IDM_WORDWRAP,
        g_cfg.word_wrap ? MF_CHECKED : MF_UNCHECKED);
      break;
    }
    case IDM_FORGETPASSWORD:
      PasswordCache_Clear();
      UpdateStatus();
      break;
    case IDM_ABOUT:
      MessageBox(hWnd, "Tombo C99 - Plain text editor with encryption",
        "About Tombo", MB_OK | MB_ICONINFORMATION);
      break;
    case IDM_OPEN_DIR:
      if (g_rightClickPath[0])
        ShellExecuteA(hWnd, "explore", g_rightClickPath, NULL, NULL, SW_SHOW);
      break;
    case IDM_OPEN_ASSOC:
      if (g_rightClickPath[0])
        ShellExecuteA(hWnd, "open", g_rightClickPath, NULL, NULL, SW_SHOW);
      break;
    case IDM_NEW_FOLDER:
      NewFolderAt(g_hTree, g_rightClickItem,
        g_rightClickPath[0] ? g_rightClickPath : g_curDir);
      break;
    case IDM_RENAME:
      if (g_rightClickItem)
        TreeView_EditLabel(g_hTree, g_rightClickItem);
      break;
    case IDM_ENCRYPT_FILE:
      if (g_rightClickPath[0] && g_rightClickItem) {
        if (_stricmp(g_rightClickPath, g_curFile) == 0 && PromptSave() == IDCANCEL)
          break;
        EncryptFileToDisk(g_rightClickPath);
      }
      break;
    case IDM_DECRYPT_FILE:
      if (g_rightClickPath[0] && g_rightClickItem) {
        if (_stricmp(g_rightClickPath, g_curFile) == 0 && PromptSave() == IDCANCEL)
          break;
        DecryptFileToDisk(g_rightClickPath);
      }
      break;
    }
    return 0;

  case WM_NOTIFY: {
    NMHDR *nm = (NMHDR *)lParam;
    if (nm->idFrom == ID_TREE && nm->code == TVN_DELETEITEM) {
      NMTREEVIEW *tv = (NMTREEVIEW *)lParam;
      if (tv->itemOld.lParam)
        free((void *)tv->itemOld.lParam);
      return 0;
    }
    if (nm->idFrom == ID_TREE && nm->code == NM_RCLICK) {
      TVHITTESTINFO ht;
      DWORD msgPos = GetMessagePos();
      POINT pt;
      pt.x = (short)LOWORD(msgPos);
      pt.y = (short)HIWORD(msgPos);
      ht.pt = pt;
      ScreenToClient(g_hTree, &ht.pt);
      {
        HTREEITEM hItem = (HTREEITEM)SendMessage(g_hTree, TVM_HITTEST, 0, (LPARAM)&ht);
        if (hItem && (ht.flags & TVHT_ONITEM)) {
          TreeView_SelectItem(g_hTree, hItem);
          {
            TVITEM ti;
            ZeroMemory(&ti, sizeof(ti));
            ti.mask = TVIF_PARAM;
            ti.hItem = hItem;
            TreeView_GetItem(g_hTree, &ti);
            if (ti.lParam) {
              strncpy(g_rightClickPath, (const char *)ti.lParam, MAX_PATH - 1);
              g_rightClickPath[MAX_PATH - 1] = '\0';
              g_rightClickItem = hItem;
            } else {
              g_rightClickPath[0] = '\0';
              g_rightClickItem = NULL;
            }
          }
        } else {
          strncpy(g_rightClickPath, g_curDir, MAX_PATH - 1);
          g_rightClickPath[MAX_PATH - 1] = '\0';
          g_rightClickItem = NULL;
        }
      }
      {
        HMENU hPopup = CreatePopupMenu();
        if (g_rightClickItem) {
          BOOL isFile = g_rightClickPath[0] && is_tombo_ext(g_rightClickPath);
          if (isFile)
            AppendMenuA(hPopup, MF_STRING, IDM_OPEN_ASSOC, "Open");
          else if (g_rightClickPath[0])
            AppendMenuA(hPopup, MF_STRING, IDM_OPEN_DIR, "Open Directory");
          AppendMenuA(hPopup, MF_STRING, IDM_RENAME, "Rename");
          AppendMenuA(hPopup, MF_SEPARATOR, 0, NULL);
        }
        {
          BOOL isFile = g_rightClickItem && g_rightClickPath[0] && is_tombo_ext(g_rightClickPath);
          if (isFile) {
            if (is_chi_file(g_rightClickPath))
              AppendMenuA(hPopup, MF_STRING, IDM_DECRYPT_FILE, "Decrypt");
            else
              AppendMenuA(hPopup, MF_STRING, IDM_ENCRYPT_FILE, "Encrypt");
          } else {
            AppendMenuA(hPopup, MF_STRING, IDM_NEW_FOLDER, "New Folder");
          }
        }
        {
          POINT scpt = pt;
          ClientToScreen(g_hTree, &scpt);
          TrackPopupMenu(hPopup, TPM_LEFTBUTTON, scpt.x, scpt.y, 0, hWnd, NULL);
          SetFocus(g_hTree);
        }
        DestroyMenu(hPopup);
      }
      return 0;
    }
    if (nm->idFrom == ID_TREE && nm->code == NM_DBLCLK) {
      HTREEITEM hSel = TreeView_GetSelection(g_hTree);
      if (hSel) {
        TVITEM ti;
        ZeroMemory(&ti, sizeof(ti));
        ti.mask = TVIF_PARAM;
        ti.hItem = hSel;
        TreeView_GetItem(g_hTree, &ti);
        if (ti.lParam && is_tombo_ext((const char *)ti.lParam) && PromptSave() != IDCANCEL)
          TomboOpenFile((const char *)ti.lParam);
      }
    }
    if (nm->idFrom == ID_TREE && nm->code == TVN_KEYDOWN) {
      NMTVKEYDOWN *kd = (NMTVKEYDOWN *)lParam;
      if (kd->wVKey == VK_RETURN) {
        HTREEITEM hSel = TreeView_GetSelection(g_hTree);
        if (hSel) {
          TVITEM ti;
          ZeroMemory(&ti, sizeof(ti));
          ti.mask = TVIF_PARAM;
          ti.hItem = hSel;
          TreeView_GetItem(g_hTree, &ti);
          if (ti.lParam && is_tombo_ext((const char *)ti.lParam)) {
            if (PromptSave() != IDCANCEL) TomboOpenFile((const char *)ti.lParam);
          } else {
            UINT state = TreeView_GetItemState(g_hTree, hSel, TVIS_EXPANDED);
            TreeView_Expand(g_hTree, hSel, (state & TVIS_EXPANDED) ? TVE_COLLAPSE : TVE_EXPAND);
          }
        }
        return 0;
      }
      if (kd->wVKey == VK_ADD || kd->wVKey == VK_SUBTRACT) {
        HTREEITEM hSel = TreeView_GetSelection(g_hTree);
        if (hSel) {
          TreeView_Expand(g_hTree, hSel,
            kd->wVKey == VK_ADD ? TVE_EXPAND : TVE_COLLAPSE);
        }
        return 0;
      }
    }
    if (nm->idFrom == ID_TREE && nm->code == TVN_BEGINLABELEDITA) {
      NMTVDISPINFOA *di = (NMTVDISPINFOA *)lParam;
      HTREEITEM hRoot = TreeView_GetRoot(g_hTree);
      if (di->item.hItem == hRoot)
        return TRUE;
      return FALSE;
    }
    if (nm->idFrom == ID_TREE && nm->code == TVN_ENDLABELEDITA) {
      NMTVDISPINFOA *di = (NMTVDISPINFOA *)lParam;
      if (di->item.pszText) {
        TVITEM ti;
        ZeroMemory(&ti, sizeof(ti));
        ti.mask = TVIF_PARAM;
        ti.hItem = di->item.hItem;
        if (TreeView_GetItem(g_hTree, &ti) && ti.lParam) {
          char oldPath[MAX_PATH], parentDir[MAX_PATH], newPath[MAX_PATH];
          char *lastSep;
          strncpy(oldPath, (const char *)ti.lParam, MAX_PATH - 1);
          oldPath[MAX_PATH - 1] = '\0';
          strncpy(parentDir, oldPath, MAX_PATH - 1);
          lastSep = strrchr(parentDir, '\\');
          if (lastSep) {
            *lastSep = '\0';
            snprintf(newPath, MAX_PATH, "%s\\%s", parentDir, di->item.pszText);
            if (strcmp(oldPath, newPath) != 0) {
              if (MoveFileA(oldPath, newPath)) {
                free((void *)ti.lParam);
                ti.lParam = (LPARAM)_strdup(newPath);
                TreeView_SetItem(g_hTree, &ti);
                if (_stricmp(oldPath, g_curFile) == 0) {
                  strncpy(g_curFile, newPath, MAX_PATH - 1);
                  g_curFile[MAX_PATH - 1] = '\0';
                  UpdateTitle();
                  UpdateStatus();
                }
              } else {
                MessageBox(g_hWnd, "Cannot rename: name may be in use or invalid",
                  "Error", MB_OK | MB_ICONERROR);
                return FALSE;
              }
            }
          }
        }
      }
      return TRUE;
    }
    return 0;
  }

  case WM_START_LABEL_EDIT:
    if (lParam) {
      SendMessage(g_hTree, TVM_EDITLABEL, 0, (LPARAM)lParam);
    }
    return 0;

  case WM_TIMER:
    if (wParam == IDT_PASSWORD) {
      if (g_pass_cached && GetTickCount() >= g_pass_expire_tick)
        PasswordCache_Clear();
    }
    return 0;

  case WM_GETMINMAXINFO: {
    MINMAXINFO *mmi = (MINMAXINFO *)lParam;
    mmi->ptMinTrackSize.x = 400;
    mmi->ptMinTrackSize.y = 300;
    return 0;
  }

  case WM_CLOSE:
    if (PromptSave() == IDCANCEL) return 0;
    PasswordCache_Clear();
    KillTimer(hWnd, IDT_PASSWORD);
    {
      RECT rc;
      AppConfig saved = g_cfg;
      if (g_cfg.persist_window) {
        GetWindowRect(hWnd, &rc);
        g_cfg.win_x = rc.left; g_cfg.win_y = rc.top;
        g_cfg.win_w = rc.right - rc.left; g_cfg.win_h = rc.bottom - rc.top;
      }
      g_cfg.tree_w = g_treeW;
      if (!config_equal(&g_cfg, &saved))
        config_save(&g_cfg, CFG_PATH);
    }
    if (g_editor_wtext) { free(g_editor_wtext); g_editor_wtext = NULL; }
    DestroyWindow(hWnd);
    return 0;

  case WM_DESTROY:
    if (g_hFont) DeleteObject(g_hFont);
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hWnd, msg, wParam, lParam);
}

/* --- Tree view --- */

static void PopulateTree(HWND hTree, const char *dir, HTREEITEM hParent) {
  WIN32_FIND_DATA fd;
  char pattern[MAX_PATH], childPath[MAX_PATH];
  HANDLE hFind;
  TVINSERTSTRUCT tvi;
  int pass;
  int dirs_first = g_cfg.sort_dirs_first;

  for (pass = 0; pass < (dirs_first ? 2 : 1); pass++) {
    snprintf(pattern, MAX_PATH, "%s\\*", dir);
    hFind = FindFirstFile(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) continue;

    do {
      if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if (fd.cFileName[0] == '.') continue;
        if (dirs_first && pass != 0) continue;
        snprintf(childPath, MAX_PATH, "%s\\%s", dir, fd.cFileName);

        ZeroMemory(&tvi, sizeof(tvi));
        tvi.hParent = hParent;
        tvi.hInsertAfter = dirs_first ? TVI_LAST : TVI_SORT;
        tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvi.item.pszText = fd.cFileName;
        tvi.item.iImage = 0;
        tvi.item.iSelectedImage = 0;
        tvi.item.lParam = (LPARAM)_strdup(childPath);

        {
          HTREEITEM hItem = TreeView_InsertItem(hTree, &tvi);
          PopulateTree(hTree, childPath, hItem);
        }
      } else if (is_tombo_ext(fd.cFileName)) {
        if (dirs_first && pass != 1) continue;
        {
          char *fullPath = (char *)malloc(MAX_PATH);
          if (!fullPath) continue;
          snprintf(fullPath, MAX_PATH, "%s\\%s", dir, fd.cFileName);

          ZeroMemory(&tvi, sizeof(tvi));
          tvi.hParent = hParent;
          tvi.hInsertAfter = dirs_first ? TVI_LAST : TVI_SORT;
          tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
          tvi.item.pszText = fd.cFileName;
          tvi.item.iImage = 1;
          tvi.item.iSelectedImage = 1;
          tvi.item.lParam = (LPARAM)fullPath;
          TreeView_InsertItem(hTree, &tvi);
        }
      }
    } while (FindNextFile(hFind, &fd));
    FindClose(hFind);
  }
}

static void RefreshTree(void) {
  TVINSERTSTRUCT tvi;
  HTREEITEM hRoot;
  TreeView_DeleteAllItems(g_hTree);
  if (!g_curDir[0]) return;
  ZeroMemory(&tvi, sizeof(tvi));
  tvi.hParent = TVI_ROOT;
  tvi.hInsertAfter = TVI_LAST;
  tvi.item.mask = TVIF_TEXT;
  tvi.item.pszText = "Root";
  hRoot = TreeView_InsertItem(g_hTree, &tvi);
  PopulateTree(g_hTree, g_curDir, hRoot);
  TreeView_Expand(g_hTree, hRoot, TVE_EXPAND);
}

static void NewFolderAt(HWND hTree, HTREEITEM hParent, const char *parentPath) {
  char newPath[MAX_PATH];
  int n = 0;
  do {
    if (n == 0)
      snprintf(newPath, MAX_PATH, "%s\\New Folder", parentPath);
    else
      snprintf(newPath, MAX_PATH, "%s\\New Folder (%d)", parentPath, n);
    n++;
  } while (GetFileAttributesA(newPath) != INVALID_FILE_ATTRIBUTES);

  if (!CreateDirectoryA(newPath, NULL)) {
    MessageBox(g_hWnd, "Cannot create directory", "Error", MB_OK | MB_ICONERROR);
    return;
  }

  if (!hParent) {
    hParent = TreeView_GetRoot(hTree);
  }

  {
    TVINSERTSTRUCT tvi;
    const char *name;
    HTREEITEM hNew;
    ZeroMemory(&tvi, sizeof(tvi));
    tvi.hParent = hParent ? hParent : TVI_ROOT;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_TEXT | TVIF_PARAM;
    name = strrchr(newPath, '\\');
    tvi.item.pszText = (LPSTR)(name ? name + 1 : newPath);
    tvi.item.lParam = (LPARAM)_strdup(newPath);
    hNew = TreeView_InsertItem(hTree, &tvi);

    if (hNew) {
      TreeView_SelectItem(hTree, hNew);
      if (hParent)
        TreeView_Expand(hTree, hParent, TVE_EXPAND);
      TreeView_EnsureVisible(hTree, hNew);
      PostMessage(g_hWnd, WM_START_LABEL_EDIT, 0, (LPARAM)hNew);
    }
  }
}

/* --- File I/O --- */

static int is_chi_file(const char *path) {
  const char *dot = strrchr(path, '.');
  return dot && (!_stricmp(dot, ".chi") || !_stricmp(dot, ".chs"));
}

static int strip_cr_w(wchar_t *buf, int len) {
  int w = 0, r;
  for (r = 0; r < len; r++) {
    if (buf[r] != L'\r') buf[w++] = buf[r];
  }
  return w;
}

static wchar_t *expand_lf_w(const wchar_t *src, int srclen, int *dstlen) {
  int i, count = 0;
  wchar_t *dst;
  for (i = 0; i < srclen; i++)
    count += (src[i] == L'\n') ? 2 : 1;
  dst = (wchar_t *)malloc((count + 1) * sizeof(wchar_t));
  if (!dst) { *dstlen = 0; return NULL; }
  {
    int w = 0;
    for (i = 0; i < srclen; i++) {
      if (src[i] == L'\n') dst[w++] = L'\r';
      dst[w++] = src[i];
    }
    dst[w] = L'\0';
    *dstlen = w;
  }
  return dst;
}

static void SetEditorTextW(const wchar_t *wtext) {
  SetWindowTextW(g_hEditor, wtext);
}

static int GetEditorTextW(wchar_t **out_w, int *out_wlen) {
  int wlen = GetWindowTextLengthW(g_hEditor);
  *out_w = (wchar_t *)malloc((wlen + 1) * sizeof(wchar_t));
  if (!*out_w) { *out_wlen = 0; return 0; }
  GetWindowTextW(g_hEditor, *out_w, wlen + 1);
  *out_wlen = wlen;
  return 1;
}

static void TomboOpenFileRaw(const unsigned char *raw, long rawlen) {
  wchar_t *wtext = NULL;
  int wlen = 0;
  int bom_len = 0;
  UINT cp;
  int i;

  cp = encoding_detect_bom(raw, rawlen, &bom_len);
  if (cp) {
    g_file_cp = cp;
    if (cp == 1200 || cp == 1201) {
      int wlen2 = MultiByteToWideChar(cp, 0, (const char *)raw + bom_len, (int)(rawlen - bom_len), NULL, 0);
      if (wlen2 > 0) {
        wtext = (wchar_t *)malloc((wlen2 + 1) * sizeof(wchar_t));
        if (wtext) {
          MultiByteToWideChar(cp, 0, (const char *)raw + bom_len, (int)(rawlen - bom_len), wtext, wlen2);
          wtext[wlen2] = L'\0';
          wlen = wlen2;
        }
      }
    } else {
      encoding_to_wide(raw + bom_len, (int)(rawlen - bom_len), cp, &wtext, &wlen);
    }
  }

  if (!wtext) {
    for (i = 0; i < g_cfg.encoding_count; i++) {
      if (encoding_to_wide(raw + bom_len, (int)(rawlen - bom_len), g_cfg.encoding_cps[i], &wtext, &wlen)) {
        g_file_cp = g_cfg.encoding_cps[i];
        break;
      }
    }
  }

  if (!wtext) {
    MessageBox(g_hWnd, "Cannot decode file with any configured encoding", "Error", MB_OK | MB_ICONERROR);
    return;
  }

  {
    int explen;
    wchar_t *expanded = expand_lf_w(wtext, wlen, &explen);
    free(wtext);
    if (!expanded) return;
    SetEditorTextW(expanded);
    if (g_editor_wtext) free(g_editor_wtext);
    g_editor_wtext = expanded;
    g_editor_wlen = explen;
  }
}

static void TomboOpenFile(const char *path) {
  if (is_chi_file(path)) {
    char pass[256] = "";
    FILE *fin;
    unsigned char *filedata;
    long filesize;
    unsigned char *plain;
    size_t plainlen;

    if (!AskPassword(pass, sizeof(pass), 0))
      return;

    fin = fopen(path, "rb");
    if (!fin) { MessageBox(g_hWnd, "Cannot open file", "Error", MB_OK | MB_ICONERROR); return; }
    fseek(fin, 0, SEEK_END);
    filesize = ftell(fin);
    rewind(fin);
    filedata = (unsigned char *)malloc(filesize);
    if (!filedata) { fclose(fin); MessageBox(g_hWnd, "Out of memory", "Error", MB_OK | MB_ICONERROR); return; }
    fread(filedata, 1, filesize, fin);
    fclose(fin);

    plain = bf01_decrypt_mem(filedata, filesize, pass, &plainlen, 0);
    free(filedata);
    if (!plain) {
      MessageBox(g_hWnd, "Decryption failed (wrong password?)", "Error", MB_OK | MB_ICONERROR);
      return;
    }
    PasswordCache_Set(pass);
    PasswordCache_ResetTimer();
    TomboOpenFileRaw(plain, (long)plainlen);
    free(plain);
  } else {
    FILE *f = fopen(path, "rb");
    long sz;
    unsigned char *buf;
    if (!f) { MessageBox(g_hWnd, "Cannot open file", "Error", MB_OK | MB_ICONERROR); return; }
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    rewind(f);
    buf = (unsigned char *)malloc(sz);
    if (buf) {
      fread(buf, 1, sz, f);
      TomboOpenFileRaw(buf, sz);
      free(buf);
    }
    fclose(f);
  }
  strncpy(g_curFile, path, MAX_PATH - 1);
  g_curFile[MAX_PATH - 1] = '\0';
  {
    const char *lastSlash = strrchr(path, '\\');
    if (!lastSlash) lastSlash = strrchr(path, '/');
    if (lastSlash) {
      int len = (int)(lastSlash - path);
      strncpy(g_curDir, path, len);
      g_curDir[len] = '\0';
    }
  }
  g_dirty = FALSE;
  SendMessage(g_hEditor, EM_SETMODIFY, FALSE, 0);
  UpdateMenuSaveState(g_hWnd);
  UpdateTitle();
  UpdateStatus();
}

static void SaveCurrentFile(void) {
  wchar_t *wbuf;
  int wlen;
  char *bytes = NULL;
  int blen = 0;
  UINT save_cp;
  if (!g_curFile[0]) { SaveFileAs(); return; }

  if (!GetEditorTextW(&wbuf, &wlen)) return;
  wlen = strip_cr_w(wbuf, wlen);

  save_cp = g_file_cp ? g_file_cp : (g_cfg.encoding_count > 0 ? g_cfg.encoding_cps[0] : CP_UTF8);
  if (!wide_to_encoding(wbuf, wlen, save_cp, &bytes, &blen)) {
    MessageBox(g_hWnd, "Cannot encode text to target encoding", "Error", MB_OK | MB_ICONERROR);
    free(wbuf);
    return;
  }
  free(wbuf);

  if (is_chi_file(g_curFile)) {
    char pass[256] = "";
    unsigned char *cipher;
    size_t cipherlen;
    if (!AskPassword(pass, sizeof(pass), 1)) {
      free(bytes);
      return;
    }
    cipher = bf01_encrypt_mem((unsigned char *)bytes, blen, pass, &cipherlen, 0);
    if (!cipher) {
      MessageBox(g_hWnd, "Encryption failed", "Error", MB_OK | MB_ICONERROR);
      free(bytes);
      return;
    }
    PasswordCache_Set(pass);
    PasswordCache_ResetTimer();

    if (g_cfg.safe_save) {
      SYSTEMTIME st;
      char tmpPath[MAX_PATH];
      FILE *f;
      GetLocalTime(&st);
      snprintf(tmpPath, MAX_PATH, "%s.tmp.%04d%02d%02d_%02d%02d%02d",
        g_curFile, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
      f = fopen(tmpPath, "wb");
      if (!f) { free(cipher); free(bytes); MessageBox(g_hWnd, "Cannot write temp file", "Error", MB_OK | MB_ICONERROR); return; }
#ifdef DEBUG_TRUNCATE_SAVE_CORRUPTION_CHECK
      fwrite(cipher, 1, cipherlen-1, f);
#else
      fwrite(cipher, 1, cipherlen, f);
#endif
      fclose(f);
      if (g_cfg.paranoid_save) {
        unsigned char *verify = (unsigned char *)malloc(cipherlen);
        FILE *fv = fopen(tmpPath, "rb");
        int ok = verify && fv && fread(verify, 1, cipherlen, fv) == cipherlen && memcmp(verify, cipher, cipherlen) == 0;
        if (fv) fclose(fv);
        free(verify);
        if (!ok) {
          char msg[MAX_PATH + 64];
          free(cipher); free(bytes);
          snprintf(msg, sizeof(msg), "Verify failed: temp file does not match source\n%s", tmpPath);
          MessageBox(g_hWnd, msg, "Error", MB_OK | MB_ICONERROR);
          return;
        }
      }
      if (!DeleteFile(g_curFile) && GetLastError() != ERROR_FILE_NOT_FOUND) {
        DeleteFile(tmpPath);
        free(cipher); free(bytes);
        MessageBox(g_hWnd, "Cannot delete original file", "Error", MB_OK | MB_ICONERROR);
        return;
      }
      if (!MoveFile(tmpPath, g_curFile)) {
        DeleteFile(tmpPath);
        free(cipher); free(bytes);
        MessageBox(g_hWnd, "Cannot rename temp file", "Error", MB_OK | MB_ICONERROR);
        return;
      }
      free(cipher);
    } else {
      FILE *f = fopen(g_curFile, "wb");
      if (!f) { free(cipher); free(bytes); MessageBox(g_hWnd, "Cannot write file", "Error", MB_OK | MB_ICONERROR); return; }
      fwrite(cipher, 1, cipherlen, f);
      fclose(f);
      free(cipher);
    }
  } else {
    if (g_cfg.safe_save) {
      SYSTEMTIME st;
      char tmpPath[MAX_PATH];
      FILE *f;
      GetLocalTime(&st);
      snprintf(tmpPath, MAX_PATH, "%s.tmp.%04d%02d%02d_%02d%02d%02d",
        g_curFile, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
      f = fopen(tmpPath, "wb");
      if (!f) { free(bytes); MessageBox(g_hWnd, "Cannot write temp file", "Error", MB_OK | MB_ICONERROR); return; }
#ifdef DEBUG_TRUNCATE_SAVE_CORRUPTION_CHECK
      fwrite(bytes, 1, blen-1, f);
#else
      fwrite(bytes, 1, blen, f);
#endif
      fclose(f);
      if (g_cfg.paranoid_save) {
        char *verify = (char *)malloc(blen);
        FILE *fv = fopen(tmpPath, "rb");
        int ok = verify && fv && fread(verify, 1, blen, fv) == (size_t)blen && memcmp(verify, bytes, blen) == 0;
        if (fv) fclose(fv);
        free(verify);
        if (!ok) {
          char msg[MAX_PATH + 64];
          free(bytes);
          snprintf(msg, sizeof(msg), "Verify failed: temp file does not match source\n%s", tmpPath);
          MessageBox(g_hWnd, msg, "Error", MB_OK | MB_ICONERROR);
          return;
        }
      }
      if (!DeleteFile(g_curFile) && GetLastError() != ERROR_FILE_NOT_FOUND) {
        DeleteFile(tmpPath);
        free(bytes);
        MessageBox(g_hWnd, "Cannot delete original file", "Error", MB_OK | MB_ICONERROR);
        return;
      }
      if (!MoveFile(tmpPath, g_curFile)) {
        DeleteFile(tmpPath);
        free(bytes);
        MessageBox(g_hWnd, "Cannot rename temp file", "Error", MB_OK | MB_ICONERROR);
        return;
      }
    } else {
      FILE *f = fopen(g_curFile, "wb");
      if (!f) { free(bytes); MessageBox(g_hWnd, "Cannot write file", "Error", MB_OK | MB_ICONERROR); return; }
      fwrite(bytes, 1, blen, f);
      fclose(f);
    }
  }
  free(bytes);
  g_dirty = FALSE;
  SendMessage(g_hEditor, EM_SETMODIFY, FALSE, 0);
  UpdateMenuSaveState(g_hWnd);
  UpdateTitle();
  RefreshTree();
}

static void SaveFileAs(void) {
  OPENFILENAME ofn;
  char file[MAX_PATH] = "";
  const char *dot;
  if (g_curFile[0]) strncpy(file, g_curFile, MAX_PATH - 1);

  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = g_hWnd;
  ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0Encrypted (*.chi)\0*.chi\0All Files (*.*)\0*.*\0";
  ofn.lpstrFile = file;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrInitialDir = g_curDir[0] ? g_curDir : NULL;
  ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

  if (GetSaveFileName(&ofn)) {
    dot = strrchr(file, '.');
    if (!dot || (_stricmp(dot, ".txt") && _stricmp(dot, ".chi") && _stricmp(dot, ".chs") && _stricmp(dot, ".md")))
      strncat(file, ".txt", MAX_PATH - strlen(file) - 1);
    strncpy(g_curFile, file, MAX_PATH - 1);
    g_curFile[MAX_PATH - 1] = '\0';
    SaveCurrentFile();
  }
}

static int PromptSave(void) {
  if (!g_dirty) return IDNO;
  {
    int r = MessageBox(g_hWnd, "Save changes?", "Tombo", MB_YESNOCANCEL | MB_ICONQUESTION);
    if (r == IDYES) SaveCurrentFile();
    return r;
  }
}

static void EncryptFileToDisk(const char *path) {
  FILE *f;
  long sz;
  unsigned char *buf, *cipher;
  size_t cipherlen;
  char pass[256];
  char chiPath[MAX_PATH];

  if (is_chi_file(path)) return;

  f = fopen(path, "rb");
  if (!f) { MessageBox(g_hWnd, "Cannot open file", "Error", MB_OK | MB_ICONERROR); return; }
  fseek(f, 0, SEEK_END);
  sz = ftell(f);
  rewind(f);
  buf = (unsigned char *)malloc(sz);
  if (!buf) { fclose(f); return; }
  fread(buf, 1, sz, f);
  fclose(f);

  if (!AskPassword(pass, sizeof(pass), 1)) { free(buf); return; }

  cipher = bf01_encrypt_mem(buf, sz, pass, &cipherlen, 0);
  free(buf);
  if (!cipher) { MessageBox(g_hWnd, "Encryption failed", "Error", MB_OK | MB_ICONERROR); return; }

  PasswordCache_Set(pass);
  PasswordCache_ResetTimer();

  strncpy(chiPath, path, MAX_PATH - 1);
  chiPath[MAX_PATH - 1] = '\0';
  {
    char *dot = strrchr(chiPath, '.');
    if (dot) *dot = '\0';
  }
  strncat(chiPath, ".chi", MAX_PATH - strlen(chiPath) - 1);

  if (g_cfg.safe_save) {
    SYSTEMTIME st;
    char tmpPath[MAX_PATH];
    FILE *ft;
    GetLocalTime(&st);
    snprintf(tmpPath, MAX_PATH, "%s.tmp.%04d%02d%02d_%02d%02d%02d",
      chiPath, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    ft = fopen(tmpPath, "wb");
    if (!ft) { free(cipher); MessageBox(g_hWnd, "Cannot write temp file", "Error", MB_OK | MB_ICONERROR); return; }
    fwrite(cipher, 1, cipherlen, ft);
    fclose(ft);
    if (!DeleteFileA(chiPath) && GetLastError() != ERROR_FILE_NOT_FOUND) {
      DeleteFileA(tmpPath);
      free(cipher);
      MessageBox(g_hWnd, "Cannot replace encrypted file", "Error", MB_OK | MB_ICONERROR);
      return;
    }
    if (!MoveFileA(tmpPath, chiPath)) {
      DeleteFileA(tmpPath);
      free(cipher);
      MessageBox(g_hWnd, "Cannot rename temp file", "Error", MB_OK | MB_ICONERROR);
      return;
    }
  } else {
    FILE *fo = fopen(chiPath, "wb");
    if (!fo) { free(cipher); MessageBox(g_hWnd, "Cannot write encrypted file", "Error", MB_OK | MB_ICONERROR); return; }
    fwrite(cipher, 1, cipherlen, fo);
    fclose(fo);
  }
  free(cipher);

  {
    char bakPath[MAX_PATH];
    snprintf(bakPath, MAX_PATH, "%s.bak", path);
    DeleteFileA(bakPath);
    MoveFileA(path, bakPath);
  }

  if (_stricmp(path, g_curFile) == 0) {
    g_curFile[0] = '\0';
    SetWindowTextW(g_hEditor, L"");
    if (g_editor_wtext) { free(g_editor_wtext); g_editor_wtext = NULL; }
    g_editor_wlen = 0;
    g_file_cp = 0;
    g_dirty = FALSE;
    UpdateMenuSaveState(g_hWnd);
    UpdateTitle();
    UpdateStatus();
  }

  RefreshTree();
}

static void DecryptFileToDisk(const char *path) {
  FILE *f;
  long sz;
  unsigned char *filedata, *plain;
  size_t plainlen;
  char pass[256];
  char txtPath[MAX_PATH];

  if (!is_chi_file(path)) return;

  f = fopen(path, "rb");
  if (!f) { MessageBox(g_hWnd, "Cannot open file", "Error", MB_OK | MB_ICONERROR); return; }
  fseek(f, 0, SEEK_END);
  sz = ftell(f);
  rewind(f);
  filedata = (unsigned char *)malloc(sz);
  if (!filedata) { fclose(f); return; }
  fread(filedata, 1, sz, f);
  fclose(f);

  if (!AskPassword(pass, sizeof(pass), 0)) { free(filedata); return; }

  plain = bf01_decrypt_mem(filedata, sz, pass, &plainlen, 0);
  free(filedata);
  if (!plain) {
    MessageBox(g_hWnd, "Decryption failed (wrong password?)", "Error", MB_OK | MB_ICONERROR);
    return;
  }

  PasswordCache_Set(pass);
  PasswordCache_ResetTimer();

  strncpy(txtPath, path, MAX_PATH - 1);
  txtPath[MAX_PATH - 1] = '\0';
  {
    char *dot = strrchr(txtPath, '.');
    if (dot) strcpy(dot, ".txt");
    else strncat(txtPath, ".txt", MAX_PATH - strlen(txtPath) - 1);
  }

  if (g_cfg.safe_save) {
    SYSTEMTIME st;
    char tmpPath[MAX_PATH];
    FILE *ft;
    GetLocalTime(&st);
    snprintf(tmpPath, MAX_PATH, "%s.tmp.%04d%02d%02d_%02d%02d%02d",
      txtPath, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    ft = fopen(tmpPath, "wb");
    if (!ft) { free(plain); MessageBox(g_hWnd, "Cannot write temp file", "Error", MB_OK | MB_ICONERROR); return; }
    fwrite(plain, 1, plainlen, ft);
    fclose(ft);
    if (!DeleteFileA(txtPath) && GetLastError() != ERROR_FILE_NOT_FOUND) {
      DeleteFileA(tmpPath);
      free(plain);
      MessageBox(g_hWnd, "Cannot replace text file", "Error", MB_OK | MB_ICONERROR);
      return;
    }
    if (!MoveFileA(tmpPath, txtPath)) {
      DeleteFileA(tmpPath);
      free(plain);
      MessageBox(g_hWnd, "Cannot rename temp file", "Error", MB_OK | MB_ICONERROR);
      return;
    }
  } else {
    FILE *fo = fopen(txtPath, "wb");
    if (!fo) { free(plain); MessageBox(g_hWnd, "Cannot write text file", "Error", MB_OK | MB_ICONERROR); return; }
    fwrite(plain, 1, plainlen, fo);
    fclose(fo);
  }
  free(plain);

  {
    char bakPath[MAX_PATH];
    snprintf(bakPath, MAX_PATH, "%s.bak", path);
    DeleteFileA(bakPath);
    MoveFileA(path, bakPath);
  }

  if (_stricmp(path, g_curFile) == 0) {
    g_curFile[0] = '\0';
    SetWindowTextW(g_hEditor, L"");
    if (g_editor_wtext) { free(g_editor_wtext); g_editor_wtext = NULL; }
    g_editor_wlen = 0;
    g_file_cp = 0;
    g_dirty = FALSE;
    UpdateMenuSaveState(g_hWnd);
    UpdateTitle();
    UpdateStatus();
  }

  RefreshTree();
}

static void UpdateTitle(void) {
  char title[512];
  if (g_curFile[0])
    snprintf(title, sizeof(title), "Tombo - %s%s", g_dirty ? "*" : "", g_curFile);
  else
    snprintf(title, sizeof(title), "Tombo - %sUntitled", g_dirty ? "*" : "");
  SetWindowTextA(g_hWnd, title);
}

static void UpdateStatus(void) {
  char status[256];
  if (g_curFile[0])
    snprintf(status, sizeof(status), "%s", g_curFile);
  else
    snprintf(status, sizeof(status), "No file");
  SendMessage(g_hStatus, SB_SETTEXT, 0, (LPARAM)status);
}

/* --- Password Dialog --- */

static int AskPassword(char *passBuf, int bufsize, int encrypt) {
  MSG msg;
  HWND hPass, hEdit, hEdit2, hOk, hCancel;
  HWND hLabel2;
  HFONT hDlgFont;
  RECT rc;

  if (PasswordCache_Get(passBuf, bufsize)) return 1;

  hDlgFont = CreateFont(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    DEFAULT_QUALITY, DEFAULT_PITCH, "MS Shell Dlg");

  GetWindowRect(g_hWnd, &rc);
  hPass = CreateWindowEx(WS_EX_DLGMODALFRAME, "PassDialog", "Password",
    WS_POPUP | WS_CAPTION | WS_SYSMENU,
    (rc.left + rc.right) / 2 - 120, (rc.top + rc.bottom) / 2 - 124,
    240, 220, g_hWnd, NULL, g_hInst, NULL);

  CreateWindow("STATIC", "Enter password:", WS_CHILD | WS_VISIBLE,
    10, 10, 200, 20, hPass, NULL, g_hInst, NULL);
  hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_PASSWORD | ES_AUTOHSCROLL,
    10, 35, 210, 24, hPass, (HMENU)IDC_PASS_EDIT, g_hInst, NULL);

  hLabel2 = CreateWindow("STATIC", "Confirm password:", WS_CHILD | WS_VISIBLE,
    10, 65, 200, 20, hPass, NULL, g_hInst, NULL);
  hEdit2 = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_PASSWORD | ES_AUTOHSCROLL,
    10, 85, 210, 24, hPass, (HMENU)IDC_PASS_EDIT2, g_hInst, NULL);

  CreateWindow("BUTTON", "Show password",
    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
    10, 118, 210, 20, hPass, (HMENU)IDC_PASS_SHOW, g_hInst, NULL);

  hOk = CreateWindow("BUTTON", "OK",
    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
    10, 148, 95, 28, hPass, (HMENU)IDC_PASS_OK, g_hInst, NULL);
  hCancel = CreateWindow("BUTTON", "Cancel",
    WS_CHILD | WS_VISIBLE | WS_TABSTOP,
    115, 148, 95, 28, hPass, (HMENU)IDC_PASS_CANCEL, g_hInst, NULL);

  SendMessage(hEdit, WM_SETFONT, (WPARAM)hDlgFont, TRUE);
  SendMessage(hLabel2, WM_SETFONT, (WPARAM)hDlgFont, TRUE);
  SendMessage(hEdit2, WM_SETFONT, (WPARAM)hDlgFont, TRUE);
  SendMessage(hOk, WM_SETFONT, (WPARAM)hDlgFont, TRUE);
  SendMessage(hCancel, WM_SETFONT, (WPARAM)hDlgFont, TRUE);

  if (!encrypt) {
    EnableWindow(hLabel2, FALSE);
    EnableWindow(hEdit2, FALSE);
  }

  SetWindowLongPtr(hPass, GWLP_USERDATA, (LONG_PTR)passBuf);
  SetWindowLongPtr(hPass, GWLP_HINSTANCE, (LONG_PTR)bufsize);

  g_passOk = 0;
  g_passEncrypt = encrypt;
  ShowWindow(hPass, SW_SHOW);
  SetFocus(hEdit);
  EnableWindow(g_hWnd, FALSE);

  while (GetMessage(&msg, NULL, 0, 0)) {
    if (!IsWindow(hPass)) break;
    if (IsDialogMessage(hPass, &msg)) continue;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  EnableWindow(g_hWnd, TRUE);
  SetForegroundWindow(g_hWnd);
  if (g_passOk) SetFocus(g_hTree);
  DeleteObject(hDlgFont);
  return g_passOk;
}

static LRESULT CALLBACK PassWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case DM_GETDEFID:
    return MAKELONG(IDC_PASS_OK, DC_HASDEFID);
  case WM_COMMAND:
    if (LOWORD(wParam) == IDC_PASS_OK) {
      char *dst = (char *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      int bufsize = (int)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
      GetDlgItemText(hWnd, IDC_PASS_EDIT, dst, bufsize);
      if (g_passEncrypt) {
        char confirm[256] = "";
        GetDlgItemText(hWnd, IDC_PASS_EDIT2, confirm, sizeof(confirm));
        if (strcmp(dst, confirm) != 0) {
          MessageBox(hWnd, "Passwords do not match", "Error", MB_OK | MB_ICONERROR);
          SetFocus(GetDlgItem(hWnd, IDC_PASS_EDIT));
          return 0;
        }
      }
      g_passOk = 1;
      DestroyWindow(hWnd);
      return 0;
    }
    if (LOWORD(wParam) == IDC_PASS_CANCEL || LOWORD(wParam) == IDCANCEL) {
      DestroyWindow(hWnd);
      return 0;
    }
    if (LOWORD(wParam) == IDC_PASS_SHOW && HIWORD(wParam) == BN_CLICKED) {
      HWND hEdit1 = GetDlgItem(hWnd, IDC_PASS_EDIT);
      HWND hEdit2 = GetDlgItem(hWnd, IDC_PASS_EDIT2);
      int show = (int)SendMessage(GetDlgItem(hWnd, IDC_PASS_SHOW), BM_GETCHECK, 0, 0);
      WPARAM ch = (show == BST_CHECKED) ? 0 : '*';
      SendMessage(hEdit1, EM_SETPASSWORDCHAR, ch, 0);
      InvalidateRect(hEdit1, NULL, TRUE);
      SendMessage(hEdit2, EM_SETPASSWORDCHAR, ch, 0);
      InvalidateRect(hEdit2, NULL, TRUE);
      return 0;
    }
    break;
  case WM_KEYDOWN:
    if (wParam == VK_RETURN) {
      SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_PASS_OK, BN_CLICKED), 0);
      return 0;
    }
    if (wParam == VK_ESCAPE) {
      SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_PASS_CANCEL, BN_CLICKED), 0);
      return 0;
    }
    break;
  case WM_CLOSE:
    DestroyWindow(hWnd);
    return 0;
  }
  return DefWindowProc(hWnd, msg, wParam, lParam);
}

/* --- Find Dialog --- */

static LRESULT CALLBACK FindWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  (void)lParam;
  switch (msg) {
  case WM_CREATE: {
    HFONT hDlgFont = CreateFont(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
      DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
      DEFAULT_QUALITY, DEFAULT_PITCH, "MS Shell Dlg");
    CreateWindow("STATIC", "Find:", WS_CHILD | WS_VISIBLE,
      10, 12, 30, 20, hWnd, NULL, g_hInst, NULL);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", g_findText,
      WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
      45, 10, 180, 24, hWnd, (HMENU)IDC_FIND_EDIT, g_hInst, NULL);
    CreateWindow("BUTTON", "Next",
      WS_CHILD | WS_VISIBLE | WS_TABSTOP,
      235, 10, 50, 24, hWnd, (HMENU)IDC_FIND_NEXT, g_hInst, NULL);
    CreateWindow("BUTTON", "Prev",
      WS_CHILD | WS_VISIBLE | WS_TABSTOP,
      235, 40, 50, 24, hWnd, (HMENU)IDC_FIND_PREV, g_hInst, NULL);
    CreateWindow("BUTTON", "Close",
      WS_CHILD | WS_VISIBLE | WS_TABSTOP,
      235, 70, 50, 24, hWnd, (HMENU)IDC_FIND_CLOSE, g_hInst, NULL);
    {
      HWND hChild = GetWindow(hWnd, GW_CHILD);
      while (hChild) {
        SendMessage(hChild, WM_SETFONT, (WPARAM)hDlgFont, TRUE);
        hChild = GetWindow(hChild, GW_HWNDNEXT);
      }
    }
    DeleteObject(hDlgFont);
    return 0;
  }
  case WM_COMMAND:
    if (LOWORD(wParam) == IDC_FIND_NEXT || LOWORD(wParam) == IDC_FIND_PREV) {
      FINDTEXTW ft;
      LONG selStart, selEnd;
      LONG pos;
      GetDlgItemTextW(hWnd, IDC_FIND_EDIT, g_findText, sizeof(g_findText)/sizeof(g_findText[0]));
      if (!g_findText[0]) return 0;
      SendMessage(g_hEditor, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
      if (LOWORD(wParam) == IDC_FIND_NEXT) {
        ft.chrg.cpMin = selEnd;
        ft.chrg.cpMax = -1;
      } else {
        ft.chrg.cpMin = selStart;
        ft.chrg.cpMax = 0;
      }
      ft.lpstrText = g_findText;
      if ((pos = (LONG)SendMessageW(g_hEditor, EM_FINDTEXTW,
            LOWORD(wParam) == IDC_FIND_PREV ? FR_DOWN : 0, (LPARAM)&ft)) >= 0)
        SendMessage(g_hEditor, EM_SETSEL, pos, pos + (int)wcslen(g_findText));
      else
        MessageBoxW(hWnd, L"Not found", L"Find", MB_OK);
      return 0;
    }
    if (LOWORD(wParam) == IDC_FIND_CLOSE || LOWORD(wParam) == IDCANCEL) {
      DestroyWindow(hWnd);
      return 0;
    }
    break;
  case WM_CLOSE:
    DestroyWindow(hWnd);
    return 0;
  case WM_DESTROY:
    g_hFindDlg = NULL;
    break;
  }
  return DefWindowProc(hWnd, msg, wParam, lParam);
}
