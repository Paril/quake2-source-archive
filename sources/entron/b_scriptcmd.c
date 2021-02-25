/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_scriptcmd.c
  Description: Script command functions 

\**********************************************************/

#include "g_local.h"
#include "b_script.h"

void parse_command(edict_t *ent);

extern int inscript;

void script_touch(edict_t *ent, edict_t *other)
{
   int cmd = ent->scr.touch_cmd;   
   if (ent->scr.on_touch < 0)
      return;
   if (ent->scr.touch_time > level.time)
      return;
	if (ent->scr.commands[cmd].ParamInt[0] != 2)
		{
	   if (ent->scr.commands[cmd].ParamInt[0])
		   {
			if (!other->client)
				return;
			}
		else 
			{
			if (!other->targetname)
				return;
			if (strcmpi(ent->scr.commands[cmd].ParamString[0], other->targetname))
	         return;
			}
		}
   ent->scr.activator = other;
   ent->scr.touch_time = level.time + ent->scr.commands[cmd].ParamFloat[0];
   if (ent->scr.save < 0)
      {
      ent->scr.save = ent->scr.index;
      ent->scr.savenextcmd = ent->scr.nextcmd;
      }
   ent->scr.index = ent->scr.on_touch;
   parse_command(ent);
}

void script_block(edict_t *ent, edict_t *other)
{
   int cmd = ent->scr.block_cmd;   
   if (ent->scr.on_block < 0)
      return;
	if (ent->scr.commands[cmd].ParamInt[0] != 2)
		{
	   if (ent->scr.commands[cmd].ParamInt[0])
		   {
			if (!other->client)
				return;
			}
		else 
			{
			if (!other->targetname)
				return;
			if (strcmpi(ent->scr.commands[cmd].ParamString[0], other->targetname))
	         return;
			}
		}
   ent->scr.activator = other;
   ent->scr.block_time = level.time + ent->scr.commands[cmd].ParamFloat[0];
   if (ent->scr.save < 0)
      {
      ent->scr.save = ent->scr.index;
      ent->scr.savenextcmd = ent->scr.nextcmd;
      }
   ent->scr.index = ent->scr.on_block;
   parse_command(ent);
}


int script_pain(edict_t *ent, edict_t *other)
{
   int cmd = ent->scr.pain_cmd;  
	if (ent->scr.commands[cmd].ParamInt[0] != 2)
		{
	   if (ent->scr.commands[cmd].ParamInt[0])
		   {
			if (!other->client)
				return 0;
			}
		else 
			{
			if (!other->targetname)
				return 0;
			if (strcmpi(ent->scr.commands[cmd].ParamString[0], other->targetname))
	         return 0;
			}
		}
   ent->scr.activator = other;
   ent->scr.pain_time = level.time + ent->scr.commands[cmd].ParamFloat[0];
   if (ent->scr.save < 0)
      {
      ent->scr.save = ent->scr.index;
      ent->scr.savenextcmd = ent->scr.nextcmd;
      }
   ent->scr.index = ent->scr.on_pain;
   parse_command(ent);
   return 1;
}

void script_use(edict_t *ent, edict_t *other, edict_t *activator)
{
   int i; 
   if (ent->scr.save >= 0)
      return;
   if (activator->targetname)
      {
      for (i = 0; i < ent->scr.num_intercepts; i++)
         if (ent->scr.ivec[i].index >= 0)
            if (!strcmpi(ent->scr.ivec[i].activator, activator->targetname))
               {
               ent->scr.save = ent->scr.index;
               ent->scr.savenextcmd = ent->scr.nextcmd;
               ent->scr.activator = activator;
               ent->scr.index = ent->scr.ivec[i].index;
               parse_command(ent);
               return;
               }
      }
   else if (activator->client)
      {
      for (i = 0; i < ent->scr.num_intercepts; i++)
         if (ent->scr.ivec[i].index >= 0)          
            if (!strcmpi(ent->scr.ivec[i].activator, ".client"))
               {
               ent->scr.save = ent->scr.index;
               ent->scr.savenextcmd = ent->scr.nextcmd;
               ent->scr.activator = activator;
               ent->scr.index = ent->scr.ivec[i].index;
               parse_command(ent);
               return;
               }
      }
   else
      {
      for (i = 0; i < ent->scr.num_intercepts; i++)
         if (ent->scr.ivec[i].index >= 0)          
            if (!strcmpi(ent->scr.ivec[i].activator, ".any"))
               {
               ent->scr.save = ent->scr.index;
               ent->scr.savenextcmd = ent->scr.nextcmd;
               ent->scr.activator = activator;
               ent->scr.index = ent->scr.ivec[i].index;
               parse_command(ent);
               return;
               }
      }
}


edict_t * script_activator(edict_t *ent, char *s, int activate)
{
   edict_t *x;
   if (activate == 255)
      return ent->scr.activator;
   if (activate == 1)
      return ent;
   for (x = g_edicts; x < &g_edicts[globals.num_edicts] ; x++)
	   {
		if (!x->inuse)
			continue;
      if (!x->targetname)
         continue;
		if (!strcmpi(x->targetname, s))
         return x;
	   }
	return NULL;
}
      

void on_see (edict_t *ent)
{
   int cmd = ent->scr.see_cmd;
   float k = ent->scr.commands[cmd].ParamFloat[1];
   edict_t *other = g_edicts;
   char *s = ent->scr.commands[cmd].ParamString[0];
   vec3_t v;
	
   ent->scr.see_time = level.time + ent->scr.commands[cmd].ParamFloat[0];
   if (ent->scr.commands[cmd].ParamInt[0])
      {
	   for ( ; other < &g_edicts[globals.num_edicts]; other++)
	      {
         if (!other->inuse)
            continue;
         if (other->flags & FL_NOTARGET)
            continue;
         if (!other->client)
            continue;
         if (other->health <= 0)
            continue;
         VectorSubtract(other->s.origin, ent->s.origin, v);
         if (!visible(ent, other) || !infront (ent, other))
				continue;
			if (VectorLength(v) < k)
				{
            ent->scr.activator = other;
            ent->scr.index = ent->scr.on_see;
            if (ent->scr.save < 0)
					{
               ent->scr.save = ent->scr.index;
               ent->scr.savenextcmd = ent->scr.nextcmd;
               }
            parse_command(ent);
            return;
            }
         }
      }
   else if (ent->scr.commands[cmd].ParamInt[0])
      {
	   for ( ; other < &g_edicts[globals.num_edicts]; other++)
	      {
         if (!other->inuse)
            continue;
         if (other->flags & FL_NOTARGET)
            continue;
         if (strcmpi(other->targetname, s))
            continue;
         if (other->health <= 0)
            continue;
         VectorSubtract(other->s.origin, ent->s.origin, v);
         if (visible(ent, other) && infront (ent, other))
            if (VectorLength(v) < k)
               {
               ent->scr.activator = other;
               ent->scr.index = ent->scr.on_see;
               if (ent->scr.save < 0)
                  {
                  ent->scr.save = ent->scr.index;
                  ent->scr.savenextcmd = ent->scr.nextcmd;
                  }
                parse_command(ent);
                return;
                }
         }
      }
}

void GoScript(edict_t *ent)
{
	//gi.dprintf (ent->client->pers.userinfo);
	/*
	if (atoi(Info_ValueForKey(ent->client->pers.userinfo, "crosshair")))
		gi.dprintf ("Cross\n");
	else
		gi.dprintf ("No Cross\n");
	*/

   if (inscript)
      {
      inscript = 0;
		if (ent->client->pers.weapon)
			ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
      ent->client->ps.fov = atoi(Info_ValueForKey(ent->client->pers.userinfo, "fov"));
		kill_camera(ent);
      }
  else 
      {
      inscript = 1;
      ent->client->ps.gunindex = 0;		
      ent->client->ps.fov = 90;
		ent->client->fov_destination = 90;
		ent->client->fov_speed = 0;
      }    
}
   
void AddUse(edict_t *ent, char *activator, int index)
{
   int i;
   for (i = 0; i < ent->scr.num_intercepts; i++)
      {
      if (ent->scr.ivec[i].activator[0])
         if (!strcmpi(ent->scr.ivec[i].activator, activator))
            {
            strcpy(ent->scr.ivec[i].activator, activator);
            ent->scr.ivec[i].index = index;
            return;
            }      
      }  
   for (i = 0; i < ent->scr.num_intercepts; i++)
      {
      if (ent->scr.ivec[i].index < 0)
         {
         strcpy(ent->scr.ivec[i].activator, activator);
         ent->scr.ivec[i].index = index;
         return;
         }
      }  
}


/*
void hand(edict_t *ent)
{
	edict_t	*dropped;
	vec3_t	forward, right;
	vec3_t	offset;
   gitem_t *item;

	dropped = G_Spawn();
   dropped->classname = item->classname;
   Drop_Item(ent, item);
}
*/


void kill_script(edict_t *self)
{
	gi.unlinkentity(self);
	self->scr.on_pain = -1;
	self->scr.on_see = -1;
	self->scr.on_touch = -1;
	self->scr.on_block = -1;
	self->scr.index = -1;
	self->scr.num_intercepts = 0;
	self->scr.commands = NULL;
	self->touch = NULL;
	self->blocked = NULL;
	gi.linkentity(self);
}

void actor_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{

   if (ent->touch_debounce_time > level.time)
      return;
   ent->touch_debounce_time = level.time + .5;
   script_touch (ent, other);
}
