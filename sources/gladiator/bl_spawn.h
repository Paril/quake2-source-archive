//===========================================================================
//
// Name:         bl_spawn.h
// Function:     spawning of bots
// Programmer:   Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:  1999-02-10
// Tab Size:     3
//===========================================================================

//spawns bots after level change
void BotSpawn(void);
//adds one deathmatch bot
void BotAddDeathmatch(edict_t *ent);
//removes one deathmatch bot
void BotRemoveDeathmatch(edict_t *ent);
//moves a bot to a free client edict
qboolean BotMoveToFreeClientEdict(edict_t *bot);
//lets a human client become a bot
void BotBecomeDeathmatch(edict_t *ent);
//destroys a bot
void BotDestroy(edict_t *bot);
//adds a bot to the spawn que
void AddBotToQueue(edict_t *ent, char *library, char *userinfo);
//spawn waiting bots
void AddQueuedBots(void);
//
void CheckMinimumPlayers(void);
