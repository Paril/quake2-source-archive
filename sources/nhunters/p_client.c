#include "g_local.h"
#include "m_player.h"
#include "g_cmd_teleport.h" // ***** NH change ***** Pred teleport.
#include "g_IRgoggles.h" // ***** NH change ***** IR goggles.
#include "g_cmd_observe.h" //***** NH change ***** observer mode.

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
			default:
				if (IsNeutral(self))
					message = "killed itself";
				else if (IsFemale(self))
					message = "killed herself";
				else
					message = "killed himself";
				break;
			}
		}
		if (message)
		{
			gi.bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			// ***** Start of NH changes *****	
			//majoon: the point of this mod is to kill the pred, NOT
			//worry about whether they killed themselves or not...
			//UPDATE: tried that, and turns out that the predators
			//would kill themselves so they could stay predator, so now
			//we punish them for that
			if (self->isPredator)
				self->client->resp.score -= 2;
			self->enemy = NULL;
			self->SuicidePredator = true;
			return;
			// ***** End of NH changes *****

			/* // ***** NH change *****
			if (deathmatch->value)
				self->client->resp.score--;
			self->enemy = NULL;
			return;
			*/ // ***** NH change *****
			
		}

		// ***** Start of NH changes *****
		//majoon: Changed the obituary death words/scoring...if one marine kills
		//another, then he loses 2 frags...if the predator kills a marine, he
		//gets 1 frag. If a marine kills the predator, then he gets 0 frags, but
		//he gets to be the predator.
		// ***** End of NH changes *****		

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
			        // ***** Start of NH changes *****
				if (attacker->isPredator && attacker->client->weapon_overload && !attacker->inWaiting)
				{
					message = "was creamed by";
					message2 = "'s overloaded impulse cannon";
				}
				else
				if (attacker->isPredator && !self->inWaiting)
				{
					message = "burst over";
					message2 = "'s impulse cannon";
				}
				else
				{
			        // ***** End of NH changes *****
				message = "ate";
				message2 = "'s rocket";
				} // ***** NH change *****
				break;
			case MOD_R_SPLASH:
			        // ***** Start of NH changes *****
				if (attacker->isPredator && attacker->client->weapon_overload && !attacker->inWaiting)
				{
					message = "was obliterated by";
					message2 = "'s overloaded impulse cannon";
				}
			        else
			        if (attacker->isPredator && !attacker->inWaiting)
				{
					message = "was smeared by";
					message2 = "'s impulse cannon";
				}
				else
				{
			        // ***** End of NH changes *****
				message = "almost dodged";
				message2 = "'s rocket";
				} // ***** NH change *****
				break;
			case MOD_HYPERBLASTER:
				message = "was melted by";
				message2 = "'s hyperblaster";
				break;
			case MOD_RAILGUN:
			        // ***** Start of NH changes *****
				if (attacker->isPredator && !attacker->inWaiting)
					message = "was sniped by";
				else
			        // ***** End of NH changes *****			  
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
			// ***** Start of NH changes ***** Flare
			case MOD_FLARE:
				message = "gets all flared up over" ;
				message2 = "'s night light" ;
				break ;
			// ***** End of NH changes *****
			}
			if (message)
			{
			  /* // ***** NH change *****

				gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if (deathmatch->value)
				{
					if (ff)
						attacker->client->resp.score--;
					else
						attacker->client->resp.score++;
				}
				return;
			  */ // ***** NH change *****

			  // ***** Start of NH changes ***** big changes..
				//majoon: The new scoring...
			  if (attacker->isPredator && !attacker->inWaiting)
			    {
			      gi.bprintf (PRINT_MEDIUM,
					  "%s %s the predator%s (%s).\n",
					  self->client->pers.netname,
					  message,
					  message2,attacker->client->pers.netname);

			      gi.sound (attacker, CHAN_AUTO,
					gi.soundindex("misc/predscore.wav"),
					1,
					ATTN_NONE,
						  0);
			      self->s.sound = 0;
			      attacker->client->resp.score++;
			    }
			  else if (!attacker->isPredator &&
				   !self->isPredator)
			    {
			      gi.bprintf (PRINT_MEDIUM,
					  "%s killed fellow marine, %s.\n",
					  attacker->client->pers.netname,
					  self->client->pers.netname);
			      self->s.sound = 0;
			      attacker->killedMarines++;
			      attacker->lastMarineKill = level.time;

			      if(getPenalty() &&
			      	(attacker->killedMarines == getPenalty()) )
			      { 
					gi.centerprintf(attacker, "You just received a PENALTY for that!!!\nNow everyone can see you!\n");
			      }
			      else
			      {	
			      	gi.centerprintf(attacker, "Do NOT kill other marines!!!\nRead the console by pressing the ~\n(tilde) key!\n");
			      	gi.cprintf (attacker, PRINT_HIGH, "\nYou're NOT supposed to kill other marines!!!\nKill the predator to become the predator!\nIf you don't get it, read the About page on\nhttp://www.planetquake.com/nighthunnters\n\n");
			      }	

			      if (attacker->killedMarines >=
				  getMaxMarineKill() )
				{
				  gi.bprintf(PRINT_MEDIUM,
					     "%s killed too many fellow marines. Goodbye %s!\n",
					     attacker->client->pers.netname,
					     attacker->client->pers.netname);
				  stuffcmd(attacker, "disconnect\n");
				  
				  attacker->s.sound = 0;
				  attacker->isPredator = false;
				  
				  return;
				}
			      attacker->client->resp.score--;
			    }
			  else
			    {
			      gi.bprintf (PRINT_MEDIUM,
					  "The predator (%s) %s %s%s.\n",
					  self->client->pers.netname,message,
					  attacker->client->pers.netname, message2);
			      gi.bprintf(PRINT_MEDIUM, 
					 "%s is now the predator!\n",
					 attacker->client->pers.netname) ;
			      gi.centerprintf(attacker,
					      "You will become the new predator!\n");
			      
			      //We want the predator to be a marine now
			      //self->isPredator = false;				// Alex removed - moved to end of player_die
					
			      //Gotta make the attacker the predator
			      // attacker->isPredator = true;				// Alex removed - already done in initPred

			      //Reward the killer with 3 frags
			      attacker->client->resp.score += 3;

			      //Remove one frag from the predator...as to discourage
			      //people from PREFERING to be a marine, since kills
			      //are worth more as a marine...
			      self->client->resp.score--;
			      
			      //Aaaaand now, we want to make the marine LOOK like one
			      //gi.configstring(CS_PLAYERSKINS+(self-g_edicts-1),	// Alex removed
			      //		va("%s\\male/nightops",			// Alex removed
			      //		   self->client->pers.netname) ); 	// Alex removed
			      //ClearFlashlight(self);				// Alex removed

			      /*
				if ( self->flashlight ) {
				G_FreeEdict(self->flashlight);
				self->flashlight = NULL;
				}*/
			      
			      //And the predator to look like the predator...
			      // initPredator(attacker); // Alex removed

			      
			    }
			  return;
			  // ***** End of NH changes *****

			  
			}
		}
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

	//***** NH changes *****
	// Don't drop flares.
	if (item == FindItem("Flares"))
	  return ;

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

	char msg[100] ; // ***** NH changes.

	// If waiting to be pred, doesn't matter that player just melted in the lava..
	if(self->inWaiting)
		return;

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

		// ***** Start of NH changes *****
		if(self->isPredator && !self->SuicidePredator)
			self->client->respawn_time = level.time + 3.0;
		else
		// ***** End of NH changes *****
			self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;

		ClientObituary (self, inflictor, attacker);
		//if (!self->isPredator) // ***** NH change ***** Taken out by Alex - dropped in 1.4!  
		        TossClientWeapon (self);
		
		// ***** Start of NH changes *****       
		
		// Drop IR goggles and shut off flashlight
		if (getIREffectTime())
		  deadDropIRgoggles(self) ;
		ClearFlashlight(self);
		ClearOverload(self);
		// ***** End of NH changes *****

		// ***** Start of NH Changes *****
		if (attacker && attacker->client &&
		!attacker->client->resp.score >= fraglimit->value)
		{
		if (deathmatch->value)
			Cmd_Help_f (self);		// show scores 
		}
		// ***** End of NH Changes *****
		// clear inventory
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < game.num_items; n++)
		{
			if (coop->value && itemlist[n].flags & IT_KEY)
				self->client->resp.coop_respawn.inventory[n] = self->client->pers.inventory[n];
			self->client->pers.inventory[n] = 0;
		}


	}


	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;

	if (self->health < -40)
	{	// gib
	
		// ***** Start of NH changes *****	
		if (self->isPredator && !attacker->inWaiting)
		{
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("predator/growl%i.wav", (rand()%2)+1)), 1, ATTN_NORM, 0);
			if(!self->SuicidePredator)
			{
				self->isPredator = false;		// Alex

				//***** NH changes *****
				// New predator switch.

				// If attacker is dead, put them into the
				// server first.
				if (attacker->deadflag != DEAD_NO)
				  PutClientInServer(attacker) ;
				
				switchPredator(attacker) ;

			}
		}
		else
		// ***** End of NH changes *****
	
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

			// ***** Start of NH changes *****	
			if (self->isPredator && !attacker->inWaiting)
			{
				gi.sound (self, CHAN_VOICE, gi.soundindex(va("predator/growl%i.wav", (rand()%2)+1)), 1, ATTN_NORM, 0);
				if(!self->SuicidePredator)
				{
					self->isPredator = false;		// Alex
					//***** NH changes *****
					// New predator switch.
					switchPredator(attacker) ;
				}
			}

			else
			// ***** End of NH changes *****

			gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}
	
	
	self->deadflag = DEAD_DEAD;

	gi.linkentity (self);

		if (self->client->resp.score < getMinScore() )
		{
			gi.bprintf(PRINT_MEDIUM, "%s's score is below %i. Goodbye %s!\n",
			self->client->pers.netname,getMinScore(),self->client->pers.netname);
			stuffcmd(self, "disconnect\n");
		}

}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
// void InitClientPersistant (gclient_t *client) // ***** NH change ******
void InitClientPersistant (edict_t  *ent) // ***** NH change *****
{
	gitem_t		*item;
	gclient_t	*client;	// ***** NH change *****

	client = ent->client; // ***** NH change *****
	
	memset (&client->pers, 0, sizeof(client->pers));

	// ***** Start of NH changes ***** If we're the predator, then we start out with
	//the rocket launcher and railgun


	// Bruce: 
	// This might all be able to go.
	if (ent->isPredator)
	{

	  initPredator(ent) ;

	}
	else
	{
	  
	  //***** NH Changes *****
	  // Adding marine initial weapon
	  gitem_t *initWeapon, *ammo ;

	  // Now check if weapon specified in server config.
	  // We have to find flares anyway.
	  // Regardless of marine starting weapon we need to set the 
	  // number of flares for the marine.
	  item = FindItem("Flares") ;
	  client->pers.inventory[ITEM_INDEX(item)] = getMaxFlares() ;
	  
	  // Max flares.
	  client->pers.max_flares = max_flares->value ;

	  // If flares are set as default weapon, select them
	  if (Q_stricmp(init_marine_weapon->string, "flares") == 0) {

	    initWeapon = item ;
	    client->pers.selected_item = ITEM_INDEX(item) ;
	    client->pers.weapon = item ;
	  }
	  else {

	    // Otherwise, determine default weapon and select that.
	    initWeapon = FindItem(init_marine_weapon->string) ;
	    if (initWeapon == NULL) {
	      client->pers.selected_item = ITEM_INDEX(item);
	      client->pers.weapon = item;
	    }
	    else {
	      client->pers.selected_item = ITEM_INDEX(initWeapon) ;
	      client->pers.weapon = initWeapon ; 
	    
	      // Need to add some ammo.
	      ammo = FindItem(initWeapon->ammo) ;
	      client->pers.inventory[ITEM_INDEX(ammo)] = ammo->quantity ;
	    
	      // Add weapon to inventory.
	      client->pers.inventory[ITEM_INDEX(initWeapon)] = 1 ;
	    }
	  }

	  // Still want the Blaster in inventory though.
	  client->pers.inventory[ITEM_INDEX(FindItem("Blaster"))] = 1 ;
	  
	  client->pers.health			= 100;
	  client->pers.max_health		= 100;

	  client->pers.max_bullets	= 200;
	  client->pers.max_shells		= 100;
	  client->pers.max_rockets	= 50;
	  client->pers.max_grenades	= 50;
	  client->pers.max_cells		= 200;
	  client->pers.max_slugs		= 50;
	  
	  client->pers.connected = true;
	  
	} // ***** NH change *****

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
	// printf(" *** respawn function called by %s\n",self->client->pers.netname);

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
		
		// printf(" *** End of respawn function for %s\n",self->client->pers.netname);
		
		return;
	}
	// printf(" *** End of respawn function (but not deathmatch) for %s\n",self->client->pers.netname);

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


  // Make sure user isn't inWaiting when trying to switch to spectator mode.

  if (ent->client->pers.spectator) {

    char *value = Info_ValueForKey (ent->client->pers.userinfo, "spectator");

    if (ent->inWaiting)
    {
      ent->client->pers.spectator = false;
      gi.WriteByte (svc_stufftext);
      gi.WriteString ("spectator 0\n");
      ent->isObserving = false ; // ***** NH change *****
      gi.unicast(ent, true);
      return;
    }
  }
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
      ent->isObserving = false ; // ***** NH change *****
      gi.unicast(ent, true);
      return;
    }
    
    // count spectators
    for (i = 1, numspec = 0; i <= maxclients->value; i++)
      if (g_edicts[i].inuse && g_edicts[i].client->pers.spectator)
	numspec++;
    
    // if (numspec >= maxspectators->value) {
    if (numspec >= 100) {		// ***** NH Change *****
      gi.cprintf(ent, PRINT_HIGH, "Server spectator limit is full.\n");
      ent->client->pers.spectator = false;
      // reset his spectator var
      gi.WriteByte (svc_stufftext);
      gi.WriteString ("spectator 0\n");
      ent->isObserving = false ; // ***** NH change *****
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

  ent->isObserving = false ;  // ***** NH change *****

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

  if (ent->client->pers.spectator) {
    gi.bprintf (PRINT_HIGH,
		"%s has moved to the sidelines\n", ent->client->pers.netname);
    ent->isObserving = true;

    // Apply any spectator penalties
    applyObservePenalties(ent) ;

    // ***** Start of NH changes *****
    if(ent->isFirstConnect) {
	    // Display MOTD
	    ShowMOTD(ent) ;
	    ent->isFirstConnect=0;
    }

    // ***** End of NH changes *****
  }
  // ***** NH changes *****
  else if (!ent->isFirstConnect) {
    gi.bprintf (PRINT_HIGH,
		"%s joined the hunt\n", ent->client->pers.netname);
    ent->isObserving = false;
    ent->ShowMOTD = 0;
  }

  ClearFlashlight(ent);	// ***** NH change *****

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
	if (deathmatch->value)
	{
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// InitClientPersistant (client); // ***** NH change *****
		InitClientPersistant (ent); // ***** NH change *****
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
	memset (client, 0, sizeof(*client));
	client->pers = saved;

	if (client->pers.health <= 0)
		// InitClientPersistant (client); // ***** NH change *****
		InitClientPersistant (ent); // ***** NH change *****
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
//	ent->model = "players/males/tris.md2";
		
	ent->pain = player_pain;
	ent->die = player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;
	ent->inWaiting = false;         // ***** NH change *****
	
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
		gi.linkentity (ent);
		return;
	} else
		client->resp.spectator = false;

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);

	// ***** NH changes *****
	// Safety Mode
	setSafetyMode(ent) ;

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

  // ***** NH changes *****
  // New NH client connect
  NH_PreConnect(ent) ;
  PutClientInServer(ent) ;
  NH_PostConnect(ent) ;

  if (level.intermissiontime) {
    MoveClientToIntermission (ent);
  }
  else {
    // send effect
    gi.WriteByte (svc_muzzleflash);
    gi.WriteShort (ent-g_edicts);
    gi.WriteByte (MZ_LOGIN);
    gi.multicast (ent->s.origin, MULTICAST_PVS);
    
    // MOTD
    // ShowMOTD(ent) ;
    ent->motd_delay = (int) level.time + 2;
    
  }
  
  gi.bprintf (PRINT_HIGH,
	      "%s entered the game\n", ent->client->pers.netname);
	  
  
  //  } // ***** NH change *****
  
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

  if (deathmatch->value) {
    ClientBeginDeathmatch (ent);
    return;
  }

  // if there is already a body waiting for us (a loadgame), just
  // take it, otherwise spawn one from scratch
  if (ent->inuse == true) {
    // the client has cleared the client side viewangles upon
    // connecting to the server, which is different than the
    // state when the game is saved, so we need to compensate
    // with deltaangles
    for (i=0 ; i<3 ; i++)
      ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
  }
  else {
    // a spawn point will completely reinitialize the entity
    // except for the persistant data that was initialized at
    // ClientConnect() time
    G_InitEdict (ent);
    ent->classname = "player";
    InitClientResp (ent->client);
    PutClientInServer (ent);
  }
  
  if (level.intermissiontime) {
      MoveClientToIntermission (ent);
  }
  else {
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
	// printf("in ClientUserInfoChanges setting to male/grunt on line 1698 in p_client\n");
	}

	// set name
	s = Info_ValueForKey (userinfo, "name");
	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	// set spectator
	s = Info_ValueForKey (userinfo, "spectator");
	// spectators are only supported in deathmatch
	
	if (deathmatch->value && *s && strcmp(s, "0")) {
		ent->client->pers.spectator = true;
	}
	else
		ent->client->pers.spectator = false;

	
	// ***** Start of NH changes *****	
	//majoon: if they're trying to rip off the server by changing skins
	//to something OTHER than what they're allowed...

	// Revamped by Bruce
	// You cannot change the predator skin.
	if (!ent->isPredator) {
	  
	  // Model check
	  checkMarineSkin(ent, userinfo) ;
	}

	// ***** NH changes *****


	// fov
	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		ent->client->ps.fov = 90;
	}
	else
	{
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));

		// ***** Start of NH changes *****	
		// If the client is the marine, and is attempting to use
		// FOV lower than 90, then set it to 90
		// Exception is IR goggles.
		if (ent->client->ps.fov < 90) {
		  // Exception here is the IR goggles.
		  // Don't reset fov when goggles are used.
		  if (ent->client->IRgoggles_framenum <= level.framenum) {

		    if (!ent->isPredator)
			ent->client->ps.fov = 90;
		  }
		}
		// ***** End of NH changes *****

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
	if (SV_FilterPacket(value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}

	// check for a spectator
	value = Info_ValueForKey (userinfo, "spectator");
	if (deathmatch->value && *value && strcmp(value, "0")) {
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

		// if (numspec >= maxspectators->value) {
		if (numspec >= 100) { // ***** NH Change *****
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
		// InitClientPersistant (ent->client); // ***** NH change *****
		InitClientPersistant (ent); // ***** NH change *****
	}

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
	// gi.dprintf ("%s connected\n", ent->client->pers.netname); // ***** NH change *****
	gi.bprintf (PRINT_HIGH, "%s connected\n", ent->client->pers.netname); // ***** NH change *****
	gi.dprintf ("%s connected from IP address: %s\n", 
		ent->client->pers.netname,
		Info_ValueForKey (ent->client->pers.userinfo, "ip"));
		 // ***** NH change *****

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
void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!ent->client)
		return;

	gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

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

	// ***** Start of NH changes ***** Clear flashlight and free pred status
	ClearFlashlight(ent);
	ent->s.sound = 0;
	if (ent->isPredator)
		ent->isPredator = false;
	// ***** End of NH changes *****

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
	
  // ***** Start of NH changes *****	
  // When we start out, we have to have the last marine kill time
  //be OVER 5 minutes...
  if (ent->lastMarineKill == 0)
    ent->lastMarineKill = (level.time - 301);
  // ***** End of NH changes *****


  if (level.intermissiontime) {
    client->ps.pmove.pm_type = PM_FREEZE;
    // can exit intermission after five seconds
    if (level.time > level.intermissiontime + 5.0 
	&& (ucmd->buttons & BUTTON_ANY) )
      level.exitintermission = true;
    return;
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

    //***** NH changes
    // Predator "switch" freeze period.
    if ((ent->isPredator) &&
	(ent->inWaiting)) {

      VectorClear(ent->velocity) ;
      
      // Very important. Turn off prediction when "freezing" someone.
      client->ps.pmove.pm_flags |= PMF_NO_PREDICTION ;

      // Clear client commands.
      ucmd->forwardmove = 0 ;
      ucmd->sidemove = 0 ;
      ucmd->upmove = 0 ;
    }
    
    client->ps.pmove.gravity = sv_gravity->value;
    pm.s = client->ps.pmove;

    for (i=0 ; i<3 ; i++) {
      pm.s.origin[i] = ent->s.origin[i]*8;
      pm.s.velocity[i] = ent->velocity[i]*8;
    }

    if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s))) {
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
    
    for (i=0 ; i<3 ; i++) {

      ent->s.origin[i] = pm.s.origin[i]*0.125;
	      
      // ***** Start of NH changes *****
      // Of course, the predator isn't human, so he
      // has more running/jumping strength
      if (ent->isPredator) {

	if (!ent->inWaiting)
	  ent->velocity[i] = pm.s.velocity[i]*0.1275;
      }
      else
	// ***** End of NH changes *****
	ent->velocity[i] = pm.s.velocity[i]*0.125;
    }

    VectorCopy (pm.mins, ent->mins);
    VectorCopy (pm.maxs, ent->maxs);

    client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
    client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
    client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

    if (ent->groundentity &&
	!pm.groundentity &&
	(pm.cmd.upmove >= 10) &&
	(pm.waterlevel == 0)) {

      // ***** Start of NH changes *****	
      if (ent->isPredator)
	gi.sound(ent, CHAN_VOICE,
		 gi.soundindex("predator/growl_75.wav"), 1, ATTN_NORM, 0);
      else
	// ***** End of NH changes *****
	gi.sound(ent, CHAN_VOICE,
		 gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
      PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
    }

    ent->viewheight = pm.viewheight;
    ent->waterlevel = pm.waterlevel;
    ent->watertype = pm.watertype;
    ent->groundentity = pm.groundentity;
	
    if (pm.groundentity)
      ent->groundentity_linkcount = pm.groundentity->linkcount;
	
    // ***** NH change *****
    // Don't change angle if waiting to be pred       
    if (!ent->inWaiting) {
      
      if (ent->deadflag) {
	client->ps.viewangles[ROLL] = 40;
	client->ps.viewangles[PITCH] = -15;
	client->ps.viewangles[YAW] = client->killer_yaw;
      }
      else {
	VectorCopy (pm.viewangles, client->v_angle);
	VectorCopy (pm.viewangles, client->ps.viewangles);
      }
      
    } // ***** NH change *****
		
    gi.linkentity (ent);

    if (ent->movetype != MOVETYPE_NOCLIP)
      G_TouchTriggers (ent);

    // touch other objects
    for (i=0 ; i<pm.numtouch ; i++) {
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
  if ((client->latched_buttons & BUTTON_ATTACK) &&
      (!ent->inWaiting)) {	 //***** NH changes *****
    if (client->resp.spectator) {

      client->latched_buttons = 0;

      if (client->chase_target) {
	client->chase_target = NULL;
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
      } else {

	//***** NH changes *****
	// If "attacking" AND in spectator mode AND MOTD is still up,
	// then show menu.
	if (ent->ShowMOTD > (int) level.time) 
	  ClearMOTD(ent) ;
	else 
	  //***** End of NH changes *****
	  GetChaseTarget(ent);
      }

    } else if (!client->weapon_thunk) {
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

  // update chase cam if being followed
  for (i = 1; i <= maxclients->value; i++) {
    other = g_edicts + i;
    if (other->inuse && other->client->chase_target == ent)
      UpdateChaseCam(other);
  }
	
  // ***** Start of NH changes *****	

  //majoon: This gives the predator 1 rocket/slug and 10 health every
  //10 seconds
  if ((level.lastBoost <= level.time) &&
      (ent->isPredator) &&
      (!ent->inWaiting) &&
      (!ent->deadflag))	{

    //If the pred's health + 10 is greater than the pred's max,
    //then we just give him max...
    ent->health += 10 ;
    if (ent->health > 200)
      ent->health = 200 ;

    if (client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] < 
	client->pers.max_rockets)
      client->pers.inventory[ITEM_INDEX(FindItem("rockets"))]++ ;

    if (client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] <
	client->pers.max_slugs)
      client->pers.inventory[ITEM_INDEX(FindItem("slugs"))]++ ;
    
    level.lastBoost = level.time + 10;
  }

  //This one is to see when the last time the player killed a fellow
  //marine

  //If the last time the player killed another marine WHILE he was a
  //marine himself was over 150 seconds (3 minutes) ago, then we can
  //reset the marinekills variable. (note: changed. now it's a server
  //var...default to 300 seconds)
  if ((level.time - ent->lastMarineKill) > getMaxTime() )
    ent->killedMarines = 0;

  if ((level.be_pred_time >= (int) level.time) &&
      (ent->isPredator) &&
      (ent->inWaiting)) {

     if (level.be_pred_time -4 == (int) level.time && 
     	level.be_pred_time_temp1 == 0)    {
	
    	//printf("4 seconds left..\n");
    	level.be_pred_time_temp1 = 1;
    }

     if (level.be_pred_time -3 == (int) level.time && 
     	level.be_pred_time_temp2 == 0)    {
    	//printf("3 seconds left..switch to pred model, wave, uncrouch\n");
	//printf("changing skin\n");
	gi.configstring (CS_PLAYERSKINS+(ent-g_edicts-1),
        	         va("%s\\%s", ent->client->pers.netname,
                            getLivePredatorSkin()) );
	//printf("starting wave\n");
	// Taunt frames
        //ent->client->anim_priority = ANIM_WAVE;
        //ent->s.frame = FRAME_taunt01-1;
        //ent->client->anim_end = FRAME_taunt17;
	
	// Salute frames
        //ent->client->anim_priority = ANIM_WAVE;
        //ent->s.frame =  FRAME_salute01-1;
	//ent->client->anim_end = FRAME_salute11;
	
	// Wave frames
        //ent->client->anim_priority = ANIM_WAVE;
	//ent->s.frame = FRAME_wave01-1;
	//ent->client->anim_end = FRAME_wave11;
	
	// Point frames
        //ent->client->anim_priority = ANIM_WAVE;
	//ent->s.frame = FRAME_point01-1;
	//ent->client->anim_end = FRAME_point12;
	
	// Flipoff frames	
        ent->client->anim_priority = ANIM_WAVE;
	ent->s.frame = FRAME_flip01-1;
	ent->client->anim_end = FRAME_flip12;
	//printf("Should do a WAVE..\n");


	//printf("playing sound\n");
	gi.sound (ent, CHAN_VOICE,
	          gi.soundindex ("world/bigpump.wav"), 1, ATTN_NORM, 0);
	
	
	                                
    	level.be_pred_time_temp2 = 1;
    }

     if (level.be_pred_time -2  == (int) level.time && 
     	level.be_pred_time_temp3 == 0)    {
    	//printf("2 seconds left..you are the new predator voice..\n");
    	level.be_pred_time_temp3 = 1;
    	gi.sound (ent, CHAN_VOICE,
    	     gi.soundindex ("misc/newpred.wav"), 1, ATTN_STATIC, 0);  
    	
    }

    
  }	    
  if ((level.time >= level.be_pred_time) && 
  	(level.be_pred_time) && 
  	 (ent->isPredator) &&
  	  (ent->inWaiting))  {

  	level.be_pred_time = 0;
	
	// New predator switch
	startPredator(ent) ;
  }                                                          

  if (( (int) level.time >= ent->motd_delay) &&
  	(ent->motd_delay))
  {
  	ent->motd_delay = 0;
  	if (ent->isObserving)
  		ShowMOTD(ent);
  }

  // ***** End of NH changes *****

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
  char		temp[20];
  
  if (level.intermissiontime)
    return;
  
  client = ent->client;



  // ***** NH changes *****
  // Eliminated redundancies in block below.
  if (deathmatch->value &&
      client->pers.spectator != client->resp.spectator) {

    if ((ent->spectator_quick_switch) ||
	((level.time - client->respawn_time) >= 5)) {
   
      spectator_respawn(ent) ;

      if (ent->spectator_quick_switch) 
	ent->spectator_quick_switch = false ;
    }
  }      

  // run weapon animations if it hasn't been done by a ucmd_t
  if (!client->weapon_thunk && !client->resp.spectator)
    Think_Weapon (ent);
  else
    client->weapon_thunk = false;

  //	if (ent->deadflag)
  // ***** NH change *****
  if (ent->deadflag &&
      !ent->inWaiting &&
      !ent->isObserving) {

    // wait for any button just going down
    if ( level.time > client->respawn_time) {

      // in deathmatch, only wait for attack button

      // ***** NH change *****
      // NH: Also, if you are the predator, respawn automatically 
      if (deathmatch->value)
	buttonMask = BUTTON_ATTACK;
      else
	buttonMask = -1;

      //***** NH change *****
      if ((( client->latched_buttons & buttonMask ) ||
	   (deathmatch->value &&
	    ((int)dmflags->value & DF_FORCE_RESPAWN) )) ||
	  (ent->isPredator && !ent->inWaiting) ) {
	respawn(ent);
	client->latched_buttons = 0;
      }
    }
    else {
    	buttonMask = -1;	// Clear the firebutton if still
    				// client->respawn_time isn't hit
    				// to try to prevent respawn if you fire right
    				// after you die and it gets buffered
				// Alex    
    }

    return;
  }

  // add player trail so monsters can follow
  if (!deathmatch->value)
    if (!visible (ent, PlayerTrail_LastSpot() ) )
      PlayerTrail_Add (ent->s.old_origin);

  client->latched_buttons = 0;

 	if(getMaxRate() > 0)
	{
		//printf("client rate is %i, maxrate is %s\n",atoi(Info_ValueForKey(ent->client->pers.userinfo, "rate")),
		//	maxrate->string);
	   if (atoi(Info_ValueForKey(ent->client->pers.userinfo, "rate")) > maxrate->value)
	   {
 		//printf("1ccc\n");
 		gi.cprintf (ent, PRINT_HIGH, "\nServer enforcing max rate of %s\n",maxrate->string);
		//printf("2ccc\n");
		//Info_SetValueForKey(userinfo, "rate", maxrate->string);
		//printf("3ccc\n");
		Com_sprintf(temp, sizeof(temp),"rate %s\n",maxrate->string);
		stuffcmd(ent, temp);
		//printf("4ccc\n");
	   }
	}


}



