//===========================================================================
// p_predator.h
//
// Predator functions.
//
// Original author: DingBat
//
//===========================================================================

#define PREDATOR_MAX_ROCKETS_DEFAULT "50"
#define PREDATOR_MAX_SLUGS_DEFAULT "20"
#define PREDATOR_MAX_CELLS_DEFAULT "100"
#define PREDATOR_START_ROCKETS_DEFAULT "10"
#define PREDATOR_START_SLUGS_DEFAULT "5"
#define PREDATOR_START_CELLS_DEFAULT "20"
#define PREDATOR_START_HEALTH_DEFAULT "200"

void initPredator(edict_t *ent) ;
void lookForPredator(edict_t *player) ;
void switchPredator(edict_t *ent) ;
void startPredator(edict_t *ent) ;
int countPlayers(edict_t *exclude) ;
edict_t *getPlayer(int num, edict_t *exclude) ;
void randomPredator(edict_t *exclude) ;
void quitPredator(edict_t *ent) ;

