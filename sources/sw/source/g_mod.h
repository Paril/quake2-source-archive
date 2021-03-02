
#include "m_map.h"

// g_mod.h contains definitions and prototypes needed for 
// Strogg mod

#ifndef __MOD_DEF_H
#define __MOD_DEF_H

typedef	unsigned short int		sound_t;
typedef	unsigned short int		flash_t;
typedef	unsigned short int		frame_t;
typedef			 short int		short_t;
typedef	unsigned short int		usi_t;	

/*
typedef struct
{
	frame_t			firstframe;
	frame_t			lastframe;
	mframe_t	*frame;
	void		(*endfunc)(edict_t *self);
} mmove_t;
*/

// power armor types
#define	POWER_ARMOR_MEGA	3

// monster AI flags
#define AI_SLIDE				0x00008000


// range
#define RANGE_CLOSE				4


void ai_run_slide(edict_t *self, float distance);
void ai_run_strafe(edict_t *self, float distance);
void predictTargPos (edict_t *self, vec3_t source, vec3_t targPos, vec3_t targVel, float speed, qboolean feet);
void FoundTarget (edict_t *self);
void ED_CallSpawn (edict_t *ent);
void AI_AlertStroggs (edict_t *self);
qboolean Dijkstra_ShortestPath (counter_t source, counter_t target);
void find_path (edict_t *self);
void find_next_node (edict_t *self);
qboolean visible_from_weapon (edict_t *ent, vec3_t source);
qboolean M_CheckBottom_id (edict_t *ent);


#endif
