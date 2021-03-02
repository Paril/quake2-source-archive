//*************************************************************************************
//*************************************************************************************
// File: mothertballz.c
//*************************************************************************************
//*************************************************************************************

//#include "stdafx.h"
#include "kots.h"

#include "user.h"
#include <string.h>
#include <stdlib.h>

extern "C"
{
#include "g_local.h"
#include "exports.h"

void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);

qboolean KillBox( edict_t *ent );

edict_t *SelectDeathmatchSpawnPoint();
}
#include "kotscpp.h"
#include "nptrarray.h"

//*************************************************************************************
//*************************************************************************************
// Function: KOTSTeleport
//*************************************************************************************
//*************************************************************************************

bool KOTSTeleport( edict_t *other )
{
	int     i;
	int     index;
	gitem_t	*it;
	edict_t *dest;

	if ( other->client->pers.kots_notball )
		return false;

	if ( other->client->pers.kots_streak_count >= 25 )
		return false;

	dest = SelectDeathmatchSpawnPoint();

	if ( !dest )
		return false;

	it = FindItem ("Damage Amp");

	if ( it != NULL )
	{
		index = ITEM_INDEX(it);

		if ( other->client->pers.inventory[index] )
			it->drop( other, it );
	}
	it = FindItem ("resist");

	if ( it != NULL )
	{
		index = ITEM_INDEX(it);

		if ( other->client->pers.inventory[index] )
			it->drop( other, it );
	}
	gi.unlinkentity (other);

	VectorCopy( dest->s.origin, other->s.origin );
	VectorCopy( dest->s.origin, other->s.old_origin );

	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear( other->velocity );
	other->client->ps.pmove.pm_time   = 160 >> 3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for( i = 0; i < 3; i++ )
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT( dest->s.angles[i] - other->client->resp.cmd_angles[i] );

	VectorClear( other->s.angles );
	VectorClear( other->client->ps.viewangles );
	VectorClear( other->client->v_angle );

	KillBox( other );

	gi.linkentity( other );

	return true;
}

//*************************************************************************************
//*************************************************************************************
// Function: T_RadiusTeleport
//*************************************************************************************
//*************************************************************************************

void T_RadiusTeleport( edict_t *inflictor, edict_t *attacker, float damage, 
                       edict_t *ignore, float radius )
{
	int     x;
	int     bMe   = 0;
	int     score = 0;
	float	  points;
	vec3_t  v;
	CUser   *user = KOTSGetUser( attacker );
	edict_t	*ent = 0;

	CNPtrArray array;

	while ( ( ent = findradius( ent, inflictor->s.origin, radius ) ) != NULL )
	{
		if ( ent == ignore || !ent->client )
			continue;

		if ( !ent->takedamage || !ent->inuse || ent->health <= 0 )
			continue;

		if ( !ent->client->resp.kots_ingame || ent->client->resp.spectator )
			continue;

		VectorAdd     ( ent->mins, ent->maxs, v   );
		VectorMA      ( ent->s.origin, 0.5, v, v  );
		VectorSubtract( inflictor->s.origin, v, v );

		points = damage - 0.5 * VectorLength( v );

		if ( ent == attacker )
			points = points * 0.5;

		if ( points <= 0 )
			continue;

		if ( !CanDamage( ent, inflictor ) )
			continue;

		if ( ent->client->pers.kots_stoptball )
		{
			ent->client->pers.inventory[ ITEM_INDEX( FindItem( "tball" ) ) ]++;

			gi.bprintf( PRINT_HIGH, "%s stopped %s's tball\n", ent->client->pers.netname, attacker->client->pers.netname );
			return;
		}
		array.Add( ent );
	}

	for ( x = 0; x < array.GetSize(); x++ )
	{
		ent = (edict_t *)array[x];

		if ( !KOTSTeleport( ent ) )
			continue;

		gi.sound( ent, CHAN_ITEM, gi.soundindex ("misc/tele1.wav"), 1, ATTN_NORM, 0 );

		if ( strcmp( ent->classname     , "player" ) == 0 &&
			   strcmp( attacker->classname, "player" ) == 0 )
		{
			if ( ent != attacker )
			{
				gi.bprintf( PRINT_HIGH, "%s was teleported by %s\n", ent->client->pers.netname, attacker->client->pers.netname );

				score++;
			}
			else
			{ 
				gi.bprintf( PRINT_HIGH,"%s teleports away\n", attacker->client->pers.netname );

				bMe = true;
			}
		}
		else if ( strncmp( ent->classname, "KOTSbot", 7 ) == 0 )
			gi.bprintf( PRINT_HIGH, "%s was teleported by %s\n", ent->classname, attacker->client->pers.netname );
	}

	if ( !bMe )
	{
		if ( user && score > 0 )
			KOTSChangeScore( user, score, USER_TELEPORT );
	}
}

//*************************************************************************************
//*************************************************************************************
// Function: KOTSRadiusTeleport
//*************************************************************************************
//*************************************************************************************

void KOTSRadiusTeleport( edict_t *ent )
{
	T_RadiusTeleport( ent, ent, 160, ent, 100 );
}

//*************************************************************************************
//*************************************************************************************
// Function: T_Ballz_Explode
//*************************************************************************************
//*************************************************************************************

static void T_Ballz_Explode (edict_t *ent)
{
	int    effect = TE_BFG_EXPLOSION;
	vec3_t origin;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);//bounce?

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	T_RadiusTeleport(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius + 40);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
	{
		if (ent->groundentity)
			gi.WriteByte (effect);
		else
			gi.WriteByte (effect);
	}
	else
	{
		if (ent->groundentity)
			gi.WriteByte (effect);
		else
			gi.WriteByte (effect);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	G_FreeEdict (ent);
}

//=============================Mother=============================

static void T_Ball_Touch (edict_t *ent, edict_t *other, cplane_s *, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

		if (!other->takedamage)
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}
	T_Ballz_Explode (ent);
}


//===============================Mother===============================

void fire_T_Ballz (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float, float damage_radius, int bMe )
{
	edict_t	*tball;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	tball = G_Spawn();//get g_spawn
	VectorCopy (start, tball->s.origin);
	VectorScale (aimdir, speed, tball->velocity);
	VectorMA (tball->velocity, 200 + crandom() * 10.0, up, tball->velocity);
	VectorMA (tball->velocity, crandom() * 10.0, right, tball->velocity);
	VectorSet (tball->avelocity, 300, 300, 300);
	tball->movetype = MOVETYPE_BOUNCE;
	tball->clipmask = MASK_SHOT;
	tball->solid = SOLID_BBOX;
	tball->s.effects |= EF_GRENADE;
	//VectorClear (tball->mins);//was for 0 size object
	//VectorClear (tball->maxs);
	//try to set the min and max bounding size
	VectorSet (tball->mins, -8, -8, -8);
	VectorSet (tball->maxs, 8, 8, 8);
	//tball->s.modelindex = gi.modelindex ("models/objects/smoke/tris.md2");
	tball->s.modelindex = gi.modelindex ("models/items/ammo/grenades/medium/tris.md2");
	tball->owner = self;
	tball->touch = T_Ball_Touch;
	tball->nextthink = level.time + 1;
	tball->think = T_Ballz_Explode;
	tball->dmg = damage;
	tball->dmg_radius = damage_radius;
	tball->classname = "T_Ball";

	gi.linkentity (tball);

	if ( bMe )
		T_Ballz_Explode (tball);
}



//=======================end Mother===============================



//*************************************************************************************
//*************************************************************************************
// Function: 
//*************************************************************************************
//*************************************************************************************

void KOTS_Use_T_Ball( edict_t *ent, gitem_t *item )
{
	//function to call if you want a player to fire a tball!
	vec3_t offset;
	vec3_t forward, right;
	vec3_t start;
	int		 damage = 160;
	int    bMe    = false;
	char   *type = gi.argv(2);
	float	 radius;
	CUser *user = KOTSGetUser( ent );

	int		  index;

	if ( !strcmp( type, "self" ) )
		bMe = true;

	//if they are dead dont shoot one!
	if ( ent->health < 1 || !user )	
		return;
 
 	if ( ent->client->pers.kots_streak_count >= 25 )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "no tballing while on a spree war!\n" );      
		return;
	}
	index = ITEM_INDEX( item );

	ent->client->pers.selected_item = index;

	radius = damage;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_T_Ballz (ent, start, forward, damage, 600, 2.5, radius, bMe);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	//ent->client->pers.T_Ballz = ent->client->pers.T_Ballz - 1;
	ent->client->pers.inventory[index] -= 1;
	//gi.cprintf(ent,PRINT_HIGH, "You now have %i T_Ballz left\n", ent->client->pers.T_Ballz);
	gi.cprintf(ent,PRINT_HIGH, "You now have %i T_Ballz left\n", ent->client->pers.inventory[index]);
}