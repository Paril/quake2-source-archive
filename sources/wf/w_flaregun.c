#include "g_local.h"
void stick(edict_t *projectile, edict_t *object); 
/*Shelton: Enemy disguised spies are purposely not safe from fire damage.  Flare ammo cost 
not changed, but should be, also, flare limit not invoked, but should be discussed (Im thinking
3 or 4). Flares last roughly 5 seconds from fire time to burnout. */

/*
================
Flare Gun
================
*/

void flare2_explode (edict_t *ent)
{
	G_FreeEdict (ent); 
	//Shelton: remove flare entity, no explosion, just burn out.
}

void flaregun2_sticky_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
    //Shelton: Sticky code taken from Snipers Flare.
	
	if (other == ent->owner) 
		return;

	if (other->wf_team == ent->wf_team)
	    return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}
	//Shelton: Flare set to bounce off of owner & team.  Remove if hits sky.

	stick(ent, other); 
	//Shelton: otherwise stick it.
	
	ent->s.effects |= EF_TELEPORTER;
			
	if (other->takedamage)
    {
		burn_person(other, ent->owner, 10, MOD_FLAREGUN);
		ent->think = flare2_explode;
	}
	/*Shelton: Direct hit on enemy, light em up.  Flare continues to be stuck in victim
	until nextthink occurs.  During that time, other enemies may be burned too */
	
	if (!other->takedamage)
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
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}
	//Shelton: stick to wall, play sound.. etc.
}

static void mod_Flare2Think(edict_t *ent)
{
    edict_t *blip = NULL;

	if(ent->delay < level.time)
	{
		ent->think = flare2_explode;
	}
	//Shelton: flare timeout occurs, destroy flare.
     while ((blip = findradius(blip, ent->s.origin, 100)) != NULL)
        {
        if (!(blip->svflags & SVF_MONSTER) && !blip->client)
            continue;
        if (!visible(ent, blip))
			continue; 
		if (blip->health <= 0)
			continue;
		if (!blip->takedamage)
            continue;
		if (blip->wf_team != ent->wf_team)
            burn_person(blip, ent->owner, 3, MOD_FLAREGUN);
		    continue;
		if (blip->wf_team == ent->wf_team)
		    continue;
		break;
        }
	 //Shelton: blip for enemies to burn in radius, ignore team.  Burn spies.
     
     ent->nextthink = level.time + .1;
}
  
void fire_burningflare2(edict_t *self, vec3_t start, vec3_t dir, vec3_t los, int damage, int speed, 
float damage_radius, int radius_damage)
{
	edict_t *rocket;

	rocket = G_Spawn();

	VectorCopy (los, rocket->pos1); /*Added: Save the line of sight of the center rocket. This is
	the axis around which the other two rockets rotate*/
	VectorCopy (start, rocket->pos2); /*Added: Save the start position of the rocket (Not sure if
	this is necessary. This info might already be somewhere else)?*/

	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_TOSS;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_ROCKET;   
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/flarep/tris.md2");
	rocket->owner = self;
	rocket->touch = flaregun2_sticky_touch;
	rocket->gravity = .4;
	rocket->nextthink = level.time + 3;
	rocket->think = mod_Flare2Think;
	rocket->dmg = 0;
	rocket->delay = level.time + 5;
	rocket->radius_dmg = 0;
	rocket->dmg_radius = 0;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->wf_team = self->wf_team;
	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

/*Shelton: rocket settings slighly altered.  Old flare firing method changed to straight shot.  
However, it's current trajectory is a bit off, and I don't know how to fix that */

void Weapon_FlareGun2_Fire (edict_t *ent)
{
	vec3_t offset, start;
	vec3_t forward, right;
	vec3_t traj_angle, lineofsight; /*Added: Trajectory angle of rocket and line of sight for the center rocket*/
	int startspeed; /*Added: Start speed of the rockets*/

	int damage;
	float damage_radius;
	int radius_damage;

	startspeed = wf_game.weapon_speed[WEAPON_FLAREGUN]; 

	damage = 0;
	radius_damage = 0;
	damage_radius = 0;

	//Shelton: removed direct damage.  All damage comes from burn_person().
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, 0, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	/*Added: Start of modified launcher code*/
	VectorCopy(forward, lineofsight); /*our line of sight is the same as our forward view vector*/
	VectorCopy(ent->client->v_angle, traj_angle); /*get the trajectory angles for later modification*/
	/*This is the original launch code*/
	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	/*Added: Now, modify the trajectory of the second rocket to 5 degrees off of the center axis*/
	traj_angle[1] = ent->client->v_angle[1]+5;
	AngleVectors (traj_angle, forward, right, NULL);

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_burningflare2 (ent, start, forward, lineofsight, damage, startspeed, damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
			// ### Hentai ### BEGIN
/*
			ent->client->anim_priority = ANIM_ATTACK;
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crattak1 - 1;
				ent->client->anim_end = FRAME_crattak9;
			}
			else
			{
				ent->s.frame = FRAME_attack1 - 1;
				ent->client->anim_end = FRAME_attack8;
			}
	
			// ### Hentai ### END
*/
	PlayerNoise(ent, start, PNOISE_WEAPON);

	//Reduce ammo
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 6;
}
void Weapon_FlareGun2 (edict_t *ent)
{
	static int	pause_frames[]	= {9,32, 0};
	static int	fire_frames[]	= {9, 0};

	Weapon_Generic (ent,5, 9, 31, 36, pause_frames, fire_frames, Weapon_FlareGun2_Fire);
}