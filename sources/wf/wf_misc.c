/*==============================================================================
The Weapons Factory - 
Misc Functions
Original code by Gregg Reno and others
==============================================================================*/
#include "g_local.h"
#include "wf_classmgr.h"
#include "stdlog.h"	//	StdLog - Mark Davies
void mturret_remove(edict_t *self);

void wf_earthquake (edict_t *owner);
void NoAmmoWeaponChange (edict_t *ent);
void weapon_grenade_fire (edict_t *ent, qboolean held);
void Weapon_Grenade (edict_t *ent);
void Pipebomb_Explode (edict_t *ent);
void alarm_remove(edict_t *self);
void Goodyear_Explode (edict_t *ent);
void Proximity_Explode (edict_t *ent);
void laser_defense_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void supplydepot_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void SetDefaultClassInfo();
void Turret_Explode (edict_t *ent);
void wf_GiveItem(gclient_t *client, char *item_name, int item_count);
void remote_remove(edict_t *ent);
void Remove_Flame(edict_t *ent);
void AutoClassConfig(edict_t *ent);
char *getWeaponState(edict_t *ent);
int fireButtonOn(edict_t *ent);
int get_mercenary_team(edict_t *ent);
void Biosentry_remove(edict_t *ent);
void HealPlayer(edict_t *ent);

extern ctfgame_t ctfgame;


#define GRENADE_TIMER 3.0

void wf_set_grenade (edict_t *ent)
{

	gitem_t	*g;
	
	g = FindItem("Grenades");

	//Switch to the grenade if are not already on it
	if (ent->client->pers.weapon != g)
	{
		ent->client->newweapon = g;
		ent->client->ps.gunframe = 1;
		ent->client->weaponstate = WEAPON_READY;
		ent->client->grenade_time = 0;
		Weapon_Grenade (ent);
	}
	lasersight_off (ent);

	wf_show_grenade_type(ent);
	//ent->cantmove = 0;	//free player movement
							//this allows cyborg free movement after
							//dropping plasma bomb. had to disable it

}

//What model is used by this class?
int wf_GetClassModel(int p_class)
{
	int p_model;

	if (p_class <= 0 || p_class > numclasses)
		p_model = CLASS_MODEL_MALE;
	else
		p_model = classinfo[p_class].model;

	return (p_model);

}

//What model is used by this class?
int wf_GetModelFromName(char *name)
{

	if (name[0] == 'F' || name[0] == 'f')
		return CLASS_MODEL_FEMALE;
	else if (name[0] == 'C' || name[0] == 'c')
		return CLASS_MODEL_CYBORG;
	else
		return CLASS_MODEL_MALE;

}


/*
=================
Cmd_Grenade_f
Set the type of grenade
=================
*/
void Cmd_Grenade_f (edict_t *ent)
{
	char    *string;
	int max_grenade_types;
	int old_type;

	//If weapon is being fired don't switch
	if(!(ent->client->weaponstate == WEAPON_READY))
		return;

	string=gi.args();

	old_type = ent->client->pers.grenade_type;

	//If player classes are on, just cycle through the three
	//grenade types
	if (((int)wfflags->value & WF_NO_PLAYER_CLASSES) == 0)
		max_grenade_types = 3;
	else
		max_grenade_types = GRENADE_TYPE_COUNT;

	//If no argument, go to next grenade type
	if (Q_stricmp ( string, "") == 0)
	{
		//Pick next grenade number
		ent->client->pers.grenade_num += 1;
		if (ent->client->pers.grenade_num > max_grenade_types)
			ent->client->pers.grenade_num = 1;

		//Set grenade type based on grenade number
		if (((int)wfflags->value & WF_NO_PLAYER_CLASSES) == 0)
		{
			if (ent->client->pers.grenade_num == 1)	ent->client->pers.grenade_type = ent->client->grenade_type1;
			if (ent->client->pers.grenade_num == 2)	ent->client->pers.grenade_type = ent->client->grenade_type2;
			if (ent->client->pers.grenade_num == 3)	ent->client->pers.grenade_type = ent->client->grenade_type3;
			//Test for empty grenade type.  If so, use slot 1 grenade
			if (ent->client->pers.grenade_type == GRENADE_TYPE_NONE)
			{
				ent->client->pers.grenade_num = 1;
				ent->client->pers.grenade_type = ent->client->grenade_type1;
			}
		}
		else
			ent->client->pers.grenade_type = ent->client->pers.grenade_num;
	}
	else if (Q_stricmp ( string, "normal") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_NORMAL;
		}
	else if (Q_stricmp ( string, "laserball") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_LASERBALL;
		}
	else if (Q_stricmp ( string, "proximity") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_PROXIMITY;
		}
	else if (Q_stricmp ( string, "cluster") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_CLUSTER;
		}
//	else if (Q_stricmp ( string, "pipebomb") == 0)
//		{
//		ent->client->pers.grenade_type = GRENADE_TYPE_PIPEBOMB;
//		}
	else if (Q_stricmp ( string, "goodyear") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_GOODYEAR;
		}
	else if (Q_stricmp ( string, "flash") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_FLASH;
		}
	else if (Q_stricmp ( string, "quake") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_EARTHQUAKE;
		}
	else if (Q_stricmp ( string, "turret") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_TURRET;
		}
	else if (Q_stricmp ( string, "napalm") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_NAPALM;
		}
	else if (Q_stricmp ( string, "concussion") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_CONCUSSION;
		}
	else if (Q_stricmp ( string, "narcotic") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_NARCOTIC;
		}
	else if (Q_stricmp ( string, "plague") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_PLAGUE;
		}
	else if (Q_stricmp ( string, "magnotron") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_MAGNOTRON;
		}
	else if (Q_stricmp ( string, "shock") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_SHOCK;
		}
	else if (Q_stricmp ( string, "pipebomb") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_PIPEBOMB;
		}
	else if (Q_stricmp ( string, "shrapnel") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_SHRAPNEL;
		}
	else if (Q_stricmp ( string, "flare") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_FLARE;
		}
	else if (Q_stricmp ( string, "slow") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_SLOW;
		}
	else if (Q_stricmp ( string, "lasercutter") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_LASERCUTTER;
		}
	else if (Q_stricmp ( string, "tesla") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_TESLA;
		}
	else if (Q_stricmp ( string, "gas") == 0)
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_GAS;
		}
	else	//default to normal grenade
		{
		ent->client->pers.grenade_type = GRENADE_TYPE_NORMAL;
		}

	//Check for valid grenade
	if (((int)wfflags->value & WF_NO_PLAYER_CLASSES) == 0)
	{
		if ((ent->client->pers.grenade_type != ent->client->grenade_type1) &&
		    (ent->client->pers.grenade_type != ent->client->grenade_type2) &&
		    (ent->client->pers.grenade_type != ent->client->grenade_type3))
		{
			safe_cprintf (ent, PRINT_HIGH, "You can't use that type of grenade.\n");
			ent->client->pers.grenade_type = old_type;
		}

	}

	wf_set_grenade(ent);
}




void Cmd_Grenade1 (edict_t *ent)
{

	//If weapon is being fired don't switch
	if(!(ent->client->weaponstate == WEAPON_READY))
		return;

	ent->client->pers.grenade_num = 1;
	ent->client->pers.grenade_type = ent->client->grenade_type1;
//	wf_show_grenade_type(ent);
	wf_set_grenade(ent);
}

void Cmd_Grenade2 (edict_t *ent)
{

	//If weapon is being fired don't switch
	if(!(ent->client->weaponstate == WEAPON_READY))
		return;

	ent->client->pers.grenade_num = 2;
	ent->client->pers.grenade_type = ent->client->grenade_type2;
//	wf_show_grenade_type(ent);
	wf_set_grenade(ent);
}

void Cmd_Grenade3 (edict_t *ent)
{

	//If weapon is being fired don't switch
	if(!(ent->client->weaponstate == WEAPON_READY))
		return;
	//This grenade type could be empty
	if (ent->client->grenade_type3 == GRENADE_TYPE_NONE)
	{
		//Use grenade 1 instead
		ent->client->pers.grenade_num = 1;
		ent->client->pers.grenade_type = ent->client->grenade_type1;
	}
	else
	{
		ent->client->pers.grenade_num = 3;
		ent->client->pers.grenade_type = ent->client->grenade_type3;
	}
//	wf_show_grenade_type(ent);
	wf_set_grenade(ent);
}

/* wf_show_grenade_type */
void wf_show_grenade_type(edict_t *ent)
{
	ent->client->weapon_damage = 0;
	if (!ent->client) return;

	if	(ent->client->pers.grenade_type > 0 && 
		 ent->client->pers.grenade_type <= GRENADE_TYPE_COUNT)
		ent->client->weapon_damage = wf_game.grenade_damage[ent->client->pers.grenade_type];

	switch (ent->client->pers.grenade_type)
		{
		case (GRENADE_TYPE_NORMAL):
			safe_cprintf (ent, PRINT_HIGH, "Normal Grenade\n");
			break;
		case (GRENADE_TYPE_PROXIMITY):
			safe_cprintf (ent, PRINT_HIGH, "Proximity Grenade\n");
			break;
		case (GRENADE_TYPE_CLUSTER):
			safe_cprintf (ent, PRINT_HIGH, "Cluster Grenade\n");
			break;
		case (GRENADE_TYPE_LASERBALL):
			safe_cprintf (ent, PRINT_HIGH, "Laserball Grenade\n");
			break;
		case (GRENADE_TYPE_FLASH):
			safe_cprintf (ent, PRINT_HIGH, "Flash Grenade\n");
			break;
		case (GRENADE_TYPE_GOODYEAR):
			safe_cprintf (ent, PRINT_HIGH, "Goodyear Grenade\n");
			break;
		case (GRENADE_TYPE_EARTHQUAKE):
			safe_cprintf (ent, PRINT_HIGH, "Earthquake Grenade\n");
			break;
		case (GRENADE_TYPE_TURRET):
			safe_cprintf (ent, PRINT_HIGH, "Turret Grenade\n");
			break;
		case (GRENADE_TYPE_CONCUSSION):
			safe_cprintf (ent, PRINT_HIGH, "Concussion Grenade\n");
			break;
		case (GRENADE_TYPE_NARCOTIC):
			safe_cprintf (ent, PRINT_HIGH, "Narcotic Grenade\n");
			break;
		case (GRENADE_TYPE_PLAGUE):
			safe_cprintf (ent, PRINT_HIGH, "Plague Grenade\n");
			break;
		case (GRENADE_TYPE_MAGNOTRON):
			safe_cprintf (ent, PRINT_HIGH, "Magnotron Grenade\n");
			break;
		case (GRENADE_TYPE_SHOCK):
			safe_cprintf (ent, PRINT_HIGH, "Shock Grenade\n");
			break;
		case (GRENADE_TYPE_PIPEBOMB):
			safe_cprintf (ent, PRINT_HIGH, "Pipebomb Grenade\n");
			break;
		case (GRENADE_TYPE_SHRAPNEL):
			safe_cprintf (ent, PRINT_HIGH, "Shrapnel Grenade\n");
			break;
		case (GRENADE_TYPE_FLARE):
			safe_cprintf (ent, PRINT_HIGH, "Flare\n");
			break;
		case (GRENADE_TYPE_SLOW):
			safe_cprintf (ent, PRINT_HIGH, "Slow Grenade\n");
			break;
		case (GRENADE_TYPE_NAPALM):
			safe_cprintf (ent, PRINT_HIGH, "Napalm Grenade\n");
			break;
		case (GRENADE_TYPE_LASERCUTTER):
			safe_cprintf (ent, PRINT_HIGH, "Lasercutter Grenade\n");
			break;
		case (GRENADE_TYPE_TESLA):
			safe_cprintf (ent, PRINT_HIGH, "Tesla Grenade\n");
			break;
		case (GRENADE_TYPE_GAS):
			safe_cprintf (ent, PRINT_HIGH, "Gas Grenade\n");
			break;
		default:
			safe_cprintf (ent, PRINT_HIGH, "UNKNOWN Grenade\n");
			ent->client->weapon_damage = 0;
			break;
		}
}


/*
=================
Cmd_Thrust_f

MUCE:
To set jetpack on or off
=================
*/
void Cmd_Thrust_f (edict_t *ent)
{
        char    *string;

        string=gi.args();

        if (Q_stricmp ( string, "on") == 0)
        {
                ent->client->thrusting=1;
                ent->client->next_thrust_sound=0;
        }
        else
        {
                ent->client->thrusting=0;
        }
}

/*
=================
Cmd_Homing_f
CCH: whole new function for adjusting homing missile state
=================
*/
void Cmd_Homing_f (edict_t *ent)
{
	char    *string;
	gitem_t	*weap;

	string=gi.args();

	if (Q_stricmp ( string, "on") == 0)
	{
		safe_cprintf (ent, PRINT_HIGH, "HOMING MISSILES ON\n");
		ent->client->pers.homing_state = 1;
	}
	else if (Q_stricmp ( string, "off") == 0)
	{
		safe_cprintf (ent, PRINT_HIGH, "Homing missiles off\n");
		ent->client->pers.homing_state = 0;
	}
	else	//If no "on" or "off", toggle state
	{
		if (ent->client->pers.homing_state == 0)
		{
			safe_cprintf (ent, PRINT_HIGH, "HOMING MISSILES ON\n");
			ent->client->pers.homing_state = 1;
		}
		else
		{
		safe_cprintf (ent, PRINT_HIGH, "Homing missiles off\n");
		ent->client->pers.homing_state = 0;
		}
	}

	//Special case for napam missile.  If it's active and
	//homing is on, turn on lasersight
	
	weap = FindItem("Rocket Napalm Launcher");
	if (ent->client->pers.weapon == weap)
	{
		if (ent->client->pers.homing_state == 1)
			lasersight_on (ent);
		else
			lasersight_off (ent);
	}
}

/*
=================
Cmd_AutoConfig_f
If set, class specific config files will be run each time
a player chooses a new class
=================
*/
void Cmd_AutoConfig_f (edict_t *ent)
{
        char    *string;

        string=gi.args();

        if (Q_stricmp ( string, "on") == 0)
			{
            safe_cprintf (ent, PRINT_HIGH, "Auto Config On\n");
            ent->client->pers.autoconfig = 1;
			}
		else if (Q_stricmp ( string, "off") == 0)
			{
            safe_cprintf (ent, PRINT_HIGH, "Auto Config Off\n");
            ent->client->pers.autoconfig = 0;
			}
        else	//If no "on" or "off", toggle state
			{
			if (ent->client->pers.autoconfig == 0)
				{
                safe_cprintf (ent, PRINT_HIGH, "Auto Config On\n");
                ent->client->pers.autoconfig = 1;
				}
			else
				{
	            safe_cprintf(ent, PRINT_HIGH, "Auto Config Off\n");
		        ent->client->pers.autoconfig = 0;
				}
            }
	AutoClassConfig(ent);
}

/*
=================
Cmd_Reno_f

Special Reno Commands
=================
*/
void Cmd_Reno_f (edict_t *ent)
{
	char    *string;

        string=gi.args();

        if (Q_stricmp ( string, "enter") == 0)
        {
                gi.sound (ent, CHAN_VOICE, gi.soundindex ("renoenter.wav"), 1, ATTN_NORM, 0);
        }
        if (Q_stricmp ( string, "exit") == 0)
        {
                gi.sound (ent, CHAN_VOICE, gi.soundindex ("renoexit.wav"), 1, ATTN_NORM, 0);
        }
        if (Q_stricmp ( string, "talk") == 0)
        {
                gi.sound (ent, CHAN_VOICE, gi.soundindex ("renotalk.wav"), 1, ATTN_NORM, 0);
        }
}

//Respawn protection function from Keys2 Mod
qboolean K2_IsProtected(edict_t *ent)
{
	if (!ent->client)
		return false;

	if (ent->client->protecttime > level.time)
		return true;
	else
		return false;
}


/*
==================================
Cmd_WFFlags

Show the Weapons Factory Flags
==================================
*/
void Cmd_WFFlags_f (edict_t *ent)
{
	if (ent->bot_client) return;
	if (!ent->client)  return;

    safe_cprintf (ent, PRINT_HIGH, "WF Flags = %d\n", (int)wfflags->value);

	if ((int)wfflags->value & WF_ALLOW_FRIENDLY_FIRE)
		safe_cprintf (ent, PRINT_HIGH, "WF_ALLOW_FRIENDLY_FIRE is ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_ALLOW_FRIENDLY_FIRE is OFF\n");

	if ((int)wfflags->value & WF_FRAG_LOGGING)
		safe_cprintf (ent, PRINT_HIGH, "WF_FRAG_LOGGING is ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_FRAG_LOGGING is OFF\n");

	if ((int)wfflags->value & WF_NO_FORT_RESPAWN)
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_FORT_RESPAWN is ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_FORT_RESPAWN is OFF\n");

	if ((int)wfflags->value & WF_NO_HOMING)
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_HOMING is ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_HOMING is OFF\n");


	if ((int)wfflags->value & WF_NO_DECOYS)
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_DECOYS is ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_DECOYS is OFF\n");

	if ((int)wfflags->value & WF_NO_FLYING)
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_FLYING is ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_FLYING is OFF\n");

	if ((int)wfflags->value & WF_DECOY_PURSUE)
		safe_cprintf (ent, PRINT_HIGH, "WF_DECOY_PURSUE is ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_DECOY_PURSUE is OFF\n");


	if ((int)wfflags->value & WF_NO_TURRET)
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_TURRET is ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_TURRET is OFF\n");

	if ((int)wfflags->value & WF_NO_EARTHQUAKES)
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_EARTHQUAKES is ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_EARTHQUAKES is OFF\n");

	if ((int)wfflags->value & WF_NO_GRAPPLE)
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_GRAPPLE is ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_GRAPPLE is OFF\n");

	if ((int)wfflags->value & WF_MAP_VOTE)
		safe_cprintf (ent, PRINT_HIGH, "WF_MAP_VOTE is ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_MAP_VOTE is OFF\n");

	if ((int)wfflags->value & WF_ZOID_FLAGCAP)
		safe_cprintf (ent, PRINT_HIGH, "WF_ZOID_FLAGCAP is ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_ZOID_FLAGCAP is OFF\n");

	if ((int)wfflags->value & WF_NO_PLAYER_CLASSES)
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_PLAYER_CLASSES are ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_NO_PLAYER_CLASSES are OFF\n");

	if ((int)wfflags->value & WF_ZBOT_DETECT)
		safe_cprintf (ent, PRINT_HIGH, "WF_ZBOT_DETECT is ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_ZBOT_DETECT is OFF\n");

	if ((int)wfflags->value & WF_AUTO_TEAM_BALANCE)
		safe_cprintf (ent, PRINT_HIGH, "WF_AUTO_TEAM_BALANCE is ON\n");
	else
		safe_cprintf (ent, PRINT_HIGH, "WF_AUTO_TEAM_BALANCE is OFF\n");
}

void SV_WFFlags_f ()
{

    gi.dprintf ( "WF Flags = %d\n", (int)wfflags->value);

	if ((int)wfflags->value & WF_ALLOW_FRIENDLY_FIRE)
		gi.dprintf ("WF_ALLOW_FRIENDLY_FIRE is ON\n");
	else
		gi.dprintf ("WF_ALLOW_FRIENDLY_FIRE is OFF\n");

	if ((int)wfflags->value & WF_FRAG_LOGGING)
		gi.dprintf ("WF_FRAG_LOGGING is ON\n");
	else
		gi.dprintf ("WF_FRAG_LOGGING is OFF\n");

	if ((int)wfflags->value & WF_NO_FORT_RESPAWN)
		gi.dprintf ("WF_NO_FORT_RESPAWN is ON\n");
	else
		gi.dprintf ("WF_NO_FORT_RESPAWN is OFF\n");

	if ((int)wfflags->value & WF_NO_HOMING)
		gi.dprintf ("WF_NO_HOMING is ON\n");
	else
		gi.dprintf ("WF_NO_HOMING is OFF\n");

	if ((int)wfflags->value & WF_NO_DECOYS)
		gi.dprintf ("WF_NO_DECOYS is ON\n");
	else
		gi.dprintf ("WF_NO_DECOYS is OFF\n");


	if ((int)wfflags->value & WF_NO_FLYING)
		gi.dprintf ("WF_NO_FLYING is ON\n");
	else
		gi.dprintf ("WF_NO_FLYING is OFF\n");

	if ((int)wfflags->value & WF_DECOY_PURSUE)
		gi.dprintf ("WF_DECOY_PURSUE is ON\n");
	else
		gi.dprintf ("WF_DECOY_PURSUE is OFF\n");


	if ((int)wfflags->value & WF_NO_TURRET)
		gi.dprintf ("WF_NO_TURRET is ON\n");
	else
		gi.dprintf ("WF_NO_TURRET is OFF\n");

	if ((int)wfflags->value & WF_NO_EARTHQUAKES)
		gi.dprintf ("WF_NO_EARTHQUAKES is ON\n");
	else
		gi.dprintf ("WF_NO_EARTHQUAKES is OFF\n");

	if ((int)wfflags->value & WF_NO_GRAPPLE)
		gi.dprintf ("WF_NO_GRAPPLE is ON\n");
	else
		gi.dprintf ("WF_NO_GRAPPLE is OFF\n");

	if ((int)wfflags->value & WF_MAP_VOTE)
		gi.dprintf ("WF_MAP_VOTE is ON\n");
	else
		gi.dprintf ("WF_MAP_VOTE is OFF\n");

	if ((int)wfflags->value & WF_ZOID_FLAGCAP)
		gi.dprintf ("WF_ZOID_FLAGCAP is ON\n");
	else
		gi.dprintf ("WF_ZOID_FLAGCAP is OFF\n");

	if ((int)wfflags->value & WF_NO_PLAYER_CLASSES)
		gi.dprintf ("WF_NO_PLAYER_CLASSES are ON\n");
	else
		gi.dprintf ("WF_NO_PLAYER_CLASSES are OFF\n");

	if ((int)wfflags->value & WF_ZBOT_DETECT)
		gi.dprintf ("WF_ZBOT_DETECT is ON\n");
	else
		gi.dprintf ("WF_ZBOT_DETECT is OFF\n");

	if ((int)wfflags->value & WF_AUTO_TEAM_BALANCE)
		gi.dprintf ("WF_AUTO_TEAM_BALANCE is ON\n");
	else
		gi.dprintf ("WF_AUTO_TEAM_BALANCE is OFF\n");


}


/*
==================================
wf_IsWeapon

Determine if an item is a weapon that should be removed 
from map (if player classes are used

Also checks for other items to remove
==================================
*/
qboolean wf_IsWeapon(char *classname)
{
	if (strcmp(classname, "weapon_shotgun") == 0) return true;
	if (strcmp(classname, "weapon_supershotgun") == 0) return true;
	if (strcmp(classname, "weapon_machinegun") == 0) return true;
	if (strcmp(classname, "weapon_chaingun") == 0) return true;
	if (strcmp(classname, "weapon_grenadelauncher") == 0) return true;
	if (strcmp(classname, "weapon_rocketlauncher") == 0) return true;
	if (strcmp(classname, "weapon_hyperblaster") == 0) return true;
	if (strcmp(classname, "weapon_railgun") == 0) return true;
	if (strcmp(classname, "weapon_bfg") == 0) return true;

	//Removes power shield from map too
	if (strcmp(classname, "item_power_shield") == 0) return true;

	return false;
}

/*==================================
wf_CanUse

Determine if an item can be used by the player
==================================
*/
qboolean wf_CanUse(gclient_t *cl, edict_t *ent)
{

	if (!ent->item) return true;
	if (!ent->item->flags) return true;
	if (!cl->player_items) return true;

	//Armor
	if (ent->item->flags & IT_ARMOR)
	{
		if ((cl->player_items & ITEM_BODYARMOR) && (ent->item->tag == ARMOR_BODY))
			return(true);
		if ((cl->player_items & ITEM_COMBATARMOR) && (ent->item->tag == ARMOR_COMBAT))
			return(true);
		if ((cl->player_items & ITEM_JACKETARMOR) && (ent->item->tag == ARMOR_JACKET))
			return(true);
		return (false);
	}

	//Ammo
	if (ent->item->flags & IT_AMMO)
	{
		if ((cl->player_ammo & WF_AMMO_BULLETS)  && (ent->item->tag == AMMO_BULLETS))
			return (true);
		if ((cl->player_ammo & WF_AMMO_SHELLS)   && (ent->item->tag == AMMO_SHELLS))
			return (true);
		if ((cl->player_ammo & WF_AMMO_ROCKETS)  && (ent->item->tag == AMMO_ROCKETS))
			return (true);
		if ((cl->player_ammo & WF_AMMO_GRENADES) && (ent->item->tag == AMMO_GRENADES))
			return (true);
		if ((cl->player_ammo & WF_AMMO_CELLS)    && (ent->item->tag == AMMO_CELLS))
			return (true);
		if ((cl->player_ammo & WF_AMMO_SLUGS)    && (ent->item->tag == AMMO_SLUGS))
			return (true);
		return (false);
	}

	//All other items can be picked up
	return (true);
}

void wf_GiveItem(gclient_t *client, char *item_name, int item_count)
{
	gitem_t *item;

	item = FindItem(item_name);
	if (item)
	{
		client->pers.inventory[ITEM_INDEX(item)] = item_count;
//        gi.dprintf ("WF: Gave item: %s\n",item_name);
	}
	else
        gi.dprintf ("WF: Can't give item: %s\n",item_name);
}


void wf_InitPlayerClass(gclient_t *client)
{
	int i;
	int j;
	int curr_weapon;
	gitem_t		*item;

	//Clear grenade count
//	for (i=1; i <= GRENADE_TYPE_COUNT; ++i)
//		client->pers.active_grenades[i] = 0;

	client->player_ammo	= WF_AMMO_SHELLS | WF_AMMO_GRENADES;

	i = client->pers.player_class;
	client->player_armor	= classinfo[i].max_armor;
	client->player_speed	= classinfo[i].max_speed;
	client->player_maxhealth = classinfo[i].max_health;
	client->grenade_type1   = classinfo[i].grenade[0];
	client->grenade_type2   = classinfo[i].grenade[1];
	client->grenade_type3   = classinfo[i].grenade[2];
	client->player_special  = classinfo[i].special;
	client->player_items	= classinfo[i].items;
	client->player_model	= classinfo[i].model;

	//Ammo limits
	client->pers.max_bullets	= classinfo[i].max_ammo[AMMO_BULLETS];
	client->pers.max_shells		= classinfo[i].max_ammo[AMMO_SHELLS];
	client->pers.max_rockets	= classinfo[i].max_ammo[AMMO_ROCKETS];
	client->pers.max_grenades	= classinfo[i].max_ammo[AMMO_GRENADES];
	client->pers.max_cells		= classinfo[i].max_ammo[AMMO_CELLS];
	client->pers.max_slugs		= classinfo[i].max_ammo[AMMO_SLUGS];

	// Set the ammo flags
	if (client->pers.max_bullets)	client->player_ammo	|= WF_AMMO_BULLETS;
	if (client->pers.max_shells)	client->player_ammo	|= WF_AMMO_SHELLS;
	if (client->pers.max_rockets)	client->player_ammo	|= WF_AMMO_ROCKETS;
	if (client->pers.max_grenades)	client->player_ammo	|= WF_AMMO_GRENADES;
	if (client->pers.max_cells)		client->player_ammo	|= WF_AMMO_CELLS;
	if (client->pers.max_slugs)		client->player_ammo	|= WF_AMMO_SLUGS;

//	client->pers.max_health	= client->pers.health;

	//Give everyone a few grenades and set default grenade
	wf_GiveItem(client,"Grenades", 5);
	client->pers.grenade_type = client->grenade_type1;


	//Give weapons and ammo to the player
	for (j = 0; j < 10; ++j)
	{
		curr_weapon = classinfo[i].weapon[j];

		//2
		if (curr_weapon == WEAPON_SHOTGUN)
		{
			wf_GiveItem(client,"Shotgun", 1);
			wf_GiveItem(client,"Shells", 10);
			client->player_ammo	|= WF_AMMO_SHELLS;
		}

		//3
		if (curr_weapon == WEAPON_SUPERSHOTGUN)
		{
			wf_GiveItem(client,"Super Shotgun", 1);
			wf_GiveItem(client,"Shells", 20);
			client->player_ammo	|= WF_AMMO_SHELLS;
		}

		//4
		if (curr_weapon == WEAPON_MACHINEGUN)
		{
			wf_GiveItem(client,"Machinegun", 1);
			wf_GiveItem(client,"Bullets", 50);
			client->player_ammo	|= WF_AMMO_BULLETS;
		}

		//5
		if (curr_weapon == WEAPON_CHAINGUN)
		{
			wf_GiveItem(client,"Chaingun", 1);
			wf_GiveItem(client,"Bullets", 150);
			client->player_ammo	|= WF_AMMO_BULLETS;
		}

		//6
		if (curr_weapon == WEAPON_HYPERBLASTER)
		{
			wf_GiveItem(client,"HyperBlaster", 1);
			wf_GiveItem(client,"Cells", 50);
			client->player_ammo	|= WF_AMMO_CELLS;
		}

		//7
		if (curr_weapon == WEAPON_ROCKETLAUNCHER)
		{
			wf_GiveItem(client,"Rocket Launcher", 1);
			wf_GiveItem(client,"Rockets", 5);
			wf_GiveItem(client,"Cells", 20);
			client->player_ammo	|= WF_AMMO_CELLS;
			client->player_ammo	|= WF_AMMO_ROCKETS;
		}

		//8
		if (curr_weapon == WEAPON_GRENADELAUNCHER)
		{
			wf_GiveItem(client,"Grenade Launcher", 1);
			wf_GiveItem(client,"Grenades", 15);
			client->player_ammo	|= WF_AMMO_GRENADES;
		}

		//9
		if (curr_weapon == WEAPON_RAILGUN)
		{
			wf_GiveItem(client,"Railgun", 1);
			wf_GiveItem(client,"Slugs", 10);
			client->player_ammo	|= WF_AMMO_SLUGS;
		}

		//10
		if (curr_weapon == WEAPON_BFG)
		{
			wf_GiveItem(client,"BFG10K", 1);
			wf_GiveItem(client,"Cells", 150);
			client->player_ammo	|= WF_AMMO_CELLS;
		}

		//New Weapons
		//11
		if (curr_weapon == WEAPON_NEEDLER)
		{
			wf_GiveItem(client,"Needler", 1);
			wf_GiveItem(client,"Bullets", 50);
			client->player_ammo	|= WF_AMMO_BULLETS;
		}

		//12
		if (curr_weapon == WEAPON_NAG)
		{
			wf_GiveItem(client,"Nervous Accelerator Gun", 1);
			wf_GiveItem(client,"Cells", 20);
			client->player_ammo	|= WF_AMMO_CELLS;
		}

		//13
		if (curr_weapon == WEAPON_TELSA)
		{
			wf_GiveItem(client,"Telsa Coil", 1);
			wf_GiveItem(client,"Cells", 50);
			client->player_ammo	|= WF_AMMO_CELLS;
		}

		//14
		if (curr_weapon == WEAPON_LIGHTNING)
		{
			wf_GiveItem(client,"Lightning Gun", 1);
			wf_GiveItem(client,"Slugs", 20);
			client->player_ammo	|= WF_AMMO_SLUGS;
		}

		//15
		if (curr_weapon == WEAPON_NAILGUN)
		{
			wf_GiveItem(client,"NailGun", 1);
			wf_GiveItem(client,"Bullets", 50);
			client->player_ammo	|= WF_AMMO_BULLETS;
		}

		//16
		if (curr_weapon == WEAPON_CLUSTERROCKET)
		{
			wf_GiveItem(client,"Cluster Rocket Launcher", 1);
			wf_GiveItem(client,"Rockets", 5);
			client->player_ammo	|= WF_AMMO_ROCKETS;
		}

		//17
		if (curr_weapon == WEAPON_MAGBOLTED)
		{
			wf_GiveItem(client,"Mag Bolted", 1);
			wf_GiveItem(client,"Bullets", 50);
			client->player_ammo	|= WF_AMMO_BULLETS;
		}

		//18
		if (curr_weapon == WEAPON_PELLET)
		{
			wf_GiveItem(client,"Pellet Rocket Launcher", 1);
			wf_GiveItem(client,"Rockets", 10);
			client->player_ammo	|= WF_AMMO_ROCKETS;
		}

		//19
		if (curr_weapon == WEAPON_PULSE)
		{
			wf_GiveItem(client,"Pulse Cannon", 1);
			wf_GiveItem(client,"Bullets", 50);
			client->player_ammo	|= WF_AMMO_BULLETS;
		}

		//20
		if (curr_weapon == WEAPON_SHC)
		{
			wf_GiveItem(client,"SHC Rifle", 1);
			wf_GiveItem(client,"Shells", 10);
			client->player_ammo	|= WF_AMMO_SHELLS;
		}

		//21
		if (curr_weapon == WEAPON_FLAREGUN)
		{
			wf_GiveItem(client,"Flare Gun", 1);
			wf_GiveItem(client,"Bullets", 50);
			client->player_ammo	|= WF_AMMO_BULLETS;
		}

		//22
		if (curr_weapon == WEAPON_NAPALMMISSLE)
		{
			wf_GiveItem(client,"Rocket Napalm Launcher", 1);
			wf_GiveItem(client,"Rockets", 10);
			client->player_ammo	|= WF_AMMO_ROCKETS;
		}

		//23
		if (curr_weapon == WEAPON_FLAMETHROWER)
		{
			wf_GiveItem(client,"FlameThrower", 1);
			wf_GiveItem(client,"Cells", 50);
			client->player_ammo	|= WF_AMMO_CELLS;
		}

		//24
		if (curr_weapon == WEAPON_TRANQUILIZER)
		{
			wf_GiveItem(client,"Tranquilizer", 1);
			wf_GiveItem(client,"Shells", 10);
			client->player_ammo	|= WF_AMMO_SHELLS;
		}

		//25
		if (curr_weapon == WEAPON_INFECTEDDART)
		{
			wf_GiveItem(client,"Infected Dart Launcher", 1);
			wf_GiveItem(client,"Shells", 10);
			client->player_ammo	|= WF_AMMO_SHELLS;
		}

		//26
		if (curr_weapon == WEAPON_LASERSNIPER)
		{
			wf_GiveItem(client,"Laser Sniper Rifle", 1);
			wf_GiveItem(client,"Slugs", 10);
			client->player_ammo	|= WF_AMMO_SLUGS;
		}

		//27
		if (curr_weapon == WEAPON_ARMORDART)
		{
			wf_GiveItem(client,"Poison Dart Launcher", 1);
			wf_GiveItem(client,"Shells", 10);
			client->player_ammo	|= WF_AMMO_SHELLS;
		}

		//28
		if (curr_weapon == WEAPON_SHOTGUNCHOKE)
		{
			wf_GiveItem(client,"Shotgun Choke", 1);
			wf_GiveItem(client,"Shells", 10);
			client->player_ammo	|= WF_AMMO_SHELLS;
		}

		//29
		if (curr_weapon == WEAPON_SNIPERRIFLE)
		{
			wf_GiveItem(client,"Sniper Rifle", 1);
			wf_GiveItem(client,"Slugs", 10);
			client->player_ammo	|= WF_AMMO_SLUGS;

			//Turn on sniping flag for this weapon
			client->player_special = client->player_special | SPECIAL_SNIPING;
		}

		//30
		if (curr_weapon == WEAPON_LRPROJECTILE)
		{
			wf_GiveItem(client,"Projectile Launcher", 1);
			wf_GiveItem(client,"Rockets", 10);
			client->player_ammo	|= WF_AMMO_ROCKETS;
		}

		//31
		if (curr_weapon == WEAPON_SENTRYKILLER)
		{
			wf_GiveItem(client,"sentry killer", 1);
			wf_GiveItem(client,"rockets", 25);
			client->player_ammo	|= WF_AMMO_ROCKETS;
		}

		//32
		if (curr_weapon == WEAPON_MEGACHAINGUN)
		{
			wf_GiveItem(client,"Mega Chaingun", 1);
			wf_GiveItem(client,"Bullets", 250);
			client->player_ammo	|= WF_AMMO_BULLETS;
		}

		//33
		if (curr_weapon == WEAPON_TRANQUILDART)
		{
			wf_GiveItem(client,"Tranquilizer Dart Launcher", 1);
			wf_GiveItem(client,"Shells", 10);
			client->player_ammo	|= WF_AMMO_SHELLS;
		}

		//34
		if (curr_weapon == WEAPON_KNIFE)
		{
			wf_GiveItem(client,"Knife", 1);
			wf_GiveItem(client,"Shells", 40);
			client->player_ammo	|= WF_AMMO_SHELLS;
		}

		//35
		if (curr_weapon == WEAPON_AK47)
		{
			wf_GiveItem(client,"AK47", 1);
			wf_GiveItem(client,"Bullets", 50);
			client->player_ammo	|= WF_AMMO_BULLETS;
		}

		//36
		if (curr_weapon == WEAPON_PISTOL)
		{
			wf_GiveItem(client,"Pistol", 1);
			wf_GiveItem(client,"Bullets", 50);
			client->player_ammo	|= WF_AMMO_BULLETS;
		}

		//37
		if (curr_weapon == WEAPON_STINGER)
		{
			wf_GiveItem(client,"Stinger Launcher", 1);
			wf_GiveItem(client,"Rockets", 20);
			client->player_ammo	|= WF_AMMO_ROCKETS;
		}

		//38
		if (curr_weapon == WEAPON_DISRUPTOR)
		{
			wf_GiveItem(client,"Disruptor", 1);
			wf_GiveItem(client,"Cells", 10);
			client->player_ammo	|= WF_AMMO_CELLS;
		}

		//39
		if (curr_weapon == WEAPON_ETF_RIFLE)
		{
			wf_GiveItem(client,"ETF Rifle", 1);
			wf_GiveItem(client,"Slugs", 10);
			client->player_ammo	|= WF_AMMO_SLUGS;
		}


		//40
		if (curr_weapon == WEAPON_PLASMA_BEAM)
		{
			wf_GiveItem(client,"Plasma Beam", 1);
			wf_GiveItem(client,"Cells", 15);
			client->player_ammo	|= WF_AMMO_CELLS;
		}

		//41
		if (curr_weapon == WEAPON_ION_RIPPER)
		{
			wf_GiveItem(client,"Ionripper", 1);
			wf_GiveItem(client,"Cells", 20);
			client->player_ammo	|= WF_AMMO_CELLS;
		}

		//42
		if (curr_weapon == WEAPON_PHALANX)
		{
			wf_GiveItem(client,"Phalanx", 1);
			wf_GiveItem(client,"Slugs", 10);
			client->player_ammo	|= WF_AMMO_SLUGS;
		}
       //43 acrid 3/99
		if (curr_weapon == WEAPON_FREEZER)
		{
			wf_GiveItem(client,"Freezer", 1);
			wf_GiveItem(client,"Slugs", 10);
			client->player_ammo	|= WF_AMMO_SLUGS;
		}
	}

	//Special items require ammo
	if (client->player_special & SPECIAL_SUPPLY_DEPOT)
	{
		wf_GiveItem(client,"Rockets", 10);
		client->player_ammo	|= WF_AMMO_ROCKETS;
		wf_GiveItem(client,"Slugs", 10);
		client->player_ammo	|= WF_AMMO_SLUGS;
	}
	if (client->player_special & SPECIAL_SENTRY_GUN)
	{
		wf_GiveItem(client,"Bullets", 100);
		client->player_ammo	|= WF_AMMO_BULLETS;
	}

	if (client->player_special & SPECIAL_BIOSENTRY)
	{
		wf_GiveItem(client,"Bullets", 100);
		client->player_ammo	|= WF_AMMO_BULLETS;
	}

	if (client->player_special & SPECIAL_REMOTE_CAMERA)
	{
		wf_GiveItem(client,"Cells", 25);
		client->player_ammo	|= WF_AMMO_CELLS;
	}


	//Don't give grapple to everyone
	if (((int)wfflags->value & WF_NO_GRAPPLE) == 0) 
		{
	
		item = FindItem("Grapple");

		//Is this class capable of using the grapple?
		if (client->player_special & SPECIAL_GRAPPLE)
		{
			client->pers.inventory[ITEM_INDEX(item)] = 1;
		}
		else
		{
			client->pers.inventory[ITEM_INDEX(item)] = 0;
		}
	}


	//Some grenades need other ammo
	if (client->grenade_type1 == GRENADE_TYPE_TURRET || client->grenade_type2 == GRENADE_TYPE_TURRET || client->grenade_type3 == GRENADE_TYPE_TURRET)
	{
		wf_GiveItem(client,"Slugs", 10);
		client->player_ammo	|= WF_AMMO_SLUGS;
	}

	if (client->grenade_type1 == GRENADE_TYPE_LASERBALL || client->grenade_type2 == GRENADE_TYPE_LASERBALL || client->grenade_type3 == GRENADE_TYPE_LASERBALL)
	{
		wf_GiveItem(client,"Cells", 10);
		client->player_ammo	|= WF_AMMO_CELLS;
	}


	//Give special items
	if (client->player_items & ITEM_REBREATHER)
	{
		wf_GiveItem(client,"Rebreather", 1);
		//client->breather_framenum = level.framenum + 999;
	}

/* Don't give out armor.  Let them pick it up
	if (client->player_items & ITEM_BODYARMOR)
		wf_GiveItem(client,"Body Armor", 1);

	if (client->player_items & ITEM_COMBATARMOR)
		wf_GiveItem(client,"Combat Armor", 1);

	if (client->player_items & ITEM_JACKETARMOR)
		wf_GiveItem(client,"Jacket Armor", 1);

*/
	if (client->player_items & ITEM_POWERSCREEN)
		wf_GiveItem(client,"Power Screen", 1);

	if (client->player_items & ITEM_POWERSHIELD)
		wf_GiveItem(client,"Power Shield", 1);

	if (client->player_items & ITEM_SILENCER)
	{
		client->silencer_shots = 999;
		wf_GiveItem(client,"Silencer", 1);
	}

	if (client->player_special & SPECIAL_HEALING)
		wf_GiveItem(client,"AutoDoc", 1);


	
}




/*
=================
Cmd_DSkin_f

Decoy Skin Commands
=================
*/
void Cmd_DSkin_f (edict_t *ent)
{
        char    *string;

        string=gi.args();

        if (Q_stricmp ( string, "0") == 0)
        {
            if (ent->decoy)
               ent->decoy->s.skinnum = 0;
        }
        if (Q_stricmp ( string, "1") == 0)
        {
            if (ent->decoy)
               ent->decoy->s.skinnum = 1;
        }
        if (Q_stricmp ( string, "2") == 0)
        {
            if (ent->decoy)
               ent->decoy->s.skinnum = 2;
        }
        if (Q_stricmp ( string, "3") == 0)
        {
            if (ent->decoy)
               ent->decoy->s.skinnum = 3;
        }
        if (Q_stricmp ( string, "0") == 0)
        {
            if (ent->decoy)
               ent->decoy->s.skinnum = 0;
        }

}
char getClassCode (gclient_t *c)
{
	if (!c) return '?';

	//Return first letter of the class name
	return (classinfo[c->pers.player_class].name[0]);
}

char *getClassName (gclient_t *c)
{

	if (!c) 
		return "(unknown)";

	return (classinfo[c->pers.player_class].name);
}

char *getNextClassName (gclient_t *c)
{

	if (!c) 
		return "(unknown)";

	return (classinfo[c->pers.next_player_class].name);
}



/*
=================
Cmd_ShowClass
=================
*/
void Cmd_ShowClass (edict_t *ent)
{

	if (ent->client->pers.player_class)
		safe_cprintf (ent, PRINT_HIGH, "You are a %s\n", classinfo[ent->client->pers.player_class].name);
	else
		safe_cprintf (ent, PRINT_HIGH, "No class selected\n");
}

//Show all the players
void Cmd_ShowPlayers(edict_t *ent)
{
//	edict_t		*cl_ent;
//	gclient_t	*cl;
	edict_t	*e;

	int i;
	char classname[32];

	strcpy(classname,"123");

	//Show red players first
	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		//getClassName(e->client, classname);
		if (e->client->resp.ctf_team == CTF_TEAM1)
		{
			safe_cprintf (ent, PRINT_HIGH, "Red  %3d %8s %s\n",
				e->client->resp.score,
				getClassName(e->client),
				e->client->pers.netname);
		}
	}

	//Next blue
	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		//getClassName(e->client, classname);
		if (e->client->resp.ctf_team == CTF_TEAM2)
		{
			safe_cprintf (ent, PRINT_HIGH, "Blue %3d %8s %s\n",
				e->client->resp.score,
				getClassName(e->client),
				e->client->pers.netname);
		}
	}

	//Then observers
	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		//getClassName(e->client, classname);
		if (e->client->resp.ctf_team <= 0)
		{
			safe_cprintf (ent, PRINT_HIGH, "Obsv %3d %8s %s\n",
				e->client->resp.score,
				getClassName(e->client),
				e->client->pers.netname);
		}
	}
}

//Play a sound for a team.  If the "all" argument is set, play it for all teams
void Cmd_WFPlayTeam (edict_t *self, char *wavename, int all)
{
	int		i;
	edict_t	*e;
	char cmd[250];

	if (wfdebug) gi.dprintf("Before: %s\n",wavename);

	for (i = 0; wavename[i] != 0 && i < 200; ++i)
	{
		if ((wavename[i] == ';')||(wavename[i] == '%')) wavename[i] = 0;
	}

	if (strlen(wavename) > 200) wavename[200] = '\0';
	sprintf(cmd, "play %s\n",wavename);

	if (wfdebug) gi.dprintf("After: %s\n",wavename);

	//Loop through all the entities to find players on the same team
	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
        if (e->bot_client)
			continue;
		if (e->client->pers.nospam_level & SPAM_NOTEAMSOUND)
			continue;	//they don't want team sounds

		if ((e->wf_team == self->wf_team) || (all))
		{
			stuffcmd(e,cmd);
			if ((e->client->pers.nospam_level & SPAM_NOTEAMSOUNDTEXT) == 0)
				safe_cprintf (e, PRINT_HIGH, "From %s: %s\n", self->client->pers.netname, wavename);
		}
	}
}

//When player dies, clean up entities
void WFPlayer_Die (edict_t *ent)
{
	edict_t *blip = NULL;
	int i;

	if (!ent) return;

	HealPlayer(ent);

	ent->disguised=0;
    ent->client->pers.feign = 0;//acrid

	if (ent->client->menu)//acrid 3/99 
		PMenu_Close(ent);

	//WF - Scanner
//	ClearScanner(ent->client);

//WF JR concussion grenades

	lasersight_off (ent);

	//See if mercenary team should change
	if (ent->client->player_special & SPECIAL_MERCENARY)
		ent->client->resp.ctf_team = get_mercenary_team(ent);

	for (i=1, blip=g_edicts+i ; i < globals.num_edicts ; i++,blip++)
	{
 	  if (blip->owner == ent)
	  {

		//Flames
		if (!strcmp(blip->classname, "fire") )
		{
			//Remove_Flame(blip);
			blip->dmg = 0;
		}

		//Pipebombs
                else if (!strcmp(blip->classname, "pipebomb") )
		{
			blip->think = Pipebomb_Explode;
			blip->nextthink = level.time + .1;
		}

		//Turret grenades
                else if (!strcmp(blip->classname, "turret") )
		{
			blip->think=Turret_Explode;	//Drops to the ground and explodes
			blip->nextthink=level.time+2;	//Looks better than just disappearing
			blip->movetype=MOVETYPE_BOUNCE;
			blip->s.effects = EF_GRENADE;  //Lights out!!!
			blip->s.renderfx = 0;	      //Goodbye shell :(
		}

	  }
	}
}

void AutoClassConfig(edict_t *ent)
{
	char exec_str[100];

	//See if we should auto exec a class file
	if (ent->client->pers.autoconfig) 
	{
		sprintf(exec_str, "exec %s.cfg\n", getNextClassName(ent->client));
		stuffcmd(ent, exec_str);
	}
}

//If player changes class or team
void WFPlayer_ChangeClassTeam (edict_t *ent)
{
	edict_t *blip = NULL;
	int i;

//if (wfdebug)
//{
//	gi.dprintf(	"Curr Class = %d, Next Class = %d\n",
//		ent->client->player_class, ent->client->resp.next_player_class);
//
//}

//gi.dprintf("ChangeClass\n");
	lasersight_off (ent);
	ent->disguised=0;

	if (ent->remotecam) remote_remove(ent->remotecam);

        //Turn off anti-grav boots
        if (ent->flags & FL_BOOTS) ent->flags -= FL_BOOTS;

	for (i=1, blip=g_edicts+i ; i < globals.num_edicts ; i++,blip++)
	{
 	  if ((blip->owner) && (blip->owner == ent))
	  {

		//Supply depot
		if (!strcmp(blip->classname, "depot") )
		{
			if (blip->owner) blip->owner->supply = NULL;
			G_FreeEdict(blip);
		}

		//Healing depot
                else if (!strcmp(blip->classname, "healingdepot") )
		{
			if (blip->owner) blip->owner->supply = NULL;
			G_FreeEdict(blip);
		}

		//Goodyear grenades
                else if (!strcmp(blip->classname, "goodyear") )
		{
			blip->think = Goodyear_Explode;
			blip->nextthink = level.time + .1;
		}

		//Proximity grenades
                else if (!strcmp(blip->classname, "proximity") )
		{
			blip->think = Proximity_Explode;
			blip->nextthink = level.time + .1;
		}


		//Alarms
                else if (!strcmp(blip->classname, "Alarm") )
		{
			alarm_remove(blip);
		}

		//Laser defense
                else if (!strcmp(blip->classname, "laser_defense_gr") )
		{
			laser_defense_die(blip, NULL, NULL, 0, NULL);
		}
	  }

	  //These items use creator instead of owner
	  //so that you can't walk through them
 	  if ((blip->creator) && (blip->creator == ent))
	  {
		//Sentry guns
		if (!strcmp(blip->classname, "SentryGun") )
		{
			turret_remove(blip->creator);
		}

		//BioSentry
		if (!strcmp(blip->classname, "biosentry") )
		{
			Biosentry_remove(blip->creator);
		}

		//Missile turrets
		if (!strcmp(blip->classname, "MissileTurret") )
		{
			mturret_remove(blip->creator);
		}
	  }
	}

	AutoClassConfig(ent);
}

#define BONUSTYPE_CAPTURE		1
#define BONUSTYPE_PLAYERFRAG	2
#define BONUSTYPE_TEAMFRAG		3

//Some map entities can award a bonus
void WFAddBonus(edict_t *self, edict_t *other)
{
	int bonusval;

	//Like they captured the flag
	if (self->bonustype == BONUSTYPE_CAPTURE)
	{
		gi.bprintf(PRINT_HIGH, "%s got the capture bonus!\n",other->client->pers.netname);

		// Log Flag Capture - MarkDavies
		sl_LogScore( &gi, other->client->pers.netname,	NULL, "F Capture", -1, 1);	//One point awarded

		// Log frag bonus
		if (self->bonusvalue)
			bonusval = self->bonusvalue;
		else
			bonusval = 1;

		if (other->wf_team == CTF_TEAM1)
			ctfgame.team1 += bonusval;
		else
			ctfgame.team2 += bonusval;

		gi.sound (self, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagcap.wav"), 1, ATTN_NONE, 0);
	}

	//Give the person a frag
	else if (self->bonustype == BONUSTYPE_PLAYERFRAG)
	{
		gi.bprintf(PRINT_HIGH, "%s got the frag bonus!\n", other->client->pers.netname);

		// Log frag bonus
		if (self->bonusvalue)
			other->client->resp.score += self->bonusvalue;
		else
			other->client->resp.score++;

		sl_LogScore( &gi, other->client->pers.netname, NULL, "Kill",0,1 );	// StdLog - Mark Davies

		gi.sound (self, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagcap.wav"), 1, ATTN_NONE, 0);
	}

	//Give the team a frag
	else if (self->bonustype == BONUSTYPE_TEAMFRAG)
	{
		//yet to do
	}
}
