/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/rus/rus_item.c,v $
 *   $Revision: 1.7 $
 *   $Date: 2002/06/04 19:49:50 $
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

#include "rus_main.h"

// g_items.c
// D-Day: Normandy Team Items


extern GunInfo_t rusguninfo[];
extern spawn_t sp_rus[]; 

void L_Use_Weapon(edict_t *ent, gitem_t *item)
{
	Use_Weapon(ent,item);
}

qboolean L_Pickup_Weapon(edict_t *ent, edict_t *item)
{
	return(Pickup_Weapon(ent,item));
}

void L_Drop_Weapon (edict_t *ent, gitem_t *item)
{
	Drop_Weapon (ent, item);
}

qboolean L_Pickup_Ammo (edict_t *ent, edict_t *other)
{
	return(Pickup_Ammo(ent,other));
}

void L_Drop_Ammo(edict_t *ent, gitem_t *item)
{
	Drop_Ammo (ent,item);
}
void L_Masher(edict_t *ent)
{
	Weapon_Grenade(ent);
}

//////////////////////////////////////////////////////////////////////////////////////
//                              rus.DLL ITEMS                                       //
//////////////////////////////////////////////////////////////////////////////////////


gitem_t rusitems[]=
{
/* Tokarev TT33 becomes the standard issue weapon
*/
	{
		"weapon_tt33",
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_tt33,
		"misc/w_pkup.wav",
		"models/weapons/rus/g_tt33/tris.md2", 0,
		"models/weapons/rus/v_tt33/tris.md2",
		"w_tt33", 
		"Tokarev TT33",
		0,
		1,
		"tt33_mag",
		IT_WEAPON,
		NULL,
		0,
		LOC_PISTOL,
		2,
		1,
		5000,
		100,
		"rus",
/* precache */ "rus/tt33/fire.wav rus/tt33/reload.wav rus/tt33/unload.wav",
		&rusguninfo[tt33_FRAME]
},	
/*QUAKED weapon_m9130 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_m9130", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_m9130,
		"misc/w_pkup.wav",
		"models/weapons/rus/g_m9130/tris.md2", 0, //EF_ROTATE
		"models/weapons/rus/v_m9130/tris.md2",
/* icon */		"w_m9130",
/* pickup */	"M91/30",
		0,
		1,
		"m9130_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_RIFLE,
		2,
		9,
		5000,
		100,
		"rus",
/* precache */ "rus/m9130/fire.wav rus/m9130/lastround.wav rus/m9130/reload.wav rus/m9130/unload.wav",
		&rusguninfo[m9130_FRAME]
	},

/*QUAKED weapon_ppsh41 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_ppsh41", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_ppsh41,
		"misc/w_pkup.wav",
		"models/weapons/rus/g_ppsh41/tris.md2", 0,
		"models/weapons/rus/v_ppsh41/tris.md2",
/* icon */		"w_ppsh41",
/* pickup */	"PPsh41",
		0,
		1,
		"ppsh41_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SUBMACHINEGUN,
		2,
		10,
		5000,
		100,
		"rus",
/* precache */ "rus/ppsh41/fire.wav rus/ppsh41/reload.wav rus/ppsh41/unload.wav",
	&rusguninfo[ppsh41_FRAME]
	},

/*QUAKED weapon_pps43 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_pps43", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_pps43,
		"misc/w_pkup.wav",
//		"models/weapons/g_bar/tris.md2", EF_ROTATE,
		"models/weapons/rus/g_pps43/tris.md2", 0,
		"models/weapons/rus/v_pps43/tris.md2",
/* icon */		"w_pps43",
/* pickup */	"PPS43",
		0,
		1,
		"pps43_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_L_MACHINEGUN,
		2,
		20,
		5000,
		100,
		"rus",
/* precache */ "rus/pps43/fire.wav rus/pps43/reload.wav rus/pps43/unload.wav",
	&rusguninfo[pps43_FRAME]
	},

	/*QUAKED weapon_dpm (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_dpm", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_dpm,
		"misc/w_pkup.wav",
		"models/weapons/rus/g_dpm/tris.md2", 0,
		"models/weapons/rus/v_dpm/tris.md2",
/* icon */		"w_dpm",
/* pickup */	"DPM",
		0,
		1,
		"dpm_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_H_MACHINEGUN,
		2,
		50,
		5000,
		100,
		"rus",
/* precache */ "rus/dpm/fire.wav rus/dpm/reload.wav rus/dpm/unload.wav",
	&rusguninfo[dpm_FRAME]
	},


/*QUAKED weapon_rpg1 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rpg1",
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_RPG1,
		"misc/w_pkup.wav",
//		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/rus/g_panzer/tris.md2", 0,
		"models/weapons/rus/v_panzer/tris.md2",
/* icon */		"w_panzer",
/* pickup */	"RPG-1",
		0,
		1,
		"rus_rockets",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_ROCKET,
		3,
		10,
		0,
		0,
		"rus",
/* precache */ "rus/panzer/fire.wav rus/panzer/reload.wav rus/panzer/rockfly.wav models/objects/rocket/tris.md2 models/objects/debris2/tris.md2",
	&rusguninfo[PSK_FRAME]
	},


/*QUAKED weapon_m9130s (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_m9130s", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_m9130s,
		"misc/w_pkup.wav",
		"models/weapons/rus/g_m9130s/tris.md2", 0,
		"models/weapons/rus/v_m9130s/tris.md2",
/* icon */		"w_m9130s",
/* pickup */	"M91/30 Sniper",
		0,
		1,
		"m9130_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SNIPER,
		2,
		8,
		5000,
		100,
		"rus",
/* precache */ "rus/m9130s/fire.wav rus/m9130s/lastround.wav rus/m9130s/reload.wav rus/m9130s/unload.wav",
	&rusguninfo[m9130S_FRAME]
	},

//////////////////////////////////////////////////////////////////////////////////////
//                                   AMMO                                           //
//////////////////////////////////////////////////////////////////////////////////////

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades_rus",
		L_Pickup_Ammo,
		L_Use_Weapon,
		L_Drop_Ammo,
		L_Masher,
		"misc/am_pkup.wav",
		"models/weapons/rus/g_grenade/tris.md2", 0,
		"models/weapons/rus/v_grenade/tris.md2",
/* icon */		"a_f1grenade",
/* pickup */	"F1 Grenade",
/* width */		3,
		5,
		"F1 Grenade",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_GRENADES,
		LOC_GRENADES,
		0,
		0.25,
		0,
		0,
		"rus",
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav weapons/throw.wav"
	},

/*QUAKED ammo_tt33 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_tt33",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/pistols/tris.md2", 0,
		NULL,
/* icon */		"a_tt33",
/* pickup */	"tt33_mag",
/* width */		3,
		tt33_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_PISTOL,
		0,
		0,
		0.25,
		0,
		0,
		"rus",
/* precache */ ""
	},

	
/*QUAKED ammo_m9130 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_m9130",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rifle/tris.md2", 0,
		NULL,
/* icon */		"a_m9130",
/* pickup */	"m9130_mag",
/* width */		3,
		MC_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_RIFLE,
		0,
		0,
		0.25,
		0,
		0,
		"rus",
/* precache */ ""
	},

/*QUAKED ammo_ppsh41 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_ppsh41",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rus_drum2/tris.md2", 0,
		NULL,
/* icon */		"a_ppsh41",
/* pickup */	"ppsh41_mag",
/* width */		3,
		ppsh41_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_SUBMG,
		0,
		0,
		0.25,
		0,
		0,
		"rus",
/* precache */ ""
	},


/*QUAKED ammo_pps43 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_pps43",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rus_pph43/tris.md2", 0,
		NULL,
/* icon */		"a_pps43",
/* pickup */	"pps43_mag",
/* width */		3,
		pps43_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_LMG,
		0,
		0,
		0.5,
		0,
		0,
		"rus",
/* precache */ ""
	},

/*QUAKED ammo_dpm (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_dpm",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rus_drum1/tris.md2", 0,
		NULL,
/* icon */		"a_dpm",
/* pickup */	"dpm_mag",
/* width */		3,
		dpm_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_HMG,
		0,
		0,
		1,
		0,
		0,
		"rus",
/* precache */ ""
	},

	
/*QUAKED ammo_rocketsG (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_rocketsG",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rockets/tris.md2", 0,
		NULL,
/* icon */		"a_panzer",
/* pickup */	"rus_rockets",
/* width */		3,
		PSK_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_ROCKETS,
		0,
		0,
		3,
		0,
		0,
		"rus",
/* precache */ ""
	},


{
	NULL
},
};

void InitItems(void)
{
	int i;
	ptrgi->dprintf("  |-- rus_index: %i\n\n",rus_index);
	for(i=0;;i++)
	{
		if(!rusitems[i].classname) break;
		rusitems[i].mag_index=rus_index;
		PlayerInsertItem(&rusitems[i],&sp_rus[i]);

	}
}







