//===========================================================================
// g_cmd_teleport.c
//
// Predator teleporter.
// Originally coded by DingBat.
//
// Copyright (c), 1999 The BatCave. All Rights Reserved.
//===========================================================================

#include "g_local.h"
#include "g_cmd_observe.h"
#include "m_player.h"

// CVAR handling
void validateMaxTeleportShots() {

  // Teleport shots must be between 0 and 20
  if ((max_teleport_shots->value < 0) ||
      (max_teleport_shots->value > 20))
    gi.cvar_set("max_teleport_shots", MAX_SHOTS_DEFAULT) ;
}

int getMaxTeleportShots() {

  // Check if the cvar has changed.
  if (max_teleport_shots->modified)
    validateMaxTeleportShots() ;

  return max_teleport_shots->value ;
}

// CVAR handling
void validateTeleportHealth() {

  // Teleport health must between 0 and 200
  if ((teleport_health->value < 0) ||
      (teleport_health->value > 200))
    gi.cvar_set("teleport_health", TELEPORT_HEALTH_DEFAULT) ;
}

int getTeleportHealth() {

  // Check if the cvar has changed.
  if (teleport_health->modified)
    validateTeleportHealth() ;

  return teleport_health->value ;
}


// CVAR handling
void validatePanicTime() {

  // Teleport shots must between 0 and 60
  if ((teleport_panic_time->value < 0) ||
      (teleport_panic_time->value >60))
    gi.cvar_set("teleport_panic_time", PANIC_TIME_DEFAULT) ;
}

int getPanicTime() {

  // Check if the cvar has changed.
  if (teleport_panic_time->modified)
    validatePanicTime() ;

  return teleport_panic_time->value ;
}


// Store teleport location
void Cmd_Store_Teleport_f(edict_t *ent) 
{

  // NH changes: Pred Teleporter.
  if (!ent->isPredator) {
    gi.cprintf(ent, PRINT_HIGH, "Only predators can teleport\n") ;
    return ;
  }
  
  // Teleport enabled?
  if (getMaxTeleportShots() <= 0) {
    gi.cprintf(ent, PRINT_HIGH, "Teleporter not enabled\n") ;
    return ;
  }

  // If out of shots, say so.
  if (ent->client->pers.teleport_shots <= 0) {
    gi.centerprintf(ent, "No more teleport shots left\n") ;
    return ;
  }

  // Copy current position to teleport storage variables.
  VectorCopy(ent->s.origin, ent->client->teleport_origin) ;
  VectorCopy(ent->s.angles, ent->client->teleport_angles) ;
  
  // Signify we have stored the proper "coordinates".
  // Beam me up, Scotty!
  ent->client->teleport_stored = true ;

  gi.centerprintf(ent, "Teleport location stored!\nBeam me up, Scotty!\n") ;

}

// Actually perform teleport.
void Cmd_Load_Teleport_f(edict_t *ent)
{

  int i ;
  vec3_t spawn_angles = {0, 0, 0} ; // Alex see below
    
  
  // Can't teleport if you just got dead.
  if (ent->deadflag) 
    return ;
  
  // NH changes: Pred Teleporter.
  if (!ent->isPredator) {
    gi.cprintf(ent, PRINT_HIGH, "Only predators can teleport\n") ;
    return ;
  }

  // Teleport enabled?
  //  if (max_teleport_shots->value <= 0) {
  if (getMaxTeleportShots() <= 0) {
    gi.cprintf(ent, PRINT_HIGH, "Teleporter not enabled\n") ;
    return ;
  }

  if (ent->client->pers.teleport_shots <= 0) {
    gi.centerprintf(ent, "No more teleport shots left\n") ;
    return ;
  }

  // Point frames
  ent->client->anim_priority = ANIM_WAVE;
  ent->s.frame = FRAME_point01-1;
  ent->client->anim_end = FRAME_point12;

  // Panic mode.
  if (!ent->client->teleport_stored) {
    
    // Check for panic mode.
    if (ent->client->teleport_panic_end < (int) level.time) {
      gi.centerprintf(ent, "Panic mode disabled. Run!\n") ;
      return ;
    }

    // Panic mode.
    // Pred is hitting teleport without anchor point.
    // That will usually mean they have just respawned in
    // the middle of a bunch of marines.
    // Allow them to respawn elsewhere without penalty.
    randomTeleport(ent) ;
    
    return ;
  }

  // Can't teleport if health is below a certain level.
  //  if (ent->health < teleport_health->value) {
  if (ent->health < getTeleportHealth()) {
    gi.centerprintf(ent, "You don't have enough health to teleport\n") ;
    return ;
  }
	
  // Ok to teleport.
  gi.WriteByte(svc_temp_entity) ;
  gi.WriteByte(TE_BOSSTPORT) ;
  gi.WritePosition(ent->s.origin) ;
  gi.multicast(ent->s.origin, MULTICAST_PVS) ;

  gi.unlinkentity(ent) ;

  // Copy stored coordinates.
  VectorCopy(ent->client->teleport_origin, ent->s.origin) ;
  VectorCopy(ent->client->teleport_origin, ent->s.old_origin) ;
  //ent->s.origin[2] += 10 ;

  // Hold the player in place briefly.
  VectorClear(ent->velocity) ;
  ent->client->ps.pmove.pm_time = 160 >> 3 ;
  ent->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT ;

  // Teleport splash.
  ent->s.event = EV_PLAYER_TELEPORT ;
  
  // Set new angles 
  for (i = 0; i < 3; i++) 
    ent->client->ps.pmove.delta_angles[i] =
      ANGLE2SHORT(ent->client->teleport_angles[i] - 
		  ent->client->resp.cmd_angles[i]) ;

  // next 3 lines added by Alex - hope to fix tilt teleport problem
  ent->s.angles[PITCH] = 0;
  ent->s.angles[YAW] = spawn_angles[YAW];
  ent->s.angles[ROLL] = 0;

				
  VectorClear(ent->s.angles) ;
  VectorClear(ent->client->ps.viewangles) ;
  VectorClear(ent->client->v_angle) ;

  // Telefrag!
  KillBox(ent) ;

  gi.linkentity(ent) ;

  // Decrement teleport shots.
  ent->client->pers.teleport_shots-- ;
  
  return ;	
}

void
randomTeleport(edict_t *ent) {

  vec3_t spawn_origin = {0, 0, 0} ;
  vec3_t spawn_angles = {0, 0, 0} ;
  int i ;

  // Teleport entity to a random spawn point.
  ent->svflags &= ~SVF_NOCLIENT;

  // Find a spawn point.
  SelectSpawnPoint(ent, spawn_origin, spawn_angles) ;
  ent->client->ps.pmove.origin[0] = spawn_origin[0]*8 ;
  ent->client->ps.pmove.origin[1] = spawn_origin[1]*8 ;
  ent->client->ps.pmove.origin[2] = spawn_origin[2]*8 ;

  // Actual teleport.
  VectorCopy(spawn_origin, ent->s.origin) ;

  // New stuff.
  ent->s.origin[2] += 1 ;
  VectorCopy(ent->s.origin, ent->s.old_origin) ;

  // Set delta angle
  for (i = 0; i < 3; i++) {
    ent->client->ps.pmove.delta_angles[i] =
      ANGLE2SHORT(spawn_angles[i] - ent->client->resp.cmd_angles[i]) ;
  }

  ent->s.angles[PITCH] = 0;
  ent->s.angles[YAW] = spawn_angles[YAW];
  ent->s.angles[ROLL] = 0;
  VectorCopy (ent->s.angles, ent->client->ps.viewangles);
  VectorCopy (ent->s.angles, ent->client->v_angle);
  // End of new stuff.
  
  // Reset ent's respawn time.
  ent->client->respawn_time = level.time ;
  
  // add a teleportation effect
  ent->s.event = EV_PLAYER_TELEPORT;
  
  // Play teleport sound.
  gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/tele1.wav"), 1, 
	   ATTN_NORM, 0) ;
  
}				


