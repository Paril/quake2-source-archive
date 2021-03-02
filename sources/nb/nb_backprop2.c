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


/*
This shit is all for imitation learning, which i never got to work that well
*/







float stepActFunc(float x)
{
	if(x >= 1)
		return 1;
	else
		return 0;
}
float activationFunction(float x)
{
	return 1/(1 + exp(-1*x));
}
float activationFunctionPrimed(float x)
{
	return activationFunction(x)*(1 - activationFunction(x));
}
/*=========================================================================================
getNeuronsForLayer
------------------
returns the number of neurons in layer 'layer' of net 'net'
hehe
=========================================================================================*/
int getNeuronsForLayer(int layer, NeuralNet* net)
{

	if((layer == (net->SYNAPSE_LAYERS))	//if this is the last neuron layer
		|| (layer == 0))			// or the first layer
		return thegame->NUM_NEURONS_IN_LAYER;
	else
		return net->HIDDEN_NEURONS;
}









float doBackProp1(edict_t* self, trainingpair_t* tpair)
{

	int layer;
	int x;
	int i;
	int		neuron_layers = thegame->SYNAPSE_LAYERS + 1;
	int		num_neurons_in_layer = thegame->NUM_NEURONS_IN_LAYER;
	
	// some shortcuts //
	NeuralNet* net = &self->client->botinfo.itsNet;
	iminfo_t*	iminfo = &self->client->iminfo;

	float weightcounter=0;
	//int test_count=0;

	int n;
	float out=0;

	int d;
	float inveccounter=0;
	float outcounter=0;
	float noutput=0;
	int k;
	float sum_error_squared=0;
	float meh=0;

	int source;

		/// backprop stuff //////
	float	delta [60][5];
	//float	weightcorrection[60][60][5];

	float sum;


	//alter inputs & outputs:
	/*
	for(i=0; i<60; i++)
	{
		if(tpair->inputvector[i] == 0)
			tpair->inputvector[i] = 0.1;
		if(tpair->inputvector[i] == 1)
			tpair->inputvector[i] = 0.9;

		if(tpair->outputvector[i] == 0)
			tpair->outputvector[i] = 0.1;
		if(tpair->outputvector[i] == 1)
			tpair->outputvector[i] = 0.9;
	}*/
	
	
	////input->hidden layer//
	//for(i=0; i<num_neurons_in_layer; i++)
	//	for(k=0; k<net->HIDDEN_NEURONS; k++)
	//		weightcounter += net->Synapse_Array[i][k][0].weight;
	//
	////hidden->output laeyr//
	//for(i=0; i<net->HIDDEN_NEURONS; i++)
	//	for(k=0; k<num_neurons_in_layer; k++)
	//		weightcounter += net->Synapse_Array[i][k][1].weight;
	


				//if(net->Synapse_Array[i][z][x].weight > 1 ||  net->Synapse_Array[i][z][x].weight < -1)
				//	test_count++;


	
	//safe_bprintf (PRINT_HIGH,"weightcounter: %i\n", (long)weightcounter);
	//safe_bprintf (PRINT_HIGH,"weights out of bounds: %i\n", (long)test_count);
	//test_count = 0;

	
	//input vector test//
	//for(i=0; i<60; i++)
		//meh += tpair->inputvector[i];

	//safe_bprintf (PRINT_HIGH,"sum inputvector: %i\n", (long)meh);

	//meh=0;
	//for(i=0; i<60; i++)
		//meh += tpair->outputvector[i];

	//safe_bprintf (PRINT_HIGH,"sum outputvector: %i\n", (long)meh);


	

	

	
	/////////// charge up input layer neurons////////
	//for(i=0; i<num_neurons_in_layer; i++)
		//neuron_output[i][0] = tpair->inputvector[i];

	//for(i=0; i<60; i++)
		//out += tpair->inputvector[i];	///		picking up problem

	//safe_bprintf (PRINT_HIGH,"inputvectorsum: %i\n", (long)out);

	///// clear charge on all neurons //////////
	for(x=0; x<neuron_layers; x++)
		for(i=0; i<num_neurons_in_layer; i++)
			net->Neuron_Array[i][x].input = 0;

	//for(i=0; i<num_neurons_in_layer; i++)
	//	for(x=0; x<num_neurons_in_layer; x++)
	//		for(layer=0; layer<neuron_layers; layer++)
	//			weightcorrection[i][x][layer] = 0;

		


	/////// feed-forward phase ///////////

	for(layer=0; layer<(neuron_layers-1); layer++)//cycle through all layers but output layer
	{
		
		if(layer == 0) //if we are dealing with the input layer of neurons
		{
			for(n=0; n<net->num_neurons_in_layer[layer]; n++)//for all the neurons this layer	
				for(i=0; i<net->num_neurons_in_layer[layer+1]; i++)//for all the neurons in the nextlayer
					net->Neuron_Array[i][1].input += net->Synapse_Array[n][i][0].weight *
															tpair->inputvector[n];	//charge them up

		}
		else	//other layers need the activation function applied to the neuron input first
		{

			for(n=0; n<net->num_neurons_in_layer[layer]; n++)
			{
				net->Neuron_Array[n][layer].activation = activationFunction(net->Neuron_Array[n][layer].input);

				for(i=0; i<net->num_neurons_in_layer[layer+1]; i++)//for all the neurons in the nextlayer
					net->Neuron_Array[i][layer+1].input += net->Synapse_Array[n][i][layer].weight *
															net->Neuron_Array[n][layer].activation;	//charge them up
				
			}
		}
	}
		
	/// calculate activation of output layer of neurons //////////
	for(i=0; i<60; i++)
		net->Neuron_Array[i][neuron_layers-1].activation = activationFunction(net->Neuron_Array[i][neuron_layers-1].input);




	///////////// backpropagation of error ///////////////////////

	layer = neuron_layers - 1;//make layer index output layer of neurons

	//compute deltas for the ouput layer neurons//
	for (x=0; x < net->num_neurons_in_layer[layer]; x++)
		delta[x][layer] = activationFunctionPrimed(net->Neuron_Array[x][layer].input)
							* ( tpair->outputvector[x] - net->Neuron_Array[x][layer].activation );

	//compute delta for the hidden layers//
	for (layer = neuron_layers - 2; layer >= 0; layer--) //working back towards the input layer of neurons
	{
      //nlPrev = nn.getLayer(l+1);
      //nlCurr = nn.getLayer(l);

		for(n=0; n<net->num_neurons_in_layer[layer]; n++)
		{	
			sum = 0;
			
			for(x=0; x<net->num_neurons_in_layer[layer+1]; x++)			//check below (layer) and order of n, x//
				sum += delta[x][layer + 1] * net->Synapse_Array[n][x][layer].weight;

			delta[n][layer] = activationFunctionPrimed(net->Neuron_Array[n][layer].input) * sum;
		}
	}
/*
      for (n = 0; n < nlCurr.getSize(); n++) {
        an = nn.getNeuron(l,n);

        // Find the sum of the previous layer
        float sum = 0.0f;
        for (int j = 0; j < nlPrev.getSize(); j++) {
          anp = nlPrev.getNeuron(j);
          sum += anp.getDelta() * anp.getWeight(n);
          sum += anp.getDelta() * anp.getThreshold();
        }
        //sum += anp.getDelta() * anp.getThreshold();
        an.setDelta(an.getOutputPrime() * sum);
      }
    }
  }*/






	///////////Update weights//////////
	
	//(neuron_layers = 3)

	for(layer = neuron_layers - 2; layer >= 0; layer--)
	{
		if(layer > 0)	//if this is not the input layer
		{
	

			for(n=0; n<net->num_neurons_in_layer[layer]; n++)
				for(x=0; x<net->num_neurons_in_layer[layer+1]; x++)//check this is right
					net->Synapse_Array[n][x][layer].weight += (iminfo->learning_rate * 
																delta[n][layer]	*	//def right
																net->Neuron_Array[x][layer-1].activation);//layer-1 definately right

		}
		else
		{
		

			for(n=0; n<net->num_neurons_in_layer[layer]; n++)
				for(x=0; x<net->num_neurons_in_layer[layer+1]; x++)//check this is right
					net->Synapse_Array[n][x][layer].weight += iminfo->learning_rate * 
																delta[n][layer]	*	//def right
																tpair->inputvector[x];

		}
	}
	
	/*
	for (int l = (nn.numLayers - 1); l >= 0; l--) {
      nl = nn.getLayer(l);
      if (l > 0)
        inputs = nn.getLayer(l-1).getOutputs();
      else
        inputs = nn.getInputs();
      for (int n = 0; n < nl.getSize(); n++) {
        an = nn.getNeuron(l,n);
        an.setThreshold(an.getThreshold() + (eta * an.getDelta() * -1) +
          (alpha * (an.getThreshold() - an.getPreviousThreshold())));
        for (int w = 0; w < an.getDegree(); w++) {
          an.setWeight(w, an.getWeight(w) + (eta * an.getDelta() * inputs[w]) +
          (alpha * (an.getWeight(w) - an.getPreviousWeight(w))));
        }
      }
    }
	*/



	sum_error_squared = 0;
	for(k=0; k<net->num_neurons_in_layer[0]; k++)
		sum_error_squared += ( (tpair->outputvector[k] - net->Neuron_Array[k][neuron_layers-1].activation) * (tpair->outputvector[k] - net->Neuron_Array[k][neuron_layers-1].activation) );

	//if(self->client->iminfo.showsumsquared)
	//	safe_bprintf (PRINT_HIGH,"sum_error_squared * 1000: %i\n", (long)(sum_error_squared*1000));


	net->generation++;
	if((net->generation % 100) == 0)
		//safe_bprintf (PRINT_HIGH,"training pairs used so far: %i\n", net->generation);
		safe_centerprintf(self, "training pairs used so far: %i\n", net->generation);

	return sum_error_squared;

}

	

float doBackProp2(edict_t* self, trainingpair_t* tpair)
{

	int layer;
	int x;
	int i;
	int		num_neurons_in_layer = thegame->NUM_NEURONS_IN_LAYER;
	
	// some shortcuts //
	NeuralNet* net = &self->client->botinfo.itsNet;
	iminfo_t*	iminfo = &self->client->iminfo;

	int		neuron_layers = net->SYNAPSE_LAYERS + 1;//thegame->SYNAPSE_LAYERS + 1;

	float weightcounter=0;
	//int test_count=0;


	int n;
	//int s;
	float out=0;

	//int d;
	float inveccounter=0;
	float outcounter=0;

	int k;
	float sum_error_squared=0;
	float meh=0;

	int source;

		/// backprop stuff //////
	//float	neuron_output[60][5];
	float	errorinfo [60][5];
	float	weightcorrection[60][60][5];

	//float temp;


	//alter inputs & outputs:
	/*
	for(i=0; i<60; i++)
	{
		if(tpair->inputvector[i] == 0)
			tpair->inputvector[i] = 0.1;
		if(tpair->inputvector[i] == 1)
			tpair->inputvector[i] = 0.9;

		if(tpair->outputvector[i] == 0)
			tpair->outputvector[i] = 0.1;
		if(tpair->outputvector[i] == 1)
			tpair->outputvector[i] = 0.9;
	}
	*/
	
//	//input->hidden layer//
//	for(i=0; i<num_neurons_in_layer; i++)
//		for(k=0; k<net->HIDDEN_NEURONS; k++)
//			weightcounter += net->Synapse_Array[i][k][0].weight;
//
//	//hidden->output laeyr//
//	for(i=0; i<net->HIDDEN_NEURONS; i++)
//		for(k=0; k<num_neurons_in_layer; k++)
//			weightcounter += net->Synapse_Array[i][k][1].weight;
//	


				//if(net->Synapse_Array[i][z][x].weight > 1 ||  net->Synapse_Array[i][z][x].weight < -1)
				//	test_count++;


	
	//safe_bprintf (PRINT_HIGH,"weightcounter: %i\n", (long)weightcounter);
	//safe_bprintf (PRINT_HIGH,"weights out of bounds: %i\n", (long)test_count);
	//test_count = 0;

	
	//input vector test//
	for(i=0; i<60; i++)
		meh += tpair->inputvector[i];

	//safe_bprintf (PRINT_HIGH,"sum inputvector: %i\n", (long)meh);

	meh=0;
	for(i=0; i<60; i++)
		meh += tpair->outputvector[i];

	//safe_bprintf (PRINT_HIGH,"sum outputvector: %i\n", (long)meh);


	

	

	
	/////////// charge up input layer neurons////////
	//for(i=0; i<num_neurons_in_layer; i++)
		//neuron_output[i][0] = tpair->inputvector[i];

	//for(i=0; i<60; i++)
		//out += tpair->inputvector[i];	///		picking up problem

	//safe_bprintf (PRINT_HIGH,"inputvectorsum: %i\n", (long)out);
	/*
	out = 0;

	for (layer=0; layer< neuron_layers; layer++)//step through neuron layers. (layer 0 == input layer, layer 'neuron_layers-1' is output layer)
	{

		if(layer == 0) // if this is the first neuron layer
		{
			//// update activatedlastinstant neuron state variable ////
			for (x=0; x< net->num_neurons_in_layer[layer]; x++)// step along row of neurons
			{
				net->Neuron_Array[x][layer].activationlastinstant = net->Neuron_Array[x][layer].activation;				
				
				net->Neuron_Array[x][layer].activation = tpair->inputvector[x];
			}

			temp=0;
			for (x=0; x< net->num_neurons_in_layer[0]; x++)
				temp += net->Neuron_Array[x][layer].activation;

			safe_bprintf(PRINT_HIGH, "sum layer 0 activation: %i ", (int)temp);
		}
		else
		{

			for(x=0; x<net->num_neurons_in_layer[layer]; x++)//step through neurons in this layer (destination neurons)
			{	
				//// update activatedlastinstant neuron state variable ////
				net->Neuron_Array[x][layer].activationlastinstant = net->Neuron_Array[x][layer].activation;	
							
				net->Neuron_Array[x][layer].input = 0;

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
	
	*/
	///// clear charge on all neurons //////////
	for(x=0; x<neuron_layers; x++)
		for(i=0; i<net->num_neurons_in_layer[0]; i++)
			net->Neuron_Array[i][x].input = 0;




	for(layer=0; layer<(neuron_layers-1); layer++)//cycle through all layers but output layer
	{
			
		if(layer == 0) //if we are dealing with the input layer of neurons
		{
			for(n=0; n<net->num_neurons_in_layer[layer]; n++)//for all the neurons this layer	
				for(i=0; i<net->num_neurons_in_layer[layer+1]; i++)//for all the neurons in the nextlayer
					net->Neuron_Array[i][1].input += net->Synapse_Array[n][i][0].weight *
															tpair->inputvector[n];	//charge them up

		}
		else	//other layers need the activation function applied to the neuron charge first
		{

			for(n=0; n<net->num_neurons_in_layer[layer]; n++)
			{
				net->Neuron_Array[n][layer].activation = activationFunction(net->Neuron_Array[n][layer].input);

				
				for(i=0; i<net->num_neurons_in_layer[layer+1]; i++)//for all the neurons in the nextlayer
					net->Neuron_Array[i][layer+1].input += net->Synapse_Array[n][i][layer].weight *
															net->Neuron_Array[n][layer].activation;	//charge them up
				
			}
		}
	}




		
	/// calculate outputs of output layer of neurons //////////
	for(i=0; i<60; i++)
		net->Neuron_Array[i][neuron_layers-1].activation = activationFunction(net->Neuron_Array[i][2].input);

	


	///////////// back propogation of error ///////////////////////

	///// adjustment of the bottom layer of synapses : layer 'neuron_layers - 2' - hidden->output layer///
	for(k=0; k<num_neurons_in_layer; k++)
		errorinfo[k][neuron_layers - 1] = (tpair->outputvector[k] - net->Neuron_Array[k][neuron_layers - 1].activation)
						* activationFunctionPrimed(net->Neuron_Array[k][neuron_layers - 1].input);

	
	for(source=0; source<net->HIDDEN_NEURONS; source++)
		for(k=0; k<num_neurons_in_layer; k++)
			weightcorrection[source][k][neuron_layers - 2] = iminfo->learning_rate * errorinfo[k][neuron_layers-1]
																* net->Neuron_Array[source][neuron_layers-2].activation;//ACTIVATION FUNCTION PRIMED?


	//// adjustment of the next layer up of synapses : layer 0 //// input->hidden layer/////
		//NOTE: make work for any num of layers
	for(i=0; i<net->HIDDEN_NEURONS; i++)
	{
		errorinfo[i][neuron_layers - 2] = 0;

		for(k=0; k<num_neurons_in_layer; k++)
			errorinfo[i][neuron_layers - 2] += errorinfo[k][neuron_layers - 1] 
												* net->Synapse_Array[i][k][neuron_layers - 2].weight;
	}											// is this the right synapse layer?

	for(i=0; i<net->HIDDEN_NEURONS; i++)
		errorinfo[i][neuron_layers - 2] = errorinfo[i][neuron_layers - 2] 
							* activationFunctionPrimed(net->Neuron_Array[i][neuron_layers - 2].input);

	for(i=0; i<net->HIDDEN_NEURONS; i++)
		for(k=0; k<num_neurons_in_layer; k++)
			weightcorrection[k][i][neuron_layers - 3] = iminfo->learning_rate*errorinfo[i][neuron_layers - 2]
																* tpair->inputvector[k];



	///////// update weights: ////////////////////
	// hidden->output layer //
	for(source=0; source<net->HIDDEN_NEURONS; source++)
		for(k=0; k<num_neurons_in_layer; k++)
			net->Synapse_Array[source][k][neuron_layers - 2].weight += weightcorrection[source][k][neuron_layers - 2];

	// input->hidden layer //
	for(source=0; source<net->HIDDEN_NEURONS; source++)
		for(k=0; k<num_neurons_in_layer; k++)
			net->Synapse_Array[source][k][neuron_layers - 3].weight += weightcorrection[source][k][neuron_layers - 3];
	
	
	//// calculate sum_error_squared ////
		
	//safe_bprintf (PRINT_HIGH,"weightc * 1000: %i\n", (long)(weightcorrection[source][k][neuron_layers - 2]*1000));
	//safe_bprintf (PRINT_HIGH,"weightcorrection[source][k][neuron_layers - 3] * 1000: %i\n", (long)(weightcorrection[source][k][neuron_layers - 3]*1000));


	sum_error_squared = 0;
	for(k=0; k<num_neurons_in_layer; k++)
		sum_error_squared += ( (tpair->outputvector[k] - net->Neuron_Array[k][neuron_layers-1].activation) * (tpair->outputvector[k] - net->Neuron_Array[k][neuron_layers-1].activation) );

	//if(self->client->iminfo.showsumsquared)
	//	safe_bprintf (PRINT_HIGH,"sum_error_squared * 1000: %i\n", (long)(sum_error_squared*1000));

	//safe_bprintf (PRINT_HIGH,"output of run forward neuron*1000: %i\n", (long)(net->Neuron_Array[14][neuron_layers-1].activation*1000));
	//safe_bprintf (PRINT_HIGH,"output of fire weapon neuron*1000: %i\n", (long)(net->Neuron_Array[10][neuron_layers-1].activation*1000));

	net->generation++;
	if((net->generation % 100) == 0)
		safe_bprintf (PRINT_HIGH,"training pairs used so far: %i\n", net->generation);

	return sum_error_squared;

}

