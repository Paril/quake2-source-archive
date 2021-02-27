#include "g_local.h"

// misc
#define CEILING_PITCH				90
#define FLOOR_PITCH					270

void spawn_grenades(edict_t *ent, vec3_t origin, float time, int damage, int num);

void carpetbomb_think (edict_t *self)
{
	float		ceil;
	qboolean	failed = false;
	vec3_t		forward, right, start, end;
	trace_t		tr, tr1;

	if (!G_EntIsAlive(self->owner) || (level.time>self->delay))
	{
		G_FreeEdict(self);
		return;
	}

	// move forward
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, GetRandom(CARPETBOMB_DAMAGE_RADIUS/2, CARPETBOMB_DAMAGE_RADIUS+1), forward, start);
	tr = gi.trace(self->s.origin, NULL, NULL, start, NULL, MASK_SOLID);
	VectorCopy(start, end);
	start[2]++;
	end[2] -= 8192;
	tr1 = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);
	start[2]--;

	if ((tr.fraction < 1) || (start[2] != tr1.endpos[2]))
	{
		//gi.dprintf("will step down, start[2] %f tr1.endpos[2] %f\n", start[2], tr1.endpos[2]);
		// get current ceiling height
		VectorCopy(start, end);
		end[2] += 8192;
		tr = gi.trace(self->s.origin, NULL, NULL, end, NULL, MASK_SOLID);
		ceil = tr.endpos[2];

		// push down from above desired position
		start[2] += CARPETBOMB_STEP_SIZE;
		if ((start[2] > ceil)) // dont go thru ceiling
			start[2] = ceil;
		VectorCopy(start, end);
		end[2] -= 8192;
		tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);

		// dont go thru walls
		if (tr.allsolid)
			failed = true;
		// try a bit lower
		if (tr.startsolid)
		{
			start[2] -= CARPETBOMB_STEP_SIZE;
			tr = gi.trace(start, NULL, NULL, end, NULL, MASK_SOLID);
			if (tr.startsolid || tr.allsolid)
				failed = true;
		}

		// dont go into water if we aren't already submerged
		VectorCopy(tr.endpos, start);
		start[2] += 8;
		if (!self->waterlevel && (gi.pointcontents(start) & MASK_WATER))
			failed = true;
	}

	// save position
	VectorCopy(tr.endpos, self->s.origin);
	VectorCopy(tr.endpos, start);
	// spawn explosions on either side
	// FIXME: step down from these positions, otherwise we get mid-air explosions!
	AngleVectors(self->s.angles, NULL, right, NULL);
	VectorMA(self->s.origin, (crandom()*GetRandom(CARPETBOMB_CARPET_WIDTH/4, CARPETBOMB_CARPET_WIDTH/2)), right, end);
	// make sure path is wide enough
	tr = gi.trace(self->s.origin, NULL, NULL, end, self, MASK_SHOT);
	VectorCopy(tr.endpos, self->s.origin);
	self->s.origin[2] += 32;

	//4.08 make sure the caster can see this spot
	//if (!visible(self->owner, self))
	if (!G_IsClearPath(self, MASK_SOLID, self->move_origin, self->s.origin))
		failed = true;

	// make sure bombspell is in a valid location
	if ((gi.pointcontents(self->s.origin) & CONTENTS_SOLID) || failed)
	{
		G_FreeEdict(self);
		return;
	}
	T_RadiusDamage(self, self->owner, self->dmg, NULL, self->dmg_radius, MOD_BOMBS);
	// write explosion effects
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	VectorCopy(start, self->s.origin); // retrieve starting position
	self->nextthink = level.time + FRAMETIME;

	gi.linkentity(self);
}

/*
void carpetbomb_think (edict_t *self)
{
	vec3_t	forward, right, start, end;
	trace_t tr;
	
	// trace up to ceiling
	VectorCopy(self->s.origin, start);
	start[2] += CARPETBOMB_MAX_HEIGHT+CARPETBOMB_ROOF_BUFFER;
	tr = gi.trace(self->s.origin, NULL, NULL, start, self, MASK_SOLID);
	VectorCopy(tr.endpos, start);
	start[2] -= CARPETBOMB_ROOF_BUFFER;
	// move position forward
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(start, GetRandom(CARPETBOMB_DAMAGE_RADIUS/2, CARPETBOMB_DAMAGE_RADIUS+1), forward, end);
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID);
	// check if we've run into a wall
	if (tr.fraction < 1)
	{
		// FIXME: write code that ducks under wall openings!
		G_FreeEdict(self);
		return;
	}
	// move spell entity to the floor
	VectorCopy(end, start);
	end[2] -= 8192;
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);
	VectorCopy(tr.endpos, self->s.origin); // save starting position
	VectorCopy(tr.endpos, start);
	// spawn explosions on either side
	AngleVectors(self->s.angles, NULL, right, NULL);
	VectorMA(self->s.origin, (crandom()*GetRandom(CARPETBOMB_CARPET_WIDTH/4, CARPETBOMB_CARPET_WIDTH/2)), right, end);
	// make sure path is wide enough
	tr = gi.trace(self->s.origin, NULL, NULL, end, self, MASK_SHOT);
	VectorCopy(tr.endpos, self->s.origin);
	self->s.origin[2] += 32;
	// make sure bombspell is in a valid location
	if (gi.pointcontents(self->s.origin) & CONTENTS_SOLID)
	{
		G_FreeEdict(self);
		return;
	}
	T_RadiusDamage(self, self->owner, self->dmg, NULL, self->dmg_radius, MOD_BOMBS);
	// write explosion effects
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	VectorCopy(start, self->s.origin); // retrieve starting position
	self->nextthink = level.time + FRAMETIME;
}
*/

void CarpetBomb (edict_t *ent, float skill_mult, float cost_mult)
{
	vec3_t forward, right, start, end, offset;
	trace_t tr;
	edict_t *spell;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called CarpetBomb()\n", ent->client->pers.netname);

	ent->client->pers.inventory[power_cube_index] -= COST_FOR_BOMB*cost_mult;

	// create bombspell entity
	spell = G_Spawn();
	spell->think = carpetbomb_think;
	spell->nextthink = level.time + FRAMETIME;
	spell->owner = ent;
	spell->svflags |= SVF_NOCLIENT;
	spell->solid = SOLID_NOT;
	spell->dmg = CARPETBOMB_INITIAL_DAMAGE + CARPETBOMB_ADDON_DAMAGE*ent->myskills.abilities[BOMB_SPELL].current_level*skill_mult;
	spell->dmg_radius = CARPETBOMB_DAMAGE_RADIUS;
	spell->delay = level.time+CARPETBOMB_DURATION;
	VectorCopy(ent->s.angles, spell->s.angles);
	spell->s.angles[PITCH] = 0;
	spell->s.angles[ROLL] = 0;
	gi.linkentity(spell);

	//4.08 store player position (used for visibility check later)
	VectorCopy(ent->s.origin, spell->move_origin);

	// get bombspell starting position
	AngleVectors(ent->s.angles, forward, right, NULL);
	VectorSet(offset, 0, 7, ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	VectorMA(start, (1+CARPETBOMB_DAMAGE_RADIUS/2), forward, start);
	VectorCopy(start, end);
	end[2] = -8192;
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SOLID);
	VectorCopy(tr.endpos, spell->s.origin);
}

void bombarea_think (edict_t *self)
{
	float	thinktime, bombtime;
	vec3_t	start;
	trace_t	tr;

	if (!G_EntIsAlive(self->owner) || (level.time>self->delay))
	{
		G_FreeEdict(self);
		return;
	}
	VectorCopy(self->s.origin, start);

	thinktime = 0.2 * ((self->delay-6)-level.time);
	if (thinktime < 0.2)
		thinktime = 0.2;
	// if the caster can't see his target, then pause the spell
	/*
	if (!visible(self, self->owner))
	{
		self->nextthink = thinktime;
		return;
	}
	*/

	// spread randomly around target
	start[0] += GetRandom(0, BOMBAREA_WIDTH/2)*crandom();
	start[1] += GetRandom(0, BOMBAREA_WIDTH/2)*crandom();	
	tr = gi.trace(self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (self->s.angles[PITCH] == 90)
		bombtime = 1 + 2*random();
	else
		bombtime = 0.5 + 2*random();
	spawn_grenades(self->owner, tr.endpos, bombtime, self->dmg, 1);
	self->nextthink = level.time + thinktime;
}

void BombArea (edict_t *ent, float skill_mult, float cost_mult)
{
	vec3_t	angles, offset;
	vec3_t	forward, right, start, end;
	trace_t	tr;
	edict_t *bomb;
	int		cost=COST_FOR_BOMB*cost_mult;

#ifdef OLD_NOLAG_STYLE
	// 3.5 don't allow bomb area to prevent lag
	if (nolag->value)
	{
		safe_cprintf(ent, PRINT_HIGH, "Bomb area is temporarily disabled to prevent lag.\n");
		return;
	}
#endif

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7, ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SOLID);

	// make sure this is a floor
	vectoangles(tr.plane.normal, angles);
	ValidateAngles(angles);
	if (angles[PITCH] == FLOOR_PITCH)
	{
		VectorCopy(tr.endpos, start);
		VectorCopy(tr.endpos, end);
		end[2] += BOMBAREA_FLOOR_HEIGHT;
		tr = gi.trace(start, NULL, NULL, end, ent, MASK_SOLID);
	}
	else if (angles[PITCH] != CEILING_PITCH)
	{
		safe_cprintf(ent, PRINT_HIGH, "You must look at a ceiling or floor to cast this spell.\n");
		return;
	}

	bomb = G_Spawn();
	bomb->solid = SOLID_NOT;
	bomb->svflags |= SVF_NOCLIENT;
	VectorClear(bomb->velocity);
	VectorClear(bomb->mins);
	VectorClear(bomb->maxs);
	bomb->owner = ent;
	bomb->delay = level.time + BOMBAREA_DURATION + BOMBAREA_STARTUP_DELAY;
	bomb->nextthink = level.time + BOMBAREA_STARTUP_DELAY;
	bomb->dmg = 50 + 10*ent->myskills.abilities[BOMB_SPELL].current_level*skill_mult;
	bomb->think = bombarea_think;
	VectorCopy(tr.endpos, bomb->s.origin);
	VectorCopy(tr.endpos, bomb->s.old_origin);
	VectorCopy(angles, bomb->s.angles);
	gi.linkentity(bomb);

	gi.sound(bomb, CHAN_WEAPON, gi.soundindex("spells/meteorlaunch_short.wav"), 1, ATTN_NORM, 0);
	ent->client->pers.inventory[power_cube_index] -= cost;

	ent->client->ability_delay = level.time + (DELAY_BOMB * cost_mult);
}

void bombperson_think (edict_t *self)
{
	int		height, max_height;
	float	bombtime, thinktime;
	vec3_t	start;
	trace_t	tr;

	// calculate drop rate
	bombtime = self->delay - 8; // max rate achieved 2 seconds after casting
	if (bombtime < level.time)
		bombtime = level.time;
	thinktime = level.time + 0.25 * ((bombtime + 1) - level.time); // max 1 bomb per 0.25 seconds

	// bomb self-terminates if the enemy dies or owner teleports away
	if (!G_EntIsAlive(self->owner) || !G_EntIsAlive(self->enemy)
		|| (level.time > self->delay)
		|| (self->owner->client->tball_delay > level.time))
	{
		//RemoveCurse(self->enemy, self);
		que_removeent(self->enemy->curses, self, true);
		return;
	}

	VectorCopy(self->enemy->s.origin, self->s.origin);
//	gi.linkentity(self);
	/*
	// if the caster can't see his target, then pause the spell
	if (!visible(self->orb, self->owner))
	{
		self->nextthink = thinktime;
		return;
	}
	*/
	// get random drop height
	max_height = 250 - (20 * self->owner->myskills.abilities[BOMB_SPELL].current_level);
	if (max_height < 150)
		max_height = 150;
	height = GetRandom(50, max_height) + self->enemy->maxs[2];
	// drop bombs above target
    VectorCopy(self->s.origin, start);
	start[2] += height;
    tr = gi.trace(self->s.origin, self->mins, self->maxs, start, self->owner, MASK_SHOT);
	VectorCopy(tr.endpos, start);
	start[2]--;
	// spread randomly around target
	start[0] += (BOMBPERSON_WIDTH/2)*crandom();
	start[1] += (BOMBPERSON_WIDTH/2)*crandom();
	spawn_grenades(self->owner, start, (0.5+2*random()), self->dmg, 1);
	self->nextthink = thinktime;
}

void BombPerson (edict_t *target, edict_t *owner, float skill_mult) 
{
	edict_t *bomb;

	gi.sound(target, CHAN_ITEM, gi.soundindex("spells/meteorlaunch.wav"), 1, ATTN_NORM, 0);
	if ((target->client) && !(target->svflags & SVF_MONSTER))
		safe_cprintf(target, PRINT_HIGH, "SOMEONE SET UP US THE BOMB !!\n");
	//target->cursed |= CURSE_BOMBS;

	bomb=G_Spawn();
	bomb->solid = SOLID_NOT;
	bomb->svflags |= SVF_NOCLIENT;
	VectorClear(bomb->velocity);
	VectorClear(bomb->mins);
	VectorClear(bomb->maxs);
	bomb->owner = owner;
	bomb->enemy = target;
	bomb->delay = level.time + BOMBPERSON_DURATION;
	bomb->dmg = 50 + 10*owner->myskills.abilities[BOMB_SPELL].current_level*skill_mult;
	bomb->mtype = CURSE_BOMBS;
	bomb->nextthink = level.time + 1.0;
	bomb->think = bombperson_think;
	VectorCopy(target->s.origin, bomb->s.origin);
	gi.linkentity(bomb);
	if (!que_addent(target->curses, bomb, BOMBPERSON_DURATION))
		G_FreeEdict(bomb);
}

void Cmd_BombPlayer(edict_t *ent, float skill_mult, float cost_mult)
{
	int		cost=COST_FOR_BOMB*cost_mult;
	vec3_t	forward, right, start, end, offset;
	trace_t	tr;
//	edict_t *other=NULL;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_BombPlayer()\n", ent->client->pers.netname);

	//gi.dprintf("%s\n", gi.args());

	if(ent->myskills.abilities[BOMB_SPELL].disable)
		return;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[BOMB_SPELL].current_level, cost))
		return;

	ent->client->ability_delay = level.time + DELAY_BOMB/* * cost_mult*/;

	// write a nice effect so everyone knows we've cast a spell
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->lastsound = level.framenum;

	// bomb an area
	//if (!Q_strcasecmp(gi.args(), "forward")) 
	if (strstr(gi.args(), "forward"))
	{
		CarpetBomb(ent, skill_mult, cost_mult);
		return;
	}
	// bomb ahead of us
	//if (!Q_strcasecmp(gi.args(), "area")) 
	if (strstr(gi.args(), "area"))
	{
		BombArea(ent, skill_mult, cost_mult);
		return;
	}

	ent->client->pers.inventory[power_cube_index] -= cost;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -3, ent->client->kick_origin);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	VectorMA(start, BOMBPERSON_RANGE, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	if (G_ValidTarget(ent, tr.ent, false))
		BombPerson(tr.ent, ent, skill_mult);
}