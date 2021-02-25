/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_scrcam.c
  Description: Scripted camera functions

\**********************************************************/

#include "g_local.h"
#include "b_player.h"

#define CAM self->client->camera_model

void camera_focus(edict_t *self);
void camera_target_think_3rd (edict_t * self);
void camera_target_think_rotate (edict_t * self);

void camera_focus(edict_t *self);
void camera_target_think_3rd (edict_t * self);
void camera_target_think_rotate (edict_t * self);

/*
Switch script to selected camera with a fade.
*/

void Show_camera_f(edict_t *self, int cam_number, float fade)
{
   edict_t *scan = &g_edicts[0];
   char command[256];
 
   if (self->client->showinventory || self->client->showscores || self->client->showhelp)
      return;

   if (CAM)
      kill_camera(self);
 
   if (self->health <= 0)
      return;

   self->s.frame = FRAME_Stand_start;
   self->client->anim_end = FRAME_Stand_end; 
   VectorClear(self->velocity);
   if (cam_number < 0)
      if (self->client->resp.camera)
         scan = self->client->resp.camera + 1;

   for (;scan < &g_edicts[globals.num_edicts]; scan++)
      {
      if (!(scan->flipping & FLIP_CAMERA))
         continue;
      if ((cam_number >= 0) && scan->team_data != cam_number)
         continue;
      self->client->resp.camera = scan;
      goto skiprestscan;
      }  
  self->client->resp.camera = NULL;
  return;

skiprestscan:
   self->client->resp.camera->decel = 1;   
   if (fade)
      {
      self->client->resp.camera->accel = fade;
      self->status |= STATUS_SCENE_VIEW;
		Com_sprintf (command, sizeof(command), "cl_blend 0\n");
	   gi.AddCommandString (command);
      }
   self->client->ps.pmove.pm_type = PM_FREEZE;
   self->client->ps.gunindex = 0;
   CAM = G_Spawn();
   CAM->master = self;
   CAM->classname = "Camera Body";
   CAM->movetype = MOVETYPE_NONE;
   CAM->clipmask = MASK_SHOT;
   CAM->takedamage = DAMAGE_NO;
   CAM->solid = SOLID_NOT;      
   CAM->master = self;
   if (scan)
      {
      if (scan->enemy)
         {  
         scan->think = camera_focus;
         scan->think(scan);
         }
      else if (scan->target)
         {
         edict_t *x = g_edicts;
         for (; x < &g_edicts[globals.num_edicts]; x++)
	         {
		      if (!x->inuse)
   			   continue;
            if (!x->targetname)
               continue;
		      if (!strcmpi(x->targetname, scan->target))
               {
               scan->enemy = x;
               x = &g_edicts[globals.num_edicts];
               scan->think = camera_focus;
               scan->think(scan);
               }
	         }
         }
      }

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
}

void camera_focus(edict_t *self)
{
   vec3_t forward;
   if (self->enemy)
      {
      VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
	   VectorNormalize (forward);
      vectoangles(forward, self->s.angles);
      self->nextthink = level.time + FRAMETIME;
      }
}


// DEMO:
void camera_target_3rd (edict_t *ent, edict_t *target)
{
      
   edict_t *camera;

   if (ent->client->showinventory || ent->client->showscores || ent->client->showhelp)
      return;

   if (ent->client->camera_model)
      kill_camera(ent);

   if (ent->health <= 0)
      return;

   camera = G_Spawn();
   camera->movetype = MOVETYPE_NONE;
   camera->master = ent;
   VectorSet(camera->mins, -16, -16, -24);
   VectorSet(camera->maxs, 16, 16, 32);

   ent->client->camera_model = G_Spawn();
   ent->client->camera_model->master = ent;
   ent->client->camera_model->movetype = MOVETYPE_NONE;
   ent->client->camera_model->clipmask = MASK_SHOT;
   ent->client->camera_model->takedamage = DAMAGE_NO;
   ent->client->camera_model->solid = SOLID_NOT;      
   ent->client->camera_model->master = ent;
   ent->status |= STATUS_FREE_CAM;
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

   camera->enemy = target;
   camera->speed = -90;
   camera->think = camera_target_think_3rd;
   camera->think(camera);
   gi.linkentity(camera);
}

void camera_target_think_3rd (edict_t * self)
{
	edict_t *targ;
	vec3_t forward, right, offset, end;
	trace_t trace;
	vec3_t angles;
   float x;

	targ = self->enemy;

   if (!targ)
      return;
  
   if (targ->enemy)
      {
      VectorSubtract (targ->enemy->s.origin, targ->s.origin, forward);
      VectorNormalize (forward);
      vectoangles(forward, angles);
      AngleVectors (angles, forward, right, NULL);
      if (self->speed > -40)
         self->speed += .3;
      if (angles[YAW] > self->s.angles[YAW] + 30)
         {
         angles[YAW] = self->s.angles[YAW] + 30;
         self->speed = -90;
         }
      else if (angles[YAW] < self->s.angles[YAW] - 30)
         {
         angles[YAW] = self->s.angles[YAW] - 30;
         self->speed = -90;
         }
      }
   else
      AngleVectors (targ->s.angles, forward, right, NULL);
   VectorSet (offset, -50, 0, 24); 
   G_ProjectSource (targ->s.origin, offset, forward, right, end);
   trace = gi.trace (targ->s.origin, targ->mins, targ->maxs, end, self->master, MASK_SOLID);
   if (trace.fraction == 1.0)
      x = self->speed;
   else
      x = -self->speed;
   
   VectorSet (offset, x, 0, 24);  
   G_ProjectSource (targ->s.origin, offset, forward, right, end);
   trace = gi.trace (targ->s.origin, targ->mins, targ->maxs, end, self->master, MASK_SOLID);
   if (trace.fraction < 1)
      {
      if (x < 0)
         VectorSet (offset, 20, 0, -12);
      else
         VectorSet (offset, -20, 0, -12);
	   G_ProjectSource (trace.endpos, offset, forward, right, end);
		if (gi.pointcontents(end))
			{
			end[0] = targ->s.origin[0];
			end[1] = targ->s.origin[1];
			end[2] = targ->s.origin[2] + 128;
			}
      }
   VectorCopy(end, self->s.origin);   
   VectorSubtract (self->enemy->s.origin, self->s.origin, forward);
   VectorNormalize (forward);
   vectoangles(forward, self->s.angles);
   gi.linkentity(self);

   self->nextthink = level.time + FRAMETIME;
}



// DEMO:
void camera_target_rotate (edict_t *ent, edict_t *target, float distance)
{
      
   edict_t *camera;

   if (ent->client->showinventory || ent->client->showscores || ent->client->showhelp)
      return;

   if (ent->client->camera_model)
      kill_camera(ent);

   if (ent->health <= 0)
      return;

   camera = G_Spawn();
   camera->movetype = MOVETYPE_NONE;
   camera->master = ent;
   VectorSet(camera->mins, -16, -16, -24);
   VectorSet(camera->maxs, 16, 16, 32);

   ent->client->camera_model = G_Spawn();
   ent->client->camera_model->master = ent;
   ent->client->camera_model->movetype = MOVETYPE_NONE;
   ent->client->camera_model->clipmask = MASK_SHOT;
   ent->client->camera_model->takedamage = DAMAGE_NO;
   ent->client->camera_model->solid = SOLID_NOT;      
   ent->client->camera_model->master = ent;
   ent->status |= STATUS_FREE_CAM;
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

   camera->enemy = target;
   camera->accel = 0;
   camera->speed = distance;
   camera->think = camera_target_think_rotate;
   camera->think(camera);
   gi.linkentity(camera);
}

void camera_target_think_rotate (edict_t * self)
{
	edict_t *targ;
	vec3_t forward, right, offset, end;
	trace_t trace;
	vec3_t angles;

	targ = self->enemy;

   if (!targ)
      return;
   angles[0] = angles[2] = 0;
   angles[YAW] = targ->s.angles[YAW] + self->accel;
   self->accel += 2;
   if (self->accel > 360) self->accel -= 360;

   AngleVectors (angles, forward, right, NULL);
   VectorSet (offset, self->speed, 0, 54);
   G_ProjectSource (targ->s.origin, offset, forward, right, end);
   trace = gi.trace (targ->s.origin, self->mins, self->maxs, end, self->master, MASK_SOLID);
   VectorCopy(end, self->s.origin);

   VectorSubtract (targ->s.origin, self->s.origin, forward);
   VectorNormalize (forward);
   vectoangles(forward, self->s.angles);
   gi.linkentity(self);

   self->nextthink = level.time + FRAMETIME;
}
