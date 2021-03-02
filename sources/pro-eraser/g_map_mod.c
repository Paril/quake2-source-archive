//
// Map Mod 
// by Jeremy Mappus - Jerm a.k.a DarkTheties
//
// This mod will load a file called maps.txt in the baseq2 directory. It will take any names
// seperated by white spaces and list them as seperate name entries into the map_mod_names_ 
// table. It also provides a function for checking the change level. 
//

#include "g_local.h"
#include "g_map_mod.h"
#include <stdio.h>

#define	MAPMOD_MAXLEVELS	256

int  map_mod_ = 0;
int  map_mod_current_level_ = -1;
int  map_mod_n_levels_ = 0;
char map_mod_names_[MAPMOD_MAXLEVELS][64];

cvar_t *mapmod_random;

//
//
void map_mod_set_up()
{
	FILE *file;
	char file_name[256];
	cvar_t *game_dir, *basedir;

	mapmod_random = gi.cvar ("mapmod_random", "0", CVAR_ARCHIVE);
	
	game_dir = gi.cvar ("game", "", 0);
	basedir = gi.cvar("basedir", ".", 0);

#if defined(linux)
	sprintf(file_name, "%s/%s/maps.txt", basedir->string, game_dir->string);
#else
	sprintf(file_name, "%s\\%s\\maps.txt", basedir->string, game_dir->string);
#endif

	file = fopen(file_name, "r");
	map_mod_ = 0;
	map_mod_current_level_ = -1;
	map_mod_n_levels_ = 0;
	if (file != NULL)
	{
		long file_size;
		char *p_buffer;
		char *p_name;
		long counter = 0;
		int n_chars = 0;

		file_size = 0;
		while (!feof(file))
		{
		  fgetc(file);
		  file_size++;
		}
		rewind(file);

		p_buffer = malloc(file_size);
		memset(p_buffer,0,file_size);

		fread((void *)p_buffer, sizeof(char), file_size, file);

		gi.dprintf ("\n==== Map Mod v1.01 set up ====\n");
		gi.dprintf("Adding maps to cycle: ");

		p_name = p_buffer;
		do
		{
			while ((((*p_name >= 'a') && (*p_name <= 'z')) || ((*p_name >= 'A') && (*p_name <= 'Z')) || ((*p_name >= '0') && (*p_name <= '9')) || (*p_name == '_') || (*p_name == '-') || (*p_name == '/') || (*p_name == '\\')) && counter < file_size)
			{
				n_chars++;
				counter++;
				p_name++;
			}
			if (n_chars)
			{
				memcpy(&map_mod_names_[map_mod_n_levels_][0], p_name - n_chars, n_chars);
				memset(&map_mod_names_[map_mod_n_levels_][n_chars], 0, 1);

				if (map_mod_n_levels_ > 0)
					gi.dprintf(", ");
				gi.dprintf("%s", map_mod_names_[map_mod_n_levels_]);

				map_mod_n_levels_++;
				n_chars = 0;

				if (map_mod_n_levels_ >= MAPMOD_MAXLEVELS)
				{
					gi.dprintf("\nMAPMOD_MAXLEVELS exceeded\nUnable to add more levels.\n");
					break;
				}
			}

			// next mapname
			counter++;
			p_name++;

			// eat up non-characters
			while (!(((*p_name >= 'a') && (*p_name <= 'z')) || ((*p_name >= 'A') && (*p_name <= 'Z')) || ((*p_name >= '0') && (*p_name <= '9')) || (*p_name == '_') || (*p_name == '-') || (*p_name == '/') || (*p_name == '\\')) && counter < file_size)
			{
				counter++;
				p_name++;
			}

		} while (counter < file_size);

		gi.dprintf("\n\n");

		free(p_buffer);
		fclose(file);

		if (map_mod_n_levels_)
		{
			map_mod_ = true;
		}
	}
	else
	{
		gi.dprintf ("==== Map Mod v1.01 - missing maps.txt file ====\n");
	}
}


//
//
char* map_mod_next_map()
{
	int i;

	if (map_mod_)
	{
		map_mod_current_level_ = -1;

		if (mapmod_random->value)
		{
			i = (int) floor(random() * ((float)(map_mod_n_levels_)));

			if (!Q_stricmp(level.mapname, map_mod_names_[i]))
			{
				if (++i >= map_mod_n_levels_)
					i=0;
			}

			map_mod_current_level_ = i;
		}
		else
		{
			for (i=0; i < map_mod_n_levels_; i++)
				if (!Q_stricmp(level.mapname, map_mod_names_[i]))
					map_mod_current_level_ = i+1;
		}

		if (map_mod_current_level_ >= map_mod_n_levels_)
		{
			map_mod_current_level_ = 0;
		}

		if (map_mod_current_level_ > -1)
		{
			return map_mod_names_[map_mod_current_level_];
		}

	}

	return NULL;
}

