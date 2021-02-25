//===========================================================================
// q_model.h
//
// Some simple model/skin manipulation routines.
//
// Original author: DingBat
//
//==========================================================================

//#define DEFAULT_PREDATOR_MODEL "female"
//#define DEFAULT_PREDATOR_SKIN "jungle"
#define DEFAULT_PREDATOR_MODEL "nhpred"
#define DEFAULT_PREDATOR_SKIN "nhpred"

#define DEFAULT_MARINE_MODEL "male"
#define DEFAULT_MARINE_SKIN "nightops"

#define SKINLEN 64
char predatorModel[SKINLEN/2] ;
char predatorSkin[SKINLEN] ;
char marineSkin[SKINLEN] ;

// Cvar handling.
void validatePredatorModel() ;
char *getPredatorModel() ;
void validatePredatorSkin() ;
char *getPredatorSkin() ;

void validateMarineModel() ;
char *getMarineModel() ;
void validateMarineSkin() ;
char *getMarineSkin() ;  

void initSkins(void) ;
char *setLivePredatorSkin(char *, char *) ;
char *getLivePredatorSkin(void) ;

void setDefaultMarineSkin(void) ;
void checkMarineSkin(edict_t *ent, char *userinfo) ;
char *getMarineSkin(void) ;
void setMarineSkin(edict_t *ent, char *) ;
qboolean parseSkin(char *full, char *model, char *skin) ;
