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

static	edict_ref	current_player;
static	gclient_t	*current_client;

static vec3_t	v_forward, v_right, v_up;
static vec_t	xyspeed;
static gtime_t	bobmove;
static int32_t	bobcycle;		// odd cycles are right foot going forward
static vec_t	bobfracsin;		// sin(bobfrac*M_PI)

/*
===============
SV_CalcRoll

===============
*/
static vec_t SV_CalcRoll (const vec3_t &angles, const vec3_t &velocity)
{
	vec_t side = abs(velocity.Dot(v_right));
	
	const vec_t value = sv_rollangle->value;

	if (side < sv_rollspeed->value)
		side = side * value / sv_rollspeed->value;
	else
		side = value;
	
	return side;
}

/*
===============
P_DamageFeedback

Handles color blends and view kicks
===============
*/
static void P_DamageFeedback ()
{
	constexpr vec3_t bcolor { 1.0, 0.0, 0.0 };

	// flash the backgrounds behind the status numbers
	current_client->ps.stats[STAT_FLASHES] = 0;
	if (current_client->damage_blood)
		current_client->ps.stats[STAT_FLASHES] |= 1;

	// total points of damage shot at the player this frame
	int32_t count = current_client->damage_blood;
	
	if (count == 0)
		return;		// didn't take any damage

	// start a pain animation if still in the player model
	if (current_client->anim_priority < ANIM_PAIN && current_player->s.modelindex == MODEL_PLAYER)
	{
		constexpr struct {
			int32_t	start;
			int32_t	end;
		} pain_anims[] = {
			{ FRAME_pain101, FRAME_pain104 },
			{ FRAME_pain201, FRAME_pain204 },
			{ FRAME_pain301, FRAME_pain304 }
		};

		current_client->anim_priority = ANIM_PAIN;
		if (current_client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			current_player->s.frame = FRAME_crpain1-1;
			current_client->anim_end = FRAME_crpain4;
		}
		else
		{
			auto &anim = pain_anims[irandom(lengthof(pain_anims) - 1)];
			current_player->s.frame = anim.start - 1;
			current_client->anim_end = anim.end;
		}
	}

	int32_t realcount = count;

	if (count < 10)
		count = 10;	// always make a visible effect

	// play an apropriate pain sound
	if ((level.time > current_player->pain_debounce_time) && !(current_player->flags & FL_GODMODE))
	{
		const int32_t r = irandom(1, 2);
		current_player->pain_debounce_time = level.time + 700;

		int32_t l = 100;

		if (current_player->health < 25)
			l = 25;
		else if (current_player->health < 50)
			l = 50;
		else if (current_player->health < 75)
			l = 75;

		current_player->PlaySound(gi.soundindex(va("*pain%i_%i.wav", l, r)), CHAN_VOICE);
	}

	// the color of the blend will vary based on how much was absorbed
	// by different armors
	vec3_t v = vec3_origin;

	if (current_client->damage_blood)
		v += bcolor * (static_cast<vec_t>(current_client->damage_blood) / realcount);

	for (int32_t i = 0; i < 3; i++)
		current_client->damage_blend[i] = v[i];

	// the total alpha of the blend is always proportional to count
	// don't go too saturated
	current_client->damage_blend[3] = clamp(max(current_client->damage_blend[3], 0.f) + (count * 0.01f), 0.2f, 0.6f);

	//
	// calculate view angle kicks
	//
	if (current_client->damage_knockback && current_player->health > 0)	// kick of 0 means no view adjust at all
	{
		const vec_t kick = clamp(current_client->damage_knockback * 100.f / current_player->health, count * 0.5f, 50.0f);

		v = current_client->damage_from - current_player->s.origin;
		v.Normalize();

		vec_t side = v.Dot(v_right);
		current_client->v_dmg_roll = kick*side*0.3f;
		
		side = -v.Dot(v_forward);
		current_client->v_dmg_pitch = kick*side*0.3f;

		current_client->v_dmg_time = level.time + DAMAGE_TIME;
	}

	//
	// clear totals
	//
	current_client->damage_blood = 0;
	current_client->damage_knockback = 0;
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
static void SV_CalcViewOffset ()
{
	// base angles
	vec3_t &angles = current_client->ps.kick_angles;

	// if dead, fix the angle and don't add any kick
	if (current_player->deadflag)
	{
		angles.Clear();
		current_client->ps.viewangles = { -15.0f, current_client->killer_yaw, 40.0f };
	}
	else
	{
		// add angles based on weapon kick
		angles = current_client->kick_angles;

		// add angles based on damage kick
		if (current_client->v_dmg_time > level.time)
		{
			const vec_t ratio = static_cast<vec_t>(current_client->v_dmg_time - level.time) / DAMAGE_TIME;
			angles[PITCH] += ratio * current_client->v_dmg_pitch;
			angles[ROLL] += ratio * current_client->v_dmg_roll;
		}
		else
		{
			current_client->v_dmg_pitch = 0;
			current_client->v_dmg_roll = 0;
		}

		// add pitch based on fall kick

		if (current_client->fall_time > level.time)
		{
			const vec_t ratio = static_cast<vec_t>(current_client->fall_time - level.time) / FALL_TIME;
			angles[PITCH] += ratio * current_client->fall_value;
		}

		// add angles based on velocity

		vec_t delta = current_player->velocity.Dot(v_forward);
		angles[PITCH] += delta*run_pitch->value;
		
		delta = current_player->velocity.Dot(v_right);
		angles[ROLL] += delta*run_roll->value;

		// add angles based on bob

		delta = bobfracsin * bob_pitch->value * xyspeed;
		if (current_client->ps.pmove.pm_flags & PMF_DUCKED)
			delta *= 6;		// crouching
		angles[PITCH] += delta;
		delta = bobfracsin * bob_roll->value * xyspeed;
		if (current_client->ps.pmove.pm_flags & PMF_DUCKED)
			delta *= 6;		// crouching
		if (bobcycle & 1)
			delta = -delta;
		angles[ROLL] += delta;
	}

//===================================

	// base origin
	// add view height

	vec3_t v = { 0.f, 0.f, current_player->viewheight };

	// add fall height
	if (current_client->fall_time > level.time)
	{
		const vec_t ratio = static_cast<vec_t>(current_client->fall_time - level.time) / FALL_TIME;
		v[2] -= ratio * current_client->fall_value * 0.4f;
	}

	// add bob height

	const vec_t bob = min(bobfracsin * xyspeed * bob_up->value, 6.f);

	v[2] += bob;

	// add kick offset

	v += current_client->kick_origin;

	// absolutely bound offsets
	// so the view can never be outside the player box
	v[0] = clamp(v[0], -14.f, 14.f);
	v[1] = clamp(v[1], -14.f, 14.f);
	v[2] = clamp(v[2], -22.f, 30.f);

	current_client->ps.viewoffset = v;
}

/*
==============
SV_CalcGunOffset
==============
*/
static void SV_CalcGunOffset ()
{
	// gun angles from bobbing
	current_client->ps.gunangles[ROLL] = xyspeed * bobfracsin * 0.005f;
	current_client->ps.gunangles[YAW] = xyspeed * bobfracsin * 0.01f;

	if (bobcycle & 1)
	{
		current_client->ps.gunangles[ROLL] = -current_client->ps.gunangles[ROLL];
		current_client->ps.gunangles[YAW] = -current_client->ps.gunangles[YAW];
	}

	current_client->ps.gunangles[PITCH] = xyspeed * bobfracsin * 0.005f;

	// gun angles from delta movement
	for (int32_t i = 0; i < 3; i++)
	{
		vec_t delta = current_client->oldviewangles[i] - current_client->ps.viewangles[i];

		if (delta > 180)
			delta -= 360;
		if (delta < -180)
			delta += 360;

		delta = clamp(delta, -45.f, 45.f);

		if (i == YAW)
			current_client->ps.gunangles[ROLL] += 0.1f * delta;

		current_client->ps.gunangles[i] += 0.2f * delta;
	}

	// gun height
	current_client->ps.gunoffset.Clear();

	// gun_x / gun_y / gun_z are development tools
	current_client->ps.gunoffset += (v_forward * gun_y->value) + (v_right * gun_x->value) + (v_up * -gun_z->value);
}


/*
=============
SV_AddBlend
=============
*/
inline void SV_AddBlend (const vec4_t &src, vec4_t &dst)
{
	if (src[3] <= 0)
		return;

	// new total alpha
	const vec_t a2 = dst[3] + (1 - dst[3]) * src[3];
	// fraction of color from old
	const vec_t a3 = dst[3] / a2;

	for (int32_t i = 0; i < 3; i++)
		dst[i] = dst[i] * a3 + src[i] * (1 - a3);
	dst[3] = src[3];
}

constexpr vec4_t lava_blend = { 1.0f, 0.3f, 0.0f, 0.6f };
constexpr vec4_t slime_blend = { 0.0f, 0.1f, 0.05f, 0.6f };
constexpr vec4_t water_blend = { 0.5f, 0.3f, 0.2f, 0.4f };

/*
=============
SV_CalcBlend
=============
*/
static void SV_CalcBlend ()
{
	current_client->ps.blend.Clear();

	// add for contents
	const vec3_t vieworg = current_player->s.origin + current_client->ps.viewoffset;
	const brushcontents_t contents = gi.pointcontents (vieworg);
	if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER))
		current_client->ps.rdflags |= RDF_UNDERWATER;
	else
		current_client->ps.rdflags &= ~RDF_UNDERWATER;

	if (contents & (CONTENTS_SOLID|CONTENTS_LAVA))
		SV_AddBlend (lava_blend, current_client->ps.blend);
	else if (contents & CONTENTS_SLIME)
		SV_AddBlend (slime_blend, current_client->ps.blend);
	else if (contents & CONTENTS_WATER)
		SV_AddBlend (water_blend, current_client->ps.blend);

	// add for damage
	SV_AddBlend (current_client->damage_blend, current_client->ps.blend);

	// drop the damage value
	current_client->damage_blend[3] = max(0.0f, current_client->damage_blend[3] - 0.06f);
}


/*
=================
P_FallingDamage
=================
*/
static void P_FallingDamage ()
{
	if (current_player->s.modelindex != MODEL_PLAYER)
		return;		// not in the player model

	if (current_player->movetype == MOVETYPE_NOCLIP)
		return;

	vec_t delta;

	if ((current_client->oldvelocity[2] < 0) && (current_player->velocity[2] > current_client->oldvelocity[2]) && !current_player->groundentity)
		delta = current_client->oldvelocity[2];
	else if (!current_player->groundentity)
		return;
	else
		delta = current_player->velocity[2] - current_client->oldvelocity[2];

	delta = delta * delta * 0.0001f;

	// never take falling damage if completely underwater
	if (current_player->waterlevel == WATER_UNDER)
		return;
	if (current_player->waterlevel == WATER_WAIST)
		delta *= 0.25f;
	if (current_player->waterlevel == WATER_FEET)
		delta *= 0.5f;

	if (delta < 1)
		return;

	if (delta < 15)
	{
		current_player->s.event = EV_FOOTSTEP;
		return;
	}

	current_client->fall_value = min(40.0f, delta * 0.5f);
	current_client->fall_time = level.time + FALL_TIME;

	if (delta > 30)
	{
		if (current_player->health > 0)
		{
			if (delta >= 55)
				current_player->s.event = EV_FALLFAR;
			else
				current_player->s.event = EV_FALL;
		}

		current_player->pain_debounce_time = level.time;	// no normal pain sound
		const int32_t damage = max(1, static_cast<int32_t>((delta - 30) / 2));

		if (!dmflags.no_falling_damage)
			T_Damage (current_player, game.world(), game.world(), { 0.f, 0.f, 1.f }, current_player->s.origin, vec3_origin, damage, 0, DAMAGE_NONE);

		return;
	}

	current_player->s.event = EV_FALLSHORT;
}



/*
=============
P_WorldEffects
=============
*/
static void P_WorldEffects ()
{
	if (current_player->movetype == MOVETYPE_NOCLIP)
	{
		current_player->air_finished = level.time + 12000;	// don't need air
		return;
	}

	const waterlevel_t waterlevel = current_player->waterlevel;
	const waterlevel_t old_waterlevel = current_client->old_waterlevel;
	current_client->old_waterlevel = waterlevel;

	//
	// if just entered a water volume, play a sound
	//
	if (!old_waterlevel && waterlevel)
	{
		if (current_player->watertype & CONTENTS_LAVA)
			current_player->PlaySound(gi.soundindex("player/lava_in.wav"), CHAN_BODY);
		else if (current_player->watertype & (CONTENTS_SLIME | CONTENTS_WATER))
			current_player->PlaySound(gi.soundindex("player/watr_in.wav"), CHAN_BODY);
		current_player->flags |= FL_INWATER;

		// clear damage_debounce, so the pain sound will play immediately
		current_player->damage_debounce_time = 0;
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if (old_waterlevel && ! waterlevel)
	{
		current_player->PlaySound(gi.soundindex("player/watr_out.wav"), CHAN_BODY);
		current_player->flags &= ~FL_INWATER;
	}

	//
	// check for head just going under water
	//
	if (old_waterlevel != WATER_UNDER && waterlevel == WATER_UNDER)
		current_player->PlaySound(gi.soundindex("player/watr_un.wav"), CHAN_BODY);

	//
	// check for head just coming out of water
	//
	if (old_waterlevel == WATER_UNDER && waterlevel != WATER_UNDER)
	{
		if (current_player->air_finished < level.time)
			// gasp for air
			current_player->PlaySound(gi.soundindex("player/gasp1.wav"), CHAN_VOICE);
		else  if (current_player->air_finished < level.time + 11000)
			// just break surface
			current_player->PlaySound(gi.soundindex("player/gasp2.wav"), CHAN_VOICE);
	}

	//
	// check for drowning
	//
	if (waterlevel == 3)
	{
		// if out of air, start drowning
		if (current_player->air_finished < level.time)
		{	// drown!
			if (current_client->next_drown_time < level.time && current_player->health > 0)
			{
				current_client->next_drown_time = level.time + 1000;

				// take more damage the longer underwater
				current_player->dmg += 2;
				if (current_player->dmg > 15)
					current_player->dmg = 15;

				// play a gurp sound instead of a normal pain sound
				if (current_player->health <= current_player->dmg)
					current_player->PlaySound(gi.soundindex("player/drown1.wav"), CHAN_VOICE);
				else if (prandom(50))
					current_player->PlaySound(gi.soundindex("*gurp1.wav"), CHAN_VOICE);
				else
					current_player->PlaySound(gi.soundindex("*gurp2.wav"), CHAN_VOICE);

				current_player->pain_debounce_time = level.time;

				T_Damage (current_player, game.world(), game.world(), vec3_origin, current_player->s.origin, vec3_origin, current_player->dmg, 0, DAMAGE_NO_ARMOR);
			}
		}
	}
	else
	{
		current_player->air_finished = level.time + 12000;
		current_player->dmg = 2;
	}

	//
	// check for sizzle damage
	//
	if (waterlevel && (current_player->watertype & (CONTENTS_LAVA | CONTENTS_SLIME)) )
	{
		if (current_player->watertype & CONTENTS_LAVA)
		{
			if (current_player->health > 0
				&& current_player->pain_debounce_time <= level.time)
			{
				if (prandom(50))
					current_player->PlaySound(gi.soundindex("player/burn1.wav"), CHAN_VOICE);
				else
					current_player->PlaySound(gi.soundindex("player/burn2.wav"), CHAN_VOICE);
				current_player->pain_debounce_time = level.time + 1000;
			}

			T_Damage (current_player, game.world(), game.world(), vec3_origin, current_player->s.origin, vec3_origin, 3*waterlevel, 0, DAMAGE_NONE);
		}

		if (current_player->watertype & CONTENTS_SLIME)
			T_Damage (current_player, game.world(), game.world(), vec3_origin, current_player->s.origin, vec3_origin, 1*waterlevel, 0, DAMAGE_NONE);
	}
}


/*
===============
G_SetClientEffects
===============
*/
static void G_SetClientEffects ()
{
	current_player->s.effects = EF_NONE;
	current_player->s.renderfx = RF_NONE;

	if (current_player->health <= 0 || level.intermissiontime)
		return;

	// show cheaters!!!
	if (current_player->flags & FL_GODMODE)
	{
		current_player->s.effects |= EF_COLOR_SHELL;
		current_player->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
	}
}


/*
===============
G_SetClientEvent
===============
*/
static void G_SetClientEvent ()
{
	if (current_player->s.event)
		return;

	if (current_player->groundentity && xyspeed > 225)
	{
		if (static_cast<int32_t>((current_client->bobtime + bobmove) / 1000.f) != bobcycle)
			current_player->s.event = EV_FOOTSTEP;
	}
}

/*
===============
G_SetClientSound
===============
*/
static void G_SetClientSound ()
{
	if (current_player->waterlevel && (current_player->watertype & (CONTENTS_LAVA | CONTENTS_SLIME)) )
		current_player->s.sound = snd_fry;
	else if (current_client->weapon_sound)
		current_player->s.sound = current_client->weapon_sound;
	else
		current_player->s.sound = SOUND_NONE;
}

/*
===============
G_SetClientFrame
===============
*/
static void G_SetClientFrame ()
{
	if (current_player->s.modelindex != MODEL_PLAYER)
		return;		// not in the player model

	const bool duck = current_client->ps.pmove.pm_flags & PMF_DUCKED;
	const bool run = !!xyspeed;

	// check for stand/duck and stop/go transitions
	if (duck != current_client->anim_duck && current_client->anim_priority < ANIM_DEATH)
		goto newanim;
	if (run != current_client->anim_run && current_client->anim_priority == ANIM_BASIC)
		goto newanim;
	if (!current_player->groundentity && current_client->anim_priority <= ANIM_WAVE)
		goto newanim;

	if(current_client->anim_priority == ANIM_REVERSE)
	{
		if(current_player->s.frame > current_client->anim_end)
		{
			current_player->s.frame--;
			return;
		}
	}
	else if (current_player->s.frame < current_client->anim_end)
	{	// continue an animation
		current_player->s.frame++;
		return;
	}

	if (current_client->anim_priority == ANIM_DEATH)
		return;		// stay there
	if (current_client->anim_priority == ANIM_JUMP)
	{
		if (!current_player->groundentity)
			return;		// stay there
		current_client->anim_priority = ANIM_WAVE;
		current_player->s.frame = FRAME_jump3;
		current_client->anim_end = FRAME_jump6;
		return;
	}

newanim:
	// return to either a running or standing frame
	current_client->anim_priority = ANIM_BASIC;
	current_client->anim_duck = duck;
	current_client->anim_run = run;

	if (!current_player->groundentity)
	{
		current_client->anim_priority = ANIM_JUMP;
		if (current_player->s.frame != FRAME_jump2)
			current_player->s.frame = FRAME_jump1;
		current_client->anim_end = FRAME_jump2;
	}
	else if (run)
	{	// running
		if (duck)
		{
			current_player->s.frame = FRAME_crwalk1;
			current_client->anim_end = FRAME_crwalk6;
		}
		else
		{
			current_player->s.frame = FRAME_run1;
			current_client->anim_end = FRAME_run6;
		}
	}
	else if (duck)
	{
		current_player->s.frame = FRAME_crstnd01;
		current_client->anim_end = FRAME_crstnd19;
	}
	else
	{
		current_player->s.frame = FRAME_stand01;
		current_client->anim_end = FRAME_stand40;
	}
}


/*
=================
ClientEndServerFrame

Called for each player at the end of the server frame
and right after spawning
=================
*/
void ClientEndServerFrame (edict_t &ent)
{
	current_player = ent;
	current_client = ent.client;

	//
	// If the origin or velocity have changed since ClientThink(),
	// update the pmove values.  This will happen when the client
	// is pushed by a bmodel or kicked by an explosion.
	// 
	// If it wasn't updated here, the view position would lag a frame
	// behind the body position when pushed -- "sinking into plats"
	//
	for (int32_t i = 0; i < 3; i++)
	{
		current_client->ps.pmove.origin[i] = ent.s.origin[i]*8.0f;
		current_client->ps.pmove.velocity[i] = ent.velocity[i]*8.0f;
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

	current_client->v_angle.AngleVectors(&v_forward, &v_right, &v_up);

	// burn from lava, etc
	P_WorldEffects ();

	if (ent.control)
	{
		ent.s.sound = SOUND_NONE;
		current_client->ps.blend.Clear();
		current_client->ps.blend[3] = 0.0;
		current_client->ps.kick_angles.Clear();
		current_client->ps.viewoffset.Clear();
		current_client->ps.viewangles.Clear();
		G_SetStats (ent);
	}
	else
	{
		//
		// set model angles from view angles so other things in
		// the world can tell which direction you are looking
		//
		if (current_client->v_angle[PITCH] > 180)
			ent.s.angles[PITCH] = (-360 + current_client->v_angle[PITCH])/3;
		else
			ent.s.angles[PITCH] = current_client->v_angle[PITCH]/3;
		ent.s.angles[YAW] = current_client->v_angle[YAW];
		ent.s.angles[ROLL] = 0;
		ent.s.angles[ROLL] = SV_CalcRoll (ent.s.angles, ent.velocity)*4;

		//
		// calculate speed and cycle to be used for
		// all cyclic walking effects
		//
		xyspeed = sqrt(ent.velocity[0]*ent.velocity[0] + ent.velocity[1]*ent.velocity[1]);

		if (xyspeed < 5)
		{
			bobmove = 0;
			current_client->bobtime = 0;	// start at beginning of cycle again
		}
		else if (ent.groundentity)
		{
			// so bobbing only cycles when on ground
			if (xyspeed > 210)
				bobmove = 250;
			else if (xyspeed > 100)
				bobmove = 125;
			else
				bobmove = 62;
		}
	
		gtime_t bobtime = (current_client->bobtime += bobmove);

		if (current_client->ps.pmove.pm_flags & PMF_DUCKED)
			bobtime *= 4;

		bobcycle = static_cast<int32_t>(bobtime / 1000.f);
		bobfracsin = abs(sin((bobtime / 1000.f) * M_PI));

		// detect hitting the floor
		P_FallingDamage ();

		// apply all the damage taken this frame
		P_DamageFeedback ();

		// determine the view offsets
		SV_CalcViewOffset ();

		// determine the gun offsets
		SV_CalcGunOffset ();

		// determine the full screen color blend
		// must be after viewoffset, so eye contents can be
		// accurately determined
		// FIXME: with client prediction, the contents
		// should be determined by the client
		SV_CalcBlend ();

		// chase cam stuff
		if (current_client->resp.spectator)
			G_SetSpectatorStats(ent);
		else
			G_SetStats (ent);

		G_CheckChaseStats(ent);

		G_SetClientEvent ();

		G_SetClientEffects ();

		G_SetClientSound ();

		G_SetClientFrame ();
	}

	current_client->oldvelocity = ent.velocity;
	current_client->oldviewangles = current_client->ps.viewangles;

	// clear weapon kicks
	current_client->kick_origin.Clear();
	current_client->kick_angles.Clear();

	// if the scoreboard is up, update it
	if (current_client->showscores && !(level.framenum & 31) )
	{
		DeathmatchScoreboardMessage (ent, ent.enemy);
		ent.Unicast(false);
	}
}

