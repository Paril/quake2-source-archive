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

#include "g_local.h"
#include "m_player.h"



static	edict_t		*current_player;
static	gclient_t	*current_client;

static	vec3_t	forward, right, up;
float	xyspeed;

float	bobmove;
int		bobcycle;		// odd cycles are right foot going forward
float	bobfracsin;		// sin(bobfrac*M_PI)

/*
===============
SV_CalcRoll

===============
*/
float SV_CalcRoll (vec3_t angles, vec3_t velocity)
{
	float	sign;
	float	side;
	float	value;
	
	side = DotProduct (velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);
	
	value = sv_rollangle->floatVal;

	if (side < sv_rollspeed->floatVal)
		side = side * value / sv_rollspeed->floatVal;
	else
		side = value;
	
	return side*sign;
	
}


/*
===============
P_DamageFeedback

Handles color blends and view kicks
===============
*/
void P_DamageFeedback (edict_t *player)
{
}

/*
===============
SV_CalcViewOffset

Auto pitching on slopes?

  fall from 128: 400 = 160000
  fall from 256: 580 = 336400
  fall from 384: 720 = 518400
  fall from 512: 800 = 640000
  fall from 640: 960 = 

  damage = deltavelocity*deltavelocity  * 0.0001

===============
*/
void SV_CalcViewOffset (edict_t *ent)
{
	float		*angles;
	float		bob;
	float		ratio;
	float		delta;
	vec3_t		v;


//===================================

	// base angles
	angles = ent->client->ps.kickAngles;

	// if dead, fix the angle and don't add any kick
	if (ent->deadflag)
	{
		Vec3Clear (angles);

		ent->client->ps.viewAngles[ROLL] = 40;
		ent->client->ps.viewAngles[PITCH] = -15;
		ent->client->ps.viewAngles[YAW] = ent->client->killer_yaw;
	}
	else
	{
		// add angles based on weapon kick

		Vec3Copy (ent->client->kick_angles, angles);

		// add angles based on damage kick

		ratio = (ent->client->v_dmg_time - level.time) / DAMAGE_TIME;
		if (ratio < 0)
		{
			ratio = 0;
			ent->client->v_dmg_pitch = 0;
			ent->client->v_dmg_roll = 0;
		}
		angles[PITCH] += ratio * ent->client->v_dmg_pitch;
		angles[ROLL] += ratio * ent->client->v_dmg_roll;

		// add pitch based on fall kick

		ratio = (ent->client->fall_time - level.time) / FALL_TIME;
		if (ratio < 0)
			ratio = 0;
		angles[PITCH] += ratio * ent->client->fall_value;

		// add angles based on velocity

		delta = DotProduct (ent->velocity, forward);
		angles[PITCH] += delta*run_pitch->floatVal;
		
		delta = DotProduct (ent->velocity, right);
		angles[ROLL] += delta*run_roll->floatVal;

		// add angles based on bob

		delta = bobfracsin * bob_pitch->floatVal * xyspeed;
		if (ent->client->ps.pMove.pmFlags & PMF_DUCKED)
			delta *= 6;		// crouching
		angles[PITCH] += delta;
		delta = bobfracsin * bob_roll->floatVal * xyspeed;
		if (ent->client->ps.pMove.pmFlags & PMF_DUCKED)
			delta *= 6;		// crouching
		if (bobcycle & 1)
			delta = -delta;
		angles[ROLL] += delta;
	}

//===================================

	// base origin

	Vec3Clear (v);

	// add view height

	v[2] += ent->viewheight;

	// add fall height

	ratio = (ent->client->fall_time - level.time) / FALL_TIME;
	if (ratio < 0)
		ratio = 0;
	v[2] -= ratio * ent->client->fall_value * 0.4;

	// add bob height

	bob = bobfracsin * xyspeed * bob_up->floatVal;
	if (bob > 6)
		bob = 6;
	//gi.DebugGraph (bob *2, 255);
	v[2] += bob;

	// add kick offset

	Vec3Add (v, ent->client->kick_origin, v);

	// absolutely bound offsets
	// so the view can never be outside the player box

	if (v[0] < -14)
		v[0] = -14;
	else if (v[0] > 14)
		v[0] = 14;
	if (v[1] < -14)
		v[1] = -14;
	else if (v[1] > 14)
		v[1] = 14;
	if (v[2] < -22)
		v[2] = -22;
	else if (v[2] > 30)
		v[2] = 30;

	Vec3Copy (v, ent->client->ps.viewOffset);
}

/*
==============
SV_CalcGunOffset
==============
*/
void SV_CalcGunOffset (edict_t *ent)
{
	int		i;
	float	delta;

	// gun angles from bobbing
	ent->client->ps.gunAngles[ROLL] = xyspeed * bobfracsin * 0.005;
	ent->client->ps.gunAngles[YAW] = xyspeed * bobfracsin * 0.01;
	if (bobcycle & 1)
	{
		ent->client->ps.gunAngles[ROLL] = -ent->client->ps.gunAngles[ROLL];
		ent->client->ps.gunAngles[YAW] = -ent->client->ps.gunAngles[YAW];
	}

	ent->client->ps.gunAngles[PITCH] = xyspeed * bobfracsin * 0.005;

	// gun angles from delta movement
	for (i=0 ; i<3 ; i++)
	{
		delta = ent->client->oldviewangles[i] - ent->client->ps.viewAngles[i];
		if (delta > 180)
			delta -= 360;
		if (delta < -180)
			delta += 360;
		if (delta > 45)
			delta = 45;
		if (delta < -45)
			delta = -45;
		if (i == YAW)
			ent->client->ps.gunAngles[ROLL] += 0.1*delta;
		ent->client->ps.gunAngles[i] += 0.2 * delta;
	}

	// gun height
	Vec3Clear (ent->client->ps.gunOffset);
//	ent->ps->gunorigin[2] += bob;

	// gun_x / gun_y / gun_z are development tools
	for (i=0 ; i<3 ; i++)
	{
		ent->client->ps.gunOffset[i] += forward[i]*(gun_y->floatVal);
		ent->client->ps.gunOffset[i] += right[i]*gun_x->floatVal;
		ent->client->ps.gunOffset[i] += up[i]* (-gun_z->floatVal);
	}
}


/*
=============
SV_AddBlend
=============
*/
void SV_AddBlend (float r, float g, float b, float a, float *v_blend)
{
	float	a2, a3;

	if (a <= 0)
		return;
	a2 = v_blend[3] + (1-v_blend[3])*a;	// new total alpha
	a3 = v_blend[3]/a2;		// fraction of color from old

	v_blend[0] = v_blend[0]*a3 + r*(1-a3);
	v_blend[1] = v_blend[1]*a3 + g*(1-a3);
	v_blend[2] = v_blend[2]*a3 + b*(1-a3);
	v_blend[3] = a2;
}


/*
=============
SV_CalcBlend
=============
*/
void SV_CalcBlend (edict_t *ent)
{
	int		contents;
	vec3_t	vieworg;
	int		remaining;

	ent->client->ps.viewBlend[0] = ent->client->ps.viewBlend[1] = 
		ent->client->ps.viewBlend[2] = ent->client->ps.viewBlend[3] = 0;

	// add for contents
	Vec3Add (ent->s.origin, ent->client->ps.viewOffset, vieworg);
	contents = gi.pointcontents (vieworg);
	if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER) )
		ent->client->ps.rdFlags |= RDF_UNDERWATER;
	else
		ent->client->ps.rdFlags &= ~RDF_UNDERWATER;

	if (contents & (CONTENTS_SOLID|CONTENTS_LAVA))
		SV_AddBlend (1.0f, 0.3f, 0.0f, 0.6f, ent->client->ps.viewBlend);
	else if (contents & CONTENTS_SLIME)
		SV_AddBlend (0.0f, 0.1f, 0.05f, 0.6f, ent->client->ps.viewBlend);
	else if (contents & CONTENTS_WATER)
		SV_AddBlend (0.5f, 0.3f, 0.2f, 0.4f, ent->client->ps.viewBlend);

	// add for powerups
	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 0, 1, 0.08f, ent->client->ps.viewBlend);
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (1, 1, 0, 0.08f, ent->client->ps.viewBlend);
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		remaining = ent->client->enviro_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 1, 0, 0.08f, ent->client->ps.viewBlend);
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		remaining = ent->client->breather_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0.4f, 1, 0.4f, 0.04f, ent->client->ps.viewBlend);
	}

	// add for damage
	if (ent->client->damage_alpha > 0)
		SV_AddBlend (ent->client->damage_blend[0],ent->client->damage_blend[1]
		,ent->client->damage_blend[2], ent->client->damage_alpha, ent->client->ps.viewBlend);

	if (ent->client->bonus_alpha > 0)
		SV_AddBlend (0.85f, 0.7f, 0.3f, ent->client->bonus_alpha, ent->client->ps.viewBlend);

	// drop the damage value
	ent->client->damage_alpha -= 0.06f;
	if (ent->client->damage_alpha < 0)
		ent->client->damage_alpha = 0;

	// drop the bonus value
	ent->client->bonus_alpha -= 0.1f;
	if (ent->client->bonus_alpha < 0)
		ent->client->bonus_alpha = 0;
}


/*
=================
P_FallingDamage
=================
*/
void P_FallingDamage (edict_t *ent)
{
}



/*
=============
P_WorldEffects
=============
*/
void P_WorldEffects (void)
{
}


/*
===============
G_SetClientEffects
===============
*/
void G_SetClientEffects (edict_t *ent)
{
	ent->s.effects = 0;
	ent->s.renderFx = 0;

	if (ent->health <= 0 || level.intermissiontime)
		return;

	if (ent->light_level < 20)
		ent->s.effects |= EF_SPHERETRANS;
	else if (ent->light_level < 60)
		ent->s.renderFx |= RF_TRANSLUCENT;

	// show cheaters!!!
	if (ent->flags & FL_GODMODE)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderFx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
	}

	if (level.seeker == ent)
	{
		ent->s.effects |= EF_HALF_DAMAGE;
	}
}


/*
===============
G_SetClientEvent
===============
*/
void G_SetClientEvent (edict_t *ent)
{
	if (ent->s.event)
		return;

	if ( ent->groundentity && xyspeed > 225)
	{
		if ( (int)(current_client->bobtime+bobmove) != bobcycle )
			ent->s.event = EV_FOOTSTEP;
	}
}

/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound (edict_t *ent)
{
}

/*
===============
G_SetClientFrame
===============
*/
void G_SetClientFrame (edict_t *ent)
{
	gclient_t	*client;
	bool	duck, run;

	if (ent->s.modelIndex != 255)
		return;		// not in the player model

	client = ent->client;

	if (client->ps.pMove.pmFlags & PMF_DUCKED)
		duck = true;
	else
		duck = false;
	if (xyspeed)
		run = true;
	else
		run = false;

	// check for stand/duck and stop/go transitions
	if (duck != client->anim_duck && client->anim_priority < ANIM_DEATH)
		goto newanim;
	if (run != client->anim_run && client->anim_priority == ANIM_BASIC)
		goto newanim;
	if (!ent->groundentity && client->anim_priority <= ANIM_WAVE)
		goto newanim;

	if(client->anim_priority == ANIM_REVERSE)
	{
		if(ent->s.frame > client->anim_end)
		{
			ent->s.frame--;
			return;
		}
	}
	else if (ent->s.frame < client->anim_end)
	{	// continue an animation
		ent->s.frame++;
		return;
	}

	if (client->anim_priority == ANIM_DEATH)
		return;		// stay there
	if (client->anim_priority == ANIM_JUMP)
	{
		if (!ent->groundentity)
			return;		// stay there
		ent->client->anim_priority = ANIM_WAVE;
		ent->s.frame = FRAME_jump3;
		ent->client->anim_end = FRAME_jump6;
		return;
	}

newanim:
	// return to either a running or standing frame
	client->anim_priority = ANIM_BASIC;
	client->anim_duck = duck;
	client->anim_run = run;

	if (!ent->groundentity)
	{
		client->anim_priority = ANIM_JUMP;
		if (ent->s.frame != FRAME_jump2)
			ent->s.frame = FRAME_jump1;
		client->anim_end = FRAME_jump2;
	}
	else if (run)
	{	// running
		if (duck)
		{
			ent->s.frame = FRAME_crwalk1;
			client->anim_end = FRAME_crwalk6;
		}
		else
		{
			ent->s.frame = FRAME_run1;
			client->anim_end = FRAME_run6;
		}
	}
	else
	{	// standing
		if (duck)
		{
			ent->s.frame = FRAME_crstnd01;
			client->anim_end = FRAME_crstnd19;
		}
		else
		{
			ent->s.frame = FRAME_stand01;
			client->anim_end = FRAME_stand40;
		}
	}
}


/*
=================
ClientEndServerFrame

Called for each player at the end of the server frame
and right after spawning
=================
*/
void ClientEndServerFrame (edict_t *ent)
{
	float	bobtime;
	int		i;

	current_player = ent;
	current_client = ent->client;

	//
	// If the origin or velocity have changed since ClientThink(),
	// update the pmove values.  This will happen when the client
	// is pushed by a bmodel or kicked by an explosion.
	// 
	// If it wasn't updated here, the view position would lag a frame
	// behind the body position when pushed -- "sinking into plats"
	//
	for (i=0 ; i<3 ; i++)
	{
		current_client->ps.pMove.origin[i] = ent->s.origin[i]*8.0;
		current_client->ps.pMove.velocity[i] = ent->velocity[i]*8.0;
	}

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if (level.intermissiontime)
	{
		// FIXME: add view drifting here?
		current_client->ps.viewBlend[3] = 0;
		current_client->ps.fov = 90;
		G_SetStats (ent);
		return;
	}

	Angles_Vectors (ent->client->v_angle, forward, right, up);

	// burn from lava, etc
	P_WorldEffects ();

	//
	// set model angles from view angles so other things in
	// the world can tell which direction you are looking
	//
	if (ent->client->v_angle[PITCH] > 180)
		ent->s.angles[PITCH] = (-360 + ent->client->v_angle[PITCH])/3;
	else
		ent->s.angles[PITCH] = ent->client->v_angle[PITCH]/3;
	ent->s.angles[YAW] = ent->client->v_angle[YAW];
	ent->s.angles[ROLL] = 0;
	ent->s.angles[ROLL] = SV_CalcRoll (ent->s.angles, ent->velocity)*4;

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	xyspeed = sqrtf(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);

	if (xyspeed < 5)
	{
		bobmove = 0;
		current_client->bobtime = 0;	// start at beginning of cycle again
	}
	else if (ent->groundentity)
	{	// so bobbing only cycles when on ground
		if (xyspeed > 210)
			bobmove = 0.25;
		else if (xyspeed > 100)
			bobmove = 0.125;
		else
			bobmove = 0.0625;
	}
	
	bobtime = (current_client->bobtime += bobmove);

	if (current_client->ps.pMove.pmFlags & PMF_DUCKED)
		bobtime *= 4;

	bobcycle = (int)bobtime;
	bobfracsin = fabs(sinf(bobtime*M_PI));

	// detect hitting the floor
	P_FallingDamage (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// determine the view offsets
	SV_CalcViewOffset (ent);

	// determine the gun offsets
	SV_CalcGunOffset (ent);

	// determine the full screen color blend
	// must be after viewoffset, so eye contents can be
	// accurately determined
	// FIXME: with client prediction, the contents
	// should be determined by the client
	SV_CalcBlend (ent);

	// chase cam stuff
	if (ent->client->resp.spectator)
		G_SetSpectatorStats(ent);
	else
		G_SetStats (ent);
	G_CheckChaseStats(ent);

	G_SetClientEvent (ent);

	G_SetClientEffects (ent);

	G_SetClientSound (ent);

	G_SetClientFrame (ent);

	Vec3Copy (ent->velocity, ent->client->oldvelocity);
	Vec3Copy (ent->client->ps.viewAngles, ent->client->oldviewangles);

	// clear weapon kicks
	Vec3Clear (ent->client->kick_origin);
	Vec3Clear (ent->client->kick_angles);

	if (level.timer_end != -1 && timelimit->floatVal && level.timer_end < level.time)
	{
		gi.bprintf (PRINT_CHAT, "Time's up! Nobody wins!\n");
		level.restartTime = level.time + 4;
	}

	if (level.hider && level.seeker && !level.notify)
	{
		level.notify = true;
		gi.bprintf (PRINT_CHAT, "==== Seeker and Hider are here, seeker go to a corner and hider hide! ====\n");

		gi.cprintf (level.seeker, PRINT_HIGH, "Seeker, stare at the floor for a bit.\n");
		gi.cprintf (level.hider, PRINT_HIGH, "Hider, go hide, then hit \"hiding\"\n");

		level.timer_end = -1;

		level.started_at = level.time;
	}

	if (level.hiding)
	{
		if (timelimit->floatVal && level.timer_end == -1)
			level.timer_end = level.time + timelimit->floatVal*60;
		if ((level.restartTime != -1))
		{
			if (level.time > level.restartTime)
			{
				edict_t *oldseek = level.seeker;
				level.seeker = level.hider;
				level.hider = oldseek;
				level.hiding = false;
				level.notify = false;

				level.restartTime = -1;
				respawn(level.seeker);
				respawn(level.hider);
				level.seconds = level.minutes = 0;
			}
		}
		else if (visible(level.seeker, level.hider) && infront(level.seeker, level.hider))
		{
			// Check if we're staring at this person
			vec3_t forward, end;
			cmTrace_t tr;

			Angles_Vectors (level.seeker->client->v_angle, forward, NULL, NULL);
			Vec3MA (level.seeker->s.origin, 8192, forward, end);
			tr = gi.trace (level.seeker->s.origin, NULL, NULL, end, level.seeker, (CONTENTS_SOLID|CONTENTS_MONSTER));

			if (tr.fraction < 1.0 && tr.ent && (tr.ent == level.hider))
			{
				gi.bprintf (PRINT_CHAT, "Seeker found the hider!! Switching players... game starts again!\n");

				level.restartTime = level.time + 4;

				// Give the player some points based on...				
				// Time
				int tl;
				if (!timelimit->floatVal)
					tl = clamp(((level.time - level.started_at) / 60), 0, 6);
				else
					tl = clamp(((level.time - level.timer_end) / level.started_at), 0, 6);

				ent->client->resp.score += tl;
			}
		}
	}
}

