/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "g_local.h"
#include "m_player.h"

void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void ThrowClientHead2(edict_t *self);

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

	for(;;)
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

void Frag_Get (edict_t *fragger, int frags)
{
	edict_t *attacker = fragger->get_monster_ent;
	int gm = iwm_gamemode->value;

	// Assumption.
	if (frags == 0)
		frags = 1;
	if (!fragger->client)
		return;

	if (gm == 2) // Monster Hunt
	{
		if (attacker && !attacker->client && attacker->is_spawned) // Yay, not a player.
		{
			fragger->client->resp.score += frags;
		}
		return;
	}
	fragger->client->resp.score += frags;

	if (gm == 1 && frags > 0) // Quake2 VS IWM
	{
		if (fragger->client->resp.team == 1)
			level.team1_frags++;
		else
			level.team2_frags++;
	}
}

void MonsterObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;
	char *atkname, *selfname;

	if (Q_stricmp(self->classname, "monster_car") == 0 || Q_stricmp(attacker->classname, "monster_car") == 0)
		return;
	
	if (attacker->client)
		atkname = attacker->client->pers.netname;
	else if (attacker->name)
		atkname = attacker->name;
	else
		atkname = attacker->classname;
	
	if (self->client)
		selfname = self->client->pers.netname;
	else if (self->name)
		selfname = self->name;
	else
		selfname = self->classname;

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
			message = " suicides";
			break;
		case MOD_FALLING:
			message = " cratered";
			break;
		case MOD_CRUSH:
			message = " was squished";
			break;
		case MOD_WATER:
			message = " sank like a rock";
			break;
		case MOD_SLIME:
			message = " melted";
			break;
		case MOD_LAVA:
			message = " does a back flip into the lava";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = " blew up";
			break;
		case MOD_EXIT:
			message = " found a way out";
			break;
		case MOD_TARGET_LASER:
			message = " saw the light";
			break;
		case MOD_TARGET_BLASTER:
			message = " got blasted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = " was in the wrong place";
			break;
		}
		if (attacker == self)
		{
			int i;
//			char *gen;

			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = " tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsNeutral(self))
					message = " tripped on its own grenade";
				else if (IsFemale(self))
					message = " tripped on her own grenade";
				else
					message = " tripped on his own grenade";
				break;
			case MOD_R_SPLASH:
				if (IsNeutral(self))
					message = " blew itself up";
				else if (IsFemale(self))
					message = " blew herself up";
				else
					message = " blew himself up";
				break;
			case MOD_BFG_BLAST:
				message = " should have used a smaller gun";
				break;
			case MOD_RAILGUN_BOUNCE:
				message = "'s railgun bounced back the wrong way";
				break;
			case MOD_MACHINEGUN:
				if (random() < 0.5)
					message = " shot a bullet the wrong way";
				else
					message = " saw the bullet bounce back";
				break;
			case MOD_FORCE_SPLASH:
				if (IsNeutral(self))
					message = " couldn't escape its own area grenade";
				else if (IsFemale(self))
					message = " couldn't escape her own area grenade";
				else
					message = " couldn't escape his own area grenade";
				break;
			case MOD_JACK_SPLASH:
				if (IsNeutral(self))
					message = " stepped over its own jacks";
				else if (IsFemale(self))
					message = " stepped over her own jacks";
				else
					message = " stepped over his own jacks";
				break;
			case MOD_JACK:
				if (IsNeutral(self))
					message = " stepped over its own jacks";
				else if (IsFemale(self))
					message = " stepped over her own jacks";
				else
					message = " stepped over his own jacks";
				break;
			case MOD_HELD_FORCE:
				message = " felt the force";
				break;
// CYBERSLASH: more pig obituries
			case MOD_PIG_GRAVITY:
				message = " tried to exit through the stratosphere";
				break;
// CYBERSLASH
			default:
				if (IsNeutral(self))
					message = " killed itself";
				else if (IsFemale(self))
					message = " killed herself";
				else
					message = " killed himself";
				break;
			case MOD_FORCE_FALLING:
				message = " played with the area grenade too much";
				break;
			case MOD_LIGHTNING_DIRECTDISCHARGE:
				i = rand()%3;
				if (i == 0)
					message = " discharged in the water";
				else if (i == 1)
				{
					if (IsNeutral(self))
						message = " gibbed from its own bolt";
					else if (IsFemale(self))
						message = " gibbed from her own bolt";
					else
						message = " gibbed from his own bolt";
				}
				else if (i == 2)
				{
					if (IsNeutral(self))
						message = " watched its own gibs fly after the discharge";
					else if (IsFemale(self))
						message = " watched her own gibs fly after the discharge";
					else
						message = " watched his own gibs fly after the discharge";
				}
				else if (i == 3)
				{
					message = " didn't realize lighting and water don't mix";
				}
				break;
			case MOD_PLASMA_SPLASH:
				message = " shot his Plasma Rifle at the wrong place and the wrong time";
				break;

			case MOD_FIRESMALL:
				message = " burnt himself alive on his own flames";
				break;
			case MOD_BURNT:
				message = " burned on his own fire";
				break;

			case MOD_GAS:
				message = " choked on his own toxic fumes";
				break;

			case MOD_LAVABALL_SPLASH:
				message = " was splashed by his own lavaballs";
				break;

			case MOD_INFESTED:
				if (IsFemale(self))
					message = " died by her own infest";
				else
					message = " died by his own infest";
				break;

			case MOD_ARTILLERY:
				message = " ran into the wrong artillery shell";
				break;
			case MOD_DETONATOR:
				message = " knew his life was over, and the button got him out";
				break;
			case MOD_TRAP:
			 	message = " sucked into his own trap";
				break;
			case MOD_CHAINLINK:
				message = " tried to go through his own lasers";
				break;
			}
		}
		if (message)
		{
			safe_bprintf (PRINT_MEDIUM, "%s%s.\n", selfname, message);
			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker)
		{
			int raa;
			switch (mod)
			{
			case MOD_BLASTER:
				message = "was blasted by";
				break;
			case MOD_STAR:
				message = "went too close to";
				message2 = " while he had a star";
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
			case MOD_RAILGUN_BOUNCE:
				message = "took a rail from";
				message2 = "'s rail bounce";
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
			case MOD_INFESTED:
				message = "was infested by";
				message2 = "'s grenade";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
			case MOD_RUBBERBALL:
				message = "was ran over by";
				message2 = "'s little rubber friend";
				break;
			case MOD_DEVASTATOR:
				message = "was pulverised from";
				message2 = "'s Devastator.";
				break;
			case MOD_ROCKETPOD:
				message = "took pods in the face from";
				message2 = "'s Rocket Pod launcher.";
				break;
			case MOD_SHARD:
				message = "ate a glass shard from";
				message2 = "'s shard cannon.";
				break;
			case MOD_GRENADELASER:
				message = "ran right into";
				message2 = "'s laser grenade.";
				break;
			case MOD_NEEDLER:
				message = "was needled by";
				break;
			case MOD_FIREFLY:
				message = "was chased down by";
				message2 = "'s fireflies.";
				break;
			case MOD_FOOBY:
				message = "choked on";
				message2 = "'s kamikaze watermelon.";
				break;
			case MOD_BALLOON:
				message = "got drenched and drowned from";
				message2 = "'s water balloon.";
				break;
			case MOD_SCREWDRIVER:
				message = "was driven into the ground from";
				message2 = "'s screwdriver projectiles.";
				break;

			case MOD_WOMD:
				raa = rand()%5;

				switch (raa)
				{
				case 0:
					message = "was atomized from";
					message2 = "'s Weapon of Mass Destruction";
					break;
				case 1:
					message = "was pulverized from";
					message2 = "'s WOMD";
					break;
				case 2:
					message = "has body parts moulded to the ground by";
					message2 = "'s Weapon of Mass Destruction";
					break;
				case 3:
					message = "decomposed many years in advanced, thanks to";
					message2 = "'s WOMD";
					break;
				case 4:
					message = "crumbled into ashes from";
					message2 = "'s Weapon of Mass Destruction";
					break;
				case 5:
					message = "felt the wrath of";
					message2 = "'s WOMD";
					break;
				default:
					message = "was radiated by";
					message2 = "'s Weapon of Mass Destruction";
					break;
				}
				break;
			case MOD_BEE:
				message = "heard the buzzing from";
				message2 = "'s kamikaze bees.";
				break;
			case MOD_BEE_SPLASH:
				message = "was hit by";
				message2 = "'s bee splashes";
				break;
			case MOD_BASKETBALL:
				message = "forgot to catch";
				message2 = "'s basketball pass";
				break;
			case MOD_BEACHBALL:
				message = "didn't have a fun time at the beach thanks to";
				message2 = "'s beach ball.";
				break;
			case MOD_HELD_FORCE:
				message = "felt";
				message2 = "'s force";
				break;
			case MOD_FORCE_SPLASH:
				message = "couldn't withstand the force of";
				message2 = "'s area grenade";
				break;
			case MOD_JACK_SPLASH:
				message = "stepped on";
				message2 = "'s jacks";
				break;
			case MOD_JACK:
				message = "lost a game of jacks thrown by";
				break;
			case MOD_ZAPPED:
				message = "discovered that";
				if (IsNeutral(self))
					message2 = "'s weather-control system was against it";
				else if (IsFemale(self))
					message2 = "'s weather-control system was against her";
				else
					message2 = "'s weather-control system was against him";
				break;
			case MOD_PACMAN:
				raa = rand()%2;
				if (raa == 0)
				{
					message = "became a dot to";
					message2 = "'s Pac-Man";
				}
				else if (raa == 1)
				{
					message = "was eaten by";
					message2 = "'s Pac-Man";
				}
				else if (raa == 2)
				{
					message = "wondered what the big yellow ball was, but found out it was";
					message2 = "'s Pac-Man";
				}
				break;
			case MOD_PLASMA:
				message = "was plasmafied from";
				message2 = "'s plasma bolts";
				break;
			case MOD_MACE:
				message = "was smashed into the ground by";
				message2 = "'s giant mace";
				break;
			case MOD_DRILL:
				message = "got a whole new set of holes thanks to";
				message2 = "'s drillbit";
				break;
			case MOD_LIGHTNING_BOLTDISCHARGE:
				message = "was in the water at the wrong time, and";
				message2 = "'s lightning bolt knew when to strike";
				break;
			case MOD_THUNDERBOLT:
				message = "was electrified, struck down and burnt by";
				message2 = "'s Thunderbolt";
				break;
			case MOD_PLASMA_SPLASH:
				message = "became ashes, thanks to the underwater power of";
				message2 = "'s plasma bolts";
				break;

			case MOD_FIRESMALL:
				message = "was burnt by";
				message2 = "'s fire";
				break;
			case MOD_BURNT:
				message = "was burned alive from";
				message2 = "'s fire";
				break;

			case MOD_GAS:
				message = "ate";
				message2 = "'s toxic gas";
				break;

			case MOD_ACIDDROP:
				message = "felt the burn of";
				message2 = "'s acid cloud";
				break;

			case MOD_DISKETTE:
				message = "wasn't a floppy drive, but";
				message2 = " didn't know that";
				break;

			case MOD_DISK:
				raa = rand()%2;
				if (raa == 0)
				{
					message = "ate";
					message2 = "'s favorite music CD";
				}
				else if (raa == 1)
				{
					message = "choked on";
					message2 = "'s own DVD";
				}
				else
				{
					message = "found out that";
					message2 = "'s CD isn't rewritable";
				}
				break;

			case MOD_SPIKEBOMB_HIT:
				message = "was spiked by";
				break;
			case MOD_LAVABALL:
				message = "was burnt by";
				message2 = "'s falling lavaball";
				break;
			case MOD_LAVABALL_SPLASH:
				message = "went too close to";
				message2 = "'s volcano's spewing lavaballs";
				break;

			case MOD_SHOCKWAVE:
				message = "had a bad migrane from";
				message2 = "'s soundwave";
				break;

			case MOD_ARTILLERY:
				message = "walked into";
				message2 = "'s artillery shell";
				break;
			case MOD_BLADE:
				message = "was ran over by";
				message2 = "'s blade";
				break;
			case MOD_DKHAMMER:
				message = "was smashed by";
				message2 = "'s donkey kong hammer";
				break;
			case MOD_DETONATOR:
				message = "went too close to";
				message2 = "'s personal kamikaze detonated bomb";
				break;
			case MOD_HOOK:
				message = "was hooked by";
				break;

//===============
//ROGUE
			case MOD_CHAINFIST:
				message = "was shredded by";
				message2 = "'s ripsaw";
				break;
			case MOD_DISINTEGRATOR:
				message = "lost his grip courtesy of";
				message2 = "'s disintegrator";
				break;
			case MOD_ETF_RIFLE:
				message = "was perforated by";
				break;
			case MOD_HEATBEAM:
				message = "was scorched by";
				message2 = "'s plasma beam";
				break;
			case MOD_TESLA:
				message = "was enlightened by";
				message2 = "'s tesla mine";
				break;
			case MOD_PROX:
				message = "got too close to";
				message2 = "'s proximity mine";
				break;
			case MOD_TRACKER:
				message = "was annihilated by";
				message2 = "'s disruptor";
				break;
//ROGUE
//===============
			// RAFAEL 14-APR-98
			case MOD_RIPPER:
				message = "ripped to shreds by";
				message2 = "'s ripper gun";
				break;
			case MOD_PHALANX:
				message = "was evaporated by";
				break;
			case MOD_TRAP:
				message = "caught in trap by";
				break;
			// END 14-APR-98
// CYBERSLASH: more pig obituries
			case MOD_PIG_GIB:
				message = "spontaeneously combusted thanks to";
				break;
			case MOD_PIG_REMOVE:
				message = "got moved to another plane of reality by";
				break;
			case MOD_PIG_TRANSFORM:
				message = "is all of a sudden not feeling quite right thanks to";
				break;
// CYBERSLASH
			case MOD_BLASTER2:
				message = "died from";
				message2 = "'s neutron particle";
				break;
			case MOD_HEATLASER:
				message = "exploded from";
				message2 = "'s hot laser";
				break;
			case MOD_CHAINLINK:
				message = "tried to go through";
				message2 = "'s laser setup";
				break;
			case MOD_STAPLE:
				message = "was stapled by";
				message2 = "'s stapler";
				break;
			case MOD_BEAMCANNON3:
				message = "was at the wrong end of";
				message2 = "'s slug explosion";
				break;
			case MOD_BEAMCANNON2:
				message = "went too close to the end of";
				message2 = "'s Beam Cannon laser";
				break;
			case MOD_BEAMCANNON:
				message = "was burned by";
				message2 = "'s beam";
				break;

			case MOD_AGM_FEEDBACK:
				message = "crossed streams with";
				message2 = "'s AGM";
				break;

			case MOD_AGM_BEAM_REF:
				message = "'s AG Manipulator beam was reflected off";
				message2 = (random()<0.8)?" ":" ... bwahaha!";
				break;

			case MOD_AGM_HIT:
				message = "was used for bowling practice by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_FLING:
				message = "was smeared across the map by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_SMASH:
				message = "was smashed into a hard surface by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_THROW:
				message = "was used to paint the walls by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_DROP:
				message = (random()<0.5)?"was used to mop the floor by":"was spread across the floor by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_LAVA_HELD:
				message = (random()<0.5)?"was dunked in lava like a donut by":"was forced to drink lava by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_LAVA_DROP:
				message = "was tossed into the lava by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_SLIME_HELD:
				message = (random()<0.5)?"was dunked in slime like a biscuit by":"was forced to drink slime by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_SLIME_DROP:
				message = "was tossed into the slime by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_WATER_HELD:
				message = "was drowned like a rat by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_TRIG_HURT:
				message = "was sent to the wrong place by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_TARG_LASER:
				message = "was shown a very bright light by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_DISRUPT:
				if (random() < 0.5)
				{
					if (IsFemale(self))
						message = "had her insides scrambled by";
					else
						message = "had his insides scrambled by";
				}
				else
					message = "was sucked dry by";
				message2 = "'s cellular disruptor";
				break;
			case MOD_LZAP:
				message = "was zapped by";
				message2 = "'s chain lightning";
				break;
			case MOD_VEHICLE:
				message = "was flattened by";
				message2 = "'s vehicle";
				break;
			}

			if (message)
			{
				safe_bprintf (PRINT_MEDIUM,"%s %s %s%s\n", selfname, message, atkname, message2);

				if (deathmatch->value && attacker->client)
				{
					attacker->get_monster_ent = self;

					if (ff)
						Frag_Get(attacker, -1);
					else
						Frag_Get(attacker,1);

					attacker->get_monster_ent = NULL;
				}
				return;
			}
		}
	}

	if (attacker)
	{
		safe_bprintf (PRINT_MEDIUM,"%s was killed by %s.\n", selfname, atkname);
	}
	else
		safe_bprintf (PRINT_MEDIUM, "%s died.\n", selfname);

	if (deathmatch->value)
		Frag_Get(self, -1);
}

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;

	if (!attacker->client && attacker->name)
	{
		MonsterObituary (self, inflictor, attacker);
		return;
	}

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
			message = " suicides";
			break;
		case MOD_FALLING:
			message = " cratered";
			break;
		case MOD_CRUSH:
			message = " was squished";
			break;
		case MOD_WATER:
			message = " sank like a rock";
			break;
		case MOD_SLIME:
			message = " melted";
			break;
		case MOD_LAVA:
			message = " does a back flip into the lava";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = " blew up";
			break;
		case MOD_EXIT:
			message = " found a way out";
			break;
		case MOD_TARGET_LASER:
			message = " saw the light";
			break;
		case MOD_TARGET_BLASTER:
			message = " got blasted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = " was in the wrong place";
			break;
		}
		if (attacker == self)
		{
			int i;
//			char *gen;

			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = " tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (IsNeutral(self))
					message = " tripped on its own grenade";
				else if (IsFemale(self))
					message = " tripped on her own grenade";
				else
					message = " tripped on his own grenade";
				break;
			case MOD_R_SPLASH:
				if (IsNeutral(self))
					message = " blew itself up";
				else if (IsFemale(self))
					message = " blew herself up";
				else
					message = " blew himself up";
				break;
			case MOD_BFG_BLAST:
				message = " should have used a smaller gun";
				break;
			case MOD_RAILGUN_BOUNCE:
				message = "'s railgun bounced back the wrong way";
				break;
			case MOD_MACHINEGUN:
				if (random() < 0.5)
					message = " shot a bullet the wrong way";
				else
					message = " saw the bullet bounce back";
				break;
			case MOD_FORCE_SPLASH:
				if (IsNeutral(self))
					message = " couldn't escape its own area grenade";
				else if (IsFemale(self))
					message = " couldn't escape her own area grenade";
				else
					message = " couldn't escape his own area grenade";
				break;
			case MOD_JACK_SPLASH:
				if (IsNeutral(self))
					message = " stepped over its own jacks";
				else if (IsFemale(self))
					message = " stepped over her own jacks";
				else
					message = " stepped over his own jacks";
				break;
			case MOD_JACK:
				if (IsNeutral(self))
					message = " stepped over its own jacks";
				else if (IsFemale(self))
					message = " stepped over her own jacks";
				else
					message = " stepped over his own jacks";
				break;
			case MOD_HELD_FORCE:
				message = " felt the force";
				break;
// CYBERSLASH: more pig obituries
			case MOD_PIG_GRAVITY:
				message = " tried to exit through the stratosphere";
				break;
// CYBERSLASH
			default:
				if (IsNeutral(self))
					message = " killed itself";
				else if (IsFemale(self))
					message = " killed herself";
				else
					message = " killed himself";
				break;
			case MOD_FORCE_FALLING:
				message = " played with the area grenade too much";
				break;
			case MOD_LIGHTNING_DIRECTDISCHARGE:
				i = rand()%3;
				if (i == 0)
					message = " discharged in the water";
				else if (i == 1)
				{
					if (IsNeutral(self))
						message = " gibbed from its own bolt";
					else if (IsFemale(self))
						message = " gibbed from her own bolt";
					else
						message = " gibbed from his own bolt";
				}
				else if (i == 2)
				{
					if (IsNeutral(self))
						message = " watched its own gibs fly after the discharge";
					else if (IsFemale(self))
						message = " watched her own gibs fly after the discharge";
					else
						message = " watched his own gibs fly after the discharge";
				}
				else if (i == 3)
				{
					message = " didn't realize lighting and water don't mix";
				}
				break;
			case MOD_PLASMA_SPLASH:
				message = " shot his Plasma Rifle at the wrong place and the wrong time";
				break;

			case MOD_FIRESMALL:
				message = " burnt himself alive on his own flames";
				break;
			case MOD_BURNT:
				message = " burned on his own fire";
				break;

			case MOD_GAS:
				message = " choked on his own toxic fumes";
				break;

			case MOD_LAVABALL_SPLASH:
				message = " was splashed by his own lavaballs";
				break;

			case MOD_INFESTED:
				if (IsFemale(self))
					message = " died by her own infest";
				else
					message = " died by his own infest";
				break;

			case MOD_ARTILLERY:
				message = " ran into the wrong artillery shell";
				break;
			case MOD_DETONATOR:
				message = " knew his life was over, and the button got him out";
				break;
			case MOD_TRAP:
			 	message = " sucked into his own trap";
				break;
			case MOD_CHAINLINK:
				message = " tried to go through his own lasers";
				break;
			}
		}
		if (message)
		{
			safe_bprintf (PRINT_MEDIUM, "%s%s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
				Frag_Get(self, -1);
			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
			int raa;
			switch (mod)
			{
			case MOD_BLASTER:
				message = "was blasted by";
				break;
			case MOD_STAR:
				message = "went too close to";
				message2 = " while he had a star";
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
			case MOD_RAILGUN_BOUNCE:
				message = "took a rail from";
				message2 = "'s rail bounce";
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
			case MOD_INFESTED:
				message = "was infested by";
				message2 = "'s grenade";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
			case MOD_RUBBERBALL:
				message = "was ran over by";
				message2 = "'s little rubber friend";
				break;
			case MOD_DEVASTATOR:
				message = "was pulverised from";
				message2 = "'s Devastator.";
				break;
			case MOD_ROCKETPOD:
				message = "took pods in the face from";
				message2 = "'s Rocket Pod launcher.";
				break;
			case MOD_SHARD:
				message = "ate a glass shard from";
				message2 = "'s shard cannon.";
				break;
			case MOD_GRENADELASER:
				message = "ran right into";
				message2 = "'s laser grenade.";
				break;
			case MOD_NEEDLER:
				message = "was needled by";
				break;
			case MOD_FIREFLY:
				message = "was chased down by";
				message2 = "'s fireflies.";
				break;
			case MOD_FOOBY:
				message = "choked on";
				message2 = "'s kamikaze watermelon.";
				break;
			case MOD_BALLOON:
				message = "got drenched and drowned from";
				message2 = "'s water balloon.";
				break;
			case MOD_SCREWDRIVER:
				message = "was driven into the ground from";
				message2 = "'s screwdriver projectiles.";
				break;

			case MOD_WOMD:
				raa = rand()%5;

				switch (raa)
				{
				case 0:
					message = "was atomized from";
					message2 = "'s Weapon of Mass Destruction";
					break;
				case 1:
					message = "was pulverized from";
					message2 = "'s WOMD";
					break;
				case 2:
					message = "has body parts moulded to the ground by";
					message2 = "'s Weapon of Mass Destruction";
					break;
				case 3:
					message = "decomposed many years in advanced, thanks to";
					message2 = "'s WOMD";
					break;
				case 4:
					message = "crumbled into ashes from";
					message2 = "'s Weapon of Mass Destruction";
					break;
				case 5:
					message = "felt the wrath of";
					message2 = "'s WOMD";
					break;
				default:
					message = "was radiated by";
					message2 = "'s Weapon of Mass Destruction";
					break;
				}
				break;
			case MOD_BEE:
				message = "heard the buzzing from";
				message2 = "'s kamikaze bees.";
				break;
			case MOD_BEE_SPLASH:
				message = "was hit by";
				message2 = "'s bee splashes";
				break;
			case MOD_BASKETBALL:
				message = "forgot to catch";
				message2 = "'s basketball pass";
				break;
			case MOD_BEACHBALL:
				message = "didn't have a fun time at the beach thanks to";
				message2 = "'s beach ball.";
				break;
			case MOD_HELD_FORCE:
				message = "felt";
				message2 = "'s force";
				break;
			case MOD_FORCE_SPLASH:
				message = "couldn't withstand the force of";
				message2 = "'s area grenade";
				break;
			case MOD_JACK_SPLASH:
				message = "stepped on";
				message2 = "'s jacks";
				break;
			case MOD_JACK:
				message = "lost a game of jacks thrown by";
				break;
			case MOD_ZAPPED:
				message = "discovered that";
				if (IsNeutral(self))
					message2 = "'s weather-control system was against it";
				else if (IsFemale(self))
					message2 = "'s weather-control system was against her";
				else
					message2 = "'s weather-control system was against him";
				break;
			case MOD_PACMAN:
				raa = rand()%2;
				if (raa == 0)
				{
					message = "became a dot to";
					message2 = "'s Pac-Man";
				}
				else if (raa == 1)
				{
					message = "was eaten by";
					message2 = "'s Pac-Man";
				}
				else if (raa == 2)
				{
					message = "wondered what the big yellow ball was, but found out it was";
					message2 = "'s Pac-Man";
				}
				break;
			case MOD_PLASMA:
				message = "was plasmafied from";
				message2 = "'s plasma bolts";
				break;
			case MOD_MACE:
				message = "was smashed into the ground by";
				message2 = "'s giant mace";
				break;
			case MOD_DRILL:
				message = "got a whole new set of holes thanks to";
				message2 = "'s drillbit";
				break;
			case MOD_LIGHTNING_BOLTDISCHARGE:
				message = "was in the water at the wrong time, and";
				message2 = "'s lightning bolt knew when to strike";
				break;
			case MOD_THUNDERBOLT:
				message = "was electrified, struck down and burnt by";
				message2 = "'s Thunderbolt";
				break;
			case MOD_PLASMA_SPLASH:
				message = "became ashes, thanks to the underwater power of";
				message2 = "'s plasma bolts";
				break;

			case MOD_FIRESMALL:
				message = "was burnt by";
				message2 = "'s fire";
				break;
			case MOD_BURNT:
				message = "was burned alive from";
				message2 = "'s fire";
				break;

			case MOD_GAS:
				message = "ate";
				message2 = "'s toxic gas";
				break;

			case MOD_ACIDDROP:
				message = "felt the burn of";
				message2 = "'s acid cloud";
				break;

			case MOD_DISKETTE:
				message = "wasn't a floppy drive, but";
				message2 = " didn't know that";
				break;

			case MOD_DISK:
				raa = rand()%2;
				if (raa == 0)
				{
					message = "ate";
					message2 = "'s favorite music CD";
				}
				else if (raa == 1)
				{
					message = "choked on";
					message2 = "'s own DVD";
				}
				else
				{
					message = "found out that";
					message2 = "'s CD isn't rewritable";
				}
				break;

			case MOD_SPIKEBOMB_HIT:
				message = "was spiked by";
				break;
			case MOD_LAVABALL:
				message = "was burnt by";
				message2 = "'s falling lavaball";
				break;
			case MOD_LAVABALL_SPLASH:
				message = "went too close to";
				message2 = "'s volcano's spewing lavaballs";
				break;

			case MOD_SHOCKWAVE:
				message = "had a bad migrane from";
				message2 = "'s soundwave";
				break;

			case MOD_ARTILLERY:
				message = "walked into";
				message2 = "'s artillery shell";
				break;
			case MOD_BLADE:
				message = "was ran over by";
				message2 = "'s blade";
				break;
			case MOD_DKHAMMER:
				message = "was smashed by";
				message2 = "'s donkey kong hammer";
				break;
			case MOD_DETONATOR:
				message = "went too close to";
				message2 = "'s personal kamikaze detonated bomb";
				break;
			case MOD_HOOK:
				message = "was hooked by";
				break;

//===============
//ROGUE
			case MOD_CHAINFIST:
				message = "was shredded by";
				message2 = "'s ripsaw";
				break;
			case MOD_DISINTEGRATOR:
				message = "lost his grip courtesy of";
				message2 = "'s disintegrator";
				break;
			case MOD_ETF_RIFLE:
				message = "was perforated by";
				break;
			case MOD_HEATBEAM:
				message = "was scorched by";
				message2 = "'s plasma beam";
				break;
			case MOD_TESLA:
				message = "was enlightened by";
				message2 = "'s tesla mine";
				break;
			case MOD_PROX:
				message = "got too close to";
				message2 = "'s proximity mine";
				break;
			case MOD_TRACKER:
				message = "was annihilated by";
				message2 = "'s disruptor";
				break;
//ROGUE
//===============
			// RAFAEL 14-APR-98
			case MOD_RIPPER:
				message = "ripped to shreds by";
				message2 = "'s ripper gun";
				break;
			case MOD_PHALANX:
				message = "was evaporated by";
				break;
			case MOD_TRAP:
				message = "caught in trap by";
				break;
			// END 14-APR-98
// CYBERSLASH: more pig obituries
			case MOD_PIG_GIB:
				message = "spontaeneously combusted thanks to";
				break;
			case MOD_PIG_REMOVE:
				message = "got moved to another plane of reality by";
				break;
			case MOD_PIG_TRANSFORM:
				message = "is all of a sudden not feeling quite right thanks to";
				break;
// CYBERSLASH
			case MOD_BLASTER2:
				message = "died from";
				message2 = "'s neutron particle";
				break;
			case MOD_HEATLASER:
				message = "exploded from";
				message2 = "'s hot laser";
				break;
			case MOD_CHAINLINK:
				message = "tried to go through";
				message2 = "'s laser setup";
				break;
			case MOD_STAPLE:
				message = "was stapled by";
				message2 = "'s stapler";
				break;
			case MOD_BEAMCANNON3:
				message = "was at the wrong end of";
				message2 = "'s slug explosion";
				break;
			case MOD_BEAMCANNON2:
				message = "went too close to the end of";
				message2 = "'s Beam Cannon laser";
				break;
			case MOD_BEAMCANNON:
				message = "was burned by";
				message2 = "'s beam";
				break;

			case MOD_AGM_FEEDBACK:
				message = "crossed streams with";
				message2 = "'s AGM";
				break;

			case MOD_AGM_BEAM_REF:
				message = "'s AG Manipulator beam was reflected off";
				message2 = (random()<0.8)?" ":" ... bwahaha!";
				break;

			case MOD_AGM_HIT:
				message = "was used for bowling practice by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_FLING:
				message = "was smeared across the map by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_SMASH:
				message = "was smashed into a hard surface by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_THROW:
				message = "was used to paint the walls by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_DROP:
				message = (random()<0.5)?"was used to mop the floor by":"was spread across the floor by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_LAVA_HELD:
				message = (random()<0.5)?"was dunked in lava like a donut by":"was forced to drink lava by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_LAVA_DROP:
				message = "was tossed into the lava by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_SLIME_HELD:
				message = (random()<0.5)?"was dunked in slime like a biscuit by":"was forced to drink slime by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_SLIME_DROP:
				message = "was tossed into the slime by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_WATER_HELD:
				message = "was drowned like a rat by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_TRIG_HURT:
				message = "was sent to the wrong place by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_TARG_LASER:
				message = "was shown a very bright light by";
				message2 = "'s AGM";
				break;

			case MOD_AGM_DISRUPT:
				if (random() < 0.5)
				{
					if (IsFemale(self))
						message = "had her insides scrambled by";
					else
						message = "had his insides scrambled by";
				}
				else
					message = "was sucked dry by";
				message2 = "'s cellular disruptor";
				break;
			case MOD_LZAP:
				message = "was zapped by";
				message2 = "'s chain lightning";
				break;
			case MOD_VEHICLE:
				message = "was flattened by";
				message2 = "'s vehicle";
				break;
			}			

			if (message)
			{
				safe_bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if (deathmatch->value)
				{
					if (ff)
						Frag_Get(attacker, -1);
					else
						Frag_Get(attacker,1);
				}
				return;
			}
		}
	}
//	gi.dprintf ("%i\n", mod);
	safe_bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (deathmatch->value)
		Frag_Get(self, -1);
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
	if (item && (strcmp (item->pickup_name, "Saw-toothed Hammer") == 0))
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Battery-Operated Drill") == 0))
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Screwdriver Launcher") == 0))
		item = NULL;
	if (item && !item->world_model)
		item = NULL;
	// Holy smoking flying cows Batman, it's not a main item!
	// Drop the main item rather than the sub-item
	/*if (item && item->owned_item)
		item = FindItem(item->owned_item);*/

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

void RemoveInfests (edict_t *infested)
{
	edict_t *yuck;
	for (yuck = g_edicts+1; yuck < &g_edicts[game.maxentities]; yuck++) 
	{
		if (!yuck->inuse)
			continue;
		if (yuck->infester && yuck->target_ent == infested) 
		{
			G_FreeEdict (yuck);
		}
	}
}

qboolean IsInfested (edict_t *infested)
{
	edict_t *yuck;
	for (yuck = g_edicts+1; yuck < &g_edicts[game.maxentities]; yuck++) 
	{
		if (!yuck->inuse)
			continue;
		if (yuck->infester && yuck->target_ent == infested) 
		{
			return true;
		}
	}
	return false;
}
/*
==================
player_die
==================
*/
void KillPlayerLinks (edict_t *p);

void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	VectorClear (self->avelocity);
	VectorCopy (self->s.angles, self->tempvec);

	KillPlayerLinks (self);

	if (self->vehicle)
		T_Damage (self->vehicle, self, self, vec3_origin, self->vehicle->s.origin, vec3_origin, 999, 0, 0, 0);

	if (!self->is_bot)
	{
		self->takedamage = DAMAGE_YES;
		self->movetype = MOVETYPE_TOSS;
	}
	// Paril
	self->chasetarget = NULL; // New line!

	self->s.modelindex2 = 0;	// remove linked weapon model
//ZOID
	self->s.modelindex3 = 0;	// remove linked ctf flag
//ZOID

	DestroyMagnets(self);

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->forcer = NULL;
	self->maxs[2] = -8;

	if ( self->flashlight )
	{
		G_FreeEdict(self->flashlight);
		self->flashlight = NULL;
	}

	self->client->agm_charge = 0;
	self->client->agm_showcharge = false;
	self->client->agm_tripped = false;
	self->client->agm_on = false;
	self->client->agm_push = false;
	self->client->agm_pull = false;
	self->held_by_agm = false;
	self->flung_by_agm = false;
	self->thrown_by_agm = false;

	if (self->client->agm_target != NULL)
	{
		self->client->agm_target->held_by_agm = false;
		self->client->agm_target->flung_by_agm = false;
		self->client->agm_target->thrown_by_agm = true;
		self->client->agm_target = NULL;
	}

	// Paril: If the player has not specified dest, just kill it.
	// Seems more efficient?
	if (self->client->resp.artillery && !self->client->resp.artillery_dest)
	{
		G_FreeEdict (self->client->resp.artillery);
		G_FreeEdict (self->client->resp.artillery_base);
		self->client->resp.artillery = NULL;
		self->client->resp.artillery_base = NULL;
		self->client->resp.artillery_placed = 0;
	}

	if (self->client->clone1)
		G_FreeEdict (self->client->clone1);
	if (self->client->clone2)
		G_FreeEdict (self->client->clone2);
	if (self->client->clone3)
		G_FreeEdict (self->client->clone3);
	if (self->client->clone4)
		G_FreeEdict (self->client->clone4);

	self->client->clone1 = self->client->clone2 = self->client->clone3 = self->client->clone4 = NULL;
	self->fire_framenum = -1;
	self->fire_hurtframenum = -1;
	if (self->fire_entity)
		G_FreeEdict (self->fire_entity);

//	self->solid = SOLID_NOT;
	if (!self->is_bot)
		self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);
	self->client->resp.deaths++;
//ZOID
		// if at start and same team, clear
		if (ctf->value && meansOfDeath == MOD_TELEFRAG &&
			self->client->resp.ctf_state < 2 &&
			self->client->resp.ctf_team == attacker->client->resp.ctf_team) {
			attacker->client->resp.score--;
			self->client->resp.ctf_state = 0;
		}

		CTFFragBonuses(self, inflictor, attacker);
//ZOID
		TossClientWeapon (self);
//ZOID
		CTFPlayerResetGrapple(self);
		CTFDeadDropFlag(self);
		CTFDeadDropTech(self);
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
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;
	self->client->dkhammer_framenum = 0;
	self->client->reg_framenum = 0;
	self->client->boots_framenum = 0;
	self->client->cloak_framenum = 0;

	// Remove infests
	RemoveInfests(self);

	/*ATTILA begin*/
	if ( Jet_Active(self) )
	{
		Jet_BecomeExplosion( self, damage );
		/*stop jetting when dead*/
		self->client->Jet_framenum = 0;
	}
	else if (self->health < -40)
	{	// gib
		gi.sound (self, CHAN_BODY, SoundIndex ("misc/udeath.wav"), 1, ATTN_NORM, 0);

		if (!(self->flags & FL_NOGIB))
		{
			for (n= 0; n < 4; n++)
				ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		}

		if (self->is_bot)
			ThrowClientHead (self, damage);
		else
			ThrowClientHead2 (self);

		self->flags &= ~FL_NOGIB;
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
			gi.sound (self, CHAN_VOICE, SoundIndex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
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

//	if (CheckBan(shotgun_ban, SHOTGUNBAN_CO2CANNON))
//	{
		item = FindItem("Shells");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 15;
		item = FindItem("CO2 Cannon");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;
//	}
//	if (CheckBan(shotgun_ban, BLASTERBAN_SCREWDRIVER))
//	{
		item = FindItem("Screwdriver Launcher");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;
//	}
//	if (CheckBan(shotgun_ban, BLASTERBAN_CHAINFIST))
//	{
		item = FindItem("Chainfist");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;
//	}
//	if (CheckBan(shotgun_ban, BLASTERBAN_SAWTOOTHEDHAMMER))
	//{
		item = FindItem("Saw-toothed Hammer");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;
	//}
	//if (CheckBan(shotgun_ban, BLASTERBAN_BATTERYOPERATEDDRILL))
	//{
		item = FindItem("Battery-Operated Drill");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;
	//}
//	if (CheckBan(shotgun_ban, BLASTERBAN_RADIODETONATOR))
//	{
		item = FindItem("Radio Detonator");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;
	//}
//	if (CheckBan(shotgun_ban, BLASTERBAN_NEUTRONBLASTER))
//	{
		item = FindItem("Neutron Blaster");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;
	//}
		item = FindItem("Porta-Puddle");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;

	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	client->pers.weapon = item;
//ZOID
	client->pers.lastweapon = item;
//ZOID

//ZOID
	item = FindItem("Grapple");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
//ZOID

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
//ZOID
	int ctf_team = client->resp.ctf_team;
	qboolean id_state = client->resp.id_state;
//ZOID

	memset (&client->resp, 0, sizeof(client->resp));
	
//ZOID
	client->resp.ctf_team = ctf_team;
	client->resp.id_state = id_state;
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
	for (;;)
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

//=====================================================
void SV_SetClientAnimation(edict_t *ent,int startframe,int endframe,int priority) {
  if (!ent->client) return;
  ent->client->anim_priority=priority;
  ent->s.frame=startframe;
  ent->client->anim_end=endframe;
  gi.linkentity(ent);
}

//========================================================
//============== PLAYER GIB HANDLING STUFF ===============
//========================================================

//========================================================
void gib_touch2(edict_t *gib,edict_t *other,cplane_t *plane,csurface_t *surf) {
  if (other==world) return;
  G_FreeEdict(gib);
}

//========================================================
void gib_think2(edict_t *gib) {

  if (gib->s.effects!=EF_FLIES) // skip if already has flies effects
    if (gib->random<=5.0) {
      if (VectorLength(gib->velocity)<1) // only start flies when gib stops moving
        if (!(gi.pointcontents(gib->s.origin) & MASK_WATER)) { // no flies in water
          //gib->s.effects=EF_FLIES;
          gib->wait=level.time + 15.0; } } // extend timer to show flies effect

  if (gib->wait<=level.time) {
    G_FreeEdict(gib);
    return; }

  gib->nextthink=level.time + 0.2;
}

//========================================================
void ThrowGib2(edict_t *self,char *gibname) 
{
	vec3_t origin;
	edict_t *gib;

	if ((self->flags & FL_NOGIB))
		return;
	
	level_gibs++;
	CheckGibOverflow();
	
	gib=G_Spawn();
	gi.setmodel(gib,gibname);
	gib->solid=SOLID_BBOX;
	gib->s.effects|=EF_GIB;
	gib->flags|=FL_NO_KNOCKBACK;
	gib->takedamage=DAMAGE_NO;
	gib->random=(float)(rand()&10);
	
	//----------------------------------------------------------//
	if (!strcmp(gibname,"models/objects/gibs/sm_meat/tris.md2")
		|| !strcmp(gibname,"models/objects/gibs/bone/tris.md2")
		|| !strcmp(gibname,"models/objects/gibs/bone2/tris.md2"))
		gib->movetype=MOVETYPE_TOSS; // these gib models don't bounce
	else
		gib->movetype=MOVETYPE_BOUNCE;

	gib->movetype = MOVETYPE_BOUNCENOSTOP;
	
	//----------------------------------------------------------//
	VectorCopy(self->s.origin,origin);
	gib->s.origin[0]=origin[0]+crandom()*self->size[0];
	gib->s.origin[1]=origin[1]+crandom()*self->size[1];
	gib->s.origin[2]=(rand()%10)+origin[2]+crandom()*self->size[2];
	//if (self->velocity[0] || self->velocity[1])
	//  VectorCopy(self->velocity,gib->velocity); // make gib fly in player's last direction
	//else
    VectorSet(gib->velocity,crandom()*1300,crandom()*1300,0); // else random directions
	if (self->velocity[2]>=0) gib->velocity[2]=900;
		VectorScale(gib->velocity,random(),gib->velocity);
	//----------------------------------------------------------//
	
	gib->wait=level.time + 10.0; // free gib model in 10 secs
	
	gib->touch=gib_touch2; // free if gib is touched
	
	gib->think=gib_think2;
	gib->nextthink=level.time + 0.1;
	gib->classname = "gib";
	gib->is_gib = 1;

	gi.linkentity(gib);
}

//========================================================
//============= THROWING BODY PARTS AROUND ===============
//========================================================

//========================================================
void ThrowBodyParts(edict_t *self) { // WON'T EXIST
int n;
int mult;

if ((self->flags & FL_NOGIB))
return;

	if (meansOfDeath == MOD_ROCKET)
		mult = 3;
	else
		mult = 1;

  gi.sound(self,4,SoundIndex("misc/udeath.wav"),1,ATTN_NORM, 0);

  // Feel free to change the amounts of bodyparts tossed..

  for (n=0;n<(2 * mult);n++)
    ThrowGib2(self,"models/objects/gibs/sm_meat/tris.md2");
  for (n=0;n<2;n++)
    ThrowGib2(self,"models/objects/gibs/bone/tris.md2");
  for (n=0;n<1;n++)
    ThrowGib2(self,"models/objects/gibs/skull/tris.md2");
  for (n=0;n<2;n++)
    ThrowGib2(self,"models/objects/gibs/arm/tris.md2");
  for (n=0;n<1;n++)
    ThrowGib2(self,"models/objects/gibs/chest/tris.md2");
  for (n=0;n<2;n++)
    ThrowGib2(self,"models/objects/gibs/leg/tris.md2");
  for (n=0;n<2;n++)
    ThrowGib2(self,"models/objects/gibs/bone2/tris.md2");
  for (n=0;n<(2 * mult-1);n++)
    ThrowGib2(self,"models/objects/gibs/sm_meat/tris.md2");
}

//========================================================
void ThrowClientHead2(edict_t *self) {
vec3_t vbottom;

  if (self->svflags & SVF_NOCLIENT) {
    G_FreeEdict(self);
    return; }

  VectorCopy(self->s.origin,vbottom);
  vbottom[2]-=(abs(self->mins[2])+3.0); // check contents at foot level

  // Only toss body parts if player DID NOT die in lava/slime
  if (!(gi.pointcontents(vbottom) & (CONTENTS_LAVA|CONTENTS_SLIME)))
  {
    ThrowBodyParts(self);
    ThrowBodyParts(self);
    ThrowBodyParts(self);
  }

  self->movetype = MOVETYPE_BOUNCENOSTOP;

  self->svflags|=SVF_NOCLIENT;
  VectorSet(self->velocity,crandom()*1300,crandom()*1300,0); // else random directions
	if (self->velocity[2]>=0) self->velocity[2]=900;
		VectorScale(self->velocity,random(),self->velocity);

		self->velocity[2] = 1200;


  SV_SetClientAnimation(self,0,0,5);
}

//========================================================
//============== PLAYER BODY QUEUE HANDLING ==============
//========================================================

// You may already have this function in your source..
//=====================================================
void G_MuzzleFlash(int rec_no,vec3_t origin,int flashnum) {
  gi.WriteByte(1);
  gi.WriteShort((short)rec_no);
  gi.WriteByte(flashnum);
  gi.multicast(origin,MULTICAST_PVS);
}

//========================================================
void body_free(edict_t *body) { // MIGHT NOT EXIST

  if (!(body->svflags & SVF_NOCLIENT)) // if player back in game
    G_MuzzleFlash(body-g_edicts,body->s.origin,11); // do sparkle effect when body is removed from game

  G_FreeEdict(body);
}

//========================================================
void body_think(edict_t *body) { // MIGHT NOT EXIST

  if ((body->wait<=level.time) || (body->owner && !(body->owner->svflags & SVF_NOCLIENT))) {
    body_free(body); // free body at moment player respawn back into game
    return; }

  body->nextthink=level.time + 0.1;
}

//========================================================
void body_touch(edict_t *body,edict_t *other,cplane_t *plane,csurface_t *surf) {
  if (other==world) return;
  G_FreeEdict(body); // disappear if touched by other
}

//========================================================
edict_t *CopyToBodyQue(edict_t *ent) {
edict_t *body;

  gi.unlinkentity(ent); // must unlink player's edict_t struct first! - don't unlink to see what happens.. pretty interesting...

  body=G_Spawn();
  body->owner=ent;
  body->s=ent->s;
  body->takedamage=DAMAGE_NO;
  body->s.number=body-g_edicts;
  body->svflags|=ent->svflags;
  VectorCopy(ent->mins,body->mins);
  VectorCopy(ent->maxs,body->maxs);
  VectorCopy(ent->absmin,body->absmin);
  VectorCopy(ent->absmax,body->absmax);
  VectorCopy(ent->size,body->size);
  body->solid=ent->solid;
  body->clipmask=ent->clipmask;
  body->movetype=ent->movetype;

  body->wait=level.time + 10.0; // auto-free in 10 secs if player doesn't rejoin game first

  body->touch=body_touch; // free body

  body->think=body_think;
  body->nextthink=level.time + 0.1;

  gi.linkentity(body);
  
  return body;
}



void respawn (edict_t *self)
{
	if (deathmatch->value || coop->value)
	{
		edict_t *yuck, *deadbody = NULL;

//JABot[start]
//		if (self->ai.is_bot){
//			BOT_Respawn (self);
//			return;
//		}
//JABot[end]
		if (self->is_bot)
		{
			Bot_Respawn(self);
			return;
		}

		// spectator's don't leave bodies
		if (self->movetype != MOVETYPE_NOCLIP)
			deadbody = CopyToBodyQue (self);

		for (yuck = g_edicts+1; yuck < &g_edicts[game.maxentities]; yuck++) {
			if (!yuck->inuse)
				continue;
			if (yuck->infester && yuck->target_ent == self) {
				yuck->target_ent = deadbody;
				//deadbody->s.effects |= 0x80000000;
			}
		}
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

//==============================================================


/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void Fire_NULL (edict_t *ent)
{
        vec3_t forward, right, offset, start, end, pos;
	edict_t *foundEnt = NULL;
	trace_t	tr;
	
	VectorSet(offset, 0, 0, ent->viewheight);
        AngleVectors (ent->client->v_angle, forward, right, NULL);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);

	VectorMA (start, 8192, forward, end);
	
	tr = gi.trace (start, NULL, NULL, end, ent, MASK_ALL);
	ent->client->ps.gunframe++;

	if (tr.ent && tr.ent->classname)
	{
		if (Q_stricmp(tr.ent->classname, "worldspawn") != 0)
			foundEnt = tr.ent;
	}
	/*
	if (!foundEnt)
	{
		edict_t	*item = NULL;
		float best, dist;
		
		best = 999;

		while ((item = findradius(item, tr.endpos, 16)) != NULL)
		{
			if (item->svflags & SVF_MONSTER)
				continue;
			if (item->solid != SOLID_TRIGGER)
				continue;
			VectorSubtract (tr.endpos, item->s.origin, pos);
			dist = VectorLength (pos);
			if (dist < best)
			{
				best = dist;
				foundEnt = item;
			}
		}
	}
	*/

	if (foundEnt)
	{
		char *targetname = "none";
		char *target = "none";
		char *message = "none";
		char sflags[384];
		char *model = "";

		Com_sprintf (sflags, sizeof(sflags), "(spawnflags %i)\n", foundEnt->spawnflags);
		
		if (foundEnt->spawnflags & SPAWNFLAG_NOT_DEATHMATCH)
			strcat(sflags, "*NOT DEATHMATCH*");
		if ((foundEnt->spawnflags & SPAWNFLAG_NOT_EASY)
		 && (foundEnt->spawnflags & SPAWNFLAG_NOT_MEDIUM)
		  && (foundEnt->spawnflags & SPAWNFLAG_NOT_HARD))
			strcat(sflags, "DEATHMATCH ONLY");
		else
		{
			if (foundEnt->spawnflags & SPAWNFLAG_NOT_EASY)
				strcat(sflags, "*NOT EASY*");
			if (foundEnt->spawnflags & SPAWNFLAG_NOT_MEDIUM)
				strcat(sflags, "*NOT MED*");
			if (foundEnt->spawnflags & SPAWNFLAG_NOT_HARD)
				strcat(sflags, "*NOT HARD*");
		}
				
		VectorCopy(foundEnt->s.origin, pos);

		if (foundEnt->target)
			target = foundEnt->target;

		if (foundEnt->targetname)
			targetname = foundEnt->targetname;
		if (foundEnt->model)
			model = foundEnt->model;
			
		if (foundEnt->svflags & SVF_MONSTER)
		{
			char *item = "none";
			
			if (foundEnt->item != NULL)
				item = foundEnt->item->classname;

			gi.centerprintf (ent, "foundEnt = %s\n origin = %s\n sFlags = %s\n item = %s\ntarget = %s\ntargetname = %s\n", foundEnt->classname, vtos(pos), sflags, item, target, targetname);
			return;
		}
		else if (foundEnt->message)
			message = foundEnt->message;

		gi.centerprintf (ent, "foundEnt = %s\n origin = %s\nsFlags = %s\n model = %s\n target = %s\ntargetname = %s\nmessage = %s\n", foundEnt->classname, vtos(pos), sflags, model, target, targetname, message);
		return;
	}

	vectoangles(tr.plane.normal, pos);
	gi.centerprintf (ent, "X-hair pos = %s\n normal = %s\n self angles =%s\n", vtos(tr.endpos), vtos(pos), vtos(ent->client->v_angle));

}
void Weapon_Null (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Fire_NULL);
}

gitem_t specialweapon[] = 
{
	{
		"do_not_put_in_map_lol", 
		NULL,
		NULL,
		NULL,
		Weapon_Null,
		NULL,
		0, 0,
		"models/weapons/v_blast/tris.md2",
		"i_blaster",
		"WEAPON EDITOR",
		3,
		0,
		NULL,
		IT_WEAPON,
		0,
		0,
		0,
		"",
		NULL
	}
};

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
	int y;
	edict_t *ye;
	char            userinfo[MAX_INFO_STRING];
//	char *wepname;
	gitem_t *newit;

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
	ent->infester = NULL;
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
	client->blindBase = 0;
	client->blindTime = 0;
	ent->frozen = 0;
	ent->DrunkTime = 0;
	ent->frozentime = 0;
	ent->client->pers.grenadetype = 1;
	ent->client->resp.awards.awards_base[AWARD_ACCURACY] = 0;
	ent->client->resp.awards.award_needed[AWARD_ACCURACY] = 0;
	ent->client->clones = 0;
	ent->s.renderfx |= RF_IR_VISIBLE;
	ent->burning_framenum = 0;
	ent->tracker_framenum = 0;
	ent->client->chamber = 0;
	ent->client->hint_framenum = 400;
	ent->headache_framenum = 0;
	// Check his modulate level.
	//stuffcmd (ent, "checkmodulate $gl_modulate\n");
	ent->fire_framenum = 0;
	ent->lagged = false;
	ent->latency_time = 0;
	ent->tempint = 0;

	RemoveInfests(ent);

	// Paril: Go through all entities, see if anything is stuck to player.
	// Free them if so.
	// FIXME: Inefficient?
	ye = &g_edicts[(int)maxclients->value+1];
	for ( y=maxclients->value+1 ; y<globals.num_edicts ; y++, ye++)
	{
		if (ye->movetype != MOVETYPE_STUCK)
			continue;

		if (ye->stuckentity == ent)
			G_FreeEdict (ye);
	}

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

	if (client->pers.weapon)
		client->ps.gunindex = ModelIndex(client->pers.weapon->view_model);
	else
		client->pers.weapon = &specialweapon[0];

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

	//JABot[start]
//	if( ent->ai.is_bot == true )
//		return;
	//JABot[end]
	
	if (iwm_gamemode->value == 1 && ent->client->resp.team == -1)
	{
		// start as 'observer'
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);
		OpenTeamMenu(ent, NULL);
		return;
	}
//ZOID
	if (ctf->value)
	{
		if (CTFStartClient(ent))
			return;
	}
//ZOID

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

	gi.linkentity (ent);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);

	ent->client->invincible_framenum = level.framenum + 30;

	memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));

	newit = FindItem(Info_ValueForKey (ent->client->pers.userinfo, "cg_startweap"));

	if (!newit) // Not found.
	{
		gi.dprintf ("Player %s has invalid item start, %s.\n", ent->client->pers.netname, Info_ValueForKey (userinfo, "cg_startweap"));
		safe_cprintf (ent, PRINT_HIGH, "Your cg_startweap variable is corrupt; please re-set it.\n");
	}
	else
	{
		if ( ent->client->pers.inventory[ITEM_INDEX(newit)]) 
		{
			client->pers.selected_item = ITEM_INDEX(newit);
			client->pers.weapon = newit;
			if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
				ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
		}
		else
			safe_cprintf (ent, PRINT_HIGH, "You can't spawn with a weapon you don't have!\n");
	}

	if (iwm_editor->value)
	{
		ent->s.modelindex2 = 0;
		ent->client->ps.gunindex = ModelIndex("models/weapons/v_blast/tris.md2");;
		ent->client->pers.weapon = &specialweapon[0];
	}
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
	ent->client->resp.team = -1;
	ent->client->resp.deaths = 0;

	// locate ent at a spawn point
	PutClientInServer (ent);
	safe_centerprintf (ent, "Welcome, %s, to IWM!\n\nType \"iwmhelp\" to navigate through\nmenus that will tell you how\nto play, or read the readme.\n\n\nVisit us at http://iwm.gaminggalaxy.net!\n", ent->client->pers.netname);

	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	//JABot[start]
//	AI_EnemyAdded(ent);
	//[end]

	safe_bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

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

			safe_bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
		}
	}
	safe_centerprintf (ent, "Welcome, %s, to IWM!\n\nType \"iwmhelp\" to navigate through\nmenus that will tell you how\nto play, or read the readme.\n\n\nVisit us at http://iwm.gaminggalaxy.net!\n", ent->client->pers.netname);

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
	char *h = Info_ValueForKey(userinfo, "cg_startweap");

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}

	// set name
	s = Info_ValueForKey (userinfo, "name");
	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	if (Q_stricmp(h, "") == 0 || h == NULL)
	{
		Info_SetValueForKey(userinfo, "cg_startweap", "Blaster");
		//gi.dprintf ("Player %s has null cg_startweap\n", ent->client->pers.netname);
	}

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
		//if (ent->client->ps.fov < 1)
		//	ent->client->ps.fov = 90;
	//	else if (ent->client->ps.fov > 160)
	//		ent->client->ps.fov = 160;
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
//	char *g;

	// check for a password
	value = Info_ValueForKey (userinfo, "password");
	if (*password->string && strcmp(password->string, "none") && 
		strcmp(password->string, value)) 
	{
		Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
		return false;
	}

	// check to see if they are on the banned IP list
	if (game.maxclients > 1)
	{
//		int i = 1;
//		edict_t *client = NULL;
		char *name = Info_ValueForKey (userinfo, "name");
		char *ip = Info_ValueForKey (userinfo, "ip");
		gi.dprintf ("%s @ %s connected\n", name, ip);
		//for (; i < game.maxclients; i++)
		//{
		//	client = &g_edicts[i];
		//	if (client->inuse)
		safe_bprintf (PRINT_CHAT, "=== %s connected ===.\n", name);
		//}
	}
	value = Info_ValueForKey (userinfo, "ip");
	if (SV_FilterPacket(value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
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
		ent->client->resp.id_state = false; 
//ZOID
		InitClientResp (ent->client);
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
	}

	ClientUserinfoChanged (ent, userinfo);

	//if (!Info_ValueForKey (ent->client->pers.userinfo, "cg_startweap"))
	//Info_SetValueForKey (ent->client->pers.userinfo, "cg_startweap", "Neutron Blaster" );

	ent->client->pers.connected = true;
	//safe_cprintf (ent, PRINT_HIGH, "We do not recommend that you download the whole mod from here; please\ngo to our website and download it from there: \nhttp://iwm.gaminggalaxy.net");
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
	edict_t *yuck;

	int		playernum;

	if (!ent->client)
		return;

	safe_bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

//ZOID
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);
//ZOID

//CW++
	ent->client->agm_on = false;
	ent->client->agm_pull = false;
	ent->client->agm_push = false;

	if (ent->client->agm_target != NULL)
	{
		ent->client->agm_target->held_by_agm = false;
		ent->client->agm_target->flung_by_agm = false;
		ent->client->agm_target = NULL;
	}
//CW--

	DestroyMagnets(ent);

	KillPlayerLinks(ent);

	if ( ent->flashlight )
	{
		G_FreeEdict(ent->flashlight);
		ent->flashlight = NULL;
	}

	// Paril: If the player has not specified dest, just kill it.
	// Seems more efficient?
	if (ent->client->resp.artillery && !ent->client->resp.artillery_dest)
	{
		G_FreeEdict (ent->client->resp.artillery);
		G_FreeEdict (ent->client->resp.artillery_base);
		ent->client->resp.artillery = NULL;
		ent->client->resp.artillery_base = NULL;
		ent->client->resp.artillery_placed = 0;
	}

	if (ent->client->clone1)
		G_FreeEdict (ent->client->clone1);
	if (ent->client->clone2)
		G_FreeEdict (ent->client->clone2);
	if (ent->client->clone3)
		G_FreeEdict (ent->client->clone3);
	if (ent->client->clone4)
		G_FreeEdict (ent->client->clone4);

	if (ent->client->resp.puddle_dest)
		G_FreeEdict (ent->client->resp.puddle_dest);
	if (ent->client->resp.puddle_tele)
		G_FreeEdict (ent->client->resp.puddle_tele);

	ent->client->clone1 = ent->client->clone2 = ent->client->clone3 = ent->client->clone4 = NULL;
	ent->fire_framenum = -1;
	ent->fire_hurtframenum = -1;
	if (ent->fire_entity)
		G_FreeEdict (ent->fire_entity);

	for (yuck = g_edicts+1; yuck < &g_edicts[game.maxentities]; yuck++) {
		if (!yuck->inuse)
			continue;
		if (yuck->infester && yuck->target_ent == ent)
			G_FreeEdict (yuck);
	}

	// Paril: Remove all artillery stuff.
	if (ent->client->resp.artillery)
	{
		G_FreeEdict (ent->client->resp.artillery);
		G_FreeEdict (ent->client->resp.artillery_base);
		ent->client->resp.artillery = NULL;
		ent->client->resp.artillery_base = NULL;
	}
	if (ent->client->resp.artillery_dest)
	{
		G_FreeEdict (ent->client->resp.artillery_dest);
		ent->client->resp.artillery_dest = NULL;
	}

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

	if (ent->is_bot)
		game.maxclients --;

	//JABot[start]
//	AI_EnemyRemoved (ent);
	//[end]
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
void RCCarUp(edict_t *self);
void RCCarDown(edict_t *self);
void RCCarStop(edict_t *self);
void CTFApplyRegeneration(edict_t *ent);

void UpdateRocketCam (edict_t *ent); // new line

// Paril: Is the player STILL stuck in a solid?
// Note: This code will only see if the player's origin is
// in a solid. If it isn't, we assume he can still hook/crouch/jump out of the spot.
void AreWeStillInSolidcommaspaceIfYesRespawnUs (edict_t *ent)
{
	if (gi.pointcontents(ent->s.origin) & CONTENTS_SOLID)
	{
		vec3_t old_origin;

		VectorCopy (ent->s.origin, old_origin);
		// Crap, we're still in a goddamn solid.
		// Let's get him out of there.
		// Note: If the player keeps getting stuck in solid, we
		// should find out how he gets in there...
		safe_cprintf (ent, PRINT_HIGH, "There you go, now stay out of that spot.\n");
		findspawnpoint(ent);
		gi.dprintf ("Player %s was stuck in solid and was teleported out from %s to %s\n", ent->client->pers.netname, vtos(old_origin), vtos(ent->s.origin));
	}
	ent->think4 = NULL;
	ent->nextthink4 = -1;
}

qboolean HasItem (edict_t *ent, char *item)
{
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem(item))])
		return true;
	else
		return false;
}

qboolean IsProjectile (edict_t *i);
void Calc_Arc (edict_t *ent);

void DeflectStuff (edict_t *ent)
{
	edict_t *blip = NULL; 
	vec3_t blipdir;
	float dist;

	while ((blip = findradius(blip, ent->s.origin, 400)) != NULL)
	{
		if (blip == ent)
			continue;
		if (!IsProjectile(blip))
			continue;
		if (!infront(ent, blip))
			continue;
		if (blip->owner == ent && !blip->deflected)
			continue;
		if (Q_stricmp(blip->classname, "block") == 0)
			continue;

		VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
		dist = VectorLength(blipdir);
		VectorNormalize(blipdir);
		

		VectorMA(blip->velocity, -(2*(80 - 160)), blipdir, blip->velocity);
		blip->velocity[0] -= crandom() * 5;
		blip->velocity[1] -= crandom() * 5;
		blip->velocity[2] -= random() * 5;

		// Paril: Remove owner from blip so it can hurt them back :)
		// Might cause problems on some entities!
		
		blip->deflected = true;
		blip->owner = ent;
		blip->prethink = Calc_Arc;
	}

	ent->nextthink2 = level.time + .1;
}

void StarKillNearby (edict_t *ent)
{
	edict_t *blip = NULL;

	while (blip = findradius(blip, ent->s.origin, 65))
	{
		if (!blip->takedamage)
			continue;

		// Instant kill!
		T_Damage (blip, ent, ent, vec3_origin, blip->s.origin, vec3_origin, 80, 0, DAMAGE_NO_ARMOR|DAMAGE_ENERGY, MOD_STAR);
	}
}

void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j;
	pmove_t	pm;

	if (IsLagged(ent) && random() < AmountToLag(ent))
		return;

//	if (random() < 0.6)
	//	return;

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

	if (ent->laz_vehicle)
		ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	else
		ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

	if (ent->client->chase_target) {
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	} else {

		// set up for pmove
		memset (&pm, 0, sizeof(pm));

		//ERASER END
		//Acrid freeze 3/99
		if (ent->frozen)
		{
			if( level.time < ent->frozentime )
			{  
				ent->client->ps.pmove.pm_type = PM_FREEZE;
				
				if (ent->client->buttons & BUTTON_ATTACK)//4/99 acrid
				{
					ent->client->buttons &= ~BUTTON_ATTACK;
				}
				
				//return;
			}
			else
			{
				unfreeze_player(ent);
			}
		}


		if (ent->vehicle)//botfreeze 3/99 needed still?
		{
		//	gi.dprintf ("Vehicle Freeze\n");
			client->ps.pmove.pm_type = PM_FREEZE;//3/99
		}
		else if (ent->client->teleing)
		{
		//	gi.dprintf ("Teleing Freeze\n");
			client->ps.pmove.pm_type = PM_FREEZE;//3/99
		}
		else if (ent->movetype == MOVETYPE_NOCLIP)
		{
		//	gi.dprintf ("PM_SPECTATOR\n");
			client->ps.pmove.pm_type = PM_SPECTATOR;
		}
		else if ((ent->s.modelindex != 255) && !client->pigged && !ent->client->cloak_turninvis_framenum)
		{
		//	gi.dprintf ("Modelindex not 255\n");
			client->ps.pmove.pm_type = PM_GIB;
		}
		else if (ent->deadflag)
			client->ps.pmove.pm_type = PM_DEAD;
		else if (ent->frozen)
			client->ps.pmove.pm_type = PM_DEAD;
		else
			client->ps.pmove.pm_type = PM_NORMAL;

// CYBERSLASH: keep pig inflicted gravity (and check for splat)
		if (ent->gravity < 0)
		{
			if (!(ent->client->ps.pmove.pm_flags & PMF_NO_PREDICTION))
				ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
			client->ps.pmove.gravity = ent->gravity;
			pig_checksplat(ent);
		}
		else
			client->ps.pmove.gravity = sv_gravity->value;

		/*ATTILA begin*/
		if ( Jet_Active(ent) )
			Jet_ApplyJet( ent, ucmd );
		/*ATTILA end*/

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
		client->ucmd = *ucmd;

		//if ( pm.cmd.upmove >= 10 )
		if (ent->gravity_lift > level.time && ent->gravity > 0)
		{
			pm.s.gravity = 50;
		}

		pm.trace = PM_trace;	// adds default parms
		pm.pointcontents = gi.pointcontents;

		if(ent->laz_vehicle)
			pm.s.pm_flags |= PMF_ON_GROUND;

		// Gravity hack..
		/*if (ent->is_bot && ent->gravity > 0)
		{
			if (ent->gravity_lift > level.time)
			{
				pm.s.gravity = 150;
			}
			else
				pm.s.gravity = 1300;
		}*/

		// perform a pmove
		gi.Pmove (&pm);
		if (ent->deadflag)
			VectorCopy (ent->tempvec, ent->s.angles);

		client->forwardmove = ucmd->forwardmove;

		if (ent->vehicle)
		{
			int s;
//			vec3_t move;
			ent->s.modelindex = 0;

			ent->movetype = MOVETYPE_NOCLIP;
			VectorCopy(ent->s.origin, ent->client->old_origin);
			gi.linkentity (ent);

			ent->vehicle->enemy = ent;
			ent->client->ps.gunindex = 0;

			if (ent->client->rc_left == 1)
			{
				s = 4;
				//ent->vehicle->ideal_yaw = ent->vehicle->s.angles[YAW];
				//ent->vehicle->ideal_yaw += s;
				ent->vehicle->s.angles[YAW] += s;
				//ai_turn(ent->vehicle, 0);

				pm.s.delta_angles[YAW] += ANGLE2SHORT(s);
			}
			else if (ent->client->rc_left == -1)
			{
				s = -4;
			//	ent->vehicle->ideal_yaw = ent->vehicle->s.angles[YAW];
			//	ent->vehicle->ideal_yaw += s;
			//	ai_turn(ent->vehicle, 0);
				ent->vehicle->s.angles[YAW] += s;

				pm.s.delta_angles[YAW] += ANGLE2SHORT(s);
			}

			if (ent->client->rc_forward == 1)
			{
				s = 5;

				//ai_move (ent->vehicle, 5);
				RCCarUp(ent->vehicle);
			}
			else if (ent->client->rc_forward == -1)
			{
				s = -5;
				RCCarDown(ent->vehicle);
			}
			else
				RCCarStop(ent->vehicle);

			client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
			client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
			client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
		}

		// save results of pmove
		client->ps.pmove = pm.s;
		client->old_pmove = pm.s;

		for (i=0 ; i<3 ; i++)
		{
			ent->s.origin[i] = pm.s.origin[i]*0.125;

			/*ATTILA begin*/
			if ( !Jet_Active(ent) || (Jet_Active(ent)&&(fabs((float)pm.s.velocity[i]*0.125) < fabs(ent->velocity[i]))) )
			/*ATTILA end*/

			ent->velocity[i] = pm.s.velocity[i]*0.125;
		}

		VectorCopy (pm.mins, ent->mins);
		VectorCopy (pm.maxs, ent->maxs);

		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
		
		/*ATTILA begin*/
		if ( Jet_Active(ent) )
			if( pm.groundentity )		/*are we on ground*/
				if ( Jet_AvoidGround(ent) )	/*then lift us if possible*/
					pm.groundentity = NULL;		/*now we are no longer on ground*/
		/*ATTILA end*/

		if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0) && !ent->laz_vehicle)
		{
			gi.sound(ent, CHAN_VOICE, SoundIndex("*jump1.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(ent, ent->s.origin, PNOISE_SELF);

			if (ent->client->boots_framenum > level.framenum)
				ent->velocity[2] += 550;
		}
		if (ent->deadflag)
			VectorCopy (ent->tempvec, ent->s.angles);

		ent->viewheight = pm.viewheight;
		ent->waterlevel = pm.waterlevel;
		ent->watertype = pm.watertype;
		ent->groundentity = pm.groundentity;
		if (pm.groundentity)
			ent->groundentity_linkcount = pm.groundentity->linkcount;

		// Lazarus - lie about ground when driving a vehicle.
		//           Pmove apparently doesn't think the ground
		//           can be "owned"
		if (ent->laz_vehicle && !ent->groundentity)
		{
			ent->groundentity = ent->laz_vehicle;
			ent->groundentity_linkcount = ent->laz_vehicle->linkcount;
		}

/*		if (ent->deadflag)
		{
			client->ps.viewangles[ROLL] = 40;
			client->ps.viewangles[PITCH] = -15;
			client->ps.viewangles[YAW] = client->killer_yaw;
		}
		else
		{*/
			VectorCopy (pm.viewangles, client->v_angle);
			VectorCopy (pm.viewangles, client->ps.viewangles);
		//}
		if (ent->deadflag)
			VectorCopy (ent->tempvec, ent->s.angles);

		//	{
		//	vec3_t forward, right, up;
			//AngleVectors (ent->client->v_angle, forward, right, up);

			//VectorMA (ent->velocity, ucmd->forwardmove, forward, ent->velocity);
			//VectorMA (ent->velocity, ucmd->sidemove, right, ent->velocity);
			//}
//ZOID
	if (client->ctf_grapple)
		CTFGrapplePull(client->ctf_grapple);
//ZOID

		gi.linkentity (ent);

		if (ent->movetype != MOVETYPE_NOCLIP)
			G_TouchTriggers (ent);
		if (ent->deadflag)
			VectorCopy (ent->tempvec, ent->s.angles);

		// touch other objects
		for (i=0 ; i<pm.numtouch ; i++)
		{
			other = pm.touchents[i];
			for (j=0 ; j<i ; j++)
				if (pm.touchents[j] == other)
					break;
			if (j != i)
			{
				//gi.dprintf ("Duplicated\n");
				continue;	// duplicated
			}
			if (!other->touch)
			{
				//gi.dprintf ("No touch\n");
				continue;	// duplicated
			}
			other->touch (other, ent, NULL, NULL);
		}

	}
		if (ent->deadflag)
			VectorCopy (ent->tempvec, ent->s.angles);

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if ((client->latched_buttons & BUTTON_ATTACK))
	{
		if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}
		if (ent->deadflag)
			VectorCopy (ent->tempvec, ent->s.angles);

//ZOID
//regen tech
	CTFApplyRegenerationTech(ent);
//ZOID
	// Regen ITEM
	CTFApplyRegeneration(ent);

//ZOID
	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}

	if (client->menudirty && client->menutime <= level.time) 
	{
		if (!ent->is_bot)
		{
			PMenu_Do_Update(ent);
			gi.unicast (ent, true);
			client->menutime = level.time;
			client->menudirty = false;
		}
	}
//ZOID

	UpdateRocketCam (ent);
	//pm.s.gravity = 0;

	if ((ucmd->buttons & BUTTON_USE))
	{
		client->use = 1;
	}
	else
	{
		client->use = 0;
	}

	//JABot[start]
//	AITools_DropNodes(ent);
	//JABot[end]

//	if (ent->ai.is_bot && !ent->takedamage)
//	{
//		gi.dprintf ("Whoa\n");
//	}

	// Onoz we are in teh solidity
	if (!ent->laz_vehicle && !ent->client->teleing && ent->movetype != MOVETYPE_NOCLIP && ent->think4 == NULL && gi.pointcontents(ent->s.origin) & CONTENTS_SOLID)
	{
		safe_cprintf (ent, PRINT_HIGH, "You look like you're stuck.\n");
		ent->think4 = AreWeStillInSolidcommaspaceIfYesRespawnUs;
		ent->nextthink4 = level.time + 2;
	}

	if (ent->vehicle)
	{
		VectorCopy (ent->client->rc_stay_here_origin, ent->s.origin);
		gi.linkentity (ent);
	}

	//if ((blaster_ban->string && Q_stricmp(blaster_ban->string, "all") == 0) || (shotgun_ban->string && Q_stricmp(shotgun_ban->string, "all") == 0)
	//{
		if (CheckBan(shotgun_ban, SHOTGUNBAN_CO2CANNON) && HasItem(ent, "CO2 Cannon"))
		{
			ent->client->pers.inventory[ITEM_INDEX(FindItem("CO2 Cannon"))]--;
		}
		if (CheckBan(blaster_ban, BLASTERBAN_GRAPPLE) && HasItem(ent, "Grapple"))
		{
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))]--;
		}
		if (CheckBan(blaster_ban, BLASTERBAN_SCREWDRIVER) && HasItem(ent, "Screwdriver Launcher"))
		{
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Screwdriver Launcher"))]--;
		}
		if (CheckBan(blaster_ban, BLASTERBAN_CHAINFIST) && HasItem(ent, "Chainfist"))
		{
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Chainfist"))]--;
		}
		if (CheckBan(blaster_ban, BLASTERBAN_SAWTOOTHEDHAMMER) && HasItem(ent, "Saw-Toothed Hammer"))
		{
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Saw-Toothed Hammer"))]--;
		}
		if (CheckBan(blaster_ban, BLASTERBAN_BATTERYOPERATEDDRILL) && HasItem(ent, "Battery-Operated Drill"))
		{
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Battery-Operated Drill"))]--;
		}
		if (CheckBan(blaster_ban, BLASTERBAN_RADIODETONATOR) && HasItem(ent, "Radio Detonator"))
		{
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Radio Detonator"))]--;
		}
		if (CheckBan(blaster_ban, BLASTERBAN_NEUTRONBLASTER) && HasItem(ent, "Neutron Blaster"))
		{
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Neutron Blaster"))]--;
		}
	//}

	if (ent->client->deflector_framenum > level.framenum)
	{
		ent->think2 = DeflectStuff;
		ent->nextthink2 = level.time + .1;
	}
	else
	{
		ent->nextthink2 = -1;
		ent->think2 = NULL;
	}
	
	if (iwm_editor->value)
	{
		int i;

		for (i = 0; i < MAX_ITEMS; i++)
		{
			if (ent->client->pers.inventory[i])
			ent->client->pers.inventory[i] = 0;
		}
	}

	if (ent->client->star_framenum > level.framenum)
		StarKillNearby(ent);
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
//	vec3_t up;

//	if (random() < 0.6)
//		return;
	if (IsLagged(ent) && random() < AmountToLag(ent))
		return;

	if (level.intermissiontime)
		return;

	// Is ent currently a 'Transformer' ?
	if (ent->vehicle && ent->is_transformed) 
	{
		//VectorCopy(ent->vehicle->s.origin, ent->s.origin);
		//VectorCopy(ent->vehicle->s.angles, ent->s.angles);
	//	AngleVectors(ent->s.angles, NULL, NULL, up);
		// Keep ent looking out Gladiator's eyes
	//	VectorMA(ent->s.origin, 25, up, ent->s.origin);
	//	ent->movetype=MOVETYPE_NOCLIP;
	//	ent->svflags=SVF_NOCLIENT;
		// No other movement..
	//	VectorClear (ent->client->ps.pmove.origin);
	//	VectorClear (ent->client->ps.viewoffset);

		client = ent->client;

		ent->client->ps.gunindex = 0;

		// run weapon animations if it hasn't been done by a ucmd_t
		if (!client->weapon_thunk)
			Think_Weapon (ent);
		else
			client->weapon_thunk = false;

		client->latched_buttons = 0;
		return; 
	}

	client = ent->client;

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk
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

			if (ent->is_bot)
			{
				respawn(ent);
				client->latched_buttons = 0;
			}

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


void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result, int left);
// constants
#define MIN_CHAIN_LEN		40		// minimum chain length
#define MAX_CHAIN_LEN		4000	// maximum chain length
#define CHAIN_LINK_LEN		55		// length between chain link origins
#define GROW_SHRINK_RATE	70		// units of lengthen/shrink chain in 0.1 sec
#define MAX_HOOKS			20		// max number of multihooks

// edict->hookstate constants
#define HOOK_ON		0x00000001		// set if hook command is active
#define HOOK_IN		0x00000002		// set if hook has attached
#define SHRINK_ON	0x00000004		// set if shrink chain is active 
#define GROW_ON		0x00000008		// set if grow chain is active



void DropHook (edict_t *ent)
{
	// remove all hook flags
	ent->owner->client->hookstate = 0;
	ent->owner->client->num_hooks = 0;

	gi.sound (ent->owner, CHAN_AUTO, SoundIndex("floater/Fltatck3.wav"), 1, ATTN_IDLE, 0);
	
	// removes hook
	G_FreeEdict (ent);
}


void MaintainLinks (edict_t *ent)
{
	vec3_t pred_hookpos;	// predicted future hook origin
	float multiplier;		// prediction multiplier
	vec3_t norm_hookvel;	// normalized hook velocity

	vec3_t	offset, start;
	vec3_t	forward, right;

// FIXME: add this and use it to make chain not clip in players view
//	vec3_t chainvec;		// vector of the chain 
//	vec3_t chainunit; 		// vector of chain with distance of 1
//	float chainlen;			// length of chain

	// predicts hook's future position since chain links fall behind
	VectorClear (norm_hookvel);
	multiplier = VectorLength(ent->velocity) / 22;
	VectorNormalize2 (ent->velocity, norm_hookvel); 
	VectorMA (ent->s.origin, multiplier, norm_hookvel, pred_hookpos);

	// derive start point of chain
	AngleVectors (ent->owner->client->v_angle, forward, right, NULL);
	VectorSet (offset, 8, 8, ent->owner->viewheight - 8.0f);
	P_ProjectSource (ent->owner->client, ent->owner->s.origin, offset, forward, right, start, 1);

// FIXME: add this and use it to make chain not clip in players view
	// get info about chain
//	_VectorSubtract (pred_hookpos,start,chainvec);
//	VectorNormalize2 (chainvec, chainunit);
//	VectorMA (chainvec, -18, chainunit, chainvec);
//	chainlen = VectorLength (chainvec);

	// create temp entity chain
	/*gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_PARASITE_ATTACK);
	gi.WriteShort (ent - g_edicts);
	gi.WritePosition (pred_hookpos);
	gi.WritePosition (start);
	gi.multicast (ent->s.origin, MULTICAST_PVS);*/
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (pred_hookpos);
	gi.WritePosition (start);
	gi.multicast (ent->s.origin, MULTICAST_PHS);
	/*gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_LIGHTNING);
	gi.WriteShort (ent - g_edicts);			// destination entity
	gi.WriteShort (ent - g_edicts);		// source entity
	gi.WritePosition (pred_hookpos);
	gi.WritePosition (start);
	gi.multicast (start, MULTICAST_PVS);*/
}


void HookBehavior(edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	vec3_t	chainvec;		// chain's vector
	float chainlen;			// length of extended chain
	vec3_t velpart;			// player's velocity component moving to or away from hook
	float f1, f2;			// restrainment forces
	float framestep;		// grow or shrink step per frame
		
	// decide when to disconnect hook
	if ( (!(ent->owner->client->hookstate & HOOK_ON)) ||// if hook has been retracted
	     (ent->enemy->solid == SOLID_NOT) ||			// if target is no longer solid (i.e. hook broke glass; exploded barrels, gibs) 
	     (ent->owner->deadflag) ||	// if player died 
		 (!ent->owner->inuse) ||	// or disconnected unexpectedly //QW//
	     (ent->owner->s.event == EV_PLAYER_TELEPORT) )	// if player goes through teleport
	{
		DropHook(ent);
		return;
	}

	// gives hook same velocity as the entity it is stuck in
	VectorCopy (ent->enemy->velocity,ent->velocity);

// chain sizing 

	// grow the length of the chain
	if ((ent->owner->client->hookstate & GROW_ON) && (ent->angle < MAX_CHAIN_LEN))
	{
		if (level.time - ent->wait > 0.1f) ent->wait = level.time - 0.1f;
		framestep = 10 * (level.time - ent->wait) * GROW_SHRINK_RATE;
		ent->angle += framestep;
		if (ent->angle > MAX_CHAIN_LEN) ent->angle = MAX_CHAIN_LEN;
		ent->wait = level.time;

		// trigger climb sound
		if (level.time - ent->delay >= 0.1f)
		{
			gi.sound (ent->owner, CHAN_AUTO, SoundIndex("doors/dr1_mid.wav"), 0.4, ATTN_IDLE, 0);
			ent->delay = level.time;
		}
	}

    if ((ent->owner->client->hookstate & SHRINK_ON) && (ent->angle > MIN_CHAIN_LEN))
	{
		if (level.time - ent->wait > 0.1f) ent->wait = level.time - 0.1f;
		framestep = 10 * (level.time - ent->wait) * GROW_SHRINK_RATE;
		ent->angle -= framestep;
		if (ent->angle < MIN_CHAIN_LEN) ent->angle = MIN_CHAIN_LEN;
		ent->wait = level.time;

		// trigger slide sound
		if (level.time - ent->delay >= 0.1f)		
		{
			gi.sound (ent->owner, CHAN_AUTO, SoundIndex("doors/dr1_mid.wav"), 0.4, ATTN_IDLE, 0);
			ent->delay = level.time;
		}
	}

// chain physics

	// derive start point of chain
	AngleVectors (ent->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->owner->viewheight - 8.0f);
	P_ProjectSource (ent->owner->client, ent->owner->s.origin, offset, forward, right, start, 1);

	// get info about chain
	_VectorSubtract (ent->s.origin, start, chainvec);
	chainlen = VectorLength (chainvec);

	// if player's location is beyond the chain's reach
	if (chainlen > ent->angle)	
	{	 
		// determine player's velocity component of chain vector
		VectorScale (chainvec, _DotProduct (ent->owner->velocity, chainvec) / _DotProduct (chainvec, chainvec), velpart);
		
		// restrainment default force 
		f2 = (chainlen - ent->angle) * 5;

		// if player's velocity heading is away from the hook
		if (_DotProduct (ent->owner->velocity, chainvec) < 0)
		{
			// if chain has streched for 25 units
			if (chainlen > ent->angle + 25)
				// remove player's velocity component moving away from hook
				_VectorSubtract(ent->owner->velocity, velpart, ent->owner->velocity);
			f1 = f2;
		}
		else  // if player's velocity heading is towards the hook
		{
			if (VectorLength (velpart) < f2)
				f1 = f2 - VectorLength (velpart);
			else		
				f1 = 0;
		}
	}
	else
		f1 = 0;
	
    // applys chain restrainment 
	VectorNormalize (chainvec);
	VectorMA (ent->owner->velocity, f1, chainvec, ent->owner->velocity);

	gi.linkentity(ent);
	
	MaintainLinks (ent);

	// prep for next think
	ent->nextthink = level.time + FRAMETIME;
}


void HookTouch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	vec3_t	chainvec;		// chain's vector

	// derive start point of chain
	AngleVectors (ent->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->owner->viewheight - 8.0f);
	P_ProjectSource (ent->owner->client, ent->owner->s.origin, offset, forward, right, start, 1);

	// member angle is used to store the length of the chain
	_VectorSubtract(ent->s.origin,start,chainvec);
	ent->angle = VectorLength (chainvec);	

	// don't attach hook to sky
	//if (surf && (surf->flags & SURF_SKY))
	//{
	//	DropHook (ent);
	//	return;
	//}

	// inflict damage on damageable items
	if (other->takedamage)
	{
		int mod;

		// Set up the means of death.
		mod = MOD_HOOK;

		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin,
			plane->normal, ent->dmg, 100, 0, mod);
	}

	if (other->solid == SOLID_BBOX)
	{
		if ((other->svflags & SVF_MONSTER) || (other->client))
			gi.sound (ent, CHAN_VOICE, SoundIndex("flyer/flyatck1.wav"), 1, ATTN_IDLE, 0);

		/*DropHook(ent);
		return;*/
		ent->other = other;
	}
	
	if (other->solid == SOLID_BSP)
	{
		// create puff of smoke
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SHOTGUN);
		gi.WritePosition (ent->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		gi.sound (ent, CHAN_VOICE, SoundIndex("flyer/Flyatck2.wav"), 1, ATTN_IDLE, 0);
		VectorClear (ent->avelocity);
	}
	else if (other->solid == SOLID_TRIGGER)
	{
		// debugging line; don't know if this will ever happen 
		safe_cprintf (ent->owner, PRINT_HIGH, "Hook touched a SOLID_TRIGGER\n");
	}
	
	// hook gets the same velocity as the item it attached to
	VectorCopy (other->velocity,ent->velocity);

	// flags hook as being attached to something
	ent->owner->client->hookstate |= HOOK_IN;

	ent->enemy = other;
	ent->touch = NULL;
	ent->think = HookBehavior;
	ent->nextthink = level.time + FRAMETIME;
}


void HookAirborne (edict_t *ent)
{
    vec3_t chainvec;		// chain's vector
	float chainlen;			// length of extended chain
	
	// get info about chain
	_VectorSubtract (ent->s.origin, ent->owner->s.origin, chainvec);
	chainlen = VectorLength (chainvec);
	
	if ( (!(ent->owner->client->hookstate & HOOK_ON)) || (chainlen > MAX_CHAIN_LEN) )
	{
		DropHook(ent);
		return;
	}
	
	MaintainLinks (ent);	

	ent->nextthink = level.time + FRAMETIME;
}


void FireHook (edict_t *ent)
{
	edict_t *newhook;
	vec3_t	offset, start;
	vec3_t	forward, right, end;
	int		damage;
	trace_t tr;

	//QW// limit the number of multi-hooks deployed
	if (ent->client->num_hooks >= MAX_HOOKS){
		safe_cprintf(ent,PRINT_HIGH, "Too many hooks activated\n");
		return;
	}

	// determine the damage the hook will inflict
	damage = 10;
	if (ent->client->quad_framenum > level.framenum)
		damage *= 4;
	
	// derive point of hook origin
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8, ent->viewheight - 8.0f);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 1);

	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (end);
	gi.multicast (end, MULTICAST_PVS);

	// spawn hook
	newhook = G_Spawn();
	VectorCopy (start, newhook->s.origin);
	VectorCopy (forward, newhook->movedir);
	vectoangles (forward, newhook->s.angles);
	VectorScale (forward, 2000, newhook->velocity);
	VectorSet(newhook->avelocity,0,0,-800);
	newhook->movetype = MOVETYPE_FLYMISSILE;
	newhook->clipmask = MASK_SHOT;
	newhook->solid = SOLID_BBOX;
	VectorClear (newhook->mins);
	VectorClear (newhook->maxs);
	newhook->can_teleport_through_destinations = 1;
	newhook->classname = "hook";
//	if (ctf->value)
//		newhook->s.modelindex = ModelIndex ("models/weapons/grapple/hook/tris.md2");
//	else
//		newhook->s.modelindex = ModelIndex ("models/weapons/grapple/hook/tris.md2");
		newhook->s.modelindex = ModelIndex ("models/objects/debris2/tris.md2");
	newhook->owner = ent;
	newhook->dmg = damage;

	// wait used to regulate climb and slide rates; tracks time between frames 
	newhook->wait = level.time;  

	// delay used to keep track of how frequent chain noise should occur 
	newhook->delay = level.time;   
    
	// play hook launching sound
	gi.sound (ent, CHAN_AUTO, SoundIndex ("parasite/Paratck2.wav"), 1, ATTN_IDLE, 0);
	
	// specify actions to follow 
	newhook->touch = HookTouch;
	newhook->think = HookAirborne;
	newhook->nextthink = level.time + FRAMETIME;

	//VectorCopy (tr.endpos, newhook->s.origin);
	gi.linkentity (newhook);
	//newhook->touch (newhook, &g_edicts[0], &tr.plane, tr.surface);
	ent->client->num_hooks++;

}


void Cmd_Hook_f (edict_t *ent)
{
	char *s;
	int	*hookstate;

	if (CheckBan(feature_ban, FEATUREBAN_HOOK))
	{
		safe_cprintf (ent, PRINT_HIGH, "Sorry, Hook is banned.\n");
		return;
	}

	// No grappling hook when you're dead.
	if (ent->deadflag) 
		return;

	// get the first hook argument
	s = gi.argv(1);

	// create intermediate value
	hookstate = &ent->client->hookstate;

	if ((!(*hookstate & HOOK_ON)) && (Q_stricmp(s, "action") == 0))
	{
		// flags hook as being active 
		*hookstate = HOOK_ON;   

		FireHook (ent);
		return;
	}

	if  (*hookstate & HOOK_ON)
	{
		// release hook	
		if (Q_stricmp(s, "action") == 0)
		{
			*hookstate = 0;
			return;
		}

// FIXME: put this in when I figure out where the jump key is handled
		// hop of chain and release hook when the following conditions apply
//		if (	(self.button2) && 					// jump is pressed
//				(self.flags & FL_JUMPRELEASED) &&	// previous jump cycle has finished
//				(self.hook & HOOK_IN) &&			// hook is attached
//				(!(self.flags & FL_ONGROUND)) &&	// player not on ground
//				(!(self.flags & FL_INWATER))	)	// player not in water
//		{
//			self.hook = self.hook - (self.hook & HOOK_ON);
//			self.velocity_z = self.velocity_z + 200;
//			sound (self, CHAN_BODY, "player/plyrjmp8.wav", 1, ATTN_NORM);
//			return;
//		}

		// deactivate chain growth or shrink
		if (Q_stricmp(s, "stop") == 0)
		{
			*hookstate -= *hookstate & (GROW_ON | SHRINK_ON);
			return;
		}

		// activate chain growth
		if (Q_stricmp(s, "grow") == 0)
		{
			*hookstate |= GROW_ON;
			*hookstate -= *hookstate & SHRINK_ON;
			return;
		}

		// activate chain shrinking
		if (Q_stricmp(s, "shrink") == 0)
		{
			*hookstate |= SHRINK_ON;		
			*hookstate -= *hookstate & GROW_ON;	
		}
	}
}
