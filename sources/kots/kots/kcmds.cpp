//*************************************************************************************
//*************************************************************************************
// File: kcmds.cpp
//*************************************************************************************
//*************************************************************************************

//#include "stdafx.h"
#include "kots.h"
#include <string.h>
#include <stdlib.h>

extern "C"
{
#include "g_local.h"
#include "exports.h"
#include "super.h"
#include "superdef.h"

qboolean is_quad;

void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void fire_bfgball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);
void FireHook( edict_t *ent );
void fire_boomerang (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed);
void fire_flash_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int speed, float timer, float damage_radius);

}
#include "../kuser/user.h"
#include "kotscpp.h"

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_Team
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_Team( edict_t *ent )
{
	int     i;
	char    *skin;
	edict_t *cl_ent;

	if ( ent->health < 1 || !ent->client->resp.kots_ingame )
		return;

	if ( !kots_teamplay->value )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "This is not kots teamplay.\n");

		return;
	}

 	for ( i = 0; i < game.maxclients; i++ )
	{
		cl_ent = g_edicts + 1 + i;

		if ( !cl_ent->inuse || !cl_ent->client )
			continue;

		if ( !cl_ent->client->resp.kots_ingame )
			continue;

		if ( cl_ent->client->resp.kots_team == ent->client->resp.kots_team )
			gi.cprintf( ent, PRINT_MEDIUM, "%s\n", cl_ent->client->pers.netname );
	}

	if ( !ent->client->resp.kots_team  )
		skin = kots_skin1->string;
	else
		skin = kots_skin2->string;

	gi.centerprintf( ent, "You are on the %s Team\n", skin );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_ChangeTeams
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_ChangeTeams( edict_t *ent )
{
	int     i;
	int     teams[2];
	char    *skin;
	edict_t *cl_ent;

	memset( teams, 0, sizeof teams );

	if ( !ent->client->resp.kots_ingame )
		return;

	if ( !kots_teamplay->value )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "This is not kots teamplay.\n");

		return;
	}

 	for ( i = 0; i < game.maxclients; i++ )
	{
		cl_ent = g_edicts + 1 + i;

		if ( !cl_ent->inuse || !cl_ent->client )
			continue;

		if ( !cl_ent->client->resp.kots_ingame )
			continue;

		teams[ cl_ent->client->resp.kots_team ]++;
	}

	if ( ( ent->client->resp.kots_team == 1 && teams[1] < teams[0] ) || 
	     ( ent->client->resp.kots_team == 0 && teams[0] < teams[1] ) ) 
	{
		gi.cprintf( ent, PRINT_MEDIUM, "That team has too many people.\n");

		return;
	}
	ent->client->resp.kots_team = !ent->client->resp.kots_team;

	skin = KOTSAssignSkin( ent );
	
	gi.bprintf( PRINT_HIGH, "%s Switches to Team %s\n", ent->client->pers.netname, skin );

	gi.centerprintf( ent, "You are on the %s Team\n", skin );

	ent->kots_shots	= 0;
	ent->kots_kills	= 0;
	ent->kots_hits 	= 0;

	ent->client->resp.score = 0;

	ent->client->pers.kots_streak_count  = 0;
	ent->client->pers.kots_streak_points = 0;

	ent->client->kots_last   = NULL;
	ent->client->kots_damage = 0;

	if ( ( level.time - ent->client->respawn_time ) < 5 )
		return;

	if ( teams[ ent->client->resp.kots_team ] + 1 < teams[ !ent->client->resp.kots_team ] )
	{
		KOTSTeleport( ent );
	 
	 	return;
	}
	if ( ent->health < 1 )
		return;

	ent->flags &= ~FL_GODMODE;

	ent->health = 0;

	meansOfDeath = MOD_SUICIDE;

	player_die (ent, ent, ent, 100000, vec3_origin);
}

//*************************************************************************************
//*************************************************************************************
// Function: CanUseItem
//*************************************************************************************
//*************************************************************************************

static bool CanUseItem( edict_t *ent, int item, char *message )
{
	bool  bRes  = false;
	CUser *user = KOTSGetUser( ent );

	if ( !user )
		return false;

	if ( ent->health < 1 || !ent->client->resp.kots_ingame )
		return false;

	if ( user->m_items[ item ] <= 0 )
	{
		gi.cprintf( ent, PRINT_MEDIUM, message );

		user->m_items[ item ] = 0;
	}
	else
	{
		user->m_items[ item ]--;
		bRes = true;
	}
	return bRes;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_Kick
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_Kick( edict_t *ent )
{
	if ( !CanUseItem( ent, KOTS_ITEM_KICK, "you don't rate kicking!\n" ) )
		return;

	KOTSSpawnKick( ent );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_Helmet
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_Helmet( edict_t *ent )
{
	if ( !CanUseItem( ent, KOTS_ITEM_HELMET, "you don't have a helmet!\n" ) )
		return;

	ent->client->pers.kots_helmet = true;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_TBall
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_TBall( edict_t *ent )
{
	if ( !CanUseItem( ent, KOTS_ITEM_TBALL, "you don't rate tballing!\n" ) )
		return;

	KOTSRadiusTeleport( ent );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_NoTBall
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_NoTBall( edict_t *ent )
{
	if ( ent && ent->client && ent->client->pers.kots_notball )
	{
		ent->client->pers.kots_notball = 0;

		gi.cprintf (ent, PRINT_MEDIUM, "KOTS No TBall: Deactivated\n");

		return;
	}

	if ( !CanUseItem( ent, KOTS_ITEM_NOTBALL, "you don't rate no tball!\n" ) )
		return;

	ent->client->pers.kots_notball = 1;

	gi.cprintf (ent, PRINT_MEDIUM, "KOTS No TBall: Activated\n");
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_StopTBall
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_StopTBall( edict_t *ent )
{
	if ( ent && ent->client && ent->client->pers.kots_stoptball )
	{
		ent->client->pers.kots_stoptball = 0;

		gi.cprintf (ent, PRINT_MEDIUM, "KOTS Stop TBall: Deactivated\n");

		return;
	}

	if ( !CanUseItem( ent, KOTS_ITEM_STOPTBALL, "you don't rate stop tball!\n" ) )
		return;

	ent->client->pers.kots_stoptball = 1;

	gi.cprintf (ent, PRINT_MEDIUM, "KOTS Stop TBall: Activated\n");
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_Help
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_Help( edict_t *ent )
{
 	ent->kotshelp = !ent->kotshelp;
 
 	if ( ent->kotshelp )
		gi.cprintf( ent, PRINT_MEDIUM, "KOTS Help System Activated\n" );
	else
		gi.cprintf (ent, PRINT_MEDIUM, "KOTS Help System Deactivated\n");
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_Cloak
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_Cloak( edict_t *ent ) 
{
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return;

	if ( user->Level() < KOTS_WISDOM_CLOAK )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "you don't rate cloaking!!!\n" );      
		return;
	}
	
	if ( !ent->client->pers.kots_cloaking )
	{
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Power Cube"))] <
		     KOTS_WISDOM_CLOAK_AMMO )
		{
			gi.cprintf( ent, PRINT_MEDIUM, "You don't have enough power to cloak\n" );
			return;
		}
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Power Cube"))] -= KOTS_WISDOM_CLOAK_AMMO;

		gi.bprintf( PRINT_MEDIUM, "%s cloaks\n", ent->client->pers.netname );

		ent->svflags                |= SVF_NOCLIENT;
		ent->client->pers.kots_cloakdrain = 0;
		ent->client->pers.kots_cloaking   = true;
	}
	else
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Cloaking Disabled\n" );

		ent->svflags &= ~SVF_NOCLIENT;

		ent->client->pers.kots_cloaking  = false;
	}
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_Fly
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_Fly( edict_t *ent ) 
{
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return;

	if ( user->Level() < KOTS_WISDOM_FLY )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "you don't rate low gravity!!!\n" );      
		return;
	}

	if ( !ent->client->pers.kots_flying )
	{
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Power Cube"))] < 
		     KOTS_WISDOM_FLY_AMMO )
		{
			gi.cprintf( ent, PRINT_MEDIUM, "You don't have enough power to fly\n" );
			return;
		}
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Power Cube"))] -= KOTS_WISDOM_FLY_AMMO;

		gi.cprintf( ent, PRINT_MEDIUM, "Low Gravity Enabled\n" );

		ent->client->pers.kots_flydrain = 0;
		ent->client->pers.kots_flying   = true;
	}
	else
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Low Gravity Disabled\n" );

		ent->client->pers.kots_flying = false;
	}
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSSVCmd_Status_f
//*************************************************************************************
//*************************************************************************************

void KOTSSVCmd_Status_f()
{
//	CString str;
//
//	str.Format( "KOTS USERS:%4d", theApp.GetUserCount() );
//	KOTSMessage( str );
//	str.Format( "KOTS OUT  :%4d", theApp.GetOutCount () );
//	KOTSMessage( str );
}

//*************************************************************************************
//*************************************************************************************
// Function: LaserSightThink
//*************************************************************************************
//*************************************************************************************

void LaserSightThink( edict_t *ent )
{   
	vec3_t start,end,endp,offset;
	vec3_t forward,right,up;   
	trace_t tr;
	edict_t *owner = ent->owner;

	if ( owner->deadflag || !owner->inuse || !owner->client->pers.kots_laser )
	{      
		G_FreeEdict( ent );

		owner->client->pers.kots_laser = false;

		return;
	}   
	AngleVectors (ent->owner->client->v_angle, forward, right, up);
	VectorSet(offset,24 , 6, ent->owner->viewheight-7);
	G_ProjectSource (ent->owner->s.origin, offset, forward, right, start);
	VectorMA(start,8192,forward,end);
	tr = gi.trace (start,NULL,NULL, end,ent->owner,CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

	if (tr.fraction != 1) 
	{      
		VectorMA(tr.endpos,-4,forward,endp);
		VectorCopy(endp,tr.endpos);   
	}

	if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
	{
		if ((tr.ent->takedamage) && (tr.ent != ent->owner)) 
			ent->s.skinnum = 1;      
	}   
	else      
		ent->s.skinnum = 0;
	
	vectoangles(tr.plane.normal,ent->s.angles);
	VectorCopy(tr.endpos,ent->s.origin);   
	gi.linkentity (ent);
	ent->nextthink = level.time + 0.1;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_Laser
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_Laser( edict_t *ent )
{
	CUser   *user = KOTSGetUser( ent );
	vec3_t  start;   
	vec3_t  forward;   
	vec3_t  right;   
	vec3_t  end;   
	edict_t *laser;

	if ( !user || ent->deadflag )
		return;

	if ( user->Level() < KOTS_TECHNO_LASERSITE )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "you don't rate the lasersight!!!\n" );      

		ent->client->pers.kots_laser = false;

		return;
	}
	
	if ( ent->client->pers.kots_laser ) 
	{      
		ent->client->pers.kots_laser = false;

		gi.cprintf( ent, PRINT_MEDIUM, "lasersight off.\n" );      
		
		return;
	}   
	ent->client->pers.kots_laser = true;

	gi.cprintf( ent, PRINT_MEDIUM, "lasersight on.\n");
  
	AngleVectors( ent->client->v_angle, forward, right, NULL );

	VectorSet( end, 100 , 0, 0 );
	
	G_ProjectSource( ent->s.origin, end, forward, right, start );

	laser = G_Spawn();   
	laser->owner = ent;   
	laser->movetype = MOVETYPE_NOCLIP;
	laser->solid = SOLID_NOT;   
	laser->classname = "lasersight";
	laser->s.modelindex = gi.modelindex ("models/objects/flash/tris.md2");
	laser->s.skinnum = 0;   
	laser->s.renderfx |= RF_FULLBRIGHT;
	laser->s.renderfx |= RF_SHELL_RED;
	laser->think = LaserSightThink;   
	laser->nextthink = level.time + 0.1;
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_Hook
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_Hook( edict_t *ent )
{
	int   *hookstate_;
	char  *s;
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return;

	if ( user->Level() < KOTS_DEXTERITY_GRAPPLE )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "you don't rate the hook!!!\n" );      
		return;
	}

	// get the first hook argument
	s = gi.argv(1);

	// create intermediate value
	hookstate_ = &ent->client->pers.kots_hookstate;

	if ((ent->solid != SOLID_NOT) && (ent->deadflag == DEAD_NO) &&
		(!(*hookstate_ & HOOK_ON)) && (Q_stricmp(s, "action") == 0))
	{
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Power Cube"))] < 
		     KOTS_DEXTERITY_HOOK_COST )
		{
			gi.cprintf( ent, PRINT_MEDIUM, "you don't have enough power to fire the hook!\n" );      
			return;
		}
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Power Cube"))] -= KOTS_DEXTERITY_HOOK_COST;

		// flags hook as being active 
		*hookstate_ = HOOK_ON;   

		FireHook (ent);
		return;
	}

	if  (*hookstate_ & HOOK_ON)
	{
		// release hook	
		if (Q_stricmp(s, "action") == 0)
		{
			*hookstate_ = 0;
			return;
		}

		// deactivate chain growth or shrink
		if (Q_stricmp(s, "stop") == 0)
		{
			*hookstate_ -= *hookstate_ & (GROW_ON | SHRINK_ON);
			return;
		}

		// activate chain growth
		if (Q_stricmp(s, "grow") == 0)
		{
			*hookstate_ |= GROW_ON;
			*hookstate_ -= *hookstate_ & SHRINK_ON;
			return;
		}

		// activate chain shrinking
		if (Q_stricmp(s, "shrink") == 0)
		{
			*hookstate_ |= SHRINK_ON;		
			*hookstate_ -= *hookstate_ & GROW_ON;
		}
	}
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_MakeMega
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_MakeMega( edict_t *ent )
{
	CUser *user = KOTSGetUser( ent );

	if ( !user || ent->deadflag )
		return;

	if ( user->Level() < KOTS_KARMA_MEGA )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "you don't rate making mega health!!!\n" );      
		return;
	}
	KOTSMakeMega( ent );
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSSendAll
//*************************************************************************************
//*************************************************************************************

void KOTSSendAll( bool bSound, char *str )
{
	int i;

	edict_t *ent;

 	for ( i = 0; i < game.maxclients; i++ )
	{
		ent = g_edicts + 1 + i;

		if ( !ent->inuse || !ent->client )
			continue;

		if ( !ent->client->resp.kots_ingame || ent->client->resp.spectator )
			continue;

		if ( bSound )
			gi.sound( ent, 5, gi.soundindex( str ), 1, ATTN_NORM, 0 );
		else
			gi.centerprintf( ent, str );
	}
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_BFGBall
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_BFGBall( edict_t *ent )
{
	int		 damage;
	float	 damage_radius = 1000;
	vec3_t offset, start;
	vec3_t forward, right;

	if ( !CanUseItem( ent, KOTS_ITEM_BFGBALL, "you don't rate bfgball!\n" ) )
		return;

	KOTSStopCloak( ent );

	damage = 500;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bfgball (ent, start, forward, damage, 1000, damage_radius);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_Boomerang
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_Boomerang( edict_t *ent )
{
	int		 damage;
//	float	 damage_radius = 1000;
	vec3_t offset, start;
	vec3_t forward, right;

	CUser *user = KOTSGetUser( ent );

	if ( !user )
		return;

	if ( ent->health < 1 || !ent->client->resp.kots_ingame )
		return;
	if(!ent->client->pers.inventory[ ITEM_INDEX( FindItem( "boomerang" ) ) ])
	{
		gi.cprintf (ent, PRINT_MEDIUM, "you don't have boomerang\n");
		return;
	}

	ent->client->pers.inventory[ ITEM_INDEX( FindItem( "boomerang" ) ) ]--;
	KOTSStopCloak( ent );

	damage = 500;

//	if ( is_quad )
//		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_boomerang (ent, start, forward, damage,900);

	PlayerNoise(ent, start, PNOISE_WEAPON);

}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_Flash
//*************************************************************************************
//*************************************************************************************

void KOTSCmd_Flash( edict_t *ent )
{
	vec3_t offset, start;
	vec3_t forward, right;

	if ( !CanUseItem( ent, KOTS_ITEM_FLASH, "you don't rate flash grenades!\n" ) )
		return;

	KOTSStopCloak( ent );

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	//parameters: who, stasrt point, aim dir, speed, fuse timer, radius
	fire_flash_grenade (ent, start, forward, 600, 2.5, 160);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSCmd_EXPack
//*************************************************************************************
//*************************************************************************************

static void kotsexplode_make_touchable( edict_t *ent )
{
	ent->touch = Touch_Item;

	ent->nextthink = level.time + 30;
	ent->think     = G_FreeEdict;
}

int  KOTSPickup_KOTSExPack( edict_t *pack, edict_t *ent )
{
	vec3_t		origin;

	T_Damage( ent, pack->owner, pack->owner, vec3_origin, pack->s.origin, vec3_origin, KOTS_ITEM_EXPACK_DAMAGE, 500, 0, MOD_KOTSEXPACK );

	VectorMA( pack->s.origin, -0.02, pack->velocity, origin );

	gi.WriteByte( svc_temp_entity );

	if (pack->waterlevel)
	{
		if (pack->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (pack->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (pack->s.origin, MULTICAST_PHS);

	return true;
}

void KOTSCmd_EXPack( edict_t *ent )
{
	edict_t *dropped;
	gitem_t *item;

	if ( !CanUseItem( ent, KOTS_ITEM_EXPACK, "you don't rate exploding packs!\n" ) )
		return;

	item = FindItem( "KOTS ExPack" );

	if ( !item )
		return;

	ent->client->v_angle[YAW] += 22.5;
	dropped = Drop_Item( ent, item );
	ent->client->v_angle[YAW] -= 22.5;

	dropped->think = kotsexplode_make_touchable;
	dropped->owner = ent;
}

//*************************************************************************************
//*************************************************************************************
//*************************************************************************************
//*************************************************************************************


