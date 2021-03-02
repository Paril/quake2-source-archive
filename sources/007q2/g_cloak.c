#include "g_local.h"


void Cmd_Cloak_f (edict_t *ent) 
{

	
	if (ent->client->cloak_framenum > level.framenum + 100)
		ent->client->cloak_framenum += 600;
	else
		ent->client->cloak_framenum = level.framenum + 600;
	
	
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BOSSTPORT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_ALL);
	ent->s.modelindex2 = 0;
}
