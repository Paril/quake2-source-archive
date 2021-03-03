================================================================
 The Weapons Factory Eraser Bot                    v1.01 (Final)
 by Acrid-
................................................................

================================================================
Title                   : WFEraser Bot
Author                  : Acrid-
Homepage                : http://www.captured.com/weaponsfactory

Description             : Human-like AI for Simulated Quake2
                           Deathmatch play and Capture the
                           Flag

Additional Credits to   : Ryan Feltrin (aka Ridah) 
                          http://impact.frag.com 
                          id Software for being id Software
                          Josh Holdaway, creator of Quick Start,
                            and the Final Eraser installer
                          The Info-Zip team for the zip/unzip tools
                            (http://www.cdrom.com/pub/infozip/)
                          Jeremy Mappus (aka DarkTheties) for
                            the MapMod source
                          Grimlock for the colored skins
                          Rowan "Sumaleth" Crawford, for the consol
                            background art
                          Brett "B-MonEy" McMahon, for support
                            and ideas
                          Nigel "rkm" Bovey for the linux port
                          IMP for the Eraser logo
                          Pete Elespuru for the consol mod
                          The SABIN Team for some Client Emulation code
                            (http://www.planetquake.com/botshop/sabin)
                          Steve Yeager (author of the ACE bot) for tips
                            on creating a static library for the nav
                            code (stevey@jps.net)
                          Paul Jordan for the Camera mode
                            (http://www.prismnet.com/~jordan/q2cam)
                          NIQ changes by Mike Fox (a.k.a. Artful Dodger)
                          SubHuman (http://www.planetquake.com/outpost/)
                            for extensive testing and feedback
						  Fred (fred@planetquake.com) for many of the
						    routes
						  L-Fire for the built-in View Weapon tips
						    (LFire@yyz.com)
						  
                          Anyone else who has contributed in
                          any way to the development of Eraser.

Build Time              : many hours that should have been
                          spent sleeping
================================================================

DESCRIPTION

   The Eraser Bot is a simulated multiplayer opponent, for
   use with id Software's Quake2. It has been developed with
   speed and accuracy in mind, so that you can play with more
   bots, with higher intelligence.


INSTALLATION

   WIN95/NT INSTALLER

   Double click on the EXE file, then specify your Quake2
   folder and press Unzip.

   -OR-

   ZIPPED VERSION (Win95/NT & Linux users)

   Just unzip the files contained in the archive, to your
   Quake2 folder, RESTORING PATHNAMES. This means
   that if you're using Winzip, you must enable the
   "Use Folder Names" option when extracting. For
   pkunzip users (bless their souls), make sure
   you use the -d option.


RUNNING THE GAME

   To run the game, type the following from the DOS Shell
   command line, whilst inside your Quake2 folder:

   quake2 +set game eraser +map <mapname>
   (please read below to find out which maps are supported)

   The from within the game, type "bot_num X" to spawn X 
   number of bots.

   If you do not see any bots in the game, then I
   advise you to read the Frequently Asked Questions
   near the end of this file.


TROUBLESHOOTING

   If you've followed all the instruction to the best of you're ability
   and yet fate still prevents you from getting Eraser to work, then
   point your browse to http://www.telefragged.com/epidemic/guides/ebgide.html
   This document contains more than could be said in an email, and is
   pretty a failsafe set of instructions.


SUPPORTED MAPS

   The Eraser is capable of dynamically learning maps, from
   humans whilst playing the game. However, maps that aren't
   supported by the release (and hence will require dynamic
   learning), will suffer from less intelligent behaviour,
   until the map has been played for a while (usually 10-15 mins).

   It is now possible to play the Eraser bot on any map, it will learn
   the map as you play, so just make sure you move around if playing
   a new, or previously unplayed map. You should find the intelligence
   pick up after about 5 minutes of play.

   The following user-made maps are highly recommended, since they are
   best suited for Deathmatch play:

   <mapname>         <url>

   ikdm1             ftp://ftp.cdrom.com/pub/idgames2/quake2/levels/deathmatch/g-i/ikdm1.zip
   severed1          http://planetquake.com/cdrom/ramshackle/SEVERED1.ZIP
   mpq1              ftp://ftp.cdrom.com/pub/idgames2/quake2/levels/deathmatch/m-o/mpq1.zip

   Many thanks to their respective authors.

   Follow the installation instructions for each map, they can be installed
   as usual, to your quake2\baseq2\maps directory, and will work fine with
   the Eraser.


MAP CYCLE

   When running a dedicated server, or using the TIMELIMIT or FRAGLIMIT
   commands, you may want to specify your own cycle of maps. This is now
   possible due to the inclusion of the MapMod code into The Eraser. 

   All you need to do, is edit the file called "maps.txt" in the Eraser directory,
   and within that file, list the series of maps you want to run. When the end
   of the list is reached, the map list will cycle back to the first map.
   
   Thanks to Jeremy Mappus (a.k.a DarkTheties) for the MapMod code.


GAMEPLAY SETTINGS

   Skill Levels

	   You can increase or decrease the level of the opponents, using
	   the "skill" setting. The default being "1", if you set this
	   to "2", then the general skill levels of all bots will be raised.
	   They will still maintain their individuality, just some will
	   be slightly better in areas they may not have been on skill "1".

	   Note that unless you disable "bot_auto_skill" (see below), Bot's 
	   will vary their skill as they play, to make the game more even.
	   The skill setting will be used as the starting skill level for
	   bot's, when auto skill adjustment is enabled.

	   Values: 0 (beginner) through 3 (advanced)

   Bot Names/Personalities

	   You can edit the names and attributes of the bots, by editing 
	   BOTS.CFG, located in the Eraser directory.

   Deathmatch Variations

	   Using the "dmflags" setting (accessed via the Multiplayer Menu),
	   you can enable a disable certain rules. Currently, all settings,
	   other than "Teamplay" and "Infinite Ammo" are supported.

	   "Weapons Stay" means that weapons will remain after being picked
	   up, unless they were dropped by another player. This is a
	   personal favourite of mine, and I think makes the game much
	   more exciting, if less strategic.

	   Please see your Quake2 manual for descriptions of the other
	   settings.

	   Values: Use the Multiplayer->Start Network Server->Deeathmatch Flags
				to set the flags you want to play with


CAPTURE THE FLAG								*** new to version v0.8 ***

	Eraser now supports CTF! Just copy the pak0.pak file from your CTF
	directory to the Eraser folder, then load up one of the CTF maps 
	(q2ctf1 - q2ctf6). Eraser will detect a CTF map, and will enable the 
	CTF code.

	To summon team members to "Raid the Enemy base", just enter:

		rushbase

	To summon team members to "Defend our base", just enter:

		defendbase

	at the consol, or bind a key:

		bind r "rushbase"
		bind d "defendbase"
		bind f "freestyle"		(return to normal)

	then just press it during play to send all available troops into the
	enemy base.


	To spawn a group of particular bots on a particular team, use:

		sv bluebots <name1> <name2> ...			(up to 10 bots at once)
		sv redbots  <name1> <name2> ...			(up to 10 bots at once)

	Otherwise, random bots can be spawned, by "ctf_auto_teams <n>" (see Consol Variables below).

	
	To enable advanced use of the grapple hook (which can be quite annoying):

		bot_tarzan 1

	--- CTF specific commands ---

	ctf_auto_teams <n>

      This sets the ideal number of players on each team during
	  CTF play. The program will try it's best to maintain
	  that number of players by spawning and dropping lowest
	  scoring bots as players leave and enter the game. If it
	  cannot do this (there are no bots to drop on a team with
	  too many players) it will spawn additional bots to keep
	  the teams even.

	ctf_special_teams [0..n]

	   When ctf_special_teams is set to 1 then each human will have 1 bot to 
	   play against. 

	   It is multiplier, you can increase the value to any number as long as 
	   it does not exceed the number of maxclients. As each human departs the
	   corresponding lowest scoring bots get dropped.

	   When all human depart all bots are removed thus saving CPU cycles for
	   another quake server.

	ctf_humanonly_teams 0/1

		If this is set then only humans will allowed in a team and bots in the
		other. 

		Once the game has started all humans joining the bot team will be forced
		into the team with humans.

		Any bots will then be added to the other only.
		Note: ctf_special_teams must be set for this to work.


	
	** new to v0.99 **

	It is now possible to turn normal maps into CTF maps. To do this, you must drop the flags
	using "redflag" and "blueflag" when standing in the desired position of the relative flags.
	They won't appear straight away, but will be saved in with the route data. To play that map
	in CTF mode, simply type "ctf 1" in the consol, and then restart the map.

	NOTE: If you drop a CTF flag in the wrong place or by accident, use "clearflags" to remove them.

	NOTE: You can use "toggle_flagpaths" to enable/disable the showing of the direct links between
			flagpaths (not the actual routes, since showing all routes will break the network code).

	You can also place any other item, using the "ctf_item <classname>" command. These items
	will only be spawned in CTF mode. Just move to the desired position, and type the
	preceeding command, specifying tha actual classname for the item you want.

	For example, "ctf_item weapon_chaingun" will place a chaingun at your current position,
	whenever you load the current map with "CTF 1" set.

	This is handy for evening out bases, in non-CTF maps (although it will work for normal CTF
	maps also).

	NOTE: You can clear all ctf_item entries by typing "clear_items" at the consol.

	Capture the Flag can be found at..

		ftp://ftp.idsoftware.com/idstuff/quake2/ctf/q2ctf102.exe

-----------------------DO NOT USE THESE BELOW-------------------------------
TEAMPLAY

	Getting Started

		The Eraser bot now fully supports teamplay, with up to 64 pre-defined teams
		(configurable via BOTS.CFG). To create a team with you and some bots, type:
		"cmd join <teamname>". Currently, you can only join teams that have been defined
		in BOTS.CFG (to get a list of available teams in the game, type "cmd teams").

		When joining a team that isn't currently in the game, a group of bots will
		automatically spawn and join your team. The number of bots that join you,
		is dependant on the current value of PLAYERS_PER_TEAM (see CONSOL VARIABLES 
		section), and the number of bots specified for that team in BOTS.CFG.

		To add a team of bots to play against, type "addteam <teamname>". If you
		are running a dedicated server you MUST do this in order for teamplay to
		function, since in dedicated mode, clients cannot create teams using 
		"cmd join <teamname>". They can only join teams that are already in the game.

		NOTE: if you want to play as a certain member of a team, that has a bot
		defined for that team in BOTS.CFG (eg. when I play for IMPACT), you must
		set your name to the bot's name + [abbrev], so for example, I play as
		"Ridah[IDT]". This way the program knows not to spawn another Ridah bot.

	Rules

		Currently, the rules of Teamplay are very simple. Your team gets a frag
		for every frag you score. It also loses a frag, everytime you suicide, 
		fall in lava, or kill a teammate. This will be expanded in future releases
		to support a wide range of Teamplay rules, similar to that of DMFLAGS.

	Scoreboard

		The Teamplay scoreboard is somewhat different to that normal deathmatch.
		When you join a team, you will start seeing the special Teamplay scoreboard.
		This sorts the teams in order, with each team having their sorted players
		to the right. Your team is indicated with the Q2 tag behind the Teamname
		and score.

	Teamplay Commands

		players_per_team <n>

			This secifies the maximum number of player that are allowed to join
			each team.

			eg. players_per_team 8

		join <teamname> OR <abbrev>

			Places you on the specified team. If the Team does not yet exist in
			the game, it will be created (ie. bot teammates will be spawned).

			eg. "cmd join impact", OR "cmd join idt"

		addteam <teamname> OR <abbrev>

			Adds the given team to the game. This team must be defined in
			BOTS.CFG, or it will not be created.

			eg. "addteam impact", OR "addteam idt"
---------------------------------------END----------------------------------
		group	(key "G" by default)

			This tells your fellow teammates that you are starting a new
			squadron. Eventually, most teammeats will get to you, but they'll
			try and pick up useful items along the way, so they may take
			some time if far away. Once they've reached you, they'll follow you
			around, attacking enemies and picking up items along the way.

		disperse

			Tells all squadron units under your control, to immediately disperse.

	Advanced Teamplay Tactics (Squadron Intelligence)

		As in the "cmd group" described above, you (and the computer) can
		form Squadrons, to increase team performance. You can use this
		feature to guard a highly valuable area of the map, or simply to
		form a posse of destruction. The Eraser is capable of starting it's
		own group, and knows how to lead a pack, so expect to encounter
		entire bot teams roaming (and guarding) in packs. You'll also
		hear your bot teammates start a squadron from time to time, it would
		be wise to follow a bot every now and then, it's possible he knows
		something that you don't.


VIEWABLE WEAPONS								*** new to version v0.8 ***

	Eraser also supports the viewable weapons patch! Just copy the pak2.pak 
	file from your Viewable Weapons directory, to the Eraser directory, 
	then set "view_weapons 1" in the consol while playing Eraser, and restart
	the map.

	** NOTE: Viewable Weapons are only supported by the MALE, FEMALE and CYBORG
	models. To add support for other models, you will need to edit the
	[view weapons] section of bots.cfg.

	The Viewable Weapons patch can be found at..

	ftp://ftp.telefragged.com/pub/tsunami/vwep_pak.zip


CAMERA MODE										*** new to version 0.96 ***

	Thanks to Paul Jordan, Eraser now contains a snazzy camera mode, which
	lets you watch the action from a TV-like perspective. To enable
	the camera, type "cam on" in the consol.

	There are two modes to the camera:

	NORMAL: automatically moves around the level to where the action is,
	good for a demo mode.

	FOLLOW: gives you control over who the camera focuses on. Use the ATTACK
	button to cycle through players.

	To change the current mode, type "cam <mode>" in the consol, so for FOLLOW
	mode, just type "cam follow" (must use lowercase).


	** NOTE ** 
	
	Unfortunately, at this stage it is not possible to de-activate the camera,
	hopefully this will be rectified in a future version.


CONSOL VARIABLES

   The Eraser provides a range of customization commands and settings,
   which enable you to populate your server with Bots, when not
   many humans are playing. This means people are more likely
   to come to your server, since there is more chance of finding
   opponents (human or not).

   The following commands are available:

   bot_num <n>

      defines the maximum number of bots

   bot_name <name>

      spawns a specific bot

   bot_auto_skill 0/1					(default = 0)

      disable/enable automatic skill adjustment. When enabled, bot's skill
	  levels will be increased when they are killed (by a human player),
	  and decreased when they kill another human player.

   bot_chat 0/1

      disable/enable bot chatting.

   lag <n>			(CLIENT ONLY)

      adds some latency to your controls. <n> should be a number between
	  0 and 1000. This in effect, adds to your ping time.

	  eg. "cmd lag 500" gives 500ms latency

   view_weapons 0/1										*** new to v0.8 ***

      disable/enable view weapons patch. Just copy the pak2.pak from
	  your view weapons patch directory, to the Eraser directory, then
	  enable view_weapons. This enables you to see which weapons the
	  other bots/players are using.

   bot_drop <name>

      disconnects a the given bot from the game. If you have set bot_num,
	  then you can expect the program to automatically add a new bot in the
	  game. So make sure you set "bot_num 0" before using this command, if
	  you don't want another random bot to join the game.

   teamplay 0/1

      disables/enables teamplay (see TEAMPLAY section above)

   sv teams <team1> <team2> ...

      spawns one or more teams at once, by name.

   sv bots <bot1> <bot2> ...

      spawns one or more bots at once, by name.

   bot_tarzan 0/1						(default = 0)

      disable/enable so called "advanced" use of the grapple hook.

   mapmod_random 0/1					(default = 0)

      disable/enable random map progression when using maps.txt. When
	  enabled, instead of sequentially traversing the maps, a random
	  map will be selected from the list on each level change.

   botpause

      pauses the game. this is included, since the normal pause button
      doesn't work during deathmatch play.

   bot_allow_client_commands 0/1		(default = 0)

      disable/enable client-side bot spawning via "cmd bots <n>"

   bot_free_clients <n>					(default = 0)

      specifies the number of client positions to keep vacant
      at all times, whilst there are bots playing. So if
      you have set "maxclients 32", and there are 20 bots
      playing, set this value to 3, so that if the total
      number of clients (players + bots) exceeds 29, a bot will
      be kicked from the game.

      The lowest scoring bot is kicked first, in this circumstance.

      As soon as more than 3 slots become vacant, a new bot will
      be automatically brought into the game, assuming the current
      number of bots is less than the current value of bot_num.

   bot_show_connect_info 0/1

      Disables/Enables the banner that's shown to clients upon
      connecting, indicating that the server is running the
      Eraser bot patch.

   bot_calc_nodes 0/1

      Disables/Enables dynamic node calculation. If you are
      sure that this map has been played enough times, that
      it is unnecessary for the bot to continue learning the
      environment from the humans, just set this to 0. This
      frees up some CPU time, which should make things run
      a bit smoother if lots of humans are playing.


	AUTOMATIC SETTINGS

		It is now possible to save your favourite bot commands/setting in
		DEFAULTS.CFG, so each time you start the Eraser, these settings
		are loaded in.

		Be careful when editing this file, if you stuff it up, Eraser will not
		perform correctly. So make sure you study the above section carefully
		before messing with it.


NEW TO THIS VERSION!

   Check out http://impact.frag.com for the latest list of bug fixes, 
   and features added.   


FREQUENTLY ASKED QUESTIONS


   Q: When I start up the map "city3" with view_weapons enabled, the game crashes with
      "ERROR: *Index overflow". What's going on?

		This is caused by the game trying to load too many models at once. The only
		way to solve this problem, is to disable view_weapons, or avoid any levels
		that show this error.

   Q: I start the game, but when I type "bot_num X" it just repeats the command,
      like I had said it instead, what's up?

		Check your Quake2 directory. If there is a gamex86.dll file in there, 
		delete it. This solves 99% of such problems. If this is not case for
		you, then you have got the command line, or installation wrong.

   Q: What sort of actions do I need to teach them most efectively ?

		Just make sure you run around, and collect things. If you
		camp the whole game, they won't learn much at all. You only
		have to do this once, then the data is saved, so the next
		time you play the map, you won't have to worry about the learning
		at all, since it will be switched off.

   Q: Will the learning be establised from both human players or just 
      the serverside player ?

		All human players in the game create node data, when dynamic table
		generation is enabled.

   Q: When the game starts, it says "ERROR: Game is version X, not Y"?

		Eraser is only compatible with Quake2 v3.12 - v3.14, other
		versions of Quake2 may not work. To get the latest version of
		Quake2, go to "http://redwood.stomped.com/".

   Q: The game starts, but I don't see any bots?

      To spawn some bots, type "bot_num <n>", where <n> is
      a number from 1 to 24. They will then enter game at 1 second
      intervals (to try and reduce telefrags).

	  eg. bot_num 4

      You can also spawn a specific bot using "bot_name <name>",
      where <name> is the bot's name. You can get the list of bots
      from bots.cfg, which you'll find in your Eraser directory.

	  eg. bot_name cipher


   Q: I'm trying to setup an Eraser CTF server, but everytime a client connects
      they can't join the game?

		That's because the client doesn't have the CTF files installed in
		Quake2\Eraser, since that's the directory the server is running. So
		you need to do ONE of the following:

		1) rename your CTF dir to CTF_back, then rename the Eraser dir to 
		   CTF. Then start the game with quake2 +set game ctf ...

				*** OR ***

		2) Tell the clients to make an Eraser dir under Quake2, and copy
		   the pak0.pak file from their CTF dir to the Eraser dir.


   Q: The bots sometimes stand around looking bored?

      There are still some glitches in the bot decision-making that can result
      in bots getting stuck. Usually a rocket up their ass helps get them down :)


MAKING CLEAN ROUTES

   Here are a few tips to keep in mind when creating routes for new maps:

		Walk up stairs first, where possible. Otherwise a series of jump/landing
		nodes will be dropped as you go down, which uses up excess nodes, and
		is less accurate than walking nodes.

		If you jump off a ledge, try and make a path back to the jumping position 
		as soon as possible. This will prevent all other nodes from having to
		recalculate best routes later on, after the return path is made.

		The same applies for teleporters and platforms, try and get back to 
		the starting position ASAP.

		Turn on bot_debug_nodes, then enable the scoreboard. If you see "optimizing
		route xxx -> yyy" showing a whole range of numbers, then it's best to stop
		here for a bit to let it catch up. You will see this come up fairly often
		while building a new map, so not to say you should always stop when it's
		optimizing, just that if you complete a link back to a jumping locaiton
		(for example) it will need to recalculate a whole bunch of new routes.
		Stopping after the link is made for a minute or so, will prevent new routes
		from having to be calculated over and over, as surrounding nodes find better
		routes.

		Also, try to walk a nice line, rather than dodging around picking up items
		all over the place. The straighter the line you walk, the more realistic the
		bot's will move. Keep in mind that the bots will automatically pick up items
		along the way, if they are available, so you don't have to leave a trail that
		picks up items, just make sure you walk within the vacinity of all items.

		When loading a map, dynamic node placement will be disabled, IF AND ONLY IF,
		all items in the level have a node nearby, OR the total number of nodes
		exceeds 512 (the absolute limit is 750 I believe).

		Only jump when necessary.

		Type "cmd showpath" to enable the green line debugger. Just hit fire to
		set it's position to where you're standing, then move around and watch
		the path change. If it disapear's, i means the path is severed, and needs
		to be traversed to fix it up.

		If you turn on bot_debug_nodes, then restart the level, you will notice
		a list of items listed upon loading the map, unless the map is fully
		pathed. Those items listed are not currently reachable by a bot, and they
		will glow RED in the game. It would be wise to make sure bot_calc_nodes is
		ENABLED and then go around collecting all glowing items.

		Use the "bot_optimize" command to speed up the optimization process. This
		will slow down the frame rate depending on how large you set this to, but
		will often speed up the process by 10x or more.

		Try not to die.

	Alternate Paths in CTF

		It is possible to create alternate paths for bots, when returning the flag
		in CTF mode. NOTE: this only work in CTF, and is only used when a bot has the
		flag.

		To do this:
		
		1. bind a key to "flagpath", eg, 'bind q "flagpath"'.

		2. go to the enemy base, and capture the flag.

		3. press the flagpath key on the way back to base to drop a SOURCE flagpath. This
			should be dropped in a position that will always be touched by a bot on it's way back
			to base. So either use a position really close the flag (but not touching) or
			in a doorway that is close to the flag, that is the only way out of the base.

		4. determine 2 paths that can be taken from this position, dropping a destination
			node along each of these 2 routes, far enough away from the SOURCE node, to force
			the bot to take an alternate route to the shortest path.

		5. repeat steps 3 and 4 to create more "branching" flagpaths as required.
			For example, at the end of one of the paths, there may be another choice between 2 paths.
			You could create another flagpath branch, by dropping a SOURCE node near the end of the flagpath
			you just dropped, and then dropping the 2 destination nodes in positions that would
			force the bot to take alternate routes.

		6. change to the other team (if you were on the red team, type "team blue").

		7. repeat steps 3-5 as required.


		Upon touching a SOURCE node while carrying the flag, a bot will then choose a random destination
		flagpath that belongs to this SOURCE flagpath. It will then proceed to that position until
		it touches the marker. Once it has done that, it will resume heading back to base. So if you
		place the destinations too close together, it might take the same route after touching either
		destination. It's best to place the destinations as far away from the SOURCE flagpath as possible.		

		For a running demo of flagpaths being constructed, download the following file, and place it into the
		quake2\eraser\demos directory. Then while running the Eraser mod, type: "demomap flagpath.dm2".

		Get the demo at: http://impact.frag.com/files/flagpath_demo.zip


DISCLAIMER

   This is a BETA release, I therefore will not take responsibility
   for your system barfing after playing the game. I can however
   guarantee that I have not purposely added any malicious content
   to this application. If you believe this to be incorrect, then
   I'd be happy to discuss the matter with you.

   You may freely distribute this archive, as long as it remains
   PERFECTLY intact, as distributed on our home page:
   "http://impact.frag.com/". Thanks.


enjoy,

Ryan Feltrin
