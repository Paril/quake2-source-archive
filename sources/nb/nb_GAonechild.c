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


qboolean choosechild(GAinfo* g);
qboolean wipechildNN(GAinfo* g);
qboolean mutateonechild(GAinfo* g);



/*============================================================================
One child-producing genetic algorithm.
-------------------------------------

The advantage of the one child GA is that the population is not homogeonised
so fast; hopefully the population will be a bit more diverse.  Well probably
not actually but it's worth a try.



//NOTE: update this!!!

============================================================================*/
void GAonechild()
{

	GAinfo g;	// information that all the functions this function calls can 
				// access.
	int i;
	
	g.sumfitness=0;
	g.NUM_SYNAPSES = thegame->NUM_NEURONS_IN_LAYER * thegame->NUM_NEURONS_IN_LAYER
		* thegame->SYNAPSE_LAYERS;	// work out total number of synapses in a neuralnet 

	//CopyBirthWeightsOver(&g);	//copy the bots' birth weights over to their normal weigths.
	
	calculateSumfitness(&g);

	switch(thegame->parentselection_type)
	{
		case PS_ROULETTE:		chooseparents_Roulette(&g);
			break;
		case PS_BEST:			chooseparents_Best(&g);
			break;
		case PS_RANK:			chooseparents_Rank(&g);
			break;
		default:			
			{
				safe_bprintf (PRINT_HIGH,"bad parentselection type\n");
				chooseparents_Roulette(&g);
			}
			break;
	}

	choosechild(&g);// different from normal; just choose 1 child instead of 2

	doMutationSuccesses(&g, true);

	generateParentChromosomes(&g);
	generateParentNeuronChromosomes(&g);

	switch(thegame->crossover_type)
	{
		case CO_LINEAR:		linear_crossover(&g);
			break;
		case CO_VERTICAL:	vertical_crossover(&g);
			break;
		default:			safe_bprintf (PRINT_HIGH,"bad crossover type\n");
			break;
	}

	linear_neuron_crossover(&g);	
		
	//discretemutate(&g);// now we have 2 strands of possibly mutated child DNA.
	//discreteneuronmutate(&g);
		//this stuff causes crashes

	continuousmutate(&g);

		
	
	wipechildNN(&g);// different from normal; only overwrite 1st child's synapse weights
	overwriteChild1Neurons(&g); //different from normal; this time just overwrite child 1's neurons

	//CopyDynOvertoBirthWeights(&g);//copy bots' normal weights over to their birth weights
	
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
		safe_bprintf (PRINT_HIGH,"child:\n");
		safe_bprintf (PRINT_HIGH,"	%s\n",thegame->Neuralbot_Array[g.child1]->client->pers.netname);
	}

	thegame->generation++; // increase generation(number of runs of the GA) counter

	safe_bprintf (PRINT_HIGH,"****Evolve Complete	(generation: %i )****\n", thegame->generation);

	///////autosave/////////
	if(thegame->autosave_period != 0)// 0 means autosave off
		if(thegame->generation%thegame->autosave_period == 0)	//every 'autosave_period' generations
			WriteNN ("autosave.dna");

	//////// save the sumfitness /////
	//DoSumfitnessHistory(thegame, g.sumfitness);

}




/*===========================================================================
onechild_crossover
------------------

Hmmmmn this is an interesting one.  What is the best way to combine 2
bits of parent DNA into one bit of child DNA?

For now I think i will not bother with embedding any knowledge in the 
crossover operation, i'll just mix around and combine the parent DNA 
with a vertical crossover.




Note: not used currently - normal functions do the job ok me thinks
===========================================================================*/

qboolean onechild_crossover(GAinfo* g)
{

	long i;
	int temp;
	long a,b;	// crossover break points
	int clayer;	//chromosome layer
	long nosinlayer = thegame->NUM_NEURONS_IN_LAYER*thegame->NUM_NEURONS_IN_LAYER;	// num synapses in layer
		// work out number of synapses per synapse layer


	/////// find where to break the chromosomes /////
	a = floor( random() * nosinlayer );
	b = floor( random() * nosinlayer );

	if ( a >= b ) // we want b to be the larger integer, so swap a and b.
	{
		temp = a;
		a = b;
		b = temp;
	}	

	// I have made it a 50% chance that the child chromosome will get parent1
	// DNA at the start of it's chromosome , and so on, so as not to be biased
	// towards one parent.  Not that it matters

	if(random() < 0.5)
		for(clayer=0; clayer<thegame->SYNAPSE_LAYERS; clayer++)
		{		// do the same crossover operation on each chromosome

			/// generate child1 DNA ///
			for ( i = 0; i < a; i++ )
				g->child1_chromosome[(clayer*nosinlayer)+i] = g->parent1_chromosome[(clayer*nosinlayer)+i];

			for ( i = a; i < b; i++ )
				g->child1_chromosome[(clayer*nosinlayer)+i] = g->parent2_chromosome[(clayer*nosinlayer)+i];
			
			for ( i = b; i < nosinlayer; i++ )
				g->child1_chromosome[(clayer*nosinlayer)+i] = g->parent1_chromosome[(clayer*nosinlayer)+i];
		}
	else
		for(clayer=0; clayer<thegame->SYNAPSE_LAYERS; clayer++)
		{		// do the same crossover operation on each chromosome

			/// generate child1 DNA ///
			for ( i = 0; i < a; i++ )
				g->child1_chromosome[(clayer*nosinlayer)+i] = g->parent2_chromosome[(clayer*nosinlayer)+i];

			for ( i = a; i < b; i++ )
				g->child1_chromosome[(clayer*nosinlayer)+i] = g->parent1_chromosome[(clayer*nosinlayer)+i];
			
			for ( i = b; i < nosinlayer; i++ )
				g->child1_chromosome[(clayer*nosinlayer)+i] = g->parent2_chromosome[(clayer*nosinlayer)+i];
		}	
		
		return true;
				
}











/*===========================================================================
choosechild
-----------

===========================================================================*/
qboolean choosechild(GAinfo* g)
{

	float worstsuccess = 10000;
	qboolean candidate_array[MAX_BOTS];
	int i;

	for(i=0;i<MAX_BOTS;i++)
		candidate_array[i] = false; // clear candidate array


	// find the lowest non-parent success //
	for (i=0; i < thegame->NUM_BOTS; i++)
	{
		if ((thegame->Neuralbot_Array[i]->client->botinfo.success < worstsuccess) && (i != g->parent1) && (i != g->parent2))
			worstsuccess = thegame->Neuralbot_Array[i]->client->botinfo.success; //find the worst success	
	}


	////////// find all bots that are children candidates ///////
	//// find all the bots with 'worstsuccess' fitness and that are not parents////
	for(i=0; i<thegame->NUM_BOTS; i++)
		if( (thegame->Neuralbot_Array[i]->client->botinfo.success == worstsuccess) && (i != g->parent1) && (i != g->parent2))
			candidate_array[i] = true;	//bot is a child candidate

		/// make child1 a random bot out of all the candidates ////
	g->child1 = choose_bot(candidate_array);

	return true;
}











/*===========================================================================
wipechildNN
-----------

===========================================================================*/
qboolean wipechildNN(GAinfo* g)
{
	long c=0;
	int m,n,p;
												// Step through:
	for ( m=0; m< thegame->SYNAPSE_LAYERS; m++ ) // synapse layers
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // source neurons
			for ( p=0; p< thegame->NUM_NEURONS_IN_LAYER; p++ ) // destination neurons
			{
				if ( thegame->Neuralbot_Array[g->child1]->client->botinfo.itsNet.Synapse_Array[n][p][m].weight != NOSYNAPSE ) 		
					thegame->Neuralbot_Array[g->child1]->client->botinfo.itsNet
					.Synapse_Array[n][p][m].weight = g->child1_chromosome[c];
			c++;
			}
		return true;
}




















