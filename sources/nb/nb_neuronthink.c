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

//This stuff isn't used right now


/*=============================================================================
NeuronMutate
------------
assigns the neuron a random think function out of all the possible different learning
rule think functions.
=============================================================================*/

void NeuronMutate(Neuron* neuron)
{
	int NUM_RULES = 15;	//num learning rules that the neuron can use
	int r;

	r = rand() % NUM_RULES;

	switch(r)
	{
	case 0: neuron->think = NT_HOMOSYNAPTIC_1;
			neuron->thinkfuncnum = 0;
		break;
	case 1: neuron->think = NT_HOMOSYNAPTIC_2;
			neuron->thinkfuncnum = 1;
		break;
	case 2: neuron->think = NT_HOMOSYNAPTIC_3;
			neuron->thinkfuncnum = 2;
		break;
	case 3: neuron->think = NT_HOMOSYNAPTIC_4;
			neuron->thinkfuncnum = 3;
		break;
	case 4: neuron->think = NT_HOMOSYNAPTIC_5;
			neuron->thinkfuncnum = 4;
		break;
	case 5: neuron->think = NT_HETEROSYNAPTIC_1;
			neuron->thinkfuncnum = 5;
		break;
	case 6: neuron->think = NT_HETEROSYNAPTIC_2;
			neuron->thinkfuncnum = 6;
		break;
	case 7: neuron->think = NT_ASSOCIATIVE_1;
			neuron->thinkfuncnum = 7;
		break;
	case 8: neuron->think = NT_ASSOCIATIVE_2;
			neuron->thinkfuncnum = 8;
		break;
	case 9: neuron->think = NT_ASSOCIATIVE_3;
			neuron->thinkfuncnum = 9;
		break;
	case 10: neuron->think = NT_ASSOCIATIVE_4;
			 neuron->thinkfuncnum = 10;
		break;
	case 11: neuron->think = NT_ASSOCIATIVE_5;
			 neuron->thinkfuncnum = 11;
		break;
	case 12: neuron->think = NT_COVARIANCE;
			 neuron->thinkfuncnum = 12;
		break;
	case 13: neuron->think = NT_MODULATORY;
			 neuron->thinkfuncnum = 13;
		break;
	case 14: neuron->think = NULL;
			 neuron->thinkfuncnum = 14;
		break;
	}
}

void printThinkFuncForNeuron(Neuron* n)
{
	char *name;

	switch(n->thinkfuncnum)
	{
	case 0: name = "HOMOSYNAPTIC_1";
			
		break;
	case 1: name = "HOMOSYNAPTIC_2";
		break;
	case 2: name = "HOMOSYNAPTIC_3";
		break;
	case 3: name = "HOMOSYNAPTIC_4";
		break;
	case 4: name = "HOMOSYNAPTIC_5";
		break;
	case 5: name = "HETEROSYNAPTIC_1";
		break;
	case 6: name = "HETEROSYNAPTIC_2";
		break;
	case 7: name = "ASSOCIATIVE_1";
		break;
	case 8: name = "ASSOCIATIVE_2";
		break;
	case 9: name = "ASSOCIATIVE_3";
		break;
	case 10: name = "ASSOCIATIVE_4";
		break;
	case 11: name = "ASSOCIATIVE_5";
		break;
	case 12: name = "COVARIANCE";
		break;
	case 13: name = "MODULATORY";
		break;
	case 14: name = "NULL";
		break;
	default: name = "unknown number";
	}

	safe_bprintf(PRINT_HIGH, "%s\n",name);
}



// dW b,a (t) = E * Ya(t)
void NT_HOMOSYNAPTIC_1 (int x, int layer, NeuralNet* net)
{
	int b;

	if(layer == net->SYNAPSE_LAYERS)	//if this is the last neuron layer
		return;		//don't want to mess with layer+1

	for(b=0; b<net->num_neurons_in_layer[layer+1]; b++)
		net->Synapse_Array[x][b][layer].weight += 0.1 * net->Neuron_Array[x][layer].activation;

}


// dW b,a (t) = E * Ya(t) * (Wmax - W b,a (t))             [asymptotic approach to max synapse weight]
void NT_HOMOSYNAPTIC_2 (int x, int layer, NeuralNet* net)
{
	int b;
	
	if(layer == net->SYNAPSE_LAYERS)	//if this is the last neuron layer
		return;		//don't want to mess with layer+1

	for(b=0; b<net->num_neurons_in_layer[layer+1]; b++)//NOTE: changed to [layer]
		net->Synapse_Array[x][b][layer].weight += 0.1 * net->Neuron_Array[x][layer].activation
												* (net->MAX_WEIGHT - fabs(net->Synapse_Array[x][b][layer].weight) );	
}

// dW b,a = E * (Ya(t) ^ -1)
void NT_HOMOSYNAPTIC_3 (int x, int layer, NeuralNet* net)
{
	int b;
	
	if(layer == net->SYNAPSE_LAYERS)	//if this is the last neuron layer
		return;		//don't want to mess with layer+1

	//NOTE: OPTIMISE! (take out 1/act)
	for(b=0; b<net->num_neurons_in_layer[layer+1]; b++)//NOTE: changed to [layer]
		net->Synapse_Array[x][b][layer].weight += 0.1 * (1 / net->Neuron_Array[x][layer].activation);
}	

// dW b,a = E * (Ya(t) ^ -1) * (Wmin - W b,a(t))	[asymptotic approach to min weight]
void NT_HOMOSYNAPTIC_4 (int x, int layer, NeuralNet* net)
{
	int b;	

	if(layer == net->SYNAPSE_LAYERS)	//if this is the last neuron layer
		return;		//don't want to mess with layer+1

	//NOTE: OPTIMISE
	for(b=0; b<net->num_neurons_in_layer[layer+1]; b++)
		net->Synapse_Array[x][b][layer].weight += 0.1 * (1 / net->Neuron_Array[x][layer].activation)
														* (net->MIN_WEIGHT - fabs(net->Synapse_Array[x][b][layer].weight));//abs?
}

// dW b,a(t) = F(Y b(t))
void NT_HOMOSYNAPTIC_5 (int x, int layer, NeuralNet* net)
{
	int b;

	if(layer == net->SYNAPSE_LAYERS)	//if this is the last neuron layer
		return;		//don't want to mess with layer+1

	for(b=0; b<net->num_neurons_in_layer[layer + 1]; b++)
		net->Synapse_Array[x][b][layer].weight += 0.01 * net->Neuron_Array[b][layer+1].activation;
}


//dW b,a(t) = F(Yc(t))	//no comment
void NT_HETEROSYNAPTIC_1(int x, int layer, NeuralNet* net)
{}

//dW b,a(t) = F(Ym(t))	modulatory neuron activation can be reinforcement:)
void NT_HETEROSYNAPTIC_2(int x, int layer, NeuralNet* net)
{
	int b;

	if(layer == net->SYNAPSE_LAYERS)	//if this is the last neuron layer
		return;		//don't want to mess with layer+1

	for(b=0; b<net->num_neurons_in_layer[layer + 1]; b++)
		net->Synapse_Array[x][b][layer].weight += net->reinforcement;
}

// dW b,a(t) = F(Ya(t), Yb(t))
void NT_ASSOCIATIVE_1(int x, int layer, NeuralNet* net)
{
	int b;

	if(layer == net->SYNAPSE_LAYERS)	//if this is the last neuron layer
		return;		//don't want to mess with layer+1

	for(b=0; b<net->num_neurons_in_layer[layer + 1]; b++)
		net->Synapse_Array[x][b][layer].weight += (0.01 * net->Neuron_Array[b][layer+1].activation
														* net->Neuron_Array[x][layer].activation) - 0.1;
}

// dW b,a(t) = E * Ya(t) * Yb(t)
void NT_ASSOCIATIVE_2(int x, int layer, NeuralNet* net)
{
	int b;

	if(layer == net->SYNAPSE_LAYERS)	//if this is the last neuron layer
		return;		//don't want to mess with layer+1

	for(b=0; b<net->num_neurons_in_layer[layer + 1]; b++)
		net->Synapse_Array[x][b][layer].weight += 0.01 * net->Neuron_Array[b][layer+1].activation
														* net->Neuron_Array[x][layer].activation;
}

// dW b,a (t) = ( E * Ya(t) * Yb(t) ) - (B * Yb(t)) - (G * Ya(t)) - C
void NT_ASSOCIATIVE_3(int x, int layer, NeuralNet* net)
{
	int b;

	if(layer == net->SYNAPSE_LAYERS)	//if this is the last neuron layer
		return;		//don't want to mess with layer+1

	//NOTE: OPTIMISE!
	for(b=0; b<net->num_neurons_in_layer[layer + 1]; b++)
		net->Synapse_Array[x][b][layer].weight += 0.01 * net->Neuron_Array[b][layer+1].activation
														* net->Neuron_Array[x][layer].activation
														- (0.005 * net->Neuron_Array[b][layer+1].activation)
														- (0.005 * net->Neuron_Array[x][layer].activation)
														- 0.005;
}


// dW b,a(t) = W b,a(t) + [E*G(Yb(t)) * (C*Ya(t) - W b,a(t))]
void NT_ASSOCIATIVE_4(int x, int layer, NeuralNet* net)
{
	int b;

	if(layer == net->SYNAPSE_LAYERS)	//if this is the last neuron layer
		return;		//don't want to mess with layer+1

	//NOTE: OPTIMISE!
	for(b=0; b<net->num_neurons_in_layer[layer + 1]; b++)
		net->Synapse_Array[x][b][layer].weight += 0.01 * net->Neuron_Array[b][layer+1].activation 
														* (0.2 * net->Neuron_Array[x][layer].activation
														- 	net->Synapse_Array[x][b][layer].weight);
}


void NT_ASSOCIATIVE_5(int x, int layer, NeuralNet* net)
{}//pass

// dW b,a (t) = [ E * (Ya(t) - YaAv) * (Yb(t) - YbAv)]
void NT_COVARIANCE(int x, int layer, NeuralNet* net)
{
	int b;
	float average = 0.2;	//NOTE: temp hack

	if(layer == net->SYNAPSE_LAYERS)	//if this is the last neuron layer
		return;		//don't want to mess with layer+1

	for(b=0; b<net->num_neurons_in_layer[layer + 1]; b++)
		net->Synapse_Array[x][b][layer].weight += 0.01 * (net->Neuron_Array[b][layer+1].activation - 0.2) 
														* (net->Neuron_Array[x][layer].activation - 0.2);
}


//dW b,a(t) = E * Ym(t) * F( Ya(t - tau) ) * (Wmax - W b,a(t)) - (G * Ya (t))
void NT_MODULATORY(int x, int layer, NeuralNet* net)
{
	int b;

	if(layer == net->SYNAPSE_LAYERS)	//if this is the last neuron layer
		return;		//don't want to mess with layer+1

	for(b=0; b<net->num_neurons_in_layer[layer + 1]; b++)
		net->Synapse_Array[x][b][layer].weight += 0.01 * net->reinforcement * 0.01 
														* net->Neuron_Array[x][layer].activation
														* net->MAX_WEIGHT - fabs(net->Synapse_Array[x][b][layer].weight)
														- 0.1 * net->Neuron_Array[x][layer].activation;
}
