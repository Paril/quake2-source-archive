#include "g_local.h"
// Same as normal match but end differently
// end on one remaining player
// That it respawn everyone into warmup and 
void RespawnAllPlayers ()
{
	edict_t *ent;
	int i;
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse)
			continue;

		if ((ent->health <= 0)||(ent->client->resp.tank))
		{
			ent->client->resp.tank = false;
			if(ent->client)
			MatrixRespawn (ent, ent);
			else
			player_die (ent, ent, ent, 100000, vec3_origin);
			// don't even bother waiting for death frames
			ent->deadflag = DEAD_DEAD;
			respawn (ent);
			ent->client->resp.score++;
		}
	}
}
void MatrixStartTank ()
{
}

void MatrixTankThink ()
{
	if (!tankmode->value)
		return;
	
}
