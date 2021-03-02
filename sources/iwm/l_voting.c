/**************************************************/
/*                Quake 2 Voting                  */
/**************************************************/

/*
* Copyright (C) 2006 by QwazyWabbit and ClanWOS.org
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
*
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
* You may freely use and alter this code so long as this banner
* remains and credit is given for its source.
*/

/**********************************************************
* This module is designed to be a generic voting module.
* Some enumerated election types are defined in l_voting.h
* even though they may not be implemented here yet.
*
* I originally intended to have a "vote kick" function
* and some game configuration voting but once I saw the map
* vote spamming and abuse I decided against it for now.
* I think a votable changeover from DM to CTF might be useful
* but right now I think the admins should decide.
* -QwazyWabbit
*
* Spectators don't count for voting and can't vote.
* When using GameCam proxy in server, ent->inuse will be false.
***********************************************************/

// Module Interface:

// Call Voting_InitVars() in InitGame to set the default cvars this module needs.
// Call Voting_CheckVoting() inside CheckDMRules(). This cycles the announcements and timers.

// Call Voting_BeginElection() from inside ClientCommand per the sample
// at the start of Voting_BeginElection below.

// Call Voting_CmdVote_f() from ClientCommand with TRUE for yes vote, FALSE for no.

// Call Voting_KillVoting() inside EndDMLevel() to clean up just before BeginIntermission.

// Add these members to client_respawn_t
/*
   float      entertime;      // level.time the client entered the game
   qboolean   voted;         // for elections
   int         votes_started;
*/

#include "g_local.h"
#include "l_voting.h"
#include "maplist.h"

cvar_t   *electpercentage;   // default is 55%, set to 0 to disable elections
cvar_t   *electduration;      // duration of an election (seconds)
cvar_t   *electreminders;   // number of reminders to send in an election
cvar_t   *electallowveto;   // whether a single NO vote can veto the election
cvar_t   *electstarts;      // the number of times per map a player can start an election
cvar_t   *electautoyes;      // configuration to allow automatic yes vote from player who starts it

voting_t   voting;

// Called by InitGame()
int Voting_InitVars(void)
{
   //voting cvar default initializations
   electpercentage = gi.cvar("electpercentage", "55", 0);   //QW// passing percentage for voting
   electduration = gi.cvar("electduration", "30", 0);   //QW// duration of election (seconds)
   electreminders = gi.cvar("electreminders", "3", 0);   //QW// number of reminders to send
   electallowveto = gi.cvar("electallowveto", "1", 0); //QW// boolean whether a NO vote vetoes the election
   electstarts = gi.cvar("electstarts", "2", 0);      //QW// the number of times a player can start an election
   electautoyes = gi.cvar("electautoyes", "0", 0);      //QW// whether the initiator automatically votes yes or not
   basedir = gi.cvar ("basedir", "", CVAR_NOSET);      // expose this cvar but mod can't change it
   return true;
}

/**********************************************************
* Example command(Add to ClientCommand):
   case 'v':
      if (Q_stricmp(cmd, "vote") == 0) {      // command is gi.argv(0)
         if (Q_stricmp(gi.argv(1), "map") == 0)   // gi.argv(1) determines election type
            Voting_BeginElection(ent, ELECT_MAP);   // set the vote type flag and who started it.
         else if (Q_stricmp(gi.argv(1), "yes") == 0)   // 'vote yes' command
            Voting_CmdVote_f(ent, YES);
         else if (Q_stricmp(gi.argv(1), "no") == 0)   // 'vote no' command
            Voting_CmdVote_f(ent, NO);
         else
            Cmd_NotRecognized(ent);   //bad subcommand
      }
      else if (Q_stricmp (cmd, "ver") == 0)
         Cmd_Ver_f (ent);
      else
         Cmd_NotRecognized(ent);
      break;
***********************************************************/

// called from ClientCommand()
// ent is player who started the vote
// type is set for different vote categories:
qboolean Voting_BeginElection(edict_t *ent, elect_t type)
{
   char *mapname;
   char mappath[MAX_QPATH];
   char msg[256];
   int time_in;
   
   if (electpercentage->value == 0) {
      gi.cprintf(ent, PRINT_HIGH, "Elections are disabled.\n");
      return false;
   }
   
   if (voting.election != ELECT_NONE) {
      gi.cprintf(ent, PRINT_HIGH, "Election already in progress.\n");
      return false;
   }
   
   // prevent vote cheating after a level change
   if (level.time < 10.0) {
      gi.cprintf(ent, PRINT_HIGH, "Too soon to start a vote, %0.0f seconds left.\n", 10.0f - level.time);
      return false;
   }
   
   // count players (preliminary count)
   voting.count = Voting_CountPlayers();
   
   // prevent new players from coming in and starting a vote right away
   // use prelim. count to decide if solo player can vote himself a new map
   time_in = level.framenum - ent->client->resp.enterframe;
   if (time_in < 450 && voting.count > 1) { // use the preliminary count (450 frames: 45 seconds)
      gi.cprintf(ent, PRINT_HIGH,
         "New players can't start an election. You have %i seconds to wait.\n",
         (450 - time_in)/10);
      return false;
   }
   
   // Time enough to rejoin all clients, clear votes & re-count players
   voting.count = Voting_CountPlayers();
   
   if (voting.count < MIN_VOTERS) {
      gi.cprintf(ent, PRINT_HIGH, "Not enough players for election.\n");
      return false;
   }
   
   //
   // map voting stuff starts here
   //
   mapname = gi.argv(2);   // argv(0) is vote, argv(1) is map, argv(2) is mapname
   
   if (type == ELECT_MAP)
   {
      if (strlen(mapname) > 64) {
         gi.cprintf(ent, PRINT_HIGH, "Map name too long.\n");
         return false;
      }
      
      if (strstr(mapname, ".")) { // no dots allowed
         gi.cprintf(ent, PRINT_HIGH, "Do not use an extension in the map name.\n");
         return false;
      }
      
      // if argv[2] is missing or contains invalid filename characters
      if (mapname[0] == 0 || strstr(mapname, "\\") || strstr(mapname, "/") ||
         strstr(mapname, ";") || strstr(mapname, ",")) {
         gi.cprintf(ent, PRINT_HIGH, "Invalid input.\n");
         return false;
      }
      
      // a user wants next map in rotation
      // we hope there are no maps named 'next' :)
      if (strstr(mapname, "next") && dmflags->value && maplist->value != 0) {
         if (Voting_PeekMaplist()) 
		 {
            strcpy (mapname, level.nextmap);
            type = ELECT_NEXTMAP;
			return true;
         }
      }
      
      // if item isn't a stock map, check for the existence of a map file (bsp)
      if (!Maplist_CheckStockmaps(mapname) && !Maplist_CheckFileExists(mapname)) {
         gi.cprintf(ent, PRINT_HIGH, "Map %s does not exist on this server.\n", mapname);
         return false;
      }

      // player only gets two shots per map to start a vote, this stops vote spamming   
      if (voting.etarget == ent && ent->client->resp.votes_started >= electstarts->value) {
         gi.cprintf(ent, PRINT_HIGH, "You can't start another vote yet.\n");
         return false;
      }
      
      strncpy(voting.elevel, mapname, sizeof(voting.elevel) - 1);
      sprintf (mappath, "%s.bsp", voting.elevel);
   }
   
   // end of map vote preliminaries
   
   /* other vote modes would be done here. */
   
   // catch undefined election setup codes (stub)
   else if (type > ELECT_NEXTMAP) // last supported mode
   {
      gi.cprintf(ent, PRINT_HIGH, "Other vote modes not implemented.\n");
      return false;
   }
   
   //proceed with election
   voting.etarget = ent;
   voting.election = type;
   voting.evotes = voting.yesvotes = voting.novotes = 0;
   ent->client->resp.votes_started++;   // count how many times this player started a vote
   
   // initial announcement of a map vote
   if (voting.election == ELECT_MAP || voting.election == ELECT_NEXTMAP) {
      sprintf(msg, "%s started a vote to change the map to ", ent->client->pers.netname);
   }
   
   //bounds checks for cvars used in the voting functions
   if (electpercentage->value < 0) gi.cvar_set("electpercentage", "0"); // 0 means never run elections
   if (electpercentage->value > 100) gi.cvar_set("electpercentage", "100"); // unanimous
   if (electduration->value < 10) gi.cvar_set("electduration", "10");
   if (electduration->value > 120) gi.cvar_set("electduration", "120");
   if (electreminders->value < 1) gi.cvar_set("electreminders", "1");
   if (electreminders->value > 6) gi.cvar_set("electreminders", "6");
   if (electstarts->value < 1) gi.cvar_set("electstarts", "1");
   if (electstarts->value > 6) gi.cvar_set("electstarts", "6");
   
   voting.needvotes = (int)(voting.count * electpercentage->value) / 100 + 1;
   voting.electstarttime = level.time;
   voting.electtime = level.time + electduration->value; // duration of an election
   voting.remindtime = level.time + electduration->value/electreminders->value; // reminders for election votes
   strncpy(voting.emsg, msg, sizeof(voting.emsg) - 1);
   
   if(electautoyes->value)
      Voting_CmdVote_f (ent, YES);   // register initiator's yes vote
   
   // tell everyone a map vote is in progress
   if (voting.election == ELECT_MAP || voting.election == ELECT_NEXTMAP)
      gi.bprintf(PRINT_CHAT, "%s%s\n", voting.emsg, mapname);
   
   // other initial messages here
   
   // Initial voting message
   gi.bprintf(PRINT_HIGH, "Type YES or NO in the console to vote on this request.\n");
   gi.bprintf(PRINT_CHAT, "Votes: Yes: %d No: %d Needed: %d  Time left: %ds\n",
      voting.yesvotes, voting.novotes, voting.needvotes,
      (int)(voting.electtime - level.time));
   
   return true;
}

// terminate election when time expires and we still don't have a winner
void Voting_CheckVoting(void) // called by CheckDMRules()
{
   if (voting.election != ELECT_NONE && voting.electtime <= level.time) {
      gi.bprintf(PRINT_CHAT, "Election timed out.\n");
      voting.election = ELECT_NONE;
   }
   
   //test if a single no vote has veto power to stop an election
   if (voting.election != ELECT_NONE && electallowveto->value && voting.novotes) {
      gi.bprintf(PRINT_CHAT, "Election was defeated.\n");
      voting.election = ELECT_NONE;
   }
   
   // a win is mathematically impossible, terminate election
   if (voting.election != ELECT_NONE
      && (voting.needvotes - voting.yesvotes) > voting.count - (voting.yesvotes + voting.novotes)) {
      gi.bprintf(PRINT_CHAT, "Election was defeated. %d YES to %d NO\n", voting.yesvotes, voting.novotes);
      voting.election = ELECT_NONE;
   }
   
   // if in progress, post a reminder to vote in case anyone missed it
   if ((voting.election == ELECT_MAP  || voting.election == ELECT_NEXTMAP)
      && voting.remindtime <= level.time) {
      gi.bprintf(PRINT_CHAT, "%s%s\n", voting.emsg, voting.elevel);
      gi.bprintf(PRINT_HIGH, "Type YES or NO in the console to vote on this request.\n");
      gi.bprintf(PRINT_CHAT, "Votes: Yes: %d No: %d Needed: %d  Time left: %ds\n",
         voting.yesvotes, voting.novotes, voting.needvotes,
         (int)(voting.electtime - level.time));
      voting.remindtime = level.time + electduration->value/electreminders->value;
   }
}

// Used to kill the vote if level ends before the
// vote session succeeds or times out.
// Call this from EndDMLevel or whenever
// we need to terminate the vote process.
void Voting_KillVoting()
{
   voting.election = ELECT_NONE;
}

//
// The actual vote command function
// Called from ClientCommand
//
void Voting_CmdVote_f(edict_t *ent, int choice)
{
   
   if (voting.election == ELECT_NONE) {
      gi.cprintf(ent, PRINT_HIGH, "No election is in progress.\n");
      return;
   }
   
   if (ent->client->resp.voted) {
      gi.cprintf(ent, PRINT_HIGH, "You already voted.\n");
      return;
   }
   
   // prevent new players from voting in an election in progress unless they are only player
   if ((voting.electstarttime < ent->client->resp.entertime) && voting.count > 1) {
      gi.cprintf(ent, PRINT_HIGH, "New players can't vote in this election.\n");
      return;
   }
   
   switch (choice)
   {
   case YES:
      voting.evotes++;
      voting.yesvotes++;
      ent->client->resp.voted = true;
      if (voting.evotes == voting.needvotes) {
         // the election has been won
         Voting_WinElection();
      }
      break;
      
   case NO:
      voting.novotes++;
      ent->client->resp.voted = true;
      break;
      
   default:   // we should never get here, but you never know
      break;
   }
}

// Called by Voting_CmdVote_f
// announce result and terminate election, invoke the resulting commands
void Voting_WinElection(void)
{
   char command[MAX_QPATH];
   long n;
   char s[64];
   
   switch (voting.election) {
      
   case ELECT_MAP:   // vote map mapname
      
      gi.bprintf(PRINT_CHAT, "Map vote passed, %i YES to %i NO. Map is changing to %s.\n",
         voting.yesvotes,
         voting.novotes,
         voting.elevel);
      sprintf(command, "gamemap %s", voting.elevel);
      gi.AddCommandString (command);
      break;
      
   case ELECT_NEXTMAP:   // from vote map next
      
      gi.bprintf(PRINT_CHAT, "Map vote passed, %i YES to %i NO. Map is changing to %s.\n",
         voting.yesvotes,
         voting.novotes,
         voting.elevel);
      sprintf(command, "gamemap %s", voting.elevel);
      gi.AddCommandString (command);
      n = maplist->value;
      sprintf (s, "%ld", ++n);
      maplist = gi.cvar_set (maplist->name, s);
      break;
      
   default:
      break;
   }
   voting.election = ELECT_NONE;
}

int Voting_CountPlayers(void)
{
   int i, count;
   edict_t *ent;
   
   count = 0;
   for (i = 1; i <= maxclients->value; i++) {
      ent = g_edicts + i;
      if (ent->inuse) {   // count only active players
         count++;
         ent->client->resp.voted = false;
      }
   }
   return (count);
}

//this peeks at the next map in the maplist
qboolean Voting_PeekMaplist(void)
{
   long n;
   char s[32];
   
   if(Maplist_Next())
   {
      // MaplistNext increments counter so we set it
      // back in case the vote fails
      n = maplist->value;
      sprintf (s, "%ld", --n);
      maplist = gi.cvar_set (maplist->name, s);
      return true;
   }
   else
      return false;
}
