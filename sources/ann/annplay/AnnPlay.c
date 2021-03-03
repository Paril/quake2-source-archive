/*@@ Wedit generated application. Written Sun May 02 22:54:43 1999
 @@header: d:\games\quake2\ann\annplay\AnnPlayres.h
 @@resources: d:\games\quake2\ann\annplay\AnnPlay.rc
 Do not edit outside the indicated areas */
/*<---------------------------------------------------------------------->*/
/*<---------------------------------------------------------------------->*/
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string.h>
#include "AnnPlayres.h"
#include "winamp.h"
#include <stdio.h>

void Sleep(DWORD cMilliseconds);
int initstartup(STARTUPINFO *startup);

/*<---------------------------------------------------------------------->*/
HINSTANCE hInst;		// Instance handle
HWND hwndMain;		//Main window handle

LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

/*<---------------------------------------------------------------------->*/
/*@@0->@@*/
static BOOL InitApplication(void)
{
	WNDCLASS wc;

	memset(&wc,0,sizeof(WNDCLASS));
	wc.style = CS_HREDRAW|CS_VREDRAW ;
	wc.lpfnWndProc = (WNDPROC)MainWndProc;
	wc.hInstance = hInst;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = "AnnPlayWndClass";
	wc.lpszMenuName = MAKEINTRESOURCE(IDMAINMENU);
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);
	wc.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDAPPLICON));
	if (!RegisterClass(&wc))
		return 0;
/*@@0<-@@*/
	// ---TODO--- Call module specific initialization routines here

	return 1;
}

/*<---------------------------------------------------------------------->*/
/*@@1->@@*/
HWND CreateAnnPlayWndClassWnd(void)
{
	return CreateWindow("AnnPlayWndClass","AnnPlay",
		WS_VISIBLE|WS_CAPTION|WS_BORDER|WS_SYSMENU|WS_MINIMIZE,
		CW_USEDEFAULT,0,112,112,
		NULL,
		NULL,
		hInst,
		NULL);
}
/*@@1<-@@*/
/*<---------------------------------------------------------------------->*/
/* --- The following code comes from e:\utils\lcc\lib\wizard\defOnCmd.tpl. */
void MainWndProc_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id) {
		// ---TODO--- Add new menu commands here
		/*@@NEWCOMMANDS@@*/
		case IDM_EXIT:
		PostMessage(hwnd,WM_CLOSE,0,0);
		break;
	}
}

/*<---------------------------------------------------------------------->*/
/*@@2->@@*/
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch (msg) {
/*@@3->@@*/
	case WM_COMMAND:
		HANDLE_WM_COMMAND(hwnd,wParam,lParam,MainWndProc_OnCommand);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd,msg,wParam,lParam);
	}
/*@@3<-@@*/
	return 0;
}
/*@@2<-@@*/

/*<---------------------------------------------------------------------->*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	MSG msg;
	HANDLE hAccelTable;
	STARTUPINFO	startup;
	PROCESS_INFORMATION process;
	COPYDATASTRUCT cds;
	HWND hwnd_winamp;
	FILE *fp;
	char control;
	short n;
	char tempstr[1024];
	char tempstr1[1024];
	OPENFILENAME ofn;

	hInst = hInstance;
	if (!InitApplication())
		return 0;
	hAccelTable = LoadAccelerators(hInst,MAKEINTRESOURCE(IDACCEL));
	if ((hwndMain = CreateAnnPlayWndClassWnd()) == (HWND)0)
		return 0;
	ShowWindow(hwndMain,SW_SHOW);

	initstartup(&startup);
	startup.wShowWindow = SW_SHOWMINNOACTIVE;
	fp=fopen("ANNPLAY.INI","rt");
	if (!fp) {
		ofn.hwndOwner=NULL;
		ofn.hInstance=NULL;
		ofn.lpstrFilter="Winamp (winamp.exe)\0winamp.exe\0\0";
		ofn.lpstrCustomFilter=NULL;
		ofn.nMaxCustFilter=0;
		ofn.nFilterIndex=0;
		ofn.lpstrFile=tempstr;
		strcpy(tempstr,"winamp.exe");
		ofn.nMaxFile=sizeof(tempstr);
		ofn.lpstrFileTitle=NULL;
		ofn.nMaxFileTitle=0;
		ofn.lpstrInitialDir="c:\\program files\\winamp";
		ofn.lpstrTitle="Please locate your Winamp EXE file...";
		ofn.Flags=OFN_EXPLORER|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
		ofn.nFileOffset=0;
		ofn.nFileExtension=0;
		ofn.lpstrDefExt="exe";
		ofn.lCustData=0;
		ofn.lpfnHook=NULL;
		ofn.lpTemplateName=NULL;
		ofn.lStructSize=sizeof(DWORD)+sizeof(HWND)+sizeof(HINSTANCE)+sizeof(LPCTSTR)+sizeof(LPTSTR)
			+sizeof(DWORD)+sizeof(DWORD)+sizeof(LPTSTR)+sizeof(DWORD)+sizeof(LPTSTR)
			+sizeof(DWORD)+sizeof(LPCTSTR)+sizeof(LPCTSTR)+sizeof(DWORD)+sizeof(WORD)
			+sizeof(WORD)+sizeof(LPCTSTR)+sizeof(DWORD)+sizeof(LPOFNHOOKPROC)+sizeof(LPCTSTR);

		if (!GetOpenFileName(&ofn)) {
			MessageBox(0,"You must have Winamp 2.xx installed to use AnnPlay!","AnnPlay",MB_ICONSTOP|MB_OK);
			return 0;
		}
		else {
			fp=fopen("ANNPLAY.INI","wt");
			if (!fp) {
				MessageBox(0,"Error opening ANNPLAY.INI!","AnnPlay",MB_ICONSTOP|MB_OK);
				return 0;
			}
			fprintf(fp,"\"%s\"\n",ofn.lpstrFile);
			strcpy(tempstr,ofn.lpstrFile);
		}
	}
	else {
		fgets(tempstr,1024,fp);
	}
	fclose(fp);

	fp=fopen("ANNPLAY.M3U","rb");
	if (!fp) {
		MessageBox(0,"Error opening ANNPLAY.M3U!","AnnPlay",MB_ICONSTOP|MB_OK);
		return 0;
	}
	fclose(fp);

	strcpy(tempstr1,tempstr);
	strcat(tempstr1," ");
	strcat(tempstr1,"ap_null.m3u");

	CreateProcess(NULL, tempstr, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &startup, &process);
	while ((hwnd_winamp=FindWindow("Winamp v1.x",NULL))==NULL)
		;

	GetCurrentDirectory(sizeof(tempstr),tempstr);
	strcat(tempstr,"\\ANNPLAY.M3U");

	SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_DELETE);

	cds.dwData = IPC_PLAYFILE;
	cds.lpData = (void *) tempstr;
	cds.cbData = strlen((char *) cds.lpData)+1;
	SendMessage(hwnd_winamp,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);

	initstartup(&startup);
	startup.wShowWindow = SW_SHOWNORMAL;
	strcpy(tempstr,"..\\QUAKE2.EXE +set game ann +exec annplay.cfg ");
	strcat(tempstr,lpCmdLine);
	CreateProcess(NULL, tempstr, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, "..", &startup, &process);

	SendMessage(hwnd_winamp, WM_COMMAND,WINAMP_BUTTON4,0);

	while (WaitForSingleObject(process.hProcess,100)==WAIT_TIMEOUT) {
		fp=fopen("ANNPLAY.TXT","rt");
		if (fp) {
			fscanf(fp,"%c",&control);
			if (control=='F')
				fscanf(fp," %s",tempstr);
			fclose(fp);
			DeleteFile("ANNPLAY.TXT");
			switch (control) {
				case '<':
					SendMessage(hwnd_winamp,WM_COMMAND,WINAMP_BUTTON1,0);
					break;
				case 'P':
					SendMessage(hwnd_winamp,WM_COMMAND,WINAMP_BUTTON2,0);
					break;
				case 'W':
					SendMessage(hwnd_winamp,WM_COMMAND,WINAMP_BUTTON3,0);
					break;
				case 'S':
					SendMessage(hwnd_winamp,WM_COMMAND,WINAMP_BUTTON4,0);
					break;
				case '>':
					SendMessage(hwnd_winamp,WM_COMMAND,WINAMP_BUTTON5,0);
					break;
				case '+':
					for (n=0;n<5;n++)
						SendMessage(hwnd_winamp,WM_COMMAND,WINAMP_VOLUMEUP,0);
					break;
				case '-':
					for (n=0;n<5;n++)
						SendMessage(hwnd_winamp,WM_COMMAND,WINAMP_VOLUMEDOWN,0);
					break;
				case '[':
					SendMessage(hwnd_winamp,WM_COMMAND,WINAMP_REW5S,0);
					break;
				case ']':
					SendMessage(hwnd_winamp,WM_COMMAND,WINAMP_FFWD5S,0);
 					break;
				case 'F':
					GetCurrentDirectory(sizeof(tempstr1),tempstr1);
					strcat(tempstr1,"\\");
					strcat(tempstr1,tempstr);

					fp=fopen(tempstr1,"rb");
					if (!fp) {
						Beep(10000,1000);
						break;
					}
					fclose(fp);

					SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_DELETE);

					cds.dwData = IPC_PLAYFILE;
					cds.lpData = (void *) tempstr1;
					cds.cbData = strlen((char *) cds.lpData)+1;
					SendMessage(hwnd_winamp,WM_COPYDATA,(WPARAM)NULL,(LPARAM)&cds);

					break;
			}
		}
	}

	PostMessage(hwnd_winamp,WM_CLOSE,0,0);

	return msg.wParam;
}

int initstartup(STARTUPINFO *startup)
{
    startup->cb = sizeof(STARTUPINFO);
    startup->lpReserved = NULL;
    startup->lpDesktop = NULL;
    startup->lpTitle = NULL;
    startup->dwFlags = STARTF_USESHOWWINDOW;
    startup->cbReserved2 = 0;
    startup->lpReserved2 = NULL;

    return 1;
}

