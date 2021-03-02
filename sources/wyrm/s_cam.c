//CHASECAM!


#include "g_local.h"

void ChasecamTrack (edict_t *ent);

/* The ent is the owner of the chasecam */

void ChasecamStart (edict_t *ent)
 {
 /* This creates a tempory entity we can manipulate within this
 * function */

 edict_t *chasecam;

 /* Tell everything that looks at the toggle that our chasecam is on
 * and working */

 ent->client->chasetoggle = 1;

 /* Make out gun model "non-existent" so it's more realistic to the
 * player using the chasecam */

 ent->client->ps.gunindex = 0;

 chasecam = G_Spawn ();
 chasecam->owner = ent;
 chasecam->solid = SOLID_NOT;

// chasecam->movetype = MOVETYPE_FLYMISSILE;
 chasecam->movetype = MOVETYPE_NOCLIP;

// chasecam->clipmask = MASK_OPAQUE;

 chasecam->chasedist1 = 20;

     // Added by WarZone - Begin
     ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION; // this turns off Quake2's inclination to predict where the camera is going,
     // making a much smoother ride
     chasecam->svflags |= SVF_NOCLIENT; // this line tells Quake2 not to send the unnecessary info about the camera to other players
     // Added by WarZone - End

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
// chasecam->think = ChasecamTrack;

 ent->client->chasecam = chasecam;
 ent->client->oldplayer = G_Spawn();

 //execute the routine only once...
 chasecam->prethink(chasecam);
}


void ChasecamRestart (edict_t *ent)
{
        /*Keep thinking this function to check all the time whether the player is out of the water */
        /* if the player is dead, the camera is not wanted... Kill me and stop the funcion */

        if (ent->owner->health <= 0)
        {
                G_FreeEdict (ent);
                return;
        }

        /* If the player is still underwater, break the routine */

        if (ent->owner->waterlevel)
                return;

        /*If the player is NOT under water, and not dead, the he is going to
        want his camera back. Create a new camera, then remove the old one
        that's not doing anything. We could quite easily 're-instate' the
        old camera, but I'm lazy :) */
        ChasecamStart (ent->owner);
        G_FreeEdict(ent);
}

void ChasecamRemove(edict_t *ent)
{
        if (ent->client->missile) return;

        if (!ent->client->chasetoggle)
                return;

        VectorClear(ent->client->chasecam->velocity);

        if (!ent->client->onturret && !ent->client->missile)
        {
                ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
                ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
        }

        ent->s.modelindex = ent->client->oldplayer->s.modelindex;

//        ent->svflags &= ~SVF_NOCLIENT;

        if (ent->client->chasetoggle)
        {
                free(ent->client->oldplayer->client); //needed?
                G_FreeEdict(ent->client->oldplayer);
        }
        ent->client->chasetoggle = 0;
        G_FreeEdict(ent->client->chasecam);

}


void ChasecamTrack (edict_t *ent)
{
        
   trace_t      tr;
   vec3_t       spot1, spot2, spot3, spot4, spot5, dir;
   vec3_t       forward, right, up;
   vec3_t       size1, size2;
   float distance, tot;
//   vec3_t       owner_origin;
   

   VectorSet(size1, -4, -4, -4);
   VectorSet(size2, 4, 4, 4);

   ent->nextthink = level.time + 0.1;

   AngleVectors (ent->owner->client->v_angle, forward, right, up);

   VectorCopy(up, dir);
   VectorNegate(dir,dir);
   if (!(ent->owner->client->ps.pmove.pm_flags & PMF_DUCKED))
           VectorMA(dir, 2, forward, dir);
   else
           VectorMA(dir, 4, forward, dir);

   VectorNormalize(dir);

//spot1 is the owner origin...
   VectorCopy (ent->owner->s.origin, spot1);
//   spot1[2] += ent->owner->viewheight;

   VectorMA (spot1, -ent->chasedist1, dir, spot2);

   tr = gi.trace (spot1, size1, size2, spot2, ent->owner, MASK_OPAQUE);

   VectorCopy(tr.endpos, spot3);

   tr = gi.trace (spot1, NULL, NULL, spot2, ent->owner, MASK_OPAQUE);

   VectorCopy(tr.endpos, spot2);

   VectorSubtract(spot3, spot1, spot4);
   VectorSubtract(spot2, spot1, spot5);

   if (VectorLength(spot4) > VectorLength(spot5))
   {
//           VectorMA(spot2, 4, dir, spot2);
           VectorMA(spot2, 4, tr.plane.normal, spot2);
   }
   else
           VectorCopy(spot3, spot2);

/*
   if (tr.fraction != 1)
   {
           VectorSubtract(spot2, ent->owner->s.origin, spot1);
           ent->chasedist1 = VectorLength(spot1);
   }
*/
   tr = gi.trace (ent->s.origin, size1, size2, spot2, ent->owner,  MASK_OPAQUE);

   if ((tr.fraction != 1)|| !visible(ent, ent->owner))
   {
           VectorCopy(spot2, ent->s.origin);
           VectorClear(ent->velocity);
   }
   else
   {
           VectorSubtract(spot2, ent->s.origin, spot1);
           distance = VectorLength(spot1);
           VectorNormalize(spot1);
           VectorCopy(spot1, dir);

           tot = 0.4 * distance;

           /* if we're going too fast, make us top speed */

           if (tot > 5.2)
                VectorScale(dir, distance * 5.2, ent->velocity);
           else
           {
                 /* if we're NOT going top speed, but we're going faster than
                  * 1, relative to the total, make us as fast as we're going */

                 if (tot > 1)
                        VectorScale(dir, distance * tot, ent->velocity);
                 else
                 {
                         /* if we're not going faster than one, don't accelerate our
                          * speed at all, make us go slow to our destination */
                         VectorScale(dir, distance, ent->velocity);
                 }
           }

           /* subtract endpos;player position, from chasecam position to get
            * a length to determine whether we should accelerate faster from
            * the player or not */

           if (distance < 20)
                VectorScale (ent->velocity, 2, ent->velocity);

   }

   /* add to the distance between the player and the camera */
   ent->chasedist1 += 2;

   /* if we're too far away, give us a maximum distance */
   if (ent->chasedist1 > ent->owner->client->cammaxdistance)
        ent->chasedist1 = ent->owner->client->cammaxdistance;

}

void Cmd_Chasecam_Toggle (edict_t *ent)
{
        if (ent->movetype == MOVETYPE_NOCLIP)
        {
                gi.cprintf (ent, PRINT_HIGH, "You must join the game before using the Chasecam.\n");
                return;
        }

        if (cam_force->value)
        {
                gi.cprintf (ent, PRINT_HIGH, "Sorry, you must play with the chasecam (to disable this, set 'cam_force' to 0).\n");
                return;
        }

        if (!ent->deadflag)
        {
                if (ent->client->chasetoggle)
                        ChasecamRemove(ent);
                else
                        ChasecamStart(ent);
        }
}

void CheckChasecam_Viewent(edict_t *ent)
{
        gclient_t *cl;

        if (!ent->client->oldplayer->client)
        {
                cl = (gclient_t *)malloc(sizeof(gclient_t));
                ent->client->oldplayer->client = cl;
        }
        if ((ent->client->chasetoggle == 1 || ent->client->missile)&&(ent->client->oldplayer))
        {
                ent->client->oldplayer->s.frame = ent->s.frame;
                VectorCopy(ent->s.origin, ent->client->oldplayer->s.origin);
                VectorCopy(ent->velocity, ent->client->oldplayer->velocity);
                VectorCopy(ent->s.angles, ent->client->oldplayer->s.angles);

                ent->client->oldplayer->s = ent->s;
                ent->client->oldplayer->mass = ent->mass;
                if (ent->svflags & SVF_NOCLIENT)
                        ent->client->oldplayer->svflags |= SVF_NOCLIENT;
                else
                        ent->client->oldplayer->svflags &= ~SVF_NOCLIENT;

                gi.linkentity(ent->client->oldplayer);
        }

}

void Cmd_CamMaxDistance (edict_t *ent)
{
		char		*name;
        int                     num;

        if (!strlen(gi.args()))
                {
                gi.cprintf (ent, PRINT_HIGH, "The actual cam_maxdistance is: %d\n", ent->client->cammaxdistance);
                return;
        }

	name = gi.args();

        num = atoi(name);

        if (num > 0 && num < 500)
        {
                ent->client->cammaxdistance = num;
                gi.cprintf (ent, PRINT_HIGH, "cam_maxdistance set to %d\n", ent->client->cammaxdistance);
        }
}
