//===========================================================================
// p_predator.c
//
// Predator functions.
//
// Original author: DingBat
//
//===========================================================================

#include "g_local.h"

static int saveHand ;

/*
===================
LookforPredator
majoon: this LOOKS to
see if there is a predator,
every 10 seconds.
===================
*/

int countPlayers(edict_t *exclude) {

  int count = 0 ;

  int i = 0 ;

  edict_t *tmp = NULL ;

  for (i = 1; i <= maxclients->value; i++) {

    tmp = &g_edicts[i] ;

    if ((tmp->inuse) &&
	(!tmp->isObserving)) {

      if (tmp != exclude)
	count++ ;
    }
  }

  return count ;

}

edict_t *getPlayer(int num, edict_t *exclude) {

  int i = 0;
  int found = 0 ;

  edict_t *tmp = NULL ;

  for (i = 1; i <= maxclients->value; i++) {

    tmp = &g_edicts[i] ;

    if ((tmp->inuse) &&
	(!tmp->isObserving)) {

      if (tmp != exclude)
	found++ ;

      if (found == num)
	return tmp ;
    }
  }

  return NULL ;

}

void quitPredator(edict_t *ent) {

  if (!ent->isPredator)
    return ;

  ent->isPredator = false ;
  ent->inWaiting = false ;
  randomPredator(ent) ;
}

void randomPredator(edict_t *exclude) {

  int i = 0 ;

  int count = 0 ;
  int new = 0 ;

  edict_t *player = NULL ;

  count = countPlayers(exclude) ;

  // No error if count = 0. 
  // Just continue.
  // May be a problem if excluded person is in game.
  if (count == 0) 
    return ;

  new = nhrand(1, count) ;

  player = getPlayer(new, exclude) ;

  switchPredator(player) ;
}


void lookForPredator(edict_t *exclude) {

  int i = 0 ;
  edict_t *tmp = NULL ;
  qboolean found = false ;

  for (i = 1; i <= maxclients->value; i++) {

    tmp = &g_edicts[i] ;

    if ((tmp->inuse) &&
	(!tmp->isObserving)) {

      if (tmp->isPredator) 
	return ;
    }
  } 

  randomPredator(NULL) ;

}

void 
initPredator(edict_t *ent) {

  gitem_t *item ;

  // Should make initial ammo a server variable.
  ent->client->pers.max_rockets        = predator_max_rockets->value ;
  ent->client->pers.max_slugs          = predator_max_slugs->value ;

  ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] =
    predator_start_rockets->value ;
  ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] =
    predator_start_slugs->value ;

  // Predator starts with rail gun and rocket launcher
  item = FindItem("Railgun");
  ent->client->pers.selected_item = ITEM_INDEX(item);
  ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

#if XATRIX

  ent->client->pers.max_cells          = predator_max_cells->value ;

  ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] =
    predator_start_cells->value ;

  // Predator also starts with ionripper
  item = FindItem("ionripper");
  ent->client->pers.selected_item = ITEM_INDEX(item);
  ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

#endif

  item = FindItem("Rocket Launcher");
  ent->client->pers.selected_item = ITEM_INDEX(item);
  ent->client->pers.inventory[ent->client->pers.selected_item] = 1;

  ent->client->pers.weapon = item;

  // Note! Order is important here.
  ent->client->pers.max_health = predator_start_health->value ;

  ent->client->pers.health = predator_start_health->value ;
  ent->health = predator_start_health->value ;

  if (ent->SuicidePredator) {
    ent->client->pers.health = ent->client->pers.health / 2 ;
    ent->health = ent->client->pers.health ;
  }

  // Needed?
  ent->client->pers.connected = true;
  
  // NH changes: Pred teleport.
  //  ent->client->pers.teleport_shots = max_teleport_shots->value ;
  ent->client->pers.teleport_shots = getMaxTeleportShots() ;
  ent->client->teleport_stored = false ;

  // Suicide flag.
  ent->SuicidePredator = false ; 

}


// Switch player to predator.
// Replacement for original initPredator.
void
switchPredator(edict_t *ent) {

//  gi.sound (ent, CHAN_VOICE,
//            gi.soundindex ("*death2.wav"), 1, ATTN_NORM, 0);

  // Switching player to predator.
  ent->isPredator = true ;

  //reset countdown temp variables
  level.be_pred_time_temp1 = 0;
  level.be_pred_time_temp2 = 0;
  level.be_pred_time_temp3 = 0;

  ent->client->ps.gunframe = 0 ;
  ent->client->ps.gunindex = 0 ;

  // Remove powerups so their effects dissapear.
  ent->client->quad_framenum = 0;
  ent->client->invincible_framenum = 0;
  ent->client->breather_framenum = 0;
  ent->client->enviro_framenum = 0;
  ent->client->IRgoggles_framenum = 0 ;

  // Reset fov.
  ent->client->ps.fov = atoi(Info_ValueForKey(ent->client->pers.userinfo,
					      "fov"));

  // Pretend to be dead?
  //  ent->deadflag = DEAD_DEAD ;

  // Don't let them take any damage.
  ent->takedamage = DAMAGE_NO ;

  // Start countdown.
  level.be_pred_time = level.time + 5 ;

  // Predator is now waiting for countdown to finish.
  ent->inWaiting = true ;


}

// Initialize predator.
void 
startPredator(edict_t *ent) {

  // First, let's clear the inventory
  memset(ent->client->pers.inventory, 0, sizeof(ent->client->pers.inventory)) ;
  
  // Then set up inventory.
  initPredator(ent) ;

  ent->client->newweapon = ent->client->pers.weapon ;

  // Force weapon up.
  ChangeWeapon(ent) ;

  // Movement
  ent->client->ps.pmove.pm_type = PM_NORMAL ;
  ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION ;
  ent->movetype = MOVETYPE_WALK ;
  ent->deadflag = DEAD_NO ;

  // No longer waiting to be predator.
  ent->inWaiting = false ;

  // Set predator skin.
  // Use new configurable predator skin.
  gi.configstring (CS_PLAYERSKINS+(ent-g_edicts-1),
                   va("%s\\%s", ent->client->pers.netname,
                      getLivePredatorSkin()) );
  
  // Hud icon.
  ent->client->ps.stats[STAT_PREDATOR] = gi.imageindex ("h_pred") ;

  // Teleport them to some random spawn point. 
  //randomTeleport(ent) ;

  PutClientInServer(ent); // to prevent bug of dead head on respawn

  //  Play teleport sound.
    gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/tele1.wav"), 1,
             ATTN_NORM, 0) ;
               

  // Annouce that they are the predator.
  //majoon: gotta TELL them they're pred!
//  gi.sound (ent, CHAN_VOICE,
//            gi.soundindex ("misc/newpred.wav"), 1, ATTN_STATIC, 0);
//	printf("Playing newpred.wav file\n");

  // Start them with their flashlight off. 
  ClearFlashlight(ent);
  
  // Set up the teleporter panic mode.
  ent->client->teleport_panic_end = (int) level.time +
    getPanicTime() ;
    //    teleport_panic_time->value ;
  
  // Start them in safety mode.
  // Set damage to yes, in case safety mode is off
  ent->takedamage = DAMAGE_YES ;
  setSafetyMode(ent) ;

}

void validatePredatorMaxRockets() {

  // predator_max_rockets must be between 0 and 200
  if ((predator_max_rockets->value < 0) ||
      (predator_max_rockets->value > 200))
    gi.cvar_set("predator_max_rockets", PREDATOR_MAX_ROCKETS_DEFAULT );
}

int getPredatorMaxRockets() {

  // Check if the cvar has changed.
  if (predator_max_rockets->modified)
    validatePredatorMaxRockets() ;

  return predator_max_rockets->value ;
}

void validatePredatorMaxSlugs() {

  // predator_max_slugs must be between 0 and 200
  if ((predator_max_slugs->value < 0) ||
      (predator_max_slugs->value > 200))
    gi.cvar_set("predator_max_slugs", PREDATOR_MAX_SLUGS_DEFAULT );
}

int getPredatorMaxSlugs() {

  // Check if the cvar has changed.
  if (predator_max_slugs->modified)
    validatePredatorMaxSlugs() ;

  return predator_max_slugs->value ;
}

void validatePredatorStartRockets() {

  // predator_start_rockets must be between 0 and 200
  if ((predator_start_rockets->value < 0) ||
      (predator_start_rockets->value > 200))
    gi.cvar_set("predator_start_rockets", PREDATOR_START_ROCKETS_DEFAULT );
}

int getPredatorStartRockets() {

  // Check if the cvar has changed.
  if (predator_start_rockets->modified)
    validatePredatorStartRockets() ;

  return predator_start_rockets->value ;
}

void validatePredatorStartSlugs() {

  // predator_start_slugs must be between 0 and 200
  if ((predator_start_slugs->value < 0) ||
      (predator_start_slugs->value > 200))
    gi.cvar_set("predator_start_slugs", PREDATOR_START_SLUGS_DEFAULT );
}

int getPredatorStartSlugs() {

  // Check if the cvar has changed.
  if (predator_start_slugs->modified)
    validatePredatorStartSlugs() ;

  return predator_start_slugs->value ;
}

void validatePredatorStartHealth() {

  // predator_start_health must be between 0 and 200
  if ((predator_start_health->value < 0) ||
      (predator_start_health->value > 200))
    gi.cvar_set("predator_start_health", PREDATOR_START_HEALTH_DEFAULT );
}

int getPredatorStartHealth() {

  // Check if the cvar has changed.
  if (predator_start_health->modified)
    validatePredatorStartHealth() ;

  return predator_start_health->value ;
}


