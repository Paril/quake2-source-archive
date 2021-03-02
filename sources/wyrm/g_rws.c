//-----------------------------------------------------------------------
//   RWS.C
//-----------------------------------------------------------------------

#include "g_local.h"
#include "m_player.h"

//***********************************************************************
//
//
//	Makes additions to the edict_s structure in "g_local.h" :
//
//		gitem_t		*org_item;
//
//
//***********************************************************************


d_RndWeapon		wrndtbl[MAX_ITEMS];

int				actual_items,
				rws_err_count;

char			map_name_copy[64];

//-----------------------------------------------------------------------

//void SaveMapName(char *map_name)
//{
//}

//-----------------------------------------------------------------------

qboolean ReadString(FILE * f,char * c)
{
	do
	{
		*c = 0;

		// reach EOF ?
		if (fgets(c,LINE_SIZE,f) == NULL)
			return false;

		if (strlen(c))
			*(c + strlen(c) - 1) = 0;
	}
	while ((*c == ';') || !strlen(c));

	return true;
}

//-----------------------------------------------------------------------

void	ClockRandomError(FILE * log,char * errstr)
{
	fprintf(log,"%s !\n",errstr);
	rws_err_count++;
}

//-----------------------------------------------------------------------

int	FindInTable(char * name)
{
        gitem_t *item;

        if (!name) return -1;

        item = FindItemByClassname(name);

        if (!item)
                return -1;
        else
                return ITEM_INDEX(item);
}

//-----------------------------------------------------------------------

void strupr(char *c);

void	ReadInRandomWeaponTable(char *map_name)
{
	int		i;

//	gitem_t	*it;

	FILE	*f,
			*o;

//			*cv_map;

	char	infsp_cfg[64],
//			mapinfsp_cfg[64],
			outfsp_cfg[64],
			dfsp_cfg[64],

			*separator = "------------------------------------------";

	qboolean	isgame,
//				ismap,
				iscfg_in,
				iscfg_out,
				iscfg_def;


	// save map name
	if (map_name)
		strcpy(map_name_copy,map_name);

	// no items to start with ...
	actual_items = 0;
	rws_err_count = 0;

	// query "rwcfgXXX" cvar's for values
	iscfg_in = (strlen(cv_rwcfgin -> string) > 0);
	iscfg_out = (strlen(cv_rwcfgout -> string) > 0);
	iscfg_def = (strlen(cv_rwcfgdef -> string) > 0);

	// query current game
	isgame = (strlen(cv_game -> string) > 0);

  	// setup default table to output ?
	if (iscfg_def)
	{
		sprintf(dfsp_cfg,"%s\\%s",(isgame) ? (cv_game -> string) : "baseq2",cv_rwcfgdef -> string);

		f = fopen(dfsp_cfg,"wt");
	}

	// create defaults ...
        for (i=0 ; i<game.num_items ; i++)
	{
		// any name ?
                //if (!it->classname)
                //        continue;

		// create default spawn info
		wrndtbl[actual_items].mode	= ITEM_FIXED;
		wrndtbl[actual_items].num	= 0;
		wrndtbl[actual_items].index	= 0;

		// set first fixed as a default ...
                wrndtbl[actual_items].rnd_item[0] = itemlist + i;

		actual_items++;

		// output to default file ?
//		if (iscfg_def && f)
//			fprintf(f,"%s\nFIXED\n%s\n",itemlist[itemlist+i] -> classname,it -> classname);
	}

	// def file open ?
	if (iscfg_def && f)
		fclose(f);

        // open configuration file - try "MAP_NAME.rws" first
        sprintf(infsp_cfg,"%s\\%s.rws",         (isgame) ? (cv_game -> string) : "baseq2",
							   			map_name_copy);

	// open ok ?
	if (f = fopen(infsp_cfg,"rt"))
	{
		// create new log file fsp ...
	  	sprintf(outfsp_cfg,"%s\\%s.log",	(isgame) ? (cv_game -> string) : "baseq2",
											map_name_copy);
	}
	else
	{
		// try other possible cfg file
		sprintf(infsp_cfg,"%s\\%s",		(isgame) ? (cv_game -> string) : "baseq2",
								   		(iscfg_in) ? (cv_rwcfgin -> string) : RND_CFG_IN);

		// create new log file fsp ...
		f = fopen(infsp_cfg,"rt");

	  	sprintf(outfsp_cfg,"%s\\%s",	(isgame) ? (cv_game -> string) : "baseq2",
										(iscfg_out) ? (cv_rwcfgout -> string) : RND_CFG_OUT);
	}

	// open log file
	o = fopen(outfsp_cfg,"wt");

	// ok files ?
	if (f && o)
	{
		char	item_name[LINE_SIZE],
				perm_name[LINE_SIZE],
				perm_str[LINE_SIZE],
				mode[LINE_SIZE];

		int		tindex,
				perms;

		// debug
		fprintf(o,"%s\nReading CFG from : %s\nWriting LOG to (this) : %s\n",	separator,
																   				infsp_cfg,
																		   		outfsp_cfg);

		// map ?
		if (map_name)
			fprintf(o,"Map Name : '%s'\n",map_name_copy);

		// loop thru weapon spawns
		while (true)
		{
			// Read item ...
			if (!ReadString(f,item_name))
				break;

			fprintf(o,"%s\nReading item : '%s'\n",separator,item_name);

			// Find item ...
			if ((tindex = FindInTable(item_name)) < 0)
			{
				ClockRandomError(o,"Can't find item");
				continue;
			}

			// Read mode ....
			if (!ReadString(f,mode))
				break;

			fprintf(o,"Mode requested : '%s'\n",mode);

			strupr(mode);

			// mode check ...
			if (!strcmp(mode,"RANDOM"))
			{
				wrndtbl[tindex].mode = ITEM_RANDOM;

				fprintf(o,"Mode RANDOM OK\n");
			}
			else if (!strcmp(mode,"CYCLE"))
			{
				wrndtbl[tindex].mode = ITEM_CYCLE;

				fprintf(o,"Mode CYCLE OK\n");
			}
			else if (!strcmp(mode,"REMOVE"))
			{
				wrndtbl[tindex].mode = ITEM_REMOVE;

				fprintf(o,"Mode REMOVE OK\n");

				continue;
			}
                        else if (!strcmp(mode,"UPGRADE"))
			{
                                wrndtbl[tindex].mode = ITEM_UPGRADE;

                                fprintf(o,"Mode UPGRADE OK\n");
			}
                        else if (!strcmp(mode,"GIVE"))
			{
                                wrndtbl[tindex].mode = ITEM_GIVE;

                                fprintf(o,"Mode GIVE OK\n");
			}
                        else if (!strcmp(mode,"CLASS"))
			{
                                wrndtbl[tindex].mode = ITEM_CLASS;

                                fprintf(o,"Mode CLASS OK\n");
			}
			else if (!strcmp(mode,"FIXED"))
			{
				fprintf(o,"Mode FIXED OK\n");
			}
			else
				ClockRandomError(o,"BAD Mode");

			// Read number of permutations
			if (wrndtbl[tindex].mode == ITEM_FIXED)
				perms = 1;
			else
			{
				if (!ReadString(f,perm_str))
					break;

				perms = atoi(perm_str);
			}

			fprintf(o,"%d permutations requested\n",perms);

			// valid count ?
			if ((perms < 1) || (perms > MAX_RANDOM_WEAPON))
			{
				ClockRandomError(o,"Illegal permutation count");
				break;
			}

			// loop thru perms
			for (i = 0 ; i < perms ; i++)
			{
				struct gitem_s	* pindex;


				// read perm name 
				if (!ReadString(f,perm_name))
					break;

				fprintf(o,"Reading perm item : '%s'\n",perm_name);

				// valid item ?
				if (!(pindex = FindItemByClassname(perm_name)))
				{
					ClockRandomError(o,"Can't find perm item");
					break;
				}
				else
				{
					wrndtbl[tindex].rnd_item[wrndtbl[tindex].num] = pindex;

					wrndtbl[tindex].num++;

					fprintf(o,"Perm item OK : %s\n",perm_name);
				}
			}

			// check to see if a RANDOM/FIXED has 0 valid items
			if (!wrndtbl[tindex].num)
			{
				wrndtbl[tindex].mode = ITEM_FIXED;

				ClockRandomError(o,"No perm items found : ITEM_FIXED set");
			}
		}

		fprintf(o,"%s\nErrors : %d\n%s",separator,rws_err_count,separator);

		fclose(f);
		fclose(o);
	}
}

//-----------------------------------------------------------------------

struct gitem_s	* FindRandomWeapon(edict_t * ent,int *ret_code)
{
	int				i;

	struct gitem_s	* p;

	// checks ...
        if (ent -> org_item)
        {
                if (ent->org_item->classname)
                {
                        i = ITEM_INDEX(ent->org_item);

                        *ret_code = wrndtbl[i].mode;

                        switch (wrndtbl[i].mode)
                        {
                                case ITEM_FIXED:
                                        return wrndtbl[i].rnd_item[0];
                                        
                                case ITEM_UPGRADE:
                                case ITEM_CLASS:
                                case ITEM_GIVE:
                                        return ent->org_item;

                                case ITEM_CYCLE:
                                        p = wrndtbl[i].rnd_item[wrndtbl[i].index];
                                        wrndtbl[i].index = (wrndtbl[i].index + 1) % wrndtbl[i].num;

                                        return p;

                                case ITEM_RANDOM:
                                        return wrndtbl[i].rnd_item[((int) (random() * 1000)) % wrndtbl[i].num];

                                case ITEM_REMOVE:
                                        return  NULL;
                        }
                }
        }

	return NULL;
}

//-----------------------------------------------------------------------
//
//-----------------------------------------------------------------------

