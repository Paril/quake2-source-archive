#include "g_local.h"

// This file contains many of the single player selfities that are not used in multiplayer.  As
// RPS is a multiplayer only mod, I thought I would decrease the size of the .dll, and remove
// all the SP code.  These functions are necessary, and just free the edict without using it.

// g_trigger.c

void SP_trigger_key (edict_t *self)
{
	G_FreeEdict(self);
}

void SP_trigger_monsterjump (edict_t *self)
{
	G_FreeEdict(self);
}

// g_turret.c

void SP_turret_breach (edict_t *self)
{
	G_FreeEdict(self);
}

void SP_turret_base (edict_t *self)
{
	G_FreeEdict(self);
}

void SP_turret_driver (edict_t *self)
{
	G_FreeEdict(self);
}

// m_actor.c

void SP_misc_actor (edict_t *self)
{
	G_FreeEdict(self);
}

void SP_target_actor (edict_t *self)
{
	G_FreeEdict(self);
}

// m_berserk.c

void SP_monster_berserk (edict_t *self)
{
	G_FreeEdict(self);
}

// m_boss2.c

void SP_monster_boss2 (edict_t *self)
{
	G_FreeEdict(self);
}

// m_boss3.c

void SP_monster_boss3_stand (edict_t *self)
{
	G_FreeEdict(self);
}

// m_boss31.c

void SP_monster_jorg (edict_t *self)
{
	G_FreeEdict(self);
}

// m_boss32.c

void SP_monster_makron (edict_t *self)
{
	G_FreeEdict(self);
}

// m_brain.c

void SP_monster_brain (edict_t *self)
{
	G_FreeEdict(self);
}

// m_chick.c

void SP_monster_chick (edict_t *self)
{
	G_FreeEdict(self);
}

// m_flipper.c

void SP_monster_flipper (edict_t *self)
{
	G_FreeEdict(self);
}

// m_float.c

void SP_monster_floater (edict_t *self)
{
	G_FreeEdict(self);
}

// m_flyer.c

void SP_monster_flyer (edict_t *self)
{
	G_FreeEdict(self);
}

// m_gladiator.c

void SP_monster_gladiator (edict_t *self)
{
	G_FreeEdict(self);
}

// m_gunner.c

void SP_monster_gunner (edict_t *self)
{
	G_FreeEdict(self);
}

// m_hover.c

void SP_monster_hover (edict_t *self)
{
	G_FreeEdict(self);
}

// m_infantry.c

void SP_monster_infantry (edict_t *self)
{
	G_FreeEdict(self);
}

// m_insane.c

void SP_misc_insane (edict_t *self)
{
	G_FreeEdict(self);
}

// m_medic.c

void SP_monster_medic (edict_t *self)
{
	G_FreeEdict(self);
}

// m_mutant.c

void SP_monster_mutant (edict_t *self)
{
	G_FreeEdict(self);
}

// m_parasite.c

void SP_monster_parasite (edict_t *self)
{
	G_FreeEdict(self);
}

// m_soldier.c

void SP_monster_soldier_light (edict_t *self)
{
	G_FreeEdict(self);
}

void SP_monster_soldier (edict_t *self)
{
	G_FreeEdict(self);
}

void SP_monster_soldier_ss (edict_t *self)
{
	G_FreeEdict(self);
}

// m_supertank.c

void SP_monster_supertank (edict_t *self)
{
	G_FreeEdict(self);
}

// m_tank.c

void SP_monster_tank (edict_t *self)
{
	G_FreeEdict(self);
}
