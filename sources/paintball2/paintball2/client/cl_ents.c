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
// cl_ents.c -- entity parsing and management

#include "client.h"


//extern	struct model_s	*cl_mod_powerscreen;

//PGM
int	vidref_val;
//PGM

/*
=========================================================================

FRAME PARSING

=========================================================================
*/


/*
=================
CL_ParseEntityBits

Returns the entity number and the header bits
=================
*/
int	bitcounts[32];	/// just for protocol profiling
int CL_ParseEntityBits (unsigned *bits)
{
	unsigned	b, total;
	int			i;
	int			number;

	total = MSG_ReadByte(&net_message);
	if (total & U_MOREBITS1)
	{
		b = MSG_ReadByte(&net_message);
		total |= b<<8;
	}
	if (total & U_MOREBITS2)
	{
		b = MSG_ReadByte(&net_message);
		total |= b<<16;
	}
	if (total & U_MOREBITS3)
	{
		b = MSG_ReadByte(&net_message);
		total |= b<<24;
	}

	// count the bits for net profiling
	for (i=0 ; i<32 ; i++)
		if (total&(1<<i))
			bitcounts[i]++;

	if (total & U_NUMBER16)
		number = MSG_ReadShort (&net_message);
	else
		number = MSG_ReadByte(&net_message);

	*bits = total;

	return number;
}

/*
==================
CL_ParseDelta

Can go from either a baseline or a previous packet_entity
==================
*/
void CL_ParseDelta (entity_state_t *from, entity_state_t *to, int number, int bits)
{
	// set everything to the state we are delta'ing from
	*to = *from;

	VectorCopy(from->origin, to->old_origin);
	to->number = number;

	if (bits & U_MODEL)
		to->modelindex = MSG_ReadByte(&net_message);

	if (bits & U_MODEL2)
		to->modelindex2 = MSG_ReadByte(&net_message);

	if (bits & U_MODEL3)
		to->modelindex3 = MSG_ReadByte(&net_message);

	if (bits & U_MODEL4)
		to->modelindex4 = MSG_ReadByte(&net_message);
		
	if (bits & U_FRAME8)
		to->frame = MSG_ReadByte(&net_message);

	if (bits & U_FRAME16)
		to->frame = MSG_ReadShort(&net_message);

	assert(to->skinnum >= 0); // jitdebug

	if ((bits & U_SKIN8) && (bits & U_SKIN16))		//used for laser colors
		to->skinnum = MSG_ReadLong(&net_message);
	else if (bits & U_SKIN8)
		to->skinnum = MSG_ReadByte(&net_message);
	else if (bits & U_SKIN16)
		to->skinnum = MSG_ReadShort(&net_message);

	assert(to->skinnum >= 0);

	if ((bits & (U_EFFECTS8|U_EFFECTS16)) == (U_EFFECTS8|U_EFFECTS16))
		to->effects = MSG_ReadLong(&net_message);
	else if (bits & U_EFFECTS8)
		to->effects = MSG_ReadByte(&net_message);
	else if (bits & U_EFFECTS16)
		to->effects = MSG_ReadShort(&net_message);

	if ((bits & (U_RENDERFX8|U_RENDERFX16)) == (U_RENDERFX8|U_RENDERFX16))
		to->renderfx = MSG_ReadLong(&net_message);
	else if (bits & U_RENDERFX8)
		to->renderfx = MSG_ReadByte(&net_message);
	else if (bits & U_RENDERFX16)
		to->renderfx = MSG_ReadShort(&net_message);

	if (bits & U_ORIGIN1)
		to->origin[0] = MSG_ReadCoord(&net_message);

	if (bits & U_ORIGIN2)
		to->origin[1] = MSG_ReadCoord(&net_message);

	if (bits & U_ORIGIN3)
		to->origin[2] = MSG_ReadCoord(&net_message);
		
	if (bits & U_ANGLE1)
		to->angles[0] = MSG_ReadAngle(&net_message);

	if (bits & U_ANGLE2)
		to->angles[1] = MSG_ReadAngle(&net_message);

	if (bits & U_ANGLE3)
		to->angles[2] = MSG_ReadAngle(&net_message);

	if (bits & U_OLDORIGIN)
		MSG_ReadPos(&net_message, to->old_origin);

	if (bits & U_SOUND)
		to->sound = MSG_ReadByte(&net_message);

	if (bits & U_EVENT)
		to->event = MSG_ReadByte(&net_message);
	else
		to->event = 0;

	if (bits & U_SOLID)
		to->solid = MSG_ReadShort(&net_message);
}

/*
==================
CL_DeltaEntity

Parses deltas from the given base and adds the resulting entity
to the current frame
==================
*/
void CL_DeltaEntity (frame_t *frame, int newnum, entity_state_t *old, int bits)
{
	centity_t	*ent;
	entity_state_t	*state;

	ent = &cl_entities[newnum];

	state = &cl_parse_entities[cl.parse_entities & (MAX_PARSE_ENTITIES - 1)];
	cl.parse_entities++;
	frame->num_entities++;

	CL_ParseDelta(old, state, newnum, bits);

	// some data changes will force no lerping
	if (state->modelindex != ent->current.modelindex
		|| state->modelindex2 != ent->current.modelindex2
		|| state->modelindex3 != ent->current.modelindex3
		|| state->modelindex4 != ent->current.modelindex4
		|| abs(state->origin[0] - ent->current.origin[0]) > 512
		|| abs(state->origin[1] - ent->current.origin[1]) > 512
		|| abs(state->origin[2] - ent->current.origin[2]) > 512
		|| state->event == EV_PLAYER_TELEPORT
		|| state->event == EV_OTHER_TELEPORT
		)
	{
		ent->serverframe = -99;
	}

	if (ent->serverframe != cl.frame.serverframe - 1)
	{	// wasn't in last update, so initialize some things
		ent->trailcount = 1024;		// for diminishing rocket / grenade trails
		// duplicate the current state so lerping doesn't hurt anything
		ent->prev = *state;

		if (state->event == EV_OTHER_TELEPORT)
		{
			VectorCopy(state->origin, ent->prev.origin);
			VectorCopy(state->origin, ent->lerp_origin);
		}
		else
		{
			VectorCopy(state->old_origin, ent->prev.origin);
			VectorCopy(state->old_origin, ent->lerp_origin);
		}
	}
	else
	{	// shuffle the last state to previous
		ent->prev = ent->current;
	}

	ent->serverframe = cl.frame.serverframe;
	ent->current = *state;

	// === jitdemo - override camera position with new target entity playernum
	if (cl.playernum_demooverride && cl.attractloop && state->number == cl.playernum + 1)
	{
		player_state_t *ps = &frame->playerstate;

		ps->pmove.origin[0] = state->origin[0] * 8;
		ps->pmove.origin[1] = state->origin[1] * 8;
		ps->pmove.origin[2] = state->origin[2] * 8;
		ps->viewoffset[0] = 0.0f;
		ps->viewoffset[1] = 0.0f;
		ps->viewoffset[2] = 22.0f; // todo: figure out if crouched and use a different offset.
		ps->viewangles[PITCH] = state->angles[PITCH] * 3.0f; // view angle is scaled by 3 so 3rd person player model doesn't look crazy going fully horizontal and such
		ps->viewangles[YAW] = state->angles[YAW];
		ps->viewangles[ROLL] = state->angles[ROLL];
		VectorClear(ps->kick_angles);
	}
	// jitdemo ===
}

/*
==================
CL_ParsePacketEntities

An svc_packetentities has just been parsed, deal with the
rest of the data stream.
==================
*/
void CL_ParsePacketEntities (frame_t *oldframe, frame_t *newframe)
{
	int			newnum;
	int			bits;
	entity_state_t	*oldstate;
	int			oldindex, oldnum;

	newframe->parse_entities = cl.parse_entities;
	newframe->num_entities = 0;

	// delta from the entities present in oldframe
	oldindex = 0;
	if (!oldframe)
		oldnum = 99999;
	else
	{
		if (oldindex >= oldframe->num_entities)
			oldnum = 99999;
		else
		{
			oldstate = &cl_parse_entities[(oldframe->parse_entities+oldindex) & (MAX_PARSE_ENTITIES-1)];
			oldnum = oldstate->number;
		}
	}

	while (1)
	{
		newnum = CL_ParseEntityBits (&bits);
		if (newnum >= MAX_EDICTS)
			Com_Error (ERR_DROP,"CL_ParsePacketEntities: bad number:%i", newnum);

		if (net_message.readcount > net_message.cursize)
			Com_Error (ERR_DROP,"CL_ParsePacketEntities: end of message");

		if (!newnum)
			break;

		while (oldnum < newnum)
		{	// one or more entities from the old packet are unchanged
			if (cl_shownet->value == 3)
				Com_Printf ("   unchanged: %i\n", oldnum);
			CL_DeltaEntity (newframe, oldnum, oldstate, 0);
			
			oldindex++;

			if (oldindex >= oldframe->num_entities)
				oldnum = 99999;
			else
			{
				oldstate = &cl_parse_entities[(oldframe->parse_entities+oldindex) & (MAX_PARSE_ENTITIES-1)];
				oldnum = oldstate->number;
			}
		}

		if (bits & U_REMOVE)
		{	// the entity present in oldframe is not in the current frame
			if (cl_shownet->value == 3)
				Com_Printf ("   remove: %i\n", newnum);
			if (oldnum != newnum)
				Com_Printf ("U_REMOVE: oldnum != newnum\n");

			oldindex++;

			if (oldindex >= oldframe->num_entities)
				oldnum = 99999;
			else
			{
				oldstate = &cl_parse_entities[(oldframe->parse_entities+oldindex) & (MAX_PARSE_ENTITIES-1)];
				oldnum = oldstate->number;
			}
			continue;
		}

		if (oldnum == newnum)
		{	// delta from previous state
			if (cl_shownet->value == 3)
				Com_Printf ("   delta: %i\n", newnum);
			CL_DeltaEntity (newframe, newnum, oldstate, bits);

			oldindex++;

			if (oldindex >= oldframe->num_entities)
				oldnum = 99999;
			else
			{
				oldstate = &cl_parse_entities[(oldframe->parse_entities+oldindex) & (MAX_PARSE_ENTITIES-1)];
				oldnum = oldstate->number;
			}
			continue;
		}

		if (oldnum > newnum)
		{	// delta from baseline
			if (cl_shownet->value == 3)
				Com_Printf ("   baseline: %i\n", newnum);
			CL_DeltaEntity (newframe, newnum, &cl_entities[newnum].baseline, bits);
			continue;
		}

	}

	// any remaining entities in the old frame are copied over
	while (oldnum != 99999)
	{	// one or more entities from the old packet are unchanged
		if (cl_shownet->value == 3)
			Com_Printf ("   unchanged: %i\n", oldnum);
		CL_DeltaEntity (newframe, oldnum, oldstate, 0);
		
		oldindex++;

		if (oldindex >= oldframe->num_entities)
			oldnum = 99999;
		else
		{
			oldstate = &cl_parse_entities[(oldframe->parse_entities+oldindex) & (MAX_PARSE_ENTITIES-1)];
			oldnum = oldstate->number;
		}
	}
}



/*
===================
CL_ParsePlayerstate
===================
*/
void CL_ParsePlayerstate (frame_t *oldframe, frame_t *newframe)
{
	int			flags;
	player_state_t	*state;
	int			i;
	int			statbits;
	qboolean	no_demo_override = !(cl.attractloop && cl.playernum_demooverride); // jitdemo

	state = &newframe->playerstate;

	// clear to old value before delta parsing
	if (oldframe)
		*state = oldframe->playerstate;
	else
		memset(state, 0, sizeof(*state));

	flags = MSG_ReadShort (&net_message);

	// parse the pmove_state_t
	if (flags & PS_M_TYPE)
		state->pmove.pm_type = MSG_ReadByte(&net_message);

	if (flags & PS_M_ORIGIN)
	{
		// === jitdemo - allow position to be overridden
		short origin[3];

		origin[0] = MSG_ReadShort(&net_message);
		origin[1] = MSG_ReadShort(&net_message);
		origin[2] = MSG_ReadShort(&net_message);

		if (no_demo_override)
		{
			state->pmove.origin[0] = origin[0];
			state->pmove.origin[1] = origin[1];
			state->pmove.origin[2] = origin[2];
#ifdef USE_PMOVE_FLOAT
			// jitmove - floating point precision - testing (todo: ignore & use stored value if off by less than .125)
			state->pmove.forigin[0] = state->pmove.origin[0] * 0.125f;
			state->pmove.forigin[1] = state->pmove.origin[1] * 0.125f;
			state->pmove.forigin[2] = state->pmove.origin[2] * 0.125f;
#endif
		}
		// jitdemo ===
	}

	if (flags & PS_M_VELOCITY)
	{
		state->pmove.velocity[0] = MSG_ReadShort(&net_message);
		state->pmove.velocity[1] = MSG_ReadShort(&net_message);
		state->pmove.velocity[2] = MSG_ReadShort(&net_message);
#ifdef USE_PMOVE_FLOAT
		// jitmove - floating point precision - testing (todo: ignore & use stored value if off by less than .125)
		state->pmove.fvelocity[0] = state->pmove.velocity[0] * 0.125f;
		state->pmove.fvelocity[1] = state->pmove.velocity[1] * 0.125f;
		state->pmove.fvelocity[2] = state->pmove.velocity[2] * 0.125f;
#endif
	}

	if (flags & PS_M_TIME)
		state->pmove.pm_time = MSG_ReadByte(&net_message);

	if (flags & PS_M_FLAGS)
		state->pmove.pm_flags = MSG_ReadByte(&net_message);

	if (flags & PS_M_GRAVITY)
		state->pmove.gravity = MSG_ReadShort(&net_message);

	if (flags & PS_M_DELTA_ANGLES)
	{
		state->pmove.delta_angles[0] = MSG_ReadShort(&net_message);
		state->pmove.delta_angles[1] = MSG_ReadShort(&net_message);
		state->pmove.delta_angles[2] = MSG_ReadShort(&net_message);
	}

	// demo playback
	if (cl.attractloop)
	{
		state->pmove.pm_type = PM_FREEZE; // don't do any weird movement prediction; just interpolate.
	}

	//
	// parse the rest of the player_state_t
	//
	if (flags & PS_VIEWOFFSET)
	{
		// === jitdemo - override camera
		int viewoffset[3];

		viewoffset[0] = MSG_ReadChar(&net_message);
		viewoffset[1] = MSG_ReadChar(&net_message);
		viewoffset[2] = MSG_ReadChar(&net_message);

		if (no_demo_override)
		{
			state->viewoffset[0] = viewoffset[0] * 0.25f;
			state->viewoffset[1] = viewoffset[1] * 0.25f;
			state->viewoffset[2] = viewoffset[2] * 0.25f;
		}
		// jitdemo ===
	}

	if (flags & PS_VIEWANGLES)
	{
		// === jitdemo - override view angles.
		float angles[3];

		angles[0] = MSG_ReadAngle16(&net_message);
		angles[1] = MSG_ReadAngle16(&net_message);
		angles[2] = MSG_ReadAngle16(&net_message);

		if (no_demo_override)
		{
			state->viewangles[0] = angles[0];
			state->viewangles[1] = angles[1];
			state->viewangles[2] = angles[2];
		}
		// jitdemo ===
	}

	if (flags & PS_KICKANGLES)
	{
		// === jitdemo - allow overriding of the camera
		int kick_angles[3];

		kick_angles[0] = MSG_ReadChar(&net_message);
		kick_angles[1] = MSG_ReadChar(&net_message);
		kick_angles[2] = MSG_ReadChar(&net_message);

		if (no_demo_override)
		{
			state->kick_angles[0] = kick_angles[0] * 0.25f;
			state->kick_angles[1] = kick_angles[1] * 0.25f;
			state->kick_angles[2] = kick_angles[2] * 0.25f;
		}
		// jitdemo ===
	}

	if (flags & PS_WEAPONINDEX)
	{
		state->gunindex = MSG_ReadByte(&net_message);
	}

	if (flags & PS_WEAPONFRAME)
	{
		state->gunframe = MSG_ReadByte(&net_message);
		state->gunoffset[0] = MSG_ReadChar(&net_message) * 0.25f;
		state->gunoffset[1] = MSG_ReadChar(&net_message) * 0.25f;
		state->gunoffset[2] = MSG_ReadChar(&net_message) * 0.25f;
		state->gunangles[0] = MSG_ReadChar(&net_message) * 0.25f;
		state->gunangles[1] = MSG_ReadChar(&net_message) * 0.25f;
		state->gunangles[2] = MSG_ReadChar(&net_message) * 0.25f;
	}

	if (flags & PS_BLEND)
	{
		state->blend[0] = MSG_ReadByte(&net_message) * 0.003921568627450980392156862745098f;
		state->blend[1] = MSG_ReadByte(&net_message) * 0.003921568627450980392156862745098f;
		state->blend[2] = MSG_ReadByte(&net_message) * 0.003921568627450980392156862745098f;
		state->blend[3] = MSG_ReadByte(&net_message) * 0.003921568627450980392156862745098f;
	}

	if (flags & PS_FOV)
		state->fov = MSG_ReadByte(&net_message);

	if (flags & PS_RDFLAGS)
		state->rdflags = MSG_ReadByte(&net_message);

	// parse stats
	statbits = MSG_ReadLong(&net_message);

	for (i = 0; i < MAX_STATS; ++i)
	{
		if (statbits & (1 << i))
			state->stats[i] = MSG_ReadShort(&net_message);
	}
}


/*
==================
CL_FireEntityEvents

==================
*/
void CL_FireEntityEvents (frame_t *frame)
{
	entity_state_t		*s1;
	int					pnum, num;

	for (pnum = 0 ; pnum<frame->num_entities ; pnum++)
	{
		num = (frame->parse_entities + pnum)&(MAX_PARSE_ENTITIES-1);
		s1 = &cl_parse_entities[num];
		if (s1->event)
			CL_EntityEvent (s1);

		// EF_TELEPORTER acts like an event, but is not cleared each frame
		if (s1->effects & EF_TELEPORTER)
			CL_TeleporterParticles (s1);
	}
}


/*
================
CL_ParseFrame
================
*/
void CL_ParseFrame (void)
{
	int			cmd;
	int			len;
	frame_t		*old;

	memset(&cl.frame, 0, sizeof(cl.frame));
	cl.frame.serverframe = MSG_ReadLong(&net_message);
	cl.frame.deltaframe = MSG_ReadLong(&net_message);
	cl.frame.servertime = cl.frame.serverframe * 100;

	// BIG HACK to let old demos continue to work
	if (cls.serverProtocol != 26)
		cl.surpressCount = MSG_ReadByte(&net_message);

	if (cl_shownet->value == 3)
		Com_Printf("   frame:%i  delta:%i\n", cl.frame.serverframe, cl.frame.deltaframe);

	// If the frame is delta compressed from data that we
	// no longer have available, we must suck up the rest of
	// the frame, but not use it, then ask for a non-compressed
	// message 
	if (cl.frame.deltaframe <= 0)
	{
		cl.frame.valid = true;		// uncompressed frame
		old = NULL;
		cls.demowaiting = false;	// we can start recording now
	}
	else
	{
		old = &cl.frames[cl.frame.deltaframe & UPDATE_MASK];

		if (!old->valid)
		{	// should never happen
			Com_Printf("Delta from invalid frame (not supposed to happen!).\n");
		}

		if (old->serverframe != cl.frame.deltaframe)
		{	// The frame that the server did the delta from
			// is too old, so we can't reconstruct it properly.
			Com_Printf("Delta frame too old.\n");
		}
		else if (cl.parse_entities - old->parse_entities > MAX_PARSE_ENTITIES-128)
		{
			Com_Printf("Delta parse_entities too old.\n");
		}
		else
		{
			cl.frame.valid = true;	// valid delta parse
		}
	}

	// clamp time 
	if (cl.time > cl.frame.servertime)
		cl.time = cl.frame.servertime;
	else if (cl.time < cl.frame.servertime - 100)
		cl.time = cl.frame.servertime - 100;

	// read areabits
	len = MSG_ReadByte(&net_message);
	MSG_ReadData(&net_message, &cl.frame.areabits, len);

	// read playerinfo
	cmd = MSG_ReadByte(&net_message);
	SHOWNET(svc_strings[cmd]);

	if (cmd != svc_playerinfo)
		Com_Error(ERR_DROP, "CL_ParseFrame: not playerinfo");

	CL_ParsePlayerstate(old, &cl.frame);

	// read packet entities
	cmd = MSG_ReadByte(&net_message);
	SHOWNET(svc_strings[cmd]);

	if (cmd != svc_packetentities)
		Com_Error(ERR_DROP, "CL_ParseFrame: not packetentities");

	CL_ParsePacketEntities(old, &cl.frame);

	// save the frame off in the backup array for later delta comparisons
	cl.frames[cl.frame.serverframe & UPDATE_MASK] = cl.frame;

	if (cl.frame.valid)
	{
		// getting a valid frame message ends the connection process
		if (cls.state != ca_active)
		{
			cls.state = ca_active;
			//cl_scores_setinuse_all(false); // jitscores - clear scoreboard
			cl.force_refdef = true;
			cl.predicted_origin[0] = cl.frame.playerstate.pmove.origin[0]*0.125;
			cl.predicted_origin[1] = cl.frame.playerstate.pmove.origin[1]*0.125;
			cl.predicted_origin[2] = cl.frame.playerstate.pmove.origin[2]*0.125;
			VectorCopy(cl.frame.playerstate.viewangles, cl.predicted_angles);

			if (cls.disable_servercount != cl.servercount && cl.refresh_prepped)
				SCR_EndLoadingPlaque();	// get rid of loading plaque
		}

		cl.sound_prepped = true;	// can start mixing ambient sounds

		// fire entity events
		CL_FireEntityEvents(&cl.frame);
		CL_CheckPredictionError();
	}
}

// PMM - used in shell code 
extern int Developer_searchpath (int who);
// pmm
/*
===============
CL_AddPacketEntities

===============
*/
void CL_AddPacketEntities (frame_t *frame)
{
	entity_t			ent;
	entity_state_t		*s1;
	float				autorotate;
	int					i;
	int					pnum;
	centity_t			*cent;
	int					autoanim;
	clientinfo_t		*ci;
	unsigned int		effects, renderfx;

	// bonus items rotate at a fixed rate
	autorotate = anglemod(cl.time / 10);

	// brush models can auto animate their frames
	autoanim = 2 * cl.time / 1000;

	memset(&ent, 0, sizeof(ent));

	for (pnum = 0; pnum < frame->num_entities; pnum++)
	{
		s1 = &cl_parse_entities[(frame->parse_entities + pnum) & (MAX_PARSE_ENTITIES - 1)];

		cent = &cl_entities[s1->number];

		effects = s1->effects;
		renderfx = s1->renderfx;

		// set frame
		if (effects & EF_ANIM01)
			ent.frame = autoanim & 1;
		else if (effects & EF_ANIM23)
			ent.frame = 2 + (autoanim & 1);
		else if (effects & EF_ANIM_ALL)
			ent.frame = autoanim;
		else if (effects & EF_ANIM_ALLFAST)
			ent.frame = cl.time / 100;
		else
			ent.frame = s1->frame;

		// quad and pent can do different things on client
		if (effects & EF_PENT)
		{
			effects &= ~EF_PENT;
			effects |= EF_COLOR_SHELL;
			renderfx |= RF_SHELL_RED;
		}

		if (effects & EF_QUAD)
		{
			effects &= ~EF_QUAD;
			effects |= EF_COLOR_SHELL;
			renderfx |= RF_SHELL_BLUE;
		}
//======
// PMM
		if (effects & EF_DOUBLE)
		{
			effects &= ~EF_DOUBLE;
			effects |= EF_COLOR_SHELL;
			renderfx |= RF_SHELL_DOUBLE;
		}

		if (effects & EF_HALF_DAMAGE)
		{
			effects &= ~EF_HALF_DAMAGE;
			effects |= EF_COLOR_SHELL;
			renderfx |= RF_SHELL_HALF_DAM;
		}
// pmm
//======
		ent.oldframe = cent->prev.frame;
		ent.backlerp = 1.0f - cl.lerpfrac;

		if (renderfx & (RF_FRAMELERP|RF_BEAM))
		{	// step origin discretely, because the frames
			// do the animation properly
			VectorCopy(cent->current.origin, ent.origin);
			VectorCopy(cent->current.old_origin, ent.oldorigin);
		}
		else
		{	// interpolate origin
			for (i = 0; i < 3; ++i)
			{
				ent.origin[i] = ent.oldorigin[i] = cent->prev.origin[i] + cl.lerpfrac * 
					(cent->current.origin[i] - cent->prev.origin[i]);
				//ent.origin[i] = ent.oldorigin[i] = cent->current.origin[i] + cl.lerpfrac * (cent->current.origin[i] - cent->prev.origin[i]); // testing extrapolation (super crappy!)
			}
		}

		// create a new entity
	
		// tweak the color of beams
		if (renderfx & RF_BEAM)
		{	// the four beam colors are encoded in 32 bits of skinnum (hack)
			ent.alpha = 0.30f;
			ent.skinnum = (s1->skinnum >> ((rand() % 4) * 8)) & 0xff;
			ent.model = NULL;
		}
		else
		{
			// set skin
			if (s1->modelindex == 255)
			{
				// use custom player skin
				ent.skinnum = 0;
				ci = &cl.clientinfo[s1->skinnum & 0xff];
//				ent.skin = ci->skin;
				memcpy(ent.skins, ci->skins, sizeof(ent.skins)); // jitskm
				ent.model = ci->model;

				if (!ent.skins[0] || !ent.model)
				{
					//ent.skin = cl.baseclientinfo.skin;
					memcpy(ent.skins, cl.baseclientinfo.skins, sizeof(ent.skins));
					ent.model = cl.baseclientinfo.model;
				}
			}
			else
			{
				ent.skinnum = s1->skinnum;
				//ent.skin = NULL;
				memset(ent.skins, 0, sizeof(ent.skins)); // jitskm
				ent.model = cl.model_draw[s1->modelindex];
			}
		}

		// only used for black hole model right now, FIXME: do better
		if (renderfx == RF_TRANSLUCENT)
			ent.alpha = 0.70;

		// render effects (fullbright, translucent, etc)
		if ((effects & EF_COLOR_SHELL))
			ent.flags = 0;	// renderfx go on color shell entity
		else
			ent.flags = renderfx;

		// calculate angles
		if (effects & EF_ROTATE)
		{	// some bonus items auto-rotate
			ent.angles[0] = 0;
			ent.angles[1] = autorotate;
			ent.angles[2] = 0;
		}
		// RAFAEL
		else if (effects & EF_SPINNINGLIGHTS)
		{
			ent.angles[0] = 0;
			ent.angles[1] = anglemod(cl.time*0.5) + s1->angles[1];
			ent.angles[2] = 180;
			{
				vec3_t forward;
				vec3_t start;

				AngleVectors(ent.angles, forward, NULL, NULL);
				VectorMA(ent.origin, 64, forward, start);
				V_AddLight(start, 100, 1, 0, 0);
			}
		}
		else
		{	// interpolate angles
			float	a1, a2;

			for (i = 0; i < 3; i++)
			{
				a1 = cent->current.angles[i];
				a2 = cent->prev.angles[i];
				ent.angles[i] = LerpAngle(a2, a1, cl.lerpfrac);
			}
		}

		if (s1->number == cl.playernum + 1)
		{
			ent.flags |= RF_VIEWERMODEL;	// only draw from mirrors
			// FIXME: still pass to refresh

			if (effects & EF_FLAG1)
				V_AddLight(ent.origin, 225, 1.0, 0.1, 0.1);
			else if (effects & EF_FLAG2)
				V_AddLight(ent.origin, 225, 0.1, 0.1, 1.0);
			else if (effects & EF_TAGTRAIL)						//PGM
				V_AddLight(ent.origin, 225, 1.0, 1.0, 0.0);		//PGM
			else if (effects & EF_TRACKERTRAIL)					//PGM
				V_AddLight(ent.origin, 225, -1.0, -1.0, -1.0);	//PGM

			continue;
		}

		// if set to invisible, skip
		if (!s1->modelindex)
			continue;

		// ZH / jit -- don't draw player model with chasecam inside
		if (s1->modelindex2 != 0)
		{
			player_state_t *ps = &frame->playerstate;
			short entpos[3], campos[3];

			entpos[0] = cent->current.origin[0];
			entpos[1] = cent->current.origin[1];
			entpos[2] = cent->current.origin[2];
			campos[0] = ps->pmove.origin[0] / 8;
			campos[1] = ps->pmove.origin[1] / 8;
			campos[2] = ps->pmove.origin[2] / 8 - 22;

			if (entpos[0] >= campos[0] - 10 && entpos[0] <= campos[0] + 10 &&
				entpos[1] >= campos[1] - 10 && entpos[1] <= campos[1] + 10 &&
				entpos[2] >= campos[2] - 5 && entpos[2] <= campos[2] + 30)
			{
				ent.flags |= RF_TRANSLUCENT; // hide players when you're inside of them
				ent.alpha = 0.0f;
			}
		}

		if (effects & EF_BFG)
		{
			ent.flags |= RF_TRANSLUCENT;
			ent.alpha = 0.30;
		}

		// RAFAEL
		if (effects & EF_PLASMA)
		{
			ent.flags |= RF_TRANSLUCENT;
			ent.alpha = 0.6;
		}

		if (effects & EF_SPHERETRANS)
		{
			ent.flags |= RF_TRANSLUCENT;
			// PMM - *sigh*  yet more EF overloading
			if (effects & EF_TRACKERTRAIL)
				ent.alpha = 0.6;
			else
				ent.alpha = 0.3;
		}
//pmm
		if (s1->modelindex2) // jitskm -- weapon model gets attached to player hand (temporary hack)
		{
			if (s1->modelindex2 == 255)
			{	// custom weapon
				ci = &cl.clientinfo[s1->skinnum & 0xff];
				i = (s1->skinnum >> 8); // 0 is default weapon model

				if (!cl_vwep->value || i > MAX_CLIENTWEAPONMODELS - 1)
					i = 0;

				ent.weapon_model = ci->weaponmodel[i];

				if (!ent.weapon_model)
				{
					if (i != 0)
						ent.weapon_model = ci->weaponmodel[0];

					if (!ent.weapon_model)
						ent.weapon_model = cl.baseclientinfo.weaponmodel[0];
				}
			}
			else
			{
				ent.weapon_model = cl.model_draw[s1->modelindex2];
			}
		}
		else
		{
			ent.weapon_model = NULL;
		}

		// add to refresh list
		V_AddEntity(&ent);


		// color shells generate a seperate entity for the main model
		if (effects & EF_COLOR_SHELL)
		{
			// PMM - at this point, all of the shells have been handled
			// if we're in the rogue pack, set up the custom mixing, otherwise just
			// keep going
//			if (Developer_searchpath(2) == 2)
//			{
				// all of the solo colors are fine.  we need to catch any of the combinations that look bad
				// (double & half) and turn them into the appropriate color, and make double/quad something special
				if (renderfx & RF_SHELL_HALF_DAM)
				{
					if (Developer_searchpath(2) == 2)
					{
						// ditch the half damage shell if any of red, blue, or double are on
						if (renderfx & (RF_SHELL_RED|RF_SHELL_BLUE|RF_SHELL_DOUBLE))
							renderfx &= ~RF_SHELL_HALF_DAM;
					}
				}

				if (renderfx & RF_SHELL_DOUBLE)
				{
					if (Developer_searchpath(2) == 2)
					{
						// lose the yellow shell if we have a red, blue, or green shell
						if (renderfx & (RF_SHELL_RED|RF_SHELL_BLUE|RF_SHELL_GREEN))
							renderfx &= ~RF_SHELL_DOUBLE;
						// if we have a red shell, turn it to purple by adding blue
						if (renderfx & RF_SHELL_RED)
							renderfx |= RF_SHELL_BLUE;
						// if we have a blue shell (and not a red shell), turn it to cyan by adding green
						else if (renderfx & RF_SHELL_BLUE)
							// go to green if it's on already, otherwise do cyan (flash green)
							if (renderfx & RF_SHELL_GREEN)
								renderfx &= ~RF_SHELL_BLUE;
							else
								renderfx |= RF_SHELL_GREEN;
					}
				}
//			}
			// pmm
			ent.flags = renderfx | RF_TRANSLUCENT;
			ent.alpha = 0.30;
			V_AddEntity(&ent);
		}

		//ent.skin = NULL;		// never use a custom skin on others
		memset(ent.skins, 0, sizeof(ent.skins)); // jitskm
		ent.skinnum = 0;
		ent.flags = 0;
		ent.alpha = 0;

		if (r_oldmodels->value) // jitskm
		{
			// duplicate for linked models
			if (s1->modelindex2)
			{
				if (s1->modelindex2 == 255)
				{	// custom weapon
					ci = &cl.clientinfo[s1->skinnum & 0xff];
					i = (s1->skinnum >> 8); // 0 is default weapon model

					if (!cl_vwep->value || i > MAX_CLIENTWEAPONMODELS - 1)
						i = 0;

					ent.model = ci->weaponmodel[i];

					if (!ent.model)
					{
						if (i != 0)
							ent.model = ci->weaponmodel[0];

						if (!ent.model)
							ent.model = cl.baseclientinfo.weaponmodel[0];
					}
				}
				else
				{
					ent.model = cl.model_draw[s1->modelindex2];
				}

				V_AddEntity(&ent);

				//PGM - make sure these get reset.
				ent.flags = 0;
				ent.alpha = 0;
				//PGM
			}
		}

		if (s1->modelindex3)
		{
			ent.model = cl.model_draw[s1->modelindex3];
			V_AddEntity(&ent);
		}

		if (s1->modelindex4)
		{
			ent.model = cl.model_draw[s1->modelindex4];
			V_AddEntity(&ent);
		}

		if (effects & EF_POWERSCREEN)
		{
			ent.oldframe = 0;
			ent.frame = 0;
			ent.flags |= (RF_TRANSLUCENT | RF_SHELL_GREEN);
			ent.alpha = 0.30;
			V_AddEntity(&ent);
		}

		// add automatic particle trails
		if ((effects&~EF_ROTATE))
		{
			if (effects & EF_ROCKET)
			{
				CL_RocketTrail(cent->lerp_origin, ent.origin, cent);
				V_AddLight(ent.origin, 200, 1, 1, 0);
			}
			// PGM - Do not reorder EF_BLASTER and EF_HYPERBLASTER. 
			// EF_BLASTER | EF_TRACKER is a special case for EF_BLASTER2... Cheese!
			else if (effects & EF_BLASTER)
			{
//PGM
				if (effects & EF_TRACKER)	// lame... problematic?
				{
					CL_BlasterTrail2(cent->lerp_origin, ent.origin);
					V_AddLight(ent.origin, 200, 0, 1, 0);		
				}
				else
				{
					CL_BlasterTrail(cent->lerp_origin, ent.origin);
					V_AddLight(ent.origin, 200, 1, 1, 0);
				}
//PGM
			}
			else if (effects & EF_HYPERBLASTER)
			{
				if (effects & EF_TRACKER)						// PGM	overloaded for blaster2.
					V_AddLight(ent.origin, 200, 0, 1, 0);		// PGM
				else											// PGM
					V_AddLight(ent.origin, 200, 1, 1, 0);
			}
			else if (effects & EF_GIB)
			{
				CL_DiminishingTrail(cent->lerp_origin, ent.origin, cent, effects);
			}
			else if (effects & EF_GRENADE)
			{
				CL_DiminishingTrail(cent->lerp_origin, ent.origin, cent, effects);
			}
			else if (effects & EF_FLIES)
			{
				CL_FlyEffect(cent, ent.origin);
			}
			else if (effects & EF_BFG)
			{
				static int bfg_lightramp[6] = {300, 400, 600, 300, 150, 75};

				if (effects & EF_ANIM_ALLFAST)
				{
					CL_BfgParticles(&ent);
					i = 200;
				}
				else
				{
					i = bfg_lightramp[s1->frame];
				}
				V_AddLight (ent.origin, i, 0, 1, 0);
			}
			// RAFAEL
			else if (effects & EF_TRAP)
			{
				ent.origin[2] += 32;
				CL_TrapParticles (&ent);
				i = (rand()%100) + 100;
				V_AddLight (ent.origin, i, 1, 0.8, 0.1);
			}
			else if (effects & EF_FLAG1)
			{
				CL_FlagTrail (cent->lerp_origin, ent.origin, 242);
				V_AddLight (ent.origin, 225, 1, 0.1, 0.1);
			}
			else if (effects & EF_FLAG2)
			{
				CL_FlagTrail (cent->lerp_origin, ent.origin, 115);
				V_AddLight (ent.origin, 225, 0.1, 0.1, 1);
			}
//======
//ROGUE
			else if (effects & EF_TAGTRAIL)
			{
				CL_TagTrail (cent->lerp_origin, ent.origin, 220);
				V_AddLight (ent.origin, 225, 1.0, 1.0, 0.0);
			}
			else if (effects & EF_TRACKERTRAIL)
			{
				if (effects & EF_TRACKER)
				{
					float intensity;

					intensity = 50.0f + (500.0f * ((float)sin(cl.time / 500.0f) + 1.0f));

					// FIXME - check out this effect in rendition
					if (vidref_val == VIDREF_GL)
						V_AddLight(ent.origin, intensity, -1.0f, -1.0f, -1.0f);
					else
						V_AddLight(ent.origin, -1.0f * intensity, 1.0f, 1.0f, 1.0f);
					}
				else
				{
					CL_Tracker_Shell(cent->lerp_origin);
					V_AddLight(ent.origin, 155.0f, -1.0f, -1.0f, -1.0f);
				}
			}
			else if (effects & EF_TRACKER)
			{
				CL_TrackerTrail(cent->lerp_origin, ent.origin, 0);

				// FIXME - check out this effect in rendition
				if (vidref_val == VIDREF_GL)
					V_AddLight(ent.origin, 200.0f, -1.0f, -1.0f, -1.0f);
				else
					V_AddLight(ent.origin, -200.0f, 1.0f, 1.0f, 1.0f);
			}
//ROGUE
//======
			// RAFAEL
			else if (effects & EF_GREENGIB)
			{
				CL_DiminishingTrail(cent->lerp_origin, ent.origin, cent, effects);				
			}
			// RAFAEL
			else if (effects & EF_IONRIPPER)
			{
				CL_IonripperTrail(cent->lerp_origin, ent.origin);
				V_AddLight(ent.origin, 100.0f, 1.0f, 0.5f, 0.5f);
			}
			// RAFAEL
			else if (effects & EF_BLUEHYPERBLASTER)
			{
				V_AddLight(ent.origin, 200.0f, 0.0f, 0.0f, 1.0f);
			}
			// RAFAEL
			else if (effects & EF_PLASMA)
			{
				if (effects & EF_ANIM_ALLFAST)
				{
					CL_BlasterTrail(cent->lerp_origin, ent.origin);
				}

				V_AddLight(ent.origin, 130.0f, 1.0f, 0.5f, 0.5f);
			}
		}

		VectorCopy(ent.origin, cent->lerp_origin);
	}
}



/*
==============
CL_AddViewWeapon
==============
*/
void CL_AddViewWeapon (player_state_t *ps, player_state_t *ops)
{
	entity_t	gun;		// view model
	int			i;

	// allow the gun to be completely removed
	if (!cl_gun->value)
		return;

	// don't draw gun if in wide angle view
	//if (ps->fov > 90)
	//	return;

	memset(&gun, 0, sizeof(gun));

	if (gun_model)
		gun.model = gun_model;	// development tool
	else
		gun.model = cl.model_draw[ps->gunindex];

	if (!gun.model)
		return;

	// set up gun position
	for (i = 0; i < 3; i++)
	{
		gun.origin[i] = cl.refdef.vieworg[i] + ops->gunoffset[i]
			+ cl.lerpfrac * (ps->gunoffset[i] - ops->gunoffset[i]);
		gun.angles[i] = cl.refdef.viewangles[i]; /* jitweapon - fix weird angles + LerpAngle (ops->gunangles[i],
			ps->gunangles[i], cl.lerpfrac);*/
	}

	if (gun_frame)
	{
		gun.frame = gun_frame;	// development tool
		gun.oldframe = gun_frame;	// development tool
	}
	else
	{
		gun.frame = ps->gunframe;

		if (gun.frame == 0)
			gun.oldframe = 0;	// just changed weapons, don't lerp from old
		else
			gun.oldframe = ops->gunframe;
	}

	gun.flags = RF_MINLIGHT | RF_DEPTHHACK | RF_WEAPONMODEL;
	gun.backlerp = 1.0f - cl.lerpfrac;
	VectorCopy(gun.origin, gun.oldorigin);	// don't lerp at all
	V_AddEntity(&gun);
}


/*
===============
CL_CalcViewValues

Sets cl.refdef view values
===============
*/
void CL_CalcViewValues (void)
{
	int			i;
	float		lerp, backlerp;
	centity_t	*ent;
	frame_t		*oldframe;
	player_state_t	*ps, *ops;

	// find the previous frame to interpolate from
	ps = &cl.frame.playerstate;
	i = (cl.frame.serverframe - 1) & UPDATE_MASK;
	oldframe = &cl.frames[i];
	if (oldframe->serverframe != cl.frame.serverframe-1 || !oldframe->valid)
		oldframe = &cl.frame;		// previous frame was dropped or involid
	ops = &oldframe->playerstate;

	// see if the player entity was teleported this frame
	if (fabs(ops->pmove.origin[0] - ps->pmove.origin[0]) > 256*8
		|| abs(ops->pmove.origin[1] - ps->pmove.origin[1]) > 256*8
		|| abs(ops->pmove.origin[2] - ps->pmove.origin[2]) > 256*8)
	{
		ops = ps;		// don't interpolate
	}

	ent = &cl_entities[cl.playernum+1];
	lerp = cl.lerpfrac;

	// calculate the origin
	if (cl_predict->value && !cl.attractloop && 	// jitdemo -- fixed jerky/choppy playback
		!(cl.frame.playerstate.pmove.pm_flags & PMF_NO_PREDICTION))
	{	// use predicted values
		backlerp = 1.0f - lerp;

		for (i = 0; i < 3; i++)
		{
			cl.refdef.vieworg[i] = cl.predicted_origin[i] + ops->viewoffset[i] 
				+ cl.lerpfrac * (ps->viewoffset[i] - ops->viewoffset[i])
				- backlerp * cl.prediction_error[i];
		}

		// smooth out stair climbing
		cl.refdef.vieworg[2] -= (cl.predicted_step + cl.predicted_step_unsent); // jitmove - new stair smoothing
	}
	else
	{	// just use interpolated values
		for (i = 0; i < 3; i++)
		{
			cl.refdef.vieworg[i] = ops->pmove.origin[i] * 0.125f + ops->viewoffset[i] 
				+ lerp * (ps->pmove.origin[i] * 0.125f + ps->viewoffset[i] 
				- (ops->pmove.origin[i] * 0.125f + ops->viewoffset[i]));
		}
	}

	// if not running a demo or on a locked frame, add the local angle movement
	if (cl.frame.playerstate.pmove.pm_type < PM_DEAD && cl_predict->value)
	{	// use predicted values
		for (i = 0; i < 3; i++)
			cl.refdef.viewangles[i] = cl.predicted_angles[i];
	}
	else
	{	// just use interpolated values
		for (i = 0; i < 3; i++)
			cl.refdef.viewangles[i] = LerpAngle(ops->viewangles[i], ps->viewangles[i], lerp);
	}

	for (i = 0; i < 3; i++)
		cl.refdef.viewangles[i] += LerpAngle(ops->kick_angles[i], ps->kick_angles[i], lerp);

	AngleVectors(cl.refdef.viewangles, cl.v_forward, cl.v_right, cl.v_up);

	// interpolate field of view
	cl.refdef.fov_x = ops->fov + lerp * (ps->fov - ops->fov);

	// don't interpolate blend color
	for (i = 0; i < 4; i++)
		cl.refdef.blend[i] = ps->blend[i];

	// add the weapon
	CL_AddViewWeapon(ps, ops);
}

/*
===============
CL_AddEntities

Emits all entities, particles, and lights to the refresh
===============
*/
void CL_AddEntities (void)
{
	if (cls.state != ca_active)
		return;

	if (cl.time > cl.frame.servertime)
	{
		if (cl_showclamp->value)
			Com_Printf("high clamp %i\n", cl.time - cl.frame.servertime);
		cl.time = cl.frame.servertime;
		cl.lerpfrac = 1.0;
	}
	else if (cl.time < cl.frame.servertime - 100)
	{
		if (cl_showclamp->value)
			Com_Printf("low clamp %i\n", cl.frame.servertime-100 - cl.time);
		cl.time = cl.frame.servertime - 100;
		cl.lerpfrac = 0;
	}
	else
		cl.lerpfrac = 1.0 - (cl.frame.servertime - cl.time) * 0.01;

	if (cl_timedemo->value)
		cl.lerpfrac = 1.0;

	CL_CalcViewValues();
	// PMM - moved this here so the heat beam has the right values for the vieworg, and can lock the beam to the gun
	CL_AddPacketEntities(&cl.frame);
	CL_AddViewLocs(); // Xile/NiceAss LOC
	CL_AddTEnts();
	CL_AddParticles();
	CL_AddDLights();
	CL_AddLightStyles();
}



/*
===============
CL_GetEntitySoundOrigin

Called to get the sound spatialization origin
===============
*/
void CL_GetEntitySoundOrigin (int ent, vec3_t org)
{
	centity_t	*old;

	if (ent < 0 || ent >= MAX_EDICTS)
		Com_Error(ERR_DROP, "CL_GetEntitySoundOrigin: bad ent");

	old = &cl_entities[ent];
	VectorCopy(old->lerp_origin, org);

	// FIXME: bmodel issues...
}
