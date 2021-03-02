#include    "g_local.h"
#include	"laser2.h"

int	laser_colour[] = {
		0xf3f3f1f1,		//0 blue
		0xf2f2f0f0,		//1 red
//			0xf2f2f0f0,		//0 red
//			0xf3f3f1f1,		//1 blue
		0xf3f3f1f1,		//2 blue
//		0xd0d1d2d3,		//2 green
		0xdcdddedf,		//3 yellow
		0xe0e1e2e3,		//4 bitty yellow strobe
		0x80818283,     //5 JR brownish purple I think
		0x70717273,		//6 JR light blue
		0x90919293,     //7 JR type of green
		0xb0b1b2b3,		//8 JR another purple
		0x40414243,		//9 JR a reddish color
		0xe2e5e3e6,		//10 JR another orange
		0xd0f1d3f3,		//11 JR mixture of color
		0xf2f3f0f1,		//12 JR red outer blue inner
		0xf3f2f1f0,		//13 JR blue outer red inner
		0xdad0dcd2,		//14 JR yellow outer green inner
		0xd0dad2dc		//15 JR green outer yellow inner
		};

//By setting the color for each, players can tell the difference
#define LASER_DEFENSE_COLOR		4
#define LASER_TRIPBOMB_COLOR	12


/*
=====================
Laser Defense
=====================
*/
void laser_cleanup(edict_t *self)
{
	vec3_t		origin;

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	T_RadiusDamage(self, self->owner, self->dmg, NULL, self->dmg_radius, MOD_LASER_DEFENSE);

	VectorMA (self->s.origin, -0.02, self->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (self->waterlevel)
	{
		if (self->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (self->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	
	//reduce # active laser defenses
// TeT++

	//Remove laser
    if (self->creator)
        G_FreeEdict (self->creator);
// TeT--

	//Remove grenade
	G_FreeEdict (self);

}

void laser_defense_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
// TeT++
    if (self->creator)
	{
        self->creator->delay  = level.time + 0.1;
	}
// TeT--

	laser_cleanup(self);

}

//Laser Defense
// TeT++
void	PlaceLaser (edict_t *ent)
{
	edict_t		*laser,
				*grenade;
	vec3_t		forward,
				wallp,
				start,
				end;
	trace_t		tr;
	trace_t		endTrace;

	// valid ent ?
  	if ((!ent->client) || (ent->health<=0))
	   return;

	// cells for laser ?
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < CELLS_FOR_LASER)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not enough cells for laser.\n");
		return;
	}

	//Are there too many laser defense systems now?
	if (ent->client->pers.active_special[ITEM_SPECIAL_LASER_DEFENSE] >= MAX_SPECIAL_LASER_DEFENSE)
	{
		safe_cprintf(ent, PRINT_HIGH, "You can only have %d active Laser Defense Systems.\n",MAX_SPECIAL_LASER_DEFENSE );
		return;
	}

	// Setup "little look" to close wall
	VectorCopy(ent->s.origin,wallp);

	// Cast along view angle
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);

	// Setup end point
	wallp[0]=ent->s.origin[0]+forward[0]*50;
	wallp[1]=ent->s.origin[1]+forward[1]*50;
	wallp[2]=ent->s.origin[2]+forward[2]*50;

	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, wallp, ent, MASK_SOLID);

	// Line complete ? (ie. no collision)
	if (tr.fraction == 1.0)
	{
		safe_cprintf (ent, PRINT_HIGH, "Too far from wall.\n");
		return;
	}

	// Hit sky ?
	if (tr.surface)
		if (tr.surface->flags & SURF_SKY)
			return;

	// Ok, lets stick one on then ...
	safe_cprintf (ent, PRINT_HIGH, "Laser attached.\n");

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= CELLS_FOR_LASER;

	++ent->client->pers.active_special[ITEM_SPECIAL_LASER_DEFENSE];

    // get enties for both objects
	grenade = G_Spawn();
	laser = G_Spawn();
	grenade->special_index = ITEM_SPECIAL_LASER_DEFENSE;

	// setup the Grenade
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
    VectorCopy (tr.endpos, grenade->s.origin);
    vectoangles(tr.plane.normal, grenade->s.angles);

	grenade->special_index = ITEM_SPECIAL_LASER_DEFENSE;
	grenade->wf_team = ent->wf_team;
	grenade -> movetype		= MOVETYPE_NONE;
	grenade -> clipmask		= MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade -> takedamage	= DAMAGE_YES;
	grenade -> die			= laser_defense_die;
	grenade -> s.modelindex	= gi.modelindex (GRNORMAL_MODEL);
    grenade -> owner        = ent;
    grenade -> creator      = laser;
    grenade -> monsterinfo.aiflags = AI_NOSTEP;
	grenade -> classname	= "laser_defense_gr";
	grenade -> nextthink	= level.time + LASER_TIME;
	grenade -> think		= laser_cleanup;
	grenade -> health		= 10;
	grenade -> max_health	= 10;


	// Now lets find the other end of the laser
    // by starting at the grenade position
    VectorCopy (grenade->s.origin, start);

	// setup laser movedir (projection of laser)
    G_SetMovedir (grenade->s.angles, laser->movedir);
    VectorMA (start, 2048, laser->movedir, end);

	endTrace = gi.trace (start, NULL, NULL, end, ent, MASK_SOLID);

	// -----------
	// Setup laser
	// -----------
	laser -> wf_team = ent->wf_team;

	laser -> movetype		= MOVETYPE_NONE;
	laser -> solid			= SOLID_NOT;
	laser -> s.renderfx		= RF_BEAM|RF_TRANSLUCENT;
	laser -> s.modelindex	= 1;			// must be non-zero
	laser -> s.sound		= gi.soundindex ("world/laser.wav");
	laser -> classname		= "laser_defense";
	laser -> s.frame		= 2;	// beam diameter
    laser -> owner          = NULL;
	laser -> s.skinnum		= laser_colour[ent->wf_team];
  	laser -> dmg			= LASER_DAMAGE;
    laser -> think          = pre_target_laser_def_think;
	laser -> delay			= level.time + LASER_TIME;
	laser -> creator		= grenade;
	laser -> activator		= ent;

	// start off ...
	target_laser_off (laser);
	VectorCopy (endTrace.endpos, laser->s.old_origin);

	// ... but make automatically come on
	laser -> nextthink = level.time + 2;

	// Set orgin of laser to point of contact with wall
	VectorCopy(endTrace.endpos,laser->s.origin);

	// convert normal at point of contact to laser angles
	vectoangles(tr.plane.normal,laser->s.angles);

	// setup laser movedir (projection of laser)
	G_SetMovedir (laser->s.angles, laser->movedir);

	VectorSet (laser->mins, -8, -8, -8);
	VectorSet (laser->maxs, 8, 8, 8);

// link to world
	gi.linkentity (laser);
	gi.linkentity (grenade);
}

void    pre_target_laser_def_think (edict_t *self)
{
	target_laser_on (self);
    self->think = target_laser_def_think;
}
void    pre_target_laser_think (edict_t *self)
{
	target_laser_on (self);
	self->think = target_laser_think;
}

void	pre_target_laserb_think (edict_t *self);

/*
===============
Laser Tripbombs
===============
*/
/*
for them to work need some code I will do later
*/

void laser_trip_cleanup(edict_t *self)
{

	//Remove laser
	if (self->owner)
		G_FreeEdict (self->owner);

	//Remove grenade
	G_FreeEdict (self);

}


void laser_tripbomb_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->owner->delay	= level.time + 0.1;

        laser_trip_cleanup(self);

}

//Laser Tripbomb
void	PlaceLaserb (edict_t *ent)
{
	edict_t		*self,
				*grenade;

	vec3_t		forward,
				wallp;

	trace_t		tr;


	// valid ent ?
  	if ((!ent->client) || (ent->health<=0))
	   return;

	// cells for laser ?
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 100)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not enough cells for laser bomb.\n");
		return;
	}

	//Are there too many laser defense systems now?
	if (ent->client->pers.active_special[ITEM_SPECIAL_TRIPBOMB] >= MAX_SPECIAL_TRIPBOMB)
	{
		safe_cprintf(ent, PRINT_HIGH, "You can only have %d active Trip Bombs.\n",MAX_SPECIAL_TRIPBOMB );
		return;
	}

	// Setup "little look" to close wall
	VectorCopy(ent->s.origin,wallp);

	// Cast along view angle
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);

	// Setup end point
	wallp[0]=ent->s.origin[0]+forward[0]*50;
	wallp[1]=ent->s.origin[1]+forward[1]*50;
	wallp[2]=ent->s.origin[2]+forward[2]*50;

	// trace
	tr = gi.trace (ent->s.origin, NULL, NULL, wallp, ent, MASK_SOLID);

	// Line complete ? (ie. no collision)
	if (tr.fraction == 1.0)
	{
		safe_cprintf (ent, PRINT_HIGH, "Too far from wall.\n");
		return;
	}

	// Hit sky ?
	if (tr.surface)
		if (tr.surface->flags & SURF_SKY)
			return;

	// Ok, lets stick one on then ...
	safe_cprintf (ent, PRINT_HIGH, "Laser attached.\n");

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 100;

	++ent->client->pers.active_special[ITEM_SPECIAL_TRIPBOMB];

	// -----------
	// Setup laser
	// -----------
	self = G_Spawn();
	self->wf_team = ent->wf_team;
	self->special_index = ITEM_SPECIAL_TRIPBOMB;


	self -> movetype		= MOVETYPE_NONE;
	self -> solid			= SOLID_NOT;
	self -> s.renderfx		= RF_BEAM|RF_TRANSLUCENT;
	self -> s.modelindex	= 1;			// must be non-zero
	self -> s.sound			= gi.soundindex ("world/laser.wav");
	self -> classname		= "lb";
	self -> s.frame			= 2;	// beam diameter
  	self -> owner			= ent;
//GREGG
//	self -> s.skinnum		= laser_colour[((int) (random() * 1000)) % 16];
	self -> s.skinnum		= laser_colour[LASER_TRIPBOMB_COLOR];
  	self -> dmg				= LASER_DAMAGE;
	self -> think			= pre_target_laserb_think;
	self -> delay			= level.time + LASER_TIME;
	//add a laser to the amount
//	ent->LaserBomb++;
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
	grenade->wf_team = ent->wf_team;

	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	VectorCopy (tr.endpos, grenade->s.origin);
	vectoangles(tr.plane.normal,grenade -> s.angles);
	grenade -> movetype		= MOVETYPE_NONE;
	grenade -> clipmask		= MASK_SHOT;
	//grenade -> solid		= SOLID_NOT;
	grenade->solid = SOLID_BBOX;
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade->takedamage=DAMAGE_YES;
	grenade->die = laser_tripbomb_die;
	grenade -> s.modelindex	= gi.modelindex (GRNORMAL_MODEL);
	grenade -> owner		= self;
	grenade -> nextthink	= level.time + LASER_TIME;
    grenade -> think                = laser_trip_cleanup;
	grenade->health= 15;
	grenade->max_health =15;

	gi.linkentity (grenade);
}

void	pre_target_laserb_think (edict_t *self)
{
	target_laser_on (self);

	self->think = target_laser_think;
}

void cmd_LaserDefense(edict_t *ent)
{
//	if (ent->LaserOrbs > MAX_LASERS -1)
//		safe_cprintf(ent, PRINT_HIGH, "Max Lasers Already Reached.\n");
//	else
		PlaceLaser (ent);
}

void cmd_TripBomb(edict_t *ent)
{
//	if (ent->LaserBomb > 4 -1)
//		safe_cprintf(ent, PRINT_HIGH, "Max Laser Trip Bombs Already Reached.\n");
//	else
		PlaceLaserb (ent);
}


//Remove all laser defenses for this entity
void cmd_RemoveLaserDefense(edict_t *ent)
{
	edict_t *blip = NULL;

	while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
	{
		if (!strcmp(blip->classname, "laser_defense_gr") && blip->owner == ent)
		{
			blip->think = laser_cleanup;
			blip->nextthink = level.time + .1;
		}
	}

}
