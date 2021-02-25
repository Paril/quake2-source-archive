#include "g_local.h"
#include "m_genqguy.h"

//======================================================================

// self is grapple, not player
void Q1GrappleReset(edict_t *self)
{
	if (self->owner->client->ctf_grapple)
	{
		gclient_t *cl;

		gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("q1weap/hook/bounce2.wav"), 1.0, ATTN_NORM, 0);
		cl = self->owner->client;
		cl->ctf_grapple = NULL;
		cl->ctf_grapplereleasetime = level.time;
		cl->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
		cl->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		G_FreeEdict(self);
	}
}


void Q1Grapple_Touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (self->owner->client->ctf_grapplestate != CTF_GRAPPLE_STATE_FLY)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		Q1GrappleReset(self);
		return;
	}

	VectorCopy(vec3_origin, self->velocity);
	VectorCopy(vec3_origin, self->avelocity);

	PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage) 
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_GRAPPLE);
		//Hook_Reset(self);
		//return;
		self->s.modelindex = 0;
	}

	self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_PULL; // we're on hook
	self->enemy = other;

	self->solid = SOLID_NOT;

//	gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhit.wav"), volume, ATTN_NORM, 0);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (self->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

// draw beam between grapple and self
void Q1GrappleDrawCable(edict_t *self)
{
	vec3_t	offset, start, end, f, r;
	vec3_t	dir;
	float	distance;

	AngleVectors (self->owner->client->v_angle, f, r, NULL);
	VectorSet(offset, 24, 4, self->owner->viewheight-8+2);
	P_ProjectSource (self->owner->client, self->owner->s.origin, offset, f, r, start);
	VectorSubtract(start, self->owner->s.origin, offset);

	VectorSubtract (start, self->s.origin, dir);
	distance = VectorLength(dir);

	// don't draw cable if close
	if (distance < 64)
		return;
	VectorCopy (self->s.origin, end);

	//CHASECAM crap	
	if(self->owner->client->chasetoggle)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GRAPPLE_CABLE);
		gi.WriteShort (self->owner->client->oldplayer - g_edicts);
		gi.WritePosition (self->owner->client->oldplayer->s.origin);
		gi.WritePosition (end);
		gi.WritePosition (offset);
		gi.multicast (self->owner->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GRAPPLE_CABLE);
		gi.WriteShort (self->owner - g_edicts);
		gi.WritePosition (self->owner->s.origin);
		gi.WritePosition (end);
		gi.WritePosition (offset);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}
}

// pull the player toward the grapple
void Q1GrapplePull(edict_t *self)
{
	vec3_t hookdir, v;
	float vlen;

	if (strcmp(self->owner->client->pers.weapon->classname, "weapon_q1_grapple") == 0 &&
		!self->owner->client->newweapon &&
		self->owner->client->weaponstate != WEAPON_FIRING &&
		self->owner->client->weaponstate != WEAPON_ACTIVATING) 
	{
		Q1GrappleReset(self);
		return;
	}

	if (self->enemy)
	{
		if (self->enemy->solid == SOLID_NOT) 
		{
			Q1GrappleReset(self);
			return;
		}
		if (self->enemy->solid == SOLID_BBOX)
		{
			VectorScale(self->enemy->size, 0.5, v);
			VectorAdd(v, self->enemy->s.origin, v);
			VectorAdd(v, self->enemy->mins, self->s.origin);
			gi.linkentity (self);
		} 
		else
			VectorCopy(self->enemy->velocity, self->velocity);
		
		if (self->enemy->takedamage &&
			!CheckTeamDamage (self->enemy, self->owner)) 
		{
			T_Damage (self->enemy, self, self->owner, self->velocity, self->s.origin, vec3_origin, 1, 1, 0, MOD_GRAPPLE);
			
			if(self->enemy->client)
				gi.sound (self->enemy, CHAN_WEAPON, gi.soundindex("q1weap/axe/axehit1.wav"), 1, ATTN_NORM, 0);
		}
		
		if (self->enemy->deadflag) 
		{ // he died
			Q1GrappleReset(self);
			return;
		}
	}

	Q1GrappleDrawCable(self);

	if (self->owner->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
	{
		// pull player toward grapple
		// this causes icky stuff with prediction, we need to extend
		// the prediction layer to include two new fields in the player
		// move stuff: a point and a velocity.  The client should add
		// that velociy in the direction of the point
		vec3_t forward, up;

		AngleVectors (self->owner->client->v_angle, forward, NULL, up);
		VectorCopy(self->owner->s.origin, v);
		v[2] += self->owner->viewheight;
		VectorSubtract (self->s.origin, v, hookdir);

		vlen = VectorLength(hookdir);

		if (self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL &&
			vlen < 64)
		{
			self->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
			gi.sound (self->owner, CHAN_WEAPON, gi.soundindex("q1weap/hook/chain3.wav"), 1.0, ATTN_NORM, 0);
			self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_HANG;
		}

		VectorNormalize (hookdir);
		VectorScale(hookdir, 800, hookdir);
		VectorCopy(hookdir, self->owner->velocity);
		//SV_AddGravity(self->owner);
	}
}

void Grapple_SpawnStar (edict_t *self, vec3_t start, vec3_t dir, int damage)
{
	edict_t *star;
	trace_t tr;

	VectorNormalize (dir);

    star = G_Spawn ();
	VectorCopy (start, star->s.origin);
	VectorCopy (start, star->s.old_origin);
	vectoangles (dir, star->s.angles);
	VectorScale (dir, 800, star->velocity);
    star->movetype = MOVETYPE_FLYMISSILE;
	star->clipmask = MASK_SHOT;
    star->solid = SOLID_BBOX;
	star->s.effects |= 0;
	VectorClear(star->mins);
	VectorClear(star->maxs);
	star->s.modelindex =gi.modelindex ("models/weapons/v_q1hook/star/tris.md2");
    star->owner = self;
	star->dmg = damage;
	star->touch = Q1Grapple_Touch;
	star->classname = "hook";
	self->client->ctf_grapple = star;
	self->client->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
	// This gives the hook some spin as it's flying
	VectorSet(star->avelocity, 0, 0, -500);

	
	gi.linkentity (star);

	tr = gi.trace(self->s.origin, NULL, NULL, star->s.origin, star, MASK_SHOT);
	if (tr.fraction < 1.0) 
	{
		VectorMA (star->s.origin, -10, dir, star->s.origin);
		star->touch(star, tr.ent, NULL, NULL);
	}
};

void Q1_GrappleFire (edict_t *ent, vec3_t g_offset, int damage, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	
	if (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
		return; // it's already out

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 6, ent->viewheight-8+2);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	//VectorScale (forward, -2, ent->client->kick_origin);
	//ent->client->kick_angles[0] = -2;

	gi.sound (ent->owner, CHAN_WEAPON, gi.soundindex("q1weap/hook/chain1.wav"), 1.0, ATTN_NORM, 0);
	Grapple_SpawnStar (ent, start, forward, damage);
	PlayerNoise(ent, start, PNOISE_WEAPON);
}


void Weapon_Q1_Grapple_Fire (edict_t *ent)
{
	int damage = 10;
	
	if(ent->client->quad_framenum > level.framenum)
		damage *= 4;
	
	Q1_GrappleFire (ent, vec3_origin, damage, 0);
}

void Weapon_Q1 (edict_t *ent, int FRAME_FIRST, int FRAME_LAST, int *fire_frames, void (*fire)(edict_t *ent));

void Weapon_Q1_Grapple (edict_t *ent)
{
	static int	fire_frames[]	= {8,0};
	int prevstate;

	// if the the attack button is still down, stay in the firing frame
	if (ent->client->weaponstate == WEAPON_FIRING && ent->client->ctf_grapple)
	{
		if(ent->client->buttons & BUTTON_ATTACK)
		{
			ent->client->ps.gunframe = 8;
			
			if(ent->client->invis_framenum < level.framenum)
			{
				ent->s.frame = FRAME_axe_D_attack3 - 1;
				ent->client->anim_end = FRAME_axe_D_attack3;
			}
		}
		else if(ent->client->invis_framenum < level.framenum)
		{
			//Player Animations
			ent->client->anim_priority = ANIM_ATTACK;
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_axe_crattak1-1;
				ent->client->anim_end = FRAME_axe_crattak3;
			}
			else 
			{
				ent->s.frame = FRAME_axe_D_attack1 - 1;
				ent->client->anim_end = FRAME_axe_D_attack3;
			}
		}
	}

	if (!(ent->client->buttons & BUTTON_ATTACK) && ent->client->ctf_grapple)
	{
		Q1GrappleReset(ent->client->ctf_grapple);
//if (ent->client->weaponstate == WEAPON_FIRING)
			ent->client->weaponstate = WEAPON_READY;
	}

	if (ent->client->newweapon && 
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY &&
		ent->client->weaponstate == WEAPON_FIRING)
	{
		// he wants to change weapons while grappled
		ent->client->weaponstate = WEAPON_DROPPING;
	}

	prevstate = ent->client->weaponstate;
	Weapon_Q1 (ent, 5, 10, fire_frames , Weapon_Q1_Grapple_Fire);

	// if we just switched back to grapple, immediately go to fire frame
	if (prevstate == WEAPON_ACTIVATING &&
		ent->client->weaponstate == WEAPON_READY &&
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
	{
		if (!(ent->client->buttons & BUTTON_ATTACK))
			ent->client->ps.gunframe = 5;
		else
			ent->client->ps.gunframe = 8;
		ent->client->weaponstate = WEAPON_FIRING;
	}
}
