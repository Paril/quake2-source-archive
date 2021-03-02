/*============================================================================

    This file is part of Lithium II Mod for Quake II
    Copyright (C) 1997, 1998, 1999, 2010 Matthew A. Ayres

    Lithium II Mod is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Lithium II Mod is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Lithium II Mod.  If not, see <http://www.gnu.org/licenses/>.

    Quake II is a trademark of Id Software, Inc.

    Code by Matt "WhiteFang" Ayres, matt@lithium.com

============================================================================*/

#include "g_local.h"
#include "net.h"

#define DELIM "\x001"

#define LMASTER_VERSION 8

#define RETRY_DELAY 15
#define RETRY_WAIT 5

int net_init = 0;
int net_talk = 0;
int net_sock = -1;
int net_addr = -1;
int net_port = 0;
int net_time = 0;

int net_trak = 0;
int net_saveinfo = 0;
int net_lhelp = 0;

float net_trytime;

lvar_t *use_lmaster;

lvar_t *lmaster;
lvar_t *lmaster_port;

cvar_t *hostname;
cvar_t *port;

qboolean redir = false;
char redir_addr[BUF_LEN];
char redir_port[BUF_LEN];

lclient_t *get_lclient(int id) {
	int i;
	edict_t *ent;

	for(i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if(!ent->inuse)
			continue;
		if(id == ent->lclient->id)
			return ent->lclient;
	}

	return NULL;
}

void lmaster_lookup(void) {
	if(!net_init)
		return;

	if(redir) {
		net_addr = Net_Lookup(redir_addr);
		net_port = atoi(redir_port);

		if(net_addr != -1)
			return;

		gi.dprintf("*** Bad lmaster redirection\n");
	}

	net_addr = Net_Lookup(lmaster->string);
	net_port = lmaster_port->value;
	if(net_addr == -1)
		gi.dprintf("*** Bad lmaster hostname\n");
}

void LNet_InitGame(void) {
	use_lmaster = lvar("use_lmaster", "1", "^", VAR_NONE);
	gi.cvar("use_lmaster", "", CVAR_NOSET);

#ifdef _DEBUG
	lmaster = lvar("lmaster", "lmaster.lithium.com", "str", VAR_NONE);
	lmaster_port = lvar("lmaster_port", "28000", "#####", VAR_NONE);
//	lmaster_port = lvar("lmaster_port", "28001", "#####", VAR_NONE);
#else
	lmaster = lvar("lmaster", "lmaster.lithium.com", "str", VAR_NONE);
	lmaster_port = lvar("lmaster_port", "28000", "#####", VAR_NONE);
#endif

	lmaster->func = lmaster_lookup;
	lmaster_port->func = lmaster_lookup;

	hostname = gi.cvar("hostname", "", 0);
	port = gi.cvar("port", "", 0);

	if(use_lmaster->value) {
		net_init = Net_Init();
		if(net_init)
			lmaster_lookup();
		else
			gi.dprintf("*** Net init failed\n");
	}
}

void LNet_InitLevel(void) {
	net_trytime = 1;

	if(!net_talk)
		return;

	Net_Sendf(net_sock, "gamemap" DELIM "%s", level.mapname);
}

void LNet_ExitLevel(void) {
	if(!net_talk)
		return;
	Net_Send(net_sock, "exit");
}

void LNet_Close(void) {
	if(net_sock != -1)
		Net_Close(net_sock);
	net_sock = -1;
	net_talk = 0;
}

void LNet_Shutdown(void) {
	if(!net_init)
		return;
	if(net_sock != -1) {
		Net_Send(net_sock, "shutdown");
		LNet_Close();
	}
	Net_Exit();
}

void LNet_Recv(char *buf);

void LNet_RunFrame(void) {
	char buf[BUF_LEN];
	int len;

	if(!net_init)
		return;

	if(!net_talk) {
		if(net_sock == -1) {
			if(level.time < net_trytime)
				return;

			if(net_addr == -1)
				return;

			net_sock = Net_Connect(net_addr, net_port);
		}
		
		if(Net_IsConnect(net_sock)) {
			gi.dprintf("*** Connected to lmaster server (%d.%d.%d.%d:%d)\n",
				net_addr & 0xFF, (net_addr & 0xFF00) >> 8, (net_addr & 0xFF0000) >> 16, (net_addr & 0xFF000000) >> 24, net_port);
			net_talk = 1;
			net_time = level.time;
		}

		else if(level.time > net_trytime + RETRY_WAIT) {
#ifdef DEBUG
			static qboolean first_fail = true;
			if(first_fail)
				gi.dprintf("*** Failed connecting to lmaster server\n");
			first_fail = false;
#endif
			LNet_Close();
			net_trytime = level.time + RETRY_DELAY;
		}

		return;
	}

	while(1) {
		len = Net_Recv(net_sock, buf, sizeof(buf));
		if(!len)
			break;
		if(len == -1) {
			gi.dprintf("*** Lost connect to lmaster server\n");
			LNet_Close();
			net_trytime = level.time + RETRY_DELAY;
			return;
		}
		LNet_Recv(buf);
	}
}

void LNet_Login(edict_t *ent) {
	if(!net_talk)
		return;
	Net_Sendf(net_sock, "login" DELIM "%d" DELIM "%s" DELIM "%d.%d.%d.%d" DELIM "%d", ent->lclient->id, ent->client->pers.netname,
		ent->lclient->ip[0], ent->lclient->ip[1], ent->lclient->ip[2], ent->lclient->ip[3], ent->lclient->port);
}

void LNet_Send(char *buf) {
	if(!net_talk)
		return;
	Net_Send(net_sock, buf);
}

void LNet_SV(char *buf) {
	if(!net_talk)
		return;
	Net_Sendf(net_sock, "cmd" DELIM "-1" DELIM "%s", buf);
}

void LNet_Recv(char *buf) {
	char cmd[BUF_LEN];
	char param[BUF_LEN];
	int i;
	edict_t *ent;

//	gi.dprintf("*** Master server message: '%s'\n", buf);

	Split(buf, cmd, sizeof(cmd), param, sizeof(param));

	if(!strcmp(buf, "ver bad")) {
		gi.dprintf("*** Protocol version different on lmaster server, disconnecting\n", buf);
		gi.cvar_set("use_lmaster", "0");
		LNet_Close();
		net_init = 0;
	}

	else if(!strcmp(buf, "ver ok")) {
		Net_Sendf(net_sock, "hostname" DELIM "%s", hostname->string);
		Net_Sendf(net_sock, "ip" DELIM "%s", gi.cvar("ip", "", 0)->string);
		Net_Sendf(net_sock, "port" DELIM "%s", port->string);
		Net_Sendf(net_sock, "lver" DELIM "%s", lithium_version);
		Net_Sendf(net_sock, "map" DELIM "%s", level.mapname);

		for(i = 0; i < game.maxclients; i++) {
			ent = g_edicts + 1 + i;
			if(!ent->inuse)
				continue;
			LNet_Login(ent);
		}
	}

	else if(!strcmp(buf, "init"))
		Net_Sendf(net_sock, "ver" DELIM "%d", LMASTER_VERSION);

	else if(!strcmp(buf, "ping"))
		Net_Send(net_sock, "pong");

	else if(!strcmp(cmd, "join")) {
		char idstr[BUF_LEN], chanstr[BUF_LEN];
		lclient_t *lclient;
		Split(param, idstr, sizeof(idstr), chanstr, sizeof(chanstr));
		lclient = get_lclient(atoi(idstr));
		if(lclient)
			lclient->chan_id = atoi(chanstr);
	}

	else if(!strcmp(cmd, "chan")) {
		char chanstr[BUF_LEN], msg[BUF_LEN];
		int chan_id;

		Split(param, chanstr, sizeof(chanstr), msg, sizeof(msg));

		chan_id = atoi(chanstr);

		for(i = 0; i < game.maxclients; i++) {
			ent = g_edicts + 1 + i;
			if(!ent->inuse)
				continue;
			if(chan_id == ent->lclient->chan_id)
				gi.cprintf(ent, 4, msg);
		}
	}

	else if(!strcmp(cmd, "discon")) {
		char name[BUF_LEN], chan[BUF_LEN];

		Split(param, name, sizeof(name), chan, sizeof(chan));

		for(i = 0; i < game.maxclients; i++) {
			ent = g_edicts + 1 + i;
			if(!ent->inuse)
				continue;
			if(ent->lclient->chan_id && !stricmp(chan, ent->lclient->chan))
				gi.cprintf(ent, 4, "*** %s has disconnected\n", name);
		}
	}

	else if(!strcmp(cmd, "chat")) {
		char chanstr[BUF_LEN], name[BUF_LEN], msg[BUF_LEN];
		int chan_id;

		Split(param, chanstr, sizeof(chanstr), param, sizeof(param));
		Split(param, name, sizeof(name), msg, sizeof(msg));

		chan_id = atoi(chanstr);
		
		for(i = 0; i < game.maxclients; i++) {
			ent = g_edicts + 1 + i;
			if(!ent->inuse)
				continue;
			if(chan_id == ent->lclient->chan_id)
				gi.cprintf(ent, PRINT_CHAT, "<%s> %s\n", name, msg);
		}
	}

	else if(!strcmp(cmd, "msg")) {
		char idstr[BUF_LEN], name[BUF_LEN], msg[BUF_LEN];
		lclient_t *lclient;
		int id;

		Split(param, idstr, sizeof(idstr), param, sizeof(param));
		Split(param, name, sizeof(name), msg, sizeof(msg));

		ent = NULL;
		id = atoi(idstr);

		lclient = get_lclient(id);
		if(lclient)
			ent = lclient->ent;

		if(id == -1 || lclient)
			gi.cprintf(ent, PRINT_CHAT, "*%s* %s\n", name, msg);
	}

	else if(!strcmp(cmd, "ackmsg")) {
		char idstr[BUF_LEN], name[BUF_LEN], msg[BUF_LEN];
		lclient_t *lclient;
		int id;

		Split(param, idstr, sizeof(idstr), param, sizeof(param));
		Split(param, name, sizeof(name), msg, sizeof(msg));

		ent = NULL;
		id = atoi(idstr);

		lclient = get_lclient(atoi(idstr));
		if(lclient)
			ent = lclient->ent;
			
		if(id == -1 || lclient)
			gi.cprintf(ent, 3, "-> *%s* %s\n", name, msg);
	}

	else if(!strcmp(cmd, "climsg")) {
		char idstr[BUF_LEN], msg[BUF_LEN];
		lclient_t *lclient;
		int id;

		Split(param, idstr, sizeof(idstr), msg, sizeof(msg));

		ent = NULL;
		id = atoi(idstr);

		lclient = get_lclient(atoi(idstr));
		if(lclient)
			ent = lclient->ent;
		
		if(id == -1 || lclient)
			gi.cprintf(ent, 4, "%s", msg);
	}

	else if(!strcmp(cmd, "srvmsg"))
		gi.bprintf(4, "%s", param);
	
	else if(!strcmp(cmd, "conmsg"))
		gi.dprintf("%s", param);

	// broadcast clients only message
	else if(!strcmp(cmd, "bcomsg")) {
		for(i = 0; i < game.maxclients; i++) {
			ent = g_edicts + 1 + i;
			if(!ent->inuse)
				continue;
			gi.cprintf(ent, PRINT_HIGH, "%s", param);
		}
	}

	else if(!strcmp(cmd, "restart")) {
		int delay = atoi(param);
		if(!delay)
			delay = RETRY_DELAY;
		gi.dprintf("*** Lmaster server is restarting\n");
		LNet_Close();
		net_talk = 0;
		net_sock = -1;
		net_trytime = level.time + delay;
	}

	else if(!strcmp(cmd, "redir")) {
		Split(param, redir_addr, sizeof(redir_addr), redir_port, sizeof(redir_port));

		gi.dprintf("*** Lmaster server redirection\n");

		LNet_Close();
		net_trytime = level.time + 2;

		redir = true;
		lmaster_lookup();
		redir = false;
	}

	else if(!strcmp(cmd, "cvar")) {
		cvar_t *cvar;
		cvar = gi.cvar(param, "", 0);
		Net_Sendf(net_sock, "cvar" DELIM "%s" DELIM "%s", param, cvar->string);
	}

	else if(!strcmp(cmd, "trak")) {
		net_trak = atoi(param);
	}

	else if(!strcmp(cmd, "saveinfo")) {
		net_saveinfo = atoi(param);
	}

	else if(!strcmp(cmd, "lhelp")) {
		net_lhelp = atoi(param);
	}

	else if(!strcmp(cmd, "userinfo")) {
		lclient_t *lclient = get_lclient(atoi(param));
		if(lclient)
			Net_Sendf(net_sock, "userinfo" DELIM "%d" DELIM "%s", lclient->id, lclient->ent->client->pers.userinfo);
	}

	else if(!strcmp(cmd, "stat")) {
		lclient_t *lclient = get_lclient(atoi(param));
		if(lclient)
			LNet_ReturnStat(lclient->ent);
	}

	else if(!strcmp(cmd, "statall")) {
		edict_t *ent;
		for(i = 0; i < game.maxclients; i++) {
			ent = g_edicts + 1 + i;
			if(!ent->inuse)
				continue;
			LNet_ReturnStat(ent);
		}
	}

	else if(!strcmp(cmd, "clicmd")) {
		char idstr[BUF_LEN], msg[BUF_LEN];
		lclient_t *lclient;

		Split(param, idstr, sizeof(idstr), msg, sizeof(msg));

		lclient = get_lclient(atoi(idstr));
		if(lclient)
			stuffcmd(lclient->ent, msg);
	}

	else if(!strcmp(cmd, "srvcmd"))
		gi.AddCommandString(param);

	else if(!strcmp(cmd, "setad")) {
		char num[BUF_LEN], str[BUF_LEN];
		Split(param, num, sizeof(num), str, sizeof(str));
		Lithium_SetAd(atoi(num), str);
	}

	else
		gi.dprintf("*** Unknown lmaster server message: '%s'\n", buf);
}

void LNet_ReturnStat(edict_t *ent) {
	if(!net_talk)
		return;
	Net_Sendf(net_sock, "stat" DELIM "%d" DELIM "%d" DELIM "%d" DELIM "%d" DELIM "%d" DELIM "%d" DELIM "%d",
		ent->lclient->id, ent->client->pers.score, ent->place, ent->play_frames,
		ent->client->ping, ent->ping_total / ent->ping_count, ent->lithium_flags & LITHIUM_OBSERVER);
}

void LNet_ClientBegin(edict_t *ent) {
	ent->lclient->id = ent - g_edicts - 1;

	if(!net_talk)
		return;

	LNet_Login(ent);
}

void LNet_ClientDisconnect(edict_t *ent) {
	if(!net_talk)
		return;
	if(ent->client && net_saveinfo)
		Net_Sendf(net_sock, "userinfo" DELIM "%d" DELIM "%s", ent->lclient->id, ent->client->pers.userinfo);
	Net_Sendf(net_sock, "logout" DELIM "%d" DELIM "*", ent->lclient->id);
}

void LNet_ClientRename(edict_t *ent, char *name) {
	if(!net_talk)
		return;
	if(ent->lclient)
		Net_Sendf(net_sock, "rename" DELIM "%d" DELIM "%s", ent->lclient->id, name);
}

void LNet_Observe(edict_t *ent) {
	if(!net_talk || !net_trak)
		return;
	Net_Sendf(net_sock, "observe" DELIM "%d", ent->lclient->id);
}

void LNet_Playing(edict_t *ent) {
	if(!net_talk || !net_trak)
		return;
	Net_Sendf(net_sock, "playing" DELIM "%d", ent->lclient->id);
}

void LNet_Score(edict_t *attacker, edict_t *target, int mod, int score, int time) {
	if(!net_talk || !net_trak)
		return;
	if(!target->lclient)
		return;
	if(!attacker->lclient)
		return;
	Net_Sendf(net_sock, "score" DELIM "%d" DELIM "%d" DELIM "%d" DELIM "%d" DELIM "%d",
		attacker->lclient->id, target->lclient->id, mod, score, time);
}

void LNet_Rune(edict_t *ent, int rune) {
	if(!net_talk || !net_trak)
		return;
	Net_Sendf(net_sock, "rune" DELIM "%d" DELIM "%d", ent->lclient->id, rune);
}

void LNet_BotNotice(edict_t *ent) {
	if(!net_talk || !net_trak)
		return;
	Net_Sendf(net_sock, "zbot" DELIM "%d", ent->lclient->id);
}

void Cmd_Say_f(edict_t *ent, qboolean team, qboolean arg0);

qboolean LNet_ClientCommand(edict_t *ent) {
	char *cmd = gi.argv(0);

	if(!ent->lclient)
		return false;

	if(!net_talk) {
		if(cmd[0] == '.') {
			gi.cprintf(ent, 4, "*** Server not connected to lmaster\n");
			return true;
		}
		return false;
	}

	if(!Q_stricmp(cmd, ".help") && !net_lhelp) {
		gi.cprintf(ent, 4, 
			"Commands:\n"
			"  .channels\n"
			"  .info\n"
			"  .join <channel>\n"
			"  .leave\n"
			"  .msg <client> <message>\n"
			"  .reply <message>\n"
			"  .say <message>\n"
			"  .search <substring>\n"
			"  .who <channel>\n"
			"  .whois <client>\n"
			"  .whoon <server>\n"
			);
	}
	else if(!Q_stricmp(cmd, ".who")) {
		if(gi.argc() < 2) {
			if(ent->lclient->chan_id)
				Net_Sendf(net_sock, "who" DELIM "%d" DELIM "%s", ent->lclient->id, ent->lclient->chan);
			else
				gi.cprintf(ent, 4, "*** Need to specify a channel\n");
		}
		else
			Net_Sendf(net_sock, "who" DELIM "%d" DELIM "%s", ent->lclient->id, gi.argv(1));
	}
	else if(!Q_stricmp(cmd, ".whois") || !Q_stricmp(cmd, ".w")) {
		if(gi.argc() < 2) {
			gi.cprintf(ent, 4, "*** Need to specify a client name\n");
			return true;
		}
		Net_Sendf(net_sock, "whois" DELIM "%d" DELIM "%s", ent->lclient->id, gi.args());
	}
	else if(!Q_stricmp(cmd, ".join") || !Q_stricmp(cmd, ".j")) {
		if(gi.argc() < 2) {
			gi.cprintf(ent, 4, "*** Need to specify a channel name\n");
			return true;
		}
		strlcpy(ent->lclient->chan, gi.argv(1), sizeof(ent->lclient->chan));
		Net_Sendf(net_sock, "join" DELIM "%d" DELIM "%s", ent->lclient->id, gi.argv(1));
	}
	else if(!Q_stricmp(cmd, ".leave") || !Q_stricmp(cmd, ".l")) {
		if(ent->lclient->chan_id) {
			Net_Sendf(net_sock, "leave" DELIM "%d" DELIM "*", ent->lclient->id);
			ent->lclient->chan_id = 0;
			strlcpy(ent->lclient->chan, "", sizeof(ent->lclient->chan));
		}
		else
			gi.cprintf(ent, 4, "*** You are not in a channel\n");
	}
	else if(!Q_stricmp(cmd, ".say") || !Q_stricmp(cmd, "."))
		Net_Sendf(net_sock, "chat" DELIM "%d" DELIM "%s", ent->lclient->id, gi.args());
	else if(!Q_stricmp(cmd, ".local"))
		Cmd_Say_f(ent, false, false);
	else if(!Q_stricmp(cmd, ".msg") || !Q_stricmp(cmd, ".m")) {
		Net_Sendf(net_sock, "msg" DELIM "%d" DELIM "%s", ent->lclient->id, gi.args());
	}
	else if(cmd[0] == '.') {
		char buf[BUF_LEN];
		if(strlen(gi.args()))
			snprintf(buf, sizeof(buf), "%s %s", gi.argv(0), gi.args());
		else
			strlcpy(buf, gi.argv(0), sizeof(buf));
		Net_Sendf(net_sock, "cmd" DELIM "%d" DELIM "%s", ent->lclient->id, buf);
	}
	else if(ent->lclient->chan_id) {
		char buf[BUF_LEN];
		if(strlen(gi.args()))
			snprintf(buf, sizeof(buf), "%s %s", gi.argv(0), gi.args());
		else
			strlcpy(buf, gi.argv(0), sizeof(buf));
		Net_Sendf(net_sock, "chat" DELIM "%d" DELIM "%s", ent->lclient->id, buf);
	}
	else
		return false;
	return true;
}
