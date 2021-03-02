/*
------------------------
IWM Weapons
===
Devastator Routines
------------------------
*/
#include "../g_local.h"
#include "../m_player.h"

void dev_explode (edict_t *self)
{
	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 7)
		self->think = G_FreeEdict;
}

void SpawnDevExplod (edict_t *self, vec3_t start)
{
	edict_t	*bfg;

	bfg = G_Spawn();
	VectorCopy (start, bfg->s.origin);
	bfg->movetype = MOVETYPE_NONE;
	bfg->solid = SOLID_NOT;
	bfg->s.modelindex = ModelIndex ("sprites/s_explo3.sp2");
	bfg->owner = self;
	bfg->nextthink = level.time + 8000;
	bfg->think = G_FreeEdict;
	bfg->classname = "devexpl";
	bfg->s.effects |= EF_HYPERBLASTER;

	bfg->think = dev_explode;
	bfg->nextthink = level.time + FRAMETIME;

	gi.linkentity (bfg);
}

void dev_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
//	int			n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_DEVASTATOR);
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	/*gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);*/

	SpawnDevExplod(ent->owner, origin);
	gi.sound (ent, CHAN_AUTO, SoundIndex ("weapons/devastator/boom.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict (ent);
}

void fire_dev (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
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
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = ModelIndex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = dev_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->classname = "rocket";
	rocket->can_teleport_through_destinations = 1;
	rocket->s.renderfx |= RF_IR_VISIBLE;

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}


void Weapon_Devastator_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right, v;
	int		damage;
	float	damage_radius;
	int		radius_damage;
	int left;
	int f = ent->client->ps.gunframe;
	int count = 1;

	if (f == 3)
		left = 0;
	else if (f == 6)
		left = 1;
	else
	{
		gi.sound (ent, CHAN_WEAPON, SoundIndex ("weapons/devastator/cooldown.wav"), 1, ATTN_NORM, 0);
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 15;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	damage = 12 + (int)(random() * 20.0);
	radius_damage = 30;
	damage_radius = 25;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);

	if (ent->client->pers.hand == LEFT_HANDED)
	{
		if (left == 1)
			left = 0;
		else
			left = 1;
	}

	if (!left)
		P_ProjectSource_Dev (ent->client, ent->s.origin, offset, forward, right, start, 0);
	else
		P_ProjectSource_Dev (ent->client, ent->s.origin, offset, forward, right, start, 1);
again:
	v[PITCH] = ent->client->v_angle[PITCH] - random()*3 + random()*3;
	v[YAW]   = ent->client->v_angle[YAW] - random()*3 + random()*3;
	v[ROLL]  = ent->client->v_angle[ROLL] - random()*3 + random()*3;
	AngleVectors (v, forward, NULL, NULL);

	fire_dev (ent, start, forward, damage, 900, damage_radius, radius_damage);
	count++;

	if (count == 2)
		goto again;

	if (f == 3)
		gi.sound (ent, CHAN_AUTO, SoundIndex ("weapons/devleft.wav"), 1, ATTN_NORM, 0);
	else
		gi.sound (ent, CHAN_AUTO, SoundIndex ("weapons/devright.wav"), 1, ATTN_NORM, 0);

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_ROCKET | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if ((f == 3 || f == 6) && (! ( (int)dmflags->value & DF_INFINITE_AMMO ) ))
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}

	if ((!(ent->client->buttons|ent->client->latched_buttons) & BUTTON_ATTACK))
		ent->client->ps.gunframe = 10;

}

void Weapon_Devastator (edict_t *ent)
{
	static int	pause_frames[]	= {15, 0};
	static int	fire_frames[]	= {3, 6, 10, 11};

	if (ent->client->buttons & BUTTON_ATTACK && ent->client->ps.gunframe == 7)
		ent->client->ps.gunframe = 2;

	Weapon_Generic (ent, 1, 14, 17, 19, pause_frames, fire_frames, Weapon_Devastator_Fire);
	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 6;
	if ((!(ent->client->buttons|ent->client->latched_buttons) & BUTTON_ATTACK) && (ent->client->ps.gunframe == 3 || ent->client->ps.gunframe == 4 || ent->client->ps.gunframe == 5 || ent->client->ps.gunframe == 6 || ent->client->ps.gunframe == 10))
		ent->client->ps.gunframe = 10;
}
