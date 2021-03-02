#include "g_local.h"
#include "dl_player.h"

/*
typedef enum {
	F_INT, 
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_EDICT,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory
	F_IGNORE
} fieldtype_t;
*/

field_t dl_weapflds[] = {
	{"name", WOFS(name), F_LSTRING},
	{"ammo_type", WOFS(base_ammo), F_LSTRING},
	{"damage_mod", WOFS(damage_mod), F_FLOAT},
	{"range_mod", WOFS(range_mod), F_FLOAT},
	{"accuracy", WOFS(accuracy), F_FLOAT},
	{"reliability", WOFS(reliability), F_FLOAT},
	{"weight", WOFS(weight), F_FLOAT},
	{"size", WOFS(size), F_FLOAT},
	{"rnds_chambered", WOFS(rnds_chambered), F_INT},
	{"clip_size", WOFS(clip_size), F_INT},
	{"use_count", WOFS(use_count), F_INT},
	{"reload_time", WOFS(reload_time), F_FLOAT},
	{"cyclic_rate", WOFS(cyclic_rate), F_FLOAT},
	{"recoil", WOFS(recoil), F_INT},
	{"flags", WOFS(flags), F_INT},
	{"burst_count", WOFS(burst_cnt), F_INT},
	{"viewer_model", WOFS(view_model), F_LSTRING},
	{"frame_raise", WOFS(frame_raise), F_INT},
	{"raise_frames", WOFS(frame_raise_num), F_INT},
	{"frame_fire", WOFS(frame_fire), F_INT},
	{"fire_frames", WOFS(frame_fire_num), F_INT},
	{"frame_idle", WOFS(frame_idle), F_INT},
	{"idle_frames", WOFS(frame_idle_num), F_INT},
	{"frame_drop", WOFS(frame_drop), F_INT},
	{"drop_frames", WOFS(frame_drop_num), F_INT},
	{"frame_reload", WOFS(frame_reload), F_INT},
	{"reload_frames", WOFS(frame_reload_num), F_INT},
	{"muzzle_flash", WOFS(s_load_index), F_INT},
	{"raise_sound", WOFS(snd_raise), F_LSTRING},
	{"fire_sound", WOFS(snd_fire), F_LSTRING},
	{"idle_sound", WOFS(snd_idle), F_LSTRING},
	{"drop_sound", WOFS(snd_drop), F_LSTRING},
	{"reload_sound", WOFS(snd_reload), F_LSTRING},
	{"fire_frame", WOFS(fire_frame), F_INT},
	{"fire_offset", WOFS(fire_offset), F_VECTOR},
	{"reload_pause_frame", WOFS(reload_pause_frame), F_INT},
	{"inventory_pic", WOFS(pic_s), F_LSTRING},
	{"statusbar_pic", WOFS(pic_b), F_LSTRING},
	{"category", WOFS(category), F_LSTRING},

	// VWEAP animation styles
	{"vweap_model", WOFS(player_model), F_LSTRING},
	{"vweap_idle", WOFS(ppmIdle_stand), F_LSTRING},
	{"vweap_run", WOFS(ppmRun_stand), F_LSTRING},
	{"vweap_attack", WOFS(ppmAttack_stand), F_LSTRING},
	{"vweap_pain", WOFS(ppmPain_stand), F_LSTRING},
	{"vweap_die", WOFS(ppmDie_stand), F_LSTRING},
	{"vweap_idle_c", WOFS(ppmIdle_crouch), F_LSTRING},
	{"vweap_run_c", WOFS(ppmRun_crouch), F_LSTRING},
	{"vweap_attack_c", WOFS(ppmAttack_crouch), F_LSTRING},
	{"vweap_pain_c", WOFS(ppmPain_crouch), F_LSTRING},
	{"vweap_die_c", WOFS(ppmDie_crouch), F_LSTRING},
	{"vweap_jump", WOFS(ppmJump), F_LSTRING},
	{NULL, 0, F_INT}
};

field_t dl_ammoflds[] = {
	{"name", AOFS(name), F_LSTRING},
	{"base_type", AOFS(base_name), F_LSTRING},
	{"type", AOFS(ftype), F_INT},
	{"weight", AOFS(weight), F_FLOAT},
	{"range", AOFS(range), F_FLOAT},
	{"damage", AOFS(damage), F_FLOAT},
	{"pellet_count", AOFS(pellet_count), F_INT},
	{"hspread", AOFS(hspread), F_FLOAT},
	{"vspread", AOFS(vspread), F_FLOAT},
	{"flight_effect", AOFS(flight_effect), F_INT},
	{"trail_effect", AOFS(trail_effect), F_INT},
	{"render_effect", AOFS(render_effect), F_INT},
	{"touch_type", AOFS(ttype), F_INT},
	{"duration", AOFS(duration), F_FLOAT},
	{"model", AOFS(model), F_LSTRING},
	{"skin", AOFS(skinnum), F_INT},
	{"sound", AOFS(sound), F_LSTRING},
	{"track_speed", AOFS(track_speed), F_VECTOR},
	{"track_fov", AOFS(track_fov), F_INT},
	{"gravity", AOFS(gravity), F_FLOAT},
	{"effect", AOFS(effect), F_LSTRING},
	{"inventory_pic", AOFS(pic_s), F_LSTRING},
	{"statusbar_pic", AOFS(pic_b), F_LSTRING},
	{NULL, 0, F_INT}
};
	
ammo_t	ammo[MAX_MUNITIONS];
weap_t	weapons[MAX_WEAPONS];

ammo_t *GetAmmoByName(char *name) {
int		i;

	if (name) {
		for (i = 0; i < MAX_MUNITIONS; i++) {
			if (ammo[i].name)
				if (!DL_strcmp(ammo[i].name, name, -1, false))
					return &ammo[i];
		}
	}
	return NULL;
}

weap_t	*GetWeaponByName(char *name) {
int		i;

	if (name) {
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i].name)
				if (!DL_strcmp(weapons[i].name, name, -1, false))
					return &weapons[i];
		}
	}
	return NULL;
}

effect_t *GetEffectByName(char *name) {
int		i;

	if (name) {
		for (i = 0; i < MAX_EFFECTS; i++) {
			if (effects[i].name)
				if (!DL_strcmp(effects[i].name, name, -1, false))
					return &effects[i];
		}
	}
	return NULL;
}

ammo_t *GetCompatibleAmmo(char *name) {
int		i;

	if (name) {
		for (i = 0; i < MAX_MUNITIONS; i++) {
			if (ammo[i].base_name)
				if (!DL_strcmp(ammo[i].base_name, name, -1, false))
					return &ammo[i];
		}
	}
	return NULL;
}
		
void fire_pellet (edict_t *self, edict_t *attacker, vec3_t origin, vec3_t aimdir, float range, float damage, ammo_t *amm) {
trace_t		tr;
vec3_t		start, forward, right, up;
vec3_t		dir, end, water_start;
float		r, u;
qboolean	water = false;
int			i, content_mask = MASK_SHOT | MASK_WATER;

//	if (dtype & DT_MOLECULAR)
//		content_mask = CONTENTS_PLAYERCLIP;

	if (!amm)
		return;

	if (!attacker)
		attacker = self;

	VectorCopy (origin, start);
	VectorNormalize (aimdir);
	VectorMA (start, range, aimdir, end);

	// Pellet Diviation
	vectoangles (aimdir, dir);
	AngleVectors (dir, NULL, right, up);
	u = (crandom() * amm->vspread) * 10;
	r = (crandom() * amm->hspread) * 10;
	VectorMA (end, r, right, end);
	VectorMA (end, u, up, end);

	if (gi.pointcontents (start) & MASK_WATER)
	{
		water = true;
		VectorCopy (start, water_start);
		content_mask &= ~MASK_WATER;
	}

	tr = gi.trace (start, NULL, NULL, end, self, content_mask);

	if (amm->trail_effect)
		MCastTempEnt(amm->trail_effect, self, NULL, start, tr.endpos, NULL, 0, 0, 0, MULTICAST_PVS);

	// see if we hit water
	if (tr.contents & MASK_WATER)
	{
		int		color;
		water = true;

		VectorCopy (tr.endpos, water_start);
		if (!VectorCompare (start, tr.endpos)) {
			VectorSubtract(end, start, forward);
			VectorNormalize(forward);

			// Laser weapon?
			if (amm->trail_effect == TE_BFG_LASER) {
				if (!(tr.contents & CONTENTS_LAVA)) {

					// Split the laser-beam, 1/2 continues in normal direction
					VectorMA(tr.endpos, 2, forward, start);
					fire_pellet (self, attacker, start, forward, range, damage / 2, amm);

					// other half is reflected off the surface
					for (i=0; i<3; i++)
						if (tr.plane.normal[i])
							forward[i] *= -(abs(tr.plane.normal[i]));

					fire_pellet (self, attacker, tr.endpos, forward, range, damage, amm);

					return;
				}
			} else {
				if (tr.contents & CONTENTS_WATER) {
					if (DL_strcmp(tr.surface->name, "*brwater", -1, false) == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				} else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
					MCastTempEnt(TE_SPLASH, self, NULL, tr.endpos, tr.plane.normal, NULL, 8, color, 0, MULTICAST_PVS);

				VectorMA(tr.endpos, 2, forward, start);
				fire_pellet (self, attacker, start, forward, range/3.0, damage/3.0, amm);

				return;
			}
		}
	}

	// apply weapon effects
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
		if (tr.fraction < 1.0)
			if (DL_strcmp (tr.surface->name, "sky", 3, false) != 0)
				apply_effects(amm->effect, tr.ent, self, attacker, start, tr.endpos, aimdir, tr.plane.normal, range, damage, amm);

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

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);
	}
}

void projectile_detonate(edict_t *self) {
vec3_t		origin, dir;

	// calculate position for the explosion entity
	VectorCopy(self->velocity, dir);
	VectorNormalize(dir);
	// Backoff from origin just a little to prevent detonations outside the world
	VectorMA (self->s.origin, -2, dir, origin);
	apply_effects(self->ammotype->effect, self->enemy, self, self->master, origin, origin, self->velocity, self->target_plane, self->speed, self->dmg, self->ammotype);
	G_FreeEdict (self);
}

void projectile_think(edict_t *self) {
vec3_t	dir, head, start, end;
edict_t	*targ, *ignore;
trace_t	tr;
int dmg, i;

	if (level.time > self->timestamp) {
		projectile_detonate(self);
		return;
	}

	self->owner = NULL;
	if (self->ammotype->flight_effect & FE_HOMING) {
		if (self->enemy) {
			VectorSubtract(self->enemy->s.origin, self->s.origin, dir);
			vectoangles(dir, head);
			DL_ChangeAngles(self->s.angles, head, self->ammotype->track_speed, self->s.angles);
		} else {
			targ = NULL;
			while (targ = G_Find(targ, FOFS(classname), "player")) {
//				VectorSubtract(self->s.origin, targ->s.origin, dir);
//				if (VectorLength(dir) < 1024) {
				if (InFOV(self->s.origin, self->s.angles, targ->s.origin, self->ammotype->track_fov, true)) {
					self->enemy = targ;
					break;
				}
			}
		}
	}

	if (self->ammotype->flight_effect & FE_DRUNK) {
		for (i=0; i<3; i++)
			dir[i] = self->ammotype->track_speed[i] * (random() - 0.5);
		VectorRotate(self->s.angles, dir, self->s.angles);
	}

	if (self->ammotype->flight_effect & FE_BFG) {
		targ = NULL;
		while ((targ = findradius(targ, self->s.origin, 256)) != NULL) {
			if ((targ == self) || (targ == self->owner))
				continue;

			// If fired from a weapon, don't laser-attack the master
			// otherwise do (ie, grenades kill thier master!)

			if (!self->supply && ((targ == self->master) || (targ == self->owner) || (targ->rotate_target && (targ->rotate_target == self->owner))))
				continue;

			if (!targ->takedamage)
				continue;

			if (!targ->client && !targ->building)
				continue;

			// don't target teammates
			if (targ->team == self->team)
				continue;

			VectorMA (targ->absmin, 0.5, targ->size, head);
			VectorSubtract (head, self->s.origin, dir);
			VectorNormalize (dir);

			ignore = self;
			VectorCopy (self->s.origin, start);
			VectorMA (start, 2048, dir, end);
			while(1) {
				tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

				if (!tr.ent)
					break;

				// hurt it if we can
				dmg = self->dmg / 100.0;
				if (dmg < 1) dmg = 1;
				if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner)) 
					T_Damage (tr.ent, self, self->master, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, 0, NULL);

				// if we hit something that's not a monster or player we're done
				if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client)) {
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_LASER_SPARKS);
					gi.WriteByte (4);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (0xD0);
					gi.multicast (tr.endpos, MULTICAST_PVS);
					break;
				}

				ignore = tr.ent;
				VectorCopy (tr.endpos, start);
			}

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_LASER);
			gi.WritePosition (self->s.origin);
			gi.WritePosition (tr.endpos);
			gi.multicast (self->s.origin, MULTICAST_PHS);
		}
	}

	AngleVectors(self->s.angles, self->movedir, NULL, NULL);
	VectorScale(self->movedir, self->speed, self->velocity);

	self->nextthink = level.time + FRAMETIME;
}

void projectile_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) {

	if (self->owner == other)
		return;

	if (surf && (surf->flags & SURF_SKY)) {
		if (self->ammotype->ttype & TT_DET_SKY)
			projectile_detonate(self);
		else
			G_FreeEdict (self);
		return;
	}

	self->enemy = other;
	self->owner = NULL;
	if (plane) VectorCopy(plane->normal, self->target_plane);
	else VectorCopy(self->movedir, self->target_plane);

	if ( (self->ammotype->ttype & TT_DETONATE) || ((self->ammotype->ttype & TT_DET_TOUCH) && (other->takedamage)) ) {
		projectile_detonate(self);
		return;
	}

	if (self->ammotype->ttype & TT_STICK) {
		stick(self, other);
		self->movetype = MOVETYPE_NONE;
		self->solid = SOLID_NOT;
		return;
	}
}

edict_t *fire_projectile(edict_t *self, edict_t *attacker, vec3_t start, vec3_t aimdir, float speed, float damg, ammo_t *amm, weap_t *weap) {
edict_t	*proj;
vec3_t	dir, forward, right, up;

	// Projectile Diviation
	vectoangles (aimdir, dir);
	dir[YAW] += (crandom() * amm->hspread);
	dir[PITCH] += (crandom() * amm->vspread);
	AngleVectors (dir, forward, right, up);

	proj = G_Spawn();
	VectorCopy (start, proj->s.origin);
	VectorCopy (forward, proj->movedir);
	VectorScale (forward, speed, proj->velocity);
	VectorCopy(forward, proj->target_plane);
	VectorCopy (dir, proj->s.angles);

	VectorClear (proj->mins);
	VectorClear (proj->maxs);

	proj->s.effects |= amm->trail_effect;
	proj->s.renderfx |= amm->render_effect;

	proj->s.skinnum = amm->skinnum;
	proj->s.modelindex = gi.modelindex(amm->model);
	proj->s.sound = gi.soundindex(amm->sound);

	proj->owner = self;
	if (self->supply) {
		proj->orig_owner = self->master;
		proj->supply = self->supply; // just to keep track of where the projectile originated from.
	} else {
		proj->orig_owner = self;	// keep the original owner 
		proj->supply = NULL;
	}

	proj->master = attacker;
	proj->team = attacker->team;
	proj->classname = "projectile";
	proj->clipmask = MASK_SHOT;
	proj->solid = SOLID_BBOX;

	if (amm->ftype == FT_FLIGHT)	{
		if (amm->ttype & TT_BOUNCE)	proj->movetype = MOVETYPE_FLYRICOCHET;
		else proj->movetype = MOVETYPE_FLY;
	} else {
		proj->movetype = MOVETYPE_BOUNCE;
		VectorMA(proj->velocity, speed/4.0, up, proj->velocity);
	}

	proj->speed = speed;
	proj->dmg = damg;
	proj->ammotype = amm;
	proj->weaptype = weap;
	proj->gravity = amm->gravity;

	proj->touch = projectile_touch;
	if (amm->flight_effect && (amm->ftype == FT_FLIGHT)) {
		proj->think = projectile_think;
		proj->nextthink = level.time + FRAMETIME;
		proj->timestamp = level.time + amm->duration;
	} else if (amm->ttype & TT_DET_TTL) {
		proj->think = projectile_detonate;
		proj->nextthink = level.time + amm->duration;
	} else {
		proj->think = G_FreeEdict;
		proj->nextthink = level.time + amm->duration;
	}

	gi.linkentity (proj);

	return proj;
}

void GenericMountedWeapon(edict_t *ent, edict_t *attacker, vec3_t origin, vec3_t dir, weap_t *weap, ammo_t *amm) {
vec3_t		angles, forward;
float		damg, range, speed, accuracy;
int			i, n;

	if (!amm) {
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
		ent->pain_debounce_time = level.time + 1;
		return;
	}

	if (DL_strcmp(amm->base_name, weap->base_ammo, -1, false) && DL_strcmp(amm->name, weap->base_ammo, -1, false)) {
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
		ent->pain_debounce_time = level.time + 1;
		return;
	}

	if (amm->pellet_count <= 0) amm->pellet_count = 1;
	damg = (amm->damage + weap->damage_mod) / amm->pellet_count;
	range = (amm->range + weap->range_mod);

	// Accuracy modifiers for movement
	speed = VectorLength(ent->velocity);
	accuracy = weap->accuracy * (1 + (speed / 200));

	// god this is so bad!
	ent->weaptype = weap;
	ent->ammotype = amm;

	VectorCopy(dir, angles);
	for (n = 0; n < weap->rnds_chambered; n++) {
		// Modify angles by accuracy, and project forward
		angles[YAW] += (crandom() * accuracy);
		angles[PITCH] += (crandom() * accuracy);
		AngleVectors (angles, forward, NULL, NULL);
		VectorNormalize(forward);

		for (i = 0; i < amm->pellet_count; i++) {
			if (amm->ftype == FT_PELLET)
				fire_pellet(ent, attacker, origin, forward, range, damg, amm);
			else
				fire_projectile(ent, attacker, origin, forward, range, damg, amm, weap);
		}
		if (!ent->inuse)
			return;
	}

	if (weap->s_load_index)
		MuzzleFlash (ent, weap->s_load_index, MULTICAST_PVS);
	else
		MuzzleFlash2 (ent, 61, MULTICAST_PVS);

	// use CHAN_AUTO because some mounted weapons carry more than one weapon,
	// CHAN_WEAP causes the muzzleflash sound to be sampled over, rather than mixed.
	gi.sound(ent, CHAN_AUTO, gi.soundindex(weap->snd_fire), 1, ATTN_NORM, 0);
}

void GenericWeapon(edict_t *ent, weap_t *weap, ammo_t *amm, int rnds_avail) {
vec3_t		start, offset, kvel, end;
vec3_t		angles, forward, right;
qboolean	water = false;
int			i, n, kick, content_mask = MASK_SHOT | MASK_WATER, rnds_fired;
float		damg, range, speed, accuracy, recoil;
trace_t		tr;

	if (!amm) {
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
		ent->pain_debounce_time = level.time + 1;
		return;
	}

	if (DL_strcmp(amm->base_name, weap->base_ammo, -1, false) && DL_strcmp(amm->name, weap->base_ammo, -1, false)) {
		gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
		ent->pain_debounce_time = level.time + 1;
		return;
	}

	damg = (amm->damage + weap->damage_mod) / amm->pellet_count;
	range = (amm->range + weap->range_mod);
	recoil = weap->recoil;
	kick = damg * 3;

	if (ent->client) {
		// Build angle set from kick values, etc.
		VectorMA (ent->client->v_angle, 1.5, ent->client->oldkick, angles);
		VectorCopy(angles, ent->client->ps.viewangles);
	} else {
		VectorCopy(ent->s.angles, angles);
	}

	// Accuracy modifiers for movement
	speed = VectorLength(ent->velocity);
	accuracy = weap->accuracy * (1 + (speed / 200));

	// Improve accuracy when crouched
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED) {
		accuracy /= 2.0;
		recoil /= 2.0;
	}

	if (rnds_avail > weap->rnds_chambered) rnds_fired = weap->rnds_chambered;
	else rnds_fired = rnds_avail;
	for (n = 0; n < rnds_avail; n++) {
		// Modify angles by accuracy, and project forward
		// Modify angles by gunangles, so the aim cooresponds to the model on screen
		angles[YAW] += (crandom() * accuracy) + ent->client->ps.gunangles[YAW];
		angles[PITCH] += (crandom() * accuracy) + ent->client->ps.gunangles[PITCH];
		AngleVectors (angles, forward, right, NULL);

		if (ent->client) {
			// Weapon recoil
			for (i=1 ; i<3 ; i++) {
				ent->client->kick_origin[i] = crandom() * (recoil / 20);
				ent->client->kick_angles[i] = crandom() * (recoil / 10);
			}
			ent->client->kick_origin[0] = crandom() * recoil;
			ent->client->kick_angles[0] = random() * -recoil;
			VectorAdd(ent->client->oldkick, ent->client->kick_angles, ent->client->oldkick);

			if (abs(weap->recoil) > 10) {
				VectorScale (forward, -500.0 * (float)recoil / ent->mass, kvel);
				VectorAdd (ent->velocity, kvel, ent->velocity);
			}
		}

		// Project Source
		VectorSet(offset, 0, 8, ent->viewheight);
		VectorAdd(offset, weap->fire_offset, offset);
		if (ent->client->pers.hand == LEFT_HANDED)
			offset[1] *= -1;
		else if (ent->client->pers.hand == CENTER_HANDED)
			offset[1] = 0;
		G_ProjectSource (ent->s.origin, offset, forward, right, start);

		// Accurate Aim (on crosshair, not off-target)
		VectorSet(offset, 0, 0, ent->viewheight);
		VectorAdd(ent->s.origin, offset, kvel);
		VectorMA(kvel, 8192, forward, end);
		tr = gi.trace(kvel, NULL, NULL, end, ent, MASK_SHOT);
		VectorSubtract(tr.endpos, start, forward);
		VectorNormalize(forward);

		for (i = 0; i < amm->pellet_count; i++) {
			if (amm->ftype == FT_PELLET)
				fire_pellet(ent, ent, start, forward, range, damg, amm);
			else
				fire_projectile(ent, ent, start, forward, range, damg, amm, weap);
		}
	}

	if (weap->s_load_index) {
		if (weap->s_load_index < 40)
			MuzzleFlash (ent, weap->s_load_index, MULTICAST_PVS);
		else
			MuzzleFlash2 (ent, weap->s_load_index, MULTICAST_PVS);
	} else 
		MuzzleFlash2 (ent, 61, MULTICAST_PVS);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex(weap->snd_fire), 1, ATTN_NORM, 0);
	if (ent->client && (ent->client->dmg_time > level.time)) {
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_DOUBLE;
	}
}
/*
WEAPON_READY, 
WEAPON_ACTIVATING,
WEAPON_DROPPING,
WEAPON_FIRING,
WEAPON_JAMMED,
WEAPON_RELOADING
*/
void WeaponThink(edict_t *ent) {
int	wState, vw_index;
weap_t *weap;
ammo_t *ammo;
item_t	*item;
vec3_t	diff;

	if (!ent)
		return;

	if (!ent->client)
		return;

	if (ent->use_this) {
		VectorSubtract(ent->s.origin, ent->use_this->s.origin, diff);
		if ((VectorLength(diff) > 64) || (ent->use_this->use_this != ent)) {
			if (ent->use_this->use_this == ent)
				ent->use_this->use_this  = NULL;
			ent->use_this->activator = NULL;
			ent->use_this = NULL;
			ent->client->nextweap = NULL;
			ent->client->curweap = ent->client->nextweap;
		}
	}

	if (ent->deadflag) {
		ent->client->ps.gunframe = 0;
		ent->client->ps.gunindex = 0;
		ent->s.modelindex2 = 0;
		return;
	}

// Check weapon/ammo vars and ensure everything is cool....

	// check for valid weapon
	if (!ent->client->curweap)
		ent->client->curweap = FindPreferedWeapon(ent);

	if (!ent->client->curweap)
		return;

	weap = ITEMWEAP(ent->client->curweap);
	if (!weap || (weap->flags & WT_NOTUSABLE)) {
		gi.cprintf(ent, PRINT_HIGH, "Invalid weapon selection\n");
		return;
	}

	// check for weapon change
	if (!ent->client->nextweap) {
		ent->client->nextweap = ent->client->curweap;

		// Check VWEAP
		if (weap->player_model) {
			if (weap->player_model[0] == '*') {
				char *p, skin[MAX_QPATH];
				strcpy(skin, Info_ValueForKey(ent->client->pers.userinfo, "skin"));
				for (p = skin; *p && (*p != '/'); p++);
				*p = 0;
				ent->client->vw_index = gi.modelindex(va("players/%s/%s", skin, &weap->player_model[1]));
			} else
				ent->client->vw_index = gi.modelindex(weap->player_model);
		}
	}

	// drop for weapon change
	if (!(weap->use_count && (ent->client->weaponstate == WEAPON_RELOADING)) )
		if (ent->client->nextweap && (ent->client->nextweap != ent->client->curweap) && (ent->client->weaponstate != WEAPON_DROPPED))
			ent->client->weaponstate = WEAPON_DROPPING;

	// check ammo compatibility
	ammo = ent->client->curweap->cliptype;
	if (!IsAmmoCompatible(weap, ammo)) {
		if (weap->flags & WT_NOAMMO)
			ammo = ent->client->curweap->cliptype = GetAmmoByName(weap->base_ammo);
		else if (item = FindPreferedCompatibleAmmo(ent, weap->base_ammo))
			ammo = ent->client->curweap->cliptype = ITEMAMMO(item);
		else if (item = FindItemByName(ent, weap->base_ammo, ITEM_AMMO))
			ammo = ent->client->curweap->cliptype = ITEMAMMO(item);
		else
			ammo = ent->client->curweap->cliptype = NULL;
	}

	// check for ammo change
	if (!IsAmmoCompatible(weap, ent->client->nextammo))
		ent->client->nextammo = ammo;

	// drop for ammo change
	if (ent->client->nextammo && (ent->client->nextammo != ammo) && (ent->client->weaponstate != WEAPON_RELOADING))
		ent->client->weaponstate = WEAPON_RELOADING;

	// do VWEAP
	if ((ent->client->weaponstate != WEAPON_DROPPING) &&
		(ent->client->weaponstate != WEAPON_ACTIVATING) &&
		(ent->client->weaponstate != WEAPON_DROPPED) ) {
		if (ent->client->vw_index) vw_index = ent->client->vw_index;
		else vw_index = 255; // default vweap index
	} else
		vw_index = 0;
	
	// check weapon model for camera use
	if (ent->client->dummy) {
		ent->client->ps.gunindex = 0;
		ent->client->dummy->s.modelindex2 = vw_index;
	} else {
		ent->client->ps.gunindex = gi.modelindex(weap->view_model);
		ent->s.modelindex2 = vw_index;
	}

// Do some misc (usually unrelated) checks for dl2

	// Check this here because, ClientThink intervals are not 0.1 seconds
	if (ent->maxs[2] == 4.0)
		SearchDeadBodies(ent);

	// Check to see if the player is "using" something else
	// if so, hide thier weapon
	if (ent->use_this && (ent->use_this->use_this == ent) && (ent->client->weaponstate != WEAPON_DROPPED))
		ent->client->weaponstate = WEAPON_DROPPING;

// Everything is setup, from here on down, life is simple (!not)

	wState = ent->client->weaponstate;


	//=============================================
	//
	// Weapon Idle
	//
	//=============================================
	ent->client->weapon_sound = gi.soundindex(weap->snd_idle);
	if ((wState == WEAPON_RELOADING) || (wState == WEAPON_DROPPING))
		ent->client->weapon_sound = 0;

	if (wState == WEAPON_READY) {
		if (ent->client->nextweap && (ent->client->nextweap != ent->client->curweap)) {
			ent->client->weaponstate = WEAPON_DROPPING;
			ent->client->ps.gunframe = weap->frame_drop;
			return;
		}

//		if (ent->client->ps.gunframe == weap->frame_idle)
//			ent->client->weapon_sound = gi.soundindex(weap->snd_idle);

		// Jump to idle frames, if were not already there..
		if ((ent->client->ps.gunframe < weap->frame_idle) ||
			(ent->client->ps.gunframe > weap->frame_idle + weap->frame_idle_num))
			ent->client->ps.gunframe = weap->frame_idle;

		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) &&
			(ent->client->weapon_firelast <= level.framenum) ) {
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			ent->client->ps.gunframe = weap->frame_fire;
			ent->client->weaponstate = WEAPON_FIRING;
			return;
		}

		// Restart idle animation if we reached the last frame in the set.
		if (ent->client->ps.gunframe >= weap->frame_idle + weap->frame_idle_num) {
			ent->client->ps.gunframe = weap->frame_idle;
			return;
		}

		// Don't pause in "idle" animations if "NOIDLEPAUSE" is set
//		if (!(weap->flags & WT_NOIDLEPAUSE) && (random() < 0.05))
//			ent->client->wpause_time = level.time + random() * 1;

//		if (ent->client->wpause_time < level.time)
			ent->client->ps.gunframe++;
		return;


	//=============================================
	//
	// Weapon Activating (Raise after switch)
	//
	//=============================================
	} else if (wState == WEAPON_ACTIVATING) {
		if (ent->client->ps.gunframe == weap->frame_raise)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex(weap->snd_raise), 1, ATTN_STATIC, 0);

		if ((ent->client->ps.gunframe < weap->frame_raise) || 
			(ent->client->ps.gunframe > weap->frame_raise + weap->frame_raise_num)) {
			ent->client->ps.gunframe = weap->frame_raise;
			return;
		}

		if (ent->client->ps.gunframe == weap->frame_raise + weap->frame_raise_num) {
			ent->client->ps.gunframe = weap->frame_idle;
			ent->client->weaponstate = WEAPON_READY;
			return;
		}

		ent->client->ps.gunframe++;
		return;

		
	//=============================================
	//
	// Weapon Dropping
	//
	//=============================================
	} else if ((wState == WEAPON_DROPPING) || (wState == WEAPON_JAMMED)) {
		if (ent->client->ps.gunframe == weap->frame_drop)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex(weap->snd_drop), 1, ATTN_STATIC, 0);

		if ((ent->client->ps.gunframe < weap->frame_drop) ||
			(ent->client->ps.gunframe > weap->frame_drop + weap->frame_drop_num)){
			ent->client->ps.gunframe = weap->frame_drop;
			return;
		}

		if (ent->client->ps.gunframe == weap->frame_drop + weap->frame_drop_num) {
			ent->client->weaponstate = WEAPON_DROPPED;
			return;
		}

		ent->client->ps.gunframe++;
		return;

		
	//=============================================
	//
	// Weapon Dropped (do switch, etc)
	//
	//=============================================
	} else if (wState == WEAPON_DROPPED) {
		// player is using something else - don't let the weapon come back up
		if (ent->use_this && ent->use_this->use_this == ent) return;
		else ent->use_this = NULL;

		ent->client->burstcnt = 0;
		// Switching weapons
		if ((ent->client->nextweap != ent->client->curweap)) {
			if (ent->client->nextweap) {
				ent->client->curweap = ent->client->nextweap;	// don't bother to set curweap, since we return at end of block
				weap = ITEMWEAP(ent->client->curweap);
			}

			// Check VWEAP
			if (weap->player_model) {
				if (weap->player_model[0] == '*') {
					char *p, skin[MAX_QPATH];
					strcpy(skin, Info_ValueForKey(ent->client->pers.userinfo, "skin"));
					for (p = skin; *p && (*p != '/'); p++);
					*p = 0;
					ent->client->vw_index = gi.modelindex(va("players/%s/%s", skin, &weap->player_model[1]));
				} else
					ent->client->vw_index = gi.modelindex(weap->player_model);
			}

			ent->client->weaponstate = WEAPON_ACTIVATING;
			ent->client->ps.gunframe = weap->frame_raise;
			// ent->client->curweap->cliptype = ITEMAMMO(FindPreferedCompatibleAmmo(ent, weap->base_ammo));
			return;
		}

		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
			ent->client->latched_buttons &= ~BUTTON_ATTACK;

		ent->client->weaponstate = WEAPON_ACTIVATING;
		ent->client->ps.gunframe = weap->frame_raise;

		
	//=============================================
	//
	// Weapon Firing
	//
	//=============================================
	} else if (wState == WEAPON_FIRING && ent->client->curweap) {
		//=========================================================
		// Weapon can't fire for one of a number of reason:
		// No ammo selected, clip empty, no ammo, not enough ammo
		//=========================================================
		if (!ammo || (!(weap->flags & WT_NOAMMO) && ((ent->client->curweap->clipcnt <= 0) || ((weap->clip_size > 1) && (ent->client->curweap->clipcnt <= 0))) )) {
			// play our generic "noammo" click
			if (ent->pain_debounce_time < level.time) {
				gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/noammo.wav"), 1, ATTN_STATIC, 0);
				ent->pain_debounce_time = level.time + 1;
			}

			if (!ammo) {
				// No ammo selected, so select a new type of compatible ammo, don't drop
				// this should never happen - ammo/curweap->cliptype will be set on next call to WeaponThink.
				ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = weap->frame_idle;
				return;
			} else if (ent->client->curweap->clipcnt <= 0) {
				// Out of current ammo, so select a new type of compatible ammo, drop for reload
				CleanInventory(ent);
				// Check for more of the same ammo
				if (ent->client->curweap->cliptype)
					if (!FindItemByName(ent, ent->client->curweap->cliptype->name, ITEM_AMMO))
						ent->client->curweap->cliptype = ent->client->nextammo = NULL;
			}
			// otherwise (clip empty, or not enough ammo)
			// just click, and make them reload - note that
			// reloading doesn't solve the "not enough ammo" problem
			ent->client->weaponstate = WEAPON_RELOADING;
			ent->client->curweap->usecnt++;
			return;
		} 

		//=========================================================
		// Cyclic rate for the weapon has expired,
		// which allows the weapon to be fired again
		//=========================================================
		if (ent->client->weapon_firelast <= level.framenum) {
		int	fire_frame;

			ent->client->weapon_firelast = level.framenum + (weap->cyclic_rate / FRAMETIME);
			// Check if the player was killed while using his weapon (likely killed himself)
			if (!ent->client->curweap)
				return;

			// start the player animation
			ent->client->anim_priority = ANIM_ATTACK;
			GetPPMFrameSet(ent->client, PPM_ATTACK, &fire_frame, &ent->client->anim_end);
			if (ent->s.frame != fire_frame + 1)
				ent->s.frame = fire_frame;

			// check to see if the weapon jams
			if (weap->reliability/100.0 > random()) {
				int rnds_fired = weap->rnds_chambered;

				if (!(weap->flags & WT_NOAMMO))
					if (rnds_fired > ent->client->curweap->clipcnt)
						rnds_fired = ent->client->curweap->clipcnt;

				// if it doesn't, fire the gun! 
				GenericWeapon(ent, weap, ammo, rnds_fired);
				if (!ent->client->curweap)
					return;

				if (!(dlsys.playmode & PM_INFINITEAMMO) && !(weap->flags & WT_NOAMMO))
					ent->client->curweap->clipcnt -= rnds_fired;

				// Check for Burst Mode
				if ((weap->flags != WT_AUTO) || ((weap->flags & WT_BURST) && (++ent->client->burstcnt >= weap->burst_cnt)) ) {
					// Semi-automatic and Burst weapons at the end of thier cycle
					// can't fire until thier frame_fire cycle has ended
					ent->client->weapon_firelast = level.framenum + weap->frame_fire_num;
				}
				// If the weapon uses a fire_frame, then
				// set gunframe to fire_frame
				if (weap->fire_frame)
					ent->client->ps.gunframe = weap->fire_frame - 1;

				if (weap->use_count && (++ent->client->curweap->usecnt >= weap->use_count)) {
					ent->client->weaponstate = WEAPON_RELOADING;
					return;
				}
			} else {
				// The weapon has jammed or malfunctioned
				if (!(dlsys.playmode & PM_INFINITEAMMO) && !(weap->flags & WT_NOAMMO))
					ent->client->curweap->clipcnt--;

				if (level.time >= ent->pain_debounce_time) {
					gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/jammed.wav"), 1, ATTN_STATIC, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				ent->client->weaponstate = WEAPON_JAMMED;
				ent->client->ps.gunframe = weap->frame_drop;

				return;
			}
		}

		ent->client->ps.gunframe++;

		// For automatic/burst mode weapons that are under-fire, keep weapon frame in fire-set
		if ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) {
			if ((weap->flags & WT_AUTO) || ((weap->flags & WT_BURST) && (ent->client->burstcnt < weap->burst_cnt))) {
				if ((ent->client->ps.gunframe < weap->frame_fire) ||
					(ent->client->ps.gunframe >= weap->frame_fire + weap->frame_fire_num))
					ent->client->ps.gunframe = weap->frame_fire;
				return;
			}
		}

		ent->client->burstcnt = 0;

		// If weapon a non-automatic is not under-fire,
		// continue to run weapon frames until the last one is reached.
		if (!(weap->flags & WT_AUTO))
			if ((ent->client->ps.gunframe >= weap->frame_fire) &&
				(ent->client->ps.gunframe < weap->frame_fire + weap->frame_fire_num) )
				return;

		// Weapon is finally idle, so reset its state
		ent->client->weaponstate = WEAPON_READY;
		return;

	//=============================================
	//
	// Weapon Reload - This is A TOTAL HACK!
	//
	//=============================================
	} else if (wState == WEAPON_RELOADING) {
		// Switching ammo
		if (!ammo && !ent->client->nextammo) {
			ent->client->ps.gunframe = weap->frame_idle;
			ent->client->weaponstate = WEAPON_READY;
			return;
		}

		if ((ent->client->nextammo != ammo)) {
			// verify candidacy of new ammo
			if (IsAmmoCompatible(weap, ent->client->nextammo))
				ent->client->curweap->cliptype = ent->client->nextammo;
			else {
				ent->client->nextammo = NULL;
				return;
			}
		}

		if (weap->frame_reload_num < 1) {
			// drop the weapon if there are no reload anims
			if ((ent->client->ps.gunframe < weap->frame_drop) ||
				(ent->client->ps.gunframe > weap->frame_drop + weap->frame_drop_num)) {
				ent->client->ps.gunframe = weap->frame_drop;
				gi.sound(ent, CHAN_WEAPON, gi.soundindex(weap->snd_reload), 1, ATTN_STATIC, 0);
				// discard the previous clip, losing any rounds it may have had...
				if (ammo)
					if (item = FindItemByName(ent, ammo->name, ITEM_AMMO))
						item->quantity += ent->client->curweap->clipcnt;
				return;
			}
			if (ammo) {
				item = FindItemByName(ent, ammo->name, ITEM_AMMO);
				if (!weap->use_count || (weap->use_count > ent->client->curweap->usecnt)) {
					if ((dlsys.playmode & PM_NORELOADS) && (item->quantity >= weap->clip_size)) {
						item->quantity -= weap->clip_size;
						ent->client->curweap->clipcnt = weap->clip_size;
					} else if (item && item->quantity) {
						if (ent->client->wreload_time < level.time) {
							ent->client->wreload_time = level.time + weap->reload_time;

							if (weap->clip_size <= item->quantity)
								ent->client->curweap->reloadrate = weap->clip_size / weap->reload_time;
							else
								ent->client->curweap->reloadrate = item->quantity / weap->reload_time;
							ent->client->curweap->totalrnds = item->quantity;
						}
						ent->client->curweap->clipcnt = ((weap->reload_time - (ent->client->wreload_time - level.time)) * ent->client->curweap->reloadrate) + 0.5;
						item->quantity = ent->client->curweap->totalrnds - ent->client->curweap->clipcnt;

						if (level.time < ent->client->wreload_time) {
							if (ent->client->ps.gunframe < weap->frame_drop + weap->frame_drop_num)
								ent->client->ps.gunframe++;
							return;
						}
					}
				} else {
					// Weapon is limited use, and has exceeded that use,
					// so discard if after the weapon is dropped
					if (ent->client->ps.gunframe < weap->frame_reload + weap->frame_reload_num) {
						ent->client->ps.gunframe++;
						return;
					}
					RemoveInventoryItem(ent, ent->client->curweap, 1);
					ent->client->ps.gunframe = weap->frame_raise;
					ent->client->weaponstate = WEAPON_ACTIVATING;
					ent->client->curweap = NULL;
					return;
				}
			}
			ent->client->ps.gunframe = weap->frame_raise;
			ent->client->weaponstate = WEAPON_ACTIVATING;
			ent->client->wreload_time = 0.0;
			ent->client->burstcnt = 0;
			return;
		} else {
			// Otherwise, start the reload anim
			if ((ent->client->ps.gunframe < weap->frame_reload) ||
				(ent->client->ps.gunframe > weap->frame_reload + weap->frame_reload_num)) {
				ent->client->ps.gunframe = weap->frame_reload;
				gi.sound(ent, CHAN_WEAPON, gi.soundindex(weap->snd_reload), 1, ATTN_STATIC, 0);
				// discard the previous clip, losing any rounds it may have had...
				if (ammo)
					if (item = FindItemByName(ent, ammo->name, ITEM_AMMO))
						item->quantity += ent->client->curweap->clipcnt;
				return;
			}
			if (ammo) {
				item = FindItemByName(ent, ammo->name, ITEM_AMMO);
				if (!weap->use_count || (weap->use_count > ent->client->curweap->usecnt)) {
					if ((dlsys.playmode & PM_NORELOADS) && (item->quantity >= weap->clip_size)) {
						item->quantity -= weap->clip_size;
						ent->client->curweap->clipcnt = weap->clip_size;
					} else if (item && item->quantity) {
						if (weap->frame_reload_num) {
							if (ent->client->ps.gunframe == weap->frame_reload) {
								if (weap->clip_size <= item->quantity)
									ent->client->curweap->reloadrate = (float)weap->clip_size / (float)weap->frame_reload_num;
								else
									ent->client->curweap->reloadrate = (float)item->quantity / (float)weap->frame_reload_num;
								ent->client->curweap->totalrnds = item->quantity;
							}
							ent->client->curweap->clipcnt = ((ent->client->ps.gunframe - weap->frame_reload) * ent->client->curweap->reloadrate) + 0.5;
						} else
							ent->client->curweap->clipcnt = ent->client->curweap->reloadrate = weap->clip_size;

						item->quantity = ent->client->curweap->totalrnds - ent->client->curweap->clipcnt;

						if (ent->client->ps.gunframe < weap->frame_reload + weap->frame_reload_num) {
							ent->client->ps.gunframe++;
							return;
						}
					}
				} else {
					if (ent->client->ps.gunframe < weap->frame_reload + weap->frame_reload_num) {
						ent->client->ps.gunframe++;
						return;
					}
					RemoveInventoryItem(ent, ent->client->curweap, 1);
					ent->client->ps.gunframe = weap->frame_raise;
					ent->client->weaponstate = WEAPON_ACTIVATING;
					ent->client->curweap = FindItemByName(ent, weap->name, ITEM_WEAPON);
					return;
				}
			}

			// Otherwise, let the animation finish, and then go immediately to the idle frames
			ent->client->ps.gunframe = weap->frame_idle;
			ent->client->weaponstate = WEAPON_READY;
			ent->client->wreload_time = 0.0;
			ent->client->burstcnt = 0;
			return;
		}
	}
}
