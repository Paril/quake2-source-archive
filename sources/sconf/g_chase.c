#include "g_local.h"

void CTFOpenJoinMenu(edict_t *ent);

void UpdateChaseCam(edict_t *ent)
{
	vec3_t o, ownerv, goal;
	edict_t *targ;
	vec3_t forward, right;
	trace_t trace;
	int i;
	char s[1024];
	int stringlength=0; // ************
	//int j;
	//char entry[1024]; // **************
	vec3_t oldgoal;
	vec3_t angles;

	// is our chase target gone?
	if (!ent->client->chase_target->inuse) {
				
		ent->client->chase_target = NULL;


		/// UNTESTED VERSION 3.0 BETA 3
		// ALLOW CAM TO JUMP TO ANOTHER PLAYER RATHER THAN FLOAT?
		if (!ent->client->chase_target) // (ALLOWS IT TO WORK) patrick
		{
			for (i = 1; i <= maxclients->value; i++) {
				ent = g_edicts + i;
				if (ent->inuse && ent->client->chase_target == ent)
					UpdateChaseCam(ent);
		}
		}
		/// END UNTESTED VERSION 3.0 BETA 3


		//if (ent->movetype == MOVETYPE_NOCLIP) {
		  ent->client->cloakable=0; // disable cloaking
		  ent->client->hudcloak_state == false; 
		  ent->client->cloaking = false;
		  ent->client->ps.stats[STAT_CLOAKCELLS] = 0;

		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->resp.ctf_team = CTF_NOTEAM;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);

		return;
	}
	
	// Fixes quirk when you are chasing someone who goes into chase cam mode themself
	// sconfig 2.8 fix (LOOK AT LATER!!, fix death of client chasing to float mode)
	// OCCURS ON A CLIENT DEATH
	if (ent->client->chase_target->inuse && ent->client->chase_target->solid == SOLID_NOT) {

		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->resp.ctf_team = CTF_NOTEAM;
		ent->client->ps.gunindex = 0;
		
		//targ = ent->client->chase_target; // with this let you reset at least
		ent->client->chase_target = NULL; // jumped off always
		gi.linkentity (ent);
		//	ent->client->update_chase = true; // ADDED 3.0 FIX
		return;
	}
	// end sconfig 2.8

	

	targ = ent->client->chase_target;

	VectorCopy(targ->s.origin, ownerv);
	VectorCopy(ent->s.origin, oldgoal);

	ownerv[2] += targ->viewheight;

	VectorCopy(targ->client->v_angle, angles);
	if (angles[PITCH] > 56)
		angles[PITCH] = 56;
	AngleVectors (angles, forward, right, NULL);
	VectorNormalize(forward);
	VectorMA(ownerv, -30, forward, o);

	if (o[2] < targ->s.origin[2] + 30) // was 20 appear to be height
		o[2] = targ->s.origin[2] + 30; // was 20

	// jump animation lifts
	if (!targ->groundentity)
		o[2] += 16;

	trace = gi.trace(ownerv, vec3_origin, vec3_origin, o, targ, MASK_SOLID);

	VectorCopy(trace.endpos, goal);

	VectorMA(goal, 2, forward, goal);

	// pad for floors and ceilings
	VectorCopy(goal, o);
	o[2] += 6;
	trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
	if (trace.fraction < 1) {
		VectorCopy(trace.endpos, goal);
		goal[2] -= 6;
	}

	VectorCopy(goal, o);
	o[2] -= 6;
	trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
	if (trace.fraction < 1) {
		VectorCopy(trace.endpos, goal);
		goal[2] += 6;
	}

	ent->client->ps.pmove.pm_type = PM_FREEZE;

	VectorCopy(goal, ent->s.origin);
	for (i=0 ; i<3 ; i++)
		ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

	VectorCopy(targ->client->v_angle, ent->client->ps.viewangles);
	VectorCopy(targ->client->v_angle, ent->client->v_angle);

	ent->viewheight = 0;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;

	gi.linkentity(ent);

	if ((!ent->client->showscores && !ent->client->menu &&
		!ent->client->showinventory && !ent->client->showhelp &&
		!(level.framenum & 31)) || ent->client->update_chase) {

		

				ent->client->update_chase = false;
		sprintf(s, "xv 0 yb -58 string2 \"Chasing %s\""
			"xr -218 yb -78 string \"Hit 'TAB' or type 'menu'\" " 
		    "xr -218 yb -68 string \"to toggle menu.\" ",
			targ->client->pers.netname);
		gi.WriteByte (svc_layout);
		gi.WriteString (s);
		gi.unicast(ent, false);

				
/*		
//			  	        "xr -218 yb -78 string \"Type 'menu' or press key\" " 
//		        "xr -218 yb -68 string \"bound to 'Inventory' to \" "
  //             "xr -218 yb -58 string \"toggle menu.\" "      

	
	  //		for (ij=0; ij<1400; ij++)
//			s[ij]='\0';

			s[0] = 0;
			stringlength = strlen(s);	

		ent->client->update_chase = false;

//			sprintf(s, "xv 0 yb -58 string2 \"Chasing %s\" "
//		        "xr -218 yb -78 string \"Type 'menu' or press key\" " 
//		        "xr -218 yb -68 string \"bound to 'Inventory' to \" "
//               "xr -218 yb -58 string \"toggle menu.\" "
//						, targ->client->pers.netname);		
		//***************
		
	
		Com_sprintf(entry, sizeof(entry), 
		"xv 0 yb -58 string2 \"Chasing %s\" "
		"xr -218 yb -78 string \"Type 'menu' or press key\" " 
		"xr -218 yb -68 string \"bound to 'Inventory' to \" "
		"xr -218 yb -58 string \"toggle menu.\" "
		, targ->client->pers.netname);
		
		j = strlen(entry); 

		if (j > 1000) 
			strncpy(s,entry ,1000);
		else
		strcpy (s + stringlength, entry); 
		
		//**************
	
	gi.WriteByte (svc_layout);
	gi.WriteString (s); //s
	gi.unicast (ent, false);


*/
	// end // sconfig 2.8


	}

}

void ChaseNext(edict_t *ent)
{
	int i;
	edict_t *e;

	if (!ent->client->chase_target)
		return;

	i = ent->client->chase_target - g_edicts;
	do {
		i++;
		if (i > maxclients->value)
			i = 1;
		e = g_edicts + i;
		if (!e->inuse)
			continue;
		if (e->solid != SOLID_NOT)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}

void ChasePrev(edict_t *ent)
{
	int i;
	edict_t *e;

	if (!ent->client->chase_target)
		return;

	i = ent->client->chase_target - g_edicts;
	do {
		i--;
		if (i < 1)
			i = maxclients->value;
		e = g_edicts + i;
		if (!e->inuse)
			continue;
		if (e->solid != SOLID_NOT)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}
