Happy Fun Hook -- Quake II Deathmatch Mod
Quake II Version 3.20

===========================================================

Please report all bugs or suggestions to the programmer,

	Chris Garrett AKA JeChOE
        garrerc@planetquake.com

Visit us at http://www.planetquake.com/funhook

===========================================================

By Spastically Happy Productions (JeChOE)

Original Code By:
   Perecli Manole - Bort

Testers:
   Brian - Lego Whore
   Chris - JeChOE
   Goo   - Dynamic Cow
   Jon   - Clown Friend
   Joe   - kenny
   Russ  - Dynamic Monkey
   
Special Thanks To:
   Perecli Manole - Bort


Last Edit: 05/07/99

Point Release Version: 2.18

===========================================================
INTRODUCTION:
===========================================================

Happy Fun Hook is a modification on the Swinging Grappling
Hook by Perecli Manole (Bort).  Meant for multiplayer
deathmatch and co-op games (though co-op support is not 
fully implemented).

Features include:
   * Original functionality as a swinging grappling hook
   * Ability to hook other players
   * Very configurable with command line options
   * Config files included
   * Excellent physics


===========================================================
INSTALLATION:
===========================================================

Solaris:
--------
   Decompress and untar the file to your Quake II
   directory.  The result should be:

	<Quake II dir>/funhook/gamesparc.so
	                      /hook.cfg
	                      /hookfun.cfg
                              /hookorig.cfg
	                      /README

   Then run Quake II with +set game funhook +exec hook.cfg.

Windows NT/95/98
----------------
   Create a funhook directory under your Quake II
   directory.  Unzip the downloaded file to this directory.
   The result should be:

	<Quake II dir>\funhook\gamex86.dll
	                      \Pak0.pak
	                      \Pak1.pak
	                      \hook.cfg
	                      \hookfun.cfg
                              \hookorig.cfg
	                      \README.TXT

   Then run Quake II like so:

	<Quake II dir>\quake2 +set game funhook +exec hook.cfg

   or from the Quake II console (available by hitting the ~
   key) by typing:

	set game funhook
	exec hook.cfg


===========================================================
USE:
===========================================================

You need to bind keys to the following functions:

   +hook               - fires the hook
   +shrink             - shinks the chain of the hook
   +grow               - grows the chain of the hook
   laser               - places a laser trip wire (25 Cells)
   laserbomb           - throws a laser bomb ;) (100 Cells)
   decoy               - places a decoy of yourself
   use sticky grenades - uses the sticky grenades

===========================================================
CONFIGURATION:
===========================================================

Variable           Default  Description             
-----------------------------------------------------------
hook_fire_speed       2000  hook firing speed       
hook_pull_speed         80  hook pulling speed      
hook_sky                 1  hook can attach to sky       
hook_players             1  hook can attach to players   
hook_is_homing           1  hook will home in on players 
hook_homing_radius     200  home within this radius 
hook_homing_factor       5  rate of homing - ratio of new
                            trajectory / current trajectory
hook_min_length         20  minimum length of hook  
hook_max_length       2000  maximum length of hook  
hook_messages            1  hook-related messages
hook_vampirism           0  vampirism rate of how much 
                            health is taken (pts/sec)
hook_vampire_ratio     0.5  vampiric health ratio of
                            given / to taken

There are two different configurations that can be quickly
set using the config files included.  The Happy Fun Hook
configuration can be set by typing hook_fun at the console
and the original Swinging Grappling Hook behavior can be
set with hook_original.  Finally, you can see the values
of the hook-related variables with hook_params.


Enjoy!
