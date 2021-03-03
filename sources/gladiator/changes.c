//===========================================================================
//
// Name:				changes.c
// Function:		game source changes
// Programmer:		Mr Elusive (MrElusive@demigod.demon.nl)
// Last update:	1999-02-24
// Tab Size:		3
//===========================================================================


/*

v0.91

the following bot modules changed:

	bl_cmd.c
	bl_cmd.h
	bl_main.c
	bl_main.h
	bl_redirgi.c
	bl_redirgi.h
	bl_spawn.c
	bl_spawn.h

the following game modules changed:

	g_cmd.c
	g_main.c
	g_spawn.c
	p_botmenu.c
	p_view.c

new modules:

	anorms.h

1999-03-19		changed the DM, RA2 and CTF bot menus
1999-03-05		added minimum players option to g_main.c and bl_spawn.c
1999-02-19		added PrecacheCTFItems() to SP_worldspawn in g_spawn.c
1999-02-20		changed offhand CTF hook on/off commands in g_cmd.c
1999-02-22		changed BotServerCmd() in bl_cmd.c
1999-02-22		added BotServerCommand() to bl_redirgi.c
1999-02-22		changed BotServerCommand() in p_botmenu.c
1999-02-22		added rcon password to ToggleBotMenu() in p_botmenu.c
1999-02-23		added	ent->client->showscores set to false for bots in ClientEndServerFrame in p_view.c
1999-02-23		changed full game network message redirection in bl_redirgi.c to remove unicast messages for bots
1999-02-23		added anorms.h to the source for the Bot_WriteDir() function in bl_redirgi.c
1999-02-28		changed botlib.h
1999-02-28		added BotLib_BotAddPointLight
1999-02-28		changed BotLib_BotUpdateSound into BotLib_BotAddSound
1999-03-08		added "cddir" libvar to library setup


*/
