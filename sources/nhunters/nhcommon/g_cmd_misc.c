//===========================================================================
// g_cmd_misc.c
//
// Miscellaneous commands
// Originally coded by majoon.
//
// Copyright (c), 1999 The BatCave. All Rights Reserved.
//===========================================================================

#include "g_local.h"

/*
==================
Cmd_Credits_f
added by majoon
==================
*/
void Cmd_Credits_f (edict_t *ent)
{
	gi.centerprintf (ent, "Credits:\n=======\nCreator/sound/coder/manager:\nmajoon\n\nPlaytesters:\nGunar, StompfesT\n\nMaps:\nMr. Hankey\n");
}

/*
==================
Cmd_Gotomap_f
added by majoon
this is until we get
some map switching in...
==================
*/
void Cmd_Gotomap_f (edict_t *ent)
{
	char	*name;
	char	*level;

	name = gi.args();

	if (Q_stricmp(name, "q2dm1") == 0)
		level = "q2dm1";
	else if (Q_stricmp(name, "q2dm2") == 0)
		level = "q2dm2";
	else if (Q_stricmp(name, "q2dm3") == 0)
		level = "q2dm3";
	else if (Q_stricmp(name, "q2dm4") == 0)
		level = "q2dm4";
	else if (Q_stricmp(name, "q2dm5") == 0)
		level = "q2dm5";
	else if (Q_stricmp(name, "q2dm6") == 0)
		level = "q2dm6";
	else if (Q_stricmp(name, "q2dm7") == 0)
		level = "q2dm7";
	else if (Q_stricmp(name, "q2dm8") == 0)
		level = "q2dm8";
	else
	{
		gi.cprintf(ent, PRINT_HIGH, "Error, bad map name.\n");
		return;
	}

	gi.configstring(CS_LIGHTS+0, "abcdefghijklmlkjihgfedcb");

	gi.bprintf (PRINT_HIGH, "Changing maps...\n");
	
	// go to a specific map
	ent = G_Spawn ();
	ent->classname = "target_changelevel";
	ent->map = level;

	BeginIntermission (ent);
}

/*
==================
Cmd_Predator_f
added by majoon
==================

void Cmd_Predator_f (edict_t *ent)
{
	if((level.time - ent->client->respawn_time) < 5)
		return;
	if (ent->isPredator)
    {
		gi.cprintf (ent, PRINT_HIGH, "Predator mode off.\n");
        ent->isPredator = false;
	   if ( ent->flashlight ) {
	    G_FreeEdict(ent->flashlight);
		ent->flashlight = NULL;
//		player_respawn (ent);
		return;
	   }
	   else
//		player_respawn (ent);
		return;
    }
    else
    {
        gi.cprintf (ent, PRINT_HIGH, "Predator mode on.\n");
        ent->isPredator = true;
	   if ( ent->flashlight ) {
	    G_FreeEdict(ent->flashlight);
		ent->flashlight = NULL;
//		player_respawn (ent);
//		InitPredator (ent->client);
		return;
	   }
	   else
//		player_respawn (ent);
//		InitPredator (ent->client);
		return;
    }
}*/
