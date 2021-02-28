#include "g_local.h"
#include "invasion.h"

//FIXME: need queue that holds all players that are waiting to respawn but all spawns are busy
edict_t		*INV_SpawnQue[MAX_CLIENTS];
int			invasion_max_playerspawns;
int			invasion_spawncount; // current spawns
edict_t		*INV_PlayerSpawns[32];

void INV_Init(void)
{
	int i;
	if (!pvm->value || !invasion->value)
		return;

	INV_InitSpawnQue();
	INVASION_OTHERSPAWNS_REMOVED = false;
	invasion_difficulty_level = 1;
	invasion_max_playerspawns = 0;
	invasion_spawncount = 0;

	for (i = 0; i < 24; i++)   // down from 32 to 24
		INV_PlayerSpawns[i] = NULL;
}

// initialize array values to NULL
void INV_InitSpawnQue(void)
{
	int i;

	for (i = 0; i < MAX_CLIENTS; i++)
		INV_SpawnQue[i] = NULL;
}

edict_t *INV_GiveRandomPSpawn()
{
	int rand = -1;
	int i;
	edict_t * rval = NULL;

	if (invasion_spawncount > 1)
	{
		while (!INV_PlayerSpawns[rand]) // Pick a random, active one.
		{
			rand = GetRandom(1, invasion_max_playerspawns) - 1;
		}

		return INV_PlayerSpawns[rand];
	}
	else if (invasion_spawncount == 1)
	{
		for (i = 0; i < invasion_spawncount; i++) // Find first valid.
			if (INV_PlayerSpawns[i])
				return INV_PlayerSpawns[i];
	}

	return NULL;
}

qboolean INV_IsSpawnQueEmpty()
{
	int i;
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (INV_SpawnQue[i] != NULL)
			return false;
	}
	return true;
}

qboolean INV_InSpawnQue(edict_t *ent)
{
	int i;

	for (i = 0; i < MAX_CLIENTS; i++)
		if (INV_SpawnQue[i] && INV_SpawnQue[i] == ent)
			return true;
	return false;
}

// add player to the spawn queue
qboolean INV_AddSpawnQue(edict_t *ent)
{
	int i;

	if (!ent || !ent->inuse || !ent->client)
		return false;

	// don't add them if they are already in the queue
	if (INV_InSpawnQue(ent))
		return false;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (!INV_SpawnQue[i])
		{
			INV_SpawnQue[i] = ent;
			//gi.dprintf("added %s to list\n", ent->client->pers.netname);
			return true;
		}
	}
	return false;
}

// remove player from the queue
qboolean INV_RemoveSpawnQue(edict_t *ent)
{
	int i;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (INV_SpawnQue[i] && (INV_SpawnQue[i] == ent))
		{
			//gi.dprintf("removed %s from list\n", ent->client->pers.netname);
			INV_SpawnQue[i] = NULL;
			return true;
		}
	}
	return false;
}

// return player that is waiting to respawn
edict_t *INV_GetSpawnPlayer(void)
{
	int i;

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (INV_SpawnQue[i] && INV_SpawnQue[i]->inuse && INV_SpawnQue[i]->client)
		{
			//gi.dprintf("found %s in list\n", INV_SpawnQue[i]->client->pers.netname);
			return INV_SpawnQue[i];
		}
	}
	return NULL;
}

edict_t *INV_GetRandomSpawn(void)
{
	int i = 0;
	edict_t *e, *spawns[32];

	for (e = g_edicts; e < &g_edicts[globals.num_edicts]; e++)
	{
		if (e && e->inuse && (e->mtype == INVASION_PLAYERSPAWN))
			spawns[i++] = e;
	}

	// didn't find any spawns
	if (i == 0)
		return NULL;
	// found one spawn
	else if (i == 1)
		return spawns[0];
	// found more than one spawn, so return a random one
	else
		return spawns[GetRandom(0, i - 1)];

}

edict_t *INV_GetMonsterSpawn(edict_t *from)
{
	if (from)
		from++;
	else
		from = g_edicts;

	for (; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (from && from->inuse && (from->mtype == INVASION_MONSTERSPAWN)
			&& (level.time > from->wait))
			return from;
	}

	return NULL;
}

void INV_AwardPlayers(void)
{
	int		i, points, credits, num_spawns = INV_GetNumPlayerSpawns(), num_winners = 0;
	edict_t *player;

	// we're not in invasion mode
	if (!INVASION_OTHERSPAWNS_REMOVED)
		return;

	// if map didn't end normally, don't award points
	if (level.time < timelimit->value * 60)
		return;

	// no award if the humans were unable to defend their spawns
	if (num_spawns < 1)
		return;

	for (i = 0; i<game.maxclients; i++)
	{
		player = g_edicts + 1 + i;
		if (!player->inuse)
			continue;

		if (invasion->value == 2)
			points = player->client->resp.score*((float)num_spawns / invasion_max_playerspawns) + 1000 * invasion_difficulty_level;
		else
			points = player->client->resp.score*((float)num_spawns / invasion_max_playerspawns);

		if (invasion->value == 1 && points > INVASION_BONUS_EXP)
			points = INVASION_BONUS_EXP;
		//points = INVASION_BONUS_EXP*((float)num_spawns/invasion_max_playerspawns);
		if (invasion->value < 2)
			credits = INVASION_BONUS_CREDITS*((float)num_spawns / invasion_max_playerspawns);
		else
			credits = INVASION_BONUS_CREDITS*((float)num_spawns / invasion_max_playerspawns) + 1000 * invasion_difficulty_level;

		//	gi.dprintf("points=%d credits=%d spawns=%d max=%d\n", 
		//		points, credits, num_spawns, invasion_max_playerspawns);

		if (!G_IsSpectator(player))
		{
			int fexp = V_AddFinalExp(player, points);
			player->myskills.credits += credits;
			safe_cprintf(player, PRINT_MEDIUM, "Earned %d exp and %d credits!\n", fexp, credits);

			if (player->client && player->client->pers.score) // we've been here for a while at least
				num_winners++;
		}
	}

	if (num_winners)
		gi.bprintf(PRINT_HIGH, "Humans win! Players were awarded a bonus.\n");
}

edict_t* INV_SpawnDrone(edict_t* self, edict_t *e, int index)
{
	edict_t *monster;
	vec3_t	start;
	trace_t	tr;
	int mhealth = 1;

	monster = SpawnDrone(self, index, true);

	// calculate starting position
	VectorCopy(e->s.origin, start);
	start[2] = e->absmax[2] + 1 + abs(monster->mins[2]);

	tr = gi.trace(start, monster->mins, monster->maxs, start, NULL, MASK_SHOT);

	if (index != 30) // not a boss?
	{
		// don't spawn here if a friendly monster occupies this space
		if ((tr.fraction < 1) || (tr.ent && tr.ent->inuse && tr.ent->activator && tr.ent->activator->inuse
			&& (tr.ent->activator == self) && (tr.ent->deadflag != DEAD_DEAD)))
		{
			// remove the monster and try again
			M_Remove(monster, false, false);
			return NULL;
		}
	}

	e->wait = level.time + 1.0; // time until spawn is available again

	monster->monsterinfo.aiflags |= AI_FIND_NAVI; // search for navi
	monster->s.angles[YAW] = e->s.angles[YAW];
	monster->prev_navi = NULL;


	// we modify the monsters' health lightly
	if (invasion->value == 1) // easy mode
	{
		if (invasion_difficulty_level < 7 && invasion_difficulty_level > 5)
			mhealth = (invasion_difficulty_level - 5) * 0.2 + 1;
		else if (invasion_difficulty_level >= 7)
			mhealth = 1.8 + 0.15 * invasion_difficulty_level;
	}
	else if (invasion->value == 2) // hard mode
	{
		mhealth = 1.71 + 0.22 * invasion_difficulty_level;
	}

	monster->max_health = monster->health = monster->max_health*mhealth;

	// move the monster onto the spawn pad
	if (index != 30)
	{
		VectorCopy(start, monster->s.origin);
		VectorCopy(start, monster->s.old_origin);
	}
	monster->s.event = EV_OTHER_TELEPORT;

	if (e->count)
		monster->monsterinfo.inv_framenum = level.framenum + e->count;
	else
	{
		if (invasion->value == 1)
			monster->monsterinfo.inv_framenum = level.framenum + 60; // give them quad/invuln to prevent spawn-camping
		else if (invasion->value == 2)
			monster->monsterinfo.inv_framenum = level.framenum + 60; // Hard mode invin
	}
	gi.linkentity(monster);
	return monster;
}

float TimeFormula()
{
	int base = 4 * 60;
	int playeramt = ActivePlayers() * 8;
	int levelamt = invasion_difficulty_level * 7;
	int cap = 60;
	int rval = base - playeramt - levelamt;

	if (invasion->value == 2) // hard mode
		cap = 50;

	if (rval < cap)
		rval = cap;

	return rval;
}

struct invdata_s
{
	int printedmessage;
	int mspawned;
	float limitframe;
	edict_t *boss;

} invasion_data;

// we'll override the other die functino to set our boss pointer to NULL.
void mytank_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

void invasiontank_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	mytank_die(self, inflictor, attacker, damage, point);
	invasion_data.boss = NULL;
}

void BossCheck(edict_t *e, edict_t *self)
{
	if (!(invasion_difficulty_level % 5))// every 5 levels, spawn a boss
	{
		int bcount = 0;
		int iter = 0;
		while (((e = INV_GetMonsterSpawn(e)) != NULL) && (bcount < 1))
		{
			if (!invasion_data.boss)
			{
				if (!(invasion_data.boss = INV_SpawnDrone(self, e, 30)))
				{
					iter++;

					if (iter < 256) // 256 tries before quitting with spawning this boss
						continue;
					else
						break;
				}
				bcount++;
				total_monsters++;
				invasion_data.mspawned++;
				invasion_data.mspawned++;
				invasion_data.boss->die = invasiontank_die;
				G_PrintGreenText(va("A level %d tank commander has spawned!", invasion_data.boss->monsterinfo.level+ 5));
				break;
			}
		}
	}
	else
		invasion_data.boss = NULL;
}

void INV_SpawnMonsters(edict_t *self)
{
	int		players, max_monsters;
	edict_t *e = NULL;
	int SpawnTries = 0, MaxTriesThisFrame = 32;
	PVM_TotalMonsters(self, true);

	players = max_monsters = JoinedPlayers();

	// there are still monsters alive
	if ((self->num_monsters_real > 0) && (self->count == MONSTERSPAWN_STATUS_IDLE))
	{
		// if there's nobody playing, remove all monsters
		if (players < 1)
		{
			PVM_RemoveAllMonsters(self);
		}
		if (level.intermissiontime)
		{
			if (self->num_monsters_real)
				PVM_RemoveAllMonsters(self);
			return;
		}
		if (invasion_data.limitframe > level.time) // we still got time?
		{
			self->nextthink = level.time + FRAMETIME;
			return;
		}
		else
		{
			gi.bprintf(PRINT_HIGH, "Time's up!\n");
			if (invasion_data.boss && invasion_data.boss->deadflag != DEAD_DEAD) // out of time for the boss.
			{
				G_PrintGreenText(va("You failed to eliminate the commander soon enough!\n"));
				gi.WriteByte(svc_temp_entity);
				gi.WriteByte(TE_BOSSTPORT);
				gi.WritePosition(invasion_data.boss->s.origin);
				gi.multicast(invasion_data.boss->s.origin, MULTICAST_PVS);
				gi.unlinkentity(invasion_data.boss);
				G_FreeEdict(invasion_data.boss);
				invasion_data.boss = NULL;
			}
			// increase the difficulty level for the next wave
			if (invasion->value == 1)
				invasion_difficulty_level += 1;
			else
				invasion_difficulty_level += 2; // Hard mode.
			invasion_data.printedmessage = 0;
			gi.sound(&g_edicts[0], CHAN_VOICE, gi.soundindex("misc/tele_up.wav"), 1, ATTN_NONE, 0);
		}
	}
	else // Timeout has happened or all monsters eliminated
	{
		self->count = MONSTERSPAWN_STATUS_WORKING;
		invasion_data.mspawned = self->num_monsters_real;
	}

	if (players < 1)
	{
		// if there's nobody playing, then wait until some join
		self->nextthink = level.time + FRAMETIME;
		return;
	}

	switch (invasion_difficulty_level)
	{
	case 1: max_monsters = 10; break;
	case 2: max_monsters = 15; break;
	case 3: max_monsters = 15; break;
	case 4: max_monsters = 15; break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
		max_monsters = 15; break; // vrxcl 3.2b decrease for not saturating the server hard.

	default: max_monsters = 16;
	}

	if (!(invasion_difficulty_level % 5))
	{
		if (invasion->value == 1)
			max_monsters = 4 * (JoinedPlayers() - 1);
		else if (invasion->value == 2)
			max_monsters = 6 * (JoinedPlayers() - 1);
	}

	if (!invasion_data.printedmessage)
	{
		invasion_data.limitframe = level.time + TimeFormula();
		if (invasion_difficulty_level == 1)
		{
			if (invasion->value == 1)
				gi.bprintf(PRINT_HIGH, "The invasion begins!\n");
			else
				gi.bprintf(PRINT_HIGH, "The invasion... begins.\n");
		}
		if (invasion_difficulty_level % 5)
			gi.bprintf(PRINT_HIGH, "Welcome to level %d. %d monsters incoming!\n", invasion_difficulty_level, max_monsters);
		else
			gi.bprintf(PRINT_HIGH, "Welcome to level %d.\n", invasion_difficulty_level, max_monsters);
		G_PrintGreenText(va("Timelimit: %dm %ds.\n", (int)TimeFormula() / 60, (int)TimeFormula() % 60));

		gi.sound(&g_edicts[0], CHAN_VOICE, gi.soundindex("misc/talk1.wav"), 1, ATTN_NONE, 0);

		BossCheck(e, self);

		invasion_data.printedmessage = 1;
	}

	while ((e = INV_GetMonsterSpawn(e)) && invasion_data.mspawned < max_monsters && SpawnTries < MaxTriesThisFrame)
	{
		int randomval = GetRandom(1, 9);

		if (invasion_difficulty_level % 5 && invasion->value == 1) // nonboss stage? easy mode?
		{
			while (randomval == 5) // disallow medics
			{
				randomval = GetRandom(1, 8);
			}
		}

		SpawnTries++;
		if (!INV_SpawnDrone(self, e, randomval)) // Wait for now
			continue;
		else
			invasion_data.mspawned++;
	}

	if (invasion_data.mspawned == max_monsters)
	{
		// increase the difficulty level for the next wave
		invasion_difficulty_level += 1;
		invasion_data.printedmessage = 0;
		invasion_data.mspawned = 0;
		self->count = MONSTERSPAWN_STATUS_IDLE;
	}

	self->nextthink = level.time + FRAMETIME;
}

void INV_SpawnPlayers(void)
{
	edict_t *e, *cl_ent;
	vec3_t	start;
	trace_t	tr;

	// we shouldn't be here if this isn't invasion mode
	if (!INVASION_OTHERSPAWNS_REMOVED)
		return;

	// if there are no players waiting to be spawned, then we're done
	if (!(cl_ent = INV_GetSpawnPlayer()))
		return;

	for (e = g_edicts; e < &g_edicts[globals.num_edicts]; e++)
	{
		// find an available spawn point
		if (e && e->inuse && (e->mtype == INVASION_PLAYERSPAWN) && (level.time > e->wait))
		{
			// get player starting position
			VectorCopy(e->s.origin, start);
			start[2] = e->absmax[2] + 1 + abs(cl_ent->mins[2]);

			tr = gi.trace(start, cl_ent->mins, cl_ent->maxs, start, NULL, MASK_SHOT);

			// don't spawn if another player is standing in the way
			if ((tr.fraction < 1) && tr.ent && tr.ent->inuse && tr.ent->client)
			{
				e->wait = level.time + 1.0;
				continue;
			}

			e->wait = level.time + 2.0; // delay before other players can use this spawn
			cl_ent->spawn = e; // player may use this spawn
			respawn(cl_ent);

			// get another waiting player
			if (!(cl_ent = INV_GetSpawnPlayer()))
				return;
		}
	}
}

edict_t *INV_SelectPlayerSpawnPoint(edict_t *ent)
{
	if (!ent || !ent->inuse)
		return NULL;

	// spectators always get a spawn
	if (G_IsSpectator(ent))
		return INV_GetRandomSpawn();

	if (ent->spawn && ent->spawn->inuse)
		return ent->spawn;
	else // We requested a spawn point, but we don't have one. What now?
	{
		// Try to find one. But only if the spawn que is empty.
		if (INV_IsSpawnQueEmpty())
			return INV_GetRandomSpawn();
	}

	return NULL;
}

int INV_GetNumPlayerSpawns(void)
{
	return invasion_spawncount;
}

void INV_RemoveFromSpawnlist(edict_t *self)
{
	int i;
	for (i = 0; i < 32; i++)
		if (INV_PlayerSpawns[i] == self)
		{
			INV_PlayerSpawns[i] = NULL;
			invasion_spawncount--;

			if (invasion_spawncount <= 0)
			{
				gi.bprintf(PRINT_HIGH, "Humans were unable to stop the invasion. Game over.\n");
				EndDMLevel();
				return;
			}
		}
}

void info_player_invasion_death(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_UseTargets(self, self);
	self->think = BecomeExplosion1;
	self->nextthink = level.time + FRAMETIME;
	INV_RemoveFromSpawnlist(self);
}

void info_player_invasion_think(edict_t *self)
{
	if (level.time > self->lasthurt + 1.0)
		M_Regenerate(self, PLAYERSPAWN_REGEN_FRAMES, PLAYERSPAWN_REGEN_DELAY, 1.0, true, false, false, &self->monsterinfo.regen_delay1);

	self->nextthink = level.time + FRAMETIME;
}

void SP_info_player_invasion(edict_t *self)
{
	//FIXME: change to invasion->value
	if (!pvm->value || !invasion->value)
	{
		G_FreeEdict(self);
		return;
	}

	// remove deathmatch spawnpoints
	if (!INVASION_OTHERSPAWNS_REMOVED)
	{
		edict_t *e = NULL;

		gi.dprintf("PvM Invasion mode activated!\n");

		while ((e = G_Find(e, FOFS(classname), "info_player_deathmatch")) != NULL)
		{
			if (e && e->inuse)
				G_FreeEdict(e);
		}
		INVASION_OTHERSPAWNS_REMOVED = true;
	}

	// this entity should be killable and game should end if all of them die
	gi.setmodel(self, "models/objects/dmspot/tris.md2");
	self->s.skinnum = 0;
	self->mtype = INVASION_PLAYERSPAWN;

	if (invasion->value == 1)
		self->health = PLAYERSPAWN_HEALTH + 250 * AveragePlayerLevel();
	else if (invasion->value == 2)
		self->health = PLAYERSPAWN_HEALTH * 2 + 250 * AveragePlayerLevel();

	self->max_health = self->health;
	self->takedamage = DAMAGE_YES;
	self->die = info_player_invasion_death;
	self->think = info_player_invasion_think;
	self->nextthink = level.time + FRAMETIME;
	//self->touch = info_player_invasion_touch;
	self->solid = SOLID_BBOX;
	VectorSet(self->mins, -32, -32, -24);
	VectorSet(self->maxs, 32, 32, -16);
	gi.linkentity(self);

	INV_PlayerSpawns[invasion_max_playerspawns] = self;

	invasion_max_playerspawns++;
	invasion_spawncount++;
}

void SP_info_monster_invasion(edict_t *self)
{
	//FIXME: change to invasion->value
	if (!pvm->value || !invasion->value)
	{
		G_FreeEdict(self);
		return;
	}

	self->mtype = INVASION_MONSTERSPAWN;
	self->solid = SOLID_NOT;
	self->s.effects |= EF_BLASTER;
	gi.setmodel (self, "models/items/c_head/tris.md2");
	self->svflags |= SVF_NOCLIENT;
	gi.linkentity(self);
}

void SP_navi_monster_invasion(edict_t *self)
{
	//FIXME: change to invasion->value
	if (!pvm->value || !invasion->value)
	{
		G_FreeEdict(self);
		return;
	}

	//gi.dprintf("navi point created!\n");

	self->solid = SOLID_NOT;
	self->mtype = INVASION_NAVI;
	gi.setmodel (self, "models/items/c_head/tris.md2");
	self->svflags |= SVF_NOCLIENT;
	gi.linkentity(self);
}

int G_GetEntityIndex(edict_t *ent)
{
	int		i = 0;
	edict_t *e;

	if (!ent || !ent->inuse)
		return 0;

	for (e = g_edicts; e < &g_edicts[globals.num_edicts]; e++)
	{
		if (e && e->inuse && (e == ent))
			return i;
		i++;
	}

	return 0;
}

void inv_defenderspawn_think(edict_t *self)
{
	//int		num=G_GetEntityIndex(self); // FOR DEBUGGING ONLY
	vec3_t	start;
	trace_t	tr;
	edict_t *monster = NULL;

	//FIXME: this isn't a good enough check if monster is dead or not
	// did our monster die?
	if (!G_EntIsAlive(self->enemy))
	{
		if (self->orders == MONSTERSPAWN_STATUS_IDLE)
		{
			// wait some time before spawning another monster
			self->wait = level.time + 30;
			self->orders = MONSTERSPAWN_STATUS_WORKING;
			//gi.dprintf("%d: monster died, waiting to build monster...\n", num);
		}

		// try to spawn another
		if ((level.time > self->wait)
			&& (monster = SpawnDrone(self, self->sounds, true)) != NULL)
		{
			//gi.dprintf("%d: attempting to spawn a monster\n", num);
			// get starting position
			VectorCopy(self->s.origin, start);
			start[2] = self->absmax[2] + 1 + abs(monster->mins[2]);

			tr = gi.trace(start, monster->mins, monster->maxs, start, NULL, MASK_SHOT);

			// kill dead bodies
			if (tr.ent && tr.ent->takedamage && (tr.ent->deadflag == DEAD_DEAD || tr.ent->health < 1))
				T_Damage(tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, 10000, 0, 0, 0);
			// spawn is blocked, try again later
			else if (tr.fraction < 1)
			{
				//gi.dprintf("%d: spawn is blocked, will try again\n", num);
				G_FreeEdict(monster);
				self->nextthink = level.time + 10.0;
				return;
			}

			// should this monster stand ground?
			if (self->style)
				monster->monsterinfo.aiflags |= AI_STAND_GROUND;

			monster->s.angles[YAW] = self->s.angles[YAW];
			// move the monster onto the spawn pad
			VectorCopy(start, monster->s.origin);
			VectorCopy(start, monster->s.old_origin);
			monster->s.event = EV_OTHER_TELEPORT;

			// give them quad/invuln to prevent spawn-camping
			if (self->count)
				monster->monsterinfo.inv_framenum = level.framenum + self->count;
			else
				monster->monsterinfo.inv_framenum = level.framenum + 60;

			gi.linkentity(monster);

			self->enemy = monster; // keep track of this monster
			//gi.dprintf("%d: spawned a monster successfully\n", num);
		}
		else
		{
			//if (level.time > self->wait)
			//	gi.dprintf("%d: spawndrone() failed to spawn a monster\n", num);
			//else if (!(level.framenum%10))
			//	gi.dprintf("%d: waiting...\n", num);
			self->orders = MONSTERSPAWN_STATUS_IDLE;
		}
	}
	else
	{
		//if (self->orders == MONSTERSPAWN_STATUS_WORKING)
		//	gi.dprintf("%d: spawn is now idle\n", num);
		self->orders = MONSTERSPAWN_STATUS_IDLE;
	}

	self->nextthink = level.time + FRAMETIME;
}

// map-editable fields - count (quad+invin), style (stand ground), angle, sounds (mtype to spawn)
void SP_inv_defenderspawn(edict_t *self)
{
	//FIXME: change to invasion->value
	if (!pvm->value || !invasion->value)
	{
		G_FreeEdict(self);
		return;
	}

	self->mtype = INVASION_DEFENDERSPAWN;
	self->solid = SOLID_NOT;
	self->think = inv_defenderspawn_think;
	self->nextthink = level.time + pregame_time->value + FRAMETIME;
	self->s.effects |= EF_BLASTER;
	gi.setmodel (self, "models/items/c_head/tris.md2");
	self->svflags |= SVF_NOCLIENT;
	gi.linkentity(self);
}

float GetPlayerBossDamage(edict_t *player, edict_t *boss);
float GetTotalBossDamage(edict_t *self);
