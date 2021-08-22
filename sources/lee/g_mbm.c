/*
Mission Based Multiplayer in functions for QUAKE 2, made by me!
===============================================================
*/

#include "g_local.h"

qboolean mbmStartClient(edict_t *ent)
{
	//check to see if the player has joined a team,
	if (ent->client->resp.ctf_team != mbm_NOTEAM)
		return false;

	// start as 'observer'
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->resp.ctf_team = mbm_NOTEAM;
	ent->client->ps.gunindex = 0;
	gi.linkentity (ent);

	mbmOpenJoinMenu(ent);
	return true;

}

// TEAM STUFF ===========================================

void mbmAssignSkin(edict_t *ent, char *s)
{
	int playernum = ent-g_edicts-1;
	char *p;
	char t[64];

	Com_sprintf(t, sizeof(t), "%s", s);

	if ((p = strrchr(t, '/')) != NULL)
		p[1] = 0;
	else
		strcpy(t, "male/");

	switch (ent->client->resp.ctf_team) {
	case mbm_TEAM1:
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
			ent->client->pers.netname, t, CTF_TEAM1_SKIN) );
		break;
	case mbm_TEAM2:
		gi.configstring (CS_PLAYERSKINS+playernum,
			va("%s\\%s%s", ent->client->pers.netname, t, CTF_TEAM2_SKIN) );
		break;
	default:
		gi.configstring (CS_PLAYERSKINS+playernum, 
			va("%s\\%s", ent->client->pers.netname, s) );
		break;
	}
//	gi.cprintf(ent, PRINT_HIGH, "You have been assigned to %s team.\n", ent->client->pers.netname);
}

void mbmAssignTeam(gclient_t *who)
{
	edict_t		*player;
	int i;
	int team1count = 0, team2count = 0;

	who->resp.ctf_state = CTF_STATE_START;
/*
	if (!((int)dmflags->value & DF_CTF_FORCEJOIN)) {
		who->resp.ctf_team = CTF_NOTEAM;
		return;
	}
*/
	for (i = 1; i <= maxclients->value; i++) {
		player = &g_edicts[i];

		if (!player->inuse || player->client == who)
			continue;

		switch (player->client->resp.ctf_team) {
		case mbm_TEAM1:
			team1count++;
			break;
		case mbm_TEAM2:
			team2count++;
		}
	}
	if (team1count < team2count)
		who->resp.ctf_team = mbm_TEAM1;
	else if (team2count < team1count)
		who->resp.ctf_team = mbm_TEAM2;
	else if (rand() & 1)
		who->resp.ctf_team = mbm_TEAM1;
	else
		who->resp.ctf_team = mbm_TEAM2;
}

char *mbmTeamName(int team)
{
	switch (team) {
	case mbm_TEAM1:
		return "RED";
	case mbm_TEAM2:
		return "BLUE";
	}
	return "UNKNOWN";
}

void mbmJoinTeam(edict_t *ent, int desired_team)
{
	char *s;

	if (strcmp(ent->classname, "player") == 0)
		PMenu_Close(ent);

	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = CTF_STATE_START;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	mbmAssignSkin(ent, s);
	
	if (strcmp(ent->classname, "bot") != 0)
	{
		PutClientInServer (ent);

		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
	}

	gi.bprintf(PRINT_HIGH, "%s joined the %s team.\n",
		ent->client->pers.netname, mbmTeamName(desired_team));
}

void mbmJoinTeam1(edict_t *ent, pmenu_t *p)
{
	mbmJoinTeam(ent, mbm_TEAM1);
}

void mbmJoinTeam2(edict_t *ent, pmenu_t *p)
{
	mbmJoinTeam(ent, mbm_TEAM2);
}



//==================================================================
//menu
//==================================================================

pmenu_t mbmjoinmenu[] = {
	{ "*Lee's MOD",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Mission Based Multiplayer",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Join Attackers",		PMENU_ALIGN_LEFT, NULL, mbmJoinTeam1 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Join Defenders",		PMENU_ALIGN_LEFT, NULL, mbmJoinTeam2 },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Chase Camera",		PMENU_ALIGN_LEFT, NULL, mbmChaseCam },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "v0.01" ,	PMENU_ALIGN_RIGHT, NULL, NULL },
};

int mbmUpdateJoinMenu(edict_t *ent)
{
	static char levelname[32];
	static char team1players[32];
	static char team2players[32];
	int num1, num2, i;

	mbmjoinmenu[4].text = "Join Attackers";
	mbmjoinmenu[4].SelectFunc = mbmJoinTeam1;
	mbmjoinmenu[6].text = "Join Defenders";
	mbmjoinmenu[6].SelectFunc = mbmJoinTeam2;

/*	if (ctf_forcejoin->string && *ctf_forcejoin->string) {
		if (Q_stricmp(ctf_forcejoin->string, "red") == 0) {
			joinmenu[6].text = NULL;
			joinmenu[6].SelectFunc = NULL;
		} else if (Q_stricmp(ctf_forcejoin->string, "blue") == 0) {
			joinmenu[4].text = NULL;
			joinmenu[4].SelectFunc = NULL;
		}
	}
*/
	if (ent->client->chase_target)
		mbmjoinmenu[8].text = "Leave Chase Camera";
	else
		mbmjoinmenu[8].text = "Chase Camera";

	levelname[0] = '*';
	if (g_edicts[0].message)
		strncpy(levelname+1, g_edicts[0].message, sizeof(levelname) - 2);
	else
		strncpy(levelname+1, level.mapname, sizeof(levelname) - 2);
	levelname[sizeof(levelname) - 1] = 0;

	num1 = num2 = 0;
	for (i = 0; i < maxclients->value; i++) {
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == mbm_TEAM1)
			num1++;
		else if (game.clients[i].resp.ctf_team == mbm_TEAM2)
			num2++;
	}

	sprintf(team1players, "  (%d players)", num1);
	sprintf(team2players, "  (%d players)", num2);

	mbmjoinmenu[2].text = levelname;
	if (mbmjoinmenu[4].text)
		mbmjoinmenu[5].text = team1players;
	else
		mbmjoinmenu[5].text = NULL;
	if (mbmjoinmenu[6].text)
		mbmjoinmenu[7].text = team2players;
	else
		mbmjoinmenu[7].text = NULL;
	
	if (num1 > num2)
		return mbm_TEAM1;
	else if (num2 > num1)
		return mbm_TEAM1;
	return (rand() & 1) ? mbm_TEAM1 : mbm_TEAM2;
}

void mbmOpenJoinMenu(edict_t *ent)
{
	int team;

	team = mbmUpdateJoinMenu(ent);
	if (ent->client->chase_target)
		team = 8;
	else if (team == mbm_TEAM1)
		team = 4;
	else
		team = 6;
	
	PMenu_Open(ent, mbmjoinmenu, team, sizeof(mbmjoinmenu) / sizeof(pmenu_t));
}

void mbmChaseCam(edict_t *ent, pmenu_t *p)
{
	int i;
	edict_t *e;

	if (ent->client->chase_target) {
		ent->client->chase_target = NULL;
		PMenu_Close(ent);
		return;
	}

	for (i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;
		if (e->inuse && e->solid != SOLID_NOT) {
			ent->client->chase_target = e;
			PMenu_Close(ent);
			ent->client->update_chase = true;
			break;
		}
	}
}

void mbm_complete(edict_t *ent)
{
	gi.dprintf ("One mission compelte..\n");

	if (level.mbm_objectives > 0)
		level.mbm_objectives--;
}

