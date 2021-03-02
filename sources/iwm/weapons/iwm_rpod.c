/*
------------------------
IWM Weapons
===
Rocket Pods Routines
------------------------
*/
#include "../g_local.h"
#include "../m_player.h"

void rpod_explode (edict_t *self)
{
	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 5)
		self->think = G_FreeEdict;
}

void SpawnRpodExplod (edict_t *self, vec3_t start)
{
	edict_t	*bfg;

	bfg = G_Spawn();
	VectorCopy (start, bfg->s.origin);
	bfg->movetype = MOVETYPE_NONE;
	bfg->solid = SOLID_NOT;
	bfg->s.modelindex = ModelIndex ("sprites/s_explod.sp2");
	bfg->owner = self;
	bfg->nextthink = level.time + 8000;
	bfg->think = G_FreeEdict;
	bfg->classname = "devexpl";
	bfg->s.effects |= EF_HYPERBLASTER;

	bfg->think = rpod_explode;
	bfg->nextthink = level.time + FRAMETIME;

	gi.linkentity (bfg);
}

void rpod_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
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
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKETPOD);
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	/*gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);*/

	SpawnRpodExplod(ent->owner, origin);
	gi.sound (ent, CHAN_AUTO, SoundIndex ("weapons/rpod/rpodhit.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict (ent);
}

void fire_rpod (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
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
	rocket->s.effects |= EF_HYPERBLASTER;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = ModelIndex ("models/proj/rpod/tris.md2");
	rocket->owner = self;
	rocket->touch = rpod_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->classname = "rpod";
	rocket->can_teleport_through_destinations = 1;
	rocket->s.renderfx |= RF_IR_VISIBLE;
//	rocket->s.frame = rocket->s.skinnum = 9;

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void Weapon_RocketPods_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;
	int l;
	vec3_t v;

	if ( !((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK))
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 7;
		return;
	}

	damage = (25 + ((int)random() * 8)) / 3;
	radius_damage = 60;
	damage_radius = 60;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 2);

	//start[2] += (rndnum (0, 12) - rndnum (0, 12));
	//start[1] += (rndnum (0, 18) - rndnum (0, 18));
	v[0] = ent->client->v_angle[PITCH];
	v[1]   = ent->client->v_angle[YAW] += (rndnum (0, 12) - rndnum (0, 12));
	v[2]  = ent->client->v_angle[ROLL] += (rndnum (0, 12) - rndnum (0, 12));
	AngleVectors (v, forward, NULL, NULL);
	fire_rpod (ent, start, forward, damage, 900, damage_radius, radius_damage);

	//start[2] += (rndnum (0, 25) - rndnum (0, 25));
	//start[1] += (rndnum (0, 25) - rndnum (0, 25));
	v[0] = ent->client->v_angle[PITCH];
	v[1]   = ent->client->v_angle[YAW] += (rndnum (0, 12) - rndnum (0, 12));
	v[2]  = ent->client->v_angle[ROLL] += (rndnum (0, 12) - rndnum (0, 12));
	AngleVectors (v, forward, NULL, NULL);
	fire_rpod (ent, start, forward, damage, 900, damage_radius, radius_damage);

	if (random() < 0.5)
	{
		gi.sound (ent, CHAN_WEAPON, SoundIndex ("weapons/rpod/rpodshot.wav"), 1, ATTN_NORM, 0);
		gi.sound (ent, CHAN_WEAPON, SoundIndex ("weapons/rpod/rpodsho2.wav"), 1, ATTN_NORM, 0.05);
	}
	else
	{
		gi.sound (ent, CHAN_WEAPON, SoundIndex ("weapons/rpod/rpodsho2.wav"), 1, ATTN_NORM, 0);
		gi.sound (ent, CHAN_WEAPON, SoundIndex ("weapons/rpod/rpodsho2.wav"), 1, ATTN_NORM, 0.05);
	}

	// send muzzle flash
	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (ent-g_edicts);
	//gi.WriteByte (MZ_ROCKET | is_silenced);
	//gi.multicast (ent->s.origin, MULTICAST_PVS);

	//ent->client->ps.gunframe++;

	l = rndnum (2, 11);

	if (l == 7)
		l = 2;
	if (l == 8)
		l = 3;
	if (l == 9)
		l = 4;
	if (l == 10)
		l = 5;
	if (l == 11)
		l = 6;
	
	ent->client->ps.gunframe = l; // Random frame number

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
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
}

void Weapon_RocketPods (edict_t *ent)
{
	static int	pause_frames[]	= {7, 0};
	static int	fire_frames[]	= {2, 3, 4, 5, 6};

	Weapon_Generic (ent, 1, 6, 8, 10, pause_frames, fire_frames, Weapon_RocketPods_Fire);
}
