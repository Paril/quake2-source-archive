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


/*==========================================================================
Structure genetic algorithm
---------------------------

This GA runs in parallel with the weight-evolving GA.  This GA evolves a good
structure for the NN - it decides in what places a synapse does and does not 
exist.

==========================================================================




/*==========================================================================================
Struct_evaluate_success
------------------------------------------------------------------------------------------
evaluates the success of all the bots and assigns the individual success 
values to the Neuralbot member variable success
===========================================================================*/
void Struct_evaluate_success(edict_t* bot)
{

	nb_info*	binfo = &bot->client->botinfo;	//botinfo


	bot->client->botinfo.success = ( thegame->fragscore * binfo->structure_fragslastperiod)
		+	(thegame->itemscore * binfo->structure_itemsgot)
		+	(thegame->damagescore * binfo->structure_damageinflicted)
		+	(thegame->explorescore * binfo->structure_explorenesssofar);


	if(bot->client->botinfo.success < 0)//might have had negative frags
		bot->client->botinfo.success = 0;

	if( thegame->obits == true)
	safe_bprintf (PRINT_HIGH,"	%s's fitness: %i\n", bot->client->pers.netname, (long)bot->client->botinfo.success);

	
}



/*==========================================================================================
Sruct_clear_botfrags
------------------------------------------------------------------------------------------
sets all the bot's frags in the last game period to 0 again
===========================================================================*/

void Struct_clear_botfrags(edict_t* bot)
{
		
	bot->client->botinfo.structure_fragslastperiod = 0;
	bot->client->botinfo.structure_itemsgot = 0;
	bot->client->botinfo.structure_damageinflicted = 0;
	bot->client->botinfo.structure_explorenesssofar = 0;

}


void Cmd_Struct_Evolve_f ()
{
	Struct_Evolve();
}

/*===========================================================================
Evolve
-----------------------------------------------------------------------------
===========================================================================*/
void Struct_Evolve ()
{

	int i	= 0;

	if(thegame->bot_spawn_index < thegame->NUM_BOTS)
	{
		safe_bprintf(PRINT_HIGH, "Structure Evolve: wait till all bots are spawned\n");
		return;
	}


	if(thegame->obits == true)
		safe_bprintf (PRINT_HIGH,"**** NN Structure Evolve ****\n");



	for(i=0; i<thegame->NUM_BOTS; i++)
		Struct_evaluate_success(thegame->Neuralbot_Array[i]);

	for(i=0; i<thegame->NUM_BOTS; i++)
		Struct_clear_botfrags(thegame->Neuralbot_Array[i]);

	
	//// go ahead with the rest of the evolutionary proccess ////////
	Struct_DNAswapandmutate(thegame);

}











/*
==========================================================================================
DNAswapandmutate
------------------------------------------------------------------------------------------

==========================================================================================
*/
void Struct_DNAswapandmutate()
{

	int i;
	GAinfo g;	// information that all the functions this function calls can 
				// access.
	
	calculateSumfitness(&g);

	g.NUM_SYNAPSES = thegame->NUM_NEURONS_IN_LAYER * thegame->NUM_NEURONS_IN_LAYER
		* thegame->SYNAPSE_LAYERS;	// work out total number of synapses in a neuralnet 


	chooseparents_Rank(&g); //Can use this ordinary function as the fitness values for the bots
							//have already been calculated the structure way, and stored in the bot
							//member variable 'success'

	struct_generateParentChromosomes(&g);//generate some binary DNA

	switch(thegame->crossover_type)
	{
		case CO_LINEAR:		linear_crossover(&g);
			break;
		case CO_VERTICAL:	vertical_crossover(&g);
			break;
		default:			safe_bprintf (PRINT_HIGH,"bad crossover type\n");
			break;
	}
	
	struct_mutate(&g);// now we have 2 strands of possibly mutated child DNA.

	choosechildren_Worst(&g);

	struct_overwriteChildNNs(&g);

	///// print the fitness of all the bots /////
	if( thegame->obits == true)
	{
		for(i=0; i<thegame->NUM_BOTS; i++)
		{
			if(i == g.parent1 || i == g.parent2)//parents get a special print so they are easy to see
			{
				safe_bprintf (PRINT_HIGH,"					");
				safe_bprintf (PRINT_HIGH,"%s's fitness: %i", thegame->Neuralbot_Array[i]->client->pers.netname, (long)thegame->Neuralbot_Array[i]->client->botinfo.success);
				safe_bprintf (PRINT_HIGH,"				\n");
			}
			else
				safe_bprintf (PRINT_HIGH,"	%s's fitness: %i\n", thegame->Neuralbot_Array[i]->client->pers.netname, (long)thegame->Neuralbot_Array[i]->client->botinfo.success);
		}
	}

	if(thegame->obits == true)
	{
		safe_bprintf (PRINT_HIGH,"parents:\n");
		safe_bprintf (PRINT_HIGH,"	%s\n", thegame->Neuralbot_Array[g.parent1]->client->pers.netname);
		safe_bprintf (PRINT_HIGH,"	%s\n", thegame->Neuralbot_Array[g.parent2]->client->pers.netname);
		safe_bprintf (PRINT_HIGH,"children:\n");
		safe_bprintf (PRINT_HIGH,"	%s\n",thegame->Neuralbot_Array[g.child1]->client->pers.netname);
		safe_bprintf (PRINT_HIGH,"	%s\n",thegame->Neuralbot_Array[g.child2]->client->pers.netname);
	}


	thegame->structure_generation++; // increase generation counter... just for fun


	safe_bprintf (PRINT_HIGH,"****NN Structure Evolve Complete(generation: %i )****\n", thegame->structure_generation);


}
















/*======================================================================================
struct_generateParentChromosomes
--------------------------------
encode for each parent whether each synapse exists or not into a binary chromsome
======================================================================================*/

qboolean struct_generateParentChromosomes(GAinfo* g)
{
	long c,m,n,p;

	///////// generate parent1 chromosome ///////////////

	c=0;
												// Step through:
	for ( m=0; m< thegame->SYNAPSE_LAYERS; m++ ) // synapse layers
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // source neurons
			for ( p=0; p< thegame->NUM_NEURONS_IN_LAYER; p++ ) // destination neurons
			{
				if( thegame->Neuralbot_Array[g->parent1]->client->botinfo.itsNet.Synapse_Array[n][p][m].weight != NOSYNAPSE)
					g->parent1_chromosome[c] = 1; // 1 if a synapse exists
				else
					g->parent1_chromosome[c] = 0; // 0 if it doesn't
			 

			 c++;
			}
			

	///////////// generate parent2 chromosome //////////////////////


	c=0;
												// Step through:
	for ( m=0; m< thegame->SYNAPSE_LAYERS; m++ ) // synapse layers
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // source neurons
			for ( p=0; p< thegame->NUM_NEURONS_IN_LAYER; p++ ) // destination neurons
			{
				if( thegame->Neuralbot_Array[g->parent2]->client->botinfo.itsNet.Synapse_Array[n][p][m].weight != NOSYNAPSE)
					g->parent2_chromosome[c] = 1; // 1 if a synapse exists
				else
					g->parent2_chromosome[c] = 0; // 0 if it doesn't
			 
			 c++;
			}

	return true;
}









/*======================================================================================
struct_mutate
-------------
mutate the chromosomes that have just been made.
======================================================================================*/

qboolean struct_mutate(GAinfo* g)
{
	long numtomutate, synapsetomutate;

	if ( random() <= (thegame->STRUCTURE_CHANCE_MUTATE / 100) )
	{
		numtomutate = floor(random()*thegame->STRUCTURE_MAX_NUM_MUTATE);
		
		if ( numtomutate != 0 ) // if we have some synaspes to mutate
		{
			if(thegame->obits == true)
			safe_bprintf (PRINT_HIGH,"	child 1 is getting %i synapses mutated\n", numtomutate);

			while ( numtomutate > 0 )
			{
				synapsetomutate = floor( random() * g->NUM_SYNAPSES );

				//// mutate the synapse /////
				if( g->child1_chromosome[synapsetomutate] == 1) // negate the value
					g->child1_chromosome[synapsetomutate] = 0;
				else
					g->child1_chromosome[synapsetomutate] = 1;
				  
				numtomutate--;
			}
		}
	}


	///////// test to see if child2's DNA gets mutated ///////////

	if ( random() <= (thegame->STRUCTURE_CHANCE_MUTATE / 100) )
	{
			numtomutate = floor(random()*thegame->STRUCTURE_MAX_NUM_MUTATE);
		
		if ( numtomutate != 0 ) // if we have some synaspes to mutate
		{
			if(thegame->obits == true)
				safe_bprintf (PRINT_HIGH,"child 2 is getting %i synapses mutated\n", numtomutate);

			while ( numtomutate > 0 )
			{
				synapsetomutate = floor( random() * g->NUM_SYNAPSES );

						//// mutate the synapse /////
				if( g->child2_chromosome[synapsetomutate] == 1) // negate the value
					g->child2_chromosome[synapsetomutate] = 0;
				else
					g->child2_chromosome[synapsetomutate] = 1;

				numtomutate--;
			}
		}
	}
	return true;
}		





/*======================================================================================
struct_overwriteChildNNs
------------------------

this is basically the reverse of the synapse to chromosome transformation allready performed


when looking at a synapse that is going to be 'overwritten', there are 4 possibilties:

1.	Used to be a synapse, still should be.
2.	Used to be a synapse, now it's due to be 'deleted'
3.	Used to be deleted, still should be deleted.
4.	Used to be deleted, now a synapse should be created.

This is what the code is going to do:

1.	Nothing.
2.	change its weight to NOSYNAPSE
3.	Nothing.
4.	randomize its weight (between -1 and 1)


======================================================================================*/

qboolean struct_overwriteChildNNs(GAinfo* g)
{


	long c=0,m,n,p;
												// Step through:
	for ( m=0; m< thegame->SYNAPSE_LAYERS; m++ ) // synapse layers
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // source neurons
			for ( p=0; p< thegame->NUM_NEURONS_IN_LAYER; p++ ) // destination neurons
			{
				/// if there was no synapse, and now there is, randomise the weight
				if (( thegame->Neuralbot_Array[g->child1]->client->botinfo.itsNet.Synapse_Array[n][p][m].weight == NOSYNAPSE )
					&& (g->child1_chromosome[c] == 1))
					thegame->Neuralbot_Array[g->child1]->client->botinfo.itsNet.Synapse_Array[n][p][m].weight = -1 + (random()*2);

				//// if there is to be no synpase now /////
				else if( g->child1_chromosome[c] == 0) 
					thegame->Neuralbot_Array[g->child1]->client->botinfo.itsNet.Synapse_Array[n][p][m].weight = NOSYNAPSE;


			 c++;
			}
			



	// replace second suckiest bot with second child:

	c=0;
												// Step through:
	for ( m=0; m< thegame->SYNAPSE_LAYERS; m++ ) // synapse layers
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // source neurons
			for ( p=0; p< thegame->NUM_NEURONS_IN_LAYER; p++ ) // destination neurons
			{
			
				/// if there was no synapse, and now there is, randomise the weight
				if (( thegame->Neuralbot_Array[g->child2]->client->botinfo.itsNet.Synapse_Array[n][p][m].weight == NOSYNAPSE )
					&& (g->child2_chromosome[c] == 1))
					thegame->Neuralbot_Array[g->child2]->client->botinfo.itsNet.Synapse_Array[n][p][m].weight = -1 + (random()*2);



			if( g->child2_chromosome[c] == 0) // if there is to be no synpase now
				thegame->Neuralbot_Array[g->child2]->client->botinfo.itsNet.Synapse_Array[n][p][m].weight = NOSYNAPSE;

			 c++;
			}
	return true;
}
			



