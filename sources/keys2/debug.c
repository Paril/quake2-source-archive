//Debug routine - create fake functions for bot code

 
#include "g_local.h"
#include "bot_procs.h"


void OptimizeRouteCache()
{
	int i;
	i = 1;
}

void	CalcRoutes(int node_index)
{
	int i;
	i = 1;
}

int	ClosestNodeToEnt(edict_t *self, int check_fullbox, int check_all_nodes)
{
	return 0;
}

float	PathToEnt(edict_t *self, edict_t *target, int check_fullbox, int check_all_nodes)
{
	return 0;
}

void	Debug_ShowPathToGoal(edict_t *self, edict_t	*goalent)
{
}

edict_t *matching_trail(vec3_t spot)
{
	return NULL;
}

void	AddTrailToPortals(edict_t	*trail)
{
}

int		GetGridPortal(float pos)
{
	return 0;
}

void	NodeDebug(char *fmt, ...)
{
}
void	CheckMoveForNodes(edict_t *ent)
{
}
void	CalcItemPaths(edict_t *ent)
{
}

/*void PlayerTrail_Init (void)
{
}

void PlayerTrail_Add (edict_t *self, vec3_t spot, edict_t *goalent, int nocheck, int calc_routes, int node_type)
{
}

void PlayerTrail_New (vec3_t spot)
{
} */

void WriteTrail ()
{
}

/*edict_t *PlayerTrail_PickFirst (edict_t *self)
{
	return NULL;
}

edict_t *PlayerTrail_PickNext (edict_t *self)
{
	return NULL;
}

edict_t	*PlayerTrail_LastSpot (void)
{
	return NULL;
}*/


