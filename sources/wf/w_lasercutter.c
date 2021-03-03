//The grenade turrets hit the ceiling and stick out halfway.
//this makes it so they drop down a bit, eliminating this.
#include "g_local.h"
void grenlaser_think2 (edict_t *ent);
void laser_Explode (edict_t *ent);

void grenlaser_think4 (edict_t *ent)
{
	vec3_t down;
	int speed;

	down[0]=0;		//We're going DOWN!!!
	down[1]=0;
	down[2]=-100;

	VectorNormalize(down);
	VectorCopy(down, ent->movedir);
	speed=75;
	VectorScale(down, speed, ent->velocity);

	ent->nextthink=level.time + .3;
	ent->think=grenlaser_think2;
}

void laser_Explode (edict_t *ent)
{
	vec3_t		origin;

	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, MOD_LASERCUTTER);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

// Second think function for da grenade turrets
//Initially this was a homing think function from qdevels www.planetquake.com/qdevels
//Imp was here (duh)
//MUST go before grenturret_think1 so that think1 can set ent->think to grenturret_think2

//Note 3/22:  Putting think2 before 1 isn't necessary anymore, since I
//prototyped it in g_local.h, but what the hell...

void grenlaser_think2 (edict_t *ent)
{
	edict_t *target = NULL;
	edict_t *blip = NULL;
//	vec3_t start;
//	vec3_t point;
//	vec3_t dir;
	trace_t tr;
	vec3_t end,right,forward;
	AngleVectors (ent->s.angles, forward, right, NULL);
	VectorScale(ent->movedir, 12, ent->velocity);  //Keep speed at 25

	if (((int) (ent->turrettime * 10)) % 10==0)
		ent->movedir[2]*=-1;
	end[0]=right[0]*8000;
	end[1]=right[1]*8000;
	end[2]=right[2]*8000;
	ent->s.angles[1]+=5;
	if (ent->s.angles[1]>360)
		ent->s.angles[1]-=360;
	tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
    //Sparks
         if ((tr.ent != ent->owner) && (tr.ent->takedamage))
              T_Damage (tr.ent, ent, ent->owner, forward, tr.endpos, tr.plane.normal,
     wf_game.grenade_damage[GRENADE_TYPE_LASERCUTTER], 0, 0, MOD_LASERCUTTER);
           else if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
           {  
			   // hit a brush, send clients 
              // a light flash and sparks temp entity.
              gi.WriteByte (svc_temp_entity);
              gi.WriteByte (TE_BLASTER);
              gi.WritePosition (tr.endpos);
              gi.WriteDir (tr.plane.normal);
              gi.multicast (tr.endpos, MULTICAST_PVS);
           }
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (ent->s.origin);
	gi.WritePosition (tr.endpos);
	gi.multicast (ent->s.origin, MULTICAST_PHS);
	end[0]=right[0]*-8000;
	end[1]=right[1]*-8000;
	end[2]=right[2]*-8000;
		tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

	//Laser sparks
         if ((tr.ent != ent->owner) && (tr.ent->takedamage))
              T_Damage (tr.ent, ent, ent->owner, forward, tr.endpos, tr.plane.normal,
     5, 0, 0, MOD_LASERCUTTER);
    else if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
    {  // hit a brush, send clients 
              // a light flash and sparks temp entity.
         gi.WriteByte (svc_temp_entity);
         gi.WriteByte (TE_BLASTER);
         gi.WritePosition (tr.endpos);
         gi.WriteDir (tr.plane.normal);
         gi.multicast (tr.endpos, MULTICAST_PVS);
    }
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (ent->s.origin);
	gi.WritePosition (tr.endpos);
	gi.multicast (ent->s.origin, MULTICAST_PHS);
	end[0]=forward[0]*8000;
	end[1]=forward[1]*8000;
	end[2]=forward[2]*8000;
		tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

	//Laser sparks
	if ((tr.ent != ent->owner) && (tr.ent->takedamage))
              T_Damage (tr.ent, ent, ent->owner, forward, tr.endpos, tr.plane.normal, 5, 0, 0, MOD_LASERCUTTER);
	else if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{  // hit a brush, send clients 
		// a light flash and sparks temp entity.
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BLASTER);
		gi.WritePosition (tr.endpos);
		gi.WriteDir (tr.plane.normal);
		gi.multicast (tr.endpos, MULTICAST_PVS);
	}
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (ent->s.origin);
	gi.WritePosition (tr.endpos);
	gi.multicast (ent->s.origin, MULTICAST_PHS);
		end[0]=forward[0]*-8000;
	end[1]=forward[1]*-8000;
	end[2]=forward[2]*-8000;
		tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

	//Laser sparks
         if ((tr.ent != ent->owner) && (tr.ent->takedamage))
              T_Damage (tr.ent, ent, ent->owner, forward, tr.endpos, tr.plane.normal,
     5, 0, 0, MOD_LASERCUTTER);
           else if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
           {  // hit a brush, send clients 
              // a light flash and sparks temp entity.
              gi.WriteByte (svc_temp_entity);
              gi.WriteByte (TE_BLASTER);
              gi.WritePosition (tr.endpos);
              gi.WriteDir (tr.plane.normal);
              gi.multicast (tr.endpos, MULTICAST_PVS);
           }
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (ent->s.origin);
	gi.WritePosition (tr.endpos);
	gi.multicast (ent->s.origin, MULTICAST_PHS);
//If our turret is out of ammo or has been too long, kill it
if (level.time>=ent->turretdie)
	{
	ent->think=laser_Explode;	//Drops to the ground and explodes
	ent->nextthink=level.time+2;	//Looks better than just disappearing
	ent->movetype=MOVETYPE_BOUNCE;

	//FIXME:  This is ugly... is it possible to subtract effects
	//instead of totally redefining the effects?
	ent->s.effects = EF_GRENADE;  //Lights out!!!
	ent->s.renderfx = 0;	      //Goodbye shell :(
	}
ent->turrettime+=.1;
	ent->nextthink=level.time +0.1;
}

void grenlaser_think1 (edict_t *ent)
{
	vec3_t up;
	vec3_t right;
	int speed;

	//Take out gravity
	ent->movetype=MOVETYPE_FLYMISSILE;

	up[0]=0;		//We're going UP!!!
	up[1]=0;
	up[2]=5;//Waist height

	right[0]=100;		//we're pointing right... ugly hack
	right[1]=0;
	right[2]=0;

	ent->s.effects |= EF_HYPERBLASTER;	//Lots of fun with green lights
	ent->s.effects |= EF_COLOR_SHELL; 	//Green shell... fun!

	//	ent->s.renderfx |= RF_SHELL_GREEN;	//It's a GREEN shell!!!

	if (ent->wf_team == CTF_TEAM1)	//team 1 is red
		ent->s.renderfx |= RF_SHELL_RED;
	else
		ent->s.renderfx |= RF_SHELL_BLUE;

	VectorNormalize(up);
	VectorCopy(up, ent->movedir);
	speed=20;
	VectorScale(up, speed, ent->velocity);

	ent->avelocity[0]=0;
	ent->avelocity[1]=360*5;
	ent->avelocity[2]=0;

	vectoangles(right, ent->s.angles);

	ent->nextthink=level.time+.5;
	ent->think=grenlaser_think4;
	ent->turrettime=1;
}

static void laser_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		laser_Explode (ent);
		return;
	}

	if (!other->takedamage)
	{
		if (ent->think==grenlaser_think4)  //Move the grenade away from
			ent->nextthink=level.time;  //the ceiling when it hits it

		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	//ent->enemy = other;
	//laser_Explode (ent);
}
void Laser_Grenade_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
	self->think = laser_Explode;
}

void fire_laser_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		if ( self->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] >= TURRET_GRENADES
			&& self->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))] >= TURRET_SLUGS)
		{
			self->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] -= TURRET_GRENADES;
			self->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))] -= TURRET_SLUGS;
		}
		else
		{
			safe_cprintf(self, PRINT_HIGH, "You need %d Grenades and %d Slugs for Laser Cutter\n",TURRET_GRENADES,TURRET_SLUGS);
			return;
		}
	}

	++self->client->pers.active_grenades[GRENADE_TYPE_LASERCUTTER];

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	grenade->grenade_index = GRENADE_TYPE_LASERCUTTER;
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
//	grenade->clipmask = MASK_SHOT;
	grenade->clipmask = MASK_PLAYERSOLID;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex (GRLASERCUTTER_MODEL);
	grenade->s.skinnum = GRLASERCUTTER_SKIN;
	grenade->owner = self;
//	grenade->touch = laser_Touch;
	grenade->wf_team = self->client->resp.ctf_team;

	// A few more attributes to let the grenade 'die'
    VectorSet(grenade->mins, -10, -10, 0);
    VectorSet(grenade->maxs, 10, 10, 10);
    grenade->mass = 40;
	grenade->health = 10;
	grenade->die = Laser_Grenade_Die;
	grenade->takedamage = DAMAGE_YES;
    grenade->monsterinfo.aiflags = AI_NOSTEP;

	grenade->nextthink = level.time + timer;
	grenade->think = grenlaser_think1;
	grenade->turrettime=0;
	grenade->turretdie=level.time+4.10 + timer;
	grenade->turretammo=6;

	grenade->dmg = DAMAGE_LASERCUTTER;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "lasercutter";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;

	//set team
	grenade->wf_team = self->client->resp.ctf_team;

	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	//if (timer <= 0.0)
		//laser_Explode (grenade);
	//else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}


