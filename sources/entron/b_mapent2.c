/**********************************************************\
(c) 1998-1999 Endor Productions. All rights reserved.
Code written and compiled by Nawfel 'Topaz' Tricha. 
NOTICE: This code is given as is. There is no direct support
for anything written here. All the information/code included
here are free to use and redistribute under no resitrictions
as long as you credit Endor Productions and the Author for
any snipet of code you might find interesting.

  FILE: b_mapent2.c
  Description: Map entities (trampolines, 
               pressure platforms, etc). Part II

\**********************************************************/

#include "g_local.h"

void BecomeExplosion2 (edict_t *self);
void fire_fireball (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_gattling (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed);
void kill_script(edict_t *self);

// Chair
// style 0: (-32, -32, 0) (32, 32, 306)  -  Model: "models/objects/trees/tree1/tris.md2"
// style 1: (-32, -32, -1) (32, 32, 277)  -  Model: "models/objects/trees/tree2/tris.md2"
// style 2: (-32, -32, -1) (32, 32, 277)  -  Model: "models/objects/trees/tree3/tris.md2"
void SP_misc_tree(edict_t *ent)
{
   char x[256] = {0};
	switch (ent->style)
		{	
		case 1:
			ent->s.modelindex = gi.modelindex("models/objects/trees/tree2/tris.md2");
			ent->s.frame = 1;
			VectorSet(ent->mins, -32, -32, -1);
			VectorSet(ent->maxs, 32, 32, 277);
			break;
		case 2:
			ent->s.modelindex = gi.modelindex("models/objects/trees/tree3/tris.md2");
			ent->s.frame = 1;
			VectorSet(ent->mins, -32, -32, -1);
			VectorSet(ent->maxs, 32, 32, 277);
			break;
		default:
			ent->s.modelindex = gi.modelindex("models/objects/trees/tree1/tris.md2");
			VectorSet(ent->mins, -32, -32, 0);
			VectorSet(ent->maxs, 32, 32, 306);
		}  
   if (ent->team_data)
      ent->s.skinnum = ent->team_data - 1;
   ent->movetype = MOVETYPE_TOSS;
	ent->solid = SOLID_BBOX;
   gi.linkentity (ent);
}

// Book
// style 0: (-9, -2, -1) (9, 2, 11)  -  "models/objects/books/tris.md2"
// style 1: (-9, -6, 0) (9, 6, 2)  -  Model: "models/objects/books/tris1.md2"
// style 2: (-2, -5, -1) (2, 5, 11)  -  Model: "models/objects/books/tris2.md2"
// style 3: (-6, -5, -1) (6, 5, 3)  -  Model: "models/objects/books/tris3.md2"
void SP_misc_book(edict_t *ent)
{
   switch (ent->style)
      {
      case 1:
			VectorSet(ent->mins, -9, -6, 0);
			VectorSet(ent->maxs, 9, 6, 2);
         ent->s.modelindex = gi.modelindex("models/objects/books/tris1.md2");
         break;
      case 2:
			VectorSet(ent->mins, -2, -5, -1);
			VectorSet(ent->maxs, 2, 5, 11);
         ent->s.modelindex = gi.modelindex("models/objects/books/tris2.md2");
         break;
      case 3:
			VectorSet(ent->mins, -6, -5, -1);
			VectorSet(ent->maxs, 6, 5, 3);
         ent->s.modelindex = gi.modelindex("models/objects/books/tris3.md2");
         break;
      default:
			VectorSet(ent->mins, -9, -2, -1);
			VectorSet(ent->maxs, 9, 2, 11);
         ent->s.modelindex = gi.modelindex("models/objects/books/tris.md2");
         break;
      }
  ent->movetype = MOVETYPE_TOSS;
  gi.linkentity (ent);
}

// Candle
// (-2, -2, 0) (1, 1, 13)
// "models/objects/candle/candle/tris.md2" && "models/objects/candle/candle/tris2.md2" 
void SP_misc_candle(edict_t *ent)
{
   VectorSet(ent->mins, -2, -2, 0);
   VectorSet(ent->maxs, 1, 1, 13);
	ent->s.modelindex = gi.modelindex("models/objects/candle/candle/tris.md2");
	if (!ent->style)
		{
		ent->s.modelindex2 = gi.modelindex("models/objects/candle/candle/tris2.md2");
		ent->flipping = FLIP_LIGHT;
		ent->s.frame = rand() & 7;
		ent->style = 1;
		}
	ent->movetype = MOVETYPE_TOSS;
	gi.linkentity (ent);
}

// Candle with handle
// (-8, -8, -1) (11, 7, 13)
// "models/objects/candle/candle_handle/tris.md2" && "models/objects/candle/candle_handle/tris2.md2"
void SP_misc_candle_handle(edict_t *ent)
{
   VectorSet(ent->mins, -8, -8, -1);
   VectorSet(ent->maxs, 11, 7, 13);
	ent->s.modelindex = gi.modelindex("models/objects/candle/candle_handle/tris.md2");
	if (!ent->style)
		{
		ent->s.modelindex2 = gi.modelindex("models/objects/candle/candle_handle/tris2.md2");
		ent->flipping = FLIP_LIGHT;
		ent->s.frame = rand() & 7;
		ent->style = 1;
		}
	ent->movetype = MOVETYPE_TOSS;
	gi.linkentity (ent);
}

// Candle stand
// (-9, -9, -32) (9, 9, 36)
// "models/objects/candle/candle_stand/tris.md2"
void SP_misc_candle_stand(edict_t *ent)
{
   VectorSet(ent->mins, -10, -10, -25);
   VectorSet(ent->maxs, 10, 10, 39);
	ent->s.modelindex = gi.modelindex("models/objects/candle/candle_stand/tris.md2");
	ent->movetype = MOVETYPE_TOSS;
	ent->solid = SOLID_BBOX;
	gi.linkentity (ent);
}

// Rock
// style 0: (-79, -53, 0) (55, 55, 51)
// style 1: (-48, -48, 0) (79, 56, 51)
// style 2: (-93, -71, 0) (68, 55, 51)
// style 3: (-68, -67, 0) (49, 48, 51)
// style 4: (-75, -53, 0) (50, 49, 51)
// "models/objects/rocks/tris.md2"
void SP_misc_rock(edict_t *ent)
{
	ent->s.modelindex = gi.modelindex("models/objects/rocks/tris.md2");
	switch (ent->style)
		{
		case 1:
			ent->s.frame = 1;
			VectorSet(ent->mins, -48, -48, 0);
			VectorSet(ent->maxs, 79, 56, 51);
			break;
		case 2:
			ent->s.frame = 2;
			VectorSet(ent->mins, -93, -71, 0);
			VectorSet(ent->maxs, 68, 55, 51);
			break;
		case 3:
			ent->s.frame = 3;
			VectorSet(ent->mins, -68, -67, 0);
			VectorSet(ent->maxs, 49, 48, 51);
			break;
		case 4:
			ent->s.frame = 4;
			VectorSet(ent->mins, -75, -53, 0);
			VectorSet(ent->maxs, 50, 49, 51);
			break;
		default:
		   VectorSet(ent->mins, -79, -53, 0);
			VectorSet(ent->maxs, 55, 51, 51);
		}
	ent->movetype = MOVETYPE_TOSS;
	ent->solid = SOLID_BBOX;
	gi.linkentity (ent);
}

// Frame
// (,,) (,,)
// 
void SP_misc_frame(edict_t *ent)
{
   VectorSet(ent->mins, -4, -4, 0);
   VectorSet(ent->maxs, 4, 4, 4);
	switch (ent->style)
		{
		case 1:
			ent->s.modelindex = gi.modelindex("models/objects/misc/frame/medium/tris.md2");
			break;
		case 2:
			ent->s.modelindex = gi.modelindex("models/objects/misc/frame/large/tris.md2");
			break;
		default:
			ent->s.modelindex = gi.modelindex("models/objects/misc/frame/small/tris.md2");
		}
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	gi.linkentity (ent);
}

// Food
// (-4, -10, -5) (6, 9, 4)
// "models/objects/misc/food/tris.md2"
void SP_misc_food(edict_t *ent)
{
   VectorSet(ent->mins, -4, -10, -5);
   VectorSet(ent->maxs, 6, 9, 4);
	ent->s.modelindex = gi.modelindex("models/objects/misc/food/tris.md2");
	ent->movetype = MOVETYPE_TOSS;
	gi.linkentity (ent);
}

// Chair
// (-8, -8, -1) (8, 7, 10)
// "models/objects/misc/chair/tris.md2"
void chair_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	
}

void chair_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	ThrowGib (self, "models/objects/misc/chair/gibs/ass.md2", damage, GIB_METALLIC);
	ThrowGib (self, "models/objects/misc/chair/gibs/leg.md2", damage, GIB_METALLIC);
	ThrowGib (self, "models/objects/misc/chair/gibs/leg.md2", damage, GIB_METALLIC);
	ThrowGib (self, "models/objects/misc/chair/gibs/leg.md2", damage, GIB_METALLIC);
	ThrowGib (self, "models/objects/misc/chair/gibs/leg.md2", damage, GIB_METALLIC);
	G_FreeEdict(self);
}

void SP_misc_chair(edict_t *ent)
{
   VectorSet(ent->mins, -10, -11, -1);
   VectorSet(ent->maxs, 10, 10, 14);
	ent->s.modelindex = gi.modelindex("models/objects/misc/chair/tris.md2");
	ent->movetype = MOVETYPE_TOSS;
	if (!ent->health)
		ent->health = 30;
	ent->takedamage = DAMAGE_YES;
	ent->pain = chair_pain;
	ent->die = chair_die;
	ent->solid = SOLID_BBOX;
	gi.linkentity (ent);
}


// Bowl:
// (-5, -5, -1) (5, 5, 2)
// "models/objects/misc/bowl/tris.md2"
void SP_misc_bowl(edict_t *ent)
{
   VectorSet(ent->mins, -5, -5, -1);
   VectorSet(ent->maxs, 5, 5, 2);
	ent->s.modelindex = gi.modelindex("models/objects/misc/bowl/tris.md2");
	ent->movetype = MOVETYPE_TOSS;
	gi.linkentity (ent);
}

// Barrel:
// (-14, -14, -1) (14, 14, 34)
// style 0: "models/objects/barrel/metal/tris.md2"
// style 1: "models/objects/barrel/wood/tris.md2"


void barrel_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	
}

void barrel_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->style)
		{
		ThrowGib (self, "models/objects/barrel/metal/gib.md2", damage, GIB_METALLIC);
		ThrowGib (self, "models/objects/barrel/metal/gib.md2", damage, GIB_METALLIC);
		ThrowGib (self, "models/objects/barrel/metal/gib.md2", damage, GIB_METALLIC);
		ThrowGib (self, "models/objects/barrel/metal/gib.md2", damage, GIB_METALLIC);
		BecomeExplosion2(self);
		}
	else 
		{
		ThrowGib (self, "models/objects/barrel/wood/gib.md2", damage, GIB_METALLIC);
		ThrowGib (self, "models/objects/barrel/wood/gib.md2", damage, GIB_METALLIC);
		ThrowGib (self, "models/objects/barrel/wood/gib.md2", damage, GIB_METALLIC);
		ThrowGib (self, "models/objects/barrel/wood/gib.md2", damage, GIB_METALLIC);
		G_FreeEdict(self);
		}
}

void SP_misc_barrel(edict_t *ent)
{
   VectorSet(ent->mins, -14, -14, -1);
   VectorSet(ent->maxs, 14, 14, 34);
	switch (ent->style)
		{
		case 1:
			ent->s.modelindex = gi.modelindex("models/objects/barrel/metal/tris.md2");
			break;
		case 2:
			ent->s.modelindex = gi.modelindex("models/objects/barrel/fancy/tris.md2");
			break;
		default:
			ent->s.modelindex = gi.modelindex("models/objects/barrel/wood/tris.md2");
			break;
		}
	if (!ent->health)
		ent->health = 100;
	ent->takedamage = DAMAGE_YES;
	ent->pain = barrel_pain;
	ent->die = barrel_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->solid = SOLID_BBOX;
	gi.linkentity (ent);
}


void cannon_use (edict_t *self, edict_t *other, edict_t *activator)
{  
	switch (self->style)
		{
		case 1:
			monster_fire_rocket (self, self->s.origin, self->s.angles, 
				self->dmg, 500, MZ2_BOSS2_ROCKET_2);
			break;
		case 2:
		   fire_fireball (self, self->s.origin, self->s.angles, self->dmg, 750, 70, 15);
			break;
		case 3:
		   fire_gattling (self, self->s.origin, self->s.angles, self->dmg, 800);
			break;
		default:
		   fire_blaster (self, self->s.origin, self->s.angles, self->dmg, 1600, 0x00000060 /*EF_HYPERBLASTER*/, 1);	
		}
}

void SP_func_cannon (edict_t *ent)
{
   ent->use = cannon_use;
	if (!ent->dmg)
		ent->dmg = 50;
	AngleVectors (ent->s.angles, ent->s.angles, NULL, NULL);
   gi.linkentity(ent);
}


void SP_func_script (edict_t *ent)
{
	kill_script(ent);
	ent->inuse = true;
	if (ent->scriptfile)
		{
		load_script(ent);
		gi.linkentity(ent);
		}
	else
		G_FreeEdict(ent);
}