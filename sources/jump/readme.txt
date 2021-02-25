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