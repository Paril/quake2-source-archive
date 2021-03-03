#include "g_local.h"
#include "m_player.h"

#ifdef BOT
#include "bl_spawn.h"
#include "bl_main.h"
#endif //BOT
#ifdef OBSERVER
#include "p_observer.h"
#endif //OBSERVER

#ifdef CLIENTLAG
#include "p_lag.h"
#endif //CLIENTLAG


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

#ifdef ROGUE
/*QUAKED info_player_coop_lava (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games on rmine2 where lava level
needs to be checked
*/
void SP_info_player_coop_lava(edict_t *self)
{
	if (!coop->value)
	{
		G_FreeEdict (self);
		return;
	}
}
#endif //ROGUE

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
	int mod, i;
	char *message[16];
	char *message2[16];
	qboolean ff;

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	if (deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		for (i = 0; i < 16; i++)
		{
			message[i] = NULL;
			message2[i] = "";
		} //end for
		
		switch (mod)
		{
		case MOD_SUICIDE:
			message[0] = "commits suicide";
			message[1] = "takes the easy way out";
			if (IsNeutral(self))
			{
				message[2] = "has fragged itself";
				message[3] = "took it's own life";
			}
			else if (IsFemale(self))
			{
				message[2] = "has fragged herself";
				message[3] = "took her own life";
			} //end if
			else
			{
				message[2] = "has fragged himself";
				message[3] = "took his own life";
			} //end else
			message[4] = "can be scraped off the pavement";
			break;
		case MOD_FALLING:
			message[0] = "cratered";
			message[1] = "discovers the effects of gravity";
			break;
		case MOD_CRUSH:
			message[0] = "was squished";
			message[1] = "was squeezed like a ripe grape";
			message[2] = "turned to juice";
			break;
		case MOD_WATER:
			message[0] = "sank like a rock";
			message[1] = "tried unsuccesfully to breathe water";
			message[2] = "tried to immitate a fish";
			message[3] = "must learn when to breathe";
			message[5] = "needs to learn how to swim";
			message[6] = "took a long walk of a short pier";
			message[7] = "might want to use a rebreather next time";
			if (IsNeutral(self))
			{
				message[4] = "thought it didn't need a rebreather";
			} //end if
			else if (IsFemale(self))
			{
				message[4] = "thought she didn't need a rebreather";
			} //end if
			else
			{
				message[4] = "thought he didn't need a rebreather";
			} //end else
			break;
		case MOD_SLIME:
			message[0] = "melted";
			message[1] = "was dissolved";
			message[2] = "sucked slime";
			message[3] = "found an alternative way to die";
			message[4] = "needs more slime-resistance";
			message[5] = "might try on an environmental suit next time";
			break;
		case MOD_LAVA:
			message[0] = "does a back flip into the lava";
			message[1] = "was fried to a crisp";
			message[2] = "thought that lava was water";
			message[3] = "turned into a real hothead";
			message[4] = "thought lava was 'funny water'";
			message[5] = "tried to hide in the lava";
			if (IsNeutral(self))
			{
				message[6] = "thought it was fire resistant";
				message[7] = "tried to emulate the demigod";
				message[8] = "needs to rebind it's 'strafe' keys";
			} //end if
			else if (IsFemale(self))
			{
				message[6] = "thought she was fire resistant";
				message[7] = "tried to emulate the goddess Pele";
				message[8] = "needs to rebind her 'strafe' keys";
			} //end if
			else
			{
				message[6] = "thought he was fire resistant";
				message[7] = "tried to emulate the god of hell-fire";
				message[8] = "needs to rebind his 'strafe' keys";
			} //end else
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message[0] = "blew up";
			break;
		case MOD_EXIT:
			message[0] = "found a way out";
			message[1] = "had enough for today";
			message[2] = "exit, stage left";
			message[3] = "has returned to real life(tm)";
			break;
		case MOD_TARGET_LASER:
			message[0] = "saw the light";
			break;
		case MOD_TARGET_BLASTER:
			message[0] = "got blasted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message[0] = "was in the wrong place";
			message[1] = "shouldn't play with equipment";
			message[2] = "can't move around moving objects";
			break;
#ifdef XAXTIX
		// RAFAEL
		case MOD_GEKK:
		case MOD_BRAINTENTACLE:
			message = "that's gotta hurt";
			break;
#endif //XATRIX
		}
		if (attacker == self)
		{
			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message[0] = "tried to put the pin back in";
				message[2] = "got the red and blue wires mixed up";
				if (IsNeutral(self))
				{
					message[1] = "held it's grenade too long";
					message[3] = "tried to disassemble it's own grenade";
				} //end if
				else if (IsFemale(self))
				{
					message[1] = "held her grenade too long";
					message[3] = "tried to disassemble her own grenade";
				} //end if
				else
				{
					message[1] = "held his grenade too long";
					message[3] = "tried to disassemble his own grenade";
				} //end else
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				message[2] = "tried to grenade-jump unsuccessfully";
				message[3] = "tried to play football with a grenade";
				message[4] = "shouldn't mess around with explosives";
				if (IsNeutral(self))
				{
					message[0] = "tripped on it's own grenade";
					message[1] = "stepped on it's own pineapple";
				}
				else if (IsFemale(self))
				{
					message[0] = "tripped on her own grenade";
					message[1] = "stepped on her own pineapple";
				} //end if
				else
				{
					message[0] = "tripped on his own grenade";
					message[1] = "stepped on his own pineapple";
				} //end else
				break;
			case MOD_R_SPLASH:
				message[2] = "knows didley squatt about rocket launchers";
				message[4] = "thought up a novel new way to fly";
				if (IsNeutral(self))
				{
					message[0] = "blew itself up";
					message[1] = "thought it was Werner von Braun";
					message[3] = "thought it had more health";
					message[5] = "found it's own rocketlauncher's trigger";
					message[6] = "thought it had more armor on";
					message[7] = "blew itself to kingdom come";
				} //end if
				else if (IsFemale(self))
				{
					message[0] = "blew herself up";
					message[1] = "thought she was Werner von Braun";
					message[3] = "thought she had more health";
					message[5] = "found her own rocketlauncher's trigger";
					message[6] = "thought she had more armor on";
					message[7] = "blew herself to kingdom come";
				} //end if	
				else
				{
					message[0] = "blew himself up";
					message[1] = "thought he was Werner von Braun";
					message[3] = "thought he had more health";
					message[5] = "found his own rocketlauncher's trigger";
					message[6] = "thought he had more armor on";
					message[7] = "blew himself to kingdom come";
				} //end else
				break;
			case MOD_BFG_BLAST:
				message[0] = "should have used a smaller gun";
				message[1] = "shouldn't play with big guns";
				message[2] = "doesn't know how to work the BFG";
				message[3] = "has trouble using big guns";
				message[4] = "can't distinguish which end is which with the BFG";
				message[5] = "should try to avoid using the BFG near obstacles";
				message[6] = "tried to BFG-jump unsuccesfully";
				break;
#ifdef XATRIX
			// RAFAEL 03-MAY-98
			case MOD_TRAP:
				if (IsNeutral(self))
				 	message[0] = "sucked into it's own trap";
				else if (IsFemale(self))
				 	message[0] = "sucked into her own trap";
				else
				 	message[0] = "sucked into his own trap";
				break;
#endif //XATRIX
#ifdef ROGUE
			case MOD_DOPPLE_EXPLODE:
				if (IsNeutral(self))
					message[0] = "got caught in it's own trap";
				else if (IsFemale(self))
					message[0] = "got caught in her own trap";
				else
					message[0] = "got caught in his own trap";
				break;
#endif //ROGUE
			default:
				message[1] = "commited suicide";
				message[2] = "went the way of the dodo";
				message[3] = "thought 'kill' was a funny console command";
				message[4] = "wanted one frag less";
				if (IsNeutral(self))
				{
					message[0] = "killed itself";
					message[5] = "thought it had one many frags";
				} //end if
				else if (IsFemale(self))
				{
					message[0] = "killed herself";
					message[5] = "thought she had one many frags";
				} //end if
				else
				{
					message[0] = "killed himself";
					message[5] = "thought he had one many frags";
				} //end else
				break;
			} //end switch
		} //end if
		if (message[0])
		{
			for (i = 0; i < 16; i++)
			{
				if (!message[i]) break;
			} //end for
			i = random() * (float) i;
			gi.bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message[i]);
			if (deathmatch->value) self->client->resp.score--;
			self->enemy = NULL;
			return;
		} //end if

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			switch (mod)
			{
				case MOD_BLASTER:
					message[0] = "(quakeweenie) was massacred by";
					message2[0] = " (quakegod)!!!";
					message[1] = "was killed with the wimpy blaster by";
					message[2] = "died a wimp's death by";
					message[3] = "can't even avoid a blaster from";
					message[4] = "was blasted by";
					break;
				case MOD_SHOTGUN:
					message[1] = "was gunned down by";
					if (IsNeutral(self))
					{
						message[0] = "found itself on the wrong end of";
						message2[0] = "'s gun";
					} //end if
					else if (IsFemale(self))
					{
						message[0] = "found herself on the wrong end of";
						message2[0] = "'s gun";
					} //end if
					else
					{
						message[0] = "found himself on the wrong end of";
						message2[0] = "'s gun";
					} //end else
					break;
				case MOD_SSHOTGUN:
					message[0] = "was blown away by";
					message2[0] = "'s super shotgun";
					if (IsNeutral(self))
					{
						message[0] = "had it ears cleaned out by";
						message2[0] = "'s super shotgun";
					} //end if
					else if (IsFemale(self))
					{
						message[0] = "had her ears cleaned out by";
						message2[0] = "'s super shotgun";
					} //end if
					else
					{
						message[0] = "had his ears cleaned out by";
						message2[0] = "'s super shotgun";
					} //end else
					message[3] = "was put full of buckshot by";
					break;
				case MOD_MACHINEGUN:
					message[0] = "was machinegunned by";
					message[1] = "was filled with lead by";
					message[2] = "was put full of lead by";
					message[3] = "was pumped full of lead by";
					message[4] = "ate lead dished out by";
					message[5] = "eats lead from";
					message[6] = "bites the bullet from";
					break;
				case MOD_CHAINGUN:
					message[0] = "was cut in half by";
					message2[0] = "'s chaingun";
					message[2] = "was turned into a strainer by";
					message[3] = "was put full of holes by";
					message[4] = "couldn't avoid death by painless from";
					if (IsNeutral(self))
					{
						message[1] = "was put so full of lead by";
						message2[1] = " you can call it a pencil";
					} //end if
					else if (IsFemale(self))
					{
						message[1] = "was put so full of lead by";
						message2[1] = " you can call her a pencil";
					} //end if
					else
					{
						message[1] = "was put so full of lead by";
						message2[1] = " you can call him a pencil";
					} //end else
					break;
				case MOD_GRENADE:
					message[0] = "was popped by";
					message2[0] = "'s grenade";
					message[1] = "caught";
					message2[1] = "'s grenade in the head";
					message[2] = "tried to headbutt the grenade of";
					break;
				case MOD_G_SPLASH:
					message[0] = "was shredded by";
					message2[0] = "'s shrapnel";
					break;
				case MOD_ROCKET:
					message[0] = "ate";
					message2[0] = "'s rocket";
					message[1] = "sucked on";
					message2[1] = "'s boomstick";
					message[2] = "tried to play 'dodge the missile' with";
					message[3] = "tried the 'patriot move' on the rocket from";
					message[4] = "had a rocket stuffed down the throat by";
					message[5] = "got a rocket up the tailpipe by";
					message[6] = "tried to headbutt";
					message2[6] = "'s rocket";
					break;
				case MOD_R_SPLASH:
					message[0] = "almost dodged";
					message2[0] = "'s rocket";
					message[1] = "was spread around the place by";
					message[2] = "was gibbed by";
					message[3] = "has been blown to smithereens by";
					message[4] = "was blown to itsie bitsie tiny pieces by";
					break;
				case MOD_HYPERBLASTER:
					message[0] = "was melted by";
					message2[0] = "'s hyperblaster";
					message[1] = "was used by";
					message2[1] = " for target practice";
					message[2] = "was hyperblasted by";
					message[3] = "was pumped full of cells by";
					message[4] = "couldn't outrun the hyperblaster from";
					break;
				case MOD_RAILGUN:
					message[0] = "was railed by";
					message[2] = "played 'catch the slug' with";
					message[4] = "bites the slug from";
					message[5] = "caught the slug from";
					if (IsNeutral(self))
					{
						message[1] = "got a slug put through it by";
						message[3] = "was corkscrewed through it's head by";
						message[6] = "had it's body pierced with a slug from";
						message[7] = "had it's brains blown out by";
					} //end if
					else if (IsFemale(self))
					{
						message[1] = "got a slug put through her by";
						message[3] = "was corkscrewed through her head by";
						message[6] = "had her body pierced with a slug from";
						message[7] = "had her brains blown out by";
					} //end if
					else
					{
						message[1] = "got a slug put through him by";
						message[3] = "was corkscrewed through his head by";
						message[6] = "had his body pierced with a slug from";
						message[7] = "had his brains blown out by";
					} //end else
					break;
				case MOD_BFG_LASER:
					message[0] = "saw the pretty lights from";
					message2[0] = "'s BFG";
					message[1] = "was diced by the BFG from";
					break;
				case MOD_BFG_BLAST:
					message[0] = "was disintegrated by";
					message2[0] = "'s BFG blast";
					message[1] = "was flatched with the green light by";
					message2[1] = "";
					break;
				case MOD_BFG_EFFECT:
					message[0] = "couldn't hide from";
					message2[0] = "'s BFG";
					message[1] = "tried to soak up green energy from";
					message2[1] = "'s BFG";
					message[2] = "was energized with 50 cells by";
					message[3] = "doesn't know when to run from";
					message[4] = "'saw the light' from";
					break;
				case MOD_HANDGRENADE:
					message[0] = "caught";
					message2[0] = "'s handgrenade";
					message[1] = "should watch more carefully for handgrenades from";
					message[2] = "caught";
					message2[2] = "'s handgrenade in the head";
					message[3] = "tried to headbutt the handgrenade of";
					break;
				case MOD_HG_SPLASH:
					message[0] = "didn't see";
					message2[0] = "'s handgrenade";
					break;
				case MOD_HELD_GRENADE:
					message[0] = "feels";
					message2[0] = "'s pain";
					break;
				case MOD_TELEFRAG:
					message[0] = "tried to invade";
					message2[0] = "'s personal space";
					message[1] = "is less telefrag aware than";
					message[2] = "should appreciate scotty more like";
					break;
#ifdef XATRIX
				// RAFAEL 14-APR-98
				case MOD_RIPPER:
					message[0] = "ripped to shreds by";
					message2[0] = "'s ripper gun";
					break;
				case MOD_PHALANX:
					message[0] = "was evaporated by";
					break;
				case MOD_TRAP:
					message[0] = "caught in trap by";
					break;
				// END 14-APR-98
#endif //XATRIX
#ifdef ROGUE
				case MOD_CHAINFIST:
					message[0] = "was shredded by";
					message2[0] = "'s ripsaw";
					break;
				case MOD_DISINTEGRATOR:
					message[0] = "lost his grip courtesy of";
					message2[0] = "'s disintegrator";
					break;
				case MOD_ETF_RIFLE:
					message[0] = "was perforated by";
					break;
				case MOD_HEATBEAM:
					message[0] = "was scorched by";
					message2[0] = "'s plasma beam";
					break;
				case MOD_TESLA:
					message[0] = "was enlightened by";
					message2[0] = "'s tesla mine";
					break;
				case MOD_PROX:
					message[0] = "got too close to";
					message2[0] = "'s proximity mine";
					break;
				case MOD_NUKE:
					message[0] = "was nuked by";
					message2[0] = "'s antimatter bomb";
					break;
				case MOD_VENGEANCE_SPHERE:
					message[0] = "was purged by";
					message2[0] = "'s vengeance sphere";
					break;
				case MOD_DEFENDER_SPHERE:
					message[0] = "had a blast with";
					message2[0] = "'s defender sphere";
					break;
				case MOD_HUNTER_SPHERE:
					message[0] = "was killed like a dog by";
					message2[0] = "'s hunter sphere";
					break;
				case MOD_TRACKER:
					message[0] = "was annihilated by";
					message2[0] = "'s disruptor";
					break;
				case MOD_DOPPLE_EXPLODE:
					message[0] = "was blown up by";
					message2[0] = "'s doppleganger";
					break;
				case MOD_DOPPLE_VENGEANCE:
					message[0] = "was purged by";
					message2[0] = "'s doppleganger";
					break;
				case MOD_DOPPLE_HUNTER:
					message[0] = "was hunted down by";
					message2[0] = "'s doppleganger";
					break;
#endif //ROGUE
#ifdef ZOID
				case MOD_GRAPPLE:
					message[0] = "was caught by";
					message2[0] = "'s grapple";
					break;
#endif //ZOID
			}
			if (message[0])
			{
				for (i = 0; i < 16; i++)
				{
					if (!message[i]) break;
				} //end for
				i = (random()-0.01) * (float) i;
				gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message[i], attacker->client->pers.netname, message2[i]);
#ifdef ROGUE
				if (gamerules && gamerules->value)
				{
					if(DMGame.Score)
					{
						if(ff)		
							DMGame.Score(attacker, self, -1);
						else
							DMGame.Score(attacker, self, 1);
					}
					return;
				}
#endif //ROGUE
				if (deathmatch->value)
				{
#ifdef CH
					if (ch->value)
					{
						//if attacker and self have the same color
						if (attacker->client->chcolor == self->client->chcolor) attacker->client->resp.score++;
						else attacker->client->resp.score--;
					} //end if
					else
#endif //CH
					{
						if (ff) attacker->client->resp.score--;
						else attacker->client->resp.score++;
					} //end else
				} //end if
				return;
			} //end if
		}
	}

	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (deathmatch->value)
#ifdef ROGUE
		if (gamerules && gamerules->value)
		{
			if(DMGame.Score)
			{
				DMGame.Score(self, self, -1);
			}
			return;
		}
		else
#endif //ROGUE
			self->client->resp.score--;
}


void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	qboolean	quad;
#ifdef XATRIX
	// RAFAEL
	qboolean	quadfire;
#endif //XATRIX
	float		spread;

	if (!deathmatch->value)
		return;

#ifdef ROCKETARENA
	//don't drop weapons in rocket arena
	if (ra->value) return;
#endif //ROCKETARENA

	item = self->client->pers.weapon;
	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Blaster") == 0))
		item = NULL;

	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else
		quad = (self->client->quad_framenum > (level.framenum + 10));

#ifdef XATRIX
	// RAFAEL
	if (!((int)(dmflags->value) & DF_QUADFIRE_DROP))
		quadfire = false;
	else
		quadfire = (self->client->quadfire_framenum > (level.framenum + 10));
#endif //XATRIX

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
#ifdef XATRIX
	// RAFAEL
	if (quadfire)
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quadfire"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quadfire_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
#endif //XATRIX
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

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
#ifdef ZOID
	self->s.modelindex3 = 0;	// remove linked ctf flag
#endif //ZOID

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
#ifdef ZOID
		if (ctf->value) CTFFragBonuses(self, inflictor, attacker);
#endif //ZOID
		TossClientWeapon (self);
#ifdef ZOID
		if (ctf->value)
		{
			CTFPlayerResetGrapple(self);
			CTFDeadDropFlag(self);
			CTFDeadDropTech(self);
		} //end if
#endif //ZOID
		if (deathmatch->value)
#ifdef BOT
			if (!(self->flags & FL_BOT))
#endif //BOT
				Cmd_Help_f (self);		// show scores

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

#ifdef XATRIX
	// RAFAEL
	self->client->quadfire_framenum = 0;
#endif //XATRIX

#ifdef ROGUE
	if (rogue->value)
	{
		self->client->double_framenum = 0;

		// if there's a sphere around, let it know the player died.
		// vengeance and hunter will die if they're not attacking,
		// defender should always die
		if(self->client->owned_sphere)
		{
			edict_t *sphere;

			sphere = self->client->owned_sphere;
			sphere->die(sphere, self, self, 0, vec3_origin);
		}

		// if we've been killed by the tracker, GIB!
		if((meansOfDeath & ~MOD_FRIENDLY_FIRE) == MOD_TRACKER)
		{
			self->health = -100;
			damage = 400;
		}

		// make sure no trackers are still hurting us.
		if(self->client->tracker_pain_framenum)
		{
			RemoveAttackingPainDaemons (self);
		}
	
		// if we got obliterated by the nuke, don't gib
		if ((self->health < -80) && (meansOfDeath == MOD_NUKE))
			self->flags |= FL_NOGIB;
	} //end if

#endif //ROGUE

	if (self->health < -40)
	{	// gib
#ifdef ROGUE
		if (rogue->value)
		{
			// PMM
			// don't toss gibs if we got vaped by the nuke
			if (!(self->flags & FL_NOGIB))
			{
				// pmm
				// gib
				gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
				
				// more meaty gibs for your dollar!
				if((deathmatch->value) && (self->health < -80))
				{
					for (n= 0; n < 4; n++)
						ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
				}

				for (n= 0; n < 4; n++)
					ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
			// PMM
			}
			self->flags &= ~FL_NOGIB;
			// pmm
		} //end if
		else
#else //ROGUE
		{
			gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
			for (n= 0; n < 4; n++)
				ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		}
#endif //ROGUE
		ThrowClientHead (self, damage);

#ifdef ZOID
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = 0;
#endif //ZOID
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

	memset (&client->pers, 0, sizeof(client->pers));

	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	client->pers.weapon = item;
#ifdef ZOID
	client->pers.lastweapon = item;

	if (ctf->value)
	{
		item = FindItem("Grapple");
		if (item) client->pers.inventory[ITEM_INDEX(item)] = 1;
	} //end if
#endif //ZOID

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 50;

#ifdef XATRIX
	// RAFAEL
	client->pers.max_magslug	= 50;
	client->pers.max_trap		= 5;
#endif //XATRIX
#ifdef ROGUE
	// FIXME - give these real numbers....
	client->pers.max_prox		= 50;
	client->pers.max_tesla		= 50;
	client->pers.max_flechettes = 200;
#ifndef KILL_DISRUPTOR
	client->pers.max_rounds     = 100;
#endif
#endif //ROGUE

	client->pers.connected = true;
}


void InitClientResp (gclient_t *client)
{
#ifdef ZOID
	int ctf_team = client->resp.ctf_team;
#endif //ZOID
#ifdef ROCKETARENA
	int context = client->resp.context;
#endif //ROCKETARENA

	memset (&client->resp, 0, sizeof(client->resp));

#ifdef ZOID
	client->resp.ctf_team = ctf_team;
#endif //ZOID

#ifdef ROCKETARENA
	client->resp.context = context;
#endif //ROCKETARENA

	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;

#ifdef ZOID
	if (ctf->value && client->resp.ctf_team < CTF_TEAM1) CTFAssignTeam(client);
#endif //ZOID
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

#ifdef ROGUE
edict_t *SelectLavaCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	float	lavatop;
	edict_t	*lava;
	edict_t *pointWithLeastLava;
	float	lowest;
	edict_t *spawnPoints [64];
	vec3_t	center;
	int		numPoints;
	edict_t *highestlava;

	lavatop = -99999;
	highestlava = NULL;

	// first, find the highest lava
	// remember that some will stop moving when they've filled their
	// areas...
	lava = NULL;
	while (1)
	{
		lava = G_Find (lava, FOFS(classname), "func_door");
		if(!lava)
			break;
		
		VectorAdd (lava->absmax, lava->absmin, center);
		VectorScale (center, 0.5, center);

		if(lava->spawnflags & 2 && (gi.pointcontents(center) & MASK_WATER))
		{
			if (lava->absmax[2] > lavatop)
			{
				lavatop = lava->absmax[2];
				highestlava = lava;
			}
		}
	}

	// if we didn't find ANY lava, then return NULL
	if (!highestlava)
		return NULL;

	// find the top of the lava and include a small margin of error (plus bbox size)
	lavatop = highestlava->absmax[2] + 64;

	// find all the lava spawn points and store them in spawnPoints[]
	spot = NULL;
	numPoints = 0;
	while(spot = G_Find (spot, FOFS(classname), "info_player_coop_lava"))
	{
		if(numPoints == 64)
			break;

		spawnPoints[numPoints++] = spot;
	}

	if(numPoints < 1)
		return NULL;

	// walk up the sorted list and return the lowest, open, non-lava spawn point
	spot = NULL;
	lowest = 999999;
	pointWithLeastLava = NULL;
	for (index = 0; index < numPoints; index++)
	{
		if(spawnPoints[index]->s.origin[2] < lavatop)
			continue;

		if(PlayersRangeFromSpot(spawnPoints[index]) > 32)
		{
			if(spawnPoints[index]->s.origin[2] < lowest)
			{
				// save the last point
				pointWithLeastLava = spawnPoints[index];
				lowest = spawnPoints[index]->s.origin[2];
			}
		}
	}

	// FIXME - better solution????
	// well, we may telefrag someone, but oh well...
	if(pointWithLeastLava)
		return pointWithLeastLava;

	return NULL;
}
#endif //ROGUE

edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

#ifdef ROGUE
	// rogue hack, but not too gross...
	if (!Q_stricmp(level.mapname, "rmine2p") || !Q_stricmp(level.mapname, "rmine2"))
		return SelectLavaCoopSpawnPoint (ent);
#endif //ROGUE

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
#ifdef ZOID
		if (ctf->value)
			spot = SelectCTFSpawnPoint(ent);
		else
#endif //ZOID
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

#ifdef OBSERVER
	if (ent->client) ent->goalentity = body;
#endif //OBSERVER

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
		// spectator's don't leave bodies
		if (self->movetype != MOVETYPE_NOCLIP)
			CopyToBodyQue (self);
		self->svflags &= ~SVF_NOCLIENT;
		PutClientInServer(self);

#ifdef ROCKETARENA
		if (!ra->value)
#endif //ROCKETARENA
		{
			// add a teleportation effect
			self->s.event = EV_PLAYER_TELEPORT;

			// hold in place briefly
			self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
			self->client->ps.pmove.pm_time = 14;

			self->client->respawn_time = level.time;
		}

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
	int		index, i;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	client_persistant_t	saved;
	client_respawn_t	resp;
#ifdef CH
	int chcolor;
#endif //CH

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
#ifdef ROGUE
	if(gamerules && gamerules->value && DMGame.SelectSpawnPoint)		// PGM
		DMGame.SelectSpawnPoint (ent, spawn_origin, spawn_angles);		// PGM
	else																// PGM
#endif //ROGUE
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
#ifdef CH
		chcolor = client->chcolor;
#endif //CH
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

#ifdef ROGUE
	ent->flags &= ~FL_SAM_RAIMI;		// PGM - turn off sam raimi flag
#endif //ROGUE

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

#ifdef ZOID
	if (ctf->value)
	{
		client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	} //end if
#endif //ZOID

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

//PGM
	if (client->pers.weapon)
		client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);
	else 
		client->ps.gunindex = 0;
//PGM

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

#ifdef ZOID
	if (ctf->value)
	{
		if (CTFStartClient(ent)) return;
	} //end if
#endif //ZOID

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

#ifdef ROCKETARENA
	if (ra->value)
	{
		RA2_MoveToArena(ent, ent->client->resp.context, true);
	} //end if
#endif //ROCKETARENA
	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);

#ifdef CH
	client->chcolor = chcolor;
#endif //CH
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

#ifdef ROGUE
	if(gamerules && gamerules->value && DMGame.ClientBegin)	
	{
		DMGame.ClientBegin (ent);
	}
#endif //ROGUE

	// locate ent at a spawn point
	PutClientInServer (ent);

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

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

#ifdef CLIENTLAG
	Lag_BeginGame(ent);
#endif //CLIENTLAG

	ent->client = game.clients + (ent - g_edicts - 1);

#ifdef CTF_HOOK
	if (!(ent->flags & FL_BOT))
	{
		stuffcmd(ent, "alias +hook hookon\n");
		stuffcmd(ent, "alias -hook hookoff\n");
	} //end if
#endif //CTF_HOOK

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

	// set spectator
	s = Info_ValueForKey (userinfo, "spectator");
	// spectators are only supported in deathmatch
	if (deathmatch->value && *s && strcmp(s, "0"))
		ent->client->pers.spectator = true;
	else
		ent->client->pers.spectator = false;

	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;

	// combine name and skin into a configstring
#ifdef ZOID
	if (!ctf->value)
#endif //ZOID
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

#ifdef ZOID
	//NOTE: set the CTF skin after copying the userinfo to the client->pers.userinfo
	// otherwise the client->pers.userinfo might contain the wrong skin
	// the CTF skin is stored in the client->pers.userinfo in the function CTFAssignSkin
	if (ctf->value)
	{
		CTFAssignSkin(ent, Info_ValueForKey(userinfo, "skin"));
	} //end if
#endif //ZOID

#ifdef BOT
	BotLib_BotClientSettings(ent);
#endif //BOT
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

#ifdef BOT
	if (ent->flags & FL_BOT)
	{
		if (!BotMoveToFreeClientEdict(ent)) return false;
	} //end if
#endif //BOT

#ifdef OBSERVER
	//never connect as an observer
	ent->flags &= ~FL_OBSERVER;
#endif //OBSERVER

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
#ifdef ZOID //-- force team join
		ent->client->resp.ctf_team = -1;
#endif //ZOID
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
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
void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!ent->client)
		return;

	gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

#ifdef ROGUE
	// make sure no trackers are still hurting us.
	if(ent->client->tracker_pain_framenum)
		RemoveAttackingPainDaemons (ent);

	if (ent->client->owned_sphere)
	{
		if(ent->client->owned_sphere->inuse)
			G_FreeEdict (ent->client->owned_sphere);
		ent->client->owned_sphere = NULL;
	}

	if (gamerules && gamerules->value)
	{
		if(DMGame.PlayerDisconnect)
			DMGame.PlayerDisconnect(ent);
	}
#endif //ROGUE

#ifdef ZOID
	if (ctf->value)
	{
		CTFDeadDropFlag(ent);
		CTFDeadDropTech(ent);
	} //end if
#endif //ZOID

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	//id bug fix... otherwise the player prediction can get screwed
	ent->svflags |= SVF_NOCLIENT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");

#ifdef BOT
	strcpy(ent->client->pers.netname, "");
	Info_SetValueForKey(ent->client->pers.userinfo, "skin", "");
	BotLib_BotClientSettings(ent);
#endif //BOT
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

#ifdef CTF_HOOK
void CTFHook_Fire (edict_t *ent);
#endif

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink(edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j;
	pmove_t	pm;

#ifdef BOT
	if (paused)
	{
		gi.centerprintf(ent, "GAME PAUSED\n\n(type \"botpause\" to resume)");
		ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
		return;
	} //end if
	//if this is a bot
	if (ent->flags & FL_BOT)
	{
		//if the bot input flag isn't set
		if (!(ent->flags & FL_BOTINPUT)) return;
	} //end if
#endif //BOT

//	{
//		vec3_t goal = {0, 0, 0};
//		ShowGPSText(ent, goal);
//	}

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

	pm_passent = ent;

	if (ent->client->chase_target)
	{
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
	}
	else
	{
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

#ifdef ROGUE
		client->ps.pmove.gravity = sv_gravity->value * ent->gravity;
#else //ROGUE
		client->ps.pmove.gravity = sv_gravity->value;
#endif //ROGUE

#ifdef BOT
		DoMenu(ent, ucmd);
#endif //BOT
#ifdef OBSERVER
		DoObserver(ent, ucmd);
#endif //OBSERVER

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

#ifdef ROGUE //sam raimi cam support
		if(ent->flags & FL_SAM_RAIMI)
			ent->viewheight = 8;
		else
#endif //ROGUE
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

#ifdef CTF_HOOK
		if (client->ctf_hookstate & CTF_HOOK_STATE_ON) CTFHook_Fire(ent);
#endif //CTF_HOOK
#ifdef ZOID
		if (client->ctf_grapple) CTFGrapplePull(client->ctf_grapple);
#endif //ZOID

		gi.linkentity (ent);

#ifdef ROGUE
//PGM trigger_gravity support
		ent->gravity = 1.0;
#endif //ROGUE

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

#ifdef CLIENTLAG
	{
		usercmd_t laggeducmd;
		vec3_t v_angle, origin;

		VectorCopy(ent->s.origin, origin);
		VectorCopy(ent->client->v_angle, v_angle);
		Lag_StoreClientInput(ent, ucmd, ent->s.origin, ent->client->v_angle);
		ucmd = &laggeducmd;
		while(Lag_GetClientInput(ent, ucmd, ent->s.origin, ent->client->v_angle))
		{
#endif //CLIENTLAG

			client->oldbuttons = client->buttons;
			client->buttons = ucmd->buttons;
			client->latched_buttons |= client->buttons & ~client->oldbuttons;

			// save light level the player is standing on for
			// monster sighting AI
			ent->light_level = ucmd->lightlevel;

			// fire weapon from final position if needed
			if (client->latched_buttons & BUTTON_ATTACK
#ifdef OBSERVER
				&& !(ent->flags & FL_OBSERVER)
#endif //OBSERVER
				&& ent->movetype != MOVETYPE_NOCLIP
#ifdef ROCKETARENA
				&& !(ra->value && ent->takedamage == DAMAGE_NO)
#endif //ROCKETARENA
				)
			{
				if (client->resp.spectator)
				{
					client->latched_buttons = 0;
					if (client->chase_target)
					{
						client->chase_target = NULL;
						client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
					} else
						GetChaseTarget(ent);
				} //end if
				else if (!client->weapon_thunk)
				{
					client->weapon_thunk = true;
					Think_Weapon(ent);
				} //end else if
			} //end if
#ifdef CLIENTLAG
		} //end while
		VectorCopy(v_angle, ent->client->v_angle);
		VectorCopy(origin, ent->s.origin);
	}
#endif //CLIENTLAG

	if (client->resp.spectator)
	{
		if (ucmd->upmove >= 10)
		{
			if (!(client->ps.pmove.pm_flags & PMF_JUMP_HELD))
			{
				client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
				if (client->chase_target)
					ChaseNext(ent);
				else
					GetChaseTarget(ent);
			}
		}
		else
			client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
	}

	// update chase cam if being followed
	for (i = 1; i <= maxclients->value; i++)
	{
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}

#ifdef ZOID
	if (ctf->value)
	{
		//regen tech
		CTFApplyRegeneration(ent);
	} //end if
#endif //ZOID

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

	if (deathmatch->value &&
		client->pers.spectator != client->resp.spectator &&
		(level.time - client->respawn_time) >= 5) {
		spectator_respawn(ent);
		return;
	}

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk && !client->resp.spectator
#ifdef OBSERVER
		&& !(ent->flags & FL_OBSERVER)
#endif //OBSERVER
		&& ent->movetype != MOVETYPE_NOCLIP
#ifdef ROCKETARENA
		&& !(ra->value && ent->takedamage == DAMAGE_NO)
#endif //ROCKETARENA
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
				(deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) ))
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

#ifdef ROGUE
/*
==============
RemoveAttackingPainDaemons

This is called to clean up the pain daemons that the disruptor attaches
to clients to damage them.
==============
*/
void RemoveAttackingPainDaemons (edict_t *self)
{
	edict_t *tracker;

	tracker = G_Find (NULL, FOFS(classname), "pain daemon");
	while(tracker)
	{
		if(tracker->enemy == self)
			G_FreeEdict(tracker);
		tracker = G_Find (tracker, FOFS(classname), "pain daemon");
	}

	if(self->client)
		self->client->tracker_pain_framenum = 0;
}

#endif //ROGUE
