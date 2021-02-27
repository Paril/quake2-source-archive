#ifndef V_SHARED_H
#define V_SHARED_H

#include "q_shared.h"
#include "weapon_def.h"
#include "ability_def.h"
#include "settings.h"
#include "v_utils.h"
#include "v_items.h"
#include "v_maplist.h"
#include "shaman.h"
#include "auras.h"
#include "Talents.h"

/**************** v_abilitylist.c ***************/
void enableAbility(edict_t *ent, int index, int level, int max_level, int general);
int getLastUpgradeIndex(edict_t *ent, int generalmode);
void disableAbilities(edict_t *ent);
int getHardMax(int index, qboolean general, int class);
void AssignAbilities(edict_t *ent);

#define INCREASED_SOFTMAX 20
#define DEFAULT_SOFTMAX 15
#define GENERAL_SOFTMAX 8

typedef struct
{
	int index;
	int start;
	int softmax;
	int general;
}abildefinition_t;

typedef abildefinition_t* AbilList;
/**************** v_abilitylist.c ***************/

//************ lasersight.c ************
void lasersight_off(edict_t *ent);
//************ lasersight.c ************

//************ supplystation.c ************
float AmmoLevel(edict_t *ent, int ammo_index);
int G_GetAmmoIndexByWeaponIndex(int weapon_index);
int G_GetRespawnWeaponIndex(edict_t *ent);
int MaxAmmoType(edict_t *ent, int ammo_index);
//************ supplystation.c ************

//************ g_misc.c ************
int PVM_RemoveAllMonsters(edict_t *monster_owner);
int AveragePlayerLevel(void);
int PvMAveragePlayerLevel(void);
int PVM_TotalMonsters(edict_t *monster_owner, qboolean update);
//************ g_misc.c ************

//************ g_utils.c ************
char *CryptString(char *text, qboolean decrypt);
qboolean G_ClearPath(edict_t *ignore1, edict_t *ignore2, int mask, vec3_t spot1, vec3_t spot2);
int G_GetHypotenuse(vec3_t v);
//************ g_utils.c ************

//************ misc_stuff.c ************
qboolean V_AssignClassSkin(edict_t *ent, char *s);
void giveAdditionalRespawnWeapon(edict_t *ent, int nextWeapon);
//************ misc_stuff.c ************

//************ g_main.c ************
void ExitLevel(void);
void VortexEndLevel(void);
//************ g_main.c ************

//************ magic.c ************
void cmd_mjump(edict_t *ent);
void proxy_remove(edict_t *self, qboolean print);
qboolean ConvertOwner(edict_t *ent, edict_t *other, float duration, qboolean print);
qboolean RestorePreviousOwner(edict_t *ent);
void ProjectileLockon(edict_t *proj);
//************ magic.c ************

//************ auras.c ************
#define QUE_MAXSIZE	10
#define QUE_AURA	1
#define QUE_CURSE	2
qboolean que_valident(que_t *que);
//************ auras.c ************

//*********** g_items.c ***********
void Tball_Aura(edict_t *owner, vec3_t origin);
void tech_dropall(edict_t *ent);
//*********** g_items.c ***********

//************ p_menu.c ***********
void setClassAbilities(edict_t *ent);
void setGeneralAbilities(edict_t *ent);
void disableAbilities(edict_t *ent);
//************ p_menu.c ***********

//********** v_file_IO.c **********
void VRXGetPath(char* path, edict_t* ent);
char *CryptPassword(char *text);
qboolean savePlayer(edict_t *ent);
void VSF_SaveRunes(edict_t *player, char *path);
qboolean openPlayer(edict_t *ent);
void createOpenPlayerThread(edict_t *ent);
//********** v_file_IO.c **********

//********** v_maplist.c **********
int v_LoadMapList(int mode);
//********** v_maplist.c **********

//*********** v_items.c ***********
edict_t *V_SpawnRune(edict_t *self, edict_t *attacker, float base_drop_chance, float levelmod);
void V_TossRune(edict_t *rune, float h_vel, float v_vel);
void SpawnRune(edict_t *self, edict_t *attacker, qboolean debug);
qboolean V_HiddenMod(edict_t *ent, item_t *rune, imodifier_t *mod);
void adminSpawnRune(edict_t *self, int type, int index);
qboolean Pickup_Rune(edict_t *ent, edict_t *other);
item_t *V_FindFreeItemSlot(edict_t *ent);
item_t *V_FindFreeTradeSlot(edict_t *ent, int index);
qboolean V_CanPickUpItem(edict_t *ent);
void V_EquipItem(edict_t *ent, int index);
void V_ItemCopy(item_t *dest, item_t *source);
void V_ItemClear(item_t *item);
void V_PrintItemProperties(edict_t *player, item_t *item);
int eqSetItems(edict_t *ent, item_t *rune);
void ApplyRune(edict_t *ent, item_t *rune);
void V_ResetAllStats(edict_t *ent);
void PurchaseRandomRune(edict_t *ent, int runetype);
void V_ApplyRune(edict_t *ent, item_t *rune);
void V_ItemSwap(item_t *item1, item_t *item2);
void cmd_Drink(edict_t *ent, int itemtype, int index);
//*********** v_items.c ***********

//*********** v_utils.c ***********
qboolean V_IsPVP(void);
char *V_GetMonsterName(edict_t *monster);
char *GetAbilityString(int ability_number);
abildefinition_t *getClassRuneStat(int cIndex);
char *GetRuneValString(item_t *rune);
char *GetShortWeaponString(int weapon_number);
char *GetRandomString(int len);
float V_EntDistance(edict_t *ent1, edict_t *ent2);
edict_t *V_getClientByNumber(int index);
int GetClientNumber(edict_t *ent);
char ReadChar(FILE *fptr);
void WriteChar(FILE *fptr, char Value);
void ReadString(char *buf, FILE *fptr);
void WriteString(FILE *fptr, char *String);
int ReadInteger(FILE *fptr);
void WriteInteger(FILE *fptr, int Value);
long ReadLong(FILE *fptr);
void WriteLong(FILE *fptr, long Value);
int CountAbilities(edict_t *player);
int FindAbilityIndex(int index, edict_t *player);
int CountWeapons(edict_t *player);
int FindWeaponIndex(int index, edict_t *player);
int CountRunes(edict_t *player);
int FindRuneIndex(int index, edict_t *player);
char *V_FormatFileName(char *name);
void V_tFileGotoLine(FILE *fptr, int linenumber, long size);
int V_tFileCountLines(FILE *fptr, long size);
char *GetClassString(int class_num);
int getClassNum(char *newclass);
char *V_MenuItemString(item_t *item, char selected);
char *GetArmoryItemString(int purchase_number);
void PrintCommands(edict_t *ent);
int CountRuneMods(item_t *rune);
void V_ResetAbilityDelays(edict_t *ent);
void SaveArmory();
void LoadArmory();
qboolean V_GiveAmmoClip(edict_t *ent, float qty, int ammotype);
int V_GetRespawnAmmoType(edict_t *ent);
void ChangeClass(char *playername, int newclass, int msgtype);
char *GetTalentString(int talent_ID);
char *V_TruncateString(char *string, int newStringLength);
//*********** v_utils.c ***********

//************ vote.c *************
void CheckPlayerVotes(void);
void V_ChangeMap(v_maplist_t *maplist, int mapindex, int gamemode);
int FindBestMap(int mode);
v_maplist_t *GetMapList(int mode);
int V_AttemptModeChange(qboolean endlevel);
//************ vote.c *************

//*********** weapons.c ***********
void resetWeapons(edict_t *ent);
//*********** weapons.c ***********

//******* weapon_upgrades.c *******
int V_WeaponUpgradeVal(edict_t *ent, int weapnum);
//******* weapon_upgrades.c *******

//************ p_hud.c ************
void VortexBeginIntermission(char *nextmap);
//************ p_hud.c ************

//************ talents.c ************
void setTalents(edict_t *ent);
void eraseTalents(edict_t *ent);
void upgradeTalent(edict_t *ent, int talentID);
int getTalentSlot(edict_t *ent, int talentID);
int getTalentLevel(edict_t *ent, int talentID);
//************ talents.c ************

//************ player.c ************
void newPlayer(edict_t *ent);
int canJoinGame(edict_t *ent);
//************ player.c ************

//************ invasion.c ************
edict_t *INV_SelectPlayerSpawnPoint(edict_t *ent);
void INV_InitSpawnQue(void);
qboolean INV_RemoveSpawnQue(edict_t *ent);
void INV_Init(void);
void INV_SpawnPlayers(void);
qboolean INV_AddSpawnQue(edict_t *ent);
int INV_GetNumPlayerSpawns(void);
void INV_AwardPlayers(void);
//************ invasion.c ************

//************ totems.c ************
void SpawnTotem(edict_t *ent, int abilityID);
void RemoveTotem(edict_t *self);
edict_t *NextNearestTotem(edict_t *ent, int totemType, edict_t *lastTotem, qboolean allied);
//************ totems.c ************

//************* MENUS *************
void OpenWeaponUpgradeMenu(edict_t *ent, int lastline);					//upgrade your weapon
void OpenUpgradeMenu(edict_t *ent);										//upgrade your abilities
void ShowInventoryMenu(edict_t *ent, int lastline, qboolean selling);	//shows the full list of items in special inventory
void OpenRespawnWeapMenu(edict_t *ent);									//set respawn weapon
void OpenArmoryMenu(edict_t *ent);										//Load the armory (buy/sell)
void OpenClassMenu(edict_t *ent, int page_num);						//select class
void OpenMyinfoMenu(edict_t *ent);										//vrxifo
void ShowTradeMenu(edict_t *ent);										//trade with another player
void ShowVoteModeMenu(edict_t *ent);									//vote for mode/map
void StartShowInventoryMenu(edict_t *ent, item_t *item);				//used for trading, selling, deleting, and viewing items
void ShowHelpMenu(edict_t *ent, int lastpick);							//help menu
void OpenGeneralMenu(edict_t *ent);									//general vrx menu
void EndTrade(edict_t *ent);
void OpenWhoisMenu(edict_t *ent);										//whois information
void OpenTalentUpgradeMenu(edict_t *ent, int lastline);				//upgrade your talents
//************* MENUS *************
void laser_remove(edict_t *self);
void RemoveAllDrones(edict_t *ent, qboolean refund_player);
void RemoveHellspawn(edict_t *ent);
void RemoveMiniSentries(edict_t *ent);
void DroneRemoveSelected(edict_t *ent, edict_t *drone);

//***** PLAYER-MONSTER STUFF ******
// general player-monster stuff
// this should eventually be moved to its own file
void PM_RestorePlayer(edict_t *ent);
void PM_RemoveMonster(edict_t *monster);
void PM_Effects(edict_t *ent);
qboolean PM_MonsterHasPilot(edict_t *monster);
qboolean PM_PlayerHasMonster(edict_t *player);
edict_t *PM_GetPlayer(edict_t *e); // returns player entity
void PM_UpdateChasePlayers(edict_t *ent);
//***** PLAYER-MONSTER STUFF ******
void dmgListCleanup(edict_t *self, qboolean clear_all);
float GetTotalBossDamage(edict_t *self);
float GetPlayerBossDamage(edict_t *player, edict_t *boss);
qboolean SpawnWaitingPlayers(void);

// etc
int GetAbilityUpgradeCost(int index);
#endif
