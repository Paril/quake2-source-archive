

Title:         Gladiator Bot for Quake2 game source code
Filename:      Gladq2096gamesrc.zip
Version:       version 0.96
Date:          1999-08-02
Author:        Mr. Elusive
Email:         MrElusive@botepidemic.com
Bot Homepage:  http://www.botepidemic.com/gladiator


Description
-----------

This archive includes the game source code used for the Gladiator Bot.
This source code incorporates the following parts:

1. The Quake2 v3.20 game source code by id Software.
   ftp.idsoftware.com/idstuff/quake2/source/
2. The additional Quake2 v3.20 mission pack 1 "The Reckoning" game
   source code by Xatrix.
   ftp.idsoftware.com/idstuff/quake2/source/
3. The additional Quake2 v3.20 mission pack 2 "Ground Zero" game
   source code by Rogue.
   ftp.idsoftware.com/idstuff/quake2/source/
4. The Quake2 CTF game source code.
   ftp.idsoftware.com/idstuff/quake2/ctf/source/
5. Parts of the Rocket Arena 2 Bot Support Routines by David Wright
   http://www.planetquake.com/arena
6. The Gladiator Bot game source code by MrElusive.

The source code of the Gladiator Bot DLL is NOT included!


Source code notes
-----------------

The source file g_local.h includes several defines at the top. These defines
have been used in combination with conditional compilation to mark the
different parts of the source code added to the Quake2 v3.20 game source code.
The following defines have been used:

#define BOT				//Gladiator Bot
#define BOT_IMPORT		//bot import redirection
#define BOT_DEBUG			//bot debug
#define OBSERVER			//observer mode
#define TRIGGER_COUNTING	//trigger counting
#define TRIGGER_LOG		//trigger log
#define FUNC_BUTTON_ROTATING	//rotating button
#define LOGFILE			//log file
#define VWEP			//VWep patch
#define ZOID			//CTF
#define CTF_HOOK			//CTF offhand hook
#define ROCKETARENA		//Rocket Arena 2
#define CH				//
#define XATRIX			//Xatrix mission pack 1
#define ROGUE			//Rogue mission pack 2


LCC and Linux-i386 Makefiles are included.

Trouble shooting
----------------

compiling with Microsoft Visual C++ 5.0/6.0

Problem:   In Quake2 the following error message appears:
	     ****************
           ERROR: failed to load game DLL
	     ****************
Solution:  make sure you included the game.def in your project.

Problem:   Quake2 crashes when loading the first bot.
Solution1: do NOT use the compile option "/Gz", this option changes the calling convention
           The GetBotAPI function from the gladiator.dll should be called with the right
           calling convention.
Solution2: do NOT use the compile option "/O2", with this option the fastest code is created
           in the majority of cases (it's a default setting for release builds)for some dark
           and mysterious reason this can cause problems

Problem:   Quake2 crashes with the following message when loading the first bot:
           Debug Error!
           The value of ESP was not properly saved across a function call. This is usually
           a result of calling a function declared with one calling convention with a
           function pointer declared with a different calling convention.
Solution:  do NOT use the compile option "/Gz", this option changes the calling convention
           The GetBotAPI function from the gladiator.dll should be called with the right
           calling convention.

Problem:   compiling the source the compiler generates up to 18 "unresolved external symbol" errors.
Solution:  open the 'm_move2_rogue.c' module, save it and build again.


Copyright and distribution permissions 
--------------------------------------

By using this source code you agree to exempt, without reservation,
the authors and owners of this production or components thereof
from any responsibility for liability, damage caused, or loss,
directly or indirectly, by this software, including but not limited
to, any interruptions of service, loss of business, or any other
consequential damages resulting from the use of or operation of
this source code or components thereof. No warranties are made, expressed
or implied, regarding the usage, functionality, or implied operability
of this source code. All elements are available solely on an "as-is"
basis. Usage is subject to the user's own risk.

id Software, Xatrix, Rogue and David Wright do not distribute,
nor support this source code.

The Gladiator Bot game source code may NOT be sold or used in
commercial products in ANY form.

Please read the included liscense.txt for the copyright information
regarding those files belonging to Id Software, Inc.

Contact David Wright for the copyright information regarding the
Rocket Arena 2 Bot Support Routines.

Should you decide to release a modified version of the
Gladiator Bot game source code or parts of it, in source
code or compiled form, you MUST add the following text to the
documentation included with your product.

    This product incorporates source code from the Gladiator bot.
    The Gladiator bot is available at the Gladiator Bot page
    http://www.botepidemic.com/gladiator.

    This program is in NO way supported by MrElusive.

    This program may NOT be sold in ANY form whatsoever. If you have paid
    for this product, you should contact MrElusive immidiately via the
    Gladiator bot page or at MrElusive@botepidemic.com



Version Changes
---------------

v0.96

- added some error codes
- new botlib.h
- changed several strncpy calls in the bl_* files (thanks to Riv)


v0.95

- added rocketjump libvar to BotInitLibrary in bl_main.c
- fixed the CTF bot model bug in ClientUserInfoChanged in p_client.c
- added tourney code to bl_botcfg.c, bl_main.c, bl_redirgi.c, bl_redirgi.h and bl_spawn.c


v0.94

- fixed ztn2dm2 "plat" when squished it flies up into the sky in g_func.c


v0.93

- moved usehook and runes lib variables to bl_main.c
- fixed entering autocam when dead screws view angles in p_observer.c
- added ra_playercycle and ra_botcycle menu options to RA2 menu in p_botmenu.c
- added "addrandom [x]" option.. to add multiple random bots at once
- added return value to AddRandomBot in bl_botcfg.c and bl_botcfg.h
- added GPS code to bl_cmd.c
- added checkpoint macro command to bl_cmd.c
- added message to rocket arena commands when used when ra disabled to g_arena.c
- removed ra->value from rocket arena commands in g_cmd.c
- adjust ping in rankings screen with the lag feature in p_lag.c, p_lag.h and p_hud.c
- fixed id bug in ClientDisconnect in p_client.c
- added credits menu to the bot menu in p_botmenu.c
- fixed menu problem (when entering chasecam) in p_menu.c
- fixed CTF userinfo bug in p_client.c
- added ra_botcycle console variable to g_main.c, g_save.c g_arena.h and g_arena.c
- added ra_playercycle console variable to g_main.c, g_save.c g_arena.h and g_arena.c
- added ra_winner and ra_time to gclient_t in g_local.h
- added player cycling to g_arena.c

v0.92

- botlib.h changed (added "laserhook" description)
- added new menu title pictures to p_botmenu.c
- added QuakeCopyMenu to p_menulib.c and p_menulib.h
- removed potential crash bug from p_menulib.c and also change p_menulib.h
- moved bot cfg loading to seperate file: p_botcfg.c and added p_botcfg.h
- added loading all *.cfg files from the "bots" sub-folder in g_botmenu.c
- removed char *botlib; from g_local.h
- removed "botlib" entity field from g_save.c
- removed botlib from bots.cfg files in bl_spawn.c and g_botmenu.c
- fixed Rocket Arena precaching of items in RA2_Init in g_arena.c
- fixed the "minimumplayers" removebot bug in bl_spawn.c
- added CTF "botctfteam" option to the menu (changed p_botmenu.c, g_ctf.c, g_ctf.h, g_main.c, g_save.c)
- in RA2 bots will never be in a waiting room, they'll always be noclip observers, g_arena.c
- added altnames cvar to BotInitLibrary in bl_main.c
- added gender command to BotCmd in bl_cmd.c for bots to set gender
- added score resetting to ExitLevel in g_main.c
- added p_lag.h to p_client.c and added lag stuff to ClientThink
- added p_lag.h to g_cmd.c and added lag command to ClientCommand
- added p_lag.c and p_lag.h
- fixed teamplay in bot menu in p_botmenu.c
- fixed setting dmflags in bot menu in p_botmenu.c
- disabled the "WARNING: Bot_WriteShort: range error" in bl_redirgi.c
- changed the extra ThrowGib to work only with Rogue MP1 (caused the msg overflow) in p_client.c


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

- changed the DM, RA2 and CTF bot menus
- added minimum players option to g_main.c and bl_spawn.c
- added PrecacheCTFItems() to SP_worldspawn in g_spawn.c
- changed offhand CTF hook on/off commands in g_cmd.c
- changed BotServerCmd() in bl_cmd.c
- added BotServerCommand() to bl_redirgi.c
- changed BotServerCommand() in p_botmenu.c
- added rcon password to ToggleBotMenu() in p_botmenu.c
- added ent->client->showscores set to false for bots in ClientEndServerFrame in p_view.c
- changed full game network message redirection in bl_redirgi.c to remove unicast messages for bots
- added anorms.h to the source for the Bot_WriteDir() function in bl_redirgi.c
- changed botlib.h
- added BotLib_BotAddPointLight
- changed BotLib_BotUpdateSound into BotLib_BotAddSound
- added "cddir" libvar to library setup

