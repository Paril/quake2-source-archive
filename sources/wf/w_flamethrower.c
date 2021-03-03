#include "g_local.h"
/*
===============
Flamethrower
===============
*/
void flame_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{

	if (other == self->owner)
		return;

	if (!other->takedamage) return;

	// clean up laser entities

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);
	T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 6, 0, 0,MOD_WF_FLAME);
	// core explosion - prevents firing it into the wall/floor
	if (other->health)
	{
		burn_person(other, self->owner, self->SniperDamage, MOD_FLAMETHROWER);
	}
	G_FreeEdict (self);
}

void Flame_IncreaseFrame(edict_t *self)
{
	self->s.frame++;
	if(self->s.frame>4)
		self->s.frame =0;
	if(self->delay<level.time)
	{
		G_FreeEdict(self);
		return;
	}
	self->nextthink=level.time+0.1;
}


void fire_flamethrower(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
	edict_t	*flame;

	flame = G_Spawn();
	flame->wf_team = self->wf_team;
	VectorCopy (start, flame->s.origin);
	VectorCopy (dir, flame->movedir);
	vectoangles (dir, flame->s.angles);
	VectorScale (dir, speed, flame->velocity);
	flame->movetype = MOVETYPE_BOUNCE;
	flame->clipmask = MASK_SHOT;
	flame->solid = SOLID_BBOX;
	flame->s.effects |= EF_PLASMA|EF_DOUBLE;//EF_ANIM_ALLFAST|EF_BFG|EF_HYPERBLASTER;//EF_BLASTER|EF_GRENADE;
	VectorSet (flame->mins,-20,-20,-20);
	VectorSet (flame->maxs,20,20,20);
	flame->s.modelindex = gi.modelindex ("models/fire/tris.md2");//("sprites/fire.sp2");
	flame->gravity=0.2;
	flame->s.frame=0;
	flame->owner = self;
	flame->touch = flame_touch;
	flame->delay = level.time + 0.8;
	flame->nextthink=level.time+0.1;
	flame->think = Flame_IncreaseFrame;
	//flame->think = G_FreeEdict;
	flame->radius_dmg = damage;
	flame->SniperDamage = damage;
	flame->dmg_radius = damage_radius;
	flame->classname = "flame";
	flame->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");

	if (self->client)
		check_dodge (self, flame->s.origin, dir, speed);

	gi.linkentity (flame);
}

void weapon_flamethrower_fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage = wf_game.weapon_damage[WEAPON_FLAMETHROWER] ;
	float	damage_radius = 1200;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 31;
		return;
	}
	else if(ent->client->ps.gunframe == 30)
		ent->client->ps.gunframe = 9;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (is_quad)
		damage *= 4;
	ent->client->v_angle[2]+=5;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	ent->client->v_angle[2]-=5;
	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (ent->waterlevel < 2)
	{
                fire_flamethrower (ent, start, forward, wf_game.weapon_damage[WEAPON_FLAMETHROWER], 450 , damage_radius);
	}

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
}

void Weapon_FlameThrower (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {12, 15, 18, 24, 27, 30, 0};
	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_flamethrower_fire);
}
