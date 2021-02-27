#include "g_local.h" 

void EndDMLevel (void);
  
void ClearMapVotes() 
{ 
	int i;
	for (i=0; i < MAX_MAPS; ++i)
		maplist.votes[i] = 0;
} 

//Find highest voted map
/*
  Returns
    -1  No votes
	0-31 Index to selected map  
*/
int MapMaxVotes() 
{ 
	int i;
	int numvotes;
	int index;

	numvotes = 0;
	index = -1;
	i = 0;

	while (i < maplist.nummaps)
	{
		if (maplist.votes[i] > numvotes)
		{
		numvotes = maplist.votes[i];
		index = i;
		}
		++i;
	}
	return(index);
} 

void VoteForMap(int i)
{
	if (i >= 0 && i < maplist.nummaps)
	{
		++maplist.votes[i];
		maplist.people_voted = 1;
	}

}

void DumpMapVotes()
{
	int i;
	for (i = 0; i < maplist.nummaps; ++i)
		gi.dprintf("%d. %s (%d votes)\n",
		   i, maplist.mapnames[i], maplist.votes[i]);
}


 // 
// ClearMapList 
// 
// Clears/invalidates maplist. Might add more features in the future, 
// but resetting .nummaps to 0 will suffice for now. 
// 
// Args: 
//   ent      - entity (client) to print diagnostic messages to (future development). 
// 
// Return: (none) 
// 
void ClearMapList() 
{ 
   maplist.nummaps = 0; 
   ClearMapVotes();
   maplist.active = 0;
} 
  

 // 
// DisplayMaplistUsage 
// 
// Displays current command options for maplists. 
// 
// Args: 
//   ent      - entity (client) to display help screen (usage) to. 
// 
// Return: (none) 
// 
void DisplayMaplistUsage(edict_t *ent) 
{ 
   gi.dprintf ("-------------------------------------\n"); 
   gi.dprintf ("usage:\n"); 
   gi.dprintf ("SV MAPLIST START to go to 1st map\n"); 
   gi.dprintf ("SV MAPLIST NEXT to go to next map\n"); 
   gi.dprintf ("SV MAPLIST to display current list\n"); 
   gi.dprintf ("SV MAPLIST OFF to clear/disable\n"); 
   gi.dprintf ("SV MAPLIST RANDOM for random map rotation\n"); 
   gi.dprintf ("SV MAPLIST SEQUENTIAL for sequential map rotation\n"); 
   gi.dprintf ("SV MAPLIST HELP for this screen\n"); 
   gi.dprintf ("-------------------------------------\n"); 
} 

// MaplistNextMap
// Choose the next map in the list, or use voting system
void MaplistNextMap(edict_t *ent)
{ 
	int votemap;
	int i;
	int j;

	DumpMapVotes();

// Jason - j is so we alter the struct member currentmaps in the loops.

	j = maplist.currentmap;

	switch (maplist.rotationflag)        // choose next map in list 
	{ 
	case ML_ROTATE_SEQ:        // sequential rotation

// Jason - If there is only one map don't infinate loop

		if (maplist.nummaps > 1)
		{

// Jason - do while loops chooses next map and obeys the voteonly array
		
			do
			{
				i = (j + 1) % maplist.nummaps; 
				j++;
			}
			while (maplist.voteonly[i] == true);
		}
		else
			i = 0;
		break; 

	case ML_ROTATE_RANDOM:     // random rotation 
		if (maplist.nummaps > 1)
		{
			do
			{
				i = (int) (random() * maplist.nummaps); 
			}
			while ((maplist.voteonly[i] == true) || (i == j));
		}
		else
			i = 0;
		break; 

	default:       // should never happen, but set to first map if it does 
		i=0; 
	} // end switch 

	//See if map voting is on
	votemap = MapMaxVotes();
	// pick this map if at least 50% of the server voted for it
	// otherwise we continue normally
	if ((votemap >= 0) && (maplist.people_voted >= 0.5*JoinedPlayers()))	//Yes there was one picked
		i = votemap;
	ClearMapVotes();

	maplist.currentmap = i; 
	ent->map = maplist.mapnames[i]; 
} 


  
// 
// Cmd_Maplist_f 
// 
// Main command line parsing function. Enables/parses/diables maplists. 
// 
// Args: 
//   ent      - entity (client) to display messages to, if necessary. 
// 
// Return: (none) 
// 
// TO DO: change "client 0" privs to be for server only, if dedicated. 
//        only allow other clients to view list and see HELP screen. 
//        (waiting for point release for this feature) 
// 
#define MLIST_ARG_1	2
#define MLIST_ARG_2	3
#define MLIST_ARG_3	4


void Cmd_Maplist_f (edict_t *ent) 
{ 
   int  i;    // looping and temp variable 
   int argcount;
//	char *filename; 

   argcount = gi.argc() - 1;
//gi.dprintf("argcount = %d, arg1 = %s, arg2 = %s\n", argcount, gi.argv(MLIST_ARG_1),
//		   gi.argv(MLIST_ARG_2)); 

   switch (argcount) 
   { 
   case 2:  // various commands, or enable and assume rotationflag default 
//gi.dprintf("arg count 2\n");
      if (Q_stricmp(gi.argv(MLIST_ARG_1), "HELP") == 0) 
      { 
         DisplayMaplistUsage(ent); 
         break; 
      } 
       if (Q_stricmp(gi.argv(MLIST_ARG_1), "START") == 0) 
      { 
		if (maplist.nummaps > 0)  // does a maplist exist? 
			EndDMLevel(); 
		else 
		{
			gi.dprintf("Can't start - No maps in current list\n");
			DisplayMaplistUsage(ent); 
		}
		break; 
      } 
      else if (Q_stricmp(gi.argv(MLIST_ARG_1), "NEXT") == 0) 
      { 
         if (maplist.nummaps > 0)  // does a maplist exist? 
            EndDMLevel(); 
         else 
		 {
			gi.dprintf("Can't do next - No maps in current list\n");
            DisplayMaplistUsage(ent); 
		 }
          break; 
      } 
      else if (Q_stricmp(gi.argv(MLIST_ARG_1), "RANDOM") == 0) 
      { 
			maplist.rotationflag = ML_ROTATE_RANDOM;
			gi.dprintf("Map rotation set to random.\n");			
			break; 
      } 
      else if (Q_stricmp(gi.argv(MLIST_ARG_1), "SEQUENTIAL") == 0) 
      { 
			maplist.rotationflag = ML_ROTATE_SEQ; 
			gi.dprintf("Map rotation set to sequential.\n");			
			break; 
      } 
      else if (Q_stricmp(gi.argv(MLIST_ARG_1), "OFF") == 0) 
      { 
         if (maplist.nummaps > 0)  // does a maplist exist? 
         { 
            gi.dprintf ("Maplist disabled.\n"); 
			maplist.active = 0;
         } 
         else 
         { 
            // maplist doesn't exist, so display usage 
			gi.dprintf("No maps in current list\n");
            DisplayMaplistUsage(ent); 
         } 
          break; 
      } 
      else 
	  {
         maplist.rotationflag = 0; 
	  }
  
        // no break here is intentional;  supposed to fall though to case 3 
    case 3:  // enable maplist - all args explicitly stated on command line 
//gi.dprintf("arg count 3\n");
	  if (gi.argc() == 3)  // this is required, because it can still = 2 
	  { 
		i = atoi(gi.argv(MLIST_ARG_2)); 
		if (Q_stricmp(gi.argv(MLIST_ARG_1), "GOTO") == 0)
		{ 
            // user trying to goto specified map # in list 
            if ((i<1) || (i>maplist.nummaps)) 
               DisplayMaplistUsage(ent); 
            else 
            { 
               ent = G_Spawn (); 
               ent->classname = "target_changelevel"; 
               ent->map = maplist.mapnames[i-1]; 
               maplist.currentmap = i-1; 
               BeginIntermission(ent); 
            } 
             break; 
         } 
         else 
         { 
            // user trying to specify new maplist 
            if ((i<0) || (i>=ML_ROTATE_NUM_CHOICES))  // check for valid rotationflag 
            {         
               // outside acceptable values for rotationflag 
               DisplayMaplistUsage(ent); 
               break; 
            } 
            else 
            { 
               maplist.rotationflag = atoi(gi.argv(MLIST_ARG_2)); 
            } 
         } 
      } 
	  gi.dprintf("The map list is now loaded automatically through the wfserver.ini file.\n");
      break; 
 case 1:  // display current maplist 
//gi.dprintf("arg count 1\n");
      if (maplist.nummaps > 0)  // does a maplist exist? 
      { 
         gi.dprintf ("-------------------------------------\n"); 
         for (i=0; i<maplist.nummaps; i++) 
         { 
            gi.dprintf ("#%2d \"%s\" (%d votes)\n", i+1, maplist.mapnames[i], maplist.votes[i]); 
         } 
          gi.dprintf ("%i map(s) in list.\n", i); 
          gi.dprintf ("Rotation flag = %i ", maplist.rotationflag); 
         switch (maplist.rotationflag) 
         { 
         case ML_ROTATE_SEQ: 
            gi.dprintf ("\"sequential\"\n"); 
            break; 
         case ML_ROTATE_RANDOM: 
            gi.dprintf ("\"random\"\n"); 
            break; 
  
         default: 
            gi.dprintf ("(ERROR)\n"); 
         } // end switch 
          if (maplist.currentmap == -1) 
         { 
            gi.dprintf ("Current map = #-1 (not started)\n"); 
         } 
         else 
         { 
            gi.dprintf ("Current map = #%i \"%s\"\n", 
               maplist.currentmap+1, maplist.mapnames[maplist.currentmap]); 
         } 
  
         gi.dprintf ("-------------------------------------\n"); 
         break; 
      } 
       // this is when the command is "sv maplist", but no maplist exists to display 
      DisplayMaplistUsage(ent); 
      break; 
    default: 
      DisplayMaplistUsage(ent); 
   }  // end switch 
}
