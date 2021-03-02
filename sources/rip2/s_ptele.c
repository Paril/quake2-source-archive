#include "g_local.h"

void BossTeleport(edict_t *ent)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BOSSTPORT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
}

qboolean findspawnpoint (edict_t *ent,edict_t *targent,vec3_t returnval)
{
	vec3_t loc = {0,0,0};
	vec3_t floor;
	vec3_t v;
	int i;
	int j = 0;
	int k = 0;
	int jmax = 1000;
	int kmax = (int)(jmax/2);
	qboolean found = false;
	trace_t tr;
	float distance;

	do 
	{
		if (found || j >= jmax || k >= kmax)
			break;

		j++;

		for (i = 0; i < 3; i++)
			loc[i] = rand() % (8192 + 1) - 4096;
		
		if (gi.pointcontents(loc) == 0)
		{
			VectorCopy(loc, floor);
			floor[2] = -4096;

			tr = gi.trace (loc, ent->maxs, ent->mins, floor, NULL, MASK_SOLID|MASK_WATER);
			k++;

			if (tr.contents & MASK_WATER)
				found = false;
			else
			{
				VectorCopy (tr.endpos, loc);
				loc[2] += ent->maxs[2] - ent->mins[2]; // make sure the entity can fit!
				found = (gi.pointcontents(loc) == 0 ? true : false);
				if (found)
				{
					VectorSubtract (loc, targent->s.origin, v);
					distance = VectorLength (v);
					if (distance > 400)
						found = false;
					
					tr = gi.trace (loc, ent->mins, ent->maxs, loc, NULL, MASK_PLAYERSOLID);
					
					if (tr.ent)
						found = false;
				}
			}
		}
		else
			found = false;
	}
	while (!found && j < jmax && k < kmax);

	if (!found && (j >= jmax || k >= kmax))
		return false;

	VectorCopy(loc,returnval);
	return true;
}

void Cmd_Teleport_f(edict_t *ent)
{
	int	n;
	edict_t	*player;
	vec3_t	spot;
	int iter = 0;
	qboolean found = false;


	for (n = 0; n < maxclients->value; n++)
	{
		player = g_edicts + n + 1;

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		if (player == ent)
			continue;

		if (rand()&1)
		{
			found = true;
			break;
		}
		
	}

	found = true;
	player = G_Find (NULL, FOFS(classname), "item_flag_team1");

	if (!found)
	{
		gi.centerprintf(ent,"No Other Players for teleport\n");
		return;
	}

	while ((findspawnpoint(ent,player,spot) == false) & (iter < 100000))
	{
		iter ++;
	}	

	if (iter == 100000)
	{
		gi.cprintf (ent,PRINT_HIGH,"No teleport spot available\n");
		return;
	}

	BossTeleport(ent);
	gi.unlinkentity (ent);	

	VectorCopy(spot,ent->s.origin);
	ent->s.event = EV_PLAYER_TELEPORT;
	gi.sound (ent, CHAN_ITEM, gi.soundindex ("world/blackhole.wav"), 1, ATTN_NORM, 0);
	gi.linkentity (ent);
}
