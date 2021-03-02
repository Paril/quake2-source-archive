#include "g_local.h"

void target_earthquake_think (edict_t *self);

void Cell_VicThink (edict_t *self)
{
	if (self->owner->playerclass != 9 && self->owner->playerclass != 2)
	{
		G_FreeEdict (self);
		return;
	}

	if (self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] == 100)
	{
    	self->nextthink = level.time + 1;
		return;
	}

	if (!G_ClientExists (self->owner))
	{
		G_FreeEdict (self);
		return;
	}

	if (!G_ClientNotDead (self->owner))
	{
		G_FreeEdict (self);
		return;
	}

	if ((self->owner->playerclass == 9) && !(self->owner->ripstate & STATE_CLOAKING))
		gi.sound(self->owner, CHAN_VOICE, gi.soundindex("items/protect4.wav"), 1, ATTN_NORM, 0);

	if (self->owner->ripstate & STATE_CLOAKING)
		self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] --;
	else
		self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] ++;

	self->nextthink = level.time + 1;
}

void Cell_VicMake (edict_t *ent)
{
	edict_t *health;

	health = G_Spawn ();
	health->owner = ent;
	health->think = Cell_VicThink;
	health->nextthink = level.time + 1;

	gi.linkentity (health);
}

void Cell_spell (edict_t *ent)
{
    if (ent->playerclass != 2 && ent->client->resp.it != 2)
		return;

	if (ent->client->resp.score < 15)
	{
		gi.cprintf(ent, PRINT_HIGH, "You don't have enough experience\n");
		return;
	}

    ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] = 0;

    Cell_VicMake (ent);
}

void Spawn_eq (edict_t *self)
{
	edict_t *ent;

    if (self->playerclass != 2 && self->client->resp.it != 2)
		return;

	if (self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 75)
	{
		gi.cprintf(self, PRINT_HIGH, "Not enough mana for spell\n");
		return;
	}

  if (self->client->resp.score < 50)
	{
    	gi.cprintf(self, PRINT_HIGH, "You don't have enough experience\n");
  		return;
	}

  self->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] -= 75;

 	ent = G_Spawn();
	ent->classname = "earthquake";
	ent->timestamp = level.time + 30;
	ent->last_move_time = 0;
	ent->think = target_earthquake_think;
	ent->activator = ent->owner = self;
	ent->nextthink = level.time + FRAMETIME;
	ent->speed = 700;
	ent->noise_index = gi.soundindex ("world/quake.wav"); // Vic :))

	VectorCopy (self->s.origin, ent->s.origin);

	gi.linkentity (ent);
}

void MageJump1 (edict_t *ent)
{
	vec3_t forward;

	if (ent->playerclass != 2 && ent->client->resp.it != 2)
		return;
	
	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 2)
	{
		gi.cprintf(ent, PRINT_HIGH, "Not enough cells for spell\n");
		return;
	}

    LessAmmo (ent, 2, "cells");
	gi.sound (ent, CHAN_BODY, gi.soundindex("weapons/slash1.wav"), 1, ATTN_NORM, 0);
	gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/attack2.wav"), 1, ATTN_NORM, 0);
	ent->velocity[2] += 350; 
    AngleVectors(ent->client->v_angle, forward, NULL, NULL);
    VectorMA(ent->velocity, -800, forward, ent->velocity);
}

void MageJump2 (edict_t *ent)
{
	vec3_t forward;

	if (ent->playerclass != 2 && ent->client->resp.it != 2)
		return;

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] < 2)
	{
		gi.cprintf(ent, PRINT_HIGH, "Not enough mana for spell\n");
		return;
	}

    LessAmmo (ent, 2, "cells");
	gi.sound (ent, CHAN_BODY, gi.soundindex("weapons/slash1.wav"), 1, ATTN_NORM, 0);
	gi.sound (ent, CHAN_WEAPON, gi.soundindex("weapons/attack2.wav"), 1, ATTN_NORM, 0);
	ent->velocity[2] += 350; 
    AngleVectors(ent->client->v_angle, forward, NULL, NULL);
    VectorMA(ent->velocity, 800, forward, ent->velocity);
}

void Health_VicThink (edict_t *self)
{
	if (!G_ClientExists (self->owner))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->client->pers.health == 90)
	{
       self->client->pers.health = 90;
	   return;
	}

	self->client->pers.health += 1;
	self->nextthink = level.time + 1;
}

void Health_VicMake (edict_t *ent)
{
	edict_t *health;
	health = G_Spawn ();
	health->owner = ent;
	health->think = Health_VicThink;
	health->nextthink = level.time + 1;

	gi.linkentity (health);
}

void Health_spell (edict_t *ent)
{
    if (ent->playerclass != 2 && ent->client->resp.it != 2)
		return;

	if (ent->client->resp.score < 30)
	{
		gi.cprintf(ent, PRINT_HIGH, "You don't have enough experience\n");
		return;
	}

    ent->client->pers.inventory[ITEM_INDEX(FindItem("Cells"))] = 0;

    Health_VicMake (ent);
}

void MyCell_Sel (edict_t *ent, int choice) 
{
	switch (choice)
	{
	case 0:
        MageJump2 (ent);
	break;
	case 1:
		MageJump1 (ent);
	break;
	case 2:
		Cell_spell (ent);
	break;
	case 3:
		Spawn_eq (ent);
	break;
	}
}

void Cmd_Spell_f (edict_t *ent)
{

  // Check to see if the menu is already open

   if (ent->client->showscores || ent->client->showinventory || 
        ent->client->showmenu || ent->client->showmsg)
        return;

   // send the layout
   
   Menu_Title(ent,"Cast spell");
   Menu_Add(ent,"Jump Forward");
   Menu_Add(ent,"Jump Back");
   Menu_Add(ent,"Cell regeneration");
   Menu_Add(ent,"Earthquake");

   // Setup the User Selection Handler

   ent->client->usr_menu_sel = MyCell_Sel;
   Menu_Open(ent);

} // Cmd_MyWave_f

//===================================================
//======= RANDOM EARTHQUAKE GENERATOR CODE ==========
//===================================================

//======================================================
// True if Ent is valid, has client, and edict_t inuse.
//======================================================
qboolean G_EntExists(edict_t *ent)
{
	return ((ent) && (ent->inuse));
}

//======================================================
// True if Ent is valid, has client, and edict_t inuse.
//======================================================
qboolean G_ClientExists(edict_t *ent)
{
	return (G_EntExists(ent) && (ent->client));
}

//======================================================
// True if ent is not DEAD or DEAD or DEAD (and BURIED!)
//======================================================
qboolean G_ClientNotDead(edict_t *ent)
{
	qboolean b1 = ent->client->ps.pmove.pm_type != PM_DEAD;
	qboolean b2 = ent->deadflag != DEAD_DEAD;
	qboolean b3 = ent->health > 0;

    if (!G_ClientExists (ent))
		return false;

	return (b3||b2||b1);
}

//======================================================
// True if ent is not DEAD and not just did a Respawn.
//======================================================
qboolean G_ClientInGame(edict_t *ent)
{
	if (!G_EntExists(ent))
		return false;

	if (!G_ClientNotDead(ent))
		return false;

	return (ent->client->respawn_time + 5.0 < level.time);
}

//======================================================
// True if start and end are within radius distance.
//======================================================
qboolean G_Within_Radius(vec3_t start, vec3_t end, float rad)
{
	vec3_t eorg = {0,0,0};
	int j;

	for (j = 0; j < 3; j++)
		eorg[j] = abs(start[j] - end[j]);

	return (VectorLength(eorg) < rad);
}
