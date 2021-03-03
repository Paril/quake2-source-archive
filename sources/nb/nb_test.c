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
#include <time.h>


long get_num_nosynapses();
long get_num_synapses();
long get_num_dynamic_neurons();
long getPercentSuccess();

void Cmd_Starttest_f()
{
	
	float weightx1000;
	long printnum;
	int i;

	float z;
	long printz;

	float somecharge = 0.666;
	thegame->Neuralbot_Array[3]->client->botinfo.itsNet.Synapse_Array[37][0][0].weight
	= somecharge;

	/*
	for ( i=25;i<50;i++)
	{
		z = weightx1000 = self->client->botinfo.mygame->bot7->itsNet.Neuron_Array[i][4].charge * 1000;
		printz = z;
		safe_bprintf (PRINT_HIGH,"charge of bot7 LAST ROW 25 - 49: %i\n", (long)printz);
	}
	*/
}

void Cmd_Report_f()
{
	/*
	float afloat;
	afloat = 5.0/3.0;
	safe_bprintf (PRINT_HIGH,"float: %i\n", (float)afloat);
	*/

	long printnum;
	printnum = (long)(random()*1000000000);
	safe_bprintf (PRINT_HIGH,"random*1000000000: %i\n", printnum);

}



void Cmd_Test_f()
{
	int i, b;
	float wsum=0;
	gitem_t* it;
	float x,y,z;
	char test[128];




	long l = 456457645;

	safe_bprintf (PRINT_HIGH, "break...\n");


	safe_bprintf (PRINT_HIGH, "long: %i\n", (int)l);


	safe_bprintf (PRINT_HIGH, "sizeof botgame: %i\n", sizeof(Botgame));


	//char	*s;

	// use an array so that multiple vtos won't collide


	//Com_sprintf (test ,sizeof(test), "hello");



	//for(i=0; i<30; i++)
	//	printThinkFuncForNeuron(&thegame->Neuralbot_Array[0].itsNet.Neuron_Array[i][0]);

	//for(i=0; i<thegame->Neuralbot_Array[0].itsNet.HIDDEN_NEURONS; i++)
		//printThinkFuncForNeuron(&thegame->Neuralbot_Array[0].itsNet.Neuron_Array[i][1]);

	//if(level.time > 5)
	for(i=0; i<thegame->NUM_BOTS; i++)
		if(thegame->Neuralbot_Array[i] && thegame->Neuralbot_Array[i]->client == NULL)
			safe_bprintf (PRINT_HIGH, "*********bot %i client NULL********\n", i);
	/*
	x=2;
	y=3;
	z = pow(x,y);
		safe_bprintf (PRINT_HIGH, "z: %i\n", (int)z);

	safe_bprintf (PRINT_HIGH, "spinner_exponent * 1000: %i\n", (long)(thegame->spinner_exponent * 1000));

	*/
	//safe_bprintf (PRINT_HIGH,"sizeof(i): %i\n", sizeof(i));

	//safe_bprintf (PRINT_HIGH,"clock(): %i\n", (long)clock());

	
	//PrintTipOfTheDay(self, true);
	/*
	safe_bprintf (PRINT_HIGH,"searching for dynamic neurons..\n");
	for(i=0; i<60; i++)
	{
		if(self->client->botinfo.mygame->Neuralbot_Array[0].itsNet.Neuron_Array[i][0].type == N_DYNAMIC)
			safe_bprintf (PRINT_HIGH,"%i\n",i);
	}
	
	
	safe_bprintf (PRINT_HIGH,"searching for stl neurons..\n");
	for(i=0; i<60; i++)
	{
		if(self->client->botinfo.mygame->Neuralbot_Array[0].itsNet.Neuron_Array[i][0].stl == N_STL)
			if(self->client->botinfo.mygame->Neuralbot_Array[0].itsNet.Neuron_Array[i][0].excitatory == 1)
				safe_bprintf (PRINT_HIGH,"+ %i\n",i);
			else
				safe_bprintf (PRINT_HIGH,"- %i\n",i);
	}	
	
	*/


	
	//for(b=0; b<self->client->botinfo.mygame->NUM_BOTS; b++)
	//{
								   
		/*for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			//if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				//continue;
			self->client->botinfo.mygame->Neuralbot_Array[b].edictpointer->client->pers.inventory[i] = 1;
		}*/
		//for(i=0; i<5; i++)
			//safe_bprintf (PRINT_HIGH, "%i \n", thegame->Neuralbot_Array[0].itsNet.num_neurons_in_layer[i]);
		
	//}
	
	//safe_bprintf (PRINT_HIGH,"totalstlweightchange for bot 0 *100 :%i\n", (long)(self->client->botinfo.mygame->Neuralbot_Array[0].itsNet.totalstlweightchange*100) );
	//safe_bprintf (PRINT_HIGH,"stl_dweight*1000  :%i\n", (long)(self->client->botinfo.mygame->stl_dweight*100) );
	
	//safe_bprintf (PRINT_HIGH,"test\n");	
	/*
	for(i=0; i<60; i++)
		wsum += self->client->iminfo.net.Synapse_Array[i][12][1].weight;

	safe_bprintf (PRINT_HIGH,"weightsum to neuron 12*1000 :%i\n", (long)(wsum*1000));
	*/


	/*




	int i, s, d, l;// s = source neuron, d = dest neuron, l = layer
	long c = 0;
	
	long printnum;
	float weightx1000;

	weightx1000 = self->client->botinfo.mygame->
		Neuralbot_Array[3].itsNet.Synapse_Array[37][0][0].weight * 1000;

	printnum = weightx1000;

	safe_bprintf (PRINT_HIGH,"weight of bot3 synapse37: %i\n", (long)printnum);
	

	for (i=0; i<self->client->botinfo.mygame->NUM_BOTS; i++ )
	{
											// Step through:
		for ( l=0; l< self->client->botinfo.mygame->SYNAPSE_LAYERS; l++ ) // synapse layers
			for ( s=0; s< self->client->botinfo.mygame->NUM_NEURONS_IN_LAYER; s++ ) // source neurons
				for ( d=0; d< self->client->botinfo.mygame->NUM_NEURONS_IN_LAYER; d++ ) // destination neurons
				{
				/////// check if weight is zero /////////////////////
				  if ( self->client->botinfo.mygame->Neuralbot_Array[i]
					 .itsNet.Synapse_Array[s][d][l].weight == 0)
						c++;
				}
	}

	safe_bprintf (PRINT_HIGH,"number of synapses with weight zero: %i\n", (long)c);
	*/
}


void Cmd_Longer_f()
{
	// increases the max period between Evolve()s by 5 secs

	thegame->evolve_period_ceiling += 5;


	safe_bprintf (PRINT_HIGH,"Evolve period ceiling: %i\n", (int)thegame->evolve_period_ceiling);
	/*
	printnum = self->client->botinfo.mygame->evolve_time;
	safe_bprintf (PRINT_HIGH,"next evolve time: %i\n", (long)printnum);

	safe_bprintf (PRINT_HIGH,"next evolve time: %i\n", (long)level.time);
	*/

}




void Cmd_Shorter_f()
{
	// shortens the max period between Evolve()s by 5 secs

	long printnum;

	thegame->evolve_period_ceiling -= 5;

	if (thegame->evolve_period_ceiling < 1)
		thegame->evolve_period_ceiling = 1; // downwards cap at 1 second
	

	safe_bprintf (PRINT_HIGH,"Evolve period ceiling: %i\n", (long)thegame->evolve_period_ceiling);
}



void Cmd_Botinfo_f()
{
	int printnum;


	safe_bprintf (PRINT_HIGH,"****Botinfo****\n");

	safe_bprintf (PRINT_HIGH,"General game parameters:\n");
	safe_bprintf (PRINT_HIGH,"		num_bots: %i\n", thegame->NUM_BOTS);
	safe_bprintf (PRINT_HIGH,"		freeweapon: %i\n", thegame->freeweapon);
	safe_bprintf (PRINT_HIGH,"		telefrag_points: %i\n", (int)thegame->telefrag_points);
	safe_bprintf (PRINT_HIGH,"		stagger_weapon_changes: %i\n", (int)thegame->stagger_weapon_changes);

	safe_bprintf (PRINT_HIGH,"Genetic algorithm parameters:\n");
	safe_bprintf (PRINT_HIGH,"		evolve_period_ceiling: %i\n", (int)thegame->evolve_period_ceiling);
	safe_bprintf (PRINT_HIGH,"		chance_mutate: %i\n", (int)thegame->CHANCE_MUTATE);
	safe_bprintf (PRINT_HIGH,"		continuous prob_mutatex1000000: %i\n", (int)(thegame->CON_PROB_MUTATE*1000000));
	safe_bprintf (PRINT_HIGH,"		max_num_mutate: %i\n", (int)thegame->MAX_NUM_MUTATE);

	safe_bprintf (PRINT_HIGH,"		neuron_chance_mutate: %i\n", (int)thegame->NEURON_CHANCE_MUTATE);
	safe_bprintf (PRINT_HIGH,"		neuron_max_num_mutate: %i\n", (int)thegame->NEURON_MAX_NUM_MUTATE);

	safe_bprintf (PRINT_HIGH,"		crossover type: %i\n", (int)thegame->crossover_type);
	safe_bprintf (PRINT_HIGH,"		parentselection type: %i\n", (int)thegame->parentselection_type);
	safe_bprintf (PRINT_HIGH,"		frag_score: %i\n", (int)thegame->fragscore);
	safe_bprintf (PRINT_HIGH,"		item_score: %i\n", (int)thegame->itemscore);
	safe_bprintf (PRINT_HIGH,"		explorescore: %i\n", (int)thegame->explorescore);
	safe_bprintf (PRINT_HIGH,"		quickevolve: %i\n", (int)thegame->quickevolve);
	safe_bprintf (PRINT_HIGH,"		quickevolve_threshold: %i\n", (int)thegame->quickevolve_threshold);

	safe_bprintf (PRINT_HIGH,"NN structure GA parameters:\n");
	safe_bprintf (PRINT_HIGH,"		structure_evolve_period: %i\n", (int)thegame->structure_evolve_period);
	safe_bprintf (PRINT_HIGH,"		structure_chance_mutate: %i\n", (int)thegame->STRUCTURE_CHANCE_MUTATE);
	safe_bprintf (PRINT_HIGH,"		structure_max_num_mutate: %i\n", (int)thegame->STRUCTURE_MAX_NUM_MUTATE);
	safe_bprintf (PRINT_HIGH,"		initial_num_nosynapses: %i\n",(int)thegame->initial_num_nosynapses);
	
	
	safe_bprintf (PRINT_HIGH,"Neural network parameters:\n");
	safe_bprintf (PRINT_HIGH,"		num_neurons_in_layer: %i\n", (int)thegame->NUM_NEURONS_IN_LAYER);
	safe_bprintf (PRINT_HIGH,"		synapse_layers: %i\n", thegame->SYNAPSE_LAYERS);
	safe_bprintf (PRINT_HIGH,"		hidden neurons / layer: %i\n", (int)thegame->Neuralbot_Array[0]->client->botinfo.itsNet.HIDDEN_NEURONS);
	safe_bprintf (PRINT_HIGH,"		max_weight_x100: %i\n", (int)(thegame->Neuralbot_Array[0]->client->botinfo.itsNet.MAX_WEIGHT * 100));
	safe_bprintf (PRINT_HIGH,"		max_charge_x100: %i\n", (int)(thegame->Neuralbot_Array[0]->client->botinfo.itsNet.MAX_CHARGE * 100));
	safe_bprintf (PRINT_HIGH,"		threshold_x100: %i\n", (int)(thegame->Neuralbot_Array[0]->client->botinfo.itsNet.THRESHOLD * 100));
	safe_bprintf (PRINT_HIGH,"		discharge_amount_x100: %i\n", (int)(thegame->Neuralbot_Array[0]->client->botinfo.itsNet.DISCHARGE_AMOUNT * 100));

	safe_bprintf (PRINT_HIGH,"kinda irrelevant parameters:\n");
	safe_bprintf (PRINT_HIGH,"		messages: %i\n", (int)thegame->obits);

	safe_bprintf (PRINT_HIGH,"interesting stuff:\n");
	safe_bprintf (PRINT_HIGH,"		generation: %i\n", (int)thegame->generation);
	safe_bprintf (PRINT_HIGH,"		structure_generation: %i\n", (int)thegame->structure_generation);
	safe_bprintf (PRINT_HIGH,"		current number of 'NOSYNAPSES': %i\n", (long)get_num_nosynapses(thegame));
	safe_bprintf (PRINT_HIGH,"		total used/'nosynapse' synapses: %i\n", (long)get_num_synapses(thegame));
	safe_bprintf (PRINT_HIGH,"		current number of dynamic neurons: %i\n", (long)get_num_dynamic_neurons(thegame));
	safe_bprintf (PRINT_HIGH,"		sumfitnesstarget: %i\n", (long)thegame->sumfitnesstarget);
	safe_bprintf (PRINT_HIGH,"		mutant successes: %i\n", (long)thegame->mutant_successes);
	safe_bprintf (PRINT_HIGH,"		mutant failures: %i\n", (long)thegame->mutant_failures);
	safe_bprintf (PRINT_HIGH,"		mutant success percent: %i\n", (long)getPercentSuccess(thegame));




}

long get_num_nosynapses()
{
	int i, s, x, z;// s = source neuron, d = dest neuron, l = layer
	long c = 0;

	int neuron_layers = thegame->SYNAPSE_LAYERS + 1;
	int num_neurons_in_layer = thegame->NUM_NEURONS_IN_LAYER;

	int num_neurons_this_layer;
	int num_neurons_last_layer;

	NeuralNet*	net = NULL;

	for (i=0; i<thegame->NUM_BOTS; i++ )
	{
		net = &thegame->Neuralbot_Array[i]->client->botinfo.itsNet;

			

		for (z=0; z< neuron_layers; z++)//step through neuron layers
		{
			//NOTE: update 
			num_neurons_last_layer = num_neurons_this_layer;//move down a layer


			if((z == (neuron_layers-1))	//if this is the last layer
				|| (z == 0))			// or the first layer
				num_neurons_this_layer = num_neurons_in_layer;
			else
				num_neurons_this_layer = net->HIDDEN_NEURONS;

			for (x=0; x< num_neurons_this_layer; x++)// step along row of neurons
			{


					if(z) // if this is not the first neuron layer
					{	

						for ( s=0; s<num_neurons_last_layer; s++ )// step through source neurons
						{

							if(net->Synapse_Array[s][x][z-1].weight == NOSYNAPSE)
								c++;

						}
											
					} // finish " if ( not first layer )"
			}
		}
	}
/*
	for (i=0; i<thegame->NUM_BOTS; i++ )
	{
		net = &thegame->Neuralbot_Array[i].itsNet;

											// Step through:
		for ( l=0; l<thegame->SYNAPSE_LAYERS; l++ ) // synapse layers
		{
			///// find number of neurons in current source layer /////
			if((l == (neuron_layers-1))	//if this is the last layer
				|| (l == 0))			// or the first layer
				num_neurons_source_layer = num_neurons_in_layer;
			else
				num_neurons_source_layer = net->HIDDEN_NEURONS;

			///// find number of neurons in current destination layer /////
			if(((l+1) == (neuron_layers-1))	//if this is the last layer
				|| ((l+1) == 0))			// or the first layer
				num_neurons_dest_layer = num_neurons_in_layer;
			else
				num_neurons_dest_layer = net->HIDDEN_NEURONS;



			for ( s=0; s< num_neurons_source_layer; s++ ) // source neurons
				for ( d=0; d< num_neurons_dest_layer; d++ ) // step through destination neurons
				{
					if(net->Synapse_Array[s][d][l].weight == NOSYNAPSE)
						c++;	// count number of 'NOSYNAPSESs'
				}
		}
	
	}
	*/

	return c;
}


/*=============================================================================================
get_num_synapses
----------------
returns the total number of synapses used for all bots- counting NOSYNAPSE synapses.  
This function does not count synapses connected to unused hidden layer neurons.

  NOTE: i am assuming all the bots have the same number of hidden neurons per layer 
  as the first bot.
=============================================================================================*/

long get_num_synapses()
{
	long c;
	int hidden_neurons = thegame->Neuralbot_Array[0]->client->botinfo.itsNet.HIDDEN_NEURONS;

	if(thegame->SYNAPSE_LAYERS >= 2)
		c = (2*(thegame->NUM_NEURONS_IN_LAYER * hidden_neurons)) //top and bottom synapse layers
				+ ((thegame->SYNAPSE_LAYERS - 2) * (hidden_neurons * hidden_neurons));//middle synapse layers
	else							//else 1 synapse layer
		c = (thegame->NUM_NEURONS_IN_LAYER * thegame->NUM_NEURONS_IN_LAYER);
				//no room for hidden layer neurons

	c *= thegame->NUM_BOTS;
 
	return c;
}

/*============================================================================================
get_num_dynamic_neurons
-----------------------
returns the number of neurons with type N_DYNAMIC. These are neurons whose synapse weights can
be dynamically changed.  Just counts the neurons within the bot of the NN that is used.  (like not
all 5 layers if only 3 are used).  Also doesn't look at hidden neurons.
============================================================================================*/
long get_num_dynamic_neurons()
{
	int b;
	int x; 
	int l;
	long c = 0;

	int neuron_layers = thegame->SYNAPSE_LAYERS + 1;


	NeuralNet*	net = NULL;

	for (b=0; b<thegame->NUM_BOTS; b++ )
	{
		net = &thegame->Neuralbot_Array[b]->client->botinfo.itsNet;

		for (l=0; l< neuron_layers; l++)//step through neuron layers
			for (x=0; x< net->num_neurons_in_layer[l]; x++)// step along row of neurons
				if(net->Neuron_Array[x][l].thinkfuncnum != 14)
					c++;		
	}

	return c;

}

/*===========================================================================================
Cmd_Purge_f
-----------
randomize all the bots' synapse weights
===========================================================================================*/
void Cmd_Purge_f()
{

	int i, s, d, l;// s = source neuron, d = dest neuron, l = layer
	long c = 0;

	for (i=0; i<thegame->NUM_BOTS; i++ )
	{
											// Step through:
		for ( l=0; l< thegame->SYNAPSE_LAYERS; l++ ) // synapse layers
			for ( s=0; s< thegame->NUM_NEURONS_IN_LAYER; s++ ) // source neurons
				for ( d=0; d< thegame->NUM_NEURONS_IN_LAYER; d++ ) // destination neurons
				{
					/////// randomize weight /////////////////////
					thegame->Neuralbot_Array[i]->client->botinfo.itsNet.Synapse_Array[s][d][l].weight = -1 + (random()*2);

					c++;
				}
	}

	safe_bprintf (PRINT_HIGH,"*** PURGING BRAIN ***\n\n");
	safe_bprintf (PRINT_HIGH,"number of synapses randomized: %i\n", (long)c);

}

/*===========================================================================================
Cmd_Purge_f
-----------
set all the bots' synapse weights to zero
===========================================================================================*/
void Cmd_Zap_f()
{
	int i, s, d, l;// s = source neuron, d = dest neuron, l = layer
	long c = 0;

	for (i=0; i<thegame->NUM_BOTS; i++ )
	{
											// Step through:
		for ( l=0; l< thegame->SYNAPSE_LAYERS; l++ ) // synapse layers
			for ( s=0; s< thegame->NUM_NEURONS_IN_LAYER; s++ ) // source neurons
				for ( d=0; d< thegame->NUM_NEURONS_IN_LAYER; d++ ) // destination neurons
				{
					/////// zero-ize weight /////////////////////
					thegame->Neuralbot_Array[i]->client->botinfo.itsNet.Synapse_Array[s][d][l].weight = 0;

					c++;
				}
	}
	
	safe_bprintf (PRINT_HIGH,"*** ZAPPING BRAIN ***\n\n");
	safe_bprintf (PRINT_HIGH,"number of synapses zer0-ised: %i\n", (long)c);

}

//NOTE: this fuction may be out of date now in terms of the input+output neuron numbers it uses
void Cmd_Hardwire_f()
{
	int i;

	safe_bprintf (PRINT_HIGH,"hardwiring disabled sorry\n");

	return;	// NOTE: hardwiring disabled!

	for (i=0; i<thegame->NUM_BOTS; i++ )
	{
		////// entity traceahead - below							
		thegame->Neuralbot_Array[i]
					 ->client->botinfo.itsNet.Synapse_Array[14][14][0].weight = 1;
		///// below - fire
		thegame->Neuralbot_Array[i]
					 ->client->botinfo.itsNet.Synapse_Array[14][10][1].weight = 1;

		///// opponent to left -> below:
		thegame->Neuralbot_Array[i]
					 ->client->botinfo.itsNet.Synapse_Array[22][22][0].weight = 1;
		thegame->Neuralbot_Array[i]
					 ->client->botinfo.itsNet.Synapse_Array[26][22][0].weight = 1;

		///// below-> turnleft
		thegame->Neuralbot_Array[i]
					 ->client->botinfo.itsNet.Synapse_Array[22][24][1].weight = 1;

		///// opponent to right -> below:
		thegame->Neuralbot_Array[i]
					 ->client->botinfo.itsNet.Synapse_Array[23][23][0].weight = 1;
		thegame->Neuralbot_Array[i]
					 ->client->botinfo.itsNet.Synapse_Array[27][23][0].weight = 1;

		///// below-> turnright
		thegame->Neuralbot_Array[i]
					 ->client->botinfo.itsNet.Synapse_Array[23][25][1].weight = 1;

	}

	safe_bprintf (PRINT_HIGH,"				hardwiring bot				\n");
}


void Cmd_Freeweapon_f (qboolean svcmd)
{
	if(svcmd)
		thegame->freeweapon = atoi(gi.argv(2));//set freeweapon to the third word typed into the console
	else
		thegame->freeweapon = atoi(gi.argv(1));//set freeweapon to the third word typed into the console

	if(thegame->freeweapon == -1)
		safe_bprintf(PRINT_HIGH, "freeweapon set to: -1 (no free weapons) \n");
	else if(thegame->freeweapon == 10)
		safe_bprintf(PRINT_HIGH, "freeweapon set to: 10 (all weapons free) \n");
	else
		safe_bprintf(PRINT_HIGH, "freeweapon set to: %i \n", thegame->freeweapon);

	//giveFreeWeapon(self);
}


void Cmd_Slower_f()
{

	float float_recip_afactor = 1/acceleration_factor;	// reciprocal of acceleration factor
	int	recip_afactor = ceil(float_recip_afactor);
	float new_recip_afactor;

	if( acceleration_factor > 1)
	{
		acceleration_factor -= 1;

		safe_bprintf (PRINT_HIGH,"Gametime acceleration factor decreased to: %i\n", (int)acceleration_factor);
	}
	else
	{
		recip_afactor++;
		new_recip_afactor = recip_afactor;
	
		acceleration_factor = 1 / new_recip_afactor;
		safe_bprintf (PRINT_HIGH,"Gametime acceleration factor decreased to: 1/%i\n", (int)recip_afactor);
	}

	
}

void Cmd_Faster_f()
{


	float float_recip_afactor = 1/acceleration_factor;	// reciprocal of acceleration factor
	int	recip_afactor = ceil(float_recip_afactor);
	float new_recip_afactor;

	if( acceleration_factor >= 1) // if we can just add one to accelration_factor
	{
		acceleration_factor += 1;

		safe_bprintf (PRINT_HIGH,"Gametime acceleration factor increased to: %i\n", (int)acceleration_factor);
	}
	else
	{
		recip_afactor--;
		new_recip_afactor = recip_afactor;
	
		acceleration_factor = 1 / new_recip_afactor;
		safe_bprintf (PRINT_HIGH,"Gametime acceleration factor increased to: 1/%i\n", (int)recip_afactor);
	}
}

void Cmd_Lotsfaster_f()
{
	int i;

	for(i=0; i<10; i++)
		Cmd_Faster_f();
}

void Cmd_Lotsslower_f()
{
	int i;

	for(i=0; i<10; i++)
		Cmd_Slower_f();
}
void Cmd_Normal_f()
{
	acceleration_factor = 1; 
	safe_bprintf (PRINT_HIGH,"Gametime acceleration factor set to 1.\n");
}

void Cmd_Cruise_f()
{
	acceleration_factor = cruise_factor; 
	safe_bprintf (PRINT_HIGH,"Gametime acceleration factor set to %i.\n", (long)acceleration_factor);
}

void Cmd_Obits_f()
{
	if( thegame->obits == false)
	{	
		thegame->obits = true;
		safe_bprintf (PRINT_HIGH,"bot obituaries on\n");
	}
	else
	{
		thegame->obits = false;
		safe_bprintf (PRINT_HIGH,"bot obituaries off\n");
	}
}

qboolean bot_cmdcheck()
{
	if(thegame == NULL)
	{
		safe_bprintf (PRINT_HIGH,"start a bot game first(type addbot)\n");
		return false;
	}
	else
		return true;
}

void Cmd_Enemytrail_f()
{
	if( thegame->enemy_trail == false)
	{
	
		thegame->enemy_trail = true;
			safe_bprintf (PRINT_HIGH,"enemy BFG trails on\n");
	}
	else
	{
		thegame->enemy_trail = false;
		safe_bprintf (PRINT_HIGH,"enemy BFG trails off\n");
	}
}



void Cmd_Itemtrail_f()
{
	if( thegame->item_trail == false)
	{
	
		thegame->item_trail = true;
			safe_bprintf (PRINT_HIGH,"item BFG trails on\n");
	}
	else
	{
		thegame->item_trail = false;
		safe_bprintf (PRINT_HIGH,"item BFG trails off\n");
	}
}

void Cmd_Invisible_f(edict_t* self)
{
	if(self->client->botinfo.invisible == true)
	{
		self->client->botinfo.invisible = false;
		safe_bprintf (PRINT_HIGH,"'invisible to bots' mode OFF\n");
	}
	else
	{
		self->client->botinfo.invisible = true;
		safe_bprintf (PRINT_HIGH,"'invisible to bots' mode ON\n");
	}
}


long getPercentSuccess()
{
	float per;


	if((thegame->mutant_successes + thegame->mutant_failures) == 0)
		return 0;	//avoid divide by zero


	per = ((float)thegame->mutant_successes / (float)(thegame->mutant_successes + thegame->mutant_failures) ) * 100;
		//percent successes = successes / 'total successes + failures' all times 100


	return (long)per;
}


void PrintIntro(edict_t* self, qboolean centerprint)
{

	char *message;
	safe_bprintf(PRINT_HIGH, "\n\n\nin printintro\n\n\n");



	message = "Neuralbot             version 0.6\n www.botepidemic.com/neuralbot\n coded by Ono-Sendai\n onosendai@botepidemic.com\n type 'addbot' to spawn bots\n";



	if(centerprint)
		safe_centerprintf(self, message);
	else
		safe_cprintf(self, PRINT_HIGH, message);
}


void PrintTipOfTheDay(edict_t* self, qboolean centerprint)
{
	int num_tips = 8;
	int r = rand() % num_tips;
	char *message;



		switch (r)
		{
		case 0: message = "extra evolve info can be turned off/on \nwith the command 'obits'\n";
				break;
		case 1: message = "to see what the bots are locking on to, \ntype 'enemytrail' or 'itemtrail'\n";
				break;
		case 2: message = "for up to date info, check out \nwww.botepidemic.com/neuralbot'\n";
				break;
		case 3: message = "use the command 'savenn xxxxx' \nto save to xxxxx.dna\n";
				break;
		case 4: message = "'purge' randomizes all the bots' \nneural-net weights.\n";
				break;
		case 5: message = "use the command 'invisible' \nto become invisible to the bots\n";
				break;
		case 6: message = "use the commmand 'botinfo' \nto see lots of bot information";
				break;
		case 7: message = "use the command 'evolve' \nto manually initiate a run of the genetic-algorithm";
				break;
		}


	if(centerprint)
		safe_centerprintf(self, "Tip o' the session:\n %s", message);
	else
		safe_cprintf(self, PRINT_HIGH, "Tip o' the session:\n %s", message);
}



/*==================================================================================
giveFreeWeapon
--------------
gives one, all or no weapons to ent, depending on what the freeweapon variable is set to.
==================================================================================*/


void giveFreeWeapon(edict_t* ent)
{

	gitem_t* it;
	edict_t		*it_ent;
	char* name;
	int i;

	//safe_bprintf(PRINT_HIGH, "***giveFreeWeapon***\n");

	if(!thegame)	//players can spawn before starting a game
		return;

	if(thegame->freeweapon == -1)	//if no free weapons
		return;


	if(thegame->freeweapon == 10) //if all weapons free
	{
		//safe_bprintf(PRINT_HIGH,"giving all weapons\n");

		for (i=0 ; i<game.num_items ; i++)
			{
				it = itemlist + i;
				if (!it->pickup)
					continue;
				if (it->flags & IT_WEAPON)
					ent->client->pers.inventory[i] = 1;
			}

		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		return;

	}

	if(thegame->freeweapon == 1)	//don't need to give blaster
		return;

	if(thegame->freeweapon == 2)
		name = "shotgun";
	else if(thegame->freeweapon == 3)
		name = "super shotgun";
	else if(thegame->freeweapon == 4)
		name = "machinegun";
	else if(thegame->freeweapon == 5)
		name = "chaingun";
	else if(thegame->freeweapon == 6)
		name = "grenade launcher";
	else if(thegame->freeweapon == 7)
		name = "rocket launcher";
	else if(thegame->freeweapon == 8)
		name = "hyperblaster";
	else if(thegame->freeweapon == 9)
		name = "railgun";
	else if(thegame->freeweapon == 0)
		name = "bfg10k";
	else
		return;	//not a number that means anything
		

	it = FindItem (name);
	if (!it)
	{
			safe_cprintf (ent, PRINT_HIGH, "***\nunknown item: %s\n***", name);
			return;
	}

	if (!it->pickup)
	{
		safe_cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	//index = ITEM_INDEX(it);

	//spawn the item and make 'ent' touch it//
	it_ent = G_Spawn();
	it_ent->classname = it->classname;
	SpawnItem (it_ent, it);
	Touch_Item (it_ent, ent, NULL, NULL);
	if (it_ent->inuse)
		G_FreeEdict(it_ent);//if not an enduring powerup, free it//

}

void printAvAbsSynapseWeight(NeuralNet* net)
{
	float sum=0;
	int l;
	int s;
	int d;
	long c=0;


	for(l=0; l<net->SYNAPSE_LAYERS; l++)
		for(s=0; s<net->num_neurons_in_layer[l]; s++)
			for(d=0; d<net->num_neurons_in_layer[l+1]; d++)
			{
				//if(net->Synapse_Array[s][d][l].weight >= 0)
				//	sum += net->Synapse_Array[s][d][l].weight;
				//else
				//	sum += -1 * net->Synapse_Array[s][d][l].weight;
				sum += fabs(net->Synapse_Array[s][d][l].weight);
				c++;
			}
			

	sum /= (float)c;

	safe_bprintf(PRINT_HIGH, "av abs synapse weight(%i synapses): %i\n", c, (int)(sum * 1000));
}









