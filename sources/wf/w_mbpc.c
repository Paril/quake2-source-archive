#include "g_local.h"
/*
===========================
Multi Barreled Pulse Cannon
===========================
*/
void fire_pulse (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;

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

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			//int		color;

			water = true;
			VectorCopy (tr.endpos, water_start);

			if (!VectorCompare (start, tr.endpos))
			{
				/*Doesn't need to splash
				if (tr.contents & CONTENTS_WATER)
				{
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				}
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}*/

				// change bullet's course when it enters water
				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, forward, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}

			// re-trace ignoring water this time
			tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}
	gi.WriteByte (svc_temp_entity);
    gi.WriteByte (TE_BFG_LASER);      
    gi.WritePosition (start);
    gi.WritePosition (tr.endpos);
    gi.multicast (start, MULTICAST_PVS);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_RAILTRAIL);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PVS);
	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					/*gi.WriteByte (svc_temp_entity);
					gi.WriteByte (te_impact);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.multicast (tr.endpos, MULTICAST_PVS);
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (rndnum(2,4));
					gi.WritePosition (self->s.origin);
					
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (SPLASH_SLIME);
					gi.multicast (self->s.origin, MULTICAST_PVS);
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (rndnum(5,10));
					gi.WritePosition (tr.endpos);
					
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (SPLASH_SLIME);
					gi.multicast (tr.endpos, MULTICAST_PVS);*/
					if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
				}
			}
		}
	}

	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t	pos;

		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);
		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		/*gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);*/
	}
}

void plasma_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 2, DAMAGE_ENERGY,MOD_MBPC);
	T_RadiusDamage(self, self->owner, 10, NULL, 200,MOD_MBPC);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPLASH);
	gi.WriteByte (rndnum(5,28));
	gi.WritePosition (self->s.origin);
	if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
	gi.WriteByte (SPLASH_SLIME);
	gi.multicast (self->s.origin, MULTICAST_PVS);
	G_FreeEdict (self);

}
static void PlasmaThink(edict_t *ent)
{
vec3_t newpos, pos;

VectorSubtract(ent->s.origin, ent->pos2, pos); /*translate current position*/
RotatePointAroundVector(newpos, ent->pos1, pos, 20); /*So we can rotate around our original line
of sight*/
VectorAdd(ent->pos2, newpos, ent->s.origin); /*then translate the current position back*/

RotatePointAroundVector(ent->velocity, ent->pos1, ent->velocity, 20); /*The velocity vector must be
rotated also*/
ent->nextthink = level.time + .1; //Need to think again in .1 seconds
}

void fire_plasmaball(edict_t *self, vec3_t start, vec3_t dir, vec3_t los, int damage, int speed, 
float damage_radius, int radius_damage)
{
edict_t *rocket;

rocket = G_Spawn();
rocket->wf_team = self->wf_team;

VectorCopy (los, rocket->pos1); /*Added: Save the line of sight of the center rocket. This is
the axis around which the other two rockets rotate*/
VectorCopy (start, rocket->pos2); /*Added: Save the start position of the rocket (Not sure if
this is necessary. This info might already be somewhere else)?*/

VectorCopy (start, rocket->s.origin);
VectorCopy (dir, rocket->movedir);
vectoangles (dir, rocket->s.angles);
VectorScale (dir, speed, rocket->velocity);
rocket->movetype = MOVETYPE_FLYMISSILE;
rocket->clipmask = MASK_SHOT;
rocket->solid = SOLID_BBOX;
rocket->s.effects |= EF_BLASTER;
VectorClear (rocket->mins);
VectorClear (rocket->maxs);
rocket->s.modelindex = gi.modelindex ("models/ball/tris.md2");
rocket->owner = self;
rocket->touch = plasma_touch;
rocket->nextthink = level.time + .1;
rocket->think = PlasmaThink;
rocket->dmg =4;
rocket->radius_dmg = 128;
rocket->dmg_radius =128;
rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");

if (self->client)
check_dodge (self, rocket->s.origin, dir, speed);

gi.linkentity (rocket);
}
void ChainMBPC_Fire (edict_t *ent)
{
	int			i;
	int			shots;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset;
	int			damage;
	int			kick = 8;

	damage = wf_game.weapon_damage[WEAPON_PULSE];

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
		shots = 3;

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
		start[2] += ent->viewheight-8;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		//fire_rail (ent, start, forward, damage, kick);
		//fire_plasmaball(ent, start, forward, forward, damage, 600, 100, 100);
		ent->ShotNumber++;
		if (ent->client->ping >500)
		{
			if (ent->ShotNumber>3)
			{
				fire_pulse (ent, start, forward, damage*4, kick*4, 0, 250, 250, MOD_MBPC);
				ent->ShotNumber =0;
			}
		}
		else
		{
			if (ent->ShotNumber>1)
			{
				fire_pulse (ent, start, forward, damage*2, kick*2, 0, 250, 250, MOD_MBPC);
				ent->ShotNumber =0;
			}
		}
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}


void Weapon_ChainMBPC (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, ChainMBPC_Fire);
}