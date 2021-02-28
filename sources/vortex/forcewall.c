#include "g_local.h"

void mylaser_think (edict_t *self)
{
	// must have an owner
	if (!self->owner || !self->owner->inuse)
	{
		G_FreeEdict(self);
		return;
	}
	
	if ((level.framenum+100 >= self->owner->count) && (level.framenum % 5) == 0)
	{
		// flash yellow if wall is about to time out
		self->s.skinnum = 0xdcdddedf;
	}
	else
	{
		if (self->owner->teamnum)
		{
			if (self->owner->teamnum == 1)
				self->s.skinnum = 0xf2f2f0f0;
			else
				self->s.skinnum = 0xf3f3f1f1;
		}
		else
		{
			self->s.skinnum = 0xd0d1d2d3;
		}
	}
	// update position
	VectorCopy(self->pos2, self->s.origin);
	VectorCopy(self->pos1, self->s.old_origin);

	self->nextthink = level.time + FRAMETIME;
}

void spawnlaser (edict_t *wall, vec3_t v1, vec3_t v2)
{
	edict_t *laser;

	laser = G_Spawn();
	laser->movetype	= MOVETYPE_NONE;
	laser->solid = SOLID_NOT;
	laser->s.renderfx = RF_BEAM|RF_TRANSLUCENT;
	laser->s.modelindex = 1; // must be non-zero
	//laser->s.sound = gi.soundindex ("world/laser.wav");
	laser->classname = "laser";
	laser->s.frame = FORCEWALL_THICKNESS/4+1; // beam diameter
    laser->owner = wall;
	// set laser color
	laser->s.skinnum = 0xd0d1d2d3;

    laser->think = mylaser_think;
	VectorCopy(v2, laser->s.origin);
	VectorCopy(v1, laser->s.old_origin);
	VectorCopy(v2, laser->pos2);
	VectorCopy(v1, laser->pos1);
	VectorClear(laser->mins);
	VectorClear(laser->maxs);
	laser->s.sound = gi.soundindex ("world/force1.wav");
	gi.linkentity(laser);

	laser->nextthink = level.time + FRAMETIME;
}

void forcewall_seteffects (edict_t *self)
{
	int		color;

	// get particle color
	if ((level.framenum+100 >= self->count) && !(level.framenum%5))
	{
		color = 0xdcdddedf;
	}
	else
	{
		if (self->activator->teamnum)
		{
			if (self->activator->teamnum == 1)
				color = 0xf2f2f0f0;
			else
				color = 0xf3f3f1f1;
		}
		else
		{
			color = 0xd0d1d2d3;
		}
	}
	// create particle effect
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_FORCEWALL);
	gi.WritePosition (self->pos1);
	gi.WritePosition (self->pos2);
	gi.WriteByte  (color);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void forcewall_regenerate (edict_t *self)
{
	int	regen;

	regen = self->max_health/300; // regenerate to full in 300 frames (30 seconds)
	if (regen < 1)
		regen = 1;
	if ((self->health < self->max_health) && self->activator->client->pers.inventory[power_cube_index])
	{
		if ((self->health < 0.25*self->max_health) && !(level.framenum%20))
			safe_cprintf(self->activator, PRINT_HIGH, "Wall is low on hp: %d/%d\n", self->health, self->max_health);
		self->health += regen;
		if (self->health > self->max_health)
			self->health = self->max_health;
		if (!(level.framenum%2))
			self->activator->client->pers.inventory[power_cube_index]--;
	//	gi.dprintf("%d/%d health\n", self->health, self->max_health);
	}
}

void forcewall_think(edict_t *self)
{
	int		dmg;
	vec3_t	zvec={0,0,0};
	trace_t	tr;

	CTF_SummonableCheck(self);

	// wall will auto-remove if it is asked to
	if ((self->removetime > 0) && (level.time > self->removetime))
	{
		if (self->activator && self->activator->inuse)
			safe_cprintf(self->activator, PRINT_HIGH, "Your wall was removed from enemy territory.\n");
		self->think = BecomeTE;
		self->nextthink = level.time + FRAMETIME;
		return;
	}

	// wall must have an owner
	if (!G_EntIsAlive(self->activator) || (level.framenum > self->count))
	{
		if (G_EntExists(self->activator))
			safe_cprintf(self->activator, PRINT_HIGH, "Your wall faded away.\n");
		BecomeTE(self);
		return;
	}
	
	forcewall_seteffects(self);

	// is this a solid wall?
	if (self->takedamage)
	{
		forcewall_regenerate(self);
		tr = gi.trace (self->s.origin, self->mins, self->maxs, self->s.origin, self, (MASK_PLAYERSOLID|MASK_MONSTERSOLID));
		// reset solid state if nobody is in the wall
		if ((self->solid == SOLID_NOT) && !G_EntIsAlive(tr.ent) && (level.time > self->wait))
			self->solid = SOLID_BBOX;
	}
	else
	{

		// find something to burn
		tr = gi.trace (self->s.origin, self->mins, self->maxs, self->s.origin, self, (MASK_PLAYERSOLID|MASK_MONSTERSOLID));
		if (G_EntExists(tr.ent) && !OnSameTeam(self, tr.ent))
		{
			if (tr.ent->client && (tr.ent->client->respawn_time == level.time))
			{
				safe_cprintf(self->activator, PRINT_HIGH, "Your wall faded away because it was too close to a spawnpoint!\n");
				BecomeTE(self);
				return;
			}

			dmg = 10 + 4*self->activator->myskills.abilities[FORCE_WALL].current_level;
			burn_person(tr.ent, self->activator, dmg);
			T_Damage (tr.ent, self, self->activator, zvec, tr.ent->s.origin, NULL, dmg, 1, DAMAGE_ENERGY, MOD_BURN);
			
			// wall can only deal so much damage before self-destructing
			self->health -= dmg;
			if (self->health < 0)
			{
				safe_cprintf(self->activator, PRINT_HIGH, "Your wall has expired.\n");
				self->think = BecomeTE;
				self->nextthink = level.time + FRAMETIME;
				return;
			}
			
		}
		//gi.dprintf("found %s\n", tr.ent->classname);
	}

	if (self->delay-10 == level.time)
		safe_cprintf(self->activator, PRINT_HIGH, "Your wall will time-out in 10 seconds.\n");

	self->nextthink = level.time + FRAMETIME;
}

void forcewall_knockback (edict_t *self, edict_t *other)
{
	int		dmg;
	vec3_t	forward, zvec={0,0,0};
	trace_t	tr;

	dmg = 20*self->activator->myskills.abilities[FORCE_WALL].current_level;
	tr = gi.trace (other->s.origin, NULL, NULL, self->s.origin, other, MASK_SHOT);
	T_Damage (other, self, self->activator, zvec, other->s.origin, NULL, dmg, 1, DAMAGE_ENERGY, MOD_BURN);
	vectoangles(tr.plane.normal, forward);
	AngleVectors(forward, forward, NULL, NULL);
	other->velocity[0] = forward[0]*250;
	other->velocity[1] = forward[1]*250;
	other->velocity[2] = 200;
}

void forcewall_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	V_Touch(self, other, plane, surf);

	// let teammates pass thru
	if (/*(other != self->activator) &&*/ G_EntIsAlive(other)) 
	{
		
		if (OnSameTeam(self->activator, other) && (other->movetype == MOVETYPE_STEP || other->movetype == MOVETYPE_WALK))
		{
			//gi.dprintf("%s touching %d \n",other->classname, other->movetype);
			self->solid = SOLID_NOT;
			self->wait = level.time + 0.2; // small buffer so monsters can get thru
		}
		else if (level.time > self->random)
		{
			// dont allow solid force wall to trap people
			if (level.framenum == self->count-900)
			{
				G_FreeEdict(self);
				return;
			}
			forcewall_knockback(self, other); // knock them back
			gi.sound(other, CHAN_ITEM, gi.soundindex("world/force2.wav"), 1, ATTN_NORM, 0);
			self->random = level.time + 0.5; // dont spam the sound
		}
	}
}

void wall_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->deadflag == DEAD_DEAD)
		return;
	if (G_EntExists(self->activator))
		safe_cprintf(self->activator, PRINT_HIGH, "Your wall was destroyed.\n");
	self->deadflag = DEAD_DEAD;
	self->think = G_FreeEdict;
	self->nextthink = level.time + FRAMETIME;
}

void SpawnForcewall(edict_t *player, int type)
{
	vec3_t  forward, point, point1, point2, start;
	trace_t  tr;
	edict_t  *wall;
  
	// spawn force wall where player is pointing at
	wall = G_Spawn();
	VectorCopy(player->s.origin, start);
	start[2] += player->viewheight;
	AngleVectors(player->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, point);
	tr = gi.trace(start, NULL, NULL, point, player, MASK_SHOT);
	tr.endpos[2]++;
	VectorCopy(tr.endpos, wall->s.origin); // copy origin to ending position of trace
  
	if(fabs(forward[0]) > fabs(forward[1]))
	{
		// set wall thickness
		wall->pos1[0] = wall->pos2[0] = wall->s.origin[0];
		wall->mins[0] =  -FORCEWALL_THICKNESS/2;
		wall->maxs[0] =   FORCEWALL_THICKNESS/2;

		// set wall width
		VectorCopy(wall->s.origin,point);
		point[1] -= FORCEWALL_WIDTH/2;
		tr = gi.trace(wall->s.origin,NULL,NULL,point,NULL,MASK_SOLID);
		wall->pos1[1] = tr.endpos[1];
		wall->mins[1] = wall->pos1[1] - wall->s.origin[1]; // set left bbox
    
		point[1] = wall->s.origin[1] + FORCEWALL_WIDTH/2;
		tr = gi.trace(wall->s.origin,NULL,NULL,point,NULL,MASK_SOLID);
		wall->pos2[1] = tr.endpos[1];
		wall->maxs[1] = wall->pos2[1] - wall->s.origin[1]; // set right bbox
		// dont spawn wall near spawnpoint or flag
		VectorCopy(wall->pos1, point1);
		VectorCopy(wall->pos2, point2);
		point1[2]--;
		point2[2]--;
		tr = gi.trace(point1,NULL,NULL,point2,wall,MASK_SOLID);
		if ((!strcmp(tr.ent->classname, "info_player_start"))
			|| (!strcmp(tr.ent->classname, "info_player_deathmatch"))
			|| (!strcmp(tr.ent->classname, "item_flagreturn_team1"))
			|| (!strcmp(tr.ent->classname, "item_flagreturn_team2"))
			|| (!strcmp(tr.ent->classname, "misc_teleporter_dest"))
			|| (!strcmp(tr.ent->classname, "info_teleport_destination")))
		{
		  	safe_cprintf(player, PRINT_HIGH, "Forcewall is too close to a spawnpoint or flag.\nForcewall removed.\n");
			G_FreeEdict(wall);
		  	return;
		}
	}
  else
  {
		// set wall width
		VectorCopy(wall->s.origin,point);
		point[0] -= FORCEWALL_WIDTH/2;
		tr = gi.trace(wall->s.origin,NULL,NULL,point,NULL,MASK_SOLID);
		wall->pos1[0] = tr.endpos[0];
		wall->mins[0] = wall->pos1[0] - wall->s.origin[0];
    
		point[0] = wall->s.origin[0] + FORCEWALL_WIDTH/2;
		tr = gi.trace(wall->s.origin,NULL,NULL,point,NULL,MASK_SOLID);
		wall->pos2[0] = tr.endpos[0];
		wall->maxs[0] = wall->pos2[0] - wall->s.origin[0];
    
		// set wall thickness
		wall->pos1[1] = wall->pos2[1] = wall->s.origin[1];
		wall->mins[1] = -FORCEWALL_THICKNESS/2;
		wall->maxs[1] =  FORCEWALL_THICKNESS/2;
		// dont spawn near spawnpoint or flag
		VectorCopy(wall->pos1, point1);
		VectorCopy(wall->pos2, point2);
		point1[2]--;
		point2[2]--;
		tr = gi.trace(point1,NULL,NULL,point2,wall,MASK_SOLID);
		if ((!strcmp(tr.ent->classname, "info_player_start"))
			|| (!strcmp(tr.ent->classname, "info_player_deathmatch"))
			|| (!strcmp(tr.ent->classname, "item_flagreturn_team1"))
			|| (!strcmp(tr.ent->classname, "item_flagreturn_team2"))
			|| (!strcmp(tr.ent->classname, "misc_teleporter_dest"))
			|| (!strcmp(tr.ent->classname, "info_teleport_destination")))
		{
		  	safe_cprintf (player, PRINT_HIGH, "Forcewall is too close to a spawnpoint or flag.\nForcewall removed.\n");
			G_FreeEdict(wall);
		  	return ;
		 }
  }
	wall->mins[2] = 0;
	wall->maxs[2] = 0;
	// set wall height
	VectorCopy(wall->s.origin,point);
	point[2] = wall->s.origin[2] + FORCEWALL_HEIGHT;
	tr = gi.trace(wall->s.origin,wall->mins,wall->maxs,point,NULL,MASK_SOLID);
	wall->maxs[2] = tr.endpos[2] - wall->s.origin[2];
	wall->pos1[2] = wall->pos2[2] = tr.endpos[2];

	wall->style = 208; // wall color
	wall->classname = "Forcewall";
	wall->mtype = M_FORCEWALL;
	wall->activator = player;
	
	//4.2 don't set owner if we are a medic, this allows medic to heal the wall
	if (player->mtype != MORPH_MEDIC)
		wall->owner = player;

	wall->s.modelindex = 1;
	if (type)
	{
		wall->max_health = 500*wall->activator->myskills.abilities[FORCE_WALL].current_level;
		wall->health = 0.1*wall->max_health;
	}
	else
	{
		wall->max_health = 300*wall->activator->myskills.abilities[FORCE_WALL].current_level;
		wall->health = wall->max_health;
	}
	wall->monsterinfo.power_armor_power = 0;
	wall->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
	wall->monsterinfo.level = wall->activator->myskills.abilities[FORCE_WALL].current_level;
	if (type)
	{
		wall->takedamage = DAMAGE_AIM;
		wall->touch = forcewall_touch;	
	}
	wall->die = wall_die;
	wall->count = level.framenum + 1000; // wall timeout
	wall->movetype = MOVETYPE_NONE;
	if (type)
		wall->solid = SOLID_BBOX;
	else
		wall->solid = SOLID_NOT;
	wall->clipmask = MASK_MONSTERSOLID;
	wall->think = forcewall_think;
	wall->nextthink = level.time + FRAMETIME;
	wall->svflags = SVF_NOCLIENT;//need this or we get some strange graphic errors (no model?)

	gi.linkentity(wall);

	// create lasers
	spawnlaser(wall, wall->pos1, wall->pos2);// top laser
	VectorCopy(wall->pos1, point1);
	VectorCopy(wall->pos2, point2);
	point1[2] -= FORCEWALL_HEIGHT;
	point2[2] -= FORCEWALL_HEIGHT;
	spawnlaser(wall, point1, point2);// bottom laser
	spawnlaser(wall, wall->pos1, point1);// left laser
	spawnlaser(wall, wall->pos2, point2);// right laser
}

void ForcewallOff(edict_t *player)
{
  vec3_t  forward, point, start;
  trace_t  tr;
  
  VectorCopy(player->s.origin,start);
  start[2] += player->viewheight;
  AngleVectors(player->client->v_angle,forward,NULL,NULL);
  VectorMA(start,8192,forward,point);
  tr = gi.trace(start,NULL,NULL,point,player,MASK_SHOT);
  if(Q_stricmp(tr.ent->classname, "forcewall"))
  {
    safe_cprintf(player,PRINT_HIGH,"Not a forcewall!\n");
    return;
  }
  if(tr.ent->activator != player)
  {
    safe_cprintf(player,PRINT_HIGH,"You don't own this forcewall, bub!\n");
    return;
  }
  G_FreeEdict(tr.ent);
}

void Cmd_Forcewall(edict_t *ent)
{
	edict_t		*scan=NULL;
	qboolean	solid=false;

	if (debuginfo->value)
		gi.dprintf("DEBUG: %s just called Cmd_Forcewall()\n", ent->client->pers.netname);

	if(ent->myskills.abilities[FORCE_WALL].disable)
		return;

	if (Q_strcasecmp(gi.args(), "solid") == 0)
		solid = true;

	// remove any existing wall owned by this player
	while((scan = G_Find(scan, FOFS(classname), "Forcewall")) != NULL) {
		if (scan && scan->activator && scan->activator->client && (scan->activator == ent)) {
			G_FreeEdict(scan);
			safe_cprintf(ent, PRINT_HIGH, "Your forcewall was removed.\n");
			return;
		} 
	}

	if (solid)
	{
		if (!G_CanUseAbilities(ent, ent->myskills.abilities[FORCE_WALL].current_level, FORCEWALL_SOLID_COST))
			return;
		ent->client->pers.inventory[power_cube_index] -= FORCEWALL_SOLID_COST;
	}
	else
	{
		if (!G_CanUseAbilities(ent, ent->myskills.abilities[FORCE_WALL].current_level, FORCEWALL_NOTSOLID_COST))
			return;
		ent->client->pers.inventory[power_cube_index] -= FORCEWALL_NOTSOLID_COST;
	}
	ent->client->ability_delay = level.time + FORCEWALL_DELAY;

	ent->lastsound = level.framenum;

	if (solid)
		SpawnForcewall(ent,1);
	else
		SpawnForcewall(ent,0);

	// if you use this spell, you uncloak!
	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->cloaking = false;
	ent->client->cloakable = 0;
}