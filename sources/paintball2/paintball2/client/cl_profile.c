/*
Copyright (C) 2006 Digital Paint

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

// cl_profile.c -- profile related functions

#include "client.h"
#include "../qcommon/md5.h"
#include "../qcommon/simplecrypt.h"
#include "../qcommon/net_common.h"

#define PROFILE_LOGIN_NAME_LEN 64
#define PROFILE_PASSWORD_LEN 64

#define VISITFORUMS "Visit the forums at digitalpaint.org for help."

cvar_t *menu_profile_pass;
static char g_szPassHash[256] = "";
static char g_szRandomString[64] = "";
static int g_nVNInitUnique;
static qboolean g_bPassHashed = false;
static char g_szUserNameURL[256];
static char g_szUserName[64];
static int g_nUserID;
qboolean g_notified_of_new_version = false;

static const char *GetUniqueSystemString (void)
{
	static char szString[1024] = "";

	if (!*szString)
	{
#ifdef WIN32
		DWORD dwDrives;
		int nDrive;
		char szRootPath[4] = "X:\\";

		dwDrives = GetLogicalDrives();

		// Get the serial number off of the first fixed volume.  That should be pretty unique.
		for (nDrive = 'A'; nDrive <= 'Z'; ++nDrive)
		{
			if (dwDrives & 0x01)
			{
				szRootPath[0] = nDrive;

				if (GetDriveType(szRootPath) == DRIVE_FIXED)
				{
					char szVolumeName[MAX_PATH + 1];
					char szFileSystemName[MAX_PATH + 1];
					DWORD dwSerial, dwMaximumComponentLength, dwSysFlags;

					GetVolumeInformation(szRootPath, szVolumeName, sizeof(szVolumeName),
						&dwSerial, &dwMaximumComponentLength, &dwSysFlags,
						szFileSystemName, sizeof(szFileSystemName));

					if (dwSerial)
					{
						BinToHex(&dwSerial, sizeof(DWORD), szString, sizeof(szString));
						return szString;
					}
				}
			}

			dwDrives >>= 1;
		}

		strcpy(szString, "NODRIVES");
#else
		strcpy(szString, "NONWIN32"); // todo: get some unique string from system.
#endif
	}

	return szString;
}


static void CL_ProfileMustSelectDialog (void)
{
	M_PrintDialog("You must select a profile and type your password!");
}


void CL_ProfileEdit_f (void)
{
	if (Cmd_Argc() < 2)
	{
		CL_ProfileMustSelectDialog();
	}
	else
	{
		char szProfilePath[MAX_OSPATH];

		Com_sprintf(szProfilePath, sizeof(szProfilePath), BASEDIRNAME "/profiles/%s.prf", Cmd_Argv(1));

		if (!FileExists(szProfilePath))
			CL_ProfileMustSelectDialog();
		else
			Cbuf_AddText("menu profile_edit\n");
	}
}


static void StripNonAlphaNum (const char *sIn, char *sOut, size_t sizeOut)
{
	int i = 0;
	int c;
	char szNoGarbage[2048];

	strip_garbage(szNoGarbage, sIn, sizeof(szNoGarbage));
	sIn = szNoGarbage;

	// strip non alpha-numeric characters from profile name
	while ((c = *sIn) && i < sizeOut - 1)
	{
		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
			sOut[i++] = c;

		sIn++;
	}

	assert(i < sizeOut - 1);
	sOut[i] = 0;
}


static void WriteProfileFile (const char *sProfilePath, const char *sLoginName, const char *sPassword, qboolean bRawPass)
{
	FILE *fp;

	fp = fopen(sProfilePath, "wb");

	if (fp)
	{
		char szLoginName[PROFILE_LOGIN_NAME_LEN];
		char szPassword[PROFILE_PASSWORD_LEN];

		memset(szLoginName, 0, sizeof(szLoginName));
		Q_strncpyz(szLoginName, sLoginName, sizeof(szLoginName));
		fwrite("PB2PROFILE1.0\0", 14, 1, fp);
		fwrite(szLoginName, sizeof(szLoginName), 1, fp);

		if (bRawPass)
		{
			// Raw password data is data read straight from the profile file
			// (I think it's only used if the profile name is changed)
			fwrite(sPassword, PROFILE_PASSWORD_LEN, 1, fp);
		}
		else
		{
			// TODO: encrypt this using something unique to this machine and store as MD5 string
			memset(szPassword, 0, sizeof(szPassword));
			Q_strncpyz(szPassword, sPassword, sizeof(szPassword));
			fwrite(szPassword, sizeof(szPassword), 1, fp);
		}

		fclose(fp);
	}

}


void CL_ProfileEdit2_f (void)
{
	if (Cmd_Argc() == 4)
	{
		char szProfileInPath[MAX_OSPATH];
		char szProfileOutPath[MAX_OSPATH];
		char szProfileData[2048];
		char szProfileOutName[64];
		int nLen;
		FILE *fp;
		char *sLogin = Cmd_Argv(3);

		StripNonAlphaNum(Cmd_Argv(2), szProfileOutName, sizeof(szProfileOutName));
		
		if (!*szProfileOutName)
			return;

		Com_sprintf(szProfileInPath, sizeof(szProfileInPath), "%s/profiles/%s.prf", FS_Gamedir(), Cmd_Argv(1));
		Com_sprintf(szProfileOutPath, sizeof(szProfileOutPath), "%s/profiles/%s.prf", FS_Gamedir(), szProfileOutName);

		if (fp = fopen(szProfileInPath, "rb"))
		{
			nLen = fread(szProfileData, 1, sizeof(szProfileData), fp);
			fclose(fp);

			if ((!FileExists(szProfileOutPath) || Q_strcasecmp(szProfileInPath, szProfileOutPath) == 0))
			{
				char *sPassword = "";
				qboolean bRawPass = false;

				if (memcmp(szProfileData, "PB2PROFILE1.0", sizeof("PB2PROFILE1.0")) == 0)
				{
					sPassword = szProfileData + sizeof("PB2PROFILE1.0") + PROFILE_LOGIN_NAME_LEN;
					bRawPass = true;
				}

				WriteProfileFile(szProfileOutPath, sLogin, sPassword, bRawPass);

				if (Q_strcasecmp(szProfileInPath, szProfileOutPath) != 0)
					remove(szProfileInPath);

				Cvar_Set("menu_profile_file", szProfileOutName);
				Cbuf_AddText("menu pop\n");
				M_ReloadMenu();
			}
		}
	}
}


void CL_ProfileAdd_f (void)
{
	char szProfileName[64];
	char szProfilePath[MAX_OSPATH];
	const char *sLoginName;
	char szBuff[1024];

	if (Cmd_Argc() < 3)
		return;

	StripNonAlphaNum(Cmd_Argv(1), szProfileName, sizeof(szProfileName));
	sLoginName = Cmd_Argv(2);

	if (strlen(szProfileName) < 1)
		return;

	Com_sprintf(szProfilePath, sizeof(szProfilePath), "%s/profiles", FS_Gamedir());
	Sys_Mkdir(szProfilePath);
	Com_sprintf(szProfilePath, sizeof(szProfilePath), "%s/profiles/%s.prf", FS_Gamedir(), szProfileName);
	WriteProfileFile(szProfilePath, sLoginName, "", false);
	Com_sprintf(szBuff, sizeof(szBuff), "menu pop;menu pop;set name \"%s\";set menu_profile_file %s\n", sLoginName, szProfileName);
	Cbuf_AddText(szBuff);
	M_ReloadMenu();
}


void CL_ProfileSelect_f (void)
{
	char szProfilePath[1024];
	FILE *fp;
	static qboolean autologin = true;

	if (Cmd_Argc() < 2)
		return;

	Com_sprintf(szProfilePath, sizeof(szProfilePath), "%s/profiles/%s.prf", FS_Gamedir(), Cmd_Argv(1));
	
	if (fp = fopen(szProfilePath, "rb"))
	{
		char szProfileData[2048];
		int nDataLen;
		char *s;

		memset(szProfileData, 0, sizeof(szProfileData));
		nDataLen = fread(szProfileData, 1, sizeof(szProfileData), fp);
		fclose(fp);

		if (memcmp(szProfileData, "PB2PROFILE1.0", sizeof("PB2PROFILE1.0")) != 0)
		{
			Cvar_Set("menu_profile_pass", "");
			Cvar_SetValue("menu_profile_remember_pass", 0.0f);
			return;
		}

		// TODO: Decrypt this once encryption implemented.
		s = szProfileData + sizeof("PB2PROFILE1.0") + PROFILE_LOGIN_NAME_LEN;
		Cvar_Set("menu_profile_pass", s);
		Cvar_SetValue("menu_profile_remember_pass", *s ? 1.0f : 0.0f);

		if (Cvar_Get("menu_profile_autologin", "0", CVAR_ARCHIVE)->value && autologin)
		{
			if (*Cvar_VariableString("menu_profile_file") && *Cvar_VariableString("menu_profile_pass"))
				Cbuf_AddText("profile_login $menu_profile_file $menu_profile_pass\n");

			autologin = false; // only autologin when the menu is first loaded
		}
	}

	menu_profile_pass->modified = false;
}


qboolean CL_HasProfile ()
{
	int filecount;
	char **files = FS_ListFiles(BASEDIRNAME "/profiles/*.prf", &filecount, 0, 0, true);
	
	if (!files)
		return false;

	FS_FreeFileList(files, filecount);

	return filecount > 0;
}


static void urlencode (char *out, int out_size, const char *in)
{
	const char *p;
	char *po = out;
	char *poe = out + out_size - 4;
	int c;

	for (p = in, po = out; *p; p++)
	{
		if (po >= poe)
		{
			*po = 0;
			return;
		}

		c = *(unsigned char*)p;

		if (c == ' ')
		{
			*po++ = '+';
		}
		else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
			(c >= '0' && c <= '9'))
		{
			*po++ = c;
		}
		else
		{
			Com_sprintf(po, out_size, "%%%02x", c);
			po += 3;
		}
	}

	*po = 0;
}


void CL_ProfileLogin_f (void)
{
	char szPassword[256];
	char *sPassword;
	netadr_t adr;
	char szProfileFile[MAX_OSPATH];
	char *pFileContents;

	if (Cmd_Argc() < 3)
	{
		CL_ProfileMustSelectDialog();
		return;
	}

	Com_sprintf(szProfileFile, sizeof(szProfileFile), "profiles/%s.prf", Cmd_Argv(1));
	
	if (FS_LoadFile(szProfileFile, (void **)&pFileContents) > 0)
	{
		if (memcmp(pFileContents, "PB2PROFILE1.0\0", 14) == 0)
		{
			strip_garbage(g_szUserName, pFileContents + 14, sizeof(g_szUserName));
		}
		else
		{
			Com_Printf("Invalid profile file: %s\n", szProfileFile);
			M_PrintDialog(va("Invalid profile file: %s", szProfileFile));
			FS_FreeFile(pFileContents);
			return;
		}

		FS_FreeFile(pFileContents);
	}
	else
	{
		Com_Printf("Failed to read profile file: %s\n", szProfileFile);
		M_PrintDialog(va("Failed to read profile file: %s\n", szProfileFile));
		return;
	}

	urlencode(g_szUserNameURL, sizeof(g_szUserNameURL), g_szUserName);
	sPassword = Cmd_Argv(2);

	if (!*g_szUserName || !*sPassword)
	{
		CL_ProfileMustSelectDialog();
		return; // blank value (shouldn't happen)
	}

	if (menu_profile_pass->modified)
	{
		// Generate password hash (raw password is never sent or stored).
		Com_sprintf(szPassword, sizeof(szPassword), "%sDPLogin001", sPassword);
		Com_MD5HashString(szPassword, strlen(szPassword), g_szPassHash, sizeof(g_szPassHash));
		g_bPassHashed = false;
	}
	else
	{
		// Password was saved.  Already hashed.
		Q_strncpyz(g_szPassHash, sPassword, sizeof(g_szPassHash));
		g_bPassHashed = true;
	}

	NET_Config(true); // allow remote

	if (NET_StringToAdr("dplogin.com:27900", &adr))
	{
		Netchan_OutOfBandPrint(NS_CLIENT, adr, "vninit\nusername=%s&uniqueid=%d", g_szUserNameURL, g_nVNInitUnique);
		Cbuf_AddText("menu profile_login\n");
	}
	else
	{
		const char *error = "Failed to resolve login server domain.  Make sure nothing is blocking Paintball 2's access to dplogin.com:27900.  " VISITFORUMS;
		Com_Printf("%s\n", error);
		M_PrintDialog(error);
	}

	Stats_Shutdown();
	Stats_Init();
}

#define MAX_BUFFER_SIZE 1024

static int SizebuffCopyZ (char *sOutString, int nOutStringSize, sizebuf_t *ptData)
{
	int nSize;

	if (!ptData)
	{
		sOutString[0] = 0;
		return 0;
	}

	nSize = ptData->cursize;
	
	if (nSize > nOutStringSize - 1)
		nSize = nOutStringSize - 1;

	memcpy(sOutString, ptData->data, nSize);
	sOutString[nSize] = 0;

	return nSize;
}


void CL_VNInitResponse (netadr_t adr_from, sizebuf_t *ptData)
{
	char szDataBack[MAX_BUFFER_SIZE];
	char *s, *s2;
	int i;
	char szPassHash2[256];
	char szUserID[32];

	if (!ptData)
		return;

	SizebuffCopyZ(szDataBack, sizeof(szDataBack), ptData);

	// Verify that this is a valid packet, not a malicious one
	if (s = strstr(szDataBack, "uniqueid:"))
	{
		s += sizeof("uniqueid:");
		
		if (atoi(s) != g_nVNInitUnique)
		{
			Com_Printf("Uniqueid mismatch.\n");
			assert(0);
			return;
		}
	}
	else
	{
		Com_Printf("Uniqueid missing.\n");
		assert(0);
		return;
	}

	// Obtain random string 
	s = strstr(szDataBack, "randstr:");
	s2 = strstr(szDataBack, "userid:");

	if (!s || !s2)
	{
		M_PopMenu("profile_login"); // Remove "Connecting to login server..." screen.

		if (s = strstr(szDataBack, "ERROR:"))
		{
			Com_Printf("%s\n", s);
			M_PrintDialog(s);
		}
		else
		{
			Com_Printf("ERROR: CL_VNInitResponse(): Unknown response from login server.\n");
			M_PrintDialog("Unknown response from login server.  " VISITFORUMS);
			assert(0);
		}

		return;
	}

	s += sizeof("randstr:");
	s2 += sizeof("userid:");
	i = 0;

	while (*s >= '0' && i < sizeof(g_szRandomString) - 1)
		g_szRandomString[i++] = *s++;

	g_szRandomString[i] = 0;
	i = 0;

	while  (*s2 >= '0' && *s2 <= '9' && i < sizeof(szUserID) - 1)
		szUserID[i++] = *s2++;

	szUserID[i] = 0;
	g_nUserID = atoi(szUserID);

	if (!g_bPassHashed)
	{
		char szPassword[256];

		// Salt with userid
		Com_sprintf(szPassword, sizeof(szPassword), "%s%s", g_szPassHash, szUserID);
		Com_MD5HashString(szPassword, strlen(szPassword), g_szPassHash, sizeof(g_szPassHash));
	}

	// Use HMAC with MD5 to authenticate message (random string) with login server.
	Com_HMACMD5String(g_szPassHash, strlen(g_szPassHash), g_szRandomString,
		strlen(g_szRandomString), szPassHash2, sizeof(szPassHash2));
	Netchan_OutOfBandPrint(NS_CLIENT, adr_from, "clvn\nusername=%s&pwhash=%s&uniqueid=%d",
		g_szUserNameURL, szPassHash2, g_nVNInitUnique);
}


void CL_VNResponse (netadr_t adr_from, sizebuf_t *ptData)
{
	char szDataBack[MAX_BUFFER_SIZE];
	char *s;
	int i;
	char szConnectID[16] = "";

	SizebuffCopyZ(szDataBack, sizeof(szDataBack), ptData);
	s = strstr(szDataBack, "GameLoginStatus: PASSED");

	M_PopMenu("profile_login"); // Remove "Connecting to login server..." screen.

	if (!s)
	{
		// breakpoint
		if (s = strstr(szDataBack, "ERROR:"))
		{
			Com_Printf("%s\n", s);
			M_PrintDialog(s);
		}
		else if (s = strstr(szDataBack, "GameLoginStatus: FAILED"))
		{
			char translated_format[1024];
			char translated_text[1024];
			translate_string(translated_format, sizeof(translated_format), "Incorrect password for username %s.");
			Com_sprintf(translated_text, sizeof(translated_text), translated_format, g_szUserName);
			Com_Printf("%s\n", translated_text);
			M_PrintDialog(translated_text);
		}
		else
		{
			Com_Printf("ERROR: Unknown response from login server.\n");
			M_PrintDialog("Unknown response from login server.  " VISITFORUMS);
		}

		return;
	}

	// Verify that this is a valid packet, not a malicious one
	if (s = strstr(szDataBack, "uniqueid:"))
	{
		s += sizeof("uniqueid:");
		
		if (atoi(s) != g_nVNInitUnique)
		{
			Com_Printf("Uniqueid mismatch.\n");
			M_PrintDialog("Uniqueid mismatch.  " VISITFORUMS);
			assert(0);
			return;
		}
	}
	else
	{
		Com_Printf("Uniqueid missing.\n");
		M_PrintDialog("Uniqueid missing.  " VISITFORUMS);
		assert(0);
		return;
	}

	s = strstr(szDataBack, "randstr:");

	if (!s)
	{
		if (s = strstr(szDataBack, "ERROR:"))
		{
			Com_Printf("%s\n", s);
			M_PrintDialog(s);
		}
		else
		{
			Com_Printf("ERROR: Unknown response from login server.\n");
			M_PrintDialog("Unknown response from login server.  " VISITFORUMS);
		}

		assert(0);
		return;
	}

	s += sizeof("randstr:");
	i = 0;

	while (*s >= '0' && i < sizeof(g_szRandomString) - 1)
		g_szRandomString[i++] = *s++;

	g_szRandomString[i] = 0;
	s = strstr(szDataBack, "connectid:");

	if (s)
	{
		s += sizeof("connectid:");
		i = 0;

		while (*s >= '0' && i < sizeof(szConnectID) - 1)
			szConnectID[i++] = *s++;

		szConnectID[i] = 0;
	}

	if (e.r)
		e.r(g_szRandomString, g_nUserID, atoi(szConnectID), 0);

	if (menu_profile_pass->modified)
	{
		char szProfileOutPath[MAX_OSPATH];

		Com_sprintf(szProfileOutPath, sizeof(szProfileOutPath), "%s/profiles/%s.prf", FS_Gamedir(), Cvar_Get("menu_profile_file", "unnamed", 0)->string);
		WriteProfileFile(szProfileOutPath, g_szUserName,
			Cvar_Get("menu_profile_remember_pass", "0", 0)->value ? g_szPassHash : "", false);
	}

	Cbuf_AddText("menu pop profile\n");

	// Check for new versions
	if (s = strstr(szDataBack, "LatestClientBuild:"))
	{
		s += sizeof("LatestClientBuild:");
		
		if (atoi(s) > BUILD)
		{
			if (!g_notified_of_new_version) // only pop it up once.
			{
				Cbuf_AddText("menu newversion\n");
				g_notified_of_new_version = true;
			}
		}
	}
}


extern qboolean g_command_stuffed;


void CL_WebLoad_f (void)
{
	char *sURL;
#ifndef WIN32
	char szCommand[1024];
#endif

	// Don't let malicious servers force clients to load web pages
	if (g_command_stuffed)
		return;

	if (Cmd_Argc() != 2)
	{
		Com_Printf("Usage: webload http://<url>\n");
		return;
	}

	sURL = Cmd_Argv(1);
	
	if (strncmp(sURL, "http://", 7) != 0 && strncmp(sURL, "https://", 8) != 0 && strncmp(sURL, "ftp://", 6) != 0)
	{
		Com_Printf("Usage: webload http://<url>\n");
		return;
	}

#ifdef WIN32
	ShellExecute(NULL, "open", sURL, NULL, NULL, SW_SHOWNORMAL);
#else
	// load default browser linux
	Com_sprintf(szCommand, sizeof(szCommand), "x-www-browser \"%s\"", sURL);
	system(sURL);
#endif
}


void CL_GlobalLogin_f (void)
{
	char *randstr;
	char szPassHash2[64];
	char szCommand[128];
	char *sUserID;

	if (Cmd_Argc() != 3)
	{
		Com_Printf("Usage: global_login <userid> <randstr>\n");
		return;
	}

	randstr = Cmd_Argv(2);
	sUserID = Cmd_Argv(1);
	Com_HMACMD5String(g_szPassHash, strlen(g_szPassHash), randstr,
		strlen(randstr), szPassHash2, sizeof(szPassHash2));
	Com_sprintf(szCommand, sizeof(szCommand), "cmd global_login %s\n", szPassHash2);
	Cbuf_AddText(szCommand);
	
	if (e.r)
		e.r(randstr, atoi(sUserID), 0, 0);
}


void CL_ProfileGetLogin_f (void)
{
	char *pProfileFile, *pOutCvar;
	char szProfilePath[MAX_OSPATH];
	char *pFileData;

	if (Cmd_Argc() != 3)
	{
		Com_Printf("Usage: get_profile_name <profile file> <output cvar>\n");
		return;
	}

	pProfileFile = Cmd_Argv(1);
	pOutCvar = Cmd_Argv(2);
	Com_sprintf(szProfilePath, sizeof(szProfilePath), "profiles/%s.prf", pProfileFile);
	
	if (FS_LoadFile(szProfilePath, (void **)&pFileData) > 0)
	{
		if (memcmp(pFileData, "PB2PROFILE1.0\0", 14) == 0)
		{
			Cvar_Set(pOutCvar, pFileData + 14);
		}
		else
		{
			Com_Printf("Invalid profile file: %s\n", pProfileFile);
			Cvar_Set(pOutCvar, "");
		}

		FS_FreeFile(pFileData);
	}
	else
	{
		Cvar_Set(pOutCvar, "");
	}
}


void CL_InitProfile (void)
{
	menu_profile_pass = Cvar_Get("menu_profile_pass", "", 0);
	g_nVNInitUnique = rand() * rand() * rand();
	Cmd_AddCommand("webload", CL_WebLoad_f);
	Cmd_AddCommand("profile_edit", CL_ProfileEdit_f);
	Cmd_AddCommand("profile_edit2", CL_ProfileEdit2_f);
	Cmd_AddCommand("profile_add", CL_ProfileAdd_f);
	Cmd_AddCommand("profile_select", CL_ProfileSelect_f);
	Cmd_AddCommand("profile_login", CL_ProfileLogin_f);
	Cmd_AddCommand("global_login", CL_GlobalLogin_f);
	Cmd_AddCommand("profile_get_login", CL_ProfileGetLogin_f);
}

