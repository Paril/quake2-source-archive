#include "g_local.h"
#include "q_devels.h"

qboolean CheckDoomRadius (edict_t *ent)
{
      edict_t *blip = NULL;

       while ((blip = findradius(blip, ent->s.origin, 250)) != NULL)
       {

               if ((blip->monsterinfo.stand != NULL) && (blip->health > 0))
               {
               return false;
               }
               if ((blip->player > 0) && (blip->health > 0))
               {
               return false;
               }

       }
return true;
}


void DoomSpawn_think (edict_t *self)
{
int arpa;
edict_t *ent;
arpa = 1 + (int)(random() * 15.0);
arpa = (int) arpa;

if (CheckDoomRadius(self)) {
ent = G_Spawn();
VectorCopy(self->s.origin, ent->s.origin);
VectorCopy(self->s.angles, ent->s.angles);

gi.WriteByte (svc_muzzleflash);
gi.WriteShort (ent-g_edicts);
gi.WriteByte (MZ_LOGIN);
gi.multicast (ent->s.origin, MULTICAST_PVS);

if (arpa == 1)
SP_monster_berserk (ent);

if (arpa == 2)
SP_monster_brain (ent);

if (arpa == 3)
SP_monster_chick (ent);

if (arpa == 4)
SP_monster_floater (ent);

if (arpa == 5)
SP_monster_flyer (ent);

if (arpa == 6)
SP_monster_gladiator (ent);

if (arpa == 7)
SP_monster_gunner (ent);

if (arpa == 8)
SP_monster_hover (ent);

if (arpa == 9)
SP_monster_infantry (ent);

if (arpa == 10)
SP_monster_soldier_ss (ent);

if (arpa == 11)
SP_monster_parasite (ent);

if (arpa == 12)
SP_monster_soldier_light (ent);

if (arpa == 13)
SP_monster_soldier (ent);

if (arpa == 14)
SP_monster_soldier_ss (ent);

if (arpa == 15)
SP_monster_tank (ent);
}

self->nextthink = level.time + 15 + (int)(random() * 20.0);
}


void ds_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        G_FreeEdict (self);

}



void DoomSpawngl_think (edict_t *self)
{
int arpa;
edict_t *ent;
arpa = 1 + (int)(random() * 15.0);
arpa = (int) arpa;

if (self->timetospawn < level.time) {

if (CheckDoomRadius(self)) {
ent = G_Spawn();
VectorCopy(self->s.origin, ent->s.origin);
VectorCopy(self->s.angles, ent->s.angles);

gi.WriteByte (svc_muzzleflash);
gi.WriteShort (ent-g_edicts);
gi.WriteByte (MZ_LOGIN);
gi.multicast (ent->s.origin, MULTICAST_PVS);

ent->s.origin[2] = self->s.origin[2]+64;
if (arpa == 1)
SP_monster_berserk (ent);

if (arpa == 2)
SP_monster_brain (ent);

if (arpa == 3)
SP_monster_chick (ent);

if (arpa == 4)
SP_monster_floater (ent);

if (arpa == 5)
SP_monster_flyer (ent);

if (arpa == 6)
SP_monster_gladiator (ent);

if (arpa == 7)
SP_monster_gunner (ent);

if (arpa == 8)
SP_monster_hover (ent);

if (arpa == 9)
SP_monster_infantry (ent);

if (arpa == 10)
SP_monster_soldier_ss (ent);

if (arpa == 11)
SP_monster_parasite (ent);

if (arpa == 12)
SP_monster_soldier_light (ent);

if (arpa == 13)
SP_monster_soldier (ent);

if (arpa == 14)
SP_monster_soldier_ss (ent);

if (arpa == 15)
SP_monster_tank (ent);
}
self->timetospawn = level.time + 15 + (int)(random() * 20.0);
}
               
//if (self->health < 1) Grenade_Explode(self);
self->nextthink = level.time +.1;
}




void SP_func_DoomSpawn (edict_t *self)
{
        self->movetype = MOVETYPE_NONE;
        self->solid = SOLID_NOT;
        self->think = DoomSpawn_think;
        self->nextthink = level.time + 15 + (int)(random() * 20.0);

        gi.linkentity (self);
}


void SP_func_DoomSpawngl (edict_t *owner)
{
	vec3_t		forward;
//				wallp;

//	trace_t		tr;
//	gitem_t		*item;
      edict_t *self;


      self = G_Spawn();

      AngleVectors(owner->client->v_angle, forward, NULL, NULL);
      VectorMA(owner->s.origin, 100, forward, self->s.origin);
      self->s.angles[PITCH] = owner->s.angles[PITCH];
     // self->s.angles[YAW] = owner->s.angles[YAW];
      self->s.angles[ROLL] = owner->s.angles[ROLL];

	gi.WriteByte (svc_muzzleflash);
        gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_LOGIN);
        gi.multicast (self->s.origin, MULTICAST_PVS);
        self->movetype = MOVETYPE_TOSS;
        self->clipmask = MASK_SHOT;
        self->solid = SOLID_BBOX;
        self->die = ds_die;
        self->think = DoomSpawngl_think;
        self->health = 250;
        self->s.modelindex = gi.modelindex ("models/objects/dmspot/tris.md2");
        self->s.effects |= EF_TELEPORTER;
        self->s.skinnum = 1;

        self->nextthink = level.time + 15 + (int)(random() * 20.0);
        VectorSet (self->mins, -24, -24, -24);
        VectorSet (self->maxs, 24, 24, -16);
	self->takedamage = DAMAGE_AIM;
        self->mass = 1000;
        self->timetospawn = level.time + 15 + (int)(random() * 20.0);

        gi.linkentity (self);
}

