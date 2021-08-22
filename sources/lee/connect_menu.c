#include "g_local.h"
void CTFChaseCam(edict_t *ent, pmenu_t *p);

void MPjoin_game(edict_t *ent, pmenu_t *p)
{
	//joined
	ent->client->resp.ctf_team = 1;

	if (strcmp(ent->classname, "player") == 0)
		PMenu_Close(ent);

	ent->svflags &= ~SVF_NOCLIENT;

	if (strcmp(ent->classname, "player") == 0)
	{
		PutClientInServer (ent);

		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
	}

}

pmenu_t MPjoinmenu[] = {
	{ "*Lee's MOD",			PMENU_ALIGN_CENTER, NULL, NULL },
	{ "*Muliplayer join menu",	PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ NULL,					PMENU_ALIGN_CENTER, NULL, NULL },
	{ "Join Game",			PMENU_ALIGN_LEFT, NULL, MPjoin_game },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Chase Camera",		PMENU_ALIGN_LEFT, NULL, CTFChaseCam },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ENTER to select",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ "(TAB to Return)",	PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, NULL, NULL },
	{ NULL,					PMENU_ALIGN_RIGHT,NULL, NULL },
};

qboolean MPStartClient(edict_t *ent)
{
	if (ent->client->resp.ctf_team != CTF_NOTEAM)
		return false;

	// start as 'observer'
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->resp.ctf_team = CTF_NOTEAM;
	ent->client->ps.gunindex = 0;
	gi.linkentity (ent);
	MPOpenJoinMenu(ent);
	return true;

}

int MPUpdateJoinMenu(edict_t *ent)
{
	static char levelname[32];
	static char team1players[32];
	static char botplayers[32];
	int num1, num2, i;

	MPjoinmenu[4].text = "Join Game";
	MPjoinmenu[4].SelectFunc = MPjoin_game;

	if (ent->client->chase_target)
		MPjoinmenu[8].text = "Leave Chase Camera";
	else
		MPjoinmenu[8].text = "Chase Camera";

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

		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			num1++;
	}

	sprintf(team1players, "  (%d players)", num1);
	sprintf(botplayers, "  (%d bots)", current_number_bot);

	MPjoinmenu[2].text = levelname;
	if (MPjoinmenu[4].text)
		MPjoinmenu[5].text = team1players;
	MPjoinmenu[6].text = botplayers;
	
	return 0;
}

void MPOpenJoinMenu(edict_t *ent)
{
	int team;

	team = MPUpdateJoinMenu(ent);
	if (ent->client->chase_target)
		team = 8;

	team = 6;
	
	PMenu_Open(ent, MPjoinmenu, team, sizeof(MPjoinmenu) / sizeof(pmenu_t));
}

