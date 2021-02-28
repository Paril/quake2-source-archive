#include "g_local.h"

// if this is a flag ent, re-spawn it
void CTF_RecoverFlag (edict_t *ent)
{
	if (!ctf->value)
		return;
	if (!ent || !ent->item)
		return;

	if (!strcmp(ent->classname, "item_redflag"))
	{
		CTF_SpawnFlagAtBase(NULL, RED_TEAM);
		gi.bprintf(PRINT_HIGH, "The %s flag was re-spawned.\n", CTF_GetTeamString(RED_TEAM));
		return;
	}

	if (!strcmp(ent->classname, "item_blueflag"))
	{
		CTF_SpawnFlagAtBase(NULL, BLUE_TEAM);
		gi.bprintf(PRINT_HIGH, "The %s flag was re-spawned.\n", CTF_GetTeamString(BLUE_TEAM));
		return;
	}
}

// returns the number of entities with matching classname that are owned by teamnum
int CTF_GetNumSummonable (char *classname, int teamnum)
{
	int		i, numPlayers=0;
	edict_t *cl_ent;

	if (!teamnum)
		return 0;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts+1+i;

		if (!cl_ent->inuse)
			continue;
		if (cl_ent->teamnum != teamnum)
			continue;
		//gi.dprintf("adding %s's monsters\n", cl_ent->client->pers.netname);
		numPlayers += G_GetNumSummonable(cl_ent, classname);
		//gi.dprintf("done\n");
		
	}

	return numPlayers;
}
	
int CTF_GetNumClassPlayers (int classnum, int teamnum)
{
	int		i, numPlayers=0;
	edict_t *cl_ent;

	if (!teamnum)
		return 0;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts+1+i;

		if (!cl_ent->inuse)
			continue;
		if (cl_ent->teamnum != teamnum)
			continue;
		if (cl_ent->myskills.class_num != classnum)
			continue;
		numPlayers++;
	}

	return numPlayers;
}

char *CTF_GetTeamString (int teamnum)
{
	switch (teamnum)
	{
	case 1: return "Red";
	case 2: return "Blue";
	default: return "Unknown";
	}
}

int CTF_GetTeamByFlagIndex (int	flag_index)
{
	if (flag_index == red_flag_index)
		return RED_TEAM;
	else if (flag_index == blue_flag_index)
		return BLUE_TEAM;
	return 0;
}

int CTF_GetEnemyTeam (int teamnum)
{
	if (teamnum == RED_TEAM)
		return BLUE_TEAM;
	else if (teamnum == BLUE_TEAM)
		return RED_TEAM;
	return 0;
}

edict_t *CTF_GetFlagBaseEnt (int teamnum)
{
	edict_t *e=NULL;

	if (!teamnum)
		return NULL;

	// if we already have a valid pointer to the base we want, use it
	if ((teamnum == RED_TEAM) && red_base && red_base->inuse 
		&& (red_base->style == teamnum))
		return red_base;
	else if ((teamnum == BLUE_TEAM) && blue_base 
		&& blue_base->inuse && (blue_base->style == teamnum))
		return blue_base;

	// otherwise, do a search
	while((e = G_Find(e, FOFS(classname), "flag_base")) != NULL)
	{
		if (!e->inuse)
			continue;
		if (e->style != teamnum)
			continue;
		
		// save the result to avoid CPU-costly searches!
		if (teamnum == RED_TEAM)
			red_base = e;
		else if (teamnum == BLUE_TEAM)
			blue_base = e;

		return e;
	}

	return NULL;
}

edict_t *CTF_GetFlagEnt (int teamnum)
{
	edict_t *e=NULL;

	if (!teamnum)
		return NULL;

	if (teamnum == RED_TEAM)
	{
		while((e = G_Find(e, FOFS(classname), "item_redflag")) != NULL)
		{
			if (!e->inuse)
				continue;

			return e;
		}
	}
	else if (teamnum == BLUE_TEAM)
	{
		while((e = G_Find(e, FOFS(classname), "item_blueflag")) != NULL)
		{
			if (!e->inuse)
				continue;

			return e;
		}
	}

	return NULL;
}

void CTF_SummonableCheck (edict_t *self)
{
	edict_t *base, *cl_ent;

	if (!ctf->value)
		return;

	// countdown already started
	if (self->removetime > 0)
		return;

	// update every second
	if (level.framenum % (int)sv_fps->value)
		return;

	//gi.dprintf("CTF_SummonableCheck()\n");

	// get summonable's owner
	if ((cl_ent = G_GetClient(self)) == NULL)
		return;

	// try to locate enemy flag base entity
	if ((base = CTF_GetFlagBaseEnt(CTF_GetEnemyTeam(cl_ent->teamnum))) == NULL)
		return;

	// if the base is secure, allow offensive summonable usage
	if (base->count == BASE_FLAG_SECURE)
		return;
	
	// is the summonable in the enemy's base?
	if (entdist(self, base) > CTF_BASE_DEFEND_RANGE)
		return;

	// tell the summonable to auto-remove
	self->removetime = level.time + CTF_SUMMONABLE_AUTOREMOVE;
}

void CTF_SpawnFlagAtBase (edict_t *base, int teamnum)
{
	edict_t *e;
	vec3_t	start;

	if (!teamnum)
		return;

	if (base)
		e = base;
	else
		e = CTF_GetFlagBaseEnt(teamnum);

	if (e && e->inuse)
	{
		e->count = BASE_FLAG_SECURE;

		// position flag above base
		VectorCopy(e->s.origin, start);
		start[2] = e->absmax[2]+8;
		CTF_SpawnFlag(teamnum, start);
	}
	else
	{
		WriteServerMsg("CTF_SpawnFlagAtBase() couldn't find a valid base entity", "ERROR", true, true);
	}
}

int CTF_ReturnFlag (edict_t *ent, edict_t *flag)
{
	edict_t *base;

	// this is not our flag
	if (ent->teamnum != flag->style)
		return 0;

	if ((base = CTF_GetFlagBaseEnt(ent->teamnum)) != NULL)
	{	
		// the flag is already secure so there's nothing to return
		if (base->count == BASE_FLAG_SECURE)
			return 1;

		gi.bprintf(PRINT_HIGH, "%s returned the %s flag.\n", 
			ent->client->pers.netname, CTF_GetTeamString(ent->teamnum));

		//Set them up for an assist.
		ent->client->pers.ctf_assist_return = level.time + CTF_ASSIST_DURATION;

		//Give them credit
		ent->myskills.flag_returns++;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("ctf/flagret.wav"), 1, ATTN_NORM, 0);

		// respawn the flag
		CTF_SpawnFlagAtBase(base, ent->teamnum);
		
		// award points and credits to the player returning the flag
		CTF_AwardPlayer(ent, CTF_FLAG_RETURN_EXP, CTF_FLAG_RETURN_CREDITS);

		return 2;
	}

	// couldn't find the flag base!
	return 0;
}

#define INITIAL_HEALTH_FC	200
#define ADDON_HEALTH_FC		20

int CTF_GetSpecialFcHealth (edict_t *ent, qboolean max)
{
	int health = INITIAL_HEALTH_FC+ADDON_HEALTH_FC*ent->myskills.level;

	if (max)
		return health;

	// don't allow more than max health
	if (ent->health > ent->max_health)
		ent->health = ent->max_health;

	health *= ent->health/(float)ent->max_health;

	return health;
}

qboolean CTF_ApplySpecialFcRules (edict_t *ent)
{
	if (!ctf_enable_balanced_fc->value)
		return false;

	// modify their health
	ent->health = CTF_GetSpecialFcHealth(ent, false);
	ent->max_health = CTF_GetSpecialFcHealth(ent, true);

	// unmorph morphed players
	if (ent->mtype)
	{
		ent->mtype = 0;
		ent->s.modelindex = 255;
		ent->s.skinnum = ent-g_edicts-1;
		ShowGun(ent);
	}

	// remove any summonables
	VortexRemovePlayerSummonables(ent);

	// make sure bless and other auras are gone too
	CurseRemove(ent, 0);

	return true;
}

void CTF_RemoveSpecialFcRules (edict_t *ent)
{
	if (!ctf_enable_balanced_fc->value)
		return;

	// restore normal health
	ent->health = ent->health/(float)ent->max_health*MAX_HEALTH(ent);
	ent->max_health = MAX_HEALTH(ent);
}

qboolean CTF_PickupFlag (edict_t *ent, edict_t *other)
{
	int		index, teamnum, value;
	edict_t *base;

	//gi.dprintf("pick-up flag %s\n", other->classname);

	//if (!G_EntExists(other))
	//	return false;
	if (!other || !other->inuse || !other->client || G_IsSpectator(other))
		return false;

	// can't pick up flag while invincible!
	if (other->client->invincible_framenum > level.framenum)
		return false;

	// should this flag be returned to the base?
	value = CTF_ReturnFlag(other, ent);

	if (value == 1) // flag already secure
		return false;
	else if (value == 2) // return flag to base
		return true;

	// if this is a player-monster, remove the monster and restore the player
	if (PM_PlayerHasMonster(other))
		PM_RemoveMonster(other);

	if (!CTF_ApplySpecialFcRules(other))
	{
		// poltergeist class and morphed players can only return the flag
		if (other->mtype || (isMorphingPolt(other)))
			return false;
	}

	// get team number this flag belongs to
	index = ITEM_INDEX(ent->item);
	teamnum = CTF_GetTeamByFlagIndex(index);

	if ((base = CTF_GetFlagBaseEnt(teamnum)) != NULL)
	{
		base->count = BASE_FLAG_TAKEN; // the base no longer has the flag

		// disable movement abilities
		//jetpack
		other->client->thrusting = 0;
		//grapple hook
		other->client->hook_state = HOOK_READY;
		// super speed
		other->superspeed = false;
		// antigrav
		other->antigrav = false;
		// reset their velocity
		VectorClear(other->velocity);
		// stun them briefly (to prevent mage lameness)
		other->holdtime = level.time + 0.5;

		// disable scanner
		if (other->client->pers.scanner_active & 1)
			other->client->pers.scanner_active = 0;

		// alert everyone
		gi.bprintf(PRINT_HIGH, "%s got the %s flag!\n", other->client->pers.netname, 
			CTF_GetTeamString(ent->style));

		gi.sound(other, CHAN_ITEM, gi.soundindex("ctf/flagtk.wav"), 1, ATTN_NORM, 0);
		other->client->pers.inventory[index] = 1;

		//Give them credit
		other->myskills.flag_pickups++;

		/*
		other->myskills.experience += CTF_FLAG_TAKE_EXP;
		other->client->resp.score += CTF_FLAG_TAKE_EXP;
		check_for_levelup(other);
		*/
		CTF_AwardPlayer(other, CTF_FLAG_TAKE_EXP, CTF_FLAG_TAKE_CREDITS);

		return true;
	}

	WriteServerMsg("CTF_PickupFlag() could not find a valid base for the flag.", "WARNING", true, false);

	// for some reason the base doesn't exist, so destroy the flag
	G_FreeEdict(ent);
	return false;
}

void CTF_laserthink (edict_t *self)
{
	// must have an owner
	if (!self->owner || !self->owner->inuse)
	{
		G_FreeEdict(self);
		return;
	}
	
	if (self->owner->style == RED_TEAM)
		self->s.skinnum = 0xf2f2f0f0;
	else if (self->owner->style == BLUE_TEAM)
		self->s.skinnum = 0xf3f3f1f1;

	// update position
	VectorCopy(self->pos2, self->s.origin);
	VectorCopy(self->pos1, self->s.old_origin);

	self->nextthink = level.time + FRAMETIME;
}

edict_t *CTF_spawnlaser (edict_t *ent, vec3_t v1, vec3_t v2)
{
	edict_t *laser;

	laser = G_Spawn();
	laser->movetype	= MOVETYPE_NONE;
	laser->solid = SOLID_NOT;
	laser->s.renderfx = RF_BEAM|RF_TRANSLUCENT;
	laser->s.modelindex = 1; // must be non-zero
	//laser->s.sound = gi.soundindex ("world/laser.wav");
	laser->classname = "laser";
	laser->s.frame = 8; // beam diameter
    laser->owner = ent;

	// set laser color
	if (ent->style == RED_TEAM)
		laser->s.skinnum = 0xf2f2f0f0; // red
	else if (ent->style == BLUE_TEAM)
		laser->s.skinnum = 0xf3f3f1f1; // blue

    laser->think = CTF_laserthink;
	VectorCopy(v2, laser->s.origin);
	VectorCopy(v1, laser->s.old_origin);
	VectorCopy(v2, laser->pos2);
	VectorCopy(v1, laser->pos1);
	gi.linkentity(laser);
	laser->nextthink = level.time + FRAMETIME;
	return laser;
}

void CTF_flagthink (edict_t *self)
{
	vec3_t	end;
	trace_t	tr;

	//3.0 allow anyone to pick up the flag
	if (self->owner) self->owner = NULL;

	if (!self->other && !VectorLength(self->velocity))
	{
		VectorCopy(self->s.origin, end);
		end[2] += 8192;
		tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SOLID);
		VectorCopy(tr.endpos, end);
		self->other = CTF_spawnlaser(self, self->s.origin, end);
	}
	self->s.effects = 0;
	self->s.effects |= (EF_ROTATE|EF_COLOR_SHELL);
	if (self->style == RED_TEAM)
		self->s.renderfx = RF_SHELL_RED;
	else if (self->style == BLUE_TEAM)
		self->s.renderfx = RF_SHELL_BLUE;
	self->nextthink = level.time + 0.5;
}

void CTF_DropFlag (edict_t *ent, gitem_t *item)
{
	int		flagindex, enemy_team;
	edict_t *flag;

	if (!ctf->value)
		return;

	flagindex = ITEM_INDEX(item);

	// don't drop it if you don't have it!
	if (ent->client->pers.inventory[flagindex] < 1)
		return;

	enemy_team = CTF_GetTeamByFlagIndex(flagindex);

	gi.bprintf(PRINT_HIGH, "%s dropped the %s flag.\n", 
		ent->client->pers.netname, CTF_GetTeamString(enemy_team));

	flag = Drop_Item (ent, item);
	flag->think = CTF_flagthink;
	flag->style = enemy_team;
	flag->count = 0;
	//Wait a second before starting to think
	flag->nextthink = level.time + 1.0;
	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;

	ValidateSelectedItem (ent);

	CTF_RemoveSpecialFcRules(ent);
}

void CTF_SpawnFlag (int teamnum, vec3_t point)
{
	edict_t *flag;

	if (teamnum == RED_TEAM)
		flag = Spawn_Item(FindItemByClassname("item_redflag"));
	else if (teamnum == BLUE_TEAM)
		flag = Spawn_Item(FindItemByClassname("item_blueflag"));
	else
		return;
		
	flag->think = CTF_flagthink;
	flag->style = teamnum; // so think function knows which flag this is
	flag->nextthink = level.time + FRAMETIME;

	if (point)
		VectorCopy(point, flag->s.origin);
	else if (!FindValidSpawnPoint(flag, false))
		gi.dprintf("WARNING: %s flag failed to spawn\n", CTF_GetTeamString(teamnum));

	gi.linkentity(flag);
}

edict_t *CTF_GetFlagCarrier (int teamnum)
{
	int		i;
	edict_t *cl_ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts+1+i;

		if (!cl_ent->inuse)
			continue;
		if (cl_ent->teamnum != teamnum)
			continue;

		if ((cl_ent->client->pers.inventory[red_flag_index] > 0)
			|| cl_ent->client->pers.inventory[blue_flag_index] > 0)
			return cl_ent;
	}

	return NULL;
}

void CTF_AwardPlayer (edict_t *ent, int points, int credits)
{
	// basic sanity checks
	if (!ent || !ent->inuse || !ent->client)
		return;
	// don't award points to spectators
	if (G_IsSpectator(ent))
		return;

	if (ActivePlayers() < CTF_MINIMUM_PLAYERS)
		return;

	/*
	ent->myskills.experience += points;
	ent->client->resp.score += points;
	check_for_levelup(ent);
	*/
	ent->myskills.credits += credits;
	V_AddFinalExp(ent, points);

}

int CTF_GetGroupNum (edict_t *ent, edict_t *base)
{
	float dist;

	ent = G_GetClient(ent);

	if (!ent || !ent->inuse)
		return 0;

	if (!ent->teamnum)
		return 0;

	// if base ent isn't supplied, try to get it
	if (!base)
	{
		// if we can't find it, abort!
		if ((base = CTF_GetFlagBaseEnt(ent->teamnum)) == NULL)
			return 0;
	}

	dist = entdist(ent, base);
			
	// calculate the player's relative distance to their home base
	// if they are inside it, they are defenders, else they are attackers
	if (dist <= CTF_BASE_DEFEND_RANGE)
		return GROUP_DEFENDERS;
	else
		return GROUP_ATTACKERS;
}

void CTF_AwardTeam (edict_t *ent, int teamnum, int points, int credits)
{
	int		i, groupnum=0;
	float	dist;
	edict_t	*cl_ent, *base;

	// determine whether or not points should be divided between
	// defenders or attackers, or--if ent isn't specified--everyone
	if (ent)
	{
		if ((base = CTF_GetFlagBaseEnt(teamnum)) != NULL)
		{
			dist = entdist(ent, base);
			
			// calculate the player's relative distance to their home base
			// if they are inside it, they are defenders, else they are attackers
			if (dist <= CTF_BASE_DEFEND_RANGE)
				groupnum = GROUP_DEFENDERS;
			else
				groupnum = GROUP_ATTACKERS;
		}
	}

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts+1+i;
		
		if (!cl_ent->inuse)
			continue;
		if (ent && ent == cl_ent)
			continue; // skip over the attacking player, award him points separately
		if (cl_ent->teamnum != teamnum)
			continue;
		if (G_IsSpectator(cl_ent))
			continue;

		// award points to a specific group
		if (groupnum)
		{
			// calculate player's distance from their flag base
			dist = entdist(cl_ent, base);

			// players outside their base are considered attackers
			if ((groupnum == GROUP_ATTACKERS) && (dist <= CTF_BASE_DEFEND_RANGE))
				continue;
			// players within their base are considered defenders
			else if ((groupnum == GROUP_DEFENDERS) && (dist > CTF_BASE_DEFEND_RANGE))
				continue;
		}

		CTF_AwardPlayer(cl_ent, points, credits);
	}
}

qboolean CTF_AllPlayerSpawnsCaptured (int type)
{
	char	classStr[50];
	edict_t *e=NULL;

	if (type == RED_TEAM)
		strcpy(classStr, "info_player_team1");
	else if (type == BLUE_TEAM)
		strcpy(classStr, "info_player_team2");
	else
		strcpy(classStr, "info_player_deathmatch");

	while((e = G_Find(e, FOFS(classname), classStr)) != NULL)
	{
		if (!e->inuse)
			continue;
		if (!e->teamnum)
			return false;
	}

	return true;
}

int CTF_NumPlayerSpawns (int type, int teamnum)
{
	int		spawns=0;
	char	classStr[50];
	edict_t *e=NULL;

	if (type == RED_TEAM)
		strcpy(classStr, "info_player_team1");
	else if (type == BLUE_TEAM)
		strcpy(classStr, "info_player_team2");
	else
		strcpy(classStr, "info_player_deathmatch");

	while((e = G_Find(e, FOFS(classname), classStr)) != NULL)
	{
		if (e->inuse && (!teamnum || e->teamnum == teamnum))
			spawns++;
	}

	return spawns;
}

void CTF_PlayerRespawnTime (edict_t *ent)
{
	int team_spawns = CTF_NumPlayerSpawns(0, ent->teamnum) + 1;
	int total_spawns = CTF_NumPlayerSpawns(0, 0) + 1;
	float ratio = team_spawns / total_spawns;
	float time = CTF_PLAYERSPAWN_TIME;
	
	// don't apply ratio unless all spawns have been captured
	if (!CTF_AllPlayerSpawnsCaptured(0))
	{
		ent->client->respawn_time = level.time + time;
		return;
	}

	time /= ratio;

	// don't make us wait too long!
	if (time > CTF_PLAYERSPAWN_MAX_TIME)
		time = CTF_PLAYERSPAWN_MAX_TIME;

	ent->client->respawn_time = level.time + time;
}

edict_t *CTF_NearestPlayerSpawn (edict_t *ent, int teamnum, float range, qboolean vis)
{
	edict_t *e=NULL;

	while((e = findclosestradius(e, ent->s.origin, range)) != NULL)
	{
		// sanity check
		if (!e || !e->inuse)
			continue;
		// visibility check
		if (vis && !visible(ent, e))
			continue;

		if (e->mtype == CTF_PLAYERSPAWN && (!teamnum || e->teamnum == teamnum))
			return e;
	}

	return NULL;
}

void ctf_playerspawn_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		points = CTF_PLAYERSPAWN_CAPTURE_EXPERIENCE;
	int		credits = CTF_PLAYERSPAWN_CAPTURE_CREDITS;
	edict_t *cl;

	if (attacker && attacker->inuse && ((cl = G_GetClient(attacker)) != NULL))
	{
		//FIXME: award attacker bonus points for capturing a spawn point
		gi.bprintf(PRINT_HIGH, "%s captured a %s spawn point!\n", cl->client->pers.netname, CTF_GetTeamString(self->teamnum));

		CTF_AwardTeam(cl, cl->teamnum, (int)(GROUP_SHARE_MULT*points), (int)(GROUP_SHARE_MULT*credits));
		CTF_AwardPlayer(cl, (int)((1.0-GROUP_SHARE_MULT)*points), (int)((1.0-GROUP_SHARE_MULT)*credits));
	}

	// swap teams
	if (self->teamnum == RED_TEAM)
	{
		self->teamnum = BLUE_TEAM;
		self->s.renderfx = RF_SHELL_BLUE;
	}
	else
	{
		self->teamnum = RED_TEAM;
		self->s.renderfx = RF_SHELL_RED;
	}

	self->health = self->max_health;
}

void ctf_playerspawn_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	gi.dprintf("ctf_playerspawn_touch()\n");

	if (!self->teamnum && other && other->inuse && other->client && other->teamnum)
	{
	//	gi.dprintf("%d team\n", other->teamnum);

		// claim this spawn for our team
		self->teamnum = other->teamnum;
		//self->touch = NULL;
		self->takedamage = DAMAGE_YES;

		if (self->teamnum == RED_TEAM)
			self->s.renderfx = RF_SHELL_RED;
		else
			self->s.renderfx = RF_SHELL_BLUE;

	//	gi.bprintf(PRINT_HIGH, "%s claimed a spawn for the %s team!\n", other->client->pers.netname, CTF_GetTeamString(self->teamnum));
	}
}

void ctf_playerspawn_think (edict_t *self)
{

}

void CTF_InitSpawnPoints (int teamnum)
{
	int		effects=0;
	char	classStr[50];
	edict_t	*e=NULL;

	if (teamnum == RED_TEAM)
	{
		effects = RF_SHELL_RED;
		strcpy(classStr, "info_player_team1");
	}
	else if (teamnum == BLUE_TEAM)
	{
		effects = RF_SHELL_BLUE;
		strcpy(classStr, "info_player_team2");
	}
	else
	{
		strcpy(classStr, "info_player_deathmatch");
	}

	while((e = G_Find(e, FOFS(classname), classStr)) != NULL)
	{
		if (!e->inuse)
			continue;
		
		e->touch = ctf_playerspawn_touch;
		e->health = CTF_PLAYERSPAWN_HEALTH;
		e->max_health = e->health;

		// spawns shouldn't be damageable until they are claimed by a team
		if (teamnum)
			e->takedamage = DAMAGE_YES;
		else
			e->takedamage = DAMAGE_NO;

		e->solid = SOLID_BBOX;
		e->die = ctf_playerspawn_die;
		e->teamnum = teamnum;
		e->s.effects |= EF_COLOR_SHELL;
		e->s.renderfx = effects;
		e->mtype = CTF_PLAYERSPAWN;
		e->think = ctf_playerspawn_think;
	}
}

void CTF_AwardFrag (edict_t *attacker, edict_t *target)
{
	int		points=0, credits=0;
	int		enemy_teamnum;
	float	mult=1.0;
	edict_t	*team_fc, *enemy_fc, *team_base, *enemy_base, *team_spawn, *enemy_spawn;

	if (ActivePlayers() < CTF_MINIMUM_PLAYERS)
		return;

	attacker = G_GetClient(attacker);
	target = G_GetClient(target);

	//4.04 give tank morph frag exp
	if (!attacker || !target || !attacker->inuse || !target->inuse 
		|| G_IsSpectator(attacker) || G_IsSpectator(target))
		return;

	if (attacker == target)
		return;
	if (attacker->health < 1)
		return;

	// get enemy team number
	if (attacker->teamnum == RED_TEAM)
		enemy_teamnum = BLUE_TEAM;
	else if (attacker->teamnum == BLUE_TEAM)
		enemy_teamnum = RED_TEAM;

	enemy_fc = CTF_GetFlagCarrier(enemy_teamnum);
	team_fc = CTF_GetFlagCarrier(attacker->teamnum);
	team_base = CTF_GetFlagBaseEnt(attacker->teamnum);
	enemy_base = CTF_GetFlagBaseEnt(enemy_teamnum);
	team_spawn = CTF_NearestPlayerSpawn(target, attacker->teamnum, CTF_PLAYERSPAWN_DEFENSE_RANGE, false);
	enemy_spawn = CTF_NearestPlayerSpawn(target, target->teamnum, CTF_PLAYERSPAWN_DEFENSE_RANGE, false);

	// are we defending our base?
	if (team_base && (entdist(attacker, team_base) <= CTF_BASE_DEFEND_RANGE))
	{
		gi.bprintf(PRINT_HIGH, "%s defends the %s base!\n", 
			attacker->client->pers.netname, CTF_GetTeamString(attacker->teamnum));

		//Give them credit
		attacker->myskills.defense_kills++;

		points = CTF_BASE_DEFEND_EXP;
		credits = CTF_BASE_DEFEND_CREDITS;
	}
	// are we assisting a flag carrier?
	else if (team_fc && (attacker != team_fc) 
		&& (entdist(attacker, team_fc) <= CTF_FLAG_DEFEND_RANGE))
	{
		gi.bprintf(PRINT_HIGH, "%s assists the %s flag carrier!\n", 
			attacker->client->pers.netname, CTF_GetTeamString(attacker->teamnum));

		points = CTF_FLAG_DEFEND_EXP;
		credits = CTF_FLAG_DEFEND_CREDITS;
	}
	// did we kill the enemy flag carrier?
	else if (enemy_fc && (target == enemy_fc))
	{
		gi.bprintf(PRINT_HIGH, "%s kills the %s flag carrier!\n", 
			attacker->client->pers.netname, CTF_GetTeamString(enemy_teamnum));

		//Give them credit
		attacker->myskills.flag_kills++;

		//Set up the player for an assist.
		attacker->client->pers.ctf_assist_frag = level.time + CTF_ASSIST_DURATION;

		points = CTF_FLAG_KILL_EXP;
		credits = CTF_FLAG_KILL_CREDITS;
	}
	// award points for killing enemy base defenders IF their flag is secure
	// no points are awarded for enemy base campers!
	else if (enemy_base && (enemy_base->count == BASE_FLAG_SECURE) 
		&& (entdist(attacker, enemy_base) <= CTF_BASE_DEFEND_RANGE))
	{
		gi.bprintf(PRINT_HIGH, "%s kills a %s defender!\n", 
			attacker->client->pers.netname, CTF_GetTeamString(enemy_teamnum));

		//Give them credit
		attacker->myskills.offense_kills++;

		points = CTF_BASE_KILL_EXP;
		credits = CTF_BASE_KILL_CREDITS;
	}
	// award points for killing spawn defenders
	else if (enemy_spawn)
	{
		points = CTF_PLAYERSPAWN_OFFENSE_EXP;
		credits = CTF_PLAYERSPAWN_OFFENSE_CREDITS;
	}
	// award points for defending team spawn
	else if (team_spawn)
	{
		points = CTF_PLAYERSPAWN_DEFENSE_EXP;
		credits = CTF_PLAYERSPAWN_DEFENSE_CREDITS;
	}
	// award points for neutral/war zone frags/interceptions
	else
	{
		points = CTF_FRAG_EXP;
		credits = CTF_FRAG_CREDITS;
	}

	// this share is subtracted from the amount awarded to the player
	mult -= GROUP_SHARE_MULT;

	CTF_AwardTeam(attacker, attacker->teamnum, 
		(int)(points*GROUP_SHARE_MULT), (int)(credits*GROUP_SHARE_MULT));
	
	// the player is awarded the remaining share
	CTF_AwardPlayer(attacker, points*mult, credits*mult);
}

int CTF_GetTeamCaps (int teamnum)
{
	if (teamnum == RED_TEAM)
		return red_flag_caps;
	else if (teamnum == BLUE_TEAM)
		return blue_flag_caps;
	return 0;
}

void CTF_AwardFlagCapture (edict_t *carrier, int teamnum)
{
	int		enemy_teamnum=0, delta;
	int		i, exp, credits;//, caps;
	float	mod=1.0;
	edict_t	*cl_ent;
	qboolean uneventeams=false;

	if (ActivePlayers() < CTF_MINIMUM_PLAYERS)
		return;

	if (teamnum == RED_TEAM)
		enemy_teamnum = BLUE_TEAM;
	else if (teamnum == BLUE_TEAM)
		enemy_teamnum = RED_TEAM;

	// calculate delta between our team caps and enemy team caps
	delta = CTF_GetTeamCaps(teamnum)-CTF_GetTeamCaps(enemy_teamnum);
	// if it's greater than 1, then the teams may be uneven
	if (delta > 1)
	{
		uneventeams = true;
		mod = 0.5; // mod will reduce exp and credits of unfair team
	}
	
	exp = CTF_FLAG_CAPTURE_EXP*mod;
	credits = CTF_FLAG_CAPTURE_CREDITS*mod;

	// award points to winning team
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts+1+i;
		if (!cl_ent->inuse)
			continue;
		if (G_IsSpectator(cl_ent))
			continue;
		if (cl_ent->teamnum != teamnum)
			continue;

		cl_ent->myskills.inuse++;

		// cap level modifier
		if (cl_ent->myskills.inuse > 0.5*cl_ent->myskills.level)
			cl_ent->myskills.inuse = 0.5*cl_ent->myskills.level;

		CTF_AwardPlayer(cl_ent, exp, credits);
	}

	// reduce level modifier for losing team
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts+1+i;
		if (!cl_ent->inuse)
			continue;
		if (G_IsSpectator(cl_ent))
			continue;
		if (cl_ent->teamnum != enemy_teamnum)
			continue;

		cl_ent->myskills.inuse--;

		// cap level modifier
		if (cl_ent->myskills.inuse < -(0.5*cl_ent->myskills.level))
			cl_ent->myskills.inuse = -(0.5*cl_ent->myskills.level);
	}

	// let players in the game that have been waiting
	//SpawnWaitingPlayers();

	// try reorganizing teams if they are unfair
	if (uneventeams)
		OrganizeTeams(false);

}

void flagbase_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int	enemy_flag_index, enemy_teamnum;

	//if (!G_EntExists(other))
	//	return;
	if (!other || !other->inuse || !other->client || G_IsSpectator(other))
		return;

	// abort if our flag is not secure, or if the client's team doesn't match
	if (other->teamnum != self->style)
		return;

	if (self->count != BASE_FLAG_SECURE)
		return;

	// get enemy flag index
	if (other->teamnum == RED_TEAM)
	{
		enemy_teamnum = BLUE_TEAM;
		enemy_flag_index = blue_flag_index;
	}
	else if (other->teamnum == BLUE_TEAM)
	{
		enemy_teamnum = RED_TEAM;
		enemy_flag_index = red_flag_index;
	}

	// if we have the enemy flag, cap it!
	if (other->client->pers.inventory[enemy_flag_index] > 0)
	{
		edict_t *p;
		int i;

		if (other->teamnum == RED_TEAM)
			red_flag_caps++;
		else if (other->teamnum == BLUE_TEAM)
			blue_flag_caps++;

		//Notify everyone
		G_PrintGreenText(va("%s captured the %s flag!", 
			other->client->pers.netname, CTF_GetTeamString(enemy_teamnum)));
		gi.sound(other, CHAN_ITEM, gi.soundindex("ctf/flagcap.wav"), 1, ATTN_NORM, 0);

		//Give them a capture credit
		other->myskills.flag_captures++;

		//Check for assists
		for_each_player(p, i)
		{
			qboolean assist;
			assist = false;

			if(G_IsSpectator(p))
				continue;
			if(p->client->pers.ctf_assist_frag > level.time)
			{
				//Notify everyone
				G_PrintGreenText(va("%s gains an assist for killing the flag carrier!", 
					p->myskills.player_name));

                //Give them an assist credit
				assist = true;
				p->client->pers.ctf_assist_frag = 0.0;

				//Give them some bonus points
				CTF_AwardPlayer(p, CTF_FLAG_ASSIST_EXP, 0);
			}
			if(p->client->pers.ctf_assist_return > level.time)
			{
				//Notify everyone
				G_PrintGreenText(va("%s gains an assist for returning the flag!", 
					p->myskills.player_name));

                //Give them an assist credit
				assist = true;
				p->client->pers.ctf_assist_return = 0.0;

				//Give them some bonus points
				CTF_AwardPlayer(p, CTF_FLAG_ASSIST_EXP, 0);
			}
			if(assist)	p->myskills.assists++;
		}		

		other->client->pers.inventory[enemy_flag_index] = 0;
		
		// Paril
		CTF_AwardFlagCapture(other, other->teamnum);

		// let players in the game that have been waiting
		if (SpawnWaitingPlayers())
			OrganizeTeams(true);

		// restore the flag carrier's health
		CTF_RemoveSpecialFcRules(other);

		CTF_SpawnFlagAtBase(NULL, enemy_teamnum);
		CTF_SpawnPlayersInBase(RED_TEAM);
		CTF_SpawnPlayersInBase(BLUE_TEAM);
		CTF_InitSpawnPoints(0);
	}
}

void flagbase_think (edict_t *self)
{
	edict_t		*e=NULL, *cl;

	// search for nearby targets
	while ((e = findclosestradius(e, self->s.origin, 64)) != NULL)
	{
		// only look for in-use non-clients
		if (!e || !e->inuse || e->client)
			continue;

		// is this a player-spawned entity?
		if ((cl = G_GetClient(e)) == NULL)
			continue;

		// we only care about our own team
		if (cl->teamnum != self->style)
			continue;

		// remove proxy grenade
		if (!strcmp(e->classname, "proxygrenade"))
		{
			proxy_remove(e, false);
			safe_cprintf(cl, PRINT_HIGH, "Your proxy was removed because it is too close to the flag base.\n");
		}
		// remove laser
		else if (!strcmp(e->classname, "emitter"))
		{
			laser_remove(e);
			safe_cprintf(cl, PRINT_HIGH, "Your laser was removed because it is too close to the flag base.\n");
		}
		// remove magmine
		else if (!strcmp(e->classname, "magmine"))
		{
			e->takedamage = DAMAGE_NO;
			e->deadflag = DEAD_DEAD;
			e->think = BecomeExplosion1;
			e->nextthink = level.time + FRAMETIME;
			cl->magmine = NULL;

			safe_cprintf(cl, PRINT_HIGH, "Your mag mine was removed because it is too close to the flag base.\n");
		}
	}

	self->nextthink = level.time + 1.0;
}

void CTF_SpawnFlagBase (int teamnum, vec3_t point)
{
	edict_t *base;

	base = G_Spawn();
	base->think = flagbase_think;
	base->touch = flagbase_touch;
	base->nextthink = level.time + FRAMETIME;
	base->s.modelindex = gi.modelindex ("models/objects/dmspot/tris.md2");
	base->solid = SOLID_TRIGGER;
	base->movetype = MOVETYPE_NONE;
	base->clipmask = MASK_MONSTERSOLID;
	base->classname = "flag_base";
	VectorSet(base->mins, -32, -32, -24);
	VectorSet(base->maxs, 32, 32, -16);
	base->s.skinnum = 1;
	base->style = teamnum; // set which team this base is for
	base->count = BASE_FLAG_TAKEN; // indicate that flag is at the base

	if (point)
	{
		VectorCopy(point, base->s.origin);
	}
	else if (!FindValidSpawnPoint(base, false))
	{
		WriteServerMsg(va("%s base failed to spawn.", CTF_GetTeamString(teamnum)), "WARNING", true, false);
		G_FreeEdict(base);
		return;
	}

	gi.linkentity(base);

	base->s.effects |= (EF_COLOR_SHELL);
	if (base->style == RED_TEAM)
	{
		base->s.renderfx |= RF_SHELL_RED;
		red_base = base; // set base pointer
	}
	else if (base->style == BLUE_TEAM)
	{
		base->s.renderfx |= RF_SHELL_BLUE;
		blue_base = base;
	}
}

qboolean CTF_GetFlagPosition (int teamnum, vec3_t pos)
{
	char	path[512];
	FILE	*fptr;
	vec3_t	v;

	if (!pos)
		return false;

	#if defined(_WIN32) || defined(WIN32)
		sprintf(path, "%s\\settings\\loc\\%s_%d.loc", game_path->string, level.mapname, teamnum);
	#else
		sprintf(path, "%s/settings/%s_%d.loc", game_path->string, level.mapname, teamnum);
	#endif
	
		gi.dprintf("Reading file %s.\n", path);

	// read flag position from file
	if ((fptr = fopen(path, "r")) != NULL)
     {
		 fscanf(fptr, "%f,%f,%f", &v[0], &v[1], &v[2]);
		 VectorCopy(v, pos);
		 //gi.dprintf("%f %f %f\n", v[0], v[1], v[2]);
		 fclose(fptr);
		 gi.dprintf("Data for team %d at {%f, %f, %f}\n", teamnum, v[0], v[1], v[2]);
         return true; 
     }  

	return false;
}

void CTF_WriteFlagPosition (edict_t *ent)  
{
	int		teamnum;
	char	path[512];
	FILE	*fptr;

	if (!ent->myskills.administrator)
		return;

	teamnum = atoi(gi.argv(2));

	if (!teamnum)
		return;

	sprintf(path, "%s/settings/%s_%s.loc", game_path->string, level.mapname, s1);

     if ((fptr = fopen(path, "w")) != NULL) // write text to file
     {  
		 // write origin along with flag team
		 fprintf(fptr, "%f,%f,%f\n", ent->s.origin[0],ent->s.origin[1], ent->s.origin[2]); 
         fclose(fptr); 

		 safe_cprintf(ent, PRINT_HIGH, "Set flag location for %s team on %s\n", 
			 CTF_GetTeamString(teamnum), level.mapname);
         return;  
     }  
     gi.dprintf("ERROR: Failed to write to server log.\n"); 
}

void CTF_RemovePlayerFlags (void)
{
	int		i;
	edict_t *cl_ent;

	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (cl_ent && cl_ent->inuse)
		{
			cl_ent->client->pers.inventory[red_flag_index] = 0;
			cl_ent->client->pers.inventory[blue_flag_index] = 0;
		}
	}
}

void CTF_CheckFlag (edict_t *ent)
{
	int		index, teamnum;
	edict_t	*base;

	if (!ctf->value)
		return;
	if (!ent || !ent->inuse || !ent->item)
		return;

	// get item index
	index = ITEM_INDEX(ent->item);

	if (!index)
		return;

	// is this a ctf flag?
	if ((index != red_flag_index) && (index != blue_flag_index))
		return;

	// determine team number by the flag's item index
	teamnum = CTF_GetTeamByFlagIndex(index);

	// return the flag to the proper base
	if ((base = CTF_GetFlagBaseEnt(teamnum)) != NULL)
	{
		CTF_SpawnFlagAtBase(base, teamnum);
		gi.bprintf(PRINT_HIGH, "The %s flag was re-spawned.\n", CTF_GetTeamString(teamnum));
		return;
	}

	WriteServerMsg("CTF was unable to re-spawn the flag.", "WARNING", true, false);
}

void CTF_ShutDown (void)
{
	edict_t *e;

	// we don't want flags carried over to the next map
	CTF_RemovePlayerFlags();

	if (!ctf->value)
		return;

	WriteServerMsg("CTF is shutting down.", "Info", true, false);

	// remove red base
	if ((e = CTF_GetFlagBaseEnt(RED_TEAM)) != NULL)
		G_FreeEdict(e);
	// remove blue base
	if ((e = CTF_GetFlagBaseEnt(BLUE_TEAM)) != NULL)
		G_FreeEdict(e);
	// remove red flag
	if ((e = CTF_GetFlagEnt(RED_TEAM)) != NULL)
		G_FreeEdict(e);
	// remove blue flag
	if ((e = CTF_GetFlagEnt(BLUE_TEAM)) != NULL)
		G_FreeEdict(e);

	// reset flag base pointers
	red_base = NULL;
	blue_base = NULL;

	// reset flag captures
	red_flag_caps = 0;
	blue_flag_caps = 0;

	WriteServerMsg("CTF shutdown complete.", "Info", true, false);
}

void CTF_Init (void)
{
	vec3_t	start;

	CTF_ShutDown();

	WriteServerMsg("CTF is initializing.", "Info", true, false);

	// spawn red base
	if (CTF_GetFlagPosition(RED_TEAM, start))
		CTF_SpawnFlagBase(RED_TEAM, start);
	else
		CTF_SpawnFlagBase(RED_TEAM, NULL);
	
	// spawn blue base
	if (CTF_GetFlagPosition(BLUE_TEAM, start))
		CTF_SpawnFlagBase(BLUE_TEAM, start);
	else
		CTF_SpawnFlagBase(BLUE_TEAM, NULL);

	// spawn the flags
	CTF_SpawnFlagAtBase(NULL, RED_TEAM);
	CTF_SpawnFlagAtBase(NULL, BLUE_TEAM);

	// organize and assign players to teams
	WriteServerMsg("CTF is organizing teams.", "Info", true, false);
	OrganizeTeams(true);

	// spawn players in their own bases
	CTF_SpawnPlayersInBase(RED_TEAM);
	CTF_SpawnPlayersInBase(BLUE_TEAM);

	// make spawn points capturable
	CTF_InitSpawnPoints(0);

	WriteServerMsg("CTF initialization is complete.", "Info", true, false);
}

edict_t *CTF_SelectTeamSpawnPoint (edict_t *ent)
{
	char	classStr[100];
	edict_t	*e=NULL;
	vec3_t	start;
	trace_t	tr;

	if (!G_EntExists(ent))
		return NULL;

	// try to use one of the spawn points designated for team use
	if (ent->teamnum == RED_TEAM)
		strcpy(classStr, "info_player_team1");
	else if (ent->teamnum == BLUE_TEAM)
		strcpy(classStr, "info_player_team2");
	else
		return NULL;

	while((e = G_Find(e, FOFS(classname), classStr)) != NULL)
	{
		if (!e->inuse)
			continue;

		// calculate starting position slightly above the spawn point
		VectorCopy(e->s.origin, start);
		start[2] += 9;

		tr = gi.trace(start, ent->mins, ent->maxs, start, NULL, MASK_SHOT);

		// player can't fit here, try another spawn point
		if (tr.fraction < 1)
			continue;

		return e;
	}

	return NULL;
}

int CTF_GetBaseStatus (int teamnum)
{
	edict_t *base;

	if (!teamnum)
		return 0;

	base = CTF_GetFlagBaseEnt(teamnum);

	if (!base)
		return 0;

	return base->count;
}

edict_t *CTF_SelectSpawnPoint (edict_t *ent)
{
	edict_t *base, *spawn;

	base = CTF_GetFlagBaseEnt(ent->teamnum);

	if (!base)
		return SelectDeathmatchSpawnPoint(ent);

	// players spawn outside of base if the flag is secure
	// the purpose of this is to encourage players to go on the offensive
	if (base->count == BASE_FLAG_SECURE)
	{
		// try to spawn at a deathmatch spawn point
		if ((spawn = SelectDeathmatchSpawnPoint(ent)) != NULL)
		{
		//	gi.dprintf("deathmatch spawn\n");
			return spawn;
		}
		// try to spawn inside base
		{
		//	gi.dprintf("ctf spawn\n");
			return CTF_SelectTeamSpawnPoint(ent);
		}
		
	}

	// FIXME: add a search for closest DM spawn to the flag's base
	if ((spawn = CTF_SelectTeamSpawnPoint(ent)) != NULL)
		return spawn;
	else
	// we couldn't find a team spawn, so use a DM spawn instead
		return SelectDeathmatchSpawnPoint(ent);
}

void CTF_SpawnPlayersInBase (int teamnum)
{
	int		i;
	edict_t *cl_ent, *spot;
	vec3_t	start;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts+1+i;

		if (!cl_ent->inuse)
			continue;
		if (cl_ent->teamnum != teamnum)
			continue;
		if (G_IsSpectator(cl_ent))
			continue;

		G_ResetPlayerState(cl_ent);

		// return player-monster to human form
		/*
		if (PM_PlayerHasMonster(cl_ent))
		{
			PM_UpdateChasePlayers(cl_ent->owner);
			M_Remove(cl_ent->owner, true);
			PM_RestorePlayer(cl_ent);
		}
		*/

		// find team spawn point
		if ((spot = CTF_SelectTeamSpawnPoint(cl_ent)) != NULL)
		{
			VectorCopy(spot->s.origin, start);
			start[2] += 9;
			VectorCopy(start, cl_ent->s.origin);
			gi.linkentity(cl_ent);

			// give players 5 seconds of invincibility so they can
			// kill any enemy base campers
			cl_ent->client->invincible_framenum = level.framenum + 50;
		}
		
		// remove all summonables
		//VortexRemovePlayerSummonables(cl_ent);

		// remove any curses player has
		//CurseRemove(cl_ent, 0);

		// delay use of teleport to prevent capping before defense can be made
		cl_ent->myskills.abilities[TELEPORT].delay = level.time + 10.0;

		// delay wormhole
		cl_ent->myskills.abilities[BLACKHOLE].delay = level.time + 10.0;
	}
}

//float CTF_DistanceFromEnemyBase (edict_t *ent)
float CTF_DistanceFromBase(edict_t *ent, vec3_t start, int base_teamnum)
{
	edict_t *base;
	vec3_t	org;

	if (!ctf->value || !base_teamnum)
		return 8192;

	if (ent && ent->inuse)
		VectorCopy(ent->s.origin, org);
	else if (start)
		VectorCopy(start, org);
	else
		return 8192;

	// return distance to flag base
	if ((base = CTF_GetFlagBaseEnt(base_teamnum)) != NULL)
		return entdist(ent, base);
	
	// can't find base
	return 8192;

}

void CTF_JoinMenuHandler (edict_t *ent, int option)
{
	int			num=0;
	joined_t	*slot=NULL;

	// exit menu
	if (option == 3)
		return;

	// let them join if we are still in pre-game or were here before
	if (level.time < pregame_time->value || ((slot = GetJoinedSlot(ent)) != NULL))
	{
		StartGame(ent);

		// set their team value and then clear the slot
		if (slot)
		{
			ent->teamnum = slot->team;
			ClearJoinedSlot(slot);
		}

		return;
	}

	// always allow administrators into the game
	if (ent->myskills.administrator)
	{
		StartGame(ent);
		OrganizeTeams(true);
		return;
	}
}

void CTF_OpenJoinMenu (edict_t *ent)
{
	if (debuginfo->value)
		gi.dprintf("DEBUG: CTF_OpenJoinMenu()\n");

	if (!ShowMenu(ent))
        return;

	// new character
	if (!ent->myskills.class_num)
	{
		OpenClassMenu(ent, 1);
		return;
	}

	clearmenu(ent);

	if (!ent->myskills.administrator && !InJoinedQueue(ent) && (level.time > pregame_time->value))
	{
//							xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
		addlinetomenu(ent, "Vortex CTF", MENU_GREEN_CENTERED);
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, "The match has already", 0);
		addlinetomenu(ent, "begun. You will", 0);
		addlinetomenu(ent, "automatically join when", 0);
		addlinetomenu(ent, "a flag is captured.", 0);
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, "Exit", 3);

		setmenuhandler(ent, CTF_JoinMenuHandler);
		ent->client->menustorage.currentline = 9;
		showmenu(ent);

		// indicate that the player wants to join
		ent->client->waiting_to_join = true;
		ent->client->waiting_time = level.time;
		return;
	}

	addlinetomenu(ent, "Vortex CTF", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Your goal is to help your", 0);
	addlinetomenu(ent, "team capture the enemy's", 0);
	addlinetomenu(ent, "flag while simultaneously", 0);
	addlinetomenu(ent, "defending your own base.", 0);
	addlinetomenu(ent, "Work together to maximize", 0);
	addlinetomenu(ent, "points! Don't let your", 0);
	addlinetomenu(ent, "flag fall into enemy hands!", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Join", 1);
	addlinetomenu(ent, "Back", 2);
	addlinetomenu(ent, "Exit", 3);

	setmenuhandler(ent, CTF_JoinMenuHandler);
	ent->client->menustorage.currentline = 11;
	showmenu(ent);
}
