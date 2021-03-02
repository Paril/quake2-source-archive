RPS v1.3.2 by Axis Productions
Go to www.planetquake.com/axis/ for more RPS related resources.

INSTALLATION
-----------------------------------------------------------------------------------------

Just unzip into your root directory.  (e.g. c:\, if your quake2 directory is c:\quake2\.)
(It's server-side only.)


RULES
-----------------------------------------------------------------------------------------

Goals

The goal is to have your team control all three cubes at once. Then, your team has made a "capture," appropriate rewards are given, and the game starts again.

 

Just Joining

When just joining a server, you will be presented with a menu, allowing you to join a team. Use the "[" and "]" keys to navigate the menu, and the enter key to pick one. If the team was empty, you will be given a Power Cube.  (Or, you will be put on a team automatically, if  the server option "autoteams" is on.)

 

Display

At the top right, there is a number for your score (frags) and, below that, a number for how many cubes you have. On the bottom right, there is the team display. All three teams' score and number of cubes currently controlled are showed. Also, to the left of each row, is a letter (R, P, or S) designating which team that row is for. The team's letter that you are on is in green.

 

Damage System

There are three teams, rock (red), paper (blue), and scissors (green). Each team does no damage to other teammates, full damage to another, and 1/4 damage to the third, emulating classic RPS as described above. If you manage to kill someone from the team that you do 1/4 damage to, you get 2 frags.

Rock does full to Scissors, a fourth to Paper.

Paper does full to Rock, a fourth to Scissors.

Scissors does full to Paper, a fourth to Rock.

 

Power Cubes

There will be one Power Cube in play for each team with players. Cubes always enter the game by "magically" appearing in one of the team member's inventory. If you are the first one to join a team, you will start off with a cube. If any team has possession of three cubes at any one time, then they get a point, and each team is allocated one cube (a random player from that team gets it), to start over again. If a player has a cube, he/she/it emits a yellow particles and glow with a light. 

 

Rewards

If any player has possession of a cube when their team makes a "capture," to equate RPS with CTF, then their max health is increased by 2, with a ceiling of 150. This isn't much, but to the team that deals 1/4 to you, it is eight points, with a maximum of an extra virtual 200 health. This bonus carries over deaths, but not map switches. No bonus is given for holding more than one cube at capture time. 

 

New Commands

The new commands are as follows: 

"goal" : Tells you which teams you should be attacking or avoiding. (I found many people would forget which team was which.) This is also displayed after every respawn.

"team" : Tells you what team you are on. If followed by a team name, i.g. rock, then switches you to that team. 

"autoteams" (Server Command): Defaults to on. While on, players are automatically placed on a team when joining the game.

"stdlogfile" (Server Command): Turns standard logging on or off.  Default is off.

"stdlogname" (Server Command): Changes the name of the log file.  Default is "rps/StdLog.log"

 

Less Than Three Teams

If there are only one or two people, then the whole damage system is ignored (else one person would have a tremendous advantage over the other) and there will not be enough cubes spawned to make a capture. 

 

Strategy

With the Rock, Paper, Scissors damage system, it is advantageous to avoid your antithesis team (the team you can barely hurt) like the plague. Instead, if they control cubes, you should back off the team you CAN hurt, and let them get the cubes. Then, you can kill them easily, and make a capture. However, if they are being "uncooperative," then you'll just have to head for the quad and hope for the best against your antithesis team.

 

Thanks

I want to especially thank the Rocket Arena people for the menu source code, and the people of CJW's huge summer LAN party for testing, especially Mr. Goat's computer. ;). And Sucka for his extremely generous support.
