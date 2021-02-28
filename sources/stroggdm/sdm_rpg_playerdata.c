/*
====================
StroggDM Functions
====================
--------
Most stuff used here is copyrighted to ID Software
under the GPL license.
--------

Thanks [QBS]Quadrant for this wonderful peice of code.
Your code has inspired me in many ways.
*/

//[QBS]===========================================
// This peice of code is designed to be used for
// player Persistance & global stats 
// Coded By [QBS]Quadrant
//================================================ 

//load_save.c
#include "g_local.h" // talks to q2 engine
#include "sdm_rpg_playerdata.h"

static const char * PSUBDIR = "playerfiles";// subdir in your mod folder to save to 
int file_version = 1; // First version!

/* this version always zero-terminates, unlike the silly standard one */
// mysnprintf is coded by Blinky
int mysnprintf(char * buffer, size_t count, const char * fmt, ...)
{
	va_list argptr;
	int ret;

	va_start(argptr, fmt);
	ret = vsnprintf(buffer, count, fmt, argptr);
	va_end(argptr);
	buffer[count-1]=0; /* Always put zero at end */
	return ret;
} 

//ok the below peices of code in this comment are to define the 
//gamepath which is very useful,, 

#ifndef __WIN32
#include <sys/stat.h>
#else
#define mkdir _mkdir
#endif

player_save_t DAT;

void SetFilename(char * filename,int len, edict_t *ent)
{
	char mod[16];
	mkdir ("stroggdm/playerfiles", S_IWUSR | S_IRUSR);//[QBS] make the dir if we dont have it 
	
	strcpy(mod, "stroggdm");
	mysnprintf(filename, len, "%s/%s/%s.sdm", mod, PSUBDIR, ent->client->pers.netname); 
} 

//======================================================================================= 

void WRITE_PLAYER_STATS (edict_t *ent)// SAVE PLAYER
{
	edict_t *attacker;
	edict_t *self;
	char filename[128];
	FILE *FH;

	if (!((int)stroggflags->value & SF_RPG_MODE))
		return;

	attacker = ent;
	self = ent;
	ent = ent; //[QBS]what the hell lol :)

	ent->client->asking_for_pass = 0; // Just incase!
	
	SetFilename(filename, sizeof(filename), ent);
	// MISC
	/*
	ok what things should we have on player persistance
	example 
	
	  Deaths
	  Kills
	  Time played
	  Kill Percentage Worked out from kills to deaths
	  Player Password (to protect players character maybe)
	*/ 
	
	safe_cprintf (ent, PRINT_HIGH, "Saving player...\n");
	
	DAT.version = file_version;

	safe_cprintf (ent, PRINT_HIGH, "File version: %i...\n", file_version);
	
	//[QBS] Player tracking anything you like can go here 
	
	DAT.exp = ent->client->resp.exp;
	DAT.lvl = ent->client->resp.lvl;
//	DAT.class = ent->client->resp.class;
	DAT.score = ent->client->resp.score;
	DAT.health = ent->health; 
	strcpy (DAT.password, ent->client->resp.password);
	
	safe_cprintf (ent, PRINT_HIGH, "Saved stats...\n");

	//[QBS]end
	
	/* 
	If you wanted you could have this post to a global system at this point to have global data for all servers running your mod
	*/

	if ((FH = fopen( filename, "wb")) == NULL)
	{
		gi.cprintf(ent, PRINT_HIGH, "File is in use or the file is corrupted somehow.\n");
		return;
	}

	fwrite ( &DAT, sizeof(DAT), 1, FH );
	fclose ( FH );

	safe_cprintf (ent, PRINT_HIGH, "Done!\n");
} 

//====================================================================================== 

#define LOADING_SUCCESS 0
#define LOADING_ERROR 1
#define LOADING_NEW 2

int READ_PLAYER_STATS (edict_t *ent)// LOAD 
{
	char filename[128];
	FILE *FH;
	//[QBS]attempt to make global
	edict_t *attacker;
	edict_t *self;
	
	if (!((int)stroggflags->value & SF_RPG_MODE))
		return 0;

	attacker = ent;
	self = ent;
	ent = ent; //what the hell lol :)
	//[QBS]end 
	
	SetFilename(filename, sizeof(filename), ent); 
	
	if ((FH = fopen( filename, "rb")) == NULL)
	{
		gi.cprintf(ent, PRINT_HIGH, "Player not found, probably because you're a new player.\nUse the Create new Character button.\n");
		return LOADING_NEW;
	}

	fread ( &DAT, sizeof(DAT), 1, FH );
	fclose ( FH );

	gi.cprintf (ent, PRINT_HIGH, "Started loading player file...\n");
	gi.cprintf (ent, PRINT_HIGH, "File version: %i\n", DAT.version);
	if (DAT.version != file_version)
	{
		// Paril: FIXME
		// In later versions, find out which version to load and load data
		// that was saved from older versions, leaving new ones to the dust.
		// This might be a problem with certain things though.
		gi.cprintf(ent, PRINT_HIGH, "Incorrect File Version, converting...\n");
	}

	// Okay, it hasn't gotten far yet.
	// Now that we have this, it's going to open the
	// messagemode box to type in password.
	// It's checked at Step2.

	// I'm sorry to have to do this to you, fair Quake2.
	ent->client->asking_for_pass = 1;
	stuffcmd (ent, "messagemode\n", -1);

	return LOADING_SUCCESS;
} 


qboolean PlayerExist (edict_t *ent)
{
	char filename[128];
	FILE *FH;
	//[QBS]attempt to make global
	edict_t *attacker;
	edict_t *self;
	
	if (!((int)stroggflags->value & SF_RPG_MODE))
		return true;

	attacker = ent;
	self = ent;
	ent = ent; //what the hell lol :)
	//[QBS]end 
	
	SetFilename(filename, sizeof(filename), ent); 
	
	if ((FH = fopen( filename, "rb")) == NULL)
	{
		gi.cprintf(ent, PRINT_HIGH, "Player not found (good thing). Enter your password:\n");
		return false;
	}

	// We got past that point, so this means...
	// IT EXISTS! HACKING ATTEMPT, RUN!

	gi.cprintf(ent, PRINT_HIGH, "A character with the name you have already exists.\nPlease change your name and try again.\n");
	return true;

	return LOADING_SUCCESS;
}

void Step2 (edict_t *ent)
{
	if (ent->client->asking_for_pass == 2)
	{
		Step3 (ent, NULL); // Creating new char.
		ent->client->asking_for_pass = 0;
		return;
	}
	ent->client->asking_for_pass = 0;
	// Oops, incorrect password usage.
	gi.cprintf (ent, PRINT_HIGH, "Checking entered password...\n");
	if (Q_stricmp(DAT.password, ent->client->entered_password))
	{
		gi.centerprintf (ent, "Incorrect Password: %s\n",ent->client->entered_password);
		return;
	}		
		
	ent->client->resp.exp = DAT.exp;
	ent->client->resp.lvl = DAT.lvl ;
//	ent->client->resp.class = DAT.class ;
	ent->client->resp.score = DAT.score ;
	ent->health = DAT.health;
	strcpy (ent->client->resp.password, DAT.password);

	gi.cprintf (ent, PRINT_HIGH, "Loaded stats...\n");
	
	gi.cprintf(ent, PRINT_HIGH, "Player was successfully loaded!\n");
	ent->client->resp.loaded = 1;

	ent->need_to_set = 1;

	OpenMainMenu (ent, NULL);
}

void PlayerData_Login (edict_t *ent, pmenuhnd_t *p)
{
	// Paril - Player logged in. Make sure that he enters password, so we do it a special way.
	// We're going to call messagemode, but replace Cmd_Say_i with a function that will
	// enter his password instead of saying something.
	// Clever?

	// We start by the main loading sequence.
	READ_PLAYER_STATS(ent);
	// It handles the rest.
}

void PlayerData_Create (edict_t *ent, pmenuhnd_t *p)
{
	// Paril - Player creates new character. Very simple, first we'll open the password box,
	// then going to do a write char, then open the class menu.

	// Revision: Creating an account under an already-existing name; defunct. Need to see if he exists first!
	if (PlayerExist (ent))
		return;
	
	// I'm sorry to have to do this to you, fair Quake2.
	ent->client->asking_for_pass = 2;
	stuffcmd (ent, "messagemode\n", -1);
}

void Step3 (edict_t *ent, pmenuhnd_t *p)
{
	// Paril - Player creates new character. Very simple, first we'll open the password box,
	// then going to do a write char, then open the class menu.
	
	// Okay, we entered our requested password. Let's do this.
	strcpy (ent->client->resp.password, ent->client->entered_password);

	WRITE_PLAYER_STATS(ent);
	// We finished. Time to open the class menu!
	OpenMainMenu (ent, NULL);
}
