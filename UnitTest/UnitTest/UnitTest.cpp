#include "stdafx.h"
#include "UnitTest.h"
#include "TestRunner.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];

HWND hStartButton = NULL;
HWND hResultBox = NULL;
TestRunner testRunner;

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_UNITTEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_UNITTEST));

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_UNITTEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	LPCREATESTRUCT pCS = (LPCREATESTRUCT)lParam;

	RECT r;
	GetClientRect(hWnd, &r);

	hStartButton = CreateWindow(TEXT("BUTTON"), TEXT("Start"),
					WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
					r.left, r.top, r.right - r.left, 50,
					hWnd, (HMENU)ID_RUN, hInst, NULL);
	hResultBox = CreateWindow(TEXT("EDIT"), NULL,
					WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
					r.left, r.top + 50, r.right - r.left, r.bottom - r.top - 50,
					hWnd, NULL, hInst, NULL);
	return 0;
}

LRESULT OnResize(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	RECT r;
	GetClientRect(hWnd, &r);

	if (hStartButton == NULL || hResultBox == NULL) return 0;
	MoveWindow(hStartButton, r.left, r.top, r.right - r.left, 50, TRUE);
	MoveWindow(hResultBox, r.left, r.top + 50, r.right - r.left, r.bottom - r.top - 50, TRUE);
	return 0;
}

void OnRunTest(HWND hWnd) {
	HMENU hMenu = GetMenu(hWnd);
	EnableMenuItem(hMenu, ID_RUN, MF_DISABLED | MF_GRAYED);

	EnableWindow(hStartButton, FALSE);
	SendMessage(hResultBox, WM_SETTEXT, 0, (LPARAM)TEXT(""));
	testRunner.run(hWnd);
}

void OnTestFinished(HWND hWnd) {
	HMENU hMenu = GetMenu(hWnd);
	EnableMenuItem(hMenu, ID_RUN, MF_ENABLED);

	EnableWindow(hStartButton, TRUE);
}

void OnWriteMsg(HWND hWnd, WPARAM wParam, LPARAM lParam) {
	SendMessage(hResultBox, EM_REPLACESEL, 0, lParam);
	delete[] (LPTSTR)lParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId, wmEvent;

	switch (message) {
	case WM_CREATE:
		return OnCreate(hWnd, wParam, lParam);
	case WM_SIZE:
		return OnResize(hWnd, wParam, lParam);
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId) {
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_RUN:
			OnRunTest(hWnd);
			break;
		case IDM_APP_TEST_FINISHED:
			OnTestFinished(hWnd);
			break;
		case IDM_APP_WRITE_RESULT:
			OnWriteMsg(hWnd, wParam, lParam);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

