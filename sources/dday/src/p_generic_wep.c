/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/p_generic_wep.c,v $
 *   $Revision: 1.23 $
 *   $Date: 2002/06/04 19:49:48 $
 * 
 ***********************************

Copyright (C) 2002 Vipersoft

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

void NoAmmoWeaponChange (edict_t *ent);


/*
================
Weapon_Generic

A generic function to handle the basics of weapon thinking
================
*/
#define FRAME_LFIRE_FIRST		(FRAME_ACTIVATE_LAST + 1)
#define FRAME_LIDLE_FIRST		(FRAME_LFIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST	(FRAME_LASTRD_LAST + 1)

//Added to incorporate reload and last round animations
#define FRAME_RELOAD_FIRST		(FRAME_LIDLE_LAST +1)
#define FRAME_LASTRD_FIRST		(FRAME_RELOAD_LAST +1)

//Added for the Hawkins Truesite system
#define FRAME_RAISE_FIRST		(FRAME_DEACTIVATE_LAST+1)
#define FRAME_AFIRE_FIRST		(FRAME_RAISE_LAST+1)
#define FRAME_AIDLE_FIRST		(FRAME_AFIRE_LAST+1)

void Weapon_Generic (edict_t *ent, 
					 int FRAME_ACTIVATE_LAST,	int FRAME_LFIRE_LAST,	int FRAME_LIDLE_LAST, 
					 int FRAME_RELOAD_LAST,		int FRAME_LASTRD_LAST,	int FRAME_DEACTIVATE_LAST,
					 int FRAME_RAISE_LAST,		int FRAME_AFIRE_LAST,	int FRAME_AIDLE_LAST,
					 int *pause_frames,			int *fire_frames,		void (*fire)(edict_t *ent))
{
	int		i, n;
	gitem_t *ammo_item;

	int		ammo_index,	*ammo_ammount;
	int		FRAME_FIRE_FIRST,FRAME_IDLE_FIRST,FRAME_IDLE_LAST;

	
	FRAME_FIRE_FIRST = (ent->client->aim)?FRAME_AFIRE_FIRST:FRAME_LFIRE_FIRST;
	//FRAME_FIRE_LAST = (ent->client->aim)?FRAME_AFIRE_LAST:FRAME_LFIRE_LAST;
	FRAME_IDLE_FIRST = (ent->client->aim)?FRAME_AIDLE_FIRST:FRAME_LIDLE_FIRST;
	FRAME_IDLE_LAST = (ent->client->aim)?FRAME_AIDLE_LAST:FRAME_LIDLE_LAST;

	if (ent->client->pers.weapon &&
		ent->client->pers.weapon->pickup_name &&
		frame_output)
		gi.dprintf("%i / %i - %s\n", ent->client->weaponstate, ent->client->ps.gunframe, ent->client->pers.weapon->pickup_name);

	if(ent->client->pers.weapon->ammo)
	{
		ammo_item = FindItem(ent->client->pers.weapon->ammo);
		ammo_index = ITEM_INDEX(ammo_item);
		ammo_ammount=&ent->client->pers.inventory[ammo_index];
	}

//	gi.dprintf(" %i < %i < %i\n",FRAME_RAISE_FIRST, ent->client->ps.gunframe, FRAME_RAISE_LAST);

	if (ent->client->aim && 
		ent->client->pers.weapon->position != LOC_SNIPER &&
		ent->client->ps.gunframe >= FRAME_RAISE_LAST - 1 )
			ent->client->ps.fov = TS_FOV;

	else if (!ent->client->aim && ent->client->pers.weapon->position != LOC_SNIPER)		
			ent->client->ps.fov = STANDARD_FOV;

	
	if( ent->client->weaponstate == WEAPON_RELOADING)
	{
		ent->client->ps.fov=STANDARD_FOV; // reset sniper

		if (ent->client->pers.weapon->position == LOC_SNIPER)
			ent->client->sniper_loaded[ent->client->resp.team_on->index] = true;

			// pbowens: show reloading
		if (ent->client->ps.gunframe == FRAME_RELOAD_FIRST) {
			ent->client->anim_priority = ANIM_REVERSE;
            if (ent->stanceflags == STANCE_STAND)
            {
                ent->s.frame = FRAME_pain304+1;
                ent->client->anim_end = FRAME_pain301;            
            }
            else if (ent->stanceflags == STANCE_DUCK)
            {
                ent->s.frame = FRAME_crpain4+1;
                ent->client->anim_end = FRAME_crpain1;
            }
            else if (ent->stanceflags == STANCE_CRAWL)
            {
                ent->s.frame = FRAME_crawlpain04+1;
                ent->client->anim_end = FRAME_crawlpain01;
            }
		}
            
		if (ent->client->aim) 
		{
			if(ent->client->ps.gunframe==FRAME_RAISE_FIRST)
			{
				ent->client->aim=false;
				//ent->client->weaponstate=WEAPON_READY;
			} 
			else if (ent->client->ps.gunframe > FRAME_RAISE_LAST || ent->client->ps.gunframe < FRAME_RAISE_FIRST)
				ent->client->ps.gunframe=FRAME_RAISE_LAST;
			else  
				ent->client->ps.gunframe--;

			return;
		}
		
		if (ent->client->ps.gunframe < FRAME_RELOAD_FIRST || ent->client->ps.gunframe > FRAME_RELOAD_LAST)
			ent->client->ps.gunframe = FRAME_RELOAD_FIRST;

		else if(ent->client->ps.gunframe < FRAME_RELOAD_LAST)
		{ 
			ent->client->ps.gunframe++;             
			//Check weapon to find out when to play reload sounds
			//pbowens: it's now defined in the user dll

				// play user sounds
			if (ent->client->pers.weapon->guninfo) {
				for (i = 0; ent->client->pers.weapon->guninfo->RSoundFrames1[i]; i++) {
					if (ent->client->ps.gunframe == ent->client->pers.weapon->guninfo->RSoundFrames1[i]) 
						gi.sound(ent, CHAN_WEAPON, gi.soundindex(ent->client->pers.weapon->guninfo->ReloadSound1), 1, ATTN_NORM, 0);
				}
				for (i = 0; ent->client->pers.weapon->guninfo->RSoundFrames2[i]; i++) {
					if (ent->client->ps.gunframe == ent->client->pers.weapon->guninfo->RSoundFrames2[i]) 
	 					gi.sound(ent, CHAN_WEAPON, gi.soundindex(ent->client->pers.weapon->guninfo->ReloadSound2), 1, ATTN_NORM, 0);
				}
			}
		}
        else
		{
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
            ent->client->weaponstate = WEAPON_READY;

			if (ent->client->pers.weapon->topoff==1)
			{
				if (*ammo_ammount) //partial mags
				{
					ent->client->pers.inventory[ammo_index]--;

					if ( ((*ent->client->p_fract) += (*ent->client->p_rnd)) >= ammo_item->quantity)
					{
						*ent->client->p_fract -= ammo_item->quantity;
						ent->client->pers.inventory[ammo_index]++;
					}

					if (ent->client->pers.inventory[ammo_index]) 
						*ent->client->p_rnd = ammo_item->quantity;
					else 
						*ent->client->p_rnd= *ent->client->p_fract;				
				}
               
			}

			else if(ent->client->pers.weapon->topoff==2)//for beltfed
			{
				if(*ammo_ammount)
				{
					ent->client->pers.inventory[ammo_index]--;
					*ent->client->p_rnd = ammo_item->quantity;
				}
			}
			
			else 
			{
				if(*ammo_ammount) //feeder clips (not topoffable
				{
					ent->client->pers.inventory[ammo_index]--;
					*ent->client->p_rnd = ammo_item->quantity;		
				}
			}

			
		} 
	} 
//Empty or unloaded weapon
	if( ent->client->weaponstate == WEAPON_END_MAG)
	{
		//gi.dprintf("%i - %i\n", FRAME_LASTRD_FIRST, FRAME_LASTRD_LAST);

		ent->client->ps.fov = STANDARD_FOV;

		if (ent->client->pers.weapon->position == LOC_SNIPER)
			ent->client->sniper_loaded[ent->client->resp.team_on->index] = false;

		// pbowens: there is no LastRound animation, skip
		if (FRAME_LASTRD_LAST == FRAME_RELOAD_LAST)
		{
			ent->client->ps.gunframe = FRAME_LIDLE_FIRST;
			ent->client->weaponstate = WEAPON_READY;
			ent->client->aim = false;
			WeighPlayer(ent);
			return;
		}

		if (ent->client->ps.gunframe < FRAME_LASTRD_FIRST)
			ent->client->ps.gunframe = FRAME_LASTRD_FIRST;

		else if (ent->client->ps.gunframe < FRAME_LASTRD_LAST)
			ent->client->ps.gunframe++;
		else 
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->aim = false; // disable truesight
			WeighPlayer(ent);
		}

		return;
	}
 
	if(ent->s.modelindex != 255) //pbowens: v_wep
        return; // not on client, so VWep animations could do wacky things
	

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->drop || ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ent->client->drop = false;
			ent->client->aim  = false;
			ChangeWeapon (ent);
			return;
		}		
        else if((FRAME_DEACTIVATE_LAST - ent->client->ps.gunframe) == 4) //pbowens: v_wep
        {
            ent->client->anim_priority = ANIM_REVERSE;

            if (ent->stanceflags == STANCE_STAND)
            {
                ent->s.frame = FRAME_pain304+1;
                ent->client->anim_end = FRAME_pain301;            
            }
            else if (ent->stanceflags == STANCE_DUCK)
            {
                ent->s.frame = FRAME_crpain4+1;
                ent->client->anim_end = FRAME_crpain1;
            }
            else if (ent->stanceflags == STANCE_CRAWL)
            {
                ent->s.frame = FRAME_crawlpain04+1;
                ent->client->anim_end = FRAME_crawlpain01;
            }


			

        } //end v_wep

		ent->client->ps.gunframe++;
		return;
	}
	
	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->pers.weapon->position == LOC_SNIPER)
			ent->client->sniper_loaded[ent->client->resp.team_on->index] = true;

		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		{
			if( (ent->client->p_rnd) && (!*ent->client->p_rnd))
			{
				if((ent->client->p_fract)&&(*ent->client->p_fract));
				else
				{ //load the weapon initially.
					ammo_item=FindItem(ent->client->pers.weapon->ammo);

					// Next two lines commented to fix reload bug.
					// Forces user to load his own damn guns.
					if (ent->client->pers.weapon->guninfo) {
						*ent->client->p_rnd = ent->client->pers.weapon->guninfo->rnd_count;
						ent->client->pers.weapon->guninfo->rnd_count = 0; // make null after use
					}

					//ent->client->pers.inventory[ITEM_INDEX(ammo_item)]--;
				}  
			}
			
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		if(stricmp(ent->client->pers.weapon->pickup_name, "BHMG") == 0) 
		{
			if(ent->client->ps.gunframe==18)
				gi.sound(ent,CHAN_WEAPON,gi.soundindex("weapons/sshotr1b.wav"), 1, ATTN_NORM, 0);
			else if(ent->client->ps.gunframe==13)
				gi.sound(ent,CHAN_WEAPON,gi.soundindex("weapons/grenl1b1.wav"), 1, ATTN_NORM, 0);
		}

 		//if(ent->client->ps.gunframe>FRAME_ACTIVATE_LAST)
		//	ent->client->ps.gunframe=FRAME_ACTIVATE_LAST;
 		//else 
		//  ent->client->ps.gunframe++;

		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;

		//pbowens: v_wep
		if((FRAME_DEACTIVATE_LAST - FRAME_DEACTIVATE_FIRST) < 4)
        {
            ent->client->anim_priority = ANIM_REVERSE;
            if (ent->stanceflags == STANCE_STAND)
            {
                ent->s.frame = FRAME_pain304+1;
                ent->client->anim_end = FRAME_pain301;            
            }
            else if (ent->stanceflags == STANCE_DUCK)
            {
                ent->s.frame = FRAME_crpain4+1;
                ent->client->anim_end = FRAME_crpain1;
            }
            else if (ent->stanceflags == STANCE_CRAWL)
            {
                ent->s.frame = FRAME_crawlpain04+1;
                ent->client->anim_end = FRAME_crawlpain01;
            }
        }

		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{

			// pbowens: weapons may not fire in water
			if (ent->waterlevel > 2 && 
				ent->client->pers.weapon->position != LOC_KNIFE &&
				ent->client->pers.weapon->position != LOC_SPECIAL)
			{
				if (ent->client->buttons != ent->client->oldbuttons)
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
				//	ent->client->weaponstate = WEAPON_READY;
				//	ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				ent->client->ps.gunframe++;
				goto no_fire;
			}

			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			ent->client->ps.gunframe = FRAME_FIRE_FIRST;
			ent->client->weaponstate = WEAPON_FIRING;

			// start the animation
/*			ent->client->anim_priority = ANIM_ATTACK;
			if (ent->stanceflags == STANCE_DUCK)
			{
				ent->s.frame = FRAME_crattak1-1;
				ent->client->anim_end = FRAME_crattak9;
			}
			else if (ent->stanceflags == STANCE_CRAWL)
			{
				ent->s.frame = FRAME_crawlattck01-1;
				ent->client->anim_end = FRAME_crawlattck09;
			}
			else
			{
				// pbowens: attack frames change
				//ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
				//ent->s.frame = FRAME_attack1 - 1 + (ent->client->ps.gunframe % 3);
			 	//ent->s.frame = FRAME_attack1-1;
				//ent->client->anim_end = FRAME_attack8;
			}
*/			
		
		}
		else
		{
no_fire:
			ent->client->machinegun_shots = 0;

			if (ent->client->pers.weapon->position == LOC_SNIPER &&
				ent->client->aim && 
				ent->client->sniper_loaded[ent->client->resp.team_on->index] &&
				ent->client->weaponstate_last != WEAPON_END_MAG)
			{
				GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;

				ent->client->ps.gunframe = guninfo->AFO[3];
			}
			else
			{		

				if (ent->client->ps.gunframe >= FRAME_IDLE_LAST ||
					ent->client->ps.gunframe < FRAME_IDLE_FIRST ||
					ent->client->weaponstate_last == WEAPON_END_MAG )
				{
					ent->client->ps.gunframe = FRAME_IDLE_FIRST;
					return;
				}
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
					WeighPlayer(ent);//faf:  quick fix for fast swimming exploit
			
				// pbowens: weapons may not fire in water (in case they jump in while firing)
				if (ent->waterlevel > 2 && 
					ent->client->pers.weapon->position != LOC_KNIFE &&
					ent->client->pers.weapon->position != LOC_SPECIAL)
				{
					ent->client->weaponstate = WEAPON_READY;
					ent->client->ps.gunframe = FRAME_IDLE_FIRST;
					return;
				}
				else
					fire (ent);

				break;
			}

		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;


		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			if (ent->client->pers.weapon->guninfo && !ent->client->p_rnd)
				goto skip_anim;

		// pbowens: attack frames change
		// start the animation

			ent->client->anim_priority = ANIM_ATTACK;
			if (ent->stanceflags == STANCE_DUCK)
			{
				ent->s.frame = FRAME_crattak1 + (ent->client->ps.gunframe % 2);
				ent->client->anim_end = FRAME_crattak3;
			}
			else if (ent->stanceflags == STANCE_CRAWL)
			{
				ent->s.frame = FRAME_crawlattck01 + (ent->client->ps.gunframe % 2);
				ent->client->anim_end = FRAME_crawlattck03;
			}
			else
			{

				if (extra_anims->value != 1)
				{
					if (ent->client->movement && !ent->client->aim) { // dont play attack animation when running
						if (ent->s.frame >= FRAME_attack1 && ent->s.frame <= FRAME_attack8)
							ent->client->anim_end = ent->s.frame;
						goto skip_anim;
					}
				}
				else
				{
					if (ent->client->movement)
					{
						if (ent->s.frame >= FRAME_attack1 && ent->s.frame <= FRAME_attack8)
							ent->client->anim_end = ent->s.frame;
						goto skip_anim;
					}
				}


				//ent->s.frame = FRAME_attack1 - (int) (random()+0.25);
				ent->s.frame = FRAME_attack1 + (ent->client->ps.gunframe % 2);
			 	//ent->s.frame = FRAME_attack1-1;
				ent->client->anim_end = FRAME_attack3;
			}
		}
skip_anim:
			
		if (ent->client->ps.gunframe >= FRAME_IDLE_FIRST) // last firing frame
			//ent->client->ps.gunframe = FRAME_FIRE_FIRST;
			ent->client->weaponstate = WEAPON_READY;

		return;
	}
	
//added for the hawkins truesite system
	if (ent->client->weaponstate==WEAPON_RAISE)
	{
		if (FRAME_RAISE_LAST == 0) {
			ent->client->aim = false;
			ent->client->weaponstate = WEAPON_READY;
			return;
		}


		ent->client->aim=true;



		if (ent->client->pers.weapon->position == LOC_SNIPER &&
			!ent->client->sniper_loaded[ent->client->resp.team_on->index])
		{
			GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;

			ent->client->crosshair = false;
			ent->client->aim=false;


			// if the frames are out of bounds, reset it to the first
			if (ent->client->ps.gunframe < guninfo->AFO[1] ||
				ent->client->ps.gunframe > guninfo->AFO[2] )
				ent->client->ps.gunframe = guninfo->AFO[1];
			else
			{
				// if not finished with bolt animation, continue
				if (ent->client->ps.gunframe < guninfo->AFO[2])
				{
					if (ent->client->ps.gunframe == ent->client->pers.weapon->guninfo->sniper_bolt_frame ) 
	 					gi.sound(ent, CHAN_WEAPON, gi.soundindex(ent->client->pers.weapon->guninfo->sniper_bolt_wav), 1, ATTN_NORM, 0);

					ent->client->ps.gunframe++;
				}
				else // else un-TS and set back to WEAPON_READY
				{
					ent->client->ps.gunframe = guninfo->FO[0];
					ent->client->weaponstate = WEAPON_READY;
					ent->client->sniper_loaded[ent->client->resp.team_on->index] = true;
				}
			}

			return;
		}


		/*
		//faf:  anti-diving measures to replace slope bug version
		if (ent->client->last_jump_time > level.time - 1 &&
			!ent->groundentity &&
			ent->client->ps.gunframe > FRAME_RAISE_FIRST &&
			(ent->client->pers.weapon->position != LOC_KNIFE)  &&
			(ent->client->pers.weapon->position != LOC_HELMET))
		return;
*/

		//faf:  anti-diving measures to replace slope bug version
		if (ent->client->last_jump_time > level.time - 1 &&
			!ent->groundentity &&
			ent->client->ps.gunframe >= FRAME_RAISE_FIRST &&
			(ent->client->pers.weapon->position != LOC_KNIFE)  &&
			(ent->client->pers.weapon->position != LOC_HELMET))
		{
				ent->client->jump_pause_time = level.time + .3;
				return;

		}
		if (ent->client->jump_pause_time)
		{
			if (ent->client->jump_pause_time > level.time)
				return;
			else
				ent->client->jump_pause_time = 0;
		}



		if(ent->client->ps.gunframe < FRAME_RAISE_FIRST)
			ent->client->ps.gunframe = FRAME_RAISE_FIRST;
		else if(ent->client->ps.gunframe >= FRAME_RAISE_LAST)
		{
			ent->client->aim=true;
			ent->client->weaponstate = WEAPON_READY;

			if(ent->client->pers.weapon->position==LOC_SNIPER)
				ent->client->ps.fov = SCOPE_FOV;

		}
		else ent->client->ps.gunframe++;

		
	}

	if(ent->client->weaponstate==WEAPON_LOWER)
	{
		//faf:  moving this from cmd_scope to avoid slowdown on mauser
		// Nick - Hack to allow a bolt action rifle reload animation to play the entirety.
		if (ent->client->pers.weapon &&
		!strcmp(ent->client->pers.weapon->classname, "weapon_mauser98k") &&
		(ent->client->ps.gunframe >= 4 && ent->client->ps.gunframe <= 15 ||
		ent->client->ps.gunframe >=86 && ent->client->ps.gunframe <=97))
		{
			ent->client->ps.gunframe++;//faf
		}
		// End Nick
		else
		{

			if (FRAME_RAISE_LAST == 0) {
				ent->client->aim = false;	
				ent->client->weaponstate = WEAPON_READY;
				return;
			}

			ent->client->ps.fov = STANDARD_FOV; //nt->client->ps.old_fov;
			if(ent->client->ps.gunframe==FRAME_RAISE_FIRST)
			{
				ent->client->aim=false;
				WeighPlayer(ent);
				ent->client->weaponstate=WEAPON_READY;
			}
			else if(ent->client->ps.gunframe>FRAME_RAISE_LAST
					|| ent->client->ps.gunframe<FRAME_RAISE_FIRST)
				ent->client->ps.gunframe=FRAME_RAISE_LAST;
			else  ent->client->ps.gunframe--;
		}
		
	}
}
  
  

void ifchangewep(edict_t *ent)
{
//	if(auto_weapon_change->value) NoAmmoWeaponChange (ent);
	return;
}
