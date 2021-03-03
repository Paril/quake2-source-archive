//===========================================================================
//
// Name:				bl_redirgi.c
// Function:		redirect the game import structure
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1999-02-10
// Tab Size:		3
//===========================================================================

#include "g_local.h"

#ifdef BOT

#include "bl_main.h"
#include "bl_redirgi.h"

#define MAX_COMMANDARGUMENTS			20
#define MAX_NETWORKMESSAGE				2048
#define MAX_MUZZLEFLASHES				32

//network message
typedef struct bot_networkmessage_s
{
	int writebyte;											//current byte to write in message
	int readbyte;											//current byte to read from the message
	unsigned char message[MAX_NETWORKMESSAGE];	//message data
} bot_networkmessage_t;
//muzzle flash information
typedef struct bot_muzzleflashinfo_s
{
	int muzzleflash;		//muzzle flash number
	char *sound;			//name of the sound
	float radius;			//light radius
	float r, g, b;			//rgb light color
	float time;				//light alive time
	float decay;			//light decay
} bot_muzzleflashinfo_t;

//the gi will be redirected through botimport
game_import_t newgameimport;
//command arguments for the bots
char *commandarguments[MAX_COMMANDARGUMENTS];
char commandline[150]; //max see g_cmds.c
//last created cvar
cvar_t *lastcreatedcvar;
//the model and sound indexes
char *modelindexes[MAX_MODELINDEXES];
char *soundindexes[MAX_SOUNDINDEXES];
char *imageindexes[MAX_SOUNDINDEXES];
//global network message
bot_networkmessage_t networkmessage;
//soundindex of sound played with muzzleflash
int muzzleflashsoundindex[MAX_MUZZLEFLASHES];
//
#define NUMVERTEXNORMALS	162

float	avertexnormals[NUMVERTEXNORMALS][3] =
{
#include "anorms.h"
};

/*

g_weapons.c
-----------

//Grenade_Touch
//GRENade Launcher Bounce 1b
gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
//bfg_touch
//BFG eXplosion 1b (bfg ball hit)
gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
//fire_bfg
//BFG Launch 1a (bfg ball movement)
bfg->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");
//fire_rocket
//"weapons/rockfly.wav"		//ROCKet FLY sound
rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");

"weapons/grenlx1a.wav"		//GRENade Launcher eXplosion 1a
"weapons/rocklx1a.wav"		//ROCKet Launcher eXplosion 1a


entity_state_t->sound
---------------------

"weapons/chngnu1a.wav",		//CHaiNGuN start Up 1a (zoom up)
"weapons/chngnl1a.wav",		//CHaiNGuN Launch 1a (zoom)
"weapons/chngnd1a.wav",		//CHaiNGuN shut Down 1a (zoom down)

"weapons/hyprbl1a.wav",		//HYPeRBlaster Lauch (constant zooming)
"weapons/hyprbd1a.wav",		//HYPeRBlaster shut Down of weapon
"weapons/hyprbu1a.wav",		//HYPeRBlaster start Up of weapon (unused?)

"weapons/bfg_hum.wav",		//BFG humming sound
*/

//muzzle flash information
bot_muzzleflashinfo_t muzzleflashinfo[MAX_MUZZLEFLASHES] =
{
//Blaster
	{MZ_BLASTER,		"weapons/blastf1a.wav"},	//BLASTer Fire
//Shotgun
	{MZ_SHOTGUN,		"weapons/shotgf1b.wav"},	//SHOTGun Fire 1b
							//"weapons/shotgr1b.wav		//SHOTGun Reload 1b
//Super Shotgun
	{MZ_SSHOTGUN,		"weapons/sshotf1b.wav"},	//Super SHOTgun Fire 1b
							//"weapons/sshotr1b.wav",	//Super SHOTgun Reload 1b
//Machinegun
	{MZ_MACHINEGUN,	"weapons/machgf1b.wav"},	//MACHineGun Fire 1b (tock)
							//"weapons/machgf2b",		//same tock, different sound frequency
							//"weapons/machgf3b",		//etc.
							//"weapons/machgf4b",
							//"weapons/machgf5b"
//Chaingun
	{MZ_CHAINGUN1,		"weapons/machgf1b.wav"},	//firing one bullet (machine gun sound)
	{MZ_CHAINGUN2,		"weapons/machgf1b.wav"},	//firing two bullets (machine gun sound)
	{MZ_CHAINGUN3,		"weapons/machgf1b.wav"},	//firing three bulltes (machine gun sound)
//Grenade Launcher
	{MZ_GRENADE,		"weapons/grenlf1a.wav"},	//GRENade Launcher Fire 1a
							//"weapons/grenlr1a.wav",	//GRENade Launcher Reload 1a
//Rocket launcher
	{MZ_ROCKET,			"weapons/rocklf1a.wav"},	//ROCKet Launcher Fire 1a
							//"weapons/rocklr1a.wav",	//ROCKet Launcher Reload 1a
//Hyperblaster
	{MZ_HYPERBLASTER,	"weapons/hyprbf1a.wav"},	//HYPeRBlaster Fire 1a
//Railgun
	{MZ_RAILGUN,		"weapons/railgf1a.wav"},	//RAILGun Fire 1a
							//"weapons/railgr1a.wav",	//RAILGun Reload 1a (unused ?)
//BFG
	{MZ_BFG,				"weapons/bfg__f1y.wav"},	//BFG Fire 1y
	//
	{MZ_LOGIN,			NULL},
	{MZ_LOGOUT,			NULL},
	{MZ_RESPAWN,		"misc/spawn1.wav"},
	{MZ_ITEMRESPAWN,	"items/respawn1.wav"},
	//{MZ_BOOMERGUN,		NULL}, //only in < v3.20
#ifdef XATRIX
	//RAFAEL
	{MZ_IONRIPPER,		NULL},
	{MZ_BLUEHYPERBLASTER,		NULL},
	{MZ_PHALANX,		NULL},
	{MZ_SILENCED,		NULL},	// bit flag ORed with one of the above numbers
	//END RAFAEL
#endif //XATRIX
#ifdef ROGUE
	//ROGUE
	{MZ_ETF_RIFLE,		NULL},
	{MZ_UNUSED,			NULL},
	{MZ_SHOTGUN2,		NULL},
	{MZ_HEATBEAM,		NULL},
	{MZ_BLASTER2,		NULL},
	{MZ_TRACKER,		NULL},
	{MZ_NUKE1,			NULL},
	{MZ_NUKE2,			NULL},
	{MZ_NUKE4,			NULL},
	{MZ_NUKE8,			NULL},
	//END ROGUE
#endif //ROGUE
	{-1, NULL}
};

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void ClearIndexes(void)
{
	//NOTE: at level changes the individual strings are already freed
	//      with gi.FreeTags(TAG_LEVEL), so there's no need to free them
	//      here, we'll just clear the pointer arrays (indexes)
	memset(modelindexes, 0, MAX_MODELINDEXES * sizeof(char *));
	memset(soundindexes, 0, MAX_SOUNDINDEXES * sizeof(char *));
	memset(imageindexes, 0, MAX_IMAGEINDEXES * sizeof(char *));
	memset(muzzleflashsoundindex, 0, MAX_MUZZLEFLASHES * sizeof(int)); //Riv++
} //end of the function ClearIndexes
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotInitMuzzleFlashToSoundindex(void)
{
	int i, j, mf;

	for (i = 0; muzzleflashinfo[i].muzzleflash >= 0; i++)
	{
		mf = muzzleflashinfo[i].muzzleflash;
		//if the muzzle flash is valid
		if (mf >= 0 && mf < MAX_MUZZLEFLASHES)
		{
			//if the muzzle flash uses sounds
			if (muzzleflashinfo[i].sound)
			{
				for (j = 0; j < MAX_SOUNDINDEXES; j++)
				{
					if (soundindexes[j])
					{
						if (!Q_stricmp(soundindexes[j], muzzleflashinfo[i].sound))
						{
							muzzleflashsoundindex[mf] = j;
//							gi.dprintf("mf %d: sound %s, index %d\n", mf, soundindexes[j], j);
							break;
						} //end if
					} //end if
				} //end for
			} //end if
		} //end if
	} //end for
} //end of the function BotInitMuzzleFlashToSoundindex
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotDumpModelindex(void)
{
	int i;
	for (i = 0; i < MAX_MODELINDEXES; i++)
	{
		gi.dprintf("%3d: %s\n", i, modelindexes[i]);
	} //end for
} //end of the function BotDumpModelindex
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotDumpSoundindex(void)
{
	int i;
	for (i = 0; i < MAX_SOUNDINDEXES; i++)
	{
		gi.dprintf("%3d: %s\n", i, soundindexes[i]);
	} //end for
} //end of the function BotDumpSoundindex
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotDumpImageindex(void)
{
	int i;
	for (i = 0; i < MAX_IMAGEINDEXES; i++)
	{
		gi.dprintf("%3d: %s\n", i, imageindexes[i]);
	} //end for
} //end of the function BotDumpImageindex
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotClearCommandArguments(void)
{
	int i;
	for (i = 0; i < MAX_COMMANDARGUMENTS; i++)
	{
		commandarguments[i] = NULL;
	} //end for
} //end of the function BotClearCommandArguments
//===========================================================================
// terminate the arguments with a NULL string
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void ClientCommand (edict_t *ent);

void BotClientCommand(int client, char *str, ...)
{
	int i;
	edict_t *clent;
	va_list ap;

	BotClearCommandArguments();
	commandarguments[0] = str;
	va_start(ap, str);
	for (i = 1; i < MAX_COMMANDARGUMENTS; i++)
	{
		commandarguments[i] = va_arg(ap, char *);
		if (!commandarguments[i]) break;
	} //end for
	va_end(ap);
	if (i >= MAX_COMMANDARGUMENTS)
	{
		newgameimport.error("BotClientCommand: too many arguments");
	} //end if
	commandline[0] = '\0';
	for (i = 1; i < MAX_COMMANDARGUMENTS; i++)
	{
		if (!commandarguments[i]) break;
		if (i > 1) strcat(commandline, " ");
		strcat(commandline, commandarguments[i]);
	} //end for
	clent = g_edicts + 1 + client;
	ClientCommand(clent);
	BotClearCommandArguments();
} //end of the function BotClientCommand
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void ServerCommand(void);

void BotServerCommand(char *str, ...)
{
	int i;
	va_list ap;

	BotClearCommandArguments();
	commandarguments[0] = str;
	va_start(ap, str);
	for (i = 1; i < MAX_COMMANDARGUMENTS; i++)
	{
		commandarguments[i] = va_arg(ap, char *);
		if (!commandarguments[i]) break;
	} //end for
	va_end(ap);
	if (i >= MAX_COMMANDARGUMENTS)
	{
		newgameimport.error("BotClientCommand: too many arguments");
	} //end if
	commandline[0] = '\0';
	for (i = 1; i < MAX_COMMANDARGUMENTS; i++)
	{
		if (!commandarguments[i]) break;
		if (i > 1) strcat(commandline, " ");
		strcat(commandline, commandarguments[i]);
	} //end for
	ServerCommand();
	BotClearCommandArguments();
} //end of the function BotServerCommand
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotStoreClientCommand(char *str, ...)
{
	int i;
	va_list ap;

	BotClearCommandArguments();
	commandarguments[0] = str;
	va_start(ap, str);
	for (i = 1; i < MAX_COMMANDARGUMENTS; i++)
	{
		commandarguments[i] = va_arg(ap, char *);
		if (!commandarguments[i]) break;
	} //end for
	va_end(ap);
	if (i >= MAX_COMMANDARGUMENTS)
	{
		newgameimport.error("BotClientCommand: too many arguments");
	} //end if
	commandline[0] = '\0';
	for (i = 1; i < MAX_COMMANDARGUMENTS; i++)
	{
		if (!commandarguments[i]) break;
		if (i > 1) strcat(commandline, " ");
		strcat(commandline, commandarguments[i]);
	} //end for
} //end of the function BotStoreClientCommand
//===========================================================================
// returns the cvar with the given name
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
/*
cvar_t *BotGet_cvar(char *var_name)
{
	cvar_t *cvar;

	if (!lastcreatedcvar)
	{
		//create a dummy cvar at the start of the Quake2 cvar list
		lastcreatedcvar = newgameimport.cvar("dummycvar", "0", CVAR_SERVERINFO);
	} //end if
	//go through the list of cvars
	for (cvar = lastcreatedcvar; cvar; cvar = cvar->next)
	{
		if (Q_strcasecmp(var_name, cvar->name) == 0)
		{
			return cvar;
		} //end if
	} //end for
	return NULL;
} //end of the function BotGet_cvar*/
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_cprintf(edict_t *ent, int printlevel, char *fmt, ...)
{
	char str[2048];
	va_list ap;

	//if (!ent) return; //Riv++ ent == NULL means print to server console (dedicated or not)

	va_start(ap, fmt);
	vsprintf(str, fmt, ap);
	if (ent && ent->flags & FL_BOT) //Riv++
	{
		if (printlevel == PRINT_CHAT) BotLib_BotConsoleMessage(ent, CMS_CHAT, str);
		else BotLib_BotConsoleMessage(ent, CMS_NORMAL, str);
	} //end if
	else
	{
		newgameimport.cprintf(ent, printlevel, "%s", str);
	} //end else
	va_end(ap);
} //end of the function Bot_cprintf
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_centerprintf(edict_t *ent, char *fmt, ...)
{
	char str[2048];
	va_list ap;

	if (!(ent->flags & FL_BOT))
	{
		va_start(ap, fmt);
		vsprintf(str, fmt, ap);
		newgameimport.centerprintf(ent,  "%s", str);
		va_end(ap);
	} //end if
} //end of the function Bot_centerprintf
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_bprintf(int printlevel, char *fmt, ...)
{
	int j;
	char str[2048];
	edict_t *other;
	va_list ap;

	va_start(ap, fmt);
	vsprintf(str, fmt, ap);
	newgameimport.bprintf(printlevel, "%s", str);
	va_end(ap);

	//print the message for all the bots
	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse) continue;
		if (!other->client) continue;
		if (!(other->flags & FL_BOT)) continue;
		BotLib_BotConsoleMessage(other, CMS_NORMAL, str);
	} //end for
} //end of the function Bot_bprintf
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_sound(edict_t *ent, int channel, int soundindex, float volume, float attenuation, float timeofs)
{
	BotLib_BotAddSound(ent, channel, soundindex, volume, attenuation, timeofs);
	newgameimport.sound(ent, channel, soundindex, volume, attenuation, timeofs);
} //end of the function Bot_sound
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int Bot_modelindex(char *name)
{
	int i;

	i = newgameimport.modelindex(name);
	if (i < 0 || i >= MAX_MODELINDEXES)
	{
		newgameimport.error("modelindex out of range\n");
	} //end if
	//if the modelindex isn't set yet
	if (!modelindexes[i] && name)
	{
		modelindexes[i] = (char *) newgameimport.TagMalloc(strlen(name) + 1, TAG_LEVEL);
		strcpy(modelindexes[i], name);
		//if there's already a bot library loaded
		if (botglobals.firstbotlib)
		{
			//gi.dprintf("WARNING: bot library already loaded when precaching model %s\n", name);
			BotLib_BotLoadMap(NULL);
		} //end if
	} //end if
	return i;
} //end of the function Bot_modelindex
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int Bot_soundindex(char *name)
{
	int i;

	i = newgameimport.soundindex(name);
	if (i < 0 || i >= MAX_SOUNDINDEXES)
	{
		newgameimport.error("soundindex out of range\n");
	} //end if
	//if the soundindex isn't set yet
	if (!soundindexes[i] && name)
	{
		soundindexes[i] = (char *) newgameimport.TagMalloc(strlen(name) + 1, TAG_LEVEL);
		strcpy(soundindexes[i], name);
		//if there's already a bot library loaded
		if (botglobals.firstbotlib)
		{
			//gi.dprintf("WARNING: bot library already loaded when precaching sound %s\n", name);
			BotLib_BotLoadMap(NULL);
		} //end if
	} //end if
	return i;
} //end of the function Bot_soundindex
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int Bot_imageindex(char *name)
{
	int i;

	i = newgameimport.imageindex(name);
	if (i < 0 || i >= MAX_IMAGEINDEXES)
	{
		newgameimport.error("imageindex out of range\n");
	} //end if
	//if the imageindex isn't set yet
	if (!imageindexes[i] && name)
	{
		imageindexes[i] = (char *) newgameimport.TagMalloc(strlen(name) + 1, TAG_LEVEL);
		strcpy(imageindexes[i], name);
		//if there's already a bot library loaded
		if (botglobals.firstbotlib)
		{
			//gi.dprintf("WARNING: bot library already loaded when precaching image %s\n", name);
			BotLib_BotLoadMap(NULL);
		} //end if
	} //end if
	return i;
} //end of the function Bot_imageindex
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_setmodel(edict_t *ent, char *name)
{
	int i;

	newgameimport.setmodel(ent, name);
	i = ent->s.modelindex;
	if (i < 0 || i >= MAX_MODELINDEXES)
	{
		newgameimport.error("modelindex out of range\n");
	} //end if
	//if the modelindex isn't set yet
	if (!modelindexes[i])
	{
		modelindexes[i] = (char *) newgameimport.TagMalloc(strlen(name) + 1, TAG_LEVEL);
		strcpy(modelindexes[i], name);
	} //end if
} //end of the function Bot_setmodel
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int ReadShort(bot_networkmessage_t *m)
{
	int c;

	c = (int) m->message[m->readbyte] + ((int) m->message[m->readbyte+1] << 8);
	m->readbyte += 2;
	return c;
} //end of the function ReadShort
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int ReadByte(bot_networkmessage_t *m)
{
	int c;

	c = m->message[m->readbyte];
	m->readbyte++;
	return c;
} //end of the function ReadByte
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotWriteMessage(bot_networkmessage_t *m)
{
	int i;

	for (i = 0; i < m->writebyte; i++)
	{
		newgameimport.WriteByte(m->message[i]);
	} //end for
	//gi.dprintf("network message written\n");
} //end of the function BotWriteMessage
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotClearMessage(void)
{
	//clear the global network message
	networkmessage.writebyte = 0;
	networkmessage.readbyte = 0;
} //end of the function BotClearMessage
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_multicast(vec3_t origin, multicast_t to)
{
	int ent, muzzleflash, id;

	//read the message id
	id = ReadByte(&networkmessage);
	//if it is a muzzle flash
	if (id == svc_muzzleflash)
	{
		//read entity number causing the muzzleflash
		ent = ReadShort(&networkmessage);
		//read muzzleflash number
		muzzleflash = ReadByte(&networkmessage);
		//check for valid muzzleflash
		if (muzzleflash >= 0 && muzzleflash < MAX_MUZZLEFLASHES)
		{
			//if the muzzle flash isn't silenced
			if (!(muzzleflash & MZ_SILENCED))
			{
				//if there is a sound for this muzzle flash
				if (muzzleflashsoundindex[muzzleflash])
				{
					BotLib_BotAddSound(DF_NUMBERENT(ent), CHAN_AUTO,
						muzzleflashsoundindex[muzzleflash], 1.0, ATTN_NORM, 0);
					//gi.dprintf("multicast: mf %d: sound %d, %s\n", muzzleflash, muzzleflashsoundindex[muzzleflash], soundindexes[muzzleflashsoundindex[muzzleflash]]);
				} //end if
			} //end if
		} //end if
	} //end if
	BotWriteMessage(&networkmessage);
	newgameimport.multicast(origin, to);
	BotClearMessage();
} //end of the function Bot_multicast
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_unicast(edict_t *ent, qboolean reliable)
{
	if (ent->flags & FL_BOT)
	{
#ifdef BOT_DEBUG
		char *ptr;
		ptr = NULL;
		*ptr = 0;
#endif //BOT_DEBUG
		gi.dprintf("WARNING: tried to use unicast for a bot");
		BotClearMessage();
		return;
	} //end else
	BotWriteMessage(&networkmessage);
	newgameimport.unicast(ent, reliable);
	BotClearMessage();
} //end of the function Bot_unicast
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_WriteChar(int c)
{
	if (c < -128 || c > 127)
	{
		newgameimport.dprintf("WARNING: Bot_WriteChar: range error");
	} //end if
	networkmessage.message[networkmessage.writebyte] = c;
	networkmessage.writebyte++;
} //end of the function Bot_WriteChar
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_WriteByte(int c)
{
	if (c < 0 || c > 255)
	{
		//NOTE: in target_laser_think: gi.WriteByte (self->s.skinnum); the
		// skin number is a LONG value this causes a write byte out of range
		// error
		//newgameimport.dprintf("WARNING: Bot_WriteByte: range error");
		c = 0;
	} //end if
	networkmessage.message[networkmessage.writebyte] = c;
	networkmessage.writebyte++;
} //end of the function Bot_WriteByte
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_WriteShort(int c)
{
	if (c < (short)0x8000 || c > (short)0x7fff)
	{
		//NOTE: I guess somewhere in the original id code is some sort of bug
		// that causes a WriteShort out of range error
		//newgameimport.dprintf("WARNING: Bot_WriteShort: range error");
		c = 0;
	} //end if
	networkmessage.message[networkmessage.writebyte] = c & 0xff;
	networkmessage.writebyte++;
	networkmessage.message[networkmessage.writebyte] = c >> 8;
	networkmessage.writebyte++;
} //end of the function Bot_WriteShort
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_WriteLong(int c)
{
	networkmessage.message[networkmessage.writebyte] = c&0xff;
	networkmessage.writebyte++;
	networkmessage.message[networkmessage.writebyte] = (c>>8)&0xff;
	networkmessage.writebyte++;
	networkmessage.message[networkmessage.writebyte] = (c>>16)&0xff;
	networkmessage.writebyte++;
	networkmessage.message[networkmessage.writebyte] = c>>24;
	networkmessage.writebyte++;
} //end of the function Bot_WriteLong
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_WriteFloat(float f)
{
	int c;

	c = LittleLong(*(int *) &f);

	memcpy(&networkmessage.message[networkmessage.writebyte], &c, 4);
	networkmessage.writebyte += 4;
} //end of the function Bot_WriteFloat
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_WriteString(char *s)
{
	if (!s)
	{
		strcpy(&networkmessage.message[networkmessage.writebyte], "");
		networkmessage.writebyte++;
	} //end if
	else
	{
		strcpy(&networkmessage.message[networkmessage.writebyte], s);
		networkmessage.writebyte += strlen(s) + 1;
	} //end else
} //end of the function Bot_WriteString
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_WritePosition(vec3_t pos)
{
	Bot_WriteShort((int)(pos[0] * 8));
	Bot_WriteShort((int)(pos[1] * 8));
	Bot_WriteShort((int)(pos[2] * 8));
} //end of the function Bot_WritePosition
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_WriteAngle(float f)
{
	Bot_WriteByte(((int)f*256/360) & 255);
} //end of the function Bot_WriteAngle
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Bot_WriteDir(vec3_t dir)
{
	int j, maxdotindex;
	float dot, maxdot;
	vec3_t v;

	if (!dir)
	{
		Bot_WriteByte(0);
		return;
	} //end if
	VectorCopy(dir, v);
	VectorNormalize(v);
	maxdot = -999999.0;
	maxdotindex = -1;
	for (j = 0; j < NUMVERTEXNORMALS; j++)
	{
		dot = DotProduct (v, avertexnormals[j]);
		if (dot > maxdot)
		{
			maxdot = dot;
			maxdotindex = j;
		} //end if
	} //end for
	Bot_WriteByte(maxdotindex);
} //end of the function Bot_WriteDir
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int Bot_argc(void)
{
	int i;

	for (i = 0; i < MAX_COMMANDARGUMENTS; i++)
	{
		if (!commandarguments[i]) break;
	} //end for
	if (i) return i;
	else return newgameimport.argc();
} //end of the function Bot_argc
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
char *Bot_argv(int n)
{
	if (commandarguments[n])
	{
		return commandarguments[n];
	} //end if
	else
	{
		return newgameimport.argv(n);
	} //end else
} //end of the function Bot_argv
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
char *Bot_args(void)
{
	if (commandarguments[1])
	{
		return commandline;
	} //end if
	else
	{
		return newgameimport.args();
	} //end else
} //end of the function Bot_args
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
/*
cvar_t *Bot_cvar(char *var_name, char *value, int flags)
{
	lastcreatedcvar = newgameimport.cvar(var_name, value, flags);
	return lastcreatedcvar;
} //end of the function Bot_cvar*/
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
#ifdef TOURNEY
void OSP_PrecacheCTFRunes(void)
{
	modelindexes[TECH1_INDEX] = TECH1_MODEL;
	modelindexes[TECH2_INDEX] = TECH2_MODEL;
	modelindexes[TECH3_INDEX] = TECH3_MODEL;
	modelindexes[TECH4_INDEX] = TECH4_MODEL;
	modelindexes[TECH5_INDEX] = TECH5_MODEL;
} //end of the function OSP_PrecacheCTFRunes
#endif //TOURNEY
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void BotRedirectGameImport(void)
{
	//copy the import structure
	memcpy(&newgameimport, &gi, sizeof(game_import_t));
	//replace some of the import functions
	gi.cprintf = Bot_cprintf;
	gi.bprintf = Bot_bprintf;
	gi.centerprintf = Bot_centerprintf;
	gi.sound = Bot_sound;

	gi.modelindex = Bot_modelindex;
	gi.soundindex = Bot_soundindex;
	gi.imageindex = Bot_imageindex;
	gi.setmodel = Bot_setmodel;

	gi.multicast = Bot_multicast;
	gi.unicast = Bot_unicast;
	gi.WriteByte = Bot_WriteByte;
	gi.WriteShort = Bot_WriteShort;
	gi.WriteChar = Bot_WriteChar;
	gi.WriteByte = Bot_WriteByte;
	gi.WriteShort = Bot_WriteShort;
	gi.WriteLong = Bot_WriteLong;
	gi.WriteFloat = Bot_WriteFloat;
	gi.WriteString = Bot_WriteString;
	gi.WritePosition = Bot_WritePosition;
	gi.WriteDir = Bot_WriteDir;
	gi.WriteAngle = Bot_WriteAngle;

	gi.argc = Bot_argc;
	gi.argv = Bot_argv;
	gi.args = Bot_args;
//	gi.cvar = Bot_cvar;
	//clear the command arguments
	BotClearCommandArguments();
#ifdef TOURNEY
	OSP_PrecacheCTFRunes();
#endif //TOURNEY
} //end of the function BotRedirectGameImport

#endif //BOT
