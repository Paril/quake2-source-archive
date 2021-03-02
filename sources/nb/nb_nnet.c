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

/*=============================================================================
gonetgo ( go net go )
-------------------------------------------------------------------------------
This function executes one cycle of the neural network.
First it calls all the input functions and charges up the first layer of
neurons by the value the input functions return.

Then all the neuron layers after the input layer are stepped through.
For each neuron in on of these layers, this happens:

	The neuron input is set to zero.  For each neuron in the layer above the neuron, the neuron's input
	is incremented by the activation of the neuron in the layer above it times the weight of the synapse 
	connecting the two neurons.

	The neuron activation/output is then set to the result of applying a sigmoidial 
	'activation' function to the neuron input.


All the neurons in the output layer are then checked to see if their activations 
are greater than a certain threshold. 
If so, then the corresponding output function is called.


	
This is what the neuron array looks like.  The horizontal position
of the neuron is the first dimension of the array, which row the neuron is on
is the second dimension.

		  neuron postion in row  [x][]

			  0   1  2

			 ____________
		0	|  |  |  |
			|__|__|__|__
neuron	1	|  |  |  |	
row			|__|__|__|_	
[][z]	2	|  |  |
			|__|__|___
		3	|  |
			|




This is a representation of a synapse array.  The first dimension is that of 
the row position of the source neuron, thatis what neuron the synapse is
coming from.  The second dimension is the row position destination neuron, 
that is the neuron the synapse is heading to.  The third dimension is the 
synapse layer, with layer 0 being at the top.




						source neuron row position [x][][]

destination				 0  1  2  3
neuron row	 			____________
position		  0	  /__/__/__/__/ | 
[][y][]			1	/__/__/__/__/ | |
			  2	  /__/__/__/__/ | |/|
			3	/__/__/__/__/ | |/| |
				|  |  |  |	| |/| |/|
synapse		0	|__|__|__|__|/| |/| |
layer			|  |  |  |	| |/| |/|
[][][z]		1	|__|__|__|__|/| |/| |
				|  |  |  |  | |/| |/
			2	|__|__|__|__|/| |/
				|  |  |  |  | |/
			3	|__|__|__|__|/


for example the synapse shown below would be synapse [1][0][1]

  @    @    @    @ 

  
  @    @    @    @ 
     /
   /  
  @    @    @    @ 

 
  @    @    @    @ 

This means the synapse in the second layer of synapses ( [][][1] ), 
going from the second neuron, neuron 1 ( [1][][] ), 
to the 1st neuron, neuron 0 ( [][0][] ).

===============================================================
*/
void gonetgo(NeuralNet*	net, edict_t* self)
{
	int x,layer,s;

	int neuron_layers = net->SYNAPSE_LAYERS + 1;

	///stuff for input functions ////
	float enemy_yaw_dif = calcEnemyYawDif(self);
	float item_yaw_dif =  calcItemYawDif(self);
	float enemy_pitch_dif = calcEnemyPitchDif(self);

	float tempthresh;
		

	/////////////// charge up input neurons /////////////////

	net->Neuron_Array[0][0].activation = allwaysfire(self);

	/// targetitem related inputs ///
	if(item_yaw_dif != 666)//item_yaw_dif of 666 means no item locked on to, hence no yaw
	{
		net->Neuron_Array[1][0].activation = check_item_close(self);

		net->Neuron_Array[2][0].activation = check_item_ahead(self, item_yaw_dif);

		net->Neuron_Array[3][0].activation = check_item_justatleft(self, item_yaw_dif);
		net->Neuron_Array[4][0].activation = check_item_justatright(self, item_yaw_dif);
		net->Neuron_Array[5][0].activation = check_item_atleft(self, item_yaw_dif);
		net->Neuron_Array[6][0].activation = check_item_atright(self, item_yaw_dif);
		
	}

	

	

								
	//net->Neuron_Array[12][0].activation = traceahead_lava(self);


	/// environmental inputs ///
	net->Neuron_Array[7][0].activation = boxtraceahead_close(self);
	net->Neuron_Array[8][0].activation = boxtraceahead_far(self);
	
	net->Neuron_Array[9][0].activation = boxtraceleft_wall(self);
	net->Neuron_Array[10][0].activation = boxtraceright_wall(self);

	net->Neuron_Array[11][0].activation = boxtracehalfleft_wall(self);
	net->Neuron_Array[12][0].activation = boxtracehalfright_wall(self);

	net->Neuron_Array[13][0].activation = traceahead_wall(self);
	net->Neuron_Array[14][0].activation = traceahead_entity(self);
	net->Neuron_Array[15][0].activation = traceahead_enemy(self);

	//net->Neuron_Array[15][0].activation = traceahead_water(self);

	//net->Neuron_Array[16][0].activation = traceleft_wall(self);
	//net->Neuron_Array[17][0].activation = traceright_wall(self);	//NOTE: 18 used

	net->Neuron_Array[16][0].activation = tracehalfleft_wall(self);
	net->Neuron_Array[17][0].activation = tracehalfright_wall(self);

	net->Neuron_Array[18][0].activation = tracediag_lava(self);
	net->Neuron_Array[19][0].activation = tracediag_wall(self);

	//net->Neuron_Array[18][0].activation = tracediag_entity(self);  probably useless right now
	net->Neuron_Array[20][0].activation = tracediag_water(self);


	/// general enemy inputs ///
	net->Neuron_Array[21][0].activation = check_enemy_close(self);	

	/// enemy actions ///
	net->Neuron_Array[22][0].activation = enemy_jump(self); 
	net->Neuron_Array[23][0].activation = enemy_duck(self);


	/// enemy position on screen in terms of yaw ///
	net->Neuron_Array[24][0].activation = check_ahead_enemy(self, enemy_yaw_dif);

	net->Neuron_Array[25][0].activation = check_justat_left(self, enemy_yaw_dif);
	net->Neuron_Array[26][0].activation = check_justat_right(self, enemy_yaw_dif);

	net->Neuron_Array[27][0].activation = check_at_left(self, enemy_yaw_dif); // of view space
	net->Neuron_Array[28][0].activation = check_at_right(self, enemy_yaw_dif);
	
	net->Neuron_Array[29][0].activation = check_far_left(self, enemy_yaw_dif); // is oppenent moving to the left of nbot's view space?
	net->Neuron_Array[30][0].activation = check_far_right(self, enemy_yaw_dif);

	/// enemy position on screen in terms of pitch ///
	net->Neuron_Array[31][0].activation = enemypitch_near0(self, enemy_pitch_dif);

	net->Neuron_Array[32][0].activation = enemyjust_up(self, enemy_pitch_dif);
	net->Neuron_Array[33][0].activation = enemyjust_down(self, enemy_pitch_dif);

	net->Neuron_Array[34][0].activation = enemy_up(self, enemy_pitch_dif);
	net->Neuron_Array[35][0].activation = enemy_down(self, enemy_pitch_dif);

	net->Neuron_Array[36][0].activation = enemylots_up(self, enemy_pitch_dif);
	net->Neuron_Array[37][0].activation = enemylots_down(self, enemy_pitch_dif);




	/// enemy movement across the bots' 'screens' ///
	net->Neuron_Array[38][0].activation = enemy_moving_left(self, enemy_yaw_dif);
	net->Neuron_Array[39][0].activation = enemy_moving_right(self, enemy_yaw_dif);
	

	/// enemy firing weapon inputs ///

	//NOTE: this stuff useless? especially hitscan weapons
	//net->Neuron_Array[8][0].activation = check_f_blaster(self);
	//net->Neuron_Array[30][0].activation = check_f_shotgun(self); // opponent fires shotgun. 
	//net->Neuron_Array[31][0].activation = check_f_sshotgun(self); 
	//net->Neuron_Array[32][0].activation = check_f_mgun(self);
	//net->Neuron_Array[33][0].activation = check_f_chaingun(self);
	net->Neuron_Array[40][0].activation = check_f_grenadel(self);
	net->Neuron_Array[41][0].activation = check_f_rocketl(self);
		//net->Neuron_Array[36][0].activation = check_f_hyperb(self);//NOTE: 36 used
	//net->Neuron_Array[37][0].activation = check_f_railgun(self);
	//net->Neuron_Array[38][0].activation = check_f_bfg10k(self);
	//net->Neuron_Array[39][0].activation = check_f_grenade(self); // hand grenade
										// standing on:
	//net->Neuron_Array[][0].activation = check_platform(self);
	//net->Neuron_Array[][0].activation = check_normal(self);
	//net->Neuron_Array[][0].activation = check_lava(self);
	//net->Neuron_Array[][0].activation = check_nothing(self);
	//net->Neuron_Array[][0].activation = check_ladder(self);
				
	//net->Neuron_Array[40][0].activation = small_random(self); getting rid of randoms right now
	//net->Neuron_Array[41][0].activation = medium_random(self);
	//net->Neuron_Array[42][0].activation = large_random(self);
	//net->Neuron_Array[3][0].activation = bell_random(self);


		/////// standing in water: ////////
	//net->Neuron_Array[45][0].activation = check_feet(self);
	//net->Neuron_Array[46][0].activation = check_waist(self);
	//net->Neuron_Array[47][0].activation = check_head(self);
									//45 used!
	//net->Neuron_Array[46][0].activation = was_blocked(self);

									// hearing sense:
	//net->Neuron_Array[][0].activation = check_noise_footsteps(self);
	//net->Neuron_Array[][0].activation = check_noise_firing(self);



	/// inputs dependant on current state of bot ///	
	net->Neuron_Array[42][0].activation = check_self_pain(self); 
	net->Neuron_Array[43][0].activation = check_just_fired(self);

	net->Neuron_Array[45][0].activation = check_inflicted_pain(self);
	net->Neuron_Array[46][0].activation = check_inflicted_death(self);	

	/// inputs dependant on current state of bot ///
	net->Neuron_Array[47][0].activation = health_low(self);
	net->Neuron_Array[48][0].activation = health_medium(self);

	/// bot holding weapon inputs ///
	net->Neuron_Array[49][0].activation = holding_grenade(self);
	net->Neuron_Array[50][0].activation = holding_blaster(self);
	net->Neuron_Array[51][0].activation = holding_shotgun(self);
	net->Neuron_Array[52][0].activation = holding_sshotgun(self);
	net->Neuron_Array[53][0].activation = holding_mgun(self);
	net->Neuron_Array[54][0].activation = holding_chaingun(self);
	net->Neuron_Array[55][0].activation = holding_grenadel(self);
	net->Neuron_Array[56][0].activation = holding_rocketl(self);
	net->Neuron_Array[57][0].activation = holding_hyperb(self);
	net->Neuron_Array[58][0].activation = holding_railgun(self);
	net->Neuron_Array[59][0].activation = holding_bfg10k(self);

		
	/////////reinforcment//////////
	net->reinforcement *= 0.5;//decay

	net->reinforcement += net->Neuron_Array[42][0].activation;//inflicted pain
	net->reinforcement += net->Neuron_Array[43][0].activation;//	''    death

	net->reinforcement -= net->Neuron_Array[10][0].activation;//self_pain

	//if(net->reinforcement)
		//safe_bprintf (PRINT_HIGH,"reinforcement*1000: %i\n", (long)(net->reinforcement*1000));
		//safe_centerprintf (self->client->botinfo.mygame->gameowner, "reinforcement*1000: %i\n", (long)(net->reinforcement*1000));
	///////////////// charge propagation through the network /////////////

	for (layer=0; layer< neuron_layers; layer++)//step through neuron layers. (layer 0 == input layer, layer 'neuron_layers-1' is output layer)
	{

		if(layer == 0) // if this is the first neuron layer
		{
			//// update activatedlastinstant neuron state variable ////
			for (x=0; x< net->num_neurons_in_layer[layer]; x++)// step along row of neurons
				net->Neuron_Array[x][layer].activationlastinstant = net->Neuron_Array[x][layer].activation;				
				//this is the only thing to do for each input layer neuron
		}
		else
		{

			for(x=0; x<net->num_neurons_in_layer[layer]; x++)//step through neurons in this layer (destination neurons)
			{	
				//// update activatedlastinstant neuron state variable ////
				net->Neuron_Array[x][layer].activationlastinstant = net->Neuron_Array[x][layer].activation;	
				
				net->Neuron_Array[x][layer].input = 0;
			
				//get input from previous layer//
				for ( s=0; s<net->num_neurons_in_layer[layer-1]; s++ )// step through source neurons
				{
					
					if (net->Synapse_Array[s][x][layer-1].weight != NOSYNAPSE)	//if a connection exists
					{	
							net->Neuron_Array[x][layer].input 
								+= net->Synapse_Array[s][x][layer-1].weight * net->Neuron_Array[s][layer-1].activation;
											
					}
				}
				
				//STL bit cut out from here
								
				net->Neuron_Array[x][layer].activation = activationFunction(net->Neuron_Array[x][layer].input);
					
				
			}

		} // finish " if ( not first layer )"
			
	}

	if(net->shorttermlearning)
	{
		//Short Term Learning:	run through the net again and run the neuron think functions this time.
		for (layer=0; layer< neuron_layers; layer++)//step through neuron layers. (layer 0 == input layer, layer 'neuron_layers-1' is output layer)
			for(x=0; x<net->num_neurons_in_layer[layer]; x++)//step through neurons in this layer (destination neurons)				
				if(net->Neuron_Array[x][layer].think) //if neuron has a think function
					net->Neuron_Array[x][layer].think(x, layer, net); //then call it
				//else
					//safe_bprintf(PRINT_HIGH, "neuron think == NULL\n");
	}


	//safe_bprintf (PRINT_HIGH,"[bot1 jump output]: %i\n", (long)(net->Neuron_Array[12][neuron_layers-1].activation*1000));

	//safe_bprintf (PRINT_HIGH,"BOT output of fire weapon neuron*1000: %i\n", (long)(net->Neuron_Array[10][neuron_layers-1].activation*1000));

	//////////////// execute relevant output functions /////////////
	tempthresh = 0.51;

	if(! (thegame->stagger_weapon_changes && self->client->botinfo.last_changeweapon_time + 2 > level.time))
	{	//if stagger weapon changes is on, and bot just changed weapon, don't do these output functions.
		
			
		if ( net->Neuron_Array[0][neuron_layers - 1].activation > tempthresh)
			select_blaster(self);
		if ( net->Neuron_Array[1][neuron_layers - 1].activation > tempthresh)
			select_shotgun(self);
		if ( net->Neuron_Array[2][neuron_layers - 1].activation > tempthresh)
			select_sshotgun(self);
		if ( net->Neuron_Array[3][neuron_layers - 1].activation > tempthresh)
			select_mgun(self);
		if ( net->Neuron_Array[4][neuron_layers - 1].activation > tempthresh)
			select_chaingun(self);
		if ( net->Neuron_Array[5][neuron_layers - 1].activation > tempthresh)
			select_grenadel(self);
		if ( net->Neuron_Array[6][neuron_layers - 1].activation > tempthresh)
			select_rocketl(self);
		if ( net->Neuron_Array[7][neuron_layers - 1].activation > tempthresh)
			select_hblaster(self);
		if ( net->Neuron_Array[8][neuron_layers - 1].activation > tempthresh)
			select_railgun(self);
		if ( net->Neuron_Array[9][neuron_layers - 1].activation > tempthresh)
			select_bfg10k(self);
			//note that this order artificially creates a bias against the earlier or perhaps later 
			// weapons being chosen. fix it?
	}

	if ( net->Neuron_Array[10][neuron_layers - 1].activation > tempthresh)
		fire_weapon(self);
	//	if ( net->Neuron_Array[][neuron_layers - 1].activation > tempthresh)throw_grenade(self);
	//if ( net->Neuron_Array[11][neuron_layers - 1].activation > tempthresh)
	//	fire_railgun(self);
	
							// movement:
	if ( net->Neuron_Array[11][neuron_layers - 1].activation > tempthresh)
		bot_duck(self);

	if ( net->Neuron_Array[12][neuron_layers - 1].activation > tempthresh)
		bot_jump(self);



	if ( net->Neuron_Array[13][neuron_layers - 1].activation > tempthresh)
		go_forwards(self);
	if ( net->Neuron_Array[14][neuron_layers - 1].activation > tempthresh)
		go_back(self);

	if ( net->Neuron_Array[15][neuron_layers - 1].activation > tempthresh)
		go_right(self);
	if ( net->Neuron_Array[16][neuron_layers - 1].activation > tempthresh)
		go_left(self);

	if ( net->Neuron_Array[17][neuron_layers - 1].activation > tempthresh)
		bot_run(self);	



	
	//if ( net->Neuron_Array[][neuron_layers - 1].activation > tempthresh)crouch(self);
							// viewpoint:

	//if ( net->Neuron_Array[20][neuron_layers - 1].activation > tempthresh)
	//	look_left(self);
	//if ( net->Neuron_Array[21][neuron_layers - 1].activation > tempthresh)
	//	look_right(self);

	if(self->client->ps.pmove.pm_type != PM_DEAD)//don't do if dying/dead
	{

		if ( net->Neuron_Array[21][neuron_layers - 1].activation > tempthresh)
			look_straight(self);

		lookjust_up(self, net->Neuron_Array[22][neuron_layers - 1].activation);
		lookjust_down(self, net->Neuron_Array[23][neuron_layers - 1].activation);

		look_up(self, net->Neuron_Array[24][neuron_layers - 1].activation);
		look_down(self, net->Neuron_Array[25][neuron_layers - 1].activation);

		looklots_up(self, net->Neuron_Array[26][neuron_layers - 1].activation);
		looklots_down(self, net->Neuron_Array[27][neuron_layers - 1].activation);



		looktiny_left(self, net->Neuron_Array[28][neuron_layers - 1].activation);
		looktiny_right(self, net->Neuron_Array[29][neuron_layers - 1].activation);

		lookjust_left(self, net->Neuron_Array[30][neuron_layers - 1].activation);
		lookjust_right(self, net->Neuron_Array[31][neuron_layers - 1].activation);

		looklots_left(self, net->Neuron_Array[32][neuron_layers - 1].activation);
		looklots_right(self, net->Neuron_Array[33][neuron_layers - 1].activation);

		lookheaps_left(self, net->Neuron_Array[34][neuron_layers - 1].activation);
		lookheaps_right(self, net->Neuron_Array[35][neuron_layers - 1].activation);

	}		




	self->client->botinfo.oldEnemyHeading = enemy_yaw_dif;



} 
















