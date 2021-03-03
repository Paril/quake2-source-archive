#include "g_local.h"

#define GIEX_MABILITITY_DAMAGE_BONUS 1.5
#define GIEX_MABILITY_SPEED_BONUS 0.8

void monster_skip2frames(edict_t *self) {
	self->s.frame += 2;
}
void monster_skip4frames(edict_t *self) {
	self->s.frame += 4;
}
void monster_skip8frames(edict_t *self) {
	self->s.frame += 8;
}

float getMonsterBonus(edict_t *self) {
	float mult = 1.0;
	if (self->monsterinfo.skill > 40) { // Very high level Hunt monsters
		mult = 3.5;
	} else if (self->monsterinfo.skill > 30) { // Highlevel Hunt monsters
		mult = 2.8;
	} else if (self->monsterinfo.skill > 23) { // Yellow monsters
		mult = 2.3;
	} else if (self->monsterinfo.skill > 15) { // Red monsters
		mult = 1.8;
	} else if (self->monsterinfo.skill > 7) { // Green monsters
		mult = 1.3;
	}
	if (self->monsterinfo.superchamp == 1) {
		mult *= 2;
	} else if (self->monsterinfo.superchamp == 2) {
		mult *= 3;
	}
	return mult;
}

void thinkBlackHole2(edict_t *ent)
{
	edict_t *scan = NULL;
	vec3_t v;
	float points, dist;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TRACKER_EXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->dmg_radius += ent->count;

	if (!visible(ent, ent->owner) || (ent->owner->health <= 0)) {
		ent->think = G_FreeEdict;
		ent->nextthink = level.time + FRAMETIME;
		return;
	}

	while ((scan = findradius(scan, ent->s.origin, ent->dmg_radius)) != NULL)
	{
		if (scan == ent)
			continue;
		if (scan == ent->owner)
			continue;
		if (!scan->takedamage)
			continue;

		VectorAdd (scan->mins, scan->maxs, v);
		VectorMA (scan->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		dist = VectorLength(v);
		points = ent->radius_dmg * ( (float)(ent->dmg_radius - dist) / (float)ent->dmg_radius);
		if (dist < 64) {
			scan->velocity[0] = 0;
			scan->velocity[1] = 0;
			scan->velocity[2] = 0;
			points += 10 * ent->dmg;
		}
		else if (dist < 256)
			points += 10 * ent->dmg * ( (float)(256 - dist) / (float)256);

		if (points > 0)
		{
			if (CanDamage (scan, ent))
			{
//				edict_t *tmp = scan->groundentity;
				scan->groundentity = NULL;
				VectorSubtract (scan->s.origin, ent->s.origin, v);
				if (dist < 64) {
					T_Damage (scan, ent, ent->owner, v, ent->s.origin, vec3_origin, (int)ceil(0.1 * points), (int)-ceil(2 * points), DAMAGE_RADIUS, MOD_BLACKHOLE);
				} else {
					T_Damage (scan, ent, ent->owner, v, ent->s.origin, vec3_origin, (int)ceil(0.1 * points), (int)-ceil(12 * points), DAMAGE_RADIUS, MOD_BLACKHOLE);
				}
//				scan->groundentity = tmp;
			}
		}
	}

	ent->count--;
	if (ent->count < 1)
		ent->think = G_FreeEdict;
	ent->nextthink = level.time + FRAMETIME;
}

void touchBlackHole(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;

	if (other == ent->owner)
		return;

/*	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}*/

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TRACKER_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->velocity[0] = 0;
	ent->velocity[1] = 0;
	ent->velocity[2] = 0;
	ent->movedir[0] = 0;
	ent->movedir[1] = 0;
	ent->movedir[2] = 0;

	ent->touch = NULL;
	ent->think = thinkBlackHole2;
	ent->nextthink = level.time + FRAMETIME;
}

void thinkBlackHole(edict_t *ent)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPLASH);
	gi.WriteByte (25);
	gi.WritePosition (ent->s.origin);
	gi.WriteDir (ent->movedir);
	gi.WriteByte (6);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_NUKE1);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
	ent->nextthink = level.time + FRAMETIME * 5;
}

void monster_fire_blackhole(edict_t *ent) {
	edict_t *hole;
	vec3_t	start, end, aim, dir;
	vec3_t	forward;

	if (!ent->enemy)
		return;

	hole = G_Spawn();

	VectorCopy (ent->s.origin, start);
	start[2] += ent->viewheight - 8;
	VectorCopy (ent->enemy->s.origin, end);
	end[2] += ent->enemy->viewheight;
	VectorSubtract (end, start, aim);
	vectoangles (aim, dir);
	AngleVectors (dir, forward, NULL, NULL);

//	AngleVectors (ent->client->v_angle, forward, right, NULL);
//	VectorScale (forward, -2, ent->client->kick_origin);

	VectorCopy (ent->s.origin, hole->s.origin);
	hole->s.origin[2] += ent->viewheight - 8;
	VectorCopy (forward, hole->movedir);
	vectoangles (forward, hole->s.angles);
	VectorScale (forward, 480, hole->velocity);
	hole->svflags = SVF_DEADMONSTER;
	hole->movetype = MOVETYPE_FLYMISSILE;
	hole->clipmask = MASK_SHOT;
	hole->solid = SOLID_BBOX;
	hole->count = 40;
	hole->dmg = 18 * getMonsterBonus(ent);
	hole->radius_dmg = 9 * getMonsterBonus(ent);
	hole->dmg_radius = 190;
	hole->s.sound = gi.soundindex ("weapons/rockfly.wav");
	hole->owner = ent;
	hole->touch = touchBlackHole;
	hole->think = thinkBlackHole;
	hole->nextthink = level.time + FRAMETIME;
	hole->s.modelindex = gi.modelindex ("sprites/s_shine.sp2");
	hole->s.effects |= EF_TRACKER;
	hole->s.effects |= EF_TRACKERTRAIL;
	if (ent->monsterinfo.ability & GIEX_MABILITY_DAMAGE) {
		hole->dmg *= GIEX_MABILITITY_DAMAGE_BONUS;
		hole->radius_dmg *= GIEX_MABILITITY_DAMAGE_BONUS;
	}

	if (!ent->radius_dmg || !(ent->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
	}
}

void touch_greenmissile(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf) {
	vec3_t		origin;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY)) {
		G_FreeEdict (ent);
		return;
	}

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_FIREBALL | MOD_MAGIC);
	T_RadiusDamage(NULL, ent, ent->owner, ent->radius_dmg, ent->radius_dmg * 0.5, NULL, ent->dmg_radius, true, MOD_FIREBALL | MOD_MAGIC);

	gi.sound (ent, CHAN_BODY, gi.soundindex ("tank/tnkatck5.wav"), 1, ATTN_NORM, 0);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	G_FreeEdict (ent);
}

void think_greenmissile(edict_t *ent)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPLASH);
	gi.WriteByte (15);
	gi.WritePosition (ent->s.origin);
	gi.WriteDir (ent->movedir);
	gi.WriteByte (4);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->count--;
	if (ent->count < 1)
		ent->think = G_FreeEdict;
	ent->nextthink = level.time + FRAMETIME;
}

void monster_fire_greenmissile(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype) {
	edict_t *ent;
	damage *= getMonsterBonus(self);

	ent = G_Spawn();
	ent->classid = CI_GREENMISSILE;
	ent->classname = "greenmissile";

	VectorCopy (start, ent->s.origin);
	ent->s.origin[2] += self->viewheight - 8;
	VectorCopy (dir, ent->movedir);
	vectoangles (dir, ent->s.angles);
	VectorScale (dir, speed, ent->velocity);
	ent->s.effects |= 0x04000000;
	ent->svflags = SVF_DEADMONSTER;
	ent->movetype = MOVETYPE_FLYMISSILE;
	ent->clipmask = MASK_SHOT;
	ent->solid = SOLID_BBOX;
	VectorClear(ent->mins);
	VectorClear(ent->maxs);
	ent->s.sound = gi.soundindex ("weapons/rockfly.wav");

	ent->count = 30;
	ent->dmg = damage;
	ent->radius_dmg = damage;
	ent->dmg_radius = 120 + 2.5 * self->monsterinfo.skill;

	if (self->radius_dmg && self->monsterinfo.ability & GIEX_MABILITY_DAMAGE) {
		ent->dmg *= GIEX_MABILITITY_DAMAGE_BONUS;
		ent->radius_dmg *= GIEX_MABILITITY_DAMAGE_BONUS;
	}

	ent->owner = self;
	ent->touch = touch_greenmissile;
	ent->think = think_greenmissile;
	ent->nextthink = level.time + FRAMETIME;
	gi.linkentity(ent);
}

//
// monster weapons
//

void monster_fire_bullet (edict_t *self, vec3_t start, vec3_t dir, int damage, int kick, int hspread, int vspread, int flashtype) {
	if (self->radius_dmg && self->monsterinfo.ability & GIEX_MABILITY_DAMAGE) {
		damage *= GIEX_MABILITITY_DAMAGE_BONUS;
	}
	damage *= getMonsterBonus(self);
	fire_bullet (self, start, dir, damage, kick, hspread, vspread, MOD_MACHINEGUN);

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.WriteByte (svc_muzzleflash2);
		gi.WriteShort (self - g_edicts);
		gi.WriteByte (flashtype);
		gi.multicast (start, MULTICAST_PVS);
	}
}

void monster_fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int flashtype) {
	if (self->radius_dmg && self->monsterinfo.ability & GIEX_MABILITY_DAMAGE) {
		damage *= GIEX_MABILITITY_DAMAGE_BONUS;
	}
	damage *= getMonsterBonus(self);
	fire_shotgun (self, start, aimdir, damage, kick, hspread, vspread, count, MOD_SHOTGUN);

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.WriteByte (svc_muzzleflash2);
		gi.WriteShort (self - g_edicts);
		gi.WriteByte (flashtype);
		gi.multicast (start, MULTICAST_PVS);
	}
}

void monster_fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect) {
	if (self->radius_dmg && self->monsterinfo.ability & GIEX_MABILITY_DAMAGE) {
		damage *= GIEX_MABILITITY_DAMAGE_BONUS;
	}
	damage *= getMonsterBonus(self);
	fire_blaster (self, start, dir, damage, speed, effect, false);

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.WriteByte (svc_muzzleflash2);
		gi.WriteShort (self - g_edicts);
		gi.WriteByte (flashtype);
		gi.multicast (start, MULTICAST_PVS);
	}
}

void monster_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int flashtype) {
	if (self->radius_dmg && self->monsterinfo.ability & GIEX_MABILITY_DAMAGE) {
		damage *= GIEX_MABILITITY_DAMAGE_BONUS;
	}
	damage *= getMonsterBonus(self);
	fire_grenade (self, start, aimdir, damage, speed, 1.0, 2.5, 120 + 3.0 * self->monsterinfo.skill);

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.WriteByte (svc_muzzleflash2);
		gi.WriteShort (self - g_edicts);
		gi.WriteByte (flashtype);
		gi.multicast (start, MULTICAST_PVS);
	}
}
void monster_fire_incendiary(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int incendiary_damage, int speed, int flashtype) {
	if (self->radius_dmg && self->monsterinfo.ability & GIEX_MABILITY_DAMAGE) {
		damage *= GIEX_MABILITITY_DAMAGE_BONUS;
	}
	damage *= getMonsterBonus(self);
	fire_incendiary(self, start, aimdir, damage, incendiary_damage, speed, 1.0, 1.5, 120 + 3.0 * self->monsterinfo.skill);

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.WriteByte (svc_muzzleflash2);
		gi.WriteShort (self - g_edicts);
		gi.WriteByte (flashtype);
		gi.multicast (start, MULTICAST_PVS);
	}
}

void monster_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype) {
	int rad = 110 + 2.5 * self->monsterinfo.skill; // ceil(damage / 18.0);
	if (self->radius_dmg && self->monsterinfo.ability & GIEX_MABILITY_DAMAGE) {
		damage *= GIEX_MABILITITY_DAMAGE_BONUS;
	}
	damage *= getMonsterBonus(self);
	fire_rocket (self, start, dir, damage, speed, rad, damage);

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.WriteByte (svc_muzzleflash2);
		gi.WriteShort (self - g_edicts);
		gi.WriteByte (flashtype);
		gi.multicast (start, MULTICAST_PVS);
	}
}

void monster_fire_railgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int flashtype) {
	if (self->radius_dmg && self->monsterinfo.ability & GIEX_MABILITY_DAMAGE) {
		damage *= GIEX_MABILITITY_DAMAGE_BONUS;
	}
	damage *= getMonsterBonus(self);
	fire_rail (self, start, aimdir, damage, kick);

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.WriteByte (svc_muzzleflash2);
		gi.WriteShort (self - g_edicts);
		gi.WriteByte (flashtype);
		gi.multicast (start, MULTICAST_PVS);
	}
}

void monster_fire_bfg (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int kick, float damage_radius, int flashtype) {
	if (self->radius_dmg && self->monsterinfo.ability & GIEX_MABILITY_DAMAGE) {
		damage *= GIEX_MABILITITY_DAMAGE_BONUS;
	}
	damage *= getMonsterBonus(self);
	fire_bfg (self, start, aimdir, damage, speed, damage_radius);

	if (!self->radius_dmg || !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
		gi.WriteByte (svc_muzzleflash2);
		gi.WriteShort (self - g_edicts);
		gi.WriteByte (flashtype);
		gi.multicast (start, MULTICAST_PVS);
	}
}



//
// Monster utility functions
//

static void M_FliesOff (edict_t *self)
{
	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;
}

static void M_FliesOn (edict_t *self)
{
	if (self->waterlevel)
		return;
	self->s.effects |= EF_FLIES;
	self->s.sound = gi.soundindex ("infantry/inflies1.wav");
	self->think = M_FliesOff;
	self->nextthink = level.time + 60;
}

void M_FlyCheck (edict_t *self)
{
	if (self->waterlevel)
		return;

	if (random() > 0.5)
		return;

	self->think = M_FliesOn;
	self->nextthink = level.time + 5 + 10 * random();
}

void AttackFinished (edict_t *self, float time)
{
	self->monsterinfo.attack_finished = level.time + time;
}


void M_CheckGround (edict_t *ent)
{
	vec3_t		point;
	trace_t		trace;

	if (ent->flags & (FL_SWIM|FL_FLY))
		return;

	if (ent->velocity[2] > 100)
	{
		ent->groundentity = NULL;
		return;
	}

// if the hull point one-quarter unit down is solid the entity is on ground
	point[0] = ent->s.origin[0];
	point[1] = ent->s.origin[1];
	point[2] = ent->s.origin[2] - 0.25;

	trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, point, ent, MASK_MONSTERSOLID);

	// check steepness
	if ( trace.plane.normal[2] < 0.7 && !trace.startsolid)
	{
		ent->groundentity = NULL;
		return;
	}

//	ent->groundentity = trace.ent;
//	ent->groundentity_linkcount = trace.ent->linkcount;
//	if (!trace.startsolid && !trace.allsolid)
//		VectorCopy (trace.endpos, ent->s.origin);
	if (!trace.startsolid && !trace.allsolid)
	{
		VectorCopy (trace.endpos, ent->s.origin);
		ent->groundentity = trace.ent;
		ent->groundentity_linkcount = trace.ent->linkcount;
		ent->velocity[2] = 0;
	}
}


void M_CatagorizePosition (edict_t *ent)
{
	vec3_t		point;
	int			cont;

//
// get waterlevel
//
	point[0] = ent->s.origin[0];
	point[1] = ent->s.origin[1];
	point[2] = ent->s.origin[2] + ent->mins[2] + 1;	
	cont = gi.pointcontents (point);

	if (!(cont & MASK_WATER))
	{
		ent->waterlevel = 0;
		ent->watertype = 0;
		return;
	}

	ent->watertype = cont;
	ent->waterlevel = 1;
	point[2] += 26;
	cont = gi.pointcontents (point);
	if (!(cont & MASK_WATER))
		return;

	ent->waterlevel = 2;
	point[2] += 22;
	cont = gi.pointcontents (point);
	if (cont & MASK_WATER)
		ent->waterlevel = 3;
}


void M_WorldEffects (edict_t *ent)
{
	int		dmg;

	if (ent->health > 0) {
		if (!(ent->flags & FL_SWIM)) {
			if (ent->waterlevel < 3) {
				ent->air_finished = level.time + 12;
			} else if (ent->air_finished < level.time) { // drown!
				if (ent->pain_debounce_time < level.time) {
					dmg = 2 + 2 * floor(level.time - ent->air_finished);
					if (dmg > 15)
						dmg = 15;
					T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
					ent->pain_debounce_time = level.time + 1;
				}
			}
		} else {
			if (ent->waterlevel > 0) {
				ent->air_finished = level.time + 9;
			} else if (ent->air_finished < level.time) {	// suffocate!
				if (ent->pain_debounce_time < level.time) {
					dmg = 2 + 2 * floor(level.time - ent->air_finished);
					if (dmg > 15)
						dmg = 15;
					T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
					ent->pain_debounce_time = level.time + 1;
				}
			}
		}
	}

	if (ent->waterlevel == 0) {
		if (ent->flags & FL_INWATER) {
			gi.sound (ent, CHAN_BODY, gi.soundindex("player/watr_out.wav"), 1, ATTN_NORM, 0);
			ent->flags &= ~FL_INWATER;
		}
		return;
	}

	if ((ent->watertype & CONTENTS_LAVA) && !(ent->flags & FL_IMMUNE_LAVA)) {
		if (ent->damage_debounce_time < level.time) {
			ent->damage_debounce_time = level.time + 0.2;
			T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 10*ent->waterlevel, 0, 0, MOD_LAVA);
		}
	}
	if ((ent->watertype & CONTENTS_SLIME) && !(ent->flags & FL_IMMUNE_SLIME)) {
		if (ent->damage_debounce_time < level.time) {
			ent->damage_debounce_time = level.time + 1;
			T_Damage (ent, world, world, vec3_origin, ent->s.origin, vec3_origin, 4*ent->waterlevel, 0, 0, MOD_SLIME);
		}
	}

	if ( !(ent->flags & FL_INWATER) ) {
		if (!(ent->svflags & SVF_DEADMONSTER)) {
			if (ent->watertype & CONTENTS_LAVA)
				if (random() <= 0.5)
					gi.sound (ent, CHAN_BODY, gi.soundindex("player/lava1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (ent, CHAN_BODY, gi.soundindex("player/lava2.wav"), 1, ATTN_NORM, 0);
			else if (ent->watertype & CONTENTS_SLIME)
				gi.sound (ent, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
			else if (ent->watertype & CONTENTS_WATER)
				gi.sound (ent, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
		}

		ent->flags |= FL_INWATER;
		ent->damage_debounce_time = 0;
	}
}


void M_droptofloor (edict_t *ent)
{
	vec3_t		end;
	trace_t		trace;

	ent->s.origin[2] += 1;
	VectorCopy (ent->s.origin, end);
	end[2] -= 256;

	trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

	if (trace.fraction == 1 || trace.allsolid)
		return;

	VectorCopy (trace.endpos, ent->s.origin);

	gi.linkentity (ent);
	M_CheckGround (ent);
	M_CatagorizePosition (ent);
}


void M_SetEffects (edict_t *ent)
{
	ent->s.effects &= ~(EF_COLOR_SHELL|EF_POWERSCREEN);
	ent->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);

/*	if (ent->monsterinfo.aiflags & AI_RESURRECTING)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_RED;
	}*/

	if (ent->health <= 0)
		return;

	if (!(ent->monsterinfo.ability & GIEX_MABILITY_STEALTH)) {
		if (ent->monsterinfo.skill > 7) {
			ent->s.effects |= EF_COLOR_SHELL;
			if (ent->monsterinfo.skill < 16) { // 8 - 15 = green
				ent->s.renderfx |= RF_SHELL_GREEN;
			} else if (ent->monsterinfo.skill < 24) { // 16 - 23 = red
				ent->s.renderfx |= RF_SHELL_RED;
			} else { // 24 - 30 = yellow
				ent->s.renderfx |= RF_SHELL_GREEN | RF_SHELL_RED;
			}
		}
		if (ent->radius_dmg) {
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_BLUE;
		}
	}
	if (ent->monsterinfo.superchamp == 1) {
		ent->s.effects |= EF_FLAG2;
	} else if (ent->monsterinfo.superchamp == 2) {
		ent->s.effects |= EF_FLAG1;
	}

	if (ent->powerarmor_time > level.time)
	{
		if (ent->monsterinfo.power_armor_type == POWER_ARMOR_SCREEN)
		{
			ent->s.effects |= EF_POWERSCREEN;
		}
		else if (ent->monsterinfo.power_armor_type == POWER_ARMOR_SHIELD)
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_GREEN;
		}
	}
}


void M_MoveFrame (edict_t *self)
{
	mmove_t	*move;
	int		index;

	move = self->monsterinfo.currentmove;
	self->nextthink = level.time + FRAMETIME;

	if ((self->monsterinfo.nextframe) && (self->monsterinfo.nextframe >= move->firstframe) && (self->monsterinfo.nextframe <= move->lastframe))
	{
		self->s.frame = self->monsterinfo.nextframe;
		self->monsterinfo.nextframe = 0;
	}
	else
	{
		if (self->s.frame == move->lastframe)
		{
			if (move->endfunc)
			{
				move->endfunc (self);

				// regrab move, endfunc is very likely to change it
				move = self->monsterinfo.currentmove;

				// check for death
				if ((self->svflags & SVF_DEADMONSTER) || (!self->inuse))
					return;
			}
		}

		if (self->s.frame < move->firstframe || self->s.frame > move->lastframe)
		{
			self->monsterinfo.aiflags &= ~AI_HOLD_FRAME;
			self->s.frame = move->firstframe;
		}
		else
		{
			if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME))
			{
				self->s.frame++;
				if (self->s.frame > move->lastframe)
					self->s.frame = move->firstframe;
			}
		}
	}

	index = self->s.frame - move->firstframe;
	if (move->frame[index].aifunc) {
		if (!(self->monsterinfo.aiflags & AI_HOLD_FRAME)) {
			float mult = 1.0;

			if ((move->frame[index].aifunc == ai_run) || (move->frame[index].aifunc == ai_charge) || (move->frame[index].aifunc == ai_run_slide)) {
				mult = 0.9 + 0.02 * self->monsterinfo.skill;
			}
			if (move->frame[index].aifunc == ai_run_circle) {
				mult = 1.0 + 0.01 * self->monsterinfo.skill;
			}
			if ((self->classid == CI_M_INSANE) && (self->owner) && (self->owner->client)) {
				mult = 4.0;
			}

			if (self->radius_dmg && self->monsterinfo.ability & GIEX_MABILITY_SPEED) {
				mult += GIEX_MABILITY_SPEED_BONUS;
			}

			mult *= (self->speed + 1);
			if (self->speed < 0)
				self->speed += 0.01;
//			gi.dprintf("%f, %f, %f\n", self->speed, self->accel, self->decel);

			move->frame[index].aifunc (self, move->frame[index].dist * self->monsterinfo.scale * mult);
		} else {
			move->frame[index].aifunc (self, 0);
		}
	}

	if (move->frame[index].thinkfunc)
		move->frame[index].thinkfunc (self);
}


void monster_think (edict_t *self) {
	edict_t *scan;
// Regen aura, every second
	if (self->radius_dmg && (((int)ceil(level.time * 10 - 0.5) % 10) == 0) && (self->health > 0) && (self->monsterinfo.ability & GIEX_MABILITY_REGENAURA)) {
		for (scan = g_edicts ; scan < &g_edicts[globals.num_edicts]; scan++) {
			if (!scan->inuse)
				continue;
			if (!(scan->svflags & SVF_MONSTER))
				continue;
			if (scan->monsterinfo.healtime > level.time)
				continue;
			if (!visible(self, scan))
				continue;
			if (scan->health >= scan->max_health)
				continue;
			if (scan->health < 1)
				continue;
			if (scan->enemy && scan->enemy == self)
				continue;

			gi.sound(scan, CHAN_ITEM, gi.soundindex("items/m_health.wav"), 0.5, ATTN_NORM, 0);
			scan->monsterinfo.healtime = level.time + 0.5;
			scan->health += 15 + self->monsterinfo.skill * 4;
			if (scan->health > scan->max_health)
				scan->health = scan->max_health;
		}
	}
	if (!self->enemy && (level.time > self->monsterinfo.strolltime)) {
		self->monsterinfo.strolltime = level.time + 2 + random() * 1;
/*		self->ideal_yaw += random() * 40 - 20;
		if (self->ideal_yaw < -180)
			self->ideal_yaw += 360;
		if (self->ideal_yaw > 180)
			self->ideal_yaw -= 360;*/
		self->monsterinfo.goal_strollyaw = self->ideal_yaw + random() * 90 - 45;
		if (self->monsterinfo.goal_strollyaw < -180)
			self->monsterinfo.goal_strollyaw += 360;
		if (self->monsterinfo.goal_strollyaw > 180)
			self->monsterinfo.goal_strollyaw -= 360;
		self->monsterinfo.walk(self);
//		gi.dprintf("%s without enemy\n", self->classname);
	}
	if (level.time < self->monsterinfo.strolltime) {
		if (abs(self->monsterinfo.goal_strollyaw - self->ideal_yaw) > 10) {
			if (self->monsterinfo.lefty == 0) {
				if (self->ideal_yaw < self->monsterinfo.goal_strollyaw) {
					if (self->ideal_yaw < self->monsterinfo.goal_strollyaw - 180) {
						self->monsterinfo.goal_strollyaw -= 360;
						self->monsterinfo.lefty = 1;
					} else {
						self->monsterinfo.lefty = -1;
					}
				} else {
					if (self->ideal_yaw > self->monsterinfo.goal_strollyaw + 180) {
						self->monsterinfo.goal_strollyaw += 360;
						self->monsterinfo.lefty = -1;
					} else {
						self->monsterinfo.lefty = 1;
					}
				}
			}
			if (self->monsterinfo.lefty > 0) {
				self->ideal_yaw -= 8;
				if (self->ideal_yaw < -180)
					self->ideal_yaw += 360;
			} else if (self->monsterinfo.lefty < 0) {
				self->ideal_yaw += 8;
				if (self->ideal_yaw > 180)
					self->ideal_yaw -= 360;
			}

	/*		if (self->ideal_yaw < self->monsterinfo.goal_strollyaw) {
				if (self->ideal_yaw < self->monsterinfo.goal_strollyaw - 180) {
					self->monsterinfo.goal_strollyaw -= 360;
					self->ideal_yaw -= 5;
				} else {
					self->ideal_yaw += 5;
				}
			} else {
				if (self->ideal_yaw > self->monsterinfo.goal_strollyaw + 180) {
					self->monsterinfo.goal_strollyaw += 360;
					self->ideal_yaw += 5;
				} else {
					self->ideal_yaw -= 5;
				}
			}*/
		} else {
			self->monsterinfo.lefty = 0;
		}
	}

	M_MoveFrame (self);
	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}
	M_CatagorizePosition (self);
	M_WorldEffects (self);
	M_SetEffects (self);
}


/*
================
monster_use

Using a monster makes it angry at the current activator
================
*/
void monster_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!activator)
		return;
	if (self->enemy)
		return;
	if (self->health <= 0)
		return;
 	if (activator->flags & FL_NOTARGET)
		return;
	if (!(activator->client) && !(activator->monsterinfo.aiflags & AI_GOOD_GUY))
		return;

// delay reaction so if the monster is teleported, its sound is still heard
	self->enemy = activator;
	FoundTarget (self);
}


void monster_start_go (edict_t *self);


void monster_triggered_spawn (edict_t *self)
{
	self->s.origin[2] += 1;
	KillBox (self);

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_STEP;
	self->svflags &= ~SVF_NOCLIENT;
	self->air_finished = level.time + 12;
	gi.linkentity (self);

	monster_start_go (self);

	if (self->enemy && !(self->spawnflags & 1) && !(self->enemy->flags & FL_NOTARGET)) {
		FoundTarget (self);
	} else {
		self->enemy = NULL;
	}
}

void monster_triggered_spawn_use (edict_t *self, edict_t *other, edict_t *activator)
{
	// we have a one frame delay here so we don't telefrag the guy who activated us
	self->think = monster_triggered_spawn;
	self->nextthink = level.time + FRAMETIME;
	if (activator->client)
		self->enemy = activator;
	self->use = monster_use;
}

void monster_triggered_start (edict_t *self)
{
	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;
	self->svflags |= SVF_NOCLIENT;
	self->nextthink = 0;
	self->use = monster_triggered_spawn_use;
}


/*
================
monster_death_use

When a monster dies, it fires all of its targets with the current
enemy as activator.
================
*/
void monster_death_use (edict_t *self)
{
	self->flags &= ~(FL_FLY|FL_SWIM);
	self->monsterinfo.aiflags &= AI_GOOD_GUY;

	if (self->item)
	{
		Drop_Item (self, self->item);
		self->item = NULL;
	}

	if (self->deathtarget)
		self->target = self->deathtarget;

	if (!self->target)
		return;

	G_UseTargets (self, self->enemy);
}


//============================================================================

void makeRandomMonsterName(edict_t *self) {
	int i, l = 0, al;
	int c = (int) (random() * 2 + 2);
	char *add;
	int r;

	for (i = 0; i < c; i++) {
		r = random() * 14;
		switch(r) {
		case 1:
			add = "Bee";
			break;
		case 2:
			add = "Moo";
			break;
		case 3:
			add = "Bah";
			break;
		case 4:
			add = "Peefo";
			break;
		case 5:
			add = "Mulke";
			break;
		case 6:
			add = "Miffo";
			break;
		case 7:
			add = "Hibba";
			break;
		case 8:
			add = "Tre";
			break;
		case 9:
			add = "Lu";
			break;
		case 10:
			add = "Huffa";
			break;
		case 11:
			add = "Finkle";
			break;
		case 12:
			add = "Derpa";
			break;
		case 13:
			add = "Ray";
			break;
		default: 
			add = "Bungle";
			break;
		}
		al = strlen(add);
		if (l + al > 127) {
			break;
		}
		strcpy(self->monsterinfo.name + l, add);
		l += al;
	}
	if (self->monsterinfo.superchamp > 1) {
		r = random() * 12;
		switch(r) {
		case 1:
			add = " the Nasty";
			break;
		case 2:
			add = " the Foul";
			break;
		case 3:
			add = " the Grief";
			break;
		case 4:
			add = " the Obliterator";
			break;
		case 5:
			add = " the Bastard";
			break;
		case 6:
			add = " the Festering";
			break;
		case 7:
			add = " the Awful";
			break;
		case 8:
			add = " the Wretched";
			break;
		case 9:
			add = " the Insane";
			break;
		case 10:
			add = " the Destroyer";
			break;
		case 11:
			add = " the Disgusting";
			break;
		default:
			r = random() * 15;
			switch(r) {
			case 1:
				add = " the Happy Clown";
				break;
			case 2:
				add = " the Fluffy Puppy";
				break;
			case 3:
				add = " the Cuddly Lamb";
				break;
			case 4:
				add = " the Silly Sod";
				break;
			case 5:
				add = " the Nifty Thingie";
				break;
			case 6:
				add = " the Little Douchebag";
				break;
			case 7:
				add = " the Krusty Clown";
				break;
			default:
				add = " the Evil";
				break;
			}
		}
		al = strlen(add);
		if (l + al < 128) {
			strcpy(self->monsterinfo.name + l, add);
			l += al;
		}
	}
}

qboolean monster_start (edict_t *self)
{
/*	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return false;
	}
*/
	if (self->monsterinfo.superchamp > 0) {
		self->monsterinfo.aggressive = true;
		makeRandomMonsterName(self);
		if (self->monsterinfo.superchamp > 1) {
			self->monsterinfo.level *= 4;
		} else {
			self->monsterinfo.level *= 8;
		}
		if (deathmatch->value) {
			if (self->monsterinfo.superchamp > 1) {
				gi.bprintf(PRINT_CHAT, "%s", self->monsterinfo.name);
				gi.bprintf(PRINT_HIGH, " enters the world! Tremble in fear!\n");
			} else {
				gi.bprintf(PRINT_CHAT, "%s", self->monsterinfo.name);
				gi.bprintf(PRINT_HIGH, " enters the world!\n");
			}
		}
	}
	if ((self->spawnflags & 4) && !(self->monsterinfo.aiflags & AI_GOOD_GUY))
	{
		self->spawnflags &= ~4;
		self->spawnflags |= 1;
//		gi.dprintf("fixed spawnflags on %s at %s\n", self->classname, vtos(self->s.origin));
	}

	if (!(self->monsterinfo.aiflags & AI_GOOD_GUY))
		level.total_monsters++;

	self->nextthink = level.time + FRAMETIME;
	self->svflags |= SVF_MONSTER;
//	self->s.renderfx |= RF_FRAMELERP;
	self->takedamage = DAMAGE_AIM;
	self->air_finished = level.time + 12;
	self->use = monster_use;
	self->health *= getMonsterBonus(self);
	self->max_health = self->health;
	self->clipmask = MASK_MONSTERSOLID;

	self->s.skinnum = 0;
	self->deadflag = DEAD_NO;
	self->svflags &= ~SVF_DEADMONSTER;

	if (!self->monsterinfo.checkattack)
		self->monsterinfo.checkattack = M_CheckAttack;
	VectorCopy (self->s.origin, self->s.old_origin);

	if (st.itemid) {
		self->item = FindItemByClassid (st.itemid);
		if (!self->item)
			gi.dprintf("%s at %s has bad item: %s\n", self->classname, vtos(self->s.origin), st.item);
	}

	// randomize what frame they start on
	if (self->monsterinfo.currentmove)
		self->s.frame = self->monsterinfo.currentmove->firstframe + (rand() % (self->monsterinfo.currentmove->lastframe - self->monsterinfo.currentmove->firstframe + 1));

	return true;
}

void monster_start_go (edict_t *self)
{
	vec3_t	v;

	if (self->health <= 0)
		return;

	// check for target to combat_point and change to combattarget
	if (self->target)
	{
		qboolean	notcombat;
		qboolean	fixup;
		edict_t		*target;

		target = NULL;
		notcombat = false;
		fixup = false;
		while ((target = G_Find (target, FOFS(targetname), self->target)) != NULL) {
			if (target->classid == CI_POINT_COMBAT) {
				self->combattarget = self->target;
				fixup = true;
			} else {
				notcombat = true;
			}
		}
		if (notcombat && self->combattarget)
			gi.dprintf("%s at %s has target with mixed types\n", self->classname, vtos(self->s.origin));
		if (fixup)
			self->target = NULL;
	}

	// validate combattarget
	if (self->combattarget) {
		edict_t		*target;

		target = NULL;
		while ((target = G_Find (target, FOFS(targetname), self->combattarget)) != NULL) {
			if (target->classid != CI_POINT_COMBAT) {
				gi.dprintf("%s at (%i %i %i) has a bad combattarget %s : %s at (%i %i %i)\n",
					self->classname, (int)self->s.origin[0], (int)self->s.origin[1], (int)self->s.origin[2],
					self->combattarget, target->classname, (int)target->s.origin[0], (int)target->s.origin[1],
					(int)target->s.origin[2]);
			}
		}
	}

	if (self->target)
	{
		self->goalentity = self->movetarget = G_PickTarget(self->target);
		if (!self->movetarget) {
			gi.dprintf ("%s can't find target %s at %s\n", self->classname, self->target, vtos(self->s.origin));
			self->target = NULL;
			self->monsterinfo.pausetime = 100000000;
			self->monsterinfo.stand (self);
		} else if (self->movetarget->classid == CI_PATHCORNER) {
			VectorSubtract (self->goalentity->s.origin, self->s.origin, v);
			self->ideal_yaw = self->s.angles[YAW] = vectoyaw(v);
			self->monsterinfo.walk (self);
			self->target = NULL;
		} else {
			self->goalentity = self->movetarget = NULL;
			self->monsterinfo.pausetime = 100000000;
			self->monsterinfo.stand (self);
		}
	}
	else
	{
		self->monsterinfo.pausetime = 100000000;
		self->monsterinfo.stand (self);
	}

	self->think = monster_think;
	if (deathmatch->value)
		self->nextthink = level.time + 35 * FRAMETIME;
	else
		self->nextthink = level.time + FRAMETIME;
	if (self->radius_dmg) {
		float r, mult = 1.0;
		int count = 1, i;

		self->s.effects |= EF_COLOR_SHELL;
		self->s.renderfx |= RF_SHELL_BLUE;

		self->health *= 3;
		self->monsterinfo.level *= 4;
		self->gib_health *= 2;
		self->monsterinfo.cts_any *= 2.0;
		self->monsterinfo.cts_class1 *= 2.0;
		self->monsterinfo.cts_class2 *= 1.8;
		self->monsterinfo.cts_class3 *= 1.5;
		self->monsterinfo.item_any *= 2.0;
		self->monsterinfo.itemmult_class1 *= 1.5;
		self->monsterinfo.itemmult_class2 *= 1.3;
		self->monsterinfo.itemmult_class3 *= 1.1;
		self->monsterinfo.itemmult_class4 *= 0.7;

		if (self->monsterinfo.skill > 7) {
			if (self->monsterinfo.skill < 16) { // 8 - 15 = green
				count = (int) ceil(0.3 + random() * 2);
			} else if (self->monsterinfo.skill < 24) { // 16 - 23 = red
				count = (int) ceil(1.4 + random() * 2);
			} else { // 24 - 30 = yellow
				count = (int) ceil(2.6 + random() * 2);
			}
		}
		if (self->monsterinfo.ability == 0) {
			float stealth_chance = 0.005;
			if (game.monsterhunt == 10) {
				count += 3;
				stealth_chance = 0.04;
			}
			for (i = 0; i < count; i++) {
				r = random();
				if ((r < 0.1) && !(self->monsterinfo.ability & GIEX_MABILITY_HEALTH)) { //10% for extra health
					self->monsterinfo.ability |= GIEX_MABILITY_HEALTH;
					self->health *= 2;
					mult += 0.25;
				} else if ((r < 0.16) && !(self->monsterinfo.ability & GIEX_MABILITY_SPEED)) { //6% for extra speed
					self->monsterinfo.ability |= GIEX_MABILITY_SPEED;
					mult += 0.4;
				} else if ((r < 0.21) && !(self->monsterinfo.ability & GIEX_MABILITY_DAMAGE)) { //5% for extra damage
					self->monsterinfo.ability |= GIEX_MABILITY_DAMAGE;
					mult += 0.35;
				} else if ((r < 0.29) && !(self->monsterinfo.ability & GIEX_MABILITY_RES_EXPL)) { //8% for explosion resistance
					self->monsterinfo.ability |= GIEX_MABILITY_RES_EXPL;
					mult += 0.2;
				} else if ((r < 0.34) && !(self->monsterinfo.ability & GIEX_MABILITY_RES_IMPACT)) { //5% for impact resistance
					self->monsterinfo.ability |= GIEX_MABILITY_RES_IMPACT;
					mult += 0.2;
				} else if ((r < 0.41) && !(self->monsterinfo.ability & GIEX_MABILITY_RES_ENERGY)) { //7% for energy resistance
					self->monsterinfo.ability |= GIEX_MABILITY_RES_ENERGY;
					mult += 0.2;
				} else if ((r < 0.47) && !(self->monsterinfo.ability & GIEX_MABILITY_RES_FMAGIC)) { //6% for fire magic resistance
					self->monsterinfo.ability |= GIEX_MABILITY_RES_FMAGIC;
					mult += 0.2;
				} else if ((r < 0.53) && !(self->monsterinfo.ability & GIEX_MABILITY_RES_BMAGIC)) { //6% for blood magic resistance
					self->monsterinfo.ability |= GIEX_MABILITY_RES_BMAGIC;
					mult += 0.2;
				} else if ((r < 0.59) && !(self->monsterinfo.ability & GIEX_MABILITY_RES_LMAGIC)) { //6% for lightning magic resistance
					self->monsterinfo.ability |= GIEX_MABILITY_RES_LMAGIC;
					mult += 0.2;
				} else if ((r < 0.63) && !(self->monsterinfo.ability & GIEX_MABILITY_REGENAURA) && (game.monsterhunt != 10)) { //4% for regen aura
					self->monsterinfo.ability |= GIEX_MABILITY_REGENAURA;
					mult += 0.3;
				} else if ((r < 0.66) && !(self->monsterinfo.ability & GIEX_MABILITY_SHARDARMOR)) { //3% for shard armor
					self->monsterinfo.ability |= GIEX_MABILITY_SHARDARMOR;
					mult += 0.3;
				} else if ((r < 0.71) && !(self->monsterinfo.ability & GIEX_MABILITY_ARMORPIERCE)) { //5% for armor piercing
					self->monsterinfo.ability |= GIEX_MABILITY_ARMORPIERCE;
					mult += 0.25;
				} else if ((r < 0.76) && !(self->monsterinfo.ability & GIEX_MABILITY_MANABURN)) { //5% for manaburn
					self->monsterinfo.ability |= GIEX_MABILITY_MANABURN;
					mult += 0.25;
				} else if ((r < 0.80) && !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) { //4% for silenced
					self->monsterinfo.ability |= GIEX_MABILITY_SILENCED;
					mult += 0.2;
				} else if ((r < 0.80 + stealth_chance) && !(self->monsterinfo.ability & GIEX_MABILITY_STEALTH) && (self->monsterinfo.skill > 23) && !coop->value && (self->classid != CI_M_FLYER)) { //0.5%/5% for STEALTH! (but not in coop and not for flyers)
					self->monsterinfo.ability |= GIEX_MABILITY_STEALTH;
					mult += 0.3;
				}
				//about 33% chance for no ability
			}
//					self->monsterinfo.ability |= GIEX_MABILITY_STEALTH;
//					mult += 0.3;
			self->monsterinfo.level *= mult;
		}

		self->max_health = self->health;

		if (self->monsterinfo.ability & GIEX_MABILITY_STEALTH) {
			addStealth();
			if ((game.monsterhunt == 10) && !(self->monsterinfo.ability & GIEX_MABILITY_SILENCED)) {
				self->monsterinfo.ability |= GIEX_MABILITY_SILENCED;
				self->monsterinfo.level *= 0.2;
			}
		}
	}
}


void walkmonster_start_go (edict_t *self)
{
	if (!(self->spawnflags & 2) && level.time < 1)
	{
		M_droptofloor (self);

		if (self->groundentity)
			if (!M_walkmove (self, 0, 0))
				gi.dprintf ("%s in solid at %s\n", self->classname, vtos(self->s.origin));
	}

	if (!self->yaw_speed)
		self->yaw_speed = 35;
	self->viewheight = 25;

	monster_start_go (self);

	if (self->spawnflags & 2)
		monster_triggered_start (self);
}

void walkmonster_start (edict_t *self)
{
	self->think = walkmonster_start_go;
	monster_start (self);
}


void flymonster_start_go (edict_t *self)
{
	if (!M_walkmove (self, 0, 0))
		gi.dprintf ("%s in solid at %s\n", self->classname, vtos(self->s.origin));

	if (!self->yaw_speed)
		self->yaw_speed = 30;
	self->viewheight = 25;

	monster_start_go (self);

	if (self->spawnflags & 2)
		monster_triggered_start (self);
}


void flymonster_start (edict_t *self)
{
	self->flags |= FL_FLY;
	self->think = flymonster_start_go;
	monster_start (self);
}


void swimmonster_start_go (edict_t *self)
{
	if (!self->yaw_speed)
		self->yaw_speed = 30;
	self->viewheight = 10;

	monster_start_go (self);

	if (self->spawnflags & 2)
		monster_triggered_start (self);
}

void swimmonster_start (edict_t *self)
{
	self->flags |= FL_SWIM;
	self->think = swimmonster_start_go;
	monster_start (self);
}

// Chamooze
// Sometimes spawn a monster in DM

void ED_CallSpawn (edict_t *ent);
qboolean KillBox2 (edict_t *ent);
float PlayersRangeFromSpot (edict_t *spot);
edict_t *SelectMonsterSpawn (int size)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_FindClassId(spot, CI_GIEX_MONSTERSPAWN)) != NULL) {
		if (spot->spawnflags != 0)
			continue;
		if (spot->dmg < size)
			continue;
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do {
		spot = G_FindClassId(spot, CI_GIEX_MONSTERSPAWN);
		if (spot->spawnflags != 0) {
			selection++;
			continue;
		}
		if (spot->dmg < size) {
			selection++;
			continue;
		}
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

edict_t *SelectMonsterSpawnGroup (int group)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_FindClassId (spot, CI_GIEX_MONSTERSPAWN)) != NULL) {
		if (spot->spawnflags == 0)
			continue;
		if (spot->count != group)
			continue;
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do {
		spot = G_FindClassId(spot, CI_GIEX_MONSTERSPAWN);
		if (!spot) {
			continue;
		}
		if (spot->spawnflags == 0) {
			selection++;
			continue;
		}
		if (spot->count != group) {
			selection++;
			continue;
		}
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

void spawnGroupMonster(unsigned int classid, int group)
{
	edict_t *ent, *spot;
	spot = SelectMonsterSpawnGroup(group);
	if (!spot) {
		gi.dprintf("No spawn found for the %d\n", classid);
		return;
	}

	ent = G_Spawn();

	VectorCopy (spot->s.origin, ent->s.origin);
	ent->s.origin[2] += 9;
	VectorCopy (spot->s.angles, ent->s.angles);

	ent->classid = classid;
	ent->classname = NULL;
	ent->radius_dmg = 0;
	ED_CallSpawn (ent);

	gi.unlinkentity (ent);
	if (!KillBox2 (ent)) {
		level.total_monsters--;
		G_FreeEdict(ent);
		return;
	}
	gi.linkentity (ent);
	gi.sound(ent, CHAN_AUTO, gi.soundindex("giex/monresp.wav"), 1, ATTN_NORM, 0);
}

void EndDMLevel (void);
void spawnMonster(unsigned int classid, int size) {
	edict_t *ent, *spot;
	int group = 0;
	float r = 0.9;
	spot = SelectMonsterSpawn(size);
	if (!spot) {
		gi.dprintf("No spawn found for the %d\n", classid);
		if (game.monsterhunt == 10) {
			gi.bprintf(PRINT_HIGH, "This map can't handle spawning big monsters, trying the next one.\n");
			//game.monsterhunt--;
			EndDMLevel ();
		}
		return;
	}

	ent = G_Spawn();

	VectorCopy (spot->s.origin, ent->s.origin);
	ent->s.origin[2] += 9;
	VectorCopy (spot->s.angles, ent->s.angles);

	ent->classid = classid;
	ent->radius_dmg = 0;
	if (spot->count > 0)
		r = 0.5; // Higher probability of Champ monster, if a "group" spawn

	if (game.monsterhunt == 10) {
		ent->radius_dmg = 1;
	} else {
		r = -1;
		if (random() > r) {
			ent->radius_dmg = 1;

			if (spot->count > 0)
				group = spot->count;
		}
		if (ent->radius_dmg > 0) {
			float r = random();
			if (r < 0.02) {
				ent->monsterinfo.superchamp = 2;
			} else if (r < 0.06) {
				ent->monsterinfo.superchamp = 1;
			}
		}
	}
	ent->classname = NULL;

	ED_CallSpawn (ent);

	gi.unlinkentity (ent);
	if (!KillBox2 (ent)) {
//		gi.dprintf("Spawn already occupied while spawning %s\n", ent->classname);
		level.total_monsters--;
		G_FreeEdict(ent);
		return;
	}
	gi.linkentity (ent);
	if (group > 0) {
		int i;
		for (i = 0; i < 7; i++) {
			spawnGroupMonster(classid, group);
		}
//		gi.dprintf("Let's spawn groupies for the nasty bastard!\n");
	}
	gi.sound(ent, CHAN_AUTO, gi.soundindex("giex/monresp.wav"), 1, ATTN_NORM, 0);
}

void spawnDMMonster(void) {
	unsigned int classid;
	int size, sum = 0;
	edict_t *ent = NULL;
	static int num_players = 0;
	int r, player, playerlvl = 0, newskill;
	char command[32];
	float ran;
	int spawn_time;
	int max_monsters = 10;
	float time_mult = 1.0;
	float level_mult = 1.0;

	if (!deathmatch->value)
		return;
	if (!dmmonsters->value)
		return;
	if (level.intermissiontime)
		return;

	if (game.monsterhunt == 10) {
		if ((int)(level.time * 1000) == 20000)
			gi.bprintf(PRINT_HIGH, "Monster hunt begins in 10 seconds.\n");
		if ((int)(level.time * 10) < 300)
			return;
	} else {
		if ((int)(level.time * 10) < 100)
			return;
	}
	if (level.monspawncount > 20) {
		max_monsters += (level.monspawncount - 20) / 2;
	}
	if (game.craze == 10) {
		time_mult = 0.1;
		level_mult = 0.5;
		max_monsters *= 2;
	}
	if (game.monsterhunt == 10) {
		time_mult = 0.1;
		level_mult = 1.2;
		max_monsters = 1;
	}
	if (teams->value != 3) {
		spawn_time = (int)((35 + num_players * 5) * dmmonsters->value * time_mult);
	} else {
		spawn_time = (int)((60 - num_players * 10) * dmmonsters->value * time_mult);
	}
	if (spawn_time < 8)
		spawn_time = 8;

	if (((int)ceil(level.time * 10 - 0.5) % spawn_time) != 1)
		return;
//	gi.dprintf("tot: %d, killed: %d, max: %d\n", level.total_monsters, level.killed_monsters, max_monsters);
	if ((level.total_monsters - level.killed_monsters) > max_monsters)
		return;
	if ((game.monsterhunt == 10) && (level.total_monsters > 0)) // Only spawn one monster in Monster hunt
		return;

	num_players = 0;
	for (r=0 ; r<maxclients->value ; r++) {
		ent = g_edicts + 1 + r;
		if (!ent->inuse)
			continue;
		if (!ent->client->pers.loggedin)
			continue;

		num_players++;
		sum += ent->radius_dmg;
	}
	if (num_players < 1) {
		return;
	}
	player = (int) (random() * num_players);
	for (r=0 ; r<maxclients->value ; r++) {
		ent = g_edicts + 1 + r;
		if (!ent->inuse)
			continue;
		if (!ent->client->pers.loggedin)
			continue;

		player--;
		if (player < 0) {
			playerlvl = ent->radius_dmg;
			break;
		}
	}

	if ((autoskill->value) || (game.monsterhunt == 10)) {
#define GIEX_MONSTER_PLAYERLEVEL_MULT		1.5
#define GIEX_MONSTER_PLAYERLEVEL_MULT_POW	0.5
		if (game.monsterhunt == 10) {
			newskill = (int) (pow( ((float) sum / (float) (num_players * 0.8)), GIEX_MONSTER_PLAYERLEVEL_MULT_POW) * GIEX_MONSTER_PLAYERLEVEL_MULT) * level_mult + 8.0;
//			newskill = 0;
		} else {
			newskill = (int) baseskill->value * ((float) pow(playerlvl, GIEX_MONSTER_PLAYERLEVEL_MULT_POW) * GIEX_MONSTER_PLAYERLEVEL_MULT) + random() * 16 - 7;
			newskill *= level_mult;
			if (newskill < 0)
				newskill = 0;
			if (newskill > 30)
				newskill = 30;
		}
		if (newskill != skill->value) {
			sprintf(command, "%d", newskill);
			gi.cvar_forceset("skill", command);
		}
	}

	if (game.monsterhunt == 10) {
		ran = random();
		if (ran<0.66) {
			classid = CI_M_JORG; size = 4;
		} else {
			classid = CI_M_SUPERTANK; size = 4;
		}
	} else {
		r = (int) (random() * 15);
		switch(r) {
			case 0: classid = CI_M_SOLDIER_LIGHT; size = 1; break;
			case 1: classid = CI_M_SOLDIER; size = 1; break;
			case 2: classid = CI_M_SOLDIER_SS; size = 1; break;
			case 3: classid = CI_M_INFANTRY; size = 1; break;
			case 4: classid = CI_M_PARASITE; size = 1; break;
			case 5: classid = CI_M_CHICK; size = 1; break;
			case 6: classid = CI_M_MUTANT; size = 2; break;
			case 7: classid = CI_M_GUNNER; size = 1; break;
			case 8: classid = CI_M_GLADIATOR; size = 2; break;
			case 9: classid = CI_M_BERSERKER; size = 1; break;
			case 10: classid = CI_M_HOVER; size = 1; break;
			case 11: classid = CI_M_MEDIC; size = 2; break;
			case 12: classid = CI_M_FLYER; size = 1; break;
			case 13: classid = CI_M_BRAIN; size = 2; break;
			case 14:
				ran = random();
				if (ran<0.02) {
					classid = CI_M_JORG; size = 4;
				} else if (ran<0.1){
					classid = CI_M_MAKRON; size = 3;
				} else if (ran<0.3){
					classid = CI_M_TANKCOMM; size = 3;
				} else {
					classid = CI_M_TANK; size = 3;
				}
				break;
			default:
				return;
		}
	}
//	classid = CI_M_BERSERKER; size = 1;
//	classid = CI_M_MAKRON; size = 3;
//	classid = CI_M_INFANTRY; size = 1;
	spawnMonster(classid, size);
}

void monster_corpse_think(edict_t *ent) {
	T_Damage (ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 0.5;
}

int getMonsterCorpseTimeout(void) {
	int result;
	int num_players = 0;
	int i;
	edict_t *ent;
	for (i=0 ; i<maxclients->value ; i++) {
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;
		if (!ent->client->pers.loggedin)
			continue;

		num_players++;
	}
	if (!deathmatch->value) { // Coop
		result = 240;
	} else {
		result = 120 - num_players * 10;
/*		if (teams->value == 3) { // PvM
			result = 120 - num_players * 20;
		} else {
			result = 80 - num_players * 15;
		}*/
	}

	if (result < 15)
		return 15;
	else
		return result;
}
