#include "g_local.h"
#include "m_player.h"

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
// MATRIX

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;

	if (attacker->client && level.framenum < attacker->LastKill + 50)
	{	
		if(attacker != self)
		{
			attacker->KillStreak ++;
			attacker->LastKill = level.framenum;
		}
	}
	else
	{
		if(attacker->client)
		attacker->KillStreak = 0;
		attacker->LastKill = level.framenum;
	}

	if(attacker->KillStreak && attacker->client && killstreakmessage->value)
	{
		
	if (attacker->KillStreak == 1)
		{
			gi.centerprintf (attacker,"%s", streakmessage2->string);
			gi.sound(attacker, CHAN_VOICE, gi.soundindex("frags/killstreak2.wav"), 1, ATTN_NORM, 0);
		}
		if (attacker->KillStreak == 2)
		{
			gi.centerprintf (attacker,"%s", streakmessage3->string);
			gi.sound(attacker, CHAN_VOICE, gi.soundindex("frags/killstreak3.wav"), 1, ATTN_NORM, 0);
		}
		if (attacker->KillStreak == 3)
		{
			gi.centerprintf (attacker,"%s", streakmessage4->string);
			gi.sound(attacker, CHAN_VOICE, gi.soundindex("frags/killstreak4.wav"), 1, ATTN_NORM, 0);
		}
		if (attacker->KillStreak == 4)
		{
			gi.centerprintf (attacker, "%s", streakmessage5->string);
			gi.sound(attacker, CHAN_VOICE, gi.soundindex("frags/killstreak5.wav"), 1, ATTN_NORM, 0);
		}
		if (attacker->KillStreak > 4)
		{
			gi.centerprintf (attacker,"%s (%i kills)", streakmessage6->string, attacker->KillStreak);
			gi.sound(attacker, CHAN_VOICE, gi.soundindex("frags/killstreak6.wav"), 1, ATTN_NORM, 0);
		}
	
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
			if (deathmatch->value)
			{
				self->client->resp.score--;
				if ((!tankmode->value) && (teamplay->value))
				{
					if (self->client->resp.team == TEAM_RED)
						matrix.team_red_score--;
					if (self->client->resp.team == TEAM_BLUE)
						matrix.team_blue_score--;
				}
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
				message = " was blasted by";
				break;
			case MOD_BLASTER+LOC_LEFTARM:
				message = " had his left arm blasted off by";
				break;
			case MOD_BLASTER+LOC_RIGHTARM:
				message = " had his left arm blasted off by";
				break;
			case MOD_BLASTER+LOC_LEFTLEG:
				message = " was shot in the leg by";
				message2 = "'s blaster";
				break;
			case MOD_BLASTER+LOC_RIGHTLEG:
				message = " was taught to hop by";
				message2 = "'s blaster";
				break;
			case MOD_BLASTER+LOC_CHEST:
				message = " was blasted by";
				break;
			case MOD_BLASTER+LOC_BACK:
				message = " was blasted by";
				message2 = " from behind";
				break;
			case MOD_BLASTER+LOC_FACE:
				message = " was blasted in the face by";
				break;
			case MOD_BLASTER+LOC_HEAD:
				message = " took a blaster bolt to the head from";
				break;
			case MOD_SHOTGUN:
				message = " was gunned down by";
				break;
			case MOD_SHOTGUN+LOC_LEFTARM:
				message = " lost an arm to";
				break;
			case MOD_SHOTGUN+LOC_RIGHTARM:
				message = " lost an arm to";
				break;
			case MOD_SHOTGUN+LOC_LEFTLEG:
				message = " lost a leg to";
				break;
			case MOD_SHOTGUN+LOC_RIGHTLEG:
				message = " lost a leg to";
				break;
			case MOD_SHOTGUN+LOC_CHEST:
				message = " was gunned down by";
				break;
			case MOD_SHOTGUN+LOC_BACK:
				message = " was gunned down in the back by";
				break;
			case MOD_SHOTGUN+LOC_FACE:
				message = " was made really pretty by";
				message = "'s shotgun";
				break;
			case MOD_SHOTGUN+LOC_HEAD:
				message = " lost a chunk of his head by";
				break;
			case MOD_SSHOTGUN:
				message = " was blown away by";
				message2 = "'s super shotgun";
				break;
			case MOD_SSHOTGUN+LOC_LEFTARM:
				message = " was blown away by";
				message2 = "'s super shotgun";
				break;
			case MOD_SSHOTGUN+LOC_RIGHTARM:
				message = " was blown away by";
				message2 = "'s super shotgun";
				break;
			case MOD_SSHOTGUN+LOC_LEFTLEG:
				message = " was hit by ";
				message2 = "'s super shotgun pellets in the leg";
				break;
			case MOD_SSHOTGUN+LOC_RIGHTLEG:
				message = " was hit by ";
				message2 = "'s super shotgun pellets in the leg";
				break;
			case MOD_SSHOTGUN+LOC_BACK:
				message = " was blown away by";
				message2 = "'s super shotgun while trying to get away";
				break;
			case MOD_SSHOTGUN+LOC_CHEST:
				message = " shows his intestines to the world because of";
				message2 = "'s super shotgun";
				break;
			case MOD_SSHOTGUN+LOC_FACE:
				message = " now has a hole in his head because of";
				message2 = "'s super shotgun";
				break;
			case MOD_SSHOTGUN+LOC_HEAD:
				message = " now has a hole in his head because of";
				message2 = "'s super shotgun";
				break;
			case MOD_MACHINEGUN+LOC_LEFTARM:
				message = " was shot in the arm by";
				break;
			case MOD_MACHINEGUN+LOC_RIGHTARM:
				message = " was shot in the arm by";
				break;
			case MOD_MACHINEGUN+LOC_LEFTLEG:
				message = " was shot in leg the by";
				break;
			case MOD_MACHINEGUN+LOC_RIGHTLEG:
				message = " was shot in leg the by";
				break;
			case MOD_MACHINEGUN+LOC_BACK:
				message = " was shot by";
				message2 = "in the back";
				break;
			case MOD_MACHINEGUN+LOC_CHEST:
				message = " was shot by";
				break;
			case MOD_MACHINEGUN+LOC_FACE:
				message = " saw";
				message2 = "'s bullet really close";
				break;
			case MOD_MACHINEGUN+LOC_HEAD:
				message = " took a bullet to the brain";
				break;
			case MOD_MACHINEGUN:
				message = " was shot by";
				break;
			case MOD_CHAINGUN+LOC_LEFTARM:
				message = " arm was hit by a bullet from";
				message2 = "'s chaingun";
				break;
			case MOD_CHAINGUN+LOC_RIGHTARM:
				message = " arm was hit by a bullet from";
				message2 = "'s chaingun";
				break;
			case MOD_CHAINGUN+LOC_LEFTLEG:
				message = " lost his legs to";
				message2 = "'s chaingun";
				break;
			case MOD_CHAINGUN+LOC_RIGHTLEG:
				message = " lost his legs to";
				message2 = "'s chaingun";
				break;
			case MOD_CHAINGUN+LOC_BACK:
				message = "'s back was cut in half by";
				message2 = "'s chaingun";
				break;
			case MOD_CHAINGUN+LOC_CHEST:
				message = " chest was cut in half by";
				message2 = "'s chaingun";
				break;
			case MOD_CHAINGUN+LOC_FACE:
				message = "'s face was cut in half by";
				message2 = "'s chaingun";
				break;
			case MOD_CHAINGUN+LOC_HEAD:
				message = " head was cut in half by";
				message2 = "'s chaingun";
				break;
			case MOD_CHAINGUN:
				message = " was cut in half by";
				message2 = "'s chaingun";
				break;
		
			case MOD_GRENADE+LOC_LEFTARM:
				message = "'s arm was popped by";
				message2 = "'s grenade";
				break;
			case MOD_GRENADE+LOC_RIGHTARM:
				message = "'s arms were popped by";
				message2 = "'s grenade";
				break;
			case MOD_GRENADE+LOC_LEFTLEG:
				message = " 's legs were popped by";
				message2 = "'s grenade";
				break;
			case MOD_GRENADE+LOC_RIGHTLEG:
				message = " 's legs were popped by";
				message2 = "'s grenade";
				break;
			case MOD_GRENADE+LOC_BACK:
				message = " 's back was popped by";
				message2 = "'s grenade";
				break;
			case MOD_GRENADE+LOC_CHEST:
				message = " 's chest was blown apart by";
				message2 = "'s grenade";
				break;
			case MOD_GRENADE+LOC_FACE:
				message = " 's legs were popped off by";
				message2 = "'s grenade";
				break;
			case MOD_GRENADE+LOC_HEAD:
				message = " 's head was popped off by";
				message2 = "'s grenade";
				break;
			case MOD_GRENADE:
				message = " was popped by";
				message2 = "'s grenade";
				break;
		
			case MOD_G_SPLASH:
				message = " was shredded by";
				message2 = "'s shrapnel";
				break;
		
			case MOD_ROCKET:
				message = " ate";
				message2 = "'s rocket";
				break;
			case MOD_ROCKET+LOC_LEFTARM:
				message = "'s arms where blown off";
				message2 = "'s rocket";
				break;
			case MOD_ROCKET+LOC_RIGHTARM:
				message = "'s arms where blown off";
				message2 = "'s rocket";
				break;
			case MOD_ROCKET+LOC_LEFTLEG:
				message = "'s legs where blown off";
				message2 = "'s rocket";
				break;
			case MOD_ROCKET+LOC_RIGHTLEG:
				message = "'s legs where blown off";
				message2 = "'s rocket";
				break;
			case MOD_ROCKET+LOC_BACK:
				message = " ate";
				message2 = "'s rocket";
				break;
			case MOD_ROCKET+LOC_CHEST:
				message = " ate";
				message2 = "'s rocket";
				break;
			case MOD_ROCKET+LOC_FACE:
				message = " ate";
				message2 = "'s rocket in the face!";
				break;
			case MOD_ROCKET+LOC_HEAD:
				message = " was decapitated by";
				message2 = "'s rocket";
				break;
			
			case MOD_R_SPLASH:
				message = " almost dodged";
				message2 = "'s rocket";
				break;
				//is now for machineguns
			case MOD_HYPERBLASTER:
				message = " was shot up by ";
				message2 = "";
				break;
			case MOD_HYPERBLASTER+LOC_LEFTARM:
				message = " was machinegunned in the left arm by ";
				message2 = "";
				break;
			case MOD_HYPERBLASTER+LOC_RIGHTARM:
				message = " was machinegunned in the right arm by ";
				message2 = "";
				break;
			case MOD_HYPERBLASTER+LOC_LEFTLEG:
				message = " leg was ripped apart by ";
				message2 = "'s machinegun fire";
				break;
			case MOD_HYPERBLASTER+LOC_RIGHTLEG:
				message = " right leg was ripped apart by ";
				message2 = "'s machinegun fire";
				break;
			case MOD_HYPERBLASTER+LOC_BACK:
				message = " was mowed down from behind by ";
				message2 = "";
				break;
			case MOD_HYPERBLASTER+LOC_CHEST:
				message = " feels hot lead in the chest, courtesy of ";
				message2 = "";
				break;
			case MOD_HYPERBLASTER+LOC_FACE:
				message = " was machinegunned in the face ";
				message2 = "";
				break;
			case MOD_HYPERBLASTER+LOC_HEAD:
				message = " head exploded by machinegun fire from ";
				message2 = "";
				break;
				

			case MOD_RAILGUN+LOC_LEFTARM:
				message = " was sniped in the arm by";
				break;
			case MOD_RAILGUN+LOC_RIGHTARM:
				message = " was sniped in the arm by";
				break;
			case MOD_RAILGUN+LOC_LEFTLEG:
				message = " was sniped in leg the by";
				break;
			case MOD_RAILGUN+LOC_RIGHTLEG:
				message = " was sniped in leg the by";
				break;
			case MOD_RAILGUN+LOC_BACK:
				message = " was sniped by";
				message2 = " in the back";
				break;
			case MOD_RAILGUN+LOC_CHEST:
				message = " was sniped by";
				message2 = " in the chest";
				break;
			case MOD_RAILGUN+LOC_FACE:
				message = " took a closer look at ";
				message2 = "'s bullet";
				break;
			case MOD_RAILGUN+LOC_HEAD:
				message = " took a slug to the brain from";
				break;
			case MOD_RAILGUN:
				message = " was sniped by";
				break;
			case MOD_BFG_LASER:
				message = " saw the pretty lights from";
				message2 = "'s BFG";
				break;
			case MOD_BFG_BLAST:
				message = " was disintegrated by";
				message2 = "'s BFG blast";
				break;
			case MOD_BFG_EFFECT:
				message = " couldn't hide from";
				message2 = "'s BFG";
				break;
				
			case MOD_HANDGRENADE:
				message = " caught";
				message2 = "'s handgrenade";
				break;
			case MOD_HANDGRENADE+LOC_LEFTARM:
				message = " caught";
				message2 = "'s hand grenade in the arm";
				break;
			case MOD_HANDGRENADE+LOC_RIGHTARM:
				message = " caught";
				message2 = "'s hand grenade in the arm";
				break;
			case MOD_HANDGRENADE+LOC_LEFTLEG:
				message = " caught";
				message2 = "'s hand grenade in the leg";
				break;
			case MOD_HANDGRENADE+LOC_RIGHTLEG:
				message = " caught";
				message2 = "'s hand grenade in the leg";
				break;
			case MOD_HANDGRENADE+LOC_BACK:
				message = " caught";
				message2 = "'s hand grenade in the back";
				break;
			case MOD_HANDGRENADE+LOC_CHEST:
				message = " caught";
				message2 = "'s hand grenade in the chest";
				break;
			case MOD_HANDGRENADE+LOC_FACE:
				message = " caught";
				message2 = "'s hand grenade in the head";
				break;
			case MOD_HANDGRENADE+LOC_HEAD:
				message = " caught";
				message2 = "'s hand grenade in the head";
				break;
				
			case MOD_HG_SPLASH:
				message = " didn't see";
				message2 = "'s handgrenade";
				break;
			case MOD_HELD_GRENADE:
				message = " feels";
				message2 = "'s pain";
				break;
			case MOD_TELEFRAG:
				message = " tried to invade";
				message2 = "'s personal space";
				break;
			case MOD_KICK:
				message = " slaps";
				message2 = " to death";
				break;
			case MOD_KICK+LOC_LEFTARM:
				message = " gave";
				message2 = " a dead left arm";
				break;
			case MOD_KICK+LOC_RIGHTARM:
				message = " gave";
				message2 = " a dead right arm";
				break;
			case MOD_KICK+LOC_LEFTLEG:
				message = " breaks";
				message2 = "'s left leg";
				break;
			case MOD_KICK+LOC_RIGHTLEG:
				message = " gets";
				message2 = " higher up on the list of right hip replacements";
				break;
			case MOD_KICK+LOC_CHEST:
				message = " winds";
				message2 = " with a swift punch to the solar plexis";
				break;
			case MOD_KICK+LOC_BACK:
				message = " rips out";
				message2 = "'s spine.";
				break;
			case MOD_KICK+LOC_FACE:
				message = " pushes jawbone into";
				message2 = "'s brain";
				break;
			case MOD_KICK+LOC_HEAD:
				message = " sneaks up and snaps";
				message2 = "'s neck. Get out of the console, dumbass!";
				break;
			case MOD_SWEEP:
				message = " trips";
				message2 = " over";
				break;
			case MOD_SWEEP+LOC_LEFTARM:
				message = " gave";
				message2 = " a spin kick to the left arm";
				break;
			case MOD_SWEEP+LOC_RIGHTARM:
				message = " gave";
				message2 = " a spin kick to the right arm";
				break;
			case MOD_SWEEP+LOC_LEFTLEG:
				message = " sends";
				message2 = "'s left leg over head";
				break;
			case MOD_SWEEP+LOC_RIGHTLEG:
				message = " gets";
				message2 = "'s right leg over head";
				break;
			case MOD_SWEEP+LOC_CHEST:
				message = " spin kicks";
				message2 = "'s torso away";
				break;
			case MOD_SWEEP+LOC_BACK:
				message = " sneaks up on";
				message2 = " and sweeps the fucker";
				break;
			case MOD_SWEEP+LOC_FACE:
				message = "'s wild break-dancing results in";
				message2 = "'s demise (pin kick to the head)";
				break;
			case MOD_SWEEP+LOC_HEAD:
				message = " makes";
				message2 = " fall badly. Very Badly.";
				break;
			case MOD_HOVER:
				message = "'s 15th flying kick downs";
				message2 = ".";
				break;
			case MOD_HOVER+LOC_LEFTARM:
				message = "'s flying roundhouse loses";
				message2 = "'s left arm";
				break;
			case MOD_HOVER+LOC_RIGHTARM:
				message = "'s flying roundhouse loses";
				message2 = "'s right arm";
				break;
			case MOD_HOVER+LOC_LEFTLEG:
				message = "'s flurry results in crutches for";
				message2 = " and his poor left leg";
				break;
			case MOD_HOVER+LOC_RIGHTLEG:
				message = "'s flurry results in crutches for";
				message2 = " and his poor left leg";
				break;
			case MOD_HOVER+LOC_CHEST:
				message = " puts 87 well placed fly kicks in";
				message2 = "'s chest";
				break;
			case MOD_HOVER+LOC_BACK:
				message = " fells";
				message2 = " with 54 stomps in the back";
				break;
			case MOD_HOVER+LOC_FACE:
				message = " gives";
				message2 = " a face massage.. with his feet of fury";
				break;
			case MOD_HOVER+LOC_HEAD:
				message = " gets ";
				message2 = "'s brain on his boots";
				break;
			// hook
			case MOD_HOOK:
				message = " punches";
				message2 = "'s lights out";
				break;
			case MOD_HOOK+LOC_LEFTARM:
				message = "'s hook breaks";
				message2 = "'s left arm";
				break;
			case MOD_HOOK+LOC_RIGHTARM:
				message = "'s hook breaks";
				message2 = "'s right arm";
				break;
			case MOD_HOOK+LOC_LEFTLEG:
				message = "'s hook busts up";
				message2 = "'s left leg";
				break;
			case MOD_HOOK+LOC_RIGHTLEG:
				message = "'s hook knackers";
				message2 = "'s right leg";
				break;
			case MOD_HOOK+LOC_CHEST:
				message = " thrusts his fist through";
				message2 = "'s chest";
				break;
			case MOD_HOOK+LOC_BACK:
				message = " takes out";
				message2 = "'s spinal column";
				break;
			case MOD_HOOK+LOC_FACE:
				message = " destroys";
				message2 = "'s beautiful complextion";
				break;
			case MOD_HOOK+LOC_HEAD:
				message = " punches";
				message2 = " upside de heed";
				break;
				// JAB
			case MOD_JAB:
				message = "'s jab takes out";
				message2 = "";
				break;
			case MOD_JAB+LOC_LEFTARM:
				message = " jabs ";
				message2 = "'s left arm";
				break;
			case MOD_JAB+LOC_RIGHTARM:
				message = " jabs ";
				message2 = "'s right arm";
				break;
			case MOD_JAB+LOC_LEFTLEG:
				message = "'s flurry of jabs results in wheelchairisation for";
				message2 = " and his poor left leg";
				break;
			case MOD_JAB+LOC_RIGHTLEG:
				message = "'s flurry of jabs results in wheelchairisation for";
				message2 = " and his poor right leg";
				break;
			case MOD_JAB+LOC_CHEST:
				message = " pummels";
				message2 = "'s chest";
				break;
			case MOD_JAB+LOC_BACK:
				message = " massages";
				message2 = "'s back with with 54 jabs";
				break;
			case MOD_JAB+LOC_FACE:
				message = " gives";
				message2 = " a face massage.. with his fist of fury";
				break;
			case MOD_JAB+LOC_HEAD:
				message = " knocks ";
				message2 = "'s block off";
				break;

				// UPPERCUT
			case MOD_UPPERCUT:
				message = " puts some sho ryu ken on";
				message2 = "";
				break;
			case MOD_UPPERCUT+LOC_LEFTARM:
				message = " rips off";
				message2 = "'s left arm with a dragon punch";
				break;
			case MOD_UPPERCUT+LOC_RIGHTARM:
				message = " rips off";
				message2 = "'s left arm with a dragon punch";
				break;
			case MOD_UPPERCUT+LOC_LEFTLEG:
				message = " upper cuts";
				message2 = " in the left leg";
				break;
			case MOD_UPPERCUT+LOC_RIGHTLEG:
				message = " upper cuts";
				message2 = " in the right leg";
				break;
			case MOD_UPPERCUT+LOC_CHEST:
				message = " puts some ha doo ken in";
				message2 = "'s chest";
				break;
			case MOD_UPPERCUT+LOC_BACK:
				message = " punches";
				message2 = " up the bracket";
				break;
			case MOD_UPPERCUT+LOC_FACE:
				message = " doesn't like";
				message2 = "'s face";
				break;
			case MOD_UPPERCUT+LOC_HEAD:
				message = " puts his fist through";
				message2 = "'s brain";
				break;

					// KNIFE
			case MOD_KNIFE:
				message = " rips";
				message2 = " open";
				break;
			case MOD_KNIFE+LOC_LEFTARM:
				message = " cuts up ";
				message2 = "'s left arm";
				break;
			case MOD_KNIFE+LOC_RIGHTARM:
				message = " cuts up ";
				message2 = "'s right arm";
				break;
			case MOD_KNIFE+LOC_LEFTLEG:
				message = " stabs ";
				message2 = " in the left leg";
				break;
			case MOD_KNIFE+LOC_RIGHTLEG:
				message = " stabs ";
				message2 = " in the right leg";
				break;
			case MOD_KNIFE+LOC_CHEST:
				message = " makes a sheath out of";
				message2 = "'s heart";
				break;
			case MOD_KNIFE+LOC_BACK:
				message = " backstabs ";
				message2 = "";
				break;
			case MOD_KNIFE+LOC_FACE:
				message = " gives";
				message2 = " a newcastle smiley";
				break;
			case MOD_KNIFE+LOC_HEAD:
				message = " tries brain surgury on";
				message2 = " and fails";
				break;
				
			}
			if (message)
			{
				if(mod >= MOD_KICK)
				gi.bprintf (PRINT_MEDIUM,"%s%s %s%s\n", attacker->client->pers.netname, message,  self->client->pers.netname,message2);
				else
				gi.bprintf (PRINT_MEDIUM,"%s%s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);

				if (deathmatch->value)
				{
					if (ff)
					{
						if ((!tankmode->value) && (teamplay->value))
						{
							if (attacker->client->resp.team == TEAM_RED)
								matrix.team_red_score--;
							if (attacker->client->resp.team == TEAM_BLUE)
								matrix.team_blue_score--;
						}
						attacker->client->resp.score--;
					}
					else
					{
						if ((!tankmode->value) && (teamplay->value))
						{
							if (attacker->client->resp.team == TEAM_RED)
								matrix.team_red_score++;
							if (attacker->client->resp.team == TEAM_BLUE)
								matrix.team_blue_score++;
						}
						attacker->client->resp.score++;
					}
				}
				return;
			}
		}
	}

	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (deathmatch->value)
	{
		if ((!tankmode->value) && (teamplay->value))
		{
			if (self->client->resp.team == TEAM_RED)
				matrix.team_red_score--;
			if (self->client->resp.team == TEAM_BLUE)
				matrix.team_blue_score--;
		}
		self->client->resp.score--;
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
	if (item && (strcmp (item->pickup_name, "Fists of fury") == 0))
		item = NULL;
	if (item && (strcmp (item->pickup_name, "gung ho knives") == 0))
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
		float angle = rand() % 360;
		self->client->v_angle[YAW] -= angle;
		drop = Drop_Item (self, item);
		self->client->v_angle[YAW] += angle;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
	}
//	if ((item)&&(self->client->akimbo))
//	{
//		float angle = rand() % 360;
//		self->client->v_angle[YAW] += angle;
//		drop = Drop_Item (self, item);
//		self->client->v_angle[YAW] -= angle;
//		drop->spawnflags = DROPPED_PLAYER_ITEM;
//	}

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
		int sound = rand()%4;
        //SKULL
        if (self->client->chasetoggle)
        {
                ChasecamRemove (self, "off");
                self->client->pers.chasetoggle = 1; //So we can turn it back on
        }
        else
                self->client->pers.chasetoggle = 0; //So we can turn it back on
        //END

	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
	self->s.modelindex3 = 0;
	self->s.modelindex4 = 0;

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
		TossClientWeapon (self);
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
	self->client->ir_framenum = 0;
	self->client->cloak_framenum = 0;
	self->bullet_framenum = 0;
	self->client->speed_framenum = 0;
	self->pinned_nextthink = 0;
	self->dodge_framenum = 0;

	self->flags &= ~FL_POWER_ARMOR;

	if (self->health < -40)
	{	// gib
	/*	gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		ThrowGib (self, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC);
			ThrowGib (self, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC);
			ThrowGib (self, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC);
			ThrowGib (self, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC);
			ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);
			//matrix PROPER gibs	
		ThrowClientHead (self, damage);

		self->takedamage = DAMAGE_NO;
	*/
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

	item = FindItem("bullets");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 50;

	item = FindItem("Fists Of Fury");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 2;

	item = FindItem("Gung Ho Knives");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 10;

	

	item = FindItem("mk 23 pistol");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 2;

	client->pers.weapon = item;

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 50;

	client->pers.connected = true;
//SKULL
        //Default chasecam to on
        client->pers.chasetoggle = 0;
//END
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
                //SKULL
                game.clients[i].pers.chasetoggle = ent->client->chasetoggle;
                //END
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
	
		//	ThrowGib (self, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC);
			
		//	ThrowGib (self, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC);

		//	ThrowGib (self, "models/objects/gibs/arm/tris.md2", damage, GIB_ORGANIC);
		//	ThrowGib (self, "models/objects/gibs/leg/tris.md2", damage, GIB_ORGANIC);
		//	ThrowGib (self, "models/objects/gibs/chest/tris.md2", damage, GIB_ORGANIC);

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
        //SKULL
        if (self->client->oldplayer)
                G_FreeEdict (self->client->oldplayer);
        if (self->client->chasecam)
                G_FreeEdict (self->client->chasecam);
        //END
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
                //SKULL
                if (ent->client->chasetoggle)
                {
                        ChasecamRemove (ent, "off");
                        ent->client->pers.chasetoggle = 1;
                }
                else
                        ent->client->pers.chasetoggle = 0;
                //END
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
        //SKULL
        int     chasetoggle;
        char            userinfo[MAX_INFO_STRING];
        //END
	client_persistant_t	saved;
	client_respawn_t	resp;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);
	
	ent->secondaryfire = false;
	ent->dodgethis = false;
//    ent->kyleisstupid = false;

	index = ent-g_edicts-1;
	client = ent->client;
        //SKULL
        chasetoggle = client->pers.chasetoggle;
        //END

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);
	}
	else if (coop->value)
	{
//		int			n;

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
//SKULL
//A bug in Q2 that you couldn't see without thirdpp
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		ClientUserinfoChanged (ent, userinfo);
//END
	}

	// clear everything but the persistant data
	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
	client->resp = resp;
        //SKULL
        client->pers.chasetoggle = chasetoggle;
        //END

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
        //SKULL
        ent->svflags &= ~SVF_NOCLIENT;
        //Turn on prediction
        ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
        //END

	//leper
//	ent->leftarm = 20;
//	ent->rightarm = 20;
//	ent->leftleg = 30;
//	ent->rightleg = 30;
//	ent->head = 60;

	//leper

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
	if(ent->client->pers.weapon->akimbo_model)
	ent->client->akimbo = true;
	if(ent->client->akimbo)
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->akimbo_model);
	else
	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	ent->s.effects = 0;
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->s.modelindex2 = 255;		// custom gun model
//	if(leper->value)
//	{
//	ent->s.modelindex3 = 255;		//arms leper
//	ent->s.modelindex4 = 255;		//legs leper
//	}

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

        //SKULL
        ent->client->chasetoggle = 0;
        //If chasetoggle set then turn on
        if (ent->client->pers.chasetoggle)
               ChasecamStart (ent);
        //END                
	MatrixClientInit(ent);
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

	MatrixBeginDM (ent);
	
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
	else
		matrix.lightsout_framenum = level.framenum;
	
stuffcmd(ent, "alias +button0 onbutton0\nalias -button0 offbutton0\nalias +button1 onbutton1\nalias -button1 offbutton1\nalias +boot booton\nalias -boot bootoff\nalias +attack2 booton\nalias -attack2 bootoff");
stuffcmd(ent, "alias +button0 onbutton0\nalias -button0 offbutton0\nalias +button1 onbutton1\nalias -button1 offbutton1\nalias +dodge dodgeon\nalias -dodge dodgeoff\nalias +attack2 dodgeon\nalias -attack2 dodgeoff");	
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
	if ((deathmatch->value && *s && strcmp(s, "0"))		||
	// MATRIX
		(!ent->client->resp.team && teamplay->value)	||
		(ent->client->resp.tank && tankmode->value)/*		||
		(!ent->client->resp.choosen && choosestuff->value)*/)
	// MATRIX
		ent->client->pers.spectator = true;
	else
		ent->client->pers.spectator = false;

	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;

	// combine name and skin into a configstring
	if (teamplay->value)
		AssignSkin(ent, s);
	else
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
	if (SV_FilterPacket(value)) {
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}
//matrix. every fucking one wants to be neo. this gets kinda confusing
	if (faglimit->value)
	{
		if (strcmp(ent->client->pers.netname, "neo") == 0 || strcmp(ent->client->pers.netname, "morpheus") == 0 || strcmp(ent->client->pers.netname, "trinity") == 0)
			{
			Info_SetValueForKey(userinfo, "rejmsg", "Please don't use names from the film.\n This gets confusing if everyone does it.");
			return false;
			}

		
		gi.bprintf (PRINT_HIGH, "Faglimit hit. Another %s tried to join the game.\n", ent->client->pers.netname);
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

        //SKULL
        if (ent->client->chasetoggle)
                ChasecamRemove (ent, "off");
        //END
	MatrixDoAtDeath(ent);
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
	// MATRIX
	ent->forwardbuttons = ucmd->forwardmove;
	ent->strafebuttons = ucmd->sidemove;
	ent->vertbuttons = ucmd->upmove;
// MATRIX
	level.current_entity = ent;
	client = ent->client;

	//leper
//Leper_Think(ent);
	//leper

	if (level.intermissiontime)
	{
                //SKULL
                if (client->chasetoggle)
                        ChasecamRemove (ent, "off");
                //END
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}

//SKULL
        if (client->chasetoggle)
                ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
        else
                ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

        //+use now does the cool look around stuff but only in SP games
        if ((ucmd->buttons & BUTTON_USE) && (!deathmatch->value))
        {
                client->use = 1;
                if ((ucmd->forwardmove < 0) && (client->zoom < 60))
                        client->zoom++;
                else if ((ucmd->forwardmove > 0) && (client->zoom > -40))
                        client->zoom--;
                ucmd->forwardmove = 0;
                ucmd->sidemove = 0;
        }
        else if (client->use)
        {
                //client->zoom = 0;
                if (client->oldplayer)
                {
                // set angles
                for (i=0 ; i<3 ; i++)
                {
                        ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->oldplayer->s.angles[i] - ent->client->resp.cmd_angles[i]);
                }
                }
                client->use = 0;
        }
//END

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

// MATRIX to counter the double move
		if (ent->client->speed_framenum > level.framenum)
			client->ps.pmove.gravity = sv_gravity->value/2;
		else// MATRIX
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
// MATRIX
//	DOUBLE MOVE
		if (ent->client->speed_framenum > level.framenum)
			gi.Pmove (&pm);
// MATRIX
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
			// MATRIX
			if (client->resp.tank)
			{
				vec3_t dir;
				AngleVectors (client->v_angle, dir, NULL, NULL);
				kick_attack (ent, ent->s.origin, dir , 1, 100, MOD_HYPERBLASTER);	
			}
			// MATRIX
			if ((client->chase_target) || (client->resp.tank)) {
				client->chase_target = NULL;
				client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			} else
				GetChaseTarget(ent);

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
	MatrixFlip (ent);

	client = ent->client;

	if (deathmatch->value &&
		client->pers.spectator != client->resp.spectator &&
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

			if ( ( client->latched_buttons & buttonMask ) ||
				(deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN) ) )
			{
				if (tankmode->value)
				{
					ent->client->resp.tank = true;
					spectator_respawn(ent);
				}
				else
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
