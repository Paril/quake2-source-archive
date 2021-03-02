#include "g_local.h"
/*
===============
Telsa Coil
===============
*/

void fire_telsa(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end,offset;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;
	edict_t *ent;

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
			int		color;

			water = true;
			VectorCopy (tr.endpos, water_start);

			if (!VectorCompare (start, tr.endpos))
			{
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
				}
				
				// change bullet's course when it enters water
				ent = NULL;
				VectorSet(offset,0,0,0);
				while ((ent = findradius(ent, self->s.origin, 1024)) != NULL)
				{
					if (!ent->takedamage)
						continue;

					if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
						continue;

					if (!ent->waterlevel)
						continue;
					
					if(isvisible (self, ent))
						continue;
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_PARASITE_ATTACK);
					gi.WriteShort (self - g_edicts);
					gi.WritePosition (tr.endpos);
					gi.WritePosition (ent->s.origin);
					gi.multicast (tr.endpos, MULTICAST_PVS);
					T_Damage (ent, self, self, aimdir, ent->s.origin, offset, damage*2, kick, DAMAGE_BULLET,MOD_TELSA);
				}
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

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET,MOD_TELSA);
			}
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_LASER_SPARKS);
					gi.WriteByte (5);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (0xd8dad9db);
					gi.multicast (tr.endpos, MULTICAST_PVS);

					if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
				}
			}
		}
	}
	VectorSet(offset,0,24,0);
	VectorAdd(offset,start,start);
	gi.WriteByte (svc_temp_entity);
    gi.WriteByte (TE_PARASITE_ATTACK);
    gi.WriteShort (self - g_edicts);
    gi.WritePosition (start);
    gi.WritePosition (tr.endpos);
    gi.multicast (start, MULTICAST_PVS);
	
	gi.sound(self, CHAN_VOICE, gi.soundindex("electric.wav"), 1, ATTN_NORM, 0);
	
}


void Weapon_Telsa_Fire (edict_t *ent)
{
	vec3_t		forward, right,start;
	vec3_t		offset;
	int			damage,kick;
	int			xspread;
	int			yspread;
	if (ent->client->pers.inventory[ent->client->ammo_index]<=0)
	{
		ent->client->ps.gunframe++;
		return;
	}	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
		return;
	}
	if (!(ent->client->pers.inventory[ent->client->ammo_index]))
	{
		ent->client->ps.gunframe++;
		return;
	}
	damage = wf_game.weapon_damage[WEAPON_TELSA];
	xspread = 25;
	yspread = 25;
	kick = 2 + (((int)(random()*1000)) % 8);

	if (is_quad)
	{
		damage *= 4;
		xspread *=2.5;
		yspread *=2.5;
		kick *=4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;
	VectorSet(offset, 0, 20, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	VectorAdd(start,offset,start);
//	fire_needle(ent, start, forward, damage, kick, TE_BLASTER, xspread, yspread);
	fire_telsa(ent, start, forward, damage, kick, TE_BLASTER, xspread, yspread);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;	
}
void Weapon_Telsa (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= { 8, 0};

	Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_Telsa_Fire);
}