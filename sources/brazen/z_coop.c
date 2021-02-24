#include "g_local.h"

void PutClientAtLatestRallyPoint(edict_t *self)
{
	edict_t	*ent, *point = NULL;
	int player;
	vec3_t origin;

	for (player = 1; player <= game.maxclients; player++)
	{
		ent = &g_edicts[player];
		if (!ent->inuse)
			continue;

		if (!ent->client)
			continue;

		// check for last touched rally point
		if (ent->goalentity != NULL)
		{
			if (point == NULL)
				point = ent->goalentity;
			else if (point->last_fire < ent->goalentity->last_fire)
				point = ent->goalentity;
		}
	}

	player = self->client - game.clients;
	
	if (point != NULL)
	{
		VectorCopy (point->s.origin, origin);

		if (player == 1)
		{
			origin[0] += 32;
			origin[1] += 32;
		}
		else if (player == 2)
		{	
			origin[0] += 32;
			origin[1] -= 32;
		}
		else if (player == 1)
		{
			origin[0] -= 32;
			origin[1] -= 32;
		}
		else if (player == 1)
		{
			origin[0] -= 32;
			origin[1] += 32;
		}
		
		origin[2] += 32;
		VectorCopy (origin, self->spawnpoint);
		self->goalentity = point;
	}

	PutClientInServer(self);
}

void MonsterDropItem (edict_t *self, gitem_t *item, int count, int flags, int ammoTag)
{
	vec3_t	velocity, angles, org;
	vec3_t	forward, right, up;
	edict_t *dropped;

	VectorCopy(self->s.angles, angles);
	angles[PITCH] = 0;      // flat
	AngleVectors(angles, velocity, NULL, NULL);
        VectorScale(velocity, 150, velocity);

        // calc position
	AngleVectors (angles, forward, right, up);
	VectorCopy(self->s.origin, org);

        VectorMA(org, 16, up, org);
        VectorMA(org, 12, forward, org);
	VectorMA(org, 8, right, org);

	dropped = LaunchItem(self, item, org, velocity);
	if (dropped)
	{
		dropped->nextthink = level.time + 30;
		dropped->count = count;
		dropped->viewheight = flags;
		dropped->last_fire = ammoTag;
	        gi.linkentity (dropped);
	}
}


void G_TouchDeadBodies(edict_t *ent)
{
	trace_t		tr;
	edict_t		*ignore;
	int i;

	if (!ent->client)
		return;

	ignore = ent;
			
	for (i = 0; i < 8; i++)
	{	// 8 tries, then fail
		tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, ent->s.origin, ignore, MASK_SHOT);
		if (!tr.ent)
			break;
			
		if (tr.ent->inuse && (tr.ent->svflags & SVF_DEADMONSTER) && tr.ent->touch)
		{
			tr.ent->touch (tr.ent, ent, NULL, NULL);
			break;
		}
		ignore = tr.ent;
	}
}

void TempQuakeThink(edict_t *self)
{
	int		i;
	edict_t	*player;
	vec3_t v;
	
	for (i = 1; i <= maxclients->value; i++)
        {
		player = &g_edicts[i];
		if (!player->inuse)
			continue;
		if (!player->client)
			continue;
		if (!player->groundentity)
			continue;
		if (player->client->pers.spectator)
			continue;
		if (player->movetype == MOVETYPE_NOCLIP)
			continue;
		if (player->client->deadflag)
			continue;

		VectorSubtract(player->s.origin, self->s.origin, v);
		if (VectorLength(v) > self->wait)
			continue;
			
		player->groundentity = NULL;
		player->velocity[0] += crandom()* 50;
		player->velocity[1] += crandom()* 50;
		player->velocity[2] = self->speed * (100.0 / player->mass);
	}

	if (level.time < self->timestamp)
		self->nextthink = level.time + FRAMETIME;
	else
		G_FreeEdict(self);
}

void StepShake(vec3_t pos, float dist, float speed)
{
	edict_t *self;
	
	self = G_Spawn();
	
	if (!self)
		return;

	self->classname = "temp_quake";

	VectorCopy(pos, self->s.origin);
	self->wait = dist;
		
	if (!self->speed)
		self->speed = 50;

	self->svflags |= SVF_NOCLIENT;
	self->think = TempQuakeThink;
	self->timestamp = level.time + 5 * FRAMETIME;
	self->think(self);
	gi.linkentity (self);
}

qboolean CheckBox (edict_t *ent)
{
        edict_t *player;
        int     solids = 0;
        int     i = 0;
		vec3_t v;

	for (i = 1; i <= maxclients->value; i++)
        {
		player = &g_edicts[i];
		if (!player->inuse)
			continue;
		if (!player->client)
			continue;
                if (player == ent)
			continue;
		if (player->client->pers.spectator)
			continue;
		if (player->movetype == MOVETYPE_NOCLIP)
			continue;
		if (player->client->deadflag)
			continue;

		VectorSubtract(player->s.origin, ent->s.origin, v);

		if (VectorLength(v) < 64)
			return true;
	}

	return false;
}

void SP_misc_teleporter_dest (edict_t *ent);

void CheckCoopAllDead (void)
{
	edict_t	*ent, *point = NULL;
	int player;
	int alive = 0;
	vec3_t origin;

	for (player = 1; player <= game.maxclients; player++)
	{
		ent = &g_edicts[player];
		if (!ent->inuse)
			continue;
		if (!ent->client)
			continue;

		// check for last touched rally point
		if (ent->goalentity != NULL)
		{
			if (point == NULL)
				point = ent->goalentity;
			else if (point->last_fire < ent->goalentity->last_fire)
				point = ent->goalentity;
		}

		if (ent->client->pers.spectator)
			continue;

		if (ent->movetype == MOVETYPE_NOCLIP)
			continue;

		if (ent->client->deadflag)
			continue;
		alive++;
	}
	
	// GRIM 8/01/2002 2:14PM - ok, just respawn straight away at last rally point
	//if (alive > 0)
		//return;
	// GRIM
	
	for (player = 1; player <= game.maxclients; player++)
	{
		ent = &g_edicts[player];
		if (!ent->inuse)
			continue;
		if (!ent->client)
			continue;
		if (ent->client->pers.spectator)
			continue;
		if (ent->movetype != MOVETYPE_NOCLIP)
			continue;
		if (!ent->client->deadflag)
			continue;

		if (point != NULL)
		{
			VectorCopy (point->s.origin, origin);

			if (player == 1)
			{
				origin[0] += 32;
				origin[1] += 32;
			}
			else if (player == 2)
			{	
				origin[0] += 32;
				origin[1] -= 32;
			}
			else if (player == 1)
			{
				origin[0] -= 32;
				origin[1] -= 32;
			}
			else if (player == 1)
			{
				origin[0] -= 32;
				origin[1] += 32;
			}
		
			origin[2] += 32;
			VectorCopy (origin, ent->spawnpoint);
			ent->goalentity = point;
		}

		ent->svflags &= ~SVF_NOCLIENT;
		PutClientInServer (ent);

		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;

		ent->client->respawn_time = level.time;
	}
}

static void rally_point_think(edict_t *ent)
{
	ent->s.frame = 1 + ((ent->s.frame + 1) % 16);
	ent->nextthink = level.time + FRAMETIME;
}

void rally_point_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*ent;
	int player;
	vec3_t origin;

	if (!other->client)
		return;

	other->goalentity = self;
	self->last_fire = level.time;
	
	if (self->count != 1)
	{
		self->count = 1;
		self->s.skinnum = 1;
		self->nextthink = level.time + FRAMETIME;
		self->think = rally_point_think;
		self->s.renderfx = RF_GLOW;
		gi.linkentity (self);
		return;
	}
	
	for (player = 1; player <= game.maxclients; player++)
	{
		ent = &g_edicts[player];
		if (!ent->inuse)
			continue;
		if (!ent->client)
			continue;
		if (ent->client->pers.spectator)
			continue;
		if (ent->movetype != MOVETYPE_NOCLIP)
			continue;
		if (!ent->client->deadflag)
			continue;

		VectorCopy (self->s.origin, origin);

		if (player == 1)
		{
			origin[0] += 32;
			origin[1] += 32;
		}
		else if (player == 2)
		{
			origin[0] += 32;
			origin[1] -= 32;
		}
		else if (player == 1)
		{
			origin[0] -= 32;
			origin[1] -= 32;
		}
		else if (player == 1)
		{
			origin[0] -= 32;
			origin[1] += 32;
		}
		
		origin[2] += 32;
		VectorCopy (origin, ent->spawnpoint);

		ent->svflags &= ~SVF_NOCLIENT;
		PutClientInServer (ent);

		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;

		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;

		ent->client->respawn_time = level.time;
		ent->goalentity = self;
	}
}

/*static void rally_point_setup(edict_t *ent)
{
	edict_t *spot;

	spot = G_Spawn();
	VectorCopy(ent->s.origin, spot->s.origin);
	if (spot)
		SP_misc_teleporter_dest(spot);

	ent->nextthink = 0;
	ent->think = NULL;
}*/

/*QUAKED info_player_rally_point
*/
void SP_info_player_rally_point (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;

	VectorSet (ent->mins, -24, -24, -16);
	VectorSet (ent->maxs, 24, 24, 24);

	gi.setmodel (ent, "models/flags/coop/tris.md2");
	ent->s.skinnum = 0;

	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_NONE;  
	ent->touch = rally_point_touch;

	VectorCopy(ent->s.origin, dest);
	dest[2] -= 128;

	tr = gi.trace (ent->s.origin, NULL, NULL, dest, ent, MASK_SOLID);

	VectorCopy (tr.endpos, ent->s.origin);
	ent->s.origin[2] += ent->maxs[2];
	ent->count = 0;
	//ent->nextthink = level.time + 2;
	//ent->think = rally_point_setup;

	gi.linkentity (ent);
}

void ClientEndServerFrames (void);
void CheckNeedPass (void);
void CheckDMRules (void);
void ExitLevel (void);

/*
================
G_RunEditFrame
================
*/
void G_RunEditFrame (void)
{
	int		i;
	edict_t	*ent;

	// exit intermissions
	if (level.exitintermission)
	{
		ExitLevel ();
		return;
	}

	// treat each object in turn
	// even the world gets a chance to think
	ent = &g_edicts[0];
	for (i=0 ; i<globals.num_edicts ; i++, ent++)
	{
		if (!ent->inuse)
			continue;

		level.current_entity = ent;

		VectorCopy (ent->s.origin, ent->s.old_origin);

		// if the ground entity moved, make sure we are still on it
		if ((ent->groundentity) && (ent->groundentity->linkcount != ent->groundentity_linkcount))
		{
			ent->groundentity = NULL;
        		// GRIM 26/06/2001 9:01AM - monsters
			if ( !(ent->flags & (FL_SWIM|FL_FLY)) && (ent->svflags & SVF_MONSTER) )
			{
				M_CheckGround (ent);
			}
        		// GRIM
		}

		if (i > 0 && i <= maxclients->value)
		{
			ClientBeginServerFrame (ent);
			continue;
		}

		// monsters do not think at all
		//if (ent->svflags & SVF_MONSTER)
		if (!ent->decoy)
			ent->nextthink = -1;
		
		G_RunEntity (ent);
	}

	// see if it is time to end a deathmatch
	CheckDMRules ();

	// see if needpass needs updated
	CheckNeedPass ();

	// build the playerstate_t structures for all players
	ClientEndServerFrames ();
}


/*
================
SP_temp_thing
================
*/
void SP_temp_thing (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;

	VectorSet (ent->mins, -24, -24, -16);
	VectorSet (ent->maxs, 24, 24, 24);

	gi.setmodel (ent, "models/items/c_head/tris.md2");

	ent->solid = SOLID_BBOX;
	ent->svflags |= SVF_DEADMONSTER;
	ent->movetype = MOVETYPE_NONE;  

	VectorCopy(ent->s.origin, dest);
	dest[2] -= 128;

	tr = gi.trace (ent->s.origin, NULL, NULL, dest, ent, MASK_SOLID);

	VectorCopy (tr.endpos, ent->s.origin);
	ent->s.origin[2] += ent->maxs[2];
	ent->s.effects |= EF_FLAG1;

	gi.linkentity (ent);
}

void CmdGotoSecret(edict_t *ent)
{
	edict_t	*e = NULL;
	edict_t	*current = NULL;
	edict_t *good = NULL;
	float best = 128;
	float dist;
	vec3_t v;
	int i;

	e = NULL;
	while ((e = G_Find (e, FOFS(classname), "target_secret")))
	{
		VectorSubtract(e->s.origin, ent->s.origin, v);

		dist = VectorLength(v);
		if (dist < best)
		{
			best = dist;
			current = e;
		}
	}

	if (!current)
		current = ent;

	i = current - g_edicts;
	do {
		i++;
		if (i > globals.num_edicts)
			i = 1;
		e = g_edicts + i;
		if (!e->inuse)
			continue;
		if (!Q_stricmp (e->classname, "target_secret"))
		{
			good = e;
			break;
		}
	} while (e != current);
	
	if (!good)
		return;

	if (strstr (good->classname, "trigger_"))
	{
		VectorAdd (good->absmin, good->absmax, v);
		VectorScale (v, 0.5, v);
	}
	else
		VectorCopy(good->s.origin, v);
		
	gi.unlinkentity (ent);

	VectorCopy (v, ent->s.origin);
	VectorCopy (v, ent->s.old_origin);
	ent->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (ent->velocity);
	ent->client->ps.pmove.pm_time = 160>>3;		// hold time
	ent->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	ent->s.event = EV_PLAYER_TELEPORT;

	gi.linkentity (ent);
		
}

void SVEdit_FixAreaPortals (void)
{
	edict_t	*ent;
	edict_t	*t = NULL;
	int i;

	for (i = 1, ent = g_edicts + i; i < globals.num_edicts; i++, ent++)
	{
		if (!ent->inuse)
			continue;
		if (!ent->target)
			continue;
		if (!strstr (ent->classname, "func_"))
			continue;

		t = NULL;
		while ((t = G_Find (t, FOFS(targetname), ent->target)))
		{
			if (Q_stricmp(t->classname, "func_areaportal") == 0)
			{
				ent->s.effects &= ~EF_SPHERETRANS;
				break;
			}
		}
	}
}

void SP_target_laser (edict_t *self);

void SVEdit_DrawPaths (void)
{
	edict_t	*ent;
	edict_t	*las;
	int i;

	ent = NULL;
	while ((ent = G_Find (ent, FOFS(classname), "path_corner")))
	{
		if (ent->target)
		{
			las = G_Spawn();
			if (!las)
				break;
			VectorCopy(ent->s.origin, las->s.origin);
			las->target = ent->target;
			las->spawnflags = 33;
			las->decoy = true;
			SP_target_laser (las);
		}
	}
}
