#include "g_local.h"
#include "m_player.h"

#define FRAME_FIRE_FIRST		(FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST		(FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST	(FRAME_IDLE_LAST + 1)

static void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}
static void Akimbo_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);

	if (client->current_hand == HAND_OTHER)
		_distance[1] *= -1;

	G_ProjectSource (point, _distance, forward, right, result);
}

void Akimbo_Weapon_Generic (edict_t *ent,
					 int FRAME_ACTIVATE_LAST,
					 int FRAME_FIRE_LAST,
					 int FRAME_IDLE_LAST,
					 int FRAME_DEACTIVATE_LAST,
					 int FRAME_FIRE_OTHER_FIRST,
					 int FRAME_FIRE_OTHER_LAST,
					 int *pause_frames,
					 int *fire_frames,
					 void (*fire)(edict_t *ent), int ammouse)
{
	int		n;
	int		FRAME_FIRE_SECONDARY_FIRST;
	int		FRAME_FIRE_SECONDARY_LAST;
	qboolean	secondary_fire_weapon = false;
	if(strcmp(ent->client->pers.weapon->classname, "weapon_m4") == 0)
	{
	FRAME_FIRE_SECONDARY_FIRST = 26;
	FRAME_FIRE_SECONDARY_LAST = 27;
	secondary_fire_weapon = true;
	}
	if(strcmp(ent->client->pers.weapon->classname, "weapon_knives") == 0)
	{
	FRAME_FIRE_SECONDARY_FIRST = 103;
	FRAME_FIRE_SECONDARY_LAST = 106;
	secondary_fire_weapon = true;
	}
	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if ((ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST) && 
			(ent->client->ps.gunindex == gi.modelindex(ent->client->pers.weapon->akimbo_model)))
		{

		}
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			//if(ent->client->akimbo)
		//	ent->client->akimbo = false;
			MatrixChangeWeapon (ent);
			return;
		}
		else if ((FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;
				
			}
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

		if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;
				
			}
		}
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) || ent->secondaryfire )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if ((!ent->client->ammo_index) || 
				( ent->client->pers.inventory[ent->client->ammo_index] >= ammouse))
			{
// MATRIX		
				if (ent->secondaryfire && secondary_fire_weapon)
				{
					
					ent->client->ps.gunframe = FRAME_FIRE_SECONDARY_FIRST;
					ent->client->current_hand = HAND_DEFAULT;
				}
				else if (ent->client->next_hand == HAND_OTHER)
				{
					ent->client->current_hand = HAND_OTHER;
					ent->client->ps.gunframe = FRAME_FIRE_OTHER_FIRST;
					ent->client->next_hand = HAND_DEFAULT;
				}
				else if (ent->client->next_hand == HAND_DEFAULT)
				{
					
					ent->client->current_hand = HAND_DEFAULT;
					ent->client->ps.gunframe = FRAME_FIRE_FIRST;
					ent->client->next_hand = HAND_OTHER;
				}

				


// MATRIX		

				ent->client->weaponstate = WEAPON_FIRING;
	
				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					ent->s.frame = FRAME_crattak1-1;
					ent->client->anim_end = FRAME_crattak9;
				}
				else
				{
					ent->s.frame = FRAME_attack1-1;
					ent->client->anim_end = FRAME_attack8;
				}
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			{
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			if (pause_frames)
			{
				for (n = 0; pause_frames[n]; n++)
				{
					if (ent->client->ps.gunframe == pause_frames[n])
					{
						if (rand()&15)
							return;
					}
				}
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

				fire (ent);
				if (ent->client->pers.inventory[ent->client->ammo_index] < 0)
					ent->client->pers.inventory[ent->client->ammo_index] = 0;
				break;
			}
		}
		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
// MATRIX
		if ((ent->client->current_hand == HAND_OTHER) && (ent->client->ps.gunframe > FRAME_FIRE_OTHER_LAST || ent->client->ps.gunframe > FRAME_FIRE_SECONDARY_LAST ))	
		{
			ent->client->ps.gunframe = FRAME_IDLE_FIRST+1;
			ent->client->weaponstate = WEAPON_READY;
		}
		if (ent->client->ps.gunframe == FRAME_FIRE_SECONDARY_LAST  && secondary_fire_weapon)
		{
			ent->client->ps.gunframe = FRAME_IDLE_FIRST+1;
			ent->client->weaponstate = WEAPON_READY;
		}
// MATRIX
		
	}
}
/*
===================
kick_attack
Actual Melee Weapon
===================
*/
void kick_attack (edict_t *ent, vec3_t start, vec3_t dir, int damage, int kick, int MOD)
{
        
    vec3_t          forward, right;
    vec3_t          offset;
    
    trace_t tr;
    vec3_t end;
    float sound;

	if (ent->deadflag || ent->health < 0)
		return;
	
	sound = random() * 3;
	//if (sound>2)
	//	sound=0;
    if (!(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
	{
	
		if(sound >=0)
		{
			ent->s.frame = FRAME_flip01-1;
			ent->client->anim_end = FRAME_flip06;
		}
		if(sound>=1)
		{
			ent->s.frame = FRAME_wave02;
			ent->client->anim_end = FRAME_wave08;
		}
		if(sound>=2)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			ent->s.frame = FRAME_wave06;
			ent->client->anim_end = FRAME_wave02;
		}
    }
	AngleVectors (ent->client->v_angle, forward, right, NULL);
    
    VectorScale (forward, 0, ent->client->kick_origin);
    
    VectorSet(offset, 0, 0,  ent->viewheight-20);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
    
    VectorMA( start, 80, forward, end );
    
    tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_MONSTERSOLID);
    if (sound >= 0)
	{
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("misc/throw.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
	}
	if (sound >= 1)
	{
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("misc/throw.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
	}
	if(sound >= 2)
	{
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("misc/swish.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
	}
    // don't need to check for water

    if (tr.fraction < 1.0)        
    {            
        if (tr.ent->takedamage)            
        {
		//	T_Damage (tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_UNKNOWN );
			T_Damage (tr.ent, ent, ent, vec3_origin, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_NO_KNOCKBACK, MOD);
			
			if (ent->solid != SOLID_NOT )
			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/kick.wav"), 1, ATTN_NORM, 0);
			//PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
		}   
    }
        
}
/*
===================
Decide_attack
Type of attack forr boot. varies with jumping, ducking and standing
===================
*/
void Decide_attack (edict_t *ent)
{
	int damage,kick;
	vec3_t	offset, start;
	vec3_t	dir, right;
	vec3_t	v;
	qboolean is_quad;
	
	if (ent->matrixflip == RWALL)//if you're running up a wall
	{
		MatrixJump (ent);	
		ent->jumping = true;
		return;// FEAR - I added this to make it work better
		//used up kick on flip off wall
	}
	if (ent->stamina < 3)
		return;

//	if ((ent->s.frame > FRAME_flip01) && (ent->s.frame < FRAME_point12))
//		return;

	if (!ent->groundentity)
	{
		if (ent->velocity[2] > 0)
			ent->velocity[2] += 10;
		else
			ent->velocity[2] += 50;
	}

	is_quad = (ent->client->quad_framenum > level.framenum);
	//sweep, angled up
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->stamina -= 6;
		kick = 400;
		damage = 25;
		if(is_quad)
		{
		damage *=4;
		kick*=4;
		}
		
		
		v[PITCH] = ent->client->v_angle[PITCH]-45;
		v[YAW]   = ent->client->v_angle[YAW];
		v[ROLL]  = ent->client->v_angle[ROLL];
		
		AngleVectors (v, dir, right, NULL);
		
		VectorScale (dir, -2, ent->client->kick_origin);
		VectorSet(offset, 8, 8, ent->viewheight-16);
		P_ProjectSource (ent->client, ent->s.origin, offset, dir, right, start);
		
		if(ent->kungfu_framenum > level.time)
		KungFu_l33t_Skillz (ent, damage, kick, MOD_SWEEP);
		else
		kick_attack (ent, start, dir , damage, kick, MOD_SWEEP);	
	}
	else if (!ent->groundentity)
	{
		if(ent->velocity[2] > -100)
		{
		if(ent->velocity[2] < 0)
			ent->velocity[2]=+10;
		else
			ent->velocity[2]+=40;
		}
		ent->stamina -= 8;
		damage = 20;
		kick = 300;
		if(is_quad)
		{
			damage *=4;
			kick*=4;
		}
		v[PITCH] = ent->client->v_angle[PITCH]+45;
		v[YAW]   = ent->client->v_angle[YAW];
		v[ROLL]  = ent->client->v_angle[ROLL];
		
		AngleVectors (v, dir, right, NULL);
		
		VectorScale (dir, -2, ent->client->kick_origin);
		VectorSet(offset, 8, 8, ent->viewheight-16);
		P_ProjectSource (ent->client, ent->s.origin, offset, dir, right, start);
		
		if(ent->kungfu_framenum > level.time)
		KungFu_l33t_Skillz (ent, damage, kick, MOD_HOVER);
		else
		kick_attack (ent, start, dir , damage, kick, MOD_HOVER);
	}
	else
	{
		ent->stamina -= 6;
		damage = 20;
		kick = 400;
		if(is_quad)
		{
			damage *=4;
			kick*=4;
		}
		
		VectorScale (dir, -2, ent->client->kick_origin);
		VectorSet(offset, 8, 8, ent->viewheight-16);
		P_ProjectSource (ent->client, ent->s.origin, offset, dir, right, start);
		
		if(ent->kungfu_framenum > level.time)
		KungFu_l33t_Skillz (ent, damage, kick, MOD_KICK);
		else
		kick_attack (ent, start, dir , damage, kick, MOD_KICK);
	}

}


void KungFu_l33t_Skillz (edict_t *ent, int damage, int kick, int mod)//kungfu auto aiming
{	
	edict_t *enemy;
	edict_t *ignore = ent;
	vec3_t	start;
	vec3_t  end;
	vec3_t  dir;
	vec3_t  point;
	trace_t tr;
	
	//enemy = findradius (enemy, ent->s.origin, 48);

	while ((enemy = findradius(NULL, ent->s.origin, 256)) != NULL)
	{
		if (enemy == ent)
		continue;

	if (!enemy->takedamage)
		continue;

	if (!(enemy->svflags & SVF_MONSTER) && (!enemy->client) && (strcmp(enemy->classname, "misc_explobox") != 0))
		continue;

	VectorMA (enemy->absmin, 0.5, enemy->size, point);

	VectorSubtract (point, ent->s.origin, dir);
	VectorNormalize (dir);

	VectorCopy (ent->s.origin, start);
	VectorMA (start, 2048, dir, end);

	tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
	
	if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != ent->owner))
		T_Damage (tr.ent, ent, ent->owner, dir, tr.endpos, vec3_origin, damage, 1, DAMAGE_ENERGY, mod);
	
	//if (!tr.ent)
	//	continue;

	//kick_attack (ent, start, dir, damage, kick, mod);
	}
	
}


void target_laser_think (edict_t *self);
void LineThink  (edict_t *self)
{
	target_laser_think(self);

	self->think = LineThink;
	self->nextthink = level.time + FRAMETIME;

	if (!(self->owner->client->buttons) & BUTTON_ATTACK)
	{
		//self->think = G_FreeEdict;
		//self->nextthink = level.time + FRAMETIME;
		VectorCopy(vec3_origin, self->s.origin);
		VectorCopy(vec3_origin, self->s.old_origin);
		VectorCopy(vec3_origin, self->movedir);
	}
}
// draws a translucent line from spos to epos
edict_t *DrawLine(edict_t *owner, vec3_t spos, vec3_t epos)
{
	edict_t *beam;

	beam = G_Spawn();

	beam->owner = owner;
	beam->spawnflags = 1 | 4;
	beam->classname = "path_beam";

	beam->movetype = MOVETYPE_NONE;
	beam->solid = SOLID_NOT;
	beam->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	beam->s.modelindex = 1; // must be non-zero
	beam->activator = owner;
	// set the beam diameter
	beam->s.frame = 2;// The thinest it can be
	// set the color (red)
	if (owner->client->resp.team == TEAM_RED)
		beam->s.skinnum =  0xf2f2f0f0;// RED
	else if (owner->client->resp.team == TEAM_BLUE)
		beam->s.skinnum =  0xf3f3f1f1;// BLUE
	else
		beam->s.skinnum =  0xf2f2f0f0;// red

	VectorSet (beam->mins, -1, -1, -1);
	VectorSet (beam->maxs, 1, 1, 1);

	beam->spawnflags |= 0x80000001;
	beam->svflags &= ~SVF_NOCLIENT;
	beam->flags |= FL_TEAMSLAVE;

	VectorCopy(spos, beam->s.origin);
	VectorCopy(epos, beam->s.old_origin);

	VectorSubtract(epos, spos, beam->movedir);
//	VectorNormalize2(beam->movedir, beam->movedir);

	beam->dmg = 0;
	beam->enemy = NULL;

	beam->think = LineThink;//target_laser_think;
	beam->nextthink = level.time + FRAMETIME;
	beam->think(beam);

	gi.linkentity (beam);

	return beam;
}
void MatrixDrawBeam(edict_t *ent)
{
	vec3_t start, end, offset, f, r, u, dir;
//	float	distance;

//	VectorCopy (ent->s.origin, start);
	AngleVectors (ent->client->v_angle, f, r, NULL);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, f, r, start);
	

	VectorMA (start, 8192, f, end);
	
	if (!ent->beam)
		ent->beam = DrawLine(ent, start, end);
	else
	{
		VectorCopy(start, ent->beam->s.origin);
		VectorCopy(end, ent->beam->s.old_origin);

		VectorSubtract(end, start, ent->beam->movedir);
	}

}


void MatrixChuckShells(edict_t *ent, int model)
{
	edict_t *gun;
	vec3_t  start, end, offset, forward, right, up;
	
	if(shellsoff->value)
		return;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 8,  ent->viewheight-8);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	
	gun = G_Spawn();
	

	VectorCopy(start, gun->s.origin);
	VectorSet(gun->avelocity, (100+200*crandom()), (100+200*crandom()), (100+200*crandom()));

	VectorScale (right, 200, right);
	VectorInverse(right);
	VectorScale (forward, 300, gun->velocity);
	VectorAdd(right, gun->velocity, gun->velocity);
	gun->velocity[2] += 600;
	VectorNormalize(gun->velocity);
	VectorScale(gun->velocity, 200, gun->velocity);
	
	gun->velocity[0] += crandom()*20;
	gun->velocity[1] += crandom()*20;
	gun->movetype = MOVETYPE_TOSS;
	gun->nextthink = level.time + 2;
	gun->think = G_FreeEdict;
	gun->s.modelindex = model;//gi.modelindex(model"models/objects/shell1/tris.md2"); also shell2
		//shell 1 = small bore casing
		//shell 2 = red shotgun casing
	gi.linkentity (gun);
}
void MatrixChuckGun(edict_t *ent)
{
	edict_t *gun;

	gun = G_Spawn();
	
    VectorCopy(ent->s.origin, gun->s.origin);
	VectorSet(gun->avelocity, 0, 800*crandom() - 400, 0);//spin around y axis.
	gun->s.angles[ROLL] = 60;
	gun->velocity[2] = 200;
	gun->velocity[0] = 200*crandom()-100;
	gun->velocity[1] = 200*crandom()-100;

	
	gun->movetype = MOVETYPE_TOSS;
	gun->nextthink = level.time + 15;
	gun->think = G_FreeEdict;
	gi.setmodel (gun, ent->client->pers.weapon->world_model);
	gi.linkentity (gun);
}


/*
=================
MatrixNoAmmoWeaponChange
=================
*/
void MatrixNoAmmoWeaponChange (edict_t *ent)
{
	ent->client->newweapon = FindItem ("Fists of Fury");
	//ent->client->pers.weapon->view_model = ent->client->newweapon->view_model;
	ent->client->akimbo =false;
	
}

/*
===============
MatrixChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void MatrixChangeWeapon (edict_t *ent)
{
	int i;
	if (ent->client->pers.inventory[ITEM_INDEX(ent->client->newweapon)] < 1)
		MatrixNoAmmoWeaponChange(ent);
	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	// set visible model
	if (ent->s.modelindex == 255) 
	{
		if (ent->client->pers.weapon)
			i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		else
			i = 0;
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}

    if (!ent->client->pers.weapon)
	{	// dead
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
	
	if(ent->client->pers.weapon->akimbo_model)
	ent->client->akimbo = true;
	
	if(ent->client->akimbo)
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->akimbo_model);
	else
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
        //SKULL
        /* Don't display weapon if chasetoggle is on */
	if (ent->client->chasetoggle)
		ent->client->ps.gunindex = 0;
        //END

	ent->client->anim_priority = ANIM_PAIN;
	if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
			ent->s.frame = FRAME_crpain1;
			ent->client->anim_end = FRAME_crpain4;
	}
	else
	{
			ent->s.frame = FRAME_pain301;
			ent->client->anim_end = FRAME_pain304;
			
	}
}
#define FRAME_FIRE_FIRST		(FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST		(FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST	(FRAME_IDLE_LAST + 1)

void MatrixWeaponGeneric (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
		return;

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			// CHUCK AND EMPTY
			MatrixChuckGun(ent);
			ent->left_in_clip = 0;
			ent->client->pers.inventory[ITEM_INDEX(ent->client->pers.weapon)]--;
			MatrixChangeWeapon (ent);
			return;
		}
		else if ((FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_wave08;
				ent->client->anim_end = FRAME_wave01;
				
			}
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		{
/*			int ammo;
			ammo = FindItem(ent->client->pers.weapon->ammo)->quantity;
			if (ammo)
				ent->left_in_clip += ammo;
*/
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

		if ((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_wave08;
				ent->client->anim_end = FRAME_wave01;
				
			}
		}
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) || ent->secondaryfire  )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->left_in_clip > 0)
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
				if ( !((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) || ent->secondaryfire  && (ent->velocity))
		
				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					ent->s.frame = FRAME_crattak1-1;
					ent->client->anim_end = FRAME_crattak9;
				}
				else
				{
					ent->s.frame = FRAME_attack1-1;
					ent->client->anim_end = FRAME_attack8;
				}
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				MatrixNoAmmoWeaponChange (ent);
			}
		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			{
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			if (pause_frames)
			{
				for (n = 0; pause_frames[n]; n++)
				{
					if (ent->client->ps.gunframe == pause_frames[n])
					{
						if (rand()&15)
							return;
					}
				}
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
	}
}
//----------------------------------------------------------------------------------------//
//----------------------------        Matrix Weapons     ---------------------------------//
//----------------------------------------------------------------------------------------//

void Matrix_Sniper_Fire (edict_t *ent)
{
	qboolean	is_quad = (ent->client->quad_framenum > level.framenum);
	byte		is_silenced = (ent->client->silencer_shots) ? MZ_SILENCED : 0;
	vec3_t		start,end, blag;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;
	int         speed=2000;
	trace_t		tr;

	//sniper scope test:
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
	{	
		AngleVectors (ent->client->v_angle, forward, NULL, NULL);
		VectorMA(ent->s.origin, 1500, forward, blag);
		VectorSet(offset, 0, 7,  ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		tr = gi.trace(start, vec3_origin, vec3_origin, blag, ent, MASK_MONSTERSOLID);

		if (!laseroff->value)
		{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (tr.endpos, MULTICAST_PHS);
		}

		if(ent->forwardbuttons || ent->strafebuttons)
		{
			ent->client->ps.fov = 90;
			return;
		}
		else
		{
		
			
			Matrix_SniperZoom (ent);
			return;
		}
	}

	
	//fix for secondary fire.
	if (deathmatch->value)
	{	// normal damage is too extreme in dm
		//damage = 120;
		damage = damage_sniper->value;
		kick = 400;
		
	//	gi.cprintf (ent, PRINT_HIGH, "Damage done:%i\n", damage);
	}
	else
	{
		// normal damage is too extreme in dm - grr i don't care :p
		damage = damage_sniper->value;
		kick = 400;
	}
	
	ent->client->ps.fov = 90;

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

//SKULL
        if (ent->client->use)
                AngleVectors (ent->client->oldplayer->s.angles, forward, right, NULL);
        else
//END
	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
//(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
	fire_matrix_bullet(ent, start, forward, damage, speed, 0.5,5, MOD_SNIPER);
	gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/sniprfire.wav"), 1, ATTN_IDLE, 0);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
//SKULL
        		if (ent->client->use)
                gi.WriteShort (ent->client->oldplayer-g_edicts);
        else
                gi.WriteShort (ent-g_edicts);
//END
	gi.WriteByte (MZ_AK_MK23 | is_silenced);
//SKULL
        //if (ent->client->oldplayer)
		if (ent->client->use)
                gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
        else
                gi.multicast (ent->s.origin, MULTICAST_PVS);

//END

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		ent->client->pers.inventory[ent->client->ammo_index]-= ammo_sniper->value;
//		ent->left_in_clip--;
	}
}
void Matrix_Sniper (edict_t *ent)
{
	// FORTRESS MODEL
//	static int	pause_frames[]	= {56, 0};
//	static int	fire_frames[]	= {24, 0};

//	Weapon_Generic(ent, 23, 28, 49, 53, pause_frames, fire_frames, Matrix_Sniper_Fire);
	static int      pause_frames[]  = {21, 40};
	static int      fire_frames[]   = {9, 0};
        
	Weapon_Generic (ent, 8, 21, 41, 50, pause_frames, fire_frames, Matrix_Sniper_Fire, ammo_sniper->value);
}

void Matrix_ak_92f_Fire (edict_t *ent)
{
	qboolean	is_quad = (ent->client->quad_framenum > level.framenum);
	byte		is_silenced = (ent->client->silencer_shots) ? MZ_SILENCED : 0;

}
void Matrix_ak_92f (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

	Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, Matrix_ak_92f_Fire);
}

void Matrix_ak_mk23_Fire (edict_t *ent)
{
	qboolean	is_quad = (ent->client->quad_framenum > level.framenum);
	byte		is_silenced = (ent->client->silencer_shots) ? MZ_SILENCED : 0;
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = damage_mk23->value;
	int			kick = 2;
	int			speed= 5000;
	vec3_t		offset;


	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
//SKULL
    if (ent->client->use)
            VectorAdd (ent->client->oldplayer->s.angles, ent->client->kick_angles, angles);
    else
//END

	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);

	if(ent->secondaryfire && ent->client->akimbo && ent->client->pers.inventory[ent->client->ammo_index]> ammo_mk23->value *2)
{

	
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, -8, ent->viewheight-8);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
//(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
	fire_matrix_bullet (ent, start, forward, damage, speed, 10, 0.5, MOD_AK_MK23);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23fire.wav"), 1, ATTN_IDLE, 0);

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_matrix_bullet (ent, start, forward, damage, speed, 10, 0.5, MOD_AK_MK23);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23fire.wav"), 1, ATTN_IDLE, 0);

	ent->client->pers.inventory[ent->client->ammo_index]-=ammo_mk23->value;

}
else
{
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_matrix_bullet (ent, start, forward, damage, speed, 5, 0.5, MOD_AK_MK23);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23fire.wav"), 1, ATTN_IDLE, 0);
}
//	fire_streak (ent, start, forward, damage);
//	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);

	gi.WriteByte (svc_muzzleflash);
		if (ent->client->use)
                gi.WriteShort (ent->client->oldplayer-g_edicts);
        else
                gi.WriteShort (ent-g_edicts);
//END
	gi.WriteByte (MZ_AK_MK23 | is_silenced);
//SKULL
        //if (ent->client->oldplayer)
		if (ent->client->use)
                gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
        else
                gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		MatrixChuckShells(ent,gi.modelindex("models/objects/shell1/tris.md2"));
//		ent->left_in_clip--;
		ent->client->pers.inventory[ent->client->ammo_index]-=ammo_mk23->value;
	}
	ent->client->ps.gunframe++;
}
/*void Matrix_ak_mk23 (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {4, 8, 0};

	Weapon_Generic (ent, 4, 9, 31, 34, pause_frames, fire_frames, Matrix_ak_mk23_Fire);
}*/


void Matrix_pistols (edict_t *ent)
{
	if (ent->client->akimbo)
	{
		static int	pause_frames[]	= {19, 32, 0};
		static int	fire_frames[]	= {9, 40, 0};
		Akimbo_Weapon_Generic (ent, 8, 10, 36, 39, 40, 41, pause_frames, fire_frames, Matrix_ak_mk23_Fire, ammo_mk23->value);

	}
	else
	{
		static int	pause_frames[]	= {0};
		static int	fire_frames[]	= {10, 0};

		Weapon_Generic (ent, 9, 12, 41, 43, pause_frames, fire_frames, Matrix_ak_mk23_Fire, ammo_mk23->value);
	}
}

void Matrix_m4_Fire (edict_t *ent)
{
	qboolean	is_quad = (ent->client->quad_framenum > level.framenum);
	byte		is_silenced = (ent->client->silencer_shots) ? MZ_SILENCED : 0;
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = damage_m4->value;
	int			kick = 2;
	int			kickback;
	int			speed=2000;
	vec3_t		offset;


	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}
    if (!(ent->client->buttons & BUTTON_ATTACK) && !ent->secondaryfire)
	{
		ent->client->ps.gunframe++;
		ent->client->machinegun_shots = 0;
		return;
	}

        //Calculate the kick angles
	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.25;
		ent->client->kick_angles[i] = crandom() * 0.5;
	}
	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -.7;
	// get start / end positions
//SKULL
    if (ent->client->use)
            VectorAdd (ent->client->oldplayer->s.angles, ent->client->kick_angles, angles);
    else
//END

if(ent->secondaryfire && ent->client->akimbo && ent->client->pers.inventory[ent->client->ammo_index]> ammo_m4->value *2)
{

	
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, -8, ent->viewheight-5);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	//(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)

	fire_matrix_bullet (ent, start, forward, damage, speed, 10, 0.5, MOD_HYPERBLASTER);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/m4a1fire.wav"), 1, ATTN_IDLE, 0);
	
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-5);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_matrix_bullet (ent, start, forward, damage, speed, 10, 0.5, MOD_HYPERBLASTER);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/m4a1fire.wav"), 1, ATTN_IDLE, 0);

	kickback = kick*1.2;
	KickBack(ent, forward, kickback);

	ent->client->pers.inventory[ent->client->ammo_index] -= ammo_m4->value;

}
else
{
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-5);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_matrix_bullet (ent, start, forward, damage, speed, 10, 0.5, MOD_HYPERBLASTER);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/m4a1fire.wav"), 1, ATTN_IDLE, 0);
	kickback = kick;
	KickBack(ent, forward, kickback);
	//m4's have huge kickback which knock you physically backwards
}
//	fire_streak (ent, start, forward, damage);
//	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);

	gi.WriteByte (svc_muzzleflash);
	
		if (ent->client->use)
                gi.WriteShort (ent->client->oldplayer-g_edicts);
        else
                gi.WriteShort (ent-g_edicts);
//END
	gi.WriteByte (MZ_AK_MK23 | is_silenced);
//SKULL
        //if (ent->client->oldplayer)
		if (ent->client->use)
                gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
        else
                gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		
			ent->client->pers.inventory[ent->client->ammo_index] -= ammo_m4->value;;
			MatrixChuckShells(ent,gi.modelindex("models/objects/shell1/tris.md2"));
		
	}
	ent->client->ps.gunframe++;
}
void Matrix_m4 (edict_t *ent)
{

	if (ent->client->akimbo)
	{
		static int	pause_frames[]	= {0};
		static int	fire_frames[]	= {13, 14, 24, 25, 26, 27, 0};

		Akimbo_Weapon_Generic (ent, 12, 14, 21, 23, 24, 25, pause_frames, fire_frames, Matrix_m4_Fire, ammo_m4->value);

	}
	else
	{
		static int	pause_frames[]	= {13, 24, 39};
		static int	fire_frames[]	= {11, 12, 0};

		Weapon_Generic (ent, 10, 12, 39, 44, pause_frames, fire_frames, Matrix_m4_Fire, ammo_m4->value);
	}

}

void Matrix_mp5_Fire (edict_t *ent)
{
	qboolean	is_quad = (ent->client->quad_framenum > level.framenum);
	byte		is_silenced = (ent->client->silencer_shots) ? MZ_SILENCED : 0;
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = damage_mp5->value;
	int			kick = 2;
	int			speed = 2000;
	vec3_t		offset;


	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
//SKULL
    if (ent->client->use)
            VectorAdd (ent->client->oldplayer->s.angles, ent->client->kick_angles, angles);
    else
//END

	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);

	if(ent->secondaryfire && ent->client->akimbo && ent->client->pers.inventory[ent->client->ammo_index]> ammo_mp5->value *2)
{

	
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, -8, ent->viewheight-8);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_matrix_bullet (ent, start, forward, damage, speed, 10, 0.5, MOD_HYPERBLASTER);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mp5fire1.wav"), 1, ATTN_IDLE, 0);

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_matrix_bullet (ent, start, forward, damage, speed, 10, 0.5, MOD_HYPERBLASTER);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mp5fire1.wav"), 1, ATTN_IDLE, 0);

	ent->client->pers.inventory[ent->client->ammo_index]-=ammo_mp5->value;

}
else
{
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_matrix_bullet (ent, start, forward, damage, speed, 10, 0.5, MOD_HYPERBLASTER);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mp5fire1.wav"), 1, ATTN_IDLE, 0);
}
//	fire_streak (ent, start, forward, damage);
//	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);

	gi.WriteByte (svc_muzzleflash);
		if (ent->client->use)
                gi.WriteShort (ent->client->oldplayer-g_edicts);
        else
                gi.WriteShort (ent-g_edicts);
//END
	gi.WriteByte (MZ_AK_MK23 | is_silenced);
//SKULL
        //if (ent->client->oldplayer)
		if (ent->client->use)
                gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
        else
                gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		MatrixChuckShells(ent,gi.modelindex("models/objects/shell1/tris.md2"));
//		ent->left_in_clip--;
		ent->client->pers.inventory[ent->client->ammo_index]-=ammo_mp5->value;
	}
	ent->client->ps.gunframe++;
}

void Matrix_mp5 (edict_t *ent)
{


	if (ent->client->akimbo)
	{
		static int	pause_frames[]	= { 8, 40};
		static int	fire_frames[]	= { 6, 7, 49, 50};

		Akimbo_Weapon_Generic (ent, 5, 7, 42, 48, 49, 50, pause_frames, fire_frames, Matrix_mp5_Fire, ammo_mp5->value);

	}
	else
	{
		static int	pause_frames[]	= {13, 24, 39};
		static int	fire_frames[]	= {11, 12, 0};

		Weapon_Generic (ent, 10, 12, 47, 51, pause_frames, fire_frames, Matrix_mp5_Fire, ammo_mp5->value);
	}
}


void Matrix_SMC_Fire (edict_t *ent)
{
	byte		is_silenced = (ent->client->silencer_shots) ? MZ_SILENCED : 0;
	int			i;
	int			shots;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset;
	int			damage;
	int			kick = 2;
	int			speed = 2000;

	
		damage = damage_smc->value;

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&& ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22)
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
	}

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_attack8;
	}

	if (ent->client->ps.gunframe <= 9)
		shots = 1;
	else if (ent->client->ps.gunframe <= 14)
	{
		if (ent->client->buttons & BUTTON_ATTACK)
			shots = 2;
		else
			shots = 1;
	}
	else
		shots = 2;

	if(ent->secondaryfire)
	{
		shots *= 2;
	if (ent->client->pers.inventory[ent->client->ammo_index] < shots * ammo_smc->value)
		shots = ent->client->pers.inventory[ent->client->ammo_index] / ammo_smc->value;
	}
	else
	{
	if (ent->client->pers.inventory[ent->client->ammo_index] < shots * ammo_smc->value)
		shots = ent->client->pers.inventory[ent->client->ammo_index] / ammo_smc->value;
	}

	if (!shots)
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}



	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	for (i=0 ; i<shots ; i++)
	{
		// get start / end positions
//SKULL
                if (ent->client->use)
                        AngleVectors (ent->client->oldplayer->s.angles, forward, right, NULL);
                else
//END
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		
		
		if(ent->secondaryfire)
		{	
		fire_matrix_bullet (ent, start, forward, damage, speed, 10, 0.5, MOD_CHAINGUN);
		fire_matrix_bullet (ent, start, forward, damage, speed, 10, 0.5, MOD_CHAINGUN);
		ent->client->pers.inventory[ent->client->ammo_index] -= ammo_smc->value;
		}
		else
		fire_matrix_bullet (ent, start, forward, damage, speed, 10, 0.5, MOD_CHAINGUN);
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
//SKULL
        if (ent->client->oldplayer)
                gi.WriteShort (ent->client->oldplayer-g_edicts);
        else
                gi.WriteShort (ent-g_edicts);
//END
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
//SKULL
        if (ent->client->oldplayer)
                gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
        else
                gi.multicast (ent->s.origin, MULTICAST_PVS);
//END

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= shots * ammo_smc->value;
}
void Matrix_SMC (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Matrix_SMC_Fire, ammo_smc->value);

}
void Matrix_Knives_Fire (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t  offset, start;
    trace_t tr;
    vec3_t end;
	gitem_t	*item;
	int damage = damage_knife->value;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
    VectorScale (forward, 0, ent->client->kick_origin);
    VectorSet(offset, 0, 0,  ent->viewheight-4);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
    
    VectorMA( start, 40, forward, end );
    
    tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_MONSTERSOLID);

	if(ent->secondaryfire)
	{
                // do throwing stuff here
                
                
                
                // throwing sound
                gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/throw.wav"), 1, ATTN_NORM, 0);

                                
                                // below is exact code from action for how it sets the knife up
                                AngleVectors (ent->client->v_angle, forward, right, NULL);
                                VectorSet(offset, 24, 8, ent->viewheight-8);
                                VectorAdd (offset, vec3_origin, offset);
                                forward[2] += .17;

                                // zucc using old style because the knife coming straight out looks
                                // pretty stupid
                                
                                
                                P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

                item = FindItem("Gung Ho Knives");
                ent->client->pers.inventory[ITEM_INDEX(item)]-=ammo_knife->value;
                
                      
                                                // zucc was at 1250, dropping speed to 1200
                        knife_throw (ent, start, forward, damage, 1200); 

						ent->client->pers.inventory[ent->client->ammo_index] -= ammo_knife->value;
						ent->client->ps.gunframe++;
						return;
                }
else if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
{
	AngleVectors (ent->client->v_angle, forward, right, NULL);
    VectorScale (forward, 0, ent->client->kick_origin);
    VectorSet(offset, 0, 0,  ent->viewheight-4);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
    
    VectorMA( start, 40, forward, end );
    
    tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_MONSTERSOLID);
	
	VectorSubtract(ent->s.origin, end, end);
	VectorNormalize(end);
	
	if(tr.fraction<1 && tr.ent->takedamage)
	{
		T_Damage (tr.ent, ent, ent, end, tr.ent->s.origin, tr.ent->s.origin, damage_knife->value, 100, 1, MOD_KNIFE);
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/stab.wav"), 1, ATTN_NORM, 0);
	}
	else return;


}

ent->client->ps.gunframe++;

}

void Matrix_Knives (edict_t *ent)
{
		static int	pause_frames[]	= {13, 24, 51};
		static int	fire_frames[]	= {6, 103, 0};

		Weapon_Generic (ent, 5, 12, 52, 59, pause_frames, fire_frames, Matrix_Knives_Fire, ammo_knife->value);
}
void Matrix_Pumps_Fire (edict_t *ent)
{
	byte		is_silenced = (ent->client->silencer_shots) ? MZ_SILENCED : 0;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = damage_pumps->value;
	int			kick = 8;

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

//SKULL
        if (ent->client->use)
                AngleVectors (ent->client->oldplayer->s.angles, forward, right, NULL);
        else
//END
	AngleVectors (ent->client->v_angle, forward, right, NULL);

if(ent->secondaryfire && ent->client->akimbo && ent->client->pers.inventory[ent->client->ammo_index]> ammo_pumps->value * 2)
{

	
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, -8, ent->viewheight-8);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_shotgun (ent, start, forward, damage, kick, 1200, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);

	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/shotgf1b.wav"), 1, ATTN_IDLE, 0);
	

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_shotgun (ent, start, forward, damage, kick, 1200, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
	
	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/shotgf1b.wav"),1, ATTN_IDLE, 0);
	VectorScale (forward, -10, ent->client->kick_origin);
	ent->client->kick_angles[0] = -10;

}
else
{
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-6);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/shotgf1b.wav"), 1, ATTN_IDLE, 0);
}		

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
//SKULL
//        if (ent->client->oldplayer)
//              gi.WriteShort (ent->client->oldplayer-g_edicts);
//    else
                gi.WriteShort (ent-g_edicts);
//END
	gi.WriteByte (MZ_AK_MK23 | is_silenced);
//SKULL
//        if (ent->client->oldplayer)
//              gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
//    else
                gi.multicast (ent->s.origin, MULTICAST_PVS);
//END

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);
	
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		if(ent->secondaryfire && ent->client->akimbo && ent->client->pers.inventory[ent->client->ammo_index]> ammo_pumps->value *2)
		{
		ent->client->pers.inventory[ent->client->ammo_index]-= ammo_pumps->value;
		MatrixChuckShells(ent,gi.modelindex("models/objects/shell2/tris.md2"));
		}
		ent->client->pers.inventory[ent->client->ammo_index]-= ammo_pumps->value;
		MatrixChuckShells(ent,gi.modelindex("models/objects/shell2/tris.md2"));
		
	}

}
void Matrix_Pumps (edict_t *ent)
{


	if (ent->client->akimbo)
	{
		static int	pause_frames[]	= {0};
		static int	fire_frames[]	= {8, 53, 0};

		Akimbo_Weapon_Generic (ent, 7, 15, 35, 41, 53, 60, pause_frames, fire_frames, Matrix_Pumps_Fire, ammo_pumps->value);

	}
	else
	{
		static int	pause_frames[]	= {16, 24, 37};
		static int	fire_frames[]	= {8, 0};

		Weapon_Generic (ent, 7, 15, 35, 41, pause_frames, fire_frames, Matrix_Pumps_Fire, ammo_pumps->value);
	}
}


void Matrix_Fists_Fire (edict_t *ent)
{

	vec3_t	forward, right;
	vec3_t  offset, start;
    trace_t tr;
    vec3_t end;


	AngleVectors (ent->client->v_angle, forward, right, NULL);
    VectorScale (forward, 0, ent->client->kick_origin);
    VectorSet(offset, 0, 0,  ent->viewheight-4);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
    
    VectorMA( start, 40, forward, end );
    
    tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_MONSTERSOLID);
	
	VectorSubtract(end, ent->s.origin, end);
	VectorNormalize(end);
	
if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
{
	if(tr.fraction<1 && tr.ent->takedamage)
	{
		T_Damage (tr.ent, ent, ent, end, tr.ent->s.origin, tr.ent->s.origin, damage_fist->value, 500, 1, MOD_HOOK);
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/kick.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);

	}
	else return;

}
else
{
T_Damage (tr.ent, ent, ent, end, tr.ent->s.origin, tr.ent->s.origin, damage_fist->value/5, 10, 1, MOD_HOOK);
//ent->client->ps.gunframe = 18;
if(tr.fraction<1 && tr.ent->takedamage)
	{
			gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/kick.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
	}
}
swoosh (ent);//swoosh noise

ent->client->ps.gunframe++;
}
void Matrix_Fists (edict_t *ent)
{
		static int	pause_frames[]	= {21, 24, 51};
		static int	fire_frames[]	= {6, 0};

		Weapon_Generic (ent, 5, 20, 52, 58, pause_frames, fire_frames, Matrix_Fists_Fire, 0);

}

void Matrix_Deserts_Fire (edict_t *ent)
{
		qboolean	is_quad = (ent->client->quad_framenum > level.framenum);
	byte		is_silenced = (ent->client->silencer_shots) ? MZ_SILENCED : 0;
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = damage_deserts->value;
	int			kick = 2;
	vec3_t		offset;
	int			speed=2000;


	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	// get start / end positions
//SKULL
    if (ent->client->use)
            VectorAdd (ent->client->oldplayer->s.angles, ent->client->kick_angles, angles);
    else
//END

	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);

if(ent->secondaryfire && ent->client->akimbo && ent->client->pers.inventory[ent->client->ammo_index] > ammo_deserts->value * 2)
{

	
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, -8, ent->viewheight-8);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_matrix_bullet (ent, start, forward, damage, speed, 10, 0.5, MOD_AK_MK23);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23fire.wav"), 1, ATTN_IDLE, 0);

	MatrixChuckShells(ent,gi.modelindex("models/objects/shell1/tris.md2"));

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_matrix_bullet (ent, start, forward, damage, speed, 10, 0.5, MOD_AK_MK23);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23fire.wav"), 1, ATTN_IDLE, 0);

	ent->client->pers.inventory[ent->client->ammo_index]-= ammo_deserts->value;

}
else
{
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	Akimbo_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_matrix_bullet (ent, start, forward, damage, speed, 10, 0.5, MOD_AK_MK23);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/mk23fire.wav"), 1, ATTN_IDLE, 0);
}
//	fire_streak (ent, start, forward, damage);
//	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);

	gi.WriteByte (svc_muzzleflash);
//SKULL
		if (ent->client->use)
                gi.WriteShort (ent->client->oldplayer-g_edicts);
        else
                gi.WriteShort (ent-g_edicts);
//END
	gi.WriteByte (MZ_AK_MK23 | is_silenced);
//SKULL
        //if (ent->client->oldplayer)
		if (ent->client->use)
                gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
        else
                gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		MatrixChuckShells(ent,gi.modelindex("models/objects/shell1/tris.md2"));
//		ent->left_in_clip--;
		ent->client->pers.inventory[ent->client->ammo_index]-= ammo_deserts->value;
	}

ent->client->ps.gunframe++;
}
void Matrix_Deserts (edict_t *ent)
{


	if (ent->client->akimbo)
	{
		static int	pause_frames[]	= {19, 32, 0};
		static int	fire_frames[]	= {9, 40, 0};
		Akimbo_Weapon_Generic (ent, 8, 11, 36, 39, 40, 42, pause_frames, fire_frames, Matrix_Deserts_Fire, ammo_deserts->value);

	}
	else
	{
		static int	pause_frames[]	= {0};
		static int	fire_frames[]	= {10, 0};

		Weapon_Generic (ent, 9, 13, 41, 43, pause_frames, fire_frames, Matrix_Deserts_Fire, ammo_deserts->value);
	}
}

void Matrix_SSMRack_Fire (edict_t *ent)
{
	byte		is_silenced = (ent->client->silencer_shots) ? MZ_SILENCED : 0;
	vec3_t	offset, start;
	vec3_t	forward, right;
	vec3_t	babang;
	int		damage, i;
	float	damage_radius;
	int		radius_damage;

	damage = fabs(radiusdamage_rack->value - 20);
	radius_damage = radiusdamage_rack->value;
	damage_radius = damageradius_rack->value;



//SKULL
        if (ent->client->use)
                AngleVectors (ent->client->oldplayer->s.angles, forward, right, NULL);
        else
//END
	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	
	if (ent->client->pers.inventory[ent->client->ammo_index] < 50)
	{
		ent->client->ps.gunframe++;
		return;
	}

	if(ent->secondaryfire)
	{
	
	damage = fabs(radiusdamage_rack->value - 40);
	radius_damage = fabs(radiusdamage_rack->value - 40);
	damage_radius = fabs(damageradius_rack->value - 20);

	
	if(ent->client->pers.inventory[ent->client->ammo_index]>= ammo_rack->value * 4)
	{
		fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);
		matrix_fire_rocket (ent, start, forward, damage, 400, damage_radius, radius_damage);
	}
	else if(ent->client->pers.inventory[ent->client->ammo_index]>= ammo_rack->value * 3 && ent->client->pers.inventory[ent->client->ammo_index] < ammo_rack->value * 4)
		matrix_fire_rocket (ent, start, forward, damage, 400, damage_radius, radius_damage);
	else if(ent->client->pers.inventory[ent->client->ammo_index]>=ammo_rack->value * 2 && ent->client->pers.inventory[ent->client->ammo_index]< ammo_rack->value * 3)
	{
		for(i=0;i<2;i++)
		{
		VectorCopy(ent->client->v_angle, babang);
		babang[0] += crandom() * 3;
		babang[1] += crandom() * 3;

		AngleVectors (babang, forward, right, NULL);
		VectorScale (forward, -2, ent->client->kick_origin);
	
		VectorSet(offset, 8, 8, ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);
		}
	}
	else
	if(ent->client->pers.inventory[ent->client->ammo_index]>= ammo_rack->value)
	fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);

	
	}
	else
	if(ent->client->pers.inventory[ent->client->ammo_index]>= ammo_rack->value)
	fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);
	
	KickBack(ent, forward, 10);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);

//if (ent->client->oldplayer)
		if (ent->client->use)
                gi.WriteShort (ent->client->oldplayer-g_edicts);
        else
                gi.WriteShort (ent-g_edicts);
//END
	gi.WriteByte (MZ_ROCKET | is_silenced);
//SKULL
        //if (ent->client->oldplayer)
		if (ent->client->use)
                gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
        else
                gi.multicast (ent->s.origin, MULTICAST_PVS);
//SKULL
        
	


	ent->client->ps.gunframe++;

	if(ent->client->pers.inventory[ent->client->ammo_index] < ammo_rack->value)
		NoAmmoWeaponChange (ent);

	PlayerNoise(ent, ent->s.origin, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		if(ent->secondaryfire)
		{
			if(ent->client->pers.inventory[ent->client->ammo_index]>= ammo_rack->value  
				&& ent->client->pers.inventory[ent->client->ammo_index]< ammo_rack->value * 2)
					ent->client->pers.inventory[ent->client->ammo_index] -=ammo_rack->value;
			if(ent->client->pers.inventory[ent->client->ammo_index]>= ammo_rack->value * 2 
				&& ent->client->pers.inventory[ent->client->ammo_index]< ammo_rack->value * 3)
					ent->client->pers.inventory[ent->client->ammo_index] -=ammo_rack->value * 2;
			else if(ent->client->pers.inventory[ent->client->ammo_index]>= ammo_rack->value * 3 
				&& ent->client->pers.inventory[ent->client->ammo_index]< ammo_rack->value * 4)
					ent->client->pers.inventory[ent->client->ammo_index] -=ammo_rack->value * 3;
			else if(ent->client->pers.inventory[ent->client->ammo_index]>= ammo_rack->value * 4)
					ent->client->pers.inventory[ent->client->ammo_index] -=ammo_rack->value * 4;
		}
	else
		ent->client->pers.inventory[ent->client->ammo_index]-= ammo_rack->value;
	}
}

void Matrix_SSMRack (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};
		
		Weapon_Generic (ent, 4, 12, 50, 54, pause_frames, fire_frames, Matrix_SSMRack_Fire, ammo_rack->value);
}

void weapon_grenade_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = radiusdamage_grenade->value;
	float	timer;
	int		speed;
	float	radius;
	qboolean held = false;
	radius = damageradius_grenade->value;

	if(ent->client->pers.inventory[ent->client->ammo_index] < ammo_grenade->value)
	{
		ent->client->ps.gunframe = 40;
		ent->client->weaponstate = WEAPON_DROPPING;
		NoAmmoWeaponChange (ent);
		return;
	}

	if ( (ent->client->buttons) & BUTTON_ATTACK) 
	{
		if(ent->client->ps.fov >= 130)
			ent->client->ps.fov = 130;
		else
		ent->client->ps.fov += 4;

		return;
	}
	speed = (ent->client->ps.fov - 90) * 60;
	ent->client->ps.fov = 90;

	VectorSet(offset, 8, 8, ent->viewheight-8);
//SKULL
        if (ent->client->use)
                AngleVectors (ent->client->oldplayer->s.angles, forward, right, NULL);
        else
//END
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = 1;
	if(ent->client->pers.inventory[ent->client->ammo_index] >= ammo_grenade->value)
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]-= ammo_grenade->value;

	ent->client->grenade_time = level.time;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
	ent->client->ps.gunframe ++;
}

void Weapon_Grenade (edict_t *ent)
{

	static int	pause_frames[]	= {20, 24, 30, 0};
	static int	fire_frames[]	= {8, 0};

	Weapon_Generic (ent, 7, 15, 39, 41, pause_frames, fire_frames, weapon_grenade_fire, ammo_grenade->value);
}
void weapon_kuml_fire (edict_t *ent)
{
	byte		is_silenced = (ent->client->silencer_shots) ? MZ_SILENCED : 0;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = 6;
	int			kick = 12;

//SKULL
        if (ent->client->use)
                AngleVectors (ent->client->oldplayer->s.angles, forward, right, NULL);
        else
//END
	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);



//SKULL
        if (ent->client->use)
        {
                v[PITCH] = ent->client->oldplayer->s.angles[PITCH];
                v[YAW]   = ent->client->oldplayer->s.angles[YAW] - 5;
                v[ROLL]  = ent->client->oldplayer->s.angles[ROLL];
        }
        else
        {
                v[PITCH] = ent->client->v_angle[PITCH];
                v[YAW]   = ent->client->v_angle[YAW] - 5;
                v[ROLL]  = ent->client->v_angle[ROLL];
        }
	AngleVectors (v, forward, NULL, NULL);
	fire_kuml (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
        if (ent->client->use)
                v[YAW]   = ent->client->oldplayer->s.angles[YAW] + 5;
        else
//END
	v[YAW]   = ent->client->v_angle[YAW] + 5;
	AngleVectors (v, forward, NULL, NULL);
	fire_kuml (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
//SKULL
        if (ent->client->use)
                gi.WriteShort (ent->client->oldplayer-g_edicts);
        else
                gi.WriteShort (ent-g_edicts);
//END
	gi.WriteByte (MZ_SSHOTGUN | is_silenced);
//SKULL
        if (ent->client->use)
                gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
        else
                gi.multicast (ent->s.origin, MULTICAST_PVS);
//END

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_Kuml (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_kuml_fire, 0);
}
