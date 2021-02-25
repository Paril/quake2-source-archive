
// p_motd.c
//
// Night Hunters menu
//
// Copyright (c), 1999 The BatCave. All Rights Reserved.
//============================================================================

// CTF-like menu for NH

#include "g_local.h"


void ShowMOTD(edict_t *ent)
{

  char textfile[500];
  char line[80];
  char entry[120];	// a line with the xv yv etc..
  char string[1400];	// contains what is sent to WriteString
	
  int i=0;	// for going through each motd.txt line - counter
  int y=16;	// for incrementing the y coord on screen
  int j=0;	// temp for creating string
  int stringlength=0;	// temp for contatenating entry's into string
	
  FILE *motd_file;

  if (motd_file = fopen("nhunters/motd.txt", "r"))  {

    // we successfully opened the file "motd.txt"
    if (fgets(textfile, 500, motd_file)) {
        
      // we successfully read a line from "motd.txt" into motd
      // ... read the remaining lines now
      while (( fgets(line, 80, motd_file) ) && (i <20)) {

	// add each new line to motd, to create a BIG message entry.
	// we are using strcat: STRing conCATenation function here.
	Com_sprintf(entry, sizeof(entry),"xv 0 yv %i string \"%s\" ",y,line);
			
	j = strlen(entry);
	strcpy(string + stringlength, entry) ;
	stringlength +=j;
	
	i++;
	y+=8; 
      }
      
    }
    // be good now ! ... close the file
    fclose(motd_file);
  }
  
#if 0	// Alex - take out for now
  else
  {
  	// can't find motd.txt file, so fill with null..
  	i=0;
  	while (i <= 20)
  	{
  		entry[i]=NULL;
		i++;
	}
	i=1;
  }
#endif

  gi.WriteByte (svc_layout);
  gi.WriteString (string);
  gi.unicast (ent, true);

  // Added by bruce
  ent->ShowMOTD = (int)(level.time + getMotdTime() ) ;
}

qboolean 
ShowingMOTD(edict_t *ent) {

  if (ent->ShowMOTD > (int)level.time)  
    return true ;

  return false ;
}

void
ClearMOTD(edict_t *ent) {
  ent->ShowMOTD = (int)(level.time) ;
}


void validateMotdTime() {

  // motd_time must be between 0 and 9999
  if ((motd_time->value < 0) ||
      (motd_time->value > 9999))
    gi.cvar_set("motd_time", MOTD_TIME_DEFAULT );
}

int getMotdTime() {

  // Check if the cvar has changed.
  if (motd_time->modified)
    validateMotdTime() ;

  return motd_time->value ;
}
