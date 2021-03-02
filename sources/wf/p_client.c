/* p_client.c */

#include "g_local.h"
#include "m_player.h"
#include "stdlog.h"	//	StdLog - Mark Davies
#include "wf_classmgr.h"//acrid for WFCopyToBodyQue

FILE *zbotfile = NULL;

int deas = 10;

//ERASER START
///Q2 Camera Begin
#include "camclient.h"
///Q2 Camera End
#include "p_trail.h"
#include "bot_procs.h"
#include <sys/timeb.h>
#include <time.h>
#define	OPTIMIZE_INTERVAL	0.1
int	num_clients	= 0;
//ERASER END

void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void SP_misc_teleporter_dest (edict_t *ent);
void WFRemoveDisguise(edict_t *ent);
void Drop_General (edict_t *ent, gitem_t *item);
void HealPlayer(edict_t *ent);
void I_AM_A_ZBOT(edict_t *ent);

qboolean ZbotCheck(edict_t *ent, usercmd_t *ucmd);

//Utility function to fix player name so it doesnt have a '%' in it
void wfFixName(char *text)
{
	int j;
	j = 0;
	while (j < 150 && text[j] != 0)
	{
		if (text[j] == '%') text[j] = '_';		// don't allow formated characters in text
		++j;
	}

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

//ERASER START
edict_t *dm_spots[64];
int		num_dm_spots;
//ERASER END

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

	dm_spots[num_dm_spots++] = self;//ERASER
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


/*QUAKED info_position (1 0 1) (-16 -16 -24) (16 16 32)
Used to describe the location within a map
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_position(edict_t *ent)
{
/*
	gi.dprintf("DEBUG (Spawned info_position) Classname = %s\n", ent->classname);
	if 	(ent->message && ent->message[0])
		gi.dprintf("DEBUG (info_position) Message = %s\n", ent->message);
*/

}

/*QUAKED item_flagreturn (1 0 1) (-16 -16 -24) (16 16 32)
Used to describe the location of the flag return base
*/
void SP_item_flagreturn(edict_t *ent)
{

//	gi.dprintf("--Spawned %s.  Target = %s, ent = %d\n", ent->classname, ent->target, ent);

}

//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{

	if (!ent->client)
		return false;

	if (ent->client->player_model == CLASS_MODEL_FEMALE)
		return true;
	else
		return false;
}

//WF
qboolean IsCyborg (edict_t *ent)
{


	if (!ent->client)
		return false;

	if (ent->client->player_model == CLASS_MODEL_CYBORG)
		return true;
	else
		return false;
}
//WF

//Not female, male or cyborg?
qboolean IsNeutral (edict_t *ent)
{

	if (!ent->client)
		return false;

	if ((ent->client->player_model == CLASS_MODEL_FEMALE) ||
	    (ent->client->player_model == CLASS_MODEL_MALE) ||
	    (ent->client->player_model == CLASS_MODEL_CYBORG))
		return false;
	else
		return true;
}

int PlayerChangeScore(edict_t *self, int points)
{
	if (wf_game.game_halted)
	{
		safe_cprintf(self, PRINT_HIGH, "No Scoring Allowed While Game Is Suspended\n");
		return false;
	}
	else
		self->client->resp.score += points;
	return true;
}


void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *atk)
{
	int			mod;
	char		*message;
	char		*message2;
	qboolean	ff;
	edict_t *attacker;

	qboolean special_message;

	special_message = false;

	attacker = atk;

	//If this is a death by sentry gun, then the attacker's owner
	//is the real attacker
	if ((atk->creator) &&(!atk->client) && (meansOfDeath == MOD_SENTRY || meansOfDeath == MOD_SENTRY_ROCKET || meansOfDeath == MOD_SHRAPNEL || meansOfDeath == MOD_PELLET) )
		attacker = atk->creator;

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

//ERASER START //ACRID FIXME? teamplay
	else if (deathmatch->value && teamplay->value && attacker->client && SameTeam(self, attacker))
	{
		meansOfDeath |= MOD_FRIENDLY_FIRE;
	}
//ERASER END

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
		case MOD_FEIGN:
			message = "died while trying to feign";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "blew up";
			break;
		case MOD_EXIT:
			message = "found a way out";
			break;
//TeT - Laser is not suicide
//		case MOD_TARGET_LASER:
//			message = "saw the light";
//			break;
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
			case MOD_FEIGN:
				message = "died while trying to feign";
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
			my_bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
			{
				if (PlayerChangeScore(self,CTF_SUICIDE_POINTS))
					sl_LogScore( &gi, self->client->pers.netname, "", "Suicide",mod, CTF_SUICIDE_POINTS );	// StdLog
			}
			self->enemy = NULL;

			if (special_message)
			{
//                                safe_cprintf(self, PRINT_HIGH, "[YOU KILLED YOURSELF: ");
//                                if ((inflictor) && (inflictor->classname))
//                                        safe_cprintf(self, PRINT_HIGH, " item=%s",inflictor->classname);
//                                safe_cprintf(self, PRINT_HIGH, "]\n");
			}


//if (wfdebug) gi.dprintf("diseased = %d. Diseased by = %d\n", self->disease, self->diseased_by);

			//If player is diseased, give credit to nurse who infected them
			if(self->disease && self->diseased_by && self->diseased_by->client && self->diseased_by->inuse)
			{
				if (PlayerChangeScore(self->diseased_by,CTF_FRAG_POINTS))
						sl_LogScore( &gi, self->diseased_by->client->pers.netname, self->client->pers.netname, "Kill",mod, CTF_FRAG_POINTS );	// StdLog - Mark Davies

				my_bprintf (PRINT_MEDIUM,"%s gets a frag for %s's suicide while diseased.\n", self->diseased_by->client->pers.netname, self->client->pers.netname);
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
			case MOD_ROCKET:
				message = "ate";
				message2 = "'s rocket";
				break;
			case MOD_MISSILE:
				message = "was eliminated by";
				message2 = "'s missile launcher";
				break;
			case MOD_HOMINGROCKET:
				message = "was slammed";
				message2 = "'s homing rocket";
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

//WF
			case MOD_WF_LASERBALL:
				message = "was fried by";
				message2 = "'s laserball";
				break;

			case MOD_WF_GOODYEAR:
				message = "was popped by";
				message2 = "'s goodyear grenade";
				break;

			case MOD_WF_PROXIMITY:
				message = "swallowed";
				message2 = "'s proximity grenade";
				break;

			case MOD_WF_CLUSTER:
				message = "was ripped by";
				message2 = "'s cluster grenade";
				break;

			case MOD_WF_PIPEBOMB:
				message = "fell for";
				message2 = "'s pipebomb";
				break;

			case MOD_WF_EARTHQUAKE:
				message = "was shaken by";
				message2 = "'s earthquake";
				break;

			case MOD_WF_FLAME:
				message = "was burned by";
				message2 = "'s flame";
				break;

			case MOD_FLAMETHROWER:
				message = "was charred by";
				message2 = "'s flame-thrower";
				break;

			case MOD_REVERSE_TELEFRAG:
				message = "was reverse telefragged by";
				break;
			case MOD_NAG:
				message = "was NAGed to death by";
				break;
			case MOD_SHRAPNEL:
				message = "was pithed by";
				message2 = "'s shrapnel grenade";
				break;
			case MOD_CLUSTERROCKET:
				message = "was splattered by";
				message2 = "'s cluster rocket";
				break;
			case MOD_DISEASE:
				message = "died from";
				message2 = "'s disease";
				break;

			case MOD_SNIPERRIFLE:
				message = "was slaughtered by";
				message2 = "'s sniper rifle";
				break;
			case MOD_SNIPERRIFLE_LEG:
				message = "had their legs shot out by";
				message2 = "'s sniper rifle";
				break;
			case MOD_SNIPERRIFLE_HEAD:
				message = "had his head blown off by";
				message2 = "'s sniper rifle";
				break;

			case MOD_SHC:
				message = "burst into flames from";
				message2 = "'s SHC rifle";
				break;

			case MOD_NEEDLER:
				message = "was needled by";
				break;

			case MOD_CONCUSSION:
				message = "lost his head from";
				message2 = "'s concussion grenade";
				break;

			case MOD_ARMORDART:
				message = "was pierced by";
				message2 = "'s dart";
				break;

			case MOD_INFECTEDDART:
				message = "was infected by";
				message2 = "'s dart";
				break;

			case MOD_NAPALMROCKET:
				message = "was smoked by";
				message2 = "'s napalm rocket";
				break;
			case MOD_NAPALMGRENADE:
				message = "got crispy from";
				message2 = "'s napalm grenade";
				break;

			case MOD_LIGHTNING:
				message = "saw lightning from";
				message2 = "'s lightning gun";
				break;

			case MOD_TELSA:
				message = "was exposed to";
				message2 = "'s telsa coil";
				break;

			case MOD_MAGNOTRON:
				message = "was sucked into";
				message2 = "'s magnotron";
				break;

			case MOD_SHOCK:
				message = "was shocked by";
				break;

			case MOD_PELLET:
				message = "was pelted by";
				break;

			case MOD_FLAREGUN:
				message = "was embarrassed to be toasted by";
				message2 = "'s flare gun";
				break;

			case MOD_FLARE:
				message = "was snuffed by";
				message2 = "'s flare";
				break;

			case MOD_TRANQUILIZER:
				message = "slowed to a stop from";
				message2 = "'s tranquilizer";
				break;

			case MOD_LRPROJECTILE:
				message = "was blasted by";
				message2 = "'s LR Projectile Launcher";
				break;

			case MOD_BOLTEDBLASTER:
				message = "expired from";
				message2 = "'s bolted blaster";
				break;

			case MOD_WF_TURRET:
				message="caught Turret's Syndrome from";
				message2="'s Turret Grenade.";
				break;

			case MOD_PLASMABOMB:
				message = "was vaporized by";
				break;
			case MOD_NAIL:
				message = "was nailed by";
				//message2 = "";
				break;
			case MOD_MBPC:
				message = "was disintegrated by";
				message2 = "'s Pulse Cannon";
				break;
			case MOD_SENTRY:
				message = "was laid low by";
				message2 = "'s Sentry Gun";
				break;
			case MOD_SENTRY_ROCKET:
				message = "was liquidated by";
				message2 = "'s Sentry Gun Rocket";
				break;
			case MOD_KAMIKAZE:
				message = "didn't survive";
				message2 = "'s Kamikaze run";
				break;
			case MOD_TRANQUILIZERDART:
				message = "did not escape";
				message2 = "'s tranquilizer dart";
				break;
			case MOD_DEPOT:
				message = "was blown to bits by";
				message2 = "'s supply depot";
				break;
			case MOD_DEPOT_EXPLODE:
				message = "was in the way when";
				message2 = " blew up a supply depot";
				break;
			case MOD_HEALINGDEPOT:
				message = "was blown to bits by";
				message2 = "'s healing depot";
				break;
			case MOD_MEGACHAINGUN:
				message = "was cut to ribbons";
				message2 = "'s mega chaingun";
				break;
			case MOD_SENTRYKILLER:
				message = "got in the way of ";
				message2 = "'s Sentry Killer Rocket";
				break;

			case MOD_KNIFE:
				message = "was punctured by";
				message2 = "'s knife";
				break;

			case MOD_STINGER:
				message = "was stopped by";
				message2 = "'s stinger";
				break;

			case MOD_KNIFEBACK:
				message = "was stabbed in the back by";
				message2 = "'s knife";
				break;

			case MOD_FREEZER:
				message = "was iced by";//acrid 3/99 death msg
				message2 = "'s freezer";
				break;

			case MOD_LASERCUTTER:
				message = "was sliced and diced by";
				message2 = "'s laser cutter grenade";
				break;

			case MOD_TESLA:
				message = "was electrified by";
				message2 = "'s tesla grenade";
				break;

			case MOD_GASGRENADE:
				message = "was fumigated by";
				message2 = "'s gas grenade";
				break;

			case MOD_AK47:
				message = "became swiss cheese from";
				message2 = "'s AK47";
				break;

			case MOD_PISTOL:
				message = "was assassinated by ";
				message2 = "'s pistol";
				break;

			case MOD_TARGET_LASER:
				message = "saw";
				message2 = "'s light";
				break;

			case MOD_CAMERA:
				message = "smiled for";
				message2 = "'s camera";
				break;


			}
			if (message)
			{
				my_bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if (deathmatch->value)
				{
				if (ff)
				{
					if (PlayerChangeScore(attacker,CTF_SUICIDE_POINTS))
						sl_LogScore( &gi, attacker->client->pers.netname, self->client->pers.netname, "Friendly Fire",mod, CTF_SUICIDE_POINTS );	// StdLog - Mark Davies
				}
				else
				{
					if (PlayerChangeScore(attacker,CTF_FRAG_POINTS))
						sl_LogScore( &gi, attacker->client->pers.netname, self->client->pers.netname, "Kill",mod, CTF_FRAG_POINTS );	// StdLog - Mark Davies
				}
			}
			return;
		}
	}
	}

	my_bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);

//        safe_cprintf(self, PRINT_HIGH, "[YOU WERE KILLED WITH: ");
//      if ((inflictor) && (inflictor->classname))
//                safe_cprintf(self, PRINT_HIGH, " %s",inflictor->classname);
//        if ((attacker) && (attacker->client) && (attacker->client->pers.netname))
//                safe_cprintf(self, PRINT_HIGH, " BY %s",attacker->client->pers.netname);
//        safe_cprintf(self, PRINT_HIGH, "]\n");

	if (deathmatch->value)
		PlayerChangeScore(self, -1);
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

//WF  If player classes are on, don't drop a weapon
	if (self->client && (((int)wfflags->value & WF_NO_PLAYER_CLASSES) == 0))
		return;
//WF

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

	//Reset fov if autozoom was on
	if (self->client->pers.autozoom)
	{
		self->client->ps.fov = 90;
		self->PlayerSnipingZoom = 0;
	}

	//Free laser sight
	if ( self->lasersight )
	{
		G_FreeEdict(self->lasersight);
		self->lasersight = NULL;
	}

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

//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);

//WF
/*
		//Drop all the keys you are holding
		for (n = 0; n < game.num_items; n++)
		{
			//If this is a key and I'm holding it, then drop it
			if ((deathmatch->value) && (itemlist[n].flags & IT_KEY) )
			{
				if  (self->client->pers.inventory[n])
				{
					Drop_General(self, &itemlist[n]);
//gi.dprintf("Dropping key: %s\n", itemlist[n].classname);
				}
				else
				{
//gi.dprintf("Key not in inventory: %d: %s\n", n, itemlist[n].classname);
				}

			}
		}
*/
//WF
		WFPlayer_Die (self);
		Kamikaze_Cancel(self);

//ZOID
		CTFFragBonuses(self, inflictor, attacker);
//ZOID
		TossClientWeapon (self);
//ZOID
		if (self->bot_client)//newgrap 4/99
			CTFPlayerResetGrapple(self);
		else//newgrap 4/99
//		CTFPlayerResetGrapple(self);
			CTFPlayerResetGrapple2(self);//newgrap 4/99

		CTFDeadDropFlag(self);
		CTFDeadDropTech(self);
//ZOID
		if (deathmatch->value && !self->client->showscores)
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

	// clear inventory
//3.20	memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));

//WF & ATTILA begin
	if ( Jet_Active(self) )
	{
		Jet_BecomeExplosion( self, damage );
		/*stop jetting when dead*/
		self->client->Jet_framenum = 0;
	}
//WF & ATTILA

	if (self->health < -40)//dont use its fixed || (self->client->player_special & SPECIAL_DISGUISE))
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
	else if (self->frozen && !self->deadflag)//acrid 3/99 frozen death animation
	{
		self->s.effects |= EF_COLOR_SHELL;
		self->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
		self->client->anim_priority = ANIM_DEATH;
		self->s.frame = FRAME_stand01;
		self->client->anim_end = FRAME_stand01;
		self->frozenbody = 1;
		gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
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
	self->frozen = 0;//acrid 3/99


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
	int			i;
	int			HasVoted = client->pers.HasVoted;
	int			tmpGrenade[GRENADE_TYPE_COUNT + 1];
	int			tmpSpecial[SPECIAL_COUNT + 1];
    qboolean	homing_state = client->pers.homing_state;
	int			laseron = client->pers.laseron;
	int			feign = client->pers.feign;
	int			autozoom = client->pers.autozoom;
	int			fastaim = client->pers.fastaim;
	int			nospam_level = client->pers.nospam_level;
	qboolean	autoconfig = client->pers.autoconfig;
    int			player_class = client->pers.player_class;
    int			next_player_class = client->pers.next_player_class;
	edict_t		*friend_ent;
	int			hasfriends = client->pers.hasfriends;
	int			isbot = client->pers.i_am_a_bot;

	friend_ent = client->pers.friend_ent[0];

	//Save grenade and special counts
	for (i=1; i <= GRENADE_TYPE_COUNT; ++i)
	{
		tmpGrenade[i] = client->pers.active_grenades[i];
	}

	for (i=1; i <= SPECIAL_COUNT; ++i)
		tmpSpecial[i] = client->pers.active_special[i];

	memset (&client->pers, 0, sizeof(client->pers));

	//Restore grenade and special counts
	for (i=1; i <= GRENADE_TYPE_COUNT; ++i)
	{
		if (tmpGrenade[i] < 0) tmpGrenade[i] = 0;
		client->pers.active_grenades[i] = tmpGrenade[i];
	}

	for (i=1; i <= SPECIAL_COUNT; ++i)
		client->pers.active_special[i] = tmpSpecial[i];


	item = FindItem("Blaster");
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.inventory[client->pers.selected_item] = 1;

	client->pers.weapon = item;
//ZOID
	client->pers.lastweapon = item;
//ZOID

//ERASER START
	item = FindItem("Grapple");
//	if (grapple->value || ctf->value)
	if (ctf->value)
	{//E
//ZOID
		client->pers.inventory[ITEM_INDEX(item)] = 1;//NORMAL LINE
//ZOID
	}//E
	else//E
	{//E
		client->pers.inventory[ITEM_INDEX(item)] = 0;//E
	}
//ERASER END GRAPPLE TROUBLE HERE FIXME ACRID

	client->pers.i_am_a_bot = isbot;

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.connected = true;
	client->pers.friend_ent[0] = friend_ent;

	// WF & CCH: reset homing_state
	client->pers.HasVoted = HasVoted;

    client->pers.homing_state = homing_state;
	client->pers.laseron = laseron;
	client->pers.feign = feign;
	client->pers.autozoom = autozoom;
	client->pers.fastaim = fastaim;
	client->pers.nospam_level = nospam_level;
	client->pers.autoconfig = autoconfig;
    client->pers.player_class = player_class;
    client->pers.next_player_class = next_player_class;

	client->pers.hasfriends = hasfriends;

	// Initialize grenade type
	client->pers.grenade_num    = 1;
	client->pers.grenade_type    = GRENADE_TYPE_NORMAL;
	if (((int)wfflags->value & WF_NO_PLAYER_CLASSES)  == 0)
		client->pers.grenade_type = client->grenade_type1;

    // Scanner
//	ClearScanner(client);

    // Scanner
//	CompassOff(client);//5/99 compass

}


qboolean is_bot=false;//ERASER
void InitClientResp (gclient_t *client, edict_t *ent)
{
//	int i;
	int ctf_team;

	//K2:begin
	qboolean inServer = client->resp.inServer;
	//K2:End 3/99 botcam

	ctf_team = client->resp.ctf_team;

	memset (&client->resp, 0, sizeof(client->resp));

	client->resp.ctf_team = ctf_team;

	//K2:begin
	client->resp.inServer = inServer;
	//K2:End 3/99 botcam

	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;

//ZOID
//	if (ctf->value && (client->resp.ctf_team < CTF_TEAM1))
	if (client->resp.ctf_team < CTF_TEAM1)
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

	/*
	for (n = 0; n < num_players; n++)//WF24 LINE IS 	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];
	*/

	for (n = 0; n < num_players; n++)//WF24 LINE IS 	for (n = 1; n <= maxclients->value; n++)
	{
		player = players[n];//ERASER ,WF USES LINE ABOVE THIS

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
	int		count = 0, spot_num;//ERASER
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;
//WF USES THIS CO LINE
//	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	while (count < num_dm_spots)
	{
		spot = dm_spots[count];//ERASER

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
	spot_num = 0;//ERASER
	do
	{
//WF24 USES spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		spot = dm_spots[spot_num++];//ERASER
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
	int		spot_num;//ERASER


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	spot_num = 0;//ERASER
//WF24 USES 	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	while (spot_num < num_dm_spots)//ERASER
	{
		spot = dm_spots[spot_num++];//ERASER

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
//WF USES 	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	spot = dm_spots[0];//ERASER

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
//		gi.dprintf("DEBUG - No CTF Spawn Point.\n");
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
//			gi.dprintf("DEBUG - No Spawn Point Without Target.\n");
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

//ERASER START
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
//ERASER END

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
 //   body->frozenbody =1;//acrid 3/99 this may be a solution if theres problems between the ent pointer and body
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
//	body->s.effects = 0;
//	body->s.renderfx = 0;
	body->movetype = ent->movetype;

	body->die = body_die;
	body->takedamage = DAMAGE_YES;

	gi.linkentity (body);
}

void WFCopyToBodyQue (edict_t *ent)
{
	edict_t		*body;
	char modelname[64];//acrid
	int classnum;//acrid

	// grab a body que and cycle to the next one
	body = &g_edicts[(int)maxclients->value + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// FIXME: send an effect on the removed body

	gi.unlinkentity (ent);

	gi.unlinkentity (body);


//new skin fix code
	body->wf_team = ent->client->resp.ctf_team;
	body->model = ent->model;//needed????? 3/99
	classnum = ent->client->pers.player_class;
	sprintf(modelname, "wfactory/models/decoys/%s/tris.md2", classinfo[classnum].model_name);
	body->s.modelindex = gi.modelindex (modelname);
	body->s.skinnum = classinfo[classnum].decoyskin;
	if (body->wf_team == CTF_TEAM1)	//team 1
	{
		++body->s.skinnum;
	}
	else
	{
     //team 2
	}
//end new skin fix

 //	body->s = ent->s;
    VectorCopy(ent->s.origin, body->s.origin);//acrid
	VectorCopy(ent->s.angles, body->s.angles);//acrid
	VectorCopy(ent->s.old_origin, body->s.old_origin);//acrid
	body->s.frame = ent->s.frame;//acrid
	body->s.number = body - g_edicts;
	body->svflags = ent->svflags;
	VectorCopy (ent->mins, body->mins);
	VectorCopy (ent->maxs, body->maxs);
	VectorCopy (ent->absmin, body->absmin);
	VectorCopy (ent->absmax, body->absmax);
	VectorCopy (ent->size, body->size);
	body->solid = ent->solid;
	body->s.event = ent->s.event;//acrid
	body->clipmask = ent->clipmask;
	body->owner = ent->owner;
	body->s.effects = ent->s.effects;
	body->s.renderfx = ent->s.renderfx;

	//If they were carrying the flag, clear the glow - GREGG
	if (body->s.effects & (EF_FLAG1 | EF_FLAG2))
	{
		body->s.effects = 0;
		body->s.renderfx = 0;
	}


	body->movetype = ent->movetype;
	body->die = body_die;
	body->takedamage = DAMAGE_YES;

	gi.linkentity (body);
}

void respawn (edict_t *self)
{
	if (deathmatch->value || coop->value)
	{
        if (((int)wfflags->value & WF_NO_PLAYER_CLASSES) == 0)
		{ //for stopping instant gib creating a extra body//may need !self->frozenbody
			if (self->health > -40 && !self->bot_client)
		        WFCopyToBodyQue (self);
		  //use normal que if instant gib
		      else
                  CopyToBodyQue (self);
		}
		//if wfflags use normal que
	    else
		{
		CopyToBodyQue (self);
		}



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

// WF: Weapons Factory user setup */
static void WFClientSetup (edict_t *ent)
{
	int i;

    //Set aliases required for mod
	stuffcmd(ent,"alias +thrust \"cmd thrust on\"\n");
	stuffcmd(ent,"alias -thrust \"cmd thrust off\"\n");
    stuffcmd(ent,"alias +grapple \"+use\"\n");//newgrap 4/99
    stuffcmd(ent,"alias -grapple \"-use\"\n");//newgrap 4/99
	stuffcmd(ent,"alias +rzoom \"fov 30;+mlook\"\n");
	stuffcmd(ent,"alias -rzoom \"fov 90;-mlook\"\n");
	//stuffcmd(ent,"alias wfhelp \"cmd wfhelp\"\n");
	//stuffcmd(ent,"alias vote \"cmd vote\"\n");

	//FORCE SOME BINDS
//	stuffcmd(ent,"bind \";\" cmd special\n");
//	stuffcmd(ent,"bind v +thrust\n");
//	stuffcmd(ent,"bind b cmd grenade\n");

	//note - removed mlook from rzoom command

	//Clear grenade and special item limits
	for (i=1; i <= GRENADE_TYPE_COUNT; ++i)
		ent->client->pers.active_grenades[i] = 0;

	for (i=1; i <= SPECIAL_COUNT; ++i)
		ent->client->pers.active_special[i] = 0;

}
//WF

/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/

void ShowGun(edict_t *ent);//ERASER

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
//ERASER START
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

	HealPlayer(ent);

	//Did class change?
	if (ent->client->pers.player_class != ent->client->pers.next_player_class)
	{
		//Remove all their old stuff lying around
//		gi.dprintf("Class changed-removing stuff.  Old class=%d, New Class=%d\n",ent->client->pers.player_class, ent->client->pers.next_player_class);
		WFPlayer_ChangeClassTeam(ent);
	}

	ent->client->pers.player_class = ent->client->pers.next_player_class;

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
//ERASER START
	team = client->team;
	lag_trail = client->lag_trail;
	lag_angles = client->lag_angles;
	latency = client->latency;
//ERASER END
	memset (client, 0, sizeof(*client));

	client->pers = saved;
	client->resp = resp;
//ERASER START
	client->team = team;
	client->lag_trail = lag_trail;
	client->lag_angles = lag_angles;
	client->latency = latency;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
//ERASER END
	// copy some data from the client to the entity
	FetchClientEntData (ent);

	if (!ent->wf_team) ent->wf_team = client->resp.ctf_team;

	// clear entity values
	ent->groundentity = NULL;
	if (!ent->bot_client)//ERASER
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

//ERASER START
	if (!ent->bot_client)
	{
		ent->pain = player_pain;
		ent->die = player_die;
	}
//ERASER END MIDDLE 2 LINES NORMAL WF24 CODE

	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;
	// acrid 3/99 start unfrozen
	ent->frozen = 0;
    ent->frozenbody = 0;
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

	if (!ent->bot_client)//ERASER
	{//E
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


	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	ent->s.effects = 0;
	// sknum is player num and weapon number
	// weapon number will be added in changeweapon
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->s.modelindex2 = 255;		// custom gun model
	if (ent->bot_client)
	{
      ShowGun(ent);
	}
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

//WF - for flash grenades
    client->blindBase = 0;
    client->blindTime = 0;

	//Clear disguise stuff
	ent->disguised = 0;
	ent->disguising = 0;
	ent->disguisetime = 0;
	ent->disguisingteam = 0;
	ent->disguisedteam = 0;
	ent->disguiseshots = 0;

	lasersight_off (ent);

	//Stop any disease
	ent->disease = 0;
	ent->cantmove = 0;
	ent->Slower = 0;

	ent->client->weapon_damage = DAMAGE_BLASTER;	//blaster is startup weapon

//WF
	//set the team of the entity
	ent->wf_team = client->resp.ctf_team;

	//set respawn protection time
    ent->client->protecttime = level.time + RESPAWN_PROTECT_TIME;

	//Set player classes

	//Fixup class if team is set but there is no next_player_class
/*
	if ((client->resp.ctf_team > 0) && (ent->client->pers.next_player_class == 0))
	{
		//Can we use current setting?
		if (ent->client->pers.player_class)
		{
			ent->client->pers.next_player_class = ent->client->pers.player_class;
		}
		//Just assign them to the first class if all else fails
		else
		{
			ent->client->pers.next_player_class = 1;
		}
	}
*/
/* Old position of this code
	//Did class change?
	if (ent->client->pers.player_class != ent->client->pers.next_player_class)
	{
		//Remove all their old stuff lying around
//		gi.dprintf("Class changed-removing stuff.  Old class=%d, New Class=%d\n",ent->client->pers.player_class, ent->client->pers.next_player_class);
		WFPlayer_ChangeClassTeam(ent);
	}

	ent->client->pers.player_class = ent->client->pers.next_player_class;

*/
	if ((((int)wfflags->value & WF_NO_PLAYER_CLASSES) == 0) && (ent->client->pers.player_class))
	{
//		gi.dprintf("Player Classes Used\n");
		wf_InitPlayerClass(ent->client);
	}
//    else if ((int)wfflags->value & WF_RESPAWN_ARMED)
//	{
//		ent->client->pers.next_player_class = CLASS_SUPERMAN;
//		ent->client->pers.player_class = CLASS_SUPERMAN;
//		wf_InitPlayerClass(ent->client);
//	}
	else
	{
		ent->client->pers.next_player_class = 0;
		ent->client->pers.player_class = 0;
	}
	Cmd_ShowClass(ent);

	//Pick the best weapon for the class
	NoAmmoWeaponChange (ent);



//ZOID
	if (CTFStartClient(ent))
		return;
//ZOID


	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);
//ERASER START
	if (!ent->map)
		ent->map = G_CopyString(ent->client->pers.netname);
//ERASER END

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	if (!ent->bot_client)//ERASER
		ChangeWeapon (ent);


	//If this is a zbot player, mess them up.
	if (ent->client->pers.i_am_a_bot)
		I_AM_A_ZBOT(ent);

//ERASER START
	ent->last_max_z = 32;
	ent->jump_ent = NULL;
	ent->duck_ent = NULL;
	ent->last_trail_dropped = NULL;

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

		if ((bot_calc_nodes->value) && (trail[0]->timestamp))
		{	// find a trail node to start checking from
			if ((i = ClosestNodeToEnt(ent, false, true)) > -1)
			{
				ent->last_trail_dropped = trail[i];
			}
		}
	}
//ERASER END
}

/* Anti-ZBot Support Code */
void wf_strdate(char *s);
void wf_strtime(char *s);

void  zbotFileOpen(  )
{
    char path[100];

	zbotfile = NULL;

	//Is zbot detection on?
    if (((int)wfflags->value & WF_ZBOT_DETECT) == 0)
        return;

    strcpy(path, gamedir->string);

#if defined(_WIN32) || defined(WIN32)
    strcat(path,"\\zbot.log");
#else
    strcat(path,"/zbot.log");
#endif

    if((zbotfile = fopen(path,"a")) != NULL)
    {
        fprintf(zbotfile,"\nSERVER RESTART\n");
		gi.dprintf("ZBot Protection Enabled\n");
    }
}

void zbotLogAttack(edict_t *ent, int type)
{
	char tDate[50];
	char tTime[50];
	char *stype;

	if (type == 2) stype = "@zbot";
	else if (type == 3) stype = "#zbot";
	else stype = "!zbot";

	if (!zbotfile) return;

	//value = Info_ValueForKey (userinfo, "ip");
	wf_strdate( tDate );
	wf_strtime( tTime );

	fprintf(zbotfile,"%s %s: Name = %s, Type = %s\n",
		tDate, tTime, ent->client->pers.netname, stype);
	fflush(zbotfile);

}

void zbotFileClose()
{
	if (zbotfile) fclose(zbotfile);
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
//ERASER START
	if (ctf->value)
		ent->client->team = NULL;
//ERASER END
	InitClientResp (ent->client, ent);//WF34 ADDED ENT
	if (!ent->wf_team) ent->wf_team = ent->client->resp.ctf_team;


	// locate ent at a spawn point
	PutClientInServer (ent);
	ent->client->pers.autozoom = 1;         //default sniper rifle to auto-zoom
//WF34 START
	// -- ANTI-ZBOT STARTS -- //
	//ent->client->resp.bot_start = level.time + 5 + (rand() % 5);
	//ent->client->resp.bot_end = 0;
	//ent->client->resp.bot_end2 = 0;
	//ent->client->resp.bot_end3 = 0;

	//Send these commands.  User must respond back with all 3
    // between 5 and 9 seconds later.
	/* C.O. For Lithium Test
    if ((int)wfflags->value & WF_ZBOT_DETECT)
	{
		stuffcmd(ent, "!zbot\n");
		ent->client->resp.bot_end = level.time + 4;
		stuffcmd(ent, "@zbot\n");
		ent->client->resp.bot_end2 = level.time + 4;
		stuffcmd(ent, "#zbot\n");
		ent->client->resp.bot_end3 = level.time + 4;

		ent->client->resp.bot_retries = 0;
		ent->client->resp.bot_retries = 2;
		ent->client->resp.bot_retries = 3;
	}
	C.O. For Lithium Test */
	// -- ANTI-ZBOT ENDS -- //

	//Enforce cl_forwardspeed and cl_sidespeed 5/99 acrid
	if (!ent->bot_client)
	{
      stuffcmd(ent,"cl_forwardspeed 200\n");
      stuffcmd(ent,"cl_sidespeed 200\n");
	}

//ERASER START
    ///Q2 Camera Begin
    EntityListAdd(ent);
    ///Q2 Camera End
//ERASER END
//WF34 SE
    // Voting
	ent->client->pers.HasVoted = false;

	// Homing
	ent->client->pers.homing_state = 1;

	// Feign
	ent->client->pers.feign = 0;

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{//WF34 E
	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	}//WF34
	my_bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

//WF24 S Setup user
    WFClientSetup(ent);
	sl_LogPlayerConnect( &gi, level, ent ); // StdLog - Mark Davies
//WF24 E

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*///WF24 IS A BIT DIF
void botAddPlayer(edict_t *ent);
void ClientBegin (edict_t *ent)
{
	int		i;

	//Check if player in cam mode botcam
	if(ent->client->bIsCamera)
	{
		ent->client->bIsCamera = false;
		botAddPlayer(ent);
	}

	ent->client = game.clients + (ent - g_edicts - 1);
//ERASER START
	players[num_players++] = ent;
	num_clients++;


	if (!ctf->value)
	{	// make sure grapple is removed if CTF has been disabled
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))] = 0;
	}
//ERASER END
	if (deathmatch->value)
	{
//ERASER START  (TeT - commented out to prevent overflows)
//		if (bot_show_connect_info->value)
//			gi.centerprintf(ent, "\n\n=====================================\nThe Eraser Bot v%1.3f\nby Ryan Feltrin (aka Ridah)\n\nRead the readme.txt file\nlocated in the Eraser directory!\n\nVisit http://impact.frag.com/\nfor Eraser news\n\n-------------------------------------\n", ERASER_VERSION, maxclients->value);
//
//		if (teamplay->value && !ctf->value)
//			safe_cprintf(ent, PRINT_HIGH, "\n\n=====================================\nServer has enabled TEAMPLAY!\n\nType: \"cmd teams\" to see the list of teams\nType: \"cmd join <teamname>\" to join a team\n\n");
//ERASER END
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
		InitClientResp (ent->client, ent);//WF34 ADDED ENT
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
//WF24 S Setup user
    WFClientSetup(ent);
//WF24 E

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

	//Remove any "%" from name
	wfFixName(s);

    // start - mdavies
    // Has the player got a name
    if( strlen(ent->client->pers.netname) )
    {
        // has the name changed
        if( strcmp( ent->client->pers.netname, s ) )
        {
            //int iTimeInSeconds = level.time;

            // log player rename
            sl_LogPlayerRename( &gi,
                                ent->client->pers.netname,
                                s);
        }
    }
    // end - mdavies
	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;

	// combine name and skin into a configstring
//ZOID
//	if (ctf->value)
		CTFAssignSkin(ent, s);
//	else
//ZOID
//		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

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
	char    *tmp;
	int i;

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");

	// drop port #
    tmp = value;
if (!ent->bot_client)
	while (tmp)
	{
		if (*(++tmp) == ':')
		{
			*tmp = (char) NULL;
			break;
		}
	}

	if (IsBanned(value))
	{
		gi.dprintf("*** Banned IP %s tried to connect!\n", value);
		Info_SetValueForKey(userinfo, "rejmsg", "You are banned.");
		return false;
	}

	// check for a password
	if (!ent->bot_client)//ERASER
	{
		value = Info_ValueForKey (userinfo, "password");
        if ((wfpassword) &&(strcmp(wfpassword->string, value) != 0))
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
//ZOID
		is_bot = ent->bot_client;//ERASER// make sure bot's join a team
		InitClientResp (ent->client, ent);//WF34 ADDED ENT
		is_bot = false;//ERASER

		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
		if (!ent->wf_team) ent->wf_team = ent->client->resp.ctf_team;
	}
//ERASER START
	// do real client specific stuff
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
//ERASER END

    // Clear active grenade and special item counts
//gi.dprintf("Debug: clear grenade counts. After =\n");
	for (i=1; i <= GRENADE_TYPE_COUNT; ++i)
		ent->client->pers.active_grenades[i] = 0;

	for (i=1; i <= SPECIAL_COUNT; ++i)
		ent->client->pers.active_special[i] = 0;

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf ("%s connected\n", ent->client->pers.netname);

   //make sure they don't start with a old players class 5/99
	if (!ent->bot_client)
	{
  		if (ent->client->pers.player_class != 0)
  		{
     		ent->client->pers.player_class = 0;
	 		ent->client->pers.next_player_class = 0;
		//	  gi.dprintf("Connected with a class\n");
  		}
	}

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
void botRemovePlayer(edict_t *self);//ERASER

void ClientDisconnect (edict_t *ent)
{
	int		playernum;
	//K2
	int i;
	edict_t *other;
	//K2

	if (!ent->client)
		return;

	//See if it's the ref
	if (wf_game.ref_ent == ent)
		wf_game.ref_ent = NULL;

//ERASER START
    ///Q2 Camera Begin
    EntityListRemove(ent);
    ///Q2 Camera End

	//K2: If player was in cammode, he was already removed 3/99
	if(!ent->client->bIsCamera)
	botRemovePlayer(ent);

//ERASER END
	my_bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

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
	//K2:End //3/99 botcam acrid

	ent->client->resp.inServer = false;//3/99 botcam


	//WF
	sl_LogPlayerDisconnect( &gi, level, ent );	// StdLog - Mark Davies
	WFPlayer_ChangeClassTeam(ent);

//WF

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

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
int	last_bot=0;//ERASER
#define		BOT_THINK_TIME	0.03//ERASER// never do bot thinks for more than this time

void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j;
	pmove_t	pm;
	qboolean jetting;
//ERASER START
	int count=0, start;
	clock_t	start_time, now;

	int playernum;
    char userinfo[MAX_INFO_STRING];//acrid 3/99 freeze

	//ZBOT DETECTION FROM LITHIUM
    if ((int)wfflags->value & WF_ZBOT_DETECT)
	{
		if(ZbotCheck(ent, ucmd))
		{
			zbotLogAttack(ent, 1);
			//my_bprintf (PRINT_MEDIUM,"SYSTEM MESSAGE: %s has been kicked for using a ZBot.\n", ent->client->pers.netname);
			safe_cprintf(ent, PRINT_HIGH, "You have been kicked for using a ZBot.\n");
			stuffcmd(ent, "disconnect\n"); // kick out zbots.
			my_bprintf(PRINT_HIGH, "%s is using a ZBot!\n",ent->client->pers.netname);
			I_AM_A_ZBOT(ent);
		}
	}

	//ZBOT DETECTION FROM LITHIUM

	if (paused)
	{
		gi.centerprintf(ent, "PAUSED\n\n(type \"botpause\" to resume)");
		ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
		return;
	}

//ERASER END
//Acrid freeze 3/99
	if (ent->frozen)
	{
		if( level.time < ent->frozentime )
		{  ent->client->ps.pmove.pm_type = PM_DEAD;

	       if (ent->client->buttons & BUTTON_ATTACK)//4/99 acrid
		   {
		       ent->client->buttons &= ~BUTTON_ATTACK;
		   }

			return;
		}
		else
		{
			playernum = ent - g_edicts - 1;
			strcpy( userinfo, ent->client->pers.userinfo );
			ent->frozen = 0;
			Info_SetValueForKey( userinfo, "skin", ent->oldskin );
			ClientUserinfoChanged( ent, userinfo );
		}
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


	//Feign Acrid
	if(client->pers.feign)
	{
		if (ent->health <=0)
	        client->pers.feign = 0;
//oldfeign	    else
	//	    return;
	}

//ERASER START
    ///Q2 Camera Begin
    if (ent->client->bIsCamera)
    {
        CameraThink(ent,ucmd);
        return;
    }
    ///Q2 Camera End

	if (ent->bot_client)
		return;

       if (client->on_hook == true)//newgrap 4/99
       {
           CTFGrapplePull2(ent);
           client->ps.pmove.gravity = 0;
       }
       else
       {
           client->ps.pmove.gravity = sv_gravity->value;
       }
//gi.dprintf("f: %i, s: %i, u: %i\n", ucmd->forwardmove, ucmd->sidemove, ucmd->upmove);
//gi.dprintf("%i, %i\n", ucmd->buttons, client->ps.pmove.pm_flags);
//ERASER END 2 LINES ALREADY CO
	pm_passent = ent;

//ZOID
	if (ent->client->chase_target) {
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
		return;
	}
//ZOID

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

	//Enforce cl_forwardspeed and cl_sidespeed 5/99 acrid
	if (!ent->bot_client)
	{
		if (ucmd->forwardmove > 400 || ucmd->sidemove > 400)
		{
	 	stuffcmd(ent,"cl_forwardspeed 200\n");
     	stuffcmd(ent,"cl_sidespeed 200\n");
		}
	}

//	client->ps.pmove.gravity = sv_gravity->value;newgrap 4/99
//WF24 S
	if (ent->flags & FL_BOOTS)
		client->ps.pmove.gravity = sv_gravity->value * 0.25;
	if(ent->Slower>level.time)
	{
		ucmd->forwardmove *= 0.5;
		ucmd->sidemove *= 0.5;
		ucmd->upmove *= 0.5;
	}
    //WF & ATTILA begin
	//Turn off thrusting state if the WF_NO_FLYING flag is set
	//This is set here so the feature can be turned off mid-game
	//Some classes can't fly
	if (ent->client &&
			(((int)wfflags->value & WF_NO_FLYING) ||
		 	((ent->client->player_special & SPECIAL_JETPACK) == 0)))
	{
		ent->client->thrusting = 0;
	}

	//Thrust if you are not all the way in the water
    if ((ent->client->thrusting) && (ent->waterlevel < 3))
    //      if ( Jet_Active(ent) )
         Jet_ApplyJet( ent, ucmd );//FIXME NODES
//WF24 E & ATTILA end

	pm.s = client->ps.pmove;

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
		pm.s.velocity[i] = ent->velocity[i]*8;
	}
//WF34 START
	if(ent->superslow)
	{
//		ucmd->forwardmove *= 0.4;
//		ucmd->sidemove *= 0.4;
//		ucmd->upmove *= 0.4;
		ucmd->forwardmove *= 0.75;
		ucmd->sidemove *= 0.75;
		ucmd->upmove *= 0.75;
	}
	if(ent->lame)
	{
		ucmd->forwardmove *= 0.5;
		ucmd->sidemove *= 0.5;
		ucmd->upmove *= 0.5;
	}
//WF34 END
	//Feign Acrid
	if(client->pers.feign)//newfeign 3/99
	{
		ucmd->forwardmove *= 0.0;
		ucmd->sidemove *= 0.0;
		ucmd->upmove *= 0.0;
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

	jetting = Jet_Active(ent);
	for (i=0 ; i<3 ; i++)
	{
			if ( !jetting
				|| (jetting
					&& (fabs((float)pm.s.velocity[i]*0.125) < fabs(ent->velocity[i]))) )
			{
				ent->velocity[i] = pm.s.velocity[i]*0.125;
			}
	}

	//FIXME NODES? dont remember //newfeign 3/99
	//Feign Acrid
	if(!client->pers.feign)
	{
	VectorCopy (pm.mins, ent->mins);
	VectorCopy (pm.maxs, ent->maxs);
	}
	client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

//WF24 S & ATTILA begin
	if (jetting)
		if( pm.groundentity )               //are we on ground
			if ( Jet_AvoidGround(ent) )       //then lift us if possible
				pm.groundentity = NULL;  //FIXME NODES       //now we are no longer on ground
//WF24 E & ATTILA end


	if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
	}

	//Feign Acrid
	if(!client->pers.feign)//newfeign 3/99
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
	//Feign Acrid
	else if(!client->pers.feign)//newfeign 3/99
	{
		VectorCopy (pm.viewangles, client->v_angle);
		VectorCopy (pm.viewangles, client->ps.viewangles);
	}


//ZOID
	if (ent->bot_client)//newgrap 4/99
	{
		if (client->ctf_grapple)
			CTFGrapplePull(client->ctf_grapple);
	}
//ZOID


	// handle cloaking ability
	if (ent->client->cloaking)
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
					if (ent->client->cloakdrain == CLOAK_DRAIN)
					{
					ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= CLOAK_AMMO;
					ent->client->cloakdrain = 0;
					}
				}
				else
				{
					ent->svflags &= ~SVF_NOCLIENT;
					ent->client->cloaking = false;
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
//acrid 3/99
			if (other->frozen && other->client )
			{
			// only touch-unfreeze if we have the same skin acrid3
			if( !strcmp( Info_ValueForKey( ent->client->pers.userinfo, "skin" ), other->oldskin))
			{botDebugPrint("frozen and have same skin \n");
				// find the player number
				playernum = other - g_edicts - 1;
				// get the userinfo
				strcpy( userinfo, other->client->pers.userinfo );
				// unfreeze me and restore skin
				other->frozen = 0;
				Info_SetValueForKey( userinfo, "skin", other->oldskin );
				ClientUserinfoChanged( other, userinfo );
			}
		}
		// ******************

		other->touch (other, ent, NULL, NULL);
	}
//ERASER START
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

	if ((client->latency > 0) && !(ucmd->buttons & BUTTON_ATTACK) && (client->firing_delay > (level.time - 0.1)) && (client->firing_delay <= level.time))
	{	// remember the button after it was released when simulating lag
		ucmd->buttons |= BUTTON_ATTACK;
	}
//ERASER END
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
		if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}
//WF24 S ST
	if ((ent->client->kamikaze_mode & 1) && (ent->client->kamikaze_framenum <= level.framenum))
	  Kamikaze_Explode(ent);
//WF24 E ST

//ZOID
//regen tech
	CTFApplyRegeneration(ent);
//ZOID

//ZOID
		//K2: Changed to use playerlist botcam
	// update chase cam if being followed
//	for (i = 1; i <= maxclients->value; i++) {
//		other = g_edicts + i;
	for (i = 0; i <= num_players; i++) {

		other = players[i];

		if (!other || other->bot_client)
			continue;
		if (other->inuse && other->client->chase_target == ent)//orig from here
			UpdateChaseCam(other);
	}
//ZOID
//WF34 START
	//WF
	if(ent->cantmove)
		VectorCopy(ent->LockedPosition,ent->s.origin);
//WF34 END

//ERASER START
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
		OptimizeRouteCache();//crash???

	// =========================================================
      // Check to see if player pressing the "use" key newgrap 4/99
	if (ent->client->buttons & BUTTON_USE && !ent->deadflag &&
    	client->hook_frame <= level.framenum)
	{
		if((ent->client->player_special & SPECIAL_GRAPPLE) == 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "Sorry - This class cannot use the grapple.\n");
			return;
		}
		else if (ent->movetype == MOVETYPE_NOCLIP)
		{
			return;
		}
		else
		{
			int		damage = 10;
			CTFGrappleFire2 (ent, vec3_origin, damage, 0);
			//   CTFWeapon_Grapple_Fire2 (ent);
		}
    }
	if (Ended_Grappling (client) && !ent->deadflag && client->ctf_grapple)
	{
    	CTFPlayerResetGrapple2(ent);
	}

}

//ERASER END

/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
//ERASER START
void trigger_elevator_use (edict_t *self, edict_t *other, edict_t *activator);
void Use_Plat (edict_t *ent, edict_t *other, edict_t *activator);
void CTFFlagThink(edict_t *ent);
//ERASER END
void VectorRotate(vec3_t in, vec3_t angles, vec3_t out); //WF34


void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			i;//ERASER
	int			buttonMask;
//	qboolean noise = false;
//	vec3_t temp,new;//WF34
//	float volume = 1.0;
//	int blood;//WF34

	if (ent->bot_client)//ERASER
		return;//E

	if (level.intermissiontime)
		return;
//ERASER START
    ///Q2 Camera Begin
    if (ent->client->bIsCamera)
    {
        return;
    }
    ///Q2 Camera End
//ERASER END
	client = ent->client;

	/* C.O. For Lithium Test
    // -- ANTI-ZBOT STARTS -- //
	if (ent->client->resp.bot_start > 0 && ent->client->resp.bot_start <= level.time)
	{
		//ent->client->resp.bot_start = 0;
		//stuffcmd(ent, "!zbot\n");
		//ent->client->resp.bot_end = level.time + 4;
		//stuffcmd(ent, "@zbot\n");
		//ent->client->resp.bot_end2 = level.time + 4;
		//stuffcmd(ent, "#zbot\n");
		//ent->client->resp.bot_end3 = level.time + 4;
	}

	if (ent->client->resp.bot_end > 0 && ent->client->resp.bot_end <= level.time)
	{
		//If the player ping is too high, try again later
		if (ent->client->resp.bot_retries <= 4)
		{
			ent->client->resp.bot_start = level.time + 5 + (rand() % 5);
			ent->client->resp.bot_end = 0;
			++ent->client->resp.bot_retries;
		}
		else
		{
			ent->client->resp.bot_end = 0;
			zbotLogAttack(ent, 1);
			//my_bprintf (PRINT_MEDIUM,"SYSTEM MESSAGE: %s has been kicked for using a ZBot.\n", ent->client->pers.netname);
			safe_cprintf(ent, PRINT_HIGH, "You have been kicked for using a ZBot (or you have a bad connection).\n");
			stuffcmd(ent, "disconnect\n"); // kick out zbots.
		}
	}

	if (ent->client->resp.bot_end2 > 0 && ent->client->resp.bot_end2 <= level.time)
	{
		//If the player ping is too high, try again later
		if (ent->client->resp.bot_retries2 <= 4)
		{
			ent->client->resp.bot_start = level.time + 5 + (rand() % 5);
			ent->client->resp.bot_end2 = 0;
			++ent->client->resp.bot_retries2;
		}
		else
		{
			ent->client->resp.bot_end2 = 0;
			zbotLogAttack(ent, 2);
			//my_bprintf (PRINT_MEDIUM,"SYSTEM MESSAGE: %s has been kicked for using a ZBot.\n", ent->client->pers.netname);
			safe_cprintf(ent, PRINT_HIGH, "You have been kicked for using a ZBot (or you have a bad connection).\n");
			stuffcmd(ent, "disconnect\n"); // kick out zbots.
		}
	}

	if (ent->client->resp.bot_end3 > 0 && ent->client->resp.bot_end3 <= level.time)
	{
		//If the player ping is too high, try again later
		if (ent->client->resp.bot_retries3 <= 4)
		{
			ent->client->resp.bot_start = level.time + 5 + (rand() % 5);
			ent->client->resp.bot_end3 = 0;
			++ent->client->resp.bot_retries3;
		}
		else
		{
			ent->client->resp.bot_end3 = 0;
			zbotLogAttack(ent, 3);
			//my_bprintf (PRINT_MEDIUM,"SYSTEM MESSAGE: %s has been kicked for using a ZBot.\n", ent->client->pers.netname);
			safe_cprintf(ent, PRINT_HIGH, "You have been kicked for using a ZBot (or you have a bad connection).\n");
			stuffcmd(ent, "disconnect\n"); // kick out zbots.
		}
	}


	C.O. For Lithium Test

	*/
	// -- ANTI-ZBOT ENDS -- //

	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk
//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
//ZOID
		)
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;

//ERASER START
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
//ERASER END

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

/*	if (!deathmatch->value)
    //	if ((int)wfflags->value & WF_DECOY_PURSUE)

		if (!visible (ent, PlayerTrail_LastSpot() ) )
//			PlayerTrail_Add (ent->s.old_origin);//WF24 LINE//ACRID FIXME MAJOR TROUBLE
            PlayerTrail_Add (ent, ent->s.old_origin, NULL, false, false, NODE_NORMAL);
*///FIXME NODES
  //WF24 E
	client->latched_buttons = 0;
//WF34 START

	if ((ent->disguised ||ent->disguisedteam) && (ent->disguiseshots==0))
	{
		WFRemoveDisguise(ent);
	}

	//See if menu is up
	if (ent->client->menu)
	{
		//check timeout value of menu
		if ((client->menu->MenuTimeout) && (level.time > client->menu->MenuTimeout))
		{
			client->menu->MenuTimeout = 0;
			PMenu_Close(ent);
		}
	}


//ERASER START
//	if (gi.pointcontents(ent->s.origin) & CONTENTS_LADDER)

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
						&&	(!plyr->target_ent ||
						(plyr->target_ent->think != CTFFlagThink) ||
						(entdist(plyr, plyr->target_ent) > 1000))
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

}
//ERASER END