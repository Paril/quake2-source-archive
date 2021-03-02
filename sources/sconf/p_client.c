#include "g_local.h"
#include "m_player.h"
// SCONFIG START
#include "s_readconfig.h"
#include "stdlog.h"
// Gender MOD
#include "gender.h"
// ServObit
#include "SrvObAPI.h"
// SCONFIG STOP

void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void endLevelshowTop10(edict_t *ent);
void SP_misc_teleporter_dest (edict_t *ent);

// SCONFIG START
void logFrag( char *frager, char *fragee ); // p_client reference
// SCONFIG STOP

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

void stuffcmd2(edict_t *ent, char *s) 	
{
   	gi.WriteByte (11);	        
	gi.WriteString (s);
    gi.unicast (ent, true);	
}

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
			if ((!self->targetname) || stricmp(self->targetname, spot->targetname) != 0)
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

	if(stricmp(level.mapname, "security") == 0)
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
	if(stricmp(level.mapname, "security") == 0)
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

	if((stricmp(level.mapname, "jail2") == 0)   ||
	   (stricmp(level.mapname, "jail4") == 0)   ||
	   (stricmp(level.mapname, "mine1") == 0)   ||
	   (stricmp(level.mapname, "mine2") == 0)   ||
	   (stricmp(level.mapname, "mine3") == 0)   ||
	   (stricmp(level.mapname, "mine4") == 0)   ||
	   (stricmp(level.mapname, "lab") == 0)     ||
	   (stricmp(level.mapname, "boss1") == 0)   ||
	   (stricmp(level.mapname, "fact3") == 0)   ||
	   (stricmp(level.mapname, "biggun") == 0)  ||
	   (stricmp(level.mapname, "space") == 0)   ||
	   (stricmp(level.mapname, "command") == 0) ||
	   (stricmp(level.mapname, "power2") == 0) ||
	   (stricmp(level.mapname, "strike") == 0))
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


void ORIGINAL_ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	if (deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
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
			if (deathmatch->value) { // SCONFIG ADDED {
				self->client->resp.score--;
				// SCONFIG START
				if (QWLOG == 1) {
					logFrag(self->client->pers.netname,self->client->pers.netname);
				}
				// SCONFIG STOP

			}
			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
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
			case MOD_GRAPPLE:
				message = "was caught by";
				message2 = "'s grapple";
			break;
			
			}



			if (message)
			{
				gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if (deathmatch->value)
				{
					if (ff) {
						attacker->client->resp.score--;
						// SCONFIG START
						if (QWLOG == 1)
					 	  logFrag(attacker->client->pers.netname, self->client->pers.netname);
						// SCONFIG STOP
					}
					else {
						attacker->client->resp.score++;
						// SCONFIG START
						if (QWLOG == 1)
						  logFrag(attacker->client->pers.netname, self->client->pers.netname);
						// SCONFIG STOP
					}
				}
				return;
			}
		}
	}

	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (deathmatch->value)
		self->client->resp.score--;
}

// START SCONFIG
// ServObit
// Renamed the original clientObituary() to ORIGINAL_ClientObituary().
// This function is basically a wrapper.
void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	// if ServObit can't find an obituary, use the original obit code.
	if (! ServObitClientObituary(self, inflictor, attacker))
		ORIGINAL_ClientObituary(self, inflictor, attacker);
}
// End ServObit
// END SCONFIG

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

	// Sconfig 2.6 (grapple)
	// Expert: Release hook if needed
	if (self->client->hook)
		Release_Grapple(self->client->hook);

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
		
		// SCONFIG START
		sl_WriteStdLogDeath( &gi, level, self, inflictor, attacker);	// StdLog - Mark Davies
		// SCONFIG STOP
		
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

// ***************************************************
// SCONFIG START
void UseMe(edict_t *ent, char *s)
{
	int			index;
	gitem_t		*it;
	
	it = FindItem (s);
	if (!it)
	{
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		return;
	}

	it->use (ent ,it);
}
// SCONFIG STOP
// ***************************************************


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

	// ************************************************************
	// SCONFIG START 
	// Prototype
	void startWeapon(gclient_t *client, gitem_t *item, char *myWeapon);
	void startAmmo(gclient_t *client, gitem_t *item, char *myAmmo, int count);
	// SCONFIG STOP
	// ************************************************************

	
	memset (&client->pers, 0, sizeof(client->pers));

	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	client->pers.weapon = item;

	// ************************************************************
	// SCONFIG START 
	/* Take out fixed values, replace with these            */
	/* Assign Health and Max values based on Config entries */
	client->pers.health			= CF_StartHealth;
	client->pers.max_health		= CF_MaxHealth;
	client->pers.max_bullets	= MA_Bullets;
	client->pers.max_shells		= MA_Shells;
	client->pers.max_rockets	= MA_Rockets;
	client->pers.max_grenades	= MA_Grenades;
	client->pers.max_cells		= MA_Cells;
	client->pers.max_slugs		= MA_Slugs;
	
	// sconfig 2.8
//	client->wasingame=1;  // SET IT HERE
	// end sconfig 2.8

	// Assign starting items if any (I want QUAD!) :)
	if (SI_QuadDamage==1) {
	  item = FindItem("Quad Damage");
	  client->pers.selected_item = ITEM_INDEX(item);
      client->pers.inventory[ITEM_INDEX(item)]++;
	}
		
	if (RegenInvulnerability==1) {
	  item = FindItem("Invulnerability");
	  client->pers.selected_item = ITEM_INDEX(item);
    }
	 
	if (SI_Invulnerability==1) {
	  item = FindItem("Invulnerability");
	  client->pers.selected_item = ITEM_INDEX(item);
      client->pers.inventory[ITEM_INDEX(item)]++;
	}

	if (SI_Silencer==1) {
	  item = FindItem("Silencer");
	  client->pers.selected_item = ITEM_INDEX(item);
      client->pers.inventory[ITEM_INDEX(item)]++;
	}

	if (SI_Rebreather==1) {
	  item = FindItem("Rebreather");
	  client->pers.selected_item = ITEM_INDEX(item);
      client->pers.inventory[ITEM_INDEX(item)]++;
	}

	if (SI_EnvironmentSuit==1) {
	  item = FindItem("Environment Suit");
	  client->pers.selected_item = ITEM_INDEX(item);
      client->pers.inventory[ITEM_INDEX(item)]++;
    }

	if (SI_PowerScreen==1) {
	  item = FindItem("Power Screen");
	  client->pers.selected_item = ITEM_INDEX(item);
      client->pers.inventory[ITEM_INDEX(item)]++;
	}

	if (SI_PowerShield==1) {
	  item = FindItem("Power Shield");
	  client->pers.selected_item = ITEM_INDEX(item);
      client->pers.inventory[ITEM_INDEX(item)]++;
	}


	// Assign starting weapons
	if (SW_ShotGun == 1)
		startWeapon(client, item, "shotgun");
	if (SW_SuperShotGun == 1)
		startWeapon(client, item, "super shotgun");
	if (SW_MachineGun == 1)
		startWeapon(client, item, "machinegun");
	if (SW_ChainGun == 1)
		startWeapon(client, item, "chaingun");
	if (SW_GrenadeLauncher == 1)
		startWeapon(client, item, "grenade launcher");
	if (SW_RocketLauncher == 1)
		startWeapon(client, item, "rocket launcher");
	if (SW_HyperBlaster == 1)
		startWeapon(client, item, "hyperblaster");
	if (SW_RailGun == 1)
		startWeapon(client, item, "railgun");
	if (SW_BFG10K == 1)
		startWeapon(client, item, "BFG10K");

	// Assign Starting Ammo and quantity
	if (SA_Bullets > 0 && SA_Bullets <= MA_Bullets)
		startAmmo(client, item, "bullets", SA_Bullets);
	if (SA_Shells > 0 && SA_Shells <= MA_Shells)
		startAmmo(client, item, "shells", SA_Shells);
	if (SA_Cells > 0 && SA_Cells <= MA_Cells)
		startAmmo(client, item, "cells", SA_Cells);
	if (SA_Grenades > 0 && SA_Grenades <= MA_Grenades)
		startAmmo(client, item, "grenades", SA_Grenades);
	if (SA_Rockets > 0 && SA_Rockets <= MA_Rockets)
		startAmmo(client, item, "rockets", SA_Rockets);
	if (SA_Slugs > 0 && SA_Slugs <= MA_Slugs)
		startAmmo(client, item, "slugs", SA_Slugs);
	// SCONFIG STOP 
	// ************************************************************
//	spectator=1; // Set spectator mode 1

	client->pers.connected = true;
}


// ************************************************************
// SCONFIG START 
/* Added function to handle adding weapons */
void startWeapon(gclient_t *client, gitem_t *item, char *myWeapon)
{
	item = FindItem(myWeapon);
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1; // ITEM_INDEX(item)
	client->pers.weapon = item;
}
/* Added function to handle adding ammo */
void startAmmo(gclient_t *client, gitem_t *item, char *myAmmo, int count)
{
	int index;

	item = FindItem(myAmmo);
	index = ITEM_INDEX(item);
	client->pers.inventory[index] += count;
	if (client->pers.inventory[index] > client->pers.max_cells)
		client->pers.inventory[index] = client->pers.max_cells;
}
// SCONFIG STOP
// ************************************************************


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
	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
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
		spot = SelectDeathmatchSpawnPoint ();
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
		
		// sconfig 2.8
		self->client->menu_state = 2; // 1 = not in game; 2 = in game
		// end sconfig 2.8
		
		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

		// Sconfig 2.6 (grapple)
		// Expert: prevent grappling on immediate respawn
		self->client->hook_frame = level.framenum + 1;

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
// SCONFIG START
// ### Hentai ### BEGIN
void ShowGun(edict_t *ent);
// ###	Hentai ### END
// SCONFIG STOP
void PutClientInServer (edict_t *ent)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i,rate;
	client_persistant_t	saved;
	client_respawn_t	resp;
	char	*s;

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


	s = Info_ValueForKey(client->pers.userinfo, "rate"); // string rep
	rate = atoi(Info_ValueForKey(client->pers.userinfo, "rate")); // numeric
	
    if (rate > MAX_CLIENT_RATE) { // Put variables here 
		Info_SetValueForKey (client->pers.userinfo, "rate", MAX_CLIENT_RATE_STRING);
		//gi.dprintf("Your rate was = %i, Server MAX is = %s\nSetting Adjusted!\n",rate, MAX_CLIENT_RATE_STRING);
		//s = Info_ValueForKey(userinfo, "rate"); // string rep
		//rate = atoi(Info_ValueForKey(userinfo, "rate")); // numeric
		//gi.dprintf("Your new rate is: %i\n",rate);
	}
	else {
		Info_SetValueForKey (client->pers.userinfo, "rate", s);
	    //gi.dprintf("Your rate was = %i, Server MAX is = %i\nSetting Adjusted!\n",rate, MAX_CLIENT_RATE);
		//s = Info_ValueForKey(userinfo, "rate"); // string rep
		//rate = atoi(Info_ValueForKey(userinfo, "rate")); // numeric
		//gi.dprintf("Your new rate is: %i\n",rate);
	}	


	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 255;		// will use the skin specified model
	//ent->s.modelindex2 = 255;		// custom gun model
	// SCONFIG START
	//if (VWPATCH != 0) {
	ShowGun(ent);					// ### Hentai ### special gun model
	//}
	// SCONFIG STOP
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
		
	// SCONFIG START
	// ZOID
	if (CTFStartClient(ent))
		return;

	// ZOID
	// SCONFIG STOP

	
	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);


	// *****************************************************
	// SCONFIG START
	// Auto use items, upon startup
	if (AutoUseQuad == 1)
	  UseMe (ent,"Quad Damage"); // Use quad automatically if flag is set
	if (AutoUseInvulnerability == 1)
	  UseMe (ent,"Invulnerability");
	if (EnvironmentSuitTime > 0)
		UseMe (ent,"Environment Suit");
	if (RebreatherTime > 0)
		UseMe (ent,"Rebreather");
	if (SilencerShots > 0)
	  UseMe (ent,"Silencer");
	if (RegenInvulnerability == 1) {
		if (ent->client->invincible_framenum > level.framenum)
		  ent->client->invincible_framenum += RegenInvulnerabilityTime;
	    else
		  ent->client->invincible_framenum = level.framenum + RegenInvulnerabilityTime;
    	
		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
	}
	// SCONFIG STOP
	// *****************************************************

	

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

	 // -- ZBOT DETECTION CODE STARTS -- Version 2.5z //
     ent->client->resp.bot_start = level.time + 5;
     ent->client->resp.bot_retry = 3;
    // -- NEW CODE ENDS -- //

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	// ServObit: Print connect message

	gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

	//ServObitAnnounceConnect(ent);
	//ServObitPrintWelcome(ent);
	// End ServObit

	// SCONFIG START
	sl_WriteStdLogPlayerEntered( &gi, level, ent );	// StdLog - Mark Davies
	// SCONFIG STOP

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

	if (allowgrapple == 1)
		stuffcmd2(ent, "alias +hook +use; alias -hook -use;\n");

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
			// VErsion 2.3 (samplename)
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

	//ThreadZ  (RATE) Version 2.3
    int    rate;
	
	//ThreadZ
	

	// version 2.3
		// Clan Banning
	    int flaggg=0;
        FILE    *banlist;
		char bandir[50];
		char restorename[50];
        // Variable to handle the file.
        char    line2[50];
        // To read the file line byline.
	// end version 2.3

		int qq;

	// Version 2.3
	// Let's clean out the temp string.
		for (qq=0; qq<50; qq++) {
		bandir[qq] = '\0';
		line2[qq]='\0';
		restorename[qq]='\0';
		}
		
	// End version 2.341

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}
    
	
	s = Info_ValueForKey (userinfo, "name");
		
    // VERSION 2.3
    if (ingamenamebanningstate == 1)  // If ingamenamebanning variable set in config file is turned on...
	{
	      sprintf(bandir, "%sbanlist.txt", bandirectory);
           if ( banlist = fopen(bandir, "r") )
           {
			  while ( fgets(line2, 50, banlist) ) // We can still read...
              {
                 line2[strlen(line2)-1] = '\0';    // chop off the trailing \n
            
				 if (matchfullnamevalue==1) {
				 // Perform 'exact match'
					if ( strcmp(s, line2 )==0)  // if we found the current nick...
					{
					  gi.centerprintf(ent, "That name is banned!\n");
					  gi.dprintf("%s tried to change name to banned match of %s\n",ent->client->pers.netname,s);
					  strcat(restorename, "name ");
					  strcat(restorename, ent->client->pers.netname);
					  stuffcmd2(ent,restorename);
				 	  gi.sound(ent, CHAN_VOICE, gi.soundindex("berserk/sight.wav"), .2, ATTN_IDLE, 0);
                      flaggg=1;
					 }
				 } else {
				 
				    if ( strstr(s, line2 ))  // if we found the current word in the nick...
					{
					  gi.centerprintf(ent, "That name is banned!\n");
					  gi.dprintf("%s tried to change name to banned match of %s\n",ent->client->pers.netname,s);
					  strcat(restorename, "name ");
					  strcat(restorename, ent->client->pers.netname);
					  stuffcmd2(ent,restorename);
				 	  gi.sound(ent, CHAN_VOICE, gi.soundindex("berserk/sight.wav"), .2, ATTN_IDLE, 0);
                      flaggg=1;
					 }
				 } // end else (fullnamevalue)
			  }
              
			    fclose(banlist);                        // Give the file back to everyone.
	   
				if (flaggg == 0) {
					
					strncpy (ent->client->pers.netname, /*ent->client->pers.netname*/ s, sizeof(ent->client->pers.netname)-1);
					
				}


		  } else {                                           // Can't open the file.
            gi.dprintf("Can't open %s\n",bandir);
	   }
      }
    // END VERSION 2.3

	else  // when namebanning = 0 perform change as normal...
	// set name (modified for 2.3 name banning)
		strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);
	
	
	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;

	// combine name and skin into a configstring
	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );
	
	//Added v3.0

	gi.configstring (NICENAME+playernum, va("%s", ent->client->pers.netname) );
	

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



	s = Info_ValueForKey(userinfo, "rate"); // string rep
	rate = atoi(Info_ValueForKey(userinfo, "rate")); // numeric
	
    if (rate > MAX_CLIENT_RATE) { // Put variables here 
		Info_SetValueForKey (userinfo, "rate", MAX_CLIENT_RATE_STRING);
			
		//gi.dprintf("Your rate was = %i, Server MAX is = %s\nSetting Adjusted!\n",rate, MAX_CLIENT_RATE_STRING);
		//s = Info_ValueForKey(userinfo, "rate"); // string rep
		//rate = atoi(Info_ValueForKey(userinfo, "rate")); // numeric
		//gi.dprintf("Your new rate is: %i\n",rate);
	}
	else {
		Info_SetValueForKey (userinfo, "rate", s);
		
	    //gi.dprintf("Your rate was = %i, Server MAX is = %i\nSetting Adjusted!\n",rate, MAX_CLIENT_RATE);
		//s = Info_ValueForKey(userinfo, "rate"); // string rep
		//rate = atoi(Info_ValueForKey(userinfo, "rate")); // numeric
		//gi.dprintf("Your new rate is: %i\n",rate);
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
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	    
	    // version 2.3
		// Clan Banning
        FILE    *banlist;
		char bandir[50];
        // Variable to handle the file.
        char    line[50];
        // To read the file line byline.
		// end version 2.3
	
	char	*value;
    int qq;


//ThreadZ
    
    char *rate;
    FILE* fp;
	char l_szTime[30];
	time_t now;
	struct tm *tblock;
	time(&now);
	tblock = localtime(&now);
//ThreadZ

	// Version 2.3
	// Let's clean out the temp string.
	for (qq=0; qq<50; qq++) {
		bandir[qq] = '\0';
		line[qq] = '\0';
	}
	// End version 2.3

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

// VERSION 2.3
if (namebanning == 1)  // If namebanning variable set in config file is turned on...
{

	  sprintf(bandir, "%sbanlist.txt", bandirectory);
       // Clan Banning
       if ( banlist = fopen(bandir, "r") )
       // Open theFile
       {
               while ( fgets(line, 50, banlist) ) // We can still read...
                {
                        line[strlen(line)-1] = '\0';    // chop off the trailing \n
                        
						if (matchfullnamevalue==1) {

						   if ( strcmp(ent->client->pers.netname, line )==0)  // if we found the current word in the nick...
						   {
						  	  gi.dprintf("User with name of %s refused connection due to match with %s in banlist.\n",ent->client->pers.netname,line);
							  return false;           // Refuse this client Entrance.
						   } 
						
						} else {
						
							if ( strstr(ent->client->pers.netname, line ))  // if we found the current word in the nick...
							{
							  gi.dprintf("User with name of %s refused connection due to match with %s in banlist.\n",ent->client->pers.netname,line);
							  return false;           // Refuse this client Entrance.
							}
						} // end else if (matchfullnamevalue==1) {
               			   
			   }
                fclose(banlist);                        // Give the file back to everyone.
        }
        else                                            // Can't open the file.
        {
                gi.dprintf("Can't open %s\n",bandir);
        }
}

// END VERSION 2.3


    if (game.maxclients > 1)
		gi.dprintf ("%s connected\n", ent->client->pers.netname);

	ent->client->pers.connected = true;

    if (namebanning == 1)  // If namebanning variable set in config file is turned on...
      ingamenamebanningstate=1; // set the check now for within the game banning.

	

	//ThreadZ ((RATE), Version 2.3, Logfile writing)
    value = Info_ValueForKey (userinfo, "ip");
    rate = Info_ValueForKey (userinfo, "rate");
    strcpy(l_szTime, asctime(tblock));
    l_szTime[24] = '\0';
    if ((fp = fopen(PLAYERS_LOGFILE,"a")) != NULL)
    {
		fprintf(fp,"Connect,,%s,,", l_szTime);
        fprintf(fp,"%s,,%s,,%s\n", ent->client->pers.netname, value, rate);
		fclose(fp);
    }
   //ThreadZ

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

//ThreadZ (uses PLAYERS_LOGFILE too)
	FILE* fp;
    char  *value;
	char  l_szTime[30];
	time_t now;
	struct tm *tblock;
	time(&now);
	tblock = localtime(&now);
//ThreadZ

	if (!ent->client)
		return;

	// ServObit
    if (wasbot==1)
		gi.bprintf (PRINT_HIGH, "%s tried to connect with a BOT, disconnected\n", ent->client->pers.netname);
	else
		gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);
	
	//ServObitAnnounceDisconnect(ent);
	// End ServObit

	// SCONFIG START
	sl_LogPlayerDisconnect( &gi, level, ent );	// StdLog - Mark Davies
	// SCONFIG STOP

    //ThreadZ
    value = Info_ValueForKey (ent->client->pers.userinfo, "ip");
    strcpy(l_szTime, asctime(tblock));
	l_szTime[24] = '\0';
    if ((fp = fopen(PLAYERS_LOGFILE,"a")) != NULL)
    {
		fprintf(fp,"Disconnect,,%s,,", l_szTime);
        if (wasbot==1)
		  fprintf(fp,"%s,,%s,,BOT_THROWN_OUT\n", ent->client->pers.netname, value);
		else
		  fprintf(fp,"%s,,%s\n", ent->client->pers.netname, value);
		fclose(fp);
    }
    //ThreadZ

	wasbot = 0;

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
	// sconfig 2.8
	ent->client->menu_state=1; // reset vals
	ent->client->wasingame=0; 
	// end sconfig 2.8
	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");
	//Added v3.0
	gi.configstring (NICENAME+playernum, "");
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

	// check here
	if (fraghit==0) {

	if (level.intermissiontime 	&& level.time >= level.intermissiontime + 5.0)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
					// ==
		if (level.time >= level.intermissiontime + 5.0 && client->showed == 0){ // Version 3.0 FIX
			//DeathmatchScoreboardMessage (ent, NULL); 
			endLevelshowTop10(ent);
			gi.unicast (ent, true);
			client->showed=1;
		}

		if (level.time > level.intermissiontime + 10.0 && (ucmd->buttons & BUTTON_ANY)){
			level.exitintermission=true;
			//client->showed=0; // Still running think... Removed other
		}
		return;
	}
	
	
	if (level.intermissiontime 	&& level.time < level.intermissiontime + 5.0)
	{
		// Fix hud if necessary
		if (timehud==1)
			ent->client->ps.stats[STAT_TIMELEFT] = atoi(GlobalTimeLimit);
		
		client->ps.pmove.pm_type = PM_FREEZE;
				
		
		return;
	}
	
	} // END IF fraghit
	else
	{

	if (level.intermissiontime)
	{
		// Fix hud if necessary
		if (timehud==1)
			ent->client->ps.stats[STAT_TIMELEFT] = atoi(GlobalTimeLimit);
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}


	} // END ELSE

	// Sconfig 2.6 (grapple)
	// Expert: Check if player needs to be pulled by hook
	if (client->on_hook == true)
		Pull_Grapple(ent);

	pm_passent = ent;

	// SCONFIG START
	// ZOID
	if (ent->client->chase_target) {
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
		return;
	}

	// ZOID
	// SCONFIG STOP

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
		gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
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

	
	
// Version 2.3 (Cloak)
if (cloaking == 1 && ent->movetype != MOVETYPE_NOCLIP) {
	
 	if (cloakgrapple==1) { //added

	// handle cloaking ability
         if (ent->client->cloakable)
         {
		  
          if (ucmd->forwardmove != 0 || ucmd->sidemove != 0)
          {
           ent->svflags &= ~SVF_NOCLIENT;
           ent->client->cloaking = false;
          }
         else
         {
         if (ent->svflags & SVF_NOCLIENT)
         {
           if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] >= CLOAK_AMMO)
		   {
             ent->client->cloakdrain ++;
             // Can do whatever else here during cloak, play sound..., low volume
       		 gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/comp_up.wav"), .06, ATTN_STATIC, 0);     
		
		     if (ent->client->cloakdrain == CLOAK_DRAIN)
			 {
               ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= CLOAK_AMMO;
               ent->client->cloakdrain = 0;
			 }
		   }
         else
         {
		   // When cells run out...
	      ent->svflags &= ~SVF_NOCLIENT;
          ent->client->cloaking = false;
		  ent->client->cloakable=0; // disable cloaking
		  ent->client->hudcloak_state == false;
		  gi.centerprintf (ent, "No more cells, motion cloaking disabled!\n");
         }
        }
         else
        {
        if (ent->client->cloaking)
        {
        if (level.time > ent->client->cloaktime)
         {
         ent->svflags |= SVF_NOCLIENT;
         ent->client->cloakdrain = 0;
       }
		}
      else
      {
        ent->client->cloaktime = level.time + CLOAK_ACTIVATE_TIME;
        ent->client->cloaking = true;
		
      }
    }
  }
}



	} // added

	else { // added
	

	// handle cloaking ability
         if (ent->client->cloakable)
         {
		  
          if (ucmd->forwardmove != 0 || ucmd->sidemove != 0 || Is_Grappling(ent->client))
          {
           ent->svflags &= ~SVF_NOCLIENT;
           ent->client->cloaking = false;
          }
         else
         {
         if (ent->svflags & SVF_NOCLIENT)
         {
           if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] >= CLOAK_AMMO)
		   {
             ent->client->cloakdrain ++;
             // Can do whatever else here during cloak, play sound..., low volume
       		 gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/comp_up.wav"), .06, ATTN_STATIC, 0);     
		
		     if (ent->client->cloakdrain == CLOAK_DRAIN)
			 {
               ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= CLOAK_AMMO;
               ent->client->cloakdrain = 0;
			 }
		   }
         else
         {
		   // When cells run out...
	      ent->svflags &= ~SVF_NOCLIENT;
          ent->client->cloaking = false;
		  ent->client->cloakable=0; // disable cloaking
		  ent->client->hudcloak_state == false;
		  gi.centerprintf (ent, "No more cells, motion cloaking disabled!\n");
         }
        }
         else
        {
        if (ent->client->cloaking)
        {
        if (level.time > ent->client->cloaktime)
         {
         ent->svflags |= SVF_NOCLIENT;
         ent->client->cloakdrain = 0;
       }
		}
      else
      {
        ent->client->cloaktime = level.time + CLOAK_ACTIVATE_TIME;
        ent->client->cloaking = true;
		
      }
    }
  }
}

} //added
} // end if cloaking == 1
else if (ent->movetype == MOVETYPE_NOCLIP) {
		  ent->client->cloakable=0; // disable cloaking
		  ent->client->hudcloak_state == false; 
		  ent->client->cloaking = false;
		  ent->client->ps.stats[STAT_CLOAKCELLS] = 0;
}
     
// End Version 2.3 (Cloak)


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


	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTON_ATTACK
		// SCONFIG START
		// ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
		// ZOID
		// SCONFIG STOP
		)
	{
		if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}
	
	
	// SCONFIG START
	// ZOID
	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}
	// ZOID
	// SCONFIG STOP

	// Sconfig 2.6 (grapple)
	// Expert Hook : Grapple command code
	// Check to see if player pressing the "use" key
	if ((allowgrapple == 1) && ent->client->buttons & BUTTON_USE && !ent->deadflag 
		&& client->hook_frame <= level.framenum)
	{
		Throw_Grapple (ent);
	}
  
	// Sconfig 2.6 (grapple)
	// Expert: Check to see if the grapple key was released
	if ((allowgrapple == 1) && Ended_Grappling (client) && 
		!ent->deadflag && client->hook)
	{
		Release_Grapple (client->hook);
	}


}



// -- ZBOT DETECTION SCHEMECODE STARTS -- Version 2.6z//
      void ZBOT_StartScan(edict_t* ent)
      {
         stuffcmd2(ent, "!zbot\n");
        stuffcmd2(ent, "#zbot\n");
        ent->client->resp.std_resp = 0;
        ent->client->resp.bot_end = level.time + 4;
         return;
      }
// -- NEW CODE ENDS -- //



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


// -- ZBOT DETECTION SCHEME NEW CODE STARTS -- Version 2.5z //

                             if (ent->client->resp.bot_start>0 && ent->client->resp.bot_start <= level.time) { 
								 ent->client->resp.bot_start = 0;
                                     ZBOT_StartScan(ent);
                             }
                             if (ent->client->resp.bot_end>0 && ent->client->resp.bot_end <= level.time) { 
								 ent->client->resp.bot_end = 0;
                                     if (!ent->client->resp.std_resp) // could be lagged.
                                             ZBOT_StartScan(ent);
                                     else
                                     {
										 if (--ent->client->resp.bot_retry <= 0)  {
		       								 gi.cprintf (ent, PRINT_HIGH, "NO BOTS ALLOWED...\n");
											 wasbot=1;
											 stuffcmd2(ent, "disconnect\n"); // kick out zbots. 

										 }
											 else ZBOT_StartScan(ent); 
							 } 
							 } // NEW CODE ENDS // 


	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk
		// SCONFIG START
		//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
		//ZOID
		// SCONFIG STOP
		)
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
