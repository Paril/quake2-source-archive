/*
e_grapple.c
Rich 'Publius' Tollerton
Charles 'Myrkul' Kendrick

Code for handling the Expert offhand grappling hook
and for Expert Pogo, which is roughly an offhand
rocket jump with no damage.
*/

#include "g_local.h"

#define HOOK_TIME			4		// Number of seconds that hook can live
#define HOOK_SPEED			1900	// Velocity of the hook itself
#define SLOW_HOOK_SPEED		550		// Velocity of the hook itself in slow mode
#define THINK_TIME			0.3		// Time between hook thinks
#define HOOK_DAMAGE			3		// Damage done by hook
#define GRAPPLE_REFIRE		5		// Refire delay for grapple, in frames

// NOTE: 650 appears to be the fastest the player can be pulled,
// without jitter (or without anti-jitter hacks).
#define PULL_SPEED			420		// How fast the player is pulled 
									// once the hook is anchored
#define SLOW_PULL_SPEED		420		// How fast the player is pulled in slow mode
									// once the hook is anchored
#define SHORT_HOOK_RANGE	700		// range of short hook

#define POGO_SPEED			500		// What forward velocity the player gets on a pogo
#define POGO_UP_FRACTION	0.4		// What fraction of the forward velocity a player 
									// gets as upward velocity on a pogo
#define POGO_REFIRE			9		// Refire delay for pogo, in frames

// Copied from p_weapon.c, because it's a static function
//extern void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);

void hackLift(edict_t *player) 
{

// A bug was introduced into Q2's physics with version 3.17 where 
// a player given an upward velocity will stay stuck to the ground
// if the velocity isn't large enough.  This workaround will lift
// the player a small amount off the ground so that the sticking
// doesn't occur.

	vec3_t traceTo;
	trace_t trace;
	
	// if there is an upward component to the player's velocity
	// (don't do this if the game is paused, it will move players out of the world!)
	if (player->velocity[2] > 0 && (int)sv_paused->value != 1) {

		// find the point immediately above the player's origin
		VectorCopy(player->s.origin, traceTo);
		traceTo[2] += 1;

		// trace to it
		trace = gi.trace(traceTo, player->mins, player->maxs, traceTo, player, MASK_PLAYERSOLID);

		// if there isn't a solid immediately above the player
		if (!trace.startsolid) {
			player->s.origin[2] += 1;	// make sure player off ground
		}
	}
}

// Ended_Grappling: Returns true if the client just stopped grappling.
qboolean Ended_Grappling (gclient_t *client)
{
	return (!(client->buttons & BUTTON_USE) && client->oldbuttons & BUTTON_USE);
}

// Is_Grappling: Returns true if the client is grappling at the moment.
qboolean Is_Grappling (gclient_t *client)
{
	return (client->hook == NULL) ? false : true;
}

// Grapple_Is_Pulling: Returns true if the client is being pulled by a grapple
qboolean Grapple_Is_Pulling (gclient_t *client)
{
	if (Is_Grappling(client) == false)
		return false;

	return client->on_hook;
}

/*
Called when the hook touches something.

If that something is a projectile: Ignore it.
If it is a player or monster: damage it and release
If it is a solid object: anchor to it and pull the player.
*/
void Grapple_Touch(edict_t *hook, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// Release if hitting its owner
	if (other == hook->owner)
		return;

	// Whether to stick to sky
	if (!(expflags & EXPERT_SKY_SOLID)) {
		if (surf && surf->flags & SURF_SKY)
		{
			Release_Grapple(hook);
			return;
		}
	}

	// ignore weapon projectiles (rockets, BFG blasts, &c)
	if (other != g_edicts && other->clipmask == MASK_SHOT)
		return;

	gi.sound(hook, CHAN_ITEM, gi.soundindex("world/land.wav"), 1, ATTN_NORM, 0);

	if (other != NULL) {
		// Do damage to those that need it
		T_Damage(other, hook, hook->owner, hook->velocity, hook->s.origin, plane->normal, HOOK_DAMAGE, 0, 0, MOD_GRAPPLE);
	}

	// The hook hit a hook/monster/maybe explobox. 
	// Pull once and release.
	if (other != g_edicts && other->health && other->solid == SOLID_BBOX) {
		Pull_Grapple(hook->owner);
		Release_Grapple(hook);
		return;
	}

	if (other != g_edicts && other->inuse &&
		(other->movetype == MOVETYPE_PUSH || other->movetype == MOVETYPE_STOP))
	{
		// Use a spare edict field on the object to store the hook flag in
		other->mynoise2 = hook;

		// Save what is being anchored to match velocities
		hook->owner->client->hook_touch = other;
		hook->enemy = other;
		hook->groundentity = NULL;

		hook->flags |= FL_TEAMSLAVE;
		
		// Turn off client prediction during the pull
		// Note: does not appear to be an improvement over
		// prediction with no gravity.
		//hook->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	}

	VectorClear(hook->velocity);
	VectorClear(hook->avelocity);
	hook->solid = SOLID_NOT;
	hook->touch = NULL;
	hook->movetype = MOVETYPE_NONE;

	// Sets how long a grapple can be anchored
	hook->delay = level.time + HOOK_TIME;

	// Show that hook is being pulled. Pull_Grapple handles movement from
	// Client_Think in p_client.c
	hook->owner->client->on_hook = true;

	// The server needs to be told that the player's off the ground
	hook->owner->groundentity = NULL;

	// do initial pull
	Pull_Grapple(hook->owner);

/*
	// Experimental: faster hooking if the player is already moving in the
	// direction of the hook
	{
	vec3_t hookPull;
	float directionMatch;

	// unit vector along hook's pull
	VectorSubtract(hook->s.origin, hook->owner->s.origin, hookPull);
	VectorNormalize(hookPull);

	// vector of hook's pull
	VectorScale(hookPull, PULL_SPEED, hookPull);

	directionMatch = DotProduct(hookPull, hook->owner->velocity) / (PULL_SPEED * PULL_SPEED);

	if (hook->owner->scale == 0) {
		hook->owner->scale = 1;
	}

	if (directionMatch > 0.55) {
		hook->owner->scale += 0.2;
		if (hook->owner->scale > 1.5) {
			hook->owner->scale = 1.5;
		}
	} else {
		hook->owner->scale = 1;
	}
	gi.bprintf(PRINT_HIGH, "DirectionMatch is %.1f, scale is %.1f\n", directionMatch, hook->owner->scale);
	}
*/

}

// Think_Grapple: Run any sounds, update any models, etc.
void Think_Grapple(edict_t *hook)
{
	// Hook has been out for too long. Release
	if (level.time > hook->delay)
		hook->prethink = Release_Grapple;
	else
	{
		if (hook->owner->client->hook_touch) {
			edict_t *obj = hook->owner->client->hook_touch;

			if (obj == g_edicts)
			{
				Release_Grapple(hook);
				return;
			}

			// Release if the edict was freed
			if (obj->inuse == false) {
				Release_Grapple(hook);
				return;
			}

			// Release if the object is dead
			if (obj->deadflag == DEAD_DEAD)
			{
				Release_Grapple(hook);
				return;
			}

			// Movement code is handled with the MOVETYPE_PUSH stuff in g_phys.c

			// Do damage to the touched entity - we're certain it's not a player anyway..
			T_Damage(obj, hook, hook->owner, hook->velocity, hook->s.origin, vec3_origin, HOOK_DAMAGE, 0, 0, MOD_GRAPPLE);
		} else if (expflags & EXPERT_SHORT_HOOK) {
			vec3_t hook_length;
			VectorSubtract(hook->s.origin, hook->owner->client->hook_start, hook_length);
			if (VectorLength(hook_length) > SHORT_HOOK_RANGE) {
				Release_Grapple(hook);
			}
		}

		hook->nextthink += THINK_TIME;
	}
}

// Make_Hook: Create and set up the hook object
void Make_Hook(edict_t *ent)
{
	edict_t *hook;
	vec3_t forward, right, start, offset;

	// Make the hook entity
	hook = G_Spawn();

	AngleVectors(ent->client->v_angle, forward, right, NULL);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorCopy(start, hook->s.origin);
	VectorCopy(forward, hook->movedir);
	vectoangles(forward, hook->s.angles);
	// for short hook range limitation
	VectorCopy(ent->s.origin, ent->client->hook_start);
	if (expflags & EXPERT_SLOW_HOOK) {
		VectorScale(forward, SLOW_HOOK_SPEED, hook->velocity);
	} else {
		VectorScale(forward, HOOK_SPEED, hook->velocity);
	}
	VectorSet(hook->avelocity, 0, 0, 500);

	hook->classname = "hook";
	hook->movetype = MOVETYPE_FLYMISSILE;
	hook->clipmask = MASK_SHOT;
	hook->solid = SOLID_BBOX;
	// no misprediction on hit
	hook->svflags |= SVF_DEADMONSTER;

	//hook->s.effects = pow(2, atoi(Info_ValueForKey(ent->client->pers.userinfo, "trail")));
	hook->s.effects = EF_GRENADE; // grey trail
		//EF_GIB; // red blood trail
		//EF_GREENGIB; // gold trail

	hook->s.renderfx = RF_FULLBRIGHT;
	VectorClear (hook->mins);
	VectorClear (hook->maxs);

	// In CTF, use Threewave grapple.  Otherwise, use activated grenade model
	// FIXME change to Expert custom hook model when auto-downloading is ready
	if (ctf->value) {
		hook->s.modelindex = gi.modelindex ("models/weapons/grapple/hook/tris.md2");
	} else {
		hook->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	}
	
	hook->owner = ent;
	hook->touch = Grapple_Touch;

	// Delay is used for when the hook is automatically released.
	// Think is used for sound/model updates
	hook->delay = level.time + HOOK_TIME;
	hook->nextthink = level.time;
	hook->think = Think_Grapple;
	
	// So that doors and others don't try to blow the hook up
	hook->svflags = SVF_MONSTER;
	
	ent->client->hook = hook;
	gi.linkentity(hook);
}

// Throw_Grapple: Initially starts the grappling hook
void Throw_Grapple (edict_t *player)
{
	vec3_t		forward, up;

	// Expert Pogo: like a rocket jump, but separate from weapons,
	// with the same control mechanism as the Expert Hook
	if (expflags & EXPERT_POGO) {

		// Refire delay for pogo 
		if (player->client->pogo_time > level.framenum) {
			return;
		}

		// get facing direction
		AngleVectors (player->client->v_angle, forward, NULL, up);

		// thrust in facing direction
		VectorMA(player->velocity, POGO_SPEED, forward, player->velocity);
		VectorMA(player->velocity, POGO_UP_FRACTION * POGO_SPEED, up, player->velocity);

		hackLift(player);

		// underwater explosion sound
		// FIXME: better sound?
		gi.sound(player, CHAN_ITEM, gi.soundindex("weapons/xpld_wat.wav"), 1, ATTN_NORM, 0);

		// Refire delay
		player->client->pogo_time = level.framenum + POGO_REFIRE;
		return;

	}

	if (player->client->hook) {
		// player already hooking
		return;
	}

	// FIXME: Possibly change channel
	gi.sound(player, CHAN_ITEM, gi.soundindex("plats/pt1_mid.wav"), 0.5, ATTN_NORM, 0);

	player->client->hook_touch = NULL;
	
	Make_Hook(player);
}

// Release_Grapple: Delete hook and perform any sounds/etc needed
void Release_Grapple (edict_t *hook)
{
	edict_t *owner = hook->owner;
	gclient_t *client = hook->owner->client;
	edict_t *link = hook->teamchain;

	client->on_hook = false;
	client->hook_touch = NULL;

	// If hook is already NULL, then the hook was already released,
	// so ignore this attempt to release 
	if (client->hook != NULL) {
		client->hook = NULL;
		gi.sound(owner, CHAN_ITEM, gi.soundindex("misc/menu3.wav"), 1, ATTN_NORM, 0);
		VectorClear(client->oldvelocity);

		hook->think = NULL;

		// If connected to something, take the hook out of that something's teamchain
		if (hook->enemy) {
//			hook->enemy->teamchain = link;
			hook->enemy->mynoise2 = NULL;
		}

		// Turn prediction back on for the player
		// Note: does not appear to be an improvement over
		// prediction with no gravity.
		//client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;

		G_FreeEdict(hook);
	}
}

// Pull_Grapple: Pulls player to hook. Called from ClientThink
void Pull_Grapple (edict_t *player)
{
	vec3_t hookDir;
	vec_t length;

	// unit vector in the direction of the hook from the player
	VectorSubtract(player->client->hook->s.origin, player->s.origin, hookDir);
	length = VectorNormalize(hookDir);

	// make the player's velocity a vector toward the hook
	if (expflags & EXPERT_SLOW_HOOK) {
		VectorScale(hookDir, SLOW_PULL_SPEED, player->velocity);
	} else {
		VectorScale(hookDir, /*player->scale * */ PULL_SPEED, player->velocity);
	}
	VectorCopy(hookDir, player->movedir);

	hackLift(player);

/*
	// Experimental: hook pulls momentarily and then releases.  zero-g?
	VectorMA(player->velocity, PULL_SPEED, hookDir, player->velocity); 
	VectorCopy(player->velocity, player->movedir);
	VectorNormalize(player->movedir);

	Release_Grapple(player->client->hook);
*/
}

