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

#include "serverbrowser.h"
#include <winsock.h>
#include <stdio.h>
#include <io.h>


static SOCKET g_nUdpSocket = 0;
static HANDLE g_hWaitThread = INVALID_HANDLE_VALUE;
static HANDLE g_hUpdateThread = INVALID_HANDLE_VALUE;
static DWORD g_dwWaitThreadID = 0;
static DWORD g_dwUpdateThreadID = 0;
map<string, serverinfo_t> g_mServers;


// Remap for extended codes so they don't mess up the console.
static unsigned char char_remap[256] = {
	'\0','-', '-', '-', '_', '*', 't', '.', 'N', '-', '\n','#', '.', '>', '*', '*',
	'[', ']', '@', '@', '@', '@', '@', '@', '<', '>', '.', '-', '*', '-', '-', '-',
	' ', '!', '\"','#', '$', '%', '&', '\'','(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\',']', '^', '_',
	'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', '<',
	'(', '=', ')', '^', '!', 'O', 'U', 'I', 'C', 'C', 'R', '#', '?', '>', '*', '*',
	'[', ']', '@', '@', '@', '@', '@', '@', '<', '>', '*', 'X', '*', '-', '-', '-',
	' ', '!', '\"','#', '$', '%', '&', '\'','(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\',']', '^', '_',
	'`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~', '<'
};


// strip out the garbage characters such as color codes
// and map extendegggd ascii to something readable...
static void strip_garbage (char *out, const char *in)
{
	register const unsigned char *s;
	register unsigned char *sbuf;
	
	for (sbuf = (unsigned char *)out, s = (unsigned char *)in; *s; s++)
	{
		switch (*s)
		{
		case CHAR_COLOR:
			if (*(s+1))
				s++;
			break;
		case CHAR_ITALICS:
		case CHAR_UNDERLINE:
		case CHAR_ENDFORMAT:
			break;
		default:
			*sbuf = char_remap[*s];
			sbuf++;
		}
	}
	
	*sbuf = 0;
}


static int GetLine (FILE *fp, char *s, int size)
{
	int c;
	int i;

	for (i = 0, c = fgetc(fp); c != EOF && c != '\r' && c != '\n' && i < size - 1; i++, c = fgetc(fp))
		s[i] = c;
	
	if (c == '\r')
	{
		c = fgetc(fp);

		if (c != '\n')
			ungetc(c, fp); // shouldn't happen.
	}

	s[i] = 0;

	if (c == EOF && i < 1)
		return -1;

	return i - 1;
}


static void SockAddrToString (const struct sockaddr_in *tAddress, char *szOut)
{
	DWORD dwAddr = ntohl(tAddress->sin_addr.S_un.S_addr);

	sprintf(szOut, "%d.%d.%d.%d:%d",
		((dwAddr & 0xFF000000) >> 24) & 0xFF,
		((dwAddr & 0x00FF0000) >> 16) & 0xFF,
		((dwAddr & 0x0000FF00) >>  8) & 0xFF,
		((dwAddr & 0x000000FF) >>  0) & 0xFF, // because it looks nicer like this. ;)
		ntohs(tAddress->sin_port));
}


void PingServer (const char *sServer)
{
	char szHostname[256];
	struct sockaddr_in address;
	int port = 27910;
	struct hostent *pHostent;
	char *s;
	char szRequest[] = "\xFF\xFF\xFF\xFFstatus\n";
	char szAddress[32];
	map<string, serverinfo_t>::iterator mIterator;

	strncpyz(szHostname, sServer, sizeof(szHostname));
	s = strchr(szHostname, ':');

	if (s)
	{
		*s = 0;
		s++;
		port = atoi(s);
	}

	if (!(pHostent = gethostbyname(szHostname)))
	{
		SetStatus("Failed to resolve hostname.");
		return;
	}

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.S_un.S_addr = *(unsigned long *)pHostent->h_addr_list[0];
    address.sin_port = htons((short)port);
	SockAddrToString(&address, szAddress);

	mIterator = g_mServers.find(szAddress);

	if (mIterator == g_mServers.end())
	{
		g_mServers[szAddress].nPingStartTime = timeGetTime();
		g_mServers[szAddress].nVisible = 0; // don't draw it on the list until we get a response
		g_mServers[szAddress].bActive = false;
	}
	else
	{
		mIterator->second.nPingStartTime = timeGetTime();
		mIterator->second.bActive = false;
	}

	sendto(g_nUdpSocket, szRequest, strlen(szRequest), 0, (struct sockaddr *)&address, sizeof(address));
}


static void LoadListFile (const char *sFileName)
{
	FILE *fp = fopen(sFileName, "rb");
	char szLine[256];
	int nLen;

	if (fp)
	{
		nLen = GetLine(fp, szLine, sizeof(szLine));

		if (memcmp(szLine, "HTTP/", 5) == 0)
		{	// skip HTTP header on downloaded files
			while (GetLine(fp, szLine, sizeof(szLine)) > 0);
			nLen = GetLine(fp, szLine, sizeof(szLine));
		}

		while (nLen >= 0 && strcmp(szLine, "X") != 0)
		{
			PingServer(szLine);
			Sleep(30);
			nLen = GetLine(fp, szLine, sizeof(szLine));
		}

		SetStatus("");
	}
	else
	{
		SetStatus("Failed to open list file.");
	}
}


static void LoadListHttp (const char *sHttpSource)
{
	char szHostname[256];
	SOCKET nSocket;
	struct sockaddr_in address;
	int port = 80;
	struct hostent *pHostent;
	char *s;
	char szBuffer[1024];
	int nLen, nSent, nRead;
	FILE *fpTemp;

	strncpyz(szHostname, sHttpSource + sizeof("http://") - 1, sizeof(szHostname));
	s = strchr(szHostname, '/');

	if (s)
		*s = 0;

	if (!(pHostent = gethostbyname(szHostname)))
	{
		if (sHttpSource == "http://otb-server.de/serverlist.txt") 
		{
//			SetStatus("Failed to resolve mirror hostname. Trying to load an old local file.");
//			LoadList("templist.txt");									// Same as RefreshList
			SetStatus("Failed to resolve mirror hostname. Trying dplogin.com again...");
			LoadListHttp("http://dplogin.com/serverlist.php");			// Try dplogin.com and otb-server.de again and again
		}
		else 
		{
			SetStatus("Failed to resolve hostname. Trying mirror...");
			UpdateList_Mirror();
		}
		return;
	}

	//nSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	nSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.S_un.S_addr = *(unsigned long *)pHostent->h_addr_list[0];
    address.sin_port = htons((short)port);

	if (connect(nSocket, (struct sockaddr *)&address, sizeof(struct sockaddr)) == -1)
	{
		SetStatus("Failed to connect. Trying mirror...");
		UpdateList_Mirror();
		return;
	}

	nLen = _snprintf(szBuffer, sizeof(szBuffer), "GET %s HTTP/1.0\n\n", sHttpSource);
	szBuffer[sizeof(szBuffer)-1] = 0;
	nSent = send(nSocket, szBuffer, nLen, 0);

	if (nSent < nLen)
	{
		SetStatus("HTTP connection rejected. Trying mirror...");
		UpdateList_Mirror();
		return;
	}

	if (!(fpTemp = fopen("templist.txt", "wb")))
	{
		SetStatus("Failed to open temporary list. Trying mirror...");
		UpdateList_Mirror();
		return;
	}

	while ((nRead = recv(nSocket, szBuffer, sizeof(szBuffer), 0)) > 0)
		fwrite(szBuffer, nRead, 1, fpTemp);

	fclose(fpTemp);
	closesocket(nSocket);
	LoadListFile("templist.txt");
}


static void LoadListMT (const char *s)
{
	if (strnicmp(s, "http://", 7) == 0)
		LoadListHttp(s);
	else
		LoadListFile(s);
}


static void LoadList (const char *s)
{
	if (g_hUpdateThread != INVALID_HANDLE_VALUE)
		CloseHandle(g_hUpdateThread);

	g_hUpdateThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LoadListMT, (void *)s, 0, &g_dwUpdateThreadID);
}


void UpdateList (void)
{
	SetStatus("Updating List...");
	LoadList("http://dplogin.com/serverlist.php");
}


void UpdateList_Mirror (void)
{
//	SetStatus("Updating List (Mirror)...");
	LoadList("http://otb-server.de/serverlist.txt");
}

void RefreshList (void)
{
	SetStatus("Refreshing List...");
	LoadList("templist.txt");
}


static void InitWinsock (void)
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(1, 1), &wsadata);
	g_nUdpSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
}


static int SplitString (vector<string> &vOutStrings, const char *sString, char cDelim)
{
	const char *s, *s2;
	string str;

	vOutStrings.clear();
	
	for (s = sString; s; )
	{
		s2 = strchr(s+1, cDelim);
		
		if (s2)
		{
			str.assign(s, s2-s);
			s = s2+1;
		}
		else
		{
			str.assign(s);
			s = NULL;
		}

		vOutStrings.push_back(str);
	}

	return vOutStrings.size();
}


static bool FileExists (const char *sFileName)
{
	return _access(sFileName, 0) != -1;
}


bool ServerActive (const char *sAddress)
{
	map<string, serverinfo_t>::iterator mIterator;

	mIterator = g_mServers.find(sAddress);

	if (mIterator != g_mServers.end())
		return mIterator->second.bActive;
	else
		return false;
}


static bool MapExists (const char *sMapName)
{
	char szFullMapPath[1024];

	_snprintf(szFullMapPath, sizeof(szFullMapPath), "%s/pball/maps/%s.bsp", g_szGameDir, sMapName);
	
	return FileExists(szFullMapPath);
}


bool MapExistsForAddress (const char *sAddress)
{
	map<string, serverinfo_t>::iterator mIterator;

	mIterator = g_mServers.find(sAddress);

	if (mIterator != g_mServers.end())
		return mIterator->second.bMapExists;
	else
		return false;
}


static void MapPlayerColors (map<int, char> &mPlayerColors, char cColor, const char *sPlayerIDs)
{
	const char *s = sPlayerIDs;

	while (s = strchr(s, '!'))
	{
		s++;
		mPlayerColors[atoi(s)] = cColor;
	}
}


static void UpdateServer (const struct sockaddr_in *tFrom, const char *pData, int nDataLen)
{
	char szAddress[128];
	char *sServerInfo;
	vector<string> vLines;
	vector<string> vInfoStrings;
	int nLines, nInfoStrings;
	map<string, serverinfo_t>::iterator mIterator;
	string sAddress;
	int i;
	char szName[64];
	
	if (nDataLen < 10 || memcmp(pData, "\xFF\xFF\xFF\xFFprint\n", 10) != 0)
		return;

	if (!(sServerInfo = (char *)malloc(nDataLen + 1)))
		return;

	SockAddrToString(tFrom, szAddress);
	sAddress = szAddress;
	strncpy(sServerInfo, pData, nDataLen);
	sServerInfo[nDataLen] = '\0';
	nLines = SplitString(vLines, sServerInfo, '\n');
	free(sServerInfo);

	if (nLines > 1)
	{
		nInfoStrings = SplitString(vInfoStrings, vLines[1].c_str() + 1, '\\');
		mIterator = g_mServers.find(sAddress);
		
		if (mIterator == g_mServers.end())
			return; // got a response from a server we didn't query

		mIterator->second.nPing = timeGetTime() - mIterator->second.nPingStartTime;
		mIterator->second.nVisible = 1;
		mIterator->second.vPlayers.clear();
		mIterator->second.mPlayerColors.clear();

		playerinfo_t tPlayerInfo;

		for (i = 0; i < nLines - 2; ++i)
		{
			const char *s = vLines[i + 2].c_str();

			tPlayerInfo.nScore = atoi(s);
			s = strchr(s, ' ');
			
			if (s)
			{
				tPlayerInfo.nPing = atoi(s + 1);
				s = strchr(s, '\"');

				if (s)
				{	// The name is within quotes, so we need to skip the first and last character
					strip_garbage(szName, s + 1);

					if (*szName)
						tPlayerInfo.sName.assign(szName, strlen(szName) - 1);
					else
						tPlayerInfo.sName = "";
				}
				else
				{	// This shouldn't happen.
					tPlayerInfo.sName.assign("unknown", 7);
				}

				mIterator->second.vPlayers.push_back(tPlayerInfo);
			}
		}

		mIterator->second.nPlayers = mIterator->second.vPlayers.size();

		int size = vInfoStrings.size() / 2;
		const char *sVar, *sValue;
		mIterator->second.nMaxPlayers = 0;
		mIterator->second.nCertificated = 0;
		mIterator->second.nNeedPassword = 0;
		mIterator->second.nGLS = 0;
		mIterator->second.sHostName = "Unnamed";
		mIterator->second.sMapName = "unknown";
		mIterator->second.bMapExists = false;
		
		for (i = 0; i < size; ++i)
		{
			sVar = vInfoStrings[i*2].c_str();
			sValue = vInfoStrings[i*2 + 1].c_str();
			mIterator->second.mVars[sVar] = sValue;
			
			if (strcmp(sVar, "mapname") == 0)
			{
				mIterator->second.sMapName = sValue;
				mIterator->second.bMapExists = MapExists(sValue);
			}
			else if (strcmp(sVar, "sv_certificated") == 0)
			{
				mIterator->second.nCertificated = atoi(sValue);
			}
			else if (strcmp(sVar, "needpass") == 0)
			{
				mIterator->second.nNeedPassword = atoi(sValue);
			}
			else if (strcmp(sVar, "sv_login") == 0)
			{
				mIterator->second.nGLS = atoi(sValue);
			}
			else if (strcmp(sVar, "hostname") == 0)
			{
				char szTemp[1024];

				strip_garbage(szTemp, sValue);
				mIterator->second.sHostName = szTemp;
			}
			else if (strcmp(sVar, "maxclients") == 0)
			{
				mIterator->second.nMaxPlayers = atoi(sValue);
			}
			else if (sVar[0] == 'p' && sVar[2] == 0) // pb, pr, pp, py, po
			{
				MapPlayerColors(mIterator->second.mPlayerColors, sVar[1], sValue);
			}
		}

		mIterator->second.bActive = true;
		UpdateServerListGUI(szAddress, mIterator->second);
	}
}


static void WaitThread (void)
{
	fd_set tFDSet;
	int nRet;
	char szBuff[1024];
	struct sockaddr_in tFrom;
	int nFromLen = sizeof(tFrom);

	while (true)
	{
		FD_ZERO(&tFDSet);
		FD_SET(g_nUdpSocket, &tFDSet);
		select(0, &tFDSet, NULL, NULL, NULL); // wait for incoming data
		nRet = recvfrom(g_nUdpSocket, szBuff, sizeof(szBuff), 0, (struct sockaddr *)&tFrom, &nFromLen);

		if (nRet < 0)
			int nError = WSAGetLastError();
		else
			UpdateServer(&tFrom, szBuff, nRet);
	}
}


void InitApp (void)
{
	LoadSettings();
	InitWinsock();
	g_hWaitThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WaitThread, NULL, 0, &g_dwWaitThreadID);
	UpdateList();
}


void ShutdownApp (void)
{
	if (g_hWaitThread != INVALID_HANDLE_VALUE)
		CloseHandle(g_hWaitThread);

	closesocket(g_nUdpSocket);
}


bool LaunchGame (const char *sAddress, const char *sPassword)
{
#ifdef WIN32
	char szProgram[256];
	char szParameters[256];

	_snprintf(szProgram, sizeof(szProgram), "%s\\paintball2.exe", g_szGameDir);
	szProgram[sizeof(szProgram) - 1] = 0;
	_snprintf(szParameters, sizeof(szParameters), "%s%s +connect %s",
		sPassword ? "+set password " : "",
		sPassword ? sPassword : "", sAddress);
	szParameters[sizeof(szParameters) - 1] = 0;

	return (int)ShellExecute(NULL, NULL, szProgram, szParameters, g_szGameDir, SW_SHOWNORMAL) > 32;
#else
	char szCommandLine[512];

	_snprintf(szCommandLine, sizeof(szCommandLine), "%s\\paintball2.exe %s%s +connect %s",
		g_szGameDir, sPassword ? "+set password " : "",
		sPassword ? sPassword : "", sAddress);
	szCommandLine[sizeof(szCommandLine) - 1] = 0;

	return system(szCommandLine) >= 0;
#endif
}

//__IN_ sName	is the name to search for
//_OUT_ pvFound	is a pointer to a vector of pairs containing
//	1. the key value of the server in g_mServers
//		where the player is playing (= "IP:PORT" of that server)
//		--> access via g_mServers[vFound->at(i).first]
//
//	2. the player's index in the server's vPlayers vector
//		--> access via g_mServers[vFound->at(i).first].vPlayers[vFound->at(i).second]

void SearchPlayer (std::string sName, std::vector < std::pair<std::string, int> > * pvFound)
{
	std::string sPlayerNameCopy;

	pvFound->clear();

	//make the name we search for lowercase so the search is case insensitive
	for(int i = 0; sName[i]; i++)
		sName[i] = tolower(sName[i]);

	//loop through all servers
	for (std::map<std::string, serverinfo_t>::iterator iServer = g_mServers.begin(); iServer != g_mServers.end(); iServer++)
	{
		for (size_t iPlayer = 0; iPlayer < iServer->second.vPlayers.size(); iPlayer++) //loop through all players
		{
			//first copy the players name so the original one doesn't get modified, then transform to lowercase
			sPlayerNameCopy.assign(iServer->second.vPlayers[iPlayer].sName);

			for(int i = 0; sPlayerNameCopy[i]; i++)
				sPlayerNameCopy[i] = tolower(sPlayerNameCopy[i]);

			//if sName is empty       or  playername contains the name we're searching for
			if ((sName.length() == 0) || (sPlayerNameCopy.find(sName) != std::string::npos))
				pvFound->push_back(std::pair<std::string, int>(iServer->first, iPlayer));
		}
	}
}
