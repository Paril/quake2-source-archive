// g_utils.c -- misc utility functions for game module

#include "g_local.h"


void G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1];
	result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1];
	result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] + distance[2];
}


/*
=============
G_Find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
edict_t *G_Find (edict_t *from, int fieldofs, char *match)
{
	char	*s;

	if (!from)
		from = g_edicts;
	else
		from++;

	for ( ; from < &g_edicts[globals.num_edicts] ; from++)
	{
		if (!from->inuse)
			continue;
		s = *(char **) ((byte *)from + fieldofs);
		if (!s)
			continue;
		if (!Q_stricmp (s, match))
			return from;
	}

	return NULL;
}


/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
edict_t *findradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
		if (from->solid == SOLID_NOT)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
}


/*
=============
G_PickTarget

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
#define MAXCHOICES	8

edict_t *G_PickTarget (char *targetname)
{
	edict_t	*ent = NULL;
	int		num_choices = 0;
	edict_t	*choice[MAXCHOICES];

	if (!targetname)
	{
		gi.dprintf("G_PickTarget called with NULL targetname\n");
		return NULL;
	}

	while(1)
	{
		ent = G_Find (ent, FOFS(targetname), targetname);
		if (!ent)
			break;
		choice[num_choices++] = ent;
		if (num_choices == MAXCHOICES)
			break;
	}

	if (!num_choices)
	{
		gi.dprintf("G_PickTarget: target %s not found\n", targetname);
		return NULL;
	}

	return choice[rand() % num_choices];
}



void Think_Delay (edict_t *ent)
{
	G_UseTargets (ent, ent->activator);
	G_FreeEdict (ent);
}

/*
==============================
G_UseTargets

the global "activator" should be set to the entity that initiated the firing.

If self.delay is set, a DelayedUse entity will be created that will actually
do the SUB_UseTargets after that many seconds have passed.

Centerprints any self.message to the activator.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function

==============================
*/
void G_UseTargets (edict_t *ent, edict_t *activator)
{
	edict_t		*t;

//
// check for a delay
//
	if (ent->delay)
	{
	// create a temp object to fire at a later time
		t = G_Spawn();
		t->classname = "DelayedUse";
		t->nextthink = level.time + ent->delay;
		t->think = Think_Delay;
		t->activator = activator;
		if (!activator)
			gi.dprintf ("Think_Delay with no activator\n");
		t->message = ent->message;
		t->target = ent->target;
		t->killtarget = ent->killtarget;
		return;
	}
	
	
//
// print the message
//
	if ((ent->message) && !(activator->svflags & SVF_MONSTER))
	{
		gi.centerprintf (activator, "%s", ent->message);
		if (ent->noise_index)
			gi.sound (activator, CHAN_AUTO, ent->noise_index, 1, ATTN_NORM, 0);
		else
			gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
	}

//
// kill killtargets
//
	if (ent->killtarget)
	{
		t = NULL;
		while ((t = G_Find (t, FOFS(targetname), ent->killtarget)))
		{
			G_FreeEdict (t);
			if (!ent->inuse)
			{
				gi.dprintf("entity was removed while using killtargets\n");
				return;
			}
		}
	}

//
// fire targets
//
	if (ent->target)
	{
		t = NULL;
		while ((t = G_Find (t, FOFS(targetname), ent->target)))
		{
			// doors fire area portals in a specific way
			if (!Q_stricmp(t->classname, "func_areaportal") &&
				(!Q_stricmp(ent->classname, "func_door") || !Q_stricmp(ent->classname, "func_door_rotating")))
				continue;

			if (t == ent)
			{
				gi.dprintf ("WARNING: Entity used itself.\n");
			}
			else
			{
				if (t->use)
					t->use (t, ent, activator);
			}
			if (!ent->inuse)
			{
				gi.dprintf("entity was removed while using targets\n");
				return;
			}
		}
	}
}


/*
=============
TempVector

This is just a convenience function
for making temporary vectors for function calls
=============
*/
float	*tv (float x, float y, float z)
{
	static	int		index;
	static	vec3_t	vecs[8];
	float	*v;

	// use an array so that multiple tempvectors won't collide
	// for a while
	v = vecs[index];
	index = (index + 1)&7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}


/*
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
char	*vtos (vec3_t v)
{
	static	int		index;
	static	char	str[8][32];
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = (index + 1)&7;

	Com_sprintf (s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

	return s;
}


vec3_t VEC_UP		= {0, -1, 0};
vec3_t MOVEDIR_UP	= {0, 0, 1};
vec3_t VEC_DOWN		= {0, -2, 0};
vec3_t MOVEDIR_DOWN	= {0, 0, -1};

void G_SetMovedir (vec3_t angles, vec3_t movedir)
{
	if (VectorCompare (angles, VEC_UP))
	{
		VectorCopy (MOVEDIR_UP, movedir);
	}
	else if (VectorCompare (angles, VEC_DOWN))
	{
		VectorCopy (MOVEDIR_DOWN, movedir);
	}
	else
	{
		AngleVectors (angles, movedir, NULL, NULL);
	}

	VectorClear (angles);
}


float vectoyaw (vec3_t vec)
{
	float	yaw;
	
	if (vec[YAW] == 0 && vec[PITCH] == 0)
		yaw = 0;
	else
	{
		yaw = (int) (atan2(vec[YAW], vec[PITCH]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;
	}

	return yaw;
}


void vectoangles (vec3_t value1, vec3_t angles)
{
	float	forward;
	float	yaw, pitch;
	
	if (value1[1] == 0 && value1[0] == 0)
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (int) (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}

char *G_CopyString (char *in)
{
	char	*out;
	
	out = gi.TagMalloc (strlen(in)+1, TAG_LEVEL);
	strcpy (out, in);
	return out;
}


void G_InitEdict (edict_t *e)
{
	e->inuse = true;
	e->classname = "noclass";
	e->gravity = 1.0;
	e->s.number = e - g_edicts;
}

/*
=================
G_Spawn

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
edict_t *G_Spawn (void)
{
	int			i;
	edict_t		*e;

	e = &g_edicts[(int)maxclients->value+1];
	for ( i=maxclients->value+1 ; i<globals.num_edicts ; i++, e++)
	{
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (!e->inuse && ( e->freetime < 2 || level.time - e->freetime > 0.5 ) )
		{
			G_InitEdict (e);
			return e;
		}
	}
	
	if (i == game.maxentities)
		gi.error ("ED_Alloc: no free edicts");
		
	globals.num_edicts++;
	G_InitEdict (e);
	return e;
}

/*
=================
G_FreeEdict

Marks the edict as free
=================
*/
void G_FreeEdict (edict_t *ed)
{
	gi.unlinkentity (ed);		// unlink from world

	if ((ed - g_edicts) <= (maxclients->value + BODY_QUEUE_SIZE))
	{
//		gi.dprintf("tried to free special edict\n");
		return;
	}

	memset (ed, 0, sizeof(*ed));
	ed->classname = "freed";
	ed->freetime = level.time;
	ed->inuse = false;
}


/*
============
G_TouchTriggers

============
*/
void	G_TouchTriggers (edict_t *ent)
{
	int			i, num;
	edict_t		*touch[MAX_EDICTS], *hit;

	// dead things don't activate triggers!
	if ((ent->client || (ent->svflags & SVF_MONSTER)) && (ent->health <= 0))
		return;

	num = gi.BoxEdicts (ent->absmin, ent->absmax, touch
		, MAX_EDICTS, AREA_TRIGGERS);
// SH
//if (!(strcmp(ent->classname, "grapple")))
//	gi.cprintf (ent->owner, PRINT_HIGH, "num: %i\n", num);
	if (!(strcmp(ent->classname, "grapple")) && num > 0)
	{
//		ent->flags |= FL_BERSERK;
		ent = ent->owner;
	}
	else if (!(strcmp(ent->classname, "angel of life")) && num > 0)
	{
//		ent->enemy = touch[0];
		ent->goalentity = NULL;
		ent->enemy = NULL;
		ent = ent->owner;
	}
// \SH
	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;
		if (!hit->touch)
			continue;
		hit->touch (hit, ent, NULL, NULL);
	}
}

/*
============
G_TouchSolids

Call after linking a new trigger in during gameplay
to force all entities it covers to immediately touch it
============
*/
void	G_TouchSolids (edict_t *ent)
{
	int			i, num;
	edict_t		*touch[MAX_EDICTS], *hit;

	num = gi.BoxEdicts (ent->absmin, ent->absmax, touch
		, MAX_EDICTS, AREA_SOLID);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;
		if (ent->touch)
			ent->touch (hit, ent, NULL, NULL);
		if (!ent->inuse)
			break;
	}
}




/*
==============================================================================

Kill box

==============================================================================
*/

/*
=================
KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
qboolean KillBox (edict_t *ent)
{
	trace_t		tr;

	while (1)
	{
		tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, ent->s.origin, NULL, MASK_PLAYERSOLID);
		if (!tr.ent)
			break;
// SH	
		if (ent->svflags & SVF_MONSTER && (tr.ent->client || tr.ent->svflags & SVF_MONSTER) && tr.ent != ent)
		{
			G_FreeEdict (ent);
			level.total_monsters--;
//gi.bprintf (PRINT_HIGH, "monster telefrag\n");
			return false;
		}
		else if (ent->client)
// \SH
		{
		// nail it
			T_Damage (tr.ent, ent, world, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
		// attacker used to be ent
		}

		// if we didn't kill it, fail
// SH
/*
		if (ent->svflags & SVF_MONSTER)
		{
			if (tr.ent->solid)
			{
				G_FreeEdict (ent);
				level.total_monsters--;
gi.bprintf (PRINT_HIGH, "monster in wall\n");
				return false;
			}
		}
		else
*/
		if (ent->svflags & SVF_MONSTER)
		{
			if (tr.ent == world)
			{
				ent->inuse = true;
				return true;
			}
			else
			{
//gi.bprintf (PRINT_HIGH, "%s\n", tr.ent->classname);
				
			}
		}
		else
// \SH
		if (tr.ent->solid)
			return false;
	}

	if (ent->flags & SVF_MONSTER)
	{
		ent->inuse = true;
	}

	return true;		// all clear
}

// SH

edict_t *nextent (edict_t *from)
{
	edict_t * count;

	if (!from)
		return world;

	for (count = from+1; count != &g_edicts[game.maxentities-1]; count++) 
	{
		if (count->inuse)
			return count;
	}

	return NULL;

//	if (!from)
//		from = g_edicts;
//	else
//		from++;
//
//	if (!from->inuse)
//		return NULL;

//	return from;
}

void stuffcmd(edict_t *e, char *s) 
{
	gi.WriteByte (11);
	gi.WriteString (s);
	gi.unicast (e, true);
}

float framerate (usercmd_t *cmd)
{
	return ((float)cmd->msec / 1000);
}

void vrandom (vec3_t dir)
{
	dir[0] = crandom();
	dir[1] = crandom();
	dir[2] = crandom();

	VectorNormalize2 (dir, dir);
}

void avrandom (vec3_t dir)
{
	dir[0] = crandom();
	dir[1] = crandom();
	dir[2] = crandom();

	if (dir[2] < 0)
		dir[2] = -dir[2];

	VectorNormalize2 (dir, dir);
}

int clientcount ()
{
	int c = -1;
	edict_t	*cur;
	cur = world;

	while (cur != NULL)
	{
		c ++;
		cur = G_Find (cur, FOFS(classname), "player");
	}

	return (c > 0 ? c : 1);

}

int monstercount ()
{
	int c = 0;
	edict_t	*cur;
	cur = world;

	while (cur != NULL)
	{
		if (cur->svflags & SVF_MONSTER)
			c ++;

		cur = nextent (cur);
	}

//	c = level.total_monsters - level.killed_monsters;

	return c;

}
/*
int ssound (char *sound1, char *sound2)
{
	int index = 0;

	if (pakon->value == 1)
		index = gi.soundindex (sound1);
	else
		index = gi.soundindex (sound2);

//	gi.bprintf (PRINT_HIGH, "%i\n", index);
	return index;
}

int setmodel (char *model1, char *model2)
{
	int index = 0;

	if (pakon->value == 1)
		index = gi.modelindex (model1);
	else
		index = gi.modelindex (model2);

//	gi.bprintf (PRINT_HIGH, "%i\n", index);
	return index;
}
*/
void tele_insert (edict_t * ent)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_EXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	gi.sound (ent, CHAN_AUTO, gi.soundindex("misc/tele1.wav"), 1, ATTN_NORM, 0);
}

void offsetset (vec3_t set, int i)
{
	if (i == 0)
	{
		VectorSet (set, 100, 0, 0);
	}
	else if (i == 1)
	{
		VectorSet (set, 66, 33, 0);
	}
	else if (i == 2)
	{
		VectorSet (set, 33, 66, 0);
	}
	else if (i == 3)
	{
		VectorSet (set, 0, 100, 0);
	}
	else if (i == 4)
	{
		VectorSet (set, -33, 66, 0);
	}
	else if (i == 5)
	{
		VectorSet (set, -66, 33, 0);
	}
	else if (i == 6)
	{
		VectorSet (set, -100, 0, 0);
	}
	else if (i == 7)
	{
		VectorSet (set, -66, -33, 0);
	}
	else if (i == 8)
	{
		VectorSet (set, -33, -66, 0);
	}
	else if (i == 9)
	{
		VectorSet (set, 0, -100, 0);
	}
	else if (i == 10)
	{
		VectorSet (set, 33, -66, 0);
	}
	else if (i == 11)
	{
		VectorSet (set, 66, -33, 0);
	}

}

int AddMonster (int meth, vec3_t loc, vec3_t dir)
{
	edict_t	* nm;
	vec3_t	spawn_origin, spawn_angles, check1, check2, offset;
	int killed = 0;
	int ran = rand()%10;
	int i = 0;

	nm = G_Spawn();
	if (meth == 0)
	{
		SelectSpawnPoint (nm, spawn_origin, spawn_angles);
//		spawn_origin[2] += 15;
	}
	else
	{
		VectorCopy (loc, spawn_origin);
		VectorCopy (dir, spawn_angles);
	}

	VectorCopy (spawn_origin, nm->s.origin);
	VectorCopy (spawn_angles, nm->s.angles);

	nm->s.origin[2] += 1;

	if (ran == 0)
	{
		SP_monster_gladiator (nm);
	}
	else if (ran == 1)
	{
		SP_monster_flyer (nm);
	}
	else if (ran == 2)
	{
		SP_monster_soldier_ss (nm);
	}
	else if (ran == 3)
	{
		SP_monster_gunner (nm);
	}
	else if (ran == 4)
	{
		SP_monster_infantry (nm);
	}
	else if (ran == 5)
	{
		SP_monster_tank (nm);
	}
	else if (ran == 6)
	{
		SP_monster_mutant (nm);
	}
	else if (ran == 7)
	{
		SP_monster_floater (nm);
	}
	else if (ran == 8)
	{
		SP_monster_berserk (nm);
	}
	else if (ran == 9)
	{
		SP_monster_parasite (nm);
	}

	VectorSet (nm->mins, -16, -16, -24);
	VectorSet (nm->maxs, 16, 16, 32);
	nm->groundentity = NULL;

	gi.unlinkentity (nm);

	nm->svflags |= SVF_MONSTER;
	
	killed = KillBox (nm);
	if (killed == false)
	{	// could't spawn in?
		if (nm->inuse)
			G_FreeEdict (nm);

//gi.bprintf (PRINT_HIGH, "removing monster\n");

		return false;
	}
	
	gi.linkentity (nm);

	tele_insert (nm);

//	gi.bprintf (PRINT_HIGH, "adding monster: %i, %i, %i\n", level.total_monsters, level.killed_monsters, monstercount());

	if (meth == 0)
	{
		ran = rand()%5;
		if (ran > 2)
			ran = 0;

		for (i=0;i<12;i++)
		{
			if (ran == 0)
			{
				i = 12;
				continue;
			}

			offsetset (offset, i);

			VectorAdd (offset, spawn_origin, offset);

			VectorCopy (offset, check1);
			VectorCopy (offset, check2);
			
			VectorAdd (check1, nm->mins, check1);
			VectorAdd (check2, nm->maxs, check2);

			if (!(gi.pointcontents(check1) & MASK_SOLID) && !(gi.pointcontents(check2) & MASK_SOLID))
			{
				AddMonster (1, offset, spawn_angles);
				ran -=1;
			}
		}
	}

	return true;
}

void tele_remove (edict_t * ent)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_EXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	gi.sound (ent, CHAN_AUTO, gi.soundindex("misc/tele1.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict (ent);

}

int rand_laser (int size)
{
	int ran = 0;
	int index = 0;

	if (size == 0)
	{
		ran = rand()%3;
		if (ran == 0)
			index = gi.soundindex ("powers/llaser1.wav");
		else if (ran == 1)
			index = gi.soundindex ("powers/llaser2.wav");
		else if (ran == 2)
			index = gi.soundindex ("powers/llaser3.wav");
	}
	if (size == 1)
	{
		ran = rand()%2;
		if (ran == 0)
			index = gi.soundindex ("powers/mlaser1.wav");
		else if (ran == 1)
			index = gi.soundindex ("powers/mlaser2.wav");
	}
	if (size == 2)
	{
		ran = rand()%3;
		if (ran == 0)
			index = gi.soundindex ("powers/hlaser1.wav");
		else if (ran == 1)
			index = gi.soundindex ("powers/hlaser2.wav");
		else if (ran == 2)
			index = gi.soundindex ("powers/hlaser3.wav");
	}
	if (size == 3)
	{
		ran = rand()%2;
		if (ran == 0)
			index = gi.soundindex ("powers/glaser1.wav");
		else if (ran == 1)
			index = gi.soundindex ("powers/glaser2.wav");
	}

	if (size == 4)
	{
		ran = rand()%3;
		if (ran == 0)
			index = gi.soundindex ("powers/ls_sw1.wav");
		else if (ran == 1)
			index = gi.soundindex ("powers/ls_sw2.wav");
		else if (ran == 2)
			index = gi.soundindex ("powers/ls_sw3.wav");
	}

	if (size == 5)
	{
		ran = rand()%3;
		if (ran == 0)
			index = gi.soundindex ("powers/ls_hit1.wav");
		else if (ran == 1)
			index = gi.soundindex ("powers/ls_hit2.wav");
		else if (ran == 2)
			index = gi.soundindex ("powers/ls_hit3.wav");
	}

	return index;
}

void b_blackhole_think (edict_t *ent)
{
	ent->s.frame ++;

	if (ent->s.frame == 11)
	{
		G_FreeEdict (ent);
		return;
	}

	ent->nextthink = level.time + FRAMETIME;
}

void make_ball (int type, vec3_t loc, edict_t *ent)
{
	edict_t		*ball;

	if (ent == NULL)
	{
		ball = G_Spawn();
		VectorCopy (loc, ball->s.origin);
	}
	else
	{
		ball = ent;
	}

	VectorCopy (loc, ball->s.origin);
	
	if (type == B_BLACKHOLE)
	{
		ball->s.modelindex = gi.modelindex ("models/super2/blackhole/tris.md2");
		ball->think = b_blackhole_think;
		ball->nextthink = level.time + FRAMETIME;
	}
	else if (type == B_FREEZE)
	{
		vec3_t	spin;
		vrandom (spin);

		ball->s.modelindex = gi.modelindex ("models/super2/freeze/tris.md2");
		ball->think = G_FreeEdict;
		ball->nextthink = level.time + 1;
		ball->movetype = MOVETYPE_NONE;
		ball->s.renderfx |= RF_FULLBRIGHT;
		VectorClear (ball->velocity);
		VectorScale (spin, rand()%100 + 50, ball->avelocity);
	}


	ball->s.frame = 0;
	gi.linkentity (ball);
}

void MV (edict_t * ent)
{
	AngleVectors (ent->client->v_angle, v_forward, v_right, v_up);

	return;
}

qboolean fromback (edict_t *self, edict_t *other)
{
	float	dot;
	vec3_t	forward1, forward2;

//gi.bprintf (PRINT_HIGH, "infront\n"); // SH	

	AngleVectors (self->s.angles, forward1, NULL, NULL);
	AngleVectors (other->s.angles, forward2, NULL, NULL);

//	VectorSubtract (other->s.origin, self->s.origin, vec);
//	VectorNormalize (vec);
	dot = DotProduct (forward1, forward2);
	
	if (dot > 0.3)
		return true;
	return false;
}

char *Green1 (char *in)
{
	int i = -1;
	static char out[50] = {0};

	for (i=0;i<50;i++)
		out[i] = 0;

	i = -1;
	do
	{
		i++;
		if (in[i] == 0)
		{
			out[i] = 0;
			break;
		}
		out[i] = in[i] | 0x80;
	} while  (in[i] != 0 && i < 50);

	return out;
}

char *Green2 (char *in)
{
	int i = -1;
	static char out[50] = {0};

	for (i=0;i<50;i++)
		out[i] = 0;

	i = -1;
	do
	{
		i++;
		if (in[i] == 0)
		{
			out[i] = 0;
			break;
		}
		out[i] = in[i] | 0x80;
	} while  (in[i] != 0 && i < 50);

	return out;
}

void MakeInvis (edict_t *ent)
{
	ent->s.modelindex = invis_index;
	ent->s.modelindex2 = 0;
	ent->s.skinnum = 0;
	ent->flags |= FL_NOTARGET;
}

void MakeVis (edict_t *ent)
{
	ent->s.modelindex = 255;
	ent->s.modelindex2 = 255;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->flags &= ~FL_NOTARGET;
}

void relocate (edict_t *ent)
{
	vec3_t	spawn_origin, spawn_angles;
	int i=0;

	SelectSpawnPoint (ent, spawn_origin, spawn_angles);
	ent->client->ps.pmove.origin[0] = spawn_origin[0]*8;
	ent->client->ps.pmove.origin[1] = spawn_origin[1]*8;
	ent->client->ps.pmove.origin[2] = spawn_origin[2]*8;

	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - ent->client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, ent->client->ps.viewangles);
	VectorCopy (ent->s.angles, ent->client->v_angle);
}

void make_tele (edict_t *ent)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_EXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	gi.sound (ent, CHAN_AUTO, gi.soundindex("misc/tele1.wav"), 1, ATTN_NORM, 0);
	ent->s.event = EV_PLAYER_TELEPORT;
}

edict_t *randplayer (edict_t *self)
{
	edict_t * ent;
	int	r, i;

	for (i=0;i<maxclients->value;i++)
	{
		r = rand()%(int)maxclients->value;
		ent = &g_edicts[r];
	
		if (ent->inuse 
			&& ent != self 
			&& ent->health > 0 
			&& !(ent->flags & FL_OBSERVER) 
			&& !OnSameTeam(ent, self))
		{
			return ent;
		}
	}

	return NULL;
}

// \SH