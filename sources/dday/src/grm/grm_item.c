/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/grm/grm_item.c,v $
 *   $Revision: 1.8 $
 *   $Date: 2002/07/23 22:48:29 $
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

#include "grm_main.h"

// g_items.c
// D-Day: Normandy Team Items


extern GunInfo_t grmguninfo[];
extern spawn_t sp_grm[]; 

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
//                              GRM.DLL ITEMS                                       //
//////////////////////////////////////////////////////////////////////////////////////


gitem_t grmitems[]=
{
/* Walther P38 becomes the standard issue weapon
*/
	{
		"weapon_p38",
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_P38,
		"misc/w_pkup.wav",
		"models/weapons/grm/g_walther/tris.md2", 0,
		"models/weapons/grm/v_walther/tris.md2",
		"w_p38", 
		"Walther P38",
		0,
		1,
		"p38_mag",
		IT_WEAPON,
		NULL,
		0,
		LOC_PISTOL,
		2,
		1,
		5000,
		100,
		"grm",
/* precache */ "grm/p38/fire.wav grm/p38/reload.wav grm/p38/unload.wav",
		&grmguninfo[P38_FRAME]
},	
/*QUAKED weapon_mauser98k (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_mauser98k", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_m98k,
		"misc/w_pkup.wav",
		"models/weapons/grm/g_m98k/tris.md2", 0, //EF_ROTATE
		"models/weapons/grm/v_m98k/tris.md2",
/* icon */		"w_m98k",
/* pickup */	"Mauser 98k",
		0,
		1,
		"mauser98k_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_RIFLE,
		2,
		9,
		5000,
		100,
		"grm",
/* precache */ "grm/m98k/fire.wav grm/m98k/lastround.wav grm/m98k/reload.wav grm/m98k/unload.wav",
		&grmguninfo[M98K_FRAME]
	},

/*QUAKED weapon_mp40 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_mp40", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_MP40,
		"misc/w_pkup.wav",
		"models/weapons/grm/g_mp40/tris.md2", 0,
		"models/weapons/grm/v_mp40/tris.md2",
/* icon */		"w_mp40",
/* pickup */	"MP40",
		0,
		1,
		"mp40_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SUBMACHINEGUN,
		2,
		10,
		5000,
		100,
		"grm",
/* precache */ "grm/mp40/fire.wav grm/mp40/reload.wav grm/mp40/unload.wav",
	&grmguninfo[MP40_FRAME]
	},

/*QUAKED weapon_mp43 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_mp43", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_MP43,
		"misc/w_pkup.wav",
//		"models/weapons/g_bar/tris.md2", EF_ROTATE,
		"models/weapons/grm/g_mp43/tris.md2", 0,
		"models/weapons/grm/v_mp43/tris.md2",
/* icon */		"w_mp43",
/* pickup */	"MP43",
		0,
		1,
		"mp43_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_L_MACHINEGUN,
		2,
		20,
		5000,
		100,
		"grm",
/* precache */ "grm/mp43/fire.wav grm/mp43/reload.wav grm/mp43/unload.wav",
	&grmguninfo[MP43_FRAME]
	},

	/*QUAKED weapon_mg42 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_mg42", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_MG42,
		"misc/w_pkup.wav",
		"models/weapons/grm/g_mg42/tris.md2", 0,
		"models/weapons/grm/v_mg42/tris.md2",
/* icon */		"w_mg42",
/* pickup */	"MG42",
		0,
		1,
		"mg42_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_H_MACHINEGUN,
		2,
		50,
		5000,
		100,
		"grm",
/* precache */ "grm/mg42/fire.wav grm/mg42/reload.wav grm/mg42/unload.wav",
	&grmguninfo[MG42_FRAME]
	},


/*QUAKED weapon_panzer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_panzer",
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_Psk,
		"misc/w_pkup.wav",
//		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/grm/g_panzer/tris.md2", 0,
		"models/weapons/grm/v_panzer/tris.md2",
/* icon */		"w_panzer",
/* pickup */	"Panzerschreck",
		0,
		1,
		"grm_rockets",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_ROCKET,
		3,
		10,
		0,
		0,
		"grm",
/* precache */ "grm/panzer/fire.wav grm/panzer/reload.wav grm/panzer/rockfly.wav models/objects/rocket/tris.md2 models/objects/debris2/tris.md2",
	&grmguninfo[PSK_FRAME]
	},


/*QUAKED weapon_m98ks (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_m98ks", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_m98ks,
		"misc/w_pkup.wav",
		"models/weapons/grm/g_m98ks/tris.md2", 0,
		"models/weapons/grm/v_m98ks/tris.md2",
/* icon */		"w_m98ks",
/* pickup */	"Mauser 98ks",
		0,
		1,
		"mauser98k_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SNIPER,
		2,
		8,
		5000,
		100,
		"grm",
/* precache */ "grm/m98ks/fire.wav grm/m98ks/lastround.wav grm/m98ks/reload.wav grm/m98ks/unload.wav",
	&grmguninfo[M98KS_FRAME]
	},

//////////////////////////////////////////////////////////////////////////////////////
//                                   AMMO                                           //
//////////////////////////////////////////////////////////////////////////////////////

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades_grm",
		L_Pickup_Ammo,
		L_Use_Weapon,
		L_Drop_Ammo,
		L_Masher,
		"misc/am_pkup.wav",
		"models/weapons/grm/g_masher/tris.md2", 0,
		"models/weapons/grm/v_masher/tris.md2",
/* icon */		"a_masher",
/* pickup */	"Potato Masher",
/* width */		3,
		5,
		"Potato Masher",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_GRENADES,
		LOC_GRENADES,
		0,
		0.25,
		0,
		0,
		"grm",
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav weapons/throw.wav"
	},

/*QUAKED ammo_p38 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_p38",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/pistols/tris.md2", 0,
		NULL,
/* icon */		"a_p38",
/* pickup */	"p38_mag",
/* width */		3,
		P38_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_PISTOL,
		0,
		0,
		0.25,
		0,
		0,
		"grm",
/* precache */ ""
	},

	
/*QUAKED ammo_mauser98k (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_mauser98k",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/rifle/tris.md2", 0,
		NULL,
/* icon */		"a_m98k",
/* pickup */	"mauser98k_mag",
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
		"grm",
/* precache */ ""
	},

/*QUAKED ammo_mp40 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_mp40",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/smg/tris.md2", 0,
		NULL,
/* icon */		"a_mp40",
/* pickup */	"mp40_mag",
/* width */		3,
		MP40_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_SUBMG,
		0,
		0,
		0.25,
		0,
		0,
		"grm",
/* precache */ ""
	},


/*QUAKED ammo_mp43 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_mp43",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/lmg1/tris.md2", 0,
		NULL,
/* icon */		"a_mp43",
/* pickup */	"mp43_mag",
/* width */		3,
		MP43_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_LMG,
		0,
		0,
		0.5,
		0,
		0,
		"grm",
/* precache */ ""
	},

/*QUAKED ammo_mg42 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_mg42",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/hmg/tris.md2", 0,
		NULL,
/* icon */		"a_mg42",
/* pickup */	"mg42_mag",
/* width */		3,
		MG42_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_HMG,
		0,
		0,
		1,
		0,
		0,
		"grm",
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
/* pickup */	"grm_rockets",
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
		"grm",
/* precache */ ""
	},


{
	NULL
},
};

void InitItems(void)
{
	int i;
	ptrgi->dprintf("  |-- grm_index: %i\n\n",grm_index);
	for(i=0;;i++)
	{
		if(!grmitems[i].classname) break;
		grmitems[i].mag_index=grm_index;
		PlayerInsertItem(&grmitems[i],&sp_grm[i]);

	}
}







