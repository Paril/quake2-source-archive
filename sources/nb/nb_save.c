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

void Cmd_Savenn_f(qboolean svcmd)
{

	char filename [100];

	filename[0] = '\0';


	if(svcmd)
		strcat (filename, gi.argv(2));//filename = gi.argv(2); 
	else
		strcat (filename, gi.argv(1));//filename = gi.argv(1);

	strcat (filename, ".dna");	  // add a .dna at the end
	
	safe_bprintf (PRINT_HIGH, "synapse weights saving to file: %s...\n", filename );


	WriteNN(filename);	// do the actual writing
		
}

void Cmd_Loadnn_f(qboolean svcmd)
{

	char filename[100];

	filename[0] = '\0';

	if(svcmd)
		strcat (filename, gi.argv(2));//filename = gi.argv(2); 
	else
		strcat (filename, gi.argv(1));//filename = gi.argv(1);

	strcat (filename, ".dna");	  // add a .dna at the end

	safe_bprintf (PRINT_HIGH, "synapse weights loading in from file: %s...\n", filename );

	ReadNN(filename);
}

void WriteNN (char *filename)
{
	long	i,l,s,d,n,z;
	FILE	*f;
	NeuralNet* net;
	int		numbots = thegame->NUM_BOTS;
	//a buffer so 5000 calls to fwrite() don't have to be made
	float	buffer[20000];//SYNAPSENUMDEPENDENT - must be bigger than num of synapses in 1 bot.
	long	bufferlength = 20000;

	long	generation = thegame->generation;
	long	structure_generation = thegame->structure_generation;	
	long	synapse_layers = thegame->SYNAPSE_LAYERS;
	long	num_hidden_neurons = thegame->Neuralbot_Array[0]->client->botinfo.itsNet.HIDDEN_NEURONS;//assuming all bots have same num of hidden neurons here
	float	version = 5.5;
	float	mut_successes = thegame->mutant_successes;
	float	mut_failures = thegame->mutant_failures;

	//neuron stuff://
	long	neuronsize;

	


	f = fopen (filename, "wb");//get the file
	if (!f)
	{
		safe_bprintf (PRINT_HIGH, "couldn't open up file!\n");
		return;
	}
		
		
	fwrite(&generation,sizeof(generation),1,f);
	fwrite(&structure_generation,sizeof(structure_generation),1,f);
	fwrite(&synapse_layers,sizeof(synapse_layers),1,f);
	fwrite(&num_hidden_neurons,sizeof(num_hidden_neurons),1,f);


	fwrite(&version,sizeof(version),1,f);	//code version
	fwrite(&mut_successes,sizeof(mut_successes),1,f);
	fwrite(&mut_failures,sizeof(mut_failures),1,f);

		// write number of bots for checking
	fwrite(&numbots,sizeof(numbots),1,f);
		//this will also be the number of sets of weights saved to the file.

	//NOTE: do check to see if buffer is big enough
	
	///////////////////////////////////write synapses to file///////////////////////////////////////////
	
	/////first copy synapse weights into buffer///////
	for(z=0; z<thegame->NUM_BOTS; z++)
	{
		net = &thegame->Neuralbot_Array[z]->client->botinfo.itsNet;	//set up a shortcut pointer


		i=0;								//cycle through:
		for ( l=0; l< net->SYNAPSE_LAYERS; l++ ) // synapse layers
			for ( s=0; s< net->num_neurons_in_layer[l]; s++ ) // source neurons
				for ( d=0; d< net->num_neurons_in_layer[l+1]; d++ ) // destination neurons
				{
					buffer[i] = net->Synapse_Array[s][d][l].weight;//copy weight to buffer
					i++;
				}
	
		fwrite(&i, sizeof(i), 1, f); // write number of bytes of the buffer filled
		fwrite(&buffer, sizeof(float) * i, 1, f); // write the used portion of the buffer to a file.
		//NOTE: check i is right here
	}

	///////////////////////////////////write neurons to file///////////////////////////////////////////

	neuronsize = sizeof(Neuron);
	fwrite (&neuronsize, sizeof(neuronsize), 1, f);//write size of Neuron structure for checking

	for(z=0; z<thegame->NUM_BOTS; z++)
	{
		net = &thegame->Neuralbot_Array[z]->client->botinfo.itsNet;	

		for ( l=0; l< (thegame->SYNAPSE_LAYERS + 1); l++ ) // neuron layers
			for ( n=0; n< net->num_neurons_in_layer[l]; n++ ) // neurons				
				fwrite (&net->Neuron_Array[n][l], sizeof(Neuron), 1, f);//write neuron structure to file			

	}


	fclose (f);
	safe_bprintf (PRINT_HIGH, "Saved.\n");

}


void ReadNN (char *filename)
{
	FILE	*f;
	NeuralNet* net;
	long	i,l,s,d,n,z;
	int		numDNAstrings;//in file
	float	buffer[20000];//SYNAPSENUMDEPENDENT
	long	bufferlength = 20000;
	long	length;

	long	generation;
	long	structure_generation;
	long	synapse_layers;
	long	num_hidden_neurons; 
	float	version;
	float	mut_successes;
	float	mut_failures; 

	long	neuronsize;


	f = fopen (filename, "rb");//open up file in 'read binary' mode
	if (!f)
	{
		safe_bprintf (PRINT_HIGH, "couldn't open up file!\n");
		return;
	}

	fread(&generation,sizeof(generation),1,f);
	thegame->generation = generation;//load in + set generation counter

	fread(&structure_generation,sizeof(structure_generation),1,f);
	thegame->structure_generation = structure_generation;

	fread(&synapse_layers,sizeof(synapse_layers),1,f);
		botgame_SetSynapseLayers(synapse_layers);
	safe_bprintf (PRINT_HIGH, "	setting synapse_layers to %i\n", synapse_layers);

	fread(&num_hidden_neurons,sizeof(num_hidden_neurons),1,f);
		botgame_setNumHiddenNeurons(num_hidden_neurons);
	safe_bprintf (PRINT_HIGH, "	setting hidden_neurons to %i\n", num_hidden_neurons);

	fread(&version,sizeof(version),1,f);
	if(version != 5.5)
		safe_bprintf (PRINT_HIGH, "warning - saved game from old version of code\n");

	fread(&mut_successes,sizeof(mut_successes),1,f);
	thegame->mutant_successes = (long)mut_successes;

	fread(&mut_failures,sizeof(mut_failures),1,f);
	thegame->mutant_failures = (long)mut_failures;


		// read in number of pieces of Bot DNA in save file
	fread(&numDNAstrings,sizeof(numDNAstrings),1,f);

	
	////// read synapses in from file /////////////
	for(z=0; z<thegame->NUM_BOTS && z<numDNAstrings; z++) // while there are more bot-brains to fill and there is still DNA to load in...
	{	
		fread(&length, sizeof(length), 1, f); //read in how much file to read into buffer
		fread(&buffer, sizeof(float) * length, 1, f); //read in data from file to buffer

		net = &thegame->Neuralbot_Array[z]->client->botinfo.itsNet;	//set up a shortcut pointer

		//// copy buffer data over to the net structures ////
		i=0;
		for ( l=0; l< thegame->SYNAPSE_LAYERS; l++ ) // synapse layers
			for ( s=0; s< net->num_neurons_in_layer[l]; s++ ) // source neurons
				for ( d=0; d< net->num_neurons_in_layer[l+1]; d++ ) // destination neurons
				{
					net->Synapse_Array[s][d][l].weight = buffer[i];
					i++;
				}

	}

	//// advance to point in file where neuron info is stored ///
	if(numDNAstrings > thegame->NUM_BOTS)	//if there are more synapse DNA strings in file than bots playing
	{
		for(i=0; i<(numDNAstrings - thegame->NUM_BOTS); i++)	
		{
			fread(&length, sizeof(length), 1, f);				//read in how much file to read into buffer
			fread(&buffer, sizeof(float) * length, 1, f);		// read in data and do nothing with it
		}
	}

	///// check neuron size ////////
	fread (&neuronsize, sizeof(neuronsize), 1, f);//read size of Neuron structure for checking
	if(neuronsize != sizeof(Neuron))
	{
		fclose (f);
		safe_bprintf(PRINT_HIGH, "ReadNN: mismatched Neuron size.. Aborting\n");
		return;
	}


	////read neurons from file //////////////
	for(z=0; z<thegame->NUM_BOTS && z<numDNAstrings; z++)//while there are bots to go and is info in file.
	{
		net = &thegame->Neuralbot_Array[z]->client->botinfo.itsNet;	

		for ( l=0; l< (thegame->SYNAPSE_LAYERS+1); l++ ) // neuron layers
			for ( n=0; n< net->num_neurons_in_layer[l]; n++ ) // neurons	
				fread (&net->Neuron_Array[n][l], sizeof(Neuron), 1, f);		
	}



	if(thegame->NUM_BOTS > numDNAstrings)
		safe_bprintf (PRINT_HIGH, "** %i bot-brains unaltered - lack of data in file.**\n",(thegame->NUM_BOTS - numDNAstrings));

	if(numDNAstrings > thegame->NUM_BOTS)
		safe_bprintf (PRINT_HIGH, "** %i sets of weights unused in file.**\n",(numDNAstrings - thegame->NUM_BOTS));


	fclose (f);	// release the file
	safe_bprintf (PRINT_HIGH, "Done\n");

}

/*========================================================================
DoSumfitnessHistory
-------------------
A history of the average sumfitness over 5 generations is kept, cause it's 
interesting information that should come in handy.

  the function
  ------------
Updates the short-term(last 5 generations) sumfitness history.  If the turn
is a multiple of HISTORY_PERIOD which is currently 5, then update the long-term
sumfitness history.

	float	sumfitness_history[200];
	float	last5_sumfitness[5];

  Parameters:
	* the game which has a place to store the history
	* the sumfitness of all the bots last evolve period.
========================================================================*/
void DoSumfitnessHistory(float sumfitness)
{
	int i;
	float	lotsafitness=0;

	return; //this code will crash quake2 :)

	////// update recent history array ////////////
	for(i=0; i<(HISTORY_PERIOD-1); i++)
		thegame->last5_sumfitness[i] = thegame->last5_sumfitness[i+1];
			// shuffle history to left

	thegame->last5_sumfitness[HISTORY_PERIOD-1] = sumfitness;
		// record the current sumfitness



	////////update evolve_period length////////////////
 	for(i=0; i<(HISTORY_PERIOD-1); i++)
		thegame->last5_period[i] = thegame->last5_period[i+1];
			// shuffle history to left

	//thegame->last5_period[HISTORY_PERIOD-1] = thegame->evolve_period;
		// record evolve_period




	//CHECK THIS!!!!!!!!!!!!

	//// if this is a 5th generation ////
	if((thegame->generation%HISTORY_PERIOD) == 0 && thegame->generation != 0)
	{		// if the generation is a multiple of 5 and not 0
		
		if(thegame->history_index > 199)
			safe_bprintf (PRINT_HIGH, "");
		{
			for(i=0; i<HISTORY_PERIOD; i++)
				lotsafitness += thegame->last5_sumfitness[i];	
					// find the total fitness

			thegame->sumfitness_history[thegame->history_index] = lotsafitness/HISTORY_PERIOD;
			
			thegame->history_index++;
		}
	}
}

/*======================================================================
WriteHistory
------------
Writes the game history right in to the file it is given
======================================================================*/
qboolean WriteHistory(FILE* f)
{
	long history_length=200;
	int period = HISTORY_PERIOD;

	///check the file /////
	if(f == NULL)
		return false;	// then don't continue

	fwrite (&history_length, sizeof(history_length), 1, f);
		//write the length of the history to file

	fwrite (&thegame->sumfitness_history, sizeof(thegame->sumfitness_history), 1, f);
			// write the longterm history data to the file

	fwrite (&thegame->history_index, sizeof(thegame->history_index), 1, f);
		// write history index ( how far through sumfitness_history array we have got)

	fwrite (&period, sizeof(period), 1, f);
		//write the length of the 'last few sumfitnesss' history to file

	fwrite (&thegame->last5_sumfitness, sizeof(thegame->last5_sumfitness), 1, f);
			// write the recent history data to the file

	return true;
}
/*======================================================================
ReadHistory
-----------
Writes the game history right in to the file it is given

Checks to see if the history and recent history lengths are the same in
the file as in the ram, does not continue if they are not.

======================================================================*/
qboolean ReadHistory(FILE* f)
{
	long history_length;
	int period;

	///check the file /////
	if(f == NULL)
		return false;	// then don't continue

	fread (&history_length, sizeof(history_length), 1, f);
		//read history length from file.
	if(history_length != 200)
	{
		safe_bprintf (PRINT_HIGH, "different history lengths: %i vs 200\n"
			,(long)history_length);
		return false;
	}

	fread (&thegame->sumfitness_history, sizeof(thegame->sumfitness_history), 1, f);
			// read the longterm history from file
	fread (&thegame->history_index, sizeof(thegame->history_index), 1, f);
		// read history index ( how far through sumfitness_history array we have got)


	fread (&period, sizeof(period), 1, f);
		//read the length of the 'last few sumfitnesss' history into ram
	if(period != HISTORY_PERIOD)
	{
		safe_bprintf (PRINT_HIGH, "different recent history lengths: %i vs 5\n"
			,(int)period);
		return false;
	}

	fread (&thegame->last5_sumfitness, sizeof(thegame->last5_sumfitness), 1, f);
			// read the recent history data into ram
	
	return true;
}

/*======================================================================
Cmd_Savehistory_f
------------------
saves the history to the file as specified by the 1st argument + the postfix
.his
======================================================================*/	
void Cmd_Savehistory_f(qboolean svcmd)
{
	int i;
	char filename [100];
	
	if(svcmd)
		strcat(filename, gi.argv(2)); 
	else
		strcat(filename, gi.argv(1));  	

	strcat (filename, ".his");	  // add a .his at the end

	for(i=0; i<98; i++)
		filename[i] = filename[i+1];

	safe_bprintf (PRINT_HIGH, "saving sumfitness history to %s...\n", filename);

	WriteHistoryToFile(filename);
}
/*======================================================================
Cmd_Loadhistory_f
------------------
loads the history from the file as specified by the 1st argument + the postfix
.his
======================================================================*/	
void Cmd_Loadhistory_f(qboolean svcmd)
{
	int i;
	char filename [100];
	
	if(svcmd)
		strcat(filename, gi.argv(2)); 
	else
		strcat(filename, gi.argv(1));  	

	strcat (filename, ".his");	  // add a .his at the end

	for(i=0; i<98; i++)
		filename[i] = filename[i+1];

	safe_bprintf (PRINT_HIGH, "reading sumfitness history from %s...\n", filename );

	ReadHistoryFromFile(filename);
}
/*======================================================================
WriteHistoryToFile
------------------
opens up the file as specified by the first argument and calls WriteHistory.
======================================================================*/
void WriteHistoryToFile(char* filename)
{
	FILE	*f;

	f = fopen (filename, "wb");
	if (!f)
	{
		safe_bprintf (PRINT_HIGH, "couldn't open up file!\n");
		return;
	}

	WriteHistory(f);	//do the actual writing
}

/*======================================================================
ReadHistoryFromFile
------------------
opens up the file as specified by the first argument and calls ReadHistory.
======================================================================*/
void ReadHistoryFromFile(char* filename)
{
	FILE	*f;

	f = fopen (filename, "rb");	// rb: read binary
	if (!f)
	{
		safe_bprintf (PRINT_HIGH, "couldn't open up file!\n");
		return;
	}

	ReadHistory(f);	//do the actual writing
}








