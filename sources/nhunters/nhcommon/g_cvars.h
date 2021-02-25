//===========================================================================
// g_cmd_teleport.h
//
// Predator teleporter.
// Originally coded by DingBat.
//
// Copyright (c), 1999 The BatCave. All Rights Reserved.
//===========================================================================

#define MAXRATE_DEFAULT "0"
#define MAX_MARINE_KILL_DEFAULT "5"
#define PENALTY_DEFAULT "0"
#define MAX_TIME_DEFAULT "300"
#define MIN_SCORE_DEFAULT "-10"
#define MAPLIST_SMALL_MAX_DEFAULT "4"
#define MAPLIST_MEDIUM_MAX_DEFAULT "8"

// Cvar validation etc
void validateMaxRate() ;
int getMaxRate() ;

void validateMaxMarineKill() ;
int getMaxMarineKill() ;

void validateMaxTime() ;
int getMaxTime() ;

void validateMinScore() ;
int getMinScore() ;

void validateMaplistMediumMax();
int getMaplistMediumMax();
