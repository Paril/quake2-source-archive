//============================================================================
// g_cmd_setup.c
//
// Player control setup
//
// Copyright(c) 1999, The BatCave, All Rights Reserved.
//============================================================================ 

#include "g_local.h"
#include "g_lightning.h"

void validateLightShowInterval() {

  // Light show interval must be between 0 and 9999
  if ((light_show_interval->value < 0) ||
      (light_show_interval->value > 9999)) {
    gi.cvar_set("light_show_interval", LIGHT_SHOW_INTERVAL_DEFAULT) ;
  }

}

int getLightShowInterval() {

  // Light show interval must be greater than 0
  if (light_show_interval->modified) 
    validateLightShowInterval() ;

  return light_show_interval->value ;

}


void Set_Lightning_Effect(int effect) {

//  edict_t *ent;
//  int i;
  
  char light[32] ;
  
  switch (effect) {
  case 1:
    strcpy(light, "daaaaaaaaaaaaaaaaaaa") ;
    break;
  case 2:
    strcpy(light, "faaaaaaaaaaaaaaaaaaa") ;
    break;
  case 3:
    strcpy(light, "daammeeaaaaaaaaaaaaa");
    break;
  case 4:
    strcpy(light, "daaaeaaaaaaaaaaaaaaa");
    break;
  case 5:
    strcpy(light, "zzzeeaaaaaaaaazzeeaa");
    break;
  default:
    strcpy(light, "daaaaaaaaaaaaaaaaaaa") ;
    break ;
  }

  gi.configstring(CS_LIGHTS+0, light) ;
  level.lightningEffect = effect ; 

} 

void Lightning_On(void) {

  // If light show not enabled, don't bother.
  if (!enable_light_show->value) 
    return ; 

  // If light show already on, don't bother.
  if (level.lightningOn)
    return ;

  Set_Lightning_Effect(nhrand(1, 5));
	
  level.lightningOn = true;
  level.lightningTimeout = (int) level.time + (1) ;
} 

void Lightning_Off(void) {

  edict_t *ent;
  int i ;
  char sound[64] ;

  // If light show not enabled, don't bother.
  if (!enable_light_show->value) 
    return ;

  gi.configstring(CS_LIGHTS+0, "a");

  // Generate a new time for lightning.
  level.lightningOn = false ;
  level.nextLightning = (int) level.time + nhrand(1, getLightShowInterval()) ;

  switch (level.lightningEffect) {
  case 1:
    strcpy(sound, "world/battle1.wav") ;
    break;
  case 2:
   strcpy(sound, "world/battle2.wav") ;
    break;
  case 3:
    strcpy(sound, "world/battle3.wav") ;
    break;
  case 4:
    strcpy(sound, "world/battle4.wav") ;
    break;
  case 5:
    strcpy(sound, "world/battle5.wav") ;
    break;
  default:
    strcpy(sound, "world/battle1.wav") ;

    break ;
  }
  for_each_player(ent, i){
    gi.sound (ent, CHAN_AUTO,
	      gi.soundindex(sound),1,ATTN_STATIC, 0);
  }


} 
