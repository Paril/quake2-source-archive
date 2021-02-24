#include "g_local.h"
#include "z_gq.h"

/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	int	streak=ent->client->resp.standard[GSTAT_KILLS] - ent->client->resp.lastkills;

	// Antibot
	if (ent->flags & FL_ANTIBOT)
		return;

	if (streak > ent->client->resp.standard[GSTAT_STREAK])
		ent->client->resp.standard[GSTAT_STREAK]=streak;

	if (deathmatch->value || coop->value)
		ent->client->showscores = true;
	// Clear teams if enabled
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
	ent->s.modelindex4 = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout

	if (deathmatch->value || coop->value)
	{
		if (matchstats) {
			GQ_MatchStats(ent, true);
			debugmsg("GQ_MatchStats true\n");
		} else {
			DeathmatchScoreboard (ent, true);
			debugmsg("DeathmatchScoreboard true\n");
		}
//		gi.unicast (ent, true);
	}

}

void BeginIntermission (edict_t *targ)
{
	int		i, n;
	edict_t	*ent, *client;
	char	soundstr[30];

	if (level.intermissiontime)
		return;		// already activated

	// Antibot
	if (targ->flags & FL_ANTIBOT)
		return;

	game.autosaved = false;

	// respawn any dead clients
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (ent->health <= 0)
			respawn(ent);
	}

	level.intermissiontime = level.time;
	level.changemap = targ->map;
	matchstats=true;
	intermissionframenum = level.framenum;

	if (strstr(level.changemap, "*"))
	{
		if (coop->value)
		{
			for (i=0 ; i<maxclients->value ; i++)
			{
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
				// strip players of all keys between units
				for (n = 0; n < MAX_ITEMS; n++)
				{
					if (itemlist[n].flags & IT_KEY)
						client->client->pers.inventory[n] = 0;
				}
			}
		}
	}
	else
	{
		if (!deathmatch->value)
		{
			level.exitintermission = 1;		// go immediately to the next level
			return;
		}
	}

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
	sprintf(soundstr, "gunslinger/waawaa%i.wav", (rand()%3)+1);
	gi.sound(ent, CHAN_AUTO, gi.soundindex(soundstr), 1, ATTN_NONE, 0);
}


/*
==================
DeathmatchScoreboardMessage

==================
*/

void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	string[1400];
	int		i;
	int		total;
	int		accuracy;
	int		picnum;
	int		x, y;
	gclient_t	*cl;
	char	*tag;
	char	tagchar;

	// sort the clients by score
	total = 0;
	while (playerlist[total]>-1)
		total++;
	// See G_RunFrame in main.c for player sort routine -- Stone
	string[0] = 0;

	// add the clients in sorted order
//	if (total>12) {
	if (1) {
		Com_sprintf(string, sizeof(string),
			"xv 8 yv 32 string \"Player          Score Ping Acc  KPH\" "
			"xv 8 yv 40 string \"%s\" ", GQ_TextBar(35));

	}
	if (total>20)
		total = 20;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[playerlist[i]];
		picnum = gi.imageindex ("i_fixme");
//		if (total>12) {
		if (1) {
			x = 32; 
			y = 48 + 8 * i; 
			// Pick "dogtag" image
			if ((cl->team==2) && ((int)playmode->value == PM_BADGE_WARS))
				tagchar = (char)157+128;
			else if (cl == ent->client) 
				tagchar = (char)156+128;
			else if (killer && (cl == killer->client)) 
				tagchar = (char)159+128;
			else 
				tagchar = ' ';

			if (cl->resp.standard[GSTAT_SHOTS]==0)
				accuracy=0;
			else
				accuracy=(int)((cl->resp.standard[GSTAT_HITS]*100)/cl->resp.standard[GSTAT_SHOTS]);
			RPS_AddToString(string,
				va("xv 0 yv %i string2 \"%c%s\" "
					"xv 136 string \"%-5i %-4i %i%c\" "
					"xv 264 string \"%i\" ",
					y, tagchar, cl->pers.netname,
					cl->resp.score,
					cl->ping,
					accuracy, '%',
					cl->ps.stats[STAT_TEAMSCORE_KPH]),
				sizeof(string));
		} else {
			x = (i>=6) ? 160 : 0;
			y = 32 + 32 * (i%6);
			
			// add a dogtag
			if ((cl->quad_framenum > level.framenum) && ((int)playmode->value == PM_BADGE_WARS))
				tag = "tag3";
			else if (cl == ent->client)
				tag = "tag1";
			else if (killer && (cl == killer->client))
				tag = "tag2";
			else
				tag = NULL;
			if (tag)
				RPS_AddToString(string, va("xv %i yv %i picn %s ",x+32, y, tag), sizeof(string));
			
			// send the layout
			RPS_AddToString(string,
				va("client %i %i %i %i %i %i ", x, y, playerlist[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe)/600),
				sizeof(string));
		}
	}
	// Show a countdown if there's a time limit
	if (timelimit->value && !level.intermissiontime){
		int		min, sec;
		float	clock;
		clock = (timelimit->value*60) - level.time;
		min = (int)(clock / 60);
		sec = (int)clock % 60;
//		debugmsg("%2i:%02i\n", min, sec);
		RPS_AddToString(string,
			va("xv 140 yv 24 string \"%2i:%02i\" ", min, sec),
			sizeof(string));
	}
//	debugmsg("%s (%i)\n", string, strlen(string));
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

void TeamplayScoreboardMessage(edict_t *ent, edict_t *killer)
{
	char	string[1000];
	int		i, k;
	int		total, max, count;
	int		x, y;
	int		index;
	gclient_t	*cl;
	char	tempstr[50];
	char	tag;

	if (ent->flags & FL_ANTIBOT)
		return;

	total = 0;

	// print level name and exit rules
	string[0] = 0;

	for (k=1; k<=(int)number_of_teams->value; k++) {
		if ((int)number_of_teams->value==2) {
			max=10;
			switch (k) {
			case 1: x=0; y=32; break;
			case 2: x=160; y=32; break;
			}
		} else if ((int)number_of_teams->value==3) {
			max=6;
			switch (k) {
			case 1: x=0; y=32; break;
			case 2: x=160; y=32; break;
			case 3: x=80; y=116; break;
			}
		} else if ((int)number_of_teams->value==4) {
			max=4;
			switch (k) {
			case 1: x=0; y=32; break;
			case 2: x=160; y=32; break;
			case 3: x=0; y=116; break;
			case 4: x=160; y=116; break;
			}
		}
		y+=4;
		// Display this team's name
		sprintf(tempstr, "%-13s%3i", va("%.12s:",teamdata[k-1].name->string), (int)teamdata[k-1].score);
		RPS_AddToString(string, 
			va("xv %i yv %i picn team%ititle xv %i yv %i string \"%s\" ", x+8, y-4, k, x+16, y, tempstr),
			sizeof(string));
		y+=12;
		count=0;
		for (i=0 ; (playerlist[i]>-1) && (count<max) ; i++) {
			cl = &game.clients[playerlist[i]];
			if (cl->team != k)
				continue;

			count++;
			index=ITEM_INDEX(FindItem("Money Bag"));
			if (cl == ent->client)
				tag=(char)156-128;
			else if ((cl->team==2) && ((int)playmode->value == PM_BADGE_WARS))
				tag=(char)157-128;
			else if (((int)playmode->value==PM_BIG_HEIST) && (cl->pers.inventory[index]))
				tag=(char)142-128;
			else if (killer && (cl == killer->client)) 
				tag=(char)159-128;
			else 
				tag=' ';
			RPS_AddToString(string, 
				va("xv %i yv %i string2 \"%c%-10.10s %-3i %i\" ",
				x, y,
				tag,
				cl->pers.netname,
				cl->resp.score, 
				(cl->ping > 999) ? 999 : cl->ping
				),
				sizeof(string));
			y+=8;
		}
	}
	// Show a countdown if there's a time limit
	if (timelimit->value && !level.intermissiontime){
		int		min, sec;
		float	clock;
		clock = (timelimit->value*60) - level.time;
		min = (int)(clock / 60);
		sec = (int)clock % 60;
//		debugmsg("%2i:%02i\n", min, sec);
		RPS_AddToString(string,
			va("xv 140 yv 24 string \"%2i:%02i\" ", min, sec),
			sizeof(string));
	}
//	debugmsg("%s (%i)\n", string, strlen(string));
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

void BHScoreboardMessage (edict_t *ent, edict_t *killer)
{
	TeamplayScoreboardMessage(ent, killer);
}

void CTFScoreboardMessage (edict_t *ent, edict_t *killer)
{
	DeathmatchScoreboardMessage(ent, killer);
}

void BWScoreboardMessage (edict_t *ent, edict_t *killer)
{
	DeathmatchScoreboardMessage(ent, killer);
}

void KOTHScoreboardMessage (edict_t *ent, edict_t *killer)
{
	DeathmatchScoreboardMessage(ent, killer);
}

void LMSFFAScoreboard (edict_t *ent, edict_t *killer)
{
	char	string[1400];
	int		i;
	int		total;
	int		accuracy;
	int		picnum;
	int		x, y;
	gclient_t	*cl;
	char	tag;

	// sort the clients by score
	total = 0;
	while (playerlist[total]>-1)
		total++;
	// See G_RunFrame in main.c for player sort routine -- Stone
	string[0] = 0;

	// add the clients in sorted order
//	if (total>12) {
	if (1) {
		Com_sprintf(string, sizeof(string),
			"xv 12 yv 32 string \"Player          Score Ping Acc\" "
			"xv 12 yv 40 string \"%s\" ", GQ_TextBar(30));

	}
	if (total>20)
		total = 20;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[playerlist[i]];
		picnum = gi.imageindex ("i_fixme");
		x = 32; 
		y = 48 + 8 * i; 
		// Pick "dogtag" image
		if (cl == ent->client) 
			tag = (char)156-128;
		else if (cl->resp.standard[GSTAT_DEATHS])
			tag=(char)158-128;
		else if (killer && (cl == killer->client)) 
			tag = (char)159-128;
		else 
			tag = ' ';
		
		if (cl->resp.standard[GSTAT_SHOTS]==0)
			accuracy=0;
		else
			accuracy=(int)((cl->resp.standard[GSTAT_HITS]*100)/cl->resp.standard[GSTAT_SHOTS]);
		RPS_AddToString(string,
			va("xv 4 yv %i string2 \"%c%s\" "
			"xv 140 string \"%-5i %-4i %i%c\" ",
			y, tag, cl->pers.netname,
			cl->resp.score,
			cl->ping,
			accuracy, '%'),
			sizeof(string));
	}
	// Show a countdown if there's a time limit or if it's preptime
	if ((timelimit->value && !level.intermissiontime) || (preptimeframe>level.framenum))  {
		int		min, sec;
		float	clock;
		if (preptimeframe>level.framenum) {
			clock = (preptimeframe - level.framenum)/10;
		} else {
			clock = (timelimit->value*60) - (level.time - (preptimeframe/10));
		}
		min = (int)(clock / 60);
		sec = (int)clock % 60;
//		debugmsg("%2i:%02i\n", min, sec);
		RPS_AddToString(string,
			va("xv 140 yv 24 string \"%2i:%02i\" ", min, sec),
			sizeof(string));
	}
//	debugmsg("%s (%i)\n", string, strlen(string));
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

void LMSTeamplayScoreboard (edict_t *ent, edict_t *killer)
{
	char	string[1000];
	int		i, k;
	int		total, max, count;
	int		x, y;
	gclient_t	*cl;
	char	tempstr[50];
	byte	tag;

	total = 0;

	// print level name and exit rules
	string[0] = 0;

	for (k=1; k<=(int)number_of_teams->value; k++) {
		if ((int)number_of_teams->value==2) {
			max=10;
			switch (k) {
			case 1: x=0; y=32; break;
			case 2: x=160; y=32; break;
			}
		} else if ((int)number_of_teams->value==3) {
			max=6;
			switch (k) {
			case 1: x=0; y=32; break;
			case 2: x=160; y=32; break;
			case 3: x=80; y=116; break;
			}
		} else if ((int)number_of_teams->value==4) {
			max=4;
			switch (k) {
			case 1: x=0; y=32; break;
			case 2: x=160; y=32; break;
			case 3: x=0; y=116; break;
			case 4: x=160; y=116; break;
			}
		}
		y+=4;
		// Display this team's name
		sprintf(tempstr, "%-12s%3i", va("%.12s:",teamdata[k-1].name->string), (int)teamdata[k-1].score);
		RPS_AddToString(string, 
			va("xv %i yv %i picn team%ititle xv %i yv %i string \"%s\" ", x+8, y-4, k, x+16, y, tempstr),
			sizeof(string));
		y+=12;
		count=0;
		for (i=0 ; (playerlist[i]>-1) && (count<max) ; i++) {
			cl = &game.clients[playerlist[i]];
			if (cl->team != k)
				continue;

			count++;
			if (cl == ent->client) 
				tag=(char)156-128;
			else if (cl->resp.standard[GSTAT_DEATHS])
				tag=(char)158-128;
			else if (killer && (cl == killer->client)) 
				tag=(char)159-128;
			else 
				tag=' ';
			RPS_AddToString(string, 
				va("xv %i yv %i string2 \"%c%-11.11s %-3i %i\" ",
				x, y,
				tag,
				cl->pers.netname,
				cl->resp.score, 
				(cl->ping > 999) ? 999 : cl->ping
				),
				sizeof(string));
			y+=8;
		}
	}
	// Show a countdown if there's a time limit or if it's preptime
	if ((timelimit->value && !level.intermissiontime) || (preptimeframe>level.framenum))  {
		int		min, sec;
		float	clock;
		if (preptimeframe>level.framenum) {
			clock = (preptimeframe - level.framenum)/10;
		} else {
			clock = (timelimit->value*60) - (level.time - (preptimeframe/10));
		}
		min = (int)(clock / 60);
		sec = (int)clock % 60;
//		debugmsg("%2i:%02i\n", min, sec);
		RPS_AddToString(string,
			va("xv 140 yv 24 string \"%2i:%02i\" ", min, sec),
			sizeof(string));
	}
//	debugmsg("%s (%i)\n", string, strlen(string));
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
void DeathmatchScoreboard (edict_t *ent, qboolean unicast)
{
	if (ent->flags & FL_ANTIBOT)
		return;

	switch ((int)playmode->value) {
	case PM_DEATHMATCH: if (teamplay->value)
							TeamplayScoreboardMessage (ent, ent->enemy);
						else
							DeathmatchScoreboardMessage (ent, ent->enemy);
						break;
	case PM_BADGE_WARS: BWScoreboardMessage (ent, ent->enemy); break;
	case PM_BIG_HEIST: BHScoreboardMessage (ent, ent->enemy); break;
	case PM_LAST_MAN_STANDING: if (teamplay->value)
							LMSTeamplayScoreboard (ent, ent->enemy);
						else
							LMSFFAScoreboard (ent, ent->enemy);
						break;
	case PM_CAPTURE_THE_FLAG: CTFScoreboardMessage (ent, ent->enemy); break;
	case PM_KING_OF_THE_HILL: KOTHScoreboardMessage (ent, ent->enemy); break;
	}
	gi.unicast (ent, unicast);
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

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores && !ent->client->showid)
	{
		if (ent->client->idplayer) {
			ent->client->showid=true;
			GQ_PlayerIdMessage(ent, false);
		} else {
			ent->client->showscores = false;
		}
		return;
	}

	ent->client->showscores = true;
	ent->client->showid = false;
	DeathmatchScoreboard (ent, true);
}


/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer (edict_t *ent)
{
	char	string[1024];
	char	*sk;

	if (ent->flags & FL_ANTIBOT)
		return;
	if (skill->value == 0)
		sk = "easy";
	else if (skill->value == 1)
		sk = "medium";
	else if (skill->value == 2)
		sk = "hard";
	else
		sk = "hard+";

	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn help "			// background
		"xv 202 yv 12 string2 \"%s\" "		// skill
		"xv 0 yv 24 cstring2 \"%s\" "		// level name
		"xv 0 yv 54 cstring2 \"%s\" "		// help 1
		"xv 0 yv 110 cstring2 \"%s\" "		// help 2
		"xv 50 yv 164 string2 \" kills     goals    secrets\" "
		"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ", 
		sk,
		level.level_name,
		game.helpmessage1,
		game.helpmessage2,
		level.killed_monsters, level.total_monsters, 
		level.found_goals, level.total_goals,
		level.found_secrets, level.total_secrets);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent)
{
	// this is for backwards compatability
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = false;

	if (ent->client->showhelp && (ent->client->pers.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->pers.helpchanged = 0;
	HelpComputer (ent);
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
	char		tempstr[30];

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
		ent->client->ps.stats[STAT_AMMO_CARRIED] = ent->client->pers.carried[ITEM_INDEX(FindItem(ent->client->pers.weapon->pickup_name))];
		ent->client->ps.stats[STAT_WEAPON_AMMO] = ent->client->pers.weapon->capacity;
	}

	if (ent->client->artifact) {
		ent->client->ps.stats[STAT_ARTIFACT] = gi.imageindex (ent->client->artifact->icon);
	} else {
		ent->client->ps.stats[STAT_ARTIFACT] = 0;
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
/*	if (power_armor_type && (!index || (level.framenum & 8) ) )
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
*/
	if (item_weight->value) {
		int topspeed=100-((ent->client->pers.weight-15));
		if (topspeed>100)
			topspeed=100;
		if (GQ_MatchItem(ent->client->artifact,"Strength of the Bear")) {
			if (topspeed<40)
				topspeed=40;
		} else {
			if (topspeed<20)
				topspeed=20;
		}
		ent->client->ps.stats[STAT_SPEED]=topspeed;
	} else
		ent->client->ps.stats[STAT_SPEED]=0;


	if (ent->client->poison) {
//		debugmsg("Poisoned???? %i\n", STAT_POISON);
		ent->client->ps.stats[STAT_POISON] = 1;
	} else {
		ent->client->ps.stats[STAT_POISON] = 0;
	}

	if (respawn_invuln->value && (ent->client->respawn_framenum > level.framenum) && (level.framenum%10<5)) {
//		debugmsg("Respawn invuln\n");
		ent->client->ps.stats[STAT_RESPAWNED] = 1;
	} else {
		ent->client->ps.stats[STAT_RESPAWNED] = 0;
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
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_sheriff");
		if (deathmatch->value && ((int)playmode->value == PM_BADGE_WARS)) {
			ent->client->ps.stats[STAT_TIMER] = 99;
		} else {
			ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum)/10;
		}
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_marshal");
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

	if (deathmatch->value)
	{
		if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}

	//
	// frags
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;


	// Team score / KPH
	if (deathmatch->value && (teamplay->value)) {
		if (ent->client->team==0) {
			// Team color
			ent->client->ps.stats[STAT_TEAMCOLOR] = 0;
			ent->client->ps.stats[STAT_TEAMSCORE_KPH]=0;
		} else {
			// Team color
			if (teamdata[ent->client->team-1].score > 99.9) {
				sprintf(tempstr,"team%i-3", ent->client->team);
			} else if (teamdata[ent->client->team-1].score > 9.9) {
				sprintf(tempstr,"team%i-2", ent->client->team);
			} else {
				sprintf(tempstr,"team%i-1", ent->client->team);
			}
			ent->client->ps.stats[STAT_TEAMCOLOR] = gi.imageindex (tempstr);
			ent->client->ps.stats[STAT_TEAMSCORE_KPH] = (int)teamdata[ent->client->team-1].score;
			ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex(va("i_health%i", ent->client->team));
		}
	} else {

		if ((level.framenum - ent->client->resp.enterframe)>0) {
			ent->client->ps.stats[STAT_TEAMSCORE_KPH] = (int)(ent->client->resp.standard[GSTAT_KILLS] * 36000 / (level.framenum - ent->client->resp.enterframe));
		} else {
			ent->client->ps.stats[STAT_TEAMSCORE_KPH]=0;
		}
	}

	if (deathmatch->value) {
		qboolean tie=false;
		if (playerlist[0]==-1)
			ent->client->ps.stats[STAT_RANKING]=0;
		else {
			for (index=0; playerlist[index]>-1; index++) {
				if (&game.clients[playerlist[index]]==ent->client)
					break;
			}
			if (playerlist[index]==-1)
				ent->client->ps.stats[STAT_RANKING]=0;
			else {
				// For ties
				while ((index>0) && (game.clients[playerlist[index]].resp.score == game.clients[playerlist[index-1]].resp.score)) {
					index--;
					tie=true;
				}
//				if ((index>0) && (game.clients[playerlist[index]].resp.score == game.clients[playerlist[index+1]].resp.score))
				if (game.clients[playerlist[index]].resp.score == game.clients[playerlist[index+1]].resp.score)
					tie=true;
				ent->client->ps.stats[STAT_RANKING]=index+1;
			}
		}
		if ((ent->client->ps.stats[STAT_RANKING]==1) && (ent->client->resp.lastrank>1) && (ent->client->team)) {
			if (tie)
				gi_bprintf(PRINT_MEDIUM, "%s ties for first place.\n", ent->client->pers.netname);
			else
				gi_bprintf(PRINT_MEDIUM, "%s moves into first place.\n", ent->client->pers.netname);
		}
		ent->client->resp.lastrank=ent->client->ps.stats[STAT_RANKING];
	}

	//
	// help icon / current weapon if not shown
	//
	if (ent->client->pers.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

	ent->client->ps.stats[STAT_SPECTATOR] = 0;

	if (deathmatch->value && ((int)playmode->value == PM_BIG_HEIST)) {
		sprintf(tempstr, "vault%i", (int)number_of_teams->value);
		ent->client->ps.stats[STAT_TEAM3]=gi.imageindex(tempstr);
		ent->client->ps.stats[STAT_TEAM1_COUNT]=teamdata[0].vault->count;
		ent->client->ps.stats[STAT_TEAM2_COUNT]=teamdata[1].vault->count;
		if ((int)number_of_teams->value>2)
			ent->client->ps.stats[STAT_TEAM3_COUNT]=teamdata[2].vault->count;
		else
			ent->client->ps.stats[STAT_TEAM3_COUNT]=0;
		if ((int)number_of_teams->value>3)
			ent->client->ps.stats[STAT_TEAM4_COUNT]=teamdata[3].vault->count;
		else
			ent->client->ps.stats[STAT_TEAM4_COUNT]=0;
		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Money Bag"))])
			ent->client->ps.stats[STAT_TEAM4]=gi.imageindex("p_money");
		else
			ent->client->ps.stats[STAT_TEAM4]=0;
	}
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

