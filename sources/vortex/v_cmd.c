#include "g_local.h"

#ifdef CMD_USEHASH
#include "fnv.h"
#endif

void Cmd_IdentifyPlayer (edict_t *ent);
void Cmd_SetMasterPassword_f (edict_t *ent);
void Cmd_SetOwner_f (edict_t *ent);
void Cmd_TransCredits(edict_t *ent);
void Cmd_AdminCmd (edict_t *ent);
void Cmd_BuildLaser (edict_t *ent);
void Cmd_Thrust_f (edict_t *ent);
void Cmd_Overload_f (edict_t *ent);
void Cmd_HolyFreeze(edict_t *ent);
void Cmd_WritePos_f (edict_t *ent);
void Cmd_Rune_f(edict_t *ent);
void Cmd_Forcewall(edict_t *ent);
void ForcewallOff(edict_t *player);
void ArmageddonSpell(edict_t *ent);
void BuildMiniSentry (edict_t *ent);
void TeleportPlayer (edict_t *player);
void SpawnTestEnt (edict_t *ent);
void TeleportForward (edict_t *ent);
void lasersight_off (edict_t *ent);
void cmd_SentryGun(edict_t *ent);
void PlayerToParasite (edict_t *ent);
void Cmd_DetPipes_f (edict_t *ent);
void Cmd_ExplodingArmor_f (edict_t *ent);
void Cmd_Cripple_f (edict_t *ent);
void Cmd_SpawnMagmine_f (edict_t *ent);
void Cmd_ExplodingArmor_f (edict_t *ent);
void Cmd_Togglesecondary_f (edict_t *ent);
void Cmd_Spike_f (edict_t *ent);
void Cmd_BuildProxyGrenade (edict_t *ent);
void Cmd_Napalm_f (edict_t *ent);
void Cmd_PlayerToTank_f (edict_t *ent);
void Cmd_AutoCannon_f (edict_t *ent);
void Cmd_BlessedHammer_f (edict_t *ent);
void Cmd_WormHole_f (edict_t *ent);
void Cmd_SpikeGrenade_f (edict_t *ent);
void Cmd_Detector_f (edict_t *ent);
void Cmd_Conversion_f (edict_t *ent);
void Cmd_Deflect_f (edict_t *ent);
void Cmd_Antigrav_f (edict_t *ent);
void Cmd_TossEMP (edict_t *ent);
void Cmd_Plasmabolt_f (edict_t *ent);
void Cmd_TossMirv (edict_t *ent);
void Cmd_Healer_f (edict_t *ent);
void Cmd_Spiker_f (edict_t *ent);
void Cmd_Obstacle_f (edict_t *ent);
void Cmd_box_f(edict_t *ent); //lepi
void Cmd_Gasser_f (edict_t *ent);
void Cmd_TossSpikeball (edict_t *ent);
void Cmd_FireAcid_f (edict_t *ent);
void Cmd_Cocoon_f (edict_t *ent);
void Cmd_Meditate_f (edict_t *ent);
void Cmd_CreateLaserPlatform_f (edict_t *ent);
void Cmd_LaserTrap_f (edict_t *ent);
void Cmd_HolyGround_f (edict_t *ent);
void Cmd_UnHolyGround_f (edict_t *ent);
void Cmd_Purge_f (edict_t *ent);
void Cmd_Boomerang_f (edict_t *ent);
qboolean autoaim_findtarget (edict_t *ent);
void Cmd_AddNode_f (edict_t *ent);
void Cmd_DeleteNode_f (edict_t *ent);
void Cmd_DeleteAllNodes_f (edict_t *ent);
void Cmd_SaveNodes_f (edict_t *ent);
void Cmd_LoadNodes_f (edict_t *ent);
void Cmd_ComputeNodes_f (edict_t *ent);
void Cmd_ToggleShowGrid (edict_t *ent);
void Cmd_SelfDestruct_f(edict_t *self);
void Grenade_Explode (edict_t *ent);
void Cmd_CorpseExplode(edict_t *ent);
void Cmd_HellSpawn_f (edict_t *ent);
void Cmd_Caltrops_f (edict_t *ent);
void Cmd_fmedi_f(edict_t *ent);
void Cmd_PrintCommandList(edict_t *ent);

#define CommandTotal sizeof(commands) / sizeof(gameCommand_s)

gameCommand_s commands[] = 
{
	{ "medic", 			Cmd_PlayerToMedic_f },
	{ "autocannon", 	Cmd_AutoCannon_f },
	{ "blessedhammer", 	Cmd_BlessedHammer_f },
	{ "armorbomb", 		Cmd_ExplodingArmor_f },
	{ "vrxmenu", 		OpenGeneralMenu },
	{ "proxy", 			Cmd_BuildProxyGrenade },
	{ "napalm", 		Cmd_Napalm_f },
	{ "cripple", 		Cmd_Cripple_f },
	{ "antigrav", 		Cmd_Antigrav_f },
	{ "masterpw", 		Cmd_SetMasterPassword_f },
	{ "owner", 			Cmd_SetOwner_f },
	{ "whois", 			OpenWhoisMenu },
	{ "bless", 			Cmd_Bless },
	{ "heal", 			Cmd_Healing },
	{ "cacodemon", 		Cmd_PlayerToCacodemon_f },
	{ "flyer", 			Cmd_PlayerToFlyer_f },
	{ "mutant", 		Cmd_PlayerToMutant_f },
	{ "brain", 			Cmd_PlayerToBrain_f },
	{ "tank", 			Cmd_PlayerToTank_f },
	{ "hellspawn", 		Cmd_HellSpawn_f },
	{ "supplystation", 	Cmd_CreateSupplyStation_f },
	{ "decoy", 			Cmd_Decoy_f },
	{ "curse", 			Cmd_Curse },
	{ "amnesia", 		Cmd_Amnesia },
	{ "weaken", 		Cmd_Weaken },
	{ "lifedrain", 		Cmd_LifeDrain },
	{ "ampdamage", 		Cmd_AmpDamage },
	{ "lowerresist", 	Cmd_LowerResist },
	{ "selfdestruct",	Cmd_SelfDestruct_f },
	{ "ally", 			ShowAllyMenu },
	{ "magmine",		Cmd_SpawnMagmine_f },
	{ "spike",			Cmd_Spike_f },
	{ "minisentry", 	Cmd_MiniSentry_f },
	{ "parasite", 		Cmd_PlayerToParasite_f },
	{ "teleport_fwd", 	TeleportForward },
	{ "admincmd", 		Cmd_AdminCmd },
	{ "transfercredits",Cmd_TransCredits },
	{ "forcewall", 		Cmd_Forcewall },
    { "forcewall_off", 	ForcewallOff },
	{ "laser", 			Cmd_BuildLaser },
    { "sentry", 		cmd_SentryGun },
	{ "lasersight", 	Cmd_LaserSight_f },
	{ "flashlight",     FL_make  },
	{ "monster", 		Cmd_Drone_f },
	{ "detpipes", 		Cmd_DetPipes_f  },
	{ "vrxinfo", 		OpenMyinfoMenu },
	{ "vrxarmory", 		OpenArmoryMenu },
	{ "vrxrespawn", 	OpenRespawnWeapMenu },
	{ "thrust",         Cmd_Thrust_f  },
	{ "vote", 			ShowVoteModeMenu },
	{ "wormhole",	    Cmd_WormHole_f },
	{ "update",		    V_UpdatePlayerAbilities},
	{ "berserker",	    Cmd_PlayerToBerserk_f },
	{ "medicpack",		 Cmd_fmedi_f }, //lepi
	{ "caltrops",	    Cmd_Caltrops_f },
	{ "spikegrenade",   Cmd_SpikeGrenade_f },
	{ "detector",	    Cmd_Detector_f },
	{ "convert",	    Cmd_Conversion_f },
	{ "deflect",	    Cmd_Deflect_f },
	{ "scanner",	    Toggle_Scanner },
	{ "emp",		    Cmd_TossEMP },
	{ "plasmabolt",	    Cmd_Plasmabolt_f },
	{ "mirv",		    Cmd_TossMirv },
	{ "healer",		    Cmd_Healer_f },
	{ "spiker",		    Cmd_Spiker_f },
	{ "box",				Cmd_box_f },
	{ "obstacle",	    Cmd_Obstacle_f },
	{ "gasser",		    Cmd_Gasser_f },
	{ "spore",		    Cmd_TossSpikeball },
	{ "acid",		    Cmd_FireAcid_f },
	{ "cocoon",		    Cmd_Cocoon_f },
	{ "meditate"   ,	Cmd_Meditate_f },
	{ "overload",	    Cmd_Overload_f },
	{ "laserplatform",  Cmd_CreateLaserPlatform_f },
	{ "lasertrap",	    Cmd_LaserTrap_f },
	{ "holyground",	    Cmd_HolyGround_f },
	{ "unholyground",   Cmd_UnHolyGround_f },
	{ "purge",		    Cmd_Purge_f },
	{ "boomerang",	    Cmd_Boomerang_f },
	{ "loadnodes",	    Cmd_LoadNodes_f },
	{ "savenodes",	    Cmd_SaveNodes_f },
	{ "deletenode",	    Cmd_DeleteNode_f },
	{ "addnode",	    Cmd_AddNode_f },
	{ "deleteallnodes", Cmd_DeleteAllNodes_f },
	{ "computenodes",   Cmd_ComputeNodes_f },
	{ "showgrid",	    Cmd_ToggleShowGrid },
	{ "writepos",	    Cmd_WritePos_f },
	{ "rune",		    Cmd_Rune_f },
	{ "vrxid",		    Cmd_IdentifyPlayer },
	{ "vrxcommands",    Cmd_PrintCommandList },
	{ "upgrade_ability",OpenUpgradeMenu },
	// { "spell_stealammo",Cmd_AmmoStealer_f },
	// { "ammosteal",		Cmd_AmmoStealer_f },
	{ "salvation",		Cmd_Salvation },
	{ "aura_salvation",	Cmd_Salvation },
	{ "spell_boost",	Cmd_BoostPlayer },
	{ "boost",	Cmd_BoostPlayer },
	{ "detonatebody", Cmd_CorpseExplode },
	{ "spell_corpseexplode", Cmd_CorpseExplode },
	{ "aura_holyfreeze", Cmd_HolyFreeze },
	{ "holyfreeze", Cmd_HolyFreeze },
	{ "togglesecondary", Cmd_Togglesecondary_f }
};

#ifdef CMD_USEHASH
#define MAXCOMMANDS 20000

gameCommand_s hashedList[MAXCOMMANDS];

void TestHash()
{
	int count = 0, i, ccount = CommandTotal;
	for (i = 0; i < MAXCOMMANDS; i++)
	{
		if (hashedList[i].Function)
			count++;
	}
	if (count != ccount)
	{
		gi.dprintf("warning: hashing failed for commands.\n");
	}
}

void InitHash()
{
	int i;
	static qboolean initialized = false;

	if (initialized)
		return;

	gi.dprintf("Initializing command list.. ");

	for (i = 0; i < CommandTotal; i++)
	{
		int index = fnv_32a_str(commands[i].FunctionName, FNV1_32A_INIT) % (MAXCOMMANDS);
		memcpy(&hashedList[index], &commands[i], sizeof(gameCommand_s));
	}

	gi.dprintf("Done.\n");

	TestHash();
	initialized = true;
}

qboolean VortexCommand(char* command, edict_t* ent)
{
	int index;

	if (G_IsSpectator(ent))
		return false;

	index = fnv_32a_str(command, FNV1_32A_INIT) % (MAXCOMMANDS);

	if (!hashedList[index].FunctionName || !hashedList[index].Function)
		return false;

	if (!Q_strcasecmp(hashedList[index].FunctionName, command)) // we found it
	{

		hashedList[index].Function(ent);
		return true;
	}
	
	return false;
};
#else

qboolean VortexCommand(char* command, edict_t* ent)
{
	int index;

	if (G_IsSpectator(ent))
		return false;

	for (index = 0; index < CommandTotal; index++)
	{
		if (!Q_stricmp(commands[index].FunctionName, command)) // we found it
		{
			commands[index].Function(ent);
			return true;
		}
	}
	
	return false;
};

#endif

static qboolean initialized = false;

void Cmd_PrintCommandList(edict_t *ent)
{
	static char bigstr[8192]; // Hopefully we don't need more than this.
	int i;

	if (!initialized)
	{
		bigstr[0] = '\0';
		strcat(bigstr, "Command List: \n");
		strcat(bigstr, "=============\n");
		for (i = 0; i < (sizeof(commands) / sizeof(gameCommand_s)); i++)
		{
			strcat(bigstr, commands[i].FunctionName);
			strcat(bigstr, "\n");
		}
		initialized = true;
	}

	safe_cprintf(ent, PRINT_LOW, "%s", bigstr);
}