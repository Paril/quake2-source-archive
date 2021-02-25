//============================================================================
// g_safety.h
//
// Safety mode.
//
// Original Author: Bruce Rennie
//
//============================================================================

#define MARINE_SAFETY_TIME_DEFAULT "10"
#define PREDATOR_SAFETY_TIME_DEFAULT "2"
#define ENABLE_PREDATOR_SAFETY_DEFAULT "1"
#define ENABLE_MARINE_SAFETY_DEFAULT "1"

void setSafetyMode(edict_t *ent) ;
void clearSafetyMode(edict_t *ent) ;

void validateMarineSafetyTime();
void validatePredatorSafetyTime();

int getMarineSafetyTime();
