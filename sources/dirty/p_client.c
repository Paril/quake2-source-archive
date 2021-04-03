#include "g_local.h"
#include "m_player.h"

// Bink : It likes men.
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
        vec3_t  d;

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

        // GRIM
        /*
        // Not visible in CTF,  unless Random Spawn enabled.
        // Match games handle them elsewhere..
        if ((!ctf->value) || (ctf->value && ((int)bflags->value & BF_CTF_R_RESPAWN)))
                SP_misc_teleporter_dest (self);
        */
        // GRIM
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

// GRIM
void DeathMessage (edict_t *ent, edict_t *attacker, char *message, char *message2)
{
        edict_t         *other;
        char            v[17];
        char            *victim;
        char            k[17];
        char            *killer;
	int		j;
        qboolean        observers_only = false;

        // Here and not clientobituary, coz we want the server to get it
        if ((int)realflags->value & RF_NO_DEATHMESSAGES)
                return;

        if (!message2)
                message2 = "";

        if (turns_on)
        {
                observers_only = true;
                if ((!max_teams) && level.living < 3)
                        observers_only = false;
                else if ((level.match_state < MATCH_START) || (level.match_state == MATCH_FINISHED))
                        observers_only = false;
        }

        strncpy (v, ent->client->pers.netname, sizeof(v)-1);
        victim = strtostr2 (v);
        //victim = ent->client->pers.netname;

        if (attacker)
        {
                if (ent == attacker)
                        killer = "";
                else
                {
                        strncpy (k, attacker->client->pers.netname, sizeof(k)-1);
                        killer = strtostr2 (k);
                }
        }
        else
                killer = "";

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];

		if (!other->inuse)
			continue;

		if (!other->client)
			continue;

                if (observers_only && (other->client->resp.state == CS_PLAYING) && (other->client->resp.lives != 0))
                        continue;

                gi.cprintf (other, PRINT_HIGH, "%s%s %s%s\n", victim, message, killer, message2);
	}
}

// Dirty
void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
        int             mod;
	char		*message;
	char		*message2;
	qboolean	ff;

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

        //if (OnSameTeam (self, attacker))
        //        meansOfDeath |= MOD_FRIENDLY_FIRE;

        ff = meansOfDeath & MOD_FRIENDLY_FIRE;
        mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
        message = NULL;
        message2 = "";

        switch (mod)
        {
                case MOD_SUICIDE:
                        message = " suicides";
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
                case MOD_EXPLOSIVE: 
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
                switch (mod)
                {
                        case MOD_BLED:
                                message = " bled to death";
                                break;
                        case MOD_LAVA:
                                if (random() < 0.25)
                                        message = " took a HOT bath";
                                else if (random() < 0.5)
                                        message = " went for a swim in hot stuff";
                                else
                                        message = " landed in lava";
				break;
                        case MOD_FALLING:
                                message = " discovered gravity the hard way";
                                break;
                        case MOD_BOOBYTRAP:
                                message = " picked his own surprise package";
                                break;
                        case MOD_BARREL:
                        case MOD_EXPLODING_AMMO:
				if (IsNeutral(self))
                                        message = " blew itself up";
                                else if (IsFemale(self))
                                        message = " blew herself up";
				else
                                        message = " blew himself up";
                                break;
			case MOD_HELD_GRENADE:
                                message = " tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
                        case MOD_G_SPLASH:
                        /* Dirty
				if (IsNeutral(self))
                                        message = " tripped on its own grenade";
				else if (IsFemale(self))
                                        message = " tripped on her own grenade";
				else
                                        message = " tripped on his own grenade";
				break;
                        */
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
                        case MOD_THROWN_KNIFE:
                                message = " played \"catch\" with their own knife";
				break;
			default:
				if (IsNeutral(self))
                                        message = " killed itself";
				else if (IsFemale(self))
                                        message = " killed herself";
				else
                                        message = " killed himself";
                                break;
                }
        }

        if (message)
        {
                DeathMessage (self, NULL, message, message2);
                if (dedicated->value)
                        gi.dprintf ("%s%s\n", self->client->pers.netname, message);
                AdjustScore(self, -1, false);
                self->enemy = NULL;
                return;
        }

        self->enemy = attacker;
        if (attacker->client)
        {
                // Dirty
                if (self->is_a_lame_mofo)
                        message = " (a TK'r) was put to death by";
                else switch (mod)
                // Dirty
                {
                        case MOD_FALLING:
                                message = " was introduced to gravity by";
                                break;
                        case MOD_LAND:
                                message = " was speckered when";
                                message2 = " landed on them";
                                break;
                        case MOD_LAVA:
                                message = " was blown into the lava by";
                                break;
                        case MOD_BLED:
                                message = " bled to death after";
                                message2 = "'s gentle persuasion";
				break;
                        case MOD_BOOBYTRAP:
                                message = " picked up";
                                message2 = "'s surprise package";
                                break;
                        case MOD_BARREL:
                                message = " was blown up when";
                                message2 = " shot a nearby barrel";
                                break;
                        case MOD_EXPLODING_AMMO:
                                message = " was blown up when";
                                message2 = " made some ammo explode";
                                break;
                        case MOD_PISTOL:
                                if (HitLocation & HIT_HEAD)
                                        message = " was capped in the head by";
                                else if ((HitLocation & HIT_BACK) && (HitLocation & (HIT_CHEST | HIT_MID)))
                                        message = " was shot in the back by";
                                else
                                {
                                        message = " took a round from";
                                        message2 = "'s Pistol";
                                }
        			break;
                        case MOD_PISTOL_JW:
                                if (random() < 0.25)
                                        message = " was John Woo'ed by";
                                else if (random() < 0.5)
                                {
                                        message = " was shot-up by";
                                        message2 = "'s stylish twins";
                                }
                                else
                                {
                                        message = " succumb to";
                                        message2 = "'s fancy two gun setup";
                                }
                                break;
			case MOD_SHOTGUN:
                                message = " took some buck shot from";
                                message2 = "'s Shotgun";
				break;
                        case MOD_ASHOTGUN:
                                if ((HitLocation & HIT_HEAD) && (HitLocation & HIT_FRONT))
                                {
                                        message = "'s face was blown off by";
                                        message2 = "'s Handcannon";
                                }
                                else if ((HitLocation & HIT_BACK) && (HitLocation & (HIT_CHEST | HIT_MID)))
                                        message = " got blasted in the back by";
                                else
                                {
                                        message = " was blown away by";
                                        message2 = "'s Handcannon";
                                }
                                break;
                        case MOD_DUAL_HC:
                                if ((HitLocation & HIT_HEAD) && (HitLocation & HIT_FRONT))
                                {
                                        message = "'s face was blown off by one of";
                                        message2 = "'s Handcannon";
                                }
                                else if ((HitLocation & HIT_BACK) && (HitLocation & (HIT_CHEST | HIT_MID)))
                                {
                                        message = " no longer has a spine thanks to";
                                        message2 = "'s Handcannons";
                                }
                                else
                                {
                                        if (random() < 0.5)
                                        {
                                                message = " was obliterated by";
                                                message2 = "'s twin Handcannon setup";
                                        }
                                        else
                                        {
                                                message = " was shown";
                                                message2 = "'s impersonation of Caleb";
                                        }
                                }
                                break;
			case MOD_MACHINEGUN:
                                if (HitLocation & HIT_HEAD)
                                {
                                        if (HitLocation & HIT_FRONT)
                                                message = " caught a round in the face from";
                                        else
                                                message = "'s head was blown apart by";
                                        message2 = "'s MP5";
                                }
                                else if ((HitLocation & HIT_BACK) && (HitLocation & (HIT_CHEST | HIT_MID)))
                                        message = " was machinegunned in the back by";
                                else if (HitLocation & HIT_MID)
                                        message = "'s lower intestine was shreaded by";
                                else
                                        message = " was machinegunned by";
				break;
                        case MOD_DUAL_MP5:
                                if (HitLocation & HIT_HEAD)
                                {
                                        if (HitLocation & HIT_FRONT)
                                                message = " caught a round in the face from one of";
                                        else
                                                message = "'s head was blown apart by one of";
                                        message2 = "'s MP5's";
                                }
                                else if ((HitLocation & HIT_BACK) && (HitLocation & (HIT_CHEST | HIT_MID)))
                                        message = " was machinegunned in the back by";
                                else if (HitLocation & HIT_MID)
                                        message = "'s lower intestine was shreaded by";
                                else if (random() < 0.5)
                                {
                                        message = " was shot up by by";
                                        message2 = "'s twin MP5 setup";
                                }
                                else
                                        message = " was machinegunned Neo style by";
				break;
			case MOD_CHAINGUN:
                                if (HitLocation & (HIT_RIGHT_LEG|HIT_LEFT_LEG))
                                {
                                        message = " was cut in half by";
                                        message2 = "'s Chaingun";
                                }
                                else
                                        message = " was Crucified by";
				break;
			case MOD_GRENADE:
                                message = " was blown apart by";
                                message2 = "'s grenade";
				break;
			case MOD_G_SPLASH:
                                message = " was shredded by";
                                message2 = "'s grenade shrapnel";
				break;
			case MOD_ROCKET:
                                message = " ate";
                                message2 = "'s rocket";
				break;
			case MOD_R_SPLASH:
                                message = " almost dodged";
				message2 = "'s rocket";
				break;
                        case MOD_RIFLE:
                                if (HitLocation & HIT_HEAD)
                                {
                                        message = "'s head was blown apart by";
                                        message2 = "'s M4";
                                }
                                else if ((HitLocation & (HIT_CHEST | HIT_MID)) && (HitLocation & HIT_BACK))
                                        message = " caught a NATO round in the back from";
                                else
                                {
                                        message = " caught a bullet from";
                                        message2 = "'s M4";
                                }
				break;
                        case MOD_DUAL_M4:
                                if (HitLocation & HIT_HEAD)
                                {
                                        message = "'s head was unfortunate enough to meet one of";
                                        message2 = "'s M4's";
                                }
                                else if ((HitLocation & (HIT_CHEST | HIT_MID)) && (HitLocation & HIT_BACK))
                                        message = " caught a NATO round in the back from";
                                else
                                {
                                        message = " learnt what OVERKILL means from one of";
                                        message2 = "'s M4's";
                                }
				break;
                        case MOD_SNIPER_RIFLE:
                                if (HitLocation & HIT_HEAD)
                                {
                                        if (HitLocation & HIT_THROAT)
                                                message = " took one to the neck from";
                                        else
                                                message = "'s head was blown off";
                                        message2 = "'s Sniper Rifle";
                                }
                                else if (HitLocation & (HIT_RIGHT_LEG|HIT_LEFT_LEG))
                                {
                                        message = "'s leg was blown off";
                                        message2 = "'s Sniper Rifle";
                                }
                                else if (HitLocation & HIT_MID)
                                        message = "'s intestines were sniped by";
                                else if (HitLocation & HIT_CHEST)
                                {
                                        if (self->client->pers.armor)
                                                message = "'s vest didn't help vs";
                                        else if (HitLocation & HIT_BACK)
                                                message = " took one in the back from";
                                        else
                                                message = " took one in the chest from";

                                        message2 = "'s Sniper Rifle";
                                }
                                else
                                        message = " was sniped by";
				break;
                        case MOD_KICKED:
                                message = " was kicked to death by";
				break;
                        case MOD_HIT:
                                message = " had the crap beaten out of them by";
				break;
			case MOD_HANDGRENADE:
                                message = " caught";
				message2 = "'s handgrenade";
				break;
			case MOD_HG_SPLASH:
                                message = " was shredded by";
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
                        // Dirty - NEW
                        case MOD_NECKSNAP:
                                message = " had their kneck snapped by";
				break;
                        case MOD_KNIFETOTHEBACK:
                                message = " was back-stabbed by";
				break;
                        // Dirty - NEW
                        case MOD_KNIFE:
                                if (HitLocation & HIT_HEAD)
                                {
                                        if (HitLocation & HIT_FRONT)
                                        {
                                                message = " had their face cut off by";
                                                message2 = "'s knife";
                                        }
                                        else if (HitLocation & HIT_BACK)
                                                message = " was scalped by";
                                        else
                                                message = " lost their ear Mr.Blonde style curtisy of";
                                }
                                else if (HitLocation & (HIT_RIGHT_LEG|HIT_LEFT_LEG))
                                        message = " legs were slashed up by";
                                else if (HitLocation & HIT_CHEST)
                                {
                                        if (HitLocation & HIT_FRONT)
                                                message = " became one with the knife god curtisy of";
                                        else if (HitLocation & HIT_BACK)
                                                message = " was slashed in the back by";
                                }
                                else if (HitLocation & HIT_MID)
                                {
                                        if (HitLocation & HIT_FRONT)
                                                message = " was gutted by";
                                        else if (HitLocation & HIT_BACK)
                                                message = " donated their kidneys to";
                                }
                                if (!message)
                                        message = " was cut up by";
				break;
                        case MOD_THROWN_KNIFE:
                                if (HitLocation & HIT_HEAD)
                                {
                                        if (HitLocation & HIT_THROAT)
                                                message = " took a knife in the throat from";
                                        else if (HitLocation & HIT_FRONT)
                                                message = " caught a flying knife in the temple from";
                                        else if (HitLocation & HIT_BACK)
                                        {
                                                message = " was hit in the back of the head by";
                                                message2 = "'s flying knife";
                                        }
                                        else
                                        {
                                                message = " was hit in the head by";
                                                message2 = "'s thrown knife";
                                        }
                                }
                                else if (HitLocation & (HIT_RIGHT_LEG|HIT_LEFT_LEG))
                                        message = " took a flying knife in a leg from";
                                else if (HitLocation & HIT_CHEST)
                                {
                                        if (HitLocation & HIT_FRONT)
                                                message = " became one with the knife god curtisy of";
                                        else if (HitLocation & HIT_BACK)
                                                message = " took a knife in the back from";
                                }
                                else if (HitLocation & HIT_MID)
                                {
                                        if (HitLocation & HIT_FRONT)
                                        {
                                                message = " caught";
                                                message2 = "'s knife with their lower intestine";
                                        }
                                        else if (HitLocation & HIT_BACK)
                                                message = " took a knife to the kidneys from";
                                }
                                if (!message)
                                {
                                        if (random() < 0.5)
                                                message = " caught a knife from";
                                        else
                                        {
                                                message = " was hit by";
                                                message2 = "'s flying knife";
                                        }
                                }
				break;
                }

                if (message)
                {
                        DeathMessage (self, attacker, message, message2);
                        if (dedicated->value)
                                gi.dprintf ("%s%s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);

                        if (ff)
                        {
                                if ((int)teamkill->value > 1)
                                        attacker->client->resp.team_kills++;
                                AdjustScore(attacker, -1, false);
                        }
                        else
                                AdjustScore(attacker, 1, false);
                        return;
                }
        }

        AdjustScore(self, -1, false);

        if ((int)realflags->value & RF_NO_DEATHMESSAGES)
                return;

        gi.bprintf (PRINT_MEDIUM, "%s", self->client->pers.netname);
        gi.bprintf (PRINT_MEDIUM," died.\n");
}
// Dirty

// GRIM
void DeathTossItems (edict_t *ent)
{
        weapons_t       *slot;
        gitem_t         *item = NULL;

        if ((!deathmatch->value) && (!coop->value))
		return;

        ent->client->weaponstate = WEAPON_READY;

        slot = ent->client->pers.largeweapon;
        if (slot->pickup_name)
        {
                item = FindItem(slot->pickup_name);
                if (item && item->drop)
                        item->drop (ent, item);
                SetSlot (ent, "Large Weapon Slot", NULL, 0, 0, NULL, 0, NULL, 0);
        }

        if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Back Pack"))])
                Drop_Pack (ent, FindItem("Back Pack"));

        ent->corpse = NULL;
        ent->client->weaponstate = WEAPON_READY; // restate
        if (ent->client->pers.weapon_handling && ent->client->pers.weapon2)
                Drop_CurrentWeapon (ent, 2);

        if (ent->client->pers.weapon)
                Drop_CurrentWeapon (ent, 1);
}

void PlayerDeathChecks (edict_t *ent)
{
        // disconnected/became observer only
        if (ent->gib_health == -1)
        {
                ent->client->resp.lives = 0;
                if (ent->client->resp.team)
                        ent->client->resp.team->members--;
        }

        if ((ent->movetype != MOVETYPE_NOCLIP) && (!ent->deadflag))
        {
                if (turns_on && (ent->client->resp.lives == 0) && MatchInProgress())
                {
                        if (max_teams && ent->client->resp.team)
                        {
                                ent->client->resp.team->living--;

                                if (ent->client->resp.team->living < 1)
                                        level.living--;
                        }
                        else
                                level.living--;
                }
                        
                if (!((int)bflags->value & BF_CHOOSE_STUFF))
                        CheckForSpecials(ent); // special items

                if (ent->gib_health != -100) // not gibbed
                        DeathTossItems (ent); // set in player_die
                else if ((int)dmflags->value & DF_WEAPONS_STAY)
                        CheckForWeapons(ent);
        }

        if (goals_exist)
                KillGoalCheck (ent);

        if (turns_on)
                MatchOverCheck ();
}
// GRIM

/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

	if (attacker && attacker != world && attacker != self)
		VectorSubtract (attacker->s.origin, self->s.origin, dir);
	else if (inflictor && inflictor != world && inflictor != self)
		VectorSubtract (inflictor->s.origin, self->s.origin, dir);
	else
        {
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	if (dir[0])
		self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
	else
        {
		self->client->killer_yaw = 0;
		if (dir[1] > 0)
			self->client->killer_yaw = 90;
		else if (dir[1] < 0)
			self->client->killer_yaw = -90;
	}

	if (self->client->killer_yaw < 0)
		self->client->killer_yaw += 360;
}

// GRIM
/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

        // remove all extra linked models (gun, flag etc)
        self->s.modelindex2 = 0;
        self->s.modelindex3 = 0;
        self->s.modelindex4 = 0;
        self->solid = SOLID_BBOX;

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;
        self->client->ps.gunindex = 0;

        // Dirty - make hard to gib
        // Check for gib
        if ((self->health < -100) && (TypeOfDamage & TOD_EXPLOSIVE)) //|| (self->health <= -999))
                self->gib_health = -100;

        // Shot-to-shit. Only for Dirty...
        if ((TypeOfDamage & TOD_LEAD) && attacker->client && self->deadflag)
        {
                if (self->client->respawn_time > (level.time + 1.5))
                {
                        self->deadflag = DEAD_DYING; // reset for STS call...
                        self->client->respawn_time = level.time + 1.9;
                }
        }
        // Dirty

        if (self->deadflag < DEAD_DYING)
	{
                // if 0 coz if lives = -1, we have unlimited lives.
                if (self->client->resp.lives > 0)
                        self->client->resp.lives--;

                ClientObituary (self, inflictor, attacker);

                if (ctf->value)
                        CTFFragBonuses(self, inflictor, attacker);

                PlayerDeathChecks (self); // generic function

                self->deadflag = DEAD_DYING;
                self->client->hanging = false;
                self->kills_in_a_row = 0;

                LookAtKiller (self, inflictor, attacker);

                self->client->respawn_time = level.time + 2.0;
                self->client->ps.pmove.pm_type = PM_DEAD;

                if (self->client->ps.fov != 90)
                        ZoomOff(self);
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;
	self->flags &= ~FL_POWER_ARMOR;
        self->client->adrenaline_framenum = 0;

        self->client->anim_priority = ANIM_DEATH;
        if (self->gib_health == -100) // gibbed, jibbed, jib or gib...???
        {
                if (self->s.modelindex == 255)
                {
                        gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
                        GibClient(self, (int)damage);
                        self->client->anim_end = 0;
                }
        } // normal death
        else if (self->deadflag < DEAD_DEAD)
        {
                static int i;

                i = (i+1)%3;

                // throat shot, no death sounds...
                if ((!(HitLocation & HIT_THROAT)) && (self->client->respawn_time == (level.time + 2.0)))
                        gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);

                // start a death animation
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
// GRIM
// Dirty
void InitClientPersistant (edict_t *ent)
{
        gitem_t         *item;
        weapons_t       *slot;
        gclient_t       *client;
        qboolean        add_pistol = true;
        qboolean        add_defaults = true;

        client = ent->client;

        memset (&client->pers, 0, sizeof(client->pers));
        memcpy (client->pers.weaponlist, weaponlist, sizeof(weaponlist));

        client->pers.grabbing = 0;

        client->pers.armor = NULL;
        client->pers.armor_index = 0;
        client->pers.carried_armor = NULL;
        client->pers.carried_armor_index = 0;

        // Clear wounds, set max health etc...
        memset(client->pers.wounds, 0, sizeof(client->pers.wounds));
        client->pers.health             = 100;
	client->pers.max_health		= 100;
        client->pers.heal_rate          = 3;
        client->pers.specials           = 0;

        client->pers.weapon = NULL;

        client->pers.gravity = (int)sv_jumpgrav->value;

        // Matrix style jumps...
        if (ent->client->resp.team && (ent->client->resp.team->jumpgrav > 0))
                client->pers.gravity = ent->client->resp.team->jumpgrav;

        if (client->pers.gravity > (int)sv_gravity->value)
                client->pers.gravity = (int)sv_gravity->value;

        if (deathmatch->value || coop->value)
        {
                // Dirty - This guy has killed to many ppl.. no items for them
                if (((int)teamkill->value > 1) && (client->resp.team_kills >= (int)teamkill->value))
                {
                        if (!client->pers.team_killer)
                                client->resp.offences += 1;

                        if (client->resp.offences >= 5)
                        {
                                stuffcmd (ent, "disconnect\n");
                                return;
                        }
                        else
                                client->pers.team_killer = true;
                }
                //gi.dprintf ("Offences %i, team killls %i\n", client->resp.offences, client->resp.team_kills);

                if (client->pers.team_killer)
                        client->pers.weapon = FindItem("Hands");
                // John Woo quick n' DIRTY mode - Just give 6 pistols
                else if ((int)bflags->value & BF_JOHN_WOO)
                {
                        // FIX ME - Give bandages n' knife?
                        //add_defaults = false;

                        item = FindItem("Pistol Clip");
                        client->pers.inventory[ITEM_INDEX(item)] = 0;
                        client->pers.clips[CLIP_INDEX(FindClip("Pistol Clip"))] = 0;

                        item = FindItem("MK23 Pistol");
                        client->pers.inventory[ITEM_INDEX(item)] = 5;
                        client->pers.weapon2 = item;

                        slot = FindSlot(ent, "Slot1");
                        slot->pickup_name = item->pickup_name,
                        slot->ammo = item->ammo;
                        slot->quantity = item->quantity;

                        slot = FindSlot(ent, "Slot2");
                        slot->pickup_name = item->pickup_name,
                        slot->ammo = item->ammo;
                        slot->quantity = item->quantity;

                        slot = FindSlot(ent, "Slot3");
                        slot->pickup_name = item->pickup_name,
                        slot->ammo = item->ammo;
                        slot->quantity = item->quantity;

                        slot = FindSlot(ent, "Slot4");
                        slot->pickup_name = item->pickup_name,
                        slot->ammo = item->ammo;
                        slot->quantity = item->quantity;

                } // If choose stuff, add stuff...
                else if ((int)bflags->value & BF_CHOOSE_STUFF)
                        ChosenEquipMent (ent);
        }

        if (client->pers.weapon == FindItem("Hands"))
        {
                add_pistol = false;
                add_defaults = false;
        }
        else if (client->pers.weapon == NULL)
                add_pistol = true;

        if (!((int)bflags->value & BF_CHOOSE_STUFF))
        {
                if (sv_weaponchoices->string && sv_weaponchoices->string[0] && (!strstr (sv_weaponchoices->string, "all")))
                {
                        if (add_pistol && (!strstr (sv_weaponchoices->string, "weapon_pistol")))
                        {
                                item = FindItem("Combat Knife");
                                client->pers.inventory[ITEM_INDEX(item)] = 13;
                                client->pers.lastweapon = item;
                                client->pers.weapon = item;
                                add_pistol = false;
                        }

                        if (strstr (sv_weaponchoices->string, "none"))
                        {
                                add_pistol = false;
                                add_defaults = false;
                        }
                        else if (!strstr (sv_weaponchoices->string, "weapon_knife"))
                                client->pers.inventory[ITEM_INDEX(FindItem("Combat Knife"))] = 0;
                }
        }

        if (add_defaults)
        {
                if (!client->pers.inventory[ITEM_INDEX(FindItem("Combat Knife"))])
                        client->pers.inventory[ITEM_INDEX(FindItem("Combat Knife"))] += 1;

                // starting bandages is now an option
                if ((!client->pers.inventory[ITEM_INDEX(FindItem("Bandages"))]) && ((int)bandages->value > 0))
                        client->pers.inventory[ITEM_INDEX(FindItem("Bandages"))] += (int)bandages->value;
        }
        
        if (add_pistol)
        {
                if (!((int)bflags->value & BF_JOHN_WOO))
                {
                        client->pers.inventory[ITEM_INDEX(FindItem("Pistol Clip"))] += 1;
                        client->pers.clips[CLIP_INDEX(FindClip("Pistol Clip"))] += 12;
                }

                item = FindItem("MK23 Pistol");
                client->pers.lastweapon = item;
                client->pers.inventory[ITEM_INDEX(item)] += 1;
                if (client->pers.weapon == NULL)
                        client->pers.weapon = item;
                else
                {
                        slot = FindBlank(ent, 1);
                        if (slot)
                        {
                                slot->pickup_name = item->pickup_name,
                                slot->ammo = item->ammo;
                                slot->quantity = item->quantity;
                        }
                        else    // Should this ever happen?
                                client->pers.inventory[ITEM_INDEX(item)] -= 1;
                }
        }

        // Set max_ammo based on carry device...
        if (client->pers.inventory[ITEM_INDEX(FindItem("Back Pack"))])
                client->pers.max_ammo = 90;
        else if (client->pers.inventory[ITEM_INDEX(FindItem("Bandolier"))])
                client->pers.max_ammo = 45;
        else
                client->pers.max_ammo = 18;

        client->pers.specials = 0;
        if (client->pers.inventory[ITEM_INDEX(FindItem("Stealth Slippers"))])
                client->pers.specials |= SI_STEALTH_SLIPPERS;

        if (client->pers.inventory[ITEM_INDEX(FindItem("Silencer"))])
                client->pers.specials |= SI_SILENCER;

        // Two weapons rock, I swear!
        if (client->pers.weapon2)
                client->pers.use_second = true;
        else
        {
                client->pers.weapon2 = NULL;
                client->pers.use_second = false;
        }

        client->pers.selected_item = -1;
        if (client->pers.weapon != FindItem("Hands"))
                client->pers.selected_item = ITEM_INDEX(client->pers.weapon);

	client->pers.connected = true;
}
// Dirty

void InitClientResp (gclient_t *client)
{
        qboolean        id_state = client->resp.id_state;
        int             team_kills = client->resp.team_kills; // Dirty
        int             offences = client->resp.offences;

	memset (&client->resp, 0, sizeof(client->resp));

	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;

        client->resp.id_state = id_state;
        client->resp.lives = -1;
        client->resp.team_kills = team_kills; // Dirty
        client->resp.offences = offences; // Dirty

        if (max_teams && forcejoin->value)
                ForceAssignTeam(client);
}
// GRIM

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
float PlayersRangeFromSpot (edict_t *spot)
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

                // GRIM - Ignore spectators etc
                if (player->movetype == MOVETYPE_NOCLIP)
			continue;
                // GRIM

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
        if ((int)(dmflags->value) & DF_SPAWN_FARTHEST)
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
void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;

        // GRIM
	if (deathmatch->value)
        {
                if (level.match_state == MATCH_CHECKING)
                        spot = SelectDeathmatchSpawnPoint();
                else if (max_teams && ent->client->resp.team)
                        spot = SelectTeamSpawnPoint(ent);
                else if ((ent == &g_edicts[1]) && turns_on)
                {
                        //gi.dprintf ("I am 1!\n");
                        spot = SelectRandomDeathmatchSpawnPoint();
                }
                else
                        spot = SelectDeathmatchSpawnPoint();
        }
        // GRIM
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
// GRIM
void TouchBodyInv (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        int     i;

        if (other->deadflag)
                return;

        if (!other->client)
                return;

        if (ent->is_a_lame_mofo) // Can't do anything
                return;

        // GTEMP - why duck?
        /*
        if (!(other->client->ps.pmove.pm_flags & PMF_DUCKED))
                return;
        */
        // GTEMP

        if (other->velocity[0] || other->velocity[1])
                return;

        if ((other->client->showinventory) || (other->client->pers.grabbing < 1))
                return;

        // This body is rigged...you're fucked :)
        if (ent->master && ent->master->trapping)
        {
                if (ent->master->trapping->die)
                {
                        if (ent->master->trapping->master)
                                ent->master->trapping->enemy = ent->master->trapping->master;
                        else
                                ent->master->trapping->enemy = ent->master->trapping;
                        ent->master->trapping->die (ent->master->trapping, ent->master->trapping, ent->master->trapping, 100000, vec3_origin);
                        ent->master->trapping = NULL;
                }
                return;
        }

        other->delay_time = level.time + 0.1;
        other->corpse = ent;
        VectorCopy(other->s.origin, other->corpse_pos);
        other->client->pers.grabbing = 2;  // On
        other->client->showinventory = true;

        gi.WriteByte (svc_inventory);
        for (i=0 ; i<MAX_ITEMS ; i++)
                gi.WriteShort (ent->inventory[i]);
        gi.unicast (other, true);
}

void CreateBodyInventory (edict_t *ent)
{
        edict_t         *body_inv;

        body_inv = G_Spawn();
        body_inv->classname = "body_inv";

        VectorCopy (ent->s.origin, body_inv->s.origin);

        body_inv->solid = SOLID_TRIGGER;
        body_inv->movetype = MOVETYPE_NONE;

        VectorSet (body_inv->mins, -15, -15, -15);
        VectorSet (body_inv->maxs, 15, 15, 15);

        body_inv->deadflag = DEAD_DEAD;
        body_inv->touch = TouchBodyInv;

        // Clear the body_inv, then copy the players inventory,
        // so that the body_inv can be searched.
        memset (&body_inv->inventory, 0, sizeof(body_inv->inventory));
        memset (&body_inv->clips, 0, sizeof(body_inv->clips));
        memset (&body_inv->weaponlist, 0, sizeof(body_inv->weaponlist));

        memcpy(body_inv->inventory, ent->client->pers.inventory, sizeof(ent->client->pers.inventory));
        memcpy(body_inv->clips, ent->client->pers.clips, sizeof(ent->client->pers.clips));
        memcpy(body_inv->weaponlist, ent->client->pers.weaponlist, sizeof(ent->client->pers.weaponlist));

        if (ent->client->pers.armor)
        {
                body_inv->armor = ent->client->pers.armor;
                body_inv->armor_index = ent->client->pers.armor_index;
        }

        if (ent->client->pers.carried_armor)
        {
                body_inv->carried_armor = ent->client->pers.carried_armor;
                body_inv->carried_armor_index = ent->client->pers.carried_armor_index;
        }
        gi.linkentity (body_inv);

        ent->master = body_inv;
}
// GRIM

void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;

	level.body_que = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
                ent->trapping = NULL; // GRIM
	}

        // GRIM - clear all limits
        instant_lead = 0;
        total_lead = 0;
        total_shit = 0;
        total_debris = 0;
        total_gibs = 0;
        knife_number = 0; // Dirty
        items_number = 0; // Dirty
        // GRIM
}

// GRIM
void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        // Dirty - hard to gib
        if ((self->health < -100) && (TypeOfDamage & TOD_EXPLOSIVE))// || (self->health <= -999))
        // Dirty - hard to gib
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);

                // Give right
                GibClient (self, (int)damage);

                self->svflags |= SVF_NOCLIENT;
		self->takedamage = DAMAGE_NO;
                self->solid = SOLID_NOT;

                // Traps go off...
                if (self->trapping && self->trapping->die)
                {
                        if (self->trapping->master)
                                self->trapping->enemy = self->trapping->master;
                        else
                                self->trapping->enemy = self->trapping;
                        self->trapping->die (self->trapping, self->trapping, self->trapping, 100000, vec3_origin);
                }
                self->trapping = NULL;

                // Check for body_inventory, and free it...
                if (self->corpse)
                {
                        G_FreeEdict (self->corpse);
                        self->corpse = NULL;
                }
	}
}

// Just in case a corpse wasn't on ground when an inventory was copied too it
void BodyAlignBodyInv (edict_t *ent)
{
        ent->nextthink = 0;

        if (!ent->corpse)
                return;

        if (!ent->groundentity)
        {
                ent->nextthink = level.time + 0.2;
                return;
        }

        VectorCopy (ent->s.origin, ent->corpse->s.origin);
        gi.linkentity (ent->corpse);
}

void CopyToBodyQue (edict_t *ent)
{
        edict_t *body;

        // We're gibbed, so put a head down and leave
        if (ent->s.modelindex != 255)
        {       
                if (ent->master)
                {
                        G_FreeEdict (ent->master);
                        ent->master = NULL;
                }
                ThrowHead (ent, "models/objects/gibs/skull/tris.md2", -1, GIB_ORGANIC);
                return;
        }

        // Gib body if in lava/slime
        if (ent->waterlevel && (ent->watertype & (CONTENTS_LAVA|CONTENTS_SLIME)))
        {
                gi.sound (ent, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
                GibClient (ent, 100);
                return;
        }

	// grab a body que and cycle to the next one
	body = &g_edicts[(int)maxclients->value + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;
        
	// FIXME: send an effect on the removed body

        // Check for body_inventory, and free it...
        if (body->corpse)
        {
                G_FreeEdict (body->corpse);
                body->corpse = NULL;
        }

	gi.unlinkentity (ent);

        // Remove and traps attached to us...
        if (body->trapping && (body->trapping->inuse))
                body->trapping->die (body->trapping, body->trapping, body->trapping, 100000, vec3_origin);

        body->trapping = NULL;

	gi.unlinkentity (body);

        body->s = ent->s;
        VectorCopy (ent->s.origin, body->s.origin);
        VectorCopy(ent->s.angles, body->s.angles);

        body->s.modelindex2 = 0;
        body->s.modelindex3 = 0;
        body->s.modelindex4 = 0;

	body->s.number = body - g_edicts;
        if (!(body->s.renderfx & RF_IR_VISIBLE))
                body->s.renderfx |= RF_IR_VISIBLE;

	body->svflags = ent->svflags;
        body->flags = ent->flags;

	VectorCopy (ent->absmin, body->absmin);
	VectorCopy (ent->absmax, body->absmax);
	VectorCopy (ent->size, body->size);

        body->movetype = MOVETYPE_TOSS;
        body->solid = SOLID_BBOX;
        body->clipmask = MASK_SHOT;

        // Consider non-solid.
        body->svflags = SVF_DEADMONSTER;
        body->flags |= FL_NO_KNOCKBACK;

        VectorSet (body->mins, -16, -24, -24);
        VectorSet (body->maxs, 16, 32, -8);

	body->takedamage = DAMAGE_YES;
        body->deadflag = DEAD_DEAD;
        body->health = ent->health;
	body->die = body_die;

        // If we have a body_inv, copy it
        if (ent->master)
        {
                body->corpse = ent->master;
                body->corpse->master = body;
                body->think = BodyAlignBodyInv;
                body->nextthink = level.time + 0.5;
                ent->master = NULL;
        }

        // If we have a trap on us, copy that too...
        if (ent->trapping)
        {
                body->trapping = ent->trapping;
                body->trapping->trapping = body;
                body->trapping->kills_in_a_row = ent->trapping->kills_in_a_row;
                ent->trapping = NULL;
        }

	gi.linkentity (body);
}
// GRIM

void respawn (edict_t *self)
{
	if (deathmatch->value || coop->value)
	{
                // spectator's don't leave bodies
                // GRIM
                if (self->movetype != MOVETYPE_NOCLIP)
			CopyToBodyQue (self);

                if (turns_on && (self->client->resp.state == CS_PLAYING))
                {
                        // Lives. Simple. How many times you can respawn.
                        if (self->client->resp.lives == 0)
                        {
                                edict_t *nme = NULL;

                                self->client->resp.state = CS_READY;
                                if (self->enemy && (self->enemy != self) && self->client)
                                        nme = self->enemy;

                                MakeObserver (self);

                                if (nme)
                                {
                                        self->client->chase_target = nme;
                                        self->client->update_chase = true;
                                        self->client->chase_mode = CHASE_ROTATE;
                                        UpdateChaseCam (self);
                                }
                                return;
                        }
                }
                // GRIM

		self->svflags &= ~SVF_NOCLIENT;
		PutClientInServer (self);

		// add a teleportation effect
                // Dirty - More is less
                //self->s.event = EV_PLAYER_TELEPORT;
                // Dirty

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
// GRIM - Changes here too...
void spectator_respawn (edict_t *ent)
{
	int i, numspec;

	// if the user wants to become a spectator, make sure he doesn't
	// exceed max_spectators

	if (ent->client->pers.spectator)
        {
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "spectator");
                if (*spectator_password->string && strcmp(spectator_password->string, "none")
                 && strcmp(spectator_password->string, value))
                {
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

		if (numspec >= maxspectators->value)
                {
			gi.cprintf(ent, PRINT_HIGH, "Server spectator limit is full.");
			ent->client->pers.spectator = false;
			// reset his spectator var
			gi.WriteByte (svc_stufftext);
			gi.WriteString ("spectator 0\n");
			gi.unicast(ent, true);
			return;
                }
        }
        else
        {
		// he was a spectator and wants to join the game
		// he must have the right password
		char *value = Info_ValueForKey (ent->client->pers.userinfo, "password");
		if (*password->string && strcmp(password->string, "none")
                 && strcmp(password->string, value))
                {
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

	if (ent->client->pers.spectator)
        {
                ent->gib_health = -1;
                PlayerDeathChecks (ent);
                if (ent->movetype != MOVETYPE_NOCLIP)
                {
                        ent->s.frame = FRAME_crdeath5;
                        ent->client->anim_end  = ent->s.frame;
                        if (!ent->master)
                                CreateBodyInventory (ent);
                        CopyToBodyQue (ent);
                }
                gi.bprintf (PRINT_HIGH, "%s has moved to the sidelines\n", ent->client->pers.netname);
                ent->client->resp.state = CS_NOT_READY;
                ent->client->respawn_time = level.time;
                ent->client->resp.spectator = true;
                MakeObserver(ent);
                return;
        }
        else
        {
                gi.bprintf (PRINT_HIGH, "%s is no longer just watching...\n", ent->client->pers.netname);
                ent->client->resp.spectator = false;
                //gi.bprintf (PRINT_HIGH, "%s joined the game\n", ent->client->pers.netname);
        }

	ent->svflags &= ~SVF_NOCLIENT;
        PutClientInServer (ent);

	// add a teleportation effect
        // Dirty - Less is more
        /*
	if (!ent->client->pers.spectator)
        {
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
	}
        */
        // Dirty

	ent->client->respawn_time = level.time;
}
// GRIM

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
	vec3_t	spawn_origin, spawn_angles;
	client_persistant_t	saved;
	client_respawn_t	resp;
	gclient_t	*client;
        int     index, i;

        // find a spawn point.
        // do it before setting health back up, so farthest
	// ranging doesn't count this client
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);

	index = ent-g_edicts-1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
                char userinfo[MAX_INFO_STRING];

		resp = client->resp;
                memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
                InitClientPersistant (ent);
                ClientUserinfoChanged (ent, userinfo);
	}
	else if (coop->value)
	{
                char userinfo[MAX_INFO_STRING];

		resp = client->resp;
                memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));

		resp.coop_respawn.game_helpchanged = client->pers.game_helpchanged;
		resp.coop_respawn.helpchanged = client->pers.helpchanged;
		client->pers = resp.coop_respawn;
                
                ClientUserinfoChanged (ent, userinfo);
		if (resp.score > client->pers.score)
                        client->pers.score = resp.score;
	}
	else
                memset (&resp, 0, sizeof(resp));

	// clear everything but the persistant data
	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;

        if (client->pers.health <= 0)
                InitClientPersistant(ent);

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

        // GRIM
        ent->client->respawn_time = level.time - 0.1;

        ent->touch = NULL;

        ent->combinetwo = NULL;
        ent->combfunc = NULL;
        ent->combine = NULL;
        ent->delay_time = 0;

        ent->client->concus_time = 0;
        ent->last_combat = 0;
        ent->last_damage = 0;
        ent->last_taken_time = 0;

        // IMPORTANT! - MUST BE SET CORRECTLY!!
        ent->last_taken = FindItem("MK23 Pistol"); // Dirty

        ent->enemy = ent;
        ent->oldenemy = ent;
        ent->blindTime = 0;

        ent->spotted_time = 0;
        ent->spotted = ent;
        ent->event_acted = false;
        ent->event_time = 0;
        ent->event = IDLE;
        ent->request_acted = false;
        ent->request_time = 0;
        ent->request = REQUEST_INFO;

        ent->goal = false;
        ent->req = false;
        ent->audio = false;
        ent->last_heard = 0;
        ent->heard = 10;

        ent->gib_health = -500;
        ent->kills_in_a_row = 0;
        ent->client->adrenaline_framenum = 0;

        if (ent->trace)
                G_FreeEdict (ent->trace);
        ent->trace = NULL;
        ent->blown_away = NULL;
        ent->corpse = NULL;
        ent->master = NULL;

        ent->count = 0; // if = hit_head, no death sounds...

        ent->action = 0;
        ent->client->hanging = false;
        ent->client->old_gunindex = 0;
        ent->client->pers.has_flag = NULL; // FIX ME - goal code?
        ent->client->ammo_index = 0;
        ent->client->ammo_index2 = 0;
        ent->client->pulled_pin = false;
        // GRIM

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

        // GRIM
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
        client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
        ent->client->ps.fov = 90;

        if (!client->pers.weapon)
                client->pers.weapon = FindItem("Hands");
        //client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);
        // GRIM

	// clear entity state values
	ent->s.effects = 0;
        ent->s.modelindex = 255;                // will use the skin specified model
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
// GRIM
	if (client->pers.spectator)
        {
		client->resp.spectator = true;
                MakeObserver (ent);
		return;
        }
        else
                client->resp.spectator = false;

        // Check to see if player should join in...
        if (!StartClient(ent))
                return;
        if (ent->pre_solid > 0)
                ent->svflags |= SVF_DEADMONSTER;
        else if ((!KillBox (ent)))
        {       // could't spawn in?
        }
// GRIM

	gi.linkentity (ent);

// GRIM
        // set weight, show item, armor etc
        //ShowItems(ent); // Dirty
        SetWeight(ent);
        SetArmorHUD(ent);
        SetSpeed(ent);

        // Dirty - set up lamer MOFO's
        ent->is_a_lame_mofo = false;
        if (client->pers.team_killer && (client->resp.team_kills > ((int)teamkill->value / 2)))
                ent->is_a_lame_mofo = true;
        else
                client->pers.team_killer = false;

        if (client->resp.team_kills > 0)
                client->resp.team_kills--;
        // Dirty

	// force the current weapon up
        client->pers.weapon_handling = NULL;
        client->pers.goodhand = FindSlot(ent, "Good Hand");
        client->pers.offhand = FindSlot(ent, "Off Hand");
        client->pers.largeweapon = FindSlot(ent, "Large Weapon Slot");

        client->pers.goodhand->pickup_name = client->pers.weapon->pickup_name;
        if (client->pers.weapon->ammo)
                client->pers.goodhand->ammo = client->pers.weapon->ammo;
        client->pers.goodhand->quantity = client->pers.weapon->quantity;
        if (client->pers.weapon->scomp)
        {
                gitem_t *item;
                item = FindItem (client->pers.weapon->scomp);
                client->pers.goodhand->s_ammo = item->ammo;
                client->pers.goodhand->s_quantity = item->quantity;
                client->pers.weapon2 = NULL;
        }

        if (client->pers.weapon2)
        {
                ChangeQuick (ent, client->pers.weapon, false);
                client->pers.offhand->pickup_name = client->pers.weapon2->pickup_name;
                client->pers.offhand->ammo = client->pers.weapon2->ammo;
                client->pers.offhand->quantity = client->pers.weapon2->quantity;
                ChangeQuick (ent, client->pers.weapon2, true);
        }
        else
                ChangeQuick (ent, client->pers.weapon, false);
// GRIM
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
        // Dirty - Less is more
        /*
	else
	{
		// send effect
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
        */
        // Dirty

        // GRIM - Not joined in yet
        gi.bprintf (PRINT_HIGH, "%s has entered the world\n", ent->client->pers.netname);
        // GRIM

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
                        // Dirty - Less is more
                        /*
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_LOGIN);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
                        */
                        // Dirty
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
        int     playernum;
        //char    n[17]; // GRIM
        
        //gi.dprintf ("ClientUserinfoChanged\n"); // DTEMP

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
                strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");

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

// GRIM
        ent->model = s; // Dirty - For extra vwep stuff (goal vwep)

        if (ent->client->resp.team)
                AssignSkin (ent, s);
        else
                gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

        // set up CS_PLAYERNAMES for ID function
        //strncpy (n, ent->client->pers.netname, sizeof(n)-1);
        //s = strtostr2 (n);
        //if (ent->client->resp.team)
        //        gi.configstring (CS_PLAYERNAMES+playernum, va("%s[%s]", name, ent->client->resp.team->name));
        //else
        //        gi.configstring (CS_PLAYERNAMES+playernum, va("%s", name));
        if (ent->client->resp.team)
                gi.configstring (CS_PLAYERNAMES+playernum, va("%s[%s]", ent->client->pers.netname, ent->client->resp.team->name));
        else
                gi.configstring (CS_PLAYERNAMES+playernum, va("%s", ent->client->pers.netname));
                            
	// fov
        ent->client->ps.fov = 90;
// GRIM

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

                if (*spectator_password->string && strcmp(spectator_password->string, "none")
                  && strcmp(spectator_password->string, value))
                {
			Info_SetValueForKey(userinfo, "rejmsg", "Spectator password required or incorrect.");
			return false;
		}

		// count spectators
		for (i = numspec = 0; i < maxclients->value; i++)
			if (g_edicts[i+1].inuse && g_edicts[i+1].client->pers.spectator)
				numspec++;

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
		if (*password->string && strcmp(password->string, "none")
                 && strcmp(password->string, value))
                {
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
                // GRIM
                ent->client->resp.team = NULL;
                ent->client->resp.state = CS_NOT_READY;
                ent->client->resp.id_state = true;
                ent->client->resp.lives = -1;
                // GRIM
                ent->client->resp.team_kills = 0; // Dirty
                ent->client->resp.offences = 0;

		// clear the respawning variables
                InitClientResp (ent->client);

                // GRIM
                ent->client->resp.weapon = NULL;
                ent->client->resp.item = NULL;
                ent->client->resp.armor = NULL;

                // Stop calling InitClientPersistant all the time...
                // weapon will be set in PutClientInServer if it doesn't exist
                //if (!game.autosaved || !ent->client->pers.weapon)
                //        InitClientPersistant (ent->client);
                memset (&ent->client->pers, 0, sizeof(ent->client->pers));
                ent->client->pers.connected = true;
                // GRIM
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

        // Dirty
        if (ent->client->resp.offences >= 5)
                gi.bprintf (PRINT_HIGH, "%s was disconnected for repeated TK'ing\n", ent->client->pers.netname);
        else
                gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);
        // Dirty

        // GRIM
        ent->gib_health = -1;
        PlayerDeathChecks (ent);
        if (ent->movetype != MOVETYPE_NOCLIP)
        {
                ent->s.frame = FRAME_crdeath5;
                ent->client->anim_end  = ent->s.frame;
                if (!ent->master)
                        CreateBodyInventory (ent);
                CopyToBodyQue (ent);
        }
        // GRIM

	// send effect
        // Dirty - Less is more
        /*
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
        */
        // Dirty

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

// Dirty
void KickGunAway (edict_t *ent, edict_t *other)
{
        vec3_t  forward, right, start, end, offset;
        int     damage;
        trace_t tr;

        if (ent->is_a_lame_mofo) // Can't kick as a lame mofo
                return;

        if (other->deadflag || (other->mindmg > 20))
                return;

        if (ent->attack_time > level.time)
                return;

        if (ent->groundentity)
                return;

        if (other->client)
        {
                if (OnSameTeam(other, ent))
                        return;
        }

        ent->attack_time = level.time + 0.4;

        AngleVectors (ent->s.angles, forward, right, NULL);
        VectorSet(offset, 0, 0,  ent->viewheight-20);
        P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
        VectorMA (start, 25, forward, end);
        
        tr = gi.trace (start, NULL, NULL, end, ent, MASK_SHOT);

        if (tr.ent == other)
        {
                damage = 14 + (rand() %8);
                
                T_Damage (other, ent, ent, forward, tr.endpos, vec3_origin, damage, 400, HIT_CHEST, TOD_NO_WOUND|TOD_NO_ARMOR, MOD_KICKED);
                gi.sound (ent, CHAN_GOODHAND, gi.soundindex ("weapons/kick1.wav"), 1, ATTN_NORM, 0);

                if (other->client)
                {
                        if ((other->client->weaponstate == WEAPON_FIRING)
                         || (other->client->weaponstate == WEAPON_FIRING2))
                                return;
                        
                        other->client->weaponstate = WEAPON_READY;
                        if (other->client->pers.weapon_handling)
                                SwitchGuns(other);
                        else
                                Drop_CurrentWeapon (other, 1);
                }
        }
}

qboolean JumpBack (edict_t *ent)
{
        vec3_t  forward, end, right;
        trace_t tr;

        if (ent->client->flip_time > level.time)
                return false;

        if (ent->client->pers.gravity < (int)sv_gravity->value)
                ent->client->flip_time = level.time + 0.2;
        else if (ent->velocity[2] < -125)
                return false; // no stopping falls =P
        else
                ent->client->flip_time = level.time + 0.6;

        AngleVectors (ent->s.angles, forward, right, NULL);
        VectorMA (ent->s.origin, 48, forward, end);
        tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);

        if ((tr.fraction < 1.0) && (tr.surface && (!(tr.surface->flags & SURF_SKY)))) 
        {
                forward[2] = 0;
                VectorScale (forward, -400, ent->velocity);
                ent->velocity[2] = 320;
                ent->action |= A_BACKFLIP;
                ent->s.angles[PITCH] = 0;
                return true;
        }

        VectorMA (ent->s.origin, -48, forward, end);
        tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);

        if ((tr.fraction < 1.0) && (tr.surface && (!(tr.surface->flags & SURF_SKY)))) 
        {
                forward[2] = 0;
                VectorScale (forward, 400, ent->velocity);
                ent->velocity[2] = 320;
                ent->action |= A_SOMERSAULT;
                ent->s.angles[PITCH] = 0;
                return true;
        }

        VectorMA (ent->s.origin, 48, right, end);
        tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);

        if ((tr.fraction < 1.0) && (tr.surface && (!(tr.surface->flags & SURF_SKY)))) 
        {
                forward[2] = 0;
                VectorScale (right, -400, ent->velocity);
                ent->velocity[2] = 320;
                ent->action |= A_LEFTFLIP;
                ent->s.angles[ROLL] = 0;
                return true;
        }

        VectorMA (ent->s.origin, -48, right, end);
        tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);

        if ((tr.fraction < 1.0) && (tr.surface && (!(tr.surface->flags & SURF_SKY)))) 
        {
                forward[2] = 0;
                VectorScale (right, 400, ent->velocity);
                ent->velocity[2] = 320;
                ent->action |= A_RIGHTFLIP;
                ent->s.angles[ROLL] = 0;
                return true;
        }

        ent->client->flip_time = 0;
        return false;
}
// Dirty


// GRIM
void UCMDMenu (edict_t *ent, usercmd_t *ucmd)
{
        if (!ent->client->menu)
                return;

        ent->client->ps.pmove.pm_type = PM_FREEZE;

        if (level.time - ent->menu_time < 0.1)
                return;

        if ((ucmd->forwardmove < 0) || (ucmd->sidemove > 0))
        {
                if (!(ent->client->ps.pmove.pm_flags & PMF_JUMP_HELD))
                {
                        ent->client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
                        PMenu_Next(ent);
                }
        }
        else if ((ucmd->forwardmove > 0) || (ucmd->sidemove < 0))
        {
                if (!(ent->client->ps.pmove.pm_flags & PMF_JUMP_HELD))
                {
                        ent->client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
                        PMenu_Prev(ent);
                }
        }
        else
                ent->client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;

        ucmd->forwardmove = 0;
        ucmd->sidemove = 0;
}
// GRIM


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
        edict_t         *other;
        int             i, j;
        pmove_t         pm;

	level.current_entity = ent;
	client = ent->client;

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
                if (level.time > level.intermissiontime + 5.0 && (ucmd->buttons & BUTTON_ANY))
			level.exitintermission = true;
		return;
	}

	pm_passent = ent;

	if (ent->client->chase_target)
        {
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

                // GRIM - From Hentai's old version of the chasecam...
                if (ucmd->forwardmove > 0)
		{
			ent->speed--;
                        if (ent->speed < 0)
                                ent->speed = 0;
		}
                else if (ucmd->forwardmove < 0)
		{
			ent->speed++;
                        if (ent->speed > 196)
                                ent->speed = 196;
		}
                // GRIM
        }
        else
        {
		// set up for pmove
		memset (&pm, 0, sizeof(pm));

		if (ent->movetype == MOVETYPE_NOCLIP)
                {
			client->ps.pmove.pm_type = PM_SPECTATOR;
                        if (client->menu)
                                UCMDMenu (ent, ucmd);
                }
                else if (ent->s.modelindex != 255)
                {
			client->ps.pmove.pm_type = PM_GIB;
                        if (client->menu)
                                UCMDMenu (ent, ucmd);
                }
                else if (ent->deadflag)
                {
			client->ps.pmove.pm_type = PM_DEAD;
                        if (client->menu)
                                UCMDMenu (ent, ucmd);
                }
		else
                {
			client->ps.pmove.pm_type = PM_NORMAL;
                        // DTEMP - Problems here?
                        /*
                        i = (440 - ent->weight);
                        // weapon causing slow down affect
                        if (client->weapon_forward_set && (client->weapon_forward_set < i))
                                i = client->weapon_forward_set;
                        j = (i * -1);
                        if (ucmd->forwardmove > (short)i)
                                ucmd->forwardmove = (short)i;
                        else if (ucmd->forwardmove < (short)j)
                                ucmd->forwardmove = (short)j;

                        i = (440 - ent->weight);
                        // weapon causing slow down affect
                        if (client->weapon_side_set && (client->weapon_side_set < i))
                                i = client->weapon_side_set;

                        j = (i * -1);
                        if (ucmd->sidemove > (short)i)
                                ucmd->sidemove = (short)i;
                        else if (ucmd->sidemove < (short)j)
                                ucmd->sidemove = (short)j;

                        // bad leg wounds = no/less jump ?? FIX ME
                        if ((ucmd->upmove >= 10) && (LegWounds(ent) > 6)) // red legs
                                ucmd->upmove = 10;
                        */
                        // DTEMP

                        // Dirty - cancel BIG jump
                        if ((ent->action & A_JUMPED) && (ucmd->upmove < 0))
                                ent->action &= ~A_JUMPED;
                        // Dirty
                }
                // GRIM
                
                // Dirty
                if (ent->action & A_JUMPED)
                        client->ps.pmove.gravity = client->pers.gravity;
                else
                        client->ps.pmove.gravity = sv_gravity->value;
                // Dirty

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
                // Dirty
                if (ent->client->hanging)
                {
                        i = pm.s.origin[2];
                        j = pm.s.velocity[2];
                }
                // Dirty

                gi.Pmove (&pm);

                // Dirty
                if (ent->client->hanging)
                {
                        if (pm.s.origin[2] < i)
                                pm.s.origin[2] = i;
                        if (pm.s.velocity[2] < j)
                                pm.s.velocity[2] = j;
                }
                // Dirty

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

                // Dirty
                if (client->ps.pmove.pm_flags & PMF_DUCKED)
                        ent->maxs[2] += 7;
                // Dirty

		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

                // Dirty
                // so we don't bother checking other stuff
                // check for noclip first...
                if ((ent->movetype == MOVETYPE_NOCLIP) || ent->deadflag)
                        pm.cmd.upmove = pm.cmd.upmove;
                else if (ent->client->hanging)
                {
                        if (ent->groundentity || pm.groundentity)
                                ent->client->hanging = false;
                        else if ((pm.cmd.upmove > 0) || (pm.cmd.forwardmove > 0))
                        {
                                if ((ent->s.origin[2] - 36) < ent->client->hang_point[2])
                                {
                                        ent->velocity[2] += 20;
                                        if (ent->velocity[2] > 200)
                                                ent->velocity[2] = 200;
                                }
                                else
                                        ent->client->hanging = false;
                        }
                        else if (pm.cmd.upmove < 0)
                                ent->client->hanging = false;
                        else
                                ent->velocity[2] = 0;

                        if (pm.cmd.sidemove == 0)
                        {
                                ent->velocity[0] = 0;
                                ent->velocity[1] = 0;
                        }

                        if (ent->client->hanging)
                                HangingCheck (ent);
                }
                else if (!pm.groundentity)
                {
                        if (ent->groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
                        {
                                if (ent->delay_time > level.time)
                                        Cmd_WeapLast_f (ent);

                                // Dirty
                                // Paril: junk has nothing to do with movement
                                /*if ((int)junk->value > 8)
                                {
                                        j = 100 - ent->weight;
                                        
                                        if ((ent->velocity[2] + i) < 32)
                                                ent->velocity[2] = 32;
                                        else
                                                ent->velocity[2] += i;

                                }*/
                                // Dirty
                                ent->action |= A_JUMPED; // Dirty
                        }
                        else if (!ent->groundentity)
                        {
                                if (ent->client->pers.grabbing == 1)
                                {
                                        if ((ent->client->weaponstate == WEAPON_READY) && (Grab_n_Climb (ent)))
                                                ent->client->hanging = true;
                                }
                                else if (ucmd->upmove < 0)
                                {
                                        if (!(ent->action & A_WANT_FLIP))
                                        {
                                                ent->action |= A_WANT_FLIP;

                                                if (JumpBack (ent))
                                                {
                                                        ent->action |= A_JUMPED; // Dirty
                                                        gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
                                                }
                                        }
                                        else if (!(ent->action & (A_SOMERSAULT | A_BACKFLIP | A_RIGHTFLIP | A_LEFTFLIP)))
                                        {
                                                if (ucmd->forwardmove > 0)
                                                        ent->action |= A_SOMERSAULT;
                                                else if (ucmd->forwardmove < 0)
                                                        ent->action |= A_BACKFLIP;
                                                else if (ucmd->sidemove > 0)
                                                        ent->action |= A_RIGHTFLIP;
                                                else if (ucmd->sidemove < 0)
                                                        ent->action |= A_LEFTFLIP;
                                        }
                                }
                                else
                                      ent->action &= ~A_WANT_FLIP;
                        }
                }
                // Dirty

                // Dirty
                ent->viewheight = pm.viewheight;
                if (pm.viewheight < 0)
                        ent->viewheight = 1;
                //else
                //        ent->viewheight = 25;
                // Dirty

		ent->waterlevel = pm.waterlevel;
		ent->watertype = pm.watertype;

		ent->groundentity = pm.groundentity;

                // Dirty
                if (pm.groundentity)
                {
                        // try to make sure body inv is on ground
                        if (ent->deadflag && ent->master)
                        {
                                VectorCopy (ent->s.origin, ent->master->s.origin);
                                gi.linkentity (ent->master);
                        }
                                
                        ent->groundentity_linkcount = pm.groundentity->linkcount;
                        // clear somersaults etc
                        ent->action &= ~(A_SOMERSAULT|A_BACKFLIP|A_RIGHTFLIP|A_LEFTFLIP|A_JUMPED);
                }
                // Dirty

		if (ent->deadflag)
		{
                        // GRIM
                        if (client->ps.viewangles[ROLL] != 0)
                                client->ps.viewangles[ROLL] = client->ps.viewangles[ROLL];
                        else if (random() < 0.5)
                                client->ps.viewangles[ROLL] = 40;
                        else
                                client->ps.viewangles[ROLL] = -40;
                        // GRIM
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
                                continue; // duplicated

                        // Dirty
                        if (other->takedamage && (!pm.groundentity) && (pm.waterlevel == 0))
                                KickGunAway(ent, other);
                        // Dirty

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

        // GRIM
        if (ent->movetype == MOVETYPE_NOCLIP)
        {
                if (client->latched_buttons & BUTTON_ATTACK)
                {
                        client->latched_buttons &= ~BUTTON_ATTACK;
                        if (client->menu)
                        {
                                if (level.time - ent->menu_time > 0.1)
                                        PMenu_Select(ent);
                        }
                        else if (client->chase_target)
                        {
                                if (client->chase_mode == CHASE_ROTATE)
                                        client->chase_mode = CHASE_POV;
                                else
                                        NoLongerChase (ent);
                        }
                        else if ((client->resp.state == CS_NOT_READY) && (!client->resp.spectator))
                                OpenInitialMenu (ent, NULL);
                        else
                        {
                                client->chase_mode = CHASE_ROTATE;
                                GetChaseTarget (ent);
                        }
		}

                if (client->latched_buttons & BUTTON_USE)
                {                
                        client->latched_buttons &= ~BUTTON_USE;
                        if (client->menu)
                        {
                                if (level.time - ent->menu_time > 0.1)
                                        PMenu_Close(ent);
                        }
                        else if ((client->resp.state == CS_NOT_READY) && (!client->resp.spectator))
                                OpenInitialMenu (ent, NULL);
                        else
                                ChangeMenu(ent);
		}

		if (ucmd->upmove >= 10)
                {
                        if ((!(client->ps.pmove.pm_flags & PMF_JUMP_HELD)) && (!client->menu))
                        {
				client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
                                if (client->chase_target)
                                        ChaseNext (ent);
				else
                                {
                                        client->chase_mode = CHASE_ROTATE;
                                        GetChaseTarget (ent);
                                }
			}
		}
                else if (!client->menu)
			client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
        } // fire weapon from final position if needed
        else if ((client->latched_buttons & BUTTON_ATTACK) || (client->latched_buttons & BUTTON_USE))
        {
                if (!client->weapon_thunk)
                {
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}
        // GRIM


        // GRIM - can we re-do the chase cam to simply
                // use one generic function or something?
	// update chase cam if being followed
	for (i = 1; i <= maxclients->value; i++)
        {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
                        UpdateChaseCam (other);
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

	if (level.intermissiontime)
		return;

	client = ent->client;

        if (deathmatch->value && ((level.time - client->respawn_time) >= 5)
         && (client->pers.spectator != client->resp.spectator))
        {
		spectator_respawn(ent);
		return;
	}

	// run weapon animations if it hasn't been done by a ucmd_t
        // GRIM 
        if (!client->weapon_thunk && ent->movetype != MOVETYPE_NOCLIP)
		Think_Weapon (ent);
	else
		client->weapon_thunk = false;

        if (ent->deadflag)
	{
		// wait for any button just going down
                if (level.time > client->respawn_time)
		{
                        if (turns_on && (ent->client->resp.lives == 0) && (ent->client->resp.state == CS_PLAYING))
                                respawn(ent);
                        else if (deathmatch->value && ((int)dmflags->value & DF_FORCE_RESPAWN))
                                respawn(ent);
                        else if (client->latched_buttons & BUTTON_ATTACK)
                        {
                                if (client->menu)
                                {
                                        if (level.time - ent->menu_time > 0.1)
                                                PMenu_Select(ent);
                                }
                                else
                                        respawn(ent);
                                client->latched_buttons &= ~BUTTON_ATTACK;
			}
                        else if (client->latched_buttons & BUTTON_USE)
                        {
                                client->latched_buttons &= ~BUTTON_USE;
                                if (client->menu)
                                {
                                        if (level.time - ent->menu_time > 0.1)
                                                PMenu_Close(ent);
                                }
                                else
                                        ChangeMenu(ent);
                        }
		}
		return;
	}
        // GRIM

	// add player trail so monsters can follow
        /* No monster code
	if (!deathmatch->value)
                if (!visible (ent, PlayerTrail_LastSpot() ))
			PlayerTrail_Add (ent->s.old_origin);
        */
        // GRIM 

	client->latched_buttons = 0;
}
