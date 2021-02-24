#include "g_local.h"
#include "m_player.h"

static	edict_t		*current_player;
static	gclient_t	*current_client;

static	vec3_t	forward, right, up;
float   xyspeed; // See z_brazen.h

float	bobmove;
int     bobcycle;               // odd cycles are right foot going forward
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

        // GRIM
        if ((player->deadflag) || (player->movetype == MOVETYPE_NOCLIP))
                return;
        // GRIM

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

        // GRIM
	if ((level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE)
         && (client->invincible_framenum <= level.framenum) && !player->deadflag)
        { // GRIM
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


	// base angles
	angles = ent->client->ps.kick_angles;

        // GRIM
        if (ent->movetype == MOVETYPE_NOCLIP)
        {
		VectorClear (angles);
                return;
        }
        // GRIM

	// if dead, fix the angle and don't add any kick
        if (ent->deadflag == DEAD_DEAD) // GRIM
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
                // GRIM
                else if ((ent->client->pers.wounds[W_RLEG] > 2) && (bobcycle & 1))
                {
                        if (ent->client->pers.wounds[W_RLEG] < 8)
                                delta *= 7 + ent->client->pers.wounds[W_RLEG];
                        else
                                delta *= 15;

                        if (ent->groundentity)
                        {
                                if (ent->client->pers.wounds[W_RLEG] > 5)
                                        ent->velocity[2] += 1;
                                ent->velocity[0] *= 0;
                                ent->velocity[1] *= 0;
                        }
                } // If SEVERE leg wounds, more hobbling
                else if ((ent->client->pers.wounds[W_LLEG] > 6) && (bobcycle & 2))
                {
                        if (ent->client->pers.wounds[W_LLEG] < 8)
                                delta *= 10 + ent->client->pers.wounds[W_LLEG];
                        else
                                delta *= 20;

                        if (ent->groundentity)
                        {
                                if (ent->client->pers.wounds[W_LLEG] > 5)
                                        ent->velocity[2] += 1;
                                ent->velocity[0] *= 0;
                                ent->velocity[1] *= 0;
                        }
                }
                // GRIM

		angles[PITCH] += delta;
		delta = bobfracsin * bob_roll->value * xyspeed;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			delta *= 6;		// crouching
		if (bobcycle & 1)
			delta = -delta;
		angles[ROLL] += delta;

                // GRIM
                if (ent->client->concus_time > level.time)
                {
                        int n; 
                        
                        n = rndnum(1, 3); 
                        if ((!ent->velocity[0]) && (!ent->velocity[1]))
                                ent->client->concus_time -= 0.4;

                        ent->client->ps.rdflags |= RDF_UNDERWATER;

                        if (n == 1) 
                                ent->client->dizzy[0] += random()*2; //6 
                        else 
                                ent->client->dizzy[0] -= random()*2; //6 

                        if (n == 2)
                                ent->client->dizzy[1] += random()*2; //5 
                        else 
                                ent->client->dizzy[1] -= random()*2; //5 

                        if (n == 3)
                                ent->client->dizzy[2] += random()*2; //5 
                        else 
                                ent->client->dizzy[2] -= random()*2; //5 

                        angles[YAW] += ent->client->dizzy[0]; 
                        angles[PITCH] += ent->client->dizzy[1]; 
                        angles[ROLL] += ent->client->dizzy[2]; 
                }
                // GRIM
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

        if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER))
		ent->client->ps.rdflags |= RDF_UNDERWATER;
	else
		ent->client->ps.rdflags &= ~RDF_UNDERWATER;

        if (contents & (CONTENTS_SOLID|CONTENTS_LAVA))
                SV_AddBlend (1.0, 0.3, 0.0, 0.6, ent->client->ps.blend);
        else if (contents & CONTENTS_SLIME)
                SV_AddBlend (0.0, 0.1, 0.05, 0.6, ent->client->ps.blend);
        else if (contents & CONTENTS_WATER)
                SV_AddBlend (0.5, 0.3, 0.2, 0.4, ent->client->ps.blend);

        // GRIM - Heh heh, BLIND drunk, ha ha
        if (ent->blindTime > level.time)
        {
                float alpha = (ent->blindTime - level.time) / 5;
                
                if (alpha > 1)
                        alpha = 1;

                SV_AddBlend (1, 1, 1, alpha, ent->client->ps.blend);
        }
        // GRIM

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
// GRIM - Take all filters
                if (remaining < 11)
                        ent->client->pers.inventory[ITEM_INDEX(FindItem("Filters"))] = 0;
                else                        
                        ent->client->pers.inventory[ITEM_INDEX(FindItem("Filters"))] = remaining / 10;
// GRIM
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
        int     damage;
	vec3_t	dir;
        edict_t *thisguy; // GRIM
        int     leg;

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
        // GRIM - Fuck that. Take less, not none.
	if (ent->waterlevel == 3)
                delta *= 0.35; //return;
                
	if (ent->waterlevel == 2)
                delta *= 0.7; // 0.25
        //if (ent->waterlevel == 1)
        //        delta *= 0.5;
        // GRIM

	if (delta < 1)
		return;

        // Dirty
        if (ent->client->pers.gravity != (int)sv_gravity->value)
                delta *= ((float)ent->client->pers.gravity / 800);
        // Dirty

	ent->client->fall_value = delta*0.5;
	if (ent->client->fall_value > 40)
		ent->client->fall_value = 40;
	ent->client->fall_time = level.time + FALL_TIME;

// GRIM
        if (ent->blown_away)
        {
                thisguy = ent->blown_away;
                ent->blown_away = NULL;
        }
        else
                thisguy = ent;

	if (delta > 30)
	{
                // Dirty
                damage = ((delta-30)*3); // ((delta-30)/2);
                // Dirty

                // Less falling damage if crouched,
                // except when its a long drop.
                if ((delta <= 60) && (ent->client->ps.pmove.pm_flags & PMF_DUCKED))
                        damage *= 0.5;

                if (damage < 1)
                        damage = 0;
                
		VectorSet (dir, 0, 0, 1);
                
                //if (!deathmatch->value || !((int)dmflags->value & DF_NO_FALLING) )
                if (damage > 0)
                {
                        if (ent->groundentity && ent->groundentity->client && !ent->deadflag && !ent->is_a_lame_mofo)
                        {
                                T_Damage (ent->groundentity, world, ent, dir, ent->s.origin, vec3_origin, (damage *= 0.6), 0, HIT_HEAD, TOD_NO_ARMOR, MOD_LAND);
                                T_Damage (ent, world, thisguy, dir, ent->s.origin, vec3_origin, (damage *= 0.4), 0, HIT_RIGHT_LEG|HIT_LEFT_LEG|HIT_UPPER_LEGS, TOD_NO_ARMOR, MOD_FALLING);
                        }
                        else
                                T_Damage (ent, world, thisguy, dir, ent->s.origin, vec3_origin, damage, 0, HIT_RIGHT_LEG|HIT_LEFT_LEG|HIT_UPPER_LEGS, TOD_NO_ARMOR, MOD_FALLING);
                        ent->groundentity = NULL;
                }

                if ((ent->health > 0) && (ent->waterlevel < 2))
		{
                        if (delta >= 55)
				ent->s.event = EV_FALLFAR;
			else
				ent->s.event = EV_FALL;
		}
		ent->pain_debounce_time = level.time;	// no normal pain sound
	}
        else
	{
                // Dirty
                if ((ent->client->pers.specials & SI_STEALTH_SLIPPERS) && (ent->client->pers.armor == NULL))
                        return;
                // Dirty

                leg = LegWounds(ent);

                if ((delta < 15) && (!ent->waterlevel))
                {
                        if ((ent->weight > 60) || leg)
                                ent->s.event = EV_FOOTSTEP;
                }
                else if ((ent->waterlevel < 2) && ent->groundentity && ((ent->weight > 35) || leg))
                        ent->s.event = EV_FALLSHORT;
	}
        // GRIM
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
        int             waterlevel, old_waterlevel;
// GRIM
        edict_t *thisguy; 
// GRIM

        // GRIM - +FL_GODMODE
        if ((current_player->movetype == MOVETYPE_NOCLIP) || (current_player->flags & FL_GODMODE))
	{
		current_player->air_finished = level.time + 12;	// don't need air
		return;
	}

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
                // GRIM
                //PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
                // GRIm
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
                // GRIM
                //PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
                // GRIM
		gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_out.wav"), 1, ATTN_NORM, 0);
		current_player->flags &= ~FL_INWATER;
	}

	//
	// check for head just going under water
	//
        if (old_waterlevel != 3 && waterlevel == 3)
		gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_un.wav"), 1, ATTN_NORM, 0);

	//
	// check for head just coming out of water
	//
        if (old_waterlevel == 3 && waterlevel != 3)
	{
		if (current_player->air_finished < level.time)
		{	// gasp for air
			gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/gasp1.wav"), 1, ATTN_NORM, 0);
                        //PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
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
                                // GRIM - No, let it grow, let it grow!
                                current_player->dmg += 9; // 2
                                //if (current_player->dmg > 15)
                                //        current_player->dmg = 15;
                                // GRIM
                                current_player->client->damage_blood += current_player->dmg;

				// play a gurp sound instead of a normal pain sound
				if (current_player->health <= current_player->dmg)
                                { // GRIM
                                        // GRIM
                                        gi.sound (current_player, CHAN_VOICE, gi.soundindex("*drown1.wav"), 1, ATTN_NORM, 0);
                                        // GRIM
                                        current_player->health = 0;
                                        meansOfDeath = MOD_WATER;
                                        player_die (current_player, world, world, 100000, current_player->s.origin);
                                        
                                } // GRIM
				else if (rand()&1)
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("*gurp1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("*gurp2.wav"), 1, ATTN_NORM, 0);

				current_player->pain_debounce_time = level.time;

                                //T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, current_player->dmg, 0, 0, 48, MOD_WATER);
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
        if (waterlevel && (current_player->watertype & (CONTENTS_LAVA|CONTENTS_SLIME)) )
	{
                // GRIM
                if (current_player->blown_away)
                        thisguy = current_player->blown_away;
                else
                        thisguy = current_player;
                current_player->blown_away = NULL;
                // GRIM

		if (current_player->watertype & CONTENTS_LAVA)
		{
			if (current_player->health > 0
				&& current_player->pain_debounce_time <= level.time
				&& current_client->invincible_framenum < level.framenum)
			{
				if (rand()&1)
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/burn1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/burn2.wav"), 1, ATTN_NORM, 0);
				current_player->pain_debounce_time = level.time + 1;
			}

                        // GRIM - Hugely increased. It's fucking lava!
			if (envirosuit)	// take 1/3 damage with envirosuit
                                T_Damage (current_player, world, thisguy, vec3_origin, current_player->s.origin, vec3_origin, 6*waterlevel, 0, 0, 160, MOD_LAVA);
			else
                                T_Damage (current_player, world, thisguy, vec3_origin, current_player->s.origin, vec3_origin, 18*waterlevel, 0, 0, 160, MOD_LAVA);
                        // GRIM
		}

		if (current_player->watertype & CONTENTS_SLIME)
		{
			if (!envirosuit)
			{	// no damage from slime with envirosuit
                                // GRIM
                                T_Damage (current_player, world, thisguy, vec3_origin, current_player->s.origin, vec3_origin, 1*waterlevel, 0, 0, 160, MOD_SLIME);
                                // GRIM
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

        // GRIM
        // Always IR visible, even when dead
        ent->s.renderfx |= RF_IR_VISIBLE;
        // GRIM

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

        CTFEffects(ent); // GRIM

        // GRIM
        if (ent->client->quad_framenum > level.framenum && (level.framenum & 8))
        // GRIM
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

        // Dirty        
        if (ent->is_a_lame_mofo) // Team killing moron here
        {
                ent->s.effects |= EF_BLASTER;

                if (level.framenum & 8)
                        ent->s.effects |= EF_QUAD;
                else if (level.framenum & 16)
                        ent->s.effects |= EF_PENT;
        }
        // Dirty        

// GRIM
        if (ent->client->concus_time > level.time)
                ent->client->ps.rdflags |= RDF_UNDERWATER;
// GRIM

        if (!(ent->s.renderfx & RF_IR_VISIBLE))
                ent->s.renderfx |= RF_IR_VISIBLE;
}

/*
===============
G_SetClientEvent
===============
*/
// GRIM
void G_SetClientEvent (edict_t *ent)
{
	if (ent->s.event)
		return;

        if (ent->waterlevel && (!(ent->client->pers.specials & SI_SILENT)) && (xyspeed > 20))
        {
                if (ent->waterlevel == 3 && (!ent->groundentity))
                        return;

                if ((int)(current_client->bobtime+bobmove) != bobcycle)
                        gi.sound (ent, CHAN_AUTO, gi.soundindex(va("player/wade%i.wav", (rand()%3)+1)), 1, ATTN_NORM, 0);
                return;
        }

        // Dirty
        if ((ent->client->pers.specials & SI_STEALTH_SLIPPERS)
         && (ent->client->pers.armor == NULL))
		return;
        // Dirty

        if ((!ent->groundentity) || (ent->client->ps.pmove.pm_flags & PMF_DUCKED))
		return;

        // Dirty
        //if ((ent->weight < 50) && (!LegWounds(ent)))
        //        return;
        // Dirty

        if ((xyspeed > 225) || (ent->weight > 150))
	{
                if ((int)(current_client->bobtime+bobmove) != bobcycle)
                        ent->s.event = EV_FOOTSTEP;
        }
}
// GRIM

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

        // GRIM
        if (ent->waterlevel && (ent->watertype & (CONTENTS_LAVA|CONTENTS_SLIME)))
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

/*
===============
G_SetClientFrame
===============
*/
// GRIM
void CreateBodyInventory (edict_t *ent);
// GRIM

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
	if (run != client->anim_run && client->anim_priority == ANIM_BASIC)
		goto newanim;
	if (!ent->groundentity && client->anim_priority <= ANIM_WAVE)
		goto newanim;

        if (client->anim_priority == ANIM_REVERSE)
	{
                if (ent->s.frame > client->anim_end)
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
                // GRIM
                // Moved to here...
                ent->svflags |= SVF_DEADMONSTER;
                ent->flags |= FL_NO_KNOCKBACK;
                ent->maxs[2] = -8;

                // Copy out inventory to the hold entity...
                if (!ent->master)
                        CreateBodyInventory (ent);
                // GRIM
		return;		// stay there
        }
        // GRIM - Shot to shit
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
        // GRIM
        if (ent->deadflag)
        {
                ent->s.angles[PITCH] = 0;
                ent->client->hanging = false;
        }
        else if (ent->action & A_SOMERSAULT)
        {
                ent->client->anim_priority = ANIM_JUMP;
                if (ent->s.frame != FRAME_jump2)
                        ent->s.frame = FRAME_jump1;
                ent->client->anim_end = FRAME_jump2;
                ent->s.angles[PITCH] += 45;
        }
        else if (ent->action & A_BACKFLIP)
        {
                ent->client->anim_priority = ANIM_JUMP;
                if (ent->s.frame != FRAME_jump2)
                        ent->s.frame = FRAME_jump1;
                ent->client->anim_end = FRAME_jump2;
                ent->s.angles[PITCH] -= 45;
        }
        else if (ent->client->v_angle[PITCH] > 180)
        // GRIM
		ent->s.angles[PITCH] = (-360 + ent->client->v_angle[PITCH])/3;
	else
		ent->s.angles[PITCH] = ent->client->v_angle[PITCH]/3;
	ent->s.angles[YAW] = ent->client->v_angle[YAW];

        // GRIM
        if (ent->action & A_RIGHTFLIP)
                ent->s.angles[ROLL] -= 45;
        else if (ent->action & A_LEFTFLIP)
                ent->s.angles[ROLL] += 45;
        else
        {
                ent->s.angles[ROLL] = 0;
                ent->s.angles[ROLL] = SV_CalcRoll (ent->s.angles, ent->velocity)*4;
        }
        // GRIM
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

	// chase cam stuff
        // GRIM
        G_SetStats (ent);
        // GRIM

        // GRIM - why go throught everyone...
        //G_CheckChaseStats(ent);
        // GRIM

	G_SetClientEvent (ent);

	G_SetClientEffects (ent);

	G_SetClientSound (ent);

	G_SetClientFrame (ent);

	VectorCopy (ent->velocity, ent->client->oldvelocity);
	VectorCopy (ent->client->ps.viewangles, ent->client->oldviewangles);

	// clear weapon kicks
	VectorClear (ent->client->kick_origin);
	VectorClear (ent->client->kick_angles);

// GRIM
        if (!(ent->flags & FL_GODMODE))
                HealthCalc(ent);                

        if (ent->pre_solid > 0)
        {
                if (level.time > ent->pre_solid_time)
                {
                        if (CheckBox (ent))
                        {
                                ent->pre_solid = 0;
                                // Dirty
                                //ent->solid = SOLID_BBOX;
                                ent->svflags &= ~SVF_DEADMONSTER;
                                // Dirty
                        }
                }
        }

        // Dirty - For open doors stuff
        if (ent->client->pers.grabbing == -1)
                ent->client->pers.grabbing = 0;

        if ((ent->spotted != ent) && ent->spotted->deadflag)
                ent->spotted = ent;

        if ((ent->spotted_time < level.time) && (ent->spotted != ent))
                ent->spotted = ent;
        // Dirty

        if (ent->corpse && (!VectorCompare (ent->corpse_pos, ent->s.origin)) && (ent->delay_time < level.time))
        {
                ent->client->pers.grabbing = 0;
                if (ent->client->showinventory)
                        ent->client->showinventory = false;
                ent->corpse = NULL;
        }

// GRIM

	// if the scoreboard is up, update it
	if (ent->client->showscores && !(level.framenum & 31) )
	{
                // GRIM
		if (ent->client->menu)
			PMenu_Update(ent);
                else if (ent->client->showhelp)
                        GoalDescription (ent);
                else
			DeathmatchScoreboardMessage (ent, ent->enemy);
                // GRIM
                gi.unicast (ent, false);
	}
}
