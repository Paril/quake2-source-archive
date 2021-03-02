#include "g_local.h"
#include "m_player.h"
///Q2 Camera Begin
#include "camclient.h"
///Q2 Camera End

//K2:Begin
#include "stdlog.h"	//	StdLog - Mark Davies
#include "gslog.h"	//	StdLog - Mark Davies
//K2:End

//ERASER
#include "p_trail.h"

#include "bot_procs.h"

//#include <sys/timeb.h>
#include <time.h>

#define	OPTIMIZE_INTERVAL	0.1

int	num_clients	= 0;
//ERASER

void ClientUserinfoChanged (edict_t *ent, char *userinfo);

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

void SP_misc_teleporter_dest (edict_t *ent);

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

//ERASER
edict_t *dm_spots[64];
int		num_dm_spots;

void SP_info_player_deathmatch(edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	SP_misc_teleporter_dest (self);

	//ERASER
	dm_spots[num_dm_spots++] = self;
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

	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	return false;
}

qboolean IsNeutral (edict_t *ent)
{
	char		*info;

	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
		return true;
	return false;
}



void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;
	else if (deathmatch->value && teamplay->value && attacker->client && SameTeam(self, attacker))
	{
		meansOfDeath |= MOD_FRIENDLY_FIRE;
	}

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
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				break;
			//K2:Begin
			case MOD_FLAME:
				message = "shouldn't play with matches";
				break;
			case MOD_HOMING_BLASTER:
				message = "homed the wrong target";
				break;
			case MOD_HOMING_HYPERBLASTER:
				message = "homed the wrong target";
				break;
			case MOD_HOMING_SPLASH:
				message = "homed the wrong target";
				break;
			case MOD_HOMING_ROCKET:
				message = "homed the wrong target";
				break;
			case MOD_FLASH:
				if (IsNeutral(self))
					message = "blinded itself to death";
				else if (IsFemale(self))
					message = "blinded herself to death";
				else
					message = "blinded himself to death";	
				break;
			case MOD_FREEZE:
				if (IsNeutral(self))
					message = "froze itself to death";
				else if (IsFemale(self))
					message = "froze herself to death";
				else
					message = "froze himself to death";	
				break;
			
			case MOD_DRUNK_SPLASH:
				if (IsNeutral(self))
					message = "got drunk on its own drunk rocket";
				else if (IsFemale(self))
					message = "got drunk on her own drunk rocket";
				else
					message = "got drunk on his own drunk rocket";	
				break;
			case MOD_SUICIDE:
				message = "made it stop";
				break;
			//K2:End
			
			default:
				if (IsNeutral(self))
					message = "killed itself";
				if (IsFemale(self))
					message = "killed herself";
				else
					message = "killed himself";
				break;
			}
		}
		if (message)
		{
			my_bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
			{
				self->client->resp.score--;
				if (self->client->team)
					self->client->team->score--;
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
//ZOID
			case MOD_GRAPPLE:
				message = "was caught by";
				message2 = "'s grapple";
				break;
//ZOID
			//K2:Begin
			case MOD_FLAME:
				message = "was torched by";
				break;
			case MOD_GIBGUN:
				message = "was gibbed to death by";
				break;
			case MOD_HOMING_BLASTER:
				message = "got homed in on by";
				break;
			case MOD_HOMING_HYPERBLASTER:
				message = "was overwhelmed by";
				message2 = "'s homing hyperblaster";
				break;
			case MOD_HOMING_SPLASH:
				message = "tried to hide from";
				message2 = "'s homing rocket";
				break;
			case MOD_HOMING_ROCKET:
				message = "couldn't avoid";
				message2 = "'s homing rocket";
				break;
			case MOD_FLASH:
				message = "was blinded to death by";
				break;
			case MOD_FREEZE:
				message = "was frozen to death by";
				break;
			case MOD_DRUNK:
				message = "got too drunk on";
				message2 = "'s drunk rockets";
				break;
			case MOD_DRUNK_SPLASH:
				message = "couldn't outrun";
				message2 = "'s drunk rockets";
				break;
			case MOD_REVERSE_TELEFRAG:
				message = "got reverse telefragged by";
				break;
			//K2:End
			}

			if (message)
			{
				my_bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if (deathmatch->value)
				{
					if (ff)
					{
						attacker->client->resp.score--;
						if (attacker->client->team)
							attacker->client->team->score--;
					}
					else
					{
						attacker->client->resp.score++;
						if (attacker->client->team)
							attacker->client->team->score++;
						//K2:Begin
						
						//Bonus frags
						//If self doesn't have a key, don't bother
						if ( self->client->key )
							 K2_BonusFrags(self,inflictor,attacker);
												
						//K2:End
					}
				}
				return;
			}
		}
	}

	my_bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (deathmatch->value)
	{
		self->client->resp.score--;
		if (self->client->team)
			self->client->team->score--;
	}
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

void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	VectorClear (self->avelocity);

	//K2:Begin
	//Toss the key if he/she had one and remove shell effect
	if (self->client->key)
		K2_SpawnKey(self, self->client->key, 1);

	//Hook - Release hook if needed
	if (self->client->hook)
		Release_Grapple(self->client->hook);
	
	//Flash Grenade - Remove blend effects
	self->client->ps.blend[0] = self->client->ps.blend[1] = self->client->ps.blend[2] = 0;
	//K2:End

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
//ZOID
	self->s.modelindex3 = 0;	// remove linked ctf flag
//ZOID

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
		//K2:Begin
		
		if(!attacker->bot_client) //Only log if attacker is a real player
		{
			//If self is a bot and there is no botfraglogging, break;
			if(self->bot_client && !botfraglogging->value)
				goto next;

			sl_WriteStdLogDeath( &gi, level, self, inflictor, attacker);	// StdLog - Mark Davies

			if(qwfraglog->value)
					WriteQWLogDeath(self,inflictor,attacker);
			
		}
		next:
		//K2:End

		
//ZOID
		CTFFragBonuses(self, inflictor, attacker);
//ZOID
		TossClientWeapon (self);
//ZOID
		CTFPlayerResetGrapple(self);
		CTFDeadDropFlag(self);
		CTFDeadDropTech(self);
//ZOID
		
		// clear inventory
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < game.num_items; n++)
		{
			if (coop->value && itemlist[n].flags & IT_KEY)
				self->client->resp.coop_respawn.inventory[n] = self->client->pers.inventory[n];
			self->client->pers.inventory[n] = 0;
		}

		if (deathmatch->value)
			Cmd_Help_f (self);		// show scores
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;

	if (self->health < -40)
	{	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead (self, damage);

//ZOID
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = 0;
//ZOID
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

	//K2:Begin - Init Client Vars
	K2_InitClientVars(self);
	K2_ResetClientKeyVars(self);
	K2_ClearPrevOwner(self);
	//K2:End

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

	memset (&client->pers, 0, sizeof(client->pers));

	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	client->pers.weapon = item;
//ZOID
	client->pers.lastweapon = item;
//ZOID

	item = FindItem("Grapple");

	//K2:Grapple change
	if(ctf->value)
	//if (grapple->value || ctf->value)
	{
//ZOID
		client->pers.inventory[ITEM_INDEX(item)] = 1;
//ZOID
	}
	else
	{
		client->pers.inventory[ITEM_INDEX(item)] = 0;
	}

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 50;

	//K2:Begin - Clear these vars
	client->key = 0;
	client->k2_regeneration_time = 0;
	client->k2_key_framenum = 0;
	//K2:End

	client->pers.connected = true;
}

//ERASER
qboolean is_bot=false;

void InitClientResp (gclient_t *client)
{
//ZOID
	int ctf_team = client->resp.ctf_team;
//ZOID

	//K2:begin
	qboolean inServer = client->resp.inServer;
	//K2:End

	memset (&client->resp, 0, sizeof(client->resp));
//ZOID
	client->resp.ctf_team = ctf_team;
//ZOID

	//K2:begin
	client->resp.inServer = inServer;
	//K2:End

	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;

//ZOID
	if (ctf->value && (client->resp.ctf_team < CTF_TEAM1))
		CTFAssignTeam(client, is_bot);
//ZOID
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

	for (n = 0; n < num_players; n++)
	{
//		player = &g_edicts[n];
		player = players[n];

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
	int		count = 0, spot_num;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

//ERASER
	//	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	while (count < num_dm_spots)
	{
		spot = dm_spots[count];

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
	spot_num = 0;
	do
	{
		spot = dm_spots[spot_num++];
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

//ERASER - many changes
edict_t *SelectFarthestDeathmatchSpawnPoint (void)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;
	int		spot_num;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	spot_num = 0;
	while (spot_num < num_dm_spots)
	{
		spot = dm_spots[spot_num++];

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
	spot = dm_spots[0];

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
//ZOID
		if (ctf->value)
			spot = SelectCTFSpawnPoint(ent);
		else
//ZOID
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

void Body_droptofloor(edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15,-15,-24);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	ent->s.origin[2] += 32;

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);

	VectorCopy(tr.endpos, ent->s.origin);

	gi.linkentity(ent);

	if (tr.ent)
		ent->nextthink = level.time + 0.1;
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

//	body->nextthink = level.time + 0.2;
//	body->think = Body_droptofloor;
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

		//K2:Begin - Hook - Disallow grappling at respawn (so that pressing +use doesn't cause a hook)
		self->client->hook_frame = level.framenum + 1;
		//K2:End


		return;
	}

	// restart the entire server
	gi.AddCommandString ("menu_loadgame\n");
}

/* 
 * only called when pers.spectator changes
 * note that resp.spectator should be the opposite of pers.spectator here
 */
void spectator_respawn (edict_t *ent)
{
	int i, numspec;

	// if the user wants to become a spectator, make sure he doesn't
	// exceed max_spectators

	if (ent->client->pers.spectator) {
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "spectator");
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
	}

	// clear client on respawn
	ent->client->resp.score = ent->client->pers.score = 0;

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

	if (ent->client->pers.spectator) 
		gi.bprintf (PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname);
	else
		gi.bprintf (PRINT_HIGH, "%s joined the game\n", ent->client->pers.netname);
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
	client_persistant_t	saved;
	client_respawn_t	resp;
	
	//ERASER
	bot_team_t	*team;
	lag_trail_t	*lag_trail;
	lag_trail_t	*lag_angles;
	float	latency;

	if (!the_client && !ent->bot_client && (num_players <= 1))
		the_client = ent;

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
		//K2:Begin
		//   Use K2_InitClientPersistant if swaat is true
		if(swaat->value)
			K2_InitClientPersistant (client);
		else
		//K2:End
			InitClientPersistant (client);
		
		ClientUserinfoChanged (ent, userinfo);
	}
	else if (coop->value)
	{
		//		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
//		for (n = 0; n < game.num_items; n++)
//		{
//			if (itemlist[n].flags & IT_KEY)
//				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
//		}
		resp.coop_respawn.game_helpchanged = client->pers.game_helpchanged;
		resp.coop_respawn.helpchanged = client->pers.helpchanged;
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
	//ERASER
	team = client->team;
	lag_trail = client->lag_trail;
	lag_angles = client->lag_angles;
	latency = client->latency;

	memset (client, 0, sizeof(*client));

	client->pers = saved;
	client->resp = resp;
	client->team = team;
	client->lag_trail = lag_trail;
	client->lag_angles = lag_angles;
	client->latency = latency;
	if (client->pers.health <= 0)
	{
		//K2:Begin
		//   Use K2_InitClientPersistant if swaat is true
		if(swaat->value)
			K2_InitClientPersistant (client);
		else
		//K2:End
			InitClientPersistant (client);
	}

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	// clear entity values
	ent->groundentity = NULL;
	//ERASER
	if (!ent->bot_client)
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

	//ERASER
	if (!ent->bot_client)
	{
		ent->pain = player_pain;
		ent->die = player_die;
	}

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

	//ERASER
	if (!ent->bot_client)
	{
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

		client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);
	}

	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->s.modelindex2 = 255;		// custom gun model
	// sknum is player num and weapon number
	// weapon number will be added in changeweapon
	ent->s.skinnum = ent - g_edicts - 1;

	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;			// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

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
		gi.linkentity (ent);
		return;
	} else
		client->resp.spectator = false;

//ZOID
	if(ctf->value)
	{
		if (CTFStartClient(ent))
			return;
	}
//ZOID
	//K2:Begin
	else if (!ent->bot_client)
	{
		if(K2_StartClient(ent))
			return;
	}
	//K2:End

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

	if (!ent->map)
		ent->map = G_CopyString(ent->client->pers.netname);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	//ERASER
	if (!ent->bot_client)
		ChangeWeapon (ent);

	ent->last_max_z = 32;
	ent->jump_ent = NULL;
	ent->duck_ent = NULL;
	ent->last_trail_dropped = NULL;

	//ERASER
	if (!ent->bot_client)
	{
		if (ent->client->lag_trail)
		{
			// init the lag_trail
			for (i=0; i<10; i++)
			{
				VectorCopy(ent->s.origin, (*ent->client->lag_trail)[i]);
				VectorCopy(ent->client->v_angle, (*ent->client->lag_angles)[i]);
			}
		}

		if (bot_calc_nodes)
    {
    	if (!(trail[0]))
			{
  			trail[0]=G_Spawn();
			}
			
      if ((bot_calc_nodes->value) && (trail[0]->timestamp))
			{	// find a trail node to start checking from
				if ((i = ClosestNodeToEnt(ent, false, true)) > -1)
				{
					ent->last_trail_dropped = trail[i];
				}
			}
  	}
	}

	//K2:Begin Spawn protect
	ent->client->protecttime = level.time + protecttime->value;
	
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

	if (ctf->value)
		ent->client->team = NULL;

	InitClientResp (ent->client);

	//K2:Begin
	K2_InitClientVars(ent);
	K2_ResetClientKeyVars(ent);
	//K2:End

	// locate ent at a spawn point
	PutClientInServer (ent);

	//K2:Zbot Scan
	ent->client->resp.bot_start = level.time + 5;
    ent->client->resp.bot_retry = 3;
	//K2

    ///Q2 Camera Begin
    EntityListAdd(ent);
    ///Q2 Camera End

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	//my_bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

	//K2:Begin
	if(ctf->value)
		// StdLog - Mark Davies
		sl_LogPlayerConnect( &gi,
								ent->client->pers.netname,
					            CTFTeamName(ent->client->resp.ctf_team),
						        level.time );        
	else
		sl_WriteStdLogPlayerEntered( &gi, level, ent );	// StdLog - Mark Davies

	//K2:End
	
	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}

//K2:Zbot Scan
void ZBOT_StartScan(edict_t* ent)
{
   stuffcmd(ent, "!zbot\n");
   stuffcmd(ent, "#zbot\n");
   ent->client->resp.std_resp = 0;
   ent->client->resp.bot_end = level.time + 4;
   return;
}
//K2

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

	players[num_players++] = ent;
	num_clients++;

	//K2:Begin - No grapple in Dm for Bots
	if (!ctf->value)
	{	// make sure grapple is removed from bots if CTF has been disabled
		if(ent->bot_client)
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))] = 0;
	}
	//K2:End

	if (deathmatch->value)
	{
		//K2:begin
		//Stuff commands
		//Get Client Version
		//stuffcmd(ent,"_version $version\n");
		//Hook
		stuffcmd(ent,"alias +hook +use; alias -hook -use;\n");
		//Feign
		stuffcmd(ent,"alias +feign cmd feign; alias -feign cmd feign;\n");
		//Take Key
		stuffcmd(ent,"alias take_key cmd take\n");
			
		//if (bot_show_connect_info->value)
		//	safe_centerprintf(ent, "\n\n=====================================\nThe Eraser Bot v%1.3f\nby Ryan Feltrin (aka Ridah)\n\nRead the readme.txt file\nlocated in the Eraser directory!\n\nVisit http://impact.frag.com/\nfor Eraser news\n\n-------------------------------------\n", ERASER_VERSION, maxclients->value);

		//if (teamplay->value && !ctf->value)
		//	gi.cprintf(ent, PRINT_HIGH, "\n\n=====================================\nServer has enabled TEAMPLAY!\n\nType: \"cmd teams\" to see the list of teams\nType: \"cmd join <teamname>\" to join a team\n\n");
		//K2:End

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

			my_bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
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

	//K2:Begin
	// start - mdavies
    // Has the player got a name

		if( strlen(ent->client->pers.netname) )
		{
			// has the name changed
			if( strcmp( ent->client->pers.netname, s ) )
			{
				// log player rename
				sl_LogPlayerRename( &gi,
					                ent->client->pers.netname,
						            s,
							        level.time );
			}
		}
    // end - mdavies
	//K2:End

	// set spectator
	s = Info_ValueForKey (userinfo, "spectator");
	// spectators are only supported in deathmatch
	if (deathmatch->value && *s && strcmp(s, "0"))
		ent->client->pers.spectator = true;
	else
		ent->client->pers.spectator = false;

	
	if (!ent->client->team || (strlen(ent->client->team->default_skin) == 0))
	{
		// set skin
		s = Info_ValueForKey (userinfo, "skin");

		playernum = ent-g_edicts-1;

		// combine name and skin into a configstring
//ZOID
		if (ctf->value)
			CTFAssignSkin(ent, s);
		else
//ZOID
			gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );
	}
	else	// only update the name
	{
		s = G_CopyString(ent->client->team->default_skin);

		playernum = ent-g_edicts-1;

		// combine name and skin into a configstring
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

		gi.TagFree(s);
	}

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
	char	*value;

	// check for a password
	if (!ent->bot_client)
	{
		// check to see if they are on the banned IP list
		value = Info_ValueForKey (userinfo, "ip");
		if (SV_FilterPacket(value))
		{
			Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
			return false;
		}

		// check for a spectator
		value = Info_ValueForKey (userinfo, "spectator");
		if (deathmatch->value && *value && strcmp(value, "0"))
		{
			int i, numspec;

			if (*spectator_password->string && 
				strcmp(spectator_password->string, "none") && 
				strcmp(spectator_password->string, value))
			{
				Info_SetValueForKey(userinfo, "rejmsg", "Spectator password required or incorrect.");
				return false;
			}

			// count spectators
			for (i = numspec = 0; i < maxclients->value; i++)
			{
				if (g_edicts[i+1].inuse && g_edicts[i+1].client->pers.spectator)
					numspec++;
			}

			if (numspec >= maxspectators->value)
			{
				Info_SetValueForKey(userinfo, "rejmsg", "Server spectator limit is full.");
				return false;
			}
		
		}
		else
		{
			// check for a password
			value = Info_ValueForKey (userinfo, "password");
			if (*password->string && strcmp(password->string, "none") && 
				strcmp(password->string, value))
			{
				Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
				return false;
			}
		}

		
		//K2:Player ip logging
		if (connectlogging->value)
			K2_LogPlayerIP(userinfo);
	
	}

	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
//ZOID -- force team join
		ent->client->resp.ctf_team = -1;
//ZOID
		//ERASER
		is_bot = ent->bot_client;			// make sure bot's join a team
		InitClientResp (ent->client);
		is_bot = false;

		if (!game.autosaved || !ent->client->pers.weapon)
			//K2:Begin
			if (swaat->value) {
				K2_InitClientPersistant(ent->client);	
			} else
			//K2:end
				InitClientPersistant (ent->client);
	}

	// do real client specific stuff
	//ERASER
	if (!ent->bot_client)
	{
		int i;

		ent->client->team = NULL;

		ent->client->lag_trail = gi.TagMalloc(sizeof(lag_trail_t), TAG_GAME);
		ent->client->lag_angles = gi.TagMalloc(sizeof(lag_trail_t), TAG_GAME);

		// init the lag_trail
		for (i=0; i<10; i++)
		{
			VectorCopy(ent->s.origin, (*ent->client->lag_trail)[i]);
			VectorCopy(ent->client->v_angle, (*ent->client->lag_angles)[i]);
		}
	}

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf ("%s connected\n", ent->client->pers.netname);

	ent->svflags = 0; // make sure we start with known default
	ent->client->pers.connected = true;

	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
//ERASER
void botRemovePlayer(edict_t *self);

void ClientDisconnect (edict_t *ent)
{
	int		playernum;
	//K2
	int i;
	edict_t *other;
	//K2

	if (!ent->client)
		return;

    ///Q2 Camera Begin
    EntityListRemove(ent);
    ///Q2 Camera End

	//ERASER
	
	//K2: If player was in cammode, he was already removed
	if(!ent->client->bIsCamera)
		botRemovePlayer(ent);

	
	//K2:Begin
	//my_bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);
	my_bprintf (PRINT_HIGH, "%s wimped out with %i lousy frags\n",
		        ent->client->pers.netname,
				ent->client->resp.score);

	// update chase cam if being followed
	for (i = 0; i <= num_players; i++) {
		
		other = players[i];
	
		if(!other || other->bot_client)
			continue;

		if (other->inuse && other->client->chase_target == ent)
		{
			other->client->chase_target->inuse = false;
			UpdateChaseCam(other);
		}
	}
	//K2:End

//ZOID
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);
//ZOID

	//K2:Begin
	//Toss the key if he/she had one
	if (ent->client->key)
		K2_SpawnKey(ent, ent->client->key, 1);
	K2_ResetClientKeyVars(ent);
	ent->client->resp.inServer = false;

	sl_LogPlayerDisconnect( &gi, level, ent );	// StdLog - Mark Davies	
	//K2:End
	
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
//ERASER
int	last_bot=0;
#define		BOT_THINK_TIME	0.03	// never do bot thinks for more than this time
//ERASER

void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j;
	pmove_t	pm;
	int count=0, start;
	//ERASER
	clock_t	start_time, now;

	if (paused)
	{
		safe_centerprintf(ent, "PAUSED\n\n(type \"botpause\" to resume)");
		ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
		return;
	}

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

    ///Q2 Camera Begin
    if (ent->client->bIsCamera)
    {
        CameraThink(ent,ucmd);
        return;
    }
    ///Q2 Camera End

	//ERASER
	if (ent->bot_client)
		return;

	//K2:Begin - Hook - Check if player needs to be pulled by hook
	if (client->on_hook == true)
		Pull_Grapple(ent);
	//K2:End

	

//gi.dprintf("f: %i, s: %i, u: %i\n", ucmd->forwardmove, ucmd->sidemove, ucmd->upmove);
//gi.dprintf("%i, %i\n", ucmd->buttons, client->ps.pmove.pm_flags);

	pm_passent = ent;

//ZOID
	if (ent->client->chase_target) {
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

		//K2:Begin improved chasecam stuff
		if(ucmd->forwardmove < 0)
		{
			ent->speed++;
			if(ent->speed < 0)
				ent->speed = 0;
		}
		else if(ucmd->forwardmove > 0)
		{
			ent->speed--;
		}
		//K2: End
	return;
	}
//ZOID

	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	//K2:Begin - Hook - If the hook is in use, just ignore gravity
	if (client->on_hook)
		client->ps.pmove.gravity = 0;
	else
	//K2:End
		client->ps.pmove.gravity = sv_gravity->value;

	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (ent->s.modelindex != 255)
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag || ent->client->is_feigning) //K2:Begin - Feign - End)
		client->ps.pmove.pm_type = PM_DEAD;
	else if (ent->client->freezeTime > level.framenum)
		client->ps.pmove.pm_type = PM_DEAD;
	else
		client->ps.pmove.pm_type = PM_NORMAL;
	
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
	else
		pm.snapinitial=false;


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
		//K2:Begin - Stealth - No jump sound if player has Stealth Key
		if(!K2_IsStealth(ent))	
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
		}
		//K2:End
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


	
//ZOID
	if (client->ctf_grapple)
		CTFGrapplePull(client->ctf_grapple);
//ZOID

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

	//ERASER
	if ((client->latency > 0) && !(client->buttons & BUTTON_ATTACK) && (ucmd->buttons & BUTTON_ATTACK))
	{
		if ((level.time - client->firing_delay) > 0.1)
			client->firing_delay = level.time + (client->latency/1000);

		if (client->firing_delay > level.time)		// turn it off
		{
			ucmd->buttons &= ~BUTTON_ATTACK;
			if (ucmd->buttons >= BUTTON_ANY)
				ucmd->buttons -= BUTTON_ANY;
		}
	}

	//ERASER
	if ((client->latency > 0) && !(ucmd->buttons & BUTTON_ATTACK) && (client->firing_delay > (level.time - 0.1)) && (client->firing_delay <= level.time))
	{	// remember the button after it was released when simulating lag
		ucmd->buttons |= BUTTON_ATTACK;
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

			if (client->chase_target) {
				client->chase_target = NULL;
				client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			} else
				GetChaseTarget(ent);

		} else if (!client->weapon_thunk && client->resp.inServer) { //K2: inServer addition
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

	if (client->resp.spectator) {
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
	}

	//K2: Changed to use playerlist
	// update chase cam if being followed
	//for (i = 1; i <= maxclients->value; i++) {
	//	other = g_edicts + i;
	for (i = 0; i <= num_players; i++) {
		
		other = players[i];
	
		if (!other || other->bot_client)
			continue;

		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}
	//K2

//ZOID
//regen tech
	CTFApplyRegeneration(ent);
//ZOID

	//K2:Begin
	K2_Regeneration(ent);
	//Check for exired key
	K2_KeyExpiredCheck(ent);
	K2_botBFKInformDanger(ent);
	//Fire/Burn effects 
	//If player gets into water, no more burn
	if (ent->waterlevel >= 2)
		ent->client->burntime = 0;
	//K2:End


	//ERASER
	// =========================================================
	// check for a bots to think
	if ((num_players > 2) && (num_clients == 1))
	{
		int	save=-1, loop_count=0;
		vec3_t	org;

		start = last_bot;
		start_time = clock();
		now = start_time;

		// go to the next bot
		last_bot++;
		if (last_bot >= num_players)
			last_bot = 0;

		VectorCopy(ent->s.origin, org);
		org[2] += ent->viewheight;

		while (	(count < 5) && (((double) (now - start_time)) < 50))
		{
			// don't do client's, or bot's in this client's view
			while	(	(players[last_bot]->last_think_time == level.time)
					 ||	(!players[last_bot]->bot_client)
					 ||	(	(gi.inPVS(org, players[last_bot]->s.origin))
						 &&	(	(players[last_bot]->nextthink >= level.time)
							 ||	(players[last_bot]->nextthink = level.time + 0.1)
							)
						) // this makes the bot think by itself (smooth movement)
					)
			{
				last_bot++;
				if (last_bot == num_players)
					last_bot = 0;
				loop_count++;

				if (players[last_bot]->bot_client && (last_bot == start))	// we've done all bots
					goto done_all_bots;

				if (loop_count > num_players)
					goto done_all_bots;
			}

			if (start == 0)
				start = last_bot;

			if ((level.time - players[last_bot]->last_think_time) >= 0.1)
			{
				players[last_bot]->think(players[last_bot]);
				players[last_bot]->nextthink = -1;			// don't think by themselves
				count++;

				save = last_bot;
			}

			if (last_bot == start)		// we've done all bots
				break;

			last_bot++;
			if (last_bot == num_players)
				last_bot = 0;
			loop_count++;
			if (loop_count > num_players)
				goto done_all_bots;

			now = clock();

		}

	}

done_all_bots:
/*
	// check for firing between server frames
	for (i=0; i<num_players; i++)
	{
		if (	(players[i]->bot_client)
			&&	(players[i]->dmg) 
//			&&	(	(timebuffer.millitm < players[i]->lastattack_time.millitm)
//				 ||	((timebuffer.millitm - players[i]->lastattack_time.millitm) >= 50)
//				)
			)
		{
			bot_FireWeapon(players[i]);
			players[i]->dmg = 0;
		}
	}
*/
	if (!dedicated->value)
		OptimizeRouteCache();


	// =========================================================
	//ERASER

	//K2:Begin - Hook
	//Hook - Grapple command code
	// Check to see if player just pressed the grapple key
	if (Started_Grappling (client) && !ent->deadflag
		&& client->hook_frame <= level.framenum
		&& !client->is_feigning && (client->freezeTime < level.framenum)
		&& client->resp.inServer && !client->resp.spectator) 
	{
		Throw_Grapple (ent);
	}
	
	//Check to see if the grapple key was released
	if (Ended_Grappling (client) && !ent->deadflag && client->hook)
	{
		Release_Grapple (client->hook);
	}

	//K2:End
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/

void trigger_elevator_use (edict_t *self, edict_t *other, edict_t *activator);
void Use_Plat (edict_t *ent, edict_t *other, edict_t *activator);
void CTFFlagThink(edict_t *ent);

void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			i;
	int			buttonMask;

	if (ent->bot_client)
		return;

	if (level.intermissiontime)
		return;

    ///Q2 Camera Begin
     if (ent->client->bIsCamera)
    {
        return;
    }
    ///Q2 Camera End

	client = ent->client;

	if (deathmatch->value &&
		client->pers.spectator != client->resp.spectator &&
		(level.time - client->respawn_time) >= 5) {
		spectator_respawn(ent);
		return;
	}


	//K2:Zbot Scan
	if(nozbots->value && !ent->bot_client)
	{
		if (ent->client->resp.bot_start>0 && ent->client->resp.bot_start <= level.time)
		{
			ent->client->resp.bot_start = 0;
		 ZBOT_StartScan(ent);
		}
	
		if (ent->client->resp.bot_end>0 && ent->client->resp.bot_end <= level.time)
		{
			ent->client->resp.bot_end = 0;
			if (!ent->client->resp.std_resp) // could be lagged.
				ZBOT_StartScan(ent);
			else
		    {
				if (--ent->client->resp.bot_retry <= 0)
				{
					safe_bprintf(PRINT_HIGH,"Zbot player--->%s has been disconnected\n",ent->client->pers.netname);
					safe_cprintf(ent,PRINT_HIGH,"No ZBots allowed!\n");
					stuffcmd(ent, "disconnect\n"); // kick out zbots.
				}
				else
					ZBOT_StartScan(ent);
			} 
		}
	}

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk
//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
//ZOID
		)
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;

	//ERASER
	if (client->latency > 0)
	{
		// -- save the location in the lag_trail
		//    filter the trails down one position
		for (i=9; i>0; i--)
		{
			VectorCopy((*client->lag_trail)[i-1], (*client->lag_trail)[i]);
			VectorCopy((*client->lag_angles)[i-1], (*client->lag_angles)[i]);
		}

		VectorCopy(ent->s.origin, (*client->lag_trail)[0]);
		VectorCopy(ent->client->v_angle, (*client->lag_angles)[0]);
		// -- done.
	}

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

	client->latched_buttons = 0;

//	if (gi.pointcontents(ent->s.origin) & CONTENTS_LADDER)

	//ERASER
	// check for new node(s)
	if (bot_calc_nodes->value)
	{
		CheckMoveForNodes(ent);
	}

//if (ent->groundentity)
//gi.dprintf("%s\n", ent->groundentity->classname);

	if (ent->flags & FL_SHOWPATH)
	{
		// set the target location
		if (!ent->goalentity)	// spawn it
		{
			ent->goalentity = G_Spawn();
			ent->goalentity->classname = "player goal";
			VectorCopy(ent->mins, ent->goalentity->mins);
			VectorCopy(ent->maxs, ent->goalentity->maxs);

			VectorCopy(ent->s.origin, ent->goalentity->s.origin);
			ent->goalentity->s.modelindex = 255;
			gi.linkentity(ent->goalentity);
		}

		if (ent->client->buttons & BUTTON_ATTACK)
		{
			VectorCopy(ent->s.origin, ent->goalentity->s.origin);
			ent->goalentity->s.modelindex = 255;
			gi.linkentity(ent->goalentity);
		}

		Debug_ShowPathToGoal(ent, ent->goalentity);
	}

//gi.dprintf("%i\n", ent->waterlevel);

	// HACK, send bots to us if we have the flag, and also summon some helper bots
	if (ctf->value && CarryingFlag(ent))// && 
//		(!ent->movetarget || !ent->movetarget->item || (ent->movetarget->item->pickup != CTFPickup_Flag)))
	{
		edict_t *flag, *enemy_flag, *plyr, *self;
		int	i=0, count=0, ideal;
		static float	last_checkhelp=0;

		self = ent;

		if (self->client->resp.ctf_team == CTF_TEAM1)
		{
			flag = flag1_ent;
			enemy_flag = flag2_ent;
		}
		else
		{
			flag = flag2_ent;
			enemy_flag = flag1_ent;
		}

		// look for some helpers
		if (last_checkhelp < (level.time - 0.5))
		{
			for (i=0; i<num_players; i++)
			{
				plyr = players[i];

				if (plyr->client->resp.ctf_team != self->client->resp.ctf_team)
				{
					if (	(plyr->enemy != self) 
						&&	(!plyr->target_ent || (plyr->target_ent->think != CTFFlagThink) || (entdist(plyr, plyr->target_ent) > 1000))
						&&	(entdist(plyr, self) < 2000))
					{	// send this enemy to us
						plyr->enemy = self;
					}
//					continue;
				}

				if ((plyr != self) && (plyr->target_ent == self))
					count++;
			}

			ideal = ((int)ceil((1.0*(float)num_players)/4.0));

			if (count < ideal)
			{
				for (i=0; (i<num_players && count<ideal); i++)
				{
					plyr = players[i];

					if (plyr->client->resp.ctf_team != self->client->resp.ctf_team)
						continue;

					if (plyr->target_ent == self)
						continue;

					if (entdist(plyr, self) > 700)
						continue;

					if (!gi.inPVS(plyr->s.origin, self->s.origin))
						continue;

					plyr->target_ent = self;
					if (++count >= ideal)
						break;
				}
			}
			else if (count > ideal)	// release a defender
			{
				for (i=0; (i<num_players && count<ideal); i++)
				{
					plyr = players[i];

					if (plyr->client->resp.ctf_team != self->client->resp.ctf_team)
						continue;

					if (plyr->target_ent != self)
						continue;

					plyr->target_ent = NULL;
					break;
				}
			}

			last_checkhelp = level.time + random()*0.5;
		}

	}

	if (ent->flags & FL_SHOW_FLAGPATHS)
	{
		edict_t *trav;

		// show lines between alternate routes
		trav = NULL;
		while (trav = G_Find(trav, FOFS(classname), "flag_path_src"))
		{
			if (!trav->last_goal || !trav->target_ent)
				continue;	// not complete, don't save

			trav->s.modelindex = gi.modelindex ("models/objects/gibs/chest/tris.md2");

			if (trav->last_goal)
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BFG_LASER);
				gi.WritePosition (trav->s.origin);
				gi.WritePosition (trav->last_goal->s.origin);
				gi.multicast (trav->s.origin, MULTICAST_PVS);
			}
			if (trav->target_ent)
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BFG_LASER);
				gi.WritePosition (trav->s.origin);
				gi.WritePosition (trav->target_ent->s.origin);
				gi.multicast (trav->s.origin, MULTICAST_PHS);
			}

		}
	}
	//ERASER

}
