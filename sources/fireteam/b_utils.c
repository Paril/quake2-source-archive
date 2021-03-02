#include "g_local.h"

void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);

//=======================================================
// Find the guard's initial starting origin by bossman.
//=======================================================
void bStartLocation(edict_t *ent, vec3_t result) {
	vec3_t forward, up, right, dummy;

	AngleVectors(ent->s.angles, forward, NULL, up);
	VectorClear(result);
	VectorMA(ent->s.origin, 50, forward, result);
	VectorMA(result, 50, forward, result);
	VectorMA(result, 1, up, result);

	if (gi.pointcontents(result) == CONTENTS_PLAYERCLIP)
	{
		//try again in another direction
		AngleVectors(ent->s.angles, NULL, right, up);
		VectorClear(result);
		VectorMA(ent->s.origin, 50, right, result);
		VectorMA(result, 50, right, result);
		VectorMA(result, 1, up, result);

		// test again
		if (gi.pointcontents(result) == CONTENTS_PLAYERCLIP)
		{
			// screw it and put him on a spawnpoint
			SelectSpawnPoint(ent, result, dummy);
		}
		else
			return;
	}
	else
		return;

}

//=======================================================
// Turn the bot and set cmd_angles accordingly.
//=======================================================
void bChangeYaw(edict_t *bot, vec3_t cmd_angles) {
	float ideal;
	float current;
	float move, speed;

	current = anglemod(cmd_angles[YAW]);
	ideal = bot->ideal_yaw;

	if (current == ideal) return;

	move = ideal - current;
	speed = bot->yaw_speed;

	if (ideal > current) {
		if (move >= 180)
			move = move - 360;}
	else {
		if (move <= -180)
			move = move + 360; }
	if (move > 0) {
		if (move > speed)
			move = speed; }
	else {
		if (move < -speed)
			move = -speed; }

	cmd_angles[YAW] = anglemod(current + move);
}

//========================================================
void bSmackHit(edict_t *bot) {
	vec3_t aim;

	if (level.time > bot->last_smack) return;

	VectorSet(aim, MELEE_DISTANCE, 0, 0);
	if (fire_hit(bot, aim, (5+(rand()%5)), 50))
		gi.sound(bot, CHAN_WEAPON, gi.soundindex("infantry/melee2.wav"), 1, ATTN_NORM, 0);

	bot->last_smack = level.time + 0.5;
}

//=======================================================
// True if bot's enemy is dead (or non-existent).
//===================================0====================
qboolean bEnemyIsDead(edict_t *bot) {

	if (!bot->enemy) return true;

	if ((bot->enemy->health < 1) || (bot->enemy->deadflag == DEAD_DEAD))
		return true;

	return false;
}

//=======================================================
// Returns tangles facing bot in opposite direction...
//=======================================================
void bAboutFace(edict_t *bot, vec3_t tangles) {
	float dir, reversedir;

	dir=anglemod(bot->s.angles[YAW]);
	reversedir=anglemod(dir-180)*(M_PI/180);

	tangles[0]=cos(reversedir);
	tangles[1]=sin(reversedir);
	tangles[2]=0;
}

//=======================================================
// Auto self-teleport if stuck someplace for 6 seconds.
//=======================================================
void bCheckIfStuck(edict_t *bot, usercmd_t *cmd) {
	vec3_t vec;

	if (bot->on_platform || bot->hold_position) return;

	VectorSubtract(bot->s.origin, bot->prev_origin, vec);

	if (VectorLength(vec) < 4) {
		if (((bot->timestuck + 6) < level.time)
		&& (G_Distance(bot, bot->bossman) > 160))
			bTeleport(bot);
			return; }

	VectorCopy(bot->s.origin, bot->prev_origin);
	bot->timestuck = level.time;
}

//==========================================================
// TRUE if bot teleports back to its guarding position.
//==========================================================
qboolean bTeleport(edict_t *bot) {
	vec3_t spawn_origin, spawn_angles;

	bot->hold_position=false; // Turn this OFF.

	if (!G_ClientInGame(bot)) return false;

	if (!G_ClientInGame(bot->bossman)) return false;

	// Large particle effect at player's old spot
	G_Spawn_Splash(TE_LASER_SPARKS, 24, 0xe2e5e3e6, bot->s.origin, tv(0,0,0), bot->s.origin);

	bStartLocation(bot->bossman, bot->pos1);

	VectorCopy(bot->pos1, spawn_origin);
	spawn_origin[2] += 1;
	VectorCopy(bot->bossman->s.angles, spawn_angles);

	// Copy over the player's last movement action!
	bot->client->ps.pmove.origin[0] = spawn_origin[0]*8;
	bot->client->ps.pmove.origin[1] = spawn_origin[1]*8;
	bot->client->ps.pmove.origin[2] = spawn_origin[2]*8;

	// Physically locate bot to spawn origin..
	VectorCopy(spawn_origin, bot->s.origin);

	// Teleport particle effect at bot's new origin.
	bot->s.event = EV_PLAYER_TELEPORT;

	// play teleport sound effects.
	gi.sound(bot, CHAN_VOICE, gi.soundindex("misc/tele1.wav"), 1, ATTN_NORM, 0);

	return true;
}

//========================================================
// Make bot guard your current vector (until death).
//========================================================
void bHoldPosition(edict_t *bot) {

	if (!G_ClientInGame(bot->bossman)) return;

	if (!G_ClientInGame(bot)) return;

	bot->hold_position=true;
	VectorCopy(bot->bossman->s.origin, bot->pos1);
	bot->itemwant=NULL;
	bot->enemy=NULL;
}

//========================================================
// Search 1000 unit radius and return 'closest' enemy.
//========================================================
edict_t *bSeekEnemy(edict_t *bot) {
	edict_t *ent=NULL, *best_ent=NULL;
	float this_dist=0, best_dist=8192;
	int i;

	// Can't seek new enemy while grappling..
	// Got one right away!!
	// Does it matter that this one may
	// not be the closest ent?
	if (bot->bossman->enemy)
		return bot->bossman->enemy;

	if (deathmatch->value) {
		for(i=0;i < game.maxclients;i++) {
			ent=g_edicts+i+1;
			if (!G_ClientInGame(ent)) continue;
			if (!ent->takedamage) continue;
			if (ent==bot || ent==bot->bossman) continue;
			if (!bCanSee(bot, ent)) continue;
			if (!G_Within_Radius(bot->s.origin, ent->s.origin, 1000)) continue;
			// Ignore the poor Bastards in Lava/Slime
			if (gi.pointcontents(ent->s.origin) & (CONTENTS_LAVA|CONTENTS_SLIME)) continue;
			// Which ent was closest?
			this_dist=G_Distance(bot, ent);
			if (this_dist < best_dist) {
				best_dist = this_dist;
				best_ent = ent; } } }
	else
		// Single Player Mode..
		while ((ent=findradius(ent, bot->s.origin, 1000))!= NULL) {
			if (!G_IsMonster(ent)) continue;
			if (!ent->takedamage) continue;
			if (ent==bot || ent==bot->bossman) continue;
			if (!bCanSee(bot, ent)) continue;
			this_dist=G_Distance(bot, ent);
			if (this_dist < best_dist) {
				best_dist = this_dist;
			best_ent = ent; } }

	return best_ent;
}
