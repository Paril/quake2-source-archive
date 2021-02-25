/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_ai.c
  Description: Generic AI

\**********************************************************/

#include "g_local.h"

void draw_line(vec3_t start, vec3_t end, float draw_time);
void draw_laser(edict_t *ent);
void draw_vertex(vec3_t start);
void BOT_EnemyChaseDir (edict_t *actor, vec3_t dest, float dist);
ambush_node_t *nearambush (ambush_node_t *from, vec3_t org, float rad);
qboolean reacheable_vector(vec3_t self, vec3_t other);
void GrenadeAlertMonsters(edict_t *self);
int HideFromGrenades(edict_t *ent);

edict_t *nearest_friend (edict_t *self)
{
   edict_t *other = NULL, *best = NULL;
   vec3_t v;
   float range = 10000;
   // Search and attack visible enemies 
   while ((other = findradius(other, self->s.origin, 1200)) != NULL)
      {
      if (!other->inuse)
         continue;
      if (other->team_data != self->team_data)
         continue;
      if (other->client)
         continue;
      if (!other->takedamage)
         continue;
		if (other->health <= 0)
			continue;
      if (other->health >= other->max_health && !(other->status & STATUS_CEC))
         continue;
      if (other == self)
         continue;
      VectorSubtract(other->s.origin, self->s.origin, v);
      if (range > VectorLength(v))
         {
         range = VectorLength(v);
         best = other;
         }
      }
   if (best)
      {
      self->monsterinfo.sight(self, other);
      self->monsterinfo.run(self);
      }
   return best;
}


edict_t *nearest_foe (edict_t *self)
{
   edict_t *other = NULL;
   vec3_t v;

	if (self->scr.commands)
		return NULL;
   if (self->scr.on_see >= 0)
		return NULL;

	if (!(self->type & TYPE_MONSTER_FOE))
		return NULL;

   if (self->status & STATUS_MONSTER_HIDE)
      return NULL;
   
   if (self->type & TYPE_MONSTER_TEAMMATE)
      return nearest_friend (self);

   // Search and attack visible enemies 
   while ((other = findradius(other, self->s.origin, 2000)) != NULL)
      {
      if (!other->inuse)
         continue;
      if (!other->client)
         continue;
      if (other->flags & FL_NOTARGET)
         continue;
      if (!other->takedamage)
         continue;
      if (other->health <= 0)
         continue;
      if (other == self)
         continue;
      VectorSubtract(other->s.origin, self->s.origin, v);
      if (visible(self, other))
         {
         if (VectorLength(v) < 200)  // hear
            {
            self->monsterinfo.sight(self, other);
            self->monsterinfo.run(self);
            return other;
            }
         if (infront (self, other))
            {
            self->monsterinfo.sight(self, other);
            self->monsterinfo.run(self);
            return other;
            }
         }
      }
   return NULL;
}

edict_t *closest_foe (edict_t *self)
{
   edict_t *other = NULL;

   if (self->status & STATUS_MONSTER_HIDE)
      return NULL;
   
   if (self->type & TYPE_MONSTER_TEAMMATE)
      return nearest_friend (self);

   // Search and attack visible enemies 
   while ((other = findradius(other, self->s.origin, 200)) != NULL)
      {
      if (!other->inuse)
         continue;
      if (!other->client)
         continue;
      if (other->flags & FL_NOTARGET)
         continue;
      if (!other->takedamage)
         continue;
      if (other->health <= 0)
         continue;
      if (!infront (other, self))
         {
         self->monsterinfo.run(self);
         return other;
         }
      }
   return NULL;
}

map_node_t *nearest_node(edict_t *ent)
{
   map_node_t *node = NULL;

   while ((node = nearnode(node, ent->s.origin, 64)) != NULL)
         if (visible_vector(ent->s.origin, node->origin))                        
            return node;
   node = NULL;
   while ((node = nearnode(node, ent->s.origin, 128)) != NULL)
         if (visible_vector(ent->s.origin, node->origin))                        
            return node;
   node = NULL;
   while ((node = nearnode(node, ent->s.origin, 256)) != NULL)
         if (visible_vector(ent->s.origin, node->origin))
            return node;
   node = NULL;
   while ((node = nearnode(node, ent->s.origin, 512)) != NULL)
         if (visible_vector(ent->s.origin, node->origin))
            return node;
   node = NULL;
   while ((node = nearnode(node, ent->s.origin, 1024)) != NULL)
         if (visible_vector(ent->s.origin, node->origin))
            return node;
   return NULL;
}

map_node_t *hide_node(edict_t *ent, edict_t *ent2)
{
   map_node_t *node = NULL, 
              *dest = NULL;
   while ((node = nearnode(node, ent->s.origin, 128)) != NULL && !dest)
         if (!visible_vector(node->origin, ent2->s.origin))
            dest = node;
   if (!dest)
      {      
      node = NULL;
      while ((node = nearnode(node, ent->s.origin, 256)) != NULL && !dest)
         if (!visible_vector(node->origin, ent2->s.origin))
         dest = node;
      }
   if (!dest)
      {
      node = NULL;
      while ((node = nearnode(node, ent->s.origin, 512)) != NULL && !dest)
         if (!visible_vector(node->origin, ent2->s.origin))
            dest = node;
      }
   if (!dest)
      {
      node = NULL;
      while ((node = nearnode(node, ent->s.origin, 1024)) != NULL && !dest)
         if (!visible_vector(node->origin, ent2->s.origin))
            dest = node;
      }
   return dest;
}

void BOT_tellteam (edict_t *self)
{
   edict_t *other = NULL;

   return;
   if (self->type & TYPE_MONSTER_TEAMMATE)
         return;

   // Tell the other bots about enemy hiding from.
   if (!self->enemy && self->monsterinfo.chaser)
      self->enemy = self->monsterinfo.chaser;

   // Leave the comparisons in this sequence, much faster.
	while ((other = findradius(other, self->s.origin, 1000)) != NULL)
      {
      if (!other->inuse || other->health <= 0)
         continue;
      if (other == self->enemy)
         continue;
		if (!other->takedamage || other->enemy)
			continue;
		if (other->status & STATUS_MONSTER_HIDE)
			continue;
		if (other->type & TYPE_SLAVE_ACTOR)
			continue;
      if (!other->monsterinfo.run)
         continue;
      if (other->team_data != self->team_data)
         continue;
      if (!visible(other, self))
         continue;
      if (!(other->type & TYPE_MONSTER_TEAMMATE))
         {
         other->enemy = self->enemy;
         other->monsterinfo.run(other);
         }
      }   

   // Restore hiding bot to not having an attackable enemy
   if (self->monsterinfo.chaser)
      self->enemy = NULL;
}

void find_node_hide(edict_t *ent)
{  
   if (!ent->enemy)
      ent->enemy = ent->monsterinfo.chaser;
   if (!ent->enemy)
      return;
   ent->monsterinfo.lastnode = ent->monsterinfo.goalnode;
   ent->monsterinfo.goalnode = hide_node(ent, ent->enemy);
   // If hiding spot found, then reset to no enemy
   ent->enemy = NULL;
   if (ent->monsterinfo.goalnode)
      ent->monsterinfo.chaser = ent->enemy;
   // If no hiding spot found, then no choice but to retaliate
   else
      {
      find_node_target(ent);
      if (ent->monsterinfo.goalnode)
         return;
		if (!(ent->type & TYPE_SLAVE_ACTOR))
			{
			ent->status &= ~STATUS_MONSTER_HIDE;
			ent->enemy = ent->monsterinfo.chaser;
			ent->monsterinfo.chaser = NULL;
			}
		else
			{
			ent->monsterinfo.stand(ent);
			}
      //ent->monsterinfo.run(ent);
      }
}

void find_node_hide2(edict_t *ent)
{  
   int i, k, j = 0;
   vec3_t forward;
   map_node_t *node = NULL, 
              *dest = NULL;
   //int found;
   // Check for nearest node to the monster. In 4 steps.
   if (!ent->enemy)
      ent->enemy = ent->monsterinfo.chaser;
   if (!ent->enemy)
      return;
   for (i = 0; ent->monsterinfo.goalnode->adjacent[i]; i++)
      if (!visible_vector(ent->monsterinfo.goalnode->adjacent[i]->origin, ent->enemy->s.origin))
         {
         dest = ent->monsterinfo.goalnode->adjacent[i];
         break;
         }
   if (i && !dest)
      {
      do 
         {
         j++;
         k = rand() % i;
         } while (ent->monsterinfo.goalnode->adjacent[k] == ent->monsterinfo.lastnode && j < 16);
      if (j < 15)
         dest = ent->monsterinfo.goalnode->adjacent[k];
      }
   if (dest)
      {     
      ent->monsterinfo.lastnode = ent->monsterinfo.goalnode;
      ent->monsterinfo.goalnode = dest;
      ent->enemy = NULL;
      return;
      }
   // If no hiding spot found, then no choice but to retaliate if not scripted
   if (!ent->scr.commands)
      {
      ent->monsterinfo.goalnode = NULL;
      ent->monsterinfo.chaser = NULL;
		if (!(ent->type & TYPE_SLAVE_ACTOR))
			ent->status &= ~STATUS_MONSTER_HIDE;
		else
			ent->enemy = NULL;
      return;            
      }

   VectorSubtract (ent->enemy->s.origin, ent->s.origin, forward);
	VectorNormalize (forward);   
   ent->s.angles[YAW] = vectoyaw(forward);
   ent->enemy = NULL;
   ent->monsterinfo.stand(ent);
}        



void find_node_target(edict_t *ent)
{  
   int k, j = 0;
   edict_t *other = NULL;
   map_node_t *node = NULL, 
              *dest = NULL;

	//gi.dprintf ("node searched by %d %f\n ", ent - g_edicts, level.time);
   //int found;
   // Check for nearest node to the monster. In 4 steps
   if (!ent->monsterinfo.goalnode)
      {
      ent->monsterinfo.lastnode = NULL;
      ent->monsterinfo.goalnode = nearest_node (ent);

      // If after 20 mins still no target reached. Choose another target;
      ent->volume = level.time + 30;
      return;
      }

   // Roam to next random node. Bot will try not to go back to where it was
   if (!ent->enemy || !ent->enemy->inuse)
      {
      // If node has any neighbors, go to one of them
      if (ent->monsterinfo.goalnode->nodes)
         {
         do 
            {
            j++;
            k = rand() % ent->monsterinfo.goalnode->nodes;
            } while ((ent->monsterinfo.goalnode->adjacent[k] == ent->monsterinfo.lastnode) && j < 16);
         dest = ent->monsterinfo.goalnode->adjacent[k];
         }
      // Otherwise, go back the way you came
      else 
         dest = ent->monsterinfo.lastnode;
      ent->monsterinfo.lastnode = ent->monsterinfo.goalnode;
      ent->monsterinfo.goalnode = dest;
      // If after 20 mins still no target reached. Choose another target;
      ent->volume = level.time + 30;
      return;
      }

   // If enemy is around
   dest = nearest_node (ent->enemy);

   // If enemy is not one node away from bot, then find route
   if (dest && dest != ent->monsterinfo.goalnode)
      {
      if (ent->monsterinfo.goalnode->route[dest->number] != ent->monsterinfo.goalnode->number)
         {
         dest = &g_nodes[ent->monsterinfo.goalnode->route[dest->number]];
         //gi.bprintf (PRINT_CHAT, "BOT: Passing Through: %i/%i\n", ent->monsterinfo.goalnode->number, dest->number); 
 			if (dest != ent->monsterinfo.goalnode)
				{
				ent->monsterinfo.lastnode = ent->monsterinfo.goalnode;
				ent->monsterinfo.goalnode = dest;			
				}
			else
				{
				if (ent->monsterinfo.goalnode->nodes)
					{
					do 
						{
						j++;
						k = rand() % ent->monsterinfo.goalnode->nodes;
						} while ((ent->monsterinfo.goalnode->adjacent[k] == ent->monsterinfo.lastnode) && j < 16);
					dest = ent->monsterinfo.goalnode->adjacent[k];
					}
				// Otherwise, go back the way you came
				else 
					dest = ent->monsterinfo.lastnode;
				ent->monsterinfo.lastnode = ent->monsterinfo.goalnode;
				ent->monsterinfo.goalnode = dest;
				}
         // If after 30 secs still no target reached. Choose another target;
         ent->volume = level.time + 30;
         return;
         }
      }
	if (dest && dest == ent->monsterinfo.goalnode)
		{
		ent->status |= STATUS_MONSTER_NO_GOAL;
		k = rand() % ent->monsterinfo.goalnode->nodes;
		ent->monsterinfo.goalnode = ent->monsterinfo.goalnode->adjacent[k];
		return;
		}

   ent->monsterinfo.goalnode = NULL;
}

void GoAmbush(edict_t *ent, edict_t *enemy, float dist)
{
   ambush_node_t *node = NULL, 
                 *dest = NULL;
   
   if (!enemy)
      {
      gi.error ("ERROR AI[303]: No enemy found in GoAmbush\n Please tell me if you read this line\n Topaz ");
      return;
      }  
   /*
   if (ent->monsterinfo.goalambush)
      {
      gi.dprintf ("Already has node\n");
      return;
      }
   */

   ent->status &= ~STATUS_MONSTER_AMBUSH;
  
  while (!dest && (node = nearambush(node, ent->s.origin, 128)) != NULL)			
         if (!visible_vector(node->origin, enemy->s.origin))
					dest = node;

	node = NULL;
   while (!dest && (node = nearambush(node, ent->s.origin, 256)) != NULL )
         if (!visible_vector(node->origin, enemy->s.origin))
					dest = node;
   node = NULL;
   while (!dest && (node = nearambush(node, ent->s.origin, 512)) != NULL)
         if (!visible_vector(node->origin, enemy->s.origin))
					dest = node;
   node = NULL;
   while (!dest && (node = nearambush(node, ent->s.origin, 1024)) != NULL)
         if (!visible_vector(node->origin, enemy->s.origin))
					dest = node;

   ent->monsterinfo.goalambush = dest;
   if (dest)
      {  
      ent->status |= STATUS_MONSTER_AMBUSH;
      BOT_EnemyChaseDir (ent, dest->origin, dist);
      ent->monsterinfo.goalnode = NULL;
      return;
      }
   if (ent->monsterinfo.goalnode)
      {
      find_node_target(ent);
      return;
      }
   ent->monsterinfo.goalnode = nearest_node(ent);
   if (ent->monsterinfo.goalnode)
      BOT_NewChaseDir (ent, ent->monsterinfo.goalnode, dist);

}

void GrenadeAlertMonsters(edict_t *self)
{
   edict_t *monster = NULL;

   while ((monster = findradius(monster, self->s.origin, 256)) != NULL)
      {
      if (!monster->inuse)
         continue;
      if (!(monster->svflags & SVF_MONSTER))
         continue;      
      if (!(monster->type & TYPE_MONSTER_AMBUSH))
         continue;
      if (monster != self->owner)
         {
         if (monster->team_data && monster->team_data == self->owner->team_data)
            {
            monster->oldenemy = monster->enemy;
            monster->enemy = self->owner->enemy;
            monster->monsterinfo.run(monster);
            }
         else
            {
            monster->oldenemy = monster->enemy;
            monster->enemy = self->owner;
            monster->monsterinfo.run(monster);
            }
         }
      GoAmbush(monster, self, 6);
      }
}

int  HideFromGrenades(edict_t *ent)
{
   edict_t *grenade = NULL;
   vec3_t forward;
   int found = 0;

   while ((grenade = findradius(grenade, ent->s.origin, 256)) != NULL)
      {
      if (!grenade->inuse)
         continue;
      if (grenade->type != TYPE_WEAPON_GRENADE)
         continue;
      if (ent != grenade->owner)
         {
         if (ent->team_data && ent->team_data == grenade->owner->team_data)
            {
            ent->oldenemy = ent->enemy;
            ent->enemy = grenade->owner->enemy;
            ent->monsterinfo.run(ent);
            }
         else
            {
            ent->oldenemy = ent->enemy;
            ent->enemy = grenade->owner;
            ent->monsterinfo.run(ent);
            }
         }
      VectorSubtract (grenade->s.origin, ent->s.origin, forward);
	   ent->s.angles[YAW] = vectoyaw(forward) + 180;
      BOT_walkmove(ent, ent->s.angles[YAW], 24);
      found = 1;
      }
return found;
}


void TakeCover(edict_t *ent, float dist)
{
   trace_t trace;
   vec3_t forward, right, offset, end, start, angle, deep, best;
   float distance, dir;
   int direction; 
   char killed_dir[12] = {0};
   
   if (ent->status & STATUS_MONSTER_AMBUSH)
      return;

   VectorCopy(ent->s.angles, angle);

   dir = random();
   if (dir < .5)
      {
      angle[YAW] += 90;   
      dir = 40;
      }
   else
      {
      angle[YAW] -= 90;   
      dir = -40;
      }
   start[0] = ent->enemy->s.origin[0];
   start[1] = ent->enemy->s.origin[1];
   start[2] = ent->enemy->s.origin[2] + ent->enemy->viewheight;
   for (distance = 40; distance < 1200; distance += 40)
      {
      for (direction = 0; direction < 12; direction++)
         {
         /*
         if (killed_dir[direction])
            {
            angle[1] += dir;
            continue;
            }
         */
         AngleVectors (angle, forward, right, NULL);
         angle[1] += dir;
         VectorSet(offset, distance, 0, 0);
	      G_ProjectSource (ent->s.origin, offset, forward, right, end);
         trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_SHOT);
         
         // Path from monster to a hidden place:
         if (trace.fraction < 1.0)
            {
            killed_dir[direction] = 1;
            continue;
            }

         // Cannot check over edges
         VectorCopy(end, deep);
         deep[2] = end[2] - 32;
         trace = gi.trace (end, ent->mins, ent->maxs, deep, ent, MASK_SOLID);
         /*
         if (trace.fraction == 1.0)
            {
            killed_dir[direction] = 1;
            continue;
            }
         if (trace.endpos[2] + 32 > end[2])
            {
            draw_line(end, trace.endpos, 1);  
            killed_dir[direction] = 1;
            continue;
            }
         */
         end[2] += ent->viewheight;
         trace = gi.trace (start, NULL, NULL, end, ent->enemy, MASK_SOLID);
         
         // Hidden place not visible to monster:
         if (trace.fraction == 1.0)
            continue;
         
         // Go deep into hidden place in 12 100-foot steps:
         VectorCopy(end, best);
         VectorSet(offset, 100, 0, 0);
         for (distance = 0; distance < 12; distance++) 
            {
            G_ProjectSource (best, offset, forward, right, end);
            trace = gi.trace (best, ent->mins, ent->maxs, end, ent, MASK_SHOT);
            // Monster can go in hidden path:
            if (trace.fraction < 1.0)
               {
               distance = 13;
               continue;
               }
            // Cannot check over edges
            VectorCopy(end, deep);
            deep[2] = end[2] - 90;
            trace = gi.trace (end, ent->mins, ent->maxs, deep, ent, MASK_SOLID);
            if (trace.fraction == 1.0)
               {
               distance = 13;
               continue;
               }
            if (trace.endpos[2] + 32 > end[2])
               {
               distance = 13;
               continue;
               }            
                       
            trace = gi.trace (start, NULL, NULL, end, ent->enemy, MASK_SOLID);
            
            // Enemy can't see monster...
            if (trace.fraction < 1.0)
               VectorCopy(end, best);
            else
               {
               distance = 13;
               continue;
               }
            }                     
         draw_line(ent->s.origin, end, 3);  
         //draw_vertex(best);
         //draw_vertex(trace.endpos);
         ent->monsterinfo.lastnode = NULL;
         ent->monsterinfo.goalnode = NULL;
         VectorCopy(best, ent->monsterinfo.goalvector);
         ent->status |= STATUS_MONSTER_AMBUSH;
         BOT_EnemyChaseDir (ent, best, dist);
         return;
         }
      }
}

void draw_line(vec3_t start, vec3_t end, float draw_time)
{
   edict_t *vertex = G_Spawn();
   VectorCopy(start, vertex->s.origin);
   VectorCopy(end, vertex->pos1);
   vertex->delay = level.time + draw_time;
   vertex->nextthink = level.time + FRAMETIME;
   vertex->think = draw_laser;
   gi.linkentity(vertex);
}
               
void draw_laser(edict_t *ent)
{
   if (level.time > ent->delay)
      {
      G_FreeEdict(ent);
      return;
      }
   gi.WriteByte (svc_temp_entity);
   gi.WriteByte (TE_BFG_LASER);
   gi.WritePosition (ent->s.origin);  //self->s.origin
   gi.WritePosition (ent->pos1);
   gi.multicast (ent->s.origin, MULTICAST_PHS);
   ent->nextthink = level.time + FRAMETIME;
}

void draw_vertex(vec3_t start)
{
   edict_t *vertex = G_Spawn();
   VectorCopy(start, vertex->s.origin);
   vertex->s.modelindex = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");
   vertex->nextthink = level.time + 2;
   vertex->think = G_FreeEdict;
   gi.linkentity(vertex);
}