/*==============================================================================
The Weapons Factory -
Locate Friends
Original code by Gregg Reno
==============================================================================*/
#include "g_local.h"

void G_Spawn_DBTrail(int type, vec3_t start, vec3_t endpos );
void G_Spawn_Trails(int type, vec3_t start, vec3_t endpos, vec3_t origin );
extern int temp_ent_type;

/*
=================
Cmd_Friend_f
Set who is a friend 
syntax is "friend name
where 
  number = player number (showplayers)
  name = name of player to look up
=================
*/
void Cmd_Friend_f (edict_t *ent)
{
	char    *string;
	edict_t *e;
	edict_t *target;
	int i;

	if (!ent->client) return;

	string=gi.args();

	if (string[0] == 0)
	{
		ent->client->pers.friend_ent[0] = NULL;
		safe_cprintf (ent, PRINT_HIGH, "Friend setting cleared\n");
		return;
	}

	target = NULL;
	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		//match on netname?
		if (Q_stricmp(e->client->pers.netname, string) == 0)
		{
			if (!e->client->pers.hasfriends)
			{
				safe_cprintf (ent, PRINT_HIGH, "Sorry, %s has no friends\n", 
					e->client->pers.netname);
				return;
			}
			else
			{
				target = e;
				break;
			}
		}
	}

	if (!target)
	{
		safe_cprintf (ent, PRINT_HIGH, "Could not find player [%s]\n",string);
	}
	else
	{
		ent->client->pers.friend_ent[0] = target;
		safe_cprintf (ent, PRINT_HIGH, "Friend set to %s\n", string);
	}
}

void Cmd_ShowFriends_f (edict_t *ent)
{
	edict_t *friend_ent;

	if (!ent->client) return;

	friend_ent = ent->client->pers.friend_ent[0];
	if (!friend_ent)
	{
		safe_cprintf (ent, PRINT_HIGH, "No friend set. Use 'friend <name>' command.\n");
		return;
	}

	if (!friend_ent->client->pers.hasfriends)
	{
		safe_cprintf (ent, PRINT_HIGH, "%s has no friends!.\n",
			friend_ent->client->pers.netname);
		return;
	}

	if (!friend_ent->client)
	{
		safe_cprintf (ent, PRINT_HIGH, "Friend is not an active player.\n");
		return;
	}

	//Now do the blue light effect
	if (temp_ent_type == 1)
		G_Spawn_Trails(TE_BFG_LASER, ent->s.origin, friend_ent->s.origin, ent->s.origin );
	else if (temp_ent_type == 2)
		G_Spawn_Trails(TE_BUBBLETRAIL, ent->s.origin, friend_ent->s.origin, ent->s.origin );
	else if (temp_ent_type == 3)
		G_Spawn_Trails(TE_RAILTRAIL, ent->s.origin, friend_ent->s.origin, ent->s.origin );
	else if (temp_ent_type == 4)
		G_Spawn_Trails(TE_RAILTRAIL2, ent->s.origin, friend_ent->s.origin, ent->s.origin );
	else if (temp_ent_type == 5)
		G_Spawn_Trails(TE_BUBBLETRAIL2, ent->s.origin, friend_ent->s.origin, ent->s.origin );
	else
		G_Spawn_DBTrail(TE_DEBUGTRAIL, ent->s.origin, friend_ent->s.origin );

}


//If the friend is close, trace a line to them
void ShowCloseFriend_f (edict_t *ent)
{
	edict_t *friend_ent;
	vec3_t distance;
	int dist;

	if (!ent->client) return;

	//Skip if no friend defined
	friend_ent = ent->client->pers.friend_ent[0];
	if (!friend_ent)
		return;

	//Skip if the friend is not allowing it
	if (!friend_ent->client->pers.hasfriends)
		return;

	//only do this every second or so
	//(15 = low 4 bits.  Which means all 3 bits will only be set
	//every 16 times through.
	if ((level.framenum & 31) != 31)
		return;

	//Calc distance to friend
	distance[0] = ent->s.origin[0] - friend_ent->s.origin[0];
	distance[1] = ent->s.origin[1] - friend_ent->s.origin[1];
	distance[2] = ent->s.origin[2] - friend_ent->s.origin[2];
	dist=VectorLength(distance);
	if(dist <= 800)
	{
		if (temp_ent_type == 1)
			G_Spawn_Trails(TE_BFG_LASER, ent->s.origin, friend_ent->s.origin, ent->s.origin );
		else if (temp_ent_type == 2)
			G_Spawn_Trails(TE_BUBBLETRAIL, ent->s.origin, friend_ent->s.origin, ent->s.origin );
		else if (temp_ent_type == 3)
			G_Spawn_Trails(TE_RAILTRAIL, ent->s.origin, friend_ent->s.origin, ent->s.origin );
		else if (temp_ent_type == 4)
			G_Spawn_Trails(TE_RAILTRAIL2, ent->s.origin, friend_ent->s.origin, ent->s.origin );
		else if (temp_ent_type == 5)
			G_Spawn_Trails(TE_BUBBLETRAIL2, ent->s.origin, friend_ent->s.origin, ent->s.origin );
		else
			G_Spawn_DBTrail(TE_DEBUGTRAIL, ent->s.origin, friend_ent->s.origin );
	}
}


