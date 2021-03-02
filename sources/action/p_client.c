#include "g_local.h"
#include "m_player.h"
#include "cgf_sfx_glass.h"

void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void ClientDisconnect(edict_t *ent);
void SP_misc_teleporter_dest (edict_t *ent);
void CopyToBodyQue (edict_t *ent);

void Add_Frag( edict_t *ent )
{
        ent->client->resp.kills++;
        ent->client->resp.streak++;
        if ( teamplay->value || (ent->client->resp.streak < 4) )
        {
                ent->client->resp.score++; // just 1 normal kill
        }
        else if ( ent->client->resp.streak < 8 )
        {
                gi.bprintf (PRINT_MEDIUM,"%s has %d kills in a row and receives %d frags for the kill!\n",
                        ent->client->pers.netname, ent->client->resp.streak, 2);
                ent->client->resp.score += 2;
        }
        else if ( ent->client->resp.streak < 16 )
        {
                gi.bprintf (PRINT_MEDIUM,"%s has %d kills in a row and receives %d frags for the kill!\n",
                        ent->client->pers.netname, ent->client->resp.streak, 4);
                ent->client->resp.score += 4;
        }
        else if ( ent->client->resp.streak < 32 )
        {
                gi.bprintf (PRINT_MEDIUM,"%s has %d kills in a row and receives %d frags for the kill!\n",
                        ent->client->pers.netname, ent->client->resp.streak, 8);
                ent->client->resp.score += 8;
        }
        else 
        {
                gi.bprintf (PRINT_MEDIUM,"%s has %d kills in a row and receives %d frags for the kill!\n",
                        ent->client->pers.netname, ent->client->resp.streak, 16);
                ent->client->resp.score += 16;
        }

//FIREBLADE
        if (!teamplay->value && ent->client->resp.streak >= 1)
//FIREBLADE
        {
                gi.cprintf(ent, PRINT_HIGH, "Kill count: %d\n", ent->client->resp.streak );
        }
}

void Subtract_Frag( edict_t *ent )
{
        ent->client->resp.score--;
        ent->client->resp.streak = 0;
}


// FRIENDLY FIRE functions
extern qboolean Ban_TeamKiller ( edict_t *ent, int rounds ); // g_svcmds.c
extern void Kick_Client ( edict_t *ent); // g_svcmds.c

void Add_TeamWound( edict_t *attacker, edict_t *victim, int mod)
{
	if (!teamplay->value || !attacker->client || !victim->client)
	{
		return;
	}

	attacker->client->team_wounds++;

	// Warn both parties that they are teammates. Since shotguns are pellet based,
	// make sure we don't overflow the client when using MOD_HC or MOD_SHOTGUN. The
	// ff_warning flag should have been reset before each attack.
	if (attacker->client->ff_warning == 0)
	{
		attacker->client->ff_warning++;
		gi.cprintf(victim, PRINT_HIGH, "You were hit by %s, your TEAMMATE!\n", 
				attacker->client->pers.netname);
		gi.cprintf(attacker, PRINT_HIGH, "You hit your TEAMMATE %s!\n", 
				victim->client->pers.netname);
	}

	// We want team_wounds to increment by one for each ATTACK, not after each 
	// bullet or pellet does damage. With the HAND CANNON this means 2 attacks
	// since it is double barreled and we don't want to go into p_weapon.c...
	attacker->client->team_wounds = (attacker->client->team_wounds_before + 1);
  
	// If count is less than MAX_TEAMKILLS*3, return. If count is greater than
	// MAX_TEAMKILLS*3 but less than MAX_TEAMKILLS*4, print off a ban warning. If
	// count equal (or greater than) MAX_TEAMKILLS*4, ban and kick the client.
	if ((int)maxteamkills->value < 1)  //FB
		return;  
	if (attacker->client->team_wounds < ((int)maxteamkills->value * 3))
	{
		return;
	}
	else if (attacker->client->team_wounds < ((int)maxteamkills->value * 4))
	{		
		// Print a note to console, and issue a warning to the player.
		gi.cprintf(NULL, PRINT_MEDIUM, 
			"%s is in danger of being banned for wounding teammates\n",
				attacker->client->pers.netname);
		gi.cprintf(attacker, PRINT_HIGH, 
			"WARNING: You'll be temporarily banned if you continue wounding teammates!\n");
		return;
	}
	else
	{
		if (attacker->client->ipaddr)
		{
			if (Ban_TeamKiller(attacker, (int)twbanrounds->value))
			{
				gi.cprintf(NULL, PRINT_MEDIUM, 
							"Banning %s@%s for team wounding\n", 
								attacker->client->pers.netname, attacker->client->ipaddr);
	
				gi.cprintf(attacker, PRINT_HIGH, 
						"You've wounded teammates too many times, and are banned for %d %s.\n", 
							(int)twbanrounds->value, (((int)twbanrounds->value > 1) ? "games" : "game"));
			}
			else
			{
				gi.cprintf(NULL, PRINT_MEDIUM, 
							"Error banning %s: unable to get ipaddr\n", 
								attacker->client->pers.netname);
			}
			Kick_Client(attacker);
		}
	}
	
	return;
}

void Add_TeamKill( edict_t *attacker )
{
	if (!teamplay->value || !attacker->client)
	{
		return;
	}

	attacker->client->team_kills++;
	// Because the stricter team kill was incremented, lower team_wounds
	// by amount inflicted in last attack (i.e., no double penalty).
	if (attacker->client->team_wounds > attacker->client->team_wounds_before)
	{
		attacker->client->team_wounds = attacker->client->team_wounds_before;
	}

	// If count is less than 1/2 MAX_TEAMKILLS, print off simple warning. If
	// count is greater than 1/2 MAX_TEAMKILLS but less than MAX_TEAMKILLS,
	// print off a ban warning. If count equal or greater than MAX_TEAMKILLS,
	// ban and kick the client.
	if (((int)maxteamkills->value < 1) ||
		(attacker->client->team_kills < (((int)maxteamkills->value % 2) + (int)maxteamkills->value / 2)))
	{
		gi.cprintf(attacker, PRINT_HIGH, "You killed your TEAMMATE!\n");
		return;
	}
	else if (attacker->client->team_kills < (int)maxteamkills->value)
	{
		// Show this on the console
		gi.cprintf(NULL, PRINT_MEDIUM, "%s is in danger of being banned for killing teammates\n", attacker->client->pers.netname);
		// Issue a warning to the player
		gi.cprintf(attacker, PRINT_HIGH, "WARNING: You'll be temporarily banned if you continue killing teammates!\n" );
		return;
	}
	else
	{
		// They've killed too many teammates this game - kick 'em for a while
		if (attacker->client->ipaddr)
		{
			if (Ban_TeamKiller(attacker, (int)tkbanrounds->value))
			{
				gi.cprintf (NULL, PRINT_MEDIUM, 
					"Banning %s@%s for team killing\n", 
					attacker->client->pers.netname, 
					attacker->client->ipaddr);
				gi.cprintf(attacker, PRINT_HIGH, "You've killed too many teammates, and are banned for %d %s.\n", (int)tkbanrounds->value,
					(((int)tkbanrounds->value > 1) ? "games" : "game"));
			}
			else
			{
				gi.cprintf(NULL, PRINT_MEDIUM, 
							"Error banning %s: unable to get ipaddr\n", 
								attacker->client->pers.netname);
			}
		}
		Kick_Client(attacker);
	}
}
// FRIENDLY FIRE


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
        edict_t *spot;
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
                        if ((!self->targetname) || stricmp(self->targetname, spot->targetname) != 0)
                        {
//                              gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
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
        edict_t *spot;

        if(stricmp(level.mapname, "security") == 0)
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
        if(stricmp(level.mapname, "security") == 0)
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

        if((stricmp(level.mapname, "jail2") == 0)   ||
           (stricmp(level.mapname, "jail4") == 0)   ||
           (stricmp(level.mapname, "mine1") == 0)   ||
           (stricmp(level.mapname, "mine2") == 0)   ||
           (stricmp(level.mapname, "mine3") == 0)   ||
           (stricmp(level.mapname, "mine4") == 0)   ||
           (stricmp(level.mapname, "lab") == 0)     ||
           (stricmp(level.mapname, "boss1") == 0)   ||
           (stricmp(level.mapname, "fact3") == 0)   ||
           (stricmp(level.mapname, "biggun") == 0)  ||
           (stricmp(level.mapname, "space") == 0)   ||
           (stricmp(level.mapname, "command") == 0) ||
           (stricmp(level.mapname, "power2") == 0) ||
           (stricmp(level.mapname, "strike") == 0))
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
        char            *info;

        if (!ent->client)
                return false;

                // "gender" below used to be "skin", 3.20 change -FB
        info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
        if (info[0] == 'f' || info[0] == 'F')
                        return true;
        return false;
}

// FROM 3.20  -FB
qboolean IsNeutral (edict_t *ent)
{
        char            *info;
        
        if (!ent->client)
                return false;
        
        info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
        if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
                return true;
        return false;
}
// ^^^

// PrintDeathMessage: moved the actual printing of the death messages to here, to handle
//  the fact that live players shouldn't receive them in teamplay.  -FB
void PrintDeathMessage(char *msg, edict_t *gibee)
{
        int j;
        edict_t *other;
        
        if (!teamplay->value)
        {
                gi.bprintf(PRINT_MEDIUM, msg);
                return;
        }
        
        if (dedicated->value)
                gi.cprintf(NULL, PRINT_MEDIUM, "%s", msg);
        
        for (j = 1; j <= game.maxclients; j++)
        {
                other = &g_edicts[j];
                if (!other->inuse || !other->client)
                        continue;
                if (gibee != other && team_round_going && other->solid != SOLID_NOT)
                        continue;
                gi.cprintf(other, PRINT_MEDIUM, "%s", msg);
        }
}

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
        int       mod;
        int       loc;
        char      *message;
        char      *message2;
        char      death_msg[2048];  // enough in all situations? -FB
        qboolean  ff;
        int       special = 0;
        int       n; 
        
        if (coop->value && attacker->client)
                meansOfDeath |= MOD_FRIENDLY_FIRE;
        
	if (attacker && attacker != self && attacker->client && OnSameTeam (self, attacker))
		meansOfDeath |= MOD_FRIENDLY_FIRE;

        if (deathmatch->value || coop->value)
        {
                ff = meansOfDeath & MOD_FRIENDLY_FIRE;
                mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
                loc = locOfDeath; // useful for location based hits
                message = NULL;
                message2 = "";
                
                switch (mod)
                {
		case MOD_BREAKINGGLASS:
			message = "ate too much glass";
			break;
                case MOD_SUICIDE:
                        message = "suicides";
                        break;
                case MOD_FALLING:
                        // moved falling to the end
                        if (self->client->push_timeout)
                                special = 1;
                        //message = "hit the ground hard, real hard";
                        if (IsNeutral(self))
                                message = "plummets to its death";
                        else if (IsFemale(self))
                                message = "plummets to her death";
                        else
                                message = "plummets to his death";
                        break;          
                case MOD_CRUSH:
                        message = "was flattened";
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
                                if (IsNeutral(self))
                                        message = "didn't throw its grenade far enough";                      
                                if (IsFemale(self))
                                        message = "didn't throw her grenade far enough";
                                else
                                        message = "didn't throw his grenade far enough";
                                break;
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
                if (message && !special)
                {
                        sprintf(death_msg, "%s %s\n", self->client->pers.netname, message);
                        PrintDeathMessage(death_msg, self);
                        if (deathmatch->value)
                                Subtract_Frag( self );//self->client->resp.score--;
                        self->enemy = NULL;
                        return;
                }
                else if ( special ) // handle falling with an attacker set
                {
                        if (self->client->attacker &&
			    self->client->attacker->client &&
                            (self->client->attacker->client != 
			     self->client))
                        {
                                sprintf(death_msg, "%s was taught how to fly by %s\n", 
                                        self->client->pers.netname, self->client->attacker->client->pers.netname );
                                PrintDeathMessage(death_msg, self);

//MODIFIED FOR FF -FB
                                if (!((int)dmflags->value & DF_NO_FRIENDLY_FIRE) &&
					OnSameTeam(self, self->client->attacker) &&
					teamplay->value)
				{
					if (!teamplay->value || team_round_going)
					{
						Add_TeamKill(self->client->attacker);
                                                Subtract_Frag( self->client->attacker );//attacker->client->resp.score--;
					}
				}
                                else
                                {
                                	if (!teamplay->value || mod != MOD_TELEFRAG)
                                        	Add_Frag(self->client->attacker );//attacker->client->resp.score++;
                                }
//END FF ADD
                        }
                        else
                        {
                                if (IsNeutral(self))
                                        sprintf(death_msg, "%s plummets to its death\n", self->client->pers.netname);
                                else if (IsFemale(self))
                                        sprintf(death_msg, "%s plummets to her death\n", self->client->pers.netname);
                                else
                                        sprintf(death_msg, "%s plummets to his death\n", self->client->pers.netname);
                                PrintDeathMessage(death_msg, self);
        	                if (deathmatch->value)
                	                Subtract_Frag( self );//self->client->resp.score--;
	                        self->enemy = NULL;
                        }
                        return;
                }
                
#if 0
                // handle bleeding, not used because bleeding doesn't get set
                if ( mod == MOD_BLEEDING )              
                {
                        sprintf(death_msg, "%s bleeds to death\n", self->client->pers.netname);
                        PrintDeathMessage(death_msg, self);
                        return; 
                }
#endif

                self->enemy = attacker;
                if (attacker && attacker->client)
                {
                        switch (mod)
                        {
                        case MOD_MK23:  // zucc
                                switch (loc)
                                {
                                case LOC_HDAM:
                                        if (IsNeutral(self))
                                                message = " has a hole in its head from";
                                        else if (IsFemale(self))
                                                message = " has a hole in her head from";
                                        else
                                                message = " has a hole in his head from";
                                        message2 = "'s Mark 23 pistol";
                                        break;
                                case LOC_CDAM:
                                        message = " loses a vital chest organ thanks to";
                                        message2 = "'s Mark 23 pistol";
                                        break;
                                case LOC_SDAM:
                                        if (IsNeutral(self))
                                                message = " loses its lunch to";
                                        else if (IsFemale(self))
                                                message = " loses her lunch to";
                                        else
                                                message = " loses his lunch to";
                                        message2 = "'s .45 caliber pistol round";
                                        break;
                                case LOC_LDAM:
                                        message = " is legless because of";
                                        message2 = "'s .45 caliber pistol round";
                                        break;
                                default:
                                        message = " was shot by";
                                        message2 = "'s Mark 23 Pistol";
                                }
                                break;
                        case MOD_MP5:
                                switch (loc)
                                {
                                case LOC_HDAM:
                                        message = "'s brains are on the wall thanks to";
                                        message2 = "'s 10mm MP5/10 round";
                                        break;
                                case LOC_CDAM:
                                        message = " feels some chest pain via";
                                        message2 = "'s MP5/10 Submachinegun";
                                        break;
                                case LOC_SDAM:
                                        message = " needs some Pepto Bismol after";
                                        message2 = "'s 10mm MP5 round";
                                        break;
                                case LOC_LDAM:
                                        if (IsNeutral(self))
                                        	message = " had its legs blown off thanks to";
                                        else if (IsFemale(self))
                                                message = " had her legs blown off thanks to";
                                        else
                                                message = " had his legs blown off thanks to";
                                        message2 = "'s MP5/10 Submachinegun";
                                        break;
                                default:
                                        message = " was shot by";
                                        message2 = "'s MP5/10 Submachinegun";
                                }
                                break;
                        case MOD_M4:
                        	switch (loc)
                                {
                                	case LOC_HDAM:
                                                message = " had a makeover by";
                                                message2 = "'s M4 Assault Rifle";
                                                break;
                                        case LOC_CDAM:
                                                message = " feels some heart burn thanks to";
                                                message2 = "'s M4 Assault Rifle";
                                                break;
                                        case LOC_SDAM:
                                                message = " has an upset stomach thanks to";
                                                message2 = "'s M4 Assault Rifle";
                                                break;
                                        case LOC_LDAM:
                                                message = " is now shorter thanks to";
                                                message2 = "'s M4 Assault Rifle";
                                                break;
                                        default:
                                        	message = " was shot by";
                                                message2 = "'s M4 Assault Rifle";
                                }
                                break;
                        case MOD_M3:
                                n = rand() % 2 + 1;
                                if (n == 1)
                                {
					message = " accepts"; 
					message2 = "'s M3 Super 90 Assault Shotgun in hole-y matrimony";
				}
                                	else
                                {
					message = " is full of buckshot from"; 
					message2 = "'s M3 Super 90 Assault Shotgun";
				}
                                break;        
                        case MOD_HC:
                                n = rand() % 2 + 1;
                                if (n == 1)
                                {
					message = " ate"; 
					message2 = "'s sawed-off 12 gauge";
				}
                                	else
                                {
					message = " is full of buckshot from"; 
					message2 = "'s sawed off shotgun";
				}
                                break;    
                        case MOD_SNIPER:
                                switch (loc)
                                {
                                case LOC_HDAM:
                                	if (self->client->ps.fov < 90)
                                        {
                                        	if (IsNeutral(self))
                                                	message = " saw the sniper bullet go through its scope thanks to";
                                                else if (IsFemale(self))
                                                	message = " saw the sniper bullet go through her scope thanks to";
                                                else 
                                                	message = " saw the sniper bullet go through his scope thanks to";
                                        }
                                        else
					{
                                        	message = " caught a sniper bullet between the eyes from";
					}
                                        break;
                                case LOC_CDAM:
                                       message = " was picked off by";
                                       break;
                                case LOC_SDAM:
                                       message = " was sniped in the stomach by";
                                       break;
                                case LOC_LDAM:
                                       message = " was shot in the legs by";
                                       break;
                                default:
                                       message = "was sniped by";
                                       //message2 = "'s Sniper Rifle";
                                }
                                break;
                        case MOD_DUAL:
                                switch (loc)
                                {
                                	case LOC_HDAM:
                                        	message = " was trepanned by";
                                                message2 = "'s akimbo Mark 23 pistols";
                                                break;
                                        case LOC_CDAM:
                                        	message = " was John Woo'd by";
                                                //message2 = "'s .45 caliber pistol round";
                                                break;
                                        case LOC_SDAM:
                                        	message = " needs some new kidneys thanks to";
                                                message2 = "'s akimbo Mark 23 pistols";
                                                break;
                                        case LOC_LDAM:
                                        	message = " was shot in the legs by";
                                                message2 = "'s akimbo Mark 23 pistols";
                                                break;
	                                default:
        					message = " was shot by";
                                                message2 = "'s pair of Mark 23 Pistols";
                                }
                                break;
                        case MOD_KNIFE:
				switch (loc)
                                {
					case LOC_HDAM:
                                        	if (IsNeutral(self))
                                                	message = " had its throat slit by";                                                                  
                                                else if (IsFemale(self))
                                                	message = " had her throat slit by";                                                                  
                                                else 
                                                	message = " had his throat slit by";                                                                  
                                       		break;
                                        case LOC_CDAM:
                                        	message = " had open heart surgery, compliments of";                                                                    
                                        	break;
                                        case LOC_SDAM:
                                        	message = " was gutted by";                                                                     
                                        	break;
					case LOC_LDAM:
                                        	message = " was stabbed repeatedly in the legs by";                                                                     
                                        	break;
					default:
                                        	message = " was slashed apart by";
                                        	message2 = "'s Combat Knife";
				}
				break;
                        case MOD_KNIFE_THROWN:
                        	switch (loc)
                                {
                                	case LOC_HDAM:                                                                  
                                        	message = " caught";
                                                if (IsNeutral(self))
                                                	message2 = "'s flying knife with its forehead";
                                                else if (IsFemale(self))
                                                        message2 = "'s flying knife with her forehead";
                                                else
                                                        message2 = "'s flying knife with his forehead";
                                                break;
                                        case LOC_CDAM:
                                                message = "'s ribs don't help against";
                                                message2 = "'s flying knife";
                                                break;
                                        case LOC_SDAM:
                                                if (IsNeutral(self))
                                                	message = " sees the contents of its own stomach thanks to";
                                                else if (IsFemale(self))
                                                        message = " sees the contents of her own stomach thanks to";
                                                else
                                                        message = " sees the contents of his own stomach thanks to";
                                                message2 = "'s flying knife";
                                                break;
                                        case LOC_LDAM:
                                                if (IsNeutral(self))
                                                	message = " had its legs cut off thanks to";
                                                else if (IsFemale(self))
                                                	message = " had her legs cut off thanks to"; 
                                                else
                                                	message = " had his legs cut off thanks to";                                                                                                                                                                                    
                                                message2 = "'s flying knife";
                                                break;
                                        default:
                                                message = " was hit by";
                                                message2 = "'s flying Combat Knife";
				}
				break;
                        case MOD_GAS:
                        	message = "sucks down some toxic gas thanks to";
                                break;
			case MOD_KICK:
                        	n = rand() % 3 + 1;
                                if (n == 1)
                                {
					if (IsNeutral(self))
                                        	message = " got its ass kicked by";
                                        else if (IsFemale(self))
                                        	message = " got her ass kicked by";
                                        else
                                        	message = " got his ass kicked by";
                                }
                                else if (n == 2)
                                {
                                        if (IsNeutral(self))
                                        {
                                                message = " couldn't remove"; message2 = "'s boot from its ass";
                                        }
                                        else if (IsFemale(self))
                                        {
                                                message = " couldn't remove"; message2 = "'s boot from her ass";
                                        }
                                        else
                                        {
                                        	message = " couldn't remove"; message2 = "'s boot from his ass";
                                        }
                                }
                                else
                                {
                                        if (IsNeutral(self))
                                        {
                                        	message = " had a Bruce Lee put on it by"; message2 = ", with a quickness";
                                        }
                                        else if (IsFemale(self))
                                        {
                                                message = " had a Bruce Lee put on her by"; message2 = ", with a quickness";
                                        }
                                        else
                                        {
                                        	message = " had a Bruce Lee put on him by"; message2 = ", with a quickness";
                                	}
                                }
				break;
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
                                message = " caught";
                                message2 = "'s handgrenade";
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
                        }

                        if (message)
                        {
//FIREBLADE
                                sprintf(death_msg, "%s%s %s%s\n", self->client->pers.netname, message, 
							attacker->client->pers.netname, message2);
                                PrintDeathMessage(death_msg, self);     
//FIREBLADE
				if (deathmatch->value)
                                {
                                	if (ff)
					{
						if (!teamplay->value || team_round_going)
						{
							Add_TeamKill(attacker);
                                                        Subtract_Frag( attacker );//attacker->client->resp.score--;
						}
					}
                                        else
                                        {
//FIREBLADE
                                                if (!teamplay->value || mod != MOD_TELEFRAG)
//FIREBLADE
                                                	Add_Frag(attacker );//attacker->client->resp.score++;
                                        }
                                }
                                return;
                        }
                 }
        }
                
//FIREBLADE
        sprintf(death_msg, "%s died\n", self->client->pers.netname);
        PrintDeathMessage(death_msg, self);
//FIREBLADE
        if (deathmatch->value)
                        Subtract_Frag( self );//self->client->resp.score--;
}

void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

// zucc used to toss an item on death
void EjectItem( edict_t *ent, gitem_t *item )
{
        edict_t         *drop;
        float spread;
        if (item)
        {
                spread = 300.0*crandom();
                ent->client->v_angle[YAW] -= spread;
                drop = Drop_Item (ent, item);
                ent->client->v_angle[YAW] += spread;
                drop->spawnflags = DROPPED_PLAYER_ITEM;
        }
        
        
}



// unique weapons need to be specially treated so they respawn properly
void EjectWeapon( edict_t *ent, gitem_t *item )
{
        edict_t         *drop;
        float spread;
        if (item)
        {
                spread = 300.0*crandom();
                ent->client->v_angle[YAW] -= spread;
                drop = Drop_Item (ent, item);
                ent->client->v_angle[YAW] += spread;
                drop->spawnflags = DROPPED_PLAYER_ITEM;
                drop->think = temp_think_specweap;
        }


}



//zucc toss items on death
void TossItemsOnDeath( edict_t *ent )
{
        gitem_t         *item;
        
        
        // don't bother dropping stuff when allweapons/items is active
        if (allitem->value && allweapon->value)
        {
        	// remove the lasersight because then the observer might have it
                item = FindItem(LASER_NAME);
                ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
                return;
        }
                
                
                // don't drop weapons if allweapons is on
                if (allweapon->value)
                  {
                    DeadDropSpec(ent);
                    return;
                  }
                
                // only drop items if allitems is not on
                if(!allitem->value)
                  DeadDropSpec(ent);
                else
                  { // remove the lasersight because then the observer might have it
                    item = FindItem(LASER_NAME);
                    ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
                    SP_LaserSight(ent, item);
                  }
                // give the player a dual pistol so they can be sure to drop one
                item = FindItem(DUAL_NAME);
                ent->client->pers.inventory[ITEM_INDEX(item)]++;
                EjectItem( ent, item );
                
                // check for every item we want to drop when a player dies
                item = FindItem(MP5_NAME);
                while ( ent->client->pers.inventory[ITEM_INDEX(item)] > 0 )
        {
                ent->client->pers.inventory[ITEM_INDEX(item)]--;
                EjectWeapon( ent, item );
        }
        item = FindItem(M4_NAME);
        while ( ent->client->pers.inventory[ITEM_INDEX(item)] > 0 )
        {
                ent->client->pers.inventory[ITEM_INDEX(item)]--;
                EjectWeapon( ent, item );
        }
        item = FindItem(M3_NAME);
        while ( ent->client->pers.inventory[ITEM_INDEX(item)] > 0 )
        {
                ent->client->pers.inventory[ITEM_INDEX(item)]--;
                EjectWeapon( ent, item );
        }
        item = FindItem(HC_NAME);
        while ( ent->client->pers.inventory[ITEM_INDEX(item)] > 0 )
        {
                ent->client->pers.inventory[ITEM_INDEX(item)]--;
                EjectWeapon( ent, item );
        }
        item = FindItem(SNIPER_NAME);
        while ( ent->client->pers.inventory[ITEM_INDEX(item)] > 0 )
        {
                ent->client->pers.inventory[ITEM_INDEX(item)]--;
                EjectWeapon( ent, item );
        }
        item = FindItem(KNIFE_NAME);
        if ( ent->client->pers.inventory[ITEM_INDEX(item)] > 0 )
        {
                EjectItem( ent, item );
        }
// special items

#if 0
        item = FindItem(SIL_NAME);
        if ( ent->client->pers.inventory[ITEM_INDEX(item)] )
                EjectItem( ent, item );
        item = FindItem(SLIP_NAME);
        if ( ent->client->pers.inventory[ITEM_INDEX(item)] )
                EjectItem( ent, item );
        item = FindItem(BAND_NAME);
        if ( ent->client->pers.inventory[ITEM_INDEX(item)] )
                EjectItem( ent, item );
        item = FindItem(KEV_NAME);
        if ( ent->client->pers.inventory[ITEM_INDEX(item)] )
                EjectItem( ent, item );
        item = FindItem(LASER_NAME);
        if ( ent->client->pers.inventory[ITEM_INDEX(item)] )
                EjectItem( ent, item );
#endif
}


void TossClientWeapon (edict_t *self)
{
        gitem_t         *item;
        edict_t         *drop;
        qboolean        quad;
        float           spread;

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
        vec3_t          dir;

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

// NEW FORMULA FOR THIS FROM 3.20  -FB
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
// ^^^
}

/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//      int             n;

        VectorClear (self->avelocity);
                
        self->takedamage = DAMAGE_YES;
        self->movetype = MOVETYPE_TOSS;
        
//FIREBLADE
        if (self->solid == SOLID_TRIGGER)
        {
                self->solid = SOLID_BBOX;
                gi.linkentity(self);
                RemoveFromTransparentList(self);
        }
//FIREBLADE

        // zucc solves problem of people stopping doors while in their dead bodies
        // 
        // ...only need it in DM though...
        // ...for teamplay, non-solid will get set soon after in CopyToBodyQue
        if (!teamplay->value)
        {
                self->solid = SOLID_NOT;
                gi.linkentity(self);
        }

        self->s.modelindex2 = 0;        // remove linked weapon model

        self->s.angles[0] = 0;
        self->s.angles[2] = 0;

        self->s.sound = 0;
        self->client->weapon_sound = 0;

                self->client->reload_attempts = 0; // stop them from trying to reload
                self->client->weapon_attempts = 0;

        self->maxs[2] = -8;

        self->svflags |= SVF_DEADMONSTER;

        if (!self->deadflag)
        {
                self->client->respawn_time = level.time + 1.0;
                LookAtKiller (self, inflictor, attacker);
                self->client->ps.pmove.pm_type = PM_DEAD;
                ClientObituary (self, inflictor, attacker);
                //TossClientWeapon (self);
                TossItemsOnDeath(self);
//FIREBLADE
                if (deathmatch->value && !teamplay->value)
//FIREBLADE
                        Cmd_Help_f (self);              // show scores
        }

        // remove powerups
        self->client->quad_framenum = 0;
        self->client->invincible_framenum = 0;
        self->client->breather_framenum = 0;
        self->client->enviro_framenum = 0;

        //zucc remove lasersight
        if (self->lasersight)
                SP_LaserSight(self, NULL);

//FIREBLADE
        // clean up sniper rifle stuff
        self->client->no_sniper_display = 0;
        self->client->resp.sniper_mode = SNIPER_1X;
        self->client->desired_fov = 90;
        self->client->ps.fov = 90;
//FIREBLADE
        
        self->client->resp.streak = 0;
        Bandage(self); // clear up the leg damage when dead sound?
                self->client->bandage_stopped = 0;

        // clear inventory
        memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));

        // zucc - check if they have a primed grenade
        if ( self->client->curr_weap == GRENADE_NUM &&
                        (
                                                ( self->client->ps.gunframe >= GRENADE_IDLE_FIRST
                        && self->client->ps.gunframe <= GRENADE_IDLE_LAST )
                                                || ( self->client->ps.gunframe >= GRENADE_THROW_FIRST
                                                && self->client->ps.gunframe <= GRENADE_THROW_LAST )
                                                ) )
        {
                self->client->ps.gunframe = 0;
                fire_grenade2 (self, self->s.origin, tv(0,0,0), GRENADE_DAMRAD, 0, 2, GRENADE_DAMRAD*2, false);
        }

        //zucc no gibbing
/*      if (self->health < -40)
        {       // gib
                gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
                for (n= 0; n < 4; n++)
                        ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
                ThrowClientHead (self, damage);

                self->takedamage = DAMAGE_NO;
        }*/
//      else
        {       // normal death
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
                                                if ((meansOfDeath == MOD_SNIPER) || (meansOfDeath == MOD_KNIFE) || (meansOfDeath == MOD_KNIFE_THROWN))
                                                        gi.sound(self, CHAN_VOICE, gi.soundindex("misc/glurp.wav"), 1, ATTN_NORM, 0);                
                                                else
                                                        gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
                }
        }
                // zucc this will fix a jump kick death generating a weapon
                self->client->curr_weap = MK23_NUM;
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
        gitem_t         *item;
/*
	client_persistant_t	oldpers;

//FB 6/3/99
	memcpy(oldpers, pers, sizeof(client->pers));
//FB 6/3/99
*/
        memset (&client->pers, 0, sizeof(client->pers));
        // changed to mk23
        item = FindItem(MK23_NAME);
        client->pers.selected_item = ITEM_INDEX(item);
        client->pers.inventory[client->pers.selected_item] = 1;
        
        client->pers.weapon = item;

        item = FindItem(KNIFE_NAME);
        client->pers.inventory[ITEM_INDEX(item)] = 1;

        client->pers.health                     = 100;
        client->pers.max_health         = 100;
        //zucc changed maximum ammo amounts
        client->pers.max_bullets        = 2;
        client->pers.max_shells         = 14;
        client->pers.max_rockets        = 2;
        client->pers.max_grenades       = 50;
        client->pers.max_cells          = 1;
        client->pers.max_slugs          = 20;
        client->knife_max                       = 10;   
        client->grenade_max                     = 2;

        client->pers.connected = true;
        //zucc
        client->fired = 0;
        client->burst = 0;
        client->fast_reload = 0;
        client->machinegun_shots = 0;
        client->unique_weapon_total = 0;
        client->unique_item_total = 0;
        client->curr_weap = MK23_NUM;
}


void InitClientResp (gclient_t *client)
{
        memset (&client->resp, 0, sizeof(client->resp));
        client->resp.enterframe = level.framenum;
        client->resp.coop_respawn = client->pers;
        client->resp.weapon = FindItem(MP5_NAME);
        client->resp.item = FindItem(KEV_NAME);
        client->resp.ir = 1;
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
        int             i;
        edict_t *ent;

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
float   PlayersRangeFromSpot (edict_t *spot)
{
        edict_t *player;
        float   bestplayerdistance;
        vec3_t  v;
        int             n;
        float   playerdistance;


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
        edict_t *spot, *spot1, *spot2;
        int             count = 0;
        int             selection;
        float   range, range1, range2;

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
        edict_t *bestspot;
        float   bestdistance, bestplayerdistance;
        edict_t *spot;


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
        int             index;
        edict_t *spot = NULL;
        char    *target;

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
                        return NULL;    // we didn't have enough...

                target = spot->targetname;
                if (!target)
                        target = "";
                if ( Q_stricmp(game.spawnpoint, target) == 0 )
                {       // this is a coop spawn point for one of the clients here
                        index--;
                        if (!index)
                                return spot;            // this is it
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
void    SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
        edict_t *spot = NULL;

//FIREBLADE     
        if (teamplay->value && ent->client->resp.team != NOTEAM)
        {
                spot = SelectTeamplaySpawnPoint(ent);
        }
                else
        {
//FIREBLADE
                if (deathmatch->value)
                        spot = SelectDeathmatchSpawnPoint ();
                else if (coop->value)
                        spot = SelectCoopSpawnPoint (ent);
        }

        // find a single player start spot
        if (!spot)
        {
//FIREBLADE
                if (deathmatch->value)
                {
                        gi.dprintf("Warning: failed to find deathmatch spawn point\n");
                }
//FIREBLADE
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
                        {       // there wasn't a spawnpoint without a target, so use any
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
        int             i;
        edict_t *ent;

        level.body_que = 0;
        for (i=0; i<BODY_QUEUE_SIZE ; i++)
        {
                ent = G_Spawn();
                ent->classname = "bodyque";
        }
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
/*      int     n;*/

        if (self->health < -40)
        {
                        // remove gibbing
/*                gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
                for (n= 0; n < 4; n++)
                        ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
                self->s.origin[2] -= 48;
                ThrowClientHead (self, damage);*/
                self->takedamage = DAMAGE_NO;
        }
}

void CopyToBodyQue (edict_t *ent)
{
        edict_t         *body;

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
        // All our bodies will be non-solid -FB
        body->solid = SOLID_NOT;  
        //body->solid = ent->solid;
        body->clipmask = ent->clipmask;
        body->owner = ent->owner;
//FB 5/31/99
        body->movetype = MOVETYPE_TOSS;  // just in case?
//        body->movetype = ent->movetype;
        VectorCopy (ent->velocity, body->velocity);
        body->mass = ent->mass;
        body->groundentity = NULL;
//FB 5/31/99
//FB 6/1/99
        body->s.renderfx = 0;
//FB

        body->die = body_die;
        body->takedamage = DAMAGE_YES;

        gi.linkentity (body);
}

void CleanBodies()
{
        edict_t *ptr;
        int i;
        ptr = g_edicts + game.maxclients + 1;
        i = 0;
        while (i < BODY_QUEUE_SIZE)
        {
                gi.unlinkentity(ptr);
                ptr->solid = SOLID_NOT;
                ptr->movetype = MOVETYPE_NOCLIP;
                ptr->svflags |= SVF_NOCLIENT;
                ptr++;
                i++;
        }
}

void respawn (edict_t *self)
{
        if (deathmatch->value || coop->value)
        {
//FIREBLADE
                if (self->solid != SOLID_NOT || self->deadflag == DEAD_DEAD)
//FIREBLADE
                        CopyToBodyQue (self);
                PutClientInServer (self);

//FIREBLADE
                self->svflags &= ~SVF_NOCLIENT;
//FIREBLADE

// Disable all this... -FB
//                // add a teleportation effect
//                self->s.event = EV_PLAYER_TELEPORT;
//
//                // hold in place briefly
//                self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
//                self->client->ps.pmove.pm_time = 14;

                self->client->respawn_time = level.time;

                return;
        }

        // restart the entire server
        gi.AddCommandString ("menu_loadgame\n");
}

//==============================================================

void AllWeapons( edict_t *ent )
{

        int i;
        gitem_t *it;

        for (i=0 ; i<game.num_items ; i++)
        {
                it = itemlist + i;
                if (!it->pickup)
                        continue;
                if (!(it->flags & IT_WEAPON))
                        continue;
                ent->client->pers.inventory[i] = 1;
        }

        for (i=0 ; i<game.num_items ; i++)
        {
                it = itemlist + i;
                if (!it->pickup)
                        continue;
                if (!(it->flags & IT_AMMO))
                        continue;
                Add_Ammo (ent, it, 1000);
        }

        ent->client->mk23_rds = ent->client->mk23_max;
        ent->client->dual_rds = ent->client->dual_max;
        ent->client->mp5_rds = ent->client->mp5_max;
        ent->client->m4_rds = ent->client->m4_max;
        ent->client->shot_rds = ent->client->shot_max;
        ent->client->sniper_rds = ent->client->sniper_max;
        ent->client->cannon_rds = ent->client->cannon_max;

        if (tgren->value <= 0) // team grenades is turned off
        {
                it = FindItem(GRENADE_NAME);
                ent->client->pers.inventory[ITEM_INDEX(it)] = 0;
        }
        // give them a reasonable number of knives
        it = FindItem(KNIFE_NAME);
        ent->client->pers.inventory[ITEM_INDEX(it)] = 10;

}

void AllItems( edict_t *ent )
{
        edict_t etemp;
        int i;
        gitem_t *it;
        
        

        for (i=0 ; i<game.num_items ; i++)
        {
                it = itemlist + i;
                if (!it->pickup)
                        continue;
                if (!(it->flags & IT_ITEM))
                        continue;
                
                etemp.item = it;
        
                if ( ent->client->unique_item_total >= unique_items->value ) 
                        ent->client->unique_item_total = unique_items->value - 1;
                Pickup_Special ( &etemp, ent );
        }

}



// equips a client with item/weapon in teamplay


void EquipClient( edict_t *ent )
{
        gclient_t *client;
        gitem_t* item;
        edict_t etemp;
        int band = 0;

        client = ent->client;
        
        
        if ( !(client->resp.item) || !(client->resp.weapon) )
                return;
        

        if ( stricmp(client->resp.item->pickup_name, BAND_NAME) == 0 )
        {
                band = 1;
                if (tgren->value > 0) // team grenades is turned on
                {
                        item = FindItem(GRENADE_NAME);
                        client->pers.inventory[ITEM_INDEX(item)] = tgren->value;
                }
                
        }

        // set them up with initial pistol ammo
        item = FindItem("Pistol Clip");
        if ( band )
                client->pers.inventory[ITEM_INDEX(item)] = 2;
        else
                client->pers.inventory[ITEM_INDEX(item)] = 1;
        

        if ( stricmp(client->resp.weapon->pickup_name, MP5_NAME ) == 0 )
        {
                item = FindItem(MP5_NAME);
                client->pers.selected_item = ITEM_INDEX(item);
                client->pers.inventory[client->pers.selected_item] = 1;
                client->pers.weapon = item;
                client->curr_weap = MP5_NUM;
                client->unique_weapon_total = 1;
                item = FindItem("Machinegun Magazine");
                if ( band )
                        client->pers.inventory[ITEM_INDEX(item)] = 2;
                else
                        client->pers.inventory[ITEM_INDEX(item)] = 1;
                client->mp5_rds = client->mp5_max;
        }
        else if ( stricmp(client->resp.weapon->pickup_name, M4_NAME ) == 0 )
        {
                item = FindItem(M4_NAME);
                client->pers.selected_item = ITEM_INDEX(item);
                client->pers.inventory[client->pers.selected_item] = 1;
                client->pers.weapon = item;
                client->curr_weap = M4_NUM;
                client->unique_weapon_total = 1;
                item = FindItem("M4 Clip");
                if ( band )
                        client->pers.inventory[ITEM_INDEX(item)] = 2;
                else
                        client->pers.inventory[ITEM_INDEX(item)] = 1;
                client->m4_rds = client->m4_max;
        }
        else if ( stricmp(client->resp.weapon->pickup_name, M3_NAME ) == 0 )
        {
                item = FindItem(M3_NAME);
                client->pers.selected_item = ITEM_INDEX(item);
                client->pers.inventory[client->pers.selected_item] = 1;
                client->pers.weapon = item;
                client->curr_weap = M3_NUM;
                client->unique_weapon_total = 1;
                item = FindItem("12 Gauge Shells");
                if ( band )
                        client->pers.inventory[ITEM_INDEX(item)] = 14;
                else
                        client->pers.inventory[ITEM_INDEX(item)] = 7;
                client->shot_rds = client->shot_max;
        }
        else if ( stricmp(client->resp.weapon->pickup_name, HC_NAME ) == 0 )
        {
                item = FindItem(HC_NAME);
                client->pers.selected_item = ITEM_INDEX(item);
                client->pers.inventory[client->pers.selected_item] = 1;
                client->pers.weapon = item;
                client->curr_weap = HC_NUM;
                client->unique_weapon_total = 1;
                item = FindItem("12 Gauge Shells");
                if ( band )
                        client->pers.inventory[ITEM_INDEX(item)] = 24;
                else
                        client->pers.inventory[ITEM_INDEX(item)] = 12;
                client->cannon_rds = client->cannon_max;
        }
        else if ( stricmp(client->resp.weapon->pickup_name, SNIPER_NAME ) == 0 )
        {
                item = FindItem(SNIPER_NAME);
                client->pers.inventory[ITEM_INDEX(item)] = 1;
                client->unique_weapon_total = 1;
                item = FindItem("AP Sniper Ammo");
                if ( band )
                        client->pers.inventory[ITEM_INDEX(item)] = 20;
                else
                        client->pers.inventory[ITEM_INDEX(item)] = 10;
                client->sniper_rds = client->sniper_max;
        }
        else if ( stricmp(client->resp.weapon->pickup_name, DUAL_NAME ) == 0 )
        {
                item = FindItem(DUAL_NAME);
                client->pers.selected_item = ITEM_INDEX(item);
                client->pers.inventory[client->pers.selected_item] = 1;
                client->pers.weapon = item;
                client->curr_weap = DUAL_NUM;
                item = FindItem("Pistol Clip");
                if ( band )
                        client->pers.inventory[ITEM_INDEX(item)] = 4;
                else
                        client->pers.inventory[ITEM_INDEX(item)] = 2;
                client->dual_rds = client->dual_max;
        }
        else if ( stricmp(client->resp.weapon->pickup_name, KNIFE_NAME ) == 0 )
        {
                item = FindItem(KNIFE_NAME);
                client->pers.selected_item = ITEM_INDEX(item);
                if ( band )
                        client->pers.inventory[client->pers.selected_item] = 20;
                else
                        client->pers.inventory[client->pers.selected_item] = 10;
                client->pers.weapon = item;
                client->curr_weap = KNIFE_NUM;
        }
        
        etemp.item = client->resp.item;
        Pickup_Special ( &etemp, ent );
}





/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/

void PutClientInServer (edict_t *ent)
{
        vec3_t  mins = {-16, -16, -24};
        vec3_t  maxs = {16, 16, 32};
        int             index;
        vec3_t  spawn_origin, spawn_angles;
        gclient_t       *client;
        int     going_observer;
        int             i;
        client_persistant_t     saved;
        client_respawn_t        resp;
        // zucc for ammo
//      gitem_t *item;

//FF
	int save_team_wounds;
	int save_team_kills;
	char save_ipaddr[100];
//FF

        // find a spawn point
        // do it before setting health back up, so farthest
        // ranging doesn't count this client
        SelectSpawnPoint (ent, spawn_origin, spawn_angles);

        index = ent-g_edicts-1;
        client = ent->client;

        // deathmatch wipes most client data every spawn
        if (deathmatch->value)
        {
                char            userinfo[MAX_INFO_STRING];

                resp = client->resp;
                memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
                InitClientPersistant (client);
                ClientUserinfoChanged (ent, userinfo);
        }
        else if (coop->value)
        {
                int                     n;
                char            userinfo[MAX_INFO_STRING];

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
//FF 
	save_team_wounds = client->team_wounds;
	save_team_kills = client->team_kills;
	
	if (client->ipaddr)
		strncpy(save_ipaddr, client->ipaddr, sizeof(save_ipaddr)-1);
//FF

        memset (client, 0, sizeof(*client));
        client->pers = saved;
//FF
	client->team_wounds = save_team_wounds;
	client->team_kills = save_team_kills;
		
	if (save_ipaddr && client->ipaddr)
		strncpy(client->ipaddr, save_ipaddr, sizeof(client->ipaddr));
//FF

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
//FIREBLADE
        if (!teamplay->value || ent->client->resp.team != NOTEAM)
        {
                ent->flags &= ~FL_GODMODE;
                ent->svflags &= ~SVF_NOCLIENT;
        }
//FIREBLADE

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
        ent->s.skinnum = ent - g_edicts - 1;
        ent->s.modelindex = 255;                // will use the skin specified model
        
        // zucc vwep
        //ent->s.modelindex2 = 255;             // custom gun model
        ShowGun(ent);
        
        ent->s.frame = 0;
        VectorCopy (spawn_origin, ent->s.origin);
        ent->s.origin[2] += 1;  // make sure off ground
        VectorCopy (ent->s.origin, ent->s.old_origin);

        // set the delta angle
        for (i=0 ; i<3 ; i++)
                client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

        ent->s.angles[PITCH] = 0;
        ent->s.angles[YAW] = spawn_angles[YAW];
        ent->s.angles[ROLL] = 0;
        VectorCopy (ent->s.angles, client->ps.viewangles);
        VectorCopy (ent->s.angles, client->v_angle);

//FIREBLADE
        if (teamplay->value)
        {
                going_observer = StartClient(ent);
        }
                else
        {
                going_observer = ent->client->pers.spectator;
                if (going_observer)
                {
                        ent->movetype = MOVETYPE_NOCLIP;
                        ent->solid = SOLID_NOT;
                        ent->svflags |= SVF_NOCLIENT;
                        ent->client->resp.team = NOTEAM;
                        ent->client->ps.gunindex = 0; 
                }
        }

//FIREBLADE
        if (!going_observer && !teamplay->value)
        { // this handles telefrags...
                KillBox(ent);
        }
//FIREBLADE

        gi.linkentity (ent);

        //zucc give some ammo
        //item = FindItem("Pistol Clip");     
        // Add_Ammo(ent,item,1);
        client->mk23_max = 12;
        client->mp5_max = 30;
        client->m4_max = 24;
        client->shot_max = 7;
        client->sniper_max = 6;
        client->cannon_max = 2;
        client->dual_max = 24;
        client->mk23_rds = client->mk23_max;
        client->dual_rds = client->mk23_max;
        client->knife_max = 10;
        client->grenade_max = 2;        

        ent->lasersight = NULL;

        //other
	client->resp.sniper_mode = SNIPER_1X;
        client->bandaging = 0;
        client->leg_damage = 0;
        client->leg_noise = 0;
        client->leg_dam_count = 0;
        client->desired_fov = 90;
        client->ps.fov = 90;
        client->idle_weapon = 0;
        client->drop_knife = 0;
        client->no_sniper_display = 0;
        client->knife_sound     = 0;
        client->doortoggle = 0;
        client->have_laser = 0; 
                client->reload_attempts = 0;
                client->weapon_attempts = 0;

//FIREBLADE
        if (!going_observer)    
        {

                        // items up here so that the bandolier will change equipclient below
                        if ( allitem->value )
                        {
                                AllItems( ent );
                        }
                        
                        
                        if (teamplay->value)
                                EquipClient(ent);
                        
                        if (ent->client->menu)
                        {
                                PMenu_Close(ent);
                                return;
                        }
//FIREBLADE
                        if ( allweapon->value )
                        {
                                AllWeapons( ent );
                        }
                        
                        // force the current weapon up
                        client->newweapon = client->pers.weapon;
                ChangeWeapon (ent);

//FIREBLADE
                if (teamplay->value)
                {
                        ent->solid = SOLID_TRIGGER;
                        gi.linkentity(ent);
                }
//FIREBLADE
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

        // locate ent at a spawn point
        PutClientInServer (ent);
		
// FROM 3.20 -FB
        if (level.intermissiontime)
        {
                MoveClientToIntermission (ent);
        }
        else
        {
// ^^^
                if (!teamplay->value)
                {  //FB 5/31/99
                        // send effect
                        gi.WriteByte (svc_muzzleflash);
                        gi.WriteShort (ent-g_edicts);
                        gi.WriteByte (MZ_LOGIN);
                        gi.multicast (ent->s.origin, MULTICAST_PVS);
                }
        }

        gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);
//FIREBLADE
        if (deathmatch->value && !teamplay->value && ent->solid == SOLID_NOT)
                gi.bprintf(PRINT_HIGH, "%s became a spectator\n", ent->client->pers.netname);
//FIREBLADE

//FIREBLADE
        PrintMOTD(ent);
        ent->client->resp.motd_refreshes = 1;
//FIREBLADE

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
        int             i;

        ent->client = game.clients + (ent - g_edicts - 1);

        // clear modes of weapons
        /*
	ent->client->resp.mk23_mode = 0;
        ent->client->resp.mp5_mode = 0;
        ent->client->resp.m4_mode = 0;
        ent->client->resp.sniper_mode = 0;
        ent->client->resp.knife_mode = 0;
        ent->client->resp.grenade_mode = 0;
	*/

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
//FIREBLADE
                        if (!teamplay->value)
                        {
//FIREBLADE
                                gi.WriteByte (svc_muzzleflash);
                                gi.WriteShort (ent-g_edicts);
                                gi.WriteByte (MZ_LOGIN);
                                gi.multicast (ent->s.origin, MULTICAST_PVS);
                        }

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
        char    *s, *r;
        int             playernum;

        // check for malformed or illegal info strings
        if (!Info_Validate(userinfo))
        {
                strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
        }

        // set name
        s = Info_ValueForKey (userinfo, "name");
        strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

//FIREBLADE     
        s = Info_ValueForKey(userinfo, "spectator");
        ent->client->pers.spectator = (strcmp(s, "0") != 0);

        r = Info_ValueForKey(userinfo, "rate");
        ent->client->rate = atoi(r);
//FIREBLADE

        // set skin
        s = Info_ValueForKey (userinfo, "skin");
        playernum = ent-g_edicts-1;

        // combine name and skin into a configstring
//FIREBLADE
        if (teamplay->value)
                AssignSkin(ent, s);
                else
//FIREBLADE
                gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

/* Not used in Action.
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
*/
        ent->client->pers.firing_style = ACTION_FIRING_CENTER;
        // handedness
        s = Info_ValueForKey (userinfo, "hand");
        if (strlen(s))
        {
                ent->client->pers.hand = atoi(s);
                if (strstr(s, "classic high") != NULL)
                        ent->client->pers.firing_style = ACTION_FIRING_CLASSIC_HIGH;
                        else if (strstr(s, "classic") != NULL)
                        ent->client->pers.firing_style = ACTION_FIRING_CLASSIC;
        }

        // save off the userinfo in case we want to check something later
        strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);

        // zucc vwep
        ShowGun(ent);

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
        char    *value, *ipaddr;
        char ipaddr_buf[100];

        // check to see if they are on the banned IP list
        ipaddr = Info_ValueForKey (userinfo, "ip");
//FIREBLADE
	if (strlen(ipaddr) > sizeof(ipaddr_buf)-1)
		gi.dprintf("ipaddr_buf length exceeded\n");
        strncpy(ipaddr_buf, ipaddr, 99);
        ipaddr_buf[99] = 0;
//FIREBLADE

// FROM 3.20  -FB
        if (SV_FilterPacket(ipaddr)) {
                Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
                return false;
        }

        // check for a password
        value = Info_ValueForKey (userinfo, "password");
        if (*password->string && strcmp(password->string, "none") && 
                        strcmp(password->string, value)) {
                Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
                return false;
        }
// ^^^

        // they can connect
        ent->client = game.clients + (ent - g_edicts - 1);

//AZEROV
	ent->client->team_kills = 0;
//AZEROV

//EEK
	ent->client->team_wounds = 0;
	ent->client->team_wounds_before = 0;
//EEK

//FIREBLADE
// We're not going to attempt to support reconnection...
        if (ent->inuse == true)
        {
                ClientDisconnect(ent);
                ent->inuse = false;
        }
//FIREBLADE

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
                gi.dprintf ("%s@%s connected\n",ent->client->pers.netname, ipaddr_buf);
//EEK
		strncpy(ent->client->ipaddr, ipaddr_buf, sizeof(ent->client->ipaddr));
//EEK

// FROM 3.20 -FB
        ent->svflags = 0;
// ^^^
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
        int             playernum, i;
        edict_t *etemp;

        if (!ent->client)
                return;

        // drop items if they are alive/not observer
        if ( ent->solid != SOLID_NOT )
                TossItemsOnDeath(ent);
        
        // zucc free the lasersight if applicable
        if (ent->lasersight)
                SP_LaserSight(ent, NULL);

//FIREBLADE
        if (teamplay->value && ent->solid == SOLID_TRIGGER)
                RemoveFromTransparentList(ent);
//FIREBLADE

        ent->lasersight = NULL;

        gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

        // go clear any clients that have this guy as their attacker

        for( i=1; i<=maxclients->value; i++ )                   
        {               
                if ( (etemp=&g_edicts[i]) && etemp->inuse )
                {
                        if ( etemp->client->attacker == ent )
                                etemp->client->attacker = NULL;
                }
        }
        if (!teamplay->value)
        {  //FB 5/31/99
                // send effect
                gi.WriteByte (svc_muzzleflash);
                gi.WriteShort (ent-g_edicts);
                gi.WriteByte (MZ_LOGOUT);
                gi.multicast (ent->s.origin, MULTICAST_PVS);
        }

        gi.unlinkentity (ent);
        ent->s.modelindex = 0;
        ent->solid = SOLID_NOT;
        ent->inuse = false;
        ent->classname = "disconnected";
        ent->client->pers.connected = false;

        playernum = ent-g_edicts-1;
        gi.configstring (CS_PLAYERSKINS+playernum, "");

//FIREBLADE
        if (teamplay->value)
        {
                CheckForUnevenTeams();
        }
//FIREBLADE
}


//==============================================================


edict_t *pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
        if (pm_passent->health > 0)
                return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
        else
                return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
        int     v,i;
        v = 0;
        for (i=0 ; i<c ; i++)
                v+= ((byte *)b)[i];
        return v;
}
void PrintPmove (pmove_t *pm)
{
        unsigned        c1, c2;

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
        gclient_t       *client;
        edict_t *other;
        int             i, j;
        pmove_t pm;

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

//FIREBLADE
        if ((int)motd_time->value > (client->resp.motd_refreshes * 2))
        {
                if (client->resp.last_motd_refresh < (level.framenum - 20))
                {
                        client->resp.last_motd_refresh = level.framenum;
                        client->resp.motd_refreshes++;
                        PrintMOTD(ent);
                }
        }
//FIREBLADE

        pm_passent = ent;

// FROM 3.20 -FB
        if (ent->client->chase_mode) {
                client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
                client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
                client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);
        } else {
// ^^^
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
        
                client->ps.pmove.gravity = sv_gravity->value;
                pm.s = client->ps.pmove;
        
                for (i=0 ; i<3 ; i++)
                {
                        pm.s.origin[i] = ent->s.origin[i]*8;
                        pm.s.velocity[i] = ent->velocity[i] * 8;
                }
                
                if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
                {
                        pm.snapinitial = true;
//                    gi.dprintf ("pmove changed!\n");
                }

                pm.cmd = *ucmd;

                pm.trace = PM_trace;    // adds default parms
                pm.pointcontents = gi.pointcontents;

                // perform a pmove
                gi.Pmove (&pm);

//FB 6/3/99 - info from Mikael Lindh from AQ:G
		if (pm.maxs[2] == 4)
		{
			ent->maxs[2] = CROUCHING_MAXS2;
			pm.maxs[2] = CROUCHING_MAXS2;
			ent->viewheight = CROUCHING_VIEWHEIGHT;
			pm.viewheight = (float)ent->viewheight;
		}
//FB 6/3/99

                // save results of pmove
                client->ps.pmove = pm.s;
                client->old_pmove = pm.s;


                                // really stopping jumping with leg damage
                                if ( ent->client->leg_damage && ent->groundentity && pm.s.velocity[2] > 10 )
                                {
                                        pm.s.velocity[2] = 0.0;
                                }

                                for (i=0 ; i<3 ; i++)
                                {
                                        ent->s.origin[i] = pm.s.origin[i]*0.125;
                                        ent->velocity[i] = pm.s.velocity[i]*0.125;
                                }


                // zucc stumbling associated with leg damage
                                if (level.framenum % 6  <= 2)
                                {
                                        
                                        //Slow down code FOO/zucc
                                        for (i=0 ; i<3 ; i++)
                                        {
                                                if ( ent->client->leg_damage && ((i < 2) || (ent->velocity[2] > 0)) && (ent->groundentity && pm.groundentity) )
                                                        ent->velocity[i] /= 4*ent->client->leghits; //FOO       
                                        }
                                        if (level.framenum % (6*12) == 0 && ent->client->leg_damage > 1)
                                                        gi.sound (ent, CHAN_BODY, gi.soundindex(va("*pain100_1.wav")), 1, ATTN_NORM, 0);
                                        ent->velocity[0] = (float)((int)(ent->velocity[0]*8))/8;
                                        ent->velocity[1] = (float)((int)(ent->velocity[1]*8))/8;
                                        ent->velocity[2] = (float)((int)(ent->velocity[2]*8))/8;
                                }

                                /*
                if ( ent->client->leg_damage )
                {
                        ent->client->leg_dam_count++;
                        ent->client->leg_noise++;
                        if ( ent->groundentity && pm.s.velocity[2] > 10 )
                        {
                        //      gi.cprintf(ent, PRINT_HIGH, "Cutting velocity\n");
                                pm.s.velocity[2] = 0.0;
                        }
                }
        
                if ( ent->client->leg_noise == 250 && ent->health > 0 )// && ent->groundentity && crandom() > 0.0 )
                {
//                      gi.cprintf(ent, PRINT_HIGH, "Playing sound?\n");
                        ent->client->leg_noise = 0;
                        if (IsFemale(ent))
                                gi.sound(ent, CHAN_VOICE, gi.soundindex("player/female/pain100_1.wav"), 1, ATTN_IDLE, 0);
                        else
                                gi.sound(ent, CHAN_VOICE, gi.soundindex("player/male/pain100_1.wav"), 1, ATTN_IDLE, 0);
                        PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
                }
        
                if ( !ent->groundentity )
                {       
                        for (i=0 ; i<3 ; i++)
                        {
                                ent->s.origin[i] = pm.s.origin[i]*0.125;
                                ent->velocity[i] = pm.s.velocity[i]*0.125;
                        }
                }
                else if ( ent->client->leg_dam_count < 10 )
                {
                                
                        for (i=0 ; i<3 ; i++)
                        {
                                ent->s.origin[i] = pm.s.origin[i]*0.125;
                                ent->velocity[i] = pm.s.velocity[i]*0.125;
                        }
                }       
                else 
                {
                        for (i=0 ; i<3 ; i++)
                        {
                                ent->velocity[i] = 0;//pm.s.velocity[i]*0.125;
                        }
                        if ( ent->client->leg_dam_count > 11 )
                                ent->client->leg_dam_count = 0;
                }
*/
                VectorCopy (pm.mins, ent->mins);
                VectorCopy (pm.maxs, ent->maxs);
        
                client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
                client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
                client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

                // don't play sounds if they have leg damage, they can't jump anyway
                if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0)
                        && !ent->client->leg_damage )
                {
                        /* don't play jumps period.
                        gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
                        PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
                        */
                        ent->client->jumping = 1;
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
        
                // stop manipulating doors
                client->doortoggle = 0;

                if ( ent->client->jumping && 
//FIREBLADE
                        ent->solid != SOLID_NOT )
//FIREBLADE
                        kick_attack( ent );

                // touch other objects
                for (i=0 ; i<pm.numtouch ; i++)
                {
                        other = pm.touchents[i];
                        for (j=0 ; j<i ; j++)
                                if (pm.touchents[j] == other)
                                        break;
                        if (j != i)
                                continue;       // duplicated
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
        if ((client->latched_buttons & BUTTON_ATTACK)
           //Black Cross - Begin
            || (((limchasecam->value && !client->chase_mode) ||
                 (limchasecam->value == 2 && client->chase_mode == 1)) &&
                team_round_going && (client->resp.team != NOTEAM) && 
                !(limchasecam->value == 2 && client->chase_mode == 2))
           //Black Cross - End 
           )
        {
                if (ent->solid == SOLID_NOT && ent->deadflag != DEAD_DEAD)
                {
                        client->latched_buttons = 0;
                        if (client->chase_mode)
                        {
                                if (client->chase_mode == 1)
                                {
                                        client->desired_fov = 90;
                                        client->ps.fov = 90;
                                        client->chase_mode++;
                                }
                                        else
                                {
                                        client->chase_mode = 0;
                                        client->chase_target = NULL;
                                        client->desired_fov = 90;
                                        client->ps.fov = 90;
                                        client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
                                }
                        }
                                else
                        {
                                client->chase_target = NULL;
                                GetChaseTarget(ent);
                                if (client->chase_target != NULL)
                                {
                                        client->chase_mode = 1;
                                        UpdateChaseCam(ent);
                                }
                        }
                }
                         else if (!client->weapon_thunk)
                {
                        client->weapon_thunk = true;
                        Think_Weapon (ent);
                }
        }

        if (client->chase_mode)
        {
                if (ucmd->upmove >= 10)
                {
                        if (!(client->ps.pmove.pm_flags & PMF_JUMP_HELD))
                        {
                                client->ps.pmove.pm_flags |= PMF_JUMP_HELD;
                                if (client->chase_target)
                                {
                                        ChaseNext(ent);
                                }
                                        else
                                {
                                        GetChaseTarget(ent);
                                        UpdateChaseCam(ent);
                                }
                        }
                }
                        else
                        client->ps.pmove.pm_flags &= ~PMF_JUMP_HELD;

//FIREBLADE
                ChaseTargetGone(ent);  // run a check...result not important.
//FIREBLADE
        }

// FROM 3.20 -FB
        // update chase cam if being followed
        for (i = 1; i <= maxclients->value; i++) {
                other = g_edicts + i;
                if (other->inuse && other->client->chase_mode && other->client->chase_target == ent)
                        UpdateChaseCam(other);
        }
// ^^^
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
        gclient_t       *client;
        int                     buttonMask;

        if (level.intermissiontime)
                return;

        client = ent->client;

//FIREBLADE
        if (deathmatch->value && !teamplay->value &&
                ((ent->solid == SOLID_NOT && ent->deadflag != DEAD_DEAD) != ent->client->pers.spectator))
        {
                if (ent->solid != SOLID_NOT || ent->deadflag == DEAD_DEAD)
                {
                        if (ent->deadflag != DEAD_DEAD)
                        {
                                ent->flags &= ~FL_GODMODE;
                                ent->health = 0;
                                meansOfDeath = MOD_SUICIDE;
                                player_die (ent, ent, ent, 100000, vec3_origin);
                                // don't even bother waiting for death frames
                                ent->deadflag = DEAD_DEAD;
        
                                // This will make ClientBeginServerFrame crank us into observer mode
                                // as soon as our death frames are done... -FB
                                ent->solid = SOLID_NOT;
                                // Also set this so we can have a way to know we've already done this...
                                ent->movetype = MOVETYPE_NOCLIP;

                                gi.linkentity(ent);

                                gi.bprintf(PRINT_HIGH, "%s became a spectator\n", ent->client->pers.netname);
                        }
                                else    // immediately become observer...
                        {
                                if (ent->movetype != MOVETYPE_NOCLIP)  // have we already done this?  see above...
                                {
                                        CopyToBodyQue(ent);
                                        ent->solid = SOLID_NOT;
                                        ent->svflags |= SVF_NOCLIENT;
                                        ent->movetype = MOVETYPE_NOCLIP;
                                        ent->client->pers.health = 100;
                                        ent->health = 100;
                                        ent->deadflag = DEAD_NO;
                                        gi.linkentity(ent);
                                        gi.bprintf(PRINT_HIGH, "%s became a spectator\n", ent->client->pers.netname);
                                }
                        }
                }
                        else
                {
                        ent->client->chase_mode = 0;
                        ent->client->chase_target = NULL;
                        ent->client->desired_fov = 90;
                        ent->client->ps.fov = 90; // FB 5/31/99 added
                        ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
                        ent->solid = SOLID_BBOX;
                        gi.linkentity(ent);
                        gi.bprintf(PRINT_HIGH, "%s rejoined the game\n", ent->client->pers.netname);
                        respawn(ent);
                }
        }

//FIREBLADE

        // run weapon animations if it hasn't been done by a ucmd_t
        if (!client->weapon_thunk)
                Think_Weapon (ent);
        else
                client->weapon_thunk = false;

        if (ent->deadflag)
        {
                // wait for any button just going down
                if (level.time > client->respawn_time)
                {
//FIREBLADE
                        if (teamplay->value || 
                                (!teamplay->value && ent->client->pers.spectator && ent->solid == SOLID_NOT &&
                                 ent->deadflag == DEAD_DEAD))
                        {
                                CopyToBodyQue(ent);
                                
                                ent->solid = SOLID_NOT;
                                ent->svflags |= SVF_NOCLIENT;
                                ent->movetype = MOVETYPE_NOCLIP;
                                ent->client->pers.health = 100;
                                ent->health = 100;
                                ent->deadflag = DEAD_NO;

                                client->ps.pmove.delta_angles[PITCH] = ANGLE2SHORT(0 - client->resp.cmd_angles[PITCH]);
                                client->ps.pmove.delta_angles[YAW] = ANGLE2SHORT(client->killer_yaw - client->resp.cmd_angles[YAW]);
                                client->ps.pmove.delta_angles[ROLL] = ANGLE2SHORT(0 - client->resp.cmd_angles[ROLL]);
                                ent->s.angles[PITCH] = 0;
                                ent->s.angles[YAW] = client->killer_yaw;
                                ent->s.angles[ROLL] = 0;
                                VectorCopy (ent->s.angles, client->ps.viewangles);
                                VectorCopy (ent->s.angles, client->v_angle);

                                gi.linkentity(ent);
                        }
//FIREBLADE
                                else
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
                }
                return;
        }

        // add player trail so monsters can follow
        if (!deathmatch->value)
                if (!visible (ent, PlayerTrail_LastSpot() ) )
                        PlayerTrail_Add (ent->s.old_origin);

        client->latched_buttons = 0;
}
