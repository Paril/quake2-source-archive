//===========================================================================
// g_nh.c
//
// NH related functions
//
// Originally code by: DingBat
//
//===========================================================================

#include "g_local.h"
#include <stdlib.h>
#include <time.h>

// General random function.
int nhrand(int low, int high) {

  int result = 0 ;
  float range = (high - low) + 1.0 ;
  float tmp = 0.0 ;

  srand((unsigned int) time(NULL)) ;

  tmp = range * rand()/(RAND_MAX + 1.0) ;
  result = low + (int) tmp ; 

  return result ;

}


void
NH_PreConnect(edict_t *ent) { 

  // NH players always start in spectator mode
  ent->isObserving = true ;
  ent->client->pers.spectator = true;
  ent->client->resp.spectator = false; 
  Info_SetValueForKey(ent->client->pers.userinfo, "spectator", "1");
  stuffcmd(ent, "spectator 1\n") ;
  ent->isFirstConnect = 1;

}

void
NH_PostConnect(edict_t *ent) {

  // Do we need these?
  // ent->spectator_quick_switch = true ;
  ent->SuicidePredator = false ;

  // Cheat checks
  stuffcmd(ent, "set gl_dynamic 1; set sw_drawflat 0\n") ; 

  ent->isCheating = false ;

  // Skin setup.
  checkMarineSkin(ent, ent->client->pers.userinfo) ;

  // Clear the flashlight
  ClearFlashlight(ent) ;

}
