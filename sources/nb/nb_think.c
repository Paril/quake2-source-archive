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

void gonetgo (NeuralNet* net, edict_t* self);

void ClientThink (edict_t *ent, usercmd_t *cmd);


/*==========================================================================
bot_think
---------

This is called every .1 secs
===========================================================================*/
void bot_think ( edict_t* self)
{	


	vec3_t      angles = { 0, 0, 0 };	//angles that bot wants to turn body to

	
	VectorSet(self->client->ps.pmove.delta_angles, 0, 0, 0);

	memset(&self->client->botinfo.ucmd, 0, sizeof(usercmd_t));	//clear the bot's commands

	
	
	if (self->deadflag == DEAD_DEAD)			//if bot is dead
	{

		self->client->buttons = 0;				

		self->client->botinfo.ucmd.buttons = BUTTON_ATTACK; //press attack so bot can respawn

	}

	//change bot angles here?

	//NOTE: put enemy find funcs up here
	self->enemy = choose_target(self);//lock onto an enemy

	if (random() < 0.4 ) // search occasionally for an item
		self->client->botinfo.targetitem = choose_targetitem(self);


	gonetgo (&self->client->botinfo.itsNet, self);	// go through one cycle of the bot's neuralnet.  gonetgo() also executes the input + output functions
		
	///// movement stuff ////
	bot_executemove(self, &self->client->botinfo.ucmd); // add up all the movement votes and set *ucmd
	bot_clearmovevotes(self); // clear votes for next time



	self->client->botinfo.ucmd.msec = 100;

	
	VectorCopy(self->client->v_angle, angles);	//make 'angles' equal to the bots view angles

	self->client->botinfo.ucmd.angles[PITCH] = ANGLE2SHORT(angles[PITCH]);
		
	self->client->botinfo.ucmd.angles[YAW]   = ANGLE2SHORT(angles[YAW]);

	self->client->botinfo.ucmd.angles[ROLL]  = ANGLE2SHORT(angles[ROLL]);
		//copy bot view angles over to ucmd so player physics can use them

	//if(self->client->ps.pmove.pm_type == PM_DEAD)//don't move if dead
	if(self->deadflag == DEAD_DEAD)
	{
		self->client->botinfo.ucmd.angles[PITCH] = 0;

		//safe_bprintf(PRINT_HIGH, "DEAD_DEAD\n");

		self->s.angles[PITCH] = 0;
	}

	



	

	if(thegame->explorescore != 0)	//don't do if it doesn't count anyway
		self->client->botinfo.explorenesssofar += getExploreness(self); //add to 'moving around' score.


	///// check for genetic algorithm evolve: /////
	//this has nothing to do with the individual bots, it's just a conveniant place to put the check.
	if(level.time >= thegame->next_evolve_check_time)//if time to check whether the GA should be run
	{
		thegame->next_evolve_check_time = level.time + 0.5;//check again in 0.5 secs
		
		if(shouldRunGA(thegame))//
		{
			Evolve();//run the GA

			safe_bprintf(PRINT_HIGH, "gametime elapsed: %i   ", (long)(level.time - thegame->last_evolve_time) );
					
			safe_bprintf(PRINT_HIGH, "realtime elapsed: %i\n", (long)( (clock() - thegame->real_last_evolve_time) * 0.001) );

			safe_bprintf(PRINT_HIGH, "actual acceleration factor * 1000: %i\n\n", (long)( (float)(level.time - thegame->last_evolve_time) / ((float)(clock() - thegame->real_last_evolve_time) * 0.000001) ) );
			
			thegame->last_evolve_time = level.time;
			thegame->real_last_evolve_time = clock();
			thegame->evolve_time = level.time + thegame->evolve_period_ceiling;		
		}
		
	}

	/// check for NN structure GA run //////
	if (thegame->structure_evolve_time < level.time)
	{
		if(thegame->struct_evolve_on)
			Struct_Evolve();
		thegame->structure_evolve_time = level.time + thegame->structure_evolve_period;
	}

	// BFG laser from bot to it's enemy: just for testing and for a laugh
	if ( (self->enemy != NULL) && thegame->enemy_trail)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (self->enemy->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PHS);
	}

	// BFG laser from bot to it's enemy: just for testing and for a laugh
	if ( (self->client->botinfo.targetitem != NULL) && thegame->item_trail)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (self->client->botinfo.targetitem->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PHS);
	}

	

	ClientThink(self, &self->client->botinfo.ucmd);
		//clientthink is predominantly player physics stuff

	if(self->deadflag == DEAD_DEAD)
	{
		self->client->botinfo.ucmd.angles[PITCH] = 0;

		//safe_bprintf(PRINT_HIGH, "DEAD_DEAD\n");

		self->s.angles[PITCH] = 0;
	}


	self->nextthink = level.time + FRAMETIME;
		//think again in 0.1 secs
	
}
/*
void bot_think ( edict_t* self)
{	
	//TANGENTIAL NEWCODE

	vec3_t      angles = { 0, 0, 0 };	//angles that bot wants to turn body to

	//NEWCODE
	


	VectorCopy(self->client->v_angle, angles);	//make 'angles' equal to the bots view angles

	VectorSet(self->client->ps.pmove.delta_angles, 0, 0, 0);


	memset(&self->client->botinfo.ucmd, 0, sizeof(usercmd_t));	//clear the bot's commands	
	

	

	self->client->botinfo.ucmd.msec = 100;



	self->client->botinfo.ucmd.angles[PITCH] = ANGLE2SHORT(angles[PITCH]);

	self->client->botinfo.ucmd.angles[YAW]   = ANGLE2SHORT(angles[YAW]);

	self->client->botinfo.ucmd.angles[ROLL]  = ANGLE2SHORT(angles[ROLL]);



	



//	if(self->client->botinfo.mygame->explorescore != 0)	//don't do if it doesn't count anyway
//		self->client->botinfo.explorenesssofar += getExploreness(self); //add to 'moving around' score.

		self->enemy = choose_target(self);

	if (random() < 0.4 ) // search occasionally for an item
		self->client->botinfo.targetitem = choose_targetitem(self);


	///// check for genetic algorithm evolve: /////
//	if (self->client->botinfo.mygame->evolve_time < level.time)
//	{
//		Evolve(self->client->botinfo.mygame);
//		if(self->client->botinfo.mygame->sumfitnesstarget != 0)
//			adaptEvolvePeriod(self->client->botinfo.mygame);
//	
//		self->client->botinfo.mygame->evolve_time = level.time + self->client->botinfo.mygame->evolve_period;		
//	}

//	///// check for NN structure GA run //////
//	if (self->client->botinfo.mygame->structure_evolve_time < level.time)
//	{
//		Struct_Evolve(self->client->botinfo.mygame);
//		self->client->botinfo.mygame->structure_evolve_time = level.time + self->client->botinfo.mygame->structure_evolve_period;
//	}

	// BFG laser from bot to it's enemy: just for testing and for a laugh
	if ( (self->enemy != NULL) && self->client->botinfo.mygame->enemy_trail)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (self->enemy->s.origin);//client->botinfo.targetitem->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PHS);
	}

	// BFG laser from bot to it's enemy: just for testing and for a laugh
	if ( (self->client->botinfo.targetitem != NULL) && self->client->botinfo.mygame->item_trail)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (self->client->botinfo.targetitem->s.origin);//client->botinfo.targetitem->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PHS);
	}

	
		
	//self->client->botinfo.Botrunning = false; // bot is not running by default.
		// Botrunning is set to true in bot_executemove if needed.
	
	//if (self->deadflag != DEAD_DEAD) //if bot is alive
	//{


		gonetgo (self);	// go through one cycle of the bot's neuralnet.  gonetgo() also executes the output functions
		
		///// movement stuff ////
		bot_executemove(self, &self->client->botinfo.ucmd); // add up all the movement votes and set *ucmd
		bot_clearmovevotes(self); // clear votes for next time
	//}

	if (self->deadflag == DEAD_DEAD)			//if bot is dead
	{

		self->client->buttons = 0;				

		self->client->botinfo.ucmd.buttons = BUTTON_ATTACK; //press attack so bot can respawn

	}

	ClientThink(self, &self->client->botinfo.ucmd);


	

	self->nextthink = level.time + FRAMETIME;
	
}
*/




