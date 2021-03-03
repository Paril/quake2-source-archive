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



/*	// actually defined in nb_nnet.h -  here for reference
typedef struct
{
	Botgame* thegame;	

	int parent1;
	int parent2;
	int child1;
	int child2;
	float sumfitness;
	int NUM_SYNAPSES;


	float child1_chromosome[10000];
	float child2_chromosome[10000];
	float parent1_chromosome[10000];
	float parent2_chromosome[10000];

}GAinfo;
*/

/*

==========================================================================================
GAnormal
------------------------------------------------------------------------------------------
creates a GAinfo object thingey;
calculates NUM_SYNAPSES,
then lets various sub-functions do their thing, giving them access to the 
GA data via a pointer to the GAinfo object.

Also calls DoSumFitnessHistory
==========================================================================================
*/
void GAnormal()
{
	int i;
	GAinfo g;	// information that all the functions this function calls can 
				// access.
	

	g.NUM_SYNAPSES = thegame->NUM_NEURONS_IN_LAYER * thegame->NUM_NEURONS_IN_LAYER
		* thegame->SYNAPSE_LAYERS;	// work out total number of synapses in a neuralnet 


	//CopyBirthWeightsOver(&g);	//copy the bots' birth weights over to their normal weights.

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


	choosechildren_Worst(&g);

	doMutationSuccesses(&g, false);

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

	discretemutate(&g);
	discreteneuronmutate(&g);

	continuousmutate(&g); 


	overwriteChildNNs(&g);//overwrite the synapses
	overwriteChild1Neurons(&g);
	overwriteChild2Neurons(&g);

	//CopyDynOvertoBirthWeights(&g);//copy bots' normal weights over to their birth weights

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

	thegame->generation++; // increase generation(number of runs of the GA) counter

	safe_bprintf (PRINT_HIGH,"****Evolve Complete	(generation: %i )****\n", thegame->generation);

	///////autosave/////////
	if(thegame->autosave_period != 0)// 0 means autosave off
		if(thegame->generation%thegame->autosave_period == 0)	//every 'autosave_period' generations
			WriteNN ("autosave.dna");


	//////// save the sumfitness /////
	//DoSumfitnessHistory(g.sumfitness);
		//causing a crash, not needed anyway



	// The evolutionary proccess is complete! ... for now.

} 





qboolean calculateSumfitness(GAinfo* g)
{
	int i;

	g->sumfitness=0;

	for ( i=0; i < thegame->NUM_BOTS; i++ )
		g->sumfitness += thegame->Neuralbot_Array[i]->client->botinfo.success;	// find sumfitness


	if ( g->sumfitness < 0 )
		g->sumfitness = 0;	// don't want a negative sumfitness
									//Note: implement something later where bots with
									// neg fitness can't be picked?
	safe_bprintf (PRINT_HIGH,"sumfitness: %i\n", (long)g->sumfitness);

	return true;

}






/*=========================================================================
chooseparents_Roulette
----------------------
	
Roullette selection mechanism:

makes sumfitness the sum of all the bot's successes(fitnesses)

makes randomproportion a random proportion of sumfitness

moves randomproportion along the list of bots - 
the bot it reaches is chosen as a parent.

for the second parent the same search is performed,
but if the first parent is selected the search is done again

This is currently pretty ugly and hard to comprehend code.
If anyone has any better ideas on how do to this kind of stuff
I'd like to hear it.

============================================================================*/
qboolean chooseparents_Roulette(GAinfo* g)
{
	int i;
	int botpick;
	double randomproportion;

	double slicefraction[MAX_BOTS];//BOTNUMDEPENDENT
	double sumslicefraction;
	long loopcounter=0;

	// NOTE: take this out later
	g->parent1=666;
	g->parent2=666;



	if ( g->sumfitness == 0 )//special case -> so just pick randomly
	{
		if(thegame->obits == true)
			safe_bprintf (PRINT_HIGH,"choosing parents at random\n");

		///// choose parent one at random ///////////
		g->parent1 = rand() % thegame->NUM_BOTS;
		
		///// choose parent two at random ///////////
		while(1)
		{
			botpick = rand() % thegame->NUM_BOTS;
			if ( botpick != g->parent1 )	// don't choose 1st parent
			{	
				g->parent2 = botpick;
				break;				// we have our bot
			}
		}						// else try again
	
		return true;	//finished picking parents; return
	}




	////// else proceed as normal and use spinner selection method////////
	if(thegame->obits == true)
			safe_bprintf (PRINT_HIGH,"using spinner selection mechanism\n");

	sumslicefraction = 0;

	for ( i=0; i<thegame->NUM_BOTS; i++)
	{		
		slicefraction[i] = pow(thegame->Neuralbot_Array[i]->client->botinfo.success, thegame->spinner_exponent);
			//			 = 'thegame->Neuralbot_Array[i].success' to the power of 'thegame->spinner_exponent'
		
		sumslicefraction += slicefraction[i];
	}

	//for ( i=0; i<thegame->NUM_BOTS; i++)
	//		safe_bprintf (PRINT_HIGH,"slicefraction before normalization: %i\n",(long)slicefraction[i]);

	for ( i=0; i<thegame->NUM_BOTS; i++)
		slicefraction[i] /= sumslicefraction; //normalize so that the 'slicefractions' add up to one	
		
	randomproportion = random();


	//////////// find parent 1 //////////////
	for ( i=0; i < thegame->NUM_BOTS; i++)
	{
		randomproportion -= slicefraction[i];
		if(randomproportion <= 0)
		{
			g->parent1 = i;
			i=10000;	//exit the loop
		}
		else if(i == thegame->NUM_BOTS - 1)
			safe_bprintf (PRINT_HIGH,"parent1 did not get chosen with spinner!\n");

	}

	///////// find parent 2 ///////////
	// First we must check whether the remaining bots have sumfitness of 0.
	// If they do the second parent is picked randomly.
	if( (g->sumfitness - thegame->Neuralbot_Array[g->parent1]->client->botinfo.success) == 0 )
	{		// then pick second parent at random
		while(1)
		{
			botpick = rand() % thegame->NUM_BOTS ;
			if ( botpick != g->parent1 )	// don't choose 1st parent
			{	
				g->parent2 = botpick;
				break;				// we have our bot
			}
		}						// else try again
	}
	else	///////// else spin the selection spinner again ////////////
	{
		//recalculate slicefractions
		
		sumslicefraction = 0;

		for ( i=0; i<thegame->NUM_BOTS; i++)	
			if(i != g->parent1)
			{
				slicefraction[i] = pow(thegame->Neuralbot_Array[i]->client->botinfo.success, thegame->spinner_exponent);
					//			 = 'thegame->Neuralbot_Array[i].success' to the power of 'thegame->spinner_exponent'
			
				sumslicefraction += slicefraction[i];
			}

		//for ( i=0; i<thegame->NUM_BOTS; i++)
		//	safe_bprintf (PRINT_HIGH,"slicefraction before normalization: %i\n",(long)slicefraction[i]);

		for ( i=0; i<thegame->NUM_BOTS; i++)
			slicefraction[i] /= sumslicefraction; //normalize so that the 'slicefractions' add up to one	
			
		randomproportion = random();


		//////////// find parent 1 //////////////
		for ( i=0; i < thegame->NUM_BOTS; i++)
			if(i != g->parent1)	//ignore parent 1
			{
				randomproportion -= slicefraction[i];

				if(randomproportion <= 0)
				{
					g->parent2 = i;
					i=10000;	//exit the loop
				}
				else if(i == thegame->NUM_BOTS - 1)
					safe_bprintf (PRINT_HIGH,"parent2 did not get chosen with spinner!\n");
			}

	}







	if((g->parent1 == 666) || (g->parent2 == 666))
	{
		g->parent1=0;
		g->parent2=0;
		safe_bprintf (PRINT_HIGH,"big fuck up no parents were chosen\n");
	}


	return true;
}









/*=========================================================================
chooseparents_Best
------------------
makes g->parent1 and g->prarent2 equal to the two bots with the highest fitness

============================================================================*/
qboolean chooseparents_Best(GAinfo* g)
{
	int i,z;


	float bestfitness=0;
	qboolean candidate_array[MAX_BOTS];

	// NOTE: take this out later
	g->parent1=666;
	g->parent2=666;


	//// clear candidate_array ////
	for(z=0;z<MAX_BOTS;z++)
		candidate_array[z] = false;

	/// find hightst fitness /////
	for(i=0; i<thegame->NUM_BOTS; i++)
	{
		if(thegame->Neuralbot_Array[i]->client->botinfo.success > bestfitness)
			bestfitness = thegame->Neuralbot_Array[i]->client->botinfo.success;
	}

	/// fill candidate_array with bots who got that fitness ///
	for(i=0; i<thegame->NUM_BOTS; i++)
	{
		if(thegame->Neuralbot_Array[i]->client->botinfo.success == bestfitness)
			candidate_array[i] = true;
	}

	/// pick one bot out of all of those ///////////
	g->parent1 = choose_bot(candidate_array);

	bestfitness = 0; //reset and find next highest fitness
	//// clear candidate_array ////
	for(z=0;z<MAX_BOTS;z++)
		candidate_array[z] = false;

	for(i=0; i<thegame->NUM_BOTS; i++)
	{
		if((thegame->Neuralbot_Array[i]->client->botinfo.success > bestfitness) && (i != g->parent1))//ignore parent1
			bestfitness = thegame->Neuralbot_Array[i]->client->botinfo.success;
	}

	/// fill candidate_array with bots who got that fitness ///
	for(i=0; i<thegame->NUM_BOTS; i++)
	{
		if(thegame->Neuralbot_Array[i]->client->botinfo.success == bestfitness && (i != g->parent1))
			candidate_array[i] = true;
	}
	g->parent2 = choose_bot(candidate_array);


	if((g->parent1 == 666) || (g->parent2 == 666) || (g->parent1 == g->parent2))
	{
		g->parent1=0;
		g->parent2=1;
		safe_bprintf (PRINT_HIGH,"big fuck up no parents were chosen or parent1 == parent2\n");
	}


	return true;
}


/*==========================================================================================
chooseparents_Rank
------------------------
==========================================================================================*/
qboolean chooseparents_Rank(GAinfo* g)
{
	int bot[MAX_BOTS];//BOTNUMDEPENDANT
		//this contains the indices of the bots
	int i;
	int temp;
	qboolean notdone;
	int botpick;
	int bottoinsert;
	int p;
	int empty = 666;
	int rankedbots;
	float prob[MAX_BOTS];
	float sumprob;
	float randomproportion;
	int loop=0;

	for(i=0; i<MAX_BOTS; i++)
		bot[i] = empty;

	/// insert bot indices into the array in random order //////
	//It is neccesary to create an randomized array of bot indices here
	//in order to avoid biases in selection due to bots' positions
	//in the 'Neuralbot_Array'
	for(bottoinsert=0; bottoinsert<thegame->NUM_BOTS; bottoinsert++)
	{
		/// insert bottoinsert into a random empty array entry ///
		while(1)
		{
			p = rand() % thegame->NUM_BOTS; //get a random array entry

			if(bot[p] == empty)		//if array entry is random
			{
				bot[p] = bottoinsert;	//insert the bot index here
				break;
			}
		}//keep cycling until inserted
	}
	//NOTE: this inefficient?
	//takes around ?30? or so calls of rand() on average
	
	///bubblesort into descending order (bot[0] = index of best bot)/////	
	do
	{
		notdone = false;

		for(i=0; i<thegame->NUM_BOTS - 1; i++)
		{
			if(thegame->Neuralbot_Array[bot[i]]->client->botinfo.success < thegame->Neuralbot_Array[bot[i+1]]->client->botinfo.success)
			{		//if out of order then swap
				temp = bot[i]; 
				bot[i] = bot[i+1];
				bot[i+1] = temp;

				notdone = true;
			}
		}
	}
	while(notdone);
	/// finished bubblesort ///

	if(thegame->Neuralbot_Array[bot[0]]->client->botinfo.success == 0)//if not bots had any fitness
	{
		chooseparents_Roulette(g);//this will choose them randomly
		return true;
	}

	if(thegame->Neuralbot_Array[bot[1]]->client->botinfo.success == 0)//if only the first bot had any fitness
	{
		g->parent1 = bot[0];

		while(1) //choose the second bot randomly
		{
			botpick = rand() % thegame->NUM_BOTS;
			if ( botpick != g->parent1 )	// don't choose 1st parent
			{	
				g->parent2 = botpick;
				break;				// we have our bot
			}
		}

		return true;
	}

	safe_bprintf (PRINT_HIGH,  "------botindex(bot fitness):-----\n");
	for(i=0; i<thegame->NUM_BOTS; i++)
		safe_bprintf (PRINT_HIGH,"%i(%i) ", bot[i], (int)thegame->Neuralbot_Array[bot[i]]->client->botinfo.success);
	safe_bprintf (PRINT_HIGH,"\n---------------------------------\n");


	///// now select parent1 and parent2 with probablily dependant on rank ////

	///find number of bots that have a ranking/have any fitness ///
	rankedbots = 0;
	for(i=0; i<thegame->NUM_BOTS; i++)
		if(bot[i] != empty && thegame->Neuralbot_Array[bot[i]]->client->botinfo.success != 0)
			rankedbots++;

	// numrankedbots: 0, 1, 2, 3,  4,  5,  6,  7
	// total ranksum: 0, 1, 3, 6, 10, 15, 21, 28
	safe_bprintf (PRINT_HIGH,"rankedbots: %i \n", rankedbots);



	//say there are 3 bots that have a ranking(have any fitness at all). assume spinner exponent = 1
	//prob of best bot (i=0) = (3 - 0) ^ spinner exponent = 3
	//prob of 2nd  bot (i=1) = (3 - 1) ^ spinner exponent = 2
	//prob of 3rd  bot (i=2) = (3 - 2) ^ spinner exponent = 1
	//
	//then all the probs are normalized so that they add up to 1
	//sumprob = 6
	//prob of best bot = 3/6 = 1/2
	//prob of 2nd  bot = 2/6 = 1/3
	//prob of 3rd  bot = 1/6 = 1/6

	sumprob = 0;
	for(i=0; i<rankedbots; i++)
	{
		prob[i] = pow((float)(rankedbots - i), thegame->spinner_exponent);
		sumprob += prob[i];
	}

	for(i=0; i<rankedbots; i++)
		prob[i] /= sumprob; //normalize so that sumprob = 1



	//f=0;
	//for(i=0; i<rankedbots; i++)
	//	f += prob[i];
	//safe_bprintf (PRINT_HIGH,"normalised sumprob: %i \n",(long)(f * 1000));


	//////////// find parent 1 //////////////
	randomproportion = random();
	for ( i=0; i < rankedbots; i++)
	{
		randomproportion -= prob[i];

		if(randomproportion <= 0)
		{
			g->parent1 = bot[i];
			i=10000;	//exit the loop
		}
		else if(i == rankedbots - 1)
		{
			safe_bprintf (PRINT_HIGH,"parent1 did not get chosen with rank spinner!\n");
			return true;
		}

	}
	//////////// find parent 2 //////////////

	sumprob = 0;
	for(i=0; i<rankedbots; i++)
		if(bot[i] != g->parent1)//ignore parent1
		{
			prob[i] = pow((float)(rankedbots - i), thegame->spinner_exponent);
			sumprob += prob[i];
		}

	for(i=0; i<rankedbots; i++)
		prob[i] /= sumprob; //normalize so that sumprob = 1


	randomproportion = random();
	for ( i=0; i < rankedbots; i++)
		if(bot[i] != g->parent1)//ignore parent1
		{
			randomproportion -= prob[i];

			if(randomproportion <= 0)
			{
				g->parent2 = bot[i];
				i=10000;	//exit the loop
			}
			else if(i == rankedbots - 1)
			{
				safe_bprintf (PRINT_HIGH,"parent2 did not get chosen with rank spinner!\n");
				return true;
			}

		}
	
	
	return true;
}









/*=========================================================================
qboolean generateParentChromosomes(GAinfo* g);
---------------------------------------------------------------------------
constructs the parent chromosomes from the parent-bots' weights
=========================================================================*/
qboolean generateParentChromosomes(GAinfo* g)
{
	long c;
	int m,n,p;


	//// generate parent1 chromosome //////////
	c=0;
												// Step through:
	for ( m=0; m< thegame->SYNAPSE_LAYERS; m++ ) // synapse layers
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // source neurons
			for ( p=0; p< thegame->NUM_NEURONS_IN_LAYER; p++ ) // destination neurons
			{
			/////// copy parent synapse weight to handy chromosome form
			 g->parent1_chromosome[c] = thegame->Neuralbot_Array[g->parent1]
				 ->client->botinfo.itsNet.Synapse_Array[n][p][m].weight;

			 c++;
			}
			

	///////////// generate parent2 chromosome //////////////////////


	c=0;
												// Step through:
	for ( m=0; m< thegame->SYNAPSE_LAYERS; m++ ) // synapse layers
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // source neurons
			for ( p=0; p< thegame->NUM_NEURONS_IN_LAYER; p++ ) // destination neurons
			{
			/////// copy parent synapse weight to handy chromosome form
			 g->parent2_chromosome[c] = thegame->Neuralbot_Array[g->parent2]
				 ->client->botinfo.itsNet.Synapse_Array[n][p][m].weight;

			 c++;
			}
	return true;
}













/*=========================================================================
qboolean generateParentNeuronChromosomes(GAinfo* g);
---------------------------------------------------------------------------

=========================================================================*/
qboolean generateParentNeuronChromosomes(GAinfo* g)
{
	long c;
	int m,n;
	int neuron_layers = thegame->SYNAPSE_LAYERS + 1;

	//// generate parent1 neuron chromosome //////////
	c=0;
												
	for ( m=0; m<neuron_layers; m++ ) 
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) 
		{
			g->parent1_neuron_chr[c] = thegame->Neuralbot_Array[g->parent1]
				 ->client->botinfo.itsNet.Neuron_Array[n][m];

			c++;
		}


	//// generate parent2 neuron chromosome //////////
	c=0;
												
	for ( m=0; m<neuron_layers; m++ ) 
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) 
		{
			g->parent2_neuron_chr[c] = thegame->Neuralbot_Array[g->parent2]
				 ->client->botinfo.itsNet.Neuron_Array[n][m];

			c++;
		}
	return true;
}






/*=========================================================================
qboolean linear_crossover(GAinfo* g);
---------------------------------------------------------------------------
performs a crossover operation on the parent DNA, resulting in mixed and 
matched child DNA.  For now the chromosomes are broken in two points.
=========================================================================*/
qboolean linear_crossover(GAinfo* g)
{
	
	long a,b,temp,i;


	/////// find where to break the chromosomes /////
	a = floor( random() * g->NUM_SYNAPSES );

	b = floor( random() * g->NUM_SYNAPSES );

	if ( a >= b ) // we want b to be the larger integer, so swap a and b.
	{
		temp = a;
		a = b;
		b = temp;
	}


	/// generate child 1 DNA ///
	for ( i = 0; i < a; i++ )
	{
		g->child1_chromosome[i] = g->parent1_chromosome[i];
	}
	for ( i = a; i < b; i++ )
	{
		g->child1_chromosome[i] = g->parent2_chromosome[i];
		
	}
	for ( i = b; i < g->NUM_SYNAPSES; i++ )
	{
		g->child1_chromosome[i] = g->parent1_chromosome[i];
		
	}

	/// generate child 2 DNA ///

	for ( i = 0; i < a; i++ )
	{
		g->child2_chromosome[i] = g->parent2_chromosome[i];
		
	}
	for ( i = a; i < b; i++ )
	{
		g->child2_chromosome[i] = g->parent1_chromosome[i];
		
	}
	for ( i = b; i <  g->NUM_SYNAPSES; i++ )
	{
		g->child2_chromosome[i] = g->parent2_chromosome[i];
		
	}

	// we now have 2 lots of child DNA
	return true;
}










/*=========================================================================
qboolean linear_neuron_crossover(GAinfo* g);
---------------------------------------------------------------------------
creates two child chromosomes from crossed-over parent chromosomes
=========================================================================*/
qboolean linear_neuron_crossover(GAinfo* g)
{	
	long a,b; //two break points
	long temp,i;
	int num_neurons = (thegame->SYNAPSE_LAYERS + 1)*thegame->NUM_NEURONS_IN_LAYER;


	/////// find where to break the chromosomes /////
	a = floor( random() * num_neurons );

	b = floor( random() * num_neurons );

	if ( a >= b ) // we want b to be the larger integer, so swap a and b.
	{
		temp = a;
		a = b;
		b = temp;
	}


	/// generate child 1 DNA ///
	for ( i = 0; i < a; i++ )
	{
		g->child1_neuron_chr[i] = g->parent1_neuron_chr[i];
	}
	for ( i = a; i < b; i++ )
	{
		g->child1_neuron_chr[i] = g->parent2_neuron_chr[i];
		
	}
	for ( i = b; i < num_neurons; i++ )
	{
		g->child1_neuron_chr[i] = g->parent1_neuron_chr[i];
		
	}

	/// generate child 2 DNA ///

	for ( i = 0; i < a; i++ )
	{
		g->child2_neuron_chr[i] = g->parent2_neuron_chr[i];
		
	}
	for ( i = a; i < b; i++ )
	{
		g->child2_neuron_chr[i] = g->parent1_neuron_chr[i];
		
	}
	for ( i = b; i <  num_neurons; i++ )
	{
		g->child2_neuron_chr[i] = g->parent2_neuron_chr[i];
		
	}

	// we now have 2 lots of child DNA
	return true;
}








/*=========================================================================
vertical_crossover
---------------------------------------------------------------------------
performs a crossover but codes the weights into chromosomes representing
each synapse layer. The same crossover operation is then performed on all (say 3) 
chromosomes in parallel.  This means vertical features that span several
synapse layers can be kept.  (perhaps)

Note:  not quite sure what exactly this crossover function will do:
might pay to encode the synapses in a different order
=====================================================================*/
qboolean vertical_crossover(GAinfo* g)
{

	long i;
	int temp;
	int	a,b;	// crossover break points
	int clayer;	//chromosome layer
	long nosinlayer = thegame->NUM_NEURONS_IN_LAYER*thegame->NUM_NEURONS_IN_LAYER;	// num synapses in layer
		// work out number of synapses per synapse layer


	/////// find where to break the chromosomes /////
	a = rand() % nosinlayer;
	b = rand() % nosinlayer;

	if ( a >= b ) // we want b to be the larger integer, so swap a and b.
	{
		temp = a;
		a = b;
		b = temp;
	}	

	for(clayer=0; clayer<thegame->SYNAPSE_LAYERS; clayer++)
	{		// do the same crossover operation on each chromosome

		/// generate child1 DNA ///
		for ( i = 0; i < a; i++ )
			g->child1_chromosome[(clayer*nosinlayer)+i] = g->parent1_chromosome[(clayer*nosinlayer)+i];

		for ( i = a; i < b; i++ )
			g->child1_chromosome[(clayer*nosinlayer)+i] = g->parent2_chromosome[(clayer*nosinlayer)+i];
		
		for ( i = b; i < nosinlayer; i++ )
			g->child1_chromosome[(clayer*nosinlayer)+i] = g->parent1_chromosome[(clayer*nosinlayer)+i];

		/// generate child 2 DNA ///
		for ( i = 0; i < a; i++ )
			g->child2_chromosome[(clayer*nosinlayer)+i] = g->parent2_chromosome[(clayer*nosinlayer)+i];

		for ( i = a; i < b; i++ )
			g->child2_chromosome[(clayer*nosinlayer)+i] = g->parent1_chromosome[(clayer*nosinlayer)+i];

		for ( i = b; i <  nosinlayer; i++ )
			g->child2_chromosome[(clayer*nosinlayer)+i] = g->parent2_chromosome[(clayer*nosinlayer)+i];
	}
	return true;
}











/*=========================================================================
qboolean discretemutate(GAinfo* g)
---------------------------------------------------------------------------
mutates the child synapse chromosomes
=========================================================================*/
qboolean discretemutate(GAinfo* g)
{	
	long numtomutate;
	long synapsetomutate;


	if ( random() <= (thegame->CHANCE_MUTATE / 100) ) //if child1 is getting mutated
	{
		numtomutate = rand() % thegame->MAX_NUM_MUTATE;//find number of synapses to mutate
		
		if ( numtomutate != 0 ) // if we have some synaspes to mutate
		{
			if(thegame->obits == true)	//if messages on
			{
			//	safe_bprintf (PRINT_HIGH,"mutating child 1 DNA \n");
				safe_bprintf (PRINT_HIGH,"	child 1 is getting %i synapses mutated\n", numtomutate);
			}
			thegame->Neuralbot_Array[g->child1]->client->botinfo.mutant = true; //child 1 is now a mutant!

			while ( numtomutate > 0 ) //while still synapses to mutate
			{
				synapsetomutate = rand() % g->NUM_SYNAPSES; //find synapse to mutate
				
				if(g->child1_chromosome[synapsetomutate] != NOSYNAPSE)//if the synapse to mutate 'exists'
				{
					if(thegame->smallmutate)
						g->child1_chromosome[synapsetomutate] = smallWeightMutate(g->child1_chromosome[synapsetomutate]);
					else
						g->child1_chromosome[synapsetomutate] = -1 + ( random() * 2 ); //big mutate
				//NOTE: does not mutate synapses that 'do not exist', but still counts
				//them towards the total number
					
				
					/*

					if(g->child1_chromosome[synapsetomutate] > 1)
						g->child1_chromosome[synapsetomutate] = 1;
					else if (g->child1_chromosome[synapsetomutate] < -1)
						g->child1_chromosome[synapsetomutate] = -1;
					*/
				}

				numtomutate--;
			}
		}
	}

	//////// do the same for child 2 ////////////
	if ( random() <= (thegame->CHANCE_MUTATE / 100) )// test to see if child2's DNA gets mutated
	{
		numtomutate = rand() % thegame->MAX_NUM_MUTATE;
		
		if ( numtomutate != 0 ) // if we have some synaspes to mutate
		{
			if(thegame->obits == true)
			{
			//	safe_bprintf (PRINT_HIGH,"mutating child 2 DNA \n");
				safe_bprintf (PRINT_HIGH,"	child 2 is getting %i synapses mutated\n", numtomutate);
			}
			thegame->Neuralbot_Array[g->child2]->client->botinfo.mutant = true;

			while ( numtomutate > 0 )
			{
				synapsetomutate = rand() % g->NUM_SYNAPSES;

				if(g->child2_chromosome[synapsetomutate] != NOSYNAPSE)
				{
					if(thegame->smallmutate)
						g->child2_chromosome[synapsetomutate] = smallWeightMutate(g->child2_chromosome[synapsetomutate]);
					else
						g->child2_chromosome[synapsetomutate] = -1 + ( random() * 2 );
					/*

					if(g->child2_chromosome[synapsetomutate] > 1)
						g->child2_chromosome[synapsetomutate] = 1;
					else if (g->child2_chromosome[synapsetomutate] < -1)
						g->child2_chromosome[synapsetomutate] = -1;
					*/

				}

				numtomutate--;
			}
		}
	}
	return true;
}










/*=========================================================================
qboolean discreteneuronmutate(GAinfo* g)
---------------------------------------------------------------------------
mutates the child neuron chromosomes
=========================================================================*/
qboolean discreteneuronmutate(GAinfo* g)
{
	long	numtomutate;
	long	neurontomutate;
	int		num_neurons = (thegame->SYNAPSE_LAYERS + 1)*thegame->NUM_NEURONS_IN_LAYER;


	if ( random() <= (thegame->NEURON_CHANCE_MUTATE / 100) )//if child1 is going to be mutated
	{
		numtomutate = floor(random()*thegame->NEURON_MAX_NUM_MUTATE);//find nuumber of neurons to mutate
		
		if ( numtomutate != 0 ) // if we have some neurons to mutate
		{
			if(thegame->obits == true)
			{
				safe_bprintf (PRINT_HIGH,"	child 1 is getting %i neurons mutated\n", numtomutate);
			}

			while ( numtomutate > 0 )
			{
				neurontomutate = floor( random() * num_neurons );

				
				NeuronMutate(&g->child1_neuron_chr[neurontomutate]);//this function is defined in nb_neuronthink.c

				numtomutate--;
			}
		}
	}


	// test to see if child2's DNA gets mutated

	if ( random() <= (thegame->NEURON_CHANCE_MUTATE / 100) )
	{
		numtomutate = floor(random()*thegame->NEURON_MAX_NUM_MUTATE);
		
		if ( numtomutate != 0 ) // if we have some synaspes to mutate
		{
			if(thegame->obits == true)
			{
				safe_bprintf (PRINT_HIGH,"	child 2 is getting %i neurons mutated\n", numtomutate);
			}

			while ( numtomutate > 0 )
			{
				neurontomutate = floor( random() * num_neurons );

				NeuronMutate(&g->child2_neuron_chr[neurontomutate]);

				numtomutate--;
			}
		}
	}
	return true;
}













/*=========================================================================
qboolean continuousmutate(GAinfo* g)
---------------------------------------------------------------------------
with this function every single synapse is given a chance to mutate 
=========================================================================*/
qboolean continuousmutate(GAinfo* g)
{
	long i;
	long child1num=0;
	long child2num=0;

	for(i=0; i<g->NUM_SYNAPSES; i++)
		if(random() < thegame->CON_PROB_MUTATE)
		{
			if(g->child1_chromosome[i] != NOSYNAPSE)
			{
				if(thegame->smallmutate)
					g->child1_chromosome[i] = smallWeightMutate(g->child1_chromosome[i]);
				else
					g->child1_chromosome[i] = -1 + ( random() * 2 );	//mutate child1
			}

			child1num++;
		}

	for(i=0; i<g->NUM_SYNAPSES; i++)
		if(random() < thegame->CON_PROB_MUTATE)
		{
			if(g->child2_chromosome[i] != NOSYNAPSE)
			{
				if(thegame->smallmutate)
					g->child2_chromosome[i] = smallWeightMutate(g->child2_chromosome[i]);//NOTE: NOT fixed
				else
					g->child2_chromosome[i] = -1 + ( random() * 2 );	//child2
			}

			child2num++;
		}

	
		
		
	if(thegame->obits == true)
	{
		safe_bprintf (PRINT_HIGH,"	child1 continuous mutate: %i synapses\n", child1num);
		safe_bprintf (PRINT_HIGH,"	child2 continuous mutate: %i synapses\n", child2num);
	}

	return true;
}
















/*=========================================================================
qboolean choosechildren_Worst(GAinfo* g);
---------------------------------------------------------------------------
sets g->child1 and g->child2 to the two bots with the lowest fitness
=========================================================================*/
qboolean choosechildren_Worst(GAinfo* g)
{
	int botpick;
	float worstsuccess;
	qboolean candidate_array[MAX_BOTS];
	int z;
	long i;

	// NOTE: take this out later
	g->child1=666;
	g->child2=666;


	//// clear candidate_array ////
	for(z=0;z<MAX_BOTS;z++)
		candidate_array[z] = false;
	
	//////////// if none of the bots were the worst; that is if they all sucked, 
				// choose a random bot that is not a parent //////////////
	if ( g->sumfitness == 0 )
	{
		////////// child1 selection ///////////
		while(1)
		{
			botpick = rand() % thegame->NUM_BOTS; // select a random bot

			if ( (botpick != g->parent1) && (botpick != g->parent2))	// don't choose a parent
			{	
				g->child1 = botpick;	// store the bot's number in child1
				break;				// we have our bot
			}
		}						// else try again

		////////// child2 selection ///////////
		while(1)
		{
			botpick = rand() % thegame->NUM_BOTS; // select a random bot

			if ( (botpick != g->parent1) && (botpick != g->parent2) && (botpick != g->child1))	// don't choose a parent or the first child (a).
			{	
				g->child2 = botpick;	// store the bot's number in child2
				break;				// we have our bot
			}
		}						// else try again

	}
	else
	{	// else if sumfitness != 0..
		// replace the two suckiest bots

		worstsuccess = 100000;	

		///////// child 1 selection ///////////////
		// find the suckiest bot and store it's number in a.
		for ( i = 0; i < thegame->NUM_BOTS; i++ )
		{
			if ((thegame->Neuralbot_Array[i]->client->botinfo.success < worstsuccess) && (i != g->parent1) && (i != g->parent2))
				worstsuccess = thegame->Neuralbot_Array[i]->client->botinfo.success; //find the worst success
		
		}
		////////// find all bots that are children candidates ///////
		//// find all the bots with 'worstsuccess' fitness and that are not parents////
		for(z=0;z<thegame->NUM_BOTS;z++)
			if( (thegame->Neuralbot_Array[z]->client->botinfo.success == worstsuccess) && (z != g->parent1) && (z != g->parent2))
				candidate_array[z] = true;	//bot is a child candidate

		/// make child1 a random bot out of all the candidates ////
		g->child1 = choose_bot(candidate_array);

		//================== child2 selection ===========================//
		if( (g->sumfitness - thegame->Neuralbot_Array[g->child1]->client->botinfo.success) == 0)
		{		// if none of the other bots had fitness != 0 ..
			// then chose the second child at random.
			while(1)
			{
				botpick = rand() % thegame->NUM_BOTS; // select a random bot

				if ( (botpick != g->parent1) && (botpick != g->parent2) && (botpick != g->child1))	// don't choose a parent or the first child.
				{	
					g->child2 = botpick;	// store the bot's number in a
					break;				// we have our bot
				}
			}						// else try again
		}
		else
		{	// else choose the child with the second lowest fitness
			worstsuccess = 100000; // reset worstsuccess

			// find the second suckiest bot and store it's number in b.
			// this search is the same with the expeption that the suckiest bot is ignored

			for ( i = 0; i < thegame->NUM_BOTS; i++ )
			{
				if ( thegame->Neuralbot_Array[i]->client->botinfo.success < worstsuccess )
				{
					if ((i != g->child1 ) && (i != g->parent1) && (i != g->parent2))	// if this is not the suckiest bot as chosen above and is not a parent..
						worstsuccess = thegame->Neuralbot_Array[i]->client->botinfo.success; // then it sets a new suckiness record!
				}
			}

			//// clear candidate_array ////
			for(z=0;z<MAX_BOTS;z++)
				candidate_array[z] = false;

			//// find all the bots with 'worstsuccess' fitness, and that are not parents ////
			for(z=0;z<thegame->NUM_BOTS;z++)
				if((thegame->Neuralbot_Array[z]->client->botinfo.success == worstsuccess) && (z != g->parent1) && (z != g->parent2) && (z != g->child1 ))
					candidate_array[z] = true;	//bot is a child candidate

			/// make child2 a random bot out of all the candidates ////
			g->child2 = choose_bot(candidate_array);

		}
	}

	if((g->child1==666) || (g->child2==666) || g->child1 >= thegame->NUM_BOTS || g->child2 >= thegame->NUM_BOTS)
	{
		g->child1=0;
		g->child2=0;
		safe_bprintf (PRINT_HIGH,"77777777777big fuck up children were chosen improperly777777777777\n");
	}

	return true;
}













/*=========================================================================
overwriteChildNNs
---------------------------------------------------------------------------
=========================================================================*/
qboolean overwriteChildNNs(GAinfo* g)
{

	long c;
	int m,n,p;

	c=0;
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
			



	// replace second suckiest bot with second child:

	c=0;
												// Step through:
	for ( m=0; m< thegame->SYNAPSE_LAYERS; m++ ) // synapse layers
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // source neurons
			for ( p=0; p< thegame->NUM_NEURONS_IN_LAYER; p++ ) // destination neurons
			{
			
				if ( thegame->Neuralbot_Array[g->child2]->client->botinfo.itsNet.Synapse_Array[n][p][m].weight != NOSYNAPSE ) 		
					thegame->Neuralbot_Array[g->child2]->client->botinfo.itsNet
					.Synapse_Array[n][p][m].weight = g->child2_chromosome[c];


			 c++;// is cool
			}


	return true;			
}













/*=========================================================================
overwriteChild1Neurons
---------------------------------------------------------------------------

=========================================================================*/
qboolean overwriteChild1Neurons(GAinfo* g)
{
	long c;
	int m,n;
	int neuron_layers = thegame->SYNAPSE_LAYERS + 1;

	//// overwrite child 1's neurons //////////
	c=0;
												
	for ( m=0; m<neuron_layers; m++ ) 
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) 
		{
			thegame->Neuralbot_Array[g->child1]->client->botinfo.itsNet.Neuron_Array[n][m] = g->child1_neuron_chr[c];

			c++;
		}

	return true;
}

/*=========================================================================
overwriteChild2Neurons
---------------------------------------------------------------------------

=========================================================================*/
qboolean overwriteChild2Neurons(GAinfo* g)
{
	long c;
	int m,n;
	int neuron_layers = thegame->SYNAPSE_LAYERS + 1;


	//// overwrite child 2's neurons //////////
	c=0;
												
	for ( m=0; m<neuron_layers; m++ ) 
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) 
		{
			thegame->Neuralbot_Array[g->child2]->client->botinfo.itsNet.Neuron_Array[n][m] = g->child2_neuron_chr[c];

			c++;
		}
	return true;
}










/*=========================================================================
choose_bot
---------------------------------------------------------------------------
takes as an argument a pointer to an array of possible candidates.
returns the number of the one chosen.
=========================================================================*/
int choose_bot(qboolean* cand_array)
{
	int numcands=0;		// = number of array entries equal to 'true'
	int i;		
	int luckybot;		// number of the lucky candidate.
	int botpointer=0;	// the lucky candidate's array number

	for(i=0; i<MAX_BOTS; i++)
		if(cand_array[i] == true)
			numcands++;	// count the number of candidates

	if(numcands <= 0)
	{
		safe_bprintf (PRINT_HIGH,"*****************numcands <= 0*****************\n");
		return 0;
	}


	luckybot = rand() % numcands; // choose the lucky bot
	

	while(cand_array[botpointer] == false)	//cycle through to first candidate
		botpointer++;


	while(luckybot>0) // go through the loop 'luckybot' times
	{
		botpointer++;	// take at least one step thorugh the array

		while(cand_array[botpointer] == false)	// if next qboolean is not 'a candidate'
			botpointer++;	//advance to the next candidate
		
		luckybot--;
	}


	// NOTE: take this out later
	if(cand_array[botpointer] == false)
		safe_bprintf (PRINT_HIGH,"$$%#$^chose a non-eligible bot in choose_bot()!$^$#^\n");
	

	return botpointer;
}















/*=========================================================================
bot_pickup_item
---------------------------------------------------------------------------
adds one to the bot's itemsgot variable
=========================================================================*/
void bot_pickup_item(edict_t* self)  
{
	//// check to see if the entity is a client ////
	if(self->client == NULL)
		return;

	self->client->botinfo.itemsgot++;
	self->client->botinfo.structure_itemsgot++;
	//safe_bprintf (PRINT_HIGH,"bot got an item\n");

}

















void Cmd_Evolve_f ()	// the console command function
{
	Evolve();
}


/*==========================================================================================
Evolve
------------------------------------------------------------------------------------------
the main evolutionary function

* evaluates the success of the bots

* resets their 'last game period frag-counter, damage-counter etc..'

* Calls the appropriate genetic algorithm function
===========================================================================*/
void Evolve ()
{

	int i;

	if(thegame->bot_spawn_index < thegame->NUM_BOTS)//if not all bots have been spawned yet
	{
		safe_bprintf(PRINT_HIGH, "Evolve: wait till all bots are spawned\n");
		return;	//don't run the GA
	}


	//if(thegame->obits == true)
		safe_bprintf (PRINT_HIGH,"**** Evolve ****\n");


	for(i=0; i<thegame->NUM_BOTS; i++)
		evaluate_success(thegame->Neuralbot_Array[i]);
	

	for(i=0; i<thegame->NUM_BOTS; i++)
		clear_scoreslastperiod(thegame->Neuralbot_Array[i]);


	//// run the genetic algorithm /////
	switch(thegame->GAtype)
	{
		case GA_NORMAL:		GAnormal(thegame);
			break;
		case GA_ONECHILD:	GAonechild(thegame);
			break;
		case GA_TOPOLOGY:	//GAtopology(thagame);
			break;
		case GA_DISTANCE:	//GAdistance(thegame);
			break;
		default:			safe_bprintf (PRINT_HIGH,"bad GA type\n");
			break;
	}
	
}


/*==========================================================================================
evaluate_success
------------------------------------------------------------------------------------------
evaluates the success of all the bots and assigns the individual success 
values to the Neuralbot member variable success
===========================================================================*/
void evaluate_success(edict_t* bot)
{

	nb_info*	binfo = &bot->client->botinfo;	//botinfo


	bot->client->botinfo.success = ( thegame->fragscore * binfo->fragslastperiod)
		+	(thegame->itemscore * binfo->itemsgot)
		+   (thegame->damagescore * binfo->damageinflicted)
		+	(thegame->explorescore * binfo->explorenesssofar);

	if(bot->client->botinfo.success < 0)//might have had negative frags
		bot->client->botinfo.success = 0;


	//if( bot->edictpointer->client->botinfo.mygame->obits == true)
	//safe_bprintf (PRINT_HIGH,"	%s's fitness: %i\n", bot->edictpointer->client->pers.netname, (long)bot->success);
	
}


/*========================================================================================
clear_botfrags
------------------------------------------------------------------------------------------
sets all the bot's frags, itemsgot etc.. in the last game period to 0 again
========================================================================================*/
void clear_scoreslastperiod(edict_t* bot)
{
	bot->client->botinfo.fragslastperiod = 0;
	bot->client->botinfo.itemsgot = 0;
	bot->client->botinfo.damageinflicted = 0;
	bot->client->botinfo.explorenesssofar = 0;
}





void DoFileDNACrossover(char* filename);

void Cmd_Crossoverwith_f(qboolean svcmd)
{

	char filename[100];

	filename[0] = '\0';

	if(svcmd)
		strcat(filename, gi.argv(2));//filename = gi.argv(2);
	else
		strcat(filename, gi.argv(1));

	strcat (filename, ".dna");	  // add a .dna at the end

	DoFileDNACrossover(filename);

}

/*================================================================================================
DoFileDNACrossover
------------------

see nb_about for lotsa details.
Crosses over the dna of the bots in the game with the dna specified
in 'filename'.dna.

  NOTE: disable for now
================================================================================================*/
void DoFileDNACrossover(char* filename)
{


	FILE	*f;

	long	NUM_SYNAPSES = thegame->NUM_NEURONS_IN_LAYER * thegame->NUM_NEURONS_IN_LAYER
			* thegame->SYNAPSE_LAYERS;

	int		braincounter = 0;
	long	break1, break2;	//crossover points
	
	long	i;
	int		numDNAstrings;
	long	c;
	int		m,n,p,z;

	float	DNA[20000];//SYNAPSENUMDEPENDENT

	long	generation;
	long	structure_generation;
	long	lplaceholder3;
	long	lplaceholder4;
	long	lplaceholder5;
	float	fplaceholder1;
	float	fplaceholder2;
	float	fplaceholder3;
	float	fplaceholder4;
	float	fplaceholder5;

	long neuronsize;
	long NUM_NEURONS = thegame->NUM_NEURONS_IN_LAYER * (thegame->SYNAPSE_LAYERS+1);

	return;//disabled for now

	safe_bprintf (PRINT_HIGH, "crossing dna with: %s.dna...\n", gi.argv(1) );


	f = fopen (filename, "rb");
	if (!f)
	{
		safe_bprintf (PRINT_HIGH, "couldn't open up file!\n");
		return;
	}

	fread(&generation,sizeof(generation),1,f);
	fread(&structure_generation,sizeof(structure_generation),1,f);
	fread(&lplaceholder3,sizeof(lplaceholder3),1,f);
	fread(&lplaceholder4,sizeof(lplaceholder4),1,f);
	fread(&lplaceholder5,sizeof(lplaceholder5),1,f);
	fread(&fplaceholder1,sizeof(fplaceholder1),1,f);
	fread(&fplaceholder2,sizeof(fplaceholder2),1,f);
	fread(&fplaceholder3,sizeof(fplaceholder3),1,f);
	fread(&fplaceholder4,sizeof(fplaceholder4),1,f);
	fread(&fplaceholder5,sizeof(fplaceholder5),1,f);
	// these are just placeholders in case I want to put some other stuff
	// here later.


	// check DNA size
	fread (&i, sizeof(i), 1, f);
	if (i != sizeof(DNA))
	{
		fclose (f);
		safe_bprintf(PRINT_HIGH, "ReadNN: mismatched DNA size.. Aborting\n");
		return;
	}

		// read in number of pieces of Bot DNA in save file
	fread(&numDNAstrings,sizeof(numDNAstrings),1,f);


	//////////// get down to crossing over the dna ////////////////
	for(z=0; z<thegame->NUM_BOTS && z<numDNAstrings; z++) // while there are more bot-brains to fill and there is still DNA to load in...
	{	
		braincounter++;

		/////// find where to break the chromosomes /////
		break1 = floor( random() * NUM_SYNAPSES );
		break2 = floor( random() * NUM_SYNAPSES );

		if ( break1 >= break2 ) // we want b to be the larger integer, so swap a and b.
		{
			long temp = break1;
			break1 = break2;
			break2 = temp;
		}		


		fread(&DNA, sizeof(DNA), 1, f); //  read in data from file to DNA array
		
		// there is a 50% chance that the incoming dna will get sections 1 and 3, and a 50% chance
		// that it will get section 2.

		if(random() < 0.5)
		{
			c=0;
			for ( m=0; m< thegame->SYNAPSE_LAYERS; m++ ) // synapse layers
				for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // source neurons
					for ( p=0; p< thegame->NUM_NEURONS_IN_LAYER; p++ ) // destination neurons
					{
						if(c < break1)	//section 1
							thegame->Neuralbot_Array[z]->client->botinfo
							 .itsNet.Synapse_Array[n][p][m].weight = DNA[c];
						else if ( c >= break2)	//section 3
							thegame->Neuralbot_Array[z]->client->botinfo
							 .itsNet.Synapse_Array[n][p][m].weight = DNA[c];
						c++;
					}
		}
		else
		{
			c=0;
			for ( m=0; m< thegame->SYNAPSE_LAYERS; m++ ) // synapse layers
				for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // source neurons
					for ( p=0; p< thegame->NUM_NEURONS_IN_LAYER; p++ ) // destination neurons
					{

						if ( c >= break1 && c < break2)	//section 2
							thegame->Neuralbot_Array[z]
							 ->client->botinfo.itsNet.Synapse_Array[n][p][m].weight = DNA[c];
						c++;
					}
		}





	}



	//// advance to point in file where neuron info is stored ///
	if(numDNAstrings > thegame->NUM_BOTS)	//if there are more synapse DNA strings in file than bots playing
	{
		for(i=0; i<(numDNAstrings - thegame->NUM_BOTS); i++)	//read in sizeof(DNA) * 'extra dna strings in file' bytes
			fread(&DNA, sizeof(DNA), 1, f);					//but do nothing with it
	}

	///// check neuron size ////////
	fread (&neuronsize, sizeof(neuronsize), 1, f);//read size of Neuron structure for checking
	if(neuronsize != sizeof(Neuron))
	{
		fclose (f);
		safe_bprintf(PRINT_HIGH, "ReadNN: mismatched Neuron size.. Aborting\n");
		return;
	}

	break1 = floor( random() * NUM_NEURONS );
	break2 = floor( random() * NUM_NEURONS );

	if ( break1 >= break2 ) // we want b to be the larger integer, so swap a and b.
	{
		long temp = break1;
		break1 = break2;
		break2 = temp;
	}	

	////read neurons from file //////////////
	c=0;
	if(random() < 0.5)
	{
		for(z=0; z<thegame->NUM_BOTS && z<numDNAstrings; z++)//while there are bots to go and is info in file.
			for ( m=0; m< (thegame->SYNAPSE_LAYERS+1); m++ ) // neuron layers
				for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // neurons	
				{
					if(c < break1)
						fread (&thegame->Neuralbot_Array[z]->client->botinfo.itsNet.Neuron_Array[n][m], sizeof(Neuron), 1, f);			
					else if(c >= break2)
						fread (&thegame->Neuralbot_Array[z]->client->botinfo.itsNet.Neuron_Array[n][m], sizeof(Neuron), 1, f);			

					c++;
				}


	}
	else
	{
		for(z=0; z<thegame->NUM_BOTS && z<numDNAstrings; z++)//while there are bots to go and is info in file.
			for ( m=0; m< (thegame->SYNAPSE_LAYERS+1); m++ ) // neuron layers
				for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // neurons	
				{
					if(c >= break1 && c < break2)
						fread (&thegame->Neuralbot_Array[z]->client->botinfo.itsNet.Neuron_Array[n][m], sizeof(Neuron), 1, f);			

					c++;
				}

	}




	safe_bprintf (PRINT_HIGH, "%i bot-brains combined with incoming dna\n",braincounter);

	if(thegame->NUM_BOTS > numDNAstrings)
		safe_bprintf (PRINT_HIGH, "** %i bot-brains unaltered - lack of data in file.**\n",(thegame->NUM_BOTS - numDNAstrings));


	fclose (f);	// release the file
	safe_bprintf (PRINT_HIGH, "Done crossing over.\n");

}





void doMutationSuccesses(GAinfo* g, qboolean onechildGA)
{
	//////////mutation success counter update /////
	if(thegame->Neuralbot_Array[g->parent1]->client->botinfo.mutant)//if a parent is a mutant
		thegame->mutant_successes++;				//1 goal for the mutants!
	if(thegame->Neuralbot_Array[g->parent2]->client->botinfo.mutant)
		thegame->mutant_successes++;

	if(thegame->Neuralbot_Array[g->child1]->client->botinfo.mutant)//if the mutant got replaced
		thegame->mutant_failures++;				// fail

	if(!onechildGA)//there is no child2 if using onechild GA
		if(thegame->Neuralbot_Array[g->child2]->client->botinfo.mutant)
			thegame->mutant_failures++;

	thegame->Neuralbot_Array[g->parent1]->client->botinfo.mutant = false;	//reset mutant flags
	thegame->Neuralbot_Array[g->parent2]->client->botinfo.mutant = false;

	thegame->Neuralbot_Array[g->child1]->client->botinfo.mutant = false;

	if(!onechildGA)
		thegame->Neuralbot_Array[g->child2]->client->botinfo.mutant = false;
}




/*
void CopyDynWeightsToBirthWeights(NeuralNet* net)
{
	int s,d, layer;

	
	for(s=0; s<net->num_neurons_in_layer[0]; s++)
		for(d=0; d<net->num_neurons_in_layer[0]; d++)
			for(layer=0; layer<(net->SYNAPSE_LAYERS + 1); layer++)
			{
				net->birthWeights[s][d][layer] = net->Synapse_Array[s][d][layer];
			}
}

void CopyBirthWeightsToDynWeights(NeuralNet* net)
{	
	int s,d, layer;

	
	for(s=0; s<net->num_neurons_in_layer[0]; s++)
		for(d=0; d<net->num_neurons_in_layer[0]; d++)
			for(layer=0; layer<(net->SYNAPSE_LAYERS + 1); layer++)
			{
				net->Synapse_Array[s][d][layer] = net->birthWeights[s][d][layer];
			}
}*/
/*================================================================
CopyDynOvertoBirthWeights
-------------------------
copies the normal weights of all the bots over to its birth weights
================================================================*//*
void CopyDynOvertoBirthWeights(GAinfo* g)
{

	int b;
	for(b=0; b<thegame->NUM_BOTS; b++)
		CopyBirthWeightsToDynWeights(&thegame->Neuralbot_Array[b]->client->botinfo.itsNet);

	//safe_bprintf(PRINT_HIGH, "copying normal weights over to birth weights\n");
}*/
/*==================================================================
CopyBirthWeightsOver
--------------------
copy all the bots birth weights over to their normal weights
==================================================================*/
/*
void CopyBirthWeightsOver(GAinfo* g)
{
	int b;
	for(b=0; b<thegame->NUM_BOTS; b++)
		CopyBirthWeightsToDynWeights(&thegame->Neuralbot_Array[b]->client->botinfo.itsNet);

	//safe_bprintf(PRINT_HIGH, "copying birth weights over to normal weights\n");
}

*/
/*=================================================================================
shouldRunGA
-----------
returns true if the GA should be run
There are 3 reasons why the GA should be run:
if quickevolve is on, and two bots each have 'quickevolve_threshold' fitness; 

if the current sumfitness of the bots is above the 'simfitnesstarget';

and if it has been longer than 'evolve_period_ceiling' since the last run of the GA.
=================================================================================*/
qboolean shouldRunGA()
{
	int i;
	int parent1;
	qboolean p1found = false;

//	safe_bprintf(PRINT_HIGH, "time since last run of the GA: %i\n", (long)(level.time - thegame->last_evolve_time));

	if(!areAllBotsSpawned(thegame))	//don't run GA when not all the bots have spawned yet
		return false;

	if(!thegame->evolution)//if main GA toggled off
		return false;

	if(thegame->quickevolve)
	{
		
		for(i=0; i<thegame->NUM_BOTS; i++)
			evaluate_success(thegame->Neuralbot_Array[i]);

		for(i=0; i<thegame->NUM_BOTS && !p1found; i++)//while stiff bots to look at and parent 1 not found
			if(thegame->Neuralbot_Array[i]->client->botinfo.success >= thegame->quickevolve_threshold)	//if a bot has got enough fitness
			{
				parent1 = i;
				p1found = true;	//exit the for loop 
			}

		if(p1found)	
		{
			for(i=0; i<thegame->NUM_BOTS; i++)
				if(thegame->Neuralbot_Array[i]->client->botinfo.success >= thegame->quickevolve_threshold && i != parent1)	//if found a bot that has fitness, and is not parent 1
					return true;
		}
	}

	/////////////////////////////////////////////////////////
	if(thegame->sumfitnesstarget)	// 0 sumfitnesstarget is a sentinel value for adaptive evolve_period OFF
		if(getSumFitness() >= thegame->sumfitnesstarget)	//if we have reached the sumiftness target
			return true;

	if(level.time >= thegame->evolve_time)	//if we have been going long enough to warrant a run of the GA	
		return true;

	return false;	


}


/*==================================================================
getSumFitness
-------------
returns the sum of the fitness/success of all the bots in the game
==================================================================*/
float getSumFitness()
{
	int i;
	float sumfitness=0;
	
	for(i=0; i<thegame->NUM_BOTS; i++)
		evaluate_success(thegame->Neuralbot_Array[i]);

	for(i=0; i<thegame->NUM_BOTS; i++)
		sumfitness += thegame->Neuralbot_Array[i]->client->botinfo.success;

	return sumfitness;
}


/*==================================================================
smallWeightMutate
-----------------
smallmutation:
adds += up to small_mutate_amount to synapse weight
keeps weight between -1 and 1
==================================================================*/
float smallWeightMutate(float currentweight)
{
	float newweight = currentweight - thegame->small_mutate_amount + (random() * 2 * thegame->small_mutate_amount);

	if(newweight > 1)
		newweight = 1;
	if(newweight < -1)
		newweight = -1;

	return newweight;
}













