/*       D-Day: Normandy by Vipersoft
 ************************************
 *   $Source: /usr/local/cvsroot/dday/src/funcarry.h,v $
 *   $Revision: 1.10 $
 *   $Date: 2002/06/04 19:49:44 $
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

struct FuncArrayStruct GlobalGameFunctionArray[] =
{
"AI_SetSightClient", (void (*))AI_SetSightClient,   // void AI_SetSightClient(void )  \dday\source/g_ai.c:31
"AddPointToBounds", (void (*))AddPointToBounds,   // void AddPointToBounds(vec3_t v , vec3_t mins , vec3_t maxs )  \dday\source/q_shared.c:637
"Add_Ammo", (void (*))Add_Ammo,   // qboolean Add_Ammo(edict_t * ent , gitem_t * item , int count )  \dday\source/g_items.c:383
"AngleMove_Begin", (void (*))AngleMove_Begin,   // void AngleMove_Begin(edict_t * ent )  \dday\source/g_func.c:155
"AngleMove_Calc", (void (*))AngleMove_Calc,   // void AngleMove_Calc(edict_t * ent , void ( * func ) ( edict_t * ) )  \dday\source/g_func.c:190
"AngleMove_Done", (void (*))AngleMove_Done,   // void AngleMove_Done(edict_t * ent )  \dday\source/g_func.c:128
"AngleMove_Final", (void (*))AngleMove_Final,   // void AngleMove_Final(edict_t * ent )  \dday\source/g_func.c:134
"AngleVectors", (void (*))AngleVectors,   // void AngleVectors(vec3_t angles , vec3_t forward , vec3_t right , vec3_t up )  \dday\source/q_shared.c:74
"AnglesNormalize", (void (*))AnglesNormalize,   // void AnglesNormalize(vec3_t vec )  \dday\source/g_turret.c:6
"ApplyFirstAid", (void (*))ApplyFirstAid,   // edict_t * ApplyFirstAid(edict_t * ent )  \dday\source/g_weapon.c:872
"ArmorIndex", (void (*))ArmorIndex,   // int ArmorIndex(edict_t * ent )  \dday\source/g_items.c:549
"AttackFinished", (void (*))AttackFinished,   // void AttackFinished(edict_t * self , float time )  \dday\source/g_monster.c:45
"BecomeExplosion1", (void (*))BecomeExplosion1,   // void BecomeExplosion1(edict_t * self )  \dday\source/g_misc.c:290
"BecomeExplosion2", (void (*))BecomeExplosion2,   // void BecomeExplosion2(edict_t * self )  \dday\source/g_misc.c:301
"BeginIntermission", (void (*))BeginIntermission,   // void BeginIntermission(edict_t * targ )  \dday\source/p_hud.c:57
"BigFloat", (void (*))BigFloat,   // float BigFloat(float l )  \dday\source/q_shared.c:929
"BigLong", (void (*))BigLong,   // int BigLong(int l )  \dday\source/q_shared.c:927
"Blade_touch", (void (*))Blade_touch,   // void Blade_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/p_weapon.c:596
"BoxOnPlaneSide", (void (*))BoxOnPlaneSide,   // int BoxOnPlaneSide(vec3_t emins , vec3_t emaxs , struct cplane_s * p )  \dday\source/q_shared.c:399
"BoxOnPlaneSide", (void (*))BoxOnPlaneSide,   // int BoxOnPlaneSide(vec3_t emins , vec3_t emaxs , struct cplane_s * p )  \dday\source/q_shared.c:330
"BoxOnPlaneSide2", (void (*))BoxOnPlaneSide2,   // int BoxOnPlaneSide2(vec3_t emins , vec3_t emaxs , struct cplane_s * p )  \dday\source/q_shared.c:291
"COM_DefaultExtension", (void (*))COM_DefaultExtension,   // void COM_DefaultExtension(char * path , char * extension )  \dday\source/q_shared.c:888
"COM_FileBase", (void (*))COM_FileBase,   // void COM_FileBase(char * in , char * out )  \dday\source/q_shared.c:840
"COM_FileExtension", (void (*))COM_FileExtension,   // char * COM_FileExtension(char * in )  \dday\source/q_shared.c:819
"COM_FilePath", (void (*))COM_FilePath,   // void COM_FilePath(char * in , char * out )  \dday\source/q_shared.c:869
"COM_Parse", (void (*))COM_Parse,   // char * COM_Parse(char * * data_p )  \dday\source/q_shared.c:1053
"COM_SkipPath", (void (*))COM_SkipPath,   // char * COM_SkipPath(char * pathname )  \dday\source/q_shared.c:788
"COM_StripExtension", (void (*))COM_StripExtension,   // void COM_StripExtension(char * in , char * out )  \dday\source/q_shared.c:807
"CanDamage", (void (*))CanDamage,   // qboolean CanDamage(edict_t * targ , edict_t * inflictor )  \dday\source/g_combat.c:94
"ChangeWeapon", (void (*))ChangeWeapon,   // void ChangeWeapon(edict_t * ent )  \dday\source/p_weapon.c:162
// "CheckArmor", (void (*))CheckArmor,   // static int CheckArmor(edict_t * ent , vec3_t point , vec3_t normal , int damage , int te_sparks , int dflags )  \dday\source/g_combat.c:341
"CheckDMRules", (void (*))CheckDMRules,   // void CheckDMRules(void )  \dday\source/g_main.c:255
// "CheckPowerArmor", (void (*))CheckPowerArmor,   // static int CheckPowerArmor(edict_t * ent , vec3_t point , vec3_t normal , int damage , int dflags )  \dday\source/g_combat.c:257
"CheckTeamDamage", (void (*))CheckTeamDamage,   // qboolean CheckTeamDamage(edict_t * targ , edict_t * attacker )  \dday\source/g_combat.c:447
"M_ChooseMOS", (void (*))M_ChooseMOS,   // void ChooseMOS(edict_t * ent , qboolean smos )  \dday\source/observer.c:145
"ChooseTeam", (void (*))ChooseTeam,   // void ChooseTeam(edict_t * ent )  \dday\source/observer.c:212
"CleanUpCmds", (void (*))CleanUpCmds,   // void CleanUpCmds()  \dday\source/g_cmds.c:1441
"ClearBounds", (void (*))ClearBounds,   // void ClearBounds(vec3_t mins , vec3_t maxs )  \dday\source/q_shared.c:631
"ClearUserDLLs", (void (*))ClearUserDLLs,   // void ClearUserDLLs()  \dday\source/g_dll.c:142
"ClientBegin", (void (*))ClientBegin,   // void ClientBegin(edict_t * ent )  \dday\source/p_client.c:1271
"ClientBeginDeathmatch", (void (*))ClientBeginDeathmatch,   // void ClientBeginDeathmatch(edict_t * ent )  \dday\source/p_client.c:1239
"ClientBeginServerFrame", (void (*))ClientBeginServerFrame,   // void ClientBeginServerFrame(edict_t * ent )  \dday\source/p_client.c:1769
"ClientCommand", (void (*))ClientCommand,   // void ClientCommand(edict_t * ent )  \dday\source/g_cmds.c:1516
"ClientConnect", (void (*))ClientConnect,   // qboolean ClientConnect(edict_t * ent , char * userinfo )  \dday\source/p_client.c:1398
"ClientDisconnect", (void (*))ClientDisconnect,   // void ClientDisconnect(edict_t * ent )  \dday\source/p_client.c:1450
"ClientEndServerFrame", (void (*))ClientEndServerFrame,   // void ClientEndServerFrame(edict_t * ent )  \dday\source/p_view.c:959
"ClientEndServerFrames", (void (*))ClientEndServerFrames,   // void ClientEndServerFrames(void )  \dday\source/g_main.c:178
"ClientObituary", (void (*))ClientObituary,   // void ClientObituary(edict_t * self , edict_t * inflictor , edict_t * attacker )  \dday\source/p_client.c:197
"ClientTeam", (void (*))ClientTeam,   // char * ClientTeam(edict_t * ent )  \dday\source/g_cmds.c:113
"ClientThink", (void (*))ClientThink,   // void ClientThink(edict_t * ent , usercmd_t * ucmd )  \dday\source/p_client.c:1518
"ClientUserinfoChanged", (void (*))ClientUserinfoChanged,   // void ClientUserinfoChanged(edict_t * ent , char * userinfo )  \dday\source/p_client.c:1340
"ClipGibVelocity", (void (*))ClipGibVelocity,   // void ClipGibVelocity(edict_t * ent )  \dday\source/g_misc.c:51
"ClipVelocity", (void (*))ClipVelocity,   // int ClipVelocity(vec3_t in , vec3_t normal , vec3_t out , float overbounce )  \dday\source/g_phys.c:126
//"Cmd_Airstrike_f", (void (*))Cmd_Airstrike_f,   // void Cmd_Airstrike_f(edict_t * ent )  \dday\source/g_arty.c:257
"Cmd_AliciaMode_f", (void (*))Cmd_AliciaMode_f,   // void Cmd_AliciaMode_f(edict_t * ent )  \dday\source/g_cmds.c:13
"Cmd_Arty_f", (void (*))Cmd_Arty_f,   // void Cmd_Artillery_f(edict_t * ent )  \dday\source/g_arty.c:88
//"Cmd_Create_Team", (void (*))Cmd_Create_Team,   // void Cmd_Create_Team(edict_t * ent )  \dday\source/g_cmds.c:1084
"Cmd_Drop_f", (void (*))Cmd_Drop_f,   // void Cmd_Drop_f(edict_t * ent )  \dday\source/g_cmds.c:593
"Cmd_GameVersion_f", (void (*))Cmd_GameVersion_f,   // void Cmd_GameVersion_f(edict_t * ent )  \dday\source/g_cmds.c:81
"Cmd_Give_f", (void (*))Cmd_Give_f,   // void Cmd_Give_f(edict_t * ent )  \dday\source/g_cmds.c:231
"Cmd_God_f", (void (*))Cmd_God_f,   // void Cmd_God_f(edict_t * ent )  \dday\source/g_cmds.c:368
"Cmd_Help_f", (void (*))Cmd_Help_f,   // void Cmd_Help_f(edict_t * ent )  \dday\source/p_hud.c:410
"Cmd_InvDrop_f", (void (*))Cmd_InvDrop_f,   // void Cmd_InvDrop_f(edict_t * ent )  \dday\source/g_cmds.c:791
"Cmd_InvUse_f", (void (*))Cmd_InvUse_f,   // void Cmd_InvUse_f(edict_t * ent )  \dday\source/g_cmds.c:659
"Cmd_Inven_f", (void (*))Cmd_Inven_f,   // void Cmd_Inven_f(edict_t * ent )  \dday\source/g_cmds.c:628
//"Cmd_Join_team", (void (*))Cmd_Join_team,   // void Cmd_Join_team(edict_t * ent )  \dday\source/g_cmds.c:1152
"Cmd_Kill_f", (void (*))Cmd_Kill_f,   // void Cmd_Kill_f(edict_t * ent )  \dday\source/g_cmds.c:817
//"Cmd_List_team", (void (*))Cmd_List_team,   // void Cmd_List_team(edict_t * ent )  \dday\source/g_cmds.c:1231
//"Cmd_MOS", (void (*))Cmd_MOS,   // void Cmd_MOS(edict_t * ent )  \dday\source/g_cmds.c:1340
"Cmd_Noclip_f", (void (*))Cmd_Noclip_f,   // void Cmd_Noclip_f(edict_t * ent )  \dday\source/g_cmds.c:424
"Cmd_Notarget_f", (void (*))Cmd_Notarget_f,   // void Cmd_Notarget_f(edict_t * ent )  \dday\source/g_cmds.c:397
"Cmd_Players_f", (void (*))Cmd_Players_f,   // void Cmd_Players_f(edict_t * ent )  \dday\source/g_cmds.c:865
"Cmd_PutAway_f", (void (*))Cmd_PutAway_f,   // void Cmd_PutAway_f(edict_t * ent )  \dday\source/g_cmds.c:835
//"Cmd_Quit_team", (void (*))Cmd_Quit_team,   // void Cmd_Quit_team(edict_t * ent )  \dday\source/g_cmds.c:1203
"Cmd_Reload_f", (void (*))Cmd_Reload_f,   // qboolean Cmd_Reload_f(edict_t * ent )  \dday\source/g_cmds.c:1251
"Cmd_Say_f", (void (*))Cmd_Say_f,   // void Cmd_Say_f(edict_t * ent , qboolean team , qboolean arg0 )  \dday\source/g_cmds.c:959
"Cmd_Scope_f", (void (*))Cmd_Scope_f,   // void Cmd_Scope_f(edict_t * ent )  \dday\source/g_cmds.c:87
"Cmd_Score_f", (void (*))Cmd_Score_f,   // void Cmd_Score_f(edict_t * ent )  \dday\source/p_hud.c:330
"Cmd_SexPistols_f", (void (*))Cmd_SexPistols_f,   // void Cmd_SexPistols_f(edict_t * ent )  \dday\source/g_cmds.c:29
"Cmd_Shout_f", (void (*))Cmd_Shout_f,   // void Cmd_Shout_f(edict_t * ent )  \dday\source/g_cmds.c:1567
"Cmd_Stance", (void (*))Cmd_Stance,   // void Cmd_Stance(edict_t * ent )  \dday\source/g_cmds.c:1552
"Cmd_Use_f", (void (*))Cmd_Use_f,   // void Cmd_Use_f(edict_t * ent )  \dday\source/g_cmds.c:492
"Cmd_Wave_f", (void (*))Cmd_Wave_f,   // void Cmd_Wave_f(edict_t * ent )  \dday\source/g_cmds.c:908
"Cmd_WeapLast_f", (void (*))Cmd_WeapLast_f,   // void Cmd_WeapLast_f(edict_t * ent )  \dday\source/g_cmds.c:764
"Cmd_WeapNext_f", (void (*))Cmd_WeapNext_f,   // void Cmd_WeapNext_f(edict_t * ent )  \dday\source/g_cmds.c:728
"Cmd_WeapPrev_f", (void (*))Cmd_WeapPrev_f,   // void Cmd_WeapPrev_f(edict_t * ent )  \dday\source/g_cmds.c:692
"Com_PageInMemory", (void (*))Com_PageInMemory,   // void Com_PageInMemory(byte * buffer , int size )  \dday\source/q_shared.c:1143
"Com_Printf", (void (*))Com_Printf,   // void Com_Printf(char * msg , ... )  \dday\source/g_main.c:156
"Com_sprintf", (void (*))Com_sprintf,   // void Com_sprintf(char * dest , int size , char * fmt , ... )  \dday\source/q_shared.c:1205
"CopyToBodyQue", (void (*))CopyToBodyQue,   // void CopyToBodyQue(edict_t * ent )  \dday\source/p_client.c:960
"CrossProduct", (void (*))CrossProduct,   // void CrossProduct(vec3_t v1 , vec3_t v2 , vec3_t cross )  \dday\source/q_shared.c:734
"Damage_Loc", (void (*))Damage_Loc,   // int Damage_Loc(edict_t * targ , vec3_t point )  \dday\source/g_combat.c:468
//"DeathmatchPlayerScore", (void (*))DeathmatchPlayerScore,   // void DeathmatchPlayerScore(edict_t * ent , edict_t * killer )  \dday\source/p_hud.c:147
"DeathmatchScoreboard", (void (*))DeathmatchScoreboard,   // void DeathmatchScoreboard(edict_t * ent )  \dday\source/p_hud.c:316
//"DeathmatchScoreboardMessage", (void (*))DeathmatchScoreboardMessage,   // void DeathmatchScoreboardMessage(edict_t * ent , edict_t * killer )  \dday\source/p_hud.c:244
"DDayScoreboardMessage", (void (*))DDayScoreboardMessage,   // void DeathmatchScoreboardMessage(edict_t * ent , edict_t * killer )  \dday\source/p_hud.c:244
"DoAnarchyStuff", (void (*))DoAnarchyStuff,   // int DoAnarchyStuff(edict_t * ent )  \dday\source/g_weapon.c:923
"DoEndOM", (void (*))DoEndOM,   // void DoEndOM(edict_t * ent , qboolean calcOfficer )  \dday\source/observer.c:78
"DoRespawn", (void (*))DoRespawn,   // void DoRespawn(edict_t * ent )  \dday\source/g_items.c:97
"Drop_Ammo", (void (*))Drop_Ammo,   // void Drop_Ammo(edict_t * ent , gitem_t * item )  \dday\source/g_items.c:457
"Drop_General", (void (*))Drop_General,   // void Drop_General(edict_t * ent , gitem_t * item )  \dday\source/g_items.c:165
"Drop_Item", (void (*))Drop_Item,   // edict_t * Drop_Item(edict_t * ent , gitem_t * item )  \dday\source/g_items.c:753
"Drop_PowerArmor", (void (*))Drop_PowerArmor,   // void Drop_PowerArmor(edict_t * ent , gitem_t * item )  \dday\source/g_items.c:671
"Drop_Weapon", (void (*))Drop_Weapon,   // void Drop_Weapon(edict_t * ent , gitem_t * item )  \dday\source/p_weapon.c:308
"ED_CallSpawn", (void (*))ED_CallSpawn,   // void ED_CallSpawn(edict_t * ent )  \dday\source/g_spawn.c:328
"ED_NewString", (void (*))ED_NewString,   // char * ED_NewString(char * string )  \dday\source/g_spawn.c:355
"ED_ParseEdict", (void (*))ED_ParseEdict,   // char * ED_ParseEdict(char * data , edict_t * ent )  \dday\source/g_spawn.c:450
"ED_ParseField", (void (*))ED_ParseField,   // void ED_ParseField(char * key , char * value , edict_t * ent )  \dday\source/g_spawn.c:394
"EndDMLevel", (void (*))EndDMLevel,   // void EndDMLevel(void )  \dday\source/g_main.c:202
"EndObserverMode", (void (*))EndObserverMode,   // void EndObserverMode(edict_t * ent )  \dday\source/observer.c:29
"ExitLevel", (void (*))ExitLevel,   // void ExitLevel(void )  \dday\source/g_main.c:323
"FacingIdeal", (void (*))FacingIdeal,   // qboolean FacingIdeal(edict_t * self )  \dday\source/g_ai.c:575
//"Feed_Ammo", (void (*))Feed_Ammo,   // void Feed_Ammo(edict_t * ent )  \dday\source/feeder.c:7
"FetchClientEntData", (void (*))FetchClientEntData,   // void FetchClientEntData(edict_t * ent )  \dday\source/p_client.c:652
"FindCommand", (void (*))FindCommand,   // struct g_cmds_t * FindCommand(char * cmd )  \dday\source/g_cmds.c:1466
"FindItem", (void (*))FindItem,   // gitem_t * FindItem(char * pickup_name )  \dday\source/g_items.c:78
"FindItemByClassname", (void (*))FindItemByClassname,   // gitem_t * FindItemByClassname(char * classname )  \dday\source/g_items.c:55
"FindNextPickup", (void (*))FindNextPickup,   // gitem_t * FindNextPos(edict_t * ent , int position )  \dday\source/g_cmds.c:450
//"FindNextPos", (void (*))FindNextPos,   // gitem_t * FindNextPos(edict_t * ent , int position )  \dday\source/g_cmds.c:450
//"FindTarget", (void (*))FindTarget,   // qboolean FindTarget(edict_t * self )  \dday\source/g_ai.c:388
"Find_Mission_Start_Point", (void (*))Find_Mission_Start_Point,   // void Find_Mission_Start_Point(edict_t * ent , vec3_t origin , vec3_t angles )  \dday\source/p_client.c:908
// "FireGrenade_Explode", (void (*))FireGrenade_Explode,   // static void FireGrenade_Explode(edict_t * ent )  \dday\source/x_fbomb.c:20
// "FireGrenade_Touch", (void (*))FireGrenade_Touch,   // static void FireGrenade_Touch(edict_t * ent , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/x_fbomb.c:97
//"Fire_Arty", (void (*))Fire_Arty,   // void Fire_Arty(edict_t * battary , edict_t * ent )  \dday\source/g_arty.c:153
"FloatNoSwap", (void (*))FloatNoSwap,   // float FloatNoSwap(float f )  \dday\source/q_shared.c:982
"FloatSwap", (void (*))FloatSwap,   // float FloatSwap(float f )  \dday\source/q_shared.c:965
//"FoundTarget", (void (*))FoundTarget,   // void FoundTarget(edict_t * self )  \dday\source/g_ai.c:328
"G_CopyString", (void (*))G_CopyString,   // char * G_CopyString(char * in )  \dday\source/g_utils.c:363
"G_Find", (void (*))G_Find,   // edict_t * G_Find(edict_t * from , int fieldofs , char * match )  \dday\source/g_utils.c:28
"G_FindTeams", (void (*))G_FindTeams,   // void G_FindTeams(void )  \dday\source/g_spawn.c:506
"G_FreeEdict", (void (*))G_FreeEdict,   // void G_FreeEdict(edict_t * ed )  \dday\source/g_utils.c:424
"G_InitEdict", (void (*))G_InitEdict,   // void G_InitEdict(edict_t * e )  \dday\source/g_utils.c:373
"G_PickTarget", (void (*))G_PickTarget,   // edict_t * G_PickTarget(char * targetname )  \dday\source/g_utils.c:101
"G_ProjectSource", (void (*))G_ProjectSource,   // void G_ProjectSource(vec3_t point , vec3_t distance , vec3_t forward , vec3_t right , vec3_t result )  \dday\source/g_utils.c:8
"G_RunEntity", (void (*))G_RunEntity,   // void G_RunEntity(edict_t * ent )  \dday\source/g_phys.c:910
"G_RunFrame", (void (*))G_RunFrame,   // void G_RunFrame(void )  \dday\source/g_main.c:359
"G_SetClientEffects", (void (*))G_SetClientEffects,   // void G_SetClientEffects(edict_t * ent )  \dday\source/p_view.c:757
"G_SetClientEvent", (void (*))G_SetClientEvent,   // void G_SetClientEvent(edict_t * ent )  \dday\source/p_view.c:810
"G_SetClientFrame", (void (*))G_SetClientFrame,   // void G_SetClientFrame(edict_t * ent )  \dday\source/p_view.c:864
"G_SetClientSound", (void (*))G_SetClientSound,   // void G_SetClientSound(edict_t * ent )  \dday\source/p_view.c:827
"G_SetMovedir", (void (*))G_SetMovedir,   // void G_SetMovedir(vec3_t angles , vec3_t movedir )  \dday\source/g_utils.c:297
"G_SetStats", (void (*))G_SetStats,   // void G_SetStats(edict_t * ent )  \dday\source/p_hud.c:441
"G_Spawn", (void (*))G_Spawn,   // edict_t * G_Spawn(void )  \dday\source/g_utils.c:392
"G_TouchSolids", (void (*))G_TouchSolids,   // void G_TouchSolids(edict_t * ent )  \dday\source/g_utils.c:480
"G_TouchTriggers", (void (*))G_TouchTriggers,   // void G_TouchTriggers(edict_t * ent )  \dday\source/g_utils.c:447
"G_UseTargets", (void (*))G_UseTargets,   // void G_UseTargets(edict_t * ent , edict_t * activator )  \dday\source/g_utils.c:156
"GetGameAPI", (void (*))GetGameAPI,   // game_export_t * GetGameAPI(game_import_t * import )  \dday\source/g_main.c:112
"GetItemByIndex", (void (*))GetItemByIndex,   // gitem_t * GetItemByIndex(int index )  \dday\source/g_items.c:40
"Give_Class_Ammo", (void (*))Give_Class_Ammo,   // void Give_Class_Ammo(edict_t * ent )  \dday\source/p_classes.c:43
"Give_Class_Weapon", (void (*))Give_Class_Weapon,   // void Give_Class_Weapon(edict_t * ent )  \dday\source/p_classes.c:8
"HelpComputer", (void (*))HelpComputer,   // void HelpComputer(edict_t * ent )  \dday\source/p_hud.c:366
"HuntTarget", (void (*))HuntTarget,   // void HuntTarget(edict_t * self )  \dday\source/g_ai.c:312
//"Impact_Airstrike", (void (*))Impact_Airstrike,   // void Impact_Airstrike(edict_t * ent )  \dday\source/g_arty.c:334
//"Impact_Arty", (void (*))Impact_Arty,   // void Impact_Arty(edict_t * ent )  \dday\source/g_arty.c:415
"In_Vector_Range", (void (*))In_Vector_Range,   // qboolean In_Vector_Range(vec3_t point , vec3_t origin , float x_range , float y_range ,  float z_range )  \dday\source/g_combat.c:455
"Info_RemoveKey", (void (*))Info_RemoveKey,   // void Info_RemoveKey(char * s , char * key )  \dday\source/q_shared.c:1277
"Info_SetValueForKey", (void (*))Info_SetValueForKey,   // void Info_SetValueForKey(char * s , char * key , char * value )  \dday\source/q_shared.c:1344
"Info_Validate", (void (*))Info_Validate,   // qboolean Info_Validate(char * s )  \dday\source/q_shared.c:1335
"Info_ValueForKey", (void (*))Info_ValueForKey,   // char * Info_ValueForKey(char * s , char * key )  \dday\source/q_shared.c:1235
"InitBodyQue", (void (*))InitBodyQue,   // void InitBodyQue(void )  \dday\source/p_client.c:932
"InitClientPersistant", (void (*))InitClientPersistant,   // void InitClientPersistant(gclient_t * client )  \dday\source/p_client.c:590
"InitClientResp", (void (*))InitClientResp,   // void InitClientResp(gclient_t * client )  \dday\source/p_client.c:617
"InitGame", (void (*))InitGame,   // void InitGame(void )  \dday\source/g_save.c:160
"InitItems", (void (*))InitItems,   // void InitItems(void )  \dday\source/g_items.c:1669
"InitMOS_List", (void (*))InitMOS_List,   // void InitMOS_List(TeamS_t * team , SMos_t * mos_list , char language )  \dday\source/p_classes.c:79
"InitTrigger", (void (*))InitTrigger,   // void InitTrigger(edict_t * self )  \dday\source/g_trigger.c:4
"InitializeUserDLLs", (void (*))InitializeUserDLLs,   // int InitializeUserDLLs(struct userdll_list_node * unode , int teamindex )  \dday\source/g_dll.c:86
"InsertCmds", (void (*))InsertCmds,   // void InsertCmds(struct g_cmds_t * cmds , int numCmds , char * src )  \dday\source/g_cmds.c:1389
"InsertEntity", (void (*))InsertEntity,   // spawn_t * InsertEntity(spawn_t * spawnInfo )  \dday\source/u_entmgr.c:104
"InsertItem", (void (*))InsertItem,   // gitem_t * InsertItem(gitem_t * it , spawn_t * spawnInfo )  \dday\source/u_entmgr.c:34
"IsFemale", (void (*))IsFemale,   // qboolean IsFemale(edict_t * ent )  \dday\source/p_client.c:183
"KillBox", (void (*))KillBox,   // qboolean KillBox(edict_t * ent )  \dday\source/g_utils.c:521
"Killed", (void (*))Killed,   // void Killed(edict_t * targ , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/g_combat.c:154
"Knife_Throw", (void (*))Knife_Throw,   // void Knife_Throw(edict_t * self , vec3_t start , vec3_t dir , int damage )  \dday\source/p_weapon.c:641
"LerpAngle", (void (*))LerpAngle,   // float LerpAngle(float a2 , float a1 , float frac )  \dday\source/q_shared.c:264
"LevelExitUserDLLs", (void (*))LevelExitUserDLLs,   // void LevelExitUserDLLs()  \dday\source/g_dll.c:175
"LevelStartUserDLLs", (void (*))LevelStartUserDLLs,   // void LevelStartUserDLLs(edict_t * ent )  \dday\source/g_dll.c:163
"LittleFloat", (void (*))LittleFloat,   // float LittleFloat(float l )  \dday\source/q_shared.c:930
"LittleLong", (void (*))LittleLong,   // int LittleLong(int l )  \dday\source/q_shared.c:928
"LoadUserDLLs", (void (*))LoadUserDLLs,   // struct userdll_list_node * LoadUserDLLs(edict_t * ent )  \dday\source/g_dll.c:14
"LongNoSwap", (void (*))LongNoSwap,   // int LongNoSwap(int l )  \dday\source/q_shared.c:960
"LongSwap", (void (*))LongSwap,   // int LongSwap(int l )  \dday\source/q_shared.c:948
"LookAtKiller", (void (*))LookAtKiller,   // void LookAtKiller(edict_t * self , edict_t * inflictor , edict_t * attacker )  \dday\source/p_client.c:462
"M_MOS_Join", (void (*))M_MOS_Join,   // void MOS_Join(edict_t * ent , int choice )  \dday\source/observer.c:126
"M_CatagorizePosition", (void (*))M_CatagorizePosition,   // void M_CatagorizePosition(edict_t * ent )  \dday\source/g_monster.c:93
"M_ChangeYaw", (void (*))M_ChangeYaw,   // void M_ChangeYaw(edict_t * ent )  \dday\source/m_move.c:285
"M_CheckAttack", (void (*))M_CheckAttack,   // qboolean M_CheckAttack(edict_t * self )  \dday\source/g_ai.c:588
"M_CheckBottom", (void (*))M_CheckBottom,   // qboolean M_CheckBottom(edict_t * ent )  \dday\source/m_move.c:18
"M_CheckGround", (void (*))M_CheckGround,   // void M_CheckGround(edict_t * ent )  \dday\source/g_monster.c:51
//"M_FliesOff", (void (*))M_FliesOff,   // static void M_FliesOff(edict_t * self )  \dday\source/g_monster.c:17
//"M_FliesOn", (void (*))M_FliesOn,   // static void M_FliesOn(edict_t * self )  \dday\source/g_monster.c:23
"M_FlyCheck", (void (*))M_FlyCheck,   // void M_FlyCheck(edict_t * self )  \dday\source/g_monster.c:33
"M_MoveFrame", (void (*))M_MoveFrame,   // void M_MoveFrame(edict_t * self )  \dday\source/g_monster.c:271
"M_MoveToGoal", (void (*))M_MoveToGoal,   // void M_MoveToGoal(edict_t * ent , float dist )  \dday\source/m_move.c:496
"M_ReactToDamage", (void (*))M_ReactToDamage,   // void M_ReactToDamage(edict_t * targ , edict_t * attacker )  \dday\source/g_combat.c:381
"M_SetEffects", (void (*))M_SetEffects,   // void M_SetEffects(edict_t * ent )  \dday\source/g_monster.c:242
"M_WorldEffects", (void (*))M_WorldEffects,   // void M_WorldEffects(edict_t * ent )  \dday\source/g_monster.c:128
"M_droptofloor", (void (*))M_droptofloor,   // void M_droptofloor(edict_t * ent )  \dday\source/g_monster.c:220
"M_walkmove", (void (*))M_walkmove,   // qboolean M_walkmove(edict_t * ent , float yaw , float dist )  \dday\source/m_move.c:523
"MegaHealth_think", (void (*))MegaHealth_think,   // void MegaHealth_think(edict_t * self )  \dday\source/g_items.c:476
/*"Menu_Add", (void (*))Menu_Add,   // void Menu_Add(edict_t * ent , char * text )  \dday\source/Qmenu.c:243
"Menu_Clear", (void (*))Menu_Clear,   // void Menu_Clear(edict_t * ent )  \dday\source/Qmenu.c:213
"Menu_Close", (void (*))Menu_Close,   // void Menu_Close(edict_t * ent )  \dday\source/Qmenu.c:369
"Menu_Dn", (void (*))Menu_Dn,   // void Menu_Dn(edict_t * ent )  \dday\source/Qmenu.c:417
"Menu_Init", (void (*))Menu_Init,   // void Menu_Init(edict_t * ent )  \dday\source/Qmenu.c:188
"Menu_Msg", (void (*))Menu_Msg,   // void Menu_Msg(edict_t * ent , char * message )  \dday\source/Qmenu.c:65
"Menu_Open", (void (*))Menu_Open,   // void Menu_Open(edict_t * ent )  \dday\source/Qmenu.c:293
"Menu_Sel", (void (*))Menu_Sel,   // void Menu_Sel(edict_t * ent )  \dday\source/Qmenu.c:443
"Menu_Title", (void (*))Menu_Title,   // void Menu_Title(edict_t * ent , char * text )  \dday\source/Qmenu.c:270
"Menu_Up", (void (*))Menu_Up,   // void Menu_Up(edict_t * ent )  \dday\source/Qmenu.c:391*/
"MoveClientToIntermission", (void (*))MoveClientToIntermission,   // void MoveClientToIntermission(edict_t * ent )  \dday\source/p_hud.c:13
"Move_Begin", (void (*))Move_Begin,   // void Move_Begin(edict_t * ent )  \dday\source/g_func.c:77
"Move_Calc", (void (*))Move_Calc,   // void Move_Calc(edict_t * ent , vec3_t dest , void ( * func ) ( edict_t * ) )  \dday\source/g_func.c:95
"Move_Done", (void (*))Move_Done,   // void Move_Done(edict_t * ent )  \dday\source/g_func.c:57
"Move_Final", (void (*))Move_Final,   // void Move_Final(edict_t * ent )  \dday\source/g_func.c:63
"NoAmmoWeaponChange", (void (*))NoAmmoWeaponChange,   // void NoAmmoWeaponChange(edict_t * ent )  \dday\source/p_weapon.c:201
"PBM_ActivePowerArmor", (void (*))PBM_ActivePowerArmor,   // qboolean PBM_ActivePowerArmor(edict_t * ent , vec3_t point )  \dday\source/x_fire.c:264
"PBM_BecomeSmallExplosion", (void (*))PBM_BecomeSmallExplosion,   // void PBM_BecomeSmallExplosion(edict_t * self )  \dday\source/x_fire.c:166
"PBM_BecomeSmoke", (void (*))PBM_BecomeSmoke,   // void PBM_BecomeSmoke(edict_t * self )  \dday\source/x_fire.c:83
"PBM_BecomeSteam", (void (*))PBM_BecomeSteam,   // void PBM_BecomeSteam(edict_t * self )  \dday\source/x_fire.c:106
"PBM_Burn", (void (*))PBM_Burn,   // void PBM_Burn(edict_t * self )  \dday\source/x_fire.c:503
"PBM_BurnDamage", (void (*))PBM_BurnDamage,   // void PBM_BurnDamage(edict_t * victim , edict_t * fire , vec3_t point , vec3_t damage , int dflags , int mod )  \dday\source/x_fire.c:332
"PBM_BurnRadius", (void (*))PBM_BurnRadius,   // void PBM_BurnRadius(edict_t * fire , float radius , vec3_t damage , edict_t * ignore )  \dday\source/x_fire.c:360
"PBM_CheckFire", (void (*))PBM_CheckFire,   // void PBM_CheckFire(edict_t * self )  \dday\source/x_fire.c:608
"PBM_CheckMaster", (void (*))PBM_CheckMaster,   // void PBM_CheckMaster(edict_t * fire )  \dday\source/x_fire.c:392
"PBM_CloudBurst", (void (*))PBM_CloudBurst,   // void PBM_CloudBurst(edict_t * self )  \dday\source/x_fire.c:739
"PBM_CloudBurstDamage", (void (*))PBM_CloudBurstDamage,   // void PBM_CloudBurstDamage(edict_t * self )  \dday\source/x_fire.c:750
"PBM_EasyFireDrop", (void (*))PBM_EasyFireDrop,   // void PBM_EasyFireDrop(edict_t * self )  \dday\source/x_fire.c:728
"PBM_FireAngleSpread", (void (*))PBM_FireAngleSpread,   // void PBM_FireAngleSpread(vec3_t spread , vec3_t dir )  \dday\source/x_fire.c:855
"PBM_FireDrop", (void (*))PBM_FireDrop,   // void PBM_FireDrop(edict_t * attacker , vec3_t spot , vec3_t damage , vec3_t radius_damage , int blast_chance )  \dday\source/x_fire.c:690
"PBM_FireDropTouch", (void (*))PBM_FireDropTouch,   // void PBM_FireDropTouch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/x_fire.c:646
"PBM_FireFlameThrower", (void (*))PBM_FireFlameThrower,   // void PBM_FireFlameThrower(edict_t * self , vec3_t start , vec3_t spread , vec3_t dir , int speed , vec3_t damage , vec3_t radius_damage , int rain_chance , int blast_chance )  \dday\source/x_fire.c:1043
"PBM_FireFlamer", (void (*))PBM_FireFlamer,   // void PBM_FireFlamer(edict_t * self , vec3_t start , vec3_t spread , vec3_t dir , int speed , vec3_t damage , vec3_t radius_damage , int rain_chance , int blast_chance )  \dday\source/x_fire.c:915
"PBM_FireResistant", (void (*))PBM_FireResistant,   // qboolean PBM_FireResistant(edict_t * ent , vec3_t point )  \dday\source/x_fire.c:304
"PBM_FireSpot", (void (*))PBM_FireSpot,   // void PBM_FireSpot(vec3_t spot , edict_t * ent )  \dday\source/x_fire.c:404
"PBM_FireballTouch", (void (*))PBM_FireballTouch,   // void PBM_FireballTouch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/x_fire.c:873
"PBM_FlameCloud", (void (*))PBM_FlameCloud,   // void PBM_FlameCloud(edict_t * attacker , vec3_t start , vec3_t cloud , vec3_t timer , qboolean deadly , vec3_t damage , vec3_t radius_damage , int rain_chance , int blast_chance )  \dday\source/x_fire.c:770
"PBM_FlameOut", (void (*))PBM_FlameOut,   // qboolean PBM_FlameOut(edict_t * self )  \dday\source/x_fire.c:443
"PBM_FlameThrowerThink", (void (*))PBM_FlameThrowerThink,   // void PBM_FlameThrowerThink(edict_t * self )  \dday\source/x_fire.c:1020
"PBM_FlameThrowerTouch", (void (*))PBM_FlameThrowerTouch,   // void PBM_FlameThrowerTouch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/x_fire.c:969
"PBM_Ignite", (void (*))PBM_Ignite,   // void PBM_Ignite(edict_t * victim , edict_t * attacker , vec3_t point )  \dday\source/x_fire.c:542
"PBM_InWater", (void (*))PBM_InWater,   // qboolean PBM_InWater(edict_t * ent )  \dday\source/x_fire.c:214
"PBM_Inflammable", (void (*))PBM_Inflammable,   // qboolean PBM_Inflammable(edict_t * ent )  \dday\source/x_fire.c:231
"PBM_KillAllFires", (void (*))PBM_KillAllFires,   // void PBM_KillAllFires(void )  \dday\source/x_fire.c:188
"PBM_SmallExplodeThink", (void (*))PBM_SmallExplodeThink,   // void PBM_SmallExplodeThink(edict_t * self )  \dday\source/x_fire.c:121
"PBM_StartSmallExplosion", (void (*))PBM_StartSmallExplosion,   // void PBM_StartSmallExplosion(edict_t * self )  \dday\source/x_fire.c:141
"PM_trace", (void (*))PM_trace,   // trace_t PM_trace(vec3_t start , vec3_t mins , vec3_t maxs , vec3_t end )  \dday\source/p_client.c:1483
"P_DamageFeedback", (void (*))P_DamageFeedback,   // void P_DamageFeedback(edict_t * player )  \dday\source/p_view.c:53
"P_FallingDamage", (void (*))P_FallingDamage,   // void P_FallingDamage(edict_t * ent )  \dday\source/p_view.c:509
"P_ProjectSource", (void (*))P_ProjectSource,   // void P_ProjectSource(gclient_t * client , vec3_t point , vec3_t distance , vec3_t forward , vec3_t right , vec3_t result )  \dday\source/p_weapon.c:17
"P_WorldEffects", (void (*))P_WorldEffects,   // void P_WorldEffects(void )  \dday\source/p_view.c:587
"PerpendicularVector", (void (*))PerpendicularVector,   // void PerpendicularVector(vec3_t dst , const vec3_t src )  \dday\source/q_shared.c:133
//"Pick_Air", (void (*))Pick_Air,   // void Pick_Air(edict_t * ent , int battary )  \dday\source/g_arty.c:191
//"Pick_Arty", (void (*))Pick_Arty,   // void Pick_Arty(edict_t * ent , int battary )  \dday\source/g_arty.c:18
"Pickup_Adrenaline", (void (*))Pickup_Adrenaline,   // qboolean Pickup_Adrenaline(edict_t * ent , edict_t * other )  \dday\source/g_items.c:176
"Pickup_Ammo", (void (*))Pickup_Ammo,   // qboolean Pickup_Ammo(edict_t * ent , edict_t * other )  \dday\source/g_items.c:425
"Pickup_Armor", (void (*))Pickup_Armor,   // qboolean Pickup_Armor(edict_t * ent , edict_t * other )  \dday\source/g_items.c:566
"Pickup_Bandolier", (void (*))Pickup_Bandolier,   // qboolean Pickup_Bandolier(edict_t * ent , edict_t * other )  \dday\source/g_items.c:193
"Pickup_Health", (void (*))Pickup_Health,   // qboolean Pickup_Health(edict_t * ent , edict_t * other )  \dday\source/g_items.c:491
"Pickup_Key", (void (*))Pickup_Key,   // qboolean Pickup_Key(edict_t * ent , edict_t * other )  \dday\source/g_items.c:355
"Pickup_Pack", (void (*))Pickup_Pack,   // qboolean Pickup_Pack(edict_t * ent , edict_t * other )  \dday\source/g_items.c:234
"Pickup_PowerArmor", (void (*))Pickup_PowerArmor,   // qboolean Pickup_PowerArmor(edict_t * ent , edict_t * other )  \dday\source/g_items.c:651
"Pickup_Powerup", (void (*))Pickup_Powerup,   // qboolean Pickup_Powerup(edict_t * ent , edict_t * other )  \dday\source/g_items.c:137
"Pickup_Weapon", (void (*))Pickup_Weapon,   // qboolean Pickup_Weapon(edict_t * ent , edict_t * other )  \dday\source/p_weapon.c:102
"PlayerDiesUserDLLs", (void (*))PlayerDiesUserDLLs,   // void PlayerDiesUserDLLs(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/g_dll.c:200
"PlayerNoise", (void (*))PlayerNoise,   // void PlayerNoise(edict_t * who , vec3_t where , int type )  \dday\source/p_weapon.c:42
"PlayerSort", (void (*))PlayerSort,   // int PlayerSort(void const * a , void const * b )  \dday\source/g_cmds.c:843
"PlayerSpawnUserDLLs", (void (*))PlayerSpawnUserDLLs,   // void PlayerSpawnUserDLLs(edict_t * ent )  \dday\source/g_dll.c:188
"PlayerTrail_Add", (void (*))PlayerTrail_Add,   // void PlayerTrail_Add(vec3_t spot )  \dday\source/p_trail.c:48
"PlayerTrail_Init", (void (*))PlayerTrail_Init,   // void PlayerTrail_Init(void )  \dday\source/p_trail.c:30
"PlayerTrail_LastSpot", (void (*))PlayerTrail_LastSpot,   // edict_t * PlayerTrail_LastSpot(void )  \dday\source/p_trail.c:124
"PlayerTrail_New", (void (*))PlayerTrail_New,   // void PlayerTrail_New(vec3_t spot )  \dday\source/p_trail.c:66
"PlayerTrail_PickFirst", (void (*))PlayerTrail_PickFirst,   // edict_t * PlayerTrail_PickFirst(edict_t * self )  \dday\source/p_trail.c:76
"PlayerTrail_PickNext", (void (*))PlayerTrail_PickNext,   // edict_t * PlayerTrail_PickNext(edict_t * self )  \dday\source/p_trail.c:105
"PlayersRangeFromSpot", (void (*))PlayersRangeFromSpot,   // float PlayersRangeFromSpot(edict_t * spot )  \dday\source/p_client.c:679
"PowerArmorType", (void (*))PowerArmorType,   // int PowerArmorType(edict_t * ent )  \dday\source/g_items.c:623
"PrecacheItem", (void (*))PrecacheItem,   // void PrecacheItem(gitem_t * it )  \dday\source/g_items.c:903
"PrintCmds", (void (*))PrintCmds,   // void PrintCmds()  \dday\source/g_cmds.c:1418
"PrintPmove", (void (*))PrintPmove,   // void PrintPmove(pmove_t * pm )  \dday\source/p_client.c:1499
"ProjectPointOnPlane", (void (*))ProjectPointOnPlane,   // void ProjectPointOnPlane(vec3_t dst , const vec3_t p , const vec3_t normal )  \dday\source/q_shared.c:111
"PutClientInServer", (void (*))PutClientInServer,   // void PutClientInServer(edict_t * ent )  \dday\source/p_client.c:1076
"Q_fabs", (void (*))Q_fabs,   // float Q_fabs(float f )  \dday\source/q_shared.c:232
"Q_log2", (void (*))Q_log2,   // int Q_log2(int val )  \dday\source/q_shared.c:771
"Q_strcasecmp", (void (*))Q_strcasecmp,   // int Q_strcasecmp(char * s1 , char * s2 )  \dday\source/q_shared.c:1198
"Q_stricmp", (void (*))Q_stricmp,   // int Q_stricmp(char * s1 , char * s2 )  \dday\source/q_shared.c:1162
"Q_strncasecmp", (void (*))Q_strncasecmp,   // int Q_strncasecmp(char * s1 , char * s2 , int n )  \dday\source/q_shared.c:1172
"R_ConcatRotations", (void (*))R_ConcatRotations,   // void R_ConcatRotations(float in1 [ 3 ] [ 3 ] , float in2 [ 3 ] [ 3 ] , float out [ 3 ] [ 3 ] )  \dday\source/q_shared.c:172
"R_ConcatTransforms", (void (*))R_ConcatTransforms,   // void R_ConcatTransforms(float in1 [ 3 ] [ 4 ] , float in2 [ 3 ] [ 4 ] , float out [ 3 ] [ 4 ] )  \dday\source/q_shared.c:200
"ReadClient", (void (*))ReadClient,   // void ReadClient(FILE * f , gclient_t * client )  \dday\source/g_save.c:405
"ReadEdict", (void (*))ReadEdict,   // void ReadEdict(FILE * f , edict_t * ent )  \dday\source/g_save.c:562
"ReadField", (void (*))ReadField,   // void ReadField(FILE * f , field_t * field , byte * base )  \dday\source/g_save.c:302
"ReadGame", (void (*))ReadGame,   // void ReadGame(char * filename )  \dday\source/g_save.c:458
"ReadLevel", (void (*))ReadLevel,   // void ReadLevel(char * filename )  \dday\source/g_save.c:653
"ReadLevelLocals", (void (*))ReadLevelLocals,   // void ReadLevelLocals(FILE * f )  \dday\source/g_save.c:581
"RemoveEntity", (void (*))RemoveEntity,   // int RemoveEntity(char * name )  \dday\source/u_entmgr.c:132
"RotatePointAroundVector", (void (*))RotatePointAroundVector,   // void RotatePointAroundVector(vec3_t dst , const vec3_t dir , const vec3_t point , float degrees )  \dday\source/q_shared.c:13
//"SMOS_Join", (void (*))SMOS_Join,   // void SMOS_Join(edict_t * ent , int choice )  \dday\source/observer.c:135
// "SP_CreateCoopSpots", (void (*))SP_CreateCoopSpots,   // static void SP_CreateCoopSpots(edict_t * self )  \dday\source/p_client.c:59
// "SP_FixCoopSpots", (void (*))SP_FixCoopSpots,   // static void SP_FixCoopSpots(edict_t * self )  \dday\source/p_client.c:28
//"SP_event_Arty_Strike", (void (*))SP_event_Arty_Strike,   // void SP_event_Arty_Strike(edict_t * ent )  \dday\source/g_ents.c:271
"SP_func_areaportal", (void (*))SP_func_areaportal,   // void SP_func_areaportal(edict_t * ent )  \dday\source/g_misc.c:25
"SP_func_button", (void (*))SP_func_button,   // void SP_func_button(edict_t * ent )  \dday\source/g_func.c:744
"SP_func_clock", (void (*))SP_func_clock,   // void SP_func_clock(edict_t * self )  \dday\source/g_misc.c:1738
"SP_func_conveyor", (void (*))SP_func_conveyor,   // void SP_func_conveyor(edict_t * self )  \dday\source/g_func.c:1822
"SP_func_door", (void (*))SP_func_door,   // void SP_func_door(edict_t * ent )  \dday\source/g_func.c:1119
"SP_func_door_rotating", (void (*))SP_func_door_rotating,   // void SP_func_door_rotating(edict_t * ent )  \dday\source/g_func.c:1242
"SP_func_door_secret", (void (*))SP_func_door_secret,   // void SP_func_door_secret(edict_t * ent )  \dday\source/g_func.c:1946
"SP_func_explosive", (void (*))SP_func_explosive,   // void SP_func_explosive(edict_t * self )  \dday\source/g_misc.c:798
"SP_func_killbox", (void (*))SP_func_killbox,   // void SP_func_killbox(edict_t * ent )  \dday\source/g_func.c:2022
"SP_func_object", (void (*))SP_func_object,   // void SP_func_object(edict_t * self )  \dday\source/g_misc.c:667
"SP_func_plat", (void (*))SP_func_plat,   // void SP_func_plat(edict_t * ent )  \dday\source/g_func.c:494
"SP_func_rotating", (void (*))SP_func_rotating,   // void SP_func_rotating(edict_t * ent )  \dday\source/g_func.c:604
"SP_func_timer", (void (*))SP_func_timer,   // void SP_func_timer(edict_t * self )  \dday\source/g_func.c:1775
"SP_func_train", (void (*))SP_func_train,   // void SP_func_train(edict_t * self )  \dday\source/g_func.c:1634
"SP_func_wall", (void (*))SP_func_wall,   // void SP_func_wall(edict_t * self )  \dday\source/g_misc.c:587
"SP_func_water", (void (*))SP_func_water,   // void SP_func_water(edict_t * self )  \dday\source/g_func.c:1359
//"SP_info_Air_Battery", (void (*))SP_info_Air_Battery,   // void SP_info_Air_Battery(edict_t * ent )  \dday\source/g_ents.c:244
//"SP_info_Arty_Battery", (void (*))SP_info_Arty_Battery,   // void SP_info_Arty_Battery(edict_t * ent )  \dday\source/g_ents.c:218
"SP_info_Engineer_Start", (void (*))SP_info_Engineer_Start,   // void SP_info_Engineer_Start(edict_t * ent )  \dday\source/p_client.c:167
"SP_info_Flamethrower_Start", (void (*))SP_info_Flamethrower_Start,   // void SP_info_Flamethrower_Start(edict_t * ent )  \dday\source/p_client.c:169
"SP_info_H_Gunner_Start", (void (*))SP_info_H_Gunner_Start,   // void SP_info_H_Gunner_Start(edict_t * ent )  \dday\source/p_client.c:165
"SP_info_Infantry_Start", (void (*))SP_info_Infantry_Start,   // void SP_info_Infantry_Start(edict_t * ent )  \dday\source/p_client.c:163
"SP_info_L_Gunner_Start", (void (*))SP_info_L_Gunner_Start,   // void SP_info_L_Gunner_Start(edict_t * ent )  \dday\source/p_client.c:164
"SP_info_Officer_Start", (void (*))SP_info_Officer_Start,   // void SP_info_Officer_Start(edict_t * ent )  \dday\source/p_client.c:164
"SP_info_Medic_Start", (void (*))SP_info_Medic_Start,   // void SP_info_Medic_Start(edict_t * ent )  \dday\source/p_client.c:168
"SP_info_Mission_Results", (void (*))SP_info_Mission_Results,   // void SP_info_Mission_Results(edict_t * ent )  \dday\source/g_ents.c:94
//"SP_info_Skin", (void (*))SP_info_Skin,   // void SP_info_Skin(edict_t * ent )  \dday\source/g_ents.c:205
"SP_info_Sniper_Start", (void (*))SP_info_Sniper_Start,   // void SP_info_Sniper_Start(edict_t * ent )  \dday\source/p_client.c:166
"SP_info_Special_Start", (void (*))SP_info_Special_Start,   // void SP_info_Special_Start(edict_t * ent )  \dday\source/p_client.c:170
"SP_info_notnull", (void (*))SP_info_notnull,   // void SP_info_notnull(edict_t * self )  \dday\source/g_misc.c:503
"SP_info_null", (void (*))SP_info_null,   // void SP_info_null(edict_t * self )  \dday\source/g_misc.c:494
"SP_info_player_coop", (void (*))SP_info_player_coop,   // void SP_info_player_coop(edict_t * self )  \dday\source/p_client.c:126
"SP_info_player_deathmatch", (void (*))SP_info_player_deathmatch,   // void SP_info_player_deathmatch(edict_t * self )  \dday\source/p_client.c:112
"SP_info_player_intermission", (void (*))SP_info_player_intermission,   // void SP_info_player_intermission(void )  \dday\source/p_client.c:160
"SP_info_player_start", (void (*))SP_info_player_start,   // void SP_info_player_start(edict_t * self )  \dday\source/p_client.c:97
"SP_info_reinforcement_start", (void (*))SP_info_reinforcement_start,   // void SP_info_reinforcement_start(edict_t * ent )  \dday\source/g_ents.c:133
"SP_info_team_start", (void (*))SP_info_team_start,   // void SP_info_team_start(edict_t * ent )  \dday\source/g_ents.c:141
"SP_item_ammo_grenades", (void (*))SP_item_ammo_grenades,   // void SP_item_ammo_grenades(edict_t * self )  \dday\source/g_items.c:1750
"SP_item_ammo_napalm", (void (*))SP_item_ammo_napalm,   // void SP_item_ammo_napalm(edict_t * self )  \dday\source/g_spawn.c:29
"SP_item_armor_body", (void (*))SP_item_armor_body,   // void SP_item_armor_body(edict_t * self )  \dday\source/g_items.c:1724
"SP_item_armor_combat", (void (*))SP_item_armor_combat,   // void SP_item_armor_combat(edict_t * self )  \dday\source/g_items.c:1729
"SP_item_armor_jacket", (void (*))SP_item_armor_jacket,   // void SP_item_armor_jacket(edict_t * self )  \dday\source/g_items.c:1734
"SP_item_armor_shard", (void (*))SP_item_armor_shard,   // void SP_item_armor_shard(edict_t * self )  \dday\source/g_items.c:1739
"SP_item_health", (void (*))SP_item_health,   // void SP_item_health(edict_t * self )  \dday\source/g_items.c:1604
"SP_item_health_large", (void (*))SP_item_health_large,   // void SP_item_health_large(edict_t * self )  \dday\source/g_items.c:1637
"SP_item_health_mega", (void (*))SP_item_health_mega,   // void SP_item_health_mega(edict_t * self )  \dday\source/g_items.c:1653
"SP_item_health_small", (void (*))SP_item_health_small,   // void SP_item_health_small(edict_t * self )  \dday\source/g_items.c:1620
"SP_item_key_blue_key", (void (*))SP_item_key_blue_key,   // void SP_item_key_blue_key(edict_t * self )  \dday\source/g_items.c:1791
"SP_item_key_red_key", (void (*))SP_item_key_red_key,   // void SP_item_key_red_key(edict_t * self )  \dday\source/g_items.c:1796
"SP_item_powerup_adrenaline", (void (*))SP_item_powerup_adrenaline,   // void SP_item_powerup_adrenaline(edict_t * self )  \dday\source/g_items.c:1776
"SP_item_powerup_bandolier", (void (*))SP_item_powerup_bandolier,   // void SP_item_powerup_bandolier(edict_t * self )  \dday\source/g_items.c:1781
"SP_item_powerup_breather", (void (*))SP_item_powerup_breather,   // void SP_item_powerup_breather(edict_t * self )  \dday\source/g_items.c:1766
"SP_item_powerup_enviro", (void (*))SP_item_powerup_enviro,   // void SP_item_powerup_enviro(edict_t * self )  \dday\source/g_items.c:1771
"SP_item_powerup_pack", (void (*))SP_item_powerup_pack,   // void SP_item_powerup_pack(edict_t * self )  \dday\source/g_items.c:1786
"SP_item_powerup_silencer", (void (*))SP_item_powerup_silencer,   // void SP_item_powerup_silencer(edict_t * self )  \dday\source/g_items.c:1761
"SP_item_weapon_flamethrower", (void (*))SP_item_weapon_flamethrower,   // void SP_item_weapon_flamethrower(edict_t * self )  \dday\source/g_spawn.c:23
"SP_item_weapon_mine", (void (*))SP_item_weapon_mine,   // void SP_item_weapon_mine(edict_t * self )  \dday\source/g_items.c:1755
"SP_light", (void (*))SP_light,   // void SP_light(edict_t * self )  \dday\source/g_misc.c:534
"SP_light_mine1", (void (*))SP_light_mine1,   // void SP_light_mine1(edict_t * ent )  \dday\source/g_misc.c:1461
"SP_light_mine2", (void (*))SP_light_mine2,   // void SP_light_mine2(edict_t * ent )  \dday\source/g_misc.c:1472
"SP_misc_actor", (void (*))SP_misc_actor,   // void SP_misc_actor(edict_t * self )  \dday\source/m_actor.c:406
"SP_misc_banner", (void (*))SP_misc_banner,   // void SP_misc_banner(edict_t * ent )  \dday\source/g_misc.c:1182
//bcass start - banner thing
"SP_misc_banner_x", (void (*))SP_misc_banner_x,
"SP_misc_banner_1", (void (*))SP_misc_banner_1,
"SP_misc_banner_2", (void (*))SP_misc_banner_2,
"SP_misc_banner_3", (void (*))SP_misc_banner_3,
"SP_misc_banner_4", (void (*))SP_misc_banner_4,
//bcass end
"SP_misc_bigviper", (void (*))SP_misc_bigviper,   // void SP_misc_bigviper(edict_t * ent )  \dday\source/g_misc.c:1304
"SP_misc_blackhole", (void (*))SP_misc_blackhole,   // void SP_misc_blackhole(edict_t * ent )  \dday\source/g_misc.c:1023
"SP_misc_deadsoldier", (void (*))SP_misc_deadsoldier,   // void SP_misc_deadsoldier(edict_t * ent )  \dday\source/g_misc.c:1212
"SP_misc_easterchick", (void (*))SP_misc_easterchick,   // void SP_misc_easterchick(edict_t * ent )  \dday\source/g_misc.c:1079
"SP_misc_easterchick2", (void (*))SP_misc_easterchick2,   // void SP_misc_easterchick2(edict_t * ent )  \dday\source/g_misc.c:1107
"SP_misc_eastertank", (void (*))SP_misc_eastertank,   // void SP_misc_eastertank(edict_t * ent )  \dday\source/g_misc.c:1051
"SP_misc_explobox", (void (*))SP_misc_explobox,   // void SP_misc_explobox(edict_t * self )  \dday\source/g_misc.c:949
"SP_misc_gib_arm", (void (*))SP_misc_gib_arm,   // void SP_misc_gib_arm(edict_t * ent )  \dday\source/g_misc.c:1484
"SP_misc_gib_head", (void (*))SP_misc_gib_head,   // void SP_misc_gib_head(edict_t * ent )  \dday\source/g_misc.c:1526
"SP_misc_gib_leg", (void (*))SP_misc_gib_leg,   // void SP_misc_gib_leg(edict_t * ent )  \dday\source/g_misc.c:1505
"SP_misc_insane", (void (*))SP_misc_insane,   // void SP_misc_insane(edict_t * self )  \dday\source/m_insane.c:602
"SP_misc_satellite_dish", (void (*))SP_misc_satellite_dish,   // void SP_misc_satellite_dish(edict_t * ent )  \dday\source/g_misc.c:1447
"SP_misc_strogg_ship", (void (*))SP_misc_strogg_ship,   // void SP_misc_strogg_ship(edict_t * ent )  \dday\source/g_misc.c:1403
"SP_misc_teleporter", (void (*))SP_misc_teleporter,   // void SP_misc_teleporter(edict_t * ent )  \dday\source/g_misc.c:1818
"SP_misc_teleporter_dest", (void (*))SP_misc_teleporter_dest,   // void SP_misc_teleporter_dest(edict_t * ent )  \dday\source/g_misc.c:1854
"SP_misc_viper", (void (*))SP_misc_viper,   // void SP_misc_viper(edict_t * ent )  \dday\source/g_misc.c:1273
"SP_misc_viper_bomb", (void (*))SP_misc_viper_bomb,   // void SP_misc_viper_bomb(edict_t * self )  \dday\source/g_misc.c:1366
"SP_monster_boss2", (void (*))SP_monster_boss2,   // void SP_monster_boss2(edict_t * self )  \dday\source/removed.c:56
"SP_monster_boss3_stand", (void (*))SP_monster_boss3_stand,   // void SP_monster_boss3_stand(edict_t * self )  \dday\source/removed.c:66
"SP_monster_brain", (void (*))SP_monster_brain,   // void SP_monster_brain(edict_t * self )  \dday\source/removed.c:31
"SP_monster_chick", (void (*))SP_monster_chick,   // void SP_monster_chick(edict_t * self )  \dday\source/removed.c:20
"SP_monster_commander_body", (void (*))SP_monster_commander_body,   // void SP_monster_commander_body(edict_t * self )  \dday\source/g_misc.c:1150
"SP_monster_flipper", (void (*))SP_monster_flipper,   // void SP_monster_flipper(edict_t * self )  \dday\source/removed.c:15
"SP_monster_floater", (void (*))SP_monster_floater,   // void SP_monster_floater(edict_t * self )  \dday\source/removed.c:36
"SP_monster_flyer", (void (*))SP_monster_flyer,   // void SP_monster_flyer(edict_t * self )  \dday\source/removed.c:26
"SP_monster_hover", (void (*))SP_monster_hover,   // void SP_monster_hover(edict_t * self )  \dday\source/removed.c:41
"SP_monster_jorg", (void (*))SP_monster_jorg,   // void SP_monster_jorg(edict_t * self )  \dday\source/removed.c:61
"SP_monster_mutant", (void (*))SP_monster_mutant,   // void SP_monster_mutant(edict_t * self )  \dday\source/removed.c:46
"SP_monster_supertank", (void (*))SP_monster_supertank,   // void SP_monster_supertank(edict_t * self )  \dday\source/removed.c:51
"SP_monster_tank", (void (*))SP_monster_tank,   // void SP_monster_tank(edict_t * self )  \dday\source/removed.c:8
"SP_path_corner", (void (*))SP_path_corner,   // void SP_path_corner(edict_t * self )  \dday\source/g_misc.c:374
"SP_point_combat", (void (*))SP_point_combat,   // void SP_point_combat(edict_t * self )  \dday\source/g_misc.c:449
"SP_target_actor", (void (*))SP_target_actor,   // void SP_target_actor(edict_t * self )  \dday\source/m_actor.c:566
"SP_target_blaster", (void (*))SP_target_blaster,   // void SP_target_blaster(edict_t * self )  \dday\source/g_target.c:412
"SP_target_changelevel", (void (*))SP_target_changelevel,   // void SP_target_changelevel(edict_t * ent )  \dday\source/g_target.c:283
"SP_target_character", (void (*))SP_target_character,   // void SP_target_character(edict_t * self )  \dday\source/g_misc.c:1551
"SP_target_crosslevel_target", (void (*))SP_target_crosslevel_target,   // void SP_target_crosslevel_target(edict_t * self )  \dday\source/g_target.c:459
"SP_target_crosslevel_trigger", (void (*))SP_target_crosslevel_trigger,   // void SP_target_crosslevel_trigger(edict_t * self )  \dday\source/g_target.c:438
"SP_target_earthquake", (void (*))SP_target_earthquake,   // void SP_target_earthquake(edict_t * self )  \dday\source/g_target.c:774
"SP_target_explosion", (void (*))SP_target_explosion,   // void SP_target_explosion(edict_t * ent )  \dday\source/g_target.c:239
"SP_target_goal", (void (*))SP_target_goal,   // void SP_target_goal(edict_t * ent )  \dday\source/g_target.c:183
"SP_target_help", (void (*))SP_target_help,   // void SP_target_help(edict_t * ent )  \dday\source/g_target.c:112
"SP_target_laser", (void (*))SP_target_laser,   // void SP_target_laser(edict_t * self )  \dday\source/g_target.c:623
"SP_target_lightramp", (void (*))SP_target_lightramp,   // void SP_target_lightramp(edict_t * self )  \dday\source/g_target.c:696
"SP_target_objective", (void (*))SP_target_objective,   // void SP_target_objective(edict_t * ent )  \dday\source/g_ents.c:117
"SP_target_secret", (void (*))SP_target_secret,   // void SP_target_secret(edict_t * ent )  \dday\source/g_target.c:145
"SP_target_spawner", (void (*))SP_target_spawner,   // void SP_target_spawner(edict_t * self )  \dday\source/g_target.c:377
"SP_target_speaker", (void (*))SP_target_speaker,   // void SP_target_speaker(edict_t * ent )  \dday\source/g_target.c:62
"SP_target_splash", (void (*))SP_target_splash,   // void SP_target_splash(edict_t * self )  \dday\source/g_target.c:333
"SP_target_string", (void (*))SP_target_string,   // void SP_target_string(edict_t * self )  \dday\source/g_misc.c:1595
"SP_target_temp_entity", (void (*))SP_target_temp_entity,   // void SP_target_temp_entity(edict_t * ent )  \dday\source/g_target.c:15
"SP_trigger_always", (void (*))SP_trigger_always,   // void SP_trigger_always(edict_t * ent )  \dday\source/g_trigger.c:354
"SP_trigger_counter", (void (*))SP_trigger_counter,   // void SP_trigger_counter(edict_t * self )  \dday\source/g_trigger.c:333
"SP_trigger_elevator", (void (*))SP_trigger_elevator,   // void SP_trigger_elevator(edict_t * self )  \dday\source/g_func.c:1730
"SP_trigger_enough_troops", (void (*))SP_trigger_enough_troops,   // void SP_trigger_enough_troops(edict_t * ent )  \dday\source/g_ents.c:81
"SP_trigger_gravity", (void (*))SP_trigger_gravity,   // void SP_trigger_gravity(edict_t * self )  \dday\source/g_trigger.c:518
"SP_trigger_hurt", (void (*))SP_trigger_hurt,   // void SP_trigger_hurt(edict_t * self )  \dday\source/g_trigger.c:477
"SP_trigger_key", (void (*))SP_trigger_key,   // void SP_trigger_key(edict_t * self )  \dday\source/g_trigger.c:263
"SP_trigger_monsterjump", (void (*))SP_trigger_monsterjump,   // void SP_trigger_monsterjump(edict_t * self )  \dday\source/g_trigger.c:567
"SP_trigger_multiple", (void (*))SP_trigger_multiple,   // void SP_trigger_multiple(edict_t * ent )  \dday\source/g_trigger.c:99
"SP_trigger_once", (void (*))SP_trigger_once,   // void SP_trigger_once(edict_t * ent )  \dday\source/g_trigger.c:149
"SP_trigger_push", (void (*))SP_trigger_push,   // void SP_trigger_push(edict_t * self )  \dday\source/g_trigger.c:405
"SP_trigger_relay", (void (*))SP_trigger_relay,   // void SP_trigger_relay(edict_t * self )  \dday\source/g_trigger.c:175
"SP_turret_base", (void (*))SP_turret_base,   // void SP_turret_base(edict_t * self )  \dday\source/g_turret.c:240
"SP_turret_breach", (void (*))SP_turret_breach,   // void SP_turret_breach(edict_t * self )  \dday\source/g_turret.c:201
"SP_viewthing", (void (*))SP_viewthing,   // void SP_viewthing(edict_t * ent )  \dday\source/g_misc.c:474
"SP_worldspawn", (void (*))SP_worldspawn,   // void SP_worldspawn(edict_t * ent )  \dday\source/g_spawn.c:807
"SV_AddBlend", (void (*))SV_AddBlend,   // void SV_AddBlend(float r , float g , float b , float a , float * v_blend )  \dday\source/p_view.c:405
"SV_AddGravity", (void (*))SV_AddGravity,   // void SV_AddGravity(edict_t * ent )  \dday\source/g_phys.c:302
"SV_AddRotationalFriction", (void (*))SV_AddRotationalFriction,   // void SV_AddRotationalFriction(edict_t * ent )  \dday\source/g_phys.c:771
"SV_CalcBlend", (void (*))SV_CalcBlend,   // void SV_CalcBlend(edict_t * ent )  \dday\source/p_view.c:426
"SV_CalcGunOffset", (void (*))SV_CalcGunOffset,   // void SV_CalcGunOffset(edict_t * ent )  \dday\source/p_view.c:352
"SV_CalcRoll", (void (*))SV_CalcRoll,   // float SV_CalcRoll(vec3_t angles , vec3_t velocity )  \dday\source/p_view.c:24
"SV_CalcViewOffset", (void (*))SV_CalcViewOffset,   // void SV_CalcViewOffset(edict_t * ent )  \dday\source/p_view.c:218
"SV_CheckVelocity", (void (*))SV_CheckVelocity,   // void SV_CheckVelocity(edict_t * ent )  \dday\source/g_phys.c:53
"SV_CloseEnough", (void (*))SV_CloseEnough,   // qboolean SV_CloseEnough(edict_t * ent , edict_t * goal , float dist )  \dday\source/m_move.c:476
"SV_FixCheckBottom", (void (*))SV_FixCheckBottom,   // void SV_FixCheckBottom(edict_t * ent )  \dday\source/m_move.c:370
"SV_FlyMove", (void (*))SV_FlyMove,   // int SV_FlyMove(edict_t * ent , float time , int mask )  \dday\source/g_phys.c:164
"SV_Impact", (void (*))SV_Impact,   // void SV_Impact(edict_t * e1 , trace_t * trace )  \dday\source/g_phys.c:101
"SV_NewChaseDir", (void (*))SV_NewChaseDir,   // void SV_NewChaseDir(edict_t * actor , edict_t * enemy , float dist )  \dday\source/m_move.c:384
"SV_Physics_Noclip", (void (*))SV_Physics_Noclip,   // void SV_Physics_Noclip(edict_t * ent )  \dday\source/g_phys.c:623
"SV_Physics_None", (void (*))SV_Physics_None,   // void SV_Physics_None(edict_t * ent )  \dday\source/g_phys.c:610
"SV_Physics_Pusher", (void (*))SV_Physics_Pusher,   // void SV_Physics_Pusher(edict_t * ent )  \dday\source/g_phys.c:542
"SV_Physics_Step", (void (*))SV_Physics_Step,   // void SV_Physics_Step(edict_t * ent )  \dday\source/g_phys.c:795
"SV_Physics_Toss", (void (*))SV_Physics_Toss,   // void SV_Physics_Toss(edict_t * ent )  \dday\source/g_phys.c:650
"SV_Push", (void (*))SV_Push,   // qboolean SV_Push(edict_t * pusher , vec3_t move , vec3_t amove )  \dday\source/g_phys.c:383
"SV_PushEntity", (void (*))SV_PushEntity,   // trace_t SV_PushEntity(edict_t * ent , vec3_t push )  \dday\source/g_phys.c:322
"SV_RunThink", (void (*))SV_RunThink,   // qboolean SV_RunThink(edict_t * ent )  \dday\source/g_phys.c:76
"SV_StepDirection", (void (*))SV_StepDirection,   // qboolean SV_StepDirection(edict_t * ent , float yaw , float dist )  \dday\source/m_move.c:334
"SV_TestEntityPosition", (void (*))SV_TestEntityPosition,   // edict_t * SV_TestEntityPosition(edict_t * ent )  \dday\source/g_phys.c:30
"SV_movestep", (void (*))SV_movestep,   // qboolean SV_movestep(edict_t * ent , vec3_t move , qboolean relink )  \dday\source/m_move.c:93
"SaveClientData", (void (*))SaveClientData,   // void SaveClientData(void )  \dday\source/p_client.c:634
"SelectCoopSpawnPoint", (void (*))SelectCoopSpawnPoint,   // edict_t * SelectCoopSpawnPoint(edict_t * ent )  \dday\source/p_client.c:816
"SelectDeathmatchSpawnPoint", (void (*))SelectDeathmatchSpawnPoint,   // edict_t * SelectDeathmatchSpawnPoint(void )  \dday\source/p_client.c:807
"SelectFarthestDeathmatchSpawnPoint", (void (*))SelectFarthestDeathmatchSpawnPoint,   // edict_t * SelectFarthestDeathmatchSpawnPoint(void )  \dday\source/p_client.c:774
"SelectNextItem", (void (*))SelectNextItem,   // void SelectNextItem(edict_t * ent , int itflags )  \dday\source/g_cmds.c:140
"SelectPrevItem", (void (*))SelectPrevItem,   // void SelectPrevItem(edict_t * ent , int itflags )  \dday\source/g_cmds.c:175
"SelectRandomDeathmatchSpawnPoint", (void (*))SelectRandomDeathmatchSpawnPoint,   // edict_t * SelectRandomDeathmatchSpawnPoint(void )  \dday\source/p_client.c:718
"SelectSpawnPoint", (void (*))SelectSpawnPoint,   // void SelectSpawnPoint(edict_t * ent , vec3_t origin , vec3_t angles )  \dday\source/p_client.c:860
"ServerCommand", (void (*))ServerCommand,   // void ServerCommand(void )  \dday\source/g_svcmds.c:23
"SetItemNames", (void (*))SetItemNames,   // void SetItemNames(void )  \dday\source/g_items.c:1690
"SetRespawn", (void (*))SetRespawn,   // void SetRespawn(edict_t * ent , float delay )  \dday\source/g_items.c:124
"Show_Mos", (void (*))Show_Mos,   // void Show_Mos(edict_t * ent )  \dday\source/p_classes.c:62
"Shrapnel_Explode", (void (*))Shrapnel_Explode,   // void Shrapnel_Explode(edict_t * ent )  \dday\source/g_weapon.c:486
//bcass start - tnt
"TNT_Explode", (void (*))Shrapnel_Explode,   // void Shrapnel_Explode(edict_t * ent )  \dday\source/g_weapon.c:486
//bcass end
// "Shrapnel_Touch", (void (*))Shrapnel_Touch,   // static void Shrapnel_Touch(edict_t * ent , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_weapon.c:559
"ShutdownGame", (void (*))ShutdownGame,   // void ShutdownGame(void )  \dday\source/g_main.c:92
"SnapToEights", (void (*))SnapToEights,   // float SnapToEights(float x )  \dday\source/g_turret.c:18
"SpawnDamage", (void (*))SpawnDamage,   // void SpawnDamage(int type , vec3_t origin , vec3_t normal , int damage )  \dday\source/g_combat.c:220
"SpawnEntities", (void (*))SpawnEntities,   // void SpawnEntities(char * mapname , char * entities , char * spawnpoint )  \dday\source/g_spawn.c:556
"SpawnItem", (void (*))SpawnItem,   // void SpawnItem(edict_t * ent , gitem_t * item )  \dday\source/g_items.c:971
"Svcmd_Test_f", (void (*))Svcmd_Test_f,   // void Svcmd_Test_f(void )  \dday\source/g_svcmds.c:5
"Swap_Init", (void (*))Swap_Init,   // void Swap_Init(void )  \dday\source/q_shared.c:992
"SwitchToObserver", (void (*))SwitchToObserver,   // void SwitchToObserver(edict_t * ent )  \dday\source/observer.c:10
"Sys_Error", (void (*))Sys_Error,   // void Sys_Error(char * error , ... )  \dday\source/g_main.c:144
"TH_viewthing", (void (*))TH_viewthing,   // void TH_viewthing(edict_t * ent )  \dday\source/g_misc.c:468
"T_Damage", (void (*))T_Damage,   // void T_Damage(edict_t * targ , edict_t * inflictor , edict_t * attacker , vec3_t dir , vec3_t point , vec3_t normal , int damage , int knockback , int dflags , int mod )  \dday\source/g_combat.c:512
"T_RadiusDamage", (void (*))T_RadiusDamage,   // void T_RadiusDamage(edict_t * inflictor , edict_t * attacker , float damage , edict_t * ignore , float radius , int mod )  \dday\source/g_combat.c:724
"M_Team_Join", (void (*))M_Team_Join,   // void Team_Join(edict_t * ent , int choice )  \dday\source/observer.c:179
"Think_AccelMove", (void (*))Think_AccelMove,   // void Think_AccelMove(edict_t * ent )  \dday\source/g_func.c:315
//"Think_Airstrike", (void (*))Think_Airstrike,   // void Think_Airstrike(edict_t * ent )  \dday\source/g_arty.c:379
"Think_Arty", (void (*))Think_Arty,   // void Think_Arty(edict_t * ent )  \dday\source/g_arty.c:469
"Think_CalcMoveSpeed", (void (*))Think_CalcMoveSpeed,   // void Think_CalcMoveSpeed(edict_t * self )  \dday\source/g_func.c:979
"Think_Delay", (void (*))Think_Delay,   // void Think_Delay(edict_t * ent )  \dday\source/g_utils.c:134
"Think_SpawnDoorTrigger", (void (*))Think_SpawnDoorTrigger,   // void Think_SpawnDoorTrigger(edict_t * ent )  \dday\source/g_func.c:1019
"Think_Weapon", (void (*))Think_Weapon,   // void Think_Weapon(edict_t * ent )  \dday\source/p_weapon.c:244
"ThrowClientHead", (void (*))ThrowClientHead,   // void ThrowClientHead(edict_t * self , int damage )  \dday\source/g_misc.c:210
"ThrowDebris", (void (*))ThrowDebris,   // void ThrowDebris(edict_t * self , char * modelname , float speed , vec3_t origin )  \dday\source/g_misc.c:262
"ThrowGib", (void (*))ThrowGib,   // void ThrowGib(edict_t * self , char * gibname , int damage , int type )  \dday\source/g_misc.c:116
"ThrowHead", (void (*))ThrowHead,   // void ThrowHead(edict_t * self , char * gibname , int damage , int type )  \dday\source/g_misc.c:164
"TossClientWeapon", (void (*))TossClientWeapon,   // void TossClientWeapon(edict_t * self )  \dday\source/p_client.c:409
"Touch_DoorTrigger", (void (*))Touch_DoorTrigger,   // void Touch_DoorTrigger(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_func.c:961
"Touch_Item", (void (*))Touch_Item,   // void Touch_Item(edict_t * ent , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_items.c:685
"Touch_Multi", (void (*))Touch_Multi,   // void Touch_Multi(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_trigger.c:53
"Touch_Plat_Center", (void (*))Touch_Plat_Center,   // void Touch_Plat_Center(edict_t * ent , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_func.c:417
"Use_Areaportal", (void (*))Use_Areaportal,   // void Use_Areaportal(edict_t * ent , edict_t * other , edict_t * activator )  \dday\source/g_misc.c:12
"Use_Breather", (void (*))Use_Breather,   // void Use_Breather(edict_t * ent , gitem_t * item )  \dday\source/g_items.c:316
"Use_Envirosuit", (void (*))Use_Envirosuit,   // void Use_Envirosuit(edict_t * ent , gitem_t * item )  \dday\source/g_items.c:331
"Use_Item", (void (*))Use_Item,   // void Use_Item(edict_t * ent , edict_t * other , edict_t * activator )  \dday\source/g_items.c:802
"Use_Multi", (void (*))Use_Multi,   // void Use_Multi(edict_t * ent , edict_t * other , edict_t * activator )  \dday\source/g_trigger.c:47
"Use_Plat", (void (*))Use_Plat,   // void Use_Plat(edict_t * ent , edict_t * other , edict_t * activator )  \dday\source/g_func.c:409
"Use_PowerArmor", (void (*))Use_PowerArmor,   // void Use_PowerArmor(edict_t * ent , gitem_t * item )  \dday\source/g_items.c:629
"Use_Silencer", (void (*))Use_Silencer,   // void Use_Silencer(edict_t * ent , gitem_t * item )  \dday\source/g_items.c:344
"Use_Target_Help", (void (*))Use_Target_Help,   // void Use_Target_Help(edict_t * ent , edict_t * other , edict_t * activator )  \dday\source/g_target.c:99
"Use_Target_Speaker", (void (*))Use_Target_Speaker,   // void Use_Target_Speaker(edict_t * ent , edict_t * other , edict_t * activator )  \dday\source/g_target.c:39
"Use_Target_Tent", (void (*))Use_Target_Tent,   // void Use_Target_Tent(edict_t * ent , edict_t * other , edict_t * activator )  \dday\source/g_target.c:7
"Use_Weapon", (void (*))Use_Weapon,   // void Use_Weapon(edict_t * ent , gitem_t * item )  \dday\source/p_weapon.c:273
"ValidateSelectedItem", (void (*))ValidateSelectedItem,   // void ValidateSelectedItem(edict_t * ent )  \dday\source/g_cmds.c:209
"VectorCompare", (void (*))VectorCompare,   // int VectorCompare(vec3_t v1 , vec3_t v2 )  \dday\source/q_shared.c:653
"VectorInverse", (void (*))VectorInverse,   // void VectorInverse(vec3_t v )  \dday\source/q_shared.c:756
"VectorLength", (void (*))VectorLength,   // vec_t VectorLength(vec3_t v )  \dday\source/q_shared.c:743
"VectorMA", (void (*))VectorMA,   // void VectorMA(vec3_t veca , float scale , vec3_t vecb , vec3_t vecc )  \dday\source/q_shared.c:700
"VectorNormalize", (void (*))VectorNormalize,   // vec_t VectorNormalize(vec3_t v )  \dday\source/q_shared.c:662
"VectorNormalize2", (void (*))VectorNormalize2,   // vec_t VectorNormalize2(vec3_t v , vec3_t out )  \dday\source/q_shared.c:681
"VectorScale", (void (*))VectorScale,   // void VectorScale(vec3_t in , vec_t scale , vec3_t out )  \dday\source/q_shared.c:763
"VelocityForDamage", (void (*))VelocityForDamage,   // void VelocityForDamage(int damage , vec3_t v )  \dday\source/g_misc.c:39
"Weapon_Antidote", (void (*))Weapon_Antidote,   // void Weapon_Antidote(edict_t * ent )  \dday\source/p_weapon.c:777
"Weapon_Antidote_Use", (void (*))Weapon_Antidote_Use,   // void Weapon_Antidote_Use(edict_t * ent )  \dday\source/p_weapon.c:770
"Weapon_Bandage", (void (*))Weapon_Bandage,   // void Weapon_Bandage(edict_t * ent )  \dday\source/p_weapon.c:855
"Weapon_Bandage_Use", (void (*))Weapon_Bandage_Use,   // void Weapon_Bandage_Use(edict_t * ent )  \dday\source/p_weapon.c:827
"Weapon_Binoculars", (void (*))Weapon_Binoculars,   // void Weapon_Binoculars(edict_t * ent )  \dday\source/p_weapon.c:753
"Weapon_Binoculars_Look", (void (*))Weapon_Binoculars_Look,   // void Weapon_Binoculars_Look(edict_t * ent )  \dday\source/p_weapon.c:737
"Weapon_Flamethrower", (void (*))Weapon_Flamethrower,   // void Weapon_Flamethrower(edict_t * ent )  \dday\source/p_weapon.c:944
"Weapon_Generic", (void (*))Weapon_Generic,   // void Weapon_Generic(edict_t * ent , int FRAME_ACTIVATE_LAST , int FRAME_LFIRE_LAST , int FRAME_LIDLE_LAST ,  int FRAME_RELOAD_LAST , int FRAME_LASTRD_LAST , int FRAME_DEACTIVATE_LAST ,  int FRAME_RAISE_LAST , int FRAME_AFIRE_LAST , int FRAME_AIDLE_LAST ,  int * pause_frames , int * fire_frames , void ( * fire ) ( edict_t * ent ) )  \dday\source/p_generic_wep.c:29
"Weapon_Grenade", (void (*))Weapon_Grenade,   // void Weapon_Grenade(edict_t * ent )  \dday\source/p_weapon.c:374
"Weapon_HMG_Fire", (void (*))Weapon_HMG_Fire,   // void Weapon_HMG_Fire(edict_t * ent )  \dday\source/g_weapon.c:1294
"Weapon_Knife", (void (*))Weapon_Knife,   // void Weapon_Knife(edict_t * ent )  \dday\source/p_weapon.c:725
"Weapon_Knife_Fire", (void (*))Weapon_Knife_Fire,   // void Weapon_Knife_Fire(edict_t * ent )  \dday\source/p_weapon.c:682
"Weapon_LMG_Fire", (void (*))Weapon_LMG_Fire,   // void Weapon_LMG_Fire(edict_t * ent )  \dday\source/g_weapon.c:1213
"Weapon_Mine", (void (*))Weapon_Mine,   // void Weapon_Mine(edict_t * ent )  \dday\source/p_weapon.c:535
"Weapon_Morphine", (void (*))Weapon_Morphine,   // void Weapon_Morphine(edict_t * ent )  \dday\source/p_weapon.c:817
"Weapon_Morphine_Use", (void (*))Weapon_Morphine_Use,   // void Weapon_Morphine_Use(edict_t * ent )  \dday\source/p_weapon.c:790
"Weapon_Pistol_Fire", (void (*))Weapon_Pistol_Fire,   // void Weapon_Pistol_Fire(edict_t * ent )  \dday\source/g_weapon.c:982
"Weapon_Rifle_Fire", (void (*))Weapon_Rifle_Fire,   // void Weapon_Rifle_Fire(edict_t * ent )  \dday\source/g_weapon.c:1067
"Weapon_Rocket_Fire", (void (*))Weapon_Rocket_Fire,   // void Weapon_Rocket_Fire(edict_t * ent )  \dday\source/g_weapon.c:1421
"Weapon_Sniper_Fire", (void (*))Weapon_Sniper_Fire,   // void Weapon_Sniper_Fire(edict_t * ent )  \dday\source/g_weapon.c:1465
"Weapon_Submachinegun_Fire", (void (*))Weapon_Submachinegun_Fire,   // void Weapon_Submachinegun_Fire(edict_t * ent )  \dday\source/g_weapon.c:1127
"WriteClient", (void (*))WriteClient,   // void WriteClient(FILE * f , gclient_t * client )  \dday\source/g_save.c:374
"WriteEdict", (void (*))WriteEdict,   // void WriteEdict(FILE * f , edict_t * ent )  \dday\source/g_save.c:498
"WriteField1", (void (*))WriteField1,   // void WriteField1(FILE * f , field_t * field , byte * base )  \dday\source/g_save.c:232
"WriteField2", (void (*))WriteField2,   // void WriteField2(FILE * f , field_t * field , byte * base )  \dday\source/g_save.c:283
"WriteGame", (void (*))WriteGame,   // void WriteGame(char * filename , qboolean autosave )  \dday\source/g_save.c:431
"WriteLevel", (void (*))WriteLevel,   // void WriteLevel(char * filename )  \dday\source/g_save.c:599
"WriteLevelLocals", (void (*))WriteLevelLocals,   // void WriteLevelLocals(FILE * f )  \dday\source/g_save.c:530
"_DotProduct", (void (*))_DotProduct,   // vec_t _DotProduct(vec3_t v1 , vec3_t v2 )  \dday\source/q_shared.c:708
"_VectorAdd", (void (*))_VectorAdd,   // void _VectorAdd(vec3_t veca , vec3_t vecb , vec3_t out )  \dday\source/q_shared.c:720
"_VectorCopy", (void (*))_VectorCopy,   // void _VectorCopy(vec3_t in , vec3_t out )  \dday\source/q_shared.c:727
"_VectorSubtract", (void (*))_VectorSubtract,   // void _VectorSubtract(vec3_t veca , vec3_t vecb , vec3_t out )  \dday\source/q_shared.c:713
"actorMachineGun", (void (*))actorMachineGun,   // void actorMachineGun(edict_t * self )  \dday\source/m_actor.c:250
"actor_attack", (void (*))actor_attack,   // void actor_attack(edict_t * self )  \dday\source/m_actor.c:372
"actor_dead", (void (*))actor_dead,   // void actor_dead(edict_t * self )  \dday\source/m_actor.c:280
"actor_die", (void (*))actor_die,   // void actor_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/m_actor.c:320
"actor_fire", (void (*))actor_fire,   // void actor_fire(edict_t * self )  \dday\source/m_actor.c:353
"actor_pain", (void (*))actor_pain,   // void actor_pain(edict_t * self , edict_t * other , float kick , int damage )  \dday\source/m_actor.c:211
"actor_run", (void (*))actor_run,   // void actor_run(edict_t * self )  \dday\source/m_actor.c:117
"actor_stand", (void (*))actor_stand,   // void actor_stand(edict_t * self )  \dday\source/m_actor.c:68
"actor_use", (void (*))actor_use,   // void actor_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/m_actor.c:382
"actor_walk", (void (*))actor_walk,   // void actor_walk(edict_t * self )  \dday\source/m_actor.c:94
//"ai_charge", (void (*))ai_charge,   // void ai_charge(edict_t * self , float dist )  \dday\source/g_ai.c:175
//"ai_checkattack", (void (*))ai_checkattack,   // qboolean ai_checkattack(edict_t * self , float dist )  \dday\source/g_ai.c:752
//"ai_move", (void (*))ai_move,   // void ai_move(edict_t * self , float dist )  \dday\source/g_ai.c:73
//"ai_run", (void (*))ai_run,   // void ai_run(edict_t * self , float dist )  \dday\source/g_ai.c:895
//"ai_run_melee", (void (*))ai_run_melee,   // void ai_run_melee(edict_t * self )  \dday\source/g_ai.c:684
//"ai_run_missile", (void (*))ai_run_missile,   // void ai_run_missile(edict_t * self )  \dday\source/g_ai.c:704
//"ai_run_slide", (void (*))ai_run_slide,   // void ai_run_slide(edict_t * self , float distance )  \dday\source/g_ai.c:724
//"ai_stand", (void (*))ai_stand,   // void ai_stand(edict_t * self , float dist )  \dday\source/g_ai.c:87
//"ai_turn", (void (*))ai_turn,   // void ai_turn(edict_t * self , float dist )  \dday\source/g_ai.c:196
//"ai_walk", (void (*))ai_walk,   // void ai_walk(edict_t * self , float dist )  \dday\source/g_ai.c:144
"anglemod", (void (*))anglemod,   // float anglemod(float a )  \dday\source/q_shared.c:274
"barrel_delay", (void (*))barrel_delay,   // void barrel_delay(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/g_misc.c:941
"barrel_explode", (void (*))barrel_explode,   // void barrel_explode(edict_t * self )  \dday\source/g_misc.c:862
"barrel_touch", (void (*))barrel_touch,   // void barrel_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_misc.c:849
"blaster_touch", (void (*))blaster_touch,   // void blaster_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_weapon.c:394
"body_die", (void (*))body_die,   // void body_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/p_client.c:945
"booldummy", (void (*))booldummy,   // qboolean booldummy(struct edict_s * i , struct edict_s * ii )  \dday\source/u_entmgr.c:85
"button_done", (void (*))button_done,   // void button_done(edict_t * self )  \dday\source/g_func.c:673
"button_fire", (void (*))button_fire,   // void button_fire(edict_t * self )  \dday\source/g_func.c:707
"button_killed", (void (*))button_killed,   // void button_killed(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/g_func.c:736
"button_return", (void (*))button_return,   // void button_return(edict_t * self )  \dday\source/g_func.c:680
"button_touch", (void (*))button_touch,   // void button_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_func.c:724
"button_use", (void (*))button_use,   // void button_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_func.c:718
"button_wait", (void (*))button_wait,   // void button_wait(edict_t * self )  \dday\source/g_func.c:692
"calcVspread", (void (*))calcVspread,   // int calcVspread(edict_t * ent , trace_t * tr )  \dday\source/g_weapon.c:110
"change_stance", (void (*))change_stance,   // void change_stance(edict_t * self , int stance )  \dday\source/g_cmds.c:1043
//"check_dodge", (void (*))check_dodge,   // static void check_dodge(edict_t * self , vec3_t start , vec3_t dir , int speed )  \dday\source/g_weapon.c:15
"check_firedodge", (void (*))check_firedodge,   // void check_firedodge(edict_t * self , vec3_t start , vec3_t dir , int speed )  \dday\source/x_fire.c:54
"commander_body_drop", (void (*))commander_body_drop,   // void commander_body_drop(edict_t * self )  \dday\source/g_misc.c:1144
"commander_body_think", (void (*))commander_body_think,   // void commander_body_think(edict_t * self )  \dday\source/g_misc.c:1126
"commander_body_use", (void (*))commander_body_use,   // void commander_body_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_misc.c:1137
"debris_die", (void (*))debris_die,   // void debris_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/g_misc.c:257
"door_blocked", (void (*))door_blocked,   // void door_blocked(edict_t * self , edict_t * other )  \dday\source/g_func.c:1057
"door_go_down", (void (*))door_go_down,   // void door_go_down(edict_t * self )  \dday\source/g_func.c:881
"door_go_up", (void (*))door_go_up,   // void door_go_up(edict_t * self , edict_t * activator )  \dday\source/g_func.c:902
"door_hit_bottom", (void (*))door_hit_bottom,   // void door_hit_bottom(edict_t * self )  \dday\source/g_func.c:869
"door_hit_top", (void (*))door_hit_top,   // void door_hit_top(edict_t * self )  \dday\source/g_func.c:851
"door_killed", (void (*))door_killed,   // void door_killed(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/g_func.c:1094
"door_secret_blocked", (void (*))door_secret_blocked,   // void door_secret_blocked(edict_t * self , edict_t * other )  \dday\source/g_func.c:1921
"door_secret_die", (void (*))door_secret_die,   // void door_secret_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/g_func.c:1940
"door_secret_done", (void (*))door_secret_done,   // void door_secret_done(edict_t * self )  \dday\source/g_func.c:1911
"door_secret_move1", (void (*))door_secret_move1,   // void door_secret_move1(edict_t * self )  \dday\source/g_func.c:1876
"door_secret_move2", (void (*))door_secret_move2,   // void door_secret_move2(edict_t * self )  \dday\source/g_func.c:1882
"door_secret_move3", (void (*))door_secret_move3,   // void door_secret_move3(edict_t * self )  \dday\source/g_func.c:1887
"door_secret_move4", (void (*))door_secret_move4,   // void door_secret_move4(edict_t * self )  \dday\source/g_func.c:1895
"door_secret_move5", (void (*))door_secret_move5,   // void door_secret_move5(edict_t * self )  \dday\source/g_func.c:1900
"door_secret_move6", (void (*))door_secret_move6,   // void door_secret_move6(edict_t * self )  \dday\source/g_func.c:1906
"door_secret_use", (void (*))door_secret_use,   // void door_secret_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_func.c:1866
"door_touch", (void (*))door_touch,   // void door_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_func.c:1106
"door_use", (void (*))door_use,   // void door_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_func.c:930
"door_use_areaportals", (void (*))door_use_areaportals,   // void door_use_areaportals(edict_t * self , qboolean open )  \dday\source/g_func.c:833
// "dopromote", (void (*))dopromote,   // void dopromote(TeamS_t * team , edict_t * ent )  \dday\source/g_combat.c:14
//"drop_make_touchable", (void (*))drop_make_touchable,   // static void drop_make_touchable(edict_t * ent )  \dday\source/g_items.c:743
//"drop_temp_touch", (void (*))drop_temp_touch,   // static void drop_temp_touch(edict_t * ent , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_items.c:735
"droptofloor", (void (*))droptofloor,   // void droptofloor(edict_t * ent )  \dday\source/g_items.c:828
"dummy1", (void (*))dummy1,   // void dummy1(struct edict_s * i , struct gitem_s * ii )  \dday\source/u_entmgr.c:90
"dummy2", (void (*))dummy2,   // void dummy2(struct edict_s * i )  \dday\source/u_entmgr.c:94
"findradius", (void (*))findradius,   // edict_t * findradius(edict_t * from , vec3_t org , float rad )  \dday\source/g_utils.c:61
"fire_Knife", (void (*))fire_Knife,   // void fire_Knife(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int kick )  \dday\source/p_weapon.c:550
"fire_blaster", (void (*))fire_blaster,   // void fire_blaster(edict_t * self , vec3_t start , vec3_t dir , int damage , int speed , int effect , qboolean hyper )  \dday\source/g_weapon.c:433
"fire_bullet", (void (*))fire_bullet,   // void fire_bullet(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int kick , int hspread , int vspread , int mod , qboolean tracers_on )  \dday\source/g_weapon.c:288
"fire_flamegrenade", (void (*))fire_flamegrenade,   // void fire_flamegrenade(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int speed , float timer , float damage_radius )  \dday\source/x_fbomb.c:132
"fire_flamegrenade2", (void (*))fire_flamegrenade2,   // void fire_flamegrenade2(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int speed , float timer , float damage_radius , qboolean held )  \dday\source/x_fbomb.c:169
"fire_flamerocket", (void (*))fire_flamerocket,   // void fire_flamerocket(edict_t * self , vec3_t start , vec3_t dir , int damage , int speed , float damage_radius , int radius_damage )  \dday\source/x_fbomb.c:291
"fire_fragment", (void (*))fire_fragment,   // void fire_fragment(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int kick , int hspread , int vspread , int mod )  \dday\source/g_weapon.c:272
//"fire_grenade", (void (*))fire_grenade,   // void fire_grenade(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int speed , float timer , float damage_radius )  \dday\source/g_weapon.c:605
"fire_grenade2", (void (*))fire_grenade2,   // void fire_grenade2(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int speed , float timer , float damage_radius , qboolean held )  \dday\source/g_weapon.c:639
"fire_hit", (void (*))fire_hit,   // qboolean fire_hit(edict_t * self , vec3_t aim , int damage , int kick )  \dday\source/g_weapon.c:46
// "fire_lead", (void (*))fire_lead,   // static void fire_lead(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int kick , int te_impact , int hspread , int vspread , int mod , qboolean calcv )  \dday\source/g_weapon.c:128
"fire_rifle", (void (*))fire_rifle,   // void fire_rifle(edict_t * self , vec3_t start , vec3_t aimdir , int damage , int kick )  \dday\source/g_weapon.c:812
"fire_rocket", (void (*))fire_rocket,   // void fire_rocket(edict_t * self , vec3_t start , vec3_t dir , int damage , int speed , float damage_radius , int radius_damage )  \dday\source/g_weapon.c:744
"fire_shell", (void (*))fire_shell,   // void fire_shell(edict_t * self , vec3_t start , vec3_t dir , int damage , int speed , float damage_radius , int radius_damage )  \dday\source/g_weapon.c:776
"fire_tracer", (void (*))fire_tracer,   // void fire_tracer(edict_t * self , vec3_t start , vec3_t dir , int damage )  \dday\source/g_weapon.c:346
"firerocket_touch", (void (*))firerocket_touch,   // void firerocket_touch(edict_t * ent , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/x_fbomb.c:220
//"fix_mos", (void (*))fix_mos,   // int fix_mos(char * argv )  \dday\source/g_cmds.c:1310
"flymonster_start", (void (*))flymonster_start,   // void flymonster_start(edict_t * self )  \dday\source/g_monster.c:636
"flymonster_start_go", (void (*))flymonster_start_go,   // void flymonster_start_go(edict_t * self )  \dday\source/g_monster.c:620
// "func_clock_format_countdown", (void (*))func_clock_format_countdown,   // static void func_clock_format_countdown(edict_t * self )  \dday\source/g_misc.c:1636
// "func_clock_reset", (void (*))func_clock_reset,   // static void func_clock_reset(edict_t * self )  \dday\source/g_misc.c:1621
"func_clock_think", (void (*))func_clock_think,   // void func_clock_think(edict_t * self )  \dday\source/g_misc.c:1663
"func_clock_use", (void (*))func_clock_use,   // void func_clock_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_misc.c:1728
"func_conveyor_use", (void (*))func_conveyor_use,   // void func_conveyor_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_func.c:1805
"func_explosive_explode", (void (*))func_explosive_explode,   // void func_explosive_explode(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/g_misc.c:720
"func_explosive_spawn", (void (*))func_explosive_spawn,   // void func_explosive_spawn(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_misc.c:789
"func_explosive_use", (void (*))func_explosive_use,   // void func_explosive_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_misc.c:784
"func_object_release", (void (*))func_object_release,   // void func_object_release(edict_t * self )  \dday\source/g_misc.c:652
"func_object_touch", (void (*))func_object_touch,   // void func_object_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_misc.c:640
"func_object_use", (void (*))func_object_use,   // void func_object_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_misc.c:658
"func_timer_think", (void (*))func_timer_think,   // void func_timer_think(edict_t * self )  \dday\source/g_func.c:1751
"func_timer_use", (void (*))func_timer_use,   // void func_timer_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_func.c:1757
"func_train_find", (void (*))func_train_find,   // void func_train_find(edict_t * self )  \dday\source/g_func.c:1581
"func_wall_use", (void (*))func_wall_use,   // void func_wall_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_misc.c:568
"gib_die", (void (*))gib_die,   // void gib_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/g_misc.c:111
"gib_think", (void (*))gib_think,   // void gib_think(edict_t * self )  \dday\source/g_misc.c:73
"gib_touch", (void (*))gib_touch,   // void gib_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_misc.c:85
"hurt_touch", (void (*))hurt_touch,   // void hurt_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_trigger.c:449
"hurt_use", (void (*))hurt_use,   // void hurt_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_trigger.c:436
"ifchangewep", (void (*))ifchangewep,   // void ifchangewep(edict_t * ent )  \dday\source/p_generic_wep.c:305
"infront", (void (*))infront,   // qboolean infront(edict_t * self , edict_t * other )  \dday\source/g_ai.c:293
"insane_checkdown", (void (*))insane_checkdown,   // void insane_checkdown(edict_t * self )  \dday\source/m_insane.c:505
"insane_checkup", (void (*))insane_checkup,   // void insane_checkup(edict_t * self )  \dday\source/m_insane.c:517
"insane_cross", (void (*))insane_cross,   // void insane_cross(edict_t * self )  \dday\source/m_insane.c:415
"insane_dead", (void (*))insane_dead,   // void insane_dead(edict_t * self )  \dday\source/m_insane.c:544
"insane_die", (void (*))insane_die,   // void insane_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/m_insane.c:562
"insane_fist", (void (*))insane_fist,   // void insane_fist(edict_t * self )  \dday\source/m_insane.c:18
"insane_moan", (void (*))insane_moan,   // void insane_moan(edict_t * self )  \dday\source/m_insane.c:28
"insane_onground", (void (*))insane_onground,   // void insane_onground(edict_t * self )  \dday\source/m_insane.c:500
"insane_pain", (void (*))insane_pain,   // void insane_pain(edict_t * self , edict_t * other , float kick , int damage )  \dday\source/m_insane.c:458
"insane_run", (void (*))insane_run,   // void insane_run(edict_t * self )  \dday\source/m_insane.c:440
"insane_scream", (void (*))insane_scream,   // void insane_scream(edict_t * self )  \dday\source/m_insane.c:33
"insane_shake", (void (*))insane_shake,   // void insane_shake(edict_t * self )  \dday\source/m_insane.c:23
"insane_stand", (void (*))insane_stand,   // void insane_stand(edict_t * self )  \dday\source/m_insane.c:527
"insane_walk", (void (*))insane_walk,   // void insane_walk(edict_t * self )  \dday\source/m_insane.c:423
// "light_use", (void (*))light_use,   // static void light_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_misc.c:520
"misc_banner_think", (void (*))misc_banner_think,   // void misc_banner_think(edict_t * ent )  \dday\source/g_misc.c:1176
//bcass start - banner thing
"misc_banner_x_think", (void (*))misc_banner_x_think,
//bcass end
"misc_blackhole_think", (void (*))misc_blackhole_think,   // void misc_blackhole_think(edict_t * self )  \dday\source/g_misc.c:1012
"misc_blackhole_use", (void (*))misc_blackhole_use,   // void misc_blackhole_use(edict_t * ent , edict_t * other , edict_t * activator )  \dday\source/g_misc.c:1001
"misc_deadsoldier_die", (void (*))misc_deadsoldier_die,   // void misc_deadsoldier_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/g_misc.c:1197
"misc_easterchick2_think", (void (*))misc_easterchick2_think,   // void misc_easterchick2_think(edict_t * self )  \dday\source/g_misc.c:1096
"misc_easterchick_think", (void (*))misc_easterchick_think,   // void misc_easterchick_think(edict_t * self )  \dday\source/g_misc.c:1068
"misc_eastertank_think", (void (*))misc_eastertank_think,   // void misc_eastertank_think(edict_t * self )  \dday\source/g_misc.c:1040
"misc_satellite_dish_think", (void (*))misc_satellite_dish_think,   // void misc_satellite_dish_think(edict_t * self )  \dday\source/g_misc.c:1433
"misc_satellite_dish_use", (void (*))misc_satellite_dish_use,   // void misc_satellite_dish_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_misc.c:1440
"misc_strogg_ship_use", (void (*))misc_strogg_ship_use,   // void misc_strogg_ship_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_misc.c:1396
"misc_viper_bomb_prethink", (void (*))misc_viper_bomb_prethink,   // void misc_viper_bomb_prethink(edict_t * self )  \dday\source/g_misc.c:1327
"misc_viper_bomb_touch", (void (*))misc_viper_bomb_touch,   // void misc_viper_bomb_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_misc.c:1318
"misc_viper_bomb_use", (void (*))misc_viper_bomb_use,   // void misc_viper_bomb_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_misc.c:1346
"misc_viper_use", (void (*))misc_viper_use,   // void misc_viper_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_misc.c:1266
"monster_death_use", (void (*))monster_death_use,   // void monster_death_use(edict_t * self )  \dday\source/g_monster.c:421
"monster_start", (void (*))monster_start,   // qboolean monster_start(edict_t * self )  \dday\source/g_monster.c:444
"monster_start_go", (void (*))monster_start_go,   // void monster_start_go(edict_t * self )  \dday\source/g_monster.c:504
"monster_think", (void (*))monster_think,   // void monster_think(edict_t * self )  \dday\source/g_monster.c:329
"monster_triggered_spawn", (void (*))monster_triggered_spawn,   // void monster_triggered_spawn(edict_t * self )  \dday\source/g_monster.c:370
"monster_triggered_spawn_use", (void (*))monster_triggered_spawn_use,   // void monster_triggered_spawn_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_monster.c:393
"monster_triggered_start", (void (*))monster_triggered_start,   // void monster_triggered_start(edict_t * self )  \dday\source/g_monster.c:403
"monster_use", (void (*))monster_use,   // void monster_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_monster.c:350
"multi_trigger", (void (*))multi_trigger,   // void multi_trigger(edict_t * ent )  \dday\source/g_trigger.c:26
"multi_wait", (void (*))multi_wait,   // void multi_wait(edict_t * ent )  \dday\source/g_trigger.c:17
"path_corner_touch", (void (*))path_corner_touch,   // void path_corner_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_misc.c:318
"plat_Accelerate", (void (*))plat_Accelerate,   // void plat_Accelerate(moveinfo_t * moveinfo )  \dday\source/g_func.c:244
"plat_CalcAcceleratedMove", (void (*))plat_CalcAcceleratedMove,   // void plat_CalcAcceleratedMove(moveinfo_t * moveinfo )  \dday\source/g_func.c:216
"plat_blocked", (void (*))plat_blocked,   // void plat_blocked(edict_t * self , edict_t * other )  \dday\source/g_func.c:388
"plat_go_down", (void (*))plat_go_down,   // void plat_go_down(edict_t * ent )  \dday\source/g_func.c:364
"plat_go_up", (void (*))plat_go_up,   // void plat_go_up(edict_t * ent )  \dday\source/g_func.c:376
"plat_hit_bottom", (void (*))plat_hit_bottom,   // void plat_hit_bottom(edict_t * ent )  \dday\source/g_func.c:353
"plat_hit_top", (void (*))plat_hit_top,   // void plat_hit_top(edict_t * ent )  \dday\source/g_func.c:339
"plat_spawn_inside_trigger", (void (*))plat_spawn_inside_trigger,   // void plat_spawn_inside_trigger(edict_t * ent )  \dday\source/g_func.c:432
"player_die", (void (*))player_die,   // void player_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/p_client.c:488
"player_pain", (void (*))player_pain,   // void player_pain(edict_t * self , edict_t * other , float kick , int damage )  \dday\source/p_client.c:177
"point_combat_touch", (void (*))point_combat_touch,   // void point_combat_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_misc.c:397
//"promoteofficer", (void (*))promoteofficer,   // void promoteofficer(edict_t * ent )  \dday\source/g_combat.c:24
"range", (void (*))range,   // int range(edict_t * self , edict_t * other )  \dday\source/g_ai.c:245
"reinforcement_think", (void (*))reinforcement_think,   // void reinforcement_think(edict_t * ent )  \dday\source/g_func.c:2032
"respawn", (void (*))respawn,   // void respawn(edict_t * self )  \dday\source/p_client.c:1021
"rocket_touch", (void (*))rocket_touch,   // void rocket_touch(edict_t * ent , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_weapon.c:693
"rotating_blocked", (void (*))rotating_blocked,   // void rotating_blocked(edict_t * self , edict_t * other )  \dday\source/g_func.c:576
"rotating_touch", (void (*))rotating_touch,   // void rotating_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_func.c:581
"rotating_use", (void (*))rotating_use,   // void rotating_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_func.c:587
"stuffcmd", (void (*))stuffcmd,   // void stuffcmd(edict_t * ent , char * s )  \dday\source/g_main.c:84
"swimmonster_start", (void (*))swimmonster_start,   // void swimmonster_start(edict_t * self )  \dday\source/g_monster.c:656
"swimmonster_start_go", (void (*))swimmonster_start_go,   // void swimmonster_start_go(edict_t * self )  \dday\source/g_monster.c:644
"target_actor_touch", (void (*))target_actor_touch,   // void target_actor_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/m_actor.c:477
"target_crosslevel_target_think", (void (*))target_crosslevel_target_think,   // void target_crosslevel_target_think(edict_t * self )  \dday\source/g_target.c:450
"target_earthquake_think", (void (*))target_earthquake_think,   // void target_earthquake_think(edict_t * self )  \dday\source/g_target.c:736
"target_earthquake_use", (void (*))target_earthquake_use,   // void target_earthquake_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_target.c:766
"target_explosion_explode", (void (*))target_explosion_explode,   // void target_explosion_explode(edict_t * self )  \dday\source/g_target.c:208
"target_laser_off", (void (*))target_laser_off,   // void target_laser_off(edict_t * self )  \dday\source/g_target.c:550
"target_laser_on", (void (*))target_laser_on,   // void target_laser_on(edict_t * self )  \dday\source/g_target.c:541
"target_laser_start", (void (*))target_laser_start,   // void target_laser_start(edict_t * self )  \dday\source/g_target.c:566
"target_laser_think", (void (*))target_laser_think,   // void target_laser_think(edict_t * self )  \dday\source/g_target.c:476
"target_laser_use", (void (*))target_laser_use,   // void target_laser_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_target.c:557
"target_lightramp_think", (void (*))target_lightramp_think,   // void target_lightramp_think(edict_t * self )  \dday\source/g_target.c:637
"target_lightramp_use", (void (*))target_lightramp_use,   // void target_lightramp_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_target.c:660
"target_objective_use", (void (*))target_objective_use,   // void target_objective_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_ents.c:108
"target_string_use", (void (*))target_string_use,   // void target_string_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_misc.c:1565
"teleporter_touch", (void (*))teleporter_touch,   // void teleporter_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_misc.c:1771
"tracer_touch", (void (*))tracer_touch,   // void tracer_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_weapon.c:310
"train_blocked", (void (*))train_blocked,   // void train_blocked(edict_t * self , edict_t * other )  \dday\source/g_func.c:1442
"train_next", (void (*))train_next,   // void train_next(edict_t * self )  \dday\source/g_func.c:1510
"train_resume", (void (*))train_resume,   // void train_resume(edict_t * self )  \dday\source/g_func.c:1566
"train_use", (void (*))train_use,   // void train_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_func.c:1613
"train_wait", (void (*))train_wait,   // void train_wait(edict_t * self )  \dday\source/g_func.c:1463
"trigger_counter_use", (void (*))trigger_counter_use,   // void trigger_counter_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_trigger.c:307
"trigger_crosslevel_trigger_use", (void (*))trigger_crosslevel_trigger_use,   // void trigger_crosslevel_trigger_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_target.c:432
"trigger_elevator_init", (void (*))trigger_elevator_init,   // void trigger_elevator_init(edict_t * self )  \dday\source/g_func.c:1706
"trigger_elevator_use", (void (*))trigger_elevator_use,   // void trigger_elevator_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_func.c:1679
"trigger_enable", (void (*))trigger_enable,   // void trigger_enable(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_trigger.c:92
"trigger_enough_troops_use", (void (*))trigger_enough_troops_use,   // void trigger_enough_troops_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_ents.c:25
"trigger_gravity_touch", (void (*))trigger_gravity_touch,   // void trigger_gravity_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_trigger.c:513
"trigger_key_use", (void (*))trigger_key_use,   // void trigger_key_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_trigger.c:193
"trigger_monsterjump_touch", (void (*))trigger_monsterjump_touch,   // void trigger_monsterjump_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_trigger.c:547
"trigger_push_touch", (void (*))trigger_push_touch,   // void trigger_push_touch(edict_t * self , edict_t * other , cplane_t * plane , csurface_t * surf )  \dday\source/g_trigger.c:375
"trigger_relay_use", (void (*))trigger_relay_use,   // void trigger_relay_use(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_trigger.c:170
"turret_blocked", (void (*))turret_blocked,   // void turret_blocked(edict_t * self , edict_t * other )  \dday\source/g_turret.c:29
"turret_breach_finish_init", (void (*))turret_breach_finish_init,   // void turret_breach_finish_init(edict_t * self )  \dday\source/g_turret.c:182
"turret_breach_fire", (void (*))turret_breach_fire,   // void turret_breach_fire(edict_t * self )  \dday\source/g_turret.c:59
"turret_breach_think", (void (*))turret_breach_think,   // void turret_breach_think(edict_t * self )  \dday\source/g_turret.c:77
"turret_driver_die", (void (*))turret_driver_die,   // void turret_driver_die(edict_t * self , edict_t * inflictor , edict_t * attacker , int damage , vec3_t point )  \dday\source/g_turret.c:259
"turret_driver_link", (void (*))turret_driver_link,   // void turret_driver_link(edict_t * self )  \dday\source/g_turret.c:335
"turret_driver_think", (void (*))turret_driver_think,   // void turret_driver_think(edict_t * self )  \dday\source/g_turret.c:281
"tv", (void (*))tv,   // float * tv(float x , float y , float z )  \dday\source/g_utils.c:249
"use_killbox", (void (*))use_killbox,   // void use_killbox(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_func.c:2017
"use_target_blaster", (void (*))use_target_blaster,   // void use_target_blaster(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_target.c:397
"use_target_changelevel", (void (*))use_target_changelevel,   // void use_target_changelevel(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_target.c:251
"use_target_explosion", (void (*))use_target_explosion,   // void use_target_explosion(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_target.c:225
"use_target_goal", (void (*))use_target_goal,   // void use_target_goal(edict_t * ent , edict_t * other , edict_t * activator )  \dday\source/g_target.c:170
"use_target_secret", (void (*))use_target_secret,   // void use_target_secret(edict_t * ent , edict_t * other , edict_t * activator )  \dday\source/g_target.c:135
"use_target_spawner", (void (*))use_target_spawner,   // void use_target_spawner(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_target.c:361
"use_target_splash", (void (*))use_target_splash,   // void use_target_splash(edict_t * self , edict_t * other , edict_t * activator )  \dday\source/g_target.c:319
"va", (void (*))va,   // char * va(char * format , ... )  \dday\source/q_shared.c:1031
"vectoangles", (void (*))vectoangles,   // void vectoangles(vec3_t value1 , vec3_t angles )  \dday\source/g_utils.c:333
"vectoyaw", (void (*))vectoyaw,   // float vectoyaw(vec3_t vec )  \dday\source/g_utils.c:316
"visible", (void (*))visible,   // qboolean visible(edict_t * self , edict_t * other )  \dday\source/g_ai.c:268
"vtos", (void (*))vtos,   // char * vtos(vec3_t v )  \dday\source/g_utils.c:276
"walkmonster_start", (void (*))walkmonster_start,   // void walkmonster_start(edict_t * self )  \dday\source/g_monster.c:613
"walkmonster_start_go", (void (*))walkmonster_start_go,   // void walkmonster_start_go(edict_t * self )  \dday\source/g_monster.c:592
"weapon_flame_fire", (void (*))weapon_flame_fire,   // void weapon_flame_fire(edict_t * ent )  \dday\source/p_weapon.c:874
"weapon_grenade_fire", (void (*))weapon_grenade_fire,   // void weapon_grenade_fire(edict_t * ent , qboolean held )  \dday\source/p_weapon.c:349
"weapon_grenadelauncher_fire", (void (*))weapon_grenadelauncher_fire,   // void weapon_grenadelauncher_fire(edict_t * ent )  \dday\source/p_weapon.c:504
"", NULL    //End of functions
};
#define  FUNARRAYLEN   760
