/*
============================
HOLY WARS: Heresy Mgm
============================
*/

#include "g_local.h"	// includes also hw_main.h
						// for prototype check

// ------------------ defined in g_ai.c
/* visible
 * returns 1 if the entity is visible to self, even if not infront ()
 */
extern qboolean visible (edict_t *self, edict_t *other);
/* infront
 * returns 1 if the entity is in front (in sight) of self
 */
extern qboolean infront (edict_t *self, edict_t *other);

#define HERESY_MALUS_MULT_KILLING       0.1
#define HERESY_BONUS_SAINT_KILLED       0.5

#define MELEE_INDX      0
#define NEAR_INDX       1
#define FAR_INDX        2

// ******************************************************
// Add heresy for a sigle action to the player
// parameters:
//      player  ,       heresy (it may be negative)
//
// This function assign the heresy for a single action:
// - single actions occurred into a range of 1.5 sec. are added as the same action:
//      it adds the higher heresy value (add the difference with the last),
//              adding a little mauls for the number of single actions
// ******************************************************

void HW_Heresy_Add(edict_t *player, float new_heresy)
 {
 if (level.time - player->client->last_heresy_time < 1.5)
        {
          // the last killing and this are the same action
          // so we punish for the worst case but add an little extra for each action
         if(new_heresy > player->client->last_heresy_value)
                {
                    // the new heresy for single action > the last added
                    //   -> add the new one
                player->client->heresy += new_heresy - player->client->last_heresy_value;
                player->client->last_heresy_value = new_heresy;
                }
         player->client->last_heresy_killed_num++; // another killed in this action
         player->client->heresy += HERESY_MALUS_MULT_KILLING;
         }
 else
        {
         // the last action is finished ...
         // firts check if the last action was a futile massacre ... the past returns !
        if (   player->client->last_heresy_killed_num > 1
            && player->client->last_heresy_value > 0.5)
                {
                // another bit of heresy on you !
                player->client->heresy +=
                       (float) player->client->last_heresy_killed_num
                        * HERESY_MALUS_MULT_KILLING;
                }

         // another action ... another judgement
                        // add new heresy to the player
        player->client->heresy += new_heresy;
                        // last heresy is set to the new one
        player->client->last_heresy_value = new_heresy;
                        // set the start time for the action
        player->client->last_heresy_time  = level.time;
                        // set the number of killed in this action
        player->client->last_heresy_killed_num  = 1;
        }
 }

 // **************
 // Sinner kill the Saint
 // Check if it's done during a multiple killing action
 // (player makes a massacre to kill the saint)
 // THEN it deletes all the blame !
 // Give an heresy bonus
 // **************
 void HW_Heresy_SaintKilled(edict_t *player)
 {
 if (level.time - player->client->last_heresy_time < 1.5)
        {
        // player makes a massacre to kill the saint ... BRAVO !
        // it subtracts heresy added in this action
        player->client->heresy -= (
                  (float) (player->client->last_heresy_killed_num - 1)
                  * HERESY_MALUS_MULT_KILLING
                  + player->client->last_heresy_value );
        }
 else
        {        // these are set for paranoid purpose
        player->client->last_heresy_value       = 0.0;
        player->client->last_heresy_time        = level.time;
        player->client->last_heresy_killed_num  = 1;
        }

        // a little gift !
 player->client->heresy -= HERESY_BONUS_SAINT_KILLED;

 if(player->client->heresy < 0)
        player->client->heresy = 0;     // no negative heresy ... please
 }

// ******************************************************
// Heresy calc. for a single action
// parameters:
//      dead    the killed
//      killer  the guy who makes the action
//      saint   the judge !
//      mod     the killing modality
//      range   vector of 3 distances: melee, near and far
// returns:
//      heresy for this action (from 0.0 to 1.0)
// ********************************************************
float HW_Heresy_SingleAction(edict_t *dead, edict_t *killer, edict_t *saint, int mod, float range[])
  {
  qboolean      visible_s_d;            // true if saint can see the killed
  qboolean      visible_s_k;            // true if saint can see the killer
  float         boom_radius;            // dead->saint radius to set heresy for explosions
  float         dist_saint_killer;      // distance between saint and killer
  float         dist_saint_dead;        // distance between saint and killed
  float         dist_killer_dead;       // distance between killer and killed
  vec3_t        aux_dir;                // auxiliary vector for distance calc.
  float         heresy = 0.0;           // heresy to add
  const float   low_heresy  = 0.3;
  const float   med_heresy  = 0.5;
  const float   high_heresy = 0.7;
  const float   heresy_malus= 0.2;


        // ---- 1st step:
        //      calc. distance between the saint and the killed (dead)
  VectorSubtract (saint->s.origin, dead->s.origin, aux_dir);
  dist_saint_dead = VectorLength(aux_dir);

        // ---- 2nd step:
        //      if the killed is too near at the saint: it is a mistake -> NO eresy
  if (dist_saint_dead <= range[MELEE_INDX])
        return 0.0;         // the saint is too near at the killed to punish the killer

        // ---- 3rd step:
        //      calc. other distances and saint visibility
    // distance between the saint and the killer
  VectorSubtract (saint->s.origin, killer->s.origin, aux_dir);
  dist_saint_killer = VectorLength(aux_dir);

    // distance between the killer and the killed
  VectorSubtract (killer->s.origin, dead->s.origin, aux_dir);
  dist_killer_dead = VectorLength(aux_dir);

   // calc. boom range
  boom_radius = range[NEAR_INDX];

    // saint visibility
  visible_s_d = visible(saint, dead);           // true if saint can see the killed
  visible_s_k = visible(saint, killer);         // true if saint can see the killer

        // ---- 4th step:
        //      set eresy according with the kind of killing
  switch (mod)
        {
      // is it a direct shot ?
  case MOD_SHOTGUN:
  case MOD_SSHOTGUN:
  case MOD_MACHINEGUN:
  case MOD_CHAINGUN:
  case MOD_RAILGUN:
        // ----- Direct shot cases
        //       if  the killer is looking at the saint
        //              it's not so bad: give an extra bonus for short distances
        //       else
        //              if the saint can see the scene or the killed is too near at the killer
        //                      this may be an error  ....
        //                      bonus if chaingun (spreaded fire)
        //              else
        //                      SHAME on the killer
        if ( infront(killer, saint) )
               { // saint is in front of the killer
               if ( dist_killer_dead < range[MELEE_INDX]
                 || dist_saint_dead  < range[NEAR_INDX]  )
                        return low_heresy;   // errare humanum est !
               else
                        return med_heresy;
               }
        else
                {
                    // the killer doesn't look at the saint
                if (  visible_s_d || visible_s_k
                   || dist_killer_dead <= range[MELEE_INDX])
                        {
                            // the saint can see the scene or the victim is in contact with killer
                            // if killer has a chaingun AND the saint is near dead -> medium
                        if(mod == MOD_CHAINGUN && dist_saint_dead < range[NEAR_INDX])
                                return med_heresy;   // may be an error
                        else
                                return high_heresy;  // an error ? I don't think so !
                        }
                else
                        return 1.0;   // ... are YOU joking ? MAX eresy to you !
                }
        break;


        // ----- BOOM cases (explosions)
        //       there's no problem:
        //              the heresy is proportional to the distance between saint and the victim
        //      range BONUS for BFG effects
        //      range EXTRA BONUS for grenades (because I like it!)
  case MOD_HG_SPLASH:
  case MOD_G_SPLASH:
        boom_radius *= 1.5;     // increase the range for grenades
  case MOD_BFG_EFFECT:
        boom_radius *= 1.5;     // increase the range for BFG effects AND grenades
  case MOD_R_SPLASH:
  case MOD_BFG_BLAST:
  case MOD_HELD_GRENADE:        // for stupids ... no paradise
        if(boom_radius < 100.0)
                boom_radius = 100.0;    // paranoia ... no division by 0, please
        heresy = dist_saint_dead / boom_radius;        // set heresy according to radius
        break;

        // ----- delayed weapon cases
        //      set heresy according to the saint point of view:
        //        low :   saint can see the killer
        //        med.:   saint cannot see the killer but can see the victim
        //        high:   saint cannot see the scene
        //      MALUS on dead's distances
        //              if killed NOT near at the saint AND NOT at conctact with killer
        //      MALUS on saint distances
        //              if saint is far from killed or killer
  case MOD_GRENADE:     // delayed
  case MOD_ROCKET:
  case MOD_HANDGRENADE:
  case MOD_BLASTER:
  case MOD_HYPERBLASTER:
            // saint visibility
        if( visible_s_k)        // saint can see the killer
                heresy = low_heresy;
        else                    // saint canNOT see the killer
                // if saint can see victim then medium else high
                heresy =  ( visible_s_d )  ? med_heresy : high_heresy;
            // victim distance malus
        if (dist_saint_dead > range[NEAR_INDX] && dist_killer_dead > range[MELEE_INDX])
                heresy += heresy_malus;
            // saint distance malus
        if (dist_saint_dead > range[FAR_INDX] || dist_saint_killer > range[FAR_INDX])
                heresy += heresy_malus;
        break;

  case MOD_BFG_LASER:
        // ----- BFG laser case
        //      BFG is a powerful weapon ... only smart guys should use it
        //      if the saint  can see the scene
        //      then    minimun heresy
        //      else
        //              if (saint can see the victim) then medium heresy else high heresy
        //              + heavy MALUS if saint is far from killer or killed
        if( visible_s_k)        // saint can see the killer
                return low_heresy;      // no other malus ... killer is a smart guy
        else                    // saint canNOT see the killer
                // if saint can see victim then medium else high
                heresy =  ( visible_s_d )  ? med_heresy : high_heresy;
        if (dist_saint_dead > range[FAR_INDX] || dist_saint_killer > range[FAR_INDX])
                heresy += heresy_malus * 2;
        break;
  case MOD_TELEFRAG:            // telefrag is not a sin !
  default:
       break;
       }

  // returns heresy saturates at max heresy
  return (heresy > 1.0) ? 1.0 : heresy;
  }

void HW_Heresy_MainCalc(edict_t *dead, edict_t *killer, int mod)
  {

  edict_t      *saint;
  int           killer_ping;            // the ping of the killer
  float         range[3];

        // ---- 1st step: it finds the saint
  saint = NULL;
  while ((saint = G_Find (saint, FOFS(classname), "player")) != NULL)
        {
        if (saint->client != NULL && saint->client->plstatus == SAINT)
                break;
        }
  if (saint == NULL) return;    // there's no saint !

        // ---- 2nd step: it loads ranges for heresy calculations
  range[MELEE_INDX] = hw_melee->value;      // too near
  range[NEAR_INDX]  = hw_near->value;       // near
  range[FAR_INDX]   = hw_far->value;        // far

        // ---- 3rd step: this gives a little help for high pings
        //                      (it increases proximity ranges)
  killer_ping = killer->client->ping;   // loads killer ping
  if(killer_ping > 1000)                // if too high saturates at 1000
        killer_ping = 1000;
  if(killer_ping > 200)         // high ping
        {
        float gain_for_high_ping = 1.0 + (float) killer_ping / 1000.0;
        range[MELEE_INDX] *= gain_for_high_ping;   // proximity distances corrected for high ping
        range[NEAR_INDX]  *= gain_for_high_ping;
        }

  HW_Heresy_Add( killer, HW_Heresy_SingleAction(dead, killer, saint, mod, range));

        // Check if the guy must become an heretic:
  if (killer->client->heresy >= hw_heresy->value)
         HW_BecomeHeretic(killer);
  }





