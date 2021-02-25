//===========================================================================
// g_cmd_teleport.h
//
// Predator teleporter.
// Originally coded by DingBat.
//
// Copyright (c), 1999 The BatCave. All Rights Reserved.
//===========================================================================

#define MAX_SHOTS_DEFAULT "1"
#define TELEPORT_HEALTH_DEFAULT "0"  
#define PANIC_TIME_DEFAULT "15" 

// Cvar validation etc
void validateMaxTeleportShots() ;
int getMaxTeleportShots() ;

void validateTeleportHealth() ;
int getTeleportHealth() ;

void validatePanicTime() ;
int getPanicTime() ;

void Cmd_Store_Teleport_f(edict_t *ent) ;
void Cmd_Load_Teleport_f(edict_t *ent) ;
void randomTeleport(edict_t *ent) ;
