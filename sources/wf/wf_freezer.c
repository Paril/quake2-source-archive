/*****************************************************************
 
 Freezer gun source code - by Acrid-, acridcola@hotmail.com
 
 ..............................................................
 
 This file is Copyright(c) 1999, Acrid-, All Rights Reserved.
 
 ..............................................................
 

 Should you decide to release a modified version of the Freezer gun, you
 MUST include the following text (minus the BEGIN and END lines) in 
 the documentation for your modification, and also on all web pages 
 related to your modification, should they exist.
 
 --- BEGIN ---
 
 The Freezer gun and related code is a product of Acrid- designed 
 for Weapons Factory, and is available as part of the Weapons Factory 
 Source Code or a seperate tutorial.
 
 This program MUST NOT be sold in ANY form. If you have paid for
 this product, you should contact Acrid- at:
 acridcola@hotmail.com
 
 --- END ---
 
 have fun,
 
 Acrid-
 
 *****************************************************************/

/*
==============================
FREEZER CODE by Acrid

1/14/99  updated 3/99-5/99
Notes: if you change ammo consumption ,there are 4 spots,
1 in g_items,and 3 here
Current rate is 3 per shot
==============================
*/
#include "g_local.h"

//New functions by Gregg Reno- used for testing freeze on self
void freeze_player(edict_t *ent)
{
    if (ent->health <= 0)//5/99
        return;

	// make em frozen
	ent->frozen = 1;

	// set the time till thaw
	ent->frozentime = level.time + 4;	//4 seconds of freeze (was 6)
	gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/freezer/freeze.wav"), 1, ATTN_NORM, 0);

}

void unfreeze_player(edict_t *ent)
{
	ent->frozen = 0;
	ent->frozentime = level.time - 1;
	gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/freezer/shatter1.wav"), 1, ATTN_NORM, 0);
}

/*
================================================
 Same as blaster_touch with some freezing info 
=================================================
*/
void freezer_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{ 
	extern void ClientUserinfoChanged (edict_t *ent, char *userinfo);

    int mod;
	

      mod = MOD_FREEZER;
//note; returns in this part of the code will produce a richocet type effect,try shooting bots
	if (other == self->owner)
		return;
//	if (other->bot_client)
//		return;

    if (other->wf_team == self->wf_team)
	{
		other->frozen = 0;
	}

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}
/*note: Probably better to ignore feigns because of returns in 
        ClientThink, but you still take damage*/
	if (other->client && !(other->frozen) && !(other->client->pers.feign) 
		&& other->wf_team != self->wf_team) 
	{
		freeze_player(other);
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
		
		T_Damage (other, self, self->owner, self->velocity,
		self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);


	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void fire_freezer (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*freezer;
	trace_t	tr;

	VectorNormalize (dir);

	freezer = G_Spawn();
	freezer->wf_team = self->wf_team;
	VectorCopy (start, freezer->s.origin);
	VectorCopy (start, freezer->s.old_origin);
	vectoangles (dir, freezer->s.angles);
	VectorScale (dir, speed, freezer->velocity);
	freezer->movetype = MOVETYPE_FLYMISSILE;
	freezer->clipmask = MASK_SHOT;
	freezer->solid = SOLID_BBOX;
	freezer->s.effects |= effect |EF_COLOR_SHELL;
	freezer->s.renderfx |= RF_SHELL_BLUE|RF_SHELL_GREEN|RF_SHELL_RED;
	VectorClear (freezer->mins);
	VectorClear (freezer->maxs);
	freezer->s.modelindex = gi.modelindex ("models/spike/tris.md2");
	freezer->s.sound = gi.soundindex ("weapons/freezer/freezefly.wav");
	freezer->classname = "iceball";
	freezer->owner = self;
	freezer->touch = freezer_touch;
	freezer->nextthink = level.time + 2;
	freezer->think = G_FreeEdict;
	freezer->dmg = damage;
	gi.linkentity (freezer);

	if (self->client)
		check_dodge (self, freezer->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, freezer->s.origin, freezer, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (freezer->s.origin, -10, dir, freezer->s.origin);
		freezer->touch (freezer, tr.ent, NULL, NULL);
	}
}
/*
================================================
 Same as orig blaster_fire but freeze code added
================================================
*/
void Freezer_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	float volume;


	// check and abort firing if we don't have enough acrid 3/99
	if (ent->client->pers.inventory[ent->client->ammo_index] < 3)
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_freezer (ent, start, forward, damage, wf_game.weapon_speed[WEAPON_FREEZER], effect);

	if (is_silenced)
		volume = .5;
	else
		volume = 1.0;

	gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/freezer/freezefire.wav"), volume, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);
    //remove ammo acrid 3/99 - modified by Gregg
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 3;
}
/*
==================================================
 Same as Weapon_Blaster_Fire but freeze code added
==================================================
*/ 
void Weapon_Freezer_Fire (edict_t *ent)
{
	int		damage;

	damage = wf_game.weapon_damage[WEAPON_FREEZER];//3/99
    
	//Change weapon if ammo falls below 3 //4/99
	if (ent->client->pers.inventory[ent->client->ammo_index] < 3)
	{

		ent->client->ps.gunframe = 19;//start of idle frames
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			
		return;
	}
	                                               //white particles trail
	Freezer_Fire (ent, vec3_origin, damage, false, EF_GRENADE);
	ent->client->ps.gunframe++;
}
/*
==================================
  Same as Weapon_Blaster with mod
==================================
*/
void Weapon_Freezer (edict_t *ent)
{
    //Rail Gun frames acrid 3/99
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, Weapon_Freezer_Fire);

}	
