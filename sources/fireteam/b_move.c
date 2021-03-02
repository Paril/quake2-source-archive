#include "g_local.h"

#define steplen 24
#define jumplen 96

#define STATE_TOP 0
#define STATE_BOTTOM 1
#define STATE_UP 2
#define STATE_DOWN 3

//====================================================
// Signal bThink() to make bot crouch/duck down.
//====================================================
void bDuck(edict_t *bot) {

	// Already crouched?
	if (bot->is_crouching) return;

	// Signal bThink() to crouch.
	bot->maxs[2]=4;
}

//====================================================
// Make the bot jump up and forward to climb ladder
//====================================================
void bClimbLadder(edict_t *bot, usercmd_t *cmd) {
	vec3_t forward, up;

	// Restrict jumps to 1 every 3 frames.
	if (level.time < bot->last_jump) return;

	AngleVectors(bot->s.angles, forward, NULL, up);
	VectorClear(bot->velocity);

	// At top? Then leap forward with small jump up.
	if (bCanMove(bot, steplen, +1)) {
		VectorMA(bot->velocity, 100, forward, bot->velocity);
		VectorMA(bot->velocity, 50, up, bot->velocity);
		cmd->forwardmove=200; cmd->upmove=50;
		bot->on_ladder=false; }
	else {
		// Then jump up with small forward motion.
		VectorMA(bot->velocity, 50, forward, bot->velocity);
		VectorMA(bot->velocity, 100, up, bot->velocity);
		cmd->upmove=200; cmd->forwardmove=100; }

	bot->last_jump = level.time + 0.3;
}

//====================================================
// Swim Rapidly in the forward direction.
//====================================================
void bSwim(edict_t *bot, usercmd_t *cmd) {
	vec3_t forward, right, up;

	if (bot->is_crouching) return;

	if (level.time < bot->last_jump) return;

	AngleVectors(bot->s.angles, forward, right, up);
	VectorClear(bot->velocity);
	VectorMA(bot->velocity, 100, up, bot->velocity);

	if (bCanMoveForward(bot)) {
		cmd->forwardmove=200;
		VectorMA(bot->velocity, 100, forward, bot->velocity); }
	else {
		cmd->sidemove=200;
		VectorMA(bot->velocity, 100, right, bot->velocity); }

	cmd->upmove=20;

	bot->last_jump = level.time + 0.2;
}

//====================================================
//============ BOT JUMPING ROUTINES ==================
//====================================================

//====================================================
// Jump in the Backward direction.
//====================================================
qboolean bJumpBackward(edict_t *bot, usercmd_t *cmd) {
	vec3_t forward, up;

	if (!bCanJump(bot)) return false;

	if (!bCanMoveBackward(bot)) return false;

	AngleVectors(bot->s.angles, forward, NULL, up);
	VectorClear(bot->velocity);
	VectorMA(bot->velocity, -100, forward, bot->velocity);
	VectorMA(bot->velocity, 100, up, bot->velocity);

	cmd->upmove=300;
	cmd->forwardmove=-300;

	bot->last_jump = level.time + 1;

	return true;
}

//====================================================
// Jump in the Forward direction.
//====================================================
qboolean bJumpForward(edict_t *bot, usercmd_t *cmd) {
	vec3_t forward, up;

	if (!bCanJump(bot)) return false;

	if (!bCanMoveForward(bot)) return false;

	AngleVectors(bot->s.angles, forward, NULL, up);
	VectorClear(bot->velocity);
	VectorMA(bot->velocity, 100, forward, bot->velocity);
	VectorMA(bot->velocity, 100, up, bot->velocity);

	cmd->upmove=20;
	cmd->forwardmove=200;

	bot->last_jump = level.time + 1;

	return true;
}

//========================================================
// Jump 'straight up' (with small forward component).
//========================================================
qboolean bJumpUp(edict_t *bot, usercmd_t *cmd) {
	vec3_t forward, up;

	if (!bCanJump(bot)) return false;

	AngleVectors(bot->s.angles, forward, NULL, up);
	VectorClear(bot->velocity);
	VectorMA(bot->velocity, 100, up, bot->velocity);
	VectorMA(bot->velocity, 30, forward, bot->velocity);

	cmd->upmove = 300;
	cmd->forwardmove = 50;

	bot->last_jump = level.time + 0.5;

	return true;
}

//========================================================
// Bot on moving func_plat so keep'em still until it stops.
//========================================================
void bRidePlatform(edict_t *bot, usercmd_t *cmd, vec3_t cmd_angles) {

	if (level.time < bot->pausetime) return;

	// Platform moving? Freeze movement for a few frames.
	if ((bot->groundentity->moveinfo.state == STATE_UP)
	|| (bot->groundentity->moveinfo.state == STATE_DOWN)) {
		bot->pausetime = level.time + 0.3; // Delay 3 frames.
		return; }

	// Platform has hit bottom or top? Then, try to Jump Off!!
	if ((bot->groundentity->moveinfo.state == STATE_TOP)
	|| (bot->groundentity->moveinfo.state == STATE_BOTTOM))
		if ((VectorLength(bot->groundentity->velocity) < 1)
		&& (VectorLength(bot->groundentity->avelocity) < 1)) {
			bot->on_platform=false;
			// Try to jump forward..
			if (bJumpForward(bot, cmd))
				return;
			// Try to step forward..
			if (bCanMove(bot, 24, +1)) {
				cmd->forwardmove=200;
				return; }
			// Try to jump left
			if (bJumpLeft(bot, cmd))
				return;
			// Try to step left/right..
			if (bCanMove(bot, 24, +2))
				cmd->sidemove=-200;
			else
				cmd->sidemove=200; }
}

//====================================================
// Stand guard (within 60 units) of pos1 vector..
//====================================================
void bGuard(edict_t *bot, usercmd_t *cmd, vec3_t cmd_angles) {
	vec3_t v;

	// Stay within 60 units of pos1 vector
	VectorSubtract(bot->pos1, bot->s.origin, v);
	vectoangles(v, cmd_angles);
	if (VectorLength(v) > 60)
		cmd->forwardmove = 100;
}

//====================================================
// Follow the bossman around..
//====================================================
void bFollowBoss(edict_t *bot, usercmd_t *cmd, vec3_t cmd_angles) {
	vec3_t v;
	float dist;

	// Where'd he run off to now??
	if (!infront(bot, bot->bossman)) {
		bMoveToGoal(bot, bot->bossman, cmd, cmd_angles);
		return; }

	VectorSubtract(bot->bossman->s.origin, bot->s.origin, v);
	vectoangles(v, cmd_angles);
	dist=VectorLength(v);

	// If bossman standing on elevator then move a bit closer..
	if (bOnPlatform(bot->bossman) && dist > 32)
		cmd->forwardmove = 100;
	else
		// Stand no closer than 70 units!
		if (dist > 70)
			cmd->forwardmove = 300;
}

//=========================================================
// TRUE if jump 'left' was activated. Still face forward!
//=========================================================
qboolean bJumpLeft(edict_t *bot, usercmd_t *cmd) {
	vec3_t right, up;

	if (!bCanJump(bot)) return false;

	if (!bCanMove(bot,jumplen,+2)) return false;

	// NEED TO MODIFY bCanMove() TO TEST IF BOT COULD BE
	// JUMPING INTO LAVA/SLIME..

	AngleVectors(bot->s.angles, NULL, right, up);
	VectorClear(bot->velocity);
	VectorMA(bot->velocity, 100, up, bot->velocity);
	VectorMA(bot->velocity, -100, right, bot->velocity);

	cmd->sidemove=-300;
	cmd->upmove=50;

	bot->last_jump = level.time + 1;

	return true;
}

//==========================================================
// TRUE if jump 'right' was activated. Still face forward!
//==========================================================
qboolean bJumpRight(edict_t *bot, usercmd_t *cmd) {
	vec3_t right, up;

	if (!bCanJump(bot)) return false;

	if (!bCanMove(bot,jumplen,+3)) return false;

	// NEED TO MODIFY bCanMove() TO TEST IF BOT COULD BE
	// JUMPING INTO LAVA/SLIME..

	AngleVectors(bot->s.angles, NULL, right, up);
	VectorClear(bot->velocity);
	VectorMA(bot->velocity, 100, up, bot->velocity);
	VectorMA(bot->velocity, 100, right, bot->velocity);

	cmd->sidemove=300;
	cmd->upmove=50;

	bot->last_jump = level.time + 1;

	return true;
}

//=======================================================
// Cause the bot to dodge incoming flak!
//=======================================================
qboolean bDodgeFire(edict_t *bot, usercmd_t *cmd, vec3_t cmd_angles) {
	vec3_t forward, end;
	trace_t tr;

	// Check for incoming flak and take evasive action!!
	AngleVectors(bot->s.angles, forward, NULL, NULL);
	VectorMA(bot->s.origin, 1000, forward, end);
	tr=gi.trace(bot->s.origin, bot->mins, bot->maxs, end, bot, MASK_SHOT);
	if (tr.ent) {
		if (tr.ent->s.effects & (EF_BLASTER|EF_ROCKET|EF_GRENADE|EF_HYPERBLASTER|EF_BFG)) {
			// 50% of time do this...
			if (rand()%10<5) {
				if (!bJumpRight(bot,cmd)) {
					if (rand()%10<5) {
						if (bCanStep(bot, 90, 32, cmd_angles))
							cmd->sidemove=300;
						else
							if (bCanStep(bot, -90, 32, cmd_angles))
								cmd->sidemove=-300;}
				else
					bDuck(bot); } }
			else {
				// Else, try doing this..
				if (!bJumpLeft(bot,cmd)) {
					if (bCanStep(bot, 90, 32, cmd_angles))
						cmd->sidemove=300;
					else
						if (bCanStep(bot, -90, 32, cmd_angles))
							cmd->sidemove=-300;
						else
							bDuck(bot); }
				else
					bDuck(bot); }
			return true; } }

	return false;
}

//=======================================================
// Cause the bot to make an attack on its current enemy.
//=======================================================
void bAttack(edict_t *bot, usercmd_t *cmd, vec3_t cmd_angles) {
	vec3_t v;
	float dist;

	if (bEnemyIsDead(bot)
	// Don't follow the poor bastard into the lava/slime!!
	|| (gi.pointcontents(bot->enemy->s.origin) & (CONTENTS_LAVA|CONTENTS_SLIME))) {
		bot->enemy = NULL;
		return; }

	// Climb up after the bastard!
	if (bot->on_ladder)
		bClimbLadder(bot, cmd);
	else
		// Where'd he go??
		if (!infront(bot, bot->enemy)) {
			bMoveToGoal(bot, bot->bossman, cmd, cmd_angles);
			return; }

	// How far away is this enemy?
	VectorSubtract(bot->enemy->s.origin, bot->s.origin, v);
	dist=VectorLength(v);

	// Got appropriate weapon?
	if (dist > 700)
		GetFarWeapon(bot);
	else if (dist < 160)
		GetCloseWeapon(bot);
	else
		GetBestWeapon(bot);

	// If bot just switched weapon, delay firing for 3 frames.
	if (bot->client->pers.weapon != bot->client->pers.lastweapon)
		bot->weap_delay = level.time + 0.3;

	// Keep facing your enemy
	vectoangles(v, cmd_angles);

	// Use ClimbLadder movements but still FIRE!
	if (!bot->on_ladder) {
		// Don't move closer than 160 units
		if (dist > 160)
			cmd->forwardmove = 300;
		else
			// 30% of time, move backward.
			if (rand()%10 < 3 || dist < 50)
				cmd->forwardmove = -300;
	else
		if (dist < 32) {
			// Hand-to-Hand combat!
			bSmackHit(bot);
			bJumpBackward(bot,cmd); } }

	// Weapon switching delay done?
	if (level.time > bot->weap_delay)
		if (visible(bot,bot->enemy))
			cmd->buttons = BUTTON_ATTACK;
}

//=========================================================
// Find a new direction toward bot's goal in the distance.
//=========================================================
void bNewChaseDir(edict_t *bot, edict_t *goal, float dist, vec3_t cmd_angles) {
	float d[3],deltax, deltay;
	float yaw, olddir, reversedir;

	// Must have a goal entity!
	if (!goal) return;

	olddir=anglemod((int)(bot->ideal_yaw/45)*45);
	reversedir=anglemod(olddir-180);

	deltax=goal->s.origin[0]-bot->s.origin[0];
	deltay=goal->s.origin[1]-bot->s.origin[1];

	if (deltax>10)
		d[1]=0;
	else if (deltax<-10)
		d[1]=180;
	else
		d[1]=-99;

	if (deltay<-10)
		d[2]=270;
	else if (deltay>10)
		d[2]=90;
	else
		d[2]=-99;

	// Try direct route first!
	if (d[1]!=-99 && d[2]!=-99) {
		if (d[1]==0)
			yaw=(d[2]==90)?45:315;
		else
			yaw=(d[2]==90)?135:215;
		if (yaw != reversedir)
		// Can we step in yaw*dist direction?
			if (bCanStep(bot, yaw, dist, cmd_angles))
			return; }

	// Rotate the vector coordinates around...
	if (((rand()&3)&1) || (abs(deltay)>abs(deltax))) {
		yaw=d[1]; d[1]=d[2]; d[2]=yaw; }

	// Let's try the d[1] direction?
	if (yaw!=-99 && yaw!=reversedir)
		// Can we step in d[1]*dist direction?
		if (bCanStep(bot, yaw, dist, cmd_angles))
			return;

	// Let's try the d[2] direction?
	yaw=d[2];
	if (yaw!=-99 && yaw!=reversedir)
		// Can we step in d[2]*dist direction?
		if (bCanStep(bot, yaw, dist, cmd_angles))
			return;

	// ----------------------------------------
	// Okay then, let's pick random direction
	// ----------------------------------------

	// Search from 0..315 in 45 deg increments?
	if (rand()&1) {
		for (yaw=0; yaw<=315; yaw+=45)
			if (yaw!=reversedir)
			// Can we step in yaw*dist direction?
				if (bCanStep(bot, yaw, dist, cmd_angles))
					return; }
	else
		// Search from 315..0 in -45 deg increments?
		for (yaw=315; yaw>=0; yaw-=45)
			if (yaw!=reversedir)
				// Can we step in yaw*dist direction?
				if (bCanStep(bot, yaw, dist, cmd_angles))
					return;

	// ----------------------------------------
	// Okay, Now we're getting desperate!
	// ----------------------------------------

	// Can we step in the reverse direction?
	if (bCanStep(bot, reversedir, dist, cmd_angles))
		return;
}

//========================================================
// Move the bot a single step towards its current goal.
//========================================================
void bMoveToGoal(edict_t *bot, edict_t *goal, usercmd_t *cmd, vec3_t cmd_angles) {
	float dist, yaw;
	vec3_t v1, v2, forward;

	if (!goal) return;

	VectorSubtract(goal->s.origin, bot->s.origin, v1);
	yaw = vectoyaw(v1);
	VectorSubtract(bot->s.origin, goal->s.origin, v2);
	dist = VectorLength(v2);

	if (!bCanStep(bot, yaw, dist, cmd_angles))
		bNewChaseDir(bot, goal, dist, cmd_angles);


	// ------------------------------------------------------
	// At this point, cmd_angles should have been changed..
	// ------------------------------------------------------

	// If goal above bot then perform a jump!
	if (goal->s.origin[2] > bot->s.origin[2]) {
		if (!bCanMoveForward(bot) || abs(goal->s.origin[2]-bot->s.origin[2]) < 32)
			bJumpForward(bot, cmd);
		else
			bot->itemwant=NULL; } // NEED TO TELL BOT ITEM CAN'T BE REACHED!
	else {
		AngleVectors(cmd_angles, forward, NULL, NULL);
		VectorMA(bot->velocity, 80, forward, bot->velocity);
		cmd->forwardmove=300; }
}
