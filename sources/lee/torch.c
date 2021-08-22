#include "g_local.h"

void make_light (edict_t *self, vec3_t start)
{
	edict_t	*bolt;

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (self->s.angles, bolt->s.angles);
	VectorScale (self->s.angles, 0, bolt->velocity);

	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_NOT;

	bolt->s.effects = 64;
	
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);

	bolt->s.modelindex = gi.modelindex ("sprites/torchC.sp2");
		//gi.modelindex ("models/objects/flash/tris.md2");
	bolt->owner = self;
	bolt->classname = "light_s";
	bolt->targetname = self->client->pers.netname;

	gi.linkentity (bolt);

}	

//delete light
void del_light(edict_t *ent)
{
	edict_t *player_light;

	player_light = G_Find (NULL, FOFS(targetname), ent->client->pers.netname);

	if (player_light != NULL)
		G_FreeEdict(player_light);

}

//spot light (movment)
void light(edict_t *ent)
{
	vec3_t move, forward, temp;
	edict_t *player_light;
	trace_t	tr; 

	if (!darkmatch->value)
		return;

	//search for the this players light
	player_light = G_Find (NULL, FOFS(targetname), ent->client->pers.netname);

	if ((ent->health < 1) || (ent->solid == 0))
	{  
		if (player_light != NULL)
			player_light->s.effects = 0;
		return;
	}
	
	//go forward alot
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);	
	VectorScale (forward, 1000, move);
	VectorAdd (move, ent->s.origin, temp);

	//trace the line
	tr = gi.trace (ent->s.origin, NULL, NULL, temp, ent, MASK_ALL);

	//where did we stop?
	VectorCopy(tr.endpos, move);			

	//move light	
	if (player_light != NULL)
	{
		VectorCopy(move, player_light->s.origin);
	
		//always keep bot lights on
		if (strcmp(ent->classname, "bot") == 0)
			player_light->s.effects = 64;
	}
}

void toggle_torch(edict_t *ent)
{
	edict_t *player_light;

	if (!darkmatch->value)
		return;

	if (strcmp(ent->classname, "bot") == 0)
		return;

	player_light = G_Find (NULL, FOFS(targetname), ent->client->pers.netname);

	//dead
	if (ent->health < 1) 
	{
		player_light->s.effects = 0;
		return;
	}


	
	//toggle light on/off
	if (player_light != NULL)
	{
		
		if (player_light->s.effects > 0)
		{
			player_light->s.effects = 0;
			if (strcmp(ent->classname, "player") == 0)
				gi.cprintf (ent, PRINT_HIGH, "Torch light off\n");
		}
		else
		{
			player_light->s.effects = EF_HYPERBLASTER;
			if (strcmp(ent->classname, "player") == 0)
				gi.cprintf (ent, PRINT_HIGH, "Torch light on\n");
		}

	}
}