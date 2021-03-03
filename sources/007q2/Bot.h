
#define MAXNODES  10000

typedef struct {
  vec3_t Pt;
  union {
    vec3_t Tcourner;
    unsigned short linkpod[6]; };
  edict_t *ent;
  short index;
  short state;
} route_t;

extern route_t Route[MAXNODES];
extern int TotalRouteNodes;

#define MAXBOTS 18 // Number of name/skin entries

typedef  struct {
  char netname[21]; //netname
  char skin[64];    //skin
  int  ingame;      //spawned
  int  skill[6];    //parameters
} botinfo_t;

extern botinfo_t Bot[MAXBOTS+1];

#include "bfiles.h"

