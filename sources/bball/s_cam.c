#include "g_local.h"

/*===========================================
 -= C H A S E =-           -= C A M E R A =-
 By James Williams for sATaN's BoX of FuN Edition 2
 ==========================================*/

void ChasecamTrack (edict_t *ent);

        /*  The ent is the owner of the chasecam  */

void ChasecamStart (edict_t *ent)
{

        /* This creates a tempory entity we can manipulate within this
         * function */

	edict_t      *chasecam;
        
        /* Tell everything that looks at the toggle that our chasecam is on
         * and working */

	ent->client->chasetoggle = 1;

        /* Make out gun model "non-existent" so it's more realistic to the
         * player using the chasecam */

	ent->client->ps.gunindex = 0;
        
	chasecam = G_Spawn ();
	chasecam->owner = ent;
	chasecam->solid = SOLID_NOT;
	chasecam->movetype = MOVETYPE_FLYMISSILE;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	ent->svflags = SVF_NOCLIENT;

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

        /* If the player is still underwater, break the routine */

	if (ent->owner->waterlevel)
		return;
        
        /* If the player is NOT under water, and not dead, then he's going to
         * want his camera back. Create a new camera, then remove the old one
         * that's not doing anything. We could quite easily 're-instate' the
         * old camera, but I'm lazy :) */

	ChasecamStart (ent->owner);
	G_FreeEdict (ent);
}

     /* Here, the "ent" is referring to the client, the player that owns the
      * chasecam, and the "opt" string is telling the function whether to
      * totally get rid of the camera, or to put it into the background while
      * it checks if the player is out of the water or not. The "opt" could
      * have easily been a string, and might have used less memory, but it is
      * easier to have a string as it is clearer to the reader */

void ChasecamRemove (edict_t *ent)
{
    if (ent->client->chasetoggle == 0)
	{
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
		ent->s.modelindex = ent->client->oldplayer->s.modelindex;

        /* Stop the chasecam from moving */

		VectorClear (ent->client->chasecam->velocity);
        
        ent->svflags &= ~SVF_NOCLIENT; 
		// Ridah, set this back
		ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		// Ridah, done.

		free(ent->client->oldplayer->client);
		G_FreeEdict(ent->client->oldplayer);
		G_FreeEdict(ent->client->chasecam);
	}
	else
	{
		ent->client->chasetoggle = 2;
	}
}

/* The "ent" is the chasecam */ 
void ChasecamTrack (edict_t *ent)
{ 
/* Create tempory vectors and trace variables */ 
	trace_t tr; 
	vec3_t spot2, headorg, angle; 
	vec3_t forward, right, up; 
	float dist; 

	//M.S.Fix This is not necessary?
	//ent->nextthink = level.time + 0.100; 

	VectorCopy(ent->owner->s.origin, headorg); 
	if(!(ent->owner->client->ps.pmove.pm_flags & PMF_DUCKED)) 
		headorg[2] += 25; 
	else 
		headorg[2] += 10; 

	if(ent->owner->client->chasedist1 <= 0) 
		ent->owner->client->chasedist1 = 80; 

	VectorCopy(ent->owner->client->v_angle, angle); 
	/* get the CLIENT's viewangle, and break it down into direction vectors, of forward, right, and up. VERY useful */ 
	AngleVectors (ent->owner->client->ps.viewangles, forward, right, up); 

	dist = ent->chasedist2 / ent->owner->client->chasedist1; 

	VectorScale(forward, -ent->owner->client->chasedist1, spot2); // Find the max distance 
	spot2[2] += 5.00; 
	VectorScale(spot2, dist, spot2); // Calculate the current distance 

	VectorAdd(headorg, spot2, spot2); 
	tr = gi.trace (headorg, NULL, NULL, spot2, ent->owner, true); 
	VectorSubtract(spot2, headorg, spot2); 

	VectorScale(spot2, tr.fraction - 0.05, spot2); // Scale the distance if the trace hit a wall 

	VectorAdd(spot2, headorg, spot2); 
	VectorCopy(spot2, ent->s.origin); 
	VectorCopy(angle, ent->s.angles); 

	/* Copy the position of the chasecam now, and stick it to the movedir variable, for position 
	checking when we rethink this function */ 
	VectorCopy (ent->s.origin, ent->movedir); 

	// If the cam is supposed to turn off zoom in 
	if(ent->owner->client->chasetoggle == 2)
	{
		ent->chasedist2 -= 6; 
		// If it has finished zooming in remove the camera 
		if (ent->chasedist2 <= 0)
		{
			ent->chasedist2 = 0; 
			ent->owner->client->chasetoggle = 0; 
			ChasecamRemove(ent->owner); 
		} 
	} 
	else if (ent->chasedist2 < ent->owner->client->chasedist1)
	{
		ent->chasedist2 += 6; 
		if (ent->chasedist2 > ent->owner->client->chasedist1)
			ent->chasedist2 = ent->owner->client->chasedist1;
	} 
	else if (ent->chasedist2 > ent->owner->client->chasedist1)
	{
		ent->chasedist2 -= 6;
		if (ent->chasedist2 < ent->owner->client->chasedist1)
			ent->chasedist2 = ent->owner->client->chasedist1;
	}
} 

void Cmd_Chasecam_Toggle (edict_t *ent)
{ 
	if (ent->client->chasetoggle > 0) 
		ChasecamRemove (ent); 
	else if(ent->movetype != MOVETYPE_NOCLIP)
		ChasecamStart (ent); 
} 

void Cmd_Chasecam_Zoom(edict_t *ent, char *opt)
{ 
	if(!strcmp(opt, "out") && ent->client->chasetoggle > 0) 
		ent->client->chasedist1 += 10; 
	else if (ent->client->chasetoggle > 0)
		ent->client->chasedist1 -= 10;

	if (ent->client->chasedist1 <= 0)
		ent->client->chasedist1 = 80;

	gi.cprintf(ent,PRINT_HIGH, "Zoom Amount: %f\n", ent->client->chasedist1); 
} 

void Cmd_Chasecam_Viewlock(edict_t *ent)
{ 
	if(ent->client->chasetoggle == 1) 
		ent->client->chasetoggle = 3; 
	else if (ent->client->chasetoggle == 3)
		ent->client->chasetoggle = 1;
	else if (ent->client->chasetoggle == 0)
	{
		ChasecamStart(ent);
		ent->client->chasetoggle = 3;
	}
	
	gi.cprintf(ent,PRINT_HIGH,"Angle: %f\n", ent->client->chasecam->chaseAngle);
} 

void Cmd_Chasecam_Reset(edict_t *ent)
{
	if (ent->client->chasetoggle != 0)
	{
		ent->client->chasetoggle = 1;
		ent->client->chasecam->chaseAngle = 0;
	}
}

void CheckChasecam_Viewent (edict_t *ent)
{ 
	gclient_t *cl; 
	if (!ent->client->oldplayer->client)
	{ 
		cl = (gclient_t *) 
		malloc(sizeof(gclient_t)); 
		ent->client->oldplayer->client = cl; 
	} 

	if ((ent->client->chasetoggle >= 1) && (ent->client->oldplayer))
	{ 
		ent->client->oldplayer->s.frame = ent->s.frame; 
		/* Copy the origin, the speed, and the model angle, NOT 
		literal angle to the display entity */ 
		VectorCopy (ent->s.origin, ent->client->oldplayer->s.origin); 
		VectorCopy (ent->velocity, ent->client->oldplayer->velocity); 
		VectorCopy (ent->s.angles, ent->client->oldplayer->s.angles); 
		/* Make sure we are using the same model + skin as selected, as well 
		as the weapon model the player model is holding. For customized 
		deathmatch weapon displaying, you can use the modelindex2 for 
		different weapon changing, as you can read in forthcoming tutorials */ 
	
		ent->client->oldplayer->s = ent->s; 
		// copies over all of the important player related information 
		gi.linkentity (ent->client->oldplayer); 
	} 
}


