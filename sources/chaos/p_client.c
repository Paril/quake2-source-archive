#include "g_local.h"
#include "m_player.h"
#include "c_base.h"
#include "c_item.h"
#include "c_botai.h"
#include "c_botnav.h"
#include "c_cam.h"
#include "stdlog.h"
#include "gslog.h"


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

	if (deathmatch->value)
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
			/*		case MOD_TARGET_LASER:
			message = "saw the light";
			break;   FWP This is now handled as a kill */
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
			case MOD_ESSHOT_SPLASH:
				if (IsFemale(self))
					message = "used the wrong end of her explosive super shotgun";
				else
					message = "used the wrong end of his explosive super shotgun";
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
			int i;

			bprintf2 (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
			self->client->resp.score--;
			self->enemy = NULL;

			if (strcmp(self->classname, "bot") == 0)
			{
				i = (int) (random() * (float) chat_linecount[BOTCHAT_SELFKILL]);

				Bot_Say (attacker, 0, "%s: ", self->client->pers.netname);
				Bot_Say (attacker, 0, chat_text[BOTCHAT_SELFKILL][i], self->client->pers.netname);
				Bot_Say (attacker, 0, "\n");
			}
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
			case MOD_ESSHOT_SPLASH:
				message = "was pulverized by";
				message2 = "'s explosive blast";
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
				message = "has been telefragged by";
				message2 = " !";
				break;
			case MOD_GRAPPLE:
				message = "was gutted by";
				message2 = "'s grappling hook";
				break;
			case MOD_ARROW:
				message = "was nailed by";
				message2 = "'s arrow";
				break;
			case MOD_PARROW:
				message = "couldn't stand all the poison off";
				message2 = "'s poison arrow";
				break;
			case MOD_ESSHOTGUN:
				message = "was flattened by";
				message2 = "'s explosive super shotgun";
				break;
			case MOD_AIRFIST:
				message = "was blown away by";
				message2 = "'s airgun";
				break;
			case MOD_PROXYMINE:
				message = "was kissed by";
				message2 = "'s proxy mine";
				break;
			case MOD_HOMING:
				message = "was the target of";
				message2 = "'s homing missile";
				break;
			case MOD_BUZZ:
				message = "was cut in half by";
				message2 = "'s buzzsaw";
				break;
			case MOD_VORTEX:
				message = "was sucked in by";
				message2 = "'s gravity vortex";
				break;
			case MOD_KAMIKAZE:
				message = "was killed by";
				message2 = "'s kamikaze bomb";
				break;
			case MOD_TURRET:
				message = "was caught by";
				message2 = "'s turret";
				break;
			case MOD_SWORD:
				message = "was cut in half";
				message2 = "'s bastard sword";
				break;
			case MOD_CHAINSAW:
				message = "couldn't hide from";
				message2 = "'s chainsaw";
				break;
			case MOD_PGRENADE:
				message = "couldn't stand all the poison off";
				message2 = "'s poison grenade";
				break;
			case MOD_FGRENADE:
				message = "couldn't stand the light off";
				message2 = "'s flash grenade";
				break;
			case MOD_AK42:
				message = "was shot by";
				break;

			}
			if (message)
			{
				bprintf2 (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if (deathmatch->value)
				{
				if (ff)
					attacker->client->resp.score--;
				else
					attacker->client->resp.score++;
			}
				
			if ((strcmp(attacker->classname, "bot") == 0) && visible (self, attacker))
			{
				float	rn = random();
				int		i;

				if (!TeamMembers(self, attacker))
				{
					if ((attacker->health > 20) && infront (self, attacker))
					{
						if (rn < 0.4)
							Bot_Wave (attacker, 0, 1.1);
						else if ((rn >= 0.4) && (rn < 0.8))
							Bot_Wave (attacker, 1, 1.1);
						else
							Bot_Wave (attacker, 2, 1.7);
					}

					i = (int) (random() * (float) chat_linecount[BOTCHAT_KILL]);

					Bot_Say (attacker, 0, "%s: ", attacker->client->pers.netname);
					Bot_Say (attacker, 0, chat_text[BOTCHAT_KILL][i], self->client->pers.netname);
					Bot_Say (attacker, 0, "\n");
				}
				else
				{
					i = (int) (random() * (float) chat_linecount[BOTCHAT_TEAMKILL]);

					Bot_Say (attacker, 0, "%s: ", attacker->client->pers.netname);
					Bot_Say (attacker, 0, chat_text[BOTCHAT_TEAMKILL][i], self->client->pers.netname);
					Bot_Say (attacker, 0, "\n");
				}
			}
			return;
		 }
	}
	else   // FWP If attacker exists but attacker->client is not known, the attacker is an
	       //     object. Retrieve attacker->owner->client instead.  So far the only item
	       //     I've found that acts this way is the rocket turret.
			  
	  {
	    if (attacker && attacker->owner && attacker->owner->client)
	      {
		if (attacker->owner == self) 
		  {
		    switch (mod)
		      {
		      case MOD_ROCKET:
			if (IsFemale(self))
			  message = "is killed by her own rocket turret";
			else
			  message = "is killed by his own rocket turret";
			break;
		      case MOD_R_SPLASH:
			if (IsFemale(self))
			  message = "is killed by her own rocket turret";
			else
			  message = "is killed by his own rocket turret";
			break;
		      }
		    if (message)
		      {
			bprintf2 (PRINT_MEDIUM,"%s %s\n", self->client->pers.netname, message);
			if (deathmatch->value)
			  {
			    if (ff)
			      attacker->owner->client->resp.score--;
			    else
			      attacker->owner->client->resp.score--;
			  }
		      }
                  }
                else
                  {            
		    switch (mod)
		      {
		      case MOD_ROCKET:
			message = "eats a rocket from";
			message2 = "'s rocket turret";
			break;
		      case MOD_R_SPLASH:
			message = "almost snuck away from";
			message2 = "'s rocket turret";
			break;
		      }
		    if (message)
		      {
			bprintf2 (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->owner->client->pers.netname, message2);
			if (deathmatch->value)
			  {
			    if (ff)
			      attacker->owner->client->resp.score--;
			    else
			      attacker->owner->client->resp.score++;
			  }
		      }
		  }
	      }
              else
		{   // fwp Laser mines are two levels of indirection deep
		  if (attacker && attacker->owner && attacker->owner->owner && attacker->owner->owner->client)
		    {
		      if (attacker->owner->owner == self) 
			{
			  switch (mod)
			    {
			    case MOD_TARGET_LASER:
			      if (IsFemale(self))
				message = "fries her brains on her own laser mine";
			      else
				message = "fries his brains on his own laser mine";
			      break;
			    }
			  if (message)
			    {
			      bprintf2 (PRINT_MEDIUM,"%s %s\n", self->client->pers.netname, message);
			      if (deathmatch->value)
				{
				  if (ff)
				    attacker->owner->owner->client->resp.score--;
				  else
				    attacker->owner->owner->client->resp.score--;
				}
			    }
			}
		      else
			{            
			  switch (mod)
			    {
			    case MOD_TARGET_LASER:
			      message = "sees the light from";
			      message2 = "'s laser mine";
			      break;
			    }
			  if (message)
			    {
			      bprintf2 (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->owner->owner->client->pers.netname, message2);
			      if (deathmatch->value)
				{
				  if (ff)
				    attacker->owner->owner->client->resp.score--;
				  else
				    attacker->owner->owner->client->resp.score++;
				}
			    }
			}
		    }
		}

	  return;
	  }


	}

	bprintf2 (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (deathmatch->value)
	self->client->resp.score--;
}


void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
        gitem_t         *ammo;   //FWP Fakedeath ammo cheat fix
	edict_t		*drop;
	qboolean	quad;
	float		spread;

	if (!deathmatch->value)
		return;

	item = self->client->pers.weapon;
	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;
	if (item && (strcmp (item->pickup_name, "AK42 Assault Pistol") == 0))
		item = NULL;

	// FWP Fix for fakedeath ammo cheat...if fakedeath, and ammo > default ammo for weapon, toss and
        //     subtract default ammo, else dont toss
	if ((item) && (self->health > 0))
	  {
	    // FWP decrease ammo unless weapon doesnt use ammo 

	    if (item->ammo)
	      {
		ammo = FindItem (item->ammo);
		if (self->client->pers.inventory[self->client->ammo_index] >  ammo->quantity)
		  self->client->pers.inventory[self->client->ammo_index]-= ammo->quantity;
		else
		  item = NULL;
	      }
}


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
	self->client->scanneractive = 0;	//MATTHIAS
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

	self->maxs[2] = -8;

	self->clipmask = MASK_PLAYERSOLID;
	self->solid = SOLID_BBOX;
	self->svflags &= SVF_DEADMONSTER;

	if (!self->deadflag || self->client->fakedeath)
	{
	  // FWP Fix for neg frags on fakedeath in lava, once real death pccurs, unset fakedeath flag
	        self->client->fakedeath = 0;

		self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);

		sl_WriteStdLogDeath( &gi, level, self, inflictor, attacker);	// StdLog - Mark Davies
//ZOID
		CTFFragBonuses(self, inflictor, attacker);
		TossClientWeapon (self);

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
	self->client->invisible_framenum = 0;

	// clear inventory
	memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));

	if (Jet_Active(self))
    {
		Jet_BecomeExplosion( self, damage );
		self->client->jet_framenum = 0;
    }

	if (self->health < -40)
	{	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		
		ThrowGib (self, "models/objects/gibs/sm_gib1/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/sm_gib2/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/sm_gib2/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/sm_gib3/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/sm_gib4/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/sm_gib5/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC);

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
		self->s.modelindex = 255;
	}

	self->deadflag = DEAD_DEAD;

	//MATTHIAS
	self->client->BlindTime = 0;
	self->client->PoisonTime = 0;
	self->client->invisible = 0;

	if (self->client->b_target)
	{
		G_FreeEdict(self->client->b_target);
		self->client->b_target = NULL;
	}

	self->client->flashlightactive = 0;
	if(self->client->flashlight)
	{
		G_FreeEdict(self->client->flashlight);
	}
	if(self->client->teleporter)
		G_FreeEdict(self->client->teleporter);

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

	it_lturret = FindItem("automatic defence turret");	//bugfix

	memset (&client->pers, 0, sizeof(client->pers));

	item = FindItem("AK42 Assault Pistol");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	client->pers.weapon = item;
//ZOID
	client->pers.lastweapon = item;
//ZOID

	//MATTHIAS  -  Startup Weapons/Items


	if (it_sword && it_chainsaw)
	{
		if (start_sword->value > 0)
			client->pers.inventory[ITEM_INDEX(it_sword)] = 1;

		if (start_chainsaw->value > 0)
			client->pers.inventory[ITEM_INDEX(it_chainsaw)] = 1;

		if (start_supershotgun->value > 0)
		  {
			client->pers.inventory[ITEM_INDEX(it_supershotgun)] = 1;
			client->pers.inventory[ITEM_INDEX(it_esupershotgun)] = 1;
                     
                  }
		if (start_crossbow->value > 0)
		{
			client->pers.inventory[ITEM_INDEX(it_crossbow)] = 1;
			client->pers.inventory[ITEM_INDEX(it_poisoncrossbow)] = 1;
			client->pers.inventory[ITEM_INDEX(it_explosivecrossbow)] = 1;
		}

		if (start_airgun->value > 0)
			client->pers.inventory[ITEM_INDEX(it_airfist)] = 1;

		if (start_grenadelauncher->value > 0)
		{
			client->pers.inventory[ITEM_INDEX(it_grenadelauncher)] = 1;
			client->pers.inventory[ITEM_INDEX(it_flashgrenadelauncher)] = 1;
			client->pers.inventory[ITEM_INDEX(it_poisongrenadelauncher)] = 1;
		}

		if (start_proxylauncher->value > 0)
		{
			client->pers.inventory[ITEM_INDEX(it_proxyminelauncher)] = 1;
		}

		if (start_rocketlauncher->value > 0)
		{
			client->pers.inventory[ITEM_INDEX(it_rocketlauncher)] = 1;
			client->pers.inventory[ITEM_INDEX(it_hominglauncher)] = 1;
		}

		if (start_hyperblaster->value > 0)
			client->pers.inventory[ITEM_INDEX(it_hyperblaster)] = 1;

		if (start_railgun->value > 0)
			client->pers.inventory[ITEM_INDEX(it_railgun)] = 1;

		if (start_buzzsaw->value > 0)
			client->pers.inventory[ITEM_INDEX(it_buzzsaw)] = 1;

		if (start_bfg->value > 0)
			client->pers.inventory[ITEM_INDEX(it_bfg)] = 1;

		if (start_grapple->value > 0)
			client->pers.inventory[ITEM_INDEX(it_grapple)] = 1;

		if (start_gravityvortex->value > 0)
			client->pers.inventory[ITEM_INDEX(it_vortex)] = 1;

		if (start_defenceturret->value > 0)
			client->pers.inventory[ITEM_INDEX(it_lturret)] = start_defenceturret->value;

		if (start_rocketturret->value > 0)
			client->pers.inventory[ITEM_INDEX(it_rturret)] = start_rocketturret->value;

		if (start_bodyarmor->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("item_armor_body"))] = 1;

		if (start_combatarmor->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("item_armor_combat"))] = 1;

		if (start_jacketarmor->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("item_armor_jacket"))] = 1;

		if (start_armorshard->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("item_armor_shard"))] = 1;

		if (start_powerscreen->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("item_power_screen"))] = 1;

		if (start_powershield->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("item_power_shield"))] = 1;

		if (start_ammo_grenades->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_grenades"))] = start_ammo_grenades->value;
		
		if (start_ammo_flashgrenades->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_flashgrenades"))] = start_ammo_flashgrenades->value;
		
		if (start_ammo_lasergrenades->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_lasermines"))] = start_ammo_lasergrenades->value;
		
		if (start_ammo_poisongrenades->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_poisongrenades"))] = start_ammo_poisongrenades->value;
		
		if (start_ammo_proximitymines->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_proxymines"))] = start_ammo_proximitymines->value;
		
		if (start_ammo_shells->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_shells"))] = start_ammo_shells->value;

		if (start_ammo_explosiveshells->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_explosiveshells"))] = start_ammo_explosiveshells->value;

		if (start_ammo_arrows->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_arrows"))] = start_ammo_arrows->value;

		if (start_ammo_poisonarrows->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_poisonarrows"))] = start_ammo_poisonarrows->value;

		if (start_ammo_explosivearrows->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_explosivearrows"))] = start_ammo_explosivearrows->value;

		if (start_ammo_cells->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_cells"))] = start_ammo_cells->value;

		if (start_ammo_rockets->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_rockets"))] = start_ammo_rockets->value;

		if (start_ammo_homingmissiles->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_homing"))] = start_ammo_homingmissiles->value;

		if (start_ammo_buzzes->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_buzzes"))] = start_ammo_buzzes->value;

		if (start_ammo_slugs->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("ammo_slugs"))] = start_ammo_slugs->value;

		if (start_quaddamage->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("item_quad"))] = start_quaddamage->value;

		if (start_jetpack->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("item_jet"))] = start_jetpack->value;

		if (start_invulnerability->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("item_invulnerability"))] = start_invulnerability->value;

		if (start_silencer->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("item_silencer"))] = start_silencer->value;

		if (start_rebreather->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("item_breather"))] = start_rebreather->value;

		if (start_environmentsuit->value > 0)
			client->pers.inventory[ITEM_INDEX(FindItemByClassname ("item_enviro"))] = start_environmentsuit->value;

	}

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 50;

	client->pers.max_eshells         = 100; //MATTHIAS
	client->pers.max_arrows          = 50;
	client->pers.max_poisonarrows    = 50;
	client->pers.max_explosivearrows = 50;
	client->pers.max_flashgrenades   = 50;
	client->pers.max_lasergrenades   = 50;
	client->pers.max_poisongrenades  = 50;
	client->pers.max_proxymines      = 50;
	client->pers.max_homing          = 50;
	client->pers.max_buzzes          = 50;
	client->pers.max_lturret	     = 1;
	client->pers.max_rturret	     = 1;
	client->pers.max_vortex          = 1;

	client->pers.connected = true;
}


void InitClientResp (gclient_t *client)
{
//ZOID
	int ctf_team = client->resp.ctf_team;
//ZOID

	memset (&client->resp, 0, sizeof(client->resp));
	
//ZOID
	client->resp.ctf_team = ctf_team;
//ZOID

	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;
 
//ZOID
	if (ctf->value && client->resp.ctf_team < CTF_TEAM1)
		CTFAssignTeam(client);
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

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->health < -50)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		ThrowGib (self, "models/objects/gibs/sm_gib1/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/sm_gib2/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/sm_gib2/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/sm_gib3/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/sm_gib4/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/sm_gib5/tris.md2", damage, GIB_ORGANIC);
		ThrowGib (self, "models/objects/gibs/bone2/tris.md2", damage, GIB_ORGANIC);
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
	body->health = 100;

	gi.linkentity (body);
}


void respawn (edict_t *self)
{
	if (strcmp(self->classname, "bot") == 0) //MATTHIAS
    {
		Bot_Respawn(self);
		return;
    }

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
	int		i,k;
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
	VectorClear (ent->avelocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;
//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID

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
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 255;		// will use the skin specified model
	
	ShowGun(ent);	//vwep

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

	//MATTHIAS
	client->BlindBase = 0;
	client->BlindTime = 0;
	client->PoisonBase = 0;
	client->PoisonTime = 0;

	client->swordstate	= 0;
	client->nextscannercell = 0;
	client->nextbeltcell = 0;
	client->nextvomit = 0;
	client->nextheartbeat = 0;
	client->grenadesactive = 1;
	client->nextrandomsound = level.time + 60 + random() * 60;
	client->b_currentnode	= -1;
	client->b_waittime	= 0;
	client->grapple_state = 0;

	client->fakedeath = 0;
	client->kamikazetime = 0;
	client->scanneractive = 0;
	client->beltactive = 0;
	client->jet_framenum = 0;
    client->jet_remaining = 0;
	client->invisible = 0;
	client->invisible_framenum = 0;
	client->grapple = NULL;

	if (start_invulnerable_time->value > 0)
		client->invincible_framenum = level.framenum + 10 * start_invulnerable_time->value;
	else
		client->invincible_framenum = 0;

	for(k = 0; k < 100; k++)
	{
		client->b_path[k] = -1;
	}

	client->flashlightactive = 0;
	if(client->flashlight)
	{
	  	client->flashlight->think = G_FreeEdict;
		G_FreeEdict(client->flashlight);
	}
	if(client->teleporter)
		G_FreeEdict(client->teleporter);

//ZOID
	if (CTFStartClient(ent))
		return;
//ZOID

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
void ClientBeginDeathmatch (edict_t *ent)
{
	G_InitEdict (ent);

	InitClientResp (ent->client);

	
	// locate ent at a spawn point
	PutClientInServer (ent);
        ent->client->camera = 0;
	ent->client->cammode = 1;
	// locate ent at a spawn point
	PutClientInServer (ent);

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	bprintf2 (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
	
	if (strcmp(ent->classname, "bot") != 0)
		if (!ctf->value)
			gi.centerprintf (ent, motd);//MATTHIAS

	// StdLog - Mark Davies
    sl_LogPlayerConnect( &gi,
                         ent->client->pers.netname,
                         CTFTeamName(ent->client->resp.ctf_team),
                         level.time );

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

        // make sure numplayers >= 0   FWP

	if (numplayers <0)
           numplayers = 0;

	players[numplayers] = ent;	//MATTHIAS
	numplayers++;

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

			bprintf2 (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
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

	// combine name and skin into a configstring
//ZOID
	if (ctf->value)
		CTFAssignSkin(ent, s);
	else
//ZOID
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

	ShowGun(ent);	//vwep
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
//ZOID -- force team join
		ent->client->resp.ctf_team = -1;
//ZOID
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

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int		playernum,i;

	if (!ent->client)
		return;

	sl_LogPlayerDisconnect( &gi, level, ent );	// StdLog - Mark Davies

	//MATTHIAS
	for (i=0; i<numplayers; i++)
		if (players[i] == ent)
			break;

	i++;
	for (; i<numplayers; i++)
		players[i-1] = players[i];

	players[i-1] = NULL;
	//MATTHIAS
	
	numplayers--;
	
	// LETHAL : start
	if ( ent->client->flashlightactive )
        {
        	ent->client->flashlightactive = false;
        
	        if ( ent->client->flashlight )
                	ent->client->flashlight->think = G_FreeEdict;
        }
	// LETHAL : end

	if (ent->client->resp.ctf_team == 1)
		numred--;
	else if (ent->client->resp.ctf_team == 2)
		numblue--;

	bprintf2 (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

//ZOID
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);
//ZOID

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

#define	STATE_TOP			0
#define	STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3

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
	int n1, n2;
	vec3_t	mins = {0, 0, 0};
	vec3_t	maxs = {0, 0, 0};

	level.current_entity = ent;
	client = ent->client;

//CAMERA
	if (ent->client->camera)	//MATTHIAS Camera
    {
        CameraThink(ent,ucmd);
        return;
    }

//DYNAMIC NODE TABLE GENERATION

	if (dntg->value > 0 && strcmp(ent->classname, "player") == 0)
	{
		if (TouchingLadder(ent)
			&& !Bot_FindNode(ent, 60, LADDER_NODE)
			&& !ent->groundentity
			&& !ent->waterlevel
			&& ent->client->ps.pmove.velocity[2] > 20) //check for ladder nodes
		{
			vec3_t org, forward;

			VectorCopy(ent->s.origin, org);

			AngleVectors(ent->s.angles, forward, NULL, NULL);
			VectorMA(org,10,forward,org);

			Bot_PlaceNode(org, LADDER_NODE, 0);
			Bot_CalcNode(ent, numnodes);
		}
		else if (!Bot_FindNode(ent, 120, ALL_NODES))
		{
			trace_t	trace;
			vec3_t	org, dest, spot;

			VectorCopy(ent->s.origin, org);
			VectorCopy(org, dest);
			dest[2] -= 150;

			trace = gi.trace(org, mins, maxs, dest, ent, MASK_SOLID);

			if (trace.ent && strcmp(trace.ent->classname, "func_plat") == 0)
			{
				//we are above of a plat
				//don't drop nodes here
			}
			else
			{
				vec3_t	end;
				trace_t	tr;

				//check if node is in air
				VectorCopy(ent->s.origin, end);
				end[2] -= 40;

				tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_SOLID);

				/*
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BFG_LASER);
				gi.WritePosition (nodes[nindex].origin);
				gi.WritePosition (end);
				gi.multicast (nodes[nindex].origin, MULTICAST_PHS);
				*/

				if (!tr.startsolid && (tr.fraction == 1))
				{
					Bot_PlaceNode(ent->s.origin, INAIR_NODE, 1);
				}
				else
				{
					VectorCopy(ent->s.origin, spot);
				
					if (!(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
					{
						spot[2] += 5;
						Bot_PlaceNode(spot, NORMAL_NODE, 0);
					}
					else
					{
						Bot_PlaceNode(spot, NORMAL_NODE, 1);
					}
				}
				
				Bot_CalcNode(ent, numnodes);
			}	
		}
	}

//DEBUGMODE
	if (client->b_target && level.time > client->b_lastdpath)
	{
		n1 = Bot_FindNodeAtEnt(ent->s.origin);
		n2 = Bot_FindNodeAtEnt(client->b_target->s.origin);
		
		if (n2 == -1)
		{
			nprintf(PRINT_HIGH,"Couldn't find node at target!\n");
		}
		else if (n1 == -1)
		{
			nprintf(PRINT_HIGH,"Couldn't find node at source!\n");
		}
		else if(!Bot_ShortestPath(n1, n2))
		{
			nprintf(PRINT_HIGH,"No path from %d to %d!\n", n1, n2);
		}
		else
		{
			int p = 0;

			do
			{
				if(p > 99)
					break;
				if(path_buffer[p] < 0)
					break;
				if(path_buffer[p] > MAX_NODES-1)
					break;

				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BFG_LASER);
				gi.WritePosition (nodes[path_buffer[p]].origin);
				gi.WritePosition (nodes[path_buffer[p+1]].origin);
				gi.multicast (nodes[path_buffer[p]].origin, MULTICAST_PHS);
				p++;

			} while ((path_buffer[p] != n1) && (path_buffer[p] != -1));

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_LASER);
			gi.WritePosition (ent->s.origin);
			gi.WritePosition (nodes[n1].origin);
			gi.multicast (ent->s.origin, MULTICAST_PHS);

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_LASER);
			gi.WritePosition (ent->client->b_target->s.origin);
			gi.WritePosition (nodes[n2].origin);
			gi.multicast (ent->client->b_target->s.origin, MULTICAST_PHS);


			nprintf(PRINT_HIGH,"Found path from %d to %d!\n", n1, n2);
		}
		client->b_lastdpath	=	level.time + 0.3;
	}
//KAMIKAZE
	if ((client->kamikazetime > 0) && (ent->flags & FL_GODMODE))
	{
		client->kamikazetime = 0;
    }
	if (client->kamikazetime == 1)
	{
		int admg;

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition(ent->s.origin);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		admg = 2 * (ent->client->pers.inventory[ITEM_INDEX(it_rockets)] + ent->client->pers.inventory[ITEM_INDEX(it_grenades)] + ent->client->pers.inventory[ITEM_INDEX(it_homings)]);
		T_RadiusDamage (ent, ent, 300 +admg, NULL, 300 +admg, MOD_KAMIKAZE);

		if (ent->health > 0)
			T_Damage (ent, ent, ent, ent->velocity, ent->s.origin, ent->velocity, 500, 0, DAMAGE_NO_PROTECTION, MOD_KAMIKAZE);

		
		client->kamikazetime = 0;
	}
	else if(client->kamikazetime > 1)
	{
		client->kamikazetime--;
	}

//SCANNER
	if ((level.time > client->nextscannercell) && (client->scanneractive > 0))
	{
		if (ent->client->pers.inventory[ITEM_INDEX(it_cells)] <= 0)
		{
			cprintf2 (ent, PRINT_HIGH, "You don't have enough cells to run your scanner!\n");
			client->scanneractive = 0;
			client->nextscannercell = level.time;
		}
		else
		{
			if (ent->waterlevel >= 3)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/sonar.wav"), 1, ATTN_IDLE, 0);
			else
				gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/radar.wav"), 1, ATTN_IDLE, 0);
			ent->client->pers.inventory[ITEM_INDEX(it_cells)]--;
			client->nextscannercell = level.time + 2;
		}
	}
//ANTI GRAVITY BELT
	if ((level.time > client->nextbeltcell) && (client->beltactive > 0))
	{
		if (ent->client->pers.inventory[ITEM_INDEX(it_cells)] <= 0)
		{
			cprintf2 (ent, PRINT_HIGH, "You don't have enough cells to run your anti gravity belt!\n");
			client->beltactive = 0;
			client->nextbeltcell = level.time;
		}
		else
		{
			ent->client->pers.inventory[ITEM_INDEX(it_cells)]--;
			client->nextbeltcell = level.time + 2;
		}
	}
// INVISIBLE
	if (ent->client->invisible_framenum > level.framenum)
	{
		if ((ent->client->invisible_framenum - level.framenum) == 1)
		{
			ent->client->invisible = 0;
			ent->s.modelindex = 255;
		}
		if (random() < 0.05)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_LASER_SPARKS);
			gi.WriteByte (5);
			gi.WritePosition (tv(ent->s.origin[0],ent->s.origin[1],ent->s.origin[2] + 20));
			gi.WriteDir (ent->velocity);
			gi.WriteByte (0xffffffff);
			gi.multicast (ent->s.origin, MULTICAST_PVS);

			if (random() < 0.3)
			gi.sound(ent, CHAN_VOICE, gi.soundindex("items/invis2.wav"), 1, ATTN_NORM, 0);
		}
	}
//HEARTBEAT
	if (level.time > client->nextheartbeat && ent->health < 40 && ent->health > 0)
	{
		
		gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/heartbeat.wav"), 1, ATTN_IDLE, 0);
		
		client->nextheartbeat = level.time + (ent->health / 15);

		if (client->nextheartbeat < level.time + 0.6)
			client->nextheartbeat = level.time + 0.6;
	}
//RANDOM SNEZZE
	if (level.time > client->nextrandomsound)
	{
		float rn = random();

		if(ent->waterlevel == 3)
		{
			if (rn < 0.5)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/bubbles1.wav"), 1, ATTN_NORM, 0);
			else if (rn >= 0.3 && rn < 0.6)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/bubbles2.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/burp.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			if (rn < 0.5)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/sneeze1.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/sneeze2.wav"), 1, ATTN_NORM, 0);
		}
		
		client->nextrandomsound = level.time + 120 + random() * 120;
	}

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}

	pm_passent = ent;

	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (ent->s.modelindex != 255 && ent->client->invisible != true)	//MATTHIAS
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag)
		client->ps.pmove.pm_type = PM_DEAD;
	else
		client->ps.pmove.pm_type = PM_NORMAL;


	if (ent->client->beltactive > 0)	//MATTHIAS
		client->ps.pmove.gravity = sv_gravity->value * 0.2;
	else if ((ent->client->grapple_state == 2) && (!ent->groundentity))	//grapple hack
		client->ps.pmove.gravity = sv_gravity->value * 0.2;
	else
		client->ps.pmove.gravity = sv_gravity->value;

	if (Jet_Active(ent))	//MATTHIAS
		Jet_ApplyJet( ent, ucmd );

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
		if ( !Jet_Active(ent) || (Jet_Active(ent)&&(fabs((float)pm.s.velocity[i]*0.125) < fabs(ent->velocity[i]))) )	//MATTHIAS
		ent->velocity[i] = pm.s.velocity[i]*0.125;
	}

	VectorCopy (pm.mins, ent->mins);
	VectorCopy (pm.maxs, ent->maxs);

	client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	if (Jet_Active(ent))	//MATTHIAS
         if(pm.groundentity)
           if (Jet_AvoidGround(ent))
             pm.groundentity = NULL;

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


	if (ent->client->fakedeath == 0 || ent->health <= 0)	//MATTHIAS
	{
		client->oldbuttons = client->buttons;
		client->buttons = ucmd->buttons;
		client->latched_buttons |= client->buttons & ~client->oldbuttons;
	}

	// save light level the player is standing on for
	// monster sighting AI
	//ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTON_ATTACK
//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
//ZOID
		)
	{
		if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

//ZOID
	CTFApplyRegeneration(ent);

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
	if (!client->weapon_thunk
//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
//ZOID
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
	/*
	if (!deathmatch->value)
		if (!visible (ent, PlayerTrail_LastSpot() ) )
			PlayerTrail_Add (ent->s.old_origin);
	*/

	client->latched_buttons = 0;
}

