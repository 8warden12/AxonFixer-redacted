#ifndef NC2_C
#define NC2_C

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <cassert>
using namespace std;

#define E 2.71828
#define THRESHOLD 20.0f
#define ACTIVATION_CHANCE(x) (x>THRESHOLD) ? 1.0f : pow(x/(float)THRESHOLD,E)
#define RF01() rand()/(double)RAND_MAX


struct neuron;

#define D_STS 1.0f
#define D_RECAL 1.0f
#define TIME_STEP 0.1f

struct connection{//connections between neurons
	
	float sts = D_STS;
	float lts = 0.0f;
	
	float recalcitrance = D_RECAL;
	
	neuron* presynaptic;
	neuron* postsynaptic;
	
};


enum NEURON_TYPE {
	NORMAL = 0,
	INPUT = 1,
	OUTPUT = 2,
	PATTERN_GENERATOR = 3,
};

struct neuron{// neurons
	vector<connection*> inbound;//connections coming into the neuron
	vector<connection*> outbound;//connections sent out by the neuron
	
	int neuronID = 0;//ID = index
	
	float sum = 0.0f;//Sum of the signals coming into the neuron, neuron will activate if this breaches the threshhold
	float resting_timer = 0.0f;//how long the neuron has been at rest
	bool activated = false; //whether the neuron is activated or not
	
	NEURON_TYPE type = NORMAL;
};

neuron* newNeuron()
{
	neuron *n = new neuron;
	return n;
}
void newConnection(neuron* presynaptic, neuron* postsynaptic, float weight = D_STS, float recalcitrance = D_RECAL)// create a new connection between 2 neurons (referenced by their IDs)
{
	connection *c = new connection;
	c->presynaptic = presynaptic;
	c->postsynaptic = postsynaptic;

	c->sts = weight;
	c->recalcitrance = recalcitrance;
	presynaptic->outbound.push_back(c); 
	postsynaptic->inbound.push_back(c);
}


struct network{// brain networks
	vector<neuron*> input_neurons;//neurons that receive external input
	vector<neuron*> output_neurons;//neurons that directly affect ations made by the "organism"
	
		
	vector<neuron*> all_neurons;//all neurons in the network, regardless of role
		
};


int save_network(const network* n, const char* filename)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return -1;
	}
	FILE *f = fopen(filename,"w");
	if(!f)
	{
		return -1;
	}
	fprintf(f,"%d\n",n->all_neurons.size());
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		fprintf(f,"%d %d %d %d\n",i,n->all_neurons[i]->type,
		n->all_neurons[i]->inbound.size(),
		n->all_neurons[i]->outbound.size());
		for(int x = 0; x<n->all_neurons[i]->inbound.size(); ++x)
		{
			fprintf(f,"%d %f %f ",n->all_neurons[i]->inbound[x]->presynaptic->neuronID,
			n->all_neurons[i]->inbound[x]->sts,n->all_neurons[i]->inbound[x]->recalcitrance);
		}
	/*	fputc('\n',f);
		for(int x = 0; x<n->all_neurons[i]->outbound.size(); ++x)
		{
			fprintf(f,"%d %f ",n->all_neurons[i]->outbound[x]->postsynaptic->neuronID,
			n->all_neurons[i]->outbound[x]->sts);
		}*/
		fputs("\n\n",f);
	}
	
	fclose(f);
	return 0;
}

network* loadNetwork(const char* filename)
{
	network* n = new network;
	FILE* f = fopen(filename,"r");
	if(!f)
	return NULL;
	
	int num_neurons = 0;
	int num_inbound = 0;
	int num_outbound = 0;
	int otherIndex = 0;
	float strength = 0.0f;
	float recal = 0.0f;
	
	fscanf(f,"%d\n",&num_neurons);

	for(int i = 0; i<num_neurons; ++i)
		n->all_neurons.push_back(newNeuron());
	
	for(int i = 0; i<num_neurons; ++i)
	{	
		fscanf(f,"%d %d %d %d\n",&n->all_neurons[i]->neuronID,
		&n->all_neurons[i]->type,&num_inbound,&num_outbound);
		
		if(n->all_neurons[i]->neuronID == INPUT)
		{
			n->input_neurons.push_back(n->all_neurons[i]);
		}else if(n->all_neurons[i]->neuronID == OUTPUT)
		{
			n->output_neurons.push_back(n->all_neurons[i]);
		}
		
		for(int x = 0; x<num_inbound; ++x)
		{
			fscanf(f,"%d %f %f ",&otherIndex,&strength,&recal);
			newConnection(n->all_neurons[otherIndex],n->all_neurons[i],strength,recal);
		}
		/*for(int x = 0; x<num_outbound; ++x)
		{
			fscanf(f,"%d %f ",&otherIndex,&strength);
			newConnection(n->all_neurons[i],n->all_neurons[otherIndex],strength);
		}*/
	}
	fclose(f);
	return n;
}

void display_minimalist(const network* n)
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
			printf("1");
		}else{
			printf("0");
		}
	}
	printf("\n");
}

void firingSumDisplay(const network* n)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		printf("Neuron %d has sum %f and is activated? %d\n",i,n->all_neurons[i]->sum,n->all_neurons[i]->activated);
	}
}

void display_nstats(const network *n)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	
	printf("%d %d %d\n",n->all_neurons.size(),n->input_neurons.size(),n->output_neurons.size());
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		printf("neuron %d has %d inbound and %d outbound connections."
		"sum = %f, TSA = %f\n",
		n->all_neurons[i]->neuronID,n->all_neurons[i]->inbound.size(),n->all_neurons[i]->outbound.size(),
		n->all_neurons[i]->sum,n->all_neurons[i]->resting_timer);

	}
}

void display_connections(const network* n)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		printf("Neuron %d has the following connections: \n",n->all_neurons[i]->neuronID);
		for(int x = 0; x<n->all_neurons[i]->outbound.size(); ++x)
		{
			printf("to neuron %d sts = %f, recal = %f\n",n->all_neurons[i]->outbound[x]->postsynaptic->neuronID,
			n->all_neurons[i]->outbound[x]->sts,n->all_neurons[i]->outbound[x]->recalcitrance
			);
		}

		for(int x = 0; x<n->all_neurons[i]->inbound.size(); ++x)
		{
			printf("from neuron %d sts = %f, recal = %f\n",n->all_neurons[i]->inbound[x]->presynaptic->neuronID,
			n->all_neurons[i]->inbound[x]->sts,n->all_neurons[i]->inbound[x]->recalcitrance
			);
		}
	}
}

bool connectionExists(const neuron* presynaptic,const neuron* postsynaptic)//for checking if a connection already exists
{
	for(int i = 0; i<presynaptic->outbound.size(); ++i)
	{
		if(presynaptic->outbound[i]->postsynaptic == postsynaptic)
		return true;
	}
	
	return false;
}

void deleteConnection(neuron* presynaptic, neuron* postsynaptic)
{
	
	connection *c;
	for(int i = 0; i<presynaptic->outbound.size(); ++i)
	{
		if(presynaptic->outbound[i]->postsynaptic == postsynaptic)
		{
			c = presynaptic->outbound[i];
			presynaptic->outbound.erase(presynaptic->outbound.begin()+i);
			break;
		}
	}
	
	for(int i = 0; i<postsynaptic->inbound.size(); ++i)
	{
		if(postsynaptic->inbound[i]->presynaptic == presynaptic)
		{
			postsynaptic->inbound.erase(postsynaptic->inbound.begin()+i);
			break;
		}
	}
	
	delete c;
	c = NULL;
	
	
}

void delete_neuron(neuron* n)
{
	int num_o = n->outbound.size();
	int num_i = n->inbound.size();
	
	for(int i = 0; i<n->outbound.size(); ++i)
	{
		deleteConnection(n,n->outbound[i]->postsynaptic);
		--i;
	}
	
	for(int i = 0; i<n->inbound.size(); ++i)
	{
		deleteConnection(n->inbound[i]->presynaptic,n);
		--i;
	}
}

network* newNetwork(int neurons, int input_neurons, int output_neurons, float density)//returns a pointer to the network
{
	network* n = new network;
	
	for(int i = 0; i<neurons; ++i)//put neurons in the network
	{
		n->all_neurons.push_back(newNeuron());
		n->all_neurons[i]->neuronID = i;
	}
	
	for(int i = 0; i<input_neurons; ++i)
	{
		n->input_neurons.push_back(n->all_neurons[i]);//the first neurons become input_neurons
		n->all_neurons[i]->type = INPUT;
	}
	
	for(int i = 0; i<output_neurons; ++i)
	{
		n->output_neurons.push_back(n->all_neurons[neurons-(i+1)]);//the last neurons become output_neurons
		n->all_neurons[neurons-(i+1)]->type = OUTPUT;
	}
	
	int r1 = 0;//ID of the other neuron
	
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		for(int x = 0; x<n->all_neurons.size()*density; ++x)
		{
			if(n->all_neurons.size() > 0)
			{
				r1 = rand()%n->all_neurons.size();
				if(i != r1 && !connectionExists(n->all_neurons[i],n->all_neurons[r1]))
				newConnection(n->all_neurons[i],n->all_neurons[r1],
				rand()%RAND_MAX/(RAND_MAX * D_STS)
				);
			}
		}
	}
	return n;

}



void insertInputVals(const network *n, vector<bool> inputVals)
{
	assert(n->input_neurons.size() >= inputVals.size());
	for(int i = 0; i<n->input_neurons.size(); ++i)//assigning activations from inputVals
	{
		n->input_neurons[i]->activated = inputVals[i];
	}
}

network* networkcpy(const network *source)
{
	network *n = new network;

	for(int i = 0; i<source->all_neurons.size(); ++i)
	{
	//	printf("%d ",source->all_neurons[i]->neuronID);
		n->all_neurons.push_back(newNeuron());
		n->all_neurons[i]->neuronID = i;
	}
//	printf("\n");
//assert(source->all_neurons.back()->neuronID == source->all_neurons.size()-1);
	for(int i = 0; i<source->all_neurons.size(); ++i)
	{
		
		n->all_neurons[i]->type = source->all_neurons[i]->type;
		if(n->all_neurons[i]->type == INPUT)
			n->input_neurons.push_back(n->all_neurons[i]);
			
		if(n->all_neurons[i]->type == OUTPUT)
			n->output_neurons.push_back(n->all_neurons[i]);
		
		for(int x = 0; x<source->all_neurons[i]->outbound.size(); ++x)
		{
		//	assert(source->all_neurons[i]->outbound[x]->postsynaptic->neuronID < n->all_neurons.size());
		//	printf("X %d %d %d %d %d\n",x,source->all_neurons[i]->outbound.size(),source->all_neurons[i]->outbound[x]->sts,source->all_neurons[i]->outbound[x]->postsynaptic->neuronID,n->all_neurons.size());
			newConnection(n->all_neurons[i],n->all_neurons[source->all_neurons[i]->outbound[x]->postsynaptic->neuronID],
			source->all_neurons[i]->outbound[x]->sts,source->all_neurons[i]->outbound[x]->recalcitrance);
		}
			
	}
	
	return n;
	
}



void delete_network(network *n)
{
	if(!n)
	{
		fprintf(stderr,"Error: NULL passed as argument in function %s()\n",__FUNCTION__);
		return;
	}
	
	for(int i = 0; i<n->all_neurons.size(); ++i)
	{
		for(int x = 0; x<n->all_neurons[i]->outbound.size(); ++x)
		{
			delete n->all_neurons[i]->outbound[x];
		}
		delete n->all_neurons[i];
	}
	
	n = NULL;
}


#endif
