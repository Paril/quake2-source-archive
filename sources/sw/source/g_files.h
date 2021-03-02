
//===================
// B_AI.C
//===================
float Bot_Calc_Path_Cost (counter_t start, counter_t end);
float Bot_Find_Item_Weight (item_table_t *item);
qboolean Bot_Needs_Item (edict_t *self, item_table_t *item);
qboolean Bot_ChangeWeapon (edict_t *ent, gitem_t *item);
void Bot_Choose_Weapon (edict_t *self);
void Bot_Attack (edict_t *self, edict_t *other, usercmd_t *ucmd);
void Bot_Move (edict_t *self, usercmd_t *ucmd);
counter_t Bot_Find_Nearest_Node (edict_t *self, counter_t ignore);
void Bot_Check_Node_Dist (edict_t *self);
void Bot_Turn_To_Goal (edict_t *self);
int Bot_Find_Enemy (edict_t *self);
void Bot_Find_Roam_Goal (edict_t *self);
void Bot_CTF_Think (edict_t *self);
void Bot_Say_Dead (edict_t *self);
void Bot_Intermission_Think (edict_t *self);
void Bot_Give (edict_t *ent, char *name);
void Bot_AI_Think (edict_t *self);

//===================
// B_SPAWN.C
//===================
void Bot_respawn (edict_t *self);
void Bot_PutClientInServer (edict_t *bot, qboolean respawn, int team);
edict_t *Bot_Find_Free_Client (void);
void Bot_Set_Data(edict_t *bot, char *name, char *skin, char *team);
void Setup_Bot (edict_t *bot);
int ValidateSkin (char *skin);
void Spawn_New_Bot (char *name, char *skin);
void Spawn_Bot (char *name, char *skin);

//===================
// FOG.C
//===================
void SP_trigger_fog (edict_t *self);
void SP_trigger_smooth_fog (edict_t *ent);
void Fog_Update (edict_t *self);

//===================
// G_AI.C
//===================
qboolean visible_vec (edict_t *self, vec3_t vec, vec3_t vec2);
void spawn_temp (vec3_t spot1, int c);
void spawn_templaser (vec3_t spot1, vec3_t spot2);
void AI_SetSightClient (void);
void ai_move (edict_t *self, float dist);
void ai_hold (edict_t *self, float dist);
void ai_stand (edict_t *self, float dist);
void ai_walk (edict_t *self, float dist);
void ai_charge (edict_t *self, float dist);
void ai_turn (edict_t *self, float dist);
int range (edict_t *self, edict_t *other);
qboolean visible (edict_t *self, edict_t *other);
qboolean infront (edict_t *self, edict_t *other);
void HuntTarget (edict_t *self);
void FoundTarget (edict_t *self);
qboolean FindTarget (edict_t *self);
qboolean FacingIdeal(edict_t *self);
qboolean M_CheckAttack (edict_t *self);
void ai_run_melee(edict_t *self);
void ai_run_missile(edict_t *self);
void ai_run_slide(edict_t *self, float distance);
qboolean ai_checkattack (edict_t *self, float dist);
void ai_run (edict_t *self, float dist);

//===================
// G_CHASE.C
//===================
void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);
void GetChaseTarget(edict_t *ent);

//===================
// G_CMDS.C
//===================
void debug_printf(char *fmt, ...);
void safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...);
void safe_centerprintf (edict_t *ent, char *fmt, ...);
void safe_bprintf (int printlevel, char *fmt, ...);
void Cmd_Change_Dir (edict_t *ent);
void Cmd_Cat (edict_t *ent);
void Cmd_Dir (edict_t *ent);
void Cmd_Change_Pass (edict_t *ent);
void Cmd_Exec (edict_t *ent);
void Cmd_sbike (edict_t *ent);
void Cmd_Set_AutoReload(edict_t *ent);
void Cmd_vehicle_f (edict_t *ent);
void cmd_holster (edict_t *ent);
void Cmd_OpenDoor(edict_t *ent);
void Cmd_Secondary_Fire (edict_t *ent);
void Cmd_Set_Force_Useable (edict_t *ent);
void Cmd_Reload (edict_t *ent);
void cmd_force_info (edict_t *ent);
void sniper_zoom (edict_t *ent);
void cmd_Weapon_Menu (edict_t *ent);
void cmd_Force_Menu (edict_t *ent);
char *ClientTeam (edict_t *ent);
qboolean OnSameTeam (edict_t *ent1, edict_t *ent2);
void SelectNextItem (edict_t *ent, int itflags);
void SelectPrevItem (edict_t *ent, int itflags);
void ValidateSelectedItem (edict_t *ent);
void Cmd_Give_f (edict_t *ent);
//RipVTide
void stuffcmd(edict_t *ent, char *s);
void watchmesing (edict_t *ent);
void imabigchicken (edict_t *ent);
void googl3 (edict_t *ent);
void Cmd_Set_Affiliation(edict_t *ent);
void Cmd_RK(edict_t *ent);

void Cmd_God_f (edict_t *ent);
void Cmd_Notarget_f (edict_t *ent);
void Cmd_Noclip_f (edict_t *ent);
void Cmd_Use_f (edict_t *ent);
void Cmd_Drop_f (edict_t *ent);
void Cmd_Inven_f (edict_t *ent);
void Cmd_InvUse_f (edict_t *ent);
void Cmd_LastWeap_f (edict_t *ent);
void Cmd_WeapPrev_f (edict_t *ent);
void Cmd_WeapNext_f (edict_t *ent);
void Cmd_WeapLast_f (edict_t *ent);
void Cmd_InvDrop_f (edict_t *ent);
void Cmd_Kill_f (edict_t *ent);
void Cmd_PutAway_f (edict_t *ent);
int PlayerSort (void const *a, void const *b);
void Cmd_Players_f (edict_t *ent);
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0);
void Cmd_PlayerList_f(edict_t *ent);
void ClientCommand (edict_t *ent);

//===================
// G_COMBAT.C
//===================
qboolean CanDamage (edict_t *targ, edict_t *inflictor);
void Killed (edict_t *targ, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage);
//static int CheckPowerArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int dflags);
//static int CheckArmor (edict_t *ent, vec3_t point, vec3_t normal, int damage, int te_sparks, int dflags);
void M_ReactToDamage (edict_t *targ, edict_t *attacker);
qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker);
void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod);
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod);

//===================
// G_ITEMS.C
//===================
#define	ITEM_INDEX(x) ((x)-itemlist)
void SpawnEmptyClip (edict_t *self, char *filename);
gitem_t	*GetItemByIndex (int index);
gitem_t	*FindItemByClassname (char *classname);
gitem_t	*FindItem (char *pickup_name);
qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count);
void DoRespawn (edict_t *ent);
qboolean Pickup_Powerup (edict_t *ent, edict_t *other);
void SetRespawn (edict_t *ent, float delay);
void Drop_General (edict_t *ent, gitem_t *item);
//int ArmorIndex (edict_t *ent);
//int PowerArmorType (edict_t *ent);
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
edict_t *Drop_Item (edict_t *ent, gitem_t *item);
void Use_Item (edict_t *ent, edict_t *other, edict_t *activator);
void droptofloor (edict_t *ent);
void PrecacheItem (gitem_t *it);
void setup_item_table (void);
void SpawnItem (edict_t *ent, gitem_t *item);
void InitItems (void);
void SetItemNames (void);

//===================
// G_MISC.C
//===================
void SP_func_areaportal (edict_t *ent);
void VelocityForDamage (int damage, vec3_t v);
void ClipGibVelocity (edict_t *ent);
void ThrowGib (edict_t *self, char *gibname, int damage, int type);
void ThrowHead (edict_t *self, char *gibname, int damage, int type);
void ThrowClientHead (edict_t *self, int damage);
void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin);
void SP_path_corner (edict_t *self);
void SP_point_combat (edict_t *self);
void SP_viewthing(edict_t *ent);
void SP_info_null (edict_t *self);
void SP_info_notnull (edict_t *self);
void SP_light (edict_t *self);
void SP_func_wall (edict_t *self);
void SP_func_object (edict_t *self);
void BecomeExplosion1 (edict_t *self);
void BecomeExplosion2 (edict_t *self);
void SP_func_explosive (edict_t *self);
void SP_misc_explobox (edict_t *self);
void SP_misc_blackhole (edict_t *ent);
void SP_misc_eastertank (edict_t *ent);
void SP_misc_easterchick (edict_t *ent);
void SP_misc_easterchick2 (edict_t *ent);
void SP_light_mine1 (edict_t *ent);
void SP_monster_commander_body (edict_t *self);
void SP_misc_banner (edict_t *ent);
void SP_misc_deadsoldier (edict_t *ent);
void SP_misc_viper (edict_t *ent);
void SP_misc_bigviper (edict_t *ent);
void SP_misc_viper_bomb (edict_t *self);
void SP_misc_strogg_ship (edict_t *ent);
void SP_misc_satellite_dish (edict_t *ent);
void SP_light_mine2 (edict_t *ent);
void SP_misc_gib_arm (edict_t *ent);
void SP_misc_gib_leg (edict_t *ent);
void SP_misc_gib_head (edict_t *ent);
void SP_target_character (edict_t *self);
void SP_target_string (edict_t *self);
void SP_func_clock (edict_t *self);
void SP_misc_teleporter (edict_t *ent);
void SP_misc_teleporter_dest (edict_t *ent);
void SP_misc_hologram (edict_t *ent);
void SP_misc_rain (edict_t *ent);
void SP_misc_object (edict_t *ent);
void SP_misc_flame (edict_t *ent);
void SP_misc_setlights (edict_t *self);
void SP_misc_flyby (edict_t *ent);
void SP_misc_highlight (edict_t *self);
void SP_misc_bigbolt (edict_t *self);
void SP_misc_rubble (edict_t *self);
void gib_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

//===================
// G_MONSTER.C
//===================
void monster_fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect);
void monster_fire_rifle (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect);
void monster_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int flashtype);
void monster_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype);
void M_FlyCheck (edict_t *self);
void AttackFinished (edict_t *self, float time);
void M_CheckGround (edict_t *ent);
void M_CatagorizePosition (edict_t *ent);
void M_WorldEffects (edict_t *ent);
void M_droptofloor (edict_t *ent);
void M_SetEffects (edict_t *ent);
void M_MoveFrame (edict_t *self);
void monster_think (edict_t *self);
void monster_use (edict_t *self, edict_t *other, edict_t *activator);
void monster_triggered_spawn (edict_t *self);
void monster_triggered_spawn_use (edict_t *self, edict_t *other, edict_t *activator);
void monster_triggered_start (edict_t *self);
void monster_death_use (edict_t *self);
qboolean monster_start (edict_t *self);
void monster_start_go (edict_t *self);
void walkmonster_start_go (edict_t *self);
void walkmonster_start (edict_t *self);
void flymonster_start_go (edict_t *self);
void flymonster_start (edict_t *self);
void swimmonster_start_go (edict_t *self);
void swimmonster_start (edict_t *self);

//===================
// G_NPC_AI.C
//===================
void ai_npc_charge (edict_t *self, float dist);
void npc_FoundTarget (edict_t *self);
void AI_npc_AlertStroggs (edict_t *self);
qboolean npc_FindTarget (edict_t *self);
void ai_npc_stand (edict_t *self, float dist);
void ai_npc_walk2 (edict_t *self, float dist);
void ai_npc_walk (edict_t *self, float dist);
void ai_npc_run_nopath (edict_t *self, float dist);

//===================
// G_SVCMDS.C
//===================
void	Svcmd_Test_f (void);
void cmd_Save_f (void);
void cmd_Load_f (void);
qboolean SV_FilterPacket (char *from);
void	ServerCommand (void);

//===================
// G_UTILS.C
//===================
float VectorLengthSquared(vec3_t v);
void G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
edict_t *G_Find (edict_t *from, int fieldofs, char *match);
edict_t *findradius (edict_t *from, vec3_t org, float rad);
edict_t *G_PickTarget (char *targetname);
void Think_Delay (edict_t *ent);
void G_UseTargets (edict_t *ent, edict_t *activator);
float	*tv (float x, float y, float z);
char	*vtos (vec3_t v);
void G_SetMovedir (vec3_t angles, vec3_t movedir);
float vectoyaw (vec3_t vec);
void vectoangles (vec3_t value1, vec3_t angles);
char *G_CopyString (char *in);
void G_InitEdict (edict_t *e);
edict_t *G_Spawn (void);
void G_FreeEdict (edict_t *ed);
void	G_TouchTriggers (edict_t *ent);
void	G_TouchSolids (edict_t *ent);
qboolean KillBox (edict_t *ent);

//===================
// G_WEAPON.C
//===================
void SWTC_MuzzleFlash (edict_t *ent, byte effect);
void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed);
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick);
void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod);
void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, qboolean trooper);
void fire_disruptor (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
void fire_repeater (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
void fire_bowcaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
void fire_beamtube (edict_t *self, vec3_t start, vec3_t dir, int damage);
void fire_thermal (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held, int secfunc);
void fire_sec_missile (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float damage_radius, qboolean held);
void fire_missile (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_homing (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);

//===================
// M_MAP.C
//===================
counter_t Find_Nearest_Node (edict_t *self);
qboolean spawn_node (vec3_t org);
qboolean should_link_up (edict_t *self, vec3_t node1, vec3_t node2);
void add_node_to_list (edict_t *self, counter_t count);
counter_t any_nodes_nearby (edict_t *self);
void show_nodes (void);
void show_visible_nodes (edict_t *ent);
void mapping_toggle (void);
qboolean make_node (edict_t *self);
void nodetest_activate (edict_t *self);
void nodetest_think (edict_t *self);
void create_nodetest (edict_t *self);

//===================
// M_MOVE.C
//===================
qboolean M_CheckBottom (edict_t *ent);
qboolean M_CheckMapBottom (edict_t *ent);
qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink);
void M_ChangeYaw (edict_t *ent);
qboolean SV_StepDirection (edict_t *ent, float yaw, float dist);
void SV_FixCheckBottom (edict_t *ent);
void SV_NewChaseDir (edict_t *actor, edict_t *enemy, float dist);
qboolean SV_CloseEnough (edict_t *ent, edict_t *goal, float dist);
void M_MoveToGoal (edict_t *ent, float dist);
qboolean M_walkmove (edict_t *ent, float yaw, float dist);

//===================
// M_ROUTE.C
//===================
void initialize_tables (void);
qboolean Dijkstra_ShortestPath (counter_t source, counter_t target);
void find_path (edict_t *self);
void find_next_node (edict_t *self);
edict_t *spawn_path (vec3_t org);
void PlayerTrail_LaserThink (edict_t *self);
void allocate_trail_edicts (void);
void remove_previous_trail (void);
void visible_trail (int source, int target);
void show_route_toggle (void);
void display_dijkstra_path (int source, int target);
void PlayerTrail_AddLaserTrail (counter_t ptr, counter_t target);

//===================
// M_UTILS.C
//===================
void predictTargPos (edict_t *self, vec3_t source, vec3_t targPos, vec3_t targVel, float speed, qboolean feet);
qboolean visible_from_weapon (edict_t *ent, vec3_t source);

//===================
// P_CAM.C
//===================
void ChasecamStart (edict_t *ent);
void ChasecamRemove (edict_t *ent, char *opt);
void ChasecamTrack (edict_t *ent);
void Cmd_Chasecam_Toggle (edict_t *ent);
void Cmd_Chasecam_Zoom(edict_t *ent, char *opt);
void Cmd_Chasecam_Viewlock(edict_t *ent, char *opt);
void CheckChasecam_Viewent (edict_t *ent);

//===================
// P_CLIENT.C
//===================
void spawn_ghost (edict_t *self);
void SP_info_player_start(edict_t *self);
void SP_info_player_deathmatch(edict_t *self);
void SP_info_player_coop(edict_t *self);
void SP_info_player_intermission(edict_t *self);
void player_pain (edict_t *self, edict_t *other, float kick, int damage);
qboolean IsFemale (edict_t *ent);
qboolean IsNeutral (edict_t *ent);
void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker);
void TossClientWeapon (edict_t *self);
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker);
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void InitClientPersistant (gclient_t *client);
void InitClientResp (gclient_t *client);
void SaveClientData (void);
void FetchClientEntData (edict_t *ent);
float	PlayersRangeFromSpot (edict_t *spot);
edict_t *SelectRandomDeathmatchSpawnPoint (void);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);
edict_t *SelectDeathmatchSpawnPoint (void);
edict_t *SelectCoopSpawnPoint (edict_t *ent);
void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
void InitBodyQue (void);
void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void CopyToBodyQue (edict_t *ent);
void respawn (edict_t *self);
void spectator_respawn (edict_t *ent);
void PutClientInServer (edict_t *ent);
void ClientBeginDeathmatch (edict_t *ent);
void ClientBegin (edict_t *ent);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientDisconnect (edict_t *ent);
trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end);
unsigned CheckBlock (void *b, int c);
void PrintPmove (pmove_t *pm);
void Client_Check_Collide (edict_t *self);
void set_fov (edict_t *ent);
void ClientThink (edict_t *ent, usercmd_t *ucmd);
void ClientBeginServerFrame (edict_t *ent);
void RespawnExplosion(edict_t *ent);
void chicken(edict_t *ent);


//===================
// P_FORCE.C
//===================
int FindPowerByName (char *name);
gforce_t *GetPowerByIndex (int index);
void force_frame (edict_t *ent);
void sort_useable_powers(edict_t *ent);
void calc_top_level_value (edict_t *ent);
void calc_darklight_value (edict_t *ent);
void calc_subgroup_values (edict_t *ent);
qboolean Force_Power_Available (edict_t *ent, int index);
int Force_constant_active (edict_t *ent, int index);
qboolean Force_Power_Useable (edict_t *ent, int index);
void force_unlearn (edict_t *ent, int index, float lose);
void force_group_learn (edict_t *ent, int curr, float gain);
void force_learn (edict_t *ent, int power);
void Drain_Force_Pool (edict_t *ent, int power);
void Drain_Force_Pool_Constant (edict_t *ent, int power);
void Check_Active_Powers (edict_t *ent);
void check_new_powers_available (edict_t *ent);
void Think_Force (edict_t *ent);
void Think_Force_hold (edict_t *ent);
void heal_effect_update (edict_t *self);
void heal_effect_spawn (vec3_t from, edict_t *to, qboolean side);
void heal_effect (edict_t *ent, int level, qboolean side);
void force_push_hold (edict_t *ent, int last);
void airburst(edict_t *self, vec3_t origin, float power, float radius);
void fire_airburst (edict_t *self, vec3_t start, vec3_t aimdir);
void force_push_fire (edict_t *ent);
void force_push (edict_t *ent, int last);
void force_pull (edict_t *ent, int last);
void force_levitate (edict_t *ent, int last);
void force_negate (edict_t *ent, int last);
void force_speed (edict_t *ent, int last);
void force_saber_throw (edict_t *ent, int last);
void force_lightheal (edict_t *ent, int last);
void force_wall_of_light (edict_t *ent, int last);
void force_shield (edict_t *ent, int last);
void force_invisibility (edict_t *ent, int last);
void force_wind (edict_t *ent, int last);
void force_reflect (edict_t *ent, int last);
void force_scout (edict_t *ent, int last);
void force_bind (edict_t *ent, int last);
void force_darkheal (edict_t *ent, int last);
void force_lightning (edict_t *ent, int last);
void force_choke (edict_t *ent, int last);
void force_absorb (edict_t *ent, int last);
void force_wall_of_darkness (edict_t *ent, int last);
void force_taint (edict_t *ent, int last);
void force_inferno (edict_t *ent, int last);
void force_rage (edict_t *ent, int last);
gitem_t *weapon_menu_use (edict_t *ent);
int Check_Weapon (edict_t *ent, char *s);
void setup_weap_menu (edict_t *ent);
void set_sub_menu1_weap (edict_t *ent);
void set_sub_menu2_weap (edict_t *ent);
void set_sub_menu3_weap (edict_t *ent);
void set_sub_menu4_weap (edict_t *ent);
void setup_force_menu (edict_t *ent);
void select_menu_power (edict_t *ent);
void set_sub_menu1 (edict_t *ent);
void set_sub_menu2 (edict_t *ent);
void set_sub_menu3 (edict_t *ent);
void set_sub_menu4 (edict_t *ent);
void set_sub_menu5 (edict_t *ent);
void set_sub_menu6 (edict_t *ent);
void sound_delay(edict_t *ent, float delay, int setto);
void sound_reinit(edict_t *ent);

//===================
// P_HUD.C
//===================
void MoveClientToIntermission (edict_t *ent);
void BeginIntermission (edict_t *targ);
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer);
void DeathmatchScoreboard (edict_t *ent);
void Cmd_Score_f (edict_t *ent);
void HelpComputer (edict_t *ent);
void Cmd_Help_f (edict_t *ent);
void G_SetStats (edict_t *ent);
void G_CheckChaseStats (edict_t *ent);
void G_SetSpectatorStats (edict_t *ent);

//===================
// P_WEAPON.C
//===================
void Reload_NULL (edict_t *ent);
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void PlayerNoise(edict_t *who, vec3_t where, int type);
qboolean Pickup_Weapon (edict_t *ent, edict_t *other);
void ChangeWeapon (edict_t *ent);
void NoAmmoWeaponChange (edict_t *ent);
void Think_Weapon (edict_t *ent, int secfunc);
void Use_Weapon (edict_t *ent, gitem_t *item);
void Drop_Weapon (edict_t *ent, gitem_t *item);
void Weapon_Generic_Saber (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int FRAME_RELOAD_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent), void (*reload)(edict_t *ent));
void Weapon_Thermal (edict_t *ent, int secfunc);
void Weapon_BeamTube(edict_t *ent, int secfunc);
void Weapon_Sniper(edict_t *ent, int secfunc);
void Weapon_Disruptor(edict_t *ent, int secfunc);
void Weapon_MissileTube (edict_t *ent, int secfunc);
void Weapon_WristRocket(edict_t *ent, int secfunc);
void Weapon_Bowcaster (edict_t *ent, int secfunc);
void Weapon_Repeater (edict_t *ent, int secfunc);
void Weapon_Rifle (edict_t *ent, int secfunc);
void Weapon_Hands (edict_t *ent, int secfunc);
void Weapon_SaberThrow (edict_t *ent, int secfunc);
void Weapon_Saber (edict_t *ent, int secfunc);
void Weapon_Blaster (edict_t *ent, int secfunc);
void weapon_saber_fire (edict_t *ent);
void weapon_thermal_fire (edict_t *ent, qboolean held, int secfunc);
void Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean trooper);


//===================
// S_COMPUTER.C
//===================
void Computer_Open (edict_t *ent, edict_t *activator);
void Computer_Load_CDS (char *filename, edict_t *activator);
void Computer_Change_Dir (char *cds, edict_t *ent);
void Computer_Cat (char *txt, edict_t *ent);
void Computer_Printout (char *targettxt, edict_t *ent);
void Computer_Pass (char *pass, edict_t *ent);
void Computer_Dir(edict_t *ent);
void Computer_Run(char *file, edict_t *ent);
void Computer_Trigger (edict_t *ent);
void Computer_Exec (char *exec, edict_t *ent);

//RipVTide

void _stuffcmd(edict_t* ent, char *s, ...);
int  ClipVelocity (vec3_t in, vec3_t normal, vec3_t out, float overbounce);
void Critter_Panic (edict_t *ent);
void Set_Panic(edict_t *ent);
void flagsnap (edict_t *self, edict_t *other, float kick, int damage);