#include "g_local.h"
#include "boss.h"

#define TANK_FRAMES_START_STAND		0
#define TANK_FRAMES_END_STAND		29
#define TANK_FRAMES_START_WALK		34
#define TANK_FRAMES_END_WALK		49
#define TANK_FRAMES_BLASTER_START	62// 65 or 64
#define TANK_FRAMES_BLASTER_END		70
#define TANK_FRAMES_START_ROCKET	136
#define TANK_FRAMES_END_ROCKET		144
#define TANK_FRAMES_START_PUNCH		98
#define TANK_FRAMES_END_PUNCH		102
#define TANK_FRAMES_START_BULLET	173
#define TANK_FRAMES_END_BULLET		191


void PM_UpdateChasePlayers (edict_t *ent)
{
	int		i, numPlayers=0;
	edict_t *cl_ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts+1+i;

		if (!cl_ent->inuse)
			continue;
		if (!cl_ent->client->chase_target)
			continue;
		if (!G_IsSpectator(cl_ent))
			continue;

		// swap player's chase target between tank and tank's owner
		if (cl_ent->client->chase_target == ent)
		{
			if (ent->owner && ent->owner->inuse)
				cl_ent->client->chase_target = ent->owner;
		}
	}
}

void PM_Touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	V_Touch(self, other, plane, surf);
}

void PM_RegenAmmo (edict_t *monster, int regen_frames, int delay)
{
	int ammo, mode;
	qboolean regen_rockets=true, regen_bullets=true, regen_cells=true;

	// don't regenerate ammo if player is firing
	if (monster->owner && monster->owner->inuse 
		&& (monster->owner->client->buttons & BUTTON_ATTACK))
	{
		mode = monster->owner->client->weapon_mode;
		if (mode == 0)
			regen_rockets = false;
		else if (mode == 2)
			regen_bullets = false;
	}

	if (delay > 0)
	{
		if (level.framenum < monster->monsterinfo.upkeep_delay)
			return;

		monster->monsterinfo.upkeep_delay = level.framenum + delay;
	}
	else
		delay = 1;

	if (regen_rockets)
	{
		ammo = floattoint( (float) monster->monsterinfo.jumpdn / ((float)regen_frames / delay) );

		if (ammo < 1)
			ammo = 1;

		if (monster->monsterinfo.jumpup < monster->monsterinfo.jumpdn)
		{
			monster->monsterinfo.jumpup += ammo;
			if (monster->monsterinfo.jumpup > monster->monsterinfo.jumpdn)
				monster->monsterinfo.jumpup = monster->monsterinfo.jumpdn;
		}
	}

	if (regen_cells)
	{
		ammo = floattoint( (float) monster->monsterinfo.inv_framenum / ((float)regen_frames / delay) );

		if (ammo < 1)
			ammo = 1;

		if (monster->monsterinfo.radius < monster->monsterinfo.inv_framenum)
		{
			monster->monsterinfo.radius += ammo;
			if (monster->monsterinfo.radius > monster->monsterinfo.inv_framenum)
				monster->monsterinfo.radius = monster->monsterinfo.inv_framenum;
		}
		
		
	}

	if (regen_bullets)
	{
		ammo = floattoint( (float) monster->monsterinfo.search_frames / ((float)regen_frames / delay) );

		if (ammo < 1)
			ammo = 1;

		if (monster->monsterinfo.lefty < monster->monsterinfo.search_frames)
		{
			monster->monsterinfo.lefty += ammo;
			if (monster->monsterinfo.lefty > monster->monsterinfo.search_frames)
				monster->monsterinfo.lefty = monster->monsterinfo.search_frames;
		}

		//gi.dprintf("max: %d current: %d ammo: %d\n", monster->monsterinfo.search_frames, monster->monsterinfo.lefty, ammo);
	}

}

void PM_SyncWithPlayer (edict_t *monster)
{
	int health = monster->health;

	if (!monster->activator || !monster->activator->inuse)
		return;

	// don't kill the player during syncronization; death will be handled
	// in the monster's die function
	if (health < 1)
		health = 1;

	// set player's health to monster's health
	monster->activator->health = health;

	// copy chat-protect status
	if (monster->activator->flags & FL_CHATPROTECT)
	{
		monster->flags |= FL_CHATPROTECT;
		monster->svflags |= SVF_NOCLIENT;//4.5
	}
	else
	{
		monster->flags &= ~FL_CHATPROTECT;
		monster->svflags &= ~SVF_NOCLIENT;//4.5
	}

	// copy godmode flag
	if (monster->activator->flags & FL_GODMODE)
		monster->flags |= FL_GODMODE;
	else
		monster->flags &= ~FL_GODMODE;
}

edict_t *PM_GetPlayer (edict_t *e)
{
	if (!e || !e->inuse)
		return NULL;
	if (e->client)
		return e;
	if (PM_MonsterHasPilot(e))
		return e->activator;
	return NULL;
}

qboolean PM_MonsterHasPilot (edict_t *monster)
{
	// make sure the monster is valid
	if (!monster || !monster->inuse || !monster->mtype || monster->client)
		return false;

	// monster must have an activator that is a client
	if (!monster->activator || !monster->activator->inuse || !monster->activator->client)
		return false;

	// client must have a reciprocal link to monster
	return (monster->activator->owner && (monster->activator->owner == monster));
}

qboolean PM_PlayerHasMonster (edict_t *player)
{
	return (player && player->inuse && player->client && player->owner 
		&& player->owner->inuse && player->owner->mtype);
}

void PM_RemoveMonster (edict_t *player)
{
	if (!player || !player->inuse || !player->client)
		return;

	// don't bother with spectators
	if (player->client->pers.spectator || player->client->resp.spectator)
		return;

	// only clients piloting a monster will legitimately be using noclip
	if (player->movetype != MOVETYPE_NOCLIP)
		return;
	
	// remove the monster provided we have one
	if (player->owner && player->owner->inuse && player->owner->mtype)
		M_Remove(player->owner, false, false);

	// restore the client (move them out of noclip, restore weapon, etc.)
	PM_RestorePlayer(player);
}

void PM_RestorePlayer (edict_t *ent)
{
	//char	userinfo[MAX_INFO_STRING];
	vec3_t start;

	if (!ent->client)
		return;

	// move player to monster's origin
	if (ent->owner && ent->owner->inuse)
	{
		VectorCopy(ent->owner->s.origin, start);
		start[2] += 8;
		VectorCopy(start, ent->s.origin);
	}

	// return player to normal state
	ent->svflags &= ~SVF_NOCLIENT;
	ent->viewheight = 22;
	ent->movetype = MOVETYPE_WALK;
	ent->solid = SOLID_BBOX;
	ent->takedamage = DAMAGE_AIM;

	// recover player info
	Pick_respawnweapon(ent);

	// clear monster pointer
	ent->owner = NULL;
}

// player-monster effects
void PM_Effects (edict_t *ent)
{
	if (ent->client)
		return;
	// in a cocoon
	if (ent->movetype == MOVETYPE_NONE)
		return;

	ent->s.effects = 0;
	ent->s.renderfx = 0;

	if (ent->health < 1)
		return;

	// activator/owner dependant stuff
	if (ent->activator && ent->activator->inuse)
	{
		ent->s.effects = ent->activator->s.effects;
		ent->s.renderfx = ent->activator->s.renderfx;
	}
}

void p_tank_punch (edict_t *self)
{
	int damage;
	trace_t tr;
	edict_t *other=NULL;
	vec3_t	v;

	// tank must be on the ground to punch
	if (!self->groundentity)
		return;

	self->lastsound = level.framenum;
	damage = P_TANK_PUNCH_INITIAL_DMG+P_TANK_PUNCH_ADDON_DMG*self->monsterinfo.level;
	gi.sound (self, CHAN_AUTO, gi.soundindex ("tank/tnkatck5.wav"), 1, ATTN_NORM, 0);
	
	while ((other = findradius(other, self->s.origin, P_TANK_PUNCH_RADIUS)) != NULL)
	{
		if (!G_ValidTarget(self, other, true))
			continue;
		if (!nearfov(self, other, 0, 30))
			continue;

		VectorSubtract(other->s.origin, self->s.origin, v);
		VectorNormalize(v);
		tr = gi.trace(self->s.origin, NULL, NULL, other->s.origin, self, (MASK_PLAYERSOLID | MASK_MONSTERSOLID));
		T_Damage (other, self, self, v, tr.endpos, tr.plane.normal, damage, damage, 0, MOD_TANK_PUNCH);
		other->velocity[2] += damage / 2;
	}
}

int p_tank_getFirePos (edict_t *self, vec3_t start, vec3_t forward)
{
	int mode, flash_number=0;
	vec3_t	right, end, org;
	trace_t tr;

	if (self->style != FRAMES_ATTACK)
		return -1; // we shouldn't be here if we're not trying to attack!

	mode = self->owner->client->weapon_mode;

	if (mode == 1)
		return 0; // punch
	else if (mode == 2) // bullet
		flash_number = MZ2_TANK_MACHINEGUN_1 + (self->s.frame - TANK_FRAMES_START_BULLET);
	else if (mode == 3) // blaster
	{
		if (self->s.frame == 64)
			flash_number = MZ2_TANK_BLASTER_1;
		else if (self->s.frame == 67)
			flash_number = MZ2_TANK_BLASTER_2;
		else
			flash_number = MZ2_TANK_BLASTER_3;
	}
	else // rocket
	{
		if (self->s.frame == 138)
			flash_number = MZ2_TANK_ROCKET_1;
		else if (self->s.frame == 141)
			flash_number = MZ2_TANK_ROCKET_2;
		else if (self->s.frame == 144)
			flash_number = MZ2_TANK_ROCKET_3;
	}

	// get starting (muzzle) position
	AngleVectors(self->s.angles, forward, right, NULL);
	G_ProjectSource(self->s.origin, monster_flash_offset[flash_number], forward, right, start);
	VectorCopy(start, org);

	// get forward vector
	VectorCopy(self->activator->s.origin, start);
	start[2]+=16;
	AngleVectors (self->activator->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, self->activator, MASK_SOLID);
	VectorSubtract(tr.endpos, org, forward);

	VectorNormalize(forward);
	VectorCopy(org, start); // copy muzzle origin to start

	return flash_number;
}

void p_tank_bullet (edict_t *self, vec3_t start, vec3_t forward, int flash_number)
{
	int damage = P_TANK_BULLET_INITIAL_DMG+P_TANK_BULLET_ADDON_DMG*self->monsterinfo.level;
	int kick = damage;

	// bullet ammo counter
	if (!self->monsterinfo.lefty)
		return;
	self->monsterinfo.lefty--;

	fire_bullet(self, start, forward, damage, kick, 225, 375, MOD_TANK_BULLET);

	gi.WriteByte (svc_muzzleflash2);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (flash_number);
	gi.multicast (start, MULTICAST_PVS);
}

void p_tank_rocket (edict_t *self, vec3_t start, vec3_t forward, int flash_number)
{
	int damage = P_TANK_ROCKET_INITIAL_DMG+P_TANK_ROCKET_ADDON_DMG*self->monsterinfo.level;
	int speed = P_TANK_ROCKET_INITIAL_SPD+P_TANK_ROCKET_ADDON_SPD*self->monsterinfo.level;

	// rocket ammo counter
	if (!self->monsterinfo.jumpup)
		return;
	self->monsterinfo.jumpup--;

	fire_rocket(self, start, forward, damage, speed, self->monsterinfo.level + 250, damage);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self-g_edicts);
	gi.WriteByte(flash_number);
	gi.multicast(start, MULTICAST_PVS);
}

void p_tank_blaster (edict_t *self, vec3_t start, vec3_t forward, int flash_number)
{
	int damage = P_TANK_BLASTER_INITIAL_DMG+P_TANK_BLASTER_ADDON_DMG*self->monsterinfo.level;
	int speed = P_TANK_BLASTER_INITIAL_SPD+P_TANK_BLASTER_ADDON_SPD*self->monsterinfo.level;

	// blaster ammo counter
	if (!self->monsterinfo.radius)
		return;
	self->monsterinfo.radius--;

	fire_blaster(self, start, forward, damage, speed, EF_BLASTER, BLASTER_PROJ_BOLT, MOD_BLASTER, 2.0, true);

	gi.WriteByte(svc_muzzleflash2);
	gi.WriteShort(self-g_edicts);
	gi.WriteByte(flash_number);
	gi.multicast(start, MULTICAST_PVS);
}

void p_tank_attack (edict_t *ent)
{
	int		flash_number;
	vec3_t	forward, start;

	if (ent->owner->monsterinfo.attack_finished > level.time)
		return;

	if ((flash_number=p_tank_getFirePos(ent, start, forward))==-1)
		return;

	ent->owner->client->idle_frames = 0; // you're not idle if you're attacking!

	// punch attack
	if (ent->owner->client->weapon_mode==1)
	{
		//begin attack
		if (ent->s.frame == TANK_FRAMES_START_PUNCH)
			gi.sound (ent, CHAN_WEAPON, gi.soundindex ("tank/tnkatck4.wav"), 1, ATTN_NORM, 0);
		else if (ent->s.frame == TANK_FRAMES_END_PUNCH)
			p_tank_punch(ent);
		return;
	}

	if (!flash_number) // not a firing frame
		return;

	// rocket attack
	if (ent->owner->client->weapon_mode==0)
	{
		p_tank_rocket(ent, start, forward, flash_number);
		return;
	}
	// bullet attack
	else if (ent->owner->client->weapon_mode==2)
	{
		p_tank_bullet(ent, start, forward, flash_number);
		return;
	}
	// bullet attack
	else if (ent->owner->client->weapon_mode==3)
	{
		if (ent->s.frame == 64 || ent->s.frame == 67 || ent->s.frame == 70)
			p_tank_blaster(ent, start, forward, flash_number);
		return;
	}
}

void p_tank_idle (edict_t *self)
{
	G_RunFrames(self, TANK_FRAMES_START_STAND, TANK_FRAMES_END_STAND, false);
}

void p_tank_think (edict_t *self)
{
	int	frame, regenFrames=P_TANK_REGEN_FRAMES, regenAmmoFrames=P_TANK_AMMOREGEN_FRAMES;
	int delayFrames=P_TANK_REGEN_DELAY, delayAmmoFrames=P_TANK_AMMOREGEN_DELAY;

	if (!boss_checkstatus(self))
		return;

	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	PM_Effects(self);

	self->monsterinfo.trail_time = level.time + 1; // stay in eye-cam
	M_Regenerate(self, regenFrames, delayFrames,  1.0, true, false, false, &self->monsterinfo.regen_delay1);
	PM_RegenAmmo(self, regenAmmoFrames, delayAmmoFrames);
	PM_SyncWithPlayer(self);

	if (self->style == FRAMES_RUN_FORWARD)
		G_RunFrames(self, TANK_FRAMES_START_WALK, TANK_FRAMES_END_WALK, false);
	else if (self->style == FRAMES_RUN_BACKWARD)
		G_RunFrames(self, TANK_FRAMES_START_WALK, TANK_FRAMES_END_WALK, true);
	else if ((self->style == FRAMES_ATTACK) && (level.time > self->owner->monsterinfo.attack_finished))
	{
		if (self->owner->client->weapon_mode==1)
			G_RunFrames(self, TANK_FRAMES_START_PUNCH, TANK_FRAMES_END_PUNCH, false);
		else if (self->owner->client->weapon_mode==2)
			G_RunFrames(self, TANK_FRAMES_START_BULLET, TANK_FRAMES_END_BULLET, false);
		else if (self->owner->client->weapon_mode==3)
		{
			if (self->s.frame != TANK_FRAMES_BLASTER_END)
				G_RunFrames(self, TANK_FRAMES_BLASTER_START, TANK_FRAMES_BLASTER_END, false);
			else
				self->s.frame = 65; // cycle from this frame forward
		}
		else
			G_RunFrames(self, TANK_FRAMES_START_ROCKET, TANK_FRAMES_END_ROCKET, false);
	}
	else
		p_tank_idle(self);
		
	p_tank_attack(self);

	frame = self->s.frame;

	// play tank step sound
	if (self->groundentity && ((frame == 41) || (frame == 49)))
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("tank/step.wav"), 1, ATTN_NORM, 0);

	self->nextthink = level.time + FRAMETIME;
}

void p_tank_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (self->health < 0.3*self->max_health)
		self->s.skinnum |= 1; // use damaged skin

	// don't play the pain sound too often
	if (level.time < self->pain_debounce_time)
		return;
	
	// play pain sound
	self->pain_debounce_time = level.time + 2;
	gi.sound (self, CHAN_VOICE, gi.soundindex ("tank/tnkpain2.wav"), 1, ATTN_NORM, 0);
}

void p_tank_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	// kill the player
	if (self->activator && self->activator->inuse)
	{
		attacker->lastkill = 0; // prevent 2fer
		PM_RestorePlayer(self->activator);
		player_die(self->activator, inflictor, attacker, 0, vec3_origin);
		self->activator->health = 0;
	}

	// player's chasing this monster should now chase the player instead
	PM_UpdateChasePlayers(self);

	// remove the tank entity
	M_Remove(self, false, false);
}

void p_tank_spawn (edict_t *ent, int cost)
{
	edict_t	*tank;
	vec3_t	boxmin, boxmax;
	trace_t	tr;
	//Talent: More Ammo
	int talentLevel = getTalentLevel(ent, TALENT_MORE_AMMO);

	// make sure we don't get stuck in a wall
	VectorSet (boxmin, -24, -24, -16);
	VectorSet (boxmax, 24, 24, 64);
	tr = gi.trace(ent->s.origin, boxmin, boxmax, ent->s.origin, ent, MASK_SHOT);
	if (tr.fraction<1)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not enough room to morph!\n");
		return;
	}

	V_ModifyMorphedHealth(ent, P_TANK, true);

	// create the tank entity that the player will pilot
	tank = G_Spawn();
	tank->classname = "player_tank";
	tank->solid = SOLID_BBOX;
	tank->takedamage = DAMAGE_YES;
	tank->movetype = MOVETYPE_STEP;
	tank->clipmask = MASK_MONSTERSOLID;
	tank->svflags |= SVF_MONSTER;
	tank->activator = ent;
	tank->die = p_tank_die;
	tank->think = p_tank_think;
	tank->mass = 500;
	tank->monsterinfo.level = ent->myskills.abilities[TANK].current_level; // tank level=ability level
	tank->health = ent->health;
	tank->max_health = ent->max_health;//MAX_HEALTH(ent);
	tank->mtype = P_TANK;
	tank->pain = p_tank_pain;
	tank->touch = PM_Touch;
	tank->monsterinfo.cost = P_TANK_INIT_COST;
	tank->flags |= FL_CHASEABLE; // 3.65 indicates entity can be chase cammed
	tank->monsterinfo.jumpup = P_TANK_SPAWN_ROCKETS; // starting rockets
	tank->monsterinfo.jumpdn = P_TANK_INITIAL_ROCKETS+P_TANK_ADDON_ROCKETS*tank->monsterinfo.level; // max rocket ammo
	tank->monsterinfo.lefty = P_TANK_SPAWN_BULLETS; // starting bullets
	tank->monsterinfo.search_frames = P_TANK_INITIAL_BULLETS+P_TANK_ADDON_BULLETS*tank->monsterinfo.level; // max bullets
	tank->monsterinfo.radius = P_TANK_SPAWN_CELLS; // starting cells
	tank->monsterinfo.inv_framenum = P_TANK_INITIAL_CELLS+P_TANK_ADDON_CELLS*tank->monsterinfo.level; // max cells

//	if (tank->monsterinfo.jumpdn > P_TANK_MAX_AMMO)
//		tank->monsterinfo.jumpdn = P_TANK_MAX_AMMO;

	// Talent: More Ammo
	// increases ammo 10% per talent level
	if(talentLevel > 0)
	{
		tank->monsterinfo.jumpdn *= 1.0 + 0.1*talentLevel;
		tank->monsterinfo.search_frames *= 1.0 + 0.1*talentLevel;
		tank->monsterinfo.inv_framenum *= 1.0 + 0.1*talentLevel;
	}

	if (tank->health < 0.3*tank->max_health)
		tank->s.skinnum |= 1; // use damaged skin

	// set up pointers
	tank->owner = ent;
	ent->owner = tank;
	ent->activator = tank;
	
	tank->s.modelindex = gi.modelindex ("models/monsters/tank/tris.md2");
	//VectorSet (tank->mins, -24, -24, -16);
	//VectorSet (tank->maxs, 24, 24, 64);
	VectorCopy(boxmin, tank->mins);
	VectorCopy(boxmax, tank->maxs);

	// Skin Commander to Poltergeist class when Teleport and Tank abilities are combined
	if( ent->myskills.abilities[TANK].current_level >= 20 )
	{
		tank->s.skinnum = 2; // commander skin
	}
	
	VectorCopy(ent->s.angles, tank->s.angles);
	tank->s.angles[PITCH] = 0; // monsters don't use pitch
	tank->nextthink = level.time + FRAMETIME;
	VectorCopy(ent->s.origin, tank->s.origin);
	VectorCopy(ent->s.old_origin, tank->s.old_origin);

	// link up entities
	gi.linkentity(tank);
	gi.linkentity(ent);

	// make the player into a ghost
	ent->svflags |= SVF_NOCLIENT;
	ent->viewheight = 0;
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->takedamage = DAMAGE_NO;
	ent->client->ps.gunindex = 0;
	//memset (ent->client->pers.inventory, 0, sizeof(ent->client->pers.inventory));
	ent->client->pers.weapon = NULL;

	VortexRemovePlayerSummonables(ent);

	tank->monsterinfo.attack_finished = level.time + 0.5;// can't attack immediately

	// player's chasing the player should now chase the monster instead
	PM_UpdateChasePlayers(ent);

	ent->client->pers.inventory[power_cube_index] -= cost;

	ent->client->refire_frames = 0; // reset charged weapon
	ent->client->weapon_mode = 0; // reset weapon mode
	lasersight_off(ent);

	gi.sound (ent, CHAN_WEAPON, gi.soundindex("spells/morph.wav") , 1, ATTN_NORM, 0);
}

void Cmd_PlayerToTank_f (edict_t *ent)
{
	int tank_cubecost = P_TANK_INIT_COST;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_PlayerToTank_f()\n", ent->client->pers.netname);

	// try to switch back
	if (ent->mtype || ent->owner)
	{
		// don't let a player-tank unmorph if they are cocooned
		if (ent->owner && ent->owner->inuse && ent->owner->movetype == MOVETYPE_NONE)
			return;

		if (que_typeexists(ent->curses, 0))
		{
			if (ent->client) // send message directly to client
				safe_cprintf(ent, PRINT_HIGH, "You can't morph while cursed!\n");
			else if (ent->owner && ent->owner->inuse && ent->owner->client) // send message to pilot
				safe_cprintf(ent, PRINT_HIGH, "You can't morph while cursed!\n");
			return;
		}

		V_RestoreMorphed(ent, 0);
		return;
	}

	//Talent: Morphing
	if(getTalentSlot(ent, TALENT_MORPHING) != -1)
		tank_cubecost *= 1.0 - 0.2 * getTalentLevel(ent, TALENT_MORPHING);

	if (!V_CanUseAbilities(ent, TANK, tank_cubecost, true))
		return;

	if (HasFlag(ent) && !hw->value)
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't morph while carrying flag!\n");
		return;
	}

	//ent->client->pers.inventory[power_cube_index] -= tank_cubecost;
	p_tank_spawn(ent, tank_cubecost);
}