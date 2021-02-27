#include    "g_local.h"

#define LASER_INITIAL_DMG		120
#define LASER_ADDON_DMG			30

// cumulative maximum damage a laser can deal
#define LASER_INITIAL_HEALTH	1000
#define LASER_ADDON_HEALTH		400

void    pre_target_laser_def_think (edict_t *self);
void target_laser_think (edict_t *self);
void target_laser_off (edict_t *self);
void target_laser_on (edict_t *self);

int	laser_colour[] = {
		0xf3f3f1f1,		//0 blue
		0xf2f2f0f0,		//1 red
		0xf3f3f1f1,		//2 blue
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
#define LASER_DEFENSE_COLOR		1
#define LASER_DEFENSE_WARNING	3
#define LASER_TRIPBOMB_COLOR	12

/*
==========
RemoveLaserDefense

removes all lasers for this entity
==========
*/
void RemoveLaserDefense (edict_t *ent)
{
	edict_t *e = NULL;

	while((e = G_Find(e, FOFS(classname), "laser_defense_gr")) != NULL) {
		if (e && (e->owner == ent))
		{
			// remove the linked laser
			if (e->creator)
				G_FreeEdict(e->creator);
			// remove the grenade
			BecomeExplosion1(e);
		}
	}
	ent->num_lasers = 0;
}

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

	self->owner->num_lasers--; //GHz: Decrement counter

	//Remove laser
    if (self->creator)
        G_FreeEdict (self->creator);

	//Remove grenade
	G_FreeEdict (self);
}

void target_laser_def_think (edict_t *self)
{
	edict_t	*ignore;
	vec3_t	start;
	vec3_t	end;
	trace_t	tr;
	vec3_t	point;
	vec3_t	last_movedir;
	int		count;

	if (!self->creator || !self->creator->inuse)
	{
		// this should never happen!
		gi.dprintf("WARNING: laser has an invalid creator!\n");
		G_FreeEdict(self);
		return;
	}

	// warn player that the laser is about to expire
	if (self->creator && ((level.time+10) >= self->creator->nextthink))
	{
		if ((level.time+10) == self->creator->nextthink)
			safe_cprintf(self->activator, PRINT_HIGH, "A laser is about to expire. \n");
		if ((level.framenum % 5) == 0)
		{
			self->s.skinnum = laser_colour[LASER_DEFENSE_WARNING];
		}
		else
		{
			self->s.skinnum = laser_colour[LASER_DEFENSE_COLOR];
		}
	}

	if (self->spawnflags & 0x80000000)
		count = 8;
	else
		count = 4;

	if (self->enemy)
	{
		VectorCopy (self->movedir, last_movedir);
		VectorMA (self->enemy->absmin, 0.5, self->enemy->size, point);
		VectorSubtract (point, self->s.origin, self->movedir);
		VectorNormalize (self->movedir);
		if (!VectorCompare(self->movedir, last_movedir))
			self->spawnflags |= 0x80000000;
	}

	ignore = self;
	VectorCopy (self->creator->s.origin, start);
	VectorMA (start, 2048, self->movedir, end);
	while(1)
	{
		tr = gi.trace (start, NULL, NULL, self->s.origin, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

		VectorCopy (tr.endpos,self->moveinfo.end_origin);

		if (!tr.ent)
			break;

		// hurt it if we can
		if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && !(OnSameTeam(self->activator, tr.ent)) && tr.ent->inuse)
		{
			if (tr.ent->client && (tr.ent->client->respawn_time == level.time))
			{
				laser_cleanup(self->creator);
				return;
			}

			if (!self->creator || !self->creator->inuse)
			{
				// 3.15 try to catch fatal crash bug
				gi.dprintf("WARNING: target_laser_def_think() doesn't have a creator (%s)\n", self->classname);
				return;
			}

			T_Damage (tr.ent, self, self->activator, self->movedir, tr.endpos, vec3_origin, self->dmg, 1, DAMAGE_ENERGY, MOD_LASER_DEFENSE);

			if (!self->creator || !self->creator->inuse)
			{
				// 3.15 try to catch fatal crash bug
				gi.dprintf("ERROR: target_laser_def_think() aborted to avoid fatal crash (%s)\n", self->classname);
				return;
			}

			self->health -= self->dmg;

		//	gi.dprintf("%d\n", self->health);

			if (self->health < 1) {
				laser_cleanup(self->creator);
				return;
			}
			//gi.dprintf("Laser health: %d Laser damage: %d\n", self->health, self->dmg);
		}


		// if we hit something that's not a monster or player or is immune to lasers, we're done
		if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client) && (strcmp(tr.ent->classname,"laser_defense_gr") != 0))
		{
			if (self->spawnflags & 0x80000000)
			{
				self->spawnflags &= ~0x80000000;
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_LASER_SPARKS);
				gi.WriteByte (count);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.WriteByte (self->s.skinnum);
				gi.multicast (tr.endpos, MULTICAST_PVS);
			}
			break;
		}

		ignore = tr.ent;
		VectorCopy (tr.endpos, start);
	}
	if (self->creator)
		VectorCopy (self->creator->s.origin, self->s.old_origin);
	if (self->health > 0)
		self->nextthink = level.time + FRAMETIME;
}

void	PlaceLaser (edict_t *ent)
{
	edict_t		*laser,
				*grenade;
	edict_t		*blip = NULL;//GHz
	vec3_t		forward,
				wallp,
				start,
				end;
	trace_t		tr;
	trace_t		endTrace;
	int health=0;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called PlaceLaser()\n", ent->client->pers.netname);

	health = LASER_INITIAL_HEALTH+LASER_ADDON_HEALTH*ent->myskills.abilities[BUILD_LASER].current_level;

	// valid ent ?
  	if ((!ent->client) || (ent->health<=0))
	   return;

	if ((deathmatch->value) && (level.time < pregame_time->value)) {
		if (ent->client)
			safe_cprintf(ent, PRINT_HIGH, "You cannot use this ability in pre-game!\n");
		return;
	}

	if (Q_strcasecmp (gi.args(), "remove") == 0) {
		RemoveLaserDefense(ent);
		return;
	}

	if(ent->myskills.abilities[BUILD_LASER].disable)
		return;

	//3.0 amnesia disables lasers
	if (que_findtype(ent->curses, NULL, AMNESIA) != NULL)
		return;

	if (ent->myskills.abilities[BUILD_LASER].current_level < 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "You can't make lasers due to not training in it!\n");
		return;
	}
	// cells for laser ?
	if (ent->client->pers.inventory[power_cube_index] < LASER_COST)
	{
		safe_cprintf(ent, PRINT_HIGH, "Not enough Power Cubes for laser.\n");
		return;
	}

	if (ent->client->ability_delay > level.time) {
		safe_cprintf (ent, PRINT_HIGH, "You can't use abilities for another %2.1f seconds\n", ent->client->ability_delay - level.time);
		return;
	}
	
	ent->client->ability_delay = level.time + DELAY_LASER;

	//gi.dprintf("DEBUG: %s is attempting to place a laser...\n", ent->client->pers.netname);
	
	// GHz: Reached max number allowed ?
	if (ent->num_lasers >= MAX_LASERS) {
		safe_cprintf(ent, PRINT_HIGH, "You have reached the max of %d lasers\n", MAX_LASERS);
		return;
	}

	// Setup "little look" to close wall
	VectorCopy(ent->s.origin,wallp);

	// Cast along view angle
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);

	// Setup end point
	wallp[0]=ent->s.origin[0]+forward[0]*128;
	wallp[1]=ent->s.origin[1]+forward[1]*128;
	wallp[2]=ent->s.origin[2]+forward[2]*128;

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
/*
	while (blip = findradius (blip, ent->s.origin, 64))
	{
		if (!visible(ent, blip))
			continue;

		 if ( (!strcmp(blip->classname, "worldspawn") )
		  || (!strcmp(blip->classname, "info_player_start") )
		  || (!strcmp(blip->classname, "info_player_deathmatch") )
		  || (!strcmp(blip->classname, "item_flagreturn_team1") )
		  || (!strcmp(blip->classname, "item_flagreturn_team2") )
		  || (!strcmp(blip->classname, "misc_teleporter_dest") )
		  || (!strcmp(blip->classname, "info_teleport_destination") ) )
		 {
		  	safe_cprintf (ent, PRINT_HIGH, "Laser is too close to a spawnpoint or flag.\n");
		  	return ;
		 }
	}
*/

	// Ok, lets stick one on then ...
	safe_cprintf (ent, PRINT_HIGH, "Laser attached.\n");
/*
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	{
		ent->client->pers.inventory[power_cube_index] -= LASER_COST;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Lasers"))]--;
	}*/

    // get entities for both objects
	grenade = G_Spawn();
	laser = G_Spawn();

	// setup the Grenade
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
    VectorCopy (tr.endpos, grenade->s.origin);
    vectoangles(tr.plane.normal, grenade->s.angles);

	grenade -> movetype		= MOVETYPE_NONE;
	grenade -> clipmask		= MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade -> takedamage	= DAMAGE_NO;
	grenade -> s.modelindex	= gi.modelindex ("models/objects/grenade2/tris.md2");
    grenade -> owner        = ent;
    grenade -> creator      = laser;
    grenade -> monsterinfo.aiflags = AI_NOSTEP;
	grenade -> classname	= "laser_defense_gr";
	grenade -> nextthink	= level.time + LASER_TIMEUP+GetRandom(0,30);//GetRandom((LASER_TIMEUP/2),(2*LASER_TIMEUP));
	grenade -> think		= laser_cleanup;


	// Now lets find the other end of the laser
    // by starting at the grenade position
    VectorCopy (grenade->s.origin, start);

	// setup laser movedir (projection of laser)
    G_SetMovedir (grenade->s.angles, laser->movedir);

	gi.linkentity (grenade);

    VectorMA (start, 2048, laser->movedir, end);

	endTrace = gi.trace (start, NULL, NULL, end, ent, MASK_SOLID);

	// -----------
	// Setup laser
	// -----------
	laser->movetype		= MOVETYPE_NONE;
	laser->solid			= SOLID_NOT;
	laser->s.renderfx		= RF_BEAM|RF_TRANSLUCENT;
	laser->s.modelindex	= 1;			// must be non-zero
	laser->s.sound		= gi.soundindex ("world/laser.wav");
	laser->classname		= "laser_defense";
	laser->s.frame		= 2 /* ent->myskills.build_lasers*/;	// as it gets higher in levels, the bigger it gets. beam diameter
    laser->owner          = laser;
	laser->s.skinnum		= laser_colour[LASER_DEFENSE_COLOR];
  	laser->dmg			= LASER_INITIAL_DMG+LASER_ADDON_DMG*ent->myskills.abilities[BUILD_LASER].current_level;
    laser->think          = pre_target_laser_def_think;
	//laser->delay			= level.time + LASER_TIMEUP;
	laser->health = health;
	laser->creator		= grenade;
	laser->activator		= ent;

	// start off ...
	target_laser_off (laser);
	VectorCopy (endTrace.endpos, laser->s.old_origin);

	// ... but make automatically come on
	laser -> nextthink = level.time + 2;

	// Set orgin of laser to point of contact with wall
	VectorCopy(endTrace.endpos,laser->s.origin);

	/*
	while (blip = findradius (blip, laser->s.origin, 64))
	{
		if (!visible(laser, blip))
			continue;

		 if ( (!strcmp(blip->classname, "worldspawn") )
		  || (!strcmp(blip->classname, "info_player_start") )
		  || (!strcmp(blip->classname, "info_player_deathmatch") )
		  || (!strcmp(blip->classname, "item_flagreturn_team1") )
		  || (!strcmp(blip->classname, "item_flagreturn_team2") )
		  || (!strcmp(blip->classname, "misc_teleporter_dest") )
		  || (!strcmp(blip->classname, "info_teleport_destination") ) )
		 {
		  	safe_cprintf (ent, PRINT_HIGH, "Laser is too close to a spawnpoint or flag.\nLaser Removed.\n");
			G_FreeEdict(laser);
			G_FreeEdict(grenade);
		  	return ;
		 }
	}
	*/

	// convert normal at point of contact to laser angles
	vectoangles(tr.plane.normal,laser->s.angles);

	// setup laser movedir (projection of laser)
	G_SetMovedir (laser->s.angles, laser->movedir);

	VectorSet (laser->mins, -18, -18, -18);
	VectorSet (laser->maxs, 18, 18, 18);

// link to world
	gi.linkentity (laser);
	ent->num_lasers++; // GHz: add to laser counter

	//If you use this spell, you uncloak!
	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->cloaking = false;
	ent->client->cloakable = 0;

	ent->client->pers.inventory[power_cube_index] -= LASER_COST;
	//gi.dprintf("DEBUG: %s successfully created a laser.\n", ent->client->pers.netname);
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

void Use_Lasers (edict_t *ent, gitem_t *item) 
{
	safe_cprintf(ent, PRINT_HIGH, "Use the command \"laser\" instead.\n");
	return;
/*	if (ent->myskills.build_lasers < 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "You can't make lasers due to not training in it!\n");
		return;
	}
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Lasers"))] < 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "You are out of lasers to place.\n");
		return;
	}

	PlaceLaser (ent);*/
}




