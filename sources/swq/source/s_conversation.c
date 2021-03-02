#include "g_local.h"
#include "s_conversation.h"

char	output_string[1024];

void JustTalk(edict_t *ent, edict_t *activator)
{

// Reset conversation variables

	clearglobals();

// Load the referenced conversation file

//	if (!loadconversation(ent->message))
	if (!loadconversation("jdan.ucs"))
		{
		gi.dprintf("Error Loading Conversation File");     
		return;
		}

// A few initialisation variables

	conversation_content[2].flags[3] = 1;
	highlighted = 2;
	NoTouch = 0;


	holdthephone = 1;

	yeah_you = ent;
	its_me = activator;




// Draw up the dialouge

//	ConversationRedraw();

}

//gi.error ("----%s------------ \n", conversation_content[content_x].message);
//if (1)
//	return;



/*****************************************************************************************
	Basic function for loading in a conversation script file
*****************************************************************************************/


int loadconversation(char *filename)
{
	FILE		*current_file;
	cvar_t		*game_dir, *basedir;
	char		file_name[256];
	char		game_dir_name[80];
	char		character;
	char		message[512];
	int			content_x;
	char		temp_char[4];
	int			temp_var;
	char		temp_char2[32];



// Load up the file

	game_dir = gi.cvar ("game", "", 0);
	basedir = gi.cvar ("basedir", ".", 0);

	if (!Q_stricmp (game_dir->string, ""))
		sprintf (game_dir_name, "baseq2");
	else 
		sprintf (game_dir_name, "%s", game_dir->string);

	sprintf (file_name, "%s\\%s\\conversation\\%s", basedir->string, game_dir_name, filename);

	current_file = fopen(file_name, "r");

	if (!current_file)
	{
		gi.dprintf ("Error Opening %s 001\n", filename);
		return(0);
	}


// Initialize variables

	temp_var = 1;
	content_x = 0;

// Load the contents of the script file

	while(temp_var)
	{
		character = fgetc(current_file);

		if (character == '_')
		{	
			fgets(message, 4, current_file);

			if(!Q_stricmp(message, "msg"))
			{
				content_x++;
				fgets(conversation_content[content_x].message, 512, current_file);
				conversation_content[content_x].flags[2] = 1;
			}
			else if(!Q_stricmp(message, "bck"))
			{
				fgets(conversation_content[content_x].background, 512, current_file);
				strcpy(output_string, conversation_content[content_x].background);
				givemequotes(output_string);
				gi.imageindex(output_string);
			}
			else if(!Q_stricmp(message, "nme"))
			{
				fgets(conversation_content[content_x].name, 32, current_file);
			}

			else if(!Q_stricmp(message, "pic"))
			{
				fgets(conversation_content[content_x].portrait, 32, current_file);
				strcpy(output_string, conversation_content[content_x].portrait);
				givemequotes(output_string);
				gi.imageindex(output_string);
			}

			else if(!Q_stricmp(message, "ded"))
			{
			 	conversation_content[content_x].flags[1] = 1;
			}
			else if(!Q_stricmp(message, "wav"))
			{
				fgets(temp_char2, 2, current_file);
				fgets(temp_char2, 32, current_file);
				strncat(conversation_content[content_x].wavfile, temp_char2, strlen(temp_char2) - 1);
			}

			else if(!Q_stricmp(message, "del"))
			{
				fgets(temp_char, 4, current_file);
				conversation_content[content_x].delay = atoi(temp_char);
			}

			else if(!Q_stricmp(message, "tgt"))
			{
				fgets(conversation_content[content_x].direct_target, 32, current_file);
			}

			else if(!Q_stricmp(message, "spt"))
			{
				fgets(conversation_content[content_x].USS_target, 32, current_file);
			}

			else if(!Q_stricmp(message, "con"))
			{
				fgets(conversation_content[content_x].next_dialouge, 32, current_file);
			}

			else if(!Q_stricmp(message, "end"))
			{
				temp_var = 0;
			}

			else
			{
				gi.dprintf ("-_Incorrect Underscore Placement_-\n");
			}


		}

	}

	fclose(current_file);
	return(1);
}



/*****************************************************************************************
	Your basic run of the mill clear all global variables function
*****************************************************************************************/

void clearglobals()
{
	int	loop;

	for (loop = 0; loop > 10; loop++)
	{
		conversation_content[loop].flags[1] = 0;
		conversation_content[loop].flags[2] = 0;
		conversation_content[loop].flags[3] = 0;

		strcpy(conversation_content[loop].portrait, "");

		strcpy(conversation_content[loop].name, "");

		strcpy(conversation_content[loop].wavfile, "");

		strcpy(conversation_content[loop].direct_target, "");

		strcpy(conversation_content[loop].USS_target, "");

		strcpy(conversation_content[loop].next_dialouge, "");
	}

}



/*****************************************************************************************
	A function that (surprise, surprise!) draws the current dialouge
	to the screen. According to the function that I stole the stuff
	I didn't feel like getting gritty with, it does this by putting it
	up on the help screen. I would assume this has something to do with
	that "layout" stuff in the p_hud.c file, but quite honestly, if it
	works, it's fine in my book.
*****************************************************************************************/

char *ConversationRedraw()
{
	char		string[1024];
	char		tmp_string[128];
	int			loop;
	int			y_distance;

// Init variables

	strcpy(output_string, "") ;

// Adds a Background image if such has been specified

	if(Q_stricmp(conversation_content[1].background, ""))
	{
		strcpy(string, "xv 0 yv 0 picn");
		strncat(string, conversation_content[1].background, strlen(conversation_content[1].background) -1);
	}

// Adds a "name" entry to the string if such an entry exists

	if(Q_stricmp(conversation_content[1].name, ""))
	{
		if(Q_stricmp(conversation_content[1].background, ""))
			strcat(string, " xv 4 yv 10 string ");
		else
			strcpy(string, "xv 4 yv 10 string ");
		strncat(output_string, conversation_content[1].name, strlen(conversation_content[1].name) - 1);
//		givemequotes2();
		strcat(string, output_string);
	}
	
// Adds your basic portrait file if such has been specified

	if(Q_stricmp(conversation_content[1].portrait, ""))
	{
		strcat(string, " xv 229 yv 0 picn");
		strncat(string, conversation_content[1].portrait, strlen(conversation_content[1].portrait) -1);
	}


// Adds the main message over 6 lines. I've set a practical limit at
// 6 lines, a limit because if you have more to say then fits in 6
// lines, people probably don't want to hear it, and practical since
// it damned well makes my job easier.

	if(conversation_content[1].flags[2] == 1)
	{
		strcat(string, " xv 20 yv 26 string ");
		strcat(string, MakeMeStrings(conversation_content[1].message, 20, 0));
		if(strlen(conversation_content[1].message) > 20)
		{
			strcat(string, " xv 20 yv 34 string ");
			strcat(string, MakeMeStrings(conversation_content[1].message, 20, 1));
		}
		if(strlen(conversation_content[1].message) > 40)
		{
			strcat(string, " xv 20 yv 42 string ");
			strcat(string, MakeMeStrings(conversation_content[1].message, 20, 2));
		}
		if(strlen(conversation_content[1].message) > 60)
		{
			strcat(string, " xv 20 yv 50 string ");
			strcat(string, MakeMeStrings(conversation_content[1].message, 20, 3));
		}
		if(strlen(conversation_content[1].message) > 80)
		{
			strcat(string, " xv 20 yv 58 string ");
			strcat(string, MakeMeStrings(conversation_content[1].message, 20, 4));
		}
		if(strlen(conversation_content[1].message) > 100)
		{
			strcat(string, " xv 20 yv 66 string ");
			strcat(string, MakeMeStrings(conversation_content[1].message, 20, 5));
		}
	}

// Here's the loop that prints the conversation options to the screen.
// First, it checks if the option is available for printing, afterwhich
// it checks if it should be highlighted or not. Before this, is a small
// routine that decides where on the screen the current text should be
// printed.


	loop = 1;

	while(loop != 10)
	{
	loop++;
		if(conversation_content[loop].flags[2] == 1)
		{
			strcat(string, " xv 0 yv ");
			y_distance = ((loop * 40) + 30);
			sprintf (tmp_string, "%d ", y_distance);
			strcat(string, tmp_string);
			if(conversation_content[loop].flags[3] == 1)
			{
				strcat(string, "string2 ");
			}
			else
			{
				strcat(string, "string ");
			}
			strcat(string, MakeMeStrings(conversation_content[loop].message, 20, 0));
			if(strlen(conversation_content[loop].message) > 20)
			{
				strcat(string, " xv 0 yv ");
				y_distance = ((loop * 40) + 38);
				sprintf (tmp_string, "%d ", y_distance);
				strcat(string, tmp_string);
				if(conversation_content[loop].flags[3] == 1)
				{
					strcat(string, "string2 ");
				}
				else
				{
					strcat(string, "string ");
				}
				strcat(string, MakeMeStrings(conversation_content[loop].message, 20, 1));
			}
			if(strlen(conversation_content[loop].message) > 40)
			{
			strcat(string, " xv 0 yv ");
			y_distance = ((loop * 40) + 46);
			sprintf (tmp_string, "%d ", y_distance);
			strcat(string, tmp_string);
				if(conversation_content[loop].flags[3] == 1)
				{
					strcat(string, "string2 ");
				}
				else
				{
					strcat(string, "string ");
				}
				strcat(string, MakeMeStrings(conversation_content[loop].message, 20, 2));
			}
			if(strlen(conversation_content[loop].message) > 60)
			{
				strcat(string, " xv 0 yv ");
				y_distance = ((loop * 40) + 52);
				sprintf (tmp_string, "%d ", y_distance);
				strcat(string, tmp_string);
				if(conversation_content[loop].flags[3] == 1)
				{
					strcat(string, "string2 ");
				}
				else
				{
					strcat(string, "string ");
				}
				strcat(string, MakeMeStrings(conversation_content[loop].message, 20, 3));
			}
		}
	}
	
	if(strlen(string) > 1024)
	{
		gi.dprintf("Error: Too long string");
		strcpy(string, "xv 12 yv 12 string2 \"Error; Too long string\" ");
		return(string);
	}

//	ent->client->showinventory = false;
//	ent->client->showhelp = false;
//	ent->client->showscores = false;

//	strcpy(string, "xv 1 yv 1 string2 \"Spectators\" ");

//	strcpy(output_string, string);
//	givemequotes();
//	strcpy(string, output_string);

//	gi.error("%s\n", string);
	
//	gi.WriteByte (svc_layout);
//	gi.WriteString (string);
//	gi.unicast (ent, false);

	return(string);

//gi.dprintf ("%s\n", string);
}





/*****************************************************************************************
	This function splits a string into X strings of the length required
	and returns the one that is requested
*****************************************************************************************/

char *MakeMeStrings(char *original_string, int wanted_strlngth, int string_request)
{

	int		PointInString;
	int		x;
	int		tmp_strlngth;
	int		PointInOutString;

// Safety Check

//	if(strlen(original_string) < ((wanted_strlngth * string_request)+1))
//		return("-_Error_-");

// Decide where in the sent string to start creating the new string from

	PointInOutString = 0;
	PointInString = wanted_strlngth * string_request;

// First check if the wanted string is from the beginning of the old one
// Otherwise, backpedal untill a space is found

	if(PointInString != 0)
	{
		while(original_string[PointInString - 1] != '_')
		{
			PointInString--;
		}
	}
	else
	{
		while((original_string[PointInString]) == '_')
			PointInString++;
	}

// A Loop To Create the new string's contents. Stops operating when the
// Beginning of an eventuell new string begins or a '\0' or '\n' character
// is reached

	while(1)
	{

// Check if the next word is longer then the allotted amount of characters
// that are left for use in the new string

		tmp_strlngth = 0;
		x = 0;

		while(x != 1)
		{
			tmp_strlngth++;
			PointInString++;
			switch(original_string[PointInString])
			{
				case '\0':	x = 1;
				case '\n':	x = 1;
				case '_':	x = 1;
			};
		}


		
// If the next word was longer then the allotted characters, simply end this
// function by sending the output string to the origin of the request

		if(PointInString > wanted_strlngth * (string_request + 1))
			{
//			givemequotes2();
			return(output_string);
			}

// If the word wasn't to long, then we'd better add it to the output string

		x = PointInString;

		PointInString = PointInString - tmp_strlngth;
		while(PointInString != x)
		{
			output_string[PointInOutString] = original_string[PointInString];
			PointInString++;
			PointInOutString++;
		}



		output_string[PointInOutString + 1] = '_';
		output_string[PointInOutString + 2] = '\n';
		output_string[PointInOutString + 3] = '\0';

// Now that that's finished, check if the variable we're sitting on is a naughty
// space character, and if it is, add it to the string and knock the pointer up one.
// If it's an end of string character, end the function and return an output string.

		x = 0;

		switch(original_string[PointInString])
		{
			case '\0':	x = 2;
			case '\n':	x = 2;
		};


		if(x == 2)
		{
			output_string[PointInOutString + 1] = '\0';
//			givemequotes2();
			return(output_string);
		}

	}

return("-_Error_-2");
}



/*****************************************************************************************
	A hastily created function I made when I noticed that quotes where needed
	when sending strings to the blablabla_layout thingamajiggy. Gahh...
*****************************************************************************************/

void givemequotes(char *string)
{
	int			x;
	int			strlngth;
	char		tmp_string[1024];



	strlngth = strlen(string);
	x = 0;
	tmp_string[0] = '"';

	while(x != strlngth)
	{
		tmp_string[x + 1] = string[x];
		x++;
	}

	tmp_string[x + 1] = '"';

	tmp_string[x + 2] = string[x];

	strcpy(string, tmp_string);

//	return(tmp_string);

}

void givemequotes2()
{


	int			x;
	int			strlngth;
	char		tmp_string[1024];



	strlngth = strlen(output_string);
	x = 0;
	tmp_string[0] = '\\';
	tmp_string[1] = '"';

	while(x != strlngth)
	{
		tmp_string[x + 2] = output_string[x];
		x++;
	}

	tmp_string[x + 2] = '\\';

	tmp_string[x + 3] = '"';

	tmp_string[x + 4] = '\0';
//	tmp_string[x + 4] = output_string[x];

	strcpy(output_string, tmp_string); 

}

void IChooseYou()
{
	FILE		*current_file;
	cvar_t		*game_dir, *basedir;
	char		file_name[256];
	char		game_dir_name[80];
	edict_t		*t;

// Checks if there is a wave file to play. If so, sets the delay, plays the wave, and ends
// the function. Also erases all of the conversation except for the highlighted response.

	NoTouch = 1;

	if(Q_stricmp(conversation_content[highlighted].wavfile, ""))
	{

//Check if the file exists

		game_dir = gi.cvar ("game", "", 0);
		basedir = gi.cvar ("basedir", ".", 0);
	
		if (!Q_stricmp (game_dir->string, ""))
			sprintf (game_dir_name, "baseq2");
		else 
			sprintf (game_dir_name, "%s", game_dir->string);
	
		sprintf (file_name, "%s\\%s\\sound\\%s", basedir->string, game_dir_name, conversation_content[highlighted].wavfile);

		current_file = fopen(file_name, "r");
	
		if (!current_file)
		{
			t = G_Spawn();
			t->nextthink = 1;
			t->think = IChooseYou2;
			//gi.error("93204823\n");
			return;
		}

		fclose(current_file);
		strcpy(output_string, conversation_content[highlighted].wavfile);
		givemequotes(output_string);
//		soundex = gi.soundindex (output_string);
//		gi.sound (its_me, CHAN_VOICE, soundex, 1, ATTN_NORM, 0);
//		gi.error ("blah");
		gi.sound (its_me, CHAN_VOICE, gi.soundindex(output_string), 1, ATTN_NORM, 0);
		t = G_Spawn();
		t->nextthink = conversation_content[highlighted].delay;
		t->think = IChooseYou2;

	}

// Basic occupational blah blah blah (ok, so I didn't feel like explaining myself. Shoot me)

		if(Q_stricmp(conversation_content[highlighted].direct_target, ""))
	{
		t = G_Spawn();
		t->target = conversation_content[highlighted].direct_target;
		G_UseTargets (t, its_me);
		G_FreeEdict (t);
	}
	

	if(conversation_content[highlighted].flags[1] == 1)
	{
		holdthephone = 0;
		NoTouch = 0;
		its_me->client->showscores = false;
		return;
	}

	if(Q_stricmp(conversation_content[highlighted].next_dialouge, ""))
	{
		if(!loadconversation(conversation_content[highlighted].next_dialouge))
			{
			gi.dprintf("Error Loading Conversation File");
			NoTouch = 0;
			return;
			}
		conversation_content[2].flags[3] = 1;
		highlighted = 2;
		NoTouch = 0;
		holdthephone = 1;
	}
}


void IChooseYou2(edict_t *ent)
{
	edict_t		*t;
	
	G_FreeEdict (ent);

	if(Q_stricmp(conversation_content[highlighted].direct_target, ""))
	{
		t = G_Spawn();
		t->target = conversation_content[highlighted].direct_target;
		G_UseTargets (t, its_me);
		G_FreeEdict (t);
	}
	

	if(conversation_content[highlighted].flags[1] == 1)
	{
		holdthephone = 0;
		NoTouch = 0;
		its_me->client->showscores = false;
		return;
	}

	if(Q_stricmp(conversation_content[highlighted].next_dialouge, ""))
	{
		if(!loadconversation(conversation_content[highlighted].next_dialouge))
			{
			gi.dprintf("Error Loading Conversation File");
			NoTouch = 0;
			return;
			}
		conversation_content[2].flags[3] = 1;
		highlighted = 2;
		NoTouch = 0;
		holdthephone = 1;
	}

}
