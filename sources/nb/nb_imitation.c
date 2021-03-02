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


void getImitationStuffFromIni(edict_t* self);
/*===========================================================================

  Imitation learning
  ------------------

grr i dislike backprop : )

===========================================================================*/

/*================================================================================
resetImitationOutputFlags
-------------------------
sets all the iminfo flags to false
================================================================================*/
void resetImitationOutputFlags(tpair_fsm_t* fsm)
{

		//safe_bprintf (PRINT_HIGH,"resetting imitationOutput flags\n");

		fsm->select_blaster_f = false;
	
		fsm->select_shotgun_f = false;
		fsm->select_sshotgun_f = false;
		fsm->select_mgun_f = false;
		fsm->select_chaingun_f = false;
		fsm->select_grenadel_f = false;
		fsm->select_rocketl_f = false;
		fsm->select_hblaster_f = false;
		fsm->select_railgun_f = false;
		fsm->select_bfg10k_f = false;


		fsm->fire_weapon_f = false;
		//fsm->fire_railgun_f = false;

							// movement:
		fsm->bot_jump_f = false;
		fsm->bot_duck_f = false;

		fsm->walk_forwards_f = false;
		fsm->run_forwards_f = false;

		fsm->walk_back_f = false;
		fsm->run_back_f = false;

		fsm->walk_left_f = false;	
		fsm->run_left_f = false;

		fsm->walk_right_f = false;
		fsm->run_right_f = false;



		fsm->look_straight_f = false;



		//------------------------

		// old_v_angle[3];
}

/*==============================================================================================
fillOutputs
-----------

fills up the output vector array, by checking if the flags in self->client->iminfo are set or not.
These flags are set in various places through the code, when the player does various things.
Do a search for IMITATION NEWCODE to find them.





NOTE: add in ducking flag etc..

==============================================================================================*/
void fillOutputs(edict_t* self, tpair_fsm_t* fsm)
{

	float*	outputs = fsm->trainingpair.outputvector;	//NOTE: check
	int i;

	for(i=0; i< thegame->NUM_NEURONS_IN_LAYER; i++)
		outputs[i] = 0.0;	//clear memory first
	
	if(fsm->select_blaster_f)
		outputs[0] = 1.0;
	if(fsm->select_shotgun_f)
		outputs[1] = 1.0;
	if(fsm->select_sshotgun_f)
		outputs[2] = 1.0;
	if(fsm->select_mgun_f)
		outputs[3] = 1.0;
	if(fsm->select_chaingun_f)
		outputs[4] = 1.0;
	if(fsm->select_grenadel_f)
		outputs[5] = 1.0;
	if(fsm->select_rocketl_f)
		outputs[6] = 1.0;
	if(fsm->select_hblaster_f)
		outputs[7] = 1.0;
	if(fsm->select_railgun_f)
		outputs[8] = 1.0;
	if(fsm->select_bfg10k_f)
		outputs[9] = 1.0;
		
	
	if(fsm->fire_weapon_f)
		outputs[10] = 1.0;


	if(fsm->bot_duck_f)
		outputs[11] = 1.0;
	if(fsm->bot_jump_f)
		outputs[12] = 1.0;

	if(fsm->walk_forwards_f)
		outputs[13] = 1.0;
	if(fsm->run_forwards_f)
		outputs[14] = 1.0;

	if(fsm->walk_back_f)
		outputs[15] = 1.0;
	if(fsm->run_back_f)
		outputs[16] = 1.0;

	if(fsm->walk_left_f)
		outputs[17] = 1.0;	
	if(fsm->run_left_f)
		outputs[18] = 1.0;

	if(fsm->walk_right_f)
		outputs[19] = 1.0;
	if(fsm->run_right_f)
		outputs[20] = 1.0;
	
	
	//if(fsm->look_straight_f)
	//	outputs[21] = 1.0;


	outputs[22] = fsm->lookjust_up_float;	
	outputs[23] = fsm->lookjust_down_float;

	outputs[24] = fsm->look_up_float;
	outputs[25] = fsm->look_down_float;

	outputs[26] = fsm->looklots_up_float;
	outputs[27] = fsm->looklots_down_float;



	outputs[28] = fsm->looktiny_left_float;
	outputs[29] = fsm->looktiny_right_float;

	outputs[30] = fsm->lookjust_left_float;
	outputs[31] = fsm->lookjust_right_float;

	outputs[32] = fsm->looklots_left_float;
	outputs[33] = fsm->looklots_right_float;

	outputs[34] = fsm->lookheaps_left_float;
	outputs[35] = fsm->lookheaps_right_float;


	//NOTE: update
	//////////// outputtest messages //////////
	if(self->client->iminfo.outputtest)
	{
		if(fsm->select_blaster_f)
			safe_bprintf (PRINT_HIGH,"select_blaster_f\n");
		if(fsm->select_shotgun_f)
			safe_bprintf (PRINT_HIGH,"select_shotgun_f\n");
		if(fsm->select_sshotgun_f)
			safe_bprintf (PRINT_HIGH,"select_sshotgun_f\n");
		if(fsm->select_mgun_f)
			safe_bprintf (PRINT_HIGH,"select_mgun_f\n");
		if(fsm->select_chaingun_f)
			safe_bprintf (PRINT_HIGH,"select_chaingun_f\n");
		if(fsm->select_grenadel_f)
			safe_bprintf (PRINT_HIGH,"select_grenadel_f\n");
		if(fsm->select_rocketl_f)
			safe_bprintf (PRINT_HIGH,"select_rocketl_f\n");
		if(fsm->select_hblaster_f)
			safe_bprintf (PRINT_HIGH,"select_hblaster_f\n");
		if(fsm->select_railgun_f)
			safe_bprintf (PRINT_HIGH,"select_railgun_f\n");
		if(fsm->select_bfg10k_f)
			safe_bprintf (PRINT_HIGH,"select_bfg10k_f\n");
		

		if(fsm->fire_weapon_f)
			safe_bprintf (PRINT_HIGH,"fire_weapon_f\n");
		//if(fsm->fire_railgun_f)
		//	safe_bprintf (PRINT_HIGH,"fire_railgun_f\n");
		if(fsm->bot_jump_f)
			safe_bprintf (PRINT_HIGH,"bot_jump_f\n");
		if(fsm->bot_duck_f)
			safe_bprintf (PRINT_HIGH,"bot_duck_f\n");
		if(fsm->walk_forwards_f)
			safe_bprintf (PRINT_HIGH,"walk_forwards_f\n");
		if(fsm->run_forwards_f)
			safe_bprintf (PRINT_HIGH,"run_forwards_f\n");
		if(fsm->walk_back_f)
			safe_bprintf (PRINT_HIGH,"walk_back_f\n");
		if(fsm->run_back_f)
			safe_bprintf (PRINT_HIGH,"run_back_f\n");

		if(fsm->walk_left_f)
			safe_bprintf (PRINT_HIGH,"walk_left_f\n");
		if(fsm->run_left_f)
			safe_bprintf (PRINT_HIGH,"run_left_f\n");

		if(fsm->walk_right_f)
			safe_bprintf (PRINT_HIGH,"walk_right_f\n");
		if(fsm->run_right_f)
			safe_bprintf (PRINT_HIGH,"run_right_f\n");

		/*
		if(fsm->look_left_f)
			safe_bprintf (PRINT_HIGH,"look_left_f\n");
		if(fsm->look_right_f)
			safe_bprintf (PRINT_HIGH,"look_right_f\n");
		if(fsm->look_up_f)
			safe_bprintf (PRINT_HIGH,"look_up_f\n");
		if(fsm->look_down_f)
			safe_bprintf (PRINT_HIGH,"look_down_f\n");
		//if(fsm->looklots_left_f)
		//	safe_bprintf (PRINT_HIGH,"looklots_left_f\n");
		//if(fsm->looklots_right_f)
		//	safe_bprintf (PRINT_HIGH,"looklots_right_f\n");
		if(fsm->looklots_up_f)
			safe_bprintf (PRINT_HIGH,"looklots_up_f\n");
		if(fsm->looklots_down_f)
			safe_bprintf (PRINT_HIGH,"looklots_down_f\n");
		if(fsm->look_straight_f)
			safe_bprintf (PRINT_HIGH,"look_straight_f\n");
		*/
		//if(fsm->lookjust_left_f)
		//	safe_bprintf (PRINT_HIGH,"lookjust_left_f\n");
		//if(fsm->lookjust_right_f)
		//	safe_bprintf (PRINT_HIGH,"lookjust_right_f\n");
		if(fsm->lookheaps_left_float)
			safe_bprintf (PRINT_HIGH,"lookheaps_left_float: %i\n", (long)(fsm->lookheaps_left_float*1000));
		if(fsm->lookheaps_right_float)
			safe_bprintf (PRINT_HIGH,"lookheaps_right_float: %i\n", (long)(fsm->lookheaps_right_float*1000));
		if(fsm->looktiny_left_float)
			safe_bprintf (PRINT_HIGH,"looktiny_left_float: %i\n", (long)(fsm->looktiny_left_float*1000));
		if(fsm->looktiny_right_float)
			safe_bprintf (PRINT_HIGH,"looktiny_right_float: %i\n", (long)(fsm->looktiny_right_float*1000));

		safe_bprintf (PRINT_HIGH,"													\n");
	}
}
/*===============================================================================================
fillInputs
----------
paramemters:
float* : the inputvector array to fill
edict_t* : ent to run the inout functions on

this function runs all the bot input functions on the entity specifies by the
second parameter - a human player.  The results are assigned to the inputvector array going by
the order specified in nb_nnet.c.
================================================================================================*/


void fillInputs(float* inputs, edict_t* self)
{
	int i;
	///stuff for input functions ////
	float enemy_yaw_dif;  
	float item_yaw_dif; 
	float enemy_pitch_dif;
	

	enemy_yaw_dif = calcEnemyYawDif(self);
	item_yaw_dif = calcItemYawDif(self);
	enemy_pitch_dif = calcEnemyPitchDif(self);

	/// first clear input vector //////
	for(i=0; i<thegame->NUM_NEURONS_IN_LAYER; i++)//self->client->botinfo.mygame->NUM_NEURONS_IN_LAYER; i++)//NOTE: CHECK THIS WORKS
		inputs[i]=0.0;

	 
	//safe_bprintf (PRINT_HIGH,"enemy_pitch_dif: %i\n", (long)enemy_pitch_dif);
	

	//NOTE: these inputs will be out of date by now.  They need to be the same as in nb.ini//
	inputs[0] = check_item_justatleft(self, item_yaw_dif);	
	inputs[1] = check_item_justatright(self, item_yaw_dif);		
	inputs[2] = check_item_atleft(self, item_yaw_dif);		
	inputs[3] = check_item_atright(self, item_yaw_dif);		

	inputs[45] = check_item_ahead(self, item_yaw_dif);		
		// NOTE: 45 used!
	//inputs[4] = upper_bell_random(self);					

	inputs[5] = check_justat_left(self, enemy_yaw_dif);		
	inputs[6] = check_justat_right(self, enemy_yaw_dif);	

	inputs[7] = allwaysfire(self);							

	inputs[8] = check_f_blaster(self);						
	inputs[9] = traceahead_enemy(self);						

	inputs[10] = check_self_pain(self);						
	inputs[11] = check_just_fired(self);					
									// some traclines inputss:
	//inputs[12] = traceahead_lava(self);
	inputs[12] = traceright_wall(self);	
	inputs[18] = traceleft_wall(self);	
	
	inputs[13] = traceahead_wall(self);						
	inputs[14] = traceahead_entity(self);					
	//inputs[15] = traceahead_water(self);
	
	inputs[15] = tracehalfleft_wall(self);
	inputs[36] = tracehalfright_wall(self);

			// trace ahead and down - eqivalent looking at  bottom of view screen
	inputs[16] = tracediag_lava(self);						
	inputs[17] = tracediag_wall(self);																
	//inputs[18] = tracediag_entity(self);  probably useless right now
	inputs[19] = tracediag_water(self);						

								// target actions:	
	inputs[20] = enemy_jump(self); 
	inputs[21] = enemy_duck(self);							
	inputs[22] = check_far_left(self, enemy_yaw_dif);		
  //is oppenent moving to the left of nbot's view space?
	inputs[23] = check_far_right(self, enemy_yaw_dif);
	
	inputs[24] = enemy_up(self, enemy_pitch_dif);
	inputs[25] = enemy_down(self, enemy_pitch_dif);


	//inputs[24] = check_move_up(self);
	//inputs[25] = check_move_down(self);//vertical dimension inputs deactivated for now
	inputs[26] = check_at_left(self, enemy_yaw_dif); 		
// of view space
	inputs[27] = check_at_right(self, enemy_yaw_dif);		

	//inputs[28] = check_at_bottom(self);
	//inputs[29] = check_at_top(self);

	inputs[29] = check_ahead_enemy(self, enemy_yaw_dif);	

	inputs[30] = boxtraceleft_wall(self);
	inputs[31] = boxtraceright_wall(self);
	inputs[32] = boxtracehalfleft_wall(self);
	inputs[33] = boxtracehalfright_wall(self);

	//inputs[30] = check_f_shotgun(self);							 // opponent fires shotgun. 
	//inputs[31] = check_f_sshotgun(self);					 
	//inputs[32] = check_f_mgun(self);						
	//inputs[33] = check_f_chaingun(self);					
	inputs[34] = check_f_grenadel(self);					
	inputs[35] = check_f_rocketl(self);						
	//inputs[36] = check_f_hyperb(self);						
	//inputs[37] = check_f_railgun(self);						
	//inputs[38] = check_f_bfg10k(self);						
	//inputs[39] = check_f_grenade(self);// hand grenade

	inputs[37] = enemy_moving_left(self, enemy_yaw_dif);
	inputs[38] = enemy_moving_right(self, enemy_yaw_dif);

	inputs[39] = check_item_close(self);
		// standing on:
	//inputs[] = check_platform(self);
	//inputs[] = check_normal(self);
	//inputs[] = check_lava(self);
	//inputs[] = check_nothing(self);
	//inputs[] = check_ladder(self);
		

	inputs[40] = boxtraceahead_close(self);					
	inputs[41] = boxtraceahead_far(self);					
					
	//inputs[40] = small_random(self); getting rid of randoms right now
	//inputs[41] = medium_random(self);
	//inputs[42] = large_random(self);
	//inputs[3] = bell_random(self);
	inputs[42] = check_inflicted_pain(self);				
	inputs[43] = check_inflicted_death(self);				

	inputs[44] = check_enemy_close(self);					

		/////// standing in water: ////////
	//inputs[45] = check_feet(self);
	//inputs[46] = check_waist(self);
	//inputs[47] = check_head(self);

	//inputs[46] = was_blocked(self);						 if(inputs[46]){safe_bprintf (PRINT_HIGH,"was_blocked\n");}
		//don't know how do to this right now - might be useless anyway
									// hearing sense:
	//inputs[] = check_noise_footsteps(self);
	//inputs[] = check_noise_firing(self);
	

	inputs[47] = health_low(self);							
	inputs[48] = health_medium(self);						
	
	inputs[49] = holding_grenade(self);						
	inputs[50] = holding_blaster(self);						
	inputs[51] = holding_shotgun(self);						
	inputs[52] = holding_sshotgun(self);					
	inputs[53] = holding_mgun(self);						
	inputs[54] = holding_chaingun(self);					
	inputs[55] = holding_grenadel(self);					
	inputs[56] = holding_rocketl(self);						
	inputs[57] = holding_hyperb(self);						
	inputs[58] = holding_railgun(self);						
	inputs[59] = holding_bfg10k(self);
	
	
	


	if(self->client->iminfo.inputtest)
	{	/*
		if(inputs[0]){safe_bprintf (PRINT_HIGH,"check_item_justatleft\n");}
		if(inputs[1]){safe_bprintf (PRINT_HIGH,"check_item_justatright\n");}	
		if(inputs[2]){safe_bprintf (PRINT_HIGH,"check_item_atleft\n");}
		if(inputs[3]){safe_bprintf (PRINT_HIGH,"check_item_atright\n");}

		if(inputs[45]){safe_bprintf (PRINT_HIGH,"check_item_ahead\n");}
			// NOTE: 45 used!

		if(inputs[5]){safe_bprintf (PRINT_HIGH,"check_justat_left\n");}
		if(inputs[6]){safe_bprintf (PRINT_HIGH,"check_justat_right\n");}

		//if(inputs[7]){safe_bprintf (PRINT_HIGH,"allwaysfire\n");}

		if(inputs[8]){safe_bprintf (PRINT_HIGH,"check_f_blaster\n");}
		if(inputs[9]){safe_bprintf (PRINT_HIGH,"traceahead_enemy\n");}

		if(inputs[10]){safe_bprintf (PRINT_HIGH,"check_self_pain\n");}
		if(inputs[11]){safe_bprintf (PRINT_HIGH,"check_just_fired\n");}
										// some traclines inputss:
		//if(inputs[12]){safe_bprintf (PRINT_HIGH,"traceahead_lava\n");}
		if(inputs[12]){safe_bprintf (PRINT_HIGH,"traceright_wall\n");}
		if(inputs[18]){safe_bprintf (PRINT_HIGH,"traceleft_wall\n");}
	
		if(inputs[13]){safe_bprintf (PRINT_HIGH,"traceahead_wall: %i\n", (long)(inputs[13]*1000));}
		
		if(inputs[14]){safe_bprintf (PRINT_HIGH,"traceahead_entity\n");}
		//if(inputs[15]){safe_bprintf (PRINT_HIGH,"traceahead_water\n");}

		if(inputs[15]){safe_bprintf (PRINT_HIGH,"tracehalfleft_wall\n");}
		if(inputs[36]){safe_bprintf (PRINT_HIGH,"tracehalfright_wall\n");}


				// trace ahead and down - eqivalent looking at  bottom of view screen
		if(inputs[16]){safe_bprintf (PRINT_HIGH,"tracediag_lava\n");}
		if(inputs[17]){safe_bprintf (PRINT_HIGH,"tracediag_wall\n");}										
		//inputs[18] = tracediag_entity(self);  probably useless right now
		if(inputs[19]){safe_bprintf (PRINT_HIGH,"tracediag_water\n");}
		
									// target actions:	inputs[20] = enemy_jump(self); 
		if(inputs[29]){safe_bprintf (PRINT_HIGH,"enemy_duck\n");}
		if(inputs[22]){safe_bprintf (PRINT_HIGH,"check_far_left\n");}
	  //is oppenent moving to the left of nbot's view space?
		if(inputs[23]){safe_bprintf (PRINT_HIGH,"check_far_right\n");}
		*/
		if(inputs[24]){safe_bprintf (PRINT_HIGH,"enemy_up: %i\n", (int)(inputs[24]*1000) );}
		if(inputs[25]){safe_bprintf (PRINT_HIGH,"enemy_down: %i\n", (int)(inputs[25]*1000));}
		/*
		if(inputs[26]){safe_bprintf (PRINT_HIGH,"check_at_left\n");}
	// of view space
		if(inputs[27]){safe_bprintf (PRINT_HIGH,"check_at_right\n");}

		//inputs[28] = check_at_bottom(self);
		//inputs[29] = check_at_top(self);

		if(inputs[29]){safe_bprintf (PRINT_HIGH,"check_ahead_enemy\n");}
		
		if(inputs[30]){safe_bprintf (PRINT_HIGH,"boxtraceleft_wall\n");} 
		if(inputs[31]){safe_bprintf (PRINT_HIGH,"boxtraceright_wall\n");} 
		if(inputs[32]){safe_bprintf (PRINT_HIGH,"boxtracehalfleft_wall\n");}
		if(inputs[33]){safe_bprintf (PRINT_HIGH,"boxtracehalfright_wall\n");}

		//if(inputs[30]){safe_bprintf (PRINT_HIGH,"check_f_shotgun\n");} // opponent fires shotgun. 
		//if(inputs[31]){safe_bprintf (PRINT_HIGH,"check_f_sshotgun\n");} 
		//if(inputs[32]){safe_bprintf (PRINT_HIGH,"check_f_mgun\n");}
		//if(inputs[33]){safe_bprintf (PRINT_HIGH,"check_f_chaingun\n");}
		if(inputs[34]){safe_bprintf (PRINT_HIGH,"check_f_grenadel\n");}
		if(inputs[35]){safe_bprintf (PRINT_HIGH,"check_f_rocketl\n");}
		//if(inputs[36]){safe_bprintf (PRINT_HIGH,"check_f_hyperb\n");}
		*/
		if(inputs[37]){safe_bprintf (PRINT_HIGH,"enemy_moving_left: %i\n", (long)(inputs[37]*1000));}
		if(inputs[38]){safe_bprintf (PRINT_HIGH,"enemy_moving_right: %i\n", (long)(inputs[38]*1000));}
		/*
		//if(inputs[39]){safe_bprintf (PRINT_HIGH,"check_f_grenade\n");} // hand grenade
		
		if(inputs[39]){safe_bprintf (PRINT_HIGH,"check_item_close %i\n", (long)(inputs[39]*1000));} // hand grenade
								// standing on:
		//inputs[] = check_platform(self);
		//inputs[] = check_normal(self);
		//inputs[] = check_lava(self);
		//inputs[] = check_nothing(self);
		//inputs[] = check_ladder(self);
			

		if(inputs[40]){safe_bprintf (PRINT_HIGH,"boxtraceahead_close\n");}
		if(inputs[41]){safe_bprintf (PRINT_HIGH,"boxtraceahead_far\n");}
					
		//inputs[40] = small_random(self); getting rid of randoms right now
		//inputs[41] = medium_random(self);
		//inputs[42] = large_random(self);
		//inputs[3] = bell_random(self);
		if(inputs[42]){safe_bprintf (PRINT_HIGH,"check_inflicted_pain\n");}
		if(inputs[43]){safe_bprintf (PRINT_HIGH,"check_inflicted_death\n");}

		//if(inputs[44]){safe_bprintf (PRINT_HIGH,"check_enemy_close\n");}

			/////// standing in water: ////////
		//inputs[45] = check_feet(self);
		//inputs[46] = check_waist(self);
		//inputs[47] = check_head(self);

		//inputs[46] = was_blocked(self);						 if(inputs[46]){safe_bprintf (PRINT_HIGH,"was_blocked\n");}
			//don't know how do to this right now - might be useless anyway
										// hearing sense:
		//inputs[] = check_noise_footsteps(self);
		//inputs[] = check_noise_firing(self);
		
		
		if(inputs[47]){safe_bprintf (PRINT_HIGH,"health_low\n");}
		if(inputs[48]){safe_bprintf (PRINT_HIGH,"health_medium\n");}

		if(inputs[49]){safe_bprintf (PRINT_HIGH,"holding_grenaden");}
		if(inputs[50]){safe_bprintf (PRINT_HIGH,"holding_blaster\n");}
		if(inputs[51]){safe_bprintf (PRINT_HIGH,"holding_shotgun\n");}
		else if(inputs[52]){safe_bprintf (PRINT_HIGH,"holding_sshotgun\n");}
		else if(inputs[53]){safe_bprintf (PRINT_HIGH,"holding_mgun\n");}
		else if(inputs[54]){safe_bprintf (PRINT_HIGH,"holding_chaingun\n");}
		else if(inputs[55]){safe_bprintf (PRINT_HIGH,"holding_grenadel\n");}
		else if(inputs[56]){safe_bprintf (PRINT_HIGH,"holding_rocketl\n");}
		else if(inputs[57]){safe_bprintf (PRINT_HIGH,"holding_hyperb\n");}
		else if(inputs[58]){safe_bprintf (PRINT_HIGH,"holding_railgun\n");}
		else if(inputs[59]){safe_bprintf (PRINT_HIGH,"holding_bfg10k\n");}
		*/
		safe_bprintf (PRINT_HIGH,"													\n");
	}

}

/*=====================================================================================
initializeOldYaw
----------------
=====================================================================================*/
void initializeOldYaw(edict_t* self)
{
	self->client->iminfo.old_v_angle[YAW] = self->client->v_angle[YAW];
}




/*====================================================================================
maintainCurrentDYaw
-------------------
====================================================================================*/
void maintainCurrentDYaw(edict_t* self)
{
	//if(level.time > self->client->iminfo.last_dyaw_time + 0.1)	//if server frame has advanced
	//{
		//self->client->iminfo.last_dyaw_time = level.time;

		self->client->iminfo.current_dyaw = self->client->v_angle[YAW] - self->client->iminfo.old_v_angle[YAW];

		self->client->iminfo.old_v_angle[YAW] = self->client->v_angle[YAW];//update old_v_angle

		if(self->client->iminfo.current_dyaw > 180)
			self->client->iminfo.current_dyaw -= 360;

		if(self->client->iminfo.current_dyaw < -180)
			self->client->iminfo.current_dyaw += 360;
	//}
//	safe_bprintf (PRINT_HIGH,"current_dyaw: %i\n",(long)(self->client->iminfo.current_dyaw));
	//safe_bprintf (PRINT_HIGH,"current yaw: %i\n",(long)(self->client->v_angle[YAW]));
}









/*====================================================================================
maintainCurrentDPitch
---------------------
====================================================================================*/
void maintainCurrentDPitch(edict_t* self)
{
	//if(level.time > self->client->iminfo.last_dyaw_time + 0.1)	//if server frame has advanced
	//{
		//self->client->iminfo.last_dyaw_time = level.time;

		self->client->iminfo.current_dpitch = self->client->v_angle[PITCH] - self->client->iminfo.old_v_angle[PITCH];

		self->client->iminfo.old_v_angle[PITCH] = self->client->v_angle[PITCH];//update old_v_angle

	//}

	//safe_bprintf (PRINT_HIGH,"current_dpitch: %i\n",(long)(self->client->iminfo.current_dpitch));

	//safe_bprintf (PRINT_HIGH,"current pitch: %i\n",(long)(self->client->v_angle[PITCH]));
}









/*=====================================================================================
setFlagsForCurrentDYaw
-------------------
	#define TURN_TINY		0.5
	#define TURN_JUST		5
	#define TURN_MEDIUM		10
	#define TURN_LOTS		20
	#define TURN_HEAPS		60	// this contols the bots' max turning rate NOTE: was 85
=====================================================================================*/
void setFlagsForCurrentDYaw(edict_t* self)
{
	float dyaw = self->client->iminfo.current_dyaw;
	float activation;
	int i;

	////set all activatios to 0////
	for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
	{
		self->client->iminfo.tpair_fsm[i].looktiny_left_float = 0;			
		self->client->iminfo.tpair_fsm[i].looktiny_right_float = 0;
		self->client->iminfo.tpair_fsm[i].lookjust_left_float = 0;			
		self->client->iminfo.tpair_fsm[i].lookjust_right_float = 0;
		self->client->iminfo.tpair_fsm[i].looklots_left_float = 0;			
		self->client->iminfo.tpair_fsm[i].looklots_right_float = 0;
		self->client->iminfo.tpair_fsm[i].lookheaps_left_float = 0;	
		self->client->iminfo.tpair_fsm[i].lookheaps_right_float = 0;
	}


	if(dyaw == 0)
	{	
		//do nothing
	}	
	else if(dyaw >= 0 && dyaw <= TURN_TINY)
	{
		safe_bprintf (PRINT_HIGH,"looktiny_left\n");

		activation = dyaw / TURN_TINY;

		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].looktiny_left_float = activation;	//set flag
			
	}
	else if(dyaw >= -1*TURN_TINY && dyaw < 0)
	{
		safe_bprintf (PRINT_HIGH,"looktiny_right\n");

		activation = dyaw / TURN_TINY;

		activation *= -1;//get a positive activation

		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].looktiny_right_float = activation;
	
	}
	
	else if(dyaw > TURN_TINY && dyaw <= TURN_JUST)
	{
		safe_bprintf (PRINT_HIGH,"lookjust_left\n");

		activation = dyaw / TURN_JUST;


		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].lookjust_left_float = activation;


	}
	else if(dyaw < -1*TURN_TINY && dyaw >= -1*TURN_JUST)
	{
		safe_bprintf (PRINT_HIGH,"lookjust_right\n");

		activation = dyaw / TURN_JUST;

		activation *= -1;

		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].lookjust_right_float = activation;

	}
	//ignore turn medium; not used right now
	else if(dyaw > TURN_JUST && dyaw <= TURN_LOTS)
	{
		safe_bprintf (PRINT_HIGH,"looklots_left\n");

		activation = dyaw / TURN_LOTS;

		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].looklots_left_float = activation;


	}
	else if(dyaw < -1*TURN_JUST && dyaw >= -1*TURN_LOTS)
	{
		safe_bprintf (PRINT_HIGH,"looklots_right\n");

		activation = dyaw / TURN_LOTS;

		activation *= -1;

		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].looklots_right_float = activation;

	}
	else if(dyaw > TURN_LOTS)
	{
		safe_bprintf (PRINT_HIGH,"lookheaps_left\n");

		activation = dyaw / TURN_HEAPS;

		if(activation > 1)
			activation = 1;

		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].lookheaps_left_float = activation;


	}
	else if(dyaw < -1*TURN_LOTS)
	{
		safe_bprintf (PRINT_HIGH,"lookheaps_right\n");

		activation = dyaw / TURN_HEAPS;

		activation *= -1;

		if(activation > 1)
			activation = 1;

		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].lookheaps_right_float = activation;

	}
	
}


/*=====================================================================================
setFlagsForCurrentDPitch
-------------------
	#define LOOKUP_JUST		4
	#define LOOKUP			15
	#define LOOKUP_LOTS		50
=====================================================================================*/
void setFlagsForCurrentDPitch(edict_t* self)
{
	float dpitch = self->client->iminfo.current_dpitch;
	float activation;
	int i;

	////set all activations to 0////
	for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
	{
		self->client->iminfo.tpair_fsm[i].lookjust_up_float = 0;			
		self->client->iminfo.tpair_fsm[i].lookjust_down_float = 0;

		self->client->iminfo.tpair_fsm[i].look_up_float = 0;			
		self->client->iminfo.tpair_fsm[i].look_down_float = 0;

		self->client->iminfo.tpair_fsm[i].looklots_up_float = 0;			
		self->client->iminfo.tpair_fsm[i].looklots_down_float = 0;

	}


	if(dpitch == 0)
	{	
		//do nothing
	}	
	else if(dpitch > 0 && dpitch <= LOOKUP_JUST)
	{
		//safe_bprintf (PRINT_HIGH,"lookjust_down\n");

		activation = dpitch / LOOKUP_JUST;

		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].lookjust_down_float = activation;	//set flag
			
	}
	else if(dpitch >= -1*LOOKUP_JUST && dpitch < 0)
	{
		//safe_bprintf (PRINT_HIGH,"lookjust_up\n");

		activation = dpitch / LOOKUP_JUST;

		activation *= -1;//get a positive activation

		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].lookjust_up_float = activation;
	
	}	
	else if(dpitch >= LOOKUP_JUST && dpitch <= LOOKUP)
	{
		//safe_bprintf (PRINT_HIGH,"look_down\n");

		activation = dpitch / LOOKUP;

		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].look_down_float = activation;	//set flag
			
	}
	else if(dpitch >= -1*LOOKUP && dpitch < -1*LOOKUP_JUST)
	{
		//safe_bprintf (PRINT_HIGH,"look_up\n");

		activation = dpitch / LOOKUP;

		activation *= -1;//get a positive activation

		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].look_up_float = activation;
	
	}
	else if(dpitch > LOOKUP)
	{
		//safe_bprintf (PRINT_HIGH,"looklots_down\n");

		activation = dpitch / LOOKUP_LOTS;

		if(activation > 1)
			activation = 1;

		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].looklots_down_float = activation;


	}
	else if(dpitch < -1*LOOKUP)
	{
		//safe_bprintf (PRINT_HIGH,"looklots_up\n");

		activation = dpitch / LOOKUP_LOTS;

		activation *= -1;

		if(activation > 1)
			activation = 1;

		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].looklots_up_float = activation;

	}
	
}
	/*
	typedef struct usercmd_s
	{
		byte	msec;
		byte	buttons;
		short	angles[3];
		short	forwardmove, sidemove, upmove;
		byte	impulse;		// remove?
		byte	lightlevel;		// light level the player is standing on
	} usercmd_t;
	*/
	/*
	qboolean	walk_forwards_f;
	qboolean	run_forwards_f;

	qboolean	walk_back_f;
	qboolean	run_back_f;

	qboolean	walk_left_f;	
	qboolean	run_left_f;

	qboolean	walk_right_f;
	qboolean	run_right_f;
	*/
/*===========================================================================================
getMoveOutput
-------------

samples the keys being held down and sets the relevant iminfo(imitation info) flags

Does jumping as well as horizontal plane movement

This function is called midway down the ClientThink() function in p_client.c, when the 
movement code is being done.
============================================================================================*/
void getMoveOutput(usercmd_t* cmd, edict_t* self)
{
	int i;

	if(cmd->forwardmove == 400)//running
	{
		//safe_bprintf (PRINT_HIGH,"run_forwards\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].run_forwards_f = true;
	}
	else if(cmd->forwardmove == 200)
	{
		//safe_bprintf (PRINT_HIGH,"walk_forwards\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].walk_forwards_f = true;
	}
	else if(cmd->forwardmove == -200)
	{
		//safe_bprintf (PRINT_HIGH,"walk_back\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].walk_back_f = true;
	}
	else if(cmd->forwardmove == -400)
	{
		//safe_bprintf (PRINT_HIGH,"run_back\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].run_back_f = true;
	}



	if(cmd->sidemove == 400)//run-strafe right
	{
		//safe_bprintf (PRINT_HIGH,"run_right\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].run_right_f = true;
	}
	else if(cmd->sidemove == 200)
	{
		//safe_bprintf (PRINT_HIGH,"walk_right\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].walk_right_f = true;
	}
	else if(cmd->sidemove == -200)
	{
		//safe_bprintf (PRINT_HIGH,"walk_left\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].walk_left_f = true;
	}
	else if(cmd->sidemove == -400)
	{
		//safe_bprintf (PRINT_HIGH,"run_left\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].run_left_f = true;
	}

//	if(cmd->sidemove)
//		safe_bprintf (PRINT_HIGH,"sidemove\n");
	if(cmd->upmove > 10)
	{
		//safe_bprintf (PRINT_HIGH,"bot_jump\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].bot_jump_f = true;
	}
	if(cmd->upmove < -10)
	{
		//safe_bprintf (PRINT_HIGH,"bot_duck\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].bot_duck_f = true;
	}
}






/*================================================================================
getChangeWeaponOutput
---------------------

this function is called by changeweapon() in p_weapon.c if a human player changes their
weapon.  It sets a iminfo flag depending on what the weapon being changed to is
================================================================================*/
void getChangeWeaponOutput(edict_t* self, gitem_t* newitem)
{
	int i;
	if(newitem == NULL)
		return;

	if(newitem->weapmodel == WEAP_BLASTER)
	{
		//safe_bprintf (PRINT_HIGH,"select_blaster\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].select_blaster_f = true;
	}
	else if(newitem->weapmodel == WEAP_SHOTGUN)
	{
		//safe_bprintf (PRINT_HIGH,"select_shotgun\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].select_shotgun_f = true;
	}
	else if(newitem->weapmodel == WEAP_SUPERSHOTGUN)
	{
		//safe_bprintf (PRINT_HIGH,"select_sshotgun\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].select_sshotgun_f = true;
	}
	else if(newitem->weapmodel == WEAP_MACHINEGUN)
	{
		//safe_bprintf (PRINT_HIGH,"select_mgun\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].select_mgun_f = true;
	}
	else if(newitem->weapmodel == WEAP_CHAINGUN)
	{
		//safe_bprintf (PRINT_HIGH,"select_chaingun\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].select_chaingun_f = true;
	}
	//if(newitem->weapmodel == WEAP_GRENADES)
	//{
	//	safe_bprintf (PRINT_HIGH,"select_grenades\n");
	//	self->client->iminfo.select_grenades_f = true;
	//}
	else if(newitem->weapmodel == WEAP_GRENADELAUNCHER)
	{
		//safe_bprintf (PRINT_HIGH,"select_grenadel\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].select_grenadel_f = true;
	}
	else if(newitem->weapmodel == WEAP_ROCKETLAUNCHER)
	{
		//safe_bprintf (PRINT_HIGH,"select_rocketl\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].select_rocketl_f = true;
	}
	else if(newitem->weapmodel == WEAP_HYPERBLASTER)
	{
		//safe_bprintf (PRINT_HIGH,"select_hblaster\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].select_hblaster_f = true;
	}
	else if(newitem->weapmodel == WEAP_RAILGUN)
	{
		//safe_bprintf (PRINT_HIGH,"select_railgun\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].select_railgun_f = true;
	}
	else if(newitem->weapmodel == WEAP_BFG)
	{
		//safe_bprintf (PRINT_HIGH,"select_bfg10k\n");
		for(i=0; i<self->client->iminfo.NUM_FSMS; i++)
			self->client->iminfo.tpair_fsm[i].select_bfg10k_f = true;
	}

}



/*
#define WEAP_BLASTER			1 
#define WEAP_SHOTGUN			2 
#define WEAP_SUPERSHOTGUN		3 
#define WEAP_MACHINEGUN			4 
#define WEAP_CHAINGUN			5 
#define WEAP_GRENADES			6 
#define WEAP_GRENADELAUNCHER	7 
#define WEAP_ROCKETLAUNCHER		8 
#define WEAP_HYPERBLASTER		9 
#define WEAP_RAILGUN			10
#define WEAP_BFG				11
*/



void initializeHumanNN(edict_t* self)
{


	int s, d, l;// s = source neuron, d = dest neuron, l = layer
	long c = 0;
								// Step through:
		for ( l=0; l< thegame->SYNAPSE_LAYERS; l++ ) // synapse layers
			for ( s=0; s< thegame->NUM_NEURONS_IN_LAYER; s++ ) // source neurons
				for ( d=0; d< thegame->NUM_NEURONS_IN_LAYER; d++ ) // destination neurons
				{
				/////// randomize weight /////////////////////
				 self->client->botinfo.itsNet.Synapse_Array[s][d][l].weight = -1 + (random()*2);
				

				 c++;
				}
	
		//NOTE: fix this so that the values = bots loads up NOTE: is this actually needed with num_neurons_in_layer code?
		self->client->botinfo.itsNet.MAX_WEIGHT	= 1;
		self->client->botinfo.itsNet.MIN_WEIGHT	= 1;
		self->client->botinfo.itsNet.MAX_CHARGE	= 3; // irrelevant right now
		self->client->botinfo.itsNet.THRESHOLD	= 1;
		self->client->botinfo.itsNet.DISCHARGE_AMOUNT=1;
		self->client->botinfo.itsNet.HIDDEN_NEURONS = 30;

		self->client->botinfo.itsNet.generation = 0;//this will be number of training sets not GA evolves


	safe_bprintf (PRINT_HIGH,"human weights initialized\n");



}

void Cmd_Imitation_f(edict_t* self)
{
	if(self->client->iminfo.imitation_on == false)
	{
		if(self->health > 0)
		{
			self->client->iminfo.imitation_on = true;

			inititalizeImitation(self);

			safe_bprintf (PRINT_HIGH,"imitation learning ON\n");
		}
		else
			safe_bprintf (PRINT_HIGH,"wait till you are alive\n");//ugly...

	}
	else
	{
		self->client->iminfo.imitation_on = false;

		shutdownImitation(self);

		safe_bprintf (PRINT_HIGH,"imitation learning OFF\n");
	}
}
/*==================================================================================
inititalizeImitation
--------------------

called when a player enters the imitation command in the console.
loads up certian parameters.
==================================================================================*/ 

void inititalizeImitation(edict_t* self)
{

	int i;
	NeuralNet*	botnet = &thegame->Neuralbot_Array[0]->client->botinfo.itsNet;
		// a net from an arbitrary bot.

	
	initializeHumanNN(self);//randomize weights and setup some parameters

	Cmd_CreateArchive_f(self);
			
	self->client->iminfo.learning_rate = 0.1;
	self->client->iminfo.upload_period = 30;
	self->client->iminfo.upload_period_frames = 0;
	self->client->iminfo.num_bp_runs_per_frame = 1;
	self->client->iminfo.inputtest = false;
	self->client->iminfo.outputtest = false;
	self->client->iminfo.showsumsquared = false;
	self->client->iminfo.sampling = true;	//sampling of players inputs/outputs ON

	for(i=0; i<10; i++)//NOTE: this must be set to max num of tpair fsms.
	{
		self->client->iminfo.tpair_fsm[i].imitation_state = IM_WAITING;//for staggering
			
		self->client->iminfo.tpair_fsm[i].delay_frames = 10;
		self->client->iminfo.tpair_fsm[i].sample_frames = 1;

		self->client->iminfo.tpair_fsm[i].waiting_frames = i*2;//stagger phase of fsms
		self->client->iminfo.tpair_fsm[i].waiting_framecount = 0;//reset frame counter
	}

	getImitationStuffFromIni(self);	//load in imitation parameters from nb.ini

	///copy nnet values over from bot 0's net///
	self->client->botinfo.itsNet.MAX_WEIGHT = botnet->MAX_WEIGHT;
	self->client->botinfo.itsNet.MIN_WEIGHT = botnet->MIN_WEIGHT;

		//thegame->Neuralbot_Array[i].itsNet.MAX_CHARGE	= 3; // irrelevant right now
	self->client->botinfo.itsNet.THRESHOLD = botnet->THRESHOLD;
		//thegame->Neuralbot_Array[i].itsNet.DISCHARGE_AMOUNT=1;

	self->client->botinfo.itsNet.HIDDEN_NEURONS = botnet->HIDDEN_NEURONS;

	self->client->botinfo.itsNet.SYNAPSE_LAYERS = botnet->SYNAPSE_LAYERS;//NOTE: hacked in 

	for(i=0; i<thegame->SYNAPSE_LAYERS + 1; i++)
	{
		self->client->botinfo.itsNet.num_neurons_in_layer[i] = 
				getNeuronsForLayer(i, &self->client->botinfo.itsNet);

		safe_bprintf (PRINT_HIGH,"setting num_neurons_in_layer[%i] to %i\n", i, self->client->botinfo.itsNet.num_neurons_in_layer[i]);
	}


	downloadBotNNWeightsToHuman(self);//NOTE: do this more than once?

}


void getImitationStuffFromIni(edict_t* self)
{
	int i;
	char *p; 
	int z;

	i = Ini_ReadIniFile("nb.ini", &ini_file);
	
	if(i != 1) // if file could not be read
	{
		safe_bprintf (PRINT_HIGH,"ini file stuffed up\n");
		return;
	}
	
		// first we make sure that the .ini file is open
    if (ini_file.ini_file_read)      
	{
		// now we use ent's classname to try and
		// find the appropriate entry in the health.ini
		// file - now you know why I used those particular
		// names in the health.ini file.

		safe_bprintf (PRINT_HIGH,"==reading in from nb.in...==\n");

		//////////// get num_fsms /////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "num_fsms");
		if (p != NULL)
		{
			self->client->iminfo.NUM_FSMS = atoi(p);
			safe_bprintf (PRINT_HIGH,"	setting num_fsms from nb.ini to %i\n", atoi(p));
		}
		//////////// get learning_rate /////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "learning_rate_x1000");
		if (p != NULL)
		{
			self->client->iminfo.learning_rate = atoi(p)*0.001;
			safe_bprintf (PRINT_HIGH,"	setting learning_rate_x1000 from nb.ini to %i\n", atoi(p));
		}
		////////////// delay_frames ///////////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "delay_frames");
		if (p != NULL)
		{
			for(z=0; z<self->client->iminfo.NUM_FSMS; z++)
				self->client->iminfo.tpair_fsm[i].delay_frames = atoi(p);

			safe_bprintf (PRINT_HIGH,"	setting delay_frames to %i\n", atoi(p));
		}
		///////////// sample_frames //////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "sample_frames");
		if (p != NULL)
		{
			for(z=0; z<self->client->iminfo.NUM_FSMS; z++)
				self->client->iminfo.tpair_fsm[i].sample_frames = atoi(p);

			safe_bprintf (PRINT_HIGH,"	setting sample_frames to %i\n", atoi(p));
		}
		///////////// num_bp_runs_per_frame //////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "num_bp_runs_per_frame");
		if (p != NULL)
		{

			self->client->iminfo.num_bp_runs_per_frame = atoi(p);

			safe_bprintf (PRINT_HIGH,"	setting num_bp_runs_per_frame to %i\n", atoi(p));
		}
		///////////// upload_period //////////////
		p = Ini_GetValue(&ini_file, "Game Parameters", "upload_period");
		if (p != NULL)
		{

			self->client->iminfo.upload_period = atoi(p);

			safe_bprintf (PRINT_HIGH,"	setting upload_period to %i\n", atoi(p));
		}


	}
}


void shutdownImitation(edict_t* self)
{
	Cmd_ClearArchive_f(self);
}


void uploadHumanNNWeightsToBots(edict_t* self)
{

	int b, s, d, l;// s = source neuron, d = dest neuron, l = layer
	long c=0;

	for(b=0; b<thegame->NUM_BOTS; b++) //step through bots
									// Step through:
		for ( l=0; l< thegame->SYNAPSE_LAYERS; l++ ) // synapse layers
			for ( s=0; s< thegame->NUM_NEURONS_IN_LAYER; s++ ) // source neurons
				for ( d=0; d< thegame->NUM_NEURONS_IN_LAYER; d++ ) // destination neurons
				{
					/////// upload weight /////////////////////
					thegame->Neuralbot_Array[b]->client->botinfo.itsNet.Synapse_Array[s][d][l].weight 
					 = self->client->botinfo.itsNet.Synapse_Array[s][d][l].weight;

					c++;
				}
	


	safe_bprintf (PRINT_HIGH,"weights uploaded to bots.\n");
}


void downloadBotNNWeightsToHuman(edict_t* self)
{


	int s, d, l;// s = source neuron, d = dest neuron, l = layer

	for ( l=0; l< thegame->SYNAPSE_LAYERS; l++ ) // synapse layers
			for ( s=0; s< thegame->NUM_NEURONS_IN_LAYER; s++ ) // source neurons
				for ( d=0; d< thegame->NUM_NEURONS_IN_LAYER; d++ ) // destination neurons
				{
					/////// download weight /////////////////////
					if(thegame->Neuralbot_Array[0]->client->botinfo.itsNet.Synapse_Array[s][d][l].weight == NOSYNAPSE)	//don't copy weight if synapse doesn't exist
						self->client->botinfo.itsNet.Synapse_Array[s][d][l].weight = 0;	//NOTE: is this the best? 
					else
						self->client->botinfo.itsNet.Synapse_Array[s][d][l].weight
							= thegame->Neuralbot_Array[0]->client->botinfo.itsNet.Synapse_Array[s][d][l].weight;		  
				}

	safe_bprintf (PRINT_HIGH,"weights downloaded from bots.\n");
}

void Cmd_Inputtest_f(edict_t* self)
{
	if(self->client->iminfo.inputtest == false)
	{
		self->client->iminfo.inputtest = true;
		safe_bprintf (PRINT_HIGH,"inputtest ON\n");
	}
	else
	{
		self->client->iminfo.inputtest = false;
		safe_bprintf (PRINT_HIGH,"inputtest OFF\n");
	}
}
void Cmd_Outputtest_f(edict_t* self)
{
	if(self->client->iminfo.outputtest == false)
	{
		self->client->iminfo.outputtest = true;
		safe_bprintf (PRINT_HIGH,"outputtest ON\n");
	}
	else
	{
		self->client->iminfo.outputtest = false;
		safe_bprintf (PRINT_HIGH,"outputtest OFF\n");
	}
}
void Cmd_Showsumsquared_f(edict_t* self)
{
	if(self->client->iminfo.showsumsquared == false)
	{
		self->client->iminfo.showsumsquared = true;
		safe_bprintf (PRINT_HIGH,"showsumsquared ON\n");
	}
	else
	{
		self->client->iminfo.showsumsquared = false;
		safe_bprintf (PRINT_HIGH,"showsumsquared OFF\n");
	}
}

void Cmd_Sampling_f(edict_t* self)
{
	if(self->client->iminfo.sampling == false)
	{
		self->client->iminfo.sampling = true;
		safe_bprintf (PRINT_HIGH,"sampling inputs/outputs ON\n");
	}
	else
	{
		self->client->iminfo.sampling = false;
		safe_bprintf (PRINT_HIGH,"sampling inputs/outputs OFF\n");
	}
}

qboolean imitationOn(edict_t* self)
{
	if(!self->client)
		return false;

	if(self->client->iminfo.imitation_on == true)
		return true;
	else
		return false;
}
void Cmd_Uploadweights_f(edict_t* self)
{
	if(self->client->iminfo.imitation_on == true)
		uploadHumanNNWeightsToBots(self);
	else
		safe_bprintf (PRINT_HIGH,"turn on imitation first\n");
}

void Cmd_BPPerFrame_f(edict_t* self)
{

	self->client->iminfo.num_bp_runs_per_frame = atoi(gi.argv(1));//set to the second word typed into the console
	
	if(self->client->iminfo.num_bp_runs_per_frame < 0)
		self->client->iminfo.num_bp_runs_per_frame = 0;

	safe_bprintf (PRINT_HIGH,"num_bp_runs_per_frame set to: %i \n", 	self->client->iminfo.num_bp_runs_per_frame);
}
void Cmd_Randomizebpnet_f(edict_t* self)
{
	RandomizeBPnet(self);
}



void RandomizeBPnet(edict_t* self)
{


	int s, d, l;// s = source neuron, d = dest neuron, l = layer

	for ( l=0; l< thegame->SYNAPSE_LAYERS; l++ ) // synapse layers
			for ( s=0; s< thegame->NUM_NEURONS_IN_LAYER; s++ ) // source neurons
				for ( d=0; d< thegame->NUM_NEURONS_IN_LAYER; d++ ) // destination neurons
				{
					self->client->botinfo.itsNet.Synapse_Array[s][d][l].weight = -1 + (random()*2);	//NOTE: is this the best? 
				  
				}

	safe_bprintf (PRINT_HIGH,"BP net weights randomized.\n");
}












//////// tpairarchive_t functions //////////////
// index is the index into the array where the next pair can be inserted
/*==================================================================================
initializeTPArchive
-------------------
Should be called when the arhive is created in memory
==================================================================================*/
void initializeTPArchive(tpairarchive_t* tpa)
{
	tpa->index = 0;
	tpa->rewriteindex = 0;
	tpa->MAX_TPAIRS = 10000;//this needs to be equal or less to the size of the array as specified in nb_nnet.h

	safe_bprintf (PRINT_HIGH,"archive initialized\n");
}

/*==================================================================================
getRandomTPair
--------------
returns a pointer to a random training pair from the archive.
==================================================================================*/
trainingpair_t* getRandomTPair(tpairarchive_t* tpa)
{
	long tpairindex;

	if(tpa_isEmpty(tpa))
		return NULL;

	tpairindex = floor(random() * tpa->index);	//get the index of a random pair

	if(tpairindex == tpa->index)	//don't go past index-1
	{
		safe_bprintf (PRINT_HIGH,"tpairindex == tpa->index\n");
		tpairindex--;
	}

	//safe_bprintf (PRINT_HIGH,"returning pair number %i\n", tpairindex);
	
	return &tpa->tpairarchive[tpairindex];
}


/*==================================================================================
overwriteTPair
--------------
Overwrites the training pair allready in the archive at position 'rewriteindex'.
Then advances rewriteindex, and loops it back to 0 if it has reached the end of the 
archive array.
==================================================================================*/
void overwriteTPair(tpairarchive_t* tpa, trainingpair_t* tpair)
{

	if(tpa_isEmpty(tpa))	//if the arhchive is empty
		addTPair(tpa, tpair);	//just add it
	else
	{
		safe_bprintf (PRINT_HIGH,"overwriting pair index num %i\n", tpa->rewriteindex);

		tpa->tpairarchive[tpa->rewriteindex] = *tpair;//overwrite it with a new pair

		tpa->rewriteindex = (tpa->rewriteindex + 1) % tpa->MAX_TPAIRS;		
	}
}


/*===================================================================================
pushTPair
---------
If the archive is full, a pair is overwritten, otherwise the archive is just added to.
returns 1 if the pair was used to overwrite another pair, 
or 2 if the pair was just added to the archive.
===================================================================================*/
int	pushTPair(tpairarchive_t* tpa, trainingpair_t* tpair)
{
	if(tpa_isFull(tpa))
	{
		overwriteTPair(tpa, tpair);
		return 1;
	}
	else
	{
		addTPair(tpa, tpair);
		return 2;
	}
}





/*===================================================================================
addTPair
--------
Adds a training pair to the archive.  returns false if there is no room.
===================================================================================*/
qboolean addTPair(tpairarchive_t* tpa, trainingpair_t* newtpair)
{
	if(tpa_isFull(tpa))
		return false;

	safe_bprintf (PRINT_HIGH,"adding pair to archive: %i\n", tpa_getIndex(tpa)  );

	tpa->tpairarchive[tpa->index] = *newtpair;
	tpa->index++;

	return true;
}

/*===================================================================================
deleteAllTPairs
---------------
gets rid of all the training pairs in the archive by setting the index to 0
===================================================================================*/
void deleteAllTPairs(tpairarchive_t* tpa)
{
	tpa->index = 0;//just reset index for now
	tpa->rewriteindex = 0;
	safe_bprintf (PRINT_HIGH,"all pairs deleted\n");
}


/*===================================================================================
tpa_isFull
----------
returns true if the archive is full
===================================================================================*/
qboolean tpa_isFull(tpairarchive_t* tpa)
{
	if(tpa->index >= tpa->MAX_TPAIRS)
		return true;
	else
		return false;
}




qboolean tpa_isEmpty(tpairarchive_t* tpa)
{
	if(tpa->index == 0)
		return true;
	else
		return false;
}

long tpa_getIndex(tpairarchive_t* tpa)
{
	return tpa->index;
}


void CreateArchive(edict_t* self);



void Cmd_CreateArchive_f(edict_t* self)
{
	if(self->client->iminfo.tpairarchive_p == NULL)
		CreateArchive(self);
	else
		safe_bprintf (PRINT_HIGH,"archive allready exists\n");	
}


void CreateArchive(edict_t* self)
{

	tpairarchive_t* tpa_pointer;
	tpa_pointer = gi.TagMalloc(sizeof(tpairarchive_t), TAG_GAME);
	
	self->client->iminfo.tpairarchive_p = tpa_pointer;


	initializeTPArchive(tpa_pointer);
}


void Cmd_ClearArchive_f(edict_t* self)
{
	if(self->client->iminfo.tpairarchive_p == NULL)
		safe_bprintf (PRINT_HIGH,"no archive to clear\n");
	else
		deleteAllTPairs(self->client->iminfo.tpairarchive_p);
}










void doTPairTest(tpair_fsm_t* fsm)
{
	int r = rand() % 60;
	int i;

	for(i=0; i<r; i++)
		fsm->trainingpair.inputvector[i] = 0;

	for(i=r; i<60; i++)
		fsm->trainingpair.inputvector[i] = 1;
	
	for(i=0; i<r; i++)
		fsm->trainingpair.outputvector[i] = 0;

	for(i=r; i<60; i++)
		fsm->trainingpair.outputvector[i] = 1;

}