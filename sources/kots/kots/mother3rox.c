#include "g_local.h"


extern qboolean	is_quad;
extern byte		is_silenced;

void rocket_touch( edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf );

//*************************************************************************************
//*************************************************************************************
// Function: fire_mrox
//*************************************************************************************
//*************************************************************************************

void fire_mrox( edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, 
                float damage_radius, int radius_damage )
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
//	rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	//rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	gi.linkentity (rocket);
}
																					
//*************************************************************************************
//*************************************************************************************
// Function: KOTS_ProjectSource
//*************************************************************************************
//*************************************************************************************

void KOTS_ProjectSource( gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result )
{
	vec3_t	_distance;

	VectorCopy ( distance, _distance );

	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
//	else if (client->pers.hand == CENTER_HANDED)
//		_distance[1] = 0;

	G_ProjectSource( point, _distance, forward, right, result );
}

//*************************************************************************************
//*************************************************************************************
// Function: Use_KOTS_MRock
//*************************************************************************************
//*************************************************************************************

void KOTS_Use_MRock( edict_t *ent, gitem_t *item )
{
	int index;
	int	timeout;

	index = ITEM_INDEX(item);

	ent->client->pers.inventory[ index ]--;
	ValidateSelectedItem (ent);

	timeout = 600;

	ent->client->pers.selected_item = index;

	if (ent->client->kots_mrock_framenum > level.framenum)
		ent->client->kots_mrock_framenum += timeout;
	else
		ent->client->kots_mrock_framenum = level.framenum + timeout;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
}

//*************************************************************************************
//*************************************************************************************
// Function: 
//*************************************************************************************
//*************************************************************************************

void KOTS_Weapon_RocketLauncher_Fire (edict_t *ent)
{
	int     num_rockets = 1;
	vec3_t	offset, start;
	vec3_t	forward, right;
	vec3_t	middle_rocket,right_rocket,left_rocket;

	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}
	//Mother added noise when shooting
	else if (ent->client->pers.inventory[ ITEM_INDEX( FindItem( "Damage Amp" ) ) ]>0)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("items/damage3.wav"), 1, ATTN_STATIC, 0);
	}

	if ( ent->client->kots_mrock_framenum > level.framenum )
		num_rockets = 3;

	//this is where rockets are created:
	if(num_rockets == 3)//if they want to shoot 3
	{
		//if they have at least 3 rockets to shoot:
		if(ent->client->pers.inventory[ent->client->ammo_index] > 2)
		{
			AngleVectors (ent->client->v_angle, forward, right, NULL);
			VectorScale (forward, -2, ent->client->kick_origin);
			ent->client->kick_angles[0] = -1;
		
			//middle
			VectorSet(offset, 8, 0, ent->viewheight-8);
			KOTS_ProjectSource (ent->client, ent->s.origin, offset, forward, right, middle_rocket);

			//right
			VectorSet(offset, 8, 16, ent->viewheight-8);
			KOTS_ProjectSource (ent->client, ent->s.origin, offset, forward, right, right_rocket);

			//left
			VectorSet(offset, 8, -16, ent->viewheight-8);
			KOTS_ProjectSource (ent->client, ent->s.origin, offset, forward, right, left_rocket);

			//fire them off!
			fire_rocket (ent, middle_rocket, forward, damage, 650, damage_radius, radius_damage);
			fire_mrox (ent, right_rocket, forward, damage, 650, damage_radius, radius_damage);
			fire_mrox (ent, left_rocket, forward, damage, 650, damage_radius, radius_damage);
	
		}//end if they have enough ammo

		else //else set them to using 2 rockets since they didnt have 3 to shoot
			num_rockets = 2;
		
	}//end if shooting 3 rox

	if(num_rockets == 2)//they want to shoot 2 rockets
	{	
		//check for enough ammo
		if(ent->client->pers.inventory[ent->client->ammo_index] > 1)
		{
			AngleVectors (ent->client->v_angle, forward, right, NULL);
			VectorScale (forward, -2, ent->client->kick_origin);
			ent->client->kick_angles[0] = -1;

			//right
			VectorSet(offset, 8, 8, ent->viewheight-8);
			KOTS_ProjectSource (ent->client, ent->s.origin, offset, forward, right, right_rocket);

			//left
			VectorSet(offset, 8, -8, ent->viewheight-8);
			KOTS_ProjectSource (ent->client, ent->s.origin, offset, forward, right, left_rocket);

			//fire them off!
			fire_rocket (ent, right_rocket, forward, damage, 650, damage_radius, radius_damage);
			fire_mrox (ent, left_rocket, forward, damage, 650, damage_radius, radius_damage);
	
		}//end if they have enough ammo
		else //else make them just shooting 1 rocket
			num_rockets = 1;

	}//end shooting 2 rox

	if(num_rockets == 1)//they are just shooting 1 rocket
	{
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorScale (forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;
		VectorSet(offset, 8, 8, ent->viewheight-8);
		KOTS_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);
	}//end 1 rocket
	

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{//ammo take happens here:
		if(num_rockets == 1)
			ent->client->pers.inventory[ent->client->ammo_index]--;
		else if(num_rockets == 2)
			ent->client->pers.inventory[ent->client->ammo_index] -= 2;
		else if(num_rockets == 3 )
			ent->client->pers.inventory[ent->client->ammo_index] -= 3;
	}//end ammo take
}//end function
