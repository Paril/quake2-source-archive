    ---============================---

       Paintball 2 (Alpha build 43)

             by Digital Paint

       http://www.digitalpaint.org/

    ---============================---


--------------------------------------------

ABOUT:

Digital Paint Paintball 2 is a fast-paced, team-based first person shooter with a paintball theme.  It's designed to be fun, not necessarily realistic.  At the moment, the game play is purely multiplayer only, so you will need a good Internet connection or a LAN with some friends to play.  For more information, read the documentation at http://digitalpaint.org/docs.html


WARNING ABOUT PERFORMANCE (FRAMERATE) ISSUES:

If you get poor framerates around water, you may want to disable reflective water (r_reflectivewater 0) and caustics (r_caustics 0).  If you still have poor performance in general or on specific maps, you may need to set gl_dynamic 0.


BOTS:

Since I've received so many inquiries about bots, I'm adding them to the readme.  Keep in mind that bot support is nowhere near complete.

Bot command examples:
addbot (name, optional)
removebot (name, can be abbreviated: ex, removebot 2)
removeallbots


KNOWN BUGS:

- Names with spaces don't work from the menu (use quotation marks to get around this).
- Edit fields don't handle text formatting (colors and such) correctly.
- More bugs/feature requests on the feature vote system: http://dplogin.com/dplogin/featurevote/

REPORTING BUGS:

PLEASE tell us if something does not work properly.
We can't fix it if we don't know it's broken.
Forum: http://dplogin.com/forums
Email: jitspoeAyahooDcom
IRC: irc.globalgamers.net, #paintball

Please provide information such as what video card you are using 
(if it's a graphical problem or paintball2.exe crashes)
Be descriptive enough so we can reproduce the problem.
Provide screenshots or demos if applicable.


WHAT'S NEW:

build 43 (2019-08-20):
- Bugfix: Fixed crash when viewing the server list while a level is loaded locally.
- Bugfix: Fixed sound falloff on left and right channels being slightly different.  Also tweaked to be slightly more physically accurate. 7:21 AM 6/24/2019
- Bugfix: Fixed serverlist connecting to the worng IP's when using the listview widgets (Thanks, xrichardx)
- Bugfix: Fixed crash when using SDL sound driver on Linux. (Thanks, your teammate) 10:10 PM 7/7/2019
- Tweak: Reduced buffer size for Linux SDL driver from 4096 to 1024 to reduce latency. 5:53 AM 8/18/2019
- Tweak: Reduced "s_mixahead" default cvar value from 0.2 to 0.08.  You may need to increase this if you get stuttery sound. 5:53 AM 8/18/2019
- Cvar: "vid_resolution" now replaces "gl_mode", "vid_resx", and "vid_resy".  You can simply set "vid_resolution" to "1920x1080" or whatever you would like. (Thanks, xrichardx).  Should automatically set to whatever you had before using the previous cvars.


build 42 (2019-06-08):
- Feature: Native IRC support to join the #paintball channel on GlobalGamers. "irc_connect", "irc_say", "irc_quit" commands.
- Feature: Listview support for UI (Thanks, xrichardx)
- Cvar: "cl_scoreboard_sorting" - 1: sort by kills only, 2: sort algorithm with priority of grabs and caps, 3: sort with priority of grabs and caps
- Bugfix: "GL_Upload8: too large" error from corrupted texture files no longer stops the level from loading.


build 41:
- Feature: New bots (WIP).
- Command: "demo_playernumber" allows you to specify which player to chase when viewing demos (very rough right now). 12:39 AM 2/5/2015, 1:40 AM 4/4/2015
- Bugfix: Fixed a crash loading maps with over 32k surfaces. 2:56 AM 11/1/2013
- Bugfix: Fixed jpeg loading code so it supports EXIF as well as JFIF formats. 2:05 AM 12/12/2013
- Bugfix: Fixed scoreboard not displaying on first key press after being closed with the console or ESC. 12:15 AM 1/7/2014
- Bugfix: Fixed issue with dalogs showing up as an error about not being able to find menus/dialog.txt. 12:21 AM 8/9/2014
- Bugfix: Made rcon command behave more like expected with multiple parameters and fixed potential exploits. 2:23 AM 1/10/2015
- Bugfix: Fixed possible security hole. 10:42 PM 3/27/2015
- Bugfix: Fixed for failed downloads due to backslashes. (Thanks, xrichardx).
- Bugfix: Fixed possible crash in custom menu files. (Thanks, xrichardx).
- Tweak: Tweaked min and max rate values to be higher. 12:48 AM 1/21/2014
- Tweak: Added 2560x1440 resolution support. 10:47 PM 5/18/2014
- Tweak: Added 50% black background to dialogs to make them easier to read/more obvious that you couldn't click buttons behind them. 12:24 AM 8/9/2014
- Tweak: Defaulted m_rawinput to 1, so mice will use raw input by default (on Windows). 2:24 AM 1/10/2015
- Tweak: Projects converted to and now compiled with Visual Studio 2010. 1:39 PM 1/19/2015
- Cvar: "gl_colorpolycount" - color codes the different polycounts of each brush face (red, green, blue, yellow, purple, grey, for 3, 4, 5, 6, 7, 8+ sides).  Only works offline ("deathmatch" must be 0).
- Cvar: "vid_borderless" - Allows for borderless windowed mode when set to 1. 10:54 PM 2/26/2014
- Cvar: "vid_resx" and "vid_resy" can set custom resolutions if "gl_mode" is set to "-1" (Thanks, xrichardx)
- Cvar: "timestamp_date" - if set to 1, logs will include the full date on each line. (Thanks, xrichardx)
- Media: Higher resolution textures are now included by default.


build 40 (2013-10-08):
- Bugfix: Fixed crash with dedicated servers. 10/7/2013
- Bugfix: Fixed integer overflow in sound code, caused by playing on a server that had been running the same map for days. 11:13 PM 10/7/2013
- Bugfix: Fixed "Can't load pics/***r_notexture***" message. 10/7/2013
- Bugfix: Fixed an issue where world would stop rendering after a vid_restart on some systems. 10:05 PM 10/8/2013
- Cvar: "m_rawinput" - If set to 1, this will use raw mouse input (to make sure any windows filtering, acceleration, etc. don't get applied and you have the most precise mouse movement in-game). 11:12 PM 10/7/2013
- Tweak: Changed default mouse sensitivity to 5. 11:37 PM 10/7/2013
- Tweak: Fixed fps counter alignment at 1000fps.
- Tweak: Changed cl_drawping display to display "ping" instead of "Png"


build 39 (2013-10-06):
- Feature: Adding "missingpic" keyword to menu system so mapshots could display a default "NO PREVIEW" image instead of a grey box.
- Tweak: Improved model rendering performance a bit by using SIMD instructions. 1:02 PM 5/29/2013
- Tweak: Allowed crosshair scale to go below 1.
- Tweak: Improved entity point lighting using LordHavoc's code from DarkPlaces (use r_oldlightpoint cvar to toggle old system). 1:34 AM 6/5/2013
- Tweak: Made it so duplicate lines don't get stored in the console history.
- Tweak: Made the window position reset if the window is not visible on any monitor. 1:24 AM 9/12/2013
- Tweak: Disabled sound on alt-tab by default, since sometimes sounds were getting stuck on (seta s_disableonalttab 0 to reenable). 1:54 AM 10/6/2013
- Cvar: "r_hardware_light" - Enables OpenGL lighting on SKM models for better performance (on most hardware).  Enabled by default.
- Cvar: "cl_conback" - specifies the console background texture. (Thanks, T3RR0R15T)
- Cvar: "cl_menuback" - specifies the menu background texture. (Thanks, T3RR0R15T)
- Cvar: "cl_maptime", "cl_maptimex", "cl_maptimey" - Displays elapsed map time on the HUD at the x and y position specified. (Thanks, T3RR0R15T/AprQ2)
- Cvar: "cl_drawping" - displays ping on the HUD. (Thanks, T3RR0R15T)
- Cvar: "sv_blockednames" - Comma-separated list of names server admins can choose to block from connecting to the server. (Thanks, T3RR0R15T)
- Cvar: "cl_sse" - Enables SIMD rendering optimizations (enabled by default).
- Bugfix: Fixed issue with loading demos and maps (trying to open "Loading....bsp", etc.)
- Bugfix: Fixed occasional black splat models (hopefully). 1:24 AM 6/11/2013
- Bugfix: Fixed transparent surfaces on moving brushmodels (ex: doors) not moving.  Note that the draw order is still not fixed, so avoid putting them near other transparent brushes, but at least they will move.  2:12 AM 6/16/2013
- Bugfix: Made reflective water effect respect scale and rotation on maps (so flowing water flows the way the mapper specified). 12:01 AM 9/12/2013


build 38 (2013-05-08):
- Feature: Loading screen. (Thanks to ViciouZ for starting this).
- Bugfix: Fix for mouse not working when fov cvar is set to an invalid value. 7:31 PM 12/15/2012
- Bugfix: Fixed rare crash with footstep sounds. 12:47 AM 1/9/2013
- Tweak: Cleaned up some code to avoid switching between char * and unsigned char *. 7:00 PM 11/28/2012
- Cvar: "s_preload" - Enabled by default, forces precached sounds to load from disk.  Hopefully this will fix the hitches some people have when sounds like flag grabs are played. 12:07 AM 2/26/2013


build 37 (2012-11-07):
- Feature: Server list sorting by type (name/ping/map/players). (Thanks, ViciouZ)
- Feature: Console input history is saved. 2:11 AM 10/3/2012
- Feature: Added buttons to menu system.
- Feature: Added "include" functionality to menu system. 2:43 PM 10/30/2012
- Feature: Text in the menus now goes through the translation system (to make supporting other languages easier). 11:26 PM 11/1/2012
- Bugfix: Fixed ability to translate dialog strings. 11:01 PM 9/26/2012
- Bugfix: Fixed bug where if you switched to full screen in a lower resolution and the window was too far to the side, the mouse would constantly register as moving. 2:28 PM 10/6/2012
- Bugfix: Fixed an issue with "getmaplist" command that was sometimes causing not all of the list to populate on the voting screen. 9:58 PM 10/29/2012
- Bugfix: Fixed a bug in my hash algorithm where it was allocating 1 less element than it should. 11:10 PM 11/1/2012
- Bugfix: Fixed a bug where if you clicked on a menu item, then moved the mouse off of it, the item would be stuck selected. 10:32 PM 11/5/2012
- Tweak: Stripped leading spaces off of server names in the server browser. 11:23 PM 10/2/2012
- Tweak: "map" command checks for the map in the "beta" and "inprogress" directories as well as the root maps directory. 9:28 PM 10/3/2012
- Tweak: Switched mouse turn variables to floating point for better turning precision. 9:49 PM 10/5/2012
- Tweak: Made it so the mouse cursor uses the windows mouse cursor position in windowed mode at the menu, and you can resize/move the window while the menu is open. 11:06 PM 10/5/2012
- Tweak: "getmaplist" command now outputs results to the console if the console is down.  9:59 PM 10/29/2012
- Tweak: "maplist" forwards to "getmaplist" command to stop overflows. 10:01 PM 10/29/2012
- Tweak: Revamped dialog boxes to use new bpic and wordwrap system and support translations. 10:33 PM 11/5/2012
- Tweak: Only attempt to auto-login if a password is set. 10:54 PM 11/5/2012
- Tweak: Made profile login failures display more informative dialogs instead of a generic one and printing the actual error to the console. 9:10 PM 11/6/2012
- Cvar: "m_fovscale" - If enabled, scales the mouse turn speed based on your FOV.  At 90 degrees there is no difference, if you zoom in or out, you will turn slower or faster, respectively.  Set to 0 to disable.
- Cvar: "cl_drawfps" - Added a new option, 3, which displays the lowest framerate every half second.  Useful for detecting single-frame drops. 11:32 PM 10/5/2012
- Cvar: "cl_menu" - Can be used to specify a subdirectory for custom menu files. (Thanks, T3RR0R15T)
- Command: "menu_refresh" - Optimized version of "menu_reload".  Only refreshes the current menu and does not reload menus from disk.  (Useful for advanced menu scripting). 10/6/2012
- Media: Revamped a bunch of the menus to use buttons and better backgrounds / borders.
- Media: Some tweaks to the tutorial map.


build 36 (2012-09-09):
- Feature: Dialog box support. (Thanks, ViciouZ)
- Feature: Added "$bind(command)" to macro processing (used in the tutorial map).
- Bugfix: Fixed bouncing when standing on crouched players. 5:05 PM 6/9/2012
- Bugfix: Fixed crouched players disappearing when standing on top of them. 5:19 PM 6/9/2012
- Tweak: In-game server browser ping colors fade from white to orange (like the CO2 bar on the HUD) to help indicate server quality.
- Tweak: Made key bindings case-insensitive and display in upper case. 8:07 PM 9/6/2012
- Cvar: cl_dialogprint - disables dialog popups (set to 0 to print to the console). 12:57 AM 9/7/2012
- Cvar: m_noaccel - Replaces the old "m_xp" cvar and disables mouse acceleration for all versions of windows.  Windows 7 had it enabled by default (yuck). 10:03 PM 9/7/2012
- Cvar: sv_enforcetime2 - A new attempt to stop speed cheats. 0 = disabled.  Anything else = threshold, in seconds, before somebody is kicked for time discrepancy (Suggested value = 1). 12:28 PM 9/8/2012
- Media: Tutorial map.


build 35 (2012-01-09):
- Bugfix: Fixed jittering/bouncing against angled walls on slopes.
- Bugfix: Fixed jumping on ramps with low ceilings not going up high enough and giving you a weird boost.
- Bugfix: Fixed the game taking mouse focus when it didn't start up in the foreground (Win32). 12:06 AM 1/6/2012
- Tweak: Reduced accel on ramps with the new physics (should behave more like the old physics).
- Tweak: Limited the length of names on the client to 30 characters so they don't get cut off on the server and cause logins to fail. 9:20 PM 1/8/2012
- Tweak: Limited menu sounds to once per frame so there isn't a really loud sound when you first launch the game. 12:45 AM 1/9/2012
- Cvar: "s_disableonalttab" - Disabling this now works with the default DirectSound sound.  Disabled by default so you can hear sound from the game while you're alt-tabbed out. 12:02 AM 1/6/2012


build 34 (2012-01-02):
- Bugfix: Fixed menu widgets not repositioning correctly after the game window was resized. 11:02 PM 12/29/2011
- Bugfix: Fixed potential crash with server "say" command and made server chat display to the console. 3:15 AM 1/2/2012
- Bugfix: Fixed non-integer crosshair scales not positioning correctly. 9:29 PM 1/2/2012
- Tweak: Removed water uber doublejump with new movement physics. 12:03 AM 12/30/2011
- Tweak: "arecord" now prepends "auto" if no prefix is specified - (T3RR0R15T)
- Tweak: Made it so crouching in the air at high velocities should no longer be possible.  It's still possible to crouch at low velocities so crouching down slopes and steps doesn't bounce you up and down.
- Cvar: "sv_skyglide_maxvel" - Allows you to tweak the maximum velocity skygliding will retain, so you can't sky glide crazy distances from ice jumps, etc. 12:05 AM 12/30/2011
- Cvar: "sv_crouchslide" - enables crouch sliding.
- Cvar: "sv_oldmovephysics" - uses the older style ramp and step physics.
- Cvar: "cl_consoleheight" - Value from 0 to 1 to adjust console height (0.5 is the default) - (T3RR0R15T)
- Cvar: "sv_consolename" - Name used when you "say" something at the server console.  0 = "console", 1 = "Info", 2 = "News", 3 = "Help", 4 = "Server", 5 = "Admin". (T3RR0R15T)


build 33 (2011-11-13):
- Tweak: Better movement control on slopes (hold the jump key to bounce and slide on slopes like you used to).
- Tweak: Made landing sound quieter for short falls. 11:57 PM 3/28/2011
- Tweak: Allowed non-integer sizes for cl_hudscale (example: 2.5 instead of 2 or 3). 12:48 AM 3/29/2011
- Tweak: Removed some (likely) obsolete code that could potentially zero out player velocity. 11:36 PM 4/12/2011
- Tweak: Made cl_drawtexinfo 1 only hit solid objects.  Use cl_drawtexinfo 2 to test clip, etc.  8:40 PM 5/5/2011
- Tweak: Improved stair smoothing.
- Bugfix/Tweak: Fixed not stepping/abrupt stops when jumping up stairs/onto objects.  
- Bugfix: Fixed not stepping in places with a low ceiling. 11:37 PM 4/12/2011
- Bugfix: Fixed application icon (in Windows). 10:36 PM 4/27/2011
- Bugfix: Fixed corrupt screenshots at 1366x768 resolution. 8:57 PM 7/20/2011
- Bugfix: Fixed bouncy/jittery screen when on elevators.
- Bugfix: Fixed multithreading crash on Linux config.
- Cvar: "oldmovephysics" - Disable new movement physics tweaks.
- Cvar: "sv_reserved_slots" - Sets the number of slots to reserve on the server for admins/VIP's. (Thanks, T3RR0R15T)
- Cvar: "sv_reserved_password" - Sets the password necessary to use one of the reserved slots. (Thanks, T3RR0R15T)
- Feature: Added minimize, maximize and resize capability in Windows. 1:54 AM 4/28/2011


build 32 (2011-02-21):
- Feature: Server list sorts by players/ping.
- Feature: Serverlist auto-updates and clears on update.  10:53 PM 2/16/2011
- Bugfix: Fixed issue with files not downloading that were in the "requiredfiles" field in maps.
- Bugfix: Fix for crash with long console lines then hitting up.
- Bugfix: (Hopefully) fixed OpenGL errors on some Linux configs.
- Bugfix: Fix for rare case where a client reconnect or potentially a laggy connect could result in an "active" entity before the connection finished.  11:59 PM 2/16/2011
- Bugfix: Fixed serverlist not updating if a player hasn't logged in or connected to a server first. 6:31 PM 2/21/2011
- Tweak: Various fixes to console text messages. (Thanks, T3RR0R15T).
- Tweak: Merged Linux and Win32 opengl code to make things more maintainable.
- Tweak: Increased MAX_INFO_STRING to 1024 to hopefully get rid of "Info string length exceeded" messages. 1:13 PM 2/21/2011


build 31 (2010-12-20):
- Feature: Hid model when chasecamming in first person. 10:42 PM 11/2/2010 (thanks zuluzet)
- Bugfix: Fixed netgraph so it doesn't wrap around in resolutions above 1024 (will still wrap at resolutions above 2048). 11:40 PM 9/8/2010
- Bugfix: Fix for server password menu not coming up sometimes (resulted in a get_password-attack.txt error). 9:25 PM 9/16/2010
- Cvar: cl_drawclockx and cl_drawclocky - Adjust position of the clock. Default is "-1" on both commands = centered on the top of the hud (like build 30).
- Tweak: Names now default to "noname#####" (##### = random number) instead of "newbie".
- Tweak: Made things MT to help reduce game hitches.  1:04 AM 11/2/2010
- Tweak: Changed filename from demos started with arecord. It's now like the filename from the autodemo on a server.
- Tweak: Added the resolution 1600 x 900.
- Tweak: Removed unused commands ("gl_nobind", "gl_poly", "gl_texsort", "gl_polyblend", "gl_ztrick", "gl_playermip").
- Tweak: Increased number of screenshots to 9999.
- Tweak: Enabled archive flag for cl_swearfilter.
- Tweak: Added ftp:// support to the webload command.

build 30 (2010-05-07):
- Feature: Added a message box to inform people when they don't have proper hardware-accelerated OpenGL drivers installed.
- Bugfix: Fixed some errors that happen in old implementations of OpenGL.
- Bugfix: Fixed a crash in rscripts for old hardware/drivers that don't support multitexture.  1:02 AM 4/14/2010
- Tweak: Made scores menu allow some game input (fixes issues if you have "score" bound to "tab"). 5:10 PM 5/2/2010
- Tweak: Switched to Visual Studio 2008 express.
- Tweak: Tweaked default volume and attenuation so sounds at a distance are easier to hear. 5/2/2010
- Media: Re-recorded carbine sounds at a different angle so they sound less clangy.
- Media: New menu screen for people that do not have a profile to hopefully make things less confusing.
- Media: Increased volume of grass footsteps.


build 29 (2010-01-20):
- Feature/Bugfix: Multi-map demo support.  If you record a demo that spans multiple maps, the game now attempts to play all of them (may have issues with maps that are downloaded while recording). 12:14 AM 1/8/2010
- Feature: "measure" command.  Type "measure" at the console (or bind it to a key) to set a start point (where the crosshair is aiming).  Move the aim point and type it again.  It will output the distance between the points to the console (useful for mappers). 1:50 AM 1/6/2010
- Feature: First pass at certified server settings.  sv_certificated will show as "1" if servers have certified settings. (Thanks, T3RR0R15T)
- Bugfix: Fix for old scoreboard data/garbage staying on the scoreboard when switching servers.
- Cvar: s_disableonalttab - Enabled by default to keep current behavior of disabling the sound when alt-tabbing out of the game window.  Not fully functional.  Setting it to 0 only seems to work for 3D sound. 12:16 AM 1/8/2010
- Cvar: sv_locked - If enabled, prevents new players from connecting (Thanks, T3RR0R15T/R1CH)
- Tweak: File lists are sorted. 1:53 AM 1/6/2010
- Tweak: Minor adjustments to footstep sounds.
- Tweak: Added new video resolutions (1920x1080, 1920x1440, 1366x768) (Thanks, T3RR0R15T)


build 28 (2009-08-14):
- Feature: Surface specific footstep sounds.
- Cvar: s_nojump - disables jumping grunt sound and plays an extra footstep sound instead.  7:04 PM 7/19/2009
- Tweak: Switched all falling sounds to use the basic land sound (pain grunts got annoying and they didn't really fit since there is no falling damage).  Also mixed in a surface-specific foot step sound so landing sounds more natural.  7:05 PM 7/19/2009 
- Tweak: Defaulted s_resamplevolume to 0.35 so old sounds aren't louder than new sounds.


build 27 (2009-04-15):
- Tweak: Added sleep to server loop to hopefully prevent crashed servers from eating all CPU.
- Cvar: s_resamplequality - If set, sound resampling will use a linear algorithm to improve quality of non-48khz sound files.
- Cvar: s_resamplevolume - Since new sounds are quieter to avoid clipping, old non-48khz sounds will be scaled down by this value (default is 0.5, 50%).
- Cvar: s_force48khz - Enabled by default, sound settings are forced to high quality (48khz).  If you wish to use
- Media: Surface specific splat sounds.
- Media: New 68 Carbine and VM-68 recordings.
- Media: Fixed admin map changing menu.


build 26 (2009-02-25):
- Feature: Added "selected" flag to menu system, so a widget can be selected by default when a menu is loaded (for things like password prompts).
- Feature: PNG texture support (Thanks: ViciouZ, R1CH)
- Bugfix: Fixed bug where first letter typed was lost in an edit box if the cursor was beyond the length of the string. 11:09 PM 2/3/2009
- Cvar: cl_crosshairscale - Allows you to change the crosshair size independently of the HUD scale. (Thanks: ViciouZ)
- Cvar: cl_passwordpopup - If set (enabled by default), a password prompt will open when attempting to connect to a passworded server. (Thanks: ViciouZ)
- Media: New splash sounds.
- Media: New footstep sounds for snow actually included this time!


build 25 (2009-01-28):
- Cvar: "cl_swearfilter" - if set to 1, chat text containing blocked words will not display.  If set to 2, blocked words will be ***'d out. (Thanks: ViciouZ)
- Cvar: "cl_blockedwords" - comma separated list of words to filter if cl_swearfilter is set. (Thanks: ViciouZ)
- Feature: Started on surface specific footstep sounds.  Snow textures and w_metal1 will make a new stepping sound.  6:39 PM 1/20/2009
- Feature: "localstats" will print cumulative kill/death/grab/cap statistics. (Thanks: ViciouZ)
- Feature: OGG Vorbis music support.  Put some .ogg files in paintball2/pball/music to test it out. (Thanks: QuDos, ViciouZ)
- Feature: "menu_reload" command for advanced menu features (Thanks: Zorchenhimer)
- Feature: Password dialog for connecting to passworded servers. (Thanks: Zorchenhimer, ViciouZ)
- Feature: Password support for paintball2:// hyperlinks: paintball2://xxx.xxx.xxx.xxx:xxxxx?serverpass (Thanks: ViciouZ)
- Tweak: Made the same footstep sound not play twice (or more) in a row, as that sounds weird. 1:37 AM 11/8/2008
- Media: More new recordings (several guns, smoke grenade, snow footsteps)


build 24 (2008-10-14):
- Cvar: cl_drawclock 1/0 enables/disables display of the current time at the top of the screen. (Thanks: ViciouZ).
- Feature: Time and server are logged in demo files. (Thanks: ViciouZ).
- Tweak: Sound quality defaulted to 48khz (this won't take effect if you've already edited your audio options).
- Media: VWep (other players actually hold the weapon they have - requires server update).
- Media: Some new, higher-quality recordings (ammo and weapon pickups, crickets).


build 23 (2008-05-27):
- Feature: Enter game and disconnect sound effects.
- Feature: Different sound effects for team chat and global chat.
- Tweak: Fixed sound attenuation in software sound mode to have exponential falloff so you're not hearing things on the other side of the map like they're right next to you. 4:33 AM 5/14/2008
- Bugfix: Fixed crash with blank file specification.
- Bugfix: Fixed occasional crash when viewing scoreboard.
- Media: New PGP and Spyder sounds.
- Media: New footstep and falling sounds.
- Media: New chat, connect, and disconnect sounds.
- Media: Tweaked PGP and Trracer animations (these require playing on a new server to work properly).


build 22 (2008-03-29):
- Feature: Added support for variables in file list widgets.
- Bugfix: Fixed "flags can only be 'u' or 's'" issue with the menu.
- Media: New entity files for castle1 and midnight to allow for more game modes, such as TDM and KOTH.


build 21 (2008-02-14):
- Feature: Added support for variables in menu widgets - usable for map previews, etc. 2:03 AM 1/18/2008
- Tweak: Up and down arrows work on select widgets (lists) 2:38 AM 1/18/2008
- Tweak: Added TAB key for selecting different widgets in the menu. 8:26 PM 1/8/2008
- Cvar: cl_drawfps 2 instantly updates framerate instead of averaging. 3:45 AM 1/4/2008
- Cvar: cl_drawtexinfo displays texture name and flags of surface in front of player. 3:43 AM 1/6/2008
- Bugfix: Fixed blending of lightmaps and transparent textures with alpha channels on trans33 or trans66 surfaces. 3:29 AM 1/6/2008
- Bugfix: Fixed crash with "dumpuser" command. 7:28 PM 1/8/2008
- Bugfix: Fixed rscript envmap on transparent surfaces. 7:03 PM 1/24/2008
- Media: New weapon sounds, including dry fires.


build 20 (2007-11-29):
- Tweak: Removed display of text formatting codes at the end of strings as they usually cause more harm than good. 1:33 AM 7/10/2007
- Tweak: Increased the delay between pinginig servers on the server list from 16 ms to 32 ms for more accurate ping times. 11:49 PM 10/3/2007
- Feature: Added "pwd" (and "cwd") commands to display the current working directory (for debugging servers). 3:24 AM 11/9/2007
- Bugfix: COM_FileExtension accounts for periods in the filename/path.
- Bugfix: COM_StripExtension accounts for periods in the filename/path.
- Feature: Fast file transfer system! 4:52 AM 11/28/2007
- Cvar: sv_fast_download enable/disable fast file downloading on a server (on by default).
- Cvar: sv_fast_download_max_rate sets the absolute max total transfer rate in KB/s.  Effectively, rates will be much slower than this, recommend leaving at 0 to disable.
- Cvar: cl_fast_download enable/disable fast file downloads on the client (on by default).


build 19 (2007-06-09):
- Bugfix: Fixed incorrect password error when password is correct after using remember password or changing password. 5:58 PM 6/4/2007
- Bugfix: Fixed crash with invalid rscript files. 5:43 PM 6/5/2007
- Bugfix: Fixed crash on error resulting in shutdown or failure to load game module.
- Tweak: Cached DNS for communicating with login server to remove lag spikes.
- Tweak: Simplified profile/login menus. 2:37 AM 5/31/2007
- Tweak: Added build number to serverinfo version string.
- Media: Added crosshair 11-13 which look similar to crosshair 1-3 in Quake 2. 3:36 AM 6/5/2007
- Media: Fixed bug in soccer1.bsp where players could get launched out of the map with "pong_nokill" enabled. 3:23 AM 6/6/2007


build 18 (2007-05-08):
- Bugfix: Fixed crash caused by status packet sent while playing a demo. 1:50 PM 3/25/2007
- Bugfix: Fixed crash when attempting to load bad wav file (drip_amb.wav) in OpenAL. 3:17 AM 5/6/2007
- Bugfix: Fixed macro processing in $$$var scenario. 2:14 PM 5/27/2007
- Tweak: Switched over to UDP for communicating with the login server. (major rewrite)
- Tweak: Switched to UDP to download the serverlist. (major rewrite)
- Tweak: Changed default timeout value to 30 seconds. 8:10 PM 4/29/2007


build 17 (2007-03-04):
- Bugfix: No longer tries to download .md2 files if the .skm file exists.
- Bugfix: Fixed r_reflective water with gl_ztrick 1 (ztrick just gets disabled, but the screen doesn't flicker).
- Bugfix: Attempted to fix some buffer overflows causing server crashes.
- Bugfix: Fixed minor buffer length issue with client side scoreboard.
- Bugfix: Fixed entity loading problem where entity string was not null-terminated.
- Bugfix: Fixed select widget to position properly when empty.
- Bugfix: Fixed see-through-wall glitch with high FOV.
- Bugfix: Fixed gl_skymip.
- Bugfix: Fixed crash caused by highly populated servers over a long period of time. 3:36 AM 10/3/2006
- Bugfix: Fixed ping time display in the server browser for LAN servers. 2:46 AM 12/5/2006
- Bugfix: Fixed crash when loading non power of two textures larger than 1024x1024. 12:59 AM 1/11/2007
- Bugfix: Screenshots work in the console again.
- Bugfix: Linux input fixes (you may need to set in_dgamouse to 0 if you have problems now).
- Tweak: Cleaned up condump command to get rid of color codes and other garbage.
- Tweak: Fixed paths and letter casing in fopen function for better Linux compatibility.
- Tweak: r_speeds now account for water polygons.
- Tweak: r_speeds now account for reflective water.
- Tweak: r_speeds are colored to help tip mappers off to areas that need polygon reduction.
- Tweak: Defaulted cl_sleep to 0 because it caused some framerate/choppy issues with some people.
- Tweak: Made "build" serverinfo, so you can see what build a server is running from server browsers.
- Tweak: Tweaked alpha channel a bit when mipmapping so textures like fences don't disappear in the distance.
- Tweak: Minor optimization in file reading.
- Tweak: Tried to fix screenshot lag problem.
- Tweak: Added more video resolutions.
- Tweak: Made weapon draw with high FOV values, use cl_gun 0 to disable.
- Tweak: Removed the netgraph background so you can see the hud while it's enabled.
- Tweak: Linux checks for libGL.so (or whatever gl_driver is set to), libGL.so.1.2, and libGL.so.1, in that order.
- Tweak: limited gl_picmip to 2.
- Tweak: Support for sv_forcesky so it won't cause framerate drop.
- Tweak: Limited cl_maxfps to higher than 5. 6:08 PM 12/6/2006
- Tweak: Changed dpball.com to dplogin.com for serverlist downloads. 2:11 AM 2/15/2007
- Tweak: Stripped garbage from names in "status" command. 11:08 PM 2/19/2007
- Tweak: Made the "GLimp_EndFrame() - SwapBuffers() failed!" message caused by buggy drivers only display once.
- Tweak: Increased console exec. buffer from 8k to 32k. 1:45 AM 2/16/2007
- Tweak: Added support for a number of new resolutions, including: 1280x1024, 720x480, 720x576, 848x480, 960x600, 1088x612, 1280x720, 1280x768, 1280x800, 1680x1050, 1440x900, and 1920x1200.
- Feature: Global login account system.
- Feature: Added password flag for text fields (used for login menu).
- Feature: Client now scans for LAN servers on port 27910-27920 so multiple LAN servers can be run on one machine. 2:25 AM 12/5/2006
- Feature: Added simple file check to warn people that modifying content will result in a ban.
- Feature: vid_windowclassname cvar to set the name of the window.  This is so people can set it to "Quake 2" in order to be compatible with keygrip2.
- Feature: Made it so pak files will load from the "pakfiles" subdirectory.
- Feature: timestamp_console cvar to print timestamps at the console.  Enabled by default for dedicated servers.
- Feature: Added ability to name screenshots.  "screenshot test" would save the screenshot as test.jpg.
- Feature: Support for local serverlist files. 4:25 AM 10/4/2006
- Feature: Autologin option for global login. 4:16 AM 11/7/2006
- Feature: Added "sharp" to rscripts to disable filtering (only works if pic hasn't already been loaded). 11/10/2006
- Feature: Xfire support (just displays team color for now). 1:16 AM 12/7/2006
- Feature: "arecord" command, same as "record" but tacks on the map name and date. 3:10 AM 1/3/2007
- Feature: Demo recording avoids file overwrites by tagging on numbers (example: demo_001.dm2 if demo.dm2 exists). 3:11 AM 1/3/2007
- Cvar: cl_customkeyboard uses .kbd files instead of Windows layout.
- Cvar: gl_skyedge cvar to tweak for black sky edges.
- Debug: Added logging for invalid serverlist error. 7:47 PM 2/15/2007


build 16 (2006-02-26):
- Bugfix: Various security patches from R1CH (R1Q2).
- Bugfix: Fixed ctrl+k and other ctrl/shift combos (broken in build 15).
- Bugfix: Fixed some roundoff errors.  Note: This may make the game choppy/laggy if the server and client are running different versions.
- Bugfix: Fixed "sticky" strafe jumping bug that randomly causes players to stop or lose momentum.  Also requires client and server to be running the same version.
- Bugfix: gl_swapinterval (vsync) properly maintains the setting after video mode changes.
- Bugfix: Fixed several potential buffer overflows.
- Bugfix: Fixed some bugs with reflective water.
- Bugfix: Scoreboard now refreshes when player names change.
- Tweak: Increased max lightmap resolution from 34x34 to 128x128.
- Tweak: Made noipx default to 1, since pretty much nobody uses ipx, and it can cause various problems having it enabled.
- Tweak: Serverlist requests are printed to the console to help troubleshoot issues people have been having with the in-game browser.
- Tweak: Increased MAX_STRING_TOKENS from 80 to 512 to prevent truncation of data.
- Tweak: gl_anisotropy defaults to 8, so people have improved graphical appearance by default.
- Feature: cl_sleep now configurable (ie: cl_sleep 100 will make the game run at 10fps, but with much less CPU usage).
- Cvar: r_oldmodels allows players to use the old .md2 models if the new ones don't work properly.
- Cvar: cl_language allows players to select different language translations for the menus and game events.  No language files have been created yet.
- Cvar: cl_centerprintkills allows players to enable/disable printing kills and deaths in the center of the screen.


build 15 (2005-08-19):
- Bugfix: Fixed transparency overlap issue.
- Bugfix: Fixed vsync setting not properly initiating when the game is reloaded.
- Bugfix: Hack to fix nodraw on tranquilhavoc water.
- Bugfix: Fixed clipping bug along walls with steep angles where you couldn't walk, only air accelerate.
- Tweak: Fixed color splotching around dark areas.
- Tweak: Made overflowed message not use extended codes on server console.
- Tweak: Tweaked water jump code to work better for maps like renoir and summer.
- Tweak: Stripped colors from server names in server browser.
- Feature: International keyboard support on Windows (without .kbd files).
- Feature: Lightmaps for transparent surfaces.
- Feature: Skeletal model support.
- Feature: Reflective water with fragment programs (if your graphics card supports them).  Type "seta r_reflectivewater 1" to enable.


build 14 (2005-04-12):
- Bugfix: Fixed serverlist crashing or not loading due to corrupt serverlist files.
- Bugfix: Fixed "seta" command not to overwrite userinfo flags.
- Bugfix: Running a dedicated server no longer wipes the serverlist file.
- Bugfix: Fixed some things with the string encoding, so you shouldn't see "(build ?)" when people connect any more -- it will show the proper number.
- Bugfix: Expanded some buffers so userinfo won't get truncated.
- Bugfix: Replaced vsprintf with _vnsprintf to prevent buffer overflows.
- Bugfix: Fixed glitches in mipmapping.
- Bugfix: Fixed bizarre weapon angles.
- Bugfix: Fixed demo recording to store team colors and other information properly.
- Bugfix: Security fix relating to configs.
- Feature: "paintball2://" url parsing support.
- Feature: Maps you don't have will be italicized in the server list.
- Feature: "alias" command with no second parameter will display what the alias is set to, rather than wiping it.
- Feature: Vote menu beefed up to display only the maps that are on the server rotation.  Temp maps are italicized.  Maps that don't meet min/max player requirements are greyed out.
- Feature: Scrollbars!  Finally!
- Feature: Notification of new versions when updating the serverlist.


build 13 (2004-11-10):
- Cvar: gl_sgis_generate_mipmap [0 or 1] Added because the mipmapping on some ATI drivers is fubar.  Hopefully disabling the extension will fix it...
- Bugfix: If you toggle the console while binding a key, it cancels the bind.
- Bugfix: Framerate issue with ATi's drivers resolved.  Higher-quality particles have been disabled as a workaround.
- Bugfix: Several little things that caused crashes.
- Bugfix: Threshold added to joystick input so it doesn't flood out packets.
- Feature: Serverlist handles vhosts, 302 redirects, and is multithreaded, meaning no temporary freezing and more accurate ping results.  Also the state of the serverlist is saved on exit, so it's there the next time you load up the game (don't have to hit refresh every time).
- Feature: 32bit icon for Windows XP users.


build 12 (2004-09-18):
- Bugfix: fixed illegible server messages, whacky pings, and flag indicator on client-side scoreboard.
- Bugfix: timestamps now work properly with encoded messages.
- Feature: Loc file support from NoCheat client.  Example: say_team "I'm at $loc_here going to $loc_there"  Type loc_help for more information.  Thanks to Xile, FourthX and NiceAss.


build 11 (2004-08-17):
- Cvar: m_xp [0, 1, 2] When set to 1, disables mouse acceleration on WindowsXP.  2 disables acceleration on any Windows version.  0 acts like regular Quake2.
- Cvar: gl_showtris [0 or 1] Now functions properly with multitexture enabled (works with deathmatch 0 only).
- Cvar: cl_drawpps [0 or 1] Displays packets per second sent on the HUD, much like cl_drawfps.
- Bugfix: Serverlist no longer crashes while running off CD (though it won't update either, obviously).
- Bugfix?: Server code no longer drops clients who overflow.  Seems better than disconnecting, but may lead to fubar data.
- Bugfix: Removed check for data CD so it doesn't probe all your drives for something that doesn't exist.
- Bugfix: Stopped game from dropping to 10fps while connecting to a server (still sends data at 10pps, though)
- Bugfix: cl_cmdrate functions more like it should.  Packets/sec sent is closer to set value now.
- Bugfix: RScripts now function properly with HR4 (high-res) textures.
- Feature: Mappers can specify "requiredfiles" in the worldspawn to specify any additional files the client will need to download (such as RScript files).
- Feature: Doubleclicking supported in menus (seen in join and start server menus).
- Feature: RScripts support width and height parameters and no longer require a source image to exist - useful for HUD images.
- Feature: Added support for a unique in-game menu.
- Feature: Chat is handled client side and can thus be colored according to teams, etc. (requires 1.80 or higher server game code).
- Feature: Client-side scoreboards and events.  Less bandwidth is used and scores can be viewed at any time, even with prerecorded demos (requires 1.80 or higher server game code).


build 10 (2004-04-03):
- Cvar: gl_free_unused_textures [0 or 1] I just threw this in for the heck of it (actually for debugging something).  If you have memory to spare, you can set it to 0 and it will keep all textures in memory from level to level, possibly making levels load faster.
- Cvar: gl_hash_textures [0 or 1] An optimization to make image lookups faster.  Leave at 1 unless you have issues with textures not loading correctly.  This cvar will be removed once everything has proven to work properly.
- Cvar: crosshair [1 to 7+] Limit of 3 removed.  You can have any number of crosshairs (pics/ch#.pcx/tga).  A couple new ones have been included.  For those of you who have been complaining about small crosshairs: set crosshair 4
- Cvar: cl_cmdrate [5 to 80] Sets rate (in packets per second) to upload to server.  The client side prediction has been tweaked quite a bit with this and should be smooth with values as low as 15.
- Bugfix: Field boxes no longer truncate colored text too soon.  (The cursor position still needs to be fixed, though).
- Feature: Several optimizations (hashed textures, faster cullbox function, faster strcmp, images have rscript pointers instead of searching for scripts each frame).
- Feature: Screenshots now have 3 digits: sshot000.jpg through sshot999.jpg.
- Feature: Mousewheel adjusts slider widgets.
- Feature: Tweaked the console characters to be brighter (requires gl_overbright 1).


build 9 (2004-03-09):
- Bugfix: Serverlist not showing any servers bug fixed.
- Bugfix: Mouse no longer trapped to the Paintball2 window when you alt-tab.
- Bugfix: Styled lights work now.
- Feature: cl_cmdrate isn't as choppy anymore and you can do all the trick jumps you could in Quake2.
- Feature: Mousewheel works with scrollable menu items (select widgets).


build 8 (2004-02-04):
- Cvar: gl_lightmapgamma [0.45 to 2.0] Replaces gl_modulate to give a proper gamma corrected lightmap.  Be sure to properly configure your gamma correction settings in the video menu for best results.
- Cvar: s_a3d [0 or 1] Enables 3D audio.  Must have OpenAL installed with proper sound drivers.  OpenAL can be obtained here: http://developer.creative.com/articles/article.asp?cat=1&sbcat=31&top=38&aid=46
- Bugfix: Menu no longer comes up when starting the game with +map or +connect.
- Bugfix: timestamps no longer crash.


build 7 (2004-01-19):
- Cvar: sv_minclientbuild [0, 7, or higher] If you are running a server, it checks that the client is using at least this build number (only works with build 7 and up).
- Cvar: con_notifytime [1 or higher] Time (in seconds) to display messages at the top of the screen.  Archives now, also the number of lines displayed has been doubled.
- Cvar: vid_lighten [0.0 to 0.5] Brightens dark areas linearly for those monitors that like to make dark stuff completely black (paired with vid_gamma for more control).
- Bugfix: fixed crashing on some cards due to a multitexture bug.
- Bugfix: Mouse no longer goes crazy when you click outside of Quake2 in windowed mode.
- Bugfix: Old HUD icons aren't so blurry anymore.
- Bugfix: Some buffer overrun security holes from the original Quake2 engine have been fixed.
- Feature: (for mappers) combining trans33+trans66 will make the surface use only the transparency of the texture.  Useful for fences, bushes, and other objects that should only be transparent in selected areas.
- Feature: New font with text formatting!  Ctrl-k followed by a character allows you to color text. 0-9 are black-white, A-Z are a full spectrum of colors.  Lots of other characters have different colors as well.  Ctrl-U underlines text.  Ctrl-I puts it in italics.
- Feature: Scriptable menu system.  Menus are easily customizable with text files and provide a direct interface with the console variables and commands.
- Feature: You can use extended characters in your name.
- Feature: New cvar macros let you place cvars directly next to other text by surrounding them with $'s.  Example: "echo name:$name$"
- Feature: writeconfig command writes settings to a specified file (also available through the menu).
- Feature: seta sets a cvar so that it's archived (saved in config.cfg) like in Quake3.
- Feature: cvar_cat <cvar> <string> command concatenates a string onto the end of a cvar's string, ie: cvar_cat name "oe" will add "oe" to the end of your name. (This is primarily for menu usage).


build 6 (2003-08-18):
- Cvar: gl_lightmap_saturation [0.0 to 1.0] Adjusts the saturation (color) of the lighting.  0 is greyscale lighting, 1 is full color.
- Cvar: r_displayrefresh [0,60,85,120,etc.] Sets the vertical refresh rate (in hz) in fullscreen mode, since WindowsXP sets it to 60hz.
- Cvar: cl_drawhud [0/1] toggles display of HUD content (ammo count, etc).  Useful for getting clean map shots.
- Cvar: gl_highres_textures [0/1] Enables use of high resolution (4x current resolution) textures stored in the hr4/ subdirectory of textures.
- Bugfix: fixed issue with console causing transparent surfaces in maps to render improperly.
- Bugfix: demo playback is no longer jerky.
- Bugfix: rscripts are now functional on transparent surfaces.
- Feature: Mappers can add fog to their maps!  "sky" "fog <r> <g> <b> <distance>" or "sky" "[skyname] fogd <r> <g> <b> <density>", ie "sky" "pbsky4 fogd .5 .5 .6 .002"
- Feature: screenshots now take into account vid_gamma and accurately reflect what you see in-game if you are using hardware gamma.
- Feature: The client no longer attempts to download the same missing file multiple times if the server doesn't have it.
- Feature: Missing textures are attempted to be downloaded in the following order: jpg, tga, pcx or wal.
- Feature: 3dfx (ancient video card) support.  Reverts to glide minidriver if a 3dfx card is present and regular OpenGL does not load.  Set gl_driver to 3dfxgl and vid_restart to manually set.


build 5 (2003-06-20):
- Cvar: gl_allow_software [0/1] allows software emulation of OpenGL if your card doesn't support opengl (REALLY SLOW).  This cvar has always been there, but now it archives.
- Bugfix: Removed age-old problem with phantom buttons being left over in the taskbar.
- Feature: added "offset" to rscripts (so I could do that cool binary console)


build 4:
- Bugfix: some issues with demo playback resolved (freelook, crashing, loading wrong maps)
- Bugfix: HUD graphics (ammo count and whatnot) should all display properly now
- Feature: Removed the "Can't find pic" spam for missing images (this shouldn't really affect anyone.  I did it just 'cuz).
- Feature: added vsync (gl_swapinterval) to the video menu


build 3:
- Cvar: gl_anisotropy [0 to 16+] Anisotropic filtering!
- Cvar: cl_hudscale [1 to 4+] adjusts the size of the hud and text
- Cvar: gl_motionblur [0.0 to 0.99] adds motion blur (only supported by some video cards)
- Cvar: gl_texture_saturation [0.0 to 1.0] adjust the amount of color in the textures (0 is greyscale, 1 is full color).
- Cvar: cl_cmdrate [10 to 80] sets the rate at which packets are sent to the server (in packets/sec).
- Cvar: cl_locknetfps [0/1] if enabled, locks the network code and framerate code together, making cl_cmdrate act like the old cl_maxfps (which is now disabled)
- Cvar: cl_sleep [0/1] let's the cpu sleep between frames to cut down on cpu usage (however it only appears to work in the debug build)
- Cvar: serverlist ["www.planetquake.com/digitalpaint/servers.txt"] sets the location to obtain the server list from.
- Cvar: name ["newbie"] The name cvar can now hold funnames (names with extended characters).  This must be set in your config BEFORE connecting to a server.
- Bugfix: fixed crashing on startup for some video cards
- Bugfix: fixed problem with particles causing HUD to flicker/disappear on some cards
- Bugfix: default.cfg only executed if config.cfg not found (fixes problem with configs getting overwritten)
- Bugfix: allowed "timedemo" to be turned on when playing ANY demo (originally this only worked for single player demos)
- Feature: Mousewheel works to scroll through the console.
- Feature: Insert text mode at the console (INS key toggles).
- Feature: console logs to qconsolePORTNUMBER.log now (so you can log the console on multiple servers)
- Feature: support for up to 5 mouse buttons (might have to disable mouseware for Logitech mice).


build 2:
- Bugfix: fixed crashing in multiplayer/start server menu


build 1:
- Cvar: splattime [0.0..10.0] time (in seconds) splats stay visible
- Cvar: cl_drawfps [0/1] toggles display of framerate on HUD
- Cvar: cl_timestamp [0/1] toggles display of timestamps on messages
- Cvar: keyboard ["qwerty", "dvorak"] sets the keyboard layout in-game (requires restarting paintball2)
- Cvar: gl_overbright [0/1] if enabled, doubles the brightness of everything (only supported by "newer" video cards in multitexturing mode)
- Cvar: gl_brightness [0.0 to 1.0] Acts similar to overbright but can decrease image quality and framerate.  Useful for old video cards that don't support overbright properly.
- Cvar: gl_autobrightness [0.0 to 1.0] paired with gl_brightness, this cvar controls the degree to which the screen brightness is controlled by the brightness of the map.
- Cvar: gl_showbox [0/1] toggle for displaying box outlines of models (debugging feature)
- Cvar: gl_ext_texture_compression [0/1] Enables texture compression, but greatly reduces image quality.
- Cvar: gl_screenshot_jpeg [0/1] toggles TGA/JPEG screenshots.
- Cvar: gl_screenshot_jpeg_quality [1 to 100] sets the quality of the jpeg compression.
- Cvar: gl_stainmaps [0/1] toggles paint stains on maps
- Cvar: vid_gamma_hw [0/1] toggles the use of hardware gamma to adjust in-game brightness (may crash on older video cards)
- Cvar: vid_gamma [0.5 to 1.3] adjusts the hardware gamma level (this will affect all of windows if you are running in windowed mode)
- Cvar: gl_debug [0/1] suppresses/enables display of all the OpenGL information to the console upon starting the game or vid_restarting
- Cvar: gl_shadows [0, 1, 2] 0 = no shadows, 1 = old (ugly) shadows, 2 = stencil-buffered shadows
- Bugfix: allowed for proper 44khz sound
- Bugfix: WSAECONNRESET errors don't kill the server
- Bugfix: "sz_getspace overflow without allowoverflow set" crash fixed (for the most part)
- Change: Base directory now "pball", not "baseq2"
- Change: gl_modulate capped at 2.0 to prevent abuse
- Change: Dropped support for software rendering mode (go spend $20 on an OpenGL capable card if you don't have one!)
- Change: Configs are now executed from the "pball/configs" directory to reduce the clutter.
- Change: Quit confirmation/credits removed
- Feature: q2msg protection
- Feature: r_speeds now display on the HUD rather than spamming the console.
- Feature: rscript support (basically like Quake3 shaders)
- Feature: JPEG and TGA image format supported for textures
- Feature: Better particle effects for paint grenades
- Feature: aliases are saved along with the binds in config.cfg
- Feature: Sky distance increased so large maps don't get cut off in the distance.
- Feature: Advanced tab completion
- Feature: /quit, /vid_restart, /disconnect, and /record are the defaults when you hit q,v,d, or r, then TAB (since those are the commands I use the most).





Please refer back to http://www.digitalpaint.org/ for further updates and documentation.

(c) 1998-2013 Digital Paint