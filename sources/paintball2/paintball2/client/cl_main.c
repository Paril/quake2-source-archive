/*
Copyright (C) 1997-2001 Id Software, Inc.

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
// cl_main.c  -- client main loop

#include "client.h"

#ifdef WIN32
#include <windows.h> // jit/pooy
#endif

cvar_t	*freelook;

cvar_t	*cl_stereo_separation;
cvar_t	*cl_stereo;

cvar_t	*rcon_client_password;
cvar_t	*rcon_address;

cvar_t	*cl_noskins;
cvar_t	*cl_autoskins;
cvar_t	*cl_footsteps;
cvar_t	*cl_timeout;
cvar_t	*cl_predict;
cvar_t	*cl_minfps;
cvar_t	*cl_maxfps;
cvar_t	*cl_locknetfps; // jitnetfps
cvar_t	*cl_cmdrate; // jitnetfps
cvar_t	*cl_sleep; // jit/pooy
cvar_t	*cl_gun;

cvar_t	*cl_add_particles;
cvar_t	*cl_add_lights;
cvar_t	*cl_add_entities;
//cvar_t	*cl_add_blend;

cvar_t	*cl_shownet;
cvar_t	*cl_showmiss;
cvar_t	*cl_showclamp;
cvar_t	*cl_shownamechange; // jit

cvar_t	*cl_paused;
cvar_t	*cl_timedemo;

cvar_t	*lookspring;
cvar_t	*lookstrafe;
cvar_t	*sensitivity;

cvar_t	*m_pitch;
cvar_t	*m_yaw;
cvar_t	*m_forward;
cvar_t	*m_side;
cvar_t	*m_invert; // jitmouse
cvar_t	*m_doubleclickspeed; // jitmenu

cvar_t	*cl_lightlevel;

// Xile/NiceAss LOC
cvar_t	*cl_drawlocs;
cvar_t	*loc_here;
cvar_t	*loc_there;

// userinfo
cvar_t	*build; // jitversion
cvar_t	*info_password;
cvar_t	*info_spectator;
cvar_t	*name;
cvar_t	*menu_tempname; // jit
cvar_t	*skin;
cvar_t	*rate;
cvar_t	*fov;
cvar_t	*m_fovscale; // jitmouse
cvar_t	*msg;
cvar_t	*hand;
cvar_t	*gender;
cvar_t	*cl_vwep;
cvar_t	*cl_drawfps; // drawfps - MrG
cvar_t	*cl_drawpps; // jitnetfps
cvar_t	*cl_drawtexinfo; // jit
cvar_t	*cl_centerprintkills;
cvar_t	*cl_timestamp; // jit
cvar_t	*cl_dialogprint; // jit
cvar_t	*cl_hudscale; // jit
cvar_t	*cl_consoleheight; // T3RR0R15T: console height
cvar_t	*cl_conback; // T3RR0R15T: console background
cvar_t	*cl_menuback; // T3RR0R15T: menu background
cvar_t  *cl_crosshairscale; // viciouz - crosshair scale
cvar_t	*cl_drawhud; // jithud
cvar_t	*cl_language; // jittrans
cvar_t	*cl_menu; // T3RR0R15T: custom menu files
cvar_t	*cl_drawclock; // viciouz - real time clock
cvar_t	*cl_drawclockx; // T3RR0R15T: clock position
cvar_t	*cl_drawclocky; // T3RR0R15T: clock position
cvar_t  *cl_maptime; // T3RR0R15T: elapsed maptime (from AprQ2)
cvar_t  *cl_maptimex; // T3RR0R15T: maptime position
cvar_t  *cl_maptimey; // T3RR0R15T: maptime position
cvar_t  *cl_drawping; // T3RR0R15T: display ping on HUD
cvar_t  *cl_autorecord; // T3RR0R15T: client side autodemo
cvar_t  *cl_scoreboard_sorting; // T3RR0R15T: scoreboard sorting
cvar_t  *cl_swearfilter; // viciouz - swear filter
cvar_t  *cl_blockedwords; // viciouz - swear filter
cvar_t	*cl_passwordpopup; // viciouz - password popup
cvar_t	*r_oldmodels;
cvar_t	*gl_highres_textures; // jit
cvar_t	*serverlist_source; // jitserverlist / jitmenu
cvar_t	*serverlist_source2; // jitserverlist / jitmenu
cvar_t	*serverlist_source3; // jitserverlist / jitmenu
cvar_t	*serverlist_blacklist;
cvar_t	*serverlist_udp_source1; // jitserverlist
#ifdef USE_DOWNLOAD3
cvar_t	*cl_fast_download; // jitdownload
#endif

client_static_t	cls;
client_state_t	cl;

centity_t		cl_entities[MAX_EDICTS];

entity_state_t	cl_parse_entities[MAX_PARSE_ENTITIES];

extern	cvar_t *allow_download;
extern	cvar_t *allow_download_players;
extern	cvar_t *allow_download_models;
extern	cvar_t *allow_download_sounds;
extern	cvar_t *allow_download_maps;

//======================================================================

extern cvar_t *Cvar_FindVar (char *var_name);
qboolean CL_HasProfile ();
void CL_Serverlist_RunFrame ();
extern qboolean g_notified_of_new_version;

void CL_Toggle_f (void)
{
	cvar_t *var;

	if (Cmd_Argc() != 2)
	{
		Com_Printf ("Usage: cvar_toggle <cvar>\n");
		return;
	}

	var = Cvar_FindVar(Cmd_Argv(1));
	if (!var) {
		Com_Printf ("Unknown cvar '%s'.\n", Cmd_Argv(1));
		return;
	}

	if (var->value)
		Cvar_SetValue(Cmd_Argv(1), 0);
	else
		Cvar_SetValue(Cmd_Argv(1), 1);
}

void CL_Increase_f (void)
{
	cvar_t *var;
	float val;
	int argc = Cmd_Argc();

	if (argc < 3)
	{
		Com_Printf("Usage: cvar_inc <cvar> <value> [min] [max]\n");
		return;
	}

	var = Cvar_FindVar(Cmd_Argv(1));

	if (!var)
	{
		Com_Printf("Unknown cvar '%s'.\n", Cmd_Argv(1));
		return;
	}

	val = atof(Cmd_Argv(2)) + var->value;

	if (argc >= 5)
	{
		float min = atof(Cmd_Argv(3));
		float max = atof(Cmd_Argv(4));

		if (val < min)
			val = max;

		if (val > max)
			val = min;
	}

	Cvar_SetValue(Cmd_Argv(1), val);
}

void CL_CatCvar_f (void) // jitconfig
{
		cvar_t *var;

	if (Cmd_Argc() < 2)
	{
		Com_Printf("Usage: cvar_cat <cvar> <string>\n");
		return;
	}

	var = Cvar_FindVar(Cmd_Argv(1));

	if (!var)
	{
		Com_Printf("Unknown cvar '%s'.\n", Cmd_Argv(1));
		return;
	}

	if (Cmd_Argc() < 3) // add a space
		Cvar_Set(Cmd_Argv(1), va("%s ", var->string));
	else
		Cvar_Set(Cmd_Argv(1), va("%s%s", var->string, Cmd_Argv(2)));
}


/*
====================
CL_WriteDemoMessage

Dumps the current net message, prefixed by the length
====================
*/
void CL_WriteDemoMessage (void)
{
	int		len, swlen;

	// the first eight bytes are just packet sequencing stuff
	len = net_message.cursize-8;
	swlen = LittleLong(len);
	fwrite (&swlen, 4, 1, cls.demofile);
	fwrite (net_message.data+8,	len, 1, cls.demofile);
}


/*
====================
CL_Stop_f

stop recording a demo
====================
*/
void CL_Stop_f (void)
{
	int		len;

	if (!cls.demorecording)
	{
		Com_Printf ("Not recording a demo.\n");
		return;
	}

// finish up
	len = -1;
	fwrite (&len, 4, 1, cls.demofile);
	fclose (cls.demofile);
	cls.demofile = NULL;
	cls.demorecording = false;
	Com_Printf ("Stopped recording a demo.\n");
}


/*
====================
CL_Record_f

record <demoname>

Begins recording a demo from the current position
====================
*/
void CL_RecordFile (const char *sDemoName) // jitdemo
{
	char	name[MAX_OSPATH];
	char	buf_data[MAX_MSGLEN];
	sizebuf_t	buf;
	int		i;
	int		len;
	entity_state_t	*ent;
	entity_state_t	nullstate;
	unsigned char *scorestr;
	time_t rawtime; // viciouz - timestamp demos
	struct tm * timeinfo; // viciouz - timestamp demos
	char timebuffer[512]; // viciouz - timestamp demos
	char servername[512];


	if (cls.demorecording)
	{
		Com_Printf("Already recording.\n");
		return;
	}

	if (cls.state != ca_active)
	{
		Com_Printf("You must be in a level to record.\n");
		return;
	}

	// === jitdemo
	Com_sprintf(name, sizeof(name), "%s/demos/%s.dm2", FS_Gamedir(), sDemoName);
	i = 0;

	while (FileExists(name))
		Com_sprintf(name, sizeof(name), "%s/demos/%s_%03d.dm2", FS_Gamedir(), sDemoName, ++i);
	// jitdemo ===

	Com_Printf("Recording to %s.\n", name);
	FS_CreatePath(name);
	cls.demofile = fopen(name, "wb");

	if (!cls.demofile)
	{
		Com_Printf("ERROR: couldn't open.\n");
		return;
	}
	cls.demorecording = true;

	// don't start saving messages until a non-delta compressed message is received
	cls.demowaiting = true;

	
	//
	// write out messages to hold the startup information
	//
	SZ_Init(&buf, buf_data, sizeof(buf_data));

	// send the serverdata
	MSG_WriteByte(&buf, svc_serverdata);
	MSG_WriteLong(&buf, PROTOCOL_VERSION);
	MSG_WriteLong(&buf, 0x10000 + cl.servercount);
	MSG_WriteByte(&buf, 1);	// demos are always attract loops
	MSG_WriteString(&buf, cl.gamedir);
	MSG_WriteShort(&buf, cl.playernum);
	MSG_WriteString(&buf, cl.configstrings[CS_NAME]);

	// configstrings
	for (i = 0; i < MAX_CONFIGSTRINGS; i++)
	{
		if (cl.configstrings[i][0])
		{
			if (buf.cursize + strlen(cl.configstrings[i]) + 32 > buf.maxsize)
			{	// write it out
				len = LittleLong(buf.cursize);
				fwrite(&len, 4, 1, cls.demofile);
				fwrite(buf.data, buf.cursize, 1, cls.demofile);
				buf.cursize = 0;
			}

			MSG_WriteByte(&buf, svc_configstring);
			MSG_WriteShort(&buf, i);
			MSG_WriteString(&buf, cl.configstrings[i]);
		}
	}

	// baselines
	memset(&nullstate, 0, sizeof(nullstate));

	for (i = 0; i < MAX_EDICTS; i++)
	{
		ent = &cl_entities[i].baseline;

		if (!ent->modelindex)
			continue;

		if (buf.cursize + 64 > buf.maxsize)
		{	// write it out
			len = LittleLong(buf.cursize);
			fwrite(&len, 4, 1, cls.demofile);
			fwrite(buf.data, buf.cursize, 1, cls.demofile);
			buf.cursize = 0;
		}

		MSG_WriteByte(&buf, svc_spawnbaseline);		
		MSG_WriteDeltaEntity(&nullstate, &cl_entities[i].baseline, &buf, true, true);
	}

	MSG_WriteByte(&buf, svc_stufftext);
	MSG_WriteString(&buf, "precache\n");

	// write it to the demo file
	len = LittleLong(buf.cursize);
	fwrite(&len, 4, 1, cls.demofile);
	fwrite(buf.data, buf.cursize, 1, cls.demofile);

	// jitscores -- we need to write the known score data here, otherwise
	// demos will be messed up.
	i = CL_ScoresDemoData(0, &scorestr);

	do {
		SZ_Init(&buf, buf_data, sizeof(buf_data));
		MSG_WriteByte(&buf, svc_print);
		MSG_WriteByte(&buf, PRINT_SCOREDATA);
		MSG_WriteString(&buf, scorestr);
		len = LittleLong(buf.cursize);
		fwrite(&len, 4, 1, cls.demofile);
		fwrite(buf.data, buf.cursize, 1, cls.demofile);
	} while ((i = CL_ScoresDemoData(i, &scorestr)));


	// viciouz - timestamp demos
	SZ_Init(&buf, buf_data, sizeof(buf_data));
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(timebuffer, sizeof(timebuffer) - 1, "Demo Recorded: %Y-%m-%d %H:%M:%S %Z\n", timeinfo);
	timebuffer[sizeof(timebuffer) - 1] = 0;
	Com_sprintf(servername, sizeof(servername), "Server: %s\n", cls.servername);
	MSG_WriteByte(&buf, svc_print);
	MSG_WriteByte(&buf, PRINT_HIGH);
	MSG_WriteString(&buf, timebuffer);
	MSG_WriteByte(&buf, svc_print);
	MSG_WriteByte(&buf, PRINT_HIGH);
	MSG_WriteString(&buf, servername);
	len = LittleLong(buf.cursize);
	fwrite(&len, 4, 1, cls.demofile);
	fwrite(buf.data, buf.cursize, 1, cls.demofile);

	// the rest of the demo file will be individual frames
}


// === jitdemo
void CL_Record_f (void)
{
	if (Cmd_Argc() != 2)
	{
		Com_Printf("Usage: record <demoname>\n");
		return;
	}

	CL_RecordFile(Cmd_Argv(1));
}


void CL_ARecord_f (void)
{
	time_t now;
	char szDemoName[MAX_OSPATH];
	char szTimeStamp[MAX_OSPATH];
	char szLevelName[MAX_OSPATH];
	char *s, *sExt = NULL;

	Q_strncpyz(szLevelName, cl.configstrings[CS_MODELS + 1] + 5, sizeof(szLevelName)); // copy map filename, sans "maps/"

	for (s = szLevelName; *s; ++s)
	{
		if (*s == '\\' || *s == '/' || *s == ' ')
			*s = '_';

		if (*s == '.')
			sExt = s;
	}

	if (sExt)
		*sExt = 0; // strip off .bsp

	time(&now);
	strftime(szTimeStamp, sizeof(szTimeStamp), "%Y%m%d_%H%M%S", localtime(&now));
	if (Cmd_Argc() != 2) 
	{
		Com_sprintf(szDemoName, sizeof(szDemoName), "auto_%s_%i_%s", szTimeStamp, cl.playernum, szLevelName);
	}
	else 
	{
		Com_sprintf(szDemoName, sizeof(szDemoName), "%s_%s_%i_%s", Cmd_Argv(1), szTimeStamp, cl.playernum, szLevelName);
	}
	CL_RecordFile(szDemoName);
}
// jitdemo ===


//======================================================================

/*
===================
Cmd_ForwardToServer

adds the current command line as a clc_stringcmd to the client message.
things like godmode, noclip, etc, are commands directed to the server,
so when they are typed in at the console, they will need to be forwarded.
===================
*/
void Cmd_ForwardToServer (void)
{
	char	*cmd;

	cmd = Cmd_Argv(0);

	if (cls.state <= ca_connected || *cmd == '-' || *cmd == '+')
	{
		Com_Printf ("Unknown command \"%s\".\n", cmd);
		return;
	}

	MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
	SZ_Print(&cls.netchan.message, cmd);

	if (Cmd_Argc() > 1)
	{
		SZ_Print(&cls.netchan.message, " ");
		SZ_Print(&cls.netchan.message, Cmd_Args());
	}
}

void CL_Setenv_f (void)
{
	int argc = Cmd_Argc();

	if ( argc > 2 )
	{
		char buffer[1000];
		int i;

		strcpy( buffer, Cmd_Argv(1) );
		strcat( buffer, "=" );

		for ( i = 2; i < argc; i++ )
		{
			strcat( buffer, Cmd_Argv( i ) );
			strcat( buffer, " " );
		}

		putenv( buffer );
	}
	else if ( argc == 2 )
	{
		char *env = getenv( Cmd_Argv(1) );

		if ( env )
		{
			Com_Printf( "%s=%s\n", Cmd_Argv(1), env );
		}
		else
		{
			Com_Printf( "%s undefined.\n", Cmd_Argv(1), env );
		}
	}
}


/*
==================
CL_ForwardToServer_f
==================
*/
void CL_ForwardToServer_f (void)
{
	if (cls.state != ca_connected && cls.state != ca_active)
	{
		Com_Printf("Can't \"%s\", not connected.\n", Cmd_Argv(0));
		return;
	}
	
	// don't forward the first argument
	if (Cmd_Argc() > 1)
	{
		MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
		SZ_Print(&cls.netchan.message, Cmd_Args());
	}
}


/*
==================
CL_Pause_f
==================
*/
void CL_Pause_f (void)
{
	// never pause in multiplayer
	if (Cvar_VariableValue("maxclients") > 1 || !Com_ServerState())
	{
		Cvar_SetValue("paused", 0);
		return;
	}

	Cvar_SetValue("paused", !cl_paused->value);
}

/*
==================
CL_Quit_f
==================
*/
void CL_Quit_f (void)
{
	CL_Disconnect();
	Com_Quit();
}


void CL_ShowTex_f (void) // jitshowtex
{
	trace_t tr;
	vec3_t end;

	VectorMA(cl.predicted_origin, 8192, cl.v_forward, end);
	tr = CM_BoxTrace(cl.predicted_origin, end, vec3_origin, vec3_origin, 0, MASK_ALL);
	Com_Printf("Surface: %s\n", tr.surface->name);
}

/*
================
CL_Drop

Called after an ERR_DROP was thrown
================
*/
void CL_Drop (void)
{
	if (cls.state == ca_uninitialized)
		return;
	if (cls.state == ca_disconnected)
		return;

	CL_Disconnect();

	// drop loading plaque unless this is the initial game start
	if (cls.disable_servercount != -1)
		SCR_EndLoadingPlaque();	// get rid of loading plaque
}


/*
=======================
CL_SendConnectPacket

We have gotten a challenge from the server, so try and
connect.
======================
*/
char lastservername[MAX_OSPATH]; // jitdownload
void clearfaileddownloads(); // jitdownload

void CL_SendConnectPacket (void)
{
	netadr_t adr;
	int port;

	if (!NET_StringToAdr(cls.servername, &adr))
	{
		Com_Printf("Bad server address: \"%s\"\n", cls.servername); // jit
		cls.connect_time = 0;
		return;
	}

	// ===
	// jitdownload -- if we're connecting to a different server,
	// clear the record of failed download attempts
	if (!Q_streq(cls.servername, lastservername))
	{
		strcpy(lastservername, cls.servername);
		clearfaileddownloads();
	}
	// jitdownload
	// ===

	if (adr.port == 0)
		adr.port = BigShort(PORT_SERVER);

	port = Cvar_VariableValue("qport");
	userinfo_modified = false;
	Netchan_OutOfBandPrint(NS_CLIENT, adr, "connect %i %i %i \"%s\"\n",
		PROTOCOL_VERSION, port, cls.challenge, Cvar_Userinfo());
}

/*
=================
CL_CheckForResend

Resend a connect message if the last one has timed out
=================
*/
void CL_CheckForResend (void)
{
	netadr_t	adr;

	// if the local server is running and we aren't
	// then connect
	if (cls.state == ca_disconnected && Com_ServerState() )
	{
		cls.state = ca_connecting;
		Q_strncpyz(cls.servername, "localhost", sizeof(cls.servername)-1);
		// we don't need a challenge on the localhost
		CL_SendConnectPacket ();
		return;
//		cls.connect_time = -99999;	// CL_CheckForResend() will fire immediately
	}

	// resend if we haven't gotten a reply yet
	if (cls.state != ca_connecting)
		return;

	if (cls.realtime - cls.connect_time < 3000)
		return;

	if (!NET_StringToAdr (cls.servername, &adr))
	{
		Com_Printf("Bad server address: \"%s\"\n", cls.servername); // jit
		cls.state = ca_disconnected;
		return;
	}
	if (adr.port == 0)
		adr.port = BigShort (PORT_SERVER);

	cls.connect_time = cls.realtime;	// for retransmit requests

	Com_Printf ("Connecting to %s...\n", cls.servername);
	cl_scores_setinuse_all(false); // jitscores - clear scoreboard

	Netchan_OutOfBandPrint (NS_CLIENT, adr, "getchallenge\n");
}


#ifdef USE_DOWNLOAD3
static void CL_ClearExtensions (void)
{
	cls.download3supported = false;
}
#endif


/*
================
CL_Connect_f
================
*/
void CL_Connect_f (void)
{
	char	*server;

	M_ForceMenuOff(); // jitmenu

	if (Cmd_Argc() != 2)
	{
		Com_Printf("Usage: connect <server>\n");
		return;	
	}
	
	// if running a local server, kill it and reissue
	if (Com_ServerState())
		SV_Shutdown("Server quit.\n", false);
	else
		CL_Disconnect();

	server = Cmd_Argv(1);
	NET_Config(true);		// allow remote
	CL_Disconnect();
#ifdef USE_DOWNLOAD3
	CL_ClearExtensions();
#endif
	cls.state = ca_connecting;
	Q_strncpyz(cls.servername, server, sizeof(cls.servername)-1);
	cls.connect_time = -99999;	// CL_CheckForResend() will fire immediately
}

void CL_ParseURL_f (void) // jiturl
{
	char buff[1024];
	char *s, *s1, *password;

	if (Cmd_Argc() < 2)
	{
		Com_Printf("Usage: parse_url <url>\n");
		return;
	}

	// Com_Printf("Parsing: %s\n", Cmd_Argv(1)); // debug

	if (!(s = strstr(Cmd_Argv(1), ":/")) && !(s = strstr(Cmd_Argv(1), ":\\")))
	{
		Com_Printf("Invalid URL: \"%s\"\n", Cmd_Argv(1));
		return;
	}

	s++;

	while (*s == '/' || *s == '\\')
		s++;

	s = strtok(Cmd_Argv(1), "&");
	password = strtok(NULL, "&");

	s = strstr(Cmd_Argv(1), ":/");
	s++;
	while (*s == '/' || *s == '\\')
		s++;

	// viciouz - fixed browser password link - luckily semicolons and spaces strip :)
	Com_sprintf(buff, sizeof(buff)-8, "connect %s;password %s\n", s, password); 
	s = strchr(buff, '/');
	s1 = strchr(buff, '\\');

	if (s1 > s)
		s = s1;

	if (s)
	{
		s[0] = '\n';
		s[1] = '\0';
	}

	Cbuf_AddText(buff);
}

/*
=====================
CL_Rcon_f

  Send the rest of the command line over as
  an unconnected command.
=====================
*/
void CL_Rcon_f (void)
{
	char	message[1024];
	int		i;
	netadr_t	to;

	if (!rcon_client_password->string)
	{
		Com_Printf ("You must set 'rcon_password' before\n"
					"issuing an rcon command.\n");
		return;
	}

	message[0] = (char)255;
	message[1] = (char)255;
	message[2] = (char)255;
	message[3] = (char)255;
	message[4] = 0;

	NET_Config(true);		// allow remote

	strcat(message, "rcon ");
	strcat(message, rcon_client_password->string);
	strcat(message, " ");

	for (i = 1; i < Cmd_Argc(); i++)
	{
		strcat(message, Cmd_Argv(i));
		strcat(message, " ");
	}

	if (cls.state >= ca_connected)
	{
		to = cls.netchan.remote_address;
	}
	else
	{
		if (!strlen(rcon_address->string))
		{
			Com_Printf("You must either be connected,\n"
						"or set the 'rcon_address' cvar\n"
						"to issue rcon commands.\n");
			return;
		}

		NET_StringToAdr(rcon_address->string, &to);

		if (to.port == 0)
			to.port = BigShort(PORT_SERVER);
	}
	
	NET_SendPacket(NS_CLIENT, strlen(message)+1, message, to);
}


/*
=====================
CL_ClearState

=====================
*/
void CL_ClearState (void)
{
	S_StopAllSounds();
	CL_ClearEffects();
	CL_ClearTEnts();
	// wipe the entire cl structure
	memset(&cl, 0, sizeof(cl));
	memset(&cl_entities, 0, sizeof(cl_entities));
	SZ_Clear(&cls.netchan.message);
}

/*
=====================
CL_Disconnect

Goes from a connected state to full screen console state
Sends a disconnect message to the server
This is also called on Com_Error, so it shouldn't cause any errors
=====================
*/
void CL_Disconnect (void)
{
	byte	final[32];

	if (cls.state == ca_disconnected)
		return;

	if (cl_timedemo && cl_timedemo->value)
	{
		int	time = Sys_Milliseconds() - cl.timedemo_start;

		if (time > 0)
			Com_Printf("%i frames, %3.1f seconds: %3.1f fps\n", cl.timedemo_frames,
				time/1000.0, cl.timedemo_frames*1000.0 / time);
	}

	VectorClear(cl.refdef.blend);
	re.CinematicSetPalette(NULL);
	M_ForceMenuOff();
	cls.connect_time = 0;
	SCR_StopCinematic();

	if (cls.demorecording)
		CL_Stop_f();

	// send a disconnect message to the server
	final[0] = clc_stringcmd;
	strcpy((char *)final + 1, "disconnect");
	Netchan_Transmit(&cls.netchan, strlen(final), final);
	Netchan_Transmit(&cls.netchan, strlen(final), final);
	Netchan_Transmit(&cls.netchan, strlen(final), final);
	CL_ClearState();

#ifdef USE_DOWNLOAD3
	CL_StopCurrentDownload();
#else
	// stop download
	if (cls.download)
	{
		fclose(cls.download);
		cls.download = NULL;
	}
#endif

	cls.state = ca_disconnected;
	cl_scores_setinuse_all(false); // jitscores - clear scoreboard
}

void CL_Disconnect_f (void)
{
	Com_Error(ERR_DROP, "Disconnected from server.");
}


/*
====================
CL_Packet_f

packet <destination> <contents>

Contents allows \n escape character
====================
*/
void CL_Packet_f (void)
{
	char	send[2048];
	int		i, l;
	char	*in, *out;
	netadr_t	adr;

	if (Cmd_Argc() != 3)
	{
		Com_Printf("Usage: packet <destination> <contents>\n");
		return;
	}

	NET_Config(true);		// allow remote

	if (!NET_StringToAdr (Cmd_Argv(1), &adr))
	{
		Com_Printf("Bad address.\n");
		return;
	}

	if (!adr.port)
		adr.port = BigShort (PORT_SERVER);

	in = Cmd_Argv(2);
	out = send+4;
	send[0] = send[1] = send[2] = send[3] = (char)0xff;

	l = strlen (in);

	for (i=0; i<l; i++)
	{
		if (in[i] == '\\' && in[i+1] == 'n')
		{
			*out++ = '\n';
			i++;
		}
		else
			*out++ = in[i];
	}
	*out = 0;

	NET_SendPacket (NS_CLIENT, out-send, send, adr);
}

/*
=================
CL_Changing_f

Just sent as a hint to the client that they should
drop to full console
=================
*/
void CL_Changing_f (void)
{
	//ZOID
	//if we are downloading, we don't change!  This so we don't suddenly stop downloading a map
	if (cls.download) // jitodo -- we SHOULD make it stop downloading and switch to the next map!
		return;

	if (Cmd_Argc() > 1) // jitloading - server specificed map name, so we can show it immediately on the loading screen (build 38 or higher server)
		SCR_BeginLoadingPlaque(Cmd_Argv(1));
	else
		SCR_BeginLoadingPlaque(NULL);

	
	cls.state = ca_connected; // not active anymore, but not disconnected
	Com_Printf("\nChanging map...\n");
	cl_scores_setinuse_all(false); // jitscores - clear scoreboard
}


/*
=================
CL_Reconnect_f

The server is changing levels
=================
*/
void CL_Reconnect_f (void)
{
	//ZOID
	//if we are downloading, we don't change!  This so we don't suddenly stop downloading a map
	if (cls.download) // jitodo -- we SHOULD make it stop downloading and switch to the next map!
		return;

	// === jitdemo - multi-map demo support
	//if (cl.attractloop)
	//	return;
	// jitdemo ===

	S_StopAllSounds();

	if (cls.state == ca_connected)
	{
		Com_Printf("Reconnecting...\n");
		cl_scores_setinuse_all(false); // jitscores - clear scoreboard
		cls.state = ca_connected;
		MSG_WriteChar(&cls.netchan.message, clc_stringcmd);
		MSG_WriteString(&cls.netchan.message, "new");		
		return;
	}

	if (*cls.servername)
	{
		if (cls.state >= ca_connected)
		{
			CL_Disconnect();
			cls.connect_time = cls.realtime - 1500;
		}
		else
		{
			cls.connect_time = -99999; // fire immediately
		}

		cls.state = ca_connecting;
		Com_Printf("Reconnecting...\n");
		cl_scores_setinuse_all(false); // jitscores - clear scoreboard
	}
}

/*
=================
CL_ParseStatusMessage

Handle a reply from a ping
=================
*/
void CL_ParseStatusMessage (void)
{
	char	*s;

	s = MSG_ReadString(&net_message);
	Com_DPrintf("%s\n", s);
	M_AddToServerList(net_from, s, false);
}

/*
=================
CL_Skins_f

Load or download any custom player skins and models
=================
*/
void CL_Skins_f (void)
{
	int		i;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (!cl.configstrings[CS_PLAYERSKINS+i][0])
			continue;

		Com_Printf("client %i: %s\n", i, cl.configstrings[CS_PLAYERSKINS+i]); 
		SCR_UpdateScreen();
		Sys_SendKeyEvents();	// pump message loop
		CL_ParseClientinfo(i);
	}
}


#ifdef USE_DOWNLOAD3 // jitdownload
static void CL_ParseDownload3 (void)
{
	int num_chunks = (cls.download3size + DOWNLOAD3_CHUNKSIZE - 1) / DOWNLOAD3_CHUNKSIZE;
	unsigned short md5sum_short;
	int chunk, i;
	int chunksize;
	char chunkdata[DOWNLOAD3_CHUNKSIZE];
	qboolean done = true;
	byte msg_data[MAX_MSGLEN];
	sizebuf_t msg;
	byte fileid;
	unsigned int md5sum;
	int realtime = Sys_Milliseconds();
	int timediff;

	if (!cls.download)
		return; // no downolad active, but there may be some trailing packets still

	fileid = MSG_ReadByte(&net_message);

	if (fileid != cls.download3fileid)
	{
		if (fileid == cls.download3lastfileid) // fileid of a previous download.  Ignore it
			return;

		assert(fileid == cls.download3fileid);
		Com_Printf("Fileid mismatch: %d != %d\n", (int)fileid, (int)cls.download3fileid);
		return;
	}

	md5sum_short = MSG_ReadShort(&net_message);
	chunk = MSG_ReadLong(&net_message);

	if (chunk >= num_chunks || chunk < 0)
	{
		assert(chunk < num_chunks && chunk > 0);
		Com_Printf("Download chunk out of range: %d\n", chunk);
		return;
	}

	// All chunks will be DOWNLOAD3_CHUNKSIZE except maybe the last one.
	if (chunk == num_chunks - 1)
	{
		chunksize = cls.download3size % DOWNLOAD3_CHUNKSIZE;

		if (!chunksize)
			chunksize = DOWNLOAD3_CHUNKSIZE;
	}
	else
	{
		chunksize = DOWNLOAD3_CHUNKSIZE;
	}

	MSG_ReadData(&net_message, chunkdata, chunksize);
	md5sum = Com_MD5Checksum(chunkdata, chunksize);

	if (md5sum_short != (unsigned short)(md5sum & 0xFFFF))
	{
		assert(md5sum_short == (unsigned short)(md5sum & 0xFFFF));
		Com_Printf("MD5 sum failed for chunk %d.\n", chunk);
		return;
	}

	if (!cls.download3data || !cls.download3chunks)
		return;

	memcpy(cls.download3data + (chunk * DOWNLOAD3_CHUNKSIZE), chunkdata, chunksize);

	if (cls.download3chunks[chunk] == DOWNLOAD3_CHUNKWAITING)
	{
		cls.download3chunks[chunk] = DOWNLOAD3_CHUNKRECEIVED;
		++cls.download3completechunks;
		cls.download3bytesreceived += chunksize;
		cls.download3bytessincelastratecheck += chunksize;
	}

	// write some data if we can
	for (i = cls.download3lastchunkwritten; i < num_chunks; ++i)
	{
		if (cls.download3chunks[i] == DOWNLOAD3_CHUNKWAITING)
		{
			done = false;
			break; // drop out of the loop.  Don'w want to write the file out of sequence
		}
		else if (cls.download3chunks[i] == DOWNLOAD3_CHUNKRECEIVED)
		{
			// Data  received but not written.  Write it.
			if (i == num_chunks - 1)
			{
				chunksize = cls.download3size % DOWNLOAD3_CHUNKSIZE;

				if (!chunksize)
					chunksize = DOWNLOAD3_CHUNKSIZE;
			}
			else
			{
				chunksize = DOWNLOAD3_CHUNKSIZE;
			}

			fwrite(cls.download3data + (i * DOWNLOAD3_CHUNKSIZE), chunksize, 1, cls.download);
			cls.download3chunks[i] = DOWNLOAD3_CHUNKWRITTEN;
			cls.download3lastchunkwritten = i;
		}
	}

	cls.downloadpercent = 100 * cls.download3completechunks / num_chunks;
	SZ_Init(&msg, msg_data, sizeof(msg_data));
	SZ_Write(&msg, "dl3ack\n", 7);
	MSG_WriteShort(&msg, (int)qport->value);
	MSG_WriteByte(&msg, cls.download3fileid);
	MSG_WriteLong(&msg, chunk);

	for (i = 0; i < DOWNLOAD3_NUMBACKUPACKS; ++i)
		MSG_WriteLong(&msg, cls.download3backacks[i]);

	cls.download3backacks[cls.download3currentbackack] = chunk;
	cls.download3currentbackack++;
	cls.download3currentbackack %= DOWNLOAD3_NUMBACKUPACKS;
	Netchan_OutOfBand(NS_CLIENT, cls.netchan.remote_address, msg.cursize, msg.data);

	// Update rate
	timediff = realtime - cls.download3lastratecheck;

	if (timediff > 1000) // check every second
	{
		float rate;

		Con_DrawDownloadBar(false);

		if (cls.loading_screen)
		{
			Cbuf_AddText("menu_refresh\n"); // loading screen
		}

		rate = 1000.0f * (float)cls.download3bytessincelastratecheck / (float)timediff;

		if (cls.download3rate)
			cls.download3rate = 0.75f * cls.download3rate + 0.25f * rate;
		else
			cls.download3rate = rate;

		cls.download3bytessincelastratecheck = 0;
		cls.download3lastratecheck = realtime;
	}

	if (done) // transfer is complete
	{
		char oldn[MAX_OSPATH];
		char newn[MAX_OSPATH];
		unsigned int md5sum;
		int file_length;
		char *file_data;
		
		// Clear the loading screen progress bar:
		Cvar_ForceSet("cs_loadingbarback", "");
		Cvar_ForceSet("cs_loadingbarfront", "");
		Cvar_ForceSet("cs_downloadspeed", "");

		MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
		MSG_WriteString(&cls.netchan.message, va("dl3complete %d", cls.download3fileid));
		CL_StopCurrentDownload();
		cls.download3requested = false;

		// Verify that the file downloaded correctly
		file_length = FS_LoadFile(cls.downloadtempname, (void *)&file_data);

		if (file_length < 0)
		{
			assert(file_length >= 0);
			Com_Printf("Failed to open temporary file %s.\n", cls.downloadtempname);
		}
		else
		{
			md5sum = Com_MD5Checksum(file_data, file_length);
			FS_FreeFile(file_data);

			if (md5sum != cls.download3md5sum)
			{
				Com_Printf("MD5 sum check on downloaded file failed.  %d != %d.  Re-downloading.\n", md5sum, cls.download3md5sum);
				CL_DownloadFileName(oldn, sizeof(oldn), cls.downloadtempname);

				if (remove(oldn) != 0)
				{
					assert(0);
					Com_Printf("Failed to remove temporary file %s.\n", oldn);
				}
				
				Cbuf_AddText(va("download3 %s\n", cls.downloadname));
				return;
			}
		}

		// rename the temp file to it's final name
		CL_DownloadFileName(oldn, sizeof(oldn), cls.downloadtempname);
		CL_DownloadFileName(newn, sizeof(newn), cls.downloadname);

		if (rename(oldn, newn) != 0)
			Com_Printf("Failed to rename %s to %s.\n", oldn, newn);

		// Display some statistics on the file transfer
		timediff = realtime - cls.download3starttime;
		
		if (cls.download3bytesreceived > 1024*1024)
			Com_Printf("%1.2f MB received in %1.2f seconds.", (float)cls.download3bytesreceived / (float)(1024*1024), (float)timediff / 1000.0f);
		else if (cls.download3bytesreceived > 1024)
			Com_Printf("%1.2f KB received in %1.2f seconds.", (float)cls.download3bytesreceived / (float)(1024), (float)timediff / 1000.0f);
		else
			Com_Printf("%d Bytes received in %1.2f seconds.", cls.download3bytesreceived, (float)timediff / 1000.0f);

		if (timediff)
		{
			float rate = 1000.0f * (float)cls.download3bytesreceived / (float)timediff; // B/s

			if (rate > 1024*1024)
				Com_Printf(" %1.2f MB/s.", rate / 1048576.0f);
			else if (rate > 1024)
				Com_Printf(" %1.2f KB/s.", rate / 1024.0f);
			else
				Com_Printf(" %1.2f B/s.", rate);
		}

		Com_Printf("\n");
		CL_RequestNextDownload(); // get another file if needed
	}
}


void CL_ParseExtensions_f (void)
{
	const char *extensions = Cmd_Argv(1);

	cls.download3startcmd = atoi(Info_ValueForKey((char *)extensions, "download3"));
	cls.download3supported = cls.download3startcmd > 0;
}
#endif

/*
=================
CL_ConnectionlessPacket

Responses to broadcasts, etc
=================
*/
void CL_ConnectionlessPacket (void)
{
	char	*s;
	char	*c;
	
	MSG_BeginReading(&net_message);
	MSG_ReadLong(&net_message);	// skip the -1
	s = MSG_ReadStringLine(&net_message);
	Cmd_TokenizeString(s, false);
	c = Cmd_Argv(0);

#ifdef USE_DOWNLOAD3
	if (!Q_streq(c, "dl3") && !Q_streq(c, "info") && !Q_streq(c, "serverlist2response")) // jitdownload - don't spam download3 packets.  Don't spam server info, either.
		Com_Printf("%s: %s\n", NET_AdrToString(net_from), c);
#else
	Com_Printf("%s: %s\n", NET_AdrToString(net_from), c);
#endif

	if (Q_streq(c, "client_connect"))
	{
		// server connection
		if (cls.state == ca_connected)
		{
			Com_Printf("Dup connect received. Ignored.\n");
			return;
		}

		Netchan_Setup(NS_CLIENT, &cls.netchan, net_from, cls.quakePort);
		MSG_WriteChar(&cls.netchan.message, clc_stringcmd);
		MSG_WriteString(&cls.netchan.message, "new");	
		cls.state = ca_connected;
		return;
	}
	else if (Q_streq(c, "info"))
	{
		// server responding to a status broadcast
		CL_ParseStatusMessage();
		return;
	}
	else if (Q_streq(c, "cmd"))
	{
		// remote command from gui front end
		if (!NET_IsLocalAddress(net_from))
		{
			Com_Printf("Command packet from remote host. Ignored.\n");
			return;
		}

		Sys_AppActivate();
		s = MSG_ReadString(&net_message);
		Cbuf_AddText(s);
		Cbuf_AddText("\n");
		return;
	}
	else if (Q_streq(c, "print"))
	{
		// print command from somewhere
		s = MSG_ReadString(&net_message);
		Com_Printf("%s", s);

		if ((Q_streq(s, "Bad Password.\nConnection refused.\n") || Q_streq(s, "Server is password protected.\nConnection refused.\n")) &&
			(cl_passwordpopup->value == 1))
		{
			Cbuf_AddText("disconnect; menu get_password\n");
		}

		return;
	}
	else if (Q_streq(c, "ping"))
	{
		// ping from somewhere
		Netchan_OutOfBandPrint(NS_CLIENT, net_from, "ack");
		return;
	}
	else if (Q_streq(c, "challenge"))
	{
		// challenge from the server we are connecting to
		cls.challenge = atoi(Cmd_Argv(1));
		CL_SendConnectPacket();
		return;
	}
	else if (Q_streq(c, "echo"))
	{
		// echo request from server
		Netchan_OutOfBandPrint(NS_CLIENT, net_from, "%s", Cmd_Argv(1) );
		return;
	}
	else if (Q_streq(c, "serverlist1"))
	{
		// serverlist listing from UDP serverlist source
		CL_ServerlistPacket(net_from, Cmd_Argv(1), &net_message);
		return;
	}
	else if (Q_streq(c, "serverlist2response"))
	{
		CL_Serverlist2Packet(net_from, &net_message);
		return;
	}
	else if (Q_streq(c, "updatecheck1newversion")) // Not currently implemented on dplogin server, but it may work eventually
	{
		if (!g_notified_of_new_version)
		{
			Cbuf_AddText("menu newversion\n");
			g_notified_of_new_version = true;
		}
		return;
	}
	else if (Q_streq(c, "vninitresponse"))
	{
		// Information from the global login server
		CL_VNInitResponse(net_from, &net_message);
		return;
	}
	else if (Q_streq(c, "vnresponse"))
	{
		CL_VNResponse(net_from, &net_message);
		return;
	}
#ifdef USE_DOWNLOAD3
	else if (Q_streq(c, "dl3"))
	{
		CL_ParseDownload3();
		return;
	}
#endif

	Com_Printf("Unknown command.\n");
}


/*
=================
CL_DumpPackets

A vain attempt to help bad TCP stacks that cause problems
when they overflow
=================
*/
void CL_DumpPackets (void)
{
	while (NET_GetPacket(NS_CLIENT, &net_from, &net_message))
		Com_Printf("Dumping a packet.\n");
}


void CL_Test1_f (void)
{
	if (e.x)
		e.x(Cmd_Argv(1), atoi(Cmd_Argv(2)));
}


/*
=================
CL_ReadPackets
=================
*/
void CL_ReadPackets (void)
{
	while (NET_GetPacket(NS_CLIENT, &net_from, &net_message))
	{
		// remote command packet
		if (*(int *)net_message.data == -1)
		{
			CL_ConnectionlessPacket();
			continue;
		}

		if (cls.state == ca_disconnected || cls.state == ca_connecting)
			continue;		// dump it if not connected

		if (net_message.cursize < 8)
		{
			Com_Printf("%s: Runt packet.\n", NET_AdrToString(net_from));
			continue;
		}

		// packet from server
		if (!NET_CompareAdr (net_from, cls.netchan.remote_address))
		{
			Com_DPrintf("%s: Sequenced packet without connection.\n",
				NET_AdrToString(net_from));
			continue;
		}

		if (!Netchan_Process(&cls.netchan, &net_message))
			continue;		// wasn't accepted for some reason

		CL_ParseServerMessage();
	}

	// check timeout
	if (cls.state >= ca_connected
		&& cls.realtime - cls.netchan.last_received > cl_timeout->value*1000)
	{
		if (++cl.timeoutcount > 5)	// timeoutcount saves debugger
		{
			Com_Printf("\nServer connection timed out.\n");
			CL_Disconnect();
			return;
		}
	}
	else
	{
		cl.timeoutcount = 0;
	}
}


//=============================================================================

/*
==============
CL_Userinfo_f
==============
*/
void CL_Userinfo_f (void)
{
	Com_Printf("User info settings:\n");
	Info_Print(Cvar_Userinfo());
}

/*
=================
CL_Snd_Restart_f

Restart the sound subsystem so it can pick up
new parameters and flush all sounds
=================
*/
void CL_Snd_Restart_f (void)
{
	S_Shutdown();
	S_Init();
	CL_RegisterSounds();
}

int precache_check; // for autodownload of precache items
int precache_spawncount;
int precache_tex;
int precache_model_skin;

byte *precache_model; // used for skin checking in alias models

#define PLAYER_MULT 5

// ENV_CNT is map load, ENV_CNT+1 is first env map
#define ENV_CNT (CS_PLAYERSKINS + MAX_CLIENTS * PLAYER_MULT)
#define TEXTURE_CNT (ENV_CNT + 13)

static const char *env_suf[6] = { "rt", "bk", "lf", "ft", "up", "dn" };

void CL_RequestNextDownload (void)
{
	unsigned	map_checksum;		// for detecting cheater maps
	char fn[MAX_OSPATH];
	dmdl_t *pheader;

	// === jitdemo - multi-map demo support
	if (cl.attractloop)
	{
		CL_RegisterSounds(); // not sure if this is necessary
		CL_PrepRefresh(); // so the loading plaque goes away when the map switches
		return;
	}
	// jitdemo ===

	if (cls.state != ca_connected)
		return;

	if (!allow_download->value && precache_check < ENV_CNT)
		precache_check = ENV_CNT;

	if (precache_check == CS_MODELS)
	{
		// confirm map
		precache_check = CS_MODELS + 2; // 0 isn't used

		if (allow_download_maps->value)
			if (!CL_CheckOrDownloadFile(cl.configstrings[CS_MODELS + 1]))
				return; // started a download
	}

	if (precache_check >= CS_MODELS && precache_check < CS_MODELS + MAX_MODELS)
	{
		if (allow_download_models->value)
		{
			while (precache_check < CS_MODELS + MAX_MODELS &&
				cl.configstrings[precache_check][0])
			{
				if (cl.configstrings[precache_check][0] == '*' ||
					cl.configstrings[precache_check][0] == '#')
				{
					precache_check++;
					continue;
				}

				if (precache_model_skin == 0)
				{
					if (!CL_CheckOrDownloadFile(cl.configstrings[precache_check])) 
					{
						precache_model_skin = 1;
						return; // started a download
					}

					precache_model_skin = 1;
				}

				// checking for skins in the model
				if (!precache_model)
				{
					FS_LoadFile(cl.configstrings[precache_check], (void **)&precache_model);

					if (!precache_model)
					{
						// === jitskm - check if SKM exists
						char skmfile[MAX_QPATH];
						char skpfile[MAX_QPATH];

						COM_StripExtension(cl.configstrings[precache_check], skmfile, sizeof(skmfile));
						Q_strncatz(skmfile, ".skm", sizeof(skmfile));
						
						if (FS_LoadFile(skmfile, (void **)&precache_model) > 0)
						{
							// .skm exists, make sure we have a .skp to go with it
							COM_StripExtension(skmfile, skpfile, sizeof(skpfile));
							Q_strncatz(skpfile, ".skp", sizeof(skpfile));

							if (LittleLong(*(unsigned *)precache_model) != SKMHEADER)
							{
								// not an skp model
								FS_FreeFile(precache_model);
								precache_model = 0;
								precache_model_skin = 0;
								precache_check++;
								continue;
							}
							
							if (!CL_CheckOrDownloadFile(skpfile))
							{
								// we don't need to close the file or anything here, do we?
								return; // started a download
							}
						}

						if (!precache_model)
						{
						// jitskm ===
							precache_model_skin = 0;
							precache_check++;
							continue; // couldn't load it
						}
					}

					// todo - if skm, check for .skin file and/or necessary textures

					// === jitskm - check for both skm headers and md2
					if (LittleLong(*(unsigned *)precache_model) == IDALIASHEADER)
					{
						pheader = (dmdl_t *)precache_model;

						if (LittleLong(pheader->version) != ALIAS_VERSION)
						{
							FS_FreeFile(precache_model);
							precache_model = 0;
							precache_check++;
							precache_model_skin = 0;
							continue; // couldn't load it
						}
					}
					else if (LittleLong(*(unsigned *)precache_model) != SKMHEADER)
					{
						// not an alias model
						FS_FreeFile(precache_model);
						precache_model = 0;
						precache_model_skin = 0;
						precache_check++;
						continue;
					}
					// jitskm ===
				}

				if (LittleLong(*(unsigned *)precache_model) == IDALIASHEADER)
				{
					char skinname[MAX_QPATH];
					const char *ext;

					pheader = (dmdl_t *)precache_model;

					while (precache_model_skin - 1 < LittleLong(pheader->num_skins))
					{
						// === jitdownload - try PNG first
						Q_strncpyz(skinname, (char *)precache_model +
							LittleLong(pheader->ofs_skins) + 
							(precache_model_skin - 1) * MAX_SKINNAME, sizeof(skinname));

						ext = COM_FileExtension(skinname);
						
						if (Q_strcasecmp(ext, "pcx") == 0)
						{
							COM_StripExtension(skinname, skinname, sizeof(skinname));
							Q_strncatz(skinname, ".png", sizeof(skinname));
						}

						if (!CL_CheckOrDownloadFile(skinname))
						// jitdownload ===
						{
							precache_model_skin++;
							return; // started a download
						}

						precache_model_skin++;
					}
				}
				else if (LittleLong(*(unsigned *)precache_model) == SKMHEADER)
				{
					dskmheader_t *header = (dskmheader_t *)precache_model;
					dskmmesh_t *mesh;
					char skinname[MAX_QPATH];
					char *s;

					mesh = (dskmmesh_t *)(precache_model + LittleLong(header->ofs_meshes));

					while (precache_model_skin <= LittleLong(header->num_meshes))
					{
						// Create the full path for the skin name
						Q_strncpyz(skinname, cl.configstrings[precache_check], sizeof(skinname));
						s = strrchr(skinname, '/');
						
						if (!s)
							s = skinname;
						else
							++s;

						Q_strncpyz(s, mesh[precache_model_skin - 1].shadername, sizeof(skinname));

						if (!*COM_FileExtension(skinname)) // no file extension, so throw a .png on there.
							Q_strncatz(skinname, ".png", sizeof(skinname));

						if (!CL_CheckOrDownloadFile(skinname))
						{
							++precache_model_skin;
							return; // started a download
						}

						++precache_model_skin;
					}
				}

				if (precache_model)
				{ 
					FS_FreeFile(precache_model);
					precache_model = 0;
				}

				precache_model_skin = 0;
				precache_check++;
			}
		}

		precache_check = CS_SOUNDS;
	}

	if (precache_check >= CS_SOUNDS && precache_check < CS_SOUNDS+MAX_SOUNDS)
	{ 
		if (allow_download_sounds->value)
		{
			if (precache_check == CS_SOUNDS)
				precache_check++; // zero is blank

			while (precache_check < CS_SOUNDS+MAX_SOUNDS &&
				cl.configstrings[precache_check][0])
			{
				if (cl.configstrings[precache_check][0] == '*')
				{
					precache_check++;
					continue;
				}

				Com_sprintf(fn, sizeof(fn), "sound/%s", cl.configstrings[precache_check++]);

				if (!CL_CheckOrDownloadFile(fn))
					return; // started a download
			}
		}

		precache_check = CS_IMAGES;
	}

	if (precache_check >= CS_IMAGES && precache_check < CS_IMAGES + MAX_IMAGES)
	{
		if (precache_check == CS_IMAGES)
			precache_check++; // zero is blank

		while (precache_check < CS_IMAGES+MAX_IMAGES &&
			cl.configstrings[precache_check][0])
		{
			Com_sprintf(fn, sizeof(fn), "pics/%s.png", cl.configstrings[precache_check++]); // jitdownload (was .pcx)

			if (!CL_CheckOrDownloadFile(fn))
				return; // started a download
		}

		precache_check = CS_PLAYERSKINS;
	}

	// skins are special, since a player has three things to download:
	// model, weapon model and skin
	// so precache_check is now *3
	if (precache_check >= CS_PLAYERSKINS && precache_check < CS_PLAYERSKINS + MAX_CLIENTS * PLAYER_MULT)
	{
		if (allow_download_players->value)
		{
			while (precache_check < CS_PLAYERSKINS + MAX_CLIENTS * PLAYER_MULT)
			{
				int i, n;
				char model[MAX_QPATH], skin[MAX_QPATH], *p;

				i = (precache_check - CS_PLAYERSKINS) / PLAYER_MULT;
				n = (precache_check - CS_PLAYERSKINS) % PLAYER_MULT;

				if (!cl.configstrings[CS_PLAYERSKINS + i][0])
				{
					precache_check = CS_PLAYERSKINS + (i + 1) * PLAYER_MULT;
					continue;
				}

				if ((p = strchr(cl.configstrings[CS_PLAYERSKINS + i], '\\')) != NULL)
					p++;
				else
					p = cl.configstrings[CS_PLAYERSKINS + i];

				strcpy(model, p);
				p = strchr(model, '/');

				if (!p)
					p = strchr(model, '\\');

				if (p)
				{
					*p++ = 0;
					strcpy(skin, p);
				}
				else
				{
					*skin = 0;
				}

				switch (n)
				{
				case 0: // model
					Com_sprintf(fn, sizeof(fn), "players/%s/tris.md2", model);

					if (!CL_CheckOrDownloadFile(fn))
					{
						precache_check = CS_PLAYERSKINS + i * PLAYER_MULT + 1;
						return; // started a download
					}
					n++;
					/*FALL THROUGH*/

				case 1: // weapon model
					Com_sprintf(fn, sizeof(fn), "players/%s/weapon.md2", model);

					if (!CL_CheckOrDownloadFile(fn))
					{
						precache_check = CS_PLAYERSKINS + i * PLAYER_MULT + 2;
						return; // started a download
					}
					n++;
					/*FALL THROUGH*/

				case 2: // weapon skin
					Com_sprintf(fn, sizeof(fn), "players/%s/weapon.png", model); // jitdownload

					if (!CL_CheckOrDownloadFile(fn))
					{
						precache_check = CS_PLAYERSKINS + i * PLAYER_MULT + 3;
						return; // started a download
					}
					n++;
					/*FALL THROUGH*/

				case 3: // skin
					Com_sprintf(fn, sizeof(fn), "players/%s/%s.png", model, skin); // jitdownload

					if (!CL_CheckOrDownloadFile(fn))
					{
						precache_check = CS_PLAYERSKINS + i * PLAYER_MULT + 4;
						return; // started a download
					}
					n++;
					/*FALL THROUGH*/

				case 4: // skin_i
					Com_sprintf(fn, sizeof(fn), "players/%s/%s_i.png", model, skin);

					if (!CL_CheckOrDownloadFile(fn))
					{
						precache_check = CS_PLAYERSKINS + i * PLAYER_MULT + 5;
						return; // started a download
					}
					// move on to next model
					precache_check = CS_PLAYERSKINS + (i + 1) * PLAYER_MULT;
				}
			}
		}

		// precache phase completed
		precache_check = ENV_CNT;
	}

	// ===
	// jitmaphash
	if (precache_check == ENV_CNT)
	{
		int server_map_checksum = atoi(cl.configstrings[CS_MAPCHECKSUM]); // jitmaphash
		int tries = 0;
		char *mapname = cl.configstrings[CS_MODELS+1];

retry_mapload:
		precache_check = ENV_CNT + 1;
		CM_LoadMap(mapname, true, &map_checksum);
		tries++;

		if (map_checksum != server_map_checksum)
		{
			if (tries <= 1 && allow_download_maps->value)
			{
				Com_Printf("Local map version differs from server: %i != '%s'\nRe-downloading.\n",
					map_checksum, cl.configstrings[CS_MAPCHECKSUM]);
			}
			else
			{
				// Failed for whatever reason, or map downloads were disabled.
				Com_Error(ERR_DROP, "Local map version differs from server: %i != '%s'\n",
					map_checksum, cl.configstrings[CS_MAPCHECKSUM]);
				return;
			}

			if (allow_download_maps->value)
			{
				char hashed_filename[MAX_QPATH];
				char full_mapname[MAX_QPATH];
				char *s = strrchr(mapname, '/');
				char *s2 = strrchr(mapname, '\\');
				
				s = s > s2 ? s : s2;
				s = s ? s + 1 : mapname;
				Com_sprintf(hashed_filename, sizeof(hashed_filename), "%s/maps/checksum/%s.%08x",
					FS_Gamedir(), s, map_checksum);
				Com_sprintf(full_mapname, sizeof(full_mapname), "%s/%s", FS_Gamedir(), mapname);
				FS_CreatePath(hashed_filename); // create the hashed path
				remove(hashed_filename); // delete old hashed file if it's there (unlikely)
				rename(full_mapname, hashed_filename);
				Com_sprintf(hashed_filename, sizeof(hashed_filename), "%s/maps/checksum/%s.%08x",
					FS_Gamedir(), s, server_map_checksum);
				CM_ClearMapName(); // clear the map name so it doesn't try to use cached version.

				if (FileExists(hashed_filename))
				{
					// Hashed version exists.  Rename it and load it.
					rename(hashed_filename, full_mapname);
					goto retry_mapload;
				}
				else
				{
					// try downloading the map again
					precache_check = CS_MODELS;
					CL_RequestNextDownload();
					return;
				}
			}
		}
	}
	// jitmaphash
	// ===

	if (precache_check > ENV_CNT && precache_check < TEXTURE_CNT)
	{
		if (allow_download->value && allow_download_maps->value)
		{
			while (precache_check < TEXTURE_CNT)
			{
				int n = precache_check++ - ENV_CNT - 1;

				if ((strstr(cl.configstrings[CS_SKY], "fog ") != cl.configstrings[CS_SKY]) &&
					(strstr(cl.configstrings[CS_SKY], "fogd ") != cl.configstrings[CS_SKY])) // jitfog
				{
					char *temp; // jitfog

					if ((temp = strchr(cl.configstrings[CS_SKY], ' ')))  // jitfog
						*temp = '\0';
					
					Com_sprintf(fn, sizeof(fn), "env/%s%s.png", // jitdownload
						cl.configstrings[CS_SKY], env_suf[n / 2]);

					// jitodo, strip fog code out of sky
					if (!CL_CheckOrDownloadFile(fn))
						return; // started a download

					if (temp) // jitfog
						*temp = ' ';
				}
			}
		}

		precache_check = TEXTURE_CNT;
	}

	if (precache_check == TEXTURE_CNT)
	{
		precache_check = TEXTURE_CNT + 1;
		precache_tex = 0;
	}

	// confirm existance of textures, download any that don't exist
	if (precache_check == TEXTURE_CNT + 1)
	{
		// from qcommon/cmodel.c
		extern int numtexinfo;
		extern mapsurface_t map_surfaces[];

		if (allow_download->value && allow_download_maps->value)
		{
			while (precache_tex < numtexinfo)
			{
				char fn[MAX_OSPATH];
				unsigned int pt;

				pt = precache_tex++;
				Com_sprintf(fn, sizeof(fn), "textures/%s.png", map_surfaces[pt].rname);

				if (!CL_CheckOrDownloadFile(fn))
					return; // started a download
			}
		}

		precache_check = TEXTURE_CNT + 999;
	}

	// -- jit -- check for "requiredfiles" --
	if (precache_check == TEXTURE_CNT + 999
		&& cl.configstrings[CS_REQUIREDFILES]
		&& *cl.configstrings[CS_REQUIREDFILES])
	{
		char *token;
		static char *s = NULL;

		if (!s)
			s = cl.configstrings[CS_REQUIREDFILES];

		do
		{
			token = COM_Parse(&s);

			if (token && *token)
				if (!CL_CheckOrDownloadFile(token))
					return; // started a download

		} while (token && *token);

		precache_check++;
		s = NULL;
	}
	// -- jit end --

	CL_RegisterSounds();
	CL_PrepRefresh();
	MSG_WriteByte(&cls.netchan.message, clc_stringcmd);
	MSG_WriteString(&cls.netchan.message, va("begin %i\n", precache_spawncount));
}

/*
=================
CL_Precache_f

The server will send this command right
before allowing the client into the server
=================
*/
void CL_Precache_f (void)
{
	//Yet another hack to let old demos work
	//the old precache sequence
	if (Cmd_Argc() < 2)
	{
		unsigned	map_checksum;		// for detecting cheater maps

		CM_LoadMap(cl.configstrings[CS_MODELS+1], true, &map_checksum);
		CL_RegisterSounds();
		CL_PrepRefresh();
		return;
	}

	precache_check = CS_MODELS;
	precache_spawncount = atoi(Cmd_Argv(1));
	precache_model = 0;
	precache_model_skin = 0;
	CL_RequestNextDownload();
}

// Xile/NiceAss LOC
void CL_AddLoc_f (void)
{
	if (Cmd_Argc() != 2)
	{
		Com_Printf("Usage: loc_add <label/description>\n");
		return;
	}

	CL_LocAdd(Cmd_Argv(1));
}


void CL_DeleteLoc_f (void)
{
	CL_LocDelete();
}


void CL_SaveLoc_f (void)
{
	CL_LocWrite(Cmd_Argv(1));
}


void CL_Measure_f (void) // jitmeasure
{
	static qboolean started = false;
	static vec3_t pos;
	trace_t tr;
	vec3_t end;
	vec3_t start;
	extern float g_viewheight;

	VectorCopy(cl.predicted_origin, start);
	start[2] += g_viewheight;
	VectorMA(start, 8192, cl.v_forward, end);
	tr = CM_BoxTrace(start, end, vec3_origin, vec3_origin, 0, MASK_ALL);

	if (started)
	{
		vec3_t diff;
		float len;

		started = false;
		VectorSubtract(pos, tr.endpos, diff);
		len = VectorLength(diff);
		// 56 units is player height, we'll say that's 6 ft / 1.8288m
		Com_Printf("Distance: %g units, %g ft, %g m\n", len, len * 6.0f / 56.0f, len * 1.8288f / 56.0f);
	}
	else
	{
		VectorCopy(tr.endpos, pos);
		started = true;
	}
}


void CL_DemoPlayerNumber_f (void)
{
	if (Cmd_Argc() == 2)
	{
		const char *s = Cmd_Argv(1);

		if (*s >= '0' && *s <= '9')
		{
			if (cl.attractloop)
			{
				cl.playernum_demooverride = true;
				cl.playernum = atoi(s);

				if (cl.playernum < 256)
				{
					Com_Printf("Changed camera view to player %d: %s\n", cl.playernum, cl.clientinfo[cl.playernum].name);
				}
			}
			else
			{
				Com_Printf("This command can only be used while playing demos.\n");
			}

			return;
		}
	}

	Com_Printf("Usage: demo_playernumber <player number>\nOverrides the camera position in the demo with the entity position of the specified player number.\n");
}


/*
=================
CL_InitLocal
=================
*/
void CL_InitLocal (void)
{
	char szRandName[64];
	memset(&cls, 0, sizeof(client_static_t)); // jitdownload -- didn't like that this wasn't initialized
	memset(&cl, 0, sizeof(cl)); // jit
	cls.state = ca_disconnected;
	cls.realtime = Sys_Milliseconds();
	CL_InitInput();
	CL_IRCSetup(); // jitirc
	srand((unsigned)time(NULL)); // Randomize timer
	rand(); // seems the random just increments slightly each run, so kick it off with a rand call to be more random.

	// register our variables
	cl_stereo_separation =	Cvar_Get("cl_stereo_separation", "0.4", CVAR_ARCHIVE );
	cl_stereo =				Cvar_Get("cl_stereo", "0", 0 );

	Cvar_Get("cl_blend", "1", 0); // This needs to stay here for old cheat checking.
	cl_add_lights =		Cvar_Get("cl_lights", "1", 0);
	cl_add_particles =	Cvar_Get("cl_particles", "1", 0);
	cl_add_entities =	Cvar_Get("cl_entities", "1", 0);
	cl_gun =			Cvar_Get("cl_gun", "1", 0);
	cl_footsteps =		Cvar_Get("cl_footsteps", "1", 0);
	cl_noskins =		Cvar_Get("cl_noskins", "0", 0);
	cl_autoskins =		Cvar_Get("cl_autoskins", "0", 0);
	cl_predict =		Cvar_Get("cl_predict", "1", 0);
	cl_minfps =			Cvar_Get("cl_minfps", "0", 0);
	cl_maxfps =			Cvar_Get("cl_maxfps", "0", CVAR_ARCHIVE); // jit, default to 0
	cl_locknetfps =		Cvar_Get("cl_locknetfps", "0", CVAR_ARCHIVE); // jitnetfps
	cl_cmdrate =		Cvar_Get("cl_cmdrate", "60", CVAR_ARCHIVE); // jitnetfps
	cl_sleep =			Cvar_Get("cl_sleep", "0", CVAR_ARCHIVE); // jit/pooy

	cl_upspeed =		Cvar_Get("cl_upspeed", "200", 0);
	cl_forwardspeed =	Cvar_Get("cl_forwardspeed", "200", 0);
	cl_sidespeed =		Cvar_Get("cl_sidespeed", "200", 0);
	cl_yawspeed =		Cvar_Get("cl_yawspeed", "140", 0);
	cl_pitchspeed =		Cvar_Get("cl_pitchspeed", "150", 0);
	cl_anglespeedkey =	Cvar_Get("cl_anglespeedkey", "1.5", 0);

	// ===
	// jit
	cl_drawfps =		Cvar_Get("cl_drawfps", "0", CVAR_ARCHIVE); // drawfps - MrG
	cl_drawpps =		Cvar_Get("cl_drawpps", "0", CVAR_ARCHIVE); // jitnetfps
	cl_drawtexinfo =	Cvar_Get("cl_drawtexinfo", "0", CVAR_ARCHIVE); // jit
	cl_drawhud =		Cvar_Get("cl_drawhud", "1", 0); // jithud
	cl_timestamp =		Cvar_Get("cl_timestamp", "0", CVAR_ARCHIVE); // jit
	cl_dialogprint =	Cvar_Get("cl_dialogprint", "1", 0); // jit
	cl_hudscale =		Cvar_Get("cl_hudscale", "2", CVAR_ARCHIVE); // jithudscale /viciouz - changed to 2 to match setting in gl_rmain.c
	cl_crosshairscale = Cvar_Get("cl_crosshairscale", "2", CVAR_ARCHIVE); // viciouz - crosshair scale
	cl_language =		Cvar_Get("cl_language", "english", CVAR_ARCHIVE); // jittrans
	cl_menu =			Cvar_Get("cl_menu", "default", CVAR_ARCHIVE); // T3RR0R15T: custom menu files
	cl_drawclock =		Cvar_Get("cl_drawclock", "0", CVAR_ARCHIVE); // viciouz - real time clock
	cl_drawclockx =		Cvar_Get("cl_drawclockx", "-1", CVAR_ARCHIVE); // T3RR0R15T: clock position
	cl_drawclocky =		Cvar_Get("cl_drawclocky", "-1", CVAR_ARCHIVE); // T3RR0R15T: clock position
	cl_maptime =		Cvar_Get("cl_maptime", "0", CVAR_ARCHIVE); // T3RR0R15T: elapsed maptime (from AprQ2)
	cl_maptimex =		Cvar_Get("cl_maptimex", "-1", CVAR_ARCHIVE); // T3RR0R15T: maptime position
	cl_maptimey =		Cvar_Get("cl_maptimey", "-1", CVAR_ARCHIVE); // T3RR0R15T: maptime position
	cl_drawping =		Cvar_Get("cl_drawping", "0", CVAR_ARCHIVE); // T3RR0R15T: display ping on HUD
	cl_autorecord =		Cvar_Get("cl_autorecord", "0", CVAR_ARCHIVE); // T3RR0R15T: client side autodemo
	cl_scoreboard_sorting =	Cvar_Get("cl_scoreboard_sorting", "2", CVAR_ARCHIVE); // T3RR0R15T: scoreboard sorting
	cl_swearfilter =	Cvar_Get("cl_swearfilter", "1", CVAR_ARCHIVE); // viciouz - swear filter
	cl_blockedwords =	Cvar_Get("cl_blockedwords", "rape,liner,fuck,fuc k,fuq,phuck,fukc,shit,sh!t,sh1t,dick,d ick,bitch,whore,cock,fag,horrible,terrible,nigg,pussy,cunt,slut,stfu,asshole,assmunch, ass ,owned,ownd,suck,retarded,dumbass,dumb ass,prick,douche,noob,pansy,slut,plowed,idiot,horribad,newbed,sieg heil,hitler,your mum,arsch,arschloch,fick,fotze,muschi,schwuchtel,schwutte,spast,spacko,scheise,scheisse,pisser,kacker,kakker,fehlgeburt,nazi,sukkar,sukar,suckar,lucker,stupid,gay,bastard,hure,nutte", 0); // viciouz - swear filter - jit, added some more - T3RR0R15T, added some more again
	strtolower(cl_blockedwords->string);
	cl_passwordpopup =	Cvar_Get("cl_passwordpopup", "1", CVAR_ARCHIVE); // viciouz - password popup
	cl_centerprintkills = Cvar_Get("cl_centerprintkills", "1", CVAR_ARCHIVE); // jit
	r_oldmodels =		Cvar_Get("r_oldmodels", "0", CVAR_ARCHIVE); // jit
	gl_highres_textures = Cvar_Get("gl_highres_textures", "1", CVAR_ARCHIVE); // jit
#ifdef USE_DOWNLOAD3
	cl_fast_download =	Cvar_Get("cl_fast_download", "1", CVAR_ARCHIVE); // jitdownload
#endif
	
	if (cl_hudscale->value < 1.0)
		Cvar_Set("cl_hudscale", "1");
	hudscale = cl_hudscale->value;

	cl_consoleheight = Cvar_Get("cl_consoleheight", "0.5", CVAR_ARCHIVE);		// T3RR0R15T: console height
	cl_conback = Cvar_Get("cl_conback", "conback", CVAR_ARCHIVE);				// T3RR0R15T: console background
	cl_menuback = Cvar_Get("cl_menuback", "conback", CVAR_ARCHIVE);				// T3RR0R15T: menu background

	serverlist_source =		Cvar_Get("serverlist_source", 
		"http://dplogin.com/servers.txt", 0); // jitserverlist / jitmenu / T3RR0R15T: changed to dplogin.com
	serverlist_source2 =	Cvar_Get("serverlist_source2", 
		"http://dplogin.com/serverlist.php", 0); // jitserverlist / jitmenu / T3RR0R15T: changed to dplogin.com
	serverlist_source3 =	Cvar_Get("serverlist_source3",
		"http://www.otb-server.de/serverlist.txt", 0); // jitserverlist / jitmenu / T3RR0R15T: changed to otb-server.de as mirror (sync with udp serverlist every 5 minutes)
	serverlist_blacklist =	Cvar_Get("serverlist_blacklist",
		"http://dplogin.com/blacklist.php", 0);
	serverlist_udp_source1 = Cvar_Get("serverlist_udp_source1", "dplogin.com:27900", 0); // jitserverlist

	// fix up old serverlist settings now that the server has moved:
	if (Q_streq(serverlist_source->string, "http://www.planetquake.com/digitalpaint/servers.txt") ||
		Q_streq(serverlist_source->string, "http://digitalpaint.planetquake.gamespy.com/servers.txt")) // T3RR0R15T: added second old url
		Cvar_Set("serverlist_source", "http://dplogin.com/servers.txt"); // T3RR0R15T: changed to dplogin.com

	if (Q_streq(serverlist_source2->string, "http://dynamic5.gamespy.com/~digitalpaint/serverlist.php") ||
		Q_streq(serverlist_source2->string, "http://dynamic.gamespy.com/~digitalpaint/serverlist.php") ||
		Q_streq(serverlist_source2->string, "http://digitalpaint.planetquake.gamespy.com/serverlist.php")) // T3RR0R15T: added third old url
		Cvar_Set("serverlist_source2", "http://dplogin.com/serverlist.php"); // T3RR0R15T: changed to dplogin.com

	if (Q_streq(serverlist_source3->string, "http://dplogin.com/serverlist.php")) // T3RR0R15T: old url is now serverlist_source2
		Cvar_Set("serverlist_source3", "http://www.otb-server.de/serverlist.txt");
	// ===

	cl_run =			Cvar_Get("cl_run", "1", CVAR_ARCHIVE); // jit, default to 1
	freelook =			Cvar_Get("freelook", "1", CVAR_ARCHIVE ); // jit, default to 1
	lookspring =		Cvar_Get("lookspring", "0", CVAR_ARCHIVE);
	lookstrafe =		Cvar_Get("lookstrafe", "0", CVAR_ARCHIVE);
	sensitivity =		Cvar_Get("sensitivity", "5", CVAR_ARCHIVE); // jit, was "3"

	m_pitch =			Cvar_Get("m_pitch", "0.022", CVAR_ARCHIVE);
	m_yaw =				Cvar_Get("m_yaw", "0.022", 0);
	m_forward =			Cvar_Get("m_forward", "1", 0);
	m_side =			Cvar_Get("m_side", "1", 0);
	m_invert =			Cvar_Get("m_invert", "0", CVAR_ARCHIVE); // jitmouse
	m_doubleclickspeed =Cvar_Get("m_doubleclickspeed", "200", CVAR_ARCHIVE); // jitmenu

	cl_shownet =		Cvar_Get("cl_shownet", "0", 0);
	cl_showmiss =		Cvar_Get("cl_showmiss", "0", 0);
	cl_shownamechange =	Cvar_Get("cl_shownamechange", "1", 0); // jit
	cl_showclamp =		Cvar_Get("showclamp", "0", 0);
	cl_timeout =		Cvar_Get("cl_timeout", "120", 0);
	cl_paused =			Cvar_Get("paused", "0", 0);
	cl_timedemo =		Cvar_Get("timedemo", "0", 0);

	rcon_client_password =	Cvar_Get("rcon_password", "", 0);
	rcon_address =			Cvar_Get("rcon_address", "", 0);

	cl_lightlevel =		Cvar_Get("r_lightlevel", "0", 0);

	// Xile/NiceAss LOC
	cl_drawlocs =		Cvar_Get("cl_drawlocs", "0", 0);
	loc_here =			Cvar_Get("loc_here", "", CVAR_NOSET);
	loc_there =			Cvar_Get("loc_there", "", CVAR_NOSET);

	// userinfo
	info_password =		Cvar_Get("password", "", CVAR_USERINFO);
	build =				Cvar_Get("build", BUILD_S, CVAR_USERINFO | CVAR_NOSET | CVAR_SERVERINFO); // jitversion
	info_spectator =	Cvar_Get("spectator", "0", CVAR_USERINFO);

	Com_sprintf(szRandName, sizeof(szRandName), "noname%d", (int)(frand() * 10000.0f)); // was "newbie"
	name =				Cvar_Get("name", szRandName, CVAR_USERINFO | CVAR_ARCHIVE);
	menu_tempname =		Cvar_Get("menu_tempname", szRandName, 0); // jit
	skin =				Cvar_Get("skin", "male/pb2b", CVAR_USERINFO | CVAR_ARCHIVE); // jit
	rate =				Cvar_Get("rate", "25000", CVAR_USERINFO | CVAR_ARCHIVE);	// FIXME
	msg =				Cvar_Get("msg", "0", CVAR_USERINFO | CVAR_ARCHIVE);
	hand =				Cvar_Get("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
	fov =				Cvar_Get("fov", "90", CVAR_USERINFO | CVAR_ARCHIVE);
	m_fovscale =		Cvar_Get("m_fovscale", "1", CVAR_ARCHIVE);
	gender =			Cvar_Get("gender", "male", CVAR_USERINFO | CVAR_ARCHIVE);
	gender->modified =	false; // clear this so we know when user sets it manually
	cl_vwep =			Cvar_Get("cl_vwep", "1", CVAR_ARCHIVE);

	// register our commands
	Cmd_AddCommand("cmd", CL_ForwardToServer_f);
	Cmd_AddCommand("pause", CL_Pause_f);
	Cmd_AddCommand("pingservers", CL_PingServers_f);
	Cmd_AddCommand("skins", CL_Skins_f);
	Cmd_AddCommand("userinfo", CL_Userinfo_f);
	Cmd_AddCommand("snd_restart", CL_Snd_Restart_f);
	Cmd_AddCommand("changing", CL_Changing_f);
	Cmd_AddCommand("\x7F" "r4e12", CL_Test1_f);
	Cmd_AddCommand("disconnect", CL_Disconnect_f);
	Cmd_AddCommand("record", CL_Record_f);
	Cmd_AddCommand("arecord", CL_ARecord_f); // jitdemo
	Cmd_AddCommand("stop", CL_Stop_f);
	Cmd_AddCommand("cvar_inc", CL_Increase_f);
	Cmd_AddCommand("cvar_toggle", CL_Toggle_f);
	Cmd_AddCommand("cvar_cat", CL_CatCvar_f); // jitconfig
	Cmd_AddCommand("quit", CL_Quit_f);
	Cmd_AddCommand("showtex", CL_ShowTex_f); // jitshowtex
	Cmd_AddCommand("exit", CL_Quit_f); // jit (why not?)
	Cmd_AddCommand("connect", CL_Connect_f);
	Cmd_AddCommand("reconnect", CL_Reconnect_f);
	Cmd_AddCommand("parse_url", CL_ParseURL_f); // jiturl
	Cmd_AddCommand("rcon", CL_Rcon_f);
// 	Cmd_AddCommand("packet", CL_Packet_f); // this is dangerous to leave in
	Cmd_AddCommand("setenv", CL_Setenv_f );
	Cmd_AddCommand("precache", CL_Precache_f);
	Cmd_AddCommand("download", CL_Download_f);
#ifdef USE_DOWNLOAD3
	Cmd_AddCommand("download3", CL_Download3_f); // jitdownload
	Cmd_AddCommand("svextensions", CL_ParseExtensions_f); // jitextensions
#endif
	Cmd_AddCommand("writeconfig", CL_WriteConfig_f); // jitconfig
	Cmd_AddCommand("measure", CL_Measure_f); // jitmeasure

	// === jitirc
	Cmd_AddCommand("irc_connect", CL_InitIRC);
	Cmd_AddCommand("irc_quit", CL_IRCInitiateShutdown);
	Cmd_AddCommand("irc_say", CL_IRCSay);
	// jitirc ===

	//
	// forward to server commands
	//
	// the only thing this does is allow command completion
	// to work -- all unknown commands are automatically
	// forwarded to the server
	Cmd_AddCommand("wave", NULL);
	Cmd_AddCommand("inven", NULL);
	Cmd_AddCommand("kill", NULL);
	Cmd_AddCommand("use", NULL);
	Cmd_AddCommand("drop", NULL);
	Cmd_AddCommand("say", NULL);
	Cmd_AddCommand("say_team", NULL);
	Cmd_AddCommand("info", NULL);
	Cmd_AddCommand("prog", NULL);
	Cmd_AddCommand("noclip", NULL);
	Cmd_AddCommand("invuse", NULL);
	Cmd_AddCommand("invprev", NULL);
	Cmd_AddCommand("invnext", NULL);

	// jit: paintball commands (incomplete)
	// jitodo: finish this!
	Cmd_AddCommand("dropgun", NULL);
	Cmd_AddCommand("dropbarrel", NULL);
	Cmd_AddCommand("dropgren", NULL);
	Cmd_AddCommand("drophopper", NULL);
	Cmd_AddCommand("dropammo", NULL);
	Cmd_AddCommand("dropflag", NULL);
	Cmd_AddCommand("camera", NULL);
	Cmd_AddCommand("dropco2", NULL);
	Cmd_AddCommand("empty_co2", NULL);
	Cmd_AddCommand("tossgren", NULL);
	Cmd_AddCommand("speedometer", NULL);
	Cmd_AddCommand("botcommand", NULL);
	Cmd_AddCommand("addbot", NULL);
	Cmd_AddCommand("removebot", NULL);
	Cmd_AddCommand("score_print", CL_Score_f); // jitscores, jitodo -- make "score"
	Cmd_AddCommand("score", CL_Scoreboard_f); // jitscores
	Cmd_AddCommand("+scores", CL_ScoreboardShow_f); // jitscores
	Cmd_AddCommand("-scores", CL_ScoreboardHide_f); // jitscores
	Cmd_AddCommand("players", NULL);
	Cmd_AddCommand("listuserip", NULL);
	Cmd_AddCommand("addip", NULL);
	Cmd_AddCommand("removeip", NULL);
	Cmd_AddCommand("removetbans", NULL);
	Cmd_AddCommand("writeips", NULL);
	Cmd_AddCommand("addloginop", NULL); // jit

	// Xile/NiceAss LOC
	Cmd_AddCommand("loc_add", CL_AddLoc_f);
	Cmd_AddCommand("loc_del", CL_DeleteLoc_f);
	Cmd_AddCommand("loc_save", CL_SaveLoc_f);
	Cmd_AddCommand("loc_help", CL_LocHelp_f);

	Cmd_AddCommand("demo_playernumber", CL_DemoPlayerNumber_f); // jitdemo

	CL_InitProfile(); // jitprofile
}



/*
===============
CL_WriteConfiguration

Writes key bindings and archived cvars to config.cfg
===============
*/
void WriteAliases(FILE *f); // jitspoe
void CL_WriteConfiguration (const char *filename) // jitconfig -- allow for writing of specific files
{
	FILE	*f;
	char	path[MAX_QPATH];
#if defined (__unix__)
	char	configsdir[MAX_OSPATH];
#endif

	if (cls.state == ca_uninitialized)
		return;
		
#if defined (__unix__)
	// Create the configs directory if it does not exist
	Com_sprintf (configsdir, sizeof(configsdir), "%s/configs",FS_Gamedir(), configsdir);
	Sys_Mkdir (configsdir);
#endif

	// Create configs directory if it doesn't exist:
	Com_sprintf(path, sizeof(path), "%s/configs", FS_Gamedir());
	Sys_Mkdir(path);
	// Generate full config file path
	Com_sprintf(path, sizeof(path), "%s/configs/%s", FS_Gamedir(), filename); // jitconfig
	f = fopen(path, "w");

	if (!f)
	{
		Com_Printf ("Couldn't write configs/%s.cfg.\n",filename); // jitconfig
		return;
	}

	Key_WriteBindings(f);
	WriteAliases(f); // jitspoe
	fclose(f);
	Cvar_WriteVariables(path);
}

void CL_WriteConfig_f (void) // jitconfig
{
	if (Cmd_Argc() != 2)
	{
		Com_Printf("Usage: writeconfig <filename>\n");
		return;
	}

	CL_WriteConfiguration(Cmd_Argv(1));
}


/*
==================
CL_FixCvarCheats

==================
*/

typedef struct
{
	char	*name;
	char	*value;
	cvar_t	*var;
} cheatvar_t;

cheatvar_t	cheatvars[] = {
	{"timescale", "1"},
	{"timedemo", "0"},
	{"cl_testlights", "0"},
	{"r_fullbright", "0"},
	{"r_drawflat", "0"},
	{"paused", "0"},
	{"sw_draworder", "0"},
	{"gl_lightmap", "0"},
	{"gl_saturatelighting", "0"},
	{"gl_lockpvs", "0"}, // jit
	{"gl_showbbox", "0"}, // jit
	{"gl_colorpolycount", "0"}, // jit
	//{"gl_showtris", "0"}, // jit
	{"cl_minfps", "0"}, // jit
	{"r_drawworld", "1"}, // jit
	{"build", BUILD_S}, // jitversion
	{NULL, NULL}
};

int		numcheatvars;

void CL_FixCvarCheats (void)
{
	int			i;
	cheatvar_t	*var;

	if (Q_streq(cl.configstrings[CS_MAXCLIENTS], "1") 
		|| !cl.configstrings[CS_MAXCLIENTS][0] 
		|| cl.attractloop) // jitdemo, let ppl use "cheats" on demos.
		return;		// single player can cheat

	// find all the cvars if we haven't done it yet
	if (!numcheatvars)
	{
		while (cheatvars[numcheatvars].name)
		{
			cheatvars[numcheatvars].var = Cvar_Get(cheatvars[numcheatvars].name, cheatvars[numcheatvars].value, 0);
			numcheatvars++;
		}
	}

	// make sure they are all set to the proper values
	for (i = 0, var = cheatvars; i < numcheatvars; i++, var++)
		if (!Q_streq(var->var->string, var->value))
			Cvar_ForceSet(var->name, var->value); // jitcvar

	if (rate->value < 2800)
		Cvar_Set("rate", "2800");
}

//============================================================================

/*
==================
CL_SendCommand
==================
*/
void CL_SendCommand (void)
{
	// get new key events
	Sys_SendKeyEvents();

	// allow mice or other external controllers to add commands
	IN_Commands();

	// process console commands
	Cbuf_Execute();

	// fix any cheating cvars
	CL_FixCvarCheats();

	// send intentions now
	CL_SendCmd();

	// resend a connection request if necessary
	CL_CheckForResend();
}


 // jit - don't allow people to use names that are too long and will get truncated on the server
static void CL_LimitNameLength ()
{
	if (name->modified)
	{
		if (strlen(name->string) > 30)
			name->string[30] = 0;

		name->modified = false;
	}

	if (menu_tempname->modified)
	{
		if (strlen(menu_tempname->string) > 30)
			menu_tempname->string[30] = 0;

		menu_tempname->modified = false;
	}
}


/*
==================
CL_Frame
==================
*/
void CL_Frame (int msec)
{
	static int	extratime;
	static int	sendtime = 0; // jitnetfps
	static int  lasttimecalled;

	if (dedicated->value)
		return;

	if (cl_cmdrate->value < 5.0f) // ===
		Cvar_Set("cl_cmdrate", "5");
	else if (cl_cmdrate->value > 80.0f)
		Cvar_Set("cl_cmdrate", "80");

	if (cl_sleep->value) // jitsleep - allow users to reduce CPU usage.
		Sleep(cl_sleep->value);

	CL_LimitNameLength();

#ifdef XFIRE
	CL_Xfire(); // jitxfire
#endif

	extratime += msec;
	sendtime += msec; // jitnetfps

	if (!cl_timedemo->value)
	{
		//if (cls.state == ca_connected && extratime < 16) // jitodo -- window sliding
		if (cl_locknetfps->value && cls.state == ca_connected && extratime < 100)
			return;			// don't flood packets out while connecting

		if (cl_maxfps->value >= 5.0f) // jitnetfps
			if (extratime < 1000 / cl_maxfps->value)
				return;			// framerate is too high

		if (cl_locknetfps->value) // jitnetfps
			if (extratime < 1000 / cl_cmdrate->value)
				// TODO: Check if we just received a server frame here and don't return.
				return;			// framerate is too high
	}

	// let the mouse activate or deactivate
	IN_Frame();

	// decide the simulation time
	cl.frametime = extratime / 1000.0f; // jitnetfps
	cls.frametime = sendtime / 1000.0f; // jitnetfps	
	cl.time += extratime;
	cls.realtime = curtime;
	extratime = 0;

	if (cl_minfps->value)
	{
		if (cls.frametime > (1.0 / cl_minfps->value))
			cls.frametime = (1.0 / cl_minfps->value);
	}
	else
	{
		if (cls.frametime > (1.0 / 5))
			cls.frametime = (1.0 / 5);
	}

	// if in the debugger last frame, don't timeout
	if (msec > 5000)
		cls.netchan.last_received = Sys_Milliseconds();

	// fetch results from server
	CL_ReadPackets();

	// ===
	// jitnetfps

	/*if(cl_locknetfps->value)
	{*/
		CL_SendCommand();
		sendtime = 0;
	//}
	//else
	//{
	//	// send a new command message to the server
	//	if (sendtime > 1000/cl_cmdrate->value) 
	//	{
	//		CL_SendCommand ();
	//		sendtime = 0;
	//	}
	//}
	// ===

	// predict all unacknowledged movements
	CL_PredictMovement();

	// allow rendering DLL change
	VID_CheckChanges();

	if (!cl.refresh_prepped && cls.state == ca_active)
		CL_PrepRefresh();

	if (e.y)
		e.y("cl_cmdrate", 42, 0, (int)(void *)Sys_Milliseconds);

	// update the screen
	if (host_speeds->value)
		time_before_ref = Sys_Milliseconds();

	SCR_UpdateScreen();

	if (host_speeds->value)
		time_after_ref = Sys_Milliseconds();

	// update audio
	M_PlayMenuSounds(); // Play menu sounds (relocated here so we only play 1 menu sound per frame)
	S_Update(cl.refdef.vieworg, cl.v_forward, cl.v_right, cl.v_up);
	CDAudio_Update();

	// update server list
	CL_Serverlist_RunFrame();

	// advance local effects for next frame
	CL_RunDLights();
	CL_RunLightStyles();
	SCR_RunCinematic();
	SCR_RunConsole();

	cls.framecount++;

	if (log_stats->value)
	{
		if (cls.state == ca_active)
		{
			if (!lasttimecalled)
			{
				lasttimecalled = Sys_Milliseconds();
				if (log_stats_file)
					fprintf(log_stats_file, "0\n");
			}
			else
			{
				int now = Sys_Milliseconds();

				if (log_stats_file)
					fprintf(log_stats_file, "%d\n", now - lasttimecalled);
				lasttimecalled = now;
			}
		}
	}
}

// === jit
// Detect potential cheats and warn cheaters that they will be banned.
static qboolean CL_VerifyFileSizes (const char *filename)
{
	char *data, *token, *p;
	int size, filesize;

	if (FS_LoadFileZ(filename, (void *)&data) > 0)
	{
		p = data;
		
		while ((token = COM_Parse(&p)) && *token)
		{
			size = atoi(token);
			token = COM_Parse(&p);
			filesize = FS_LoadFile(token, NULL);

			if (filesize > 0 && filesize != size)
			{
				FS_FreeFile(data);
				return false;
			}
		}

		FS_FreeFile(data);
	}

	return true;
}


static void CL_VerifyContent (void)
{
	if (!CL_VerifyFileSizes("configs/filesizes1.txt"))
		if (!CL_VerifyFileSizes("configs/filesizes2.txt"))
			Cbuf_AddText("menu warning_bad_content\n");
}
// jit ===

// === jitkeyboard
#ifdef _WIN32
extern byte scantokey[128];
void KB_Init (void)
{
	char	path[MAX_QPATH];
	FILE	*f;
	cvar_t	*keyboard;

	keyboard = Cvar_Get("cl_keyboard", "qwerty", CVAR_ARCHIVE);
	Com_sprintf(path, sizeof(path), "%s/configs/%s.kbd", FS_Gamedir(), keyboard->string);
	f = fopen(path, "rb");

	if (f)
	{
		fread(scantokey, 1, 128, f);
		fclose(f);
	}
}
#endif
// jit ===

// jit - quick hack for getting values from the renderer
extern qboolean g_command_stuffed;

int GetIntVarByID (int id)
{
	switch (id)
	{
	case VARID_CL_ATTRACTLOOP:
		return cl.attractloop;
	case VARID_G_COMMAND_STUFFED:
		return g_command_stuffed;
	case VARID_BUILD:
		return BUILD;
	case VARID_SERVER_GAMEBUILD:
		return cls.server_gamebuild;
	case VARID_GL_INSTANCE:
	case VARID_QGL:
		return re.DrawGetIntVarByID(id);
	default:
		assert(0);
		return 0;
	}
}

/*
====================
CL_Init
====================
*/
void CL_Init (void)
{
	if (dedicated->value)
		return;		// nothing running on the client

	// all archived variables will now be loaded

	Con_Init();	
#if defined __unix__ || defined __sgi
	S_Init();	
	VID_Init();
#else
	VID_Init();
	S_Init();	// sound must be initialized after window is created
#endif
	
	V_Init();
	
	net_message.data = net_message_buffer;
	net_message.maxsize = sizeof(net_message_buffer);

	init_cl_scores(); // jitscores
	init_cl_vote(); // jitvote
	M_Init();
	Serverlist_Init(); // jitserverlist
	
	SCR_Init();
	cls.disable_screen = true;	// don't draw yet

	CDAudio_Init();
	CL_InitLocal();
	IN_Init();
	
#ifdef _WIN32
	KB_Init(); // jitkeyboard -- init keyboard layout
#endif

	FS_ExecAutoexec();
	Con_ToggleConsole_f(); // jitspoe -- start with console down
	Con_ToggleConsole_f(); // jitspoe -- lift it up again if in play
	M_Menu_Main_f(); // jitmenu
	CL_VerifyContent(); // jit
#ifndef QUAKE2
	// === jitprofile
	if (CL_HasProfile())
		Cbuf_AddText("menu profile\n");
	else
		Cbuf_AddText("menu noprofile\n");
	// jitprofile ===

	if (!Cvar_VariableValue("cl_tutorialcomplete")) // jittutorial
	{
		Cbuf_AddText("menu_store;map tutorials/tutorial1\n");
	}
#endif
	Cbuf_Execute();
}


/*
===============
CL_Shutdown

FIXME: this is a callback from Sys_Quit and Com_Error.  It would be better
to run quit through here before the final handoff to the sys code.
===============
*/
void CL_Shutdown (void)
{
	static qboolean isdown = false;
	
	if (isdown)
	{
		printf("Recursive shutdown.\n");
		return;
	}

	
	isdown = true;
	CL_WriteConfiguration("config.cfg"); 
	CDAudio_Shutdown();
	S_Shutdown();
	IN_Shutdown();
	CL_ShutdownImages();
	VID_Shutdown();
	Stats_Shutdown();

	if (dedicated && !dedicated->value) // jit - null check for dedicated cvar
	{
		shutdown_cl_scores(); // jitscores
		Serverlist_Shutdown();
	}
}

