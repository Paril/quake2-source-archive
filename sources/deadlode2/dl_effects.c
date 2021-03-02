#include "g_local.h"

effect_t effects[MAX_EFFECTS];

field_t dl_efxflds[] =
{
	{"name", EOFS(name), F_LSTRING},
	{"type", EOFS(type), F_INT},
	{"damage", EOFS(damage), F_LSTRING},
	{"offset", EOFS(offset), F_VECTOR},
	{"angle", EOFS(offset), F_VECTOR},
	{"range", EOFS(range), F_INT},
	{"spread", EOFS(spread), F_VECTOR},
	{"tint_color", EOFS(spread), F_VECTOR},
	{"duration", EOFS(duration), F_FLOAT},
	{"delay", EOFS(delay), F_FLOAT},
	{"speed", EOFS(speed), F_FLOAT},
	{"count", EOFS(count), F_INT},
	{"health", EOFS(health), F_INT},
	{"flags", EOFS(flags), F_INT},
	{"dmgflags", EOFS(dmgflags), F_INT},
	{"model", EOFS(model), F_LSTRING},
	{"skin", EOFS(skinnum), F_INT},
	{"frame", EOFS(frame), F_INT},
	{"num_frames", EOFS(num_frames), F_INT},
	{"sound", EOFS(sound), F_LSTRING},
	{"special", EOFS(effect), F_LSTRING},
	{NULL, 0, F_INT}
};

// Explosion
void DieMarker(vec3_t pos) {
edict_t	*mkr;

	// Die Marker
	mkr = G_Spawn();
	VectorCopy (pos, mkr->s.origin);
	mkr->movetype = MOVETYPE_NONE;
	mkr->solid = SOLID_NOT;
	VectorClear (mkr->mins);
	VectorClear (mkr->maxs);
	mkr->s.modelindex = gi.modelindex ("sprites/s_bubble.sp2");
	mkr->owner = NULL;
	mkr->nextthink = level.time + 5.0;
	mkr->think = G_FreeEdict;
	mkr->classname = "die marker";

	gi.linkentity (mkr);
}

void Explosion_Normal(edict_t *src, vec3_t origin) {
vec3_t		end;
trace_t		tr;
int			pc, type;


	VectorCopy(origin, end);
	end[2] -= 8;

	tr = gi.trace(origin, NULL, NULL, end, src, MASK_SOLID);
	pc = gi.pointcontents(origin);

	if (tr.fraction != 1.0) type = 0;
	else {
		type = TE_ROCKET_EXPLOSION;
		if (pc & MASK_WATER) type += 10;
	}

	if (!type)
		type = (random() * 4.0) + TE_EXPLOSION1;

	MCastTempEnt(type, NULL, NULL, origin, NULL, NULL, 0, 0, 0, MULTICAST_PVS);
}



qboolean Efx_CanDamage (edict_t *targ, edict_t *inflictor, vec3_t origin)
{
	vec3_t	dest;
	trace_t	trace;

// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
	{
		VectorAdd (targ->absmin, targ->absmax, dest);
		VectorScale (dest, 0.5, dest);
		trace = gi.trace (origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
		if (trace.ent == targ)
			return true;
		return false;
	}
	
	trace = gi.trace (origin, vec3_origin, vec3_origin, targ->s.origin, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
	trace = gi.trace (origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
	trace = gi.trace (origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
	trace = gi.trace (origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	VectorCopy (targ->s.origin, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	trace = gi.trace (origin, vec3_origin, vec3_origin, dest, inflictor, MASK_SOLID);
	if (trace.fraction == 1.0)
		return true;

	return false;
}


void Effect_TempEntity (edict_t *targ, edict_t *owner, edict_t *attacker, vec3_t origin, vec3_t dir, vec3_t plane, float range, float damage, effect_t *effect) {
char thisEffect[255];
int cnt, te;

	cnt = sub_count(effect->damage);
	if (!cnt)
		return;

	sub_string(effect->damage, thisEffect, random() * cnt);
	DL_GetIdentValue(thisEffect, &te);

	if (TempEntType(te) == 3)
		VectorCopy(attacker->s.origin, dir);

	if (effects->flags & ~EFF_SWAPORIGINS)
		MCastTempEnt(te, targ, attacker, origin, dir, effect->offset, effect->count, effect->skinnum, effect->frame, effect->flags & ~EFF_SWAPORIGINS);
	else
		MCastTempEnt(te, targ, attacker, origin, dir, effect->offset, effect->count, effect->skinnum, effect->frame, MULTICAST_PVS);
}

void Effect_MuzzleFlash (edict_t *owner, vec3_t origin, effect_t *effect) {
char thisEffect[255];
int cnt, mz, mc;

	cnt = sub_count(effect->damage);
	if (!cnt)
		return;

	sub_string(effect->damage, thisEffect, random() * cnt);
	DL_GetIdentValue(thisEffect, &mz);

	if (effect->flags & ~EFF_SWAPORIGINS) mc = effect->flags & ~EFF_SWAPORIGINS;
	else mc = MULTICAST_PVS;

	if (mz >= 1000) MuzzleFlash2(owner, mz-1000, mc);
	else MuzzleFlash(owner, mz, mc);
}

void Effect_Sound (vec3_t origin, effect_t *effect) {
	gi.positioned_sound (origin, g_edicts, CHAN_AUTO, gi.soundindex(effect->sound), 1.0, ATTN_NORM, 0);
}


void ModelEffect_Think(edict_t *ent) {
	if (ent->effect->duration) {
		if (ent->timestamp < level.time) {
			G_FreeEdict(ent);
			return;
		}
	} else if (ent->count <= 0) {
		G_FreeEdict(ent);
		return;
	}

	ent->s.frame++;
	if (ent->s.frame > ent->frame_last) {
		ent->s.frame = ent->frame_first;
		ent->count--;
	}
	ent->nextthink = level.time + FRAMETIME;
}

void Effect_Model (vec3_t origin, effect_t *effect) {
edict_t *model;

	model = G_Spawn();
	VectorCopy(origin, model->s.origin);
	model->classname = "effect_model";
	model->movetype = MOVETYPE_NONE; // effect->dmflags;

	model->s.effects = effect->dmgflags;
	model->s.renderfx = effect->flags & ~EFF_SWAPORIGINS;
	model->s.modelindex = gi.modelindex(effect->model);
	model->s.skinnum = effect->skinnum;
	model->s.frame = effect->frame;
	model->frame_first = effect->frame;
	model->frame_last = effect->frame + effect->num_frames;
	model->s.sound = gi.soundindex(effect->sound);

	model->effect = effect;
	model->timestamp = level.time + effect->duration;
	model->count = effect->count;
	if (!model->count) model->count = 1;

	if (effect->num_frames > 1) {
		model->think = ModelEffect_Think;
		model->nextthink = level.time + FRAMETIME;
	} else {
		model->think = G_FreeEdict;
		if (effect->duration)
			model->nextthink = level.time + effect->duration;
		else
			model->nextthink = level.time + (model->count * FRAMETIME);
	}
	gi.linkentity(model);
}

void Effect_Pusher(edict_t *targ, edict_t *owner, edict_t *attacker, vec3_t src_org, vec3_t dest_org, vec3_t dir, float range, float damage, effect_t *effect) {
float	distance;
double	factor;
vec3_t	dist;

	VectorSubtract(src_org, dest_org, dist);
	distance = VectorLength(dist);

	if (effect->damage) {
		if (evaluate(effect->damage, &factor, damage, distance, range) != 0) {
			gi.dprintf("Effect: %s: Invalid Expression (damage = %s)\n", effect->name, effect->damage);
			return;
		}
	}

	VectorNormalize(dir);
	targ->groundentity = NULL;
	targ->pusher = attacker;
	targ->pushframe = 3; // targ must die in .3 seconds once on-ground for attacker to get credit
	if (effect->flags & 0x01)	factor = -factor;  // pull instead of push
	VectorMA(targ->velocity, (factor / targ->mass) * 30000, dir, targ->velocity);
	targ->velocity[2] += (factor / targ->mass) * 10000;
	gi.dprintf("Push Factor: %f\n", (factor / targ->mass) * 30000);
}

void Effect_Tint(edict_t *targ, edict_t *other, vec3_t src_org, vec3_t dest_org, float range, float damage, effect_t *effect) {
float	distance;
double	factor;
vec3_t	dist;
	
	if (!targ || !targ->client)
		return;

	VectorSubtract(src_org, dest_org, dist);
	distance = VectorLength(dist);

	if (effect->damage) {
		if (evaluate(effect->damage, &factor, damage, distance, range) != 0) {
			gi.dprintf("Effect: %s: Invalid Expression (damage = %s)\n", effect->name, effect->damage);
			return;
		}
	}

	VectorAdd(targ->client->tint, effect->spread, targ->client->tint);
	VectorScale(targ->client->tint, 0.5, targ->client->tint);
	targ->client->tint_alpha += factor;
	targ->client->tint_alpha /= 2.0;
	targ->client->tint_fade = 0.2;
}

void Effect_Confusion(edict_t *targ, edict_t *other, vec3_t src_org, vec3_t dest_org, float range, float damage, effect_t *effect) {
float	distance;
double	factor;
vec3_t	dist;
	
	if (!targ || !targ->client)
		return;

	VectorSubtract(src_org, dest_org, dist);
	distance = VectorLength(dist);

	if (effect->damage) {
		if (evaluate(effect->damage, &factor, damage, distance, range) != 0) {
			gi.dprintf("Effect: %s: Invalid Expression (damage = %s)\n", effect->name, effect->damage);
			return;
		}
	}

	targ->client->concussion_bob += factor;
}

void Effect_Blind(edict_t *targ, edict_t *other, vec3_t src_org, vec3_t dest_org, float range, float damage, effect_t *effect) {
int		ent_type, fov;
vec3_t	dist, end, ang;
float	distance;
double	factor;
edict_t	*player;
trace_t	tr;
	
	// Fast check to see if the entity can see this effect
	if (!gi.inPVS(targ->s.origin, src_org))
		return;

	if (targ->client) {
		ent_type = 0x01; // Its a Client/Player
		VectorCopy(targ->client->ps.viewangles, ang);
		fov = targ->client->ps.fov;
	} else if (targ->supply && targ->supply->type == SUP_VIDSENSOR) {
		ent_type = 0x02; // Its a Video Sensor
		VectorCopy(targ->s.angles, ang);
		fov = targ->supply->range;
	} else if (!DL_strcmp(targ->classname, "camera", -1, false)) {
		ent_type = 0x04; // Its a Camera
		VectorCopy(targ->s.angles, ang);
		fov = targ->random;
	} else if (!DL_strcmp(targ->classname, "projectile", -1, false)) {
		ent_type = 0x08; // Its a Projectile
		VectorCopy(targ->s.angles, ang);
		if (targ->ammotype) fov = targ->ammotype->track_fov;
		else fov = 15;
	} else
		return;	// Its NOT RELEVANT... :)

	// effect those entities who match the flag type
	if ((effect->flags & ~EFF_SWAPORIGINS) && !(effect->flags & ent_type))
		return;

	// Determine view-point for the recieving entity
	VectorCopy(targ->s.origin, end);
	end[2] += targ->viewheight;

	if (!InFOV(end, ang, src_org, fov, false))
		return;

	tr = gi.trace (src_org, NULL, NULL, end, NULL, MASK_OPAQUE);
	if (tr.fraction < 1.0)
		return;

	VectorSubtract(src_org, dest_org, dist);
	distance = VectorLength(dist);

	if (effect->damage) {
		if (evaluate(effect->damage, &factor, damage, distance, range) != 0) {
			gi.dprintf("Effect: %s: Invalid Expression (damage = %s)\n", effect->name, effect->damage);
			return;
		}
	}

	if (factor <= 0.01) factor = 0.01;
	switch (ent_type) {
	case 0x01:	// Blind the player
		targ->client->flash_alpha = 1.5;
		targ->client->flash_fade = factor;
		break;
	case 0x04:	// Blind all players viewing the camera
		for (player = &g_edicts[1]; player <= &g_edicts[(int)maxclients->value]; player++) {
			if (player->inuse && player->client->camera == targ) {
				player->client->flash_alpha = 1.5;
				player->client->flash_fade = factor;
			}
		}
		break;
	case 0x02:	// Blind Video Sensors
		if ((factor > 1.5)	&& (targ->supply->flags & SF_INFRARED))
			G_FreeEdict(targ); // Bright light destroys IR video sensors
		else
			targ->flash_time = level.time + factor;
		break;
	case 0x08:	// and Projectiles (Usually homing ones)
		targ->flash_time = level.time + factor;
		break;
	}
}

void Effect_Persistant (edict_t *targ, edict_t *owner, edict_t *attacker, vec3_t origin, vec3_t dir, vec3_t plane, float damage, ammo_t *amm, effect_t *effect);

void Effect_Damage (edict_t *targ, edict_t *owner, edict_t *attacker, vec3_t src_org, vec3_t dest_org, vec3_t dir, vec3_t plane, float range, float damage, ammo_t *amm, effect_t *effect) {
vec3_t	dist, start;
float	distance, factor = 1.0;
double	dmg = 0.0;

	VectorSubtract(src_org, dest_org, dist);
	distance = VectorLength(dist);

	if (effect->damage) {
		if (evaluate(effect->damage, &dmg, damage, distance, range)) {
			gi.dprintf("Effect: %s: Invalid Expression (damage = %s)\n", effect->name, effect->damage);
			return;
		}
	}
	if (targ && targ->takedamage) {
		if (effect->flags & 0x01)
			Effect_Persistant(targ, owner, attacker, dest_org, dir, plane, dmg, amm, effect);
		else
			T_Damage (targ, owner, attacker, dir, dest_org, plane, (int)dmg, (int)dmg, effect->dmgflags, 0, effect->sound);
	}
	if (amm) {
		if ((distance = DL_FindFarSide(dest_org, dir, start, dmg)) > 0.0) {
			if (dmg > 0.0) factor = (dmg - (distance * distance)) / dmg;
			damage = dmg * factor;
			range = (range - distance) * factor;
			if (amm->ftype == FT_PELLET)
				fire_pellet(owner, attacker, start, dir, range, damage, amm);
			else
				fire_projectile(owner, attacker, start, dir, range, damage, amm, NULL);
		}
	}
}

void Effect_ClientState (edict_t *target, vec3_t src_org, vec3_t dest_org, float range, float damage, effect_t *effect) {
double value;
vec3_t diff;
float distance;

	if (!target || !target->client && effect->damage)
		return;

	VectorSubtract(src_org, dest_org, diff);
	distance = VectorLength(diff);
	if (evaluate(effect->damage, &value, damage, distance, range)) {
		gi.dprintf("Effect: %s: Invalid Expression (damage = %s)\n", effect->name, effect->damage);
		return;
	}
	if (effect->flags & 0x01) {
		switch (effect->type) {
		case BE_STATEDMG: 
			target->client->dmg_factor = (target->client->dmg_factor + value) / 2.0;
			target->client->dmg_time = level.time + effect->duration;
			break;
		case BE_STATEPROT: 
			target->client->prot_factor = (target->client->prot_factor + value) / 2.0;
			target->client->prot_time = level.time + effect->duration;
			break;
		case BE_STATEGRAV: 
			target->client->grav_factor = (target->client->grav_factor + value) / 2.0;
			target->client->grav_time = level.time + effect->duration;
			break;
		case BE_STATESWIM: 
			target->client->swim_factor = (target->client->swim_factor + value) / 2.0;
			target->client->swim_time = level.time + effect->duration;
			break;
		}
	} else {
		switch (effect->type) {
		case BE_STATEDMG: 
			target->client->dmg_factor = (target->client->dmg_factor + effect->speed) / 2.0;
			target->client->dmg_time = level.time + value;
			break;
		case BE_STATEPROT: 
			target->client->prot_factor = (target->client->prot_factor + effect->speed) / 2.0;
			target->client->prot_time = level.time + value;
			break;
		case BE_STATEGRAV: 
			target->client->grav_factor = (target->client->grav_factor + effect->speed) / 2.0;
			target->client->grav_time = level.time + value;
			break;
		case BE_STATESWIM: 
			target->client->swim_factor = (target->client->swim_factor + effect->speed) / 2.0;
			target->client->swim_time = level.time + value;
		break;
		}
	}
}

void Effect_Radius (edict_t *owner, edict_t *attacker, vec3_t src_org, vec3_t dir, vec3_t plane, float range, float damage, ammo_t *amm, effect_t *effect) {
edict_t	*other = NULL;
double	dmg = 0.0;
vec3_t	v, d2, eorg;
int j;

	if (!effect)
		return;

	while ((other = findradius(other, src_org, effect->range)) != NULL) {
		if (!other->takedamage)
			continue;

		// Don't check for solids
		if (!(effect->dmgflags & 0x01))
			if (!Efx_CanDamage (other, owner, src_org))
				continue;

		// FOV effect - only apply effects to ents within the FOV (specified by count)
		if (effect->dmgflags & 0x02) {
			vectoangles(plane, v);
			if (!InFOV(src_org, v, other->s.origin, effect->count, false))
				continue;
		}

		for (j=0 ; j<3 ; j++)
			eorg[j] = (other->s.origin[j] + (other->mins[j] + other->maxs[j])*0.5);

		VectorSubtract (eorg, src_org, d2);
		VectorNormalize(d2);
		apply_effects(effect->damage, other, owner, attacker, src_org, eorg, d2, plane, effect->range, damage, amm);
	}
}

void LoopEffect_think(edict_t *ent) {
effect_t *eff;
char thisEffect[255];
int cnt;

	cnt = sub_count(ent->effect->damage);
	if (!cnt) {
		G_FreeEdict(ent);
		return;
	}	

	if ((ent->effect->flags & ~EFF_SWAPORIGINS) == 2)
		sub_string(ent->effect->damage, thisEffect, random() * cnt);
	else if ((ent->effect->flags & ~EFF_SWAPORIGINS) == 2)
		DL_strcpy(thisEffect, ent->effect->damage, 255);
	else
		sub_string(ent->effect->damage, thisEffect, ent->health++ % cnt);

	eff = GetEffectByName(thisEffect);
	if (eff && (eff->type != BE_LOOP))
		apply_effects(thisEffect, ent->enemy, ent, ent->master, ent->s.origin, ent->s.origin, ent->offset, ent->target_plane, ent->dmg_radius, ent->last_dmg, ent->ammotype);

	if ((ent->count && (ent->health >= (ent->count * cnt))) ||
		(level.time >= ent->timestamp)) {
		G_FreeEdict(ent);
		return;
	}

	ent->think = LoopEffect_think;
	ent->nextthink = level.time + ent->delay;

	if (ent->s.modelindex) {
		ent->s.frame++;
		if (ent->s.frame > ent->frame_last)
			ent->s.frame = ent->frame_first;
	}
}

void Effect_Loop (edict_t *targ, edict_t *owner, edict_t *attacker, vec3_t src_org, vec3_t dir, vec3_t plane, float range, float damage, ammo_t *amm, effect_t *effect) {
edict_t *loop;
char datablock[1024];

	loop = G_Spawn();
	// backup server-only vars
	memcpy(datablock, &loop->linkcount, (&loop->movetype - &loop->linkcount));
	// copy edict information
	memcpy(loop, owner, ((char *)(&owner->angle) - (char *)(owner)));
	// restore server-only vars
	memcpy(&loop->linkcount, datablock, (&loop->movetype - &loop->linkcount));
	G_InitEdict(loop);

	VectorCopy(src_org, loop->s.origin);
	loop->classname = "effect_loop";
	loop->s.sound = 0;
	loop->touch = NULL;
	loop->think = NULL;
	loop->health = 0;
	loop->delay = effect->delay ? effect->delay : 0.1;
	loop->timestamp = effect->duration ? level.time + effect->duration : 0.0;
	if (!loop->timestamp)
		loop->count = effect->count ? effect->count : 1;
	else
		loop->count = 0;

	loop->master = attacker;
	loop->enemy = targ;
	if (dir) VectorCopy(dir, loop->offset);
	else VectorClear(loop->offset);
	if (plane) VectorCopy(plane, loop->target_plane);
	else VectorClear(loop->target_plane);
	loop->dmg_radius = range;
	loop->last_dmg = damage;
	loop->ammotype = amm;
	loop->effect = effect;

	if (effect->model) {
		VectorClear(loop->s.angles);
		VectorClear(loop->avelocity);
		loop->s.renderfx = effect->dmgflags;
		loop->s.effects = effect->health;
		loop->s.modelindex = gi.modelindex(effect->model);
		loop->s.skinnum = effect->skinnum;
		loop->s.frame = effect->frame;
		loop->frame_first = effect->frame;
		loop->frame_last = effect->frame + effect->num_frames;
	}
	if (effect->sound)
		loop->s.sound = gi.soundindex(effect->sound);

	LoopEffect_think(loop);
	gi.linkentity(loop);
}

void SmokeEffect_Think(edict_t *ent) {
edict_t	*other = NULL;
vec3_t	dir;
double	len = 0.0;
trace_t	tr;

	if (ent->timestamp < level.time) {
		G_FreeEdict(ent);
		return;
	}

	while ((other = findradius(other, ent->s.origin, ent->dmg_radius)) != NULL) {
		if (!other->client || (other->supply && (other->supply->type != SUP_VIDSENSOR)))
			continue;

		// IR Video sensors are immune to smoke
		if (other->supply && (other->supply->flags & SF_INFRARED))
			continue;

		if (!gi.inPVS(other->s.origin, ent->s.origin))
			continue;

		VectorSubtract(other->s.origin, ent->s.origin, dir);
		if (ent->effect->damage) {
			if (evaluate(ent->effect->damage, &len, 1.0, VectorLength(dir), ent->dmg_radius) != 0) {
				gi.dprintf("Effect: %s: Invalid Expression (damage = %s)\n", ent->effect->name, ent->effect->damage);
				G_FreeEdict(ent);
				return;
			}
		}

		// reduce effect if something is directly blocking player from smoke.
		tr = gi.trace(ent->s.origin, NULL, NULL, other->s.origin, NULL, MASK_SOLID|MASK_WATER);
		if (tr.fraction != 1.0)
			len /= 2.0;

		if (len <= 0.0)
			len = 0.01;

		if (other->client) {
			VectorAdd(other->client->tint, ent->effect->spread, other->client->tint);
			VectorScale(other->client->tint, 0.5, other->client->tint);
			other->client->tint_alpha += len;
			other->client->tint_alpha /= 2.0;
			other->client->tint_fade = 0.2;
			other->in_smoke = true;
		} else
			other->flash_time = level.time + 0.2;
	}

	ent->s.frame++;
	if (ent->s.frame > ent->frame_last)
		ent->s.frame = ent->frame_first;

	ent->nextthink = level.time + FRAMETIME;
}

void Effect_Smoke (vec3_t origin, effect_t *effect) {
edict_t *smoke, *other = NULL;
vec3_t	dist;
float	len;
int		i = 0;

	while ((other = G_Find(other, FOFS(classname), "effect_smoke")) != NULL) {
		// only combine same-type smoke
		if (other->effect != effect)
			continue;

		VectorSubtract(origin, other->s.origin, dist);
		if ((len = VectorLength(dist)) < (effect->range / 2.0)) {
			VectorScale(dist, 0.5, dist);
			VectorAdd(other->s.origin, dist, other->s.origin);
			other->dmg_radius += len;
			other->timestamp += (effect->duration / 2.0);
			i++;
		}
	}
	if (i)	// Did the new smoke merge with an existing?
		return;

	smoke = G_Spawn();
	VectorCopy(origin, smoke->s.origin);
	smoke->classname = "effect_smoke";
	smoke->movetype = MOVETYPE_TOSS;
	smoke->solid = SOLID_BBOX;
	smoke->s.renderfx = RF_TRANSLUCENT;
	smoke->s.modelindex = gi.modelindex(effect->model);
	smoke->s.skinnum = effect->skinnum;
	smoke->s.frame = effect->frame;
	smoke->frame_first = effect->frame;
	smoke->frame_last = effect->frame + effect->num_frames;
//	flare->s.sound = gi.soundindex(effect->sound);
	smoke->effect = effect;
	smoke->timestamp = level.time + effect->duration;
	smoke->dmg_radius = effect->range;
	smoke->flags = effect->flags & ~EFF_SWAPORIGINS;
	smoke->think = SmokeEffect_Think;
	smoke->nextthink = level.time + FRAMETIME;

	gi.linkentity(smoke);
}

void PersistantEffect_Think (edict_t *self) {
vec3_t	blood_pos;
double	dmg = 0.0;

	if ((self->timestamp < level.time) || !self->enemy || !self->enemy->client || (self->enemy->takedamage != DAMAGE_AIM) || self->enemy->deadflag) {
		G_FreeEdict(self);
		return;
	}
/*
	if (self->effect->damage) {
		if (evaluate(self->effect->damage, &dmg, self->last_dmg, 0, 0)) {
			gi.dprintf("Effect: %s: Invalid Persistant Expression (damage = %s)\n", self->effect->name, self->effect->damage);
			return;
		}
	}
*/
	dmg = self->last_dmg;

	if (dmg < 1.0) {
		G_FreeEdict(self);
		return;
	}

	self->last_dmg = dmg;
	VectorAdd(self->enemy->s.origin, self->offset, blood_pos);
	T_Damage (self->enemy, self->owner, self->master, self->movedir, blood_pos, self->target_plane, (int)(dmg+0.5), 0, self->effect->dmgflags, 0, self->effect->sound);

	self->think = PersistantEffect_Think;
	self->nextthink = level.time + self->delay;
}

void Effect_Persistant (edict_t *targ, edict_t *owner, edict_t *attacker, vec3_t origin, vec3_t dir, vec3_t plane, float damage, ammo_t *amm, effect_t *effect) {
edict_t	*pers = NULL;

	if (!targ->client || (targ->takedamage != DAMAGE_AIM) || targ->deadflag)
		return;

	while (pers = G_Find(pers, FOFS(classname), "effect_persistant"))
		if ((pers->enemy == targ) && (pers->effect == effect))
			break;

	if (!pers) {
		pers = G_Spawn();
		pers->classname = "effect_persistant";
		pers->effect = effect;
		pers->last_dmg = damage;
		pers->delay = effect->delay;
	} else {
		if (pers->pain_debounce_time > level.time)
			return;

		pers->last_dmg += damage;
		pers->last_dmg /= 2;
		pers->delay /= 2;
	}
	pers->pain_debounce_time = level.time + pers->delay;
	pers->timestamp = level.time + effect->duration;
	pers->master = attacker;	// the player who is resposible for this effect (for kill tracking)
	pers->owner = owner;		// the object which applied the effect (player or other (like rockets)).
	pers->enemy = targ;			// the object which takes the damage given by the effect
	pers->ammotype = amm;
	pers->svflags = SVF_NOCLIENT;


	VectorSubtract(origin, targ->s.origin, pers->offset);
	VectorCopy(plane, pers->target_plane);
	VectorCopy(dir, pers->movedir);

	if (effect->flags & 0x04) {
		pers->think = PersistantEffect_Think;
		if (pers->nextthink < level.time)
			pers->nextthink = level.time + pers->delay;
	} else
		PersistantEffect_Think(pers);
}


// Gravity effects
#define GRAVCONSTANT	150

void GravityField_think (edict_t *ent)
{
	edict_t *other = NULL;
	vec3_t dir;
	int len, pull;

	while ((other = findradius(other, ent->s.origin, ent->length)) != NULL)
	{
		if (!(ent->flags & 0x01) && !other->client)
			continue;
		if (other->movetype == MOVETYPE_NONE || 
			other->movetype == MOVETYPE_PUSH ||
			other->movetype == MOVETYPE_NOCLIP ||
			other->movetype == MOVETYPE_STOP)
			continue;
		
		if (other == ent || other->item)
			continue;
		
		if (!visible(ent, other))
			continue;

		if (ent->flags & 0x08) {
			CastTrail (TE_BFG_LASER, ent, NULL, ent->s.origin, other->s.origin, vec3_origin, 0);
			gi.multicast(ent->s.origin, MULTICAST_PVS);
		}

		VectorSubtract (ent->s.origin, other->s.origin, dir);
		len = VectorNormalize(dir);
		pull = (GRAVCONSTANT*ent->length)/(len^2);
		VectorScale (dir, pull, dir);

		VectorAdd(dir, other->velocity, other->velocity);
		// if client, don't shake back and forth violently
		if (VectorLength(other->velocity)*FRAMETIME > len && other->client)
		{
			VectorNormalize(other->velocity);
			VectorScale(other->velocity, len/FRAMETIME, other->velocity);
		}
		other->nofriction_time = level.time + FRAMETIME;
	}

	if (ent->timestamp < level.time) {
		G_FreeEdict (ent);
		return;
	}
	// cycle through frames if needed
	if (ent->effect->num_frames)
		ent->s.frame = ++ent->s.frame % ent->effect->num_frames;

	ent->nextthink = level.time + FRAMETIME;
}

void Effect_Gravity (edict_t *owner, vec3_t origin, float duration, effect_t *effect) {
edict_t	*field;
double	dur = 0.0;

	if (!effect)
		return;

	if (effect->damage) {
		if (evaluate(effect->damage, &dur, duration, 1.0, 1.0) != 0) {
			gi.dprintf("Effect: %s: Invalid Expression (damage = %s)\n", effect->name, effect->damage);
			return;
		}
	}

	field = G_Spawn();
	VectorCopy(origin, field->s.origin);
	field->classname	= "effect_gravity";
	field->master		= owner->master;
	field->flags		= effect->flags & ~EFF_SWAPORIGINS;
	field->length		= effect->range;
	field->effect		= effect;
	if (dur > effect->duration) field->timestamp = level.time + dur;
	else field->timestamp = level.time + effect->duration;
	field->nextthink	= level.time + FRAMETIME;
	field->think		= GravityField_think;

	if (effect->model) {
		field->s.modelindex = gi.modelindex(effect->model);
		field->s.skinnum = effect->skinnum;
		field->s.frame = effect->frame;
	}
	gi.linkentity(field);
}

// Shockwave effect
void ShockLaser_think (edict_t *ent) {
	VectorCopy (ent->move_origin, ent->s.old_origin);
	ent->nextthink = level.time + FRAMETIME;
}


void Shockwave_think (edict_t *ent) {
vec3_t		point, oldpoint, pos;
int			i, num = ent->length;
trace_t		tr;
float		d, distance;
edict_t		*ignore = NULL, *cur = NULL, *laser;
qboolean	keepgoing = false;
double		dmg = 0.0;
effect_t	*effect;

	d = level.time - ent->timestamp;
	VectorMA(ent->s.origin, d*ent->speed, ent->movedir, oldpoint);
	for (i = 0, cur = ent->chain; i < num; i++, cur = cur->chain)
	{
		ignore = NULL;
		VectorSubtract (oldpoint, ent->s.origin, pos);
		RotatePointAroundVector(point, ent->move_angles, pos, 360/num);
		VectorAdd (point, ent->s.origin, point);

		// once we are in a wall, don't draw lasers from us
		if (gi.pointcontents(oldpoint) & MASK_SOLID)
			cur->show_hostile = true;
		
		if (!cur->show_hostile)
			keepgoing = true;
		else
			cur->svflags |= SVF_NOCLIENT;

		tr = gi.trace (oldpoint, NULL, NULL, point, NULL, MASK_SHOT);
		// only perform if point is not invalid
		if (!cur->show_hostile)
		{
			VectorCopy(oldpoint, cur->s.origin);
			VectorCopy(tr.endpos, cur->s.old_origin);

			if (tr.ent->takedamage)
			{
				effect = GetEffectByName(ent->message);
				if (effect) {
					VectorSubtract(tr.endpos, ent->s.origin, pos);
					distance = VectorLength(pos);

					if (effect->damage) {
						if (evaluate(effect->damage, &dmg, (float)ent->dmg, distance, (float)ent->dmg) != 0) {
							gi.dprintf("Effect: %s: Invalid Expression (damage = %s)\n", effect->name, effect->damage);
							return;
						}
					}
					T_Damage (tr.ent, ent, ent->master, vec3_origin, tr.endpos, tr.plane.normal, (int)dmg, (int)dmg*10, DAMAGE_BLAST, 0, NULL);
				}
				ignore = tr.ent;
			}
		}

		// if blocked, start fom other point
		if (tr.fraction < 1.0 && !cur->chain->show_hostile)
		{
			tr = gi.trace (point, NULL, NULL, oldpoint, ignore, MASK_SHOT);
			laser = G_Spawn();
			memcpy(laser, cur, sizeof(laser));
			VectorCopy (point, laser->s.origin);
			VectorCopy (tr.endpos, laser->s.old_origin);
			laser->think = G_FreeEdict;

			if (tr.ent->takedamage) {
				effect = GetEffectByName(ent->message);
				if (effect) {
					VectorSubtract(tr.endpos, ent->s.origin, pos);
					distance = VectorLength(pos);
					if (effect->damage) {
						if (evaluate(effect->damage, &dmg, (float)ent->dmg, distance, (float)ent->dmg) != 0) {
							gi.dprintf("Effect: %s: Invalid Expression (damage = %s)\n", effect->name, effect->damage);
							return;
						}
					}
					T_Damage (tr.ent, ent, ent->master, vec3_origin, tr.endpos, tr.plane.normal, (int)dmg, (int)dmg*10, DAMAGE_BLAST, 0, NULL);
				}
			}
		}
		VectorCopy (point, oldpoint);
	}

	ent->nextthink = level.time + FRAMETIME;
	if (!keepgoing || ent->timestamp + ent->wait < level.time)
	{
		for (cur = ent->chain; cur < cur->chain; cur = cur->chain)
			G_FreeEdict (cur);
		G_FreeEdict (ent);
	}
}

void Effect_Shockwave (edict_t *owner, vec3_t origin, vec3_t normal, float damage, effect_t *effect) {
edict_t	*center, *laser, *prev = NULL, *first;
vec3_t	dir;
int		i, num;

	if (!effect)
		return;

	VectorNormalize(normal);
	PerpendicularVector(dir, normal);

	num = effect->count;
	if (num > 32) num = 32;
	
	center				= G_Spawn();
	center->classname	= "effect_shockwave";
	center->timestamp	= level.time;
	center->master		= owner;
	center->solid		= SOLID_NOT;
	center->movetype	= MOVETYPE_NONE;

	VectorCopy(origin, center->s.origin);
	VectorCopy(dir, center->movedir);
	VectorCopy(normal, center->move_angles);

	center->speed		= effect->delay;
	center->wait		= effect->duration;
	center->flags		= effect->flags & ~EFF_SWAPORIGINS;
	center->dmg			= damage;
	center->message		= effect->name;
	center->length		= num;

	gi.linkentity (center);

	center->think		= Shockwave_think;
	center->nextthink	= level.time + FRAMETIME;
	
	prev = center;
	// spawn laser ents
	for (i = 0; i < num; i++)
	{
		laser				= G_Spawn();
		laser->classname	= "shockwave laser";
		laser->movetype		= MOVETYPE_NONE;
		laser->solid		= SOLID_NOT;
		laser->s.renderfx	= RF_BEAM;
		laser->s.modelindex	= 1;
		laser->s.frame		= effect->num_frames;
		laser->s.skinnum	= effect->skinnum;
		prev->chain			= laser;
		laser->teammaster	= prev;
		VectorCopy(origin, laser->s.origin);
		VectorCopy(origin, laser->s.old_origin);
		gi.linkentity (laser);

//		laser->think		= ShockLaser_think;
//		laser->nextthink	= level.time + FRAMETIME;

		prev = laser;
		if (i == 0) first = laser;
	}
	laser->chain = first;
	first->teammaster = laser;
}

#define MAX_SHRAPNEL	24

void Effect_Shrapnel (edict_t *owner, edict_t *attacker, vec3_t origin, vec3_t normal, float damage, effect_t *effect) {
vec3_t	dir, angles, end, dist;
double	pel_dmg = 0.0;
float	max_dmg, distance;
trace_t	tr;
int		i, pel_cnt;

	if (!effect)
		return;

	max_dmg = 45.0;
	pel_cnt = (int)damage / 5;

	for (i=0; i < pel_cnt; i++) {
		if (normal) {
			VectorCopy(normal, dir);
			vectoangles(dir, angles);
			angles[YAW] = anglemod(angles[YAW] + (crandom() * 120));
			angles[PITCH] = anglemod(angles[PITCH] + (crandom() * 120));
		} else {
			VectorClear(dir);
			vectoangles(dir, angles);
			angles[YAW] = anglemod(angles[YAW] + (crandom() * 360));
			angles[PITCH] = anglemod(angles[PITCH] + (crandom() * 360));
		}
		AngleVectors(angles, dir, NULL, NULL);
		VectorMA(origin, damage*2, dir, end);

		tr = gi.trace(origin, NULL, NULL, end, NULL, MASK_SHOT);
		if (tr.ent->takedamage) {
			if (!tr.startsolid) {
				VectorSubtract(tr.endpos, origin, dist);
				distance = VectorLength(dist);
			} else distance = 0.1;
			if (effect->damage) {
				if (evaluate(effect->damage, &pel_dmg, max_dmg, distance, damage*2) != 0) {
					gi.dprintf("Effect: %s: Invalid Expression (damage = %s)\n", effect->name, effect->damage);
					return;
				}
			}
			T_Damage (tr.ent, owner, attacker, dir, tr.endpos, tr.plane.normal, (int)pel_dmg, (int)pel_dmg*3, DAMAGE_PIERCE, 0, NULL);
		}

		if (i < MAX_SHRAPNEL)
			if (effect->flags & 0x01)
				MCastTempEnt(TE_BUBBLETRAIL, owner, NULL, origin, tr.endpos, NULL, 0, 0, 0, MULTICAST_PVS);
	}
}

void Effect_MIRV (edict_t *owner, edict_t *attacker, vec3_t origin, vec3_t normal, effect_t *effect) {
vec3_t		dir, angles;
float		range, damage;
ammo_t		*amm;
weap_t		*weap;
effect_t	*eff;
edict_t		*proj;
int			i, j;
char		curefx[32];

	if (!effect)
		return;


	amm = GetAmmoByName(effect->effect);
	if (amm) {
		i = sub_count(amm->effect);
		for (j = 0; j < i; j++) {
			eff = GetEffectByName(sub_string(amm->effect, curefx, j));
			if (eff) {
				if (eff->type == BE_MIRV) {
					gi.dprintf("Effect: %s: Redundant effect type definitions for MIRV\n", effect->name);
					return;
				}
			} else {
				gi.dprintf("Ammo: %s: References unknown effect (%s)\n", amm->name, curefx);
				return;
			}
		}
	} else {
		gi.dprintf("EFFECTS: %s: Field \"Special\" References an unknown ammo type: %s\n", effect->name, effect->effect);
		return;
	}

	// Do this here, before we change the owner
	// If the event fires in mid-air, use the velocity as
	// the plane to fire into.
	if (VectorLength(normal))
		VectorCopy(normal, dir);
	else
		VectorCopy(owner->velocity, dir);

	if (!DL_strcmp(owner->classname, "projectile", -1, false)) {
		if (owner->orig_owner)
			owner = owner->orig_owner;
		else
			owner = owner->owner;
	}

	if (!owner)
		return;

	for (i=0; i < effect->count; i++) {
		if (effect->flags & 0x01)
			VectorCopy(effect->offset, angles);
		else {
			vectoangles(dir, angles);
			VectorAdd(angles, effect->offset, angles);
		}
		angles[YAW] = anglemod(angles[YAW] + (crandom() * effect->spread[YAW]));
		angles[PITCH] = anglemod(angles[PITCH] + (crandom() * effect->spread[PITCH]));
		AngleVectors(angles, dir, NULL, NULL);
		range = amm->range;
		damage = amm->damage;

		if (amm->ftype == FT_TOSS)
			range /= 2.0;

		if (owner->client && owner->client->curweap)
			weap = (weap_t *)owner->client->curweap->item;
		else
			weap = owner->weaptype;

		if (amm->ftype == FT_PELLET) {
			if (owner->owner == attacker) owner->owner = NULL;
			fire_pellet(owner, attacker, origin, dir, range, damage, amm);
		} else {
			proj = fire_projectile(owner, attacker, origin, dir, range, damage, amm, weap);
			proj->nextthink += (crandom() * (amm->duration / 3.0));
		}
	}
}


void Spinner_think(edict_t *ent) {
vec3_t	dir, end;
trace_t	tr;

	if (ent->timestamp && (ent->timestamp < level.time)) {
		G_FreeEdict(ent);
		return;
	}

	if (ent->effect->count && !ent->count) {
		G_FreeEdict(ent);
		return;
	}

	VectorCopy(ent->s.origin, end);
	end[2] -= ent->viewheight;
	tr = gi.trace(ent->s.origin, NULL, NULL, end, NULL, MASK_SOLID|MASK_WATER);
	if (tr.fraction != 1.0) {
		// since abs() accepts an INT - what kind of shit is that?
		if (ent->velocity[2] < 0.0) ent->velocity[2] /= 2.0;
		else ent->velocity[2] = (-1.0 * ent->velocity[2]) / 2.0;

		if (ent->velocity[2] < 100)
			ent->velocity[2] = 100;
		VectorCopy(ent->effect->spread, ent->avelocity);
		if (VectorLength(ent->effect->offset))
			VectorCopy(ent->effect->offset, ent->s.angles);
	}

	ent->nextthink = level.time + FRAMETIME;

	// cycle through frames if needed
	if (ent->effect->num_frames)
		ent->s.frame = ++ent->s.frame % ent->effect->num_frames;

	if (ent->delay > level.time)
		return;

	AngleVectors(ent->s.angles, dir, NULL, NULL);
	if (ent->ammotype->ftype == FT_PELLET)
		fire_pellet(ent, ent->enemy, ent->s.origin, dir, ent->ammotype->range, ent->ammotype->damage, ent->ammotype);
	else
		fire_projectile(ent, ent->enemy, ent->s.origin, dir, ent->ammotype->range, ent->ammotype->damage, ent->ammotype, ent->weaptype);

	if (ent->flags)
		MuzzleFlash2(ent, ent->flags, MULTICAST_PVS);

	ent->delay = level.time + ent->effect->delay;
	ent->count--;	// don't bother checking for ent->effect->count
}

void Effect_Spinner(edict_t *owner, edict_t *attacker, vec3_t src_org, vec3_t plane, float duration, effect_t *effect) {
vec3_t	pos, dir, forward, right, up;
edict_t *spin;
ammo_t	*amm;
trace_t	tr;
double	dur = 0.0;
int		i;

	if (owner->client) {
		gi.dprintf("EFFECT: %s: Invalid use of effect type.\n", effect->name);
		return;
	}

	amm = GetAmmoByName(effect->effect);
	if (!amm) {
		gi.dprintf("EFFECTS: %s: Field \"Special\" References an unknown ammo type: %s\n", effect->name, effect->effect);
		return;
	}

	if (effect->damage) {
		if (evaluate(effect->damage, &dur, duration, 1.0, 1.0) != 0) {
			gi.dprintf("Effect: %s: Invalid Expression (damage = %s)\n", effect->name, effect->damage);
			return;
		}
	}

	VectorCopy(src_org, pos);
	VectorCopy(owner->velocity, dir);

	spin = G_Spawn();
	VectorSet(spin->mins, -8, -8, -4);
	VectorSet(spin->maxs, 8, 8, 4);
	VectorCopy(src_org, spin->s.origin);
	VectorCopy(owner->velocity, spin->velocity);
	VectorCopy(effect->spread, spin->avelocity);

	// If an object spawns with its bbox intersecting another
	// bbox (or bsp) it will ignore it until clear.  So
	// we need to prevent that from happening - which usually
	// occurs when the owner detonates against a wall.
	if (SV_TestEntityPosition(spin)) {
		// first check to see if entity origin is inside solid
		if (gi.pointcontents(spin->s.origin) & MASK_SOLID) {
			// FIXME: Do something else - maybe throw debri, or explode?
			G_FreeEdict(spin);
			return;
		}
		vectoangles(spin->velocity, dir);
		AngleVectors(dir, forward, right, up);

		for (i=0; i<6; i++) {
			switch (i) {
			case 0:	VectorMA(spin->s.origin, 12, forward, pos); break;
			case 1: VectorMA(spin->s.origin, 12, right, pos); break;
			case 2: VectorMA(spin->s.origin, 12, up, pos); break;
			case 3:	VectorMA(spin->s.origin, -12, forward, pos); break;
			case 4: VectorMA(spin->s.origin, -12, right, pos); break;
			case 5: VectorMA(spin->s.origin, -12, up, pos); break;
			}
			tr = gi.trace(spin->s.origin, NULL, NULL, pos, NULL, MASK_SOLID);
			if (tr.fraction != 1.0) {
				ClipVelocity (spin->velocity, tr.plane.normal, spin->velocity, 2.0);
				VectorMA(tr.endpos, 12.0, tr.plane.normal, spin->s.origin);
			}
		}

	}

	spin->classname = "effect_spinner";
	spin->movetype = MOVETYPE_BOUNCE;
	spin->solid = SOLID_BBOX;
	spin->flags = effect->flags & ~EFF_SWAPORIGINS;
	spin->owner =  NULL;
	spin->supply = owner->supply;	// keep track of the item that spawned this spinner
	spin->master = owner->master;
	spin->enemy = attacker;
	spin->s.effects = owner->s.effects;

	spin->effect = effect;
	if (effect->range) spin->viewheight = effect->range;
	else spin->viewheight = 32;
	spin->ammotype = amm;
	if (dur > effect->duration) spin->timestamp = level.time + dur;
	else spin->timestamp = level.time + effect->duration;
	spin->delay = effect->delay;
	spin->count = effect->count;
	spin->team = owner->master->client->resp.team;

	if (!effect->model) {
		spin->s.modelindex = owner->s.modelindex;
		spin->s.skinnum = owner->s.skinnum;
		spin->s.frame = owner->s.frame;
	} else {
		spin->s.modelindex = gi.modelindex(effect->model);
		spin->s.skinnum = effect->skinnum;
		spin->s.frame = effect->frame;
	}

	spin->think = Spinner_think;
	spin->nextthink = level.time + effect->delay;

	gi.linkentity(spin);
}

// AffectSensor will damage, disable, or otherwise affect the behaviour of sensors - Does nothing to players
// Possible effects include timed-disable, lock on effect origin, random target locks, random-delay disable, etc
void Effect_AffectSensor(vec3_t origin, effect_t *effect) {
edict_t *other = NULL;
vec3_t	dir, end;
int		fov, type, i;
double	len = 0.0;
trace_t	tr;

	if (!effect)
		return;

	while ((other = findradius(other, origin, effect->range)) != NULL)
	{
		if (!other->inuse || !other->supply)
			continue;

		type = other->supply->type;
		if ((type != SUP_VIDSENSOR) && (type != SUP_PROXY))
			continue;	// Its NOT RELEVANT... :)

		if (!gi.inPVS(other->s.origin, origin))
			continue;

		// Determine view-point for the recieving entity
		VectorCopy(other->s.origin, end);
		end[2] += other->viewheight;

		if (type == SUP_VIDSENSOR) { 
			fov = other->supply->range;
			if (!InFOV(end, other->s.angles, origin, fov, false))
				continue;
		}

		tr = gi.trace (origin, NULL, NULL, end, NULL, MASK_OPAQUE);
		if (tr.fraction < 1.0)
			continue;

		VectorSubtract(origin, other->s.origin, dir);
		if (effect->damage) {
			if (evaluate(effect->damage, &len, 1.0, VectorLength(dir), effect->range) != 0) {
				gi.dprintf("Effect: %s: Invalid Expression (damage = %s)\n", effect->name, effect->damage);
				return;
			}
		}

		if (len <= 0.01) len = 0.01;

		if (effect->flags & AS_TIMEDDISABLE)
			other->flash_time = level.time + len;
		if (effect->flags & AS_RANDOMDISABLE)
			other->flash_time = level.time + (random() * len);
		if (effect->flags & AS_TARGETORIGIN) {
			VectorCopy(origin, other->pos3);
			if (other->target_ent && other->target_ent->building && other->target_ent->use)
				other->target_ent->use(other->target_ent, other, NULL);
			other->flash_time = level.time + len;
		}
		if (effect->flags & AS_TARGETRANDOM) {
			// Generate a random target point somewhere near the origin
			for (i=0; i<3; i++)
				other->pos3[i] = origin[i] + (crandom() * 64);
			if (other->target_ent && other->target_ent->building && other->target_ent->use)
				other->target_ent->use(other->target_ent, other, NULL);
			other->flash_time = level.time + len;
		}
		if (effect->flags & AS_DESTROY)
			G_FreeEdict(other);
	}
}


//  Drone - If anchored to another object, it will not fire, and will attempt to intercept incoming rounds
//  If anchored to the world, will attack nearby targets, and will attempt to dodge incoming rounds

void Drone_pain(edict_t *self, edict_t *other, float kick, int damage) {
int i;

	for (i=0; i<3; i++)
		self->velocity[i] = (crandom() * damage) * 50; // velocity 

	for (i=0; i<2; i++)
		self->orot2[i] = (crandom() * damage) * 50;  // avelocity
}

void Drone_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point){
	BecomeExplosion1 (self);
}

void Drone_Think (edict_t *self) {
edict_t *other = &g_edicts[0];
float dist, targ_range = 9999.0;
vec3_t	v, vel, ang;
qboolean inView;
trace_t tr;
int i, ignore;

	if (level.time > self->timestamp) {
		BecomeExplosion1(self);
		return;
	}

	VectorCopy(self->velocity, self->offset);
	// VectorCopy(self->avelocity, self->orot2);

	self->enemy = NULL;
	self->s.renderfx = RF_IR_VISIBLE;
	// Scan for targets - priority is closest
	while ((other = findradius(other, self->s.origin, self->effect->range)) != NULL) {
		// Ignore all edicts by clients and projectiles
		if (!other->client && DL_strcmp(other->classname, "projectile", -1, false))
			continue;
		// Ignore children of this drone (so they don't shot at thier own missiles)
		if ((other->master == self) || (other->owner == self))
			continue;
		// Ignore teammates
		//if (other->team && (other->team == self->team))
		//	continue;
		// Ignore non-threatening objects (ie, objects moving away from the drone)
		if (other->client) {
			ignore = !InFOV(other->s.origin, other->client->ps.viewangles, self->s.origin, other->client->ps.fov, false);
		} else {
			if (VectorLength(other->velocity) > 100) vectoangles(other->velocity, v);
			else VectorCopy(other->s.angles, v);
			ignore = !InFOV(other->s.origin, v, self->s.origin, 15, false);
		}
		if (ignore)
			continue;
		VectorSubtract(other->s.origin, self->s.origin, v);
		dist = VectorLength(v);
		if (dist < targ_range) {
			targ_range = dist;
			self->enemy = other;
		}
	}
		
	if (self->enemy && CanDamage(self->enemy, self)) {
		self->s.renderfx = RF_GLOW|RF_IR_VISIBLE;

		if (self->enemy->client) {
			VectorSubtract(self->enemy->client->ps.viewangles, self->move_angles, ang);
			VectorCopy(self->enemy->client->ps.viewangles, self->move_angles);
		} else {
			VectorSubtract(self->enemy->s.angles, self->move_angles, ang);
			VectorCopy(self->enemy->s.angles, self->move_angles);
		}
		if ((VectorLength(self->enemy->velocity) > 32.0) || (VectorLength(ang) > 1.0)) {

			inView = InFOV(self->enemy->s.origin, self->move_angles, self->s.origin, 15, false);
			VectorSubtract(self->s.origin, self->pos1, v);

			if ((VectorLength(v) < 5.0) && inView) {
				for (i=0; i<3; i++)
					self->pos1[i] = self->pos2[i] + (crandom() * 128.0);
				tr = gi.trace(self->s.origin, self->mins, self->maxs, self->pos1, self, MASK_SHOT);
				VectorCopy(tr.endpos, self->pos1);
			}
			self->oldenemy_time = level.time + 0.5;
		}

		
		if (self->ammotype && (level.time < self->oldenemy_time) && (level.time > self->flash_time) &&
			InFOV(self->s.origin, self->s.angles, self->enemy->s.origin, 2, false) && (random() > 0.66) ) {

			AngleVectors(self->s.angles, v, NULL, NULL);
			if (self->ammotype->ftype == FT_PELLET)
				fire_pellet(self, self->master, self->s.origin, v, self->ammotype->range, self->ammotype->damage, self->ammotype);
			else
				fire_projectile(self, self->master, self->s.origin, v, self->ammotype->range, self->ammotype->damage, self->ammotype, self->weaptype);
				
			if (self->effect->flags & ~EFF_SWAPORIGINS)
				MuzzleFlash2(self, self->effect->flags & ~EFF_SWAPORIGINS, MULTICAST_PVS);

			self->flash_time = level.time + self->effect->delay;
		}
		// Always face targets
		// Calculate view angles from the target destination, so it tracks faster
		VectorSubtract(self->enemy->s.origin, self->pos1, ang);
		vectoangles(ang, self->orot1);
	}
//	else {
//		VectorCopy(self->pos2, self->pos1);
//	}

	// Move to dest;
	VectorSubtract(self->pos1, self->s.origin, v);
	VectorScale(v, 3.0 * self->effect->speed, vel);
	VectorScale(self->offset, 0.5, self->offset);
	VectorAdd(vel, self->offset, self->velocity);

	// rotate to angles
	for (i=0; i<3; i++) {
		// clip velocities while we're here
		if (self->velocity[i] > 1000) self->velocity[i] = 1000;
		else if (self->velocity[i] < -1000) self->velocity[i] = -1000;
		v[i] = -1 * (anglemod(self->s.angles[i]) - anglemod(self->orot1[i]));
	}
	VectorScale(v, 5.0 * self->effect->speed, ang);
	VectorScale(self->orot2, 0.5, self->orot2);
	VectorAdd(ang, self->orot2, self->avelocity);

	self->nextthink = level.time + FRAMETIME;
	gi.linkentity(self);
}

void Effect_Drone (edict_t *targ, edict_t *owner, edict_t *attacker, vec3_t origin, vec3_t dir, vec3_t plane, float range, float damage, ammo_t *amm, effect_t *effect) {
edict_t *drone;
vec3_t	pos, forward, right, up;
trace_t tr;
int i;

	drone = G_Spawn();

	// HomePos is 48 "out" from its origin (out is the plane normal)
	VectorCopy(origin, drone->s.origin);
	VectorSet(drone->mins, -16, -16, -16);
	VectorSet(drone->maxs, 16, 16, 16);

	// If an object spawns with its bbox intersecting another
	// bbox (or bsp) it will ignore it until clear.  So
	// we need to prevent that from happening - which usually
	// occurs when the owner detonates against a wall.
	if (SV_TestEntityPosition(drone)) {
		// first check to see if entity origin is inside solid
		if (gi.pointcontents(drone->s.origin) & MASK_SOLID) {
			// FIXME: Do something else - maybe throw debri, or explode?
			G_FreeEdict(drone);
			return;
		}
		AngleVectors(drone->s.angles, forward, right, up);

		for (i=0; i<6; i++) {
			switch (i) {
			case 0:	VectorMA(drone->s.origin, 20, forward, pos); break;
			case 1: VectorMA(drone->s.origin, 20, right, pos); break;
			case 2: VectorMA(drone->s.origin, 20, up, pos); break;
			case 3:	VectorMA(drone->s.origin, -20, forward, pos); break;
			case 4: VectorMA(drone->s.origin, -20, right, pos); break;
			case 5: VectorMA(drone->s.origin, -20, up, pos); break;
			}
			tr = gi.trace(drone->s.origin, NULL, NULL, pos, NULL, MASK_SOLID);
			if (tr.fraction != 1.0) {
				VectorMA(tr.endpos, 32.0, tr.plane.normal, drone->s.origin);
				if (plane) {
					VectorAdd(plane, tr.plane.normal, plane);
					VectorNormalize(plane);
				}
			}
		}

	}

	drone->classname = "effect_drone";
	drone->effect = effect;
//	drone->s.renderfx |= RF_GLOW;
	drone->timestamp = level.time + effect->duration;
	drone->master = attacker;	// the player who is resposible for this effect (for kill tracking)
	drone->owner = owner;		// the object which applied the effect (player or other (like rockets)).
	drone->enemy = targ;
	drone->ammotype = amm;
	drone->movetype = MOVETYPE_FLYRICOCHET;
	drone->solid = SOLID_BBOX;

	drone->vid_decoy = true;

	if (attacker) {
		if (attacker->client)
			drone->team = attacker->client->resp.team;
		else 
			drone->team = attacker->team;
	}

	VectorMA(drone->s.origin, 48, plane, drone->pos2);
	VectorCopy(drone->pos2, drone->pos1);

	drone->ammotype = GetAmmoByName(effect->effect);

	if (!effect->model) {
		drone->s.modelindex = owner->s.modelindex;
		drone->s.skinnum = owner->s.skinnum;
		drone->s.frame = owner->s.frame;
	} else {
		drone->s.modelindex = gi.modelindex(effect->model);
		drone->s.skinnum = effect->skinnum;
		drone->s.frame = effect->frame;
	}

	if (effect->health > 0) {
		drone->health = effect->health;
		drone->takedamage = DAMAGE_AIM;
		drone->pain = Drone_pain;
		drone->die = Drone_die;
	}
	drone->think = Drone_Think;
	drone->nextthink = level.time + FRAMETIME;
	gi.linkentity(drone);
}


void apply_effects(char *effect, edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t src_org, vec3_t dest_org, vec3_t dir, vec3_t plane, float range, float damage, ammo_t *amm) {
char dup[256], *t, *e[8];
effect_t *efx;
vec3_t src_o, dest_o;
int i = 0;

	if (!effect || !*effect)
		return;

	// parse effect string for multiple effects
	memset(dup, 0, 255);
	DL_strcpy(dup, effect, 255);
	e[i++] = t = dup;
	while (((t=strstr(t, "|")) != NULL) && (i < 8)) {
		*t++=0;
		e[i++] = t;
	}
	
	while (--i >= 0) {
		if (!e || !*e)
			continue;

		efx = GetEffectByName(e[i]);
		if (!efx) {
			gi.dprintf("Effect: %s: Unknown Effect\n", e[i]);
			return;
		}

		if (efx->flags & EFF_SWAPORIGINS) {
			VectorCopy(src_org, dest_o);
			VectorCopy(dest_org, src_o);
		} else {
			VectorCopy(src_org, src_o);
			VectorCopy(dest_org, dest_o);
		}

		switch (efx->type) {
		case BE_MODEL:			Effect_Model (src_o, efx); break;
		case BE_SOUND:			Effect_Sound (src_o, efx); break;
		case BE_MUZZLEFLASH:	Effect_MuzzleFlash (inflictor, src_o, efx); break;
		case BE_TEMPENTITY:		Effect_TempEntity (targ, inflictor, attacker, src_o, dir, plane, range, damage, efx); break;

		case BE_TINT:			Effect_Tint(targ, inflictor, src_o, dest_o, range, damage, efx); break;
		case BE_BLIND:			Effect_Blind(targ, inflictor, src_o, dest_o, range, damage, efx); break;
		case BE_CONFUSION:		Effect_Confusion(targ, inflictor, src_o, dest_o, range, damage, efx); break;
		case BE_PUSH:			Effect_Pusher(targ, inflictor, attacker, src_o, dest_o, dir, range, damage, efx); break;
		case BE_DAMAGE:			Effect_Damage(targ, inflictor, attacker, src_o, dest_o, dir, plane, range, damage, amm, efx); break;

		case BE_STATEDMG:
		case BE_STATEPROT:
		case BE_STATEGRAV:
		case BE_STATESWIM:		Effect_ClientState(targ, src_o, dest_o, range, damage, efx); break;

		case BE_RADIUS:			Effect_Radius(inflictor, attacker, src_o, dir, plane, range, damage, amm, efx); break;
		case BE_LOOP:			Effect_Loop(targ, inflictor, attacker, src_o, dir, plane, range, damage, amm, efx); break;

		case BE_GRAVITY:		Effect_Gravity (inflictor, src_o, damage, efx); break;
		case BE_SMOKE:			Effect_Smoke(src_o, efx); break;
		case BE_SHRAPNEL:		Effect_Shrapnel (inflictor, attacker, src_o, plane, damage, efx); break;

		case BE_MIRV:			Effect_MIRV (inflictor, attacker, src_o, plane, efx); break;
		case BE_SPINNER:		Effect_Spinner (inflictor, attacker, src_o, plane, damage, efx); break;
		case BE_DRONE:			Effect_Drone(targ, inflictor, attacker, src_o, dir, plane, range, damage, amm, efx); break;

		case BE_AFFECTSENSOR:	Effect_AffectSensor (src_o, efx); break;
		default:			gi.dprintf("Effect: %s: Unknown effect type (%d)\n", e[i], efx->type);
		}
	}
}