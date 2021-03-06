// TradeRisk.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "TradeRisk.h"
#include "trading.h"

#define MAX_LOADSTRING 100

// Global Variables:
static HINSTANCE hInst;                                // current instance
static WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
static WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
static rwt::price_ladder ladder;						// hold trade info
static int avg_char_width;								// from metrics
static int char_height;								// from metrics
#define line_height (char_height + 1)
static int screen_height;
static int top_visible_index;   // the last time we painted, what was the topmost visible line

// Forward declarations of functions included in this code module:
static ATOM                MyRegisterClass (HINSTANCE hInstance);
static BOOL                InitInstance (HINSTANCE, int);
static LRESULT CALLBACK    WndProc (HWND, UINT, WPARAM, LPARAM);
static INT_PTR CALLBACK    About (HWND, UINT, WPARAM, LPARAM);
static INT_PTR CALLBACK    SetRisk (HWND, UINT, WPARAM, LPARAM);
static INT_PTR CALLBACK    SetCenter (HWND, UINT, WPARAM, LPARAM);
static INT_PTR CALLBACK    InstrumentSettings (HWND, UINT, WPARAM, LPARAM);

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
static ATOM MyRegisterClass (HINSTANCE hInstance)
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
	wcex.hIconSm = LoadIcon (wcex.hInstance, MAKEINTRESOURCE (IDI_TRADERISK));

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
static BOOL InitInstance (HINSTANCE hInstance, int nCmdShow)
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

static COLORREF background_for_r (double r)
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

// resets the scrollbar info when the
// number of lines changes...
static void set_scrollinfo (HWND hWnd)
{
	SCROLLINFO si;
	si.cbSize = sizeof (si);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = 0;
	si.nMax = ladder.steps () - 1;
	si.nPage = screen_height / line_height;
	si.nPos = (ladder.steps () - si.nPage)/ 2;
	SetScrollInfo (hWnd, SB_VERT, &si, TRUE);
	top_visible_index = si.nPos;
}

// convert a client Y-position to a ladder index
#define client_2_index(ypos)   (top_visible_index + (ypos) / line_height)

static void paint_ladder (HWND hWnd)
{
	PAINTSTRUCT ps;
	rwt::price_description pd;
	wchar_t buffer[20];
	int bufflen;
	const int line_width = 35 * avg_char_width;

	SCROLLINFO si;
	si.cbSize = sizeof (si);
	si.fMask = SIF_POS;
	GetScrollInfo (hWnd, SB_VERT, &si);
	top_visible_index = si.nPos;

	HDC hdc = BeginPaint (hWnd, &ps);
	{
		const int first_line = max (0, si.nPos + ps.rcPaint.top / line_height);
		const int last_line = min (ladder.steps () - 1, si.nPos + ps.rcPaint.bottom / line_height);

		SetTextAlign (hdc, TA_RIGHT | TA_TOP);
		SetBkMode (hdc, TRANSPARENT);
		cached_brush bkbrush;

		for (int i = first_line; i <= last_line; ++i)
		{
			int y = (i - si.nPos) * line_height;
			ladder.describe (i, pd);
			bkbrush.set_color (background_for_r (pd.risk_multiple));
			RECT line_rect{ 0, y, line_width, y + char_height };
			FillRect (hdc, &line_rect, *bkbrush);

			// print the size, if there is one...
			if (pd.shares != 0)
			{
				bufflen = swprintf (buffer, 20, L"%d", pd.shares);
				TextOut (hdc, 6 * avg_char_width, y, buffer, bufflen);
			}

			// print the price level...
			bufflen = ladder.format_price (buffer, 20, pd.price);
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

// respond to scroll events
static void scroll_ladder (HWND hWnd, int cmd, int multiplier = 1)
{
	// Get all the vertical scroll bar information
	SCROLLINFO si;
	si.cbSize = sizeof (si);
	si.fMask = SIF_ALL;
	GetScrollInfo (hWnd, SB_VERT, &si);

	// Save the position for comparison later on
	int iVertPos = si.nPos;

	switch (cmd)
	{
	case SB_TOP:
		si.nPos = si.nMin;
		break;

	case SB_BOTTOM:
		si.nPos = si.nMax;
		break;

	case SB_LINEUP:
		si.nPos -= multiplier;
		break;

	case SB_LINEDOWN:
		si.nPos += multiplier;
		break;

	case SB_PAGEUP:
		si.nPos -= si.nPage;
		break;

	case SB_PAGEDOWN:
		si.nPos += si.nPage;
		break;

	case SB_THUMBTRACK:
		si.nPos = si.nTrackPos;
		break;

	default:
		break;
	}
	// Set the position and then retrieve it.  Due to adjustments
	//   by Windows it may not be the same as the value set.
	si.fMask = SIF_POS;
	SetScrollInfo (hWnd, SB_VERT, &si, TRUE);
	GetScrollInfo (hWnd, SB_VERT, &si);

	// If the position has changed, scroll the window and update it
	if (si.nPos != iVertPos)
	{
		ScrollWindow (hWnd, 0, line_height * (iVertPos - si.nPos),
					  NULL, NULL);
		UpdateWindow (hWnd);
	}
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
static LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	INT_PTR result;
	static int trade_size = 1;

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
		case ID_ACTIONS_CLEAR:
			ladder.clear_transactions ();
			InvalidateRect (hWnd, nullptr, true);
			break;
		case ID_ACTIONS_INSTRUMENT:
			result = DialogBox (hInst, MAKEINTRESOURCE (IDD_INSTR_DLG), hWnd, InstrumentSettings);
			if (result == IDOK)
			{
				set_scrollinfo (hWnd);
				InvalidateRect (hWnd, nullptr, true);
			}
			break;
		case ID_ACTIONS_RISK:
			result = DialogBox (hInst, MAKEINTRESOURCE (IDD_RISKAMT_DLG), hWnd, SetRisk);
			if (result == IDOK) 
				InvalidateRect (hWnd, nullptr, true);
			break;
		case ID_ACTIONS_CENTER:
			result = DialogBox (hInst, MAKEINTRESOURCE (IDD_CENTERP_DLG), hWnd, SetCenter);
			if (result == IDOK)
			{
				set_scrollinfo (hWnd);
				InvalidateRect (hWnd, nullptr, true);
			}
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

		// Determine a decent width for the window... just guess...
		RECT winrect;
		GetWindowRect (hWnd, &winrect);
		MoveWindow (hWnd, winrect.left, winrect.top, 40 * avg_char_width, winrect.bottom - winrect.top, true);
	}
	break;
	case WM_SIZE:
		// cxClient = LOWORD (lParam);
		screen_height = HIWORD (lParam);
		set_scrollinfo (hWnd);
		break;
	case WM_PAINT:
		paint_ladder (hWnd);
		break;
	case WM_VSCROLL:
		scroll_ladder (hWnd, LOWORD (wParam));
		break;
	case WM_MOUSEWHEEL:
	{
		int zDelta = GET_WHEEL_DELTA_WPARAM (wParam) / WHEEL_DELTA;
		if (zDelta >= 0)
			scroll_ladder (hWnd, SB_LINEUP, zDelta);
		else
			scroll_ladder (hWnd, SB_LINEDOWN, -zDelta);
	}
	break;
	case WM_LBUTTONUP:
		ladder.adjust (client_2_index (GET_Y_LPARAM (lParam)), trade_size);
		InvalidateRect (hWnd, nullptr, true);
		break;
	case WM_RBUTTONUP:
		ladder.adjust (client_2_index (GET_Y_LPARAM (lParam)), -trade_size);
		InvalidateRect (hWnd, nullptr, true);
		break;
	case WM_KEYUP:
		if (wParam == 0x30)
		{
			trade_size = 10;
		}
		else if ((wParam >= 0x31) && (wParam <= 0x39))
		{
			trade_size = wParam - 0x30;
		}
		else
		{
			return DefWindowProc (hWnd, message, wParam, lParam);
		}
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
static INT_PTR CALLBACK About (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

// Message handler for risk setting box.
static INT_PTR CALLBACK SetRisk (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER (lParam);
	wchar_t buff[20];

	switch (message)
	{
	case WM_INITDIALOG:
	{
		swprintf (buff, 20, L"%.2lf", ladder.risk ());
		SetDlgItemText (hDlg, IDC_EDIT_RISKAMT, buff);
		return (INT_PTR)TRUE;
	}
	case WM_COMMAND:
	{
		auto cmd = LOWORD (wParam);
		if (cmd == IDOK)
		{
			double newr = -1;
			GetDlgItemText (hDlg, IDC_EDIT_RISKAMT, buff, 20);
			swscanf_s (buff, L"%lf", &newr);
			if (newr > 0) ladder.set_risk (newr);
		}
		if ((cmd == IDOK) || (cmd == IDCANCEL))
		{
			EndDialog (hDlg, LOWORD (wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	}
	return (INT_PTR)FALSE;
}

// Message handler for center setting box.
static INT_PTR CALLBACK SetCenter (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER (lParam);
	wchar_t buff[20];

	switch (message)
	{
	case WM_INITDIALOG:
	{
		ladder.format_price (buff, 20, ladder.center ());
		SetDlgItemText (hDlg, IDC_EDIT_CENTERP, buff);
		return (INT_PTR)TRUE;
	}
	case WM_COMMAND:
	{
		auto cmd = LOWORD (wParam);
		if (cmd == IDOK)
		{
			double newc = -1;
			GetDlgItemText (hDlg, IDC_EDIT_CENTERP, buff, 20);
			swscanf_s (buff, L"%lf", &newc);
			if (newc > 0) ladder.recenter (newc);
		}
		if ((cmd == IDOK) || (cmd == IDCANCEL))
		{
			EndDialog (hDlg, LOWORD (wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	}
	return (INT_PTR)FALSE;
}

// Message handler for instrument settings box.
static INT_PTR CALLBACK InstrumentSettings (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER (lParam);
	wchar_t buff[20];

	switch (message)
	{
	case WM_INITDIALOG:
	{
		swprintf (buff, 20, L"%d", ladder.steps ());
		SetDlgItemText (hDlg, IDC_NUMROWS, buff);
		CheckDlgButton (hDlg, IDC_INST_ES, BST_CHECKED);
		return (INT_PTR)TRUE;
	}
	case WM_COMMAND:
	{
		auto cmd = LOWORD (wParam);
		if (cmd == IDOK)
		{
			double new_center = -1;
			int new_rows = -1;
			double itick_size = 0.25;
			double itick_value = 12.5;

			// get the starting price
			GetDlgItemText (hDlg, IDC_CENTERP, buff, 20);
			swscanf_s (buff, L"%lf", &new_center);
			if (new_center <= 0) return (INT_PTR)FALSE;

			// get the number of rows
			GetDlgItemText (hDlg, IDC_NUMROWS, buff, 20);
			swscanf_s (buff, L"%d", &new_rows);
			if (new_rows <= 0) return (INT_PTR)FALSE;

			// get the instrument
			if (IsDlgButtonChecked (hDlg, IDC_INST_ES) == BST_CHECKED)
			{
				itick_size = 0.25;
				itick_value = 12.5;
			}
			else if (IsDlgButtonChecked (hDlg, IDC_INST_6E) == BST_CHECKED)
			{
				itick_size = 0.00005;
				itick_value = 6.25;
			}
			else if (IsDlgButtonChecked (hDlg, IDC_INST_CL) == BST_CHECKED)
			{
				itick_size = 0.01;
				itick_value = 10.0;
			}
			else if (IsDlgButtonChecked (hDlg, IDC_INST_STOCK) == BST_CHECKED)
			{
				itick_size = 0.01;
				itick_value = 0.01;
			}

			ladder.reset (rwt::instrument{ itick_size, itick_value }, new_center, new_rows);
		}
		if ((cmd == IDOK) || (cmd == IDCANCEL))
		{
			EndDialog (hDlg, cmd);
			return (INT_PTR)TRUE;
		}
		break;
	}
	}
	return (INT_PTR)FALSE;
}
