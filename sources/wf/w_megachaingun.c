#include "g_local.h"
/*
==========================
Mega Chaingun
==========================
*/
/*

Gregg put this code in client think At the very end
if(ent->cantmove)
	VectorCopy(ent->LockedPosition,ent->s.origin);

  Alright also make this use shells
*/
void fire_ChainBlast(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		ran;
	float		u;
	float		adjustedkick;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;
	vec3_t distance;
	int			dist;

	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);

		r = crandom()*hspread;
		u = crandom()*vspread;
		VectorMA (start, 8192, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		if (gi.pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace (start, NULL, NULL, end, self, content_mask);
	}

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				//Push or kick is dependant on distance
				distance[0] = tr.ent->s.origin[0] - self->s.origin[0];
				distance[1] = tr.ent->s.origin[1] - self->s.origin[1];
				distance[2] = tr.ent->s.origin[2] - self->s.origin[2];
				dist=VectorLength(distance);

				if (dist < 200)
					adjustedkick = kick;
				else if (dist < 350)
					adjustedkick = 10; 
				else
					adjustedkick = 5; 


				if (tr.ent->wf_team == self->wf_team)
					T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, damage, DAMAGE_BULLET, MOD_MEGACHAINGUN);
				else
					T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, (int)adjustedkick, DAMAGE_BULLET, MOD_MEGACHAINGUN);
			}
//			else
//			{
//				if (strncmp (tr.surface->name, "sky", 3) != 0)
//				{
//					if (self->client)
//						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
//				}
//			}
		}
	}

	ran = random();
	if(ran<0.1)//Well this is for shots that somehow misfire
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (start, MULTICAST_PVS);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GUNSHOT);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (tr.plane.normal);
		gi.multicast (tr.endpos, MULTICAST_PVS);

	}

}
void MegaChaingun_Fire (edict_t *ent)
{
	int			i;
	int			shots;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset;
	int			damage;

	int			kick = 20;

	if (!ent->groundentity && !ent->cantmove)
	{
		ent->client->ps.gunframe = 32;

		safe_cprintf (ent, PRINT_HIGH, "Must be on the ground to fire. \n");
		ent->cantmove = 0;
		ent->client->weapon_sound = 0;
		return;
	}

	damage = wf_game.weapon_damage[WEAPON_MEGACHAINGUN];

	if (ent->client->ps.gunframe == 5)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);
		VectorCopy(ent->s.origin,ent->LockedPosition);
		ent->cantmove=1;
	}

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->cantmove = 0;
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
		ent->cantmove = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
	}

	if (ent->client->ps.gunframe <= 9)
		shots = 1;
	else if (ent->client->ps.gunframe <= 14)
	{
		if (ent->client->buttons & BUTTON_ATTACK)
			shots = 1;
		else
			shots = 1;
	}
	else
		shots = 2;

	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
		shots = ent->client->pers.inventory[ent->client->ammo_index];

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

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	for (i=0 ; i<shots ; i++)
	{
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		//WF JR LAG fixing in case lag gets high
		//Hopefully this will help during a fierce battle and the
		//ping times rocket up
		if (ent->client->ping >550)
		{
			fire_ChainBlast(ent, start, forward, damage*2, kick, 450, 450);
			fire_ChainBlast(ent, start, forward, damage*2, kick, 450, 450);
		}
		else
		{
			fire_ChainBlast(ent, start, forward, damage, kick, 500, 500);
			fire_ChainBlast(ent, start, forward, damage, kick, 500, 500);
			fire_ChainBlast(ent, start, forward, damage, kick, 500, 500);
			fire_ChainBlast(ent, start, forward, damage, kick, 500, 500);
		}
		//end lag
	}
//WF
	//Send a tracer bullet
	//if ((shots == 3) && (random() < .16))
	//{
	//	fire_blaster (ent, start, forward, 0, 1000, EF_HYPERBLASTER, true);
	//}
//WF
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		// TeT we do fire four bullets per shot
		//ent->client->pers.inventory[ent->client->ammo_index] -= (shots * 3);
		ent->client->pers.inventory[ent->client->ammo_index] -= (int)(shots * 1.5);
	}
}


void Weapon_MegaChaingun (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, MegaChaingun_Fire);
}