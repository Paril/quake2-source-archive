#define DECAMPER_VERSION "DeCamper 1.0 12/16/98"

#define YELLOW_DEBUG 0

//#include "q_shared.h"


#define DC_SHELL_COLOR_NONE 0
#define DC_SHELL_COLOR_RED (RF_SHELL_RED)
#define DC_SHELL_COLOR_GREEN (RF_SHELL_GREEN)
#define DC_SHELL_COLOR_BLUE (RF_SHELL_BLUE)
#define DC_SHELL_COLOR_WHITE (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE)
#define DC_SHELL_COLOR_YELLOW (RF_SHELL_RED | RF_SHELL_GREEN)
#define DC_SHELL_COLOR_PURPLE (RF_SHELL_RED | RF_SHELL_BLUE)
#define DC_SHELL_COLOR_CYAN (RF_SHELL_GREEN | RF_SHELL_BLUE)

#define DC_VOTE_NONE 0
#define DC_VOTE_NOMINATED 1
#define DC_VOTE_YES 2
#define DC_VOTE_NO 3

#define DC_CAMPER_STATUS_NONE 0
#define DC_CAMPER_STATUS_NOMINATED 1
#define DC_CAMPER_STATUS_DECAMPED 2
#define DC_CAMPER_STATUS_IMMUNE 3

struct CamperVoter {
	edict_t *edict;
	struct CamperVoter *next, *prev;
	int vote;
};

struct VotingRegistry {
	// Linked list of voters
	struct CamperVoter *voters;

	// Voting config info
	int min_num_yes;
	int min_perc_yes;
	int vote_seconds;			// seconds a vote should take place for
	int vote_disabled_minutes;	// Number of minutes vote is disabled between nominations
	// Camper action info
	int decamp_time_perc, decamp_time_minutes;
	int immune_time;  // Number of seconds a (failed) voted Camper is immune from another vote
	int frag_reduce;
	int pickup_powerup;
	int replace_powerup;
	int replace_weapon;
	int replace_ammo;
	int damage_perc;
	int replace_armor;
	// Current vote info
	struct Camper *nominated_camper;
	int num_voters, yes_votes, no_votes;
	int next_nomination;	// Time of next allowable nomination
	float expiration;			// Time current vote expires
};

struct CamperInventoryItem {
	edict_t *edict;
	float expiration;
	struct CamperInventoryItem *next, *prev;
};

struct Camper {
	edict_t *edict;
	struct Camper *next, *prev;
	struct CamperInventoryItem *inventory;
	char *name;
	int status;
	float change_time;  // time to change/check status (if immune, remove immunity, etc.)
	int IP_address;   // this is so wrong, fix it.
};

void Cmd_DeCamp (edict_t *ent);
void EvaluateDeCamperState ();
void RenderCamperEffects (edict_t *ent);
int HasBeenDeCamped(edict_t *ent);
void CheckCamperStatus (edict_t *ent);
void RemoveAllCamperInfo (edict_t *ent);
void AddCamperInventoryItem (edict_t *ent, edict_t *item, int respawn_time);
int CamperCanPickupInventoryItem (edict_t *ent, edict_t *item);
void DeCamperEndLevel();
void DeCamperGreeting (edict_t *ent);
void InitializeDeCamper();

extern struct VotingRegistry CamperVotingRegistry;


/*

  SMC 12/15/98: *** NOTE ***  The comments below may be inaccurate with respect
                              to the actual implementation.



Camper voting system
--------------------

We need to support three different commands:

1) "nominate camper XXX"
2) "agree camper XXX"
3) "disagree camper XXX"   


On the nominate, I could set up a global variable that lists all the
clients at the time of nomination, in some "camper voting" structures.
We would have to be careful to remove clients that disconnect, etc.

In a "camper voting" structure we would have:
- pointer to the client object
- vote - 1 for nominated (implicit yes), 2 for yes, 3 for no, 0 for
hasn't voted yet

We also keep a "voting record" structure:
- number of clients who are participating in the vote (reduce when
  clients leave the game)
- min. num yes votes
- min. % votes
- how long to have decamping - either % of level time left, or
  absolute #minutes
- whether to allow Quad pickups       
- whether to immediately replace items that have been taken by a camper
- percentage of damage reduction by camper
- pointer to "camper structure" (see below)
- num yes votes
- num no votes
- time of vote expiration
- time of next allowable nomination
- number of frags to deduct on a DeCamped person

We keep a camper structure:
- pointer to camper client object
- camper name
- camper IP
- camper status: nominated, official, immune
- time left on camping (if server owner set percentage of level time,
  calculate it when the camper is chosen); or, if immune, time left for
  immunity to being voted a camper
- pointer to voting record structure
 
We have a list of camper structures and a single voting structure.
When someone nominates a camper, a new camper structure is created,
camper info is filled in, and status is set to "nominated."  Each time
someone votes, the voting record is updated, percentage/num of yes
votes is tallied.  If the camper is nominated, then a camper structure
is created and the info filled in.  If the camper tries to leave and
come back, we will have the name and the IP; we can print a "warning"
to all that XXX is a suspected camper so they can be on the lookout.

If a voting client leaves the game, their camper voting structure is
removed from the list, and their vote is removed from the voting
structure.

Once the voting time has expired (assuming the "yes" votes didn't win
earlier), the results are announced.  If yes, the person is DeCamped.
If no, they are made "immune" from DeCamping for X amount of time (to
prevent nuisance nominations).  Another person can't be nominated
until XXX number of seconds have passed (maybe 0).

*/

#if YELLOW_DEBUG
#define DPRINT0(m) {printf(m); fflush(stdout);}
#define DPRINT1(m, a) {printf(m, a); fflush(stdout);}
#define DPRINT2(m, a, b) {printf(m, a, b); fflush(stdout);}
#define DPRINT3(m, a, b, c) {printf(m, a, b, c); fflush(stdout);}
#define DPRINT6(m, a, b, c, d, e, f) {printf(m, a, b, c, d, e, f); fflush(stdout);}

#define BPRINT0(m) printf(m)
#define BPRINT1(m, a) printf(m, a)
#define BPRINT2(m, a, b) printf(m, a, b)
#define BPRINT3(m, a, b, c) printf(m, a, b, c)
#define BPRINT4(m, a, b, c, d) printf(m, a, b, c, d)

#define CPRINT0(c, m) printf(c, m)
#define CPRINT1(c, m, a) printf(c, m, a)
#define CPRINT2(c, m, a, b) printf(c, m, a, b)

#define PRINT0(m) printf(m)
#define PRINT1(m, a) printf(m, a)
#define PRINT2(m, a, b) printf(m, a, b)
#define PRINT3(m, a, b, c) printf(m, a, b, c)

#else

#define DPRINT0(m) gi.dprintf(m)
#define DPRINT1(m, a) gi.dprintf(m, a)
#define DPRINT2(m, a, b) gi.dprintf(m, a, b)
#define DPRINT3(m, a, b, c) gi.dprintf(m, a, b, c)
#define DPRINT6(m, a, b, c, d, e, f) gi.dprintf(m, a, b, c, d, e, f)

#define BPRINT0(m) gi.bprintf(PRINT_MEDIUM, m)
#define BPRINT1(m, a) gi.bprintf(PRINT_MEDIUM, m, a)
#define BPRINT2(m, a, b) gi.bprintf(PRINT_MEDIUM, m, a, b)
#define BPRINT3(m, a, b, c) gi.bprintf(PRINT_MEDIUM, m, a, b, c)
#define BPRINT4(m, a, b, c, d) gi.bprintf(PRINT_MEDIUM, m, a, b, c, d)

#define CPRINT0(c, m) gi.cprintf(c, PRINT_MEDIUM, m)
#define CPRINT1(c, m, a) gi.cprintf(c, PRINT_MEDIUM, m, a)
#define CPRINT2(c, m, a, b) gi.cprintf(c, PRINT_MEDIUM, m, a, b)

#define PRINT0(m) 
#define PRINT1(m, a) 
#define PRINT2(m, a, b) 
#define PRINT3(m, a, b, c) 

#endif
