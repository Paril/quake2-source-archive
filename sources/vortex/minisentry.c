#include "g_local.h"

#define SENTRY_IDLE_LEFT		1
#define SENTRY_IDLE_RIGHT		2
#define SENTRY_TARGET_RANGE		1512		// target aquisition distance
#define SENTRY_ATTACK_RANGE		1812	// maximum attacking distance
#define SENTRY_BUILD_TIME		1.7	// time it takes to build a sentry
#define SENTRY_FOV				60		// field of vision, in degrees
#define SENTRY_REGEN_TIME		21		// seconds it takes sentry to regenerate to full
//#define SENTRY_MAX_HEALTH		(50+5*self->creator->myskills.abilities[BUILD_SENTRY].current_level)
//#define SENTRY_MAX_ARMOR		(50+15*self->creator->myskills.abilities[BUILD_SENTRY].current_level)
#define SENTRY_MAX_AMMO			(MINISENTRY_INITIAL_AMMO+MINISENTRY_ADDON_AMMO*self->creator->myskills.abilities[BUILD_SENTRY].current_level)

void minisentry_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (!self->enemy && G_ValidTarget(self, other, true) 
		&& (entdist(self, other)<SENTRY_ATTACK_RANGE))
		self->enemy = other;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 2;
	gi.sound (self, CHAN_VOICE, gi.soundindex ("tank/tnkpain2.wav"), 1, ATTN_NORM, 0);	
}

void minisentry_remove (edict_t *self)
{
	if (self->deadflag == DEAD_DEAD)
		return;

	if (self->creator && self->creator->inuse)
		self->creator->num_sentries--;

	// remove base immediately
	if (self->owner && self->owner->inuse)
		G_FreeEdict(self->owner);

	// prep sentry for removal
	self->think = BecomeExplosion1;
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->svflags |= SVF_NOCLIENT;
	self->deadflag = DEAD_DEAD;
	self->nextthink = level.time + FRAMETIME;
	gi.unlinkentity(self);
}

void minisentry_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	//FIXME: it is unsafe to remove an entity immediately!
	//gi.dprintf("minisentry_die()\n");

	if (self->creator && self->creator->inuse)
		safe_cprintf(self->creator, PRINT_HIGH, "Your mini-sentry was destroyed.\n");

	minisentry_remove(self);
}

void minisentry_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	V_Touch(self, other, plane, surf);

	//4.2 if this is a player-monster (morph), then we should be looking at the client/player
	if (PM_MonsterHasPilot(other))
		other = other->activator;

	if (other && other->inuse && (other == self->creator) && (level.time > self->random))
	{
		safe_cprintf(self->creator, PRINT_HIGH, "Rotating sentry gun 45 degrees.\n");

		VectorCopy(self->move_angles, self->s.angles); // reset angles to default
		self->s.angles[YAW]+=45;
		ValidateAngles(self->s.angles);
		VectorCopy(self->s.angles, self->move_angles); // update default angles
		self->random = level.time + 1;
		self->nextthink = level.time + 2;
	}
}

qboolean minisentry_checkposition (edict_t *self)
{
	vec3_t	end;
	trace_t tr;
	
	// make sure there is still ground below or above us
	if (self->style == SENTRY_UPRIGHT)
	{
		VectorCopy(self->s.origin, end);
		end[2]--;// -= abs(self->mins[2])+1;
		tr = gi.trace(self->s.origin, self->mins, self->maxs, end, self, MASK_SHOT);//MASK_SOLID);
		if (tr.fraction == 1.0 || (tr.ent != world && tr.ent->mtype != M_LASERPLATFORM))
		{
			//gi.dprintf("%d %d %d\n", tr.fraction==1.0?1:0,tr.ent==world?1:0,tr.ent->mtype==M_LASERPLATFORM?1:0);
			return false;
		}
	}
	else
	{
		VectorCopy(self->s.origin, end);
		end[2]++;// += self->maxs[2]+1;
		tr = gi.trace(self->s.origin, self->mins, self->maxs, end, self, MASK_SHOT);//MASK_SOLID);
		if (tr.fraction == 1.0 || (tr.ent != world && tr.ent->mtype != M_LASERPLATFORM))
		{
			//gi.dprintf("%d %d %d\n", tr.fraction==1.0?1:0,tr.ent==world?1:0,tr.ent->mtype==M_LASERPLATFORM?1:0);
			return false;
		}
	}
	return true;
}

qboolean minisentry_findtarget (edict_t *self)
{
	edict_t	*target=NULL;

	// don't retarget too quickly
	if (self->last_move_time > level.time)
		return false;
	while ((target = findclosestradius (target, self->s.origin, SENTRY_TARGET_RANGE)) != NULL)
	{
		if (!G_ValidTarget(self, target, true))
			continue;
		if (!infov(self, target, SENTRY_FOV))
			continue;
		self->enemy = target;
		self->last_move_time = level.time + 1.0;
		gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/turrspot.wav"), 1, ATTN_NORM, 0);
		return true;
	}
	return false;
}

void minisentry_lockon (edict_t *self)
{
	float	max, temp;
	vec3_t	angles, v;

	// curse causes minisentry to fail to lock-on to enemy
	if ((que_typeexists(self->curses, CURSE)) && random() <= 0.8)
		return;

	temp = self->yaw_speed;
	self->yaw_speed *= 3;
	VectorSubtract(self->enemy->s.origin, self->s.origin, v);
	vectoangles(v, angles);
	self->ideal_yaw = vectoyaw(v);
	M_ChangeYaw(self);
	self->yaw_speed = temp; // restore original yaw speed
	self->s.angles[PITCH] = angles[PITCH];
	ValidateAngles(self->s.angles);
	// maximum pitch is 65 degrees in either direction
	if (self->enemy->s.origin[2] > self->s.origin[2]) // if the enemy is above
	{	
		if (self->owner && self->owner->style == SENTRY_FLIPPED)
			max = 340; // allow 20 degrees up
		else
			max = 315; // allow 45 degrees up
		if (self->s.angles[PITCH] < max)
			self->s.angles[PITCH] = max;
	}
	else
	{
		if (self->owner && self->owner->style == SENTRY_FLIPPED)
			max = 45; // allow 45 degrees down
		else
			max = 20; // allow 20 degrees down
		if (self->s.angles[PITCH] > max)
			self->s.angles[PITCH] = max;
	}
}

qboolean infov (edict_t *self, edict_t *other, int degrees);
void minisentry_attack (edict_t *self)
{
	int			speed, dmg_radius;
	float		chance;
	vec3_t		forward, start, aim;
	qboolean	slowed=false;

	minisentry_lockon(self);
	if (self->light_level < 1)
		return; // out of ammo
	// are we affected by holy freeze?
	//if (HasActiveCurse(self, AURA_HOLYFREEZE))
	if (que_typeexists(self->curses, AURA_HOLYFREEZE))
		slowed = true;
	
	// calculate muzzle location
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorCopy(self->s.origin, start);
	if (self->owner && self->owner->style == SENTRY_FLIPPED)
		start[2] -= abs(self->mins[2]);
	else
		start[2] += self->maxs[2];
	VectorMA(start, (self->maxs[0] + 16), forward, start);

	if ((level.time > self->wait) && infov(self, self->enemy, 30))
	{
		speed = 650 + 35*self->creator->myskills.abilities[BUILD_SENTRY].current_level;
		MonsterAim(self, -1, speed, true, 0, aim, start);
		dmg_radius = self->radius_dmg;
		if (dmg_radius > 150)
			dmg_radius = 150;
		fire_rocket (self, start, aim, self->radius_dmg, speed, dmg_radius, self->radius_dmg);
		if (slowed)
			self->wait = level.time + 2.0;
		else if (self->chill_time > level.time)
			self->wait = level.time + (1.0 * (1 + CHILL_DEFAULT_BASE + CHILL_DEFAULT_ADDON * self->chill_level));
		else
			self->wait = level.time + 1.0;
	
	}
	self->light_level--; // decrease ammo
	
	if (slowed && !(level.framenum%2))
		return;

	// chill effect reduces attack rate/refire
	if (self->chill_time > level.time)
	{
		chance = 1 / (1 + CHILL_DEFAULT_BASE + CHILL_DEFAULT_ADDON * self->chill_level);
		if (random() > chance)
			return;
	}

	fire_bullet (self, start, forward, self->dmg, 2*self->dmg, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_SENTRY);
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/plaser.wav"), 1, ATTN_NORM, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_IONRIPPER|MZ_SILENCED);
	gi.multicast (start, MULTICAST_PVS);
}

void minisentry_idle (edict_t *self)
{
	float	min_yaw, max_yaw;

	if (self->delay > level.time)
		return;
	if (self->delay == level.time)
		gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/gunidle1.wav"), 0.5, ATTN_NORM, 0);

//	self->yaw_speed = 5;
	max_yaw = self->move_angles[YAW] + 45;
	min_yaw = self->move_angles[YAW] - 45;
	// validate angles
	if (max_yaw < 0)
		max_yaw += 360;
	else if (max_yaw > 360)
		max_yaw -= 360;
	if (min_yaw < 0)
		min_yaw += 360;
	else if (min_yaw > 360)
		min_yaw -= 360;
	// sentry scans side-to-side looking for targets
	if (self->style == SENTRY_IDLE_RIGHT)
	{
		self->ideal_yaw = max_yaw;
		M_ChangeYaw(self);
		if (self->s.angles[YAW] == self->ideal_yaw)
		{
			self->style = SENTRY_IDLE_LEFT;
			self->delay = level.time + 1.0;
		}
	}
	else
	{	
		self->ideal_yaw = min_yaw;
		M_ChangeYaw(self);
		if (self->s.angles[YAW] == self->ideal_yaw)
		{
			self->style = SENTRY_IDLE_RIGHT;
			self->delay = level.time + 1.0;
		}
	}	
}

void minisentry_regenerate (edict_t *self)
{
	if (!(level.framenum%10))
	{
		// regenerate health
		if (self->health < self->max_health)
			self->health += self->max_health/SENTRY_REGEN_TIME;
		if (self->health > self->max_health)
			self->health = self->max_health;
		// regenerate armor
		if (self->monsterinfo.power_armor_power < self->monsterinfo.max_armor)
			self->monsterinfo.power_armor_power += self->monsterinfo.max_armor/SENTRY_REGEN_TIME;
		if (self->monsterinfo.power_armor_power > self->monsterinfo.max_armor)
			self->monsterinfo.power_armor_power = self->monsterinfo.max_armor;
		// regenerate ammo
		if (self->light_level < SENTRY_MAX_AMMO)
			self->light_level += SENTRY_MAX_AMMO/SENTRY_REGEN_TIME;
		if (self->light_level > SENTRY_MAX_AMMO)
			self->light_level = SENTRY_MAX_AMMO;
	}
//	gi.dprintf("%d/%d armor\n", self->monsterinfo.power_armor_power, self->monsterinfo.max_armor);
}

void minisentry_think (edict_t *self)
{
	float	modifier, temp;
	que_t	*slot=NULL;

	if (!self->owner || !self->owner->inuse)
	{
		minisentry_remove(self);
		return;
	}
	else if (self->removetime > 0)
	{
		qboolean converted=false;

		if (self->flags & FL_CONVERTED)
			converted = true;

		if (level.time > self->removetime)
		{
			// if we were converted, try to convert back to previous owner
			if (converted && self->prev_owner && self->prev_owner->inuse)
			{
				if (!ConvertOwner(self->prev_owner, self, 0, false))
				{
					minisentry_remove(self);
					return;
				}
			}
			else
			{
				minisentry_remove(self);
				return;
			}
		}
		// warn the converted monster's current owner
		else if (converted && self->creator && self->creator->inuse && self->creator->client 
			&& (level.time > self->removetime-5) && !(level.framenum%10))
				safe_cprintf(self->creator, PRINT_HIGH, "%s conversion will expire in %.0f seconds\n", 
					V_GetMonsterName(self), self->removetime-level.time);	
	}

	// sentry is stunned
	if (self->holdtime > level.time)
	{
		M_SetEffects(self);
		self->nextthink = level.time + FRAMETIME;
		return;
	}

	// toggle sentry spotlight
	if (level.daytime && self->flashlight)
		FL_make(self);
	else if (!level.daytime && !self->flashlight)
		FL_make(self);

	// is the sentry slowed by holy freeze?
	temp = self->yaw_speed;
	slot = que_findtype(self->curses, slot, AURA_HOLYFREEZE);
	if (slot)
	{
		modifier = 1 / (1 + 0.1 * slot->ent->owner->myskills.abilities[HOLY_FREEZE].current_level);
		if (modifier < 0.25) modifier = 0.25;
		self->yaw_speed *= modifier;
	}

	// chill effect slows sentry rotation speed
	if(self->chill_time > level.time)
		self->yaw_speed *= 1 / (1 + CHILL_DEFAULT_BASE + CHILL_DEFAULT_ADDON * self->chill_level);

	if (!self->enemy)
	{
		minisentry_regenerate(self);
		if (minisentry_findtarget(self))
			minisentry_attack(self);
		else
			minisentry_idle(self);
	}
	else
	{
		if (G_ValidTarget(self, self->enemy, true) 
			&& (entdist(self, self->enemy)<=SENTRY_ATTACK_RANGE))
		{
			minisentry_attack(self);
		}
		else
		{
			self->enemy = NULL;
			if (minisentry_findtarget(self))
			{
				minisentry_attack(self);
			}
			else
			{
				minisentry_idle(self);
				VectorCopy(self->move_angles, self->s.angles);
			}
		}
	}

	self->yaw_speed = temp; // restore original yaw speed
	M_SetEffects(self);
	self->nextthink = level.time + FRAMETIME;


	

}

void base_think (edict_t *self)
{
	// make sure sentry has settled down
	if (!minisentry_checkposition(self))
	{
		BecomeExplosion1(self);
		return;
	}
	self->nextthink = level.time + FRAMETIME;
}

void base_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->creator && self->creator->inuse)
	{
		self->creator->num_sentries--;
		if (self->creator->num_sentries < 0)
			self->creator->num_sentries = 0;
	}
	BecomeExplosion1(self);
}

void base_createturret (edict_t *self)
{
	edict_t		*sentry;
	vec3_t		end;
	trace_t		tr;
	int			casterlevel, talentLevel;
	float		ammo_mult=1.0;

	// make sure sentry has settled down
	if (!G_EntIsAlive(self->creator) || !minisentry_checkposition(self))
	{
		if (self->creator)
		{
			self->creator->num_sentries--;
			if (self->creator->num_sentries < 0)
				self->creator->num_sentries = 0;
		}
		BecomeExplosion1(self);
		return;
	}
	self->movetype = MOVETYPE_NONE; // lock down base
	self->takedamage = DAMAGE_NO; // the base is invulnerable

	// 3.8 base bbox no longer necessary, turret takes over
	VectorClear(self->mins);
	VectorClear(self->maxs);
	self->solid = SOLID_NOT;

	// create basic ent for sentry
	sentry = G_Spawn();
	sentry->creator = self->creator;
	sentry->owner = self; // the base becomes the owner
	VectorCopy(self->s.angles, sentry->s.angles);
	sentry->think = minisentry_think;
	sentry->nextthink = level.time + FRAMETIME;
	sentry->s.modelindex = gi.modelindex ("models/weapons/g_bfg/tris.md2");
	sentry->s.renderfx |= RF_IR_VISIBLE;
	// who really wanted to chase sentries anyway
	// sentry->flags |= FL_CHASEABLE; // 3.65 indicates entity can be chase cammed
	sentry->solid = SOLID_BBOX;
	sentry->movetype = MOVETYPE_NONE;
	sentry->clipmask = MASK_MONSTERSOLID;
	sentry->mass = 100;
	sentry->classname = "msentrygun";
	//sentry->viewheight = 16;
	sentry->takedamage = DAMAGE_AIM;
	sentry->mtype = M_MINISENTRY;
	sentry->touch = minisentry_touch;
	
	sentry->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;

	//Talent: Storage Upgrade
	talentLevel = getTalentLevel(self->creator, TALENT_STORAGE_UPGRADE);
	ammo_mult += 0.2 * talentLevel;

	// set ammo
	sentry->monsterinfo.jumpdn = SENTRY_MAX_AMMO * ammo_mult; // max ammo
	sentry->light_level = sentry->monsterinfo.jumpdn; // current ammo

	//get player's current_level for BUILD_SENTRY
	casterlevel = self->monsterinfo.level;
	sentry->monsterinfo.level = casterlevel; // used for adding monster exp
	sentry->monsterinfo.control_cost = 20; // used for adding monster exp

	//set health
	sentry->health = MINISENTRY_INITIAL_HEALTH + (MINISENTRY_ADDON_HEALTH * casterlevel);
	sentry->monsterinfo.power_armor_power = MINISENTRY_INITIAL_ARMOR + (MINISENTRY_ADDON_ARMOR * casterlevel);
//	if (sentry->health > MINISENTRY_MAX_HEALTH)
//		sentry->health = MINISENTRY_MAX_HEALTH;
	//if (sentry->monsterinfo.power_armor_power > MINISENTRY_MAX_ARMOR)
	//	sentry->monsterinfo.power_armor_power = MINISENTRY_MAX_ARMOR;
	sentry->max_health = sentry->health;
	sentry->monsterinfo.max_armor = sentry->monsterinfo.power_armor_power;

	//set damage
	sentry->dmg = MINISENTRY_INITIAL_BULLET + (MINISENTRY_ADDON_BULLET * casterlevel);// bullet damage
	sentry->radius_dmg = MINISENTRY_INITIAL_ROCKET + (MINISENTRY_ADDON_ROCKET * casterlevel); // rocket damage
	if (sentry->dmg > MINISENTRY_MAX_BULLET)
		sentry->dmg = MINISENTRY_MAX_BULLET;
	if (sentry->radius_dmg > MINISENTRY_MAX_ROCKET)
		sentry->radius_dmg = MINISENTRY_MAX_ROCKET;

	sentry->die = minisentry_die;
	sentry->pain = minisentry_pain;
	sentry->yaw_speed = 5;

	if (self->style == SENTRY_UPRIGHT)
	{
		VectorSet(sentry->mins, -28, -28, -12);
		VectorSet(sentry->maxs, 28, 28, 24);
		VectorCopy(self->s.origin, end);
		//end[2] += self->maxs[2] + sentry->mins[2] + 1;
		end[2] += abs(sentry->mins[2])+1;
	}
	else
	{
		VectorSet(sentry->mins, -28, -28, -24);
		VectorSet(sentry->maxs, 28, 28, 12);
		VectorCopy(self->s.origin, end);
		//end[2] -= abs(self->mins[2]) + sentry->maxs[2] + 1;
		end[2] -= sentry->maxs[2]+1;
	}

	// make sure position is valid
	tr = gi.trace(end, sentry->mins, sentry->maxs, end, sentry, MASK_SHOT);
	if (tr.contents & MASK_SHOT)
	{
		if (self->creator)
		{
			self->creator->num_sentries--;
			if (self->creator->num_sentries < 0)
				self->creator->num_sentries = 0;
		}
		//gi.dprintf("%s\n", tr.ent?tr.ent->classname:"null");
		BecomeExplosion1(self);
		BecomeExplosion1(sentry);
		return;
	}
	VectorCopy(tr.endpos, sentry->s.origin);
	VectorCopy(sentry->s.angles, sentry->move_angles);// save for idle animation
	gi.linkentity(sentry);
	
	gi.sound(sentry, CHAN_VOICE, gi.soundindex("weapons/turrset.wav"), 1, ATTN_NORM, 0);
	self->think = base_think; // base is done creating gun
	self->nextthink = level.time + FRAMETIME;
}

void SpawnMiniSentry (edict_t *ent, int cost, float skill_mult, float delay_mult)
{
	int			sentries=0;//3.9
	qboolean	failed=false;
	vec3_t		offset, forward, right, start, end;
	edict_t		*base;
	trace_t		tr;

	if (debuginfo->value)
		gi.dprintf("DEBUG: SpawnMiniSentry()\n");

	// create basic ent for sentry base
	base = G_Spawn();
	base->activator = base->creator = ent;
	VectorCopy(ent->s.angles, base->s.angles);
	base->s.angles[PITCH] = 0;
	base->s.angles[ROLL] = 0;
	base->think = base_createturret;
	base->nextthink = level.time + SENTRY_BUILD_TIME * delay_mult;
	base->s.modelindex = gi.modelindex ("models/objects/turret/turret_heavy.md2");
	base->solid = SOLID_BBOX;
	base->movetype = MOVETYPE_TOSS;
	base->clipmask = MASK_MONSTERSOLID;
	base->mass = 300;
	base->health = 590;
	base->die = base_die;
	base->classname = "msentrybase";
	base->monsterinfo.level = ent->myskills.abilities[BUILD_SENTRY].current_level * skill_mult;
	base->takedamage = DAMAGE_YES; // need this to let sentry get knocked around
	VectorSet(base->mins, -32, -32, 0);
	VectorSet(base->maxs, 32, 32, 12);

	// calculate starting position
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	VectorMA(start, 128, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	// base spawned in mid-air
	if (tr.fraction == 1.0)
	{	
		base->style = SENTRY_UPRIGHT;
	}
	// base spawned below us
	else if(tr.endpos[2] <= ent->s.origin[2])
	{
		// add the height of the base
		VectorCopy(tr.endpos, end);
		end[2] += abs(base->mins[2]);
		//base->movetype = MOVETYPE_NONE;
		base->style = SENTRY_UPRIGHT;
	}
	// base spawned above us
	else if (tr.ent && (tr.ent == world || tr.ent->mtype == M_LASERPLATFORM))
	{
		// flip base upside-down
		base->movetype = MOVETYPE_NONE;
		base->s.angles[ROLL] += 180;
		if (base->s.angles[ROLL] < 0)
			base->s.angles[ROLL] += 360;
		else if (base->s.angles[ROLL] > 360)
			base->s.angles[ROLL] -= 360;
		VectorSet(base->mins, -32, -32, -12);
		VectorSet(base->maxs, 32, 32, 0);
		// subtract the height of the base
		VectorCopy(tr.endpos, end);
		end[2] -= base->maxs[2];
		base->style = SENTRY_FLIPPED;
	}
	
	// make sure the position is valid
	tr = gi.trace(end, base->mins, base->maxs, end, NULL, MASK_SHOT);
	if (tr.contents & MASK_SHOT)
	{
		G_FreeEdict(base);
		return;
	}
	VectorCopy(tr.endpos, base->s.origin);
	gi.linkentity(base);
	gi.sound(base, CHAN_VOICE, gi.soundindex("weapons/repair.wav"), 1, ATTN_NORM, 0);

	// 3.9 double sentry cost if there are too many sentries in CTF
	if (ctf->value)
	{
		sentries += 2*CTF_GetNumSummonable("Sentry_Gun", ent->teamnum);
		sentries += CTF_GetNumSummonable("msentrygun", ent->teamnum);

		if (sentries > MAX_MINISENTRIES)
			cost *= 2;
	}

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5) 
		|| (ent->mtype == MORPH_CACODEMON && ent->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	ent->client->pers.inventory[power_cube_index] -= cost;
	ent->client->ability_delay = level.time + SENTRY_BUILD_TIME * delay_mult;
	ent->holdtime = level.time + SENTRY_BUILD_TIME * delay_mult;
	ent->num_sentries++;
	ent->lastsound = level.framenum;
}

void RemoveMiniSentries (edict_t *ent)
{
	edict_t *e=NULL;

	while((e = G_Find(e, FOFS(classname), "msentrygun")) != NULL) 
	{
		if (e && e->creator && (e->creator == ent) && !RestorePreviousOwner(e))
			minisentry_remove(e);
	}

	ent->num_sentries = 0;
}

void Cmd_MiniSentry_f (edict_t *ent)
{
	int talentLevel, sentries=0, cost=SENTRY_COST;
	float skill_mult=1.0, cost_mult=1.0, delay_mult=1.0;//Talent: Rapid Assembly & Precision Tuning
	edict_t *scan=NULL;

	if (debuginfo->value)
		gi.dprintf("%s just called Cmd_MiniSentry_f\n", ent->client->pers.netname);

	if (ent->myskills.abilities[BUILD_SENTRY].disable)
		return;

	if (!Q_strcasecmp(gi.args(), "remove"))
	{
		RemoveMiniSentries(ent);
		return;
	}

	// 3.9 double sentry cost if there are too many sentries in CTF
	if (ctf->value)
	{
		sentries += 2*CTF_GetNumSummonable("Sentry_Gun", ent->teamnum);
		sentries += CTF_GetNumSummonable("msentrygun", ent->teamnum);

		if (sentries > MAX_MINISENTRIES)
			cost *= 2;
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

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[BUILD_SENTRY].current_level, cost))
		return;

	if (ent->num_sentries >= MAX_MINISENTRIES)
	{
		safe_cprintf(ent, PRINT_HIGH, "You have reached the max of %d sentries\n", MAX_MINISENTRIES);
		return;
	}

	if (ctf->value && (CTF_DistanceFromBase(ent, NULL, CTF_GetEnemyTeam(ent->teamnum)) < CTF_BASE_DEFEND_RANGE))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't build in enemy base!\n");
		return;
	}

	SpawnMiniSentry(ent, cost, skill_mult, delay_mult);
}
