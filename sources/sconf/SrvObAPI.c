/* ServObit API to rest of the game.  

  The API is separated from ServObit's core functionality to allow
  the test program to run without the game at all.
  */

#include "g_local.h"
#include "gender.h"
#include "ServObits.h"
#include "SrvObAPI.h"
#include "ModUtils.h"
#include "motd.h"
#include "s_readconfig.h"
// SCONFIG START
void logFrag( char *frager, char *fragee ); // p_client reference
// SCONFIG STOP

/* *******************************
   External Declarations
   ******************************* */

extern void Cmd_Score_f(edict_t *ent);
extern void Cmd_Inven_f(edict_t *ent);


/* **************************************************************
   Player Characteristics
   ************************************************************** */

int GetPlayerBodyState (edict_t *ent)
{
	//gi.dprintf("Ent %s health: %d\n", ent->client->pers.netname, ent->health);
	if (ent->health < ServObit.MegaGib)
		return(SO_BODY_STATE_MEGAGIB);
	else if (ent->health < -40)  // Hard coded because id hard coded it
		return(SO_BODY_STATE_GIB);
	else return(SO_BODY_STATE_NORMAL);
}

int PlayerHasQuadDamage (edict_t *entity)
{
	if(entity->client->quad_framenum > level.framenum)
		return 1;
	else return 0;
}


/* **************************************************************
   Means Of Death
   ************************************************************** */

/* idsoftware's Means of death variable is not sufficient for ServObit because
there are other "generic" specifiers for means of death, plus I wanted
to be able to quickly determine whether a means of death was environment or
weapon, so I use _END and _START constants to mark those.   So I make up 
a map from idsoftware's means of death to ServObit's.
*/

int ConvertMeansOfDeath (int mod)
{
	if (mod == MOD_UNKNOWN) 
		return(0);
	else if (mod == MOD_BLASTER)
		return(SO_WEAPON_BLASTER);
	else if (mod == MOD_SHOTGUN)
		return(SO_WEAPON_SHOTGUN);
	else if (mod == MOD_SSHOTGUN)
		return(SO_WEAPON_SUPER_SHOTGUN);
	else if (mod == MOD_MACHINEGUN)
		return(SO_WEAPON_MACHINEGUN);
	else if (mod == MOD_CHAINGUN)
		return(SO_WEAPON_CHAINGUN);
	else if (mod == MOD_GRENADE)
		return(SO_WEAPON_GRENADE_LAUNCHER);
	else if (mod == MOD_G_SPLASH)
		return(SO_WEAPON_GRENADE_SPLASH);
	else if (mod == MOD_ROCKET)
		return(SO_WEAPON_ROCKET_LAUNCHER);
	else if (mod == MOD_R_SPLASH)
		return(SO_WEAPON_ROCKET_LAUNCHER_SPLASH);
	else if (mod == MOD_HYPERBLASTER)
		return(SO_WEAPON_HYPER_BLASTER);
	else if (mod == MOD_RAILGUN)
		return(SO_WEAPON_RAILGUN);
	else if (mod == MOD_BFG_LASER)
		return(SO_WEAPON_BFG_LASER);
	else if (mod == MOD_BFG_BLAST)
		return(SO_WEAPON_BFG_BLAST);
	else if (mod == MOD_BFG_EFFECT)
		return(SO_WEAPON_BFG_EFFECT);
	else if (mod == MOD_HANDGRENADE)
		return(SO_WEAPON_HAND_GRENADE);
	else if (mod == MOD_HG_SPLASH)
		return(SO_WEAPON_HAND_GRENADE_SPLASH);
	else if (mod == MOD_WATER)
		return(SO_ENV_WATER);
	else if (mod == MOD_SLIME)
		return(SO_ENV_SLIME);
	else if (mod == MOD_LAVA)
		return(SO_ENV_LAVA);
	else if (mod == MOD_CRUSH)
		return(SO_ENV_SQUISH);
	else if (mod == MOD_TELEFRAG)
		return(SO_WEAPON_TELEFRAG);
	else if (mod == MOD_FALLING)
		return(SO_ENV_FALL);
	else if (mod == MOD_SUICIDE)
		return(SO_WEAPON_SUICIDE);
	else if (mod == MOD_HELD_GRENADE)
		return(SO_WEAPON_HAND_GRENADE_HELD);
	else if (mod == MOD_EXPLOSIVE)
		return(SO_ENV_TOUCH);
	else if (mod == MOD_BARREL)
		return(SO_ENV_TOUCH);
	else if (mod == MOD_BOMB)
		return(SO_ENV_TOUCH);
	else if (mod == MOD_EXIT)
		return(SO_ENV_EXIT);
	else if (mod == MOD_SPLASH)
		return(SO_ENV_TOUCH);
	else if (mod == MOD_TARGET_LASER)
		return(SO_ENV_LASER);
	else if (mod == MOD_TRIGGER_HURT)
		return(SO_ENV_TOUCH);
	else if (mod == MOD_HIT)
		return(SO_ENV_TOUCH);
	else if (mod == MOD_TARGET_BLASTER)
		return(SO_ENV_TOUCH);
	else return(0);
}

int StripMeansOfDeath (int mod)
{
	return (mod & ~MOD_FRIENDLY_FIRE);
}

int MeansOfDeathToServObitDeath (int mod)
{
	mod = StripMeansOfDeath(mod);
	if (mod < 0) return(0);
	else if (mod > ServObit.MaxMeansOfDeath)
		return(0);
	else return(ServObit.MeansOfDeathMap[mod]);
}

void InitServObitMeansOfDeathMap()
{
	int i;

	// ***** MaxMeansOfDeath should be the maximum value for the means of death
	// as specified by the game (ignoring friendly fire and other bitmasks).
	// If you modify the means of death variable, then modify this too.

	ServObit.MaxMeansOfDeath = MOD_TARGET_BLASTER;
	ServObit.MeansOfDeathMap = (int *) malloc(sizeof (int) * 
											   (ServObit.MaxMeansOfDeath + 1));

	for (i=0; i<ServObit.MaxMeansOfDeath; i++)
	{
		ServObit.MeansOfDeathMap[i] = ConvertMeansOfDeath(i);
	}
}

/* **************************************************************
   Obituaries, Connect/Disconnect Messages
   ************************************************************** */

int ServObitClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int quad, wtype, vgender, kgender, bodystate;
	char *self_name, *attacker_name;
	qboolean	ff;

	if (! (deathmatch->value || coop->value))
		return(0);

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	//gi.dprintf("Means of death: %d\n", meansOfDeath);

	ff = meansOfDeath & MOD_FRIENDLY_FIRE;

	self_name = self->client->pers.netname;
	vgender = GetPlayerGender(self);
	bodystate = GetPlayerBodyState(self);

	wtype = MeansOfDeathToServObitDeath (meansOfDeath);

	if (wtype == 0) return(0); // some death we didn't check for?

	//gi.dprintf("**** bodystate: %d; health = %d\n", bodystate, ent->health);
	//wtype = Get_314_Environment_Death_Type(self, inflictor, attacker);

	// Environmental "weapon"
	if (IsEnvironmentWeapon(wtype))
	{
		if (PlayerHasQuadDamage(self))
			quad = SO_POWER_QUAD;
		else quad = SO_POWER_NORMAL;
		DisplayObituary(self_name, "", SO_KILLER_ENVIRONMENT, vgender, vgender,
						wtype, quad, bodystate);
		self->client->resp.score--;
		// SCONFIG START (record lava, etc...)
		if (QWLOG == 1)
			logFrag(self->client->pers.netname,self->client->pers.netname);
		// SCONFIG STOP
		
		return(1);
	}

	// Suicide
	if (attacker == self)
	{
		if (PlayerHasQuadDamage(self))
			quad = SO_POWER_QUAD;
		else quad = SO_POWER_NORMAL;
		DisplayObituary(self_name, self_name, SO_KILLER_SELF, vgender, vgender,
			wtype, quad, bodystate);

		if (deathmatch->value) {
			self->client->resp.score--;
			// SCONFIG START
			if (QWLOG == 1)
				logFrag(self->client->pers.netname,self->client->pers.netname);
			// SCONFIG STOP
		}
		self->enemy = NULL;
		return(1);
	}

	// Killed by enemy
	// wtype = Get_314_Killer_Death_Type(self, inflictor, attacker);

	self->enemy = attacker;
	if (attacker && attacker->client)
	{
		attacker_name = attacker->client->pers.netname;
		if (PlayerHasQuadDamage(attacker))
			quad = SO_POWER_QUAD;
		else quad = SO_POWER_NORMAL;

		kgender = GetPlayerGender(attacker);
		DisplayObituary(self_name, attacker_name, SO_KILLER_ENEMY, kgender, vgender,
						wtype, quad, bodystate);
		if (ff) {
			attacker->client->resp.score--;
			// SCONFIG START
			if (QWLOG == 1)
		 	  logFrag(attacker->client->pers.netname, self->client->pers.netname);
			// SCONFIG STOP
		} else {
			attacker->client->resp.score++;
			if (QWLOG == 1)
			  logFrag(attacker->client->pers.netname, self->client->pers.netname);
			// SCONFIG STOP
		}
		return(1);
	}

	// Hell if I know, he's just dead!  (saw that in Quake I source code)
	return(0);
}

void ServObitPrintWelcome (edict_t *ent)
{
//	char mess[MAX_STRING_CHARS];
//
//	if (strlen(ServObit.Welcome))
//	{
//		strcpy(mess, ServObit.Welcome);
//		PerformSubstitutions(mess);
//
//		ServObitInsertValue(mess, ServObit.Welcome, "$N", ent->client->pers.netname, 
//							MAX_STRING_CHARS);
		// ServObit 1.3: change to timed message
		//gi.centerprintf(ent, mess);
		
//		FormatBoldString(mess);
//		DisplayTimedMessage(ent, mess, ServObit.WelcomeDisplayTime);
//	}
}

void ServObitAnnounceConnect (edict_t *ent)
{
//	char format_string[MAX_STRING_CHARS], mess[MAX_STRING_CHARS];
//
//	if (ServObit.ConnectMessageCount > 0)
//		strcpy(format_string, ServObit.ConnectMessages[rand() % ServObit.ConnectMessageCount]);
//	else strcpy(format_string, "$N entered the game\n");
//	
//	FormatConnectMessage(format_string, mess, ent->client->pers.netname, 
//						 GetPlayerGender(ent));
//	// Since mess can have % in it, don't print as a format string to printf ;-)
//	gi.bprintf(PRINT_MEDIUM, "%s\n", mess);
}


void ServObitAnnounceDisconnect (edict_t *ent)
{
	char format_string[MAX_STRING_CHARS], mess[MAX_STRING_CHARS];

	// Thanks to Eradicator (aka Derek Westcott) for finding the bug!  I wasn't
	// copying the original format string so it was getting changed dynamically.
	// Plain stupid :-(   Not a good thing for supposedly read-only disconnect
	// messages!
	if (ServObit.DisconnectMessageCount > 0)
		strcpy(format_string, ServObit.DisconnectMessages[rand() % ServObit.DisconnectMessageCount]);
	else strcpy(format_string, "$N disconnected\n");
	
	FormatConnectMessage(format_string, mess, ent->client->pers.netname,
						 GetPlayerGender(ent));

	// Since mess can have % in it, don't print as a format string to printf ;-)
	gi.bprintf(PRINT_MEDIUM, "%s\n", mess);
}

/* **************************************************************
   User Commands
   ************************************************************** */

void ServObit_Cmd_Help_f (edict_t *ent)
{
	if (GetTimedMessage(ent) != NULL)
		StopTimedMessage(ent);
	else
		Cmd_Help_f(ent);
}

void ServObit_Cmd_Score_f (edict_t *ent)
{
	if (GetTimedMessage(ent) != NULL)
		StopTimedMessage(ent);
	else
		Cmd_Score_f(ent);
}

void ServObit_Cmd_Inven_f (edict_t *ent)
{
	if (GetTimedMessage(ent) != NULL)
		StopTimedMessage(ent);
	else
		Cmd_Inven_f(ent);
}

/* **************************************************************
   Initialization
   ************************************************************** */

void ServObitInitGame ()
{
	InitializeServObit();
	InitServObitMeansOfDeathMap();
}
