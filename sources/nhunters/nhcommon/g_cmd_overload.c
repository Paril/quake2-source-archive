//============================================================================
// g_cmd_overload.c
//
// Predator rocket overload command toggle.
//
// Original code by: DingBat
//
// Copyright(c) 1999, The BatCave, All Rights Reserved.
//============================================================================

#include "g_local.h"
#include "g_cmd_overload.h"

void Cmd_Overload_f(edict_t *ent) {

  // Can't do it unless you're the predator or dead or inWaiting (to be pred).
  if (!ent->isPredator || ent->deadflag || ent->inWaiting)   
    return ;

  // Can't do it if overloads are disabled.
  if (!enable_predator_overload->value)
    return ;

  // Toggle the overload flag.
  ent->client->weapon_overload = !ent->client->weapon_overload ;

  if (ent->client->weapon_overload)
    // Play overload sound
    gi.sound(ent, CHAN_AUTO,
	   gi.soundindex("world/fusein.wav"), 1, ATTN_STATIC, 0);  
  else 
    gi.sound(ent, CHAN_AUTO,
	     gi.soundindex("world/fuseout.wav"), 1, ATTN_STATIC, 0);  


  return ;
}

void ClearOverload(edict_t *ent) {

  ent->client->ps.stats[STAT_OVERLOAD] = 0; // Alex
  ent->client->weapon_overload = false ;

  return ;
}

void validatePredatorOverloadCost() {

  // predator_overload_cost must be between 0 and 10
  if ((predator_overload_cost->value < 0) ||
      (predator_overload_cost->value > 10))
    gi.cvar_set("predator_overload_cost", PRED_OVERLOAD_COST_DEFAULT );
}

int getPredatorOverloadCost() {

  // Check if the cvar has changed.
  if (predator_overload_cost->modified)
    validatePredatorOverloadCost() ;

  return predator_overload_cost->value ;
}

