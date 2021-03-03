//===========================================================================
//
// Name:				bl_spawn.c
// Function:		spawning of bots
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1999-02-10
// Tab Size:		3
//===========================================================================

#include "g_local.h"

#ifdef BOT

#include "bl_main.h"
#include "bl_redirgi.h"
#include "bl_botcfg.h"

//#define TOURNEY

#ifdef TOURNEY
#define ENTERED_ENTERED 0x01	// TOURNEY-related
extern int bots_votedin;
#endif


void ClientBegin(edict_t *ent);
void ClientUserinfoChanged(edict_t *ent, char *userinfo);
qboolean ClientConnect(edict_t *ent, char *userinfo);
void ClientDisconnect(edict_t *ent);

typedef struct queuedbot_s
{
	int count;
	char library[MAX_PATH];
	char userinfo[MAX_INFO_STRING];
	edict_t *ent;
	struct queuedbot_s *next;
} queuedbot_t;

queuedbot_t *queuedbots;

//===========================================================================
// spawns a client entity, initializes the edict and sets the pointer
// to the gclient_t structure
// searches a free client entity from top to bottom
//
// Parameter:				-
// Returns:					the spawned free client edict
// Changes Globals:		-
//===========================================================================
edict_t *G_SpawnClient(void)
{
	int i;
	edict_t *cl_ent;

	for (i = game.maxclients-1; i >= 0; i--)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
		{
			memset(cl_ent, 0, sizeof(*cl_ent));
			G_InitEdict(cl_ent);
			cl_ent->client = &game.clients[cl_ent-g_edicts-1];
			return cl_ent;
		} //end if
	} //end for
	return NULL;
} //end of the function G_SpawnClient
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void G_FreeClientEdict(edict_t *ent)
{
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;
} //end of the function G_FreeClientEdict
//===========================================================================
// the entity will become a bot
//
// Parameter:				lib				: library the bot will use
// Returns:					-
// Changes Globals:		botglobals.states
//								botglobals.numbots
//===========================================================================
void BotBecome(edict_t *ent, bot_library_t *lib)
{
	bot_state_t *bs;

	//remove bot flag before calling ClientConnect to make sure
	//BotMoveToFreeClientEdict won't be called there
	ent->flags &= ~FL_BOT;
	//begin
	ClientBegin(ent);
	//set the bot flag
	ent->flags |= FL_BOT;
	//get pointer to the bot state structure
	bs = &botglobals.botstates[DF_ENTCLIENT(ent)];
	//clear the bot state
	memset(bs, 0, sizeof(bot_state_t));
	//set botstate active flag
	bs->active = true;
	//pointer to the library used by the bot
	bs->library = lib;
	//one extra bot
	botglobals.numbots++;
	//setup the bot client in the library
	BotLib_BotSetupClient(ent, ent->client->pers.userinfo);
} //end of the function BotBecome
//===========================================================================
// destroy the given bot
//
// Parameter:				bot				: bot to destroy
// Returns:					-
// Changes Globals:		botglobals.botstates
//								botglobals.numbots
//===========================================================================
void BotDestroy(edict_t *bot)
{
	bot_state_t *bs;

	if (!(bot->flags & FL_BOT)) return;
	if (!bot->client) return;
	//shutdown the bot client in the library
	BotLib_BotShutdownClient(bot);
	//remove bot flag before disconnecting to prevent printing messages to
	//the bot library
	bot->flags &= ~FL_BOT;
	//disconnect the client
	ClientDisconnect(bot);
	//pointer to the bot state
	bs = &botglobals.botstates[DF_ENTCLIENT(bot)];
	//remove botstate active flag
	bs->active = false;
	//clear the entity
	memset(bot, 0, sizeof(*bot));
	//pointer to gclient_t structure
	bot->client = &game.clients[bot-g_edicts-1];
	//clear the gclient_t structure
	memset(bot->client, 0, sizeof(gclient_t));
	//there is a bot less
	botglobals.numbots--;
	//free the library used by the bot
	BotFreeLibrary(bs->library);
	//remove library pointer
	bs->library = NULL;
	//free up the client edict
	G_FreeClientEdict(bot);
} //end of the function BotDestroy
//===========================================================================
// create a bot with the given user info
//
// Parameter:				userinfo			: userinfo for the bot to create
//								lib				: library the bot will use
// Returns:					created bot
// Changes Globals:		botglobals.states
//								botglobals.numbots
//===========================================================================
edict_t *BotCreate(char *userinfo, bot_library_t *lib)
{
	edict_t *ent;
	bot_state_t *bs;

	//spawn a client entity
	ent = G_SpawnClient();
	//check if there was a free client entity
	if (!ent) return NULL;
	//remove bot flag before calling ClientConnect to make sure
	//BotMoveToFreeClientEdict won't be called there
	ent->flags &= ~FL_BOT;
	//connect the client
	//NOTE: set entity inuse flag to false because the bot isn't spawned
	//			from a savegame
	ent->inuse = false;
	if (!ClientConnect(ent, userinfo))
	{
		//free the client edict
		G_FreeClientEdict(ent);
		return NULL;
	} //end if
	//set the inuse flag after connecting
	ent->inuse = true;
	//set the bot flag
	ent->flags |= FL_BOT;
	//get pointer to the bot state structure
	bs = &botglobals.botstates[DF_ENTCLIENT(ent)];
	//clear the bot state
	memset(bs, 0, sizeof(bot_state_t));
	//set botstate active flag
	bs->active = true;
	//pointer to the library used by the bot
	bs->library = lib;
	//setup the bot client in the library
	//NOTE: call after the bs->library pointer is set
	if (!BotLib_BotSetupClient(ent, userinfo))
	{
		//remove botstate active flag
		bs->active = false;
		//clear the entity
		memset(ent, 0, sizeof(*ent));
		//pointer to gclient_t structure
		ent->client = &game.clients[ent-g_edicts-1];
		//clear the gclient_t structure
		memset(ent->client, 0, sizeof(gclient_t));
		//remove library pointer
		bs->library = NULL;
		//free the client edict
		G_FreeClientEdict(ent);
		return NULL;
	} //end if
	//
	//memcpy(ent->client->pers.userinfo, userinfo, MAX_INFO_STRING);
	//
#ifdef ROCKETARENA
	if (ra->value)
	{
		ent->client->resp.context = (int) atof(Info_ValueForKey(userinfo, "arena"));
		ent->client->ra_time = 0;
	} //end if
#endif //ROCKETARENA
	//one extra bot
	botglobals.numbots++;
	//
	return ent;
} //end of the function CreateBot
//===========================================================================
// move the given bot from it's current client edict_t to a free one
// returns true if the bot is moved otherwise returns false
//
// Parameter:				bot				: bot to move
// Returns:					boolean depending on a succesfull move
// Changes Globals:		botglobals.botstates
//===========================================================================
qboolean BotMoveToFreeClientEdict(edict_t *bot)
{
	edict_t *newcl;
	bot_state_t *bs, *newbs;
	int playernum;

	if (!bot->inuse) return true;
	if (!(bot->flags & FL_BOT)) return true;
	//spawn a free client edict
	newcl = G_SpawnClient();
	//if there isn't a free client edict available
	if (!newcl) return false;
	//copy the bot to the new client edict
	memcpy(newcl, bot, sizeof(edict_t));
	//copy the contents of the g_client_t structure
	memcpy(newcl->client, bot->client, sizeof(gclient_t));
	//copy bot state
	bs = &botglobals.botstates[bot-g_edicts-1];
	newbs = &botglobals.botstates[newcl-g_edicts-1];
	memcpy(newbs, bs, sizeof(bot_state_t));
	//old bot state isn't used anymore
	bs->active = false;
	//the new state is
	newbs->active = true;
	//change the user info
	ClientUserinfoChanged(newcl, bot->client->pers.userinfo);
	//Initialize client edict previously used by the bot
	//clear the gclient_t structure the bot was using
	memset(bot->client, 0, sizeof(gclient_t));
	//clear the edict the bot was using
	memset(bot, 0, sizeof(edict_t));
	//initialize edict
	G_InitEdict(bot);
	//set pointer to g_client structure
	bot->client = &game.clients[bot-g_edicts-1];
	//remove client userinfo
	playernum = bot-g_edicts-1;
	gi.configstring(CS_PLAYERSKINS+playernum, "");
	//move the bot client in the library
	BotLib_BotMoveClient(bot, newcl);
	//the bot has been succesfully moved
	return true;
} //end of the function BotMoveToFreeClientEdict
//===========================================================================
// spawn bots after level changes
// called from SpawnEntities in g_spawn.c
// ClientConnect for real clients is called after SpawnEntities is executed
// so the bot will be moved to another client edict when new clients come
// into the game during the level change
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotSpawn(void)
{
	int i;
	edict_t *cl_ent;

	for (i = 0; i < game.maxclients; i++)
	{
		//if the bot state was in use
		if (botglobals.botstates[i].active)
		{
			cl_ent = DF_CLIENTENT(i);
			//set started to false
			botglobals.botstates[i].started = false;
			//entity is used
			cl_ent->inuse = true;
			//set the bot flag
			cl_ent->flags |= FL_BOT;
			//set user info because Quake2 likes to remove it for fake clients
			ClientUserinfoChanged(cl_ent, cl_ent->client->pers.userinfo);
		} //end if
	} //end for
} //end of the function BotSpawn
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void ShowLoadImage(edict_t *ent)
{
#ifndef TOURNEY
	char loadstring[1400];

	if (!ent->client) return;
	sprintf(loadstring, "xv 104 yv 128 picn loading");
	SendStatusBar(ent, loadstring);
	ent->client->showloading = true;
#endif //TOURNEY
} //end of the function ShowLoadImage
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
extern char *dm_statusbar;			//g_spawn.c
extern char *single_statusbar;	//g_spawn.c

void RemoveLoadImage(edict_t *ent)
{
#ifndef TOURNEY
	if (!ent->client) return;
	//
	if (!ent->client->showmenu)
	{
		//clear the image by drawing the status bar
		if (deathmatch->value) SendStatusBar(ent, dm_statusbar);
		else SendStatusBar(ent, single_statusbar);
	} //end if
	ent->client->showloading = false;
#endif //TOURNEY
} //end of the function RemoveLoadImage
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AddBotToQueue(edict_t *ent, char *library, char *userinfo)
{
	queuedbot_t *bot;

	gi.dprintf("loading...\n");
	if (ent) ShowLoadImage(ent);
	bot = gi.TagMalloc(sizeof(queuedbot_t), TAG_GAME);
	bot->ent = ent;
	bot->count = 2;
	strcpy(bot->library, library);
	memcpy(bot->userinfo, userinfo, MAX_INFO_STRING);

	bot->next = queuedbots;
	queuedbots = bot;
} //end of the function AddBotToQueue
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void AddQueuedBots(void)
{
	queuedbot_t *bot;
	bot_library_t *lib;
	edict_t *botent;

	if (queuedbots)
	{
		if (queuedbots->count-- <= 0)
		{
			bot = queuedbots;
			queuedbots = queuedbots->next;
			//load the default library
			lib = BotUseLibrary(bot->library);
			if (!lib)
			{
				gi.cprintf(bot->ent, PRINT_HIGH, "%s not available\n", bot->library);
			} //end if
			else
			{
				botent = BotCreate(bot->userinfo, lib);
				if (!botent)
				{
					gi.cprintf(bot->ent, PRINT_HIGH, "can't create bot, maxclients = %d\n", game.maxclients);
					//free the library used by the bot
					BotFreeLibrary(lib);
				} //end if
			} //end else
			if (bot->ent) RemoveLoadImage(bot->ent);
			gi.TagFree(bot);
		} //end if
	} //end if
} //end of the function AddQueuedBots
//===========================================================================
// returns true if there is a client with the given name
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
qboolean ClientNameExists(char *name)
{
	int i;
	edict_t *cl_ent;

	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (cl_ent->inuse)
		{
			if (Q_strcasecmp(cl_ent->client->pers.netname, name) == 0)
			{
				return true;
			} //end if
		} //end if
	} //end for
	return false;
} //end of the function ClientNameExists
//===========================================================================
// add a deathmatch bot
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotAddDeathmatch(edict_t *ent)
{
	char uinfo[MAX_INFO_STRING];
	int max, i;
#ifdef TOURNEY
	char bname[32];
#endif

	if (!stricmp(gi.argv(0), "sv"))
	{
		max = 6;
		i = 1;
	} //end if
	else
	{
		max = 5;
		i = 0;
	} //end else
	//need at least 6 parmeters (command name incuded)
	if (gi.argc() < max)
	{
		gi.cprintf(ent, PRINT_HIGH, "too few parameters\n");
		gi.cprintf(ent, PRINT_HIGH, 
						"Usage:   addbot <name> <skin> <charfile> <charname>\n"
						"Example: addbot brianna female/brianna char.c Brianna\n"
						"\n"
						"<name>     = name of the bot\n"
						"<skin>     = skin of the bot\n"
						"<charfile> = character file\n"
						"<charname> = character name\n");
		return;
	} //end if

	memset(uinfo, 0, MAX_INFO_STRING);
#ifdef TOURNEY
	strcpy(bname, gi.argv(i+1));
#endif
	if (ClientNameExists(gi.argv(i+1)))
	{
#ifdef TOURNEY
		if(strlen(bname) < 15)
		{
			char suff[3] = "0";
			strcat(bname, suff);
			while(ClientNameExists(bname))
			{
				suff[0]++;
				strcpy(bname, gi.argv(i+1));
				strcat(bname, suff);
			}
		}
		else
		{
			bname[14] = '0';
			while(ClientNameExists(bname))
			{
				bname[14]++;
			}
		}
#else
		gi.cprintf(ent, PRINT_HIGH, "client name %s is already used\n", gi.argv(i+1));
		return;
#endif //TOURNEY

	} //end if
#ifdef TOURNEY
	Info_SetValueForKey(uinfo, "name", bname);
#else
	Info_SetValueForKey(uinfo, "name", gi.argv(i+1));
#endif //TOURNEY
	Info_SetValueForKey(uinfo, "skin", gi.argv(i+2));
	Info_SetValueForKey(uinfo, "charfile", gi.argv(i+3));
	Info_SetValueForKey(uinfo, "charname", gi.argv(i+4));
	//
#ifdef ROCKETARENA
	if (ra->value)
	{
		if (arena->value > 0 && arena->value <= RA2_NumArenas())
		{
			Info_SetValueForKey(uinfo, "arena", arena->string);
		} //end if
		else Info_SetValueForKey(uinfo, "arena", "1");
	} //end if
#endif //ROCKETARENA
	//
#ifdef ZOID
	if (ctf->value)
	{
		Info_SetValueForKey(uinfo, "ctfteam", gi.cvar("botctfteam", "0", 0)->string);
	} //end if
#endif //ZOID
	//load the default library
	/*
	bot_library_t *lib;
	edict_t *bot;

	lib = BotUseLibrary(gi.argv(1));
	if (!lib)
	{
		str = "%s not available\n";
		if (ent) gi.cprintf(ent, PRINT_HIGH, str, gi.argv(1));
		else gi.dprintf(str, gi.argv(1));
		return;
	} //end if
	bot = BotCreate(uinfo, lib);
	if (!bot)
	{
		str = "can't create bot, maxclients = %d\n";
		if (ent) gi.cprintf(ent, PRINT_HIGH, str, game.maxclients);
		else gi.dprintf(str, game.maxclients);
		//free the library
		BotFreeLibrary(lib);
		return;
	} //end if*/
#if defined(WIN32) || defined(_WIN32)
	AddBotToQueue(ent, gi.cvar("botlib", "gladiator.dll", 0)->string, uinfo);
#else
	AddBotToQueue(ent, gi.cvar("botlib", "gladi386.so", 0)->string, uinfo);
#endif
} //end of the function AddDeathmatchBot
//===========================================================================
// become a bot
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotBecomeDeathmatch(edict_t *ent)
{
	bot_library_t *lib;

	//need at least 1 parmeters (command name incuded)
	if (gi.argc() < 1)
	{
		gi.cprintf(ent, PRINT_HIGH,
						"Usage:   becomebot\n"
						"Example: becomebot\n");
		return;
	} //end if

	//load the default library
#if defined(WIN32) || defined(_WIN32)
	lib = BotUseLibrary(gi.cvar("botlib", "gladiator.dll", 0)->string);
#else
	lib = BotUseLibrary(gi.cvar("botlib", "gladi386.so", 0)->string);
#endif
	if (!lib)
	{
		gi.cprintf(ent, PRINT_HIGH, "%s not available\n", gi.cvar("botlib", "", 0)->string);
		return;
	} //end if
	BotBecome(ent, lib);
} //end of the function BotBecomeDeathmatchBot
//===========================================================================
// add a deathmatch bot
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================

/*QUAKED bot (0 .5 .8) ?
Spawns a bot.

"name"			name
"skin"			skin
"charfile"		file which contains the bot character
"charname"		name of the bot character
*/

void SP_bot(edict_t *self)
{
	BotStoreClientCommand("sv", "addbot", st.name, st.skin, st.charfile, st.charname, NULL);
	BotAddDeathmatch(NULL);
	BotClearCommandArguments();
} //end of the function SP_bot
//===========================================================================
// remove a deathmatch bot
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotRemoveDeathmatch(edict_t *ent)
{
	int i;
	edict_t *cl_ent;
	char *name;

	//
	if (!stricmp(gi.argv(0), "sv")) i = 2;
	else i = 1;
	//check if there's a name available
	if (gi.argc() > i) name = gi.argv(i);
	else name = NULL;
	//
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse) continue;
		if (cl_ent->flags & FL_BOT)
		{
			if (!name || (Q_strcasecmp(cl_ent->client->pers.netname, name) == 0))
			{
				BotDestroy(cl_ent);
				return;
			} //end if
		} //end if
	} //end for
	if (name) gi.cprintf(ent, PRINT_HIGH, "no bot found with name %s\n", name);
	else gi.cprintf(ent, PRINT_HIGH, "no bots to remove\n");
} //end of the functoin BotRemoveDeathmatch
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void CheckMinimumPlayers(void)
{
	cvar_t *minplayers;
	edict_t *cl_ent;
	queuedbot_t *bot;
	int i, numplayers, numbots;
	char buf[32];

#ifndef TOURNEY
	minplayers = gi.cvar("minimumplayers", "0", 0);
#else
	minplayers = gi.cvar("bots_minplayers", "0", 0);
#endif

	if (!minplayers->value) return;
	//
	if (level.framenum & 31) return;
#ifdef ROCKETARENA
	//only in dm or ctf
	if (ra->value) return;
#endif //ROCKETARENA
	//count the number of players and the number of bots
	numplayers = 0;
	numbots = 0;
	for (i = 0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse) continue;
#ifdef OBSERVER
		if (cl_ent->flags & FL_OBSERVER) continue;
#endif //OBSERVER
#ifdef TOURNEY
		if(!cl_ent->client || cl_ent->client->resp.entered != ENTERED_ENTERED)
			continue;
#endif //TOURNEY
		numplayers++;
		if (cl_ent->flags & FL_BOT) numbots++;
	} //end for
	//add the queued bots to the bot count
	for (bot = queuedbots; bot; bot = bot->next)
	{
		numbots++;
	} //end for
	//
#ifdef TOURNEY
	if(((numplayers - bots_votedin - 1) < minplayers->value) &&
	  (numplayers < (int)maxclients->value) && numbots < minplayers->value)
#else
	if (numplayers < minplayers->value)
#endif //TOURNEY
	{
		if (!AddRandomBot(NULL))
		{
			sprintf(buf, "%d", numbots);
			gi.cvar_set("minimumplayers", buf);
		} //end if
	} //end if
#ifdef TOURNEY
	else if((numplayers - bots_votedin - 1) > minplayers->value)
#else
	else if (numplayers > minplayers->value)
#endif //TOURNEY
	{
		if (numbots > 0) BotServerCommand("sv", "removebot", NULL);
	} //end if
} //end of the function CheckMinimumPlayers

#endif //BOT
