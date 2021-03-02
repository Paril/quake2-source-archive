/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/jpn/jpn_item.c,v $
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

#include "jpn_main.h"

// g_items.c
// D-Day: Normandy Team Items


extern GunInfo_t jpnguninfo[];
extern spawn_t sp_jpn[]; 

void Molotov_Drop(edict_t *ent, gitem_t *item)
{
	return;
}

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
void Weapon_97_Grenade(edict_t *ent)
{
	Weapon_Grenade(ent);
}

//////////////////////////////////////////////////////////////////////////////////////
//                              JPN.DLL ITEMS                                       //
//////////////////////////////////////////////////////////////////////////////////////


gitem_t jpnitems[]=
{
/* Nambu Pistol becomes the standard issue weapon
*/
	{
		"weapon_nambu",
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_Nambu,
		"misc/w_pkup.wav",
		"models/weapons/jpn/g_nambu/tris.md2", 0,
		"models/weapons/jpn/v_nambu/tris.md2",
		"w_nambu", 
		"Nambu Pistol",
		0,
		1,
		"nambu_mag",
		IT_WEAPON,
		NULL,
		0,
		LOC_PISTOL,
		2,
		1,
		5000,
		100,
		"jpn",
/* precache */ "jpn/nambu/fire.wav jpn/nambu/reload.wav jpn/nambu/unload.wav",
		&jpnguninfo[NAMBU_FRAME]
},	
/*QUAKED weapon_arisaka (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_arisaka", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_Arisaka,
		"misc/w_pkup.wav",
		"models/weapons/jpn/g_arisaka/tris.md2", 0, //EF_ROTATE
		"models/weapons/jpn/v_arisaka/tris.md2",
/* icon */		"w_arisaka",
/* pickup */	"Arisaka Type 99",
		0,
		1,
		"arisaka_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_RIFLE,
		2,
		9,
		5000,
		100,
		"jpn",
/* precache */ "jpn/arisaka/fire.wav jpn/arisaka/lastround.wav jpn/arisaka/reload.wav jpn/arisaka/unload.wav",
		&jpnguninfo[ARISAKA_FRAME]
	},

/*QUAKED weapon_type_100 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_type_100", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_Type_100,
		"misc/w_pkup.wav",
		"models/weapons/jpn/g_type_100/tris.md2", 0,
		"models/weapons/jpn/v_type_100/tris.md2",
/* icon */		"w_type_100",
/* pickup */	"Type 100 SMG",
		0,
		1,
		"type_100_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SUBMACHINEGUN,
		2,
		10,
		5000,
		100,
		"jpn",
/* precache */ "jpn/type_100/fire.wav jpn/type_100/reload.wav jpn/type_100/unload.wav",
	&jpnguninfo[TYPE_100_FRAME]
	},

/*QUAKED weapon_type_99 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_type_99", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_Type_99,
		"misc/w_pkup.wav",
//		"models/weapons/g_bar/tris.md2", EF_ROTATE,
		"models/weapons/jpn/g_type_99/tris.md2", 0,
		"models/weapons/jpn/v_type_99/tris.md2",
/* icon */		"w_type_99",
/* pickup */	"Type 99 LMG",
		0,
		1,
		"type_99_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_L_MACHINEGUN,
		2,
		20,
		5000,
		100,
		"jpn",
/* precache */ "jpn/type_99/fire.wav jpn/type_99/reload.wav jpn/type_99/unload.wav",
	&jpnguninfo[TYPE_99_FRAME]
	},

/* QUAKED weapon_m1carb (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
/*	{
		"weapon_m1carb", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_M1Carbine,
		"misc/w_pkup.wav",
		"models/weapons/usa/g_m1carb/tris.md2", 0,
		"models/weapons/usa/v_m1carb/tris.md2",
/* icon */	//	"w_m1carb",
/* pickup *///	"M1 Carbine",
/*		0,
		1,
		"m1carb_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_H_MACHINEGUN,
		2,
		9,
		5000,
		100,
		"jpn",
/* precache */ /*"usa/m1carb/fire.wav usa/m1carb/reload.wav usa/m1carb/unload.wav",
		&jpnguninfo[M1CARB_FRAME]
	},
*/


/*QUAKED weapon_m98ks (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_arisakas", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_Arisakas,
		"misc/w_pkup.wav",
		"models/weapons/jpn/g_arisakas/tris.md2", 0,
		"models/weapons/jpn/v_arisakas/tris.md2",
/* icon */		"w_arisakas",
/* pickup */	"Arisaka Scoped",
		0,
		1,
		"arisakas_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SNIPER,
		2,
		8,
		5000,
		100,
		"jpn",
/* precache */ "jpn/arisakas/fire.wav jpn/arisakas/lastround.wav jpn/arisakas/reload.wav jpn/arisakas/unload.wav",
	&jpnguninfo[ARISAKAS_FRAME]
	},


/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_shotgun", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_Shotgun,
		"misc/w_pkup.wav",
		"models/weapons/usa/g_shotgun/tris.md2", 0,
		"models/weapons/usa/v_shotgun/tris.md2",
/* icon */		"w_shotgun",
/* pickup */	"Shotgun",
		0,
		1,
		"shotgun_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_ROCKET,//LOC_SUBMACHINEGUN,
		1,
		10,
		5000,
		100,
		"jpn",
/* precache */ "usa/shotgun/fire.wav usa/shotgun/reload.wav",
		&jpnguninfo[SHOTGUN_FRAME]
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
		"jpn",
/* precache */ "grm/mg42/fire.wav grm/mg42/reload.wav grm/mg42/unload.wav",
	&jpnguninfo[MG42_FRAME]
	},







/* Weapon_Katana (sword)
 
*/
{
       "weapon_katana", 
       L_Pickup_Weapon,
       L_Use_Weapon,                             //How to use
       L_Drop_Weapon,
       Weapon_Katana,                           //What the function is
       "misc/w_pkup.wav",
       "models/weapons/jpn/g_katana/tris.md2", 0, 
       "models/weapons/jpn/v_katana/tris.md2",      //The models stuff
       "w_katana",                                    //Icon to be used
       "Katana",                                        //Pickup name
       0,
       1,
       "Katana",
       IT_WEAPON,
       NULL,
       0,
	   LOC_KNIFE,
	   0,
	   0,
	   0,
		0,
		"jpn",
       "misc/fhit3.wav knife/fire.wav knife/hit.wav knife/pullout.wav"
		
		},


{
       "weapon_molotov", 
       L_Pickup_Ammo,
       L_Use_Weapon,                             //How to use
       L_Drop_Weapon,
       Weapon_Molotov,                           //What the function is
       "misc/w_pkup.wav",
       "models/weapons/jpn/g_molotov/tris.md2", 0, 
       "models/weapons/jpn/v_molotov/tris.md2",      //The models stuff
       "w_molotov",                                    //Icon to be used
       "Molotov Cocktail",                                        //Pickup name
       1,
       1,//5,
       "Molotov Cocktail",
       IT_WEAPON,
       NULL,
       0,//AMMO_ROCKETS,
	   0,//LOC_ROCKET,
	   0,
	   .25,
	   0,
		0,
		"jpn",
       "jpn/molotov/throw.wav jpn/molotov/light.wav jpn/molotov/break.wav jpn/molotov/pullout.wav"
	   

		},





//////////////////////////////////////////////////////////////////////////////////////
//                                   AMMO                                           //
//////////////////////////////////////////////////////////////////////////////////////

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades_jpn",
		L_Pickup_Ammo,
		L_Use_Weapon,
		L_Drop_Ammo,
		Weapon_97_Grenade,
		"misc/am_pkup.wav",
		"models/weapons/jpn/g_nade/tris.md2", 0,
		"models/weapons/jpn/v_nade/tris.md2",
/* icon */		"a_jpn_grenade",
/* pickup */	"Type 97 Grenade",
/* width */		3,
		5,
		"Type 97 Grenade",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_GRENADES,
		LOC_GRENADES,
		0,
		0.25,
		0,
		0,
		"jpn",
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav weapons/throw.wav"
	},

/*QUAKED ammo_nambu (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_nambu",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/jpn/pistolmag/tris.md2", 0,
		NULL,
/* icon */		"a_nambu",
/* pickup */	"nambu_mag",
/* width */		3,
		NAMBU_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_PISTOL,
		0,
		0,
		0.25,
		0,
		0,
		"jpn",
/* precache */ ""
	},

	
/*QUAKED ammo_mauser98k (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_arisaka",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/jpn/riflemag/tris.md2", 0,
		NULL,
/* icon */		"a_arisaka",
/* pickup */	"arisaka_mag",
/* width */		3,
		ARISAKA_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_RIFLE,
		0,
		0,
		0.25,
		0,
		0,
		"jpn",
/* precache */ ""
	},
	
/*QUAKED ammo_mauser98k (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_arisakas",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/jpn/riflemag/tris.md2", 0,
		NULL,
/* icon */		"a_arisakas",
/* pickup */	"arisakas_mag",
/* width */		3,
		ARISAKAS_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_SNIPER,
		0,
		0,
		0.25,
		0,
		0,
		"jpn",
/* precache */ ""
	},

/*QUAKED ammo_type_100 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_type_100",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/jpn/smgmag/tris.md2", 0,
		NULL,
/* icon */		"a_type_100",
/* pickup */	"type_100_mag",
/* width */		3,
		TYPE_100_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_SUBMG,
		0,
		0,
		0.25,
		0,
		0,
		"jpn",
/* precache */ ""
	},


/*QUAKED ammo_type_99 (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_type_99",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/jpn/lmgmag/tris.md2", 0,
		NULL,
/* icon */		"a_type_99",
/* pickup */	"type_99_mag",
/* width */		3,
		TYPE_99_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_LMG,
		0,
		0,
		0.5,
		0,
		0,
		"jpn",
/* precache */ ""
	},



/*QUAKED ammo_m1carb (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
/*	{
		"ammo_m1carb",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/m1/tris.md2", 0,
		NULL,
/* icon */	//	"a_m1carb",
/* pickup *///	"m1carb_mag",
/* width */	//	3,
/*		M1CARB_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_HMG,//AMMO_CARBINE,
		0,
		0,
		1,
		0,
		0,
		"jpn",
/* precache */ //""
//	},

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

/*QUAKED ammo_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_shotgun",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/shells/tris.md2", 0,
		NULL,
/* icon */		"a_shells",
/* pickup */	"shotgun_mag",
/* width */		3,
		SHOTGUNMAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_ROCKETS,
		0,
		0,
		0.25,
		0,
		0,
		"jpn",
/* precache */ ""
	},




{
	NULL
},
};

void InitItems(void)
{
	int i;
	ptrgi->dprintf("  |-- jpn_index: %i\n\n",jpn_index);
	for(i=0;;i++)
	{
		if(!jpnitems[i].classname) break;
		jpnitems[i].mag_index=jpn_index;
		PlayerInsertItem(&jpnitems[i],&sp_jpn[i]);

	}
}







