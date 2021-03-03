/*========================================================================
	Neuralbot v0.6						23/November/99
	--------------
		
Neuralbot is made by Nicholas Chapman (c)1999

Feel free to slice and dice this code as you see fit.
	If you want to use any of the code for any of your projects, feel free.
If you release this project, drop me a line as I would like to see what 
this code has been used for.
	If you wish to use any of this code in a commercial release, you must gain 
my written consent first.

The area of bot AI is large, and way more experimentation needs to be done
with neural-networks and g.a.s in bots than I could ever do.  So I encourage 
you all to use this code as a base or inspiration or whatever and see if you 
can squeeze some learning out of a bot.

  Nick Chapman/Ono-Sendai		nickamy@paradise.net.nz		
								onosendai@botepidemic.com
								icq no# 19054613

==========================================================================*/
#include "g_local.h"



/*============================================================================
bot_move stuff
------------------------------------------------------------------------------
This unusual system keeps updates the tally of the all the directions the 
bot wants to go in. :)
  Bot_executemove is called later in bot_think and makes the bot actually 
move depending on what the tally is at.

This voting system may come in useful if the neuralnet executes at a greater 
frequency than the bot moving code.
============================================================================*/


void bot_clearmovevotes (edict_t* bot)
{
	bot->client->botinfo.runvote = true;
	bot->client->botinfo.forwardvotes = 0;
	bot->client->botinfo.rightvotes = 0;
}



/*============================================================================
bot_executemove
------------------------------------------------------------------------------
this function sets the 'cmd' values after tallying up all the movement votes.
cmd is a pointer to the user command struct that holds the movement and firing 
commands that the bot wants to execute.
============================================================================*/
void bot_executemove(edict_t* bot, usercmd_t* cmd)
{

	int forward = 0; 
	int right = 0;    

	if(bot->client->botinfo.forwardvotes >= 1)
		forward = 1;
	else if(bot->client->botinfo.forwardvotes <= -1)
		forward = -1;

	if(bot->client->botinfo.rightvotes >= 1)
		right = 1;
	else if(bot->client->botinfo.rightvotes <= -1)
		right = -1;
	
	if(bot->client->botinfo.runvote)//if bot wants to run
	{
		forward *= 2;
		right *= 2;
	}

	cmd->forwardmove = forward * 200;	//players run at 400, walk at 200
	cmd->sidemove = right * 200;

	//NOTE: set bot->client->botinfo.wasblocked ?
}

