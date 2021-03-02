#include "g_local.h"

#define HOOK_MODEL "models/objects/debris2/tris.md2"

#define PRESENT_TIME level.time

#define HOOK_HIT_SOUND gi.soundindex("flyer/Flyatck1.wav")
#define HOOK_SMACK_SOUND gi.soundindex("flyer/Medatck3.wav")
#define HOOK_MOTOR1_SOUND gi.soundindex("world/turbine1.wav")
#define HOOK_MOTOR2_SOUND gi.soundindex("world/turbine1.wav")
#define HOOK_MOTOR3_SOUND gi.soundindex("world/turbine1.wav")
#define HOOK_RETRACT_SOUND gi.soundindex("weapons/Sshotr1b.wav")
#define HOOK_LAUNCH_SOUND gi.soundindex("medic/Medatck2.wav")

// hookstates
#define HOOK_OFF 0x00000000 // set if hook command is active
#define HOOK_ON 0x00000001 // set if hook command is active
#define SHRINK_ON 0x00000002 // set if shrink chain is active
#define GROW_ON 0x00000004 // set if grow chain is active 

void G_Spawn_Trails(int type, vec3_t start, vec3_t endpos, vec3_t origin )
{
	gi.WriteByte(SVC_TEMP_ENTITY);
	gi.WriteByte(type);
	gi.WritePosition(start);
	gi.WritePosition(endpos);
	gi.multicast(origin, MULTICAST_PVS);
}

void G_Spawn_Models(int type, short rec_no, vec3_t start, vec3_t endpos, vec3_t offset, vec3_t origin )
{
	gi.WriteByte(SVC_TEMP_ENTITY);
	gi.WriteByte(type);
	gi.WriteShort(rec_no);
	gi.WritePosition(start);
	gi.WritePosition(endpos);
	if (rec_no == TE_GRAPPLE_CABLE)
		gi.WritePosition(offset);
	gi.multicast(origin, MULTICAST_PVS);
} 

//======================================================
// True if Ent is valid, has client, and edict_t inuse.
//======================================================
bool G_EntExists(edict_t *ent) {
	return ((ent) && (ent->client) && (ent->inUse));
}

//======================================================
// True if ent is not DEAD or DEAD or DEAD (and BURIED!)
//======================================================
bool G_ClientNotDead(edict_t *ent) {
	bool b1=ent->client->ps.pMove.pmType!=PMT_DEAD;
	bool b2=ent->deadflag != DEAD_DEAD;
	bool b3=ent->health > 0;
	return (b3 || b2 || b1);
}

bool G_ClientInGame(edict_t *ent) {
	if (!G_EntExists(ent)) return false;
	if (!G_ClientNotDead(ent)) return false;
	return (ent->client->respawn_time + 5.0 < level.time);
}

//======================================================
//======================================================
//======================================================
/*
Spawns sparks of (type) from {start} in direction of {movdir} and
Broadcasts to all in Potentially Visible Set from vector (origin)

TE_BLASTER - Spawns blaster sparks
TE_BLOOD - Spawns spurt of red blood
TE_BULLET_SPARKS - Same as TE_SPARKS, with a bullet puff and richochet sound
TE_GREENBLOOD - NOT IMPLEMENTED - Spawns a spurt of green blood
TE_GUNSHOT - Spawns a grey splash of particles, with a bullet puff
TE_SCREEN_SPARKS - Spawns a large green/white splash of sparks
TE_SHIELD_SPARKS - Spawns a large blue/violet splash of sparks
TE_SHOTGUN - Spawns a small grey splash of spark particles, with a bullet puff
TE_SPARKS - Spawns a red/gold splash of spark particles
*/
//======================================================
void G_Spawn_Sparks(int type, vec3_t start, vec3_t movdir, vec3_t origin ) {
gi.WriteByte(SVC_TEMP_ENTITY);
gi.WriteByte(type);
gi.WritePosition(start);
gi.WriteDir(movdir);
gi.multicast(origin, MULTICAST_PVS);
}

#define GRAPPLE_STYLE 1
#define LASER_STYLE 0

void P_ProjectSource_Reverse(gclient_t *client,
							 vec3_t point,
							 vec3_t distance,
							 vec3_t forward,
							 vec3_t right,
							 vec3_t result)
{
	vec3_t dist={0,0,0};

	Vec3Copy(distance, dist);
	if (client->pers.hand == RIGHT_HANDED)
		dist[1] *= -1; // Left Hand already defaulted
	else if (client->pers.hand == CENTER_HANDED)
		dist[1]= 0;
	G_ProjectSource(point, dist, forward, right, result);
} 

#define ENTS_HOOKSTATE ent->client->hookstate
#define ENTS_HOOKTYPE ent->client->hooktype

#define HOOK_OWNERS_HOOKSTATE hook->owner->client->hookstate
#define HOOK_OWNERS_HOOKTYPE hook->owner->client->hooktype
#define ENTS_V_ANGLE ent->client->v_angle
#define ENTS_VIEW_HEIGHT ent->viewheight
#define ENTS_S_ORIGIN ent->s.origin

// edict->sounds constants
#define MOTOR_OFF 0 // motor sound has not been triggered
#define MOTOR_START 1 // motor start sound has been triggered
#define MOTOR_ON 2 // motor running sound has been triggered

//==========================================================
void get_start_position(edict_t *ent, vec3_t start) {
	vec3_t offset={0,0,0};
	vec3_t forward={0,0,0};
	vec3_t right={0,0,0};
	vec3_t zvec={0,0,0};

	// Get forward and right(direction) vectors
	Angles_Vectors(ent->owner->client->v_angle, forward, right, NULL);

	Vec3Set(offset, 8, 8, ent->owner->viewheight-8);
	// Add to the global offset of the world
	Vec3Add(offset, zvec, offset);

	// Get start vector
	P_ProjectSource_Reverse(ent->owner->client, ent->owner->s.origin, offset, forward, right, start);
}

//==========================================================
void play_moving_chain_sound(edict_t *hook, bool chain_moving) {

	// determine sound play if climbing or sliding
	if (chain_moving)
		switch (hook->sounds) {
case MOTOR_OFF:
	// play start of chain climbing motor sound
	gi.sound(hook->owner, CHAN_WEAPON, HOOK_MOTOR1_SOUND, 1, ATTN_NORM, 0);
	hook->sounds = MOTOR_START;
	break;
case MOTOR_START:
	// play repetitive chain climbing sound
	gi.sound(hook->owner, CHAN_WEAPON, HOOK_MOTOR2_SOUND, 1, ATTN_NORM, 0);
	hook->sounds = MOTOR_ON;
	break;
	} // end switch
	else
		if (hook->sounds != MOTOR_OFF) {
			gi.sound(hook->owner, CHAN_WEAPON, HOOK_MOTOR3_SOUND, 1, ATTN_NORM, 0);
			hook->sounds = MOTOR_OFF; }
}

//==========================================================
void DropHook(edict_t *hook) {

	HOOK_OWNERS_HOOKSTATE = HOOK_OFF;

	gi.sound(hook->owner, CHAN_WEAPON, HOOK_RETRACT_SOUND, 1, ATTN_NORM, 0);

	// removes hook
	G_FreeEdict(hook);
}

//==========================================================
void Hook_Behavior(edict_t *hook) {
	vec3_t zvec={0,0,0};
	vec3_t start={0,0,0};
	vec3_t chainvec={0,0,0}; // chain's vector
	float chainlen; // length of extended chain
	vec3_t velpart={0,0,0}; // player's velocity in relation to hook
	float f1, f2, dprod; // restrainment forces
	bool chain_moving=false;
	int state;

	state=HOOK_OWNERS_HOOKSTATE;

	// Decide when to disconnect hook
	// if hook is not ON OR
	// if target is no longer solid OR
	//(i.e. hook broke glass; exploded barrels, gibs)
	// if player has died OR
	// if player goes through teleport

	if ((!(state & HOOK_ON))
		||(hook->enemy->solid == SOLID_NOT)
		||(hook->owner->deadflag)
		||(hook->owner->s.event == EV_PLAYER_TELEPORT)){
			DropHook(hook);
			return; }

	// gives hook same velocity as the entity it stuck to
	Vec3Copy(hook->enemy->velocity, hook->velocity);

	// Grow the length of the chain
	if (state & GROW_ON) {
		chain_moving = true;
		hook->angle += 20; }
	else
		// Shrink the length of the chain to Minimum
		if (state & SHRINK_ON)
			if (hook->angle > 10) {
				chain_moving = true;
				hook->angle -= 60;}
			if (hook->angle < 10) {
				hook->angle = 10;
				chain_moving = false;}

			// Make some chain sound occur..
			play_moving_chain_sound(hook, chain_moving);
			chain_moving=false; // chain not always moving..

			//================
			// chain physics
			//================

			// Get chain start position
			get_start_position(hook, start);

			Vec3Subtract(hook->s.origin, start, chainvec); // get chain's vector
			chainlen = Vec3Length(chainvec); // get Chain Length at Vector

			f1=0; // default state

			// if player's location is beyond the chain's reach
			if (chainlen > hook->angle) {
				// determine player's velocity component of chain vector
				dprod=DotProduct(hook->owner->velocity, chainvec)/DotProduct(chainvec, chainvec);
				Vec3Scale(chainvec, dprod, velpart);
				// restrainment default force
				f2 =(chainlen - hook->angle)*5;
				// if player's velocity heading is away from the hook
				if (DotProduct(hook->owner->velocity, chainvec) < 0) {
					// if chain has streched for 25 units
					if (chainlen > hook->angle + 25)
						// remove player's velocity component moving away from hook
						Vec3Subtract(hook->owner->velocity, velpart, hook->owner->velocity);
					f1 = f2; }
				else
					// if player's velocity heading is towards the hook
					if (Vec3Length(velpart) < f2)
						f1 = f2 - Vec3Length(velpart);
			} // end if

			// applies chain restrainment
			VectorNormalizeFastf(chainvec);
			Vec3MA(hook->owner->velocity, f1, chainvec, hook->owner->velocity);

			get_start_position(hook, start);

			if (hook->owner->client->hooktype == GRAPPLE_STYLE)
				G_Spawn_Models(TE_PARASITE_ATTACK,(short)(hook-g_edicts), start, hook->s.origin, zvec, start);
			else
				G_Spawn_Trails(TE_BFG_LASER, start, hook->s.origin, hook->s.origin);

			// set next think time
			hook->nextthink = PRESENT_TIME + FRAMETIME;
}

//==========================================================
void Hook_Airborne(edict_t *hook) {
	vec3_t zvec={0,0,0},start={0,0,0};
	int state;

	state=HOOK_OWNERS_HOOKSTATE;

	// if hook not ON then exit..
	if (!(state & HOOK_ON)) {
		DropHook(hook);
		return; }

	get_start_position(hook, start);

	if (hook->owner->client->hooktype == GRAPPLE_STYLE)
		G_Spawn_Models(TE_PARASITE_ATTACK,(short)(hook-g_edicts), start, hook->s.origin, zvec, start);
	else
		G_Spawn_Trails(TE_BFG_LASER, start, hook->s.origin, hook->s.origin);

	hook->nextthink = PRESENT_TIME+FRAMETIME;
}

//==========================================================
void Hook_Touch(edict_t *hook, edict_t *other, plane_t *plane, cmBspSurface_t *surf){
	vec3_t start={0,0,0}; // chain's start vector
	vec3_t chainvec={0,0,0}; // chain's end vector
	float chainlen;
	int state;

	state=HOOK_OWNERS_HOOKSTATE;

	// if hook not ON or been fired at self then exit..
	if (!(state & HOOK_ON) ||(hook == other)) {
		DropHook(hook);
		return; }

	// Get chain start position
	get_start_position(hook, start);

	Vec3Subtract(hook->s.origin,start,chainvec);
	chainlen = Vec3Length(chainvec);
	// member angle is used to store the length of the chain
	hook->angle = chainlen;

	switch (other->solid) {
case SOLID_BBOX:
	if (other->client) {
		gi.sound(hook, CHAN_VOICE, HOOK_SMACK_SOUND, 1, ATTN_NORM, 0);
		// Show spurts of blood upon impact with player.
		G_Spawn_Sparks(TE_BLOOD, hook->s.origin, plane->normal, hook->s.origin);}
	break;
case SOLID_BSP:
	// Show some sparks upon impact.
	//G_Spawn_Sparks(TE_SPARKS, hook->s.origin, plane->normal, hook->s.origin);
	// Play chain hit 'clink'
	gi.sound(hook, CHAN_VOICE, HOOK_HIT_SOUND, 1, ATTN_NORM, 0);
	// Clear the velocity vector
	Vec3Clear(hook->avelocity);
	break;
	} // end switch

	// inflict damage on damageable items
	if (other->takedamage)
		T_Damage(other, hook, hook->owner, hook->velocity, hook->s.origin, plane->normal, hook->dmg, 100, 0, MOD_HIT);

	// hook gets the same velocity as the item it attached to
	Vec3Copy(other->velocity, hook->velocity);

	// Automatic hook pulling upon contact.
	//HOOK_OWNERS_HOOKSTATE |= SHRINK_ON;

	hook->enemy = other;
	hook->touch = NULL;
	hook->think = Hook_Behavior;

	hook->nextthink = PRESENT_TIME + FRAMETIME;
}

//==========================================================
void Fire_Grapple_Hook(edict_t *ent) {
	edict_t *hook=NULL;
	vec3_t offset={0,0,0}, start={0,0,0};
	vec3_t forward={0,0,0}, right={0,0,0};
	int *hookstate=NULL;

	gi.sound(ent, CHAN_WEAPON, HOOK_LAUNCH_SOUND, 1, ATTN_NORM, 0);

	hookstate = &ENTS_HOOKSTATE;
	*hookstate = HOOK_ON;

	// derive point of hook origin
	Angles_Vectors(ENTS_V_ANGLE, forward, right, NULL);
	Vec3Set(offset, 8, 8, ENTS_VIEW_HEIGHT-8);
	P_ProjectSource_Reverse(ent->client, ENTS_S_ORIGIN, offset, forward, right, start);

	// spawn hook hook
	hook = G_Spawn();
	Vec3Copy(start, hook->s.origin);
	Vec3Copy(forward, hook->movedir);
	VecToAngles(forward, hook->s.angles);
	Vec3Scale(forward, 1600, hook->velocity);
	hook->movetype = MOVETYPE_FLY;
	hook->clipMask = CONTENTS_MASK_SHOT;
	hook->solid = SOLID_BBOX;
	Vec3Clear(hook->mins);
	Vec3Clear(hook->maxs);
	hook->owner = ent;
	hook->dmg = 20; // 20 Units of damage at impact.
	hook->sounds = MOTOR_OFF; // keeps track of motor chain sound played

	if (hook->owner->client->hooktype == GRAPPLE_STYLE) {
		hook->s.modelIndex = gi.modelindex(HOOK_MODEL);
		Vec3Set(hook->avelocity,0,0,-2000); }

	hook->touch = Hook_Touch; // Function to call upon impact
	hook->think = Hook_Airborne; // Function to call while hook in air
	hook->nextthink = PRESENT_TIME + FRAMETIME;

	gi.linkentity(hook);
}


//==========================================================
void Cmd_Hook_f(edict_t *ent, char *cmd) {
	char *s=gi.argv(1);
	int *hookstate=NULL;

	if (!G_ClientInGame(ent)) return;

	if (Q_stricmp(cmd, "hook") == 0)
		ent->client->hooktype = GRAPPLE_STYLE;
	else
		ent->client->hooktype = LASER_STYLE;

	// create intermediate value
	hookstate = &ent->client->hookstate;

	if ((!(*hookstate & HOOK_ON))
		&& (Q_stricmp(s, "action") == 0)) {
			Fire_Grapple_Hook(ent);
			return; }

	if (*hookstate & HOOK_ON) {
		if (Q_stricmp(s, "action") == 0) {
			*hookstate = HOOK_OFF; // release hook
			return; }

		// deactivate chain growth or shrink
		if (Q_stricmp(s, "stop") == 0) {
			*hookstate -= *hookstate &(GROW_ON | SHRINK_ON);
			return; }

		// activate chain growth
		if (Q_stricmp(s, "grow") == 0) {
			*hookstate |= GROW_ON;
			*hookstate -= *hookstate & SHRINK_ON;
			return; }

		// activate chain shrinking
		if (Q_stricmp(s, "shrink") == 0) {
			*hookstate |= SHRINK_ON;
			*hookstate -= *hookstate & GROW_ON; }
	} // endif
}