#include "g_local.h"

char lightlevel[2] = { 'm', '\0' };
int day = 1;

/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.
*/

//=====================================================

void Use_Areaportal(edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->count ^= 1;		// toggle state
	//	gi.dprintf ("portalstate: %i = %i\n", ent->style, ent->count);
	gi.SetAreaPortalState(ent->style, ent->count);
}

/*QUAKED func_areaportal (0 0 0) ?

This is a non-visible object that divides the world into
areas that are seperated when this portal is not activated.
Usually enclosed in the middle of a door.
*/
void SP_func_areaportal(edict_t *ent)
{
	ent->use = Use_Areaportal;
	ent->count = 0;		// always start closed;
}

//=====================================================


/*
=================
Misc functions
=================
*/
void VelocityForDamage(int damage, vec3_t v)
{
	v[0] = 100.0 * crandom();
	v[1] = 100.0 * crandom();
	v[2] = 200.0 + 100.0 * random();

	if (damage < 50)
		VectorScale(v, 0.7, v);
	else
		VectorScale(v, 1.2, v);
}

void ClipGibVelocity(edict_t *ent)
{
	if (ent->velocity[0] < -300)
		ent->velocity[0] = -300;
	else if (ent->velocity[0] > 300)
		ent->velocity[0] = 300;
	if (ent->velocity[1] < -300)
		ent->velocity[1] = -300;
	else if (ent->velocity[1] > 300)
		ent->velocity[1] = 300;
	if (ent->velocity[2] < 200)
		ent->velocity[2] = 200;	// always some upwards
	else if (ent->velocity[2] > 500)
		ent->velocity[2] = 500;
}


/*
=================
gibs
=================
*/
void gib_think(edict_t *self)
{
	self->s.frame++;
	self->nextthink = level.time + FRAMETIME;

	if (self->s.frame == 10)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 8 + random() * 10;
	}
}

void gib_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	normal_angles, right;

	if (!self->groundentity)
		return;

	self->touch = NULL;

	if (plane)
	{
		gi.sound(self, CHAN_VOICE, gi.soundindex("misc/fhit3.wav"), 1, ATTN_NORM, 0);

		vectoangles(plane->normal, normal_angles);
		AngleVectors(normal_angles, NULL, right, NULL);
		vectoangles(right, self->s.angles);

		if (self->s.modelindex == sm_meat_index)
		{
			self->s.frame++;
			self->think = gib_think;
			self->nextthink = level.time + FRAMETIME;
		}
	}
}

void gib_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict(self);
}

void ThrowGib(edict_t *self, char *gibname, int damage, int type)
{
	edict_t *gib;
	vec3_t	vd;
	vec3_t	origin;
	vec3_t	size;
	float	vscale;

#ifndef OLD_NOLAG_STYLE
	if (nolag->value)
		return;
#endif

	gib = G_Spawn();

	VectorScale(self->size, 0.5, size);
	VectorAdd(self->absmin, size, origin);
	gib->s.origin[0] = origin[0] + crandom() * size[0];
	gib->s.origin[1] = origin[1] + crandom() * size[1];
	gib->s.origin[2] = origin[2] + crandom() * size[2];

	gi.setmodel(gib, gibname);
	gib->solid = SOLID_NOT;
	gib->s.effects |= EF_GIB;
	gib->flags |= FL_NO_KNOCKBACK;
	gib->takedamage = DAMAGE_YES;
	gib->die = gib_die;

	if (type == GIB_ORGANIC)
	{
		gib->movetype = MOVETYPE_TOSS;
		gib->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		gib->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}

	VelocityForDamage(damage, vd);
	VectorMA(self->velocity, vscale, vd, gib->velocity);
	ClipGibVelocity(gib);
	gib->avelocity[0] = random() * 600;
	gib->avelocity[1] = random() * 600;
	gib->avelocity[2] = random() * 600;

	gib->think = G_FreeEdict;
	gib->nextthink = level.time + GetRandom(2, 10);

	gi.linkentity(gib);
}

void ThrowHead(edict_t *self, char *gibname, int damage, int type)
{
	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;
	// prevent the die() function from being called again
	// since it is already gibbed, avoiding entity overflow
	self->takedamage = DAMAGE_NO;
	return; // 3.2 FIXME: this function is buggy, replace it!
}

void ThrowHead2(edict_t *self, char *gibname, int damage, int type)
{
	vec3_t	vd;
	float	vscale;

	self->s.skinnum = 0;
	self->s.frame = 0;
	VectorClear(self->mins);
	VectorClear(self->maxs);

	self->s.modelindex2 = 0;
	gi.setmodel(self, gibname);
	self->solid = SOLID_NOT;
	self->s.effects |= EF_GIB;
	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;
	//	self->svflags &= ~SVF_MONSTER;
	self->takedamage = DAMAGE_YES;
	self->die = gib_die;

	if (type == GIB_ORGANIC)
	{
		self->movetype = MOVETYPE_TOSS;
		self->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		self->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}

	VelocityForDamage(damage, vd);
	VectorMA(self->velocity, vscale, vd, self->velocity);
	ClipGibVelocity(self);

	self->avelocity[YAW] = crandom() * 600;

	//	self->think = G_FreeEdict;
	//	self->nextthink = level.time + 10 + random()*10;

	gi.linkentity(self);
}

void ThrowClientHead(edict_t *self, int damage)
{
	vec3_t	vd;
	char	*gibname;

	if (rand() & 1)
	{
		gibname = "models/objects/gibs/head2/tris.md2";
		self->s.skinnum = 1;		// second skin is player
	}
	else
	{
		gibname = "models/objects/gibs/skull/tris.md2";
		self->s.skinnum = 0;
	}

	self->s.origin[2] += 32;
	self->s.frame = 0;
	gi.setmodel(self, gibname);
	VectorSet(self->mins, -16, -16, 0);
	VectorSet(self->maxs, 16, 16, 16);

	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->s.effects = EF_GIB;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;

	self->movetype = MOVETYPE_BOUNCE;
	VelocityForDamage(damage, vd);
	VectorAdd(self->velocity, vd, self->velocity);

	if (self->client)	// bodies in the queue don't have a client anymore
	{
		if (!(self->svflags & SVF_MONSTER))
		{
			self->client->anim_priority = ANIM_DEATH;
			self->client->anim_end = self->s.frame;
		}
		else
		{
			self->s.modelindex2 = 0;
			self->s.modelindex3 = 0;
			self->s.frame = 0;
			self->client->anim_end = self->s.frame;
		}
	}
	else
	{
		self->think = NULL;
		self->nextthink = 0;
	}

	gi.linkentity(self);
}


/*
=================
debris
=================
*/
void debris_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict(self);
}

void ThrowDebris(edict_t *self, char *modelname, float speed, vec3_t origin)
{
	edict_t	*chunk;
	vec3_t	v;

	chunk = G_Spawn();
	VectorCopy(origin, chunk->s.origin);
	gi.setmodel(chunk, modelname);
	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
	VectorMA(self->velocity, speed, v, chunk->velocity);
	chunk->movetype = MOVETYPE_BOUNCE;
	chunk->solid = SOLID_NOT;
	chunk->avelocity[0] = random() * 600;
	chunk->avelocity[1] = random() * 600;
	chunk->avelocity[2] = random() * 600;
	chunk->think = G_FreeEdict;
	chunk->nextthink = level.time + 5 + random() * 5;
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "debris";
	chunk->takedamage = DAMAGE_YES;
	chunk->die = debris_die;
	gi.linkentity(chunk);
}


void BecomeExplosion1(edict_t *self)
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_EXPLOSION1);
	gi.WritePosition(self->s.origin);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	G_FreeEdict(self);
}


void BecomeExplosion2(edict_t *self)
{
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_EXPLOSION2);
	gi.WritePosition(self->s.origin);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	G_FreeEdict(self);
}

void BecomeTE(edict_t *self)
{
	// GHz: Make a cool effect!
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_BFG_EXPLOSION);
	gi.WritePosition(self->s.origin);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	G_FreeAnyEdict(self);
}

void BecomeBigExplosion(edict_t *self)
{
	// GHz: Make a cool effect!
	gi.WriteByte(svc_temp_entity);
	gi.WriteByte(TE_EXPLOSION1_BIG);
	gi.WritePosition(self->s.origin);
	gi.multicast(self->s.origin, MULTICAST_PVS);

	G_FreeEdict(self);
}

void FindIdleObserver(edict_t *scanent)
{
	edict_t *player;
	//	edict_t *found = NULL;
	int i;
	//	int checkteam, lowteam, desiredlevel, foundlevel = 0;	

	for (i = 1; i <= maxclients->value; i++)
	{
		player = &g_edicts[i];

		if (!player->inuse)
			continue;
		if (player->svflags & SVF_MONSTER)
			continue;
		if (!player->solid == SOLID_NOT)
		{

			/*G_StuffPlayerCmds(player, "checkclientsettings 1 $gl_modulate\n");
			G_StuffPlayerCmds(player, "checkclientsettings 2 $gl_dynamic\n");
			G_StuffPlayerCmds(player, "checkclientsettings 3 $sw_drawflat\n");
			G_StuffPlayerCmds(player, "checkclientsettings 4 $gl_showtris\n");
			G_StuffPlayerCmds(player, "checkclientsettings 5 $r_fullbright\n");
			G_StuffPlayerCmds(player, "checkclientsettings 6 $timescale\n");
			G_StuffPlayerCmds(player, "checkclientsettings 7 $gl_lightmap\n");
			G_StuffPlayerCmds(player, "checkclientsettings 8 $gl_saturatelighting\n");
			G_StuffPlayerCmds(player, "checkclientsettings 9 $r_drawflat\n");
			G_StuffPlayerCmds(player, "checkclientsettings 10 $cl_testlights\n");
			G_StuffPlayerCmds(player, "checkclientsettings 11 $fixedtime\n");
			G_StuffPlayerCmds(player, "say sd8fh34ewu73hg frkq2_bot=$frkq2_bot\n");*/
			/*
			//GHz: Check client settings for cheaters
			if (random() > 0.5)
			{
			stuffcmd(player, "checkclientsettings 1 $gl_modulate\n");
			stuffcmd(player, "checkclientsettings 2 $gl_dynamic\n");
			stuffcmd(player, "checkclientsettings 3 $sw_drawflat\n");
			stuffcmd(player, "checkclientsettings 4 $gl_showtris\n");
			stuffcmd(player, "checkclientsettings 5 $r_fullbright\n");
			}
			else
			{
			stuffcmd(player, "checkclientsettings 6 $timescale\n");
			stuffcmd(player, "checkclientsettings 7 $gl_lightmap\n");
			stuffcmd(player, "checkclientsettings 8 $gl_saturatelighting\n");
			stuffcmd(player, "checkclientsettings 9 $r_drawflat\n");
			stuffcmd(player, "checkclientsettings 10 $cl_testlights\n");
			stuffcmd(player, "checkclientsettings 11 $fixedtime\n");
			}
			*/
			// check for frkq2 bot
			//stuffcmd(player, "say sd8fh34ewu73hg frkq2_bot=$frkq2_bot\n");
			continue;
		}
		if (ActivePlayers() + 1 >= maxclients->value)
		{
			//3.0 don't kick players watching a domination game
			if (domination->value && level.time > pregame_time->value)
				continue;
			if (player->client->pers.spectator == false)
				continue;
			if (player->client->resp.spectator == false)
				continue;
			if (player->client->disconnect_time > level.time)
				continue;
			if (player->myskills.administrator > 0)//GHz: Dont kick admins!
				continue;

			if (!player->client->disconnect_time){
				player->client->disconnect_time = level.time + 30;
				safe_cprintf(player, PRINT_HIGH, "WARNING: Server is nearly full. Idle-timeout restrictions are in effect.\n");
				safe_cprintf(player, PRINT_HIGH, "You have 30 seconds to start your reign.\n");
				break;
			}

			gi.bprintf(PRINT_HIGH, "%s was kicked for idle-timeout.\n", player->client->pers.netname);
			stuffcmd(player, "disconnect\n");
		}
	}
	scanent->nextthink = level.time + 15;//GetRandom(1,10);
}

//GHz: This ent will scan for idle observers and kick them to free up slots
void InitScanEntity(void)
{
	edict_t *scanent;

	if (ptr->value)
		return;

	scanent = G_Spawn();
	scanent->svflags |= SVF_NOCLIENT;
	scanent->think = FindIdleObserver;
	scanent->nextthink = level.time + 10.0;
	gi.linkentity(scanent);
}

char fullbright;
char nightbright;
char nighttimelight;

void Sun_Think(edict_t *self)
{
	// Setup light animation tables. 'a' is total darkness, 'z' is doublebright.

	if (day)
	{
		lightlevel[0]++;
		if (lightlevel[0] == fullbright){
			self->nextthink = level.time + 120;
			day = 0;
		}
		else
			self->nextthink = level.time + 5;
	}
	else
	{
		lightlevel[0]--;
		if (lightlevel[0] <= nightbright){
			self->nextthink = level.time + 100;
			day = 1;
		}
		else
			self->nextthink = level.time + 5;
	}

	if (lightlevel[0] < nighttimelight)
		level.daytime = false;
	else
		level.daytime = true;

	gi.configstring(CS_LIGHTS + 0, lightlevel);
	//gi.dprintf("just called Sun_Think() at %c\n", lightlevel[0]);
}

void InitSunEntity(void)
{
	edict_t *sun;
	sun = G_Spawn();
	sun->svflags |= SVF_NOCLIENT;
	sun->think = Sun_Think;
	sun->nextthink = level.time + 10.0;
	gi.linkentity(sun);
	if (invasion->value < 2)
	{
		fullbright = 'z';
		nightbright = 'd';
		nighttimelight = 'k';
	}
	else
	{
		fullbright = 's';
		nightbright = 'c';
		nighttimelight = 'i';
	}
}

int HighestLevelPlayer(void)
{
	edict_t *player;
	int highest = 0, i;

	for (i = 1; i <= maxclients->value; i++){
		player = &g_edicts[i];

		if (!player->inuse)
			continue;
		if (G_IsSpectator(player))
			continue;
		if (player->myskills.boss)
			continue;

		if (player->myskills.level > highest)
			highest = player->myskills.level;
	}

	if (highest < 1)
		highest = 1;

	return highest;
}

int PvMHighestLevelPlayer(void)
{
	edict_t *player;
	int highest = 0, i;

	for (i = 1; i <= maxclients->value; i++){
		player = &g_edicts[i];

		if (!player->inuse)
			continue;
		if (G_IsSpectator(player))
			continue;
		if (player->myskills.boss)
			continue;

		if (player->myskills.level > highest)
			highest = player->myskills.level;
	}

	if (highest < 1)
		highest = 1;

	return highest;
}

int PvMLowestLevelPlayer(void)
{
	edict_t *player;
	int lowest = 999, i;

	for (i = 1; i <= maxclients->value; i++){
		player = &g_edicts[i];

		if (!player->inuse)
			continue;
		if (G_IsSpectator(player))
			continue;
		if (player->myskills.boss)
			continue;

		if (player->myskills.level < lowest)
			lowest = player->myskills.level;
	}

	if (lowest < 1 || lowest == 999)
		lowest = 1;

	return lowest;
}

int LowestLevelPlayer(void)
{
	edict_t *player;
	int lowest = 999, i;

	for (i = 1; i <= maxclients->value; i++){
		player = &g_edicts[i];

		if (!player->inuse)
			continue;

		//decino: don't check for bot levels because this also calculates theirs!
		if (player->ai.is_bot)
			continue;

		if (G_IsSpectator(player))
			continue;
		if (player->myskills.boss)
			continue;

		if (player->myskills.level < lowest)
			lowest = player->myskills.level;
	}

	if (lowest < 1 || lowest == 999)
		lowest = 1;

	return lowest;
}

int JoinedPlayers()
{
	edict_t *player;
	int i, clients = 0;

	for (i = 1; i <= maxclients->value; i++){
		player = &g_edicts[i];

		if (!player->inuse)
			continue;
		if (G_IsSpectator(player))
			continue;
		if (player->ai.is_bot)
			continue;

		clients++;
	}

	if (clients < 1)
		return 0;

	return clients;
}

int ActivePlayers (void)
{
	edict_t *player;
	int i, clients = 0;

	for (i = 1; i <= maxclients->value; i++){
		player = &g_edicts[i];

		if (!player->inuse)
			continue;
		if (G_IsSpectator(player))
			continue;
		if (player->myskills.boss)
			continue;
		if (!G_EntIsAlive(player))
			continue;
		if (player->ai.is_bot)
			continue;

		clients++;
	}

	if (clients < 1)
		return 0;

	return clients;
}

int PvMAveragePlayerLevel(void)
{
	edict_t *player;
	int players = 0, levels = 0, average, i;

	for (i = 1; i <= maxclients->value; i++)
	{
		player = &g_edicts[i];

		if (!player->inuse)
			continue;
		if (G_IsSpectator(player))
			continue;
		if (player->myskills.boss)
			continue;
		players++;
		levels += player->myskills.level;
	}

	if (players < 1)
		return 0;
	if (levels < 1)
		levels = 1;


	average = levels / players;

	if (average < 1)
		average = 1;

	if (debuginfo->value)
		gi.dprintf("DEBUG: PvM Average player level %d\n", average);
	return average;
}

int AveragePlayerLevel(void)
{
	edict_t *player;
	int players = 0, levels = 0, average, i;

	for (i = 1; i <= maxclients->value; i++){
		player = &g_edicts[i];

		if (!player->inuse)
			continue;
		if (G_IsSpectator(player))
			continue;
		if (player->myskills.boss)
			continue;

		if (player->ai.is_bot) // az: heheh
			continue;

		players++;
		levels += player->myskills.level;
		//	gi.dprintf("%s level %d added, total %d\n", player->client->pers.netname,
		//		player->myskills.level, players);
	}

	if (players < 1)
		return 0;
	if (levels < 1)
		levels = 1;

	average = levels / players;

	if (average < 1)
		average = 1;

	if (debuginfo->value)
		gi.dprintf("DEBUG: Average player level %d\n", average);
	return average;
}

int PVM_TotalMonsters(edict_t *monster_owner, qboolean update)
{
	int		i = 0;
	edict_t *e = NULL;

	if (update)
	{
		e = DroneList_Iterate();

		while (e)
		{
			// found a live monster that we own
			if (G_EntIsAlive(e) && (e->activator == monster_owner))
			{
				i++;
			}
			e = DroneList_Next(e);
		}
		monster_owner->num_monsters_real = i;
		return monster_owner->num_monsters_real;
	}
	return monster_owner->num_monsters_real; // will this work?
}

int PVM_RemoveAllMonsters(edict_t *monster_owner)
{
	int		i = 0;
	edict_t *e = NULL;

	e = DroneList_Iterate();

	while (e)
	{
		if (G_EntExists(e) && e->activator && (e->activator == monster_owner))
		{
			M_Remove(e, false, false);
			i++;
		}
		e = DroneList_Next(e);
	}
	return i;
}


//qboolean SpawnWorldMonster(edict_t *ent, int mtype);
void FindMonsterSpot(edict_t *self)
{
	edict_t *scan=NULL;
	int		players = JoinedPlayers();
	int		total_monsters, max_monsters=0;
	int		mtype=0, num=0, i=0;
	total_monsters = PVM_TotalMonsters(self, false);

	max_monsters = level.r_monsters;

	// dm_monsters cvar sets minimum number of monsters that will spawn, regardless of map
	if (max_monsters < dm_monsters->value)
		max_monsters = dm_monsters->value;

	if (level.time > self->delay)
	{
		total_monsters = PVM_TotalMonsters(self, true);
		// adjust spawning delay based on efficiency of player monster kills
		if (total_monsters < max_monsters)
		{
			if (total_monsters < 0.5 * max_monsters)
			{
				self->random -= 1.0;

				// minimum delay
				if (self->random < 5.0)
					self->random = 5.0;
			}
			else
			{
				self->random += 1.0;

				// maximum delay
				if (self->random > ffa_respawntime->value)
					self->random = ffa_respawntime->value;
			}
		}

		// spawn monsters until we reach the limit
		while (total_monsters < max_monsters)
		{
			if ((scan = SpawnDrone(self, GetRandom(1, 9), true)) != NULL)
			{
				if (scan->monsterinfo.walk && random() > 0.5)
					scan->monsterinfo.walk(scan);
				total_monsters++;
			}
		}

		WriteServerMsg(va("World has %d/%d monsters. Next update in %.1f seconds.", total_monsters, max_monsters, self->random), "Info", true, false);

		// wait awhile before trying to spawn monsters again
		self->delay = level.time + self->random;
	}

	// there is a 10-20% chance that a boss will spawn
	// if the server is more than 1/6 full

	if (!BossExists() && (self->num_sentries < 1) && (players > 0.6*maxclients->value))
	{
		int chance = 10;

		if (ffa->value)
			chance *= 2;

		if (GetRandom(1, 200) <= chance) // oh yeah!
			CreateRandomPlayerBoss(true);
	}


	self->nextthink = level.time + FRAMETIME;
}

void SpawnRandomBoss(edict_t *self)
{
	// 3% chance for a boss to spawn a boss if there isn't already one spawned
	if (!SPREE_WAR && ActivePlayers() >= 8 && self->num_sentries < 1)
	{
		int chance = GetRandom(1, 100);

		if ((chance >= 97) && SpawnDrone(self, GetRandom(30, 31), true))
		{
			//gi.dprintf("Spawning a boss monster (chance = %d) at %.1f. Waiting 300 seconds to try again.\n", 
			//	chance, level.time);
			// 5 minutes until we can try to spawn another boss
			self->nextthink = level.time + 300.0;
			return;
		}
	}

	self->nextthink = level.time + 60.0;// try again in 1 minute
}

void INV_SpawnMonsters(edict_t *self);

edict_t *InitMonsterEntity(qboolean manual_spawn)
{
	edict_t *monster;

	if (ctf->value || domination->value || ptr->value || tbi->value || trading->value || V_IsPVP())
		return NULL;//4.4

	//if (!pvm->value && !ffa->value)
	//	return NULL;

	monster = G_Spawn();

	if (manual_spawn)
		monster->style = 1;

	monster->classname = "MonsterSpawn";
	monster->svflags |= SVF_NOCLIENT;
	monster->mtype = M_WORLDSPAWN;
	monster->num_monsters_real = 0;

	/*if (V_IsPVP())
	monster->think = SpawnRandomBoss;//4.4
	else */
	// most of the time people don't like having their pvp interrupted by a boss
	if (INVASION_OTHERSPAWNS_REMOVED)
		monster->think = INV_SpawnMonsters;
	else
		monster->think = FindMonsterSpot;

	monster->nextthink = pregame_time->value + FRAMETIME;
	monster->random = ffa_respawntime->value;//4.52 default delay to begin respawning monsters
	gi.linkentity(monster);

	return monster;
}

void Add_ctfteam_exp(edict_t *ent, int points)
{
	int		i, addexp = (int)(0.5 * points);
	float	level_diff;
	edict_t *player;

	for (i = 1; i <= maxclients->value; i++) {
		player = &g_edicts[i];
		if (!player->inuse)
			continue;
		if (player == ent)
			continue;
		if (player->solid == SOLID_NOT)
			continue;
		if (player->health <= 0)
			continue;
		// players must help the team in order to get shared points!
		if ((!pvm->value && (player->lastkill + 30 < level.time))
			|| (player->lastkill + 60 < level.time))
			continue;

		if (OnSameTeam(ent, player))
		{
			level_diff = (float)(1 + player->myskills.level) / (float)(1 + ent->myskills.level);
			if (level_diff > 1.5 || level_diff < 0.66)
				continue;
			player->client->resp.score += addexp;
			player->myskills.experience += addexp;
			check_for_levelup(player);
		}
	}
}

void Grenade_Explode(edict_t *ent);
void RemoveLaserDefense(edict_t *ent);
/*
=============
VortexRemovePlayerSummonables

Removes all edicts that the player owns
=============
*/
void RemoveProxyGrenades(edict_t *ent);
void RemoveNapalmGrenades(edict_t *ent);
void sentrygun_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void RemoveExplodingArmor(edict_t *ent);
void RemoveAutoCannons(edict_t *ent);
void caltrops_removeall(edict_t *ent);
void spikegren_removeall(edict_t *ent);
void detector_removeall(edict_t *ent);
void minisentry_remove(edict_t *self);
void organ_remove(edict_t *self, qboolean refund);
void organ_removeall(edict_t *self, char *classname, qboolean refund);
void RemoveMiniSentries(edict_t *ent);
void RemoveAllLaserPlatforms(edict_t *ent);//4.5
void holyground_remove(edict_t *ent, edict_t *self);
void depot_remove(edict_t *self, edict_t *owner, qboolean effect);
void lasertrap_removeall(edict_t *ent, qboolean effect);

void VortexRemovePlayerSummonables(edict_t *self)
{
	edict_t *from;

	//	gi.dprintf("VortexRemovePlayerSummonables()\n");

	lasertrap_removeall(self, true);
	holyground_remove(self, self->holyground);
	detector_removeall(self);
	caltrops_removeall(self);
	organ_removeall(self, "box", false);
	organ_removeall(self, "fmedi", false);
	organ_removeall(self, "gasser", false);
	organ_removeall(self, "obstacle", false);
	organ_removeall(self, "spiker", false);
	organ_removeall(self, "spikeball", false);
	organ_remove(self->healer, false);
	organ_remove(self->cocoon, false);
	spikegren_removeall(self);
	RemoveTotem(self->totem1);
	RemoveTotem(self->totem2);
	AuraRemove(self, 0);
	RemoveLasers(self);
	RemoveProxyGrenades(self);
	RemoveNapalmGrenades(self);
	RemoveExplodingArmor(self);
	RemoveAutoCannons(self);
	RemoveMiniSentries(self);
	RemoveAllLaserPlatforms(self);

	// scan for edicts that we own
	for (from = g_edicts; from<&g_edicts[globals.num_edicts]; from++)
	{
		edict_t *cl_ent;

		if (!from->inuse)
			continue;
		// remove sentry
		if ((from->creator) && (from->creator == self)
			&& !Q_stricmp(from->classname, "Sentry_Gun") && !RestorePreviousOwner(from))
		{
			sentrygun_die(from, NULL, from->creator, from->health, from->s.origin);
			continue;
		}

		// remove monsters
		if ((from->activator) && (from->activator == self)
			&& !Q_stricmp(from->classname, "drone") && !RestorePreviousOwner(from))
		{
			DroneRemoveSelected(self, from);
			M_Remove(from, false, true);
			self->num_monsters = 0;
			self->num_monsters_real = 0;
			continue;
		}

		// remove force wall
		if ((from->activator) && (from->activator == self)
			&& !Q_stricmp(from->classname, "Forcewall"))
		{
			from->think = BecomeTE;
			from->takedamage = DAMAGE_NO;
			from->deadflag = DEAD_DEAD;
			from->nextthink = level.time + FRAMETIME;
			continue;
		}

		// remove grenades
		if ((from->owner) && (from->owner == self)
			&& !Q_stricmp(from->classname, "grenade"))
		{
			self->max_pipes = 0;
			G_FreeEdict(from);
			continue;
		}

		// remove hammers
		cl_ent = G_GetClient(from);
		if (cl_ent && cl_ent->inuse && cl_ent == self
			&& !Q_stricmp(from->classname, "hammer"))
		{
			self->num_hammers = 0;
			G_FreeEdict(from);
			continue;
		}
	}
	// remove everything else
	if (self->lasersight)
	{
		G_FreeEdict(self->lasersight);
		self->lasersight = NULL;
	}
	if (self->flashlight)
	{
		G_FreeEdict(self->flashlight);
		self->flashlight = NULL;
	}

	if (self->supplystation)
	{
		depot_remove(self->supplystation, self, true);
	}
	if (self->skull && !RestorePreviousOwner(self->skull))
	{
		//BecomeExplosion1(self->skull);
		self->skull->think = BecomeExplosion1;
		self->skull->takedamage = DAMAGE_NO;
		self->skull->deadflag = DEAD_DEAD;
		self->skull->nextthink = level.time + FRAMETIME;
		self->skull = NULL;
	}
	if (self->magmine)
	{
		//BecomeExplosion1(self->magmine);
		self->magmine->think = BecomeExplosion1;
		self->magmine->takedamage = DAMAGE_NO;
		self->magmine->deadflag = DEAD_DEAD;
		self->magmine->nextthink = level.time + FRAMETIME;
		self->magmine = NULL;
	}
	if (self->spirit)
	{
		self->spirit->think = G_FreeEdict;
		self->spirit->deadflag = DEAD_DEAD;
		self->spirit->nextthink = level.time + FRAMETIME;
		self->spirit = NULL;
	}
	hook_reset(self->client->hook);
}

int vrx_GetMonsterCost(int mtype)
{
	int cost;

	switch (mtype)
	{
	case M_FLYER: cost = M_FLYER_COST; break;
	case M_INSANE: cost = M_INSANE_COST; break;
	case M_SOLDIERLT: cost = M_SOLDIERLT_COST; break;
	case M_SOLDIER: cost = M_SOLDIER_COST; break;
	case M_GUNNER: cost = M_GUNNER_COST; break;
	case M_CHICK: cost = M_CHICK_COST; break;
	case M_PARASITE: cost = M_PARASITE_COST; break;
	case M_MEDIC: cost = M_MEDIC_COST; break;
	case M_BRAIN: cost = M_BRAIN_COST; break;
	case M_TANK: cost = M_TANK_COST; break;
	case M_HOVER: cost = M_HOVER_COST; break;
	case M_SUPERTANK: cost = M_SUPERTANK_COST; break;
	case M_COMMANDER: cost = M_COMMANDER_COST; break;
	default: cost = M_DEFAULT_COST; break;
	}
	return cost;
}

int vrx_GetMonsterControlCost(int mtype)
{
	int cost;

	switch (mtype)
	{
	case M_FLYER: cost = M_FLYER_CONTROL_COST; break;
	case M_INSANE: cost = M_INSANE_CONTROL_COST; break;
	case M_SOLDIERLT: cost = M_SOLDIERLT_CONTROL_COST; break;
	case M_SOLDIER: cost = M_SOLDIER_CONTROL_COST; break;
	case M_GUNNER: cost = M_GUNNER_CONTROL_COST; break;
	case M_CHICK: cost = M_CHICK_CONTROL_COST; break;
	case M_PARASITE: cost = M_PARASITE_CONTROL_COST; break;
	case M_MEDIC: cost = M_MEDIC_CONTROL_COST; break;
	case M_BRAIN: cost = M_BRAIN_CONTROL_COST; break;
	case M_TANK: cost = M_TANK_CONTROL_COST; break;
	case M_HOVER: cost = M_HOVER_CONTROL_COST; break;
	case M_SUPERTANK: cost = M_SUPERTANK_CONTROL_COST; break;
	case M_COMMANDER: cost = M_COMMANDER_CONTROL_COST; break;
	default: cost = M_DEFAULT_CONTROL_COST; break;
	}
	return cost;
}

/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT
Target: next path corner
Pathtarget: gets used when an entity that has
this path_corner targeted touches it
*/

void path_corner_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		v;
	edict_t		*next;

	if (other->movetarget != self)
		return;

	if (other->enemy)
		return;

	if (self->pathtarget)
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		G_UseTargets(self, other);
		self->target = savetarget;
	}

	if (self->target)
		next = G_PickTarget(self->target);
	else
		next = NULL;

	if ((next) && (next->spawnflags & 1))
	{
		VectorCopy(next->s.origin, v);
		v[2] += next->mins[2];
		v[2] -= other->mins[2];
		VectorCopy(v, other->s.origin);
		next = G_PickTarget(next->target);
	}

	other->goalentity = other->movetarget = next;

	if (self->wait)
	{
		other->monsterinfo.pausetime = level.time + self->wait;
		other->monsterinfo.stand(other);
		return;
	}

	if (!other->movetarget)
	{
		other->monsterinfo.pausetime = level.time + 100000000;
		other->monsterinfo.stand(other);
	}
	else
	{
		VectorSubtract(other->goalentity->s.origin, other->s.origin, v);
		other->ideal_yaw = vectoyaw(v);
	}
}

void SP_path_corner(edict_t *self)
{
	if (!self->targetname)
	{
		gi.dprintf("path_corner with no targetname at %s\n", vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	self->solid = SOLID_TRIGGER;
	self->touch = path_corner_touch;
	VectorSet(self->mins, -8, -8, -8);
	VectorSet(self->maxs, 8, 8, 8);
	self->svflags |= SVF_NOCLIENT;
	gi.linkentity(self);
}


/*QUAKED point_combat (0.5 0.3 0) (-8 -8 -8) (8 8 8) Hold
Makes this the target of a monster and it will head here
when first activated before going after the activator.  If
hold is selected, it will stay here.
*/
void point_combat_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*activator;

	if (other->movetarget != self)
		return;

	if (self->target)
	{
		other->target = self->target;
		other->goalentity = other->movetarget = G_PickTarget(other->target);
		if (!other->goalentity)
		{
			gi.dprintf("%s at %s target %s does not exist\n", self->classname, vtos(self->s.origin), self->target);
			other->movetarget = self;
		}
		self->target = NULL;
	}
	else if ((self->spawnflags & 1) && !(other->flags & (FL_SWIM | FL_FLY)))
	{
		other->monsterinfo.pausetime = level.time + 100000000;
		other->monsterinfo.aiflags |= AI_STAND_GROUND;
		other->monsterinfo.stand(other);
	}

	if (other->movetarget == self)
	{
		other->target = NULL;
		other->movetarget = NULL;
		other->goalentity = other->enemy;
		other->monsterinfo.aiflags &= ~AI_COMBAT_POINT;
	}

	if (self->pathtarget)
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		if (other->enemy && other->enemy->client)
			activator = other->enemy;
		else if (other->oldenemy && other->oldenemy->client)
			activator = other->oldenemy;
		else if (other->activator && other->activator->client)
			activator = other->activator;
		else
			activator = other;
		G_UseTargets(self, activator);
		self->target = savetarget;
	}
}

void SP_point_combat(edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict(self);
		return;
	}
	self->solid = SOLID_TRIGGER;
	self->touch = point_combat_touch;
	VectorSet(self->mins, -8, -8, -16);
	VectorSet(self->maxs, 8, 8, 16);
	self->svflags = SVF_NOCLIENT;
	gi.linkentity(self);
};


/*QUAKED viewthing (0 .5 .8) (-8 -8 -8) (8 8 8)
Just for the debugging level.  Don't use
*/
void TH_viewthing(edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 7;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_viewthing(edict_t *ent)
{
	gi.dprintf("viewthing spawned\n");

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.renderfx = RF_FRAMELERP;
	VectorSet(ent->mins, -16, -16, -24);
	VectorSet(ent->maxs, 16, 16, 32);
	ent->s.modelindex = gi.modelindex("models/objects/banner/tris.md2");
	gi.linkentity(ent);
	ent->nextthink = level.time + 0.5;
	ent->think = TH_viewthing;
	return;
}


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for spotlights, etc.
*/
void SP_info_null(edict_t *self)
{
	G_FreeEdict(self);
};


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for lightning.
*/
void SP_info_notnull(edict_t *self)
{
	VectorCopy(self->s.origin, self->absmin);
	VectorCopy(self->s.origin, self->absmax);
};


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) START_OFF
Non-displayed light.
Default light value is 300.
Default style is 0.
If targeted, will toggle between on and off.
Default _cone value is 10 (used to set size of light for spotlights)
*/

#define START_OFF	1

static void light_use(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & START_OFF)
	{
		gi.configstring(CS_LIGHTS + self->style, "m");
		self->spawnflags &= ~START_OFF;
	}
	else
	{
		gi.configstring(CS_LIGHTS + self->style, "a");
		self->spawnflags |= START_OFF;
	}
}

void SP_light(edict_t *self)
{
	// no targeted lights in deathmatch, because they cause global messages
	if (!self->targetname || deathmatch->value)
	{
		G_FreeEdict(self);
		return;
	}

	if (self->style >= 32)
	{
		self->use = light_use;
		if (self->spawnflags & START_OFF)
			gi.configstring(CS_LIGHTS + self->style, "a");
		else
			gi.configstring(CS_LIGHTS + self->style, "m");
	}
}


/*QUAKED func_wall (0 .5 .8) ? TRIGGER_SPAWN TOGGLE START_ON ANIMATED ANIMATED_FAST
This is just a solid wall if not inhibited

TRIGGER_SPAWN	the wall will not be present until triggered
it will then blink in to existance; it will
kill anything that was in it's way

TOGGLE			only valid for TRIGGER_SPAWN walls
this allows the wall to be turned on and off

START_ON		only valid for TRIGGER_SPAWN walls
the wall will initially be present
*/

void func_wall_use(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
	{
		self->solid = SOLID_BSP;
		self->svflags &= ~SVF_NOCLIENT;
		KillBox(self);
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}
	gi.linkentity(self);

	if (!(self->spawnflags & 2))
		self->use = NULL;
}

void SP_func_wall(edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel(self, self->model);

	if (self->spawnflags & 8)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 16)
		self->s.effects |= EF_ANIM_ALLFAST;

	// just a wall
	if ((self->spawnflags & 7) == 0)
	{
		self->solid = SOLID_BSP;
		gi.linkentity(self);
		return;
	}

	// it must be TRIGGER_SPAWN
	if (!(self->spawnflags & 1))
	{
		//		gi.dprintf("func_wall missing TRIGGER_SPAWN\n");
		self->spawnflags |= 1;
	}

	// yell if the spawnflags are odd
	if (self->spawnflags & 4)
	{
		if (!(self->spawnflags & 2))
		{
			gi.dprintf("func_wall START_ON without TOGGLE\n");
			self->spawnflags |= 2;
		}
	}

	self->use = func_wall_use;
	if (self->spawnflags & 4)
	{
		self->solid = SOLID_BSP;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}
	gi.linkentity(self);
}


/*QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
This is solid bmodel that will fall if it's support it removed.
*/

void func_object_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// only squash thing we fall on top of
	if (!plane)
		return;
	if (plane->normal[2] < 1.0)
		return;
	if (other->takedamage == DAMAGE_NO)
		return;
	T_Damage(other, self, self, vec3_origin, self->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void func_object_release(edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->touch = func_object_touch;
}

void func_object_use(edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox(self);
	func_object_release(self);
}

void SP_func_object(edict_t *self)
{
	gi.setmodel(self, self->model);

	self->mins[0] += 1;
	self->mins[1] += 1;
	self->mins[2] += 1;
	self->maxs[0] -= 1;
	self->maxs[1] -= 1;
	self->maxs[2] -= 1;

	if (!self->dmg)
		self->dmg = 100;

	if (self->spawnflags == 0)
	{
		self->solid = SOLID_BSP;
		self->movetype = MOVETYPE_PUSH;
		self->think = func_object_release;
		self->nextthink = level.time + 2 * FRAMETIME;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->movetype = MOVETYPE_PUSH;
		self->use = func_object_use;
		self->svflags |= SVF_NOCLIENT;
	}

	if (self->spawnflags & 2)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 4)
		self->s.effects |= EF_ANIM_ALLFAST;

	self->clipmask = MASK_MONSTERSOLID;

	gi.linkentity(self);
}


/*QUAKED func_explosive (0 .5 .8) ? Trigger_Spawn ANIMATED ANIMATED_FAST
Any brush that you want to explode or break apart.  If you want an
ex0plosion, set dmg and it will do a radius explosion of that amount
at the center of the bursh.

If targeted it will not be shootable.

health defaults to 100.

mass defaults to 75.  This determines how much debris is emitted when
it explodes.  You get one large chunk per 100 of mass (up to 8) and
one small chunk per 25 of mass (up to 16).  So 800 gives the most.
*/
void func_explosive_explode(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t	origin;
	vec3_t	chunkorigin;
	vec3_t	size;
	int		count;
	int		mass;

	// bmodel origins are (0 0 0), we need to adjust that here
	VectorScale(self->size, 0.5, size);
	VectorAdd(self->absmin, size, origin);
	VectorCopy(origin, self->s.origin);

	self->takedamage = DAMAGE_NO;

	if (self->dmg)
		T_RadiusDamage(self, attacker, self->dmg, NULL, self->dmg + 40, MOD_EXPLOSIVE);

	VectorSubtract(self->s.origin, inflictor->s.origin, self->velocity);
	VectorNormalize(self->velocity);
	VectorScale(self->velocity, 150, self->velocity);

	// start chunks towards the center
	VectorScale(size, 0.5, size);

	mass = self->mass;
	if (!mass)
		mass = 75;

	// big chunks
	if (mass >= 100)
	{
		count = mass / 100;
		if (count > 8)
			count = 8;
		while (count--)
		{
			chunkorigin[0] = origin[0] + crandom() * size[0];
			chunkorigin[1] = origin[1] + crandom() * size[1];
			chunkorigin[2] = origin[2] + crandom() * size[2];
			ThrowDebris(self, "models/objects/debris1/tris.md2", 1, chunkorigin);
		}
	}

	// small chunks
	count = mass / 25;
	if (count > 16)
		count = 16;
	while (count--)
	{
		chunkorigin[0] = origin[0] + crandom() * size[0];
		chunkorigin[1] = origin[1] + crandom() * size[1];
		chunkorigin[2] = origin[2] + crandom() * size[2];
		ThrowDebris(self, "models/objects/debris2/tris.md2", 2, chunkorigin);
	}

	G_UseTargets(self, attacker);

	if (self->dmg)
		BecomeExplosion1(self);
	else
		G_FreeEdict(self);
}

void func_explosive_use(edict_t *self, edict_t *other, edict_t *activator)
{
	func_explosive_explode(self, self, other, self->health, vec3_origin);
}

void func_explosive_spawn(edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox(self);
	gi.linkentity(self);
}

void SP_func_explosive(edict_t *self)
{
	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict(self);
		return;
	}

	self->movetype = MOVETYPE_PUSH;

	gi.modelindex("models/objects/debris1/tris.md2");
	gi.modelindex("models/objects/debris2/tris.md2");

	gi.setmodel(self, self->model);

	if (self->spawnflags & 1)
	{
		self->svflags |= SVF_NOCLIENT;
		self->solid = SOLID_NOT;
		self->use = func_explosive_spawn;
	}
	else
	{
		self->solid = SOLID_BSP;
		if (self->targetname)
			self->use = func_explosive_use;
	}

	if (self->spawnflags & 2)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 4)
		self->s.effects |= EF_ANIM_ALLFAST;

	if (self->use != func_explosive_use)
	{
		if (!self->health)
			self->health = 100;
		self->die = func_explosive_explode;
		self->takedamage = DAMAGE_YES;
	}

	gi.linkentity(self);
}


/*QUAKED misc_explobox (0 .5 .8) (-16 -16 0) (16 16 40)
Large exploding box.  You can override its mass (100),
health (80), and dmg (150).
*/

void barrel_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)

{
	float	ratio;
	vec3_t	v;

	if ((!other->groundentity) || (other->groundentity == self))
		return;

	ratio = (float)other->mass / (float)self->mass;
	VectorSubtract(self->s.origin, other->s.origin, v);
	//	M_walkmove (self, vectoyaw(v), 20 * ratio * FRAMETIME);
}

void barrel_explode(edict_t *self)
{
	vec3_t	org;
	float	spd;
	vec3_t	save;

	T_RadiusDamage(self, self->activator, self->dmg, NULL, self->dmg + 40, MOD_BARREL);

	VectorCopy(self->s.origin, save);
	VectorMA(self->absmin, 0.5, self->size, self->s.origin);

	// a few big chunks
	spd = 1.5 * (float)self->dmg / 200.0;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris(self, "models/objects/debris1/tris.md2", spd, org);

	// bottom corners
	spd = 1.75 * (float)self->dmg / 200.0;
	VectorCopy(self->absmin, org);
	ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);
	VectorCopy(self->absmin, org);
	org[0] += self->size[0];
	ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);
	VectorCopy(self->absmin, org);
	org[1] += self->size[1];
	ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);
	VectorCopy(self->absmin, org);
	org[0] += self->size[0];
	org[1] += self->size[1];
	ThrowDebris(self, "models/objects/debris3/tris.md2", spd, org);

	// a bunch of little chunks
	spd = 2 * self->dmg / 200;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris(self, "models/objects/debris2/tris.md2", spd, org);

	VectorCopy(save, self->s.origin);
	if (self->groundentity)
		BecomeExplosion2(self);
	else
		BecomeExplosion1(self);
}

void barrel_delay(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + 2 * FRAMETIME;
	self->think = barrel_explode;
	self->activator = attacker;
}

void SP_misc_explobox(edict_t *self)
{
	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict(self);
		return;
	}

	gi.modelindex("models/objects/debris1/tris.md2");
	gi.modelindex("models/objects/debris2/tris.md2");
	gi.modelindex("models/objects/debris3/tris.md2");

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_STEP;

	self->model = "models/objects/barrels/tris.md2";
	self->s.modelindex = gi.modelindex(self->model);
	VectorSet(self->mins, -16, -16, 0);
	VectorSet(self->maxs, 16, 16, 40);

	if (!self->mass)
		self->mass = 400;
	if (!self->health)
		self->health = 10;
	if (!self->dmg)
		self->dmg = 150;

	self->die = barrel_delay;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.aiflags = AI_NOSTEP;

	self->touch = barrel_touch;

	self->think = M_droptofloor;
	self->nextthink = level.time + 2 * FRAMETIME;

	gi.linkentity(self);
}


//
// miscellaneous specialty items
//

/*QUAKED misc_blackhole (1 .5 0) (-8 -8 -8) (8 8 8)
*/

void misc_blackhole_use(edict_t *ent, edict_t *other, edict_t *activator)
{
	/*
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BOSSTPORT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	*/
	G_FreeEdict(ent);
}

void misc_blackhole_think(edict_t *self)
{
	if (++self->s.frame < 19)
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = 0;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_blackhole(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	VectorSet(ent->mins, -64, -64, 0);
	VectorSet(ent->maxs, 64, 64, 8);
	ent->s.modelindex = gi.modelindex("models/objects/black/tris.md2");
	ent->s.renderfx = RF_TRANSLUCENT;
	ent->use = misc_blackhole_use;
	ent->think = misc_blackhole_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity(ent);
}

/*QUAKED misc_eastertank (1 .5 0) (-32 -32 -16) (32 32 32)
*/

void misc_eastertank_think(edict_t *self)
{
	if (++self->s.frame < 293)
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = 254;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_eastertank(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet(ent->mins, -32, -32, -16);
	VectorSet(ent->maxs, 32, 32, 32);
	ent->s.modelindex = gi.modelindex("models/monsters/tank/tris.md2");
	ent->s.frame = 254;
	ent->think = misc_eastertank_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity(ent);
}

/*QUAKED misc_easterchick (1 .5 0) (-32 -32 0) (32 32 32)
*/


void misc_easterchick_think(edict_t *self)
{
	if (++self->s.frame < 247)
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = 208;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_easterchick(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet(ent->mins, -32, -32, 0);
	VectorSet(ent->maxs, 32, 32, 32);
	ent->s.modelindex = gi.modelindex("models/monsters/bitch/tris.md2");
	ent->s.frame = 208;
	ent->think = misc_easterchick_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity(ent);
}

/*QUAKED misc_easterchick2 (1 .5 0) (-32 -32 0) (32 32 32)
*/


void misc_easterchick2_think(edict_t *self)
{
	if (++self->s.frame < 287)
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = 248;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_easterchick2(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet(ent->mins, -32, -32, 0);
	VectorSet(ent->maxs, 32, 32, 32);
	ent->s.modelindex = gi.modelindex("models/monsters/bitch/tris.md2");
	ent->s.frame = 248;
	ent->think = misc_easterchick2_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity(ent);
}


/*QUAKED monster_commander_body (1 .5 0) (-32 -32 0) (32 32 48)
Not really a monster, this is the Tank Commander's decapitated body.
There should be a item_commander_head that has this as it's target.
*/

void commander_body_think(edict_t *self)
{
	if (++self->s.frame < 24)
		self->nextthink = level.time + FRAMETIME;
	else
		self->nextthink = 0;

	if (self->s.frame == 22)
		gi.sound(self, CHAN_BODY, gi.soundindex("tank/thud.wav"), 1, ATTN_NORM, 0);
}

void commander_body_use(edict_t *self, edict_t *other, edict_t *activator)
{
	self->think = commander_body_think;
	self->nextthink = level.time + FRAMETIME;
	gi.sound(self, CHAN_BODY, gi.soundindex("tank/pain.wav"), 1, ATTN_NORM, 0);
}

void commander_body_drop(edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->s.origin[2] += 2;
}

void SP_monster_commander_body(edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;
	self->model = "models/monsters/commandr/tris.md2";
	self->s.modelindex = gi.modelindex(self->model);
	VectorSet(self->mins, -32, -32, 0);
	VectorSet(self->maxs, 32, 32, 48);
	self->use = commander_body_use;
	self->takedamage = DAMAGE_YES;
	self->flags = FL_GODMODE;
	self->s.renderfx |= RF_FRAMELERP;
	gi.linkentity(self);

	gi.soundindex("tank/thud.wav");
	gi.soundindex("tank/pain.wav");

	self->think = commander_body_drop;
	self->nextthink = level.time + 5 * FRAMETIME;
}


/*QUAKED misc_banner (1 .5 0) (-4 -4 -4) (4 4 4)
The origin is the bottom of the banner.
The banner is 128 tall.
*/
void misc_banner_think(edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_banner(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex("models/objects/banner/tris.md2");
	ent->s.frame = rand() % 16;
	gi.linkentity(ent);

	ent->think = misc_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}

/*QUAKED misc_deadsoldier (1 .5 0) (-16 -16 0) (16 16 16) ON_BACK ON_STOMACH BACK_DECAP FETAL_POS SIT_DECAP IMPALED
This is the dead player model. Comes in 6 exciting different poses!
*/
void misc_deadsoldier_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	if (self->health > -80)
		return;

	gi.sound(self, CHAN_BODY, gi.soundindex("misc/udeath.wav"), 1, ATTN_NORM, 0);
	for (n = 0; n < 4; n++)
		ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
	ThrowHead(self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
}

void SP_misc_deadsoldier(edict_t *ent)
{
	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict(ent);
		return;
	}

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex("models/deadbods/dude/tris.md2");

	// Defaults to frame 0
	if (ent->spawnflags & 2)
		ent->s.frame = 1;
	else if (ent->spawnflags & 4)
		ent->s.frame = 2;
	else if (ent->spawnflags & 8)
		ent->s.frame = 3;
	else if (ent->spawnflags & 16)
		ent->s.frame = 4;
	else if (ent->spawnflags & 32)
		ent->s.frame = 5;
	else
		ent->s.frame = 0;

	VectorSet(ent->mins, -16, -16, 0);
	VectorSet(ent->maxs, 16, 16, 16);
	ent->deadflag = DEAD_DEAD;
	ent->takedamage = DAMAGE_YES;
	ent->svflags |= SVF_MONSTER | SVF_DEADMONSTER;
	ent->die = misc_deadsoldier_die;
	ent->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity(ent);
}

/*QUAKED misc_viper (1 .5 0) (-16 -16 0) (16 16 32)
This is the Viper for the flyby bombing.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"		How fast the Viper should fly
*/

extern void train_use(edict_t *self, edict_t *other, edict_t *activator);
extern void func_train_find(edict_t *self);

void misc_viper_use(edict_t *self, edict_t *other, edict_t *activator)
{
	self->svflags &= ~SVF_NOCLIENT;
	self->use = train_use;
	train_use(self, other, activator);
}

void SP_misc_viper(edict_t *ent)
{
	if (!ent->target)
	{
		gi.dprintf("misc_viper without a target at %s\n", vtos(ent->absmin));
		G_FreeEdict(ent);
		return;
	}

	if (!ent->speed)
		ent->speed = 300;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex("models/ships/viper/tris.md2");
	VectorSet(ent->mins, -16, -16, 0);
	VectorSet(ent->maxs, 16, 16, 32);

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;
	ent->use = misc_viper_use;
	ent->svflags |= SVF_NOCLIENT;
	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	gi.linkentity(ent);
}


/*QUAKED misc_bigviper (1 .5 0) (-176 -120 -24) (176 120 72)
This is a large stationary viper as seen in Paul's intro
*/
void SP_misc_bigviper(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet(ent->mins, -176, -120, -24);
	VectorSet(ent->maxs, 176, 120, 72);
	ent->s.modelindex = gi.modelindex("models/ships/bigviper/tris.md2");
	gi.linkentity(ent);
}


/*QUAKED misc_viper_bomb (1 0 0) (-8 -8 -8) (8 8 8)
"dmg"	how much boom should the bomb make?
*/
void misc_viper_bomb_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	G_UseTargets(self, self->activator);

	self->s.origin[2] = self->absmin[2] + 1;
	T_RadiusDamage(self, self, self->dmg, NULL, self->dmg + 40, MOD_BOMB);
	BecomeExplosion2(self);
}

void misc_viper_bomb_prethink(edict_t *self)
{
	vec3_t	v;
	float	diff;

	self->groundentity = NULL;

	diff = self->timestamp - level.time;
	if (diff < -1.0)
		diff = -1.0;

	VectorScale(self->moveinfo.dir, 1.0 + diff, v);
	v[2] = diff;

	diff = self->s.angles[2];
	vectoangles(v, self->s.angles);
	self->s.angles[2] = diff + 10;
}

void misc_viper_bomb_use(edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*viper;

	self->solid = SOLID_BBOX;
	self->svflags &= ~SVF_NOCLIENT;
	self->s.effects |= EF_ROCKET;
	self->use = NULL;
	self->movetype = MOVETYPE_TOSS;
	self->prethink = misc_viper_bomb_prethink;
	self->touch = misc_viper_bomb_touch;
	self->activator = activator;

	viper = G_Find(NULL, FOFS(classname), "misc_viper");
	VectorScale(viper->moveinfo.dir, viper->moveinfo.speed, self->velocity);

	self->timestamp = level.time;
	VectorCopy(viper->moveinfo.dir, self->moveinfo.dir);
}

void SP_misc_viper_bomb(edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	VectorSet(self->mins, -8, -8, -8);
	VectorSet(self->maxs, 8, 8, 8);

	self->s.modelindex = gi.modelindex("models/objects/bomb/tris.md2");

	if (!self->dmg)
		self->dmg = 1000;

	self->use = misc_viper_bomb_use;
	self->svflags |= SVF_NOCLIENT;

	gi.linkentity(self);
}


/*QUAKED misc_strogg_ship (1 .5 0) (-16 -16 0) (16 16 32)
This is a Storgg ship for the flybys.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"		How fast it should fly
*/

extern void train_use(edict_t *self, edict_t *other, edict_t *activator);
extern void func_train_find(edict_t *self);

void misc_strogg_ship_use(edict_t *self, edict_t *other, edict_t *activator)
{
	self->svflags &= ~SVF_NOCLIENT;
	self->use = train_use;
	train_use(self, other, activator);
}

void SP_misc_strogg_ship(edict_t *ent)
{
	if (!ent->target)
	{
		gi.dprintf("%s without a target at %s\n", ent->classname, vtos(ent->absmin));
		G_FreeEdict(ent);
		return;
	}

	if (!ent->speed)
		ent->speed = 300;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex("models/ships/strogg1/tris.md2");
	VectorSet(ent->mins, -16, -16, 0);
	VectorSet(ent->maxs, 16, 16, 32);

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;
	ent->use = misc_strogg_ship_use;
	ent->svflags |= SVF_NOCLIENT;
	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	gi.linkentity(ent);
}

// RAFAEL 17-APR-98
/*QUAKED misc_transport (1 0 0) (-8 -8 -8) (8 8 8) TRIGGER_SPAWN
Maxx's transport at end of game
*/
void SP_misc_transport(edict_t *ent)
{

	if (!ent->target)
	{
		gi.dprintf("%s without a target at %s\n", ent->classname, vtos(ent->absmin));
		G_FreeEdict(ent);
		return;
	}

	if (!ent->speed)
		ent->speed = 300;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex("models/objects/ship/tris.md2");

	VectorSet(ent->mins, -16, -16, 0);
	VectorSet(ent->maxs, 16, 16, 32);

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;
	ent->use = misc_strogg_ship_use;
	ent->svflags |= SVF_NOCLIENT;
	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	if (!(ent->spawnflags & 1))
	{
		ent->spawnflags |= 1;
	}

	gi.linkentity(ent);
}
// END 17-APR-98

/*QUAKED misc_satellite_dish (1 .5 0) (-64 -64 0) (64 64 128)
*/
void misc_satellite_dish_think(edict_t *self)
{
	self->s.frame++;
	if (self->s.frame < 38)
		self->nextthink = level.time + FRAMETIME;
}

void misc_satellite_dish_use(edict_t *self, edict_t *other, edict_t *activator)
{
	self->s.frame = 0;
	self->think = misc_satellite_dish_think;
	self->nextthink = level.time + FRAMETIME;
}

void SP_misc_satellite_dish(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet(ent->mins, -64, -64, 0);
	VectorSet(ent->maxs, 64, 64, 128);
	ent->s.modelindex = gi.modelindex("models/objects/satellite/tris.md2");
	ent->use = misc_satellite_dish_use;
	gi.linkentity(ent);
}


/*QUAKED light_mine1 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine1(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex("models/objects/minelite/light1/tris.md2");
	gi.linkentity(ent);
}


/*QUAKED light_mine2 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine2(edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex("models/objects/minelite/light2/tris.md2");
	gi.linkentity(ent);
}


/*QUAKED misc_gib_arm (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_arm(edict_t *ent)
{
	gi.setmodel(ent, "models/objects/gibs/arm/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random() * 200;
	ent->avelocity[1] = random() * 200;
	ent->avelocity[2] = random() * 200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	gi.linkentity(ent);
}

/*QUAKED misc_gib_leg (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_leg(edict_t *ent)
{
	gi.setmodel(ent, "models/objects/gibs/leg/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random() * 200;
	ent->avelocity[1] = random() * 200;
	ent->avelocity[2] = random() * 200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	gi.linkentity(ent);
}

/*QUAKED misc_gib_head (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_head(edict_t *ent)
{
	gi.setmodel(ent, "models/objects/gibs/head/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random() * 200;
	ent->avelocity[1] = random() * 200;
	ent->avelocity[2] = random() * 200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	gi.linkentity(ent);
}

//=====================================================

/*QUAKED target_character (0 0 1) ?
used with target_string (must be on same "team")
"count" is position in the string (starts at 1)
*/

void SP_target_character(edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel(self, self->model);
	self->solid = SOLID_BSP;
	self->s.frame = 12;
	gi.linkentity(self);
	return;
}


/*QUAKED target_string (0 0 1) (-8 -8 -8) (8 8 8)
*/

void target_string_use(edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *e;
	int		n, l;
	char	c;

	l = strlen(self->message);
	for (e = self->teammaster; e; e = e->teamchain)
	{
		if (!e->count)
			continue;
		n = e->count - 1;
		if (n > l)
		{
			e->s.frame = 12;
			continue;
		}

		c = self->message[n];
		if (c >= '0' && c <= '9')
			e->s.frame = c - '0';
		else if (c == '-')
			e->s.frame = 10;
		else if (c == ':')
			e->s.frame = 11;
		else
			e->s.frame = 12;
	}
}

void SP_target_string(edict_t *self)
{
	if (!self->message)
		self->message = "";
	self->use = target_string_use;
}


/*QUAKED func_clock (0 0 1) (-8 -8 -8) (8 8 8) TIMER_UP TIMER_DOWN START_OFF MULTI_USE
target a target_string with this

The default is to be a time of day clock

TIMER_UP and TIMER_DOWN run for "count" seconds and the fire "pathtarget"
If START_OFF, this entity must be used before it starts

"style"		0 "xx"
1 "xx:xx"
2 "xx:xx:xx"
*/

#define	CLOCK_MESSAGE_SIZE	16

// don't let field width of any clock messages change, or it
// could cause an overwrite after a game load

static void func_clock_reset(edict_t *self)
{
	self->activator = NULL;
	if (self->spawnflags & 1)
	{
		self->health = 0;
		self->wait = self->count;
	}
	else if (self->spawnflags & 2)
	{
		self->health = self->count;
		self->wait = 0;
	}
}

static void func_clock_format_countdown(edict_t *self)
{
	if (self->style == 0)
	{
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i", self->health);
		return;
	}

	if (self->style == 1)
	{
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i", self->health / 60, self->health % 60);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		return;
	}

	if (self->style == 2)
	{
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", self->health / 3600, (self->health - (self->health / 3600) * 3600) / 60, self->health % 60);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		if (self->message[6] == ' ')
			self->message[6] = '0';
		return;
	}
}

void func_clock_think(edict_t *self)
{
	if (!self->enemy)
	{
		self->enemy = G_Find(NULL, FOFS(targetname), self->target);
		if (!self->enemy)
			return;
	}

	if (self->spawnflags & 1)
	{
		func_clock_format_countdown(self);
		self->health++;
	}
	else if (self->spawnflags & 2)
	{
		func_clock_format_countdown(self);
		self->health--;
	}
	else
	{
		struct tm	*ltime;
		time_t		gmtime;

		time(&gmtime);
		ltime = localtime(&gmtime);
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		if (self->message[6] == ' ')
			self->message[6] = '0';
	}

	self->enemy->message = self->message;
	self->enemy->use(self->enemy, self, self);

	if (((self->spawnflags & 1) && (self->health > self->wait)) ||
		((self->spawnflags & 2) && (self->health < self->wait)))
	{
		if (self->pathtarget)
		{
			char *savetarget;
			char *savemessage;

			savetarget = self->target;
			savemessage = self->message;
			self->target = self->pathtarget;
			self->message = NULL;
			G_UseTargets(self, self->activator);
			self->target = savetarget;
			self->message = savemessage;
		}

		if (!(self->spawnflags & 8))
			return;

		func_clock_reset(self);

		if (self->spawnflags & 4)
			return;
	}

	self->nextthink = level.time + 1;
}

void func_clock_use(edict_t *self, edict_t *other, edict_t *activator)
{
	if (!(self->spawnflags & 8))
		self->use = NULL;
	if (self->activator)
		return;
	self->activator = activator;
	self->think(self);
}

void SP_func_clock(edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("%s with no target at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	if ((self->spawnflags & 2) && (!self->count))
	{
		gi.dprintf("%s with no count at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict(self);
		return;
	}

	if ((self->spawnflags & 1) && (!self->count))
		self->count = 60 * 60;;

	func_clock_reset(self);

	self->message = V_Malloc(CLOCK_MESSAGE_SIZE, TAG_LEVEL);

	self->think = func_clock_think;

	if (self->spawnflags & 4)
		self->use = func_clock_use;
	else
		self->nextthink = level.time + 1;
}

//=================================================================================

void PM_UseTeleporter(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		i;
	edict_t	*dest;
	vec3_t	start;
	trace_t	tr;

	dest = G_Find(NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf("Couldn't find destination\n");
		return;
	}

	// place the entity slightly above the teleporter's destination target
	VectorCopy(dest->s.origin, start);
	start[2] += abs(other->mins[2]) - 14;

	// make sure we don't get stuck
	tr = gi.trace(start, other->mins, other->maxs, start, other, MASK_SHOT);
	if (tr.fraction < 1)
		return;

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity(other);

	// move entity into new position
	VectorCopy(start, other->s.origin);
	VectorCopy(start, other->s.old_origin);

	// clear the velocity and hold them in place briefly
	VectorClear(other->velocity);
	//other->owner->client->ps.pmove.pm_time = 160>>3;// hold time
	//other->owner->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->owner->s.event = EV_PLAYER_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i = 0; i<3; i++)
		other->owner->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i]
		- other->owner->client->resp.cmd_angles[i]);

	VectorClear(other->owner->s.angles);
	VectorClear(other->owner->client->ps.viewangles);
	VectorClear(other->owner->client->v_angle);

	// kill anything at the destination
	KillBox(other);

	gi.linkentity(other);
}

void teleporter_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*dest;
	int			i;

	// must be a live entity
	if (!G_EntIsAlive(other))
		return;
	// non-client check
	if (!other->client)
	{
		//4.05 allow player-monsters to use teleporters
		if (PM_MonsterHasPilot(other))
			PM_UseTeleporter(self, other, plane, surf);
		return;
	}

	dest = G_Find(NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf("Couldn't find destination\n");
		return;
	}

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity(other);

	VectorCopy(dest->s.origin, other->s.origin);
	VectorCopy(dest->s.origin, other->s.old_origin);
	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear(other->velocity);
	other->client->ps.pmove.pm_time = 160 >> 3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->owner->s.event = EV_PLAYER_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i = 0; i<3; i++)
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);

	VectorClear(other->s.angles);
	VectorClear(other->client->ps.viewangles);
	VectorClear(other->client->v_angle);

	// kill anything at the destination
	KillBox(other);

	gi.linkentity(other);
}

/*QUAKED misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16)
Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.
*/
void SP_misc_teleporter(edict_t *ent)
{
	edict_t		*trig;

	if (!ent->target)
	{
		gi.dprintf("teleporter without a target.\n");
		G_FreeEdict(ent);
		return;
	}

	gi.setmodel(ent, "models/objects/dmspot/tris.md2");
	ent->s.skinnum = 1;
	ent->s.effects = EF_TELEPORTER;
	ent->s.sound = gi.soundindex("world/amb10.wav");
	ent->solid = SOLID_BBOX;

	VectorSet(ent->mins, -32, -32, -24);
	VectorSet(ent->maxs, 32, 32, -16);
	gi.linkentity(ent);

	trig = G_Spawn();
	trig->touch = teleporter_touch;
	trig->solid = SOLID_TRIGGER;
	trig->target = ent->target;
	trig->owner = ent;
	VectorCopy(ent->s.origin, trig->s.origin);
	VectorSet(trig->mins, -8, -8, 8);
	VectorSet(trig->maxs, 8, 8, 24);
	gi.linkentity(trig);

}

/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
*/
void SP_misc_teleporter_dest(edict_t *ent)
{
	gi.setmodel(ent, "models/objects/dmspot/tris.md2");
	ent->s.skinnum = 0;
	ent->solid = SOLID_BBOX;
	//	ent->s.effects |= EF_FLIES;
	VectorSet(ent->mins, -32, -32, -24);
	VectorSet(ent->maxs, 32, 32, -16);
	gi.linkentity(ent);
}

/*QUAKED misc_amb4 (1 0 0) (-16 -16 -16) (16 16 16)
Mal's amb4 loop entity
*/
static int amb4sound;

void amb4_think(edict_t *ent)
{
	ent->nextthink = level.time + 2.7;
	gi.sound(ent, CHAN_VOICE, amb4sound, 1, ATTN_NONE, 0);
}

void SP_misc_amb4(edict_t *ent)
{
	ent->think = amb4_think;
	ent->nextthink = level.time + 1;
	amb4sound = gi.soundindex("world/amb4.wav");
	gi.linkentity(ent);
}

/*QUAKED misc_nuke (1 0 0) (-16 -16 -16) (16 16 16)
*/

void use_nuke(edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *from = g_edicts;

	for (; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (from == self)
			continue;
		if (from->client)
		{
			T_Damage(from, self, self, vec3_origin, from->s.origin, vec3_origin, 100000, 1, 0, MOD_TRAP);
		}
		else if (from->svflags & SVF_MONSTER)
		{
			G_FreeEdict(from);
		}
	}

	self->use = NULL;
}

void SP_misc_nuke(edict_t *ent)
{
	ent->use = use_nuke;
}