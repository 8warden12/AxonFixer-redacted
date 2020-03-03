#ifndef MECHANIC_C
#define MECHANIC_C

#include "NC2.c"
#include <math.h>

#define SIGM(x) (double)exp(x)/(1.0f + (double)exp(x))

#define RECAL_INCREASE 2.0f
#define RECAL_DECREASE 2.0f

//by the Alternating thought theorem, the strengthening of inhibition should, on average,
//be weaker than the strengthening of excitation.
#define INHIBITORY_STRENGTHENING_MULTIPLIER 0.7

//change the attributes of neuron* n, which has just activated.
//FUNCTION NOTES:
//An open question is whether or not the change in recalcitrance should be proportional to dSTS
void activation_plasticity(neuron* n)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed to %s()\n",__FUNCTION__);
		return;
	}
	
	//look at inbound neurons and strengthen connecting synapses if a pre-synaptic neuron has
	//also just fired recently
	for(int i = 0; i<n->inbound.size(); ++i)
	{
		//if activation coincides with the activation of a connected neuron, do nothing (?)
		if(n->inbound[i]->presynaptic->resting_timer > 0.0f) 
		{
			//recalcitrance-mechanic: since wer are STRENGTHENING(+) the presynaptic connections,
			//the recalcitrance should only work its stubbornness if the CURRENT STRENGTH is INHIBITORY(?)
			if(n->inbound[i]->sts > 0.0f)//excitatory (cooperative)
			{
				n->inbound[i]->sts += (1/(n->inbound[i]->presynaptic->resting_timer + 0.1));
				
				//change recalcitrance accordingly (increase)
				n->inbound[i]->recalcitrance += (1/(n->inbound[i]->presynaptic->resting_timer + 0.1));
			}else if(n->inbound[i]->sts < 0.0f)//(inhibitory
			{
				
				
				n->inbound[i]->sts += (2.0f*SIGM(-n->inbound[i]->recalcitrance)/(n->inbound[i]->presynaptic->resting_timer + 0.1));
				
				//change recalcitrance accordingly (decrease)
				n->inbound[i]->recalcitrance -= (2.0f*SIGM(-n->inbound[i]->recalcitrance)/(n->inbound[i]->presynaptic->resting_timer + 0.1));
			}
			
		}
	}
	
	//Paddle through the list of outbound neurons and weaken the connections joining us if that neuron
	//had just fired
	//this loop should be a mirror of the above loop; instead od strengthening, we are weakening,
	for(int i = 0; i<n->outbound.size(); ++i)
	{
		if(n->outbound[i]->postsynaptic->resting_timer > 0.0f) 
		{
			if(n->outbound[i]->sts > 0.0f)//excitatory (noncooperative)
			{
				n->outbound[i]->sts -= (2.0f*SIGM(-n->outbound[i]->recalcitrance)/(n->outbound[i]->postsynaptic->resting_timer + 0.1))
				* INHIBITORY_STRENGTHENING_MULTIPLIER;
				
				n->outbound[i]->recalcitrance -= (2.0f*SIGM(-n->outbound[i]->recalcitrance)/(n->outbound[i]->postsynaptic->resting_timer + 0.1));
			}else if(n->outbound[i]->sts < 0.0f)//(inhibitory
			{
				n->outbound[i]->sts -= (1/(n->outbound[i]->postsynaptic->resting_timer + 0.1))
				* INHIBITORY_STRENGTHENING_MULTIPLIER;
				
				//increase recal by |dSTS| for now
				n->outbound[i]->recalcitrance += (1/(n->outbound[i]->postsynaptic->resting_timer + 0.1));
			}
			
		}
	}
}


//update the "temporal" attributes of neurons, such as resting_timer
void temporal_cycle(const network* n)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		n->all_neurons[i]->resting_timer += TIME_STEP;
	}
}


void activation_cycle(network *n)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		if(ACTIVATION_CHANCE(n->all_neurons[i]->sum) > RF01())
		{
			n->all_neurons[i]->activated = true;
			n->all_neurons[i]->resting_timer = 0.0f;
			n->all_neurons[i]->sum = 0;
		}
	}
}


void summation_cycle(network *n)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		if(n->all_neurons[i]->activated)
		{
			//propagate signal
			for(int x = 0; x<n->all_neurons[i]->outbound.size(); ++x)
			{
				n->all_neurons[i]->outbound[x]->postsynaptic->sum += n->all_neurons[i]->outbound[x]->sts;
			}
			n->all_neurons[i]->activated = false;
			
			activation_plasticity(n->all_neurons[i]);
		}
	}
}
#endif
