/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_weapons.c
  Description: Weapons routines. 

\**********************************************************/

#include "g_local.h"

void Teargas_Explode (edict_t *ent);
void Weapon_Generic2 (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int FRAME_FIRE2_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent), void (*fire2)(edict_t *ent));
void Weapon_Generic3 (edict_t *ent, int, int, int, int, int, int, int *, int *, 
void (*fire)(edict_t *), 
void (*fire2)(edict_t *), 
void (*fire3)(edict_t *));
void Staff_fireball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void fire_missile (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_gattling (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
void fire_gun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int mod);
void Staff_Lightning (edict_t *self, vec3_t start, vec3_t dir, int damage);

#define SND_ASSIM_PICKUP      "weapons/assim/pickup.wav"
#define SND_ASSIM_FIRE        "weapons/assim/fire.wav"
#define SND_ASSIM_RELOAD      "weapons/assim/reload.wav"
#define SND_ASSIM_WALLHIT     "weapons/assim/wallhit.wav"    
#define SND_ASSIM_NOAMMO      "weapons/assim/noammo.wav"    

#define SND_HAMMER_PICKUP     "weapons/hammer/pickup.wav"     // uh like WTF?!
#define SND_HAMMER_FIRE       "weapons/hammer/fire.wav"     
#define SND_HAMMER_HIT        "weapons/hammer/hit.wav"      
#define SND_HAMMER_WALLHIT    "weapons/hammer/wallhit.wav"      

#define SND_PLASMA_PICKUP     "weapons/plasma/pickup.wav"
#define SND_PLASMA_FIRE       "weapons/plasma/fire.wav"
#define SND_PLASMA_RELOAD     "weapons/plasma/reload.wav"
#define SND_PLASMA_NOAMMO     "weapons/plasma/noammo.wav"
#define SND_PLASMA_WALLHIT    "weapons/plasma/wallhit.wav"

#define SND_RIFLE_PICKUP      "weapons/rifle/pickup.wav"
#define SND_RIFLE_NOAMMO      "weapons/rifle/noammo.wav"
#define SND_RIFLE_FIRE        "weapons/rifle/fire.wav"
#define SND_RIFLE_ZOOM        "weapons/rifle/zoom.wav"
#define SND_RIFLE_RELOAD      "weapons/rifle/reload.wav"
#define SND_RIFLE_WALLHIT     "weapons/rifle/wallhit.wav"

#define SND_RIPPER_PICKUP     "weapons/ripper/pickup.wav"
#define SND_RIPPER_SPIN       "weapons/ripper/spin.wav"
#define SND_RIPPER_FIRE       "weapons/ripper/fire.wav"
#define SND_RIPPER_SPINDOWN   "weapons/ripper/spindowm.wav"
#define SND_RIPPER_SPINUP     "weapons/ripper/spinup.wav"
#define SND_RIPPER_RELOAD     "weapons/ripper/reload.wav"
#define SND_RIPPER_NOAMMO     "weapons/ripper/noammo.wav"

#define SND_STAFF_FIRE        "weapons/staff/fire.wav"
#define SND_STAFF_LIGHTNING   "weapons/staff/fire2.wav"
#define SND_STAFF_PICKUP      "weapons/staff/pickup.wav"

#define SND_TRIGAT_PICKUP     "weapons/trigat/pickup.wav"
#define SND_TRIGAT_FIRE       "weapons/trigat/fire.wav"
#define SND_TRIGAT_RELOAD     "weapons/trigat/reload.wav"
#define SND_TRIGAT_WALLHIT    "weapons/trigat/wallhit.wav"
#define SND_TRIGAT_NOAMMO     "weapons/trigat/noammo.wav"

#define SND_U23K_FIRE         "weapons/u23k/fire.wav"       
#define SND_U23K_PICKUP       "weapons/u23k/pickup.wav"       
#define SND_U23K_RELOAD       "weapons/u23k/reload.wav"
#define SND_U23K_NOAMMO       "weapons/u23k/noammo.wav"

#define SND_WASP_FIRE         "weapons/wasp/fire.wav"
#define SND_WASP_RELOAD       "weapons/wasp/reload.wav"
#define SND_WASP_NOAMMO       "weapons/wasp/noammo.wav"
#define SND_WASP_HEATUP       "weapons/wasp/heatup.wav"

// hmmm?
#define SND_CEC_FIRE          "weapons/cec/fire.wav"
#define SND_CEC_BALL          "weapons/cec/ball.wav"        //"misc/lasfly.wav"
#define SND_CEC_NOAMMO        "weapons/cec/noammo.wav"      

///////////////////////////////////
// RELOAD: U23K, ASSIM, WASP
///////////////////////////////////
void weapon_reload (edict_t *ent)
{
   if (ent->client->pers.weapon == FindItem("u23k"))
      {
      ent->client->reload_time = -1;
      if (ent->client->ps.gunframe < 56)
         {
         ent->client->ps.gunframe = 56;
         gi.sound (ent, CHAN_RELIABLE, gi.soundindex(SND_U23K_RELOAD), 1, ATTN_STATIC, 0);         
         }
      else
         {
         ent->client->ps.gunframe++;
         if (ent->client->ps.gunframe >= 68)
            {
            ent->client->reload_time = 0;
            ent->client->ps.gunframe = 9;
            }
         }
      return;
      }
   if (ent->client->pers.weapon == FindItem("u23k2"))
      {
      ent->client->reload_time = -1;
      if (ent->client->ps.gunframe < 64)
         {
         ent->client->ps.gunframe = 64;
         gi.sound (ent, CHAN_RELIABLE, gi.soundindex(SND_U23K_RELOAD), 1, ATTN_STATIC, 0);         
         }
      else
         {
         ent->client->ps.gunframe++;
         if (ent->client->ps.gunframe == 76)
            gi.sound (ent, CHAN_RELIABLE, gi.soundindex(SND_U23K_RELOAD), 1, ATTN_STATIC, 0);
         if (ent->client->ps.gunframe >= 89)
            {
            ent->client->reload_time = 0;
            ent->client->ps.gunframe = 9;
            }
         }
      return;
      }
   if (ent->client->pers.weapon == FindItem("assimilator"))
      {
      ent->client->reload_time = -1;
      if (ent->client->ps.gunframe < 42)
         {
         ent->client->sub_weapon = 0;
         ent->client->ps.gunframe = 42;
         gi.sound (ent, CHAN_RELIABLE, gi.soundindex(SND_ASSIM_RELOAD), 1, ATTN_STATIC, 0);
         }
      else
         {
         ent->client->ps.gunframe++;
         if (ent->client->ps.gunframe >= 48)
            {           
            ent->client->sub_weapon++;
            if (ent->client->sub_weapon > 5)
               {
               ent->client->reload_time = 0;
               ent->client->ps.gunframe = 12;
               }
            else
               {
               ent->client->ps.gunframe = 42;
               gi.sound (ent, CHAN_RELIABLE, gi.soundindex(SND_ASSIM_RELOAD), 1, ATTN_STATIC, 0);
               }
            }
         }
      return;
      }   
   ent->client->reload_time = 0;
}


///////////////////////////////////
// THROW SHELL: U23K, SNIPER
///////////////////////////////////
void ThrowShell (edict_t *ent, vec3_t start)
{
	edict_t *shell;
	shell = G_Spawn();

   VectorCopy (start, shell->s.origin);
   VectorCopy (ent->client->v_angle, shell->s.angles);

   shell->s.angles[1] += 90;
	gi.setmodel (shell, "models/objects/shell.md2");
	shell->solid = SOLID_NOT;
   shell->flipping = FLIP_SHELL;
	shell->movetype = MOVETYPE_BOUNCE;

	shell->velocity[2] = 75 + rand() % 75;
   shell->velocity[1] = -60 + rand() % 120;
   shell->velocity[0] = -60 + rand() % 120;


	shell->think = G_FreeEdict;
	shell->nextthink = level.time + 5;
	gi.linkentity (shell);
}


///////////////////////////////////
// HAMMER
///////////////////////////////////
void hammer_hit (edict_t *self, vec3_t start, vec3_t offset, int damage, int kick)
{
	trace_t		tr;
	vec3_t		forward, right, end;

	//see if enemy is in range

   AngleVectors (self->s.angles, forward, right, NULL);
   G_ProjectSource (start, offset, forward, right, end);   
   tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);

   if (tr.fraction < 1)
      {
      if (tr.ent)
	  {
		  T_Damage (tr.ent, self, self, forward, vec3_origin, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, MOD_HAMMER);
		if (!tr.ent->takedamage)
		{
            gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_SHOTGUN);
			gi.WritePosition (tr.endpos);
			gi.WriteDir (tr.plane.normal);
			gi.multicast (tr.endpos, MULTICAST_PVS);
		}
        gi.sound (self, CHAN_RELIABLE, gi.soundindex (SND_HAMMER_HIT), 1, ATTN_NORM, 0);
	  }
	else 
	{
		gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BLOOD);
			gi.WritePosition (tr.endpos);
			gi.WriteDir (tr.plane.normal);
			gi.multicast (tr.endpos, MULTICAST_PVS);
            gi.sound (self, CHAN_RELIABLE, gi.soundindex (SND_HAMMER_WALLHIT), 1, ATTN_NORM, 0);
	  }
	}
}

void weapon_hammer_fire (edict_t *ent)
{
	vec3_t		aim, start;

	VectorSet (aim, 60, 0, 0);
   VectorCopy (ent->s.origin, start);
   start[2] += ent->viewheight;
   gi.sound (ent, CHAN_WEAPON, gi.soundindex (SND_HAMMER_FIRE), 1, ATTN_NORM, 0);
   hammer_hit (ent, start, aim, 30, 80);
	ent->client->ps.gunframe++;
}

void Weapon_Hammer (edict_t *ent)
{
	static int	pause_frames[]	= {23, 32, 44, 0};
	static int	fire_frames[]	= {7, 58, 0};
	Weapon_Generic2 (ent, 4, 9, 51, 54, 61, pause_frames, fire_frames, weapon_hammer_fire, weapon_hammer_fire);
}

///////////////////////////////////
// U23K
///////////////////////////////////
void u23k_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	vec3_t		offset;

   if (ent->client->reload_time < ent->client->pers.weapon->quantity 
      && ent->client->reload_time >= 0)
      ent->client->reload_time++;
   else
      {
      ent->client->ps.gunframe = 8;
      gi.sound(ent, CHAN_VOICE, gi.soundindex(SND_U23K_NOAMMO), 1, ATTN_NORM, 0);      
      return;
      }
  
   ent->client->ps.gunframe++;
	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	   {
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex(SND_U23K_NOAMMO), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		//NoAmmoWeaponChange (ent);
		return;
	   }

	for (i = 1 ; i < 3 ; i++)
	   {
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	   }
	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 18, 8, ent->viewheight - 8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
   
   fire_gun(ent, start, forward, 20, 30, MOD_U23K);
	VectorSet(offset, 18, 8, ent->viewheight - 8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
   if (!(ent->client->pers.inventory[ent->client->ammo_index] & 1))
      ThrowShell(ent, start);
	//PlayerNoise(ent, start, PNOISE_WEAPON);
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
   gi.WriteByte (svc_temp_entity);
   gi.WriteByte (TE_SPARKS);
   gi.WritePosition (start);
   gi.WriteDir (forward);
   gi.multicast (start, MULTICAST_PVS);
   gi.sound (ent, CHAN_RELIABLE, gi.soundindex(SND_U23K_FIRE), 1, ATTN_STATIC, 0);
}

void Weapon_u23k (edict_t *ent)
{
   // ADDED:
	static int	pause_frames[]	= {21, 34, 47, 0};
   static int	fire_frames[]	= {5, 0};
   Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, u23k_Fire);  
}

void u23k2_Fire2 (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	vec3_t		offset;

   if (ent->client->reload_time < RELOAD_U23K2 && ent->client->reload_time >= 0)
      ent->client->reload_time++;
   else
      {
      ent->client->ps.gunframe = 8;
      gi.sound(ent, CHAN_VOICE, gi.soundindex(SND_U23K_NOAMMO), 1, ATTN_NORM, 0);      
      return;
      }
  
   ent->client->ps.gunframe++;
	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	   {
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex(SND_U23K_NOAMMO), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		//NoAmmoWeaponChange (ent);
		return;
	   }

	for (i = 1 ; i < 3 ; i++)
	   {
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	   }
	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 18, -8, ent->viewheight - 8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
   
   fire_gun(ent, start, forward, 20, 30, MOD_U23K);
	VectorSet(offset, 18, -8, ent->viewheight - 8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
   if (!(ent->client->pers.inventory[ent->client->ammo_index] & 1))
      ThrowShell(ent, start);
	//PlayerNoise(ent, start, PNOISE_WEAPON);
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
   gi.WriteByte (svc_temp_entity);
   gi.WriteByte (TE_SPARKS);
   gi.WritePosition (start);
   gi.WriteDir (forward);
   gi.multicast (start, MULTICAST_PVS);
   gi.sound (ent, CHAN_RELIABLE, gi.soundindex(SND_U23K_FIRE), 1, ATTN_STATIC, 0);
}

void u23k2_Fire3 (edict_t *ent)
{
   u23k_Fire (ent);
   u23k2_Fire2 (ent);
}

void Weapon_u23k2 (edict_t *ent)
{
   // ADDED:
	static int	pause_frames[]	= {21, 34, 47, 0};
   static int	fire_frames[]	= {5, 57, 61, 0};
   Weapon_Generic3 (ent, 4, 8, 52, 55, 59, 63, pause_frames, fire_frames, u23k_Fire, u23k2_Fire2, u23k2_Fire3);  
}

///////////////////////////////////
// GATTLING
///////////////////////////////////
void Gattling_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	vec3_t		offset;
 
   ent->client->ps.gunframe++;
	ent->s.frame--;
	if (!(ent->client->buttons & BUTTON_ATTACK))
      return;
   if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	   {
		ent->client->ps.gunframe = 17;
		if (level.time >= ent->pain_debounce_time)
		   {
			gi.sound(ent, CHAN_VOICE, gi.soundindex(SND_TRIGAT_NOAMMO), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		   }
		//NoAmmoWeaponChange (ent);
		return;
	   }

	for (i = 1 ; i < 3 ; i++)
	   {
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	   }
	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;
	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 24, 4, ent->viewheight - 8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);    
   fire_gattling (ent, start, forward, 20, 800);

	//PlayerNoise(ent, start, PNOISE_WEAPON);
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
   gi.WriteByte (svc_temp_entity);
   gi.WriteByte (TE_SPARKS);
   gi.WritePosition (start);
   gi.WriteDir (forward);
   gi.multicast (start, MULTICAST_PVS);
   if (ent->client->ps.gunframe > 16 )
      ent->client->ps.gunframe = 14;
}

void Weapon_Gattling (edict_t *ent)
{
   // ADDED:
	static int	pause_frames[]	= {27, 34, 41, 0};
   static int	fire_frames[]	= {10, 12, 14, 15, 16, 0};
   Weapon_Generic (ent, 8, 23, 46, 50, pause_frames, fire_frames, Gattling_Fire);  
}

///////////////////////////////////
// ASSIMILATOR
///////////////////////////////////
void Assim_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	vec3_t		offset;

   if (ent->client->reload_time < RELOAD_ASSIM && ent->client->reload_time >= 0)
      ent->client->reload_time++;
   else
      {
      ent->client->ps.gunframe = 11;
      gi.sound(ent, CHAN_VOICE, gi.soundindex(SND_ASSIM_NOAMMO), 1, ATTN_NORM, 0);      
      return;
      }
  
   ent->client->ps.gunframe++;
	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	   {
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex(SND_ASSIM_NOAMMO), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		//NoAmmoWeaponChange (ent);
		return;
	   }

	for (i = 1 ; i < 3 ; i++)
	   {
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	   }
	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
   AngleVectors (angles, forward, right, NULL);
   VectorSet(offset, 24, 5, ent->viewheight - 8);
   if (!ent->client->weapon_mode) 
      {
	   P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
      fire_missile (ent, start, forward, 80, 800, 70, 170);
	   if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		   ent->client->pers.inventory[ent->client->ammo_index]--;
      }
   // mode 1 is split fire
   else if(ent->client->weapon_mode == 1)
      {
      angles[0] -= 7;
      AngleVectors (angles, forward, right, NULL);
	   P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
      fire_missile (ent, start, forward, 80, 800, 70, 170);

      angles[0] += 14;
      AngleVectors (angles, forward, right, NULL);
	   P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
      fire_missile (ent, start, forward, 80, 800, 70, 170);
	   
      angles[0] -= 3;
      angles[1] -= 5;
      AngleVectors (angles, forward, right, NULL);
	   P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
      fire_missile (ent, start, forward, 80, 800, 70, 170);

      angles[1] += 10;
      AngleVectors (angles, forward, right, NULL);
      P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
      fire_missile (ent, start, forward, 80, 800, 70, 170);

      angles[0] -= 8;
      AngleVectors (angles, forward, right, NULL);
	   P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
      fire_missile (ent, start, forward, 80, 800, 70, 170);

      angles[1] -= 10;
      AngleVectors (angles, forward, right, NULL);
      P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
      fire_missile (ent, start, forward, 80, 800, 70, 170);

      ent->client->reload_time = RELOAD_ASSIM;
      if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		   ent->client->pers.inventory[ent->client->ammo_index] -= 6;
	     weapon_reload (ent);

   }
   else if (ent->client->pers.inventory[ent->client->ammo_index] >= 3)
      {
		if (ent->client->locked)
			{
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
			fire_missile (ent, start, forward, 80, 800, 70, 170);
			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
				ent->client->pers.inventory[ent->client->ammo_index] -= 3;
			}	
		else
			return;
      }
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (start);
	gi.WriteDir (forward);
	gi.multicast (start, MULTICAST_PVS);
	gi.sound (ent, CHAN_RELIABLE, gi.soundindex(SND_ASSIM_FIRE), 1, ATTN_STATIC, 0);
}

void Weapon_Assim (edict_t *ent)
{
   // ADDED:
	static int	pause_frames[]	= {15, 24, 30, 0};
   static int	fire_frames[]	= {6, 0};
   Weapon_Generic (ent, 4, 11, 31, 36, pause_frames, fire_frames, Assim_Fire);  	
	if (ent->client->weapon_mode == 2)
		{
		ent->status |= STATUS_IR;
		if (!(level.framenum & 31))
			{
			edict_t *nearby_unit = NULL;
			while ((nearby_unit = findradius(nearby_unit, ent->s.origin, 600)) != NULL)
				{
				if (!nearby_unit->client && !(nearby_unit->svflags & SVF_MONSTER))
		         continue;
				if (nearby_unit == ent)
		         continue;
				if (nearby_unit == ent->client->locked)
		         continue;
				if (!nearby_unit->takedamage)
		         continue;
				if (nearby_unit->health <= 0)
					continue;
				if (!visible(ent, nearby_unit))
					continue;
				if (!infront(ent, nearby_unit))
					continue;
				if (ent->client->locked)
					{	
					ent->client->locked->s.renderfx &= ~RF_SHELL_RED;
					gi.linkentity(ent->client->locked);
					}				
				ent->client->locked = nearby_unit;
				nearby_unit->s.renderfx |= RF_SHELL_RED;
				gi.linkentity(nearby_unit);
				return;
				}
			if (ent->client->locked && !visible(ent, ent->client->locked))
				{
				ent->client->locked->s.renderfx &= ~RF_SHELL_RED;
				gi.linkentity(ent->client->locked);
				ent->client->locked = NULL;
				}
			}
		}
	else
		{
		ent->status &= ~STATUS_IR;
		if (ent->client->locked)
			{
			ent->client->locked->s.renderfx &= ~RF_SHELL_RED;
			gi.linkentity(ent->client->locked);
			ent->client->locked = NULL;
			}
		}
}

///////////////////////////////////
// CEC
///////////////////////////////////
void CEC_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	   {
		G_FreeEdict (self);
		return;
	   }

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage && (other->type & TYPE_MONSTER_METALIC))
	   {      
      other->status |= STATUS_CEC;
      //other->enemy = self;
      other->s.effects |= EF_FLAG1;
      self->nextthink = level.time + 10;
      self->think = CEC_Kill;
      self->enemy = other;
	   self->s.modelindex = 0;
	   self->s.sound = 0;
	   self->movetype = MOVETYPE_NONE;
	   self->solid = SOLID_NOT;
      self->touch = NULL;
      other->chain = self;
	   }
	else
	   {
      gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_LASER_SPARKS);
		gi.WriteByte (16);
		gi.WritePosition (self->s.origin);
		if (!plane)
			gi.WriteDir (vec3_origin);
		else
			gi.WriteDir (plane->normal);
		gi.WriteByte (0xf2f2f0f0);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	   G_FreeEdict (self);
	   }

}

void CEC_think (edict_t *self)
{
   if (self->style-- < 0)
      {
      G_FreeEdict (self);
      return;
      }
	self->nextthink = level.time + FRAMETIME;
   if (self->s.frame++ >= 3)
      {
      self->s.frame = 0;
      }
}

void fire_CEC (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*bolt;
   trace_t tr;

	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLY;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= EF_FLAG1;
	VectorSet(bolt->mins, -16, -16, -16);
	VectorSet(bolt->maxs, 16, 16, 16);
	//bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->s.modelindex = gi.modelindex ("sprites/w_cdc1.sp2");
	bolt->s.sound = gi.soundindex (SND_CEC_BALL);
   bolt->avelocity[2] = 90;
	bolt->owner = self;
	bolt->touch = CEC_touch;
   bolt->nextthink = level.time + FRAMETIME;
   bolt->think = CEC_think;
	bolt->dmg = damage;
   bolt->style = 300;
	gi.linkentity (bolt);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	   {
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	   }
}	

void CEC_Fire (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

   ent->client->ps.gunframe++;
   if (!(ent->client->buttons & BUTTON_ATTACK))
		return;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 10)
	   {
		ent->client->ps.gunframe = 8;
		ent->client->latched_buttons &= ~BUTTON_ATTACK;
		gi.sound(ent, CHAN_VOICE, gi.soundindex(SND_CEC_NOAMMO), 1, ATTN_NORM, 0);
		ent->pain_debounce_time = level.time + 1;
		//NoAmmoWeaponChange (ent);
		return;
	   }

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 15, ent->viewheight);
   P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	ent->client->pers.inventory[ent->client->ammo_index] -= 10;
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_CEC (ent, start, forward, 30, 400, EF_BLASTER);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
   gi.sound (ent, CHAN_WEAPON, gi.soundindex(SND_CEC_FIRE), 1, ATTN_STATIC, 0);
	PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_CECCannon (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 38, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 5, 8, 37, 41, pause_frames, fire_frames, CEC_Fire);
}

void CEC_Kill(edict_t *self)
{
   if (self->enemy && (self->enemy->status & STATUS_CEC))
      {
      T_Damage (self->enemy, self, self->owner, vec3_origin, self->enemy->s.origin, 
               vec3_origin, self->dmg, 1, DAMAGE_ENERGY, MOD_CEC);		    
      self->enemy->s.effects &= ~EF_FLAG1;
      gi.WriteByte (svc_temp_entity);
	   gi.WriteByte (TE_BFG_EXPLOSION);
	   gi.WritePosition (self->enemy->s.origin);
	   gi.multicast (self->enemy->s.origin, MULTICAST_PHS);
      }
   G_FreeEdict (self);
}


///////////////////////////////////
// RIFLE
///////////////////////////////////
void Rifle_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
   gitem_t     *item;

   ent->client->sub_weapon = 0;

	ent->s.frame--;
   if (!ent->client->weapon_mode)
	if ((ent->client->buttons & BUTTON_ATTACK))
	   {
		if (ent->client->machinegun_shots)
         {
         gi.sound(ent, CHAN_WEAPON, gi.soundindex(SND_RIFLE_ZOOM), .3, ATTN_STATIC, 0);
         ent->client->machinegun_shots = 0;
         }
      if (ent->client->beserk < level.framenum && ent->client->ps.fov > 30)
         ent->client->ps.fov-=4;
		return;
	   }

   if (ent->client->weapon_mode == 1)
	if ((ent->client->buttons & BUTTON_ATTACK))
	   {
		if (ent->client->machinegun_shots)
         ent->client->machinegun_shots = 0;
		return;
	   }

   ent->client->ps.gunframe++;
   if (ent->client->weapon_mode == 2)
	   if (!(ent->client->buttons & BUTTON_ATTACK))
         return;

   ent->client->machinegun_shots++;
   if (ent->client->weapon_mode != 2)
	   if (ent->client->machinegun_shots > 1)
	      return;

	if (ent->client->beserk < level.framenum)
		ent->client->ps.fov = atoi(Info_ValueForKey(ent->client->pers.userinfo, "fov"));
   //if (ent->client->flashlight)
      //Cmd_FlashLight(ent, 0); 

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	   {
		ent->client->ps.gunframe = 6;
		gi.sound(ent, CHAN_VOICE, gi.soundindex(SND_RIFLE_NOAMMO), 1, ATTN_NORM, 0);
		ent->pain_debounce_time = level.time + 1;
		//NoAmmoWeaponChange (ent);
		return;
	   }

   // get start / end positions
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 0, ent->viewheight);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
   gi.WriteByte (svc_temp_entity);
   gi.WriteByte (TE_SPARKS);
   gi.WritePosition (start);
   gi.WriteDir (forward);
   gi.multicast (start, MULTICAST_PVS);
   gi.sound (ent, CHAN_RELIABLE, gi.soundindex(SND_RIFLE_FIRE), .6, ATTN_STATIC, 0);
   
   if (ent->client->weapon_mode != 2)
      {
      fire_bullet (ent, start, forward, 30, 200, MOD_RIFLE);
	   if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		   ent->client->pers.inventory[ent->client->ammo_index]--;
		VectorSet(offset, 18, 8, ent->viewheight - 8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
		ThrowShell(ent, start);
      }
   else
      {
      int index, i;
      item = FindItem("Clip");
      index = ITEM_INDEX(item);
      if (!(ent->client->pers.inventory[index] & 3))
			{
			VectorSet(offset, 18, 8, ent->viewheight - 8);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
			ThrowShell(ent, start);
			}
		for (i = 1 ; i < 3 ; i++)
			{
			ent->client->kick_origin[i] = crandom() * 0.35;
			ent->client->kick_angles[i] = crandom() * 0.5;
			}
		ent->client->kick_origin[0] = crandom() * 0.35;
		//ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;
      fire_gun (ent, start, forward, 30, 200, MOD_RIFLE_AUTO);
	   if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		   ent->client->pers.inventory[index]--;
      ent->client->ps.gunframe = 5;
      }

	//PlayerNoise(ent, start, PNOISE_WEAPON);
}

void Weapon_Sniper (edict_t *ent)
{
   // ADDED:
	static int	pause_frames[]	= {8, 15, 0};
   static int	fire_frames[]	= {5, 0};
   Weapon_Generic (ent, 4, 7, 25, 30, pause_frames, fire_frames, Rifle_Fire);  
}


///////////////////////////////////
// WASP
///////////////////////////////////
void Wasp_think(edict_t *self)
{
	int		i;
	edict_t	*e;
	vec3_t forward;
   if (++self->s.frame == 12)
      {
      G_FreeEdict(self);
      return;
      }

	for (i = 1, e = g_edicts+i; i < globals.num_edicts; i++,e++)
		{
		if (!e->inuse)
			continue;
		if (!e->client && !e->monsterinfo.stand)
			continue;
		if (!e->groundentity)
			continue;
		if (e== self->owner)
			continue;

		VectorSubtract (self->s.origin, e->s.origin, forward);
		if (VectorLength(forward) > 400)
			continue;

		e->groundentity = NULL;
		e->velocity[0] += crandom()* 150;
		e->velocity[1] += crandom()* 150;
		e->velocity[2] = self->speed * (100.0 / e->mass);
		}
   self->nextthink = level.time + FRAMETIME;
}

void fire_Wasp (edict_t *self, vec3_t start)
{
	edict_t	*wave, *other = NULL;
    //gi.cprintf (self, PRINT_HIGH, "Making a wave...!\n");

	wave = G_Spawn();
	start[2] += 40;
	VectorCopy (start, wave->s.origin);
	VectorCopy (start, wave->s.old_origin);
	wave->movetype = MOVETYPE_FLY;
	wave->s.effects |= EF_FLAG2;
	wave->s.modelindex = gi.modelindex ("models/objects/waves/tris.md2");
	//wave->s.sound = gi.soundindex ("misc/lasfly.wav");
   wave->avelocity[1] = 180;
   wave->velocity[2] = 30;
	wave->owner = self;
   wave->nextthink = level.time + FRAMETIME;
   wave->think = Wasp_think;
   wave->s.renderfx = RF_TRANSLUCENT;
	gi.linkentity (wave);
   while ((other = findradius(other, self->s.origin, 600)) != NULL)
      {
      if (!other->client && !(other->svflags & SVF_MONSTER))
         continue;
      if (other == self)
         continue;
      if (!other->takedamage)
         continue;
      if (other->health <= 0)
         continue;
      if (!visible(self, other))
         continue;
      T_Damage (other, self, self, vec3_origin, vec3_origin, vec3_origin, 90, 0, DAMAGE_NO_KNOCKBACK, MOD_WASP);
      }
}	

void Wasp_Fire (edict_t *ent)
{
	int	i;

   ent->client->ps.gunframe++;
   if (!(ent->client->buttons & BUTTON_ATTACK))
		return;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 10)
	   {
		ent->client->ps.gunframe = 19;
		if (level.time >= ent->pain_debounce_time)
		   {
			ent->client->latched_buttons &= ~BUTTON_ATTACK;			
			gi.sound(ent, CHAN_VOICE, gi.soundindex(SND_WASP_NOAMMO), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		   }
		//NoAmmoWeaponChange (ent);
		return;
	   }
   
   if (ent->client->ps.gunframe == 7)
      {
      gi.sound(ent, CHAN_WEAPON, gi.soundindex(SND_WASP_HEATUP), 1, ATTN_NORM, 0);
      return;
      }


	for (i = 1 ; i < 3 ; i++)
	   {
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	   }
	ent->client->kick_origin[0] = -4;
	ent->client->kick_angles[0] = -15;

	// get start / end positions
   fire_Wasp (ent, ent->s.origin);
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 10;
   gi.WriteByte (svc_temp_entity);
   gi.WriteByte (TE_SPARKS);
   gi.WritePosition (ent->s.origin);
   gi.WriteDir (vec3_origin);
   gi.multicast (ent->s.origin, MULTICAST_PVS);
   gi.sound (ent, CHAN_WEAPON, gi.soundindex(SND_WASP_FIRE), 1, ATTN_NORM, 0);
   ent->client->ps.gunframe = 15;
}

void Weapon_Wasp (edict_t *ent)
{
   // ADDED:
	static int	pause_frames[]	= {27, 34, 41, 0};
   static int	fire_frames[]	= {6, 18, 0};
   Weapon_Generic (ent, 4, 19, 49, 54, pause_frames, fire_frames, Wasp_Fire);  
}


///////////////////////////////////
// STAFF
///////////////////////////////////	
void Staff_Fire_Fireball (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	vec3_t		offset;
 
   ent->client->ps.gunframe++;
	if (ent->client->pers.inventory[ent->client->ammo_index] < 5)
		return;
	for (i = 1 ; i < 3 ; i++)
	   {
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	   }
	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 18, 0, 0); //ent->viewheight - 4
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
   Staff_fireball (ent, start, forward, 60, 750, 70, 70); 
   if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 5;
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (start);
	gi.WriteDir (forward);
   gi.multicast (start, MULTICAST_PVS);
   gi.sound (ent, CHAN_RELIABLE, gi.soundindex(SND_STAFF_FIRE), 1, ATTN_STATIC, 0);
}


// shogun: lightning mode
void Staff_Fire_Lightning (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	vec3_t		offset;

	ent->client->ps.gunframe++;
	if (!(ent->client->buttons & BUTTON_ATTACK))
      return;
	if (ent->client->pers.inventory[ent->client->ammo_index] <= 0)
		return;
	ent->client->ps.gunframe--;
	ent->s.frame--;

	for (i = 1 ; i < 3 ; i++)
	   {
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	   }
	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 30, 0, ent->viewheight-4);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);   
   if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
   Staff_Lightning (ent, start, forward, 40); 
   gi.sound (ent, CHAN_RELIABLE, gi.soundindex(SND_STAFF_LIGHTNING), 1, ATTN_STATIC, 0);
}

// shogun: choices in here for which mode, just a hack for now
void Staff_Fire(edict_t *ent)
{
	if(!ent->client->weapon_mode)
		Staff_Fire_Fireball(ent);
	else
		Staff_Fire_Lightning(ent);
}

void Staff_fireball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
   rocket->s.modelindex = gi.modelindex ("models/objects/fireball/tris.md2");
   rocket->s.effects |= EF_ROCKET;
     rocket->s.renderfx = RF_TRANSLUCENT;
   rocket->touch = rocket_touch;
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
   rocket->avelocity[2] = 180;
   rocket->nextthink = level.time + 8000/speed;
	rocket->owner = self;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex (SND_STAFF_FIRE);
	rocket->classname = "fireball";
	gi.linkentity (rocket);
	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);
}

// shogun: lightning casting
void Staff_Lightning (edict_t *self, vec3_t start, vec3_t dir, int damage)
{
	trace_t tr;
	vec3_t end;
	edict_t *k = Ent1;
	
	// find something to damage
	VectorMA(start, 4096, dir, end);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	VectorScale(dir, -1, dir);
	if (tr.fraction < 1)
		{
		if (tr.ent->takedamage)
			T_Damage (tr.ent, self, self, vec3_origin, tr.ent->s.origin, vec3_origin, damage, 1, 0, MOD_STAFF);
		else
			{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_SPARKS);
			gi.WritePosition (tr.endpos);
			gi.WriteDir (dir);
			gi.multicast (tr.endpos, MULTICAST_PVS);
			}
		}

		VectorCopy(start, k->s.origin);
		gi.WriteByte(svc_temp_entity);
		gi.WriteByte(TE_PARASITE_ATTACK);
		gi.WriteShort(k - g_edicts);
		gi.WritePosition(k->s.origin);
		gi.WritePosition(tr.endpos);
		gi.multicast(k->s.origin, MULTICAST_PVS);

		check_dodge (self, self->s.origin, dir, 600);
}

void Weapon_Staff (edict_t *ent)
{
   // ADDED:
	static int	pause_frames[]	= {21, 31, 41, 0};
   static int	fire_frames[]	= {12, 0};
   if (!(level.framenum & 7))
		if (ent->client->pers.inventory[ent->client->ammo_index] 
			< ent->client->pers.max_liquidfire)
		ent->client->pers.inventory[ent->client->ammo_index]++;
   Weapon_Generic (ent, 8, 17, 40, 44, pause_frames, fire_frames, Staff_Fire);  
}

void heat_seek (edict_t *self)
{
	if (self->enemy)
		{
		vec3_t forward;		
		VectorSubtract(self->enemy->s.origin, self->s.origin, forward);
		//VectorCopy (forward, self->movedir);
		VectorNormalize(forward);
		vectoangles (forward, self->s.angles);
		VectorScale(self->velocity, .8, self->velocity);
		VectorScale(forward, self->speed, forward);
		VectorAdd(self->velocity, forward, self->velocity);
		self->nextthink = level.time + FRAMETIME;
		return;
		}
	BecomeExplosion1(self);
}

void fire_missile (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;
   trace_t tr;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYARC;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rockets/tris.md2");
	rocket->owner = self;
	rocket->avelocity[2] = 440;
	rocket->touch = rocket_touch;
	rocket->speed = speed;
	if (self->client && self->client->locked)
		{
		rocket->enemy = self->client->locked;
		rocket->nextthink = level.time + FRAMETIME;
		rocket->think = heat_seek;
		}
	else
		{
		rocket->nextthink = level.time + 8000/speed;
		rocket->think = G_FreeEdict;
		}
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "missile";
   
   rocket->gravity = .1;
	gi.linkentity (rocket);

	if (self->client)
		check_dodge (self, rocket->s.origin, dir, speed);

   tr = gi.trace (self->s.origin, NULL, NULL, rocket->s.origin, rocket, MASK_SHOT);
	if (tr.fraction < 1.0)
	   {
		VectorMA (rocket->s.origin, -10, dir, rocket->s.origin);
		rocket->touch (rocket, tr.ent, NULL, NULL);
	   }
}

void fire_gattling (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed)
{
	edict_t	*bolt;
	trace_t	tr;

   gi.sound (self, CHAN_WEAPON, gi.soundindex(SND_TRIGAT_FIRE), 1, ATTN_STATIC, 0);
	VectorNormalize (dir);

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= EF_COLOR_SHELL | EF_BOOMER;
	bolt->s.renderfx |= RF_TRANSLUCENT; /*RF_SHELL_BLUE | RF_SHELL_GREEN;*/
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 2;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->spawnflags = 1;
	gi.linkentity (bolt);

	check_dodge (self, bolt->s.origin, dir, speed);

	tr = gi.trace (self->s.origin, NULL, NULL, bolt->s.origin, bolt, MASK_SHOT);
	if (tr.fraction < 1.0)
	   {
		VectorMA (bolt->s.origin, -10, dir, bolt->s.origin);
		bolt->touch (bolt, tr.ent, NULL, NULL);
	   }
}	

///////////////////////////////////
// TEARGAS
///////////////////////////////////
static void Teargas_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	   {
		G_FreeEdict (ent);
		return;
	   }

	if (!other->takedamage || (ent->owner->team_data == other->team_data))
	   {
		if (ent->spawnflags & 1)
		   {
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		   }
		else
		   {
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		   }
		return;
	   }

	ent->enemy = other;
	Teargas_Explode (ent);
}

static void Teargas_Explode (edict_t *ent)
{
	//vec3_t	origin;
   float	   points;
	vec3_t	v;
	vec3_t	dir;
	edict_t	*nearby = NULL;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	   {

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, MOD_TEARGAS);
	   }


	while ((nearby = findradius(nearby, ent->s.origin, ent->dmg_radius)) != NULL)
	   {
		if (!nearby->takedamage)
			continue;
		if (!nearby->client)
			continue;
		if (!nearby->client->SPEC_data & SPEC_GASMASK)
			continue;
      if ((nearby->team_data == ent->owner->team_data) && (nearby != ent->owner))
			continue;      

		VectorSubtract (ent->s.origin, nearby->s.origin, v);
      nearby->client->teargas_time = 18 - (VectorLength (v)/20) + level.time;
      }

   gi.sound (ent, CHAN_VOICE, gi.soundindex("world/electro.wav"), 1, ATTN_STATIC, 0);
   ent->nextthink = level.time + 5;
   ent->think = G_FreeEdict;
}

void fire_teargas (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = Teargas_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Teargas_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "teargas";
	gi.linkentity (grenade);
}

///////////////////////////////////
// FLASHLIGHT
///////////////////////////////////
void Cmd_FlashLight(edict_t *self, int light) 
{
   vec3_t  start,
           forward,
           right,
           end;   
   if (self->client->flashlight)
      {
      gi.unlinkentity(self->client->flashlight);
      G_FreeEdict(self->client->flashlight);
      self->client->flashlight = NULL;
      return;
      }
   AngleVectors (self->client->v_angle, forward, right, NULL);
   VectorSet(end, 100, 0, 0);
   G_ProjectSource (self->s.origin, end, forward, right, start);
   self->client->flashlight = G_Spawn ();
   self->client->flashlight->owner = self;
   self->client->flashlight->movetype = MOVETYPE_NOCLIP;
   self->client->flashlight->solid = SOLID_NOT;
   self->client->flashlight->classname = "flashlight";   
   self->client->flashlight->s.modelindex = gi.modelindex ("models/objects/crosshair/tris.md2");   
   if (light)
      {
      self->client->flashlight->s.effects |= EF_HYPERBLASTER;
      self->client->flashlight->s.renderfx = RF_TRANSLUCENT | RF_BEAM;
      }
   self->client->flashlight->think = FlashLightThink;
   self->client->flashlight->nextthink = level.time + FRAMETIME;
   gi.linkentity(self->client->flashlight);
}

void FlashLightThink (edict_t *self)
{
   vec3_t start,
          end,
          endp,
          offset;
   vec3_t forward,
          right,
          up;
   trace_t tr;
   AngleVectors (self->owner->client->v_angle, forward, right, up);
   VectorSet(offset, 0 , 0, self->owner->viewheight);
	P_ProjectSource (self->owner->client, self->owner->s.origin, offset, forward, right, start);
	VectorMA (start, 8192, forward, end);
   tr = gi.trace (start, NULL, NULL, end, self->owner,
                  CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
   if (tr.fraction != 1) 
     {
     VectorMA(tr.endpos, -15, forward, endp); //-4
     VectorCopy(endp, tr.endpos);
     }
   vectoangles(tr.plane.normal, self->s.angles);
   self->s.angles[0]++;
   VectorCopy(tr.endpos, self->s.origin);
   gi.linkentity(self);
   self->nextthink = level.time + FRAMETIME;
}

