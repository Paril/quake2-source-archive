// 
// maplist.c -- server maplist rotation/manipulation routines 
// 
// 
// 4/98 - L. Allan Campbell (Geist) 
// 

// INCLUDES ///////////////////////////////////////////////// 

#include "g_local.h" 
       

// FUNCTIONS //////////////////////////////////////////////// 

     // 
     // LoadMapList 
     // 
     // Opens the specified file and scans/loads the maplist names 
     // from the file's [maplist] section. (list is terminated with 
     // "###") 
     // 
     // Args: 
     //   filename - name of file containing maplist. 
     // 
     // Return: 0 = normal exit, maplist loaded 
     //         1 = abnormal exit 
     // 
     int LoadMapList(char *filename) 
     { 
        FILE *fp; 
        int  i = 0; 
        char szLineIn[80];

        fp = OpenFile(filename); 

        if (fp)  // opened successfully? 
        { 
           // scan for [maplist] section 
           do 
           { 
              fscanf(fp, "%s", szLineIn); 
           } while (!feof(fp) && (Q_stricmp(szLineIn, "[maplist]") != 0)); 

           if (feof(fp)) 
           { 
              // no [maplist] section 
              gi.dprintf ("-------------------------------------\n"); 
              gi.dprintf ("ERROR - No [maplist] section in \"%s\".\n", filename); 
              gi.dprintf ("-------------------------------------\n"); 
           } 
           else 
           { 
              gi.dprintf ("-------------------------------------\n"); 
       
              // read map names into array 
              while ((!feof(fp)) && (i<MAX_MAPS)) 
              { 
                 fscanf(fp, "%s", szLineIn); 

                 if (Q_stricmp(szLineIn, "###") == 0)  // terminator is "###" 
                    break; 

                 // TODO: check that maps exist before adding to list 
                 //       (might be difficult to search a .pak file for these) 

                 strncpy(maplist.mapnames[i], szLineIn, MAX_MAPNAME_LEN); 
                 gi.dprintf("...%s\n", maplist.mapnames[i]); 
                 i++; 
              } 

              strncpy(maplist.filename, filename, 20); 
           } 

           CloseFile(fp); 

           if (i == 0) 
           { 
              gi.dprintf ("No maps listed in [maplist] section of %s\n", filename); 
              gi.dprintf ("-------------------------------------\n"); 
              return 0;  // abnormal exit -- no maps in file 
           } 
       
           gi.dprintf ("%i map(s) loaded.\n", i); 
           gi.dprintf ("-------------------------------------\n"); 
           maplist.nummaps = i; 
           return 1; // normal exit 
        } 
       
        return 0;  // abnormal exit -- couldn't open file 
     } 
       

     // 
     // ClearMapList 
     // 
     // Clears/invalidates maplist. Might add more features in the future, 
     // but resetting .nummaps to 0 will suffice for now. 
     // 
     // Args:   (none) 
     // 
     // Return: (none) 
     // 
     void ClearMapList() 
     { 
        maplist.nummaps = 0; 
        dmflags->value = (int) dmflags->value & ~DF_MAP_LIST; 
        gi.dprintf ("Maplist cleared/disabled.\n"); 
     } 
       

     // 
     // DisplayMaplistUsage 
     // 
     // Displays current command options for maplists.  If not dedicated console (ent==NULL), 
     // then do not display all options. 
     // 
     // Args: 
     //   ent      - entity (client) to display help screen (usage) to. 
     //              if NULL, will print to dedicated console. 
     // 
     // Return: (none) 
     // 
     void DisplayMaplistUsage(edict_t *ent) 
     { 
        gi.cprintf (ent, PRINT_HIGH, "-------------------------------------\n"); 
        gi.cprintf (ent, PRINT_HIGH, "usage:\n"); 

        if (ent==NULL) 
        { 
           gi.cprintf (ent, PRINT_HIGH, "MAPLIST <filename> [<rotate_f>]\n"); 
           gi.cprintf (ent, PRINT_HIGH, "  <filename> - server ini file\n"); 
           gi.cprintf (ent, PRINT_HIGH, "  <rotate_f> - 0 = sequential (def)\n"); 
           gi.cprintf (ent, PRINT_HIGH, "               1 = random\n"); 
           gi.cprintf (ent, PRINT_HIGH, "MAPLIST START    - go to 1st map\n"); 
           gi.cprintf (ent, PRINT_HIGH, "MAPLIST NEXT     - go to next map\n"); 
           gi.cprintf (ent, PRINT_HIGH, "MAPLIST GOTO <n> - go to map #<n>\n"); 
        } 
       
        gi.cprintf (ent, PRINT_HIGH, "MAPLIST          - show current list\n"); 
        gi.cprintf (ent, PRINT_HIGH, "MAPLIST HELP     - (this screen)\n"); 

        if (ent==NULL) 
        { 
           gi.cprintf (ent, PRINT_HIGH, "MAPLIST OFF      - clear/disable\n"); 
        } 
       
        gi.cprintf (ent, PRINT_HIGH, "-------------------------------------\n"); 
     } 
       

     // 
     // ShowCurrentMaplist 
     // 
     // Displays current maplist. 
     // 
     // Args: 
     //   ent      - entity (client) to display help screen (usage) to. 
     //              if NULL, will print to dedicated console. 
     // 
     // Return: (none) 
     // 
     void ShowCurrentMaplist(edict_t *ent) 
     { 
        int i; 

        gi.cprintf (ent, PRINT_HIGH, "-------------------------------------\n"); 

        if (ent==NULL)     // only show filename to server 
           gi.dprintf ("FILENAME: %s\n", maplist.filename); 

        for (i=0; i<maplist.nummaps; i++) 
        { 
           gi.cprintf (ent, PRINT_HIGH, "#%2d \"%s\"\n", i+1, maplist.mapnames[i]); 
        } 

        gi.cprintf (ent, PRINT_HIGH, "%i map(s) in list.\n", i); 

        gi.cprintf (ent, PRINT_HIGH, "Rotation flag = %i ", maplist.rotationflag); 
        switch (maplist.rotationflag) 
        { 
        case ML_ROTATE_SEQ: 
           gi.cprintf (ent, PRINT_HIGH, "\"sequential\"\n"); 
           break; 

        case ML_ROTATE_RANDOM: 
           gi.cprintf (ent, PRINT_HIGH, "\"random\"\n"); 
           break; 
       
        default: 
           gi.cprintf (ent, PRINT_HIGH, "(ERROR)\n"); 
        } // end switch 

        if (maplist.currentmap == -1) 
        { 
           gi.cprintf (ent, PRINT_HIGH, "Current map = #-1 (not started)\n"); 
        } 
        else 
        { 
           gi.cprintf (ent, PRINT_HIGH, "Current map = #%i \"%s\"\n", 
                      maplist.currentmap+1, maplist.mapnames[maplist.currentmap]); 
        } 
       
        gi.cprintf (ent, PRINT_HIGH, "-------------------------------------\n"); 
     } 
       

     // 
     // Cmd_Maplist_f 
     // 
     // Client command line parsing function. Either displays the current list, or 
     // the syntax of the command. 
     // 
     // Args: 
     //   ent      - entity (client) to display messages to, if necessary. 
     // 
     // Return: (none) 
     // 
     void Cmd_Maplist_f (edict_t *ent) 
     { 
        switch (gi.argc()) 
        { 
        case 1:  // display current maplist 
           if (maplist.nummaps > 0)  // does a maplist exist? 
           { 
              ShowCurrentMaplist(ent); 
           } 
           else       // no maplist 
           { 
              gi.cprintf (ent, PRINT_HIGH, "*** No MAPLIST active ***\n"); 
              DisplayMaplistUsage(ent); 
           } 

           break; 

        case 2: 
           if (Q_stricmp(gi.argv(1), "HELP") == 0) 
           { 
              DisplayMaplistUsage(ent); 
           } 
           else // no other parameters allowed for clients 
           { 
              gi.cprintf (ent, PRINT_HIGH, "MAPLIST options locked by server.\n"); 
           } 

           break; 

        default: 
           DisplayMaplistUsage(ent); 
        }  // end switch 
     } 
       

     // 
     // Svcmd_Maplist_f 
     // 
     // Main command line parsing function. Enables/parses/diables maplists. 
     // 
     // Args:   (none) 
     // 
     // Return: (none) 
     // 
     void Svcmd_Maplist_f () 
     { 
        int  i;    // temp variable 
        char *filename; 
        edict_t *ent;           // for map changing, if necessary 

        switch (gi.argc()) 
        { 
        case 3:  // various commands, or enable and assume rotationflag default 
           if (Q_stricmp(gi.argv(2), "HELP") == 0) 
           { 
              DisplayMaplistUsage(NULL); 
              break; 
           } 

           if (Q_stricmp(gi.argv(2), "START") == 0) 
           { 
              if (maplist.nummaps > 0)  // does a maplist exist? 
                 EndDMLevel(); 
              else 
                 DisplayMaplistUsage(NULL); 

              break; 
           } 
           else if (Q_stricmp(gi.argv(2), "NEXT") == 0) 
           { 
              if (maplist.nummaps > 0)  // does a maplist exist? 
                 EndDMLevel(); 
              else 
                 DisplayMaplistUsage(NULL); 

              break; 
           } 
           else if (Q_stricmp(gi.argv(2), "OFF") == 0) 
           { 
              if (maplist.nummaps > 0)  // does a maplist exist? 
              { 
                 ClearMapList(); 
              } 
              else 
              { 
                 // maplist doesn't exist, so display usage 
                 DisplayMaplistUsage(NULL); 
              } 

              break; 
           } 
           else 
              maplist.rotationflag = 0; 
       
           // no break here is intentional;  supposed to fall though to case 3 

        case 4:  // enable maplist - all args explicitly stated on command line 
           if (gi.argc() == 4)  // this is required, because it can still = 2 
           { 
              i = atoi(gi.argv(3)); 

              if (Q_stricmp(gi.argv(2), "GOTO") == 0) 
              { 
                 // user trying to goto specified map # in list 
                 if ((i<1) || (i>maplist.nummaps)) 
                 { 
                    gi.dprintf("*** Map# out of range ***\n"); 
                    ShowCurrentMaplist(NULL); 
                 } 
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
                    DisplayMaplistUsage(NULL); 
                    break; 
                 } 
                 else 
                 { 
                    maplist.rotationflag = atoi(gi.argv(3)); 
                 } 
              } 
           } 

           filename = gi.argv(2);   // get filename from command line 

           if ((int) dmflags->value & DF_MAP_LIST) 
           { 
              // tell user to cancel current maplist before starting new maplist 
              gi.dprintf ("You must disable current maplist first. (SV MAPLIST OFF)\n"); 
           } 
           else 
           { 
              // load new maplist 
              if (LoadMapList(filename))  // return 1 = success 
              { 
                 dmflags->value = (int) dmflags->value | DF_MAP_LIST; 
                 gi.dprintf ("Maplist created/enabled. You can now use START or NEXT.\n"); 
                 maplist.currentmap = -1; 
              } 
           } 

           break; 

        case 2:  // display current maplist 
           if (maplist.nummaps > 0)  // does a maplist exist? 
           { 
              ShowCurrentMaplist(NULL); 
           } 
           else 
           { 
              DisplayMaplistUsage(NULL); 
           } 
           break; 

        default: 
           DisplayMaplistUsage(NULL); 
        }  // end switch 
     }


