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
///Q2 Camera Begin
#include "camclient.h"
///Q2 Camera End

void Bot_Spawn(edict_t *ent);
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);

void gonetgo (NeuralNet* net, edict_t* self);
void ClientThink (edict_t *ent, usercmd_t *cmd);

/*==========================================================================
Reference bots

These are bots that exist purely to provide the other bots some variety in 
terms of their opponents.  These bots do not evolve, are not considered part 
of the GA population, and are not pointed to with the 'Neuralbot_Array'.
==========================================================================*/

/*==========================================================================
Cmd_Addrefbot_f
---------------
add a reference bot.  The parameter of the command is the neuralnet DNA to be loaded up
and used as the bot's net.
==========================================================================*/

void Cmd_Addrefbot_f(qboolean svcmd)
{
	char dnafile[100];
	edict_t* refbot;

	if(svcmd)
		strcpy(dnafile, gi.argv(2));//strcpy(gi.argv(2), dnafile);
	else
		strcpy(dnafile, gi.argv(1));//strcpy(gi.argv(1), dnafile);

	//safe_bprintf (PRINT_HIGH, "result of strcpy on dnafile: %s\n", dnafile);

	if(!dnafile || dnafile[0] == '\0')//if player forgot to enter dna file in command
	{
		safe_bprintf (PRINT_HIGH, "use: 'addrefbot filename'\nwhere filename.dna is the dna to use\n");
		return;
	}

	refbot = AddRefBot(dnafile);//spawn a reference bot (minus a brain) into the game
	

	if(!refbot)
		return;

	thegame->refbotnum++;

	if(!SetUpRefBotNet(refbot, dnafile))//if failed to set up bot net properly
	{
		removeRefBot(refbot);//remove the bot
		safe_bprintf (PRINT_HIGH, "failed to set up net; bot removed\n");
	}


}

/*==========================================================================
SetUpRefBotNet
--------------
load in some dna from the file 'dnafile'.
Just use the first lot of bot dna from the file.
==========================================================================*/
qboolean SetUpRefBotNet(edict_t* self, char* dnafile)
{
	
	char filename[100];

	

	FILE	*f;
	long	i;
	int		numDNAstrings;
	int		l,s,d,n;
	float	buffer[20000];//SYNAPSENUMDEPENDENT
	long	bufferlength = 20000;
	long	length;

	long generation;
	long structure_generation;

	long synapse_layers;
	long num_hidden_neurons; 
	

	float version;
	float mut_successes;
	float mut_failures; 

	long neuronsize;


	strcpy(filename, dnafile);
	strcat (filename, ".dna");	  // add a .dna at the end

	safe_bprintf (PRINT_HIGH, "using %s\n", filename);

	f = fopen (filename, "rb");
	if (!f)
	{
		safe_bprintf (PRINT_HIGH, "couldn't open up file!\n");
		return false;
	}

	fread(&generation,sizeof(generation),1,f);
	fread(&structure_generation,sizeof(structure_generation),1,f);

	fread(&synapse_layers,sizeof(synapse_layers),1,f);
	net_SetSynapseLayers(&self->client->botinfo.itsNet, synapse_layers);//set synapse layers
	safe_bprintf (PRINT_HIGH, "	setting refbot's synapse_layers to %i\n", synapse_layers);

	fread(&num_hidden_neurons,sizeof(num_hidden_neurons),1,f);
	net_setNumHiddenNeurons(&self->client->botinfo.itsNet, num_hidden_neurons);
	safe_bprintf (PRINT_HIGH, "	setting refbot's num_hidden_neurons to %i\n", num_hidden_neurons);


	fread(&version,sizeof(version),1,f);
	if(version != 5.5)
		safe_bprintf (PRINT_HIGH, "warning - saved game from old version of code\n");

	fread(&mut_successes,sizeof(mut_successes),1,f);
	fread(&mut_failures,sizeof(mut_failures),1,f);


	// read in number of pieces of Bot DNA in save file
	fread(&numDNAstrings,sizeof(numDNAstrings),1,f);

		
	fread(&length, sizeof(length), 1, f); //read in how much file to read into buffer
	fread(&buffer, sizeof(float) * length, 1, f); //read in data from file to buffer

	//// copy buffer data over to the net structure ////
	i=0;
	for ( l=0; l< thegame->SYNAPSE_LAYERS; l++ ) // synapse layers
		for ( s=0; s< self->client->botinfo.itsNet.num_neurons_in_layer[l]; s++ ) // source neurons
			for ( d=0; d< self->client->botinfo.itsNet.num_neurons_in_layer[l+1]; d++ ) // destination neurons
			{
				self->client->botinfo.itsNet.Synapse_Array[s][d][l].weight = buffer[i];
				i++;
			}



	//// advance to point in file where neuron info is stored ///
	if(numDNAstrings > 1)	//if there are more synapse DNA strings in file than bots playing
	{
		
		for(i=0; i<(numDNAstrings - 1); i++)
		{
			fread(&length, sizeof(length), 1, f);			//read in how much file to read into buffer
			fread(&buffer, sizeof(float) * length, 1, f);	//read data in and do nothing with it
		}
	}

	///// check neuron size ////////
	fread (&neuronsize, sizeof(neuronsize), 1, f);//read size of Neuron structure for checking
	if(neuronsize != sizeof(Neuron))
	{
		fclose (f);
		safe_bprintf(PRINT_HIGH, "ReadNN: mismatched Neuron size.. Aborting\n");
		return true;//synapses are load in so this wil do for now
	}


	////read neurons from file //////////////
	for ( l=0; l< (thegame->SYNAPSE_LAYERS+1); l++ ) // neuron layers
		for ( n=0; n< self->client->botinfo.itsNet.num_neurons_in_layer[l]; n++ ) // neurons				
			fread (&self->client->botinfo.itsNet.Neuron_Array[n][l], sizeof(Neuron), 1, f);			




	fclose (f);	// release the file
	

	safe_bprintf (PRINT_HIGH, "refbot net set up\n");

	return true;

}


/*==========================================================================
AddRefBot
---------
Spawn a reference bot into the game.  Very similar to Addbot()
==========================================================================*/
edict_t* AddRefBot(char* dnafile)
{

	int      i;

	char     userinfo[MAX_INFO_STRING];

	edict_t *bot;

	char	name[100];

	strcpy(name, "refbot[");
	strcat(name, dnafile);
	strcat(name, "]");


	for (i = maxclients->value; i > 0; i--)	//look throught the client type entities
	{
	
		bot = g_edicts + i + 1;

		if (!bot->inuse)	//if found a client type entity that is not in use
			break;
	}



	if (bot->inuse)
		bot = NULL;



	if (bot)
	{
		memset(userinfo, 0, MAX_INFO_STRING);

		
		Info_SetValueForKey(userinfo, "name", name); 
		Info_SetValueForKey(userinfo, "skin", "male/armor7");//custom skin
		Info_SetValueForKey(userinfo, "hand", "2");
		


		ClientConnect(bot, userinfo);//register the bot as connected

		G_InitEdict(bot);//set gravity, inuse for bot

		InitClientResp(bot->client);



		Bot_Spawn(bot);//place the bot into the level

		if(!bot->client)
		{
			safe_bprintf(PRINT_HIGH, "client is null\n");
			return NULL;
		}


		
		//////////// NEURALCODE ///// pointers to bits of my fucked up coding /////
		// point bot's pointer to it's parallel structure containing it's 
		// neuralnet to the right place
		//bot->client->botinfo.Bot_pointer 
		//	= &thegame->Neuralbot_Array[botnum];

		// point the bot's parallel Neuralbot structure's edictpointer to bot.
		//thegame->Neuralbot_Array[botnum].edictpointer
		//	= bot;

		bot->client->botinfo.isrefbot = true;//this is different from normal bot spawn

		bot->client->botinfo.isbot = true;
		bot->client->botinfo.invisible = false;



		// GA stuff: ///
		//bot->client->botinfo.fragslastperiod=0; //and this stuff is not needed
		//bot->client->botinfo.itemsgot=0;
		//bot->client->botinfo.damageinflicted=0;
		//bot->client->botinfo.explorenesssofar=0;
		//bot->client->botinfo.structure_fragslastperiod=0;
		//bot->client->botinfo.structure_itemsgot=0;
		//bot->client->botinfo.structure_damageinflicted=0;
		//bot->client->botinfo.structure_explorenesssofar=0;


		bot->client->botinfo.wasblocked = false;
		////////////////END NEURALCODE//////////////////////////// 

		///Q2 Camera Begin
		EntityListAdd(bot);//NOTE: UNCOMMENT
		///Q2 Camera End

	
		



		gi.WriteByte(svc_muzzleflash);

		gi.WriteShort(bot - g_edicts);
		
		gi.WriteByte(MZ_LOGIN);

		gi.multicast(bot->s.origin, MULTICAST_PVS);//some lights, flash for spawning in



		//safe_bprintf(PRINT_HIGH, "%s entered the game\n", bot->client->pers.netname);

		ClientEndServerFrame(bot);


	}
	else
		gi.dprintf("%s cannot connect - server is full!\n", "Neuralbot");

	return bot;
}

/*============================================================================
Cmd_Removerefbots_f
-------------------
removes all the reference bots from the game.
============================================================================*/
void ClientDisconnect (edict_t *ent);

void Cmd_Removerefbots_f()
{
	
	edict_t* entity = NULL;		// entity currently being considered


	while ((entity = G_Find (entity, FOFS(classname), "player")) != NULL)//search through entities with classname player
		if(entity->client)
			if(entity->client->botinfo.isrefbot)
				removeRefBot(entity);
				

	gi.dprintf("all reference bots removed\n");
}

/*========================================================================
removeRefBot
------------
removes one bot from the game
========================================================================*/
void removeRefBot(edict_t* ent)
{
	ClientDisconnect(ent);
	G_FreeEdict(ent);
}
