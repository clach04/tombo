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
#define IDM_ABOUT    1030

#define ID_TREE      2001
#define ID_EDITOR    2002
#define ID_STATUS    2003

#define IDC_PASS_EDIT    3001
#define IDC_PASS_CONFIRM 3002
#define IDC_PASS_OK      3003
#define IDC_PASS_CANCEL  3004

#define IDC_FIND_EDIT  3101
#define IDC_FIND_NEXT  3102
#define IDC_FIND_PREV  3103
#define IDC_FIND_CLOSE 3104

static HINSTANCE g_hInst;
static HWND g_hWnd;
static HWND g_hTree, g_hEditor, g_hStatus;
static HWND g_hFindDlg;
static int g_passOk;
static AppConfig g_cfg;
static char g_curFile[MAX_PATH];
static char g_curDir[MAX_PATH];
static BOOL g_dirty;
static HFONT g_hFont;
static char g_findText[256];

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK PassWndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK FindWndProc(HWND, UINT, WPARAM, LPARAM);
static void PopulateTree(HWND hTree, const char *dir, HTREEITEM hParent);
static void RefreshTree(void);
static void TomboOpenFile(const char *path);
static void SaveCurrentFile(void);
static void SaveFileAs(void);
static int PromptSave(void);
static void UpdateTitle(void);
static void UpdateStatus(void);
static void SetEditorFont(HWND hEd);
static int AskPassword(char *passBuf, int bufsize);

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

  g_hWnd = CreateWindowEx(0, "TomboClass", "Tombo",
    WS_OVERLAPPEDWINDOW, g_cfg.win_x, g_cfg.win_y,
    g_cfg.win_w, g_cfg.win_h,
    NULL, NULL, hInst, NULL);

  {
    RECT wrc;
    POINT pt = { g_cfg.win_x + (g_cfg.win_w / 2), g_cfg.win_y + (GetSystemMetrics(SM_CYCAPTION) / 2) };
    if (!MonitorFromPoint(pt, MONITOR_DEFAULTTONULL)) {
      SetWindowPos(g_hWnd, NULL, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
        SWP_NOSIZE | SWP_NOZORDER);
    }
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

static HWND CreateEditor(HWND hParent, int wrap) {
  DWORD style = WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL;
  if (!wrap) style |= WS_HSCROLL | ES_AUTOHSCROLL;
  return CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
    style, 0, 0, 100, 100, hParent, (HMENU)ID_EDITOR, g_hInst, NULL);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_CREATE: {
    HMENU hMenu = CreateMenu();
    HMENU hFile = CreatePopupMenu();
    HMENU hEdit = CreatePopupMenu();
    HMENU hView = CreatePopupMenu();
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

    AppendMenu(hHelp, MF_STRING, IDM_ABOUT, "&About");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelp, "&Help");
    SetMenu(hWnd, hMenu);
    InitMenu(hWnd);

    g_hTree = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, "",
      WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS,
      0, 0, 200, 400, hWnd, (HMENU)ID_TREE, g_hInst, NULL);

    g_hEditor = CreateEditor(hWnd, 0);

    g_hStatus = CreateWindowEx(0, STATUSCLASSNAME, "",
      WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
      0, 0, 0, 0, hWnd, (HMENU)ID_STATUS, g_hInst, NULL);

    SetEditorFont(g_hEditor);
    SendMessage(g_hEditor, EM_SETLIMITTEXT, 0, 0);

    if (g_cfg.last_dir[0]) strncpy(g_curDir, g_cfg.last_dir, MAX_PATH - 1);
    else GetCurrentDirectory(MAX_PATH, g_curDir);
    RefreshTree();
    SetFocus(g_hTree);
    return 0;
  }

  case WM_SIZE: {
    RECT rc;
    int w = LOWORD(lParam), h = HIWORD(lParam);
    int treeW = 200;
    int statusH = 0;
    HDWP hdwp;

    SendMessage(g_hStatus, WM_SIZE, 0, 0);
    GetWindowRect(g_hStatus, &rc);
    statusH = rc.bottom - rc.top;

    hdwp = BeginDeferWindowPos(3);
    DeferWindowPos(hdwp, g_hTree, NULL, 0, 0, treeW, h - statusH, SWP_NOZORDER);
    DeferWindowPos(hdwp, g_hEditor, NULL, treeW, 0, w - treeW, h - statusH, SWP_NOZORDER);
    DeferWindowPos(hdwp, g_hStatus, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
    EndDeferWindowPos(hdwp);
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
        SetWindowText(g_hEditor, "");
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
      FINDTEXTA ft;
      LONG selEnd;
      LONG pos;
      SendMessage(g_hEditor, EM_GETSEL, 0, (WPARAM)&selEnd);
      ft.chrg.cpMin = selEnd;
      ft.chrg.cpMax = -1;
      ft.lpstrText = g_findText;
      if (g_findText[0] && (pos = (LONG)SendMessage(g_hEditor, EM_FINDTEXT, 0, (LPARAM)&ft)) >= 0)
        SendMessage(g_hEditor, EM_SETSEL, pos, pos + (LONG)strlen(g_findText));
      break;
    }
    case IDM_FINDPREV: {
      FINDTEXTA ft;
      LONG selStart;
      LONG pos;
      SendMessage(g_hEditor, EM_GETSEL, (WPARAM)&selStart, 0);
      ft.chrg.cpMin = selStart;
      ft.chrg.cpMax = 0;
      ft.lpstrText = g_findText;
      if (g_findText[0] && (pos = (LONG)SendMessage(g_hEditor, EM_FINDTEXT, FR_DOWN, (LPARAM)&ft)) >= 0)
        SendMessage(g_hEditor, EM_SETSEL, pos, pos + (LONG)strlen(g_findText));
      break;
    }
    case IDM_WORDWRAP: {
      int len = GetWindowTextLength(g_hEditor);
      char *text = NULL;
      RECT edRc;
      HWND hOld = g_hEditor;
      if (len > 0) {
        text = (char *)malloc(len + 1);
        if (text) GetWindowText(g_hEditor, text, len + 1);
      }
      GetWindowRect(g_hEditor, &edRc);
      MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&edRc, 2);
      g_cfg.word_wrap = !g_cfg.word_wrap;
      g_hEditor = CreateEditor(hWnd, g_cfg.word_wrap);
      SetWindowPos(g_hEditor, NULL, edRc.left, edRc.top,
        edRc.right - edRc.left, edRc.bottom - edRc.top, SWP_NOZORDER);
      SetEditorFont(g_hEditor);
      SendMessage(g_hEditor, EM_SETLIMITTEXT, 0, 0);
      if (text) {
        SetWindowText(g_hEditor, text);
        free(text);
      }
      DestroyWindow(hOld);
      SetFocus(g_hEditor);
      CheckMenuItem(GetMenu(hWnd), IDM_WORDWRAP,
        g_cfg.word_wrap ? MF_CHECKED : MF_UNCHECKED);
      break;
    }
    case IDM_ABOUT:
      MessageBox(hWnd, "Tombo C99 - Plain text editor with encryption",
        "About Tombo", MB_OK | MB_ICONINFORMATION);
      break;
    }
    return 0;

  case WM_NOTIFY: {
    NMHDR *nm = (NMHDR *)lParam;
    if (nm->idFrom == ID_TREE && nm->code == NM_DBLCLK) {
      HTREEITEM hSel = TreeView_GetSelection(g_hTree);
      if (hSel) {
        TVITEM ti;
        char path[MAX_PATH];
        ZeroMemory(&ti, sizeof(ti));
        ti.mask = TVIF_PARAM | TVIF_TEXT;
        ti.hItem = hSel;
        ti.pszText = path;
        ti.cchTextMax = MAX_PATH;
        TreeView_GetItem(g_hTree, &ti);
        if (ti.lParam && PromptSave() != IDCANCEL) TomboOpenFile((const char *)ti.lParam);
      }
    }
    if (nm->idFrom == ID_TREE && nm->code == TVN_KEYDOWN) {
      NMTVKEYDOWN *kd = (NMTVKEYDOWN *)lParam;
      if (kd->wVKey == VK_RETURN) {
        HTREEITEM hSel = TreeView_GetSelection(g_hTree);
        if (hSel) {
          TVITEM ti;
          char path[MAX_PATH];
          ZeroMemory(&ti, sizeof(ti));
          ti.mask = TVIF_PARAM | TVIF_TEXT;
          ti.hItem = hSel;
          ti.pszText = path;
          ti.cchTextMax = MAX_PATH;
          TreeView_GetItem(g_hTree, &ti);
          if (ti.lParam) {
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
    return 0;
  }

  case WM_GETMINMAXINFO: {
    MINMAXINFO *mmi = (MINMAXINFO *)lParam;
    mmi->ptMinTrackSize.x = 400;
    mmi->ptMinTrackSize.y = 300;
    return 0;
  }

  case WM_CLOSE:
    if (PromptSave() == IDCANCEL) return 0;
    {
      RECT rc;
      GetWindowRect(hWnd, &rc);
      g_cfg.win_x = rc.left; g_cfg.win_y = rc.top;
      g_cfg.win_w = rc.right - rc.left; g_cfg.win_h = rc.bottom - rc.top;
      config_save(&g_cfg, CFG_PATH);
    }
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

  snprintf(pattern, MAX_PATH, "%s\\*", dir);
  hFind = FindFirstFile(pattern, &fd);
  if (hFind == INVALID_HANDLE_VALUE) return;

  do {
    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      if (fd.cFileName[0] == '.') continue;
      snprintf(childPath, MAX_PATH, "%s\\%s", dir, fd.cFileName);

      ZeroMemory(&tvi, sizeof(tvi));
      tvi.hParent = hParent;
      tvi.hInsertAfter = TVI_SORT;
      tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
      tvi.item.pszText = fd.cFileName;
      tvi.item.iImage = 0;
      tvi.item.iSelectedImage = 0;
      tvi.item.lParam = 0;

      {
        HTREEITEM hItem = TreeView_InsertItem(hTree, &tvi);
        PopulateTree(hTree, childPath, hItem);
      }
    } else if (is_tombo_ext(fd.cFileName)) {
      char *fullPath = (char *)malloc(MAX_PATH);
      if (!fullPath) continue;
      snprintf(fullPath, MAX_PATH, "%s\\%s", dir, fd.cFileName);

      ZeroMemory(&tvi, sizeof(tvi));
      tvi.hParent = hParent;
      tvi.hInsertAfter = TVI_SORT;
      tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
      tvi.item.pszText = fd.cFileName;
      tvi.item.iImage = 1;
      tvi.item.iSelectedImage = 1;
      tvi.item.lParam = (LPARAM)fullPath;
      TreeView_InsertItem(hTree, &tvi);
    }
  } while (FindNextFile(hFind, &fd));
  FindClose(hFind);
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

/* --- File I/O --- */

static int is_chi_file(const char *path) {
  const char *dot = strrchr(path, '.');
  return dot && (!_stricmp(dot, ".chi") || !_stricmp(dot, ".chs"));
}

static long strip_cr(char *buf, long len) {
  long w = 0;
  long r;
  for (r = 0; r < len; r++) {
    if (buf[r] != '\r') buf[w++] = buf[r];
  }
  return w;
}

static char *expand_lf(const char *src, long srclen, long *dstlen) {
  long i, count = 0;
  char *dst;
  for (i = 0; i < srclen; i++)
    count += (src[i] == '\n') ? 2 : 1;
  dst = (char *)malloc(count + 1);
  if (!dst) { *dstlen = 0; return NULL; }
  {
    long w = 0;
    for (i = 0; i < srclen; i++) {
      if (src[i] == '\n') dst[w++] = '\r';
      dst[w++] = src[i];
    }
    dst[w] = '\0';
    *dstlen = w;
  }
  return dst;
}

static void TomboOpenFile(const char *path) {
  if (is_chi_file(path)) {
    char pass[256] = "";
    FILE *fin;
    unsigned char *filedata;
    long filesize;
    unsigned char *plain;
    size_t plainlen;

    if (!AskPassword(pass, sizeof(pass)))
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
    {
      char *exp;
      long explen;
      exp = expand_lf((const char *)plain, (long)plainlen, &explen);
      SetWindowText(g_hEditor, exp ? exp : (const char *)plain);
      free(exp);
    }
    free(plain);
  } else {
    FILE *f = fopen(path, "rb");
    long sz;
    char *buf;
    if (!f) { MessageBox(g_hWnd, "Cannot open file", "Error", MB_OK | MB_ICONERROR); return; }
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    rewind(f);
    buf = (char *)malloc(sz + 1);
    if (buf) {
      char *exp;
      long explen;
      fread(buf, 1, sz, f);
      buf[sz] = '\0';
      exp = expand_lf(buf, sz, &explen);
      SetWindowText(g_hEditor, exp ? exp : buf);
      free(exp);
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
  char *buf;
  long len;
  if (!g_curFile[0]) { SaveFileAs(); return; }

  len = GetWindowTextLength(g_hEditor);
  buf = (char *)malloc(len + 1);
  if (!buf) return;
  GetWindowText(g_hEditor, buf, len + 1);

  if (is_chi_file(g_curFile)) {
    char pass[256] = "";
    unsigned char *cipher;
    size_t cipherlen;
    FILE *f;
    if (!AskPassword(pass, sizeof(pass))) {
      free(buf);
      return;
    }
    len = strip_cr(buf, len);
    cipher = bf01_encrypt_mem((unsigned char *)buf, len, pass, &cipherlen, 0);
    if (!cipher) {
      MessageBox(g_hWnd, "Encryption failed", "Error", MB_OK | MB_ICONERROR);
      free(buf);
      return;
    }
    f = fopen(g_curFile, "wb");
    if (!f) { free(cipher); free(buf); MessageBox(g_hWnd, "Cannot write file", "Error", MB_OK | MB_ICONERROR); return; }
    fwrite(cipher, 1, cipherlen, f);
    fclose(f);
    free(cipher);
  } else {
    FILE *f = fopen(g_curFile, "wb");
    if (!f) { free(buf); MessageBox(g_hWnd, "Cannot write file", "Error", MB_OK | MB_ICONERROR); return; }
    len = strip_cr(buf, len);
    fwrite(buf, 1, len, f);
    fclose(f);
  }
  free(buf);
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

static void UpdateTitle(void) {
  char title[512];
  if (g_curFile[0])
    snprintf(title, sizeof(title), "Tombo - %s%s", g_dirty ? "*" : "", g_curFile);
  else
    snprintf(title, sizeof(title), "Tombo - %sUntitled", g_dirty ? "*" : "");
  SetWindowText(g_hWnd, title);
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

static int AskPassword(char *passBuf, int bufsize) {
  MSG msg;
  HWND hPass, hEdit, hOk, hCancel;
  HFONT hDlgFont;
  RECT rc;

  hDlgFont = CreateFont(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    DEFAULT_QUALITY, DEFAULT_PITCH, "MS Shell Dlg");

  GetWindowRect(g_hWnd, &rc);
  hPass = CreateWindowEx(WS_EX_DLGMODALFRAME, "PassDialog", "Password",
    WS_POPUP | WS_CAPTION | WS_SYSMENU,
    (rc.left + rc.right) / 2 - 120, (rc.top + rc.bottom) / 2 - 50,
    240, 120, g_hWnd, NULL, g_hInst, NULL);

  CreateWindow("STATIC", "Enter password:", WS_CHILD | WS_VISIBLE,
    10, 10, 200, 20, hPass, NULL, g_hInst, NULL);
  hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_PASSWORD | ES_AUTOHSCROLL,
    10, 35, 210, 24, hPass, (HMENU)IDC_PASS_EDIT, g_hInst, NULL);
  hOk = CreateWindow("BUTTON", "OK",
    WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
    10, 70, 95, 28, hPass, (HMENU)IDC_PASS_OK, g_hInst, NULL);
  hCancel = CreateWindow("BUTTON", "Cancel",
    WS_CHILD | WS_VISIBLE | WS_TABSTOP,
    115, 70, 95, 28, hPass, (HMENU)IDC_PASS_CANCEL, g_hInst, NULL);

  SendMessage(hEdit, WM_SETFONT, (WPARAM)hDlgFont, TRUE);
  SendMessage(hOk, WM_SETFONT, (WPARAM)hDlgFont, TRUE);
  SendMessage(hCancel, WM_SETFONT, (WPARAM)hDlgFont, TRUE);

  SetWindowLongPtr(hPass, GWLP_USERDATA, (LONG_PTR)passBuf);
  SetWindowLongPtr(hPass, GWLP_HINSTANCE, (LONG_PTR)bufsize);

  g_passOk = 0;
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
      g_passOk = 1;
      DestroyWindow(hWnd);
      return 0;
    }
    if (LOWORD(wParam) == IDC_PASS_CANCEL || LOWORD(wParam) == IDCANCEL) {
      DestroyWindow(hWnd);
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
    CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", g_findText,
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
      FINDTEXTA ft;
      LONG selStart, selEnd;
      LONG pos;
      GetDlgItemText(hWnd, IDC_FIND_EDIT, g_findText, sizeof(g_findText));
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
      if ((pos = (LONG)SendMessage(g_hEditor, EM_FINDTEXT,
            LOWORD(wParam) == IDC_FIND_PREV ? FR_DOWN : 0, (LPARAM)&ft)) >= 0)
        SendMessage(g_hEditor, EM_SETSEL, pos, pos + (LONG)strlen(g_findText));
      else
        MessageBox(hWnd, "Not found", "Find", MB_OK);
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
