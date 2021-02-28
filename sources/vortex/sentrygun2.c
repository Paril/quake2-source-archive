//***************
//**Sentry Code**
//***************
//***By Archer***
//***************

#include "g_local.h"

// 3.19 sentry attack and sight FOV limits
#define SENTRY_FOV_SIGHT			60
#define SENTRY_FOV_ATTACK			30

#define SENTRY_TARGET_RANGE			1536	//Maximum distance a player can be targeted (quake2 units)
#define SENTRY_RELOAD_DELAY			2.5		//How often you can reload your sentry (seconds)
#define SENTRY_BUILD_COST			35		//Cost for sentry gun (in power cubes)
#define SENTRY_BUILD_DELAY			1.5		//Delay before other abilities can be used (seconds)
#define SENTRY_MAX_PLAYER_DISTANCE	1536	//Maximum distance a player may be from the gun before it explodes (quake2 units)

#define SENTRY_MODEL_BFG			"models/weapons/g_bfg/tris.md2"
#define SENTRY_MODEL_1				"models/sentry/turret1/tris.md2"
#define SENTRY_MODEL_2				"models/sentry/turret2/tris.md2"
#define SENTRY_MODEL_3				"models/sentry/turret3/tris.md2"

//Basic sentry attributes (in game)
#define SENTRY_HEALTH_MULT			75		//Multiply by myskills.abilities[BUILD_SENTRY].current_level
#define SENTRY_HEALTH_BASE			600	//Base health
#define SENTRY_ARMOR_MULT			35		//Multiply by myskills.abilities[BUILD_SENTRY].current_level
#define SENTRY_ARMOR_BASE			350		//Base armour
#define SENTRY_HEAL_COST			0.05		//Amount of pc's required to heal 1 hp
#define SENTRY_REPAIR_COST			0.05	//Amount of pc's required to heal 1 armor point
#define SENTRY_MAX_UPGRADE_LEVEL	3		//Maximum sentry upgrade level (like it says)
#define	SENTRY_UPGRADE_DAMAGE_MULT	0.9	//Amount of damage each upgrade gives the gun (percent)
//Example: 0.2 == 120% at lvl 2 and 140% at lvl 3 (168% of original damage)
#define SENTRY_UPGRADE_COST			25		//Cost in cubes to upgrade sentry gun

//Aiming / rotating
#define SENTRY_ROTATE_SPEED			5		//How many degrees the gun rotates each idle think
#define SENTRY_ROTATE_DELAY			1		//Time sentry waits before changing direction (seconds)
#define SENTRY_YAW_LIMIT			45		//Absolute value of the max a gun can rotate (horizontal)
//note: must be divisible by SENTRY_ROTATE_SPEED

//Almost all the weapon attributes are now related to the player's
//build_sentry level. These are kept coded in case of future additions where
//the creator will not be a player (should work)

//Bullets
#define SENTRY_BULLET_DAMAGE_LEVEL	56 	//Multiply by myskills.abilities[BUILD_SENTRY].current_level
#define SENTRY_BULLETCOST			1	

//BFG
#define SENTRY_BFG_REFIRE			3		//BFG sentry refire rate (seconds)
#define SENTRY_BFG_DAMAGE_LEVEL		25		//Imitates a player with BFG upgrades (damage)
#define SENTRY_BFG_SPEED_LEVEL		15		//Imitates a player with BFG upgrades (speed)
#define SENTRY_BFG_AMMOCOST			50		//Cost per BFG shot

//Rockets
#define SENTRY_ROCKET_DAMAGE_LEVEL	25		//Imitates a player with RL upgrades (damage)
#define SENTRY_ROCKET_SPEED_LEVEL	15		//Imitates a player with RL upgrades (speed)
#define SENTRY_ROCKET_RADIUS_LEVEL	10		//Imitates a player with RL upgrades (radius)
#define SENTRY_ROCKET_REFIRE		1.0		//Sentry refire rate for rockets only (seconds)
#define SENTRY_ROCKETCOST			1

//Ammo
#define SENTRY_MAX_CELLS			300
#define SENTRY_MAX_ROCKETS			50 
#define SENTRY_MAX_BULLETS			SENTRY_INITIAL_AMMO+SENTRY_ADDON_AMMO*ent->myskills.abilities[BUILD_SENTRY].current_level

/**********
*
*	sentrygun_die()
*
*	Called when the sentry gun is destroyed
*
***********/

void sentrygun_remove(edict_t *self)
{
	if (self->deadflag == DEAD_DEAD)
		return;

	if (self->creator && self->creator->inuse)
		self->creator->num_sentries -= 2;
	if (self->creator->num_sentries < 0)
		self->creator->num_sentries = 0;

	// remove base immediately
	if (self->sentry && self->sentry->inuse)
		G_FreeEdict(self->sentry);

	// prep sentry for removal
	self->think = BecomeExplosion1;
	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->svflags |= SVF_NOCLIENT;
	self->deadflag = DEAD_DEAD;
	self->nextthink = level.time + FRAMETIME;
	gi.unlinkentity(self);
}

void sentrygun_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	gi.sound(self, CHAN_ITEM, gi.soundindex("world/drip3.wav"), 1, ATTN_NORM, 0);

	//Don't crash
	if ((self->creator) && (attacker == self->creator))
	{
		sentrygun_remove(self);
		return;
	}

	//If someone else destroyed your sentry gun
	safe_cprintf(self->creator, PRINT_HIGH, "ALERT: Your sentry gun has been destroyed");
	if (attacker->client)
		safe_cprintf(self->creator, PRINT_HIGH, " by %s!\n", attacker->client->pers.netname);
	else
		safe_cprintf(self->creator, PRINT_HIGH, "!\n");
	sentrygun_remove(self);
}

void sentrygun_pain(edict_t *self, edict_t *other, float kick, int damage)
{
	if (!self->enemy && G_ValidTarget(self, other, true))
		self->enemy = other;

	if (level.time < self->pain_debounce_time)
		return;

	self->pain_debounce_time = level.time + 2;
	gi.sound(self, CHAN_VOICE, gi.soundindex("tank/tnkpain2.wav"), 1, ATTN_NORM, 0);
}

/**********
*
*	sentFireBullet()
*
***********/

void sentFireBullet(edict_t *self)
{
	vec3_t forward, origin;
	int damage;

	damage = ((float)self->orders / 3.0)*(SENTRY_INITIAL_BULLETDAMAGE +
		SENTRY_ADDON_BULLETDAMAGE*self->monsterinfo.level);

	//Aim at target, find bullet origin (mussle of gun)
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, self->maxs[1] + 1, forward, origin);

	//Adjust aim
	MonsterAim(self, -1, 0, false, 0, forward, origin);

	//Fire
	fire_bullet(self, origin, forward, damage, 2 * damage, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_SENTRY);
	gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/plaser.wav"), 1, ATTN_NORM, 0);

	//Graphics
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(self - g_edicts);
	gi.WriteByte(MZ_IONRIPPER | MZ_SILENCED);
	gi.multicast(origin, MULTICAST_PVS);

	//Reset
	self->light_level--;
}


/**********
*
*	sentFireBFG()
*
***********/

/*
void sentFireBFG(edict_t *self)
{
vec3_t forward, origin;
int damage, speed, range;

//Set damage
damage = BFG10K_INITIAL_DAMAGE;
speed = BFG10K_INITIAL_SPEED;
range = BFG10K_INITIAL_RANGE;
if (!self->creator)
{
damage += BFG10K_ADDON_DAMAGE * SENTRY_BFG_DAMAGE_LEVEL;
speed += BFG10K_ADDON_SPEED * SENTRY_BFG_SPEED_LEVEL;
}
else //Set BFG damage Bookmark(Archer)
{
damage *= self->creator->myskills.abilities[BUILD_SENTRY].current_level;
speed *= self->creator->myskills.abilities[BUILD_SENTRY].current_level / 2;
}

//Aim at target, find firing origin (mussle of gun)
AngleVectors(self->s.angles, forward, NULL, NULL);
VectorMA(self->s.origin, self->maxs[1] + 1, forward, origin);
//Adjust aim (aim at the ground)
MonsterAim(self, 1.0, speed, true, 0, forward, origin);

//Fire
fire_bfg(self, origin, forward, damage, speed, range);
gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/plaser.wav"), 1, ATTN_STATIC, 0);

//Graphics
gi.WriteByte (svc_muzzleflash);
gi.WriteShort (self-g_edicts);
gi.WriteByte (MZ_BFG);
gi.multicast (self->s.origin, MULTICAST_PVS);

//Reset
self->delay = level.time + SENTRY_BFG_REFIRE;
self->count -= SENTRY_BFG_AMMOCOST;

}
*

/**********
*
*	sentFireRocket()
*
***********/

void sentFireRocket(edict_t *self)
{
	vec3_t forward, origin;
	int damage, speed, damage_radius;

	damage = SENTRY_INITIAL_ROCKETDAMAGE + SENTRY_ADDON_ROCKETDAMAGE*self->monsterinfo.level;
	speed = SENTRY_INITIAL_ROCKETSPEED + SENTRY_ADDON_ROCKETSPEED*self->monsterinfo.level;
	damage_radius = damage;
	if (damage_radius > 150)
		damage_radius = 150;

	//Aim at target, find firing origin (mussle of gun)
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->s.origin, self->maxs[1] + 1, forward, origin);
	//Adjust aim (aim at the ground)
	MonsterAim(self, -1, speed, true, 0, forward, origin);

	//Fire
	fire_rocket(self, origin, forward, damage, speed, damage_radius, damage);

	//Reset

	// are we affected by holy freeze?
	//if (HasActiveCurse(self, AURA_HOLYFREEZE))
	if (que_typeexists(self->curses, AURA_HOLYFREEZE))
		self->delay = level.time + 2 * SENTRY_ROCKET_REFIRE;
	else if (self->chill_time > level.time)
		self->delay = level.time + (SENTRY_ROCKET_REFIRE * (1 + CHILL_DEFAULT_BASE + CHILL_DEFAULT_ADDON * self->chill_level));
	else
		self->delay = level.time + SENTRY_ROCKET_REFIRE;
	self->style--;
}

/**********
*
*	sentry_findtarget()
*
*	Finds the next valid target for the sentry gun
*
***********/
edict_t *sentry_findtarget(edict_t *self)
{
	edict_t *target = NULL;

	while ((target = findclosestradius(target, self->s.origin, SENTRY_TARGET_RANGE)) != NULL)
	{
		if (!G_ValidTarget(self, target, true))
			continue;
		//if (!infov(self, target, SENTRY_FOV_SIGHT)) // 3.19 reduced sentry FOV
		if (!nearfov(self, target, 0, SENTRY_FOV_SIGHT))
			continue;
		self->enemy = target;
		break;
	}
	if (self->enemy)
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/turrspot.wav"), 1, ATTN_NORM, 0);
	return(target);
}

/**********
*
*	CanTarget()
*
*	Returns true if the target can be shot by the sentry gun
*
***********/

qboolean CanTarget(edict_t *self)
{
	if (G_EntIsAlive(self->enemy) && !OnSameTeam(self, self->enemy) && visible(self, self->enemy))
		return true;
	else return false;
}

/**********
*
*	attack()
*
*	General attack function for the sentry gun
*
***********/

void attack(edict_t *self)
{
	vec3_t v;
	vec3_t angle;
	float chance;

	//Point at target
	VectorSubtract(self->enemy->s.origin, self->s.origin, v);
	VectorNormalize(v);
	vectoangles(v, angle);

	self->yaw_speed *= 2;
	self->ideal_yaw = angle[YAW];

	//if (!infov(self, self->enemy, SENTRY_FOV_ATTACK))
	if (!nearfov(self, self->enemy, 0, SENTRY_FOV_ATTACK))
		return; // 3.19 sentry can't fire outside its FOV

	//Fire
	switch (self->mtype)
	{
	case M_SENTRY:
		//Fire a rocket if there is sufficient ammo
		if ((level.time >= self->delay) && (self->style >= SENTRY_ROCKETCOST) && (self->orders >= 3))
			sentFireRocket(self);
		// are we affected by holy freeze?
		if (que_typeexists(self->curses, AURA_HOLYFREEZE)/*HasActiveCurse(self, AURA_HOLYFREEZE)*/ && !(level.framenum % 2))
			break;
		// chill effect reduces attack rate/refire
		if (self->chill_time > level.time)
		{
			chance = 1 / (1 + CHILL_DEFAULT_BASE + CHILL_DEFAULT_ADDON * self->chill_level);
			if (random() > chance)
				break;
		}
		//Fire a bullet if there is sufficient ammo
		if (self->light_level >= SENTRY_BULLETCOST)
		{
			sentFireBullet(self);

			// firing animation
			if (self->s.frame == 1)
				self->s.frame = 2;
			else
				self->s.frame = 1;
		}
		break;
		/*
		//New bfg firing (wee!)
		case M_BFG_SENTRY:
		if(self->count >= SENTRY_BFG_AMMOCOST)
		{
		if(self->delay <= level.time)
		sentFireBFG(self);
		else if(self->delay - 1 == level.time)
		gi.sound(self, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_STATIC, 0);
		}
		break;
		*/
	}

	//	if(self->s.angles[PITCH] != 0)
	//		self->s.angles[PITCH] = 0;
}

/**********
*
*	checkAmmo()
*
*	Prints a low ammunition warning to the sentry gun's owner
*
***********/

void checkAmmo(edict_t *self)
{
	switch (self->mtype)
	{
	case M_SENTRY:
		if (self->light_level < SENTRY_BULLETCOST)
			safe_cprintf(self->creator, PRINT_HIGH, "ALERT: Your sentry gun is out of bullets.\n");
		else if (self->light_level < 50)	//Less than 50 bullets left
			safe_cprintf(self->creator, PRINT_HIGH, "WARNING: Your sentry gun is low on bullets.\n");

		if (self->style < SENTRY_ROCKETCOST)
			safe_cprintf(self->creator, PRINT_HIGH, "ALERT: Your sentry gun is out of rockets.\n");
		else if (self->style < 10)		//Less than 10 rockets left
			safe_cprintf(self->creator, PRINT_HIGH, "WARNING: Your sentry gun is low on rockets.\n");
		break;
	case M_BFG_SENTRY:
		if (self->count < SENTRY_BFG_AMMOCOST)
			safe_cprintf(self->creator, PRINT_HIGH, "ALERT: Your sentry gun is out of cells.\n");
		else if (self->count < SENTRY_BFG_AMMOCOST * 2 + 1)	//If only 2 shots left
			safe_cprintf(self->creator, PRINT_HIGH, "WARNING: Your sentry gun is low on cells.\n");
		break;
	}
}

/**********
*
*	checkHealth()
*
*	Prints a low health warning to the sentry gun's owner
*
***********/

qboolean checkLowHealth(edict_t *self)
{
	if (((float)self->health / (float)self->max_health < 0.25) ||
		((float)self->monsterinfo.power_armor_power / (float)self->rocket_shots < 0.5))
		return true;
	return false;
}

/**********
*
*	sentSpraySparks()
*
*	Randomly sprays a spark from the sentry gun
*
***********/

void sentSpraySparks(edict_t *self)
{
	vec3_t origin, up, angle;

	VectorCopy(self->s.origin, origin);
	origin[2] += self->maxs[2];
	VectorCopy(self->s.angles, angle);

	angle[ROLL] = GetRandom(0, 200) - 100;
	angle[PITCH] = GetRandom(0, 200) - 100;
	AngleCheck(&angle[ROLL]);
	AngleCheck(&angle[PITCH]);

	AngleVectors(angle, NULL, NULL, up);
	VectorMA(origin, 12, up, origin);

	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_WELDING_SPARKS);
	gi.WriteByte(20);
	gi.WritePosition(origin);
	gi.WriteDir(up);
	gi.WriteByte(0x000000FF);
	gi.multicast(origin, MULTICAST_PVS);

	gi.sound(self, CHAN_VOICE, gi.soundindex("world/spark1.wav"), 1, ATTN_STATIC, 0);
}

/**********
*
*	sentRepair()
*
*	Repairs the sentry gun with player's power cubes
*
***********/

qboolean sentRepair(edict_t *self, edict_t *other)
{
	int client_powercubes = other->client->pers.inventory[power_cube_index];

	//If hp is not at max, repair it
	if (self->health < self->max_health)
	{
		//Exit if sentry is fully repaired
		if ((self->health == self->max_health) && (self->monsterinfo.power_armor_power == self->rocket_shots))
			return false;

		//Exit if player does not have enough power cubes (pc's)
		if (client_powercubes < 1)
			return false;

		//If the player has more than enough pc's to fill up the sentry
		if (client_powercubes * (1 / SENTRY_HEAL_COST) + self->health > self->max_health)
		{
			//Spend just enough pc's to repair the sentry
			client_powercubes -= (self->max_health - self->health) * SENTRY_HEAL_COST;
			self->health = self->max_health;
		}
		else //Use all the player's pc's to repair as much as possible
		{
			self->health += client_powercubes * (1 / SENTRY_HEAL_COST);
			client_powercubes = 0;
		}
	}

	//If armour is not at max, repair it if we have enough cubes
	if ((self->monsterinfo.power_armor_power < self->rocket_shots) && (client_powercubes > 0))
	{

		//If the player has more than enough pc's to fill up the sentry
		if (client_powercubes * (1 / SENTRY_REPAIR_COST) + self->monsterinfo.power_armor_power > self->rocket_shots)
		{
			//Spend just enough pc's to repair the sentry
			client_powercubes -= (self->rocket_shots - self->monsterinfo.power_armor_power) * SENTRY_REPAIR_COST;
			self->monsterinfo.power_armor_power = self->rocket_shots;
		}
		else //Use all the player's pc's to repair as much as possible
		{
			self->monsterinfo.power_armor_power += client_powercubes * (1 / SENTRY_REPAIR_COST);
			client_powercubes = 0;
		}
	}

	//Update power cube count
	if (other->client->pers.inventory[power_cube_index] != client_powercubes)
	{
		other->client->pers.inventory[power_cube_index] = client_powercubes;
		gi.sound(self, CHAN_VOICE, gi.soundindex("weapons/repair.wav"), 1, ATTN_NORM, 0);
		return true;
	}
	return false;
}

/**********
*
*	sentReload()
*
*	Loads the sentry gun with player's ammo
*
***********/

qboolean sentReload(edict_t *self, edict_t *other)
{
	switch (self->mtype)
	{
	case M_SENTRY:
	{
		int client_bullets, client_rockets;
		edict_t *player = other;

		//Point to player's ammo
		client_bullets = player->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))];
		client_rockets = player->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))];

		//Not enough ammo?
		if ((client_rockets < 1) && (client_bullets < 1))
			return false;

		//If gun did not have enough rockets before 
		//it was loaded, reset the RL firing delay
		if ((self->style < 1) && (client_rockets > 0))
			self->delay = level.time + SENTRY_ROCKET_REFIRE;

		//If player has more bullets than needed to fill up the gun
		if (self->light_level + 4 * client_bullets > self->monsterinfo.jumpup)
		{
			client_bullets -= 0.25*(self->monsterinfo.jumpup - self->light_level);
			self->light_level = self->monsterinfo.jumpup;
		}
		else	//Player loads all their bullets into the gun
		{
			self->light_level += 4 * client_bullets;
			client_bullets = 0;
		}

		//If player has more rockets than needed to fill up the gun
		if (self->style + client_rockets > self->monsterinfo.jumpdn)
		{
			client_rockets -= (self->monsterinfo.jumpdn - self->style);
			self->style = self->monsterinfo.jumpdn;
		}
		else	//Player loads all their rockets into the gun
		{
			self->style += client_rockets;
			client_rockets = 0;
		}

		if ((client_rockets != player->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))])
			|| (client_bullets != player->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))]))
		{
			player->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] = client_rockets;
			player->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] = client_bullets;
			return true;
		}
	}
	break;
	/*
	case M_BFG_SENTRY:
	{
	int client_cells;
	edict_t *player = self->creator;

	//If we already have max ammo
	if (self->count == SENTRY_MAX_CELLS)
	return false;

	//Point to player's cells
	client_cells = player->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))];

	//Not enough ammo?
	if (client_cells  < 1)
	return false;

	//If gun did not have enough cells before
	//it was loaded, reset the BFG firing delay
	if (self->count < 50)
	self->delay = level.time + SENTRY_BFG_REFIRE;

	//If player has more cells than needed to fill up the gun
	if (self->count + client_cells > SENTRY_MAX_CELLS)
	{
	client_cells -= (SENTRY_MAX_CELLS - self->count);
	self->count = SENTRY_MAX_CELLS;
	}
	else	//Player loads all their cells into the gun
	{
	self->count += client_cells;
	client_cells = 0;
	}

	if (client_cells != player->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))])
	{
	player->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] = client_cells;
	return true;
	}
	}
	break;
	*/
	}
	return false;
}

/**********
*
*	statusUpdate()
*
*	Prints the status of the sentry gun to the user
*
***********/

void statusUpdate(edict_t *self, edict_t *player)
{
	safe_cprintf(player, PRINT_HIGH, "Health:(%d/%d) ", self->health, self->monsterinfo.power_armor_power);
	safe_cprintf(player, PRINT_HIGH, "Ammo:");
	switch (self->mtype)
	{
	case M_SENTRY:
		safe_cprintf(player, PRINT_HIGH, "(%d/%d)\n", self->light_level, self->style);
		break;
		/*
		case M_BFG_SENTRY:
		safe_cprintf(player, PRINT_HIGH, "(%d)\n",self->count);
		*/
	}
}

/**********
*
*	toofar()
*
*	Returns true if the sentry is too far away from the player
*
***********/

qboolean toofar(edict_t *self, edict_t *player)
{
	vec3_t v; //Vector distance in x, y, z
	float dist;

	VectorSubtract(self->s.origin, player->s.origin, v);
	dist = (float)sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

	if (dist > SENTRY_MAX_PLAYER_DISTANCE)
		return true;

	return false;
}

/**********
*
*	sentUpgrade()
*
*	Sentry gun upgrade function
*
***********/

void sentUpgrade(edict_t *self, edict_t *other)
{
	if (other->client->pers.inventory[power_cube_index] < SENTRY_UPGRADE_COST)
	{
		safe_cprintf(other, PRINT_HIGH, "Not enough cubes to upgrade sentry.\n");
		return;
	}

	switch (self->mtype)
	{
	case M_SENTRY:
	{
		if (self->orders < SENTRY_MAX_UPGRADE_LEVEL)
		{
			self->orders++;
			switch (self->orders)
			{
			case 1: self->s.modelindex = gi.modelindex(SENTRY_MODEL_1); break;
			case 2: self->s.modelindex = gi.modelindex(SENTRY_MODEL_2); break;
			case 3: self->s.modelindex = gi.modelindex(SENTRY_MODEL_3); break;
			}
			/*
			self->health *= 1.5;
			self->max_health = self->health;
			self->rocket_shots = self->rocket_shots * 1.5;
			self->monsterinfo.power_armor_power = self->rocket_shots;
			*/

			other->client->pers.inventory[power_cube_index] -= SENTRY_UPGRADE_COST;
			gi.sound(self, CHAN_ITEM, gi.soundindex("weapons/tnkatck4.wav"), 1, ATTN_STATIC, 0);
		}
		return;
	}
	default: return;
	}
}

/**********
*
*	sentRotate()
*
*	Rotates the sentry (in idle mode)
*
***********/

void sentRotate(edict_t *self)
{
	float min_yaw = (float)(self->move_angles[YAW] - SENTRY_YAW_LIMIT);
	float max_yaw = (float)(self->move_angles[YAW] + SENTRY_YAW_LIMIT);

	self->yaw_speed = SENTRY_ROTATE_SPEED;

	AngleCheck(&min_yaw);
	AngleCheck(&max_yaw);
	AngleCheck(&self->ideal_yaw);

	if (self->wait == 0)
	{
		if (abs(self->s.angles[YAW] - self->ideal_yaw) < SENTRY_ROTATE_SPEED)
			self->s.angles[YAW] = self->ideal_yaw;
		if (self->s.angles[YAW] == self->ideal_yaw)
		{
			//gi.sound(self, CHAN_VOICE, gi.soundindex("plats/pt1_end.wav"), 1, ATTN_STATIC, 0);
			self->wait = level.time + SENTRY_ROTATE_DELAY;
		}
	}
	else if (level.time > self->wait)
	{
		//gi.sound(self, CHAN_VOICE, gi.soundindex("plats/pt1_strt.wav"), 1, ATTN_STATIC, 0);
		gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/gunidle1.wav"), 0.5, ATTN_NORM, 0);
		if (self->ideal_yaw == max_yaw)
			self->ideal_yaw = min_yaw;
		else if ((self->ideal_yaw == min_yaw) || (self->ideal_yaw == self->move_angles[YAW]))
			self->ideal_yaw = max_yaw;
		self->wait = 0;
	}
}

/**********
*
*	sentrygun_think()
*
*	Sentry gun general think function
*
***********/

void sentrygun_think(edict_t *self)
{
	edict_t *target = NULL;
	qboolean damaged = false;
	float temp, modifier;
	que_t *slot = NULL;

	//Explode if it is not supposed to be there
	if (!self->creator || !self->creator->inuse || self->creator->health < 1 || self->creator->num_sentries < 1)
	{
		//	gi.dprintf("creator: %s inuse: %s health %d sentries %d\n", 
		//		self->creator?"true":"false",self->creator->inuse?"true":"false",self->creator->health, self->creator->num_sentries);
		sentrygun_die(self, NULL, self->creator, self->health, self->s.origin);
		return;
	}
	else if (self->removetime > 0)
	{
		qboolean converted = false;

		if (self->flags & FL_CONVERTED)
			converted = true;

		if (level.time > self->removetime)
		{
			// if we were converted, try to convert back to previous owner
			if (!RestorePreviousOwner(self))
			{
				sentrygun_die(self, NULL, self->creator, self->health, self->s.origin);
				return;
			}
		}
		// warn the converted monster's current owner
		else if (converted && self->creator && self->creator->inuse && self->creator->client
			&& (level.time > self->removetime - 5) && !(level.framenum % 10))
			safe_cprintf(self->creator, PRINT_HIGH, "%s conversion will expire in %.0f seconds\n",
			V_GetMonsterName(self), self->removetime - level.time);
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
		modifier = 1 - 0.05*slot->ent->owner->myskills.abilities[HOLY_FREEZE].current_level;
		self->yaw_speed *= modifier;
	}

	// chill effect slows sentry rotation speed
	if (self->chill_time > level.time)
		self->yaw_speed *= 1 / (1 + CHILL_DEFAULT_BASE + CHILL_DEFAULT_ADDON * self->chill_level);

	if (!self->enemy) //If we do not have a target yet
	{
		if (target = sentry_findtarget(self))
			attack(self);
	}
	else if (CanTarget(self))
		attack(self);
	else //Clear target and reset the direction the gun is pointing
	{
		self->enemy = NULL;
		self->ideal_yaw = self->move_angles[YAW];
	}

	damaged = checkLowHealth(self);

	//Every 5 seconds do the following
	if (!(level.framenum % 50))
	{
		checkAmmo(self);
		if (damaged) safe_cprintf(self->creator, PRINT_HIGH, "ALERT: Your sentry gun needs repairs!\n");
	}

	if (damaged)
	{
		//Every 2 seconds put in some sparks
		if (level.framenum % 20 == 0)
			sentSpraySparks(self);
	}

	if (!self->enemy)
	{
		self->s.frame = 0;
		sentRotate(self);
	}

	M_ChangeYaw(self);
	M_SetEffects(self);

	self->yaw_speed = temp; // restore original yaw speed
	//Reset think time
	self->nextthink = level.time + FRAMETIME;













	vec3_t	angles;//4.5 aiming angles are different than model/gun angles
	vec3_t	start, end, forward;
	trace_t tr;


	//	M_Regenerate(self, AUTOCANNON_REGEN_FRAMES, 10, true, false, false);
	M_ChangeYaw(self);



	// trace directly ahead of autocannon
	VectorCopy(self->s.origin, start);
	start[2] += self->viewheight;
	VectorCopy(self->s.angles, angles);//4.5
	angles[PITCH] = self->random;//4.5
	AngleVectors(angles, forward, NULL, NULL);
	VectorMA(start, 1512, forward, end);
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);





	if (self->wait > level.time)
	{
		// draw aiming laser
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_BFG_LASER);
		gi.WritePosition(start);
		gi.WritePosition(tr.endpos);
		gi.multicast(start, MULTICAST_PHS);
	}
}

/**********
*
*	SentryGun_Touch()
*
*	Sentry gun general touch function
*
***********/

void SentryGun_Touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean Repaired_Reloaded = false;	//Has a sentry gun been repaired OR reloaded

	V_Touch(ent, other, plane, surf);

	//Sentry can only be repaired so often
	if (ent->sentrydelay > level.time)
		return;

	/*
	Only owner can do things to the sentry
	(!other->client)		:Must be a player
	(!ent->creator)			:don't crash if the gun's creator is invalid
	(other != ent->creator)	:Touching entity must be the sentry gun's creator
	*/

	//4.2 if this is a player-monster, we should be looking at the client/player
	if (PM_MonsterHasPilot(other))
		other = other->activator;

	if (!other->client)
		return;
	//if (!pvm->value && (other != ent->creator))
	//	return;
	if (!OnSameTeam(ent, other))
		return;

	if (sentRepair(ent, other))
		Repaired_Reloaded = true;
	if (sentReload(ent, other))
		Repaired_Reloaded = true;

	if ((ent->mtype == M_SENTRY) && (ent->orders < 3))
	{
		sentUpgrade(ent, other);
		Repaired_Reloaded = false;
	}

	if (Repaired_Reloaded)	//Tell user he has repaired/reloaded his sentry
	{
		gi.sound(ent, CHAN_ITEM, gi.soundindex("plats/pt1_strt.wav"), 1, ATTN_STATIC, 0);
		safe_cprintf(other, PRINT_HIGH, "Sentry gun repaired/reloaded. ");
	}
	else	//Just print gun status to user
	{
		safe_cprintf(other, PRINT_HIGH, "SENTRY GUN STATUS: ");
	}

	statusUpdate(ent, other);

	ent->sentrydelay = level.time + SENTRY_RELOAD_DELAY;
}

/**********
*
*	sentryStand_think()
*
*	Sentry stand general touch function
*
***********/

void sentryStand_think(edict_t *self)
{
	if ((!self->standowner) || (self->standowner->deadflag == DEAD_DEAD) || (self->standowner->health < 1))
	{
		G_FreeEdict(self);
		return;
	}

	//Always be under the gun
	VectorCopy(self->standowner->s.origin, self->s.origin);
	if (self->standowner->mtype == M_BFG_SENTRY)
		self->s.origin[2] += 12;
	self->nextthink = level.time + FRAMETIME;

	gi.linkentity(self);
}

/**********
*
*	SpawnSentry()
*
*	Spawns a sentry gun
*
***********/

void SpawnSentry1(edict_t *ent, int sentryType, int cost, float skill_mult, float delay_mult)
{
	int		talentLevel, sentries = 0;//3.9
	float	ammo_mult = 1.0;
	vec3_t	angles;
	vec3_t forward, end;

	trace_t tr;

	edict_t *sentry;
	edict_t *sentrystand;

	sentry = G_Spawn();

	//Sentry gun properties
	sentry->activator = sentry->creator = ent;
	sentry->classname = "Sentry_Gun";
	sentry->movetype = MOVETYPE_TOSS;
	sentry->clipmask = MASK_MONSTERSOLID;
	sentry->svflags |= SVF_MONSTER;
	sentry->solid = SOLID_BBOX;
	sentry->mass = 500;
	sentry->takedamage = DAMAGE_YES;
	sentry->viewheight = 16;
	sentry->flags |= FL_CHASEABLE; // 3.65 indicates entity can be chase cammed
	VectorCopy(ent->s.angles, sentry->move_angles);

	//sentry->monsterinfo.level = ent->myskills.abilities[BUILD_SENTRY].current_level * skill_mult;

	//Fix ALL angle values
	AngleCheck(&sentry->move_angles[YAW]);
	sentry->move_angles[PITCH] = 0;
	sentry->move_angles[ROLL] = 0;
	VectorCopy(sentry->move_angles, sentry->s.angles);

	//Think/Die/Touch
	sentry->think = sentrygun_think;
	sentry->nextthink = level.time + 2.0;
	sentry->touch = SentryGun_Touch;
	sentry->sentrydelay = level.time; //Timer used for reloading
	sentry->die = sentrygun_die;
	sentry->pain = sentrygun_pain;

	// used for monster exp
	sentry->monsterinfo.level = ent->myskills.abilities[BUILD_SENTRY].current_level * skill_mult;
	sentry->monsterinfo.control_cost = 20;

	//Health
	sentry->health = sentry->max_health = SENTRY_HEALTH_BASE + SENTRY_HEALTH_MULT * sentry->monsterinfo.level;

	//Armour (max armour is currently stored as rocket_shots)
	//Note: (rocket_shots) was previously used by K03 for homing rockets

	sentry->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;

	sentry->monsterinfo.power_armor_power = sentry->rocket_shots = SENTRY_ARMOR_BASE + SENTRY_ARMOR_MULT * sentry->monsterinfo.level;
	sentry->monsterinfo.max_armor = sentry->monsterinfo.power_armor_power;//GHz

	//Talent: Storage Upgrade
	talentLevel = getTalentLevel(ent, TALENT_STORAGE_UPGRADE);
	ammo_mult += 0.2 * talentLevel;

	//Ammo
	sentry->light_level = SENTRY_INITIAL_AMMO;	//Bullets
	sentry->style = 5;			//Rockets
	sentry->count = 100;		//Cells
	sentry->monsterinfo.jumpdn = SENTRY_MAX_ROCKETS * ammo_mult;
	sentry->monsterinfo.jumpup = SENTRY_MAX_BULLETS * ammo_mult;

	//Upgrade level
	sentry->orders = 1;

	//Rotating
	sentry->yaw_speed = SENTRY_ROTATE_SPEED;
	sentry->ideal_yaw = sentry->move_angles[YAW] + SENTRY_YAW_LIMIT;
	sentry->wait = 0;

	sentry->s.renderfx |= RF_IR_VISIBLE;

	//Sentry Type
	sentry->mtype = sentryType;
	switch (sentry->mtype)
	{
	case M_SENTRY:
		VectorSet(sentry->mins, -20, -20, -40);
		VectorSet(sentry->maxs, 20, 20, 16);
		sentry->s.modelindex = gi.modelindex(SENTRY_MODEL_1);
		break;
		/*
		case M_BFG_SENTRY:
		VectorSet(sentry->mins, -18, -18, -24); //old == -16, -16, -35
		VectorSet(sentry->maxs, 18, 26, 24);	//old == 16, 24, 24
		sentry->s.modelindex = gi.modelindex (SENTRY_MODEL_BFG);
		sentry->health *= 2;
		sentry->max_health = sentry->health;
		sentry->rocket_shots = sentry->rocket_shots * 2;
		sentry->monsterinfo.power_armor_power = sentry->rocket_shots;
		break;
		*/
	default: sentry->s.modelindex = gi.modelindex(SENTRY_MODEL_1);
	}

	//Sentry stand properties
	sentrystand = G_Spawn();
	sentrystand->creator = ent;
	sentrystand->classname = "SentryStand";
	sentrystand->solid = SOLID_NOT;
	sentrystand->mass = 0;
	sentrystand->takedamage = DAMAGE_NO;
	VectorClear(sentrystand->mins);
	VectorClear(sentrystand->maxs);
	//sentrystand->health = 10;
	sentrystand->s.modelindex = gi.modelindex("models/stand/tris.md2");

	//Think/Die/Touch
	sentrystand->think = sentryStand_think;
	sentrystand->nextthink = level.time + FRAMETIME;

	//Link stand with the gun
	sentrystand->standowner = sentry;
	sentry->sentry = sentrystand;

	//Starting position for gun
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(ent->s.origin, 96, forward, end);
	tr = gi.trace(ent->s.origin, NULL, NULL, end, ent, MASK_SOLID);
	VectorCopy(tr.endpos, end);
	vectoangles(tr.plane.normal, angles);
	ValidateAngles(angles);
	// player is aiming at the ground
	if ((tr.fraction != 1.0) && (tr.endpos[2] < ent->s.origin[2]) && (angles[PITCH] == 270))
	{
		//gi.dprintf("aiming at ground\n");
		end[2] += abs(sentry->mins[2]) + 1;
	}
	// make sure sentry doesn't spawn in a solid
	tr = gi.trace(end, sentry->mins, sentry->maxs, end, NULL, MASK_SHOT);
	if (tr.contents & MASK_SHOT)
	{
		safe_cprintf(ent, PRINT_HIGH, "Failed to spawn sentrygun.\n");
		G_FreeEdict(sentry);
		return;
	}

	VectorCopy(end, sentry->s.origin);
	VectorCopy(sentry->s.origin, sentrystand->s.origin);

	//Done
	gi.sound(ent, CHAN_ITEM, gi.soundindex("makron/popup.wav"), 1, ATTN_STATIC, 0);
	gi.linkentity(sentrystand);
	gi.linkentity(sentry);

	// 3.9 double sentry cost if there are too many sentries in CTF
	if (ctf->value)
	{
		sentries += 2 * CTF_GetNumSummonable("Sentry_Gun", ent->teamnum);
		sentries += CTF_GetNumSummonable("msentrygun", ent->teamnum);

		if (sentries > MAX_MINISENTRIES)
			cost *= 2;
	}

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5)
		|| (ent->mtype == MORPH_CACODEMON && ent->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	//Alter player
	ent->num_sentries += 2;
	ent->client->pers.inventory[power_cube_index] -= cost;
	ent->client->ability_delay = level.time + SENTRY_BUILD_DELAY * delay_mult;

	//Player is stuck while creating the sentry gun
	ent->holdtime = level.time + SENTRY_BUILD_DELAY * delay_mult;

	//If you use this ability, you uncloak!
	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->cloaking = false;
	ent->client->cloakable = 0;
}

/**********
*
*	canBuildSentry()
*
*	Returns true if the player is allowed to make a sentry gun
*
***********/

qboolean canBuildSentry(edict_t *ent, int cost)
{
	int sentries = 0;

	//Check for disabled sentry ability
	if (ent->myskills.abilities[BUILD_SENTRY].disable)
		return false;

	// 3.9 double sentry cost if there are too many sentries in CTF
	if (ctf->value)
	{
		sentries += 2 * CTF_GetNumSummonable("Sentry_Gun", ent->teamnum);
		sentries += CTF_GetNumSummonable("msentrygun", ent->teamnum);

		if (sentries > MAX_MINISENTRIES)
			cost *= 2;
	}

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5)
		|| (ent->mtype == MORPH_CACODEMON && ent->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[BUILD_SENTRY].current_level, cost))
		return false;

	//Check if player has too many sentries already
	if (!(ent->num_sentries < SENTRY_MAXIMUM))
	{
		safe_cprintf(ent, PRINT_HIGH, "You have reached the max of %d sentry gun(s).\n", SENTRY_MAXIMUM);
		return false;
	}

	if (ctf->value && (CTF_DistanceFromBase(ent, NULL, CTF_GetEnemyTeam(ent->teamnum)) < CTF_BASE_DEFEND_RANGE))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't build in enemy base!\n");
		return false;
	}
	return true;
}

void rotateSentry(edict_t *ent)
{
	edict_t *e = NULL, *sentry = NULL;
	vec3_t	forward, right, start, offset, end;
	trace_t	tr;

	// locate our sentry
	while ((e = G_Find(e, FOFS(classname), "Sentry_Gun")) != NULL)
	{
		if (e && e->inuse && e->creator && e->creator->client && (e->creator == ent))
		{
			sentry = e;
			break;
		}
	}

	// did we find it?
	if (!sentry)
		return;

	// get start position for trace
	AngleVectors(ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight - 8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	// get end position for trace
	VectorMA(start, 8192, forward, end);

	// get vector to the point client is aiming at and convert to angles
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SOLID);
	VectorSubtract(tr.endpos, sentry->s.origin, forward);
	vectoangles(forward, forward);

	// copy angles to sentrygun
	sentry->move_angles[YAW] = forward[YAW];
	sentry->ideal_yaw = forward[YAW];
	AngleCheck(&sentry->move_angles[YAW]);
	AngleCheck(&sentry->ideal_yaw);
	sentry->wait = 0;

	safe_cprintf(ent, PRINT_HIGH, "Rotating sentrygun...\n");
}

/**********
*
*	cmd_SentryGun()
*
*	Sentry gun command called from g_cmds.c
*
***********/

void cmd_SentryGun(edict_t *ent)
{
	int talentLevel, cost = SENTRY_COST;
	float skill_mult = 1.0, cost_mult = 1.0, delay_mult = 1.0;//Talent: Rapid Assembly & Precision Tuning
	char *arg;
	edict_t *scan = NULL;

	arg = gi.args();

	if (!Q_strcasecmp(arg, "remove"))
	{
		while ((scan = G_Find(scan, FOFS(classname), "Sentry_Gun")) != NULL) {
			if (scan && scan->inuse && scan->creator && scan->creator->client && (scan->creator == ent) && !RestorePreviousOwner(scan)) {
				sentrygun_die(scan, NULL, scan->creator, 10000, scan->s.origin);
				ent->num_sentries = 0;
			}
		}
		return;
	}

	if (!Q_strcasecmp(arg, "rotate"))
	{
		rotateSentry(ent);
		return;
	}

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

	if (!canBuildSentry(ent, cost))
		return;

	SpawnSentry1(ent, M_SENTRY, cost, skill_mult, delay_mult);
}
