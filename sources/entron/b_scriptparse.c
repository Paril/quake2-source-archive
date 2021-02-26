/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_scriptparse.c
  Description: Real-Time script parser / Broken for efficiency 

\**********************************************************/

#include "g_local.h"
#include "b_script.h"

edict_t * script_activator(edict_t *ent, char *s, int activate);
int script_Drop_Item (edict_t *ent);
void script_spawn(edict_t *ent, char *s, char *targetname, vec3_t origin, int spawnflags);
void GoScript(edict_t *ent);
void Show_camera_f(edict_t *self, int cam_number, float fade);
void camera_target_3rd (edict_t *ent, edict_t *target);
void camera_target_rotate (edict_t *ent, edict_t *target, float distance);
void AddUse(edict_t *ent, char *activator, int index);

extern int inscript;

void parse_set1(edict_t *ent);
void parse_set2(edict_t *ent);
void parse_set3(edict_t *ent);
void parse_set4(edict_t *ent);
void parse_set5(edict_t *ent);
void kill_script(edict_t *self);

void parse_command(edict_t *ent)
{
   if (ent->scr.index > ent->scr.numcommands)
      {
		kill_script(ent);
      return;
      }
reparse:
   ent->scr.cmd = &ent->scr.commands[ent->scr.index];
   ent->scr.nextcmd = 0;   
   switch (ent->scr.cmd->command)
      {
		case SCR_GOTO:
			ent->scr.save = -1;
			ent->scr.index = ent->scr.cmd->ParamInt[0];
			ent->status &= ~STATUS_SCR_CRITICAL;
			goto reparse;
			break;
		case SCR_RETURN:
			ent->scr.nextcmd = ent->scr.savenextcmd;
			ent->scr.index = ent->scr.save - 1;
			if (ent->scr.index < 0)
				{
				kill_script(ent);
				return;
				}
			ent->scr.save = -1;
			ent->status &= ~STATUS_SCR_CRITICAL;
			goto reparse;
			break;
		case SCR_STOP:
			kill_script(ent);
			ent->status &= ~STATUS_SCR_CRITICAL;
			return;
		}

	if (ent->scr.cmd->command < SCR_ON_BLOCK)
		parse_set1(ent);
	else if (ent->scr.cmd->command < SCR_STAND)
		parse_set2(ent);
	else if (ent->scr.cmd->command < SCR_STAND_SLAVE)
		parse_set3(ent);
	else if (ent->scr.cmd->command < SCR_CRITICAL)
		parse_set4(ent);
	else 
		parse_set5(ent);

   ent->scr.index++;
	if (ent->status & STATUS_SCR_CRITICAL)
		goto reparse;
}

void parse_set1(edict_t *ent)
{
   switch (ent->scr.cmd->command)
      {
      case SCR_WAIT:
         ent->scr.nextcmd = level.time + ent->scr.cmd->ParamFloat[0];
         return;
      case SCR_SAY:
         gi.sound (ent, CHAN_AUTO, 
                  gi.soundindex(ent->scr.cmd->ParamString[0]),
                  ent->scr.cmd->ParamFloat[0], 
                  ent->scr.cmd->ParamInt[0], 0);
         return;
      case SCR_WALK:
         {
         edict_t *x;
         x = script_activator(ent, 
                              ent->scr.cmd->ParamString[0], 
                              ent->scr.cmd->ParamInt[0]);
         if (x && ent->monsterinfo.walk)
            {
            ent->scr.nextcmd = -1;
            ent->monsterinfo.walk(ent);
            ent->enemy = x;
            }
         return;
         }
      case SCR_HIDE:
      case SCR_CHASE:
      case SCR_RUN:
         {
         edict_t *x;
         x = script_activator(ent, 
                              ent->scr.cmd->ParamString[0], 
                              ent->scr.cmd->ParamInt[0]);
         if (x && ent->monsterinfo.run)
            {
            switch (ent->scr.cmd->command)
               {
               case SCR_HIDE:
                  ent->monsterinfo.chaser = x;
                  ent->enemy = NULL;
                  ent->scr.nextcmd = -1;
                  break;
               case SCR_CHASE:
                  ent->enemy = x;
                  ent->status = STATUS_SLAVE_CHASE;
                  break;
               case SCR_RUN:
                  ent->enemy = x;
                  ent->scr.nextcmd = -1;
                  ent->monsterinfo.run(ent);
                  break;
               }
            ent->monsterinfo.run(ent);
            }
         return;
         }
      case SCR_DIE:
         if (ent->die)
            {
            ent->die (ent, ent, ent, ent->scr.cmd->ParamInt[0], vec3_origin);
            if (ent && ent->health > 0)
               ent->health = 0;
            }
         return;
      case SCR_DROP:
         //script_Drop_Item (ent);
         return;
      case SCR_ROTATE_SMOOTH:
         {
         edict_t *x;
			float delta, speed;
         x = script_activator(ent, ent->scr.cmd->ParamString[0], ent->scr.cmd->ParamInt[0]);
         if (x)
				{
				VectorSubtract (ent->scr.cmd->ParamVector, x->s.angles, x->scr.angledest);
				speed = ent->scr.cmd->ParamFloat[0];
				delta = x->scr.angledest[0];
				if (!delta)
					delta = x->scr.angledest[1];
				else if (!delta)
					delta = x->scr.angledest[2];
				else if (!delta)
					return;
				speed /= delta;
				if (speed < 0)
					speed = -speed;
				x->scr.angledelta[0] = x->scr.angledest[0] * speed;
				x->scr.angledelta[1] = x->scr.angledest[1] * speed;
				x->scr.angledelta[2] = x->scr.angledest[2] * speed;
				x->scr.anglespeed = speed;
				VectorCopy (ent->scr.cmd->ParamVector, x->scr.angledest);
				}
         }
         return;
      case SCR_REMOVE:
         {
         edict_t *x;
         x = script_activator(ent, ent->scr.cmd->ParamString[0], 0);
         if (x && ent != x)
	         G_FreeEdict(x);
         }
         return;
		}
}

void parse_set2(edict_t *ent)
{
   switch (ent->scr.cmd->command)
		{
      case SCR_ON_BLOCK:
			if (ent->scr.cmd->ParamInt[0] >= 0)
				{
				ent->scr.on_block = ent->scr.cmd->ParamInt[1];
				ent->scr.block_cmd = ent->scr.index;
				}
			else
				{
				ent->scr.on_block = -1;
				ent->blocked = NULL;
				}
         return;
      case SCR_ON_USE:
         AddUse(ent, ent->scr.cmd->ParamString[0], ent->scr.cmd->ParamInt[0]);
         return;
      case SCR_ACTION_TARGET:
         {
         edict_t *x;
         int k = ent->scr.cmd->ParamInt[0];
         x = script_activator(ent, 
                              ent->scr.cmd->ParamString[0], 
                              ent->scr.cmd->ParamInt[1]);
         if (x)
            {
            ent->enemy = x;
            ent->scr.nextcmd = -1;
            if (k && k < 17 && ent->monsterinfo.action[k - 1])
               ent->monsterinfo.action[k - 1](ent);
            }
         return;
         }
      case SCR_ATTACK:
         {
         edict_t *x;
         x = script_activator(ent, 
                              ent->scr.cmd->ParamString[0], 
                              ent->scr.cmd->ParamInt[0]);
         if (x && ent->monsterinfo.run)
            {
            ent->enemy = x;
            ent->status = STATUS_SLAVE_ATTACK;
            ent->dmg = ent->scr.cmd->ParamInt[1];
				ent->monsterinfo.run(ent);
            }
         return;
         }
      case SCR_SPAWN:
         //gi.dprintf ("---> %s\n", ent->scr.cmd->ParamString[0]);
         script_spawn(ent, ent->scr.cmd->ParamString[0], 
                      ent->scr.cmd->ParamString[1], 
                      ent->scr.cmd->ParamVector, 
                      ent->scr.cmd->ParamInt[0]);
         return;
      case SCR_ON_TOUCH:
			if (ent->scr.cmd->ParamInt[1] >= 0)
				{
				ent->scr.on_touch = ent->scr.cmd->ParamInt[1];
				ent->scr.touch_cmd = ent->scr.index;
				}
			else
				{
				ent->scr.on_touch = -1;
				ent->touch = NULL;
				}
         return;
      case SCR_ON_SEE:
         if (ent->scr.cmd->ParamInt[1] >= 0)
            {
            ent->scr.on_see = ent->scr.cmd->ParamInt[1];
            ent->scr.see_cmd = ent->scr.index;
            }
         else
            ent->scr.on_see = -1;
         return;
      case SCR_ON_PAIN:
         ent->scr.on_pain = ent->scr.cmd->ParamInt[1];
         ent->scr.pain_cmd = ent->scr.index;
         return;
      case SCR_LOOK_AT:
         {
         edict_t *x;
         vec3_t forward;
         x = script_activator(ent, 
                              ent->scr.cmd->ParamString[0], 
                              ent->scr.cmd->ParamInt[0]);
         if (x)
            {
	         VectorSubtract (x->s.origin, ent->s.origin, forward);
	         VectorNormalize (forward);   
            ent->s.angles[YAW] = vectoyaw(forward);
            }
         }
         return;
      case SCR_ACTION:
         {
         int k = ent->scr.cmd->ParamInt[0];
         if (k && k < 17 && ent->monsterinfo.action[k - 1])
            ent->monsterinfo.action[k - 1](ent);
         return;
         }
		}
}

void parse_set3(edict_t *ent)
{
   switch (ent->scr.cmd->command)
		{
      case SCR_STAND:
         if (ent->monsterinfo.stand)
				{
				ent->enemy = NULL;
            ent->monsterinfo.stand(ent);
				}
         return;
      case SCR_USE:
         {
         edict_t *x, *y;
         x = script_activator(ent, 
                              ent->scr.cmd->ParamString[1],
                              ent->scr.cmd->ParamInt[1]);
         y = script_activator(ent, ent->scr.cmd->ParamString[0], ent->scr.cmd->ParamInt[0]);
         if (y && y->use)
            {
            y->use(y, x, x);
            }
         return;
         }
      case SCR_CAMERA:
         {
         int i;
         edict_t *client;
			for (i = 0; i < maxclients->value; i++)
			   {
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
            Show_camera_f(client, ent->scr.cmd->ParamInt[0], ent->scr.cmd->ParamFloat[0]);
            }
         return;
         }
      case SCR_SCOPE:
         {
         int i, x = inscript;
         edict_t *client;
			for (i = 0; i < maxclients->value; i++)
			   {
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
            GoScript(client);
            }
         if (x)
            inscript = 0;
         else
            inscript = 1;
         return;
         }
      case SCR_RUN_SLAVE:
         {
         edict_t *x, *y;
         x = script_activator(ent, ent->scr.cmd->ParamString[0], 0);
         y = script_activator(ent, ent->scr.cmd->ParamString[1], ent->scr.cmd->ParamInt[0]);
			if (x && y && x->monsterinfo.run && x->health > 0)
					{
					x->activator = ent;
					x->enemy = y;
					ent->scr.nextcmd = -1;
					x->monsterinfo.run(x);
					}
         return;
         }
      case SCR_TARGET_CAMERA:
         {
         int i;
         edict_t *client, *x;
         x = script_activator(ent, ent->scr.cmd->ParamString[0], 0);
         if (x)
			for (i = 0; i < maxclients->value; i++)
			   {
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
            camera_target_3rd (client, x);
            }
         return;
         }
      case SCR_ROTATE_CAMERA:
         {
         int i;
         edict_t *client, *x;
         x = script_activator(ent, ent->scr.cmd->ParamString[0], 0);
         if (x)
			for (i = 0; i < maxclients->value; i++)
			   {
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
            camera_target_rotate (client, x, ent->scr.cmd->ParamFloat[0]);
            }
         return;
         }
      case SCR_ROTATE:
         VectorCopy(ent->scr.cmd->ParamVector, ent->s.angles);
         return;
      case SCR_ROTATE_SLAVE:
         {
         edict_t *x;
         x = script_activator(ent, ent->scr.cmd->ParamString[0], 0);
         if (x)
            VectorCopy(ent->scr.cmd->ParamVector, x->s.angles);
         }
         return;
      case SCR_LOOK_AT_SLAVE:
         {
         edict_t *x, *y;
         vec3_t forward;
         x = script_activator(ent, ent->scr.cmd->ParamString[0], 0);
         y = script_activator(ent, ent->scr.cmd->ParamString[1], 0);
         if (x && y)
            {
	         VectorSubtract (y->s.origin, x->s.origin, forward);
	         VectorNormalize (forward);   
            x->s.angles[YAW] = vectoyaw(forward);
            }
         }
         return;
      case SCR_ACTION_SLAVE:
         {
         edict_t *x;
         int k = ent->scr.cmd->ParamInt[0];
         x = script_activator(ent, ent->scr.cmd->ParamString[0], 0);
         if (x && x->health > 0)
            {
            if (k && k < 17 && x->monsterinfo.action[k - 1])
               x->monsterinfo.action[k - 1](x);
            }
         return;
         }
		}
}

void parse_set4(edict_t *ent)
{
   switch (ent->scr.cmd->command)
      {
      case SCR_STAND_SLAVE:
         {
         edict_t *x;
         x = script_activator(ent, ent->scr.cmd->ParamString[0], 0);
         if (x && x->monsterinfo.stand && x->health > 0)
            {
            x->enemy = NULL;
            x->monsterinfo.stand(x);
            }
         return;
         }
      case SCR_SAY_SLAVE:
         {
         edict_t *x;
         x = script_activator(ent, ent->scr.cmd->ParamString[0], 0);
         if (x && x->health > 0)
         gi.sound (x, CHAN_AUTO, 
                  gi.soundindex(ent->scr.cmd->ParamString[1]),
                  ent->scr.cmd->ParamFloat[0], ATTN_IDLE, 0);
         return;
         }
      case SCR_LOAD_MAP:
         {
         char x[255] = {0};
         if (!Q_stricmp(ent->scr.cmd->ParamString[0], ".self"))
            {
            sprintf(x, "map %s\n", level.mapname);
	         gi.AddCommandString (x);
            }
         else
            {
            sprintf(x, "map %s\n", ent->scr.cmd->ParamString[0]);
	         gi.AddCommandString (x);
            }
         return;
         }
      case SCR_STUFFCMD:
         {
         int i;
         edict_t *client;
			for (i = 0; i < maxclients->value; i++)
			   {
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
				gi.WriteByte (11);
				gi.WriteString (ent->scr.cmd->ParamString[0]);
				gi.unicast (client, true);
            }
         }
		case SCR_LISTEN:
			ent->scr.save = -1;
			return;
		case SCR_CREDITS:
			{
			int i;
         edict_t *client;
			for (i = 0; i < maxclients->value; i++)
			   {
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
				client->client->SPEC_data++;
				client->decel = 400;
            }
			return;
			}
		case SCR_MULTI_USE:
			{	
         edict_t *x, *other = g_edicts;
			float k = ent->scr.cmd->ParamFloat[0];
			vec3_t v;
         x = script_activator(ent, ent->scr.cmd->ParamString[0], ent->scr.cmd->ParamInt[0]);
			if (x)
				for ( ; other < &g_edicts[globals.num_edicts]; other++)
					{
					if (!other->inuse)
						continue;
					if (!other->use)
						continue;
					if (other == ent)
						continue;
					if (!other->monsterinfo.attack)
						continue;
					VectorSubtract(other->s.origin, ent->s.origin, v);
					if (!visible(ent, other))
						continue;
					if (VectorLength(v) < k)
						other->use(other, x, x);
					}
			return;
         }	
       case SCR_ZOOM:
         {
         int i;
         edict_t *client;
			for (i = 0; i < maxclients->value; i++)
			   {
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
				client->client->fov_destination = ent->scr.cmd->ParamFloat[0];
				if (client->client->ps.fov <= client->client->fov_destination)
					client->client->fov_speed = ent->scr.cmd->ParamFloat[1];
				else 
					client->client->fov_speed = -ent->scr.cmd->ParamFloat[1];
            }
         }
      case SCR_SND_LOOP:
			if (ent->scr.cmd->ParamInt[0])
				ent->s.sound = 0;
			else
				ent->s.sound = gi.soundindex(ent->scr.cmd->ParamString[0]);
         return;
      case SCR_SND_LOOP_SLAVE:
         {
         edict_t *x;
         x = script_activator(ent, ent->scr.cmd->ParamString[0], 0);
         if (x)
				{				
				if (ent->scr.cmd->ParamInt[0])
					x->s.sound = 0;
				else
					x->s.sound = gi.soundindex(ent->scr.cmd->ParamString[1]);
				}
			return;
         }
		case SCR_MOVE:
			{
         edict_t *x;
         x = script_activator(ent, ent->scr.cmd->ParamString[0], 0);
			if (x)
				{
				VectorCopy(x->s.origin, ent->s.origin);
				VectorCopy(x->s.origin, ent->s.old_origin);
				gi.linkentity (ent);
				}
			return;
			}
		case SCR_MOVE_SLAVE:
			{
         edict_t *x, *y;
         x = script_activator(ent, 
                              ent->scr.cmd->ParamString[0], 
                              ent->scr.cmd->ParamInt[0]);
         y = script_activator(ent, ent->scr.cmd->ParamString[1], 0);
         if (x && y)
            {
				VectorCopy(y->s.origin, x->s.origin);
				VectorCopy(y->s.origin, x->s.old_origin);
				gi.linkentity (x);
            }
			return;
			}
     }
}

void parse_set5(edict_t *ent)
{
   switch (ent->scr.cmd->command)
      {
		case SCR_CRITICAL:
			ent->status |= STATUS_SCR_CRITICAL;
			return;
		case SCR_FREE:
			ent->status &= ~STATUS_SCR_CRITICAL;
			return;
		}
}