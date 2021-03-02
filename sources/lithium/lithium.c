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

float lithium_ver = 1.32;
int lithium_beta = 2;

char lithium_version[16];
char lithium_modname[48];
static char file_gamedir_buffer[256];

lvar_t *use_safety;
lvar_t *use_startobserver;
lvar_t *use_startchasecam;
lvar_t *use_suicidekick;
lvar_t *use_chatmuzzle;
lvar_t *use_chatkick;
lvar_t *use_zbotkick;
lvar_t *use_zbotlog;
lvar_t *use_hook;
lvar_t *use_vwep;
lvar_t *use_hitfactor;
lvar_t *use_highscores;
lvar_t *timestamp;

lvar_t *max_rate;

lvar_t *fast_respawn;
lvar_t *fast_minpbound;
lvar_t *fast_maxpbound;

lvar_t *fast_wep;

cvar_t *upgrade;
cvar_t *bounce;

lvar_t *dropweapammo;
lvar_t *safety_time;

lvar_t *gamedir;

lvar_t *intermission_time;
lvar_t *intermission_maxtime;
lvar_t *intermission_sound;
 
lvar_t *ofp_base;
lvar_t *ofp_perplayer;
lvar_t *ofp_maxframes;
lvar_t *ofp_minping;
lvar_t *ofp_maxping;

lvar_t *motd;
lvar_t *news;
lvar_t *news_time;
lvar_t *banlist;

lvar_t *def_bestweapon;
lvar_t *def_id;
lvar_t *def_hud;
lvar_t *def_scores;
lvar_t *def_boardshow;

cvar_t *first;

float qver;

void Lithium_InitGame(void) {

	if(!lithium_beta)
		snprintf(lithium_version, sizeof(lithium_version), "%1.2f", lithium_ver);
	else
		snprintf(lithium_version, sizeof(lithium_version), "%1.2f-beta%d", lithium_ver, lithium_beta);
	snprintf(lithium_modname, sizeof(lithium_modname), "Lithium II Mod v%s", lithium_version);

	sscanf(gi.cvar("version", 0, 0)->string, "%f", &qver);

	ofp_base = lvar("ofp_base", "20", "##", VAR_NONE);
	ofp_perplayer = lvar("ofp_perplayer", "0.75", "#.##", VAR_NONE);
	ofp_maxframes = lvar("ofp_maxframes", "30", "##", VAR_NONE);
	ofp_minping = lvar("ofp_minping", "30", "###", VAR_NONE);
	ofp_maxping = lvar("ofp_maxping", "500", "###", VAR_NONE);

	Rune_InitGame();
	Pack_InitGame();

	use_safety = lvar("use_safety", "1", "^", VAR_USE);
	use_startobserver = lvar("use_startobserver", "1", "^", VAR_USE);
	use_startchasecam = lvar("use_startchasecam", "0", "^", VAR_USE);

	use_suicidekick = lvar("use_suicidekick", "1", "^", VAR_USE);
	use_chatmuzzle = lvar("use_chatmuzzle", "1", "^", VAR_USE);
	use_chatkick = lvar("use_chatkick", "1", "^", VAR_USE);

	use_zbotkick = lvar("use_zbotkick", "1", "^", VAR_USE);
	use_zbotlog = lvar("use_zbotlog", "0", "^", VAR_USE);

	def_bestweapon = lvar("def_bestweapon", "0", "^", VAR_DEF);
	def_id = lvar("def_id", "0", "^", VAR_DEF);
	def_hud = lvar("def_hud", "2", "None:Normal:Lithium:Ammo", VAR_DEF);
	def_scores = lvar("def_scores", "1", "Old:By Frags:By FPH", VAR_DEF);
	def_boardshow = lvar("def_boardshow", "10", "##", VAR_DEF);

	use_hook = lvar("use_hook", "1", "^", VAR_USE);
	use_vwep = lvar("use_vwep", "0", "^", VAR_NONE);
	gi.cvar_set(use_vwep->cvar->name, "1"); // 3.15+ compliant
	use_hitfactor = lvar("use_hitfactor", "0", "^", VAR_USE);

	use_highscores = lvar("use_highscores", "0", "^", VAR_USE);

	upgrade = gi.cvar("upgrade", "0", 0);
	bounce = gi.cvar("bounce", "", 0);

	fast_respawn = lvar("fast_respawn", ".65", "#.##", VAR_OTHER);
	fast_minpbound = lvar("fast_minpbound", "4", "##", VAR_OTHER);
	fast_maxpbound = lvar("fast_maxpbound", "16", "##", VAR_OTHER);

	fast_wep = lvar("fast_wep", "2", "#", VAR_OTHER);

	dropweapammo = lvar("dropweapammo", "0.6", "#.##", VAR_OTHER);
	safety_time = lvar("safety_time", "4.0", "#.##", VAR_OTHER);

	timestamp = lvar("timestamp", "0", "^", VAR_OTHER);

	intermission_time = lvar("intermission_time", "8.5", "##", VAR_OTHER);
	intermission_maxtime = lvar("intermission_maxtime", "30", "##", VAR_OTHER);
	intermission_sound = lvar("intermission_sound", "world/xian1.wav", "wav", VAR_NONE);

	max_rate = lvar("max_rate", "8000", "#####", VAR_OTHER);

	motd = lvar("motd", "motd.txt", "str", VAR_NONE);
	news = lvar("news", "news.txt", "str", VAR_NONE);
	news_time = lvar("news_time", "10", "str", VAR_NONE);
	banlist = lvar("banlist", "admin.lst", "str", VAR_NONE);

	first = gi.cvar("first", "1", 0);

	NoCamp_InitGame();
	Admin_InitGame();
	GSLog_InitGame();
	Mapqueue_InitGame();
	Hook_InitGame();
	Vote_InitGame();

//	LNet_InitGame();

	// must be last for features var
	Var_InitGame();
}

void Lithium_Shutdown(void) {
	GSLog_Shutdown();
	Var_Shutdown();
	LNet_Shutdown();
}

int last_clients = 0;
extern lvar_t *mapqueue;
extern lvar_t *use_mapqueue;
void ExitLevel(void);
float empty_time = 0;
float end_time = 0;

char *colorize(char *str) {
	static char ret[128];
	char *c = ret;
	strlcpy(ret, str, sizeof(ret));
	while(*c)
		*c++ += 128;
	return ret;
}

void Lithium_InitLevel(void) {
	if(first->value == 1 && use_mapqueue->value && file_exist(file_gamedir(mapqueue->string))) {
		first->value = 2;
		level.changemap = Mapqueue_GetMapName();
		return;
	}
	if(first->value == 2)
		first->value = 0;

	gi.dprintf("Lithium II Mod v%s\n", lithium_version);
#ifdef GIT_HASH
	gi.dprintf("GIT hash " GIT_HASH "\n");
#endif
	gi.dprintf("Map: %s  Clients: %d  Mode: %s\n", level.mapname, last_clients, ctf->value ? "CTF" : "DM");

	if(use_highscores->value) {
		if(ctf->value) {
			gi.dprintf("Setting use_highscores 0 (ctf is on).\n");
			gi.cvar_set("use_highscores", "0");
		}
		else {
			if(fraglimit->value) {
				gi.dprintf("Setting fraglimit 0 (use_highscores is on).\n");
				gi.cvar_set("fraglimit", "0");
			}
			if(!timelimit->value) {
				gi.dprintf("Setting timelimit 15 (use_highscores is on).\n");
				gi.cvar_set("timelimit", "15");
			}
		}
	}

	gi.configstring(CS_RUNE1, "Resist Rune");
	gi.configstring(CS_RUNE2, "Strength Rune");
	gi.configstring(CS_RUNE3, "Haste Rune");
	gi.configstring(CS_RUNE4, "Regen Rune");
	gi.configstring(CS_RUNE5, "Vampire Rune");
	gi.configstring(CS_OBSERVING, "Observing.  FIRE to join.  0 for menu.");
	gi.configstring(CS_SAFETY, "Safety");
	gi.configstring(CS_HUDRUNE0, colorize("    None"));
	gi.configstring(CS_HUDRUNE1, colorize("  Resist"));
	gi.configstring(CS_HUDRUNE2, colorize("Strength"));
	gi.configstring(CS_HUDRUNE3, colorize("   Haste"));
	gi.configstring(CS_HUDRUNE4, colorize("   Regen"));
	gi.configstring(CS_HUDRUNE5, colorize(" Vampire"));

	gi.soundindex(intermission_sound->string);

	Highscores_Read();

	FragTrak_InitLevel();
	GSLog_InitLevel();
	Rune_InitLevel();
	Var_InitLevel();
	LNet_InitLevel();

	empty_time = 0;
	end_time = 0;
}

void Lithium_BeginIntermission(void) {
	edict_t *ent;
	int i, size;

	size = 200 + countclients() * 50;

	for(i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if(!ent->inuse)
			continue;
		if(size > ent->update_size)
			ent->update_size = size;
		ent->update_frame = level.framenum + 1;
		if (ent->menu)
			Menu_Destroy(ent);
	}

	if(strlen(intermission_sound->string))
		gi.sound(world, CHAN_AUTO | CHAN_RELIABLE, gi.soundindex(intermission_sound->string), 1, ATTN_NONE, 0);
}

void Lithium_FillUserinfo(edict_t *ent) {
	char buf[MAX_INFO_KEY];
	int flags = 0;

	if(!ent->inuse || !ent->client || !ent->lclient)
		return;

	if(!strlen(Info_ValueForKey(ent->client->pers.userinfo, "ip"))) {
		snprintf(buf, sizeof(buf), "%d.%d.%d.%d:%d", ent->lclient->ip[0], ent->lclient->ip[1], ent->lclient->ip[2], ent->lclient->ip[3], ent->lclient->port);
		Info_SetValueForKey(ent->client->pers.userinfo, "ip", buf);
	}

	if(ent->id_state)
		flags |= INFOFLAG_IDSTATE;
	if(ent->bestweap)
		flags |= INFOFLAG_BESTWEAP;
	if(ent->client->chase_target)
		flags |= INFOFLAG_CHASING;
	if(ent->lithium_flags & LITHIUM_CHASELEADER)
		flags |= INFOFLAG_CHASELEADER;
	snprintf(buf, sizeof(buf), "%d", flags);
	Info_SetValueForKey(ent->client->pers.userinfo, "flags", buf);
	snprintf(buf, sizeof(buf), "%d", ent->hud);
	Info_SetValueForKey(ent->client->pers.userinfo, "hud", buf);
	snprintf(buf, sizeof(buf), "%d", ent->board);
	Info_SetValueForKey(ent->client->pers.userinfo, "board", buf);
	snprintf(buf, sizeof(buf), "%d", ent->lclient->board_show);
	Info_SetValueForKey(ent->client->pers.userinfo, "bshow", buf);
	snprintf(buf, sizeof(buf), "%d", ent->lclient->chan_id);
	Info_SetValueForKey(ent->client->pers.userinfo, "chan_id", buf);
	Info_SetValueForKey(ent->client->pers.userinfo, "channel", ent->lclient->chan);
}

qboolean Lithium_EndDMLevel(void) {
	edict_t *ent;

	if(use_highscores->value)
		Highscores_Update();

	FragTrak_Save();

	ent = Mapqueue_EndDMLevel();
	if(ent) {
		BeginIntermission(ent);
		return true;
	}

	return false;
}

void Lithium_ExitLevel(void) {
	int i;
	edict_t *ent;

	last_clients = 0;

	for(i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if(!ent->inuse)
			continue;
		Lithium_FillUserinfo(ent);
		last_clients++;
	}

	GSLog_ExitLevel();
	LNet_ExitLevel();

	if(upgrade->value)
		Lithium_DoUpgrade();
}

char *GetNews(void);

void News_RunFrame(void) {
	static float news_check_time = 0;
	static char oldnews[1024] = "";
	char *news;

	if(level.time > news_check_time) {
		news = GetNews();
		if(stricmp(news, oldnews)) {
			int i;
			edict_t *ent;

			for(i = 0; i < game.maxclients; i++) {
				ent = g_edicts + 1 + i;
				if(!ent->inuse)
					continue;
				Lithium_LayoutOn(ent, LAYOUT_NEWS);
				ent->news_time = level.time + news_time->value;
			}

			strlcpy(oldnews, news, sizeof(oldnews));
		}
		news_check_time = level.time + 30;
	}

	if(timestamp->value && (level.framenum - 5) % ((int)timestamp->value * 600) == 0) {
		time_t t;
		time(&t);
		gi.dprintf("[TIMESTAMP] %s", ctime(&t));
	}
}

void Lithium_RunFrame(void) {

	if(first->value == 2) {
		char buf[64];
		snprintf(buf, sizeof(buf), "gamemap %s\n", level.changemap);
		gi.AddCommandString(buf);
		return;
	}

	Rune_RunFrame();
	News_RunFrame();
	Var_RunFrame();
	Vote_RunFrame();
	LNet_RunFrame();
}

void IP_Scan(char *ipstr, int ip[4]) {
	char *c, buf[64];
	int scan;

	strlcpy(buf, ipstr, sizeof(buf));
	buf[sizeof(buf) - 1] = 0;

	// turn *'s into -1's
	c = strchr(buf, '*');
	while(c) {
		memmove(c + 1, c, c - buf);
		*c++ = '-';
		*c = '1';
		c = strchr(buf, '*');
	}

	scan = sscanf(buf, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

	if(scan < 4) ip[3] = -1;
	if(scan < 3) ip[2] = -1;
	if(scan < 2) ip[1] = -1;
	if(scan < 1) ip[0] = -1;
}

qboolean IP_Match(int ip1[4], int ip2[4]) {
	return
		(ip1[0] == ip2[0] || ip1[0] == -1 || ip2[0] == -1) &&
		(ip1[1] == ip2[1] || ip1[1] == -1 || ip2[1] == -1) &&
		(ip1[2] == ip2[2] || ip1[2] == -1 || ip2[2] == -1) &&
		(ip1[3] == ip2[3] || ip1[3] == -1 || ip2[3] == -1);
}

// IP banning
qboolean Lithium_ClientConnect(edict_t *ent, char *userinfo) {
	char *c, *ipstr;
	int i, ip[4], cip[4], ban[4], port, max, count;
	FILE *file;
	char buf[256];
	edict_t *cl_ent;
	qboolean allow = true;

	file = fopen(file_gamedir(banlist->string), "rt");
	if(file) {
		ipstr = Info_ValueForKey(userinfo, "ip");
		sscanf(ipstr, "%d.%d.%d.%d:%d", &ip[0], &ip[1], &ip[2], &ip[3], &port);

		while(fgets(buf, 256, file)) {
			if(buf[0] == '/' && buf[1] == '/')
				continue;
			if(buf[0] == '#')
				continue;
			if(!(strchr(buf, '.') || strchr(buf, '*')))
				continue;

			c = strchr(buf, ' ');
			if(!c)
				c = strchr(buf, '\t');
			if(c) {
				max = atoi(c);
				*c = 0;
			}
			else
				max = 0;

			IP_Scan(buf, ban);
			
			// if client IP doesn't match ban, move on
			if(!IP_Match(ip, ban))
				continue;

			allow = false;

			if(max) {
				count = 0;
				for(i = 0; i < game.maxclients; i++) {
					cl_ent = g_edicts + 1 + i;
					if(!cl_ent->client->pers.connected || cl_ent == ent)
						continue;
					ipstr = Info_ValueForKey(cl_ent->client->pers.userinfo, "ip");
					sscanf(ipstr, "%d.%d.%d.%d:%d", &cip[0], &cip[1], &cip[2], &cip[3], &port);
					if(IP_Match(cip, ban))
						count++;
				}
				if(count < max) {
					fclose(file);
					return true;
				}
			}
		}
		fclose(file);
	}

	ent->lclient = NULL;

	return allow;
}

void Lithium_ClientBegin(edict_t *ent) {
	char *info;
	int flags;

	if(strlen(bounce->string)) {
		char cmd[80];
		snprintf(cmd, sizeof(cmd), "connect %s\n", bounce->string);
		stuffcmd(ent, cmd);
		gi.dprintf("%s bounced to %s\n", ent->client->pers.netname, bounce->string);
	}

	ent->lclient = (lclient_t *)gi.TagMalloc(sizeof(lclient_t), TAG_LEVEL);
	memset(ent->lclient, 0, sizeof(lclient_t));
	ent->lclient->ent = ent;
	ent->lclient->ping_last = level.time;
	ent->lclient->board_show = def_boardshow->value;

	sscanf(Info_ValueForKey(ent->client->pers.userinfo, "ip"), "%d.%d.%d.%d:%d",
		&ent->lclient->ip[0], &ent->lclient->ip[1], &ent->lclient->ip[2], &ent->lclient->ip[3], &ent->lclient->port);

	ent->lithium_flags = 0;
	ent->layout = 0;
	ent->motd_time = level.time + 8.0;
	ent->news_time = 0;
 	ent->place = 0;
	ent->ping_total = 0;
	ent->ping_count = 1;
	ent->safety_warns = 0;
	ent->suicides = 0;
	ent->play_frames = 0;
	ent->sel = 0;
	ent->hud = def_hud->value;
	ent->board = def_scores->value;
	ent->bestweap = def_bestweapon->value;
	ent->update_frame = 0;
	ent->update_size = 0;
	ent->update_other = 0;
	ent->rune = 0;
	ent->menu = NULL;
	ent->id_state = def_id->value;
	ent->id_ent = NULL;
	ent->pvar = NULL;
	ent->admin = 0;
	ent->client->last_hook_time = 0;
	ent->last_sound_time = 0;
	if (ctf->value)
		ent->client->resp.ctf_team = CTF_NOTEAM;

	ent->lithium_flags |= LITHIUM_STATUSBAR;

//	if(ctf->value && !ent->client->resp.ctf_team)
//		Lithium_CTFMenu(ent);

	Lithium_LayoutOn(ent, LAYOUT_MOTD);

	pvar(ent, "id_state", (void *)&ent->id_state, "^");
	pvar(ent, "hud", (void *)&ent->hud, "None:Normal:Lithium:Ammo");
	pvar(ent, "bestweap", (void *)&ent->bestweap, "^");
	pvar(ent, "board", (void *)&ent->board, "Old:By Frags:By FPH");
	pvar(ent, "board_show", (void *)&ent->lclient->board_show, "##");
	pvar(ent, "admin_code", (void *)&ent->admin_code, "#####");

	ent->lclient->cam_freelook = 0;
	ent->lclient->cam_distance = 40;
	ent->lclient->cam_height = 40;
	ent->lclient->cam_jump = 16;
	ent->lclient->cam_maxpitch = 56;

	pvar(ent, "cam_freelook", (void *)&ent->lclient->cam_freelook, "^");
	pvar(ent, "cam_distance", (void *)&ent->lclient->cam_distance, "###");
	pvar(ent, "cam_height", (void *)&ent->lclient->cam_height, "###");
	pvar(ent, "cam_jump", (void *)&ent->lclient->cam_jump, "###");
	pvar(ent, "cam_maxpitch", (void *)&ent->lclient->cam_maxpitch, "###");

	//----------------------------
	// restore settings from last map
	info = Info_ValueForKey(ent->client->pers.userinfo, "flags");
	if(strlen(info)) {
		flags = atoi(info);
		ent->id_state = flags & INFOFLAG_IDSTATE ? 1 : 0;
		ent->bestweap = flags & INFOFLAG_BESTWEAP ? 1 : 0;
		if(flags & INFOFLAG_CHASING)
			ent->lithium_flags |= LITHIUM_GOCHASECAM;
		if(flags & INFOFLAG_CHASELEADER)
			ent->lithium_flags |= LITHIUM_CHASELEADER;
	}
	else {
		stuffcmd(ent, "alias +scores scores\n");
		stuffcmd(ent, "alias -scores noscores\n");

		stuffcmd(ent, "alias +highscores hscores\n");
		stuffcmd(ent, "alias -highscores nohscores\n");

		stuffcmd(ent, "alias +hook hook\n");
		stuffcmd(ent, "alias -hook unhook\n");
	}

	info = Info_ValueForKey(ent->client->pers.userinfo, "hud");
	if(strlen(info))
		ent->hud = atoi(info);

	info = Info_ValueForKey(ent->client->pers.userinfo, "board");
	if(strlen(info))
		ent->board = atoi(info);

	info = Info_ValueForKey(ent->client->pers.userinfo, "bshow");
	if(strlen(info))
		ent->lclient->board_show = atoi(info);

	info = Info_ValueForKey(ent->client->pers.userinfo, "chan_id");
	if(strlen(info))
		ent->lclient->chan_id = atoi(info);
	else
		ent->lclient->chan_id = 0;

	info = Info_ValueForKey(ent->client->pers.userinfo, "channel");
	strlcpy(ent->lclient->chan, info, sizeof(ent->lclient->chan));
	ent->lclient->chan[sizeof(ent->lclient->chan) - 1] = 0;

	if(ent->id_state)
		Lithium_LayoutOn(ent, LAYOUT_ID);
	//----------------------------

	FragTrak_ClientBegin(ent);
	LNet_ClientBegin(ent);

	if(use_startobserver->value || use_startchasecam->value) {
		Lithium_Observer(ent, false);
		if(use_startchasecam->value)
			ent->lithium_flags |= LITHIUM_GOCHASECAM | LITHIUM_CHASELEADER;
	}
	else
		GSLog_ClientBegin(ent);
}

// ??WARNING: ent->client may be NULL
void Lithium_ClientDisconnect(edict_t *ent) {
	if(!ent->lclient)
		return;

	Lithium_FillUserinfo(ent);

	FragTrak_ClientDisconnect(ent);
	LNet_ClientDisconnect(ent);
	if(ent->lithium_flags & LITHIUM_PLAYING)
		GSLog_ClientDisconnect(ent);

	gi.TagFree(ent->lclient);
	ent->lclient = NULL;
}

qboolean Lithium_Observer(edict_t *ent, qboolean check) {
	if(check && ent->deadflag != DEAD_DEAD && ent->health < start_health->value) {
		gi.cprintf(ent, PRINT_HIGH, "You must be dead or have full health to become an observer.\n");
		return false;
	}

	if(ent->client->hook_out)
		return false;

	if (ctf->value && ent->solid != SOLID_NOT) {
		CTFDeadDropFlag(ent);
		CTFDeadDropTech(ent);
		ent->client->resp.ctf_team = CTF_NOTEAM;
	}

	gi.setmodel(ent, "");
	ent->movetype = MOVETYPE_NOCLIP;
	ent->client->ps.gunindex = 0;
	ent->solid = SOLID_NOT;

	///---------------
	// from MoveClientToIntermission
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

//	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	///---------------

	// clean up frame nums?
	ent->client->ps.gunframe = 0;
	ent->s.frame = 0;

	if(ent->lithium_flags & LITHIUM_PLAYING)
		GSLog_ClientDisconnect(ent);

	if(!(ent->lithium_flags & LITHIUM_OBSERVER))
		LNet_Observe(ent);

	ent->lithium_flags |= LITHIUM_OBSERVER;
	ent->lithium_flags &= ~LITHIUM_PLAYING;
	
	Lithium_LayoutOff(ent, LAYOUT_SCORES);

	ent->place = 0;

	ent->client->chase_target = NULL;

	Rune_Drop(ent);

	return true;
}

void NoAmmoWeaponChange(edict_t *ent);

void Lithium_PutClientInServer(edict_t *ent) {

	if(!ent->lclient)
		Lithium_ClientBegin(ent);

	if(ent->lithium_flags & LITHIUM_PLAYING)
		ent->motd_time -= 3.0;
	else if(ent->lithium_flags & LITHIUM_OBSERVER)
		;
	else {
		if(ctf->value) {
			if(ent->client->resp.ctf_team)
				ent->lithium_flags |= LITHIUM_PLAYING;
			else
				ent->lithium_flags |= LITHIUM_OBSERVER;
		}
		else {
			if(use_startobserver->value)
				ent->lithium_flags |= LITHIUM_OBSERVER;
			else
				ent->lithium_flags |= LITHIUM_PLAYING;
		}
	}

	Lithium_LayoutOff(ent, LAYOUT_SCORES);

	Lithium_LayoutOn(ent, LAYOUT_NEWS);
	ent->news_time = level.time + news_time->value;

	ent->rune = 0;
	ent->motd_start = 0.0;
	ent->mega_rot = 0;

	ent->max_health = ent->client->pers.max_health;

	if(ent->lithium_flags & LITHIUM_OBSERVER) {
		// make observer
		Lithium_Observer(ent, false);
		return;
	}

	NoCamp_PutClientInServer(ent);
	Var_PutClientInServer(ent);

	if(use_safety->value && safety_time->value)
		ent->lithium_flags |= LITHIUM_ATTACKWAIT;

	if(start_weapon->value == 1) ent->client->newweapon = FindItem("blaster");
	else if(start_weapon->value == 2) ent->client->newweapon = FindItem("shotgun");
	else if(start_weapon->value == 3) ent->client->newweapon = FindItem("super shotgun");
	else if(start_weapon->value == 4) ent->client->newweapon = FindItem("machinegun");
	else if(start_weapon->value == 5) ent->client->newweapon = FindItem("chaingun");
	else if(start_weapon->value == 6) ent->client->newweapon = FindItem("grenade launcher");
	else if(start_weapon->value == 7) ent->client->newweapon = FindItem("rocket launcher");
	else if(start_weapon->value == 8) ent->client->newweapon = FindItem("hyperblaster");
	else if(start_weapon->value == 9) ent->client->newweapon = FindItem("railgun");
	else if(start_weapon->value == 10) ent->client->newweapon = FindItem("bfg");
	else NoAmmoWeaponChange(ent);
	
	if(use_safety->value && safety_time->value)
		ent->safety_time = level.time + safety_time->value;
	else
		ent->safety_time = 0;
}

void Lithium_Playing(edict_t *ent) {
	if(ent->lithium_flags & LITHIUM_PLAYING)
		return;

	GSLog_ClientBegin(ent);
	LNet_Playing(ent);
	ent->lithium_flags &= ~LITHIUM_OBSERVER;
	ent->lithium_flags &= ~LITHIUM_GOCHASECAM;
	ent->lithium_flags |= LITHIUM_PLAYING;
	ent->lithium_flags |= LITHIUM_ATTACKWAIT;
	ChaseSet(ent, NULL);
}

void ZbotHandle(edict_t *ent) {
	if(use_zbotkick->value) {
		gi.bprintf(PRINT_HIGH, ">>> Kicking %s for using Zbot auto-aim.\n", ent->client->pers.netname);
		LNet_BotNotice(ent);
		gi.AddCommandString(va("kick %d\n", ent - g_edicts - 1));
	}
	if(use_zbotlog->value) {
		FILE *file = fopen(file_gamedir("zbot.log"), "at");
		if(file) {
			time_t t;
			char buf[32];
			time(&t);
			strlcpy(buf, ctime(&t), sizeof(buf));
			fprintf(file, "[%s] %s (%d.%d.%d.%d:%d)\n", buf, ent->client->pers.netname, 
				ent->lclient->ip[0], ent->lclient->ip[1], ent->lclient->ip[2], ent->lclient->ip[3], ent->lclient->port);
			fclose(file);
		}
	}
}

qboolean ZbotCheck(edict_t *ent, usercmd_t *ucmd);

qboolean Lithium_ClientThink(edict_t *ent, usercmd_t *ucmd) {
	gclient_t *client = ent->client;

	if(level.intermissiontime) {
		client->ps.pmove.pm_type = PM_FREEZE;
		if(level.time > level.intermissiontime + intermission_time->value && (ucmd->buttons & BUTTON_ANY))
			level.exitintermission = true;
		if(intermission_maxtime->value >= 0 && level.time > level.intermissiontime + intermission_maxtime->value)
			level.exitintermission = true;
		return true;
	}

	if(!ent->lclient->rate_check) {
		ent->lclient->rate_check = true;
		Lithium_MaxRate(ent);
	}

	if(ZbotCheck(ent, ucmd))
		ZbotHandle(ent);

	Admin_ClientThink(ent);

	if(ent->lithium_flags & LITHIUM_GOCHASECAM) {
		if(ucmd->buttons & BUTTON_ATTACK)
			ent->lithium_flags &= ~LITHIUM_GOCHASECAM;
		else {
			if(ent->lithium_flags & LITHIUM_PLAYING)
				Lithium_Observer(ent, false);
			if(!ent->client->chase_target)
				ChaseFind(ent, 1, 1);
			if(ent->client->chase_target)
				ent->lithium_flags &= ~LITHIUM_GOCHASECAM;
		}
	}

	if(ent->lithium_flags & LITHIUM_OBSERVER && ucmd->buttons & BUTTON_ATTACK) {
		if(ctf->value)
			ucmd->buttons &= ~BUTTON_ATTACK;
		if(ctf->value && !ent->client->resp.ctf_team) {
			if(!ent->menu)
				Lithium_CTFMenu(ent);
		}
		else {
			Lithium_Playing(ent);
			PutClientInServer(ent);
		}
	}

	// handle MOTD
	if(ucmd->forwardmove || ucmd->sidemove || ucmd->upmove || 
		ucmd->buttons & BUTTON_ATTACK || ent->client->chase_target) {
		if(level.time > ent->motd_time)
			Lithium_LayoutOff(ent, LAYOUT_MOTD);
		ent->motd_start = 0.0;
	}
	else {
		if(ent->motd_start != 0.0) {
			if(level.time > ent->motd_start && !(ent->layout & LAYOUT_MOTD))
				Lithium_LayoutOn(ent, LAYOUT_MOTD);
		}
		else {
			if(client->pers.weapon == FindItem("blaster") && !(ent->lithium_flags & LITHIUM_OBSERVER))
				ent->motd_start = level.time + 5.0;
		}
	}

	// ping stuff
	if(ent->client->ping)
		ent->lclient->ping = ent->client->ping;
	else
		ent->lclient->ping += (int)(ent->lclient->ping_last - level.time);

	ent->lclient->ping_last = level.time;
	ent->ping_total += ent->lclient->ping;
	ent->ping_count++;

	// check if it's time to remove chat muzzle
	if(ent->lclient->muzzled && level.time > ent->lclient->muzzle_time) {
		gi.cprintf(ent, PRINT_HIGH, "Muzzle removed, stay calm %s.\n", IsFemale(ent) ? "girl" : "boy");
		ent->lclient->muzzled = false;
	}

	// chasecam
	if(client->chase_target) {
		ent->lclient->cam_distance -= ucmd->forwardmove / 200;
		ent->lclient->cam_height += ucmd->sidemove / 200;
		ent->lclient->cam_height += ucmd->upmove / 200;
		if(ent->lclient->cam_distance < -20) ent->lclient->cam_distance = -20;
		if(ent->lclient->cam_distance > 100) ent->lclient->cam_distance = 100;
		if(ent->lclient->cam_height < -20) ent->lclient->cam_height = -20;
		if(ent->lclient->cam_height > 100) ent->lclient->cam_height = 100;
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

		// is our chase target gone?
		if(!client->chase_target->inuse || client->chase_target->lithium_flags & LITHIUM_OBSERVER) {
			if(!ChaseFind(ent, client->chase_target->place, -1))
				ChaseSet(ent, NULL);
		}

		// update to leader if chaseleader
		if(ent->lithium_flags & LITHIUM_CHASELEADER && client->chase_target) {
			if(ent->board == SCORES_BYFPH) {
				if(client->chase_target->fph_place != 1)
					ChaseFind(ent, 1, 1);
			}
			else {
				if(client->chase_target->place != 1)
					ChaseFind(ent, 1, 1);
			}
		}

		return false;
	}

	if(ent->lithium_flags & LITHIUM_ATTACKWAIT) {
		if(!(ucmd->buttons & BUTTON_ATTACK))
			ent->lithium_flags &= ~LITHIUM_ATTACKWAIT;
		else
			ucmd->buttons &= ~BUTTON_ATTACK;
	}

	// safety shield
	if(ent->safety_time && level.time > ent->safety_time)
		ent->safety_time = 0;

	if(ent->safety_time && ucmd->buttons & BUTTON_ATTACK && 
		level.time > ent->safety_time - (safety_time->value - 0.5)) {
		ucmd->buttons &= ~BUTTON_ATTACK;
		ent->safety_time = 0;
	}

	NoCamp_ClientThink(ent, ucmd);
	Var_ClientThink(ent);

	// Orange 2 Hook
	if(client->hook_on && ent->client->hook) 
		Hook_Service(client->hook);

	if(ent->client->chase_target)
		return true;

	return false;
}

void Lithium_MaxRate(edict_t *ent) {
	if(!max_rate->value)
		return;
	if(!ent->inuse)
		return;
	if(!ent->lclient)
		return;
	if(!ent->client->pers.connected)
		return;
//	if(!ent->lclient->rate_check)
//		return;

	if(atoi(Info_ValueForKey(ent->client->pers.userinfo, "rate")) > (int)max_rate->value) {
		Info_SetValueForKey(ent->client->pers.userinfo, "rate", max_rate->string);
		stuffcmd(ent, va("rate %s\n", max_rate->string));
		gi.cprintf(ent, PRINT_HIGH, "Server enforcing max rate of %d.\n", (int)max_rate->value);
	}
}

void Lithium_ClientUserinfoChanged(edict_t *ent, char *userinfo) {
	int i;
	edict_t *cl_ent;
	char *name;

	if(!ent->inuse)
		return;

	name = Info_ValueForKey(userinfo, "name");

	if(strcmp(ent->client->pers.netname, name))
		LNet_ClientRename(ent, name);

	if(ent->lithium_flags & LITHIUM_PLAYING && strcmp(ent->client->pers.netname, name)) {
		// player name changed
		GSLog_PlayerRename(ent->client->pers.netname, name);

		for(i = 0; i < game.maxclients; i++) {
			cl_ent = g_edicts + 1 + i;
			if(!cl_ent->inuse || !cl_ent->client)
				continue;
			if(ent == cl_ent->client->chase_target)
				cl_ent->layout_update = true;
		}
	}
}

void Lithium_LayoutOn(edict_t *ent, int flag) {
	ent->layout |= flag;
	ent->layout_update = true;
	ent->client->showscores = true;
	ent->client->showinventory = false;
}

void Lithium_LayoutOff(edict_t *ent, int flag) {
	if(!(ent->layout & flag)) {
		ent->client->showscores = ent->layout ? 1 : 0;
		return;
	}
	ent->layout &= ~flag;
	ent->client->showscores = ent->layout ? 1 : 0;
	if(ent->layout)
		ent->layout_update = true;
}

void Lithium_LayoutTog(edict_t *ent, int flag) {
	ent->layout ^= flag;
	ent->client->showscores = ent->layout ? 1 : 0;
	if(ent->layout)
		ent->layout_update = true;
}

void Lithium_ClientBeginFrame(edict_t *ent) {
	Rune_ClientFrame(ent);
	Menu_ClientFrame(ent);
}

void CTFSetIDView(edict_t *ent);

void Lithium_ClientEndFrame(edict_t *ent) {
	if(!ent->lclient)
		return;

	if(ent->lithium_flags & LITHIUM_PLAYING && !level.intermissiontime)
		ent->play_frames++;

	/*
	if(ent->lithium_flags & LITHIUM_OBSERVER && atoi(Info_ValueForKey(ent->client->pers.userinfo, "spectator")) == 0)
		Lithium_Playing(ent);
	else if(ent->lithium_flags & LITHIUM_PLAYING && atoi(Info_ValueForKey(ent->client->pers.userinfo, "spectator")) == 1)
		Lithium_Observer(ent, true);
	*/

	if(!level.intermissiontime && ent->layout & LAYOUT_ID)
		CTFSetIDView(ent);

	if(ent->news_time && level.time > ent->news_time) {
		Lithium_LayoutOff(ent, LAYOUT_NEWS);
		ent->news_time = 0;
	}

	if(ent->centerprint_time && level.time > ent->centerprint_time) {
		centerprintf(ent, "");
		ent->centerprint_time = 0;
	}

	// heh, hacky...
	if(ctf->value && level.time > ent->ctf_players_time && ent->menu && ent->menu->firstline && ent->menu->firstline->next && strstr(ent->menu->firstline->next->text, "ThreeWave"))
		Lithium_UpdateCTFMenu(ent);

	if(level.framenum - ent->client->resp.enterframe < 2)
		return;

	if(level.framenum - ent->update_frame > ofp_maxframes->value ||
			(level.framenum - ent->update_frame) * 500000 /
			(int)(ofp_base->value + (float)countplayers() * ofp_perplayer->value) + 1 >
			(ent->update_size + ent->update_other) * 
			MIN(MAX(ent->lclient->ping, ofp_minping->value), ofp_maxping->value)) {
		ent->update_size = StatusBar_Update(ent);
		if(!ent->update_size)
			ent->update_size = Layout_Update(ent);
		ent->update_frame = level.framenum + 1;
		ent->update_other = 0;
	}

	// client has hit ESC key
	if(ent->layout && !ent->client->showscores) {
		if(ent->layout & LAYOUT_MENU && ent->menu->editing) {
			strlcpy(ent->menu->edit, "  ", sizeof(ent->menu->edit));
			Menu_EditEnd(ent);
			ent->client->showscores = true;
			return;
		}
		if(ent->layout & LAYOUT_SCORES) {
			Lithium_LayoutOff(ent, LAYOUT_SCORES);
			return;
		}
		ent->lithium_flags ^= LITHIUM_MAINMENU;
		if(ent->lithium_flags & LITHIUM_MAINMENU) {
			stuffcmd(ent, "menu_main\n");
			ent->client->showscores = ent->layout ? 1 : 0;
		}
	}
}

// happens after clientobit and score is adjusted
void Lithium_PlayerDie(edict_t *attacker, edict_t *self) {

	FragTrak_Frag(attacker, self);
	Hook_PlayerDie(attacker, self);
}


void Lithium_DoUpgrade(void) {
	char cmd[100];
	edict_t *ent;
	int i;

	gi.bprintf(PRINT_HIGH, "\nServer is upgrading.  Please wait...\n\n");

	snprintf(cmd, sizeof(cmd), "reconnect\n");

	for(i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if(!ent->inuse)
			continue;
		stuffcmd(ent, cmd);
	}

	gi.AddCommandString("quit\n");
}

void Armor_Realize(gitem_armor_t *armor_info, char *string) {
	sscanf(string, "%d %d %f %f", &armor_info->base_count, &armor_info->max_count,
		&armor_info->normal_protection, &armor_info->energy_protection);
}

// take time from respawn delay
int FasterRespawn(edict_t *ent, float delay) {
	float faster = fast_respawn->value;
	float pcount = (float)countplayers();

	// quad, invul, power shield, etc are >= 60 second, don't let them come back as fast
	if(delay >= 60)
		faster /= 2.0;

	// not the sort of thing you ask about
	pcount = MAX(pcount, fast_minpbound->value);
	pcount = MIN(pcount, fast_maxpbound->value);
	return (int)(delay * (1.0f - faster * pcount / MAX(fast_maxpbound->value, 1.0f)) + 0.5f);
}

//fix
extern edict_t *trak_ent[];

//===============================
// pick best weapon

int Weapon_GetValue(char *weappickup, gitem_t *item) {
	int weap = 0;
	if(item == FindItem("blaster")) weap = 0;
	else if(item == FindItem("shotgun")) weap = 1;
	else if(item == FindItem("super shotgun")) weap = 2;
	else if(item == FindItem("machinegun")) weap = 3;
	else if(item == FindItem("chaingun")) weap = 4;
	else if(item == FindItem("grenade launcher")) weap = 5;
	else if(item == FindItem("rocket launcher")) weap = 6;
	else if(item == FindItem("hyperblaster")) weap = 7;
	else if(item == FindItem("railgun")) weap = 8;
	else if(item == FindItem("bfg")) weap = 9;
	return weappickup[weap] - '0';

/*
	if(item == FindItem("blaster")) return 1;
	if(item == FindItem("shotgun")) return 2;
	if(item == FindItem("super shotgun")) return 4;
	if(item == FindItem("machinegun")) return 3;
	if(item == FindItem("chaingun")) return 4;
	if(item == FindItem("grenade launcher")) return 3;
	if(item == FindItem("rocket launcher")) return 5;
	if(item == FindItem("hyperblaster")) return 6;
	if(item == FindItem("railgun")) return 4;
	if(item == FindItem("bfg")) return 4;
	return 0;
*/
}

void Weapon_PickBest(edict_t *ent, edict_t *other) {
	char *weappickup;
	char *defpickup = "1243435644";

	if(other->client->pers.weapon == ent->item)
		return;

	weappickup = Info_ValueForKey(other->client->pers.userinfo, "weappickup");
 	if(strlen(weappickup) != 10)
		weappickup = defpickup;

	if(other->client->pers.weapon == FindItem("blaster") &&
		other->client->pers.inventory[ITEM_INDEX(ent->item)] == 1) {
		other->client->newweapon = ent->item;
		return;
	}

	if(other->client->buttons & BUTTON_ATTACK)
		return;

	if(other->bestweap && Weapon_GetValue(weappickup, ent->item) > Weapon_GetValue(weappickup, other->client->pers.weapon))
		other->client->newweapon = ent->item;
}

struct {
	char *weap;
	char *ammo;
	int rounds;
} weaplist[9] = {
	{"blaster", "blaster", 1},
	{"shotgun", "shells", 1},
	{"super shotgun", "shells", 2},
	{"machinegun", "bullets", 1},
	{"chaingun", "bullets", 1},
	{"grenade launcher", "grenades", 1},
	{"rocket launcher", "rockets", 1},
	{"hyperblaster", "cells", 1},
	{"railgun", "slugs", 1},
};

void Lithium_NoAmmoWeaponChange(edict_t *ent) {
	char *weaporder;
	char *deforder = "8543921";
	int i, j;

	weaporder = Info_ValueForKey(ent->client->pers.userinfo, "weaporder");
	if(!strlen(weaporder))
		weaporder = deforder;

	for(i = 0; i < strlen(weaporder); i++) {
		j = weaporder[i] - '0' - 1;
		if(j < 0 || j > 8)
			continue;
		if(ent->client->pers.inventory[ITEM_INDEX(FindItem(weaplist[j].ammo))] >= weaplist[j].rounds
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem(weaplist[j].weap))]) {
			ent->client->newweapon = FindItem(weaplist[j].weap);
			return;
		}
	}

	ent->client->newweapon = FindItem("blaster");
}

void SelectPrevItem(edict_t *ent, int itflags);
void SelectNextItem(edict_t *ent, int itflags);

void Lithium_SelectPrevItem(edict_t *ent) {
	if(ent->layout & LAYOUT_SCORES && !ent->centerprint_time && ent->board) {
		int old = ent->sel;
		if(ent->sel >= 4)
			ent->sel -= 4;
		else
			ent->sel = 0;
		if(ent->sel != old)
			ent->layout_update = true;
		return;
	}

	if(ent->client->chase_target) {
		ChasePrev(ent);
		return;
	}

	SelectPrevItem(ent, -1);
}

void Lithium_SelectNextItem(edict_t *ent) {
	if(ent->layout & LAYOUT_SCORES && !ent->centerprint_time && ent->board) {
		int players = countclients();
		int old = ent->sel;
		if(ent->sel <= players - ent->lclient->board_show - 4)
			ent->sel += 4;
		else
			ent->sel = MAX(0, players - ent->lclient->board_show);
		if(ent->sel != old)
			ent->layout_update = true;
		return;
	}

	if(ent->client->chase_target) {
		ChaseNext(ent);
		return;
	}

	SelectNextItem(ent, -1);
}

int clients = 0;
int players = 0;

//CTF
int team1 = 0;
int team2 = 0;
char team1players[32] = "";
char team2players[32] = "";
//CTF

int countclients(void) {
	return clients;
}

int countplayers(void) {
	return players;
}

int countteam1(void) {
	return team1;
}

int countteam2(void) {
	return team2;
}

int sorted_ents = 0;
edict_t *sorted_ent[MAX_CLIENTS];
edict_t *fph_sorted_ent[MAX_CLIENTS];

// called each frame
void Lithium_CalcPlaces(void) {
	int i, j, k;
	int score, total;
	int sorted[MAX_CLIENTS];
	int sortedscores[MAX_CLIENTS];
	int fph_sorted[MAX_CLIENTS];
	int fph_sortedscores[MAX_CLIENTS];
	edict_t *ent;
	int score1, score2;

	clients = 0;
	players = 0;
	team1 = 0;
	team2 = 0;
	score1 = 0;
	score2 = 0;

	total = 0;
	for(i = 0; i < game.maxclients; i++) {
		ent = g_edicts + 1 + i;
		if(!ent->inuse || ent->lithium_flags & LITHIUM_HIDDEN) {
			ent->place = -1;
			continue;
		}

		clients++;

		score = -999;

		if(ent->lithium_flags & LITHIUM_PLAYING) {
			score = ent->client->resp.score;
			players++;

			if(ctf->value) {
				if(ent->client->resp.ctf_team == CTF_TEAM1) {
					team1++;
					score1 += score;
				}
				if(ent->client->resp.ctf_team == CTF_TEAM2) {
					team2++;
					score2 += score;
				}
			}
		}

		for(j = 0; j < total; j++)
			if(score > sortedscores[j])
				break;
		for(k = total; k > j; k--) {
			sorted[k] = sorted[k - 1];
			sortedscores[k] = sortedscores[k - 1];
		}
		sorted[j] = i;
		sortedscores[j] = score;

		if(ent->play_frames)
			ent->fph = 36000 * ent->client->resp.score / ent->play_frames;
		else
			ent->fph = 0;

		if(ent->lithium_flags & LITHIUM_PLAYING)
			score = ent->fph;

		for(j = 0; j < total; j++)
			if(score > fph_sortedscores[j])
				break;
		for(k = total; k > j; k--) {
			fph_sorted[k] = fph_sorted[k - 1];
			fph_sortedscores[k] = fph_sortedscores[k - 1];
		}
		fph_sorted[j] = i;
		fph_sortedscores[j] = score;

		total++;
	}

	sorted_ents = total;

	for(i = 0; i < total; i++) {
		ent = g_edicts + 1 + sorted[i];
		if(ent->lithium_flags & LITHIUM_PLAYING)
			ent->place = i + 1;
		else
			ent->place = 0;
		sorted_ent[i] = ent;

		ent = g_edicts + 1 + fph_sorted[i];
		if(ent->lithium_flags & LITHIUM_PLAYING)
			ent->fph_place = i + 1;
		else
			ent->fph_place = 0;
		fph_sorted_ent[i] = ent;
	}

	if(ctf->value) {
		snprintf(team1players, sizeof(team1players), "  (%d players, %d score)", team1, score1);
		snprintf(team2players, sizeof(team2players), "  (%d players, %d score)", team2, score2);
	}
}

void Lithium_SetClientEffects(edict_t *ent) {
	if(ent->deadflag & DEAD_DEAD)
		return;
	if(ent->safety_time) {
		ent->s.effects |= EF_COLOR_SHELL;
		if(qver >= 3.19f)
			ent->s.renderfx = RF_SHELL_DOUBLE;
		else
			ent->s.renderfx = RF_SHELL_RED | RF_SHELL_GREEN;
	}
}

//------------------------
// chasecam

void ChaseSet(edict_t *self, edict_t *ent) {
	edict_t *old_chase = self->client->chase_target;

	if(self->client->chase_target == ent)
		return;

	if(self->deadflag == DEAD_DEAD)
		respawn(self);

	self->client->chase_target = ent;

	if(old_chase && old_chase->inuse && old_chase->lithium_flags & LITHIUM_CHASENOTICE)
		gi.cprintf(old_chase, PRINT_HIGH, "%s is no longer chasing you.\n", self->client->pers.netname);
	if(ent && ent->lithium_flags & LITHIUM_CHASENOTICE)
		gi.cprintf(ent, PRINT_HIGH, "%s is chasing you now.\n", self->client->pers.netname);

	if(ent) {
		Lithium_LayoutOn(self, LAYOUT_CHASECAM);
		Lithium_LayoutOff(self, LAYOUT_MENU);
		self->motd_time -= 3.0;
	}
	else
		Lithium_LayoutOff(self, LAYOUT_CHASECAM);
}

qboolean ChaseFind(edict_t *ent, int place, int searchdir) {
	int i;
	edict_t *e;
	int players = countplayers();
	int start;

	if(place < 1)
		place = players;
	else if(place > players)
		place = 1;
	
	start = place;
	while(true) {
		for(i = 0; i < game.maxclients; i++) {
			e = g_edicts + 1 + i;
			if(e->inuse && (!place || e->place == place) && e != ent && e->lithium_flags & LITHIUM_PLAYING) {
				ChaseSet(ent, e);
				return true;
			}
		}

		if(!place || !searchdir)
			break;

		place += searchdir;
		if(place < 1)
			place = players;
		else if(place > players)
			place = 1;
		if(place == start)
			place = 0;
	}

	return false;
}

void ChaseStart(edict_t *ent) {
	if(ent->client->chase_target && !strlen(gi.argv(1))) {
		ChaseSet(ent, NULL);
		return;
	}

	if(!Lithium_Observer(ent, true))
		return;

	ent->lithium_flags &= ~LITHIUM_CHASELEADER;

	if(strlen(gi.argv(1))) {
		int val = atoi(gi.argv(1));
		if(val > countplayers() || val <= 0 || !ChaseFind(ent, val, 0))
			gi.cprintf(ent, PRINT_HIGH, "Couldn't chase place #%d.\n", val);
		return;
	}

	if(!ChaseFind(ent, 1, 1))
		gi.cprintf(ent, PRINT_HIGH, "No one to chase.\n");
}

void ChaseNext(edict_t *ent) {
	if(!ent->client->chase_target)
		return;

	ent->lithium_flags &= ~LITHIUM_CHASELEADER;
	if(ent->board == SCORES_BYFPH)
		ChaseFind(ent, ent->client->chase_target->fph_place + 1, 1);
	else
		ChaseFind(ent, ent->client->chase_target->place + 1, 1);
}

void ChasePrev(edict_t *ent) {
	if(!ent->client->chase_target)
		return;

	ent->lithium_flags &= ~LITHIUM_CHASELEADER;
	if(ent->board == SCORES_BYFPH)
		ChaseFind(ent, ent->client->chase_target->fph_place - 1, -1);
	else
		ChaseFind(ent, ent->client->chase_target->place - 1, -1);
}

// from CTF2
void UpdateChaseCam(edict_t *ent) {
	vec3_t o, ownerv, goal;
	edict_t *targ;
	vec3_t forward;
	trace_t trace;
	vec3_t angles;

	targ = ent->client->chase_target;

	// is our chase target gone?
	/* handled in Lithium_ClientThink now
	if(!targ->inuse) {
		ChaseSet(ent, NULL);
		return;
	}
	*/

	VectorCopy(targ->s.origin, ownerv);

	ownerv[2] += targ->viewheight;

	VectorCopy(targ->client->v_angle, angles);
	if (angles[PITCH] > ent->lclient->cam_maxpitch)
		angles[PITCH] = ent->lclient->cam_maxpitch;
	AngleVectors (angles, forward, NULL, NULL);
	VectorNormalize(forward);
	VectorMA(ownerv, -ent->lclient->cam_distance, forward, o);

	if (o[2] < targ->s.origin[2] + ent->lclient->cam_height)
		o[2] = targ->s.origin[2] + ent->lclient->cam_height;

	// jump animation lifts
	if (!targ->groundentity)
		o[2] += ent->lclient->cam_jump;

	trace = gi.trace(ownerv, vec3_origin, vec3_origin, o, targ, MASK_SOLID);

	VectorCopy(trace.endpos, goal);

	VectorMA(goal, 2, forward, goal);

	// pad for floors and ceilings
	VectorCopy(goal, o);
	o[2] += 6;
	trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
	if (trace.fraction < 1) {
		VectorCopy(trace.endpos, goal);
		goal[2] -= 6;
	}

	VectorCopy(goal, o);
	o[2] -= 6;
	trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
	if (trace.fraction < 1) {
		VectorCopy(trace.endpos, goal);
		goal[2] += 6;
	}

	VectorCopy(goal, ent->s.origin);

	if(!ent->lclient->cam_freelook) {
		int i;
		ent->client->ps.pmove.pm_type = PM_FREEZE;
		for(i = 0; i < 3; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

		VectorCopy(targ->client->v_angle, ent->client->ps.viewangles);
		VectorCopy(targ->client->v_angle, ent->client->v_angle);
	}
	else
		ent->client->ps.pmove.pm_type = PM_NORMAL;

	ent->viewheight = 0;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	gi.linkentity(ent);
}

//------------
// player id, mainly from CTF2

static void loc_buildboxpoints(vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
{
	VectorAdd(org, mins, p[0]);
	VectorCopy(p[0], p[1]);
	p[1][0] -= mins[0];
	VectorCopy(p[0], p[2]);
	p[2][1] -= mins[1];
	VectorCopy(p[0], p[3]);
	p[3][0] -= mins[0];
	p[3][1] -= mins[1];
	VectorAdd(org, maxs, p[4]);
	VectorCopy(p[4], p[5]);
	p[5][0] -= maxs[0];
	VectorCopy(p[0], p[6]);
	p[6][1] -= maxs[1];
	VectorCopy(p[0], p[7]);
	p[7][0] -= maxs[0];
	p[7][1] -= maxs[1];
}

static qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
{
	trace_t	trace;
	vec3_t	targpoints[8];
	int i;
	vec3_t viewpoint;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
		return false; // bmodels not supported

	loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);
	
	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;

	for (i = 0; i < 8; i++) {
		trace = gi.trace (viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
	}

	return false;
}

void CTFSetIDView(edict_t *ent) {
	vec3_t	forward, dir;
	edict_t	*who, *best;
	float bd = 0, d;
	int i;

	if(ent->id_ent && level.framenum & 1)
		return;

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	best = NULL;
	for (i = 1; i <= maxclients->value; i++) {
		who = g_edicts + i;
		if (!who->inuse || who->lithium_flags & LITHIUM_OBSERVER)
			continue;
		VectorSubtract(who->s.origin, ent->s.origin, dir);
		VectorNormalize(dir);
		d = DotProduct(forward, dir);
		if (d > bd && loc_CanSee(ent, who)) {
			bd = d;
			best = who;
		}
	}

	if(!(bd > 0.90))
		best = NULL;

	if(best == ent || best == ent->client->chase_target || (best && best->deadflag == DEAD_DEAD))
		best = NULL;

	if(ent->id_ent != best) {
		ent->id_ent = best;
		if(ent->layout & (LAYOUT_CENTERPRINT | LAYOUT_CHASECAM | LAYOUT_ID))
			ent->layout_update = true;
	}
}

// ------------
// misc

char *file_gamedir(char *name) {
	char gdir[256];

	if(strchr(name, '/') || strchr(name, '\\'))
		strlcpy(file_gamedir_buffer, name, sizeof(file_gamedir_buffer));
	else {
		strlcpy(gdir, gi.cvar("gamedir", "", 0)->string, sizeof(gdir));
		if(!strlen(gdir))
			strlcpy(gdir, "baseq2", sizeof(gdir));
		snprintf(file_gamedir_buffer, sizeof(file_gamedir_buffer), "%s/%s", gdir, name);
	}

	return file_gamedir_buffer;
}

qboolean file_exist(char *name) {
	FILE *file;
	if(!name)
		return false;
	if(!strlen(name))
		return false;
	file = fopen(name, "rb");
	if(!file)
		return false;
	fclose(file);
	return true;
}

// removing beginning and trailing whitespaces
void String_Crop(char *str) {
	char *c;
	int l;

	l = strlen(str)+1;
	c = gi.TagMalloc(l, TAG_GAME);
	if (!c)
		return;
	strlcpy(c, str, l);

	while(*c != '\0' && (*c == ' ' || *c == '\t'))
		c++;
	strlcpy(str, c, l);
	gi.TagFree(c);

	c = str + strlen(str) - 1;
	while(c > str && (*c == ' ' || *c == '\t'))
		c--;

	c++;
	if(*c)
		*c = '\0';
}

int strip(char *str) {
	char *c;
	c = strchr(str, 10);
	if(c)
		*c = 0;
	c = strchr(str, 13);
	if(c)
		*c = 0;

	String_Crop(str);

	return strlen(str);
}

void stuffcmd(edict_t *e, char *s) {
	gi.WriteByte(11);
	gi.WriteString(s);
	gi.unicast(e, true);
}

void centerprintf(edict_t *ent, char *format, ...) {
	va_list argptr;

	char old[1000];

	if(!ent->centerprint)
		return;

	if(!strlen(format)) {
		strlcpy(ent->centerprint, "", sizeof(ent->centerprint));
		Lithium_LayoutOff(ent, LAYOUT_CENTERPRINT);
		return;
	}

	strlcpy(old, ent->centerprint, sizeof(old));

	va_start(argptr, format);
	vsnprintf(ent->centerprint, sizeof(ent->centerprint), format, argptr);
	va_end(argptr);

	ent->centerprint_time = level.time + 2.5;

	if(strcmp(old, ent->centerprint))
		Lithium_LayoutOn(ent, LAYOUT_CENTERPRINT);
}

void centerprintf2(edict_t *ent, char *format, ...) {
	va_list argptr;

	char old[1000];

	if(!ent->centerprint2)
		return;

	if(!strlen(format)) {
		strlcpy(ent->centerprint2, "", sizeof(ent->centerprint2));
		Lithium_LayoutOff(ent, LAYOUT_CENTERPRINT);
		return;
	}

	strlcpy(old, ent->centerprint2, sizeof(old));

	va_start(argptr, format);
	vsnprintf(ent->centerprint2, sizeof(ent->centerprint2), format, argptr);
	va_end(argptr);

	if(strcmp(old, ent->centerprint))
		Lithium_LayoutOn(ent, LAYOUT_CENTERPRINT);
}


qboolean Lithium_IsObserver(edict_t *ent) {
	return ent->lithium_flags & LITHIUM_OBSERVER;
}

qboolean Lithium_IsHidden(edict_t *ent) {
	return ent->lithium_flags & LITHIUM_HIDDEN;
}

//-----------------------------------------------------
// commands

void Cmd_Inven_f (edict_t *ent);

qboolean Lithium_ClientCommand(edict_t *ent) {
	char *cmd = gi.argv(0);

	if(!ent->lclient)
		return false;

	if(Menu_ClientCommand(ent))
		return true;

	if(Admin_ClientCommand(ent))
		return true;

	if(Vote_ClientCommand(ent))
		return true;

	if(Q_stricmp(cmd, "use") == 0) {
		if(!Lithium_Use(ent))
			return false;
	}
	else if(Q_stricmp(cmd, "zbotinfo") == 0) {
		if(use_zbotkick->value)
			gi.cprintf(ent, PRINT_HIGH, "Server Zbot detection is on.  The Lithium II Mod can reliably detect Zbot auto-aim clients.  They are kicked immediately.\n");
		else if(use_zbotlog->value)
			gi.cprintf(ent, PRINT_HIGH, "Server Zbot detection is on.  The Lithium II Mod can reliably detect Zbot auto-aim clients.  They are logged only.\n");
		else
			gi.cprintf(ent, PRINT_HIGH, "Server Zbot detection is off.\n");
	}
	else if(Q_stricmp(cmd, "invnext") == 0)
		Lithium_SelectNextItem(ent);
	else if (Q_stricmp(cmd, "invprev") == 0)
		Lithium_SelectPrevItem(ent);
	else if(!Q_stricmp(cmd, "rune_drop") || !Q_stricmp(cmd, "runedrop") || 
			!Q_stricmp(cmd, "drop_rune") || !Q_stricmp(cmd, "droprune"))
		Rune_Drop(ent);
	else if(Q_stricmp(cmd, "observer") == 0 || Q_stricmp(cmd, "observe") == 0)
		Lithium_Observer(ent, true);

	// menu handling
	else if(!Q_stricmp(cmd, "menu")) {
		if(ent->menu)
			Menu_Destroy(ent);
		else
			Lithium_Menu(ent);
	}

	else if(Q_stricmp(cmd, "chasecam") == 0 || Q_stricmp(cmd, "chase") == 0)
		ChaseStart(ent);
	else if(Q_stricmp(cmd, "chasenext") == 0)
		ChaseNext(ent);
	else if(Q_stricmp(cmd, "chaseprev") == 0)
		ChasePrev(ent);
	else if(Q_stricmp(cmd, "chaseleader") == 0) {
		if(Lithium_Observer(ent, true)) {
			ent->lithium_flags |= LITHIUM_CHASELEADER;
			ChaseFind(ent, 1, 1);
		}
	}
	else if(Q_stricmp(cmd, "chaselist") == 0) {
		edict_t *cl_ent;
		int i;
		for(i = 0; i < game.maxclients; i++) {
			cl_ent = g_edicts + 1 + i;
			if(!cl_ent->inuse)
				continue;
			if(cl_ent->client->chase_target == ent)
				gi.cprintf(ent, PRINT_HIGH, "   %s\n", cl_ent->client->pers.netname);
		}
	}
	else if(Q_stricmp(cmd, "chasenotice") == 0) {
		ent->lithium_flags ^= LITHIUM_CHASENOTICE;
		if(ent->lithium_flags & LITHIUM_CHASENOTICE)
			gi.cprintf(ent, PRINT_HIGH, "Chase notice enabled.\n");
		else
			gi.cprintf(ent, PRINT_HIGH, "Chase notice disabled.\n");
	}
	else if(Q_stricmp(cmd, "chaseself") == 0) {
	}
	else if(Q_stricmp(cmd, "cam_freelook") == 0) {
		if(strlen(gi.argv(1)))
			ent->lclient->cam_freelook = atoi(gi.argv(1));
		else
			gi.cprintf(ent, PRINT_HIGH, "\"cam_freelook\" is \"%d\"\n", ent->lclient->cam_freelook);
	}
	else if(Q_stricmp(cmd, "cam_distance") == 0) {
		if(strlen(gi.argv(1)))
			ent->lclient->cam_distance = atoi(gi.argv(1));
		else
			gi.cprintf(ent, PRINT_HIGH, "\"cam_distance\" is \"%d\"\n", ent->lclient->cam_distance);
	}
	else if(Q_stricmp(cmd, "cam_height") == 0) {
		if(strlen(gi.argv(1)))
			ent->lclient->cam_height = atoi(gi.argv(1));
		else
			gi.cprintf(ent, PRINT_HIGH, "\"cam_height\" is \"%d\"\n", ent->lclient->cam_height);
	}
	else if(Q_stricmp(cmd, "cam_jump") == 0) {
		if(strlen(gi.argv(1)))
			ent->lclient->cam_jump = atoi(gi.argv(1));
		else
			gi.cprintf(ent, PRINT_HIGH, "\"cam_jump\" is \"%d\"\n", ent->lclient->cam_jump);
	}
	else if(Q_stricmp(cmd, "cam_maxpitch") == 0) {
		if(strlen(gi.argv(1)))
			ent->lclient->cam_maxpitch = atoi(gi.argv(1));
		else
			gi.cprintf(ent, PRINT_HIGH, "\"cam_maxpitch\" is \"%d\"\n", ent->lclient->cam_maxpitch);
	}
	else if(Q_stricmp(cmd, "board_show") == 0) {
		if(strlen(gi.argv(1)))
			ent->lclient->board_show = atoi(gi.argv(1));
		else
			gi.cprintf(ent, PRINT_HIGH, "\"board_show\" is \"%d\"\n", ent->lclient->board_show);
		if(ent->lclient->board_show < 4)
			ent->lclient->board_show = 4;
		if(ent->lclient->board_show > 16)
			ent->lclient->board_show = 16;
	}
	else if (Q_stricmp (cmd, "invuse") == 0 && ent->client->chase_target)
		ent->lclient->cam_freelook ^= 1;
	else if (Q_stricmp (cmd, "invdrop") == 0 && ent->client->chase_target) {
		ent->lclient->cam_freelook = 0;
		ent->lclient->cam_distance = 40;
		ent->lclient->cam_height = 40;
		ent->lclient->cam_jump = 16;
		ent->lclient->cam_maxpitch = 56;
	}
	else if(Q_stricmp(cmd, "board") == 0) {
		ent->board = atoi(gi.argv(1));
	}
	else if(Q_stricmp(cmd, "scores") == 0) {
		Lithium_LayoutOn(ent, LAYOUT_SCORES);
		Lithium_LayoutOff(ent, LAYOUT_HSCORES);
	}
	else if(Q_stricmp(cmd, "noscores") == 0) {
		if(!level.intermissiontime)
			Lithium_LayoutOff(ent, LAYOUT_SCORES);
	}
	else if(Q_stricmp(cmd, "hscores") == 0) {
		Lithium_LayoutOn(ent, LAYOUT_HSCORES);
		Lithium_LayoutOff(ent, LAYOUT_SCORES);
	}
	else if(Q_stricmp(cmd, "nohscores") == 0) {
		Lithium_LayoutOff(ent, LAYOUT_HSCORES);
	}
	// Orange 2 Hook
	else if (!Q_stricmp (cmd, "hook")) {
		if(use_hook->value) {
			if(!(ent->lithium_flags & LITHIUM_OBSERVER) && ent->deadflag == DEAD_NO) {
				Weapon_Hook_Fire(ent);
				ent->safety_time = 0;
			}
		}
		else
			gi.cprintf(ent, PRINT_HIGH, "Hook disabled on this server.\n");
	}
	else if (!Q_stricmp (cmd, "unhook")) {
		if(use_hook->value && !(ent->lithium_flags & LITHIUM_OBSERVER))
			Hook_Reset(ent->client->hook);
	}
	else if(!Q_stricmp(cmd, "id")) {
		ent->id_state ^= 1;
		if(ent->id_state) {
			Lithium_LayoutOn(ent, LAYOUT_ID);
			gi.cprintf(ent, PRINT_MEDIUM, "Player ID enabled.\n");
		}
		else {
			Lithium_LayoutOff(ent, LAYOUT_ID);
			gi.cprintf(ent, PRINT_MEDIUM, "Player ID disabled.\n");
		}
	}
	else if(!Q_stricmp(cmd, "bestweap")) {
		ent->bestweap ^= 1;
		if(ent->bestweap)
			gi.cprintf(ent, PRINT_MEDIUM, "Best weapon auto-select enabled.\n");
		else
			gi.cprintf(ent, PRINT_MEDIUM, "Best weapon auto-select disabled.\n");
	}
	else if(!Q_stricmp(cmd, "weappickup"))
		Info_SetValueForKey(ent->client->pers.userinfo, "weappickup", gi.argv(1));
	else if(!Q_stricmp(cmd, "weaporder"))
		Info_SetValueForKey(ent->client->pers.userinfo, "weaporder", gi.argv(1));
	else if(!Q_stricmp(cmd, "ping")) {
		edict_t *cl_ent;
		int i;
		for(i = 0; i < game.maxclients; i++) {
			cl_ent = g_edicts + 1 + i;
			if(!cl_ent->inuse)
				continue;
			gi.cprintf(ent, PRINT_HIGH, "%4d %s\n", cl_ent->client->ping, cl_ent->client->pers.netname);
		}
	}
	else if(!Q_stricmp(cmd, "motd")) {
		Lithium_LayoutOn(ent, LAYOUT_MOTD);
		ent->motd_time = level.time + 8.0;
	}
	else if(!Q_stricmp(cmd, "hud")) {
		if(strlen(gi.argv(1))) {
			ent->hud = atoi(gi.argv(1));
			if(ent->hud < 0 || ent->hud > HUD_MAX)
				ent->hud = HUD_LITHIUM;
		}
		else {
			ent->hud++;
			if(ent->hud > HUD_MAX)
				ent->hud = 1;
		}
		ent->lithium_flags |= LITHIUM_STATUSBAR;
	}

	else if(!Q_stricmp (cmd, "inven") || !Q_stricmp (cmd, "inventory")) {
		ent->layout = 0;
		ent->client->showscores = false;
		Cmd_Inven_f(ent);
	}

	else if(!Q_stricmp(cmd, "ver"))
		gi.cprintf(ent, PRINT_HIGH, "Lithium II v%s (" __DATE__ ")\n", lithium_version);

	else if(!Q_stricmp(cmd, "timeleft")) {
		if(timelimit->value) {
			int sec = (int)(timelimit->value * 60 + empty_time - level.time);
			gi.cprintf(ent, PRINT_HIGH, "Time left is %d:%02d\n", sec / 60, sec % 60);
		}
		else
			gi.cprintf(ent, PRINT_HIGH, "No timelimit set.\n");
	}
	else if(!Q_stricmp(cmd, "mapname")) {
		gi.cprintf(ent, PRINT_HIGH, "Map name is %s\n", level.mapname);
	}
	else if(!Q_stricmp(cmd, "highscores") || !Q_stricmp(cmd, "hiscores")) {
		Highscores_List(ent);
	}
	else if(!Q_stricmp(cmd, "eval")) {
		lvar_t *lvar = first_lvar;
		while(lvar) {
			if(!stricmp(gi.argv(1), lvar->cvar->name)) {
				gi.cprintf(ent, PRINT_HIGH, "\"%s\" is \"%s\"\n", lvar->cvar->name, lvar->cvar->string);
				return true;
			}
			lvar = lvar->next;
		}
		return true;
	}

	/*
	else if(!Q_stricmp(cmd, "givescore")) {
		ent->client->resp.score = atoi(gi.argv(1));
		end_time = level.time;
	}

	*/

	else
		return false;
	return true;
}


qboolean Lithium_Use(edict_t *ent) {
	char *cmd = gi.args();
	int key;

	if(!Q_stricmp(cmd, "blaster")) key = 1;
	else if(!Q_stricmp(cmd, "shotgun")) key = 2;
	else if(!Q_stricmp(cmd, "super shotgun")) key = 3;
	else if(!Q_stricmp(cmd, "machinegun")) key = 4;
	else if(!Q_stricmp(cmd, "chaingun")) key = 5;
	else if(!Q_stricmp(cmd, "grenade launcher")) key = 6;
	else if(!Q_stricmp(cmd, "rocket launcher")) key = 7;
	else if(!Q_stricmp(cmd, "hyperblaster")) key = 8;
	else if(!Q_stricmp(cmd, "railgun")) key = 9;
	else key = 0;

	if(ent->layout & LAYOUT_MENU) {
		Menu_Key(ent, key);
		return true;
	}

	if(ent->layout & LAYOUT_SCORES) {
		if(key == 0)
			Lithium_Menu(ent);
		else if(key == 1)
			ent->board = SCORES_NORMAL;
		else if(key == 2)
			ent->board = SCORES_BYFRAGS;
		else if(key == 3)
			ent->board = SCORES_BYFPH;
		else 
			return false;
		ent->layout_update = true;
		return true;
	}

	if(ent->lithium_flags & LITHIUM_OBSERVER) {
		if(key == 0 || key == 1)
			Lithium_Menu(ent);
		return true;
	}
	
//	if(!Q_stricmp(gi.args(), "grapple") || !Q_stricmp(gi.args(), "hook"))
//		return true;

	return false;
}

qboolean Lithium_CmdSay(edict_t *ent) {
	char *p = gi.args();

	lclient_t *lclient = ent->lclient;
	if(!lclient)
		return false;

	// prevent v3.12/3.13 clients from spamming because of unknown command binds
	if(*p == '+' || *p == '-') {
		gi.cprintf(ent, PRINT_HIGH, "Unknown command \"%s\"\n", p);
		return false;
	}

	if(Lithium_IsHidden(ent))
		return false;

	// being kicked already
	if(lclient->kick_says == -1)
		return false;

	// determine if they need kickin
	if(level.time < lclient->kick_say_time + chatkick_time->value) {
		lclient->kick_says++;
		if(use_chatkick->value && lclient->kick_says > chatkick_says->value) {
			lclient->kick_says = -1;
			if(lclient->muzzled)
				gi.bprintf(PRINT_HIGH, ">>> Kicking %s for trying to chew %s muzzle off.\n", ent->client->pers.netname,
					IsFemale(ent) ? "her" : "his");
			else
				gi.bprintf(PRINT_HIGH, ">>> Kicking %s for chatting too much.\n", ent->client->pers.netname);
			gi.AddCommandString(va("kick %d\n", ent - g_edicts - 1));
			return false;
		}
	}
	else
		lclient->kick_says = 1;
	lclient->kick_say_time = level.time;

	// determine if they need a muzzle
	if(!lclient->muzzled && level.time < lclient->muzzle_say_time + chatmuzzle_time->value) {
		lclient->muzzle_says++;
		if(use_chatmuzzle->value && lclient->muzzle_says > chatmuzzle_says->value) {

			// kick if too many muzzles
			lclient->muzzles++;
			if(lclient->muzzles >= chatkick_muzzles->value) {
				gi.bprintf(PRINT_HIGH, ">>> Kicking %s for chatting too much.\n", ent->client->pers.netname);
				gi.AddCommandString(va("kick %d\n", ent - g_edicts - 1));
				return false;
			}

			// muzzle them
			gi.bprintf(PRINT_HIGH, ">>> Muzzling %s for chatting too much.\n", ent->client->pers.netname);
			lclient->muzzled = true;
			lclient->muzzle_time = level.time + chatmuzzle_holdtime->value;
			return false;
		}
	}
	else
		lclient->muzzle_says = 1;
	lclient->muzzle_say_time = level.time;

	// can't chat with a muzzle on
	if(lclient->muzzled) {
		int i, r;
		char buf[512] = "M";
		for(i = 0; i < MAX(strlen(gi.args()), 8); i++) {
			r = random() * 10;
			if(r < 4) strlcat(buf, "m", sizeof(buf));
			else if(r < 7) strlcat(buf, "r", sizeof(buf));
			else strlcat(buf, "h", sizeof(buf));
		}
		strlcat(buf, ".\n", sizeof(buf));
		gi.cprintf(ent, PRINT_HIGH, buf);
		return false;
	}

	return true;
}

extern lvar_t *hook_damage;
extern lvar_t *rune_spawn;

void Lithium_Info(edict_t *ent);

void Lithium_Commands1(edict_t *ent) {
	Menu_Create(ent, 18, 18);
	Menu_Title(ent, "Console Commands");

	Menu_AddLine(ent, MENU_TEXT, 4, "observe", "lc");
	Menu_AddLine(ent, MENU_TEXT, 5, "  Become an observer", "l");
	Menu_AddLine(ent, MENU_TEXT, 6, "chase", "lc");
	Menu_AddLine(ent, MENU_TEXT, 7, "  Chasecam, follow someone", "l");
	Menu_AddLine(ent, MENU_TEXT, 8, "bestweap", "lc");
	Menu_AddLine(ent, MENU_TEXT, 9, "  Toggle best weapon pickup", "l");
	Menu_AddLine(ent, MENU_TEXT, 10, "id", "lc");
	Menu_AddLine(ent, MENU_TEXT, 11, "  Toggle player ID", "l");
	Menu_AddLine(ent, MENU_TEXT, 12, "hud", "lc");
	Menu_AddLine(ent, MENU_TEXT, 13, "  Toggle HUD", "l");
	Menu_AddLine(ent, MENU_TEXT, 14, "menu", "lc");
	Menu_AddLine(ent, MENU_TEXT, 15, "  Lithium II main menu", "l");
	Menu_AddLine(ent, MENU_TEXT, 16, "admin", "lc");
	Menu_AddLine(ent, MENU_TEXT, 17, "  Admin menu", "l");

	Menu_AddLine(ent, MENU_TEXT, 18, "Press Backspace to go back", "m");
	Menu_CancelFunc(ent, Lithium_Info);
}

void Lithium_Commands2(edict_t *ent) {
	Menu_Create(ent, 18, 18);
	Menu_Title(ent, "Commands to Bind");

	Menu_AddLine(ent, MENU_TEXT, 4, "drop rune", "lc");
	Menu_AddLine(ent, MENU_TEXT, 5, "  Drops your current rune", "l");
	Menu_AddLine(ent, MENU_TEXT, 6, "+scores", "lc");
	Menu_AddLine(ent, MENU_TEXT, 7, "  Quake 1 style scoreboard", "l");
	Menu_AddLine(ent, MENU_TEXT, 8, "+hook", "lc");
	Menu_AddLine(ent, MENU_TEXT, 9, "  Grappling hook", "l");

	Menu_AddLine(ent, MENU_TEXT, 18, "Press Backspace to go back", "m");
	Menu_CancelFunc(ent, Lithium_Info);
}

void Lithium_AboutRunes(edict_t *ent) {
	Menu_Create(ent, 18, 18);
	Menu_Title(ent, "About the Runes");
	Menu_AddLine(ent, MENU_TEXT, 4, "The colored pyramid keys", "l");
	Menu_AddLine(ent, MENU_TEXT, 5, "are considered 'runes'. To", "l");
	Menu_AddLine(ent, MENU_TEXT, 6, "use one, just pick it up.", "l");

	Menu_AddLine(ent, MENU_TEXT, 8, "Strength (Red)", "lc");
	Menu_AddLine(ent, MENU_TEXT, 9, "  Inflict 2x damage", "l");
	Menu_AddLine(ent, MENU_TEXT, 10, "Resist (Blue)", "lc");
	Menu_AddLine(ent, MENU_TEXT, 11, "  Take 1/2 damage", "l");
	Menu_AddLine(ent, MENU_TEXT, 12, "Haste (Yellow)", "lc");
	Menu_AddLine(ent, MENU_TEXT, 13, "  Shoot twice as fast", "l");
	Menu_AddLine(ent, MENU_TEXT, 14, "Regen (Green)", "lc");
	Menu_AddLine(ent, MENU_TEXT, 15, "  Regain health slowly", "l");
	Menu_AddLine(ent, MENU_TEXT, 16, "Vampire (Purple)", "lc");
	Menu_AddLine(ent, MENU_TEXT, 17, "  Gain health for damage", "l");

	Menu_AddLine(ent, MENU_TEXT, 18, "Press Backspace to go back", "m");
	Menu_CancelFunc(ent, Lithium_Info);
}

void Lithium_AboutChasecam(edict_t *ent) {
	Menu_Create(ent, 18, 18);
	Menu_Title(ent, "Using the Chasecam");
	Menu_AddLine(ent, MENU_TEXT, 4, "Using your forward and", "l");
	Menu_AddLine(ent, MENU_TEXT, 5, "back controls will change", "l");
	Menu_AddLine(ent, MENU_TEXT, 6, "your camera distance.  Use", "l");
	Menu_AddLine(ent, MENU_TEXT, 7, "your up and down controls", "l");
	Menu_AddLine(ent, MENU_TEXT, 8, "to change the height.", "l");

	Menu_AddLine(ent, MENU_TEXT, 10, "Enter (invuse) toggles", "l");
	Menu_AddLine(ent, MENU_TEXT, 11, "free look, and Backspace", "l");
	Menu_AddLine(ent, MENU_TEXT, 12, "(invdrop) will reset", "l");
	Menu_AddLine(ent, MENU_TEXT, 13, "all chasecam defaults.", "l");

	Menu_AddLine(ent, MENU_TEXT, 18, "Press Backspace to go back", "m");
	Menu_CancelFunc(ent, Lithium_Info);
}

void Lithium_About(edict_t *ent) {
	Menu_Create(ent, 18, 18);
	Menu_Title(ent, "Author Info");

	Menu_AddLine(ent, MENU_TEXT, 4, "Name", "lc");
	Menu_AddLine(ent, MENU_TEXT, 5, "  Matt Ayres", "l");
	Menu_AddLine(ent, MENU_TEXT, 7, "Handle", "lc");
	Menu_AddLine(ent, MENU_TEXT, 8, "  WhiteFang", "l");
	Menu_AddLine(ent, MENU_TEXT, 10, "Email", "lc");
	Menu_AddLine(ent, MENU_TEXT, 11, "  matt@lithium.com", "l");

	Menu_AddLine(ent, MENU_TEXT, 18, "Press Backspace to go back", "m");
	Menu_CancelFunc(ent, Lithium_Info);
}

void Lithium_Info(edict_t *ent) {
	Menu_Create(ent, 4, 15);
	Menu_Title(ent, "Information");

	Menu_AddLine(ent, MENU_FUNC, 0, "Author Info", Lithium_About);
	Menu_AddLine(ent, MENU_FUNC, 0, "Console commands", Lithium_Commands1);
	Menu_AddLine(ent, MENU_FUNC, 0, "Commands to bind", Lithium_Commands2);
	Menu_AddLine(ent, MENU_FUNC, 0, "About the runes", Lithium_AboutRunes);
	Menu_AddLine(ent, MENU_FUNC, 0, "Using the chasecam", Lithium_AboutChasecam);

	Menu_AddLine(ent, MENU_TEXT, 16, "Use [ and ] keys to pick", "m");
	Menu_AddLine(ent, MENU_TEXT, 17, "Press Enter to select", "m");
	Menu_AddLine(ent, MENU_TEXT, 18, "Press Backspace to go back", "m");
	Menu_CancelFunc(ent, Lithium_Menu);
}

void Lithium_Menu(edict_t *ent) {
	Menu_Create(ent, 4, 16);
	Menu_Title(ent, "Main Menu");

	Menu_AddLine(ent, MENU_FUNC, 0, "Information", Lithium_Info);
	if(ctf->value && !ent->client->resp.ctf_team)
		Menu_AddLine(ent, MENU_CMD, 0, "CTF Team Menu", "inventory");
	Menu_AddLine(ent, MENU_CMD, 0, "Observer mode", "menu;observe");
	Menu_AddLine(ent, MENU_CMD, 0, "Chasecam mode", "menu;chase");
	Menu_AddLine(ent, MENU_CMD, 0, "Map vote", "vote");
	Menu_AddLine(ent, MENU_BLANK, 0, "", 0);
	Menu_AddLine(ent, MENU_PVAR, 0, "Best weapon", "bestweap");
	Menu_AddLine(ent, MENU_PVAR, 0, "Player ID", "id_state");
	if(!ctf->value) {
		Menu_AddLine(ent, MENU_PVAR, 0, "HUD layout", "hud");
		Menu_AddLine(ent, MENU_PVAR, 0, "Scores layout", "board");
	}
	Menu_AddLine(ent, MENU_BLANK, 0, "", 0);
	Menu_AddLine(ent, MENU_CMD, 0, "Admin Menu", "admin");

	Menu_AddLine(ent, MENU_TEXT, 17, "Use [ and ] keys to pick", "m");
	Menu_AddLine(ent, MENU_TEXT, 18, "Press Enter to select", "m");
}

void Lithium_CTFCredits(edict_t *ent) {
	Menu_Create(ent, 5, 16);

	Menu_AddLine(ent, MENU_TEXT, 1, "Quake II", "mc");
	Menu_AddLine(ent, MENU_TEXT, 2, "ThreeWave Capture the Flag", "mc");

	Menu_AddLine(ent, MENU_TEXT, 4, "Programming", "mc");
	Menu_AddLine(ent, MENU_TEXT, 5, "Dave 'Zoid' Kirsch", "m");
	Menu_AddLine(ent, MENU_TEXT, 6, "Level Design", "mc");
	Menu_AddLine(ent, MENU_TEXT, 7, "Christian Antkow", "m");
	Menu_AddLine(ent, MENU_TEXT, 8, "Tim Willits", "m");
	Menu_AddLine(ent, MENU_TEXT, 9, "Dave 'Zoid' Kirsch", "m");
	Menu_AddLine(ent, MENU_TEXT, 10, "Art", "mc");
	Menu_AddLine(ent, MENU_TEXT, 11, "Adrian Carmack", "m");
	Menu_AddLine(ent, MENU_TEXT, 12, "Paul Steed", "m");
	Menu_AddLine(ent, MENU_TEXT, 13, "Kevin Cloud", "m");
	Menu_AddLine(ent, MENU_TEXT, 14, "Sound", "mc");
	Menu_AddLine(ent, MENU_TEXT, 15, "Tom 'Bjorn' Klok", "m");
	Menu_AddLine(ent, MENU_TEXT, 16, "Brian 'Whaleboy' Cozzens", "m");
	
	Menu_AddLine(ent, MENU_TEXT, 18, "Press Backspace to go back", "m");

	Menu_CancelFunc(ent, Lithium_CTFMenu);
}

void Lithium_CTFMenu(edict_t *ent) {
	Menu_Create(ent, 5, 16);
	Menu_Offsets(ent, 15, 16);

	Menu_AddLine(ent, MENU_TEXT, 1, "Quake II", "mc");
	Menu_AddLine(ent, MENU_TEXT, 2, "ThreeWave Capture the Flag", "mc");
	Menu_AddLine(ent, MENU_TEXT, 3, lithium_modname, "mc");

	Menu_AddLine(ent, MENU_CMD, 0, "Join Red Team", "menu;team red");
	Menu_AddLine(ent, MENU_CMD, 0, "Join Blue Team", "menu;team blue");
	Menu_AddLine(ent, MENU_BLANK, 0, "", 0);
	Menu_AddLine(ent, MENU_CMD, 0, "Observer mode", "menu;observe");
	Menu_AddLine(ent, MENU_CMD, 0, "Chasecam mode", "menu;chase");
	Menu_AddLine(ent, MENU_FUNC, 0, "CTF Credits", Lithium_CTFCredits);
	Menu_AddLine(ent, MENU_CMD, 0, "Lithium II Menu", "menu;menu");

	Menu_AddLine(ent, MENU_TEXT, 17, "Use [ and ] keys to pick", "m");
	Menu_AddLine(ent, MENU_TEXT, 18, "Press Enter to select", "m");

	Menu_AddLine(ent, MENU_TEXT, 6, team1players, "");
	Menu_AddLine(ent, MENU_TEXT, 8, team2players, "");

	ent->ctf_players_time = level.time + 1.0;

	if(countteam2() < countteam1())
		ent->menu->sel = ent->menu->sel->next;
}

void Lithium_UpdateCTFMenu(edict_t *ent) {
	ent->ctf_players_time = level.time + 1.0;
	ent->menu->changed = true;
	ent->layout_update = true;
}
