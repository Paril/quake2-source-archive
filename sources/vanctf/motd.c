#include "g_local.h"
#include "fileio.h"
#include "teamplay.h"
#include "motd.h"

void MOTD_show(edict_t *ent) {
	char	string[1024];
	char	*custommsg;

	// remove help icon
	game.helpchanged = false;

	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
	ent->client->showmotd = true;

	custommsg = ReadTextFile(va("%s/motd.txt", gamecvar->string));

	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn help "			// background
//		"xv 202 yv 12 string2 \"%s\" "		// skill
		"xv 0 yv 24 cstring2 \"%s (%s)\" "	// level name
		"xv 0 yv 54 cstring2 \"%s\" "		// help 1
		"xv 0 yv 108 cstring2 \"%s\" "		// help 2
		"xv 50 yv 164 string2 \"           Team:\" "
		"xv 50 yv 172 string2 \"           %s\" ",
//		"xv 50 yv 164 string2 \" kills     goals    secrets\" "
//		"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ", 
//		sk,
		level.level_name, level.mapname,
		"Welcome!  Running Vanilla\nCTF v0.9.  Press F1 to\nclear this message.  Use\n'ctfhelp' for help.",
		custommsg ? custommsg: "",		// server specific message
//		level.killed_monsters, level.total_monsters, 
//		level.found_goals, level.total_goals,
//		level.found_secrets, level.total_secrets
		IS_TEAM1(ent) ? " RED" : IS_TEAM2(ent) ? "BLUE" : "NONE"
		);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
	
	if (custommsg) gi.TagFree(custommsg);	// no memory leak here
}

