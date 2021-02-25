//===========================================================================
// g_cmd_observe.c
//
// Observer mode. 
// Originally coded by DingBat.
//
// Copyright (c), 1999 The BatCave. All Rights Reserved.
//===========================================================================

#include "g_local.h"
#include "g_cmd_observe.h"

void applyObservePenalties(edict_t *ent) {

  // Apply penalties to players entering observer mode.
  if (ent->isPredator) {

    if ((ent->client->resp.score - 1) >= getMinScore()) 
      ent->client->resp.score-- ;

    // Pick a new predator
    quitPredator(ent) ;
  }
  else {

    // If a marine has full health, or is dead, they can go to spectator
    // mode for free.
    if ((ent->health != 100) &&
	(ent->deadflag == DEAD_NO)) {
      if ((ent->client->resp.score - 1) >= getMinScore())
	ent->client->resp.score-- ;
    }
  }

}


void Start_Observe_f(edict_t *ent) 
{

  applyObservePenalties(ent) ;
  stuffcmd(ent,"spectator 1\n");
  ent->spectator_quick_switch = true;
  
}

void Cmd_Observe_f(edict_t *ent)
{
  Start_Observe_f(ent) ;
}

void Start_Play_f(edict_t *ent) {

  // Model check
  checkMarineSkin(ent, ent->client->pers.userinfo) ;

  stuffcmd(ent, "spectator 0\n");	
  ent->spectator_quick_switch = true;
  ent->ShowMOTD = false;

}

void Cmd_Play_f(edict_t *ent)
{
  Start_Play_f(ent) ;
}






