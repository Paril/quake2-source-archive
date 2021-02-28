#include "g_local.h"
#include "boss.h"

#define MAKRON_FRAMES_STAND_START	112
#define MAKRON_FRAMES_STAND_END		162
#define MAKRON_FRAMES_WALK_START	168
#define MAKRON_FRAMES_WALK_END		181
#define MAKRON_FRAMES_CHAIN_START	8
#define MAKRON_FRAMES_CHAIN_END		13
#define MAKRON_FRAMES_BFG_START		18
#define MAKRON_FRAMES_BFG_END		26//30

void boss_makron_stepleft (edict_t *self)
{
	gi.sound (self, CHAN_BODY, gi.soundindex ("boss3/step1.wav"), 1, ATTN_NORM, 0);
}

void boss_makron_stepright (edict_t *self)
{
	gi.sound (self, CHAN_BODY, gi.soundindex ("boss3/step2.wav"), 1, ATTN_NORM, 0);
}

void boss_makron_sounds (edict_t *self)
{
	if (!self->groundentity)
		return;

	switch (self->s.frame)
	{
	case 146: boss_makron_stepleft(self); break;
	case 150: boss_makron_stepright(self); break;
	case 159: boss_makron_stepleft(self); break;
	case 162: boss_makron_stepright(self); break;
	case 168: boss_makron_stepleft(self); break;
	case 175: boss_makron_stepright(self); break;
	}
}

void boss_makron_idle (edict_t *self)
{
	G_RunFrames(self, MAKRON_FRAMES_STAND_START, MAKRON_FRAMES_STAND_END, false);
}

void boss_makron_locktarget (edict_t *self)
{
	int		i;
	vec3_t	forward;
	
	if (!self->enemy)
		return;

	VectorSubtract(self->enemy->s.origin, self->owner->s.origin, forward);
	VectorNormalize(forward);
	vectoangles(forward, forward);
	ValidateAngles(forward);

	// set view angles to target
	for (i = 0 ; i < 3 ; i++)
		self->owner->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(forward[i]-self->owner->client->resp.cmd_angles[i]);
	VectorCopy(forward, self->owner->client->ps.viewangles);
	VectorCopy(forward, self->owner->client->v_angle);
}

void boss_makron_bfg (edict_t *self, int flash_number)
{
	int			damage;
	vec3_t		forward, right, start;

	if (self->enemy)
	{
		MonsterAim(self, -1, MAKRON_BFG_SPEED, true, flash_number, forward, start);
	}
	else
	{
		// we don't have a target, so fire where client is aiming
		AngleVectors(self->s.angles, forward, right, NULL);
		G_ProjectSource(self->s.origin, monster_flash_offset[flash_number], forward, right, start);
		AngleVectors(self->owner->client->v_angle, forward, NULL, NULL);
	}
	
	damage = MAKRON_BFG_INITIAL_DAMAGE+MAKRON_BFG_ADDON_DAMAGE*self->monsterinfo.level;
	fire_bfg (self, start, forward, damage, MAKRON_BFG_SPEED, MAKRON_BFG_RADIUS);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (flash_number);
	gi.multicast (start, MULTICAST_PVS);
}

void boss_makron_chain (edict_t *self, int flash_number)
{
	int			damage;
	vec3_t		forward, right, start;
	vec3_t		end;
	trace_t		tr;

	if (self->enemy)
	{
		MonsterAim(self, -1, 0, false, flash_number, forward, start);
	}
	else
	{
		// we don't have a target, so fire where client is aiming
		AngleVectors(self->s.angles, forward, right, NULL);
		G_ProjectSource(self->s.origin, monster_flash_offset[flash_number], forward, right, start); // get muzzle origins
		AngleVectors(self->owner->client->v_angle, forward, NULL, NULL); // get client viewing angle
		VectorMA(start, 8192, forward, end);
		tr = gi.trace(self->owner->s.origin, NULL, NULL, end, self, MASK_SHOT); // point player is aiming at
		VectorSubtract(tr.endpos, start, forward); // vector from muzzle to point
	}
	
	damage = MAKRON_CG_INITIAL_DAMAGE+MAKRON_CG_ADDON_DAMAGE*self->monsterinfo.level;
	fire_bullet (self, start, forward, damage, damage, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MAKRON_CHAINGUN);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (flash_number);
	gi.multicast (start, MULTICAST_PVS);
}

void boss_makron_attack (edict_t *self)
{
	if (!G_ValidTarget(self, self->enemy, true))
	{
		if (!boss_findtarget(self))
			self->enemy = NULL;
	}

	boss_makron_locktarget(self);

	if (self->owner->client->weapon_mode)
	{
		if ((self->s.frame == MAKRON_FRAMES_BFG_START) 
			&& (level.time > self->monsterinfo.attack_finished))
		{
			boss_makron_bfg(self, MZ2_JORG_BFG_1);
			self->monsterinfo.attack_finished = level.time + MAKRON_BFG_DELAY;
		}
	}
	else
	{
		boss_makron_chain(self, MZ2_JORG_MACHINEGUN_R1);
		boss_makron_chain(self, MZ2_JORG_MACHINEGUN_L1);
	}
}

void boss_makron_crush (edict_t *self)
{
	vec3_t	add={32,32,0}, sub={-32,-32,0}; // box must be bigger than movesize
	vec3_t	boxmin, boxmax;
	trace_t tr;

	// only call this function when the boss is moving
	// his legs will crush anyone near them
	if ((self->style != FRAMES_RUN_FORWARD) && (self->style != FRAMES_RUN_BACKWARD))
		return;

	// check box just outside of our bounding box for entities
	VectorAdd(self->mins, sub, boxmin);
	VectorAdd(self->maxs, add, boxmax);
	tr = gi.trace(self->s.origin, boxmin, boxmax, self->s.origin, self, MASK_SHOT);
	if ((tr.fraction < 1) && G_EntExists(tr.ent) 
		&& (tr.ent->absmin[2] <= self->absmin[2]+32)) // target is at foot level
		T_Damage(tr.ent, self, self, vec3_origin, self->s.origin, 
			tr.plane.normal, MAKRON_TOUCH_DAMAGE, MAKRON_TOUCH_DAMAGE, 0, MOD_MAKRON_CRUSH);
}



int makron_walk_skipframes[] = {169,170,176,177,0};

void boss_makron_think (edict_t *self)
{
	if (!boss_checkstatus(self))
		return;

	if (self->style == FRAMES_RUN_FORWARD)
		G_RunFrames1(self, MAKRON_FRAMES_WALK_START, MAKRON_FRAMES_WALK_END, makron_walk_skipframes, false);
	else if (self->style == FRAMES_RUN_BACKWARD)
		G_RunFrames1(self, MAKRON_FRAMES_WALK_START, MAKRON_FRAMES_WALK_END, makron_walk_skipframes, true);
	else if (self->style == FRAMES_ATTACK)
	{
		if (self->owner->client->weapon_mode)
			G_RunFrames(self, MAKRON_FRAMES_BFG_START, MAKRON_FRAMES_BFG_END, false);
		else
			G_RunFrames(self, MAKRON_FRAMES_CHAIN_START, MAKRON_FRAMES_CHAIN_END, false);
		boss_makron_attack(self);
	}
	else
		boss_makron_idle(self);

	boss_makron_sounds(self);
	boss_regenerate(self);
	boss_makron_crush(self);
//	boss_makron_attack(self);

	self->nextthink = level.time + FRAMETIME;
}

void boss_makron_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	V_Touch(self, other, plane, surf);

	if (!G_EntExists(other))
		return;
	if (other->absmin[2] > self->absmin[2]+32) // below footheight
		return;
	// hurt them
	if (!self->groundentity)
		T_Damage(other, self, self, vec3_origin, self->s.origin, 
			plane->normal, MAKRON_TOUCH_DAMAGE, MAKRON_TOUCH_DAMAGE, 0, MOD_MAKRON_CRUSH);
}

void boss_makron_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	respawn(self->activator);

	self->deadflag = DEAD_DEAD;
	self->takedamage = DAMAGE_NO; // don't call damage/die function any more
	self->think = BecomeExplosion1;
	self->nextthink = level.time + FRAMETIME;
	SpawnRune(self, attacker, false);
	SpawnRune(self, attacker, false);
	SpawnRune(self, attacker, false);
}

void boss_makron_spawn (edict_t *ent)
{
	char	userinfo[MAX_INFO_STRING];
	edict_t	*boss;

	if (G_EntExists(ent->owner) && (ent->owner->mtype == BOSS_MAKRON))
	{
		G_PrintGreenText(va("%s got bored and left the game.", ent->client->pers.netname));

		BecomeTE(ent->owner);
		ent->svflags &= ~SVF_NOCLIENT;
		ent->viewheight = 22;
		ent->movetype = MOVETYPE_WALK;
		ent->solid = SOLID_BBOX;
		ent->takedamage = DAMAGE_AIM;

		// recover player info
		memcpy(userinfo, ent->client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant(ent->client);
		ClientUserinfoChanged(ent, userinfo);
		modify_max(ent);
		Pick_respawnweapon(ent);
		ent->owner = NULL;
		return;
	}

	G_PrintGreenText(va("A level %d boss known as %s has spawned!", average_player_level, ent->client->pers.netname));

	// create the tank entity that the player will pilot
	boss = G_Spawn();
	boss->classname = "boss";
	boss->solid = SOLID_BBOX;
	boss->takedamage = DAMAGE_YES;
	boss->movetype = MOVETYPE_STEP;
	boss->clipmask = MASK_MONSTERSOLID;
	boss->svflags |= SVF_MONSTER;
	boss->activator = ent;
	boss->die = boss_makron_die;
	boss->think = boss_makron_think;
	boss->touch = boss_makron_touch;
	boss->mass = 1000;
	boss->monsterinfo.level = average_player_level;
	boss->health = MAKRON_INITIAL_HEALTH+MAKRON_ADDON_HEALTH*boss->monsterinfo.level + 15000;
	boss->max_health = boss->health;
	boss->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	boss->monsterinfo.power_armor_power = MAKRON_INITIAL_ARMOR+MAKRON_ADDON_ARMOR*boss->monsterinfo.level;
	boss->monsterinfo.max_armor = boss->monsterinfo.power_armor_power;
	boss->svflags |= SVF_MONSTER; // needed for armor

	// okay maybe someone WOULD be interested in a boss.
	boss->flags |= FL_CHASEABLE; // 3.65 indicates entity can be chase cammed
	boss->mtype = BOSS_MAKRON;
	boss->pain = boss_pain;
	// set up pointers
	boss->owner = ent;
	ent->owner = boss;
	
	boss->s.modelindex = gi.modelindex ("models/monsters/boss3/jorg/tris.md2");
	VectorSet (boss->mins, -70, -70, 0);
	VectorSet (boss->maxs, 70, 70, 140);
	VectorCopy(ent->s.angles, boss->s.angles);
	boss->s.angles[PITCH] = 0; // monsters don't use pitch
	boss->nextthink = level.time + FRAMETIME;
	VectorCopy(ent->s.origin, boss->s.origin);
	VectorCopy(ent->s.old_origin, boss->s.old_origin);

	// link up entities
	gi.linkentity(boss);
	gi.linkentity(ent);

	// make the player into a ghost
	ent->svflags |= SVF_NOCLIENT;
	ent->viewheight = 0;
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->takedamage = DAMAGE_NO;
	ent->client->ps.gunindex = 0;
	memset (ent->client->pers.inventory, 0, sizeof(ent->client->pers.inventory));
	ent->client->pers.weapon = NULL;
}

