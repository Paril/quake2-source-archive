// Copyright 1999 by Christopher Gabriel Sisk, aka RipVTide

#include "g_local.h"

puter_data dis_puta;
double puter_safety;

void Computer_Open (edict_t *ent, edict_t *activator)
{
	char filename[16];

	dis_puta.bitwise[6] = 0;
	dis_puta.bitwise[7] = 0;
	dis_puta.bitwise[8] = 0;
	dis_puta.bitwise[9] = 0;

	gi.AddCommandString("toggleconsole\n");
	safe_cprintf(activator, PRINT_HIGH, "\n");
	safe_cprintf(activator, PRINT_HIGH, "\n");
	safe_cprintf(activator, PRINT_HIGH, "\n");
	safe_cprintf(activator, PRINT_HIGH, "System Online\n");
	safe_cprintf(activator, PRINT_HIGH, "\n");

	sprintf (filename, "%s.cds", ent->message);

	puter_safety = level.time;

	Computer_Load_CDS(filename, activator);	
}


void Computer_Load_CDS (char *filename, edict_t *activator)
{
	FILE		*current_cds;
	char		txtstring[64];
	int		counter;
	cvar_t		*game_dir, *basedir;
	char		file_name[256];
	char		game_dir_name[80];
	int		strlngth;
	char		x_pass[24];

	game_dir = gi.cvar ("game", "", 0);
	basedir = gi.cvar ("basedir", ".", 0);

	if (!Q_stricmp (game_dir->string, ""))
		sprintf (game_dir_name, "baseq2");
	else 
		sprintf (game_dir_name, "%s", game_dir->string);

	sprintf (file_name, "%s\\%s\\cds\\%s", basedir->string, game_dir_name, filename);

	current_cds = fopen(file_name, "r");

	if (!current_cds)
	{
		gi.dprintf ("Error Opening %s 001\n", filename);
		return;
	}

//
//
// Password
//
//

	fgets(txtstring, 7, current_cds);

	if(Q_strcasecmp(txtstring, "_pass ") == 0)
	{
		fgets(txtstring, 24, current_cds);
		strlngth = strlen(txtstring)-1;
		strcpy(x_pass, "");
		strncat(x_pass, txtstring, strlngth);

		if(Q_strcasecmp(x_pass, dis_puta.pass))
		{
			safe_cprintf(activator, PRINT_HIGH, "Incorrect Password\n");
			return;
		}
	}
	else
	{
		fgets(txtstring, 24, current_cds);
	}

	fgets(txtstring, 24, current_cds);

	if(Q_stricmp(txtstring, "_message_") == 0)
	{
		gi.dprintf ("Error Reading File 001\n");
		return;
	}

	fgets(txtstring, 64, current_cds);
	strcpy(dis_puta.message, txtstring);
	safe_cprintf(activator, PRINT_HIGH, dis_puta.message);
	safe_cprintf(activator, PRINT_HIGH, "\n");


//
//
//			Directory Loading Operations
//
//

	fgets(txtstring, 24, current_cds);
	if(Q_stricmp(txtstring, "_dirs_") == 0)
		{
		safe_cprintf(activator, PRINT_HIGH,"Error Reading File 002\n");
		return;
		}


	fgets(txtstring, 4, current_cds);
	dis_puta.dir_n = atoi(txtstring);

	safe_cprintf(activator, PRINT_HIGH,"Directories\n");
	counter = dis_puta.dir_n;
	if(dis_puta.dir_n == 0)
	{
			strcpy(dis_puta.dirs.target1, "");
			strcpy(dis_puta.dirs.name1, "");
			strcpy(dis_puta.dirs.target2, "");
			strcpy(dis_puta.dirs.name2, "");
			strcpy(dis_puta.dirs.target3, "");
			strcpy(dis_puta.dirs.name3, "");
			strcpy(dis_puta.dirs.target4, "");
			strcpy(dis_puta.dirs.name4, "");
			strcpy(dis_puta.dirs.target5, "");
			strcpy(dis_puta.dirs.name5, "");
			strcpy(dis_puta.dirs.target6, "");
			strcpy(dis_puta.dirs.name6, "");
			strcpy(dis_puta.dirs.target7, "");
			strcpy(dis_puta.dirs.name7, "");
			strcpy(dis_puta.dirs.target8, "");
			strcpy(dis_puta.dirs.name8, "");
			strcpy(dis_puta.dirs.target9, "");
			strcpy(dis_puta.dirs.name9, "");
	}
	if((dis_puta.dir_n >> 0) && (dis_puta.dir_n << 10))
	{
		while(1)
		{
			//	Dir_1_Read
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
			{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
			}
			strcpy(dis_puta.dirs.name1, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.dirs.name1);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.name1, "");
			strncat(dis_puta.dirs.name1, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
			{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
			}
			strcpy(dis_puta.dirs.target1, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.target1, "");
			strncat(dis_puta.dirs.target1, txtstring, strlngth);
			counter--;
			if(counter == 0)
			{
				strcpy(dis_puta.dirs.target2, "");
				strcpy(dis_puta.dirs.name2, "");
				strcpy(dis_puta.dirs.target3, "");
				strcpy(dis_puta.dirs.name3, "");
				strcpy(dis_puta.dirs.target4, "");
				strcpy(dis_puta.dirs.name4, "");
				strcpy(dis_puta.dirs.target5, "");
				strcpy(dis_puta.dirs.name5, "");
				strcpy(dis_puta.dirs.target6, "");
				strcpy(dis_puta.dirs.name6, "");
				strcpy(dis_puta.dirs.target7, "");
				strcpy(dis_puta.dirs.name7, "");
				strcpy(dis_puta.dirs.target8, "");
				strcpy(dis_puta.dirs.name8, "");
				strcpy(dis_puta.dirs.target9, "");
				strcpy(dis_puta.dirs.name9, "");
				break;
			}

			//	Dir_2_Read
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.name2, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.dirs.name2);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.name2, "");
			strncat(dis_puta.dirs.name2, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.target2, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.target2, "");
			strncat(dis_puta.dirs.target2, txtstring, strlngth);
			counter--;
			if(counter == 0)
			{
				strcpy(dis_puta.dirs.target3, "");
				strcpy(dis_puta.dirs.name3, "");
				strcpy(dis_puta.dirs.target4, "");
				strcpy(dis_puta.dirs.name4, "");
				strcpy(dis_puta.dirs.target5, "");
				strcpy(dis_puta.dirs.name5, "");
				strcpy(dis_puta.dirs.target6, "");
				strcpy(dis_puta.dirs.name6, "");
				strcpy(dis_puta.dirs.target7, "");
				strcpy(dis_puta.dirs.name7, "");
				strcpy(dis_puta.dirs.target8, "");
				strcpy(dis_puta.dirs.name8, "");
				strcpy(dis_puta.dirs.target9, "");
				strcpy(dis_puta.dirs.name9, "");
				break;
			}

			//	Dir_3_Read
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.name3, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.dirs.name3);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.name3, "");
			strncat(dis_puta.dirs.name3, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.target3, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.target3, "");
			strncat(dis_puta.dirs.target3, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.dirs.target4, "");
				strcpy(dis_puta.dirs.name4, "");
				strcpy(dis_puta.dirs.target5, "");
				strcpy(dis_puta.dirs.name5, "");
				strcpy(dis_puta.dirs.target6, "");
				strcpy(dis_puta.dirs.name6, "");
				strcpy(dis_puta.dirs.target7, "");
				strcpy(dis_puta.dirs.name7, "");
				strcpy(dis_puta.dirs.target8, "");
				strcpy(dis_puta.dirs.name8, "");
				strcpy(dis_puta.dirs.target9, "");
				strcpy(dis_puta.dirs.name9, "");
				break;
				}

			//	Dir_4_Read
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.name4, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.dirs.name4);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.name4, "");
			strncat(dis_puta.dirs.name4, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.target4, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.target4, "");
			strncat(dis_puta.dirs.target4, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.dirs.target5, "");
				strcpy(dis_puta.dirs.name5, "");
				strcpy(dis_puta.dirs.target6, "");
				strcpy(dis_puta.dirs.name6, "");
				strcpy(dis_puta.dirs.target7, "");
				strcpy(dis_puta.dirs.name7, "");
				strcpy(dis_puta.dirs.target8, "");
				strcpy(dis_puta.dirs.name8, "");
				strcpy(dis_puta.dirs.target9, "");
				strcpy(dis_puta.dirs.name9, "");
				break;
				}

			//	Dir_5_Read
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.name5, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.dirs.name5);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.name5, "");
			strncat(dis_puta.dirs.name5, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.target5, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.target5, "");
			strncat(dis_puta.dirs.target5, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.dirs.target6, "");
				strcpy(dis_puta.dirs.name6, "");
				strcpy(dis_puta.dirs.target7, "");
				strcpy(dis_puta.dirs.name7, "");
				strcpy(dis_puta.dirs.target8, "");
				strcpy(dis_puta.dirs.name8, "");
				strcpy(dis_puta.dirs.target9, "");
				strcpy(dis_puta.dirs.name9, "");
				break;
				}

			//	Dir_6_Read
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.name6, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.dirs.name6);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.name6, "");
			strncat(dis_puta.dirs.name6, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.target6, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.target6, "");
			strncat(dis_puta.dirs.target6, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.dirs.target7, "");
				strcpy(dis_puta.dirs.name7, "");
				strcpy(dis_puta.dirs.target8, "");
				strcpy(dis_puta.dirs.name8, "");
				strcpy(dis_puta.dirs.target9, "");
				strcpy(dis_puta.dirs.name9, "");
				break;
				}

			//	Dir_7_Read
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.name7, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.dirs.name7);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.name7, "");
			strncat(dis_puta.dirs.name7, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.target7, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.target7, "");
			strncat(dis_puta.dirs.target7, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.dirs.target8, "");
				strcpy(dis_puta.dirs.name8, "");
				strcpy(dis_puta.dirs.target9, "");
				strcpy(dis_puta.dirs.name9, "");
				break;
				}

			//	Dir_8_Read
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.name8, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.dirs.name8);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.name8, "");
			strncat(dis_puta.dirs.name8, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.target8, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.target8, "");
			strncat(dis_puta.dirs.target8, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.dirs.target9, "");
				strcpy(dis_puta.dirs.name9, "");
				break;
				}

			//	Dir_9_Read
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.name9, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.dirs.name9);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.name9, "");
			strncat(dis_puta.dirs.name9, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_execs_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.dirs.target9, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.dirs.target9, "");
			strncat(dis_puta.dirs.target9, txtstring, strlngth);
			break;
			}
		}
//
//
//			Exec Loading Operations
//
//




	fgets(txtstring, 24, current_cds);
	if(Q_stricmp(txtstring, "_execs_") == 0)
		{
		safe_cprintf(activator, PRINT_HIGH,"Error Reading File 002\n");
		return;
		}

	fgets(txtstring, 4, current_cds);
	dis_puta.exec_n = atoi(txtstring);

	safe_cprintf(activator, PRINT_HIGH,"Executables\n");
	counter = dis_puta.exec_n;
	if(dis_puta.exec_n == 0)
		{
			strcpy(dis_puta.execs.target1, "");
			strcpy(dis_puta.execs.name1, "");
			strcpy(dis_puta.execs.target2, "");
			strcpy(dis_puta.execs.name2, "");
			strcpy(dis_puta.execs.target3, "");
			strcpy(dis_puta.execs.name3, "");
			strcpy(dis_puta.execs.target4, "");
			strcpy(dis_puta.execs.name4, "");
			strcpy(dis_puta.execs.target5, "");
			strcpy(dis_puta.execs.name5, "");
			strcpy(dis_puta.execs.target6, "");
			strcpy(dis_puta.execs.name6, "");
			strcpy(dis_puta.execs.target7, "");
			strcpy(dis_puta.execs.name7, "");
			strcpy(dis_puta.execs.target8, "");
			strcpy(dis_puta.execs.name8, "");
			strcpy(dis_puta.execs.target9, "");
			strcpy(dis_puta.execs.name9, "");
		}
	if((dis_puta.exec_n >> 0) && (dis_puta.exec_n << 10))
		{
		while(1)
			{
			//	Exec_1_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.name1, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.execs.name1);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.name1, "");
			strncat(dis_puta.execs.name1, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.target1, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.target1, "");
			strncat(dis_puta.execs.target1, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.execs.target2, "");
				strcpy(dis_puta.execs.name2, "");
				strcpy(dis_puta.execs.target3, "");
				strcpy(dis_puta.execs.name3, "");
				strcpy(dis_puta.execs.target4, "");
				strcpy(dis_puta.execs.name4, "");
				strcpy(dis_puta.execs.target5, "");
				strcpy(dis_puta.execs.name5, "");
				strcpy(dis_puta.execs.target6, "");
				strcpy(dis_puta.execs.name6, "");
				strcpy(dis_puta.execs.target7, "");
				strcpy(dis_puta.execs.name7, "");
				strcpy(dis_puta.execs.target8, "");
				strcpy(dis_puta.execs.name8, "");
				strcpy(dis_puta.execs.target9, "");
				strcpy(dis_puta.execs.name9, "");
				break;
				}

			//	Exec_2_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.name2, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.execs.name2);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.name2, "");
			strncat(dis_puta.execs.name2, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.target2, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.target2, "");
			strncat(dis_puta.execs.target2, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.execs.target3, "");
				strcpy(dis_puta.execs.name3, "");
				strcpy(dis_puta.execs.target4, "");
				strcpy(dis_puta.execs.name4, "");
				strcpy(dis_puta.execs.target5, "");
				strcpy(dis_puta.execs.name5, "");
				strcpy(dis_puta.execs.target6, "");
				strcpy(dis_puta.execs.name6, "");
				strcpy(dis_puta.execs.target7, "");
				strcpy(dis_puta.execs.name7, "");
				strcpy(dis_puta.execs.target8, "");
				strcpy(dis_puta.execs.name8, "");
				strcpy(dis_puta.execs.target9, "");
				strcpy(dis_puta.execs.name9, "");
				break;
				}

			//	Exec_3_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.name3, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.execs.name3);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.name3, "");
			strncat(dis_puta.execs.name3, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.target3, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.target3, "");
			strncat(dis_puta.execs.target3, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.execs.target4, "");
				strcpy(dis_puta.execs.name4, "");
				strcpy(dis_puta.execs.target5, "");
				strcpy(dis_puta.execs.name5, "");
				strcpy(dis_puta.execs.target6, "");
				strcpy(dis_puta.execs.name6, "");
				strcpy(dis_puta.execs.target7, "");
				strcpy(dis_puta.execs.name7, "");
				strcpy(dis_puta.execs.target8, "");
				strcpy(dis_puta.execs.name8, "");
				strcpy(dis_puta.execs.target9, "");
				strcpy(dis_puta.execs.name9, "");
				break;
				}

			//	Exec_4_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.name4, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.execs.name4);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.name4, "");
			strncat(dis_puta.execs.name4, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.target4, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.target4, "");
			strncat(dis_puta.execs.target4, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.execs.target5, "");
				strcpy(dis_puta.execs.name5, "");
				strcpy(dis_puta.execs.target6, "");
				strcpy(dis_puta.execs.name6, "");
				strcpy(dis_puta.execs.target7, "");
				strcpy(dis_puta.execs.name7, "");
				strcpy(dis_puta.execs.target8, "");
				strcpy(dis_puta.execs.name8, "");
				strcpy(dis_puta.execs.target9, "");
				strcpy(dis_puta.execs.name9, "");
				break;
				}

			//	Exec_5_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.name5, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.execs.name5);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.name5, "");
			strncat(dis_puta.execs.name5, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.target5, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.target5, "");
			strncat(dis_puta.execs.target5, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.execs.target6, "");
				strcpy(dis_puta.execs.name6, "");
				strcpy(dis_puta.execs.target7, "");
				strcpy(dis_puta.execs.name7, "");
				strcpy(dis_puta.execs.target8, "");
				strcpy(dis_puta.execs.name8, "");
				strcpy(dis_puta.execs.target9, "");
				strcpy(dis_puta.execs.name9, "");
				break;
				}

			//	Exec_6_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.name6, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.execs.name6);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.name6, "");
			strncat(dis_puta.execs.name6, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.target6, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.target6, "");
			strncat(dis_puta.execs.target6, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.execs.target7, "");
				strcpy(dis_puta.execs.name7, "");
				strcpy(dis_puta.execs.target8, "");
				strcpy(dis_puta.execs.name8, "");
				strcpy(dis_puta.execs.target9, "");
				strcpy(dis_puta.execs.name9, "");
				break;
				}


			//	Exec_7_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.name7, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.execs.name7);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.name7, "");
			strncat(dis_puta.execs.name7, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.target7, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.target7, "");
			strncat(dis_puta.execs.target7, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.execs.target8, "");
				strcpy(dis_puta.execs.name8, "");
				strcpy(dis_puta.execs.target9, "");
				strcpy(dis_puta.execs.name9, "");
				break;
				}

			//	Exec_8_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.name8, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.execs.name8);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.name8, "");
			strncat(dis_puta.execs.name8, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.target8, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.target8, "");
			strncat(dis_puta.execs.target8, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.execs.target9, "");
				strcpy(dis_puta.execs.name9, "");
				break;
				}

			//	Exec_9_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.name9, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.execs.name9);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.name9, "");
			strncat(dis_puta.execs.name9, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_txts_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.execs.target9, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.execs.target9, "");
			strncat(dis_puta.execs.target9, txtstring, strlngth);
			counter--;
			break;
			}
		}



//
//
//			Txt Loading Operations
//
//




	fgets(txtstring, 24, current_cds);
	if(Q_stricmp(txtstring, "_txts_") == 0)
		{
		safe_cprintf(activator, PRINT_HIGH,"Error Reading File 002\n");
		return;
		}

	fgets(txtstring, 4, current_cds);
	dis_puta.txt_n = atoi(txtstring);

	safe_cprintf(activator, PRINT_HIGH,"Text Files\n");
	counter = dis_puta.txt_n;
	if(dis_puta.txt_n == 0)
		{
			strcpy(dis_puta.txts.target1, "");
			strcpy(dis_puta.txts.name1, "");
			strcpy(dis_puta.txts.target2, "");
			strcpy(dis_puta.txts.name2, "");
			strcpy(dis_puta.txts.target3, "");
			strcpy(dis_puta.txts.name3, "");
			strcpy(dis_puta.txts.target4, "");
			strcpy(dis_puta.txts.name4, "");
			strcpy(dis_puta.txts.target5, "");
			strcpy(dis_puta.txts.name5, "");
			strcpy(dis_puta.txts.target6, "");
			strcpy(dis_puta.txts.name6, "");
			strcpy(dis_puta.txts.target7, "");
			strcpy(dis_puta.txts.name7, "");
			strcpy(dis_puta.txts.target8, "");
			strcpy(dis_puta.txts.name8, "");
			strcpy(dis_puta.txts.target9, "");
			strcpy(dis_puta.txts.name9, "");
		}
	if((dis_puta.txt_n >> 0) && (dis_puta.txt_n << 10))
		{
		while(1)
			{
			//	Txt_1_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.name1, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.txts.name1);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.name1, "");
			strncat(dis_puta.txts.name1, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.target1, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.target1, "");
			strncat(dis_puta.txts.target1, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.txts.target2, "");
				strcpy(dis_puta.txts.name2, "");
				strcpy(dis_puta.txts.target3, "");
				strcpy(dis_puta.txts.name3, "");
				strcpy(dis_puta.txts.target4, "");
				strcpy(dis_puta.txts.name4, "");
				strcpy(dis_puta.txts.target5, "");
				strcpy(dis_puta.txts.name5, "");
				strcpy(dis_puta.txts.target6, "");
				strcpy(dis_puta.txts.name6, "");
				strcpy(dis_puta.txts.target7, "");
				strcpy(dis_puta.txts.name7, "");
				strcpy(dis_puta.txts.target8, "");
				strcpy(dis_puta.txts.name8, "");
				strcpy(dis_puta.txts.target9, "");
				strcpy(dis_puta.txts.name9, "");
				break;
				}

			//	Txt_2_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.name2, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.txts.name2);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.name2, "");
			strncat(dis_puta.txts.name2, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.target2, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.target2, "");
			strncat(dis_puta.txts.target2, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.txts.target3, "");
				strcpy(dis_puta.txts.name3, "");
				strcpy(dis_puta.txts.target4, "");
				strcpy(dis_puta.txts.name4, "");
				strcpy(dis_puta.txts.target5, "");
				strcpy(dis_puta.txts.name5, "");
				strcpy(dis_puta.txts.target6, "");
				strcpy(dis_puta.txts.name6, "");
				strcpy(dis_puta.txts.target7, "");
				strcpy(dis_puta.txts.name7, "");
				strcpy(dis_puta.txts.target8, "");
				strcpy(dis_puta.txts.name8, "");
				strcpy(dis_puta.txts.target9, "");
				strcpy(dis_puta.txts.name9, "");
				break;
				}

			//	Txt_3_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.name3, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.txts.name3);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.name3, "");
			strncat(dis_puta.txts.name3, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.target3, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.target3, "");
			strncat(dis_puta.txts.target3, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.txts.target4, "");
				strcpy(dis_puta.txts.name4, "");
				strcpy(dis_puta.txts.target5, "");
				strcpy(dis_puta.txts.name5, "");
				strcpy(dis_puta.txts.target6, "");
				strcpy(dis_puta.txts.name6, "");
				strcpy(dis_puta.txts.target7, "");
				strcpy(dis_puta.txts.name7, "");
				strcpy(dis_puta.txts.target8, "");
				strcpy(dis_puta.txts.name8, "");
				strcpy(dis_puta.txts.target9, "");
				strcpy(dis_puta.txts.name9, "");
				break;
				}

			//	Txt_4_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.name4, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.txts.name4);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.name4, "");
			strncat(dis_puta.txts.name4, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.target4, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.target4, "");
			strncat(dis_puta.txts.target4, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.txts.target5, "");
				strcpy(dis_puta.txts.name5, "");
				strcpy(dis_puta.txts.target6, "");
				strcpy(dis_puta.txts.name6, "");
				strcpy(dis_puta.txts.target7, "");
				strcpy(dis_puta.txts.name7, "");
				strcpy(dis_puta.txts.target8, "");
				strcpy(dis_puta.txts.name8, "");
				strcpy(dis_puta.txts.target9, "");
				strcpy(dis_puta.txts.name9, "");
				break;
				}

			//	Txt_5_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.name5, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.txts.name5);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.name5, "");
			strncat(dis_puta.txts.name5, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.target5, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.target5, "");
			strncat(dis_puta.txts.target5, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.txts.target6, "");
				strcpy(dis_puta.txts.name6, "");
				strcpy(dis_puta.txts.target7, "");
				strcpy(dis_puta.txts.name7, "");
				strcpy(dis_puta.txts.target8, "");
				strcpy(dis_puta.txts.name8, "");
				strcpy(dis_puta.txts.target9, "");
				strcpy(dis_puta.txts.name9, "");
				break;
				}

			//	Txt_6_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.name6, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.txts.name6);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.name6, "");
			strncat(dis_puta.txts.name6, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.target6, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.target6, "");
			strncat(dis_puta.txts.target6, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.txts.target7, "");
				strcpy(dis_puta.txts.name7, "");
				strcpy(dis_puta.txts.target8, "");
				strcpy(dis_puta.txts.name8, "");
				strcpy(dis_puta.txts.target9, "");
				strcpy(dis_puta.txts.name9, "");
				break;
				}

			//	Txt_7_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.name7, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.txts.name7);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.name7, "");
			strncat(dis_puta.txts.name7, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.target7, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.target7, "");
			strncat(dis_puta.txts.target7, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.txts.target8, "");
				strcpy(dis_puta.txts.name8, "");
				strcpy(dis_puta.txts.target9, "");
				strcpy(dis_puta.txts.name9, "");
				break;
				}

			//	Txt_8_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.name8, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.txts.name8);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.name8, "");
			strncat(dis_puta.txts.name8, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.target8, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.target8, "");
			strncat(dis_puta.txts.target8, txtstring, strlngth);
			counter--;
			if(counter == 0)
				{
				strcpy(dis_puta.txts.target9, "");
				strcpy(dis_puta.txts.name9, "");
				break;
				}

			//	Txt_9_Read




			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.name9, txtstring);
			safe_cprintf(activator, PRINT_HIGH,"  ");
			safe_cprintf(activator, PRINT_HIGH,dis_puta.txts.name9);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.name9, "");
			strncat(dis_puta.txts.name9, txtstring, strlngth);
			fgets(txtstring, 24, current_cds);
			if(txtstring == "_topdir_")
				{
				safe_cprintf(activator, PRINT_HIGH,"Error Reading File 003\n");
				return;
				}
			strcpy(dis_puta.txts.target9, txtstring);
			strlngth = strlen(txtstring)-1;
			strcpy(dis_puta.txts.target9, "");
			strncat(dis_puta.txts.target9, txtstring, strlngth);
			counter--;
			break;
			}
		}

	fgets(txtstring, 24, current_cds);
	if(Q_stricmp(txtstring, "_topdir_") == 0)
		{
		safe_cprintf(activator, PRINT_HIGH,"Error Reading File 002\n");
		return;
		}

	fgets(txtstring, 24, current_cds);
	strcpy(dis_puta.topdir, txtstring);

	if(fclose(current_cds) == EOF)
		safe_cprintf(activator, PRINT_HIGH, "Error Closing file 001\n");
}


void Computer_Change_Dir (char *cds, edict_t *ent)
{
	char		targetcds[16];

	if(puter_safety != level.time)
		{
		safe_cprintf(ent, PRINT_HIGH, "Not at Computer Console\n");
		return;
		}

	safe_cprintf(ent, PRINT_HIGH, "\n\n");

	if(Q_strcasecmp(cds, dis_puta.dirs.name1) == 0)
	strcpy(targetcds, dis_puta.dirs.target1);

	if(Q_strcasecmp(cds, dis_puta.dirs.name2) == 0)
	strcpy(targetcds, dis_puta.dirs.target2);

	if(Q_strcasecmp(cds, dis_puta.dirs.name3) == 0)
	strcpy(targetcds, dis_puta.dirs.target3);

	if(Q_strcasecmp(cds, dis_puta.dirs.name4) == 0)
	strcpy(targetcds, dis_puta.dirs.target4);

	if(Q_strcasecmp(cds, dis_puta.dirs.name5) == 0)
	strcpy(targetcds, dis_puta.dirs.target5);

	if(Q_strcasecmp(cds, dis_puta.dirs.name6) == 0)
	strcpy(targetcds, dis_puta.dirs.target6);

	if(Q_strcasecmp(cds, dis_puta.dirs.name7) == 0)
	strcpy(targetcds, dis_puta.dirs.target7);

	if(Q_strcasecmp(cds, dis_puta.dirs.name8) == 0)
	strcpy(targetcds, dis_puta.dirs.target8);

	if(Q_strcasecmp(cds, dis_puta.dirs.name9) == 0)
	strcpy(targetcds, dis_puta.dirs.target9);

	if(Q_strcasecmp(cds, "..") == 0)
		{
		if(Q_stricmp(dis_puta.topdir, "NULL") == 0)
			{
			safe_cprintf(ent, PRINT_HIGH,"Already at Top Directory\n");
			return;
			}
		else
			{
			strcpy(targetcds, dis_puta.topdir);
			Computer_Load_CDS(targetcds, ent);
			return;
			}
		}

	Computer_Load_CDS(targetcds, ent);
}


void Computer_Cat (char *txt, edict_t *ent)
{
	char		targettxt[16];



	if(puter_safety != level.time)
		{
		safe_cprintf(ent, PRINT_HIGH, "Not at Computer Console\n");
		return;
		}

	safe_cprintf(ent, PRINT_HIGH, "\n\n");

	if(Q_strcasecmp(txt, dis_puta.txts.name1) == 0)
	strcpy(targettxt, dis_puta.txts.target1);

	if(Q_strcasecmp(txt, dis_puta.txts.name2) == 0)
	strcpy(targettxt, dis_puta.txts.target2);

	if(Q_strcasecmp(txt, dis_puta.txts.name3) == 0)
	strcpy(targettxt, dis_puta.txts.target3);

	if(Q_strcasecmp(txt, dis_puta.txts.name4) == 0)
	strcpy(targettxt, dis_puta.txts.target4);

	if(Q_strcasecmp(txt, dis_puta.txts.name5) == 0)
	strcpy(targettxt, dis_puta.txts.target5);

	if(Q_strcasecmp(txt, dis_puta.txts.name6) == 0)
	strcpy(targettxt, dis_puta.txts.target6);

	if(Q_strcasecmp(txt, dis_puta.txts.name7) == 0)
	strcpy(targettxt, dis_puta.txts.target7);

	if(Q_strcasecmp(txt, dis_puta.txts.name8) == 0)
	strcpy(targettxt, dis_puta.txts.target8);

	if(Q_strcasecmp(txt, dis_puta.txts.name9) == 0)
	strcpy(targettxt, dis_puta.txts.target9);

	if(Q_strcasecmp(txt, "..") == 0)
		{
		if(Q_stricmp(dis_puta.topdir, "NULL") == 0)
			{
			safe_cprintf(ent, PRINT_HIGH,"Already at Top Directory\n");
			return;
			}
		else
			{
			strcpy(targettxt, dis_puta.topdir);
			Computer_Load_CDS(targettxt, ent);
			return;
			}
		}

	Computer_Printout(targettxt, ent);

}



void Computer_Printout (char *targettxt, edict_t *ent)
{
	FILE		*current_txt;

	char		txtstring[72];

	cvar_t		*game_dir, *basedir;

	char		file_name[256];

	char		game_dir_name[80];

	int		strlngth;

	char		x_pass[24];


	if(puter_safety != level.time)
		{
		safe_cprintf(ent, PRINT_HIGH, "Not at Computer Console\n");
		return;
		}

	game_dir = gi.cvar ("game", "", 0);
	basedir = gi.cvar ("basedir", ".", 0);

	if (!Q_stricmp (game_dir->string, ""))
		sprintf (game_dir_name, "baseq2");
	else 
		sprintf (game_dir_name, "%s", game_dir->string);

	sprintf (file_name, "%s\\%s\\cds\\%s", basedir->string, game_dir_name, targettxt);

	current_txt = fopen(file_name, "r");

	if (!current_txt)
		{
		gi.dprintf ("Error Opening %s 001\n", targettxt);
		return;
		}

//
//
//	Check for Password Protection
//
//

	fgets(txtstring, 7, current_txt);

	if(Q_strcasecmp(txtstring, "_pass ") == 0)
		{
		fgets(txtstring, 24, current_txt);
		strlngth = strlen(txtstring)-1;
		strcpy(x_pass, "");
		strncat(x_pass, txtstring, strlngth);

		if(Q_strcasecmp(x_pass, dis_puta.pass))
			{
			safe_cprintf(ent, PRINT_HIGH, "Incorrect Password\n");
			return;
			}
		}

		else
			fgets(txtstring, 24, current_txt);


//
//
//	Text Output
//
//



	while(fgets(txtstring, 72, current_txt))
		{
		safe_cprintf(ent, PRINT_HIGH, txtstring);
		}

	fclose(current_txt);
	safe_cprintf(ent, PRINT_HIGH, "\n");
}


void Computer_Pass (char *pass, edict_t *ent)
{
	if(puter_safety != level.time)
		{
		safe_cprintf(ent, PRINT_HIGH, "Not at Computer Console\n");
		return;
		}

	strcpy(dis_puta.pass, pass);

	safe_cprintf(ent, PRINT_HIGH, "\nNew Password: ");

	safe_cprintf(ent, PRINT_HIGH, dis_puta.pass);

	safe_cprintf(ent, PRINT_HIGH, "\n\n");
}

void Computer_Dir(edict_t *ent)
{
	if(puter_safety != level.time)
		{
		safe_cprintf(ent, PRINT_HIGH, "Not at Computer Console\n");
		return;
		}

	safe_cprintf(ent, PRINT_HIGH, dis_puta.message);

	safe_cprintf(ent, PRINT_HIGH, "\nDirectories\n");

	if(Q_stricmp(dis_puta.dirs.name1, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.dirs.name1);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.dirs.name2, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.dirs.name2);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.dirs.name3, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.dirs.name3);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.dirs.name4, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.dirs.name4);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.dirs.name5, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.dirs.name5);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.dirs.name6, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.dirs.name6);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.dirs.name7, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.dirs.name7);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.dirs.name8, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.dirs.name8);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.dirs.name9, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.dirs.name9);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	safe_cprintf(ent, PRINT_HIGH, "Executables\n");


	if(Q_stricmp(dis_puta.execs.name1, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.execs.name1);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.execs.name2, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.execs.name2);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.execs.name3, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.execs.name3);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.execs.name4, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.execs.name4);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.execs.name5, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.execs.name5);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.execs.name6, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.execs.name6);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.execs.name7, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.execs.name7);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.execs.name8, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.execs.name8);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.execs.name9, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.execs.name9);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	safe_cprintf(ent, PRINT_HIGH, "Text Files\n");


	if(Q_stricmp(dis_puta.txts.name1, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.txts.name1);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.txts.name2, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.txts.name2);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.txts.name3, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.txts.name3);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.txts.name4, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.txts.name4);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.txts.name5, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.txts.name5);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.txts.name6, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.txts.name6);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.txts.name7, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.txts.name7);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.txts.name8, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.txts.name8);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}

	if(Q_stricmp(dis_puta.txts.name9, ""))
		{
		safe_cprintf(ent, PRINT_HIGH, "  ");
		safe_cprintf(ent, PRINT_HIGH, dis_puta.txts.name9);
		safe_cprintf(ent, PRINT_HIGH, "\n");
		}
}


void Computer_Run(char *file, edict_t *ent)
{
	FILE		*current_exec;

	char		txtstring[72];
	char		txtstring2[72];
	char		txtstring2b[72];

	cvar_t		*game_dir, *basedir;

	char		file_name[256];

	char		game_dir_name[80];

	int		strlngth;

	char		x_pass[24];

	float		targettime;

	edict_t		*t;

	targettime = 0;

	if(puter_safety != level.time)
		{
		safe_cprintf(ent, PRINT_HIGH, "Not at Computer Console\n");
		return;
		}

	game_dir = gi.cvar ("game", "", 0);
	basedir = gi.cvar ("basedir", ".", 0);

	if (!Q_stricmp (game_dir->string, ""))
		sprintf (game_dir_name, "baseq2");
	else 
		sprintf (game_dir_name, "%s", game_dir->string);

	sprintf (file_name, "%s\\%s\\cds\\%s", basedir->string, game_dir_name, file);

	current_exec = fopen(file_name, "r");

	if (!current_exec)
		{
		gi.dprintf ("Error Opening %s 001\n", file);
		return;
		}

//
//
//	Check for Password Protection
//
//

	fgets(txtstring, 7, current_exec);

	if(Q_strcasecmp(txtstring, "_pass ") == 0)
		{
		fgets(txtstring, 24, current_exec);
		strlngth = strlen(txtstring)-1;
		strcpy(x_pass, "");
		strncat(x_pass, txtstring, strlngth);
		if(Q_strcasecmp(x_pass, dis_puta.pass))
			{
			safe_cprintf(ent, PRINT_HIGH, "Incorrect Password\n");
			return;
			}
		}

		else
			fgets(txtstring, 24, current_exec);


//
//
//	Exec Handling (Computer Command Structure)
//
//



	while(fgets(txtstring, 5, current_exec))
		{
		if(Q_strcasecmp(txtstring, "_txt") == 0)
			{
			fgets(txtstring2, 72, current_exec);
			safe_cprintf(ent, PRINT_HIGH, txtstring2);
			}

		else if(Q_strcasecmp(txtstring, "_tgt") == 0)
			{
			targettime = targettime+0.01;
			fgets(txtstring2b, 2, current_exec);
			fgets(txtstring2b, 72, current_exec);
			strlngth = strlen(txtstring2b)-1;
			strcpy(txtstring2, "");
			strncat(txtstring2, txtstring2b, strlngth);
			t = G_Spawn();
			t->nextthink = targettime;
			t->think = Computer_Trigger;
			t->activator = ent;
			if(Q_strcasecmp(txtstring2, "putertarget1") == 0)
				t->target = "putertarget1";
			else if(Q_strcasecmp(txtstring2, "putertarget2") == 0)
				t->target = "putertarget2";
			else if(Q_strcasecmp(txtstring2, "putertarget3") == 0)
				t->target = "putertarget3";
			else if(Q_strcasecmp(txtstring2, "putertarget4") == 0)
				t->target = "putertarget4";
			else if(Q_strcasecmp(txtstring2, "putertarget5") == 0)
				t->target = "putertarget5";
			else if(Q_strcasecmp(txtstring2, "putertarget6") == 0)
				t->target = "putertarget6";
			else if(Q_strcasecmp(txtstring2, "putertarget7") == 0)
				t->target = "putertarget7";
			else if(Q_strcasecmp(txtstring2, "putertarget8") == 0)
				t->target = "putertarget8";
			else if(Q_strcasecmp(txtstring2, "putertarget9") == 0)
				t->target = "putertarget9";
			else if(Q_strcasecmp(txtstring2, "putertarget10") == 0)
				t->target = "putertarget10";
			else if(Q_strcasecmp(txtstring2, "putertarget11") == 0)
				t->target = "putertarget11";
			else if(Q_strcasecmp(txtstring2, "putertarget12") == 0)
				t->target = "putertarget12";
			else if(Q_strcasecmp(txtstring2, "putertarget13") == 0)
				t->target = "putertarget13";
			else if(Q_strcasecmp(txtstring2, "putertarget14") == 0)
				t->target = "putertarget14";
			else if(Q_strcasecmp(txtstring2, "putertarget15") == 0)
				t->target = "putertarget15";
			else if(Q_strcasecmp(txtstring2, "putertarget16") == 0)
				t->target = "putertarget16";
			else if(Q_strcasecmp(txtstring2, "putertarget17") == 0)
				t->target = "putertarget17";
			else if(Q_strcasecmp(txtstring2, "putertarget18") == 0)
				t->target = "putertarget18";
			else if(Q_strcasecmp(txtstring2, "putertarget19") == 0)
				t->target = "putertarget19";
			else if(Q_strcasecmp(txtstring2, "putertarget20") == 0)
				t->target = "putertarget20";
			else
				return;
			return;
			}

		else if(Q_strcasecmp(txtstring, "_bt1") == 0)
			{
			if(dis_puta.bitwise[1] == 1)
				dis_puta.bitwise[1] = 0;
			else if(dis_puta.bitwise[1] == 0)
				dis_puta.bitwise[1] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_on1") == 0)
			{
			dis_puta.bitwise[1] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_of1") == 0)
			{
			dis_puta.bitwise[1] = 0;
			fgets(txtstring2b, 2, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_bt2") == 0)
			{
			if(dis_puta.bitwise[2] == 1)
				dis_puta.bitwise[2] = 0;
			else if(dis_puta.bitwise[2] == 0)
				dis_puta.bitwise[2] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_on2") == 0)
			{
			dis_puta.bitwise[2] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_of2") == 0)
			{
			dis_puta.bitwise[2] = 0;
			fgets(txtstring2b, 2, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_bt3") == 0)
			{
			if(dis_puta.bitwise[3] == 1)
				dis_puta.bitwise[3] = 0;
			else if(dis_puta.bitwise[3] == 0)
				dis_puta.bitwise[3] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_on3") == 0)
			{
			dis_puta.bitwise[3] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_of3") == 0)
			{
			dis_puta.bitwise[3] = 0;
			fgets(txtstring2b, 2, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_bt4") == 0)
			{
			if(dis_puta.bitwise[4] == 1)
				dis_puta.bitwise[4] = 0;
			else if(dis_puta.bitwise[4] == 0)
				dis_puta.bitwise[4] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_on4") == 0)
			{
			dis_puta.bitwise[4] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_of4") == 0)
			{
			dis_puta.bitwise[4] = 0;
			fgets(txtstring2b, 2, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_bt5") == 0)
			{
			if(dis_puta.bitwise[5] == 1)
				dis_puta.bitwise[5] = 0;
			else if(dis_puta.bitwise[5] == 0)
				dis_puta.bitwise[5] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_on5") == 0)
			{
			dis_puta.bitwise[5] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_of5") == 0)
			{
			dis_puta.bitwise[5] = 0;
			fgets(txtstring2b, 2, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_bt6") == 0)
			{
			if(dis_puta.bitwise[6] == 1)
				dis_puta.bitwise[6] = 0;
			else if(dis_puta.bitwise[6] == 0)
				dis_puta.bitwise[6] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_on6") == 0)
			{
			dis_puta.bitwise[6] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_of6") == 0)
			{
			dis_puta.bitwise[6] = 0;
			fgets(txtstring2b, 2, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_bt7") == 0)
			{
			if(dis_puta.bitwise[7] == 1)
				dis_puta.bitwise[7] = 0;
			else if(dis_puta.bitwise[7] == 0)
				dis_puta.bitwise[7] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_on7") == 0)
			{
			dis_puta.bitwise[7] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_of7") == 0)
			{
			dis_puta.bitwise[7] = 0;
			fgets(txtstring2b, 2, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_bt8") == 0)
			{
			if(dis_puta.bitwise[8] == 1)
				dis_puta.bitwise[8] = 0;
			else if(dis_puta.bitwise[8] == 0)
				dis_puta.bitwise[8] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_on8") == 0)
			{
			dis_puta.bitwise[8] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_of8") == 0)
			{
			dis_puta.bitwise[8] = 0;
			fgets(txtstring2b, 2, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_bt9") == 0)
			{
			if(dis_puta.bitwise[9] == 1)
				dis_puta.bitwise[9] = 0;
			else if(dis_puta.bitwise[9] == 0)
				dis_puta.bitwise[9] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_on9") == 0)
			{
			dis_puta.bitwise[9] = 1;
			fgets(txtstring2b, 2, current_exec);
			}
		else if(Q_strcasecmp(txtstring, "_of9") == 0)
			{
			dis_puta.bitwise[9] = 0;
			fgets(txtstring2b, 2, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_if1") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[1] == 0)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_if2") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[2] == 0)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_if3") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[3] == 0)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_if4") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[4] == 0)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_if5") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[5] == 0)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_if6") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[6] == 0)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_if7") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[7] == 0)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_if8") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[8] == 0)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_if9") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[9] == 0)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_nt1") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[1] == 1)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_nt2") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[2] == 1)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_nt3") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[3] == 1)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_nt4") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[4] == 1)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_nt5") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[5] == 1)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_nt6") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[6] == 1)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_nt7") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[7] == 1)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_nt8") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[8] == 1)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_nt9") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			if(dis_puta.bitwise[9] == 1)
				fgets(txtstring2, 72, current_exec);
			}

		else if(Q_strcasecmp(txtstring, "_tgl") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			gi.AddCommandString("toggleconsole\n");
			return;
			}

		else if(Q_strcasecmp(txtstring, "_cds") == 0)
			{
			fgets(txtstring2b, 2, current_exec);
			fgets(txtstring2b, 72, current_exec);
			strlngth = strlen(txtstring2b)-1;
			strcpy(txtstring2, "");
			strncat(txtstring2, txtstring2b, strlngth);
			dis_puta.bitwise[6] = 0;
			dis_puta.bitwise[7] = 0;
			dis_puta.bitwise[8] = 0;
			dis_puta.bitwise[9] = 0;
			Computer_Load_CDS(txtstring2, ent);	
			return;
			}

		else
			{
			fgets(txtstring2, 72, current_exec);
			}
		}

	fclose(current_exec);
	safe_cprintf(ent, PRINT_HIGH, "\n");
}

void Computer_Trigger (edict_t *ent)
{
	G_UseTargets (ent, ent->activator);
	G_FreeEdict (ent);
}

void Computer_Exec (char *exec, edict_t *ent)
{
	char		targetexec[16];

	if(puter_safety != level.time)
		{
		safe_cprintf(ent, PRINT_HIGH, "Not at Computer Console\n");
		return;
		}

	safe_cprintf(ent, PRINT_HIGH, "\n\n");

	if(Q_strcasecmp(exec, dis_puta.execs.name1) == 0)
	strcpy(targetexec, dis_puta.execs.target1);

	if(Q_strcasecmp(exec, dis_puta.execs.name2) == 0)
	strcpy(targetexec, dis_puta.execs.target2);

	if(Q_strcasecmp(exec, dis_puta.execs.name3) == 0)
	strcpy(targetexec, dis_puta.execs.target3);

	if(Q_strcasecmp(exec, dis_puta.execs.name4) == 0)
	strcpy(targetexec, dis_puta.execs.target4);

	if(Q_strcasecmp(exec, dis_puta.execs.name5) == 0)
	strcpy(targetexec, dis_puta.execs.target5);

	if(Q_strcasecmp(exec, dis_puta.execs.name6) == 0)
	strcpy(targetexec, dis_puta.execs.target6);

	if(Q_strcasecmp(exec, dis_puta.execs.name7) == 0)
	strcpy(targetexec, dis_puta.execs.target7);

	if(Q_strcasecmp(exec, dis_puta.execs.name8) == 0)
	strcpy(targetexec, dis_puta.execs.target8);

	if(Q_strcasecmp(exec, dis_puta.execs.name9) == 0)
	strcpy(targetexec, dis_puta.execs.target9);

	Computer_Run(targetexec, ent);
}
