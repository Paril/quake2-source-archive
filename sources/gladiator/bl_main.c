//===========================================================================
//
// Name:				bl_main.c
// Function:		bot setup
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1999-02-10
// Tab Size:		3
//===========================================================================

#include "g_local.h"

#ifdef BOT

#include "bl_main.h"
#include "bl_spawn.h"
#include "bl_redirgi.h"
#include "bl_botcfg.h"

//#define TOURNEY

#if defined(WIN32) || defined(_WIN32)
#include <io.h>
#define PATHSEPERATOR_CHAR			'\\'
#define PATHSEPERATOR_STR			"\\"
#else
#include <dlfcn.h>
#define PATHSEPERATOR_CHAR			'/'
#define PATHSEPERATOR_STR			"/"
#endif

// OSP Tourney DM -- Start
#ifdef TOURNEY
#define MODE_TEAM	0x02
extern int m_mode;			// OSP Tourney match mode param
extern cvar_t *hook_enable;	// OSP Tourney DM hook status --JKK
extern void OSP_serverbotsRemove(void);
#endif
// OSP Tourney DM -- End


bot_globals_t botglobals;

void ClientThink (edict_t *ent, usercmd_t *ucmd);

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void StringMakeGreen(char *str)
{
	cvar_t *v;
	//never make green on a dedicated server
	v = gi.cvar("dedicated", "0", 0);
	if (v->value) return;
	//set the last bit
	while(*str)
	{
		if (*str > ' ') *str |= 128;
		str++;
	} //end while
} //end of the function StringMakeGreen
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int BotSwimming(vec3_t origin)
{
	vec3_t testorg;

	VectorCopy(origin, testorg);
	testorg[2] += 3;
	if (gi.pointcontents(testorg) & MASK_WATER) return true;
	return false;
} //end of the function BotSwimming
//===========================================================================
// hooked in G_RunFrame in g_main.c
// NOTE: don't call between: ClientBeginServerFrame and ClientEndServerFrames
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotExecuteInput(edict_t *bot)
{
	vec3_t angles, forward, right;
	usercmd_t ucmd;
	bot_input_t *bi;
	int client;

#ifdef BOT_DEBUG
	if (botglobals.nobotinput) return;
#endif //BOT_DEBUG

	client = DF_ENTCLIENT(bot);
	//
	if (!bot->client)
	{
		gi.dprintf("client %d without client structure\n", client);
		return;
	} //end if
	//get the input for this client
	if (botglobals.botnewinput[client])
	{
		botglobals.botnewinput[client] = false;
		bi = &botglobals.botinputs[client];
	} //end if
	else
	{
		//there's no new input
		return;
	} //end else
	//
	// usercmd_t
	//
	// byte msec;
	//		number of milliseconds since the last user command update of the client
	//
	// byte buttons;
	//		only BUTTON_ATTACK = 1, BUTTON_USE = 2 and BUTTON_ANY = 128 are
	//		sent from the client to the server. The client command +attack
	//		sets BUTTON_ATTACK and the +use command sets BUTTON_USE
	//		all other commands like +forward, +back, +right etc. set BUTTON_ANY
	//
	// short angles[3];
	//		the current view angles of the client
	//
	// short forwardmove, sidemove, upmove;
	//		forwardmove and sidemove are relative to the given yaw
	//		upmove is not related to the given yaw
	//		for jumping upmove is set to 400
	//		for ducking upmove is set to -400
	//
	//	byte impulse;
	//		will be set by the client command impulse x, where x is a number
	//		in the range [0-255]. This impulse command isn't used anymore.
	//
	//	byte lightlevel;
	//		the current light level of the area the client is currently in
	//
	//clear the whole structure
	memset(&ucmd, 0, sizeof(usercmd_t));
	//the duration for the user command in milli seconds
	ucmd.msec = 1000 * bi->thinktime;
	//
	if (botglobals.nocldouble && (bi->actionflags & ACTION_DELAYEDJUMP))
	{
		bi->actionflags |= ACTION_JUMP;
		bi->actionflags &= ~ACTION_DELAYEDJUMP;
	} //end if
	//set the buttons
	if (bi->actionflags & ACTION_RESPAWN)
	{
		bot->client->latched_buttons |= BUTTON_ATTACK;
	} //end if
	if (bi->actionflags & ACTION_ATTACK)
	{
		ucmd.buttons |= BUTTON_ATTACK;
	} //end if
	if (bi->actionflags & ACTION_USE)
	{
		ucmd.buttons |= BUTTON_USE;
	} //end if
	//set the view angles
	ucmd.angles[PITCH] = ANGLE2SHORT(bi->viewangles[PITCH]);
	ucmd.angles[YAW] = ANGLE2SHORT(bi->viewangles[YAW]);
	ucmd.angles[ROLL] = ANGLE2SHORT(bi->viewangles[ROLL]);
	//get the horizontal forward and right vector
	//if swimming movement is true 3d
   //get the pitch in the range [-180, 180]
	if (BotSwimming(bot->s.origin)) angles[PITCH] = bi->viewangles[PITCH];
	else angles[PITCH] = 0;
	angles[YAW] = bi->viewangles[YAW];
	angles[ROLL] = 0;
	AngleVectors(angles, forward, right, NULL);
	//set the view independent movement
	ucmd.forwardmove = DotProduct(forward, bi->dir) * bi->speed;
	ucmd.sidemove = DotProduct(right, bi->dir) * bi->speed;
	ucmd.upmove = abs(forward[2]) * bi->dir[2] * bi->speed;
	//normal keyboard movement
	if (bi->actionflags & ACTION_MOVEFORWARD) ucmd.forwardmove += 400;
	if (bi->actionflags & ACTION_MOVEBACK) ucmd.forwardmove -= 400;
	if (bi->actionflags & ACTION_MOVELEFT) ucmd.sidemove -= 400;
	if (bi->actionflags & ACTION_MOVERIGHT) ucmd.sidemove += 400;
	//jump/moveup
	if (bi->actionflags & ACTION_JUMP) ucmd.upmove += 400;
	//crouch/movedown
	if (bi->actionflags & ACTION_CROUCH) ucmd.upmove -= 400;
	//impulse always zero
	ucmd.impulse = 0;
	//light level at client location
	ucmd.lightlevel = 64;
	//
	bot->flags |= FL_BOTINPUT;
	//the Pmove function was probably designed to run at a higer frequency
	//than 10 Hz.
	//At this low frequency the movement code performs amazingly bad:
	//- The step checking fails half the time.
	//- Out of water jumping gets pretty difficult.
	//- Gravity seems to fail when walking down a stairs,
	//  which results in hoovering down the stairs...
	//To get a higer frequency of the Pmove calls the calls are doubled
	//here by calling the ClientThink twice. Note that ofcourse the
	//ucmd milli seconds are halved.
	//The AI still runs at 10 Hz (could be changed though).
	if (!botglobals.nocldouble)
	{
		if (!botglobals.nocldouble) ucmd.msec /= 2;
		ClientThink(bot, &ucmd);
		if (bi->actionflags & ACTION_DELAYEDJUMP) ucmd.upmove += 400;
		ClientThink(bot, &ucmd);
	} //end if
	else
	{
		//call the client think function to execute the usercmd_t of the bot
		ClientThink(bot, &ucmd);
	} //end else
	//
	bot->flags &= ~FL_BOTINPUT;
	//set the ping of the bot
	bot->client->ping = 1000 * bi->thinktime + crandom() * 20;
} //end of the function BotExecuteInput
//===========================================================================
// set the pmove_state_t of the bot
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotSetPMoveState(edict_t *bot)
{
	#define PMF_DUCKED			1			//ducked
	#define PMF_JUMP_HELD		2			//jump held down
	#define PMF_ON_GROUND		4			//set when on the ground
	#define PMF_TIME_WATERJUMP	8			//pm_time is waterjump
	#define PMF_TIME_LAND		16			//pm_time is time before rejump
	#define PMF_TIME_TELEPORT	32			//pm_time is non-moving time
	#define PMF_NO_PREDICTION	64			//temporarily disables prediction (used for grappling hook)
	#define PMF_UNUSED			128		//unused
	//
	// pmove_state_t				//everything is set when calling gi.Pmove
	//
	// pmtype_t pm_type;
	//		type of movement, set in ClientThink
	//
	// short origin[3];			// 12.3
	//		origin of the edict, override in ClientThink after being copied
	//
	// short velocity[3];		// 12.3
	//		velocity of the edict, override in ClientThink after being copied
	//
	// byte pm_flags;				// ducked, jump_held, etc
	//		several movement flags, set by the Quake2 engine for real clients
	//		see above for possible flags
	//
	// byte pm_time;
	//		pmove time, works with one of the PMF_TIME_? flags
	//
	// short gravity;
	//		the current gravity, set in ClientThink
	//
	// short delta_angles[3];	// add to command angles to get view direction
	//									// changed by spawns, rotating objects, and teleporters
	//		angles are added to the client angles during one client frame and
	//		after that the delta angles are cleared by the Quake2 engine
	//
	//the Quake2 engine does this for real clients
	VectorClear(bot->client->ps.pmove.delta_angles);
} //end of the function BotSetPMoveState
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_library_t *GetBotLibrary(edict_t *bot)
{
	bot_library_t *lib;

	lib = botglobals.botstates[DF_ENTCLIENT(bot)].library;
	if (!lib)
	{
		gi.dprintf("bot (client %d) without bot library\n", DF_ENTCLIENT(bot));
	} //end if
	return lib;
} //end of the function GetBotLibrary
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void ClientBegin(edict_t *ent);

qboolean BotStarted(edict_t *bot)
{
	bot_library_t *lib;

	//if the bot already started
	if (botglobals.botstates[DF_ENTCLIENT(bot)].started) return true;
	//get the library the bot uses
	lib = GetBotLibrary(bot);
	//if the library is initialized
	if (lib->funcs.BotLibraryInitialized())
	{
		//NOTE: set the inuse flag to false because the bot isn't loaded
		//			from a savegame
		bot->inuse = false;
		//the Quake2 server calls this function for real clients
		ClientBegin(bot);
		//
		bot->flags |= FL_BOT;
		//the bot has started
		botglobals.botstates[DF_ENTCLIENT(bot)].started = true;
		return true;
	} //end if
	return false;
} //end of the function BotStarted

//==========================================================================
//
// usage of imported functions from library
//
//==========================================================================

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotLib_BotLoadMap(char *mapname)
{
	bot_library_t *lib, *nextlib;
	int errno;

	for (lib = botglobals.firstbotlib; lib; lib = nextlib)
	{
		nextlib = lib->next;
		errno = lib->funcs.BotLoadMap(mapname, MAX_MODELINDEXES, modelindexes,
												MAX_SOUNDINDEXES, soundindexes,
												MAX_IMAGEINDEXES, imageindexes);
		if (errno != BLERR_NOERROR)
		{
			int i;
			edict_t *cl_ent;

			//remove all bots using this library
			for (i = 0; i < game.maxclients; i++)
			{
				cl_ent = g_edicts + 1 + i;
				if (!cl_ent->inuse) continue;
				if (!(cl_ent->flags & FL_BOT)) continue;
				if (GetBotLibrary(cl_ent) == lib)
				{
					BotDestroy(cl_ent);
				} //end if
			} //end for
		} //end if
	} //end for
} //end of the function BotLib_BotLoadMap
//==========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//==========================================================================
int BotLib_BotSetupClient(edict_t *ent, char *userinfo)
{
	char *s;
	bot_library_t *lib;
	bot_settings_t settings;

	lib = GetBotLibrary(ent);
	if (!lib) return false;
	memset(&settings, 0, sizeof(bot_settings_t));
	//
	s = Info_ValueForKey(userinfo, "charfile");
	strncpy(settings.characterfile, s, MAX_FILEPATH-1); //Riv++
	settings.characterfile[MAX_FILEPATH-1] = '\0';
	//
	s = Info_ValueForKey(userinfo, "charname");
	strncpy(settings.charactername, s, MAX_CHARACTERNAME-1); //Riv++
	settings.charactername[MAX_CHARACTERNAME-1] = '\0';
	//
	return lib->funcs.BotSetupClient(DF_ENTCLIENT(ent), &settings);
} //end of the function BotLib_BotSetupClient
//==========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//==========================================================================
void BotLib_BotShutdownClient(edict_t *client)
{
	bot_library_t *lib;

	lib = GetBotLibrary(client);
	if (!lib) return;
	lib->funcs.BotShutdownClient(DF_ENTCLIENT(client));
} //end of the function BotLib_BotShutDownClient
//==========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//==========================================================================
void BotLib_BotMoveClient(edict_t *oldclient, edict_t *newclient)
{
	bot_library_t *lib;

	lib = GetBotLibrary(oldclient);
	if (!lib) return;
	lib->funcs.BotMoveClient(DF_ENTCLIENT(oldclient), DF_ENTCLIENT(newclient));
} //end of the function BotLib_BotMoveClient
//==========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//==========================================================================
void BotLib_BotClientSettings(edict_t *ent)
{
	bot_clientsettings_t settings;
	bot_library_t *lib;

	if (!ent->client)
	{
		gi.dprintf("client %d without client structure\n", DF_ENTCLIENT(ent));
		return;
	} //end if
	//copy the client name
	strncpy(settings.netname, ent->client->pers.netname, MAX_NETNAME-1); //Riv++
	settings.netname[MAX_NETNAME-1] = '\0'; //Riv++
	//client skin
	strncpy(settings.skin,
		Info_ValueForKey(ent->client->pers.userinfo, "skin"), MAX_CLIENTSKINNAME-1); //Riv++
	settings.skin[MAX_CLIENTSKINNAME-1] = '\0'; //Riv++

	for (lib = botglobals.firstbotlib; lib; lib = lib->next)
	{
		lib->funcs.BotClientSettings(DF_ENTCLIENT(ent), &settings);
	} //end for
} //end of the function BotLib_BotClientSettings
//==========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//==========================================================================
void BotLib_UpdateAllClientSettings(void)
{
	int i;
	edict_t *ent;

	for (i = 0; i < game.maxclients; i++)
	{
		ent = DF_CLIENTENT(i);
		if (!ent->inuse) continue;
		BotLib_BotClientSettings(ent);
	} //end for
} //end of the function BotLib_UpdateAllClientSettings
//==========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//==========================================================================
void BotLib_BotSettings(edict_t *bot, bot_settings_t *settings)
{
} //end of the function BotLib_BotSettings
//==========================================================================
// sends a client (state) update to the bot library
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//==========================================================================
void BotLib_BotUpdateClient(edict_t *bot)
{
	bot_updateclient_t buc;
	bot_library_t *lib;
	int i;

	if (!bot->inuse) return;
	lib = GetBotLibrary(bot);
	if (!lib) return;

	if (!bot->client)
	{
		gi.dprintf("client %d without client structure\n", DF_ENTCLIENT(bot));
		return;
	} //end if
	//movement type
	buc.pm_type = bot->client->ps.pmove.pm_type;
	//origin of the bot
	VectorCopy(bot->s.origin, buc.origin);
	//velocity of the bot
	VectorCopy(bot->velocity, buc.velocity);
	//pm_flags
	buc.pm_flags = bot->client->ps.pmove.pm_flags;
	//pm_time
	buc.pm_time = bot->client->ps.pmove.pm_time;
	//gravity
	buc.gravity = sv_gravity->value;
	//delta_angles (NOTE: the bot->client->ps.pmove.delta_angles are of type short)
	VectorCopy(bot->client->ps.pmove.delta_angles, buc.delta_angles);
	//====================================
	//view angles
	VectorClear(buc.viewangles);
	//view offset
	VectorCopy(bot->client->ps.viewoffset, buc.viewoffset);
	//kick angles
	VectorCopy(bot->client->ps.kick_angles, buc.kick_angles);
	//gun angles
	VectorCopy(bot->client->ps.gunangles, buc.gunangles);
	//gun offset
	VectorCopy(bot->client->ps.gunoffset, buc.gunoffset);
	//gun index
	buc.gunindex = bot->client->ps.gunindex;
	//gun frame
	buc.gunframe = bot->client->ps.gunframe;
	//blend
	for (i = 0; i < 4; i++) buc.blend[i] = bot->client->ps.blend[i];
	//field of vision
	buc.fov = bot->client->ps.fov;
	//rdflags
	buc.rdflags = bot->client->ps.rdflags;
	//
	memcpy(buc.stats, bot->client->ps.stats, MAX_STATS * sizeof(short));
	//====================================
	//inventory
	memcpy(buc.inventory, bot->client->pers.inventory, MAX_ITEMS * sizeof(int));
	//update the client
	lib->funcs.BotUpdateClient(DF_ENTCLIENT(bot), &buc);
	//====================================
	BotSetPMoveState(bot);
} //end of the function BotLib_BotUpdateClient
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotLib_BotStartFrame(float time)
{
	bot_library_t *lib;

	//NOTE: not really nice to put this call here ... but it's functional
	BotLib_UpdateAllClientSettings();
	//
	for (lib = botglobals.firstbotlib; lib; lib = lib->next)
	{
		//set the dmflags
		lib->funcs.BotLibVarSet("dmflags", dmflags->string);
		//start the server frame
		lib->funcs.BotStartFrame(time);
	} //end for
} //end of the function BotLib_BotStartFrame
//===========================================================================
// sends an entity update to the bot library
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotLib_BotUpdateEntity(edict_t *ent)
{
	bot_updateentity_t bue;
	bot_library_t *lib;

	VectorCopy(ent->s.origin, bue.origin);
	VectorCopy(ent->s.angles, bue.angles);
	VectorCopy(ent->s.old_origin, bue.old_origin);
	VectorCopy(ent->mins, bue.mins);
	VectorCopy(ent->maxs, bue.maxs);
	bue.solid = ent->solid;
	bue.modelindex = ent->s.modelindex;
	bue.modelindex2 = ent->s.modelindex2;
	bue.modelindex3 = ent->s.modelindex3;
	bue.modelindex4 = ent->s.modelindex4;
	bue.frame = ent->s.frame;
	bue.skinnum = ent->s.skinnum;
	bue.effects = ent->s.effects;
	bue.renderfx = ent->s.renderfx;
	bue.sound = ent->s.sound;
	bue.event = ent->s.event;

#ifdef TOURNEY
	if (ent->item && (ent->item->flags & IT_RUNE))
	{
		if (ent->item->quantity == STAT_RUNE_RESIST) bue.modelindex = TECH1_INDEX;
		else if (ent->item->quantity == STAT_RUNE_STRENGTH) bue.modelindex = TECH2_INDEX;
		else if (ent->item->quantity == STAT_RUNE_HASTE) bue.modelindex = TECH3_INDEX;
		else if (ent->item->quantity == STAT_RUNE_REGEN) bue.modelindex = TECH4_INDEX;
		else if (ent->item->quantity == STAT_RUNE_VAMPIRE) bue.modelindex = TECH5_INDEX;
	} //end if
#endif //TOURNEY

	for (lib = botglobals.firstbotlib; lib; lib = lib->next)
	{
		if (lib->funcs.BotLibraryInitialized())
		{
			lib->funcs.BotUpdateEntity(DF_ENTNUMBER(ent), &bue);
		} //end if
	} //end for
	//if this is a client, FIXME: is this necessary?
	if (ent->client)
	{
		//if the entity has a sound
		if (ent->s.sound)
		{
			//send the sound seperately
			BotLib_BotAddSound(ent, CHAN_AUTO, ent->s.sound, 1.0, ATTN_IDLE, 0);
		} //end if
	} //end if
} //end of the function BotLib_BotUpdateEntity
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotLib_BotAddSound(edict_t *ent, int channel, int soundindex, float volume, float attenuation, float timeofs)
{
	bot_library_t *lib;
	int entnum;

	if (soundindex < 0 || soundindex > 255)
	{
//		gi.dprintf("soundindex %d out of range [0, %d]\n", soundindex, 255);
		return;
	} //end if
	entnum = DF_ENTNUMBER(ent);
	for (lib = botglobals.firstbotlib; lib; lib = lib->next)
	{
		if (lib->funcs.BotLibraryInitialized())
		{
			lib->funcs.BotAddSound(ent->s.origin, entnum, channel, soundindex, volume, attenuation, timeofs);
		} //end if
	} //end for
} //end of the function BotLib_BotAddSound
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotLib_BotAI(edict_t *bot, float thinktime)
{
	bot_library_t *lib;

#ifdef BOT_DEBUG
	if (botglobals.nobotai) return;
#endif //BOT_DEBUG

	lib = GetBotLibrary(bot);
	if (!lib) return;
	lib->funcs.BotAI(DF_ENTCLIENT(bot), thinktime);
} //end of the function BotLib_BotAI
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotLib_BotConsoleMessage(edict_t *bot, int type, char *message)
{
	bot_library_t *lib;

	lib = GetBotLibrary(bot);
	if (!lib) return;
	lib->funcs.BotConsoleMessage(DF_ENTCLIENT(bot), type, message);
} //end of the function BotLib_BotConsoleMessage
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotLib_BotLibVarSet(char *var_name, char *value)
{
	bot_library_t *lib;
	
	for (lib = botglobals.firstbotlib; lib; lib = lib->next)
	{
		lib->funcs.BotLibVarSet(var_name, value);
	} //end for
} //end of the function BotLib_BotLibVarSet
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int BotLib_Test(int parm0, char *parm1, vec3_t parm2, vec3_t parm3)
{
	bot_library_t *lib;
	
	for (lib = botglobals.firstbotlib; lib; lib = lib->next)
	{
		lib->funcs.Test(parm0, parm1, parm2, parm3);
	} //end for
	return 0;
} //end of the function BotLib_Test

//==========================================================================
//
// functions exported to the bot library
//
//==========================================================================

//===========================================================================
// stores the new bot input
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotLibImport_BotInput(int client, bot_input_t *bi)
{
	if (client < 0 || client >= game.maxclients)
	{
		gi.dprintf("BotInput: client number out of range\n");
		return;
	} //end if
	memcpy(&botglobals.botinputs[client], bi, sizeof(bot_input_t));
	botglobals.botnewinput[client] = true;
} //end of the function BotLibImport_BotInput
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotLibImport_Print(int type, char *fmt, ...)
{
	char str[2048];
	char warning[64] = "Warning: ";
	char error[64] = "Error: ";
	char fatal[64] = "Fatal: ";
	va_list ap;

	va_start(ap, fmt);
	vsprintf(str, fmt, ap);
	va_end(ap);

	switch(type)
	{
		case PRT_MESSAGE:
		{
			gi.dprintf("%s", str);
			//gi.bprintf(PRINT_HIGH, str);
			break;
		} //end case
		case PRT_WARNING:
		{
			StringMakeGreen(warning);
			gi.dprintf("%s%s", warning, str);
			break;
		} //end case
		case PRT_ERROR:
		{
			StringMakeGreen(error);
			gi.dprintf("%s%s", error, str);
			break;
		} //end case
		case PRT_FATAL:
		{
			StringMakeGreen(fatal);
			//gi.error("%s%s", fatal, str);
			gi.dprintf("%s%s", fatal, str);
			break;
		} //end case
		case PRT_EXIT:
		{
			StringMakeGreen(str);
			gi.error("Exit: %s", str);
			break;
		} //end case
		default:
		{
			gi.dprintf("unknown print type\n");
			break;
		} //end case
	} //end switch
} //end of the function BotLibImport_Print
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void *BotLibImport_GetMemory(int size)
{
	void *ptr;
	//NOTE: don't use TAG_LEVEL, because all that memory will be freed
	//      at level changes. The game library doesn't change during level
	//      changes except for a LoadMap call. The game library assumes
	//      the allocated memory will stay during level changes, so the
	//      memory should not be freed in the game dll.
	ptr = gi.TagMalloc(size, TAG_GAME);
	if (!ptr)
	{
		gi.error("out of memory\n");
	} //end if
	return ptr;
} //end of the function BotLibImport_GetMemory
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotLibImport_FreeMemory(void *ptr)
{
	gi.TagFree(ptr);
} //end of the function BotLibImport_FreeMemory
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bsp_trace_t BotLibImport_Trace(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, int passent, int contentmask)
{
	bsp_trace_t bsptrace;
	trace_t trace;
	edict_t *p;

	//and another dirty LCC warning prevention
	memset(&trace, 0, sizeof(trace_t));
	//just for the errors
	memset(&bsptrace, 0, sizeof(bsp_trace_t));
	//check for valid passent entity number
	if (passent < 0 || passent >= game.maxentities)
	{
		gi.dprintf("BotLibTrace: invalid passent\n");
		return bsptrace;
	} //end if
	p = DF_NUMBERENT(passent);
	//
	trace = gi.trace(start, mins, maxs, end, p, contentmask);
	memcpy(bsptrace.surface.name, trace.surface->name, 16);
	bsptrace.surface.flags = trace.surface->flags;
	bsptrace.surface.value = trace.surface->value;
	bsptrace.allsolid = trace.allsolid;
	bsptrace.startsolid = trace.startsolid;
	bsptrace.fraction = trace.fraction;
	VectorCopy(trace.endpos, bsptrace.endpos);
	bsptrace.ent = DF_ENTNUMBER(trace.ent);
	bsptrace.contents = trace.contents;
	memcpy(&bsptrace.plane, &trace.plane, sizeof(cplane_t));
#ifdef __LCC__ //Riv++ Prevent dll from crashing, heh... Some issues remain though
  gi.dprintf("");
#endif
	return bsptrace;
} //end of the function BotLibImport_Trace

//==========================================================================
//
// bot library loading, initialization etc.
//
//==========================================================================

//===========================================================================
// initialize the bot library
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int BotInitLibrary(bot_library_t *lib)
{
	int errno;
	cvar_t *cvar;
	char buf[144];

	//set the maxclients and maxentities library variables before calling BotSetupLibrary
	lib->funcs.BotLibVarSet("maxclients", maxclients->string);
	lib->funcs.BotLibVarSet("maxentities", maxentities->string);
	//maximum number of aas links
	cvar = gi.cvar("max_aaslinks", "", 0);
	if (cvar && cvar->value > 0) lib->funcs.BotLibVarSet("max_aaslinks", cvar->string);
	//maximum number of bsp links
	cvar = gi.cvar("max_bsplinks", "", 0);
	if (cvar && cvar->value > 0) lib->funcs.BotLibVarSet("max_bsplinks", cvar->string);
	//maximum number of items in a level
	cvar = gi.cvar("max_levelitems", "", 0);
	if (cvar && cvar->value > 0) lib->funcs.BotLibVarSet("max_levelitems", cvar->string);
	//automatically launch WinBSPC if AAS file not available
	cvar = gi.cvar("autolaunchbspc", "", 0);
	if (cvar && cvar->value) lib->funcs.BotLibVarSet("autolaunchbspc", "1");
	//deathmatch flags
	lib->funcs.BotLibVarSet("dmflags", dmflags->string);
	sprintf(buf, "DMFLAGS %s", dmflags->string);
	lib->funcs.BotDefine(buf);
#ifdef ZOID
	lib->funcs.BotLibVarSet("ctf", ctf->string);
	if (ctf->value)
	{
		lib->funcs.BotLibVarSet("usehook", "1");
		lib->funcs.BotLibVarSet("runes", "1");
	} //end if
#endif //ZOID
#ifdef CH
	lib->funcs.BotLibVarSet("ch", ch->string);
#endif //CH
#ifdef ROCKETARENA
	lib->funcs.BotLibVarSet("ra", ra->string);
#endif //ROCKETARENA
#ifdef XATRIX
	lib->funcs.BotLibVarSet("xatrix", xatrix->string);
#endif //XATRIX
#ifdef ROGUE
	lib->funcs.BotLibVarSet("rogue", rogue->string);
#endif //ROGUE
	//log file
	cvar = gi.cvar("log", "1", 0);
	lib->funcs.BotLibVarSet("log", cvar->string);
	//no chatting
	cvar = gi.cvar("nochat", "", 0);
	if (cvar && cvar->value) lib->funcs.BotLibVarSet("nochat", "1");
	//fast chatting
	cvar = gi.cvar("fastchat", "", 0);
	if (cvar && cvar->value) lib->funcs.BotLibVarSet("fastchat", "0");
	//alternative names
	cvar = gi.cvar("altnames", "", 0);
	if (cvar && cvar->value) lib->funcs.BotLibVarSet("altnames", "1");
	//enable rocket jumping
	cvar = gi.cvar("rocketjump", "1", 0);
	if (cvar && cvar->value) lib->funcs.BotLibVarSet("rocketjump", "1");
	//forced clustering calculations
	cvar = gi.cvar("forceclustering", "", 0);
	if (cvar && cvar->value) lib->funcs.BotLibVarSet("forceclustering", "1");
	//forced reachability calculations
	cvar = gi.cvar("forcereachability", "", 0);
	if (cvar && cvar->value) lib->funcs.BotLibVarSet("forcereachability", "1");
	//force writing of AAS to file
	cvar = gi.cvar("forcewrite", "", 0);
	if (cvar && cvar->value) lib->funcs.BotLibVarSet("forcewrite", "1");
	//no AAS optimization
	cvar = gi.cvar("nooptimize", "", 0);
	if (cvar && cvar->value) lib->funcs.BotLibVarSet("nooptimize", "1");
	//number of reachabilities to calculate each frame
	cvar = gi.cvar("framereachability", "20", 0);
	lib->funcs.BotLibVarSet("framereachability", cvar->string);
	//base directory
	cvar = gi.cvar("basedir", "", 0);
	if (cvar) lib->funcs.BotLibVarSet("basedir", cvar->string);
	else lib->funcs.BotLibVarSet("basedir", "");
	//game directory
	cvar = gi.cvar("gamedir", "", 0);
	if (cvar) lib->funcs.BotLibVarSet("gamedir", cvar->string);
	else lib->funcs.BotLibVarSet("gamedir", "");
	//cd directory
	cvar = gi.cvar("cddir", "", 0);
	if (cvar) lib->funcs.BotLibVarSet("cddir", cvar->string);
	else lib->funcs.BotLibVarSet("cddir", "");
	//setup the bot library
	errno = lib->funcs.BotSetupLibrary();
	if (errno != BLERR_NOERROR) return false;
	//load the map
	errno = lib->funcs.BotLoadMap(level.mapname, MAX_MODELINDEXES, modelindexes,
																MAX_SOUNDINDEXES, soundindexes,
																MAX_IMAGEINDEXES, imageindexes);
	if (errno != BLERR_NOERROR) return false;
#ifdef TOURNEY
	// Handle the Tourney hook --JKK
	if((int)hook_enable->value)
	{
		lib->funcs.BotLibVarSet("usehook", "1");
		lib->funcs.BotLibVarSet("laserhook", "1");
	} //end if
	else
	{
		lib->funcs.BotLibVarSet("usehook", "0");
		lib->funcs.BotLibVarSet("laserhook", "0");
	} //end else

	if(m_mode == MODE_TEAM)
		lib->funcs.BotLibVarSet("teamplay", "1");
	else
		lib->funcs.BotLibVarSet("teamplay", "0");

	lib->funcs.BotLibVarSet("log", "0");
#endif //TOURNEY
	return true;
} //end of the function BotInitLibrary
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotUnloadLibrary(bot_library_t *lib)
{
	//unlink library from list
	if (lib->prev) lib->prev->next = lib->next;
	else botglobals.firstbotlib = lib->next;
	if (lib->next) lib->next->prev = lib->prev;
	//shut down the library
	lib->funcs.BotShutdownLibrary();
#if defined(WIN32) || defined(_WIN32)
	//Win32 free the bot library
	FreeLibrary(lib->handle);
#else
	//free the shared object
	dlclose(lib->handle);
#endif
	//free the memory of the library structure
	gi.TagFree(lib);
} //end of the function BotUnloadLibrary
//===========================================================================
// bot library loading
//
// NOTE: this is platform dependent code
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
#if defined(WIN32) || defined(_WIN32)

typedef bot_export_t *(WINAPI *PFNGetBotAPI)(bot_import_t *import);

bot_library_t *BotLoadLibrary(char *botlibdir)
{
	bot_library_t *lib = NULL;
	PFNGetBotAPI GetBotAPI = NULL;
	HANDLE botlibhandle = NULL;

	//load the library
	botlibhandle = LoadLibrary(botlibdir);
	if (botlibhandle == NULL)
	{
		gi.dprintf("couldn't load %s\n", botlibdir);
	} //end if
	else
	{
		GetBotAPI = (PFNGetBotAPI) GetProcAddress(botlibhandle, "GetBotAPI");
		if (GetBotAPI == NULL)
		{
			FreeLibrary(botlibhandle);
			gi.dprintf("couldn't find GetBotAPI in %s\n", botlibdir);
		} //end if
		else
		{
			lib = (bot_library_t *) gi.TagMalloc(sizeof(bot_library_t), TAG_GAME);
			memset(lib, 0, sizeof(bot_library_t));
			strncpy(lib->path, botlibdir, MAX_PATH-1); //Riv++
			lib->path[MAX_PATH-1] = '\0';
			lib->handle = botlibhandle;
			lib->funcs = (*GetBotAPI(&botglobals.gamebotimport));
			//add the library to the list
			lib->next = botglobals.firstbotlib;
			lib->prev = NULL;
			if (botglobals.firstbotlib) botglobals.firstbotlib->prev = lib;
			botglobals.firstbotlib = lib;
			//initialize library
			if (!BotInitLibrary(lib))
			{
				BotUnloadLibrary(lib);
				return NULL;
			} //end if
			//
			gi.dprintf("loaded %s\n", botlibdir);
		} //end else
	} //end else
	return lib;
} //end of the function BotLoadLibrary

#else //!win32

bot_library_t *BotLoadLibrary(char *botlibdir)
{
	bot_library_t *lib = NULL;
	bot_export_t *(*GetBotAPI)(bot_import_t *import);
	void *botlibhandle;
	const char *e;

	//load the library
	botlibhandle = dlopen(botlibdir, RTLD_NOW);
	if (!botlibhandle)
	{
		gi.dprintf("couldn't load %s: %s\n", botlibdir, dlerror());
	} //end if
	else
	{
		GetBotAPI = dlsym(botlibhandle, "GetBotAPI");
		e = dlerror();
		if (e)
		{
			dlclose(botlibhandle);
			gi.dprintf("couldn't find GetBotAPI in %s: %s\n", botlibdir, e);
		} //end if
		else
		{
			lib = (bot_library_t *) gi.TagMalloc(sizeof(bot_library_t), TAG_GAME);
			memset(lib, 0, sizeof(bot_library_t));
			strncpy(lib->path, botlibdir, MAX_PATH-1); //Riv++
			lib->path[MAX_PATH-1] = '\0';
			lib->handle = botlibhandle;
			lib->funcs = (*GetBotAPI(&botglobals.gamebotimport));
			//add the library to the list
			lib->next = botglobals.firstbotlib;
			lib->prev = NULL;
			if (botglobals.firstbotlib) botglobals.firstbotlib->prev = lib;
			botglobals.firstbotlib = lib;
			//initialize library
			if (!BotInitLibrary(lib))
			{
				BotUnloadLibrary(lib);
				return NULL;
			} //end if
			//
			gi.dprintf("loaded %s\n", botlibdir);
		} //end else
	} //end else
	return lib;
} //end of the function BotLoadLibrary
#endif
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotUnloadAllLibraries(void)
{
	bot_library_t *lib;

	for (lib = botglobals.firstbotlib; lib; lib = botglobals.firstbotlib)
	{
		BotUnloadLibrary(lib);
	} //end for
} //end of the function BotUnloadAllLibraries
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
bot_library_t *BotUseLibrary(char *path)
{
	cvar_t *cvar;
	char botlibdir[MAX_PATH] = "";
	bot_library_t *lib;

	//if the file is not directly accessable
	if (access(path, 0x04))
	{
		//get the base directory
		cvar = gi.cvar("basedir", "", 0);
		if (cvar) strncat(botlibdir, cvar->string, MAX_PATH-1); //Riv++
		botlibdir[MAX_PATH-1] = '\0'; //Riv++
		AppendPathSeperator(botlibdir, MAX_PATH);
		//user specified game directory
		cvar = gi.cvar("gamedir", "", 0);
		if (cvar) strncat(botlibdir, cvar->string, MAX_PATH - strlen(botlibdir) - 1); //Riv++
		botlibdir[MAX_PATH-1] = '\0'; //Riv++
		AppendPathSeperator(botlibdir, MAX_PATH);
	} //end if
	//the dll name
	strncat(botlibdir, path, MAX_PATH - strlen(botlibdir) - 1); //Riv++
	botlibdir[MAX_PATH-1] = '\0'; //Riv++
	//check if the library is loaded already
	for (lib = botglobals.firstbotlib; lib; lib = lib->next)
	{
		if (!Q_stricmp(lib->path, botlibdir))
		{
			lib->users++;
			return lib;
		} //end if
	} //end for
	if (botglobals.firstbotlib)
	{
		return NULL;
	} //end if
	lib = BotLoadLibrary(botlibdir);
	if (lib) lib->users++;
	return lib;
} //end of the function BotUseLibrary
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotFreeLibrary(bot_library_t *lib)
{
	cvar_t *freebotlib;

	lib->users--;
	if (lib->users <= 0)
	{
		freebotlib = gi.cvar("freebotlib", "1", 0);
		if (freebotlib->value) BotUnloadLibrary(lib);
	} //end if
} //end of the function BotFreeLibrary
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotLibraryDump(void)
{
	bot_library_t *lib;
	bot_state_t *bs;
	edict_t *ent;
	int i;

	gi.dprintf("Library Dump:\n");
	if (!botglobals.firstbotlib)
	{
		gi.dprintf("no libraries found\n");
	} //end if
	else
	{
		for (lib = botglobals.firstbotlib; lib; lib = lib->next)
		{
			gi.dprintf("-------------------------------------\n");
			gi.dprintf("%s\n", lib->path);
			for (i = 1; i <= game.maxclients; i++)
			{
				bs = &botglobals.botstates[i-1];
				if (bs->active)
				{
					if (bs->library == lib)
					{
						ent = DF_NUMBERENT(i);
						gi.dprintf("    client %3d: %s\n", i-1, ent->client->pers.netname);
					} //end if
				} //end if
			} //end for
		} //end for
	} //end else
} //end of the function BotLibraryDump
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
char *Ptr2PathWithMaxSize(char *path, int size)
{
	int length;
	char *ptr, *bestptr;

	bestptr = path;
	length = strlen(bestptr);
	if (length > size)
	{
		ptr = &bestptr[length - 1];
		bestptr = NULL;
		for (length = 0; length < size; length++)
		{
			if (*ptr == '\\' || *ptr == '/') bestptr = ptr;
			ptr--;
		} //end for
		if (!bestptr) bestptr = ptr;
	} //end if
	return bestptr;
} //end of the function Ptr2PathWithMaxSize
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotClientDump(void)
{
	edict_t *ent;
	int i;
	char *path;

	for (i = 0; i < game.maxclients; i++)
	{
		ent = DF_CLIENTENT(i);
		if (!ent->inuse)
		{
			gi.dprintf("%3d: -\n", i);
		} //end if
		else if (ent->flags & FL_BOT)
		{
			gi.dprintf("%3d: %-16s ", i, ent->client->pers.netname);
			path = botglobals.botstates[i].library->path;
			if (strlen(path) > 25)
			{
				//minus three for the dots
				path = Ptr2PathWithMaxSize(path, 25-3);
				gi.dprintf("...");
			} //end if
			gi.dprintf("%s\n", path);
		} //end if
		else
		{
			gi.dprintf("%3d: %-16s human\n", i, ent->client->pers.netname);
		} //end else
	} //end for
} //end of the function BotClientDump
//===========================================================================
// setup the bot library import functions
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotSetupBotLibImport(void)
{
	bot_import_t gamebotimport;

	gamebotimport.BotInput = BotLibImport_BotInput;
	gamebotimport.BotClientCommand = BotClientCommand;		//bl_redirgi.c
	gamebotimport.Print = BotLibImport_Print;
	//to be removed later below this
	gamebotimport.Trace = BotLibImport_Trace;
	gamebotimport.PointContents = gi.pointcontents;
	gamebotimport.GetMemory = BotLibImport_GetMemory;
	gamebotimport.FreeMemory = BotLibImport_FreeMemory;
	//debug lines
	gamebotimport.DebugLineCreate = DebugLineCreate;		//bl_debug.c
	gamebotimport.DebugLineDelete = DebugLineDelete;		//bl_debug.c
	gamebotimport.DebugLineShow = DebugLineShow;				//bl_debug.c
	//
	memcpy(&botglobals.gamebotimport, &gamebotimport, sizeof(bot_import_t));
} //end of the function BotSetupLibrary
//===========================================================================
// initialize bot globals and allocate bot states
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotSetup(void)
{
	//allocate memory for the bot states
	botglobals.botstates = (bot_state_t *) gi.TagMalloc(game.maxclients * sizeof(bot_state_t), TAG_GAME);
	memset(botglobals.botstates, 0, game.maxclients * sizeof(bot_state_t));
	//allocate memory for the bot input
	botglobals.botinputs = (bot_input_t *) gi.TagMalloc(game.maxclients * sizeof(bot_input_t), TAG_GAME);
	memset(botglobals.botinputs, 0, game.maxclients * sizeof(bot_input_t));
	//allocate memory for the array with new input flags
	botglobals.botnewinput = (qboolean *) gi.TagMalloc(game.maxclients * sizeof(qboolean), TAG_GAME);
	memset(botglobals.botnewinput, 0, game.maxclients * sizeof(qboolean));
	//number of bots currently in the game
	botglobals.numbots = 0;
	//setup the bot library import structure
	BotSetupBotLibImport();
} //end of the function BotSetup

#endif //BOT
