#include "g_local.h"

#define INCLUDE_ETF_RIFLE		1
#define INCLUDE_PROX			1
//#define INCLUDE_FLAMETHROWER	1
//#define INCLUDE_INCENDIARY		1
#define INCLUDE_NUKE			1
#define INCLUDE_MELEE			1
#define INCLUDE_TESLA			1
#define INCLUDE_BEAMS			1

extern void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed);
extern void hurt_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
extern void droptofloor (edict_t *ent);
extern void Grenade_Explode (edict_t *ent);

extern void drawbbox (edict_t *ent);

#ifdef INCLUDE_ETF_RIFLE
/*
========================
fire_flechette
========================
*/
void flechette_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		dir;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
//gi.dprintf("t_damage %s\n", other->classname);
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal,
			self->dmg, self->dmg_radius, DAMAGE_NO_REG_ARMOR, MOD_ETF_RIFLE);
	}
	else
	{
		if(!plane)
			VectorClear (dir);
		else
			VectorScale (plane->normal, 256, dir);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_FLECHETTE);
		gi.WritePosition (self->s.origin);
		gi.WriteDir (dir);
		gi.multicast (self->s.origin, MULTICAST_PVS);

//		T_RadiusDamage(self, self->owner, 24, self, 48, MOD_ETF_RIFLE);
	}

	G_FreeEdict (self);
}

void fire_flechette (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int kick)
{
	edict_t *flechette;

	VectorNormalize (dir);

	flechette = G_Spawn();
	VectorCopy (start, flechette->s.origin);
	VectorCopy (start, flechette->s.old_origin);
	vectoangles2 (dir, flechette->s.angles);

	VectorScale (dir, speed, flechette->velocity);
	flechette->movetype = MOVETYPE_FLYMISSILE;
	flechette->clipmask = MASK_SHOT;
	flechette->solid = SOLID_BBOX;
	flechette->s.renderfx = RF_FULLBRIGHT;
	VectorClear (flechette->mins);
	VectorClear (flechette->maxs);
	
	flechette->s.modelindex = gi.modelindex ("models/proj/flechette/tris.md2");

//	flechette->s.sound = gi.soundindex ("");			// FIXME - correct sound!
	flechette->owner = self;
	flechette->touch = flechette_touch;
	flechette->nextthink = level.time + 8000/speed;
	flechette->think = G_FreeEdict;
	flechette->dmg = damage;
	flechette->dmg_radius = kick;

	gi.linkentity (flechette);
	
	if (self->client)
		check_dodge (self, flechette->s.origin, dir, speed);
}
#endif

// **************************
// PROX
// **************************

#ifdef INCLUDE_PROX
#define PROX_TIME_TO_LIVE	45		// 45, 30, 15, 10
#define PROX_TIME_DELAY		0.5
#define PROX_BOUND_SIZE		96
#define PROX_DAMAGE_RADIUS	192
#define PROX_HEALTH			20
#define	PROX_DAMAGE			90

//===============
//===============
void Prox_Explode (edict_t *ent)
{
	vec3_t		origin;
	edict_t		*owner;

// free the trigger field

	//PMM - changed teammaster to "mover" .. owner of the field is the prox
	if(ent->teamchain && ent->teamchain->owner == ent)
		G_FreeEdict(ent->teamchain);

	owner = ent;
	if(ent->teammaster)
	{
		owner = ent->teammaster;
		PlayerNoise(owner, ent->s.origin, PNOISE_IMPACT);
	}

	// play quad sound if appopriate
	if (ent->dmg > PROX_DAMAGE)
		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

	ent->takedamage = DAMAGE_NO;
	T_RadiusDamage(ent, owner, ent->dmg, ent, PROX_DAMAGE_RADIUS, MOD_PROX);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->groundentity)
		gi.WriteByte (TE_GRENADE_EXPLOSION);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	G_FreeEdict (ent);
}

//===============
//===============
void prox_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	gi.dprintf("prox_die\n");
	// if set off by another prox, delay a little (chained explosions)
	if (strcmp(inflictor->classname, "prox"))
	{
		self->takedamage = DAMAGE_NO;
		Prox_Explode(self);
	}
	else
	{
		self->takedamage = DAMAGE_NO;
		self->think = Prox_Explode;
		self->nextthink = level.time + FRAMETIME;
	}
}

//===============
//===============
void Prox_Field_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t *prox;

	if (!(other->svflags & SVF_MONSTER) && !other->client)
		return;

	// trigger the prox mine if it's still there, and still mine.
	prox = ent->owner;

	if (other == prox) // don't set self off
		return;

	if (prox->think == Prox_Explode) // we're set to blow!
	{
//		if ((g_showlogic) && (g_showlogic->value))
//			gi.dprintf ("%f - prox already gone off!\n", level.time);
		return;
	}

	if(prox->teamchain == ent)
	{
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/proxwarn.wav"), 1, ATTN_NORM, 0);
		prox->think = Prox_Explode;
		prox->nextthink = level.time + PROX_TIME_DELAY;
		return;
	}

	ent->solid = SOLID_NOT;
	G_FreeEdict(ent);
}

//===============
//===============
void prox_seek (edict_t *ent)
{
	if(level.time > ent->wait)
	{
		Prox_Explode(ent);
	}
	else
	{
		ent->s.frame++;
		if(ent->s.frame > 13)
			ent->s.frame = 9;
		ent->think = prox_seek;
		ent->nextthink = level.time + 0.1;
	}
}

//===============
//===============
void prox_open (edict_t *ent)
{
	edict_t *search;

	search = NULL;
//	gi.dprintf("prox_open %d\n", ent->s.frame);	
//	gi.dprintf("%f\n", ent->velocity[2]);
	if(ent->s.frame == 9)	// end of opening animation
	{
		// set the owner to NULL so the owner can shoot it, etc.  needs to be done here so the owner
		// doesn't get stuck on it while it's opening if fired at point blank wall
		ent->s.sound = 0;
		ent->owner = NULL;
		if(ent->teamchain)
			ent->teamchain->touch = Prox_Field_Touch;
		while ((search = findradius(search, ent->s.origin, PROX_DAMAGE_RADIUS+10)) != NULL)
		{
			if (!search->classname)			// tag token and other weird shit
				continue;

//			if (!search->takedamage)
//				continue;
			// if it's a monster or player with health > 0
			// or it's a player start point
			// and we can see it
			// blow up
			if (
				(
					(((search->svflags & SVF_MONSTER) || (search->client)) && (search->health > 0))	|| 
					(
						(deathmatch->value) && 
						(
						(!strcmp(search->classname, "info_player_deathmatch")) ||
						(!strcmp(search->classname, "info_player_start")) ||
						(!strcmp(search->classname, "info_player_coop")) ||
						(!strcmp(search->classname, "misc_teleporter_dest"))
						)
					)
				) 
				&& (visible (search, ent))
			   )
			{
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/proxwarn.wav"), 1, ATTN_NORM, 0);
				Prox_Explode (ent);
				return;
			}
		}

		if (strong_mines && (strong_mines->value))
			ent->wait = level.time + PROX_TIME_TO_LIVE;
		else
		{
			switch (ent->dmg/PROX_DAMAGE)
			{
				case 1:
					ent->wait = level.time + PROX_TIME_TO_LIVE;
					break;
				case 2:
					ent->wait = level.time + 30;
					break;
				case 4:
					ent->wait = level.time + 15;
					break;
				case 8:
					ent->wait = level.time + 10;
					break;
				default:
//					if ((g_showlogic) && (g_showlogic->value))
//						gi.dprintf ("prox with unknown multiplier %d!\n", ent->dmg/PROX_DAMAGE);
					ent->wait = level.time + PROX_TIME_TO_LIVE;
					break;
			}
		}

//		ent->wait = level.time + PROX_TIME_TO_LIVE;
		ent->think = prox_seek;
		ent->nextthink = level.time + 0.2;
	}
	else
	{
		if (ent->s.frame == 0)
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/proxopen.wav"), 1, ATTN_NORM, 0);
		//ent->s.sound = gi.soundindex ("weapons/proxopen.wav");
		ent->s.frame++;
		ent->think = prox_open;
		ent->nextthink = level.time + 0.05;	
	}
}

//===============
//===============
void prox_land (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*field;
	vec3_t	dir;
	vec3_t	forward, right, up;
	int		makeslave = 0;
	int		movetype = MOVETYPE_NONE;
	int		stick_ok = 0;
	vec3_t	land_point;

	// must turn off owner so owner can shoot it and set it off
	// moved to prox_open so owner can get away from it if fired at pointblank range into
	// wall
//	ent->owner = NULL;

//	if ((g_showlogic) && (g_showlogic->value))
//		gi.dprintf ("land - %2.2f %2.2f %2.2f\n", ent->velocity[0], ent->velocity[1], ent->velocity[2]);

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict(ent);
		return;
	}

	if (plane->normal)
	{
		VectorMA (ent->s.origin, -10.0, plane->normal, land_point);
		if (gi.pointcontents (land_point) & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			Prox_Explode (ent);
			return;
		}
	}

	if ((other->svflags & SVF_MONSTER) || other->client || (other->svflags & SVF_DAMAGEABLE))
	{
		if(other != ent->teammaster)
			Prox_Explode(ent);

		return;
	}

#define STOP_EPSILON	0.1

	else if (other != world)
	{
		//Here we need to check to see if we can stop on this entity.
		//Note that plane can be NULL

		//PMM - code stolen from g_phys (ClipVelocity)
		vec3_t out;
		float backoff, change;
		int i;

		if (!plane->normal) // this happens if you hit a point object, maybe other cases
		{
			// Since we can't tell what's going to happen, just blow up
//			if ((g_showlogic) && (g_showlogic->value))
//				gi.dprintf ("bad normal for surface, exploding!\n");

			Prox_Explode(ent);
			return;
		}

		if ((other->movetype == MOVETYPE_PUSH) && (plane->normal[2] > 0.7))
			stick_ok = 1;
		else
			stick_ok = 0;

		backoff = DotProduct (ent->velocity, plane->normal) * 1.5;
		for (i=0 ; i<3 ; i++)
		{
			change = plane->normal[i]*backoff;
			out[i] = ent->velocity[i] - change;
			if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
				out[i] = 0;
		}

		if (out[2] > 60)
			return;

		movetype = MOVETYPE_BOUNCE;

		// if we're here, we're going to stop on an entity
		if (stick_ok)
		{ // it's a happy entity
			VectorCopy (vec3_origin, ent->velocity);
			VectorCopy (vec3_origin, ent->avelocity);
		}
		else // no-stick.  teflon time
		{
			if (plane->normal[2] > 0.7)
			{
//				if ((g_showlogic) && (g_showlogic->value))
//					gi.dprintf ("stuck on entity, blowing up!\n");

				Prox_Explode(ent);
				return;
			}
			return;
		}
	}
	else if (other->s.modelindex != 1)
		return;

	vectoangles2 (plane->normal, dir);
	AngleVectors (dir, forward, right, up);

	if (gi.pointcontents (ent->s.origin) & (CONTENTS_LAVA|CONTENTS_SLIME))
	{
		Prox_Explode (ent);
		return;
	}

	field = G_Spawn();

	VectorCopy (ent->s.origin, field->s.origin);
	VectorClear(field->velocity);
	VectorClear(field->avelocity);
	VectorSet(field->mins, -PROX_BOUND_SIZE, -PROX_BOUND_SIZE, -PROX_BOUND_SIZE);
	VectorSet(field->maxs, PROX_BOUND_SIZE, PROX_BOUND_SIZE, PROX_BOUND_SIZE);
	field->movetype = MOVETYPE_NONE;
	field->solid = SOLID_TRIGGER;
	field->owner = ent;
	field->classname = "prox_field";
	field->teammaster = ent;
	gi.linkentity (field);

	VectorClear(ent->velocity);
	VectorClear(ent->avelocity);
	// rotate to vertical
	dir[PITCH] = dir[PITCH] + 90;
	VectorCopy (dir, ent->s.angles);
	ent->takedamage = DAMAGE_AIM;
	ent->movetype = movetype;		// either bounce or none, depending on whether we stuck to something
	ent->die = prox_die;
	ent->teamchain = field;
	ent->health = PROX_HEALTH;
	ent->nextthink = level.time + 0.05;
	ent->think = prox_open;
	ent->touch = NULL;
	ent->solid = SOLID_BBOX;
	// record who we're attached to
//	ent->teammaster = other;

	gi.linkentity(ent);
}

//===============
//===============
void fire_prox (edict_t *self, vec3_t start, vec3_t aimdir, int damage_multiplier, int speed)
{
	edict_t	*prox;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles2 (aimdir, dir);
	AngleVectors (dir, forward, right, up);

//	if ((g_showlogic) && (g_showlogic->value))
//		gi.dprintf ("start %s    aim %s   speed %d\n", vtos(start), vtos(aimdir), speed);
	prox = G_Spawn();
	VectorCopy (start, prox->s.origin);
	VectorScale (aimdir, speed, prox->velocity);
	VectorMA (prox->velocity, 200 + crandom() * 10.0, up, prox->velocity);
	VectorMA (prox->velocity, crandom() * 10.0, right, prox->velocity);
	VectorCopy (dir, prox->s.angles);
	prox->s.angles[PITCH]-=90;
	prox->movetype = MOVETYPE_BOUNCE;
	prox->solid = SOLID_BBOX; 
	prox->s.effects |= EF_GRENADE;
	prox->clipmask = MASK_SHOT|CONTENTS_LAVA|CONTENTS_SLIME;
	prox->s.renderfx |= RF_IR_VISIBLE;
	//FIXME - this needs to be bigger.  Has other effects, though.  Maybe have to change origin to compensate
	// so it sinks in correctly.  Also in lavacheck, might have to up the distance
	VectorSet (prox->mins, -6, -6, -6);
	VectorSet (prox->maxs, 6, 6, 6);
	prox->s.modelindex = gi.modelindex ("models/weapons/g_prox/tris.md2");
	prox->owner = self;
	prox->teammaster = self;
	prox->touch = prox_land;
//	prox->nextthink = level.time + PROX_TIME_TO_LIVE;
	prox->think = Prox_Explode;
	prox->dmg = PROX_DAMAGE*damage_multiplier;
	prox->classname = "prox";
	prox->svflags |= SVF_DAMAGEABLE;
	prox->flags |= FL_MECHANICAL;

	switch (damage_multiplier)
	{
	case 1:
		prox->nextthink = level.time + PROX_TIME_TO_LIVE;
		break;
	case 2:
		prox->nextthink = level.time + 30;
		break;
	case 4:
		prox->nextthink = level.time + 15;
		break;
	case 8:
		prox->nextthink = level.time + 10;
		break;
	default:
//		if ((g_showlogic) && (g_showlogic->value))
//			gi.dprintf ("prox with unknown multiplier %d!\n", damage_multiplier);
		prox->nextthink = level.time + PROX_TIME_TO_LIVE;
		break;
	}

	gi.linkentity (prox);
}
#endif

// *************************
// FLAMETHROWER
// *************************

#ifdef INCLUDE_FLAMETHROWER
#define FLAMETHROWER_RADIUS		8

void fire_remove (edict_t *ent)
{
	if(ent == ent->owner->teamchain)
		ent->owner->teamchain = NULL;

	G_FreeEdict(ent);	
}

void fire_flame (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
{
	edict_t *flame;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles2 (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	flame = G_Spawn();

	// the origin is the first control point, put it speed forward.
	VectorMA(start, speed, forward, flame->s.origin);

	// record that velocity
	VectorScale (aimdir, speed, flame->velocity);

	VectorCopy (dir, flame->s.angles);
	flame->movetype = MOVETYPE_NONE;
	flame->solid = SOLID_NOT;

	VectorSet(flame->mins, -FLAMETHROWER_RADIUS, -FLAMETHROWER_RADIUS, -FLAMETHROWER_RADIUS);
	VectorSet(flame->maxs, FLAMETHROWER_RADIUS, FLAMETHROWER_RADIUS, FLAMETHROWER_RADIUS);

	flame->s.sound = gi.soundindex ("weapons/flame.wav");
	flame->owner = self;
	flame->dmg = damage;
	flame->classname = "flame";

	// clear control points and velocities
	VectorCopy (flame->s.origin, flame->flameinfo.pos1);
	VectorCopy (flame->velocity, flame->flameinfo.vel1);
	VectorCopy (flame->s.origin, flame->flameinfo.pos2);
	VectorCopy (flame->velocity, flame->flameinfo.vel2);
	VectorCopy (flame->s.origin, flame->flameinfo.pos3);
	VectorCopy (flame->velocity, flame->flameinfo.vel3);
	VectorCopy (flame->s.origin, flame->flameinfo.pos4);

	// hook flame stream to owner
	self->teamchain = flame;

	gi.linkentity (flame);
}

// fixme - change to use start location, not entity origin
void fire_maintain (edict_t *ent, edict_t *flame, vec3_t start, vec3_t aimdir, int damage, int speed)
{
	trace_t	tr;

	// move the control points out the appropriate direction and velocity
	VectorAdd(flame->flameinfo.pos3, flame->flameinfo.vel3, flame->flameinfo.pos4);
	VectorAdd(flame->flameinfo.pos2, flame->flameinfo.vel2, flame->flameinfo.pos3);
	VectorAdd(flame->flameinfo.pos1, flame->flameinfo.vel1, flame->flameinfo.pos2);
	VectorAdd(flame->s.origin,		 flame->velocity,       flame->flameinfo.pos1);

	// move the velocities for the control points
	VectorCopy(flame->flameinfo.vel2, flame->flameinfo.vel3);
	VectorCopy(flame->flameinfo.vel1, flame->flameinfo.vel2);
	VectorCopy(flame->velocity,		  flame->flameinfo.vel1);

	// set velocity and location for new control point 0.
	VectorMA(start, speed, aimdir, flame->s.origin);
	VectorScale(aimdir, speed, flame->velocity);
	
	//
	// does it hit a wall? if so, when?
	//

	// player fire point to flame origin.
	tr = gi.trace(start, flame->mins, flame->maxs,
					flame->s.origin, flame, MASK_SHOT);
	if(tr.fraction == 1.0)
	{
		// origin to point 1
		tr = gi.trace(flame->s.origin, flame->mins, flame->maxs,
						flame->flameinfo.pos1, flame, MASK_SHOT);
		if(tr.fraction == 1.0)
		{
			// point 1 to point 2
			tr = gi.trace(flame->flameinfo.pos1, flame->mins, flame->maxs,
							flame->flameinfo.pos2, flame, MASK_SHOT);
			if(tr.fraction == 1.0)
			{
				// point 2 to point 3
				tr = gi.trace(flame->flameinfo.pos2, flame->mins, flame->maxs,
							flame->flameinfo.pos3, flame, MASK_SHOT);
				if(tr.fraction == 1.0)
				{
					// point 3 to point 4, point 3 valid
					tr = gi.trace(flame->flameinfo.pos3, flame->mins, flame->maxs,
								flame->flameinfo.pos4, flame, MASK_SHOT);
					if(tr.fraction < 1.0) // point 4 blocked
					{
						VectorCopy(tr.endpos, flame->flameinfo.pos4);
					}
				}
				else	// point 3 blocked, point 2 valid
				{
					VectorCopy(flame->flameinfo.vel2, flame->flameinfo.vel3);
					VectorCopy(tr.endpos, flame->flameinfo.pos3);
					VectorCopy(tr.endpos, flame->flameinfo.pos4);
				}
			}
			else	// point 2 blocked, point 1 valid
			{
				VectorCopy(flame->flameinfo.vel1, flame->flameinfo.vel2);
				VectorCopy(flame->flameinfo.vel1, flame->flameinfo.vel3);
				VectorCopy(tr.endpos, flame->flameinfo.pos2);
				VectorCopy(tr.endpos, flame->flameinfo.pos3);
				VectorCopy(tr.endpos, flame->flameinfo.pos4);
			}
		}
		else	// point 1 blocked, origin valid
		{
			VectorCopy(flame->velocity, flame->flameinfo.vel1);
			VectorCopy(flame->velocity, flame->flameinfo.vel2);
			VectorCopy(flame->velocity, flame->flameinfo.vel3);
			VectorCopy(tr.endpos, flame->flameinfo.pos1);
			VectorCopy(tr.endpos, flame->flameinfo.pos2);
			VectorCopy(tr.endpos, flame->flameinfo.pos3);
			VectorCopy(tr.endpos, flame->flameinfo.pos4);
		}
	}
	else // origin blocked!
	{
//		gi.dprintf("point 2 blocked\n");
		VectorCopy(flame->velocity, flame->flameinfo.vel1);
		VectorCopy(flame->velocity, flame->flameinfo.vel2);
		VectorCopy(flame->velocity, flame->flameinfo.vel3);
		VectorCopy(tr.endpos, flame->s.origin);
		VectorCopy(tr.endpos, flame->flameinfo.pos1);
		VectorCopy(tr.endpos, flame->flameinfo.pos2);
		VectorCopy(tr.endpos, flame->flameinfo.pos3);
		VectorCopy(tr.endpos, flame->flameinfo.pos4);
	}
	
	if(tr.fraction < 1.0 && tr.ent->takedamage)
	{
		T_Damage (tr.ent, flame, ent, flame->velocity, tr.endpos, tr.plane.normal, 
					damage, 0, DAMAGE_NO_KNOCKBACK | DAMAGE_ENERGY | DAMAGE_FIRE);
	}

	gi.linkentity(flame);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_FLAME);
	gi.WriteShort(ent - g_edicts);
	gi.WriteShort(6);
	gi.WritePosition (start);
	gi.WritePosition (flame->s.origin);
	gi.WritePosition (flame->flameinfo.pos1);
	gi.WritePosition (flame->flameinfo.pos2);
	gi.WritePosition (flame->flameinfo.pos3);
	gi.WritePosition (flame->flameinfo.pos4);
	gi.multicast (flame->s.origin, MULTICAST_PVS);
}

/*QUAKED trap_flameshooter (1 0 0) (-8 -8 -8) (8 8 8)
*/
#define FLAMESHOOTER_VELOCITY			50
#define FLAMESHOOTER_DAMAGE				20
#define FLAMESHOOTER_BURST_VELOCITY		300
#define FLAMESHOOTER_BURST_DAMAGE		30

//#define FLAMESHOOTER_PUFF	1
#define FLAMESHOOTER_STREAM	1

void flameshooter_think (edict_t *self)
{
	vec3_t	forward, right, up;
	edict_t *flame;
	
	if(self->delay)
	{
		if(self->teamchain)
			fire_remove (self->teamchain);
		return;
	}

	self->s.angles[1] += self->speed;
	if(self->s.angles[1] > 135 || self->s.angles[1] < 45)
		self->speed = -self->speed;
		 
	AngleVectors (self->s.angles, forward, right, up);

#ifdef FLAMESHOOTER_STREAM
	flame = self->teamchain;
	if(!self->teamchain)
		fire_flame (self, self->s.origin, forward, FLAMESHOOTER_DAMAGE, FLAMESHOOTER_VELOCITY);
	else
		fire_maintain (self, flame, self->s.origin, forward, FLAMESHOOTER_DAMAGE, FLAMESHOOTER_VELOCITY);

	self->think = flameshooter_think;
	self->nextthink = level.time + 0.05;
#else
	fire_burst (self, self->s.origin, forward, FLAMESHOOTER_BURST_DAMAGE, FLAMESHOOTER_BURST_VELOCITY);
	
	self->think = flameshooter_think;
	self->nextthink = level.time + 0.1;
#endif
}

void flameshooter_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if(self->delay)
	{
		self->delay = 0;
		self->think = flameshooter_think;
		self->nextthink = level.time + 0.1;
	}	
	else
		self->delay = 1;
}

void SP_trap_flameshooter(edict_t *self)
{
	vec3_t	tempAngles;

	self->solid = SOLID_NOT;
	self->movetype = MOVETYPE_NONE;

	self->delay = 0;

	self->use =	flameshooter_use;
	if(self->delay == 0)
	{
		self->think = flameshooter_think;
		self->nextthink = level.time  + 0.1;
	}

//	self->flags |= FL_NOCLIENT;

	self->speed = 10;

//	self->speed = 0;	// FIXME this stops the spraying

	VectorCopy(self->s.angles, tempAngles);

	if (!VectorCompare(self->s.angles, vec3_origin))
		G_SetMovedir (self->s.angles, self->movedir);

	VectorCopy(tempAngles, self->s.angles);

//	gi.setmodel (self, self->model);
	gi.linkentity (self);
}

// *************************
// fire_burst
// *************************

#define FLAME_BURST_MAX_SIZE	64
#define FLAME_BURST_FRAMES		20
#define FLAME_BURST_MIDPOINT	10

void fire_burst_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		powerunits;
	int		damage, radius;
	vec3_t	origin;
	
	if (surf && (surf->flags & SURF_SKY))
	{
//		gi.dprintf("Hit sky. Removed\n");
		G_FreeEdict (ent);
		return;
	}

	if(other == ent->owner || ent == other)
		return;

	// don't let flame puffs blow each other up
	if(other->classname && !strcmp(other->classname, ent->classname))
		return;

	if(ent->waterlevel)
	{
//		gi.dprintf("Hit water. Removed\n");
		G_FreeEdict(ent);		
	}

	if(!(other->svflags & SVF_MONSTER) && !other->client)
	{
		powerunits = FLAME_BURST_FRAMES - ent->s.frame;
		damage = powerunits * 6;
		radius = powerunits * 4;

//		T_RadiusDamage (inflictor, attacker, damage, ignore, radius)
		T_RadiusDamage(ent, ent->owner, damage, ent, radius, DAMAGE_FIRE);

//		gi.dprintf("Hit world: %d pts, %d rad\n", damage, radius);

		// calculate position for the explosion entity
		VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_PLAIN_EXPLOSION);
		gi.WritePosition (origin);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		G_FreeEdict (ent);
	}
}

void fire_burst_think (edict_t *self)
{
	int	current_radius;

	if(self->waterlevel)
	{
		G_FreeEdict(self);
		return;
	}

	self->s.frame++;
	if(self->s.frame >= FLAME_BURST_FRAMES)
	{
		G_FreeEdict(self);
		return;
	}

	else if(self->s.frame < FLAME_BURST_MIDPOINT)
	{
		current_radius = (FLAME_BURST_MAX_SIZE / FLAME_BURST_MIDPOINT) * self->s.frame;
	}
	else
	{
		current_radius = (FLAME_BURST_MAX_SIZE / FLAME_BURST_MIDPOINT) * (FLAME_BURST_FRAMES - self->s.frame);
	}

	if(self->s.frame == 3)
		self->s.skinnum = 1;
	else if (self->s.frame == 7)
		self->s.skinnum = 2;
	else if (self->s.frame == 10)
		self->s.skinnum = 3;
	else if (self->s.frame == 13)
		self->s.skinnum = 4;
	else if (self->s.frame == 16)
		self->s.skinnum = 5;
	else if (self->s.frame == 19)
		self->s.skinnum = 6;

	if(current_radius < 8)
		current_radius = 8;
	else if(current_radius > FLAME_BURST_MAX_SIZE)
		current_radius = FLAME_BURST_MAX_SIZE;

	T_RadiusDamage(self, self->owner, self->dmg, self, current_radius, DAMAGE_FIRE);

	self->think = fire_burst_think;
	self->nextthink = level.time + 0.1;
}

void fire_burst (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
{
	edict_t *flame;
	vec3_t	dir;
	vec3_t	baseVel;
	vec3_t	forward, right, up;

	vectoangles2 (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	flame = G_Spawn();
	VectorCopy(start, flame->s.origin);
//	VectorScale (aimdir, speed, flame->velocity);

	// scale down so only 30% of player's velocity is taken into account.
	VectorScale (self->velocity, 0.3, baseVel);
	VectorMA(baseVel, speed, aimdir, flame->velocity);

	VectorCopy (dir, flame->s.angles);
	flame->movetype = MOVETYPE_FLY;
	flame->solid = SOLID_TRIGGER;

	VectorSet(flame->mins, -FLAMETHROWER_RADIUS, -FLAMETHROWER_RADIUS, -FLAMETHROWER_RADIUS);
	VectorSet(flame->maxs, FLAMETHROWER_RADIUS, FLAMETHROWER_RADIUS, FLAMETHROWER_RADIUS);

	flame->s.sound = gi.soundindex ("weapons/flame.wav");
	flame->s.modelindex = gi.modelindex ("models/projectiles/puff/tris.md2");
	flame->owner = self;
	flame->touch = fire_burst_touch;
	flame->think = fire_burst_think;
	flame->nextthink = level.time + 0.1;
	flame->dmg = damage;
	flame->classname = "flameburst";
	flame->s.effects = EF_FIRE_PUFF;

	gi.linkentity (flame);
}
#endif

// *************************
//	INCENDIARY GRENADES
// *************************

#ifdef INCLUDE_INCENDIARY
void FireThink (edict_t *ent)
{
	if(level.time > ent->wait)
		G_FreeEdict(ent);
	else
	{
		ent->s.frame++;
		if(ent->s.frame>10)
			ent->s.frame = 0;
		ent->nextthink = level.time + 0.05;
		ent->think = FireThink;
	}
}

#define FIRE_HEIGHT		64
#define FIRE_RADIUS		64
#define FIRE_DAMAGE		3
#define FIRE_DURATION	15

edict_t *StartFire(edict_t *fireOwner, vec3_t fireOrigin, float fireDuration, float fireDamage)
{
	edict_t	*fire;

	fire = G_Spawn();
	VectorCopy (fireOrigin, fire->s.origin);
	fire->movetype = MOVETYPE_TOSS;
	fire->solid = SOLID_TRIGGER;
	VectorSet(fire->mins, -FIRE_RADIUS, -FIRE_RADIUS, 0);
	VectorSet(fire->maxs, FIRE_RADIUS, FIRE_RADIUS, FIRE_HEIGHT);

	fire->s.sound = gi.soundindex ("weapons/incend.wav");
	fire->s.modelindex = gi.modelindex ("models/objects/fire/tris.md2");

	fire->owner = fireOwner;
	fire->touch = hurt_touch;
	fire->nextthink = level.time + 0.05;
	fire->wait = level.time + fireDuration;
	fire->think = FireThink;
//	fire->nextthink = level.time + fireDuration;
//	fire->think = G_FreeEdict;
	fire->dmg = fireDamage;
	fire->classname = "incendiary_fire";
	
	gi.linkentity (fire);

//	gi.sound (fire, CHAN_VOICE, gi.soundindex ("weapons/incend.wav"), 1, ATTN_NORM, 0);
	return fire;
}

static void Incendiary_Explode (edict_t *ent)
{
	vec3_t		origin;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, DAMAGE_FIRE);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->groundentity)
		gi.WriteByte (TE_GRENADE_EXPLOSION);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	StartFire(ent->owner, ent->s.origin, FIRE_DURATION, FIRE_DAMAGE);

	G_FreeEdict (ent);

}

static void Incendiary_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!(other->svflags & SVF_MONSTER) && !(ent->client))
//	if (!other->takedamage)
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb2b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	Incendiary_Explode (ent);
}

void fire_incendiary_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles2 (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
//	if (self->client)
//		grenade->s.effects &= ~EF_TELEPORT;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/projectiles/incend/tris.md2");
	grenade->owner = self;
	grenade->touch = Incendiary_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Incendiary_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "incendiary_grenade";

	gi.linkentity (grenade);
}
#endif

// *************************
// MELEE WEAPONS
// *************************

#ifdef INCLUDE_MELEE
void fire_player_melee (edict_t *self, vec3_t start, vec3_t aim, int reach, int damage, int kick, int quiet, int mod)
{
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	trace_t		tr;

	vectoangles2 (aim, v);
	AngleVectors (v, forward, right, up);
	VectorNormalize (forward);
	VectorMA( start, reach, forward, point);

	//see if the hit connects
	tr = gi.trace(start, NULL, NULL, point, self, MASK_SHOT);
	if(tr.fraction ==  1.0)
	{
		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/swish.wav"), 1, ATTN_NORM, 0);
		//FIXME some sound here?
		return;
	}

	if(tr.ent->takedamage == DAMAGE_YES || tr.ent->takedamage == DAMAGE_AIM)
	{
		// pull the player forward if you do damage
		VectorMA(self->velocity, 75, forward, self->velocity);
		VectorMA(self->velocity, 75, up, self->velocity);

		// do the damage
		// FIXME - make the damage appear at right spot and direction
		if(mod == MOD_CHAINFIST)
			T_Damage (tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, damage, kick/2, 
						DAMAGE_DESTROY_ARMOR | DAMAGE_NO_KNOCKBACK, mod);
		else
			T_Damage (tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, mod);

		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/meatht.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		if(!quiet)
			gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/tink1.wav"), 1, ATTN_NORM, 0);

		VectorScale (tr.plane.normal, 256, point);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_GUNSHOT);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (point);
		gi.multicast (tr.endpos, MULTICAST_PVS);
	}
}
#endif



// *************************
//	BLASTER 2
// *************************

/*
=================
fire_blaster2

Fires a single green blaster bolt.  Used by monsters, generally.
=================
*/
void blaster2_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;
	int		damagestat;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner && self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		// the only time players will be firing blaster2 bolts will be from the 
		// defender sphere.
		if(self->owner->client)
			mod = MOD_DEFENDER_SPHERE;
		else
			mod = MOD_BLASTER2;

		if (self->owner)
		{
			damagestat = self->owner->takedamage;
			self->owner->takedamage = DAMAGE_NO;
			if (self->dmg >= 5)
				T_RadiusDamage(self, self->owner, self->dmg*3, other, self->dmg_radius, 0);
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
			self->owner->takedamage = damagestat;
		}
		else
		{
			if (self->dmg >= 5)
				T_RadiusDamage(self, self->owner, self->dmg*3, other, self->dmg_radius, 0);
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
		}
	}
	else
	{
		//PMM - yeowch this will get expensive
		if (self->dmg >= 5)
			T_RadiusDamage(self, self->owner, self->dmg*3, self->owner, self->dmg_radius, 0);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER2);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (self);
}

void fire_blaster2 (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles2 (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	
		if (effect)
			bolt->s.effects |= EF_TRACKER;
		bolt->dmg_radius = 128;
		bolt->s.modelindex = gi.modelindex ("models/proj/laser2/tris.md2");
		bolt->touch = blaster2_touch;

	bolt->owner = self;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";
 	gi.linkentity (bolt);

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	

// *************************
// tracker
// *************************

/*
void tracker_boom_think (edict_t *self)
{
	self->s.frame--;
	if(self->s.frame < 0)
		G_FreeEdict(self);
	else
		self->nextthink = level.time + 0.1;
}

void tracker_boom_spawn (vec3_t origin)
{
	edict_t *boom;

	boom = G_Spawn();
	VectorCopy (origin, boom->s.origin);
	boom->s.modelindex = gi.modelindex ("models/items/spawngro/tris.md2");
	boom->s.skinnum = 1;
	boom->s.frame = 2;
	boom->classname = "tracker boom";
	gi.linkentity (boom);

	boom->think = tracker_boom_think;
	boom->nextthink = level.time + 0.1;
	//PMM
//	boom->s.renderfx |= RF_TRANSLUCENT;
	boom->s.effects |= EF_SPHERETRANS;
	//pmm
}
*/

#define TRACKER_DAMAGE_FLAGS	(DAMAGE_NO_POWER_ARMOR | DAMAGE_ENERGY | DAMAGE_NO_KNOCKBACK)
#define TRACKER_IMPACT_FLAGS	(DAMAGE_NO_POWER_ARMOR | DAMAGE_ENERGY)

#define TRACKER_DAMAGE_TIME		0.5		// seconds

void tracker_pain_daemon_think (edict_t *self)
{
	static vec3_t	pain_normal = { 0, 0, 1 };
	int				hurt;

	if(!self->inuse)
		return;

	if((level.time - self->timestamp) > TRACKER_DAMAGE_TIME)
	{
		if(!self->enemy->client)
			self->enemy->s.effects &= ~EF_TRACKERTRAIL;
		G_FreeEdict (self);
	}
	else
	{
		if(self->enemy->health > 0)
		{
//			gi.dprintf("ouch %x\n", self);
			T_Damage (self->enemy, self, self->owner, vec3_origin, self->enemy->s.origin, pain_normal,
						self->dmg, 0, TRACKER_DAMAGE_FLAGS, MOD_TRACKER);
			
			// if we kill the player, we'll be removed.
			if(self->inuse)
			{
				// if we killed a monster, gib them.
				if (self->enemy->health < 1)
				{
					if(self->enemy->gib_health)
						hurt = - self->enemy->gib_health;
					else
						hurt = 500;

//					gi.dprintf("non-player killed. ensuring gib!  %d\n", hurt);
					T_Damage (self->enemy, self, self->owner, vec3_origin, self->enemy->s.origin,
								pain_normal, hurt, 0, TRACKER_DAMAGE_FLAGS, MOD_TRACKER);
				}

				/* GRIM if(self->enemy->client)
					self->enemy->client->tracker_pain_framenum = level.framenum + 1;
				else*/ // GRIM
					self->enemy->s.effects |= EF_TRACKERTRAIL;
				
				self->nextthink = level.time + FRAMETIME;
			}
		}
		else
		{
			if(!self->enemy->client)
				self->enemy->s.effects &= ~EF_TRACKERTRAIL;
			G_FreeEdict (self);
		}
	}
}

void tracker_pain_daemon_spawn (edict_t *owner, edict_t *enemy, int damage)
{
	edict_t	 *daemon;

	if(enemy == NULL)
		return;

	daemon = G_Spawn();
	daemon->classname = "pain daemon";
	daemon->think = tracker_pain_daemon_think;
	daemon->nextthink = level.time + FRAMETIME;
	daemon->timestamp = level.time;
	daemon->owner = owner;
	daemon->enemy = enemy;
	daemon->dmg = damage;
}

void tracker_explode (edict_t *self, cplane_t *plane)
{
	vec3_t	dir;

	if(!plane)
		VectorClear (dir);
	else
		VectorScale (plane->normal, 256, dir);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TRACKER_EXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

//	gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/disrupthit.wav"), 1, ATTN_NORM, 0);
//	tracker_boom_spawn(self->s.origin);

	G_FreeEdict (self);
}

void tracker_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float	damagetime;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		if((other->svflags & SVF_MONSTER) || other->client)
		{
			if(other->health > 0)		// knockback only for living creatures
			{
				// PMM - kickback was times 4 .. reduced to 3
				// now this does no damage, just knockback
				T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal,
							/* self->dmg */ 0, (self->dmg*3), TRACKER_IMPACT_FLAGS, MOD_TRACKER);
				
				if (!(other->flags & (FL_FLY|FL_SWIM)))
					other->velocity[2] += 140;
				
				damagetime = ((float)self->dmg)*FRAMETIME;
				damagetime = damagetime / TRACKER_DAMAGE_TIME;
//				gi.dprintf ("damage is %f\n", damagetime);

				tracker_pain_daemon_spawn (self->owner, other, (int)damagetime);
			}
			else						// lots of damage (almost autogib) for dead bodies
			{
				T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal,
							self->dmg*4, (self->dmg*3), TRACKER_IMPACT_FLAGS, MOD_TRACKER);
			}
		}
		else	// full damage in one shot for inanimate objects
		{
			T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal,
						self->dmg, (self->dmg*3), TRACKER_IMPACT_FLAGS, MOD_TRACKER);
		}
	}

	tracker_explode (self, plane);
	return;
}

void tracker_fly (edict_t *self)
{
	vec3_t	dest;
	vec3_t	dir;
	vec3_t	center;

	if ((!self->enemy) || (!self->enemy->inuse) || (self->enemy->health < 1))
	{
		tracker_explode (self, NULL);
		return;
	}
/*
	VectorCopy (self->enemy->s.origin, dest);
	if(self->enemy->client)
		dest[2] += self->enemy->viewheight;
*/
	// PMM - try to hunt for center of enemy, if possible and not client
	if(self->enemy->client)
	{
		VectorCopy (self->enemy->s.origin, dest);
		dest[2] += self->enemy->viewheight;
	}
	// paranoia
	else if (VectorCompare(self->enemy->absmin, vec3_origin) || VectorCompare(self->enemy->absmax, vec3_origin))
	{
		VectorCopy (self->enemy->s.origin, dest);
	}
	else
	{
		VectorMA (vec3_origin, 0.5, self->enemy->absmin, center);
		VectorMA (center, 0.5, self->enemy->absmax, center);
		VectorCopy (center, dest);
	}

	VectorSubtract (dest, self->s.origin, dir);
	VectorNormalize (dir);
	vectoangles2 (dir, self->s.angles);
	VectorScale (dir, self->speed, self->velocity);
	VectorCopy(dest, self->monsterinfo.saved_goal);

	self->nextthink = level.time + 0.1;
}

void fire_tracker (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, edict_t *enemy)
{
	edict_t	*bolt;
	trace_t	tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles2 (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->speed = speed;
	bolt->s.effects = EF_TRACKER;
	bolt->s.sound = gi.soundindex ("weapons/disrupt.wav");
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	
	bolt->s.modelindex = gi.modelindex ("models/proj/disintegrator/tris.md2");
	bolt->touch = tracker_touch;
	bolt->enemy = enemy;
	bolt->owner = self;
	bolt->dmg = damage;
	bolt->classname = "tracker";
	gi.linkentity (bolt);

	if(enemy)
	{
		bolt->nextthink = level.time + 0.1;
		bolt->think = tracker_fly;
	}
	else
	{
		bolt->nextthink = level.time + 10;
		bolt->think = G_FreeEdict;
	}

	if (self->client)
		check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	}
}	
