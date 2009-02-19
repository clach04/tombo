#ifndef TOMBO_H
#define TOMBO_H

class PasswordManager;

///////////////////////////////////////
// ウィンドウメッセージ
///////////////////////////////////////

// MWM_OPEN_REQUEST : メモのオープン要求
// WPARAM : オプション
// LPARAM : オープンされるべきノードへのポインタ
// #define MWM_OPEN_REQUEST (0x8000 + 1)

// MWM_OPEN_REQUEST オプション
#define OPEN_REQUEST_MDVIEW_ACTIVE 0
#define OPEN_REQUEST_MSVIEW_ACTIVE 1

// MWM_RAISE_MAINFRAME : トップレベルウィンドウをRaiseして再前面にもってくる
// WPARAM, LPARAM: 0
#define MWM_RAISE_MAINFRAME (0x8000 + 2)

// MWM_SWITCH_VIEW : switch focus
#define MWM_SWITCH_VIEW (0x8000 + 3)

///////////////////////////////////////
// 共通定義
///////////////////////////////////////


#define TOMBO_MAIN_FRAME_WINDOW_CLSS TEXT("TomboMainFrmWnd")

#define TOMBO_APP_NAME TEXT("Tombo")

// 文字列リソースの最大長
#define MESSAGE_MAX_SIZE 1024

// リストビューが持つイメージリスト
#define IMG_FOLDER 0
#define IMG_FOLDER_SEL 1
#define IMG_ARTICLE 2
#define IMG_ARTICLE_ENCRYPTED 3
#define IMG_ARTICLE_MASKED 4
#define IMG_ARTICLE_ENC_MASKED 5
#define IMG_FOLDER_MASKED 6
#define IMG_FOLDER_SEL_MASKED 7
#define IMG_VFOLDER 8
#define IMG_VFOLDER_SEL 9

extern HINSTANCE g_hInstance;

#include "Logger.h"
extern Logger *g_pLogger;

int TomboMessageBox(HWND hWnd, LPCTSTR pText, LPCTSTR pCaption, UINT uType); 
extern BOOL bDisableHotKey;

extern PasswordManager *g_pPasswordManager;

// application button defintions
#define APP_BUTTON1 (0xC1)
#define APP_BUTTON2 (0xC2)
#define APP_BUTTON3 (0xC3)
#define APP_BUTTON4 (0xC4)
#define APP_BUTTON5 (0xC5)

#define APPBUTTON_ACTION_DISABLE 0
#define APPBUTTON_ACTION_ENABLE  1

// Virtual folder definition file name
// path is defind by user(Property).
#define TOMBO_VFOLDER_DEF_FILE TEXT("vfolder.xml")

// message definition file
#define TOMBO_MSG_DEF_FILE TEXT("TomboMsg.txt")

// Number of preserving history about search string.
#define NUM_SEARCH_HISTORY 10

enum SearchResult {
	SR_FOUND,
	SR_NOTFOUND,
	SR_FAILED,
	SR_CANCELED
};

// execution type
enum ExeAppType {
	ExecType_Assoc,
	ExecType_ExtApp1,
	ExecType_ExtApp2,
};

//
#define VFOLDER_IMAGE_PERSIST	8
#define VFOLDER_IMAGE_TEMP		6

#ifdef STRICT 
typedef WNDPROC SUPER_WND_PROC;
#else 
typedef FARPROC SUPER_WND_PROC;
#endif 

#define NUM_RETRY_INVALID_PASSWORD 3

#define VFOLDER_REPO_NAME TEXT("@vfolder")
#endif
