//============================================================================
// p_nhscoreboard.c
//
// Night Hunters scoreboard
//
// Originally coded by DingBat
//
// Copyright (c), 1999 The BatCave. All Rights Reserved.
//============================================================================


#include "g_local.h"
#include "p_nhscoreboard.h"

int findChasePlayerNumber(edict_t *target, int sorted[], int max) {

  int i ;
  edict_t *temp = NULL ;

  for (i = 0; i < max; i++) {

    temp = g_edicts + 1 + sorted[i] ;
    if (temp == target)
      break ;

  }

  if (i == max)
    return -1 ;
  else return (i + 1) ;

}

void NHScoreboardMessage (edict_t *ent, edict_t *killer)
{
  char    entry[1024];
  char    string[1400];
  int             stringlength;
  int             i, j, k;
  int             sorted[MAX_CLIENTS];
  int             sortedscores[MAX_CLIENTS];
  int             score, total;
  int             picnum;
  int             x, y;
  gclient_t       *cl;
  edict_t         *cl_ent;
  char    *tag;

  char status[20] ; // Status of players.

  // sort the clients by score
  total = 0;
  for (i=0 ; i<game.maxclients ; i++) {

    cl_ent = g_edicts + 1 + i;
    if (!cl_ent->inuse)
      continue;
    score = game.clients[i].resp.score;
    for (j=0 ; j<total ; j++) {
      if (score > sortedscores[j])
	break;
    }
     
    for (k=total ; k>j ; k--) {
      sorted[k] = sorted[k-1];
      sortedscores[k] = sortedscores[k-1];
    }
    sorted[j] = i;
    sortedscores[j] = score;
    total++;
  }

  // print level name and exit rules
  string[0] = 0;

  stringlength = strlen(string);

  // add the clients in sorted order
  if (total > 12)
    total = 12;

  // Display new header.
  Com_sprintf(entry, sizeof(entry),
	      "xv 32 yv 16 string2 \"%-2.2s %-15.15s  %-5.5s  %-4.4s %-4.4s  %s\" ",
	      "#", "Player", "Score", "Ping", "Time", "Status") ;

  // Update lengths
  j = strlen(entry) ;
  if (stringlength + j > 1024) 
    gi.dprintf("Overflow problem in NHScoreboardmessage\n") ;
  else {
    strcpy(string + stringlength, entry) ;
    stringlength += j ;
  }


  for (i=0 ; i<total ; i++) {
    cl = &game.clients[sorted[i]];
    cl_ent = g_edicts + 1 + sorted[i];

    picnum = gi.imageindex ("i_fixme");

    // New locations for entries
    x = 32 ;
    y = 32 + (8 * i) ;
    
    // Determine player status.
    if (ent->isCheating)
      Com_sprintf(status, sizeof(status), "Cheating") ;
    else if (cl_ent->isPredator)
      Com_sprintf(status, sizeof(status), "Predator") ;

    else if (cl_ent->isObserving) {

      if ((cl_ent->client->chase_target) &&
	  (cl_ent->client->chase_target->inuse)) {

	int chased = -1 ;
	chased = findChasePlayerNumber(cl_ent->client->chase_target,
				       sorted, total) ;

	if (chased == -1)
	  Com_sprintf(status, sizeof(status),
		      "Chasing...") ;
	else
	  Com_sprintf(status, sizeof(status),
		      "Chasing #%d", chased) ;
      }
      else
	Com_sprintf(status, sizeof(status), "Observing") ;
    }
    else
      if (cl_ent->killedMarines >= getPenalty())
      	Com_sprintf(status, sizeof(status), "(P)Marine") ;
     else
      	Com_sprintf(status, sizeof(status), "Marine") ;

    // No dog tags.
    if (cl_ent == ent) {

      // Entry for myself.
      Com_sprintf(entry, sizeof(entry), 
		  "xv 32 yv %i string \"%2d %-15.15s  %4i %5i %4i   %s\" ",
		  y,
		  i + 1,
		  cl->pers.netname,
		  cl->resp.score,
		  cl->ping,
		  (level.framenum - cl->resp.enterframe)/600,
		  status) ;
    }
    else {

      // This is the way to do it.
      Com_sprintf(entry, sizeof(entry),
		  "xv 32 yv %i string2 \"%2d %-15.15s  %4i %5i %4i   %s\" ",
		  y,
		  i + 1,
		  cl->pers.netname,
		  cl->resp.score,
		  cl->ping,
		  (level.framenum - cl->resp.enterframe)/600,
		  status) ;

    }

    j = strlen(entry);
    if (stringlength + j > 1024)
      break;
    strcpy (string + stringlength, entry);
    stringlength += j;
  }

  gi.WriteByte (svc_layout);
  gi.WriteString (string);
}

/*
==================
NHScoreboard

Night Hunters custom scoreboard.
==================
*/
void NHScoreboard (edict_t *ent)
{
	NHScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}

/*
==================
Cmd_NHScore_f

Display the Night Hunters custom scoreboard
==================
*/
void Cmd_NHScore_f (edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;

	ent->ShowMOTD = 0; // Added by Alex


	if (!deathmatch->value && !coop->value)
		return;
	
	// NH change: CTF-like menu.
	// If the menu is showing, close it.
	if (ent->client->menu)
	  PMenu_Close(ent);

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
//		ent->client->update_chase = true;
		return;
	}

	ent->client->showscores = true;
	NHScoreboard (ent);
}



