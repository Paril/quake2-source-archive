/*==========================================================================
//  dl_light.c -- by Patrick Martin             Last updated:  11-29-1998
//--------------------------------------------------------------------------
//  This file contains code that creates Quake1 style light decor.
//========================================================================*/

#include "g_local.h"


/*=============================/  Torches  /=============================*/

/*QUAKED light_torch_small_walltorch (0 .5 0) (-4 -4 -16) (4 4 32)
Short wall torch.  Quake1 style.
*/

void small_walltorch_animate (edict_t *self)
{
        if (++self->s.frame > FRAME_LAST_WALLTORCH)
                self->s.frame = FRAME_FIRST_TORCH;

	self->nextthink = level.time + FRAMETIME;
}

void SP_light_torch_small_walltorch (edict_t *ent)
{
	ent->movetype     = MOVETYPE_NONE;
	ent->solid        = SOLID_NOT;
	VectorSet (ent->mins, -4, -4, 16);
	VectorSet (ent->maxs, 4, 4, 32);
        ent->s.modelindex = MD2_WALLTORCH;
        ent->s.frame      = FRAME_FIRST_TORCH;
        ent->s.skinnum    = 0;
	ent->s.renderfx   = RF_FULLBRIGHT;
        ent->s.sound      = WAV_FIRE;
        ent->think        = small_walltorch_animate;
	ent->nextthink    = level.time + FRAMETIME;
	gi.linkentity (ent);
}

/*QUAKED light_torch_small_bowltorch (0 .5 0) (-3 -3 -4) (3 3 16)
Short bowl torch.  .md2 done by Andrew.
*/

void small_bowltorch_animate (edict_t *self)
{
        if (++self->s.frame > FRAME_LAST_BOWLTORCH)
                self->s.frame = FRAME_FIRST_TORCH;

	self->nextthink = level.time + FRAMETIME;
}

void SP_light_torch_small_bowltorch (edict_t *ent)
{
	ent->movetype     = MOVETYPE_NONE;
	ent->solid        = SOLID_NOT;
	VectorSet (ent->mins, -3, -3, -4);
	VectorSet (ent->maxs, 3, 3, 16);
        ent->s.modelindex = MD2_BOWLTORCH;
        ent->s.frame      = FRAME_FIRST_TORCH;
        ent->s.skinnum    = 0;
	ent->s.renderfx   = RF_FULLBRIGHT;
        ent->s.sound      = WAV_FIRE;
        ent->think        = small_bowltorch_animate;
	ent->nextthink    = level.time + FRAMETIME;
	gi.linkentity (ent);
}


/*=============================/  Flames  /=============================*/

/*QUAKED light_flame_large_yellow (0 1 0) (-8 -8 0) (8 8 64)
This is for large Quake1 style flames.
*/

void large_flame_animate (edict_t *self)
{
        if (++self->s.frame > FRAME_LAST_LARGEFIRE)
                self->s.frame = FRAME_FIRST_LARGEFIRE;

	self->nextthink = level.time + FRAMETIME;
}

void SP_light_flame_large_yellow (edict_t *ent)
{
	ent->movetype     = MOVETYPE_NONE;
	ent->solid        = SOLID_NOT;
	VectorSet (ent->mins, -8, -8, 0);
	VectorSet (ent->maxs, 8, 8, 64);
        ent->s.modelindex = MD2_FIRE;
        ent->s.frame      = FRAME_FIRST_LARGEFIRE;
        ent->s.skinnum    = SKIN_FIRE;
	ent->s.renderfx   = RF_FULLBRIGHT;
        ent->s.sound      = WAV_FIRE;
	ent->think        = large_flame_animate;
	ent->nextthink    = level.time + FRAMETIME;
	gi.linkentity (ent);
}

/*QUAKED light_flame_small_yellow (0 1 0) (-4 -4 0) (4 4 32)
This is for small Quake1 style flames.
*/

void small_flame_animate (edict_t *self)
{
        if (++self->s.frame > FRAME_LAST_SMALLFIRE)
                self->s.frame = FRAME_FIRST_SMALLFIRE;

	self->nextthink = level.time + FRAMETIME;
}

void SP_light_flame_small_yellow (edict_t *ent)
{
	ent->movetype     = MOVETYPE_NONE;
	ent->solid        = SOLID_NOT;
	VectorSet (ent->mins, -4, -4, 0);
	VectorSet (ent->maxs, 4, 4, 32);
        ent->s.modelindex = MD2_FIRE;
        ent->s.frame      = FRAME_FIRST_SMALLFIRE;
        ent->s.skinnum    = SKIN_FIRE;
	ent->s.renderfx   = RF_FULLBRIGHT;
        ent->s.sound      = WAV_FIRE;
	ent->think        = small_flame_animate;
	ent->nextthink    = level.time + FRAMETIME;
	gi.linkentity (ent);
}

/*QUAKED light_flame_small_white (0 1 0) (-4 -4 0) (4 4 32)
This is for small Quake1 style flames.  I could not tell the
difference between it and its yellow brother in Quake1.
*/

void SP_light_flame_small_white (edict_t *ent)
{
	SP_light_flame_small_yellow (ent);
}


/*============================/  Lavaballs  /============================*/

/*QUAKED misc_fireball (0 .5 .8) (-8 -8 -8) (8 8 8)
Lava Balls
*/

void lavaball_touch
(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        T_Damage (other, self, self, self->velocity, self->s.origin, self->velocity, self->dmg, self->dmg, 0, MOD_LAVABALL, NULL);
	G_FreeEdict (self);
}

void lavaball_toss (edict_t *self)
{
	edict_t   *lavaball;

	lavaball = G_Spawn();
	VectorCopy (self->s.origin, lavaball->s.origin);
	VectorSet (lavaball->velocity, 0, 0, 1000);
	lavaball->velocity[0]  = random() * 100 - 50;
	lavaball->velocity[1]  = random() * 100 - 50;
	lavaball->velocity[2]  = random() * 200 + self->speed;
	lavaball->movetype     = MOVETYPE_TOSS;
	lavaball->clipmask     = MASK_SHOT;
	lavaball->solid        = SOLID_BBOX;   /* Was SOLID_TRIGGER in Q1. */
	lavaball->s.effects    = EF_ROCKET;
	lavaball->classname    = "fireball";
	VectorClear (lavaball->mins);
	VectorClear (lavaball->maxs);
        lavaball->s.modelindex = MD2_LAVABALL;
	lavaball->touch        = lavaball_touch;
	lavaball->nextthink    = level.time + 5;
	lavaball->think        = G_FreeEdict;
	lavaball->dmg          = self->dmg;

	gi.linkentity (lavaball);

/* Throw another lava ball soon. */
	self->nextthink = level.time + (random() * 5) + 3;
}

void SP_misc_fireball (edict_t *ent)
{
	ent->movetype     = MOVETYPE_NONE;
	ent->solid        = SOLID_NOT;
	VectorSet (ent->mins, -8, -8, -8);
	VectorSet (ent->maxs, 8, 8, 8);
	if (!ent->dmg)
		ent->dmg = 20;
	if (!ent->speed)
		ent->speed = 1000;
	ent->think        = lavaball_toss;
	ent->nextthink    = level.time + (random() * 5);
	gi.linkentity (ent);
}


/*===========================/  END OF FILE  /===========================*/
