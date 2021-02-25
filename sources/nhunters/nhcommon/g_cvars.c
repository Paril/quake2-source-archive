//===========================================================================
// g_cvars.c
//
// Predator teleporter.
// Originally coded by BatMax.
//
// Copyright (c), 1999 The BatCave. All Rights Reserved.
//===========================================================================

#include "g_local.h"
#include "g_cvars.h"

// CVAR handling
void validateMaxRate() {

  // maxrate must be between 2000 and 65535
  if ((maxrate->value < 2000) ||
      (maxrate->value > 65535))
    gi.cvar_set("maxrate", MAXRATE_DEFAULT) ;
}

int getMaxRate() {

  // Check if the cvar has changed.
  if (maxrate->modified)
    validateMaxRate() ;

  return maxrate->value ;
}

// CVAR handling
void validateMaxMarineKill() {

  // maxmarinekill must be between 0 and 20
  if ((maxmarinekill->value < 0) ||
      (maxmarinekill->value > 20))
    gi.cvar_set("maxmarinekill", MAX_MARINE_KILL_DEFAULT) ;
}

int getMaxMarineKill() {

  // Check if the cvar has changed.
  if (maxmarinekill->modified)
    validateMaxMarineKill() ;

  return maxmarinekill->value ;
}

void validatePenalty() {

  if ((penalty_threshold->value < 0) ||
      (penalty_threshold->value >= getMaxMarineKill()))
    gi.cvar_set("penalty_threshold", PENALTY_DEFAULT) ;

}

int getPenalty() {

  if (penalty_threshold->modified) 
    validatePenalty() ;

  return penalty_threshold->value ;
}


void validateMaxTime() {

  // maxtime must be between 0 and 1200 (seconds)
  if ((maxtime->value < 0) ||
      (maxtime->value > 1220))
    gi.cvar_set("maxtime", MAX_TIME_DEFAULT) ;
}

int getMaxTime() {

  // Check if the cvar has changed.
  if (maxtime->modified)
    validateMaxTime() ;

  return maxtime->value ;
}

void validateMinScore() {

  // minscore must be between 0 and -10
  if ((minscore->value < -10) ||
      (minscore->value > 0))
    gi.cvar_set("minscore", MIN_SCORE_DEFAULT );
}

int getMinScore() {

  // Check if the cvar has changed.
  if (minscore->modified)
    validateMinScore() ;

   return minscore->value ;
}

void validateMaplistSmallMax() {

  // sv_maplist_small_max must be between 0 and 999
  if ((sv_maplist_small_max->value < 0) ||
      (sv_maplist_small_max->value > 999))
    gi.cvar_set("sv_maplist_small_max", MAPLIST_SMALL_MAX_DEFAULT );
}

int getMaplistSmallMax() {

  // Check if the cvar has changed.
  if (sv_maplist_small_max->modified)
    validateMaplistSmallMax() ;

  return sv_maplist_small_max->value ;
}

void validateMaplistMediumMax() {

  // sv_maplist_medium_max must be between 0 and 999
  if ((sv_maplist_medium_max->value < 0) ||
      (sv_maplist_medium_max->value > 999))
    gi.cvar_set("sv_maplist_medium_max", MAPLIST_MEDIUM_MAX_DEFAULT );
}

int getMaplistMediumMax() {

  // Check if the cvar has changed.
  if (sv_maplist_medium_max->modified)
    validateMaplistMediumMax() ;

  return sv_maplist_medium_max->value ;
}
 


