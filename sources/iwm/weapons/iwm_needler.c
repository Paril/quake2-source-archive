#include "../g_local.h"
#include "../m_player.h"

void Needle_Kaboom (edict_t *ent)
{
	vec3_t		origin;
//	int			n;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (ent->stuckentity->takedamage)
	{
		T_Damage (ent->stuckentity, ent, ent->owner, ent->velocity, ent->s.origin, ent->tempplane->normal, ent->dmg, 0, 0, MOD_NEEDLER);
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_FLECHETTE);
	gi.WritePosition (origin);
	if (!ent->tempplane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (ent->tempplane->normal);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

//	gi.sound (ent, CHAN_AUTO, SoundIndex ("weapons/laserhit.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict (ent);
}

// Main sticky code by QwazyWabbit(WOS).
// Modified by Paril to fit the Needlah.
void Stick_Needle(edict_t *ent, edict_t *other)
{
	if (ent->movetype == MOVETYPE_STUCK)
		return;
	
	// Paril: Fixes the impeding movement and jump bug.
	ent->solid = SOLID_NOT;
	ent->movetype = MOVETYPE_STUCK;
	VectorClear(ent->velocity);
	VectorClear(ent->avelocity);
	ent->stuckentity = other;
	VectorCopy(other->s.origin,ent->oldstuckorigin);
	VectorCopy(other->s.angles,ent->oldstuckangles);

	// Needler Specific
	ent->nextthink = level.time+1.8;
	ent->think = Needle_Kaboom;
}

void Needle_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	vec3_t		origin;
//	int			n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	ent->tempplane = plane;
	ent->tempsurf = surf;

	Stick_Needle(ent, other);
}

void fire_needler (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
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
	rocket->s.modelindex = ModelIndex ("models/proj/shard/tris.md2");
	rocket->owner = self;
	rocket->touch = Needle_Touch;
	rocket->nextthink = level.time + 8000;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	//rocket->s.effects |= EF_QUAD|EF_PENT;
//	rocket->radius_dmg = radius_damage;
//	rocket->dmg_radius = damage_radius;
	rocket->classname = "needler";
	rocket->can_teleport_through_destinations = 1;
	rocket->s.renderfx |= RF_IR_VISIBLE;
//	rocket->s.frame = rocket->s.skinnum = 3;

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void Needler_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 8;
	int			kick = 2;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 4)
		ent->client->ps.gunframe = 3;
	else
		ent->client->ps.gunframe = 4;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, SoundIndex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
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

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start, 0);
	//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
	fire_needler (ent, start, forward, damage, 1000);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

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

void Weapon_Needler (edict_t *ent)
{
	static int	pause_frames[]	= {7, 10, 0};
	static int	fire_frames[]	= {3, 4, 0};

	Weapon_Generic (ent, 2, 4, 7, 10, pause_frames, fire_frames, Needler_Fire);
}
