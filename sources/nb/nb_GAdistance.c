/*========================================================================
	Neuralbot v0.5						14/July/99
	--------------
		
Neuralbot is made by Nicholas Chapman (c)1999

Feel free to slice and dice this code as you see fit.
	If you want to use any of the code for any of your projects, feel free.
If you release this project, drop me a line as I would like to see what 
this code has been used for.
	If you wish to use any of this code in a commercial release, you must gain 
my consent first.

The area of bot AI is large, and way more experimentation needs to be done
with neural-networks and g.a.s in bots than I could ever do.  So I encourage 
you all to use this code as a base or inspiration or whatever and see if you 
can squeeze some learning out of a bot.

  Nick Chapman					nickamy@paradise.net.nz  icq no# 19054613

==========================================================================*/
#include "g_local.h"


void GAdistance(Botgame* thegame)
{
}

void dgdfhsjtgGAdistance(Botgame* thegame)
{

	int parent1;
	int parent2;
	float sumfitness = 0;
	float randomproportion;
	int	botpick;
	int i, z;
	int a, b;
	int m, n, p;	//counter
	int temp;
	long c;
	float badsuccess;
	int NUM_SYNAPSES;
	int numtomutate;
	int synapsetomutate;
	int printnum; // used for printing floats

	float average_weight;
	float prob;



	/////////////////////// create chromosomes ///////////////////////////////

	/*
	float child1_chromosome[NUM_SYNAPSES];

	float child2_chromosome[NUM_SYNAPSES];

	float parent1_chromosome[NUM_SYNAPSES];

	float parent2_chromosome[NUM_SYNAPSES];
	*/

	// IMPORTANT NOTE: the number of floats has to be manually set to 
	// the number of synapses

	float child_chromosome[10000][2];



	float parent1_chromosome[10000][2];

	float parent2_chromosome[10000][2];

	const int WEIGHT = 0;
	const int DIST = 1;


	///////////// work out total number of synapses in a neuralnet //////////

	NUM_SYNAPSES = thegame->NUM_NEURONS_IN_LAYER * thegame->NUM_NEURONS_IN_LAYER
			* thegame->SYNAPSE_LAYERS;



	/*
	 Roullette selection mechanism:

	  makes sumfitness the sum of all the bot's successes(fitnesses)

	  makes randomproportion a random proportion of sumfitness

	  moves randomproportion along te list of bots - 
	the bot it reaches is chosen as a parent.

	  for the second parent the same search is performed,
		but if the first parent is selected the search is done again

	*/


	for ( i=0; i < thegame->NUM_BOTS; i++ )
	{
		sumfitness += thegame->Neuralbot_Array[i]->client->botinfo.success;	// find sumfitness
	}

	if ( sumfitness < 0 )
		sumfitness = 0;	// don't want a negative sumfitness

		//printnum = sumfitness;
		//safe_bprintf (PRINT_HIGH,"sumfitness: %i\n", (long)sumfitness);
 
	if ( sumfitness == 0 )
	{
		if(thegame->obits == true)
			safe_bprintf (PRINT_HIGH,"choosing parents at random\n");

		///// choose parent one at random ///////////
		parent1 = floor(random() * thegame->NUM_BOTS );
		
		///// choose parent two at random ///////////

		while(1)
		{
			botpick = floor(random() * thegame->NUM_BOTS );
			if ( botpick != parent1 )	// don't choose 1st parent
			{	
				parent2 = botpick;
				break;				// we have our bot
			}
		}						// else try again

	}
	else {

	// else use spinner selection method
	if(thegame->obits == true)
		safe_bprintf (PRINT_HIGH,"using spinner selection mechanism\n");

	 randomproportion = random() * sumfitness;

	//////////// find parent 1 //////////////

	for ( i=0; i < thegame->NUM_BOTS, randomproportion >= 0; i++ ) // randomproportion > 0 so loop exits when parent is found
	{
		randomproportion -= thegame->Neuralbot_Array[i]->client->botinfo.success;
		
			if ( randomproportion < 0 )
			{
				parent1 = i;
			}
	}

	// find parent 2

	randomproportion = random()*sumfitness;	// reset randomproportion

	z = 1;
	while (z == 1)// continuous loop
	{
		for ( i=0; i < thegame->NUM_BOTS; i++ )
		{
			randomproportion -= thegame->Neuralbot_Array[i]->client->botinfo.success;
		
			if ( randomproportion < 0 ) // if this bot is probably the lucky DNA donor
			{
			// see if parent 2 is the same as parent 1:
				if ( i == parent1 )
					continue;	// start search for parent 2 again
			// if it is not
				parent2 = i;
				z = 0;	// so we can get out of the while loop
				break;
			}
		}

	}

	}
	// now hopefully we have two parents selected.
	if(thegame->obits == true)
		safe_bprintf (PRINT_HIGH,"parent 1 is %s\n", thegame->Neuralbot_Array[parent1]->client->pers.netname);
	if(thegame->obits == true)
		safe_bprintf (PRINT_HIGH,"parent 2 is %s\n", thegame->Neuralbot_Array[parent2]->client->pers.netname);


	//////////// lets get down to the breeding.//////////////////////////

	//Crossover operation: this will make some child DNA from the two parent
	// chromosomes.


	/////////////////////// create chromosomes ///////////////////////////////



	///////// generate parent1 chromosome ///////////////

	c=0;
												// Step through:
	for ( m=0; m< thegame->SYNAPSE_LAYERS; m++ ) // synapse layers
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // source neurons
			for ( p=0; p< thegame->NUM_NEURONS_IN_LAYER; p++ ) // destination neurons
			{
			/////// copy parent synapse weight to handy chromosome form
			 parent1_chromosome[c][WEIGHT] = thegame->Neuralbot_Array[parent1]
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
				parent2_chromosome[c][WEIGHT] = thegame->Neuralbot_Array[parent2]
				 ->client->botinfo.itsNet.Synapse_Array[n][p][m].weight;

			 c++;
			}






	// Make b a random number between the number of synaspes,  
	// and greater than a.


	a = floor( random() * NUM_SYNAPSES );

	b = floor( random() * NUM_SYNAPSES );

	if ( a >= b ) // we want b to be the larger integer, so swap a and b.
	{
		temp = a;
		a = b;
		b = temp;
	}


	for( i = 0; i < NUM_SYNAPSES; i++ )
		child_chromosome[i][WEIGHT] = NOSYNAPSE;	//no connection by default



	for ( i = 0; i < NUM_SYNAPSES; i++ )
	{
		//// if both parents have a certain synapse
		if( (parent1_chromosome[i][WEIGHT] != NOSYNAPSE) &&
			(parent2_chromosome[i][WEIGHT] != NOSYNAPSE) )
		{	// then give it to the child
		
			prob = 1 - (550 - child_chromosome[i][DISTANCE])

			if( random()child_chromosome[i][DISTANCE]

			child_chromosome[i][WEIGHT]
	















	// we now have 2 lots of child DNA

	// time to see if the children's DNA is going to mutate
	// for know i'll give it a one in ten chance of mutating


	/////// test to see if child1's DNA gets mutated /////////////////

		// NOTE: i am making an asumption that the random() returns
		// a value with enough decimal places



	if ( random() <= (thegame->CHANCE_MUTATE / 100) )
	{
		numtomutate = floor(random()*thegame->MAX_NUM_MUTATE);
		
		if ( numtomutate != 0 ) // if we have some synaspes to mutate
		{
			if(thegame->obits == true)
				safe_bprintf (PRINT_HIGH,"mutating child 1 DNA \n");
		//	safe_bprintf (PRINT_HIGH,"	child 1 is getting %s synapses mutated\n",(int)numtomutate);

			while ( numtomutate > 0 )
			{
				synapsetomutate = floor( random() * NUM_SYNAPSES );

				child1_chromosome[synapsetomutate] = -1 + ( random() * 2 );

				numtomutate--;
			}
		}
	}









		/*
		// it is getting mutated, so work out how many synapses to mutate
				
		numtomutate = floor( random() * thegame->MAX_NUM_MUTATE );
		// NOTE: work out random stuff

		safe_bprintf (PRINT_HIGH,"child 1 is getting %s synapses mutated\n",(int)numtomutate);

		for ( ; numtomutate > 0; numtomutate-- )
		{
			// Find a random gene( correct terminology?) representing a 
			// synapse weight
			synapsetomutate = floor( random() * NUM_SYNAPSES );
			// mutate it
			child1_chromosome[synapsetomutate] = -1 + (random()*2);

		

		}
		
	}
	*/
	// test to see if child2's DNA gets mutated

	if ( random() <= (thegame->CHANCE_MUTATE / 100) )
	{
			numtomutate = floor(random()*thegame->MAX_NUM_MUTATE);
		
		if ( numtomutate != 0 ) // if we have some synaspes to mutate
		{
			if(thegame->obits == true)
				safe_bprintf (PRINT_HIGH,"mutating child 2 DNA \n");
		//	safe_bprintf (PRINT_HIGH,"child 2 is getting %s synapses mutated\n",(int)numtomutate);

			while ( numtomutate > 0 )
			{
				synapsetomutate = floor( random() * NUM_SYNAPSES );

				child2_chromosome[synapsetomutate] = -1 + ( random() * 2 );

				numtomutate--;
			}
		}
	}
		/*
		// it is getting mutated, so work out how many synapses to mutate
				
		numtomutate = floor( random() * thegame->MAX_NUM_MUTATE );
		// NOTE: work out random stuff

		
		safe_bprintf (PRINT_HIGH,"child 2 is getting %s synapses mutated\n",(int)numtomutate);

		for ( ; numtomutate > 0; numtomutate-- )
		{
			// Find a random gene( correct terminology?) representing a 
			// synapse weight
			synapsetomutate = floor( random() * NUM_SYNAPSES );
			// mutate it
			child2_chromosome[synapsetomutate] = -1 + (random()*2);
		

		}
		
	}
	*/
	// now we have 2 strands of possibly mutated child DNA.
	// for now the two children will replace the two least succesful bots
	// in the previous round.


	//////////// if none of the bots were the worst; that is if they all sucked, 
				// choose a random bot that is not a parent
	if ( sumfitness == 0 )
	{
		////////// child1 selection ///////////
		while(1)
		{
			botpick = floor(random() * thegame->NUM_BOTS ); // select a random bot

			if ( (botpick != parent1) && (botpick != parent2))	// don't choose a parent
			{	
				a = botpick;	// store the bot's number in a
				break;				// we have our bot
			}
		}						// else try again

		////////// child2 selection ///////////
		while(1)
		{
			botpick = floor(random() * thegame->NUM_BOTS ); // select a random bot

			if ( (botpick != parent1) && (botpick != parent2) && (botpick != a))	// don't choose a parent or the first child (a).
			{	
				b = botpick;	// store the bot's number in a
				break;				// we have our bot
			}
		}						// else try again

	}
	else {
	// else replace the two suckiest bots


	badsuccess = 100000;

	// find the suckiest bot and store it's number in a.

	for ( i = 0; i < thegame->NUM_BOTS; i++ )
	{
		if ( thegame->Neuralbot_Array[i].success < badsuccess )
		{
			badsuccess = thegame->Neuralbot_Array[i]->client->botinfo.success;
			a = i;	// store the bot's number in a
		}

	}


	badsuccess = 100000; // reset badsuccess

	// find the second suckiest bot and store it's number in b.
	// this search is the same with the expeption that the suckiest bot is ignored

	for ( i = 0; i < thegame->NUM_BOTS; i++ )
	{
		if ( thegame->Neuralbot_Array[i]->client->botinfo.success < badsuccess )
		{
			if ( i != a )	// if this is not the suckiest bot
			{	 	
			badsuccess = thegame->Neuralbot_Array[i]->client->botinfo.success;
			b = i;	// store the bot's number in b
			}
			// if we have not found the suckiest bot,
			//  skip to the top of the for loop, hence ignoring the suckiest bot
		}

	}

	} // end of else section

	if(thegame->obits == true)
		safe_bprintf (PRINT_HIGH,"%s's DNA is getting purged\n",thegame->Neuralbot_Array[a]->client->pers.netname);

	if(thegame->obits == true)
			safe_bprintf (PRINT_HIGH,"%s's brain is getting replaced\n",thegame->Neuralbot_Array[b]->client->pers.netname);


	// Now it is time to replace the unsuccessful bots with the new children

	// replace suckiest bot with first child:

	// this is basically the reverse of the synapse to DNA transformation
	// allready performed

	c=0;
												// Step through:
	for ( m=0; m< thegame->SYNAPSE_LAYERS; m++ ) // synapse layers
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // source neurons
			for ( p=0; p< thegame->NUM_NEURONS_IN_LAYER; p++ ) // destination neurons
			{
				if ( thegame->Neuralbot_Array[a]->client->botinfo.itsNet.Synapse_Array[n][p][m].weight != NOSYNAPSE ) 		
					thegame->Neuralbot_Array[a]->client->botinfo.itsNet
					.Synapse_Array[n][p][m].weight = child1_chromosome[c];

			 c++;
			}
			



	// replace second suckiest bot with second child:

	c=0;
												// Step through:
	for ( m=0; m< thegame->SYNAPSE_LAYERS; m++ ) // synapse layers
		for ( n=0; n< thegame->NUM_NEURONS_IN_LAYER; n++ ) // source neurons
			for ( p=0; p< thegame->NUM_NEURONS_IN_LAYER; p++ ) // destination neurons
			{
			
				if ( thegame->Neuralbot_Array[b]->client->botinfo.itsNet.Synapse_Array[n][p][m].weight != NOSYNAPSE ) 		
					thegame->Neuralbot_Array[b]->client->botinfo.itsNet
					.Synapse_Array[n][p][m].weight = child2_chromosome[c];


			 c++;// is cool
			}
			
	thegame->generation++; // increase generation counter... just for fun

	printnum = thegame->generation;
	safe_bprintf (PRINT_HIGH,"		Evolve Complete	(generation: %i )			\n", (int)printnum);


	// The evolutionary proccess is now complete!

} // end of evolve function








void initialize_synapse_distances(Botgame* thegame)
{
	
	int i, s, d, l;// s = source neuron, d = dest neuron, l = layer
	long c = 0;
	float thelength;

	float xsep = 10;// distance between 2 adjacent neurons in the same layer
	float ysep = 50;// distance between neuron layers

	float xdis;
	float ydis;

	for (i=0; i<thegame->NUM_BOTS; i++ )
	{
											// Step through:
		for ( l=0; l< thegame->SYNAPSE_LAYERS; l++ ) // synapse layers
			for ( s=0; s< thegame->NUM_NEURONS_IN_LAYER; s++ ) // source neurons
				for ( d=0; d< thegame->NUM_NEURONS_IN_LAYER; d++ ) // destination neurons
				{
					xdis = abs( xsep(d - s) );
					ydis = ysep

				
					 thegame->Neuralbot_Array[i]->client->botinfo
					.itsNet.Synapse_Array[s][d][l].length = sqrt( (xdis*xdis) + 1 ); 
							//pythagoras's theorem
				
				 c++;
				}
	}


	safe_bprintf (PRINT_HIGH,"num synapse distances init'd: %i", (long)c);
}













