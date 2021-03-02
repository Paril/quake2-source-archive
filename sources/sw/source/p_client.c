#include "g_local.h"
#include "m_player.h"

void falling (edict_t *ent);
void deadboy (edict_t *ent);
void deadboy2 (edict_t *ent);
void ddc (edict_t *ent);

void spawn_ghost (edict_t *self)
{
	edict_t *ghost;
	vec3_t	vel;

	ghost = G_Spawn();
	VectorScale (self->velocity, 0.1, vel);
	VectorSubtract (self->s.origin, vel, ghost->s.origin);
	VectorCopy (self->s.angles, ghost->s.angles);

	ghost->movetype = MOVETYPE_NOCLIP;
	ghost->solid = SOLID_NOT;
	ghost->s.effects |= EF_SPHERETRANS;
	ghost->s.modelindex = self->s.modelindex;
	ghost->owner = self;

	ghost->s.frame = self->s.frame;

	ghost->nextthink = level.time + trailtime->value;
	ghost->think = G_FreeEdict;
                
	gi.linkentity (ghost);
}

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
			if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "suicides";
				else
					message = "knocked himself off";
			}
			else if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "did it on purpose. Honest.";
				else
					message = "is having an out of body experience";
			}
			else
				message = "spontaniously self combusts";
			break;
		case MOD_FALLING:
			if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "cratered";
				else 
					message = "can't feel his legs";
			}
			else if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "is looking at things from a new perspective";
				else
					message = "is 2 inches tall";
			}
			else
				message = "fell from grace";
			break;
		case MOD_CRUSH:
			if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "was squished";
				else
					message = "got thin quick";
			}
			else if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "is showing agoraphobic tendancies";
				else
					message = "is in a jam";
			}
			else
				message = "didn't escape from the garbage compressor";
			break;
		case MOD_WATER:
			if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "sank like a rock";
				else
					message = "sank like an Ewok";
			}
			else if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "ate just before bathing";
				else
					message = "is keeping Ackbar company";
			}
			else
				message = "is experiencing flow";
			break;
		case MOD_SLIME:
			if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "melted";
				else
					message = "learned not to swallow acid";
			}
			else if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "could be poured into a glass";
				else
					message = "finished chemistry 101";
			}
			else
				message = "learned the dangers of chemical warfare";
			break;
		case MOD_LAVA:
			if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "did a backflip into the lava";
				else
					message = "found the downside to geothermal energy";
			}
			else if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "is burning with desire";
				else
					message = "learned not to play with fire";
			}
			else
				message = "went AGWAR";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "blew up";
				else
					message = "is decorating the walls";
			}
			else if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "is spreading himself thin";
				else
					message = "went out with a bang";
			}
			else
				message = "got his stupid self blown up";
			break;
		case MOD_EXIT:
			if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "found a way out";
				else
					message = "chickened out";
			}
			else if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "saw his boss coming";
				else
					message = "got scared";
			}
			else
				message = "is lost to us";
			break;
		case MOD_TARGET_LASER:
			if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "saw the light";
				else
					message = "was always a bright student";
			}
			else if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "learned not to stare at bright lights";
				else
					message = "has a few photons to spare";
			}
			else
				message = "stopped playing with his laser pen";
			break;
		case MOD_TARGET_BLASTER:
			if(random() > 0.5)
				message = "got blasted";
			else if(random() > 0.5)
				message = "chews plasma";
			else
				message = "got blasta mastad";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "was in the wrong place";
				else
					message = "got a booboo";
			}
			else if(random() > 0.5)
			{
				if(random() > 0.5)
					message = "wants a refund";
				else
					message = "was pre-emptivly struck";
			}
			else
				message = "was hit by a low yield nuclear chicken";
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
			case MOD_THERMAL:
				if (IsNeutral(self))
				{
					if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "tripped on it's own thermal detonator";
						else
							message = "made itself dissapear";
					}
					else if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "tried bluffing Jabba for a little too long";
						else
							message = "wanted the thermal detonator back";
					}
					else
						message = "found out that what it picked up was NOT a baseball";
				}
				else if (IsFemale(self))
				{
					if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "tripped on her own thermal detonator";
						else
							message = "made herself dissapear";
					}
					else if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "tried bluffing Jabba for a little too long";
						else
							message = "wanted the thermal detonator back";
					}
					else
						message = "found out the hard way that what she picked up was NOT a remote";
				}
				else
				{
					if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "tripped on his own thermal detonator";
						else
							message = "made himself dissapear";
					}
					else if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "tried bluffing Jabba for a little too long";
						else
							message = "wanted the thermal detonator back";
					}
					else
						message = "cfound out the hard way that what he picked up was NOT a remote";
				}
				break;
			case MOD_MISSILETUBE:
				if (IsNeutral(self))
				{
					if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "blew itself up";
						else
							message = "aimed at it's feet";
					}
					else if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "held the missile tube backwards";
						else
							message = "self combusted";
					}
					else
						message = "wanted it's rocket back";
				}
				else if (IsFemale(self))
				{
					if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "blew herself up";
						else
							message = "aimed at her feet";
					}
					else if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "held the missile tube backwards";
						else
							message = "self combusted";
					}
					else
						message = "wanted her rocket back";
				}
				else
				{
					if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "blew himself up";
						else
							message = "aimed at his feet";
					}
					else if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "held the missile tube backwards";
						else
							message = "self combusted";
					}
					else
						message = "wanted his rocket back";
				}
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
			safe_bprintf(PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
				self->client->resp.score--;
			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			switch (mod)
			{
			case MOD_SABER:
				if (IsFemale(self))
				{
					if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "was sliced in two by";
						else
						{
							message = "was reminded by";
							message2 = " of her wedding night";
						}
					}
					else if(random() > 0.5)
					{
						if(random() > 0.5)
						{
							message = "learned";
							message2 = "'s piercing technique";
						}
						else
						{
							message = "is having an out of limbs experience,";
							message2 = " is helping";
						}
					}
					else
					{
						message = "gave";
						message2 = " a hand, among other things";
					}
				}
				else if (IsNeutral(self))
				{
					if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "was sliced in two by";
						else
						{
							message = "failed";
							message2 = "'s fencing 101";
						}
					}
					else if(random() > 0.5)
					{
						if(random() > 0.5)
						{
							message = "is having an out of limbs experience,";
							message2 = " is helping";
						}
						else
						{
							message = "tried to grab ";
							message2 = "'s shiny stick";
						}
					}
					else
					{
						message = "gave";
						message2 = " a hand, among other things";
					}
				}
				else
				{
					if(random() > 0.5)
					{
						if(random() > 0.5)
							message = "was sliced in two by";
						else
						{
							message = "was taught by";
							message2 = " not to bend after the soap";
						}
					}
					else if(random() > 0.5)
					{
						if(random() > 0.5)
						{
							message = "is having an out of limbs experience,";
							message2 = " is helping";
						}
						else
						{
							message = "tried to grab ";
							message2 = "'s shiny stick";
						}
					}
					else
					{
						message = "gave ";
						message2 = " a hand, among other things";
					}
				}
				break;
			case MOD_BLASTER:
				if(random() > 0.5)
				{
					if(random() > 0.5)
						message = "got blasted by";
					else
					{
						message = "chews";
						message2 = "'s plasma";
					}
				}
				else if(random() > 0.5)
				{
					if(random() > 0.5)
						message = "got blasta mastad by";
					else
						message = "was humiliated by";
				}
				else
				{
					message = "was blistered by";
					message2 = "'s blaster";
				}
				break;
			case MOD_RIFLE:
				if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "was actually hit by";
						message2 = "'s trooper rifle";
					}
					else
					{
						message = "is riddled with";
						message2 = "'s holes";
					}
				}
				else if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "re-evaluates";
						message2 = "'s shooting skills";
					}
					else
					{
						message = "wishes";
						message2 = " wouldn't shoot so fast";
					}
				}
				else
				{
					message = "was energized by";
					message2 = "'s trooper rifle";
				}
				break;
			case MOD_REPEATER:
				if(random() > 0.5)
				{
					if(random() > 0.5)
						message = "was repeatedly hit by";
					else
					{
						message = "thinks";
						message2 = " is getting repeticious";
					}
				}
				else if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = " learns that quality looses out to";
						message2 = "'s quantity";
					}
					else
					{
						message = "wants";
						message2 = " to stop hurting him";
					}
				}
				else
				{
					message = "dances to";
					message2 = "'s rythm";
				}
				break;
			case MOD_DISRUPTOR:
				if(random() > 0.5)
				{
					if(random() > 0.5)
						message = "got disrupted by";
					else
					{
						message = "wonders if";
						message2 = " can turn the lights back on";
					}
				}
				else if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "was put to sleep by";
						message2 = "'s portable sandman";
					}
					else
					{
						message = "saw";
						message2 = "'s black angel of death";
					}
				}
				else
				{
					message = "tried to parry";
					message2 = "'s disruptor blast";
				}
				break;
			case MOD_BOWCASTER:
				if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "was pierced by";
						message2 = "'s bowcaster";
					}
					else
					{
						message = "learns not to argue with";
						message2 = "'s wookie";
					}
				}
				else if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "tries to pull";
						message2 = "'s bowcaster bolt out of his back";
					}
					else
					{
						message = "had";
						message2 = " teach him another way to die";
					}
				}
				else
				{
					message = "borrowed";
					message2 = "'s spare bowcaster bolt";
				}
				break;
			case MOD_THERMAL:
				if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "was vaporised";
						message2 = "'s shrapnel";
					}
					else
					{
						message = "swallowed";
						message2 = "'s thermal detonator";
					}
				}
				else if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "tried to throw back";
						message2 = "'s thermal detonator";
					}
					else
					{
						message = "found out why ";
						message2 = "'s ball was blinking so fast";
					}
				}
				else
				{
					message = "was cremated by";
					message2 = "'s thermal detonator";
				}
				break;
			case MOD_WRISTROCKET:
				if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "ate";
						message2 = "'s mini rocket";
					}
					else
					{
						message = "tried to catch";
						message2 = "'s mini rocket";
					}
				}
				else if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "tried to grab";
						message2 = "'s boba fett souvenir";
					}
					else
					{
						message = "was overwhelmed by";
						message2 = "'s mini rockets";
					}
				}
				else
				{
					message = "learned that";
					message2 = "'s mini rockets are just as bad as the big ones";
				}
				break;
			case MOD_MISSILETUBE:
				if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "almost dodged";
						message2 = "'s rocket";
					}
					else
					{
						message = "failed";
						message2 = "'s rocket science 101";
					}
				}
				else if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "tried to catch";
						message2 = "'s rocket to the stars";
					}
					else
					{
						message = "saw the license plate on";
						message2 = "'s rocket";
					}
				}
				else
				{
					message = "had his horizons widened by";
					message2 = "'s rocket";
				}
				break;
			case MOD_BEAMTUBE:
				if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "was melted by";
						message2 = "'s beamtube";
					}
					else
					{
						message = "thinks";
						message2 = " is a big meanie";
					}
				}
				else if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "was beamed up by";
						message2 = "'s beam tube";
					}
					else
					{
						message = "wonder's where";
						message2 = " got the oversized lightsaber";
					}
				}
				else
				{
					message = "was dominated by";
					message2 = "'s beamtube";
				}
				break;
			case MOD_NIGHTSTINGER:
				if(random() > 0.5)
				{
					if(random() > 0.5)
						message = "was picked off by";
					else
					{
						message = "didn't see";
						message2 = "'s nightstinger trail untill it was too late";
					}
				}
				else if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "watches";
						message2 = "'s nightstinger trail go through his chest";
					}
					else
					{
						message = "learned the dangers of";
						message2 = "'s modern warfare";
					}
				}
				else
				{
					message = "got a long distance call from";
					message2 = "'s nightstinger";
				}
				break;
			case MOD_CHOKE:
				if(random() > 0.5)
				{
					if(random() > 0.5)
						message = "had his throat crushed by";
					else
					{
						message = "wasn't allowed to borrow";
						message2 = "'s astma medication";
					}
				}
				else if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "got his larynx crushed by";
						message2 = "'s telekinetics";
					}
					else
					{
						message = "was forgiven by";
						message2 = ", Darthy style";
					}
				}
				else
					message = "had his throat crushed by";
				break;
			case MOD_LIGHTNING:
				if(random() > 0.5)
				{
					if(random() > 0.5)
						message = "was fried by";
					else
					{
						message = "wonders why";
						message2 = "couldn't just touch a radiator";
					}
				}
				else if(random() > 0.5)
				{
					if(random() > 0.5)
					{
						message = "learned that";
						message2 = "'s lightning -can- strike twice";
					}
					else
					{
						message = "was tutored by";
						message2 = " on the true power of the darkside";
					}
				}
				else
					message = "got zapped by";
				break;
			}
			if (message)
			{
				safe_bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if (deathmatch->value)
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

	safe_bprintf (PRINT_MEDIUM,"It looks like %s didn't use the force\n", self->client->pers.netname);
	if (deathmatch->value)
		self->client->resp.score--;
}

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;

	if (!deathmatch->value)
		return;

	item = self->client->pers.weapon;
	if (!self->client->pers.inventory[self->client->ammo_index] && !self->client->pers.clipammo[ITEM_INDEX(item)])
		item = NULL;
//	if (item && (strcmp (item->pickup_name, "Blaster") == 0))
//		item = NULL;

	if (item)
	{
		drop = Drop_Item (self, item);
		drop->spawnflags = DROPPED_PLAYER_ITEM;
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

void falling (edict_t *ent)
{
	ent->s.frame++;
	ent->movetype = MOVETYPE_NONE;
	if (ent->s.frame == 497)
	{
		ent->think = ddc;
		ent->nextthink = level.time + 0.8;
	}
	else if (ent->groundentity && (ent->s.frame > 492 && ent->s.frame < 498))
	{
		ent->nextthink = level.time + 0.01;
	}
	else if (ent->groundentity)
	{
		ent->s.frame = FRAME_impactA;
		ent->client->anim_end = FRAME_impactB;
	}
	else if (ent->s.frame == ent->client->anim_end)
	{
		ent->s.frame = FRAME_fallingA;
		ent->client->anim_end = FRAME_fallingB;
		ent->nextthink = level.time + 0.01;
	}
	else
	{
		//Don't do crap
		ent->nextthink = level.time + 0.01;
	}
}




/*
==================
player_die
==================
*/
//
//	RipVTide 7-Aug-2000 Some editing is going to be needed here
//	2 problems are probably related with this code, namely
//	the camera jittering bug and immediate corpse clearance if
//	respawned after chasecam.
//
//	RipVTide 15-Aug-2000 Above issues are resolved. However, a
//	bug with the bots causing a crash may be originating from
//	the added code.
//
//	RipVTide 16-Aug-2000 Added code for random player deaths and
//	use of chokedeath animations if MOD is choke.
//
//	RipVTide 27-Dec-2000 Problem with crash at cratering possibly
//	originating from player_die related code.
//

void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int			n;

//
//	Code for removal of Chasecam commented out by RK
//	since the chasecam after death was just "too cool"
//
//	if(self->client->chasetoggle != 0)
//		ChasecamRemove (self, "off");

	if(!self->client->chasetoggle && !self->is_bot)
		ChasecamStart (self);

//CaRRaC - BEGIN
	if(self->client->in_snipe)
	{
		self->client->ps.gunindex = gi.modelindex(self->client->pers.weapon->view_model);
		self->client->in_snipe = 0;
		self->movetype = MOVETYPE_WALK;
	}
//CaRRaC - END

//	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;

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
//		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);
//ZOID
		CTFFragBonuses(self, inflictor, attacker);
//ZOID
		TossClientWeapon (self);
//ZOID
		if (ctf->value)
		{
			CTFPlayerResetGrapple(self);
			CTFDeadDropFlag(self);
			CTFDeadDropTech(self);
		}
//ZOID
		if (deathmatch->value)
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
	self->client->breather_framenum = 0;
	self->client->medikit_framenum = 0;
	self->client->glowlamp_framenum = 0;
//	self->flags &= ~FL_POWER_ARMOR;

	if (self->health < -60)
	{	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
//		for (n= 0; n < 4; n++)
//			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
//		ThrowClientHead (self, damage);

//ZOID
//		self->client->anim_priority = ANIM_DEATH;
//		self->client->anim_end = 0;
//ZOID

		self->takedamage = DAMAGE_NO;

//RipVTide Start

		self->solid = SOLID_NOT;
		self->s.effects = EF_SPHERETRANS;
		VectorClear (self->avelocity);
		VectorClear (self->velocity);
		self->gravity = 0;
		self->movetype = MOVETYPE_NOCLIP;

//RipVTide End
	}
	else
	{	// normal death
		self->movetype = MOVETYPE_TOSS;
		if (!self->deadflag)
		{
			float	i;

			i = random();
			// start a death animation
			self->client->anim_priority = ANIM_DEATH;

//CaRRaC Frame Alterations
			if(self->waterlevel >= 2)
			{
//				gi.dprintf("waterdeath\n");
				self->s.frame = FRAME_swdeathA-1;
				self->client->anim_end = FRAME_swdeathB;
				self->think = ddc;
				self->nextthink = level.time + 0.8;
			}
			else if(self->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				self->s.frame = FRAME_crdeathA-1;
				self->client->anim_end = FRAME_crdeathB;
				self->think = ddc;
				self->nextthink = level.time + 0.8;
			}
			else if(!self->groundentity/* && !self->is_bot*/)
			{
				self->s.frame = FRAME_ftransA;
				self->client->anim_end = FRAME_ftransB;
				self->think = falling;
				self->nextthink = 0.01;
			}
			//else if(attacker->client->pers.weapon == FindItem("Lightsaber"))
			else if(self->health < -25)
			{
				if(self->client->headshot)
				{
					self->s.frame = FRAME_noheadA-1;
					self->client->anim_end = FRAME_noheadB;
					self->think = ddc;
					self->nextthink = level.time + 0.8;
				}
				else if(self->client->chestshot)
				{
					if(random() < 0.5)
					{
						self->s.frame = FRAME_rightarmA-1;
						self->client->anim_end = FRAME_rightarmB;
						self->think = ddc;
						self->nextthink = level.time + 0.8;
					}
					else
					{
						self->s.frame = FRAME_leftarmA-1;
						self->client->anim_end = FRAME_leftarmB;
						self->think = ddc;
						self->nextthink = level.time + 0.8;
					}
				}
				else
				{
					self->s.frame = FRAME_death3A-1;
					self->client->anim_end = FRAME_death3B;
					self->think = ddc;
					self->nextthink = level.time + 0.8;
				}
			}
			else if(self->client->choking)
			{
				self->s.frame = FRAME_chokedeathA;
				self->client->anim_end = FRAME_chokedeathB;
				self->think = falling;
				self->nextthink = 0.01;
			}
			else if(i > 0.66)
			{
				self->s.frame = FRAME_death1A-1;
				self->client->anim_end = FRAME_death1B;
				self->think = ddc;
				self->nextthink = level.time + 1;
			}
			else if(i > 0.33)
			{
				self->s.frame = FRAME_death2A-1;
				self->client->anim_end = FRAME_death2B;
				self->think = ddc;
				self->nextthink = level.time + 1;
			}
			else
			{
				self->s.frame = FRAME_death3A-1;
				self->client->anim_end = FRAME_death3B;
				self->think = ddc;
				self->nextthink = level.time + 1;
			}
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}

	self->deadflag = DEAD_DEAD;

//	VectorCopy(self->client->chasecam->s.origin, temporg);
//	VectorCopy(self->client->chasecam->s.angles, tempangle);

//	fakechase(self, temporg, tempangle);

//	ChasecamRemove (self, "off");

//	VectorCopy(temporg, ent->s.origin);
//	VectorCopy(tempangle, ent->s.angles);

//	if (self->movetype != MOVETYPE_NOCLIP)
//		CopyToBodyQue (self->client->oldplayer);

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
	int			ammo;

	memset (&client->pers, 0, sizeof(client->pers));

	if(!deathmatch->value || !saberonly->value)
	{
		item = FindItem("Blaster");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;

		ammo = ITEM_INDEX(FindItem(item->ammo));
		client->pers.inventory[ammo] = 1;
		client->pers.clipammo[WEAP_PISTOL] = CLIP_PISTOL;

		client->pers.weapon = item;
//ZOID
		client->pers.lastweapon = item;
//ZOID
	}

	if(deathmatch->value)
	{
		item = FindItem("Lightsaber");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[ITEM_INDEX(item)] = 1;

		if(saberonly->value)	
		{
			client->pers.weapon = item;
//ZOID
			client->pers.lastweapon = item;
//ZOID
		}
	}
//RipVTide to edit

	//CaRRaC -- Force defining Start
	client->pers.force.pool = 0;
	client->pers.force.pool_max = 0;
	client->pers.force_power = GetPowerByIndex(1);
	client->pers.current_power = 1;
	client->pers.force_table[0] = 255;
	client->pers.force_table[1] = 255;
	client->pers.force_table[2] = 255;
	client->pers.force_table[3] = 255;
	client->pers.force_table[4] = 255;
	client->pers.force_table[5] = 255;
	client->pers.active_constants = 0;
	client->force_time = 10000;
	client->force_user = 0;
	//CaRRaC -- Force defining End

//RipVTide Set chasecam flag for saber use

	client->waschase = false;

	client->pers.health			= 100;
	client->pers.max_health		= 100;

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
		game.clients[i].pers.savedFlags = (ent->flags & (FL_GODMODE|FL_NOTARGET/*FL_POWER_ARMOR*/));
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

	if (ctf->value)
	{
//ZOID
		spot = SelectCTFSpawnPoint(ent);
//ZOID
	}
	else if (deathmatch->value)
	{
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
//	int	n;

	if (self->health < -40)
	{
//		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
//		for (n= 0; n < 4; n++)
//			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
//		self->s.origin[2] -= 48;
//		ThrowClientHead (self, damage);
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
//	body->takedamage = DAMAGE_YES;

	VectorCopy(body->s.origin, body->oldorg);
	body->think = deadboy;
	body->nextthink = level.time + 0.01;
	gi.linkentity (body);
}


void respawn (edict_t *self)
{
	int			x;
	gforce_t	*power;

	if (self->client->oldplayer) 
		G_FreeEdict (self->client->oldplayer); 
	if (self->client->chasecam) 
		G_FreeEdict (self->client->chasecam);

	self->s.modelindex = 255;

	if (deathmatch->value || coop->value)
	{
		// spectator's don't leave bodies
//		if (self->movetype != MOVETYPE_NOCLIP)
//			CopyToBodyQue (self);
		self->svflags &= ~SVF_NOCLIENT;
		PutClientInServer (self);

		// add a teleportation effect
//		self->s.event = EV_PLAYER_TELEPORT;

		RespawnExplosion(self);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_NUKEBLAST);
			gi.WritePosition (self->s.origin);
			gi.multicast (self->s.origin, MULTICAST_PHS);
		gi.sound (self, CHAN_VOICE, gi.soundindex("misc/spawn.wav"), 1, ATTN_NORM, 0);

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

		self->client->respawn_time = level.time;
		if (theforce->value)
		{
			self->client->force_user = 1;
			if (self->client->resp.side == 11)
			{
				watchmesing(self);
			}
			else if (self->client->resp.side == 12)
			{
				imabigchicken(self);
			}
			else if(self->client->resp.side == 13)
			{
				googl3(self);
			}
			else
			{
//				stuffcmd(ent, "give force");
				self->client->force_user = 1;
				for(x=1; x<NUM_POWERS+1; x++)
				{
					power = &powerlist[x];
					self->client->pers.force.powers[x] = 1;
					self->client->pers.force.power_values[x] = 1000;
				}

				calc_subgroup_values(self);
				calc_darklight_value(self);
				calc_top_level_value(self);
				self->client->pers.force.pool_max = ceil(self->client->pers.force.top_level*2);
				sort_useable_powers(self);
			}
		}
	}

	return;

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

	int			x;
	gforce_t	*power;


	// if the user wants to become a spectator, make sure he doesn't
	// exceed max_spectators

	if (ent->client->pers.spectator) {
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "spectator");
		if (*spectator_password->string && 
			strcmp(spectator_password->string, "none") && 
			strcmp(spectator_password->string, value)) {
			safe_cprintf(ent, PRINT_HIGH, "Spectator password incorrect.\n");
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
			safe_cprintf(ent, PRINT_HIGH, "Server spectator limit is full.");
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
			safe_cprintf(ent, PRINT_HIGH, "Password incorrect.\n");
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

	if (deathmatch->value)
	{
		ClientBeginDeathmatch (ent);
		if (theforce->value)
		{
			ent->client->force_user = 1;
			if (ent->client->resp.side == 11)
			{
				watchmesing(ent);
			}
			else if (ent->client->resp.side == 12)
			{
				imabigchicken(ent);
			}
			else if(ent->client->resp.side == 13)
			{
				googl3(ent);
			}
			else
			{
//				stuffcmd(ent, "give force");
				ent->client->force_user = 1;
				for(x=1; x<NUM_POWERS+1; x++)
				{
					power = &powerlist[x];
					ent->client->pers.force.powers[x] = 1;
					ent->client->pers.force.power_values[x] = 1000;
				}

				calc_subgroup_values(ent);
				calc_darklight_value(ent);
				calc_top_level_value(ent);
				ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
				sort_useable_powers(ent);
			}
		}
	}
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
		safe_bprintf (PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname);
	else
		safe_bprintf (PRINT_HIGH, "%s joined the game\n", ent->client->pers.netname);
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

	int			x;
	gforce_t	*power;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	index = ent-g_edicts-1;
	client = ent->client;

	ent->client->forcesound = 0;
	
	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
//Stupid force hack...
		if (theforce->value)
		{
			ent->client->force_user = 1;
			if (ent->client->resp.side == 11)
			{
				watchmesing(ent);
			}
			else if (ent->client->resp.side == 12)
			{
				imabigchicken(ent);
			}
			else if(ent->client->resp.side == 13)
			{
				googl3(ent);
			}
			else
			{
//				stuffcmd(ent, "give force");
				ent->client->force_user = 1;
				for(x=1; x<NUM_POWERS+1; x++)
				{
					power = &powerlist[x];
					ent->client->pers.force.powers[x] = 1;
					ent->client->pers.force.power_values[x] = 1000;
				}

				calc_subgroup_values(ent);
				calc_darklight_value(ent);
				calc_top_level_value(ent);
				ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
				sort_useable_powers(ent);
			}
		}

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

	ent->client->togo = 1;

	// clear everything but the persistant data
	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
	{
		InitClientPersistant(client);
		if (theforce->value)
		{
			ent->client->force_user = 1;
			if (ent->client->resp.side == 11)
			{
				watchmesing(ent);
			}
			else if (ent->client->resp.side == 12)
			{
				imabigchicken(ent);
			}
			else if(ent->client->resp.side == 13)
			{
				googl3(ent);
			}
			else
			{
//				stuffcmd(ent, "give force");
				ent->client->force_user = 1;
				for(x=1; x<NUM_POWERS+1; x++)
				{
					power = &powerlist[x];
					ent->client->pers.force.powers[x] = 1;
					ent->client->pers.force.power_values[x] = 1000;
				}

				calc_subgroup_values(ent);
				calc_darklight_value(ent);
				calc_top_level_value(ent);
				ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
				sort_useable_powers(ent);
			}
		}
	}
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

//CaRRaC
	ent->fog_fog = 0;
	ent->is_bot = 0;
	ent->saber_colour = CRYSTAL_RED;
	client->duel = 0;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
//ZOID

//	if (!client->in_snipe)
//	{
	client->ps.fov = 90;
//	}
//	else
//	{
//		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
//		if (client->ps.fov < 1)
//			client->ps.fov = 90;
//		else if (client->ps.fov > 160)
//			client->ps.fov = 160;
//		client->ps.fov = client->zoom_factor;
//	}

	client->zoom_factor = 90;
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

	if(mouse_s->value && !ent->is_bot)
	{
		client->mouse_s = mouse_s->value;
		_stuffcmd(ent, "sensitivity \"%f\"\n", ent->client->mouse_s);
		safe_cprintf(ent, PRINT_MEDIUM, "Custom mouse sensitivity is %f\n", client->mouse_s);
	}
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.
=====================
*/
//CaRRaC BOT
int num_players = 0;
edict_t *players[MAX_CLIENTS];
//CaRRaC END

void ClientBeginDeathmatch (edict_t *ent)
{
	G_InitEdict (ent);
	
	InitClientResp (ent->client);

	players[num_players++] = ent;

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

	safe_bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

//RipVTide

	ent->client->zoom_factor = 45;

	RespawnExplosion(ent);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_NUKEBLAST);
		gi.WritePosition (ent->s.origin);
		gi.multicast (ent->s.origin, MULTICAST_PHS);

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

	int			x;
	gforce_t	*power;

	ent->client = game.clients + (ent - g_edicts - 1);

	if (deathmatch->value)
	{
		ClientBeginDeathmatch (ent);
		if (theforce->value)
		{
			ent->client->force_user = 1;
			if (ent->client->resp.side == 11)
			{
				watchmesing(ent);
			}
			else if (ent->client->resp.side == 12)
			{
				imabigchicken(ent);
			}
			else if(ent->client->resp.side == 13)
			{
				googl3(ent);
			}
			else
			{
//				stuffcmd(ent, "give force");
				ent->client->force_user = 1;
				for(x=1; x<NUM_POWERS+1; x++)
				{
					power = &powerlist[x];
					ent->client->pers.force.powers[x] = 1;
					ent->client->pers.force.power_values[x] = 1000;
				}

				calc_subgroup_values(ent);
				calc_darklight_value(ent);
				calc_top_level_value(ent);
				ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
				sort_useable_powers(ent);
			}
		}
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

			safe_bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
		}
	}

//RipVTide

//	ent->client->zoom_factor = 45;

	RespawnExplosion(ent);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_NUKEBLAST);
		gi.WritePosition (ent->s.origin);
		gi.multicast (ent->s.origin, MULTICAST_PHS);
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
//ZOID
	if (ctf->value)
		CTFAssignSkin(ent, s);
	else
//ZOID
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s));

	// fov
	if (!ent->client->in_snipe)
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

	int			x;
	gforce_t	*power;

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");
	if (SV_FilterPacket(value)) {
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
//ZOID -- force team join
		ent->client->resp.ctf_team = -1;
//ZOID
		// clear the respawning variables
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
		{
			InitClientPersistant (ent->client);
			if (theforce->value)
			{
				ent->client->force_user = 1;
				if (ent->client->resp.side == 11)
				{
					watchmesing(ent);
				}
				else if (ent->client->resp.side == 12)
				{
					imabigchicken(ent);
				}
				else if(ent->client->resp.side == 13)
				{
					googl3(ent);
				}
				else
				{
//					stuffcmd(ent, "give force");
					ent->client->force_user = 1;
					for(x=1; x<NUM_POWERS+1; x++)
					{
						power = &powerlist[x];
						ent->client->pers.force.powers[x] = 1;
						ent->client->pers.force.power_values[x] = 1000;
					}

					calc_subgroup_values(ent);
					calc_darklight_value(ent);
					calc_top_level_value(ent);
					ent->client->pers.force.pool_max = ceil(ent->client->pers.force.top_level*2);
					sort_useable_powers(ent);
				}
			}
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
void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!ent->client)
		return;

	safe_bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

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

void Client_Check_Collide (edict_t *self)
{
	vec3_t	end;
	vec3_t	forward, side;
	trace_t	tr;
	vec3_t normal;

	AngleVectors (self->client->v_angle, forward, side, NULL);

	if(self->client->forwardmove > 100)
	{
		VectorMA(self->s.origin, 95, forward, end);

		tr = gi.trace(self->s.origin, self->mins, self->maxs, end, self, MASK_ALL);

		if(tr.fraction != 1)
		{
			if(tr.ent)
			{
				VectorNormalize2(forward, normal);
				T_Damage (tr.ent, self, self, forward, tr.endpos, normal, 30, 1000, 1, 1);
			}
		}
	}

	if(self->client->forwardmove > 0)
	{
		VectorMA(self->s.origin, 95, side, end);

		tr = gi.trace(self->s.origin, self->mins, self->maxs, end, self, MASK_ALL);

		if(tr.fraction != 1)
		{
			if(tr.ent)
			{
				VectorNormalize2(forward, normal);
				T_Damage (tr.ent, self, self, side, tr.endpos, normal, 30, 1000, 1, 1);
			}
		}
	}
	else if(self->client->forwardmove < 0)
	{
		VectorMA(self->s.origin, 95, side, end);

		VectorNegate(side, side);

		tr = gi.trace(self->s.origin, self->mins, self->maxs, end, self, MASK_ALL);

		if(tr.fraction != 1)
		{
			if(tr.ent)
			{
				VectorNormalize2(side, normal);
				T_Damage (tr.ent, self, self, side, tr.endpos, normal, 30, 1000, 1, 1);
			}
		}
	}
}

void set_fov (edict_t *ent)
{
//	char string[16];


	if(ent->is_bot)
		return;

//	gi.dprintf("zooming\n");


	ent->client->ps.fov = ent->client->zoom_factor;


//	sprintf (string, "fov %f\n", ent->client->zoom_factor);
//	gi.WriteByte(11);
//	gi.WriteString(string);
//	gi.unicast(ent, 1);



/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/

}

void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t		*client;
	edict_t			*other;
	int				i, j;
	pmove_t			pm;
//	vec3_t start, end, forward2;
//	trace_t tr2;
	vec3_t			tempvec;


//CaRRaC
	gitem_t			*newweapon;
	unsigned short	levitate;
	unsigned short	invisi;
	unsigned short	jump;
	unsigned short	speed;
	unsigned short	bind;
	unsigned short	ghosted;
	float			force_jump_add;
	float			f_lev_power;

//RipVTide

	qboolean		saber;
	char			*weap;
	vec3_t			angles_v;

	vec3_t			tv, forward, side;
	trace_t			tr;
	vec3_t			start, end, forw;
//	trace_t			trace;
	usercmd_t		*ucmd2;

	level.current_entity = ent;
	client = ent->client;

	if (ent->client->pers.weapon)
		weap = ent->client->pers.weapon->classname;
	else
		weap = "";

	if (strcmp(weap, "weapon_saber") == 0)
		saber = true;
	else
		saber = false;

	if(ent->forceflags[AFF_LIGHT] & FFL_BIND)
	{
		ent->forceflags[AFF_LIGHT] &= ~FFL_BIND;
		bind = 1;
	}
	else
	{
		bind = 0;
	}

	//RipVTide

//	VectorCopy(ent->s.origin, start);
//	start[2] += 24;
//	AngleVectors (ent->client->v_angle, forward, NULL, NULL);
//	VectorMA (start, 200, forward, end);
//
//	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
//
//	if(strcmp(tr.ent->classname, "worldspawn") == 0)
//	{
//		return;
//	}


	if(Force_constant_active(ent, NFORCE_LEVITATE) == 255)
		levitate = 0;
	else
		levitate = 1;

	if(Force_constant_active(ent, NFORCE_SPEED) == 255)
		speed = 0;
	else
		speed = 1;

	if(Force_constant_active(ent, NFORCE_JUMP) == 255)
		jump = 0;
	else
		jump = 1;

	if(Force_constant_active(ent, LFORCE_INVISIBILITY) == 255 && ent->client->invisi_time < level.time)
		invisi = 0;
	else
		invisi = 1;

	force_frame(ent); //run the important force power functions

//	RipVTide - Temp function if player position needs to
//	be determined

//	gi.dprintf("--%f--%f--%f--\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
	
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
		else if (ent->s.modelindex != 255 && !invisi && !(ent->client->vflags & IN_VEHICLE))
			client->ps.pmove.pm_type = PM_GIB;
		else if (ent->deadflag)
			client->ps.pmove.pm_type = PM_DEAD;
		else if (ent->client->in_snipe)
			client->ps.pmove.pm_type = PM_FREEZE;
		else
			client->ps.pmove.pm_type = PM_NORMAL;

		if(levitate || (ent->client->vflags & IN_VEHICLE))
		{
			client->ps.pmove.gravity = (random()*100)-50;
		}
		else if (speed)
		{
			if(ent->client->pers.force.power_values[NFORCE_SPEED] > 600)
			{
				client->ps.pmove.gravity = sv_gravity->value/2;
			}
			else if (ent->client->pers.force.power_values[NFORCE_SPEED] > 100)
			{
				client->ps.pmove.gravity = sv_gravity->value/1.5;
			}
		}
		else if (ent->client->state_edit == true)
		{	
			client->ps.pmove.gravity = 0;
		}
		else
		{
			client->ps.pmove.gravity = sv_gravity->value;
		}
		pm.s = client->ps.pmove;

		if(ent->client->vflags & IN_VEHICLE)
		{
			ent->client->forwardmove = ucmd->forwardmove; //copy all movements variables into temps
			ent->client->sidemove = ucmd->sidemove;
			ent->client->upmove = ucmd->upmove;

			ucmd->forwardmove = 0;	//clear all movement variables
			ucmd->sidemove = 0;
			ucmd->upmove = 0;

			//VERTICAL MOVEMENT
			VectorCopy(ent->s.origin, tv);
			tv[2] -= ent->client->max_height;

			tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, tv, ent, MASK_SOLID);

			if((tr.fraction != 1) || (ent->client->current_height > 0))
			{
				pm.s.origin[2] = (tr.endpos[2] + ent->client->max_height + (random()*4))*8;
			}
			else
			{
				pm.s.origin[2] = (ent->s.origin[2] - (sv_gravity->value/100))*8;
			}

			//HORIZONTAL MOVEMENT
			AngleVectors(client->v_angle, forward, side, NULL);

			if(ent->client->forwardmove > 0 || ent->client->sidemove)
			{
				for(i=0; i<2; i++)
				{
//					pm.s.origin[i] = (ent->s.origin[i] + (ent->client->forwardmove*forward[i]))*8;
					pm.s.velocity[i] = ((ent->client->forwardmove*forward[i])+((ent->client->sidemove/2)*side[i]))*8;
				}
			}
			if(ent->client->forwardmove < 0)
			{
				pm.s.velocity[0] = 0;
				pm.s.velocity[1] = 0;
			}
		}
		else if(ent->client->in_snipe)
		{
			pm.s.origin[0] = ent->s.origin[0]*8;
			pm.s.velocity[0] = 0;
			pm.s.origin[1] = ent->s.origin[1]*8;
			pm.s.velocity[1] = 0;
			pm.s.origin[2] = ent->s.origin[2]*8;
			pm.s.velocity[2] = ent->velocity[2]*8;

			ent->client->zoom_factor -= (ucmd->forwardmove/200);
			if(ent->client->zoom_factor > 90)
				ent->client->zoom_factor = 90;
			else if(ent->client->zoom_factor < 10)
				ent->client->zoom_factor = 10;
			if(ucmd->forwardmove)
//				ent->client->ps.fov = ucmd->forwardmove;
				set_fov(ent);
			ent->client->forwardmove = 0;
			pm.s.velocity[0] = 0;
			pm.s.velocity[1] = 0;
			pm.s.velocity[2] = 0;
			ucmd->forwardmove = 0;	//clear all movement variables
			ucmd->sidemove = 0;
			ucmd->upmove = 0;
		}
		else if (bind)
		{
			pm.s.velocity[0] = 0;
			pm.s.velocity[1] = 0;
			pm.s.velocity[2] = 0;
		}
//RipVTide
//
//Cool gravity Hack =)
		else if (coolgrav->value)
		{
			if(!(ent->client->true_grav == 0))
			{
				if(VectorCompare(ent->client->true_vec, ent->client->wanted_vec))
//					VectorCopy(ent->client->true_vec, ucmd->angles);
					ucmd->upmove = ucmd->upmove;
				else
				{
					VectorCopy(ent->client->wanted_vec, ent->client->true_vec);
					VectorCopy(ent->client->true_vec, ucmd->angles);
//					short	forwardmove, sidemove, upmove
				}

				VectorClear (angles_v);

				if(!(ucmd->forwardmove == 0))
				{
					VectorCopy(ent->s.origin, start);
					AngleVectors (angles_v, forw, NULL, NULL);
					VectorMA (start, ucmd->forwardmove, forw, end);
					VectorSubtract (ent->s.origin, end, angles_v);
					VectorAdd (pm.s.velocity, angles_v, angles_v);
					ucmd2->forwardmove =+ angles_v[0];
					ucmd2->sidemove =+ angles_v[1];
					ucmd2->upmove =+ angles_v[2];
				}					
				if(!(ucmd->sidemove == 0))
				{
					VectorCopy(ent->s.origin, start);
					AngleVectors (angles_v, NULL, forw, NULL);
					VectorMA (start, ucmd->forwardmove, forw, end);
					VectorSubtract (ent->s.origin, end, angles_v);
					VectorAdd (pm.s.velocity, angles_v, angles_v);
					ucmd2->forwardmove =+ angles_v[0];
					ucmd2->sidemove =+ angles_v[1];
					ucmd2->upmove =+ angles_v[2];
				}

				if(ent->client->v_angle[0] > -1 && ent->client->v_angle[0] < 91)
				{
					angles_v[0] = ent->client->v_angle[2] + 90;
					angles_v[1] = ent->client->v_angle[1];
				}
				else
				{
					if(ent->client->v_angle[1] > 180)
						angles_v[1] = ent->client->v_angle[1] - 180;
					else
						angles_v[1] = ent->client->v_angle[1] + 180;
				}
				angles_v[2] = ent->client->v_angle[2];
				VectorCopy(ent->s.origin, start);
				AngleVectors (angles_v, forw, NULL, NULL);
				if (client->ps.pmove.pm_flags & PMF_JUMP_HELD && ent->groundentity)
				{
					ent->client->jumping = true;
					client->ps.pmove.pm_flags &= PMF_JUMP_HELD;
					ent->client->airtime = true;
					VectorMA (start, ent->client->true_grav/64, forw, end);
					VectorSubtract (ent->s.origin, end, angles_v);
					VectorAdd (pm.s.velocity, angles_v, angles_v);
					ucmd2->forwardmove =+ angles_v[0];
					ucmd2->sidemove =+ angles_v[1];
					ucmd2->upmove =+ angles_v[2];
				}
				else if(client->ps.pmove.pm_flags & PMF_JUMP_HELD && ent->client->airtime)
				{
					ent->client->jumping = true;
					client->ps.pmove.pm_flags &= PMF_JUMP_HELD;
					ent->client->airtime = false;
					VectorMA (start, ent->client->true_grav/32, forw, end);
					VectorSubtract (ent->s.origin, end, angles_v);
					VectorAdd (pm.s.velocity, angles_v, angles_v);
					ucmd2->forwardmove =+ angles_v[0];
					ucmd2->sidemove =+ angles_v[1];
					ucmd2->upmove =+ angles_v[2];
				}
				else
				{
					ent->client->jumping = false;
					ent->client->airtime = false;
				}
				if(!ent->groundentity)
				{
					if(ent->client->v_angle[0] > -91 && ent->client->v_angle[0] < 1)
					{
						angles_v[0] = ent->client->v_angle[2] - 90;
						angles_v[1] = ent->client->v_angle[1];
					}
					else
					{
						if(ent->client->v_angle[1] > 180)
							angles_v[1] = ent->client->v_angle[1] - 180;
						else
							angles_v[1] = ent->client->v_angle[1] + 180;
					}

					angles_v[2] = ent->client->v_angle[2];

					VectorMA (start, ent->client->true_grav/24, forw, end);
					VectorSubtract (ent->s.origin, end, angles_v);
					VectorAdd (pm.s.velocity, angles_v, angles_v);
					ucmd2->forwardmove =+ angles_v[0];
					ucmd2->sidemove =+ angles_v[1];
					ucmd2->upmove =+ angles_v[2];
				}
				ucmd->forwardmove = ucmd2->forwardmove;
				ucmd->sidemove = ucmd2->sidemove;
				ucmd->upmove = ucmd2->upmove;

//				pm.s.velocity[0] = ucmd->forwardmove;
//				pm.s.velocity[1] = ucmd->sidemove;
//				pm.s.velocity[1] = ucmd->upmove;

//				gi.dprintf("forward %d side %d vertical %d\n", ucmd->forwardmove, ucmd->sidemove, ucmd->upmove);

				for (i=0 ; i<3 ; i++)
				{
					pm.s.origin[i] = ent->s.origin[i]*8;
					pm.s.velocity[i] = ent->velocity[i]*8;
				}
			}
			else if (ent->client->true_grav == 0)
			{
				ucmd->forwardmove = ent->client->true_speed[0];
				ucmd->sidemove = ent->client->true_speed[1];
				ucmd->upmove = ent->client->true_speed[2];
//				VectorAdd(ucmd->angles, ent->client->wanted_vec ,ucmd->angles);
			}
		}
		else
		{
//			if(!ent->is_bot)
//			{
//				gi.WriteByte(11);		// 11 = svc_stufftext
//				gi.WriteString("fov 90\n");
//				gi.unicast(ent, 1);
//			}
			for (i=0 ; i<3 ; i++)
			{
				pm.s.origin[i] = ent->s.origin[i]*8;
				pm.s.velocity[i] = ent->velocity[i]*8;
			}
			if (ent->holstered == 1 || (ent->client->duel && saber))
			{
				tempvec[0] = pm.s.velocity[0];
				tempvec[1] = pm.s.velocity[1];
				tempvec[2] = pm.s.velocity[2];
				VectorScale (tempvec, 0.3, tempvec);
				if(ent->groundentity)
				{
					pm.s.velocity[0] = tempvec[0];
					pm.s.velocity[1] = tempvec[1];
					pm.s.velocity[2] = tempvec[2];
				}
//				if(pm.s.velocity[0] > 180)
//					pm.s.velocity[0] = 180;
//				if(pm.s.velocity[0] < -180)
//					pm.s.velocity[0] = -180;
//				if(pm.s.velocity[1] > 180)
//					pm.s.velocity[1] = 180;
//				if(pm.s.velocity[1] < -180)
//					pm.s.velocity[1] = -180;
			}
		}

		if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
		{
			pm.snapinitial = true;
	//		gi.dprintf ("pmove changed!\n");
			if (ent->holstered == 1 || (ent->client->duel && saber))
			{
				tempvec[0] = pm.s.velocity[0];
				tempvec[1] = pm.s.velocity[1];
				tempvec[2] = pm.s.velocity[2];
				VectorScale (tempvec, 0.2, tempvec);
				pm.s.velocity[0] = tempvec[0];
				pm.s.velocity[1] = tempvec[1];
				pm.s.velocity[2] = tempvec[2];
//				if(pm.s.velocity[0] > 180)
//					pm.s.velocity[0] = 180;
//				if(pm.s.velocity[0] < -180)
//					pm.s.velocity[0] = -180;
//				if(pm.s.velocity[1] > 180)
//					pm.s.velocity[1] = 180;
//				if(pm.s.velocity[1] < -180)
//					pm.s.velocity[1] = -180;
			}
		}

//
//	RipVTide 15-Aug-2000
//	Crippling routine for choking. Also lowers current choke
//	count by one.
//
		if(ent->client->choking)
		{
			for (i=0 ; i<3 ; i++)
			{
				pm.s.velocity[i] = ent->velocity[i]*0.5;
			}
			ent->client->choking--;

		}

//
//		RipVTide - 24-Aug-2000 Added slowdown if taunting.
//
		if(((ent->s.frame > FRAME_flashsbrA - 1) && (ent->s.frame < FRAME_flashsbrB - 1)) || ((ent->s.frame > FRAME_flashwepA - 1) && (ent->s.frame < FRAME_flashwepB - 1)))
		{
			for (i=0 ; i<3 ; i++)
			{
				pm.s.velocity[i] = ent->velocity[i]*0.2;
			}
		}

		pm.cmd = *ucmd;

		pm.trace = PM_trace;	// adds default parms
		pm.pointcontents = gi.pointcontents;

		if(ent->deadflag && ent->movetype == MOVETYPE_NOCLIP)
		{
		}
		else
		{
			// perform a pmove
			if(ent->client->vflags & IN_VEHICLE)
			{
				gi.Pmove (&pm);
			}
			else if(speed)
			{
				if(ent->client->pers.force.power_values[NFORCE_SPEED] > 100)
				{
					gi.Pmove (&pm);
				}
				if (ent->client->pers.force.power_values[NFORCE_SPEED] > 600)
				{
					gi.Pmove (&pm);
				}

				if((pm.s.velocity[0] != 0) || (pm.s.velocity[1] != 0) || (pm.s.velocity[2] != 0))
				{
					if(trailtime->value)
					{
						ghosted = 1;
						spawn_ghost (ent);
					}
				}
			}
		}

		if(ent->client->in_snipe)
		{
			pm.s.velocity[0] = 0;
			pm.s.velocity[1] = 0;
			pm.s.velocity[2] = 0;
		}
			
		if(ent->deadflag && ent->movetype == MOVETYPE_NOCLIP)
		{
		}
		else
			gi.Pmove (&pm);

		client->forwardmove = pm.cmd.forwardmove;
		client->sidemove = pm.cmd.sidemove;
		client->upmove = pm.cmd.upmove;

		// save results of pmove
		client->ps.pmove = pm.s;
		client->old_pmove = pm.s;

		if(!ent->client->in_snipe)
		{
			for (i=0 ; i<3 ; i++)
			{
				ent->s.origin[i] = pm.s.origin[i]*0.125;
				ent->velocity[i] = pm.s.velocity[i]*0.125;
			}
		}

		VectorCopy (pm.mins, ent->mins);
		VectorCopy (pm.maxs, ent->maxs);

		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

		if((pm.s.velocity[2] != 0) && (ent->jumping) && (jump))
		{
			if(trailtime->value && !ghosted)
				spawn_ghost (ent);
		}

		if(levitate && !pm.groundentity)
		{
			if((pm.cmd.upmove) && (!pm.waterlevel))
			{
				f_lev_power = ent->client->pers.force.power_values[NFORCE_LEVITATE]/1000.0;
				if(f_lev_power < 0.3)
					f_lev_power = 0.3;
				ent->velocity[2] += (pm.cmd.upmove/50)*f_lev_power;
			}
			else if (!pm.waterlevel)
			{
				if(ent->velocity[2] > 0)
					ent->velocity[2] -= 5;
				else if(ent->velocity[2] < 0)
					ent->velocity[2] += 5;
			}
		}
		else if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
		{

			if(jump)
			{
				if (ent->client->forcesound == 0)
				{
					sound_delay(ent, 0.6, 0);
					gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/jump.wav"), 1, ATTN_NORM, 0);
				}
				else if (ent->client->forcesound == 2)
				{
					sound_delay(ent, 0.6, 2);
					gi.sound (ent, CHAN_ITEM, gi.soundindex ("force/jump.wav"), 1, ATTN_NORM, 0);
				}
				ent->jumping = 1;
				Drain_Force_Pool (ent, NFORCE_JUMP);
				if(pm.s.velocity[2] != 0)
				{
					if(trailtime->value && !ghosted)
						spawn_ghost (ent);
				}

				force_jump_add = ent->client->pers.force.power_values[NFORCE_JUMP]/10.0;
				force_jump_add *= 3;
				ent->velocity[2] += force_jump_add;
			}
			else
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
			}

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
			vec3_t angle;

			VectorCopy (pm.viewangles, angle);

			if (client->chasetoggle == 3)
			{
				client->chasecam->chaseAngle = pm.viewangles[YAW] - client->v_angle[YAW];
				VectorCopy (client->oldplayer->s.angles, client->v_angle);
			}
			else if(client->chasetoggle > 0)
			{
				angle[YAW] -= client->chasecam->chaseAngle;
				VectorCopy (angle, client->v_angle);
			}
			else
			{
				VectorCopy (pm.viewangles, client->v_angle);
			}
			VectorCopy (pm.viewangles, client->ps.viewangles);
		}

		gi.linkentity (ent);

//		gi.dprintf("vector0 -%f- vector1 -%f- vector3 -%f-\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);

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
	if (client->latched_buttons & BUTTON_ATTACK
//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
//ZOID
	)
	{
		//CaRRaC Start
		if (client->resp.spectator)
		{
			client->latched_buttons = 0;

			if (client->chase_target)
			{
				client->chase_target = NULL;
				client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			}
			else
				GetChaseTarget(ent);
		}
		else if((ent->holstered == 0) && (ent->client->icon_menu_active == 2))
		{
			newweapon = weapon_menu_use(ent);
			if(ent->client->pers.weapon != newweapon)
			{
				ent->client->newweapon = weapon_menu_use(ent);
				ent->client->menu_time = level.time;
			}
			ent->client->menu_time = level.time;
		}
		else if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			Think_Weapon (ent, 0);
		}
		//CaRRaC End
	}

//ZOID
//regen tech
//	CTFApplyRegeneration(ent);
//ZOID

//ZOID
//	for (i = 1; i <= maxclients->value; i++) {
//		other = g_edicts + i;
//		if (other->inuse && other->client->chase_target == ent)
//			UpdateChaseCam(other);
//	}
//ZOID

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

	if (deathmatch->value && client->pers.spectator != client->resp.spectator && (level.time - client->respawn_time) >= 5)
	{
		spectator_respawn(ent);
		return;
	}

	if((!ent->is_bot) && ((ent->fog_changed) || (ent->client->ps.rdflags & RDF_UNDERWATER)))
		Fog_Update(ent);

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->resp.spectator && client->force_user)
	{
		Think_Force (ent);
	}
	if (!client->weapon_thunk && !client->resp.spectator)
		Think_Weapon (ent, 0);
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
				if(ent->is_bot)
					Bot_respawn(ent);
				else
					respawn(ent);
				client->latched_buttons = 0;
			}
		}
		return;
	}

	// add player trail so monsters can follow
	if(path_time)
		make_node (ent);

	client->latched_buttons = 0;
}


void RespawnExplosion(edict_t *ent)
{
	edict_t		*t;

	t = G_Spawn();
	t->nextthink = 0.02;
	t->think = chicken;
	t->s.modelindex = gi.modelindex("models/objects/shockwave/tris.md2");
	VectorCopy(ent->s.origin, t->s.origin);
	VectorCopy(ent->s.angles, t->s.angles);
	t->movetype = MOVETYPE_NONE;
	t->solid = SOLID_NOT;
	t->owner = ent;
	t->enemy = ent;
	t->classname = "respawn_effect";
	t->s.frame = 0;
	t->s.effects = EF_SPHERETRANS;
	t->s.renderfx = RF_FULLBRIGHT;

	gi.linkentity (t);

//	gi.sound (ent, CHAN_ITEM, gi.soundindex("sound/force.wav"), 1, ATTN_NORM, 0);
}

void chicken(edict_t *ent)
{
	if(ent->s.frame == 14)
	{
		G_FreeEdict(ent);
	}
	else
	{
		ent->nextthink = 0.01;
		ent->s.frame++;
	}
}

void deadboy (edict_t *ent)
{
	ent->think = deadboy2;
	ent->nextthink = level.time + 12;
}

void deadboy2 (edict_t *ent)
{
	ent->s.origin[2]--;
	if((ent->oldorg[2] - 9) > ent->s.origin[2])
	{
		ent->s.modelindex = 0;
		gi.unlinkentity (ent);
	}
	else
	{
		ent->nextthink = level.time + 0.1;
	}
}
void ddc (edict_t *ent)
{
	if(!ent->is_bot)
	{
		CopyToBodyQue (ent->client->oldplayer);
		ent->s.effects = EF_SPHERETRANS;
	}
	else
	{
		CopyToBodyQue (ent);
		ent->think = Bot_AI_Think;
		ent->nextthink = level.time + 0.01;
		ent->s.effects = EF_SPHERETRANS;
		ent->deadflag = 3;
	}
}