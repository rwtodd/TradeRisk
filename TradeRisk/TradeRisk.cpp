// TradeRisk.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TradeRisk.h"
#include "trading.h"
#include <sstream>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
rwt::price_ladder ladder;						// hold trade info
int avg_char_width;								// from metrics
int char_height;								// from metrics


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass (HINSTANCE hInstance);
BOOL                InitInstance (HINSTANCE, int);
LRESULT CALLBACK    WndProc (HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About (HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain (_In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPWSTR    lpCmdLine,
					   _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER (hPrevInstance);
	UNREFERENCED_PARAMETER (lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW (hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW (hInstance, IDC_TRADERISK, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass (hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators (hInstance, MAKEINTRESOURCE (IDC_TRADERISK));

	MSG msg;

	// Main message loop:
	while (GetMessage (&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator (msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass (HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof (WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_TRADERISK));
	wcex.hCursor = LoadCursor (nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW (IDC_TRADERISK);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon (wcex.hInstance, MAKEINTRESOURCE (IDI_SMALL));

	return RegisterClassExW (&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance (HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW (szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_VSCROLL,
							   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow (hWnd, nCmdShow);
	UpdateWindow (hWnd);

	return TRUE;
}

COLORREF background_for_r (double r)
{
	COLORREF ans;
	if (r < -1.0) { ans = RGB (128, 64, 50); }
	else if (r < 0) { ans = RGB (221, 126, 107); }
	else if (r < 0.5) { ans = RGB (255, 229, 153); }
	else if (r < 2) { ans = RGB (217, 234, 211); }
	else if (r < 4) { ans = RGB (182, 215, 168); }
	else if (r < 6) { ans = RGB (147, 196, 125); }
	else if (r < 8) { ans = RGB (106, 168, 79); }
	else { ans = RGB (68, 145, 35); }
	return ans;
}

// a little helper class which caches a brush until the colorref
// given to it changes.
class cached_brush
{
private:
	COLORREF cvalue;
	HBRUSH brush;
public:
	cached_brush () : cvalue{ 0 }, brush{ CreateSolidBrush (0) } {}
	~cached_brush () { if (brush != nullptr) DeleteObject (brush); }

	HBRUSH set_color (COLORREF c)
	{
		if (cvalue != c)
		{
			DeleteObject (brush);
			cvalue = c;
			brush = CreateSolidBrush (cvalue);
		}
		return brush;
	}

	HBRUSH operator*() { return brush; }
};

void paint_ladder (HWND hWnd)
{
	PAINTSTRUCT ps;
	rwt::price_description pd;
	wchar_t buffer[20];
	int bufflen;
	const int line_width = 35 * avg_char_width;

	HDC hdc = BeginPaint (hWnd, &ps);
	{
		int idxEnd = ladder.steps ();
		SetTextAlign (hdc, TA_RIGHT | TA_TOP);
		SetBkMode (hdc, TRANSPARENT);
		cached_brush bkbrush;

		for (int i = 0; i < idxEnd; ++i)
		{
			int y = i * (char_height + 1);
			ladder.describe (i, pd);
			bkbrush.set_color (background_for_r (pd.risk_multiple));
			RECT line_rect{ 0, y, line_width, y + char_height };
			FillRect (hdc, &line_rect, *bkbrush);

			// print the size, if there is one...
			if (pd.shares != 0)
			{
				bufflen = swprintf (buffer, 20, L"%2d", pd.shares);
				TextOut (hdc, 5 * avg_char_width, y, buffer, bufflen);
			}

			// print the price level...
			bufflen = swprintf (buffer, 20, L"%5.2f", pd.price);
			TextOut (hdc, 15 * avg_char_width, y, buffer, bufflen);

			// print the PnL
			bufflen = swprintf (buffer, 20, L"%5.2f", pd.profit);
			TextOut (hdc, 25 * avg_char_width, y, buffer, bufflen);

			// print the R-value
			bufflen = swprintf (buffer, 20, L"%5.1f", pd.risk_multiple);
			TextOut (hdc, 35 * avg_char_width, y, buffer, bufflen);

			MoveToEx (hdc, 0, y + char_height, nullptr);
			LineTo (hdc, line_width, y + char_height);
		}
	}
	EndPaint (hWnd, &ps);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD (wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox (hInst, MAKEINTRESOURCE (IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow (hWnd);
			break;
		default:
			return DefWindowProc (hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_CREATE:
	{
		TEXTMETRIC tm;
		HDC hdc = GetDC (hWnd);
		GetTextMetrics (hdc, &tm);
		avg_char_width = tm.tmAveCharWidth;
		char_height = tm.tmExternalLeading + tm.tmHeight;
		ReleaseDC (hWnd, hdc);
	}
	break;
	case WM_PAINT:
		paint_ladder (hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage (0);
		break;
	default:
		return DefWindowProc (hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER (lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD (wParam) == IDOK || LOWORD (wParam) == IDCANCEL)
		{
			EndDialog (hDlg, LOWORD (wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
