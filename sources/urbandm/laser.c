#include "g_local.h"

#include	"laser.h"

extern qboolean is_quad;


void TripBomb_Explode (edict_t *self)
{
	edict_t *trip_grenade = NULL;
// bit of damage
	T_RadiusDamage (self, self, LASER_MOUNT_DAMAGE_RADIUS, NULL, LASER_MOUNT_DAMAGE, MOD_LASER_DEFENCE);
	// BANG !
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition(self -> s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	// bye bye laser
	while ((trip_grenade = G_Find (trip_grenade, FOFS(classname), "tripbomb_grenade")))
	{
		if (trip_grenade->owner == self)
		{
			G_FreeEdict (trip_grenade);
		}
	}

	G_FreeEdict (self);
}

void	PlaceLaser (edict_t *ent)
{
	edict_t		*self,
				*grenade;

	vec3_t		forward,
				wallp;

	trace_t		tr;


	ent->client->ps.gunframe++;	

	// valid ent ?
  	if ((!ent->client) || (ent->health<=0))
	   return;

	// cells for laser ?
/*	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < CELLS_FOR_LASER)
	{
 		gi.cprintf(ent, PRINT_HIGH, "Not enough cells for laser.\n");
		return;
	}*/

	// Setup "little look" to close wall
	VectorCopy(ent->s.origin,wallp);         

	// Cast along view angle
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);

	// Setup end point
	wallp[0]=ent->s.origin[0]+forward[0]*50;
	wallp[1]=ent->s.origin[1]+forward[1]*50;
	wallp[2]=ent->s.origin[2]+forward[2]*50;  

	// trace
	/*tr = gi.trace (ent->s.origin, NULL, NULL, wallp, ent, MASK_SOLID);

	// Line complete ? (ie. no collision)
	if (tr.fraction == 1.0)
	{
	 	gi.cprintf (ent, PRINT_HIGH, "Too far from wall.\n");
		return;
	}

	// Hit sky ?
	if (tr.surface)
		if (tr.surface->flags & SURF_SKY)
			return;

	// Ok, lets stick one on then ...
	gi.cprintf (ent, PRINT_HIGH, "Laser attached.\n");*/

	ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= CELLS_FOR_LASER;


	// -----------
	// Setup laser
	// -----------
	self = G_Spawn();

	self -> movetype		= MOVETYPE_NONE;
	self -> solid			= SOLID_NOT;
	self -> s.renderfx		= RF_BEAM|RF_TRANSLUCENT;
	self -> s.modelindex	= 1;			// must be non-zero
	self -> s.sound			= gi.soundindex ("world/laser.wav");
	self -> classname		= "laser_yaya";
	self -> s.frame			= 2;	// beam diameter
  	self -> owner			= self;
	self -> s.skinnum		= 0xf2f2f0f0;

  	self -> dmg				= LASER_DAMAGE;
	self -> think			= pre_target_laser_think;
	self -> delay			= level.time + LASER_TIME;

	// Set orgin of laser to point of contact with wall
	VectorCopy(tr.endpos,self->s.origin);

	// convert normal at point of contact to laser angles
	vectoangles(tr.plane.normal,self -> s.angles);

	// setup laser movedir (projection of laser)
	G_SetMovedir (self->s.angles, self->movedir);

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);

	// link to world
	gi.linkentity (self);

	// start off ...
	target_laser_off (self);

	// ... but make automatically come on
	self -> nextthink = level.time + 2;



		grenade = G_Spawn();

	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	VectorCopy (tr.endpos, grenade->s.origin);
	vectoangles(tr.plane.normal,grenade -> s.angles);

	grenade -> movetype		= MOVETYPE_NONE;
	grenade -> clipmask		= MASK_SHOT;
	grenade -> solid		= SOLID_NOT;
	grenade -> s.modelindex	= gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade -> owner		= self;
	grenade -> nextthink		= level.time + LASER_TIME;
	grenade -> think		= G_FreeEdict;
	grenade -> classname	= "tripbomb_grenade";

	gi.linkentity (grenade);

	//### SPK ### 2/08/98
	//self->attached_entity = grenade;


	//ent->client->ps.gunframe++;	
	//end spk
}


void	pre_target_laser_think (edict_t *self)
{
	target_laser_on (self);

	self->think = target_laser_think;
}

void LaserDef_Laser (edict_t *ent, cplane_t *plane)
{
	edict_t *self;

	// -----------
	// Setup laser
	// -----------
	self = G_Spawn();

	self -> movetype		= MOVETYPE_NONE;
	self -> solid			= SOLID_NOT;
	self -> s.renderfx		= RF_BEAM|RF_TRANSLUCENT;
	self -> s.modelindex	= 1;			// must be non-zero
	self -> s.sound			= gi.soundindex ("world/laser.wav");
	self -> classname		= "laser_defence";
	self -> s.frame			= 2;	// beam diameter
  	self -> owner			= ent;
		self -> s.skinnum		= 0xf2f2f0f0;
  	self -> dmg				= ent->dmg;
	self -> think			= pre_target_laser_think;
	self -> delay			= level.time + LASER_TIME;

	// Set orgin of laser to point of contact with wall
	VectorCopy(ent->s.origin, self->s.origin);

	// convert normal at point of contact to laser angles
	vectoangles(plane->normal,	self -> s.angles);

	// setup laser movedir (projection of laser)
	G_SetMovedir (self->s.angles, self->movedir);

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);

	// link to world
	gi.linkentity (self);

	// start off ...
	target_laser_off (self);

	// ... but make automatically come on
	self -> nextthink = level.time + 2;
}

static void Laser_Defence_Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)
	{
		ent->movetype = MOVETYPE_NONE;
		VectorClear (ent->velocity);
		ent->nextthink = level.time + LASER_TIME;
		ent->think = G_FreeEdict;
		gi.linkentity (ent);
		LaserDef_Laser (ent, plane);
		return;
	}
	else
	{
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
	}
}


void fire_laser_defence_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
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
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->touch = Laser_Defence_Grenade_Touch;
	grenade->dmg = damage;
	grenade->classname = "laser_defence";

	gi.linkentity (grenade);
}

void LaserDefenceGrenade (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int damage = LASER_DAMAGE;

	if (is_quad)
		damage *= 2;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < CELLS_FOR_LASER)
	{
 		gi.cprintf(ent, PRINT_HIGH, "You need at least %d cells for laser-defence\n", CELLS_FOR_LASER);
		return;
	}
	else
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= CELLS_FOR_LASER;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_laser_defence_grenade (ent, start, forward, damage, 600);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

}

//### SPK ### 2/08/98
/*void Weapon_LaserDef (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, PlaceLaser);
}*/