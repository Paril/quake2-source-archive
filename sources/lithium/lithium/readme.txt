
                =======================================
                   Lithium II Mod v1.31 for Quake II
                =======================================

By Matt "WhiteFang" Ayres, matt@lithium.com
Available at http://quake2.lithium.com

Lithium II is a very configurable server side deathmatch modification
for Quake II v3.20.  Clients do not need to download anything for DM
mode.  Lithium II CTF mode requires clients to have ThreeWave CTF II.

Main Features
----------------------------------------------------------------------------
   * All features are toggleable and configurable.  Use the ones you want.
   * Menu system for in-game remote administration and client settings.
   * Runes; strength, resist, regen, haste, and vampire.
   * Improved HUD with frags, frags/hr, players, your place, current rune.
   * Optional HUD that displays ammo counts.
   * Damage, speed, and radius adjustable for all weapons.
   * Obituaries with your enemy highlighted in green text.
   * Grappling hook, offhand (red laser style).
   * Faster respawning as more players join.
   * Configurable map queue with randomize option.
   * Customizable MOTD.
   * Observer fly-thru mode.
   * Chasecam to follow other players around (cam is client adjustable).
   * Player ID, to identify who you're looking at.
   * Safety time period for player spawns.
   * No camp option kills campers (default off).
   * Options to kick chat and kill flooders.
   * IP banning, also min and max ping requirements.
   * Adjust start/max values for health, ammos, and armor.  Also setup which
     weapons clients will start with.
   * Server configuration and toggles implemented as cvars.  This allows
     .cfg files for startup and instant changes on the server console.
   * Based on v3.20 game source.
   * Compiled for Windows 32-bit, Linux 32-bit, and Linux 64-bit.
   * VWep support, see which weapon your enemy is using!  (requires a pak).
   * GSLog frag logging (Gibstats Log Standard).
   * Fast weapon switching.
   * Admin list feature with access levels and passwords.
   * Option for Pro-Rocket specs (via procket.cfg).
   * Map voting system.
   * Overflow protection.
   * Integrated ThreeWave CTF II.
   * High scores feature.
   * Zbot (auto-aim cheat) detection to kick and log them.
   * More...


Quick Start (DM)
----------------------------------------------------------------------------
Create a "lithium" directory under your Quake II directory, and extract the
archive there.  From a command prompt, change to your Quake II directory.
To run a dedicated server, type this:

   quake2 +set dedicated 1 +set game lithium

If you want to play on the machine that will host the server, use this:

   quake2 +set deathmatch 1 +set game lithium +map base1

When running a non-dedicated server, you must specify a map or a config
file (that contains a map specification at the end).


Quick Start (CTF)
----------------------------------------------------------------------------
First install ThreeWave CTF II.  Extract the Lithium II archive into the
"ctf" directory under your Quake II directory.  From a command prompt,
change to your Quake II directory.  To run a dedicated server, type this:

   quake2 +set dedicated 1 +set game ctf +set ctf 1


Configuration
----------------------------------------------------------------------------
To configure things to your liking, it is recommended that you pick one of
the following files:

   lithium.cfg  - settings for a default Lithium II server
   stock.cfg    - settings for a stock Quake II server
   lithctf.cfg  - settings for a default Lithium II CTF server
   stockctf.cfg - settings for a stock CTF II server

And copy it to server.cfg.  Edit server.cfg as you please, and include
"+exec server.cfg" (without quotes) when starting your server from the
command prompt.

See config.txt for a list of which each variable in the config does.  When
installing over an old version of Lithium II, it is recommended that you
use a fresh server.cfg (as the configs change some between versions).

Any of the variables you see in the .cfg files can be entered directly to
the server console.  The change will take effect immediately for most
settings, some may not take effect until the next map.

Adjust the maxclients value accordingly in server.cfg.  You can modify
the MOTD to your liking by editing the motd.txt in the lithium directory
(or ctf directory for CTF mode).

If you don't want your server listed publicly, set public 0 in your config.


Administration
----------------------------------------------------------------------------
If you'd like to use the in-game remote administration feature, you'll
need to set the admin_code variable in lithium.cfg.  The default is 0,
which disables the feature.

Alternatively you can setup an admin.lst specifying specific admins, each
with different access levels and passwords.  A bit more complex, see
admin.lst for more info.

In the game, to access the admin menu, simply type "admin" on the console.
You will be prompted for the admin_code.

Please note that numbers you type in the admin menus are detected by key
bindings.  If you've changed the binds for your number keys, you'll have a
problem.  Unfortunately this is the only way to detect those keys being
pressed in the game .dll code.


Playing
----------------------------------------------------------------------------
Clients require no special files to play on Lithium servers.  There is
nothing they need to download.  Simply connect and play.  That's the
beauty of server side mods.

Client commands:
   drop rune           Drop your current rune
   +scores             Quake 1 style scoreboard
   +hook               Grappling hook
   observe             Observer mode
   chase               Chasecam mode
   chaseleader         Always chase first place
   chaselist           List the clients chasing you
   highscores          List high scores for current map
   bestweap            Toggle best weapon pickup
   id                  Toggle player identity
   hud                 Toggle Lithium/Ammo HUD
   menu                Lithium II main menu
   admin               Admin menu
   vote map <mapname>  Start map voting process for <mapname>

It's recommended that you bind keys to "drop rune", +scores, and +hook.
For example, you'd type something like this on the console (or add to
your autoexec.cfg in your quake2\lithium directory):

   bind e "drop rune"
   bind tab +scores
   bind q +hook
   bind v +vote
   bind m +menu
   bind z +admin

Runes:
   Strength (Red)     You inflict 2x damage
   Resist   (Blue)    You take 1/2 damage
   Haste    (Yellow)  You fire twice as fast
   Regen    (Green)   Your health regenerates slowly
   Vampire  (Purple)  You gain health for damage inflicted


Feedback
----------------------------------------------------------------------------
I appreciate feedback, bug reports, new ideas, and comments (good or bad).
Send them to matt@lithium.com.  Please understand you may not receive a
reply, however do know that it will be read.
