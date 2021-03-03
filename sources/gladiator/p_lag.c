//===========================================================================
//
// Name:				p_lag.c
// Function:		client lag
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1999-04-02
// Tab Size:		3
//===========================================================================

#include "g_local.h"
#include "bl_main.h"

#include "p_lag.h"

#define LAGHEAPSIZE		100

//delayed ucmd struct
typedef struct delayeducmd_s
{
	usercmd_t ucmd;
	vec3_t origin, v_angle;
	int sec, msec;
	struct delayeducmd_s *next;
} delayeducmd_t;
//heap with delayed ucmd structs
typedef struct delayeducmdheap_s
{
	struct delayeducmdheap_s *next;
} delayeducmdheap_t;
//
typedef struct clientlag_s
{
	delayeducmd_t *firstucmd;	//first ucmd to process by ClientThink
	delayeducmd_t *lastucmd;	//last ucmd to process
	int lag, lagvariance;		//the lag and the lag variance
	int delay;						//lagg in milli seconds
	int sec, msec;					//absolute client time
} clientlag_t;

clientlag_t clientlag[256];
delayeducmdheap_t *heap;
delayeducmd_t *freedelayeducmds;
int gameshutdown;

//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void Lag_ShutdownGame(void)
{
	gameshutdown = true;
} //end of the function Lag_ShutdownGame
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void Lag_FreeDelayeducmd(delayeducmd_t *ducmd)
{
	ducmd->next = freedelayeducmds;
	freedelayeducmds = ducmd;
} //end of the function Lag_FreeDelayeducmd
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void Lag_ExtendDelayeducmdHeap(void)
{
	delayeducmdheap_t *h;
	char *ptr;
	int i;

	h = gi.TagMalloc(sizeof(delayeducmdheap_t) + LAGHEAPSIZE * sizeof(delayeducmd_t), TAG_GAME);
	ptr = (char *) h + sizeof(delayeducmdheap_t);
	for (i = 0; i < LAGHEAPSIZE; i++)
	{
		Lag_FreeDelayeducmd((delayeducmd_t *) ptr);
		ptr += sizeof(delayeducmd_t);
	} //end for
	h->next = heap;
	heap = h;
	gi.dprintf("extended delayed ucmd heap\n");
} //end of the function Lag_ExtendDelayeducmdHeap
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
delayeducmd_t *Lag_AllocDelayeducmd(void)
{
	delayeducmd_t *ducmd;

	if (!freedelayeducmds)
	{
		Lag_ExtendDelayeducmdHeap();
	} //end if
	ducmd = freedelayeducmds;
	freedelayeducmds = freedelayeducmds->next;
	return ducmd;
} //end of the function Lag_AllocDelayeducmd
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
int Lag_MilliSecondsSubtract(int seca, int mseca, int secb, int msecb)
{
	int msec;

	msec = (seca-secb) * 1000;
	msec += mseca;
	msec -= msecb;
	return msec;
} //end of the function Lag_MilliSecondsSubtract
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void Lag_BeginGame(edict_t *ent)
{
	delayeducmd_t *firstucmd;
	int client;

	client = DF_ENTCLIENT(ent);
	while(clientlag[client].firstucmd)
	{
		firstucmd = clientlag[client].firstucmd;
		clientlag[client].firstucmd = clientlag[client].firstucmd->next;
		if (!clientlag[client].firstucmd) clientlag[client].lastucmd = NULL;
		Lag_FreeDelayeducmd(firstucmd);
	} //end if
	memset(&clientlag[client], 0, sizeof(clientlag_t));
} //end of the function Lag_BeginGame
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void Lag_StoreClientInput(edict_t *ent, usercmd_t *ucmd, vec3_t origin, vec3_t v_angle)
{
	delayeducmd_t *ducmd;
	int client;

	//
	client = DF_ENTCLIENT(ent);
	if (client < 0 || client > maxclients->value) gi.error("Lag_StoreClientInput: client out of range\n");
	//update the absolute client time
	clientlag[client].msec += ucmd->msec;
	while(clientlag[client].msec > 1000)
	{
		clientlag[client].msec -= 1000;
		clientlag[client].sec++;
		//the lag fluctuates ever second
		clientlag[client].delay = clientlag[client].lag + crandom() * clientlag[client].lagvariance;
		//set the client ping
		ent->client->ping = clientlag[client].delay;
		//
		if (clientlag[client].delay < 0) clientlag[client].delay = 0;
		else if (clientlag[client].delay > 2000) clientlag[client].delay = 2000;
	} //end if
	//
	ducmd = Lag_AllocDelayeducmd();
	//copy the ucmd
	memcpy(&ducmd->ucmd, ucmd, sizeof(usercmd_t));
	VectorCopy(origin, ducmd->origin);
	VectorCopy(v_angle, ducmd->v_angle);
	ducmd->sec = clientlag[client].sec;
	ducmd->msec = clientlag[client].msec;
	ducmd->next = NULL;
	//add the ucmd to the delayed list for this client
	if (clientlag[client].lastucmd) clientlag[client].lastucmd->next = ducmd;
	else clientlag[client].firstucmd = ducmd;
	clientlag[client].lastucmd = ducmd;
} //end of the function Lag_StoreClientInput
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
int Lag_GetClientInput(edict_t *ent, usercmd_t *laggeducmd, vec3_t origin, vec3_t v_angle)
{
	delayeducmd_t *firstucmd;
	int client, msec;

	//
	client = DF_ENTCLIENT(ent);
	if (client < 0 || client > maxclients->value) gi.error("Lag_GetClientInput: client out of range\n");
	//first delayed ucmd that might be processed
	firstucmd = clientlag[client].firstucmd;
	if (!firstucmd) return false;
	//if the first ucmd isn't delayed anough
	msec = Lag_MilliSecondsSubtract(clientlag[client].sec, clientlag[client].msec,
											firstucmd->sec, firstucmd->msec);
	//
	//gi.dprintf("msec = %d, delay = %d\n", msec, clientlag[client].delay);
	if (msec < clientlag[client].delay - 1) return false;
	//
	memcpy(laggeducmd, &firstucmd->ucmd, sizeof(usercmd_t));
	VectorCopy(firstucmd->origin, origin);
	VectorCopy(firstucmd->v_angle, v_angle);
	//remove the ducmd from the client lagg and free it
	clientlag[client].firstucmd = clientlag[client].firstucmd->next;
	if (!clientlag[client].firstucmd) clientlag[client].lastucmd = NULL;
	Lag_FreeDelayeducmd(firstucmd);
	return true;
} //end of the function Lag_GetClientInput
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void Lag_SetClientPing(edict_t *ent)
{
	int client;

	client = DF_ENTCLIENT(ent);
	//
	if (clientlag[client].delay > ent->client->ping)
	{
		ent->client->ping = clientlag[client].delay;
	} //end if
} //end of the function Lag_SetClientPing
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void Lag_SetClientLag(edict_t *ent, int lag)
{
	if (lag < 0) lag = 0;
	if (lag > 2000) lag = 2000;
	//set the client lag
	clientlag[DF_ENTCLIENT(ent)].lag = lag;
	clientlag[DF_ENTCLIENT(ent)].delay = lag;
	//
	gi.cprintf(ent, PRINT_HIGH, "lag set to %d\n", lag);
} //end of the function Lag_SetClientLag
//========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//========================================================================
void Lag_SetClientLagVariance(edict_t *ent, int lagvariance)
{
	if (lagvariance < 0) lagvariance = 0;
	if (lagvariance > 2000) lagvariance = 2000;
	//set the client lag variance
	clientlag[DF_ENTCLIENT(ent)].lagvariance = lagvariance;
	//
	gi.cprintf(ent, PRINT_HIGH, "lag variance set to %d\n", lagvariance);
} //end of the function Lag_SetClientLagVariance
