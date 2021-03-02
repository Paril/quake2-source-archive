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

void UpdateRocketCam (edict_t *ent)
{
	vec3_t temp;
	//if (!ent->client->pers.view_rocket || ent->deadflag)
		//ent->chasetarget = NULL;

//	if (ent->vehicle)
//		return;
	
	if (ent->chasetarget && ent->chasetarget->inuse && !Q_stricmp(ent->chasetarget->classname, "monster_car"))
	{
		AngleVectors (ent->chasetarget->s.angles, temp, NULL, NULL);
		VectorMA (ent->chasetarget->s.origin, 2, temp, temp);
		VectorClear (ent->client->ps.pmove.origin);

		//temp[0] += -40;	
		temp[0] *= 8;
		temp[1] *= 8;
		temp[2] += 8;
		temp[2] *= 8;
		temp[2] += 15;
		VectorCopy (temp, ent->client->ps.pmove.origin);
		
		ent->client->ps.gunindex = 0;
		VectorClear (ent->client->ps.viewoffset);
		//VectorCopy (ent->vehicle->s.angles, ent->client->ps.viewoffset);
		//gi.dprintf ("DEBUG: Rocket cam started!\n");

		//gi.dprintf ("Test\n");

		return;
	}
	else
	{
		if (!ent->deadflag && ent->client->pers.weapon)
			ent->client->ps.gunindex = ModelIndex(ent->client->pers.weapon->view_model);
		ent->chasetarget = NULL;
	}
}


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
	if (client->anim_priority < ANIM_PAIN && (player->s.modelindex != 255 && !player->client->cloak_turninvis_framenum))
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
	if ((level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
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
		gi.sound (player, CHAN_VOICE, SoundIndex(va("*pain%i_%i.wav", l, r)), 1, ATTN_NORM, 0);
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

		client->v_dmg_time = level.time + DAMAGE_TIME;
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
	edict_t		*Bub;
	int bubble = 0;
	int YDirection = 0, PDirection = 0, RDirection = 0;

//===================================
	
	if (ent->vehicle)
		ent->client->ps.gunindex = 0;
	if (ent->vehicle)
	{
		//VectorCopy (ent->vehicle->s.angles, angles);
		ent->client->ps.viewoffset[PITCH] = ent->vehicle->s.angles[PITCH];
		goto Setcam;
	}

	// base angles
	angles = ent->client->ps.kick_angles;

	// if dead, fix the angle and don't add any kick

	if (ent->deadflag)
	{
		//VectorClear (angles);

		//ent->client->ps.viewangles[ROLL] = 40;
		//ent->client->ps.viewangles[PITCH] = -15;
		//ent->client->ps.viewangles[YAW] = ent->client->killer_yaw;

		goto lol;
	}
	else
	{
lol:
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

		//add angles based on drunkness
		if (ent->DrunkTime > level.time)
		{
			bubble += 1;
			if (bubble == 10)
			{
				Bub = G_Spawn();
				Bub->movetype = MOVETYPE_NOCLIP;
				Bub->clipmask = MASK_SHOT;
				Bub->solid = SOLID_NOT;
				VectorClear (Bub->mins);
				VectorClear (Bub->maxs);
				VectorSet (Bub->velocity,0,0,10);
				Bub->s.modelindex = ModelIndex ("sprites/s_bubble.sp2");
				Bub->nextthink = level.time + 2;
				Bub->think = G_FreeEdict;
				VectorCopy(ent->s.origin,Bub->s.origin);
				gi.linkentity(Bub);
				bubble = 0;
			}
			ent->client->ps.rdflags |= RDF_UNDERWATER;

			if (ent->DizzyYaw < -35)
				YDirection = 1;
			if (ent->DizzyYaw > 35)
				YDirection = 0;
			if (ent->DizzyPitch < -25)
				PDirection = 1;
			if (ent->DizzyPitch > 25)
				PDirection = 0;
			if (ent->DizzyRoll < -25)
				RDirection = 1;
			if (ent->DizzyRoll > 25)
				RDirection = 0;

			if(YDirection)
				ent->DizzyYaw += random()*6;
			else
				ent->DizzyYaw -= random()*6;
			if(PDirection)
				ent->DizzyPitch += random()*5;
			else
				ent->DizzyPitch -= random()*5;
			if(RDirection)
				ent->DizzyRoll += random()*3;
			else
				ent->DizzyRoll -= random()*3;

			angles[YAW] += ent->DizzyYaw -= random()*6;
			angles[PITCH] += ent->DizzyPitch -= random()*6;
			angles[ROLL] += ent->DizzyRoll -= random()*6;
		}

		if (ent->headache_framenum > level.framenum)
		{
			ent->client->ps.rdflags |= RDF_UNDERWATER;

			if (ent->DizzyYaw < -35)
				YDirection = 1;
			if (ent->DizzyYaw > 35)
				YDirection = 0;
			if (ent->DizzyPitch < -25)
				PDirection = 1;
			if (ent->DizzyPitch > 25)
				PDirection = 0;
			if (ent->DizzyRoll < -25)
				RDirection = 1;
			if (ent->DizzyRoll > 25)
				RDirection = 0;

			if(YDirection)
				ent->DizzyYaw += random()*2;
			else
				ent->DizzyYaw -= random()*2;
			if(PDirection)
				ent->DizzyPitch += random()*2;
			else
				ent->DizzyPitch -= random()*2;
			if(RDirection)
				ent->DizzyRoll += random()*2;
			else
				ent->DizzyRoll -= random()*2;

			angles[YAW] += ent->DizzyYaw -= random()*2;
			angles[PITCH] += ent->DizzyPitch -= random()*2;
			angles[ROLL] += ent->DizzyRoll -= random()*2;
		}

	}

//===================================

	// base origin

Setcam:

	VectorClear (v);

	// add view height

	// Fix frozen thingy
	if (ent->frozen)
		v[2] += 24;

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

	// Paril
	UpdateRocketCam (ent); // new line!!
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

	if (ent->client->blindTime > 0)
	{
		float alpha = ent->client->blindTime / ent->client->blindBase;
		if (alpha > 1)
			alpha = 1;
		SV_AddBlend (1, 1, 1, alpha, ent->client->ps.blend);
	}

	ent->client->blindTime -= 1;
	
	// add for powerups
	/*ATTILA begin*/
	if ( Jet_Active(ent) )
	{
		/*GOD -> dont burn out*/
		if ( ent->flags & FL_GODMODE )
			if ( (ent->client->Jet_framenum - level.framenum) <= 100 )
				ent->client->Jet_framenum = level.framenum + 300;
			
			/*update the fuel time*/
			ent->client->Jet_remaining = ent->client->Jet_framenum - level.framenum;
			
			/*if no fuel remaining, remove jetpack from inventory*/ 
//			if ( (ent->client->Jet_framenum - level.framenum) == 0 )
//				ent->client->pers.inventory[ITEM_INDEX(FindItem("Jetpack"))] = 0;
			
			/*Play jetting sound every 0.6 secs (sound of monster icarus)*/
//			if ( ((int)ent->client->Jet_remaining % 6) == 0 )
//				gi.sound (ent, CHAN_AUTO, SoundIndex("hover/hovidle1.wav"), 0.9, ATTN_NORM, 0);
			
			/*beginning to fade if 4 secs or less*/
			if (ent->client->Jet_remaining <= 40)
				/*play on/off sound every sec*/
				if ( ((int)ent->client->Jet_remaining % 10) == 0 )
					gi.sound(ent, CHAN_ITEM, SoundIndex("items/protect.wav"), 1, ATTN_NORM, 0);
				
				//if (ent->client->Jet_remaining > 40 || ( (int)ent->client->Jet_remaining & 4) )
				//	SV_AddBlend (0, 0, 1, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, SoundIndex("items/damage2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 0, 1, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, SoundIndex("items/protect2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (1, 1, 0, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		remaining = ent->client->enviro_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, SoundIndex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 1, 0, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		remaining = ent->client->breather_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, SoundIndex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0.4, 1, 0.4, 0.04, ent->client->ps.blend);
	}

	if (ent->client->reg_framenum < level.framenum)
	{
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Regeneration"))] = 0;
	}

//PGM
	if (ent->client->ir_framenum > level.framenum)
	{
		remaining = ent->client->ir_framenum - level.framenum;
		if(remaining > 30 || (remaining & 4))
		{
			ent->client->ps.rdflags |= RDF_IRGOGGLES;
			SV_AddBlend (1, 0, 0, 0.2, ent->client->ps.blend);
		}
		else
			ent->client->ps.rdflags &= ~RDF_IRGOGGLES;
	}
	else
	{
		ent->client->ps.rdflags &= ~RDF_IRGOGGLES;
	}
//PGM

	// add for damage
	if (ent->client->damage_alpha > 0)
		SV_AddBlend (ent->client->damage_blend[0],ent->client->damage_blend[1]
		,ent->client->damage_blend[2], ent->client->damage_alpha, ent->client->ps.blend);

	if (ent->client->bonus_alpha > 0)
		SV_AddBlend (0.85, 0.7, 0.3, ent->client->bonus_alpha, ent->client->ps.blend);

	// drop the damage value
	ent->client->damage_alpha -= 0.06;
	if (ent->client->damage_alpha < 0)
		ent->client->damage_alpha = 0;

	// drop the bonus value
	ent->client->bonus_alpha -= 0.1;
	if (ent->client->bonus_alpha < 0)
		ent->client->bonus_alpha = 0;

	if (ent->burning_framenum > level.framenum)
	{
		float red;

		red = ((ent->burning_framenum - level.framenum) / 10) / 3;

		if (red > 1)
			red = 1;

//		gi.dprintf ("%s %f\n", ent->client->pers.netname, (ent->burning_framenum - level.framenum));
		SV_AddBlend (red, 0, 0, 0.6, ent->client->ps.blend);
	}
}


/*
=================
P_FallingDamage
=================
*/
//CW++
vec3_t vec3_up = {0.0F, 0.0F, 1.0F};
vec3_t vec3_dn = {0.0F, 0.0F, -1.0F};
vec3_t vec3_border = {4.0F, 4.0F, 4.0F};
//CW--

void P_FallingDamage (edict_t *ent)
{
	float	delta;
	int		damage;
	vec3_t	dir;
//CW++
	trace_t	tr;
	vec3_t	bmins;
	vec3_t	bmaxs;
	int		mod = 0;
	float	xyvel;
	float	xyvel_old;
//CW--

	//if ((int)dmflags->value & DF_NO_FALLING)
	//	return;

	if (ent->client->pers.temp_nofall)
	{
		ent->client->pers.temp_nofall = 0;
		return;
	}

	if (ent->s.modelindex != 255 && !ent->client->cloak_turninvis_framenum)
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
//CW++
			goto wallcheck;
//CW--
		delta = ent->velocity[2] - ent->client->oldvelocity[2];
	}

//CW++
	delta *= delta;
	delta *= (ent->is_bot && (ent->thrown_by_agm || ent->flung_by_agm))?0.0005:0.0001;
//CW--

//ZOID++
	// never take damage if just release grapple or on grapple
	if ((level.time - ent->client->ctf_grapplereleasetime <= FRAMETIME * 2.0) ||
		(ent->client->ctf_grapple && (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)))
//CW++
	{
		ent->thrown_by_agm = false;
		ent->flung_by_agm = false;

		if (!ent->held_by_agm)
			ent->agm_enemy = NULL;
//CW--
		return;
	}
//ZOID--

	// never take falling damage if completely underwater
	if (ent->waterlevel == 3)
		return;
	if (ent->waterlevel == 2)
		delta *= 0.25;
	if (ent->waterlevel == 1)
		delta *= 0.5;
	
	if (ent->client->boots_framenum > level.time)
		delta *= 0.5;

	if (delta < 1.0)
//CW++
	{
		if (!ent->waterlevel)
		{
			ent->thrown_by_agm = false;
			ent->flung_by_agm = false;

			if (!ent->held_by_agm)
				ent->agm_enemy = NULL;
		}
//CW--
		return;
	}


	// Lazarus: Changed here to NOT play footstep sounds if ent isn't on the ground.
	//          So player will no longer play footstep sounds when descending a ladder.
	if (delta < 15.0)
	{
//CW++
		ent->thrown_by_agm = false;
		ent->flung_by_agm = false;
		
		if (!ent->held_by_agm && !ent->laz_vehicle)
		{
			ent->agm_enemy = NULL;
			PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
//CW--
			ent->s.event = EV_FOOTSTEP;
		}
		return;
	}

	if (!(int)dmflags->value & DF_NO_FALLING)
	{
		ent->client->fall_value = delta*0.5;
		if (ent->client->fall_value > 40)
			ent->client->fall_value = 40;
		ent->client->fall_time = level.time + FALL_TIME;
	}

	if (delta > 30)
	{
		if (ent->health > 0)
		{
//CW++
			if (!ent->held_by_agm)
			{
				PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
//CW--
				if (!(int)dmflags->value & DF_NO_FALLING)
				{
					if (delta >= 55)
						ent->s.event = EV_FALLFAR;
					else
						ent->s.event = EV_FALL;
				}
			}
		}
		ent->pain_debounce_time = level.time;	// no normal pain sound
		damage = (delta-30)/2;
		if (damage < 1)
			damage = 1;
		VectorSet (dir, 0, 0, 1);

		//if (!deathmatch->value || !((int)dmflags->value & DF_NO_FALLING) )
//CW++
		//{
			if (ent->flung_by_agm || ent->held_by_agm)
			{
				if (ent->groundentity && (ent->client->oldvelocity[2] < 0.0))
				{
					if (ent->flung_by_agm)
					{
						mod = MOD_AGM_FLING;
						if ((ent->agm_enemy != NULL) && (ent->agm_enemy->client->quad_framenum > level.framenum))
							damage *= 4;
					}
					else
						mod = MOD_AGM_SMASH;
				}
			}
			else
			{
				if (ent->agm_enemy != NULL)
				{
					if (ent->client->oldvelocity[2] < 0.0)
					{
						mod = MOD_AGM_DROP;
						if (damage > 100)
							damage = 100 + (int)(0.2 * (damage - 100));
					}
				}
				else
					mod = MOD_FALLING;
			}

			if (mod)
			{
				if (mod != MOD_FALLING && ent->client->force_framenum < level.framenum)
					T_Damage(ent, world, ent->agm_enemy, dir, ent->s.origin, vec3_origin, damage, 0, 0, mod);
				else if (!(int)dmflags->value & DF_NO_FALLING)
				{
					if (ent->client->force_framenum > level.framenum)
						T_Damage (ent, world, ent->forcer, dir, ent->s.origin, vec3_origin, damage, 0, 0, MOD_FORCE_FALLING);
					else
						T_Damage(ent, world, world, dir, ent->s.origin, vec3_origin, damage, 0, 0, MOD_FALLING);
				}

				ent->thrown_by_agm = false;
				ent->flung_by_agm = false;

				if (!ent->held_by_agm)
					ent->agm_enemy = NULL;
			}
			return;
//CW--
//CW--
		//}
	}
	else
	{
//CW++
		ent->thrown_by_agm = false;
		ent->flung_by_agm = false;

		if (!ent->held_by_agm)
		{
			ent->agm_enemy = NULL;
			PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
//CW--
			ent->s.event = EV_FALLSHORT;
		}
		return;
	}

//CW++
wallcheck:

//	We're only interested in impact damage caused by an AGM wielder.

	if (ent->agm_enemy == NULL)
		return;

//	The victim can't get damaged if they're grappling.

	if ((level.time - ent->client->ctf_grapplereleasetime <= FRAMETIME * 2.0) ||
		(ent->client->ctf_grapple && (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)))
		return;

//	Check to see if we're very close to a hard surface. If so, there's a chance we've been
//	smacked into it hard enough to cause damage.

	VectorSubtract(ent->mins, vec3_border, bmins);
	VectorAdd(ent->maxs, vec3_border, bmaxs);
	tr = gi.trace(ent->s.origin, bmins, bmaxs, ent->s.origin, ent, MASK_PLAYERSOLID);
	if ((tr.fraction < 1.0) && !(tr.surface->flags & SURF_SKY))
	{
		if (!ent->agm_enemy->client->agm_pull)
		{

//			Determine previous and current XY-plane velocities.

			xyvel = sqrt((ent->velocity[0] * ent->velocity[0]) + (ent->velocity[1] * ent->velocity[1]));
			xyvel_old = sqrt((ent->client->oldvelocity[0] * ent->client->oldvelocity[0]) +
							 (ent->client->oldvelocity[1] * ent->client->oldvelocity[1]));
			delta = xyvel_old - xyvel;

//			If the XY-plane velocity change is insufficient to cause damage, check for a ceiling hit.

			if (delta < 30.0)
			{
				if ((ent->client->oldvelocity[2] > 0) && (ent->velocity[2] < ent->client->oldvelocity[2]))
					delta = ent->client->oldvelocity[2] - ent->velocity[2];
			}

//			If the change in velocity is large enough, apply damage.

			if (delta > 30.0)
			{	
				damage = (int)((ent->is_bot)?(0.15 * (delta - 30.0)):(0.03 * (delta - 30.0)));		//CW

				if (ent->agm_enemy->client->quad_framenum > level.framenum)
					damage *= 4;

				if (damage < 1)
					damage = 1;

				if (tr.ent->client)
				{
					if (!CheckTeamDamage(tr.ent, ent->agm_enemy))
						T_Damage(tr.ent, ent, ent->agm_enemy, ent->client->oldvelocity, tr.ent->s.origin, vec3_origin, damage, damage, 0, MOD_AGM_HIT);

					T_Damage(ent, tr.ent, ent->agm_enemy, vec3_origin, ent->s.origin, vec3_origin, damage, 0, 0, MOD_AGM_HIT);
				}
				else
				{
					if (ent->flung_by_agm)
						mod = MOD_AGM_FLING;
					else if (ent->held_by_agm)
						mod = MOD_AGM_SMASH;
					else
						mod = MOD_AGM_THROW;
					T_Damage(ent, world, ent->agm_enemy, vec3_origin, ent->s.origin, vec3_origin, damage, 0, 0, mod);
				}
			}
		}
	}
//CW--
}



/*
=============
P_WorldEffects
=============
*/
void P_WorldEffects (void)
{
	qboolean	breather;
	qboolean	envirosuit;
	int			waterlevel, old_waterlevel;

	if (current_player->movetype == MOVETYPE_NOCLIP)
	{
		current_player->air_finished = level.time + 12;	// don't need air
		return;
	}

	//Give air time if frozen 4/99 Acrid
	if (current_player->frozen)
		current_player->air_finished = level.time + 6;

	waterlevel = current_player->waterlevel;
	old_waterlevel = current_client->old_waterlevel;
	current_client->old_waterlevel = waterlevel;

	breather = current_client->breather_framenum > level.framenum;
	envirosuit = current_client->enviro_framenum > level.framenum;

	//
	// if just entered a water volume, play a sound
	//
	if (!old_waterlevel && waterlevel)
	{
		PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
		if (current_player->watertype & CONTENTS_LAVA)
			gi.sound (current_player, CHAN_BODY, SoundIndex("player/lava_in.wav"), 1, ATTN_NORM, 0);
		else if (current_player->watertype & CONTENTS_SLIME)
			gi.sound (current_player, CHAN_BODY, SoundIndex("player/watr_in.wav"), 1, ATTN_NORM, 0);
		else if (current_player->watertype & CONTENTS_WATER)
			gi.sound (current_player, CHAN_BODY, SoundIndex("player/watr_in.wav"), 1, ATTN_NORM, 0);
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
		gi.sound (current_player, CHAN_BODY, SoundIndex("player/watr_out.wav"), 1, ATTN_NORM, 0);
		current_player->flags &= ~FL_INWATER;
	}

	//
	// check for head just going under water
	//
	if (old_waterlevel != 3 && waterlevel == 3)
	{
		gi.sound (current_player, CHAN_BODY, SoundIndex("player/watr_un.wav"), 1, ATTN_NORM, 0);
	}

	//
	// check for head just coming out of water
	//
	if (old_waterlevel == 3 && waterlevel != 3)
	{
		if (current_player->air_finished < level.time)
		{	// gasp for air
			gi.sound (current_player, CHAN_VOICE, SoundIndex("player/gasp1.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
		}
		else  if (current_player->air_finished < level.time + 11)
		{	// just break surface
			gi.sound (current_player, CHAN_VOICE, SoundIndex("player/gasp2.wav"), 1, ATTN_NORM, 0);
		}
	}

	//
	// check for drowning
	//
	if (waterlevel == 3)
	{
		/*ATTILA begin*/
		//if ( Jet_Active(current_player) ) /*dont jet and dive and stay alive*/
		//	T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, current_player->health+1, 0, DAMAGE_NO_ARMOR, MOD_UNKNOWN); // MOD FIXME?
		/*ATTILA end*/

		// breather or envirosuit give air
		if (breather || envirosuit)
		{
			current_player->air_finished = level.time + 10;

			if (((int)(current_client->breather_framenum - level.framenum) % 25) == 0)
			{
				if (!current_client->breather_sound)
					gi.sound (current_player, CHAN_AUTO, SoundIndex("player/u_breath1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_AUTO, SoundIndex("player/u_breath2.wav"), 1, ATTN_NORM, 0);
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
					gi.sound (current_player, CHAN_VOICE, SoundIndex("player/drown1.wav"), 1, ATTN_NORM, 0);
				else if (rand()&1)
					gi.sound (current_player, CHAN_VOICE, SoundIndex("*gurp1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_VOICE, SoundIndex("*gurp2.wav"), 1, ATTN_NORM, 0);

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
				if (rand()&1)
					gi.sound (current_player, CHAN_VOICE, SoundIndex("player/burn1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_VOICE, SoundIndex("player/burn2.wav"), 1, ATTN_NORM, 0);
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

int IsEven (int x)
{
	if (x % 2 == 0)
		return 1;
	else
		return 0;
}

/*
===============
G_SetClientEffects
===============
*/
void Cmd_Gib_f (edict_t *ent);
void G_SetClientEffects (edict_t *ent)
{
	int		pa_type;
	int		remaining;
	float h = ((ent->burning_framenum - level.framenum) / 10);

	ent->s.effects = 0;
	ent->s.renderfx = 0;

	if (IsInfested(ent))
		ent->s.effects |= EF_FLIES;

	if (ent->client->cloak_turninvis_framenum < level.framenum + 10 && ent->client->cloak_turninvis_framenum > level.framenum)
		ent->s.effects |= EF_SPHERETRANS;
	else if (ent->client->cloak_turninvis_framenum < level.framenum)
	{
		ent->s.effects |= EF_SPHERETRANS;
		ent->s.renderfx |= RF_TRANSLUCENT;
		ent->s.modelindex = 0;
	}

	//if (ent->client->cloak_framenum > level.framenum)
	//	ent->client->cloak_framenum += 0.1;

	if (ent->client->cloak_framenum > level.framenum)
	{
		//if (ent->client->ucmd.forwardmove == 0)
		//	ent->s.effects |= EF_SPHERETRANS;
		//else
		//gi.dprintf ("%f\n", xyspeed);
		ent->s.renderfx |= RF_TRANSLUCENT;
		if (xyspeed/*ent->client->ucmd.forwardmove != 0 && ent->client->ucmd.sidemove != 0*/)
		{
			ent->client->cloak_turninvis_framenum = level.framenum + 20;
			ent->s.modelindex = 255;
		}
	}
	else
	{
		ent->s.renderfx &= ~RF_TRANSLUCENT;
		ent->s.effects &= ~EF_SPHERETRANS;
		if (!ent->vehicle)
			ent->s.modelindex = 255;
	}

	if (ent->client->star_framenum > level.framenum && ent->client->star_waitflash < level.framenum)
	{
		int ev = (ent->client->star_framenum - level.framenum);
		int h;
		ent->s.effects |= EF_COLOR_SHELL;

		if (ev <= 50)
			h = 4;
		else
			h = -1;

	//	gi.dprintf ("%f %i %i\n", ent->client->star_framenum, (int)ent->client->star_framenum, IsEven((int)ent->client->star_framenum));

		if (h != -1)
		{
			if (ev & h)
			{
				if (IsEven(ev))
					ent->s.renderfx |= RF_SHELL_RED;
				else
					ent->s.effects |= EF_DOUBLE;

				ent->s.effects |= EF_TAGTRAIL;
			}
		}
		else
		{
			if (IsEven(ev))
				ent->s.renderfx |= RF_SHELL_RED;
			else
				ent->s.effects |= EF_DOUBLE;
			
			ent->s.effects |= EF_TAGTRAIL;
		}
	}


	if (ent->deadflag && ent->infester) {
		ent->s.effects |= EF_FLIES;
	}

//ZOID
	CTFEffects(ent);
//ZOID

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

	if (ent->client->deflector_framenum > level.framenum)
	{
		ent->s.effects |= EF_POWERSCREEN;
	}

	if (ent->DrunkTime>level.time)
		ent->client->ps.rdflags |= RDF_UNDERWATER;

	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
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
	
	//acrid 3/99
	// Paril 2.00
	// Added a blue model thingy for frozen rather than white shell.
	// Changed color to blue.
	// Added EF_FLAG2
	if (ent->frozen)
	{
		VectorCopy (ent->s.origin, ent->freeze_ent->s.origin);
		ent->client->ps.rdflags |= RDF_UNDERWATER;
		ent->s.effects |= EF_FLAG2;
		SV_AddBlend (0, 0, 0.75, 0.6, ent->client->ps.blend);
	}

	ent->s.renderfx |= RF_IR_VISIBLE;

//	gi.dprintf ("%f\n", h);
	if (h && ent->waterlevel)
	{
		ent->burner_entity = NULL;
		ent->burning_framenum = 0;
	}

	if (!h)
		ent->burner_entity = NULL;
	if (h >= 1)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_RED;
	}
	if (h >= 2)
	{
		// Smoke!
		/*gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_CHAINFIST_SMOKE);
		gi.WritePosition (ent->s.origin);
		gi.multicast (ent->s.origin, MULTICAST_PVS);*/

		if (ent->fire_framenum < level.framenum)
		{
			CreateStickingFire (ent->burner_entity, ent->s.origin, ent);
			ent->fire_framenum = level.framenum + 40;
		}

		//gi.WriteByte (svc_temp_entity);
		//gi.WriteByte (TE_CHAINFIST_SMOKE);
		//gi.WritePosition (ent->s.origin);
		//gi.unicast (ent, 0);
	}
	if (h >= 10)
	{
		G_Spawn_Explosion (TE_EXPLOSION1, ent->s.origin, ent->s.origin);
		//Cmd_Gib_f (ent, ent->burner_entity);
		T_Damage (ent, ent->last_heater, ent->last_heater, vec3_origin, ent->s.origin, vec3_origin, ent->health + 41, 0, 0, MOD_HEATLASER);

		ent->burning_framenum = 0;
	}

	if (ent->tracker_framenum > level.framenum)
		ent->s.effects |= EF_TRACKERTRAIL;
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

	if (!ent->waterlevel && ( xyspeed > 225) && !ent->laz_vehicle && ent->groundentity)
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
		gi.sound (ent, CHAN_VOICE, SoundIndex ("misc/pc_up.wav"), 1, ATTN_STATIC, 0);
	}

	if (ent->client->star_framenum > level.framenum)
	{
		ent->s.sound = SoundIndex ("weapons/star.wav");
		return;
	}

	if (ent->client->dkhammer_framenum > level.framenum)
	{
		ent->s.sound = SoundIndex ("weapons/dkhammer.wav");
		return;
	}


	if (ent->client->pers.weapon)
		weap = ent->client->pers.weapon->classname;
	else
		weap = "";

	if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) )
		ent->s.sound = snd_fry;
	else if (strcmp(weap, "weapon_railgun") == 0)
		ent->s.sound = SoundIndex("weapons/rg_hum.wav");
	else if (strcmp(weap, "weapon_bfg") == 0)
		ent->s.sound = SoundIndex("weapons/bfg_hum.wav");
	else if (ent->client->pers.weapon && Q_stricmp (ent->client->pers.weapon->pickup_name, "Gravity Manipulator") == 0 && (ent->client->agm_target != NULL))
		ent->s.sound = SoundIndex("world/amb15.wav");
	// RAFAEL
	else if (strcmp (weap, "weapon_phalanx") == 0)
		ent->s.sound = SoundIndex ("weapons/phaloop.wav");
	else if (ent->client->weapon_sound)
		ent->s.sound = ent->client->weapon_sound;
	else
		ent->s.sound = 0;

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

	if( ent->frozen ) //acrid3 might not be needed
		return;

	if (ent->s.modelindex != 255 && !ent->client->cloak_turninvis_framenum)
		return;		// not in the player model

	client = ent->client;

	if (client->ps.pmove.pm_flags & PMF_DUCKED)
		duck = true;
	else
		duck = false;
	if (xyspeed)
		run = true;
	else
		run = false;

	// Lazarus: override run animations for vehicle drivers
	if (ent->laz_vehicle)
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
	{
		return;		// stay there
	}
	if (client->anim_priority == ANIM_JUMP && !ent->client->Jet_remaining)
	{
		if (!ent->client->Jet_remaining)
		{
			if (!ent->groundentity)
			{
				return;		// stay there
			}
			ent->client->anim_priority = ANIM_WAVE;
			ent->s.frame = FRAME_jump3;
			ent->client->anim_end = FRAME_jump6;
			return;
		}
	}
	else if (client->anim_priority == ANIM_JUMP && ent->client->Jet_remaining)
		goto newanim;

newanim:
	// return to either a running or standing frame
	client->anim_priority = ANIM_BASIC;
	client->anim_duck = duck;
	client->anim_run = run;

	if (!ent->groundentity)
	{
//ZOID: if on grapple, don't go into jump frame, go into standing
//frame
		if (client->ctf_grapple || ent->client->Jet_remaining) 
		{
			ent->s.frame = FRAME_stand01;
			client->anim_end = FRAME_stand40;
		} 
		else 
		{
//ZOID
		client->anim_priority = ANIM_JUMP;
		if (ent->s.frame != FRAME_jump2)
			ent->s.frame = FRAME_jump1;
		client->anim_end = FRAME_jump2;
		}
	}
	else if (run)
	{	// running
		if (client->forwardmove == -200 || client->forwardmove == -400)
		{
			if (duck)
			{
				ent->s.frame = FRAME_crwalk6;
				client->anim_end = FRAME_crwalk1;
			}
			else
			{
				ent->s.frame = FRAME_run6;
				client->anim_end = FRAME_run1;
			}

			client->anim_priority = ANIM_REVERSE;
		}
		else
		{
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
			client->anim_priority = 0;
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

char *GetRandomHint (void)
{
	char *hints[14];

	hints[0] = "Make sure to change your bindings if you haven't in mycfg.cfg!";
	hints[1] = "Type \"iwmhelp\" to see the help menu.";
	hints[2] = "Press a weapon's key more than once to use a subweapon";
	hints[3] = "Please don't spam the servers, this mod is technically still unbalanced!";
	hints[4] = va("The current version is %s", GAMEVERSION);
	hints[5] = "You can only use the WOMD and Volcano Launcher once per life with Infinite Ammo, so use it wisely!";

	return hints[rand()%5];
}


/*
=================
ClientEndServerFrame

Called for each player at the end of the server frame
and right after spawning
=================
*/
void MoveUpFromGround (edict_t *ent);
void SinkIntoGround (edict_t *ent);

void ClientEndServerFrame (edict_t *ent)
{
	float	bobtime;
	int		i;
	int tips = atoi(Info_ValueForKey(ent->client->pers.userinfo, "cg_hints"));

//	if (random() < 0.6)
//		return;
	if (IsLagged(ent) && random() < AmountToLag(ent))
		return;

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
		current_client->ps.pmove.origin[i] = ent->s.origin[i]*8.0;
		current_client->ps.pmove.velocity[i] = ent->velocity[i]*8.0;
	}

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if (level.intermissiontime)
	{
		// FIXME: add view drifting here?
		current_client->ps.blend[3] = 0;
		current_client->ps.fov = 90;
		G_SetStats (ent);
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, up);

	// burn from lava, etc
	P_WorldEffects ();

	//
	// set model angles from view angles so other things in
	// the world can tell which direction you are looking
	//
	if (ent->deadflag)
	{
		if (ent->deadflag)
			VectorCopy (ent->tempvec, ent->s.angles);
	}
	else
	{
		if (ent->client->resp.puddle_dest && ent->client->teleing)
		{
			if (ent->think4 == MoveUpFromGround)
			{
				vec3_t forward;
				float roll, yaw;
				AngleVectors (ent->client->resp.puddle_dest->s.angles, forward, NULL, NULL);
				roll = ent->client->resp.puddle_dest->s.angles[PITCH];
				yaw = ent->client->resp.puddle_dest->s.angles[YAW];
				roll += 90;

				//VectorCopy (or, ent->s.angles);
				if (ent->client->v_angle[PITCH] > 180)
					ent->s.angles[PITCH] = (-360 + ent->client->v_angle[PITCH])/3;
				else
					ent->s.angles[PITCH] = ent->client->v_angle[PITCH]/3;
				ent->s.angles[YAW] = yaw;
					ent->s.angles[PITCH] = ent->client->v_angle[YAW];
				ent->s.angles[PITCH] = roll;
			}
			else if (ent->think4 == SinkIntoGround)
			{
				vec3_t forward;
				float roll, yaw;
				AngleVectors (ent->client->resp.puddle_tele->s.angles, forward, NULL, NULL);
				roll = ent->client->resp.puddle_tele->s.angles[PITCH];
				yaw = ent->client->resp.puddle_tele->s.angles[YAW];
				roll += 90;

				//VectorCopy (or, ent->s.angles);
				if (ent->client->v_angle[PITCH] > 180)
					ent->s.angles[PITCH] = (-360 + ent->client->v_angle[PITCH])/3;
				else
					ent->s.angles[PITCH] = ent->client->v_angle[PITCH]/3;
				ent->s.angles[YAW] = yaw;
					ent->s.angles[PITCH] = ent->client->v_angle[YAW];
				ent->s.angles[PITCH] = roll;
			}
		}
		else
		{
			if (ent->movetype == MOVETYPE_NOCLIP)
					ent->s.angles[PITCH] = ent->client->v_angle[PITCH];
			else
			{
				if (ent->client->v_angle[PITCH] > 180)
					ent->s.angles[PITCH] = (-360 + ent->client->v_angle[PITCH])/3;
				else
					ent->s.angles[PITCH] = ent->client->v_angle[PITCH]/3;
			}
			ent->s.angles[YAW] = ent->client->v_angle[YAW];
			ent->s.angles[ROLL] = 0;
			ent->s.angles[ROLL] = SV_CalcRoll (ent->s.angles, ent->velocity)*4;
		}
	}

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

	// Lazarus: vehicle drivers don't bob
	if(ent->laz_vehicle)
		bobfracsin = 0.;
	else
		bobfracsin = fabs(sin(bobtime*M_PI));

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

	// Burn

		if (ent->fire_framenum > level.framenum + 80)
			ent->fire_framenum = level.framenum + 80;
		if (ent->fire_hurtframenum > level.framenum + 80)
			ent->fire_hurtframenum = level.framenum + 80;

		if (ent->fire_hurtframenum < level.framenum && ent->fire_framenum > level.framenum && ent->fire_entity)
		{
			T_Damage (ent, ent->fire_entity, ent->fire_entity->owner, vec3_origin, ent->s.origin, vec3_origin, 5, 0, DAMAGE_NO_ARMOR|DAMAGE_ENERGY, MOD_BURNT); // FIXMEH
			ent->fire_hurtframenum = level.framenum + 10;
		}
		if (ent->fire_framenum < level.framenum && ent->fire_entity) // Pssssss!
		{
			G_FreeEdict (ent->fire_entity);
			ent->fire_framenum = -1;
			ent->fire_entity = NULL;
		}

//ZOID
	if (!ent->client->chase_target)
//ZOID
		G_SetStats (ent);

//ZOID
//update chasecam follower stats
	for (i = 1; i <= maxclients->value; i++) {
		edict_t *e = g_edicts + i;
		if (!e->inuse || e->client->chase_target != ent)
			continue;
		memcpy(e->client->ps.stats, 
			ent->client->ps.stats, 
			sizeof(ent->client->ps.stats));
		e->client->ps.stats[STAT_LAYOUTS] = 1;
		break;
	}
//ZOID

	// chase cam stuff
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
	if (ent->client->showscores && !(level.framenum & 31))
	{
		if (ent->client->menu) 
		{
			PMenu_Do_Update(ent);
			ent->client->menudirty = false;
			ent->client->menutime = level.time;
		} 
		else
			DeathmatchScoreboardMessage (ent, ent->enemy);
		
		if (!ent->is_bot)
			gi.unicast (ent, false);
	}

//	SV_RunThink(ent);
//	SV_RunThink2(ent);
	//SV_RunThink3(ent);
//	SV_RunThink4(ent);
	RunThinks(ent);

//	gi.dprintf ("%s %i\n", ent->client->pers.netname, tips);

	// Paril 1.70.2000: Hints.
	// Display a hint every ten seconds.
	if (tips > 0)
	{
		if (ent->client->hint_framenum < 200)
		{
			safe_cprintf (ent, PRINT_HIGH, "REMINDER: %s\n", GetRandomHint());
			ent->client->hint_framenum += 200;
			if (tips > 1)
				unicastSound (ent, SoundIndex("misc/comp_up.wav"), 1);
		}

		ent->client->hint_framenum--;

	//	gi.dprintf ("%f\n", ent->client->hint_framenum);
	}
	if (!ent->client->resp.seconds)
		ent->client->resp.seconds = 0.1;
	else
		ent->client->resp.seconds += 0.1;

	if (ent->client->resp.seconds >= 60)
		ent->client->resp.seconds = 0;
}

