/*==============================================================================
The Weapons Factory - 
Laserball Functions
Original code by Gregg Reno
==============================================================================*/
#include <assert.h>
#include "g_local.h"
#include "q_devels.h"
extern void target_laser_start (edict_t *self);

/*
=================
laserball_explode
=================
*/

void laserball_cleanup (edict_t *ent)
{
    edict_t *lent;
	edict_t	*next;


	// clean up laser entities
    for (lent = ent->teamchain; lent; lent = next)
	{
         next = lent->teamchain;
         G_FreeEdict (lent);
	}
}

void laserball_explode (edict_t *ent)
{
	vec3_t		origin;

	laserball_cleanup(ent);

	if (ent->owner->client)
	{
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);
	}

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	T_RadiusDamage(ent, ent->owner, ent->dmg, NULL, ent->dmg_radius, MOD_WF_LASERBALL);

	//Do the explosion
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	G_FreeEdict (ent);
}

/*
=================
laserball_think
=================
*/
void laserball_think (edict_t *ent)
     {
     edict_t *target = NULL;
     edict_t *blip = NULL;
     edict_t *lent = NULL;
     edict_t *next = NULL;
     edict_t *beam = NULL;
     edict_t *check;
     vec3_t  targetdir, blipdir;
     int found;

     float   points;
     vec3_t  v;
     float   dist;

	 //Exlode the laserball if time has run out
     if (level.time > ent->delay)	//don't exploce on contact
        {
		laserball_explode(ent);
        return;
        }

	 lent = ent->teamchain;
     //ent->teamchain = NULL;

     while ((blip = findradius(blip, ent->s.origin, 350)) != NULL)
         {
         if (!(blip->svflags & SVF_MONSTER) && !blip->client)
             continue;
         if (blip == ent->owner)
             continue;
		 //dont attack same team unless friendly fire is allowed
		 if ((blip->wf_team == ent->wf_team) && (((int)wfflags->value & WF_ALLOW_FRIENDLY_FIRE)==0))
			continue;
		 if (blip->disguised)
			continue;
         if (!blip->takedamage)
             continue;
         if (blip->health <= 0)
             continue;
         if (!visible(ent, blip))
             continue;
//         if (!infront(ent, blip))
//             continue;
         VectorSubtract(blip->s.origin, ent->s.origin, blipdir);
         blipdir[2] += 16;
         if ((target == NULL) || (VectorLength(blipdir) < VectorLength(targetdir)))
             {
             target = blip;
             VectorCopy(blipdir, targetdir);
             }
         }
                    
     if (target != NULL)
         {
         // see if this is already on the list
         found = false;
         for (check = ent->teamchain; check; check = check->teamchain)
             if (check->enemy == target) found = true;
         if (found == false) {
           //Make the laser entity

//safe_cprintf (owner, PRINT_HIGH, "LB enemy. MyTeam=%d, Enemyteam=%d\n", self->s.modelindex, self->wf_team, target->wf_team );

           beam = G_Spawn();
           beam->flags |= FL_TEAMSLAVE;
           beam->teamchain = ent->teamchain;
           beam->teammaster = ent;
           ent->teamchain = beam;
           beam->owner = ent->owner;

			//Set the laser color based  on team
			if (ent->wf_team == CTF_TEAM1)	//team 1 is red
			{
				beam->spawnflags = 1 | 2;
			}
			else
			{
				beam->spawnflags = 1 | 8;	//team 2 is blue
			}

           beam->enemy = target;
           beam->activator = ent->owner;
           beam->dmg = wf_game.grenade_damage[GRENADE_TYPE_LASERBALL];
           beam->classname = "laserball laser";
           target_laser_start (beam);
           beam->movetype = MOVETYPE_FLYMISSILE;
           gi.linkentity (beam);

           //Green flash the enemy
           VectorAdd (target->mins, target->maxs, v);
           VectorMA (target->s.origin, 0.5, v, v);
           VectorSubtract (ent->s.origin, v, v);
           dist = VectorLength(v);
           points = ent->radius_dmg * (1.0 - sqrt(dist/ent->dmg_radius));
           if (target == ent->owner)
              points = points * 0.5;

           gi.WriteByte (svc_temp_entity);
           gi.WriteByte (TE_BFG_EXPLOSION);
           gi.WritePosition (target->s.origin);
           gi.multicast (target->s.origin, MULTICAST_PVS);
           //T_Damage (target, ent, ent->owner, ent->velocity, target->s.origin, vec3_origin, (int)points, 0, DAMAGE_ENERGY);
           }
         }
     
     ent->nextthink = level.time + .1;
	 ent->think = laserball_think;
 }

/*
=================
laserball_touch
=================
*/
void laserball_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{

	if (other == ent->owner)
		return;

	//gi.dprintf("TOUCH\n");

	if (surf && (surf->flags & SURF_SKY))
	{
		laserball_explode (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//laserball_explode (ent);

}

/*
=================
fire_laserball
=================
*/
//void fire_laserball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
void fire_laserball (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
    edict_t *laserball;

	vec3_t	dir;
	vec3_t	forward, right, up;

	++self->client->pers.active_grenades[GRENADE_TYPE_LASERBALL];
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= LASERBALL_CELLS;

	//Increase the speed on this !
	speed = speed * 1.5;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

    laserball = G_Spawn();
	laserball->grenade_index = GRENADE_TYPE_LASERBALL;

	VectorCopy (start, laserball->s.origin);
	VectorScale (aimdir, speed, laserball->velocity);
	VectorMA (laserball->velocity, 200 + crandom() * 10.0, up, laserball->velocity);
	VectorMA (laserball->velocity, crandom() * 10.0, right, laserball->velocity);
	VectorSet (laserball->avelocity, 300, 300, 300);
	laserball->movetype = MOVETYPE_BOUNCE;
//		laserball->movetype = MOVETYPE_FLOAT;
	laserball->clipmask = MASK_SHOT;
	laserball->solid = SOLID_BBOX;
	laserball->s.effects |= EF_GRENADE;
	VectorClear (laserball->mins);
	VectorClear (laserball->maxs);

	//Try to raise up the laserball a bit
	VectorSet(laserball->mins, -7,-7,-7);
	VectorSet(laserball->maxs, 7, 7, 7);

	laserball->s.modelindex = gi.modelindex (GRLASERBALL_MODEL);
	laserball->s.skinnum = GRLASERBALL_SKIN;
//      laserball->s.modelindex = gi.modelindex ("models/objects/laserball/tris.md2");
	laserball->owner = self;
	laserball->touch = laserball_touch;
	laserball->teammaster = laserball;
	laserball->teamchain = NULL;

	laserball->nextthink = level.time + .1;
	laserball->think = laserball_think;
	laserball->delay = level.time + 3.0;

	//set the team
	laserball->wf_team = self->client->resp.ctf_team;

	laserball->dmg = damage;
	laserball->radius_dmg = damage_radius;
	laserball->dmg_radius = damage_radius;
	laserball->classname = "laserball";

	gi.linkentity (laserball);
}


