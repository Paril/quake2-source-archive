#include "g_local.h"
#include "morph.h"

#define FLYER_FRAMES_STAND_START	13
#define FLYER_FRAMES_STAND_END		57
#define FLYER_FRAMES_BANK_R_START	96
#define FLYER_FRAMES_BANK_R_END		99//102
#define FLYER_FRAMES_BANK_L_START	103
#define FLYER_FRAMES_BANK_L_END		106//109

#define FLYER_BRAKE_SPEED			5
#define FLYER_ACCEL_SPEED			20
#define FLYER_MAX_VELOCITY			400
#define FLYER_ROCKET_LOCKFRAMES		5
//#define FLYER_ROCKET_MAX_TURNRATE	9	// number of degrees per frame rocket will turn
//#define FLYER_ROCKET_MAX_LOCKFRAMES	15	// number of locked frames to achieve maximum turn rate


void FlyerBrakeVertical (edict_t *ent)
{
	if (ent->velocity[2] > 0)
	{
		ent->velocity[2] -= FLYER_BRAKE_SPEED;
		if (ent->velocity[2] < 0)
			ent->velocity[2] = 0;
	}
	else if (ent->velocity[2] < 0)
	{
		ent->velocity[2] += FLYER_BRAKE_SPEED;
		if (ent->velocity[2] > 0)
			ent->velocity[2] = 0;
	}
}

void FlyerBrakeHorizontal (edict_t *ent)
{
	if (ent->velocity[0] > 0)
	{
		ent->velocity[0] -= FLYER_BRAKE_SPEED;
		if (ent->velocity[0] < 0)
			ent->velocity[0] = 0;
	}
	else if (ent->velocity[0] < 0)
	{
		ent->velocity[0] += FLYER_BRAKE_SPEED;
		if (ent->velocity[0] > 0)
			ent->velocity[0] = 0;
	}

	if (ent->velocity[1] > 0)
	{
		ent->velocity[1] -= FLYER_BRAKE_SPEED;
		if (ent->velocity[1] < 0)
			ent->velocity[1] = 0;
	}
	else if (ent->velocity[1] < 0)
	{
		ent->velocity[1] += FLYER_BRAKE_SPEED;
		if (ent->velocity[1] > 0)
			ent->velocity[1] = 0;
	}
}

void FlyerAccelerate (edict_t *ent, vec3_t dir, int speed, int max_speed)
{
	float	cspd, nspd, value, max;
	vec3_t	move;

	VectorMA(ent->velocity, speed, dir, move);
	nspd = VectorLength(move); // new velocity
	cspd = VectorLength(ent->velocity); // current velocity
	value = max_speed-cspd; // delta between max velocity and new velocity
	max = 2*CS_AIRACCEL; // maximum brake speed

	if ((speed > 0) && (speed > value) && (nspd > cspd))
	{
		if (value > -max)
			speed = value;
		else
			speed = -max;
		// get opposite vector
		VectorCopy(ent->velocity, dir);
		VectorNormalize(dir);
	}
	else if ((speed < 0) && (-speed > value) && (nspd > cspd))
	{
		if (value > -max)
			speed = value;
		else
			speed = -max;
		// get opposite vector
		VectorCopy(ent->velocity, dir);
		VectorNormalize(dir);
	}
	VectorCopy(ent->velocity, move);
	VectorMA(move, speed, dir, move);
	VectorCopy(move, ent->velocity);
}

void FlyerVerticalThrust (edict_t *ent, int speed, int max_speed)
{
	float delta, max, cspd, nspd;

	max = 2*CS_AIRACCEL; // absolute maximum acceleration
	cspd = ent->velocity[2]; // current velocity
	nspd = cspd+speed; // new velocity
	delta = max_speed-fabs(cspd); // difference between max speed and current velocity

	if (speed > 0) // player wants to go up
	{
		if ((delta > speed) || (nspd < 0)) // we're going slower than our max
			ent->velocity[2] += speed;
		else if (delta > 0) // we've almost reached top speed
			ent->velocity[2] += delta;
		else if (delta < 0) // we're going too fast, so slow us down!
		{
			if (delta < -max) // we're way over out limit
				ent->velocity[2] -= max;
			else
				ent->velocity[2] += delta;
		}
	}
	else // player wants to go down
	{
		if ((delta > -speed) || (nspd > 0)) // we're going slower than our max
			ent->velocity[2] += speed;
		else if (delta > 0) // we've almost reached top speed
			ent->velocity[2] -= delta;
		else if (delta < 0) // we're going too fast, so slow us down!
		{
			if (delta < -max) // we're way over out limit
				ent->velocity[2] += max;
			else
				ent->velocity[2] += delta;
		}
	}
}

void PlayerAutoThrust (edict_t *ent, usercmd_t *ucmd)
{
	vec3_t	forward, right;
	int max_velocity = FLYER_MAX_VELOCITY;

	AngleVectors(ent->s.angles, forward, right, NULL);

	if (ucmd->upmove > 0) // up
		FlyerVerticalThrust(ent, FLYER_ACCEL_SPEED*2, max_velocity);
	else if (ucmd->upmove < 0) // down
		FlyerVerticalThrust(ent, -FLYER_ACCEL_SPEED*2, max_velocity);
	else
		FlyerBrakeVertical(ent);

	if (ucmd->forwardmove > 0) // forward
		FlyerAccelerate(ent, forward, FLYER_ACCEL_SPEED, max_velocity);
	else if (ucmd->forwardmove < 0) // backward
		FlyerAccelerate(ent, forward, -FLYER_ACCEL_SPEED, max_velocity);

	if (ucmd->sidemove > 0) // right
		FlyerAccelerate(ent, right, FLYER_ACCEL_SPEED, max_velocity);
	else if (ucmd->sidemove < 0) // left
		FlyerAccelerate(ent, right, -FLYER_ACCEL_SPEED, max_velocity);

	if (!ucmd->forwardmove && !ucmd->sidemove)
		FlyerBrakeHorizontal(ent);
}

void FlyerCheckForImpact (edict_t *ent)
{
	float	speed;

	if (ent->myskills.abilities[WORLD_RESIST].current_level > 0)
		return;

	speed = VectorLength(ent->velocity);
	if (ent->client->oldspeed-speed > FLYER_IMPACT_VELOCITY) // check for drastic decelleration
	{
		int flyer_selfdamage = FLYER_IMPACT_DAMAGE;

		gi.sound (ent, CHAN_AUTO, gi.soundindex ("tank/thud.wav"), 1, ATTN_NORM, 0);
		T_Damage(ent, ent, ent, vec3_origin, ent->s.origin, 
			vec3_origin, FLYER_IMPACT_DAMAGE, flyer_selfdamage, 0, 0); // damage self
		T_RadiusDamage(ent, ent, FLYER_IMPACT_DAMAGE, ent, 64, 0); // damage others
	}
	ent->client->oldspeed = speed;
}

void FlyerAttack (edict_t *ent)
{
	int		damage, radius, speed;
	//float	value;
	vec3_t	forward, right, start, end, offset;
	vec3_t	minbox={-4,-4,-4}, maxbox={4,4,4};
	trace_t	tr;

	ent->client->idle_frames = 0;

	if (ent->client->weapon_mode)
	{
		if (!ent->client->refire_frames)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex ("weapons/prefire.wav"), 1, ATTN_IDLE, 0);
		ent->client->refire_frames++;

		// get muzzle origin and forward vector
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 0, 8,  ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		// trace from firing origin to find a target
		VectorMA(start, 8192, forward, end);
		tr = gi.trace(start, minbox, maxbox, end, ent, MASK_SHOT);

		//value = (float) FLYER_ROCKET_MAX_TURNRATE/FLYER_ROCKET_MAX_LOCKFRAMES;

		// we've got a target in our sights
		if (G_ValidTarget(ent, tr.ent, false))
		{
			// if we don't already have a previous target, or if our new target is
			// not the same as our old one, then use our new target and reset the
			// lock counter
			if (!ent->client->lock_target || (ent->client->lock_target != tr.ent))
			{
				ent->client->lock_target = tr.ent;
				ent->client->lock_frames = 1;
			}
			else
			{
				// we're locked onto the same target, so increment the lock counter
				ent->client->lock_frames++;

				// we have a lock
				if (ent->client->lock_frames == SMARTROCKET_LOCKFRAMES)
				{
					// let them know the rocket will seek
					gi.centerprintf(ent, "Target locked\n");
					// warn their enemy
					if (tr.ent->client)
						gi.centerprintf(tr.ent, "Incoming rocket\n");
				}
			}
		}

		if (level.time >= ent->monsterinfo.attack_finished && ent->client->refire_frames >= FLYER_ROCKET_PREFIRE_FRAMES)
		{
			damage = FLYER_ROCKET_INITIAL_DMG+FLYER_ROCKET_ADDON_DMG*ent->myskills.abilities[FLYER].current_level;
			radius = FLYER_ROCKET_INITIAL_RADIUS+FLYER_ROCKET_ADDON_RADIUS*ent->myskills.abilities[FLYER].current_level;

			// check for adequate ammo
			if (ent->myskills.abilities[FLYER].ammo < FLYER_ROCKET_AMMO)
				return;
			ent->myskills.abilities[FLYER].ammo -= FLYER_ROCKET_AMMO;


			AngleVectors(ent->client->v_angle, forward, NULL, NULL);
			
			// if a lock has been established, report the quality of lock
			if (ent->client->lock_frames >= SMARTROCKET_LOCKFRAMES)
			{
				int	quality = 10 * (ent->client->lock_frames - (SMARTROCKET_LOCKFRAMES - 1));
				if (quality > 100)
					quality = 100;
				gi.centerprintf(ent, "Lock quality: %d%c", quality, '%');
			}

			// fire smart rocket
			fire_smartrocket(ent, ent->client->lock_target, ent->s.origin, forward, damage, 
				FLYER_ROCKET_SPEED, ent->client->lock_frames, radius, damage);

			ent->client->refire_frames = 0;
			ent->client->lock_frames = 0;
			ent->client->lock_target = NULL;

			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/srocket_launch.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->client->refire_frames = 0; // changing weapon modes resets secondary weapon
	ent->client->lock_frames = 0;
	ent->client->lock_target = NULL;

	if (level.time >= ent->monsterinfo.attack_finished
		&& level.framenum >= ent->monsterinfo.stuck_frames) // used to avoid rounding errors
	{
		// check for adequate ammo
		if (ent->myskills.abilities[FLYER].ammo < FLYER_HB_AMMO)
			return;

		ent->myskills.abilities[FLYER].ammo -= FLYER_HB_AMMO;

		damage = FLYER_HB_INITIAL_DMG+FLYER_HB_ADDON_DMG*ent->myskills.abilities[FLYER].current_level;
		speed = FLYER_HB_SPEED;

		AngleVectors(ent->client->v_angle, forward, right, NULL);

		G_ProjectSource(ent->s.origin, monster_flash_offset[MZ2_FLYER_BLASTER_1], forward, right, start);
		fire_blaster(ent, start, forward, damage, speed, EF_HYPERBLASTER, BLASTER_PROJ_BOLT, MOD_HYPERBLASTER, 2.0, false);

		G_ProjectSource(ent->s.origin, monster_flash_offset[MZ2_FLYER_BLASTER_2], forward, right, start);
		fire_blaster(ent, start, forward, damage, speed, EF_HYPERBLASTER, BLASTER_PROJ_BOLT, MOD_HYPERBLASTER, 2.0, false);

		gi.sound (ent, CHAN_WEAPON, gi.soundindex ("weapons/disruptor.wav"), 1, ATTN_NORM, 0);
		//ent->monsterinfo.attack_finished = level.time + (FRAMETIME * FLYER_HB_REFIRE_FRAMES);//level.framenum + FLYER_HB_REFIRE_FRAMES;
		ent->monsterinfo.stuck_frames = level.framenum + FLYER_HB_REFIRE_FRAMES;
	}
}

void MorphRegenerate (edict_t *ent, int regen_delay, int regen_frames)
{
	int	amt, frames;

	if (ent->mtype == MORPH_FLYER)
		V_RegenAbilityAmmo(ent, FLYER, FLYER_HB_REGEN_FRAMES, FLYER_HB_REGEN_DELAY);
	else if (ent->mtype == MORPH_CACODEMON)
		V_RegenAbilityAmmo(ent, CACODEMON, CACODEMON_SKULL_REGEN_FRAMES, CACODEMON_SKULL_REGEN_DELAY);

	if ((level.framenum >= ent->monsterinfo.control_cost)
		&& (ent->health < ent->max_health))
	{
		// calculate healing cycle
		frames = regen_frames;
		if (ent->myskills.abilities[MORPH_MASTERY].current_level > 0)
			frames *= 0.66;

		// calculate regen amount based on cycle duration
		amt = floattoint((float)ent->max_health / ((float)frames / regen_delay));
		if (amt < 1)
			amt = 1;

		// add the health
		ent->health += amt;
		if (ent->health > ent->max_health)
			ent->health = ent->max_health;

		ent->monsterinfo.control_cost = level.framenum + regen_delay;
	}
}

void RunFlyerFrames (edict_t *ent, usercmd_t *ucmd)
{
	// don't run frames if we are dead or aren't a flyer
	if ((ent->mtype != MORPH_FLYER) || (ent->deadflag == DEAD_DEAD))
		return;

	ent->s.modelindex2 = 0; // no weapon model
	ent->s.skinnum = 0;

	FlyerCheckForImpact(ent); // this needs to run every client frame to be accurate

	if (level.framenum >= ent->count) // don't run this more than once a server frame
	{
		PlayerAutoThrust(ent, ucmd);
		MorphRegenerate(ent, FLYER_REGEN_DELAY, FLYER_REGEN_FRAMES);
		//FlyerCheckForImpact(ent);

		ent->count = level.framenum + 1;

		if (ent->client->buttons & BUTTON_ATTACK)
		{
			FlyerAttack(ent);
		}
		else
		{
			ent->client->refire_frames = 0; // reset for secondary attack charge-up
			ent->client->lock_frames = 0; // reset lock counter
			ent->client->lock_target = NULL;
		}

		if (ucmd->sidemove > 0) // right
		{
			if (ent->s.frame == FLYER_FRAMES_BANK_R_END) // done with bank animation
				return; // so don't update the frames
			G_RunFrames(ent, FLYER_FRAMES_BANK_R_START, FLYER_FRAMES_BANK_R_END, false);
		}
		else if (ucmd->sidemove < 0) // left
		{
			if (ent->s.frame == FLYER_FRAMES_BANK_L_END)
				return;
			G_RunFrames(ent, FLYER_FRAMES_BANK_L_START, FLYER_FRAMES_BANK_L_END, false);
		}
		else // default
			G_RunFrames(ent, FLYER_FRAMES_STAND_START, FLYER_FRAMES_STAND_END, false);
	}
}

void Cmd_PlayerToFlyer_f (edict_t *ent)
{
	int flyer_cubecost = FLYER_INIT_COST;
	//Talent: More Ammo
	int talentLevel = getTalentLevel(ent, TALENT_MORE_AMMO);

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_PlayerToFlyer_f()\n", ent->client->pers.netname);

	// try to switch back
	if (ent->mtype || PM_PlayerHasMonster(ent))
	{
		// don't let a player-tank unmorph if they are cocooned
		if (ent->owner && ent->owner->inuse && ent->owner->movetype == MOVETYPE_NONE)
			return;

		if (que_typeexists(ent->curses, 0))
		{
			safe_cprintf(ent, PRINT_HIGH, "You can't morph while cursed!\n");
			return;
		}

		V_RestoreMorphed(ent, 0);
		return;
	}

	//Talent: Morphing
	if(getTalentSlot(ent, TALENT_MORPHING) != -1)
		flyer_cubecost *= 1.0 - 0.25 * getTalentLevel(ent, TALENT_MORPHING);

	//if (!G_CanUseAbilities(ent, ent->myskills.abilities[FLYER].current_level, flyer_cubecost))
	//	return;
	if (!V_CanUseAbilities(ent, FLYER, flyer_cubecost, true))
		return;

	if (HasFlag(ent) && !hw->value)
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't morph while carrying flag!\n");
		return;
	}

	ent->monsterinfo.attack_finished = level.time + 0.5;// can't attack immediately

	ent->client->pers.inventory[power_cube_index] -= flyer_cubecost;

	ent->mtype = MORPH_FLYER;
	ent->s.modelindex = gi.modelindex ("models/monsters/flyer/tris.md2");
	ent->s.modelindex2 = 0;
	ent->s.skinnum = 0;

	VectorSet(ent->mins, -16, -16, -24);
	VectorSet(ent->maxs, 16, 16, 8);
	ent->viewheight = 0;

	// set maximum hyperblaster ammo
	ent->myskills.abilities[FLYER].max_ammo = FLYER_HB_INITIAL_AMMO+FLYER_HB_ADDON_AMMO
		*ent->myskills.abilities[FLYER].current_level;

	// Talent: More Ammo
	// increases ammo 10% per talent level
	if(talentLevel > 0) ent->myskills.abilities[FLYER].max_ammo *= 1.0 + 0.1*talentLevel;

	// give them some starting ammo
	ent->myskills.abilities[FLYER].ammo = FLYER_HB_START_AMMO;

	ent->client->refire_frames = 0; // reset charged weapon
	ent->client->weapon_mode = 0; // reset weapon mode

	// pick null gun
	ent->client->pers.weapon = NULL;
	ent->client->ps.gunindex = 0;

	lasersight_off(ent);

	gi.sound (ent, CHAN_WEAPON, gi.soundindex("spells/morph.wav") , 1, ATTN_NORM, 0);
}