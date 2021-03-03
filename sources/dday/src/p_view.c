/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/p_view.c,v $
 *   $Revision: 1.24 $
 *   $Date: 2002/07/12 00:52:57 $
 * 
 ***********************************

Copyright (C) 2002 Vipersoft

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

//bcass start - flamer sound thing
#define FLAMER1		0
#define FLAMER2		25
#define FLAMER3		50
#define FLAMER4		75
#define FLAMERH		100
//bcass end

void SV_AddBlend (float r, float g, float b, float a, float *v_blend);
qboolean Surface(char *name, int type);
qboolean strcmpwld (char *give, char *check);

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
	
	value = sv_rollangle->value;

	if (side < sv_rollspeed->value)
		side = side * value / sv_rollspeed->value;
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
	gclient_t	*client;
	float	side;
	float	realcount, count, kick;
	vec3_t	v;
	int		r, l;
	static	vec3_t	power_color = {0.0, 1.0, 0.0};
	static	vec3_t	acolor = {1.0, 1.0, 1.0};
	static	vec3_t	bcolor = {1.0, 0.0, 0.0};

//bcass start - flamer sound thing
	int			randnum;
//bcass end

	client = player->client;

	// flash the backgrounds behind the status numbers
	client->ps.stats[STAT_FLASHES] = 0;
	if (client->damage_blood)
		client->ps.stats[STAT_FLASHES] |= 1;
	if (client->damage_armor && !(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
		client->ps.stats[STAT_FLASHES] |= 2;

	// total points of damage shot at the player this frame
	count = (client->damage_blood + client->damage_armor + client->damage_parmor);
	if (count == 0)
		return;		// didn't take any damage

	// start a pain animation if still in the player model
	if (client->anim_priority < ANIM_PAIN && player->s.modelindex == 255)
	{
		static int		i;

		client->anim_priority = ANIM_PAIN;
		if (player->stanceflags==STANCE_DUCK)
		{
			player->s.frame = FRAME_crpain1-1;
			client->anim_end = FRAME_crpain4;
		}
		else if(player->stanceflags==STANCE_CRAWL)
		{
			player->s.frame=FRAME_crawlpain01-1;
			client->anim_end = FRAME_crawlpain04;
		}
		else
		{
			i = (i+1)%3;
			switch (i)
			{
			case 0:
				player->s.frame = FRAME_pain101-1;
				client->anim_end = FRAME_pain104;
				break;
			case 1:
				player->s.frame = FRAME_pain201-1;
				client->anim_end = FRAME_pain204;
				break;
			case 2:
				player->s.frame = FRAME_pain301-1;
				client->anim_end = FRAME_pain304;
				break;
			}
		}
	}

	realcount = count;
	if (count < 10)
		count = 10;	// always make a visible effect

	// play an apropriate pain sound
	if ((level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
	{

	/*-----/ PM /-----/ MODIFIED:  Scream if on fire! /-----*/
		if (player->burnout > level.time)
		{ 
			player->pain_debounce_time = level.time + 0.7;

//bcass start - flamer sound thing - copy/updated medic sound thing
		srand(rand());
		randnum=rand()%100;
		
		//let the fun begin defining sounds
		if(randnum > FLAMER1 && randnum < FLAMER2) {
			gi.sound (current_player, CHAN_WEAPON, gi.soundindex ("player/burn1.wav"), 1.0, ATTN_NORM, 0);
		} else if (randnum > FLAMER2 && randnum < FLAMER3) {
			gi.sound (current_player, CHAN_WEAPON, gi.soundindex ("player/burn2.wav"), 1.0, ATTN_NORM, 0);
		} else if (randnum > FLAMER3 && randnum < FLAMER4) {
			gi.sound (current_player, CHAN_WEAPON, gi.soundindex ("player/burn3.wav"), 1.0, ATTN_NORM, 0);
		} else if (randnum > FLAMER4 && randnum < FLAMERH) {
			gi.sound (current_player, CHAN_WEAPON, gi.soundindex ("player/burn4.wav"), 1.0, ATTN_NORM, 0);
		} else {
			gi.sound (current_player, CHAN_WEAPON, gi.soundindex ("player/burn1.wav"), 1.0, ATTN_NORM, 0);
		}
//bcass end	
			
//			if (rand()&1)
//				gi.sound (player, CHAN_VOICE, gi.soundindex("player/burn1.wav"), 1, ATTN_NORM, 0);
//			else
//				gi.sound (player, CHAN_VOICE, gi.soundindex("player/burn2.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			r = 1 + (rand()&1);
			player->pain_debounce_time = level.time + 0.7;
			if (player->health < 25)
				l = 25;
			else if (player->health < 50)
				l = 50;
			else if (player->health < 75)
				l = 75;
			else
				l = 100;
                        gi.sound (player, CHAN_VOICE, gi.soundindex(va("*pain%i_%i.wav", l, r)), 1, ATTN_NORM, 0);
		}
	/*------------------------------------------------------*/

	}

	// the total alpha of the blend is always proportional to count
	if (client->damage_alpha < 0)
		client->damage_alpha = 0;
	client->damage_alpha += count*0.01;
	if (client->damage_alpha < 0.2)
		client->damage_alpha = 0.2;
	// rezmoth - was 0.6
	if (client->damage_alpha > 0.3)
		client->damage_alpha = 0.3;		// don't go too saturated

	// the color of the blend will vary based on how much was absorbed
	// by different armors
	VectorClear (v);
	if (client->damage_parmor)
		VectorMA (v, (float)client->damage_parmor/realcount, power_color, v);
	if (client->damage_armor)
		VectorMA (v, (float)client->damage_armor/realcount,  acolor, v);
	if (client->damage_blood)
		VectorMA (v, (float)client->damage_blood/realcount,  bcolor, v);
	VectorCopy (v, client->damage_blend);


	//
	// calculate view angle kicks
	//
	kick = abs(client->damage_knockback);
	if (kick && player->health > 0)	// kick of 0 means no view adjust at all
	{
		kick = kick * 100 / player->health;

		if (kick < count*0.5)
			kick = count*0.5;
		if (kick > 50)
			kick = 50;

		VectorSubtract (client->damage_from, player->s.origin, v);
		VectorNormalize (v);
		
		side = DotProduct (v, right);
		client->v_dmg_roll = kick*side*0.3;
		
		side = -DotProduct (v, forward);
		client->v_dmg_pitch = kick*side*0.3;

		// divided by 3 to speed up knockback
		client->v_dmg_time = level.time + (DAMAGE_TIME / 2);
	}


	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_parmor = 0;
	client->damage_knockback = 0;
}


/*
===============
P_ExplosionEffects

Handles explosion kicks, colors, and sounds

2001-10-09 pbowens: original
===============
*/

void P_ExplosionEffects (edict_t *player)
{
	int frame;
	int intensity;
	//vec3_t	v;

	frame = level.framenum - player->client->dmgef_startframe;

	if (frame > ((SWAY_BREAK * SWAY_MULTI) + SWAY_START) || !player->client->dmgef_intensity)
	{
		player->client->dmgef_intensity		= 0;
		player->client->dmgef_sway_value	= 0;
		player->client->dmgef_sway_switch	= 0;
		player->client->dmgef_ablend		= 0;
		player->client->dmgef_flash			= 0;

		return;
	}

	if (player->client->dmgef_intensity < 0) // negative numbers are
		player->client->dmgef_intensity = 50;

	intensity = (int)(player->client->dmgef_intensity / 10);
	//gi.dprintf("P_ExplosionEffects::intensity: %i\n",intensity);

	if (frame <= (SWAY_BREAK * SWAY_MULTI) + SWAY_START) { // start/finish view kicks & color blend

		// Start the rumble sound
		if (frame == 1)
			gi.sound (player, CHAN_AUTO, gi.soundindex("misc/rumble.wav"), 1, ATTN_STATIC, 0);

		// All this could probably be replaced with a simple sin/cosin function
		// however, i'm lacking the ambition to actually figure it out

		// if it aint broke dont fix it

		if (frame > SWAY_START) {
			if (player->client->dmgef_sway_value == 999) { // recently modified
				player->client->dmgef_sway_switch = 0;
			}

			//gi.dprintf("sway_switch: %i\nsway_value: %i\n", player->client->dmgef_sway_switch, player->client->dmgef_sway_value);

			// This causes the views to go back and forth 
			switch (player->client->dmgef_sway_switch) 
			{
			case 0: // unset, continue the patern defined above
				player->client->dmgef_sway_switch		= 
					player->client->dmgef_sway_value	= 
						(intensity % 2) ? 1 : -1;

				break;
			case 1: // was negative, now start incrementing
				player->client->dmgef_sway_value++;
			
				if (player->client->dmgef_sway_value == SWAY_BREAK )
					player->client->dmgef_sway_switch = -1;	// go negative

				break;
			case -1: // was positive, now decrement
				player->client->dmgef_sway_value--;

				if (player->client->dmgef_sway_value == -SWAY_BREAK )
					player->client->dmgef_sway_switch = 1;	// go positive

				break;
			default: // erroneous sway value, dont bother
				return;
				break;
			}
		}

		/* pbowens
			I know this looks complicated, and it is,  but after some digestion you can get the hang of it.
			Oh yeah, and I DIDNT USE ANY ADVANCED MATH !!
			(but rez did)

			PITCH:	1) initial frame is large jolt for explosion (intensity is a good value)
					2) the rest of the frames (up to frame 5, where the sequence ends), just bob up and down
						gradually decaying because of frame's constant progression

			YAW:	1) do nothing for the jolt
					2) use the sway value calculated above in the sway frametime 

			ROLL:	1) initially (first 3 frames) help out with jolt based on intensity
					2) afterwards, help out with the sway by complimenting the yaw in a lesser degree

			Thats it.
		*/

		// rezmoth - proof of concept
		//gi.dprintf("frame %f, max %f, amount %f\n", (float)(frame), (float)(SWAY_BREAK * SWAY_MULTI + SWAY_START), (float)(1 - (float)(frame) / (SWAY_BREAK * SWAY_MULTI + SWAY_START)));
/*
		player->client->explosion_angles[PITCH] = (frame == 0)?intensity * 1.65:sin(frame) * 6 * (1 - (float)(frame) / (SWAY_BREAK * SWAY_MULTI + SWAY_START));
*/
		player->client->explosion_angles[PITCH] =(frame == 1) ?											\
												intensity * 1.65									\
											:	(frame < SWAY_START) ?								\
													(frame % 2) ? frame : -frame					\
												:	0;
/*
		player->client->explosion_angles[YAW] =	(frame > SWAY_START) ?				\
												sin((float)(frame * 0.8)) * 4 * (1 - (float)(frame) / (SWAY_BREAK * SWAY_MULTI + SWAY_START))					\
											:	 0;
*/
		player->client->explosion_angles[YAW] =	(frame > SWAY_START && intensity > 15) ?				\
												sin(frame + 145) * (intensity / 10) * 2 * (1 - (float)(frame) / (SWAY_BREAK * SWAY_MULTI + SWAY_START))					\
											:	 0;
/*
		player->client->explosion_angles[ROLL] =	(frame == 0) ?											\
												(frame % 2) ?										\
													intensity *  0.5								\
												:	intensity * -0.5								\
											:	(frame > SWAY_START) ?			\
													(intensity % 2) ?								\
														sin(frame - 4) * 4 * (1 - (float)(frame) / (SWAY_BREAK * SWAY_MULTI + SWAY_START))	\
													:	sin(frame - 4) * 4 * (1 - (float)(frame) / (SWAY_BREAK * SWAY_MULTI + SWAY_START))	\
												:	0;
*/
		player->client->explosion_angles[ROLL] =	(frame < 3) ?											\
												(frame % 2) ?										\
													intensity *  0.5								\
												:	intensity * -0.5								\
											:	(frame > SWAY_START && intensity > 15) ?			\
													(intensity % 2) ?								\
														sin(frame + 90) * (intensity / 10) * 2 *  0.35 * (1 - (float)(frame) / (SWAY_BREAK * SWAY_MULTI + SWAY_START))	\
													:	sin(frame + 90) * (intensity / 10) * 2 * -0.35 * (1 - (float)(frame) / (SWAY_BREAK * SWAY_MULTI + SWAY_START))	\
												:	0;

		if (frame > 0 && frame < 11 && player->client->dmgef_flash == true) {
			if (frame == 3) // play ears ringing
				gi.sound (player, CHAN_AUTO, gi.soundindex("misc/ring.wav"), 1, ATTN_STATIC, 0);

			player->client->dmgef_ablend = (10 - frame) * 0.01;
		} else
			player->client->dmgef_ablend = 0;

	}

	return;
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
//	int			anglesave;


//===================================

	// base angles
	angles = ent->client->ps.kick_angles;

	
	
	// if dead, fix the angle and don't add any kick
	if (ent->deadflag)
	{
		VectorClear (angles);

		ent->client->ps.viewangles[ROLL] = 40;
		ent->client->ps.viewangles[PITCH] = -15;
		ent->client->ps.viewangles[YAW] = ent->client->killer_yaw;
	}
	
	
	else
	{
		// add angles based on weapon kick

		VectorCopy (ent->client->kick_angles, angles);

		//faf:  separated kick_angles and explosion_angles calculations so explosions
		//      dont screw up the smg kick
		VectorAdd (angles, ent->client->explosion_angles, angles);


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
		angles[PITCH] += delta*run_pitch->value;
		
		delta = DotProduct (ent->velocity, right);
		angles[ROLL] += delta*run_roll->value;

		// add angles based on bob

		delta = bobfracsin * bob_pitch->value * xyspeed;
		//if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		if (ent->stanceflags==STANCE_DUCK)
			delta *= 4;		// crouching
		else if(ent->stanceflags==STANCE_CRAWL)
			delta *= 6;		// prone
		angles[PITCH] += delta;

		delta = bobfracsin * bob_roll->value * xyspeed;
		if (ent->stanceflags==STANCE_DUCK)
			delta *= 4;		// crouching
		else if(ent->stanceflags==STANCE_CRAWL)
			delta *= 6;		// prone
		
		if (bobcycle & 1)
			delta = -delta;
		angles[ROLL] += delta;
	 }

//===================================

	// base origin

	VectorClear (v);

	// add view height

	v[2] += ent->viewheight;


/*
faf:  this mades it so people's view turns in a circle instead of on a pin point.
       some people didnt like it so removing it.

  anglesave = v[2];

    AngleVectors (ent->s.angles, forward, NULL, NULL);
    VectorNormalize (forward);

	if(ent->stanceflags == STANCE_CRAWL)
		VectorScale (forward, 10, forward);
	else
		VectorScale (forward, 5, forward);
		

    VectorAdd (v, forward, v);

	if (v[2] < (anglesave -2))
		v[2] = anglesave -2;

*/


	// add fall height

	ratio = (ent->client->fall_time - level.time) / FALL_TIME;
	if (ratio < 0)
		ratio = 0;
	v[2] -= ratio * ent->client->fall_value * 0.4;

	// add bob height

	bob = bobfracsin * xyspeed * bob_up->value;
	if (bob > 6)
		bob = 6;
	//gi.DebugGraph (bob *2, 255);
	v[2] += bob;

	// add kick offset

	VectorAdd (v, ent->client->kick_origin, v);

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

	VectorCopy (v, ent->client->ps.viewoffset);
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
	ent->client->ps.gunangles[ROLL] = xyspeed * bobfracsin * 0.005;
	ent->client->ps.gunangles[YAW] = xyspeed * bobfracsin * 0.01;
	if (bobcycle & 1)
	{
		ent->client->ps.gunangles[ROLL] = -ent->client->ps.gunangles[ROLL];
		ent->client->ps.gunangles[YAW] = -ent->client->ps.gunangles[YAW];
	}

	ent->client->ps.gunangles[PITCH] = xyspeed * bobfracsin * 0.005;

	
	if (ent->client->last_jump_time > level.time - 3)
	{

		if (4 *(level.time - ent->client->last_jump_time) < (3.1416))
		{
			ent->client->ps.gunangles[PITCH] = 
			ent->client->ps.gunangles[PITCH] - 8 * (sin (4 *(level.time - ent->client->last_jump_time)));
		}
	}


	/*  make this a model change later
	//faf:  mauser tweak
	if (ent->client->pers.weapon &&
		!strcmp(ent->client->pers.weapon->classname, "weapon_mauser98k"))
		ent->client->ps.gunangles[PITCH] = ent->client->ps.gunangles[PITCH] - .7;
*/

	// gun angles from delta movement
	for (i=0 ; i<3 ; i++)
	{
		delta = ent->client->oldviewangles[i] - ent->client->ps.viewangles[i];
		if (delta > 180)
			delta -= 360;
		if (delta < -180)
			delta += 360;
		if (delta > 45)
			delta = 45;
		if (delta < -45)
			delta = -45;
		if (i == YAW)
			ent->client->ps.gunangles[ROLL] += 0.1*delta;
		ent->client->ps.gunangles[i] += 0.2 * delta;
	}

	// gun height
	VectorClear (ent->client->ps.gunoffset);
//	ent->ps->gunorigin[2] += bob;

	// gun_x / gun_y / gun_z are development tools
	for (i=0 ; i<3 ; i++)
	{
		ent->client->ps.gunoffset[i] += forward[i]*(gun_y->value);
		ent->client->ps.gunoffset[i] += right[i]*gun_x->value;
		ent->client->ps.gunoffset[i] += up[i]* (-gun_z->value);
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
//	float	blendtime_remain;

	ent->client->ps.blend[0] = ent->client->ps.blend[1] = 
		ent->client->ps.blend[2] = ent->client->ps.blend[3] = 0;

	// add for contents
	VectorAdd (ent->s.origin, ent->client->ps.viewoffset, vieworg);
	contents = gi.pointcontents (vieworg);
	if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER) )
		ent->client->ps.rdflags |= RDF_UNDERWATER;
	else
		ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	// make blind if on fire
	if (ent->burnout)
		SV_AddBlend (0.9, 0.9, 0.6, 0.84, ent->client->ps.blend);

	// fade into lobby
	if (level.framenum < ((int)level_wait->value * 10))
	{
		SV_AddBlend (0.0, 0.0, 0.0, (1.0 - (float)(level.framenum / (level_wait->value * 10.0))), ent->client->ps.blend);
	}

	// fade to black if dead
	if (ent->deadflag)
	{
		if (ent->client->resp.deathblend < 1)
			ent->client->resp.deathblend += 0.03;
		if (ent->client->resp.deathblend > 1)
			ent->client->resp.deathblend = 1;
		SV_AddBlend (0.5 - (0.5 * ent->client->resp.deathblend), 0.0, 0.0, ent->client->resp.deathblend, ent->client->ps.blend);

	} else if (ent->client->resp.deathblend) {
		ent->client->resp.deathblend = 0;
	}

	if (contents & (CONTENTS_SOLID|CONTENTS_LAVA))
		SV_AddBlend (1.0, 0.3, 0.0, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_SLIME)
		SV_AddBlend (0.0, 0.1, 0.05, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_WATER)
		SV_AddBlend (0.5, 0.3, 0.2, 0.4, ent->client->ps.blend);
	//else if (ent->client->display_info)	// pbowens: objectives
		//SV_AddBlend	(0.3, 0.3, 0.3, 0.7, ent->client->ps.blend);
	else if (ent->client->dmgef_flash) //pbowens: explosion effect
		SV_AddBlend (1, 1, 1, ent->client->dmgef_ablend,  ent->client->ps.blend);


	// add for powerups
	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 0, 1, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (1, 1, 0, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		remaining = ent->client->enviro_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 1, 0, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		remaining = ent->client->breather_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0.4, 1, 0.4, 0.04, ent->client->ps.blend);
	}

	// add for damage
	// pbowens: long delayed damage
	if (ent->client->damage_blendtime > level.time) 
	{
		if (ent->client->damage_alpha > 0)
			SV_AddBlend (ent->client->damage_blend[0],
						 ent->client->damage_blend[1],
						 ent->client->damage_blend[2], 
						 ent->client->damage_alpha, 
						 ent->client->ps.blend);

		if (ent->client->bonus_alpha > 0)
			SV_AddBlend (0.85, 0.7, 0.3, 
						 ent->client->bonus_alpha, 
						 ent->client->ps.blend);
	}
	else 
	{ // drop the damage value
		ent->client->damage_alpha -= 0.06;
		if (ent->client->damage_alpha < 0)
			ent->client->damage_alpha = 0;

		// drop the bonus value
		ent->client->bonus_alpha -= 0.1;
		if (ent->client->bonus_alpha < 0)
			ent->client->bonus_alpha = 0;
	}
}


/*
=================
P_FallingDamage
=================
*/
void P_FallingDamage (edict_t *ent)
{
	float	delta;
	int		damage;
	vec3_t	dir;

	if (ent->s.modelindex != 255)
		return;		// not in the player model

	if (ent->movetype == MOVETYPE_NOCLIP)
		return;

	if ((ent->client->oldvelocity[2] < 0) && (ent->velocity[2] > ent->client->oldvelocity[2]) && (!ent->groundentity))
	{
		delta = ent->client->oldvelocity[2];
	}
	else
	{
		if (!ent->groundentity)
			return;
		delta = ent->velocity[2] - ent->client->oldvelocity[2];
	}
	delta = delta*delta * 0.0001;

	// never take falling damage if airborne class
	if (ent->client->resp.mos == SPECIAL && level.time < (ent->client->spawntime + 10)) 
		return;

	// never take falling damage if completely underwater
	if (ent->waterlevel == 3)
		return;
	// rezmoth - used to be 0.25
	if (ent->waterlevel == 2)
		delta *= 1;
	// rezmoth - used to be 0.5
	if (ent->waterlevel == 1)
		delta *= 1;

	if (delta < 1)
		return;

	if (delta < 15)
	{
		ent->s.event = EV_FOOTSTEP;
		return;
	}

	ent->client->fall_value = delta*0.5;
	if (ent->client->fall_value > 40)
		ent->client->fall_value = 40;
	ent->client->fall_time = level.time + FALL_TIME;

	if (delta > 30)
	{
		if (ent->health > 0)
		{
			if (delta >= 55)
				ent->s.event = EV_FALLFAR;
			else
				ent->s.event = EV_FALL;
		}
		ent->pain_debounce_time = level.time;	// no normal pain sound
		damage = (delta-30)/2;
		if (damage < 1)
			damage = 1;
		VectorSet (dir, 0, 0, 1);

		// rezmoth - used to be 10
		damage *= 30; // increase it for realism
		if (!deathmatch->value || !((int)dmflags->value & DF_NO_FALLING) )
			T_Damage (ent, world, world, dir, ent->s.origin, vec3_origin, damage, 0, 0, MOD_FALLING);
	}
	else
	{
		ent->s.event = EV_FALLSHORT;
		return;
	}
}

/*
=============
P_PenaltyCheck
=============
*/
void P_PenaltyCheck (edict_t *ent)
{
	int dmg;
	vec3_t dir;

	if (ent->client->penalty == PENALTY_NONE)
		return;

	switch (ent->client->penalty)
	{
	default:
		gi.cprintf(ent, PRINT_HIGH, "unknown penalty %i!\n", ent->client->penalty);
		dmg = 0;
		break;
	case PENALTY_TEAM_KILL:
		gi.bprintf(PRINT_MEDIUM, "%s was penalized by execution for killing a fellow teammate.\n",
			ent->client->pers.netname);
		dmg = 100; // punishable by death;
		break;
	case PENALTY_SPAWN_CAMP:
		gi.bprintf(PRINT_MEDIUM, "%s was penalized by execution for spawn camping.\n",
			ent->client->pers.netname);
		dmg = 100; // punishable by death
		break;
	case PENALTY_TEAM_CHANGE:
		gi.bprintf(PRINT_MEDIUM, "%s was penalized by execution for changing alliance.\n",
			ent->client->pers.netname);
		dmg = 100; // punishable by death
		break;
	}

	VectorSet (dir, 0, 0, 1);
	ent->client->penalty = PENALTY_NONE;
	T_Damage (ent, world, world, dir, ent->s.origin, vec3_origin, dmg, 0, 0, MOD_PENALTY);

}

/*
=============
P_ShowID
=============
*/
void P_ShowID (edict_t *ent)
{
	trace_t tr;
	vec3_t start, forward, end;

	VectorCopy(ent->s.origin, start);
    start[2] += ent->viewheight;
    AngleVectors(ent->client->v_angle, forward, NULL, NULL);
    VectorMA(start, 8192, forward, end);

    tr = gi.trace(start, NULL, NULL, end, ent, MASK_ALL); 
	if (tr.ent->client)
	{
		if (ent->flyingnun && tr.ent->client->resp.team_on && tr.ent->client->resp.mos)
		{
			ent->client->ps.stats[STAT_IDENT] = 1;
			ent->client->ps.stats[STAT_IDENT_PLAYER] = CS_PLAYERSKINS + (tr.ent - g_edicts - 1);
			ent->client->ps.stats[STAT_IDENT_HEALTH] = 0;
			ent->client->ps.stats[STAT_IDENT_ICON] = 0;
			ent->client->last_id_time = level.time;  //faf:  to put delay on player id

		}
		else if (ent->client->resp.mos == MEDIC ||
			ent->client->resp.show_id &&
			(  ent->client->resp.team_on &&    ent->client->resp.mos &&
			tr.ent->client->resp.team_on && tr.ent->client->resp.mos &&
			ent->client->resp.team_on->index == tr.ent->client->resp.team_on->index))
		{
			ent->client->ps.stats[STAT_IDENT] = 1;
			ent->client->ps.stats[STAT_IDENT_PLAYER] = CS_PLAYERSKINS + (tr.ent - g_edicts - 1);	
			ent->client->last_id_time = level.time; //faf:  to put delay on player id

			if (ent->client->resp.mos == MEDIC)
			{
				gi.configstring(CS_GENERAL + (ent - g_edicts - 1), va("Health: %i", tr.ent->health));

				ent->client->ps.stats[STAT_IDENT_HEALTH] = CS_GENERAL + (ent - g_edicts - 1);

				if ((tr.ent->health < tr.ent->max_health ) && OnSameTeam(ent, tr.ent)) 
					ent->client->ps.stats[STAT_IDENT_ICON] = gi.imageindex ("i_medic");
				else 
					ent->client->ps.stats[STAT_IDENT_ICON] = 0;

			}
			else
			{
				ent->client->ps.stats[STAT_IDENT_HEALTH] = 0;
				ent->client->ps.stats[STAT_IDENT_ICON] = 0;
			}
		}
	}
	else if ((ent->client->last_id_time + 1)< level.time) //delay on player id: faf
	{
		ent->client->ps.stats[STAT_IDENT]			= 0;
		ent->client->ps.stats[STAT_IDENT_PLAYER]	= 0;
		ent->client->ps.stats[STAT_IDENT_HEALTH]	= 0;
		ent->client->ps.stats[STAT_IDENT_ICON]		= 0;
	}

}

void P_WorldEffects (void)
{
	qboolean	breather;
	qboolean	envirosuit;
	int			waterlevel, watertype, old_waterlevel, old_watertype;

//bcass start - flamer sound thing
	int			randnum;
//bcass end

	if (current_player->movetype == MOVETYPE_NOCLIP)
	{
		current_player->air_finished = level.time + 12;	// don't need air
		return;
	}

	waterlevel = current_player->waterlevel;
	watertype  = current_player->watertype;

	old_waterlevel = current_client->old_waterlevel;
	old_watertype = current_client->old_watertype;

	current_client->old_waterlevel = waterlevel;
	current_client->old_watertype = watertype;

	breather = current_client->breather_framenum > level.framenum;
	envirosuit = current_client->enviro_framenum > level.framenum;

	// Treat lava as if it's barbedwire
	if (current_player->watertype & CONTENTS_LAVA)
	{
		T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 3*waterlevel, 0, 0, MOD_LAVA);
		return;
	}

	//
	// if just entered a water volume, play a sound
	//
	if (!old_waterlevel && waterlevel)
	{
		PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
		if (current_player->watertype & CONTENTS_LAVA)
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/lava_in.wav"), 1, ATTN_NORM, 0);
		else if (current_player->watertype & CONTENTS_SLIME)
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
		else if (current_player->watertype & CONTENTS_WATER)
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
		current_player->flags |= FL_INWATER;

		// clear damage_debounce, so the pain sound will play immediately
		current_player->damage_debounce_time = level.time - 1;

	}

	//
	// if just completely exited a water volume, play a sound
	//
	if (old_waterlevel && !waterlevel)
	{
		PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);

		// pbowens: new barbed wire sound
		if (old_watertype & CONTENTS_LAVA)
			gi.sound (current_player, CHAN_BODY, gi.soundindex("players/cloth.wav"), 1, ATTN_NORM, 0);
		else
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_out.wav"), 1, ATTN_NORM, 0);


		current_player->flags &= ~FL_INWATER;
	}

	//
	// check for head just going under water
	//
	if (old_waterlevel != 3 && waterlevel == 3)
	{
		gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_un.wav"), 1, ATTN_NORM, 0);
	}

	//
	// check for head just coming out of water
	//
	if (old_waterlevel == 3 && waterlevel != 3)
	{
		if (current_player->air_finished < level.time)
		{	// gasp for air
			gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/gasp1.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
		}
		else  if (current_player->air_finished < level.time + 11)
		{	// just break surface
			gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/gasp2.wav"), 1, ATTN_NORM, 0);
		}
	}

	//
	// check for drowning
	//
	if (waterlevel == 3)
	{
		// breather or envirosuit give air
		if (breather || envirosuit)
		{
			current_player->air_finished = level.time + 10;

			if (((int)(current_client->breather_framenum - level.framenum) % 25) == 0)
			{
				if (!current_client->breather_sound)
					gi.sound (current_player, CHAN_AUTO, gi.soundindex("player/u_breath1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_AUTO, gi.soundindex("player/u_breath2.wav"), 1, ATTN_NORM, 0);
				current_client->breather_sound ^= 1;
				PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
				//FIXME: release a bubble?
			}
		}

		// if out of air, start drowning
		if (current_player->air_finished < level.time)
		{	// drown!
			if (current_player->client->next_drown_time < level.time 
				&& current_player->health > 0)
			{
				current_player->client->next_drown_time = level.time + 1;

				// take more damage the longer underwater
				current_player->dmg += 2;
				if (current_player->dmg > 15)
					current_player->dmg = 15;

				// play a gurp sound instead of a normal pain sound
				if (current_player->health <= current_player->dmg)
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("*drown1.wav"), 1, ATTN_NORM, 0);

					//gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/drown1.wav"), 1, ATTN_NORM, 0);
				else if (rand()&1)
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("*gurp1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("*gurp2.wav"), 1, ATTN_NORM, 0);

				current_player->pain_debounce_time = level.time;

				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, current_player->dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	}
	else
	{
		current_player->air_finished = level.time + 12;
		current_player->dmg = 2;
	}

	//
	// check for sizzle damage
	//
	if (waterlevel && (current_player->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) )
	{
		if (current_player->watertype & CONTENTS_LAVA)
		{
			if (current_player->health > 0
				&& current_player->pain_debounce_time <= level.time
				&& current_client->invincible_framenum < level.framenum)
			{

//bcass start - flamer sound thing - copy/updated medic sound thing
		srand(rand());
		randnum=rand()%100;
		
		//let the fun begin defining sounds
		if(randnum > FLAMER1 && randnum < FLAMER2) {
			gi.sound (current_player, CHAN_WEAPON, gi.soundindex ("player/burn1.wav"), 1.0, ATTN_NORM, 0);
		} else if (randnum > FLAMER2 && randnum < FLAMER3) {
			gi.sound (current_player, CHAN_WEAPON, gi.soundindex ("player/burn2.wav"), 1.0, ATTN_NORM, 0);
		} else if (randnum > FLAMER3 && randnum < FLAMER4) {
			gi.sound (current_player, CHAN_WEAPON, gi.soundindex ("player/burn3.wav"), 1.0, ATTN_NORM, 0);
		} else if (randnum > FLAMER4 && randnum < FLAMERH) {
			gi.sound (current_player, CHAN_WEAPON, gi.soundindex ("player/burn4.wav"), 1.0, ATTN_NORM, 0);
		} else {
			gi.sound (current_player, CHAN_WEAPON, gi.soundindex ("player/burn1.wav"), 1.0, ATTN_NORM, 0);
		}
//bcass end		

//				if (rand()&1)
//					gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/burn1.wav"), 1, ATTN_NORM, 0);
//				else
//					gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/burn2.wav"), 1, ATTN_NORM, 0);
				current_player->pain_debounce_time = level.time + 1;
			}

			if (envirosuit)	// take 1/3 damage with envirosuit
				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 1*waterlevel, 0, 0, MOD_LAVA);
			else
				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 3*waterlevel, 0, 0, MOD_LAVA);
		}

		if (current_player->watertype & CONTENTS_SLIME)
		{
			if (!envirosuit)
			{	// no damage from slime with envirosuit
				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 1*waterlevel, 0, 0, MOD_SLIME);
			}
		}
	}
}

/*
===============
G_SetClientEffects
===============
*/
void G_SetClientEffects (edict_t *ent)
{
	int		pa_type;
	int		remaining;

	ent->s.effects = 0;
	ent->s.renderfx = 0;

	if (ent->health <= 0 || level.intermissiontime)
		return;

	if (ent->powerarmor_time > level.time)
	{
		pa_type = PowerArmorType (ent);
		if (pa_type == POWER_ARMOR_SCREEN)
		{
			ent->s.effects |= EF_POWERSCREEN;
		}
		else if (pa_type == POWER_ARMOR_SHIELD)
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_GREEN;
		}
	}

	if (ent->client->medic_call > level.framenum) 
	{
		remaining = ent->client->medic_call - level.framenum;
		if ((remaining & 2)) {
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_RED;
		}

	}

	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining > 30 || (remaining & 2) )
			ent->s.effects |= EF_QUAD;
	}

	if (ent->client->invincible_framenum > level.framenum)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			ent->s.effects |= EF_PENT;
	}

	// show cheaters!!!
	if (ent->flags & FL_GODMODE)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
	}
}


/*
===============
G_SetClientEvent
===============
*/
void G_SetClientEvent (edict_t *ent)
{
	vec3_t end = { 0, 0, -200};
	trace_t tr;

	VectorMA (ent->s.origin, 50, end, end);
	tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_ALL);

	/*
	// pbowens: the rest of this is modified for surface types

	if (ent->s.event)
		return;

	if ( ent->groundentity && xyspeed > 225)
	{
		if ( (int)(current_client->bobtime+bobmove) != bobcycle )
			ent->s.event = EV_FOOTSTEP;
	}
	*/

	if (!ent->groundentity || ent->stanceflags != STANCE_STAND)
		return;
	
	if (!ent->client->movement)
		return;

	if ((ent->client->footstep_framenum) &&
		(level.framenum < ent->client->footstep_framenum))
		return;
	else //if (!ent->client->footstep_framenum) 
		ent->client->footstep_framenum = level.framenum;

	
	if (ent->client)
	{
		int sound;
		float volume = (float)((VectorLength(ent->velocity) /200) - (random() * .3));
		
	//		float volume = (float)(VectorLength(ent->velocity))/200;

		vec3_t end, down = { 0, 0, -1};
		trace_t tr;


		if (volume > .9)
			volume = .9;
		if (volume < 0)
			volume = 0;


		VectorMA (ent->s.origin, 50, down, end);
		tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, CONTENTS_SOLID);
		//gi.dprintf("surface name: %s\n", tr.surface->name);
		
		if (Surface(tr.surface->name, SURF_GRASS)) // grass, snow etc
		{
			if (random() < 0.5)
				sound = gi.soundindex("foot/grass1.wav");
			else
				sound = gi.soundindex("foot/grass2.wav");
			volume /= 3;				
		}
		else if (Surface(tr.surface->name, SURF_WOOD)) // wood
		{
			if (random() < 0.5)
				sound = gi.soundindex("foot/wood1.wav");
			else
				sound = gi.soundindex("foot/wood2.wav");
			volume /= 3;
		}
		else if (Surface(tr.surface->name, SURF_METAL)) // metal
		{
			if (random() < 0.5)
				sound = gi.soundindex("foot/metal1.wav");
			else
				sound = gi.soundindex("foot/metal2.wav");
			volume /= 3;
		}
		else if (Surface(tr.surface->name, SURF_SAND)) // sand (beach)
		{
			if (random() < 0.5)
				sound = gi.soundindex("foot/sand1.wav");
			else
				sound = gi.soundindex("foot/sand2.wav");
			volume /= 3;				
		}
		else //default
		{
			volume = volume * .8; //faf

			if (random() < 0.25)
				sound = gi.soundindex("player/step1.wav");
			else if (random() < 0.50)
				sound = gi.soundindex("player/step2.wav");
			else if (random() < 0.75)
				sound = gi.soundindex("player/step3.wav");
			else
				sound = gi.soundindex("player/step4.wav");
		}		
			
		gi.sound (ent, CHAN_AUTO, sound, volume, ATTN_NORM, 0);
	}

	if (ent->client->aim)
		ent->client->footstep_framenum = level.framenum + 5;
	else if (ent->wound_location == LEG_WOUND)
	{
		if (level.framenum % 8 < 3)
			ent->client->footstep_framenum = level.framenum + 2;
		else
			ent->client->footstep_framenum = level.framenum + 5;
	}			
	else
		ent->client->footstep_framenum = level.framenum +4;
}


/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound (edict_t *ent)
{
	char	*weap;

	if (ent->client->resp.game_helpchanged != game.helpchanged)
	{
		ent->client->resp.game_helpchanged = game.helpchanged;
		ent->client->resp.helpchanged = 0;
	}

	// help beep (no more than three times)
	if (ent->client->resp.helpchanged && ent->client->resp.helpchanged <= 3 && !(level.framenum&63) )
	{
		ent->client->resp.helpchanged = 0;
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("misc/pc_up.wav"), 1, ATTN_STATIC, 0);
	}


	if (ent->client->pers.weapon)
		weap = ent->client->pers.weapon->classname;
	else
		weap = "";

	if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME))) 
	{
		ent->s.sound = snd_fry;
	} 
	// Nick - to stop Flamer making noise when empty.
	else if (strcmp(weap, "weapon_flamethrower") == 0)
	{
		if (ent->client->flame_rnd && ent->waterlevel < 3) 
		{
			ent->s.sound = gi.soundindex("weapons/flamer/fireitup.wav");
		} 
		else
		{
			ent->s.sound = 0;
		} 
	}
	else if (ent->client->weapon_sound) 
	{
		ent->s.sound = ent->client->weapon_sound;
	} 
	else 
	{
		ent->s.sound = 0;
	}
	// End Nick
}

/*
===============
G_SetClientFrame
===============
*/
void G_SetClientFrame (edict_t *ent)
{
	gclient_t	*client;
	qboolean	duck, run, crawl, aim;

	if (ent->s.modelindex != 255)
		return;		// not in the player model

	client = ent->client;

	//faf: stops delay on crawl to standing animation:
	if (ent->stanceflags==STANCE_CRAWL)
		crawl = true;
	else
		crawl = false;

	if (ent->stanceflags==STANCE_DUCK)
		duck = true;
	else
		duck = false;
	if (xyspeed)
		run = true;
	else
		run = false;

	//faf: bug fix.  Fix delay on player model going into aiming
	if (client->aim && ent->stanceflags == STANCE_STAND)
		aim = true;
	else
		aim = false;
	//faf: end


	// check for stand/duck and stop/go transitions
	if (crawl != client->anim_crawl && client->anim_priority < ANIM_DEATH)
		goto newanim;

	if (duck != client->anim_duck && client->anim_priority < ANIM_DEATH)
		goto newanim;
	if (run != client->anim_run && client->anim_priority == ANIM_BASIC
		&& !(run == false && client->sidestep_anim != 0))//faf: let them finish sidestepping
		goto newanim;
	if (!ent->groundentity && client->anim_priority <= ANIM_WAVE)
		goto newanim;
	//faf: go to new sidestep animations immediately
	if (extra_anims->value !=0 && 
		ent->stanceflags == STANCE_STAND &&
		run && 
		client->sidestep_anim != client->last_sidestep_anim &&
		client->movement)
		goto newanim;
	if (aim != client->anim_aim && ent->stanceflags == STANCE_STAND  
		&& !ent->deadflag)
		goto newanim;



	//pbowens: v_wep
    if (client->anim_priority == ANIM_REVERSE)
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

		// pbowens: do crawl/crouch anims
		if (ent->stanceflags == STANCE_CRAWL) 
		{
			ent->s.frame = FRAME_crawlidle01;
			ent->client->anim_end = FRAME_crawlidle03;

		}
		else if (ent->stanceflags == STANCE_DUCK)
		{
			ent->s.frame = FRAME_crstnd01;
			ent->client->anim_end = FRAME_crstnd03;
		} 
		else // standing
		{
			ent->s.frame = FRAME_jump3;
			ent->client->anim_end = FRAME_jump6;
		}

		return;
	}

newanim:
	// return to either a running or standing frame
	client->anim_priority = ANIM_BASIC;
	client->anim_duck = duck;
	client->anim_run = run;
	client->anim_aim = aim;
	client->anim_crawl = crawl;

	if (!ent->groundentity)
	{
		client->anim_priority = ANIM_JUMP;

		// pbowens: do crawl/crouch anims
		if (ent->stanceflags == STANCE_CRAWL) 
		{
			if (ent->s.frame != FRAME_crawlidle02)
				ent->s.frame = FRAME_crawlidle01;
			client->anim_end = FRAME_crawlidle02;

		}
		else if (ent->stanceflags == STANCE_DUCK)
		{
			if (ent->s.frame != FRAME_crstnd02)
				ent->s.frame = FRAME_crstnd01;
			client->anim_end = FRAME_crstnd02;
		} 
		else // standing
		{
			if (ent->s.frame != FRAME_jump2)
				ent->s.frame = FRAME_jump1;
			client->anim_end = FRAME_jump2;
		}

	}
	else if (run)
	{	// running
		if (duck)
		{
			ent->s.frame = FRAME_crwalk1;
			client->anim_end = FRAME_crwalk6;
		}
		else if(ent->stanceflags==STANCE_CRAWL)
		{
			ent->s.frame = FRAME_crawlwalk01;
			client->anim_end = FRAME_crawlwalk08;
		}
		else
		{
			//faf:  for Parts' sidestep animations
			if (client->sidestep_anim == MOVE_RIGHT)
			{
				if (client->aim)
				{
				ent->s.frame = FRAME_stepleftaim01;
				client->anim_end = FRAME_stepleftaim06;
				}
				else
				{
				ent->s.frame = FRAME_strafeleft01;
				client->anim_end = FRAME_strafeleft06;
				}
			}
			else if (client->sidestep_anim == MOVE_LEFT)
			{
				if (client->aim)
				{
					ent->s.frame = FRAME_steprightaim01;
					client->anim_end = FRAME_steprightaim06;
				}
				else
				{
					ent->s.frame = FRAME_straferight01;
					client->anim_end = FRAME_straferight06;
				}
			}
//			else
//			{
//				ent->s.frame = FRAME_run1;
//				client->anim_end = FRAME_run6;
//			}
			else if (client->sidestep_anim == MOVE_BACKWARD)
			{
				if (client->aim)
				{
					ent->s.frame = FRAME_walkbackwardsaim01;
					client->anim_end = FRAME_walkbackwardsaim06;
				}
				else
				{
					ent->s.frame = FRAME_runbackwards01;
					client->anim_end = FRAME_runbackwards06;
				}
			}
			else //run forward
			{
				if (client->aim)
				{
					ent->s.frame = FRAME_walkaim01;
					client->anim_end = FRAME_walkaim06;
				}
				else
				{
					ent->s.frame = FRAME_run1;
					client->anim_end = FRAME_run6;
				}
			}



			if (extra_anims->value != 1)
			{
				ent->s.frame = FRAME_run1;
				client->anim_end = FRAME_run6;
			}
			client->last_sidestep_anim = client->sidestep_anim;



		}
	}
	else
	{	// standing
		if (duck)
		{
			ent->s.frame = FRAME_crstnd01;
			client->anim_end = FRAME_crstnd19;
		}
		else if(ent->stanceflags == STANCE_CRAWL)
		{
			ent->s.frame =FRAME_crawlidle01;
			client->anim_end = FRAME_crawlidle15;
		}
		else
		{
			if (ent->client->aim)
			{
				ent->s.frame = FRAME_attack1;//faf:  was FRAME_attack2;
				client->anim_end = FRAME_attack1;//faf: was FRAME_attack2;
			}
			else
			{
				ent->s.frame = FRAME_stand01;
				client->anim_end = FRAME_stand40;
			}
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

edict_t *FindOverlap(edict_t *ent, edict_t *last_overlap)
{
        int i;
        edict_t *other;
        vec3_t diff;

        for (i = last_overlap ? last_overlap - g_edicts : 0; i < game.maxclients; i++)
        {
                other = &g_edicts[i+1];

                if (!other->inuse || !other->client->resp.team_on
								  || !other->client->resp.mos
                                  || other == ent 
                                  || other->solid == SOLID_NOT
                                  || other->deadflag == DEAD_DEAD)
                        continue;

                VectorSubtract(ent->s.origin, other->s.origin, diff);

                if (diff[0] >= -33 && diff[0] <= 33 &&
                                diff[1] >= -33 && diff[1] <= 33 &&
                                diff[2] >= -65 && diff[2] <= 65)
                        return other;
        }

        return NULL;
}

void A_ScoreboardMessage (edict_t *ent);

void ClientEndServerFrame (edict_t *ent)
{
	float	bobtime;
	int		i;

	current_player = ent;
	current_client = ent->client;

	if (ent->client->damage_div>0)
	{
		ent->client->damage_div-=FRAMETIME;
		if (ent->client->damage_div<0)
			ent->client->damage_div=0;
	}
	
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
		current_client->ps.pmove.origin[i] = ent->s.origin[i]*8.0;
		current_client->ps.pmove.velocity[i] = ent->velocity[i]*8.0;
	}

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if (level.intermissiontime)
	{
		float delay = (level.intermissiontime + INTERMISSION_DELAY);

		// FIXME: add view drifting here?
		current_client->ps.blend[3] = 0;
		current_client->ps.fov = STANDARD_FOV;
		G_SetStats (ent);

		// pbowens: overflow avoidance
		if (level.time < delay)
		{
			PMenu_Close(ent);
			ent->client->showinventory = false;
			ent->client->showscores = false;
			ent->client->showhelp = false;
			ent->client->showpscores = false;
		}

		if (level.time == delay)
		{
			if (deathmatch->value || coop->value)
			{
				ent->client->showscores = true;
			}


			A_ScoreboardMessage(ent);//
			//faf DDayScoreboardMessage(ent);
			gi.unicast (ent, true);
		}

		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, up);

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
	xyspeed = sqrt(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);

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

	if (current_client->ps.pmove.pm_flags & PMF_DUCKED)
		bobtime *= 4;

	bobcycle = (int)bobtime;
	bobfracsin = fabs(sin(bobtime*M_PI));

	// detect hitting the floor
	P_FallingDamage (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// pbowens: do the explosion effects
	P_ExplosionEffects (ent);

	// pbowens: penalties
	P_PenaltyCheck (ent);

	// pbowens: player id's
	P_ShowID (ent);


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

	G_SetStats (ent);

	G_SetClientEvent (ent);

	G_SetClientEffects (ent);

	G_SetClientSound (ent);

	G_SetClientFrame (ent);

	VectorCopy (ent->velocity, ent->client->oldvelocity);
	VectorCopy (ent->client->ps.viewangles, ent->client->oldviewangles);
	// clear weapon kicks
	VectorClear (ent->client->kick_origin);
	VectorClear (ent->client->kick_angles);

	// if the scoreboard is up, update it
/*	if (ent->client->showscores && !(level.framenum & 31) )
	{
		DeathmatchScoreboardMessage (ent, ent->enemy);
		gi.unicast (ent, false);
	}*/
	// if the scoreboard is up, update it

	if (ent->client->showscores && !(level.framenum & 31) )
	{ 
		if (ent->client->menu)
			PMenu_Update(ent);
		else
			A_ScoreboardMessage(ent);
			//faf DDayScoreboardMessage(ent);

		gi.unicast (ent, false);
	}

	// This was absolutely 100% borrowed from AQ2. 
	if (ent->solid == SOLID_TRIGGER)
	{       
		edict_t *overlap;

		if (ent->client &&
			ent->client->resp.team_on &&
			ent->client->resp.mos == MEDIC &&
			invuln_medic->value == 1)
			return; // unsafe, but will do for now

		if ((overlap = FindOverlap(ent, NULL)) == NULL)
		{
			ent->solid = SOLID_BBOX;	
			gi.linkentity(ent);
		}
		else    
		{
			do
			{
				if (overlap->solid == SOLID_BBOX)
				{
					overlap->solid = SOLID_TRIGGER;
					gi.linkentity(overlap);
				}
				overlap = FindOverlap(ent, overlap);

			} while (overlap != NULL);
		}
	}
}


