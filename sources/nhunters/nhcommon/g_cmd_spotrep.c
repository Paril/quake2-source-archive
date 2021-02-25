//============================================================================
// g_cmd_spotrep.c
//
// Marine spot report command.
//
// Original code by: DingBat
//
// Copyright(c) 1999, The BatCave, All Rights Reserved.
//============================================================================

#include "g_local.h"
#include "g_cmd_overload.h"                                                    

void Cmd_SpotRep_f(edict_t *ent) {

  char message[100] ;

  // Spot report only available to marines.
  if ((ent->isPredator) || (ent->isObserving) || (ent->deadflag))  
    return ;

  // Talking takes you out of safety mode.
  clearSafetyMode(ent) ;

  strcpy(message, "** predator seen from %l **") ;

  CTFSay_Team(ent, message) ;

}
