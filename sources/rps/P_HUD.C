#include "g_local.h"
#include "q_devels.h"


/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	ent->client->showscores = true;
	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
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

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout

	DeathmatchScoreboardMessage (ent, NULL);
	gi.unicast (ent, true);
}

void BeginIntermission (edict_t *targ)
{
	int		i;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

	game.autosaved = false;

	// respawn any dead clients
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;

		if (client->health <= 0)
			respawn(client);
	}

	level.intermissiontime = level.time;
	level.changemap = targ->map;

	level.exitintermission = 0;

	// find an intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission (client);
	}
}

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	const int INDENT=72; //how much to indent lines (9 spaces).
	const int SCORES=INDENT+128; //16 characters later.  (15 for name, 1 for space)
	const int TIME=SCORES+48; //6 characters later. (5 for frags, 1 for space)
	const int PING=TIME+40;  
	int row=60; //row placements.
	char    *name;
	char *teamname; // MIKE
	char	entry[1024];
	char	string[1400];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int		i, j, k,index;
	int     order[3];
	int		sorted[3][MAX_CLIENTS];
	int		sortedscores[3][MAX_CLIENTS];
	int		score, total[3], totalscore[3];
	int team;
	int maxsize = 1300;
 	int first=1;

	// sort the clients by team and score
	total[0] = total[1] = total[2] = 0;
	totalscore[0] = totalscore[1] = totalscore[2] = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

		if (game.clients[i].resp.team == ROCK)
			team = 0;
		else if (game.clients[i].resp.team == PAPER)
			team = 1;
		else if (game.clients[i].resp.team == SCISSORS)
			team = 2;
		else
			continue; // unknown team?

		score = game.clients[i].resp.score;
		for (j=0 ; j<total[team] ; j++)
		{
			if (score > sortedscores[team][j])
				break;
		}
		for (k=total[team] ; k>j ; k--)
		{
			sorted[team][k] = sorted[team][k-1];
			sortedscores[team][k] = sortedscores[team][k-1];
		}
		sorted[team][j] = i;
		total[team]++;
	}

	*string = 0;
	*entry = 0;

	for(i=0;i<3;++i)
		order[i]=i;
/*		order[i]=-1;

	if(RockScore>=PaperScore && RockScore>=ScissorsScore)
		order[0]=0;
	else if(RockScore>=PaperScore || RockScore>=ScissorsScore)
		order[1]=0;
	else
		order[2]=0;

	if(order[0]==-1 && PaperScore>=ScissorsScore)
		order[0]=1;
	else if(order[1]==-1 && PaperScore>=ScissorsScore)
		order[1]=1;
	else
		order[2]=1;

	for(i=0;i<3;++i)
		if(order[i]==-1)
			order[i]=2;
*/
	sprintf(string+strlen(string),"xl 0 yt %i string \"Team     Name            Frags Time Ping\" yt %i string \"-------- --------------- ----- ---- ----\"",
			row,row+8);
	row+=16;

	for(i=0;i<3;++i)
		if(total[order[i]])
		{
			*entry = 0;

			switch(order[i])
			{
			case 0: teamname="ROCK"; break;
			case 1: teamname="PAPER"; break;
			default:teamname="SCISSORS"; break;
			}

			sprintf(entry+strlen(entry), "xl 0 yt %d string \"%s\"",
				row, teamname);

			for(index=0;index<total[order[i]];++index)
			{
				cl = &game.clients[sorted[order[i]][index]];
				cl_ent = g_edicts + 1 + sorted[order[i]][index];

				name=game.clients[sorted[order[i]][index]].pers.netname;

				sprintf(entry+strlen(entry),
					"xl %i yt %i %s \"%s\" xl %i %s %i xl %i %s %i xl %i %s %i ",
					INDENT,
					row,
					(cl_ent == ent || cl_ent == killer) ? "string2" : "string",//make so that killer's name only appears green...
					name, 
					SCORES,
					(cl_ent == ent) ? "string2" : "string", 
					cl->resp.score,
					TIME,
					(cl_ent==ent) ? "string2" : "string",
					(level.framenum - cl->resp.enterframe)/600,
					PING,
					(cl_ent == ent) ? "string2" : "string", 
					cl->ping > 999 ? 999 : cl->ping);

				row+=8;

				if(strlen(string)+strlen(entry)<=maxsize)
					strcat(string, entry);
			}

			row+=8;
		}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard (edict_t *ent)
{
	DeathmatchScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f (edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		return;
	}

	ent->client->showscores = true;
	DeathmatchScoreboard (ent);
}

/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent)
{
	Cmd_Score_f (ent);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t *ent)
{
	gitem_t		*item;
	int			index, cells;
	int			power_armor_type;

	//
	// health
	//
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	//
	// ammo
	//
	if (!ent->client->ammo_index /* || !ent->client->pers.inventory[ent->client->ammo_index] */)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}
	
	//
	// armor
	//
	power_armor_type = PowerArmorType (ent);
	if (power_armor_type)
	{
		cells = ent->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];
		if (cells == 0)
		{	// ran out of cells for power armor
			ent->flags &= ~FL_POWER_ARMOR;
			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;;
		}
	}

	index = ArmorIndex (ent);
	if (power_armor_type && (!index || (level.framenum & 8) ) )
	{	// flash between power armor and other armor icon
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powershield");
		ent->client->ps.stats[STAT_ARMOR] = cells;
	}
	else if (index)
	{
		item = GetItemByIndex (index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
	}
	else
	{
		ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
		ent->client->ps.stats[STAT_ARMOR] = 0;
	}

	//
	// pickup message
	//
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	//
	// timers
	//
	if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quad");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum)/10;
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum)/10;
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_envirosuit");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum)/10;
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_rebreather");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum)/10;
	}
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
	}

	//
	// selected item
	//
	if (ent->client->pers.selected_item == -1)
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (itemlist[ent->client->pers.selected_item].icon);

	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (ent->client->pers.health <= 0 || level.intermissiontime
		|| ent->client->showscores)
		ent->client->ps.stats[STAT_LAYOUTS] |= 1;
	if (ent->client->showinventory && ent->client->pers.health > 0)
		ent->client->ps.stats[STAT_LAYOUTS] |= 2;

	//
	// frags
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	/* MIKE */

	//
	// flags (In RPS, the cubes are still refered to as flags in the code.  Just ignore it.)
	//
	ent->client->ps.stats[STAT_MYFLAGS]=ent->client->pers.inventory[ITEMLIST_FLAG];
	ent->client->ps.stats[STAT_ROCKFLAGS] = FlagsOnTeam(ROCK);
	ent->client->ps.stats[STAT_PAPERFLAGS] = FlagsOnTeam(PAPER);
	ent->client->ps.stats[STAT_SCISSORSFLAGS] = FlagsOnTeam(SCISSORS);
	ent->client->ps.stats[STAT_ROCKSCORE] = RockScore;
	ent->client->ps.stats[STAT_PAPERSCORE] = PaperScore;
	ent->client->ps.stats[STAT_SCISSORSSCORE] = ScissorsScore;

	//God, I hate this.... There must be a better way of doing team.
	if(ent->client->resp.team==ROCK)
		ent->client->ps.stats[STAT_ISROCK]=1;
	else
		ent->client->ps.stats[STAT_ISROCK]=0;

	if(ent->client->resp.team==PAPER)
		ent->client->ps.stats[STAT_ISPAPER]=1;
	else
		ent->client->ps.stats[STAT_ISPAPER]=0;
	
	if(ent->client->resp.team==SCISSORS)
		ent->client->ps.stats[STAT_ISSCISSORS]=1;
	else
		ent->client->ps.stats[STAT_ISSCISSORS]=0;

	/* MIKE */

	//
	// help icon / current weapon if not shown
	//
	if (ent->client->pers.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)	&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

	ent->client->ps.stats[STAT_SPECTATOR] = 0;
}

/*
===============
G_CheckChaseStats
===============
*/
void G_CheckChaseStats (edict_t *ent)
{
	int i;
	gclient_t *cl;

	for (i = 1; i <= maxclients->value; i++) {
		cl = g_edicts[i].client;
		if (!g_edicts[i].inuse || cl->chase_target != ent)
			continue;
		memcpy(cl->ps.stats, ent->client->ps.stats, sizeof(cl->ps.stats));
		G_SetSpectatorStats(g_edicts + i);
	}
}

/*
===============
G_SetSpectatorStats
===============
*/
void G_SetSpectatorStats (edict_t *ent)
{
	gclient_t *cl = ent->client;

	if (!cl->chase_target)
		G_SetStats (ent);

	cl->ps.stats[STAT_SPECTATOR] = 1;

	// layouts are independant in spectator
	cl->ps.stats[STAT_LAYOUTS] = 0;
	if (cl->pers.health <= 0 || level.intermissiontime || cl->showscores)
		cl->ps.stats[STAT_LAYOUTS] |= 1;
	if (cl->showinventory && cl->pers.health > 0)
		cl->ps.stats[STAT_LAYOUTS] |= 2;

	if (cl->chase_target && cl->chase_target->inuse)
		cl->ps.stats[STAT_CHASE] = CS_PLAYERSKINS + 
			(cl->chase_target - g_edicts) - 1;
	else
		cl->ps.stats[STAT_CHASE] = 0;
}

