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
#include <math.h>





int getNeuronsForLayer(int layer, NeuralNet* net)
{

	if((layer == (thegame->SYNAPSE_LAYERS))	//if this is the last layer
		|| (layer == 0))			// or the first layer
		return thegame->NUM_NEURONS_IN_LAYER;
	else
		return net->HIDDEN_NEURONS;
}

void doBackProp(edict_t* self)
{


	int		x,y,z,s,source,i,k;
	int		layer;
	int		n;
	int		d;


	int		neuron_layers = self->client->botinfo.mygame->SYNAPSE_LAYERS + 1;
	int		num_neurons_in_layer = self->client->botinfo.mygame->NUM_NEURONS_IN_LAYER;

	int		num_neurons_this_layer=0;
	int		num_neurons_last_layer=0;
	int		num_neurons_next_layer=0;

	/// some shortcuts ///
	NeuralNet*	net = &self->client->iminfo.net;

	iminfo_t*	iminfo = &self->client->iminfo;

	trainingpair_t* tpair = &self->client->iminfo.trainingpair;
	
	/// backprop stuff //////
	float	errorinfo [60][5];
	float	weightcorrection[60][60][5];

	float	neuron_output[60][5];//kind of a hack

	float	sum_error_squared;
	float	sum_error;

	int		num_inputs= 0;

	float	noutput;
	float	outputs[60];

	int		test_count = 0;
	int		test_count2 = 0;
	int		test_count3=0;
	int		test_count4=0;
	int		test_count5=0;

	float	weightcounter=0;
	qboolean fuckup = false;

	float	out = 0;
	float	outcounter = 0;
	float	inveccounter=0;








	//safe_bprintf (PRINT_HIGH,"tester: %i\n", (long)iminfo->tester);


/*

	/// first propagate charge through the network ///


	///charge up input neurons////
	for(i=0; i<num_neurons_in_layer; i++)
	{
		
		neuron_output[i][0] = tpair->inputvector[i];
		net->Neuron_Array[i][0].charge = tpair->inputvector[i];
		if(neuron_output[i][0])
			num_inputs++;
	}
	safe_bprintf (PRINT_HIGH,"num input neurons with charge != 0 : %i\n", (long)num_inputs);

	test_count=0;

	///propagate the charge///
	for (z=0; z< neuron_layers; z++)//step through neuron layers
	{
		num_neurons_last_layer = num_neurons_this_layer;//move down a layer


		if((z == (neuron_layers-1))	//if this is the last layer
			|| (z == 0))			// or the first layer
			num_neurons_this_layer = num_neurons_in_layer;
		else
			num_neurons_this_layer = net->HIDDEN_NEURONS;

			safe_bprintf (PRINT_HIGH,"num_neurons_this_layer: %i\n",num_neurons_this_layer);


		for (x=0; x< num_neurons_this_layer; x++)// step along this row of neurons
		{
	
				if(z) // if this is not the first neuron layer
				{
					net->Neuron_Array[x][z].charge = 0;//temp hack - i'm assuming all charge lost per time instant

					test_count++;
		
					/////////// propagate charge along the synapses ////////////////
					// Step through synapses:
					// The destination stays the same, the source neuron is stepped along
					// so the first dimension of the synapse array is stepped along
					// S is the source neuron for the synapse
					for ( s=0; s<num_neurons_last_layer; s++ )// step through source neurons
					{			
						
						net->Neuron_Array[x][z].charge 
							+= (net->Synapse_Array[s][x][z-1].weight * neuron_output[s][z-1]);//CHECK		

						//safe_bprintf (PRINT_HIGH,"neuron_output[s][z-1]*1000: %i\n",(long)(neuron_output[s][z-1]*1000));

						if(net->Synapse_Array[s][x][z-1].weight < -1 || net->Synapse_Array[s][x][z-1].weight > 1)
							safe_bprintf (PRINT_HIGH,"oout of bounds synapse weight!\n");

						if(net->Synapse_Array[s][x][z-1].weight == 0)
							safe_bprintf (PRINT_HIGH,"synapse weight == 0!\n");


					}

					neuron_output[x][z] = net->Neuron_Array[x][z].charge;//activationFunction(net->Neuron_Array[x][z].charge);
					
				
				} // finish " if ( not first layer )"
				

				if(neuron_output[x][z] > 1)
					safe_bprintf (PRINT_HIGH,"neuron_output[x][z] > 1\n");
		}
	}
	safe_bprintf (PRINT_HIGH,"num times in if(z) loop bit: %i\n", (long)test_count);


	num_inputs=0;
	for(i=0; i<num_neurons_in_layer; i++)
	{
		
		//if(neuron_output[i][neuron_layers-1])
		if(tpair->outputvector[i])
			num_inputs++;
	}
	safe_bprintf (PRINT_HIGH,"num_outputs in outputvector: %i\n", (long)num_inputs);

	test_count = 0;
	for(i=0; i<num_neurons_in_layer; i++)
		if(neuron_output[i][neuron_layers-3])
			test_count++;
	safe_bprintf (PRINT_HIGH,"input neurons firing: %i\n", (long)test_count);

	test_count=0;
	for(i=0; i<net->HIDDEN_NEURONS; i++)
		if(neuron_output[i][neuron_layers-2])
			test_count++;
	safe_bprintf (PRINT_HIGH,"hidden neurons firing: %i\n", (long)test_count);

	test_count=0;
	for(i=0; i<num_neurons_in_layer; i++)
		if(neuron_output[i][neuron_layers-1])
			test_count++;

	safe_bprintf (PRINT_HIGH,"output neurons firing: %i\n", (long)test_count);












*/

	///// clear charge on neurons //////////
	for(x=0; x<neuron_layers; x++)
		for(i=0; i<num_neurons_in_layer; i++)
			net->Neuron_Array[i][x].charge = 0;


	for(i=0; i<num_neurons_in_layer; i++)
		for(z=0; z<num_neurons_in_layer; z++)
			for(x=0; x<(neuron_layers-1); x++)
			{
				weightcounter += net->Synapse_Array[i][z][x].weight;

				if(net->Synapse_Array[i][z][x].weight > 1 ||  net->Synapse_Array[i][z][x].weight < -1)
					test_count++;
			}




	safe_bprintf (PRINT_HIGH,"weightcounter: %i\n", (long)weightcounter);


	//safe_bprintf (PRINT_HIGH,"weights out of bounds: %i\n", (long)test_count);
	test_count = 0;


	
	//----------------input vector test-------------------------------

	for(i=0; i<60; i++)
	{
		if(tpair->inputvector[i] == 1)	//not picking up problem
			test_count++;

		if(tpair->inputvector[i] == 0)
			test_count2++;

		if(tpair->inputvector[i] != 0 && tpair->inputvector[i] != 1)
			test_count3++;

		out += tpair->inputvector[i];

	}

	safe_bprintf (PRINT_HIGH,"inputvector bits == 1: %i\n", test_count);
	safe_bprintf (PRINT_HIGH,"inputvector bits == 0: %i\n", test_count2);
	safe_bprintf (PRINT_HIGH,"inputvector bits fucked: %i\n", test_count3);
	safe_bprintf (PRINT_HIGH,"sum inputvector: %i\n", (long)out);


	test_count = 0;
	test_count2 = 0;
	test_count3 = 0;
	out = 0;


	//safe_bprintf (PRINT_HIGH,"num_neurons_in_layer: %i\n", (long)num_neurons_in_layer);



	//for(i=0; i<num_neurons_in_layer; i++)
	//	tpair->inputvector[i]=0;

	//for(i=0; i<3; i++)
	//	tpair->inputvector[i]=1;

	//tpair->inputvector[0] = 10000;
	//--------------------------------------------------------------


	

	

	
	/////////// charge up input layer neurons////////
	//for(i=0; i<num_neurons_in_layer; i++)
		//neuron_output[i][0] = tpair->inputvector[i];

	for(i=0; i<60; i++)
		out += tpair->inputvector[i];	///		picking up problem

	safe_bprintf (PRINT_HIGH,"inputvectorsum: %i\n", (long)out);

	out = 0;




	weightcounter = 0;
	///// first layer ///////
	for(n=0; n<60; n++)
	{
		neuron_output[n][0] = tpair->inputvector[n];

		for(d=0; d<30; d++)
		{
			inveccounter += tpair->inputvector[n];	///invec is the problem!!!!!!!!!!!
			weightcounter += net->Synapse_Array[n][d][0].weight;
			out = tpair->inputvector[n] * net->Synapse_Array[n][d][0].weight;
			net->Neuron_Array[d][1].charge += out;
			outcounter += out;
		}
	}	/////////////problem before here  - either synapses weights ot inputvector is fucked /////
	safe_bprintf (PRINT_HIGH,"outcounter*1000: %i\n", (long)(outcounter*1000));
	safe_bprintf (PRINT_HIGH,"inveccounter*1000: %i\n", (long)(inveccounter*1000));		
	safe_bprintf (PRINT_HIGH,"weightcounter*1000: %i\n", (long)(weightcounter*1000));


	////second layer/////////
	for(n=0; n<30; n++)
	{
		neuron_output[n][1] = noutput = stepActFunc(net->Neuron_Array[n][1].charge);

		if(noutput)
			test_count3++;

		for(d=0; d<60; d++)
		{
			net->Neuron_Array[d][2].charge += noutput * net->Synapse_Array[n][d][1].weight;
		}
	}

/*

	for(layer=0; layer<(neuron_layers-1); layer++)//cycle through all layers but output layer
	{
		
		num_neurons_this_layer = getNeuronsForLayer(layer, net, self->client->botinfo.mygame);
		num_neurons_next_layer = getNeuronsForLayer(layer+1, net, self->client->botinfo.mygame);
	

		for(n=0; n<num_neurons_this_layer; n++)//for all the neurons this layer
		{
			if(layer == 0)
			{
				noutput = tpair->inputvector[n];

				if(noutput)
					test_count++;
			}
			else
			{
				if(net->Neuron_Array[n][layer].charge)
					test_count3++;


				noutput = activationFunction(net->Neuron_Array[n][layer].charge); //work out their output

				if(noutput)
					test_count2++;	

				
			}
			//---------------------------
			if(noutput > 1 || noutput < 0)
					fuckup = true;
			//---------------------------

			for(i=0; i<num_neurons_next_layer; i++)//for all the neurons in the nextlayer
			{
				net->Neuron_Array[i][layer+1].charge += net->Synapse_Array[n][i][layer].weight *
														noutput;	//charge them up
				if(net->Synapse_Array[n][i][layer].weight * noutput)
					test_count4++;
														
			}



		}

	}

*/

	for(i=0; i<60; i++)
		outputs[i] = stepActFunc(net->Neuron_Array[i][2].charge);


//	if(fuckup == true)
//		safe_bprintf (PRINT_HIGH,"noutput out of bounds\n");
		

//	safe_bprintf (PRINT_HIGH,"num times nouput had a value due to inputvector: %i\n", (long)(test_count));
		safe_bprintf (PRINT_HIGH,"num times charge had a value due to charge prop: %i\n", (long)(test_count3));
//	safe_bprintf (PRINT_HIGH,"num times nouput had a value due to charge prop: %i\n", (long)(test_count2));
//	safe_bprintf (PRINT_HIGH,"num times neurons in next layer charged up: %i\n", (long)(test_count4));
//	safe_bprintf (PRINT_HIGH,"in else loop: %i\n", (long)(test_count5));






	//----------------output vector test-------------------------------
	//for(i=0; i<num_neurons_in_layer; i++)
		//tpair->outputvector[i]=0;

	//tpair->outputvector[0] = 2;
	//--------------------------------------------------------------



	


	/*

	///// adjustment of the bottom layer of synapses : layer 'neuron_layers - 2' ///
	for(k=0; k<num_neurons_in_layer; k++)
		errorinfo[k][neuron_layers - 1] = (tpair->outputvector[k] - neuron_output[k][neuron_layers - 1])
						* activationFunctionPrimed(net->Neuron_Array[k][neuron_layers - 1].charge);

	
	for(source=0; source<net->HIDDEN_NEURONS; source++)
		for(k=0; k<num_neurons_in_layer; k++)
			weightcorrection[source][k][neuron_layers - 2] = iminfo->learning_rate * errorinfo[k][neuron_layers-1]
																* neuron_output[source][neuron_layers-2];


	//// adjustment of the next layer up of synapses : layer 0 /////////////
		//NOTE: make work for any num of layers
	for(i=0; i<net->HIDDEN_NEURONS; i++)
		for(k=0; k<num_neurons_in_layer; k++)
			errorinfo[i][neuron_layers - 2] += errorinfo[k][neuron_layers - 1] 
												* net->Synapse_Array[i][k][neuron_layers - 2].weight;
												// is this the right synapse layer?

	for(i=0; i<net->HIDDEN_NEURONS; i++)
		errorinfo[i][neuron_layers - 2] = errorinfo[i][neuron_layers - 2] 
							* activationFunctionPrimed(net->Neuron_Array[i][neuron_layers - 2].charge);

	for(i=0; i<net->HIDDEN_NEURONS; i++)
		for(k=0; k<num_neurons_in_layer; k++)
			weightcorrection[k][i][neuron_layers - 3] = iminfo->learning_rate*errorinfo[i][neuron_layers - 2]
																* tpair->inputvector[k];
			
	///////// update weights: ////////////////////
	for(source=0; source<num_neurons_in_layer; source++)
		for(k=0; k<num_neurons_in_layer; k++)
		{
			net->Synapse_Array[source][k][neuron_layers - 2].weight += weightcorrection[source][k][neuron_layers - 2];

			net->Synapse_Array[source][k][neuron_layers - 3].weight += weightcorrection[source][k][neuron_layers - 3];
		}
	*/
	//// calculate sum_error_squared ////
		
	sum_error_squared = 0;
	for(k=0; k<num_neurons_in_layer; k++)
		sum_error_squared += ( (tpair->outputvector[k] - outputs[k]) * (tpair->outputvector[k] - outputs[k]) );

	safe_bprintf (PRINT_HIGH,"sum_error_squared * 1000: %i\n", (long)(sum_error_squared*1000));

	sum_error_squared = 0;
	for(k=0; k<num_neurons_in_layer; k++)
		sum_error_squared += (tpair->outputvector[k] - outputs[k]);

	safe_bprintf (PRINT_HIGH,"sum_error * 1000: %i\n", (long)(sum_error_squared*1000));

	test_count = 0;
	for(k=0; k<num_neurons_in_layer; k++)
		test_count += outputs[k];

	safe_bprintf (PRINT_HIGH,"output sum: %i\n", (long)(test_count));




}

/*


	///// adjustment of the bottom layer of synapses : layer 'neuron_layers - 2' ///
	for(i=0; i<num_neurons_in_layer; i++)
		errorinfo[i][neuron_layers - 1] = (tpair->outputvector[i] - neuron_output[i][neuron_layers - 1])
						* activationFunctionPrimed(net->Neuron_Array[i][neuron_layers - 1].charge);

	
	for(source=0; source<num_neurons_in_layer; source++)
		for(i=0; i<num_neurons_in_layer; i++)
			weightcorrection[source][i][neuron_layers - 2] = iminfo->learning_rate*errorinfo[i][neuron_layers-1]
																* neuron_output[source][neuron_layers-2];


	//// adjustment of the next layer up of synapses : layer 0 /////////////
		//NOTE: make work for any num of layers
	for(i=0; i<net->HIDDEN_NEURONS; i++)
		for(k=0; k<num_neurons_in_layer; k++)
			errorinfo[i][neuron_layers - 2] += errorinfo[k][neuron_layers - 1] 
												* net->Synapse_Array[i][k][neuron_layers - 2].weight;
												// is this the right synapse layer?

	for(i=0; i<net->HIDDEN_NEURONS; i++)
		errorinfo[i][neuron_layers - 2] = errorinfo[i][neuron_layers - 2] 
							* activationFunctionPrimed(net->Neuron_Array[i][neuron_layers - 2].charge);

	for(i=0; i<net->HIDDEN_NEURONS; i++)
		for(k=0; k<num_neurons_in_layer; k++)
			weightcorrection[k][i][neuron_layers - 3] = iminfo->learning_rate*errorinfo[i][neuron_layers - 2]
																* tpair->inputvector[k];
	/*		
	///////// update weights: ////////////////////
	for(source=0; source<num_neurons_in_layer; source++)
		for(k=0; k<num_neurons_in_layer; k++)
		{
			net->Synapse_Array[source][k][neuron_layers - 2].weight += weightcorrection[source][k][neuron_layers - 2];

			net->Synapse_Array[source][k][neuron_layers - 3].weight += weightcorrection[source][k][neuron_layers - 3];
		}

  */
/*					most recent:

*/