#include "g_local.h"




int GetLineFromFile(FILE *in, char s[])
{
	int i, c;

	// This reads characters from in into s until MAX_LINE_SIZE-1 is reached,
	// a newline character is reached, or an EOF is reached.
	for (i = 0; i < 254 && (c = fgetc(in)) != '\n' && c != EOF; i++)
		s[i] = c;
	// Add a '\0' to the end of s
	s[i] = '\0';
	return i;
}




qboolean findspawnpoint (edict_t *ent)
{
  vec3_t loc = {0,0,0};
  vec3_t floor;
  int i;
  int j = 0;
  int k = 0;
  int jmax = 1000;
  int kmax = (int)(jmax/2);
  qboolean found = false;
  trace_t tr;
  do {
    if (found || j >= jmax || k >= kmax)
      break;
    j++;
    for (i = 0; i < 3; i++)
      loc[i] = rand() % (8192 + 1) - 4096;
    if (gi.pointcontents(loc) == 0)
    {
      VectorCopy(loc, floor);
      floor[2] = -4096;
      tr = gi.trace (loc, ent->maxs, ent->mins, floor, NULL, MASK_SOLID|MASK_WATER);
      k++;
      if (tr.contents & MASK_WATER)
        found = false;
      else
      {
        VectorCopy (tr.endpos, loc);
        loc[2] += ent->maxs[2] - ent->mins[2]; // make sure the entity can fit!
        found = (gi.pointcontents(loc) == 0 ? true : false);
      }
    }
    else
      found = false;
  } while (!found && j < jmax && k < kmax);
  if (!found && (j >= jmax || k >= kmax))
    return false;
  VectorCopy(loc,ent->s.origin);
  VectorCopy(loc,ent->s.old_origin);
  return true;
}

qboolean spawn_random_item(char *classname)
{

    edict_t     *ent;
    gitem_t     *item;

//    int spawnrecurse;

    item = FindItemByClassname(classname);

    if (!item)
	{
        gi.dprintf("doesn't exist");
	return 0;
	}

    ent = G_Spawn();

    ent->classname=gi.TagMalloc(64*sizeof(char), TAG_LEVEL);
    strcpy(ent->classname,classname);
    
    ent->item = item;
    ent->s.effects = ent->item->world_model_flags;
    ent->s.renderfx = RF_GLOW;
    ent->touch = Touch_Item;
   
    VectorSet (ent->mins, -15, -15, -15);
    VectorSet (ent->maxs, 15, 15, 15);
    gi.setmodel (ent, ent->item->world_model);
    ent->solid = SOLID_TRIGGER;
    ent->movetype = MOVETYPE_BOUNCE;  
    ent->owner = ent;
    

    while (findspawnpoint(ent) == false){}
    
   

    gi.linkentity (ent);
    gi.dprintf("%s Added at %d,%d,%d\n",ent->classname,(int)ent->s.origin[0],(int)ent->s.origin[1],(int)ent->s.origin[2]);
    return 1;
}






qboolean load_random_items(void)
{
 FILE        *fp;
    char        filename[256],classname[64],buffer[256];
    cvar_t      *game_dir;
//    vec3_t      spot;
    long        line;
    int         i;
    qboolean    inkey=0;
    
    game_dir = gi.cvar ("game", "", 0);

#ifndef AMIGA
    i =  sprintf(filename, ".\\");
    i += sprintf(filename + i, game_dir->string);
#else
    i = sprintf(filename,game_dir->string);	
#endif
#ifndef AMIGA
    i += sprintf(filename + i, "\\config\\");
#else
    i += sprintf(filename + i, "/config/");
#endif	
    i += sprintf(filename + i, level.mapname);
    i += sprintf(filename + i, ".rif");   // Random Item file :)


    fp = fopen (filename, "rt");
    if (!fp){
	gi.dprintf ("No Random item file found\n");	
        return 0;
	}
    
    line=0;
    while (!feof(fp))
    {
	
        line++;
	GetLineFromFile(fp,buffer);        
        if (buffer[0]=='\0' || buffer[0]=='#')
            continue;
        classname[0]='\0';
        sscanf(buffer,"%63s",classname);

        if (strlen(classname)==0 || !FindItemByClassname(classname))
        {
           gi.dprintf("Error in config/%s.rif on line %d!\n",level.mapname,line);
        }
        else 
        {
            if (!spawn_random_item(classname))
		gi.dprintf("fuck up at line %d\n",line);
	}
    }

    fclose(fp);
    
    return 1;
}




