// CUSTOM SOURCE FILE

#include "g_local.h"
#include "m_player.h"
#include "botshook.h"
#include "botsteam.h"
#include "botsutil.h"

void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));

// move the two ends of the laser beam to the proper positions
void hook_laser_think (edict_t *self)
{
	vec3_t	forward, right, offset, start;
	
	// stupid check for NULL pointers ...
	if (self->owner) {
		if (self->owner->owner) {
			if (self->owner->owner->client) {
				// put start position into start
				AngleVectors (self->owner->owner->client->v_angle, forward, right, NULL);
				VectorSet(offset, 24, 8, self->owner->owner->viewheight-8);
				P_ProjectSource (self->owner->owner->client, self->owner->owner->s.origin, offset, forward, right, start);

				// move the two ends
				gi.unlinkentity(self);
				VectorCopy (start, self->s.origin);
				VectorCopy (self->owner->s.origin, self->s.old_origin);
				gi.linkentity(self);

				// set up to go again
				self->nextthink = level.time + FRAMETIME;
				return;
			}
		}
	}

	// ok, something's screwy.  we should not get here
//	G_FreeEdict(self);		// go away
}

// create a laser and return a pointer to it
edict_t *hook_laser_start (edict_t *ent)
{
	edict_t *self;

	self = G_Spawn();
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex = 1;			// must be non-zero
	self->owner = ent;

	// set the beam diameter
	self->s.frame = 2;

	// set the color
	if (ent->owner->client->pers.team == 1)
		self->s.skinnum = 0xf3f3f1f1;		// sorta blue
	else if (ent->owner->client->pers.team == 2)
		self->s.skinnum = 0xf2f2f0f0;		// red
	else if (ent->owner->client->pers.team == 1)
		self->s.skinnum = 0xd0d1d2d3;		// green ?
	else
		self->s.skinnum = 0xe0e1e2e3;		// red+brown

	self->think = hook_laser_think;

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	gi.linkentity (self);

	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
	self->nextthink = level.time + FRAMETIME;

//	hook_laser_think (self);

	return(self);
}


// reset the hook.  pull all entities out of the world and reset
// the clients weapon state
void hook_reset (edict_t *rhook)
{
	// start with NULL pointer checks
	if (rhook->owner) { // && rhook->owner != world) {
		if (rhook->owner->client) {
			// client's hook is no longer out (duh)
			rhook->owner->client->hook_out = false;
			rhook->owner->client->hook_on = false;
			rhook->owner->client->hook = NULL;
			// if they have the hook selected, reset for another firing
			if (rhook->owner->client->pers.weapon &&
				strcmp(rhook->owner->client->pers.weapon->pickup_name, "Hook") == 0) {
				rhook->owner->client->ps.gunframe++;				// = 9;	// unlock animation
				rhook->owner->client->weaponstate = WEAPON_READY;	// can fire again immediately
			}
		}
	}
	// this should always be true and free the laser beam
	if (rhook->laser) 
		G_FreeEdict(rhook->laser);

	// delete ourself
	G_FreeEdict(rhook);
};

// resets the hook if it needs to be
qboolean hook_cond_reset(edict_t *self) 
{
	if ((self->owner) && (self->enemy)) 
	{
		// drop the hook if either party dies/leaves the game/etc.
	    if ((!self->enemy->inuse) || (!self->owner->inuse) ||
			(self->enemy->client && self->enemy->health <= 0) || 
			(self->owner->health <= 0))
		{
	        hook_reset (self);
			return (true);
		}
	}
	else
	{
        hook_reset (self);
		return (true);
	}

	if (self->owner)
	{
		if (self->owner->client)
		{
			// drop the hook if player lets go of button
			// and has the hook as current weapon
			if (!((self->owner->client->latched_buttons|self->owner->client->buttons) & BUTTON_ATTACK)
				&& (strcmp(self->owner->client->pers.weapon->pickup_name, "Hook") == 0))
			{
				hook_reset (self);
				return (true);
			}
		}
		else
		{
		    hook_reset (self);
			return (true);
		}
	}
	else
	{
        hook_reset (self);
		return (true);
	}

	return(false);
}

void hook_service (edict_t *self) 
{
    vec3_t	hook_dir;

	// if hook should be dropped, just return
	if (hook_cond_reset(self)) return;

	// give the client some velocity ...
	if (self->enemy->client)
		_VectorSubtract(self->enemy->s.origin, self->owner->s.origin, hook_dir);
	else
		_VectorSubtract(self->s.origin, self->owner->s.origin, hook_dir);
    VectorNormalize(hook_dir);
	VectorScale(hook_dir, 750, self->owner->velocity);
}

// keeps the invisible hook entity on hook->enemy (can be world or an entity)
void hook_track (edict_t *self) 
{
	vec3_t	normal;

	// if hook should be dropped, just return
	if (hook_cond_reset(self)) return;

	// bring the pAiN!
    if (self->enemy->client)
    {
		// move the hook along with the player.  It's invisible, but
		// we need this to make the sound come from the right spot
		gi.unlinkentity(self);
		VectorCopy(self->enemy->s.origin, self->s.origin);
		gi.linkentity(self);
		
		_VectorSubtract(self->owner->s.origin, self->enemy->s.origin, normal);

		T_Damage (self->enemy, self, self->owner, vec3_origin, self->enemy->s.origin, normal, 1, 0, DAMAGE_NO_KNOCKBACK, MOD_HOOK);
    } 
	else 
	{
        // If the hook is not attached to the player, constantly copy
	    // copy the target's velocity. Velocity copying DOES NOT work properly
	    // for a hooked client. 
        VectorCopy(self->enemy->velocity, self->velocity);
	}

    self->nextthink = level.time + 0.1;
}

// the hook has hit something.  what could it be? :)
void hook_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf) 
{
	vec3_t	dir, normal;
	int numcellsreq;

	if (!self->owner)
		hook_reset(self);

	if (!self->owner->client)
		hook_reset(self);
	
	// ignore hitting the person who launched us
	if (other == self->owner)
		return;
	
	// ignore hitting items/projectiles/etc.
	if (other->solid == SOLID_NOT || other->solid == SOLID_TRIGGER || other->movetype == MOVETYPE_FLYMISSILE)
		return;
	
	if (other->client) 
	{		
		// we hit a player
		if (teamplay == 1)
		{
			// hit a teammate
			if (OnSameTeam(other, self->owner)) 
			{
				if (self->owner->client->pers.player_class == 8)
					other->health = other->max_health;

				// stop moving
				VectorClear(self->velocity);
				hook_reset (self);
				return;
			}
		}
		// we hit an enemy, so do a bit of damage
		_VectorSubtract(other->s.origin, self->owner->s.origin, dir);
		_VectorSubtract(self->owner->s.origin, other->s.origin, normal);
		T_Damage(other, self, self->owner, dir, self->s.origin, normal, 30, 10, 0, MOD_HOOK);

		hook_reset (self);
		return;
	} 
	else if ( (surf && (surf->flags & SURF_SKY)) || (Q_stricmp (other->classname, "worldspawn")) )
	{
		// stop moving
		VectorClear(self->velocity);
        hook_reset (self);
		return;    // don't attach hook to sky
	}
	else // we hit something thats not a player
	{					
		// if we can hurt it, then do a bit of damage
		if (other->takedamage) 
		{
			_VectorSubtract(other->s.origin, self->owner->s.origin, dir);
			_VectorSubtract(self->owner->s.origin, other->s.origin, normal);
			T_Damage(other, self, self->owner, dir, self->s.origin, normal, 1, 1, 0, MOD_HOOK);
		}
		// stop moving
		VectorClear(self->velocity);
		
		// gi.sound() doesnt work because the origin of an entity with no model is not 
		// transmitted to clients or something.  hoped this would be fixed in Q2 ...
		gi.positioned_sound(self->s.origin, self, CHAN_WEAPON, gi.soundindex("flyer/Flyatck2.wav"), 1, ATTN_NORM, 0);
	}


	if (self->owner->client->pers.player_class == 1)
	{
		if (teamplay_IsOffense(self->owner))
			numcellsreq = 15;
		else
			numcellsreq = 25;
	}
	else if (self->owner->client->pers.player_class == 2)
		numcellsreq = 25;
	else if (self->owner->client->pers.player_class == 3)
		numcellsreq = 5;
	else if (self->owner->client->pers.player_class == 5)
		numcellsreq = 25;
	else if (self->owner->client->pers.player_class == 6)
		numcellsreq = 10;
	else if (self->owner->client->pers.player_class == 7)
		numcellsreq = 10;
	else if (self->owner->client->pers.player_class == 8)
	{
		if (teamplay_IsOffense(self->owner))
			numcellsreq = 2;
		else
			numcellsreq = 5;
	}

	if (!IsFemale(self->owner))
		numcellsreq *= 2;

	self->owner->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= numcellsreq;
	if (self->owner->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 0)
	{
		self->owner->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] = 0;
	}

	// remember who/what we hit
	// this must be set before hook_cond_reset() is called
	self->enemy = other;

	// if hook should be dropped, just return
	if (hook_cond_reset(self)) return;

	// we are now anchored
	self->owner->client->hook_on = true;
	
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
    hook->movetype = MOVETYPE_FLYMISSILE;
    hook->solid = SOLID_BBOX;
	hook->clipmask = MASK_SHOT;
    hook->owner = owner;			// this hook belongs to me
	owner->client->hook = hook;		// this is my hook
    hook->classname = "hook";		// this is a hook

	vectoangles (forward, hook->s.angles);
    VectorScale(forward, 800, hook->velocity);

    hook->touch = hook_touch;
	hook->think = hook_reset;
	hook->nextthink = level.time + 10;

	gi.setmodel(hook, "models/objects/flash/tris.md2");

    VectorCopy(start, hook->s.origin);
	VectorCopy(hook->s.origin, hook->s.old_origin);

	VectorClear(hook->mins);
	VectorClear(hook->maxs);

	// start up the laser
	if (teamplay == 1)
	{
		if ((owner->client->pers.player_class == 3) || (owner->client->pers.player_class == 6))
			hook->laser = NULL;
		else
			hook->laser = hook_laser_start(hook);
	}
	else
		hook->laser = hook_laser_start(hook);

	hook->s.effects |= EF_COLOR_SHELL;
	hook->s.renderfx |= teamkeys[owner->client->pers.team-1].keyeffect;

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

	// due to the way Weapon_Generic was written up, if weaponstate
	// is not WEAPON_FIRING, then we can switch away
	// since we don't want to be in any other real state,
	// we just set it to some 'invalid' state and everything works
	// fine :)
	ent->client->weaponstate = -1;	// allow weapon change

	if (ent->client->invisible)
	{
		ent->client->last_movement = level.time;
		ent->client->invisible = false;
		gsutil_centerprint(ent, "%s", "You are visible again!\n");
		ent->s.modelindex = 255;
	}

	if (ent->client->hook_out)		// reject subsequent calls from Weapon_Generic
		return;

    ent->client->hook_out = true;

	// calculate start position and forward direction
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// kick back??
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	// actually launch the hook off
	fire_hook (ent, start, forward);
	gi.sound(ent, CHAN_WEAPON, gi.soundindex("medic/medatck2.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);
}

// boring service routine
void NoAmmoWeaponChange (edict_t *ent);
void ChangeWeapon (edict_t *ent);

void Weapon_Hook (edict_t *ent)
{
	static int	pause_frames[]	= {10, 18, 27, 0};
	static int	fire_frames[]	= {6, 0};
	int numcellsreq=0;

	if (ent->client->pers.player_class == 1)
		numcellsreq = 25;
	else if (ent->client->pers.player_class == 2)
		numcellsreq = 25;
	else if (ent->client->pers.player_class == 3)
		numcellsreq = 5;
	else if (ent->client->pers.player_class == 5)
		numcellsreq = 25;
	else if (ent->client->pers.player_class == 6)
		numcellsreq = 10;
	else if (ent->client->pers.player_class == 7)
		numcellsreq = 10;
	else if (ent->client->pers.player_class == 8)
		numcellsreq = 5;

	if (!IsFemale(ent))
		numcellsreq *= 2;

	if ((ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < numcellsreq) && (ent->client->hook_out == false))
	{
		gsutil_centerprint(ent, "%s", "Not enough cells for grappling hook!\n");
		NoAmmoWeaponChange (ent);
		ChangeWeapon(ent);
		return;
	}

	Weapon_Generic (ent, 5, 9, 31, 36, pause_frames, fire_frames, hook_fire);
}

