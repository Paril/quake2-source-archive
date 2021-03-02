STAR WARS QUAKE - Multiplayer Test 1.9 - Public Release
========================================================================

IMPORTANT STUFF:
------------------------------------------------------------------------
The Star Wars logo, and the Star Wars universe and all its objects are the property of Lucasfilm Ltd. All rights reserved. Quake, Quake 2, and the stylized "Q" are all trademarks of id Software. All rights reserverd.

LucasArts and Lucasfilm are the rightful, licensed developers of Star Wars products. Id Software and a wide assortment of licensed developers represent the only official creators of commercial Quake, and Quake-based, products.

Star Wars Quake is a completely unqiue and original work of 'fan-dom' and by no means a commercial venture of any sort, nor should it be considered a substitute for LucasArts and Lucasfilm products. The creators and authors of 'Star Wars Quake' are in no way affiliated with Id Software, LucasArts, Lucasfilm or any licensed developer of official Star Wars or Quake products. The 'Star Wars Quake' (or SWQ) modification itself is in no way associated with Id Software or official Star Wars products and is NOT representative of officially licensed Star Wars and or Quake products. The creators of Star Wars Quake make no claim to ownership or formal association with the trademarked names "Star Wars", "Quake", or any derivative. SWQ is an amatuer patch, modification, add-on designed for use exclusively with, a fully registered version (3.20), of Id Software's Quake 2. Yet Id Software is NOT affiliated with Star Wars Quake in any way.

Distribution, etc:
SWQ shall not be included, in whole or in part, with a commercial product of any sort. Nor can SWQ be exchanged for money, valuables, baseball cards, chewing gum, or rare printings of those wide-screen Star Wars trading cards. We're serious about this people, don't even charge a $1.00 to cover the cost of a CD-R. This is as FREE as an airbrushed sketch resembling a stormtrooper or the Star Wars logo. Star Wars Quake must be distributed with all of the original files on the condition that they remain intact and identical to those available on the Star Wars Quake website. No part of SWQ shall be distributed separately or in modified form without the express written consent of its creators. Any faults (i.e. bugs), flaws, comments, or questions about Star Wars Quake should be directed to the SWQ team and NOT to Id Software, LucasArts, Lucasfilm or any other licensed developer/distributor of official Star Wars and or Quake products.

WELCOME!
------------------------------------------------------------------------

* This release contains both SWQ DM Test 1 and psychospaz's latest patch (patch #9).

*  Those unfamiliar with Quake 2 editing or Quake 2 modifications (aka "mods") should carefully read the instructions to follow.

* The Star Wars Quake website (swtc.telefragged.com) is the only place to find people willing to return your e-mail or answer anything Star Wars Quake related. Please respect the folks at LucasArts, Lucasfilm, and Id Software by refraining from malicious messages, bragging, complaining or asking about Star Wars Quake. Without these fine people working hard to develop and support fun and entertaining commercial products, Star Wars would have never been and this project could have never been conceived.

INSTALLATION:
------------------------------------------------------------------------
- Extract SWQTEST1.ZIP to your quake2 directory. For example:
	C:\Quake2\

PLAYING:
------------------------------------------------------------------------
- Run SWQ.BAT from within the SWQ subdirectory of Quake 2 (don't move it!)

- Go to the Multiplayer menu and select
  "Player Setup"  -  configure your name and appearance
  "Start a New Multiplayer Game"  -  to setup a new game

- Choose the map you'd like to use and adjust various other settings

- Parameters
  (extra commands to follow SWQ.BAT):

  +map swqdm1         - skip any demos and begin on map SWQDM1
  +set theforce 1     - enable force powers (some disabled)
  +set saberonly 1    - lightsabers only - some bugs
  +set mouse_s #      - set your mouse sensitivity. This allows ingame
                        sensitivity changing when using the Nightstinger zoom.
                        12 is the fastest used by most players, while ordinary
                        players often prefer something between 6 and 9.
  +set advanced 1     - Turns on advanced saber dueling

- Often, my commandline looks something like:

   C:\quake2\quake2.exe +set game swq +set deathmatch 1 +set theforce 1
                        +set mouse_s 12

[Rip's comment: Now me, I find all this rather tiresome. If you have an openGL card chances are you can just use the same settings I do. In that case, use RIPV.BAT in the same directory and you'll be right in full openGL goodness without the hassle of setting it up. Course, you're gonne have to live with my bunnyhop settings but that's the way life can be ;) ]

CONTROLS:
------------------------------------------------------------------------
These are as set in the commands.cfg file.

Please cutomize to your hearts content from the "Options Menu" or open the file COMMANDS.CFG contained in your SWQ folder and notice the keys assigned to various functions in this manner: bind <key> "command"

EXAMPLE: bind space "jump"

DEFAULT BINDINGS
w                   =  move forward
s                   =  move back
a                   =  move left
d                   =  move right
shift               =  crouch (+movedown)
1-4                 =  weapon menu
5-9                 =  force menu
space               =  secondary weapon fire
e                   =  use or select force power
r                   =  reload
q                   =  use buttons/doors
g                   =  taunt
c                   =  quick forcejump (toggle)
z                   =  quick forcespeed (toggle)
n or t              =  talk to everyone
m                   =  talk to your teammates only (if teamplay enabled)
f                   =  chasecam (automaticly activated for lightsaber)

LIGHTSABER
------------------------------------------------------------------------
Fighting with the lightsaber is similar to LucasArt's incredible Jedi Knight game, in that movement effects which way the saber swings. Moving forward and attacking produces a thrust whilst moving sideways and attacking producing a swing in the direction opposing movement. Some servers now have the option of allowing 'duel mode' for players, and with this advanced flag set players must toggle into duel mode to produce special swings or block incoming laser fire. Otherwise swings are random and conventional weapons fire isn't blocked. This is only the case for servers using the advanced saber system. 

DUEL MODE:
Activating or deactivating the 'duel mode' requires you bind a key to the command "duel". This command will toggle on or off dueling mode on yourself which will effect you if you are toting a lightsaber. In this mode (set by typing in +set advanced 1 when starting up or advanced 1 in the console) only people in duel mode can parry with the lightsaber or execute any advanced moves with said weapon. Unlike normal mode, one can in duel mode also parry devastator bolts although the imperial nightstinger fires are still too fast even for the quickest Jedi. 

CHANGING MAPS:
------------------------------------------------------------------------
Press the ~ key (above TAB) and type:
	
map mapname

In the prompt that appears, valid map names are:
swqdm1              - multiplayer map (4-8 players)
swqdm2              - multiplayer map (8-12)
storage             - multiplayer map (2-4)

EXTRA OPTIONS
------------------------------------------------------------------------
Console Commands (press ~ to access prompt):
These can be typed into the console or bound to certain keys in this manner: bind <key> "command"

map <name>          - immediately change the map
bot                 - add a bot (unstable with theforce or saberonly enabled)
mouse_s #           - mouse sensitivity (for zooming)
secondary           - alt fire on certain weapons; toggles zoom on/off as well
taunt               - display a taunting motion (seen by other players)
set theforce        - enables the force; zero by default, set this to "1" for the next game (quotes unnessesary)
advanced 1          - Turn on dueling mode
duel                - Toggles saber handling when in advanced (duel enabled) mode.
set saberonly       - also zero by default, set this to 1 for sabers only
id                  - enable the display of player names
exec <file>.cfg     - Quake 2 allows you to store multiple configurations for controls, video, sound, and options in individual files known as config.cfg files. They must end in .cfg and can be executed at the console prompt by typing "exec configname.cfg" where configname is the name of your configuration file. Any cfg files stored within "baseq2" will apply to all Quake 2 games and 'mods' while cfg files in the 'SWQ' folder will only work for Star Wars Quake. If you are experiencing trouble with the SWQ menu, try openning the console (Press ~) and typing "exec video.cfg" without the quotes.

KNOWN BUGS
------------------------------------------------------------------------
-lightsaber animations don't always play (move THEN click and they will) reguardless of the animation the saber will always inflict damage

-bots cause instability

STAR WARS QUAKE WEBSITE
------------------------------------------------------------------------
http://swtc.telefragged.com

Stop by the forum and let us know what you think!
-Privateer

Booya!
-RipVTide

m1n3 HTML sK1Llz 0wN j00.
-FuShanks

The TEAM + folks who've contributed greatly
------------------------------------------------------------------------
Red Knight   -  redknight@planethalflife.com

and in alphabetical order...

ApocX        -  apocx@planetwheeloftime.com
Brad         -  braddas@hotmail.com
CaRRaC       -  carrac_nz@hotmail.com
EleCtRiC     -  darb1@direct.ca
falkon2      -  falkon2@tm.net.my
FuShanks     -  fushanks@hotmail.com
Grey Knight  -  gryknght@hotmail.com
Maxer        -  maxer@happy-soft.com
MamboKing    -  edlima@edlima.com (oooh, nice URL =)
Privateer    -  privateer@lightstream.net
psychospaz   -  psychospaz@telefragged.com
RipVTide     -  ripvtide@telefragged.com
Tim Elek     -  4tim2@ameritech.net
ViolentBlue  -  violentblue@hotmail.com
War|ock      -  warlock@telefragged.com

Special Thanks
------------------------------------------------------------------------
Lucasfilm Ltd. & Industrial Light and Magic - www.starwars.com
LucasArts                                   - www.lucasarts.com
Id Software                                 - www.idsoftware.com
Telefragged                                 - www.telefragged.com
LAZARUS (Quake 2 modification)
   Mr. Hyde and Mad Dog                     - http://lazarus.gamedesign.net/index.htm

all you great fans who've been patiently waiting

[Rip Note: And monkeys]

[Priv Note: my cat]

[Fu Note: Definitely the monkeys.]

Various People We've Forgotten
------------------------------------------------------------------------
\n
\n
\n

LEGAL STUFF (again):
The Star Wars logo, and the Star Wars universe and all its objects are the property of Lucasfilm Ltd. All rights reserved. Quake, Quake 2, and the stylized "Q" are all trademarks of id Software. All rights reserverd.

LucasArts and Lucasfilm are the rightful, licensed developers of Star Wars products. Id Software and a wide assortment of licensed developers represent the only official creators of commercial Quake, and Quake-based, products.

Star Wars Quake is a completely unqiue and original work of 'fan-dom' and by no means a commercial venture of any sort, nor should it be considered a substitute for LucasArts and Lucasfilm products. The creators and authors of 'Star Wars Quake' are in no way affiliated with Id Software, LucasArts, Lucasfilm or any licensed developer of official Star Wars or Quake products. The 'Star Wars Quake' (or SWQ) modification itself is in no way associated with Id Software or official Star Wars products and is NOT representative of officially licensed Star Wars and or Quake products. The creators of Star Wars Quake make no claim to ownership or formal association with the trademarked names "Star Wars", "Quake", or any derivative. SWQ is an amatuer patch, modification, add-on designed for use exclusively with, a fully registered version (3.20), of Id Software's Quake 2. Yet Id Software is NOT affiliated with Star Wars Quake in any way.

Distribution, etc:
SWQ shall not be included, in whole or in part, with a commercial product of any sort. Nor can SWQ be exchanged for money, valuables, baseball cards, chewing gum, or rare printings of those wide-screen Star Wars trading cards. We're serious about this people, don't even charge a $1.00 to cover the cost of a CD-R. This is as FREE as an airbrushed sketch resembling a stormtrooper or the Star Wars logo. Star Wars Quake must be distributed with all of the original files on the condition that they remain intact and identical to those available on the Star Wars Quake website. No part of SWQ shall be distributed separately or in modified form without the express written consent of its creators. Any faults (i.e. bugs), flaws, comments, or questions about Star Wars Quake should be directed to the SWQ team and NOT to Id Software, LucasArts, Lucasfilm or any other licensed developer/distributor of official Star Wars and or Quake products.