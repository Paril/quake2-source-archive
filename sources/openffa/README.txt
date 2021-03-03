OpenFFA
=======

OpenFFA is a simple free-for-all deathmatch mod for Quake 2, an open source replacement
for ‘RegularDM’ mode of OSP Tourney. Currently in development, it features
familiar scoreboard and HUD, voting system, configurable map rotation support,
high scores logging, ‘proper’ in-eyes chasecam support. Besides that, OpenFFA fixes
some gameplay bugs and annoyances original Quake 2 game library has.

Client commands
---------------

OpenFFA supports a number of custom client commands, some of them are given below.

* menu — Show OpenFFA menu
* join — Enter the game
* observe — Leave the game
* chase — Enter chasecam mode
* settings — Show match settings
* oldscore — Show previous scoreboard
* vote — Propose new settings
* stats — Show accuracy stats
* players — Show players on server
* highscores — Show the best results on map
* id — Toggle player ID display

Download game library
---------------------

If you wish to try OpenFFA on your R1Q2 or Q2PRO server, you can start by downloading
a [nightly][1] build for your system. Binary builds for Win32 (x86) and GNU/Linux
(i386 and amd64) are provided, along with source snapshots.

[1]: http://skuller.net/openffa/nightly/

Linux packages
--------------

In addition to the manual installation method above, it is also possible to
install OpenFFA pre-packaged for your Linux distribution, ready to use together
with Q2PRO server.

Debian GNU/Linux users can install OpenFFA from the skuller.net APT [repository][2].
Ubuntu users can install OpenFFA from a [PPA][3] at launchpad.net.

[2]: http://skuller.net/q2pro/download/apt/
[3]: https://launchpad.net/~skuller/+archive/q2pro

SVN access
----------

Anonymous read-only access to OpenFFA SVN repository is available.
Use the following commands to checkout a fresh working copy and build it:

    $ svn co svn://skuller.net/openffa/trunk openffa
    $ cd openffa
    $ svnversion > REVISION
    $ make

You can also browse OpenFFA source code [online][4].

[4]: http://skuller.net/openffa/cvstrac/dir?d=trunk

Bug tracker
-----------

For reporting bugs and proposing enhancements to OpenFFA, please use
the [bug tracker][5]. No registration (a trivial process though) is required.

[5]: http://skuller.net/openffa/cvstrac

Server configuration
--------------------

Some custom OpenFFA cvars are described below.

- `set g_idle_time 0` (integer)  
Time, in seconds, after which inactive players are automatically put into
spectator mode. Default value of zero disables this feature.

- `set g_maps_random 2` (integer)  
Specifies whether map list is traversed in random on sequental order.
   - 0 - sequental order
   - 1 - random order
   - 2 - random order, never allows the same map to be picked twice in a row

- `set g_maps_file ""` (string)  
Specifies name of the file to load map list from. Should not include any
extenstion part or slashes.

- `set g_defaults_file ""` (string)  
If this variable is not empty and there are some settings modified by voting,
server will execute the specified config file after 5 minutes pass without
any active players. Config file should reset all votable variables
to their default values.

- `set g_skins_file ""` (string)  
Specifies name of the file to load skin list from. Should not include any
extenstion part or slashes. If skin list is specified, players may only use
skins defined in the list.

- `set g_bugs 0` (integer)  
Specifies whether some known Quake 2 gameplay bugs are enabled or not.
   - 0 - all bugs are fixed
   - 1 - ‘serious’ bugs are fixed
   - 2 - original Quake 2 behaviour

- `set g_teleporter_nofreeze 0` (boolean)  
Enables ‘no freeze’ (aka ‘q3’) teleporter behaviour.

- `set g_spawn_mode 1` (integer)  
Specifies deathmatch spawn point selection mode.
   - 0 - select random spawn point, avoiding two closest (bugged version)
   - 1 - select random spawn point, avoiding two closest (fixed version)
   - 2 - select random spawn point

- `set g_item_ban 0` (bitmask)  
Allows one to remove certain items from the map.
   - 1 - quad damage
   - 2 - invulnerability
   - 4 - BFG10K
   - 8 - power armor (screen and shield items)

- `set g_vote_mask 0` (bitmask)  
Specifies what proposals are available for voting.
   - 1 - change time limit
   - 2 - change frag limit
   - 4 - change item bans
   - 8 - kick a player
   - 16 - mute a player
   - 32 - change current map
   - 64 - toggle weapon stay
   - 128 - toggle respawn protection (between 0 and 1.5 sec)
   - 256 - change teleporter mode

- `set g_vote_time 60` (integer)  
Time, in seconds, after which undecided vote times out.

- `set g_vote_treshold 50` (integer)  
Vote passes or fails when percentage of players who voted
either ‘yes’ or ‘no’ becomes greater than this value.

- `set g_vote_limit 3` (integer)  
Maximum number of votes each player can initiate.
Value of zero disables this limit.

- `set g_vote_flags 11` (bitmask)  
Specifies misc voting parameters.
    - 1 - each player's decision is globally announced as they vote
    - 2 - current vote status is visible in the left corner of the screen
    - 4 - spectators are also allowed to vote
    - 8 - players are allowed to change their votes

- `set g_intermission_time 10` (float)  
Time, in seconds, for the final scoreboard and high scores to be visible
before automatically changing to the next map.

- `set g_admin_password ""` (string)  
If not empty, clients can execute `admin <password>` command to
become server admins. Right now this gives them a decider voice in votes,
ability to see IP addresses in the output of ‘playerlist’ command and grants
access to a number of privileged commands (listed in `acommands` command output).

- `set g_team_chat 0` (boolean)  
Specifies if ‘say_team’ messages from players are visible to others.
This setting does not affect ‘say_team’ messages from spectators.
   - 0 - visible only to players themselves
   - 1 - visible to every player in game

- `set g_mute_chat 0` (integer)  
Allows one to globally disallow chat during the match (chat is still allowed
during the intermission).
   - 0 - chat is enabled for everyone
   - 1 - player chat is disabled, spectators are forced to use ‘say_team’
   - 2 - chat is disabled for everyone

- `set g_protection_time 0` (float)  
Time, in seconds, for newly respawned players to be invincible.

Map list format
---------------

Map list is loaded at server startup from  `./openffa/mapcfg/${g_maps_file}.txt`.
Each line of the map list file should have the following format:
`<mapname> [min_players] [max_players] [flags]`. Minimum and maximum player
counts are optional, as well as flags. Flags can be either 1 or 2, which makes
the map entry not automatically selectable and not votable, respectively.

Skin list format
---------------

Skin list is loaded at server startup from  `./openffa/${g_skins_file}.txt`.
Each line of the skin list file should specify either a model name, or a skin name.
It should begin with a model (directory) name, optionally followed by skin names
allowed for that model. Model names are distinguished from skin names by the presence of
trailing slash, e.g. `male/` and `female/` are models, `grunt` and `anthena` are skins.
There may be several model names defined in the file. If there are no skins defined
for the model, then any skin can be used for that model. When player skins are
validated, if there is no matching skin found, then the last one defined for the matching
model is used. Likewise, if there is no matching model found, then the last one defined
in the file is used.



