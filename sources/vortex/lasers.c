#include "g_local.h"

#define LASER_SPAWN_DELAY		1.0	// time before emitter creates laser beam
#define LASER_INITIAL_DAMAGE	100	// beam damage per frame
#define LASER_ADDON_DAMAGE		40
#define LASER_TIMEOUT_DELAY		120

// cumulative maximum damage a laser can deal
#define LASER_INITIAL_HEALTH	0
#define LASER_ADDON_HEALTH		400

void RemoveLasers (edict_t *ent)
{
	edict_t *e=NULL;

	while((e = G_Find(e, FOFS(classname), "emitter")) != NULL)
	{
		if (e && (e->activator == ent))
		{
			// remove the laser beam
			if (e->creator)
			{
				e->creator->think = G_FreeEdict;
				e->creator->nextthink = level.time + FRAMETIME;
				//G_FreeEdict(e->creator);
			}
			// remove the emitter
			
			e->think = BecomeExplosion1;
			e->nextthink = level.time + FRAMETIME;
			//BecomeExplosion1(e);
		}
	}

	// reset laser counter
	ent->num_lasers = 0;
}

qboolean NearbyLasers (edict_t *ent, vec3_t org)
{
	edict_t *e=NULL;

	while((e = findradius(e, org, 8)) != NULL)
	{
		// is this a laser than we own?
		if (e && e->inuse && G_EntExists(e->activator) 
			&& (e->activator == ent) && !strcmp(e->classname, "emitter"))
			return true;
	}

	return false;
}

qboolean NearbyProxy (edict_t *ent, vec3_t org)
{
	edict_t *e=NULL;

	while((e = findradius(e, org, 8)) != NULL)
	{
		// is this a proxy than we own?
		if (e && e->inuse && G_EntExists(e->owner) 
			&& (e->owner == ent) && !strcmp(e->classname, "proxygrenade"))
			return true;
	}

	return false;
}

void laser_remove (edict_t *self)
{
	// remove emitter/grenade
	self->think = BecomeExplosion1;
	self->nextthink = level.time+FRAMETIME;

	// remove laser beam
	if (self->creator && self->creator->inuse)
	{
		self->creator->think = G_FreeEdict;
		self->creator->nextthink = level.time+FRAMETIME;
	}

	// decrement laser counter
	if (self->activator && self->activator->inuse)
	{
		self->activator->num_lasers--;
		safe_cprintf(self->activator, PRINT_HIGH, "Laser destroyed. %d/%d remaining.\n", 
			self->activator->num_lasers, MAX_LASERS);
	}
}

void laser_beam_effects (edict_t *self)
{
	if (self->activator->teamnum == RED_TEAM || ffa->value)
		self->s.skinnum = 0xf2f2f0f0; // red
	else
		self->s.skinnum = 0xf3f3f1f1; // blue

}

void laser_beam_think (edict_t *self)
{
	int		size;
	int		damage;
	vec3_t	forward;
	trace_t tr;

	// can't have a laser beam without an emitter!
	if (!self->creator)
	{
		G_FreeEdict(self);
		return;
	}

	// set beam color
	if (self->health < 1)// emitter burned out
		size = 0;
	else if (self->monsterinfo.level >= 10)// high-level beam is wider
		size = 4;
	else
		size = 2;
	self->s.frame = size;// set
	
	// set beam color
	laser_beam_effects(self);

	// trace from beam emitter out as far as we can go
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->pos1, 8192, forward, self->pos2);
	tr = gi.trace (self->pos1, NULL, NULL, self->pos2, self->creator, MASK_SHOT);
	VectorCopy(tr.endpos, self->s.origin);
	VectorCopy(self->pos1, self->s.old_origin);

	// set maximum damage output
	if (size)
	{
		damage = self->dmg;
		// don't deal more damage than we have health
		if (damage > self->health)
			damage = self->health;
	}
	else
		damage = 0;// no damage, emitter burned out

	// what is in lasers path?
	if (damage && G_EntExists(tr.ent) && tr.ent != self->activator)
	{
		// remove lasers near spawn positions
		if (tr.ent->client && (tr.ent->client->respawn_time-1.5 > level.time))
		{
			safe_cprintf(self->activator, PRINT_HIGH, "Laser touched respawning player, so it was removed. (%d/%d remain)\n", self->activator->num_lasers, MAX_LASERS);
			laser_remove(self->creator);
			return;
		}

		if (G_GetClient(tr.ent) && invasion->value > 1) // don't deal damage to friends in invasion hard.
			damage = 0;

		// deal damage to anything in the beam's path
		if (!T_Damage(tr.ent, self, self->activator, forward, tr.endpos, 
			vec3_origin, damage, 0, DAMAGE_ENERGY, MOD_LASER_DEFENSE))
			damage = 0; // emitter hasn't burned out yet, but couldn't damage anything, so we're idle
	}
	else
		damage = 0; // emitter is either burned out or hit nothing valid

	// emitter burns out slowly even when idle
	if (size && !damage && !(level.framenum % 10))
	{
		damage = 0.008333 * self->max_health;
		if (damage < 1)
			damage = 1;
	}

	// reduce maximum damage counter
	self->health -= damage;

	// if the counter reaches 0, then shut-down
	if (damage && self->health < 1)
	{
		self->health = 0;
		safe_cprintf(self->activator, PRINT_HIGH, "Laser emitter burned out.\n");
	}

	//gi.dprintf("%d/%d\n", self->health, self->max_health);

	self->nextthink = level.time + FRAMETIME;
}

void laser_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//gi.dprintf("laser_touch\n");
	//gi.dprintf("%s %s %s\n", OnSameTeam(ent, other)?"y":"n", ent->health<ent->max_health?"y":"n", level.framenum>ent->monsterinfo.regen_delay1?"y":"n");

	if (G_EntIsAlive(other) && other->client && OnSameTeam(ent, other) // a player on our team
		&& other->client->pers.inventory[power_cube_index] >= 5 // has power cubes
		&& ent->creator->health < 0.5*ent->creator->max_health // only repair below 50% health (to prevent excessive cube use/repair noise)
		&& level.framenum > ent->creator->monsterinfo.regen_delay1) // check delay
	{
		ent->creator->health = ent->creator->max_health;
		other->client->pers.inventory[power_cube_index] -= 5;
		ent->creator->monsterinfo.regen_delay1 = level.framenum + 20;
		gi.sound(other, CHAN_VOICE, gi.soundindex("weapons/repair.wav"), 1, ATTN_NORM, 0);
		safe_cprintf(other, PRINT_HIGH, "Emitter repaired. Maximum output: %d/%d damage.\n", ent->creator->health, ent->creator->max_health);
	}

}

void emitter_think (edict_t *self)
{
	/*
	if (level.time >= self->delay)
	{
		laser_remove(self);
		return;
	}*/

	// flash yellow when we are about to expire
	if (self->creator->health < (0.1 * self->creator->max_health))
	{
		if (level.framenum & 8)
		{
			self->s.renderfx |= RF_SHELL_YELLOW;
			self->s.effects |= EF_COLOR_SHELL;
		}
		else
		{
			self->s.renderfx = self->s.effects = 0;
		}
	}
	else
		self->s.renderfx = self->s.effects = 0;

	self->nextthink = level.time + FRAMETIME;
}

void SpawnLaser (edict_t *ent, int cost, float skill_mult, float delay_mult)
{
	int		talentLevel = getTalentLevel(ent, TALENT_RAPID_ASSEMBLY);//Talent: Rapid Assembly
	float	delay;
	vec3_t	forward, right, start, end, offset;
	trace_t	tr;
	edict_t *grenade, *laser;

	// get starting position and forward vector
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	// get end position
	VectorMA(start, 64, forward, end);

	tr = gi.trace (start, NULL, NULL, end, ent, MASK_SOLID);

	// can't build a laser on sky
	if (tr.surface && (tr.surface->flags & SURF_SKY))
		return;

	if (tr.fraction == 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "Too far from wall.\n");
		return;
	}

	if (NearbyLasers(ent, tr.endpos))
	{
		safe_cprintf(ent, PRINT_HIGH, "Too close to another laser.\n");
		return;
	}

	if (NearbyProxy(ent, tr.endpos))
	{
		safe_cprintf(ent, PRINT_HIGH, "Too close to a proxy grenade.\n");
		return;
	}

	laser = G_Spawn();
	grenade = G_Spawn();

	// create the laser beam
	laser->monsterinfo.level = ent->myskills.abilities[BUILD_LASER].current_level * skill_mult;
	laser->dmg = LASER_INITIAL_DAMAGE+LASER_ADDON_DAMAGE*laser->monsterinfo.level;
	laser->health = LASER_INITIAL_HEALTH+LASER_ADDON_HEALTH*laser->monsterinfo.level;

	// nerf lasers in CTF and invasion
	//if (ctf->value || invasion->value)
	//	laser->health *= 0.5;

	laser->max_health = laser->health;

	// set beam diameter
	if (laser->monsterinfo.level >= 10)
	{
		
		laser->s.frame = 4;
		laser->s.skinnum = 0xf2f2f0f0; //red beam color
	}
	if (laser->monsterinfo.level >= 20)
	{

		laser->s.frame = 5;
		laser->s.skinnum = 0xdad0dcd2; //Yellow Golden
	}
	else
		{ 
		laser->s.frame = 2;	
		}
	laser->movetype	= MOVETYPE_NONE;
	laser->solid = SOLID_NOT;
	laser->s.renderfx = RF_BEAM|RF_TRANSLUCENT;
	laser->s.modelindex = 1; // must be non-zero
	laser->s.sound = gi.soundindex ("world/laser.wav");
	laser->classname = "laser";
    laser->owner = laser->activator = ent; // link to player
	laser->creator = grenade; // link to grenade
	laser->s.skinnum = 0xf3f3f1f1; // purple beam color
    laser->think = laser_beam_think;
	laser->nextthink = level.time + LASER_SPAWN_DELAY * delay_mult;
	VectorCopy(ent->s.origin, laser->s.origin);
	VectorCopy(tr.endpos, laser->s.old_origin);
	VectorCopy(tr.endpos, laser->pos1); // beam origin
	vectoangles(tr.plane.normal, laser->s.angles);
	gi.linkentity(laser);

	delay = LASER_TIMEOUT_DELAY+GetRandom(0, (int)(0.5*LASER_TIMEOUT_DELAY));

	// laser times out faster in CTF because it's too strong
	//if (ctf->value || invasion->value)
	//	delay *= 0.5;

	// create the laser emmitter (grenade)
    VectorCopy(tr.endpos, grenade->s.origin);
    vectoangles(tr.plane.normal, grenade->s.angles);
	grenade->movetype = MOVETYPE_NONE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade->takedamage = DAMAGE_NO;
	grenade->s.modelindex = gi.modelindex("models/objects/grenade2/tris.md2");
    grenade->activator = ent; // link to player
    grenade->creator = laser; // link to laser
	grenade->classname = "emitter";
	grenade->mtype = M_LASER;//4.5
	grenade->nextthink = level.time+FRAMETIME;//delay; // time before self-destruct
	//grenade->delay = level.time + delay;//4.4 time before self destruct
	grenade->think = emitter_think;//laser_remove;
	grenade->touch = laser_touch;//4.4
	gi.linkentity(grenade);

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5) 
		|| (ent->mtype == MORPH_CACODEMON && ent->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	ent->num_lasers++;
	safe_cprintf(ent, PRINT_HIGH, "Laser built. You have %d/%d lasers.\n", ent->num_lasers, (int)MAX_LASERS);
	ent->client->pers.inventory[power_cube_index] -= cost;
	ent->client->ability_delay = level.time + 0.5 * delay_mult;
	ent->holdtime = level.time + 0.5 * delay_mult;

	ent->lastsound = level.framenum;
}

void Cmd_BuildLaser (edict_t *ent)
{
	int talentLevel, cost=LASER_COST;
	float skill_mult=1.0, cost_mult=1.0, delay_mult=1.0;//Talent: Rapid Assembly & Precision Tuning

	if(ent->myskills.abilities[BUILD_LASER].disable)
		return;

	if (Q_strcasecmp (gi.args(), "remove") == 0)
	{
		RemoveLasers(ent);
		safe_cprintf(ent, PRINT_HIGH, "All lasers removed.\n");
		return;
	}

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5) 
		|| (ent->mtype == MORPH_CACODEMON && ent->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	//Talent: Rapid Assembly
	talentLevel = getTalentLevel(ent, TALENT_RAPID_ASSEMBLY);
	if (talentLevel > 0)
		delay_mult -= 0.1 * talentLevel;
	//Talent: Precision Tuning
	else if ((talentLevel = getTalentLevel(ent, TALENT_PRECISION_TUNING)) > 0)
	{
		cost_mult -= PRECISION_TUNING_COST_FACTOR * talentLevel;
		delay_mult -= PRECISION_TUNING_DELAY_FACTOR * talentLevel;
		skill_mult += PRECISION_TUNING_SKILL_FACTOR * talentLevel;
	}
	cost *= cost_mult;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[BUILD_LASER].current_level, cost))
		return;

	if (ent->num_lasers >= MAX_LASERS)
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't build any more lasers.\n");
		return;
	}

	if (ctf->value && (CTF_DistanceFromBase(ent, NULL, CTF_GetEnemyTeam(ent->teamnum)) < CTF_BASE_DEFEND_RANGE))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't build in enemy base!\n");
		return;
	}

	SpawnLaser(ent, cost, skill_mult, delay_mult);
}
