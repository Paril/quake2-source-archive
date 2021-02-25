/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_mreferee.c
  Description: AI/Physics for Referee

\**********************************************************/

#include "g_local.h"
#include "b_mreferee.h"

void find_node_hide (edict_t *self);
void referee_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void parse_command(edict_t *ent);
void script_touch(edict_t *ent, edict_t *other);
void referee_restore(edict_t *self);
void script_block(edict_t *self, edict_t *other);
void script_use(edict_t *self, edict_t *other, edict_t *activator);

mframe_t referee_frames_stand [] =
{
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL,
	scrBOT_stand, 0, NULL
};
mmove_t referee_move_stand = {FRAME_Stand_start, FRAME_Stand_end, referee_frames_stand, NULL};

void referee_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &referee_move_stand;
   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
}

void referee_restore(edict_t *self)
{
   self->monsterinfo.currentmove = self->monsterinfo.save;
}

mframe_t referee_frames_walk [] =
{
	scrBOT_run, 0,  NULL,
	scrBOT_run, 1,  NULL,
	scrBOT_run, 1,  NULL,
	scrBOT_run, 1,  NULL,
	scrBOT_run, 1,  NULL,
	scrBOT_run, 1,  NULL,
	scrBOT_run, 3,  NULL,
	scrBOT_run, 3,  NULL,
	scrBOT_run, 3,  NULL,
	scrBOT_run, 3,  NULL,
	scrBOT_run, 3,  NULL,
	scrBOT_run, 3,  NULL,
	scrBOT_run, 0,  NULL,
	scrBOT_run, 1,  NULL,
	scrBOT_run, 1,  NULL,
	scrBOT_run, 1,  NULL,
	scrBOT_run, 1,  NULL,
	scrBOT_run, 1,  NULL,
	scrBOT_run, 3,  NULL,
	scrBOT_run, 3,  NULL,
	scrBOT_run, 3,  NULL,
	scrBOT_run, 3,  NULL,
	scrBOT_run, 3,  NULL,
	scrBOT_run, 3,  NULL
};
mmove_t referee_move_walk = {FRAME_Walk_start, FRAME_Walk_end, referee_frames_walk, NULL};

void referee_walk (edict_t *self)
{
	self->monsterinfo.currentmove = &referee_move_walk;
}

mframe_t referee_frames_run [] =
{
	scrBOT_run, 24, NULL,
	scrBOT_run, 16, NULL,
	scrBOT_run, 7,  NULL,
	scrBOT_run, 24, NULL,
	scrBOT_run, 16, NULL,
	scrBOT_run, 7,  NULL
};
mmove_t referee_move_run = {FRAME_Run_start, FRAME_Run_end, referee_frames_run, NULL};

void referee_run (edict_t *self)
{
	self->monsterinfo.currentmove = &referee_move_run;
}

mframe_t referee_frames_lookup [] =
{
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL
};
mmove_t referee_move_lookup = {FRAME_Lookup_start, FRAME_Lookup_end, referee_frames_lookup, referee_restore};

void referee_lookup(edict_t *self)
{
   self->monsterinfo.save = self->monsterinfo.currentmove;
   self->monsterinfo.currentmove = &referee_move_lookup;
}


mframe_t referee_frames_cry [] =
{
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL,
	BOT_move, 0, NULL
};
mmove_t referee_move_cry = {FRAME_Cry_start, FRAME_Cry_end, referee_frames_cry, referee_restore};

void referee_cry(edict_t *self)
{
   self->monsterinfo.save = self->monsterinfo.currentmove;
   self->monsterinfo.currentmove = &referee_move_cry;
}


mframe_t referee_frames_button [] =
{
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL,
	BOT_move, 0,  NULL
};
mmove_t referee_move_button = {FRAME_Button_start, FRAME_Button_end, referee_frames_button, referee_restore};

void referee_button(edict_t *self)
{
   self->monsterinfo.save = self->monsterinfo.currentmove;
   self->monsterinfo.currentmove = &referee_move_button;
}


/*QUAKED monster_referee (1 .5 0) (-16 -16 -24) (16 16 32) Ambush Trigger_Spawn Sight
*/
void SP_referee (edict_t *self)
{
	if (deathmatch->value || !self->scriptfile)
	{
		G_FreeEdict (self);
		return;
	}
  
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	self->s.modelindex = gi.modelindex("models/monsters/referee/tris.md2");
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);

   if (!self->health)
	   self->health = 400;
	if (!self->gib_health)
		self->gib_health = -220;
	self->mass = 200;

   //referee_stand;
   self->monsterinfo.stand = referee_stand;
	self->monsterinfo.walk = referee_walk;
   self->monsterinfo.run = referee_run;
	self->monsterinfo.melee = NULL;
 	self->monsterinfo.idle = NULL;
   self->monsterinfo.action[0] = referee_lookup;
   self->monsterinfo.action[1] = referee_cry;
   self->monsterinfo.action[2] = referee_button;

   self->status = 0;
   self->type = TYPE_SLAVE_ACTOR;
   self->touch = referee_touch;
   self->blocked = script_block;

   self->monsterinfo.goalnode = self->monsterinfo.lastnode = NULL;
   self->enemy = self->oldenemy = NULL;

	gi.linkentity (self);
	
	self->monsterinfo.currentmove = &referee_move_stand;
   walkmonster_start (self);
   self->takedamage = DAMAGE_NO;
   self->use = script_use;
	self->monsterinfo.scale = MODEL_SCALE;
   self->type |= TYPE_MONSTER_ORGANIC;
   load_script(self);
}


void referee_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{

   if (ent->touch_debounce_time > level.time)
      return;
   ent->touch_debounce_time = level.time + .5;
   script_touch (ent, other);
}


