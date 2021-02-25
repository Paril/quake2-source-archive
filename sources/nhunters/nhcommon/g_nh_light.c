//===========================================================================
// g_nh_light.c
//
// Simple lighting commands.
//
// Originally coded by: DingBat
//
//===========================================================================

#include "g_local.h"

qboolean
toggleStuffLight() {

  stuff_light = !stuff_light ;

  return stuff_light ;

}

qboolean stuffLight() {
  return stuff_light ;
}

void getUserVar(edict_t *ent, char *var) {

  char cmd[128] ;
  char varname[32] ;
  char *value ;

  sprintf(varname, "u_%s", var) ;
  sprintf(cmd, "set %s $%s u\n", varname, var) ;
  stuffcmd(ent, cmd) ;

} 

void
checkCheating(edict_t *ent, char *userinfo) {

  char *value ;

  value = Info_ValueForKey(userinfo, "u_gl_dynamic") ;
  if (value != NULL) {

    if (atoi(value) == 0) {

      ent->isCheating = true ;
    }
    else
      ent->isCheating = false ;
  }

}


