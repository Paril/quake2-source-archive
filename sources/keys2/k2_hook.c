/*
k2_hook.c
Original code written by Rich 'Publius' Tollerton, 1/1/98
Modified for Keys2 by Rich Shetina 1/28/98

Derived from grapple.qc in Expert Quake 1.31
(http://www.planetquake.com/expert/), written by
Charles 'Myrkul' Kendrick, which is based on the QW-friendly
grappling code written by Steve 'Wedge' Bond
(http://www.nuc.net/quake/).

See grapple.qc in Expert Quake 1.31 source for more credits..
*/

#include "g_local.h"

#define THINK_TIME 0.3	// Time between hook thinks
//RS
//This is our own P_ProjectSource, so that the hook will always launch from
//the hand that is *not* holding the weapon.
void P_ProjectHookSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	//RS: Always make the hook come from opposite or left hand
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	
	if ( (client->pers.hand == RIGHT_HANDED) ||
		 (client->pers.hand == CENTER_HANDED))
		_distance[1] *= -1;
	G_ProjectSource (point, _distance, forward, right, result);
}


// Started_Grappling: Returns true if the client just pressed the grapple key.
qboolean Started_Grappling (gclient_t *client)
{
	return client->latched_buttons & BUTTON_USE;
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

/*
Grapple_Touch: Called when the hook touches something.

If that something is a projectile: Ignore it.
If it is a player or monster: damage it and release
If it is a solid object: anchor to it and pull the player.

RS:Future - If it is an item, pull it to player
*/
void Grapple_Touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float volume = 1.0;

	if(K2_IsStealth(self->owner))
		volume = 0.2;

	// Release if hitting its owner
	if (other == self->owner)
		return;

	// Release if already released or has been voided via health
	// (this is a carryover from a QW bug..)
	if (!Is_Grappling(self->owner->client) && self->health == 0)
		return;
	self->health = 0;

	// The hook needs to ignore weapon projectiles (rockets, BFG blasts,
	// &c). But manually checking for each and every shot type is crude,
	// unportable, and bug-prone. So, since 99.44% of the projectiles out
	// there are going to use the clip mask MASK_SHOT, this code checks
	// to see if MASK_SHOT is completely in its clipmask.
	if (other != g_edicts && (other->clipmask == MASK_SHOT))
		return;
	
	gi.sound(self, CHAN_ITEM, gi.soundindex("world/fusein.wav"), volume, ATTN_NORM, 0);

	if (other != NULL)
		// Do damage to those that need it
		T_Damage(other, self, self->owner, self->velocity, self->s.origin, plane->normal, (int)hook_damage->value, 0, 0, 0);

	// The hook hit a player/monster/maybe explobox. Release.
	if (other != g_edicts && ((other->health)) && (other->solid == SOLID_BBOX)) {
		Release_Grapple(self);
		return;
	}

	if (other != g_edicts && other->inuse &&
		(other->movetype == MOVETYPE_PUSH || other->movetype == MOVETYPE_STOP))
	{
		// Use a spare edict field on the object to store the hook in
		other->mynoise2 = self;

		// Save what is being anchored to match velocities
		self->owner->client->hook_touch = other;
		self->enemy = other;
		self->groundentity = NULL;

		self->flags |= FL_TEAMSLAVE;
	}

	VectorClear(self->velocity);
	VectorClear(self->avelocity);
	
	self->solid = SOLID_NOT;
	
	self->touch = NULL;
	
	self->movetype = MOVETYPE_NONE;
	
	// Sets how long a grapple can be anchored
	self->delay = level.time + hook_time->value;

	// Show that player is being pulled. Pull_Grapple handles movement from
	// Client_Think in p_client.c
	self->owner->client->on_hook = true;

	// The server needs to be told that the player's off the ground
	self->owner->groundentity = NULL;
}

// Think_Grapple: Run any sounds, update any models, etc.
void Think_Grapple(edict_t *self)
{
	// Hook has been out for too long. Release
	if (level.time > self->delay)
		self->prethink = Release_Grapple;
	else
	{

		if (self->owner->client->hook_touch) {
			edict_t *obj = self->owner->client->hook_touch;

			if (obj == g_edicts)
			{
				Release_Grapple(self);
				return;
			}

			// Release if the edict was freed
			if (obj->inuse == false) {
				Release_Grapple(self);
				return;
			}

			// Release if the object is dead
			if (obj->deadflag == DEAD_DEAD)
			{
				Release_Grapple(self);
				return;
			}

			// Movement code is handled with the MOVETYPE_PUSH stuff in g_phys.c

			// Do damage to the touched entity - we're certain it's not a player anyway..
			T_Damage(obj, self, self->owner, self->velocity, self->s.origin, vec3_origin, hook_damage->value, 0, 0, 0);
		}

		//RS: Make a trail 
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (self->owner->s.origin);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->owner->s.origin, MULTICAST_PVS);
			
		self->nextthink += THINK_TIME;
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
	P_ProjectHookSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorCopy(start, hook->s.origin);
	VectorCopy(forward, hook->movedir);
	vectoangles(forward, hook->s.angles);
	VectorScale(forward, hook_speed->value, hook->velocity);
	VectorSet(hook->avelocity, 0, 0, 800);

	hook->classname = "hook";
	hook->movetype = MOVETYPE_FLYMISSILE;
	hook->clipmask = MASK_SHOT;
	hook->solid = SOLID_BBOX;
	VectorClear (hook->mins);
	VectorClear (hook->maxs);

	hook->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	
	hook->owner = ent;
	hook->touch = Grapple_Touch;

	// Delay is used for when the hook is automatically released.
	// Think is used for sound/model updates
	hook->delay = level.time + hook_time->value;
	hook->nextthink = level.time + THINK_TIME;

	hook->think = Think_Grapple;

	// Code to check if the hook has already touched something.
	// (QW had a double-touch bug; assuming Q2 does too)
	hook->health = 100;

	// So that doors and others don't try to blow the hook up
	hook->svflags = SVF_MONSTER;
	
	ent->client->hook = hook;
	gi.linkentity(hook);
}

// Throw_Grapple: Initially starts the grappling hook
void Throw_Grapple (edict_t *self)
{
	vec3_t forward, right,end;
	trace_t	tr;
	float volume = 1.0;
	
	if (K2_IsStealth(self))
		volume = 0.2;

	// Mask off BUTTON_USE so that this function isn't called twice
	self->client->latched_buttons ^= BUTTON_USE;

	self->client->hook_touch = NULL;
	
	//Sky Hook check
	if(!skyhook->value)
	{
		AngleVectors(self->client->v_angle, forward, right, NULL);
		VectorMA (self->s.origin, 8192, forward, end);
		tr = gi.trace (self->s.origin, NULL,NULL,end, self, MASK_SHOT);
	
		if (tr.surface->flags & SURF_SKY)
		{
			gi.sound(self,CHAN_ITEM,gi.soundindex ("weapons/noammo.wav"),volume,ATTN_NORM,0);
			return;
		}
	}
	
	// FIXME: This function seemes lagged relative to the keypress..
	Make_Hook(self);
	gi.sound(self,CHAN_AUTO,gi.soundindex ("medic/medatck2.wav"),volume,ATTN_NORM,0);
	
}

// Release_Grapple: Delete hook and perform any sounds/etc needed
void Release_Grapple (edict_t *hook)
{
	//CHANGED
	//edict_t *owner = hook->owner;
	//gclient_t *client = hook->owner->client;
	edict_t *owner;
	gclient_t *client;
	float volume = 1.0;
	
	if (K2_IsStealth(hook->owner))
		volume = 0.2;


	owner = hook->owner;
	client = hook->owner->client;
	
	client->on_hook = false;
	client->hook_touch = NULL;

	// If hook is already NULL, then the hook was released before the key
	// was released. Ignore it.
	if (client->hook != NULL)
	{
		client->hook = NULL;
		gi.sound(owner, CHAN_ITEM, gi.soundindex("medic/medatck5.wav"), volume, ATTN_NORM, 0);
		
		VectorClear(client->oldvelocity);

		// So that the player doesn't get hurt when on the ground
		if (owner->groundentity)
			VectorClear(owner->velocity);

		hook->think = G_FreeEdict;

		// If connected to something, take the hook out of that something's teamchain
		if (hook->enemy)
			hook->enemy->mynoise2 = NULL;
					
		G_FreeEdict(hook);
	}
}

// Pull_Grapple: Pulls player to hook. Called from p_client.qc
// Argument is the edict for the CLIENT, not the hook!
void Pull_Grapple (edict_t *player)
{
	
	vec3_t hookDir;
	
	// unit vector in the direction of the hook from the player
	VectorSubtract(player->client->hook->s.origin, player->s.origin, hookDir);
	VectorNormalize(hookDir);

	// make the player's velocity a vector toward the hook
	VectorScale(hookDir, /*player->scale * */ pull_speed->value, player->velocity);
	VectorCopy(hookDir, player->movedir);

	//HACK
	// A bug was introduced into Q2's physics with version 3.17 where 
	// a player given an upward velocity will stay stuck to the ground
	// if the velocity isn't large enough.  This workaround will lift
	// the player a small amount off the ground so that the sticking
	// doesn't occur.
	
	// if there is an upward component to the player's velocity
	if (player->velocity[2] > 0) {

		vec3_t traceTo;
		trace_t trace;

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
