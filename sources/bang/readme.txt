BANG 1.3 Readme - http://www.r1ch.net/q2/bang/
==============================================

Hi,
Thanks for downloading BANG 1.3! To play BANG, you will need the following:
o Quake2 version 3.20 (or compatible)
o 10mb of free hard disk space on your Quake2 drive

What's new in 1.3?
==================
Previous to 1.3 I didn't really understand much about models and skin
linking etc, and as such each grenade launcher and model had its own
model with its own skin. This new version uses a single model with
multiple skins, which should be more network and CPU friendly for
everyone. In addition, Saig has made a new map, symic.bsp to satisfy
your explosive desires. A few other small changes - nukes now spawn
at random intervals (ie if a map has 10 nukes on it, they will slowly
spawn throughout the  game, but once used they don't respawn). This
should help those nuke-fests at the start of the map :). Infests have
been tweaked a little, infested dead bodies spew gibs, complete with
3 extra squishy sounds (thanks to Saig again). Single player and coop
support is vastly improved (monsters use grenades properly, infest from
monsters work, monsters drop ammo and more). A potential crash bug
with bio grenades was also fixed.

Grenades only?
==============
Yup. BANG is pure grenade deathmatch insanity. It might sound boring
at first, but once you try it you'll be addicted :). There are a whole
ton of different types of grenade - in order to make updating easier,
these are listed on the BANG website at http://www.r1ch.net/q2/bang/.
You are most likely to find me playing on 216.37.2.25:27915 when it is
up. If you can't find anywhere to play, pop by #gloom on irc.edgeirc.net
as many of the players from gloom also play BANG, and if we're in the
mood I might throw up a server.

Running the mod
===============
BANG 1.3 is a multiplayer deathmatch mod for Quake 2. While intended
for multiplayer only, version 1.3 adds significant changes to single
player such as fixing of crashes, monsters dropping ammo, infestations
and other grenades work properly, etc and as such, BANG 1.3 is quite
playable in single player or coop mode if you so desire.

There are a couple of things you can do:

- Play on an Internet server
- Play on a local network (LAN)

To play on the Internet, you need to find a BANG 1.3 server. This is
best achieved by getting GameSpy 3D (www.gamespy.com) and following
the instructions there. When you get a list of all Quake 2 servers,
press the 'game' tab to sort by the mod they are running, and scroll
down and look for BANG and double-click the server to join the chaos.

You can also run a server yourself for other people to join, but unless
you have a lot of bandwidth (eg T1, SDSL, etc), it will be probably too
laggy for other people to have fun on. If you still want to try, there are
two options available: a DEDICATED server or a LISTEN server. The difference
between the two is you can play on a LISTEN server, but a DEDICATED server
just sits there and you can't play. DEDICATED servers are usually
preferred for long-term servers, and LISTEN servers for quick games.

To run a DEDICATED server:
1. Open a command prompt (DOS prompt, CMD, etc)
2. Go to your Quake 2 directory (eg CD C:\QUAKE2\)
3. Enter the following command (all on one line):
   quake2 +set dedicated 1 +set hostname "BANG 1.3 Server" +set maxclients 16 +set public 1 +set deathmatch 1 +map q2dm1
   ...and press ENTER.
4. The server should now be running.

To run a LISTEN server:
1. Start Quake 2
2. Bring down the console (press the ~ key)
3. Type the following commands, pressing ENTER after each:
   disconnect
   game bang
   set hostname "BANG 1.3 Server"
   set deathmatch 1
   set public 1
   set maxclients 8
   map tetra
4. The server should now be running with you in the game too.

If you run a permanent dedicated server, please let me know the IP so I
can post it on the BANG web site.

A LAN game requires a server in the same sort of way, just run a listen
server as described above, then tell the other player to goto 'Multiplayer'
'Join game' from Quake 2, and they should see your game.

Binds
=====
Here is a list of all the new weapon names (eg, use x)
fire grenades
force grenades
bio grenades
swarm grenades
laser grenades
nuke grenades
infest grenades
grenades

fire grenade launcher
force grenade launcher
bio grenade launcher
swarm grenade launcher
laser grenade launcher
nuke grenade launcher
infest grenade launcher
mortar launcher

Support
=======
Goto the BANG Website - http://www.r1ch.net/q2/bang/ for files and stuff. If
you have any problems with the mod, found a bug or something, feel free to
email me.

Credits
=======
R1CH   - Coding, haxing models, etc
Gumby  - Skins for 90% of the models, testing
ValouR - Skins for 10% of the models, converting scheissdreck, testing
#gloom - Testing, ideas, feedback, etc
Saig   - tetra.bsp, symic.bsp, sounds
Sul    - sulbang.bsp
ValouR - dynabang.bsp
DevilW - theta.bsp


Enjoy!

R1CH
http://www.r1ch.net/q2/bang/
