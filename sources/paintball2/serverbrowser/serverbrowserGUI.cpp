/*
Copyright (C) 2006 Nathan Wulf (jitspoe) / Digital Paint

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


//ADAPTIONS TO COMPILE WITH GCC:
/*
#define strcpy_s strcpy
#define _snprintf_s _snprintf
#define sprintf_s sprintf
*/

#include "serverbrowser.h"
#include <commctrl.h>
#include <windowsx.h>
#include <Shlwapi.h>

#define MAX_LOADSTRING 100
#define WM_TRAY_ICON_NOTIFY_MESSAGE (WM_USER + 1)
#define NUM_ICONS 5
#define ICON_SIZE 16

// Global Variables:
static HWND g_hStatus;
static HWND g_hServerList;
static HWND g_hPlayerList;
static HWND g_hServerInfoList;
static HWND g_hSearchPlayerDlg;
static HWND g_hVerticalSeperator;
static HWND g_hHorizontalSeperator;
static bool g_bVerticalSeperatorDragged = false;
static bool g_bHorizontalSeperatorDragged = false;
static bool g_abSortDirServers[8] = { false, false, true, false, false }; // Make # players sort highest to lowest by default
static bool g_abSortDirPlayers[8];
static bool g_abSortDirServerInfo[4];
static HINSTANCE g_hInst;
static NOTIFYICONDATA g_tTrayIcon;
static HMENU g_hMenuTray;
static HMENU g_hMenuRightClick;
static int g_nStatusBarHeight = 20;
static DWORD g_dwDefaultTextColor = RGB(0, 0, 0); // todo - obtain defaults from windows api
static DWORD g_dwDefaultTextBackground = RGB(255, 255, 255); // todo
static DWORD g_dwWhiteTextBackground = RGB(255, 255, 255); // todo
static DWORD g_dwRedTextBackground = RGB(180, 5, 0); // todo - might need tweaking
static DWORD g_dwBlueTextBackground = RGB(10, 40, 150);
static DWORD g_dwPurpleTextBackground = RGB(120, 40, 120);
static DWORD g_dwYellowTextBackground = RGB(255, 255, 80);
static DWORD g_dwBlackTextColor = RGB(0, 0, 0);
static DWORD g_dwWhiteTextColor = RGB(255, 255, 255);
static DWORD g_dwFadedTextColor = RGB(160, 160, 160); // Greyed out color
static int g_nServerlistSortColumn = -1;
static DWORD g_iIconBlank = 0;
static DWORD g_iIconCertificated = 0;
static DWORD g_iIconNeedPassword = 0;
static DWORD g_iIconGLS1 = 0;
static DWORD g_iIconGLS2 = 0;
static bool g_bAlreadyOpen = false;
static DWORD g_dwExistingProcess = 0;
static bool g_bThemed = false;
static int g_iPlayerSearchSortOrder = 1; //sort order for list view in player search dialog; abs = column number; sign = ascending / descending

char g_szGameDir[256];
string g_sCurrentServerAddress;
#define PID_FILE "%USERPROFILE%\\DPPB2_Serverbrowser.pid"



// Ugh, what a pain.  This is a callback for every window
BOOL CALLBACK AlreadyOpenEnumWindowsProc (HWND hWnd, LPARAM lParam)
{
	DWORD dwProcess;
	GetWindowThreadProcessId(hWnd, &dwProcess);
	
	if (g_dwExistingProcess == dwProcess)
	{
		HWND hWndRoot = GetAncestor(hWnd, GA_ROOT);
		ShowWindow(hWnd, SW_SHOW);
		SetForegroundWindow(hWnd);
		g_bAlreadyOpen = true;
		return FALSE;
	}

	return TRUE;
}


// If process already exists, give it focus, otherwise save this process id in case somebody tries to open the server browser twice
bool AlreadyOpen ()
{
	char szPIDFile[1024];
	DWORD dwProcess = GetCurrentProcessId();
	DWORD dwWritten;
	DWORD dwRead;

	ExpandEnvironmentStrings(PID_FILE, szPIDFile, sizeof(szPIDFile));

	HANDLE hFile = CreateFile(szPIDFile, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		DWORD dwError = GetLastError();
		return false;
	}

	ReadFile(hFile, &g_dwExistingProcess, sizeof(DWORD), &dwRead, NULL);

	if (g_dwExistingProcess && dwRead == sizeof(DWORD))
	{
		EnumWindows(AlreadyOpenEnumWindowsProc, 0);

		if (g_bAlreadyOpen)
		{
			CloseHandle(hFile);
			return true;
		}
	}

	WriteFile(hFile, &dwProcess, sizeof(dwProcess), &dwWritten, NULL);
	CloseHandle(hFile);

	return false;
}


// Don't leave junk in the user's directory
void DeletePIDFile ()
{
	char szPIDFile[1024];

	ExpandEnvironmentStrings(PID_FILE, szPIDFile, sizeof(szPIDFile));
	DeleteFile(szPIDFile);
}


// Foward declarations of functions included in this code module:
static LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK About (HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK SearchPlayerDlg (HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK OnSearchPlayerDlgSort (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

int APIENTRY WinMain (HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPSTR     lpCmdLine,
                      int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;
	WNDCLASSEX wcex;
	HWND hWnd;
	TCHAR szTitle[MAX_LOADSTRING];
	TCHAR szWindowClass[MAX_LOADSTRING];
	HMENU hMenu;
	hMenu = NULL;
	HINSTANCE hinstDll;
	DLLGETVERSIONPROC pDllGetVersion;

	if (AlreadyOpen())
	{
		return 0;
	}

	//Use ComCtl32.dll v6 when available, set g_bThemed to true if ComCtl32.dll version >= 6
	InitCommonControls();
    hinstDll = GetModuleHandle("ComCtl32.dll");
	if(hinstDll)
    {
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");
        if(pDllGetVersion)
        {
            DLLVERSIONINFO2 dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.info1.cbSize = sizeof(dvi);

            hr = (*pDllGetVersion)(&dvi.info1);
            if(SUCCEEDED(hr))
            {
				if (dvi.info1.dwMajorVersion >= 6)
					g_bThemed = true;
				else
					g_bThemed = false;
			}
		}
	}

	// Create the application window
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SERVERBROWSER, szWindowClass, MAX_LOADSTRING);
	memset(&wcex, 0, sizeof(wcex));
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_SERVERBROWSER);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_SERVERBROWSER;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);
	RegisterClassEx(&wcex);
	g_hInst = hInstance;
	hWnd = CreateWindowEx(0, szWindowClass, szTitle,
		WS_POPUP | WS_BORDER |
		WS_CAPTION | WS_THICKFRAME |
		WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		100, 100, 825, 500, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_SERVERBROWSER);
	hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU_TRAY));
	g_hMenuTray = GetSubMenu(hMenu, 0);
	g_hMenuRightClick = GetSubMenu(hMenu, 1);
	InitApp();

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(hWnd, hAccelTable, &msg) && !IsDialogMessage (g_hSearchPlayerDlg, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DestroyMenu(hMenu);
	DeletePIDFile();
	ShutdownApp();
	return msg.wParam;
}


static BOOL OnCreate (HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	int i;
	char *pServerList[] = { "C", "PW", "GLS", "Server Name", "Map", "Players", "Ping", "Address" };
//	char *pServerList[] = { "Server Name", "Map", "Players", "Ping", "Address" };
	int iaServerListWidths[] = { 20, 20, 20, 300, 150, 55, 40, -2 };
//	int iaServerListWidths[] = { 350, 150, 55, 40, -2 };
	char *pPlayerList[] = { "Player Name", "Kills", "Ping" };
	int iaPlayerListWidths[] = { 200, 60, -2 };
	char *pInfoList[] = { "Variable", "Value" };
	int iaInfoListWidths[] = { 100, -2 };
	LV_COLUMN lvColumn;
	RECT rect;
    HIMAGELIST hImageList;
    HICON hIcon;

	memset(&lvColumn, 0, sizeof(lvColumn));
	lvColumn.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;

	// Status Bar
	g_hStatus = CreateStatusWindow(WS_CHILD | WS_CLIPSIBLINGS |
		WS_VISIBLE | CCS_BOTTOM | SBARS_SIZEGRIP,
		"Ready.", hWnd, IDC_STATUSBAR);
	ShowWindow(g_hStatus, SW_SHOW);

	if (SendMessage(g_hStatus, SB_GETRECT, 0, (LPARAM)&rect))
		g_nStatusBarHeight = rect.bottom;

	// Main Server List
	g_hServerList = CreateWindowEx(0, WC_LISTVIEW, "",
		WS_CHILD | WS_BORDER | WS_VISIBLE | LVS_REPORT | LVS_AUTOARRANGE | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
		0, 0, 300, 300, hWnd, NULL, g_hInst, NULL);
	ListView_SetExtendedListViewStyle(g_hServerList, LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES | LVS_EX_DOUBLEBUFFER);

	for (i = 0; i < SERVERLIST_OFFSET_MAX; i++)
	{
		lvColumn.cx = 150;
		lvColumn.iSubItem = i;
		lvColumn.pszText = pServerList[i];

		if (ListView_InsertColumn(g_hServerList, i, &lvColumn) == -1)
			return FALSE;

		ListView_SetColumnWidth(g_hServerList, i, iaServerListWidths[i]);
	}

	// Icons for main server list
    if ((hImageList = ImageList_Create(ICON_SIZE, ICON_SIZE, ILC_MASK, 0, NUM_ICONS)) == NULL)
	{
        return FALSE;
	}

    hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_BLANK));
    g_iIconBlank = ImageList_AddIcon(hImageList, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_CERTIFICATED));
    g_iIconCertificated = ImageList_AddIcon(hImageList, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_NEEDPASSWORD));
    g_iIconNeedPassword = ImageList_AddIcon(hImageList, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_GLS1));
    g_iIconGLS1 = ImageList_AddIcon(hImageList, hIcon);
	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_GLS2));
    g_iIconGLS2 = ImageList_AddIcon(hImageList, hIcon);
    ListView_SetImageList(g_hServerList, hImageList, LVSIL_SMALL);

	// Player List
	g_hPlayerList = CreateWindowEx(0, WC_LISTVIEW, "",
		WS_CHILD | WS_BORDER | WS_VISIBLE | LVS_REPORT | LVS_AUTOARRANGE | LVS_SINGLESEL,
		0, 0, 300, 200, hWnd, NULL, g_hInst, NULL);
	ListView_SetExtendedListViewStyle(g_hPlayerList, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	for (i = 0; i < PLAYERLIST_OFFSET_MAX; i++)
	{
		lvColumn.cx = 150;
		lvColumn.iSubItem = i;
		lvColumn.pszText = pPlayerList[i];

		if (ListView_InsertColumn(g_hPlayerList, i, &lvColumn) == -1)
			return FALSE;

		ListView_SetColumnWidth(g_hPlayerList, i, iaPlayerListWidths[i]);
	}

	// Server Info List
	g_hServerInfoList = CreateWindowEx(0, WC_LISTVIEW, "",
		WS_CHILD | WS_BORDER | WS_VISIBLE | LVS_REPORT | LVS_AUTOARRANGE | LVS_SINGLESEL,
		0, 0, 300, 200, hWnd, NULL, g_hInst, NULL);
	ListView_SetExtendedListViewStyle(g_hServerInfoList, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	for (i = 0; i < 2; i++)
	{
		lvColumn.cx = 150;
		lvColumn.iSubItem = i;
		lvColumn.pszText = pInfoList[i];

		if (ListView_InsertColumn(g_hServerInfoList, i, &lvColumn) == -1)
			return FALSE;

		ListView_SetColumnWidth(g_hServerInfoList, i, iaInfoListWidths[i]);
	}

	// Tray icon
	memset(&g_tTrayIcon, 0, sizeof(g_tTrayIcon));
	g_tTrayIcon.cbSize = sizeof(g_tTrayIcon);
	g_tTrayIcon.hIcon = LoadIcon(g_hInst, (LPCTSTR)IDI_SMALL);
	g_tTrayIcon.hWnd = hWnd;
	strcpy_s(g_tTrayIcon.szTip, "Digital Paint: Paintball 2 - Serverbrowser");
	g_tTrayIcon.uCallbackMessage = WM_TRAY_ICON_NOTIFY_MESSAGE;
	g_tTrayIcon.uFlags = NIF_ICON|NIF_TIP|NIF_MESSAGE;
	g_tTrayIcon.uID = 0;
	Shell_NotifyIcon(NIM_ADD, &g_tTrayIcon);

	//movable Separators
	GetClientRect (hWnd, &rect);
	g_hHorizontalSeperator = CreateWindowEx(0, WC_STATIC, "", SS_SIMPLE | WS_VISIBLE | WS_CHILD,
		0, rect.bottom / 2, 300, 3, hWnd, NULL, g_hInst, NULL);
	g_hVerticalSeperator = CreateWindowEx(0, WC_STATIC, "", SS_SIMPLE | WS_VISIBLE | WS_CHILD,
		rect.right / 2, 200, 3, 200, hWnd, NULL, g_hInst, NULL);

	return TRUE;
}


static void TweakListviewWidths (void)
{
	ListView_SetColumnWidth(g_hServerList, SERVERLIST_ADDRESS_OFFSET, LVSCW_AUTOSIZE_USEHEADER);
	ListView_SetColumnWidth(g_hPlayerList, 2, LVSCW_AUTOSIZE_USEHEADER);
	ListView_SetColumnWidth(g_hServerInfoList, 1, LVSCW_AUTOSIZE_USEHEADER);
}


static void OnResize (HWND hWnd, UINT state, int cx, int cy)
{
	static int old_cx;
	static int old_cy;

	if (state != SIZE_MINIMIZED)
	{
		RECT rc, rcVertSeperator, rcHorzSeperator, rcInvalidate;
		GetClientRect(hWnd, &rc);
		rc.bottom -= g_nStatusBarHeight;

		GetWindowRect(g_hHorizontalSeperator, &rcHorzSeperator);
		MapWindowPoints (HWND_DESKTOP, hWnd, (LPPOINT) &rcHorzSeperator, 2);
		GetWindowRect(g_hVerticalSeperator, &rcVertSeperator);
		MapWindowPoints (HWND_DESKTOP, hWnd, (LPPOINT) &rcVertSeperator, 2);

		if (old_cy != cy && old_cy != 0) //adapt position of horizontal seperator
		{
			if (old_cy < cy)
				MoveWindow (g_hHorizontalSeperator, 0, rcHorzSeperator.top * cy / old_cy + 1,
					rc.right, rcHorzSeperator.bottom - rcHorzSeperator.top, FALSE);
			else
				MoveWindow (g_hHorizontalSeperator, 0, rcHorzSeperator.top * cy / old_cy,
					rc.right, rcHorzSeperator.bottom - rcHorzSeperator.top, FALSE);
			
			GetWindowRect(g_hHorizontalSeperator, &rcHorzSeperator); //get new position
			MapWindowPoints (HWND_DESKTOP, hWnd, (LPPOINT) &rcHorzSeperator, 2);
		}
		else
		{
			MoveWindow (g_hHorizontalSeperator, 0, rcHorzSeperator.top, rc.right - rc.left, //adapt width of horizontal seperator
				rcHorzSeperator.bottom - rcHorzSeperator.top, FALSE);
			
			GetWindowRect(g_hHorizontalSeperator, &rcHorzSeperator); //get new position
			MapWindowPoints (HWND_DESKTOP, hWnd, (LPPOINT) &rcHorzSeperator, 2);
		}

		if (old_cx != cx && old_cx != 0)
		{
			if (old_cx < cx)
				MoveWindow (g_hVerticalSeperator, rcVertSeperator.left * cx / old_cx + 1,
					rcHorzSeperator.bottom, rcVertSeperator.right - rcVertSeperator.left, rc.bottom - rcHorzSeperator.bottom , TRUE);
			else
				MoveWindow (g_hVerticalSeperator, rcVertSeperator.left * cx / old_cx,
					rcHorzSeperator.bottom, rcVertSeperator.right - rcVertSeperator.left, rc.bottom - rcHorzSeperator.bottom, TRUE);
			
			GetWindowRect(g_hVerticalSeperator, &rcVertSeperator); //get new position
			MapWindowPoints (HWND_DESKTOP, hWnd, (LPPOINT) &rcVertSeperator, 2);
		}
		else
		{
			MoveWindow (g_hVerticalSeperator, rcVertSeperator.left, rcHorzSeperator.bottom,  //adapt height and position of vertical seperator
				rcVertSeperator.right - rcVertSeperator.left, rc.bottom - rcHorzSeperator.bottom, TRUE);

			GetWindowRect(g_hVerticalSeperator, &rcVertSeperator); //get new position
			MapWindowPoints (HWND_DESKTOP, hWnd, (LPPOINT) &rcVertSeperator, 2);
		}
		
		MoveWindow(g_hServerList, rc.left, rc.top, rc.right, rcHorzSeperator.top, (old_cy == cy && old_cx == cx) ? TRUE : FALSE);
		MoveWindow(g_hPlayerList, rc.left, rcHorzSeperator.bottom, rcVertSeperator.left, rc.bottom - rcHorzSeperator.bottom, (old_cy == cy && old_cx == cx) ? TRUE : FALSE);
		MoveWindow(g_hServerInfoList, rcVertSeperator.right , rcHorzSeperator.bottom,
			rc.right - rcVertSeperator.right, rc.bottom - rcHorzSeperator.bottom, (old_cy == cy && old_cx == cx) ? TRUE : FALSE);
		TweakListviewWidths();

		if (old_cy == cy && old_cx == cx)
		{
			GetClientRect(g_hPlayerList, &rcInvalidate); //Invalidate top border of the player list
			rcInvalidate.bottom = g_nStatusBarHeight;
			InvalidateRect(g_hPlayerList, &rcInvalidate, FALSE);
			
			GetClientRect(g_hServerInfoList, &rcInvalidate); //Invalidate top border of the server info list
			rcInvalidate.bottom = g_nStatusBarHeight;
			InvalidateRect(g_hServerInfoList, &rcInvalidate, FALSE);

			if (!g_bThemed) //invalidate left border of server info list, smaller area necessary with ComCtl32.dll v6
			{
				GetClientRect(g_hServerInfoList, &rcInvalidate);
				rcInvalidate.right = g_nStatusBarHeight * 2;
				InvalidateRect(g_hServerInfoList, &rcInvalidate, TRUE);
			}
			else
			{
				GetClientRect(g_hServerInfoList, &rcInvalidate);
				rcInvalidate.right = 5;
				InvalidateRect(g_hServerInfoList, &rcInvalidate, TRUE);
			}
		}
		
		FORWARD_WM_SIZE(g_hStatus, state, cx, cy, SendMessage);
	}
	old_cx = cx;
	old_cy = cy;
}


static void OnRefresh (void)
{
	if (g_hSearchPlayerDlg != 0)
		SendMessage(g_hSearchPlayerDlg, WM_COMMAND, MAKEWPARAM(IDC_SP_UPDATE, 0), (LPARAM) GetDlgItem(g_hSearchPlayerDlg, IDC_SP_UPDATE));
	else
		RefreshList();
}

static BOOL CopyAddress (HWND hWnd, int nItem, copytype_t eCopyType)
{
	char szAddress[64];
	char szHostname[64];
	char szFullText[1024];
	int nLen = 0;
	char *pClipboardData;
	HGLOBAL hClipboard;

	ListView_GetItemText(g_hServerList, nItem, SERVERLIST_ADDRESS_OFFSET,
		szAddress, sizeof(szAddress));

	switch (eCopyType)
	{
	case COPYTYPE_ADDRESS:
		strncpyz(szFullText, szAddress, sizeof(szFullText));
		nLen = strlen(szFullText);
		break;
	case COPYTYPE_URL:
		nLen = _snprintf_s(szFullText, sizeof(szFullText), "paintball2://%s", szAddress);
		szFullText[sizeof(szFullText)-1] = 0;
		break;
	case COPYTYPE_NAMEIP:
		ListView_GetItemText(g_hServerList, nItem, SERVERLIST_HOSTNAME_OFFSET,
				szHostname, sizeof(szHostname));
		nLen = _snprintf_s(szFullText, sizeof(szFullText), "%s - %s", szHostname, szAddress);
		szFullText[sizeof(szFullText)-1] = 0;
		break;
	case COPYTYPE_FULL:
		{
//			char szCertificated[16];
//			char szNeedPassword[16];
//			char szGLS[16];
			char szPing[16];
			char szMap[32];
			char szPlayers[16];

#if 0
			ListView_GetItemText(g_hServerList, nItem, SERVERLIST_CERTIFICATED_OFFSET,
				szCertificated, sizeof(szCertificated));
			ListView_GetItemText(g_hServerList, nItem, SERVERLIST_NEEDPASSWORD_OFFSET,
				szNeedPassword, sizeof(szNeedPassword));
			ListView_GetItemText(g_hServerList, nItem, SERVERLIST_GLS_OFFSET,
				szGLS, sizeof(szGLS));
#endif
			ListView_GetItemText(g_hServerList, nItem, SERVERLIST_HOSTNAME_OFFSET,
				szHostname, sizeof(szHostname));
			ListView_GetItemText(g_hServerList, nItem, SERVERLIST_PING_OFFSET,
				szPing, sizeof(szPing));
			ListView_GetItemText(g_hServerList, nItem, SERVERLIST_MAP_OFFSET,
				szMap, sizeof(szMap));
			ListView_GetItemText(g_hServerList, nItem, SERVERLIST_PLAYERS_OFFSET,
				szPlayers, sizeof(szPlayers));
//			nLen = _snprintf_s(szFullText, sizeof(szFullText), "%s|%s|%s|%s|%s|%s|%s|%s",
//				szAddress, szCertificated, szNeedPassword, szGLS, szHostname, szPing, szMap, szPlayers);
			nLen = _snprintf_s(szFullText, sizeof(szFullText), "%s|%s|%s|%s|%s",
				szAddress, szHostname, szPing, szMap, szPlayers);
			szFullText[sizeof(szFullText)-1] = 0;
			break;
		}
	}

	if (!nLen)
		return FALSE;

	if (!OpenClipboard(hWnd))
        return FALSE;

	hClipboard = GlobalAlloc(GMEM_MOVEABLE, (nLen + 1));

	if (!hClipboard)
	{
		CloseClipboard();
		return FALSE;
	}

	EmptyClipboard();
	pClipboardData = (char *)GlobalLock(hClipboard);
	strcpy(pClipboardData, szFullText);
	GlobalUnlock(hClipboard);
	SetClipboardData(CF_TEXT, pClipboardData);
	CloseClipboard();

	return TRUE;
}


static BOOL OnCommand (HWND hWnd, int wmId, HWND hWndCtl, UINT codeNotify)
{
	char szServerbrowserINI[256];
	_snprintf(szServerbrowserINI, sizeof(szServerbrowserINI), "%s\\serverbrowser.ini", g_szGameDir);

	switch (wmId)
	{
	case IDM_ABOUT:
		DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
		return TRUE;
	case IDM_WEBSITE:
		ShellExecute(NULL, "open", "http://www.digitalpaint.org", NULL, NULL, SW_SHOW);
		return TRUE;
	case IDM_WEBSITE_GLS:
		ShellExecute(NULL, "open", "http://www.dplogin.com/index.php?action=main", NULL, NULL, SW_SHOW);
		return TRUE;
	case IDM_WEBSITE_PLAYER:
		ShellExecute(NULL, "open", "http://www.dplogin.com/index.php?action=displaymembers", NULL, NULL, SW_SHOW);
		return TRUE;
	case IDM_WEBSITE_FORUM:
		ShellExecute(NULL, "open", "http://dplogin.com/forums/index.php", NULL, NULL, SW_SHOW);
		return TRUE;
	case IDM_WEBSITE_DOCS:
		ShellExecute(NULL, "open", "http://digitalpaint.org/docs.html", NULL, NULL, SW_SHOW);
		return TRUE;
	case IDM_UPDATE:
		UpdateList();
		return TRUE;
	case IDM_REFRESH:
		OnRefresh();
		return TRUE;
	case IDM_EXIT:
		DestroyWindow(hWnd);
		return TRUE;
	//Edit by Richard
	case IDM_SEARCHPLAYER:
		g_hSearchPlayerDlg = CreateDialog(g_hInst, (LPCTSTR)IDD_SEARCHPLAYER, hWnd, (DLGPROC)SearchPlayerDlg);
		return TRUE;
	case ID_TRAY_EXIT:
		DestroyWindow(hWnd);
		return TRUE;
	case ID_TRAY_RESTORE:
		ShowWindow(hWnd, SW_SHOW);
		SetForegroundWindow(hWnd);
		return TRUE;
	case ID_RIGHTCLICK_COPYADDRESS:
		return CopyAddress(hWnd, codeNotify, COPYTYPE_ADDRESS);
	case ID_RIGHTCLICK_COPYURL:
		return CopyAddress(hWnd, codeNotify, COPYTYPE_URL);
	case ID_RIGHTCLICK_COPYFULLSERVERINFO:
		return CopyAddress(hWnd, codeNotify, COPYTYPE_FULL);
	case ID_RIGHTCLICK_COPYNAMEIP:
		return CopyAddress(hWnd, codeNotify, COPYTYPE_NAMEIP);
	default:
		return FALSE;
	}
}


static void OnDestroy (HWND hWnd)
{
	// TODO: save registry data
	Shell_NotifyIcon(NIM_DELETE, &g_tTrayIcon);
	PostQuitMessage(0);
}


static BOOL OnErase (HWND hWnd, HDC hdc)
{
	// Erasing makes the window flicker, so do nothing and pretend we handled it:
	return TRUE;
}


// Function for sorting the player list
static int CALLBACK PlayerCompareProc (LPARAM lParam1, LPARAM lParam2, LPARAM lParam3)
{
	char szBuff1[64];
	char szBuff2[64];
	LVFINDINFO lvFindInfo;
	int nItem1, nItem2;

	lvFindInfo.flags = LVFI_PARAM;
	lvFindInfo.lParam = lParam1;
	nItem1 = ListView_FindItem(g_hPlayerList, -1, &lvFindInfo);
	lvFindInfo.lParam = lParam2;
	nItem2 = ListView_FindItem(g_hPlayerList, -1, &lvFindInfo);
	ListView_GetItemText(g_hPlayerList, nItem1, lParam3, szBuff1, sizeof(szBuff1));
	ListView_GetItemText(g_hPlayerList, nItem2, lParam3, szBuff2, sizeof(szBuff2));

	if (lParam3 == PLAYERLIST_NAME_OFFSET)
	{
		if (g_abSortDirPlayers[lParam3])
			return _stricmp(szBuff1, szBuff2);
		else
			return _stricmp(szBuff2, szBuff1);
	}
	else
	{
		if (g_abSortDirServers[lParam3])
			return atoi(szBuff1) - atoi(szBuff2);
		else
			return atoi(szBuff2) - atoi(szBuff1);
	}
}


// Function for sorting the server list
static int CALLBACK ServerInfoCompareProc (LPARAM lParam1, LPARAM lParam2, LPARAM lParam3)
{
	char szBuff1[64];
	char szBuff2[64];
	LVFINDINFO lvFindInfo;
	int nItem1, nItem2;

	lvFindInfo.flags = LVFI_PARAM;
	lvFindInfo.lParam = lParam1;
	nItem1 = ListView_FindItem(g_hServerInfoList, -1, &lvFindInfo);
	lvFindInfo.lParam = lParam2;
	nItem2 = ListView_FindItem(g_hServerInfoList, -1, &lvFindInfo);
	ListView_GetItemText(g_hServerInfoList, nItem1, lParam3, szBuff1, sizeof(szBuff1));
	ListView_GetItemText(g_hServerInfoList, nItem2, lParam3, szBuff2, sizeof(szBuff2));

	if (g_abSortDirServerInfo[lParam3])
		return _stricmp(szBuff1, szBuff2);
	else
		return _stricmp(szBuff2, szBuff1);
}


static bool ServerActive (int nRow)
{
	char szAddress[64];

	ListView_GetItemText(g_hServerList, nRow, SERVERLIST_ADDRESS_OFFSET, szAddress, sizeof(szAddress));

	return ServerActive(szAddress);
}


// Function for sorting the server list
static int CALLBACK ServerCompareProc (LPARAM lParam1, LPARAM lParam2, LPARAM lParam3)
{
	char szBuff1[64];
	char szBuff2[64];
	LVFINDINFO lvFindInfo;
	int nItem1, nItem2;
	bool bActive1, bActive2;

	lvFindInfo.flags = LVFI_PARAM;
	lvFindInfo.lParam = lParam1;
	nItem1 = ListView_FindItem(g_hServerList, -1, &lvFindInfo);
	lvFindInfo.lParam = lParam2;
	nItem2 = ListView_FindItem(g_hServerList, -1, &lvFindInfo);
	ListView_GetItemText(g_hServerList, nItem1, lParam3, szBuff1, sizeof(szBuff1));
	ListView_GetItemText(g_hServerList, nItem2, lParam3, szBuff2, sizeof(szBuff2));
	bActive1 = ServerActive(nItem1);
	bActive2 = ServerActive(nItem2);

#ifdef SEPARATE_INACTIVE
	if (bActive1 == bActive2)
	{
#endif
		if (lParam3 == SERVERLIST_PLAYERS_OFFSET || lParam3 == SERVERLIST_PING_OFFSET)
		{
			if (g_abSortDirServers[lParam3])
				return atoi(szBuff1) - atoi(szBuff2);
			else
				return atoi(szBuff2) - atoi(szBuff1);
		}
		else
		{
			if (g_abSortDirServers[lParam3])
				return _stricmp(szBuff1, szBuff2);
			else
				return _stricmp(szBuff2, szBuff1);
		}
#ifdef SEPARATE_INACTIVE // Note: does not (always) work because serverlist isn't resorted on activity change
	}
	else
	{
		// Put inactive (greyed-out) servers at the bottom of the list
		return bActive1 < bActive2;
	}
#endif
}


static bool MapExists (int nRow)
{
	char szAddress[64];

	ListView_GetItemText(g_hServerList, nRow, SERVERLIST_ADDRESS_OFFSET, szAddress, sizeof(szAddress));

	return MapExistsForAddress(szAddress);
}


static void LaunchGameFromList (int nRow)
{
	char szAddress[64];

	ListView_GetItemText(g_hServerList, nRow, SERVERLIST_ADDRESS_OFFSET, szAddress, sizeof(szAddress));
	LaunchGame(szAddress, NULL);
}


static LRESULT ProcessCustomDraw (LPARAM lParam)
{
    LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;

    switch (lplvcd->nmcd.dwDrawStage)
    {
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;
	case CDDS_ITEMPREPAINT:
		return CDRF_NOTIFYSUBITEMDRAW;
	case CDDS_SUBITEM|CDDS_ITEMPREPAINT:
		switch (lplvcd->iSubItem)
		{
		case SERVERLIST_MAP_OFFSET:
			lplvcd->clrText = ServerActive(lplvcd->nmcd.dwItemSpec) ? (MapExists(lplvcd->nmcd.dwItemSpec) ? g_dwDefaultTextColor : RGB(255, 0, 0)) : g_dwFadedTextColor;
			lplvcd->clrTextBk = g_dwDefaultTextBackground;
			return CDRF_NEWFONT;
		case SERVERLIST_CERTIFICATED_OFFSET:
			lplvcd->clrText = g_dwWhiteTextColor;
			lplvcd->clrTextBk = g_dwDefaultTextBackground;
			return CDRF_NEWFONT;
		case SERVERLIST_NEEDPASSWORD_OFFSET:
			lplvcd->clrText = g_dwWhiteTextColor;
			lplvcd->clrTextBk = g_dwDefaultTextBackground;
			return CDRF_NEWFONT;
		case SERVERLIST_GLS_OFFSET:
			lplvcd->clrText = g_dwWhiteTextColor;
			lplvcd->clrTextBk = g_dwDefaultTextBackground;
			return CDRF_NEWFONT;
		default:
			lplvcd->clrText = ServerActive(lplvcd->nmcd.dwItemSpec) ? g_dwDefaultTextColor : g_dwFadedTextColor;
			lplvcd->clrTextBk = g_dwDefaultTextBackground;
			return CDRF_DODEFAULT;
		}
    }

    return CDRF_DODEFAULT;
}


static char GetPlayerColor (int nRow)
{
	map<string, serverinfo_t>::iterator mIterator;
	mIterator = g_mServers.find(g_sCurrentServerAddress);

	if (mIterator != g_mServers.end())
	{
		return mIterator->second.mPlayerColors[nRow];
	}
	else
	{
		return '?'; // todo
	}
}


static LRESULT CustomDrawPlayerList (LPARAM lParam)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
	char cColor;

	switch (lplvcd->nmcd.dwDrawStage)
    {
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;
	case CDDS_ITEMPREPAINT:
		cColor = GetPlayerColor(lplvcd->nmcd.lItemlParam);

		switch (cColor)
		{
		case 'r':
		case 'b':
		case 'p':
			lplvcd->clrText = g_dwWhiteTextColor;
			break;
		case 'y':
		case 'o':
		default:
			lplvcd->clrText = g_dwBlackTextColor;
		}

		switch (cColor)
		{
		case 'r':
			lplvcd->clrTextBk = g_dwRedTextBackground;
			break;
		case 'b':
			lplvcd->clrTextBk = g_dwBlueTextBackground;
			break;
		case 'p':
			lplvcd->clrTextBk = g_dwPurpleTextBackground;
			break;
		case 'y':
			lplvcd->clrTextBk = g_dwYellowTextBackground;
			break;
		case 'o':
		default:
			lplvcd->clrTextBk = g_dwWhiteTextBackground;
			break;
		}

		return CDRF_NEWFONT;
    }

    return CDRF_DODEFAULT;
}


static void UpdateInfoLists (int nID, bool bRefresh)
{
	char szAddress[128];
	map<string, serverinfo_t>::iterator mIterator;

	ListView_GetItemText(g_hServerList, nID, SERVERLIST_ADDRESS_OFFSET, szAddress, sizeof(szAddress));

	if (bRefresh)
		PingServer(szAddress);

	ListView_DeleteAllItems(g_hPlayerList);
	ListView_DeleteAllItems(g_hServerInfoList);
	mIterator = g_mServers.find(szAddress);

	if (mIterator != g_mServers.end())
	{
		int i;
		LVITEM tLVItem;
		vector<playerinfo_t>::iterator itrPlayers;
		vector<playerinfo_t>::iterator itrPlayersEnd;
		map<string, string>::iterator itrCvars;
		map<string, string>::iterator itrCvarsEnd;
		char szTemp[128];
		string sStatusBar;
		const char *sTimeLeft = NULL;
		const char *sScores = NULL;

		memset(&tLVItem, 0, sizeof(tLVItem));
		tLVItem.mask = LVIF_TEXT|LVIF_PARAM;
		g_sCurrentServerAddress = szAddress;

		// Update the player list
		itrPlayers = mIterator->second.vPlayers.begin();
		itrPlayersEnd = mIterator->second.vPlayers.end();

		for (i = 0; itrPlayers != itrPlayersEnd; ++i, ++itrPlayers)
		{
			tLVItem.iItem = i;
			tLVItem.lParam = i;
			tLVItem.pszText = (char *)itrPlayers->sName.c_str();
			nID = ListView_InsertItem(g_hPlayerList, &tLVItem);
			ListView_SetItemText(g_hPlayerList, nID, 1, _itoa(itrPlayers->nScore, szTemp, 10));
			ListView_SetItemText(g_hPlayerList, nID, 2, _itoa(itrPlayers->nPing, szTemp, 10));
		}

		// Update server cvar list
		itrCvars = mIterator->second.mVars.begin();
		itrCvarsEnd = mIterator->second.mVars.end();

		for (i = 0; itrCvars != itrCvarsEnd; ++i, ++itrCvars)
		{
			tLVItem.iItem = i;
			tLVItem.lParam = i;
			tLVItem.pszText = (char *)itrCvars->first.c_str();
			nID = ListView_InsertItem(g_hServerInfoList, &tLVItem);
			ListView_SetItemText(g_hServerInfoList, nID, 1, (char *)itrCvars->second.c_str());

			if (itrCvars->first == "_scores")
				sScores = itrCvars->second.c_str();

			if (itrCvars->first == "TimeLeft")
				sTimeLeft = itrCvars->second.c_str();
		}

		if (sScores)
		{
			sStatusBar = sScores;
		
			if (sTimeLeft)
				sStatusBar += "with ";
		}

		if (sTimeLeft)
		{
			sStatusBar += sTimeLeft;
			sStatusBar += " left in match.";
		}

		SetStatus(sStatusBar.c_str());
	}

	TweakListviewWidths();
}


static void CorrectRowIds (HWND hList)
{
	int nCount, i;

	nCount = ListView_GetItemCount(hList);
//todo;
	for (i = 0; i < nCount; ++i)
	{
		LVITEM tLVItem;

		memset(&tLVItem, 0, sizeof(tLVItem));
		ListView_GetItem(hList, &tLVItem);
	}
}


static void SortServerList (void)
{
	if (g_nServerlistSortColumn > -1)
		ListView_SortItems(g_hServerList, ServerCompareProc, g_nServerlistSortColumn);
}


static BOOL OnNotify (HWND hWnd, int idFrom, NMHDR FAR *pnmhdr)
{
	BOOL bRet;
	POINT pt;

	if (!pnmhdr)
		return FALSE; // shouldn't happen

	if (pnmhdr->hwndFrom == g_hServerList)
	{
		NM_LISTVIEW *pNMLV = (NM_LISTVIEW *)pnmhdr;
		LPNMLVKEYDOWN pnmlvkd = (LPNMLVKEYDOWN)pnmhdr;

		switch (pnmhdr->code)
		{
		case LVN_COLUMNCLICK:
			g_abSortDirServers[pNMLV->iSubItem] = !g_abSortDirServers[pNMLV->iSubItem];
			g_nServerlistSortColumn = pNMLV->iSubItem;
			SortServerList();
			break;
		case LVN_KEYDOWN:

			if (pnmlvkd->wVKey == VK_RETURN)
				LaunchGameFromList(ListView_GetSelectionMark(g_hServerList));
			
			break;
		case LVN_ITEMCHANGED:
			UpdateInfoLists(ListView_GetSelectionMark(g_hServerList), false);
			break;
		case NM_CLICK:
			UpdateInfoLists(pNMLV->iItem, true);
			break;
		case NM_DBLCLK:
			UpdateInfoLists(pNMLV->iItem, true);
			LaunchGameFromList(pNMLV->iItem);
			break;
		case NM_RCLICK:
			UpdateInfoLists(pNMLV->iItem, true);
			GetCursorPos(&pt);
			bRet = TrackPopupMenu(g_hMenuRightClick, TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);
			return OnCommand(hWnd, bRet, NULL, pNMLV->iItem);
		case NM_CUSTOMDRAW:
			return ProcessCustomDraw((LPARAM)pnmhdr);
		}
	}

	if (pnmhdr->hwndFrom == g_hPlayerList)
	{
		NM_LISTVIEW *pNMLV = (NM_LISTVIEW *)pnmhdr;

		switch (pnmhdr->code)
		{
		case LVN_COLUMNCLICK:
			g_abSortDirPlayers[pNMLV->iSubItem] = !g_abSortDirPlayers[pNMLV->iSubItem];
			ListView_SortItems(g_hPlayerList, PlayerCompareProc, pNMLV->iSubItem);
			break;
		case NM_CUSTOMDRAW:
			return CustomDrawPlayerList((LPARAM)pnmhdr);
		}
	}

	if (pnmhdr->hwndFrom == g_hServerInfoList)
	{
		NM_LISTVIEW *pNMLV = (NM_LISTVIEW *)pnmhdr;

		switch (pnmhdr->code)
		{
		case LVN_COLUMNCLICK:
			g_abSortDirServerInfo[pNMLV->iSubItem] = !g_abSortDirServerInfo[pNMLV->iSubItem];
			ListView_SortItems(g_hServerInfoList, ServerInfoCompareProc, pNMLV->iSubItem);
			break;
		}
	}

	return FORWARD_WM_NOTIFY(hWnd, idFrom, pnmhdr, DefWindowProc);
}


static BOOL OnTrayNotify (HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	BOOL bRet;

	switch (lParam)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		ShowWindow(hWnd, SW_SHOW);
		SetForegroundWindow(hWnd);
		return TRUE;
	case WM_RBUTTONDOWN:
		GetCursorPos(&pt);
		bRet = TrackPopupMenu(g_hMenuTray, TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);
		return OnCommand(hWnd, bRet, NULL, 0);
	default:
		return FALSE;
	}
}


static BOOL OnSysCommand (HWND hWnd, UINT cmd, WPARAM wParam, LPARAM lParam)
{
	switch (cmd)
	{
	case SC_MINIMIZE:
		ShowWindow(hWnd, SW_HIDE);
		return TRUE;
	default:
		FORWARD_WM_SYSCOMMAND(hWnd, cmd, wParam, lParam, DefWindowProc);
		return FALSE;
	}
}

static bool IsOnSeperator (HWND hwnd, int x, int y, HWND hSeperator)
{
	RECT rcSeperator;
	GetWindowRect(hSeperator, &rcSeperator);
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT) &rcSeperator, 2);
	return ((rcSeperator.top - 1 <= y) && (y <= rcSeperator.bottom + 1) && (rcSeperator.left - 1 <= x) && (rcSeperator.right + 1>= x));
}

static BOOL OnLButtonDown (HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	if (IsOnSeperator(hwnd, x, y, g_hHorizontalSeperator))
	{
		g_bHorizontalSeperatorDragged = true;
		SetCapture(hwnd);
		return FALSE;
	}
	else
	{
		if (IsOnSeperator(hwnd, x, y, g_hVerticalSeperator))
		{
			g_bVerticalSeperatorDragged = true;
			SetCapture(hwnd);
			return FALSE;
		}
	}
	FORWARD_WM_LBUTTONDOWN(hwnd, fDoubleClick, x, y, keyFlags, DefWindowProc);
	return FALSE;
}

static BOOL OnLButtonUp (HWND hwnd, int x, int y, UINT keyFlags)
{
	ReleaseCapture();
	g_bVerticalSeperatorDragged = false;
	g_bHorizontalSeperatorDragged = false;
	FORWARD_WM_LBUTTONUP(hwnd, x, y, keyFlags, DefWindowProc);
	return FALSE;
}

static BOOL OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	if (g_bHorizontalSeperatorDragged)
	{
		RECT rcHorz;
		RECT rcMainWin;
		GetClientRect(g_hHorizontalSeperator, &rcHorz);
		GetClientRect(hwnd, &rcMainWin);

		if (y < 0 || y > (rcMainWin.bottom - rcHorz.bottom - g_nStatusBarHeight))
			return FALSE;
		
		GetWindowRect(g_hHorizontalSeperator, &rcHorz);
		MapWindowPoints (HWND_DESKTOP, hwnd, (LPPOINT) &rcHorz, 2);

		MoveWindow (g_hHorizontalSeperator, 0, y, rcMainWin.right - rcMainWin.left, rcHorz.bottom - rcHorz.top, TRUE);
		
		RedrawWindow(g_hVerticalSeperator, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);
		//Vertical seperator has to be redrawn, otherwise there might be graphical errors

		OnResize(hwnd, SIZE_RESTORED, rcMainWin.right - rcMainWin.left, rcMainWin.bottom - rcMainWin.top);
	}
	
	if (g_bVerticalSeperatorDragged)
	{
		RECT rcMainWin;
		RECT rcWin;
		GetClientRect(g_hVerticalSeperator, &rcWin);
		GetClientRect(hwnd, &rcMainWin);

		if (x < 0 || x > (rcMainWin.right - rcWin.right))
			return FALSE;

		GetWindowRect(g_hVerticalSeperator, &rcWin);
		MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT) &rcWin, 2);

		MoveWindow (g_hVerticalSeperator, x, rcWin.top, rcWin.right - rcWin.left, rcWin.bottom - rcWin.top, TRUE);
		RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);

		OnResize(hwnd, SIZE_RESTORED, rcMainWin.right - rcMainWin.left, rcMainWin.bottom - rcMainWin.top);
	}
	return FALSE;
}

static BOOL OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
	if (hwndCursor == hwnd)
	{
		POINT pPoint;
		GetCursorPos(&pPoint);
		MapWindowPoints(HWND_DESKTOP, hwnd, &pPoint, 1);

		if (IsOnSeperator(hwnd, pPoint.x, pPoint.y, g_hHorizontalSeperator))
			SetCursor (LoadCursor(NULL, IDC_SIZENS));
		else if (IsOnSeperator(hwnd, pPoint.x, pPoint.y, g_hVerticalSeperator))
			SetCursor (LoadCursor(NULL, IDC_SIZEWE));
		else
			FORWARD_WM_SETCURSOR(hwnd, hwndCursor, codeHitTest, msg, DefWindowProc);

		return TRUE;
	}
	else
		FORWARD_WM_SETCURSOR(hwnd, hwndCursor, codeHitTest, msg, DefWindowProc);
	
	return TRUE;
}

static void OnPaint(HWND hwnd)
{
	PAINTSTRUCT Ps;
	HDC hDC;
	RECT rcPos;
	DWORD dwColor = GetSysColor(COLOR_WINDOW + 1);

	hDC = BeginPaint(hwnd, &Ps);

	GetWindowRect(g_hHorizontalSeperator, &rcPos); //clear area of horizontal seperator
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT) &rcPos, 2);
	FillRect(hDC, &rcPos, (HBRUSH) COLOR_WINDOW);

	GetWindowRect(g_hVerticalSeperator, &rcPos); //clear area of vertical seperator
	MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT) &rcPos, 2);
	FillRect(hDC, &rcPos, (HBRUSH) COLOR_WINDOW);

	EndPaint(hwnd, &Ps);

	FORWARD_WM_PAINT(hwnd, DefWindowProc);
}

static LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hWnd, WM_NOTIFY, OnNotify);
		HANDLE_MSG(hWnd, WM_SIZE, OnResize);
		HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hWnd, WM_SYSCOMMAND, OnSysCommand);
		HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hWnd, WM_ERASEBKGND, OnErase);
		HANDLE_MSG(hWnd, WM_TRAY_ICON_NOTIFY_MESSAGE, OnTrayNotify);
		HANDLE_MSG(hWnd, WM_LBUTTONDOWN, OnLButtonDown);
		HANDLE_MSG(hWnd, WM_LBUTTONUP, OnLButtonUp);
		HANDLE_MSG(hWnd, WM_MOUSEMOVE, OnMouseMove);
		HANDLE_MSG(hWnd, WM_SETCURSOR, OnSetCursor);
		HANDLE_MSG(hWnd, WM_PAINT, OnPaint);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

// Mesage handler for about box.
static LRESULT CALLBACK About (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}

    return FALSE;
}


void SetStatus (const char *s)
{
	SendMessage(g_hStatus, SB_SETTEXT, (WPARAM)0, (LPARAM)s);
}


// Returns true if the text changes
static bool UpdateListviewText (HWND hList, int nItem, int nColumn, const char *sText)
{
	char szField[1024];

	ListView_GetItemText(hList, nItem, nColumn, szField, sizeof(szField));
	ListView_SetItemText(hList, nItem, nColumn, (char *)sText);

	return strcmp(sText, szField) != 0;
}


int GetListIDFromAddress (const char *sAddress)
{
	int nID;
	int nCount = ListView_GetItemCount(g_hServerList);
	char szAddress[128];

	for (nID = 0; nID < nCount; ++nID)
	{
		ListView_GetItemText(g_hServerList, nID, SERVERLIST_ADDRESS_OFFSET, szAddress, sizeof(szAddress));

		if (strcmp(szAddress, sAddress) == 0)
			return nID;
	}

	return -1;
}


void UpdateServerListGUI (const char *sAddress, serverinfo_t &tServerInfo)
{
	int nID;
	char szTemp[128];
	bool bFound = false;
	bool bUpdated = false;
	int nCount = ListView_GetItemCount(g_hServerList);
	int iImageCertificated = g_iIconBlank;
	int iImageNeedPassword = g_iIconBlank;
	int iImageGLS = g_iIconBlank;

	
	if (tServerInfo.nCertificated)
	{
		iImageCertificated = g_iIconCertificated;
	}
	if (tServerInfo.nNeedPassword)
	{
		iImageNeedPassword = g_iIconNeedPassword;
	}
	if (tServerInfo.nGLS == 1)
	{
		iImageGLS = g_iIconGLS1;
	}
	else if (tServerInfo.nGLS == 2)
	{
		iImageGLS = g_iIconGLS2;
	}

	// Find a matching address to update
	nID = GetListIDFromAddress(sAddress);

	// Add a new server to the list if the address is not found.
	if (nID < 0)
	{
		LVITEM tLVItem;

		memset(&tLVItem, 0, sizeof(tLVItem));
		tLVItem.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE | LVIF_DI_SETITEM;
		tLVItem.state = 0;
		tLVItem.stateMask = 0;

		tLVItem.iSubItem = 0;
		tLVItem.iItem = nCount;
		tLVItem.lParam = nCount;
		tLVItem.iImage = iImageCertificated;
		tLVItem.pszText = "";
		nID = ListView_InsertItem(g_hServerList, &tLVItem);

		tLVItem.mask = LVIF_TEXT | LVIF_IMAGE;
		tLVItem.iSubItem = 1;
		tLVItem.iItem = nCount;
		tLVItem.iImage = iImageNeedPassword;
		tLVItem.pszText = "";
		ListView_SetItem(g_hServerList, &tLVItem);

		tLVItem.mask = LVIF_TEXT | LVIF_IMAGE;
		tLVItem.iSubItem = 2;
		tLVItem.iItem = nCount;
		tLVItem.iImage = iImageGLS;
		tLVItem.pszText = "";
		ListView_SetItem(g_hServerList, &tLVItem);
		TweakListviewWidths();
	}

	// set address first so we can reference it
	if (UpdateListviewText(g_hServerList, nID, SERVERLIST_ADDRESS_OFFSET, sAddress) &&
		g_nServerlistSortColumn == SERVERLIST_ADDRESS_OFFSET)
	{
		SortServerList();
		nID = GetListIDFromAddress(sAddress);
	}

	if (UpdateListviewText(g_hServerList, nID, SERVERLIST_CERTIFICATED_OFFSET, _itoa(tServerInfo.nCertificated, szTemp, 10)) &&
		g_nServerlistSortColumn == SERVERLIST_CERTIFICATED_OFFSET)
	{
		SortServerList();
		nID = GetListIDFromAddress(sAddress);
	}

	if (UpdateListviewText(g_hServerList, nID, SERVERLIST_NEEDPASSWORD_OFFSET, _itoa(tServerInfo.nNeedPassword, szTemp, 10)) &&
		g_nServerlistSortColumn == SERVERLIST_NEEDPASSWORD_OFFSET)
	{
		SortServerList();
		nID = GetListIDFromAddress(sAddress);
	}

	if (UpdateListviewText(g_hServerList, nID, SERVERLIST_GLS_OFFSET, _itoa(tServerInfo.nGLS, szTemp, 10)) &&
		g_nServerlistSortColumn == SERVERLIST_GLS_OFFSET)
	{
		SortServerList();
		nID = GetListIDFromAddress(sAddress);
	}

	if (UpdateListviewText(g_hServerList, nID, SERVERLIST_HOSTNAME_OFFSET, tServerInfo.sHostName.c_str()) &&
		g_nServerlistSortColumn == SERVERLIST_HOSTNAME_OFFSET)
	{
		SortServerList();
		nID = GetListIDFromAddress(sAddress);
	}

	if (UpdateListviewText(g_hServerList, nID, SERVERLIST_MAP_OFFSET, tServerInfo.sMapName.c_str()) &&
		g_nServerlistSortColumn == SERVERLIST_MAP_OFFSET)
	{
		SortServerList();
		nID = GetListIDFromAddress(sAddress);
	}

	sprintf_s(szTemp, "%d / %d", tServerInfo.nPlayers, tServerInfo.nMaxPlayers);

	if (UpdateListviewText(g_hServerList, nID, SERVERLIST_PLAYERS_OFFSET, szTemp) &&
		g_nServerlistSortColumn == SERVERLIST_PLAYERS_OFFSET)
	{
		SortServerList();
		nID = GetListIDFromAddress(sAddress);
	}

	if (UpdateListviewText(g_hServerList, nID, SERVERLIST_PING_OFFSET, _itoa(tServerInfo.nPing, szTemp, 10)) &&
		g_nServerlistSortColumn == SERVERLIST_PING_OFFSET)
	{
		SortServerList();
		nID = GetListIDFromAddress(sAddress);
	}
}


void LoadSettings (void)
{
	HKEY hKey;
	DWORD dwSize;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Digital Paint\\Paintball2",
		0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS ||
		RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Digital Paint\\Paintball2",
		0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		dwSize = sizeof(g_szGameDir);
		RegQueryValueEx(hKey, "INSTDIR", NULL, NULL, (LPBYTE)g_szGameDir, &dwSize);
		RegCloseKey (hKey);
	}
	else
	{
		MessageBox(NULL, "Paintball2 installation directory not found.", "PB2 Server Browser Error", MB_OK);
		strcpy(g_szGameDir, "c:\\games\\paintball2");
	}
}

// SearchThread which will refesh servers, then wait iDelay ms, then redo the search
struct SearchThreadArgs
{
	HWND hDlg;
	std::vector <std::pair<std::string, int> > * pvFound;
	int iDelay;
};

static void SearchThread (SearchThreadArgs* args)
{
	std::string sContentBuffer;
	char szNameBuffer[256];
	char szOldEntry[256];
	char * pStrTokResult;
	int index;
	HWND hList = GetDlgItem(args->hDlg, IDC_SP_LIST);
	HWND hEdit = GetDlgItem(args->hDlg, IDC_SP_EDIT);
	LVITEM LvItem;
	
	//animate the button while waiting so the user sees that something is loading
	if (args->iDelay)
	{
		EnableWindow(GetDlgItem(args->hDlg, IDC_SP_UPDATE), FALSE);
		EnableWindow(GetDlgItem(args->hDlg, IDC_SP_EDIT), FALSE);

		GetDlgItemText(args->hDlg, IDC_SP_UPDATE, szOldEntry, sizeof(szOldEntry) / sizeof(szOldEntry[0]));
		for (index = 0; index < 10; index++)
		{
			sprintf_s(szNameBuffer, "%.*s", index, "................");
			SetDlgItemText(args->hDlg, IDC_SP_UPDATE, szNameBuffer);
			Sleep(args->iDelay / 10);
		}
		SetDlgItemText(args->hDlg, IDC_SP_UPDATE, szOldEntry);
	}
	
	index = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	ListView_GetItemText(hList, index, 0, szOldEntry, sizeof(szOldEntry) / sizeof(szOldEntry[0]));

	GetWindowText(hEdit, szNameBuffer, sizeof(szNameBuffer) / sizeof (szNameBuffer[0]));
	SearchPlayer(szNameBuffer, args->pvFound);

	GetDlgItemText(args->hDlg, IDC_SP_RESULTS, szNameBuffer, sizeof(szNameBuffer) / sizeof (szNameBuffer[0]));
	pStrTokResult = strtok (szNameBuffer, ":");
	sprintf_s(szNameBuffer, sizeof(szNameBuffer) / sizeof (szNameBuffer[0]), "%s: %d", pStrTokResult, args->pvFound->size());
	SetDlgItemText(args->hDlg, IDC_SP_RESULTS, szNameBuffer);
	
	ListView_DeleteAllItems(hList);

	if (args->pvFound->size() == 0) //if no results, add the "no player found" text in the correct language
	{
		LoadString(g_hInst, IDS_SP_NOPLAYERFOUND, szNameBuffer, sizeof(szNameBuffer)/sizeof(szNameBuffer[0]));
		LvItem.mask = LVIF_TEXT | LVIF_PARAM;
		LvItem.lParam = (-1);
		LvItem.iSubItem = 0;
		LvItem.iItem = 0;
		LvItem.pszText = szNameBuffer;
		ListView_InsertItem(hList, &LvItem);
		
		if (args->iDelay)
		{
			EnableWindow(GetDlgItem(args->hDlg, IDC_SP_UPDATE), TRUE);
			EnableWindow(GetDlgItem(args->hDlg, IDC_SP_EDIT), TRUE);
		}
		delete args;
		return;
	}
	
	for (size_t i = 0; i < args->pvFound->size(); i++) //add found results, also set lParam of each item to index of that player in pvFound
	{
		LvItem.mask = LVIF_TEXT | LVIF_PARAM;
		LvItem.lParam = i;
		LvItem.iItem = 0;
		LvItem.iSubItem = 0;
		LvItem.pszText = (LPSTR) g_mServers[args->pvFound->at(i).first].vPlayers[args->pvFound->at(i).second].sName.c_str();
		index = ListView_InsertItem(hList, &LvItem);

		LvItem.iSubItem = 1;
		LvItem.pszText = (LPSTR) g_mServers[args->pvFound->at(i).first].sHostName.c_str();
		SendMessage (hList, LVM_SETITEMTEXT, index, (LPARAM) &LvItem);

		if (strcmp(g_mServers[args->pvFound->at(i).first].vPlayers[args->pvFound->at(i).second].sName.c_str(), szOldEntry) == 0)
		{
			ListView_SetItemState(hList, -1, 0, 0x000F);
			ListView_SetItemState(hList, index, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
		}
	}

	ListView_SortItems(hList, OnSearchPlayerDlgSort, (LPARAM) args->pvFound);

	index = ListView_GetNextItem(hList, -1, LVNI_SELECTED | LVNI_FOCUSED);
	ListView_EnsureVisible(hList, index, FALSE);

	if (args->iDelay)
	{
		EnableWindow(GetDlgItem(args->hDlg, IDC_SP_UPDATE), TRUE);
		EnableWindow(GetDlgItem(args->hDlg, IDC_SP_EDIT), TRUE);
	}
	delete args;
}

// Message handlers for search players dialog
static void OnSearchPlayerDlgUpdate (HWND hDlg, int iDelay, std::vector <std::pair<std::string, int> > * pvFound)
{
	RefreshList();
	SearchThreadArgs* args = new SearchThreadArgs;
	args->hDlg = hDlg;
	args->pvFound = pvFound;
	args->iDelay = iDelay;
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SearchThread, (LPVOID)args, 0, NULL);
}

static void OnSearchPlayerDlgEditChange(HWND hDlg, std::vector <std::pair<std::string, int> > * pvFound)
{
	SearchThreadArgs* args = new SearchThreadArgs;
	args->hDlg = hDlg;
	args->pvFound = pvFound;
	args->iDelay = 0;
	SearchThread(args);
}

static void OnSearchPlayerDlgSelChange (HWND hDlg, std::vector <std::pair<std::string, int> > * pvFound)
{
	int iListId;
	LVITEM LvItem;

	memset(&LvItem, 0, sizeof(LvItem));
	LvItem.iItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_SP_LIST), -1, LVNI_SELECTED | LVNI_FOCUSED);
	LvItem.mask = LVIF_PARAM;

	ListView_GetItem(GetDlgItem(hDlg, IDC_SP_LIST), &LvItem);
	
	if (LvItem.lParam == (-1)) //the lParam of the "no player found" text is -1
		return;

	//select the server the player is playing on in the main windoww
	iListId = GetListIDFromAddress(pvFound->at(LvItem.lParam).first.c_str());
	if (iListId >= 0)
	{
		ListView_SetItemState(g_hServerList, -1, 0, 0x000F);
		ListView_SetItemState(g_hServerList, iListId, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
		ListView_EnsureVisible(g_hServerList, iListId, FALSE);
		UpdateInfoLists(iListId, false);
	}
}

static void OnSearchPlayerDlgGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
	DWORD dwBaseUnits = GetDialogBaseUnits();
	lpMinMaxInfo->ptMinTrackSize.x = MulDiv(100, LOWORD(dwBaseUnits), 4);
	lpMinMaxInfo->ptMinTrackSize.y = MulDiv(120, HIWORD(dwBaseUnits), 8);
}

static void OnSearchPlayerDlgSize (HWND hDlg, UINT state, int cx, int cy)
{
	DWORD dwBaseUnits;
	int iMW, iMH; //multipliers to calculate pixel coordinates out of dialog unit coordinates

	dwBaseUnits = GetDialogBaseUnits();
	iMW = LOWORD(dwBaseUnits) / 4; //Multiplier width for base units to pixels
    iMH = HIWORD(dwBaseUnits) / 8; //Multiplier height for base units to pixels
	
	MoveWindow (GetDlgItem(hDlg, IDC_SP_EDIT), 5*iMW, 16*iMH, cx - 10*iMW, 12*iMH, TRUE);
	MoveWindow (GetDlgItem(hDlg, IDC_SP_LIST), 5*iMW, 50*iMH, cx - 10*iMW, cy- 72*iMH, TRUE);
	MoveWindow (GetDlgItem(hDlg, IDC_SP_UPDATE), cx - 84*iMW, cy - 17*iMH, 37*iMW, 11*iMH, TRUE);
	MoveWindow (GetDlgItem(hDlg, IDOK),          cx - 42*iMW, cy - 17*iMH, 37*iMW, 11*iMH, TRUE);
	RedrawWindow(GetDlgItem(hDlg, IDOK), NULL, NULL, RDW_ERASE | RDW_INVALIDATE);

	ListView_SetColumnWidth(GetDlgItem(hDlg, IDC_SP_LIST), 0, -1);
	ListView_SetColumnWidth(GetDlgItem(hDlg, IDC_SP_LIST), 1, -2);

}

static void OnSearchPlayerDlgCommand (HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
	static std::vector <std::pair<std::string, int> > vFound;

	switch (id)
	{
		case IDOK:
		case IDCANCEL:
			g_hSearchPlayerDlg = 0;
			EndDialog(hDlg, id);
			break;
		
		case IDC_SP_UPDATE:
			OnSearchPlayerDlgUpdate (hDlg, 1000, &vFound);
			break;

		case IDC_SP_EDIT:
			if (codeNotify == EN_CHANGE)
				OnSearchPlayerDlgEditChange(hDlg, &vFound);
			break;

		case IDC_SP_LIST: //will only happen if sent manually from OnSearchPlayerDlgNotify, 
			if (codeNotify == 0x800) //select right server in main window
				OnSearchPlayerDlgSelChange(hDlg, &vFound);
			else if (codeNotify = 0x800 + 1) //resort the items
				ListView_SortItems(GetDlgItem(hDlg, IDC_SP_LIST), OnSearchPlayerDlgSort, &vFound);
			break;
	}
}

static BOOL OnSearchPlayerDlgInit (HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
	LVCOLUMN LvCol;
	char szBuffer[256];
	RECT rect;

	ListView_SetExtendedListViewStyle(GetDlgItem(hDlg, IDC_SP_LIST), LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	memset(&LvCol, 0, sizeof(LvCol));
	LvCol.mask = LVCF_TEXT | LVCF_SUBITEM;
	LvCol.pszText  = szBuffer;
	
	LvCol.iSubItem = 0;
	LoadString(g_hInst, IDS_SP_PLAYER, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]));
	ListView_InsertColumn(GetDlgItem(hDlg, IDC_SP_LIST), 0, &LvCol);

	LvCol.iSubItem = 1;
	LvCol.pszText  = szBuffer;
	LoadString(g_hInst, IDS_SP_ONSERVER, szBuffer, sizeof(szBuffer)/sizeof(szBuffer[0]));
	ListView_InsertColumn(GetDlgItem(hDlg, IDC_SP_LIST), 1, &LvCol);

	//fill the listbox with all players currently playing, manually calling the function is not possible
	//because we dont have vFound in this scope.
	SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_SP_EDIT, EN_CHANGE), (LPARAM) GetDlgItem(hDlg, IDC_SP_EDIT));
	
	//position all controls and adapt column width
	GetClientRect(hDlg, &rect);
	OnSearchPlayerDlgSize(hDlg, SIZE_RESTORED, rect.right, rect.bottom); 
	return TRUE;
}

//used to forward all listview messages that come in as WM_NOTIFY messages
//to the WM_COMMAND handler because it holds the static vFound
static LRESULT OnSearchPlayerDlgNotify (HWND hDlg, int idFrom, NMHDR* pNMHdr)
{
	if( (pNMHdr->code == LVN_ITEMCHANGED) && (pNMHdr->idFrom == IDC_SP_LIST) )
	{
		if  ( ( ((NMLISTVIEW*)pNMHdr)->uNewState & (LVIS_FOCUSED | LVIS_SELECTED) ) == (LVIS_FOCUSED | LVIS_SELECTED) )
			SendMessage (hDlg, WM_COMMAND, MAKEWPARAM(IDC_SP_LIST, 0x800), 0);
	}
	else if ( (pNMHdr->code == LVN_COLUMNCLICK) &&(pNMHdr->idFrom == IDC_SP_LIST) )
	{
		g_iPlayerSearchSortOrder = (abs(g_iPlayerSearchSortOrder) == ((NMLISTVIEW*)pNMHdr)->iSubItem + 1)
									? g_iPlayerSearchSortOrder * (-1)
									: ((NMLISTVIEW*)pNMHdr)->iSubItem + 1;

		SendMessage (hDlg, WM_COMMAND, MAKEWPARAM(IDC_SP_LIST, 0x800 + 1), 0);
	}
	return 0;
}

static LRESULT CALLBACK SearchPlayerDlg (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hDlg, WM_SIZE,			OnSearchPlayerDlgSize);
		HANDLE_MSG(hDlg, WM_NOTIFY,			OnSearchPlayerDlgNotify);
		HANDLE_MSG(hDlg, WM_COMMAND,		OnSearchPlayerDlgCommand);
		HANDLE_MSG(hDlg, WM_INITDIALOG,		OnSearchPlayerDlgInit);
		HANDLE_MSG(hDlg, WM_GETMINMAXINFO,	OnSearchPlayerDlgGetMinMaxInfo);
	}
    return FALSE;
}

static LRESULT CALLBACK OnSearchPlayerDlgSort (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int iResult;
	std::vector <std::pair<std::string, int> > vFound = *((std::vector <std::pair<std::string, int> > *) lParamSort);

	if (g_iPlayerSearchSortOrder == -2) //-2 for 2nd column, ascending
	{
		iResult = _stricmp(g_mServers[vFound.at(lParam2).first].sHostName.c_str(),
							g_mServers[vFound.at(lParam1).first].sHostName.c_str());
		iResult = (iResult != 0) ? iResult : _stricmp(g_mServers[vFound.at(lParam1).first].vPlayers[vFound.at(lParam1).second].sName.c_str(),
											 g_mServers[vFound.at(lParam2).first].vPlayers[vFound.at(lParam2).second].sName.c_str());
	}

	else if (g_iPlayerSearchSortOrder == -1) //-1 for 1st column, ascending
	{
		iResult = _stricmp(g_mServers[vFound.at(lParam2).first].vPlayers[vFound.at(lParam2).second].sName.c_str(),
							g_mServers[vFound.at(lParam1).first].vPlayers[vFound.at(lParam1).second].sName.c_str());
		iResult = (iResult != 0) ? iResult : _stricmp(g_mServers[vFound.at(lParam1).first].sHostName.c_str(),
											 g_mServers[vFound.at(lParam2).first].sHostName.c_str());
	}

	else if (g_iPlayerSearchSortOrder == 1) //1 for for 1st column, descending
	{
		iResult = _stricmp(g_mServers[vFound.at(lParam1).first].vPlayers[vFound.at(lParam1).second].sName.c_str(),
							g_mServers[vFound.at(lParam2).first].vPlayers[vFound.at(lParam2).second].sName.c_str());
		iResult = (iResult != 0) ? iResult : _stricmp(g_mServers[vFound.at(lParam1).first].sHostName.c_str(),
											 g_mServers[vFound.at(lParam2).first].sHostName.c_str());
	}

	else if (g_iPlayerSearchSortOrder == 2) //2 for 2nd column, descending
	{
		iResult = _stricmp(g_mServers[vFound.at(lParam1).first].sHostName.c_str(),
							g_mServers[vFound.at(lParam2).first].sHostName.c_str());
		iResult = (iResult != 0) ? iResult : _stricmp(g_mServers[vFound.at(lParam1).first].vPlayers[vFound.at(lParam1).second].sName.c_str(),
											 g_mServers[vFound.at(lParam2).first].vPlayers[vFound.at(lParam2).second].sName.c_str());
	}
	else
		iResult = 0;

	return iResult;
}