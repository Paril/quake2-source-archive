

#define	HISTORY_PERIOD	5
#define MAX_BOTS 100


/*=======================================================================
Synapse type
-------------------------------------------------------------------------
=======================================================================*/


typedef struct
{
	float	weight;

}Synapse;


/*=======================================================================
Neuron type
-------------------------------------------------------------------------
=======================================================================*/


typedef struct
{
	
	float	input;

	float	activation;

	int		type;

	float	activationlastinstant;

	int		thinkfuncnum;

	//void	(*think)(int x, int layer, NeuralNet* net);
	void		(*think)(int x, int layer, ...);


} Neuron;













/*=======================================================================
NeuralNet type
-------------------------------------------------------------------------
each bot/human player owns one NeuralNet.
=======================================================================*/

typedef struct
{
	
	int generation;	// generation of the neuralnet
	

	
	float	MAX_WEIGHT;// = 1;
	float	MIN_WEIGHT;// = -1;

	float	MAX_CHARGE;// = 3;

	float	THRESHOLD;// = 1;// the charge that has to build up
				// on a neuron before it starts conducting
	float	DISCHARGE_AMOUNT;// = 0.1;
						
	int		HIDDEN_NEURONS;	// num hidden neurons per layer

	int		SYNAPSE_LAYERS;

	Synapse Synapse_Array[60]	// 10000+ synapses! better than neuralbot Version.classicquake who had 20 :)
		[60][4];

	//Synapse birthWeights[60]	
	//	[60][4];

	Neuron Neuron_Array[60][5];
	/*
	Synapse Synapse_Array[NUM_NEURONS_IN_LAYER]
		[NUM_NEURONS_IN_LAYER][SYNAPSE_LAYERS];

	Neuron Neuron_Array[NUM_NEURONS_IN_LAYER][SYNAPSE_LAYERS + 1]
	*/
	qboolean	shorttermlearning; //is short term learning enabled?
	float		STL_learningrate;

	float		totalstlweightchange;

	float		reinforcement;

	int			num_neurons_in_layer[5];


}NeuralNet;



typedef struct
{

	///type of GA ///
	int GAtype;

	int crossover_type;//type of crossover operation
	int parentselection_type;

	//// NN training parameters /////
	float evolve_time; // time to call Evolve()
	long generation;   // how many evolves have taken place + 1
	qboolean quickevolve;//quickevolve on/off
	float quickevolve_threshold;//fitness needed to be reached

		//adaptive evolve period//
	float sumfitnesstarget;	//sumfitness to aim for
	float evolve_period_ceiling;//period to not go above 

	//// NN structure GA parameters ///
	qboolean struct_evolve_on;
	float structure_evolve_time;
	float structure_evolve_period;
	long structure_generation;

	///// fitness function: /////
	float itemscore;
	float fragscore;
	float damagescore;
	float explorescore;


	qboolean evolution;//toggle whether ot run the main GA or not
	int		freeweapon;
	qboolean obits;	// obituaries on/off
	qboolean enemy_trail;
	qboolean item_trail;
	qboolean telefrag_points; // telefrag frags count towards fitness function on/off
	long	autosave_period; //gens between autosaves, 0=autosave off
	qboolean lookupanddown;	//can bots look up and down
	qboolean allowcrouch;//can bots crouch
	qboolean allowcenterview;//can bots use the centerview command
	qboolean stagger_weapon_changes;//if true, bots can't change weapons for about 2 secs after last changing weapon.

	
	int		NUM_NEURONS_IN_LAYER; // number of neurons in one layer
	int		SYNAPSE_LAYERS;  // Number of synapse layers; one less than the 
							// number of neuron layers 

	//// GA parameters ////
	float	CHANCE_MUTATE;// = 10;	//  percentage 
				// chance of mutation per child when it is created
	int		MAX_NUM_MUTATE;// = 20;	// maximum number of synapses that can 
									// mutate

	float	CON_PROB_MUTATE; // prob per synapse of mutation

	qboolean smallmutate;
	float	small_mutate_amount;

	//// neuron parameters ////////			for neuron mutation
	float	NEURON_CHANCE_MUTATE;

	int		NEURON_MAX_NUM_MUTATE;

	float	spinner_exponent;	//for spinner/roullette selection mechanism


	//// NN structure training parameters ////
	float	STRUCTURE_CHANCE_MUTATE;// = 10;	//  percentage 
				// chance of mutation per child when it is created

	int		STRUCTURE_MAX_NUM_MUTATE;// = 20;	// maximum number of synapses that can 
									// mutate
	int		initial_num_nosynapses;
	
	int		NUM_BOTS;//number of bots to be spawned/that are in game.  Does not include reference bots


	///// This is the array of pointers to edict_t structures///////
	edict_t* Neuralbot_Array[MAX_BOTS];
		//reference bots are not pointed to with this array
	


	/////// learning history data /////////
	float	sumfitness_history[200];
		// Records the average sumfitness over the last 5 generations every
		// 5 generations

	float	last5_sumfitness[HISTORY_PERIOD];
		// Records the sumfitness per generation of the last 5 generations.
	float	last5_period[HISTORY_PERIOD];
		//Records the evolve_period for the last 5 generations

	long	history_index;	// The place in the sumfitness_history array 
							// to write to next






	float	stl_dweight;

	long	mutant_successes;
	long	mutant_failures;

	int		bot_spawn_index;//for staggered bot spawns

	float	last_evolve_time;
	float	next_evolve_check_time;

	long	real_last_evolve_time;//actual clock time of last evolve

	int		refbotnum;

} Botgame;



////////////////////// g_cmds.c ////////////////////////


////////////// input functions: nb_in.c /////////////////////

	/// stuff for input functions /////
	float calcEnemyYawDif(edict_t* self);
	float calcItemYawDif(edict_t* self);
	float calcEnemyPitchDif(edict_t* self);

	edict_t* choose_target(edict_t* self);
	edict_t* choose_targetitem(edict_t* self);

		//////actual input functions/////
	float check_item_justatleft(edict_t* self, float difference);
	float check_item_justatright(edict_t* self, float difference);
	float check_item_atleft(edict_t* self, float difference);
	float check_item_atright(edict_t* self, float difference);
	float check_item_ahead(edict_t* self, float difference);


	float small_random(edict_t* self);
	float medium_random(edict_t* self);
	float large_random(edict_t* self);
	float bell_random(edict_t* self);
	float upper_bell_random(edict_t* self);
	float lower_bell_random(edict_t* self);
	float medium_bell_random(edict_t* self);
	float allwaysfire(edict_t* self);
	float check_f_blaster(edict_t* self);
	float bot_fired_rocket(edict_t* self);

	float check_self_pain(edict_t* self); 
	float check_just_fired(edict_t* self);
									// some traclines inputs:
	float traceahead_enemy(edict_t* self);

	float traceright_wall(edict_t* self);
	float traceleft_wall(edict_t* self);

	float traceahead_lava(edict_t* self);
	float traceahead_wall(edict_t* self);
	float traceahead_entity(edict_t* self);
	float traceahead_water(edict_t* self);
					// trace ahead and down - eqivalent looking at  bottom of view screen
	float tracediag_lava(edict_t* self);
	float tracediag_wall(edict_t* self);
	float tracediag_entity(edict_t* self);
	float tracediag_water(edict_t* self);

	float tracehalfleft_wall(edict_t* self);
	float tracehalfright_wall(edict_t* self);

	float boxtraceahead_close (edict_t* self);
	float boxtraceahead_far (edict_t* self);

	float boxtraceleft_wall(edict_t* self);
	float boxtraceright_wall(edict_t* self);
	float boxtracehalfleft_wall(edict_t* self);
	float boxtracehalfright_wall(edict_t* self);

	////// target position in bot viewspace: ///////
	float enemy_jump(edict_t* self); 
	float enemy_duck(edict_t* self);
	float check_far_left(edict_t* self, float difference); // is oppenent at the left of nbot's view space?
	float check_far_right(edict_t* self, float difference);
	float check_at_left(edict_t* self, float difference); // of view space
	float check_at_right(edict_t* self, float difference);

	float check_justat_left (edict_t* self, float difference);
	float check_justat_right (edict_t* self, float difference);

	float enemypitch_near0(edict_t* self, float pitchdif);
	float enemyjust_up(edict_t* self, float difference);
	float enemyjust_down(edict_t* self, float difference);
	float enemy_up(edict_t* self, float difference);
	float enemy_down(edict_t* self, float difference);
	float enemylots_up(edict_t* self, float difference);
	float enemylots_down(edict_t* self, float difference);

	float check_ahead_enemy (edict_t* self, float difference);

	float check_inflicted_pain(edict_t* self);//for pshycopathic pain association :) (damn i can't spell that)
	float check_inflicted_death(edict_t* self);// ditto :)

	float check_f_shotgun(edict_t* self); // opponent fires shotgun. 
	float check_f_sshotgun(edict_t* self); 
	float check_f_mgun(edict_t* self);
	float check_f_chaingun(edict_t* self);
	float check_f_grenadel(edict_t* self);
	float check_f_rocketl(edict_t* self);
	float check_f_hyperb(edict_t* self);
	float check_f_railgun(edict_t* self);
	float check_f_bfg10k(edict_t* self);
	float check_f_grenade(edict_t* self); // hand grenade
	/*									// standing on:
	float check_platform(edict_t* self);
	float check_normal(edict_t* self);
	float check_lava(edict_t* self);
	float check_nothing(edict_t* self);
	float check_ladder(edict_t* self);
	*/						// standing in water:
	float check_feet(edict_t* self);
	float check_waist(edict_t* self);
	float check_head(edict_t* self);
	/*								// hearing sense:
	float check_noise_footsteps(edict_t* self);
	float check_noise_firing(edict_t* self);
	*/

	float was_blocked(edict_t* self);


	float holding_blaster(edict_t* self);
	float holding_shotgun(edict_t* self);
	float holding_sshotgun(edict_t* self);
	float holding_mgun(edict_t* self);
	float holding_chaingun(edict_t* self);
	float holding_grenadel(edict_t* self);
	float holding_rocketl(edict_t* self);
	float holding_hyperb(edict_t* self);
	float holding_railgun(edict_t* self);
	float holding_bfg10k(edict_t* self);
	float holding_grenade(edict_t* self);

	float health_low(edict_t* self);
	float health_medium(edict_t* self);

	float check_enemy_close(edict_t* self);
	float check_item_close(edict_t* self);


	float enemy_moving_left(edict_t* self, float heading);
	float enemy_moving_right(edict_t* self, float heading);


	/////////////output functions nb_out.c ////////////////////////
	void select_blaster(edict_t* self);
	void select_shotgun(edict_t* self);
	void select_sshotgun(edict_t* self);
	void select_mgun(edict_t* self);
	void select_chaingun(edict_t* self);
	void select_grenadel(edict_t* self);
	void select_rocketl(edict_t* self);
	void select_hblaster(edict_t* self);
	void select_railgun(edict_t* self);
	void select_bfg10k(edict_t* self);

	void fire_weapon(edict_t* self);
//	void throw_grenade(edict_t* self);
	void fire_railgun(edict_t* self);
	
		/*
	thegame->oFunction_Array[13] =
	thegame->oFunction_Array[14] =
	thegame->oFunction_Array[15] =
	thegame->oFunction_Array[16] =
	thegame->oFunction_Array[17] =
	thegame->oFunction_Array[18] =
	thegame->oFunction_Array[19] =
	*/
	//////// movement /////////
	void bot_jump(edict_t* self);
	void bot_duck (edict_t* self);

	void go_forwards (edict_t* self);
	void go_back (edict_t* self);
	void go_right (edict_t* self);
	void go_left (edict_t* self);
	void bot_run (edict_t* self);


	//////// viewpoint change functions /////////
	void look_left(edict_t* self);
	void look_right(edict_t* self);

	void look_up(edict_t* self, float activation);
	void look_down(edict_t* self, float activation);

	void looklots_left(edict_t* self, float activation);
	void looklots_right(edict_t* self, float activation);

	void lookjust_up(edict_t* self, float activation);
	void lookjust_down(edict_t* self, float activation);

	void looklots_up(edict_t* self, float activation);
	void looklots_down(edict_t* self, float activation);

	void look_straight(edict_t* self);

	void lookjust_left(edict_t*self, float activation);
	void lookjust_right(edict_t*self, float activation);

	void looktiny_left(edict_t*self, float activation);
	void looktiny_right(edict_t*self, float activation);

	void lookheaps_left(edict_t*self, float activation);
	void lookheaps_right(edict_t*self, float activation);


	/// defines for output functions ////////
	#define LOOKUP_JUST		4
	#define LOOKUP			15
	#define LOOKUP_LOTS		50

	#define TURN_TINY		0.5
	#define TURN_JUST		5
	#define TURN_MEDIUM		10
	#define TURN_LOTS		20
	#define TURN_HEAPS		60	// this contols the bots' max turning rate NOTE: was 85

////////////// nb_think.c /////////////////////
	void bot_think ( edict_t* self);


////////////// nb_spawn.c ////////////////////
	edict_t* bot_GetLastFreeClient (void);

	//void Respawn (edict_t *self, edict_t *enemy); 
	void Bot_Respawn(edict_t *ent);

	//void CopyToBodyQue (edict_t* self);
	//void  SelectSpawnPoint (edict_t*self, vec3_t spawn_origin, vec3_t spawn_angles);


	

typedef struct
{
	float nextwaypointdroptime;
	float waypointdropperiod;
	vec3_t waypoints[5];
	int NUM_WAYPOINTS;
	int index;	//circular array


} exploreinfo_t;


 // this is part of the client structure and is saved across deathmatch respawns
typedef struct
{
	
	// Neuralbot fragcounter for the period since the last evolutionary
	// step
	int         fragslastperiod;
	int			structure_fragslastperiod;

	int			itemsgot;	// num items picked up since last run of the GA
	int			structure_itemsgot;

	long		damageinflicted;
	long		structure_damageinflicted;

	float		explorenesssofar;
	float		structure_explorenesssofar;

	// stuff for movement //
	int			forwardvotes;
	int			rightvotes;
	qboolean	runvote;

	edict_t*	targetitem;	// item bot is locked on to.
	qboolean	Botrunning;	// is bot running?

	qboolean	isbot;	//true if ent is a bot

	qboolean	wasblocked;//if bot ran into an obstacle while moving

	//--------stuff for input functions--------
	float		lastpaintime;		// used by check_self_pain

	float		lastinflictedpaintime;
	float		lastinflicteddeathtime;
	
	float		oldEnemyHeading;
	//-----------------------------------------

	qboolean	invisible;	//invisible to bots

	exploreinfo_t exploreinfo;

	usercmd_t	ucmd;	//for storing commands bot wants to execute

	float		last_changeweapon_time; //time bot last changed weapon

	qboolean	isrefbot;

	NeuralNet	itsNet;	// bots use this neuralnet as their brain, human players use it for imitation learning

	float		success;	// calculated when needed, stored temporarily here
	qboolean	mutant;	// dna mutated when it was created?


} nb_info;


extern	float	acceleration_factor;
	// number of server-thinks called per previous server-think
	//	 (see G_RunFrame) in g_main.c

extern float	cruise_factor;

extern int RunFrame_counter;
	// number of G_RunFrame calls that have been made.


///////////// NEWCODE: Neuralbot weapon defines ///////////////////////
//NOTE: not used
#define NB_BLASTER	1
#define NB_SHOTGUN	2
#define NB_SSHOTGUN 3
#define NB_MGUN		4
#define NB_CHAINGUN	5
#define NB_GRENADEL	6
#define NB_ROCKETL	7
#define NB_HYPERB	8
#define NB_RAILGUN	9
#define NB_BFG10K	10
#define NB_GRENADE	11

#define NOSYNAPSE	666 // the weight a synapse has when it doesn't exist

/////// GA types ////
#define GA_NORMAL	1
#define GA_ONECHILD 2
#define GA_TOPOLOGY	3
#define GA_DISTANCE	4

////// crossover operation types ///////
#define CO_LINEAR	1
#define CO_VERTICAL 2

///// parent selection types //////////
#define PS_ROULETTE 1
#define PS_BEST		2
#define PS_RANK		3

// GAinfo: this is information that all the different sub-functions of the genetic algorithm share
typedef struct
{
	int parent1;
	int parent2;
	int child1;
	int child2;
	float sumfitness;
	int NUM_SYNAPSES;

	// IMPORTANT NOTE: the number of floats has to be manually set to 
	// be at least the maximum possible number of synapses

	// SYNAPSENUMDEPENDENT // u can do a search for this word (a sign of quality programming :)
	float child1_chromosome[20000];
	float child2_chromosome[20000];
	float parent1_chromosome[20000];
	float parent2_chromosome[20000];

	Neuron	parent1_neuron_chr[300];
	Neuron	parent2_neuron_chr[300];
	Neuron	child1_neuron_chr[300];
	Neuron	child2_neuron_chr[300];


}GAinfo;

////////////nb_move.c ///////////////////
	int	bot_move(edict_t *self, qboolean run, int forwards, int right);
	void bot_executemove(edict_t* bot, usercmd_t* cmd);
	void bot_clearmovevotes(edict_t* bot);

//////////nb_GAonechild.c////////////////////
	void GAonechild();

////////// nb_geneticalgorithm.c://////////////
	void GAnormal();
	void Evolve();
	void evaluate_success(edict_t* bot);

	qboolean calculateSumfitness(GAinfo* g);
	qboolean chooseparents_Roulette(GAinfo* g);
	qboolean chooseparents_Best(GAinfo* g);
	qboolean chooseparents_Rank(GAinfo* g);
	qboolean generateParentChromosomes(GAinfo* g);
	qboolean generateParentNeuronChromosomes(GAinfo* g);
	qboolean linear_crossover(GAinfo* g);
	qboolean linear_neuron_crossover(GAinfo* g);
	qboolean vertical_crossover(GAinfo* g);
	qboolean discretemutate(GAinfo* g);
	qboolean discreteneuronmutate(GAinfo* g);
	qboolean continuousmutate(GAinfo* g);
	qboolean choosechildren_Worst(GAinfo* g);
	qboolean overwriteChildNNs(GAinfo* g);
	qboolean overwriteChild1Neurons(GAinfo* g);
	qboolean overwriteChild2Neurons(GAinfo* g);
	int choose_bot(qboolean* cand_array);
	void bot_pickup_item(edict_t* self);
	void clear_scoreslastperiod(edict_t* bot);
	void clear_itemsgot(edict_t* bot);
	void clear_damageinflicted(edict_t* bot);
	float getSumFitness();
	qboolean shouldRunGA();
	void doMutationSuccesses(GAinfo* g, qboolean onechildGA);
	float smallWeightMutate(float currentweight);

/////////////// nb_structGA.c ///////////////////
	void Struct_evaluate_success(edict_t* bot);
	void Struct_clear_botfrags(edict_t* bot);
	void Struct_Evolve ();
	void Struct_DNAswapandmutate();
	qboolean struct_generateParentChromosomes(GAinfo* g);
	qboolean struct_mutate(GAinfo* g);
	qboolean struct_overwriteChildNNs(GAinfo* g);


typedef struct
{
	float inputvector[60];
	float outputvector[60];

} trainingpair_t;
// this is a training pair for training the backprop algorithm.  It records the inputs of a human player
// and the players outputs a little later (like human reaction time later)

typedef struct
{	//flags:
	qboolean	select_blaster_f;
	qboolean	select_shotgun_f;
	qboolean	select_sshotgun_f;
	qboolean	select_mgun_f;
	qboolean	select_chaingun_f;
	qboolean	select_grenadel_f;
	qboolean	select_rocketl_f;
	qboolean	select_hblaster_f;
	qboolean	select_railgun_f;
	qboolean	select_bfg10k_f;


	qboolean	fire_weapon_f;
	//qboolean	fire_railgun_f;

							// movement://
	qboolean	bot_jump_f;
	qboolean	bot_duck_f;

	qboolean	walk_forwards_f;
	qboolean	run_forwards_f;

	qboolean	walk_back_f;
	qboolean	run_back_f;

	qboolean	walk_left_f;	
	qboolean	run_left_f;

	qboolean	walk_right_f;
	qboolean	run_right_f;

	qboolean	look_left_f;
	qboolean	look_right_f;


	
	float	lookjust_up_float;	
	float	lookjust_down_float;

	float	look_up_float;
	float	look_down_float;

	float	looklots_up_float;
	float	looklots_down_float;


	float	looktiny_left_float;
	float	looktiny_right_float;

	float	lookjust_left_float;
	float	lookjust_right_float;

	float	looklots_left_float;
	float	looklots_right_float;

	float	lookheaps_left_float;
	float	lookheaps_right_float;

	qboolean	look_straight_f;



	//---------------------------

	
	
	//------- general stuff ---------
	int		sample_frames;	//number of client frames that outputs should be sampled for
	int		delay_frames;		//delay in client frames between sampling input and output
	int		waiting_frames;	

	int		sample_framecount;
	int		delay_framecount;
	int		waiting_framecount;

	int		imitation_state;

	trainingpair_t trainingpair;

} tpair_fsm_t;	//training pair finite state machine type





/*====================================================================
tpairarchive_t
--------------
this structure stores training pairs, and has various 'methods' related 
to it.
====================================================================*/
typedef struct
{
	trainingpair_t tpairarchive[10000];
	long MAX_TPAIRS;	
	long index;
	long rewriteindex;

} tpairarchive_t;

//////// tpairarchive_t functions //////////////
	// implemented in nb_imitation.c
	void initializeTPArchive(tpairarchive_t* tpa);
	trainingpair_t* getRandomTPair(tpairarchive_t* tpa);
	qboolean addTPair(tpairarchive_t* tpa, trainingpair_t* tpair);
	void deleteAllTPairs(tpairarchive_t* tpa);
	qboolean tpa_isFull(tpairarchive_t* tpa);
	qboolean tpa_isEmpty(tpairarchive_t* tpa);
	long tpa_getIndex(tpairarchive_t* tpa);
	int	pushTPair(tpairarchive_t* tpa, trainingpair_t* tpair);
	void overwriteTPair(tpairarchive_t* tpa, trainingpair_t* tpair);


typedef struct
{
	tpair_fsm_t tpair_fsm[10];	//10 backprop training pair gatherers operating in parallel. awesome :)
	int			NUM_FSMS;

	qboolean	imitation_on;

	float		learning_rate;	//learning rate for back-prop algorithm

	vec3_t		old_v_angle;

	long		upload_period;	// video frames between 'human to bot' weight uploads	
	long		upload_period_frames;	//counter for frames elapsed this upload period.

	int			num_bp_runs_per_frame;	//number of back-prop runs per human video-frame

	qboolean	inputtest;	
	qboolean	outputtest;
	qboolean	showsumsquared;

	qboolean	sampling;	//sample the players inputs/outputs?

	tpairarchive_t* tpairarchive_p;

	///////// stuff for view orientation change output functions ///////////
	float current_dyaw;		//updated at 10 Hz
	float current_dpitch;	//updated at 10 Hz

	float last_dyaw_time;


} iminfo_t;//imitation info

/// imitation states ////////
#define IM_SAMPLING_INPUT	0
#define IM_DELAYING			1
	// waiting to sample output
#define IM_SAMPLING_OUTPUT	2

#define IM_PAUSING			3
	//for when you are dead :).. don't want the bots to copy that.
#define	IM_WAITING			4
// a timed pause, for staggering the phase of the fsms relative to each other



//////// nb_backprop2.c /////////
	float stepActFunc(float x);
	float doBackProp1(edict_t* self, trainingpair_t* tpair);
	float doBackProp2(edict_t* self, trainingpair_t* tpair);
	float activationFunction(float x);
	float activationFunctionPrimed(float x);





//////// nb_imitation.c ///////////

	//void CopyDynWeightsToBirthWeights(NeuralNet* net);
	//void CopyBirthWeightsToDynWeights(NeuralNet* net);

	//void CopyBirthWeightsOver(GAinfo* g);
	//void CopyDynOvertoBirthWeights(GAinfo* g);

	void Cmd_CreateArchive_f(edict_t* self);

	//void getChangeWeaponOutput(edict_t* self, gitem_t* newitem);	//declared in p_client.c
	//void AssignNBDefinesForWeapon(edict_t* self, gitem_t* weapon);
	//void getMoveOutput(usercmd_t* cmd, edict_t* self);

	void resetImitationOutputFlags(tpair_fsm_t* fsm);

	void fillInputs(float* inputs, edict_t* self);
	void fillOutputs(edict_t* self, tpair_fsm_t* fsm);

	void initializeOldYaw(edict_t* self);
	void calculateYawChange(edict_t* self);

	void uploadHumanNNWeightsToBots(edict_t* ent);
	void downloadBotNNWeightsToHuman(edict_t* self);

	void initializeHumanNN(edict_t* self);

	void inititalizeImitation(edict_t* self);
	void getImitationStuffFromIni(edict_t* self);
	void shutdownImitation(edict_t* self);
	qboolean imitationOn(edict_t* self);

	void RandomizeBPnet(edict_t* self);

	void maintainCurrentDYaw(edict_t* self);
	void maintainCurrentDPitch(edict_t* self);

	void setFlagsForCurrentDYaw(edict_t* self);
	void setFlagsForCurrentDPitch(edict_t* self);

///////////nb_explore.c - exploring incentive/////////////////
	float	getExploreness(edict_t* self);
	void	initializeExploreinfo(edict_t* self, exploreinfo_t* ex);
	int		getIndex(exploreinfo_t* einfo);
	void	dropWaypoint(edict_t* self);
	void	exploreThink(edict_t* self, exploreinfo_t* einfo);


////////// nb_game.c /////////////////////////
	

	void creategame(); // spawn a new game
	edict_t* Addbot(int botnum);	//add a bot to the game
	void initialize_bot_synapses();// randomize weights
	void initialize_nnet_variables(edict_t* bot);// setup THRESHOLD etc..		
	void initialize_NN_structure(edict_t* bot);
	void initialize_history();
	void initialize_neurons(edict_t* bot);
						
	qboolean areAllBotsSpawned();
	void initializeBot(edict_t* bot);

	void BotgameReadIniFile();
	void BotReadIniFile(edict_t* bot);

	/// safe print functions ////
	void safe_centerprintf (edict_t *ent, char *fmt, ...);
	void safe_bprintf (int printlevel, char *fmt, ...);
	void safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...);
	void stuffcmd(edict_t *ent, char *text);

	void botspawner_think(edict_t* self);
	void PrintTipOfTheDay(edict_t* self, qboolean centerprint);
	void PrintIntro(edict_t* self, qboolean centerprint);

	void giveFreeWeapon(edict_t* ent);

	void initializeNNIL(NeuralNet* net);
	int getNeuronsForLayer(int layer, NeuralNet* net);

	void lookupanddownChangeWeights();

	void botgame_setNumHiddenNeurons(int h);
	void botgame_SetSynapseLayers(int l);
	void net_SetSynapseLayers(NeuralNet* net, int l);
	void net_setNumHiddenNeurons(NeuralNet* net, int h);

////////////// nb_neuronthink.c ////////////////////////////
	void NeuronMutate(Neuron* neuron);
	void NT_HOMOSYNAPTIC_1 (int x, int layer, NeuralNet* net);
	void NT_HOMOSYNAPTIC_2 (int x, int layer, NeuralNet* net);
	void NT_HOMOSYNAPTIC_3 (int x, int layer, NeuralNet* net);
	void NT_HOMOSYNAPTIC_4 (int x, int layer, NeuralNet* net);
	void NT_HOMOSYNAPTIC_5 (int x, int layer, NeuralNet* net);
	void NT_HETEROSYNAPTIC_1 (int x, int layer, NeuralNet* net);
	void NT_HETEROSYNAPTIC_2 (int x, int layer, NeuralNet* net);
	void NT_ASSOCIATIVE_1 (int x, int layer, NeuralNet* net);
	void NT_ASSOCIATIVE_2 (int x, int layer, NeuralNet* net);
	void NT_ASSOCIATIVE_3 (int x, int layer, NeuralNet* net);
	void NT_ASSOCIATIVE_4 (int x, int layer, NeuralNet* net);
	void NT_ASSOCIATIVE_5 (int x, int layer, NeuralNet* net);
	void NT_COVARIANCE (int x, int layer, NeuralNet* net);
	void NT_MODULATORY (int x, int layer, NeuralNet* net);
	void printThinkFuncForNeuron(Neuron* n);

//////////// nb_save.c //////////////////
	void WriteNN (char *filename);
	void ReadNN (char *filename);
	void WriteHistoryToFile(char* filename);
	void ReadHistoryFromFile(char* filename);
	qboolean WriteHistory(FILE* f);
	qboolean ReadHistory(FILE* f);
	void DoSumfitnessHistory(float sumfitness);



////////////// various commands ////////////////

	void Cmd_Lotsfaster_f();
	void Cmd_Lotsslower_f();	
	void Cmd_Faster_f();
	void Cmd_Slower_f();
	void Cmd_Normal_f();
	void Cmd_Cruise_f();

	qboolean bot_cmdcheck();
	void Cmd_Startgame_f();
		
	void Cmd_Evolve_f ();
	void Cmd_Struct_Evolve_f ();
	void Cmd_Purge_f();
	void Cmd_Zap_f();
	void Cmd_Report_f();

	void Cmd_Starttest_f();
	void Cmd_Test_f();
	void Cmd_Longer_f();
	void Cmd_Shorter_f();

	void Cmd_Botinfo_f();
	void Cmd_Hardwire_f();
	void Cmd_Freeweapon_f(qboolean svcmd);

	void Cmd_Obits_f();

	void Cmd_Savenn_f(qboolean svcmd);
	void Cmd_Loadnn_f(qboolean svcmd);

	void Cmd_Savehistory_f(qboolean svcmd);

	void Cmd_Enemytrail_f();
	void Cmd_Itemtrail_f();

	void Cmd_Imitation_f(edict_t* ent);

	void Cmd_Crossoverwith_f(qboolean svcmd);
	void Cmd_Invisible_f(edict_t* ent);

	void Cmd_Inputtest_f();
	void Cmd_Outputtest_f(edict_t* ent);

	void Cmd_Showsumsquared_f(edict_t* ent);
	void Cmd_Uploadweights_f(edict_t* ent);

	void Cmd_ClearArchive_f(edict_t* self);

	void Cmd_BotShowScores_f(edict_t* self);

	void Cmd_Sampling_f(edict_t* self);
	void Cmd_BPPerFrame_f(edict_t* self);

	void Cmd_Randomizebpnet_f(edict_t* self);

	void Cmd_Quickevolve_f();

	void Cmd_Lookupanddown_f();

	void Cmd_Addrefbot_f(qboolean svcmd);
	void Cmd_Removerefbots_f();
	void Cmd_Allowcrouch_f();
	void Cmd_Allowcenterview_f();

	void Cmd_Evolution_f();


/////// nb_refbot.c /////////////
	void Cmd_Addrefbot_f(qboolean svcmd);
	edict_t* AddRefBot(char* dnafile);
	qboolean SetUpRefBotNet(edict_t* self, char* dnafile);
	void refbot_think(edict_t* self);
	void removeRefBot(edict_t* ent);


	




///// global pointer to the botgame //////////
	extern Botgame* thegame;


