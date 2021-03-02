/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "client.h"


extern float g_stepheight; // jitmove - for step smoothing


/*
===================
CL_CheckPredictionError
===================
*/
void CL_CheckPredictionError (void)
{
	int		frame;
	int		delta[3];
	int		i;
	int		len;

	if (!cl_predict->value || (cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION))
		return;

	// calculate the last usercmd_t we sent that the server has processed
	frame = cls.netchan.incoming_acknowledged;
	frame &= (CMD_BACKUP-1);

	// compare what the server returned with what we had predicted it to be
	VectorSubtract (cl.frame.playerstate.pmove.origin, cl.predicted_origins[frame], delta);

	// save the prediction error for interpolation
	len = abs(delta[0]) + abs(delta[1]) + abs(delta[2]);
	if (len > 640)	// 80 world units
	{	// a teleport or something
		VectorClear (cl.prediction_error);
	}
	else
	{
		if (cl_showmiss->value && (delta[0] || delta[1] || delta[2]) )
			Com_Printf ("Prediction miss on %i: %i\n", cl.frame.serverframe, 
			delta[0] + delta[1] + delta[2]);

		VectorCopy (cl.frame.playerstate.pmove.origin, cl.predicted_origins[frame]);

		// save for error interpolation
		for (i = 0; i < 3; i++)
			cl.prediction_error[i] = delta[i]*0.125;
	}
}


/*
====================
CL_ClipMoveToEntities

====================
*/
void CL_ClipMoveToEntities (const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, trace_t *tr)
{
	int			i, x, zd, zu;
	trace_t		trace;
	int			headnode;
	float		*angles;
	entity_state_t	*ent;
	int			num;
	cmodel_t	*cmodel;
	vec3_t		bmins, bmaxs;

	for (i = 0; i < cl.frame.num_entities; ++i)
	{
		num = (cl.frame.parse_entities + i) & (MAX_PARSE_ENTITIES - 1);
		ent = &cl_parse_entities[num];

		if (!ent->solid)
			continue;

		if (ent->number == cl.playernum + 1)
			continue;

		if (ent->solid == 31)
		{
			// special value for bmodel
			cmodel = cl.model_clip[ent->modelindex];

			if (!cmodel)
				continue;

			headnode = cmodel->headnode;
			angles = ent->angles;
		}
		else
		{
			// encoded bbox
			x = 8 * (ent->solid & 31);
			zd = 8 * ((ent->solid >> 5) & 31);
			zu = 8 * ((ent->solid >> 10) & 63) - 32;

			// Massive hack - when the player is crouched, he's actually -24 and 4 mins/maxs,
			// but since it gets encoded as multiples of 8, 4 gets rounded down to 0.
			// This causes the prediction to be "bouncy", so we'll just hardcode -24 and 0 to be -24 and 4.
			if (zd == 24 && zu == 0)
				zu = 4;

			bmins[0] = bmins[1] = -x;
			bmaxs[0] = bmaxs[1] = x;
			bmins[2] = -zd;
			bmaxs[2] = zu;

			headnode = CM_HeadnodeForBox(bmins, bmaxs);
			angles = vec3_origin;	// boxes don't rotate
		}

		if (tr->allsolid)
			return;

		trace = CM_TransformedBoxTrace(start, end, mins, maxs, headnode, MASK_PLAYERSOLID, ent->origin, angles);

		if (trace.allsolid || trace.startsolid || trace.fraction < tr->fraction)
		{
			trace.ent = (struct edict_s *)ent;

		 	if (tr->startsolid)
			{
				*tr = trace;
				tr->startsolid = true;
			}
			else
			{
				*tr = trace;
			}
		}
		else if (trace.startsolid)
		{
			tr->startsolid = true;
		}
	}
}


/*
================
CL_PMTrace
================
*/
trace_t CL_PMTrace (const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end)
{
	trace_t	t;

	// check against world
	t = CM_BoxTrace(start, end, mins, maxs, 0, MASK_PLAYERSOLID);

	if (t.fraction < 1.0f)
		t.ent = (struct edict_s *)1;

	// check all other solid models
	CL_ClipMoveToEntities(start, mins, maxs, end, &t);

	return t;
}


int CL_PMpointcontents (vec3_t point)
{
	int			i;
	entity_state_t	*ent;
	int			num;
	cmodel_t		*cmodel;
	int			contents;

	contents = CM_PointContents (point, 0);

	for (i=0 ; i<cl.frame.num_entities ; i++)
	{
		num = (cl.frame.parse_entities + i)&(MAX_PARSE_ENTITIES-1);
		ent = &cl_parse_entities[num];

		if (ent->solid != 31) // special value for bmodel
			continue;

		cmodel = cl.model_clip[ent->modelindex];
		if (!cmodel)
			continue;

		contents |= CM_TransformedPointContents (point, cmodel->headnode, ent->origin, ent->angles);
	}

	return contents;
}


/*
=================
CL_PredictMovement

Sets cl.predicted_origin and cl.predicted_angles
=================
*/
void CL_PredictMovement (void)
{
	int			ack, current;
	int			frame;
	pmove_t		pm;
	int			i;
	float		stepheight;
	float		unsentstepheight = 0.0f;
	char		*s;

	if (cls.state != ca_active)
		return;

	if (cl_paused->value)
		return;

	for (i = 0; i < 3; i++) // jitnetfps -- just use the stinkin' view angles
	{
		cl.predicted_angles[i] = cl.viewangles[i] + SHORT2ANGLE(cl.frame.playerstate.pmove.delta_angles[i]);
	}

	if (!cl_predict->value || (cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION))
		return;

	ack = cls.netchan.incoming_acknowledged;
	current = cls.netchan.outgoing_sequence;

	// if we are too far out of date, just freeze
	if (current - ack >= CMD_BACKUP)
	{
		if (cl_showmiss->value)
			Com_Printf("Exceeded CMD_BACKUP.\n");

		return;	
	}

	// copy current state to pmove
	memset(&pm, 0, sizeof(pm));
	pm.trace = CL_PMTrace;
	pm.pointcontents = CL_PMpointcontents;
	pm_airaccelerate = atof(cl.configstrings[CS_AIRACCEL]);
	pm.s = cl.frame.playerstate.pmove;
	frame = 0;

	// === jitmove - get move settings from server
	pm_oldmovephysics = true;
	pm_crouchslidefriction = 0.0f; // 0 = disabled
	pm_skyglide_maxvel = 0.0f;
	s = strstr(cl.configstrings[CS_MOVEPHYSICS], "MovePhysVer: ");

	if (s)
	{
		int ver = atoi(s + sizeof("MovePhysVer:")); // space left out because null is counted

		if (ver >= 1)
			pm_oldmovephysics = false;
	}

	s = strstr(cl.configstrings[CS_MOVEPHYSICS], "CrouchSlide: ");

	if (s)
		pm_crouchslidefriction = atof(s + sizeof("CrouchSlide:"));

	s = strstr(cl.configstrings[CS_MOVEPHYSICS], "SkyGlide: ");

	if (s)
		pm_skyglide_maxvel = atof(s + sizeof("SkyGlide:"));
	// jitmove ===

	// run frames
	while (++ack < current)
	{
		frame = ack & (CMD_BACKUP-1);
		pm.cmd = cl.cmds[frame];
		Pmove(&pm);
		VectorCopy(pm.s.origin, cl.predicted_origins[frame]); // save for comparing against server results so we can smooth corrections
	}

	stepheight = g_stepheight; // jitmove - take the step height from the last sent cmd

	// ===
	// jitnetfps - now that we're not sending a network packet for every frame rendered, we need to execute this pmove
	// to fill in the gap between when we last transmitted a packet and the current time.
	frame = current & (CMD_BACKUP-1);
	pm.cmd = cl.cmds[frame];
	pm.cmd.msec = cls.realtime - cls.last_transmit_time;

	if (pm.cmd.msec > 0)
	{
		Pmove(&pm);
		VectorCopy(pm.s.origin, cl.predicted_origins[frame]);
		unsentstepheight = g_stepheight;
		stepheight = 0.0f; // in this case, we're between sent cmd frames, so we don't want to count the step multiple times
	}
	// jit
	// ===

	// === jitmove - new step smoothing
	if (stepheight > 0.0f)
		cl.predicted_step += stepheight;

	cl.predicted_step = DampIIR(cl.predicted_step, 0.0f, 0.085f, cls.frametime);

	if (cl.predicted_step < 0.0f)
		cl.predicted_step = 0.0f;

	cl.predicted_step_unsent = unsentstepheight;
	// jitmove ===

	// copy results out for rendering
	cl.predicted_origin[0] = pm.s.origin[0] * 0.125f;
	cl.predicted_origin[1] = pm.s.origin[1] * 0.125f;
	cl.predicted_origin[2] = pm.s.origin[2] * 0.125f;
}
