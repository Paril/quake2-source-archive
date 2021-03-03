/*****************************************************************
 
 Grapple source code - by Acrid-, acridcola@hotmail.com
 
 ..............................................................
 
 This file is Copyright(c) 1999, Acrid-, All Rights Reserved.
 
 ..............................................................
 

 Should you decide to release a modified version of the Grapple, you
 MUST include the following text (minus the BEGIN and END lines) in 
 the documentation for your modification, and also on all web pages 
 related to your modification, should they exist.
 
 --- BEGIN ---
 
 The Grapple and related code ,minus the pushcode or lines marked tut, 
 are a product of Acrid- designed for Weapons Factory, and is available as 
 part of the Weapons Factory Source Code or a seperate tutorial.
 
 This program MUST NOT be sold in ANY form. If you have paid for
 this product, you should contact Acrid- at:
 acridcola@hotmail.com
 
 --- END ---
 
 have fun,
 
 Acrid-
 
 *****************************************************************/

//#define USE_OLD_GRAPPLE	1


/*
==============================
Grapple Code by Acrid

4/6/99  
code in other files: noted by newgrap 4/99
g_cmds.c
g_items.c
g_misc.c
g_phys.c
p_client.c
wf_local_client.h
ctf.h
I looked a few tutors but they never quite stopped bouncing on my system.
This code stops all bounce by turning off gravity and changing velocity
to 100 when length is 64. 
It also uses pusher code from a tutorial I found to correct bounce when 
grappled to objects like doors,plats etc.
Plus you can use this code for a normal ctfsytle grapple,a offhand 
ctfsytle grapple and a ctfsytle weapon grapple.
WF notes:
bind key +grapple for offhand
bind key grapple for normal
use grapple for weapon_grapple
by using BUTTON_USE offhand alias will no longer get reversed.
==============================
*/
#include "g_local.h"
#define DELAY_TIME         5000

void CTFFireGrapple (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect);
void CTFGrappleFire (edict_t *ent, vec3_t g_offset, int damage, int effect);


void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
qboolean Ended_Grappling (gclient_t *client)
   {
 return (!(client->buttons & BUTTON_USE) && client->oldbuttons & BUTTON_USE);
   }

qboolean Is_Grappling (gclient_t *client)
   {
       return (client->ctf_grapple == NULL) ? false : true;
   }
void CTFGrappleTouch2 (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float volume = 1.0;

       // Release if hitting its owner
       if (other == self->owner)
           return;
       //tut s
       if (!Is_Grappling(self->owner->client) && self->health == 0)
	   { botDebugPrint("!isgrap\n");
           return;
       }// tut e

       self->health = 0;//tut

	if (self->owner->client->ctf_grapplestate != CTF_GRAPPLE_STATE_FLY)
		return;

       if (surf && surf->flags & SURF_SKY)
       {
		   botDebugPrint("reset\n");
           CTFPlayerResetGrapple2(self->owner);
           return;
       }

//	VectorCopy(vec3_origin, self->velocity);

	PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

       if (other != g_edicts && other->clipmask == MASK_SHOT)//tut
           return;//tut

	if (other->takedamage || other->client) 
	{  //ERASER ADDED || other->client
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, 0, MOD_GRAPPLE);
		CTFResetGrapple2(self);
		return;
	}

	self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_PULL; // we're on hook
	self->enemy = other;

	self->solid = SOLID_NOT;

	if (self->owner->client->silencer_shots)
		volume = 0.2;

	gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grpull.wav"), volume, ATTN_NORM, 0);
	gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhit.wav"), volume, ATTN_NORM, 0);


	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (self->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (self->s.origin, MULTICAST_PVS);
//end zoid
    
	   //Pusher code
       if (other != g_edicts && other->inuse && (other->movetype == MOVETYPE_PUSH || 
           other->movetype == MOVETYPE_STOP))
       { 
           other->mynoise2 = self;
           self->owner->client->hook_touch = other;
//           self->enemy = other;
           self->groundentity = NULL;
           self->flags |= FL_TEAMSLAVE;
       }

       VectorClear(self->velocity);
       VectorClear(self->avelocity);
       self->touch = NULL;
       self->movetype = MOVETYPE_NONE;
       self->delay = level.time + DELAY_TIME;
       self->owner->client->on_hook = true;
       self->owner->groundentity = NULL;
       CTFGrapplePull2(self->owner);
}

void CTFGrappleThink2( edict_t *self )
{
	if (!self->owner || !self->owner->client)
	{
		G_FreeEdict(self);
		return;
	}

	if ((self->owner->health <= 0) || (self->owner->client->ctf_grapple != self))
	{
		gclient_t *cl;

		cl = self->owner->client;

		if (cl->ctf_grapple == self)
		{
			cl->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;	//sever codefix test
			gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grreset.wav"), 1, ATTN_NORM, 0);
			cl->ctf_grapple = NULL;
			cl->ctf_grapplereleasetime = level.time;
			cl->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
		}

		G_FreeEdict(self);

		return;
	}
       //start of tut
       if (level.time > self->delay)
           self->prethink = CTFPlayerResetGrapple2;
       else
       {
           if (self->owner->client->hook_touch) 
		   {
              edict_t *obj = self->owner->client->hook_touch;

              if (obj == g_edicts)
               {
                   CTFPlayerResetGrapple2(self->owner);
				   botDebugPrint("resetting 1\n");
                   return;
               }
               if (obj->inuse == false) 
			   {
                   CTFPlayerResetGrapple2(self->owner);
				   botDebugPrint("resetting 2\n");
                   return;
               }
               if (obj->deadflag == DEAD_DEAD)
               {
                   CTFPlayerResetGrapple2(self->owner);
				   botDebugPrint("resetting 3\n");
                   return;
               }
		   }
	self->nextthink = level.time + FRAMETIME;
       }
}

void CTFGrappleDrawCable2 (edict_t *self)
{
	vec3_t	offset, start, end, f, r;
	vec3_t	dir;
	float	distance;

	AngleVectors (self->owner->client->v_angle, f, r, NULL);
	VectorSet(offset, 16, 16, self->owner->viewheight-8);
	P_ProjectSource (self->owner->client, self->owner->s.origin, offset, f, r, start);

	VectorSubtract(start, self->owner->s.origin, offset);

	VectorSubtract (start, self->s.origin, dir);
	distance = VectorLength(dir);
	// don't draw cable if close
	if (distance < 64)
		return;

#if 0
	if (distance > 256)
		return;

	// check for min/max pitch
	vectoangles (dir, angles);
	if (angles[0] < -180)
		angles[0] += 360;
	if (fabs(angles[0]) > 45)
		return;

	trace_t	tr; //!!

	tr = gi.trace (start, NULL, NULL, self->s.origin, self, MASK_SHOT);
	if (tr.ent != self) {
		CTFResetGrapple(self);
		return;
	}
#endif


	VectorCopy (self->s.origin, end);

	gi.WriteByte (svc_temp_entity);
#if 1 //def USE_GRAPPLE_CABLE
	gi.WriteByte (TE_GRAPPLE_CABLE);
	gi.WriteShort (self->owner - g_edicts);
	gi.WritePosition (self->owner->s.origin);
	gi.WritePosition (end);
	gi.WritePosition (offset);
#else
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (end);
	gi.WritePosition (start);
#endif
	gi.multicast (self->s.origin, MULTICAST_PVS);
}


void CTFFireGrapple2 (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*grapple;
	trace_t	tr;

#ifdef USE_OLD_GRAPPLE
	CTFFireGrapple (self, start, dir, damage, speed, effect);
	return;
#endif

	VectorNormalize (dir);

	grapple = G_Spawn();
	VectorCopy (start, grapple->s.origin);
	VectorCopy (start, grapple->s.old_origin);
	vectoangles (dir, grapple->s.angles);
	VectorScale (dir, speed, grapple->velocity);
    VectorSet(grapple->avelocity, 0, 0, 500);//tut
    grapple->classname = "hook";//tut
	grapple->movetype = MOVETYPE_FLYMISSILE;
	grapple->clipmask = MASK_SHOT;
    grapple->svflags |= SVF_DEADMONSTER;//tut
	grapple->solid = SOLID_BBOX;
	grapple->s.effects |= effect;
	VectorClear (grapple->mins);
	VectorClear (grapple->maxs);

	grapple->s.modelindex = gi.modelindex ("models/weapons/grapple/hook/tris.md2");
	grapple->owner = self;
	grapple->touch = CTFGrappleTouch2;
    grapple->delay = level.time + DELAY_TIME;//tut
//    grapple->nextthink = level.time;//tut
	grapple->nextthink = level.time + FRAMETIME;
	grapple->think = CTFGrappleThink2;
    grapple->prethink = CTFGrappleDrawCable2;
    grapple->health = 100;//tut
    grapple->svflags = SVF_MONSTER;//tut
	grapple->dmg = damage;
	self->client->ctf_grapple = grapple;
	self->client->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook

	gi.linkentity (grapple);

	tr = gi.trace (self->s.origin, NULL, NULL, grapple->s.origin, grapple, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (grapple->s.origin, -10, dir, grapple->s.origin);
		grapple->touch (grapple, tr.ent, NULL, NULL);
	}


}
void CTFGrappleFire2 (edict_t *ent, vec3_t g_offset, int damage, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	float volume = 1.0;
     
	//Feign 5/99
	if (ent->client->pers.feign)
		return;

#ifdef USE_OLD_GRAPPLE
	CTFGrappleFire (ent, g_offset, damage, effect);
	return;
#endif

	if (ent->client->ctf_grapple) //tut s not needed? 
	   {
           return;
       }//tut e

	if (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
		return; // it's already out

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;
	VectorSet(offset, 24, 8, ent->viewheight-8+2);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (ent->client->silencer_shots)
		volume = 0.2;

	gi.sound (ent, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grfire.wav"), volume, ATTN_NORM, 0);
    CTFFireGrapple2 (ent, start, forward, damage, CTF_GRAPPLE_SPEED, effect);

	ent->client->hook_touch = NULL;//tut

#if 0
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
#endif

	PlayerNoise(ent, start, PNOISE_WEAPON);

//Eraser START: record this position, so we drop a grapple node here, rather than where the player is when they leave the ground
	if (!ent->bot_client)
	{	botDebugPrint("Grapple pull\n");
	VectorCopy(ent->s.origin, ent->animate_org);
	}
//ERASER END
}
void CTFWeapon_Grapple_Fire2 (edict_t *ent)
{
	int		damage;

	damage = 10;
	CTFGrappleFire2 (ent, vec3_origin, damage, 0);
//	ent->client->ps.gunframe++;//COed by Gregg Reno
}

void CTFWeapon_Grapple2 (edict_t *ent)
{
	static int	pause_frames[]	= {10, 18, 27, 0};
	static int	fire_frames[]	= {6, 0};
	int prevstate;

#ifdef USE_OLD_GRAPPLE
	CTFWeapon_Grapple (ent);
	return;
#endif

	// if the the attack button is still down, stay in the firing frame
	if ((ent->client->buttons & BUTTON_ATTACK) &&
		ent->client->weaponstate == WEAPON_FIRING &&
		ent->client->ctf_grapple)
	{
		botDebugPrint("firing\n");
		ent->client->ps.gunframe = 9;
	}
	if (!(ent->client->buttons & BUTTON_ATTACK) && ent->client->ctf_grapple) 
	{		botDebugPrint("reset 3\n");
		CTFResetGrapple2(ent->client->ctf_grapple);
		if (ent->client->weaponstate == WEAPON_FIRING)
			ent->client->weaponstate = WEAPON_READY;
	}


	if (ent->client->newweapon && ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY &&
		ent->client->weaponstate == WEAPON_FIRING) 
	{		botDebugPrint("state\n");
		// he wants to change weapons while grappled
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = 32;
	}

	prevstate = ent->client->weaponstate;
    Weapon_Generic (ent, 5, 9, 31, 36, pause_frames, fire_frames,CTFWeapon_Grapple_Fire2);

	// if we just switched back to grapple, immediately go to fire frame
	if (prevstate == WEAPON_ACTIVATING &&
		ent->client->weaponstate == WEAPON_READY &&
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) 
	{
		if (!(ent->client->buttons & BUTTON_ATTACK))
			ent->client->ps.gunframe = 9;
		else
			ent->client->ps.gunframe = 5;
		ent->client->weaponstate = WEAPON_FIRING;
	}
}
// ent is player
void CTFPlayerResetGrapple2(edict_t *ent)
{ 
	if (ent->client && ent->client->ctf_grapple)
//		CTFResetGrapple(ent->client->ctf_grapple);
		CTFResetGrapple2(ent->client->ctf_grapple);
}

// self is grapple, not player
void CTFResetGrapple2(edict_t *self)
{
	   float volume = 1.0;
//	   gclient_t *cl;


	   edict_t *owner = self->owner;//tut
       gclient_t *client = self->owner->client;//tut replaces cl
       edict_t *link = self->teamchain;//tut used for map ents push code
       client->on_hook = false;//tut
       client->hook_touch = NULL;//tut
       botDebugPrint("SelfReset %s\n",self->classname);

 #ifdef USE_OLD_GRAPPLE
	CTFResetGrapple(self);
	return;
#endif

 
 //	if (self->owner->client->ctf_grapple) 
//	{
       if (client->ctf_grapple != NULL)//tut section
	   {

           client->ctf_grapple = NULL;//replaced cl 
           VectorClear(client->oldvelocity);
           self->think = NULL;
           if (self->enemy) 
		   {
               self->enemy->mynoise2 = NULL;
           }
                               //tut section end

		if (self->owner->client->silencer_shots)
			volume = 0.2;

		if ((self->owner) && (self->owner->client))
		{
			gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grreset.wav"), volume, ATTN_NORM, 0);
//			cl = self->owner->client;
//			client->ctf_grapple = NULL;
			client->ctf_grapplereleasetime = level.time;
			client->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
			client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;	//sever codefix test
		}//tut
		G_FreeEdict(self);//tut
	}//tut

}
void CTFGrapplePull2(edict_t *player)
{
       vec3_t hookDir ,v;
       vec_t vlen;
	   edict_t *self = player->client->ctf_grapple;

#ifdef USE_OLD_GRAPPLE
	CTFGrapplePull(player);
	return;
#endif

	if (strcmp(player->client->pers.weapon->classname, "weapon_grapple") == 0 &&
		!player->client->newweapon &&
		player->client->weaponstate != WEAPON_FIRING &&
		player->client->weaponstate != WEAPON_ACTIVATING)
	{
		CTFResetGrapple2(self);
		return;
	}

	if (self->enemy) 
	{
		if (self->enemy->solid == SOLID_NOT) 
		{
			CTFResetGrapple2(self);
			return;
		}
		if (self->enemy->solid == SOLID_BBOX) 
		{
			VectorScale(self->enemy->size, 0.5, v);
			VectorAdd(v, self->enemy->s.origin, v);
			VectorAdd(v, self->enemy->mins, self->s.origin);
			gi.linkentity (self);
		} else
			VectorCopy(self->enemy->velocity, self->velocity);
		if (self->enemy->takedamage &&
			!CheckTeamDamage (self->enemy, self->owner)) 
		{
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			T_Damage (self->enemy, self, self->owner, self->velocity, self->s.origin, vec3_origin, 1, 1, 0, MOD_GRAPPLE);
			gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhurt.wav"), volume, ATTN_NORM, 0);
		}
		// he died /ERASER ADDED || self->enemy->deadflag
		if (!self->enemy || self->enemy->deadflag) 
		{ 
			CTFResetGrapple2(self);
			return;
		}
	}

       VectorSubtract(self->s.origin, player->s.origin,hookDir);
       vlen = VectorNormalize(hookDir);
	   //vector speeds
	   if (vlen < 64)
	   {
       VectorScale(hookDir, 100,player->velocity);
	   }
	   else
	   {
       VectorScale(hookDir, CTF_GRAPPLE_PULL_SPEED,player->velocity);//tut
	   }
       VectorCopy(hookDir, player->movedir);//tut

	if (player->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) 
	{

//sever codefix test
		/*			vec3_t forward, up;

		AngleVectors (self->owner->client->v_angle, forward, NULL, up);
		VectorCopy(self->owner->s.origin, v);
		v[2] += self->owner->viewheight;
		VectorSubtract (self->s.origin, v, hookDir);*/
//server codefix end

		if (player->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL &&
			vlen < 64) {
			float volume = 1.0;

			if (player->client->silencer_shots)
				volume = 0.2;

			self->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;	//sever codefix test
			gi.sound (player, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grhang.wav"), volume, ATTN_NORM, 0);
			player->client->ctf_grapplestate = CTF_GRAPPLE_STATE_HANG;
		}
	}
        /*To move the player off the ground just a bit 
          so he doesn't staystuck (version 3.17 bug)tut*/
       if (player->velocity[2] > 0) 
	   {
           vec3_t traceTo;
           trace_t trace;
           // find the point immediately above the player's origin
           VectorCopy(player->s.origin, traceTo);
           traceTo[2] += 1;
           // trace to it
           trace = gi.trace(traceTo, player->mins, player->maxs, traceTo, player, MASK_PLAYERSOLID);
           // if there isn't a solid immediately above the player
           if (!trace.startsolid) 
		   {
               player->s.origin[2] += 1;    // make sure player off ground
           }
       }

   }

