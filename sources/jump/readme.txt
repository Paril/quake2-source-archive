What is it?
Jump is a mod for Quake II where instead of plain deathmatch and killing, the idea is to jump around various jump maps in the fastest possible time. There are all sorts of different jumps you need to pull off, from double and triple jumps to circle and ladder jumps. It's annoyingly addictive too.
NOTE: This is NOT the same mod as the jolt "JumpMod" which many servers run. This version is extremely simple and has no weapons, no web based scoring setup, etc, it's just plain jumping with a high score list. If you want the jolt JumpMod, please visit www.q2jump.jolt.co.uk

Where can I get it?
The mod basically consists of the server DLL (which prevents players from walking into each other and weapons firing, etc) and the jump maps. You can get the jump map pack which contains a whole bunch of jump maps (over 150). This map pack contains all the maps from Azbok's and jolt, with the Deathmatch and "stupid" maps removed. Still, there are new maps coming out that won't be in the pack yet, so always set auto downloading (set allow_download 1, set allow_download_maps 1) on before playing.
These archives both contain the same maps, however the .bz2 contains NO textures, sounds or environment maps and is intended for dedicated servers only.
jump map pack .zip (29.7mb, all maps, textures, sounds and environment files)
jump map pack .tar.bz2 (22.17mb, .BSP files only for dedicated servers, untar in your /jump/maps/ directory)

You can grab a copy of the server DLL from May 3rd too, but it is recommended you visit the IRC channel for latest updates.
gamex86.dll.zip for Windows, 88.2kb. Extract to your quake2/jump directory.
gamei386.so.gz for linux, 60kb. Note, due to a bug in the iD Software Quake II 3.20 for linux, you should rebuild your q2 server from source. Failure to do so will cause a crash when a player joins.

You can grab the source code if you wish. By downloading, you agree to the terms of the Open Source Mod License under which this code is placed.

Servers
At the moment there is one server running my version of the jump mod. You can find it at 66.162.58.41:27914. Many thanks to OCLD.COM for hosting this. If you would like to run your own server, be sure to find me at irc.edgeirc.net #jump and I'll send you the latest DLL.

=========
Commands
=========

-There are various client commands you can use...

kill
Immediately respawns you at the start of the map (or last checkpoint) and resets your timer.

checkpoint
Set a checkpoint at your current location. When you die you will respawn there instead of at the 
start. You are limited to usually two checkpoints per map, and using a checkpoint disqualifies you 
from time scoring.

killcp
Erases all your checkpoints and sends you back to the start, as if you'd just connected.

votemap
Vote to change the map. Eg, votemap atr_kjump33 would start a vote to change the map to atr_kjump33. 
Other players then have 30 seconds to either vote by typing YES or NO in the console.

================
Server Commands
================

-Note, when running a server you'll probably want to create a directory /quake2/jump/scores/. 
The record scores for each map are stored there in binary format. Without the directory, no 
records will be kept. There are a few server cvars servers can use to customize various things:

sv_maxcheckpoints
Sets the maximum number of checkpoints allowed to be deployed per client. Default 2.

sv_fpscap
Limit the maximum cl_maxfps a client can use. Default 0, meaning no limit.

sv_voting
Enable or disable map voting. Default 1, meaning enabled. Since there is no good map rotation 
code as of yet, it is recommended to leave this at 1 to allow new maps to be played.