// Kidedit.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Kidedit.h"

HINSTANCE g_hinst;                          /* This application's HINSTANCE */
HWND g_hwndChild;                           /* Optional child window */
HFONT g_hfEdit;

#define MARGIN 20

LRESULT CALLBACK EditSubclassProc(
	HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
	UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch (uMsg) 
	{
	case WM_NCDESTROY:
		RemoveWindowSubclass(hwnd, EditSubclassProc, uIdSubclass);
		break;
	case WM_CHAR:
		if ((lParam & 0x40000000) && wParam != VK_BACK) return 0;
		break;
	}
	return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

/*
*  OnSize
*      If we have an inner child, resize it to fit.
*/
void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (g_hwndChild) 
	{
		MoveWindow(g_hwndChild, MARGIN, MARGIN, cx - 2 * MARGIN, cy - 2 * MARGIN, TRUE);
	}
}

/*
*  OnCreate
*      Applications will typically override this and maybe even
*      create a child window.
*/
BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
	g_hfEdit = CreateFont(-72, 0, 0, 0, FW_NORMAL,
		FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		TEXT("Arial"));
	if (!g_hfEdit)
	{
		return FALSE;
	}

	g_hwndChild = CreateWindow(
		TEXT("edit"),                   /* Class Name */
		NULL,                           /* Title */
		WS_CHILD | WS_VISIBLE |
		//ES_UPPERCASE | ES_MULTILINE,    /* Style */
		ES_MULTILINE,    /* Style */
		0, 0, 0, 0,                     /* Position and size */
		hwnd,                           /* Parent */
		NULL,                           /* No menu */
		g_hinst,                        /* Instance */
		0);                             /* No special parameters */
	if (!g_hwndChild)
	{
		return FALSE;
	}

	SetWindowSubclass(g_hwndChild, EditSubclassProc, 0, 0);
	SetWindowFont(g_hwndChild, g_hfEdit, TRUE);
	return TRUE;
}

/*
*  OnDestroy
*      Post a quit message because our application is over when the
*      user closes this window.
*/
void OnDestroy(HWND hwnd)
{
	PostQuitMessage(0);
}

/*
*  OnDestroy
*      Post a quit message because our application is over when the
*      user closes this window.
*/
void OnNcDestroy(HWND hwnd)
{
	if (g_hfEdit)
	{
		DeleteObject(g_hfEdit);
	}
}

/*
*  PaintContent
*      Interesting things will be painted here eventually.
*/
void PaintContent(HWND hwnd, PAINTSTRUCT *pps)
{
}

/*
*  OnPaint
*      Paint the content as part of the paint cycle.
*/
void OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	BeginPaint(hwnd, &ps);
	PaintContent(hwnd, &ps);
	EndPaint(hwnd, &ps);
}

/*
*  OnPrintClient
*      Paint the content as requested by USER.
*/
void OnPrintClient(HWND hwnd, HDC hdc)
{
	PAINTSTRUCT ps;
	ps.hdc = hdc;
	GetClientRect(hwnd, &ps.rcPaint);
	PaintContent(hwnd, &ps);
}

void OnSetFocus(HWND hwnd, HWND hwndOldFocus)
{
	if (g_hwndChild)
	{
		SetFocus(g_hwndChild);
	}
}

void ChangeFont(HWND hwnd)
{
	LOGFONT lf;
	GetObject(g_hfEdit, sizeof(lf), &lf);
	CHOOSEFONT cf = { sizeof(cf) };
	cf.hwndOwner = hwnd;
	cf.lpLogFont = &lf;
	cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
	if (ChooseFont(&cf))
	{
		HFONT hfNew = CreateFontIndirect(&lf);
		if (hfNew)
		{
			DeleteObject(g_hfEdit);
			g_hfEdit = hfNew;
			SetWindowFont(g_hwndChild, g_hfEdit, TRUE);
		}
	}
}

void ClearEdit()
{
	SendMessage(g_hwndChild, WM_SETTEXT, 0, 0);
}

BOOL CALLBACK AboutDialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_COMMAND && (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL))
	{
		EndDialog(hwndDlg, 0);
		return TRUE;
	}

	return FALSE;
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDM_ABOUT:
		DialogBox(g_hinst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, (DLGPROC)AboutDialogProc);
		break;
	case IDM_CHOOSEFONT:
		ChangeFont(hwnd);
		break;
	case IDM_CLEAR:
		ClearEdit();
		break;
	case IDM_QUIT:
		PostQuitMessage(0);
		break;
	}
}

/*
*  Window procedure
*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg) 
	{
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hwnd, WM_SIZE, OnSize);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hwnd, WM_NCDESTROY, OnNcDestroy);
		HANDLE_MSG(hwnd, WM_SETFOCUS, OnSetFocus);
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
	case WM_PRINTCLIENT: 
		OnPrintClient(hwnd, (HDC)wParam); 
		return 0;
	}

	return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}

BOOL InitApp(void)
{
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g_hinst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("Kidedit");

	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	InitCommonControls();               /* In case we use a common control */

	return TRUE;
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hinstPrev,
	LPSTR lpCmdLine, int nShowCmd)
{
	MSG msg;
	HWND hwnd;

	g_hinst = hinst;

	if (!InitApp())
	{
		return 0;
	}

	if (SUCCEEDED(CoInitialize(NULL)))	/* In case we use COM */
	{    
		hwnd = CreateWindow(
			TEXT("Kidedit"),                /* Class Name */
			TEXT("Kidedit"),                /* Title */
			WS_POPUP,                       /* Style */
			0, 0,                           /* Position */
			GetSystemMetrics(SM_CXSCREEN),
			GetSystemMetrics(SM_CYSCREEN),  /* Size */
			NULL,                           /* Parent */
			NULL,                           /* No menu */
			hinst,                          /* Instance */
			0);                             /* No special parameters */

		ShowWindow(hwnd, nShowCmd);

		HACCEL hacc = LoadAccelerators(hinst, MAKEINTRESOURCE(IDC_KIDEDIT));

		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (!TranslateAccelerator(hwnd, hacc, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		DestroyAcceleratorTable(hacc);

		CoUninitialize();
	}

	return 0;
}
