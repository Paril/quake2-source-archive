#include "g_local.h"

char *ED_NewString (char *string);

//load messages for deathmatch
void load_bot_messages(edict_t *ent)
{
	FILE *in;
	char filename[25] = "lee/bots/killer.txt", temp[60];
	char filename2[25] = "lee/bots/killed.txt", c;
	int i;	

	if ((in = fopen(filename, "r")) == NULL) 
			gi.dprintf ("Could not load message file %s.\n", filename);
		else 	
	{
		//found the file
		bot_total_msg_killer = 1;
		memset(temp, 0, 60);
		i = 0;

		while ((c = fgetc(in)) != EOF)
		{			
			
			if (c == 10)
			{
				strcpy(bot_msg_killer[bot_total_msg_killer], temp);
				bot_total_msg_killer++;				

				memset(temp, 0, 60);

				i = 0;				
			} else {
				temp[i] = c;
				i++; 
			}
		}		
		
	}
	fclose(in);

	
	//load killed messages
		if ((in = fopen(filename2, "r")) == NULL) 
			gi.dprintf ("Could not load message file %s.\n", filename2);
		else 	
	{
		//found the file
		bot_total_msg_killed = 1;
		memset(temp, 0, 60);
		i = 0;

		while ((c = fgetc(in)) != EOF)
		{			
			
			if (c == 10)
			{
				strcpy(bot_msg_killed[bot_total_msg_killed], temp);
				bot_total_msg_killed++;				

				memset(temp, 0, 60);

				i = 0;				
			} else {
				temp[i] = c;
				i++; 
			}
		}		
		
	}

	//subtract one
	bot_total_msg_killer--;
	bot_total_msg_killed--;

	gi.dprintf ("%i killer and %i killed message(s)\n", bot_total_msg_killer, bot_total_msg_killed);
	fclose(in);
}

//load names and skins for bots
void load_bot_names(edict_t *ent)
{
	FILE *in;
	char filename[25] = "lee/bots/names.txt", temp[60];
	char c;
	int i;	

	if ((in = fopen(filename, "r")) == NULL) 
			gi.dprintf ("Could not load name file %s.\n", filename);
		else 	
	{
		//found the file
		bot_name_total = 0;		

		for (i = 0; i < 20; i++)
			memset(bot_names[i], 0, 60);

		memset(temp, 0, 60);
		i = 0;

		while ((c = fgetc(in)) != EOF)
		{			
			
			if (c == 10)
			{
				strcpy(bot_names[bot_name_total], temp);
				bot_name_total++;				
				
				memset(temp, 0, 60);

				i = 0;				
			} else {
				temp[i] = c;
				i++; 
			}
		}		
		
		gi.dprintf ("%i names and skins loaded.\n",bot_name_total);

	}
	fclose(in);
}

//load the bot_points from file and put into the game world


void Make_bot_points (edict_t *ent)
{
	
	char temp1[128], temp2[40] = "lee/routes/", org1[10], org2[10], org3[10];
	FILE *in;
	int file_status;
	float v1,v2,v3;
		

	strcat (temp2, level.mapname);
	strcat (temp2, ".bpf");

	if ((in = fopen(temp2, "r")) == NULL) 
			gi.dprintf ("Could not load route file %s.\n", temp2);		
		
		else 
		{		
			file_status =3;
			bot_point_stuff = 0;

			while (file_status == 3)
			{
					edict_t *bot_point;
				
					bot_point = G_Spawn();					
					file_status = fscanf( in, "%s %s %s\n", org1, org2, org3);					
					if (file_status == -1)
						break;

					v1 = atof(org1);
					v2 = atof(org2);
					v3 = atof(org3);

					((float *)(bot_point->s.origin))[0] = v1;
					((float *)(bot_point->s.origin))[1] = v2;
					((float *)(bot_point->s.origin))[2] = v3;

					bot_point->classname = "bot_point";
					bot_point->svflags = SVF_NOCLIENT;
					bot_point->solid = SOLID_TRIGGER;

					if (bot_point_stuff == 0) bot_point_stuff = 1;
	
					sprintf(temp1, "%i_route", bot_point_stuff);
					bot_point->targetname = ED_NewString(temp1);

					bot_point_stuff++;	
	
					sprintf(temp1, "%i_route", bot_point_stuff);
					bot_point->targetname = ED_NewString(temp1);
					
//					gi.dprintf ("%s - %s\n",temp1, vtos(bot_point->s.origin) );

					gi.linkentity (bot_point);
			}
			gi.dprintf ("%i point(s) loaded.\n", bot_point_stuff-1);
			fclose(in);
		}
}


void create_bot_point(edict_t *ent)
{
	edict_t *bot_point;
	char temp1[128],temp2[40] = "lee/routes/";
	FILE *in;

	// spawn the bot on a spawn spot
	bot_point = G_Spawn();

	VectorCopy (ent->s.origin, bot_point->s.origin);	
	bot_point->classname = "bot_point";

	bot_point->solid = SOLID_TRIGGER;		
//	bot_point->model = "models/objects/gibs/head2/tris.md2";	
	bot_point->s.frame = 0;
	bot_point->s.skinnum = 1;
	bot_point->clipmask = MASK_OPAQUE;

	VectorSet (bot_point->mins, -8, -8, -8);
	VectorSet (bot_point->maxs, 8, 8, 8);
	bot_point->svflags |= SVF_NOCLIENT;
	bot_point->s.origin[2] += 3;


	if (bot_point_stuff == 0) bot_point_stuff = 1;
	
	sprintf(temp1, "%i_route", bot_point_stuff);
	bot_point->targetname = ED_NewString(temp1);

	bot_point_stuff++;	

	sprintf(temp1, "%i_route", bot_point_stuff);
	bot_point->target = ED_NewString(temp1);

	gi.dprintf ("Bot created at:%s %s %s\n",bot_point->targetname, bot_point->target, vtos(bot_point->s.origin));

	gi.linkentity(bot_point);

	//save the location to file
	//temp2 ;
	strcat (temp2, level.mapname);
	strcat (temp2, ".bpf");
	in  = fopen( temp2, "a");
	fprintf( in, "%f %f %f\n", bot_point->s.origin[0], bot_point->s.origin[1],bot_point->s.origin[2]);
	fclose(in);

}
