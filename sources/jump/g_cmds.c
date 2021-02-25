#include "g_local.h"
#include "m_player.h"


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gi.cprintf (ent, PRINT_HIGH, msg);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = 0;
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;

	i = atoi (gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		gi.cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		gi.cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		gi.cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		gi.cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;

	if (gi.argc () < 2 && !arg0)
		return;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	strcat(text, "\n");

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		/*if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}*/
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
}

void Cmd_Checkpoint_f (edict_t *ent)
{
	if (!sv_maxcheckpoints->value) {
		gi.cprintf (ent, PRINT_HIGH, "Checkpoints are disabled on this server.\n");
		return;
	}

	if (level.nochex) {
		gi.cprintf (ent, PRINT_HIGH, "Checkpoints have been disabled on this map.\n");
		return;
	}

	if (ent->client->pers.spectator) {
		gi.cprintf (ent, PRINT_HIGH, "Cannot set checkpoints from spectator mode.\n");
		return;
	}

	if (ent->client->resp.checkpoint >= (int)sv_maxcheckpoints->value) {
		gi.cprintf (ent, PRINT_HIGH, "You have already used all %d checkpoints this map.\n", (int)sv_maxcheckpoints->value);
		return;
	}

	if (ent->velocity[0] || ent->velocity[1] || ent->velocity[2]) {
		gi.cprintf (ent, PRINT_HIGH, "You must be standing still to set a checkpoint.\n");
		return;
	}

	ent->client->resp.checkpoint++;

	VectorCopy (ent->s.origin, ent->client->resp.checkpoint_origin);
	VectorCopy (ent->s.angles, ent->client->resp.checkpoint_angles);

	gi.cprintf (ent, PRINT_HIGH, "Checkpoint %d/%d deployed at %s\n", ent->client->resp.checkpoint, (int)sv_maxcheckpoints->value, vtos (ent->s.origin));
}

void Cmd_Nochecks_f (edict_t *ent)
{
	if (!sv_maxcheckpoints->value) {
		gi.cprintf (ent, PRINT_HIGH, "Checkpoints are disabled on this server.\n");
		return;
	}

	if (!level.nochex) {
		level.nochex = 1;
		gi.bprintf (PRINT_HIGH, "%s disabled checkpoints for this map.\n", ent->client->pers.netname);
	} else {
		gi.cprintf (ent, PRINT_HIGH, "Checkpoints are already off on this map.\n");
	}
}

void Cmd_Killcp_f (edict_t *ent)
{
	if (!sv_maxcheckpoints->value) {
		gi.cprintf (ent, PRINT_HIGH, "Checkpoints are disabled on this server.\n");
		return;
	}

	if (!ent->client->resp.checkpoint) {
		gi.cprintf (ent, PRINT_HIGH, "You don't have any checkpoints to kill.\n");
		return;
	}

	ent->client->resp.checkpoint = 0;
	Cmd_Kill_f (ent);
}

void Cmd_Ghostmode_f (edict_t *ent)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};

	if (!ent->client->pers.spectator) {
		gi.cprintf (ent, PRINT_HIGH, "You can only ghost join from spectator mode.\n");
		return;
	}

	if (ent->movetype == MOVETYPE_WALK) {
		gi.cprintf (ent, PRINT_HIGH, "You have exited ghost mode.\n");
		PutClientInServer (ent);
		return;
	}

	// clear entity values
	ent->health	= ent->max_health = 250;
	ent->groundentity = NULL;
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags = SVF_NOCLIENT;
	ent->s.modelindex = 0;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);

	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(ent->client->ps));

	gi.cprintf (ent, PRINT_HIGH, "You have entered ghost mode.\n");
}

void Cmd_Showents_f (edict_t *ent)
{
	char message[1200];
	edict_t *from = NULL;
	
	message[0] = '\0';

	while ((from = findradius (from, ent->s.origin, 200)) != NULL) {
		if (from->classname) {
			strcat (message, va("%s [%s], ", from->classname, vtos(from->s.origin)));
		} else {
			strcat (message, "unknown ent!");
		}
		if (strlen(message) > 900)
			break;
	}

	gi.cprintf (ent, PRINT_HIGH, "%s\n", message);
}

void Cmd_Votemap_f (edict_t *ent) {
	char	*map = gi.argv(1);
	FILE	*mapfile = NULL;
	char	*filename;
	int		i;

	if (!sv_voting->value) {
		gi.cprintf (ent, PRINT_HIGH, "Map voting is disabled.\n");
		return;
	} else if (level.vote) {
		gi.cprintf (ent, PRINT_HIGH, "There is already a vote in progress! Wait for it to end first.\n");
		return;
	} else if (!*map) {
		gi.cprintf (ent, PRINT_HIGH, "Usage: votemap <mapname>\n");
		return;
	} else if (level.votetimer && (level.votetimer + 600) > level.framenum) {
		gi.cprintf (ent, PRINT_HIGH, "You must wait a short while before another vote can be started.\n");
		return;
	} else if (!Q_stricmp (map, level.mapname)) {
		gi.cprintf (ent, PRINT_HIGH, "You can't vote for the map that is currently being played!\n");
		return;
	} else if (strstr (map, ".") || strstr (map, "\\") || strstr (map, "/") || strstr (map, " ")) {
		gi.cprintf (ent, PRINT_HIGH, "Invalid filename: %s\n", map);
	}

	filename = va("jump/maps/%s.bsp", map);

	mapfile = fopen (filename, "r");

	if (!mapfile) {
		gi.cprintf (ent, PRINT_HIGH, "%s.bsp doesn't exist on the server!\n", map);
		return;
	} else {
		fclose (mapfile);
		gi.bprintf (PRINT_HIGH, "%s started a vote to change the map to %s.\nType YES or NO in the console to vote.\n", ent->client->pers.netname, map);
		level.vote = true;
		level.votetimer = level.framenum + 300;
		strncpy (level.votemap, map, sizeof(level.votemap)-1);
		for (i = 0; i < maxclients->value; i++) {
			game.clients[i].resp.vote = VOTE_INVALID;
		}
		ent->client->resp.vote = VOTE_YES;
	}
}

void Cmd_Vote (edict_t *ent, int value)
{
	if (!level.vote) {
		gi.cprintf (ent, PRINT_HIGH, "No vote is running! Use votemap <mapname> to start a vote.\n");
		return;
	}
	ent->client->resp.vote = value;
	gi.cprintf (ent, PRINT_HIGH, "You have voted: %s\n", value == VOTE_YES ? "YES" : "NO");
}

/*
=================
ClientCommand
=================
*/
void DeathmatchScoreboard (edict_t *ent);
void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;		// not fully in game yet

	cmd = gi.argv(0);

	if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}

	if (level.intermissiontime)
		return;

	if (Q_stricmp (cmd, "use") == 0)
		return;
	else if (Q_stricmp (cmd, "yes") == 0)
		Cmd_Vote (ent, VOTE_YES);
	else if (Q_stricmp (cmd, "no") == 0)
		Cmd_Vote (ent, VOTE_NO);
	else if (Q_stricmp (cmd, "votemap") == 0)
		Cmd_Votemap_f (ent);
	else if (Q_stricmp (cmd, "showents") == 0)
		Cmd_Showents_f (ent);
	else if (Q_stricmp (cmd, "checkpoint") == 0)
		Cmd_Checkpoint_f (ent);
	else if (Q_stricmp (cmd, "killcp") == 0)
		Cmd_Killcp_f (ent);
	else if (Q_stricmp (cmd, "nochecks") == 0)
		Cmd_Nochecks_f (ent);
	else if (Q_stricmp (cmd, "ghostmode") == 0)
		Cmd_Ghostmode_f (ent);
	else if (Q_stricmp (cmd, "drop") == 0)
		return;
	else if (Q_stricmp (cmd, "god") == 0)
		return;
	else if (Q_stricmp (cmd, "notarget") == 0)
		return;
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "inven") == 0)
		return;
	else if (Q_stricmp (cmd, "invnext") == 0) {
		if (ent->client->showscores == 1) {
			ent->client->showscores = 2;
			DeathmatchScoreboard (ent);
		} else if (ent->client->chase_target) {
			ChaseNext(ent);
		}
		return;
	}
	else if (Q_stricmp (cmd, "invprev") == 0) {
		if (ent->client->showscores == 2) {
			ent->client->showscores = 1;
			DeathmatchScoreboard (ent);
		} else if (ent->client->chase_target) {
			ChasePrev(ent);
		}
		return;
	}
	else if (Q_stricmp (cmd, "coords") == 0)
		gi.cprintf (ent, PRINT_HIGH, "%s\n", vtos(ent->s.origin));
	else if (Q_stricmp (cmd, "invnextw") == 0)
		return;
	else if (Q_stricmp (cmd, "invprevw") == 0)
		return;
	else if (Q_stricmp (cmd, "invnextp") == 0)
		return;
	else if (Q_stricmp (cmd, "invprevp") == 0)
		return;
	else if (Q_stricmp (cmd, "invuse") == 0)
		return;
	else if (Q_stricmp (cmd, "invdrop") == 0)
		return;
	else if (Q_stricmp (cmd, "weapprev") == 0)
		return;
	else if (Q_stricmp (cmd, "weapnext") == 0)
		return;
	else if (Q_stricmp (cmd, "weaplast") == 0)
		return;
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	else if (Q_stricmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	else	// anything that doesn't match a command will be a chat
		//Cmd_Say_f (ent, false, true);
		gi.cprintf (ent, PRINT_HIGH, "Unknown client command: %s\n", cmd);
}
