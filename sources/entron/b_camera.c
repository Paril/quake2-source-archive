/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_camera.c
  Description: Camera functions.

\**********************************************************/

#include "g_local.h"
#include "b_player.h"

#define CAM self->client->camera_model



void Cmd_camera_f(edict_t *self, edict_t *activator)
{
   edict_t *scan = &g_edicts[0];
 
   if (self->client->showinventory || self->client->showscores || self->client->showhelp)
      return;


   if (CAM)
      kill_camera(self);
   
   if (!activator)
      activator = scan;

   if (self->health <= 0)
      return;

   self->s.frame = FRAME_Stand_start;
   self->client->anim_end = FRAME_Stand_end; 
   VectorClear(self->velocity);
   if (self->client->resp.camera)
      scan = self->client->resp.camera + 1;

   for (;scan < &g_edicts[globals.num_edicts]; scan++)
      {
      if (!(scan->flipping & FLIP_CAMERA))
         continue;
      if (activator->team_data && (scan->team_data != activator->team_data))
         continue;
      self->client->resp.camera = scan;
      goto skiprestscan;
      }  
  self->client->resp.camera = NULL;
  return;

skiprestscan:
   self->client->ps.pmove.pm_type = PM_FREEZE;
   CAM = G_Spawn();
   CAM->master = self;
   CAM->classname = "Camera Body";
   CAM->movetype = MOVETYPE_NONE;
   CAM->clipmask = MASK_SHOT;
   CAM->takedamage = DAMAGE_NO;
   CAM->solid = SOLID_NOT;      
   CAM->master = self;
   gi.linkentity(CAM);

   VectorCopy (self->s.origin, self->client->camera_model->s.origin);
   VectorCopy (self->s.angles, self->client->camera_model->s.angles);
   VectorCopy (self->velocity, self->client->camera_model->velocity);

   self->client->camera_model->s.modelindex = self->s.modelindex;
   self->client->camera_model->s.modelindex2 = self->s.modelindex2;
   self->client->camera_model->s.modelindex3 = self->s.modelindex3;
   self->client->camera_model->s.modelindex4 = self->s.modelindex4;
   self->client->camera_model->s.frame = self->s.frame;
   self->client->camera_model->s.skinnum = self->s.skinnum;
   self->client->camera_model->s.effects = self->s.effects;
   self->client->camera_model->s.renderfx = self->s.renderfx;
   gi.linkentity (self->client->camera_model);
   self->client->ps.gunindex = 0;
}


void camera_think (edict_t *self)
{
   if ((self->s.angles[self->style] < 
      (self->count + self->move_angles[0]))
      || 
      (self->s.angles[self->style] > 
      (self->count + self->move_angles[1])))
      self->move_angles[2] = -self->move_angles[2];
   self->s.angles[self->style] += self->move_angles[2];
   self->nextthink = level.time + self->speed;
}

void SP_func_camera (edict_t *ent)
{
   ent->takedamage = DAMAGE_NO;  //AIM
   ent->movetype = MOVETYPE_FLY;
	ent->solid = SOLID_NOT;
   ent->style = (int) ent->style;
   ent->flipping |= FLIP_CAMERA;
   if (ent->style >= 0 && ent->style < 3 && ent->move_angles[2] && ent->speed)
      {
      ent->count = (int) ent->s.angles[ent->style];
      ent->think = camera_think;
      ent->nextthink = level.time + 3;
      ent->speed = 1/ent->speed;
      }
   gi.linkentity(ent);
}

void kill_camera (edict_t *ent)
{
   ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	if (ent->client->pers.weapon)
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);

   if (ent->client->resp.camera)
      if (ent->client->resp.camera->think && ent->client->resp.camera <= 0)
         ent->client->resp.camera->nextthink = level.time + 16000;
   
	if (ent->client->camera_model)
      {
      G_FreeEdict(ent->client->camera_model);
      ent->client->camera_model = NULL;
      ent->client->ps.pmove.pm_type = PM_NORMAL;
      }

   if (ent->status & STATUS_3RD_VIEW)
      {
      G_FreeEdict(ent->client->resp.camera);
		ent->client->resp.camera = NULL;
      ent->status &= ~STATUS_3RD_VIEW;
      }
   else if (ent->status & STATUS_FREE_CAM)
      {
      G_FreeEdict(ent->client->resp.camera);
		ent->client->resp.camera = NULL;
	   ent->status &= ~STATUS_FREE_CAM;
      }

   if (ent->status & STATUS_SCENE_VIEW)
      {
	   char	command[256];
   	Com_sprintf (command, sizeof(command), "cl_blend 1\n");
	   gi.AddCommandString (command);
      if (!(ent->client->resp.camera->flipping & FLIP_CAMERA))
			{
			G_FreeEdict(ent->client->resp.camera);
			ent->client->resp.camera = NULL;
			}
      ent->status &= ~STATUS_SCENE_VIEW;
      }
   
}

void camera_3rd (edict_t *ent)
{
      
   edict_t *camera;

   if (ent->client->showinventory || ent->client->showscores || ent->client->showhelp)
      return;

   if (ent->client->camera_model)
      {
      kill_camera(ent);
      return;
      }

   if (ent->health <= 0)
      return;

   camera = G_Spawn();
   camera->movetype = MOVETYPE_NONE;
   camera->master = ent;
   VectorSet(camera->mins, -16, -16, -24);
   VectorSet(camera->maxs, 16, 16, 32);
   ent->status |= STATUS_3RD_VIEW;

   ent->client->camera_model = G_Spawn();
   ent->client->camera_model->master = ent;
   ent->client->camera_model->movetype = MOVETYPE_NONE;
   ent->client->camera_model->clipmask = MASK_SHOT;
   ent->client->camera_model->takedamage = DAMAGE_NO;
   ent->client->camera_model->solid = SOLID_NOT;      
   ent->client->camera_model->master = ent;
   gi.linkentity(ent->client->camera_model);

   VectorCopy (ent->s.origin, ent->client->camera_model->s.origin);
   VectorCopy (ent->s.angles, ent->client->camera_model->s.angles);
   VectorCopy (ent->velocity, ent->client->camera_model->velocity);

   ent->client->camera_model->s.modelindex = ent->s.modelindex;
   ent->client->camera_model->s.modelindex2 = ent->s.modelindex2;
   ent->client->camera_model->s.modelindex3 = ent->s.modelindex3;
   ent->client->camera_model->s.modelindex4 = ent->s.modelindex4;
   ent->client->camera_model->s.frame = ent->s.frame;
   ent->client->camera_model->s.skinnum = ent->s.skinnum;
   ent->client->camera_model->s.effects = ent->s.effects;
   ent->client->camera_model->s.renderfx = ent->s.renderfx;
   gi.linkentity (ent->client->camera_model);

   ent->client->resp.camera = camera;
   ent->client->ps.gunindex = 0;
   gi.linkentity(camera);
}

void camera_think_3rd (edict_t * self)
{
   vec3_t o, ownerv, goal;
	edict_t *targ;
	vec3_t forward, right;
	trace_t trace;
	vec3_t oldgoal;
	vec3_t angles;

	targ = self->master;

	VectorCopy(targ->s.origin, ownerv);
	VectorCopy(self->s.origin, oldgoal);

	ownerv[2] += targ->viewheight;

	VectorCopy(targ->client->v_angle, angles);
	if (angles[PITCH] > 56)
		angles[PITCH] = 56;
	AngleVectors (angles, forward, right, NULL);
	VectorNormalize(forward);
	VectorMA(ownerv, -130, forward, o);

	if (o[2] < targ->s.origin[2] + 20)
		o[2] = targ->s.origin[2] + 20;

	// jump animation lifts
	if (!targ->groundentity)
		o[2] += 16;

	trace = gi.trace(ownerv, targ->mins, targ->maxs, o, targ, MASK_SOLID);

	VectorCopy(trace.endpos, goal);

	VectorMA(goal, 2, forward, goal);

	// pad for floors and ceilings
	VectorCopy(goal, o);
	o[2] += 6;
	trace = gi.trace(goal, self->mins, self->maxs, o, targ, MASK_SOLID);
	if (trace.fraction < 1) 
      {
		VectorCopy(trace.endpos, goal);
		goal[2] -= 6;
	   }

	VectorCopy(goal, o);
	o[2] -= 6;
	trace = gi.trace(goal, self->mins, self->maxs, o, targ, MASK_SOLID);
	if (trace.fraction < 1) 
      {
		VectorCopy(trace.endpos, goal);
		goal[2] += 6;
	   }

   //VectorCopy(vec3_origin, targ->client->ps.viewangles);
   VectorCopy(goal, self->s.origin);
   //gi.linkentity(self);
   /*
   VectorSet (offset, -70, 0, 24); 
   AngleVectors (self->master->client->v_angle, forward, right, NULL);
   G_ProjectSource (self->master->s.origin, offset, forward, right, end);
   trace = gi.trace (self->master->s.origin, self->mins, self->maxs, 
           end, self->master, MASK_SOLID);
   if (trace.fraction < 1)
      {
      VectorSet (offset, 20, 0, -12);
	   G_ProjectSource (trace.endpos, offset, forward, right, end);
      }
   VectorCopy(end, self->s.origin);
   */
}


void camera_enemy (edict_t *ent, edict_t *user);

void camera_think_scene (edict_t * self)
{
	if (self->decel > 0)
	   self->decel -= self->accel;
	else
	   self->decel = 0;
}

void camera_enemy(edict_t *ent, edict_t *user)
{
   edict_t *other = g_edicts;
   int found = 0;
   if (user->client->showinventory || user->client->showscores || user->client->showhelp)
      return;

   if (user->client->camera_model)
      {
      kill_camera(user);
      return;
      }

	if (ent != user)
		{
		while (other < &g_edicts[globals.num_edicts] && !found)
			{           
			if (other->inuse)
	         if (other->monsterinfo.run && other->targetname)
		         if (!Q_stricmp(ent->target, other->targetname))
			         found = 1;
			if (!found)
	         other++;
			}
		}
	else
		{
		while (other < &g_edicts[globals.num_edicts] && !found)
			{           
			if (other->inuse)
	         if (other->monsterinfo.run)
		         if (other->enemy, user)
			         found = 1;
			if (!found)
	         other++;
			}
		}
   
   if (!found)
      return;

   if (user->health <= 0)
      return;

   user->client->camera_model = G_Spawn();
   user->client->camera_model->master = user;
   user->client->camera_model->classname = "Camera Body";
   user->client->camera_model->movetype = MOVETYPE_NONE;
   user->client->camera_model->clipmask = MASK_SHOT;
   user->client->camera_model->takedamage = DAMAGE_NO;
   user->client->camera_model->solid = SOLID_NOT;      
   user->client->camera_model->master = user;
   gi.linkentity(user->client->camera_model);

   VectorCopy (user->s.origin, user->client->camera_model->s.origin);
   VectorCopy (user->s.angles, user->client->camera_model->s.angles);
   VectorCopy (user->velocity, user->client->camera_model->velocity);

   user->client->camera_model->s.modelindex = user->s.modelindex;
   user->client->camera_model->s.modelindex2 = user->s.modelindex2;
   user->client->camera_model->s.modelindex3 = user->s.modelindex3;
   user->client->camera_model->s.modelindex4 = user->s.modelindex4;
   user->client->camera_model->s.frame = user->s.frame;
   user->client->camera_model->s.skinnum = user->s.skinnum;
   user->client->camera_model->s.effects = user->s.effects;
   user->client->camera_model->s.renderfx = user->s.renderfx;
   gi.linkentity (user->client->camera_model);

   user->client->ps.gunindex = 0;
   user->client->resp.camera = other;
}

void camera_terminal_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   if (!other->client)
      return;
   if (self->touch_debounce_time > level.time)
      return;
   self->touch_debounce_time = level.time + 1.5;
   VectorClear(other->velocity);
	if (!self->target)
		Cmd_camera_f(other, self);
	else
		camera_enemy(self, other);
}

void camera_terminal_off (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
   if (!other->client)
      return;
   if (self->touch_debounce_time > level.time)
      return;
   self->touch_debounce_time = level.time + 1.5;
   if (self->message)
      gi.centerprintf(other, self->message);   
}

void camera_terminal_use (edict_t *self, edict_t *other, edict_t *activator)
{      
  self->touch = camera_terminal_touch;
}

void SP_func_camera_terminal (edict_t *ent)
{
   ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_BSP;
   ent->use = camera_terminal_use;
   ent->touch = camera_terminal_off;
   gi.setmodel(ent, ent->model);
   if (ent->spawnflags & 1)
      ent->use (ent, NULL, NULL);
   gi.linkentity(ent);
}






