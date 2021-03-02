#include "g_local.h"
#include "m_player.h"

// SH
void MakeObserver (edict_t *);
void boot (edict_t *, edict_t *, float);
char *GetSub();
void target_laser_start (edict_t *);
void target_laser_off (edict_t *);
void MV (edict_t *);
void SpawnDamage (int, vec3_t, vec3_t, int);
void stuffcmd(edict_t *e, char *s);
extern float framerate (usercmd_t *);
extern void death_blossom (edict_t *, edict_t *);
void ClientDisconnect (edict_t *ent);
edict_t *randplayer (edict_t *ent);
// \SH

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

static void SP_FixCoopSpots (edict_t *self)
{
	edict_t	*spot;
	vec3_t	d;

	spot = NULL;

	while(1)
	{
		spot = G_Find(spot, FOFS(classname), "info_player_start");
		if (!spot)
			return;
		if (!spot->targetname)
			continue;
		VectorSubtract(self->s.origin, spot->s.origin, d);
		if (VectorLength(d) < 384)
		{
			if ((!self->targetname) || Q_stricmp(self->targetname, spot->targetname) != 0)
			{
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

static void SP_CreateCoopSpots (edict_t *self)
{
	edict_t	*spot;

	if(Q_stricmp(level.mapname, "security") == 0)
	{
		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 - 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
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
void SP_info_player_start(edict_t *self)
{
	if (!coop->value)
		return;
	if(Q_stricmp(level.mapname, "security") == 0)
	{
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
	if (!deathmatch->value)
	{
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
	if (!coop->value)
	{
		G_FreeEdict (self);
		return;
	}

	if((Q_stricmp(level.mapname, "jail2") == 0)   ||
	   (Q_stricmp(level.mapname, "jail4") == 0)   ||
	   (Q_stricmp(level.mapname, "mine1") == 0)   ||
	   (Q_stricmp(level.mapname, "mine2") == 0)   ||
	   (Q_stricmp(level.mapname, "mine3") == 0)   ||
	   (Q_stricmp(level.mapname, "mine4") == 0)   ||
	   (Q_stricmp(level.mapname, "lab") == 0)     ||
	   (Q_stricmp(level.mapname, "boss1") == 0)   ||
	   (Q_stricmp(level.mapname, "fact3") == 0)   ||
	   (Q_stricmp(level.mapname, "biggun") == 0)  ||
	   (Q_stricmp(level.mapname, "space") == 0)   ||
	   (Q_stricmp(level.mapname, "command") == 0) ||
	   (Q_stricmp(level.mapname, "power2") == 0) ||
	   (Q_stricmp(level.mapname, "strike") == 0))
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
void SP_info_player_intermission(edict_t *self)
{
}


/*
int	SexedSoundIndex (edict_t *ent, char *base)
{
	char	buffer[MAX_QPATH];

	Com_sprintf (buffer, sizeof(buffer), "%s/%s.wav", ent->client->pers.sounddir, base);

	return gi.soundindex(buffer);
}
*/
//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}

qboolean IsFemale (edict_t *ent)
{
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	return false;
}


void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff, luck;

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	if (deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		luck = mod & MOD_LUCKY;
		mod = mod & ~MOD_LUCKY;;
		message = NULL;
		message2 = "";

		switch (mod)
		{
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
		if (attacker == self)
		{
			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = "tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsFemale(self))
					message = "tripped on her own grenade";
				else
					message = "tripped on his own grenade";
				break;
			case MOD_R_SPLASH:
				if (IsFemale(self))
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				break;
// SH			
			case MOD_LIGHT:
				message = "turns to dust";
				break;
			case MOD_BERSERK:
				message = "is consumed by his rage";
				break;
			case MOD_ELEC:
				message = "electrocutes himself";
				break;
// \SH
			default:
				if (IsFemale(self))
					message = "killed herself";
				else
					message = "killed himself";
				break;
			}
		}
		if (message)
		{
			gi.bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
				self->client->resp.score--;
			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			int type = 0;
// SH
			if (attacker->client && attacker->client->pers.combo == C_JEDI)
			{
				if (mod == MOD_KINTHROW && attacker->wait < 2)
					attacker->wait += 1;
				else if (attacker->wait < 2)
					attacker->wait = 0;

				if (attacker->wait == 2)
				{
					attacker->wait = 3;
					gi.sound(attacker, CHAN_AUTO, gi.soundindex("powers/jediup.wav"), 1, ATTN_NONE, 0);
				}
			}			
// \SH

			switch (mod)
			{
			case MOD_BLASTER:
				message = "was blasted by";
				break;
			case MOD_SHOTGUN:
				message = "was gunned down by";
				break;
			case MOD_SSHOTGUN:
				message = "was blown away by";
				message2 = "'s super shotgun";
				break;
			case MOD_MACHINEGUN:
				message = "was machinegunned by";
				break;
			case MOD_CHAINGUN:
				message = "was cut in half by";
				message2 = "'s chaingun";
				break;
			case MOD_GRENADE:
				message = "was popped by";
				message2 = "'s grenade";
				break;
			case MOD_G_SPLASH:
				message = "was shredded by";
				message2 = "'s shrapnel";
				break;
			case MOD_ROCKET:
				message = "ate";
				message2 = "'s rocket";
				break;
			case MOD_R_SPLASH:
				message = "almost dodged";
				message2 = "'s rocket";
				break;
			case MOD_HYPERBLASTER:
				message = "was melted by";
				message2 = "'s hyperblaster";
				break;
			case MOD_RAILGUN:
				message = "was railed by";
				break;
			case MOD_BFG_LASER:
				message = "saw the pretty lights from";
				message2 = "'s BFG";
				break;
			case MOD_BFG_BLAST:
				message = "was disintegrated by";
				message2 = "'s BFG blast";
				break;
			case MOD_BFG_EFFECT:
				message = "couldn't hide from";
				message2 = "'s BFG";
				break;
			case MOD_HANDGRENADE:
				message = "caught";
				message2 = "'s handgrenade";
				break;
			case MOD_HG_SPLASH:
				message = "didn't see";
				message2 = "'s handgrenade";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
// SH			
			case MOD_PSIBLAST:
				message = "melted";
				message2 = "'s brain";
				type = 1;
				break;
			case MOD_BOPGUN:
				message = "got bopped with";
				message2 = "'s bop gun";
				break;
			case MOD_FLAMEBALL:
				message = "was flamed by";
				message2 = "'s ball";
				break;
			case MOD_KINTHROW:
				message = "is thrown around by";
				break;
			case MOD_KINCARD:
				message = "deals";
				message2 = " a bad hand";
				type = 1;
				break;
			case MOD_GRAPPLE:
				message = "is eviscerated by";
				break;
			case MOD_CASCADE:
				message = "gets torched by";
				break;
			case MOD_WEIRD:
				message = "was freaked by";
				break;
			case MOD_WAIL:
				message = "gets spooked by";
				break;
			case MOD_OPTIC:
				message = "destroys";
				message2 = " with his eye-beams";
				type = 1;
				break;
			case MOD_ROBOT:
				message = "is assimilated by";
				break;
			case MOD_LASER:
				message = "gets lazed by";
				break;
			case MOD_RADIO:
				message = "was irradiated by";
				break;
			case MOD_ELEC:
				message = "is electrocuted by";
				break;
			case MOD_SABER:
				message = "instructs";
				message2 = " in the ways of the Force";
				type = 1;
				break;
			case MOD_PROX:
				message = "was bombed by";
				break;
			case MOD_UNKNOWN:
				message = "was mysteriously killed by";
				break;
			case MOD_BOOT:
				message = "boots";
				message2 = " in the head";
				type = 1;
				break;
			case MOD_CRIP:
				message = "gets run over by";
				break;
			case MOD_SOLAR:
				message = "looked directly at";
				break;
			case MOD_HAPPY:
				message = "plays with";
				message2 = "'s Happy Fun Balls";
				break;
// \SH
			}
			if (self->client->pers.combo == C_CRIP)
			{
				stuffcmd (attacker, "say That's one less cripple.\n");
			}
			if (luck)
			{
				gi.bprintf (PRINT_MEDIUM, "%s sure got lucky.\n", attacker->client->pers.netname);
			}
			if (message)
			{
				if (type == 0)
					gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				else
					gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", attacker->client->pers.netname, message, self->client->pers.netname, message2);

				if (deathmatch->value)
				{
					if (ff)
						attacker->client->resp.score--;
					else
					{
						attacker->client->resp.score++;
// SH
						if (attacker->client->pers.passive == MORTAL)
						{
							attacker->client->resp.score += 2;
						}
						else if (attacker->client && self->client 
							&& attacker->asstarg == self 
							&& attacker->client->pers.combo == C_ASS)
						{
							int b = rand()%3 + 2;
							attacker->client->resp.score += b;
							gi.centerprintf (attacker, "%s\nBounty: %i\n",Green1("Target terminated"), b);
						}
						else if (attacker->client && self->client
							&& self->asstarg == attacker)
						{
							int b = rand()%2 + 1;
							attacker->client->resp.score += b;
							gi.centerprintf (attacker, "%s\nReward: %i\n",Green1("Assassin terminated"), b);
						}
// \SH

					}
				}
				return;
			}
		}
	}

	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (!deathmatch->value)
		self->client->resp.score--;
	else
	{
		if ((attacker == inflictor || inflictor->owner == attacker) && attacker->client)
		{
			attacker->client->resp.score++;
		}
	}
}
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	qboolean	quad;
	float		spread;
// SH
	if (self->flags & FL_BERSERK)
	{
		return;
	}
	if (self->client->pers.combo == C_MAGE || self->client->pers.combo == C_ROBOT || self->client->pers.combo == C_CRIP)
	{
		return;
	}
// \SH
	if (!deathmatch->value)
		return;

	item = self->client->pers.weapon;
	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Blaster") == 0))
		item = NULL;

	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else
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
		drop = Drop_Item (self, FindItemByClassname ("item_quad"));
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

	self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
}

/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;
// SH
	if (self->client->pers.special == BLOSSOM && attacker != self && self->health > -40)
	{
		death_blossom(self, attacker);
	}

	CTFDrop_Flag (self);

	if (self->goalentity != NULL)
	{
		G_FreeEdict (self->goalentity);
		self->goalentity = NULL;
	}

	if (self->s.modelindex == invis_index || self->s.modelindex == sun_index)
		self->s.modelindex = 255;
// \SH

	if (self->teamchain != NULL)
	{
		G_FreeEdict (self->teamchain);
		self->teamchain = NULL;
	}

	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->maxs[2] = -8;

//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);

		if (attacker != self) // SH
			TossClientWeapon (self);
		
		if (deathmatch->value)
			Cmd_Help_f (self);		// show scores
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;

	// clear inventory
	memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));

	if (self->health < -40)
	{	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead (self, damage);

		self->takedamage = DAMAGE_NO;
	}
	else
	{	// normal death
		if (!self->deadflag)
		{
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
// SH	
	int			active, passive, special, combo, admin, lucky;

	active = client->pers.active;
	passive = client->pers.passive;
	special = client->pers.special;
	combo = client->pers.combo;
	admin = client->pers.admin;
	lucky = client->pers.lucky;
// \SH
	memset (&client->pers, 0, sizeof(client->pers));

	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

// SH  make shotgun default weapon
	item = FindItem("Shotgun");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;
	client->pers.inventory[ITEM_INDEX(FindItem("shells"))] = 25;
// \SH

	client->pers.weapon = item;

	client->pers.health			= 100;
// SH
	if (passive == LIFE)
		client->pers.max_health		= 400;
	else if (passive == ROBOT)
		client->pers.max_health		= 250;
	else if (passive == CRIP)
		client->pers.max_health		= 40;
	else
// \SH
		client->pers.max_health		= 100;

	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 50;
// SH
	client->pers.active = active;
	client->pers.passive = passive;
	client->pers.special = special;
	client->pers.combo = combo;
	client->pers.admin = admin;
	client->pers.lucky = lucky;
// \SH
	client->pers.connected = true;
}


void InitClientResp (gclient_t *client)
{
	memset (&client->resp, 0, sizeof(client->resp));
	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;
	client->resp.initialized = 1;
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
		game.clients[i].pers.powerArmorActive = (ent->flags & FL_POWER_ARMOR);
		if (coop->value)
			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	if (ent->client->pers.powerArmorActive)
		ent->flags |= FL_POWER_ARMOR;
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
// SH
		if (!(player->client || player->svflags & SVF_MONSTER))
			continue;
// \SH
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
edict_t *SelectRandomDeathmatchSpawnPoint (void)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
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
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
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
edict_t *SelectFarthestDeathmatchSpawnPoint (void)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
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
	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}

edict_t *SelectDeathmatchSpawnPoint (void)
{
	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)//|| deathmatch->value == 2) // SH
		return SelectFarthestDeathmatchSpawnPoint ();
	else
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
		spot = G_Find (spot, FOFS(classname), "info_player_coop");
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}


	return spot;
}


/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;

	if (deathmatch->value)
	{
// SH CTF
		if (ctf->value && ent->client && ent->client->resp.score == 0)
			spot = SelectCTFSpawnPoint(ent);
		else
// SH CTF
		spot = SelectDeathmatchSpawnPoint ();
	}
	else if (coop->value)
		spot = SelectCoopSpawnPoint (ent);

	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (!spot)
		{
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find (spot, FOFS(classname), "info_player_start");
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
		ent->classname = "bodyque";
	}
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	n;

	if (self->health < -40)
	{
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

	body->die = body_die;
	body->takedamage = DAMAGE_YES;

	gi.linkentity (body);
}


void respawn (edict_t *self)
{
	if (deathmatch->value || coop->value)
	{
		CopyToBodyQue (self);
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

//==============================================================


/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void PutClientInServer (edict_t *ent)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	int		c=0, z=0;
	client_persistant_t	saved;
	client_respawn_t	resp;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	index = ent-g_edicts-1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);
	}
	else if (coop->value)
	{
		int			n;
		char		userinfo[MAX_INFO_STRING];
// SH	
		int			active, passive, special, combo, admin;

		active = client->pers.active;
		passive = client->pers.passive;
		special = client->pers.special;
		combo = client->pers.combo;
		admin = client->pers.admin;
// \SH
		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < MAX_ITEMS; n++)
		{
			if (itemlist[n].flags & IT_KEY)
				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
		}
		client->pers = resp.coop_respawn;
		ClientUserinfoChanged (ent, userinfo);
		if (resp.score > client->pers.score)
			client->pers.score = resp.score;
// SH
		if (passive == LIFE)
			client->pers.max_health		= 400;
		else if (passive == ROBOT)
			client->pers.max_health		= 250;
		else if (passive == CRIP)
			client->pers.max_health		= 40;
		else
			client->pers.max_health		= 100;
	
		client->pers.active = active;
		client->pers.passive = passive;
		client->pers.special = special;
		client->pers.combo = combo;
		client->pers.admin = admin;
// \SH

	}
	else
	{
		memset (&resp, 0, sizeof(resp));
	}

	// clear everything but the persistant data
	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
	client->resp = resp;

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	// clear entity values
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
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
	ent->svflags &= ~SVF_DEADMONSTER;
//  SH
	ent->wait = 0;
	ent->flags &= ~(FL_BLUE|FL_RED|FL_GREEN|FL_BERSERK|FL_DEATHBLOW|FL_OPTIC|FL_HIT);
	ent->ants = 0;
	ent->wait = 0;
	ent-> freezetime = 0;
	ent->bleed = 0;
	ent->blind_time = 0;
	ent->flash_time = 0;
	ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

	if (ent->flags & FL_RANDOM_ACT)
	{
		z = (rand()%(MAX_ACTIVE-2))+1;
		while (game.act_ban[z] == 1)
		{
			z = (rand()%(MAX_ACTIVE-2))+1;
			c++;
			if (c > 50)
			{
				z = 0;
				break;
			}
		}
		ent->client->pers.active = z;
		z = c = 0;
	}
	if (ent->flags & FL_RANDOM_PAS)
	{
		z = (rand()%(MAX_PASSIVE-2))+1;
		while (game.pas_ban[z] == 1)
		{
			z = (rand()%(MAX_PASSIVE-2))+1;
			c++;
			if (c > 50)
			{
				z = 0;
				break;
			}
		}
		ent->client->pers.passive = z;
		z = c = 0;
	}
	if (ent->flags & FL_RANDOM_SPE)
	{
		z = (rand()%(MAX_SPECIAL-2))+1;
		while (game.spe_ban[z] == 1)
		{
			z = (rand()%(MAX_SPECIAL-2))+1;
			c++;
			if (c > 50)
			{
				z = 0;
				break;
			}
		}
		ent->client->pers.special = z;
		z = c = 0;
		if (ent->client->pers.special < AODEATH || ent->client->pers.special > AOMERCY)
		{
			ent->angel = NULL;
		}
	}

	if (!(ent->flags & FL_OBSERVER) && (!ent->client->pers.active || !ent->client->pers.passive || !ent->client->pers.special))
	{
//gi.cprintf(ent, PRINT_HIGH, "making observer...\n");
		MakeObserver(ent);
		return;
	}

	if (ent->flags & FL_OBSERVER)
	{
//gi.cprintf(ent, PRINT_HIGH, "observer\n");
		ent->takedamage = DAMAGE_NO;
		ent->solid = SOLID_TRIGGER;
		ent->movetype = MOVETYPE_FLY;
		ent->mass = 0;
		ent->model = "";
	}
// \SH
// SH
	if (ent->client->pers.passive == LIFE)
		ent->max_health		= 400;
	else if (ent->client->pers.passive == ROBOT)
		ent->max_health		= 250;
	else if (ent->client->pers.passive == CRIP)
		ent->max_health		= 40;
	else
// \SH
		ent->max_health		= 100;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}

//	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

// SH
	if (!(ent->flags & FL_OBSERVER))
// \SH
	{
		ent->s.modelindex = 255;		// will use the skin specified model
		ent->s.modelindex2 = 255;		// custom gun model
		ent->client->pers.active_finished = 0;
		ent->client->pers.passive_finished = 0;
		ent->client->pers.special_finished = 0;
		client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);
	}
// SH
	else
	{
		ent->s.modelindex = 0;		// null?
		ent->s.modelindex2 = 0;		// null?
		ent->s.modelindex3 = 0;
		ent->client->ps.gunindex = 0;
	}
// \SH

	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
//	ent->s.modelindex = 255;		// will use the skin specified model
//	ent->s.modelindex2 = 255;		// custom gun model
	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	if (!KillBox (ent))
	{	// could't spawn in?
	}

// force the current weapon up
// SH
	if (ent->flags & FL_OBSERVER)
	{
		;
	}
	else if (ent->client->pers.passive == ROBOT)
	{
//		ent->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))] = 1;
//		ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] = 999;
//		ent->client->newweapon = FindItem ("machinegun");
		
		ent->s.modelindex = robot_index;
		ent->s.modelindex2 = 0;
		CTFTripSkin (ent);
		ent->viewheight = 30;
		ent->mass = 500;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("rocket launcher"))] = 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] = 1;
		ent->client->newweapon = FindItem ("rocket launcher");
		ChangeWeapon (ent);
	}
	else if (ent->client->pers.passive == CRIP)
	{
		ent->s.modelindex = cripple_index;
		ent->s.modelindex2 = 0;
		CTFTripSkin (ent);
		ent->viewheight = 0;
		ent->mass = 100;
		ent->health = 40;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))] = 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] = 1;
		ent->client->newweapon = FindItem ("chaingun");
		ChangeWeapon (ent);
	}
	else if (ent->client->pers.passive == MAGE)
	{
		ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))] = 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] = 1;
		ent->client->newweapon = FindItem ("hyperblaster");
		ChangeWeapon (ent);
	}
	else if (ent->client->pers.passive == SUN)
	{
		ent->s.modelindex = sun_index;
		ent->s.modelindex2 = 0;
		ent->s.frame = 5;
		ent->s.skinnum = 0;
		ent->mass == 1000;
	}
	else if (ent->client->pers.passive == JEDI)
	{
		ent->client->pers.inventory[body_armor_index] = 50;
//		client->pers.weapon = NULL;
//		client->newweapon = NULL;
		client->ps.gunindex = 0;
	}
	else if (ent->client->pers.passive == ASS)
	{
		edict_t *target;
		target = randplayer(ent);

		ent->asstarg = target;

		if (target != NULL)
		{
			gi.cprintf (ent, PRINT_HIGH, "Your target is %s\n",Green1(target->client->pers.netname));
			gi.centerprintf (target, "There is a price on your head!\n");
		}
	}
	else if (ent->client->pers.passive == ARMOR)
	{
		ent->client->pers.inventory[body_armor_index] = 10;
	}

	if (ent->client->pers.combo != C_ASS)
	{
		ent->asstarg = NULL;
	}
// \SH
	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);

	gi.linkentity (ent);
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
//	PutClientInServer (ent);

	MakeObserver(ent);	// SH

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

// SH
	srand((unsigned)time(NULL));
	ent->client->pers.lucky = rand()%10;

	stuffcmd (ent, "bind ins  \"use a_inc\"\n");
	stuffcmd (ent, "bind del  \"use a_dec\"\n");
	stuffcmd (ent, "bind home \"use p_inc\"\n");
	stuffcmd (ent, "bind end  \"use p_dec\"\n");
	stuffcmd (ent, "bind pgup \"use s_inc\"\n");
	stuffcmd (ent, "bind pgdn \"use s_dec\"\n");
	stuffcmd (ent, "bind ] \"use c_inc\"\n");
	stuffcmd (ent, "bind [ \"use c_dec\"\n");
	stuffcmd (ent, "bind o \"use s_obs\"\n");
	stuffcmd (ent, "bind p \"use s_pwr\"\n");

	gi.centerprintf (ent, "Superheroes ][\n%s\n\nUse INS DEL HOME END PGUP PGDN\nto select your powers\nor [ and ] to use pre-made combos\n*O* will make you an observer\n*P* will display your powers\nHit fire to become a player\nWritten by SoulScythe\nphead@ucla.edu\nwww.planetquake.com/modsquad/super2\nYour lucky number is: %i", GetSub(), ent->client->pers.lucky);

// \SH

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
// SH
	stuffcmd (ent, "bind ins  \"use a_inc\"\n");
	stuffcmd (ent, "bind del  \"use a_dec\"\n");
	stuffcmd (ent, "bind home \"use p_inc\"\n");
	stuffcmd (ent, "bind end  \"use p_dec\"\n");
	stuffcmd (ent, "bind pgup \"use s_inc\"\n");
	stuffcmd (ent, "bind pgdn \"use s_dec\"\n");
	stuffcmd (ent, "bind ] \"use c_inc\"\n");
	stuffcmd (ent, "bind [ \"use c_dec\"\n");
	stuffcmd (ent, "bind o \"use s_obs\"\n");
	stuffcmd (ent, "bind p \"use s_pwr\"\n");
// \SH

	ent->client = game.clients + (ent - g_edicts - 1);

	if (deathmatch->value)
	{
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
		ent->classname = "player";
		InitClientResp (ent->client);
		MakeObserver(ent);		// SH
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
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char	*s;
	int		playernum;

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}

	// set name
	s = Info_ValueForKey (userinfo, "name");
	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;

// SH CTF
	if (ctf->value)
		CTFAssignSkin(ent, s);
	else
// /SH CTF
	// combine name and skin into a configstring
	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

	// fov
	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		ent->client->ps.fov = 90;
	}
	else
	{
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}

	// handedness
	s = Info_ValueForKey (userinfo, "hand");
	if (strlen(s))
	{
		ent->client->pers.hand = atoi(s);
	}

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
}

void ClientDisconnect (edict_t *); // SH
/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	char	*value;

// SH
	if (ent->client->resp.initialized)
	{
		gi.dprintf("%s: reconnect without disconnect\n",ent->client->pers.netname); 
		ClientDisconnect(ent);	
	}
// \SH
		// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");

	// check for a password
	value = Info_ValueForKey (userinfo, "password");
	if (strcmp(password->string, value) != 0)
		return false;

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

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf ("%s connected\n", ent->client->pers.netname);

	ent->client->pers.connected = true;
	return true;
}

/*
===========
ClientDisconnect

called when a player drops from the server

============
*/
void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!ent->client)
		return;

	gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);
// SH	
	if (ent->angel != NULL)
	{
		G_FreeEdict (ent->angel);
	}
	ent->client->resp.initialized = 0;
	CTFDrop_Flag (ent);

	ent->client->pers.active = 0;
	ent->client->pers.passive = 0;
	ent->client->pers.special = 0;
	ent->client->pers.combo = 0;
	ent->flags &= ~(FL_BLUETEAM|FL_REDTEAM);
// \SH
	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");
}


//==============================================================


void respawn (edict_t *self);

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
// SH
	if (ent->freezetime > level.time && !(ent->flags & FL_OBSERVER))
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		return;
	}

	if (!(ent->flags & FL_OBSERVER) && ent->deadflag == DEAD_NO)
	{
		if (ent->health > 999)
			ent->health = 999;

		if (ent->flags & FL_OPTIC && ent->teamchain == NULL)
		{
			edict_t *beam;

			beam = G_Spawn();
			beam->teammaster = ent;
			ent->teamchain = beam;
			beam->owner = ent;
			beam->spawnflags = 1 | 2;
			beam->enemy = NULL;

			if (ent->client->pers.active == OPTIC)
			{
				beam->spawnflags = 1 | 2;	// red
				beam->dmg = 12;
				beam->classname = "optic blast";
				beam->wait = 2;

//				gi.sound (ent, CHAN_AUTO, gi.soundindex ("weapons/laser2.wav"), 1, ATTN_NORM, 0);
				gi.sound (ent, CHAN_AUTO, rand_laser (3), 1, ATTN_NORM, 0);
			}
			else if (ent->client->pers.active == SABER || ent->client->pers.active == JEDI)
			{
				int type = rand()%7;

				if (ent->wait == 3)
				{
					beam->spawnflags = 1 | 2;		// red
					beam->dmg = 40;
				}
				else
				{
					beam->spawnflags = 1 | 8;		// blue
					beam->dmg = 22;
				}

				beam->classname = "light saber";
				beam->wait = 0.1;

				if (type == 0)
				{
					beam->random = -5.0;
					beam->delay = 0;
					beam->style = 0;
				}
				else if (type == 1)
				{
					beam->random = 5.0;
					beam->delay = 0;
					beam->style = 1;
				}
				else if (type == 2)
				{
					beam->random = 0;
					beam->delay = 0;
					beam->style = 2;
				}
				else if (type == 3)
				{
					beam->random = 0;
					beam->delay = 0;
					beam->style = 3;
				}
				else if (type == 4)
				{
					beam->random = 5;
					beam->delay = 5;
					beam->style = 4;
				}
				else if (type == 5)
				{
					beam->random = -5;
					beam->delay = 5;
					beam->style = 5;
				}
				else if (type == 6)
				{
					beam->random = 0;
					beam->delay = -5;
					beam->style = 6;
				}

//				gi.sound (ent, CHAN_AUTO, gi.soundindex ("powers/lightsaberon.wav"), 1, ATTN_IDLE, 0);
				gi.sound (ent, CHAN_AUTO, rand_laser(4), 1, ATTN_IDLE, 0);
			}
			else if (ent->client->pers.active == ROBOT)
			{
				if (ent->wait == 0)
				{
					beam->spawnflags = 1 | 4;	// green
					beam->dmg = 16;
					beam->classname = "robot laser";
					beam->wait = 0.5;
					gi.sound (ent, CHAN_AUTO, rand_laser (1), 1, ATTN_NORM, 0);
				}
				else
				{
					beam->spawnflags = 1 | 128 | 64;  // fat green
					beam->dmg = 24;
					beam->classname = "robot laser";
					beam->wait = 0.4;
					gi.sound (ent, CHAN_AUTO, rand_laser (2), 1, ATTN_NORM, 0);
				}
	
//				gi.sound (ent, CHAN_AUTO, gi.soundindex ("weapons/laser2.wav"), 1, ATTN_NORM, 0);
			}

			VectorCopy (ent->s.angles, beam->s.angles);
			target_laser_start (beam);
			beam->movetype = MOVETYPE_FLYMISSILE;
			gi.linkentity (beam);
		}

		if (ent->flags & FL_OPTIC && ent->teamchain != NULL && level.time + ent->teamchain->wait > ent->client->pers.active_finished && ent->client->pers.combo != C_JEDI)
		{
			target_laser_off (ent->teamchain);
			G_FreeEdict (ent->teamchain);
			ent->flags &= ~FL_OPTIC;
			ent->teamchain = NULL;
		}

		if (ent->flags & FL_OPTIC && ent->teamchain != NULL && level.time + ent->teamchain->wait > ent->jedi_time && ent->client->pers.combo == C_JEDI)
		{
			target_laser_off (ent->teamchain);
			G_FreeEdict (ent->teamchain);
			ent->flags &= ~FL_OPTIC;
			ent->teamchain = NULL;
			ent->client->weaponstate = WEAPON_READY;
		}

		if (ent->flags & FL_DEATHBLOW)
		{
			if (((((int)(ent->client->pers.active_finished - level.time))*10)%6) == 0)
			{
				gi.sound (ent, CHAN_AUTO, gi.soundindex("world/scan1.wav"), 1, ATTN_NORM, 0);
			}
		}

		if (ent->cards > 0 && ent->client->pers.active_finished - ((float)(ent->cards)/10) - 0.3 < level.time)
		{
			card_fire (ent, ent->cards, 14, 2000, 0, 0);
			ent->cards -= 1;

//			if (ent->cards)
//				ent->client->pers.active_finished = level.time + 0.1;
//			else
//				ent->client->pers.active_finished = level.time + 0.3;
		}

		if (ent->client->pers.combo == C_ROBOT)
		{
			int index;
			index = ArmorIndex (ent);
			if (index != 0)
			{
				if (ent->wait == 1 && client->pers.inventory[index] < 100)
				{
					ent->wait = 0;
					ent->max_health = 250;
					gi.cprintf (ent, PRINT_HIGH, "Killer Robot Upgrade Lost!\n");
				}
				else if (ent->wait == 0 && client->pers.inventory[index] >= 100)
				{
					ent->wait = 1;
					ent->max_health = 300;
					gi.cprintf (ent, PRINT_HIGH, "Killer Robot Upgrade!\n");
				}

				if (ent->health > ent->max_health)
					ent->health = ent->max_health;
			}
		}

		if (ent->client->pers.combo == C_ASS)
		{
			if (ent->s.modelindex == 255 && ent->client->weaponstate == WEAPON_READY && ent->light_level <= 50)
			{
				MakeInvis (ent);
			}
			else if (ent->s.modelindex == invis_index && (ent->light_level > 50 || ent->client->weaponstate != WEAPON_READY))
			{
				MakeVis (ent);
			}
			if (ent->s.modelindex == invis_index && rand()%10 == 0)
			{
				vec3_t	loc;
				avrandom (loc);
				VectorMA (ent->s.origin, 100, loc, loc);
				SpawnDamage (TE_SHOTGUN, loc, NULL, 10);
			}
		}

		if (ent->client->pers.passive == REPULSE && ent->client->pers.passive_finished < level.time)
		{
			edict_t *cur;
			vec3_t	vel;

//			cur = world + 1;
			cur = findradius (world, ent->s.origin, 250);			
			while (cur != NULL)
			{

				if (cur == world || cur == ent || (cur->client && cur->client->pers.passive == DENSE))
				{
					cur = findradius (cur, ent->s.origin, 250);
					continue;
				}

				if (cur != ent && (cur->takedamage || cur->movetype == MOVETYPE_BOUNCE || cur->movetype == MOVETYPE_FLYMISSILE) && cur != world && cur->solid != SOLID_BSP && !(cur->flags & FL_OBSERVER))
				{
//gi.cprintf(ent, PRINT_HIGH, "repulse: %s\n", cur->classname);
					VectorSubtract (cur->s.origin, ent->s.origin, vel);
					VectorNormalize2 (vel, vel);
					if (cur->groundentity)
					{
						cur->groundentity = NULL;
// ONE					cur->s.origin[2] += 1;
					}
					VectorMA (cur->velocity, 1300 * FRAMETIME, vel, cur->velocity);
					VectorMA (cur->velocity, 12 * FRAMETIME, ent->velocity, cur->velocity);
				}

				cur = findradius (cur, ent->s.origin, 200);
			}
			ent->client->pers.passive_finished = level.time + 0.1;
		}

		if (ent->client->pers.passive == REGEN && ent->client->pers.passive_finished < level.time)
		{
			float del = 0.8;

			if (ent->health < 30)
			{
				ent->health += 1;  
				del = 0.3;
			}
			else if (ent->health < 100)
			{
				ent->health += 1;
				del = 0.6;
			}
			else if (ent->health < 200)
			{
				ent->health += 1;
				del = 0.9;
			}
			else if (ent->health < 250)
			{
				ent->health += 1;
				del = 1.5;
			}
//			if (ent->health < 250)
//				ent->health += 1;

			ent->client->pers.passive_finished = level.time + del;
		}

		if (ent->client->pers.passive == RADIO && ent->client->pers.passive_finished < level.time)
		{
			if (rand()%10 < 5)
			{
				T_RadiusDamage (ent, ent, 8, ent, 200, DAMAGE_RADIO, MOD_RADIO);
			}
			ent->client->pers.passive_finished = level.time + 0.1;
		}

		if (ent->client->pers.passive == SUN && ent->client->pers.passive_finished < level.time)
		{
			if (rand()%10 < 5)
			{
				T_RadiusDamage (ent, ent, 15, ent, 100, DAMAGE_RADIO, MOD_SOLAR);
			}
			ent->client->pers.passive_finished = level.time + 0.1;
		}

		if (ent->client->pers.passive == INVIS || ent->client->pers.passive == JEDI)
		{
			int chance = 5;
			
			if (ent->client->pers.passive == JEDI)
			{
				chance = 10-(VectorLength (ent->velocity))/60;
				if (chance < 5)
					chance = 5;

				if (chance > 10)
					chance = 10;
			}

			if (ent->client->pers.passive_finished < level.time && ent->s.modelindex == 255 && ent->client->weaponstate == WEAPON_READY)
			{
				gi.sound(ent, CHAN_ITEM, gi.soundindex("world/force2.wav"), 1, ATTN_NORM, 0);
				MakeInvis (ent);
			}
			else if (ent->client->pers.passive_finished > level.time && ent->s.modelindex == invis_index)
			{
				MakeVis (ent);
			}
			
			if (ent->s.modelindex == invis_index && chance > 0 && rand()%chance == 0)
			{
				vec3_t	loc;
				avrandom (loc);
				VectorMA (ent->s.origin, 50, loc, loc);
				SpawnDamage (TE_SHOTGUN, loc, NULL, 15);
			}
		}

		if (ent->client->pers.passive == PRISMATIC && ent->client->pers.passive_finished < level.time)
		{
			if (rand()%100 < 5)
			{
				if (ent->flags & FL_BLUE)
					ent->flags &= ~FL_BLUE;
				else
					ent->flags |= FL_BLUE;
			}
			if (rand()%100 < 5)
			{
				if (ent->flags & FL_GREEN)
					ent->flags &= ~FL_GREEN;
				else
					ent->flags |= FL_GREEN;
			}
			if (rand()%100 < 5)
			{
				if (ent->flags & FL_RED)
					ent->flags &= ~FL_RED;
				else
					ent->flags |= FL_RED;
			}

			ent->client->pers.passive_finished = level.time + 0.1;
		}
		if (ent->client->pers.passive == LFORM)
		{
			if (ent->watertype & (CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA) && ent->s.modelindex == 255)
			{
				MakeInvis (ent);
			}
			else if (!(ent->watertype & (CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA)) && ent->s.modelindex == invis_index)
			{
				MakeVis (ent);
			}
			
			if (ent->s.modelindex == invis_index && rand()%10 == 0)
			{
				vec3_t	loc;
				avrandom (loc);
				VectorMA (ent->s.origin, 50, loc, loc);
				SpawnDamage (TE_SHOTGUN, loc, NULL, 10);
			}
		}

		if (ent->client->pers.passive == SHIN)
		{
			if (ent->light_level < 40 && ent->client->pers.passive_finished < level.time)
			{
				ent->client->pers.passive_finished = level.time + 0.5 + (ent->light_level / 10);
				T_Damage (ent, ent, ent, v_up, ent->s.origin, NULL, 1, 0, DAMAGE_NO_PROTECTION, MOD_LIGHT);
			}

			if (ent->light_level > 85 && ent->client->pers.passive_finished < level.time)
			{
				ent->client->pers.passive_finished = level.time + 4 - ((ent->light_level - 85) / 10);
				if (ent->client->pers.passive_finished < level.time + 0.5)
					ent->client->pers.passive_finished = level.time + 0.5;
				ent->health += 1;
				if (ent->health > 200)
					ent->health = 200;
			}
//			gi.cprintf (ent, PRINT_HIGH, "%i\n", ent->light_level);
		}

		if (ent->client->pers.passive == ARMOR && ent->client->pers.passive_finished < level.time)
		{
			int index;
			index = ArmorIndex (ent);
			if (index != 0)
			{
				ent->client->pers.inventory[index]++;

				if (ent->client->pers.inventory[index] > 500)
					ent->client->pers.inventory[index] = 500;

				ent->client->pers.passive_finished = level.time + 0.8;
			}
		}

		if (ent->client->pers.special == VAMP && ent->client->pers.special_finished < level.time)
		{
			if (ent->health < 101)
			{
				ent->client->pers.special_finished = level.time + 1.1;
			}
			else if (ent->health < 200)
			{
				ent->health -= 1;
				ent->client->pers.special_finished = level.time + 1.6;
			}
			else if (ent->health < 300)
			{
				ent->health -= 1;
				ent->client->pers.special_finished = level.time + 0.9;
			}
			else if (ent->health < 400)
			{
				ent->health -= 1;
				ent->client->pers.special_finished = level.time + 0.5;
			}
			else if (ent->health < 500)
			{
				ent->health -= 3;
				ent->client->pers.special_finished = level.time + 0.6;
			}
			else
			{
				ent->health -= 3;
				ent->client->pers.special_finished = level.time + 0.2;
			}
		}
		if (ent->client->pers.special == ELEC && ent->waterlevel > 0 && rand()%10 < 1 && ent->client->pers.special_finished < level.time)
		{
//			MV (ent);
			T_RadiusDamage (ent, world, ent->waterlevel, NULL, 200, DAMAGE_RADIO, MOD_ELEC);
			ent->client->pers.special_finished = level.time + 0.1;
		}

		if (ent->client->pers.special == BERSERK && ent->flags & FL_BERSERK && ent->client->pers.special_finished < level.time && ent->health > 0)
		{
			MV (ent);
			T_Damage (ent, ent, ent, v_up, ent->s.origin, NULL, 10000, 0, DAMAGE_NO_PROTECTION, MOD_BERSERK);
			ent->flags &= ~FL_BERSERK;
		}

		if (ent->client->pers.special == BERSERK && !(ent->flags & FL_BERSERK) && ent->health < 20 && ent->health > 0)
		{
			ent->flags |= FL_BERSERK;
			ent->health = 300;
			ent->client->pers.special_finished = level.time + 15;
			gi.bprintf (PRINT_HIGH, "%s goes berserk!\n", ent->client->pers.netname);
			
			ent->client->pers.inventory[ITEM_INDEX(FindItem("rocket launcher"))] = 1;
			ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] = 100;
			ent->client->newweapon = FindItem ("rocket launcher");
			ChangeWeapon (ent);

			if (IsFemale (ent))
				gi.sound(ent, CHAN_AUTO, gi.soundindex("powers/berzfemale.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_AUTO, gi.soundindex("powers/berzmale.wav"), 1, ATTN_NORM, 0);

//			ent->client->resp.score--;
			if (ent->enemy->client && ent->enemy != ent)
			{
				ent->enemy->client->resp.score++;
			}
		}

		if ((ent->client->pers.special == AOLIFE || ent->client->pers.special == AODEATH || ent->client->pers.special == AOMERCY) && ent->angel == NULL)
		{
			spawn_angel (ent);
		}

		if (ent->bleed > 0 && ent->health > 5 && rand()%35 < ent->bleed && ent->bleed_time <= level.time)
		{
			MV (ent);
			ent->health -= 1;
			ent->bleed_time = level.time + FRAMETIME;
			SpawnDamage (TE_BLOOD, ent->s.origin, v_forward, 100);
//			gi.centerprintf(ent, "You are bleeding\nFind health quickly!\n");
		}
		if (ent->bleed > 0 && ent-> health < 6)
		{
			ent->bleed = 0;
		}

		if (ent->bleed > 0 && ent->health >= 90 )
		{
			ent->bleed -= 1;
		}
		
		if (ent->ants > level.time)
		{
			if (ent->groundentity)
			{
				ucmd->upmove = 300;
//				ent->groundentity = NULL;
//				ent->velocity[2] = 300;
			}
		}
	}
// \SH
	pm_passent = ent;

	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (!(ent->flags & FL_OBSERVER) && ent->s.modelindex != 255 && ent->s.modelindex != invis_index && ent->s.modelindex != robot_index && ent->s.modelindex != cripple_index && ent->s.modelindex != sun_index) // SH
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag)
		client->ps.pmove.pm_type = PM_DEAD;
	else
		client->ps.pmove.pm_type = PM_NORMAL;

	client->ps.pmove.gravity = sv_gravity->value;	
	pm.s = client->ps.pmove;

// SH
	if (ent->client->pers.passive == SPEED && !(ent->flags & FL_OBSERVER) && ent->health > 0)
	{
		float vel = 0, mul = 0;

		MV (ent);
//		VectorMA (ent->velocity, 500, v_forward, ent->velocity);
		if (ucmd->forwardmove > 0)
		{
			ent->velocity[0] += 1200 * v_forward[0] * framerate(ucmd);
			ent->velocity[1] += 1200 * v_forward[1] * framerate(ucmd);
		}
		else if (ucmd->forwardmove < 0)
		{
			ent->velocity[0] -= 1200 * v_forward[0] * framerate(ucmd);
			ent->velocity[1] -= 1200 * v_forward[1] * framerate(ucmd);
		}

		if (ucmd->sidemove > 0)
		{
			ent->velocity[0] += 1200 * v_right[0] * framerate(ucmd);
			ent->velocity[1] += 1200 * v_right[1] * framerate(ucmd);
		}
		else if (ucmd->sidemove < 0)
		{
			ent->velocity[0] -= 1200 * v_right[0] * framerate(ucmd);
			ent->velocity[1] -= 1200 * v_right[1] * framerate(ucmd);
		}

		if (ucmd->upmove >= 10 && ent->groundentity)
		{
			ent->velocity[2] += 300;
		}

//		ent->velocity[0] *= 1.15;
//		ent->velocity[1] *= 1.15;

		vel = sqrt (ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);

		if (vel > 550)
		{
			mul = 550 / vel;
			ent->velocity[0] *= mul;
			ent->velocity[1] *= mul;
		}

	}
	else if (ent->client->pers.passive == ROBOT & !(ent->flags & FL_OBSERVER) && ent->health > 0)
	{
		float vel = 0, mul = 0, max_vel = 0;

		if (ent->wait == 0)
			max_vel = 180;
		else
			max_vel = 250;

		vel = sqrt (ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);

		if (vel > max_vel)
		{
			mul = max_vel / vel;
			ent->velocity[0] *= mul;
			ent->velocity[1] *= mul;
		}
		if (ucmd->upmove >= 10)// && ent->client->pers.passive_finished < level.time)
		{
			ent->velocity[2] += 900 * framerate(ucmd);
			if (ent->velocity[2] > max_vel)
				ent->velocity[2] = max_vel;
			ent->groundentity = NULL;
			gi.sound(ent, CHAN_AUTO, gi.soundindex ("weapons/rockfly.wav"), 0.35, ATTN_NORM, 0);
		}
	}
	else if (ent->client->pers.passive == CRIP && !(ent->flags & FL_OBSERVER) && ent->health > 0)
	{
		float vel = 0, mul = 0;

		MV (ent);
		if (ucmd->forwardmove > 0)
		{
			ent->velocity[0] += 1200 * v_forward[0] * framerate(ucmd);
			ent->velocity[1] += 1200 * v_forward[1] * framerate(ucmd);
		}
		else if (ucmd->forwardmove < 0)
		{
			ent->velocity[0] -= 1200 * v_forward[0] * framerate(ucmd);
			ent->velocity[1] -= 1200 * v_forward[1] * framerate(ucmd);
		}

		if (ucmd->sidemove > 0)
		{
			ucmd->sidemove /= 5;
		}
		else if (ucmd->sidemove < 0)
		{
			ucmd->sidemove /= 5;
		}

		if (ucmd->upmove >= 10 && ent->groundentity)
		{
			ent->velocity[2] += 300;
		}

		vel = sqrt (ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);

		if (vel > 500)
		{
			mul = 500 / vel;
			ent->velocity[0] *= mul;
			ent->velocity[1] *= mul;
		}
	}
	else if ((ent->client->pers.passive == FLY || (ent->flags & FL_OBSERVER)) && ent->health > 0)
	{
		float vel = 0, mul = 0;

		MV (ent);

		if (ucmd->upmove >= 10)
		{
			ent->velocity[2] += 1700 * framerate(ucmd);
			if (ent->velocity[2] > 300)
				ent->velocity[2] = 300;
			ent->groundentity = NULL;
			ent->client->pers.passive_finished = level.time + 0.1;
		}
		if (ent->groundentity == NULL)
		{
			if (ucmd->forwardmove > 0)
			{
				ent->velocity[0] += 1000 * v_forward[0] * framerate(ucmd);
				ent->velocity[1] += 1000 * v_forward[1] * framerate(ucmd);
			}
			else if (ucmd->forwardmove < 0)
			{
				ent->velocity[0] -= 1000 * v_forward[0] * framerate(ucmd);
				ent->velocity[1] -= 1000 * v_forward[1] * framerate(ucmd);
			}
	
			if (ucmd->sidemove > 0)
			{
				ent->velocity[0] += 1000 * v_right[0] * framerate(ucmd);
				ent->velocity[1] += 1000 * v_right[1] * framerate(ucmd);
			}
			else if (ucmd->sidemove < 0)
			{
				ent->velocity[0] -= 1000 * v_right[0] * framerate(ucmd);
				ent->velocity[1] -= 1000 * v_right[1] * framerate(ucmd);
			}
		}

		vel = sqrt (ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1] + ent->velocity[2]*ent->velocity[2]);

		if (vel > 300)
		{
			mul = 300 / vel;
			ent->velocity[0] *= mul;
			ent->velocity[1] *= mul;
			ent->velocity[2] *= mul;
		}

	}
	else if (ent->client->pers.passive == DENSE && !(ent->flags & FL_OBSERVER))
	{
		if (!ent->groundentity)
		{
			ent->velocity[2] -= 900 * v_up[2] * framerate(ucmd);
		}

		if (ent->waterlevel)
		{
			ucmd->upmove = 0;
		}
	}


//	if (VectorLength (ent->velocity) != 0)
//		gi.cprintf (ent, PRINT_HIGH, "%f\n", VectorLength (ent->velocity));

// \SH

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
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

// SH
	if (!(ent->flags & FL_OBSERVER) && ent->client->pers.passive != ROBOT && ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
	{		
// SH
		if ((ent->client->pers.special == JUMP || ent->client->pers.special == JEDI) && !(ent->flags & FL_HASFLAG))
		{
			AngleVectors (ent->client->v_angle, v_forward, v_right, v_up);
			VectorClear (ent->velocity);
			VectorMA(ent->velocity, 850, v_forward, ent->velocity);
			VectorMA(ent->velocity, 210, v_up, ent->velocity);
		}
		if (ent->client->pers.combo != C_ASS && ent->client->pers.combo != C_SUN)
		{
// \SH
			gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
		} // SH
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
// SH                                             
	if (ent->movetype != MOVETYPE_NOCLIP && !(ent->flags & FL_OBSERVER))
// \SH
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

// SH
		if (!(ent->flags & FL_OBSERVER) && other->takedamage && ent->client->pers.passive == BOOT && (ent->client->pers.passive_finished < level.time) && !CTFSameTeam (ent, other))
		{
			boot (ent, other, 60);
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("powers/bootf.wav"), 1, ATTN_NORM, 0);
//			gi.sound (ent, CHAN_VOICE, gi.soundindex ("powers/bootf.wav"), 1, ATTN_NORM, 0);
//			gi.cprintf (ent, PRINT_HIGH, "Boot to the head!\n");
//			if (other->client)
//				gi.cprintf (other, PRINT_HIGH, "Boot to the head!\n");
			ent->client->pers.passive_finished = level.time + 1;

		}
		if (!(ent->flags & FL_OBSERVER) && other->takedamage && ent->client->pers.passive == CRIP && (ent->client->invincible_framenum > level.framenum) && !CTFSameTeam (ent, other))
		{
			boot (ent, other, 20);
		}
// \SH

		if (!other->touch)
			continue;

		other->touch (other, ent, NULL, NULL);
	}


	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (!(ent->flags & FL_OBSERVER) && (client->latched_buttons & BUTTON_ATTACK || ent->ants > level.time)) // SH
	{
// SH
		if (ent->client->pers.passive == INVIS)
		{
			ent->client->pers.passive_finished = level.time + 3;
		}
// \SH
		if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
// SH
//		if (rand()%10 == 0 && ent->ants > level.time)
//		{
//			ent->client->pers.weapon->drop (ent, ent->client->pers.weapon);
//		}
// \SH
	}
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			buttonMask;

	if (level.intermissiontime)
		return;

	client = ent->client;

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk)
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

			if ( ( client->latched_buttons & buttonMask ) ||
				(deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) ) )
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

	client->latched_buttons = 0;
}
