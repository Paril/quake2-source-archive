#include "g_local.h"
#include "m_player.h"

#define GIEX_VERSION "v0.42"
void ClientUserinfoChanged (edict_t *ent, char *userinfo);

void SP_misc_teleporter_dest (edict_t *ent);

//
// Gross, ugly, disgustuing hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

static void SP_FixCoopSpots (edict_t *self) {
	edict_t	*spot;
	vec3_t	d;

	spot = NULL;

	while(1)	{
		spot = G_FindClassId(spot, CI_P_START);
		if (!spot)
			return;
		if (!spot->targetname)
			continue;
		VectorSubtract(self->s.origin, spot->s.origin, d);
		if (VectorLength(d) < 384) {
			if ((!self->targetname) || Q_strcasecmp(self->targetname, spot->targetname) != 0) {
//				gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
				self->targetname = spot->targetname;
			}
			return;
		}
	}
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

static void SP_CreateCoopSpots (edict_t *self) {
	edict_t	*spot;

	if(Q_strcasecmp(level.mapname, "security") == 0) {
		spot = G_Spawn();
		spot->classid = CI_P_COOP;
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 - 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classid = CI_P_COOP;
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classid = CI_P_COOP;
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 128;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		return;
	}
}


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self) {
	if (!coop->value)
		return;
	if(Q_strcasecmp(level.mapname, "security") == 0) {
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_CreateCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
	if (!deathmatch->value) {
		G_FreeEdict (self);
		return;
	}
	SP_misc_teleporter_dest (self);
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

void SP_info_player_coop(edict_t *self)
{
	if (!coop->value) {
		G_FreeEdict (self);
		return;
	}

	if((Q_strcasecmp(level.mapname, "jail2") == 0)   ||
	   (Q_strcasecmp(level.mapname, "jail4") == 0)   ||
	   (Q_strcasecmp(level.mapname, "mine1") == 0)   ||
	   (Q_strcasecmp(level.mapname, "mine2") == 0)   ||
	   (Q_strcasecmp(level.mapname, "mine3") == 0)   ||
	   (Q_strcasecmp(level.mapname, "mine4") == 0)   ||
	   (Q_strcasecmp(level.mapname, "lab") == 0)     ||
	   (Q_strcasecmp(level.mapname, "boss1") == 0)   ||
	   (Q_strcasecmp(level.mapname, "fact3") == 0)   ||
	   (Q_strcasecmp(level.mapname, "biggun") == 0)  ||
	   (Q_strcasecmp(level.mapname, "space") == 0)   ||
	   (Q_strcasecmp(level.mapname, "command") == 0) ||
	   (Q_strcasecmp(level.mapname, "power2") == 0) ||
	   (Q_strcasecmp(level.mapname, "strike") == 0))
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_FixCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(void)
{
}


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage) {
	// player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent) {
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	return false;
}

qboolean IsNeutral (edict_t *ent) {
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
		return true;
	return false;
}

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker) {
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;
	qboolean weapon = false; // If player kills player with a weapon, set to true and append weapon's item name

	mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	if (deathmatch->value || coop->value) {
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = "";

		switch (mod) {
		case MOD_SUICIDE:
			message = "suicides";
			break;
		case MOD_FALLING:
			message = "cratered";
			break;
		case MOD_CRUSH:
			message = "was squished";
			break;
		case MOD_WATER:
			message = "sank like a rock";
			break;
		case MOD_SLIME:
			message = "melted";
			break;
		case MOD_LAVA:
			message = "does a back flip into the lava";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "blew up";
			break;
		case MOD_EXIT:
			message = "found a way out";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			break;
		case MOD_TARGET_BLASTER:
			message = "got blasted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			break;
		}
		if (attacker == self) {
			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = "tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsNeutral(self))
					message = "tripped on its own grenade";
				else if (IsFemale(self))
					message = "tripped on her own grenade";
				else
					message = "tripped on his own grenade";
				break;
			case MOD_R_SPLASH:
				if (IsNeutral(self))
					message = "blew itself up";
				else if (IsFemale(self))
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
			case MOD_DRAIN:
				if (IsNeutral(self))
					message = "drained itself to death";
				else if (IsFemale(self))
					message = "drained herself to death";
				else
					message = "drained himself to death";
				break;
			case MOD_LEECH:
				if (IsNeutral(self))
					message = "tried to leech more than it could handle";
				else if (IsFemale(self))
					message = "tried to leech more than she could handle";
				else
					message = "tried to leech more than he could handle";
				break;
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				break;
			case MOD_BLACKHOLE:
				if (IsNeutral(self))
					message = "was caught in it's own black hole";
				else if (IsFemale(self))
					message = "was caught in her own black hole";
				else
					message = "was caught in his own black hole";
				break;
			case MOD_FIREBALL:
				if (IsNeutral(self))
					message = "burnt its feet with a fireball";
				else if (IsFemale(self))
					message = "burnt her feet with a fireball";
				else
					message = "burnt his feet with a fireball";
				break;
			case MOD_PLAGUEBOMB:
				if (IsNeutral(self))
					message = "caught its own plague";
				else if (IsFemale(self))
					message = "caught her own plague";
				else
					message = "caught his own plague";
				break;
			case MOD_CORPSEEXPLOSION:
				message = "should stay away from the exploding corpses";
				break;
			case MOD_DRONE:
				if (IsNeutral(self))
					message = "blew itself to small pieces with its drones";
				else if (IsFemale(self))
					message = "blew herself to small pieces with her drones";
				else
					message = "blew himself to small pieces with his drones";
				break;
			}
		}
		if (message) {
			gi.bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
				self->client->resp.score--;
			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client) {
			switch (mod) {
			case MOD_BLASTER:
				message = "was blasted by";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_SHOTGUN:
				message = "was gunned down by";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_SSHOTGUN:
				message = "was blown away by";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_MACHINEGUN:
				message = "was machinegunned by";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_CHAINGUN:
				message = "was cut in half by";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_GRENADE:
				message = "was popped by a grenade from";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_G_SPLASH:
				message = "was shredded by shrapnel from";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_ROCKET:
				message = "sucked on";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_R_SPLASH:
				message = "couldn't dodge";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_HYPERBLASTER:
				message = "was melted by";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_RAILGUN:
				message = "bit a slug from";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_BFG_LASER:
				message = "saw the pretty lights from";
				message2 = "'s BFG";
				break;
			case MOD_BFG_BLAST:
				message = "was disintegrated by";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_BFG_EFFECT:
				message = "couldn't hide from";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_HANDGRENADE:
				message = "caught";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_HG_SPLASH:
				message = "didn't see";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_HELD_GRENADE:
				message = "felt the pain of";
				message2 = "'s";
				weapon = true;
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
			case MOD_INFERNO:
				message = "was burnt by";
				message2 = "'s inferno";
				break;
			case MOD_FIREBOLT:
				message = "was pummeled to death by";
				message2 = "'s firebolts";
				break;
			case MOD_FIREBALL:
				message = "got in the way of";
				message2 = "'s fireball";
				break;
			case MOD_PLAGUEBOMB:
				message = "inhaled too much of";
				message2 = "'s plague bomb";
				break;
			case MOD_CORPSEEXPLOSION:
				message = "stood too close to";
				message2 = "'s exploding corpses";
				break;
			case MOD_LASERMINE:
				message = "was cut up by";
				message2 = "'s laser grid";
				break;
			case MOD_DRAIN:
				message = "was drained dry by";
				break;
			case MOD_LEECH:
				message = "was leeched to death by";
				break;
			case MOD_BLACKHOLE:
				message = "was sucked into";
				message2 = "'s black hole";
				break;
			case MOD_SPORE:
				message = "couldn't avoid";
				message2 = "'s corpse spores";
				break;
			case MOD_GIBRAIN:
				message = "should have stayed out of";
				message2 = "'s rain of gibs";
				break;
			case MOD_LIGHTNING:
				message = "was fried by";
				message2 = "'s lightning";
				break;
			case MOD_SPARK:
				message = "got shocked by";
				message2 = "'s sparks";
				break;
			case MOD_BOLT:
				message = "was bolted by";
				break;
			case MOD_LASERBEAM:
				message = "was toasted by";
				message2 = "'s laserbeam";
				break;
			case MOD_ARM:
				message = "couldn't escape";
				message2 = "'s grasp";
				break;
			case MOD_BITE:
				message = "was bitten by";
				break;
			case MOD_DRONE:
				message = "was torn apart by";
				message2 = "'s drone support";
				break;
			case MOD_BURST:
				message = "got in the way of";
				message2 = "'s corpse burst";
				break;
			case MOD_BLAZE:
				message = "was set ablaze by";
				break;
			case MOD_SWARM:
				message = "was stung by";
				message2 = "'s swarm";
				break;
			case MOD_STORM:
				message = "was struck down by";
				message2 = "'s lightning storm";
				break;
			}
			if (message) {
				if (weapon) {
					iteminfo_t *iteminfo = getWornItemInfo(attacker, GIEX_ITEM_WEAPON);
					gi.bprintf (PRINT_MEDIUM,"%s %s %s%s %s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2, iteminfo->name);
				} else {
					gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				}
				if (deathmatch->value) {
					if (ff)
						attacker->client->resp.score--;
					else
						attacker->client->resp.score++;
				}
				return;
			}
		}
	}

	if (attacker->svflags & SVF_MONSTER) {
		if (mod == MOD_TANKSTRIKE) {
			gi.bprintf(PRINT_MEDIUM, "%s was pounded to a pulp by %s\n", self->client->pers.netname, attacker->monsterinfo.name);
		} else {
			gi.bprintf(PRINT_MEDIUM, "%s was killed by %s\n", self->client->pers.netname, attacker->monsterinfo.name);
		}
		return;
	}

	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (deathmatch->value)
		self->client->resp.score--;
}


void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	qboolean	quad;
	float		spread;

	if (!deathmatch->value)
		return;

	item = self->client->pers.weapon;
	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Blaster") == 0))
		item = NULL;

/*	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else*/

	quad = (self->client->quad_framenum > (level.framenum + 10));

	if (item && quad)
		spread = 22.5;
	else
		spread = 0.0;

	if (item)
	{
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item (self, item);
		self->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
	}

	if (quad)
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassid (CI_I_QUAD));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

	if (attacker && attacker != world && attacker != self)
	{
		VectorSubtract (attacker->s.origin, self->s.origin, dir);
	}
	else if (inflictor && inflictor != world && inflictor != self)
	{
		VectorSubtract (inflictor->s.origin, self->s.origin, dir);
	}
	else
	{
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	if (dir[0])
		self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
	else {
		self->client->killer_yaw = 0;
		if (dir[1] > 0)
			self->client->killer_yaw = 90;
		else if (dir[1] < 0)
			self->client->killer_yaw = -90;
	}
	if (self->client->killer_yaw < 0)
		self->client->killer_yaw += 360;


}

/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point) {
	int		n;

	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	//r1: only copy if not NULL (since player_die can be called multiple times)
	if (self->client->pers.weapon) {
		self->client->pers.old_weapon = self->client->pers.weapon;
		self->client->pers.weapon = NULL;
	}
	if (self->client->flashlight) {
		G_FreeEdict(self->client->flashlight);
		self->client->flashlight = NULL;
	}

	self->maxs[2] = -8;

//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag) {
		deductExp(attacker, self);
		saveCharacter(self);
		self->client->respawn_time = level.time + 1.0;
//		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);
		// TossClientWeapon (self);
		if (deathmatch->value)
			Cmd_Help_f (self);		// show scores

		// clear inventory
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < game.num_items; n++) {
//			if (coop->value && itemlist[n].flags & IT_KEY)
				self->client->resp.coop_respawn.inventory[n] = self->client->pers.inventory[n];
//			self->client->pers.inventory[n] = 0;
		}
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;

	if (self->health < self->gib_health) {	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead (self, damage);

		self->takedamage = DAMAGE_NO;
	} else {	// normal death
		if (!self->deadflag) {
			static int i;

			i = (i+1)%3;
			// start a death animation
			self->client->anim_priority = ANIM_DEATH;
			if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				self->s.frame = FRAME_crdeath1-1;
				self->client->anim_end = FRAME_crdeath5;
			}
			else switch (i)
			{
			case 0:
				self->s.frame = FRAME_death101-1;
				self->client->anim_end = FRAME_death106;
				break;
			case 1:
				self->s.frame = FRAME_death201-1;
				self->client->anim_end = FRAME_death206;
				break;
			case 2:
				self->s.frame = FRAME_death301-1;
				self->client->anim_end = FRAME_death308;
				break;
			}
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}

	self->deadflag = DEAD_DEAD;

	gi.linkentity (self);
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant (gclient_t *client)
{
	gitem_t		*item;
	//int			inven[MAX_ITEMS];

	memset (&client->pers, 0, sizeof(client->pers));

	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.weapon = item;
	client->pers.inventory[client->pers.selected_item] = 1;

/*	item = FindItem("Shotgun");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Super Shotgun");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Machinegun");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Chaingun");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Grenade launcher");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Rocket launcher");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Hyperblaster");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Railgun");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("BFG 10k");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
*/

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 50;

	client->pers.connected = true;
}


void InitClientResp (gclient_t *client)
{
	memset (&client->resp, 0, sizeof(client->resp));
	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;
}

/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData (void)
{
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;
		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
		game.clients[i].pers.savedFlags = (ent->flags & (FL_GODMODE|FL_NOTARGET|FL_POWER_ARMOR));
		if (coop->value)
			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	ent->flags |= ent->client->pers.savedFlags;
	if (coop->value)
		ent->client->resp.score = ent->client->pers.score;
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot (edict_t *spot)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;

	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (void) {
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_FindClassId (spot, CI_P_DEATHMATCH)) != NULL) {
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1) {
			range1 = range;
			spot1 = spot;
		} else if (range < range2) {
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2) {
		spot1 = spot2 = NULL;
	} else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do {
		spot = G_FindClassId (spot, CI_P_DEATHMATCH);
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
/*
edict_t *SelectFarthestDeathmatchSpawnPoint (void)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_FindClassId(spot, CI_P_DEATHMATCH)) != NULL)
	{
		bestplayerdistance = PlayersRangeFromSpot (spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
	{
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_FindClassId(NULL, CI_P_DEATHMATCH);

	return spot;
}
*/
edict_t *SelectDeathmatchSpawnPoint (void) {
//	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
//		return SelectFarthestDeathmatchSpawnPoint ();
//	else
		return SelectRandomDeathmatchSpawnPoint ();
}


edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

	index = ent->client - game.clients;

	// player 0 starts in normal player spawn point
	if (!index)
		return NULL;

	spot = NULL;

	// assume there are four coop spots at each spawnpoint
	while (1)
	{
		spot = G_FindClassId(spot, CI_P_COOP);
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_strcasecmp(game.spawnpoint, target) == 0 )
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}


	//return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles) {
	edict_t	*spot = NULL;

	if (deathmatch->value)
		spot = SelectDeathmatchSpawnPoint ();
	else if (coop->value)
		spot = SelectCoopSpawnPoint (ent);

	// find a single player start spot
	if (!spot) {
		while ((spot = G_FindClassId(spot, CI_P_START)) != NULL) {
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_strcasecmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (!spot) {
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_FindClassId(spot, CI_P_START);
			}
			if (!spot)
				gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
		}
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);
}

//======================================================================


void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;

	level.body_que = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classid = CI_P_BODYQUE;
		ent->classname = "bodyque";
	}
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	n;

	if (self->health < self->gib_health) {
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		self->s.origin[2] -= 48;
		ThrowClientHead (self, damage);
		self->takedamage = DAMAGE_NO;
	}
}

void CopyToBodyQue (edict_t *ent)
{
	edict_t		*body;

	// grab a body que and cycle to the next one
	body = &g_edicts[(int)maxclients->value + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// FIXME: send an effect on the removed body

	gi.unlinkentity (ent);

	gi.unlinkentity (body);
	body->s = ent->s;
	body->s.number = body - g_edicts;

	body->svflags = ent->svflags;
	VectorCopy (ent->mins, body->mins);
	VectorCopy (ent->maxs, body->maxs);
	VectorCopy (ent->absmin, body->absmin);
	VectorCopy (ent->absmax, body->absmax);
	VectorCopy (ent->size, body->size);
	body->solid = ent->solid;
	body->clipmask = ent->clipmask;
	body->owner = ent->owner;
	body->movetype = ent->movetype;
	body->gib_health = -(100 + ent->radius_dmg * 15);
	body->die = body_die;
	body->takedamage = DAMAGE_YES;
	body->deadflag = DEAD_DEAD;

	gi.linkentity (body);
}


void respawn (edict_t *self)
{
	if (deathmatch->value || coop->value)
	{
		// spectator's don't leave bodies
		if (self->movetype != MOVETYPE_NOCLIP)
			CopyToBodyQue (self);
		self->svflags &= ~SVF_NOCLIENT;
		PutClientInServer (self);

		// add a teleportation effect
		self->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

		self->client->respawn_time = level.time;

		return;
	}

	// restart the entire server
	gi.AddCommandString ("menu_loadgame\n");
}

/*
 * only called when pers.spectator changes
 * note that resp.spectator should be the opposite of pers.spectator here
 */
int loadCharacter(edict_t *ent, char *password);
void spectator_respawn (edict_t *ent) {
	int i, numspec;

	// if the user wants to become a spectator, make sure he doesn't
	// exceed max_spectators

	if (ent->client->pers.spectator) {
		char *value;

		if (ent->client->pers.loggedin) {
			gi.cprintf(ent, PRINT_HIGH, "Can't go spectator after logging in.\n");
			ent->client->pers.spectator = false;
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 0\n");
			gi.unicast(ent, true);
			return;
		}

		value = Info_ValueForKey (ent->client->pers.userinfo, "spectator");
		if (*spectator_password->string &&
			strcmp(spectator_password->string, "none") &&
			strcmp(spectator_password->string, value)) {
			gi.cprintf(ent, PRINT_HIGH, "Spectator password incorrect.\n");
			ent->client->pers.spectator = false;
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 0\n");
			gi.unicast(ent, true);
			return;
		}

		// count spectators
		for (i = 1, numspec = 0; i <= maxclients->value; i++)
			if (g_edicts[i].inuse && g_edicts[i].client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value) {
			gi.cprintf(ent, PRINT_HIGH, "Server spectator limit is full.");
			ent->client->pers.spectator = false;
			// reset his spectator var
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 0\n");
			gi.unicast(ent, true);
			return;
		}
	} else {
		// he was a spectator and wants to join the game
		// he must have the right password
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "password");
		if (*password->string && strcmp(password->string, "none") &&
			strcmp(password->string, value)) {
			gi.cprintf(ent, PRINT_HIGH, "Password incorrect.\n");
			ent->client->pers.spectator = true;
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 1\n");
			gi.unicast(ent, true);
			return;
		}
		if (!ent->client->pers.loggedin) {
//			gi.cprintf(ent, PRINT_HIGH, "Use \"loadchar <password>\" to play\n");
			ent->client->pers.spectator = true;
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 1\n");
			gi.unicast(ent, true);
			return;
		}
	}

	// clear client on respawn
	//ent->client->resp.score = ent->client->pers.score = 0;

	Info_SetValueForKey (ent->client->pers.userinfo, "spectator", "0");
	ent->svflags &= ~SVF_NOCLIENT;
	PutClientInServer (ent);

	// add a teleportation effect
	if (!ent->client->pers.spectator)  {
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
	}

	ent->client->respawn_time = level.time;

	if (!ent->client->pers.spectator) {
		gi.bprintf (PRINT_HIGH, "%s joined the game\n", ent->client->pers.netname);
		//loadCharacter(ent, ent->client->pers.skills.password);
	}
}

//==============================================================


/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void printVotePrivate(edict_t *target);
void PutClientInServer (edict_t *ent) {
	char		userinfo[MAX_INFO_STRING];
	static vec3_t	mins = {-16, -16, -24};
	static vec3_t	maxs = {16, 16, 32};
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;
	iteminfo_t *info;
	gitem_t		*item;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	index = ent-g_edicts-1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
/*	if (deathmatch->value)
	{


		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);
	}*/
	if (0) {
//		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
//		memcpy (resp.coop_respawn.inventory, client->pers.inventory, sizeof(client->pers.inventory));
		// this is kind of ugly, but it's how we want to handle keys in coop
//		for (n = 0; n < game.num_items; n++)
//		{
//			if (itemlist[n].flags & IT_KEY)
//				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
//		}
		resp.coop_respawn.game_helpchanged = client->pers.game_helpchanged;
		resp.coop_respawn.helpchanged = client->pers.helpchanged;
		resp.coop_respawn.skills = client->pers.skills;
		resp.coop_respawn.loggedin = client->pers.loggedin;
//		resp.coop_respawn.weapon = client->pers.weapon;
		resp.coop_respawn.selspell = client->pers.selspell;

		client->pers = resp.coop_respawn;
		ClientUserinfoChanged (ent, userinfo);
		if (resp.score > client->pers.score)
			client->pers.score = resp.score;
	} else {
		memset (&resp, 0, sizeof(resp));
	}

	// clear everything but the persistant data
	saved = client->pers;
	memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
	memset (client, 0, sizeof(*client));
	//InitClientPersistant (client);
	client->pers = saved;
	ClientUserinfoChanged (ent, userinfo);

	//if (client->pers.health <= 0)
	//	InitClientPersistant(client);
	client->resp = resp;

	if (!client->pers.weapon)
		client->pers.weapon = client->pers.old_weapon;

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	// clear entity values
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classid = CI_PLAYER;
	ent->classname = "player";
	ent->mass = 200;
	ent->gib_health = -100;
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
	ent->svflags &= ~SVF_DEADMONSTER;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

/*	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{*/
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
//	}

	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	ent->s.effects = 0;
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->s.modelindex2 = 255;		// custom gun model
	// sknum is player num and weapon number
	// weapon number will be added in changeweapon
	ent->s.skinnum = ent - g_edicts - 1;

	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
	{
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);
	}

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	// spawn a spectator
	if (client->pers.spectator) {
		client->chase_target = NULL;

		client->resp.spectator = true;

		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;
		ent->client->showscores = true;
		ent->client->showmenu = 5; //Display main menu
		showGiexMainMenu(ent);
		gi.linkentity (ent);
		return;
	} else
		client->resp.spectator = false;

	if (!KillBox (ent)) { // could't spawn in?
	}

	gi.linkentity (ent);

	// force the current weapon up
	info = getWornItemInfo(ent, 0);
//	gi.dprintf("name: %s\n", info->name);
	if (info->name[0] == '\0') {
		item = FindItem("Blaster");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.weapon = item;
		client->pers.inventory[client->pers.selected_item] = 1;
	} else {
		client->pers.weapon = info->item;
	}

	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);
	ApplyMax(ent);
	ent->health = ent->max_health;
	ent->client->magic = ent->client->max_magic;

	// respawn armor
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Body Armor"))] = ceil(0.1 * ent->client->max_armor * ent->client->pers.skill[22]);

	// Count skill levels
	ent->radius_dmg = 0;
	for (i = 0; i<GIEX_NUMCLASSES; i++)
		ent->radius_dmg += ent->client->pers.skills.classLevel[i];

	// Add some invulerability at spawn
	if (ent->client->invincible_framenum < level.framenum + 30)
		ent->client->invincible_framenum = level.framenum + 30;
	if ((level.vote_time > level.time) && (!ent->client->pers.vote))
		printVotePrivate(ent);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch (edict_t *ent)
{
	G_InitEdict (ent);

	InitClientResp (ent->client);

	// locate ent at a spawn point
	PutClientInServer (ent);

	if (level.intermissiontime) {
		MoveClientToIntermission (ent);
	} else {
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	if (ent->client->pers.loggedin) {
		if (deathmatch->value && teams->value) {
			assignToTeam(ent);
		}
		gi.bprintf (PRINT_HIGH, "%s joined the game\n", ent->client->pers.netname);
	} else {
		gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
	}
	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
	int		i;

	ent->client = game.clients + (ent - g_edicts - 1);

	gi.WriteByte (svc_stufftext);
	gi.WriteString ("alias +castspell \"cmd startspell\"\n");
	gi.unicast(ent, true);

	gi.WriteByte (svc_stufftext);
	gi.WriteString ("alias -castspell \"cmd stopspell\"\n");
	gi.unicast(ent, true);

	if (!ent->client->pers.loggedin) {
		gi.WriteByte (svc_stufftext);
		gi.WriteString ("spectator 1\n");
		gi.unicast(ent, true);
	}

	if (deathmatch->value) {
		ClientBeginDeathmatch (ent);
		return;
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == true)
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect() time
		G_InitEdict (ent);
		ent->classid = CI_PLAYER;
		ent->classname = "player";
		InitClientResp (ent->client);
		PutClientInServer (ent);
	}

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect if in a multiplayer game
		if (game.maxclients > 1)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_LOGIN);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
		}
	}

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo) {
	char	*s;
	char	msg[128];
	int	playernum;

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo)) {
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}

	// set name
	s = Info_ValueForKey (userinfo, "name");
	s[15] = '\0'; //Player name limited to 15 chars
	if (strcmp(ent->client->pers.netname, s) && strlen(ent->client->pers.netname)) {
		sprintf(msg, "%s changed name to %s.\n", ent->client->pers.netname, s);
		logmsg(msg);
		gi.bprintf(PRINT_HIGH, msg);
	}
	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	// set spectator
	s = Info_ValueForKey (userinfo, "spectator");
	// spectators are only supported in deathmatch
	if (*s && strcmp(s, "0"))
		ent->client->pers.spectator = true;
	else
		ent->client->pers.spectator = false;

	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

	// fov
/*	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV)) {
		ent->client->ps.fov = 90;
	} else {*/
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
//	}

	// handedness
	s = Info_ValueForKey (userinfo, "hand");
	if (strlen(s)) {
		ent->client->pers.hand = atoi(s);
	}

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
int getIpStringLength (const char *ip)
{
	int i = 0;

	while((ip[i] != ':') && (ip[i] != 0)) {
		i++;
	}
	return i;
}

//r1: added (warning, static string)
char *TimeToString (int secs)
{
	int days = 0;
	int hours = 0;
	int mins = 0;
	int	years = 0;
	int weeks = 0;
	static char banmsg[64];

	banmsg[0] = '\0';

	while (secs/60/60/24/365.25 >= 1) {
		years++;
		secs -= 60*60*24*365.25;
	}

	if (years > 0) {
		strcat (banmsg, va("%d year",years));
		if (years != 1)
			strcat (banmsg, "s");
	}

	while (secs/60/60/24/7 >= 1) {
		weeks++;
		secs -= 60*60*24*7;
	}

	if (weeks > 0) {
		if (banmsg[0])
			strcat (banmsg, va(", %d week",weeks));
		else
			strcat (banmsg, va("%d week", weeks));

		if (weeks != 1)
			strcat (banmsg, "s");
	}

	while (secs/60/60/24 >= 1) {
		days++;
		secs -= 60*60*24;
	}

	if (days > 0) {
		if (banmsg[0])
			strcat (banmsg, va(", %d day",days));
		else
			strcat (banmsg, va("%d day", days));

		if (days != 1)
			strcat (banmsg, "s");
	}

	while (secs/60/60 >= 1) {
		hours++;
		secs -= 60*60;
	}
	if (hours > 0) {
		if (banmsg[0])
			strcat (banmsg, va(", %d hour",hours));
		else
			strcat (banmsg, va("%d hour", hours));

		if (hours != 1)
			strcat (banmsg, "s");
	}

	while (secs/60 >= 1) {
		mins++;
		secs -= 60;
	}
	if (mins > 0) {
		if (banmsg[0])
			strcat (banmsg, va(", %d min",mins));
		else
			strcat (banmsg, va("%d min", mins));

		if (mins != 1)
			strcat (banmsg, "s");
	}

	if (secs > 0) {
		if (banmsg[0])
			strcat (banmsg, va(", %d sec",secs));
		else
			strcat (banmsg, va("%d sec", secs));

		if (secs != 1)
			strcat (banmsg, "s");
	}

	return banmsg;
}

//r1: hack
#define	BAN_UNABLE_TO_DETERMINE_IP	-1
#define	BAN_NOT_BANNED				-2

qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	int		baninfo;
	char	*ipa;
	char	*value;
	char	msg[64];
	edict_t	*scan;
	int		i;

	// check to see if they are on the banned IP list

	//r1: this borks on broken q2
	/*value = Info_ValueForKey (userinfo, "ip");
	if (SV_FilterPacket(value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}
	value[getIpStringLength(value)] = 0;
	strcpy(ip, value);*/

	// find \ip variable
	ipa = strstr(userinfo,"\\ip\\");

	if (ipa == NULL) {
		userinfo[0] = '\0';
		Info_SetValueForKey(userinfo,"rejmsg","Your userinfo string is malformed, please restart your Quake 2.");
		return false;
	}

	// skip "\ip\"
	ipa += 4;

	// if \ip is on banned list, kick the player
	baninfo = SV_FilterPacket(ipa);

	//exception ban (couldn't determine IP)
	if (baninfo == BAN_UNABLE_TO_DETERMINE_IP) {
		userinfo[0] = '\0';
		Info_SetValueForKey(userinfo,"rejmsg","Your userinfo string is malformed, please restart your Quake 2.");
		return false;
	}

	if (baninfo != BAN_NOT_BANNED) {
		//r1: empty out the userinfo, exe only looks for rejmsg after this point so the string can safely
		//    be destroyed.
		userinfo[0] = '\0';

		if (ipfilters[baninfo].expiretime > 0) {
			Info_SetValueForKey (userinfo, "rejmsg",va("You are banned for %s (id=%-2.2X)", TimeToString(ipfilters[baninfo].expiretime - time(0)), baninfo));
			return false;
		} else {
			Info_SetValueForKey (userinfo, "rejmsg", "You are banned from this server.");
			return false;
		}
	}

	for (i=0 ; i<maxclients->value ; i++) {
		scan = g_edicts + 1 + i;
		if (!scan->inuse)
			continue;
		if (!scan->client)
			continue;
		if (scan == ent)
			continue;
		if (!Q_stricmp(ipa, scan->client->pers.ip) && (Q_stricmp(ipa, "127.0.0.1"))) {
			Info_SetValueForKey(userinfo, "rejmsg", "Someone is already connected from your IP.");
			return false;
		}
	}

	// check for a spectator (r1: this is dangerous!! shouldn't really userinfo in client_connect)
	// also broke my ban code... note, this will be destroyed by next client userinfo update anyway
	//Info_SetValueForKey (userinfo, "spectator", "1");
	value = Info_ValueForKey (userinfo, "spectator");
	if (*value && strcmp(value, "0")) {
		int i, numspec;

		if (*spectator_password->string && 
			strcmp(spectator_password->string, "none") && 
			strcmp(spectator_password->string, value)) {
			Info_SetValueForKey(userinfo, "rejmsg", "Spectator password required or incorrect.");
			return false;
		}

		// count spectators
		for (i = numspec = 0; i < maxclients->value; i++)
			if (g_edicts[i+1].inuse && g_edicts[i+1].client->pers.spectator)
				numspec++;

		if (numspec >= maxspectators->value) {
			Info_SetValueForKey(userinfo, "rejmsg", "Server spectator limit is full.");
			return false;
		}
	} else {
		// check for a password
		value = Info_ValueForKey (userinfo, "password");
		if (*password->string && strcmp(password->string, "none") && 
			strcmp(password->string, value)) {
			Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
			return false;
		}
	}

	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
	}

	// take copy of the \ip.
	strncpy(ent->client->pers.ip,ipa,sizeof(ent->client->pers.ip));

	//r1: now 
	Info_SetValueForKey (userinfo, "spectator", "1");
	ClientUserinfoChanged (ent, userinfo);

	gi.bprintf(PRINT_HIGH, "%s connected\n", ent->client->pers.netname);
	sprintf(msg, "%s (%s) connected\n", ent->client->pers.netname, ent->client->pers.ip);
	logmsg(msg);
//	if (game.maxclients > 1)
//		gi.dprintf(msg);

	ent->svflags = 0; // make sure we start with known default
	ent->client->pers.connected = true;

	//strcpy(ent->client->pers.ip, ip);

	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int		playernum;
	char msg[64];

	if (!ent->client)
		return;

	saveCharacter(ent);
	sprintf(msg, "%s disconnected\n", ent->client->pers.netname);
	logmsg(msg);
	gi.bprintf (PRINT_HIGH, msg);

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classid = CI_P_DISCONNECTED;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;
	ent->svflags |= SVF_NOCLIENT;

	memset(&ent->client->pers.skills, 0, sizeof(ent->client->pers.skills));
	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");

//Check votes, if vote is running to kick or mute this player, abort vote.
	if ((level.vote_time >= level.time) && ((level.vote_type == VOTE_KICK) || (level.vote_type == VOTE_MUTE))) {
		if ((g_edicts + 1 + level.vote_target[0]) == ent) {
			gi.bprintf(PRINT_HIGH, "Target for vote left server, aborting vote\n");
			level.vote_type = 0;
		}
	}
}


//==============================================================


edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}
void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j;
	pmove_t	pm;

	level.current_entity = ent;
	client = ent->client;

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5.0
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}

	//gi.dprintf("%f, %f\n", level.time, ent->client->itemchangetime);

	if ((ent->client->pers.itemchanging < 0) && !ent->client->pers.spectator && !ent->client->resp.spectator) {
		for (i = 0; i < GIEX_BASEITEMS; i++) {
			//gi.dprintf("%d, %d\n", ent->client->pers.skills.wornItem[i], ent->client->pers.skills.activatingItem[i]);

			if ((ent->client->pers.skills.activatingItem[i] > -1) && (ent->client->pers.skills.wornItem[i] != ent->client->pers.skills.activatingItem[i])) {
				float bonus = 0;
				if (i == GIEX_ITEM_WEAPON) {
					iteminfo_t *iteminfo = getItemInfo(ent->client->pers.skills.itemId[(int) ent->client->pers.skills.activatingItem[GIEX_ITEM_WEAPON]]);
					bonus = ent->client->pers.skills.classLevel[0] * 0.08 + ent->client->pers.skills.classLevel[1] * 0.04 + ent->client->pers.skills.classLevel[2] * 0.05 + ent->client->pers.skills.classLevel[3] * 0.02;
					ent->client->newweapon = iteminfo->item;
					//gi.dprintf("new: %s\n", iteminfo->name);
				}
				if (bonus > 2.9)
					bonus = 2.9;
				ent->client->pers.itemchangetime = level.time + (3 - bonus);
				//gi.dprintf("%f, %f\n", level.time, ent->client->pers.itemchangetime);
				ent->client->pers.itemchanging = i;
				break;
			}
		}
	}
//To prevent item-changing hanging in middle of level change, or when dead.
	if ((ent->client->pers.itemchanging > -1) && ((level.time + 5 < ent->client->pers.itemchangetime) || (ent->health < 1))) {
		ent->client->pers.itemchanging = -1;
	}

	pm_passent = ent;

	if (ent->client->chase_target) {

		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	} else {

		// set up for pmove
		memset (&pm, 0, sizeof(pm));

		if (ent->movetype == MOVETYPE_NOCLIP)
			client->ps.pmove.pm_type = PM_SPECTATOR;
		else if (ent->s.modelindex != 255)
			client->ps.pmove.pm_type = PM_GIB;
		else if (ent->deadflag)
			client->ps.pmove.pm_type = PM_DEAD;
		else
			client->ps.pmove.pm_type = PM_NORMAL;

		client->ps.pmove.gravity = sv_gravity->value;
		pm.s = client->ps.pmove;

		for (i=0 ; i<3 ; i++)
		{
			pm.s.origin[i] = ent->s.origin[i]*8;
//			ent->delay = 0.5;
//			if ((ent->delay > 0) && (i != 2))
//				pm.s.velocity[i] = ent->velocity[i]*8*ent->delay;
//			else
				pm.s.velocity[i] = ent->velocity[i]*8;
		}

		if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
		{
			pm.snapinitial = true;
	//		gi.dprintf ("pmove changed!\n");
		}

		pm.cmd = *ucmd;

		pm.trace = PM_trace;	// adds default parms
		pm.pointcontents = gi.pointcontents;

		// perform a pmove
		gi.Pmove (&pm);

		// save results of pmove
		client->ps.pmove = pm.s;
		client->old_pmove = pm.s;

		for (i=0 ; i<3 ; i++)
		{
			ent->s.origin[i] = pm.s.origin[i]*0.125;
			ent->velocity[i] = pm.s.velocity[i]*0.125;
		}

		VectorCopy (pm.mins, ent->mins);
		VectorCopy (pm.maxs, ent->maxs);

		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

		if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
		{
			if ((ent->client->silencer_shots > 0) && (ent->client->pers.skill[58] > 5)) {
				ent->client->silencer_shots -= 10;
			} else {
				gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
				PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
			}
		}

		ent->viewheight = pm.viewheight;
		ent->waterlevel = pm.waterlevel;
		ent->watertype = pm.watertype;
		ent->groundentity = pm.groundentity;
		if (pm.groundentity)
			ent->groundentity_linkcount = pm.groundentity->linkcount;

		if (ent->deadflag)
		{
			client->ps.viewangles[ROLL] = 40;
			client->ps.viewangles[PITCH] = -15;
			client->ps.viewangles[YAW] = client->killer_yaw;
		}
		else
		{
			VectorCopy (pm.viewangles, client->v_angle);
			VectorCopy (pm.viewangles, client->ps.viewangles);
		}

		gi.linkentity (ent);

		if (ent->movetype != MOVETYPE_NOCLIP)
			G_TouchTriggers (ent);

		// touch other objects
		for (i=0 ; i<pm.numtouch ; i++)
		{
			other = pm.touchents[i];
			for (j=0 ; j<i ; j++)
				if (pm.touchents[j] == other)
					break;
			if (j != i)
				continue;	// duplicated
			if (!other->touch)
				continue;
			other->touch (other, ent, NULL, NULL);
		}

	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTON_ATTACK)
	{
		if (client->resp.spectator) {

			client->latched_buttons = 0;

//			if (!ent->client->showscores) {
				client->chase_target = NULL;
				client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				closeGiexMenu(ent);
				ent->client->showscores = true;
				ent->client->showmenu = 5;
				showGiexMainMenu(ent);
/*			} else {
				if (client->chase_target) {
					client->chase_target = NULL;
					client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
				} else {
					GetChaseTarget(ent);
				}
			}*/

		} else if (!client->weapon_thunk) {
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

/*	if (client->resp.spectator) {
		if (ucmd->upmove >= 10) {
			if (!(client->ps.pmove.pm_flags & PMF_JUMP_HELD)) {
				client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
				if (client->chase_target)
					ChaseNext(ent);
				else
					GetChaseTarget(ent);
			}
		} else
			client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
	}*/

	// update chase cam if being followed
	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}

//	gi.dprintf("%f, %f: type: %d, item: %d\n", ent->client->itemchangetime, level.time, ent->client->itemchanging, ent->client->pers.skills.activatingItem[ent->client->itemchanging]);
	if ((ent->client->pers.itemchanging > -1) && (ent->client->pers.itemchangetime <= level.time)) {
		iteminfo_t *info = getItemInfo(ent->client->pers.skills.itemId[(int) ent->client->pers.skills.activatingItem[ent->client->pers.itemchanging]]);
//		gi.dprintf("%f %f %d %d %d\n", level.time, ent->client->itemchangetime, ent->client->itemchanging, ent->client->pers.skills.wornItem[ent->client->itemchanging], ent->client->pers.skills.activatingItem[ent->client->itemchanging]);
		ent->client->pers.skills.wornItem[ent->client->pers.itemchanging] = ent->client->pers.skills.activatingItem[ent->client->pers.itemchanging];
		ent->client->pers.skills.activatingItem[ent->client->pers.itemchanging] = -1;
		ent->client->pers.itemchanging = -1;
		updateItemLevels(ent);
		if (info->name[0] != '\0') {
			gi.cprintf(ent, PRINT_HIGH, "%s equipped\n", info->name);
		}
	}

	if (ent->client->pers.entering_pwd && (ent->client->pers.entered_pwd[0] != '\0')) {
		if (ent->client->menuchoice[0] == 1) { // Create new character
			ent->client->pers.entering_pwd = false;
			newCharacter(ent, ent->client->menuchoice[1], ent->client->pers.entered_pwd);
			//gi.dprintf("%d: %s\n", ent->client->menuchoice[1], ent->client->pers.entered_pwd);
			closeGiexMenu(ent);

			ent->client->pers.spectator = false;
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 0\n");
			gi.unicast(ent, true);

			assignToTeam(ent);
//			gi.cprintf(ent, PRINT_HIGH, "Character created successfully, joining game..\n");
			ent->client->pers.entered_pwd[0] = '\0';
		} else if (ent->client->menuchoice[0] == 0) { // Load existing character
			int i;
			edict_t	*scan;

			closeGiexMenu(ent);
			ent->client->pers.entering_pwd = false;
			for (i=0; i<maxclients->value ; i++) {
				scan = g_edicts + 1 + i;
				if (!scan->inuse)
					continue;
				if (!scan->client)
					continue;
				if (strncmp(ent->client->pers.netname, scan->client->pers.skills.password + 16, 16) == 0) {
					gi.cprintf(ent, PRINT_HIGH, "This character is already logged in. Keep your password safe and don't try to cheat.\n");
					ent->client->pers.entered_pwd[0] = '\0';
					return;
				}
			}

			if (!loadCharacter(ent, ent->client->pers.entered_pwd)) {
				ent->client->pers.spectator = true;
				gi.WriteByte (svc_stufftext);
				gi.WriteString ("spectator 1\n");
				gi.unicast(ent, true);
				//r1: inserted missing return here
				ent->client->pers.entered_pwd[0] = '\0';
				return;
			}
		// Go out of spectator mode
			ent->client->pers.spectator = false;
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 0\n");
			gi.unicast(ent, true);

		// Sum up upgrades
			ent->radius_dmg = 0;
			ent->count = 0;
			for (i = 0; i < GIEX_NUMCLASSES; i++)
				ent->radius_dmg += ent->client->pers.skills.classLevel[i];
			ent->client->pers.expRemain = getClassExpLeft(ent, ent->client->pers.skills.activeClass);
			ent->client->pers.itemchanging = -1; // set client ready to equip items

			assignToTeam(ent);
//			gi.cprintf(ent, PRINT_HIGH, "Character loaded successfully, joining game..\n");
			ent->client->pers.entered_pwd[0] = '\0';
		}
	}
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void giveWeapons (edict_t *ent)
{
	//gitem_t		*item;

	// Give player one of each weapon
	/*item = FindItem("Blaster");
	ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Shotgun");
	ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Super Shotgun");
	ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Machinegun");
	ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Chaingun");
	ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Grenade launcher");
	ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Rocket launcher");
	ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Hyperblaster");
	ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("Railgun");
	ent->client->pers.inventory[ITEM_INDEX(item)] = 1;
	item = FindItem("BFG10K");
	ent->client->pers.inventory[ITEM_INDEX(item)] = 1;*/
}

void kick(edict_t *player)
{
	gi.AddCommandString (va("kick %d\n", (player - g_edicts)-1 ));
}

void castPizza(edict_t *self, qboolean first);
void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			buttonMask, lvl, index, old, max;
	int aclvl = ent->client->pers.skill[25];
	float bonus = 0;
	float time = level.time;

	if (level.intermissiontime)
		return;

	if (aclvl > 40) // To cap ammo regen penalty
		aclvl = 40;
	client = ent->client;

	// check idle timer
	if (client->pers.idletime++ >= 600 * idletimeout->value) {
		if (!(!dedicated->value && ent->s.number == 1)) {
			gi.bprintf (PRINT_HIGH, "Kicking %s for prolonged inactivity.\n", client->pers.netname);
			gi.cprintf (ent, PRINT_HIGH, "You have been kicked due to inactivity.\n");
			kick (ent);
			return;
		}
		client->pers.idletime = 0;
	} else if (client->latched_buttons & BUTTON_ANY) // latched prevents players putting heavy objects to keyb
		client->pers.idletime = 0;

	if (client->pers.spectator != client->resp.spectator &&
		(level.time - client->respawn_time) >= 5) {
		spectator_respawn(ent);
		return;
	}

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk && !client->resp.spectator)
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;

	if (ent->deadflag)
	{
		// wait for any button just going down
		if ( level.time > client->respawn_time)
		{
			// in deathmatch, only wait for attack button
			if (deathmatch->value)
				buttonMask = BUTTON_ATTACK;
			else
				buttonMask = -1;

			if ( ( client->latched_buttons & buttonMask ) /*||
				(deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) )*/ )
			{
				respawn(ent);
				client->latched_buttons = 0;
			}
		}
		return;
	}

	// add player trail so monsters can follow
	if (!deathmatch->value)
		if (!visible (ent, PlayerTrail_LastSpot() ) )
			PlayerTrail_Add (ent->s.old_origin);

	{
		powerupinfo_t *puinfo = getPowerupInfo(ent->client->pers.selspell);
		if ((puinfo->isspell & 64) == 64) {
			if (((int)ceil(level.time * 10 - 0.5) % 4) == 0) {
				castAura(ent);
			}
		} else if (client->castspell) {
			castSpell(ent);
		}
		checkAuras(ent);
	}

	//ApplyMax(ent);

	if ((((int)ceil(level.time * 10 - 0.5) % 5) == 0) && (ent->health > 0)) {
		// regen aura
		if ( ((lvl = getAuraLevel(ent, 66)) > 0) && (ent->health < ent->max_health)) {
			int slot = getAuraSlot(ent, 66);
			if (ent->client->aura_caster[slot]->client->magic > 0) {
				bonus = getMagicBonuses(ent->client->aura_caster[slot], 66);
				ent->health += ceil(1.7 * ent->client->aura_level[slot]) * bonus;
				addExp(ent->client->aura_caster[slot], ent, (int) -(ceil(1.7 * ent->client->aura_level[slot]) * bonus));
				if (ent->health > ent->max_health)
					ent->health = ent->max_health;
				ent->client->aura_caster[slot]->client->magic -= (int) ceil(1.5 * lvl);
				if (lvl > 30) {
					ent->client->aura_caster[slot]->client->magic -= (int) ceil(1.5 * (lvl - 30));
				}
				ent->client->aura_caster[slot]->client->magregentime = level.time + 1.0;
				gi.sound(ent, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 0.5, ATTN_NORM, 0);
			}
		}
	}
	if ((((int)ceil(level.time * 10 - 0.5) % 10) == 0) && (ent->health > 0)) {
		// Health regen
		if (( (lvl = ent->client->pers.skill[30]) > 0) && (ent->health < ent->max_health)) {
			ent->health += (int)(2.6 * lvl);
			if (ent->health > ent->max_health)
				ent->health = ent->max_health;
		}

		// Armor regen
		if (( ((lvl = ent->client->pers.skill[31]) > 0) || (ent->client->pers.skills.classLevel[1] > 0)) && (ent->client->pers.inventory[ITEM_INDEX(FindItem("Body Armor"))] < ent->client->max_armor)) {
			iteminfo_t *info = getWornItemInfo(ent, GIEX_ITEM_ARMOR);
			int ind = ITEM_INDEX(FindItem("Body Armor"));
			ent->client->pers.inventory[ind] += (int)(info->arg5 + info->arg6 * lvl) + ceil(ent->client->pers.skills.classLevel[1] * 0.3);
			if (ent->client->pers.inventory[ind] > ent->client->max_armor)
				ent->client->pers.inventory[ind] = ent->client->max_armor;
		}
	}
	if (((int)ceil(level.time * 10 - 0.5) % 100) == 0) {
		if ( ((lvl = ent->client->pers.skill[32]) > 0) && (ent->health > 0) ) {
// Ammo regen from powerup
			gitem_t *item;
			int index;
			float mult = (0.07 * (float) lvl) * (1.0-(0.024 * (float) aclvl));
			if (mult < 0.05)
				mult = 0.05;

			item = FindItem("Bullets");
			if (item) {
				index = ITEM_INDEX(item);
				ent->client->pers.inventory[index] += (int) ceil((float) 60 * mult);
			}

			item = FindItem("Shells");
			if (item) {
				index = ITEM_INDEX(item);
				ent->client->pers.inventory[index] += (int) ceil((float) 12 * mult);
			}

			item = FindItem("Cells");
			if (item) {
				index = ITEM_INDEX(item);
				ent->client->pers.inventory[index] += (int) ceil((float) 30 * mult);
			}

			item = FindItem("Grenades");
			if (item) {
				index = ITEM_INDEX(item);
				ent->client->pers.inventory[index] += (int) ceil((float) 4 * mult);
			}

			item = FindItem("Rockets");
			if (item) {
				index = ITEM_INDEX(item);
				ent->client->pers.inventory[index] += (int) ceil((float) 4 * mult);
			}

			item = FindItem("Slugs");
			if (item) {
				index = ITEM_INDEX(item);
				ent->client->pers.inventory[index] += (int) ceil((float) 4 * mult);
			}

			item = FindItem("Blaster cells");
			if (item) {
				index = ITEM_INDEX(item);
				ent->client->pers.inventory[index] += (int) ceil((float) 20 * mult);
			}
			checkAmmoAmounts(ent);
		}

		if (ent->client->pers.weapon && ((ent->client->pers.weapon)->ammo != NULL) && (ent->health > 0) && !((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)) {
			iteminfo_t *iteminfo = getWornItemInfo(ent, GIEX_ITEM_WEAPON);
			gitem_t *type = FindItem((ent->client->pers.weapon)->ammo);
			int q;
			float mult = 0;
			if (iteminfo->type & GIEX_ITEM_SOLDIER) {
				mult += 0.025 * ent->client->pers.skills.classLevel[0];
			}
			if (iteminfo->type & GIEX_ITEM_TANK) {
				mult += 0.03 * ent->client->pers.skills.classLevel[1];
			}
			if (iteminfo->type & GIEX_ITEM_CLERIC) {
				mult += 0.02 * ent->client->pers.skills.classLevel[2];
			}
			if (iteminfo->type & GIEX_ITEM_MAGE) {
				mult += 0.05 * ent->client->pers.skills.classLevel[3];
			}
			mult *= (1.0 - (0.024 * (float) aclvl));
			switch (type->quantity) {
				case 5: //rockets & grenades
					q = 4;
					break;
				case 10: //slugs
					q = 4;
					break;
				case 12: //shells
					q = 6;
					break;
				case 50: //cells
					q = 30;
					break;
				case 60: //bullets
					q = 40;
					break;
				default:
					//FIXME: insert suitable default
					q = 25;
					break;
			}
			if (mult < 0.05)
				mult = 0.05;

			ent->client->pers.inventory[ITEM_INDEX(type)] += (int) ceil((float) q * mult);
			checkAmmoAmounts(ent);
		}
	}

// Magic regen
	time = level.time - (1.5 - 0.02 * ent->client->pers.skill[33] - 0.01 * ent->client->pers.skills.classLevel[3] - 0.01 * ent->client->pers.skills.classLevel[4]);
	if (time > level.time)
		time = level.time;
	if ((ent->client->magregentime < time) && (ent->health > 0)) {
		if (ent->client->magic < ent->client->max_magic) {
			ent->client->magic += 1 + 3.0 * ent->client->pers.skill[33] + 0.2 * pow(ent->client->pers.skills.classLevel[2], 1.08) + 0.25 * pow(ent->client->pers.skills.classLevel[3], 1.08) + 0.2 * pow(ent->client->pers.skills.classLevel[4], 1.08);
			if (ent->client->magic > ent->client->max_magic)
				ent->client->magic = ent->client->max_magic;
		}
// Ammo converter + Ammo regen = magic bonus
/*		if ((ent->client->pers.skill[25] > 0) && ((ent->client->magic < ent->client->max_magic * 2))) {
			ent->client->magic += (int) ceil(ent->client->pers.skill[25] * ent->client->pers.skill[32] * 0.015);
			if (ent->client->magic > ent->client->max_magic * 2)
				ent->client->magic = ent->client->max_magic * 2;
		}*/
	}

	index = ITEM_INDEX(FindItem("Blaster cells"));
	if ((old = ent->client->pers.inventory[index]) < (max = 100 + 12 * ent->client->pers.skill[28]) && !((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)) {
		int lvl = ent->client->pers.skill[32];
		int am = 0;
		am = (int) ceil((float) (5.0 + 0.5 * lvl) * (float)(1.0-(0.024 * aclvl)));
//		else
//			am = (int) ceil((float) (2.5 + 0.05 * lvl) * (float)(1.0-(0.031 * aclvl)));
		ent->client->pers.inventory[index] += am;
		if ((old < max) && (ent->client->pers.inventory[index] >= max) && !((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK)) {
			gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/comp_up.wav"), 1, ATTN_NORM, 0);
			ent->client->pers.inventory[index] = max;
		}
		if (ent->client->pers.inventory[index] < 0) {
			ent->client->pers.inventory[index] = 0;
		}
	}

	if ((ent->health > ent->max_health * 1.75) && (!((int)(level.time * 10) % 10)))
		ent->health -= 2;
	else if ((ent->health > ent->max_health * 1.5) && (!((int)(level.time * 10) % 10)))
		ent->health--;
	else if ((ent->health > ent->max_health * 1.25) && (!((int)(level.time * 10) % 25)))
		ent->health--;
	else if ((ent->health > ent->max_health) && (!((int)(level.time * 10) % 50)))
		ent->health--;

	client->latched_buttons = 0;
/*	if (ent->client->invincible_framenum == (level.framenum + 29)) {
		giveWeapons(ent);
	}*/
	if (ent->client->barf_time > level.time) {
		ent->client->barf_frames--;
		if (ent->client->barf_frames < 1) {
			castPizza(ent, false);
			ent->client->barf_frames = random() * 10 + 20;
		}
	}
/*
	if (client->resp.spectator && (level.framenum < client->resp.enterframe + 80) && (level.framenum > client->resp.enterframe + 5) && ((level.framenum % 30) == 0)) {
		gi.centerprintf(ent, "===================================================\nWelcome to Giex!\n-------------------\nNeed help?\nType 'giexhelp' in the console\nor visit http://www.planetquake.com/giex\n=========================================%s==\n", GIEX_VERSION);
	}
*/

// If we have stealthed monsters around, give player a flashlight
	if ((level.light_level < 8) && (!ent->client->pers.spectator) && (!ent->client->resp.spectator)) {
		trace_t tr;
		vec3_t	forward, offset, end;

		if (ent->client->flashlight == NULL) {
			ent->client->flashlight = G_Spawn();
			ent->client->flashlight->classid = CI_P_FLASHLIGHT;
			ent->client->flashlight->classname = "flashlight";

			ent->client->flashlight->s.effects |= EF_HYPERBLASTER;
			ent->client->flashlight->svflags = SVF_DEADMONSTER;
			ent->client->flashlight->movetype = MOVETYPE_NONE;
			ent->client->flashlight->solid = SOLID_NOT;
			ent->client->flashlight->owner = ent;
			ent->client->flashlight->s.modelindex = gi.modelindex ("sprites/s_shine.sp2");
			//ent->client->flashlight->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
			//VectorCopy(ent->s.origin, ent->client->flashlight->s.origin);
			//gi.linkentity(ent->client->flashlight);
		}

		AngleVectors(ent->client->v_angle, forward, NULL, NULL);
		VectorCopy(ent->s.origin, offset);
		offset[2] += ent->viewheight - 8;
		VectorMA(offset, 2048, forward, end);
		tr = gi.trace (offset, NULL, NULL, end, ent, MASK_SHOT);
		VectorCopy(tr.endpos, end);
		VectorNormalize(forward);
		VectorMA(end, -1.0, forward, end);
		VectorCopy(end, ent->client->flashlight->s.origin);
		gi.linkentity(ent->client->flashlight);
	} else {
		if (ent->client->flashlight != NULL) {
			G_FreeEdict(ent->client->flashlight);
			ent->client->flashlight = NULL;
		}
	}
}
