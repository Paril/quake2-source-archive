//
// Q_DEVELS.C - cool support functions for Quake II development
// Version 1.5 (last updated Jan 29, 1998)
// 
// Published at http://www.planetquake.com/qdevels
// Code by various authors, released by SumFuka@planetquake.com
//
// Please use this code in your mods... if you distribute it,
// KEEP THE AUTHOR'S NAMES with the code. They deserve the credit.
//

//
// INSTRUCTIONS :
//
// To use the functions in this file, first add the file to your project.
// Then add '#include "q_devels.h"' to the top of every source file you
// need to call the functions from, or even better, put that same line
// in g_local.h.
//
// Compile away, rock and roll.
//


#include "g_local.h"
#include "q_devels.h"


// 1. FOR_EACH_PLAYER by SumFuka
//
// Use this macro to quickly apply code to each player in the game
// ! you must supply arguments of type (edict_t *) and (int) !
// e.g. int i; edict_t *joe_bloggs;
//      for_each_player(joe_bloggs,i) { joe_bloggs->client->pers.health = 0; }

// ---=== The code is #define'd in header file qdevels.h ===---


// 2. STUFFCMD (author unkown...)
//
// Use this function to send a command string to a CLIENT.
// E.g. stuffcmd(player, "alias ready \"cmd ready\"\n");

void stuffcmd(edict_t *e, char *s) {
        gi.WriteByte (11);
        gi.WriteString (s);
        gi.unicast (e, true);
}


// 3. ENT_BY_NAME by Kevin Sullivan (Ignitor)
//
// Here is a nice little function I wrote to find a player's
// entity structure by his name.

edict_t *ent_by_name (char *target)
{
    int i;
    edict_t *targ=NULL;
    for (i=0;;i++)
     {
        if (i > globals.num_edicts)
        return (NULL);

        targ = G_Find (targ, FOFS(classname), "player");

        if (strcmp(targ->client->pers.netname, target) == 0)
            return (targ);
     }
}



// 4. CENTERPRINT_ALL by SumFuka
//
// Use this function to centerprint to all players.
// e.g. centerprint_all("---<<< FIGHT ! >>>---\n");

void centerprint_all (char *msg)
{
	int i;
	edict_t *joe_bloggs;

	for_each_player(joe_bloggs,i)
	{
		gi.sound(joe_bloggs,CHAN_AUTO,gi.soundindex("misc/talk1.wav"),1,ATTN_NORM,0);
		gi.centerprintf (joe_bloggs, msg);
	}
}



// 5. RNDNUM by RoJoMo7
// 
// Returns a random number between "y" and "z".
//		y = lower limit of number to generate.
//		z = upper limit of number to generate.
// e.g. rndnum (10,20);   -- returns a random number between 10 an 20.

// ---=== The code is #define'd in header file qdevels.h ===---



// 6. RANDOM_PLAYER by SumFuka
//
// Select a random player, supply NULL for any player,
// or supply a player to exclude the player from being chosen.
//
// e.g. gi.centerprintf (random_player(NULL), "You're it !!!");
// e.g. gi.centerprintf (random_player(ent), "You're it !!!");

edict_t *random_player (edict_t *notme)
{
	int i;
	int count;
	int random_player;
	edict_t *joe_bloggs;

	// count the number of players
	count = 0;
	for_each_player(joe_bloggs,i)
	{
		if (joe_bloggs != notme)
			count++;
	}

	// no players ?
	if (count == 0)
	{
		gi.dprintf("ERROR: tried to select a random player when none are available.\n");
		return NULL;
	}
	
	// select a random player
	random_player = rand() % count;

	// find the randomly selected player
	count = 0;
	for_each_player(joe_bloggs,i)
	{
		if (joe_bloggs != notme)
		{
			if (count == random_player)
				return joe_bloggs;
			else
				count++;
		}
	}
	return NULL;
} 


// 7. Item lists by smith57@airmail.net
//
// These are the indexes into the entity->client->pers->inventory and 
// the itemlist array.  You could use ITEM_INDEX( FindItem( <item string ) )
// call, but i find that a stupid waste of cycles for finding an index
// into an array that does not change....
// (unless you add an item or id moves them around)
//
// [Use with 3.05 thru 3.10 code base.]

// ---=== The code is #define'd in header file qdevels.h ===---
