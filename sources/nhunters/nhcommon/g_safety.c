//============================================================================
// g_safety.c
//
// Safety mode
//
// Original Author: DingBat
//
//============================================================================

#include "g_local.h"

void setSafetyMode(edict_t *ent) {

  // Predator and marines may have seperate safety modes
  if (ent->isPredator) {

    if (getPredatorSafetyTime() <= 0 || !enable_predator_safety->value)
      return ;

    ent->safety_end = (int) level.time + (getPredatorSafetyTime() );

  }
  else {

    if (getMarineSafetyTime() <= 0 || !enable_marine_safety->value)
      return ;

    ent->safety_end = (int) level.time + getMarineSafetyTime() ;

  }

  // Make player invulnerable
  ent->takedamage = DAMAGE_NO ;

  ent->client->ps.stats[STAT_SAFETY] = 1 ;
  ent->safety = true ;

}

void clearSafetyMode(edict_t *ent) {

  if (!ent->safety) 
    return ;

  if (ent->isPredator) {
    if ((getPredatorSafetyTime() <= 0))
      return ;
  }
  else {
    if (getMarineSafetyTime() <= 0) 
      return ;
  }

  ent->takedamage = DAMAGE_YES ;
  ent->safety = false ;
  ent->client->ps.stats[STAT_SAFETY] = 0 ;

}

void validateMarineSafetyTime() {

  // marine_safety_time must be between 0 and 10
  if ((marine_safety_time->value < 0) ||
      (marine_safety_time->value > 10))
    gi.cvar_set("marine_safety_time", MARINE_SAFETY_TIME_DEFAULT );
}

int getMarineSafetyTime() {

  // Check if the cvar has changed.
  if (marine_safety_time->modified)
    validateMarineSafetyTime() ;

  return marine_safety_time->value ;
}

void validatePredatorSafetyTime() {

  // predator_safety_time must be between 0 and 10
  if ((predator_safety_time->value < 0) ||
      (predator_safety_time->value > 10))
    gi.cvar_set("predator_safety_time", PREDATOR_SAFETY_TIME_DEFAULT );
}

int getPredatorSafetyTime() {

  // Check if the cvar has changed.
  if (predator_safety_time->modified)
    validatePredatorSafetyTime() ;

  return predator_safety_time->value ;
}
