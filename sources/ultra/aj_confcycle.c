/* config cycling */

#include "g_local.h"
#include "aj_confcycle.h"

conflist_node *config_list=NULL, *this_config=NULL;


void clear_configlist(void)
{
	conflist_node *current=config_list, *next_node;

	while (current)
	{
		next_node = current->next;

		if (current->filename)
			free(current->filename);
		free(current);
		current=next_node;
	}

	config_list=NULL;
	this_config=NULL;
}


void add_config(char *config)
{
	conflist_node *current, *new_node;

	for (current=config_list; (current && current->next); current=current->next)
		;

	if (!current)
	{
		current=(conflist_node *) gi.TagMalloc (sizeof(conflist_node), TAG_GAME);
//		current=(conflist_node *) malloc (sizeof(replace_details));
		if (!current)
			return; // should signal an error, but wtf...

		current->next=NULL;
		config_list = current;
		new_node=current;
	}
	else
	{
		new_node=(conflist_node *) gi.TagMalloc (sizeof(conflist_node), TAG_GAME);
//		new_node=(conflist_node *) malloc (sizeof(conflist_node));
		if (!new_node)
			return; // should signal an error, but wtf...

		new_node->next=NULL;

		current->next = new_node;
	}

	new_node->filename=(char *) gi.TagMalloc (sizeof(char)*(strlen(config)+1), TAG_GAME);
	if (!new_node->filename)
		return;
	strcpy(new_node->filename, config);
}




void read_configlist(void)
{
	FILE	*f;
	int		i, configs=0;
	char	filename[256];
	cvar_t	*game_dir;
	char	strbuf[256];
	int		newline;

	game_dir = gi.cvar ("game", "", 0);

#ifdef	_WIN32
	i =  sprintf(filename, ".\\");
	i += sprintf(filename + i, game_dir->string);
	i += sprintf(filename + i, "\\");
	i += sprintf(filename + i, config_file->string);
#else
      strcpy(filename, "./");
      strcat(filename, game_dir->string);
      strcat(filename, "/");
	  strcat(filename, config_file->string);
#endif

	f = fopen (filename, "r");
	if (!f)
	{
//		gi.error("Unable to read the replace file.\n");
		return;
	}

	clear_configlist();

	// initialise the teams
	for (i=0; i<MAX_TEAMS; i++)
		bot_teams[i] = NULL;

	gi.dprintf("\nReading config list..\n");

	fscanf(f, "%c", &strbuf[0]);

	do 
	{
		if (feof(f))
			break;

		if (strbuf[0] == '#')		// commented line
		{
			do { 
				fscanf(f, "%c", &strbuf[0]);
			} while (!feof(f) && (strbuf[0] != '\n') /*&& !feof(f)*/);
		}
		else if (strbuf[0] == '\n' || strbuf[0] == ' ' || strbuf[0] == '\t')		// blank line
		{
			do {
				fscanf(f, "%c", &strbuf[0]);
			} while (!feof(f) && (strbuf[0] == '\n' || strbuf[0] == ' ' || strbuf[0] == '\t'));
		}
		else // start of some data
		{
			i=0;
			do 
			{
				i++;
				fscanf(f, "%c", &strbuf[i]);
			} while ((strbuf[i] != ' ') && (strbuf[i] != '\t') && (strbuf[i] != '\n') && (i < 255) && !feof(f));
			
			if (strbuf[i]!='\n')
				newline=1;
			else newline=0;
			strbuf[i]='\0';

			add_config(strbuf);

			if (!newline)
				do {	
					fscanf(f, "%c", &strbuf[0]);
				} while (((strbuf[0] == ' ') || (strbuf[i] == '\t')) && !feof(f));
			
			configs++;
		}
		
	} while (!feof(f));

	sprintf(strbuf, "Read %d configs\n", configs);
	gi.dprintf(strbuf);

	fclose (f);

	this_config=config_list;
}

void setup_config(void)
{
	char execstring[128];

	if (!use_configlist->value)
		return;

	if (this_config && this_config->filename)
	{
		sprintf(execstring, "exec %s\n", this_config->filename);
		gi.AddCommandString(execstring);
	}

	if (this_config && this_config->next)
		this_config=this_config->next;
	else
		this_config=config_list;

}