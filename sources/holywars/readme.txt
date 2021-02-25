Welcome to the Mean Mod!

                        =========
                        HOLY WARS
                        =========

Version  : 2.22 SE (for Quake 2)
Date     : November 6, 1999
Authors  : Paolo "Nusco" Perrotta and Roberto "Taz" Bettazzoni
           Additional modification by Chris "Dexter" Davis
           Mac version by Craig "Acru" Hickey
Email    : nusco@planetquake.com
WWW page : http://www.planetquake.com/holywars/

==========================================
Take it easy, people... you don't need to
read all this stuff to play. Give a glance
at section #1, jump on a server and
      GET DOWN LIKE A SEX MACHINE. 
==========================================


#1. THE GAME
    --------

1.1. What is Holy Wars
     -----------------

Holy Wars is a deathmatch mod for 3 or more players. It's extrememly simple in concept 
and operation, but incredibly fun. 

Holy Wars is strongly oriented towards mayhem and carnage, rather than subtle strategy 
and tactics. Who needs strategy, btw? GORE! WE WANT MORE GORE! 


1.2. The rules
     ---------

At the start of a level, all players are Sinners. They won't score frags for fragging each 
other. Soon, a luminous Halo appears. Pick it up to become a Saint and receive some armour 
and health. The Saint scores frags. Being the Saint is a Good Thing.

If you kill the Saint, the Halo falls on the ground. Pick it up! An abandoned Halo will soon 
disappear, to respawn in its starting position later. The Sinners should ideally avoid 
killing each other and concentrate on the Saint, their common enemy. Of course, hunting 
accidents always happen. But watch out! If you kill other Sinners indiscriminately, you'll 
lose a lot of frags and become an Heretic.

Heretics cannot pick up items, and cannot use armor and most weapons. Killing an Heretic 
grants bonus frags bonus to both Saints and Sinners - so Heretics have no friends. Becoming 
an Heretic is a Bad Thing.

There is more, but you'll find out by yourself eventually. What's important is:

			=====================================================
			                 THE CENTRAL IDEA

			Everybody tries to kill the Saint and steal the Halo.
			The game is always a furious "The Saint vs. The Rest
			of the World". The result is total carnage. 
			=====================================================


1.3. Score system
     ------------

 The defailt scores are:

 Sinner killing Saint:    3 points
 Saint killing Sinner:    1 point 
 Sinner killing Heretic:  3 points
 Saint killing Heretic:   4 points
 Becoming an Heretic:    -5 points
 Picking the Halo:        2 points and Sanctity!


1.4. Installing and launching
     ------------------------

Just unzip the file in your QUAKE2 directory. IMPORTANT: Remember to keep subdirectories when 
unzipping. If you're upgrading from a previous version, overwrite the old files (warning: you 
might want to backup any customized files first). Launch it as you would with any other mod, 
or use the holywars.bat that you find in your HOLYWARS directory. The same directory contains 
an hwdedic.bat to start a dedicated server.

*MacIntosh Users*

The necessary gameppc.lib file is stuffed and saved as a binary. You must have
Aladdin's Stuffit Expander, or Mindvision's MindExpander to uncompress the archive.
If you have either of these two applications on your hard drive simply double-click 
the file named 'gameppc.lib.sit.bin' - it will inflate to your holywars directory 
by default. Once you have it all the way decompressed (the target will be named 
'gameppc.lib') simply follow the instructions for starting a Q2 server. The server 
setup instructions, which are outlined below.


*NEW for SE*
The full version of Holy Wars now has a single pak0.pak file for miscellaneous resources.  
If you are upgrading from a previous installation, you can remove the models, pics, sound, 
and textures subdirectories.


#2. CONFIGURING AN HOLY WARS SERVER
    -------------------------------

2.1. Maps
     ----

 Holy Wars includes some top-quality maps for you to play into:

HW1 ("Kick the P.A.", by Dennis "headshot" Kaltwasser)
HW2 ("Fear in your Eyes", by Warren "Taskmaster" Marshall)
HW3 ("IKDM1", by Iikka "Fingers" Keranen)
HW4 ("Painkiller for Holy Wars", by Sten Uusvali)
HW5 ("Infernal Base", by Daniel "Gom Jabbar" Leinich)
HW6 ("The Dark Zone", by DaTa)
HW7 ("The Gun for Quake 2", by Dennis "headshot" Kaltwasser)
HW8 ("The Killing Machine", by Sten Uusvali)

HW2SE1 ("UltraViolence", by Escher)


2.2. Customizing
     -----------

HolyWars is a very, very customizable mod. If you want to change something, chances are that 
you can. There are many different ways to customize the mod:

MESSAGE OF THE DAY:
You can change the message displayed when a player connects by editing the motd.txt file in 
the HOLYWARS/CONFIG directory. Instructions are in the file itself in the form of comments. 
Server admins are ancouraged to put detailed informations about their server in the MOTD, 
and keep it updated.

MAPS ROTATION:
Server administrators can customize map rotation by editing the map_load.ini file in 
HOLYWARS/CONFIG. Instructions and examples are in the file. You can put options on the maps, 
so that they're loaded only when the number of players or their average ping is higher/lower 
than a certain number.

PARAMETERS:
Many things in the mod can be adapted to your tastes. These include the scoring system, the 
Halo movement, all the timings for halo spawn/disappearance/respawn and much more.

Warning: the Holy Wars parameters were set to defaults which were chosen after extensive 
playtesting with a variety of different values. Changing a parameter which affects gameplay 
might look like a very good idea at first, but sometimes it isn't. Always test your ideas in 
an actual game.

*NEW for SE*
Rather than using a param.ini file in the holywars/config directory, the configuration 
parameters are now console variables.  This allows server operators to change values 
on-the-fly.  Some values will change immediately and others will take effect after a map 
change.  All of these parameters can be "permanently" set from server.cfg as well.

CUSTOM MAPS:
If you launch the game with a map which is not part of the original set of Quake 2 maps, 
the Halo base will automatically be put in a randomly chosen deathmatch respawn point. This 
sucks. You should only use this feature to test a level. If you decide to play a custom level 
with Holy Wars more than once, you'll probably want to choose a nice place for the Halo base 
in it. This way, the base will stay at the same spot in every game. It's incredibly easy to 
add a permanent Halo base in a level - in fact, we used this feature to set the bases in all 
the original Quake 2 levels and the official Holy Wars levels. To set an Halo base in a level 
which doesn't have one, or modify its position in level which already has one, just follow 
this simple procedure:
 1) type HW_EDIT 1 on the console to enter "edit mode";
 2) start the map;
 3) walk around until you find a place you like for the Halo base (you can use 
    CHEATS 1 followed by NOCLIP to reach every place in the level, even if it's in mid-air);
 4) type HW_FIX (if desired) to toggle the telefrag workaround described in the next section
 5) type HW_BASE on the console to set the new base at your current position.
 6) Restart the map.
As soon as you give the HW_BASE command, the Halo base will be placed in your current 
position. The position data will be saved in a file called levels.ini, into the HOLYWARS
/CONFIG directory. The level file itself remains untouched, so don't worry about ruining 
your levels. After restarting the map, you can write HW_HALORESET as many times as you want 
to force a respawn of the Halo and see if you like your new base (this useful to test strange 
bases in mid-air, etc.). The HW_EDIT console variable can only be changed on the server. Set 
it back to 0 to forbid Halo base editing. Please, always keep this variable at 0 when playing 
(changing the base and forcing respawns in the middle of a game is cheating).

*NEW for 2.22*
The levels.ini file now accepts an optional parameter called "fix" (without the quotes).  
When this parameter is present for a given map, the Holy Wars server will use a workaround 
in its telefrag calculations to compensate for teleporter pads which are "buried" in the map 
floor.

2.3. Logging
     -------

Holy Wars is compatible with the Quake2 Logging Standard 1.2. It can produce logs which can 
later be processed by a program like GibStats (www.planetquake.com\gibstats). To start a log, 
type SV LOG on the server. To stop logging and unlock the log file, type SV UNLOG. The game 
will be logged to a file into the quake2 directory. The name of the file is set by the 
STDLOGNAME variable. You can keep the server up for some time and then, when noone is playing, 
use UNLOG, rename the log file and use LOG again (if you just copy the file, the new log will 
be appended at the end of the previous one). This way, you can get periodical loggings of all 
activity on your server.


#3. DISTRIBUTION NOTES
    ------------------

3.1. Disclaimer, copyright and permissions
     -------------------------------------

We don't make any implicit or explicit guarantees about this mod: you're using it at your 
own risk. BTW, it was playtested extensively by many players on different servers before 
release.

Quake 2 is copyright by id Software. This mod is subject to all the legal stuff distributed 
by id Software in their Quake2 license. 

Holy Wars is copyright by Paolo "Nusco" Perrotta (nusco@planetquake.com) and Roberto "Taz" 
Bettazzoni (bettazzoni@sis.dsnet.it).

The maps included are copyright by their respective authors. Look into each map text file 
(directory HOLYWARS/MAPS) for copyright info specific for each map. We don't own these maps.

Authors CAN use this mod to build other mods. In fact, you can do what you want with this 
mod, excluded: 

1) strip out this copyright notice from it; 
2) use it in any way which doesn't comply to copyright laws or any other law;
3) use part of the source code into your own project without making your projects' source available to others;
4) make money from it in any way without asking for the author's permission first.

"In any way" means: "No, you cannot pick up this stuff and put it on a commercial CD".
Any non-commercial use is allowed and welcome. You can redistribute this patch at your own 
leisure. You can modify it or include it into your own multimod patch. You can strip code 
from it and use it for you own mod. Clearly and publicly acknowledging our work is a must 
here. Letting us know of any use of this patch (appearances on web pages or in compilations, 
modifications, etc.) would be polite. The strange language that I used to write this document 
is my own version of English, and it's copyright of myself. 


3.2. Availability
     ------------

Official Holy Wars web page: 

        http://www.planetquake.com/holywars/ 

You'll also find HolyWars 1.5 for Classic Quake on the same site.


3.3. Holy Wars credits:
     ------------------

These are all the credits for versions 1.x and 2.x.

 Programming: Paolo "Nusco" Perrotta, Roberto "TaZ" Bettazzoni
 Additional Programming: Chris "Dexter" Davis
 Map collection: Adriano "Escher" Lorenzini
 Maps: Adriano "Escher" Lorenzini, DaTa, Dennis "Headshot" Kaltwasser, 
       Iikka "Fingers" Keranen, Daniel "Gom Jabbar" Leinich, 
       Warren "Taskmaster" Marshall, Sten Uusvali
 Sounds: Walter "Tolwyn" Sammarchi, Tiziano Dimastrogiovanni
 QuakeWorld and FrogBot Editions: Gerard "numb" Ryan, Robert "Frog" Field
 Additional work: Harris "VorTrab" Hancock, Marco "Cable" Folle, Ben Reser, 
                  Zoran "buddah" Lazovski, Warphead, John Spickes, Kenneth Scott, 
                  Simone Masala, @lbo, Justin "Bocephus" Patterson, 
                  Chris "Stickfigure" Richards
 Menu code from: Rocket Arena
 Standard Log from: GSLogMod
 Support: Paolo "Earthbreaker" Petrini, Nazz Nomad, Paolo "CiC" Spadari, 
          Marc "Grasshopper", The Gib
 Main betatesting: R. Cameron Lewis, Marc "DedMon" Roberts, Marco "Marco" Powerstation, 
                   Jeremy Emerson, Bill "Mangler" Krause, Bryan Beretta, Ed Smith, 
                   Bryan "Fedaykin", Jason "Ender" Dawson, Travis "Tigger" Hildebrand, 
                   John "Kill Me" Lasater, Matt "Meat Bag" Lasater, 
                   Russell "DangeRuss" Phillips, Steve "Yomama" Sanderson

And to all the friends, betatesters and the incredible number of people helping, 
commenting and giving suggestions:

                           Thank you, guys.
