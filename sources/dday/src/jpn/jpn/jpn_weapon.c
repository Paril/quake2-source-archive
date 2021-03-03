/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/jpn/jpn_weapon.c,v $
 *   $Revision: 1.13 $
 *   $Date: 2002/07/23 07:26:15 $
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

#include "jpn_main.h"

// p_weapon.c
// D-Day: Normandy Player Weapon Code


/*
void Weapon_Generic (edict_t *ent, 
 int FRAME_ACTIVATE_LAST,	int FRAME_LFIRE_LAST,	int FRAME_LIDLE_LAST, 
 int FRAME_RELOAD_LAST,		int FRAME_LASTRD_LAST,	int FRAME_DEACTIVATE_LAST,
 int FRAME_RAISE_LAST,		int FRAME_AFIRE_LAST,	int FRAME_AIDLE_LAST,
 int *pause_frames,			int *fire_frames,		void (*fire)(edict_t *ent))
*/


/////////////////////////////////////////////////
// Nambu Pistol
/////////////////////////////////////////////////

void Weapon_Nambu (edict_t *ent)
{
	static int	pause_frames[]	= {0};//{13, 32,42};
	static int	fire_frames[1];	//= {4,59,0};

	fire_frames[0]=(ent->client->aim)?81:4;

	ent->client->p_fract= &ent->client->mags[jpn_index].pistol_fract;
	ent->client->p_rnd= &ent->client->mags[jpn_index].pistol_rnd;

	ent->client->crosshair = false;

if ((ent->client->weaponstate == WEAPON_FIRING || ent->client->weaponstate == WEAPON_READY)
			&& !ent->client->heldfire && (ent->client->buttons & BUTTON_ATTACK)
			&& ent->client->ps.gunframe!=((ent->client->aim)?80:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?81:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?82:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?83:6)
			//gotta do it this way for both firing modes
)
		{
			if (ent->client->ps.gunframe<4)
//				firetype = abs(5-ent->client->ps.gunframe);  unknown function
			ent->client->ps.gunframe = 4;
			ent->client->weaponstate = WEAPON_READY;
			ent->client->latched_buttons |= BUTTON_ATTACK;
			ent->client->heldfire = true;
		}
		else
		{
			ent->client->buttons &= ~BUTTON_ATTACK;
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
		}
//else
//		ent->client->heldfire = false;  // have to comment out or else semi-auto doesn't work

	Weapon_Generic (ent, 
		3,  6,  47, 
		70, 73, 76, 
		80, 83, 92, 
		
		pause_frames, fire_frames, Weapon_Pistol_Fire);
}


/////////////////////////////////////////////////
// arisaka rifle
/////////////////////////////////////////////////

void Weapon_Arisaka (edict_t *ent)
{
	static int	pause_frames[]	= {0};//{4,25, 50, 0};
	static int	fire_frames[1];// = {4};

	fire_frames[0]=(ent->client->aim)?61:4;
	
	ent->client->p_fract = &ent->client->mags[jpn_index].rifle_fract;
	ent->client->p_rnd= &ent->client->mags[jpn_index].rifle_rnd;

	ent->client->crosshair = false;

	//faf
	if (ent->client->ps.gunframe == 9 ||
		ent->client->ps.gunframe == 63)
		ptrgi->sound(ent, CHAN_VOICE, ptrgi->soundindex("jpn/arisaka/bolt.wav"), 1, ATTN_NORM, 0);


if ((ent->client->weaponstate == WEAPON_FIRING || 
	 ent->client->weaponstate == WEAPON_READY)
		&& !ent->client->heldfire && (ent->client->buttons & BUTTON_ATTACK)
			&& ent->client->ps.gunframe!=((ent->client->aim)?60:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?61:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?62:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?63:6)
			&& ent->client->ps.gunframe!=((ent->client->aim)?64:7)
			&& ent->client->ps.gunframe!=((ent->client->aim)?65:8)
			&& ent->client->ps.gunframe!=((ent->client->aim)?66:9)
			&& ent->client->ps.gunframe!=((ent->client->aim)?67:10)
			&& ent->client->ps.gunframe!=((ent->client->aim)?68:11)
			&& ent->client->ps.gunframe!=((ent->client->aim)?69:12)			
			&& ent->client->ps.gunframe!=((ent->client->aim)?70:13)
	//		&& ent->client->ps.gunframe!=((ent->client->aim)?71:14)
	//		&& ent->client->ps.gunframe!=((ent->client->aim)?72:15)
	//		&& ent->client->ps.gunframe!=((ent->client->aim)?73:16)
	//		&& ent->client->ps.gunframe!=((ent->client->aim)?0:17)
			
			//gotta do it this way for both firing modes
)
		{
//			if (ent->client->ps.gunframe<4)
//				firetype = abs(5-ent->client->ps.gunframe);  unknown function
		ent->client->ps.gunframe = (ent->client->aim)?98:16;
			ent->client->weaponstate = WEAPON_READY;
			ent->client->latched_buttons |= BUTTON_ATTACK;
			ent->client->heldfire = true;
		}
		else
		{
			ent->client->buttons &= ~BUTTON_ATTACK;
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
		}
//else
//		ent->client->heldfire = false;  // have to comment out or else semi-auto doesn't work

	Weapon_Generic (ent, 
		3,  17, 29,//56, 
		53, 53, 56,
		60, 72, 78, 
		
		pause_frames, fire_frames, Weapon_Rifle_Fire);
}


/////////////////////////////////////////////////
// Type 100 SMG
/////////////////////////////////////////////////

void Weapon_Type_100 (edict_t *ent)
{

	static int	pause_frames[]	= {0};
	static int	fire_frames[2];//try to put stutter back in

	fire_frames[0]=(ent->client->aim)?79:4;
	fire_frames[1]=(ent->client->aim)?80:5;


	ent->client->p_fract= &ent->client->mags[jpn_index].submg_fract;
	ent->client->p_rnd= &ent->client->mags[jpn_index].submg_rnd;

	ent->client->crosshair = false;
 
	Weapon_Generic (ent, 
		 3,  5, 38, 
		70, 70, 74, 
		78, 80, 88, 
		
		pause_frames, fire_frames, Weapon_Submachinegun_Fire);

}

/////////////////////////////////////////////////
// Type 99 LMG
/////////////////////////////////////////////////

void Weapon_Type_99 (edict_t *ent)
{
	static int	pause_frames[]	= {0};//{23, 45, 0};
	static int	fire_frames[2];


	fire_frames[0]=(ent->client->aim)?62:4;
	fire_frames[1]=(ent->client->aim)?63:5;

	ent->client->p_fract= &ent->client->mags[jpn_index].lmg_fract;
	ent->client->p_rnd= &ent->client->mags[jpn_index].lmg_rnd;

	ent->client->crosshair = false;

	Weapon_Generic (ent, 
		 3,  5, 25, 
		53, 53, 57, 
		61, 63, 70, 
		
		pause_frames, fire_frames, Weapon_LMG_Fire);
}


/*
void Weapon_Carbine_Fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			kick=200;
	int			i;
	
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int	mod=guninfo->MeansOfDeath;
	int	damage=guninfo->damage_direct;


//faf	if (ent->client->mags[mag_index].rifle_rnd != 1) 
	ent->client->ps.gunframe++;


	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if (ent->client->aim)
		{
			ent->client->ps.gunframe = guninfo->LastAFire;
		}
		else
			ent->client->ps.gunframe = guninfo->LastFire;

		ent->client->machinegun_shots = 0;
//		ent->client->ps.gunframe++;
		ent->client->buttons |= BUTTON_ATTACK;
		//ent->client->latched_buttons &= ~BUTTON_ATTACK;
		ent->client->weaponstate = WEAPON_READY;
		return;
	}

	if (ent->client->next_fire_frame > level.framenum)
		ent->client->ps.gunframe = ((ent->client->aim)? guninfo->LastAFire : guninfo->LastFire) + 1;


	if ( *ent->client->p_rnd == 0 )
	{
		ent->client->ps.gunframe = ((ent->client->aim)? guninfo->LastAFire : guninfo->LastFire) + 1;

		 if (level.time >= ent->pain_debounce_time)
		 {
			 gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
			 ent->pain_debounce_time = level.time + 1;
		 }

//		if (auto_reload->value)
//			Cmd_Reload_f(ent);
		return;
	}

//	ent->client->ps.gunframe++;
	if (ent->client->next_fire_frame > level.framenum)
		return;
	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;



	if (ent->client->mags[mag_index].rifle_rnd == 1) 
	{ // last round fire sounds
		//Hard coded for reload only.

		//faf
		if (strcmp(ent->client->pers.weapon->classname, "weapon_m1"))
		{
			ent->client->ps.gunframe=guninfo->LastReload+1;
			ent->client->weaponstate = WEAPON_END_MAG;
		}
		
		Play_WepSound(ent,guninfo->LastRoundSound);

	}

	/*
	if (ent->client->mags[mag_index].rifle_rnd == 1) 
	{ // last round fire sounds
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
		Play_WepSound(ent,guninfo->LastRoundSound);

	}
	
	else
		ent->client->ps.gunframe++;
		*/
/*

	for (i=1 ; i<3 ; i++)
	{
		// rezmoth - changed for new firing system
		//ent->client->kick_origin[i] = crandom() * 0.35;
		//ent->client->kick_angles[i] = crandom() * 0.7;
	}
	// rezmoth - changed for new firing system
	//ent->client->kick_origin[0] = crandom() * 0.35;
	//ent->client->kick_angles[0] = -5.25;     
	ent->client->machinegun_shots++;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	// rezmoth - changed for new firing system
	//VectorSet(offset, 0, (ent->client->aim)?0:7,  ent->viewheight-8);
	if (ent->client->pers.weapon->position == LOC_RIFLE)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//2
	else
		gi.dprintf("*** Firing System Error\n");

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_gun(ent, start, forward, damage, kick, 0, 0, mod, false);
	// rezmoth - cosmetic recoil
//	if (ent->client->aim)
//		ent->client->kick_angles[0] -= 2.5;
//	else
//		ent->client->kick_angles[0] -= 5;

	ent->client->last_fire_time = level.time;//faf


	ent->client->mags[mag_index].rifle_rnd--;
	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
//	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;

}
/*
void Weapon_M1Carbinex (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[1];
	
	fire_frames[0] = (ent->client->aim)?76:4;

	//ent->client->p_fract = (&ent->client->mags[jpn_index].hmg_rnd);
	ent->client->p_rnd= &ent->client->mags[jpn_index].hmg_rnd;

	ent->client->crosshair = false;

	if ((ent->client->weaponstate == WEAPON_FIRING || ent->client->weaponstate == WEAPON_READY)
		&& !ent->client->heldfire && (ent->client->buttons & BUTTON_ATTACK)
		&& ent->client->ps.gunframe!=((ent->client->aim)?75:3)
		&& ent->client->ps.gunframe!=((ent->client->aim)?76:4)
		&& ent->client->ps.gunframe!=((ent->client->aim)?77:5)
/*		&& ent->client->ps.gunframe!=((ent->client->aim)?78:6)*/ //)remove to fire faster
/*	{
		if (ent->client->ps.gunframe<4)
			ent->client->ps.gunframe = 4;

		ent->client->weaponstate = WEAPON_READY;
		ent->client->heldfire = true;
	}
	else
	{
		ent->client->buttons &= ~BUTTON_ATTACK;
		ent->client->latched_buttons &= ~BUTTON_ATTACK;
	}

	Weapon_Generic (ent, 
		 3,  6, 47, 
		64, 67, 72, 
		75, 78, 89, 	
		pause_frames, fire_frames, Weapon_M1Carbine_Fire);
}*/





/////////////////////////////////////////////////
//M1 Garand
/////////////////////////////////////////////////

/*void Weapon_M1Carbine (edict_t *ent)
{
	static int	pause_frames[]	= {0};//{47, 80};
	static int	fire_frames[1];
	
	fire_frames[0] = (ent->client->aim)?76:4;

	// Wheaty: Uncomment next line to allow topping off
	ent->client->p_fract = &ent->client->mags[jpn_index].hmg_rnd;
	ent->client->p_rnd= &ent->client->mags[jpn_index].hmg_rnd;

	ent->client->crosshair = false;

if ((ent->client->weaponstate == WEAPON_FIRING || ent->client->weaponstate == WEAPON_READY)
			&& !ent->client->heldfire && (ent->client->buttons & BUTTON_ATTACK)
			&& ent->client->ps.gunframe!=((ent->client->aim)?75:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?76:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?77:5)
		&& ent->client->ps.gunframe!=((ent->client->aim)?78:6))//remove to fire faster
			//gotta do it this way for both firing modes

		{
			if (ent->client->ps.gunframe<4)
//				firetype = abs(5-ent->client->ps.gunframe);  unknown function
			ent->client->ps.gunframe = 4;
			ent->client->weaponstate = WEAPON_READY;
		//	ent->client->latched_buttons |= BUTTON_ATTACK;
			ent->client->heldfire = true;
		}
		else
		{
			ent->client->buttons &= ~BUTTON_ATTACK;
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
		}
//else
//		ent->client->heldfire = false;  // have to comment out or else semi-auto doesn't work

	Weapon_Generic (ent, 
		 3,  6, 47, 
		64, 67, 72, 
		75, 78, 89, 
		
		pause_frames, fire_frames, Weapon_Carbine_Fire);

}


*/

/////////////////////////////////////////////////
// arisaka Sniper Rifle
/////////////////////////////////////////////////

void Weapon_Arisakas (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[4];

	fire_frames[0]=(ent->client->aim)?52:4;//fire here
	fire_frames[1]=(ent->client->aim)?59:0;//faf 60:0;//sniper bolt
	fire_frames[2]=(ent->client->aim)?75:0;//sniper start zoom
	fire_frames[3]=(ent->client->aim)?80:0;//sniper end zoom

	ent->client->p_fract= &ent->client->mags[jpn_index].sniper_fract;
	ent->client->p_rnd= &ent->client->mags[jpn_index].sniper_rnd;

	if (ent->client->aim) 
	{
		if ( (ent->client->ps.gunframe >= fire_frames[0] && ent->client->ps.gunframe <= fire_frames[1]) ||
			  ent->client->ps.gunframe >= fire_frames[3])
			ent->client->crosshair = true;
		else
			ent->client->crosshair = false;
	} 
	else
		ent->client->crosshair = false;

	if ((ent->client->weaponstate == WEAPON_FIRING || ent->client->weaponstate == WEAPON_READY)
			&& !ent->client->heldfire && (ent->client->buttons & BUTTON_ATTACK)
			&& ent->client->ps.gunframe!=((ent->client->aim)?51:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?52:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?53:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?54:6)
/*			&& ent->client->ps.gunframe!=((ent->client->aim)?55:7)
			&& ent->client->ps.gunframe!=((ent->client->aim)?56:8)
			&& ent->client->ps.gunframe!=((ent->client->aim)?57:9)
			&& ent->client->ps.gunframe!=((ent->client->aim)?58:10)
			&& ent->client->ps.gunframe!=((ent->client->aim)?59:11)
			&& ent->client->ps.gunframe!=((ent->client->aim)?60:12)
			&& ent->client->ps.gunframe!=((ent->client->aim)?61:13)
			&& ent->client->ps.gunframe!=((ent->client->aim)?62:14)
			&& ent->client->ps.gunframe!=((ent->client->aim)?63:15)
			&& ent->client->ps.gunframe!=((ent->client->aim)?64:16)
			&& ent->client->ps.gunframe!=((ent->client->aim)?65:17)
			&& ent->client->ps.gunframe!=((ent->client->aim)?66:18)
			&& ent->client->ps.gunframe!=((ent->client->aim)?67:19)
			&& ent->client->ps.gunframe!=((ent->client->aim)?68:20)
			&& ent->client->ps.gunframe!=((ent->client->aim)?69:21)
			&& ent->client->ps.gunframe!=((ent->client->aim)?70:22)
			&& ent->client->ps.gunframe!=((ent->client->aim)?71:23)
			&& ent->client->ps.gunframe!=((ent->client->aim)?72:24)
			&& ent->client->ps.gunframe!=((ent->client->aim)?73:25)
			&& ent->client->ps.gunframe!=((ent->client->aim)?74:26)
			&& ent->client->ps.gunframe!=((ent->client->aim)?75:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?76:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?77:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?78:6)
			&& ent->client->ps.gunframe!=((ent->client->aim)?79:7)
			&& ent->client->ps.gunframe!=((ent->client->aim)?80:8)*/
//			&& ent->client->ps.gunframe!=((ent->client->aim)?81:9)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?82:10)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?83:11)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?84:12)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?85:13)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?86:14)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?87:15)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?88:16)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?89:17)
//			&& ent->client->ps.gunframe!=((ent->client->aim)?90:18)

			//gotta do it this way for both firing modes
)
		{
			if (ent->client->ps.gunframe<4)
//				firetype = abs(5-ent->client->ps.gunframe);  unknown function
			ent->client->ps.gunframe = 4;
			ent->client->weaponstate = WEAPON_READY;
			ent->client->latched_buttons |= BUTTON_ATTACK;
			ent->client->heldfire = true;
		}
		else
		{
			ent->client->buttons &= ~BUTTON_ATTACK;
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
		}
//else
//		ent->client->heldfire = false;  // have to comment out or else semi-auto doesn't work

	Weapon_Generic (ent, 
		 3, 8, 19,//25, 27,//faf 26, 26, 
		43, 43, 48, 
		51, 80, 80, 
		
		pause_frames, fire_frames, Weapon_Sniper_Fire);
}




qboolean fire_katana ( edict_t *self, vec3_t start, vec3_t dir, int damage, int kick)
{    
    trace_t tr; //detect whats in front of you up to range "vec3_t end"

    vec3_t end;

	vec3_t	vec;
	float	dot;
	vec3_t	forward;


    // Figure out what we hit, if anything:

    VectorMA (start, 55, dir, end);  //calculates the range vector //50 is sword range
    tr = ptrgi->trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
                        // figures out what in front of the player up till "end"
    
   // Figure out what to do about what we hit, if anything

    if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))    
    {
        if (tr.fraction < 1.0)        
        {            
            if (tr.ent->takedamage)            
            {

				if (tr.ent->client && 
					tr.ent->client->pers.weapon &&
					!strcmp(tr.ent->client->pers.weapon->classname, "weapon_katana"))
				{
					AngleVectors (tr.ent->s.angles, forward, NULL, NULL);
					VectorSubtract (self->s.origin, tr.ent->s.origin, vec);
					VectorNormalize (vec);
					dot = DotProduct (vec, forward);
					//ptrgi->bprintf (PRINT_HIGH, "dot = %f \n",dot);
					if (dot > 0.6  &&
						tr.ent->client->weaponstate != WEAPON_FIRING)
					{
						if (random() < .5)	
							ptrgi->sound (self, CHAN_AUTO, ptrgi->soundindex("jpn/katana/hit1.wav") , 1, ATTN_NORM, 0);
						else
							ptrgi->sound (self, CHAN_AUTO, ptrgi->soundindex("jpn/katana/hit2.wav") , 1, ATTN_NORM, 0);
					}

					else
					{
						T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, 100, 50, 0,MOD_KNIFE);//faf
					  ptrgi->sound (self, CHAN_AUTO, ptrgi->soundindex("brain/melee3.wav") , 1, ATTN_NORM, 0); 
					}

			
				}
				else
				{
					T_Damage (tr.ent, self, self, dir, tr.endpos, tr.plane.normal, 100, 50, 0,MOD_KNIFE);//faf
	                ptrgi->sound (self, CHAN_AUTO, ptrgi->soundindex("brain/melee3.wav") , 1, ATTN_NORM, 0); 
				}

            }        
            else        
            {                
                ptrgi->WriteByte (svc_temp_entity);    
                ptrgi->WriteByte (TE_SPARKS);
                ptrgi->WritePosition (tr.endpos);    
                ptrgi->WriteDir (tr.plane.normal);
                ptrgi->multicast (tr.endpos, MULTICAST_PVS);

				if (random() < .5)	
					ptrgi->sound (self, CHAN_AUTO, ptrgi->soundindex("jpn/katana/hit1.wav") , 1, ATTN_NORM, 0);
				else
					ptrgi->sound (self, CHAN_AUTO, ptrgi->soundindex("jpn/katana/hit2.wav") , 1, ATTN_NORM, 0);

            }    
			return true;
        }
    }
    return false;
} 
 


void Weapon_Katana_Fire (edict_t *ent)
{
	vec3_t  forward, right;
    vec3_t  start;
    vec3_t  offset;
	vec3_t g_offset;

	vec3_t direction;

	ent->client->ps.gunframe++;//faf


	VectorCopy (vec3_origin,g_offset);

    AngleVectors (ent->client->v_angle, forward, right, NULL);
    VectorSet(offset, 24, 8, ent->viewheight-8);
    VectorAdd (offset, g_offset, offset);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

    VectorScale (forward, -2, ent->client->kick_origin);
    ent->client->kick_angles[0] = -1;

	VectorAdd (forward, right, direction);
	VectorAdd (direction, forward, direction);
	VectorNormalize (direction);
 
	if (!(fire_katana (ent, start, direction, 50, 0)))
	{
		VectorAdd (direction, forward, direction);
		VectorSubtract (forward, right, direction);
		VectorNormalize (direction);
		if (!(fire_katana (ent, start, direction, 50, 0)))
		{
			if (!(fire_katana (ent, start, forward, 50, 0)))
			{
				VectorAdd (direction, forward, direction);
				VectorSubtract (forward, right, direction);
				VectorNormalize (direction);
				if (!(fire_katana (ent, start, direction, 50, 0)))
				{}
			}

		}

	}



	
	ent->client->ps.gunframe++;

	PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
}






void Weapon_Katana (edict_t *ent)
{
	static int      pause_frames[]  = {0};//{19, 32, 0};
    int				fire_frames[] = {9,10};

	if (ent->client->aim)
		ent->client->aim = false;

	ent->client->crosshair = false;

	//ent->client->aim=false;
	//fire_frames[0]=(ent->client->aim)?54:4;
	ent->client->p_rnd=NULL;
	
	//faf
		fire_frames[0]=9;

		fire_frames[1] = 0;

	if(ent->client->ps.gunframe == 1)
		ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex("jpn/katana/draw.wav"), 1, ATTN_NORM, 0);//faf




	if (ent->client->ps.gunframe == 9)
	{
		ent->client->anim_priority = ANIM_REVERSE;
		if (ent->stanceflags == STANCE_STAND)
			{
				ent->s.frame = 66;//FRAME_pain304+1;
				ent->client->anim_end = 62;//FRAME_pain301;            
			}
			else if (ent->stanceflags == STANCE_DUCK)
			{
				ent->s.frame = 173;// FRAME_crpain4+1;
				ent->client->anim_end = 169;//FRAME_crpain1;
			}
			else if (ent->stanceflags == STANCE_CRAWL)
			{
				ent->s.frame = 234;// FRAME_crawlpain04+1;
				ent->client->anim_end = 230;//FRAME_crawlpain01;
			}
	}



	if(ent->client->ps.gunframe == 9)
	{
		if (random() < .333)	
		{
			ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex("jpn/katana/swing1.wav"), 1, ATTN_NORM, 0);
		}
		else if (random() < .5)	
		{
			ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex("jpn/katana/swing2.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex("jpn/katana/swing3.wav"), 1, ATTN_NORM, 0);
		}
	}







	Weapon_Generic (ent, 
		7, 14, 29, 
		29,29,36,
		36,36,36, 
		pause_frames, fire_frames, Weapon_Katana_Fire);
}




//molotov cocktail


void Molotov_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t *attacker;
	int i;
	vec3_t v;


    if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
        return;
	}

	if (ent->waterlevel)
	{
		ent->touch = NULL;
		ent->s.sound = 0;
		return;
	}



	attacker = ent->owner;

	/*
	//explode
	ptrgi->WriteByte (svc_temp_entity);
	ptrgi->WriteByte (TE_EXPLOSION1);
	ptrgi->WritePosition (ent->s.origin);
	ptrgi->multicast (ent->s.origin, MULTICAST_PHS);
*/

	//burn
	PBM_Ignite (other, attacker, ent->s.origin);
	
	//ptrgi->bprintf (PRINT_HIGH, "ignite \n");




	for (i = 0; i <3; i ++)
	{
		edict_t *glass;

		glass = G_Spawn();
		glass->s.modelindex = ptrgi->modelindex("models/objects/glass1/tris.md2");
		glass->s.frame      = 0;
		glass->s.skinnum    = 0;
		VectorClear (glass->mins);
		VectorClear (glass->maxs);
		VectorCopy (ent->s.origin, glass->s.origin);
		VectorClear (glass->s.angles);
		VectorClear (glass->velocity);
		glass->movetype     = MOVETYPE_TOSS;
		glass->clipmask     = MASK_SHOT;
		glass->solid        = SOLID_BBOX;
		glass->takedamage   = DAMAGE_NO;
		glass->s.effects    = 0;

		glass->classname    = "glass";
		glass->nextthink    = ptrlevel->time + 3;
		glass->think        = G_FreeEdict;

							
		v[0] = 50 + 50 * crandom();
		v[1] = 50 + 50 * crandom();
		if (random() < .5)
		{
			v[0] = -v[0];
		}
		if (random() < .5)
		{
			v[1] = -v[1];
		}


		v[2] = 50 + 50 * crandom();
		VectorMA (glass->velocity, 2, v, glass->velocity);

	VectorSet (glass->avelocity, (500 - 1000 * random()), (500 - 1000 * random()), (500 - 1000 * random()));

		ptrgi->linkentity (glass);

		
	}








 
	for (i = 0; i <4; i ++)
	{
		edict_t *fire;

		fire = G_Spawn();
		fire->s.modelindex = ptrgi->modelindex("models/fire/tris.md2");
		fire->s.frame      = 0;
		fire->s.skinnum    = 0;
		VectorClear (fire->mins);
		VectorClear (fire->maxs);
		VectorCopy (ent->s.origin, fire->s.origin);
		VectorClear (fire->s.angles);
		VectorClear (fire->velocity);
		fire->movetype     = MOVETYPE_TOSS;
		fire->clipmask     = MASK_SHOT;
		fire->solid        = SOLID_BBOX;
		fire->takedamage   = DAMAGE_NO;
		fire->s.effects    = 0;
		fire->s.renderfx   = RF_FULLBRIGHT;

		fire->owner        = fire;
		fire->master       = attacker;
		fire->classname    = "fire";
		fire->touch        = PBM_FireDropTouch;
		fire->burnout      = ptrlevel->time + 2 + random() * 3;
		fire->timestamp    = ptrlevel->time;
		fire->nextthink    = ptrlevel->time + FRAMETIME;
		fire->think        = PBM_CheckFire;
	//				VectorCopy (30, fire->pos1);
	//				VectorCopy (30, fire->pos2);
		fire->dmg_radius   = 30;
		fire->dmg          = 1;//blast_chance;

//		fire->s.origin[0] = ent->s.origin[0] + crandom() * 100;				
//		fire->s.origin[1] = ent->s.origin[1] + crandom() * 100;
//		fire->s.origin[2] = ent->s.origin[2] + crandom() * 100;

							
		v[0] = 75 + 50 * crandom();
		v[1] = 75 + 50 * crandom();
		if (random() < .5)
		{
			v[0] = -v[0];
		}
		if (random() < .5)
		{
			v[1] = -v[1];
		}


		v[2] = 75 + 50 * crandom();
		VectorMA (fire->velocity, 2, v, fire->velocity);


		ptrgi->linkentity (fire);

		
	}
	ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex("jpn/molotov/break.wav"), 1, ATTN_NORM, 0);

	G_FreeEdict(ent);

}




void Molotov_Throw (edict_t *self, vec3_t start, vec3_t dir)
{
	edict_t	*molotov;
//	trace_t	tr;

	VectorNormalize (dir);

	molotov = G_Spawn();
	VectorCopy (start, molotov->s.origin);
	VectorCopy (start, molotov->s.old_origin);
	vectoangles (dir, molotov->s.angles);
	VectorScale (dir, 650, molotov->velocity);
//faf	molotov->movetype = MOVETYPE_STEP;
	molotov->clipmask = MASK_SHOT;
	molotov->solid = SOLID_BBOX;
//faf 	molotov->s.effects |= effect;


	molotov->movetype = MOVETYPE_TOSS;//STEP;
	VectorSet (molotov->mins, -4, -4, 0);
	VectorSet (molotov->maxs, 4, 4, 4);

	VectorSet (molotov->avelocity, (500 - 1000 * random()), (500 - 1000 * random()), (500 - 1000 * random()));

//	molotov->s.modelindex = ptrgi->modelindex("models/weapons/jpn/g_molotov/tris.md2");
	molotov->s.modelindex = ptrgi->modelindex("models/objects/molotov/tris.md2");

	molotov->owner = self;
//faf: for separate helmet/molotov death messages  	molotov->touch = Blade_touch;
	molotov->touch = Molotov_Touch;//faf

	molotov->nextthink = ptrlevel->time + 5;//faf 2;
	molotov->think = G_FreeEdict;
	molotov->dmg = 0;
	molotov->s.frame = 0;
	
	molotov->classname = "molotov";

//	molotov->s.effects = EF_GRENADE;

	molotov->s.sound = ptrgi->soundindex("inland/fire.wav");

	molotov->spawnflags = 1;
	ptrgi->linkentity (molotov);

  }


void Weapon_Molotov_Fire (edict_t *ent)
{
//	int molotovs;		//var to keep track of how many molotovs are left.
//	int molotov_index; //index to molotov
	vec3_t  forward, right;
    vec3_t  start;
    vec3_t  offset;
	vec3_t g_offset;

	int mag_index=ent->client->pers.weapon->mag_index;


	ent->client->ps.gunframe++;//faf



	
	VectorCopy (vec3_origin,g_offset);

    AngleVectors (ent->client->v_angle, forward, right, NULL);
    VectorSet(offset, 24, 8, ent->viewheight-8);
    VectorAdd (offset, g_offset, offset);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

    VectorScale (forward, -2, ent->client->kick_origin);
    ent->client->kick_angles[0] = -1;
 
//	if(ent->client->pers.inventory[molotov_index])
//	{
		start[2]+= 5;//faf
		Molotov_Throw(ent, start,forward);
//		ent->client->pers.inventory[molotov_index]--;
	//	ent->client->mags[jpn_index].antitank_rnd--;
//	ent->client->mags[jpn_index].antitank_rnd--;
		ent->client->pers.inventory[ent->client->ammo_index]--;

	
	if (ent->client->pers.inventory[ent->client->ammo_index] ==0)
	{
		ent->client->weaponstate=WEAPON_LOWER;
		Use_Weapon (ent, FindItem("fists"));
		return;
	} 
	
	ent->client->ps.gunframe++;
	ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex("jpn/molotov/throw.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
}






void Weapon_Molotov (edict_t *ent)
{
	static int      pause_frames[]  = {0};//{19, 32, 0};
    int				fire_frames[] = {7, 9};

	ent->client->p_rnd= NULL;//&ent->client->mags[jpn_index].antitank_rnd;


	ent->client->crosshair = false;

	if (ent->client->aim)
		ent->client->aim = false;

	
	//faf
	fire_frames[0]=19;//faf54:7;

	fire_frames[1] = 0;

	if (ent->client->ps.gunframe == 5)
		ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex("jpn/molotov/light.wav"), 1, ATTN_NORM, 0);

	// Darwin's xtra molotov sounds
//	if(!armedfists && ent->client->ps.gunframe == 1)
//		Play_WepSound(ent, "molotov/pullout.wav");

	Weapon_Generic (ent, 
		3,  20, 54, 
		54, 54, 57,
		57, 57, 57, 
		pause_frames, fire_frames, Weapon_Molotov_Fire);
}

/////////////////////////////////////////////////
// MG42
/////////////////////////////////////////////////

void Weapon_MG42 (edict_t *ent)
{
	static int	pause_frames[]	= {0};//{38, 61, 0};
	static int	fire_frames[2];

//	fire_frames[0]=(ent->client->aim)?99:20;
	fire_frames[0]=(ent->client->aim)?86:21;
	fire_frames[1]=(ent->client->aim)?87:22;

	ent->client->p_rnd= &ent->client->mags[jpn_index].hmg_rnd;

	ent->client->crosshair = false;

	Weapon_Generic (ent, 
		20, 22, 62, 
		79, 79, 82, 
		85, 87, 99, 
		
		pause_frames, fire_frames, Weapon_HMG_Fire);
}



void fire_shotgun(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod, qboolean calcv)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;

	vec3_t	diststart, dist;
	vec3_t	distend = {0, 0, -8192};
	trace_t	disttr;

	VectorCopy(self->s.origin, diststart);
	VectorAdd(diststart, distend, distend);

	disttr = ptrgi->trace (diststart, self->mins, self->maxs, distend, self, MASK_SOLID);
	VectorSubtract(self->s.origin, disttr.endpos, dist);
	tr = ptrgi->trace(self->s.origin, NULL, NULL, start, self, MASK_SHOT);

	if (!(tr.fraction < 1.0))
	{
		vectoangles(aimdir, dir);
		AngleVectors(dir, forward, right, up);

		calcv = false;

		if (self->client->aim)
		{
			r = crandom() * 400;
			u = crandom() * 400;
		}
		else
		{
			r = crandom() * 400;
			u = crandom() * 400;
		}

		VectorMA (start, 8192, aimdir, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		if (ptrgi->pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = ptrgi->trace (start, NULL, NULL, end, self, content_mask);

		if(calcv) calcVspread(self,&tr);

		if (tr.contents & MASK_WATER)
		{
			int		color;
			water = true;

			VectorCopy (tr.endpos, water_start);

			if (!VectorCompare (start, tr.endpos))
			{
				if (tr.contents & CONTENTS_WATER)
				{
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				}
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
				{
					ptrgi->WriteByte (svc_temp_entity);
					ptrgi->WriteByte (TE_SPLASH);
					ptrgi->WriteByte (8);
					ptrgi->WritePosition (tr.endpos);
					ptrgi->WriteDir (tr.plane.normal);
					ptrgi->WriteByte (color);
					ptrgi->multicast (tr.endpos, MULTICAST_PVS);
				}

				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, aimdir, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}

			tr = ptrgi->trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage)
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					ptrgi->WriteByte (svc_temp_entity);
					if (crandom() < 0.5)
						ptrgi->WriteByte (TE_GUNSHOT);
					else
						ptrgi->WriteByte (TE_BULLET_SPARKS);
					ptrgi->WritePosition (tr.endpos);
					ptrgi->WriteDir (tr.plane.normal);
					ptrgi->multicast (tr.endpos, MULTICAST_PVS);

					if (self->client) PlayerNoise(self, tr.endpos, PNOISE_IMPACT);						
				}
			}
		}
	}

	if (water)
	{
		vec3_t	pos;

		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);

		if (ptrgi->pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = ptrgi->trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		ptrgi->WriteByte (svc_temp_entity);
		ptrgi->WriteByte (TE_BUBBLETRAIL);
		ptrgi->WritePosition (water_start);
		ptrgi->WritePosition (tr.endpos);
		ptrgi->multicast (pos, MULTICAST_PVS);
	}
} 

/////////////////////////////////////////////////
// shotgun
/////////////////////////////////////////////////
void Weapon_Shotgun_Fire (edict_t *ent)
{
	int		kick=2, i;
	vec3_t		offset;
	vec3_t		forward, right;
	vec3_t		start;
	vec3_t		angles;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;
	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage=guninfo->damage_direct;
	
	//ent->client->aim = false;

	if (ent->client->next_fire_frame > ptrlevel->framenum) return;
//	if (!(ent->client->buttons & BUTTON_ATTACK)) return;

	ent->client->ps.gunframe++;     
	

	if ((!ent->client->mags[mag_index].antitank_rnd))
	{
		ptrgi->sound(ent, CHAN_VOICE, ptrgi->soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
		ent->pain_debounce_time = ptrlevel->time + 1;

		ent->client->ps.gunframe = (ent->client->aim)?85:16;
		ent->client->weaponstate = WEAPON_READY;
		ent->client->next_fire_frame = ptrlevel->framenum + 10;	

		return;
	}

	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 0, ent->viewheight - 0);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	for (i=0; i<DEFAULT_DEATHMATCH_SHOTGUN_COUNT; i++) fire_shotgun(ent, start, forward, damage, kick, 0, 0, mod, false);
	
	ent->client->kick_angles[0] -= 2;


	ent->client->mags[mag_index].antitank_rnd--;
	ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex("usa/shotgun/fire.wav"), 1, ATTN_NORM, 0);

	//ptrgi->WriteByte (svc_muzzleflash);
	//ptrgi->WriteShort (ent-g_edicts);
	//ptrgi->WriteByte (MZ_MACHINEGUN);
	//ptrgi->multicast (ent->s.origin, MULTICAST_PVS);

//	ent->client->next_fire_frame = ptrlevel->framenum + guninfo->frame_delay;	
}

#define FRAME_LIDLE_FIRST		(FRAME_LFIRE_LAST+1)
#define FRAME_RELOAD_FIRST		(FRAME_LIDLE_LAST+1)
#define FRAME_RAISE_FIRST		(FRAME_DEACTIVATE_LAST+1)
#define FRAME_AIDLE_FIRST		(FRAME_AFIRE_LAST+1)

#define FRAME_pain304         	65
#define FRAME_pain301         	62
#define FRAME_crpain1         	169
#define FRAME_crpain4         	172
#define FRAME_crawlpain01		230
#define FRAME_crawlpain04		233

void Shotgun_Reload (edict_t *ent, 
					 int FRAME_ACTIVATE_LAST,	int FRAME_LFIRE_LAST,	int FRAME_LIDLE_LAST, 
					 int FRAME_RELOAD_LAST,		int FRAME_LASTRD_LAST,	int FRAME_DEACTIVATE_LAST,
					 int FRAME_RAISE_LAST,		int FRAME_AFIRE_LAST,	int FRAME_AIDLE_LAST,
					 int *pause_frames,			int *fire_frames,		void (*fire)(edict_t *ent))
{
	gitem_t *ammo_item;
	int		ammo_index,	*ammo_ammount;
	int		FRAME_IDLE_FIRST = (ent->client->aim)?FRAME_AIDLE_FIRST:FRAME_LIDLE_FIRST;

	if(ent->client->pers.weapon->ammo)
	{
		ammo_item = FindItem(ent->client->pers.weapon->ammo);
		ammo_index = ent->client->ammo_index;
		ammo_ammount=&ent->client->pers.inventory[ammo_index];
	}

	ent->client->ps.fov=STANDARD_FOV;

	if (ent->client->ps.gunframe == FRAME_RELOAD_FIRST)
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
        
	if (ent->client->aim) 
	{
		if(ent->client->ps.gunframe==FRAME_RAISE_FIRST)
			ent->client->aim=false;
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
		if ((ent->client->pers.weapon->guninfo) && (ent->client->ps.gunframe == 0/*RELOAD SOUND FRAME*/))
			ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex(ent->client->pers.weapon->guninfo->ReloadSound1), 1, ATTN_NORM, 0);
	}
    else
	{
		ent->client->ps.gunframe = FRAME_IDLE_FIRST;
        ent->client->weaponstate = WEAPON_READY;

		if (*ammo_ammount)
		{
			ent->client->pers.inventory[ammo_index]--;
			ent->client->mags[ent->client->pers.weapon->mag_index].antitank_rnd++;
		}
	} 
}
void Weapon_Shotgun (edict_t * ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[1];

	fire_frames[0]=(ent->client->aim)?76:8;



	
//	ent->client->p_fract = &ent->client->mags[usm_index].antitank_fract;
	ent->client->p_rnd= &ent->client->mags[jpn_index].antitank_rnd;

	ent->client->crosshair = false;

	if (ent->client->ps.gunframe == 4)
		ptrgi->sound(ent, CHAN_VOICE, ptrgi->soundindex("usa/shotgun/load.wav"),1, ATTN_NORM, 0);
	else if (ent->client->ps.gunframe == 62)
		ptrgi->sound(ent, CHAN_VOICE, ptrgi->soundindex("usa/shotgun/reload.wav"),1, ATTN_NORM, 0);

	if (ent->client->weaponstate == WEAPON_RELOADING)
	{
		Shotgun_Reload(ent, 
			 7,  16, 55, 
			67, 67, 72,
			75, 84, 92, 
			pause_frames, fire_frames, Weapon_Shotgun_Fire);
	}
	else
	{
		Weapon_Generic (ent, 
			 7,  16, 55, 
			67, 67, 72,
			75, 84, 92, 
			pause_frames, fire_frames, Weapon_Shotgun_Fire);
	}
}
