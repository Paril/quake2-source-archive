/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/gbr/gbr_item.c,v $
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

#include "gbr_main.h"

// g_items.c
// D-Day: Normandy Team Items


extern GunInfo_t gbrguninfo[];
extern spawn_t sp_gbr[]; 

void L_Use_Weapon(edict_t *ent, gitem_t *item) 
{ 
	Use_Weapon(ent,item); 
}

qboolean L_Pickup_Weapon(edict_t *ent, edict_t *item) 
{ 
	return (Pickup_Weapon(ent,item)); 
}

void L_Drop_Weapon (edict_t *ent, gitem_t *item) 
{	
	Drop_Weapon (ent, item); 
}

qboolean L_Pickup_Ammo (edict_t *ent, edict_t *other) 
{	
	return (Pickup_Ammo(ent,other)); 
}

void L_Drop_Ammo(edict_t *ent, gitem_t *item) 
{	
	Drop_Ammo (ent,item); 
}

void L_Weapon_Grenade (edict_t *ent)
{
	Weapon_Grenade(ent);
}



/*
//faf:  this one wont work in team dlls with  4.1
void L_Play_WepSound (edict_t *ent, char *sound)
{
	Play_WepSound(ent, *sound);
}

//faf: fire_gun wont work in team dlls with 4.1 so use fire_bullet
void L_fire_gun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod, qboolean calcv)
{
	fire_gun (self, start, aimdir, damage, kick, hspread, vspread, mod, calcv);
}

void L_fire_tracer (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int mod)
{
	fire_tracer (self, start, aimdir, damage, mod);
}

void L_fire_bullet (edict_t * self , vec3_t start , vec3_t aimdir , int damage , int kick , int hspread , int vspread , int mod , qboolean tracers_on )
{
	fire_bullet(self, start, aimdir , damage , kick , hspread , vspread, mod, tracers_on);
}


//faf: fire_gun2 stuff:
void L_vectoangles (vec3_t vec, vec3_t angles)
{
	vectoangles (vec, angles);
}

vec_t L_VectorLength (vec3_t v)
{
	return VectorLength (v);
}

void L_VectorMA (vec3_t veca , float scale , vec3_t vecb , vec3_t vecc )
{
	VectorMA(veca , scale , vecb , vecc );
}

int L_calcVspread (edict_t * ent , trace_t * tr )
{
	return calcVspread(ent, tr);
}

vec_t L_VectorNormalize(vec3_t v )
{
	return VectorNormalize(v);
}

void L_T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod)
{
	T_Damage (targ, inflictor, attacker, dir, point, normal, damage, knockback, dflags, mod);
}

int L_VectorCompare(vec3_t v1 , vec3_t v2 )
{
	return VectorCompare(v1 , v2);
}


//rocket stuff
void L_G_FreeEdict(edict_t * ed )
{
	G_FreeEdict(ed );
}

void L_rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	rocket_touch (ent, other, plane, surf);
}

// end faf
*/ // this stuff above wasnt necessary //faf




//////////////////////////////////////////////////////////////////////////////////////
//                              GBR.DLL ITEMS                                       //
//////////////////////////////////////////////////////////////////////////////////////

gitem_t gbritems[]=
{
/* Webley becomes the standard issue weapon */
	{
		"weapon_webley",
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_Webley,
		"misc/w_pkup.wav",
		"models/weapons/gbr/g_webley/tris.md2", 0,
		"models/weapons/gbr/v_webley/tris.md2",
/* icon */		"w_webley",
/* pickup */	"Webley",
		0,
		1,
		"webley_mag",
		IT_WEAPON,
		NULL,
		0,
		LOC_PISTOL,
		2,
		1,
		5000,
		100,
		"gbr",
/* precache */ "gbr/webley/fire.wav gbr/webley/reload.wav gbr/webley/unload.wav",
		&gbrguninfo[Webley_FRAME]
	},	
/* QUAKED weapon_Enfield (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_Enfield", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_Enfield,
		"misc/w_pkup.wav",
		"models/weapons/gbr/g_303/tris.md2", 0, //EF_ROTATE
		"models/weapons/gbr/v_303/tris.md2",
/* icon */		"w_303",
/* pickup */	"Lee Enfield",
		0,
		1,
		"303_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_RIFLE,
		2,
		9,
		5000,
		100,
		"gbr",
/* precache */ "gbr/303/fire.wav gbr/303/lastround.wav gbr/303/reload.wav gbr/303/unload.wav gbr/bayonet/hit.wav gbr/bayonet/stab.wav gbr/bayonet/swipe.wav",
		&gbrguninfo[Enfield_FRAME]
	},

/*QUAKED weapon_sten (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_sten", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_Sten,
		"misc/w_pkup.wav",
		"models/weapons/gbr/g_sten/tris.md2", 0,
		"models/weapons/gbr/v_sten/tris.md2",
/* icon */		"w_sten",
/* pickup */	"Sten",
		0,
		1,
		"sten_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SUBMACHINEGUN,
		2,
		10,
		5000,
		100,
		"gbr",
/* precache */ "gbr/sten/fire.wav gbr/sten/reload.wav gbr/sten/unload.wav gbr/sten/jam.wav",
		&gbrguninfo[Sten_FRAME]
	},

/*QUAKED weapon_Bren (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_Bren", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_Bren,
		"misc/w_pkup.wav",
		"models/weapons/gbr/g_bren/tris.md2", 0,
		"models/weapons/gbr/v_bren/tris.md2",
/* icon */		"w_bren",
/* pickup */	"Bren",
		0,
		1,
		"Bren_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_L_MACHINEGUN,
		2,
		20,
		5000,
		100,
		"gbr",
/* precache */ "gbr/bren/fire.wav gbr/bren/reload.wav gbr/bren/unload.wav",
		&gbrguninfo[Bren_FRAME],
		20
	},

	/*QUAKED weapon_vickers (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_vickers", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_Vickers,
		"misc/w_pkup.wav",
		"models/weapons/gbr/g_vickers/tris.md2", 0,
		"models/weapons/gbr/v_vickers/tris.md2",
/* icon */		"w_vickers",
/* pickup */	"Vickers",
		0,
		1,
		"vickers_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_H_MACHINEGUN,
		2,
		50,
		5000,
		100,
		"gbr",
/* precache */ "gbr/vickers/fire.wav gbr/vickers/reload.wav gbr/vickers/unload.wav gbr/vickers/bolt.wav",
		&gbrguninfo[Vickers_FRAME]
	},

	/*QUAKED weapon_PIAT (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_piat",
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_PIAT,
		"misc/w_pkup.wav",
		"models/weapons/gbr/g_piat/tris.md2", 0,
		"models/weapons/gbr/v_piat/tris.md2",
/* icon */		"w_piat",
/* pickup */	"PIAT",
		0,
		1,
		"gbr_rockets",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_ROCKET,
		3,
		10,
		0,
		0,
		"gbr",
/* precache */ "gbr/piat/fire.wav gbr/piat/reload.wav gbr/piat/rockfly.wav models/objects/rocket/tris.md2 models/objects/debris2/tris.md2",
		&gbrguninfo[PIAT_FRAME]
	},


/*QUAKED weapon_enfields (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_enfields", 
		L_Pickup_Weapon,
		L_Use_Weapon,
		L_Drop_Weapon,
		Weapon_Enfields,
		"misc/w_pkup.wav",
		"models/weapons/gbr/g_303s/tris.md2", 0,
		"models/weapons/gbr/v_303s/tris.md2",
/* icon */		"w_303s",
/* pickup */	"Lee Enfield Sniper",
		0,
		1,
		"303s_mag",
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
		LOC_SNIPER,
		2,
		8,
		5000,
		100,
		"gbr",
/* precache */ "gbr/303s/fire.wav gbr/303s/lastround.wav gbr/303s/reload.wav gbr/303s/unload.wav",
		&gbrguninfo[Enfields_FRAME]
	},

//////////////////////////////////////////////////////////////////////////////////////
//                                   AMMO                                           //
//////////////////////////////////////////////////////////////////////////////////////

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_grenades_gbr",
		L_Pickup_Ammo,
		L_Use_Weapon,
		L_Drop_Ammo,
		L_Weapon_Grenade,
		"misc/am_pkup.wav",
		"models/weapons/gbr/g_grenade/tris.md2", 0,
		"models/weapons/gbr/v_grenade/tris.md2",
/* icon */		"a_mills_bomb",
/* pickup */	"Mills Bomb",
/* width */		3,
		5,
		"Mills Bomb",
		IT_AMMO|IT_WEAPON,
		NULL,
		AMMO_GRENADES,
		LOC_GRENADES,
		0,
		0.25,
		0,
		0,
		"gbr",
/* precache */ "weapons/hgrent1a.wav weapons/hgrena1b.wav weapons/hgrenb1a.wav weapons/hgrenb2a.wav weapons/throw.wav "
	},

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_webley",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/gbr/webbullets/tris.md2", 0,
		NULL,
/* icon */		"a_webley",
/* pickup */	"webley_mag",
/* width */		3,
		WebleyMAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_PISTOL,
		0,
		0,
		0.25,
		0,
		0,
		"gbr",
/* precache */ ""
	},

	
/*QUAKED ammo_enfield (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_enfield",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/gbr/303mag/tris.md2", 0,
		NULL,
/* icon */		"a_303",
/* pickup */	"303_mag",
/* width */		3,
		Enfield_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_RIFLE,
		0,
		0,
		0.25,
		0,
		0,
		"gbr",
/* precache */ ""
	},

/*QUAKED ammo_sten (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_sten",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/gbr/stenmag/tris.md2", 0,
		NULL,
/* icon */		"a_sten",
/* pickup */	"sten_mag",
/* width */		3,
		StenMAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_SUBMG,
		0,
		0,
		0.25,
		0,
		0,
		"gbr",
/* precache */ ""
	},


/*QUAKED ammo_bren (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bren",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/gbr/brenmag/tris.md2", 0,
		NULL,
/* icon */		"a_bren",
/* pickup */	"bren_mag",
/* width */		3,
		Bren_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_LMG,
		0,
		0,
		0.5,
		0,
		0,
		"gbr",
/* precache */ ""
	},

/*QUAKED ammo_vickers (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_vickers",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/gbr/vickdrum/tris.md2", 0,
		NULL,
/* icon */		"a_vickers",
/* pickup */	"vickers_mag",
/* width */		3,
		Vickers_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_HMG,
		0,
		0,
		1,
		0,
		0,
		"gbr",
/* precache */ ""
	},

	
/*QUAKED ammo_piat (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_piat",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/gbr/piatrocks/tris.md2", 0,
		NULL,
/* icon */		"a_piat",
/* pickup */	"gbr_rockets",
/* width */		3,
		PIAT_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_ROCKETS,
		0,
		0,
		3,
		0,
		0,
		"gbr",
/* precache */ ""
	},


/*QUAKED ammo_enfields (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_enfields",
		L_Pickup_Ammo,
		NULL,
		L_Drop_Ammo,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/gbr/303smag/tris.md2", 0,
		NULL,
/* icon */		"a_303s",
/* pickup */	"303s_mag",
/* width */		3,
		Enfields_MAG,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_SNIPER,
		0,
		0,
		0.25,
		0,
		0,
		"gbr",
/* precache */ ""
	},

	


{
	NULL
},
};


void InitItems(void)
{
	int i;
	ptrgi->dprintf("  |-- gbr_index: %i\n\n",gbr_index);
	for(i=0;gbritems[i].classname;i++)
	{
		gbritems[i].mag_index=gbr_index;
		PlayerInsertItem(&gbritems[i],&sp_gbr[i]);
	}
}
