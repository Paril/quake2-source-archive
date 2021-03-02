#include "g_local.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

DArray waitList;
DArray arenaList;

// Arena state
float gCountdownSince;
int gArenaState;

#define STATE_WAITING 0
#define STATE_COUNTDOWN 1 
#define STATE_FIGHTING 2

// the edict are all in a continuous chunk of memory.
// since ordering doesn't matter, we can compare by pointers
int edictCompare(const void *elem1, const void *elem2) {
	const edict_t **edict1, **edict2;

	edict1 = elem1;
	edict2 = elem2;	
	if (*edict1 == *edict2)
		return 0;
	else if (*edict1 > *edict2)
		return 1;
	else
		return -1;
}

void arenaInitLevel() {
	waitList = ArrayNew(sizeof(edict_t *), (int)maxclients->value, edictCompare);
	arenaList = ArrayNew(sizeof(edict_t *), (int)maxclients->value, edictCompare);
	gArenaState = STATE_WAITING;
}

void arenaEndLevel() {
	ArrayFree(waitList);
	ArrayFree(arenaList);
}

void setArenaStats(edict_t *fighter) {

	gclient_t *client = fighter->client;
	gitem_t		*item;

	// maxes
	client->pers.max_health		= 100;

	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 50;

	// armor and health
	client->pers.health			= 100;

	item = FindItem("Combat Armor");
	client->pers.inventory[ITEM_INDEX(item)] = 200;

	// weapons
	item = FindItem("Blaster");
	client->pers.inventory[ITEM_INDEX(item)] = 1;

	item = FindItem("Shotgun");
	client->pers.inventory[ITEM_INDEX(item)] = 1;

	item = FindItem("Super Shotgun");
	client->pers.inventory[ITEM_INDEX(item)] = 1;

	item = FindItem("Machinegun");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	
	item = FindItem("Chaingun");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	
	item = FindItem("Grenade Launcher");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	
	item = FindItem("Rocket Launcher");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	client->pers.selected_item = ITEM_INDEX(item);
	client->pers.weapon = item;
	
	item = FindItem("HyperBlaster");
	client->pers.inventory[ITEM_INDEX(item)] = 1;

	item = FindItem("Railgun");
	client->pers.inventory[ITEM_INDEX(item)] = 1;
	
	item = FindItem("BFG10K");
	client->pers.inventory[ITEM_INDEX(item)] = 1;

	// ammo
	item = FindItem("Shells");
	client->pers.inventory[ITEM_INDEX(item)] = 25;

	item = FindItem("Bullets");
	client->pers.inventory[ITEM_INDEX(item)] = 50;
	
	item = FindItem("Cells");
	client->pers.inventory[ITEM_INDEX(item)] = 40;
	
	item = FindItem("Grenades");
	client->pers.inventory[ITEM_INDEX(item)] = 5;
	
	item = FindItem("Slugs");
	client->pers.inventory[ITEM_INDEX(item)] = 5;
	
	item = FindItem("Rockets");
	client->pers.inventory[ITEM_INDEX(item)] = 10;

	
}

void arenaSpawn(edict_t *player) {
	if (player->client->resp.inArena) {
		// about to fight in arena
		player->movetype = MOVETYPE_WALK;
		player->solid = SOLID_BBOX;
		player->clipmask = MASK_PLAYERSOLID;
		player->svflags &= ~SVF_NOCLIENT;
		setArenaStats(player);
	} else {
		// observer
		player->movetype = MOVETYPE_NOCLIP; // goes through walls
		player->solid = SOLID_NOT; // no collisions with other ents
		player->clipmask = 0; // not stopped by other ents
		player->svflags |= SVF_NOCLIENT;
	}
	// pickArenaSpawn();
}
 
/**
 * Add a new player to the arena from the spectator waiting list.
 * Only here can a countdown start.
 */
void cycleArena() {

	edict_t *newChallenger;

	if (ArraySize(waitList) == 0) {
		// no one in the spectator waiting list.
		gArenaState = STATE_WAITING;
		return;
	}

	// grab a new challenger from the waitList
	newChallenger = *( (edict_t **)ArrayElementAt(waitList, 0) );
	ArrayDeleteAt(waitList, 0);

	// and put him in the arena
	ArrayAppend(arenaList, &newChallenger);
	newChallenger->client->resp.inArena = true;
	respawn(newChallenger);

	gi.bprintf(PRINT_HIGH, "%s steps into the arena\n", newChallenger->client->pers.netname);

	if (ArraySize(arenaList) > 1) {
		gArenaState = STATE_COUNTDOWN;
		gCountdownSince = level.time;
	} else {
		gArenaState = STATE_WAITING;
	}
}

void arenaConnect(edict_t *player) {

	// reconnect bug workaround: check if the player 
	// is already in one of the two lists
/*
	if (!ArrayContains(waitList, &player) &&
	    !ArrayContains(arenaList, &player)) {
*/
		// stick the new joiner in the waiting list
		player->client->resp.inArena = false;
		ArrayAppend(waitList, &player);
		if (ArraySize(arenaList) < 2) {
			// not enough players for a match currently in arena
			// send the new joiner into the arena
			cycleArena();
		}
/*
	} else {
		// reconnect bug in quake2/quakeworld engine
		gi.dprintf("Reconnect without disconnect, ignoring: %s\n", player->client->pers.netname);
	}
*/
	
}

/**
 * 
 */
void arenaDisconnect(edict_t *player) {
	if (player->client->resp.inArena) {
		ArrayDelete(arenaList, &player);
		// replace the disconnecting player in the arena
		cycleArena();
	} else {
		ArrayDelete(waitList, &player);
	}
}

/**
 *
 */
void arenaKilled(edict_t *victim) {

	// the player who was in the arena first (ie 
	// won the previous battle) will be first in the array
	if (victim == ArrayElementAt(arenaList, 0)) {
		// loser was the reigning champion
		gi.bprintf(PRINT_HIGH, "The champion, ");
	} else {
		// loser was the challenger
		gi.bprintf(PRINT_HIGH, "The challenger, ");
	}
	gi.bprintf(PRINT_HIGH, "%s, has been defeated\n", victim->client->pers.netname);
	
	// move the losing player back to the spectator waiting list
	ArrayDelete(arenaList, &victim);
	ArrayAppend(waitList, &victim);
	victim->client->resp.inArena = false;
	// FIXME make spectator

	// replace the losing player with a new challenger
	cycleArena();
}

/**
 * Plays sounds, etc, during the countdown to fighting
 * This is the only place actual fighting is allowed to begin
 */ 
void arenaCountdown() {

	int elapsed;

	if (gArenaState != STATE_COUNTDOWN) {
		return;
	}

	elapsed = level.time - gCountdownSince;

/*
	if (elapsed > 7 && elapsed <= 8) {

	} else if (elapsed > 8 && elapsed <= 9) {

	} else 
	*/
	if (elapsed > 10) {
		gArenaState = STATE_FIGHTING;	
		gi.bprintf(PRINT_HIGH, "Fight!!!\n");
	}
}

/**
 * In Arena mode, whether "player" is allowed to fire and take damage
 */
qboolean arenaCombatAllowed(edict_t *player) {
	return (player->client->resp.inArena && gArenaState == STATE_FIGHTING);
}
