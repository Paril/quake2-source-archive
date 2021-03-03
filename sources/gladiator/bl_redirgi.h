//===========================================================================
//
// Name:         bl_redirgi.c
// Function:     bot setup
// Programmer:   Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:  1999-02-10
// Tab Size:     3
//===========================================================================

#define MAX_MODELINDEXES				256
#define MAX_SOUNDINDEXES				256
#define MAX_IMAGEINDEXES				256

//global botimport structure
extern game_import_t newgameimport;
//model index
extern char *modelindexes[MAX_MODELINDEXES];
//sound index
extern char *soundindexes[MAX_SOUNDINDEXES];
//image index
extern char *imageindexes[MAX_IMAGEINDEXES];

//initializes the newgameimport structure
void BotRedirectGameImport(void);
//execute a client command but now for a bot
void BotClientCommand(int client, char *str, ...);
//execute a server command
void BotServerCommand(char *str, ...);
//only stores a client command, does not execute it
void BotStoreClientCommand(char *str, ...);
//clears the bot command arguments
void BotClearCommandArguments(void);
//get the cvar with the given name
//cvar_t *BotGet_cvar(char *var_name);
//clears the model and sound index
void ClearIndexes(void);
//initializes the muzzleflash to sound index table
void BotInitMuzzleFlashToSoundindex(void);
//dumps the model index
void BotDumpModelindex(void);
//dumps the sound index
void BotDumpSoundindex(void);
//dumps the image index
void BotDumpImageindex(void);

#ifdef TOURNEY
#define TECH1_MODEL	"models/ctf/resistance/tris.md2"
#define TECH2_MODEL	"models/ctf/strength/tris.md2"
#define TECH3_MODEL	"models/ctf/haste/tris.md2"
#define TECH4_MODEL	"models/ctf/regeneration/tris.md2"
#define TECH5_MODEL	"models/ctf/vampire/tris.md2"

#define TECH1_INDEX	251
#define TECH2_INDEX	252
#define TECH3_INDEX	253
#define TECH4_INDEX	254
#define TECH4_INDEX	255
#endif //TOURNEY
