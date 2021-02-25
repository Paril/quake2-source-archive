#ifndef GLOOM_PMOVE
//You need to include gloompmove.h after including where pmove_t was defined.
#define GLOOM_PMOVE

#include <stdint.h>

#define PMF_GLOOM_PMOVE 128

#define PM_WRAITH  8
#define PM_JETPACK 9

enum { //gloom_pmove_state_t->flags
  GPMF_GRUNTBOX_SMALL = 1,  //classes smaller than a grunt get grunt sized box
  GPMF_GRUNTBOX_BIG   = 2,  //classes bigger than a grunt get grunt sized box
  GPMF_NO_SPEED_DIFF  = 4,  //all classes move at same speed
  GPMF_MAX_SPEED_DIFF = 8,  //maximum speed difference (not recommended when clients without prediction are in game)
};

enum { //gloom_pmove_state_t->sounds
  GPMS_JUMP = 1
};

typedef struct {
  uint8_t gloom_class;
  uint8_t jump_index;
  uint8_t flags;
  uint8_t grapple_state;
  float   grapple_point[3];
  float   grapple_dir[3];
  float   grapple_hold[3]; //for client
  float   grapple_prestep;
} gloom_pmove_state_t;

typedef struct {
  pmove_t *pmove;      //original pmove data.
  float* ent_origin;   //Pointer to vec3_t  ent->s.origin
  float* ent_velocity; //Pointer to vec3_t  ent->velocity
  gloom_pmove_state_t*state;   //Pointer to client's state initialized in GloomPMoveInitState
  uint16_t layout_bits; //hack to use unused bits of layout for player movement
  uint8_t  sounds;      //GPMS_ sounds to emit
  
  void* self;
  
  qboolean (*Collide)(void*player, trace_t*other, qboolean lastchance);
     //A player just collided (position at collision place, velocity before blocking)
     //returns true if entity will fully block, false if should attempt to continue (will always block on second hit)
     //if lastchance is true, the player's velocity can be adjusted to the blocker so it wont block again and return false

  trace_t (*trace)(vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end, void* passent, int mask);
    //full trace

/*
  qboolean (*IsMapGeometry)(void*ent);
    //returns true if is a solid_bsp
*/
} gloom_pmove_t;

int GuessGloomClass(const char*skinname);
  //Provides gloom_class to pass to GloomPMoveInitState from the entity skin. Returns < 0 if class is unknown

uint8_t GloomPMoveInitState(gloom_pmove_t*gpm, int gloom_class, int flags);
  //Call this after the player's model changes (normally in client's CL_SetEntState), before next GloomPMove.
  //Returns nonzero on success, 0 if original pmove should be used.

void GloomPMoveUnsnap(gloom_pmove_t*gpm);
  //Converts pmove origin and velocity to ent_*

void GloomPMoveClientUnpack(gloom_pmove_t*gpm, int cs_layout_bits);
  //Call on clinets just before GloomPMove to unpack STAT_LAYOUT bits into gloom_pmove_state_t

uint8_t GloomPMove(gloom_pmove_t*gpm);
  //Call this instead of original Pmove if GloomPMoveInitState returned true
  //Returns nonzero on success, 0 if original pmove should be called for this frame.

void GloomPMoveSetGrapplePoint(gloom_pmove_state_t*s, float* vec3point, float* vec3dir, float prestep, float*holdpoint);
  //Call in response to a TE_MEDIC_CABLE_ATTACK with owner the predicted player
  //Sets the point towards which grapple will move to


uint8_t GloomPMoveIsAirAccelerateEnabled();
  //Client and server must provide this to check if air accelerate is enabled.
  //Client has to check CS_AIRACCEL configstring or return pmove.c's pm_airaccelerate value. Server uses sv_airaccelerate

#endif
