#include "g_local.h"

/*===========================================
 -= C H A S E =-           -= C A M E R A =-
 By James Williams for sATaN's BoX of FuN Edition 2
 ==========================================*/

void ChasecamTrack (edict_t *ent);
void CheckChasecam_Viewent (edict_t *ent);

/*  The ent is the owner of the chasecam  */
void ChasecamStart (edict_t *ent)
{
   /* This creates a tempory entity we can manipulate within this
    * function */

	edict_t      *chasecam;
        
    /* Tell everything that looks at the toggle that our chasecam is on
    * and working */

    /* Make out gun model "non-existent" so it's more realistic to the
    * player using the chasecam */

	ent->client->ps.gunindex = 0;
    
    chasecam = G_Spawn ();
	chasecam->owner = ent;
    chasecam->solid = SOLID_NOT;
    chasecam->movetype = MOVETYPE_FLYMISSILE;

    ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION; // this turns off Quake2's inclination to predict where the camera is going,
    ent->svflags |= SVF_NOCLIENT; // this line tells Quake2 not to send the unnecessary info about the camera to other players

    /* Now, make the angles of the player model, (!NOT THE HUMAN VIEW!) be
    * copied to the same angle of the chasecam entity */

    VectorCopy (ent->s.angles, chasecam->s.angles);
        
    /* Clear the size of the entity, so it DOES technically have a size,
    * but that of '0 0 0'-'0 0 0'. (xyz, xyz). mins = Minimum size,
    * maxs = Maximum size */

    VectorClear (chasecam->mins);
    VectorClear (chasecam->maxs);
        
    /* Make the chasecam's origin (position) be the same as the player
    * entity's because as the camera starts, it will force itself out
    * slowly backwards from the player model */

    VectorCopy (ent->s.origin, chasecam->s.origin);
    
    chasecam->classname = "chasecam";
	chasecam->prethink = ChasecamTrack;

	ent->client->chasecam = chasecam;
    ent->client->oldplayer = G_Spawn();
    ent->client->chasetoggle = 1;
}

void ChasecamRestart (edict_t *ent)
{
     /* If the player is dead, the camera is not wanted... Kill me and stop
     * the function. (return;) */

     if (ent->owner->health <= 0)
	 {
		 G_FreeEdict (ent);
		 return;
	 }

     /* If the player is NOT under water, and not dead, then he's going to
     * want his camera back. Create a new camera, then remove the old one
     * that's not doing anything. We could quite easily 're-instate' the
     * old camera, but I'm lazy :) */

	 if (ent->owner->health > 0)
	 {
		 ChasecamStart (ent->owner);
		 G_FreeEdict (ent);
	 }
}

/* Here, the "ent" is referring to the client, the player that owns the
* chasecam, and the "opt" string is telling the function whether to
* totally get rid of the camera, or to put it into the background while
* it checks if the player is out of the water or not. The "opt" could
* have easily been a string, and might have used less memory, but it is
* easier to have a string as it is clearer to the reader */

void ChasecamRemove (edict_t *ent, char *opt)
{
   /* Stop the chasecam from moving */

   VectorClear (ent->client->chasecam->velocity);
        
   /* Make the weapon model of the player appear on screen for 1st
   * person reality and aiming */

   ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

   /* Make our invisible appearance the same model as the display entity
   * that mimics us while in chasecam mode */

   ent->s.modelindex = ent->client->oldplayer->s.modelindex = 255; // valid modelindex
   ent->s.modelindex2 = ent->client->oldplayer->s.modelindex2;
   ent->s.modelindex3 = ent->client->oldplayer->s.modelindex3;
   ent->svflags &= ~SVF_NOCLIENT;

   if (!strcmp(opt, "background"))
   {
       G_FreeEdict (ent->client->chasecam);
       G_FreeEdict (ent->client->oldplayer);

	   ent->client->chasetoggle = 0;
       ent->client->chasecam = G_Spawn ();
       ent->client->chasecam->owner = ent;
       ent->client->chasecam->solid = SOLID_NOT;
       ent->client->chasecam->movetype = MOVETYPE_FLYMISSILE;
       VectorClear (ent->client->chasecam->mins);
       VectorClear (ent->client->chasecam->maxs);
       ent->client->chasecam->classname = "chasecam";
       ent->client->chasecam->prethink = ChasecamRestart; // begin checking for emergence from the water
   }
   else if (!strcmp(opt, "off"))
   {
      if (ent->client->chasetoggle)
	  {
          free (ent->client->oldplayer->client); // needed??
          G_FreeEdict (ent->client->oldplayer);
	  }

      ent->client->chasetoggle = 0;
      ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION; // this turns off Quake2's inclination to predict where the camera is going,

	  Rip_SetSkin (ent);
      G_FreeEdict (ent->client->chasecam);
   }
}

/* The "ent" is the chasecam */

void ChasecamTrack (edict_t *ent)
{
   /* Create tempory vectors and trace variables */
   trace_t      tr;
   vec3_t       spot1, spot2, dir;
   vec3_t       forward, right, up;
   int          dist;
   int          cap;
        
   /* if our owner is under water, run the remove routine to repeatedly
   * check for emergment from water */

       
   /* get the CLIENT's angle, and break it down into direction vectors,
   * of forward, right, and up. VERY useful */

   AngleVectors (ent->owner->client->v_angle, forward, right, up);
        
   /* go starting at the player's origin, forward, ent->chasedist1
   * distance, and save the location in vector spot2 */

   VectorMA (ent->owner->s.origin, -ent->chasedist1, forward, spot2);
        
   /* make spot2 a bit higher, but adding 20 to the Z coordinate */

   spot2[2] += 20.000;

   /* if the client is looking down, do backwards up into the air, 0.6
   * to the ratio of looking down, so the crosshair is still roughly
   * aiming at where the player is aiming. */

   if (ent->owner->client->v_angle[0])
      VectorMA (spot2, (ent->owner->client->v_angle[0] * 0.2), up, spot2);

   /* if the client is looking up, do the same, but do DOWN rather than
   * up, so the camera is behind the player aiming in a similar dir */

   else if (ent->owner->client->v_angle[0] > 0.000)
      VectorMA (spot2, (ent->owner->client->v_angle[0] * 0.2), up, spot2);

   /* make the tr traceline trace from the player model's position, to spot2,
   * ignoring the player, with MASK_SHOT. These masks have been fixed
   * from the previous version. The MASK_SHOT will stop the camera from
   * getting stuck in walls, sky, etc. */

   tr = gi.trace (ent->owner->s.origin, NULL, NULL, spot2, ent->owner, MASK_SHOT);
        
   /* subtract the endpoint from the start point for length and
   * direction manipulation */

   VectorSubtract (tr.endpos, ent->owner->s.origin, spot1);

   /* in this case, length */

   ent->chasedist1 = VectorLength (spot1);
        
   /* go, starting from the end of the trace, 2 points forward (client
   * angles) and save the location in spot2 */

   VectorMA (tr.endpos, 2, forward, spot2);

   /* make spot1 the same for tempory vector modification and make spot1
   * a bit higher than spot2 */

   VectorCopy (spot2, spot1);
   spot1[2] += 32;

   /* another trace from spot2 to spot2, ignoring player, no masks */

   tr = gi.trace (spot2, NULL, NULL, spot1, ent->owner, MASK_SHOT);

   /* if we hit something, copy the trace end to spot2 and lower spot2 */

   if (tr.fraction < 1.000)
   {
        VectorCopy (tr.endpos, spot2);
        spot2[2] -= 32;
   }

   /* subtract endpos spot2 from startpos the camera origin, saving it to
   * the dir vector, and normalize dir for a direction from the camera
   * origin, to the spot2 */

   VectorSubtract (spot2, ent->s.origin, dir);
   VectorNormalize (dir);
        
   /* subtract the same things, but save it in spot1 for a temporary
   * length calculation */

   VectorSubtract (spot2, ent->s.origin, spot1);
   dist = VectorLength (spot1);
        
   /* another traceline */

   tr = gi.trace (ent->s.origin, NULL, NULL, spot2, ent->owner, MASK_SHOT);
        
   /* if we DON'T hit anyting, do some freaky stuff <G> */

   if (tr.fraction == 1)
   {
       /* Make the angles of the chasecam, the same as the player, so
       * we are always behind the player. (angles) */

       VectorCopy (ent->owner->s.angles, ent->s.angles);
        
       /* calculate the percentages of the distances, and make sure we're
       * not going too far, or too short, in relation to our panning
       * speed of the chasecam entity */

       cap = (dist * 0.4);

       /* if we're going too fast, make us top speed */

       if (cap > 5.200)
		   VectorSet(ent->velocity, ((dir[0] * dist) * 5.2), ((dir[1] * dist) * 5.2), ((dir[2] * dist) * 5.2));
       else
       {
           /* if we're NOT going top speed, but we're going faster than
           * 1, relative to the total, make us as fast as we're going */

          if ((cap > 1))
		   VectorSet(ent->velocity, ((dir[0] * dist) * cap), ((dir[1] * dist) * cap), ((dir[2] * dist) * cap));
          else
          {
              /* if we're not going faster than one, don't accelerate our
              * speed at all, make us go slow to our destination */
              VectorSet(ent->velocity, (dir[0] * dist), (dir[1] * dist), (dir[2] * dist));
          }
       }
                
       /* subtract endpos;player position, from chasecam position to get
       * a length to determine whether we should accelerate faster from
       * the player or not */

       VectorSubtract (ent->owner->s.origin, ent->s.origin, spot1);

       if (VectorLength(spot1) < 20)
       {
           ent->velocity[0] *= 2; 
           ent->velocity[1] *= 2; 
           ent->velocity[2] *= 2; 
       }
   }

   /* if we DID hit something in the tr.fraction call ages back, then
   * make the spot2 we created, the position for the chasecamera. */
   else
      VectorCopy (spot2, ent->s.origin);

   /* If the distance is less than 90, then we haven't reached the
   * furthest point. If we HAVEN'T reached the furthest point, keep
   * going backwards. This was a fix for the "shaking". The camera was
   * getting forced backwards, only to be brought back, next think */

   if (ent->chasedist1 < 30)
        ent->chasedist1 = 30;

   /* if we're too far away, give us a maximum distance */

   else if (ent->chasedist1 > 90)
        ent->chasedist1 = 120;

   /* if we haven't gone anywhere since the last think routine, and we
   * are greater than 20 points in the distance calculated, add one to
   * the second chasedistance variable

   * The "ent->movedir" is a vector which is not used in this entity, so
   * we can use this a tempory vector belonging to the chasecam, which
   * can be carried through think routines. */

   if (ent->movedir == ent->s.origin)
   {
      if (dist > 20)
         ent->chasedist2++;
   }

   /* if we've buggered up more than 3 times, there must be some mistake,
   * so restart the camera so we re-create a chasecam, destroy the old one,
   * slowly go outwards from the player, and keep thinking this routing in
   * the new camera entity */

   if (ent->chasedist2 > 3)
   {
      ChasecamStart (ent->owner);
      G_FreeEdict(ent);
      return;
   }

   /* Copy the position of the chasecam now, and stick it to the movedir
   * variable, for position checking when we rethink this function */

   VectorCopy (ent->s.origin, ent->movedir);
}

void Cmd_Chasecam_Toggle (edict_t *ent)
{
	if (ent->ripstate & STATE_FEINING)
		return;

	if (!ent->playerclass)
		return;

	if (ent->teamstate & STATE_BUILDING)
		return;

    if (!ent->deadflag)
    {
        if (ent->client->chasetoggle)
            ChasecamRemove (ent, "off");
        else
            ChasecamStart (ent);
	}
}

void CheckChasecam_Viewent (edict_t *ent)
{
	gclient_t *cl;

	if (ent->deadflag)
		return;

	if (!ent->client->oldplayer->client)
	{
        cl = (gclient_t *) malloc(sizeof(gclient_t));
		ent->client->oldplayer->client = cl;
	}

    if ((ent->client->chasetoggle == 1) && G_EntExists(ent->client->oldplayer))
    {
       /* Copy the origin, the speed, and the model angle, NOT
       * literal angle to the display entity */

       VectorCopy (ent->velocity, ent->client->oldplayer->velocity);

       /* Make sure we are using the same model + skin as selected,
       * as well as the weapon model the player model is holding.
       * For customized deathmatch weapon displaying, you can
       * use the modelindex2 for different weapon changing, as you
       * can read in forthcoming tutorials */

       ent->client->oldplayer->s = ent->s;

       gi.linkentity (ent->client->oldplayer);
    }
}

