/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/gbr/gbr_main.h,v $
 *   $Revision: 1.8 $
 *   $Date: 2002/07/23 21:11:37 $
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

#define USER_EXCLUDE_FUNCTIONS 1
#include "gbr_defines.h"
#include "../g_local.h"
#include "../g_cmds.h"
#include "../g_dll.h"

int gbr_index;
byte *is_silenced;
edict_t *g_edicts;


//g_items.c
void InitItems(void);
qboolean (*Pickup_Weapon)(edict_t *, edict_t *);
void (*Drop_Weapon)(edict_t *, gitem_t *);

void(*Use_Weapon)(edict_t *, gitem_t *);
qboolean (*Pickup_Ammo)(edict_t *, edict_t *);
void (*Drop_Ammo)(edict_t *, gitem_t *);
void misc_banner_think (edict_t *ent);

//float *CVscope_setting;
//gbr.c

game_import_t *ptrgi;
game_export_t *ptrGlobals;
level_locals_t *ptrlevel;
game_locals_t *ptrGame;


void (*PlayerInsertCommands)(g_cmds_t *, int, char *);
void (*(*PlayerFindFunction)(char *t));
gitem_t *(*PlayerInsertItem)(gitem_t *it, spawn_t *spawn);

gitem_t *(*FindItem)(char *);
void (*SpawnItem)(edict_t *, gitem_t *);
gitem_t *(*FindItemByClassname)(char *);

// p_weapon.c
void (*Weapon_Generic)(edict_t *, int, int, int, int,int, int,int,int,int,int*, int*, void (*fire)(edict_t *ent));
void (*ifchangewep)(edict_t *ent);
void (*fire_bullet)(edict_t *, vec3_t, vec3_t, int, int, int, int, int, qboolean);
void (*ifchangewep)(edict_t *); 
void (*AngleVectors)(vec3_t , vec3_t, vec3_t, vec3_t);
void (*P_ProjectSource)(gclient_t *, vec3_t, vec3_t, vec3_t, vec3_t, vec3_t);
void (*PlayerNoise)(edict_t *, vec3_t, int);
qboolean (*Cmd_Reload_f)(edict_t *);
void (*fire_rifle)(edict_t *, vec3_t, vec3_t, int, int, int);
void (*VectorScale)(vec3_t, vec_t, vec3_t);
void (*fire_rocket)(edict_t *, vec3_t, vec3_t, int, int, float, int);
void (*PBM_FireFlameThrower)(edict_t *, vec3_t, vec3_t, vec3_t, int, vec3_t, vec3_t, int , int);
void (*Weapon_Pistol_Fire)(edict_t *);
void (*Weapon_Rifle_Fire)(edict_t *);
void (*Weapon_Submachinegun_Fire)(edict_t *);
void (*Weapon_LMG_Fire)(edict_t *);
void (*Weapon_HMG_Fire)(edict_t *);
void (*Weapon_Rocket_Fire)(edict_t *);
void (*Weapon_Sniper_Fire)(edict_t *);
void (*Weapon_Grenade)(edict_t *);


void Weapon_Webley (edict_t *ent);
void Weapon_Enfield(edict_t *ent);
void Weapon_Sten(edict_t *ent);
void Weapon_Bren(edict_t *ent);
void Weapon_Vickers(edict_t *ent);
void Weapon_PIAT(edict_t *ent);
void Weapon_Enfields(edict_t *ent);

// g_spawnn.c
void (*SP_misc_banner_generic) (edict_t *ent, char *model);


void Weapon_Bren_Fire (edict_t *ent);
void Weapon_Vickers_Fire (edict_t *ent);
void (*Play_WepSound)(edict_t *, char);
void (*fire_gun)(edict_t *, vec3_t, vec3_t, int, int, int, int, int, qboolean);

void (*fire_tracer)(edict_t *, vec3_t, vec3_t, int, int);
void (*fire_bullet)(edict_t *, vec3_t, vec3_t, int, int, int, int, int, qboolean);

//void L_fire_gun(edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod, qboolean calcv);

void (*vectoangles)(vec3_t, vec3_t);
vec_t (*VectorLength)(vec3_t);
void (*VectorMA)(vec3_t, float, vec3_t, vec3_t);
int (*calcVspread)(edict_t *, trace_t *);
vec_t (*VectorNormalize)(vec3_t);
void (*T_Damage)(edict_t *, edict_t *, edict_t *, vec3_t, vec3_t, vec3_t, int, int, int, int);
int (*VectorCompare)(vec3_t, vec3_t);
void (*G_FreeEdict)(edict_t *);
void (*rocket_touch) (edict_t *, edict_t *, cplane_t *, csurface_t *);
edict_t *(*G_Spawn) (void);
