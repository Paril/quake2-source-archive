
#include "g_local.h"
#include "b_player.h"



static	edict_t		*current_player;
static	gclient_t	*current_client;

static	vec3_t	forward, right, up;
float	xyspeed;

float	bobmove;
int	bobcycle;		// odd cycles are right foot going forward
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
	if (client->damage_armor && !(player->flags & FL_GODMODE))
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
			player->s.frame = FRAME_CrouchPain_start - 1;
			client->anim_end = FRAME_CrouchPain_end;
		}
		else
		{
			i++;
         i %= 3;
			switch (i)
			{
			case 0:
				player->s.frame = FRAME_Pain1_start - 1;
				client->anim_end = FRAME_Pain1_end;
				break;
			case 1:
				player->s.frame = FRAME_Pain2_start - 1;
				client->anim_end = FRAME_Pain2_end;
				break;
			case 2:
            if (!player->groundentity && !player->waterlevel)
               {
				   player->s.frame = FRAME_Flip_start - 1;
				   client->anim_end = FRAME_Flip_end;
               }
            else
               {
				   player->s.frame = FRAME_Pain3_start - 1;
				   client->anim_end = FRAME_Pain3_end;
               }
				break;
			}
		}
	}

	realcount = count;
	if (count < 10)
		count = 10;	// always make a visible effect

	// play an apropriate pain sound
	if ((level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE))
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
	else if (!ent->client->camera_model)
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

      // Another goof id left. Leave commented out.
		//delta = bobfracsin * bob_roll->value * xyspeed;
		//if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			//delta *= 6;		// crouching

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

	// absolutely bound offsets
	// so the view can never be outside the player box
   if (!ent->client->camera_model)
      {
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
   else if (ent->client->resp.camera)
      {       
      if (ent->status & STATUS_3RD_VIEW)
         camera_think_3rd(ent->client->resp.camera);
      VectorCopy (ent->s.origin, ent->client->camera_model->s.origin);
      VectorCopy (ent->s.angles, ent->client->camera_model->s.angles);
      VectorCopy (ent->velocity, ent->client->camera_model->velocity);
      ent->client->camera_model->s.skinnum = ent->s.skinnum;
      ent->client->camera_model->s.effects = ent->s.effects;
      ent->client->camera_model->s.renderfx = ent->s.renderfx;
      gi.linkentity (ent->client->camera_model);

      ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
      ent->client->ps.pmove.origin[0] =
         ent->client->resp.camera->s.origin[0] * 8;
      ent->client->ps.pmove.origin[1] = 
         ent->client->resp.camera->s.origin[1] * 8;
      ent->client->ps.pmove.origin[2] = 
         (ent->client->resp.camera->s.origin[2] /*+ 
         ent->client->resp.camera->viewheight*/) * 8;
      if (!(ent->status & STATUS_3RD_VIEW))
         {
         VectorCopy (ent->client->resp.camera->s.angles, ent->client->ps.viewangles);
         // Add message text:
         if (ent->client->resp.camera->message)
            {
            char string[1400];
            string[0] = 0;
            Com_sprintf (string, 1400, "xv 0 yb -56 string2  \"%s\" ",
                           ent->client->resp.camera->message);
	         gi.WriteByte(svc_layout); 
	         gi.WriteString (string);
	         gi.unicast (ent, false);
            }
         }
      }

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

	if (ent->status & STATUS_SCENE_VIEW)
      {      
		SV_AddBlend (.7, .7, .7, 
         ent->client->resp.camera->decel, ent->client->ps.blend);
		return;
      }


	// add for contents
	if (!ent->client->resp.camera)
		{
		VectorAdd (ent->s.origin, ent->client->ps.viewoffset, vieworg);
		contents = gi.pointcontents (vieworg);
		}
	else
		{
		contents = gi.pointcontents (ent->client->resp.camera->s.origin);
		}
	if (ent->status & STATUS_IR)
      {      
		SV_AddBlend (0, .7 , 0, .4, ent->client->ps.blend);
		return;
      }
	if (ent->client->invincible > level.framenum)
      {      
		SV_AddBlend (0, 0, .7, .2, ent->client->ps.blend);
		return;
      }
	if (ent->client->beserk > level.framenum)
      {      
		SV_AddBlend (.7, .7, 0, .2, ent->client->ps.blend);
		if (ent->client->beserk > level.framenum + 290)
			ent->client->ps.fov = 150;
		else if (ent->client->beserk < level.framenum + 10)
			ent->client->ps.fov = atoi(Info_ValueForKey(ent->client->pers.userinfo, "fov"));
		return;
      }
	else if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER) )
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
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 0, 1, 0.08, ent->client->ps.blend);
	}

	// add for damage
	if (ent->client->damage_alpha > 0)
		SV_AddBlend (ent->client->damage_blend[0],ent->client->damage_blend[1]
		,ent->client->damage_blend[2], ent->client->damage_alpha, ent->client->ps.blend);

	if (ent->client->bonus_alpha > 0)
		SV_AddBlend (0.85, 0.7, 0.3, ent->client->bonus_alpha, ent->client->ps.blend);

   if (ent->client->teargas_time > level.time)
      {
      float k = (ent->client->teargas_time - level.time)/ 13;
      if (k > 1.0) k = 1.0;
		SV_AddBlend (.7, .7, .7, k, ent->client->ps.blend); 
      }

	// drop the damage value
	ent->client->damage_alpha -= 0.06;
	if (ent->client->damage_alpha < 0)
		ent->client->damage_alpha = 0;

	// drop the bonus value
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


	// never take falling damage if completely underwater
	if (ent->waterlevel == 3)
		return;
	if (ent->waterlevel == 2)
		delta *= 0.25;
	if (ent->waterlevel == 1)
		delta *= 0.5;

   // Bounce:
   if (ent->groundentity && (ent->groundentity->flipping & FLIP_TRAMPOLINE))
      {
      //gi.bprintf (PRINT_HIGH, "%f\n", ent->client->oldvelocity[2]);
      if (ent->client->oldvelocity[2] < -50)
         {
         if (ent->client->oldvelocity[2] < -500)
            ent->velocity[2] = 500;
         else
            ent->velocity[2] = -ent->client->oldvelocity[2] * ent->groundentity->speed;
         gi.sound (ent->groundentity, CHAN_BODY, gi.soundindex("world/wood1.wav"), 1, ATTN_NORM, 0);
         return;
         }
      }

	if (delta < 1)
		return;

	if (delta < 15)
	   {
		vec3_t end;
		trace_t trace;
		end[0] = ent->s.origin[0];
		end[1] = ent->s.origin[1];
		end[2] = ent->s.origin[2] - 90;
		trace = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SOLID);
		if (trace.surface->flags & TEXTURE_DIRT)
			{
			switch (rand() & 3)
				{
				case 0:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_dirt1.wav"), 1, ATTN_STATIC, 0);
					break;
				case 1:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_dirt2.wav"), 1, ATTN_STATIC, 0);
					break;
				case 2:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_dirt3.wav"), 1, ATTN_STATIC, 0);
					break;
				case 3:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_dirt4.wav"), 1, ATTN_STATIC, 0);
					break;
				}
			}
		else if (trace.surface->flags & TEXTURE_METAL)
			{
			switch (rand() & 3)
				{
				case 0:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_metal1.wav"), 1, ATTN_STATIC, 0);
					break;
				case 1:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_metal2.wav"), 1, ATTN_STATIC, 0);
					break;
				case 2:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_metal3.wav"), 1, ATTN_STATIC, 0);
					break;
				case 3:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_metal4.wav"), 1, ATTN_STATIC, 0);
					break;
				}
			}
		else if (trace.surface->flags & TEXTURE_TILES)
			{
			switch (rand() & 3)
				{
				case 0:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_tile1.wav"), 1, ATTN_STATIC, 0);
					break;
				case 1:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_tile2.wav"), 1, ATTN_STATIC, 0);
					break;
				case 2:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_tile3.wav"), 1, ATTN_STATIC, 0);
					break;
				case 3:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_tile4.wav"), 1, ATTN_STATIC, 0);
					break;
				}
			}
		else 
			{
			switch (rand() & 3)
				{
				case 0:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_step1.wav"), 1, ATTN_STATIC, 0);
					break;
				case 1:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_step2.wav"), 1, ATTN_STATIC, 0);
					break;
				case 2:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_step3.wav"), 1, ATTN_STATIC, 0);
					break;
				case 3:
					gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_step4.wav"), 1, ATTN_STATIC, 0);
					break;
				}
			}
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
		// Modified by Lkebsh
      damage = delta * 1.5;
      // damage = (delta-30)/2;
		if (damage < 1)
			damage = 1;
		VectorSet (dir, 0, 0, 1);

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
P_WorldEffects
=============
*/
void P_WorldEffects (void)
{
	int   waterlevel, 
         old_waterlevel;

	if (current_player->movetype == MOVETYPE_NOCLIP)
	{
		current_player->air_finished = level.time + 12;	// don't need air
		return;
	}

	waterlevel = current_player->waterlevel;
	old_waterlevel = current_client->old_waterlevel;
	current_client->old_waterlevel = waterlevel;

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
	if (old_waterlevel && ! waterlevel)
	{
		PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
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
	if (waterlevel && (current_player->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) )
	   {
		if (current_player->watertype & CONTENTS_LAVA)
		   {
			if (current_player->health > 0
				&& current_player->pain_debounce_time <= level.time)
			   {
				if (rand()&1)
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/burn1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/burn2.wav"), 1, ATTN_NORM, 0);
				current_player->pain_debounce_time = level.time + 1;
			   }   

				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 3*waterlevel, 0, 0, MOD_LAVA);
		   }

		if (current_player->watertype & CONTENTS_SLIME)
			T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 1*waterlevel, 0, 0, MOD_SLIME);
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

	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			ent->s.effects |= EF_QUAD;
	}

	// show cheaters!!!
	if (ent->flags & FL_GODMODE && (deathmatch->value || coop->value))
	   {
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_RED;
      //ent->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
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
      if (ent->client->anim_priority == ANIM_BASIC)
      if (ent->client->anim_end != FRAME_Run_end)
         {
         ent->client->anim_end = FRAME_Run_end;
         ent->s.frame = FRAME_Run_start - 1;
         }
		if ( (int)(current_client->bobtime+bobmove) != bobcycle )
			{
			vec3_t end;
			trace_t trace;
			end[0] = ent->s.origin[0];
			end[1] = ent->s.origin[1];
			end[2] = ent->s.origin[2] - 90;
			trace = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SOLID);
			if (trace.surface->flags & TEXTURE_DIRT)
				{
				switch (rand() & 3)
					{
					case 0:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_dirt1.wav"), 1, ATTN_STATIC, 0);
						break;
					case 1:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_dirt2.wav"), 1, ATTN_STATIC, 0);
						break;
					case 2:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_dirt3.wav"), 1, ATTN_STATIC, 0);
						break;
					case 3:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_dirt4.wav"), 1, ATTN_STATIC, 0);
						break;
					}
				}
			else if (trace.surface->flags & TEXTURE_METAL)
				{
				switch (rand() & 3)
					{
					case 0:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_metal1.wav"), 1, ATTN_STATIC, 0);
						break;
					case 1:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_metal2.wav"), 1, ATTN_STATIC, 0);
						break;
					case 2:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_metal3.wav"), 1, ATTN_STATIC, 0);
						break;
					case 3:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_metal4.wav"), 1, ATTN_STATIC, 0);
						break;
					}
				}
			else if (trace.surface->flags & TEXTURE_TILES)
				{
				switch (rand() & 3)
					{
					case 0:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_tile1.wav"), 1, ATTN_STATIC, 0);
						break;
					case 1:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_tile2.wav"), 1, ATTN_STATIC, 0);
						break;
					case 2:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_tile3.wav"), 1, ATTN_STATIC, 0);
						break;
					case 3:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_tile4.wav"), 1, ATTN_STATIC, 0);
						break;
					}
				}
			else 
				{
				switch (rand() & 3)
					{
					case 0:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_step1.wav"), 1, ATTN_STATIC, 0);
						break;
					case 1:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_step2.wav"), 1, ATTN_STATIC, 0);
						break;
					case 2:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_step3.wav"), 1, ATTN_STATIC, 0);
						break;
					case 3:
						gi.sound (ent, CHAN_BODY, gi.soundindex ("sfx/pl_step4.wav"), 1, ATTN_STATIC, 0);
						break;
					}
				}
			//ent->s.event = EV_FOOTSTEP;
			}
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
   edict_t *entv = ent;
   
   //ADDED:
   if (ent->client->camera_model)
      entv = ent->client->camera_model;

	if (ent->client->resp.game_helpchanged != game.helpchanged)
	{
		ent->client->resp.game_helpchanged = game.helpchanged;
		ent->client->resp.helpchanged = 1;
	}

	// help beep (no more than three times)
	if (ent->client->resp.helpchanged && ent->client->resp.helpchanged <= 3 && !(level.framenum&63) )
	{
		ent->client->resp.helpchanged++;
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("misc/pc_up.wav"), 1, ATTN_STATIC, 0);
	}


	if (ent->client->pers.weapon)
		weap = ent->client->pers.weapon->classname;
	else
		weap = "";

	if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) )
		entv->s.sound = snd_fry;
	else if (ent->client->weapon_sound)
		entv->s.sound = ent->client->weapon_sound;
	else
		entv->s.sound = 0;
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

	if (ent->s.modelindex != 255)
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

	// check for stand/duck and stop/go transitions
	if (duck != client->anim_duck && client->anim_priority < ANIM_DEATH)
		goto newanim;
	if (client->anim_priority == ANIM_SWIM && !ent->waterlevel)
		goto newanim;
	if (run != client->anim_run && client->anim_priority == ANIM_BASIC)
		goto newanim;
	if (!ent->groundentity && client->anim_priority <= ANIM_WAVE)
      if (ent->velocity[2] < -220 || ent->velocity[2] > 0)
			goto newanim;
   if (client->sidemove && !ent->waterlevel && client->anim_priority == ANIM_BASIC)
      goto newanim;

	if (ent->s.frame < client->anim_end)
	   {	// continue an animation
		ent->s.frame++;
      if (ent->client->camera_model)
         {
         ent->client->camera_model->s.frame = ent->s.frame;
         ent->client->camera_model->s.modelindex = ent->s.modelindex;
         ent->client->camera_model->s.modelindex2 = ent->s.modelindex2;
         ent->client->camera_model->s.modelindex3 = ent->s.modelindex3;
         ent->client->camera_model->s.modelindex4 = ent->s.modelindex4;
         }         
		return;
	   }

	if (client->anim_priority == ANIM_DEATH)
		return;		// stay there
	if (client->anim_priority == ANIM_JUMP)
	   {
      if (ent->waterlevel)
		   goto newanim;
		if (!ent->groundentity)
         {
         if (ent->velocity[2] < -400)
            {
            client->anim_priority = ANIM_FLIP;
            ent->s.frame = FRAME_Flip_start;
            client->anim_end = FRAME_Flip_end;
            }  
         return;
         }
		ent->client->anim_priority = ANIM_WAVE;
		ent->s.frame = FRAME_Jump_stay;
		ent->client->anim_end = FRAME_Jump_end;
		return;
	   }

newanim:
	// return to either a running or standing frame
	client->anim_priority = ANIM_BASIC;
	client->anim_duck = duck;
	client->anim_run = run;

   if (ent->waterlevel > 2)
      {
      if (ent->maxs[0] != 38)
         {
         VectorSet(ent->maxs, 38, 10, 32);
         VectorSet(ent->mins, -38, -10, 0);
         }
      client->anim_priority = ANIM_SWIM;
		ent->s.frame = FRAME_Swim_start;
		client->anim_end = FRAME_Swim_end;
      return;
		}
   if (ent->mins[0] != -24);
      {
      VectorSet(ent->maxs, 16, 16, 32);
      VectorSet(ent->mins, -16, -16, -24);
      }
	if (!ent->groundentity)
	   {
      if (ent->waterlevel && ent->waterlevel < 3)
         {
         ent->mins[2] = -40;
         ent->maxs[2] = 16;
         client->anim_priority = ANIM_SWIM;
         ent->s.frame = FRAME_SwimStand_start;
         client->anim_end = FRAME_SwimStand_end;
         return;
         }
      if ((ent->velocity[2] < -220 && ent->velocity[2] >= -400) 
			|| ent->velocity[2] > 0)
         {
         client->anim_priority = ANIM_JUMP;
		   if (ent->s.frame != FRAME_Jump_stay - 1)
	         ent->s.frame = FRAME_Jump_start;
         client->anim_end = FRAME_Jump_stay - 1;
         return;
         }
      else if (ent->velocity[2] < -400)
         {
         client->anim_priority = ANIM_FLIP;
         ent->s.frame = FRAME_Flip_start;
         client->anim_end = FRAME_Flip_end;
         return;
         }
	   }

	if (run) // running
      {
		if (duck)
		   {
         if (client->sidemove > 0)
            {
            client->anim_priority = ANIM_WAVE;
            ent->s.frame = FRAME_CrouchRight_start;
			   client->anim_end = FRAME_CrouchRight_end;
            }
         else if (client->sidemove < 0)
            {
            client->anim_priority = ANIM_WAVE;
			   ent->s.frame = FRAME_CrouchLeft_start;
			   client->anim_end = FRAME_CrouchLeft_end;
            }
         else
            {
			   ent->s.frame = FRAME_CrouchWalk_start;
			   client->anim_end = FRAME_CrouchWalk_end;
            }
         return;
		   }
      if (client->sidemove > 0)
         {
         client->anim_priority = ANIM_WAVE;
         ent->s.frame = FRAME_Right_start;
			client->anim_end = FRAME_Right_end;
         }
      else if (client->sidemove < 0)
         {
         client->anim_priority = ANIM_WAVE;
			ent->s.frame = FRAME_Left_start;
			client->anim_end = FRAME_Left_end;
         }
		else
		   {
         if (xyspeed > 225)
            {
			   ent->s.frame = FRAME_Run_start;
			   client->anim_end = FRAME_Run_end;
            }
         else
            {
			   ent->s.frame = FRAME_Walk_start;
			   client->anim_end = FRAME_Walk_end;
            }
		   }
      return;
	   }

	if (duck)   // standing
	   {
		ent->s.frame = FRAME_Crouch_start;
		client->anim_end = FRAME_Crouch_end;
	   }
	else
	   {
		ent->s.frame = FRAME_Stand_start;
		client->anim_end = FRAME_Stand_end;
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

   //ent->client->ps.kick_angles[2] += 90;

	// if the scoreboard is up, update it
	if (ent->client->showscores && !(level.framenum & 31) )
	{
		DeathmatchScoreboardMessage (ent, ent->enemy);
		gi.unicast (ent, false);
	}
}

