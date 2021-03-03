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
#include     <time.h>	//for seeding rand()


float acceleration_factor = 1.00;
float cruise_factor = 150.0;
int RunFrame_counter = 0;

IniFile ini_file = {0,0,0,0,0,NULL,NULL,NULL,NULL};

Botgame* thegame = NULL;

/*======================================================================
Cmd_Startgame_f
------------------------------------------------------------------------
This is the function called when the console command 'addbot' is entered.
======================================================================*/
void Cmd_Startgame_f()
{
	if (thegame != NULL)
	{
		safe_bprintf (PRINT_HIGH,"Botgame allready going\n");
		return;
	}
	else
	{
		creategame();
	}
}


/*======================================================================
creategame
------------------------------------------------------------------------
Creates a botgame
creates a spawner that adds the bots
======================================================================*/
void creategame()
{

	edict_t* botspawner;//the entity to spawn the bots later

	thegame = gi.TagMalloc(sizeof(Botgame), TAG_GAME);
		//reserve memory for the Botgame data

	memset (thegame, 0, sizeof(Botgame));
		//make sure the memory is nice and clean

	thegame->NUM_BOTS= 16;//default number of bots. don't set above MAX_BOTS

	thegame->evolution = true;//do run the GA
	//set parameters to their default values: these will probably be overidden //
	////// GA parameters /////
	thegame->GAtype = GA_NORMAL; // type of GA to use
	thegame->crossover_type = CO_VERTICAL; //type of crossover operator to use
	thegame->parentselection_type = PS_ROULETTE;	//type of parent-selection mechanism
	thegame->NUM_NEURONS_IN_LAYER = 60; // number of neurons in one layer 
	thegame->CHANCE_MUTATE = 10;	//  percentage chance of mutation per child when it is created
	thegame->MAX_NUM_MUTATE= 500;	// Maximum number of synapses that can mutate
	thegame->CON_PROB_MUTATE = 0.001; // prob per synapse of mutation
	thegame->evolve_time = level.time + 10; //thegame->evolve_period;
	thegame->sumfitnesstarget = 0;	//adaptive evolve period OFF
	thegame->evolve_period_ceiling = 300;//maximum time between runs of the GA
	thegame->quickevolve = false;	//quickevolve off
	thegame->quickevolve_threshold = 1;
	thegame->spinner_exponent = 1.0;	//normal spinner selction mechanism
	thegame->smallmutate = false;//do big mutations, not small ones
	thegame->small_mutate_amount = 0.2;//if smallmutate is turned on, change weight by at most this amount when mutating a synapse

	thegame->fragscore = 10;// amount a frag contributes to bot's success
	thegame->itemscore = 0;// amount picking up an item contributes to bot's success
	thegame->damagescore = 0;//amount inflicting 1 point of damage contributes to a bot's success.
	thegame->explorescore = 0;//amount doing 1 unit of 'exploring' contributes to bot's success

	////// NN parameter /////
	thegame->SYNAPSE_LAYERS = 2;  // Number of synapse layers; one less than the 
							// number of neuron layers 
	

	////// neuron mutating parameters /////
	thegame->NEURON_CHANCE_MUTATE = 10;
	thegame->NEURON_MAX_NUM_MUTATE = 40;

	////// Miscellaneous /////
	thegame->generation = 0;
	thegame->freeweapon = -1;	// no free weapons
	thegame->obits = true;	// Death messages when bots die and extra evolve info.
	thegame->telefrag_points = true; // whether the fitness-function rewards telefrag-gained frags.

	thegame->enemy_trail = false;	//for testing bot vision
	thegame->item_trail = false;

	thegame->autosave_period = 20; //num gens between autosaves of bot dna
	thegame->stl_dweight = 0;

	thegame->mutant_successes = 0;
	thegame->mutant_failures = 0;

	thegame->lookupanddown = true;	//bots can look up and down
	thegame->allowcrouch = true;//bots can crouch
	thegame->allowcenterview = false;//bots can use the centerview command
	thegame->stagger_weapon_changes = true;//enforce a delay between weapon changes


	////////////// NN structure stuff ///////////////
	thegame->struct_evolve_on = false;	// should the structure evolving GA be used?
	thegame->structure_generation = 0;
	thegame->structure_evolve_period = 90.0; // how long between Evolve()s
	thegame->structure_evolve_time = level.time + 10;//thegame->structure_evolve_period;
	thegame->STRUCTURE_CHANCE_MUTATE = 10;	
	thegame->STRUCTURE_MAX_NUM_MUTATE = 200;
	thegame->initial_num_nosynapses = 0;

	srand((unsigned int) time(NULL));	//seed random number generator

	BotgameReadIniFile();//read in parameter values to overwrite the defaults here

	thegame->bot_spawn_index = 0;//bot_spawn_index is will be the index in the Neuralbot_Array of the enxt bot to be spawned

	/// spawn a 'botspawner' that will do the actual spawning of the bots ///
	botspawner = G_Spawn();
	botspawner->classname = "botspawner";
	botspawner->think = botspawner_think;
	botspawner->nextthink = level.time + 0.1;

	//initialize_history();	// initialize sumfitness_history memory
	
	

	//NOTE: put these elsewhere

	//IMITATION NEWCODE: imitation is off by default
	//self->client->iminfo.imitation_on = false;

	thegame->evolve_time = level.time + thegame->evolve_period_ceiling;
	thegame->structure_evolve_time = level.time + thegame->structure_evolve_period;
		//update the next evolve time now evolve period has been read in from nb.ini


	//giveFreeWeapon(self);	//give free weapon 

	thegame->last_evolve_time = level.time;
	thegame->next_evolve_check_time = level.time + 0.5;//check to see if the GA should be run in 0.5 secs


	safe_bprintf (PRINT_HIGH,"Botgame started\n");


}


	

/*=======================================================================
initializeBot
-------------
Initializes neuralnet type stuff.
called on each bot when it is spawned into the map.
=======================================================================*/
void initializeBot(edict_t* bot)
{
	initialize_nnet_variables(bot);	// Set up variables(might end up constants)
					// like THRESHOLD and MAX_CHARGE
	initialize_bot_synapses(bot); // Set the synapse weights to random values

	initialize_NN_structure(bot); //generate some 'NOSYNAPSES' if needed

	initialize_neurons(bot);//set up the initial think functions for the neurons

	BotReadIniFile(bot);//read in neuralnet parameters for bot from nb.ini

	initializeNNIL(&bot->client->botinfo.itsNet);	//initialize num neurons in layer array for the bot's net

}






/*======================================================================
initialize_bot_synapses
------------------------------------------------------------------------
randomizes the synapses of all the bots
======================================================================*/
void initialize_bot_synapses(edict_t* bot)
{
	
	int s, d, l;// s = source neuron, d = dest neuron, l = layer
	long c = 0;

											// Step through:
	for ( l=0; l< thegame->SYNAPSE_LAYERS; l++ ) // synapse layers
		for ( s=0; s< thegame->NUM_NEURONS_IN_LAYER; s++ ) // source neurons
			for ( d=0; d< thegame->NUM_NEURONS_IN_LAYER; d++ ) // destination neurons
			{
				/////// randomize weight /////////////////////
				//bot->client->botinfo.itsNet.Synapse_Array[s][d][l].weight = 
				//bot->client->botinfo.itsNet.birthWeights[s][d][l].weight = -1 + (random()*2);
				
				bot->client->botinfo.itsNet.Synapse_Array[s][d][l].weight = -1 + (random()*2);

				c++;
			}




	//safe_bprintf (PRINT_HIGH,"number of synapses initialised: %i\n", (long)c);

	//safe_bprintf (PRINT_HIGH,"Synapse weights initialised\n");

}

/*======================================================================
initialize_NN structure
------------------------------------------------------------------------
'deletes' initial_num_nosynapses number of synapses.  
The synapses to be deleted are chosen at random.
note that a synapse maybe chosen more than once.
======================================================================*/
void initialize_NN_structure(edict_t* bot)
{
	//SYNAPSENUMDEPENDENT
	int synapse[20000]; // an array used to mark which synapses are to be 'deleted'
	//long numsynapses;
	int s, d, l;// s = source neuron, d = dest neuron, l = layer
	int i;
	long c = 0;
	long t = 0;
	long synapsetomutate;
	
	// 1 = nosynapse
	// 0 = synapse

	long NUM_SYNAPSES;
	
	NUM_SYNAPSES = thegame->NUM_NEURONS_IN_LAYER * thegame->NUM_NEURONS_IN_LAYER
		* thegame->SYNAPSE_LAYERS;
		// work out number of synapses in each bot's NN


	for(i=0; i<NUM_SYNAPSES; i++)	// reset the nosynapse marker array
		synapse[i] = 0;

	for(i=0; i<thegame->initial_num_nosynapses; i++)
	{
		synapsetomutate = rand() % NUM_SYNAPSES;// find out which synapses are to be 'deleted'
		synapse[synapsetomutate] = 1; // mark the array to show that they should be deleted
	}				

	c = 0;								// Step through:
	for ( l=0; l< thegame->SYNAPSE_LAYERS; l++ ) // synapse layers
		for ( s=0; s< thegame->NUM_NEURONS_IN_LAYER; s++ ) // source neurons
			for ( d=0; d< thegame->NUM_NEURONS_IN_LAYER; d++ ) // destination neurons
			{
				if( synapse[c] == 1 ) // if synapse is to be deleted
				{
					bot->client->botinfo.itsNet.Synapse_Array[s][d][l].weight = NOSYNAPSE;
					t++;
				}
				c++;
			}


	//safe_bprintf (PRINT_HIGH,"initializing NN structure\n");
	//safe_bprintf (PRINT_HIGH,"number of synapses 'deleted': %i\n", (long)t);


}




/*======================================================================
initializeNNIL
------------------------------------------------------------------------
work out the number of neurons in each layer and assign this value to the
num_neurons_in_layer array for future reference.
For example, a hidden neuron layer would have less neurons than an input/output
layer.
======================================================================*/
void initializeNNIL(NeuralNet* net)
{
	int layer;

	for(layer=0; layer<thegame->SYNAPSE_LAYERS + 1; layer++)	//cycle through all neuron layers, whether used or not (neuron layers = synapse layers + 1)
		net->num_neurons_in_layer[layer]
			= getNeuronsForLayer(layer, net);

}


/*======================================================================
initialize_nnet_variables
------------------------------------------------------------------------
sets up certain NN parameters. Note that each NN holds a copy of the
parameters
======================================================================*/
void initialize_nnet_variables(edict_t* bot)
{
		
	bot->client->botinfo.itsNet.SYNAPSE_LAYERS = thegame->SYNAPSE_LAYERS;
	bot->client->botinfo.itsNet.MAX_WEIGHT	= 1;
	bot->client->botinfo.itsNet.MIN_WEIGHT	= -1;
	bot->client->botinfo.itsNet.MAX_CHARGE	= 3; // irrelevant right now
	bot->client->botinfo.itsNet.THRESHOLD	= 1;
	bot->client->botinfo.itsNet.DISCHARGE_AMOUNT=1;
	bot->client->botinfo.itsNet.HIDDEN_NEURONS = 25;
		// number of neurons in a hidden layer (not input or output layer)

	bot->client->botinfo.itsNet.totalstlweightchange = 0;
	bot->client->botinfo.itsNet.shorttermlearning = false;//NOTE
	bot->client->botinfo.itsNet.STL_learningrate = 1.0;
	bot->client->botinfo.itsNet.reinforcement = 0;
		

}
	
/*======================================================================
initialize_neurons
------------------------------------------------------------------------
sets up neuron.think for all the bot's neurons.
======================================================================*/
void initialize_neurons(edict_t* bot)
{
	int l,n;
	int num_neuron_layers = thegame->SYNAPSE_LAYERS + 1;


	for(l=0; l<num_neuron_layers; l++)
		for(n=0; n<thegame->NUM_NEURONS_IN_LAYER; n++)
		{
			bot->client->botinfo.itsNet.Neuron_Array[n][l].think = NULL;//mutate it to start off?
			bot->client->botinfo.itsNet.Neuron_Array[n][l].thinkfuncnum = 14;//for printing the name of the thinkfunc
		}


	safe_bprintf (PRINT_HIGH,"neurons initialized\n");
	

}
/*======================================================================
initialize_history
------------------------------------------------------------------------
initialises the history data

  	float	sumfitness_history[200];

	float	last5_sumfitness[5];

======================================================================*/
void initialize_history()
{
	long i;

	for(i=0; i<200; i++)
		thegame->sumfitness_history[i] = 0;

	for(i=0; i<HISTORY_PERIOD; i++)
	{
		thegame->last5_sumfitness[i] = 0;
		thegame->last5_period[i] = 0;
	}

	thegame->history_index=0;
}


/*======================================================================
BotgameReadIniFile
------------------------------------------------------------------------
reads in parameter values for 'thegame' from the .ini (initialization text file) file
named nb.ini.
If it finds a relevant entry in the file, it overwrites the default value
specified in creategame()
======================================================================*/
void BotgameReadIniFile()
{
	int i;
	char *p; 
	float newvalue;

	i = Ini_ReadIniFile("nb.ini", &ini_file);
	
	if(i != 1) // if file could not be read
	{
		safe_bprintf (PRINT_HIGH,"\nWARNING: could not find nb.ini:\n");
		safe_bprintf (PRINT_HIGH,"make sure nb.ini is in your Quake2 directory\n\n");
		return;
	}
	
		// first we make sure that the .ini file is open
    if (ini_file.ini_file_read)      
	{

		safe_bprintf (PRINT_HIGH,"==reading in from nb.ini...==\n");

	// ================ General parameters ========================
		//////////// get NUM_BOTS /////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "num_bots");
		if (p != NULL)
		{
			thegame->NUM_BOTS = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting NUM_BOTS from nb.ini to %i\n", thegame->NUM_BOTS);
		}
		////////////// freerails ///////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "freeweapon");
		if (p != NULL)
		{
			thegame->freeweapon = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting freeweapon\n");
		}
		///////////// telefrag_points //////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "telefrag_points");
		if (p != NULL)
		{
			thegame->telefrag_points = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting telefrag_points\n");
		}

	//===================== GA parameters ==========================
		///////////// GA type //////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "GA_type");
		if (p != NULL)
		{
			thegame->GAtype = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting GA_type\n");
		}
		///////////// crossover type //////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "crossover_type");
		if (p != NULL)
		{
			thegame->crossover_type = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting crossover_type\n");
		}
		///////////// parent selection type //////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "parentselection_type");
		if (p != NULL)
		{
			thegame->parentselection_type = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting parentselection_type\n");
		}
		///////////// sumfitnesstarget //////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "sumfitness_target");
		if (p != NULL)
		{
			thegame->sumfitnesstarget = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting sumfitness_target\n");
		}
		///////////// evolve period ceiling //////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "evolve_period_ceiling");
		if (p != NULL)
		{
			thegame->evolve_period_ceiling = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting evolve_period_ceiling\n");
		}
		////////////// mutate_chance /////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "mutate_chance");
		if (p != NULL)
		{
			thegame->CHANCE_MUTATE = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting mutate_chance\n");
		}
		////////////// num_synapses_to_mutate ////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "num_synapses_to_mutate");
		if (p != NULL)
		{
			thegame->MAX_NUM_MUTATE = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting num_synapses_to_mutate\n");
		}
		////////////// neuron mutate_chance /////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "neuron_mutate_chance");
		if (p != NULL)
		{
			thegame->NEURON_CHANCE_MUTATE = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting neuron_mutate_chance\n");
		}
		////////////// num_neurons_to_mutate ////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "num_neurons_to_mutate");
		if (p != NULL)
		{
			thegame->NEURON_MAX_NUM_MUTATE = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting num_neurons_to_mutate\n");
		}
		////////////// continuous mutate chance /////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "con_mutate_prob_x1000000");
		if (p != NULL)
		{
			newvalue = atoi(p);
			thegame->CON_PROB_MUTATE = newvalue/1000000;
			safe_bprintf (PRINT_HIGH,"	setting con_mutate_chance_x100000\n");

		}
		////////////// fragscore ////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "frag_score");
		if (p != NULL)
		{
			thegame->fragscore = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting frag_score\n");
		}
		////////////// itemscore ////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "item_score");
		if (p != NULL)
		{
			thegame->itemscore = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting item_score\n");
		}
		////////////// damagescore ////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "damagescore");
		if (p != NULL)
		{
			thegame->damagescore = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting damagescore\n");
		}
		////////////// explorescore ////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "explorescore");
		if (p != NULL)
		{
			thegame->explorescore = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting explorescore\n");
		}
		////////////// quickevolve ////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "quickevolve");
		if (p != NULL)
		{
			thegame->quickevolve = atoi(p);
			safe_bprintf (PRINT_HIGH,"	setting quickevolve \n");//			NOTE: comment out?
		}
		////////////// quickevolve_threshold ////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "quickevolve_threshold");
		if (p != NULL)
		{
			thegame->quickevolve_threshold = atoi(p);
			safe_bprintf (PRINT_HIGH,"	setting quickevolve_threshold \n");//			NOTE: comment out?
		}

		////////////// spinner_exponent ////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "spinner_exponent_x1000");
		if (p != NULL)
		{
			newvalue = atoi(p);
			thegame->spinner_exponent = newvalue / 1000;
			safe_bprintf (PRINT_HIGH,"	setting spinner_exponent_x1000 \n");
		}
		////////////// smallmutate ////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "smallmutate");
		if (p != NULL)
		{			
			thegame->smallmutate = atoi(p);
			safe_bprintf (PRINT_HIGH,"	setting smallmutate \n");
		}
		////////////// small_mutate_amount ////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "small_mutate_amount_x1000");
		if (p != NULL)
		{
			newvalue = atoi(p);
			thegame->small_mutate_amount = newvalue / 1000;
			safe_bprintf (PRINT_HIGH,"	setting small_mutate_amount to %i\n", (int)(thegame->small_mutate_amount * 1000));
		}

	//==================== NN parameters ========================
		////////////// synapse layers ///////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "synapse_layers");
		if (p != NULL)
		{
			thegame->SYNAPSE_LAYERS = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting synapse layers\n");
		}
		
	//==================== Misc. ================================
		///////////// messages //////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "messages");
		if (p != NULL)
		{
			thegame->obits = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting messages (obits)\n");
		}
		///////////// autosave_period //////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "autosave_period");
		if (p != NULL)
		{
			thegame->autosave_period = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting autosave_period\n");
		}
		p = Ini_GetValue(&ini_file, "Game Parameters", "cruise_speed");
		if (p != NULL)
		{
			cruise_factor = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting cruise_factor\n");
		}
		p = Ini_GetValue(&ini_file, "Game Parameters", "stagger_weapon_changes");
		if (p != NULL)
		{
			thegame->stagger_weapon_changes = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting stagger_weapon_changes\n");
		}

	//=============== NN structure GA parameters ===================
		///////////// struct_evolve_on //////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "struct_evolve_on");
		if (p != NULL)
		{
			thegame->struct_evolve_on = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting struct_evolve_on\n");
		}
		///////////// structure_evolve_period //////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "structure_evolve_period");
		if (p != NULL)
		{
			thegame->structure_evolve_period = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting structure_evolve_period\n");
		}
		////////////// structure_mutate_chance /////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "structure_mutate_chance");
		if (p != NULL)
		{
			thegame->STRUCTURE_CHANCE_MUTATE = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting structure_mutate_chance\n");
		}
		////////////// structure_num_synapses_to_mutate ////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "structure_num_synapses_to_mutate");
		if (p != NULL)
		{
			thegame->STRUCTURE_MAX_NUM_MUTATE = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting structure_num_synapses_to_mutate\n");
		}
		////////////// structure_initial_num_nosynapses ////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "initial_num_nosynapses");
		if (p != NULL)
		{
			thegame->initial_num_nosynapses = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting initial_num_nosynapses\n");
		}


    }


}


/*======================================================================
BotReadIniFile
------------------------------------------------------------------------
reads in parameter values for bot from the .ini (initialization text file) file
named nb.ini.
======================================================================*/
void BotReadIniFile(edict_t* bot)
{
	int i;
	char *p; 
	float newvalue;

	i = Ini_ReadIniFile("nb.ini", &ini_file);
	
	if(i != 1) // if file could not be read
	{
		safe_bprintf (PRINT_HIGH,"ini file stuffed up\n");
		return;
	}
	
		// first we make sure that the .ini file is open
    if (ini_file.ini_file_read)      
	{

		safe_bprintf (PRINT_HIGH,"==reading in from nb.ini...==\n");

		////////////// hidden neurons per hidden layer /////
		p = Ini_GetValue(&ini_file, "Game Parameters", "hidden_neurons");
		if (p != NULL)
		{
			newvalue = atoi(p);
			//safe_bprintf (PRINT_HIGH,"	setting hidden_neurons\n");

			bot->client->botinfo.itsNet.HIDDEN_NEURONS = newvalue;
		}
		////////////// max_weight_x100 /////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "max_weight_x100");
		if (p != NULL)
		{
			newvalue = atoi(p) / 100;
			//safe_bprintf (PRINT_HIGH,"	setting max_weight_x100\n");

			bot->client->botinfo.itsNet.MAX_WEIGHT = newvalue;
		}
		////////////// min_weight_x100 /////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "min_weight_x100");
		if (p != NULL)
		{
			newvalue = atoi(p) / 100;
			//safe_bprintf (PRINT_HIGH,"	setting min_weight_x100 to %i\n",(long)newvalue);

			bot->client->botinfo.itsNet.MIN_WEIGHT = newvalue;
		}
		////////////// max_charge_x100 /////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "max_charge_x100");
		if (p != NULL)
		{
			newvalue = atoi(p) / 100;
			//safe_bprintf (PRINT_HIGH,"	setting max_charge_x100\n");

			bot->client->botinfo.itsNet.MAX_CHARGE = newvalue;
		}	
		////////////// threshold_x100 /////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "threshold_x100");
		if (p != NULL)
		{
			newvalue = atoi(p) / 100;
			//safe_bprintf (PRINT_HIGH,"	setting threshold_x100\n");

			bot->client->botinfo.itsNet.THRESHOLD = newvalue;
		}
		////////////// discharge_amount_x100 /////////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "discharge_amount_x100");
		if (p != NULL)
		{
			newvalue = atoi(p) / 100;
			//safe_bprintf (PRINT_HIGH,"	setting discharge_amount_x100\n");

			bot->client->botinfo.itsNet.DISCHARGE_AMOUNT = newvalue;
		}
	
	
    }

}

/*============================================================================
safe print functions
--------------------
these are used to avoid the crash caused by printing to a bot entity
============================================================================*/

void safe_centerprintf (edict_t *ent, char *fmt, ...)
{
	char bigbuffer[0x10000];
	va_list  argptr;
	int len;

	if(!ent->inuse || ent->client->botinfo.isbot)
	  return;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	gi.centerprintf(ent, bigbuffer);


}

void safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...)
{
	char bigbuffer[0x10000];
	va_list  argptr;
	int len;

	if(ent != NULL)	//a dedicated server print message has ent == NULL
	{
		if(!ent->inuse || ent->client->botinfo.isbot)
		  return;
	}

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);

	gi.cprintf(ent, printlevel, bigbuffer);


}







// botsafe bprintf
void safe_bprintf (int printlevel, char *fmt, ...)
{
	int i;
	char bigbuffer[0x10000];//NOTE: causing stack overflow?
	int  len;
	va_list  argptr;
	edict_t *cl_ent;

	va_start (argptr,fmt);
	len = vsprintf (bigbuffer,fmt,argptr);
	va_end (argptr);


	if (dedicated->value)
		gi.cprintf(NULL, printlevel, bigbuffer);

	// This is to be compatible with Eraser (ACE)
	//for (i=0; i<num_players; i++)
	//{
	// Ridah, changed this so CAM works
	for (i=0 ; i<maxclients->value ; i++)
	{
		cl_ent = g_edicts + 1 + i;

		if(cl_ent->inuse && cl_ent->client && !cl_ent->client->botinfo.isbot)
			gi.cprintf(cl_ent, printlevel, bigbuffer);
	}

}

/*
=======================================================
stuffcmd
-------

QC equivalent, sends a command to the client's consol

from the erasor bot source code
======================================================
*/
void stuffcmd(edict_t *ent, char *text)
{
	gi.WriteByte(11);				// 11 = svc_stufftext
	gi.WriteString(text);
	gi.unicast(ent, 1);
}



/*============================================================================
botspawner_think
----------------
the botspawner entity is used to spawn bots into the game in a staggered fashion
============================================================================*/

void botspawner_think(edict_t* self)
{
			
	if(thegame->bot_spawn_index < thegame->NUM_BOTS)	//if still bots to spawn
	{
		if(thegame->bot_spawn_index < (maxclients->value - 1) ) //if we have not run out of clients to use
		{
			thegame->Neuralbot_Array[thegame->bot_spawn_index] = Addbot(thegame->bot_spawn_index);//spawn a bot into the map

			//NOTE: check if null?
			initializeBot(thegame->Neuralbot_Array[thegame->bot_spawn_index]);

			thegame->bot_spawn_index++;

			self->nextthink = level.time + 0.1; //spawn another bot soon
		}
		else	// The console variable maxclients, the maximum number of players alowed in the game, 
		{		// is set to lower than NUM_BOTS.  So stop spawning bots and decrease NUM_BOTS
				// to how many bots there actually are.
		
			safe_bprintf(PRINT_HIGH, "can't spawn bot, quit + set maxclients to a higher value \n");
			thegame->NUM_BOTS = thegame->bot_spawn_index;

			safe_bprintf(PRINT_HIGH, "setting NUM_BOTS to %i\n", thegame->NUM_BOTS);
			
			//PrintTipOfTheDay(self->owner, true);

			G_FreeEdict(self);	//don't spawn any more bots

			
		}
	}
	else //if spawned all bots now
	{
		//PrintTipOfTheDay(self->owner, true);

		G_FreeEdict(self);	//free the spawner entity	

	}


}
/*===============================================================
areAllBotsSpawned
-----------------
returns true if all the bots have been spawned into the game
===============================================================*/
qboolean areAllBotsSpawned()
{	
	if(thegame->bot_spawn_index >= thegame->NUM_BOTS)
		return true;
	else
		return false;
}

void Cmd_Quickevolve_f()
{
	if(thegame->quickevolve)
	{
		thegame->quickevolve = false;
		safe_bprintf(PRINT_HIGH, "quickevolve set to OFF\n");
	}
	else
	{
		thegame->quickevolve = true;
		safe_bprintf(PRINT_HIGH, "quickevolve set to ON\n");
	}
}

void Cmd_Lookupanddown_f()
{
	if(thegame->lookupanddown)
	{
		thegame->lookupanddown = false;
		safe_bprintf(PRINT_HIGH, "lookupanddown set to OFF\n");
	}
	else
	{
		thegame->lookupanddown = true;

		//lookupanddownChangeWeights();

		safe_bprintf(PRINT_HIGH, "lookupanddown set to ON\n");
	}
}
void Cmd_Allowcrouch_f()
{
	if(thegame->allowcrouch)
	{
		thegame->allowcrouch = false;
		safe_bprintf(PRINT_HIGH, "allowcrouch set to OFF\n");
	}
	else
	{
		thegame->allowcrouch = true;

		safe_bprintf(PRINT_HIGH, "allowcrouch set to ON\n");
	}
}
void Cmd_Allowcenterview_f()
{
	if(thegame->allowcenterview)
	{
		thegame->allowcenterview = false;
		safe_bprintf(PRINT_HIGH, "allowcenterview set to OFF\n");
	}
	else
	{
		thegame->allowcenterview = true;
		safe_bprintf(PRINT_HIGH, "allowcenterview set to ON\n");
	}
}
void lookupanddownChangeWeights()
{
	int b;
	int s;

	for(b=0; b<thegame->NUM_BOTS; b++)//for all the bots
		for(s=0; s<thegame->Neuralbot_Array[b]->client->botinfo.itsNet.num_neurons_in_layer[thegame->SYNAPSE_LAYERS - 1]; s++)	//for all the neurons in the 2nd to last layer
		{
			thegame->Neuralbot_Array[b]->client->botinfo.itsNet.Synapse_Array[s][22][thegame->SYNAPSE_LAYERS - 1].weight = 0;//set the synapses to look_up neuron to 0
			thegame->Neuralbot_Array[b]->client->botinfo.itsNet.Synapse_Array[s][23][thegame->SYNAPSE_LAYERS - 1].weight = 0;//set the synapses to look_down neuron to 0
		}

}

/*=============================================================
botgame_setNumHiddenNeurons
---------------------------
call this to change the HIDDN_NEURONS variable in each bots' net
=============================================================*/
void botgame_setNumHiddenNeurons(int h)
{
	int i;

	for(i=0; i<thegame->NUM_BOTS; i++)
		net_setNumHiddenNeurons(&thegame->Neuralbot_Array[i]->client->botinfo.itsNet, h);
	
}

/*=============================================================
net_setNumHiddenNeurons
---------------------------
sets the HIDDEN_NEURONS variable in 'net' to 'h',
and recalculates the num. neurons in layer array for the net
=============================================================*/
void net_setNumHiddenNeurons(NeuralNet* net, int h)
{
	net->HIDDEN_NEURONS = h;

	initializeNNIL(net);//recalculate 'num neurons per layer' array

}


/*=============================================================
botgame_SetSynapseLayers
------------------------
call this to change the number of synapse layers of all the bots
=============================================================*/
void botgame_SetSynapseLayers(int l)
{
	int i;

	thegame->SYNAPSE_LAYERS = l;

	for(i=0; i<thegame->NUM_BOTS; i++)
	{
		net_SetSynapseLayers(&thegame->Neuralbot_Array[i]->client->botinfo.itsNet, l);
	}
}


/*=============================================================
botgame_SetSynapseLayers
------------------------
call this to change the number of synapse layers of a net
=============================================================*/
void net_SetSynapseLayers(NeuralNet* net, int l)
{
	net->SYNAPSE_LAYERS = l;

	initializeNNIL(net);
}


/*=============================================================
Cmd_Evolution_f
------------------------
toggle whether to run the main GA
=============================================================*/
void Cmd_Evolution_f()
{
	if(thegame->evolution)
	{
		thegame->evolution = false;
		safe_bprintf(PRINT_HIGH, "main GA toggled to not running\n");
	}
	else
	{
		thegame->evolution = true;
		safe_bprintf(PRINT_HIGH, "main GA toggled to running\n");
	}
}




