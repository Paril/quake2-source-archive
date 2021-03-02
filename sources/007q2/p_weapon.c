// g_weapon.c

#include "g_local.h"
#include "m_player.h"
// #include "g_bhand.h"

static qboolean	is_quad;
static byte		is_silenced;
//extern cvar_t *muzzleon;



void weapon_grenade_fire (edict_t *ent, qboolean held);
// ion
void weapon_proxmine_fire (edict_t *ent, qboolean held);
void weapon_remote_fire (edict_t *ent, qboolean held);
void weapon_laser_fire (edict_t *ent, qboolean held);
// end ion
// +DL
void LoadAmmo(edict_t *ent);
// -DL

//static 
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource (point, _distance, forward, right, result);
}


/*
===============
PlayerNoise

Each player can have two noise objects associated with it:
a personal noise (jumping, pain, weapon firing), and a weapon
target noise (bullet wall impacts)

Monsters that don't directly see the player can move
to a noise in hopes of seeing the player from there.
===============
*/
void PlayerNoise(edict_t *who, vec3_t where, int type)
{
	edict_t		*noise;

	if (type == PNOISE_WEAPON)
	{
		if (who->client->silencer_shots)
		{
			who->client->silencer_shots--;
			return;
		}
	}

	if (deathmatch->value)
		return;

	if (who->flags & FL_NOTARGET)
		return;


	if (!who->mynoise)
	{
		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise = noise;

		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise2 = noise;
	}

	if (type == PNOISE_SELF || type == PNOISE_WEAPON)
	{
		noise = who->mynoise;
		level.sound_entity = noise;
		level.sound_entity_framenum = level.framenum;
	}
	else // type == PNOISE_IMPACT
	{
		noise = who->mynoise2;
		level.sound2_entity = noise;
		level.sound2_entity_framenum = level.framenum;
	}

	VectorCopy (where, noise->s.origin);
	VectorSubtract (where, noise->maxs, noise->absmin);
	VectorAdd (where, noise->maxs, noise->absmax);
	noise->teleport_time = level.time;
	gi.linkentity (noise);
}


qboolean Pickup_Weapon (edict_t *ent, edict_t *other)
{
	int			index;
	gitem_t		*ammo;
//    gitem_t     *item;
	int			markerA;
	int			markerB;


//	markerA = 0;

//	markerB = 0;

	index = ITEM_INDEX(ent->item);

  CheckCampSite(ent,other); // Maj++

	if ( ( ((int)(dmflags->value) & DF_WEAPONS_STAY) || coop->value)
		&& other->client->pers.inventory[index])
	{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
			return false;	// leave the weapon for others to pickup
	}


	other->client->pers.inventory[index]++;

	if (!(ent->spawnflags & DROPPED_ITEM) )
	{
		if (!(ent->item == FindItem ("Laser")))
		{
		// give them some ammo with it
		ammo = FindItem (ent->item->ammo);
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
			Add_Ammo (other, ammo, 1000);
		else
			Add_Ammo (other, ammo, ammo->quantity);
		}
		if (! (ent->spawnflags & DROPPED_PLAYER_ITEM) )
		{
			if (deathmatch->value)
			{
				if ((int)(dmflags->value) & DF_WEAPONS_STAY)
					ent->flags |= FL_RESPAWN;
				else
					SetRespawn (ent, 30);
			}
			if (coop->value)
				ent->flags |= FL_RESPAWN;
		}
	}

// fuzzysteve's idea:
    if ( (ent->item == FindItem ("ZMG 9mm")) && (other->client->pers.inventory[index]>1)) //&& (markerA == 0))
{
other->client->pers.inventory[ITEM_INDEX(FindItem("Dual ZMG 9mm's"))]=1;
//other->client->pers.selected_item[ITEM_INDEX(FindItem("Dual ZMG 9mm's"))];
//
//ent->item = FindItem("Dual ZMG 9mm's");
  other->client->pers.selected_item = other->client->pers.inventory[ITEM_INDEX(FindItem("Dual ZMG 9mm's"))];
//  	markerA = 1;
//ent->client->pers.inventory[ITEM_INDEX(FindItem("Dual ZMG 9mm's"))];
            }  // end of idea

//    if ( (other->client->pers.inventory[index]>1) && (markerB == 0))
    if ( (ent->item == FindItem ("ZMG 9mm")) && (other->client->pers.inventory[ITEM_INDEX(FindItem("ZMG 9mm"))]>1)) //&& (markerB == 0))
{
    ent->item=FindItem("Dual ZMG 9mm's");
//	markerB = 1;
}

	if (other->client->pers.weapon != ent->item &&
		(other->client->pers.inventory[index] == 1) &&
		( !deathmatch->value || other->client->pers.weapon == FindItem("PP7")) )

		other->client->newweapon = ent->item;

	return true;
}

/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void ChangeWeapon (edict_t *ent)
{
	int i;

	if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_grenade_fire (ent, false);
		ent->client->grenade_time = 0;
	}

// +DL - Save old ammo
	if (!(ent->client->pers.weapon == FindItem("Laser")))
	{
	ent->client->pers.inventory[ent->client->ammo_index] += ent->client->ammo_clip;
	ent->client->ammo_clip = 0;
	}
// -DL
	
/* ION
	if (ent->client->proxmine_time)
	{
		ent->client->proxmine_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_proxmine_fire (ent, false);
		ent->client->proxmine_time = 0;
	}
 ION  */
	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	// set visible model
	if (ent->s.modelindex == 255) {
		if (ent->client->pers.weapon)
			i = ((ent->client->pers.weapon->weapmodel & 0xff) << 8);
		else
			i = 0;
		ent->s.skinnum = (ent - g_edicts - 1) | i;
	}

	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
		ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
	else
		ent->client->ammo_index = 0;

//	CAUSES CRASH .ion fixed +DL - Init Clip System
	LoadAmmo(ent);
//	CAUSES CRASH .ion fixed -DL

	if (!ent->client->pers.weapon)
	{	// dead
		ent->client->ps.gunindex = 0;
		return;
	}


//	CrosshairSwitch(ent);

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
// ion



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
}

/*
=================
NoAmmoWeaponChange
=================
*/
void NoAmmoWeaponChange (edict_t *ent)
{

	if ( ((ent->client->pers.weapon == FindItem("PP7")) ||
		 (ent->client->pers.weapon == FindItem("ZMG 9mm")) ||
 		 (ent->client->pers.weapon == FindItem("Dual ZMG 9mm's")) ||
		 (ent->client->pers.weapon == FindItem("AR33 Assault Rifle"))) &&
		 (ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]) )
	{
//		ent->client->newweapon = ent->client->pers.weapon;
		Cmd_ReloadAmmo(ent);
		return;
	}

	if (g_gametype->value == 4)
	{
		if ((ent->client->pers.weapon == FindItem("RCP-90")) &&
			(ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]))
		{
			Cmd_ReloadAmmo(ent);
			return;
		}
		else if ((ent->client->pers.weapon == FindItem("KF7 Soviet")) &&
			(ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]))
		{
			Cmd_ReloadAmmo(ent);
			return;
		}
		else if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
			&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("KF7 Soviet"))] )
		{
			ent->client->newweapon = FindItem ("KF7 Soviet");
			return;
		}
	}
	if (g_gametype->value == 3)
	{
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))])
		{
			ent->client->newweapon = FindItem ("grenades");
			return;
		}
	}
	if (g_gametype->value == 2)
	{
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Remote Mines"))])
		{
			ent->client->newweapon = FindItem ("Remote Mines");
			return;
		}
	}
	if (g_gametype->value == 1)
	{
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Proximity Mines"))])
		{
			ent->client->newweapon = FindItem ("Proximity Mines");
			return;
		}
	}
	if (g_gametype->value == 0)
	{
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))]
			&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Rocket Launcher"))] )
		{
			ent->client->newweapon = FindItem ("Rocket Launcher");
			return;
		}
	}
	if (g_gametype->value != 5)
	{
		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
			&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("AR33 Assault Rifle"))] )
		{
			ent->client->newweapon = FindItem ("AR33 Assault Rifle");
			return;
		}

		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
			&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("Dual ZMG 9mm's"))] )
		{
			ent->client->newweapon = FindItem ("Dual ZMG 9mm's");
			return;
		}

		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
			&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("ZMG 9mm"))] )
		{
			ent->client->newweapon = FindItem ("ZMG 9mm");
			return;
		}

		if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
			&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("PP7"))] )
		{
			ent->client->newweapon = FindItem ("PP7");
			return;
		}
	}

	if(ent->client->newweapon == NULL) 
		ent->client->newweapon = FindItem ("Slapper");//FindItem ("blaster");

	ent->client->newweapon = FindItem ("Slapper");

}

#define DELAY_IN_SECONDS 2.5;

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon (edict_t *ent)
{
    static float check = 0;

	// if just died, put the weapon away
	if (ent->health < 1)
	{
		ent->client->newweapon = NULL;
		ChangeWeapon (ent);
	}

	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
		is_quad = (ent->client->quad_framenum > level.framenum);
		if (ent->client->silencer_shots)
			is_silenced = MZ_SILENCED;
		else
			is_silenced = 0;
		ent->client->pers.weapon->weaponthink (ent);
	}


/*	if( ent->client->weaponstate == WEAPON_RELOADING && checkon == false)
	{
	check = level.time + DELAY_IN_SECONDS-1; // here: 5 seconds
	checkon = true;
	}
	if ( ent->client->weaponstate == WEAPON_RELOADING)
	{
		if (level.time <= check)
			{
				ent->client->ps.gunframe = 0;
			}
			else
			{
				ent->client->weaponstate = WEAPON_ACTIVATING;
				checkon = false;
			}
	}
*/
}


/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Use_Weapon (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;

	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		{
			gi.cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
			gi.cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	// change to this weapon when down
	ent->client->newweapon = item;
}



/*
================
Drop_Weapon
================
*/
void Drop_Weapon (edict_t *ent, gitem_t *item)
{
	int		index;

	if ((int)(dmflags->value) & DF_WEAPONS_STAY)
		return;

	index = ITEM_INDEX(item);
	// see if we're already using it
	if ( ((item == ent->client->pers.weapon) || (item == ent->client->newweapon))&& (ent->client->pers.inventory[index] == 1) )
	{
		gi.cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	Drop_Item (ent, item);
	ent->client->pers.inventory[index]--;
}

/*
================

Weapon_Generic

================
*/


	#define FRAME_FIRE_FIRST		(FRAME_ACTIVATE_LAST + 1)
	#define FRAME_IDLE_FIRST		(FRAME_FIRE_LAST + 1)
	#define FRAME_DEACTIVATE_FIRST	(FRAME_IDLE_LAST + 1)

	//+BD - Added to incorporate reload and last round animations
	#define FRAME_RELOAD_FIRST		(FRAME_DEACTIVATE_LAST +1)
	#define FRAME_LASTRD_FIRST   (FRAME_RELOAD_LAST +1)
	//+BD end add

//+BD Create a local define to make changing clip size easy
	#define ZMG 32

//void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, /*+BD added*/int FRAME_RELOAD_LAST, int FRAME_LASTRD_LAST, /*+BD end add*/int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, 
			         int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, 
			         /*+BD added*/
			         int FRAME_RELOAD_LAST, int FRAME_LASTRD_LAST,
			         /*+BD end add*/
			         int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))

	{
		int		n;

//==== added (player muzzle) ====
/*if (muzzleon->value)
	{
		if (ent->muzzle_ent)
	  {
		G_FreeEdict(ent->muzzle_ent);
		ent->muzzle_ent = NULL;
	  }
	}
*/
  //========

//+BD - Added Reloading weapon, done manually via a cmd
		if( ent->client->weaponstate == WEAPON_RELOADING)
		{
			if(ent->client->ps.gunframe < FRAME_RELOAD_FIRST || ent->client->ps.gunframe > FRAME_RELOAD_LAST)
				ent->client->ps.gunframe = FRAME_RELOAD_FIRST;
			else if(ent->client->ps.gunframe < FRAME_RELOAD_LAST)
			{
				ent->client->ps.gunframe++;		
			}
			else
			{
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				ent->client->weaponstate = WEAPON_READY;			
			}
		}
		

		if (ent->client->weaponstate == WEAPON_DROPPING)
		{
			if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
			{
				ChangeWeapon (ent);
				return;
			}

			ent->client->ps.gunframe++;
			return;
		}

		if (ent->client->weaponstate == WEAPON_ACTIVATING)
		{
			if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
			{
				ent->client->weaponstate = WEAPON_READY;
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}
		
			//+BD - Check the current weapon to find out when to play reload sounds
//			if(stricmp(ent->client->pers.weapon->pickup_name, "ZMG 9mm") == 0)
//			{
//				if(ent->client->ps.gunframe == 6)
//					gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/zmg.wav"), 1, ATTN_NORM, 0);
//					ent->client->ZMG_max = ZMG;	//set mag rounds 					
//					ent->client->ZMG_rds = ZMG;	//fill the mag...
//			}
			ent->client->ps.gunframe++;
			return;
		}

		if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
		{
			ent->client->weaponstate = WEAPON_DROPPING;
			ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
			return;
		}

		if (ent->client->weaponstate == WEAPON_READY)
		{
			if (((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK))
			{
				ent->client->latched_buttons &= ~BUTTON_ATTACK;
				if ((!ent->client->ammo_index) || // ( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
				( ent->client->ammo_clip /* +DL ( ent->client->pers.inventory[ent->client->ammo_index] */ >= ent->client->pers.weapon->quantity))
				{
					ent->client->ps.gunframe = FRAME_FIRE_FIRST;
					ent->client->weaponstate = WEAPON_FIRING;

					// start the animation
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
				else
				{
					if (level.time >= ent->pain_debounce_time)
					{
						gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
						ent->pain_debounce_time = level.time + 1;
					}
				if (ent->client->pers.inventory[ent->client->ammo_index]  >= ent->client->pers.weapon->quantity)
					Cmd_ReloadAmmo(ent);
				else
					NoAmmoWeaponChange (ent);
					}
					//+BD - Disabled for manual weapon change

//					NoAmmoWeaponChange (ent);
//					}
//				}
			}
			else
			{
				if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
				{
					ent->client->ps.gunframe = FRAME_IDLE_FIRST;
					return;
				}

				if (pause_frames)
				{
					for (n = 0; pause_frames[n]; n++)
					{
						if (ent->client->ps.gunframe == pause_frames[n])
						{
							if (rand()&15)
							return;
						}
					}
				}

				ent->client->ps.gunframe++;
				return;
			}
		}

		if (ent->client->weaponstate == WEAPON_FIRING)
		{
			for (n = 0; fire_frames[n]; n++)
			{
				if (ent->client->ps.gunframe == fire_frames[n])
				{
					if (ent->client->quad_framenum > level.framenum)								
		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

if (ent->client->pers.weapon == FindItem("RCP-90")) {
// ion
 //==== added (player muzzle) ====
/*	if (muzzleon->value)
        {
        if (!ent->muzzle_ent)
        {
          if (ent->client->ps.gunindex)
          {
            ent->muzzle_ent = G_Spawn();
            VectorSet(ent->muzzle_ent->s.origin, 50, 0, 0);
            VectorCopy(ent->muzzle_ent->s.origin, ent->muzzle_ent->s.old_origin);
            ent->muzzle_ent->s.modelindex = gi.modelindex("models/muzzleflash/tris.md2");
            ent->muzzle_ent->s.renderfx = RF_WEAPONMODEL | RF_FULLBRIGHT | RF_FLASH;
//            ent->muzzle_ent->s.effects = EF_SPHERETRANS;
            ent->muzzle_ent->owner = ent;
            gi.linkentity(ent->muzzle_ent);
          }
        }
	}        //========
// ion*/

				fire (ent);
				fire (ent);
//                fire (ent);
break;
//break;
} else {
//ZOID
/*			if (!CTFApplyStrengthSound(ent))
//ZOID
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);
//ZOID
				CTFApplyHasteSound(ent);*/
//ZOID
// ion
 //==== added (player muzzle) ====
/*if (muzzleon->value)
{
	if (!ent->muzzle_ent)
        {
          if (ent->client->ps.gunindex)
          {
            ent->muzzle_ent = G_Spawn();
            VectorSet(ent->muzzle_ent->s.origin, 50, 0, 0);
            VectorCopy(ent->muzzle_ent->s.origin, ent->muzzle_ent->s.old_origin);
            ent->muzzle_ent->s.modelindex = gi.modelindex("models/muzzleflash/tris.md2");
            ent->muzzle_ent->s.renderfx = RF_WEAPONMODEL | RF_FULLBRIGHT | RF_FLASH;
//            ent->muzzle_ent->s.effects = EF_SPHERETRANS;
            ent->muzzle_ent->owner = ent;
            gi.linkentity(ent->muzzle_ent);
          }
        }
}*/
        //========
// ion
					fire (ent);
					break; }
				}
			}

			if (!fire_frames[n])
				ent->client->ps.gunframe++;

			if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
				ent->client->weaponstate = WEAPON_READY;
		}
}

//ZOID
void Weapon_Generic2 (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int oldstate = ent->client->weaponstate;

	Weapon_Generic (ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, 
		FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST,
		0,
		0,
		pause_frames, 
		fire_frames, fire);

	// run the weapon frame again if hasted
/*	if (stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
		ent->client->weaponstate == WEAPON_FIRING)
		return;*/

/*	if ((CTFApplyHaste(ent) ||
		(Q_stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
		ent->client->weaponstate != WEAPON_FIRING))
		&& oldstate == ent->client->weaponstate) {
		Weapon_Generic (ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST, 
			FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST,
		0,
		0,
		pause_frames, 
			fire_frames, fire);
	}*/
}
//ZOID


/*
======================================================================

ION SWORD

======================================================================
*/

// * sword variable definitions, I have this here because the sword is most likely still unbalanced
//* I know this because I killed a super tank with it without taking ANY damaga

#define SWORD_NORMAL_DAMAGE 20
#define SWORD_DEATHMATCH_DAMAGE 30
#define SWORD_KICK 200
#define SWORD_RANGE 35
/*
=============
fire_sword

attacks with the beloved sword of the highlander
 edict_t *self - entity producing it, yourself
 vec3_t start - The place you are
 vec3_t aimdir - Where you are looking at in this case
 int damage - the damage the sword inflicts
 int kick - how much you want that bitch to be thrown back
=============
*/

void fire_sword ( edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{    
    trace_t tr; //detect whats in front of you up to range "vec3_t end"

    vec3_t end;

    // Figure out what we hit, if anything:

    VectorMA (start, SWORD_RANGE, aimdir, end);  //calculates the range vector                      

    tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
                        // figuers out what in front of the player up till "end"
    
   // Figure out what to do about what we hit, if anything


	
 	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				//This tells us to damage the thing that in our path...hehe
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_SLAPPER);
                gi.sound (self, CHAN_AUTO, gi.soundindex("weapons/tink1.wav") , 1, ATTN_NORM, 0);
                // Sound if hit player/monster -- added 1-13-98 by DanE

			}
	  		else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPARKS);   //Changed 1-13-98 by DanE to make impact look like sparks
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.multicast (tr.endpos, MULTICAST_PVS);

			   		if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
                gi.sound (self, CHAN_AUTO, gi.soundindex("weapons/oldtink1.wav") , 1, ATTN_NORM, 0); 
                //Sound if hit wall  -- added 1-13-98 by DanE
				}
			}
		}
	}
	return;
}

void sword_attack (edict_t *ent, vec3_t g_offset, int damage)
{
	vec3_t  forward, right;
	vec3_t  start;
	vec3_t  offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
//	VectorSet(offset, 0, 0, ent->viewheight);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

// wave added
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
// wave added


	fire_sword (ent, start, forward, damage, SWORD_KICK );
}

void Weapon_Sword_Fire (edict_t *ent)
{
	int damage;
	if (deathmatch->value)
		damage = SWORD_DEATHMATCH_DAMAGE;
	else
		damage = SWORD_NORMAL_DAMAGE;
	sword_attack (ent, vec3_origin, damage);
	ent->client->ps.gunframe++;
}

void Weapon_Sword (edict_t *ent)
{
	static int      pause_frames[]  = {19, 32, 0};
	static int      fire_frames[]   = {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, 0, 0, pause_frames, fire_frames, Weapon_Sword_Fire);
}

//
//!Pridkett

/*
======================================================================

ION KNIFE

======================================================================
*/

// * sword variable definitions, I have this here because the sword is most likely still unbalanced
//* I know this because I killed a super tank with it without taking ANY damaga

#define KNIFE_NORMAL_DAMAGE 20
#define KNIFE_DEATHMATCH_DAMAGE 30
#define KNIFE_KICK 200
#define KNIFE_RANGE 35
/*
=============
fire_knife

attacks with the beloved knife of the highlander
 edict_t *self - entity producing it, yourself
 vec3_t start - The place you are
 vec3_t aimdir - Where you are looking at in this case
 int damage - the damage the knife inflicts
 int kick - how much you want that bitch to be thrown back
=============
*/

void fire_knife ( edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{    
    trace_t tr; //detect whats in front of you up to range "vec3_t end"

    vec3_t end;

    // Figure out what we hit, if anything:

    VectorMA (start, KNIFE_RANGE, aimdir, end);  //calculates the range vector                      

    tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
                        // figuers out what in front of the player up till "end"
    
   // Figure out what to do about what we hit, if anything


	
 	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				//This tells us to damage the thing that in our path...hehe
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_KNIFE);
                gi.sound (self, CHAN_AUTO, gi.soundindex("weapons/tink1.wav") , 1, ATTN_NORM, 0);
                // Sound if hit player/monster -- added 1-13-98 by DanE

			}
	  		else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPARKS);   //Changed 1-13-98 by DanE to make impact look like sparks
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.multicast (tr.endpos, MULTICAST_PVS);

			   		if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
                gi.sound (self, CHAN_AUTO, gi.soundindex("weapons/oldtink1.wav") , 1, ATTN_NORM, 0); 
                //Sound if hit wall  -- added 1-13-98 by DanE
				}
			}
		}
	}
	return;
}

void knife_attack (edict_t *ent, vec3_t g_offset, int damage)
{
	vec3_t  forward, right;
	vec3_t  start;
	vec3_t  offset;

	if (is_quad)
		damage *= 4;
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
//	VectorSet(offset, 0, 0, ent->viewheight);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

// wave added
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
// wave added


	fire_knife (ent, start, forward, damage, KNIFE_KICK );
}

void Weapon_Knife_Fire (edict_t *ent)
{
	int damage;
	if (deathmatch->value)
		damage = KNIFE_DEATHMATCH_DAMAGE;
	else
		damage = KNIFE_NORMAL_DAMAGE;
	knife_attack (ent, vec3_origin, damage);
	ent->client->ps.gunframe++;
}

void Weapon_Knife (edict_t *ent)
{
	static int      pause_frames[]  = {19, 32, 0};
	static int      fire_frames[]   = {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, 0, 0, pause_frames, fire_frames, Weapon_Knife_Fire);
}

//
//!Pridkett
/*
======================================================================

GRENADE

======================================================================
*/

#define GRENADE_TIMER		3.0
#define GRENADE_MINSPEED	400
#define GRENADE_MAXSPEED	800

void weapon_grenade_fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 250;
	float	timer;
	int		speed;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
//	VectorSet(offset, 0, 0, ent->viewheight);

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
//		ent->client->ammo_clip--;

	ent->client->grenade_time = level.time + 1.0;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Weapon_Grenade (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;

			if (ent->client->pers.inventory[ent->client->ammo_index])
//			if (ent->client->pers.inventory[ent->client->ammo_clip])
//			if (ent->client->ammo_clip > 0)
			{
// removal of that twisting animation (looks dumb) .ion
//				ent->client->ps.gunframe = 1;
				ent->client->ps.gunframe = 6;

// removal of that twisting animation (looks dumb) .ion
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
// removal of that twisting animation (looks dumb) .ion
//		if (ent->client->ps.gunframe == 5)
//			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);
// removal of that twisting animation (looks dumb) .ion

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_grenade_fire (ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			weapon_grenade_fire (ent, false);
		}

		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}


/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void weapon_grenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 120;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;


	VectorSet(offset, 8, 8, ent->viewheight-8);
//	VectorSet(offset, 0, 0, ent->viewheight);


	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	// Set variables for MuzzleFlash first
		VectorSet(offset, 20, 3, ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		//MuzzleFlash_Create(ent);

/*	// Muzzle Flash
	VectorSet(offset, 20, 7, ent->viewheight-5);
	VectorSet(offset, 20, 7, ent->viewheight-5);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	MuzzleFlash_Create(start, forward, "rcp90");
*/
	fire_grenade (ent, start, forward, damage, 600, 2.5, radius);
//    fire_grenade (ent, start, forward, damage, 600, radius);
	gi.WriteByte (svc_muzzleflash);

	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);

	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->ammo_clip--;
}

void Weapon_GrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};
//	static int	fire_frames[]	= {9, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, 0, 0, pause_frames, fire_frames, weapon_grenadelauncher_fire);
//	Weapon_Generic (ent, 9, 16, 59, 64, 0, 0, pause_frames, fire_frames, weapon_grenadelauncher_fire);
}
/*
======================================================================

ROCKET

======================================================================
*/

void Weapon_RocketLauncher_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = 2500 + (int)(random() * 20.0);
	radius_damage = 240;
	damage_radius = 240;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;


//	VectorSet(offset, 0, 0, ent->viewheight);



//	VectorSet(offset, 8, 8, ent->viewheight-8);
	VectorSet(offset, 16, 16, ent->viewheight-8);

/*	// Set variables for MuzzleFlash first
		VectorSet(offset, 20, 3, ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		//MuzzleFlash_Create(ent);

*/
/*	// Muzzle Flash
	VectorSet(offset, 20, 7, ent->viewheight-5);
	VectorSet(offset, 20, 7, ent->viewheight-5);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	MuzzleFlash_Create(start, forward, "rcp90");
*/
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rocket (ent, start, forward, damage, 650, damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);


	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_ROCKET | is_silenced);

    gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->ammo_clip--;
}

void Weapon_RocketLauncher (edict_t *ent)
{
//	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int  pause_frames[]  = {50, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 3, 12, 50, 54, 84, 0, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
}

/*	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 3, 18, 56, 61, 0, 0, pause_frames, fire_frames, weapon_railgun_fire);

	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0}; */

/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/

void Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	
	
// start IoN
	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

//end ion   */
	if (is_quad)
		damage *= 4;

//	if (ent->client->weaponstate == WEAPON_FIRING && ent->client->pers.weapon == FindItem("PP7 Silenced"))
//			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/ppksilenced.wav"), 1, ATTN_NORM, 0);	
	
	

//	if (ent->client->weaponstate == WEAPON_FIRING)
//			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hyprbf1a.wav"), 1, ATTN_NORM, 0);

//END     */


	AngleVectors (ent->client->v_angle, forward, right, NULL);
//	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorSet(offset, 0, 0, ent->viewheight);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	/* Set variables for MuzzleFlash first
		VectorSet(offset, 20, 3, ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		MuzzleFlash_Create(ent);*/

/*	// Muzzle Flash
	VectorSet(offset, 20, 7, ent->viewheight-5);
	VectorSet(offset, 20, 7, ent->viewheight-5);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	MuzzleFlash_Create(start, forward, "rcp90");
*/

//	fire_blaster (ent, start, forward, damage, 1000, effect, hyper);
	fire_bullet (ent, start, forward, damage, 0, DEFAULT_PP7_HSPREAD, DEFAULT_PP7_VSPREAD, MOD_PP7);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);

	gi.WriteShort (ent-g_edicts);
//	if (hyper)
//		gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
//	else
  	gi.WriteByte (MZ_BLASTER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);


	PlayerNoise(ent, start, PNOISE_WEAPON);
//shit added by IoN_PuLse

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->ammo_clip--;
/*
	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}            */
}


void Weapon_Blaster_Fire (edict_t *ent)
{
	int		damage;
//   	vec3_t tempvec;

	if (deathmatch->value)
		damage = 10;
	else
		damage = 10;
	Blaster_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	//ion
/*    VectorSet(tempvec, 0, 8, 0);
    VectorAdd(tempvec, vec3_origin, tempvec);
    Blaster_Fire (ent, tempvec, damage, false, EF_BLASTER);

    VectorSet(tempvec, 0, -8, 0);
    VectorAdd(tempvec, vec3_origin, tempvec);
    Blaster_Fire (ent, tempvec, damage, false, EF_BLASTER);     */

    ent->client->ps.gunframe++;
}

void Weapon_Blaster (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 54, 60, 0, pause_frames, fire_frames, Weapon_Blaster_Fire);
}

/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/

void Blaster2_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	
	
// start IoN

//	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	if (ent->client->ammo_clip < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

//end ion   */
	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
//	VectorSet(offset, 24, 8, ent->viewheight-8);
	VectorSet(offset, 0, 0, ent->viewheight);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	// Set variables for MuzzleFlash first
//		VectorSet(offset, 20, 3, ent->viewheight-8);
//		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		//MuzzleFlash_Create(ent);
/*
	// Muzzle Flash
	VectorSet(offset, 20, 7, ent->viewheight-5);
	VectorSet(offset, 20, 7, ent->viewheight-5);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	MuzzleFlash_Create(start, forward, "rcp90");
*/
//	fire_blaster (ent, start, forward, damage, 1000, effect, hyper);
	fire_bullet (ent, start, forward, damage, 0, DEFAULT_PP7_HSPREAD, DEFAULT_PP7_VSPREAD, MOD_PP7);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);

	gi.WriteShort (ent-g_edicts);
//	if (hyper)
//		gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
//	else
  	gi.WriteByte (MZ_BLASTER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
//shit added by IoN_PuLse

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->ammo_clip--;
/*
	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}            */
}


void Weapon_Blaster2_Fire (edict_t *ent)
{
	int		damage;
//   	vec3_t tempvec;

	if (deathmatch->value)
		damage = 10;
	else
		damage = 10;
	Blaster2_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	//ion
/*    VectorSet(tempvec, 0, 8, 0);
    VectorAdd(tempvec, vec3_origin, tempvec);
    Blaster_Fire (ent, tempvec, damage, false, EF_BLASTER);

    VectorSet(tempvec, 0, -8, 0);
    VectorAdd(tempvec, vec3_origin, tempvec);
    Blaster_Fire (ent, tempvec, damage, false, EF_BLASTER);     */

    ent->client->ps.gunframe++;
}

void Weapon_Blaster2 (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, 0, 0, pause_frames, fire_frames, Weapon_Blaster2_Fire);
}

/*void Weapon_HyperBlaster_Fire (edict_t *ent)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;

	ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (! ent->client->pers.inventory[ent->client->ammo_index] )
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange (ent);
		}
		else
		{
			rotation = (ent->client->ps.gunframe - 5) * 2*M_PI/6;
			offset[0] = -4 * sin(rotation);
			offset[1] = 0;
			offset[2] = 4 * cos(rotation);

			if ((ent->client->ps.gunframe == 6) || (ent->client->ps.gunframe == 9))
				effect = EF_HYPERBLASTER;
			else
				effect = 0;
			if (deathmatch->value)
				damage = 10;
			else
				damage = 10;
			Blaster_Fire (ent, offset, damage, true, effect);
			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
				ent->client->pers.inventory[ent->client->ammo_index]--;

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
		}

		ent->client->ps.gunframe++;
		if (ent->client->ps.gunframe == 12 && ent->client->pers.inventory[ent->client->ammo_index])
			ent->client->ps.gunframe = 6;
	}

	if (ent->client->ps.gunframe == 12)
	{
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}

}

void Weapon_HyperBlaster (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};

	Weapon_Generic (ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
} */

/*
======================================================================

MACHINEGUN / CHAINGUN

======================================================================
*/

void Machinegun_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 8;
	int			kick = 2;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
//		Muzzleon(0);
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/ak471.wav"), 1, ATTN_NORM, 0);
//			Muzzleon(1);

	}

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;

//	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	if (ent->client->ammo_clip < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

	// raise the gun as it is firing
/*	if (!deathmatch->value)
	{
		ent->client->machinegun_shots++;
		if (ent->client->machinegun_shots > 9)
			ent->client->machinegun_shots = 9;
	}               */
//REMED by IoN_PuLse
	// get start / end positions

	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
//	VectorSet(offset, 0, 8, ent->viewheight-8);
	VectorSet(offset, 0, 0, ent->viewheight);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// Set variables for MuzzleFlash first
/*
	VectorSet(offset, 20, 3, ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
//		MuzzleFlash_Create(start, forward, "muzzle");
		//MuzzleFlash_Create(ent);
	
*/
	// Muzzle Flash
/*	VectorSet(offset, 20, 7, ent->viewheight-5);
	VectorSet(offset, 20, 7, ent->viewheight-5);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	MuzzleFlash_Create(start, forward, "rcp90");
*/
//	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

/* -=IoN_PuLse=- */
//	fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_MACHINEGUN);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_AK47);
	gi.WriteByte (svc_muzzleflash);
//	gi.WriteByte (TE_MUZZLEFLASH);

	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);

	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->ammo_clip--;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}

}

void Weapon_Machinegun (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

	Weapon_Generic (ent, 3, 5, 45, 49, 69, 0, pause_frames, fire_frames, Machinegun_Fire);
}

void Chaingun_Fire (edict_t *ent)
{
	int			i;
	int			shots;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset;
	int			damage;
	int			kick = 2;

	if (deathmatch->value)
		damage = 6;
	else
		damage = 8;

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&& ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22)
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
	}

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (ent->client->ps.gunframe & 1);
		ent->client->anim_end = FRAME_attack8;
	}

	if (ent->client->ps.gunframe <= 9)
		shots = 1;
	else if (ent->client->ps.gunframe <= 14)
	{
		if (ent->client->buttons & BUTTON_ATTACK)
			shots = 2;
		else
			shots = 1;
	}
	else
		shots = 3;

	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
		shots = ent->client->pers.inventory[ent->client->ammo_index];

	if (!shots)
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	for (i=0 ; i<shots ; i++)
	{
		// get start / end positions

		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
//    	VectorSet(offset, 0, 0, ent->viewheight);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);

	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);

	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}


void Weapon_Chaingun (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, 0, 0, pause_frames, fire_frames, Chaingun_Fire);
}


/*
======================================================================

SHOTGUN / SUPERSHOTGUN

======================================================================
*/

void weapon_shotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 4;
	int			kick = 8;

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

//	VectorSet(offset, 0, 8,  ent->viewheight-8);
	VectorSet(offset, 0, 0, ent->viewheight);


	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	if (deathmatch->value)
		fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
	else
		fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);

	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);

    gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->ammo_clip--;
}

void Weapon_Shotgun (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= {8, 9, 0};

	Weapon_Generic (ent, 7, 18, 36, 39, 0, 0, pause_frames, fire_frames, weapon_shotgun_fire);
}


void weapon_supershotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = 6;
	int			kick = 12;



	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	// Set variables for MuzzleFlash first

		VectorSet(offset, 20, 3, ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		//MuzzleFlash_Create(ent);

/*
	// Muzzle Flash
	VectorSet(offset, 20, 7, ent->viewheight-5);
	VectorSet(offset, 20, 7, ent->viewheight-5);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	MuzzleFlash_Create(start, forward, "rcp90");
*/
	VectorSet(offset, 0, 8,  ent->viewheight-8);
//	VectorSet(offset, 0, 0, ent->viewheight);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 5;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
	v[YAW]   = ent->client->v_angle[YAW] + 5;
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);

	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SSHOTGUN | is_silenced);

	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
		ent->client->ammo_clip -= 2;}

void Weapon_SuperShotgun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, 0, 0, pause_frames, fire_frames, weapon_supershotgun_fire);
}



/*
======================================================================

RAILGUN

======================================================================
*/

void weapon_railgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	if (deathmatch->value)
	{	// normal damage is too extreme in dm
		damage = 100;
		kick = 200;
	}
	else
	{
		damage = 150;
		kick = 250;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}


	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	// Set variables for MuzzleFlash first
		VectorSet(offset, 20, 3, ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		//MuzzleFlash_Create(ent);


/*	// Muzzle Flash
	VectorSet(offset, 20, 7, ent->viewheight-5);
	VectorSet(offset, 20, 7, ent->viewheight-5);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	MuzzleFlash_Create(start, forward, "rcp90");
*/

	VectorSet(offset, 0, 7,  ent->viewheight-8);
//	VectorSet(offset, 0, 0, ent->viewheight);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rail (ent, start, forward, damage, kick);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);

	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RAILGUN | is_silenced);

	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->ammo_clip--;
}


void Weapon_Railgun (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 3, 18, 56, 61, 0, 0, pause_frames, fire_frames, weapon_railgun_fire);
}


/*
======================================================================

BFG10K

======================================================================
*/

void weapon_bfg_fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius = 1000;

	if (deathmatch->value)
		damage = 200;
	else
		damage = 500;

	if (ent->client->ps.gunframe == 9)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);

		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BFG | is_silenced);

		gi.multicast (ent->s.origin, MULTICAST_PVS);

		ent->client->ps.gunframe++;

		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
	if (ent->client->pers.inventory[ent->client->ammo_index] < 50)
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (is_quad)
		damage *= 4;


	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom()*8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
//	VectorSet(offset, 0, 0, ent->viewheight);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bfg (ent, start, forward, damage, 400, damage_radius);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index] -= 50;
		ent->client->ammo_clip -= 50;
}

void Weapon_BFG (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	Weapon_Generic (ent, 8, 32, 55, 58, 0, 0, pause_frames, fire_frames, weapon_bfg_fire);
}


//======================================================================
/*
======================================================================

ION PROXIMITY MINES

======================================================================
*/

#define PROXMINE_TIMER		10.0
#define PROXMINE_MINSPEED	400
#define PROXMINE_MAXSPEED	800

void weapon_proxmine_fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 250;
	float	timer;
	int		speed;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
//	VectorSet(offset, 0, 0, ent->viewheight);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = PROXMINE_MINSPEED + (PROXMINE_TIMER - timer) * ((PROXMINE_MAXSPEED - PROXMINE_MINSPEED) / PROXMINE_TIMER);
	fire_proxmine (ent, start, forward, damage, speed, timer, radius, held);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
//		ent->client->ammo_clip--;

	ent->client->grenade_time = level.time + 1.0;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Weapon_Proxmine (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
//			if (ent->client->ammo_clip > 0)
			{
// removal of that twisting animation (looks dumb) .ion
				ent->client->ps.gunframe = 7;
// removal of that twisting animation (looks dumb) .ion
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
// removal of that twisting animation (looks dumb) .ion
//		if  (ent->client->ps.gunframe == 5)
//			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);
// removal of that twisting animation (looks dumb) .ion

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + PROXMINE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_proxmine_fire (ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			weapon_proxmine_fire (ent, false);
		}

		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}


//======================================================================
/*
======================================================================

ION REMOTE MINES

======================================================================
*/

#define REMOTE_TIMER		10.0
#define REMOTE_MINSPEED	400
#define REMOTE_MAXSPEED	800

void weapon_remote_fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 250;
	float	timer;
	int		speed;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
//	VectorSet(offset, 0, 0, ent->viewheight);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->remote_time - level.time;
	speed = REMOTE_MINSPEED + (REMOTE_TIMER - timer) * ((REMOTE_MAXSPEED - REMOTE_MINSPEED) / REMOTE_TIMER);
	fire_remote (ent, start, forward, damage, speed, timer, radius, held);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
//		ent->client->ammo_clip--;

	ent->client->remote_time = level.time + 1.0;

	if(ent->deadflag || ent->s.modelindex != 255) // VWep animations screw up corpses
	{
		return;
	}

	if (ent->health <= 0)
		return;

	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->client->anim_priority = ANIM_ATTACK;
		ent->s.frame = FRAME_crattak1-1;
		ent->client->anim_end = FRAME_crattak3;
	}
	else
	{
		ent->client->anim_priority = ANIM_REVERSE;
		ent->s.frame = FRAME_wave08;
		ent->client->anim_end = FRAME_wave01;
	}
}

void Weapon_Remote (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
//			if (ent->client->ammo_clip > 0)
			{
// removal of that twisting animation (looks dumb) .ion
				ent->client->ps.gunframe = 6;
// removal of that twisting animation (looks dumb) .ion
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->remote_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
// removal of that twisting animation (looks dumb) .ion
//		if (ent->client->ps.gunframe == 5)
//			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);
// removal of that twisting animation (looks dumb) .ion

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->remote_time)
			{
				ent->client->remote_time = level.time + REMOTE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->remote_blew_up && level.time >= ent->client->remote_time)
			{
				ent->client->weapon_sound = 0;
				weapon_remote_fire (ent, true);
				ent->client->remote_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->remote_blew_up)
			{
				if (level.time >= ent->client->remote_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->remote_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			weapon_remote_fire (ent, false);
		}

		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->remote_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
			ent->client->remote_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

/*
======================================================================

LASER

======================================================================
*/


void Laser_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
// start IoN
//    	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
//	{
//		ent->client->ps.gunframe = 6;
//		if (level.time >= ent->pain_debounce_time)
//		{
//			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
//			ent->pain_debounce_time = level.time + 1;
//		}
//		NoAmmoWeaponChange (ent);
//		return;
//	}
//end ion   
	if (is_quad)
		damage *= 4;

	if (ent->client->weaponstate == WEAPON_FIRING)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hyprbf1a.wav"), 1, ATTN_NORM, 0);
            
/*	// Set variables for MuzzleFlash first
		VectorSet(offset, 20, 3, ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		//MuzzleFlash_Create(ent);
*/

/*	// Muzzle Flash
	VectorSet(offset, 20, 7, ent->viewheight-5);
	VectorSet(offset, 20, 7, ent->viewheight-5);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	MuzzleFlash_Create(start, forward, "rcp90");
*/
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 1, 3.8, ent->viewheight-2.5);
//	VectorSet(offset, 0, 3, ent->viewheight-3);
//	VectorSet(offset, 0, 0, ent->viewheight);
	VectorAdd (offset, g_offset, offset);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

//	fire_blaster (ent, start, forward, damage, 1000, effect, hyper);
//	fire_laser (ent, start, forward, damage, 1000, effect);
//	VectorSet(offset, 1, 3.8, ent->viewheight-2.5);
	VectorSet(offset, 0, 0, ent->viewheight);

	fire_laser (ent, start, forward, damage, 2000, effect);
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);

	gi.WriteShort (ent-g_edicts);
//	if (hyper)
//		gi.WriteByte (MZ_HYPERBLASTER | is_silenced);
//	else
		gi.WriteByte (MZ_BLASTER | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
//shit added by IoN_PuLse

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}           
}

void Weapon_Laser_Fire (edict_t *ent)
{
	int		damage;

	if (deathmatch->value)
		damage = 25;
	else
		damage = 25;
	Laser_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
	//ion
/*    VectorSet(tempvec, 0, 8, 0);
    VectorAdd(tempvec, vec3_origin, tempvec);
    Blaster_Fire (ent, tempvec, damage, false, EF_BLASTER);

    VectorSet(tempvec, 0, -8, 0);
    VectorAdd(tempvec, vec3_origin, tempvec);
    Blaster_Fire (ent, tempvec, damage, false, EF_BLASTER);      */

    ent->client->ps.gunframe++;
}

void Weapon_Laser (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, 0, 0, pause_frames, fire_frames, Weapon_Laser_Fire);
}      

/*
======================================================================

ZMG 9mm UZI

======================================================================
*/

void ZMG_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 8;
	int			kick = 2;
	vec3_t		offset;
//	ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
//    ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");
		ent->client->ps.gunframe++;
//        PNOISE_WEAPON
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/zmg1.wav"), 1, ATTN_NORM, 0);

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
//    ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");
	else
		ent->client->ps.gunframe = 5;

//    ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");

//	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	if (ent->client->ammo_clip < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);

			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
//	ent->client->kick_origin[0] = crandom() * 0.35;
//	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;
	// raise the gun as it is firing
	if (!deathmatch->value)
	{
		ent->client->machinegun_shots++;
		if (ent->client->machinegun_shots > 9)
			ent->client->machinegun_shots = 9;
	}
//commented by IoN_PuLse
	// get start / end positions

	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
//	VectorSet(offset, 0, 8, ent->viewheight-8);
	VectorSet(offset, 0, 0, ent->viewheight);
	// Set variables for MuzzleFlash first

//		VectorSet(offset, 20, 3, ent->viewheight-8);
//		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		//MuzzleFlash_Create(ent);

/*
	// Muzzle Flash
	VectorSet(offset, 20, 7, ent->viewheight-5);
	VectorSet(offset, 20, 7, ent->viewheight-5);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	MuzzleFlash_Create(start, forward, "rcp90");
*/
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
/* -=IoN_PuLse=- */
//	fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_MACHINEGUN);

//		if (ent->client->pers.inventory[ent->client->ammo_index] == 1 || (ent->client->ZMG_rds == 1))
//		{
//			//Hard coded for reload only.
//			ent->client->ps.gunframe=49;
//			ent->client->weaponstate = WEAPON_END_MAG;
//	fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_MACHINEGUN);
//			ent->client->ZMG_rds--;
//		}
//		else
//		{
			//If no reload, fire normally.
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_ZMG);

			//+BD and uncomment these two also
//			ent->client->ZMG_rds--;
//		}

		
/*			if (ent->client->pers.inventory[ent->client->ammo_index] == 1 || (ent->client->ZMG_rds == 1))
		{
			//Hard coded for reload only.
			ent->client->ps.gunframe=0;
			ent->client->weaponstate = WEAPON_END_MAG;
//			fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD,MOD_Mk23);
			fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_MACHINEGUN);

			ent->client->ZMG_rds--;
		}
		else
		{


	fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_MACHINEGUN);
//    fire_lead (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_MACHINEGUN);

	//+BD and uncomment these two also
			ent->client->ZMG_rds--;
		}*/

	gi.WriteByte (svc_muzzleflash);

	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);

	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->ammo_clip--;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}

//ent->client->pers.inventory[ent->client->ammo_index] -= ent->client->pers.weapon->quantity;
//   if (self->client)
//    PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}

void Weapon_ZMG (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

//	Weapon_Generic (ent, 3, 5, 45, 49, 0, 0, pause_frames, fire_frames, ZMG_Fire);
	Weapon_Generic (ent, 3, 5, 45, 49, 72, 54, pause_frames, fire_frames, ZMG_Fire);
}

//-----------------------------------------------------------------------------
// end of ZMG 9mm UZI CODE

/*
======================================================================

DUAL ZMG 9mm UZIs

======================================================================
*/

void DualZMG_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 8;
	int			kick = 2;
	vec3_t		offset;
//	ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
//    ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");
		ent->client->ps.gunframe++;
//        PNOISE_WEAPON
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/zmg1.wav"), 1, ATTN_NORM, 0);


	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
//    ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");
	else
		ent->client->ps.gunframe = 5;

//    ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");

//	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	if (ent->client->ammo_clip < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);

			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
//	ent->client->kick_origin[0] = crandom() * 0.35;
//	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;
	// raise the gun as it is firing
	if (!deathmatch->value)
	{
		ent->client->machinegun_shots++;
		if (ent->client->machinegun_shots > 9)
			ent->client->machinegun_shots = 9;
	}
//commented by IoN_PuLse
	// get start / end positions

	// Set variables for MuzzleFlash first

//		VectorSet(offset, 20, 3, ent->viewheight-8);
//		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		//MuzzleFlash_Create(ent);

/*
	// Muzzle Flash
	VectorSet(offset, 20, 7, ent->viewheight-5);
	VectorSet(offset, 20, 7, ent->viewheight-5);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	MuzzleFlash_Create(start, forward, "rcp90");
*/
	VectorSet(offset, 0, -8, ent->viewheight);
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
//	VectorSet(offset, 0, 8, ent->viewheight-8);
//	VectorSet(offset, 0, -8, 0);


	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
/* -=IoN_PuLse=- */
//	fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_MACHINEGUN);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_DUALZMG);
//    fire_lead (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_MACHINEGUN);
	VectorSet(offset, 0, 8, ent->viewheight);
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);

	// Set variables for MuzzleFlash first
//		VectorSet(offset, 20, 3, ent->viewheight-8);
//		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		//MuzzleFlash_Create(ent);

/*
	// Muzzle Flash
	VectorSet(offset, 20, 7, ent->viewheight-5);
	VectorSet(offset, 20, 7, ent->viewheight-5);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	MuzzleFlash_Create(start, forward, "rcp90");
*/	
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_DUALZMG);

// double firing


//	AngleVectors (angles, forward, right, NULL);


//	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
//	gi.WriteByte (svc_muzzleflash);
	gi.WriteByte (svc_muzzleflash);

	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);

	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;
//		ent->client->ammo_clip =- 2;
		ent->client->ammo_clip--;
		ent->client->ammo_clip--;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}

//    if (self->client)
//    PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}

void Weapon_DualZMG (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

	Weapon_Generic (ent, 3, 5, 45, 49, 72, 0, pause_frames, fire_frames, DualZMG_Fire);
}

//-----------------------------------------------------------------------------
// end of ZMG 9mm UZI CODE

/*
======================================================================

AR33 Assault Rifle

======================================================================
*/

void AssRifle_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 10;
	int			kick = 2;
	vec3_t		offset;
//	ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
//    ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");
		ent->client->ps.gunframe++;
//        PNOISE_WEAPON
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/ar33.wav"), 1, ATTN_NORM, 0);


	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
//    ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");
	else
		ent->client->ps.gunframe = 5;

//    ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");

//	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	if (ent->client->ammo_clip < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);

			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
//	ent->client->kick_origin[0] = crandom() * 0.35;
//	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;
	// raise the gun as it is firing
	if (!deathmatch->value)
	{
		ent->client->machinegun_shots++;
		if (ent->client->machinegun_shots > 9)
			ent->client->machinegun_shots = 9;
	}
//commented by IoN_PuLse
	// get start / end positions

	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
//	VectorSet(offset, 0, 8, ent->viewheight-8);
	VectorSet(offset, 0, 0, ent->viewheight);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// Set variables for MuzzleFlash first

//		VectorSet(offset, 20, 3, ent->viewheight-8);
//		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		//MuzzleFlash_Create(ent);
	
/*
	// Muzzle Flash
	VectorSet(offset, 20, 7, ent->viewheight-5);
	VectorSet(offset, 20, 7, ent->viewheight-5);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	MuzzleFlash_Create(start, forward, "rcp90");
*/
/* -=IoN_PuLse=- */
//	fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_MACHINEGUN);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_AR33);
//    fire_lead (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_MACHINEGUN);
//	gi.WriteByte (svc_muzzleflash);
//	gi.WriteByte (svc_temp_entity);
//		gi.WriteByte (TE_MUZZLEFLASH);
//	gi.WriteByte (svc_temp_entity);
//	gi.WriteByte (TE_MUZZLEFLASH);
//	gi.WritePosition (ent->s.origin);
//		gi.WriteDir (vec3_origin);
//	gi.multicast (ent->s.origin, MULTICAST_PVS);

//	gi.WriteShort (ent-g_edicts);
/*	gi.WriteByte (svc_muzzleflash);
	gi.WriteByte (svc_temp_entity);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
*/
	gi.WriteByte (svc_muzzleflash);

	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);

	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->ammo_clip--;
	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}

//    if (self->client)
//    PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}

void Weapon_AssRifle (edict_t *ent)
{
//	static int	pause_frames[]	= {23, 45, 0};
	static int	pause_frames[]	= {9, 28, 0};
//	static int	fire_frames[]	= {4, 5, 0};
	static int	fire_frames[]	= {4, 5, 0};

	Weapon_Generic (ent, 3, 8, 28, 32, 52, 0, pause_frames, fire_frames, AssRifle_Fire);
}

//-----------------------------------------------------------------------------
// end of AR33 Assault Rifle Code

/*
======================================================================

RCP-90

======================================================================
*/

void RCP90_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 8;
	int			kick = 2;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING) {
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/machgf1b.wav"), 1, ATTN_NORM, 0);
	}

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;


	if (ent->client->ammo_clip < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);

			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
	if (!deathmatch->value)
	{
		ent->client->machinegun_shots++;
		if (ent->client->machinegun_shots > 9)
			ent->client->machinegun_shots = 9;
	}
//commented by IoN_PuLse
	// get start / end positions

	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);


	// Set variables for MuzzleFlash first

//		VectorSet(offset, 20, 3, ent->viewheight-8);
//		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
//		MuzzleFlash_Create(start, forward, "rcp_90");


	// Muzzle Flash
/*	VectorSet(offset, 20, 7, ent->viewheight-5);
	VectorSet(offset, 20, 7, ent->viewheight-5);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	MuzzleFlash_Create(start, forward, "rcp90");

*/		
//	VectorSet(offset, 0, 8, ent->viewheight-8);
	VectorSet(offset, 0, 0, ent->viewheight);

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_RCP90);

	gi.WriteByte (svc_muzzleflash);

	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);

	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->ammo_clip--;

	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}

//    if (self->client)
//    PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}

void Weapon_RCP90 (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
//	static int	fire_frames[]	= {4, 5, 0};
	static int	fire_frames[]	= {4, 5, 0};

//	Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, RCP90_Fire);
	Weapon_Generic (ent, 3, 5, 45, 49, 67, 0, pause_frames, fire_frames, RCP90_Fire);

}

//-----------------------------------------------------------------------------
// end of RCP-90 Code

/*
======================================================================

D5K

======================================================================
*/

void D5K_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = 10;
	int			kick = 2;
	vec3_t		offset;
//	ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
//    ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");
		ent->client->ps.gunframe++;
//        PNOISE_WEAPON
		return;
	}

//	if (ent->client->weaponstate == WEAPON_FIRING)
//			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/ar33.wav"), 1, ATTN_NORM, 0);


	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
//    ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");
	else
		ent->client->ps.gunframe = 5;

//    ent->client->weapon_sound = gi.soundindex("weapons/zmg1.wav");

//	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	if (ent->client->ammo_clip < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);

			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
//	ent->client->kick_origin[0] = crandom() * 0.35;
//	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;
	// raise the gun as it is firing
	if (!deathmatch->value)
	{
		ent->client->machinegun_shots++;
		if (ent->client->machinegun_shots > 9)
			ent->client->machinegun_shots = 9;
	}
//commented by IoN_PuLse
	// get start / end positions

	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
//	VectorSet(offset, 0, 8, ent->viewheight-8);
	VectorSet(offset, 0, 0, ent->viewheight);
	// Set variables for MuzzleFlash first

//		VectorSet(offset, 20, 3, ent->viewheight-8);
//		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		//MuzzleFlash_Create(ent);


/*	// Muzzle Flash
	VectorSet(offset, 20, 7, ent->viewheight-5);
	VectorSet(offset, 20, 7, ent->viewheight-5);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	MuzzleFlash_Create(start, forward, "rcp90");
*/
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
/* -=IoN_PuLse=- */
//	fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_MACHINEGUN);
	fire_bullet (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_D5K);
//    fire_lead (ent, start, forward, damage, kick, DEFAULT_KF7_HSPREAD, DEFAULT_KF7_VSPREAD, MOD_MACHINEGUN);
	gi.WriteByte (svc_muzzleflash);

	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);

	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//		ent->client->pers.inventory[ent->client->ammo_index]--;
		ent->client->ammo_clip--;
	ent->client->anim_priority = ANIM_ATTACK;
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
		ent->s.frame = FRAME_crattak1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_crattak9;
	}
	else
	{
		ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
		ent->client->anim_end = FRAME_attack8;
	}

//    if (self->client)
//    PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}

void Weapon_D5K (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

	Weapon_Generic (ent, 3, 5, 45, 49, 0, 0, pause_frames, fire_frames, D5K_Fire);
}

//-----------------------------------------------------------------------------
// end of D5K Code
// +DL - Load Ammo
void LoadAmmo(edict_t *ent)
{
	// Set size of clip based on weapon
	// DL:TODO - Add All Clip Based Weapons Here

	int ClipSize = 0;

//+ ION FIX
	if ((ent->client->pers.weapon == FindItem ("Slapper")) ||
		(ent->client->pers.weapon == FindItem ("Grenades")) ||
		(ent->client->pers.weapon == FindItem ("Proximity Mines")) ||
		(ent->client->pers.weapon == FindItem ("Laser")) ||
 		(ent->client->pers.weapon == FindItem ("Remote Mines")))
		return;
//-ION FIX

 	if(ent->deadflag == DEAD_DEAD)
	return;

	if (ent->client->pers.weapon == FindItem ("PP7"))
		ClipSize = 7;

//	if (ent->client->pers.weapon == FindItem ("PP7 Silenced"))
//		ClipSize = 7;

/*	if (ent->client->pers.weapon == FindItem ("Grenades"))
		ClipSize = 1;
*/
	if (ent->client->pers.weapon == FindItem ("ZMG 9mm"))
		ClipSize = 32;

	if (ent->client->pers.weapon == FindItem ("Dual ZMG 9mm's"))
		ClipSize = 64;

	if (ent->client->pers.weapon == FindItem ("KF7 Soviet"))
		ClipSize = 30;

	if (ent->client->pers.weapon == FindItem ("AR33 Assault Rifle"))
		ClipSize = 30;

	if (ent->client->pers.weapon == FindItem ("Rocket Launcher"))
		ClipSize = 1;
/*
	if (ent->client->pers.weapon == FindItem ("Remote Mines"))
		ClipSize = 1;

	if (ent->client->pers.weapon == FindItem ("Proximity Mines"))
		ClipSize = 1;
*/
	if (ent->client->pers.weapon == FindItem ("RCP-90"))
		ClipSize = 80;

		/*if(!strcmp(ent->client->pers.weapon->pickup_name, "Shotgun"))
		ClipSize = 3;
	if(!strcmp(ent->client->pers.weapon->pickup_name, "Super Shotgun"))
		ClipSize = 5;
	if(!strcmp(ent->client->pers.weapon->pickup_name, "Machinegun"))
		ClipSize = 20;
	if(!strcmp(ent->client->pers.weapon->pickup_name, "Rocket Launcher"))
		ClipSize = 1;*/

/*	if (ent->client->ammo_clip == ClipSize)
		return;

	ent->client->pers.inventory[ent->client->ammo_index] += ent->client->ammo_clip;
	ent->client->ammo_clip = 0;
*/
	// Load Ammo
	if (ent->client->pers.inventory[ent->client->ammo_index] >= ClipSize)
	{
		ent->client->ammo_clip = ClipSize;
		ent->client->pers.inventory[ent->client->ammo_index] -= ClipSize;
	}
	else
	{
		ent->client->ammo_clip = ent->client->pers.inventory[ent->client->ammo_index];
		ent->client->pers.inventory[ent->client->ammo_index] = 0;
	}
/*	// Redisplay Weapon
	if (!ent->client->chasecam){
	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
	ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
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
			
	}*/
}
// -DL

