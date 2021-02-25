/******************************************************************************
**
**	GibStats Logging functions
**
**
**	$Id: gslog.c 1.4 1999/04/07 21:17:13 mdavies Exp $
**
**  Copyright (c) 1998-1999 Mark Davies.
**  Distributed under the "Artistic License".
**  Please read the file artistic.txt for complete licensing and
**  redistribution information.
**
******************************************************************************/

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
            char       *pScoreType  =   NULL; /* Kill, Suicide */
            char       *pWeaponName =   NULL;
            int        iScore       =   0;    /* 1,    -1 */
            int        killerPing   =  -1;


            if( attacker == self )
            {
                /* Suicide - weapon */
                pKillerName = self->client->pers.netname;
                killerPing  = self->client->ping;
                pScoreType  = "Suicide";
                iScore      = -1;

                /* Get weapon name, being very careful - mdavies */
                /* This weapon may not have been the weapon used if the weapon was changed before the death */
                pWeaponName = (NULL != attacker->client->pers.weapon)?(attacker->client->pers.weapon->pickup_name):(NULL);

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
                    case MOD_TARGET_LASER:
                    {
                        pWeaponName = "Lasered";
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
                    killerPing  = self->client->ping;
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
                        default:
                        {
                            /* Kill - weapon */
                            pTargetName = self->client->pers.netname;
                            pKillerName = attacker->client->pers.netname;
                            killerPing  = attacker->client->ping;
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
                            killerPing  = attacker->client->ping;
                            pScoreType  = "Kill";
                            iScore      = 1;

                            /* Set weapon name - mdavies */
                            pWeaponName = "Telefrag";
                            break;
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
                         level.time,
                         killerPing );

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
                 level.time,
                 -1 );
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


/* end of file */