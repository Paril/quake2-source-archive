//===========================================================================
// q_model.c
//
// Simple functions for predator model/skin handling.
//
// Original author: DingBat
//
//===========================================================================

#include "g_local.h"

// Cvar handling
void validatePredatorModel() {

  // Predator model must not be null.
  if ((predator_model->string == NULL) ||
      (strlen(predator_model->string) == 0))
    gi.cvar_set("predator_model", DEFAULT_PREDATOR_MODEL) ;
}

char *getPredatorModel() {
  if (predator_model->modified) 
    validatePredatorModel() ;

  return predator_model->string ;
}

void validatePredatorSkin() {

  // Predator skin must not be null.
  if ((predator_skin->string == NULL) ||
      (strlen(predator_skin->string) == 0))
    gi.cvar_set("predator_skin", DEFAULT_PREDATOR_SKIN) ;
}

char *getPredatorSkin() {
  if (predator_skin->modified) 
    validatePredatorSkin() ;

  return predator_skin->string ;
}

void validateMarineModel() {

  // Marine model must not be null.
  if ((marine_model->string == NULL) ||
      (strlen(marine_model->string) == 0))
    gi.cvar_set("marine_model", DEFAULT_MARINE_MODEL) ;
}

char *getMarineModel() {
  if (marine_model->modified) 
    validateMarineModel() ;

  return marine_model->string ;
}

void validateMarineSkin() {

  // Marine skin must not be null.
  if ((marine_skin->string == NULL) ||
      (strlen(marine_skin->string) == 0))
    gi.cvar_set("marine_skin", DEFAULT_MARINE_SKIN) ;
}

char *getMarineSkin() {
  if (marine_skin->modified) 
    validateMarineSkin() ;

  return marine_skin->string ;
}


void initSkins() {

  (void) setLivePredatorSkin(getPredatorModel(), getPredatorSkin()) ;
  setDefaultMarineSkin() ;
}

char *
setLivePredatorSkin(char *model, char *skin) {

  if ((model == NULL) ||
      (strlen(model) == 0) ||
      (skin == NULL) ||
      (strlen(skin) == 0) ||
      (strlen(model) + strlen(skin)) > SKINLEN) 
  {
    
    // Some wierd model or skin values. 
    // Default to female/jungle.
    sprintf(predatorSkin, "%s/%s",
	    DEFAULT_PREDATOR_MODEL, DEFAULT_PREDATOR_SKIN) ;
    strcpy(predatorModel, DEFAULT_PREDATOR_MODEL) ;
  }
  else {
    sprintf(predatorSkin, "%s/%s", model, skin) ;
    strcpy(predatorModel, model) ;
  }

  return predatorSkin ;

}  

char *
getLivePredatorSkin() {
  return predatorSkin ;
}

void 
setDefaultMarineSkin() {

  // Just in case the predator uses the male model.

  // Check the currently set predator skin, which MAY NOT be 
  // the same as the server variables. 
  if (strcmp(predatorSkin, getMarineModel()) == 0) {
    if (strcmp(getMarineModel(), "male") == 0) 
      sprintf(marineSkin, "female/jungle") ;
    else
      sprintf(marineSkin, "male/nightops") ;
  } 
  else
    sprintf(marineSkin, "%s/%s",
	    getMarineModel(), 
	    getMarineSkin()) ;
	    

}

void checkMarineSkin(edict_t *ent, char *userinfo)
{

  char *value ;
  char saveSkin[64] ;

  char model[32], skin[32] ;

  value = Info_ValueForKey(userinfo, "skin") ;
  strcpy(saveSkin, value) ;

  if ((!marine_allow_custom->value) ||
      (!parseSkin(value, model, skin))) {
    setMarineSkin(ent, marineSkin) ;
    return ;
  }

  // Do NOT use predator_model->value. This may not be the 
  // current default predator skin. Use predatorSkin. 
  if (strcmp(model, predatorModel) == 0) {
    setMarineSkin(ent, marineSkin) ;
  }
  else 
    setMarineSkin(ent, saveSkin) ; // Skin as specified by user.

}

void setMarineSkin(edict_t *ent, char *skin) {
  int playernum ;

  playernum = ent - g_edicts - 1 ;

  gi.configstring (CS_PLAYERSKINS + playernum,
		   va("%s\\%s", ent->client->pers.netname, skin) );

  return;

}  

qboolean parseSkin(char *fullSkin, char *model, char *skin) {

  char *tmp ;  
  size_t ind ;
  int i, j=0;
  

  // Initialize
  model[0] = '\0' ;
  skin[0] = '\0' ;

  // Look for model portion of skin.
  if (skin == NULL) 
    return false ;

  // Check for / in skin var
  if (strchr(fullSkin, '/') == NULL)
    return false ;
     	 
  // Get model
  tmp = fullSkin ;
  strcpy(model, strsep(&tmp, "/")) ;
  
  // Get skin
  strcpy(skin, strsep(&tmp, "\0")) ;

  return true ; 

}










