#include "g_local.h"
#include "boss.h"

#define PVB_BOSS_EXPERIENCE		1000
#define PVB_BOSS_MIN_EXP		100
#define PVB_BOSS_MAX_EXP		1000
#define PVB_BOSS_CREDITS		3000
#define PVB_BOSS_FRAG_EXP		300
#define PVB_BOSS_FRAG_CREDITS	100
#define PVB_BOSS_TIMEOUT		10

qboolean Boss_CanFit (edict_t *ent, vec3_t boss_mins, vec3_t boss_maxs)
{
	trace_t tr;

	tr = gi.trace(ent->s.origin, boss_mins, boss_maxs, ent->s.origin, ent, MASK_SHOT);
	if (tr.fraction < 1)
		return false;
	return true;
}

edict_t *Boss_CreateTank (void)
{
	edict_t *tank;

	// create the tank entity that the player will pilot
	tank = G_Spawn();
	tank->classname = "boss";
	tank->solid = SOLID_BBOX;
	tank->takedamage = DAMAGE_YES;
	tank->movetype = MOVETYPE_STEP;
	tank->clipmask = MASK_MONSTERSOLID;
	tank->svflags |= SVF_MONSTER;
//	tank->activator = ent;
	tank->die = boss_tank_die;
	tank->think = boss_tank_think;
	tank->mass = 500;
	tank->monsterinfo.level = average_player_level;
	tank->health = (TANK_INITIAL_HEALTH * 4 +TANK_ADDON_HEALTH*tank->monsterinfo.level);
	tank->max_health = tank->health;
	tank->mtype = BOSS_TANK;
	tank->pain = boss_pain;
	tank->flags |= FL_CHASEABLE;
	// set up pointers
//	tank->owner = ent;
//	ent->owner = tank;
	
	tank->s.modelindex = gi.modelindex ("models/monsters/tank/tris.md2");
	VectorSet (tank->mins, -24, -24, -16);
	VectorSet (tank->maxs, 24, 24, 64);
	tank->s.skinnum = 2; // commander skin
	//VectorCopy(ent->s.angles, tank->s.angles);
	//tank->s.angles[PITCH] = 0; // monsters don't use pitch
	tank->nextthink = level.time + FRAMETIME;
//	VectorCopy(ent->s.origin, tank->s.origin);
//	VectorCopy(ent->s.old_origin, tank->s.old_origin);

	// link up entities
	gi.linkentity(tank);
//	gi.linkentity(ent);

	//gi.dprintf("DEBUG: Boss entity created\n");
	return tank;
}

void Boss_ReadyPlayer (edict_t *player, edict_t *boss)
{
	if (!boss || !player)
		return;

	//gi.dprintf("getting boss ready\n");

	boss->activator = player;
	boss->owner = player;
	VectorCopy(player->s.angles, boss->s.angles);
	boss->s.angles[PITCH] = 0;
	VectorCopy(player->s.origin, boss->s.origin);
	VectorCopy(player->s.old_origin, boss->s.old_origin);
	gi.linkentity(boss);

	//gi.dprintf("getting player ready\n");

	player->owner = boss;

	// make the player into a ghost
	player->svflags |= SVF_NOCLIENT;
	player->viewheight = 0;
	player->movetype = MOVETYPE_NOCLIP;
	player->solid = SOLID_NOT;
	player->takedamage = DAMAGE_NO;
	player->client->ps.gunindex = 0;
	memset (player->client->pers.inventory, 0, sizeof(player->client->pers.inventory));
	player->client->pers.weapon = NULL;

	G_PrintGreenText(va("A level %d boss known as %s has spawned!", boss->monsterinfo.level, player->client->pers.netname));
	//gi.dprintf("DEBUG: player and boss ready\n");
}

qboolean BossExists (void)
{
	int		i;
	edict_t	*player;

	// if a player is piloting a boss, return true
	for (i=0; i<game.maxclients; i++) 
	{
		player = g_edicts+1+i;
		if (player && player->inuse && player->owner 
			&& player->owner->inuse && IsABoss(player->owner))
			return true;
	}
	return false;
}

static edict_t	*SelectedBossPlayer;
static float	boss_timeout;

void CreateBoss (edict_t *ent)
{
	edict_t *boss;
	//gi.dprintf("DEBUG: Waiting for player to become valid...\n");
	
	boss = Boss_CreateTank(); // create the boss entity

	// ready the player and boss if the player is alive and there
	// is enough room to spawn the boss
	if (G_EntIsAlive(ent) 
		&& Boss_CanFit(ent, boss->mins, boss->maxs))
	{
		Boss_ReadyPlayer(ent, boss);
		SelectedBossPlayer = NULL; // reset pointer so someone else can have a turn
	}
	else
	{
		if (level.time > boss_timeout)
			SelectedBossPlayer = NULL;
	//	gi.dprintf("Waiting for the player to be valid...\n");
		// remove the boss entity
		G_FreeEdict(boss);
		return;
	}
}

void CreateRandomPlayerBoss (qboolean find_new_candidate)
{
	int		i, j=0;
	edict_t *player;//, *boss;
	edict_t	*e[MAX_CLIENTS];

	if (BossExists())
		return;

	// find valid boss candidates
	if (!SelectedBossPlayer)
	{
		if (!find_new_candidate)
			return;

		// initialize pointers
		for (i=0; i<MAX_CLIENTS; i++)
		{
			e[i] = NULL;
		}

		// create a list of valid players
		for (i=0; i<game.maxclients; i++) 
		{
			player = g_edicts+1+i;
			if (G_EntExists(player))
			{
				//gi.dprintf("DEBUG: valid player: %s\n", player->client->pers.netname);
				e[j++] = player;
			}
		}

		if (!j)
			return;

		// choose a player randomly
		SelectedBossPlayer = e[GetRandom(0, j-1)];
		//gi.dprintf("DEBUG: %s was selected to be the boss\n", SelectedBossPlayer->client->pers.netname);

		boss_timeout = level.time + PVB_BOSS_TIMEOUT;
	}
	else
	{
		CreateBoss(SelectedBossPlayer);
	}
}

void AddBossExp (edict_t *attacker, edict_t *target)
{
	int		exp_points, credits;
	float	levelmod;
	edict_t *boss;

	//if (!pvb->value)
	//	return;

	attacker = G_GetClient(attacker);

	if (!IsBossTeam(attacker))
		return;

	target = G_GetClient(target);
	if (!target)
		return;

	boss = attacker->owner;

	levelmod = (((float)target->myskills.level+1) / ((float)boss->monsterinfo.level+1));
	exp_points = levelmod*PVB_BOSS_FRAG_EXP;
	credits = levelmod*PVB_BOSS_FRAG_CREDITS;

	// award points and credits
	/*
	attacker->myskills.experience += exp_points;
	attacker->client->resp.score += exp_points;
	check_for_levelup(attacker);
	*/
	attacker->myskills.credits += credits;
	V_AddFinalExp(attacker, exp_points);
}

void AwardBossKill (edict_t *boss)
{
	int			i, damage, exp_points, credits;
	float		levelmod, dmgmod;
	edict_t		*player;
	dmglist_t	*slot=NULL;

	// find the player that did the most damage
	slot = findHighestDmgPlayer(boss);

	for (i=0; i<game.maxclients; i++) 
	{
		player = g_edicts+1+i;
		if (!player->inuse)
			continue;


		levelmod = ((float)boss->monsterinfo.level+1) / ((float)player->myskills.level+1);

		damage = GetPlayerBossDamage(player, boss);
		if (damage < 1)
			continue; // they get nothing if they didn't touch the boss

		dmgmod = (float)damage / GetTotalBossDamage(boss);

		exp_points = levelmod*dmgmod*PVB_BOSS_EXPERIENCE;
		credits = levelmod*dmgmod*PVB_BOSS_CREDITS;

		// award extra points for the player that did the most damage
		if (slot && (player == slot->player))
		{
			dmgmod = 100*(slot->damage/GetTotalBossDamage(boss));
			G_PrintGreenText(va("%s got a hi-damage bonus! %d damage (%.1f%c)", 
				slot->player->client->pers.netname, (int)slot->damage, dmgmod, '%'));
			exp_points *= BOSS_DAMAGE_BONUSMOD;
		}


		if (exp_points > PVB_BOSS_MAX_EXP)
			exp_points = PVB_BOSS_MAX_EXP;
		else if (exp_points < PVB_BOSS_MIN_EXP)
			exp_points = PVB_BOSS_MIN_EXP;

		player->myskills.credits += credits;
		V_AddFinalExp(player, exp_points);

		safe_cprintf(player, PRINT_HIGH, "You gained %d experience and %d credits!\n", exp_points, credits);
	}
}







