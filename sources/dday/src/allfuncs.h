/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/allfuncs.h,v $
 *   $Revision: 1.14 $
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

// File : \dday\source/g_ai.c
void AI_SetSightClient(void );   // found @ line:31
void ai_move(edict_t * self , float dist );   // found @ line:73
void ai_stand(edict_t * self , float dist );   // found @ line:87
void ai_walk(edict_t * self , float dist );   // found @ line:144
void ai_charge(edict_t * self , float dist );   // found @ line:175
void ai_turn(edict_t * self , float dist );   // found @ line:196
int range(edict_t * self , edict_t * other );   // found @ line:245
qboolean visible(edict_t * self , edict_t * other );   // found @ line:268
qboolean infront(edict_t * self , edict_t * other );   // found @ line:293
void HuntTarget(edict_t * self );   // found @ line:312
void FoundTarget(edict_t * self );   // found @ line:328
qboolean FindTarget(edict_t * self );   // found @ line:388
qboolean FacingIdeal(edict_t * self );   // found @ line:575
qboolean M_CheckAttack(edict_t * self );   // found @ line:588
void ai_run_melee(edict_t * self );   // found @ line:684
void ai_run_missile(edict_t * self );   // found @ line:704
void ai_run_slide(edict_t * self , float distance );   // found @ line:724
qboolean ai_checkattack(edict_t * self , float dist );   // found @ line:752
void ai_run(edict_t * self , float dist );   // found @ line:895

// File : \dday\source/feeder.c
void Feed_Ammo(edict_t * ent );   // found @ line:7

// File : \dday\source/g_arty.c

//void Pick_Arty(edict_t * ent , int battary );   // found @ line:18
void Cmd_Arty_f(edict_t * ent );   // found @ line:88
//void Fire_Arty(edict_t * battary , edict_t * ent );   // found @ line:153
//void Pick_Air(edict_t * ent , int battary );   // found @ line:191
//void Cmd_Airstrike_f(edict_t * ent );   // found @ line:257
//void Impact_Airstrike(edict_t * ent );   // found @ line:334
//void Think_Airstrike(edict_t * ent );   // found @ line:379
//void Impact_Arty(edict_t * ent );   // found @ line:415
void Think_Arty(edict_t * ent );   // found @ line:469

// File : \dday\source/g_cmds.c
void Cmd_AliciaMode_f(edict_t * ent );   // found @ line:13
void Cmd_SexPistols_f(edict_t * ent );   // found @ line:29
void Cmd_GameVersion_f(edict_t * ent );   // found @ line:81
void Cmd_Scope_f(edict_t * ent );   // found @ line:87
char * ClientTeam(edict_t * ent );   // found @ line:113
void SelectNextItem(edict_t * ent , int itflags );   // found @ line:140
void SelectPrevItem(edict_t * ent , int itflags );   // found @ line:175
void ValidateSelectedItem(edict_t * ent );   // found @ line:209
void Cmd_Give_f(edict_t * ent );   // found @ line:231
void Cmd_God_f(edict_t * ent );   // found @ line:368
void Cmd_Notarget_f(edict_t * ent );   // found @ line:397
void Cmd_Noclip_f(edict_t * ent );   // found @ line:424
gitem_t	*FindNextPickup (edict_t *ent, int location);
//gitem_t * FindNextPos(edict_t * ent , int position );   // found @ line:450
void Cmd_Use_f(edict_t * ent );   // found @ line:492
void Cmd_Drop_f(edict_t * ent );   // found @ line:593
void Cmd_Inven_f(edict_t * ent );   // found @ line:628
void Cmd_InvUse_f(edict_t * ent );   // found @ line:659
void Cmd_WeapPrev_f(edict_t * ent );   // found @ line:692
void Cmd_WeapNext_f(edict_t * ent );   // found @ line:728
void Cmd_WeapLast_f(edict_t * ent );   // found @ line:764
void Cmd_InvDrop_f(edict_t * ent );   // found @ line:791
void Cmd_Kill_f(edict_t * ent );   // found @ line:817
void Cmd_PutAway_f(edict_t * ent );   // found @ line:835
int PlayerSort(void const * a , void const * b );   // found @ line:843
void Cmd_Players_f(edict_t * ent );   // found @ line:865
void Cmd_Wave_f(edict_t * ent );   // found @ line:908
void Cmd_Say_f(edict_t * ent , qboolean team , qboolean arg0 );   // found @ line:959
void change_stance(edict_t * self , int stance );   // found @ line:1043
//void Cmd_Create_Team(edict_t * ent );   // found @ line:1084
//void Cmd_Join_team(edict_t * ent );   // found @ line:1152
//void Cmd_Quit_team(edict_t * ent );   // found @ line:1203
void Cmd_List_team(edict_t * ent );   // found @ line:1231
qboolean Cmd_Reload_f(edict_t * ent );   // found @ line:1251
//int fix_mos(char * argv );   // found @ line:1310
//void Cmd_MOS(edict_t * ent );   // found @ line:1340
void InsertCmds(g_cmds_t * cmds , int numCmds , char * src );   // found @ line:1389
void PrintCmds();   // found @ line:1418
void CleanUpCmds();   // found @ line:1441
g_cmds_t * FindCommand(char * cmd );   // found @ line:1466
void ClientCommand(edict_t * ent );   // found @ line:1516
void Cmd_Stance(edict_t * ent );   // found @ line:1552
void Cmd_Shout_f(edict_t * ent );   // found @ line:1567

// File : \dday\source/g_combat.c
//void dopromote(TeamS_t * team , edict_t * ent );   // found @ line:14
void promoteofficer(edict_t * ent );   // found @ line:24
qboolean CanDamage(edict_t * targ , edict_t * inflictor );   // found @ line:94
void Killed(edict_t * targ , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:154
void SpawnDamage(int type , vec3_t origin , vec3_t normal , int damage );   // found @ line:220
// static int CheckPowerArmor(edict_t * ent , vec3_t point , vec3_t normal , int damage , int dflags );   // found @ line:257
// static int CheckArmor(edict_t * ent , vec3_t point , vec3_t normal , int damage , int te_sparks , int dflags );   // found @ line:341
void M_ReactToDamage(edict_t * targ , edict_t * attacker );   // found @ line:381
qboolean CheckTeamDamage(edict_t * targ , edict_t * attacker );   // found @ line:447
qboolean In_Vector_Range(vec3_t point , vec3_t origin , float x_range , float y_range ,  float z_range );   // found @ line:455
int Damage_Loc(edict_t * targ , vec3_t point );   // found @ line:468
void T_Damage(edict_t * targ , edict_t * inflictor , edict_t * attacker , vec3_t dir , vec3_t point , vec3_t normal , int damage , int knockback , int dflags , int mod );   // found @ line:512
void T_RadiusDamage(edict_t * inflictor , edict_t * attacker , float damage , edict_t * ignore , float radius , int mod );   // found @ line:724

// File : \dday\source/g_ents.c
void trigger_enough_troops_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:25
void SP_trigger_enough_troops(edict_t * ent );   // found @ line:81
void SP_info_Mission_Results(edict_t * ent );   // found @ line:94
void target_objective_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:108
void SP_target_objective(edict_t * ent );   // found @ line:117
void SP_info_reinforcement_start(edict_t * ent );   // found @ line:133
void SP_info_team_start(edict_t * ent );   // found @ line:141
void SP_info_Max_MOS(edict_t * ent );   // found @ line:182
void SP_info_Skin(edict_t * ent );   // found @ line:205
void SP_info_Arty_Battery(edict_t * ent );   // found @ line:218
void SP_info_Air_Battery(edict_t * ent );   // found @ line:244
void SP_event_Arty_Strike(edict_t * ent );   // found @ line:271

// File : \dday\source/g_func.c
void Move_Done(edict_t * ent );   // found @ line:57
void Move_Final(edict_t * ent );   // found @ line:63
void Move_Begin(edict_t * ent );   // found @ line:77
void Move_Calc(edict_t * ent , vec3_t dest , void ( * func ) ( edict_t * ) );   // found @ line:95
void AngleMove_Done(edict_t * ent );   // found @ line:128
void AngleMove_Final(edict_t * ent );   // found @ line:134
void AngleMove_Begin(edict_t * ent );   // found @ line:155
void AngleMove_Calc(edict_t * ent , void ( * func ) ( edict_t * ) );   // found @ line:190
void plat_CalcAcceleratedMove(moveinfo_t * moveinfo );   // found @ line:216
void plat_Accelerate(moveinfo_t * moveinfo );   // found @ line:244
void Think_AccelMove(edict_t * ent );   // found @ line:315
void plat_hit_top(edict_t * ent );   // found @ line:339
void plat_hit_bottom(edict_t * ent );   // found @ line:353
void plat_go_down(edict_t * ent );   // found @ line:364
void plat_go_up(edict_t * ent );   // found @ line:376
void plat_blocked(edict_t * self , edict_t * other );   // found @ line:388
void Use_Plat(edict_t * ent , edict_t * other , edict_t * activator );   // found @ line:409
void Touch_Plat_Center(edict_t * ent , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:417
void plat_spawn_inside_trigger(edict_t * ent );   // found @ line:432
void SP_func_plat(edict_t * ent );   // found @ line:494
void rotating_blocked(edict_t * self , edict_t * other );   // found @ line:576
void rotating_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:581
void rotating_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:587
void SP_func_rotating(edict_t * ent );   // found @ line:604
void button_done(edict_t * self );   // found @ line:673
void button_return(edict_t * self );   // found @ line:680
void button_wait(edict_t * self );   // found @ line:692
void button_fire(edict_t * self );   // found @ line:707
void button_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:718
void button_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:724
void button_killed(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:736
void SP_func_button(edict_t * ent );   // found @ line:744
void door_use_areaportals(edict_t * self , qboolean open );   // found @ line:833
void door_hit_top(edict_t * self );   // found @ line:851
void door_hit_bottom(edict_t * self );   // found @ line:869
void door_go_down(edict_t * self );   // found @ line:881
void door_go_up(edict_t * self , edict_t * activator );   // found @ line:902
void door_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:930
void Touch_DoorTrigger(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:961
void Think_CalcMoveSpeed(edict_t * self );   // found @ line:979
void Think_SpawnDoorTrigger(edict_t * ent );   // found @ line:1019
void door_blocked(edict_t * self , edict_t * other );   // found @ line:1057
void door_killed(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:1094
void door_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:1106
void SP_func_door(edict_t * ent );   // found @ line:1119
void SP_func_door_rotating(edict_t * ent );   // found @ line:1242
void SP_func_water(edict_t * self );   // found @ line:1359
void train_blocked(edict_t * self , edict_t * other );   // found @ line:1442
void train_wait(edict_t * self );   // found @ line:1463
void train_next(edict_t * self );   // found @ line:1510
void train_resume(edict_t * self );   // found @ line:1566
void func_train_find(edict_t * self );   // found @ line:1581
void train_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:1613
void SP_func_train(edict_t * self );   // found @ line:1634
void trigger_elevator_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:1679
void trigger_elevator_init(edict_t * self );   // found @ line:1706
void SP_trigger_elevator(edict_t * self );   // found @ line:1730
void func_timer_think(edict_t * self );   // found @ line:1751
void func_timer_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:1757
void SP_func_timer(edict_t * self );   // found @ line:1775
void func_conveyor_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:1805
void SP_func_conveyor(edict_t * self );   // found @ line:1822
void door_secret_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:1866
void door_secret_move1(edict_t * self );   // found @ line:1876
void door_secret_move2(edict_t * self );   // found @ line:1882
void door_secret_move3(edict_t * self );   // found @ line:1887
void door_secret_move4(edict_t * self );   // found @ line:1895
void door_secret_move5(edict_t * self );   // found @ line:1900
void door_secret_move6(edict_t * self );   // found @ line:1906
void door_secret_done(edict_t * self );   // found @ line:1911
void door_secret_blocked(edict_t * self , edict_t * other );   // found @ line:1921
void door_secret_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:1940
void SP_func_door_secret(edict_t * ent );   // found @ line:1946
void use_killbox(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:2017
void SP_func_killbox(edict_t * ent );   // found @ line:2022
void reinforcement_think(edict_t * ent );   // found @ line:2032

// File : \dday\source/g_items.c
gitem_t * GetItemByIndex(int index );   // found @ line:40
gitem_t * FindItemByClassname(char * classname );   // found @ line:55
gitem_t * FindItem(char * pickup_name );   // found @ line:78
void DoRespawn(edict_t * ent );   // found @ line:97
void SetRespawn(edict_t * ent , float delay );   // found @ line:124
qboolean Pickup_Powerup(edict_t * ent , edict_t * other );   // found @ line:137
void Drop_General(edict_t * ent , gitem_t * item );   // found @ line:165
qboolean Pickup_Adrenaline(edict_t * ent , edict_t * other );   // found @ line:176
qboolean Pickup_Bandolier(edict_t * ent , edict_t * other );   // found @ line:193
qboolean Pickup_Pack(edict_t * ent , edict_t * other );   // found @ line:234
void Use_Breather(edict_t * ent , gitem_t * item );   // found @ line:316
void Use_Envirosuit(edict_t * ent , gitem_t * item );   // found @ line:331
void Use_Silencer(edict_t * ent , gitem_t * item );   // found @ line:344
qboolean Pickup_Key(edict_t * ent , edict_t * other );   // found @ line:355
qboolean Add_Ammo(edict_t * ent , gitem_t * item , int count );   // found @ line:383
qboolean Pickup_Ammo(edict_t * ent , edict_t * other );   // found @ line:425
void Drop_Ammo(edict_t * ent , gitem_t * item );   // found @ line:457
void MegaHealth_think(edict_t * self );   // found @ line:476
qboolean Pickup_Health(edict_t * ent , edict_t * other );   // found @ line:491
int ArmorIndex(edict_t * ent );   // found @ line:549
qboolean Pickup_Armor(edict_t * ent , edict_t * other );   // found @ line:566
int PowerArmorType(edict_t * ent );   // found @ line:623
void Use_PowerArmor(edict_t * ent , gitem_t * item );   // found @ line:629
qboolean Pickup_PowerArmor(edict_t * ent , edict_t * other );   // found @ line:651
void Drop_PowerArmor(edict_t * ent , gitem_t * item );   // found @ line:671
void Touch_Item(edict_t * ent , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:685
// static void drop_temp_touch(edict_t * ent , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:735
// static void drop_make_touchable(edict_t * ent );   // found @ line:743
edict_t * Drop_Item(edict_t * ent , gitem_t * item );   // found @ line:753
void Use_Item(edict_t * ent , edict_t * other , edict_t * activator );   // found @ line:802
void droptofloor(edict_t * ent );   // found @ line:828
void PrecacheItem(gitem_t * it );   // found @ line:903
void SpawnItem(edict_t * ent , gitem_t * item );   // found @ line:971
void SP_item_health(edict_t * self );   // found @ line:1604
void SP_item_health_small(edict_t * self );   // found @ line:1620
void SP_item_health_large(edict_t * self );   // found @ line:1637
void SP_item_health_mega(edict_t * self );   // found @ line:1653
void InitItems(void );   // found @ line:1669
void SetItemNames(void );   // found @ line:1690
void SP_item_armor_body(edict_t * self );   // found @ line:1724
void SP_item_armor_combat(edict_t * self );   // found @ line:1729
void SP_item_armor_jacket(edict_t * self );   // found @ line:1734
void SP_item_armor_shard(edict_t * self );   // found @ line:1739
void SP_item_ammo_grenades(edict_t * self );   // found @ line:1750
void SP_item_weapon_mine(edict_t * self );   // found @ line:1755
void SP_item_powerup_silencer(edict_t * self );   // found @ line:1761
void SP_item_powerup_breather(edict_t * self );   // found @ line:1766
void SP_item_powerup_enviro(edict_t * self );   // found @ line:1771
void SP_item_powerup_adrenaline(edict_t * self );   // found @ line:1776
void SP_item_powerup_bandolier(edict_t * self );   // found @ line:1781
void SP_item_powerup_pack(edict_t * self );   // found @ line:1786
void SP_item_key_blue_key(edict_t * self );   // found @ line:1791
void SP_item_key_red_key(edict_t * self );   // found @ line:1796

// File : \dday\source/g_main.c
void stuffcmd(edict_t * ent , char * s );   // found @ line:84
void ShutdownGame(void );   // found @ line:92
game_export_t * GetGameAPI(game_import_t * import );   // found @ line:112
void Sys_Error(char * error , ... );   // found @ line:144
void Com_Printf(char * msg , ... );   // found @ line:156
void ClientEndServerFrames(void );   // found @ line:178
void EndDMLevel(void );   // found @ line:202
void CheckDMRules(void );   // found @ line:255
void ExitLevel(void );   // found @ line:323
void G_RunFrame(void );   // found @ line:359

// File : \dday\source/g_misc.c
void Use_Areaportal(edict_t * ent , edict_t * other , edict_t * activator );   // found @ line:12
void SP_func_areaportal(edict_t * ent );   // found @ line:25
void VelocityForDamage(int damage , vec3_t v );   // found @ line:39
void ClipGibVelocity(edict_t * ent );   // found @ line:51
void gib_think(edict_t * self );   // found @ line:73
void gib_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:85
void gib_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:111
void ThrowGib(edict_t * self , char * gibname , int damage , int type );   // found @ line:116
void ThrowHead(edict_t * self , char * gibname , int damage , int type );   // found @ line:164
void ThrowClientHead(edict_t * self , int damage );   // found @ line:210
void debris_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:257
void ThrowDebris(edict_t * self , char * modelname , float speed , vec3_t origin );   // found @ line:262
void BecomeExplosion1(edict_t * self );   // found @ line:290
void BecomeExplosion2(edict_t * self );   // found @ line:301
void path_corner_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:318
void SP_path_corner(edict_t * self );   // found @ line:374
void point_combat_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:397
void SP_point_combat(edict_t * self );   // found @ line:449
void TH_viewthing(edict_t * ent );   // found @ line:468
void SP_viewthing(edict_t * ent );   // found @ line:474
void SP_info_null(edict_t * self );   // found @ line:494
void SP_info_notnull(edict_t * self );   // found @ line:503
// static void light_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:520
void SP_light(edict_t * self );   // found @ line:534
void func_wall_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:568
void SP_func_wall(edict_t * self );   // found @ line:587
void func_object_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:640
void func_object_release(edict_t * self );   // found @ line:652
void func_object_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:658
void SP_func_object(edict_t * self );   // found @ line:667
void func_explosive_explode(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:720
void func_explosive_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:784
void func_explosive_spawn(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:789
void SP_func_explosive(edict_t * self );   // found @ line:798
void barrel_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:849
void barrel_explode(edict_t * self );   // found @ line:862
void barrel_delay(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:941
void SP_misc_explobox(edict_t * self );   // found @ line:949
void misc_blackhole_use(edict_t * ent , edict_t * other , edict_t * activator );   // found @ line:1001
void misc_blackhole_think(edict_t * self );   // found @ line:1012
void SP_misc_blackhole(edict_t * ent );   // found @ line:1023
void misc_eastertank_think(edict_t * self );   // found @ line:1040
void SP_misc_eastertank(edict_t * ent );   // found @ line:1051
void misc_easterchick_think(edict_t * self );   // found @ line:1068
void SP_misc_easterchick(edict_t * ent );   // found @ line:1079
void misc_easterchick2_think(edict_t * self );   // found @ line:1096
void SP_misc_easterchick2(edict_t * ent );   // found @ line:1107
void commander_body_think(edict_t * self );   // found @ line:1126
void commander_body_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:1137
void commander_body_drop(edict_t * self );   // found @ line:1144
void SP_monster_commander_body(edict_t * self );   // found @ line:1150
void misc_banner_think(edict_t * ent );   // found @ line:1176
void SP_misc_banner(edict_t * ent );   // found @ line:1182
//bcass start - banner thing
void misc_banner_x_think(edict_t * ent );   // found @ line:1176
void SP_misc_banner_x(edict_t * ent );   // found @ line:1182
void SP_misc_banner_1(edict_t * ent );   // found @ line:1182
void SP_misc_banner_2(edict_t * ent );   // found @ line:1182
void SP_misc_banner_3(edict_t * ent );   // found @ line:1182
void SP_misc_banner_4(edict_t * ent );   // found @ line:1182
//bcass end
void misc_deadsoldier_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:1197
void SP_misc_deadsoldier(edict_t * ent );   // found @ line:1212
void misc_viper_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:1266
void SP_misc_viper(edict_t * ent );   // found @ line:1273
void SP_misc_bigviper(edict_t * ent );   // found @ line:1304
void misc_viper_bomb_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:1318
void misc_viper_bomb_prethink(edict_t * self );   // found @ line:1327
void misc_viper_bomb_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:1346
void SP_misc_viper_bomb(edict_t * self );   // found @ line:1366
void misc_strogg_ship_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:1396
void SP_misc_strogg_ship(edict_t * ent );   // found @ line:1403
void misc_satellite_dish_think(edict_t * self );   // found @ line:1433
void misc_satellite_dish_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:1440
void SP_misc_satellite_dish(edict_t * ent );   // found @ line:1447
void SP_light_mine1(edict_t * ent );   // found @ line:1461
void SP_light_mine2(edict_t * ent );   // found @ line:1472
void SP_misc_gib_arm(edict_t * ent );   // found @ line:1484
void SP_misc_gib_leg(edict_t * ent );   // found @ line:1505
void SP_misc_gib_head(edict_t * ent );   // found @ line:1526
void SP_target_character(edict_t * self );   // found @ line:1551
void target_string_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:1565
void SP_target_string(edict_t * self );   // found @ line:1595
// static void func_clock_reset(edict_t * self );   // found @ line:1621
// static void func_clock_format_countdown(edict_t * self );   // found @ line:1636
void func_clock_think(edict_t * self );   // found @ line:1663
void func_clock_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:1728
void SP_func_clock(edict_t * self );   // found @ line:1738
void teleporter_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:1771
void SP_misc_teleporter(edict_t * ent );   // found @ line:1818
void SP_misc_teleporter_dest(edict_t * ent );   // found @ line:1854

// File : \dday\source/g_monster.c
// static void M_FliesOff(edict_t * self );   // found @ line:17
// static void M_FliesOn(edict_t * self );   // found @ line:23
void M_FlyCheck(edict_t * self );   // found @ line:33
void AttackFinished(edict_t * self , float time );   // found @ line:45
void M_CheckGround(edict_t * ent );   // found @ line:51
void M_CatagorizePosition(edict_t * ent );   // found @ line:93
void M_WorldEffects(edict_t * ent );   // found @ line:128
void M_droptofloor(edict_t * ent );   // found @ line:220
void M_SetEffects(edict_t * ent );   // found @ line:242
void M_MoveFrame(edict_t * self );   // found @ line:271
void monster_think(edict_t * self );   // found @ line:329
void monster_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:350
void monster_triggered_spawn(edict_t * self );   // found @ line:370
void monster_triggered_spawn_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:393
void monster_triggered_start(edict_t * self );   // found @ line:403
void monster_death_use(edict_t * self );   // found @ line:421
qboolean monster_start(edict_t * self );   // found @ line:444
void monster_start_go(edict_t * self );   // found @ line:504
void walkmonster_start_go(edict_t * self );   // found @ line:592
void walkmonster_start(edict_t * self );   // found @ line:613
void flymonster_start_go(edict_t * self );   // found @ line:620
void flymonster_start(edict_t * self );   // found @ line:636
void swimmonster_start_go(edict_t * self );   // found @ line:644
void swimmonster_start(edict_t * self );   // found @ line:656

// File : \dday\source/g_phys.c
edict_t * SV_TestEntityPosition(edict_t * ent );   // found @ line:30
void SV_CheckVelocity(edict_t * ent );   // found @ line:53
qboolean SV_RunThink(edict_t * ent );   // found @ line:76
void SV_Impact(edict_t * e1 , trace_t * trace );   // found @ line:101
int ClipVelocity(vec3_t in , vec3_t normal , vec3_t out , float overbounce );   // found @ line:126
int SV_FlyMove(edict_t * ent , float time , int mask );   // found @ line:164
void SV_AddGravity(edict_t * ent );   // found @ line:302
trace_t SV_PushEntity(edict_t * ent , vec3_t push );   // found @ line:322
qboolean SV_Push(edict_t * pusher , vec3_t move , vec3_t amove );   // found @ line:383
void SV_Physics_Pusher(edict_t * ent );   // found @ line:542
void SV_Physics_None(edict_t * ent );   // found @ line:610
void SV_Physics_Noclip(edict_t * ent );   // found @ line:623
void SV_Physics_Toss(edict_t * ent );   // found @ line:650
void SV_AddRotationalFriction(edict_t * ent );   // found @ line:771
void SV_Physics_Step(edict_t * ent );   // found @ line:795
void G_RunEntity(edict_t * ent );   // found @ line:910

// File : \dday\source/g_save.c
void InitGame(void );   // found @ line:160
void WriteField1(FILE * f , field_t * field , byte * base );   // found @ line:232
void WriteField2(FILE * f , field_t * field , byte * base );   // found @ line:283
void ReadField(FILE * f , field_t * field , byte * base );   // found @ line:302
void WriteClient(FILE * f , gclient_t * client );   // found @ line:374
void ReadClient(FILE * f , gclient_t * client );   // found @ line:405
void WriteGame(char * filename , qboolean autosave );   // found @ line:431
void ReadGame(char * filename );   // found @ line:458
void WriteEdict(FILE * f , edict_t * ent );   // found @ line:498
void WriteLevelLocals(FILE * f );   // found @ line:530
void ReadEdict(FILE * f , edict_t * ent );   // found @ line:562
void ReadLevelLocals(FILE * f );   // found @ line:581
void WriteLevel(char * filename );   // found @ line:599
void ReadLevel(char * filename );   // found @ line:653

// File : \dday\source/g_spawn.c
void SP_item_weapon_flamethrower(edict_t * self );   // found @ line:23
void SP_item_ammo_napalm(edict_t * self );   // found @ line:29
void ED_CallSpawn(edict_t * ent );   // found @ line:328
char * ED_NewString(char * string );   // found @ line:355
void ED_ParseField(char * key , char * value , edict_t * ent );   // found @ line:394
char * ED_ParseEdict(char * data , edict_t * ent );   // found @ line:450
void G_FindTeams(void );   // found @ line:506
void SpawnEntities(char * mapname , char * entities , char * spawnpoint );   // found @ line:556
void SP_worldspawn(edict_t * ent );   // found @ line:807

// File : \dday\source/g_svcmds.c
void Svcmd_Test_f(void );   // found @ line:5
void ServerCommand(void );   // found @ line:23

// File : \dday\source/g_target.c
void Use_Target_Tent(edict_t * ent , edict_t * other , edict_t * activator );   // found @ line:7
void SP_target_temp_entity(edict_t * ent );   // found @ line:15
void Use_Target_Speaker(edict_t * ent , edict_t * other , edict_t * activator );   // found @ line:39
void SP_target_speaker(edict_t * ent );   // found @ line:62
void Use_Target_Help(edict_t * ent , edict_t * other , edict_t * activator );   // found @ line:99
void SP_target_help(edict_t * ent );   // found @ line:112
void use_target_secret(edict_t * ent , edict_t * other , edict_t * activator );   // found @ line:135
void SP_target_secret(edict_t * ent );   // found @ line:145
void use_target_goal(edict_t * ent , edict_t * other , edict_t * activator );   // found @ line:170
void SP_target_goal(edict_t * ent );   // found @ line:183
void target_explosion_explode(edict_t * self );   // found @ line:208
void use_target_explosion(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:225
void SP_target_explosion(edict_t * ent );   // found @ line:239
void use_target_changelevel(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:251
void SP_target_changelevel(edict_t * ent );   // found @ line:283
void use_target_splash(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:319
void SP_target_splash(edict_t * self );   // found @ line:333
void use_target_spawner(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:361
void SP_target_spawner(edict_t * self );   // found @ line:377
void use_target_blaster(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:397
void SP_target_blaster(edict_t * self );   // found @ line:412
void trigger_crosslevel_trigger_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:432
void SP_target_crosslevel_trigger(edict_t * self );   // found @ line:438
void target_crosslevel_target_think(edict_t * self );   // found @ line:450
void SP_target_crosslevel_target(edict_t * self );   // found @ line:459
void target_laser_think(edict_t * self );   // found @ line:476
void target_laser_on(edict_t * self );   // found @ line:541
void target_laser_off(edict_t * self );   // found @ line:550
void target_laser_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:557
void target_laser_start(edict_t * self );   // found @ line:566
void SP_target_laser(edict_t * self );   // found @ line:623
void target_lightramp_think(edict_t * self );   // found @ line:637
void target_lightramp_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:660
void SP_target_lightramp(edict_t * self );   // found @ line:696
void target_earthquake_think(edict_t * self );   // found @ line:736
void target_earthquake_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:766
void SP_target_earthquake(edict_t * self );   // found @ line:774

// File : \dday\source/g_trigger.c
void InitTrigger(edict_t * self );   // found @ line:4
void multi_wait(edict_t * ent );   // found @ line:17
void multi_trigger(edict_t * ent );   // found @ line:26
void Use_Multi(edict_t * ent , edict_t * other , edict_t * activator );   // found @ line:47
void Touch_Multi(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:53
void trigger_enable(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:92
void SP_trigger_multiple(edict_t * ent );   // found @ line:99
void SP_trigger_once(edict_t * ent );   // found @ line:149
void trigger_relay_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:170
void SP_trigger_relay(edict_t * self );   // found @ line:175
void trigger_key_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:193
void SP_trigger_key(edict_t * self );   // found @ line:263
void trigger_counter_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:307
void SP_trigger_counter(edict_t * self );   // found @ line:333
void SP_trigger_always(edict_t * ent );   // found @ line:354
void trigger_push_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:375
void SP_trigger_push(edict_t * self );   // found @ line:405
void hurt_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:436
void hurt_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:449
void SP_trigger_hurt(edict_t * self );   // found @ line:477
void trigger_gravity_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:513
void SP_trigger_gravity(edict_t * self );   // found @ line:518
void trigger_monsterjump_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:547
void SP_trigger_monsterjump(edict_t * self );   // found @ line:567

// File : \dday\source/g_turret.c
void AnglesNormalize(vec3_t vec );   // found @ line:6
float SnapToEights(float x );   // found @ line:18
void turret_blocked(edict_t * self , edict_t * other );   // found @ line:29
void turret_breach_fire(edict_t * self );   // found @ line:59
void turret_breach_think(edict_t * self );   // found @ line:77
void turret_breach_finish_init(edict_t * self );   // found @ line:182
void SP_turret_breach(edict_t * self );   // found @ line:201
void SP_turret_base(edict_t * self );   // found @ line:240
void turret_driver_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:259
void turret_driver_think(edict_t * self );   // found @ line:281
void turret_driver_link(edict_t * self );   // found @ line:335

// File : \dday\source/g_utils.c
void G_ProjectSource(vec3_t point , vec3_t distance , vec3_t forward , vec3_t right , vec3_t result );   // found @ line:8
edict_t * G_Find(edict_t * from , int fieldofs , char * match );   // found @ line:28
edict_t * findradius(edict_t * from , vec3_t org , float rad );   // found @ line:61
edict_t * G_PickTarget(char * targetname );   // found @ line:101
void Think_Delay(edict_t * ent );   // found @ line:134
void G_UseTargets(edict_t * ent , edict_t * activator );   // found @ line:156
float * tv(float x , float y , float z );   // found @ line:249
char * vtos(vec3_t v );   // found @ line:276
void G_SetMovedir(vec3_t angles , vec3_t movedir );   // found @ line:297
float vectoyaw(vec3_t vec );   // found @ line:316
void vectoangles(vec3_t value1 , vec3_t angles );   // found @ line:333
char * G_CopyString(char * in );   // found @ line:363
void G_InitEdict(edict_t * e );   // found @ line:373
edict_t * G_Spawn(void );   // found @ line:392
void G_FreeEdict(edict_t * ed );   // found @ line:424
void G_TouchTriggers(edict_t * ent );   // found @ line:447
void G_TouchSolids(edict_t * ent );   // found @ line:480
qboolean KillBox(edict_t * ent );   // found @ line:521

// File : \dday\source/g_weapon.c
// static void check_dodge(edict_t * self , vec3_t start , vec3_t dir , int speed );   // found @ line:15
qboolean fire_hit(edict_t * self , vec3_t aim , int damage , int kick );   // found @ line:46
int calcVspread(edict_t * ent , trace_t * tr );   // found @ line:110
// static void fire_lead(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int kick , int te_impact , int hspread , int vspread , int mod , qboolean calcv );   // found @ line:128
void fire_fragment(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int kick , int hspread , int vspread , int mod );   // found @ line:272
void fire_bullet(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int kick , int hspread , int vspread , int mod , qboolean tracers_on );   // found @ line:288
void tracer_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:310
void fire_tracer(edict_t * self , vec3_t start , vec3_t dir , int damage , int mod);   // found @ line:346
void blaster_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:394
void fire_blaster(edict_t * self , vec3_t start , vec3_t dir , int damage , int speed , int effect , qboolean hyper );   // found @ line:433
void Shrapnel_Explode(edict_t * ent );   // found @ line:486
//bcass start - TNT
void TNT_Explode(edict_t * ent );
//bcass end
// static void Shrapnel_Touch(edict_t * ent , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:559
//void fire_grenade(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int speed , float timer , float damage_radius );   // found @ line:605
void fire_grenade2(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int speed , float time , float damage_radius , int team );   // found @ line:639
void rocket_touch(edict_t * ent , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:693
void fire_rocket(edict_t * self , vec3_t start , vec3_t dir , int damage , int speed , float damage_radius , int radius_damage );   // found @ line:744
void fire_shell(edict_t * self , vec3_t start , vec3_t dir , int damage , int speed , float damage_radius , int radius_damage );   // found @ line:776
void fire_rifle(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int kick, int mod );   // found @ line:812
edict_t * ApplyFirstAid(edict_t * ent );   // found @ line:872
int DoAnarchyStuff(edict_t * ent );   // found @ line:923
void Weapon_Pistol_Fire(edict_t * ent );   // found @ line:982
void Weapon_Rifle_Fire(edict_t * ent );   // found @ line:1067
void Weapon_Submachinegun_Fire(edict_t * ent );   // found @ line:1127
void Weapon_LMG_Fire(edict_t * ent );   // found @ line:1213
void Weapon_HMG_Fire(edict_t * ent );   // found @ line:1294
void Weapon_Rocket_Fire(edict_t * ent );   // found @ line:1421
void Weapon_Sniper_Fire(edict_t * ent );   // found @ line:1465

// File : \dday\source/m_actor.c
void actor_stand(edict_t * self );   // found @ line:68
void actor_walk(edict_t * self );   // found @ line:94
void actor_run(edict_t * self );   // found @ line:117
void actor_pain(edict_t * self , edict_t * other , float kick , int damage );   // found @ line:211
void actorMachineGun(edict_t * self );   // found @ line:250
void actor_dead(edict_t * self );   // found @ line:280
void actor_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:320
void actor_fire(edict_t * self );   // found @ line:353
void actor_attack(edict_t * self );   // found @ line:372
void actor_use(edict_t * self , edict_t * other , edict_t * activator );   // found @ line:382
void SP_misc_actor(edict_t * self );   // found @ line:406
void target_actor_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:477
void SP_target_actor(edict_t * self );   // found @ line:566

// File : \dday\source/p_client.c
// static void SP_FixCoopSpots(edict_t * self );   // found @ line:28
// static void SP_CreateCoopSpots(edict_t * self );   // found @ line:59
void SP_info_player_start(edict_t * self );   // found @ line:97
void SP_info_player_deathmatch(edict_t * self );   // found @ line:112
void SP_info_player_coop(edict_t * self );   // found @ line:126
void SP_info_player_intermission(void );   // found @ line:160
void SP_info_Infantry_Start(edict_t * ent );   // found @ line:163
void SP_info_L_Gunner_Start(edict_t * ent );   // found @ line:164
void SP_info_H_Gunner_Start(edict_t * ent );   // found @ line:165
void SP_info_Sniper_Start(edict_t * ent );   // found @ line:166
void SP_info_Engineer_Start(edict_t * ent );   // found @ line:167
void SP_info_Medic_Start(edict_t * ent );   // found @ line:168
void SP_info_Flamethrower_Start(edict_t * ent );   // found @ line:169
void SP_info_Special_Start(edict_t * ent );   // found @ line:170
void SP_info_Officer_Start(edict_t * ent);
void player_pain(edict_t * self , edict_t * other , float kick , int damage );   // found @ line:177
qboolean IsFemale(edict_t * ent );   // found @ line:183
void ClientObituary(edict_t * self , edict_t * inflictor , edict_t * attacker );   // found @ line:197
void TossClientWeapon(edict_t * self );   // found @ line:409
void LookAtKiller(edict_t * self , edict_t * inflictor , edict_t * attacker );   // found @ line:462
void player_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:488
void InitClientPersistant(gclient_t * client );   // found @ line:590
void InitClientResp(gclient_t * client );   // found @ line:617
void SaveClientData(void );   // found @ line:634
void FetchClientEntData(edict_t * ent );   // found @ line:652
float PlayersRangeFromSpot(edict_t * spot );   // found @ line:679
edict_t * SelectRandomDeathmatchSpawnPoint(void );   // found @ line:718
edict_t * SelectFarthestDeathmatchSpawnPoint(void );   // found @ line:774
edict_t * SelectDeathmatchSpawnPoint(void );   // found @ line:807
edict_t * SelectCoopSpawnPoint(edict_t * ent );   // found @ line:816
void SelectSpawnPoint(edict_t * ent , vec3_t origin , vec3_t angles );   // found @ line:860
void Find_Mission_Start_Point(edict_t * ent , vec3_t origin , vec3_t angles );   // found @ line:908
void InitBodyQue(void );   // found @ line:932
void body_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:945
void CopyToBodyQue(edict_t * ent );   // found @ line:960
void respawn(edict_t * self );   // found @ line:1021
void PutClientInServer(edict_t * ent );   // found @ line:1076
void ClientBeginDeathmatch(edict_t * ent );   // found @ line:1239
void ClientBegin(edict_t * ent );   // found @ line:1271
void ClientUserinfoChanged(edict_t * ent , char * userinfo );   // found @ line:1340
qboolean ClientConnect(edict_t * ent , char * userinfo );   // found @ line:1398
void ClientDisconnect(edict_t * ent );   // found @ line:1450
trace_t PM_trace(vec3_t start , vec3_t mins , vec3_t maxs , vec3_t end );   // found @ line:1483
void PrintPmove(pmove_t * pm );   // found @ line:1499
void ClientThink(edict_t * ent , usercmd_t * ucmd );   // found @ line:1518
void ClientBeginServerFrame(edict_t * ent );   // found @ line:1769

// File : \dday\source/m_flash.c

// File : \dday\source/m_insane.c
void insane_fist(edict_t * self );   // found @ line:18
void insane_shake(edict_t * self );   // found @ line:23
void insane_moan(edict_t * self );   // found @ line:28
void insane_scream(edict_t * self );   // found @ line:33
void insane_cross(edict_t * self );   // found @ line:415
void insane_walk(edict_t * self );   // found @ line:423
void insane_run(edict_t * self );   // found @ line:440
void insane_pain(edict_t * self , edict_t * other , float kick , int damage );   // found @ line:458
void insane_onground(edict_t * self );   // found @ line:500
void insane_checkdown(edict_t * self );   // found @ line:505
void insane_checkup(edict_t * self );   // found @ line:517
void insane_stand(edict_t * self );   // found @ line:527
void insane_dead(edict_t * self );   // found @ line:544
void insane_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:562
void SP_misc_insane(edict_t * self );   // found @ line:602

// File : \dday\source/m_move.c
qboolean M_CheckBottom(edict_t * ent );   // found @ line:18
qboolean SV_movestep(edict_t * ent , vec3_t move , qboolean relink );   // found @ line:93
void M_ChangeYaw(edict_t * ent );   // found @ line:285
qboolean SV_StepDirection(edict_t * ent , float yaw , float dist );   // found @ line:334
void SV_FixCheckBottom(edict_t * ent );   // found @ line:370
void SV_NewChaseDir(edict_t * actor , edict_t * enemy , float dist );   // found @ line:384
qboolean SV_CloseEnough(edict_t * ent , edict_t * goal , float dist );   // found @ line:476
void M_MoveToGoal(edict_t * ent , float dist );   // found @ line:496
qboolean M_walkmove(edict_t * ent , float yaw , float dist );   // found @ line:523

// File : \dday\source/observer.c
void SwitchToObserver(edict_t * ent );   // found @ line:10
void EndObserverMode(edict_t * ent );   // found @ line:29
//void DoEndOM(edict_t * ent);   // found @ line:78
//void M_MOS_Join(edict_t * ent , int choice );   // found @ line:126
//void SMOS_Join(edict_t * ent , int choice );   // found @ line:135
void M_ChooseMOS(edict_t * ent , qboolean smos );   // found @ line:145
//void M_Team_Join(edict_t * ent , int choice );   // found @ line:179
void ChooseTeam(edict_t * ent );   // found @ line:212

//
void M_Team_Join	(edict_t *ent, pmenu_t *p, int choice);
void M_MOS_Join		(edict_t *ent, pmenu_t *p, int choice);
void M_Main_Menu	(edict_t *ent, pmenu_t *p, int choice);
void M_Team_Choose	(edict_t *ent, pmenu_t *p, int choice);
void M_View_Credits	(edict_t *ent, pmenu_t *p, int choice);
void MainMenu(edict_t *ent);

// File : \dday\source/p_classes.c
void Give_Class_Weapon(edict_t * ent );   // found @ line:8
void Give_Class_Ammo(edict_t * ent );   // found @ line:43
void Show_Mos(edict_t * ent );   // found @ line:62
void InitMOS_List(TeamS_t * team , SMos_t * mos_list  );   // found @ line:79

// File : \dday\source/p_generic_wep.c
void Weapon_Generic(edict_t * ent , int FRAME_ACTIVATE_LAST , int FRAME_LFIRE_LAST , int FRAME_LIDLE_LAST ,  int FRAME_RELOAD_LAST , int FRAME_LASTRD_LAST , int FRAME_DEACTIVATE_LAST ,  int FRAME_RAISE_LAST , int FRAME_AFIRE_LAST , int FRAME_AIDLE_LAST ,  int * pause_frames , int * fire_frames , void ( * fire ) ( edict_t * ent ) );   // found @ line:29
void ifchangewep(edict_t * ent );   // found @ line:305

// File : \dday\source/p_hud.c
void MoveClientToIntermission(edict_t * ent );   // found @ line:13
void BeginIntermission(edict_t * targ );   // found @ line:57
//void DeathmatchPlayerScore(edict_t * ent , edict_t * killer );   // found @ line:147
//void DeathmatchScoreboardMessage(edict_t * ent , edict_t * killer );   // found @ line:244
void DDayScoreboardMessage(edict_t * ent);
void DeathmatchScoreboard(edict_t * ent );   // found @ line:316
void Cmd_Score_f(edict_t * ent );   // found @ line:330
void HelpComputer(edict_t * ent );   // found @ line:366
void Cmd_Help_f(edict_t * ent );   // found @ line:410
void G_SetStats(edict_t * ent );   // found @ line:441

// File : \dday\source/p_trail.c
void PlayerTrail_Init(void );   // found @ line:30
void PlayerTrail_Add(vec3_t spot );   // found @ line:48
void PlayerTrail_New(vec3_t spot );   // found @ line:66
edict_t * PlayerTrail_PickFirst(edict_t * self );   // found @ line:76
edict_t * PlayerTrail_PickNext(edict_t * self );   // found @ line:105
edict_t * PlayerTrail_LastSpot(void );   // found @ line:124

// File : \dday\source/p_view.c
float SV_CalcRoll(vec3_t angles , vec3_t velocity );   // found @ line:24
void P_DamageFeedback(edict_t * player );   // found @ line:53
void SV_CalcViewOffset(edict_t * ent );   // found @ line:218
void SV_CalcGunOffset(edict_t * ent );   // found @ line:352
void SV_AddBlend(float r , float g , float b , float a , float * v_blend );   // found @ line:405
void SV_CalcBlend(edict_t * ent );   // found @ line:426
void P_FallingDamage(edict_t * ent );   // found @ line:509
void P_WorldEffects(void );   // found @ line:587
void G_SetClientEffects(edict_t * ent );   // found @ line:757
void G_SetClientEvent(edict_t * ent );   // found @ line:810
void G_SetClientSound(edict_t * ent );   // found @ line:827
void G_SetClientFrame(edict_t * ent );   // found @ line:864
void ClientEndServerFrame(edict_t * ent );   // found @ line:959

// File : \dday\source/p_weapon.c
void P_ProjectSource(gclient_t * client , vec3_t point , vec3_t distance , vec3_t forward , vec3_t right , vec3_t result );   // found @ line:17
void PlayerNoise(edict_t * who , vec3_t where , int type );   // found @ line:42
qboolean Pickup_Weapon(edict_t * ent , edict_t * other );   // found @ line:102
void ChangeWeapon(edict_t * ent );   // found @ line:162
void NoAmmoWeaponChange(edict_t * ent );   // found @ line:201
void Think_Weapon(edict_t * ent );   // found @ line:244
void Use_Weapon(edict_t * ent , gitem_t * item );   // found @ line:273
void Drop_Weapon(edict_t * ent , gitem_t * item );   // found @ line:308
void weapon_grenade_fire (edict_t *ent);
void Weapon_Grenade(edict_t * ent );   // found @ line:374
void weapon_grenadelauncher_fire(edict_t * ent );   // found @ line:504
void Weapon_Mine(edict_t * ent );   // found @ line:535
void fire_Knife(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int kick );   // found @ line:550
void Blade_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:596
void Knife_Throw(edict_t * self , vec3_t start , vec3_t dir , int damage );   // found @ line:641
void Weapon_Knife_Fire(edict_t * ent );   // found @ line:682
void Weapon_Knife(edict_t * ent );   // found @ line:725
void Weapon_Binoculars_Look(edict_t * ent );   // found @ line:737
void Weapon_Binoculars(edict_t * ent );   // found @ line:753
void Weapon_Antidote_Use(edict_t * ent );   // found @ line:770
void Weapon_Antidote(edict_t * ent );   // found @ line:777
void Weapon_Morphine_Use(edict_t * ent );   // found @ line:790
void Weapon_Morphine(edict_t * ent );   // found @ line:817
void Weapon_Bandage_Use(edict_t * ent );   // found @ line:827
void Weapon_Bandage(edict_t * ent );   // found @ line:855
void weapon_flame_fire(edict_t * ent );   // found @ line:874
void Weapon_Flamethrower(edict_t * ent );   // found @ line:944

// File : \dday\source/q_shared.c
void RotatePointAroundVector(vec3_t dst , const vec3_t dir , const vec3_t point , float degrees );   // found @ line:13
void AngleVectors(vec3_t angles , vec3_t forward , vec3_t right , vec3_t up );   // found @ line:74
void ProjectPointOnPlane(vec3_t dst , const vec3_t p , const vec3_t normal );   // found @ line:111
void PerpendicularVector(vec3_t dst , const vec3_t src );   // found @ line:133
void R_ConcatRotations(float in1 [ 3 ] [ 3 ] , float in2 [ 3 ] [ 3 ] , float out [ 3 ] [ 3 ] );   // found @ line:172
void R_ConcatTransforms(float in1 [ 3 ] [ 4 ] , float in2 [ 3 ] [ 4 ] , float out [ 3 ] [ 4 ] );   // found @ line:200
float Q_fabs(float f );   // found @ line:232
float LerpAngle(float a2 , float a1 , float frac );   // found @ line:264
float anglemod(float a );   // found @ line:274
int BoxOnPlaneSide2(vec3_t emins , vec3_t emaxs , struct cplane_s * p );   // found @ line:291
int BoxOnPlaneSide(vec3_t emins , vec3_t emaxs , struct cplane_s * p );   // found @ line:330
int BoxOnPlaneSide(vec3_t emins , vec3_t emaxs , struct cplane_s * p );   // found @ line:399
void ClearBounds(vec3_t mins , vec3_t maxs );   // found @ line:631
void AddPointToBounds(vec3_t v , vec3_t mins , vec3_t maxs );   // found @ line:637
int VectorCompare(vec3_t v1 , vec3_t v2 );   // found @ line:653
vec_t VectorNormalize(vec3_t v );   // found @ line:662
vec_t VectorNormalize2(vec3_t v , vec3_t out );   // found @ line:681
void VectorMA(vec3_t veca , float scale , vec3_t vecb , vec3_t vecc );   // found @ line:700
vec_t _DotProduct(vec3_t v1 , vec3_t v2 );   // found @ line:708
void _VectorSubtract(vec3_t veca , vec3_t vecb , vec3_t out );   // found @ line:713
void _VectorAdd(vec3_t veca , vec3_t vecb , vec3_t out );   // found @ line:720
void _VectorCopy(vec3_t in , vec3_t out );   // found @ line:727
void CrossProduct(vec3_t v1 , vec3_t v2 , vec3_t cross );   // found @ line:734
vec_t VectorLength(vec3_t v );   // found @ line:743
void VectorInverse(vec3_t v );   // found @ line:756
void VectorScale(vec3_t in , vec_t scale , vec3_t out );   // found @ line:763
int Q_log2(int val );   // found @ line:771
char * COM_SkipPath(char * pathname );   // found @ line:788
void COM_StripExtension(char * in , char * out );   // found @ line:807
char * COM_FileExtension(char * in );   // found @ line:819
void COM_FileBase(char * in , char * out );   // found @ line:840
void COM_FilePath(char * in , char * out );   // found @ line:869
void COM_DefaultExtension(char * path , char * extension );   // found @ line:888
int BigLong(int l );   // found @ line:927
int LittleLong(int l );   // found @ line:928
float BigFloat(float l );   // found @ line:929
float LittleFloat(float l );   // found @ line:930
int LongSwap(int l );   // found @ line:948
int LongNoSwap(int l );   // found @ line:960
float FloatSwap(float f );   // found @ line:965
float FloatNoSwap(float f );   // found @ line:982
void Swap_Init(void );   // found @ line:992
char * va(char * format , ... );   // found @ line:1031
char * COM_Parse(char * * data_p );   // found @ line:1053
void Com_PageInMemory(byte * buffer , int size );   // found @ line:1143
int Q_stricmp(char * s1 , char * s2 );   // found @ line:1162
int Q_strncasecmp(char * s1 , char * s2 , int n );   // found @ line:1172
int Q_strcasecmp(char * s1 , char * s2 );   // found @ line:1198
void Com_sprintf(char * dest , int size , char * fmt , ... );   // found @ line:1205
char * Info_ValueForKey(char * s , char * key );   // found @ line:1235
void Info_RemoveKey(char * s , char * key );   // found @ line:1277
qboolean Info_Validate(char * s );   // found @ line:1335
void Info_SetValueForKey(char * s , char * key , char * value );   // found @ line:1344

// File : \dday\source/removed.c
void SP_monster_tank(edict_t * self );   // found @ line:8
void SP_monster_flipper(edict_t * self );   // found @ line:15
void SP_monster_chick(edict_t * self );   // found @ line:20
void SP_monster_flyer(edict_t * self );   // found @ line:26
void SP_monster_brain(edict_t * self );   // found @ line:31
void SP_monster_floater(edict_t * self );   // found @ line:36
void SP_monster_hover(edict_t * self );   // found @ line:41
void SP_monster_mutant(edict_t * self );   // found @ line:46
void SP_monster_supertank(edict_t * self );   // found @ line:51
void SP_monster_boss2(edict_t * self );   // found @ line:56
void SP_monster_jorg(edict_t * self );   // found @ line:61
void SP_monster_boss3_stand(edict_t * self );   // found @ line:66

/*
// File : \dday\source/Qmenu.c
void Menu_Msg(edict_t * ent , char * message );   // found @ line:65
void Menu_Init(edict_t * ent );   // found @ line:188
void Menu_Clear(edict_t * ent );   // found @ line:213
void Menu_Add(edict_t * ent , char * text );   // found @ line:243
void Menu_Title(edict_t * ent , char * text );   // found @ line:270
void Menu_Open(edict_t * ent );   // found @ line:293
void Menu_Close(edict_t * ent );   // found @ line:369
void Menu_Up(edict_t * ent );   // found @ line:391
void Menu_Dn(edict_t * ent );   // found @ line:417
void Menu_Sel(edict_t * ent );   // found @ line:443
*/

// File : \dday\source/x_fire.c
void check_firedodge(edict_t * self , vec3_t start , vec3_t dir , int speed );   // found @ line:54
void PBM_BecomeSmoke(edict_t * self );   // found @ line:83
void PBM_BecomeSteam(edict_t * self );   // found @ line:106
void PBM_SmallExplodeThink(edict_t * self );   // found @ line:121
void PBM_StartSmallExplosion(edict_t * self );   // found @ line:141
void PBM_BecomeSmallExplosion(edict_t * self );   // found @ line:166
void PBM_KillAllFires(void );   // found @ line:188
qboolean PBM_InWater(edict_t * ent );   // found @ line:214
qboolean PBM_Inflammable(edict_t * ent );   // found @ line:231
qboolean PBM_ActivePowerArmor(edict_t * ent , vec3_t point );   // found @ line:264
qboolean PBM_FireResistant(edict_t * ent , vec3_t point );   // found @ line:304
void PBM_BurnDamage(edict_t * victim , edict_t * fire , vec3_t point , vec3_t damage , int dflags , int mod );   // found @ line:332
void PBM_BurnRadius(edict_t * fire , float radius , vec3_t damage , edict_t * ignore );   // found @ line:360
void PBM_CheckMaster(edict_t * fire );   // found @ line:392
void PBM_FireSpot(vec3_t spot , edict_t * ent );   // found @ line:404
qboolean PBM_FlameOut(edict_t * self );   // found @ line:443
void PBM_Burn(edict_t * self );   // found @ line:503
void PBM_Ignite(edict_t * victim , edict_t * attacker , vec3_t point );   // found @ line:542
void PBM_CheckFire(edict_t * self );   // found @ line:608
void PBM_FireDropTouch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:646
void PBM_FireDrop(edict_t * attacker , vec3_t spot , vec3_t damage , vec3_t radius_damage , int blast_chance );   // found @ line:690
void PBM_EasyFireDrop(edict_t * self );   // found @ line:728
void PBM_CloudBurst(edict_t * self );   // found @ line:739
void PBM_CloudBurstDamage(edict_t * self );   // found @ line:750
void PBM_FlameCloud(edict_t * attacker , vec3_t start , vec3_t cloud , vec3_t timer , qboolean deadly , vec3_t damage , vec3_t radius_damage , int rain_chance , int blast_chance );   // found @ line:770
void PBM_FireAngleSpread(vec3_t spread , vec3_t dir );   // found @ line:855
void PBM_FireballTouch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:873
void PBM_FireFlamer(edict_t * self , vec3_t start , vec3_t spread , vec3_t dir , int speed , vec3_t damage , vec3_t radius_damage , int rain_chance , int blast_chance );   // found @ line:915
void PBM_FlameThrowerTouch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:969
void PBM_FlameThrowerThink(edict_t * self );   // found @ line:1020
void PBM_FireFlameThrower(edict_t * self , vec3_t start , vec3_t spread , vec3_t dir , int speed , vec3_t damage , vec3_t radius_damage , int rain_chance , int blast_chance );   // found @ line:1043

// File : \dday\source/x_fbomb.c
// static void FireGrenade_Explode(edict_t * ent );   // found @ line:20
// static void FireGrenade_Touch(edict_t * ent , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:97
void fire_flamegrenade(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int speed , float timer , float damage_radius );   // found @ line:132
void fire_flamegrenade2(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int speed , float timer , float damage_radius , qboolean held );   // found @ line:169
void firerocket_touch(edict_t * ent , edict_t * other , cplane_t * plane , csurface_t * surf );   // found @ line:220
void fire_flamerocket(edict_t * self , vec3_t start , vec3_t dir , int damage , int speed , float damage_radius , int radius_damage );   // found @ line:291

// File : \dday\source/g_dll.c
struct userdll_list_node * LoadUserDLLs(edict_t * ent, int team );   // found @ line:14
int InitializeUserDLLs(struct userdll_list_node * unode , int teamindex );   // found @ line:86
void ClearUserDLLs();   // found @ line:142
void LevelStartUserDLLs(edict_t * ent );   // found @ line:163
void LevelExitUserDLLs();   // found @ line:175
void PlayerSpawnUserDLLs(edict_t * ent );   // found @ line:188
void PlayerDiesUserDLLs(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point );   // found @ line:200

// File : \dday\source/u_findfunc.c

// File : \dday\source/u_entmgr.c
gitem_t * InsertItem(gitem_t * it , spawn_t * spawnInfo );   // found @ line:34
qboolean booldummy(struct edict_s * i , struct edict_s * ii );   // found @ line:85
void dummy1(struct edict_s * i , struct gitem_s * ii );   // found @ line:90
void dummy2(struct edict_s * i );   // found @ line:94
spawn_t * InsertEntity(spawn_t * spawnInfo );   // found @ line:104
int RemoveEntity(char * name );   // found @ line:132
