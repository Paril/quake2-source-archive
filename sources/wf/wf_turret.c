/*
Grenade Turrets
by xxxx xxx
Modified by Gregg Reno
*/

#include "g_local.h"

void Turret_Explode (edict_t *ent);

//The grenade turrets hit the ceiling and stick out halfway.
//this makes it so they drop down a bit, eliminating this.
void grenturret_think4 (edict_t *ent)
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
	ent->think=grenturret_think2;
}

void grenturret_think3 (edict_t *ent)
{
	if (((int) (ent->turrettime * 10)) % 10==0)
		ent->movedir[2]*=-1;

	//Finally, KILL KILL KILL!!!
	if (ent->show_hostile==false)
	{
		ent->show_hostile=true;
		ent->nextthink=level.time + .1;
		ent->think=grenturret_think3;
	}
	else
	{
		fire_rail (ent, ent->s.origin, ent->targetdir, wf_game.grenade_damage[GRENADE_TYPE_TURRET], 250, MOD_WF_TURRET);
		ent->nextthink=level.time + .1;
		ent->think=grenturret_think2;
		ent->turrettime=0;
		ent->turretammo-=1;
	}
		ent->turrettime+=.1;
}

// Second think function for da grenade turrets
//Initially this was a homing think function from qdevels www.planetquake.com/qdevels
//Imp was here (duh)
//MUST go before grenturret_think1 so that think1 can set ent->think to grenturret_think2

//Note 3/22:  Putting think2 before 1 isn't necessary anymore, since I
//prototyped it in g_local.h, but what the hell...

void grenturret_think2 (edict_t *ent)

{
	edict_t *target = NULL;
	edict_t *blip = NULL;
	vec3_t  targetdir, blipdir;
//	vec3_t *start;
//	vec3_t point;
//	vec3_t dir;

	VectorScale(ent->movedir, 12, ent->velocity);  //Keep speed at 25

	if (((int) (ent->turrettime * 10)) % 10==0)
		ent->movedir[2]*=-1;

	if (!(ent->turrettime < 2.0))
	{
		//Find targets
		while ((blip = findradius(blip, ent->s.origin, 1024)) != NULL)
		{
			if (!(blip->svflags & SVF_MONSTER) && !blip->client)
			       continue;
			if (blip->solid == SOLID_NOT)
				continue;	//don't see observers
	 	    //dont attack same team
			if (blip->wf_team == ent->wf_team)
			    continue;

			if (!blip->takedamage)
				continue;
			if (blip->health <= 0)
				continue;
			if (blip->disguised)
				continue;

			if (!visible(ent, blip))
				continue;
			VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
			blipdir[2] += 20; //don't shoot at the feet
			if ((target == NULL) || (VectorLength(blipdir) < VectorLength(targetdir)))
			{
				target = blip;
				VectorCopy(blipdir, targetdir);
				VectorCopy(targetdir, ent->targetdir);
			}
		}
	}

	ent->nextthink = level.time + .1;

	if (target!=NULL)
	{
		// target acquired, shoot it DOWN!!!!
		// Well, not quite.  Call think3 to shoot .2 seconds later...
        ent->enemy = target;
		ent->think=grenturret_think3;
		ent->show_hostile=false;  //
	}

//If our turret is out of ammo or has been too long, kill it
if (level.time>=ent->turretdie || ent->turretammo<=0)
	{
	ent->think=Turret_Explode;	//Drops to the ground and explodes
	ent->nextthink=level.time+2;	//Looks better than just disappearing
	ent->movetype=MOVETYPE_BOUNCE;

	//FIXME:  This is ugly... is it possible to subtract effects
	//instead of totally redefining the effects?
	ent->s.effects = EF_GRENADE;  //Lights out!!!
	ent->s.renderfx = 0;	      //Goodbye shell :(
	}
ent->turrettime+=.1;
}

//First new think function for grenade turrets
//Imp was here
void grenturret_think1 (edict_t *ent)
{
	vec3_t up;
	vec3_t right;
	int speed;

	//Take out gravity
	ent->movetype=MOVETYPE_FLYMISSILE;

	up[0]=0;		//We're going UP!!!
	up[1]=0;
	up[2]=100;

	right[0]=100;		//we're pointing right... ugly hack
	right[1]=0;
	right[2]=0;

	ent->s.effects |= EF_HYPERBLASTER;	//Lots of fun with green lights
/* For now, turn off special effects.  Use skins instead
	ent->s.effects |= EF_COLOR_SHELL; 	//Green shell... fun!

	//	ent->s.renderfx |= RF_SHELL_GREEN;	//It's a GREEN shell!!!

	if (ent->wf_team == CTF_TEAM1)	//team 1 is red
		ent->s.renderfx |= RF_SHELL_RED;
	else
		ent->s.renderfx |= RF_SHELL_BLUE;
*/

	VectorNormalize(up);
	VectorCopy(up, ent->movedir);
	speed=300;
	VectorScale(up, speed, ent->velocity);

	ent->avelocity[0]=0;
//	ent->avelocity[1]=360*5;
	ent->avelocity[1]=360;
	ent->avelocity[2]=0;

	vectoangles(right, ent->s.angles);

	ent->nextthink=level.time+.5;
	ent->think=grenturret_think4;
	ent->turrettime=1;
}

void Turret_Explode (edict_t *ent)
{
	vec3_t		origin;

	if (ent->owner->client)
	{
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	}

	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, MOD_WF_TURRET);

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

static void Turret_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		Turret_Explode (ent);
		return;
	}

	if (!other->takedamage)
	{
//WF
		if (ent->think==grenturret_think4)  //Move the grenade away from
			ent->nextthink=level.time;  //the ceiling when it hits it
//WF


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

	ent->enemy = other;
	Turret_Explode (ent);
}

// When a grenade 'dies', it blows up next frame
void Turret_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + .1;
	self->think = Turret_Explode;
}


void fire_turret_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;


if (wfdebug)
{
gi.dprintf("fire_turret_grenade start.\n");
}
	++self->client->pers.active_grenades[GRENADE_TYPE_TURRET];

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	grenade->grenade_index = GRENADE_TYPE_TURRET;
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
//	grenade->clipmask = MASK_PLAYERSOLID;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);

	//Try to raise up the object a bit
	VectorSet(grenade->mins, -20,-20,-20);
	VectorSet(grenade->maxs, 20, 20, 20);

	grenade->s.modelindex = gi.modelindex ("models/objects/turretg/tris.md2");
	if (self->wf_team == CTF_TEAM1)	//team 1 is red
		grenade->s.skinnum = 0;
	else
		grenade->s.skinnum = 1;

//	grenade->s.modelindex = gi.modelindex (GRTURRET_MODEL);
//	grenade->s.skinnum = GRTURRET_SKIN;

	grenade->owner = self;
	grenade->touch = Turret_Touch;

	if ((int)wfflags->value & WF_ANARCHY)
		grenade->wf_team = 0;	//fire at anybody
	else
		grenade->wf_team = self->client->resp.ctf_team;

	// A few more attributes to let the grenade 'die'
    VectorSet(grenade->mins, -10, -10, 0);
    VectorSet(grenade->maxs, 10, 10, 10);
    grenade->mass = 40;
    grenade->health = 10;
    grenade->gib_health = -10;
    grenade->max_health = 10;
    grenade->die = Turret_Die;
    grenade->takedamage = DAMAGE_AIM;
    grenade->monsterinfo.aiflags = AI_NOSTEP;

	if ((int)wfflags->value & WF_NO_TURRET) 
	{
		safe_cprintf(self, PRINT_HIGH, "Turret Grenades Are Disabled\n");
		grenade->nextthink = level.time + 2;
		grenade->think = Turret_Explode;
	}
	else if ( self->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] >= TURRET_GRENADES
		&& self->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))] >= TURRET_SLUGS)
	{
		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		{
			self->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] -= TURRET_GRENADES;
			self->client->pers.inventory[ITEM_INDEX(FindItem("Slugs"))] -= TURRET_SLUGS;
		}
		grenade->nextthink = level.time + timer;
		grenade->think = grenturret_think1;
		grenade->turrettime=0;
//		grenade->turretdie=level.time+30.55 + timer;
		grenade->turretdie=level.time+61.10 + timer;
		grenade->turretammo=6;

	}
	else
	{
		safe_cprintf(self, PRINT_HIGH, "You need %d Grenades and %d Slugs for Grenade Turret\n",TURRET_GRENADES,TURRET_SLUGS);
		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			self->client->pers.inventory[ITEM_INDEX(FindItem("Grenades"))] -= 1;
		grenade->nextthink = level.time + 2;
		grenade->think = Turret_Explode;
	}

	grenade->dmg = wf_game.grenade_damage[GRENADE_TYPE_TURRET];
	grenade->dmg_radius = damage_radius;
	grenade->classname = "turret";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;

	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		Turret_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}


