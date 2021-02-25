Catch the Chicken
Release Version 2.0

   Installing and Running
   Installation
Extract ctc20.zip to your Quake 2 directory. Overwriting existing files. If you are using pkunzip use the -d option to extract directories.

   Running the Game
Run Quake2 with the "+set game ctc" command line option.

e.g.
quake2 +set game ctc

   Using GameSpy
To setup the CTC GameSpy tab, from the GameSpy menu select:

View->Custom Tab->Import...

then go to your "quake2\ctc" directory and select the "ctc.qst" file.


   Game Setup/Configuration
   Configuring a game (using the "ctc.ini")
The options in the "ctc.ini" affect the server only, so don't try and change things on a client machine and expect them to work. The default ini file has comments in there that tell you what everything does, you should be able to figure it out if you know what your doing. We would highly recommend the use of the Catch the Chicken Config File Creator to setup the game options, it's a LOT easier.


   Configuring Teams (or not)
To start CTC as a team game first it must be setup in the "ctc.ini" file.

This is how the "teams" ini file option works

0		None (Normal game)
1		Red team
2		Blue team
4		Yellow team
8		Green team

Add the numbers together to get the teams you want in the game.

i.e.
Red + Blue + Green
 1  +   2  +   8   = 11

the line would read:
teams 11

Set "teams 0" to disable teamplay.


   Console Commands
chicken	Invokes the Chicken menu. From this menu you can change CTC runtime options. It most likely will not be available as it defaults to being disabled in the "ctc.ini" file. It is not available if you are playing a team game.
observer	Enter observer mode. Enters player into noclip mode and prevents any interaction with the game. Player must enter the kill command from the console in order to return to CTC. Note that when entering observer mode the observers score is reset.
options	Displays current CTC server options.
vis_weap	Toggles visible weapons on/off. Just incase you set it wrong before the game. ie. in the ini file. This affects all clients.
sv respawn	(Server only) Respawns the chicken to a deathmatch start. This is used if the chicken ends up somewhere that the players can't get to.
cam on	Turn camera mode on. Use the kill command to get out (you will lose all your score).
cam follow	Enter follow camera mode. Pressing fire will cycle through players. You must first use "cam on".
cam chicken	Same as follow except it always follows the chicken. You must first use "cam on".
cam normal	Automatically trace where the action is. Sort of. :) You must first use "cam on".
stdlogfile	Set to 1 to start logging game or 0 to stop. eg. "stdlogfile 1"


   Menu
To use the menus use your inventory selection keys. Defaults are [ and ] to scroll through items, and Enter to select.


   Map Cycling
Map cycling must first be enabled in the servers "ctc.ini" file, then you need to edit the "maplist.ini" to list all the maps you want cycled through. One map name per line using the maps file name minus the ".bsp".

e.g.
q2dm1
base1
q2dm2
q2dm5
cool1


   How to Play
   Standard Play
The object is to grab the chicken and hold onto it for as long as possible before being killed by your opponents. You cannot defend yourself while holding the chicken.

   Team Play
	When you connect to a game you will be prompted to select a team. If you're using an unsupported player model you will be prompted to select one from the list of supported models. The objective is similar to a standard game accept you score for your team. You also have team mates who try protect you from your foe by shooting them with eggs and throwing them around a bit. You can only score a few points each time you get the chicken and must pass it on to your team mates in order to continue scoring.

You throw the chicken in the same way you throw a grenade. Pressing and holding fire will draw your arm back, when you release it you will throw the chicken, the longer you hold down fire the further you throw the chicken. Beware though, the chicken gets restless if you hold him back to long and starts scratching and getting nasty.

Each teams scores are show on the right hand side of the screen, the team holding the chicken will have a chicken over their teams icon. When viewing the full screen scoreboard (by pressing F1), the player holding the the chicken will be written in yellow while all other players will be written in white.



