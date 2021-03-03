/*
Copyright (C) 2006 Nathan Wulf (jitspoe)

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

#pragma warning(disable: 4786)
#pragma warning(disable: 4996)

//Use ComCtl.dll V6
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include "resource.h"
#include <vector>
#include <string>
#include <map>

using namespace std;

#define PROTOCOL_VERSION_S "34"

void UpdateList (void);
void UpdateList_Mirror (void);
void RefreshList (void);
void SetStatus (const char *s);
void InitApp (void);
void ShutdownApp (void);
void LoadSettings (void);
bool LaunchGame (const char *sAddress, const char *sPassword);
bool MapExistsForAddress (const char *sAddress);
bool ServerActive (const char *sAddress);
void PingServer (const char *sServer);

//Edit by Richard
void SearchPlayer(std::string, std::vector < std::pair<std::string, int> > *);

typedef struct PLAYERINFO {
	string sName;
	int nScore;
	int nPing;
} playerinfo_t;

typedef struct SERVERINFO {
	int nPlayers;
	int nMaxPlayers;
	int nPingStartTime;
	int nPing;
	int nVisible;
	int nCertificated;
	int nNeedPassword;
	int nGLS;
	string sAddress;
	string sHostName;
	string sMapName;
	vector<playerinfo_t> vPlayers;
	map<string, string> mVars;
	bool bMapExists;
	bool bActive;
	map<int, char> mPlayerColors;
} serverinfo_t;

typedef enum COPYTYPE {
	COPYTYPE_ADDRESS,
	COPYTYPE_URL,
	COPYTYPE_FULL,
	COPYTYPE_NAMEIP
} copytype_t;

void UpdateServerListGUI (const char *sAddress, serverinfo_t &tServerInfo);

enum serverlist_offset
{
	SERVERLIST_CERTIFICATED_OFFSET,
	SERVERLIST_NEEDPASSWORD_OFFSET,
	SERVERLIST_GLS_OFFSET,
	SERVERLIST_HOSTNAME_OFFSET = 3,
	SERVERLIST_MAP_OFFSET,
	SERVERLIST_PLAYERS_OFFSET,
	SERVERLIST_PING_OFFSET,
	SERVERLIST_ADDRESS_OFFSET,
	SERVERLIST_OFFSET_MAX
};


enum playerlist_offset {
	PLAYERLIST_NAME_OFFSET = 0,
	PLAYERLIST_SCORE_OFFSET,
	PLAYERLIST_PING_OFFSET,
	PLAYERLIST_OFFSET_MAX
};

#define CHAR_ENDFORMAT	133
#define CHAR_UNDERLINE	134
#define CHAR_ITALICS	135
#define CHAR_COLOR		136

extern char g_szGameDir[256];
extern map<string, serverinfo_t> g_mServers;

#define HANDLE_WM_TRAY_ICON_NOTIFY_MESSAGE(hwnd, wParam, lParam, fn) \
    (fn)((hwnd), (wParam), (lParam))


static __inline void strncpyz (char *out, const char *in, size_t size)
{
	strncpy(out, in, size);
	out[size-1] = '\0';
}

