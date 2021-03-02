#ifdef OHOOK

//////////////////////////////////////////////////////////////////////////////
// This code was taken from James Abbatiello's Vanilla CTF 0.5 source code and
// modified slightly for use in NIQ.

#include "g_local.h"
#include "m_player.h"
#include "hook.h"
#include "g_niq.h"

void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));

// move the two ends of the laser beam to the proper positions
void hook_laser_think (edict_t *self)
{
	vec3_t	forward, right, offset, start;

	// stupid check for NULL pointers ...
	if (!self->owner || !self->owner->owner || !self->owner->owner->client) 
	{
		// ok, something's screwy
		G_FreeEdict(self);		// go away
		return;	
	}

	// put start position into start
	AngleVectors (self->owner->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, self->owner->owner->viewheight-8);
	P_ProjectSource (self->owner->owner->client, self->owner->owner->s.origin, offset, forward, right, start);

	// move the two ends
//	gi.unlinkentity(self);
	VectorCopy (start, self->s.origin);
	VectorCopy (self->owner->s.origin, self->s.old_origin);
	gi.linkentity(self);

	// set up to go again
	self->nextthink = level.time + FRAMETIME;

	return;
}

// create a laser and return a pointer to it
edict_t *hook_laser_start (edict_t *ent, edict_t *owner)
{
	edict_t *self;

	self = G_Spawn();
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex = 1;			// must be non-zero
	self->owner = ent;

	// set the beam diameter
	self->s.frame = 4;

	// set the color
	if(ctf->value)
		{
		// show team color letter after score on mini-SB
		if(owner && owner->client && owner->client->resp.ctf_team == CTF_TEAM1)
			self->s.skinnum = 0xf2f2f0f0;		// red
		else
			self->s.skinnum = 0xf3f3f1f1;		// blue
		}
	else
		self->s.skinnum = 0xf2f2f0f0;			// default to red

	self->think = hook_laser_think;

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	gi.linkentity (self);

	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
	hook_laser_think (self);

	return(self);
}

// reset the hook.  pull all entities out of the world and reset
// the clients weapon state
void hook_reset (edict_t *rhook)
{
	// start with NULL pointer checks
	if (!rhook) 
		return;

	if (rhook->owner) { // && rhook->owner != world) {
		if (rhook->owner->client) {
			// client's hook is no longer out (duh)
			rhook->owner->client->hook_state = HOOK_READY;
			rhook->owner->client->hook = NULL;
			// if they have the hook selected, reset for another firing
			if (rhook->owner->client->pers.weapon && strcmp(rhook->owner->client->pers.weapon->pickup_name, "Hook") == 0) {
				rhook->owner->client->ps.gunframe++;				// = 9;	// unlock animation
//!!!				rhook->owner->client->weaponstate = WEAPON_READY;	// can fire again immediately
			}
			// avoid all falling damage
			// seems to work, but i'm not quite sure if its correct
//			VectorClear(rhook->owner->client->oldvelocity);
		}
	}
	// NIQ change -- laser is in client
	if(rhook->owner && rhook->owner->client && rhook->owner->client->laser)
		G_FreeEdict(rhook->owner->client->laser);

	// delete ourself
	G_FreeEdict(rhook);

	if (rhook->owner && rhook->owner->client)
		rhook->owner->client->weaponstate = WEAPON_READY;	// can fire again immediately
};

// resets the hook if it needs to be
qboolean hook_cond_reset(edict_t *self) 
{
 	// this should never be true
 	if (!self->owner || (!self->enemy && self->owner->client && self->owner->client->hook_state == HOOK_ON)) {
	    hook_reset (self);
        return (true);
    }
	
	// drop the hook if either party dies/leaves the game/etc.
    if ((self->enemy && !self->enemy->inuse) || (!self->owner->inuse) ||
		(self->enemy && self->enemy->client && self->enemy->health <= 0) || 
		(self->owner->health <= 0))
    {
        hook_reset (self);
        return (true);
    }

    // drop the hook if player lets go of button
	// and has the hook as current weapon
    if (!((self->owner->client->latched_buttons|self->owner->client->buttons) & BUTTON_ATTACK)
			&& self->owner->client->pers.weapon && (strcmp(self->owner->client->pers.weapon->pickup_name, "Hook") == 0))
    {
		hook_reset (self);
		return (true);
    }

	return(false);
}

void hook_cond_reset_think(edict_t *hook)
{
	if (hook_cond_reset(hook))
		return;
	hook->nextthink = level.time + FRAMETIME;
}

void hook_service (edict_t *self) 
{
    vec3_t	hook_dir;

	// if hook should be dropped, just return
	if (hook_cond_reset(self)) 
		return;

	// give the client some velocity ...
	if (self->enemy->client)
		VectorSubtract(self->enemy->s.origin, self->owner->s.origin, hook_dir);
	else
		VectorSubtract(self->s.origin, self->owner->s.origin, hook_dir);

    VectorNormalize(hook_dir);
	VectorScale(hook_dir, 750, self->owner->velocity);
	// avoid "falling" damage
//		VectorCopy(self->owner->velocity, self->owner->client->oldvelocity);
}

// keeps the invisible hook entity on hook->enemy (can be world or an entity)
void hook_track (edict_t *self) 
{
	vec3_t	normal;

	// if hook should be dropped, just return
	if (hook_cond_reset(self)) 
		return;

	// bring the pAiN!
    if (self->enemy->client)
    {
		// move the hook along with the player.  It's invisible, but
		// we need this to make the sound come from the right spot
//			gi.unlinkentity(self);
		VectorCopy(self->enemy->s.origin, self->s.origin);
//			gi.linkentity(self);
			
		VectorSubtract(self->owner->s.origin, self->enemy->s.origin, normal);

		T_Damage (self->enemy, self, self->owner, vec3_origin, self->enemy->s.origin, normal, 1, 0, DAMAGE_NO_KNOCKBACK, MOD_GRAPPLE);
    } 
	else 
	{

      	// If the hook is not attached to the player, constantly copy
	    // copy the target's velocity. Velocity copying DOES NOT work properly
	    // for a hooked client. 
        VectorCopy(self->enemy->velocity, self->velocity);
	}

	gi.linkentity(self);
    self->nextthink = level.time + 0.1;
}

// the hook has hit something.  what could it be? :)
void hook_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) 
{
	vec3_t	dir, normal;

	// ignore hitting the person who launched us
	if (other == self->owner)
		return;
	
	// ignore hitting items/projectiles/etc.
	if (other->solid == SOLID_NOT || other->solid == SOLID_TRIGGER || other->movetype == MOVETYPE_FLYMISSILE)
		return;
	
	// mfox-NIQ: don't hook onto the sky unless this is enabled (also check for 
	// surf != NULL or GPF when releasing then refiring hook really quickly
	if(!niq_hooksky->value && surf != NULL && surf->flags & SURF_SKY)
	{
		hook_reset(self);
		return;
	}

	if (other->client) 
	{	// we hit a player
		// ignore hitting a teammate
		if (OnSameTeam(other, self->owner))
			return;
		// we hit an enemy, so do a bit of damage
		VectorSubtract(other->s.origin, self->owner->s.origin, dir);
		VectorSubtract(self->owner->s.origin, other->s.origin, normal);
		T_Damage(other, self, self->owner, dir, self->s.origin, normal, 10, 10, 0, MOD_GRAPPLE);
		// dont latch onto players
		hook_reset(self);
		return;
	} 
	else 
	{	// we hit something thats not a player
		// if we can hurt it, then do a bit of damage
		if (other->takedamage) 
		{
			VectorSubtract(other->s.origin, self->owner->s.origin, dir);
			VectorSubtract(self->owner->s.origin, other->s.origin, normal);
			T_Damage(other, self, self->owner, dir, self->s.origin, normal, 1, 1, 0, MOD_GRAPPLE);
		}

		// gi.sound() doesnt work because the origin of an entity with no model is not 
		// transmitted to clients or something.  hoped this would be fixed in Q2 ...
		gi.positioned_sound(self->s.origin, self, CHAN_WEAPON, gi.soundindex("flyer/Flyatck2.wav"), 1, ATTN_NORM, 0);
	}
	
	// stop moving
	VectorClear(self->velocity);

// handled in hook_cond_reset()
	// check to see if we already let up on the fire button
//	if ( !((self->owner->client->latched_buttons|self->owner->client->buttons) & BUTTON_ATTACK) ) {
//		hook_reset(self);
//		return;
//	}

	// remember who/what we hit
	// this must be set before hook_track() is called
	self->enemy = other;

	// if hook should be dropped, just return
//	if (hook_cond_reset(self)) return;

	// pull us off the ground (figuratively)
//	self->owner->groundentity = NULL;

	// we are now anchored
	self->owner->client->hook_state = HOOK_ON;
	
	// keep up with that thing
	self->think = hook_track;
	self->nextthink = level.time + 0.1;
	
	self->solid = SOLID_NOT;
}

// creates the invisible hook entity and sends it on its way
// attaches a laser to it
void fire_hook (edict_t *owner, vec3_t start, vec3_t forward) 
{
	edict_t	*hook;
	trace_t tr;

    hook = G_Spawn();
	if(!hook)
		return;

    hook->movetype = MOVETYPE_FLYMISSILE;
    hook->solid = SOLID_BBOX;
	hook->clipmask = MASK_SHOT;
    hook->owner = owner;			// this hook belongs to me
	owner->client->hook = hook;		// this is my hook
    hook->classname = "hook";		// this is a hook

	vectoangles (forward, hook->s.angles);
    VectorScale(forward, 800, hook->velocity);

    hook->touch = hook_touch;

	hook->think = hook_cond_reset_think;
	hook->nextthink = level.time + FRAMETIME;
//	hook->noblock = true;

	gi.setmodel(hook, "");

    VectorCopy(start, hook->s.origin);
	VectorCopy(hook->s.origin, hook->s.old_origin);

	VectorClear(hook->mins);
	VectorClear(hook->maxs);

	// start up the laser
	// NIQ change -- laser is in client
	owner->client->laser = hook_laser_start(hook, owner);

	// put it in the world
	gi.linkentity(hook);


	// from id's code.  I don't question these things...		
	tr = gi.trace (owner->s.origin, NULL, NULL, hook->s.origin, hook, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (hook->s.origin, -10, forward, hook->s.origin);
		hook->touch (hook, tr.ent, NULL, NULL);
	}
}

// a call has been made to fire the hook
void hook_fire (edict_t *ent) 
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (!ent || !ent->client || !ent->client->pers.weapon)
		return;

	// due to the way Weapon_Generic was written up, if weaponstate
	// is not WEAPON_FIRING, then we can switch away
	// since we don't want to be in any other real state,
	// we just set it to some 'invalid' state and everything works
	// fine :)
	if (ent->client->pers.weapon &&	strcmp(ent->client->pers.weapon->pickup_name, "Hook") == 0)
		ent->client->weaponstate = -1;	// allow weapon change

	if (ent->client->hook_state)		// reject subsequent calls from Weapon_Generic
		return;

    ent->client->hook_state = HOOK_OUT;

	// calculate start position and forward direction
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// kick back??
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	// actually launch the hook off
	fire_hook (ent, start, forward);

	if (ent->client->silencer_shots)
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("flyer/Flyatck3.wav"), 0.2, ATTN_NORM, 0);
	else
		gi.sound(ent, CHAN_WEAPON, gi.soundindex("flyer/Flyatck3.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

// boring service routine
void Weapon_Hook (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

//	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, hook_fire);
}

#endif//OHOOK
