/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: beholder.h
  Description: Function declarations and all the miserable
               datatypes (see b_define.h)

\**********************************************************/

#ifndef BEHOLDER_H
#define BEHOLDER_H

// Topaz:
// C.E.C Cannon functions: TOP_CECCannon.C
void fire_CEC (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect);
void CEC_Fire (edict_t *ent);
void Weapon_CECCannon_Fire (edict_t *ent);
void Weapon_CECCannon (edict_t *ent);
void CEC_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void CEC_Kill (edict_t *ent);

// Shred
void Weapon_Sniper (edict_t *ent);

// CCH
// Chris Hilton (dctank@jump.net)
// Proximity Launcher functions: CCH_Proxim.C
void Weapon_ProximLauncher (edict_t *ent);

// CCH
// Chris Hilton (dctank@jump.net)
// RCDC Launcher: CCH_RCDC.C
void Weapon_RCDCLauncher (edict_t *ent);
void Blow_Charges (edict_t *ent);

// CCH
// Chris Hilton (dctank@jump.net)
// Cloak: CCH_Cloak.C
void Use_Cloak (edict_t *ent, gitem_t *item);


// Topaz:
// Navigation nodes functions:

void SP_func_node (edict_t *self);
map_node_t *G_SpawnNode (void);
qboolean visible_vector(vec3_t self, vec3_t other);
void findnodepath (map_node_t *node1, map_node_t *node2);
map_node_t *pathnode (map_node_t *node1, map_node_t *node2);
map_node_t *nearnode (map_node_t *from, vec3_t org, float rad);
void tell_node(edict_t *ent);
void find_node_target(edict_t *ent);

// Navigation info:
extern int  node_hops, max_hops;

// BOT:
qboolean BOT_movestep (edict_t *ent, vec3_t move, qboolean relink);
qboolean BOT_StepDirection (edict_t *ent, float yaw, float dist);
void BOT_NewChaseDir (edict_t *actor, map_node_t *node, float dist);
qboolean BOT_CloseEnough (edict_t *ent, vec3_t end, float dist);
void BOT_MoveToGoal (edict_t *ent, float dist);
void BOT_walk (edict_t *self, float dist);
void BOT_run (edict_t *self, float dist);
void BOT_stand (edict_t *self, float dist);
void BOT_move (edict_t *self, float dist);
void BOT_move_right (edict_t *self, float dist);
void BOT_move_left (edict_t *self, float dist);
void BOT_charge (edict_t *self, float dist);
qboolean BOT_walkmove (edict_t *ent, float yaw, float dist);

// Camera Code:
void SP_func_camera (edict_t *ent);
void SP_func_trampoline (edict_t *ent);
void Cmd_camera_f(edict_t *self, edict_t *other);
void camera_think (edict_t *self);
void Cmd_add_camera(edict_t *ent);
void Cmd_load_camera(edict_t *ent);
void kill_camera (edict_t *ent);
void camera_3rd (edict_t *ent);
void camera_think_3rd (edict_t * self);
void camera_scene (edict_t *ent);
void camera_think_scene (edict_t * self);

// Random Speakers:
void randomspeaker_think (edict_t *self);
void SP_target_randomspeaker (edict_t *ent);

// FlashLight:
void Cmd_FlashLight (edict_t *self, int);
void FlashLightThink (edict_t *self);

// Debug Code
void draw_nodes(edict_t *ent);

// Startup NNE:
void close_doors();


// Scripts:
void load_script(edict_t *ent);
void parse_cmd(edict_t *ent);
void script_see(edict_t *ent, edict_t *other);
void script_touch(edict_t *ent, edict_t *other);
int script_pain(edict_t *ent, edict_t *other);
void script_block(edict_t *ent, edict_t *other);
void fix_slope(edict_t *ent);


void scrBOT_run (edict_t *self, float dist);
void scrBOT_stand (edict_t *self, float dist);

// Data:
extern edict_t *Ent1, *Ent2;
extern edict_t *bullet_hole[35];
extern int bullet_index;

#endif