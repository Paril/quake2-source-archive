#include "g_local.h"
#include "flag.h"
#include "teamplay.h"
#include "runes.h"

// arrays are for {CTF_NOTEAM, CTF_TEAM1, CTF_TEAM2}
int flag_renderfx[] = {0, RF_SHELL_RED, RF_SHELL_BLUE};
gitem_t	*flag_item[] = {NULL, NULL, NULL};
char *flag_classnameforteam[] = {NULL, "item_flag_team1", "item_flag_team2"};

// fills in flag_item[]
void flag_init()
{
	flag_item[CTF_TEAM1] = FindItem("Red Flag");
	flag_item[CTF_TEAM2] = FindItem("Blue Flag");
	if (zoidctf->value)
	{
		flag_item[CTF_TEAM1]->world_model = "players/male/flag1.md2";
		flag_item[CTF_TEAM2]->world_model = "players/male/flag2.md2";
	}
}

// returns the team of the flag carried by ent
int	flag_has_flag(edict_t *ent)
{
	if (!ent->client)
		return CTF_NOTEAM;

	if (ent->client->pers.inventory[ITEM_INDEX(flag_item[CTF_TEAM1])])
		return CTF_TEAM1;

	if (ent->client->pers.inventory[ITEM_INDEX(flag_item[CTF_TEAM2])])
		return CTF_TEAM2;

	return CTF_NOTEAM;
}

// returns the team that a given flag is on
int flag_team(edict_t *flag)
{
	if (!flag)
		return CTF_NOTEAM;
	
	if (strcmp(flag->classname, flag_classnameforteam[CTF_TEAM1]) == 0)
		return CTF_TEAM1;

	if (strcmp(flag->classname, flag_classnameforteam[CTF_TEAM2]) == 0)
		return CTF_TEAM2;

	return CTF_NOTEAM;
}

// resets the flag of the given team
// assumes nobody is carrying it
void flag_reset(int team)
{
	edict_t *ent = NULL;
	while (ent = G_Find (ent, FOFS(classname), flag_classnameforteam[team])) {
		if (ent->spawnflags & DROPPED_ITEM)
			G_FreeEdict(ent);
		else
		{
			ent->svflags &= ~SVF_NOCLIENT;
			ent->solid = SOLID_TRIGGER;
			gi.linkentity(ent);
			ent->s.event = EV_ITEM_RESPAWN;
		}
	}
}

// respawns the given flag
void flag_respawn(edict_t *self)
{
	flag_reset(flag_team(self));	// will free self
}


// drops whatever flag you may be carrying
void flags_drop(edict_t *owner) {
	edict_t	*new_flag;
	int carried_team = flag_has_flag(owner);
	
	if (!carried_team)
		return;

	if (owner->client)
	{
		gi.bprintf(PRINT_MEDIUM, "%s lost the %s flag\n", owner->client->pers.netname, team_nameforteam[carried_team]);
		owner->client->pers.inventory[ITEM_INDEX(flag_item[carried_team])]--;
		if (owner->client->flag)
		{
			G_FreeEdict(owner->client->flag);
			owner->client->flag = NULL;
		}
	}

	new_flag = G_Spawn();

	new_flag->item = flag_item[carried_team];
	new_flag->classname = flag_classnameforteam[carried_team];
	new_flag->s.effects = new_flag->item->world_model_flags;
	new_flag->s.renderfx = flag_renderfx[carried_team];

	VectorSet(new_flag->mins, -16, -16, -32);
	VectorSet(new_flag->maxs,  16,  16,  16);

	gi.setmodel (new_flag, new_flag->item->world_model);
	new_flag->solid = SOLID_TRIGGER;
	new_flag->movetype = MOVETYPE_TOSS;  
	new_flag->touch = flag_touch;

	VectorCopy (owner->s.origin, new_flag->s.origin);
	VectorCopy (owner->velocity, new_flag->velocity);
	new_flag->velocity[2] = 300;

	new_flag->think = flag_respawn;
	new_flag->nextthink = level.time + 120;
	new_flag->groundentity = NULL;		// make sure it falls
	
	new_flag->spawnflags |= DROPPED_ITEM;

	gi.linkentity(new_flag);
}

// .think to make the flag follow flag->owner around
void flag_follow(edict_t *self) {
	// catch errors
	if (!self->owner || !self->owner->client || !self->owner->inuse) {
		G_FreeEdict(self);
		return;
	}
	VectorCopy(self->owner->s.origin, self->s.origin);
	VectorCopy(self->owner->velocity, self->velocity);
	gi.linkentity(self);
	self->nextthink = level.time + FRAMETIME;
}

void flag_spawn2(edict_t *owner, int team)
{
	edict_t *new_flag;

	new_flag = G_Spawn();

	new_flag->item = flag_item[team];
	new_flag->classname = "flag_fake";
	new_flag->s.effects = new_flag->item->world_model_flags;
	new_flag->s.renderfx = flag_renderfx[team];

	VectorSet(new_flag->mins, -16, -16, -32);
	VectorSet(new_flag->maxs,  16,  16,  16);

	gi.setmodel (new_flag, new_flag->item->world_model);
	new_flag->solid = SOLID_NOT;
	new_flag->movetype = MOVETYPE_NOCLIP;  

	new_flag->groundentity = NULL;		// make sure it falls

	new_flag->owner = owner;

	if (owner->client)
		owner->client->flag = new_flag;

	new_flag->think = flag_follow;
	new_flag->think(new_flag);

}


// a live client has touched the flag
qboolean flag_pickup(edict_t *self, edict_t *other) {
	edict_t	*from;
	int	self_team, carried_team;

	// team of the flag touched
	self_team = flag_team(self);
	// team of the flag carried by the person touching us
	carried_team = flag_has_flag(other);

	// gotta be on a team
	if (!other->client->resp.ctf_team)
		return false;

	if (self_team == other->client->resp.ctf_team) {
		// on same team as flag touched
		if (self->spawnflags & DROPPED_ITEM) {
			// flag is about.  return the flag to base
			gi.sound (self, CHAN_ITEM, gi.soundindex ("gunner/Gunatck3.wav"), 1, ATTN_NONE, 0);
			gi.bprintf(PRINT_MEDIUM, "%s returned the %s flag\n", other->client->pers.netname, team_nameforteam[self_team]);
			flag_reset(self_team); // frees self
			other->client->resp.score += 2;
		} else if (self->solid != SOLID_NOT && carried_team) {
			// we touched our own flag which is at base and we have
			// a flag.  we capture it
			other->client->pers.inventory[ITEM_INDEX(flag_item[carried_team])]--;
			if (other->client->flag)
			{
				G_FreeEdict(other->client->flag);
				other->client->flag = NULL;
			}
			gi.sound (self, CHAN_ITEM, gi.soundindex ("world/xianbeats.wav"), 1, ATTN_NONE, 0);
			gi.bprintf(PRINT_MEDIUM, "%s captured the %s flag\n", other->client->pers.netname, team_nameforteam[carried_team]);
			flag_reset(carried_team);
			other->client->resp.score += 5;		// 5 points plus 10 points below
			
			from = NULL;
			while (from = G_Find(from, FOFS(classname), "player")) {
				if (from->client && OnSameTeam(from, other))
					from->client->resp.score += 10;
			}
		}
	} else {
		// on opposite team from flag touched
		// pickup the flag
		other->client->pers.inventory[ITEM_INDEX(flag_item[self_team])]++;
		gi.sound (self, CHAN_ITEM, gi.soundindex ("world/klaxon2.wav"), 1, ATTN_NONE, 0);
		gi.bprintf(PRINT_MEDIUM, "%s got the %s flag\n", other->client->pers.netname, team_nameforteam[self_team]);
		if (!zoidctf->value)
			flag_spawn2(other, self_team);
		return (true);		// we picked it up
	}

	// we don't want to pick up the flag 
	return (false);

}

void flag_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {

	if (!other->client)
		return;
	if (other->health < 1)
		return;		// dead people can't pickup
	if (!ent->item->pickup(ent, other))
		return;		// player can't hold it


	// flash the screen
	other->client->bonus_alpha = 0.25;	

	// show icon and name on status bar
	other->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);
	other->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);
	other->client->pickup_msg_time = level.time + 3.0;

	if (ent->spawnflags & DROPPED_ITEM) {
		// picking up a dropped flag makes it go away
		G_FreeEdict(ent);
	} else {
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		gi.linkentity(ent);
	}
}


static void flag_zoid_think(edict_t *ent)
{
        if (ent->solid != SOLID_NOT)
                ent->s.frame = 173 + (((ent->s.frame - 173) + 1) % 16);
        ent->nextthink = level.time + FRAMETIME;
}

// points the lasers where they have to go
void flag_update_lasers(edict_t *l) {
	vec3_t	dest;
	trace_t	tr;

	if (!l->owner) {
		G_FreeEdict (l);
		return;
	}

	if (l->owner->solid != SOLID_NOT) {
		// flag at base
		VectorCopy(l->owner->s.origin, l->s.old_origin);
	} else {
		// flag elsewhere
		VectorCopy(l->s.origin, dest);
		dest[2] += 2048;
		tr = gi.trace(l->s.origin, NULL, NULL, dest, l, MASK_SOLID);
		VectorCopy(tr.endpos, l->s.old_origin);
	}
	gi.linkentity(l);

	l->nextthink = level.time + FRAMETIME;
}

// links the lasers into the world
void flag_lasers_link(edict_t *l) {
	vec3_t	dest;
	trace_t	tr;

	VectorCopy(l->s.origin, dest);
	dest[2] -= 128;
	tr = gi.trace(l->s.origin, NULL, NULL, dest, l, MASK_SOLID);
	VectorCopy(tr.endpos, l->s.origin);

	gi.linkentity(l);
	
	l->think = flag_update_lasers;
	l->nextthink = level.time + FRAMETIME;
}


// spawns the 4 lasers for a flag
void flag_lasers_spawn(edict_t *flag) {
	static	vec3_t	offset[4] = {{16,16,0}, {-16,16,0}, {-16,-16,0}, {16,-16,0}};
	edict_t	*l[4];		// could eliminate need for this to be an array
	int		i;

	for (i=0; i<4; i++) {
		l[i] = G_Spawn();
//		l[i]->flags &= FL_TEAMSLAVE;
		l[i]->owner = flag;
		
		// set up individual positions
		VectorAdd(flag->s.origin, offset[i], l[i]->s.origin);

//		l[i]->noblock = true;
		
		l[i]->movetype = MOVETYPE_NONE;
		l[i]->solid = SOLID_NOT;
		l[i]->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
		l[i]->s.modelindex = 1;			// must be non-zero

		// set the width
		l[i]->s.frame = 4;

		// set the color
		if (flag_team(flag) == CTF_TEAM1)
			l[i]->s.skinnum = 0xf2f2f0f0;	// red
		else
			l[i]->s.skinnum = 0xf3f3f1f1;	// blue

		VectorSet (l[i]->mins, -8, -8, -8);
		VectorSet (l[i]->maxs, 8, 8, 8);
	
		l[i]->think = flag_lasers_link;
		l[i]->nextthink = level.time + 2 * FRAMETIME;
	}
}

// links the flag into the world	
void flag_place (edict_t *flag) {
	trace_t		tr;
	vec3_t		dest;

	VectorSet(flag->mins, -16, -16, -32);
	VectorSet(flag->maxs,  16,  16,  16);

	gi.setmodel (flag, flag->item->world_model);
	flag->solid = SOLID_TRIGGER;
	flag->movetype = MOVETYPE_TOSS;  
	flag->touch = flag_touch;

	VectorAdd (flag->s.origin, tv(0,0,-128), dest);

	tr = gi.trace (flag->s.origin, flag->mins, flag->maxs, dest, flag, MASK_SOLID);

	if (tr.startsolid)
	{
		gi.dprintf ("flag_place: %s startsolid at %s\n", flag->classname, vtos(flag->s.origin));
		G_FreeEdict (flag);
		return;
	}
	VectorCopy (tr.endpos, flag->s.origin);
//	VectorCopy (flag->s.origin, flag->baseorigin);

	gi.linkentity (flag);

	flag_lasers_spawn(flag);

	if (zoidctf->value)
	{
		flag->think = flag_zoid_think;
		flag->nextthink = level.time + FRAMETIME;
	}
}


// spawn a flag with team = {CTF_TEAM1, CTF_TEAM2} as appropriate
// and a corresponding base
void flag_spawn (edict_t *flag, int team) {
	gitem_t *item;

	// do not spawn 
	if (!ctf->value)
		return;

// q2ctf hack
	if (strstr(level.mapname, "q2ctf") || strstr(level.mapname, "Q2CTF"))
		flag->s.origin[2] += 64;
// end q2ctf hack
	item = FindItem( (team == CTF_TEAM1) ? "Red Flag" : "Blue Flag");
//	flag->flagstat = FLAG_ATBASE;

	PrecacheItem (item);
	flag->item = item;
	flag->think = flag_place;
	flag->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	flag->s.effects = item->world_model_flags;
	flag->s.renderfx = flag_renderfx[team];
//	flag->noblock = true;		// haha, can't block anything!
}

void SP_item_flag_team1 (edict_t *ent) {
	flag_spawn(ent, CTF_TEAM1);
}

void SP_item_flag_team2 (edict_t *ent) {
	flag_spawn(ent, CTF_TEAM2);
}

int flag_state(int team, edict_t **carrier)
{
	edict_t	*e = NULL;
	if (carrier) *carrier = NULL;
	
	// look for dropped flags
	while (e = G_Find(e, FOFS(classname), flag_classnameforteam[team]))
	{
		if (e->spawnflags & DROPPED_ITEM)
			return FLAG_ABOUT;
	}

	// no dropped flags
	// see if we found a flag and it is touchable
	e = G_Find(NULL, FOFS(classname), flag_classnameforteam[team]);
	if (e && e->solid != SOLID_NOT)
	{
			return FLAG_ATBASE;
	}

	// someone has it
	e = NULL;
	while (e = G_Find(e, FOFS(classname), "player"))
	{
		if (flag_has_flag(e) == team)
		{
			if (carrier) *carrier = e;
			return FLAG_CARRIED;
		}
	}
	
	// don't know where it is
	flag_reset(team);
	return -1;
}

// prints the status of the flags
void Cmd_FlagStat_f (edict_t *ent) {
	edict_t	*carrier;
	char	*state, *s_carrier;
	int		i;

	for (i = CTF_TEAM1; i <= CTF_TEAM2; i++) {
		s_carrier = "";
		switch (flag_state(i, &carrier)) {
			case FLAG_ATBASE:
				state = "at base";
				break;
			case FLAG_CARRIED:
				state = "carried by";
				if (carrier && carrier->client)
				{
					if (ent == carrier)
						s_carrier = "you";
					else
						s_carrier = carrier->client->pers.netname;
				}
				break;
			case FLAG_ABOUT:
				state = "lying about";
				break;
			default:
				state = "missing";
		}
		gi.cprintf(ent, PRINT_HIGH, "%s is %s %s\n", (i == CTF_TEAM1) ? "RED " : "BLUE", state, s_carrier);
	}
}

// returns distance between a player and his/her own flag
/*
int	flag_range_to (edict_t *ent) {
	edict_t	*flag;
	vec3_t	dir;
	if (!ent->client) return (9999);
	if (IS_RED(ent))
		flag = G_Find(NULL, FOFS(classname), "item_flag_team1");
	else
		flag = G_Find(NULL, FOFS(classname), "item_flag_team2");
	if (!flag) return (9999);
	VectorSubtract(flag->s.origin, ent->s.origin, dir);
	return (VectorLength(dir));
}
*/
void flag_set_effects (edict_t *player)
{
	if (zoidctf->value)
		if (player->health > 0) {
			if (flag_has_flag(player) == CTF_TEAM1) {
				player->s.effects |= EF_FLAG1;
				player->s.modelindex3 = gi.modelindex(flag_item[CTF_TEAM1]->world_model);
			} else if (flag_has_flag(player) == CTF_TEAM2) {
				player->s.effects |= EF_FLAG2;
				player->s.modelindex3 = gi.modelindex(flag_item[CTF_TEAM2]->world_model);
			} else {
				player->s.modelindex3 = 0;
			}
		}
}