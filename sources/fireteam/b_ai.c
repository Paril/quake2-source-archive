#include "g_local.h"

//==========================================================
// TRUE if bot currently standing on a 'func_plat' platform.
//===========================================================
qboolean bOnPlatform(edict_t *bot) {
	return ((bot->groundentity)
	&& (bot->groundentity->use==Use_Plat));
}

//=======================================================
// TRUE if bot is too far away from bossman's position.
//=======================================================
qboolean bTooFar(edict_t *bot) {
	return (G_Distance(bot, bot->bossman) > 1200);
}

//=======================================================
// TRUE if bot is 'touching' a ladder.
//=======================================================
qboolean bTouchingLadder(edict_t *bot) {
	return (gi.pointcontents(bot->s.origin) & CONTENTS_LADDER);
}

//=======================================================
// TRUE if bot is 'touching' a ladder.
//
// The trick here is to make the player's BBOX really narrow
// because the BBOX is so big (relatively speaking) that the
// gi.pointcontents() function returns true even though the
// ladder is off to one side of the bot. So, we temporarily
// shrink the size of the bot (for trace interaction purposes),
// and if the bot is touching the ladder now then it has a higher
// likelihood that the ladder will be actually 'infront' of the
// bot so the bot can proceed to climb it!!

// For some reason, this routine freezes bot at top of ladder!???'
//=======================================================
qboolean bTouchingLadder2(edict_t *bot) {
	vec3_t tmins, tmaxs, forward, end;
	trace_t tr;

	if (bot->is_crouching)
		return false;

	// Make BBOX really narrow (but same height)
	VectorSet(tmins, -2, -2, -24);
	VectorSet(tmaxs, 2, 2, 32);

	AngleVectors(bot->s.angles, forward, NULL, NULL);
	VectorMA(bot->s.origin, 2, forward, end);

	tr=gi.trace(bot->s.origin, tmins, tmaxs, end, bot, MASK_SHOT);

	return (tr.contents & CONTENTS_LADDER);
}

//=======================================================
// TRUE if bot can 'stand up' at present location.
//=======================================================
qboolean bCanStand(edict_t *bot) {
	vec3_t point;

	// I'm still standing better than I ever did...
	if (!bot->is_crouching) return true;

	VectorCopy(bot->s.origin, point);
	point[2] += bot->viewheight;

	return (!(gi.pointcontents(point) & MASK_SOLID));
}

//=======================================================
// TRUE if bot can 'jump backward' from present location.
//=======================================================
qboolean bCanMoveBackward(edict_t *bot) {
	vec3_t forward, end;

	AngleVectors(bot->s.angles, forward, NULL, NULL);
	VectorMA(bot->s.origin, -32, forward, end);
	return (!(gi.pointcontents(end) & MASK_SHOT));
}

//=======================================================
// TRUE if bot can 'jump forward' from present location.
//=======================================================
qboolean bCanMoveForward(edict_t *bot) {
	vec3_t forward, end;

	AngleVectors(bot->s.angles, forward, NULL, NULL);
	VectorMA(bot->s.origin, 32, forward, end);
	return (!(gi.pointcontents(end) & MASK_SHOT));
}

//=========================================================
// TRUE if bot can perform a jump from present location.
//=========================================================
qboolean bCanJump(edict_t *bot) {
	vec3_t point;

	// Can't jump from crouched position.
	if (bot->is_crouching) return false;

	// Already in the air?
	if (!bot->groundentity) return false;

	// Timer still active?
	if (level.time < bot->last_jump) return false;

	// Any obstructions 2X above bot's head?
	VectorCopy(bot->s.origin, point);
	point[2] += bot->viewheight*2; // 2X height

	return (gi.trace(bot->s.origin, bot->mins, bot->maxs, point, bot, MASK_PLAYERSOLID).fraction == 1);
}

//==========================================================
// TRUE if bot is in deep water.
//=========================================================
qboolean bCanDrown(edict_t *bot) {
	return (bot->waterlevel==3);
}

//=======================================================
// TRUE if object is 'visible' and 'infront' of bot.
//=======================================================
qboolean bCanSee(edict_t *bot, edict_t *object) {
	return (visible(bot,object)
	&& (infront(bot,object) || G_Distance(bot,object) < 160));
}

//=======================================================
// TRUE if bot can move in direction for length amount.
//
// Does a simple test to see if an obstruction is in the
// bot's path in the desired direction from start to end.
//
// dir = +1 (forward), dir = -1 (backward)
// dir = +2 (left), dir = +3 (right)
//=======================================================
qboolean bCanMove(edict_t *bot, float length, int dir) {
	vec3_t start, forward, right, end;

	AngleVectors(bot->s.angles, forward, NULL, NULL);
	VectorCopy(bot->s.origin, start);

	// Any offset to start vector?
	if (!bot->is_crouching)
		start[2] += bot->viewheight; // Standing.
	else
		start[2] += bot->maxs[2]; // Crouching.

	// NEED TO SEE IF LAVA/SLIME IS BETWEEN START AND END!

	switch (dir) {
		case 2: // Left = negative amount to the right
			VectorMA(start, -length, right, end);
			break;
		case 3: // Right = positive amount to the right
			VectorMA(start, length, right, end);
			break;
		default: // End is (+/-) length in forward direction
			VectorMA(start, dir*length, forward, end); }

	// Trace start to end and see if we hit anything solid.
	return (gi.trace(start, bot->mins, bot->maxs, end, bot, MASK_SHOT).fraction == 1);
}

//======================================================
// TRUE if bot can take the next step in yaw direction.
//
// This is a homegrown function. What I intended to achieve
// with this function was to trace a line 2 steplengths forward
// and then trace down to see what would the bot be stepping onto
// if it took the next step. So, we trace out then trace down and
// ask the world() to tell us is what is there.
//======================================================
qboolean bCanStep(edict_t *bot, float yaw, float dist, vec3_t cmd_angles) {
	vec3_t start, up, forward, end1, end2, tangle;
	trace_t tr;

	// Turn the bot
	bot->ideal_yaw=yaw;
	bChangeYaw(bot, cmd_angles);

	yaw *= (M_PI/180);
	tangle[0]=cos(yaw)*dist;
	tangle[1]=sin(yaw)*dist;
	tangle[2]=0;

	VectorCopy(bot->s.origin, start);

	// Any offsets needed?
	if (!bot->is_crouching)
		start[2] += bot->viewheight; // Standing.
	else
		start[2] += bot->maxs[2]; // Crouched.

	// Which way is forward from here?
	AngleVectors(tangle, forward, NULL, NULL);

	// end1 is out 48 units (2*steplength).
	VectorMA(start, 48, forward, end1);

	// Trace start to end1 and see if we hit anything.
	tr=gi.trace(start, bot->mins, bot->maxs, end1, bot, MASK_SHOT);

	// Up against a wall? Don't take the step..
	if (tr.fraction < 1) return false;

	// Trace down from end1 and see what we'll eventually step onto..
	// Use a slightly smaller BBOX for this second trace...
	VectorMA(end1, -1000, up, end2);
	tr=gi.trace(end1, tv(-8,-8,-8), tv(8,8,8), end2, bot, MASK_SOLID|MASK_WATER);

	// Don't step if trace hit lava/slime.
	return (!(tr.contents & (CONTENTS_LAVA|CONTENTS_SLIME)));
}

//=======================================================
// Main AI.. Called from bThink()..
//
// The use of the usercmd_t makes life a whole lot simpler.
// What we have to concern ourselves with is making a change
// to the cmd->forwardmove, cmd->sidemove, cmd->upmove and
// to the cmd_angles (which is the turn of the bot), and whether
// we want to add some additional velocity in any direction.
// Other than that, bThink() and the engine take care of the rest.
//
// Now, since bThink() will be called EVERY FRAME then this will
// be called each frame too! So, we only want to make ONE CHANGE
// to the usercmd_t (with any associated cmd_angle and/or velocity
// changes) then return as quickly as we can so that the new moves
// can be processed and sent to the engine. We'll make another change
// the next frame... And so on. And so on...
//
// The things which don't change the bot's movement (like the
// weapon change or the chat stuff) don't need to be immediately
// returned because these type things can be done in conjunction
// with movement. Otherwise, change movement and exit!! So, if
// you add any new functionality to the AI, keep this in mind else
// only the last movement changes will, of course, be processed..
//
// Actually, we can also add separate dummy ents whose sole function
// is to sit in the background and use their think functions to
// timely process in parallel some of the bot's AI stuff.
// Whoaaa!! That gave me some ideas!!
//=======================================================
void bAI(edict_t *bot, usercmd_t *cmd, vec3_t cmd_angles) {

	// -------------------------------------
	// Crouching, if any, has already
	// been processed so see if bot can
	// stand up this frame..
	// -------------------------------------
	if (bot->is_crouching)
		if (bCanStand(bot))
			bot->maxs[2] = 32;

	// ------------------------------------
	// Check for best weapon every 10 sec.
	// ------------------------------------
	if ((int)level.time%10 < 1)
		GetBestWeapon(bot);

	// ----------------------------
	// Do a bit of random chatting
	// ----------------------------
	if ((rand()%5) < 1 && (level.time - bot->last_chat > 10))
		bFakeChat(bot);

	// ------------------------------
	// Detect & Dodge incoming flak!
	// ------------------------------
	if (bDodgeFire(bot, cmd, cmd_angles))
		return;

	// --------------------------
	// Detect & Exit deep water.
	// --------------------------
	if (bCanDrown(bot)) {
		bSwim(bot,cmd);
		return; }

	// -------------------------------
	// Check if bot touching a ladder.
	// -------------------------------
	bot->on_ladder=bTouchingLadder(bot);
	if (bot->on_ladder) {
		bClimbLadder(bot, cmd);
		return; }

	// ---------------------------------
	// Check if bot riding a func_plat.
	// ---------------------------------
	bot->on_platform=bOnPlatform(bot);
	if (bot->on_platform) {
		bRidePlatform(bot, cmd, cmd_angles);
		return; }

	// ----------------------------
	// Check if bot is stuck!
	// ----------------------------
	bCheckIfStuck(bot, cmd);

	// -----------------------------
	// AutoTeleport if too far away.
	// -----------------------------
	if (!bot->hold_position)
		if (bTooFar(bot))
			if (bTeleport(bot)) {
				bot->itemwant=NULL;
				bot->enemy=NULL; // Start over.
				return; }

	// ---------------------------
	// Find & Attack your enemies.
	// ---------------------------
	if (!bot->enemy)
		bot->enemy = bSeekEnemy(bot);
		if (bot->enemy) {
			bot->itemwant=NULL;
			bAttack(bot, cmd, cmd_angles);
			return; }

	// -----------------------------
	// Look for high priority items!
	// -----------------------------
	if (!bot->itemwant)
		bot->itemwant = bSeekItem(bot);
		if (bot->itemwant!=NULL) {
			if (!visible(bot, bot->itemwant))
				bot->itemwant=NULL;
			else
				bMoveToGoal(bot, bot->itemwant, cmd, cmd_angles);
		return; }

	// ----------------------------
	// Guard vector bot->pos1!
	// ----------------------------
	if (bot->hold_position) {
		bGuard(bot, cmd, cmd_angles);
		return; }

	// ----------------------------
	// Follow your bossman around.
	// ----------------------------

	bFollowBoss(bot, cmd, cmd_angles);
}

