//===========================================================================
//
// Name:				bl_botcfg.h
// Function:		bot configuration files
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1998-01-12
// Tab Size:		3
//===========================================================================

typedef struct bot_s
{
	char name[MAX_PATH];
	char skin[MAX_PATH];
	char charfile[MAX_PATH];
	char charname[MAX_PATH];
	struct bot_s *next;
} bot_t;

extern bot_t *botlist;

void AppendPathSeperator(char *path, int length);
bot_t *FindBotWithName(char *name);
void CheckForNewBotFile(void);
int AddRandomBot(edict_t *ent);
