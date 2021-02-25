#include "g_local.h"
#include "g_main.h"

//Knightmare added
#include "g_sogcvar.h"
#include "m_player.h"

// Skid added
#include "g_sogutil.h"
#include "m_sogblaze.h"
#include "m_sogflynn.h"
#include "m_sogaxe.h"
// End Skid

extern qboolean	is_quad;

//======================================================================
//======================================================================
//
//   Quake1 and Doom Weapon routines -Skid
//
//======================================================================
//======================================================================

/*
//======================================================================
//======================================================================
OLD NoAmmoWeaponChange -Skid

called by Doom and Quake1 weapon funtions when out of ammo
//======================================================================
//======================================================================
*/

void Q1_NoAmmoWeaponChange (edict_t *ent)
{
	if ((ent->waterlevel <=1) &&
		ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] &&
	    ent->client->pers.inventory[ITEM_INDEX(FindItem("discharger"))] )
	{
		ent->client->newweapon = FindItem ("discharger");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] > 1
	&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("heavy spiker"))] )
	{
		ent->client->newweapon = FindItem ("heavy spiker");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] > 1
	&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("sawed-off shotgun"))] )
	{
		ent->client->newweapon = FindItem ("sawed-off shotgun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
	&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("spiker"))] )
	{
		ent->client->newweapon = FindItem ("spiker");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))]
	&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Boomstick"))] )
	{
		ent->client->newweapon = FindItem ("boomstick");
		return;
	}
	else
		ent->client->newweapon = FindItem ("axe");
}


void Doom_NoAmmoWeaponChange (edict_t *ent)
{
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] > 1
	&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Double Barreled Shotgun"))] )
		{
		ent->client->newweapon = FindItem ("Double Barreled Shotgun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))]
	&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("RL3000"))] )
	{
		ent->client->newweapon = FindItem ("RL3000");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] > 40
	&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Goddamn Big Gun"))] )
	{
		ent->client->newweapon = FindItem ("Goddamn Big Gun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]
	&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Plasma Rifle"))] )
	{
		ent->client->newweapon = FindItem ("Plasma Rifle");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
	&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Mini Gun"))] )
	{
		ent->client->newweapon = FindItem ("Mini Gun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("shells"))]
	&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Combat Shotgun"))] )
	{
		ent->client->newweapon = FindItem ("Combat Shotgun");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
	&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Pistol"))] )
	{
		ent->client->newweapon = FindItem ("Pistol");
		return;
	}
	if(	ent->client->pers.inventory[ITEM_INDEX(FindItem ("Chainsaw"))])
	{
		ent->client->newweapon = FindItem ("Chainsaw");
		return;
	}
	else
		ent->client->newweapon = FindItem ("Fists");

}

void Wolf_NoAmmoWeaponChange (edict_t *ent)
{
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))]
	&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("bazooka"))] )
	{
		ent->client->newweapon = FindItem ("bazooka");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))]
	&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("flamethrower"))] )
	{
		ent->client->newweapon = FindItem ("flamethrower");
		return;
	}
	
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] > 3
	&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("gatling guns"))] )
	{
		ent->client->newweapon = FindItem ("gatling guns");
		return;
	}
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] > 1
	&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("walter p-38"))] )
	{
		ent->client->newweapon = FindItem ("walter p-38");
		return;
	}
	else
		ent->client->newweapon = FindItem ("knife");
}

void Oldie_NoAmmoWeaponChange( edict_t *ent)
{
	switch(ent->client->resp.player_class)
	{
		case CLASS_WOLF:
			Wolf_NoAmmoWeaponChange (ent);
			break;
		case CLASS_DOOM:
			Doom_NoAmmoWeaponChange(ent);
			break;
		case CLASS_Q1:
			Q1_NoAmmoWeaponChange(ent);
			break;
	}
}



/*
================
Weapon_Oldie QUAKE

Generic weapon function to handle the basics of weapon thinking
Needed new to support Older Weapon Animations -Skid
================
*/

void Weapon_Q1(edict_t *ent, int FRAME_FIRST, int FRAME_LAST, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;

	if(ent->deadflag || 
	  ((ent->s.modelindex != 255) && (!ent->flags & FL_INVIS))) 
		return;
	
	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = FRAME_FIRST;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;

		if(ITEM_INDEX(ent->client->pers.weapon) == Q1_AXE)
		{
			ent->client->anim_priority = ANIM_REVERSE;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain4+1;
				ent->client->anim_end = FRAME_crpain1;
			}
			else
			{
				ent->s.frame = FRAME_pain304+1;
				ent->client->anim_end = FRAME_pain301;
			}
		}

		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if ((!ent->client->ammo_index) || 
				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
			{
				ent->client->ps.gunframe = FRAME_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
			
				// start the animation - if weapon isnt Axe
				if(ITEM_INDEX(ent->client->pers.weapon)!= Q1_AXE)
				{
					ent->client->anim_priority = ANIM_ATTACK;
					if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
					{
						ent->s.frame = FRAME_crattak1-1;
						ent->client->anim_end = FRAME_crattak9;
					}
					else
					{
						ent->s.frame = FRAME_attack1-1;
						ent->client->anim_end = FRAME_attack8;
					}
				}
			}
			else
			{
				ent->client->weaponstate = WEAPON_DROPPING;
				Oldie_NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->oldweapon =0.0;
			
			if(ent->client->ps.gunframe < FRAME_FIRST)
				ent->client->ps.gunframe++;
			else
			{
				if (ent->client->anim_run)	
					ent->client->ps.gunframe = 0;
				else
				{
					if(ent->client->ps.gunframe < FRAME_FIRST)
						ent->client->ps.gunframe++;
				}
			}
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{   
		if ((ent->client->pers.inventory[ent->client->ammo_index] < ent->client->pers.weapon->quantity) &&
			(ITEM_INDEX(ent->client->pers.weapon)!=Q1_AXE))
		{
			ent->client->ps.gunframe = FRAME_FIRST;
			ent->client->weaponstate = WEAPON_DROPPING;
			Oldie_NoAmmoWeaponChange (ent);
			return;
		}

		if(ent->client->ps.gunframe < FRAME_FIRST)
			ent->client->ps.gunframe= FRAME_FIRST;

		ent->client->ps.gunframe++;

		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if(ent->client->super_damage_finished < level.time)
				{
					if (!CTFApplyStrengthSound(ent))
					
					if (ent->client->quad_framenum > level.framenum)
						gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
					
					if(sog_ctf->value)           
						CTFApplyHasteSound(ent);

					ent->client->super_damage_finished = level.time + 1;
				}
				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
		{
			if (ent->client->ps.gunframe >= FRAME_LAST)
			{
	   			ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = FRAME_FIRST;
				return;
			}
		}
	}
}

//================
//Quake Weapon bobbing
//================

void Weapon_Quake (edict_t *ent, int FRAME_FIRST, int FRAME_LAST, int *fire_frames, void (*fire)(edict_t *ent))
{
	int oldstate = ent->client->weaponstate;

	if (ent->s.modelindex != 255 &&
		!(ent->flags & FL_INVIS)) 
		return; 

	Weapon_Q1 (ent,FRAME_FIRST, FRAME_LAST,fire_frames, fire);

	if(sog_ctf->value)
	if (CTFApplyHaste(ent) && oldstate == ent->client->weaponstate) 
	{
		Weapon_Q1 (ent, FRAME_FIRST, FRAME_LAST, fire_frames, fire);
	}
}

/*
======================================================================
QUAKE 1 WEAPONS -  Skid
most are straightforward
======================================================================
/*
======================================================================
AXE
======================================================================
*/

void weapon_q1_axe_fire (edict_t *ent)
{
	vec3_t		start,
				forward,
				right,
				offset;
	int			damage=20;
		
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if(ent->client->ps.gunframe == 10 || ent->client->ps.gunframe == 16) 
		{
			ent->client->ps.gunframe= 5; 
			ent->client->weaponstate = WEAPON_READY;
			return;
		}
	}
	
	if(ent->client->ps.gunframe == 9 || ent->client->ps.gunframe == 14)
	{
		if (is_quad)
			damage *= 4;
	
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 8, 0, ent->viewheight - 4); 
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		
		gi.sound (ent, CHAN_RELIABLE|CHAN_WEAPON, gi.soundindex("idg3weapons/axe/ax1.wav"), 1.0, ATTN_NORM, 0);
		q1_fire_axe(ent, start, forward, damage);
	}
}

void Weapon_Q1_Axe (edict_t *ent)
{
	static int	fire_frames[]	= {6,7,8,9,10,11,12,13,14,15,16,0};  
	Weapon_Q1 (ent, 5, 16, fire_frames , weapon_q1_axe_fire);
	
	if(ent->client->weaponstate == WEAPON_FIRING && 
	  !(ent->flags & FL_INVIS) &&
	  (ent->client->ps.gunframe == 6 || ent->client->ps.gunframe == 10))
	{
		ent->client->anim_priority = ANIM_ATTACK;
		

		// Figure out next SWING animation to play
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_axe_crattak1-1;
			ent->client->anim_end = FRAME_axe_crattak6;
		}
		else 
		{
			float anim=random();
			if (anim < 0.25)
			{
				ent->s.frame = FRAME_axe_A_attack1 - 1;
				ent->client->anim_end = FRAME_axe_A_attack6;
			}
			else if (anim < 0.5)
			{
				ent->s.frame = FRAME_axe_B_attack1 - 1;
				ent->client->anim_end = FRAME_axe_B_attack6;
			}
			else if (anim < 0.75)
			{
				ent->s.frame = FRAME_axe_C_attack1 - 1;
				ent->client->anim_end = FRAME_axe_C_attack6;
			}
			else
			{	
				ent->s.frame = FRAME_axe_D_attack1 - 1;
				ent->client->anim_end = FRAME_axe_D_attack6;
			}
		}
	
	}
}



/*
======================================================================

Q1 SG

======================================================================
*/

void weapon_q1_sg_fire (edict_t *ent)
{
	vec3_t		start,
				forward, 
				right,
				offset;
	int			damage = 4;

	if(!deathmatch->value)
		damage = 5;
	
	if (is_quad)
		damage *= 4;
	
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 10, 0,  ent->viewheight-8);
 	P_ProjectSource(ent->client,ent->s.origin, offset, forward, right, start);

	//fire_shotty 
	fire_shotgun(ent, start, forward, damage, damage, DEFAULT_Q1_SG_HSPREAD,DEFAULT_Q1_SG_VSPREAD, DEFAULT_Q1_SG_COUNT, MOD_Q1_SG);
		
	if(ent->client->chasetoggle)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_SHOTGUN2);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_SHOTGUN2);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	gi.sound (ent, CHAN_AUTO, gi.soundindex("idg3weapons/sshotgun/shotgn2.wav"), 1.0, ATTN_NORM, 0);

	
	if (!((int)dmflags->value & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Q1_SG (edict_t *ent)
{
	static int	fire_frames[]	= {6,0}; 
	Weapon_Quake (ent, 5, 10, fire_frames , weapon_q1_sg_fire);  //11
}


/*
======================================================================

Q1 SSG

======================================================================
*/


void weapon_q1_ssg_fire (edict_t *ent)
{
	vec3_t		start,
				forward,
				right,
				offset,
				v;
	int			damage = 4;

	if(!deathmatch->value)
		damage = 5;
	
	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -5; 

	VectorSet(offset, 8, 0,  ent->viewheight-8);
 	P_ProjectSource(ent->client,ent->s.origin, offset, forward, right, start);

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 3.0;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	//fire_shotty 
	fire_shotgun(ent, start, forward, damage, damage, 
		DEFAULT_Q1_SG_HSPREAD, 	DEFAULT_Q1_SG_VSPREAD, DEFAULT_Q1_SSG_COUNT, MOD_Q1_SSG);
	
	v[YAW]   = ent->client->v_angle[YAW] + 3.0;
	AngleVectors (v, forward, NULL, NULL);
	//fire_shotty 
	fire_shotgun(ent, start, forward, damage, damage, 
		DEFAULT_Q1_SG_HSPREAD,DEFAULT_Q1_SG_VSPREAD, DEFAULT_Q1_SSG_COUNT, MOD_Q1_SSG);

	if(ent->client->chasetoggle)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	gi.sound (ent, CHAN_AUTO, gi.soundindex("idg3weapons/sshotgun/shotgn2.wav"), 1.0, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_Q1_SSG (edict_t *ent)
{
	static int	fire_frames[]	= {6,0}; 
	Weapon_Quake (ent, 5, 12, fire_frames , weapon_q1_ssg_fire); 
}


/*
======================================================================

Q1 NG

======================================================================
*/

void weapon_q1_ng_fire (edict_t *ent)
{
	vec3_t	offset,
			forward,
			right,
			start;
	int		damage=6;
	
		
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe= 5; 
		ent->client->weaponstate = WEAPON_READY;
		gi.sound (ent, CHAN_AUTO, gi.soundindex("idg3weapons/nails/r_end.wav"), 1.0, ATTN_NORM, 0);
		return;

	}
	else
	{
		if (ent->client->pers.inventory[ent->client->ammo_index] < 1) 
		{	
			ent->client->ps.gunframe = 5; 
			ent->client->weaponstate = WEAPON_DROPPING;
			gi.sound (ent, CHAN_AUTO, gi.soundindex("idg3weapons/nails/r_end.wav"), 1.0, ATTN_NORM, 0);
			Oldie_NoAmmoWeaponChange (ent);
			return;
		}

		if(!deathmatch->value)
			damage = 10;

		if (is_quad)
			damage *= 4;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24 , 0, ent->viewheight-7);
		
		// So nails spawn from the 2 barrels of the gun instead of centered -Skid
		if ( (ent->client->ps.gunframe %2) == 0)
		{
			ent->client->kick_angles[0] = -2;
			offset[1]= 8;
		}
		else
		{
			offset[1]= 2;
			ent->client->kick_angles[0] = -1;
		}

		P_ProjectSource(ent->client,ent->s.origin, offset, forward, right, start);

		if(ent->client->chasetoggle)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent->client->oldplayer-g_edicts);
			gi.WriteByte (MZ_ETF_RIFLE);
			gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
		}
		else
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_ETF_RIFLE);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
		
		
		// PlayerNoise(ent, start, PNOISE_WEAPON);

		gi.sound (ent, CHAN_AUTO, gi.soundindex("idg3weapons/nails/spike.wav"), 1.0, ATTN_NORM, 0);


		q1_fire_nail(ent, start, forward, damage, 900, false);

		ent->client->anim_priority = ANIM_ATTACK;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crattak1 - 1;
			ent->client->anim_end = FRAME_crattak9;
		}
		else
		{
			ent->s.frame = FRAME_attack1 - 1;
			ent->client->anim_end = FRAME_attack8;
		}
	
		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index]--;
		
		if((ent->client->ps.gunframe == 13) && 
		   (ent->client->pers.inventory[ent->client->ammo_index]))
		{
			if(ent->client->buttons & BUTTON_ATTACK)
				ent->client->ps.gunframe = 5;
			else
				gi.sound (ent, CHAN_AUTO, gi.soundindex("idg3weapons/nails/r_end.wav"), 1.0, ATTN_NORM, 0);
		}
	}
}


void Weapon_Q1_NG (edict_t *ent)
{
	static int	fire_frames[]	= {6,7,8,9,10,11,12,13,0}; 
	Weapon_Q1 (ent, 5, 13, fire_frames , weapon_q1_ng_fire); 
}


/*
======================================================================

Q1 SUPER NG

======================================================================
*/


void weapon_q1_sng_fire (edict_t *ent)
{
	vec3_t	offset,
			forward,
			right,
			start;
	int		damage=9;
	
		
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe=5; 
		ent->client->weaponstate = WEAPON_READY;
		gi.sound (ent, CHAN_AUTO, gi.soundindex("idg3weapons/nails/s_end.wav"), 1.0, ATTN_NORM, 0);
		return;

	}
	else
	{
		if (ent->client->pers.inventory[ent->client->ammo_index] < 2) 
		{	
			ent->client->ps.gunframe = 5;
			ent->client->weaponstate = WEAPON_DROPPING;
			gi.sound (ent, CHAN_AUTO, gi.soundindex("idg3weapons/nails/s_end.wav"), 1.0, ATTN_NORM, 0);
			Oldie_NoAmmoWeaponChange (ent);
			return;
		}

		if(!deathmatch->value)
			damage = 20;

		if (is_quad)
			damage *= 4;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
				
		if ( (ent->client->ps.gunframe %2) == 0)
			ent->client->kick_angles[0] = -2; 
		else
			ent->client->kick_angles[0] = -1;

		VectorSet(offset, 24 , 7, ent->viewheight-6);
		
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		P_ProjectSource(ent->client,ent->s.origin, offset, forward, right, start);
					
		q1_fire_nail(ent, start, forward, damage, 1000, true);

		if(ent->client->chasetoggle)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent->client->oldplayer-g_edicts);
			gi.WriteByte (MZ_PHALANX);
			gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
		}
		else
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_PHALANX);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
		//gi.sound (ent, CHAN_AUTO, gi.soundindex("q1weap/nails/spike.wav"), 1.0, ATTN_NORM, 0);

		gi.sound (ent, CHAN_AUTO, gi.soundindex("idg3weapons/nails/spike.wav"), 1.0, ATTN_NORM, 0);

		
		ent->client->anim_priority = ANIM_ATTACK;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crattak1 - 1;
			ent->client->anim_end = FRAME_crattak9;
		}
		else
		{
			ent->s.frame = FRAME_attack1 - 1;
			ent->client->anim_end = FRAME_attack8;
		}
	
		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index] -= 2;

		if(ent->client->ps.gunframe == 23 && 
		  (ent->client->pers.inventory[ent->client->ammo_index] >= 2))
		{
			if(ent->client->buttons & BUTTON_ATTACK)
				ent->client->ps.gunframe = 5;
			else
				gi.sound (ent, CHAN_AUTO, gi.soundindex("idg3weapons/nails/s_end.wav"), 1.0, ATTN_NORM, 0);
		}
	}
}


void Weapon_Q1_SNG (edict_t *ent)
{
	static int	fire_frames[]	= {6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,0}; 
	Weapon_Q1 (ent, 5, 23, fire_frames , weapon_q1_sng_fire); 
}

/*
======================================================================

Q1 GRENADELAUNCHER

======================================================================
*/


void weapon_q1_gl_fire (edict_t *ent)
{
	vec3_t	offset,
			forward,
			right,
			start;
	int		damage = 105;
	float	radius = 130.0;
	
	if (is_quad)
		damage *= 4;

    AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 10, 8, ent->viewheight-8);
	P_ProjectSource(ent->client,ent->s.origin, offset, forward, right, start);
	
	q1_fire_grenade (ent, start, forward, damage, 600, 3.0, radius);

	if(ent->client->chasetoggle)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	gi.sound (ent, CHAN_WEAPON|CHAN_RELIABLE, gi.soundindex("idg3weapons/grenades/grenade.wav"), 1.0, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Q1_GL (edict_t *ent)
{
	static int	fire_frames[]	= {6,0}; 
	Weapon_Quake (ent, 5, 11, fire_frames , weapon_q1_gl_fire); 
}



/*
======================================================================

Q1 ROCKET

======================================================================
*/

void weapon_q1_rl_fire (edict_t *ent)
{
	vec3_t	offset,
			start,
			forward,
			right;
	int		damage;
	int		radius_damage= 115;
	float	damage_radius= 130.0; 

	damage = 70 + (int)(random() * 20);

	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2; 
	
	VectorSet(offset,10,8,ent->viewheight-8);
    
	P_ProjectSource(ent->client,ent->s.origin, offset, forward, right, start);
	q1_fire_rocket (ent, start, forward, damage, 950, damage_radius, radius_damage); //1000
	
	if(ent->client->chasetoggle)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_IONRIPPER);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_IONRIPPER);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	if (! ((int)dmflags->value & DF_INFINITE_AMMO ))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Q1_RL (edict_t *ent)
{
	static int	fire_frames[]	= {6,0}; 
	Weapon_Quake (ent, 5, 13, fire_frames , weapon_q1_rl_fire); 
}


/*
======================================================================

LIGHTNING GUN

======================================================================
*/

void weapon_q1_lg_fire (edict_t *ent)
{
	vec3_t		start,
				forward,
				right,
				offset;
	int			damage=22;
	int			cells;

		
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe= 5;
		ent->client->weaponstate = WEAPON_READY;
		ent->client->oldweapon=0.0;
		return;
	}
	else
	{     
		if (ent->client->pers.inventory[ent->client->ammo_index] < 1) 
		{	
			ent->client->ps.gunframe = 5;
			ent->client->weaponstate = WEAPON_DROPPING;
			ent->client->oldweapon =0.0;
			Oldie_NoAmmoWeaponChange (ent);
			return;
		}
		

		if (ent->waterlevel >= 2 ) 
		{
			float	points;
			edict_t	*hent = NULL;
			vec3_t	v;
			vec3_t	dir;
			int damage;
			int radius;

			cells = ent->client->pers.inventory[ent->client->ammo_index];
			ent->client->pers.inventory[ent->client->ammo_index] = 0;
			ent->client->oldweapon =0.0;

			damage= cells*35;
			radius = damage/2;

			
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_MACHINEGUN | 128);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
			
			gi.sound (ent,CHAN_WEAPON|CHAN_RELIABLE , gi.soundindex("idg3weapons/lightning/lstart.wav"), 1.0, ATTN_NORM, 0);

			while ((hent = findradius(hent, ent->s.origin, radius)) != NULL)
			{
				if (!hent->takedamage)
					continue;
				if (!hent->waterlevel)
					continue;

				VectorAdd (hent->mins, ent->maxs, v);
				VectorMA (hent->s.origin, 0.5, v, v);
				VectorSubtract (ent->s.origin, v, v);
				points = damage - 0.5 * VectorLength (v);
				if (hent == ent)
					points = points * 0.5;
				if (points > 0)
				{
					if (CanDamage (hent, ent))
					{
						VectorSubtract (hent->s.origin, ent->s.origin, dir);
						T_Damage (hent, ent, ent, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, MOD_Q1_LG_SPLASH);
					}
				}
			}
			return;
		}
		else
		{
			if (is_quad)
				damage *= 4;

			AngleVectors (ent->client->v_angle, forward, right, NULL);
			if ( (ent->client->ps.gunframe %2) == 0)
				ent->client->kick_angles[0] =  1;
			else
				ent->client->kick_angles[0] = -1;

			VectorSet(offset,10, 8, ent->viewheight-8); 
			G_ProjectSource(ent->s.origin, offset, forward, right, start);
			q1_fire_lightning(ent, start, forward, damage);
		
			if(!ent->client->oldweapon) 
			{
				if(ent->client->chasetoggle)
				{
					gi.WriteByte (svc_muzzleflash);	
					gi.WriteShort (ent->client->oldplayer-g_edicts);
					gi.WriteByte (MZ_MACHINEGUN | 128);
					gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
				}
				else
				{	
					gi.WriteByte (svc_muzzleflash);
					gi.WriteShort (ent-g_edicts);
					gi.WriteByte (MZ_MACHINEGUN | 128);
					gi.multicast (ent->s.origin, MULTICAST_PVS); 
				}
				
				gi.sound (ent,CHAN_AUTO, gi.soundindex("idg3weapons/lightning/lstart.wav"), 1.0, ATTN_NORM, 0);
				gi.sound (ent,CHAN_WEAPON|CHAN_RELIABLE, gi.soundindex("idg3weapons/lightning/lhit.wav"), 0.8 , ATTN_NORM, 0);
				ent->client->oldweapon = level.time + 0.6;
			}
			else if(ent->client->oldweapon < level.time) 
			{
				if(ent->client->chasetoggle)
				{
					gi.WriteByte (svc_muzzleflash);	
					gi.WriteShort (ent->client->oldplayer-g_edicts);
					gi.WriteByte (MZ_MACHINEGUN | 128);
					gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
				}
				else
				{				
					gi.WriteByte (svc_muzzleflash);
					gi.WriteShort (ent-g_edicts);
					gi.WriteByte (MZ_MACHINEGUN | 128);
					gi.multicast (ent->s.origin, MULTICAST_PVS);
				}
				gi.sound (ent,CHAN_WEAPON|CHAN_RELIABLE, gi.soundindex("idg3weapons/lightning/lhit.wav"), 0.8 , ATTN_NORM, 0);
				ent->client->oldweapon = level.time + 0.6;
			}
		
			ent->client->anim_priority = ANIM_ATTACK;
			if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crattak1 - 1;
				ent->client->anim_end = FRAME_crattak9;
			}
			else	
			{
				ent->s.frame = FRAME_attack1 - 1;
				ent->client->anim_end = FRAME_attack8;
			}
	
			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
				ent->client->pers.inventory[ent->client->ammo_index] --;
		}
	}

	if(ent->client->ps.gunframe == 9 && ent->client->pers.inventory[ent->client->ammo_index])
	{
		if(ent->client->buttons & BUTTON_ATTACK)
			ent->client->ps.gunframe = 6;
		else
			ent->client->oldweapon =0.0;
	}
}

void Weapon_Q1_LG (edict_t *ent)
{
	static int	fire_frames[]	= {6,7,8,9,0};
	Weapon_Quake (ent, 5, 9, fire_frames , weapon_q1_lg_fire);
}



/*
======================================================================

DOOM Weapons -Skid

======================================================================
*/


/*
================
Weapon_Oldie DOOM

Generic weapon function to handle the basics of weapon thinking
Needed new to support Older Weapon Animations -Skid
================
*/

#define DOOM_UP_LAST	3
#define DOOM_DOWN_FIRST	4
#define DOOM_DOWN_LAST	7
#define DOOM_STATIONARY 8
#define DOOM_SWAY_FIRST	11
#define DOOM_SWAY_LAST	22
#define DOOM_MOVE_L1    23
#define DOOM_MOVE_L2    24
#define DOOM_MOVE_R1	25
#define DOOM_MOVE_R2	26

#define DNONE			0
#define DRIGHT_FORWARD	1
#define DRIGHT			2
#define DLEFT_REVERSE	3
#define	DLEFT			4


void Weapon_Doom2 (edict_t *ent, int FRAME_FIRST, int FRAME_LAST, 
								 int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;
	int     i;
	
	if(ent->deadflag || (ent->s.modelindex != 255 
		&& !(ent->flags & FL_INVIS)))
		return;
	
	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == DOOM_DOWN_LAST)
		{
			ChangeWeapon (ent);
			return;
		}
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe == DOOM_UP_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_FIRST;
			return;
		}
		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = DOOM_DOWN_FIRST;
		return;
	}

	i = ITEM_INDEX(ent->client->pers.weapon);

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if ((!ent->client->ammo_index) || 
				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
			{
				ent->client->ps.gunframe = FRAME_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
				if(i != D_FISTS) 
				{
					ent->client->anim_priority = ANIM_ATTACK;
					if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
					{
						ent->s.frame = FRAME_crattak1-1;
						ent->client->anim_end = FRAME_crattak9;
					}
					else
					{
						ent->s.frame = FRAME_attack1-1;
						ent->client->anim_end = FRAME_attack8;
					}
				}
			}
			else
			{
				ent->client->weaponstate = WEAPON_DROPPING;
				ent->client->ps.gunframe = DOOM_DOWN_FIRST;
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				Oldie_NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->oldweapon =0.0;
			
			if(ent->client->anim_run)
			{
				if(ent->client->ps.gunframe < DOOM_SWAY_LAST &&		//in the middle of bobbinf
				   ent->client->ps.gunframe >= DOOM_STATIONARY)
					ent->client->ps.gunframe++;
				else if(ent->client->ps.gunframe == DOOM_SWAY_LAST)  
					ent->client->ps.gunframe = DOOM_SWAY_FIRST;
				else												//just finished firing
					ent->client->ps.gunframe = DOOM_STATIONARY;
				ent->client->machinegun_shots = 0;
			}
			else
			{
				if(ent->client->machinegun_shots == DRIGHT_FORWARD ||
				   (ent->client->ps.gunframe >= 14 &&		//is on the way right 
				    ent->client->ps.gunframe <= 17))
				{
					if(ent->client->ps.gunframe == 17)
					{
						ent->client->ps.gunframe = DOOM_MOVE_R1;
						ent->client->machinegun_shots = DRIGHT;
						return;
					}
					ent->client->machinegun_shots = DRIGHT_FORWARD;
					ent->client->ps.gunframe++;
				}
				else if(ent->client->machinegun_shots == DLEFT_REVERSE ||
					(ent->client->ps.gunframe >= 11 &&		//is on the way left, reverse
					 ent->client->ps.gunframe < 14))
				{
					if(ent->client->ps.gunframe == 11)
					{
						ent->client->ps.gunframe = DOOM_MOVE_L1;
						ent->client->machinegun_shots = DLEFT;
						return;
					}
					ent->client->machinegun_shots = DLEFT_REVERSE;
					ent->client->ps.gunframe--;
				}
				else if(ent->client->machinegun_shots == DLEFT)
				{
					if(ent->client->ps.gunframe == DOOM_MOVE_L2)
					{
						ent->client->machinegun_shots = 0;
						ent->client->ps.gunframe = DOOM_STATIONARY;
					}
					ent->client->ps.gunframe++;
				}
				else if(ent->client->machinegun_shots == DRIGHT)
				{
					if(ent->client->ps.gunframe == DOOM_MOVE_R2)
					{
						ent->client->machinegun_shots = 0;
						ent->client->ps.gunframe = DOOM_STATIONARY;
					}
					ent->client->ps.gunframe++;
				}
				else
				{
					ent->client->ps.gunframe = DOOM_STATIONARY;
					ent->client->machinegun_shots = 0;
				}
			}
			return;
		}
	}


	if (ent->client->weaponstate == WEAPON_FIRING)
	{   
		if ((ent->client->pers.inventory[ent->client->ammo_index] < ent->client->pers.weapon->quantity) 
			&& (i != D_CSAW)
			&& (i != D_FISTS))
		{
			ent->client->ps.gunframe = DOOM_DOWN_FIRST;
			ent->client->weaponstate = WEAPON_DROPPING;
			Oldie_NoAmmoWeaponChange (ent);
			return;
		}

		if(ent->client->ps.gunframe < FRAME_FIRST)
			ent->client->ps.gunframe= FRAME_FIRST;

		ent->client->ps.gunframe++;
		ent->client->machinegun_shots = 0;

		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if(ent->client->super_damage_finished < level.time)
				{
					if (!CTFApplyStrengthSound(ent))

					if(sog_ctf->value)           
						CTFApplyHasteSound(ent);

					ent->client->super_damage_finished = level.time + 1;
				}
				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
		{
			if (ent->client->ps.gunframe >= FRAME_LAST)
			{
	   			ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = FRAME_FIRST;
				return;
			}
		}
	}
}


void Weapon_Doom (edict_t *ent, int FRAME_FIRST, int FRAME_LAST, int *fire_frames, void (*fire)(edict_t *ent))
{
	int oldstate = ent->client->weaponstate;

	if (ent->s.modelindex != 255 &&
		ent->client->invis_framenum < level.framenum) 
		return; 

	Weapon_Doom2 (ent,FRAME_FIRST, FRAME_LAST,fire_frames, fire);

	if (CTFApplyHaste(ent) && oldstate == ent->client->weaponstate) //try
	{
		Weapon_Doom2 (ent, FRAME_FIRST, FRAME_LAST, fire_frames, fire);
	}
}

/*
======================================================================

DOOM CHAINSAW - yeah bAb33

======================================================================
*/

void weapon_d_fist_fire (edict_t *ent)
{
	vec3_t		start,
				forward,
				right,
				offset;
	int			damage=12;
		
/*	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe= 27; 
		ent->client->weaponstate = WEAPON_READY;
		return;
	}
*/	
	if(ent->client->ps.gunframe == 30)
	{
		if(!deathmatch->value)
			damage = 15;

		if (ent->flags & FL_D_BERSERK)
			damage *= 15;
	
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 8, 0, ent->viewheight - 4); 
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		
		gi.sound (ent, CHAN_RELIABLE|CHAN_WEAPON, gi.soundindex("idg3weapons/axe/ax1.wav"), 1.0, ATTN_NORM, 0);
		d_fire_punch (ent, start, forward, damage);
	}
}

void Weapon_D_Fists(edict_t *ent)
{
	static int	fire_frames[]	= {28,29,30,31,32,33,0};  
	Weapon_Doom (ent, 27, 35, fire_frames , weapon_d_fist_fire); 
	
	if(ent->client->weaponstate == WEAPON_FIRING && 
	  !(ent->flags & FL_INVIS) &&
	   (ent->client->ps.gunframe == 28))
	{
		ent->client->anim_priority = ANIM_ATTACK;
		
		// Figure out next SWING animation to play
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crpunch1 -1;
			ent->client->anim_end = FRAME_crpunch5;
		}
		else 
		{
			if(random() > 0.4)
			{
				ent->s.frame = FRAME_punch01 - 1;
				ent->client->anim_end = FRAME_punch05;
			}
			else
			{
				ent->s.frame = FRAME_punch06 - 1;
				ent->client->anim_end = FRAME_punch10;
			}
		}
	}
}

/*
======================================
Chainsaw
======================================
*/


void weapon_d_saw_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage=10;
			
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe= 27;
		ent->client->weaponstate = WEAPON_READY;
		ent->client->oldweapon = 0.0;
		return;
	}
	else
	{
		if(!deathmatch->value)
			damage = 12;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 16, 0, ent->viewheight-8);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		
		d_fire_saw(ent, start, forward, damage);
		
		// ### Hentai ### BEGIN
		ent->client->anim_priority = ANIM_ATTACK;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crattak1 - 1;
			ent->client->anim_end = FRAME_crattak9;
		}
		else
		{
			ent->s.frame = FRAME_attack1 - 1;
			ent->client->anim_end = FRAME_attack8;
		}
		// ### Hentai ### END
	}

	if(ent->client->ps.gunframe >= 37 && ent->client->buttons & BUTTON_ATTACK)
		ent->client->ps.gunframe = 28;
}


void Weapon_D_Saw (edict_t *ent)
{
	static int	fire_frames[]	= {28, 29, 30, 31,32,33,34,35,36,37,0};
	Weapon_Doom (ent, 27, 37, fire_frames , weapon_d_saw_fire);
}


/*
======================================================================

DOOM PISTOL

======================================================================
*/

void weapon_d_pistol_fire (edict_t *ent)
{
	vec3_t	offset,forward,right,up,start;
	int		damage=16;
			
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe= 27;
		ent->client->weaponstate = WEAPON_READY;
		return;

	}
	else
	{
		if(!deathmatch->value)
			damage = 18;

		if (ent->client->pers.inventory[ent->client->ammo_index] < 1) 
		{	
			ent->client->ps.gunframe= 27;
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			Oldie_NoAmmoWeaponChange (ent);
			return;
		}


		AngleVectors (ent->client->v_angle, forward, right, up);
		VectorSet(offset, 24 , 0, ent->viewheight-8);
		
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		fire_bullet (ent, start, forward, damage, 5, 100, 200, MOD_DOOM_PISTOL);
			
		gi.sound (ent, CHAN_WEAPON|CHAN_RELIABLE, gi.soundindex("idg2weapons/pistol.wav"), 1.0, ATTN_NORM, 0);
		
		if(ent->client->chasetoggle)
		{	
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent->client->oldplayer-g_edicts);
			gi.WriteByte (MZ_MACHINEGUN | 128);
			gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
		}
		else
		{	
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_MACHINEGUN | 128);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
		// ### Hentai ### BEGIN
		ent->client->anim_priority = ANIM_ATTACK;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crattak1 - 1;
			ent->client->anim_end = FRAME_crattak9;
		}
		else
		{
			ent->s.frame = FRAME_attack1 - 1;
			ent->client->anim_end = FRAME_attack8;
		}
		// ### Hentai ### END


		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index]--;
	}
}


void Weapon_D_Pistol (edict_t *ent)
{
	static int	fire_frames[]	= {28,0};
	Weapon_Doom (ent, 27, 33, fire_frames , weapon_d_pistol_fire);
}

/*
======================================================================

DOOM SG

======================================================================
*/


void weapon_d_sg_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 10;
	int			kick = 10;
	

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 8, 0,  ent->viewheight-8);
	G_ProjectSource(ent->s.origin, offset, forward, right, start);

	if(!deathmatch->value)
		damage = 10;

	//if(deathmatch->value)
	//fire_shotty 
	fire_shotgun
		(ent, start, forward, damage, kick, 
		DEFAULT_D_SG_HSPREAD,DEFAULT_D_SG_VSPREAD, DEFAULT_D_SG_COUNT,MOD_DOOM_SG);

	if(ent->client->chasetoggle)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	
	gi.sound (ent, CHAN_RELIABLE|CHAN_WEAPON, gi.soundindex("idg2weapons/sg.wav"), 1.0, ATTN_NORM, 0);
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_D_SG (edict_t *ent)
{
	static int	fire_frames[]	= {28,0};
	Weapon_Doom (ent, 27, 38, fire_frames , weapon_d_sg_fire);
}


/*
======================================================================

DOOM SSG

======================================================================
*/


void weapon_d_ssg_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = 14;
	int			kick = 15;
	
	if(!deathmatch->value)
		damage = 15;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 0,  ent->viewheight-8);
	G_ProjectSource(ent->s.origin, offset, forward, right, start);

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 4.5;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	//fire_shotty 
	fire_shotgun	(ent, start, forward, damage, kick, 
		DEFAULT_D_SG_HSPREAD,DEFAULT_D_SG_VSPREAD*2, (DEFAULT_D_SSG_COUNT/2),MOD_DOOM_SSG);
	
	v[YAW]   = ent->client->v_angle[YAW] + 4.5;
	AngleVectors (v, forward, NULL, NULL);
	//	fire_shotty 
	fire_shotgun(ent, start, forward, damage, kick, 
		DEFAULT_D_SG_HSPREAD,DEFAULT_D_SG_VSPREAD*2, (DEFAULT_D_SSG_COUNT/2),MOD_DOOM_SSG);
	
	if(ent->client->chasetoggle)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	gi.sound (ent, CHAN_WEAPON|CHAN_RELIABLE, gi.soundindex("idg2weapons/ssg.wav"), 1.0, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_D_SSG (edict_t *ent)
{
	static int	fire_frames[]	= {28,0};
	Weapon_Doom (ent, 27, 45, fire_frames , weapon_d_ssg_fire);
}



/*
======================================================================

DOOM CHAINGUN

======================================================================
*/
void weapon_d_cg_fire (edict_t *ent)
{
	vec3_t	offset,forward,right,up, start;
	int		damage=8;
	int		kick=2;
	float   r,u;
		
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe= 27;
		ent->client->weaponstate = WEAPON_READY;
		return;

	}
	else
	{
		if(!deathmatch->value)
			damage = 8;

		if (ent->client->pers.inventory[ent->client->ammo_index] < 1) 
		{	
			ent->client->ps.gunframe= 27;
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			Oldie_NoAmmoWeaponChange (ent);
			return;
		}

		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		gi.sound (ent, CHAN_AUTO, gi.soundindex("idg2weapons/pistol.wav"), 1.0, ATTN_NORM, 0);
		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_DOOM_CG);
				
//disint2,disrupt,disrupthit
		if(ent->client->chasetoggle)
		{	
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent->client->oldplayer-g_edicts);
			gi.WriteByte (MZ_MACHINEGUN | 128);
			gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);

		}
		else
		{		
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_MACHINEGUN | 128);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}

		// ### Hentai ### BEGIN
		ent->client->anim_priority = ANIM_ATTACK;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crattak1 - 1;
			ent->client->anim_end = FRAME_crattak9;
		}
		else
		{
			ent->s.frame = FRAME_attack1 - 1;
			ent->client->anim_end = FRAME_attack8;
		}
		// ### Hentai ### END

		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index]--;

		if(ent->client->ps.gunframe >= 35 && ent->client->pers.inventory[ent->client->ammo_index] && ent->client->buttons & BUTTON_ATTACK)
			ent->client->ps.gunframe = 28;
	}

}


void Weapon_D_CG (edict_t *ent)
{
	static int	fire_frames[]	= {28,29,30,31,32,33,34,35,0};
	Weapon_Doom (ent, 27, 35, fire_frames , weapon_d_cg_fire);
}

/*
======================================================================

DOOM PLASMAGUN

======================================================================
*/


void weapon_d_pg_fire (edict_t *ent)
{
	vec3_t	offset,
			forward,
			right,
			start;
	int		damage=22;
			
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if(ent->client->ps.gunframe < 31)
		{
			ent->client->ps.gunframe= 31;
			ent->client->weaponstate = WEAPON_FIRING;
			return;
		}

	}
	else if( (ent->client->ps.gunframe == 28) ||
			 (ent->client->ps.gunframe == 29) ||
			 (ent->client->ps.gunframe == 30))
	{
		if(!deathmatch->value)
			damage = 25;

		if (ent->client->pers.inventory[ent->client->ammo_index] < 1) 
		{	
			ent->client->ps.gunframe = 0;
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			Oldie_NoAmmoWeaponChange (ent);
			return;
		}

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24 , 0, ent->viewheight-8);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		
		d_fire_plasma(ent, start, forward, damage, 800); 
		
		if(ent->client->chasetoggle)
		{	
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent->client->oldplayer-g_edicts);
			gi.WriteByte (MZ_RAILGUN | 128);
			gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
		}
		else
		{		
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_RAILGUN | 128);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
		gi.sound (ent, CHAN_AUTO, gi.soundindex("idg2weapons/plasma.wav"), 1.0, ATTN_NORM, 0);

		// ### Hentai ### BEGIN
		ent->client->anim_priority = ANIM_ATTACK;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crattak1 - 1;
			ent->client->anim_end = FRAME_crattak9;
		}
		else
		{
			ent->s.frame = FRAME_attack1 - 1;
			ent->client->anim_end = FRAME_attack8;
		}
		// ### Hentai ### END

		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index] --;
		
		if(ent->client->ps.gunframe == 30 && ent->client->pers.inventory[ent->client->ammo_index] && ent->client->buttons & BUTTON_ATTACK)
			ent->client->ps.gunframe = 28;
	}
}


void Weapon_D_PG (edict_t *ent)
{
	static int	fire_frames[]	= {28,29,30,31,32,33,34,35,36,37,0};
	Weapon_Doom (ent, 27, 37, fire_frames , weapon_d_pg_fire);
}



/*
======================================================================

DOOM ROCKET

======================================================================
*/

void weapon_d_rl_fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius=120.0;
	int		radius_damage=120;
		
//	damage = 100 + (int)(random() * 60);

// Arno changed - decreased damage
   
   damage = 70 + (int)(random() * 60);


//gi.dprintf("Damage :%d\n",damage);
//	if(!deathmatch->value)
//		damage += 20;
	
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 12, 0, ent->viewheight-8);
    G_ProjectSource(ent->s.origin, offset, forward, right, start);
	
 	// Arno changed - speed increased from 680 to 950
 	
 	d_fire_rocket (ent, start, forward, damage, 950, damage_radius, radius_damage);
	
	// send muzzle flash
	if(ent->client->chasetoggle)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	
	gi.sound (ent, CHAN_AUTO, gi.soundindex("idg2weapons/rocket.wav"), 1.0, ATTN_NORM, 0);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_D_RL (edict_t *ent)
{
	static int	fire_frames[]	= {28,0};
	Weapon_Doom (ent, 27, 34, fire_frames , weapon_d_rl_fire);
}


/*
======================================================================

DOOM BFG

======================================================================
*/

void weapon_d_bfg_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;
	float	damage_radius = 1000;

		
//	if (!(ent->client->buttons & BUTTON_ATTACK))
//	{
//		ent->client->ps.gunframe= 0;
//		ent->client->weaponstate = WEAPON_READY;
//		return;
//	}
//	else
//	{     
		
	if (ent->client->pers.inventory[ent->client->ammo_index] < 40) 
	{	
		ent->client->ps.gunframe= 27;
		ent->client->weaponstate = WEAPON_DROPPING;
		Oldie_NoAmmoWeaponChange (ent);
		return;
	}


	if(ent->client->ps.gunframe == 29)
	{
		gi.sound (ent, CHAN_WEAPON|CHAN_RELIABLE, gi.soundindex("idg2weapons/bfg.wav"), 1.0, ATTN_NORM, 0);

		// ### Hentai ### BEGIN
		ent->client->anim_priority = ANIM_ATTACK;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crattak1 - 1;
			ent->client->anim_end = FRAME_crattak9;
		}
		else
		{
			ent->s.frame = FRAME_attack1 - 1;
			ent->client->anim_end = FRAME_attack8;
		}
	 // ### Hentai ### END
	}


	if(ent->client->ps.gunframe == 35)
	{
		if (deathmatch->value)
		{	// normal damage is too extreme in dm
			damage = 250;
			kick = 40;
		}
		else
		{
			damage = 500;
			kick = 60;
		}


		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 8, 0, ent->viewheight-8);
		
		ent->client->v_dmg_pitch = -15;
		ent->client->v_dmg_roll = crandom()*2;
		ent->client->v_dmg_time = level.time + DAMAGE_TIME;
		
			
		// send muzzle flash
		if(ent->client->chasetoggle)
		{	
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent->client->oldplayer-g_edicts);
			gi.WriteByte (MZ_BFG | 128);
			gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
		}
		else
		{		
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_BFG | 128);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
		
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		d_fire_bfg (ent, start, forward, damage, 580, damage_radius);
		PlayerNoise(ent, start, PNOISE_WEAPON);

		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 40;
	}
}


void Weapon_D_BFG (edict_t *ent)
{
	static int	fire_frames[]	= {28,29,30,31,32,33,34,35,0};
	Weapon_Doom (ent, 27, 41, fire_frames , weapon_d_bfg_fire);
}


/*
======================================================================

======================================================================
 WOLF
======================================================================

======================================================================
*/

void Weapon_Wolfy (edict_t *ent, int FRAME_FIRST, int FRAME_LAST, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;
	int     i;
	
	if(ent->deadflag || 
	  ((ent->s.modelindex != 255) && (!ent->flags & FL_INVIS))) 
		return;

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = FRAME_FIRST;
		return;
	}

	i = ITEM_INDEX(ent->client->pers.weapon);

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;

/*		if((i == W_KNIFE) || (i==W_GG2))
		{
			ent->client->anim_priority = ANIM_ATTACK;
			ent->client->anim_end = ent->s.frame;
		}
*/
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if ((!ent->client->ammo_index) || 
				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
			{
				ent->client->ps.gunframe = FRAME_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
			
				// start the animation
				if((i!=W_GG2)) //(i != W_KNIFE) && 
				{
					ent->client->anim_priority = ANIM_ATTACK;
					if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
					{
						ent->s.frame = FRAME_crattak1-1;
						ent->client->anim_end = FRAME_crattak9;
					}
					else
					{
						ent->s.frame = FRAME_attack1-1;
						ent->client->anim_end = FRAME_attack8;
					}
				}
			}
			else
			{
				ent->client->weaponstate = WEAPON_DROPPING;
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				Oldie_NoAmmoWeaponChange (ent);
			}
		}
		else
		{

			ent->client->ps.gunframe = FRAME_FIRST;
			ent->client->weaponstate = WEAPON_READY;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{   
		//Ignore if using ammoless weapons
		if ( (ent->client->pers.inventory[ent->client->ammo_index] < 1 )
			&& (i != W_KNIFE))
		{
			ent->client->ps.gunframe = FRAME_FIRST;
			ent->client->weaponstate = WEAPON_DROPPING;
			Oldie_NoAmmoWeaponChange (ent);
			return;
		}

//gi.dprintf("Fire frame :%d\n",ent->client->ps.gunframe);

		ent->client->ps.gunframe++;

		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if (!CTFApplyStrengthSound(ent))
				if(sog_ctf->value)           
					CTFApplyHasteSound(ent);
				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
		{
			if (ent->client->ps.gunframe == FRAME_LAST)
			{
	   			ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = FRAME_FIRST;
				return;
			}
		}
		
	}
}


void Weapon_Wolf3d (edict_t *ent, int FRAME_FIRST, int FRAME_LAST, int *fire_frames, void (*fire)(edict_t *ent))
{
	int oldstate = ent->client->weaponstate;

	if (ent->s.modelindex != 255 &&
		ent->client->invis_framenum < level.framenum) 
		return; 

	Weapon_Wolfy (ent,FRAME_FIRST, FRAME_LAST,fire_frames, fire);

	if (CTFApplyHaste(ent) && oldstate == ent->client->weaponstate) //try
	{
		Weapon_Wolfy (ent, FRAME_FIRST, FRAME_LAST, fire_frames, fire);
	}
}


//
// Knife
//
void weapon_w_knife_fire (edict_t *ent)
{
	vec3_t		start,
				forward,
				right,
				offset;
	int			damage=20;
		
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if(ent->client->ps.gunframe == 7)
		{
			ent->client->ps.gunframe= 0; 
			ent->client->weaponstate = WEAPON_READY;
			return;
		}
	}
	
	if(ent->client->ps.gunframe == 4)
	{
		if (is_quad)
			damage *= 4;
	
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 8, 0, ent->viewheight - 4); 
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("idg1weapons/knifeair.wav"), 1.0, ATTN_NORM, 0);
		w_fire_knife(ent, start, forward, damage);

		ent->client->anim_priority = ANIM_ATTACK;

		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crknife1 - 1;
			ent->client->anim_end = FRAME_crknife5;
		}
		else
		{
			ent->s.frame = FRAME_knife1 - 1;
			ent->client->anim_end = FRAME_knife5;
		}		
	}
}

void Weapon_W_Knife(edict_t *ent)
{
	static int	fire_frames[]	= {1,2,3,4,5,6,0}; 
	Weapon_Wolf3d (ent, 0, 7, fire_frames , weapon_w_knife_fire); 
}


//Pistol

void weapon_w_pistol_fire (edict_t *ent)
{
	vec3_t	offset,
			forward,
			right,
			up,
			start;
	int		damage=16;
			
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe= 0;
		ent->client->weaponstate = WEAPON_READY;
		return;

	}
	else
	{
		if (ent->client->pers.inventory[ent->client->ammo_index] < 1) 
		{	
			ent->client->ps.gunframe = 0;
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			Oldie_NoAmmoWeaponChange (ent);
			return;
		}

		if(!deathmatch->value)
			damage = 20;

		if(is_quad)
			damage *= 4;

		AngleVectors (ent->client->v_angle, forward, right, up);
		VectorSet(offset, 24 , 0, ent->viewheight-8);
		
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		fire_bullet (ent, start, forward, damage, 5, 100, 200, MOD_WOLF_PISTOL);
			
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("idg1weapons/pshot.wav"), 1.0, ATTN_NORM, 0);
		
	if(ent->client->chasetoggle)
	{	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

		// ### Hentai ### BEGIN
		ent->client->anim_priority = ANIM_ATTACK;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crattak1 - 1;
			ent->client->anim_end = FRAME_crattak9;
		}
		else
		{
			ent->s.frame = FRAME_attack1 - 1;
			ent->client->anim_end = FRAME_attack8;
		}
		// ### Hentai ### END

		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index]--;
	}
}

void Weapon_W_Pistol(edict_t *ent)
{
	static int	fire_frames[]	= {1,0};
	Weapon_Wolf3d (ent, 0, 6, fire_frames , weapon_w_pistol_fire);
}

//
//Machine Gun
//

void weapon_w_mg_fire (edict_t *ent)
{
	vec3_t	offset,forward,right,up, start;
	int		damage=5;
	int		kick=2;
	float   r,u;
		
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe= 0;
		ent->client->weaponstate = WEAPON_READY;
		return;

	}
	else
	{
		if (ent->client->pers.inventory[ent->client->ammo_index] < 1) 
		{	
			ent->client->ps.gunframe = 0;
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			Oldie_NoAmmoWeaponChange (ent);
			return;
		}

		if(!deathmatch->value)
			damage = 6;

		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 4 + crandom()*2;
		u = crandom()*2;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);

		if(is_quad)
			damage *=4;
		
		gi.sound (ent, CHAN_AUTO, gi.soundindex("idg1weapons/mgshot.wav"), 1.0, ATTN_NORM, 0);
		
		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_WOLF_MACHINEGUN);
				
	if(ent->client->chasetoggle)
	{	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
		// ### Hentai ### BEGIN
		ent->client->anim_priority = ANIM_ATTACK;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crattak1 - 1;
			ent->client->anim_end = FRAME_crattak9;
		}
		else
		{
			ent->s.frame = FRAME_attack1 - 1;
			ent->client->anim_end = FRAME_attack8;
		}
		// ### Hentai ### END

		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index]--;

		if(ent->client->ps.gunframe == 4 && ent->client->pers.inventory[ent->client->ammo_index] && 
		   ent->client->buttons & BUTTON_ATTACK)
			ent->client->ps.gunframe = 1;
	}

}

void Weapon_W_MG(edict_t *ent)
{
	static int	fire_frames[]	= {1,2,3,4};
	Weapon_Wolf3d (ent, 0, 4, fire_frames , weapon_w_mg_fire);
}


//====================================================
//
//Chaingun
//
//====================================================


void weapon_w_cg_fire (edict_t *ent)
{
	vec3_t	offset,forward,right,up, start;
	int		damage=8;
	int		kick=6;
	float   r,u;
		
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe= 0;
		ent->client->weaponstate = WEAPON_READY;
		return;

	}
	else
	{
		if (ent->client->pers.inventory[ent->client->ammo_index] < 1) 
		{	
			ent->client->ps.gunframe = 0;
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			Oldie_NoAmmoWeaponChange (ent);
			return;
		}

		if(!deathmatch->value)
			damage = 8;

		if(is_quad)
			damage *=4;

		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 6 + crandom()*4;
		u = crandom()*2;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		
		gi.sound (ent, CHAN_AUTO, gi.soundindex("idg1weapons/ggshot.wav"), 1.0, ATTN_NORM, 0);
		
		fire_bullet (ent, start, forward, damage, kick, 350, 500, MOD_WOLF_GG);
				
	if(ent->client->chasetoggle)
	{	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

		// ### Hentai ### BEGIN
		ent->client->anim_priority = ANIM_ATTACK;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crattak1 - 1;
			ent->client->anim_end = FRAME_crattak9;
		}
		else
		{
			ent->s.frame = FRAME_attack1 - 1;
			ent->client->anim_end = FRAME_attack8;
		}
		// ### Hentai ### END

		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index] -= 2;

		if(ent->client->ps.gunframe == 9 && ent->client->pers.inventory[ent->client->ammo_index] && 
		   ent->client->buttons & BUTTON_ATTACK)
			ent->client->ps.gunframe = 1;
	}

}

void Weapon_W_GG(edict_t *ent)
{
	static int	fire_frames[]	= {1,2,3,4,5,6,7,8,9,0};
	Weapon_Wolf3d (ent, 0, 8, fire_frames , weapon_w_cg_fire);
}

//====================================================
//====================================================

void weapon_w_cg2_fire (edict_t *ent)
{
	vec3_t	offset,forward,right,up, start;
	int		damage=10;
	int		kick=6;
	float   r,u;
		
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe= 0;
		ent->client->weaponstate = WEAPON_READY;
		return;

	}
	else
	{
		if (ent->client->pers.inventory[ent->client->ammo_index] < 1) 
		{	
			ent->client->ps.gunframe = 0;
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			ent->client->anim_priority = ANIM_PAIN;
			if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				ent->s.frame = FRAME_crpain1;
				ent->client->anim_end = FRAME_crpain4;
			}
			else
			{
					ent->s.frame = FRAME_pain301;
					ent->client->anim_end = FRAME_pain304;
			}
			Oldie_NoAmmoWeaponChange (ent);
			return;
		}

		if(!deathmatch->value)
			damage = 10;

		if(is_quad)
			damage *=4;

		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 8 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		fire_bullet (ent, start, forward, damage, kick, 350, 600, MOD_WOLF_GG2);

		r = 8 - crandom()*6;
		u = crandom()*3;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);
		fire_bullet (ent, start, forward, damage, kick, 350, 600, MOD_WOLF_GG2);

		gi.sound (ent, CHAN_AUTO, gi.soundindex("idg1weapons/ggshot.wav"), 1.0, ATTN_NORM, 0);
				
	if(ent->client->chasetoggle)
	{	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

		ent->client->anim_priority = ANIM_ATTACK;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = 210-1;
			ent->client->anim_end = 211;
		}
		else
		{
			ent->s.frame = 208-1;
			ent->client->anim_end = 209;
		}

		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index] -= 4;

		if(ent->client->ps.gunframe == 9 && ent->client->pers.inventory[ent->client->ammo_index] && 
		   ent->client->buttons & BUTTON_ATTACK)
			ent->client->ps.gunframe = 1;
	}

}

void Weapon_W_GG2(edict_t *ent)
{
	static int	fire_frames[]	= {1,2,3,4,5,6,7,8,9,0};
	Weapon_Wolf3d (ent, 0, 8, fire_frames , weapon_w_cg2_fire);
}

//====================================================
//Bazooka
//====================================================

void weapon_w_rl_fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius=130.0;
	int		radius_damage=110;
		
	damage = 100 + (int)(random() * 20);

	if(!deathmatch->value)
		damage += 15;

	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 12, 0, ent->viewheight-8);
    G_ProjectSource(ent->s.origin, offset, forward, right, start);
	
 	w_fire_rocket (ent, start, forward, damage, 725, damage_radius, radius_damage);
	
	// send muzzle flash
	if(ent->client->chasetoggle)
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent->client->oldplayer-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
	}
	else
	{
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_MACHINEGUN | 128);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	gi.sound (ent, CHAN_AUTO, gi.soundindex("idg1weapons/rlshot.wav"), 1.0, ATTN_NORM, 0);

		// ### Hentai ### BEGIN
		ent->client->anim_priority = ANIM_ATTACK;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crattak1 - 1;
			ent->client->anim_end = FRAME_crattak9;
		}
		else
		{
			ent->s.frame = FRAME_attack1 - 1;
			ent->client->anim_end = FRAME_attack8;
		}
		// ### Hentai ### END	
	
	
	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_W_RL(edict_t *ent)
{
	static int	fire_frames[]	= {1,0};
	Weapon_Wolf3d (ent, 0, 10, fire_frames , weapon_w_rl_fire);
}


void weapon_w_ft_fire(edict_t *ent)
{
//FIXME - water firing ??
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage=10;

	
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe= 3;
		ent->client->weaponstate = WEAPON_FIRING;
		return;
	}
	else
	{
		if (ent->client->pers.inventory[ent->client->ammo_index] < 1) 
		{	
			ent->client->ps.gunframe = 0;
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			Oldie_NoAmmoWeaponChange (ent);

			return;
		}

		if(ent->waterlevel >= 2)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			return;
		}

		if(!deathmatch->value)
			damage = 12;
		
		if(is_quad)
			damage *=4;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 36, 0, ent->viewheight-8);
		G_ProjectSource(ent->s.origin, offset, forward, right, start);

        w_fire_flame(ent, start, forward, damage,380);
		gi.sound (ent, CHAN_WEAPON, gi.soundindex("idg1weapons/ftshot.wav"), 1.0, ATTN_NORM, 0);

		// ### Hentai ### BEGIN
		ent->client->anim_priority = ANIM_ATTACK;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crattak1 - 1;
			ent->client->anim_end = FRAME_crattak9;
		}
		else
		{
			ent->s.frame = FRAME_attack1 - 1;
			ent->client->anim_end = FRAME_attack8;
		}
		// ### Hentai ### END

		if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
			ent->client->pers.inventory[ent->client->ammo_index] --;
	}

		if((ent->client->ps.gunframe == 3) &&
			ent->client->pers.inventory[ent->client->ammo_index] && ent->client->buttons & BUTTON_ATTACK)
			ent->client->ps.gunframe = 1;
}


void Weapon_W_FT(edict_t *ent)
{
	static int	fire_frames[]	= {1,2,3,0};
	Weapon_Wolf3d (ent, 0, 7, fire_frames , weapon_w_ft_fire);
}


/*
======================================================================

COMMANDER KEEN RAYGUN   !!!!!!!!!!!!!!!

======================================================================
*/

void weapon_Keen_Raygun_fire (edict_t *ent)
{
	vec3_t	offset,forward,right,up,start;
	int		damage=23;
			
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe= 27;
		ent->client->weaponstate = WEAPON_READY;
		return;

	}
	else
	{
		AngleVectors (ent->client->v_angle, forward, right, up);
		VectorSet(offset, 24 , 8, ent->viewheight-6);
		
		P_ProjectSource(ent->client,ent->s.origin, offset, forward, right, start);
		keen_fire_raygunshot (ent, start, forward, damage, 400);
			
		gi.sound (ent, CHAN_WEAPON|CHAN_RELIABLE, gi.soundindex("idg3weapons/lightning/lstart.wav"), 1.0, ATTN_NORM, 0);
		
		if(ent->client->chasetoggle)
		{	
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent->client->oldplayer-g_edicts);
			gi.WriteByte (MZ_BLASTER | 128);
			gi.multicast (ent->client->oldplayer->s.origin, MULTICAST_PVS);
		}
		else
		{	
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_BLASTER | 128);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
		}
		// ### Hentai ### BEGIN
		ent->client->anim_priority = ANIM_ATTACK;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			ent->s.frame = FRAME_crattak1 - 1;
			ent->client->anim_end = FRAME_crattak9;
		}
		else
		{
			ent->s.frame = FRAME_attack1 - 1;
			ent->client->anim_end = FRAME_attack8;
		}
		// ### Hentai ### END
	}
}


void Weapon_Keen_Raygun (edict_t *ent)
{
	static int	fire_frames[]	= {28,0};
	Weapon_Doom (ent, 27, 33, fire_frames , weapon_Keen_Raygun_fire);
}
