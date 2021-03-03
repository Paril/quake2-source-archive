/* IP banning */

#include "g_local.h"
#include "aj_banning.h"

banlist_node *ban_list=NULL;


void clear_banlist(void)
{
	banlist_node *current=ban_list, *next_node;

	while (current)
	{
		next_node = current->next;

		if (current->ip)
			free(current->ip);
		free(current);
		current=next_node;
	}

	ban_list=NULL;
}


void add_ip(char *config)
{
	banlist_node *current, *new_node;

	for (current=ban_list; (current && current->next); current=current->next)
		;

	if (!current)
	{
		current=(banlist_node *) gi.TagMalloc (sizeof(banlist_node), TAG_GAME);
//		current=(banlist_node *) malloc (sizeof(replace_details));
		if (!current)
			return; // should signal an error, but wtf...

		current->next=NULL;
		ban_list = current;
		new_node=current;
	}
	else
	{
		new_node=(banlist_node *) gi.TagMalloc (sizeof(banlist_node), TAG_GAME);
//		new_node=(banlist_node *) malloc (sizeof(banlist_node));
		if (!new_node)
			return; // should signal an error, but wtf...

		new_node->next=NULL;

		current->next = new_node;
	}

	new_node->ip=(char *) gi.TagMalloc (sizeof(char)*(strlen(config)+1), TAG_GAME);
	if (!new_node->ip)
		return;
	strcpy(new_node->ip, config);
}


void read_banlist(void)
{
	FILE	*f;
	int		i, ips=0;
	char	filename[256];
	cvar_t	*game_dir;
	char	strbuf[256];
	int		newline;

	game_dir = gi.cvar ("game", "", 0);

#ifdef	_WIN32
	i =  sprintf(filename, ".\\");
	i += sprintf(filename + i, game_dir->string);
	i += sprintf(filename + i, "\\");
	i += sprintf(filename + i, banlist->string);
#else
      strcpy(filename, "./");
      strcat(filename, game_dir->string);
      strcat(filename, "/");
	  strcat(filename, banlist->string);
#endif

	f = fopen (filename, "r");
	if (!f)
	{
//		gi.error("Unable to read the replace file.\n");
		return;
	}

	clear_banlist();

	gi.dprintf("\nReading banlist..\n");

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
		else if (strbuf[0] == '\n' || strbuf[0] == ' ' || strbuf[0] == '\t' || feof(f))		// blank line
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

			add_ip(strbuf);

			if (!newline)
				do {	
					fscanf(f, "%c", &strbuf[0]);
				} while (((strbuf[0] == ' ') || (strbuf[i] == '\t')) && !feof(f));
			
			ips++;
		}
		
	} while (!feof(f));

	sprintf(strbuf, "Read %d banned IP's\n", ips);
	gi.dprintf(strbuf);

	fclose (f);
}

// returns 0 if not in the list, 1 if they are banned
int check_ip(char *player_ip)
{
	int allowed=0, colonchar;
	banlist_node *current;
	char	strbuf[256];

	for (colonchar=0; (player_ip[colonchar] != ':' && player_ip[colonchar] != '\0'); colonchar++)
		;

	if (use_iplogging->value)
	{
		sprintf(strbuf, "IP=%s\n", player_ip);
		gi.dprintf(strbuf);
	}

	if (player_ip[colonchar]==':')
		player_ip[colonchar]='\0';
	else colonchar=0;

	for (current=ban_list; current; current=current->next)
		if (strcmp(current->ip, player_ip)==0)
			allowed=1;

	if (colonchar)
		player_ip[colonchar]=':';

	return allowed;
}
