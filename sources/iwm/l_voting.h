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

/**************************************************/
/*                Quake 2 Voting                  */
/**************************************************/

#ifndef L_VOTING_H
#define L_VOTING_H

#define YES   1
#define NO   0

typedef enum {      // some election types
	ELECT_NONE,
		ELECT_MAP,      // change the map
		ELECT_NEXTMAP,   // go to next map in list
		ELECT_KICK,      // kick a player
		ELECT_OPTION,   // turn on/off an option
		ELECT_CONFIG,
		ELECT_MODE,
		ELECT_MATCH,
		ELECT_ADMIN,
} elect_t;

typedef struct voting_s
{
   elect_t   election;   // election type
   edict_t *etarget;   // who initiated the election
   char   elevel[MAX_QPATH];   // for map election, target level
   int      count;      // number of players at start of election
   int      evotes;      // votes so far
   int      yesvotes;   // yes vote count
   int      novotes;   // no vote count
   int      needvotes;   // votes needed
   float   electtime;   // remaining time until election times out
   float   remindtime;   // time remaining to next reminder to vote
   float   electstarttime; //the level.time the election was started
   char   emsg[256];   // election name

} voting_t;

extern cvar_t   *electpercentage;   // default is 55%, set to 0 to disable elections
extern cvar_t   *electduration;      // duration of an election (seconds)
extern cvar_t   *electreminders;   // number of reminders to send in an election
extern cvar_t   *electallowveto;   // whether a single NO vote can veto the election
extern cvar_t   *electstarts;      // the number of times per map a player can start an election
extern cvar_t   *electautoyes;      // configuration to allow automatic yes vote from player who starts it

#define MIN_VOTERS   1   // minimum number of players need for voting to proceed.
                  // 1 means a single player can vote himself a map.

//public
qboolean Voting_BeginElection(edict_t *ent, elect_t type);
void Voting_CmdVote_f(edict_t *ent, int choice);
void Voting_CheckVoting(void);
void Voting_KillVoting(void);
int Voting_InitVars(void);

//private
static void Voting_WinElection(void);
static int Voting_CountPlayers(void);
static qboolean Voting_PeekMaplist(void);

#endif
