#include "g_local.h"
#include "m_player.h"
#ifdef MIDI
#include "g_midi.h"
#include <io.h>
#endif

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
				self->targetname = spot->targetname;
			
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
void SP_info_player_intermission(edict_t *ent)
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


void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	if (ctf->value || deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = ".";

		switch (mod)
		{
		case MOD_SUICIDE:			
			if (random() < 0.5)
				message = "tries to shoot backwards";
			else
				message = "had a mid-life crisis";
			break;
		case MOD_FALLING:
			if (random() < 0.5)
				message = "fell off the world";
			else
				if (IsFemale(self))
					message = "broke her neck";
				else
					message = "broke his neck";
			break;
		case MOD_CRUSH:
			message = "was squished";
			break;
		case MOD_WATER:
			if (random() < 0.5)
				message = "lives with the fishes";
			else
				message = "sinks like a lead balloon";
			break;
		case MOD_SLIME:
			if (random() < 0.5)
				message = "meets the slime monster";
			else
				message = "slivers into extinction";
			break;
		case MOD_LAVA:
			if (random() < 0.5)
				message = "burns in lava";
			else
				if (IsFemale(self))
					message = "loses her cool";
				else
					message = "loses his cool";
			break;
		case MOD_EXPLOSIVE:
			message = "blew up";
		case MOD_BARREL:
			message = "became a barrel of laughs";
			break;
		case MOD_EXIT:
			message = "found a way out";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light at the end of the tunnel";
			break;
		case MOD_TARGET_BLASTER:
			message = "got blasted to hell";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place at the wrong time";
			break;
		}
		if (attacker == self)
		{
			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = "took the pin out and didn't throw";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (random() < 0.5)
					message = "got legless";
				else
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
			gi.bprintf (PRINT_CHAT, "%s", self->client->pers.netname);
			gi.bprintf (PRINT_MEDIUM, " %s.\n", message);
			if (ctf->value || deathmatch->value)
				self->client->resp.score--;
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
				if (random() < 0.5)
					message = "was gunned down by";
				else 
					message = "shellshocked by";
				break;
			case MOD_SSHOTGUN:
				message = "was blown away by";
				message2 = "'s super shotgun.";
				break;
			case MOD_MACHINEGUN:
				message = "ate";
				message2 = "'s bullets.";
				break;				
			case MOD_CHAINGUN:
				message = "was cut in half by";
				message2 = "'s chaingun.";
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
				if (random() < 0.5)
					message = "says hello to";
				else
					message = "eats";
				message2 = "'s rocket.";
				break;
			case MOD_R_SPLASH:
				message = "almost dodged";
				message2 = "'s rocket.";
				break;
			case MOD_HYPERBLASTER:
				message = "was melted by";
				message2 = "'s hyperblaster.";
				break;
			case MOD_RAILGUN:
				message = "was railed by";
				break;
			case MOD_BFG_LASER:
				message = "saw the pretty coloured lights from";
				message2 = "'s BFG.";
				break;
			case MOD_BFG_BLAST:
				message = "was disintegrated by";
				message2 = "'s BFG blast.";
				break;
			case MOD_BFG_EFFECT:
				message = "couldn't hide from";
				message2 = "'s BFG.";
				break;
			case MOD_HANDGRENADE:
				message = "caught";
				message2 = "'s handgrenade.";
				break;
			case MOD_HG_SPLASH:
				message = "didn't see";
				message2 = "'s handgrenade.";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain.";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space.";
				break;
			case MOD_GRAPPLE:	//ctf
				message = "was caught by";
				message2 = "'s grapple";
				break;
			}
			if (message)
			{
				gi.bprintf (PRINT_CHAT,"%s ", self->client->pers.netname);
				gi.bprintf (PRINT_MEDIUM,"%s",message);
				gi.bprintf (PRINT_CHAT, " %s", attacker->client->pers.netname);
				gi.bprintf (PRINT_MEDIUM,"%s\n",message2);			
				
				if ( (attacker != NULL) && (attacker->killing_time > 0) && (strcmp(attacker->classname, "player") == 0) &&
					(deathmatch->value || ctf->value || mbm->value))
				{
					if (attacker->kills_in_time == 0)
						attacker->kills_in_time = 1;

					attacker->kills_in_time++;

					if (attacker->kills_in_time == 2)
						message = "Double kill!";

					else if (attacker->kills_in_time == 3)
						message = "Triple kill!";

					else if (attacker->kills_in_time == 4)
						message = "Quad kill!";

					else if (attacker->kills_in_time == 5)
						message = "Pentahedron kill!";

					else if (attacker->kills_in_time == 5)
						message = "Hexastyle kill!";
					
					message2= " ";

					if (attacker->head_shot == 1)
						message2 = "Head Shot";

					if (attacker->kills_in_time <6)
						gi.centerprintf (attacker,"%s\n\n%s\n",message,message2);
					else if (attacker->kills_in_time > 5)
						gi.centerprintf (attacker,"%i kills\n\n%s\n",attacker->kills_in_time, message2);
				

				}
				if (strcmp(attacker->classname, "player") == 0)
					attacker->killing_time=300;					
				

				if (ctf->value || deathmatch->value)
				{
					if (ff)
						attacker->client->resp.score--;
					else
						attacker->client->resp.score++;
				}
				return;
			}
		}
	}

	gi.bprintf (PRINT_CHAT,"%s", self->client->pers.netname);
	gi.bprintf (PRINT_MEDIUM," died.\n");
	if (ctf->value || deathmatch->value)
		self->client->resp.score--;
}

void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	qboolean	quad;
	float		spread;
	
	if (!ctf->value && !deathmatch->value)
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
	
	if ((item) && (drop_all_wep->value == 0) || ((!deathmatch->value) && (!ctf->value )))
	{
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item (self, item);
		self->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
		
	}
	else if (item)
	{
		if ( self->client->pers.inventory[ITEM_INDEX(FindItem("BFG10K"))] )
		{
			self->client->v_angle[YAW] =self->client->v_angle[YAW] + 20;
			drop = Drop_Item (self,  FindItem ("BFG10K"));
			drop->spawnflags = DROPPED_PLAYER_ITEM;
		}
		if ( self->client->pers.inventory[ITEM_INDEX(FindItem("railgun"))] )
		{
			self->client->v_angle[YAW] =self->client->v_angle[YAW] + 10;
			drop = Drop_Item (self,  FindItem ("railgun"));
			drop->spawnflags = DROPPED_PLAYER_ITEM;
		}
		
		if ( self->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))] )
		{
			self->client->v_angle[YAW] =self->client->v_angle[YAW] + 20;
			drop = Drop_Item (self,  FindItem ("hyperblaster"));
			drop->spawnflags = DROPPED_PLAYER_ITEM;
		}
		
		if ( self->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))] )
		{
			self->client->v_angle[YAW] =self->client->v_angle[YAW] + 20;
			drop = Drop_Item (self,  FindItem ("chaingun"));
			drop->spawnflags = DROPPED_PLAYER_ITEM;
		}
		
		if ( self->client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))] )
		{
			self->client->v_angle[YAW] =self->client->v_angle[YAW] + 20;
			drop = Drop_Item (self,  FindItem ("machinegun"));
			drop->spawnflags = DROPPED_PLAYER_ITEM;
		}
		
		if ( self->client->pers.inventory[ITEM_INDEX(FindItem("super shotgun"))] )
		{
			self->client->v_angle[YAW] =self->client->v_angle[YAW] + 20;
			drop = Drop_Item (self,  FindItem ("super shotgun"));
			drop->spawnflags = DROPPED_PLAYER_ITEM;
		}
		
		if ( self->client->pers.inventory[ITEM_INDEX(FindItem("Grenade Launcher"))] )
		{
			self->client->v_angle[YAW] =self->client->v_angle[YAW] + 20;
			drop = Drop_Item (self,  FindItem ("Grenade Launcher"));
			drop->spawnflags = DROPPED_PLAYER_ITEM;
		}
		
		if (self->client->pers.inventory[ITEM_INDEX(FindItem("Bandolier"))] )		
		{
			self->client->v_angle[YAW] =self->client->v_angle[YAW] + 20;
			drop = Drop_Item (self,  FindItem ("Bandolier"));
			drop->spawnflags = DROPPED_PLAYER_ITEM;
		}
		
		if (self->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] )		
		{
			self->client->v_angle[YAW] =self->client->v_angle[YAW] + 20;
			drop = Drop_Item (self,  FindItem ("Grenades"));
			drop->spawnflags = DROPPED_PLAYER_ITEM;
		}
		
		
		if (self->client->pers.inventory[ITEM_INDEX(FindItem("Ammo Pack"))] )		
		{
			self->client->v_angle[YAW] =self->client->v_angle[YAW] + 20;
			drop = Drop_Item (self,  FindItem ("Ammo Pack"));
			drop->spawnflags = DROPPED_PLAYER_ITEM;
		}
		
		if ( self->client->pers.inventory[ITEM_INDEX(FindItem("Rocket Launcher"))] )
		{
			self->client->v_angle[YAW] =self->client->v_angle[YAW] + 10;
			drop = Drop_Item (self,  FindItem ("Rocket Launcher"));
			drop->spawnflags = DROPPED_PLAYER_ITEM;
		}
		/*
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item (self, item);
		self->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;*/
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

	if (Q_stricmp(self->classname, "bot") == 0)
           return;
            
	//gi.dprintf ("%s\n",attacker->classname);
	if (attacker  != world)
	{
		float r;

		self->client->killer_info = attacker;

		r = random ();
		if (random() < 0.3)
			{
				cam_way0 =2;
				cam_way1 =2;
			}
			else if (r < 0.6)
			{
				cam_way0 =3;
				cam_way1 =3;
			}
			else
			{
				cam_way0 =0;
				cam_way1 =0;
			}

	}
	else
	{
		self->client->killer_info = self;
	}

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

	VectorClear (self->avelocity);

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

	//remove all effects
	self->s.effects = 0;

	self->maxs[2] = -8;

	//switch back to normal grenades
	self->grenade_type = false;

//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		self->client->respawn_time = level.time + 1.0;
		
		LookAtKiller (self, inflictor, attacker);

		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);
		
		//ctf
		CTFFragBonuses(self, inflictor, attacker);

		TossClientWeapon (self);

		//ctf
		CTFPlayerResetGrapple(self);
		CTFDeadDropFlag(self);
		CTFDeadDropTech(self);

		if (deathmatch->value && !self->client->showscores)
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
	self->deadflag = DEAD_DEAD;

	if (strcmp(self->classname, "player") == 0)
	{
		CopyToBodyQue(self);
		self->s.modelindex = gi.modelindex ("sprites/torchC.sp2");
		self->solid = SOLID_NOT;
		//gi.setmodel (self, "models/objects/gibs/skull/tris.md2");
		self->s.frame = 0;
	}

	gi.linkentity (self);
}

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

	//ctf
	//client->pers.lastweapon = item;

	//ctf
	//item = FindItem("Grapple");
	//client->pers.inventory[ITEM_INDEX(item)] = 1;

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
	//ctf	
	int ctf_team = client->resp.ctf_team;

	memset (&client->resp, 0, sizeof(client->resp));
	
	//ctf
	if (ctf->value)
		client->resp.ctf_team = ctf_team;

	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;
 
	//ctf
	if (ctf->value && client->resp.ctf_team < CTF_TEAM1)
		CTFAssignTeam(client);
	
	//mbm-a
//	if (mbm->value && client->resp.ctf_team < CTF_TEAM1)
//		mbmAssignTeam(client);
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

	//mbm-a
	if ((ctf->value) || (mbm->value))//ctf
			spot = SelectCTFSpawnPoint(ent);

	else if (deathmatch->value)
		spot = SelectDeathmatchSpawnPoint ();
	//else if ((ctf->value)	|| (mbm->value))//ctf
	//		spot = SelectCTFSpawnPoint(ent);		
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
	//mbm-a
	if (mbm->value || ctf->value || deathmatch->value || coop->value)
	{        
		if (Q_stricmp(self->classname, "bot") == 0)
	    {
	       Bot_Respawn(self);
	       return;
		}		
        // end if        
    
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
	client_persistant_t	saved;
	client_respawn_t	resp;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	index = ent-g_edicts-1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
	//mbm-a
	if (mbm->value || ctf->value || deathmatch->value)
	{
		edict_t *player_light;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);

		//turn on the light for the player
		if (strcmp(ent->classname, "player") == 0)
		{
			player_light = G_Find (NULL, FOFS(targetname), ent->client->pers.netname);
	
			if (player_light != NULL)
				player_light->s.effects = 64;
		}

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

		if ((number_bot> 0) && (current_number_bot != number_bot))
		{
			for (i=0;i<number_bot;i++)
					Bot_Create();
			
		}

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

	//ctf
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

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
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	//ctf
	if (ctf->value) 
		if (CTFStartClient(ent))
			return;
	//mbm-a
	if (mbm->value)
	{
		if (mbmStartClient(ent))
			return;
	}

	if (deathmatch->value)
		if (MPStartClient(ent))
			return;

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.
=====================
*/
char *ClientTeam (edict_t *ent);

void ClientBeginDeathmatch (edict_t *ent)
{
	int i;

	G_InitEdict (ent);

	InitClientResp (ent->client);

	// locate ent at a spawn point
	PutClientInServer (ent);

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.bprintf(PRINT_CHAT, "%s", ent->client->pers.netname);
	gi.bprintf(PRINT_HIGH, " entered the game.\n");

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);

	if (number_bot > 0)
	{
		current_number_bot = 0;

		if ((deathmatch->value) && ((int)(dmflags->value) & (DF_MODELTEAMS)))
		{
			if (strcmp(ClientTeam(ent),"male") == 0)
				TP_player_team = 0;
			if (strcmp(ClientTeam(ent),"female") == 0)
				TP_player_team = 1;
			if (strcmp(ClientTeam(ent),"cyborg") == 0)
				TP_player_team = 2;
		}

		for (i=0;i<number_bot;i++)
			Bot_Create();
	}

	if ( (strcmp(ent->classname, "player") == 0) && (!ctf->value && !mbm->value))
		gi.centerprintf (ent,"Prepare for battle!\n");

	//make a light
	if (darkmatch->value)
		make_light(ent, ent->s.origin );	

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

	//load_bot_names(ent);
	//Make_bot_points(ent);
	//load_bot_messages(ent);

	//ctf_file_flags();
	//midi
//	if (midi->value)
//		MidiCmd(ent, MCMD_PLAY);

	//mdm-a
	if (mbm->value || ctf->value || deathmatch->value )
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
		current_number_bot = 0;		
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

	//make a light
	if (darkmatch->value)
		make_light(ent, ent->s.origin );

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

	//ctf
	//mdm-a
	if ( (ctf->value) || (mbm->value))
		CTFAssignSkin(ent, s);
	else
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

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");

	// check for a password
	//value = Info_ValueForKey (userinfo, "password");
	//if (strcmp(password->string, value) != 0)
		//return false;

	//return true;
	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{

		//ctf
		ent->client->resp.ctf_team = -1;		
		// clear the respawning variables
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
	}

	ClientUserinfoChanged (ent, userinfo);

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
void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!ent->client)
		return;

	gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

	//ctf
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);

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

	if (darkmatch->value)
		del_light(ent);

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
	float distance, travel;
	vec3_t temp,move;

    level.current_entity = ent;
	client = ent->client;

//	check_for_stay_at_base(ent);
//	if (ent->bot_stuff != NULL)
//		gi.dprintf ("%s\n",ent->bot_stuff->classname);
	//gi.dprintf ("obj:%i  type:%i  time:%i\n",level.mbm_objectives, level.mbm_type, level.mbm_timelimit );
	//draw_path(ent);
//	if (ent->groundentity != NULL)
//	gi.dprintf("%s...%i   \n",ent->groundentity->classname,ent->groundentity->moveinfo.state);
//	gi.dprintf("[%s]  [%s] \n",vtos(ent->groundentity->mins), vtos(ent->s.origin));}

	if (level.intermissiontime) 
	{
		char *temp;

		client->ps.pmove.pm_type = PM_FREEZE;		
		
		if( (darkmatch->value) && (strcmp(ent->classname, "player") == 0) && (ent->count < 1200) )
		{
			
//			if (strcmp(ent->classname, "player") == 0)
//			{
//				if (ent->count < 1200)
//				{
				ent->count +=3;
								
				//c, d, e, f, g, h, i, j, k, l, m (11 states)				
				if (ent->count >= 1100)	
					temp="m";
				else if (ent->count >= 1000)	
					temp="l";
				else if (ent->count >= 900)	
					temp="k";
				else if (ent->count >= 800)	
					temp="j";
				else if (ent->count >= 700)	
					temp="i";
				else if (ent->count >= 600)
					temp="h";
				else if (ent->count >= 500)
					temp="g";
				else if (ent->count >= 400)	
					temp="f";
				else if (ent->count >= 300)
					temp="e";
				else if (ent->count >= 200)	
					temp="d";
				else if (ent->count >= 100)	
					temp="c";
				else
					temp="b";
				
				ent->linkcount=0;
				//		gi.dprintf ("%i \n", ent->count );
				
				gi.configstring(CS_LIGHTS+0, temp );//"m");
//				} else
//					ent->client->ps.viewangles[1] = ent->client->ps.viewangles[1] + 0.2;
//			}
		} 
		else	
		{
			//spin cam around
			if (ent->count2) 
			{
				ent->client->ps.viewangles[1] = ent->client->ps.viewangles[1] + 0.2;
				ent->linkcount++;
			}
			else
				ent->linkcount++;
		}

		//if ((ent->client->ps.viewangles[1] > 500) && (ctf->value || deathmatch->value))
		if ((ent->linkcount > 900) && (ctf->value || deathmatch->value))			
		{
			edict_t	*newcam;
			newcam=NULL;
			
			//maybe look at the flags in CTF
			if ((ctf->value) && (random() < 0.2))
			{
				char *c;

				if (random() < 0.5)
					c = "item_flag_team1";
				else
					c = "item_flag_team2";
	
				while ((newcam = G_Find (newcam, FOFS(classname), c)) != NULL) 
					if (!(newcam->spawnflags & DROPPED_ITEM))
						break;
			}
			else
				//or not,?
				newcam = SelectRandomDeathmatchSpawnPoint();

			if (newcam != NULL)
			{
			//spin around?
			ent->count2 = random() *1.6;
			//used has timer
			ent->linkcount=0;
	
			VectorCopy (newcam->s.origin,level.intermission_origin);
			VectorCopy (newcam->s.angles,level.intermission_angle);
			MoveClientToIntermission(ent);
			}
		}
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 3.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}

	pm_passent = ent;


	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else
		if (ent->s.modelindex != 255)
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
		pm.snapinitial = true;
	

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

	if (!ent->deadflag)
	{
		VectorCopy (pm.viewangles, client->v_angle);
		VectorCopy (pm.viewangles, client->ps.viewangles);
	}
	
	//ctf
//	if (client->ctf_grapple)
//		CTFGrapplePull(client->ctf_grapple);

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
	//ctf
	if ((client->latched_buttons & BUTTON_ATTACK )
	&& (ent->movetype != MOVETYPE_NOCLIP || !(deathmatch->value || ctf->value || mbm->value)))
	{
		if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

	//ctf
	//regen tech
	if ((ctf->value) || (deathmatch->value == 3))
		CTFApplyRegeneration(ent);

	//
	if (darkmatch->value)
		light(ent);

	//double, tri etc.. counter
	if (ent->killing_time > 0) 
		ent->killing_time--;
	else 
	{
		ent->kills_in_time = 0;
		ent->head_shot = 0;
	}

//	if (strcmp(ent->classname, "player") == 0)
//	gi.dprintf("%i [] %i\n",ent->client->resp.enterframe,level.framenum );
	//midi
#ifdef MIDI
	if (miditried == 0)
		if ((ent->client->resp.enterframe)+20 <= level.framenum)
		{
			miditried = 1;
			if ((Q_stricmp(ent->classname, "player") == 0))		
			{
			//	ent->miditried = 1;
				if (midi->value)
					MidiCmd(ent, MCMD_PLAY);
			}
		}

	//check to see midi needs to be repeated
	if (midi->value)	
		ChkLocalMidi(ent);
#endif
	
	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
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
	//ctf
	if ((!client->weapon_thunk)
	&& (ent->movetype != MOVETYPE_NOCLIP || !(deathmatch->value ||ctf->value || mbm->value)))
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
