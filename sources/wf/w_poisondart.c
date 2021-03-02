#include "g_local.h"
void dart_prethink (edict_t *ent);
void stick(edict_t *projectile, edict_t *object); 

/*
======================
Poison Dart Launcher 
======================
*/
void poisondart_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	int index;

	if (other == self->owner)
		return;

	if (!other->client)	//only damage players
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		
		if (other->wf_team != self->wf_team)
		{
			if (other->s.origin[2]-self->s.origin[2] > self->viewheight-6)
			{
				T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg*5, 2, DAMAGE_ENERGY, MOD_TRANQUILIZERDART);
				other->Slower=level.time+20;
			}
			else
			{
				T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 2, DAMAGE_ENERGY, MOD_TRANQUILIZERDART);
				other->Slower=level.time+10;
			}
		}
	}
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

	self->think = G_FreeEdict;
	self->nextthink = level.time + 3 + 8 * crandom();

	stick(self, other);
}

void fire_poisondart (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	bolt->wf_team = self->wf_team;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->velocity[2] += 100;
	bolt->movetype = MOVETYPE_TOSS; //JR 1/4/98 changed so it bounces
	bolt->clipmask = MASK_SHOT;
	bolt->prethink = dart_prethink; // Keeps the arrow aligned, so it arcs through the air nicely.
	bolt->gravity = 0.4;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	VectorSet (bolt->mins,-1,-1,-3.5);
	VectorSet (bolt->maxs,1,1,2.5);
	bolt->s.modelindex = gi.modelindex ("models/dart/tris.md2");
	bolt->s.skinnum = 1;
	bolt->owner = self;
	bolt->touch = poisondart_touch;
//	bolt->touch = infecteddart_touch;
	bolt->nextthink = level.time + 1.7;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "poison dart"; 
	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	

void weapon_poisondartlauncher_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;

	if (deathmatch->value)
	{	damage = 5;
	}
	else
	{
		damage = 10;
	}

	if (is_quad)
	{
		damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	//start[0]=ent->s.origin[0] + forward[0]*1+right[0]*6;
	//start[1]=ent->s.origin[1] + forward[1]*1+right[1]*6;
	//start[2]=ent->s.origin[2] + forward[2]*1+right[2]*6+ent->viewheight-8;

	/* new stuff to fix left handedness */
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	/* new stuff to fix left handedness */

	fire_poisondart (ent, start, forward, damage, 850, EF_GREENGIB);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
}


void Weapon_PoisonDartLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_poisondartlauncher_fire);
}