/*****************************************************************

	Eraser Bot source code - by Ryan Feltrin, Added to by Acrid-

	..............................................................

	This file is Copyright(c) 1998, Ryan Feltrin, All Rights Reserved.

	..............................................................

	All other files are Copyright(c) Id Software, Inc.

	Please see liscense.txt in the source directory for the copyright
	information regarding those files belonging to Id Software, Inc.

	..............................................................
	
	Should you decide to release a modified version of Eraser, you MUST
	include the following text (minus the BEGIN and END lines) in the 
	documentation for your modification.

	--- BEGIN ---

	The Eraser Bot is a product of Ryan Feltrin, and is available from
	the Eraser Bot homepage, at http://impact.frag.com.

	This program is a modification of the Eraser Bot, and is therefore
	in NO WAY supported by Ryan Feltrin.

	This program MUST NOT be sold in ANY form. If you have paid for 
	this product, you should contact Ryan Feltrin immediately, via
	the Eraser Bot homepage.

	--- END ---

	..............................................................

	You will find p_trail.c has not been included with the Eraser
	source code release. This is NOT an error. I am unable to 
	distribute this file because it contains code that is bound by
	legal documents, and signed by myself, never to be released
	to the public. Sorry guys, but law is law.

	I have therefore include the compiled version of these files
	in .obj form in the src\Release and src\Debug directories.
	So while you cannot edit and debug code within these files,
	you can still compile this source as-is. Although these will only
	work in MSVC v5.0, linux versions can be made available upon
	request.

	NOTE: When compiling this source, you will get a warning
	message from the compiler, regarding the missing p_trail.c
	file. Just ignore it, it will still compile fine.

	..............................................................

	I, Ryan Feltrin/Acrid-, hold no responsibility for any harm caused by the
	use of this source code. I also am NOT willing to provide any form
	of help or support for this source code. It is provided as-is,
	as a service by me, with no documentation, other then the comments
	contained within the code. If you have any queries, I suggest you
	visit the "official" Eraser source web-board, at
	http://www.telefragged.com/epidemic/. I will stop by there from
	time to time, to answer questions and help with any problems that
	may arise.

	Otherwise, have fun, and I look forward to seeing what can be done
	with this.

	-Ryan Feltrin
	-Acrid-

 *****************************************************************/
#include "g_local.h"
#include "bot_procs.h"
#include "m_player.h"
#include "g_items.h"

#include <sys/timeb.h>
 
int	aborted_fire;
void fire_stab (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int mod);
void fire_stinger (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_pellet_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_napalmrocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_clusterrocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_pulse (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod);
void fire_needle(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread);
void fire_flamethrower(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);
void fire_telsa(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread);
void fire_armordart (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect);
void fire_poisondart (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect);
void fire_infecteddart (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect);
void fire_lightning (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int kick);
void fire_shc (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread);
//int	is_quad;

void	bot_FireWeapon(edict_t	*self)
{
	if (!self->enemy)
		return;

	if (!self->bot_client)	//Gregg added
		return;


	//no firing while botfrozen 3/99
	if (level.time < self->frozentime)
		return;

//ACRID ADDED
			if (K2_IsProtected(self))
			self->client->protecttime = 0;
//END
	// set Quad flag
	is_quad = (self->client->quad_framenum > level.framenum);

	aborted_fire = false;
	self->bot_fire(self);

	if (!aborted_fire)
	{
		if (!CTFApplyStrengthSound(self))
		if (is_quad)
			gi.sound(self, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

		self->last_fire = level.time;

		if (self->maxs[2] == 4)
			self->s.frame = FRAME_crattak1;
		else
			self->s.frame = FRAME_attack1;

		if (self->bot_fire != botBlaster)
			self->last_reached_trail = level.time;
	}
	// check for no ammo ON CURRENT WEAPON//WORKS
	if ((self->client->pers.weapon != item_blaster) && (!self->client->pers.inventory[self->client->ammo_index]))
	{
		botPickBestWeapon(self);
	botDebugPrint("BOT No Ammo (ACRID)\n");
	}

	// check for using knife while carring flag
	if ((self->client->pers.weapon == item_knife) && 
		(CarryingFlag(self)))
	{
		botPickBestWeapon(self);
	botDebugPrint("BOT change knife for other (ACRID)\n");
	}

/*ACRID CHANGE WEAPONS IF IN ITEMS LIST AND HAVE AMMO AND FIRING BLASTER.
  */
//		if ( client->pers.inventory[ITEM_INDEX(item_bullets)]
//			&&  client->pers.inventory[ITEM_INDEX(item_needler)] &&
//			(self->client->pers.inventory[self->client->ammo_index] <=50)
if ((self->bot_fire == botBlaster) &&
	(self->client->pers.inventory[ITEM_INDEX(item_shells)]  != 0))
	{
   if ((self->client->pers.inventory[ITEM_INDEX(item_supershotgun)] != 0) ||
			(self->client->pers.inventory[ITEM_INDEX(item_shotgun)] != 0) ||
			(self->client->pers.inventory[ITEM_INDEX(item_shc)]     != 0) ||
			(self->client->pers.inventory[ITEM_INDEX(item_poisondart)] != 0) ||
			(self->client->pers.inventory[ITEM_INDEX(item_knife)] != 0) ||
			(self->client->pers.inventory[ITEM_INDEX(item_infecteddart)] != 0))
		{
		botPickBestWeapon(self);
//        botDebugPrint("BOT new fire (SHELLS ACRID)\n");
		}
}
else if ((self->bot_fire == botNeedler) &&
	(self->client->pers.inventory[ITEM_INDEX(item_bullets)]  >= 50))
	{
		botPickBestWeapon(self);
//        botDebugPrint("BOT new needler switch (NEEDLER ACRID)\n");
		}
  
else if ((self->bot_fire == botBlaster) &&
	(self->client->pers.inventory[ITEM_INDEX(item_slugs)]  != 0))
	{
   if ((self->client->pers.inventory[ITEM_INDEX(item_sniperrifle)] != 0) ||
			(self->client->pers.inventory[ITEM_INDEX(item_railgun)] != 0) ||
			(self->client->pers.inventory[ITEM_INDEX(item_lightninggun)] != 0))
		{
		botPickBestWeapon(self);
//        botDebugPrint("BOT new fire (SLUGS ACRID)\n");
		}
}
else if ((self->bot_fire == botBlaster) &&
	(self->client->pers.inventory[ITEM_INDEX(item_bullets)]  != 0))
	{
   if ((self->client->pers.inventory[ITEM_INDEX(item_machinegun)] != 0) ||
	   (self->client->pers.inventory[ITEM_INDEX(item_pulsecannon)] != 0) ||
	   (self->client->pers.inventory[ITEM_INDEX(item_needler)] != 0) ||
	   (self->client->pers.inventory[ITEM_INDEX(item_ak47)] != 0) ||
	   (self->client->pers.inventory[ITEM_INDEX(item_pistol)] != 0) ||
			(self->client->pers.inventory[ITEM_INDEX(item_chaingun)] != 0))
		{
		botPickBestWeapon(self);
 //       botDebugPrint("BOT new fire (BULLETS ACRID)\n");
		}
}
else if ((self->bot_fire == botBlaster) &&
	(self->client->pers.inventory[ITEM_INDEX(item_cells)]  != 0))
	{
   if ((self->client->pers.inventory[ITEM_INDEX(item_hyperblaster)] != 0) ||
	      (self->client->pers.inventory[ITEM_INDEX(item_telsacoil)] != 0) ||
		  (self->client->pers.inventory[ITEM_INDEX(item_flamethrower)] != 0) ||	
		  (self->client->pers.inventory[ITEM_INDEX(item_bfg10k)] != 0))
		{
		botPickBestWeapon(self);
 //       botDebugPrint("BOT new fire (CELLS ACRID)\n");
		}
}
else if ((self->bot_fire == botBlaster) &&
	(self->client->pers.inventory[ITEM_INDEX(item_rockets)]  != 0))
	{
   if ((self->client->pers.inventory[ITEM_INDEX(item_rocketlauncher)] != 0) ||
       (self->client->pers.inventory[ITEM_INDEX(item_rocketnapalmlauncher)] != 0) ||
	   (self->client->pers.inventory[ITEM_INDEX(item_pelletrocketlauncher)] != 0) ||
	   (self->client->pers.inventory[ITEM_INDEX(item_stingerrocketlauncher)] != 0) ||	   
	   (self->client->pers.inventory[ITEM_INDEX(item_rocketclusterlauncher)] != 0))
		{
		botPickBestWeapon(self);
 //       botDebugPrint("BOT new fire (ROCKETS ACRID)\n");
		}
}
else if ((self->bot_fire == botBlaster) &&
	(self->client->pers.inventory[ITEM_INDEX(item_grenades)]  != 0))
	{
   if ((self->client->pers.inventory[ITEM_INDEX(item_grenadelauncher)] != 0) ||
	   (self->client->pers.inventory[ITEM_INDEX(item_handgrenades)] != 0) ||
			(self->client->pers.inventory[ITEM_INDEX(item_grenadelauncher)] != 0))
		{
		botPickBestWeapon(self);
//        botDebugPrint("BOT new fire (GRENADES ACRID)\n");
		}
}	
//ACRID END FIX CHANGE WEAPON ON AMMO PICK UP
	if ((self->bot_fire == botBlaster) && (self->enemy) && !CarryingFlag(self->enemy))
	{	// abort attacking enemy?
		gclient_t	*client;

		client = self->enemy->client;

		if ((	(self->health < 20) &&
				(self->enemy->health > 15) &&
				((self->enemy->health > self->health) || client->pers.weapon != item_blaster /*ClientHasAnyWeapon(client)*/)))
		{	// abort the attack
			self->enemy = NULL;
		}
	}
}


void	bot_Attack(edict_t *self)
{
	float	strafe_speed=BOT_STRAFE_SPEED;

	if (!self->enemy || (self->health <= 0))// || (self->enemy->health <= 0))
		return;//sentry co fixme

	if (!self->bot_client)	//Gregg added
		return;

	// see if the enemy is visible
	if (	(self->last_enemy_sight > (level.time - 0.2))
		||	(	(visible(self, self->enemy) && gi.inPVS(self->s.origin, self->enemy->s.origin))
			 &&	(self->last_enemy_sight = level.time)))
	{
		trace_t	trace;

		// make sure we don't hit a friend
		if (!((int)(dmflags->value) & DF_NO_FRIENDLY_FIRE) && (ctf->value || self->client->team))
		{
			trace = gi.trace (self->s.origin, vec3_origin, vec3_origin, self->enemy->s.origin, self, MASK_PLAYERSOLID);
	
/*if ((self->client->player_class == 2) &&
	trace.ent && trace.ent->client && SameTeam(trace.ent, self) && 
	(trace.ent->disease) && 
      (self->client->pers.inventory[ITEM_INDEX(item_shells)]  != 0))
{botDebugPrint("trtrace ent im a nurse");}

else*/ if (trace.ent && (trace.ent->client) && SameTeam(trace.ent,self))
                 //nurse
			{	// we might hit a good guy!
				return;
			}
		}

		// BFG delayed firing
		if ((self->bot_fire == botBFG) && (self->last_fire > level.time) && (self->last_fire <= (level.time+0.1)))
		{
			gi.sound(self, CHAN_WEAPON, gi.soundindex("weapons/bfg__f1y.wav"), 1, 2, 0);
			self->last_fire = level.time - FIRE_INTERVAL_BFG + 0.5;
		}

		if (	((self->last_fire + self->fire_interval) < level.time)
			&&	(self->sight_enemy_time < (level.time - (SIGHT_FIRE_DELAY * ((5 - self->bot_stats->combat*0.5)/5)))))
		{
			bot_FireWeapon(self);

			if (CTFApplyHaste(self))
				CTFApplyHasteSound(self);
		}
        // can't strafe if bad Combat skills
		if (self->bot_stats->combat == 1)	
			strafe_speed = 0;

		if (self->enemy != self->last_movegoal)
		{	// only strafe slowly, so we don't go too far off course
			strafe_speed = 0;
		}
		else if (self->maxs[2] == 4)
		{
			strafe_speed *= 0.5;
		}

		// do attack movements, like strafing
		if ((strafe_speed > 0) && self->enemy && self->groundentity && (self->strafe_changedir_time < level.time) && (self->bot_stats->combat > 1))
		{
			self->strafe_dir = !self->strafe_dir;
			self->strafe_changedir_time = level.time + 0.5 + random() * 1.5;

			// check for ducking or jumping
			if (self->crouch_attack_time < level.time)
			{
				float	rnd, dist;

				dist = entdist(self, self->enemy);
				rnd = random()*4;

				// if low combat, then skip jumping for a bit
				if (self->bot_stats->combat < rnd)
				{
					self->crouch_attack_time = level.time + 1;
					goto nojump;
				}
                // crouch if far away
				if ((self->maxs[2] > 4) && (dist > 400) && (rnd < 3))	
				{
					if (self->bot_stats->combat > 4)
					{
						self->crouch_attack_time = level.time + random()*0.5 + 0.5;
						self->maxs[2] = 4;
					}
				}
				else if (	(dist < 700)
						 &&	(	(self->last_seek_enemy < level.time)
							 ||	(entdist(self->last_movegoal, self) > 256))
						 &&	(CanJump(self)))	// jump
				{
					vec3_t	right, dest, mins;
					trace_t	trace;
                    // if combat = 2, jump less frequently 
					if ((self->bot_stats->combat >= 3) || (random() < 0.3))	
					{
						vec3_t	rvec;

						AngleVectors(self->s.angles, NULL, right, NULL);
						VectorCopy(right, rvec);
						VectorScale(right, ((self->strafe_dir * 2) - 1), right);
						VectorScale(right, BOT_STRAFE_SPEED, right);

						// check that the jump will be safe
//						VectorAdd(self->mins, tv(0,0,12), mins);
						VectorAdd(self->s.origin, right, dest);
						trace = gi.trace(self->s.origin, mins, self->maxs, dest, self, MASK_SOLID);
						VectorSubtract(trace.endpos, rvec, trace.endpos);
						VectorAdd(trace.endpos, tv(0,0,-256), dest);
						trace = gi.trace(trace.endpos, VEC_ORIGIN, VEC_ORIGIN, dest, self, MASK_SOLID | MASK_WATER);
//lavacode
						if ((trace.fraction < 1) && !(trace.contents & (CONTENTS_LAVA | CONTENTS_SLIME)))
						{
							VectorCopy(right, self->velocity);

							self->velocity[2] = 300;
							self->groundentity = NULL;
//							self->s.origin[2] += 1;

							gi.linkentity(self);

							VectorCopy(self->velocity, self->jump_velocity);

							gi.sound(self, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, 2, 0);
						}
						else	// jump straight up
						{
							VectorClear(self->velocity);

							self->velocity[2] = 300;
							self->groundentity = NULL;
//							self->s.origin[2] += 1;

							gi.linkentity(self);

							VectorCopy(self->velocity, self->jump_velocity);

							gi.sound(self, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, 2, 0);
						}
					}
				}

				if (	(rnd >= 1) && (self->maxs[2] == 4)
					&&	(!self->goalentity || (self->goalentity->maxs[2] > 4))
					&&	(CanStand(self)))
				{	// resume standing
					self->maxs[2] = 32;
				}
			}
		}

nojump:

		if (self->groundentity && (strafe_speed > 0))
		{
			if (!M_walkmove(self, 
							self->s.angles[YAW] + (90 * ((self->strafe_dir * 2) - 1)),
							strafe_speed * bot_frametime ))
			{
				self->strafe_dir = !self->strafe_dir;
				self->strafe_changedir_time = level.time + 0.5 + (random() * 0.5);
			}
		}


	}

	else	// once we sight them again, don't fire instantaneously (super-human powers)
	{
		self->sight_enemy_time = level.time;
//FIXME ACRID SENTRY
		// abort chasing a RL welding human, with enough health//Acrid sec
		if (!self->enemy->bot_client && (self->enemy->client) &&
(self->enemy->client->pers.weapon == item_rocketlauncher) && 
!CarryingFlag(self->enemy)
&& ((self->enemy->health > 25)))//3/99 || (self->bot_fire == botBlaster || self->bot_fire == botShotgun)))
		{	// abort the attack
			// move away
			if (self->goalentity)
			{
				self->goalentity->ignore_time = level.time + 1;
				self->goalentity = NULL;
			}

			if (self->enemy->client->pers.weapon == item_rocketlauncher)
				self->enemy->ignore_time = level.time + 2;

			self->enemy = NULL;
		}
	}
}
/*****************************************
 * Bot firing code                       *
 * uses g_spawn g_local.h                *
 * bot_procs.h and bot_misc.c            *
 *****************************************/
void botBlaster (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	float	dist, tf;
	int	damage;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);
	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	dist = entdist(self, self->enemy);

	if (self->enemy && infront(self, self->enemy))
	{

		if ((self->enemy->health > 0) && (skill->value > 1) && (self->enemy->client) && (dist > 64))
		{
			VectorMA (self->enemy->s.origin, dist * (1/1000), self->enemy->velocity, target);
			target[2] += self->enemy->viewheight - 8;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= 0.5 + (VectorLength(self->enemy->velocity)/600);
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	damage = 15;
	if (is_quad)
		damage *= 4;

	monster_fire_blaster (self, start, forward, damage, 1000, MZ_BLASTER, EF_BLASTER);

}

//knife
void botKnife (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int		damage = DAMAGE_KNIFE;
	int		kick = 4;

botDebugPrint("knife\n");
	if ((int)(level.time*10) & 1)	// only calculate every other frame
	{
		AngleVectors (self->s.angles, forward, right, NULL);
		G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

		VectorScale(forward, 8, ofs);
	    self->client->kick_angles[0] = -2;
		VectorAdd(self->s.origin, ofs, start);
		start[2] += self->viewheight - 8;


		if (self->enemy && infront(self, self->enemy))
		{
			float	dist, tf;

			dist = entdist(self, self->enemy);

			if (self->enemy->health > 0)
			{
				VectorCopy (self->enemy->s.origin, target);

				if (skill->value <= 1)
				{	// trail the player's velocity
					VectorMA(target, -0.2, self->enemy->velocity, target);
				}

				target[2] += self->enemy->viewheight - 8;
			}
			else
			{
				VectorCopy (self->enemy->s.origin, target);
				target[2] += self->enemy->viewheight - 8;
			}

			if (self->bot_stats->accuracy < 5)
			{//sentry ->client
				tf = (dist < 256) ? dist/2 : 256;
				tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
				if (self->enemy->client && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));
				VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.1), target);
			}

			VectorSubtract (target, start, forward);
			VectorNormalize (forward);

			vectoangles(forward, self->s.angles);
			// don't go more than 15 degrees up or down
			if (abs(self->s.angles[PITCH]) > 15)	
				self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
		}
		else
		{
			aborted_fire = true;
			return;
		}

		VectorCopy(forward, self->last_forward);
		VectorCopy(start, self->last_start);
	}
	else
	{
		VectorCopy(self->last_forward, forward);
		VectorCopy(self->last_start, start);
	}

	if (is_quad)
	{
		damage *= 4;
		kick *=4;
	}
	fire_stab (self, start, forward, damage, kick, MOD_KNIFE);
    self->client->pers.inventory[self->client->ammo_index]--;
}
void botMachineGun (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int	damage;

	if ((int)(level.time*10) & 1)	// only calculate every other frame
	{
		AngleVectors (self->s.angles, forward, right, NULL);
		G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

		VectorScale(forward, 8, ofs);
		VectorAdd(self->s.origin, ofs, start);
		start[2] += self->viewheight - 8;

		if (self->enemy && infront(self, self->enemy))
		{
			float	dist, tf;

			dist = entdist(self, self->enemy);

			if (self->enemy->health > 0)
			{
				VectorCopy (self->enemy->s.origin, target);

				if (skill->value <= 1)
				{	// trail the player's velocity
					VectorMA(target, -0.2, self->enemy->velocity, target);
				}

				target[2] += self->enemy->viewheight - 8;
			}
			else
			{
				VectorCopy (self->enemy->s.origin, target);
				target[2] += self->enemy->viewheight - 8;
			}

			if (self->bot_stats->accuracy < 5)
			{//sentry ->client
				tf = (dist < 256) ? dist/2 : 256;
				tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
				if (self->enemy->client && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));
				VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.1), target);
			}

			VectorSubtract (target, start, forward);
			VectorNormalize (forward);

			vectoangles(forward, self->s.angles);
			if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
				self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
		}
		else
		{
			aborted_fire = true;
			return;
		}

		VectorCopy(forward, self->last_forward);
		VectorCopy(start, self->last_start);
	}
	else
	{
		VectorCopy(self->last_forward, forward);
		VectorCopy(self->last_start, start);
	}

	damage = 3;
	if (is_quad)
		damage *= 4;

	monster_fire_bullet (self, start, forward, damage, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MZ2_ACTOR_MACHINEGUN_1, MOD_MACHINEGUN);

	self->client->pers.inventory[self->client->ammo_index]--;

/*
#ifdef	_WIN32
	_ftime(&self->lastattack_time);
#else
	ftime(&self->lastattack_time);
#endif
*/
}
void botAk47 (edict_t *self)
{
	int	i;
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int	    damage = 13;
	int	    kick = 3;

	if ((int)(level.time*10) & 1)	// only calculate every other frame
	{
		AngleVectors (self->s.angles, forward, right, NULL);
		G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

		VectorScale(forward, 8, ofs);
		VectorAdd(self->s.origin, ofs, start);
		start[2] += self->viewheight - 8;

		if (self->enemy && infront(self, self->enemy))
		{
			float	dist, tf;

			dist = entdist(self, self->enemy);

			if (self->enemy->health > 0)
			{
				VectorCopy (self->enemy->s.origin, target);

				if (skill->value <= 1)
				{	// trail the player's velocity
					VectorMA(target, -0.2, self->enemy->velocity, target);
				}

				target[2] += self->enemy->viewheight - 8;
			}
			else
			{
				VectorCopy (self->enemy->s.origin, target);
				target[2] += self->enemy->viewheight - 8;
			}

			if (self->bot_stats->accuracy < 5)
			{//sentry ->client
				tf = (dist < 256) ? dist/2 : 256;
				tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
				if (self->enemy->client && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));
				VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.1), target);
			}

			VectorSubtract (target, start, forward);
			VectorNormalize (forward);

			vectoangles(forward, self->s.angles);
			if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
				self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
		}
		else
		{
			aborted_fire = true;
			return;
		}

		VectorCopy(forward, self->last_forward);
		VectorCopy(start, self->last_start);
	}
	else
	{
		VectorCopy(self->last_forward, forward);
		VectorCopy(self->last_start, start);
	}
	for (i=1 ; i<3 ; i++)
	{
		self->client->kick_origin[i] = crandom() * 0.35;
		self->client->kick_angles[i] = crandom() * 0.7;
	}
	self->client->kick_origin[0] = crandom() * 0.35;
	self->client->kick_angles[0] = self->client->machinegun_shots * -1.5;
	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	fire_bullet (self, start, forward, damage, kick, 200, 300, MOD_AK47);

	self->client->pers.inventory[self->client->ammo_index]--;
//send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}
void botPistol (edict_t *self)//WF34 FIXME
{
	int	i;
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int	    damage = 35;
	int	    kick = 2;

	if ((int)(level.time*10) & 1)	// only calculate every other frame
	{
		AngleVectors (self->s.angles, forward, right, NULL);
		G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

		VectorScale(forward, 8, ofs);
		VectorAdd(self->s.origin, ofs, start);
		start[2] += self->viewheight - 8;

		if (self->enemy && infront(self, self->enemy))
		{
			float	dist, tf;

			dist = entdist(self, self->enemy);

			if (self->enemy->health > 0)
			{
				VectorCopy (self->enemy->s.origin, target);

				if (skill->value <= 1)
				{	// trail the player's velocity
					VectorMA(target, -0.2, self->enemy->velocity, target);
				}

				target[2] += self->enemy->viewheight - 8;
			}
			else
			{
				VectorCopy (self->enemy->s.origin, target);
				target[2] += self->enemy->viewheight - 8;
			}

			if (self->bot_stats->accuracy < 5)
			{//sentry ->client
				tf = (dist < 256) ? dist/2 : 256;
				tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
				if (self->enemy->client && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));
				VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.1), target);
			}

			VectorSubtract (target, start, forward);
			VectorNormalize (forward);

			vectoangles(forward, self->s.angles);
			if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
				self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
		}
		else
		{
			aborted_fire = true;
			return;
		}

		VectorCopy(forward, self->last_forward);
		VectorCopy(start, self->last_start);
	}
	else
	{
		VectorCopy(self->last_forward, forward);
		VectorCopy(self->last_start, start);
	}
	for (i=1 ; i<3 ; i++)
	{
		self->client->kick_origin[i] = crandom() * 0.35;
		self->client->kick_angles[i] = crandom() * 0.7;
	}
	self->client->kick_origin[0] = crandom() * 0.35;
	self->client->kick_angles[0] = self->client->machinegun_shots * -1.5;
	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}
gi.sound(self, CHAN_AUTO, gi.soundindex("weapons/pistol.wav"), 1, ATTN_NORM, 0);//FIXME ACRID
	fire_bullet (self, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD / 2, DEFAULT_BULLET_VSPREAD / 2, MOD_PISTOL);

	self->client->pers.inventory[self->client->ammo_index]--;
}
void botShotgun (edict_t *self)//WF34 FIXME
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int	damage;
	float	dist=0, tf;
botDebugPrint("shotgun\n");
	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{

		dist = entdist(self, self->enemy);

		if (self->enemy->health > 0)
		{
			VectorCopy (self->enemy->s.origin, target);

			if (skill->value <= 1)
			{	// trail the player's velocity
				VectorMA(target, -0.2, self->enemy->velocity, target);
			}

			target[2] += self->enemy->viewheight - 8;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	damage = 3;
	if (is_quad)
		damage *= 4;

	monster_fire_shotgun (self, start, forward, damage, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MZ_SHOTGUN, MOD_SHOTGUN);
	self->client->pers.inventory[self->client->ammo_index]--;

	if (dist > 600)
	{	// check for a better long distance weapon

		botPickBestFarWeapon(self);
	}
}

void botSuperShotgun (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	vec3_t	angles;
	int damage;
	float	dist=0, tf;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{

		dist = entdist(self, self->enemy);

		if (self->enemy->health > 0)
		{
			VectorCopy (self->enemy->s.origin, target);

			if (skill->value <= 1)
			{	// trail the player's velocity
				VectorMA(target, -0.2, self->enemy->velocity, target);
			}

			target[2] += self->enemy->viewheight - 8;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	damage = 3*2;	// OPTIMIZE: increase damage, decrease number of bullets
	if (is_quad)
		damage *= 4;

	vectoangles(forward, angles);

	angles[YAW] += 5;
	AngleVectors(angles, forward, NULL, NULL);
	monster_fire_shotgun (self, start, forward, damage, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, DEFAULT_SSHOTGUN_COUNT/4, MZ_SSHOTGUN, MOD_SSHOTGUN);

	angles[YAW] -= 10;
	AngleVectors(angles, forward, NULL, NULL);
	monster_fire_shotgun (self, start, forward, damage, 4, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, DEFAULT_SSHOTGUN_COUNT/4, MZ_SSHOTGUN | MZ_SILENCED, MOD_SSHOTGUN);

	self->client->pers.inventory[self->client->ammo_index] -= 2;
	if (self->client->pers.inventory[self->client->ammo_index] < 0)
		self->client->pers.inventory[self->client->ammo_index] = 0;

	if (dist > 600)
	{	// check for a better long distance weapon

		botPickBestFarWeapon(self);
	}
}

void botChaingun (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int		shots, damage, kick;

	if ((int)(level.time*10) & 1)	// only calculate every other frame
	{
		AngleVectors (self->s.angles, forward, right, NULL);
		G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

		VectorScale(forward, 8, ofs);
		VectorAdd(self->s.origin, ofs, start);
		start[2] += self->viewheight - 8;

		if (self->enemy && infront(self, self->enemy))
		{
			float	dist, tf;

			dist = entdist(self, self->enemy);

			if (self->enemy->health > 0)
			{
				VectorCopy (self->enemy->s.origin, target);

				if (skill->value <= 1)
				{	// trail the player's velocity
					VectorMA(target, -0.2, self->enemy->velocity, target);
				}

				target[2] += self->enemy->viewheight - 8;
			}
			else
			{
				VectorCopy (self->enemy->s.origin, target);
				target[2] += self->enemy->viewheight - 8;
			}

			if (self->bot_stats->accuracy < 5)
			{//sentry ->client
				tf = (dist < 256) ? dist/2 : 256;
				tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
				if (self->enemy->client && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));

				if (tf > 0)
					VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
			}

			VectorSubtract (target, start, forward);
			VectorNormalize (forward);

			vectoangles(forward, self->s.angles);
			if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
				self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
		}
		else
		{
			aborted_fire = true;
			return;
		}

		VectorCopy(forward, self->last_forward);
		VectorCopy(start, self->last_start);
	}
	else
	{
		VectorCopy(self->last_forward, forward);
		VectorCopy(self->last_start, start);
	}

	if (self->client->killer_yaw < (level.time - 0.3))
	{
		if (self->client->killer_yaw < (level.time - 0.5))
		{	// must have stopped firing, so need to restart wind-up
			self->client->machinegun_shots = 0;
		}

		self->client->killer_yaw = level.time;
		self->client->machinegun_shots++;
	}

	shots = self->client->machinegun_shots;
	if (shots > 3)
		shots = self->client->machinegun_shots = 3;

	// optimize, simulate more shots by increasing the damage, but still only firing one shot
	// chaingun is responsible for a LOT of cpu usage
	damage = 3 * shots;
	kick = 2;

	if (is_quad)
		damage *= 4;

//	for (i=0 ; i<shots ; i++)
//	{
		// get start / end positions
		fire_bullet (self, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD * 0.5, DEFAULT_BULLET_VSPREAD * 0.5, MOD_CHAINGUN);
//	}

	if ((self->client->pers.inventory[self->client->ammo_index] -= shots) < 0)
	{
		self->client->pers.inventory[self->client->ammo_index] = 0;
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1));
	gi.multicast (self->s.origin, MULTICAST_PVS);

/*
#ifdef	_WIN32
	_ftime(&self->lastattack_time);
#else
	ftime(&self->lastattack_time);
#endif
*/
}

void botRailgun (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int		damage, kick;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{
		float	dist, tf=0;

		dist = entdist(self, self->enemy);

		if (self->enemy->health > 0)
		{
			VectorCopy (self->enemy->s.origin, target);

//			if (skill->value <= 1)
			{	// trail the player's velocity
				VectorMA(target, -0.2, self->enemy->velocity, target);
			}

			target[2] += self->enemy->viewheight - 8;
//sentry ->client
			if (self->enemy->client && !self->enemy->bot_client)
				tf = (VectorLength(self->enemy->velocity) / 300) * 100;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		tf = 32;

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf += (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
		}

		if (tf > 0)
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.1), target);

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	damage	= 100;
	kick	= 200;

	if (is_quad)
		damage *= 4;

	fire_rail (self, start, forward, damage, kick, MOD_RAILGUN);//WF ADDED false
	self->client->pers.inventory[self->client->ammo_index]--;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_RAILGUN);
	gi.multicast (self->s.origin, MULTICAST_PVS);

}

void botRocketLauncher (edict_t *self)
{
	vec3_t	start, target, end_trace;
	vec3_t	forward, right, ofs;
	vec3_t	oldorg, vel;
	int		damage;
	float	damage_radius;
	int		radius_damage;
	float	dist=0, tf;
	trace_t	trace;

	// fire at peak of jump
	if ((self->bot_stats->combat > 3) && !self->groundentity && (self->velocity[2] > 50))
	{
		aborted_fire = true;
		return;
	}

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{
		dist = entdist(self, self->enemy);
                                                               //sentry ->client              
		if ((skill->value > 1) && (self->enemy->health > 0) && (self->enemy->client && !self->enemy->bot_client) && (dist > 64))
		{
			VectorCopy(self->enemy->velocity, vel);
			if (vel[2] > 0)
				vel[2] = 0;

			VectorMA (self->enemy->s.origin, (float) dist / 650, vel, target);
			target[2] += self->enemy->viewheight - 8;

			if (self->bot_stats->combat > 3)
			{	// aim towards the ground?
				trace_t tr;

				VectorCopy(target, end_trace);
				end_trace[2] -= 64;
				tr = gi.trace(target, NULL, NULL, end_trace, self->enemy, CONTENTS_SOLID);

				if (tr.fraction < 1)
				{
					vec3_t end, org;

					VectorCopy(tr.endpos, end);

					VectorCopy(self->s.origin, org);
					org[2] += self->viewheight;

					tr = gi.trace(org, NULL, NULL, end, self, CONTENTS_SOLID);

					if (tr.fraction == 1)
					{
						VectorCopy(end, target);
					}
				}
			}

		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	// check to make sure the rocket won't explode in our face
//sentry ->client
	if (!self->enemy->bot_client)// && self->enemy->client)//3/99 reversed
	{
		// move the enemy to the predicted position
		VectorCopy(self->enemy->s.origin, oldorg);
		VectorMA (self->enemy->s.origin, (float) dist / 650, self->enemy->velocity, self->enemy->s.origin);
		gi.linkentity(self->enemy);
	}

	VectorScale(forward, 130, end_trace);
	VectorAdd(start, end_trace, end_trace);
	trace = gi.trace(start, tv(-12,-12,-4), tv(12,12,4), end_trace, self, MASK_PLAYERSOLID);
//sentry ->client
	if (!self->enemy->bot_client)// && self->enemy->client)//3/99 reversed
	{
		// move the enemy back to their correct position
		VectorCopy(oldorg, self->enemy->s.origin);
		gi.linkentity(self->enemy);
	}

	if (	(trace.fraction < 1)
		&&	(	(self->health > 15)
			 ||	(!trace.ent)))		// dangerous situation, only fire if almost dead and obstacle is another player
	{
		// walk backwards
		if (!M_walkmove(self, self->s.angles[YAW] + 180, BOT_RUN_SPEED * bot_frametime))
		{
			// FIXME: can't walk backwards, select a good close-range weapon

			botPickBestCloseWeapon(self);
		}

		aborted_fire = true;
		return;
	}

	if (is_quad)
		damage *= 4;

	fire_rocket (self, start, forward, damage, 650, damage_radius, radius_damage, MOD_ROCKET);//WF ADDED
	self->client->pers.inventory[self->client->ammo_index]--;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if (dist > 700)
	{	// check for a better long distance weapon

		botPickBestFarWeapon(self);
	}
}
void botStingerRocketLauncher (edict_t *self)
{
	vec3_t	start, target, end_trace;
	vec3_t	forward, right, ofs;
	vec3_t	oldorg, vel;
	int		damage;
	float	damage_radius;
	int		radius_damage;
	float	dist=0, tf;
	trace_t	trace;

	// fire at peak of jump
	if ((self->bot_stats->combat > 3) && !self->groundentity && (self->velocity[2] > 50))
	{
		aborted_fire = true;
		return;
	}

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{
		dist = entdist(self, self->enemy);
                                                               //sentry ->client              
		if ((skill->value > 1) && (self->enemy->health > 0) && (self->enemy->client && !self->enemy->bot_client) && (dist > 64))
		{
			VectorCopy(self->enemy->velocity, vel);
			if (vel[2] > 0)
				vel[2] = 0;

			VectorMA (self->enemy->s.origin, (float) dist / 650, vel, target);
			target[2] += self->enemy->viewheight - 8;

			if (self->bot_stats->combat > 3)
			{	// aim towards the ground?
				trace_t tr;

				VectorCopy(target, end_trace);
				end_trace[2] -= 64;
				tr = gi.trace(target, NULL, NULL, end_trace, self->enemy, CONTENTS_SOLID);

				if (tr.fraction < 1)
				{
					vec3_t end, org;

					VectorCopy(tr.endpos, end);

					VectorCopy(self->s.origin, org);
					org[2] += self->viewheight;

					tr = gi.trace(org, NULL, NULL, end, self, CONTENTS_SOLID);

					if (tr.fraction == 1)
					{
						VectorCopy(end, target);
					}
				}
			}

		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	// check to make sure the rocket won't explode in our face
//sentry ->client
	if (self->enemy->client && !self->enemy->bot_client)
	{
		// move the enemy to the predicted position
		VectorCopy(self->enemy->s.origin, oldorg);
		VectorMA (self->enemy->s.origin, (float) dist / 650, self->enemy->velocity, self->enemy->s.origin);
		gi.linkentity(self->enemy);
	}

	VectorScale(forward, 130, end_trace);
	VectorAdd(start, end_trace, end_trace);
	trace = gi.trace(start, tv(-12,-12,-4), tv(12,12,4), end_trace, self, MASK_PLAYERSOLID);
//sentry ->client
	if (self->enemy->client && !self->enemy->bot_client)
	{
		// move the enemy back to their correct position
		VectorCopy(oldorg, self->enemy->s.origin);
		gi.linkentity(self->enemy);
	}

	if (	(trace.fraction < 1)
		&&	(	(self->health > 15)
			 ||	(!trace.ent)))		// dangerous situation, only fire if almost dead and obstacle is another player
	{
		// walk backwards
		if (!M_walkmove(self, self->s.angles[YAW] + 180, BOT_RUN_SPEED * bot_frametime))
		{
			// FIXME: can't walk backwards, select a good close-range weapon

			botPickBestCloseWeapon(self);
		}

		aborted_fire = true;
		return;
	}

	if (is_quad)
		damage *= 4;

	fire_stinger (self, start, forward, damage, 800, damage_radius, radius_damage);
	self->client->pers.inventory[self->client->ammo_index]--;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if (dist > 700)
	{	// check for a better long distance weapon

		botPickBestFarWeapon(self);
	}
}
void botPelletRocketLauncher (edict_t *self)
{
	vec3_t	start, target, end_trace;
	vec3_t	forward, right, ofs;
	vec3_t	oldorg, vel;
	int		damage;
	float	damage_radius;
	int		radius_damage;
	float	dist=0, tf;
	trace_t	trace;

	// fire at peak of jump
	if ((self->bot_stats->combat > 3) && !self->groundentity && (self->velocity[2] > 50))
	{
		aborted_fire = true;
		return;
	}

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{
		dist = entdist(self, self->enemy);
                                                                //sentry ->client
		if ((skill->value > 1) && (self->enemy->health > 0) && (self->enemy->client && !self->enemy->bot_client) && (dist > 64))
		{
			VectorCopy(self->enemy->velocity, vel);
			if (vel[2] > 0)
				vel[2] = 0;

			VectorMA (self->enemy->s.origin, (float) dist / 650, vel, target);
			target[2] += self->enemy->viewheight - 8;

			if (self->bot_stats->combat > 3)
			{	// aim towards the ground?
				trace_t tr;

				VectorCopy(target, end_trace);
				end_trace[2] -= 64;
				tr = gi.trace(target, NULL, NULL, end_trace, self->enemy, CONTENTS_SOLID);

				if (tr.fraction < 1)
				{
					vec3_t end, org;

					VectorCopy(tr.endpos, end);

					VectorCopy(self->s.origin, org);
					org[2] += self->viewheight;

					tr = gi.trace(org, NULL, NULL, end, self, CONTENTS_SOLID);

					if (tr.fraction == 1)
					{
						VectorCopy(end, target);
					}
				}
			}

		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	// check to make sure the rocket won't explode in our face
//sentry ->client
	if (self->enemy->client && !self->enemy->bot_client)
	{
		// move the enemy to the predicted position
		VectorCopy(self->enemy->s.origin, oldorg);
		VectorMA (self->enemy->s.origin, (float) dist / 650, self->enemy->velocity, self->enemy->s.origin);
		gi.linkentity(self->enemy);
	}

	VectorScale(forward, 130, end_trace);
	VectorAdd(start, end_trace, end_trace);
	trace = gi.trace(start, tv(-12,-12,-4), tv(12,12,4), end_trace, self, MASK_PLAYERSOLID);
//sentry ->client
	if (self->enemy->client && !self->enemy->bot_client)
	{
		// move the enemy back to their correct position
		VectorCopy(oldorg, self->enemy->s.origin);
		gi.linkentity(self->enemy);
	}

	if (	(trace.fraction < 1)
		&&	(	(self->health > 15)
			 ||	(!trace.ent)))		// dangerous situation, only fire if almost dead and obstacle is another player
	{
		// walk backwards
		if (!M_walkmove(self, self->s.angles[YAW] + 180, BOT_RUN_SPEED * bot_frametime))
		{
			// FIXME: can't walk backwards, select a good close-range weapon

			botPickBestCloseWeapon(self);
		}

		aborted_fire = true;
		return;
	}

	if (is_quad)
		damage *= 4;

	fire_pellet_rocket (self, start, forward, damage, 650, damage_radius, radius_damage);//WF ADDED
	self->client->pers.inventory[self->client->ammo_index]--;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if (dist > 600)//FIXME > FAR
	{	// check for a better long distance weapon

		botPickBestFarWeapon(self);
	}

}
void botRocketNapalmLauncher (edict_t *self)
{
	vec3_t	start, target, end_trace;
	vec3_t	forward, right, ofs;
	vec3_t	oldorg, vel;
	int		damage;
	float	damage_radius;
	int		radius_damage;
	float	dist=0, tf;
	trace_t	trace;

	// fire at peak of jump
	if ((self->bot_stats->combat > 3) && !self->groundentity && (self->velocity[2] > 50))
	{
		aborted_fire = true;
		return;
	}

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{
		dist = entdist(self, self->enemy);
                                                                //sentry ->client                
		if ((skill->value > 1) && (self->enemy->health > 0) && (self->enemy->client && !self->enemy->bot_client) && (dist > 64))
		{
			VectorCopy(self->enemy->velocity, vel);
			if (vel[2] > 0)
				vel[2] = 0;

			VectorMA (self->enemy->s.origin, (float) dist / 650, vel, target);
			target[2] += self->enemy->viewheight - 8;

			if (self->bot_stats->combat > 3)
			{	// aim towards the ground?
				trace_t tr;

				VectorCopy(target, end_trace);
				end_trace[2] -= 64;
				tr = gi.trace(target, NULL, NULL, end_trace, self->enemy, CONTENTS_SOLID);

				if (tr.fraction < 1)
				{
					vec3_t end, org;

					VectorCopy(tr.endpos, end);

					VectorCopy(self->s.origin, org);
					org[2] += self->viewheight;

					tr = gi.trace(org, NULL, NULL, end, self, CONTENTS_SOLID);

					if (tr.fraction == 1)
					{
						VectorCopy(end, target);
					}
				}
			}

		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	// check to make sure the rocket won't explode in our face
//sentry ->client
	if (self->enemy->client && !self->enemy->bot_client)
	{
		// move the enemy to the predicted position
		VectorCopy(self->enemy->s.origin, oldorg);
		VectorMA (self->enemy->s.origin, (float) dist / 650, self->enemy->velocity, self->enemy->s.origin);
		gi.linkentity(self->enemy);
	}

	VectorScale(forward, 130, end_trace);
	VectorAdd(start, end_trace, end_trace);
	trace = gi.trace(start, tv(-12,-12,-4), tv(12,12,4), end_trace, self, MASK_PLAYERSOLID);
//sentry ->client
	if (self->enemy->client && !self->enemy->bot_client)
	{
		// move the enemy back to their correct position
		VectorCopy(oldorg, self->enemy->s.origin);
		gi.linkentity(self->enemy);
	}

	if (	(trace.fraction < 1)
		&&	(	(self->health > 15)
			 ||	(!trace.ent)))		// dangerous situation, only fire if almost dead and obstacle is another player
	{
		// walk backwards
		if (!M_walkmove(self, self->s.angles[YAW] + 180, BOT_RUN_SPEED * bot_frametime))
		{
			// FIXME: can't walk backwards, select a good close-range weapon

			botPickBestCloseWeapon(self);
		}

		aborted_fire = true;
		return;
	}

	if (is_quad)
		damage *= 4;

	fire_napalmrocket (self, start, forward, damage, 650, damage_radius, radius_damage);//WF ADDED
	self->client->pers.inventory[self->client->ammo_index]--;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if (dist > 700)//FIXME > FAR 700
	{	// check for a better long distance weapon

		botPickBestFarWeapon(self);
	}

}
void botRocketClusterLauncher (edict_t *self)
{
	vec3_t	start, target, end_trace;
	vec3_t	forward, right, ofs;
	vec3_t	oldorg, vel;
	int		damage;
	float	damage_radius;
	int		radius_damage;
	float	dist=0, tf;
	trace_t	trace;

	// fire at peak of jump
	if ((self->bot_stats->combat > 3) && !self->groundentity && (self->velocity[2] > 50))
	{
		aborted_fire = true;
		return;
	}

	damage = 100 + (int)(random() * 20.0);
	radius_damage = 120;
	damage_radius = 120;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{
		dist = entdist(self, self->enemy);
                                                                 //sentry ->client
		if ((skill->value > 1) && (self->enemy->health > 0) && (self->enemy->client && !self->enemy->bot_client) && (dist > 64))
		{
			VectorCopy(self->enemy->velocity, vel);
			if (vel[2] > 0)
				vel[2] = 0;

			VectorMA (self->enemy->s.origin, (float) dist / 650, vel, target);
			target[2] += self->enemy->viewheight - 8;

			if (self->bot_stats->combat > 3)
			{	// aim towards the ground?
				trace_t tr;

				VectorCopy(target, end_trace);
				end_trace[2] -= 64;
				tr = gi.trace(target, NULL, NULL, end_trace, self->enemy, CONTENTS_SOLID);

				if (tr.fraction < 1)
				{
					vec3_t end, org;

					VectorCopy(tr.endpos, end);

					VectorCopy(self->s.origin, org);
					org[2] += self->viewheight;

					tr = gi.trace(org, NULL, NULL, end, self, CONTENTS_SOLID);

					if (tr.fraction == 1)
					{
						VectorCopy(end, target);
					}
				}
			}

		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	// check to make sure the rocket won't explode in our face
//sentry ->client
	if (self->enemy->client && !self->enemy->bot_client)
	{
		// move the enemy to the predicted position
		VectorCopy(self->enemy->s.origin, oldorg);
		VectorMA (self->enemy->s.origin, (float) dist / 650, self->enemy->velocity, self->enemy->s.origin);
		gi.linkentity(self->enemy);
	}

	VectorScale(forward, 130, end_trace);
	VectorAdd(start, end_trace, end_trace);
	trace = gi.trace(start, tv(-12,-12,-4), tv(12,12,4), end_trace, self, MASK_PLAYERSOLID);
//sentry ->client
	if (self->enemy->client && !self->enemy->bot_client)
	{
		// move the enemy back to their correct position
		VectorCopy(oldorg, self->enemy->s.origin);
		gi.linkentity(self->enemy);
	}

	if (	(trace.fraction < 1)
		&&	(	(self->health > 15)
			 ||	(!trace.ent)))		// dangerous situation, only fire if almost dead and obstacle is another player
	{
		// walk backwards
		if (!M_walkmove(self, self->s.angles[YAW] + 180, BOT_RUN_SPEED * bot_frametime))
		{
			// FIXME: can't walk backwards, select a good close-range weapon

			botPickBestCloseWeapon(self);
		}

		aborted_fire = true;
		return;
	}

	if (is_quad)
		damage *= 4;

	fire_clusterrocket (self, start, forward, damage, 650, damage_radius, radius_damage);//WF ADDED
	self->client->pers.inventory[self->client->ammo_index]--;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if (dist < 600)//FIXME FAR > 700
	{	// check for a better long distance weapon

		botPickBestCloseWeapon(self);
	}

}
void botGrenadeLauncher (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs, angles;
	int		damage;
	int		radius;
	float	dist=0, tf;

	damage = 120;
	radius = damage + 40;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	dist = entdist(self, self->enemy);

	if (self->enemy && infront(self, self->enemy))
	{//sentry ->client
		if ((self->enemy->health > 0) && (self->enemy->client && !self->enemy->bot_client) && (dist > 64))
		{
			VectorMA (self->enemy->s.origin, dist / 550, self->enemy->velocity, target);
			target[2] += self->enemy->viewheight - 8;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	if (is_quad)
		damage *= 4;

	vectoangles(forward, angles);

	// angle upwards a bit
	angles[PITCH] -= 15 * ((dist < 384) ? ((dist / 384) * 2) - 1: 1);
	AngleVectors(angles, forward, NULL, NULL);

	fire_grenade (self, start, forward, damage, 600, 2.5, radius);
	self->client->pers.inventory[self->client->ammo_index]--;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_GRENADE);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if (dist > 700)
	{	// check for a better long distance weapon

		botPickBestFarWeapon(self);
	}
	else if (dist < radius)
	{
		botPickBestCloseWeapon(self);
	}
}
#define GRENADE_TIMER		3.0
#define GRENADE_MINSPEED	400
#define GRENADE_MAXSPEED	800

void botGrenades (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs, angles;
	int		damage;
	int		radius;
	float	dist=0, tf;
//    qboolean held;
	int		lbdamage = 15; //reduce damage
	float	timer;
	int		speed;
	damage = 125;
	radius = damage + 40;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	dist = entdist(self, self->enemy);//fixme????
	timer = self->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);

	if (self->enemy && infront(self, self->enemy))
	{//sentry ->client
		if ((self->enemy->health > 0) && (self->enemy->client && !self->enemy->bot_client) && (dist > 64))
		{
			VectorMA (self->enemy->s.origin, dist / 550, self->enemy->velocity, target);
			target[2] += self->enemy->viewheight - 8;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	if (is_quad)
		damage *= 4;

	vectoangles(forward, angles);

	// angle upwards a bit
	angles[PITCH] -= 15 * ((dist < 384) ? ((dist / 384) * 2) - 1: 1);
	AngleVectors(angles, forward, NULL, NULL);
			
	fire_grenade2 (self, start, forward, damage, 600, 2.5, radius, false);//fixme held
	self->client->pers.inventory[self->client->ammo_index]--;

	if (dist > 700)
	{	// check for a better long distance weapon

		botPickBestFarWeapon(self);
	}
	else if (dist < radius)
	{
		botPickBestCloseWeapon(self);
	}
	self->client->grenade_time = level.time + 1.0;
}//TEST ACRID FIXME GRENADES
void botHyperblaster (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	float	dist=0, tf;
	int		damage, effect;

	damage = 15;
	if (is_quad)
		damage *= 4;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{
		dist = entdist(self, self->enemy);
                                                               //sentry ->client
		if ((skill->value > 1) && (self->enemy->health > 0) && (self->enemy->client) && (dist > 64))
		{
			VectorMA (self->enemy->s.origin, dist/1000, self->enemy->velocity, target);
			target[2] += self->enemy->viewheight - 8;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (VectorLength(self->enemy->velocity)/600);
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}


	if ((random() * 3) < 1)
		effect = EF_HYPERBLASTER;
	else
		effect = 0;

	fire_blaster (self, start, forward, damage, 1000, effect, true);
	self->client->pers.inventory[self->client->ammo_index]--;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_HYPERBLASTER);
	gi.multicast (self->s.origin, MULTICAST_PVS);

}

void botBFG (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int		damage;
	float	damage_radius;
	float	dist=0, tf;

	damage = 500;
	damage_radius = 1000;
	if (is_quad)
		damage *= 4;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	dist = entdist(self, self->enemy);

	if (self->enemy && infront(self, self->enemy))
	{
//sentry ->client
		if ((self->enemy->health > 0) && (self->enemy->client && !self->enemy->bot_client) && (dist > 64))
		{
			VectorMA (self->enemy->s.origin, entdist(self, self->enemy) * (1/550), self->enemy->velocity, target);
			target[2] += self->enemy->viewheight - 8;

			if ((dist > 200) && self->enemy->groundentity)	// aim towards ground
				target[2] -= (4 * self->bot_stats->combat);
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	fire_bfg (self, start, forward, damage, 400, damage_radius);
	if ((self->client->pers.inventory[self->client->ammo_index] -= 60) < 0)
		self->client->pers.inventory[self->client->ammo_index] = 0;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_BFG);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if (dist > 1000)
	{	// check for a better long distance weapon
		botPickBestFarWeapon(self);
	}
}
//ACRID START WF WEAPONS
void botPulseCannon (edict_t *self)
{
//fixme	int     i;
	vec3_t	start, target;
	vec3_t	forward, right, ofs;// , up;
//fixme	float   r, u;
	int		shots, damage, kick = 8;

	if (deathmatch->value)
		damage = 3;
	else
		damage = 6;

	if ((int)(level.time*10) & 1)	// only calculate every other frame
	{
		AngleVectors (self->s.angles, forward, right, NULL);

		G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

		VectorScale(forward, 8, ofs);
		VectorAdd(self->s.origin, ofs, start);
		start[2] += self->viewheight - 8;

		if (self->enemy && infront(self, self->enemy))
		{
			float	dist, tf;

			dist = entdist(self, self->enemy);

			if (self->enemy->health > 0)
			{
				VectorCopy (self->enemy->s.origin, target);

				if (skill->value <= 1)
				{	// trail the player's velocity
					VectorMA(target, -0.2, self->enemy->velocity, target);
				}

				target[2] += self->enemy->viewheight - 8;
			}
			else
			{
				VectorCopy (self->enemy->s.origin, target);
				target[2] += self->enemy->viewheight - 8;
			}

			if (self->bot_stats->accuracy < 5)
			{//sentry ->client
				tf = (dist < 256) ? dist/2 : 256;
				tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
				if (self->enemy->client && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));

				if (tf > 0)
					VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
			}

			VectorSubtract (target, start, forward);
			VectorNormalize (forward);

			vectoangles(forward, self->s.angles);
			if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
				self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
		}
		else
		{
			aborted_fire = true;
			return;
		}

		VectorCopy(forward, self->last_forward);
		VectorCopy(start, self->last_start);
	}
	else
	{
		VectorCopy(self->last_forward, forward);
		VectorCopy(self->last_start, start);
	}

	if (self->client->killer_yaw < (level.time - 0.3))
	{
		if (self->client->killer_yaw < (level.time - 0.5))
		{	// must have stopped firing, so need to restart wind-up
			self->client->machinegun_shots = 0;
		}

		self->client->killer_yaw = level.time;
		self->client->machinegun_shots++;
	}

	shots = self->client->machinegun_shots;
	if (shots > 3)
		shots = self->client->machinegun_shots = 3;

	// optimize, simulate more shots by increasing the damage, but still only firing one shot
	// chaingun is responsible for a LOT of cpu usage
	damage = 3 * shots;
	kick = 2;

	if (is_quad)
		damage *= 4;

		self->ShotNumber++;
		if (self->client->ping >500)
		{
			if (self->ShotNumber>3)
			{
				fire_pulse (self, start, forward, damage*4, kick*4, 0, 250, 250, MOD_MBPC);
				self->ShotNumber =0;
			}
		}
		else 
		{
			if (self->ShotNumber>1)
			{
fire_pulse (self, start, forward, damage*2, kick*2, 0, 250, 250, MOD_MBPC);
				self->ShotNumber =0;
			}
		}		
		
	if ((self->client->pers.inventory[self->client->ammo_index] -= shots) < 0)
	{
		self->client->pers.inventory[self->client->ammo_index] = 0;
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast (self->s.origin, MULTICAST_PVS);

/*
#ifdef	_WIN32
	_ftime(&self->lastattack_time);
#else
	ftime(&self->lastattack_time);
#endif
*/
}//NEEDLER
void botNeedler (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int	damage, kick;
	int	    xspread;
	int		yspread;



	if ((int)(level.time*10) & 1)	// only calculate every other frame
	{
		AngleVectors (self->s.angles, forward, right, NULL);
		G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

		VectorScale(forward, 8, ofs);
	    self->client->kick_angles[0] = -2;
		VectorAdd(self->s.origin, ofs, start);
		start[2] += self->viewheight - 8;


		if (self->enemy && infront(self, self->enemy))
		{
			float	dist, tf;

			dist = entdist(self, self->enemy);

			if (self->enemy->health > 0)
			{
				VectorCopy (self->enemy->s.origin, target);

				if (skill->value <= 1)
				{	// trail the player's velocity
					VectorMA(target, -0.2, self->enemy->velocity, target);
				}

				target[2] += self->enemy->viewheight - 8;
			}
			else
			{
				VectorCopy (self->enemy->s.origin, target);
				target[2] += self->enemy->viewheight - 8;
			}

			if (self->bot_stats->accuracy < 5)
			{//sentry ->client
				tf = (dist < 256) ? dist/2 : 256;
				tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
				if (self->enemy->client && !self->enemy->bot_client)
					tf *= (1 - (VectorLength(self->enemy->velocity)/600));
				VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.1), target);
			}

			VectorSubtract (target, start, forward);
			VectorNormalize (forward);

			vectoangles(forward, self->s.angles);
			if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
				self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
		}
		else
		{
			aborted_fire = true;
			return;
		}

		VectorCopy(forward, self->last_forward);
		VectorCopy(start, self->last_start);
	}
	else
	{
		VectorCopy(self->last_forward, forward);
		VectorCopy(self->last_start, start);
	}

	if (deathmatch->value)
	{	// normal damage is too extreme in dm
		damage = 3 + (((int)(random()*1000)) % 5);
		xspread = 125;
		yspread = 125;
		kick = 2 + (((int)(random()*1000)) % 8);
	}
	else
	{
		damage = 4 + (((int)(random()*1000)) % 5);
		xspread = 325;
		yspread = 375;
		kick = 3 + (((int)(random()*1000)) % 6);

	}

	if (is_quad)
	{
		damage *= 4;
		xspread *=2.5;
		yspread *=2.5;
		kick *=4;
	}

fire_needle(self, start, forward, damage, kick, TE_BLASTER, xspread, yspread);	
self->client->pers.inventory[self->client->ammo_index]--;
}//FLAME THROWER
void botFlameThrower (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int		damage = 5;
	float	damage_radius = 1000;
	float	dist=0, tf;



	if (is_quad)
		damage *= 4;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	dist = entdist(self, self->enemy);

	if (self->enemy && infront(self, self->enemy))
	{
//sentry ->client
		if ((self->enemy->health > 0) && (self->enemy->client && !self->enemy->bot_client) && (dist > 64))
		{
			VectorMA (self->enemy->s.origin, entdist(self, self->enemy) * (1/550), self->enemy->velocity, target);
			target[2] += self->enemy->viewheight - 8;

			if ((dist > 200) && self->enemy->groundentity)	// aim towards ground
				target[2] -= (4 * self->bot_stats->combat);
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

fire_flamethrower (self, start, forward, damage*3, 600, damage_radius*2.5);
self->client->pers.inventory[self->client->ammo_index]--;//find all these fixme acrid
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if (dist > 700)//FIXME 1000
	{	// check for a better long distance weapon
		botPickBestFarWeapon(self);
	}
}
//PoisonDart//ArmorDart
void botPoisonDart (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int	damage;
	float	dist=0, tf;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{

		dist = entdist(self, self->enemy);

		if (self->enemy->health > 0)
		{
			VectorCopy (self->enemy->s.origin, target);

			if (skill->value <= 1)
			{	// trail the player's velocity
				VectorMA(target, -0.2, self->enemy->velocity, target);
			}

			target[2] += self->enemy->viewheight - 8;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	if (deathmatch->value)
	{	// normal damage is too extreme in dm
		damage = 20;
	}
	else
	{
		damage = 25;
	}
	if (is_quad)
		damage *= 4;
    //confusing someone coded a poisondart and armor dart???
//	fire_poisondart (self, start, forward, damage, 750, EF_GREENGIB);
	fire_armordart (self, start, forward, damage, 750, EF_GREENGIB);
	self->client->pers.inventory[self->client->ammo_index]--;

	if (dist > 600)
	{	// check for a better long distance weapon
		botPickBestFarWeapon(self);
	}
		// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}
void botTeslaCoil (edict_t *self)//fixme acrid
{
	vec3_t	forward, right ,start, target;
	vec3_t	ofs;
	float	dist=0, tf;
//fixme    trace_t	tr;
	int		damage, kick;
    int     xspread;
	int     yspread;

	if (deathmatch->value)
	{	// normal damage is too extreme in dm
		damage = 3 + (((int)(random()*1000)) % 4);
		xspread = 25;
		yspread = 25;
		kick = 2 + (((int)(random()*1000)) % 8);
	}
	else
	{
		damage = 6 + (((int)(random()*1000)) % 4);
		xspread = 75;
		yspread = 75;
		kick = 3 + (((int)(random()*1000)) % 6);

	}

	if (is_quad)
	{
		damage *= 4;
		xspread *=2.5;
		yspread *=2.5;
		kick *=4;
	}
	AngleVectors (self->client->v_angle, forward, right, NULL);//WF
//	AngleVectors (self->s.angles, forward, right, NULL);//ERASER
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);
	VectorScale(forward, 8, ofs);

	VectorAdd(self->s.origin, ofs, start);
//	start[2] += self->viewheight - 8;//ERASER

	if (self->enemy && infront(self, self->enemy))
	{
		dist = entdist(self, self->enemy);
//sentry ->client
		if ((skill->value > 1) && (self->enemy->health > 0) && (self->enemy->client) && (dist > 64))
		{
			VectorMA (self->enemy->s.origin, dist/1000, self->enemy->velocity, target);
			target[2] += self->enemy->viewheight - 8;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (VectorLength(self->enemy->velocity)/600);
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

//	VectorCopy (self->enemy->s.origin, end);

	fire_telsa(self, start, forward, damage, kick, TE_BLASTER, xspread, yspread);
	self->client->pers.inventory[self->client->ammo_index]--;


}

void botInfectedDart (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int	damage, speed;
	float	dist=0, tf;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{

		dist = entdist(self, self->enemy);

		if (self->enemy->health > 0)
		{
			VectorCopy (self->enemy->s.origin, target);

			if (skill->value <= 1)
			{	// trail the player's velocity
				VectorMA(target, -0.2, self->enemy->velocity, target);
			}

			target[2] += self->enemy->viewheight - 8;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	if (deathmatch->value)
	{	// normal damage is too extreme in dm
		damage = 20;
	}
	else
	{
		damage = 25;
	}
	if (is_quad)
		damage *= 4;
	speed = 180;		//was 850
	fire_infecteddart (self, start, forward, damage, speed, EF_GIB);
	self->client->pers.inventory[self->client->ammo_index]--;

	if (dist > 600)//fixme acrid nurse
	{	// check for a better long distance weapon
		botPickBestFarWeapon(self);
	}
		// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}
void botLightningGun (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int		damage, kick, speed;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{
		float	dist, tf=0;

		dist = entdist(self, self->enemy);

		if (self->enemy->health > 0)
		{
			VectorCopy (self->enemy->s.origin, target);

//			if (skill->value <= 1)
			{	// trail the player's velocity
				VectorMA(target, -0.2, self->enemy->velocity, target);
			}

			target[2] += self->enemy->viewheight - 8;
//sentry ->client
			if (self->enemy->client && !self->enemy->bot_client)
				tf = (VectorLength(self->enemy->velocity) / 300) * 100;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		tf = 32;

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf += (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
		}

		if (tf > 0)
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.1), target);

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}
	if (deathmatch->value)
	{	// normal damage is too extreme in dm
		speed = 880; //Slow it down a little
		damage = 100 + (((int)(random()*1000)) % 30);
		kick = 165+ (((int)(random()*1000)) % 30) ;
	}
	else
	{
		speed = 960;
		damage = 150 + (((int)(random()*1000)) % 55);
		kick = 175 + (((int)(random()*1000)) % 55);
	}


	if (is_quad)
		damage *= 4;

	fire_lightning (self, start, forward, damage, speed, kick);//fixme, false);
	self->client->pers.inventory[self->client->ammo_index]--;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_RAILGUN | is_silenced);
	gi.multicast (self->s.origin, MULTICAST_PVS);

}
void botSHC (edict_t *self)
{
	vec3_t	start, target;
	vec3_t	forward, right, ofs;
	int	damage = 3;
	int kick = 8;
	float	dist=0, tf;

	AngleVectors (self->s.angles, forward, right, NULL);
	G_ProjectSource (self->s.origin, tv(8,8,self->viewheight-8), forward, right, start);

	VectorScale(forward, 8, ofs);
	VectorAdd(self->s.origin, ofs, start);
	start[2] += self->viewheight - 8;

	if (self->enemy && infront(self, self->enemy))
	{

		dist = entdist(self, self->enemy);

		if (self->enemy->health > 0)
		{
			VectorCopy (self->enemy->s.origin, target);

			if (skill->value <= 1)
			{	// trail the player's velocity
				VectorMA(target, -0.2, self->enemy->velocity, target);
			}

			target[2] += self->enemy->viewheight - 8;
		}
		else
		{
			VectorCopy (self->enemy->s.origin, target);
			target[2] += self->enemy->viewheight - 8;
		}

		if (self->bot_stats->accuracy < 5)
		{//sentry ->client
			tf = (dist < 256) ? dist/2 : 256;
			tf *= (float) ((5.0 - self->bot_stats->accuracy) / 5.0) * 2;
			if (self->enemy->client && !self->enemy->bot_client)
				tf *= (1 - (VectorLength(self->enemy->velocity)/600));
			VectorAdd(target, tv(crandom() * tf, crandom() * tf, crandom() * tf * 0.2), target);
		}

		VectorSubtract (target, start, forward);
		VectorNormalize (forward);

		vectoangles(forward, self->s.angles);
		if (abs(self->s.angles[PITCH]) > 15)	// don't go more than 15 degrees up or down
			self->s.angles[PITCH] = (((self->s.angles[PITCH] > 0) * 2) - 1) * 15;
	}
	else
	{
		aborted_fire = true;
		return;
//		AngleVectors (self->s.angles, forward, NULL, NULL);
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	if (deathmatch->value)
		fire_shc (self, start, forward, damage, kick,TE_GUNSHOT, 500, 500);
	else
		fire_shc (self, start, forward, damage, kick,TE_GUNSHOT, 500, 500);
	self->client->pers.inventory[self->client->ammo_index]--;

	if (dist > 700)
	{	// check for a better long distance weapon
		botPickBestFarWeapon(self);
	}
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}
//ACRID END
/**********************/
/* BOT BEST,CLOSE,FAR */
/**********************/
////////////////////////////////////////////////////////
// Returns the ammo index for the given new weapon item/
////////////////////////////////////////////////////////
int botAmmoIndex(gitem_t *weapon) {
//Shell Weapons
  if (weapon==item_shotgun)         return ITEM_INDEX(item_shells);
  if (weapon==item_supershotgun)    return ITEM_INDEX(item_shells);
  if (weapon==item_infecteddart)    return ITEM_INDEX(item_shells);
  if (weapon==item_poisondart)      return ITEM_INDEX(item_shells);
  if (weapon==item_shc)             return ITEM_INDEX(item_shells);
//3/99  if (weapon==item_knife)           return ITEM_INDEX(item_shells);
//Bullet Weapons
  if (weapon==item_pistol)          return ITEM_INDEX(item_bullets);
  if (weapon==item_needler)         return ITEM_INDEX(item_bullets);
  if (weapon==item_machinegun)      return ITEM_INDEX(item_bullets);
  if (weapon==item_chaingun)        return ITEM_INDEX(item_bullets);
  if (weapon==item_ak47)            return ITEM_INDEX(item_bullets);
  if (weapon==item_pulsecannon)     return ITEM_INDEX(item_bullets);
//Rocket Weapons
  if (weapon==item_rocketlauncher)        return ITEM_INDEX(item_rockets);
  if (weapon==item_rocketnapalmlauncher)  return ITEM_INDEX(item_rockets);
  if (weapon==item_pelletrocketlauncher)  return ITEM_INDEX(item_rockets);
  if (weapon==item_rocketclusterlauncher) return ITEM_INDEX(item_rockets);
  if (weapon==item_stingerrocketlauncher) return ITEM_INDEX(item_rockets);
//Slug Weapons
  if (weapon==item_railgun)         return ITEM_INDEX(item_slugs);
  if (weapon==item_lightninggun)    return ITEM_INDEX(item_slugs);
  if (weapon==item_sniperrifle)     return ITEM_INDEX(item_slugs);
//Cell Weapons
  if (weapon==item_hyperblaster)    return ITEM_INDEX(item_cells);
  if (weapon==item_bfg10k)          return ITEM_INDEX(item_cells);
  if (weapon==item_flamethrower)    return ITEM_INDEX(item_cells);
  if (weapon==item_telsacoil)       return ITEM_INDEX(item_cells);
//Grenade Weapons
  if (weapon==item_handgrenades)    return ITEM_INDEX(item_grenades);
  if (weapon==item_grenadelauncher) return ITEM_INDEX(item_grenades);

  return 0; // Else blaster
}
///////////////////////////////////////////////////////////////////////////////
// Check inventory ammo against new weapon and ammo specials like homing cells/
///////////////////////////////////////////////////////////////////////////////
qboolean botHasAmmoForWeapon(edict_t *self, gitem_t *weapon) {

  if ((int)dmflags->value & DF_INFINITE_AMMO)
    return true;

  if (weapon==item_blaster)
    return true;

  if (weapon == item_knife)//3/99
	  return true;

  if (weapon==item_bfg10k)
    return (self->client->pers.inventory[botAmmoIndex(item_bfg10k)]>=50);

  if (weapon==item_supershotgun)
    return (self->client->pers.inventory[botAmmoIndex(item_supershotgun)]>=2);

  return (self->client->pers.inventory[botAmmoIndex(weapon)]);
}
/////////////////////////////////////////
// Return a weapon in the bots inventory/
/////////////////////////////////////////
qboolean botHasWeaponInInventory(edict_t *self, gitem_t *item) {
  return (self->client->pers.inventory[ITEM_INDEX(item)]);
}
///////////////////////////////////////
// Returns true if bot has that weapon/
///////////////////////////////////////
qboolean botHasThisWeapon(edict_t *self, gitem_t *weapon) 
{   
	gclient_t	*client;
	gitem_t		*oldweapon;//test
	client = self->client;

	oldweapon = client->pers.weapon;//test


  if (botHasWeaponInInventory(self,weapon))
    if (botHasAmmoForWeapon(self,weapon)) {
      client->newweapon = weapon;

 // already using this weapon 
  if (client->pers.weapon == client->newweapon)
  return true;

      GetBotFireForWeapon(self->client->newweapon, &self->bot_fire);
	  self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;

		if (client->newweapon == item_blaster)
		self->fire_interval = FIRE_INTERVAL_BLASTER;
		if (client->newweapon == item_shotgun)
		self->fire_interval = FIRE_INTERVAL_SHOTGUN;
		else if (client->newweapon == item_supershotgun)
			self->fire_interval = FIRE_INTERVAL_SSHOTGUN;
		else if (client->newweapon == item_rocketlauncher)
			self->fire_interval = FIRE_INTERVAL_ROCKETLAUNCHER;
		else if (client->newweapon == item_grenadelauncher)
			self->fire_interval = FIRE_INTERVAL_GRENADELAUNCHER;
		else if (client->newweapon == item_railgun)
			self->fire_interval = FIRE_INTERVAL_RAILGUN;
		else if (client->newweapon == item_hyperblaster)
			self->fire_interval = FIRE_INTERVAL_HYPERBLASTER;
		else if (client->newweapon == item_chaingun)
			self->fire_interval = FIRE_INTERVAL_CHAINGUN;
		else if (client->newweapon == item_machinegun)
			self->fire_interval = FIRE_INTERVAL_MACHINEGUN;
		else if (client->newweapon == item_bfg10k)
			self->fire_interval = FIRE_INTERVAL_BFG;
//ACRID NEW WF STUFF
		else if (client->newweapon == item_sniperrifle)
			self->fire_interval =FIRE_INTERVAL_SNIPERRIFLE;
		else if (client->newweapon == item_lightninggun)
			self->fire_interval =FIRE_INTERVAL_LIGHTNINGGUN;
		else if (client->newweapon == item_infecteddart)
		    self->fire_interval =FIRE_INTERVAL_INFECTEDDART;
		else if (client->newweapon == item_pulsecannon)
		    self->fire_interval =FIRE_INTERVAL_PULSECANNON;
		else if (client->newweapon == item_telsacoil)
		    self->fire_interval =FIRE_INTERVAL_TELSACOIL;
		else if (client->newweapon == item_telsacoil)
		    self->fire_interval =FIRE_INTERVAL_FLAMETHROWER;
		else if (client->newweapon == item_pelletrocketlauncher)
		    self->fire_interval =FIRE_INTERVAL_PELLETROCKETLAUNCHER;
		else if (client->newweapon == item_rocketnapalmlauncher)
		    self->fire_interval =FIRE_INTERVAL_ROCKETNAPALMLAUNCHER;
		else if (client->newweapon == item_rocketclusterlauncher)
		    self->fire_interval =FIRE_INTERVAL_ROCKETCLUSTERLAUNCHER;
		else if (client->newweapon == item_stingerrocketlauncher)
			self->fire_interval = FIRE_INTERVAL_STINGERROCKETLAUNCHER;
		else if (client->newweapon == item_needler)
		    self->fire_interval =FIRE_INTERVAL_NEEDLER;
		else if (client->newweapon == item_shc)
		    self->fire_interval =FIRE_INTERVAL_SHC;
		else if (client->newweapon == item_handgrenades)
		    self->fire_interval =FIRE_INTERVAL_GRENADES;
		else if (client->newweapon == item_poisondart)
		    self->fire_interval =FIRE_INTERVAL_POISONDART;
		else if (client->newweapon == item_ak47)
		    self->fire_interval =FIRE_INTERVAL_AK47;
		else if (client->newweapon == item_pistol)
		    self->fire_interval =FIRE_INTERVAL_PISTOL;
		else if (client->newweapon == item_knife)
		    self->fire_interval =FIRE_INTERVAL_KNIFE;

	  if (CTFApplyHaste(self))
		  self->fire_interval *= 0.5;

        self->client->ammo_index=botAmmoIndex(weapon);
			client->pers.weapon = client->newweapon;
			self->client->pers.weapon = client->newweapon;

	// set visible model vwep with 3.20 code
	if (oldweapon != client->pers.weapon)
		ShowGun(self);


      return true; }

  return false;
}
/*
===================
botPickBestWeapon

  called everytime a weapon/ammo is picked up, or ammo runs out
===================
*/
void	botPickBestWeapon(edict_t *self)
{
	gclient_t	*client;
	gitem_t		*oldweapon;
	client = self->client;

	oldweapon = client->pers.weapon;
	// check favourite weapon
//	if ( client->pers.inventory[self->bot_stats->fav_weapon->tag]
//		&&  //acrid added if (
//NURSE Acrid FIXME
/*	if ((self->enemy->client) && (self->enemy->disease) &&
		 SameTeam(self->enemy, self)&&
		 (self->client->player_class == 2)) && 
      (self->client->pers.inventory[ITEM_INDEX(item_shells)] > 0))//crashing here
	{botDebugPrint("Nurse testing 2\n");
goto NCure;
	}*/
	if ((self->client->pers.weapon == item_knife) && 
		(CarryingFlag(self)))
	{
	botDebugPrint("Best knife for other (ACRID)\n");
	goto skip;
	}
	//new check favourite weapon// fav not knife 3/99
if ((client->pers.inventory[ITEM_INDEX(self->bot_stats->fav_weapon)]&&
	self->bot_stats->fav_weapon != item_knife && 
	client->pers.inventory[ITEM_INDEX(FindItem(self->bot_stats->fav_weapon->ammo))])
     //or fav is knife3/99
	||(client->pers.inventory[ITEM_INDEX(self->bot_stats->fav_weapon)] &&
       self->bot_stats->fav_weapon == item_knife))	
	{
//        botDebugPrint("%s picked favourite weapon\n", self->client->pers.netname);
		client->newweapon = self->bot_stats->fav_weapon;

		if (client->pers.weapon == client->newweapon)
			return;		// already using this weapon

		GetBotFireForWeapon(self->bot_stats->fav_weapon, &self->bot_fire);

		self->last_fire = level.time + BOT_CHANGEWEAPON_DELAY;

		if (client->newweapon == item_shotgun)
			self->fire_interval = FIRE_INTERVAL_SHOTGUN;
		else if (client->newweapon == item_supershotgun)
			self->fire_interval = FIRE_INTERVAL_SSHOTGUN;
		else if (client->newweapon == item_rocketlauncher)
			self->fire_interval = FIRE_INTERVAL_ROCKETLAUNCHER;
		else if (client->newweapon == item_grenadelauncher)
			self->fire_interval = FIRE_INTERVAL_GRENADELAUNCHER;
		else if (client->newweapon == item_railgun)
			self->fire_interval = FIRE_INTERVAL_RAILGUN;
		else if (client->newweapon == item_hyperblaster)
			self->fire_interval = FIRE_INTERVAL_HYPERBLASTER;
		else if (client->newweapon == item_chaingun)
			self->fire_interval = FIRE_INTERVAL_CHAINGUN;
		else if (client->newweapon == item_machinegun)
			self->fire_interval = FIRE_INTERVAL_MACHINEGUN;
		else if (client->newweapon == item_bfg10k)
			self->fire_interval = FIRE_INTERVAL_BFG;
//ACRID NEW WF STUFF
		else if (client->newweapon == item_sniperrifle)
			self->fire_interval =FIRE_INTERVAL_SNIPERRIFLE;
		else if (client->newweapon == item_lightninggun)
			self->fire_interval =FIRE_INTERVAL_LIGHTNINGGUN;
		else if (client->newweapon == item_infecteddart)
		    self->fire_interval =FIRE_INTERVAL_INFECTEDDART;
		else if (client->newweapon == item_pulsecannon)
		    self->fire_interval =FIRE_INTERVAL_PULSECANNON;
		else if (client->newweapon == item_telsacoil)
		    self->fire_interval =FIRE_INTERVAL_TELSACOIL;
		else if (client->newweapon == item_telsacoil)
		    self->fire_interval =FIRE_INTERVAL_FLAMETHROWER;
		else if (client->newweapon == item_pelletrocketlauncher)
		    self->fire_interval =FIRE_INTERVAL_PELLETROCKETLAUNCHER;
		else if (client->newweapon == item_rocketnapalmlauncher)
		    self->fire_interval =FIRE_INTERVAL_ROCKETNAPALMLAUNCHER;
		else if (client->newweapon == item_rocketclusterlauncher)
		    self->fire_interval =FIRE_INTERVAL_ROCKETCLUSTERLAUNCHER;
		else if (client->newweapon == item_stingerrocketlauncher)
			self->fire_interval = FIRE_INTERVAL_STINGERROCKETLAUNCHER;
		else if (client->newweapon == item_needler)
		    self->fire_interval =FIRE_INTERVAL_NEEDLER;
		else if (client->newweapon == item_shc)
		    self->fire_interval =FIRE_INTERVAL_SHC;
		else if (client->newweapon == item_handgrenades)
		    self->fire_interval =FIRE_INTERVAL_GRENADES;
		else if (client->newweapon == item_poisondart)
		    self->fire_interval =FIRE_INTERVAL_POISONDART;
		else if (client->newweapon == item_ak47)
		    self->fire_interval =FIRE_INTERVAL_AK47;
		else if (client->newweapon == item_pistol)
		    self->fire_interval =FIRE_INTERVAL_PISTOL;
		else if (client->newweapon == item_knife)
		    self->fire_interval =FIRE_INTERVAL_KNIFE;
//ACRID NEW WF STUFF

		if (CTFApplyHaste(self))
		{
			self->fire_interval *= 0.5;
		}

//		client->ammo_index = self->bot_stats->fav_weapon->tag;
		if (client->newweapon != item_knife)//3/99
        client->ammo_index = ITEM_INDEX(FindItem(self->bot_stats->fav_weapon->ammo));
		client->pers.weapon = client->newweapon;
		self->client->pers.weapon = client->newweapon;
		goto found;
	}
skip:
  if (botHasThisWeapon(self,item_bfg10k))         return;
  if (botHasThisWeapon(self,item_pulsecannon))    return;
  if (botHasThisWeapon(self,item_needler))        return;
  if (botHasThisWeapon(self,item_flamethrower))   return;
  if (botHasThisWeapon(self,item_lightninggun))   return;
  if (botHasThisWeapon(self,item_infecteddart))   return;
  if (botHasThisWeapon(self,item_pelletrocketlauncher))       return;
  if (botHasThisWeapon(self,item_rocketnapalmlauncher))       return;
  if (botHasThisWeapon(self,item_rocketclusterlauncher))      return;  
  if (botHasThisWeapon(self,item_shc))            return;  
  if (botHasThisWeapon(self,item_telsacoil))      return;
  if (botHasThisWeapon(self,item_ak47))           return;
  if (botHasThisWeapon(self,item_poisondart))     return;
  if (botHasThisWeapon(self,item_pistol))         return;  
  if (botHasThisWeapon(self,item_railgun))        return;
  if (botHasThisWeapon(self,item_hyperblaster))   return;
  if (botHasThisWeapon(self,item_rocketlauncher)) return;
  if (botHasThisWeapon(self,item_chaingun))       return;
  if (botHasThisWeapon(self,item_supershotgun))   return;
  if (botHasThisWeapon(self,item_grenadelauncher))return;
  if (botHasThisWeapon(self,item_machinegun))     return;
  if (botHasThisWeapon(self,item_stingerrocketlauncher))      return;
  if (botHasThisWeapon(self,item_shotgun))        return;
  if (botHasThisWeapon(self,item_handgrenades))   return;
  if (botHasThisWeapon(self,item_knife))          return;

  if (botHasThisWeapon(self,item_blaster))        return;

found:
//FOUND
	// set visible model vwep with 3.20 code
	if (oldweapon != client->pers.weapon)
		ShowGun(self);
};

int botHasWeaponForAmmo (gclient_t *client, gitem_t *item)//WORKS ACRID
{
	switch (item->tag)
	{
		case (AMMO_SHELLS) :
			{
				return (client->pers.inventory[ITEM_INDEX(item_shotgun)]  ||
				    client->pers.inventory[ITEM_INDEX(item_supershotgun)] ||
			        client->pers.inventory[ITEM_INDEX(item_infecteddart)] ||
				    client->pers.inventory[ITEM_INDEX(item_poisondart)] ||
                    client->pers.inventory[ITEM_INDEX(item_shc)]);// ||
				//3/99	client->pers.inventory[ITEM_INDEX(item_knife)] );
		}

		case (AMMO_ROCKETS) :
		{
			return (client->pers.inventory[ITEM_INDEX(item_rocketlauncher)] ||
				    client->pers.inventory[ITEM_INDEX(item_pelletrocketlauncher)] ||
				    client->pers.inventory[ITEM_INDEX(item_rocketnapalmlauncher)] ||
				    client->pers.inventory[ITEM_INDEX(item_stingerrocketlauncher)] ||
				    client->pers.inventory[ITEM_INDEX(item_rocketclusterlauncher)]);
		}

		case (AMMO_CELLS) :
		{
			return (client->pers.inventory[ITEM_INDEX(item_hyperblaster)] ||
                    client->pers.inventory[ITEM_INDEX(item_telsacoil)] ||
					client->pers.inventory[ITEM_INDEX(item_flamethrower)] ||
					client->pers.inventory[ITEM_INDEX(item_bfg10k)]);
		}
		
		case (AMMO_BULLETS) :
		{
			return (client->pers.inventory[ITEM_INDEX(item_chaingun)] ||
				  client->pers.inventory[ITEM_INDEX(item_pulsecannon)]||
				  client->pers.inventory[ITEM_INDEX(item_needler)]    ||
				  client->pers.inventory[ITEM_INDEX(item_ak47)]    ||
				  client->pers.inventory[ITEM_INDEX(item_pistol)]    ||				  
				  client->pers.inventory[ITEM_INDEX(item_machinegun)]);
		}

		case (AMMO_SLUGS) :
		{
			return (client->pers.inventory[ITEM_INDEX(item_railgun)] ||
				client->pers.inventory[ITEM_INDEX(item_sniperrifle)] ||
				client->pers.inventory[ITEM_INDEX(item_lightninggun)]);
		}

		case (AMMO_GRENADES) :
		{
			return (client->pers.inventory[ITEM_INDEX(item_grenadelauncher)] ||
				client->pers.inventory[ITEM_INDEX(item_handgrenades)]);
		}

		default :
		{
			gi.dprintf("botHasWeaponForAmmo: unkown ammo type - %i\n", item->ammo);
			return false;
		}

	}
}
/*//Acrid coed
int	ClientHasAnyWeapon(gclient_t	*client)
{
	if (client->pers.weapon != item_blaster)
		return true;

	if (client->pers.inventory[ITEM_INDEX(item_shotgun)])
		botDebugPrint("HAS ANY WEP (SHOTGUN)\n");
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_supershotgun)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_machinegun)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_chaingun)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_grenadelauncher)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_rocketlauncher)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_railgun)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_hyperblaster)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_bfg10k)])
		return true;
//FIXME ACRID NEW WEAPONS
    if (client->pers.inventory[ITEM_INDEX(item_sniperrifle)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_lightninggun)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_pulsecannon)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_telsacoil)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_needler)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_flamethrower)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_pelletrocketlauncher)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_rocketnapalmlauncher)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_rocketclusterlauncher)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_shc)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_handgrenades)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_ak47)])
		return true;
	if (client->pers.inventory[ITEM_INDEX(item_pistol)])
		return true;
	return false;
}
*///acrid coed
int	botCanPickupAmmo (gclient_t *client, gitem_t *item)
{
	int max, index;

	switch (item->tag)
	{
		case AMMO_BULLETS	: max = client->pers.max_bullets; break;
		case AMMO_SHELLS	: max = client->pers.max_shells; break;
		case AMMO_ROCKETS	: max = client->pers.max_rockets; break;
		case AMMO_GRENADES	: max = client->pers.max_grenades; break;
		case AMMO_CELLS		: max = client->pers.max_cells; break;
		case AMMO_SLUGS		: max = client->pers.max_slugs; break;
		default				: return false;
	}

//gi.dprintf("Max ammo set\n");

	index = ITEM_INDEX(item);

	if (client->pers.inventory[index] == max)
		return false;

//gi.dprintf("Can pickup ammo\n");

	return true;
}

void GetBotFireForWeapon(gitem_t	*weapon,	void (**bot_fire)(edict_t	*self))
{
	if (weapon == item_blaster)
		*bot_fire = botBlaster;//acrid
	if (weapon == item_rocketlauncher)
		*bot_fire = botRocketLauncher;
	else if (weapon == item_chaingun)
		*bot_fire = botChaingun;
	else if (weapon == item_supershotgun)
		*bot_fire = botSuperShotgun;
	else if (weapon == item_grenadelauncher)
		*bot_fire = botGrenadeLauncher;
	else if (weapon == item_railgun)
		*bot_fire = botRailgun;
	else if (weapon == item_hyperblaster)
		*bot_fire = botHyperblaster;
	else if (weapon == item_bfg10k)
		*bot_fire = botBFG;
	else if (weapon == item_shotgun)
		*bot_fire = botShotgun;
	else if (weapon == item_machinegun)
		*bot_fire = botMachineGun;
//ACRID
	else if (weapon == item_lightninggun)
        *bot_fire = botLightningGun;
	else if (weapon == item_infecteddart)
        *bot_fire = botInfectedDart;
	else if (weapon == item_pulsecannon)
        *bot_fire = botPulseCannon;
	else if (weapon == item_telsacoil)
        *bot_fire = botTeslaCoil;
	else if (weapon == item_sniperrifle)
        *bot_fire = botTeslaCoil;
	else if (weapon == item_flamethrower)
        *bot_fire = botFlameThrower;
	else if (weapon == item_pelletrocketlauncher)
        *bot_fire = botPelletRocketLauncher;
	else if (weapon == item_rocketnapalmlauncher)
        *bot_fire = botRocketNapalmLauncher;
	else if (weapon == item_rocketclusterlauncher)
        *bot_fire = botRocketClusterLauncher;
	else if (weapon == item_stingerrocketlauncher)
        *bot_fire = botStingerRocketLauncher;
	else if (weapon == item_needler)
        *bot_fire = botNeedler;
	else if (weapon == item_shc)
        *bot_fire = botSHC;
	else if (weapon == item_handgrenades)
        *bot_fire = botGrenades;
    else if (weapon == item_poisondart)
		*bot_fire = botPoisonDart;
    else if (weapon == item_ak47)
		*bot_fire = botAk47;
    else if (weapon == item_pistol)
		*bot_fire = botPistol;
    else if (weapon == item_knife)
		*bot_fire = botKnife;
//ACRID END
}

/*
===================
botPickBestCloseWeapon

  called when close to enemy, don't use RL, GL, BFG
===================
*/
void	botPickBestCloseWeapon(edict_t *self)
{
botDebugPrint("best close weapon\n");
//INFECTED DART CLOSE
//PELLET ROCKETLAUNCHER CLOSE
//PISTOL CLOSE
//AK47 CLOSE
//PULSECANNON CLOSE
//LIGHNING GUN CLOSE
//NEEDLER CLOSE
//FLAME THROWER CLOSE
//NAPALM ROCKETLAUNCHER CLOSE
//SHC CLOSE
//CHAINGUN CLOSE	
//SUPER SHOTGUN CLOSE
//HYPERBLASTER CLOSE
//GRENADE FIXME ACRID CLOSE
//MACHINEGUN CLOSE
//POISON DART CLOSE
//TESLA COIL CLOSE
//BFG CLOSE
//RAILGUN CLOSE
//CLUSTER ROCKETLAUNCHER CLOSE
//SHOTGUN CLOSE
//GRENADE LAUNCHER CLOSE
//ROCKETS CLOSE
  if (botHasThisWeapon(self,item_knife))          return;
  if (botHasThisWeapon(self,item_infecteddart))   return;
  if (botHasThisWeapon(self,item_pelletrocketlauncher))       return;
  if (botHasThisWeapon(self,item_pistol))         return;
  if (botHasThisWeapon(self,item_ak47))           return;
  if (botHasThisWeapon(self,item_pulsecannon))    return;
  if (botHasThisWeapon(self,item_lightninggun))   return;
  if (botHasThisWeapon(self,item_needler))        return;
  if (botHasThisWeapon(self,item_flamethrower))   return;
  if (botHasThisWeapon(self,item_rocketnapalmlauncher))       return;
  if (botHasThisWeapon(self,item_shc))            return;
  if (botHasThisWeapon(self,item_chaingun))       return;
  if (botHasThisWeapon(self,item_supershotgun))   return;
  if (botHasThisWeapon(self,item_hyperblaster))   return;
  if (botHasThisWeapon(self,item_handgrenades))   return;
  if (botHasThisWeapon(self,item_machinegun))     return;
  if (botHasThisWeapon(self,item_poisondart))     return;
  if (botHasThisWeapon(self,item_telsacoil))      return;
  if (botHasThisWeapon(self,item_bfg10k))         return;
  if (botHasThisWeapon(self,item_railgun))        return;  
  if (botHasThisWeapon(self,item_rocketclusterlauncher))      return;
  if (botHasThisWeapon(self,item_shotgun))        return;
  if (botHasThisWeapon(self,item_grenadelauncher))return;
  if (botHasThisWeapon(self,item_stingerrocketlauncher))      return;  
  if (botHasThisWeapon(self,item_rocketlauncher)) return;
  
  if (botHasThisWeapon(self,item_blaster))        return;

};

/*
===================
botPickBestFarWeapon

  called when far from enemy
===================
*/
void	botPickBestFarWeapon(edict_t *self)
{

botDebugPrint("best far weapon\n");
//BFG FAR
//AK47 FAR
//NAPALM ROCKETLAUNCHER FAR
//CLUSTER ROCKETLAUNCHER FAR
//ROCKETS FAR
//RAILGUN FAR
//TESLA COIL FAR
//LIGHNING GUN FAR
//INFECTED DART FAR
//POISON DART FAR
//HYPERBLASTER FAR
//PELLET ROCKETLAUNCHER FAR
//CHAINGUN FAR
//MACHINE GUN FAR
//FLAME THROWER FAR
//GRENADE LAUNCHER FAR
//SHC FAR
//SUPER SHOTGUN FAR
//PISTOL FAR
//SHOTGUN FAR
//NEEDLER FAR
  if (botHasThisWeapon(self,item_bfg10k))         return;
  if (botHasThisWeapon(self,item_rocketnapalmlauncher))       return;
  if (botHasThisWeapon(self,item_rocketclusterlauncher))      return;
  if (botHasThisWeapon(self,item_rocketlauncher)) return;
  if (botHasThisWeapon(self,item_railgun))        return;
  if (botHasThisWeapon(self,item_ak47))           return;
  if (botHasThisWeapon(self,item_stingerrocketlauncher))      return;
  if (botHasThisWeapon(self,item_pulsecannon))    return;
  if (botHasThisWeapon(self,item_lightninggun))   return;
  if (botHasThisWeapon(self,item_chaingun))       return;
  if (botHasThisWeapon(self,item_hyperblaster))   return;
  if (botHasThisWeapon(self,item_pelletrocketlauncher))       return;
  if (botHasThisWeapon(self,item_telsacoil))      return;
  if (botHasThisWeapon(self,item_infecteddart))   return;
  if (botHasThisWeapon(self,item_poisondart))     return;  
  if (botHasThisWeapon(self,item_grenadelauncher))return;
  if (botHasThisWeapon(self,item_flamethrower))   return;
  if (botHasThisWeapon(self,item_machinegun))     return;
  if (botHasThisWeapon(self,item_shc))            return;
  if (botHasThisWeapon(self,item_supershotgun))   return;
  if (botHasThisWeapon(self,item_pistol))         return;
  if (botHasThisWeapon(self,item_shotgun))        return;  
  if (botHasThisWeapon(self,item_needler))        return;
  if (botHasThisWeapon(self,item_handgrenades))   return;
  if (botHasThisWeapon(self,item_knife))          return;
  
  if (botHasThisWeapon(self,item_blaster))        return;
};
