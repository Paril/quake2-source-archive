//===========================================================================
//
// Name:				p_lag.h
// Function:		client lag
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1999-04-02
// Tab Size:		3
//===========================================================================


//store the client ucmd input
void Lag_StoreClientInput(edict_t *ent, usercmd_t *ucmd, vec3_t origin, vec3_t v_angle);
//get the lagged client ucmd input
int Lag_GetClientInput(edict_t *ent, usercmd_t *laggeducmd, vec3_t origin, vec3_t v_angle);
//reset client lag at the beginning of a new game
void Lag_BeginGame(edict_t *ent);
//sets the client lag
void Lag_SetClientLag(edict_t *ent, int delay);
//sets the client lag variance
void Lag_SetClientLagVariance(edict_t *ent, int lagvariance);
//returns the ping of the client entity adjusted for the lag feature
void Lag_SetClientPing(edict_t *ent);
