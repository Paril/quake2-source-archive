/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/gbr/gbr_weapon.c,v $
 *   $Revision: 1.12 $
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

#include "gbr_main.h"


// p_weapon.c
// D-Day: Normandy Player Weapon Code


/*
void Weapon_Generic (edict_t *ent, 
 int FRAME_ACTIVATE_LAST,	int FRAME_LFIRE_LAST,	int FRAME_LIDLE_LAST, 
 int FRAME_RELOAD_LAST,		int FRAME_LASTRD_LAST,	int FRAME_DEACTIVATE_LAST,
 int FRAME_RAISE_LAST,		int FRAME_AFIRE_LAST,	int FRAME_AIDLE_LAST,
 int *pause_frames,			int *fire_frames,		void (*fire)(edict_t *ent))
*/

void fire_gun2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod, qboolean calcv);

void Weapon_Bayonet_Fire (edict_t *ent);
void fire_Knife2 ( edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, char *wav, qboolean fists);
void Weapon_Enfield_Fire (edict_t *ent);
void Weapon_Sten_Fire (edict_t *ent);
void Weapon_PIAT_Fire (edict_t *ent);
void fire_rocket2 (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);


void Weapon_Pistol_Fire2 (edict_t *ent);


/////////////////////////////////////////////////
// Webley
/////////////////////////////////////////////////

void Weapon_Webley (edict_t *ent)
{
	static int	pause_frames[]	= {0};//{13, 32,42};
	static int	fire_frames[1];	//= {4,59,0};
	
	fire_frames[0]=(ent->client->aim)?82:4;  //75

	ent->client->p_fract= &ent->client->mags[gbr_index].pistol_fract;
	ent->client->p_rnd= &ent->client->mags[gbr_index].pistol_rnd;

	ent->client->crosshair = false;

if ((ent->client->weaponstate == WEAPON_FIRING || ent->client->weaponstate == WEAPON_READY)
			&& !ent->client->heldfire && (ent->client->buttons & BUTTON_ATTACK)
			&& ent->client->ps.gunframe!=((ent->client->aim)?81:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?82:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?83:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?84:6)
			//gotta do it this way for both firing modes
)
		{
//			if (ent->client->ps.gunframe<4)
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

		// Nick 21/12/2002 - Stop the 'nil ammo' idle -> reload animation.
		// This bug can be seen on any other Teams' pistol.  Empty a clip, and
		// then go T/S, fire a 'blank'.  Let the model go into idle, sit back and
		// watch as the model cycles thru' idle -> reload frames -> back to T/S!.

		//if ((ent->client->ps.gunframe == 47) & (!ent->client->mags[gbr_index].pistol_rnd)) {
		//	ent->client->ps.gunframe = 7;
		//}

		// See the pistol_fire2 routine.  Can leave that as is and use above code
		// or as it is now to get the gun to animate firing with no ammo.
		// End Nick

//else
//		ent->client->heldfire = false;  // have to comment out or else semi-auto doesn't work


	Weapon_Generic (ent, 
		 3,  6, 47, 
		69, 72, 76,
		81, 84, 95, 
		
		pause_frames, fire_frames, Weapon_Pistol_Fire2);
}


/////////////////////////////////////////////////
//Lee Enfield
/////////////////////////////////////////////////
//(based on mauser code)
void Weapon_Enfield (edict_t *ent)
{
	static int	pause_frames[]	= {0};//{4,25, 50, 0};
	static int	fire_frames[1];// = {4};

	fire_frames[0]=(ent->client->aim)?89:4;
	
	ent->client->p_fract = &ent->client->mags[gbr_index].rifle_fract;
	ent->client->p_rnd= &ent->client->mags[gbr_index].rifle_rnd;

	ent->client->crosshair = false;



//faf: bayonet code
	if (ent->client->ps.gunframe == 108)
	{
		ent->client->ps.gunframe = 18;
		return;
	}

	if (ent->client->ps.gunframe > 104)
	{
		ent->client->ps.gunframe++;
		return;
	}
//end bayonet



if ((ent->client->weaponstate == WEAPON_FIRING || 
	 ent->client->weaponstate == WEAPON_READY)
		&& !ent->client->heldfire && (ent->client->buttons & BUTTON_ATTACK)
			&& ent->client->ps.gunframe!=((ent->client->aim)?88:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?89:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?90:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?91:6)
			&& ent->client->ps.gunframe!=((ent->client->aim)?92:7)
			&& ent->client->ps.gunframe!=((ent->client->aim)?93:8)
			&& ent->client->ps.gunframe!=((ent->client->aim)?94:9)
			&& ent->client->ps.gunframe!=((ent->client->aim)?95:10)
			&& ent->client->ps.gunframe!=((ent->client->aim)?96:11)
			&& ent->client->ps.gunframe!=((ent->client->aim)?97:12)			
			&& ent->client->ps.gunframe!=((ent->client->aim)?98:13)
			&& ent->client->ps.gunframe!=((ent->client->aim)?99:14)
			&& ent->client->ps.gunframe!=((ent->client->aim)?100:15)
			&& ent->client->ps.gunframe!=((ent->client->aim)?101:16)
	//		&& ent->client->ps.gunframe!=((ent->client->aim)?0:17)
			
			//gotta do it this way for both firing modes
)
		{
//			if (ent->client->ps.gunframe<4)
//				firetype = abs(5-ent->client->ps.gunframe);  unknown function
		ent->client->ps.gunframe = (ent->client->aim)?101:16;
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

/*	if (( *ent->client->p_rnd == 0 ) && 
		(ptrlevel->time >= ent->pain_debounce_time) &&
		(ent->client->weaponstate == WEAPON_READY))
	{
		Weapon_Generic (ent,
			100,  102, 105, 
			106,  108, 110,
			112,  114, 116,
			
			pause_frames, fire_frames, Weapon_Bayonet_Fire);

	}
	else
	{ */
	Weapon_Generic (ent, 
		3,  16, 56, 
		81, 81, 85, 
		88, 101, 103, 
		
		pause_frames, fire_frames, Weapon_Enfield_Fire);
	//}
}


/////////////////////////////////////////////////
// Sten
/////////////////////////////////////////////////

void Weapon_Sten (edict_t *ent)
{

	static int	pause_frames[]	= {0};
	static int	fire_frames[2];//try to put stutter back in

	fire_frames[0]=(ent->client->aim)?79:4;
	fire_frames[1]=(ent->client->aim)?80:5;

	ent->client->p_fract= &ent->client->mags[gbr_index].submg_fract;
	ent->client->p_rnd= &ent->client->mags[gbr_index].submg_rnd;

	ent->client->crosshair = false;

	Weapon_Generic (ent, 
		 3,  5, 45, 
		71, 71, 75,
		78, 80, 92, 
		
		pause_frames, fire_frames, Weapon_Sten_Fire);

}

/////////////////////////////////////////////////
// Bren (LMG)
/////////////////////////////////////////////////

void Weapon_Bren (edict_t *ent)
{
	static int	pause_frames[]	= {0};//{23, 45, 0};
	static int	fire_frames[2];

	fire_frames[0]=(ent->client->aim)?77:4;
	fire_frames[1]=(ent->client->aim)?78:5;

	ent->client->p_fract= &ent->client->mags[gbr_index].lmg_fract;
	ent->client->p_rnd= &ent->client->mags[gbr_index].lmg_rnd;

	ent->client->crosshair = false;

	Weapon_Generic (ent, 
		 3,  5, 44,
		69, 69, 73,
		76, 78, 89,
		
		pause_frames, fire_frames, Weapon_Bren_Fire);
}


/////////////////////////////////////////////////
// Heavy Machine Gun (Vickers 'K')
/////////////////////////////////////////////////

void Weapon_Vickers (edict_t *ent)
{
	static int	pause_frames[]	= {0};//{38, 61, 0};
	static int	fire_frames[2];

	fire_frames[0]=(ent->client->aim)?104:20;
	fire_frames[1]=(ent->client->aim)?105:21;
//	fire_frames[2]=(ent->client->aim)?91:0;

	ent->client->p_rnd= &ent->client->mags[gbr_index].hmg_rnd;

	ent->client->crosshair = false;

	// Nick 23/11/2002 - Parts 7th frame bolt.wav (added bolt.wav to weapon cache preload also).
	if (ent->client->ps.gunframe == 7)
	ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex("gbr/vickers/bolt.wav"), 1, ATTN_NORM, 0);

	Weapon_Generic (ent, 
		19, 21,  61, 
		93, 96,  99,
		104, 105, 116,
		
		pause_frames, fire_frames, Weapon_Vickers_Fire);
}


/////////////////////////////////////////////////
// PIAT
/////////////////////////////////////////////////

void Weapon_PIAT (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[1];

	// Nick 25/11/2002 added next declare for empty barrel hack.
	int mag_index;

	mag_index = ent->client->pers.weapon->mag_index;

	fire_frames[0]=(ent->client->aim)?83:4;

	ent->client->p_rnd = &ent->client->mags[gbr_index].antitank_rnd;

	ent->client->crosshair = false;

	// Nick 25/11/2002 - Hack to make the 'barrel' empty after firing a shell.
	// I added 10 extra 'empty' frames in the idle cycle, and if PIAT is not
	// loaded it will cycle through that as opposed to the 'loaded' frames.
	//  If this is used as a 'if else' logic test, the frames go wonky on reload animation.

		if (ent->client->mags[mag_index].antitank_rnd < 1) {

	Weapon_Generic (ent, 
		 3,  45, 56, 
		75, 75, 79, 
		82, 85, 96,
					
		pause_frames, fire_frames, Weapon_PIAT_Fire);
		} 

		if (ent->client->mags[mag_index].antitank_rnd > 0) {

		Weapon_Generic (ent, 
		 3,  5, 45, 
		75, 75, 79, 
		82, 85, 96,
					
		pause_frames, fire_frames, Weapon_PIAT_Fire);
	}
}
// End Nick


/////////////////////////////////////////////////
// Lee Enfield Sniper Springfield Sniper Rifle
/////////////////////////////////////////////////

void Weapon_Enfields (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[4];

	fire_frames[0]=(ent->client->aim)?62:4;//fire here
	fire_frames[1]=(ent->client->aim)?71:0;//sniper bolt
	fire_frames[2]=(ent->client->aim)?86:0;//sniper start zoom
	fire_frames[3]=(ent->client->aim)?91:0;//sniper end zoom

	ent->client->p_fract= &ent->client->mags[gbr_index].sniper_fract;
	ent->client->p_rnd= &ent->client->mags[gbr_index].sniper_rnd;

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
			&& ent->client->ps.gunframe!=((ent->client->aim)?61:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?62:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?63:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?64:6)
			&& ent->client->ps.gunframe!=((ent->client->aim)?65:7)
			&& ent->client->ps.gunframe!=((ent->client->aim)?66:8)
			&& ent->client->ps.gunframe!=((ent->client->aim)?67:9)
			&& ent->client->ps.gunframe!=((ent->client->aim)?68:10)
			&& ent->client->ps.gunframe!=((ent->client->aim)?69:11)
			&& ent->client->ps.gunframe!=((ent->client->aim)?70:12)
			&& ent->client->ps.gunframe!=((ent->client->aim)?71:13)
			&& ent->client->ps.gunframe!=((ent->client->aim)?72:14)
			&& ent->client->ps.gunframe!=((ent->client->aim)?73:15)
			&& ent->client->ps.gunframe!=((ent->client->aim)?74:16)
			&& ent->client->ps.gunframe!=((ent->client->aim)?75:17)
			&& ent->client->ps.gunframe!=((ent->client->aim)?76:18)
			&& ent->client->ps.gunframe!=((ent->client->aim)?77:19)
			&& ent->client->ps.gunframe!=((ent->client->aim)?78:20)
			&& ent->client->ps.gunframe!=((ent->client->aim)?79:21)
			&& ent->client->ps.gunframe!=((ent->client->aim)?80:22)
			&& ent->client->ps.gunframe!=((ent->client->aim)?81:23)
			&& ent->client->ps.gunframe!=((ent->client->aim)?82:24)
			&& ent->client->ps.gunframe!=((ent->client->aim)?83:25)
			&& ent->client->ps.gunframe!=((ent->client->aim)?84:26)
			&& ent->client->ps.gunframe!=((ent->client->aim)?85:3)
			&& ent->client->ps.gunframe!=((ent->client->aim)?86:4)
			&& ent->client->ps.gunframe!=((ent->client->aim)?87:5)
			&& ent->client->ps.gunframe!=((ent->client->aim)?88:6)
			&& ent->client->ps.gunframe!=((ent->client->aim)?89:7)
			&& ent->client->ps.gunframe!=((ent->client->aim)?90:8)
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

	// Nick 25/12/2002 - Terrible, terrible hack to get the zoomout right
	// after firing a shot, rather than the gun suddenly appearing idle.
	// Two new 'remove from eye' frames added/replaced on model.
	if (ent->client->ps.gunframe >=69 && ent->client->ps.gunframe <= 71) {
		ent->client->crosshair = false;
		ent->client->ps.fov = STANDARD_FOV;
		// Nick 27/12/2002 - Need the next bit to stop reload animation
		// half starting the zoom-in again then going back to idle if reload 
		// (use weapon) button is hit immediately after firing a shot.
		// Had me beat for 2 hours 8=)
		ent->client->aim = false;
	}

	Weapon_Generic (ent, 
		 3, 26, 26, 
		53, 53, 58, 
		61, 92, 92, 
		
		pause_frames, fire_frames, Weapon_Sniper_Fire);
}

void Weapon_Vickers_Fire (edict_t *ent)  //faf:  this was taken from the game dll- weapon_hmg_fire base
{
	int			i;
	int			shots=1;
	vec3_t		start;
	vec3_t		forward, right, up;
	vec3_t		offset;
	vec3_t		angles;
	int			kick = 30;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage=guninfo->damage_direct;

	if (ent->client->next_fire_frame > ptrlevel->framenum)
		return;

	//Wheaty: Disable HMG while standing, totally
	if (ent->stanceflags == STANCE_STAND && (ent->client->buttons & BUTTON_ATTACK))
	{
		if (ent->client->machinegun_shots == 0) {
			ptrgi->centerprintf(ent, "Kneel when you fire the bloody Vickers!!\n"); // Nick
			ent->client->machinegun_shots = 1;
		}
		return;
	}

	// Nick 22/11/2002 - Next lines taken out - was for the BIG Vickers.
	//if (!ent->client->aim && (ent->client->buttons & BUTTON_ATTACK))
	//{
	//	ptrgi->cprintf(ent, PRINT_HIGH, "You must be aiming to fire!\n");
	//	return;
	//}


// this is for when the trigger is released
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if (!ent->client->aim)
			ent->client->ps.gunframe = guninfo->LastFire;
		else 
			ent->client->ps.gunframe=guninfo->LastAFire;

		ent->client->weapon_sound = 0;
		ent->client->machinegun_shots=0;
		
		ent->client->buttons &= ~BUTTON_ATTACK;
		ent->client->latched_buttons &= ~BUTTON_ATTACK;
		ent->client->weaponstate = WEAPON_READY;

		return;
	}

	// pbowens: the following assumes HMGs use only 2 firing frames
	i = (ptrlevel->framenum % 2) ? 1 : 0;

	if (ent->client->aim)
		ent->client->ps.gunframe = guninfo->AFO[i];
	else
		ent->client->ps.gunframe = guninfo->FO[i];


/*
	if (ent->client->mags[mag_index].hmg_rnd < shots)
		shots = ent->client->mags[mag_index].hmg_rnd;
*/
	if (ent->client->p_rnd && *ent->client->p_rnd == 0)
	{
		if (ent->client->weaponstate != WEAPON_FIRING)
			return;

		if (ptrlevel->time >= ent->pain_debounce_time)
		{
			ptrgi->sound(ent, CHAN_VOICE, ptrgi->soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = ptrlevel->time + 1;
		}
//		if(auto_weapon_change->value) NoAmmoWeaponChange (ent);

		 if (ent->client->aim) 
			 ent->client->ps.gunframe = guninfo->LastAFire;
		 else 
			 ent->client->ps.gunframe = guninfo->LastFire;

		 ent->client->weaponstate = WEAPON_READY;

		 return;
	}

	//ent->client->ps.gunframe++;

	// get start / end positions
	
	//if not crouched, make gun jump sporadicly
	if (ent->stanceflags == STANCE_STAND || !ent->client->aim)
	{	
		for (i=0 ; i<3 ; i++)
		{
			//rezmoth - changed for new firing system
			ent->client->kick_origin[i] = (crandom() * 3.35)-1.5;
			ent->client->kick_angles[i] += (crandom() * 13.7)-1.5;
		}
		//rezmoth - changed for new firing system
		ent->client->kick_origin[0] = crandom() * 0.35;
		ent->client->kick_angles[0] += ent->client->machinegun_shots * -1.8;
		// Raise HMG faster
		ent->client->machinegun_shots += 2;
		VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
		AngleVectors (angles, forward, right, up);

	}
	
	else
	{
		for (i=0 ; i<3 ; i++)
		{
			//rezmoth - changed for new firing system
			ent->client->kick_origin[i] = crandom() * 0.35;
			ent->client->kick_angles[i] += crandom() * 0.7;
		}
		VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
		AngleVectors (angles, forward, right, up);
	}
	
	// Instead of limit, force the aim down and start over for jumpiness
	if (ent->client->machinegun_shots > 10)
		ent->client->machinegun_shots -= 10;

//	for (i=0 ; i<shots ; i++)
//	{
		// get start / end positions

		//rezmoth - changed for new firing system
		//if(!ent->client->aim) 
		//	//VectorSet(offset, 50, 9, ent->viewheight-6);
		//	VectorSet(offset, 50, 9, crandom() * 40);
		//else 
		//	VectorSet(offset, 50, 0, ent->viewheight-1);

		//rezmoth - changed for new firing system
		/*
		if (!strcmp(ent->client->pers.weapon->ammo, "mg42_mag"))
		{
			VectorSet(offset, 200, 0, ent->viewheight + 4);
		} else if (!strcmp(ent->client->pers.weapon->ammo, "hmg_mag")) {
			VectorSet(offset, 0, 0, ent->viewheight + 20);
		} else {
			gi.dprintf("*** Firing System Error\n");
		}
		*/

		//start[2] += ent->viewheight;
		//start[2] += ent->viewheight;

		VectorSet(offset, 0, 0, ent->viewheight - 0);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		// rezmoth - tracers moved to here
		//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, true);
		++ent->numfired;

		if (ent->numfired % TRACERSHOT == 1)
			fire_tracer (ent, start, forward, damage, mod);
		else
//			fire_gun(ent, start, forward, damage, kick, 0, 0, mod, false);
			fire_gun2(ent, start, forward, damage, kick, 0, 0, mod, false);

		
		//faf:  fire 2 bullets.  to simulate 900 rpm firing rate ****************
		if (ent->numfired % 2 == 1)
		{
			for (i=0 ; i<3 ; i++)
			{
				//rezmoth - changed for new firing system
				ent->client->kick_origin[i] = crandom() * 0.35;
				ent->client->kick_angles[i] += crandom() * 0.7;
			}
			VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
			AngleVectors (angles, forward, right, up);
			
			VectorSet(offset, 0, 0, ent->viewheight - 0);
			P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

			fire_gun2(ent, start, forward, damage, kick, 0, 0, mod, false);
		}
		//**********************************************************************




		// rezmoth - changed to new firing code
		//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, true);

//	}

//	Play_WepSound(ent,guninfo->FireSound);//
//	PlayerNoise(ent, start, PNOISE_WEAPON);
//faf	ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf
	if (ent->numfired % 2 == 1)
		ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex("gbr/vickers/firea.wav"), 1, ATTN_NORM, 0);//faf
	else
		ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex("gbr/vickers/fireb.wav"), 1, ATTN_NORM, 0);//faf



//	gi.sound(ent, CHAN_WEAPON, gi.soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);
	
	// send muzzle flash
	ptrgi->WriteByte (svc_muzzleflash);
	ptrgi->WriteShort (ent-g_edicts);
	ptrgi->WriteByte (MZ_MACHINEGUN);// | is_silenced);
	ptrgi->multicast (ent->s.origin, MULTICAST_PVS);

	if (ent->client->p_rnd && *ent->client->p_rnd==1)
	  { 
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload + 1;
        ent->client->weaponstate = WEAPON_END_MAG;
//		Play_WepSound(ent,guninfo->LastRoundSound);
		
		//faf: this is equivalent to using the play_wepsound function
		ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf
		

	}


	//if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
	//	ent->client->pers.inventory[ent->client->ammo_index] -= shots;

	if (ent->numfired % 2 == 1)
		ent->client->mags[mag_index].hmg_rnd-= 2;// faf  shots;
	else
		ent->client->mags[mag_index].hmg_rnd-= 1;// faf  shots;
//	if(ent->client->mags[mag_index].hmg_rnd==0 && auto_reload->value) Cmd_Reload_f(ent);
	ent->client->next_fire_frame = ptrlevel->framenum + guninfo->frame_delay;

}


void Weapon_Bren_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			kick = 2;
	vec3_t		offset;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage = guninfo->damage_direct;

	if (ent->client->next_fire_frame > ptrlevel->framenum)
		return;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if(ent->client->aim)
	{
		if (ent->client->ps.gunframe == guninfo->LastAFire)
			ent->client->ps.gunframe = guninfo->LastAFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastAFire;
	}

	else
	{
		if (ent->client->ps.gunframe == guninfo->LastFire)
			ent->client->ps.gunframe = guninfo->LastFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastFire;
	}

	if (!ent->client->mags[mag_index].lmg_rnd)
	{
		ent->client->ps.gunframe = (ent->client->aim)?guninfo->LastAFire+1:guninfo->LastFire+1;
	
		if (ptrlevel->time >= ent->pain_debounce_time)
		{
			ptrgi->sound(ent, CHAN_VOICE, ptrgi->soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = ptrlevel->time + 1;
		}
		
/*
		if (auto_weapon_change->value) 
			NoAmmoWeaponChange (ent);
*/
		
		return;
	}


	// raise the gun as it is firing
//	if (!deathmatch->value)
//	{
	if ((ent->stanceflags == STANCE_STAND) || (!ent->client->aim))
		ent->client->machinegun_shots++;

	if (ent->client->machinegun_shots > 9)
		ent->client->machinegun_shots = 9;

	if ((!ent->stanceflags == STANCE_STAND) && (ent->client->aim))
		ent->client->machinegun_shots = 0;

//	}

	// vspread
	//VectorSet(offset, 0, (ent->client->aim)?0:8, ent->viewheight-8 + (crandom() * 15));
	// rezmoth - changed for new firing system
	//VectorSet(offset, 0, (ent->client->aim)?0:8, (ent->client->aim)?ent->viewheight-8:crandom() * 15);
	if (ent->client->pers.weapon->position == LOC_L_MACHINEGUN)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//10
	else
		ptrgi->dprintf("*** Firing System Error\n");

	// rezmoth - cosmetic recoil
	if (ptrlevel->framenum % 3 == 0)
	{
		if (ent->client->aim)
			ent->client->kick_angles[0] -= 1.5;
		else
			ent->client->kick_angles[0] = -3;
	}

	// pbowens: for darwin's 3.2 kick
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	/*	
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
*/

	fire_gun2(ent, start, forward, damage, kick, 0, 0, mod, false);

	if(ent->client->mags[mag_index].lmg_rnd==1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
//faf	Play_WepSound(ent,guninfo->LastRoundSound);
		ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex(guninfo->LastRoundSound), 1, ATTN_NORM, 0);//faf

	}

	// rezmoth - changed to new firing code
	//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, false);
/*	
	    ent->client->anim_priority = ANIM_ATTACK;
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
    {
        ent->s.frame = FRAME_crattak1 - 1 + (ent->client->ps.gunframe % 3);
        ent->client->anim_end = FRAME_crattak9;
    }
    else
    {
        ent->s.frame = FRAME_attack1 - 1 + (ent->client->ps.gunframe % 3);
        ent->client->anim_end = FRAME_attack8;
    }
*/
	//gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/machgf2b.wav"), 1, ATTN_NORM, 0);


//faf	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);
	ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf

	ptrgi->WriteByte (svc_muzzleflash);
	ptrgi->WriteShort (ent-g_edicts);
	ptrgi->WriteByte (MZ_MACHINEGUN);// | is_silenced);
	ptrgi->multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->mags[mag_index].lmg_rnd--;
	ent->client->next_fire_frame = ptrlevel->framenum + guninfo->frame_delay;
}

void Weapon_Bayonet_Fire (edict_t *ent)
{
	vec3_t  forward, right;
    vec3_t  start;
    vec3_t  offset;
	vec3_t g_offset;
	
	VectorCopy (vec3_origin,g_offset);

    AngleVectors (ent->client->v_angle, forward, right, NULL);
    VectorSet(offset, 24, 8, ent->viewheight-8);
    VectorAdd (offset, g_offset, offset);
    P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

    VectorScale (forward, -2, ent->client->kick_origin);
    ent->client->kick_angles[0] = -1;
 
	//gi.dprintf("going to fire_knife\n");
	// Nick 30/11/2002 - New bayonet sound
	fire_Knife2 (ent, start, forward, 50, 0, "gbr/bayonet/hit.wav", 0);

	ent->client->ps.gunframe++;

//	Play_WepSound(ent, (armedfists)?"fists/fire.wav":"knife/fire.wav");  //faf
	// Nick 30/11/2002 - New bayonet sound
	ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex("gbr/bayonet/swipe.wav"), 1, ATTN_NORM, 0);//faf

	//	gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
	PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
}

void fire_Knife2 ( edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, char *wav, qboolean fists)
{    
    trace_t tr; //detect whats in front of you up to range "vec3_t end"

    vec3_t end;

    // Figure out what we hit, if anything:

	//faf:  30 = bayonet range (knife range = 20)
    VectorMA (start, 30, aimdir, end);  //calculates the range vector                      
    tr = ptrgi->trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);
                        // figuers out what in front of the player up till "end"
    
   // Figure out what to do about what we hit, if anything

    if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))    
    {
        if (tr.fraction < 1.0)        
        {            
            if (tr.ent->takedamage)            
            {
                //This tells us to damage the thing that in our path...hehe
                T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_NO_KNOCKBACK,(fists)?MOD_FISTS:MOD_KNIFE);
                // Nick 30/11/2002 - New bayonet sound
				ptrgi->sound (self, CHAN_AUTO, ptrgi->soundindex((fists)?wav:"gbr/bayonet/stab.wav") , 1, ATTN_NORM, 0); 

            }        
            else        
            {                
                ptrgi->WriteByte (svc_temp_entity);    
                ptrgi->WriteByte (TE_SPARKS);
                ptrgi->WritePosition (tr.endpos);    
                ptrgi->WriteDir (tr.plane.normal);
                ptrgi->multicast (tr.endpos, MULTICAST_PVS);

                ptrgi->sound (self, CHAN_AUTO, ptrgi->soundindex(wav) , 1, ATTN_NORM, 0);

            }    
        }
    }
    return;
} 


void Weapon_Enfield_Fire (edict_t *ent)
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

//faf: hacked bayonet code
	if ((ent->client->buttons & BUTTON_ATTACK) &&
		(ent->client->mags[mag_index].rifle_rnd == 0) &&
//		(ent->client->weaponstate == WEAPON_READY) &&
		(ent->client->ps.gunframe < 104) &&
		(!ent->client->aim))
		{
			ent->client->ps.gunframe = 104;
			Weapon_Bayonet_Fire (ent);
			return;
		}
	if (ent->client->ps.gunframe == 108)
	{
		ent->client->ps.gunframe = 18;
		return;
	}

	if (ent->client->ps.gunframe > 104)
	{
		ent->client->ps.gunframe++;
		return;
	}
//end bayonet

	if (ent->client->mags[mag_index].rifle_rnd != 1) 
		ent->client->ps.gunframe++;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if (ent->client->aim)
			ent->client->ps.gunframe = guninfo->LastAFire;
		else
			ent->client->ps.gunframe = guninfo->LastFire;

		ent->client->machinegun_shots = 0;
//		ent->client->ps.gunframe++;
		ent->client->buttons |= BUTTON_ATTACK;
		//ent->client->latched_buttons &= ~BUTTON_ATTACK;
		ent->client->weaponstate = WEAPON_READY;
		return;
	}

	if (ent->client->next_fire_frame > ptrlevel->framenum)
		ent->client->ps.gunframe = ((ent->client->aim)? guninfo->LastAFire : guninfo->LastFire) + 1;


	if ( *ent->client->p_rnd == 0 )
	{
		ent->client->ps.gunframe = ((ent->client->aim)? guninfo->LastAFire : guninfo->LastFire) + 1;

		 if (ptrlevel->time >= ent->pain_debounce_time)
		 {
			 ptrgi->sound(ent, CHAN_VOICE, ptrgi->soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
			 ent->pain_debounce_time = ptrlevel->time + 1;
		 }

//		if (auto_reload->value)
//			Cmd_Reload_f(ent);
		return;
	}

//	ent->client->ps.gunframe++;
	if (ent->client->next_fire_frame > ptrlevel->framenum)
		return;
	ent->client->next_fire_frame = ptrlevel->framenum + guninfo->frame_delay;

	if (ent->client->mags[mag_index].rifle_rnd == 1) 
	{ // last round fire sounds
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
		//fafPlay_WepSound(ent,guninfo->LastRoundSound);
		ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex(guninfo->LastRoundSound), 1, ATTN_NORM, 0);//faf

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
	if (ent->client->pers.weapon->position = LOC_RIFLE)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//2
	else
		ptrgi->dprintf("*** Firing System Error\n");

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	fire_gun2(ent, start, forward, damage, kick, 0, 0, mod, false);
	// rezmoth - cosmetic recoil
	if (ent->client->aim)
		ent->client->kick_angles[0] -= 2.5;
	else
		ent->client->kick_angles[0] -= 5;
/*
	if (ent->client->mags[mag_index].rifle_rnd == 1) 
	{ // last round fire sounds
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
		Play_WepSound(ent,guninfo->LastRoundSound);

	}
*/
	// rezmoth - changed to new firing code
	//fire_rifle (ent, start, forward, damage, kick, mod);

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
    }
*/
	ent->client->mags[mag_index].rifle_rnd--;
//faf	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);
	ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf



	ptrgi->WriteByte (svc_muzzleflash);
	ptrgi->WriteShort (ent-g_edicts);
	ptrgi->WriteByte (MZ_MACHINEGUN);//faf | is_silenced);
	ptrgi->multicast (ent->s.origin, MULTICAST_PVS);
	
//	ent->client->next_fire_frame = level.framenum + guninfo->frame_delay;

}

void Weapon_Sten_Fire (edict_t *ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			kick = 2;
	vec3_t		offset;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage = guninfo->damage_direct;


	int randnum; //faf

	srand(rand());
	randnum=rand()%300;

	if (ent->client->next_fire_frame > ptrlevel->framenum)
		return;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if(ent->client->aim)
	{
		if (ent->client->ps.gunframe == guninfo->LastAFire)
			ent->client->ps.gunframe = guninfo->LastAFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastAFire;
	}

	else
	{
		if (ent->client->ps.gunframe == guninfo->LastFire)
			ent->client->ps.gunframe = guninfo->LastFire-1;
		else
			ent->client->ps.gunframe = guninfo->LastFire;
	}

	if (!ent->client->mags[mag_index].submg_rnd)
	{
		ent->client->ps.gunframe = (ent->client->aim)?guninfo->LastAFire+1:guninfo->LastFire+1;
	
		if (ptrlevel->time >= ent->pain_debounce_time)
		{
			ptrgi->sound(ent, CHAN_VOICE, ptrgi->soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = ptrlevel->time + 1;
		}
		
/*
		if (auto_weapon_change->value) 
			NoAmmoWeaponChange (ent);
*/
		
		return;
	}

	if (!ent->client->aim)
	{
		for (i=0 ; i<3 ; i++)
		{
			// Thompson extra-kick (1.7 to 1.9)
			// rezmoth - changed for new firing system
			//ent->client->kick_origin[i] = (crandom() * 0.5)-1.5;
			//ent->client->kick_angles[i] = (crandom() * 1.9)-1.5;
		}

		// rezmoth - changed for new firing system
		//ent->client->kick_origin[0] = crandom() * 0.38;
		//ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.8;

	}
	else
	{
		for (i=1 ; i<3 ; i++)
		{	
			// rezmoth - changed for new firing system
			//ent->client->kick_origin[i] = crandom() * 0.35;
			//ent->client->kick_angles[i] = crandom() * 0.7;
		}

		// rezmoth - changed for new firing system
		//ent->client->kick_origin[0] = crandom() * 0.35;
		//ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;
	}

	// raise the gun as it is firing
//	if (!deathmatch->value)
//	{
	ent->client->machinegun_shots++;
	if (ent->client->machinegun_shots > 9)
		ent->client->machinegun_shots = 9;
//	}

	// vspread
	//VectorSet(offset, 0, (ent->client->aim)?0:8, ent->viewheight-8 + (crandom() * 15));
	// rezmoth - changed for new firing system
	//VectorSet(offset, 0, (ent->client->aim)?0:8, (ent->client->aim)?ent->viewheight-8:crandom() * 15);
	if (ent->client->pers.weapon->position == LOC_SUBMACHINEGUN)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//10
	else
		ptrgi->dprintf("*** Firing System Error\n");

//faf for testing	ptrgi->cprintf(ent, PRINT_HIGH, "%i randnum\n", randnum);

//	jamchance = rand() % 100;
	if (randnum == 5)
	{
		// Nick 23/11/2002 - add a specific Sten jam noise (also declared in weapon pre-cache).
		ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex("gbr/sten/jam.wav"), 1, ATTN_NORM, 0);
		// End Nick
		ptrgi->centerprintf(ent, "The bloody Sten jammed!\n"); // Nick 23/11/2002 - Britishised.
		ent->client->mags[mag_index].submg_rnd= 0;
		return;
	}


	// rezmoth - cosmetic recoil
	if (ptrlevel->framenum % 3 == 0)
	{
		if (ent->client->aim)
			ent->client->kick_angles[0] -= 1.5;
		else
			ent->client->kick_angles[0] = -3;
	}

	// pbowens: for darwin's 3.2 kick
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1;
	ent->client->kick_angles[1] = ent->client->machinegun_shots * .3;

	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	/*	
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
*/

	fire_gun2(ent, start, forward, damage, kick, 0, 0, mod, false);

	if(ent->client->mags[mag_index].submg_rnd==1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
//		Play_WepSound(ent,guninfo->LastRoundSound);
		ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex(guninfo->LastRoundSound), 1, ATTN_NORM, 0);//faf

	
	}

	// rezmoth - changed to new firing code
	//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, false);
/*	
	    ent->client->anim_priority = ANIM_ATTACK;
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
    {
        ent->s.frame = FRAME_crattak1 - 1 + (ent->client->ps.gunframe % 3);
        ent->client->anim_end = FRAME_crattak9;
    }
    else
    {
        ent->s.frame = FRAME_attack1 - 1 + (ent->client->ps.gunframe % 3);
        ent->client->anim_end = FRAME_attack8;
    }
*/
	//gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/machgf2b.wav"), 1, ATTN_NORM, 0);


//	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);
	ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf

	
	
	ptrgi->WriteByte (svc_muzzleflash);
	ptrgi->WriteShort (ent-g_edicts);
	ptrgi->WriteByte (MZ_MACHINEGUN);//faf | is_silenced);
	ptrgi->multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->mags[mag_index].submg_rnd--;
	ent->client->next_fire_frame = ptrlevel->framenum + guninfo->frame_delay;
}

//faf:  gotta move fire_gun in here like this
void fire_gun2(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod, qboolean calcv)
{
	// standard defines
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;

	// rezmoth - start dist trace
	vec3_t	diststart, dist;
	vec3_t	distend = {0, 0, -8192};
	trace_t	disttr;

	VectorCopy(self->s.origin, diststart);	// initial value
	//VectorAdd(start, ent->mins, start); // go to the bottom of the player
	VectorAdd(diststart, distend, distend);			// add distance for end

	disttr = ptrgi->trace (diststart, self->mins, self->maxs, distend, self, MASK_SOLID);
	VectorSubtract(self->s.origin, disttr.endpos, dist);
	// rezmoth - end dist trace

	// Extra debugging propaganda
	//gi.dprintf("self    %s\n", self->client->pers.netname);
	//gi.dprintf("damage  %i\n", damage);
	//gi.dprintf("kick    %i\n", kick);
	//gi.dprintf("mod     %i\n", mod);
	//gi.dprintf("calcv   %s\n", (calcv) ? "true":"false");

	// Useful debugging information
	//gi.dprintf("hspread %i\n", hspread);
	//gi.dprintf("vspread %i\n", vspread);
	//showvector("start   ", start);
	//showvector("aimdir  ", aimdir);

	// fetch trace results
	tr = ptrgi->trace(self->s.origin, NULL, NULL, start, self, MASK_SHOT);

	// if the trace hit anything before distance termination
	if (!(tr.fraction < 1.0))
	{
		// seperate the aimdir into three parts
		vectoangles(aimdir, dir);
		AngleVectors(dir, forward, right, up);

		// rezmoth - TODO: fix this part
		// random spread calculation
		calcv = false;
		//r = (calcv) ? (crandom() * hspread) : hspread;
		//u = (calcv) ? (crandom() * vspread) : vspread;

		// add spread to hip shots
		if (!self->client->aim)
		{
			r = crandom() * 600;
			u = crandom() * 600;
		} else {
			r = crandom() * 50;
			u = crandom() * 50;
		}

		if (VectorLength(dist) > 20 && self->velocity[2] != 0)
		{
			r = crandom() * 1600;
			u = crandom() * 1600;
		}

		// end = start[i] + 8192 * forward[i]
		VectorMA (start, 8192, aimdir, end);//forward, end);
		// scale right angle by calculated horizontal spread
		VectorMA (end, r, right, end);
		// scale up angle by calculated vertical spread
		VectorMA (end, u, up, end);

		// if trace starts in water?
		if (ptrgi->pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			// remove water from possible impacts during trace
			content_mask &= ~MASK_WATER;
		}

		// retrace from point of impact with water
		tr = ptrgi->trace (start, NULL, NULL, end, self, content_mask);

		// if trace impacts dead player

//faf	if (tr.contents & MASK_DEADSOLID)
//faf		SprayBlood(self, tr.endpos, up, 0, MOD_UNKNOWN);
//faf:  sprayblood wont work here


		// more spread calculation
		if(calcv) calcVspread(self,&tr);

		// if the trace impacts water?
		if (tr.contents & MASK_WATER)
		{
			int		color;
			water = true;

			// copy trace's impact with water (end) to water_start
			VectorCopy (tr.endpos, water_start);

			// if the trace's start and end were not the same
			if (!VectorCompare (start, tr.endpos))
			{
				// if trace impacts water
				if (tr.contents & CONTENTS_WATER)
				{
					// if water is brown
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					// if water is blue
					else
						color = SPLASH_BLUE_WATER;
				}
				// if trace impacts slime
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				// if trace impacts lava
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				// if trace impacts unknown water
				else
					color = SPLASH_UNKNOWN;

				// if trace impacted known water
				if (color != SPLASH_UNKNOWN)
				{
					// display water splash particles
					ptrgi->WriteByte (svc_temp_entity);
					ptrgi->WriteByte (TE_SPLASH);
					ptrgi->WriteByte (8);
					ptrgi->WritePosition (tr.endpos);
					ptrgi->WriteDir (tr.plane.normal);
					ptrgi->WriteByte (color);
					ptrgi->multicast (tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, aimdir, end);//faf forward, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}

			// retrace starting from impact with water
			tr = ptrgi->trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	// if trace does not impact a surface and the surface is not the sky
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		// if the trace impacted anything before distance termination
		if (tr.fraction < 1.0)
		{
			// if the impacted player can take damage
			if (tr.ent->takedamage)
			{
				// damage impacted player
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				// if the trace impacted a surface other than the sky
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					// display impact on surface
					ptrgi->WriteByte (svc_temp_entity);
					if (crandom() < 0.5)
						ptrgi->WriteByte (TE_GUNSHOT);
					else
						ptrgi->WriteByte (TE_BULLET_SPARKS);
					ptrgi->WritePosition (tr.endpos);
					ptrgi->WriteDir (tr.plane.normal);
					ptrgi->multicast (tr.endpos, MULTICAST_PVS);

					// output impact sound
					if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
//						ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf

											
				}
			}
		}
	}

	// if trace impacted water
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

		// display bubble trail
		ptrgi->WriteByte (svc_temp_entity);
		ptrgi->WriteByte (TE_BUBBLETRAIL);
		ptrgi->WritePosition (water_start);
		ptrgi->WritePosition (tr.endpos);
		ptrgi->multicast (pos, MULTICAST_PVS);
	}
} 

void Weapon_PIAT_Fire (edict_t *ent)
{

	vec3_t	offset, start;
	vec3_t	forward, right;

	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	radius_damage = guninfo->damage_radius; //The *damage* within the radius
	int	damage		= guninfo->damage_direct;

	float	damage_radius; // The *radius* of the damage

	// Nick 28/11/2002 - flag for incorrect firing.
	short int		firewrong;
	firewrong = 0;


//faf: for testing	ptrgi->bprintf (PRINT_HIGH, "%i machinegun shots\n", ent->client->machinegun_shots); 

	if (ent->client->next_fire_frame > ptrlevel->framenum)
		return;

	// Nick 28/11/2002 - Tidied up the warnings somewhat (i.e. made it specific to the problem).
	// and of course 'Bristishised' them :).

	// Remove the old warnings first.

	//	if (ent->stanceflags == STANCE_STAND 
	//	ent->stanceflags == STANCE_CRAWL ||
	//	ent->client->movement			 ||
	//   !ent->client->aim				 ||
	//    ptrgi->pointcontents(ent->s.origin) & MASK_WATER) //Wheaty: Don't let them fire in water
	//{
	//	ptrgi->cprintf(ent, PRINT_HIGH, "You must kneel (crouch), be on dry land, and aim before firing that thing!\n");
		


		// Add the new ones.
		if (ptrgi->pointcontents(ent->s.origin) & MASK_WATER) {
			if (ent->client->machinegun_shots == 0)
				ptrgi->centerprintf(ent, "Get out the water to fire!!\n");
			firewrong = 1;
			ent->client->machinegun_shots = 1;
				}

		else if (!ent->client->aim) {

			if (ent->client->machinegun_shots == 0)
				ptrgi->centerprintf(ent, "You have gotta AIM it, Tommy Atkins!!\n");
			firewrong = 1;
			ent->client->machinegun_shots = 1;

				}

		else if (ent->client->movement) {

			if (ent->client->machinegun_shots == 0)
				ptrgi->centerprintf(ent, "Stop bloody moving when you fire!!\n");
			firewrong = 1;
			ent->client->machinegun_shots = 1;

				}

		else if ((ent->stanceflags == STANCE_STAND) || (ent->stanceflags == STANCE_CRAWL)) {

			if (ent->client->machinegun_shots == 0)
				ptrgi->centerprintf(ent, "Kneel when you fire that bloody thing!!\n");
			firewrong = 1;
			ent->client->machinegun_shots = 1;

				}

		//else if (ent->stanceflags == STANCE_CRAWL) {
		//	if (ent->client->machinegun_shots == 0)
		//		ptrgi->centerprintf(ent, "Get up on one knee, Tommy Atkins!!\n");
		//	firewrong = 1;
		//	ent->client->machinegun_shots = 1;
		//		}

		if (firewrong == 1) {
		//gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
		//ent->pain_debounce_time = level.time + 1;
		//ent->client->ps.gunframe= (ent->client->aim)?guninfo->LastAFire + 1:
		//													guninfo->LastFire + 1;

		// Nick - 25/11/2002 Had to add this to allow the new 'empty' idle frames to work here.

			if (ent->client->mags[mag_index].antitank_rnd == 0) {
			ent->client->ps.gunframe = 46;
			} else {
			// Original code follows.
			ent->client->ps.gunframe= (ent->client->aim)?guninfo->LastAFire + 1:
															guninfo->LastFire + 1;
			}
		// End Nick

 		ent->client->weapon_sound = 0;
		ent->client->weaponstate=WEAPON_READY;
		return;
	}

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		if (ent->client->aim)
			ent->client->ps.gunframe = guninfo->LastAFire+1;
		else
			ent->client->ps.gunframe = guninfo->LastFire+1;
		return;
	}

	// pbowens: rasied rocket dmg from 175 to 225
	damage_radius = 175;//faf 225;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorScale (forward, -2, ent->client->kick_origin);
	//ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight-8);			//z,x,y
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (!ent->client->mags[mag_index].antitank_rnd)
	 {

		ent->client->ps.gunframe = (ent->client->aim) ? guninfo->LastAFire+1 : guninfo->LastFire+1;
		 if (ptrlevel->time >= ent->pain_debounce_time)
		 {
			 ptrgi->sound(ent, CHAN_VOICE, ptrgi->soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
			 ent->pain_debounce_time = ptrlevel->time + 1;
		 }

//		if (auto_reload->value)
//			Cmd_Reload_f(ent);

		return;
	}

	if (ent->client->mags[mag_index].antitank_rnd == 1) { // last round fire sounds

		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
//		Play_WepSound(ent,guninfo->LastRoundSound);
		ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex(guninfo->LastRoundSound), 1, ATTN_NORM, 0);//faf

	}
/*
	if (ent->client->pers.inventory[ent->client->ammo_index] == 1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
		Play_WepSound(ent,guninfo->LastRoundSound);
	}

	if (!ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = guninfo->LastFire;
		
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}

		//Make the user change weapons MANUALLY!

		if(auto_weapon_change->value) 
			NoAmmoWeaponChange (ent);

		return;
	}
	*/

	fire_rocket2 (ent, start, forward, damage, 1600, damage_radius, radius_damage); //faf: was 1400 
	// rezmoth - cosmetic recoil
	ent->client->kick_angles[0] -= 7;
	ent->client->kick_origin[2] -= 5;

//	Play_WepSound(ent,guninfo->FireSound);//PlayerNoise(ent, start, PNOISE_WEAPON);
	ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf

	// send muzzle flash
	ptrgi->WriteByte (svc_muzzleflash);
	ptrgi->WriteShort (ent-g_edicts);
	ptrgi->WriteByte (MZ_ROCKET);
	ptrgi->multicast (ent->s.origin, MULTICAST_PVS);
	
	ent->client->ps.gunframe++;


	//ent->client->pers.inventory[ent->client->ammo_index]--;
	ent->client->mags[mag_index].antitank_rnd--;
	ent->client->next_fire_frame = ptrlevel->framenum + guninfo->frame_delay;

}

//faf:  adds gravity to the rocket
void fire_rocket2 (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_BOUNCE,//MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	//rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = ptrgi->modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = ptrlevel->time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = ptrgi->soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";
	
	rocket->gravity =  1;//.9; //faf


//faf:causing error and dont think is needed	if (self->client)
//faf		check_dodge (self, rocket->s.origin, dir, speed);

	ptrgi->linkentity (rocket);
}

void Weapon_Pistol_Fire2 (edict_t *ent)
{
	int		i;		//temp var
	int		kick=2;
	
	vec3_t		offset;
	vec3_t		forward, right;
	vec3_t		start;
	vec3_t		angles;
	GunInfo_t *guninfo=ent->client->pers.weapon->guninfo;	
	int mag_index=ent->client->pers.weapon->mag_index;
	int mod=guninfo->MeansOfDeath;
	int	damage=guninfo->damage_direct;
	
	if (ent->client->next_fire_frame > ptrlevel->framenum)
		return;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	ent->client->ps.gunframe++;     

	if ((!ent->client->mags[mag_index].pistol_rnd))
	{
		// ent->client->ps.gunframe = guninfo->LastFire+1; // Nick 21/12/2002 - This is the 'bug' line. Can push the
		// if (ptrlevel->time >= ent->pain_debounce_time)  // frame # past the idle animation into reload sequence!
		// {  4 lines here 'removed'.  If these are used, then use the added frame change code
		// in the Weapon_Webley code.
          ptrgi->sound(ent, CHAN_VOICE, ptrgi->soundindex("weapons/noammo.wav"),1, ATTN_NORM, 0);
          ent->pain_debounce_time = ptrlevel->time + 1;
		// } End Nick

//Make the user change weapons MANUALLY!
	
//	if(auto_weapon_change->value) NoAmmoWeaponChange (ent);
	return;
	}

	for (i=1 ; i<3 ; i++)
	{
		// rezmoth - changed for new firing system
		//ent->client->kick_origin[i] = crandom() * 0.35;
		//ent->client->kick_angles[i] = crandom() * 0.7;
	}
	// rezmoth - changed for new firing system
	//ent->client->kick_origin[0] = crandom() * 0.35;
	//ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;



	// get start / end positions
	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors (angles, forward, right, NULL);

	// rezmoth - changed for new firing system
	if (ent->client->pers.weapon->position == LOC_PISTOL)
		VectorSet(offset, 0, 0, ent->viewheight - 0);	//10
	else
		ptrgi->dprintf("*** Firing System Error\n");

	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

//	if (ent->client->pers.inventory[ent->client->ammo_index] == 1 || 

	fire_gun2(ent, start, forward, damage, kick, 0, 0, mod, false);
	// rezmoth - cosmetic recoil
	if (ent->client->aim)
		ent->client->kick_angles[0] -= 2;
	else
		ent->client->kick_angles[0] -= 5;

	if(ent->client->mags[mag_index].pistol_rnd==1)
	{
		//Hard coded for reload only.
        ent->client->ps.gunframe=guninfo->LastReload+1;
        ent->client->weaponstate = WEAPON_END_MAG;
//		Play_WepSound(ent,guninfo->LastRoundSound);
		ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex(guninfo->LastRoundSound), 1, ATTN_NORM, 0);//faf

	}

	// rezmoth - changed to new firing code
	//fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, mod, false);	

	ent->client->mags[mag_index].pistol_rnd--;
/*

	ent->client->anim_priority = ANIM_ATTACK;i
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
*/    
//faf	ptrgi->sound(ent, CHAN_WEAPON, DoAnarchyStuff(ent,guninfo->FireSound), 1, ATTN_NORM, 0);
		ptrgi->sound(ent, CHAN_WEAPON, ptrgi->soundindex(guninfo->FireSound), 1, ATTN_NORM, 0);//faf

	ptrgi->WriteByte (svc_muzzleflash);
	ptrgi->WriteShort (ent-g_edicts);
	ptrgi->WriteByte (MZ_MACHINEGUN);//faf | is_silenced);
	ptrgi->multicast (ent->s.origin, MULTICAST_PVS);

	//if(!ent->sexpistols)Play_WepSound(ent,guninfo->FireSound);
	//PlayerNoise(ent, start, PNOISE_WEAPON);

//	if(ent->client->mags[mag_index].pistol_rnd==0 && auto_reload->value) Cmd_Reload_f(ent);
	ent->client->next_fire_frame = ptrlevel->framenum + guninfo->frame_delay;	
	
}
