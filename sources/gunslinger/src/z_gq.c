/*
This file contains special code for the various playmodes of Gunslinger
Quake 2.

Quintin Stone
*/
#define PAIN_RADIUS		200

#include "g_local.h"
#include "z_gq.h"
#include "stdlib.h"

char	*team_colors[] =
{
	"Red",
	"Blue",
	"Green",
	"White"
};

void spectator_respawn (edict_t *ent);
void ResetStatusBar(void);
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void func_explosive_explode (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

void GQ_CountTeamPlayers(void) {
	int i;
	for (i=0; i<(int)number_of_teams->value; i++)
		teamdata[i].players=0;

	for (i=0; playerlist[i]>-1; i++) {
		if (game.clients[playerlist[i]].team)
			teamdata[game.clients[playerlist[i]].team-1].players++;
	}
}

int GQ_FindSmallestTeam(edict_t *ent) {
	int i, lowest;
	int team;
	lowest=9999;
	GQ_CountTeamPlayers();
	for (i=0;i<(int)number_of_teams->value;i++) {
		if (teamdata[i].players<lowest) {
			lowest=teamdata[i].players;
			team=i+1;
		}
	}
//	debugmsg("Smallest team %i: %i players\n", team, lowest);
	return team;
}

// Attaches a colored team identifier to a player model
// All models now use the same team identifier model
void GQ_AttachTriangle(edict_t *ent) {
	char z[128];

//	debugmsg("Userinfo: %s\n", s);

	if (!ent->client->team)
		return;

//	debugmsg("Okay, setting identifier model.\n");
	switch (ent->client->team) {
		case 1: sprintf(z,"models/objects/teams/red/tris.md2"); break;
		case 2: sprintf(z,"models/objects/teams/blue/tris.md2"); break;
		case 3: sprintf(z,"models/objects/teams/green/tris.md2"); break;
		case 4: sprintf(z,"models/objects/teams/white/tris.md2"); break;
	}
//	debugmsg("Assigning model: %s\n", z);
	ent->s.modelindex4 = gi.modelindex (z);
}

// Borrowed from Zoid's CTF source code
void CTFAssignSkin(edict_t *ent, char *s)
{
	int playernum = ent-g_edicts-1;
	char *p;
	char t[64];

	if (!ent->client->team)
		return;

	Com_sprintf(t, sizeof(t), "%s", s);

	if ((p = strrchr(t, '/')) != NULL)
		p[1] = 0;
	else
		strcpy(t, "male/");

	switch (ent->client->team) {
	case 1:
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s",
			ent->client->pers.netname, t, "ctf_r") );
		break;
	case 2:
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s",
			ent->client->pers.netname, t, "ctf_b") );
		break;
	case 3:
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s",
			ent->client->pers.netname, t, "ctf_g") );
		break;
	case 4:
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s",
			ent->client->pers.netname, t, "ctf_y") );
		break;
	default:
		gi.configstring (CS_PLAYERSKINS+playernum,
			va("%s\\%s", ent->client->pers.netname, s) );
		break;
	}
//	gi_cprintf(ent, PRINT_HIGH, "You have been assigned to %s team.\n", ent->client->pers.netname);
}

// Decides how the model's team should be identified
void GQ_SetTeamIdentifier(edict_t *ent) {
	char *s;
	if (!ent->client->team)
		return;

	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");

	if (use_ctf_skins->value) {
		CTFAssignSkin(ent, s);
	} else {
		GQ_AttachTriangle(ent);
	}
}

// Logic for the menu choice
void GQ_ChooseTeam(edict_t *ent, int choice) {
	int team;
	debugmsg("Chose team %i\n",choice);
	if (teamplay->value) {
		if (choice>(int)number_of_teams->value) {
			team=0;
			gi_centerprintf(ent, "Type 'team_menu' to access team choices again\n");
		} else if (choice==0) {
			team=GQ_FindSmallestTeam(ent);
		} else
			team=choice;
	} else {
		team=!choice;
	}
//	gi_cprintf(ent, PRINT_HIGH,"Your choice was %i\n",choice);
	if (team || !(teamplay->value)) {
		ent->client->pers.spectator=false;
		ent->client->resp.spectator=false;
		ent->client->team=team;
		debugmsg("Chose team %i\n",choice);
		debugmsg("Chose team %i\n",team);
		debugmsg("Chose team %i\n",ent->client->team);
		debugmsg("spectator_respawn\n");
		spectator_respawn(ent);
		debugmsg("GQ_ResetPrepTime\n");
		GQ_ResetPrepTime();
	}
	debugmsg("RPS_MenuClose\n");
	RPS_MenuClose(ent);
	debugmsg("GQ_MOTD\n");
	GQ_MOTD(ent);
}

void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles, qboolean teamspawn);

void GQ_SetupClient (edict_t *ent, vec3_t spawn_origin, vec3_t spawn_angles) {
	int			i;
	gclient_t	*client=ent->client;

	if (!deathmatch->value)
		return;

	if (ent->flags & FL_ANTIBOT)
		return;

	// antibot downloads
	if (!ent->client->pers.allowed_download && sv_botdetection->value)
	{
		stuffcmd (ent, "set allow_download 1\n");
//		ent->message=RPS_AddToStringPtr(ent->message, "set allow_download 1\n", true);
		stuffcmd (ent, "set allow_download_players 1\n");
//		ent->message=RPS_AddToStringPtr(ent->message, "set allow_download_players 1\n", true);
		stuffcmd (ent, "set allow_download_models 1\n");
//		ent->message=RPS_AddToStringPtr(ent->message, "set allow_download_models 1\n", true);
		stuffcmd (ent, "set cl_not_bot 1 u\n"); //sets the flag so we only reconnect one time
//		ent->message=RPS_AddToStringPtr(ent->message, "set cl_not_bot 1 u\n", true);

		gi_cprintf(ent, PRINT_HIGH, "-=BotCRuSher=- checking for bot...\nplease wait......");
		stuffcmd (ent, "reconnect\n");
//		ent->message=RPS_AddToStringPtr(ent->message, "reconnect\n", true);
		return;
	}
	//end

//	gi_bprintf(PRINT_HIGH,"Team2: %i\n",ent->client->team);
	if (((int)playmode->value == PM_LAST_MAN_STANDING) && (preptimeframe < level.framenum)) {
		gi_centerprintf(ent, "You'll have to wait until\nthe next match to play\n");
		for (i=0; i<MAX_ITEMS; i++)
			ent->client->pers.inventory[i] = 0;
		if (!client->pers.spectator)
			client->pers.spectator=1;
		return;
	}
	// No team?  Spectator mode -- Stone
	if (!ent->client->team) {
		//		gi_bprintf(PRINT_HIGH, "No team!\n");
		ent->s.modelindex3 = 0;
		ent->s.modelindex4 = 0;
		if (force_join->value) {
			if (teamplay->value) {
				debugmsg("Finding smallest team.\n");
				ent->client->team=GQ_FindSmallestTeam(ent);
				debugmsg("Player is on team %i\n", ent->client->team);
				SelectSpawnPoint (ent, spawn_origin, spawn_angles, true);
				VectorCopy (spawn_origin, ent->s.origin);
				ent->s.origin[2] += 1;	// make sure off ground
				VectorCopy (ent->s.origin, ent->s.old_origin);
				client->ps.pmove.origin[0] = spawn_origin[0]*8;
				client->ps.pmove.origin[1] = spawn_origin[1]*8;
				client->ps.pmove.origin[2] = spawn_origin[2]*8;
				ent->s.angles[YAW] = spawn_angles[YAW];
				GQ_SetTeamIdentifier(ent);
			} else {
				ent->client->team=1;
			}
			GQ_ResetPrepTime();
			GQ_MOTD(ent);
			ent->client->pers.spectator=false;
			ent->client->resp.spectator=false;
			if ((int)playmode->value == PM_LAST_MAN_STANDING) {
				if (use_classes->value)
					RPS_MenuOpen(ent, GQ_ClassMenu, GQ_ChooseClass, false, 10);
				else
					RPS_MenuOpen(ent, GQ_WeaponsMenu, GQ_ChooseWeapon, false, 10);
			}
		} else {
			for (i=0; i<MAX_ITEMS; i++)
				ent->client->pers.inventory[i] = 0;
			client->pers.spectator=1;
			if (teamplay->value) {
				RPS_MenuOpen(ent, GQ_TeamMenu, GQ_ChooseTeam, true, 10);
			} else {
				RPS_MenuOpen(ent, GQ_FFAMenu, GQ_ChooseTeam, true, 10);
			}
		}
	} else {
		if (teamplay->value) {
			GQ_SetTeamIdentifier(ent);
		}
		if ((int)playmode->value == PM_LAST_MAN_STANDING) {
			if (use_classes->value)
				RPS_MenuOpen(ent, GQ_ClassMenu, GQ_ChooseClass, false, 10);
			else
				RPS_MenuOpen(ent, GQ_WeaponsMenu, GQ_ChooseWeapon, false, 10);
		} else {
			ent->client->respawn_framenum = level.framenum+(RESPAWN_INVULN_TIME*10);
		}
	}
}

// Like findradius, but only returns entities that can take damage,
// aren't owned by the player, and aren't on the same team
edict_t *GQ_FindEnemy (edict_t *from, edict_t *self, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if ((from==self) || (from->owner==self) || OnSameTeam(from, self))
			continue;
		if (!from->inuse)
			continue;
		if (from->solid == SOLID_NOT)
			continue;
		if (!from->takedamage)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
}


// The special hasn't been picked up yet... let's respawn it, because
// it might be in an inaccessible location
void GQ_RespawnSpecial (edict_t *self) {
	edict_t *loc=GQ_FindRandomSpawn();

	gi.unlinkentity(self);
//	debugmsg("Respawning %s\n",self->item->pickup_name);
	VectorCopy (loc->s.origin, self->s.origin);
	VectorSet(self->velocity, 500-(rand()%1000), 500-(rand()%1000), (rand()%500)+500);
	self->nextthink = level.time + 5;
	self->think = GQ_FinishSpecialSpawn;
	self->svflags = SVF_NOCLIENT;
	self->touch = NULL;
	gi.linkentity(self);
}

// Okay, the entity should have settled down now... let's make it visible
void GQ_FinishSpecialSpawn (edict_t *ent) {
//	debugmsg("FinishSpecialSpawn -- %s\n", ent->item->pickup_name);
	ent->touch = Touch_Item;
	if (ent->svflags == SVF_NOCLIENT)
		ent->s.event = EV_ITEM_RESPAWN;
	ent->svflags = 0;
	ent->nextthink = level.time + SPECIAL_RESPAWN_TIME + (rand() % SPECIAL_RESPAWN_RANGE);
	ent->think = GQ_RespawnSpecial;
	ent->spawnflags |= DROPPED_ITEM;
}

// Randomly pick a deathmatch player spawn point
edict_t *GQ_FindRandomSpawn(void) {
	edict_t *loc=NULL;
	int target=rand() % number_of_spawns;
	int i;

//	debugmsg("Random spawn number: %i\n",target);
	for (i=0; i<=target; i++)
		loc = G_Find (loc, FOFS(classname), "info_player_deathmatch");
	if (!loc)
		loc = G_Find (loc, FOFS(classname), "info_player_deathmatch");
	return loc;
}

void GQ_DropUniques (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	float		spread;

	if (!deathmatch->value)
		return;

	if (weapons_unique->value) {
		int index,i;
		spread = 24;
		item=FindItem("Double Revolver");
		index=ITEM_INDEX(item);
		for (i=0;i<7;i++, index++, item++) {
			if (self->client->pers.inventory[index]>0) {
//				debugmsg("Dropping %s\n",item->pickup_name);
				self->client->v_angle[YAW] -= i*spread;
				drop = Drop_Item (self, item);
				self->client->v_angle[YAW] += i*spread;
				drop->spawnflags = DROPPED_PLAYER_ITEM;
			}
		}
	}

	if (((int)playmode->value == PM_BADGE_WARS) && (self->client->team==2)) {
//		debugmsg("Dropping Sheriff Badge!!\n");
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quad"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->s.effects |= EF_HYPERBLASTER;
		drop->nextthink = level.time + SPECIAL_RESPAWN_TIME + (rand() % SPECIAL_RESPAWN_RANGE);
		drop->think = GQ_RespawnSpecial;
	}
	if (self->client->artifact) {
//		debugmsg("Okay, throwing %s\n",self->client->artifact->pickup_name);
		drop = Drop_Item(self, self->client->artifact);
		drop->s.angles[0] = rand() % 360;
		drop->think = GQ_FinishSpecialSpawn;
		VectorSet(drop->velocity, 200-(rand()%100), 200-(rand()%100), (rand()%100)+100);
		VectorCopy (self->s.origin, drop->s.origin);
		drop->nextthink = level.time + .2;
		drop->movetype = MOVETYPE_BOUNCE;
		drop->s.renderfx = self->client->artifact->quantity;
		self->client->artifact=NULL;
	}
	item=FindItem("Money Bag");
	if (((int)playmode->value == PM_BIG_HEIST) && self->client->pers.inventory[ITEM_INDEX(item)]) {
		GQ_DropMoneyBag (self, item);
	}
}

void GQ_DelayedRespawn (edict_t *ent) {
	int i;
	edict_t *from;
	for (from=g_edicts,i=0 ; from < &g_edicts[globals.num_edicts]; from++,i++)
	{
		if (from->item) {
//			debugmsg("%i: %s (%i/%i)\n", i, from->item->pickup_name, from->spawnflags, from->flags);
			if ((from->item == ent->item)
			&& !(from->spawnflags & DROPPED_ITEM)
			&& !(from->spawnflags & DROPPED_PLAYER_ITEM)
			&& (from->svflags & SVF_NOCLIENT)
			&& (from->solid == SOLID_NOT)) {
//				debugmsg("Found a match!\n");
				SetRespawn(from, 1);
				G_FreeEdict(ent);
				return;
			}
		}
	}
	G_FreeEdict(ent);
}

void GQ_InitGame(void) {
	if (!deathmatch->value)
		return;

//	if ((!antibot)&&(dedicated->value)) {
	if (!antibot) {
		ravenspawnantibot();
		move_bot_time = level.time +15;
		bot_next_msg = level.time + 10;
	}

	if ((int)playmode->value == PM_LAST_MAN_STANDING) {
		preptimeframe=level.framenum+401;
		gi_bprintf(PRINT_HIGH, "Game starts in 1 minute\n");
		gameoverframe=0;
	}
	if ((int)playmode->value == PM_BIG_HEIST) {
		if (number_of_bases < 2) {
			gi.dprintf("Not enough flags to support this mode!\n");
			gi.cvar_forceset("playmode", "1");
			ResetStatusBar();
		}
		else if (number_of_bases < (int)number_of_teams->value) {
			char tempstr[8];
			sprintf(tempstr, "%i", number_of_bases);
			gi.cvar_forceset("number_of_teams", tempstr);
			gi.dprintf("More teams than flags.  Setting number of teams to %s\n", tempstr);
		}
	}
	if (((int)playmode->value == PM_LAST_MAN_STANDING) && teamplay->value) {
		if (number_of_bases < 2) {
			gi.dprintf("Not enough bases to support teamplay!\n");
			gi.cvar_forceset("teamplay", "0");
			ResetStatusBar();
		} else if (number_of_bases < (int)number_of_teams->value) {
			char tempstr[8];
			sprintf(tempstr, "%i", number_of_bases);
			gi.cvar_forceset("number_of_teams", tempstr);
			gi.dprintf("More teams than bases.  Setting number of teams to %s\n", tempstr);
		}
	}
}

// Create special items from scratch in random locations
void GQ_SpawnSpecials(void) {
	gitem_t		*item;
	int			i;
	edict_t		*tempent;

	if (!deathmatch->value)
		return;

	if ((int)playmode->value == PM_BADGE_WARS) {
		edict_t *badge;

		item=FindItem("Sheriff Badge");
		item->drop = GQ_DropBadge;
		badge = Drop_Item (GQ_FindRandomSpawn(), item);
		badge->s.angles[0] = rand() % 360;
		badge->s.effects |= EF_HYPERBLASTER;
		badge->think = GQ_FinishSpecialSpawn;
		VectorSet(badge->velocity, 500-(rand()%1000), 500-(rand()%1000), (rand()%500)+500);
		badge->nextthink = level.time + 5;
		badge->movetype = MOVETYPE_BOUNCE;
		badge->touch = NULL;
		gi.dprintf("Okay, spawning sheriff badge.\n");
	}
	if (((int)playmode->value == PM_LAST_MAN_STANDING) && (preptimeframe>level.framenum))
		return;
//	debugmsg("Spawning artifacts\n");
	for (i=0,item=itemlist ; i<game.num_items ; i++,item++) {
		if (item->flags & IT_ARTIFACT) {
			if (!allow_hands_of_lightning->value && GQ_MatchItem(item, "Hands of Lightning"))
				continue;
			if (!allow_strength_of_the_bear->value && GQ_MatchItem(item, "Strength of the Bear"))
				continue;
			if (!allow_flesh_of_the_salamander->value && GQ_MatchItem(item, "Flesh of the Salamander"))
				continue;
			if (!allow_aura_of_pain->value && GQ_MatchItem(item, "Aura of Pain"))
				continue;
			if (!allow_tongue_of_the_leech->value && GQ_MatchItem(item, "Tongue of the Leech"))
				continue;
			if (!allow_tail_of_the_scorpion->value && GQ_MatchItem(item, "Tail of the Scorpion"))
				continue;
			if (!allow_shroud_of_darkness->value && GQ_MatchItem(item, "Shroud of Darkness"))
				continue;
			tempent = Drop_Item(GQ_FindRandomSpawn(), item);
			tempent->s.angles[0] = rand() % 360;
			tempent->think = GQ_FinishSpecialSpawn;
			VectorSet(tempent->velocity, 500-(rand()%1000), 500-(rand()%1000), (rand()%500)+500);
			tempent->nextthink = level.time + 5;
			tempent->touch = NULL;
			tempent->movetype = MOVETYPE_BOUNCE;
			if (artifact_glow->value)
				tempent->s.effects |= EF_PLASMA;
			tempent->s.renderfx = item->quantity;
			tempent->svflags = SVF_NOCLIENT;
			//				debugmsg("Okay, spawning %s.\n",item->pickup_name);
		}
	}
	if ((int)playmode->value != PM_BIG_HEIST) {
		if ((int)number_of_money_bags->value>0) {
			for (i=0; i<(int)number_of_money_bags->value; i++) {
				item=FindItem("Money Bag");
				tempent = Drop_Item(GQ_FindRandomSpawn(), item);
				VectorSet(tempent->s.angles, (rand() % 360), 0, 0);
				tempent->think = GQ_FinishSpecialSpawn;
				VectorSet(tempent->velocity, 500-(rand()%1000), 500-(rand()%1000), (rand()%500)+500);
				tempent->nextthink = level.time + (rand()%50) +5;
				tempent->touch = NULL;
				tempent->movetype = MOVETYPE_BOUNCE;
				tempent->s.renderfx = item->quantity;
				tempent->svflags = SVF_NOCLIENT;
			}
		}
	}
}

void GQ_ResetPrepTime(void) {
	if (deathmatch->value && ((int)playmode->value == PM_LAST_MAN_STANDING) && (preptimeframe > level.framenum) && (preptimeframe-level.framenum < 300)) {
		if (use_classes->value)
			preptimeframe = level.framenum + 301;
		else
			preptimeframe = level.framenum + 601;
	}
}

int GQ_TeamsLeft(void) {
	int			i;
	edict_t		*ent;
	int			teamsleft=0;

	if (teamplay->value) {
		int			stillalive[MAX_TEAMS];
		for (i=0; i<MAX_TEAMS; i++)
			stillalive[i]=0;
		for (i=0 ; i<maxclients->value ; i++) {
			ent = g_edicts + 1 + i;
			if (!ent->inuse || !ent->client)
				continue;
			if (ent->client->resp.spectator)
				continue;
			if (!ent->client->team)
				continue;
			if (ent->deadflag)
				continue;
			//		debugmsg("%s Team %i\n", ent->client->pers.netname, ent->client->team-1);
			stillalive[ent->client->team-1]++;
		}
		for (i=0; i<(int)number_of_teams->value; i++) {
			//		debugmsg("I: %i\n", i);
			if (stillalive[i]>0) {
				teamsleft++;
				//			debugmsg("Found a team left\n");
			}
		}
		return teamsleft;
	} else {
		teamsleft=0;
		for (i=0 ; i<maxclients->value ; i++) {
			ent = g_edicts + 1 + i;
			if (!ent->inuse || !ent->client)
				continue;
			if (ent->client->resp.spectator)
				continue;
			if (!ent->client->team)
				continue;
			if (ent->deadflag)
				continue;
			teamsleft++;
//			debugmsg("%s is in the game\n", ent->client->pers.netname);
		}
//		debugmsg("There are %i players left\n", teamsleft);
		return teamsleft;
	}
}

void GQ_StartLastManStanding(void) {
	edict_t	*ent;
	int		i, j;
	vec3_t	spawn_origin, spawn_angles;

	gi.dprintf("Starting game\n");
	if (!deathmatch->value || ((int)playmode->value != PM_LAST_MAN_STANDING))
		return;

	// Make sure there is more than 1 team/player
	if (teamplay->value) {
		if (GQ_TeamsLeft()<2) {
			gi.dprintf("Aren't enough teams for a game!\n");
			preptimeframe=level.framenum+601;
			return;
		}
	}

	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		if (!ent->client->team) {
			gi_centerprintf(ent, "You'll have to wait until\nthe next match to play\n");
		}
		if (ent->client->showscores && ent->client->menu_data.showmenu)
			RPS_MenuClose(ent);
		if (!ent->style)
			GQ_ChooseClass(ent, 1);
		else
			GQ_ChooseClass(ent, ent->style);
		for (j=0; j<MAX_STANDARD; j++)
			ent->client->resp.standard[j]=0;
	}

	for (ent = g_edicts ; ent < &g_edicts[globals.num_edicts]; ent++) {
		if (!ent->inuse)
			continue;
		debugmsg("Entity: %s\n", ent->classname);
		if (ent->item) {
			debugmsg("Freeing %s\n", ent->item->pickup_name);
			G_FreeEdict(ent);
		}
		else if (!Q_strcasecmp(ent->classname, "bullethole")) {
			debugmsg("Freeing %s\n", ent->classname);
			G_FreeEdict(ent);
		}
		else if (!Q_strcasecmp(ent->classname, "cocktail")) {
			debugmsg("Freeing %s\n", ent->classname);
			G_FreeEdict(ent);
		}
		else if (!Q_strcasecmp(ent->classname, "hatchet")) {
			debugmsg("Freeing %s\n", ent->classname);
			G_FreeEdict(ent);
		}
		else if (!Q_strcasecmp(ent->classname, "shrapnel")) {
			debugmsg("Freeing %s\n", ent->classname);
			G_FreeEdict(ent);
		}
	}

	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;
		if (!ent->client->team)
			continue;
		debugmsg("Respawning %s\n", ent->client->pers.netname);
		SelectSpawnPoint (ent, spawn_origin, spawn_angles, true);
		VectorCopy (spawn_origin, ent->s.origin);
		ent->s.origin[2] += 1;	// make sure off ground
		VectorCopy (ent->s.origin, ent->s.old_origin);
		ent->s.event = EV_PLAYER_TELEPORT;
		ent->client->bonus_alpha = 1;
		ent->client->ps.pmove.origin[0] = spawn_origin[0]*8;
		ent->client->ps.pmove.origin[1] = spawn_origin[1]*8;
		ent->client->ps.pmove.origin[2] = spawn_origin[2]*8;
		ent->s.angles[PITCH] = 0;
		ent->s.angles[YAW] = spawn_angles[YAW];
		ent->s.angles[ROLL] = 0;
		VectorCopy (ent->s.angles, ent->client->ps.viewangles);
		VectorCopy (ent->s.angles, ent->client->v_angle);
		VectorSet (ent->velocity, 0, 0, 0);
	}
	GQ_SpawnSpecials();
}


// Special events to occur at the beginning of a frame
void GQ_GameEffects (edict_t *ent) {
	if (deathmatch->value && ((int)playmode->value == PM_BADGE_WARS) && ent->client && (ent->client->quad_framenum > level.framenum)) {
		ent->client->badge_counter++;
		if (ent->client->badge_counter>=30) {
			ent->client->resp.score++;
			ent->client->badge_counter=0;
			ent->client->quad_framenum = level.framenum + 1000;
			ent->client->resp.special[GSTAT_SHERIFFTIME]++;
//			debugmsg("Sheriff time: %i\n", ent->client->resp.special[GSTAT_SHERIFFTIME]);
		}
	}
	if (GQ_MatchItem(ent->client->artifact,"Flesh of the Salamander") && !(level.framenum % 8) && (ent->health < 200))
		ent->health++;

	if (GQ_MatchItem(ent->client->artifact,"Aura of Pain")) {
		float	distance;
		int		range;
		vec3_t	point1, point2, dir;
		edict_t	*nearby;

		nearby=GQ_FindEnemy(NULL, ent, ent->s.origin, PAIN_RADIUS);
		while (nearby) {
			VectorMA (ent->absmin, 0.5, ent->size, point1);
			VectorMA (nearby->absmin, 0.5, nearby->size, point2);
			VectorSubtract(point1, point2, dir);
			distance=VectorLength(dir);
			range=(int)(distance/50);
//			debugmsg("Range: %i\n",range);
			if (!(level.framenum % (range+1)))
				T_Damage (nearby, ent, ent, dir, point2, vec3_origin, 2, (range)*20, DAMAGE_ENERGY, MOD_BFG_LASER);

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_LASER);
			gi.WritePosition (point1);
			gi.WritePosition (point2);
			gi.multicast (ent->s.origin, MULTICAST_PHS);
			nearby=GQ_FindEnemy(nearby, ent, ent->s.origin, PAIN_RADIUS);
		}
	}
}

// Executed when a player touches an artifact
qboolean GQ_PickupArtifact (edict_t *ent, edict_t *other) {
	if (other->client->artifact) {
		if (other->client->last_artifact_msg < level.time-5) {
			gi_centerprintf (other, "You are already carrying an artifact");
			other->client->last_artifact_msg=level.time;
		}
		return false;
	}
	ent->flags &= ~FL_RESPAWN;
	other->client->artifact=ent->item;
	if (artifact_min_timeout->value && artifact_max_timeout->value) {
		other->client->artifact_expire = level.time + (rand() % (((int)artifact_max_timeout->value - (int)artifact_min_timeout->value) + 1)) + (int)artifact_min_timeout->value;
	} else {
		other->client->artifact_expire = 0;
	}
	CalcWeight(other);
	return true;
}

void GQ_DropBadge (edict_t *ent, gitem_t *item) {
	edict_t *badge;
	if (!drop_badge->value) {
		gi_cprintf (ent, PRINT_HIGH, "Not allowed to drop the Sheriff Badge.\n");
		return;
	}
	badge = Drop_Item (ent, item);
	badge->s.effects |= EF_HYPERBLASTER;
	badge->nextthink = level.time + 2;
	badge->think = GQ_FinishSpecialSpawn;
	badge->spawnflags &= ~DROPPED_ITEM;
	ent->client->team = 1;
	ent->client->quad_framenum = 0;
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
}

// Dropping an artifact
void GQ_DropArtifact (edict_t *ent, gitem_t *item) {
	int index;
	edict_t *dropped = Drop_Item (ent, item);
	index = ITEM_INDEX(item);
	if (ent->client->pers.inventory[index])
		ent->client->pers.inventory[index]=0;
	ValidateSelectedItem (ent);
	dropped->s.renderfx = item->quantity;
	dropped->nextthink = level.time + 2;
	dropped->think = GQ_FinishSpecialSpawn;
	dropped->spawnflags |= DROPPED_ITEM;
	ent->client->artifact=NULL;
}

// When a player touches a money bag
qboolean GQ_PickupMoneyBag (edict_t *ent, edict_t *other) {
	// Special behavior in Big Heist games
	if (deathmatch->value && ((int)playmode->value == PM_BIG_HEIST)) {
		return GQ_PickupVaultBag(ent, other);
	} else {
	// Regular deathmatch behavior
//		debugmsg("Regular deathmatch behavior\n");
		other->client->resp.score++;
		if (teamplay->value) {
			teamdata[other->client->team-1].score++;
		}
		other->client->resp.standard[GSTAT_BAGS]++;
		ent->flags |= FL_RESPAWN;
		ent->think = GQ_RespawnSpecial;
		ent->nextthink = level.time + SPECIAL_RESPAWN_TIME + (rand() % SPECIAL_RESPAWN_RANGE);
		ent->svflags = SVF_NOCLIENT;
		ent->touch = NULL;
		return true;
	}
}

qboolean GQ_PickupVaultBag (edict_t *ent, edict_t *other) {
	if (!(ent->spawnflags & DROPPED_ITEM) && (ent->style == other->client->team)) {
		// This is player's vault
		if (other->client->pers.inventory[ITEM_INDEX(ent->item)]) {
			other->client->pers.inventory[ITEM_INDEX(ent->item)]=0;
			other->client->resp.score+=5;
			if (teamplay->value) {
				teamdata[other->client->team-1].score+=5;
			}
			ent->count++;
			if (ent->count>0) {
				ent->svflags=0;
			}
			ent->flags |= FL_RESPAWN;
			other->client->bonus_alpha = 0.25;
			gi_bprintf(PRINT_MEDIUM, "%s of Team %s (%s) drops off a bag in his team's vault!\n", other->client->pers.netname, teamdata[other->client->team-1].name->string, team_colors[other->client->team-1]);
			gi.sound(other, CHAN_AUTO, gi.soundindex("gunslinger/deposit.wav"), 1, ATTN_NONE, 0);
			other->client->resp.special[GSTAT_DEPOSITS]++;
			ValidateSelectedItem(other);
		}
		return false;
	} else {
		// This is the enemy vault
		if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
			return false;
		if ((ent->count>0) || (ent->spawnflags & DROPPED_ITEM)) {
			other->client->pers.inventory[ITEM_INDEX(ent->item)]=1;
			if (!(ent->spawnflags & DROPPED_ITEM)) {
				ent->count--;
				if (ent->count<1) {
					ent->svflags = SVF_NOCLIENT;
				}
				ent->flags |= FL_RESPAWN;
				other->client->resp.score+=5;
				if (teamplay->value) {
					teamdata[other->client->team-1].score+=5;
				}
				gi_bprintf(PRINT_MEDIUM, "%s raids the vault of Team %s (%s)!\n", other->client->pers.netname, teamdata[ent->style-1].name->string, team_colors[ent->style-1]);
				other->client->resp.special[GSTAT_RAIDS]++;
				gi.sound(other, CHAN_AUTO, gi.soundindex("gunslinger/withdraw3.wav"), 1, ATTN_NONE, 0);
			} else {
				ent->flags &= ~FL_RESPAWN;
			}
			return true;
		} else {
			return false;
		}
	}
}

void GQ_MakeTouchable (edict_t *ent)
{
	ent->touch = Touch_Item;
}

void GQ_DropMoneyBag (edict_t *ent, gitem_t *item) {
	edict_t *bag;
	vec_t	d;
	int		i;
	vec3_t	dist;
	int		min=500;

	if ((int)playmode->value != PM_BIG_HEIST)
		return;
	// How close to an enemy vault?
	if (ent->health > 0) {
		for (i=0; i<(int)number_of_teams->value; i++) {
			if (i+1 == ent->client->team)
				continue;
			VectorSubtract(ent->s.origin, teamdata[i].vault->s.origin, dist);
			d=VectorLength(dist);
//			debugmsg("Team %i: %f\n", i+1, d);
			if (d < min) {
				gi_cprintf(ent, PRINT_HIGH, "You are too close to an enemy vault.\n");
				return;
			}
		}
	}
	bag=Drop_Item (ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]=0;
	bag->think = GQ_MakeTouchable;
	bag->s.effects = EF_HYPERBLASTER;
	ValidateSelectedItem(ent);
}

// The effects of being poisoned
void GQ_PoisonThink (edict_t *ent) {
//	debugmsg("Poison_Think: %i (%s->%s)\n",ent->count, ent->owner->client->pers.netname, ent->enemy->client->pers.netname);
	if (ent->enemy)
		T_Damage (ent->enemy, ent, ent->owner, vec3_origin, ent->enemy->s.origin, vec3_origin, 1, 0, DAMAGE_RADIUS | DAMAGE_NO_ARMOR | DAMAGE_ENERGY | DAMAGE_NO_KNOCKBACK, MOD_POISON);
	if (ent->owner && ent->owner->client && GQ_MatchItem(ent->owner->client->artifact,"Tail of the Scorpion") && (ent->count) && (ent->enemy->health>0))
		ent->nextthink = level.time + .5;
	else {
		ent->enemy->client->poison=NULL;
		G_FreeEdict(ent);
//		debugmsg("Poison ended\n");
	}
	ent->count--;
}


// Many effects (like EF_HYPERBLASTER and EF_TRACKER) are not visible for
// a player if they are set on that player's entity object.  Some of them
// we want the player to be able to see, but still have them centered on
// a particular player.  So we have a little satellite entity that follows
// the player around everywhere he goes.  Any effects that we want to put
// on a player that they normally won't be able to see, we put on the
// satellite object instead.
void GQ_CreateSatellite(edict_t *ent) {
	edict_t *temp;
	if (ent->client->satellite)
		return;
	temp=G_Spawn();
	VectorCopy (ent->s.origin, temp->s.origin);
	temp->clipmask = MASK_SHOT;
	temp->solid = SOLID_NOT;
	temp->classname = "satellite";
	temp->svflags = SVF_DEADMONSTER;
	temp->touch = NULL;
	VectorSet (temp->mins, -1, -1, -1);
	VectorSet (temp->maxs, 1, 1, 1);
	gi.setmodel (temp, "models/objects/dot/tris.md2");
	temp->movetype = MOVETYPE_FLYMISSILE;
	temp->owner = ent;
	gi.linkentity(temp);
	ent->client->satellite=temp;
}

// Clears the effects from a player's satellite
void GQ_ClearSatellite(edict_t *ent) {
	if (!ent->client || !ent->client->satellite)
		return;
	ent->client->satellite->s.effects=0;
}

// Adds an effect to a player's satellite
void GQ_AddSatelliteEffect(edict_t *ent, int effect) {
	if (!ent->client)
		return;
	if (!ent->client->satellite)
		GQ_CreateSatellite(ent);
	ent->client->satellite->s.effects |= effect;
}

// Destroy a player's satellite
void GQ_DestroySatellite(edict_t *ent) {
	if (!ent->client || !ent->client->satellite)
		return;
	gi.unlinkentity(ent->client->satellite);
	G_FreeEdict(ent->client->satellite);
	ent->client->satellite=NULL;
}

void GQ_UpdateSatellite(edict_t *ent) {
	if (!ent->client || !ent->client->satellite)
		return;
	VectorCopy(ent->s.origin,ent->client->satellite->s.origin);
}

void GQ_MOTD(edict_t *ent) {
	FILE *motd_file;
	char *filename;
	char motd[500];
	char line[500];
	cvar_t *gamedir;

	gamedir=gi.cvar ("gamedir", "" , CVAR_SERVERINFO | CVAR_LATCH);

	filename=gi.TagMalloc((25+strlen(gamedir->string))*sizeof(char), TAG_LEVEL);
#if !id386 || defined __linux__
	sprintf(filename,"%s/motd.txt", gamedir->string);
#else
	sprintf(filename,"%s\\motd.txt", gamedir->string);
#endif

//	sprintf(filename, "%s\\motd.txt", gamedir->string);
//	debugmsg("MOTD file: %s\n", filename);

	sprintf(motd,"Welcome to Gunslinger Quake 2!\nBuild: %s\n\n", GAMEVERSION);

	if ((motd_file = fopen(filename, "r")))
	{
		// we successfully opened the file "motd.txt"
		if ( fgets(line, 500, motd_file) )
		{
			strcat(motd,line);
			// we successfully read a line from "motd.txt" into motd
			// ... read the remaining lines now
			while ( fgets(line, 80, motd_file) )
			{
				// add each new line to motd, to create a BIG message string.
				// we are using strcat: STRing conCATenation function here.
				strcat(motd, line);
			}

			// print our message.
		}
		// be good now ! ... close the file
		fclose(motd_file);
	}
	switch ((int)playmode->value) {
		case PM_DEATHMATCH: sprintf(line, "\nPlaymode is Deathmatch (%s)", (teamplay->value) ? "Team" : "FFA"); break;
		case PM_BADGE_WARS: sprintf(line, "\nPlaymode is Badge Wars (FFA)"); break;
		case PM_BIG_HEIST: sprintf(line, "\nPlaymode is The Big Heist (Teamplay)"); break;
		case PM_LAST_MAN_STANDING: sprintf(line, "\nPlaymode is Last Man Standing (Teamplay)"); break;
		case PM_CAPTURE_THE_FLAG: sprintf(line, "\nPlaymode is CTF (Teamplay)"); break;
		case PM_KING_OF_THE_HILL: sprintf(line, "\nPlaymode is KOTH (%s)", (teamplay->value) ? "Teamplay" : "FFA"); break;
	}
	strcat(motd, line);
	gi_centerprintf (ent, motd);
}

void GQ_TeamMenu(edict_t *ent) {
	char	tempstr[300];
	int		i;
//	debugmsg("Creating teammenu from scratch.\n");
	GQ_CountTeamPlayers();
	RPS_MenuAdd(ent,"Welcome to Gunslinger Quake!", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent,"Created by Quintin Stone", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent,"and the", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent,"Rebel Programmers Society", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent,"", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent," Please choose a team:", ALIGN_LEFT, 0, false);
	RPS_MenuAdd(ent," Join the smallest team", ALIGN_LEFT, 0, true);
	for (i=0; i<8; i++) {
		if (i<(int)number_of_teams->value) {
//			debugmsg("Team %i has %i players\n", i, teamdata[i].players);
			sprintf(tempstr," %-16s(%i plrs)", teamdata[i].name->string, teamdata[i].players);
			RPS_MenuAdd(ent, tempstr, ALIGN_LEFT, i+1, true);
		} else if (i==(int)number_of_teams->value) {
			RPS_MenuAdd(ent," Enter as a Spectator", ALIGN_LEFT, (int)number_of_teams->value+1, true);
		} else {
			RPS_MenuAdd(ent,"", ALIGN_CENTER, 0, false);
		}
	}
	RPS_MenuAdd(ent,"Use [ and ] to scroll", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent,"Press Enter to select", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent,"Hit Escape to exit menu", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent,"Type TEAM_MENU for this menu", ALIGN_CENTER, 0, false);
}

void GQ_FFAMenu(edict_t *ent) {
	int		i;
	RPS_MenuAdd(ent,"Welcome to Gunslinger Quake!", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent,"Created by Quintin Stone", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent,"and the", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent,"Rebel Programmers Society", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent,"", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent," Join the game", ALIGN_LEFT, 0, true);
	RPS_MenuAdd(ent," Enter as a Spectator", ALIGN_LEFT, 1, true);
	for (i=1; i<9; i++) {
		RPS_MenuAdd(ent,"", ALIGN_CENTER, 0, false);
	}
	RPS_MenuAdd(ent,"Use [ and ] to scroll", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent,"Press Enter to select", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent,"Hit Escape to exit menu", ALIGN_CENTER, 0, false);
	RPS_MenuAdd(ent,"Type TEAM_MENU for this menu", ALIGN_CENTER, 0, false);
}

void GQ_ChooseClass(edict_t *ent, int choice) {
	gitem_t		*item;
	gclient_t	*client=ent->client;
	int		i;
	for (i=0; i<MAX_ITEMS; i++)
		ent->client->pers.inventory[i] = 0;

	item = FindItem("Bowie Knife");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Lantern");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Revolver");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;
	client->pers.carried[ITEM_INDEX(item)] = item->capacity;
	client->pers.max_bullets45LC= 0;
	client->pers.max_shells		= 0;
	client->pers.max_hatchets	= 0;
	client->pers.max_cocktails	= 0;
	client->pers.max_bullets3030= 0;
	client->pers.max_bullets4570= 0;
	client->pers.max_cannonballs= 0;

	switch (choice) {
	case 1:	item = FindItem("Double Revolver");
			client->pers.inventory[ITEM_INDEX(item)] = 1;
			client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 200;
			client->pers.carried[ITEM_INDEX(item)] = item->capacity;
			client->newweapon = item;
			client->pers.max_bullets45LC = 200;
			break;
	case 2:	client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 50;
			client->pers.max_bullets45LC= 50;
			item = FindItem("Coach Gun");
			client->pers.inventory[ITEM_INDEX(item)] = 1;
			client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 50;
			client->pers.carried[ITEM_INDEX(item)] = item->capacity;
			client->newweapon = item;
			client->pers.max_shells = 50;
			break;
	case 3:	client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 20;
			client->pers.max_bullets45LC= 20;
			item = FindItem("Pump Shotgun");
			client->pers.inventory[ITEM_INDEX(item)] = 1;
			client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 30;
			client->pers.carried[ITEM_INDEX(item)] = item->capacity;
			client->newweapon = item;
			client->pers.max_shells = 30;
			break;
	case 4:	client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 50;
			client->pers.max_bullets45LC= 50;
			item = FindItem("Molotov Cocktail");
			client->pers.inventory[ITEM_INDEX(item)] = 15;
			client->newweapon = item;
			client->pers.max_cocktails = 15;
			break;
	case 5:	client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 50;
			client->pers.max_bullets45LC= 50;
			item = FindItem("Hatchet");
			client->pers.inventory[ITEM_INDEX(item)] = 20;
			client->newweapon = item;
			client->pers.max_hatchets = 20;
			break;
	case 6:	client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 30;
			client->pers.max_bullets45LC= 30;
			item = FindItem("Winchester Rifle");
			client->pers.inventory[ITEM_INDEX(item)] = 1;
			client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 40;
			client->pers.carried[ITEM_INDEX(item)] = item->capacity;
			client->newweapon = item;
			client->pers.max_bullets3030 = 40;
			break;
	case 7:	client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 20;
			client->pers.max_bullets45LC= 20;
			item = FindItem("Sharps Rifle");
			client->pers.inventory[ITEM_INDEX(item)] = 1;
			client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 20;
			client->pers.carried[ITEM_INDEX(item)] = item->capacity;
			client->newweapon = item;
			client->pers.max_bullets4570 = 20;
			break;
	case 8:	client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 10;
			client->pers.max_bullets45LC= 10;
			item = FindItem("Cannon");
			client->pers.inventory[ITEM_INDEX(item)] = 1;
			client->pers.inventory[ITEM_INDEX(FindItem(item->ammo))]= 8;
			client->pers.carried[ITEM_INDEX(item)] = item->capacity;
			client->newweapon = item;
			client->pers.max_cannonballs = 8;
			break;
	}
	if (preptimeframe > level.framenum)
		gi_centerprintf(ent, "You can change your class choice\nby typing \"class_menu\" on your console\nbefore the match starts");
	RPS_MenuClose(ent);
	ent->style=choice;
}

void GQ_ClassMenu(edict_t *ent) {
	RPS_MenuAdd(ent, "Choose your class:", ALIGN_CENTER | FULL_SPACE, 0, false);
	RPS_MenuAdd(ent, " Pistoleer", ALIGN_LEFT | FULL_SPACE, 1, true);
	RPS_MenuAdd(ent, " Rancher", ALIGN_LEFT | FULL_SPACE, 2, true);
	RPS_MenuAdd(ent, " Shotgunner", ALIGN_LEFT | FULL_SPACE, 3, true);
	RPS_MenuAdd(ent, " Brawler", ALIGN_LEFT | FULL_SPACE, 4, true);
	RPS_MenuAdd(ent, " Hunter", ALIGN_LEFT | FULL_SPACE, 5, true);
	RPS_MenuAdd(ent, " Rifleman", ALIGN_LEFT | FULL_SPACE, 6, true);
	RPS_MenuAdd(ent, " Sharpshooter", ALIGN_LEFT | FULL_SPACE, 7, true);
	if (allow_cannon->value)
		RPS_MenuAdd(ent, " Artilleryman", ALIGN_LEFT, 8, true);
}

void GQ_ChooseWeapon(edict_t *ent, int choice) {
	gitem_t	*item;
	if (!choice) {
		RPS_MenuClose(ent);
		gi_centerprintf(ent, "You can change your choices\nby typing \"weapon_menu\" on your console\nbefore the match starts");
		return;
	}
	if (choice==99) {
		if (ent->client->menu_data.create_menu == GQ_WeaponsMenu) {
			debugmsg("Opening AmmoMenu\n");
			RPS_MenuOpen(ent, GQ_AmmoMenu, GQ_ChooseWeapon, false, 0);
		} else {
			debugmsg("Opening WeaponsMenu\n");
			RPS_MenuOpen(ent, GQ_WeaponsMenu, GQ_ChooseWeapon, false, 0);
		}
		return;
	}
	if (choice>0) {
		item=&itemlist[choice];
		debugmsg("Chose %s (%i)\n", item->pickup_name, choice);
		if (item->price > ent->client->pers.cash) {
			gi_cprintf(ent, PRINT_HIGH, "You can't afford it!");
			return;
		}
		ent->client->pers.cash-=item->price;
		if (item->flags & IT_WEAPON) {
			ent->client->pers.inventory[choice]=1;
			ent->client->pers.carried[choice]=item->capacity;
		}
		if (item->flags & IT_AMMO) {
//			ent->client->pers.inventory[choice]+=item->quantity;
			if (!Add_Ammo(ent, item, item->quantity)) {
				gi_cprintf(ent, PRINT_HIGH, "You are at max capacity for that\n");
				ent->client->pers.cash+=item->price;
			}
		}
	} else {
		choice=-1*choice;
		item=&itemlist[choice];
		debugmsg("Chose %s (%i)\n", item->pickup_name, choice);
		ent->client->pers.cash+=item->price;
		if (item->flags & IT_WEAPON) {
			ent->client->pers.inventory[choice]=0;
			ent->client->pers.carried[choice]=0;
		}
		if (item->flags & IT_AMMO) {
			ent->client->pers.inventory[choice]-=item->quantity;
			if (ent->client->pers.inventory[choice]<1)
				ent->client->pers.inventory[choice]=0;
		}
	}
	RPS_MenuUpdate(ent);
	gi.unicast (ent, false);
}

void GQ_WeaponsMenu(edict_t *ent) {
	int		index;
	gitem_t	*item;
	RPS_MenuAdd(ent, va("Choose your weaponry [$%i]", ent->client->pers.cash), ALIGN_CENTER | HALF_SPACE, 0, false);
	item=FindItem("Double Revolver");
	index=ITEM_INDEX(item);
	if (!ent->client->pers.inventory[index]) {
		RPS_MenuAdd(ent, va("Second Revolver [$%i]", item->price), ALIGN_LEFT, 0, false);
		RPS_MenuAdd(ent, " Buy", ALIGN_LEFT | NO_SPACE, index, true);
		RPS_MenuAdd(ent, "Sell ", ALIGN_RIGHT | HALF_SPACE, 0, false);
	} else {
		RPS_MenuAdd(ent, va("Second Revolver [$%i] *", item->price), ALIGN_LEFT, 0, false);
		RPS_MenuAdd(ent, "  Buy", ALIGN_LEFT | NO_SPACE, 0, false);
		RPS_MenuAdd(ent, "Sell", ALIGN_RIGHT | HALF_SPACE, -1*index, true);
	}
	item=FindItem("Coach Gun");
	index=ITEM_INDEX(item);
	if (!ent->client->pers.inventory[index]) {
		RPS_MenuAdd(ent, va("Coachgun [$%i]", item->price), ALIGN_LEFT, 0, false);
		RPS_MenuAdd(ent, " Buy", ALIGN_LEFT | NO_SPACE, index, true);
		RPS_MenuAdd(ent, "Sell ", ALIGN_RIGHT | HALF_SPACE, 0, false);
	} else {
		RPS_MenuAdd(ent, va("Coachgun [$%i] *", item->price), ALIGN_LEFT, 0, false);
		RPS_MenuAdd(ent, "  Buy", ALIGN_LEFT | NO_SPACE, 0, false);
		RPS_MenuAdd(ent, "Sell", ALIGN_RIGHT | HALF_SPACE, -1*index, true);
	}
	item=FindItem("Pump Shotgun");
	index=ITEM_INDEX(item);
	if (!ent->client->pers.inventory[index]) {
		RPS_MenuAdd(ent, va("Pump Shotgun [$%i]", item->price), ALIGN_LEFT, 0, false);
		RPS_MenuAdd(ent, " Buy", ALIGN_LEFT | NO_SPACE, index, true);
		RPS_MenuAdd(ent, "Sell ", ALIGN_RIGHT | HALF_SPACE, 0, false);
	} else {
		RPS_MenuAdd(ent, va("Pump Shotgun [$%i] *", item->price), ALIGN_LEFT, 0, false);
		RPS_MenuAdd(ent, "  Buy", ALIGN_LEFT | NO_SPACE, 0, false);
		RPS_MenuAdd(ent, "Sell", ALIGN_RIGHT | HALF_SPACE, -1*index, true);
	}
/*	item=FindItem("Molotov Cocktail");
	index=ITEM_INDEX(item);
	RPS_MenuAdd(ent, va(" You have %i cocktails", ent->client->pers.inventory[index]), ALIGN_RIGHT, 0, false);
	RPS_MenuAdd(ent, "Take 1 more cocktail", ALIGN_LEFT | HALF_SPACE, index, true);

	item=FindItem("Hatchet");
	index=ITEM_INDEX(item);
	RPS_MenuAdd(ent, va(" You have %i hatchets", ent->client->pers.inventory[index]), ALIGN_RIGHT, 0, false);
	RPS_MenuAdd(ent, "Take 1 more hatchet", ALIGN_LEFT | HALF_SPACE, index, true);
*/
	item=FindItem("Winchester Rifle");
	index=ITEM_INDEX(item);
	if (!ent->client->pers.inventory[index]) {
		RPS_MenuAdd(ent, va("Winchester Rifle [$%i]", item->price), ALIGN_LEFT, 0, false);
		RPS_MenuAdd(ent, " Buy", ALIGN_LEFT | NO_SPACE, index, true);
		RPS_MenuAdd(ent, "Sell ", ALIGN_RIGHT | HALF_SPACE, 0, false);
	} else {
		RPS_MenuAdd(ent, va("Winchester Rifle [$%i] *", item->price), ALIGN_LEFT, 0, false);
		RPS_MenuAdd(ent, "  Buy", ALIGN_LEFT | NO_SPACE, 0, false);
		RPS_MenuAdd(ent, "Sell", ALIGN_RIGHT | HALF_SPACE, -1*index, true);
	}
	item=FindItem("Sharps Rifle");
	index=ITEM_INDEX(item);
	if (!ent->client->pers.inventory[index]) {
		RPS_MenuAdd(ent, va("Sharps Rifle [$%i]", item->price), ALIGN_LEFT, 0, false);
		RPS_MenuAdd(ent, " Buy", ALIGN_LEFT | NO_SPACE, index, true);
		RPS_MenuAdd(ent, "Sell ", ALIGN_RIGHT | HALF_SPACE, 0, false);
	} else {
		RPS_MenuAdd(ent, va("Sharps Rifle [$%i] *", item->price), ALIGN_LEFT, 0, false);
		RPS_MenuAdd(ent, "  Buy", ALIGN_LEFT | NO_SPACE, 0, false);
		RPS_MenuAdd(ent, "Sell", ALIGN_RIGHT | HALF_SPACE, -1*index, true);
	}
	if (allow_cannon->value) {
		item=FindItem("Cannon");
		index=ITEM_INDEX(item);
		if (!ent->client->pers.inventory[index]) {
			RPS_MenuAdd(ent, va("Cannon [$%i]", item->price), ALIGN_LEFT, 0, false);
			RPS_MenuAdd(ent, " Buy", ALIGN_LEFT | NO_SPACE, index, true);
			RPS_MenuAdd(ent, "Sell ", ALIGN_RIGHT | HALF_SPACE, 0, false);
		} else {
			RPS_MenuAdd(ent, va("Cannon [$%i] *", item->price), ALIGN_LEFT, 0, false);
			RPS_MenuAdd(ent, "  Buy", ALIGN_LEFT | NO_SPACE, 0, false);
			RPS_MenuAdd(ent, "Sell", ALIGN_RIGHT | HALF_SPACE, -1*index, true);
		}
	}
	RPS_MenuAdd(ent, "Select Ammo", ALIGN_CENTER | HALF_SPACE, 99, true);
	RPS_MenuAdd(ent, "Done", ALIGN_CENTER, 0, true);
}

void GQ_AmmoMenu(edict_t *ent) {
	int		index;
	gitem_t	*item;
	RPS_MenuAdd(ent, va("Choose your ammo [%i]", ent->client->pers.cash), ALIGN_CENTER | HALF_SPACE, 0, false);

	item=FindItem(".45 Colt Bullets");
	index=ITEM_INDEX(item);
	RPS_MenuAdd(ent, va("%i rounds of .45 Colt [$%i]", ent->client->pers.inventory[index], item->price), ALIGN_LEFT, 0, false);
	RPS_MenuAdd(ent, va(" Buy %i", item->quantity), ALIGN_LEFT | NO_SPACE, index, true);
	if (ent->client->pers.inventory[index]) {
		RPS_MenuAdd(ent, va("Sell %i ", item->quantity), ALIGN_RIGHT | HALF_SPACE, -1*index, true);
	} else {
		RPS_MenuAdd(ent, va("Sell %i  ", item->quantity), ALIGN_RIGHT | HALF_SPACE, 0, false);
	}

	item=FindItem("12 Gauge Shells");
	index=ITEM_INDEX(item);
	RPS_MenuAdd(ent, va("%i rounds of 12 Gauge [$%i]", ent->client->pers.inventory[index], item->price), ALIGN_LEFT, 0, false);
	RPS_MenuAdd(ent, va(" Buy %i", item->quantity), ALIGN_LEFT | NO_SPACE, index, true);
	if (ent->client->pers.inventory[index]) {
		RPS_MenuAdd(ent, va("Sell %i ", item->quantity), ALIGN_RIGHT | HALF_SPACE, -1*index, true);
	} else {
		RPS_MenuAdd(ent, va("Sell %i  ", item->quantity), ALIGN_RIGHT | HALF_SPACE, 0, false);
	}

	item=FindItem(".30-30 Bullets");
	index=ITEM_INDEX(item);
	RPS_MenuAdd(ent, va("%i rounds of .30-30 [$%i]", ent->client->pers.inventory[index], item->price), ALIGN_LEFT, 0, false);
	RPS_MenuAdd(ent, va(" Buy %i", item->quantity), ALIGN_LEFT | NO_SPACE, index, true);
	if (ent->client->pers.inventory[index]) {
		RPS_MenuAdd(ent, va("Sell %i ", item->quantity), ALIGN_RIGHT | HALF_SPACE, -1*index, true);
	} else {
		RPS_MenuAdd(ent, va("Sell %i  ", item->quantity), ALIGN_RIGHT | HALF_SPACE, 0, false);
	}

	item=FindItem(".45-70 Bullets");
	index=ITEM_INDEX(item);
	RPS_MenuAdd(ent, va("%i rounds of .45-70 [$%i]", ent->client->pers.inventory[index], item->price), ALIGN_LEFT, 0, false);
	RPS_MenuAdd(ent, va(" Buy %i", item->quantity), ALIGN_LEFT | NO_SPACE, index, true);
	if (ent->client->pers.inventory[index]) {
		RPS_MenuAdd(ent, va("Sell %i ", item->quantity), ALIGN_RIGHT | HALF_SPACE, -1*index, true);
	} else {
		RPS_MenuAdd(ent, va("Sell %i  ", item->quantity), ALIGN_RIGHT | HALF_SPACE, 0, false);
	}

	if (allow_cannon->value) {
		item=FindItem("Cannonballs");
		index=ITEM_INDEX(item);
		RPS_MenuAdd(ent, va("%i Cannonballs [$%i]", ent->client->pers.inventory[index], item->price), ALIGN_LEFT, 0, false);
		RPS_MenuAdd(ent, va(" Buy %i", item->quantity), ALIGN_LEFT | NO_SPACE, index, true);
		if (ent->client->pers.inventory[index]) {
			RPS_MenuAdd(ent, va("Sell %i ", item->quantity), ALIGN_RIGHT | HALF_SPACE, -1*index, true);
		} else {
			RPS_MenuAdd(ent, va("Sell %i  ", item->quantity), ALIGN_RIGHT | HALF_SPACE, 0, false);
		}
	}

	item=FindItem("Molotov Cocktail");
	index=ITEM_INDEX(item);
	RPS_MenuAdd(ent, va("%i Molotov Cocktails [$%i]", ent->client->pers.inventory[index], item->price), ALIGN_LEFT, 0, false);
	RPS_MenuAdd(ent, va(" Buy %i", item->quantity), ALIGN_LEFT | NO_SPACE, index, true);
	if (ent->client->pers.inventory[index]) {
		RPS_MenuAdd(ent, va("Sell %i ", item->quantity), ALIGN_RIGHT | HALF_SPACE, -1*index, true);
	} else {
		RPS_MenuAdd(ent, va("Sell %i  ", item->quantity), ALIGN_RIGHT | HALF_SPACE, 0, false);
	}
 
	item=FindItem("Hatchet");
	index=ITEM_INDEX(item);
	RPS_MenuAdd(ent, va("%i Hatchets [$%i]", ent->client->pers.inventory[index], item->price), ALIGN_LEFT, 0, false);
	RPS_MenuAdd(ent, va(" Buy %i", item->quantity), ALIGN_LEFT | NO_SPACE, index, true);
	if (ent->client->pers.inventory[index]) {
		RPS_MenuAdd(ent, va("Sell %i ", item->quantity), ALIGN_RIGHT | HALF_SPACE, -1*index, true);
	} else {
		RPS_MenuAdd(ent, va("Sell %i  ", item->quantity), ALIGN_RIGHT | HALF_SPACE, 0, false);
	}

//	RPS_MenuAdd(ent, "", FULL_SPACE, 0, false);

	RPS_MenuAdd(ent, "Select Weapons", ALIGN_CENTER, 99, true);
//	RPS_MenuAdd(ent, "Done", ALIGN_CENTER, 0, true);
}

void GQ_FormatString(char *message, int size, char *left, char *right, int y) {
	int i, k;
	i=160-(strlen(left)*8);
	for (k=0; k<strlen(right) ; k++)
		if ((char)right[k] <= 127)
			right[k]=(char)right[k]+128;
	RPS_AddToString(message, va("xv %i yv %i string2 \"%s: %s\" ", i, y, left, right), size);
}

void GQ_MatchStats(edict_t *ent, qboolean unicast) {
	char		message[1400], temp[64];
	int			i, y=28;
	// General
	int			teamscore=0;
	int			kills=0;
	int			streak=0;
	int			deaths=0;
	int			kph=0;
	int			kpd=0;
	int			bags=0;
	int			headshot=0;
	int			teamkills=0;
	int			suicides=0;

	// Badge Wars
	int			sherifftime=0;
	int			sheriffkills=0;
	int			outlawkills=0;

	// The Big Heist
	int			raids=0;
	int			deposits=0;
	int			bagkills=0;

	if (ent->flags & FL_ANTIBOT)
		return;
/*
	gi_cprintf(ent, PRINT_HIGH, "Your personal match statistics:\n");

	gi_cprintf(ent, PRINT_HIGH, " Score: %i\n", ent->client->resp.score);
	gi_cprintf(ent, PRINT_HIGH, " Kills: %i\n", ent->client->resp.standard[GSTAT_KILLS]);
	gi_cprintf(ent, PRINT_HIGH, " High Streak: %i\n", ent->client->resp.standard[GSTAT_STREAK]);
	gi_cprintf(ent, PRINT_HIGH, " Deaths: %i\n", ent->client->resp.standard[GSTAT_DEATHS]);
	if (ent->client->resp.standard[GSTAT_DEATHS]==0)
		strcpy(temp,"Infinite");
	else
		sprintf(temp, "%.2f", ent->client->resp.standard[GSTAT_KILLS]/ent->client->resp.standard[GSTAT_DEATHS]);
	gi_cprintf(ent, PRINT_HIGH, " Kills Per Death: %i:%i (%s)\n", ent->client->resp.standard[GSTAT_KILLS], ent->client->resp.standard[GSTAT_DEATHS], temp);
	gi_cprintf(ent, PRINT_HIGH, " Kills Per Hour: %i\n", ent->client->resp.standard[GSTAT_KILLS] * 36000 / (intermissionframenum - ent->client->resp.enterframe));
	gi_cprintf(ent, PRINT_HIGH, " Bags Collected: %i\n", ent->client->resp.standard[GSTAT_BAGS]);
	gi_cprintf(ent, PRINT_HIGH, " Headshot Damage: %i\n", ent->client->resp.standard[GSTAT_HEADSHOT]);

	if ((int)playmode->value == PM_BADGE_WARS) {
		gi_cprintf(ent, PRINT_HIGH, " Sheriff Time: %i\n", ent->client->resp.special[GSTAT_SHERIFFTIME]);
		gi_cprintf(ent, PRINT_HIGH, " Sheriffs Killed: %i\n", ent->client->resp.special[GSTAT_SHERIFFKILLS]);
		gi_cprintf(ent, PRINT_HIGH, " Outlaws Killed: %i\n", ent->client->resp.special[GSTAT_OUTLAWKILLS]);
	}

	if ((int)playmode->value == PM_BIG_HEIST) {
		gi_cprintf(ent, PRINT_HIGH, " Vault Raids: %i\n", ent->client->resp.special[GSTAT_RAIDS]);
		gi_cprintf(ent, PRINT_HIGH, " Vault Deposits: %i\n", ent->client->resp.special[GSTAT_DEPOSITS]);
		gi_cprintf(ent, PRINT_HIGH, " Bag-carriers killed: %i\n", ent->client->resp.special[GSTAT_BAGKILLS]);
	}

	gi_cprintf(ent, PRINT_HIGH, " Team kills: %i\n", ent->client->resp.standard[GSTAT_TEAMKILLS]);
	gi_cprintf(ent, PRINT_HIGH, " Suicides: %i\n", ent->client->resp.standard[GSTAT_SUICIDES]);
*/
	sprintf(message, "xv 96 yv 16 string \"Match Statistics\" ");
	RPS_AddToString(message, va("xv 96 yv 24 string \"%s\" ", GQ_TextBar(16)), sizeof(message));

	for (i=0 ; playerlist[i]>-1; i++)
	{
		if (intermissionframenum < game.clients[i].resp.enterframe)
			continue;
		if (game.clients[playerlist[i]].resp.standard[GSTAT_KILLS] > game.clients[kills].resp.standard[GSTAT_KILLS])
			kills=playerlist[i];
		if (game.clients[playerlist[i]].resp.standard[GSTAT_STREAK] > game.clients[streak].resp.standard[GSTAT_STREAK])
			streak=playerlist[i];
//		if (game.clients[playerlist[i]].resp.standard[GSTAT_DEATHS] < game.clients[deaths].resp.standard[GSTAT_DEATHS])
//			deaths=playerlist[i];

		// Current player and current record holder both never died
		if (!game.clients[playerlist[i]].resp.standard[GSTAT_DEATHS] && !game.clients[kpd].resp.standard[GSTAT_DEATHS]) {
			// Compare kills only
			if (game.clients[playerlist[i]].resp.standard[GSTAT_KILLS] > game.clients[kph].resp.standard[GSTAT_KILLS])
				kpd=playerlist[i];
		// Current player never died, record holder did
		} else if (!game.clients[playerlist[i]].resp.standard[GSTAT_DEATHS] && game.clients[kpd].resp.standard[GSTAT_DEATHS]) {
			// No comparison necessary
			kpd=playerlist[i];
		// Record holder never died, player did
		} else if (game.clients[playerlist[i]].resp.standard[GSTAT_DEATHS] && !game.clients[kpd].resp.standard[GSTAT_DEATHS]) {
			// No comparison necessary
		// Both have died at least once
		} else {
			// Compare ratios
			if (game.clients[playerlist[i]].resp.standard[GSTAT_KILLS] / game.clients[playerlist[i]].resp.standard[GSTAT_DEATHS] > game.clients[kpd].resp.standard[GSTAT_KILLS] / game.clients[kpd].resp.standard[GSTAT_DEATHS])
				kpd=playerlist[i];
		}

		if (game.clients[playerlist[i]].resp.standard[GSTAT_KILLS] * 36000 / (intermissionframenum - game.clients[playerlist[i]].resp.enterframe) > game.clients[kph].resp.standard[GSTAT_KILLS] * 36000 / (intermissionframenum - game.clients[kph].resp.enterframe)) {
			kph=playerlist[i];
		}
		if (game.clients[playerlist[i]].resp.standard[GSTAT_BAGS] > game.clients[bags].resp.standard[GSTAT_BAGS])
			bags=playerlist[i];
//		if (game.clients[playerlist[i]].resp.standard[GSTAT_HEADSHOT] > game.clients[headshot].resp.standard[GSTAT_HEADSHOT])
//			headshot=playerlist[i];
		if (teamplay->value) {
			if (game.clients[playerlist[i]].resp.standard[GSTAT_TEAMKILLS] > game.clients[teamkills].resp.standard[GSTAT_TEAMKILLS])
				teamkills=playerlist[i];
		}

//		if (game.clients[playerlist[i]].resp.standard[GSTAT_SUICIDES] > game.clients[suicides].resp.standard[GSTAT_SUICIDES])
//			suicides=playerlist[i];

		if ((int)playmode->value == PM_BADGE_WARS) {
			if (game.clients[playerlist[i]].resp.special[GSTAT_SHERIFFTIME] > game.clients[sherifftime].resp.special[GSTAT_SHERIFFTIME])
				sherifftime=playerlist[i];
			if (game.clients[playerlist[i]].resp.special[GSTAT_SHERIFFKILLS] > game.clients[sheriffkills].resp.special[GSTAT_SHERIFFKILLS])
				sheriffkills=playerlist[i];
			if (game.clients[playerlist[i]].resp.special[GSTAT_OUTLAWKILLS] > game.clients[outlawkills].resp.special[GSTAT_OUTLAWKILLS])
				outlawkills=playerlist[i];
		}

		if ((int)playmode->value == PM_BIG_HEIST) {
			if (game.clients[playerlist[i]].resp.special[GSTAT_RAIDS] > game.clients[raids].resp.special[GSTAT_RAIDS])
				raids=playerlist[i];
			if (game.clients[playerlist[i]].resp.special[GSTAT_DEPOSITS] > game.clients[deposits].resp.special[GSTAT_DEPOSITS])
				deposits=playerlist[i];
			if (game.clients[playerlist[i]].resp.special[GSTAT_BAGKILLS] > game.clients[bagkills].resp.special[GSTAT_BAGKILLS])
				bagkills=playerlist[i];
		}
	}
	if (teamplay->value) {
		for (i=0; i<(int)number_of_teams->value; i++) {
			if (teamdata[i].score > teamdata[teamscore].score)
				teamscore=i;
		}
	}
	sprintf(temp, "%s (%i)", game.clients[playerlist[0]].pers.netname, game.clients[playerlist[0]].resp.score);
	GQ_FormatString(message, sizeof(message), "High score", temp, y+=8);
	if (teamplay->value) {
		sprintf(temp, "%s (%i)", teamdata[teamscore].name->string, (int)teamdata[teamscore].score);
		GQ_FormatString(message, sizeof(message), "High team score", temp, y+=8);
	}

	sprintf(temp, "%s (%i)", game.clients[kills].pers.netname, game.clients[kills].resp.standard[GSTAT_KILLS]);
	GQ_FormatString(message, sizeof(message), "Kills", temp, y+=8);
	sprintf(temp, "%s (%i)", game.clients[streak].pers.netname, game.clients[streak].resp.standard[GSTAT_STREAK]);
	GQ_FormatString(message, sizeof(message), "Streak", temp, y+=8);
	if ((int)playmode->value != PM_LAST_MAN_STANDING) {
//		sprintf(temp, "%s (%i)", game.clients[deaths].pers.netname, game.clients[deaths].resp.standard[GSTAT_DEATHS]);
//		GQ_FormatString(message, sizeof(message), "Fewest deaths", temp, y+=8);
		sprintf(temp, "%s (%i:%i)", game.clients[kpd].pers.netname, game.clients[kpd].resp.standard[GSTAT_KILLS], game.clients[kpd].resp.standard[GSTAT_DEATHS]);
		GQ_FormatString(message, sizeof(message), "Kills/death", temp, y+=8);
		sprintf(temp, "%s (%i)", game.clients[kph].pers.netname, (int)(game.clients[kph].resp.standard[GSTAT_KILLS] * 36000 / (intermissionframenum - game.clients[kph].resp.enterframe)));
		GQ_FormatString(message, sizeof(message), "Kills/hour", temp, y+=8);
	}
	if ((int)playmode->value != PM_BIG_HEIST) {
		sprintf(temp, "%s (%i)", game.clients[bags].pers.netname, game.clients[bags].resp.standard[GSTAT_BAGS]);
		GQ_FormatString(message, sizeof(message), "Bags collected", temp, y+=8);
	}
//	sprintf(temp, "%s (%i)", game.clients[headshot].pers.netname, game.clients[headshot].resp.standard[GSTAT_HEADSHOT]);
//	GQ_FormatString(message, sizeof(message), "Headshot damage", temp, y+=8);


	if ((int)playmode->value == PM_BADGE_WARS) {
		y+=8;
		sprintf(temp, "%s (%i)", game.clients[sherifftime].pers.netname, game.clients[sherifftime].resp.special[GSTAT_SHERIFFTIME]);
		GQ_FormatString(message, sizeof(message), "Time as Sheriff", temp, y+=8);
		sprintf(temp, "%s (%i)", game.clients[outlawkills].pers.netname, game.clients[outlawkills].resp.special[GSTAT_OUTLAWKILLS]);
		GQ_FormatString(message, sizeof(message), "Outlaws killed", temp, y+=8);
		sprintf(temp, "%s (%i)", game.clients[sheriffkills].pers.netname, game.clients[sheriffkills].resp.special[GSTAT_SHERIFFKILLS]);
		GQ_FormatString(message, sizeof(message), "Sheriffs killed", temp, y+=8);
	}

	if ((int)playmode->value == PM_BIG_HEIST) {
		y+=8;
		sprintf(temp, "%s (%i)", game.clients[raids].pers.netname, game.clients[raids].resp.special[GSTAT_RAIDS]);
		GQ_FormatString(message, sizeof(message), "Vault raids", temp, y+=8);
		sprintf(temp, "%s (%i)", game.clients[deposits].pers.netname, game.clients[deposits].resp.special[GSTAT_DEPOSITS]);
		GQ_FormatString(message, sizeof(message), "Vault deposits", temp, y+=8);
		sprintf(temp, "%s (%i)", game.clients[bagkills].pers.netname, game.clients[bagkills].resp.special[GSTAT_BAGKILLS]);
		GQ_FormatString(message, sizeof(message), "Carriers killed", temp, y+=8);
	}
	y+=8;
//	sprintf(temp, "%s (%i)", game.clients[suicides].pers.netname, game.clients[suicides].resp.standard[GSTAT_SUICIDES]);
//	GQ_FormatString(message, sizeof(message), "Suicides", temp, y+=8);
	if (teamplay->value && team_damage->value) {
		sprintf(temp, "%s (%i)", game.clients[teamkills].pers.netname, game.clients[teamkills].resp.standard[GSTAT_TEAMKILLS]);
		GQ_FormatString(message, sizeof(message), "Teamkills", temp, y+=8);
	}
	gi.WriteByte (svc_layout);
	gi.WriteString (message);
	gi.unicast (ent, unicast);
}

void GQ_SetupTeamBase(edict_t *ent, int team) {
	edict_t *bag;
	vec3_t	up;
//	debugmsg("SetupTeamBase %i\n", team);
	if (deathmatch->value && ((int)playmode->value == PM_BIG_HEIST)) {
		if ((int)number_of_teams->value<team) {
			G_FreeEdict (ent);
			return;
		}
		if (number_of_bases<team)
			number_of_bases=team;
		gi.setmodel (ent, "models/objects/vault/tris.md2");
		ent->s.skinnum = 0;
		ent->solid = SOLID_BBOX;
		ent->movetype = MOVETYPE_NONE;
		ent->clipmask = MASK_PLAYERSOLID;
		//	ent->spawnflags |= ITEM_NO_TOUCH;
		ent->s.effects |= EF_HYPERBLASTER;
		VectorSet (ent->mins, -32, -24, -24);
		VectorSet (ent->maxs, 32, 24, -16);
		gi.linkentity (ent);
		bag=G_Spawn();
		bag->clipmask = MASK_PLAYERSOLID;
		VectorSet(up, 0, 0, 10);
		VectorAdd(ent->s.origin, up, bag->s.origin);
		SpawnItem(bag, FindItem("Money Bag"));
		bag->style = team;
		teamdata[team-1].vault=bag;
		bag->s.effects = 0;
		bag->count = (int)number_of_money_bags->value;
		bag->svflags = 0;
		VectorSet(bag->mins, -32, -32, -15);
		VectorSet(bag->maxs, 32, 32, 15);
	} else {
		G_FreeEdict (ent);
		return;
	}
}

/*QUAKED info_flag_team# (1 0 1) (-16 -16 -24) (16 16 32)
Vault/flag
*/
void SP_item_flag_team1(edict_t *ent)
{
	GQ_SetupTeamBase(ent, 1);
}

void SP_item_flag_team2(edict_t *ent)
{
	GQ_SetupTeamBase(ent, 2);
}

void SP_item_flag_team3(edict_t *ent)
{
	GQ_SetupTeamBase(ent, 3);
}

void SP_item_flag_team4(edict_t *ent)
{
	GQ_SetupTeamBase(ent, 4);
}

void GQ_SetupTeamSpawnPoint(edict_t *ent, int team)
{
	if (deathmatch->value && ((int)playmode->value == PM_LAST_MAN_STANDING)) {
		if ((int)number_of_teams->value<team) {
			debugmsg("Freeing spawn point for team %i\n", team);
			G_FreeEdict (ent);
			return;
		}
		if (number_of_bases<team)
			number_of_bases=team;
	}
}

void SP_info_player_team1(edict_t *ent)
{
	GQ_SetupTeamSpawnPoint(ent, 1);
}

void SP_info_player_team2(edict_t *ent)
{
	GQ_SetupTeamSpawnPoint(ent, 2);
}

void SP_info_player_team3(edict_t *ent)
{
	GQ_SetupTeamSpawnPoint(ent, 3);
}

void SP_info_player_team4(edict_t *ent)
{
	GQ_SetupTeamSpawnPoint(ent, 4);
}


void GQ_UnzoomPlayer(edict_t *ent) {
	if (ent->client->zoomed) {
		char	hand[128];
//		debugmsg("unzoom: %f\n", ent->client->unzoomsensitivity);
		sprintf(hand, "set hand %i\nset sensitivity $unzoomsensitivity\nset cl_pitchspeed $unzoompitch\nset cl_yawspeed $unzoomyaw\n",ent->client->oldhand);
//		debugmsg(hand);
//		stuffcmd(ent, hand);
		ent->message=RPS_AddToStringPtr(ent->message, hand, true);
		ent->client->ps.fov=ent->client->oldfov;
		ent->client->pers.hand=ent->client->oldhand;
		ent->client->zoomed=false;
	}
}

/*
================
Based on SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
float	PlayersRangeFromSpot (edict_t *spot, edict_t *ent);

edict_t *GQ_SelectTeamSpawnPoint (edict_t *player)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	char	object[128];

	if (!player->client->team) {
//		debugmsg("Not on a team.  Returning NULL.\n");
		return NULL;
	}
	sprintf(object, "info_player_team%i", player->client->team);
//	debugmsg("Searching for %s\n", object);

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), object)) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot, player);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), object);
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

void GQ_PlayerIdMessage (edict_t *ent, qboolean unicast)
{
	char	string[256];
	char	*message1, *message2;
	int		health;
	message1="";
	message2=NULL;

	if (ent->flags & FL_ANTIBOT)
		return;
//	debugmsg("Sending playeridmessage\n");
	if (ent->client->idplayer) {
//		debugmsg("T: %i  T1: %i  T2: %i\n",(int)teamplay->value, ent->client->team, ent->client->idplayer->client->team);
		if (teamplay->value) {
			if (ent->client->team == ent->client->idplayer->client->team) {
				message1="teammate ";
				health=ent->client->idplayer->health;
				if (health>=90)
					message2="Healthy";
				else if (health>=65)
					message2="Hurt";
				else if (health>=40)
					message2="Wounded";
				else if (health>=20)
					message2="Injured";
				else if (health>=0)
					message2="Near Death";
				else
					message2="Dead";
			} else {
				message2=va("Team %s", teamdata[ent->client->idplayer->client->team-1].name->string);
			}
		}
		if (message2)
			sprintf(string, "xv 0 yb -33 cstring2 \"Viewing %s%s (%s)\" ", message1, ent->client->idplayer->client->pers.netname, message2);
		else
			sprintf(string, "xv 0 yb -33 cstring2 \"Viewing %s%s\" ", message1, ent->client->idplayer->client->pers.netname);
	}
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, unicast);
}


void GQ_FreeHole(edict_t *ent) {
	// If the chain has become broken, panic.  Just destroy this bullethole
	// and reinitialize the chain.  This should never happen, but this is
	// just in case.  Better safe than sorry.  A bullethole may at sometime
	// get destroyed by an opening door or something.
	if (!firsthole) {
		gi.dprintf("What?  No firsthole?  Panic!\n");
		lasthole=NULL;
		holes=0;
		G_FreeEdict(ent);
		return;
	}
	gi.unlinkentity(firsthole);
	// Somehow a bullthole is dying before another hole that should have
	// died first.  Kill this entity and then proceed at the beginning of
	// the chain as normal.  This should never happen, but this is just in
	// case.  Better safe than sorry.  A bullethole may at sometime get
	// destroyed by an opening door or something.
	if (ent!=firsthole) {
		gi.dprintf("Uh oh, that's strange.  Firsthole isn't the one dying\n");
//		G_FreeEdict(old);
		firsthole->s.effects |= EF_SPHERETRANS;
		firsthole->nextthink = level.time+5;
		firsthole->think = G_FreeEdict;
	}
	// Now we kill off the first hole, the next hole takes its place
	firsthole=firsthole->owner;
//	G_FreeEdict(ent);
	ent->s.effects |= EF_SPHERETRANS;
	ent->nextthink = level.time+5;
	ent->think = G_FreeEdict;
	holes--;
//	debugmsg("Erasing hole.  Now %i\n", holes);
}

char *GQ_TextBar(int length) {
	static char buffer[128];
	if (length>128)
		length=128;
	memset(buffer+1, 30, length);
	buffer[0]=29;
	buffer[length-1]=31;
	buffer[length]=0;
	return buffer;
}

void GQ_SettingsChoice(edict_t *ent, int choice) {
//	debugmsg("Choice %i!\n", choice);
	RPS_MenuClose(ent);
	if (choice==1)
		RPS_MenuOpen(ent, GQ_SettingsMenu1, GQ_SettingsChoice, false, 0);
	else
		RPS_MenuOpen(ent, GQ_SettingsMenu2, GQ_SettingsChoice, false, 0);
}

void GQ_GetSettingValue(char *string, char *field, int flag) {
		sprintf(string, "%-22s %i", field, flag);
}

void GQ_GetSettingFlag(char *string, char *field, int flag) {
	if (flag)
		sprintf(string, "%-22s On", field);
	else
		sprintf(string, "%-22s Off", field);
}

void GQ_SettingsMenu1(edict_t *ent){
	char tempstring[30];
	RPS_MenuAdd(ent,"Current Game Configuration:", ALIGN_LEFT | HALF_SPACE, 0, false);
	switch ((int)playmode->value) {
	case PM_DEATHMATCH: sprintf(tempstring, "  Deathmatch (%s)", (teamplay->value) ? "Team" : "FFA"); break;
	case PM_BADGE_WARS: sprintf(tempstring, "  Badge Wars (FFA)"); break;
	case PM_BIG_HEIST: sprintf(tempstring, "  The Big Heist (Teamplay)"); break;
	case PM_LAST_MAN_STANDING: sprintf(tempstring, "  Last Man Standing (%s)", (teamplay->value) ? "Team" : "FFA"); break;
	case PM_CAPTURE_THE_FLAG: sprintf(tempstring, "  CTF (Teamplay)"); break;
	case PM_KING_OF_THE_HILL: sprintf(tempstring, "  KOTH (%s)", (teamplay->value) ? "Teamplay" : "FFA"); break;
	}
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  TEAM DAMAGE:", team_damage->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  USE CTF SKINS:", use_ctf_skins->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  CLEAR TEAMS:", clear_teams->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);

	if ((int)playmode->value == PM_BADGE_WARS) {
		GQ_GetSettingFlag(tempstring, "  DROP BADGE:", drop_badge->value);
		RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	}
	if ((int)playmode->value == PM_BIG_HEIST) {
		GQ_GetSettingFlag(tempstring, "  DROP BAG:", drop_bag->value);
		RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	}
	if ((int)playmode->value == PM_LAST_MAN_STANDING) {
		GQ_GetSettingFlag(tempstring, "  USE CLASSES:", use_classes->value);
		RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
		if (!use_classes->value) {
			GQ_GetSettingValue(tempstring, "  STARTING CASH:", starting_cash->value);
			RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
		}
		GQ_GetSettingFlag(tempstring, "  ANNOUNCE DEATHS:", announce_deaths->value);
		RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	}
	GQ_GetSettingValue(tempstring, "  SV_BOTDETECTION:", sv_botdetection->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  BOT TALK:", bot_talk->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);

	GQ_GetSettingFlag(tempstring, "  FORCE TEAM JOIN:", force_join->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  RESPAWN INVULN:", respawn_invuln->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  UNIQUE WEAPONS:", weapons_unique->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  DROP ALL ON DEATH:", drop_all_on_death->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  BULLET RICOCHET:", bullet_ricochet->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  ITEM WEIGHT:", item_weight->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  HIT LOCATIONS:", hit_locations->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingValue(tempstring, "  MAX HOLES:", max_holes->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingValue(tempstring, "  ARTIFACT MIN TIME:", artifact_min_timeout->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingValue(tempstring, "  ARTIFACT MAX TIME:", artifact_max_timeout->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);

	RPS_MenuAdd(ent, "", ALIGN_LEFT | NO_SPACE | HALF_SPACE, 0, false);

	RPS_MenuAdd(ent, "Next Page", ALIGN_CENTER, 2, true);
}

void GQ_SettingsMenu2(edict_t *ent){
	char tempstring[30];

	RPS_MenuAdd(ent,"Current Game Configuration:", ALIGN_LEFT | HALF_SPACE, 0, false);

	RPS_MenuAdd(ent, "Artifacts", ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  HANDS OF LIGHTNING:", allow_hands_of_lightning->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  STRENGTH OF BEAR:", allow_strength_of_the_bear->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  FLESH OF SALAMANDER:", allow_flesh_of_the_salamander->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  AURA OF PAIN:", allow_aura_of_pain->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  TONGUE OF LEECH:", allow_tongue_of_the_leech->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  TAIL OF SCORPION:", allow_tail_of_the_scorpion->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  SHROUD OF DARKNESS:", allow_shroud_of_darkness->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT | HALF_SPACE, 0, false);

	RPS_MenuAdd(ent, "Items", ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  ALLOW CANNON:", allow_cannon->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  ALLOW SHERIFF BADGE:", allow_sheriff_badge->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  ALLOW MARSHAL BADGE:", allow_marshal_badge->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  ALLOW BANDOLIER:", allow_bandolier->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  ALLOW AMMO PACK:", allow_pack->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  ALLOW LANTERN:", allow_lantern->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	GQ_GetSettingFlag(tempstring, "  ALLOW BANDAGES:", allow_bandages->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);
	sprintf(tempstring, "  ALLOW MONEY BAGS:    %-2i", (int)number_of_money_bags->value);
	RPS_MenuAdd(ent, tempstring, ALIGN_LEFT, 0, false);

	RPS_MenuAdd(ent, "", ALIGN_LEFT | NO_SPACE | HALF_SPACE, 0, false);

	RPS_MenuAdd(ent, "Previous Page", ALIGN_CENTER, 1, true);
}

/*
GQ_MatchItem - Faster than comparing an item pointer to FindItem.  Only useful
			when you want to see what the item is, though.
*/
qboolean GQ_MatchItem(gitem_t *item, char *name) {
	if (item && !Q_strcasecmp(item->pickup_name,name))
		return true;
	return false;
}

void MakeBreakable (edict_t *ent) {
	if (!ent->mass)
		ent->mass = 99;
	if (!ent->health)
		ent->health = 10;

	ent->die = func_explosive_explode;
	ent->takedamage = DAMAGE_YES;
	ent->monsterinfo.aiflags = AI_NOSTEP;

	ent->think = M_droptofloor;
	ent->nextthink = level.time + 2 * FRAMETIME;
}

void GQ_AnimateThink(edict_t *ent) {
	ent->nextthink = level.time + 0.1;
	ent->s.frame = ent->s.frame + 1;
	if (ent->s.frame>ent->style) {
		ent->s.frame=0;
	}
}

void SP_light_lamp1 (edict_t *ent) {
	gi.setmodel (ent, "models/objects/lights/lamp1/tris.md2");
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -5, -5, -8);
	VectorSet (ent->maxs, 5, 5, 18);
	gi.linkentity (ent);
}

void SP_light_lamp2 (edict_t *ent) {
	gi.setmodel (ent, "models/objects/lights/lamp2/tris.md2");
	ent->s.skinnum = 0;
	ent->s.effects = EF_SPHERETRANS;
	ent->nextthink = level.time + 0.1;
	ent->think = GQ_AnimateThink;
	ent->style=1;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -3, -3, -8);
	VectorSet (ent->maxs, 3, 3, -2);
	gi.linkentity (ent);
}

void SP_light_lamp3 (edict_t *ent) {
	gi.setmodel (ent, "models/objects/lights/lamp3/tris.md2");
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -12, -12, -8);
	VectorSet (ent->maxs, 12, 12, 28);
	gi.linkentity (ent);
}

void SP_light_candle1 (edict_t *ent) {
	gi.setmodel (ent, "models/objects/lights/candle1/tris.md2");
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
	ent->nextthink = level.time + 0.1;
	ent->think = GQ_AnimateThink;
	ent->style=1;
	VectorSet (ent->mins, -1, -1, -8);
	VectorSet (ent->maxs, 1, 1, 4);
	gi.linkentity (ent);
}

void SP_light_candle2 (edict_t *ent) {
	gi.setmodel (ent, "models/objects/lights/candle2/tris.md2");
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
	ent->nextthink = level.time + 0.1;
	ent->think = GQ_AnimateThink;
	ent->style=1;
	VectorSet (ent->mins, -5, -5, -8);
	VectorSet (ent->maxs, 5, 5, 5);
	gi.linkentity (ent);
}

void SP_light_torch1 (edict_t *ent) {
	gi.setmodel (ent, "models/objects/lights/torch1/tris.md2");
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
	ent->nextthink = level.time + 0.1;
	ent->think = GQ_AnimateThink;
	ent->style=7;
	VectorSet (ent->mins, -6, -6, -8);
	VectorSet (ent->maxs, 6, 6, 24);
	gi.linkentity (ent);
}

void SP_light_hearth1 (edict_t *ent) {
	gi.setmodel (ent, "models/objects/lights/hearth1/tris.md2");
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
	ent->nextthink = level.time + 0.1;
	ent->think = GQ_AnimateThink;
	ent->style=7;
	VectorSet (ent->mins, -15, -15, -8);
	VectorSet (ent->maxs, 15, 15, 22);
	gi.linkentity (ent);
}

void SP_light_campfire1 (edict_t *ent) {
	gi.setmodel (ent, "models/objects/lights/campfire1/tris.md2");
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
	ent->nextthink = level.time + 0.1;
	ent->think = GQ_AnimateThink;
	ent->style=7;
	VectorSet (ent->mins, -13, -13, -8);
	VectorSet (ent->maxs, 13, 13, 16);
	gi.linkentity (ent);
}

void SP_misc_mug (edict_t *ent) {
	gi.setmodel (ent, "models/objects/mug/tris.md2");
	ent->s.skinnum = 0;
	ent->s.effects = EF_SPHERETRANS;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -7, -7, -8);
	VectorSet (ent->maxs, 7, 7, 1);

	MakeBreakable(ent);

	gi.linkentity (ent);
}

void SP_misc_glass (edict_t *ent) {
	gi.setmodel (ent, "models/objects/glass/tris.md2");
	ent->s.skinnum = 0;
	ent->s.effects = EF_SPHERETRANS;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -4, -4, -8);
	VectorSet (ent->maxs, 4, 4, -1);

	MakeBreakable(ent);

	gi.linkentity (ent);
}

void SP_misc_plate (edict_t *ent) {
	gi.setmodel (ent, "models/objects/plate/tris.md2");
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -9, -9, -8);
	VectorSet (ent->maxs, 9, 9, -6);

	MakeBreakable(ent);

	gi.linkentity (ent);
}

void SP_misc_brown_bottle (edict_t *ent) {
	gi.setmodel (ent, "models/objects/bottle2/tris.md2");
	ent->s.skinnum = 0;
//	ent->s.effects = EF_SPHERETRANS;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -4, -4, -8);
	VectorSet (ent->maxs, 4, 4, 8);

	MakeBreakable(ent);

	gi.linkentity (ent);
}

void SP_misc_green_bottle (edict_t *ent) {
	gi.setmodel (ent, "models/objects/bottle2/tris.md2");
	ent->s.skinnum = 1;
//	ent->s.effects = EF_SPHERETRANS;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -4, -4, -8);
	VectorSet (ent->maxs, 4, 4, 8);

	MakeBreakable(ent);

	gi.linkentity (ent);
}

void GQ_DeadNooseThink(edict_t *ent) {
	ent->nextthink = level.time + 0.1;
	if (ent->groundentity) {
//		debugmsg("Noose has hit the ground\n");
		G_FreeEdict(ent);
		gi.unlinkentity(ent);
	}
}

void GQ_NooseDie (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point) {
	self->movetype = MOVETYPE_TOSS;
	self->nextthink = level.time + 0.1;
	self->takedamage = DAMAGE_NO;
	self->think = GQ_DeadNooseThink;
}

void SP_misc_noose (edict_t *ent) {
	gi.setmodel (ent, "models/objects/noose/tris.md2");
	ent->s.skinnum = 1;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -5, -5, -33);
	VectorSet (ent->maxs, 5, 5, 1);

	if (!ent->mass)
		ent->mass = 99;
	if (!ent->health)
		ent->health = 10;

//	ent->die = func_explosive_explode;
	ent->takedamage = DAMAGE_YES;
	ent->die = GQ_NooseDie;
	ent->nextthink = level.time + 0.1;
	ent->think = GQ_AnimateThink;
	ent->style=19;

	gi.linkentity (ent);
}

// Print only to spectators!
void gi_sprintf(int printlevel, char *fmt, ...)
{
	int		i;
	char	bigbuffer[0x10000];
	int		len;
	va_list	argptr;
	edict_t	*cl_ent;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	if (dedicated->value)
		gi.cprintf(NULL, printlevel, bigbuffer);

	for (i=0 ; i<maxclients->value ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent || !cl_ent->client || !cl_ent->client->resp.spectator || !cl_ent->inuse || (cl_ent->flags & FL_ANTIBOT))
			continue;

		gi.cprintf(cl_ent, printlevel, bigbuffer);
	}
}
