
#include "g_local.h"
#include "m_player.h"

// Skid added
#include "m_genqguy.h"
#include "m_gendguy.h"
#include "m_genwguy.h"

static	edict_t		*current_player;
static	gclient_t	*current_client;

static	vec3_t	forward, right, up;
float	xyspeed;

float	bobmove;
int		bobcycle;		// odd cycles are right foot going forward
float	bobfracsin;		// sin(bobfrac*M_PI)

// Skid added
static int current_player_class;

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


//============================
// Skid added
// Q1 style Roll
// faster to seperate them ?
//============================

float SV_Q1CalcRoll (vec3_t angles, vec3_t velocity)
{
	float	sign;
	float	side;

	side = DotProduct (velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);

	if (side < 250)
		side = side * 2.5 / 250.0;
	else
		side = 2.5;

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

//Skid added
	int		player_class= player->client->resp.player_class;
	
	client = player->client;

	// flash the backgrounds behind the status numbers
	client->ps.stats[STAT_FLASHES] = 0;
	if (client->damage_blood)
		client->ps.stats[STAT_FLASHES] |= 1;
	if (client->damage_armor && !(player->flags & FL_GODMODE) && 
	   (client->invincible_framenum <= level.framenum))
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
		if (client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			player->s.frame = FRAME_crpain1-1;
			client->anim_end = FRAME_crpain4;
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
	if ((level.time > player->pain_debounce_time) && 
		!(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
	{
//Skid added
		player->pain_debounce_time = level.time + 0.7;
		switch(player_class)
		{
			case CLASS_WOLF:
			{
				if(player->health > 75)
					gi.sound (player, CHAN_VOICE, gi.soundindex("wolfguy/pain1.wav"), 1, ATTN_NORM, 0);
				else if(player->health > 50)
					gi.sound (player, CHAN_VOICE, gi.soundindex("wolfguy/pain2.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (player, CHAN_VOICE, gi.soundindex("wolfguy/pain3.wav"), 1, ATTN_NORM, 0);
				break;
			}
			case CLASS_DOOM:
			{
				gi.sound (player, CHAN_VOICE, gi.soundindex("doomguy/pain.wav"), 1, ATTN_NORM, 0);
				break;
			}
			case CLASS_Q1:
			case CLASS_Q2:
			default:
			{
//End Skid
				r = 1 + (rand()&1);
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
		}
	}

	// the total alpha of the blend is always proportional to count
	if (client->damage_alpha < 0)
		client->damage_alpha = 0;
	client->damage_alpha += count*0.01;
	if (client->damage_alpha < 0.2)
		client->damage_alpha = 0.2;
	if (client->damage_alpha > 0.6)
		client->damage_alpha = 0.6;		// don't go too saturated

	// the color of the blend will vary based on how much was absorbed
	// by different armors
	VectorClear (v);

//Skid added
	switch(player_class)
	{
		case CLASS_Q2:
			{
				if (client->damage_parmor)
				{
					VectorMA (v, (float)client->damage_parmor/realcount, power_color, v);
				}
			}
		case CLASS_Q1:
			{
				if (client->damage_armor)
				{
					VectorMA (v, (float)client->damage_armor/realcount,  acolor, v);
				}
			}
//kinda buggy in giveall mode
		case CLASS_WOLF:
		case CLASS_DOOM:
		default:
			{
				if (client->damage_blood) 
				{
					VectorMA (v, (float)client->damage_blood/realcount,  bcolor, v);
				}
				break;
			}
	}
//End Skid
	
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
		
//Skid added
//face routine for Doomguy
		switch(player_class)
		{
		case CLASS_WOLF:
			{
				WolfieFaceTheMusic(client,side,player->health);
				break;
			}
		case CLASS_DOOM:
			{
				DoomFaceTheMusic(client,side,player->health);
				break;
			}
		case CLASS_Q1:
		case CLASS_Q2:
		default:
			{
//end Skid
				client->v_dmg_roll = kick*side*0.3;
				side = -DotProduct (v, forward);
				client->v_dmg_pitch = kick*side*0.3;
				client->v_dmg_time = level.time + DAMAGE_TIME;
				break;
			}
		}
	}

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_parmor = 0;
	client->damage_knockback = 0;
}


/*======================================================================
//Skid added
//Common stuff in the 4 View Offset Functions
//end Skid
======================================================================*/

static void CommonViewOffsets(edict_t *ent, vec3_t v)
{

//CHASECAM added
	if (!ent->client->chasetoggle)
	{
//end
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

//CHASECAM added	
	}
	else
	{
		// absolutely bound offsets
		// so the view can never be outside the player box
    
		VectorSet (v, 0, 0, 0);
		if (ent->client->chasecam != NULL)
		{
			ent->client->ps.pmove.origin[0] = ent->client->chasecam->s.origin[0]*8;
			ent->client->ps.pmove.origin[1] = ent->client->chasecam->s.origin[1]*8;
			ent->client->ps.pmove.origin[2] = ent->client->chasecam->s.origin[2]*8;
			VectorCopy (ent->client->chasecam->s.angles, ent->client->ps.viewangles);
		}
	}
//end CHASECAM
	VectorCopy (v, ent->client->ps.viewoffset);
}

//======================================================================


/*
===============
SV_Q1CalcViewOffset
Skid added
===============
*/

void SV_Q1CalcViewOffset (edict_t *ent)
{
	float		*angles;
	float		ratio;
	float		delta;
	vec3_t		v;

//===================================

	// base angles
	angles = ent->client->ps.kick_angles;

	// if dead, fix the angle and don't add any kick
	if (ent->deadflag)
	{
		VectorClear (angles);
		ent->client->ps.viewangles[ROLL] = 40;
	}
	else
	{
		// add angles based on weapon kick
		VectorCopy (ent->client->kick_angles, angles);

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

		// fall
		ratio = (ent->client->fall_time - level.time) / FALL_TIME;
		if (ratio < 0)
			ratio = 0;
		angles[PITCH] += ratio * ent->client->fall_value * 0.25; 

		delta = DotProduct (ent->velocity, right);
		angles[ROLL] += delta* 0.008;
	}
//===================================

	// base origin
	VectorClear (v);
	// add view height
	v[2] += ent->viewheight;
	// add kick offset
	VectorAdd (v, ent->client->kick_origin, v);

	CommonViewOffsets(ent,v);
}

/*
===============
SV_DoomCalcViewOffset
Skid added
===============
*/

void SV_DoomCalcViewOffset (edict_t *ent)
{
	float		*angles;
	float		delta;
	vec3_t		v;
	float       bob;

//===================================

	// base angles
	angles = ent->client->ps.kick_angles;

	// if dead, fix the angle and don't add any kick
	if(!ent->deadflag)
	{
		// add angles based on weapon kick
		VectorCopy (ent->client->kick_angles, angles);

		//Chainsaw shakin'
		if(ent->client->oldweapon > level.time)
		{
			angles[YAW] += ent->client->v_dmg_roll;
//gi.dprintf("DMG ROLL :%f\n",ent->client->v_dmg_roll);
			angles[PITCH] += ent->client->v_dmg_pitch;
//gi.dprintf("DMG PITCH: %f\n",ent->client->v_dmg_pitch);
		}
		else
		{
			ent->client->v_dmg_roll = 0;
			ent->client->v_dmg_pitch = 0;
		}
			
		// add angles based on bob
		delta = bobfracsin * .0066 * xyspeed;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			delta *= 2;		// crouching
		
		if (bobcycle & 1)
		{
			angles[PITCH] -= delta;
		}
	}

//===================================

	// base origin
	VectorClear (v);

	// add view height
	v[2] += ent->viewheight;

	//subtract fall height
	if(ent->client->fall_time > level.time)
	{
		v[2] -=  (ent->client->fall_time - level.time) * 15.0; 
	}
	//Chainsaw shakes
	else if(ent->client->v_dmg_pitch)
	{
		v[2] += (ent->client->v_dmg_pitch);
//gi.dprintf("DMG PITCH: %f\n",ent->client->v_dmg_pitch);
	}

	// add kick offset
	VectorAdd (v, ent->client->kick_origin, v);

	bob = bobfracsin * xyspeed * 0.01;
	if (bob > 5)
		bob = 5;
		
	if(bobcycle & 1)
//SKID	
		v[2] -= bob;

	CommonViewOffsets(ent,v);
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
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			delta *= 6;		// crouching
		angles[PITCH] += delta;


		delta = bobfracsin * bob_roll->value * xyspeed;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			delta *= 6;		// crouching
		if (bobcycle & 1)
			delta = -delta;
		angles[ROLL] += delta;

	}

//===================================

	// base origin
	VectorClear (v);

	// add view height
	v[2] += ent->viewheight;

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

	CommonViewOffsets(ent,v);
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
			ent->client->ps.gunangles[ROLL] += 0.1 * delta;
		ent->client->ps.gunangles[i] += 0.2 * delta;
	}

	// gun height
	VectorClear (ent->client->ps.gunoffset);

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

	ent->client->ps.blend[0] = ent->client->ps.blend[1] = 
		ent->client->ps.blend[2] = ent->client->ps.blend[3] = 0;

	// add for contents
	VectorAdd (ent->s.origin, ent->client->ps.viewoffset, vieworg);
	contents = gi.pointcontents (vieworg);
	if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER) )
		ent->client->ps.rdflags |= RDF_UNDERWATER;
	else
		ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	if (contents & (CONTENTS_SOLID|CONTENTS_LAVA))
		SV_AddBlend (1.0, 0.3, 0.0, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_SLIME)
		SV_AddBlend (0.0, 0.1, 0.05, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_WATER)
		SV_AddBlend (0.5, 0.3, 0.2, 0.4, ent->client->ps.blend);

	// add for powerups
	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining == 30 && // beginning to fade
//Skid added			
			ent->client->resp.player_class !=CLASS_DOOM)	
				gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
		{
//Skid added
//Doom Adernaline counter uses quad_framenum			
			if(ent->client->resp.player_class == CLASS_DOOM)
				SV_AddBlend (0.9, 0, 0, remaining/450.0, ent->client->ps.blend);	// red
			else
				SV_AddBlend (0, 0, 1, 0.08, ent->client->ps.blend);					// green
//red/blue blends end
//End Skid
		}
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
// FIXME - QUAKE DOOM
		remaining = ent->client->invincible_framenum - level.framenum;

		switch(ent->client->resp.player_class)
		{
		case CLASS_WOLF:
			{
				if (remaining == 30)	// beginning to fade
					gi.sound(ent, CHAN_ITEM, gi.soundindex("wolfguy/allright.wav"), 1, ATTN_NORM, 0);
				if (remaining > 30 || (remaining & 4) )
					SV_AddBlend (1, 1, 0, 0.08, ent->client->ps.blend);
				break;
			}
		case CLASS_DOOM:
			{
				if (remaining == 30)	// beginning to fade
					gi.sound(ent, CHAN_ITEM, gi.soundindex("ditems/getpow.wav"), 1, ATTN_NORM, 0);
				if (remaining > 30 || (remaining & 4) )
					SV_AddBlend (0.8, 0.8, 0.8, 0.3, ent->client->ps.blend);
				break;
			}
		default:
		case CLASS_Q1:
		case CLASS_Q2:
			{
				if (remaining == 30)	// beginning to fade
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect2.wav"), 1, ATTN_NORM, 0);
				if (remaining > 30 || (remaining & 4) )
					SV_AddBlend (1, 1, 0, 0.08, ent->client->ps.blend);
				break;
			}
		}
	}

//===================================
// Skid added - Invisible
//===================================
	else if (ent->client->invis_framenum > level.framenum)
	{
		remaining = ent->client->invis_framenum - level.framenum;
		if(ent->client->resp.player_class == CLASS_Q1)
		{
			if (remaining == 30)	// beginning to fade
				gi.sound(ent, CHAN_ITEM, gi.soundindex("q1items/inv2.wav"), 1, ATTN_NORM, 0);
			if (remaining > 30 || (remaining & 4) )
				SV_AddBlend (.8, .8, .8, 0.20, ent->client->ps.blend);
		}
		else if(ent->client->resp.player_class == CLASS_DOOM)
		{
			if (remaining == 30)	// beginning to fade
				gi.sound(ent, CHAN_ITEM, gi.soundindex ("ditems/getpow.wav"), 1, ATTN_NORM, 0);
		}
		else if(ent->client->resp.player_class == CLASS_WOLF)
		{
			if (remaining == 30)	// beginning to fade
				gi.sound(ent, CHAN_ITEM, gi.soundindex("q1items/inv2.wav"), 1, ATTN_NORM, 0);
			if (remaining > 30 || (remaining & 4) )
				SV_AddBlend (.8, .8, .8, 0.25, ent->client->ps.blend);
		}
	}
//===================================
//===================================

	else if (ent->client->enviro_framenum > level.framenum)
	{
		remaining = ent->client->enviro_framenum - level.framenum;
		if (remaining == 30)
		{
			if(ent->client->resp.player_class == CLASS_Q1)
				gi.sound(ent, CHAN_ITEM, gi.soundindex("q1items/suit2.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		}
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 1, 0, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		remaining = ent->client->breather_framenum - level.framenum;
		if (remaining == 30
//Skid added
			&& ent->client->resp.player_class ==CLASS_Q2)	// beginning to fade
//end Skid		
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0.4, 1, 0.4, 0.04, ent->client->ps.blend);
	}

	// add for damage
	if (ent->client->damage_alpha > 0)
		SV_AddBlend (ent->client->damage_blend[0],ent->client->damage_blend[1]
		,ent->client->damage_blend[2], ent->client->damage_alpha, ent->client->ps.blend);

	if (ent->client->bonus_alpha > 0)
		SV_AddBlend (0.85, 0.7, 0.3, ent->client->bonus_alpha, ent->client->ps.blend);

	// drop the damage value

// blends stay longer for Doomguy - Skid
	if(ent->client->resp.player_class == CLASS_DOOM)
		ent->client->damage_alpha -= 0.03;
	else
		ent->client->damage_alpha -= 0.06;
	
	if (ent->client->damage_alpha < 0)
		ent->client->damage_alpha = 0;

	// drop the bonus value

// blends stay longer for Doomguy - Skid
	if(ent->client->resp.player_class == CLASS_DOOM)
		ent->client->bonus_alpha -= 0.06;
	else
		ent->client->bonus_alpha -= 0.1;
	
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
	float	delta;
	int		damage;
	vec3_t	dir;
	
//Skid added	
	int		player_class=ent->client->resp.player_class;

	if (ent->s.modelindex != 255
// another check to allow Eyes - Skid		
		&& (!ent->flags & FL_INVIS))
		return;		// not in the player model
	
	if (ent->movetype == MOVETYPE_NOCLIP)
		return;
	
	if(ent->deadflag)// || !ent->health)
		return;

	if ((ent->client->oldvelocity[2] < 0) && 
		(ent->velocity[2] > ent->client->oldvelocity[2]) && 
		(!ent->groundentity))
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

//ZOID
	// never take damage if just release grapple or on grapple
	if (((level.time - ent->client->ctf_grapplereleasetime) <= (FRAMETIME*2)) ||
		(ent->client->ctf_grapple && ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY))
		return;
//ZOID

	// never take falling damage if completely underwater
	if (ent->waterlevel == 3)
		return;
	if (ent->waterlevel == 2)
		delta *= 0.25;
	if (ent->waterlevel == 1)
		delta *= 0.5;

	if (delta < 1)
		return;

//Skid Added		
	switch(player_class)
	{
	case CLASS_Q1:
		{
			if(delta < 6)
				return;
			break;
		}
	case CLASS_DOOM:
	case CLASS_WOLF:
		{
			if(delta < 12)
				return;
			break;
		}
	case CLASS_Q2:
	default:
		{
//End Skid		
			if (delta < 15)
			{
				ent->s.event = EV_FOOTSTEP;
				return;
			}
			break;
		}
	}

	ent->client->fall_value = delta*0.5;
	
	if (ent->client->fall_value > 40)
		ent->client->fall_value = 40;
	
//Skid added	
	if((player_class == CLASS_DOOM) ||
	   (player_class == CLASS_WOLF))
		ent->client->fall_time = level.time + 0.8;
	else
//End Skid
		ent->client->fall_time = level.time + FALL_TIME;

	if (delta > 30)
	{
//Skid added		
		switch(player_class)
		{
		case CLASS_WOLF:
			{
				if(ent->health > 0)
					gi.sound (current_player, CHAN_BODY, gi.soundindex("wolfguy/oof.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time;	// no normal pain sound
				damage = 0;
				//return;
				break;
			}
		case CLASS_DOOM:
			{
				if(ent->health > 0)
				{
					ent->pain_debounce_time = level.time;	// no normal pain sound
					if(ent->client->dlastfallsound < level.time)
					{
						gi.sound (current_player, CHAN_BODY, gi.soundindex("doomguy/noway.wav"), 1, ATTN_NORM, 0);
						current_player->client->dlastfallsound = level.time + 1.4;
					}
					damage = 0;
					//return;
				}
				break;
			}
		case CLASS_Q1:
			{
				if(ent->health >0)
				{
					if(delta >= 60)
						gi.sound (current_player, CHAN_BODY, gi.soundindex("q1guy/fall1.wav"), 1, ATTN_NORM, 0);
					else
						gi.sound (current_player, CHAN_BODY, gi.soundindex("q1guy/fall2.wav"), 1, ATTN_NORM, 0);
				}
				ent->pain_debounce_time = level.time;	// no normal pain sound
				damage = (delta-30)/3;
				if(damage >5)
					damage=5;
				break;
			}
		case CLASS_Q2:
		default:
			{
//end Skid
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
				break;
			}
		}
		
		VectorSet (dir, 0, 0, 1);
		if (!deathmatch->value || !((int)dmflags->value & DF_NO_FALLING))
			T_Damage (ent, world, world, dir, ent->s.origin, vec3_origin, damage, 0, 0, MOD_FALLING);
	}
	else
	{
		switch(player_class)
		{
		case CLASS_Q1:
			gi.sound (current_player, CHAN_BODY, gi.soundindex("q1guy/fall2.wav"), 1, ATTN_NORM, 0);
			break;
		case CLASS_WOLF:
			gi.sound (current_player, CHAN_BODY, gi.soundindex("wolfguy/oof.wav"), 1, ATTN_NORM, 0);
			break;
		case CLASS_DOOM:
			{
				if(ent->health)
				if(ent->client->dlastfallsound < level.time)
				{
					gi.sound (current_player, CHAN_BODY, gi.soundindex("doomguy/noway.wav"), 1, ATTN_NORM, 0);
					ent->client->dlastfallsound = level.time + 1.4;
				}
				break;
			}
		default:
		case CLASS_Q2:
			{
				ent->s.event = EV_FALLSHORT;
				break;
			}
		}
	}
}


/*
==========================
==========================
P_WorldEffects
==========================
==========================
*/

void P_WorldEffects (void)
{
	qboolean	breather;
	qboolean	envirosuit;
	int			waterlevel, old_waterlevel;
	
// Skid added
	qboolean    quakeguy;

	if (current_player->movetype == MOVETYPE_NOCLIP)
	{
		current_player->air_finished = level.time + 12;	// don't need air
		return;
	}

	waterlevel = current_player->waterlevel;
	old_waterlevel = current_client->old_waterlevel;
	current_client->old_waterlevel = waterlevel;

	breather = current_client->breather_framenum > level.framenum;
	envirosuit = current_client->enviro_framenum > level.framenum;

	if(current_player_class == CLASS_Q1)
		quakeguy = true;

//
// if just entered a water volume, play a sound
//
	if (!old_waterlevel && waterlevel)
	{
		PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
//Skid added
		if(current_player_class == CLASS_DOOM)
			gi.sound(current_player, CHAN_BODY, gi.soundindex("doomguy/splash.wav"),1, ATTN_NORM, 0);
		else if(current_player_class == CLASS_WOLF)
			gi.sound(current_player, CHAN_BODY, gi.soundindex("wolfguy/waterin.wav"),1, ATTN_NORM, 0);
		else if(current_player_class == CLASS_Q1)
		{
			if (current_player->watertype & CONTENTS_LAVA)
				gi.sound (current_player, CHAN_BODY, gi.soundindex("q1guy/inlava.wav"), 1, ATTN_NORM, 0);
			else if (current_player->watertype & CONTENTS_SLIME)
				gi.sound (current_player, CHAN_BODY, gi.soundindex("q1guy/inslime.wav"), 1, ATTN_NORM, 0);
			else if (current_player->watertype & CONTENTS_WATER)
				gi.sound (current_player, CHAN_BODY, gi.soundindex("q1guy/watr_in.wav"), 1, ATTN_NORM, 0);
		}
		else 
		{
			if (current_player->watertype & CONTENTS_LAVA)
				gi.sound (current_player, CHAN_BODY, gi.soundindex("player/lava_in.wav"), 1, ATTN_NORM, 0);
			else if (current_player->watertype & CONTENTS_SLIME)
				gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
			else if (current_player->watertype & CONTENTS_WATER)
				gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
		}
		current_player->flags |= FL_INWATER;

		// clear damage_debounce, so the pain sound will play immediately
		current_player->damage_debounce_time = level.time - 1;
	}

//
// if just completely exited a water volume, play a sound
//
	if (old_waterlevel && ! waterlevel)
	{
		PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
//Skid Added
		switch(current_player_class)
		{
		case CLASS_WOLF:
		case CLASS_DOOM:
			gi.sound (current_player, CHAN_BODY, gi.soundindex("doomguy/wout.wav"), 1, ATTN_NORM, 0);
			break;
		case CLASS_Q1:
			gi.sound (current_player, CHAN_BODY, gi.soundindex("q1guy/watr_out.wav"), 1, ATTN_NORM, 0);
			break;
		case CLASS_Q2:
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_out.wav"), 1, ATTN_NORM, 0);
			break;
		}
//end Skid
		current_player->flags &= ~FL_INWATER;
	}

//
// check for head just going under water
//
	if (old_waterlevel != 3 && waterlevel == 3)
	{
//Skid Added
		if(current_player_class == CLASS_Q2)
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_un.wav"), 1, ATTN_NORM, 0);
		else if(current_player_class == CLASS_DOOM || current_player_class == CLASS_WOLF)
			gi.sound(current_player, CHAN_BODY, gi.soundindex("doomguy/splash.wav"),1, ATTN_NORM, 0);
	}

//
// check for head just coming out of water
//
	if (old_waterlevel == 3 && waterlevel != 3)
	{
//Skid added		
		if(quakeguy)
		{
			if(current_player->air_finished < level.time)
			{
				gi.sound (current_player, CHAN_BODY, gi.soundindex("q1guy/gasp2.wav"), 1, ATTN_NORM, 0);
				PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
			}
			else if(current_player->air_finished < level.time + 9)
			{
				gi.sound (current_player, CHAN_BODY, gi.soundindex("q1guy/gasp1.wav"), 1, ATTN_NORM, 0);
			}
		}
//end Skid
		else
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
//Skid added 				
				if(current_player_class == CLASS_DOOM || current_player_class == CLASS_Q1)
				{
					if(current_player->dmg > 15)
						current_player->dmg = 10;
				}
				else
				{
//End Skid
					if (current_player->dmg > 15)
						current_player->dmg = 15;
				}

				// play a gurp sound instead of a normal pain sound
				if (current_player->health <= current_player->dmg)
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/drown1.wav"), 1, ATTN_NORM, 0);
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

	if (waterlevel && (current_player->watertype&(CONTENTS_LAVA|CONTENTS_SLIME))
//Doomguys do fine in lava with envirosuits					
		&& !(current_player_class == CLASS_DOOM && envirosuit)
		&& current_player->health > 0)

	{
		if (current_player->watertype & CONTENTS_LAVA)
		{
			if (current_player->health > 0
				&& current_player->pain_debounce_time <= level.time
				&& current_client->invincible_framenum < level.framenum)
			{
// Skid added 				
				if(quakeguy)
				{
					if (rand()&1)
						gi.sound (current_player, CHAN_VOICE, gi.soundindex("q1guy/burn1.wav"), 1, ATTN_NORM, 0);
					else
						gi.sound (current_player, CHAN_VOICE, gi.soundindex("q1guy/burn2.wav"), 1, ATTN_NORM, 0);
				}
				else
				{
// end Skid			
					if (rand()&1)
						gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/burn1.wav"), 1, ATTN_NORM, 0);
					else
						gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/burn2.wav"), 1, ATTN_NORM, 0);
				}
				current_player->pain_debounce_time = level.time + 1;
			}
			
//Skid added - Damage bursts for older guys
			if(current_client->resp.player_class != CLASS_Q2)
			{
				if (current_player->client->next_drown_time < level.time 
					&& current_player->health > 0)
				{
					if(envirosuit)
						current_player->client->next_drown_time = level.time + 1;
					else
					{
						if(current_client->resp.player_class == CLASS_DOOM)
							current_player->client->next_drown_time = level.time + 0.4;
						else
							current_player->client->next_drown_time = level.time + 0.2;
					}
					T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 10*waterlevel, 0, 0, MOD_LAVA);
				}
			}
			else
			{
// end Skid
				if (envirosuit)	// take 1/3 damage with envirosuit
					T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 1*waterlevel, 0, 0, MOD_LAVA);
				else
					T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 3*waterlevel, 0, 0, MOD_LAVA);
			}
		}

		if (current_player->watertype & CONTENTS_SLIME)
		{
			if (!envirosuit)
			{	// no damage from slime with envirosuit
				if(current_client->resp.player_class == CLASS_Q2)
					T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 1*waterlevel, 0, 0, MOD_SLIME);
// Skid added
				else
				{
					if ((current_player->client->next_drown_time < level.time)
					&& current_player->health > 0)
					{
						current_player->client->next_drown_time = level.time + 1;
						T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 10*waterlevel, 0, 0, MOD_SLIME);
					}
				}
// end Skid
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

//Skid
	int		pclass = ent->client->resp.player_class;

	if(!(pclass == CLASS_DOOM && ent->flags & FL_INVIS))
		ent->s.effects = 0;
	
	if (ent->health <= 0 || level.intermissiontime)
		return;

	ent->s.renderfx = 0;

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

//ZOID
	if(gen_ctf->value)
		CTFEffects(ent);
//ZOID	
	
	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
		{
//Skid - blink for Q2, blue glow for Q1
			if(pclass == CLASS_Q1 || pclass == CLASS_WOLF)
				ent->s.effects |= EF_BLUEHYPERBLASTER; 
			else if(pclass == CLASS_DOOM)
				ent->s.effects |= EF_FLAG1;
			else
			{
				if(gen_ctf->value)
				{
					if(level.framenum & 8)
						ent->s.effects |= EF_QUAD;
				}
				else
					ent->s.effects |= EF_QUAD;
			}
//end
		}
	}
	if (ent->client->invincible_framenum > level.framenum)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
		{
//Skid - different effect for old guys 
			if(pclass == CLASS_Q1 || pclass == CLASS_WOLF)
				ent->s.effects |= EF_FLAG1; 
			else if(pclass == CLASS_DOOM)
				ent->s.effects |= EF_HYPERBLASTER;
			else
			{
				if(gen_ctf->value)
				{
					if(level.framenum & 8)
						ent->s.effects |= EF_PENT;
				}
				else
					ent->s.effects |= EF_PENT;
			}
//end
		}
	}

//Skid 
//=======================
// Invis
//=======================
	if (ent->client->invis_framenum > level.framenum)
	{
		remaining = ent->client->invis_framenum - level.framenum;
		
		if (remaining <= 1)
		{
			if(!ent->client->chasetoggle)
				ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

			ent->s.skinnum = ent - g_edicts - 1; 
			ent->s.modelindex =255;
			if (ent->s.modelindex == 255) 
			{
				int i=0;

				if (ent->client->pers.weapon)
					i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
				else
					i = 0;
				ent->s.skinnum = (ent - g_edicts - 1) | i;
			}		
			ent->s.modelindex2 = 255;
			ent->flags &= ~FL_INVIS;

			if(gen_sp->value)
				ent->flags &= ~FL_NOTARGET;
		}
		else
		{
			ent->client->ps.gunindex = 0;
			if(pclass == CLASS_Q1)
			{
				ent->s.modelindex = gi.modelindex("models/items/q1pwrups/ring/q1eyes/tris.md2");
				ent->s.skinnum = 0;
			}
			else if(pclass == CLASS_DOOM)
			{
				ent->s.effects = EF_SPHERETRANS;
				ent->s.modelindex = gi.modelindex("models/objects/dinvis/tris.md2");
				ent->s.skinnum = 0;
			}
			else if(pclass == CLASS_WOLF)
			{
				ent->s.effects = EF_SPHERETRANS;
			}
			
			ent->s.modelindex2 = 0;
			ent->flags |= FL_INVIS;

			if(gen_sp->value)
				ent->flags |= FL_NOTARGET;
		}
	}
//=======================
//End Skid

	// show cheaters!!!
/*	if (ent->flags & FL_GODMODE)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
	}*/
}


/*
===============
G_SetClientEvent
===============
*/
void G_SetClientEvent (edict_t *ent)
{
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
	char	*weap;

	if (ent->client->pers.game_helpchanged != game.helpchanged)
	{
		ent->client->pers.game_helpchanged = game.helpchanged;
		ent->client->pers.helpchanged = 1;
	}

	// help beep (no more than three times)
	if (ent->client->pers.helpchanged && ent->client->pers.helpchanged <= 3 && !(level.framenum&63) )
	{
		ent->client->pers.helpchanged++;
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("misc/pc_up.wav"), 1, ATTN_STATIC, 0);
	}

	if (ent->client->pers.weapon)
		weap = ent->client->pers.weapon->classname;
	else
		weap = "";

	if(ent->client->resp.player_class == CLASS_Q2)
	{
		if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) )
			ent->s.sound = snd_fry;
		else if (strcmp(weap, "weapon_railgun") == 0)
			ent->s.sound = gi.soundindex("weapons/rg_hum.wav");
		else if (strcmp(weap, "weapon_bfg") == 0)
			ent->s.sound = gi.soundindex("weapons/bfg_hum.wav");
		else if (ent->client->weapon_sound)
			ent->s.sound = ent->client->weapon_sound;
		else
			ent->s.sound = 0;
	}
	else if(ent->client->resp.player_class == CLASS_Q1)
	{
		if(ent->flags & FL_INVIS)
			ent->s.sound = gi.soundindex("q1items/inv3.wav");
		else
			ent->s.sound = 0;
	}
	else if(ent->client->resp.player_class == CLASS_DOOM)
	{
		if ((strcmp(weap, "weapon_d_saw") == 0))
			ent->s.sound = gi.soundindex("dweap/sawidle.wav");
		else 
			ent->s.sound = 0;
	}
}


/*
===============
G_SetClientFrame
===============
*/

void G_SetClientFrame (edict_t *ent)
{
	gclient_t	*client;
	qboolean	duck, run;

//Skid added
	qboolean    axe=false;
	
	// not in the player model
	if (ent->s.modelindex != 255 
//Skid added - another check to allow Eyes		
		&& !(ent->flags & FL_INVIS)
		&& !(ent->flags & FL_D_GIBBED)
		)
		return;		

	client = ent->client;

	if (client->ps.pmove.pm_flags & PMF_DUCKED)
		duck = true;
	else
		duck = false;
	if (xyspeed)
		run = true;
	else
		run = false;

//Skid added
	if(!ent->deadflag && 
	   !(ent->flags & FL_INVIS) &&
	   (ITEM_INDEX(client->pers.weapon) == 18)) 
		axe=true;
//End Skid

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
	// continue an animation
	else if (ent->s.frame < client->anim_end) 
	{	
		ent->s.frame++;
		return;
	}

	// stay there
	if (client->anim_priority == ANIM_DEATH)
	{
		return;		
	}
	else if(ent->deadflag)
	{
						static int i;

                        i = (i+1)%3;
                        // start a death animation
                        client->anim_priority = ANIM_DEATH;
                        if (client->ps.pmove.pm_flags & PMF_DUCKED)
                        {
                                ent->s.frame = FRAME_crdeath1-1;
                                client->anim_end = FRAME_crdeath5;
                        }
                        else switch (i) 
                        {
                        case 0:
                                ent->s.frame = FRAME_death101-1;
                                client->anim_end = FRAME_death106;
                                break;
                        case 1:
                                ent->s.frame = FRAME_death201-1;
                                client->anim_end = FRAME_death206;
                                break;
                        case 2:
						default:
                                ent->s.frame = FRAME_death301-1;
                                client->anim_end = FRAME_death308;
                                break;
                        }
//                        }

		/*ent->s.frame = FRAME_death101-1;
        client->anim_end = FRAME_death106;
		client->anim_priority = ANIM_DEATH;*/
		return;
	}
	
	if (client->anim_priority == ANIM_JUMP)
	{
		ent->client->anim_priority = ANIM_WAVE;
		if(client->resp.player_class == CLASS_Q2)
		{
			//stay there
			if (!ent->groundentity ) 
				return;	

			ent->s.frame = FRAME_jump3;
			ent->client->anim_end = FRAME_jump6;
			//return;
		}
		return;
	}

newanim:
	// return to either a running or standing frame
	client->anim_priority = ANIM_BASIC;
	client->anim_duck = duck;
	client->anim_run = run;

	if (!ent->groundentity) 
	{
//ZOID: 
		//if on grapple, don't go into jump frame, go into standing frame
		if (client->ctf_grapple) 
		{
//Skid added			
			if(axe)
			{
				ent->s.frame = FRAME_axe_stand01;
				client->anim_end = FRAME_axe_stand12;
			}
			else
			{
//End Skid
				ent->s.frame = FRAME_stand01;
				client->anim_end = FRAME_stand40;
			}
		} 
		else 
		{
//ZOID
//SKIDJUMP
			client->anim_priority = ANIM_JUMP;
			if(client->resp.player_class == CLASS_Q2)
			{
				//client->anim_priority = ANIM_JUMP;
				if (ent->s.frame != FRAME_jump2)
					ent->s.frame = FRAME_jump1;
				client->anim_end = FRAME_jump2;
			}
//Skid Added			
			else //if(client->resp.player_class == CLASS_Q1)
			{
				if(axe)
				{
					if(run)
					{
						ent->s.frame = FRAME_axe_run1;
						client->anim_end = FRAME_axe_run6;
					}
					else
					{
						ent->s.frame = FRAME_axe_stand01;
						client->anim_end = FRAME_axe_stand06; 
					}
				}
				else
				{
					if(run)
					{
						ent->s.frame = FRAME_run1;
						client->anim_end = FRAME_run6;
					}
					else
					{
						ent->s.frame = FRAME_stand01;
						client->anim_end = FRAME_stand06; 
					}
				}
			}
//End Skid
		}
	}
	else if (run)
	{	
		// running
		if (duck)
		{
//Skid added		
			if(axe)
			{
				ent->s.frame = FRAME_axe_crwalk1;
				client->anim_end = FRAME_axe_crwalk5;
			}
			else
			{
//End Skid			
				ent->s.frame = FRAME_crwalk1;
				client->anim_end = FRAME_crwalk6;
			}
		}
		else
		{
//Skid Added		
			if(axe)
			{
				ent->s.frame = FRAME_axe_run1;
				client->anim_end = FRAME_axe_run6;
			}
			else
			{
//End Skid					
				ent->s.frame = FRAME_run1;
				client->anim_end = FRAME_run6;
			}
		}
	}
	else
	{	// standing
		if (duck)
		{
//Skid Added
			if(axe)
			{
				ent->s.frame = FRAME_axe_crstnd01;
				client->anim_end = FRAME_axe_crstnd06;
			}
			else
			{
//End Skid					
				ent->s.frame = FRAME_crstnd01;
				client->anim_end = FRAME_crstnd19;
			}
		}
		else
		{
// Skid		
			if(axe)
			{
				ent->s.frame = FRAME_axe_stand01;
				client->anim_end = FRAME_axe_stand12;
			}
			else
			{
// end Skid					
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

void ClientEndServerFrame (edict_t *ent)
{
	float	bobtime;
	int		i;
	
	current_player = ent;
	current_client = ent->client;
	
// Skid added
// faster maybe ?
	current_player_class = ent->client->resp.player_class;

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

	
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	
	if (level.intermissiontime)
	{
		// FIXME: add view drifting here?
		current_client->ps.blend[3] = 0;
		current_client->ps.fov = 90;
//Skid added 
//stat bars, faster ?		
		switch(current_player_class)
		{
			case CLASS_Q1:
				G_Q1SetStats(ent);
				break;
			case CLASS_DOOM:
				G_DoomSetStats(ent);
				break;
			case CLASS_WOLF:
				G_WolfSetStats(ent);
				break;
			case CLASS_Q2:
			default:
				G_SetStats (ent);
				break;
		}
		if(gen_ctf->value)
			SetCTFStats(ent);
//End Skid
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
	

//Skid added
//VIEW ROLL
	switch(current_player_class)
	{
	case CLASS_Q1:
		ent->s.angles[ROLL] = SV_Q1CalcRoll (ent->s.angles, ent->velocity)*4;
		break;
	case CLASS_Q2:
		ent->s.angles[ROLL] = SV_CalcRoll (ent->s.angles, ent->velocity)*4;
		break;
	default:
		ent->s.angles[ROLL] = 0;
		break;
	}
//End Skid

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
	{	
		// so bobbing only cycles when on ground
		if(current_player_class == CLASS_DOOM)
		{
			if (xyspeed > 210)
				bobmove = 0.26;
			else if (xyspeed > 100)
				bobmove = 0.135;
			else
				bobmove = 0.07;
		}
		else
		{	
			if (xyspeed > 210)
				bobmove = 0.25;
			else if (xyspeed > 100)
				bobmove = 0.125;
			else
				bobmove = 0.0625;
		}
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


//Skid added
//Different View offsets 	
	switch(current_player_class)
	{
		case CLASS_Q1:
		case CLASS_WOLF:
			SV_Q1CalcViewOffset(ent);
			break;
		case CLASS_DOOM:
			SV_DoomCalcViewOffset(ent);
			break;
		case CLASS_Q2:
			SV_CalcViewOffset (ent);
			SV_CalcGunOffset (ent);
			break;
		default:
			SV_CalcViewOffset (ent);
			break;
	}
//End Skid

	// determine the full screen color blend
	// must be after viewoffset, so eye contents can be
	// accurately determined
	// FIXME: with client prediction, the contents
	// should be determined by the client
	
	SV_CalcBlend (ent);

	// chase cam stuff
	if (ent->client->resp.spectator)
		G_SetSpectatorStats(ent);
	else if(!level.intermissiontime)
	{
		switch(current_player_class)
		{
			case CLASS_Q1:
				G_Q1SetStats(ent);
				break;
			
			case CLASS_DOOM:
				G_DoomSetStats(ent);
				break;
			case CLASS_WOLF:
				G_WolfSetStats(ent);
				break;
			case CLASS_Q2:
			default:
				G_SetStats (ent);
				break;
		}
		if(gen_ctf->value)
			SetCTFStats(ent);
	}
	G_CheckChaseStats(ent);

//Skid added
//no footsteps for other guys
	if(current_player_class == CLASS_Q2)
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
	if (ent->client->showscores && !(level.framenum & 31) )
	{
		DeathmatchScoreboardMessage (ent, ent->enemy);
		gi.unicast (ent, false);
	}

// CHASECAM
	if (ent->client->chasetoggle == 1)
         CheckChasecam_Viewent(ent);
}

