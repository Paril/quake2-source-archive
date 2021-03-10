==========================================
      AWAKENING II: The Resurrection
                  v2.05
              28-April-2007
==========================================
Site   : http://awakening2.planetquake.gamespy.com/
IRC    : irc://irc.accessirc.net/awaken2

------------------------------------------
Author : Chris "Musashi" Walker
E-mail : musashi@planetquake.com
Site   : http://musashi.planetquake.gamespy.com/



===============================
GENERAL NOTES
===============================

This Quake2 mod was originally designed and created by Eric "redchurch" von Rothkirch (http://www.redchurch.com) and Patrick Martin back in 1999. The version 1 source code was not available, so I had to recode the mod from scratch (with kind permission) before I could develop it further with the new features intended for version 2.

There are four gameplay modes available:
- DeathMatch (FFA)  (gametype = 0)
- Capture-The-Flag  (gametype = 1)
- Team-Deathmatch   (gametype = 2)
- Assault           (gametype = 3)

These are played by setting the cvar 'gametype' to the required value (and then loading an appropriate map). AwakenBots can be added for FFA and Team-DM games so players can practice off-line, or server games running large maps can be padded out. See "\docs\readme_bots.txt" for more information.

NB: You must have Quake2 version 3.20 installed first in order to run this mod.


===============================
INSTALLATION & RUNNING
===============================

(1) Unzip into your Quake2 folder, using folder names (an 'awaken2' directory should be created).

(2) Copy your normal config file into this new "Quake2\awaken2\" folder.

(3) Connect to an Awakening 2 server, and you're good to go! Once in the server, you can type 'menu' in the console to make the game options menu appear.

If you want to run around a map on your own and try out the weapons and powerups, then:
(a) Run the file "awakening2.bat" in the "\awaken2" folder.
 OR
(b) Launch Quake2 and type "set game awaken2" in the console, then load a map of your choice using the command "map <bspname>"
 OR
(c) Bring up the DOS command prompt and type:
quake2.exe +set game awaken2 +deathmatch 1 +map <bspname>

To add in some AwakenBots, type "addbots <number>" in the console, or use the in-game voting menu.


There are three PAK files currently availabe:
- pak0.pak = ThreeWave CTF resources (required)
- pak1.pak = Awakening II resources (required)
- pak2.pak = Map pack #1 (optional)
- pak3.pak = Map pack #2 (optional)


===============================
IMPORTANT GAMEPLAY INFORMATION
===============================

WEAPONS:
Certain weapons have secondary firing modes - they are:

(2) Gauss Pistol
(5) Explosive Spike Gun
(7) Rocket Launcher
(8) Flamethrower
(0) Shock Rifle
(F) Anti-Gravity Manipulator

To use the secondary mode select that weapon twice. For example, if you want to use the secondary mode for the Gauss Pistol, you would press '2' to select the weapon, then once the weapon is selected, press '2' again.


MISCELLANEOUS:
It is worth binding a key to "+use", as this enables you to use the personal Teleporter powerup quickly, as well as activate the remote-control turret screens in certain CTF and Assault maps. The default bind is the ALT key.

If the server has the off-hand Grapple enabled, you will also need to bind a key to "+hook".


===============================
WEAPONS
===============================

See the file 'awaken.cfg' for the new weapon commands. Note, however, that config files with the vanilla Q2DM weapon names will still work - the mod will automatically interpret a key bind like "use blaster" as "use desert eagle", for example. (The Chainsaw, AGM and Disc Launcher will all require new keybinds, however).

(1) DESERT EAGLE:
This pistol is the initial weapon you start out with when you spawn in the game. It has fair accuracy that will kill an unarmored player with three rounds.
[Substituted for the Blaster]


(2) GAUSS PISTOL:
The primary mode is an instant-hit blaster which charges up; the damage of a shot equals the value of the charge. At full charge, a single shot will kill an unarmored opponent. This mode requires one cell per shot.

The secondary mode fires a particle beam, with a similar to a railgun shot; it uses 25 cells and 5 slugs per shot. This mode is special in that the gun can target and shoot enemies through walls, making it ideal for killing a person camping in a well defended area. Players being targeted by the secondary mode's scanner will hear a warning sound - at this point, get moving fast!
[Substituted for the Shotgun]


(3) JACKHAMMER:
A single-barreled shotgun with high rate of fire. Useful for sweeping corridors and smaller rooms clean of enemies.
[Substituted for the Super Shotgun]


(4) MAC-10:
This machinegun is powerful, but very inaccurate - especially at long range. It also uses up ammo at a furious rate. At a close enough range, it will shred an enemy in less than a second.
[Substituted for the Machinegun]


(5) EXPLOSIVE SPIKE GUN (E.S.G.):
This gun shoots spikes which will stick into players or walls and explode after a couple of seconds. When this happens, they do a lot of damage ... if a player is hit directly with a spike, they can expect to die within moments.

The primary mode fires individual spikes, with up to three being fired in fairly rapid succession before a pause to reload. For the secondary mode, the longer you hold down the fire-button, the more spikes are built up in the loading chamber. When you release the fire-button, or there are six spikes loaded, or you run out of ammo, the spikes are released simultaneously in a deadly spread.
[Substituted for the Chaingun]


(6) TRACTOR-BEAM TRAPS:
These orange spider-like devices will stick to the first surface they hit, and wait for an opponent to come within range. Once an opponent is within range, the trap will shoot out a hook at them. If this hits the player, a tractor-beam will be activated, pulling the opponent towards the trap whilst doing damage. To escape a trap, you can simply shoot one - if hit, the trap will fall apart.
[Substituted for the Grenade Launcher]


(7) ROCKET LAUNCHER:
The primary mode fires guided rockets, meaning the rockets will move to whatever point you have the crosshair aimed at. You can have multiple guided rockets in flight simultaneously, for that extra-special fiery welcome. The secondary mode fires normal rockets.


(8) FLAMETHROWER:
This weapon has a short range, but can, of course, set people on fire. Players who are on fire can only extinguish the flames by reaching health or water. Using a teleporter can also put the fire out (if enabled by the server). The secondary mode shoots a single firebomb, which explodes on impact to ignite players nearby.  Note that the flamethrower does not work underwater (!).
[Substituted for the Hyperblaster]


(9) RAILGUN:
This is the regular Q2 railgun; a familiar companion in an inconstant and ever-changing world. 

"Blue-white spiral light...
Uranium death screams out!
Your gibs fall like rain."


(0) SHOCK RIFLE:
The primary mode fires a blue ball of energy which will disintegrate an opponent on direct hit. If it is not a direct hit, the energy will freeze the opponent for a second or so. The secondary mode fires a ball of green plasma which will lock onto an opponent if they are in range. The plasma will keep homing in on them as long as they stay in range, and so can follow them around corners.
[Substituted for the BFG10K]


(F) ANTI-GRAV MANIPULATOR:
This weapon is, admittedly, a shameless rip-off of (er..."homage to") HL2's gravity gun. Firing it causes a blue seeker beam to appear in the direction you're aiming - if you hit a player with this, then the seeker beam vanishes and your target glows pale blue. At this point, as long as you keep holding down the fire button, you can move the victim around. If you quickly flick your mouse and release the fire button at the right moment, you can launch them at fatal speeds into a hard surface (or another player). You can also keep hold of them and repeatedly bash them into solid objects to inflict damage. (Note that if an object comes between your gun and the victim, the beam is broken). Finally, you can drop them to their doom, hold them underwater, and dunk them in and out of lava/slime like a doughnut :)

The AGM charge starts at 100; firing the weapon decreases this amount. The AGM recharges when you don't fire, up to a maximum of 100. If the charge reaches zero, the AGM trips off and needs to recharge right back up to 100 before it can be fired again. If you keep the charge above zero with shorter bursts, you can fire at will without it tripping off. Each press of the fire button costs a certain number of cells, and there's an additional cost every 0.1 seconds that you continue to fire the beam. Note that when manipulating a victim, the AGM doesn't discharge or recharge - it simply costs cells to keep playing with your victim.

The secondary mode fires a red beam which, rather than allowing you to manipulate someone as a whole, manipulates their internal structure instead. This causes a disruption effect at the cellular level (which armour will not protect against). In a brutal abuse of the matter manipulation technology, some of the victim's life essence is transferred to the attacker.

The AGM has a continuous mode of operation (ie. no charging required), but this is configured by the server admin rather than being selectable by the player.
[New weapon]


(G) C4 EXPLOSIVE:
These are bundles of plastic explosives which can stick to walls and be detonated subsequently at will. To detonate a C4 bundle, press G. A server-side cvar can be set to enable proximity detection on stickied C4s; players passing close by will receive an audible warning shortly before automatic detonation. Another cvar enables the C4 bundles to be sensitive to touch, so players running over them will set them off as well.
[Substituted for the Grenades]


For both C4 and Traps, you can have only five of these devices active at one time in total (although this limit may be altered by a server admin). Extra devices thrown will replace the oldest ones if the player exceeds the limit. Also, a device automatically breaks if thrown into slime or lava, or actually onto another player. The longer you hold down the firing button, the further you will throw them. Server-side timelimit cvars may be set, where if a device is held for too long it either explodes (C4) or automatically starts tractor-beaming you (Traps). In this case, a warning sound is played at the two- and one-second points before auto-detonation/trapping to warn you to throw the device quickly.


(N) DISC LAUNCHER:
This weapon fires razor-sharp spinning discs which bounce off hard surfaces. This makes them especially devastating in corridors and smaller rooms, and you can also use them to shoot around corners. A disc will last for a few seconds before automatically fragmenting.
[New weapon]


(M) CHAINSAW:
Players always have one of these beauties. Get nice and close to someone, and hold down the fire-button. 'nuff said!
[New weapon]


===============================
GRAPPLING-HOOK
===============================

This handy device is available in both standard and off-hand styles - the type in play depends on the server-side cvar setting.

For the standard grapple, you need to bind a key to "use grapple". It works as the ThreeWave grapple does - ie. you need to select the grapple as a weapon, then press and hold the fire button to launch the grapple and be pulled along by it. See the 'docs/readme_ctf.txt' file for more details.

For the off-hand grapple, you need to bind a key to "+hook". Pressing and holding down that key will launch the grapple, and pull you along once it's embedded in a surface. Releasing the key will instantly switch the grapple off. You can switch to, and use, other weapons at the same time as using the off-hand grapple.


===============================
ITEMS AND POWERUPS
===============================

THE AWAKENING (a.k.a. Siphon):
This powerup is an 'A' with a ring around it. Whenever you inflict damage on another player, you will gain that exact amount of health. It's basically health point vampirism.
[Substituted for the Quad Damage].


D89 (a.k.a. Needle):
This powerup looks like a syringe. The D89 halves all damage AFTER armor adjustments (e.g. say you have Body Armor, and are hit with an attack that does 100 damage. The Body Armor absorbs 80 damage, leaving 20 damage. The D89 halves this remaining 20 down to 10.) If you use too much D89 (that is, you pick up a D89 when you have still have more than 60 seconds remaining), you will take poison damage. The formula is based on the number of seconds of D89 the player currently has left.
     0 secs: Time = 60. No damage.
0 - 60 secs: Time = 60 + (half of current time left). No damage.
  > 60 secs: Time = no change. Player takes damage equal to time left. Armor does not absorb this damage, but the Invulnerability will.
[Substituted for the Invulnerability].


HASTE:
This powerup is a pair of wings with a ring around them. The firing rate of all weapons (apart from the AGM) is doubled. Get that Mac-10 out, and grab a Quad :)
[Substituted for the Silencer].


BEAM REFLECTOR:
This powerup is the large CD key. Not only will it completely protect you against the Gauss Pistol particle beam and the AGM beam, but it will reflect those beams back against the player that fired them (causing them damage). It also prevents homing plasma balls from the Shock Rifle from locking-on and following you.


ENVIRONMENT SUIT:
This is the standard Quake 2 environment suit. It reduces damage taken from the Flamethrower, and prevents the wearer from catching on fire. It also saves the wearer from being frozen by a nearby shockbolt impact.


PERSONAL TELEPORTER:
This powerup is the power-cube. It is a one-shot item, activated by pressing whatever key you have bound to "+use". It will teleport you to a random point according to the game type:
- For FFA and Team-DM games -> any DM spawn point
- For CTF games -> one of your team's CTF spawn points
- For Assault games -> one of your team's current spawn points

It will also affect Traps and burning players in the same way that fixed teleporters do (as specified by the 'trap_thru_tele' and 'flame_thru_tele' cvars), and will break an AGM beam if you're being held by one. Note that for CTF games, you'll drop the flag (if you're carrying it) when you activate the Teleporter.


MYSTERY BOX:
This powerup is a grey box with a red question mark on each side. They are spawned randomly near player start points. It will give you a random powerup/item from the following list:
- Invulnerability
- Quad Damage
- The Awakening
- D89
- Haste
- Beam Reflector
- Environment Suit
- Personal Teleporter


===============================
COLOR SHELL KEY
===============================

RED:
For FFA games, this means the player has an Invulnerability powerup.
For team games, it means that the player is on the red team, and has either a Quad or an Invulnerability. If they have the latter, there will be an invulnerability icon above their head.

PALE RED:
The player has a D89 powerup.

BLUE:
For FFA games, this means the player has a Quad Damage powerup.
For team games, it means that the player is on the blue team, and has either a Quad or an Invulnerability. If they have the latter, there will be an invulnerability icon above their head.

PALE BLUE:
The player is being manipulated by someone with the AGM.

GREEN:
The player has an active Power Shield (only flashes when hit).

PALE GREEN:
The player has a Beam Reflector powerup.

MAGENTA:
The player has an Awakening powerup.

YELLOW:
The player has a Haste powerup.

PALE CYAN:
The player is temporarily frozen in place (by a near miss from a shockbolt). Have your wicked way with them...

WHITE:
The player has Godmode on.


===============================
NEW KEY BINDS
===============================

"use AG Manipulator"
Activates the AGM; toggles between the Manipulation and Cellular Disruptor modes if the weapon is already selected.

"use Chainsaw"
Activates the Chainsaw.

"use Disc Launcher"
Activates the Disc Launcher.

"+push"
Moves your AGM victim away from you.

"+pull"
Brings your AGM victim closer to you.

"fling"
Throws your AGM victim at a high speed away from you.

"use Grapple"
Activates the standard (non-offhand) Grappling Hook.

"+hook"
Fire your offhand Grapple.

"+use"
(1) If you're beside a control terminal and looking at it, pressing your +use key will activate the terminal, allowing you to control the turret(s) remotely. Mouselooking and firing work as normal. The strafe keys will switch between other turrets in the control chain (if any are linked together).

(2) Otherwise, if you have a Personal teleporter powerup, the +use key will instantly activate it.


===============================
NEW CONSOLE COMMANDS
===============================

NB: [] = optional command parameters
    <> = specified name or number


"menu"
Display the in-game menu.

"observer"
Leave the current game and become a spectator (your score will be zeroed).

"vote [map <bspname>]"
The 'vote' command on its own brings up the in-game voting menu. Otherwise, you can initiate a vote to switch to the specified map (must be in the server's map list).

"yes"
Agree with the currently proposed vote.

"no"
Disagree with the currently proposed vote.

"ready"
Set your state as being ready for the current match.

"notready"
Removes your readiness state for the current match.

"team [red / blue]"
Switch to the other team (in team games only, of course).

"id"
Toggle player identification on/off (default = off). This is for team-games only. When you look at a player, their name and team icon appear in your HUD.

"idtrap"
Toggle Trap and C4 identification on/off (default = on). This is for FFA games only. When you look at one of your own Traps or C4s, a "don't shoot" icon appears in your HUD.

"weap_note <mode number>"
Sets your preference for the type of weapon mode notification that occurs when you select a new weapon, or toggle the secondary mode of your current one:
  0: no voice sounds or text
  1: text only, for weapons with secondary modes (default)
  2: text, plus voice for weapons with secondary modes
  3: no text; voice for weapons with secondary modes
  4: no text; voice for all weapons
  5: text, plus voice for all weapons

"play_voice <wav>"
Play the specified sound file at your current location, so that any nearby players can hear it (normal attenuation).

"play_team <wav>"
Play the specified sound file to all your team-mates (only they will hear it).

"ghost <code>"
Re-enter the game after losing your connection in a match. This requires the ghost code that was provided when the match started.

"stats"
Show the statistics of any matches recently completed, or in progress.

"mission"
Display the mission objectives for the current map in an Assault game.

"playerlist"
Display number, name, connect time, ping, score, and Operator status for all players.

"maplist"
Display the list of maps for the server's map cycle (these can be requested for a map change vote).

"motd"
Display the server's Message Of the Day.

"addbots <number>"
Add in the specified number of AwakenBots. Note that this command only works when you run the mod on your local machine, rather than connecting to a dedicated server.

"clearbots [<number>]"
Remove the specified number of AwakenBots (or all of them if no number is given). This command is also valid only on a local machine.


===============================
NOTES TO SERVER ADMINS
===============================

(1) See the 'docs/readme_cvars.txt' file for the complete list of the mod-specific console variables, what they all mean, and their default values. A default config file without the extra text is provided -- 'docs/cvars.cfg'. It is recommended that you don't include every cvar setting in your own configs, as they'll exceed the 8KB limit. Instead, just use those that you change from their default values.


(2) The cvar 'map_file' specifies the file that contains the server's map list. The default is "mapcfg/maps.txt" - an example one has been provided. You can have as many map files as you need, and they can be as large as you need (well, to the limited of your HD space). The game will work through the map list in order, then go back to the start once it reaches the end.

If you manually load a map, then once it has been played, the current map_file will be scanned for that bsp name. If it finds it, the game will load the next level in the list. If it doesn't find it, the game will load the first level in the list. If the bsp name itself is incorrect (ie. the file doesn't exist), then the current level will be replayed. and a warning message displayed to the players.

The maplist file also determines which maps players can call a vote for - if it's not in the list, it can't be selected (even if the server has the map). Operators will be bound by this restriction too, unless the 'op_ignore_maplist' cvar is set.


(3) The cvar 'config_file' specifies the file that contains the server's config file list. The default is "configs.txt". This list determines which configs/setups players can call a vote for.


(4) The cvar 'motd_file' specifies the file that is used as the server's Message Of The Day. The default is "motd/motd.txt" - an example one has been provided. Note that the maximum line width is 27 characters, and the maximum number of lines is 15.


(5) Players can be granted Operator status, which allows them to perform certain server admin actions without needing to know the rcon password. See the list of Operator console commands for more information. In order to enable the use of Operators on your server, you need to set the 'op_password' cvar (it is blank by default). It is recommended that you only let trusted players know this password.


(6) A number of the available client spaces on the server (out of a total of 'maxclients') can be marked as reserved slots, using the 'reserved' cvar. These will be held free whilst the remaining (public) slots are filled up. If the 'rsv_password' cvar is set, any player who sets their 'password' string to the same value will connect into a reserved slot. Note that if the standard server 'password' cvar is set, all slots are effectively considered reserved.


(7) Since the AGM is a new weapon, it isn't available on maps that haven't been built with the relevant entity. However, AGMs can still be placed manually in maps at run-time via the AGM drop list feature, specified by the 'agm_drop_file' cvar (the default is "mapcfg/agm.txt"). Each line of this file should be in the following format:

<bspname> x y z 

where <bspname> = q2dm1, etc

If the current map name is listed in the file, an AGM will be dropped at the specified coordinates. Multiple drop points may be specified; they don't need to be consecutive. Finally, the 'agm_drop' cvar must be set to 1 to tell the server to use the drop file.


(8) Similar to point (7), applied to the Disc Launcher (which is a new weapon too). The cvar 'disc_drop_file' points to "mapcfg/disc.txt" by default, and will be used by the server if the cvar 'disc_drop' is set to 1.


===============================
NEW SERVER COMMANDS
===============================

"sv add_op <playernum>"
Grant Operator status to the specified player. Note: use the client command 'playerlist' to obtain the list of player names and numbers. 

"sv del_op <playernum>"
Remove Operator status from the specified player.

"sv addbots <number>"
Add in the specified number of AwakenBots.

"sv clearbots [<number>]"
Remove the specified number of AwakenBots (or all of them if no number is given).

"sv dev_edicts"
Print out a list of information about the current entries in the global edicts array.


===============================
CONSOLE COMMANDS FOR OPERATORS
===============================

"op <op_password>"
Request Operator status from the server. Note that the password is case-sensitive.

"op_menu"
Display the in-game Operator menu.

"op_kick <playernum> [<reason>]"
Force the specified player to disconnect fron the server. If you want to give a reason, enclose it in double quotes (eg. op_kick 1 "you're a llama"). The number of a particular player can be found using the 'playerlist' command.

"op_ban <playernum> [<reason>]"
Force the specified player to disconnect fron the server, and add them to the list of banned IPs. This creates or appends to the file "listip.cfg", which can be edited off-line as required.

"op_mute <playernum> [<reason>]"
The specified player can no longer use the 'say', 'play_voice' or 'play_team' commands.

"op_unmute <playernum> [<reason>]"
Remove the mute restriction on the specified player.

"op_swapteam <playernum>"
Force the specified player to swap to the other team.

"op_iplist"
Display the number, name, IP address and Operator status for all players.

"op_vote [map <bspname>]"
The 'op_vote' command on its own brings up the in-game Operator voting menu (any vote initiated by an Operator is automatically passed). Otherwise, you can initiate a vote to switch to the specified map.

"op_yes"
Force the currently proposed vote to be passed.

"op_no"
Force the currently proposed vote to be dropped.

"op_start"
Force the current match to start (assuming it's in the pre-game phase). All players currently assigned to teams are forced into the ready state; those not on a team are forced into observer mode.

"op_slots"
Display a list of which public and reserved player slots are taken, and by whom.

"op_dropagm [x y z]"
The 'op_dropagm' command on its own writes the map name and player's current coordinates to the server's AGM drop points file. Alternatively, coordinates can be specified manually.

"op_dropdisc [x y z]"
The 'op_dropdisc' command on its own writes the map name and player's current coordinates to the server's Disc Launcher drop points file. Alternatively, coordinates can be specified manually. 

"op_addbots <number>"
Add in the specified number of AwakenBots.

"op_clearbots [<number>]"
Remove the specified number of AwakenBots (or all of them if no number is given).


===============================
NEW DEATHMATCH SERVER FLAGS
===============================

DF_FAST_SWITCH = 65536
Allows fast switching between weapons.

DF_EXTRA_ITEMS = 131072
Random powerups are spawned at the DM start points. These are: one Adrenaline, one Backpack, and one or more Mystery Boxes.

DF_NO_REPLACEMENTS = 262144
Use the normal Quake2 powerups instead of substituting them with the Awakening ones.

DF_FORCEJOIN = 524288
For team games, players are automatically assigned to a team. For FFA games, players are placed into the game automatically upon connection (rather than starting as spectators).

DF_ARMOR_PROTECT = 1048576
Attacks do not harm team-mates' armor (in team games).

DF_CTF_NO_TECH = 2097152
The Tech powerups are not spawned during CTF games.

DF_CTF_SPAWNS_ONLY = 4194304
Players only spawn at info_player_team points during CTF games.


Note that with the new addition of the team skin and model cvars, the DF_SKINTEAMS and DF_MODELTEAMS dmflags are now obsolete. If these dmflags are set, they will be ignored (and a warning message will be displayed in the server console).

The DF_NO_FRIENDLY_FIRE dmflag is not used either; it is effectively always on. This means that team mates cannot harm each other, and weapons such as Traps and homing plasma will not target players who are on the same team as their owner.


===============================
THANKS
===============================

Huge thanks first of all, of course, to id Software for making this all possible in the first place by making these fun and wonderfully mod-able games.

I'd like to give many thanks to Redchurch for kindly giving his permission to recode this mod and distribute the original art content. Kudos to him and the rest of the original Awakening team (see 'credits_v1.txt') for all their efforts in creating such an addictive mod in the first place - you guys rock! (see the end of this file for contact details).

Many thanks to Zoid and the ThreeWave CTF team, whose source code was used as the baseline for this mod. Thanks also to Zoid for his kind permission to redistribute the CTF resources with this mod. Please visit http://www.threewave.com for more information about CTF for all three Quake games.

This mod's existence owes a great deal to the beta-testing team, who have put in a lot of effort to make sure that it plays well and runs smoothly. They have contributed many good ideas (and witty conversation ;) along the way, and were responsible for introducing me to the original mod in the first place. Big sloppy kisses to Sn4xx0r, Maric, Wizkid, Sherminator, CyberGhost, Quiet, Bass, Wild, Mistery, Death, Ledhead, Fatty and Flash for all their help.

Thanks to Maric for all his time and effort in putting together a map-pack of superbly enjoyable levels - especially the Assault maps which were made specifically for this mod. Additionally, he put in a lot of time and effort to make the AwakenBot route files. Thanks also to Sherminator and Panzer for their cool map contributions. See http://www.backshooters.com for plenty more quality levels.

Thanks to Sn4xx0r for all the time spent doing Linux compiles, server admin work, voice recording, shrubbery-planting and bug-fixing help. *Nobody* expects the Source Reviewers!

Thanks to sonNeh (sonneh@leksanized.com) for his awesome work on putting together the Awakening II website. He offers web design services over at http://www.leksanized.com/

Thanks to David Hyde for his source code for the turret, model_spawn and func_monitor entities, from the totally excellent Lazarus mod (http://planetquake.com/lazarus).

Thanks to Doug "Raven" Buckley for the beefed-up chase-cam code (theraven@planetquake.com).

Thanks to Ponpoko and Maj.Bitch for the excellent 3ZBII / BitchBot code - without the work of these guys, there would be no AwakenBots. Also, thanks to Fatty for giving me the Bitchbot source code and tutorials that allowed me to add the bots.

Thanks to r1ch (http://www.r1ch.net/) for giving the code a good going-over, and helping to fix a bunch of exploits and vulnerabilities.

Thanks to Lee David Ash for his cool weapon model and skin (used for the AGM) - you can find more of these at http://www.planetquake.com/violationE.

Thanks to Paolo "Nusco" Perrotta and Roberto "Taz" Bettazzoni for the spangly Halo model from their Q2 Holywars mod (http://www.planetquake.com/holywars).

Thanks to Awesund for the MG turret sound. Go visit http://www.parodycentral.net/ to download more of his great sound effects.

Thanks to Pan for a couple of sounds (Gauss Pistol scanner warning, and the AGM stream crossing). Check out this guy's great maps at http://home.wanadoo.nl/realgamer

Thanks to aXon for the misty ocean skybox (http://axon.drunkencoon.com).

Thanks to Steffen Haeuser for the compiling the AmigaOS 4 and LinuxPPC versions fo the game binaries.

Last but certainly not least, a staggeringly big thank you to my wife for indulging me in my mega coding spree - you're the best :)


===============================
DISCLAIMER
===============================

There is no warranty for this software. It may well cause your computer to explode, melt, vapourise or otherwise molest you in some unspeakably sordid fashion. You use it entirely at your own risk.

Quake 2 is copyright by id Software.

--------------------------------------------------------

Still reading? Have an Arcturan Mega-Cookie!
