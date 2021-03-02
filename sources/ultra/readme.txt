
 Eraser ULTRA                                v0.47 (Public Beta)
 by Anthony Jacques (aka AnthonyJ)
 and ScarFace
................................................................

================================================================
Title                   : Eraser ULTRA
Author                  : Anthony Jacques (aka AnthonyJ) and ScarFace (scarface@quakemail.com)
Homepage                : http://www.btinternet.com/~AnthonyJ

Description             : Lithium-like game with CTF and Eraser
                          Bot Support, for practice.

Additional Credits to   : id Software. 
                          Ryan Feltrin (aka Ridah) for the 
                          excelent Eraser Bot (and source).

                          Matt "WhiteFang" Ayres for writing Lithium
                          (shame you dont release your source).

                          Charles Kendrick, Rich Tollerton
                          Tim Adamec, Michael Buttrey and Nelson Hicks
                          for ExpertCTF (ie. the grapple code)

                          Nathan O'Sullivan for Zbot detector code.

                          Richard Jacques (Swelter[SLH]), Shryqe,
                          Subhuman and Bong for ideas/testing/encouragement.

                          Credits from the Eraser, which carry through:
                             Jeremy Mappus (aka DarkTheties)
                                for the MapMod source
                             The SABIN Team for some Client Emulation code
                             Steve Yeager (author of the ACE bot) for tips
                                on creating a static library for the nav
                                code (stevey@jps.net)
                             Paul Jordan for the Camera mode
                                (http://www.prismnet.com/~jordan/q2cam)

Build Time              : unknown... 
================================================================

DESCRIPTION

   The Eraser Bot is a simulated multiplayer opponent, for
   use with id Software's Quake2. Lithium II is a very configurable
   server side deathmatch modification for Quake II.
   Eraser ULTRA provides the a similar set of configurable options, 
   however with the added bonus of CTF (Capture the Flag) and 
   Eraser Bot support.

   Unlike Lithium, EraserULTRA is intended for PLAYERS - it is
   not designed for server admins, but is simply for practicing.
   EraserULTRA is not in competition with Lithium because of this.

INSTALLATION

   Just unzip the files contained in the archive, to your
   Quake2 folder, RESTORING PATHNAMES. This means
   that if you're using Winzip, you must enable the
   "Use Folder Names" option when extracting. Pkunzip
   users should use the -d option.

   To install routes tables, after unzipping them put the .RTZ
   files into the quake2\ultra\routes\ directory.

   Note that you MUST have some routes (either created by yourself
   or downloaded) to have a "good" game. For notes on creating routes,
   read Ridah's comments in eraser.txt

LINUX NOTES

   Linux users should unzip the .rtz files to get a .rt3 file
   before placing into the quake2/ultra/routes/ directory.

   Also note that all text files (including replace.lst, config.lst
   and any other file referenced) should have ^M's stripped, as
   there are issues with CR/LF under linux. Use the "dos2unix"
   command to process this. 

RUNNING THE GAME

   Like any Quake2 mod, to start EraserULTRA you must start 
   Quake2 with a command line. Windows users should right-click and 
   select "new" -> "shortcut". When prompted, use the browse button
   to find where you installed Quake2, and then add the necessary
   extra parameters in the "command line" box. 

   If "config cycling" is to be used (see below), then use
   a command line such as:

      quake2.exe +set game ultra +exec cycle.cfg

   A CTF cycle is also provided - use the command line:

      quake2.exe +set game ultra +exec ctfcycle.cfg

   If you do not wish to use config cycling, use a command line
   such as the one below. Note that the configs supplied for
   config-cycling need alternation if used without config cycling
   (copy the contents of cycle.cfg to the file):
      
      quake2.exe +set game ultra +exec ultra.cfg

3-Team CTF

   This is still very "beta". I am releasing this version
   as there have been a significant number of requests for
   this release.
   ScarFace- bots will now fill out the green team.  Just make
   to sure to spawn a quantity of bots that is one less than a
   multiple of 3.

   I have had great difficulty in producing a "good" route
   file for any of the 3tctf levels due to their size. This
   may make this game very limited, as CTF does rely heavily
   on having a good route.

   To play 3tctf, simply  type "ttctf 1" and change map to a 3tctf
   level (eg using the command "map 3tctf1")

Xatrix/Rogue support
 
   Again, this is still in development. All weapons are now
   spawned, however bots will not use the new weapons. In progress.

   ScarFace- the Rogue powerups/items are now implemented.  The bots
   will use the double damage and will grab and activate the IR goggles.
   The spheres do work against bots, but bots will not use them.  The bots
   will attack dopplegangers.

   To launch a mission-pack game, all you need to do is:
      1) copy/move the PAK files from the xatrix and/or rogue
         directories
      2) launch quake2 with the command line:
            quake2.exe +set game ultra +exec xatrix.cfg
         or
            quake2.exe +set game ultra +exec rogue.cfg
   If you choose, you can simply put the xatrix levels into your
   map cycle

MEGA GIBS OPTION
   Type "mega_gibs 1" in the console to enable.  Enables tons of bouncy,
   squishy gibs.  Set to 0 to disable or type "purgegibs" if you get overflowed.

ADDING BOTS

   From within the game, type "bot_num X" to spawn X 
   number of bots. Replace X with your chosen number.

   If you are having problems getting bots to work, I
   recommend that you read the Eraser Bot documentation
   (supplied), or perhaps use a front-end such as Bot Johnny
   or EBS - see the FAQ below for details. 

CONFIG CYCLING

   EraserULTRA is very flexible - you can create configs
   to cover many of the major playing styles, and game
   variants. Because of this, it might be that you want to
   switch between them so that you get to practice all of
   them.

   To allow this flexibility, EraserULTRA comes with a 
   number of pre-built configs (in the configs directory), 
   and a start-up config called "cycle.cfg" which can be
   put on the command line to start a config cycle. This
   will cause EraserULTRA to change config every map to
   the next one in the file "config.lst".

KEY BINDINGS

   It is recommended that you bind a key to the following:
      +hook
         (this fires the off-hand grapple)
      drop tech
         (these drops tech/rune/artifacts)
      drop rune
         (identical to drop tech, so not required)
      drop flag
         (drop a CTF flag, if allow_flagdrop set to 1)
      menu
         (brings up the in-game menu - recommend key is TAB)


SUPPORTED MAPS

   The Eraser is capable of dynamically learning maps, from
   humans whilst playing the game. It is advised that 
   instead of the user teaching the bots routes for all
   the levels, the "route tables" are downloaded - they 
   can be obtained as part of the standard EraserBot
   distribution, and also available for download from the
   following WWW sites:

     http://www.botepidemic/q2rfd/
     http://phil.online-kdc.de/philsera.htm 

   To install route tables, after unzipping them put the .RTZ
   files into the quake2\ultra\routes\ directory.

   Producing a good route is an art - a poorly created route
   will suffer from bots being unable to handle some parts of
   the map correctly, or taking unusual routes across the
   level. Ridah made some notes on how to create a good route
   in the Eraser docs, although this wont instantly make you
   able to create the "perfect" route.

   Note that although route tables designed for standard Eraser
   will work with EraserULTRA, bots will not make full use of
   the grapple unless it was built with the grapple enabled.

NEW TO THIS VERSION!
0.47
   Removed all explicit tests against the deathmatch flag due to reports that
      the load menu is displayed on respawn.
   Added ammo_respawn and weapon_respawn to allow respawn times to be tweaked.
0.46a
   Fixed reversed team flag capture sounds in LMCTF mode.
   Fixed bots shooting wildly in the air at no target with the hyperblaster.
   Now the runes really do increase with the number of players-
      I forgot to check if this was working.  You can check the
      number of runes in the game at any time by typing runecount
      in the console.
   Func_explosives can now show up.  Set allow_func_explosives
      to 1 before starting a game.
   Added pack_magslug, pack_flechettes, pack_rounds, pack_prox,
      pack_tesla, and pack_traps cvars.
0.46
   Fixed bug where players on opposite CTF teams couldn't damage
      each other when they had the same user-selected non-CTF skin.
   The number of runes spawned now increases with the number of
      players: # of runes == # of players * rune_perplayer, with a minimum
      of 5 runes and a maximum of rune_max.
   In LMCTF mode, players are randomly assigned one of 27 male or
      one of 11 female skins for each team.  Females now have the correct
      sounds, and Crackwhores are forced to female, and all other PPMs
      are forced to male.
   Added a quick config (lmctfquick.cfg) to allow players to quickly switch
   into LMCTF mode.
0.45
   Added health_bonus cvar for pickup value of stimpacks.
   Off-hand grapple and teleporter crash fixed.
   Fixed typo that prevented pack_grenades cvar from working.
   Fixed wrong scoreboard pic names in CTF.
   sv bluebots and sv redbots commands now only work in CTF mode.
   3Team CTF fixes: bots now fill out all 3 teams
	show correct flag carrying models
	show correct icon on green flag pickup
	show flashing green flag icon when carrying green flag
	show highlighted green team tile when on green team
	sv greenbots command now only woks in 3TCTF mode
   Rogue powerups/items implemented.
   Bots will now attack dopplegangers.
   Added double dropping when quad dropping is enabled.
   Added sorted Rogue/Xatrix ammo HUD: def_hud 4
   Added mega gibs option: mega_gibs 1
   Added changeable blaster bolt color:
	blaster_type 1 for default yellow-orange
	blaster_type 2 for green blaster and hyperblaster
	blaster_type 3 for blue blaster and hyperblaster
	blaster_type 4 for green blaster and blue hyperblaster
   Added support for additional CTF skins starting with ctf_r, ctf_b, and ctf_g.
0.40
   rune_regenmax replaced with rune_regen_health_max and rune_regen_armor_max.
   rune_regen_armor_always cvar added to allow LMCTF style regen rune.
   Scarface has implemented weapon balancing for the missionpack weapons - 
   check the supplied xatrix.cfg and rogue.cfg for the names of all the cvars.
   Scarface has implemented max_armor and pack_armor cvars.
   Scarface has added cvars to allow the hud-icon to be changed for the runes.
   Added support for "hook_style" cvar, where 0 is the familiar grapple, while 1
   gives a laser grapple like Lithium, or Abandon Hope.
   Added "hook_color" cvar with possible values such as "red", "green", "blue".
   Added "inivisible_dmstarts" cvar which hides the spawn points (useful for LMCTF)
   Bugfix: Unable to switch to green team when in normal CTF
   Bugfix: Finally fixed the auto-download problem with the HUD strings
   Bugfix: Fixed the "no pic" messages when respawning with the ammo-hud
   Added armor_bonus_value cvar for pickup value of armor shards.
   Added powerup_max cvar for powerup carrying capacity.
   Added time duration cvars for each powerup.
   Other unrecorded changes.
0.35
   Rogue weapons re-implemented, and working correctly for players.
   "Funhook" support added (request)
   Grapple head now controlled by CVAR "grapple_mdl".
   Bugfix: Strength and Haste runes now have sounds!
   Bugfix: bots now understand the Xatrix ammo, so they dont complain about it now.
   Bugfix: Both flags are now dropped if a player is killed with both (3tctf)
   Bugfix: CTF mode crashes resolved.
0.30
   Re-introduced Rogue platforms... not certain of this, so can disable 
   with "showrogueplats" cvar.
   Added slime_mod and lava_mod cvars to control PLAYER damage as well (request)
   Rogue support REMOVED due to the damn bugs it caused....
   Lots of new cvars (eg start_chainfist) for rogue/xatrix support
   OneGun values up to 17 now valid - for the rogue/xatrix weapons
   Rogue support added - all the weapons/items, and the lifts as well...
   Xatrix support MUCH improved... just got to teach the bots now.
   Added IP banning (lots of people seem to want it set banlist banlist.txt)
   Added IP logging (set use_iplogging 1)
   3-Team CTF support added - fairly stable
   Bots can now pickup Adrenaline and Powershield/screen
   Bugfix: bot AI corrected for vampire rune
   Bugfix: altered Hyperblaster aim code!
   Bugfix: "cam off" no longer leaves the player invisible
   Bugfix: launcher.cfg only launched once now
   Bugfix: HUD now cycles to/from CTF mode correctly
   Bugfix: "use_configlist" cvar now actually works!
   Bugfix: Safety mode now works properly for bots
   Bugfix: AI tweak was required to get vampire used properly by bots
   Changed DMFLAGS so that it can be set on a per-level basis
0.27
   Fixed grapple usage - "use_hook" enables a CTF style grapple, "hook_offhand" enables 
   the offhand grapple. Note that you can have both at once! (bots prefer offhand).
   Implemented anti-Zbot code (thanks Nathan O'Sullivan)
   Removed dm/ctf_config - replaced by "ctffraglimit" cvar for CTF mode
   Implemented config cycling - new file config.lst
   Implemented "start_rune" which gives players a rune on startup
   Implemented "allow_runedrop" - if 0, cannot drop runes
   Implemented command "bot_drop_all" to kick all bots (if bot_num set, they reconnect)
   Bugfix: replace_list is now cleared when reloading the list
   Bugfix: vampire rune no longer LOWERS health if above rune_vampire_max
   Bugfix: bot_hyperblaster_damage now works correctly
   rune_flags now implemented
   Picking up the flag now cancels safety mode
   Runes (again) spawn throught the level, rather than just at DM restarts
   Implemented "allow_pickuprune" cvar to allow LMCTF/Lithium style ability to pickup 
   runes/tech's previously dropped.
   Implemented "allow_flagdrop" cvar to allow LMCTF style "drop flag" commands
0.26
   Started work on some Xatrix (The Reckoning) features.
   Fixed yellow shells (haste rune, and safety mode) for 3.19
   Implemented yellow shell when in safety mode
   Implemented bot_railtime to tweak lag on bot railgun aiming
   Implemented use_lasers to allow target_laser entities to be disabled to lower suicides
   Implemented bot_lava_mod and bot_slime_mod to reduce damage caused by lava/slime 
   Implemented *_speed variables (bot and normal weapon balancing)
   Bugfix: bots now use their starting weapons, even on their first spawn
   Bugfix: HUD resetting to 0 on respawn reduced
   Bugfix: FPH counter now stops at the end of the game
   Bugfix: onegun will force the starting weapon to the chosen weapon
   Bugfix: if start_weapon has no ammo, it is still used
   Bugfix: offhand grapple now drops nodes in the same way as on-hand grapple.
   Bugfix: Correct "Navlib" now used, so hopefully no incompatibility with 1.01 routes
   The "ammo hud" (def_hud 3) is now implemented
   Item replacement now configured via a separate config file
   Onegun mode forces Weapons-Stay (dmflags) to OFF, as it makes no sense
   Item/weapon banning is disabled when bot_calc_nodes is set, because it will
   cause problems when using the route when items are not banned.
   Linux version working
0.25
   Bugfix: fixed replace/onegun (broken during migration to Eraser 1.0)
   Bugfix: fixed haste rune sound (broken during migration to Eraser 1.0)
   Bugfix: +hook should now work on all clients in a multiplayer game
   Bugfix: observers can no longer use off-hand grapple to hurt other players
   Bugfix: lag command now affects offhand grapple (it is still wrong though!!)
   Config files for LMCTF and ExpertCTF style setups supplied.
   New cvar (rune_regen_armor) enables/disables CTF style armor regeneration
   when using the regen rune.
   New Cvar's for bot weapon balancing
   Bots no longer use CTF grapple, so it works without a CTF pak file
   Migrated across to Eraser 1.01 source code
   Further optimisation to spawn/init code

0.24
   Migrated across to Eraser 1.0 source code
   Bugfix: intermission nolonger aborts immediately
   Bugfix: vwep now works properly
   Bugfix: weapon balancing is implemented for bots as well as humans
   Bugfix: real players are now visable in multiplayer games
   Bugfix: clients are no-longer silent (no footsteps)
   Optimisation to spawn/init code - should be smoother now.

0.21
   Bugfix: now WILL switch to blaster (if you have it) when no ammo
   Bugfix: def_id is working again.
   changed the directory name from eraser\ to ultra\ to save existing configs.

0.20
   "One-gun" implemented
   Item Replacement
   Bugfix: number of players
   Bugfix: blaster working when no ammo when start_blaster 0
   Many other bits I cannot remember.   

0.10 (private beta)
   Everything, as its new!


FREQUENTLY ASKED QUESTIONS

   Q: Will [xxx MOD] be supported?
  
                Just implementing Lithium was quite a bit of work - however
                because a lot of the mods are effectively doing the same
                thing, it is quite possible that with only a small(ish)
                amount of work, I can implement other mods. I will not
                say that any mod will definately be supported, but I am
                looking at a few that I may attempt to implement.

   Q: What front-ends support EraserULTRA?

                BotJohnny has support for EraserULTRA. See http://bj.quake2.co.uk/
      
                EBS (Eraser Bot Shell) supports most EraserULTRA settings. 
                See http://www.altern.org/incubus/
             
                EBL can launcher ULTRA by changing the mod directory, but
                cannot edit ULTRA options. The same is true for Eraser QuickStart.

                Other launchers may or may not support EraserULTRA. 
                [details, anyone?]


DISCLAIMER

   This is a BETA release, I therefore will not take responsibility
   for your system barfing after playing the game. I can however
   guarantee that I have not purposely added any malicious content
   to this application. If you believe this to be incorrect, then
   I'd be happy to discuss the matter with you.

   You may freely distribute this archive, as long as it remains
   PERFECTLY intact, as distributed on my home page:
      http://www.btinternet.com/~AnthonyJ/

LEGAL BIT

   Ridah asks that the following statement be included in the
   docs of any modified version of Eraser. Fine. :-)

	--- BEGIN ---

	The Eraser Bot is a product of Ryan Feltrin, and is available from
	the Eraser Bot homepage, at http://impact.frag.com.

	This program is a modification of the Eraser Bot, and is therefore
	in NO WAY supported by Ryan Feltrin.

	This program MUST NOT be sold in ANY form. If you have paid for 
	this product, you should contact Ryan Feltrin immediately, via
	the Eraser Bot homepage.

	--- END ---

KNOWN BUGS

 - Spectacular deaths cause the netchan-transmit error.
 - OneGun doesn't respect weapon banning.
 - OneGun has problems choosing an ammo/weapon type not found on the current map.
 - Ammo HUD causes small number of "pic not found" error messages.
 - Autodownload causes dodgy files to be created in the PLAYERS directory
 - When MAX_CLIENTS is set to high values (eg 128) strings within the game are overwritten.

Please report bugs to: AnthonyJ@planetquake.com
(please note the new address)

enjoy,
-Anthony Jacques
