=============================================================================
Title    : Artifact Quake 2
Filename : aq2_4b.zip
Release  : 4 (beta cause it's unfinished. But likely, it's the final release)
Date     : 6.30.2001
Authors  : Nick "Singe" Bousman, code and such
           Glenn "Griphis" Saaiman, models and such
E-mail   : singe@ix.netcom.com
           griphis@mindspring.com
=============================================================================

 SPECIAL NOTE:
 -------------
   I was not able to finish AQ2 due to lack of free time and other various
factors. But this is as complete as I plan to make it. Some artifacts don't
work perfectly, a couple others aren't implemented, and the whole shebang
has plenty of bugs in it. But I hope some people can manage to enjoy it in
a deathmatch sometime somewhere. :)

 Type of Mod
 -----------
 gamex86.dll : yes
 Sound       : yes
 MDL         : yes
 Level       : no

 Format of Mod
 -------------
 A pak0.pak file and a gamex86.dll file

 Description:
 ------------
   Artifact is a tongue-in-cheek deathmatch mode intended to enhance the
quality of deathmatch without changing the main intent (getting frags) of
the game. Originally, Quake CTF had four runes in it. These granted special
powers when picked up. They were Haste, Regeneration, Strength, and
Resistance. "Artifact" takes these original four artifacts plus many many 
others into regular deathmatch. Each artifact grants its own special power,
ability, or trait to the lucky player who finds it. It's a very simple 
concept which is easy to get a quick hang of, but it still offers a wide
variety of techniques players can use due to the vast number of artifacts
added to the game.
   Artifact has a few different methods of play, each determined by what
settings the server administrator chooses to run his or her Artifact server
on. Following below are descriptions of the various artifacts, commands,
settings, and examples of game modes.

 Installation Instructions:
 --------------------------
   This text file is packaged with the Artifact Quake 2 ZIP. This means you've
probably figured out how to run WinZIP or another sort of ZIP utility. Make
a sub-directory in your Quake2/ directory called "artifact". Unzip the contents
of the archive into this directory. To run Artifact Quake 2, start Quake 2 with
the command line "Quake2 +set game artifact". Another way of starting up an
Artifact Quake 2 game is to start Quake 2 the normal way, then at the console,
enter:
] game artifact
 or
] set game artifact
 Don't type in the close bracket characer, that's just here to represent the
console prompt.

 How to Play:
 ------------
   These instructions detail how you play Artifact in deathmatch mode. There
are slight but key changes to a single-player mode of play, but won't be
covered here.In Artifact Quake 2, the basic premise is that various
"artifacts" are randomly thrown into the level either at the start of a game,
or at various times in the middle of a game.
   Walk over the artifact to pick it up. If it's the first artifact you've
picked up, it will automatically be equipped. On the lower left of your
screen, you will see the name of your equipped artifact. On the lower right
of your screen, you will see the icon representing your equipped artifact.
   Depending on how the server was configured, you might be able to pick up
just one artifact, or you might be able to pick up many artifacts. If you
wish to equip another artifact you've picked up, select it in your inventory
and press the standard "use" button to equip it. Only one artifact may be
equipped at any given time. To get rid of an artifact, if the server allows
it, select from your inventory the artifact you wish to discard and press the
standard "drop" button. The advantage to picking up artifacts is that they
grant special abilities to players who possess them. Some are "active"
abilities, and some are "passive" abilities.
   Passive abilities happen automatically. You don't have to do anything for
a passive effect to occur. An example is the Resistance Artifact. Damage you
take is automatically halved if you have that artifact equipped. If the server
has the "all passive" game setting on, all passive effects of artifacts in
your inventory will be on. If the "all passive" setting is off, only the
passive effects of your equipped artifact will be on.
   For "active" abilities, you need to press a button to use or activate the
ability. The best way to do this is to bind a key to the command "aq-use".
The artifact descriptions refer to this command and the key you bind it to as
"The Button". If an artifact has active abilities, they are controlled with
this command. Another way of using an "active" ability is to press the
standard "use" button when the artifact selected in your inventory is the
same as your currently equipped artifact.
   When you die, you lose your artifacts. Artifacts are also not kept
across level changes.

 Player Commands:
 ----------------
   The following is a list of commands available to players in AQ2 games.
 "aq-button" - Attempt to use active power of your equipped artifact.
 "aq-use" - Attempt to use active power of your equipped artifact.
 "aq-drop" - Discard/drop your equipped artifact. (Regardless of inventory)
 "aq-give <name>" - Give yourself the <name> artifact. (If cheats are allowed)
 "aq-help" - Print a generic help screen, or other help screens as specified.
 "aq-info" - Display information about your equipped artifact.
 "aq-info <name>" - Display information about the <name> artifact.
 "aq-motd" - Display the server's connect Message Of The Day.
 "aq-server" - Display server settings.
 "aq-banlist" - Display a list of which artifacts have been banned from the server.

 Helpful Information:
 --------------------
- Every artifact has a long and short name. The short name is displayed
in your inventory, on the banlist, with the ban commands, and with the
aq-give command. The long name is displayed on your HUD, upon pickup, and
on the with the aq-info screen.
- For those of you who don't know how to bind commands, the syntax is as
follows below. Don't include the <> brackets or the ] bracket.
] bind <key> "<command>"
An example of this would be:
] bind p "aq-use"
- Because of client prediction, some items that follow the player will
have delayed reaction times in relation to the player. Flight is also
more awkward with this on. If simply messing around on your own, or
in a LAN game where ping is mostly irrelevent, turn off client
prediction with the console command:
] cl_predict 0

 Artifact List:
 --------------
 Action Movie Arsenal - All weapons are given upon pickup. All ammunition
recharges slowly.
-----------------------------------------------------------------------------
 Berserker Artifact - You deal 6x damage, but soak up 3x damage.
-----------------------------------------------------------------------------
 Blink Artifact - Press "The Button" to randomly teleport. Doesn't always
work.
-----------------------------------------------------------------------------
 Camouflage Artifact - You're fully invisible as long as you don't move or
shoot. If you do either of these, you're simply very transparent.
-----------------------------------------------------------------------------
 Cloaking Artifact - Walk around as a hard-to-see pair of eyes. When
firing or when damaged, cloaking is reduced to faint transparency. Weapons
are silenced, standard pickups make no sound for you, and if you're careful
you shall make no footsteps.
-----------------------------------------------------------------------------
 Death Spots - Press "The Button" to create a death spot and lose 10 health.
The first person to touch a death spot, dies. Death spots explode on their
own after a while. You may have a maximum of 5 Death Spots out at once.
-----------------------------------------------------------------------------
 Artifact of Divine Wind - Press "The Button" to begin a three click
countdown, after which you explode with enough force to vaporize anybody in
visible range. No frags are lost for this method of suicide. If you die by
other means, you'll explode, but with not quite AS much force as it could be.
-----------------------------------------------------------------------------
 DUAL Recall Artifact - Press "The Button" the first time to teleport back
to where the artifact was picked up. Press "The Button" at any other time
to teleport to where you last teleported from.
-----------------------------------------------------------------------------
 Artifact Electric - Electrification! Any enemy within a certain radius to
you will be zapped, providing you can spare some cells to power it.
Water BAD!
-----------------------------------------------------------------------------
 Environmental Protection Artifact - Slime and lava regenerate your health.
Water regenerates health slowly. If stuck in liquid, press "The Button" to
jump out. You've also got immunity to to falling damage, crushing damage,
ice traps, death spots, radiation, death orbs, artifact electric attacks,
and so on.
-----------------------------------------------------------------------------
 Artifact of Fire Walking - When running, a harmful trail of fire is left on
the ground where you've passed. Helps in keeping people from chasing you.
-----------------------------------------------------------------------------
 Flight Artifact - Fly around as you please. Pretty self-explanatory.
-----------------------------------------------------------------------------
 Floating Spiky Death Cloud - A big spiked cloud floats above your head,
dropping little pieces of harmful chaff on the ground.
-----------------------------------------------------------------------------
 Artifact of Good Health - Your upper limit on health is 666. Feel free to
grab just about as many health boxes as you like. Or perhaps an adrenaline...
-----------------------------------------------------------------------------
 Gravity Well - Press "The Button" first to drop a Gravity Well.
Gravity Wells suck players and flying items inward. Gravity wells damage
anything that gets too close. Press "The Button" when Gravity Well is
deployed to recall it. If you are killed, Gravity Well will dissapear after
about 30 seconds.
-----------------------------------------------------------------------------
 Haste Artifact - Rate of fire is dramatically increased. Hyperblaster shots
move faster. Press "The Button" to perform a super jump.
-----------------------------------------------------------------------------
 Holo-Artifact - Press "The Button" to drop a holographic decoy image of
yourself. Each image dissapears in about 30 seconds. You may have a maximum
of 5 of these out at once.
-----------------------------------------------------------------------------
 Ice Traps - Press "The Button" to set an ice trap. The first enemy to run
into an ice trap will be frozen for 12 seconds. Said enemy is unfrozen in a
few seconds or when killed.
-----------------------------------------------------------------------------
 Impact Artifact - Run head-on into an enemy to slam them up
against, whatever. Hurts a lot.
-----------------------------------------------------------------------------
 Orb of Death - A menacing orb rotates around you, severely bashing anybody
who comes in contact (except yourself). If you die, it'll bounce around
aimlessly for a bit by means of Artificial Stupidity.
-----------------------------------------------------------------------------
 Phased Artifact - Walk through some walls, but hard to control.
-----------------------------------------------------------------------------
 Phoenix Artifact - Three extra lives are granted. Instead of respawning
elsewhere after death, you respawn where you died with health fully
restored, inventory and weapons intact, and 10 seconds of lovely Quad power.
-----------------------------------------------------------------------------
 Artifact of Radioactivity - Press "The Button" to set a radiation point,
which in 5 seconds will emit high amounts of radiation for 45 seconds.
Anything in that area will be damaged while in visible range of said point.
A radiation point cannot be set in view of another radiation point. If
you're killed, a radiation point is automatically set where you fell.
-----------------------------------------------------------------------------
 Recall Artifact - Press "The Button" a first time to set recall point.
Press "The Button" while on the recall point to erase it. Press
"The Button" anywhere else to teleport to the recall point.
-----------------------------------------------------------------------------
 Regeneration Artifact - Your health regenerates at 5 units per 1.5 seconds.
If you've got armour, it too regenerates at 5 units per 1.5 seconds.
-----------------------------------------------------------------------------
 Repulsion Artifact - Automatically repel players and various floating
objects.
-----------------------------------------------------------------------------
 Resistance Artifact - All damage you take is halved.
-----------------------------------------------------------------------------
 Shielding Artifact - Press "The Button", and invincibility is granted for
a few seconds. This ability takes a few seconds to recharge after each use.
-----------------------------------------------------------------------------
 Shub Hat - Huge rotating monster image is placed on head.
-----------------------------------------------------------------------------
 Stats Switcher - Press "The Button" while aiming at another player. If
their stats are better regarding weapons, health, items, and ammo, those
appropriate stats will be switched.
-----------------------------------------------------------------------------
 Strength Artifact - You deal double damage.
-----------------------------------------------------------------------------
 Tank Helper - Tank materializes to fight on your side. Press "The Button"
to teleport 'im to your location. If others deal enough damage to 'im,
he'll teleport back to you automatically.
-----------------------------------------------------------------------------
 The Teleport Shuffle - Aim in a direction you wish to go and press
"The Button". If possible, you will be teleported to the spot you were
aiming at. If you aim at an enemy, positions will be switched. 10% chance
of telefrag.
-----------------------------------------------------------------------------
 Artifact of Uncontrollable Jumping - Any enemy in visual range will jump
uncontrollably like a giddy schoolgirl.
-----------------------------------------------------------------------------
 Vampiric Artifact - Any damage you dish out is given back to you in health.
1/5th effective versus non-sentient entities.
-----------------------------------------------------------------------------
 Artifact of Vengeance - All damage you take is dealt right back at your
attacker.
-----------------------------------------------------------------------------
 Artifact of Weirdness - Tiny bubbles float off you. Press "The Button" to
blow one in the direction you're facing. If killed, you release a few
automatically. Players who touch the bubbles will experience various odd,
annoying effects for a while.
-----------------------------------------------------------------------------

 Running an Artifact Server:
 ---------------------------
   Sorry, I don't know much about running servers so I can't give much help
regarding this. At least, that goes for dedicated internet servers. But it
should be the same method used for running a regular Quake 2 server, with
a few minor differences.
   Artifact Quake 2 has various settings you can play with. These are
controlled via new cvars. They, and their descriptions, are listed in the
following section. Like the built-in cvars, you can change their values in
the console. To set a server variable upon startup you can either _add_ it
to your Quake 2 command line, or put it into a cfg file. The syntax for
both methods is "+set <cvarname> <value>". An example of this would
be "+set aq_allpassive 1".
   Another feature of Artifact Quake 2 is the ability to ban certain
artifacts from your server. To do this first create a file called
"aq_banlist.txt" in your AQ2 directory. For each artifact you want banned
from the game, place its (short) name on a line by itself in the text
file. Having this file is not required for game operation. You can ban
artifacts manually on the server with the "aq-ban" and "aq-unban" commands
but remember, every time the dll is loaded the banlist is reset and/or
reloaded from the banlist file. The "map" command will reload the dll, but
the "gamemap" command will not.
   Artifact Quake 2 also allows custom MOTDs. This is the message displayed
to players when they connect to the server. To use the custom MOTD, simply
put the message you want in a text file called "aq2_motd.txt", and place the
file in your Artifact Quake 2 directory. Having this file is not required
for game operation.

 Server Variables:
 ----------------- 
   There are various variables that Artifact Quake 2 uses to determine the
type of game which is to be played. Below is a comprehensive list and
description for each variable. The number in parenthese following the name
is the cvar's default value.
 "aq_existmax" (16) - This is the maximum number of artifacts that can
   be on the map at any single given time.
 "aq_migrate" (256) - This is the amount of time, in seconds, in which
   artifacts will warp to another spot on the map. This keeps away the
   possibility of them accumulating all in one inaccessible spot. A
   zero value will prevent the artifacts from migrating at all.
 "aq_pickupmax" (1) - This is the maximum number of artifacts each player
   is allowed to hold. I personally prefer this to always be set to 1, but
   some people like running games where players can have more than one
   artifact at once.
 "aq_mortality" (1) - This determines how artifacts are treated when
   acquired by the player. Mortal artifacts, a value of "0", work like the
   old Artifact Quake. Artifacts die with the player, and the world slowly
   replentishes the vaccum by spawning new artifacts in random spots.
   Immortal artifacts, a value of "1", work like the CTF runes. When a
   player holding artifacts is killed, the artifacts pop back out of the
   player. Ghost artifacts, a value of "2", work like weapons when
   weaponstay is on. Artifacts die with the players, like mortal artifacts,
   but when a player picks up and acquires an artifact, the pickup remains
   on the ground for others to get.
 "aq_spawnall" (1) - This determines spawntime of artifacts. If "1", all
   artifacts are spawned at the beginning of the level. If "0", the level
   starts with no artifacts, and slowly begins spawning new ones.
 "aq_itype" (0) - This determines whether or not players know what kind of
   artifact they're picking up is before they actually get it. The old
   Artifact Quake had invisible types because we didn't use any custom
   models. We have custom models for all artifacts now, but some sysadmin
   like the idea of players not knowing what they're picking up before
   they've actually done so. So for a value of "0", all artifacts look
   different. For a value of "1", all artifacts use the same pickup model.
 "aq_droptime" (32) - This is the amount of time, in seconds, in which
   players are required to wait before dropping or discarding an artifact
   they've picked up. A value of zero allows artifacts to be dropped
   instantly. A negative value prevents players from ever dropping any
   artifacts.
 "aq_allpassive" (1) - This is an obscure, but very important variable.
   If "1", all the abilities and traits of artifacts in a player's inventory
   will be active. If "0", only the abilities and traits of a player's
   equipped artifact will be active, regardless of what other artifacts
   he or she might have in their inventory.
 "aq_xrandom" (0) - Artifacts are generally thrown into the level with types
   selected at random (barring types on the banlist, of course). When 
   this variable is set to 1, any type of artifact spawned will not be 
   spawned again until all other types of artifacts have been spawned.
   For example consider aq_existmax is 4 and all but four artifacts are
   banned. With aq_xrandom off, the four artifacs that appear in the level 
   could be any combination of the four not-banned artifacts. With
   aq_xrandom on the only combination would be one of each of the four
   artifacts.

 Server Commands:
 ----------------
   These are Artifact Quake 2 commands which can be used by the server alone.
 "sv aq-ban <name>" - Bans <name> artifact from appearing in the game.
 "sv aw-unban <name>" - Removes from the banlist, the <name> artifact.

 Examples of game modes:
 -----------------------
Original - aq_existmax = 30  aq_pickupmax = 1  aq_mortality = 0
           aq_spawnall = 0   aq_itype = 1      aq_droptime = 20
- Just like the original Artifact Quake. Fairly well balanced. Though for
appearences, I'd probably go with aq_itype being set to 0 just so you can
see all the pretty pickup models
-----------------------------------------------------------------------------
CTF - aq_existmax = 4  aq_pickupmax = 1  aq_mortality = 1
      aq_spawnall = 1  aq_itype = 0      aq_xrandom = 1
- Be sure you ban all artifacts but the original four: Haste, Regeneration,
Strength, and Resistance. Four artifacts, can only hold one at once, and
they pop out of a player when the player's killed. This is exactly the same
setup of artifacts, runes, relics, or whatever you wish to call that CTF
uses.
-----------------------------------------------------------------------------
Artifacts As Weapons - aq_existmax = 20  aq_pickupmax = 4  aq_mortality = 2
                       aq_spawnall = 1   aq_migrate = 0    aq_allpassive = 0
- Never tried it, but I believe it'd be interesting. Run around and take
your pick favourite four artifacts (even though only one can be active at
a time). Everybody can make the same choice too, cause those artifacts
are GLUED to their spots, and ain't ever gonna move.
-----------------------------------------------------------------------------
Psycho - aq_existmax = 50  aq_pickupmax = 30  aq_mortality = 1
         aq_spawnall = 1   aq_allpassive = 1
- This is bad. Don't seriously try to deathmatch like this. :)
-----------------------------------------------------------------------------

 Known Bugs:
 -----------
(check the aq2_todo.txt for a full listing)

 Releases:
 ---------
- 1.b - The first release. Untested in full-scale deathmatch or on any 
        internet servers.

- 2.b - The second release. Some artifacts added. Some bugs fixed.

- 3.b - The third release. All but 4 artifacts in place. Again, some bugs
        fixed. Some multiplayer testing conducted.

- 4.b - The fourth and final release. All but 2 artifacts in place. Still
        plenty of bugs, but I just don't have the time to continue this 
        project.

 Misc:
 -----
   Visit http://singe.telefragged.com/ for information on all our projects.

 Thanks to:
 ----------
*** QDLevels tutorial people ***
*** anybody we stole sounds from :) ***

 Copyright and Distribution Permissions
 --------------------------------------
   Authors may NOT use these modifications as a basis for other
publically available work unless they ask me, Singe, first.
   You may distribute this Quake 2 modification in any electronic
format, provided it goes unmodified and includes all files found in
the ZIP. You can only add it to a CD-Rom collection if you get permission
from the authors (Griphis and myself) first.
   Actura Software may not do anything with this mod.

 Availability:
 -------------
   http://singe.telefragged.com/artifact (if anywhere)
