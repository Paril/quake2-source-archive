/*
 *  GibStats Logging functions
 *
 *  Public header file.
 *
 * $Id: gslog.c,v 1.9 2000/02/22 00:49:02 fpeeler Exp $
 *
 */

#include <stdlib.h>
#include <time.h>

#include "g_local.h"

#include "stdlog.h"


/*
 *  PUBLIC FUNCTIONS
 *
 *
 */

static int fWasAlreadyOpen = 0;
static char     *pPatch     = NULL;     /* PatchName - Should never change */

int sl_Logging( game_import_t  *gi,
                char           *pPatchName )
{
    int fFileOpen = sl_OpenLogFile( gi );
	
    if( fFileOpen && !fWasAlreadyOpen )
    {
        cvar_t *deathflags = gi->cvar( "dmflags", "0", CVAR_SERVERINFO );

        sl_LogVers( gi );
        
        pPatch = pPatchName;
        sl_LogPatch( gi, pPatchName );

        sl_LogDate( gi );
        sl_LogTime( gi );
        sl_LogDeathFlags( gi, (unsigned long)deathflags->value );

        fWasAlreadyOpen = fFileOpen;
    }

    return fFileOpen;
}

void sl_GameStart( game_import_t    *gi,
                   level_locals_t    level )
{
    if( sl_Logging( gi, pPatch ) )
    {
        // log name of map
        sl_LogMapName( gi, level.level_name );

        // start counting frags
        sl_LogGameStart( gi, level.time );
    }
}

void sl_GameEnd( game_import_t      *gi,
                 level_locals_t      level )
{
    if( sl_Logging( gi, pPatch ) )
    {
        sl_LogGameEnd( gi, level.time );
        sl_CloseLogFile();

        fWasAlreadyOpen = 0;
    }   
}


void sl_WriteStdLogDeath( game_import_t     *gi,
                          level_locals_t     level,
                          edict_t           *self,
                          edict_t           *inflictor,
                          edict_t           *attacker )
{
    /* StdLogging for Deathmatch only */
    if( deathmatch->value )
    {
        if( sl_Logging( gi, pPatch ) )
        {
            int			mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
            char       *pKillerName =   NULL;
            char       *pTargetName =   NULL;
            char       *pScoreType  =   NULL; /* Kill, Suicde */
            char       *pWeaponName =   NULL;
            int        iScore       =   0;    /* 1,    -1 */


            if( attacker == self )
            {
                /* Suicide - weapon */
                pKillerName = self->client->pers.netname;
                pScoreType  = "Suicide";
                iScore      = -1;

                /* Get weapon name, being very careful - mdavies */
                /* This weapon may not have been the weapon used if the weapon was changed before the death */

                 switch (mod)  {
		   case MOD_VORTEX:  {
		     pWeaponName = "Vortex";
                     break;
                    }
		   case MOD_TURRET:  {
		     pWeaponName = "Defence_Turret";
                     break;
                    }
		 case MOD_KAMIKAZE: {
                      pWeaponName = "Kamikaze";
                      break;
		 }
		   default : {
		     pWeaponName = (NULL != attacker->client->pers.weapon)?(attacker->client->pers.weapon->pickup_name):(NULL); 
		   }
		 }
            }
            else
            {
                int fSuicide = 0;
                
                /* Suicide - no weapon */
                switch (mod)
                {
                    case MOD_FALLING:
                    {
                        pWeaponName = "Fell";
                        fSuicide = 1;
                        break;
                    }
                    case MOD_CRUSH:
                    {
                        pWeaponName = "Crushed";
                        fSuicide = 1;
                        break;
                    }
                    case MOD_WATER:
                    {
                        pWeaponName = "Drowned";
                        fSuicide = 1;
                        break;
                    }
                    case MOD_SLIME:
                    {
                        pWeaponName = "Melted";
                        fSuicide = 1;
                        break;
                    }
                    case MOD_LAVA:
                    {
                        pWeaponName = "Lava";
                        fSuicide = 1;
                        break;
                    }
                    case MOD_BOMB:
                    case MOD_EXPLOSIVE:
                    case MOD_BARREL:
                    {
                        pWeaponName = "Explosion";
                        fSuicide = 1;
                        break;
                    }
                    case MOD_TARGET_BLASTER:
                    {
                        pWeaponName = "Blasted";
                        fSuicide = 1;
                        break;
                    }
                    case MOD_SPLASH:
                    case MOD_TRIGGER_HURT:
                    case MOD_EXIT:
                    case MOD_SUICIDE:
                    {
                        fSuicide = 1;
                        break;
                    }
                }

                if( fSuicide )
                {
                    pKillerName = self->client->pers.netname;
                    pScoreType = "Suicide";
                    iScore     = -1;
                }
                    
            }

            if( (NULL == pKillerName) || (NULL == pScoreType) )
            {
                /* Kills */

                // self->enemy = attacker;
                if( attacker && attacker->client )
                {
                    switch (mod)
                    {
		    case MOD_PGRENADE: {
                            pTargetName = self->client->pers.netname;
                            pKillerName = attacker->owner->client->pers.netname;
                            pScoreType  = "Kill";
                            iScore      = 1;
                            pWeaponName = "Poison_Grenade";
                            break;
		    }
		    case MOD_VORTEX: {
                            pTargetName = self->client->pers.netname;
                            pKillerName = attacker->client->pers.netname;
                            pScoreType  = "Kill";
                            iScore      = 1;
                            pWeaponName = "Vortex";
                            break;
		    }
		    case MOD_TURRET: {
                            pTargetName = self->client->pers.netname;
                            pKillerName = attacker->client->pers.netname;
                            pScoreType  = "Kill";
                            iScore      = 1;
                            pWeaponName = "Defence_Turret";
                            break;
		    }
		    case MOD_KAMIKAZE: {
                            pTargetName = self->client->pers.netname;
                            pKillerName = attacker->client->pers.netname;
                            pScoreType  = "Kill";
                            iScore      = 1;
                            pWeaponName = "Kamikaze";
                            break;
                    }
                        case MOD_BLASTER:
                        case MOD_SHOTGUN:
                        case MOD_SSHOTGUN:
                        case MOD_MACHINEGUN:
                        case MOD_CHAINGUN:
                        case MOD_GRENADE:
                        case MOD_G_SPLASH:
                        case MOD_ROCKET:
                        case MOD_R_SPLASH:
                        case MOD_HYPERBLASTER:
                        case MOD_RAILGUN:
                        case MOD_BFG_LASER:
                        case MOD_BFG_BLAST:
                        case MOD_BFG_EFFECT:
                        case MOD_HANDGRENADE:
                        case MOD_HG_SPLASH:
                        case MOD_HELD_GRENADE:
			case MOD_ARROW:
			case MOD_PARROW:
			case MOD_ESSHOTGUN:
		        case MOD_PROXYMINE:
			case MOD_AIRFIST:
			case MOD_HOMING:
			case MOD_BUZZ:
			case MOD_SWORD:
			case MOD_CHAINSAW:
			case MOD_AK42:
                        default:
                        {
                            /* Kill - weapon */
                            pTargetName = self->client->pers.netname;
                            pKillerName = attacker->client->pers.netname;
                            pScoreType  = "Kill";
                            iScore      = 1;

                            /* Get weapon name, being very careful - mdavies */
                            /* This weapon may not have been the weapon used if the weapon was changed before the death */
                            pWeaponName = (NULL != attacker->client->pers.weapon)?(attacker->client->pers.weapon->pickup_name):(NULL);
                            break;
                        }

                        case MOD_TELEFRAG:
                        {
                            /* Kill - weapon */
                            pTargetName = self->client->pers.netname;
                            pKillerName = attacker->client->pers.netname;
                            pScoreType  = "Kill";
                            iScore      = 1;

                            /* Set weapon name - mdavies */
                            pWeaponName = "Telefrag";
                            break;
                        }                            
                    }
                }
                else {
                      if (attacker && attacker->owner && attacker->owner->client)
			  {
                             if (Q_strncasecmp(attacker->classname, "rocket_turret", 13) == 0)
			       {
				 if (attacker->owner == self) {
				   pKillerName = self->client->pers.netname;
				   pScoreType  = "Suicide";
				   iScore      = -1;
                                   pWeaponName = "Rocket_Turret";
				 }
                                 else {
				   pTargetName = self->client->pers.netname;
				   pKillerName = attacker->owner->client->pers.netname;
				   pScoreType  = "Kill";
				   iScore      = 1;
                                   pWeaponName = "Rocket_Turret";
				 }
			       }
                             if (mod == MOD_PGRENADE) 
			       {
				 if (attacker->owner == self) {
				   pKillerName = self->client->pers.netname;
				   pScoreType  = "Suicide";
				   iScore      = -1;
                                   pWeaponName = "Poison_Grenade";
				 }
                                 else {
				   pTargetName = self->client->pers.netname;
				   pKillerName = attacker->owner->client->pers.netname;
				   pScoreType  = "Kill";
				   iScore      = 1;
                                   pWeaponName = "Poison_Grenade";
				 }
			       }
                             if (mod == MOD_FGRENADE) 
			       {
				 if (attacker->owner == self) {
				   pKillerName = self->client->pers.netname;
				   pScoreType  = "Suicide";
				   iScore      = -1;
                                   pWeaponName = "Flash_Grenade";
				 }
                                 else {
				   pTargetName = self->client->pers.netname;
				   pKillerName = attacker->owner->client->pers.netname;
				   pScoreType  = "Kill";
				   iScore      = 1;
                                   pWeaponName = "Flash_Grenade";
				 }
			       }
			  }
		      else {
			if (attacker && attacker->owner && attacker->owner->owner && 
                            attacker->owner->owner->client) {
			     if (mod ==  MOD_TARGET_LASER) {
			       if (attacker->owner->owner == self) {
				 pKillerName = self->client->pers.netname;
				 pScoreType  = "Suicide";
				 iScore      = -1;
				 pWeaponName = "Laser_Mine";
			       }
			       else {
				 pTargetName = self->client->pers.netname;
				 pKillerName = attacker->owner->owner->client->pers.netname;
				 pScoreType  = "Kill";
				 iScore      = 1;
				 pWeaponName = "Laser_Mine";
			       }
			     }
			}
		      }
                }
            }

            /* Log a score */
            sl_LogScore( gi,
                         pKillerName,
                         pTargetName,
                         pScoreType,
                         pWeaponName,
                         iScore,
                         level.time );

            return;
        }
    }

    /* default - not multplayer */
    /* Death - Not Logged */
    sl_LogScore( gi,
                 "",
                 "",
                 "ERROR",
                 "",
                 0,
                 level.time );
    return;
}

void sl_WriteStdLogPlayerEntered( game_import_t     *gi,
                                  level_locals_t     level,
                                  edict_t           *ent )
{
    if( sl_Logging( gi, pPatch ) )
    {
        sl_LogPlayerConnect( gi,
                             ent->client->pers.netname,
                             NULL,
                             level.time);        
    }
}

void sl_LogPlayerDisconnect( game_import_t      *gi,
                             level_locals_t      level,
                             edict_t            *ent )
{
    // GSLogMod Start: Player disconnected
    if( sl_Logging( gi, pPatch ) )
    {
        sl_LogPlayerLeft( gi,
                          ent->client->pers.netname,
                          level.time );
    }
}

