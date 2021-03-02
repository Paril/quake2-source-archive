//===========================================================================
//
// Name:				p_observer.h
// Function:		observer mode
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1998-01-12
// Tab Size:		3
//===========================================================================

#define CAMFL_NOSMOOTHING		1
#define CAMFL_FIXED				2
#define CAMFL_AUTOCAM			4
#define CAMFL_CHASECAM			8
#define CAMFL_NAME				16

float AngleDifference(float ang1, float ang2);
int DoObserver(edict_t *ent, usercmd_t *ucmd);
void ClientCycleCamera(edict_t *ent);
void ClientSetCamera(edict_t *ent);
void ClientToggleObserver(edict_t *ent);
void ClientSetViewAngles(edict_t *ent, vec3_t ang, vec3_t realang);
void ClientToggleCameraFixed(edict_t *ent);
void ClientObserverHelp(edict_t *ent);
qboolean ClientObserverCmd(char *cmd, edict_t *ent);
