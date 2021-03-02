//*************************************************************************************
//*************************************************************************************
// File: server.cpp
//*************************************************************************************
//*************************************************************************************

//#include "stdafx.h"
#include "kots.h"

#include "user.h"
#include "shared.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern "C"
{
#include "g_local.h"
#include "exports.h"
#include "p_menu.h"
#include "super.h"
#include "superdef.h"

void stuffcmd( edict_t *e, char *s );
}
#include "kotscpp.h"

//*************************************************************************************
//*************************************************************************************
// Function: KOTSStartup
//*************************************************************************************
//*************************************************************************************

void KOTSStartup()
{
	char str[ _MAX_PATH ];

	strcpy( str, "Current KOTS Version: " );
	strcat( str, KOTS_VERSION );

	KOTSMessage( str );

	theApp.SetDataDir( kots_datapath->string );

	theApp.SaveAll();
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSMessage
//*************************************************************************************
//*************************************************************************************

void KOTSMessage( const char *msg )
{
	char str[ _MAX_PATH ];

	strcpy( str, "KOTS: " );
	strcat( str, msg );
	strcat( str, "\n" );

	gi.dprintf( str );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSAssignSkin
//*************************************************************************************
//*************************************************************************************

char *KOTSAssignSkin( edict_t *ent )
{
	int  playernum = ent-g_edicts - 1;
	char t[64];
	char *skin;

	if ( !ent->client->resp.kots_team  )
		skin = kots_skin1->string;
	else
		skin = kots_skin2->string;

	strcpy( t, skin );

	gi.configstring( CS_PLAYERSKINS + playernum, 
	                 va("%s\\%s", ent->client->pers.netname, t ) );

	return skin;
}

//*************************************************************************************
//*************************************************************************************
// Function: GetNumPlayers
//*************************************************************************************
//*************************************************************************************

int KOTSNumPlayers()
{
	int     i;
	int     total = 0;
	edict_t *ent;

 	for ( i = 0; i < game.maxclients; i++ )
	{
		ent = g_edicts + 1 + i;

		if ( !ent->inuse )
			continue;

		if ( !ent->inuse || !ent->client->resp.kots_ingame || ent->client->resp.spectator )
			continue;

		total++;
	}
	return total;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSTime
//*************************************************************************************
//*************************************************************************************

void KOTSTime( edict_t *ent )
{
	long  ltime;
	CUser *user = KOTSGetUser( ent );

	if ( !user )
		return;

	if ( !ent->inuse || !ent->client->resp.kots_ingame || ent->client->resp.spectator )
		return;

	if ( ent->client->pers.health < 1 || level.intermissiontime )
		return;

	time( &ltime );

	if ( ent->client->resp.kots_playtime < 1 || ent->client->resp.kots_playtime < ltime )
	{
		user->m_playtime += 20;

		ent->client->resp.kots_playtime = ltime + 20;
	}
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSEnter
//*************************************************************************************
//*************************************************************************************

void KOTSEnter( edict_t *ent )
{
	gi.bprintf( PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname );

	ent->kotshelp   = 0;
	ent->kots_vote  = 0;
	ent->kots_hits	= 0;
	ent->kots_shots	= 0;
	ent->kots_kills	= 0;
 
 	ent->kots_endstat  = 0;
	ent->kots_endscore = 0;

	ent->kots_lives = kots_lives->value;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSGetUser
//*************************************************************************************
//*************************************************************************************

CUser *KOTSGetUser( edict_t *ent )
{
 	CUser *user;
	
	if ( !ent )
		return NULL;

	if ( !ent->client )
		return NULL;

	if ( !ent->client->pers.kotsdata )
		return NULL;

	user = (CUser *)ent->client->pers.kotsdata;

	return user;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSGetClientData
//*************************************************************************************
//*************************************************************************************

int KOTSGetClientData( edict_t *, char *userinfo )
{
	char *name;
	char *ip;
	char temp[ 16 ];

	name = Info_ValueForKey( userinfo, "name" );

	if ( strlen( name ) < 1 )
	{
	 	Info_SetValueForKey( userinfo, "rejmsg", "You need a name to play." );
	 	return false;
	}

	if ( strlen( name ) > 15 )
	{
		memset( temp, 0, sizeof temp );
		memcpy( temp, name, 15 );

	 	Info_SetValueForKey( userinfo, "name", temp );

		name = Info_ValueForKey( userinfo, "name" );
	}
	ip = Info_ValueForKey( userinfo, "ip" );

	return true;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSClientCanEnter
//*************************************************************************************
//*************************************************************************************

int KOTSClientCanEnter( edict_t *ent )
{
	int  x;
	char *ip;
	char *pass;
	char *name;
	char *userinfo = ent->client->pers.userinfo;
	char password[ 100 ];

	CUser *user;

	pass = Info_ValueForKey( userinfo, "kots_password" );
	name = Info_ValueForKey( userinfo, "name" );

	strcpy( password, pass );

	if ( strlen( password ) < 1 )
		strcpy( password, USER_DEFPASS );

	if ( strlen( password ) > NAMESIZE - 1 )
	{
		gi.centerprintf( ent, "Password must be less than 30 chars long." );
		return false;
	}
	ip = Info_ValueForKey( userinfo, "ip" );

	user = KOTSGetUser( ent );

	if ( user )
	{
		theApp.DelUser( user, false );

	 	ent->client->pers.kotsdata = NULL;
	}
	user = theApp.AddUser( name, password, x );

	if ( !user )
	{
		switch ( x )
		{
			case KOTS_INGAME:
				gi.centerprintf( ent, "Player is already in the game." );
				break;

			case KOTS_INVALID_P_PASS:
				gi.centerprintf( ent, "Invalid password for username." );
				break;
			default:
				gi.centerprintf( ent, "Unknown Error." );
				break;
		}
		return false;
	}
	
	if ( user->Level() < kots_levelmin->value || user->Level() > kots_levelmax->value )
	{
		theApp.DelUser( user, false );

		gi.centerprintf( ent, "Cannot enter due to level restrictions." );

		return false;
	}

	if ( kots_lives->value && user->m_bBossFlag )
	{
		theApp.DelUser( user, false );

		gi.centerprintf( ent, "No bosses in KOTS Lives." );

		return false;
	}

	if ( kots_lives->value && level.time > 60.0 )
	{
		theApp.DelUser( user, false );

		gi.centerprintf( ent, "Cannot enter in the middle of KOTS Lives." );

		return false;
	}
	ent->client->pers.kotsdata = user;

	ent->kots_lives = kots_lives->value;

	user->m_ent = ent;

	return true;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSEnd
//*************************************************************************************
//*************************************************************************************

void KOTSEnd()
{
	theApp.Cleanup();
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSLeave
//*************************************************************************************
//*************************************************************************************

int KOTSLeave( edict_t *ent )
{
	CUser *user;

 	user = KOTSGetUser( ent );

	if ( !user )
	{
		gi.cprintf( NULL, PRINT_HIGH, "Edict leaving with no user.\n" );
		return true;
	}
	user->m_health = ent->health;

	if ( !theApp.DelUser( user ) )
		gi.cprintf( NULL, PRINT_HIGH, "User leaving but not found.\n" );

 	ent->client->pers.kotsdata = NULL;

	return true;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSInit
//*************************************************************************************
//*************************************************************************************

void KOTSInit( edict_t *ent )
{
 	CUser *user = KOTSGetUser( ent );

	if ( !user )
		return;

	user->SetMaxAmmo  ();
	user->SetMaxHealth();

	if ( user->m_health <= 0 )
		user->Respawn();

	user->Init();

	user->m_health = 0;

	ent->client->invincible_framenum = level.framenum + USER_SPAWN_INVULN;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSHelp
//*************************************************************************************
//*************************************************************************************

void KOTSHelp( CUser *user, int blevel )
{
 	edict_t *ent = user->m_ent;
	
	if ( !user || !ent )
		return;

	if ( user->Level() > 0 && !ent->kotshelp )
		return;

	if ( blevel )
	{
		gi.centerprintf( ent, "visit www.planetquake.com/kots to choose new abilities." );

		return;
	}

	switch ( ent->kotshelp )
	{
		case 1:
			gi.centerprintf( ent, "To teleport yourself:\n bind x use tball self" );
			ent->kotshelp++;
			break;
		case 2:
			gi.centerprintf( ent, "To teleport another:\n bind x use tball" );
			ent->kotshelp++;
			break;
		case 3:
			gi.centerprintf( ent, "" );
			ent->kotshelp++;
			break;
		case 4:
			gi.centerprintf( ent, "" );
			ent->kotshelp++;
			break;
		case 5:
			gi.centerprintf( ent, "Vote for next map\n by typing vote in console." );
			ent->kotshelp++;
			break;
		case 6:
			gi.centerprintf( ent, "" );
			ent->kotshelp++;
			break;
		case 7:
			gi.centerprintf( ent, "Game score is your score for this map" );
			ent->kotshelp++;
			break;
		case 8:
			gi.centerprintf( ent, "Total score is your saved score\n for every time you've played kots" );
			ent->kotshelp++;
			break;
		case 9:
			gi.centerprintf( ent, "" );
			ent->kotshelp++;
			break;
		case 10:
			gi.centerprintf( ent, "To set a password:\n set kots_password mypass u" );
			ent->kotshelp++;
			break;
		case 11:
			gi.centerprintf( ent, "" );
			ent->kotshelp++;
			break;
		case 12:
			gi.centerprintf( ent, "The first Quake based RPG!" );
			ent->kotshelp++;
			break;
		case 13:
			gi.centerprintf( ent, "The saber replaces the blaster\n to use it, just use the blaster" );
			ent->kotshelp++;
			break;
		case 14:
			gi.centerprintf( ent, "You get 1 point for each player you teleport\n if you don't teleport yourself" );
			ent->kotshelp++;
			break;
		case 15:
			gi.centerprintf( ent, "Level 1 is reached when you have\n 100 total points" );
			ent->kotshelp++;
			break;
		case 16:
			gi.centerprintf( ent, "Sometimes you have to type your password 3\n times at the console to get in the game" );
			ent->kotshelp++;
			break;
		case 17:
			gi.centerprintf( ent, "" );
			ent->kotshelp++;
			break;
		case 18:
			gi.centerprintf( ent, "Type kotsinfo in the console" );
			ent->kotshelp++;
			break;
		case 19:
			gi.centerprintf( ent, "" );
			ent->kotshelp++;
			break;
		case 20:
			gi.centerprintf( ent, "This help stops after you reach\n Level 1" );
			ent->kotshelp++;
			break;
		case 21:
			gi.centerprintf( ent, "Type kotshelp to get this help\n after Level 0" );
			ent->kotshelp++;
			break;
		case 22:
			gi.centerprintf( ent, "A spree war is everyone against\n the guy glowing white" );
			ent->kotshelp++;
			break;
		case 23:
			gi.centerprintf( ent, "Someone glowing green\n is on a killing spree" );
			ent->kotshelp++;
			break;
		default:
			gi.centerprintf( ent, "Player help is at\n www.planetquake.com/kots" );
			ent->kotshelp = 1;
			break;
	};
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSDeathSound
//*************************************************************************************
//*************************************************************************************

void KOTSDeathSound( edict_t *ent )
{
	int  index;
	char sound[ 50 ];

	if ( !ent || !ent->client )
		return;
	
	switch ( ent->kots_deathsound )
	{
		case 1:
		case 2:
		case 3:
			sprintf( sound, "makron/laf%d.wav", ent->kots_deathsound );
			break;
		case 4:
		case 5:
			sprintf( sound, "makron/roar%d.wav", ent->kots_deathsound - 3 );
			break;
		case 6:
		case 7:
		case 8:
		case 9:
			sprintf( sound, "makron/voice%d.wav", ent->kots_deathsound - 5 );
			break;
		default:
			ent->kots_deathsound = 0;
			strcpy( sound, "makron/voice.wav" );
			break;
	}
	ent->kots_deathsound++;

	index = gi.soundindex( sound );

	gi.sound( ent, 5, index, 1, ATTN_NORM, 0 );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSSpawnKick
//*************************************************************************************
//*************************************************************************************

void KOTSSpawnKick( edict_t *attacker )
{
	float	  points;
	float   radius = 100;
	float   kick   = 800;
	vec3_t	v;
	vec3_t	dir;
	edict_t	*ent = NULL;

	while( ( ent = findradius( ent, attacker->s.origin, radius ) ) != NULL )
	{
		if ( !ent->client || ent == attacker )
			continue;

		if ( !ent->takedamage || !ent->inuse || ent->health <= 0 )
			continue;

		if ( !ent->client->resp.kots_ingame || ent->client->resp.spectator )
			continue;

		//add mins + max and save in vector "v"
		VectorAdd( ent->mins, ent->maxs, v );

		//multiply the origin by 1/2 and add to vector "v" then save result in "v"
		VectorMA( ent->s.origin, 0.5, v, v );

		//Subtract origin from v and save back into "v"
		VectorSubtract( attacker->s.origin, v, v );

		points = radius - 0.5 * VectorLength( v );

 		if ( points <= 0 )
			continue;

		if ( !CanDamage( ent, attacker ) )
			continue;

		//it looks like they are drawing a line to from the teleported player to the 
		//player on the other side

		//this should work ok if not tweak it to look like void T_RadiusTeleport()
		if ( strcmp( ent->classname, "player" ) == 0 &&
			   strcmp( attacker->classname, "player" ) == 0 )
		{
			VectorSubtract( ent->s.origin, attacker->s.origin, dir );

			T_Damage( ent, attacker, attacker, dir, attacker->s.origin, vec3_origin, 0, kick, 0, MOD_TELEFRAG );

			//this gives a "HUH" sound (jump sound) but will really make it feel nice
			gi.sound( ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0 );

			gi.bprintf( PRINT_HIGH, "%s got kicked out of the way by %s\n", ent->client->pers.netname, attacker->client->pers.netname );
		}
	}
}

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************


