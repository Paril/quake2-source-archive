/**
 * e_arena.h
 *
 * Prototypes for the arena subsystem
 */

void arenaInitLevel();  // called when a new level is entered
void arenaEndLevel(); // called at the end of a level
void arenaConnect(edict_t *player); // called when a player connects
void arenaDisconnect(edict_t *player); // called when a player disconnects
void arenaKilled(edict_t *victim); // called when a player dies
void arenaSpawn(edict_t* player); // called by PutClientInServer during spawning/respawning
void arenaCountdown();
// determines whether a player can attack 
// and be damaged when in arena mode
qboolean arenaCombatAllowed();

