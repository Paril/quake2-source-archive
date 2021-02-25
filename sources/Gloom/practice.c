#include "g_local.h"


//renamed field recycling
#define failcount dmg
#define classtype radius_dmg
#define animstart gib1
#define animend gib2
#define bloodlimit random
#define exshelltime air_finished
#define exshellcount noise_index

int practice_target_hits;

static void SetModel(edict_t*t)
{
  const gclass_t*c;

  t->s.renderfx &= ~RF_IR_VISIBLE;
  if (!t->solid) {
    t->s.modelindex = gi.modelindex("models/objects/flash/tris.md2");
    t->s.frame = 0;
    t->s.skinnum = 0;
    t->s.sound = 0;
    t->s.modelindex2 = 0;
    t->s.effects = 0;
    t->s.renderfx = 0;
    return;
  }

  if (t->spawnflags & 8) {
    if (t->classtype != CLASS_KAMIKAZE)
      t->s.effects |= EF_DOUBLE;
    else
      t->s.renderfx |= RF_SHELL_RED;
  }
  if (t->spawnflags & 16) t->s.renderfx |= RF_SHELL_RED;
  if (t->spawnflags & 32) t->s.renderfx |= RF_SHELL_GREEN;
  if (t->spawnflags & 64) t->s.renderfx |= RF_SHELL_BLUE;
  
  if (t->s.renderfx & (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE))
    t->s.effects |= EF_COLOR_SHELL;

  if (t->classtype == -1) {
    t->s.modelindex = gi.modelindex("models/objects/smokexp/tris.md2");
    return;
  }
  
  t->s.renderfx |= RF_IR_VISIBLE;

  c = classlist+t->classtype;
  
  t->s.modelindex = gi.modelindex(c->model);
  
  if (t->classtype == CLASS_GRUNT)
    t->s.modelindex2 = gi.modelindex("players/male/autogun.md2");
  else
  if (t->classtype == CLASS_SHOCK) {
    t->s.skinnum = 1;
    t->s.modelindex2 = gi.modelindex("players/male/shotgun.md2");
  } else
  if (t->classtype == CLASS_BIO)
    t->s.modelindex2 = gi.modelindex("players/female/weapon.md2");
  else
  if (t->classtype == CLASS_HEAVY)
    t->s.modelindex2 = gi.modelindex("players/hsold/weapon.md2");
  else
  if (t->classtype == CLASS_COMMANDO) {
    t->s.skinnum = 2;
    t->s.modelindex2 = gi.modelindex("players/male/smg.md2");
  } else
  if (t->classtype == CLASS_KAMIKAZE) t->s.skinnum = 1;
  else
  if (t->classtype == CLASS_BREEDER && (t->spawnflags & 1024))
    t->s.modelindex = gi.modelindex("players/breeder2/tris.md2");
}

static void SetSparks(edict_t*t)
{
  if (t->health > t->bloodlimit || t->classtype == CLASS_ENGINEER || t->classtype == CLASS_MECH || t->classtype == CLASS_EXTERM)
    t->svflags &= ~SVF_MONSTER;  //armour sparks for next damage
  else
    t->svflags |= SVF_MONSTER;  //blood sparks for next damage
}

static void target_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  trace_t tr;
  if (!plane) {
    vec3_t a, b;
    VectorMA(self->s.origin, -0.5f, self->velocity, a);
    VectorMA(self->s.origin, +0.5f, self->velocity, b);
    tr = gi.trace(a, self->mins, self->maxs, b, self, self->clipmask);
    plane = &tr.plane;
  }

  VectorNormalize(self->velocity);
  VectorAdd(self->velocity, plane->normal, self->velocity);
  VectorNormalize(self->velocity);
  VectorScale(self->velocity, self->speed, self->velocity);
  
  if (self->classtype) {
    vectoangles(self->velocity, self->s.angles);
    if (plane->normal[2] > 0.7) {
      self->velocity[2] = classlist[self->classtype].jumpheight[0];
//      gi.sound(self, CHAN_AUTO, gi.soundindex(va("players/%s/jump1.wav", ...)), 1, ATTN_IDLE, 0); FIXME: some "jump" sound
    }
  }
  
  if (other->client && (self->spawnflags & 4) && other->client->resp.team == TEAM_HUMAN && self->touch_debounce_time < level.time) {
    vec3_t dir, point;
    VectorSubtract(self->s.origin, other->s.origin, dir);
    VectorNormalize(dir);
    point[0] = self->s.origin[0] + self->maxs[0]*dir[0];
    point[1] = self->s.origin[1] + self->maxs[1]*dir[1];
    if (dir[2] > 0)
      point[0] = self->s.origin[2] + self->maxs[2]*dir[2];
    else
      point[0] = self->s.origin[2] - self->mins[2]*dir[2];
    if (T_Damage(other, self, self, dir, point, plane->normal, (self->spawnflags&1)?25:56, 40, 0, MOD_MELEE)) {
      self->touch_debounce_time = level.time+0.2f;
      gi.sound(self, CHAN_AUTO, SoundIndex(mutant_mutatck2), 1, ATTN_NORM, 0);
    } else
      gi.sound(self, CHAN_AUTO, SoundIndex(mutant_mutatck1), 1, ATTN_NORM, 0);
  }
}

static void target_think(edict_t *self)
{
  trace_t tr;
  vec3_t end;
  qboolean randomize = VectorLength(self->velocity) <= 0.9*self->speed;

  if (self->spawnflags & 2) {
    self->velocity[2] -= sv_gravity->value*0.05f;
    randomize = randomize && self->groundentity;
  }

  if (self->s.angles[0] < 4 || self->s.angles[0] > 340)
    self->s.angles[0] = 0;
  else
  if (self->s.angles[0] < 90)
    self->s.angles[0] -= 4;
  else
    self->s.angles[0] += 4;
  
  VectorMA(self->s.origin, FRAMETIME, self->velocity, end);
  tr = gi.trace(self->s.origin, self->mins, self->maxs, end, self, self->solid?MASK_SHOT:MASK_SOLID);
  if (!tr.startsolid) {
    VectorCopy(tr.endpos, self->s.origin);
    gi.linkentity(self);
    if (tr.fraction < 1)
      target_touch(self, tr.ent, &tr.plane, tr.surface);
  } else
    randomize = true;

  self->nextthink = level.time+0.1f;
  if (self->fly_sound_debounce_time && self->fly_sound_debounce_time < level.time) {
    if (self->spawnflags & 8192) {
      edict_t*e;
      edict_t*swap = NULL;
      int best = -1;
      for (e = g_edicts+game.maxclients+1; e < g_edicts+globals.num_edicts; e++) if (e->enttype == ENT_PRACTICE_TARGET && e != self) {
        int r = randomMT()&1023;
        if (r > best) {
          trace_t tr = gi.trace(e->s.origin, self->mins, self->maxs, e->s.origin, e, MASK_SHOT);
          trace_t tr2 = gi.trace(self->s.origin, e->mins, e->maxs, self->s.origin, self, MASK_SHOT);
          if (!tr.allsolid && !tr2.allsolid) {
            best = r;
            swap = e;
          }
        }
      }
    
      if (swap) {
        vec3_t p;

        VectorCopy(swap->s.origin, p);
        VectorCopy(self->s.origin, swap->s.origin);
        VectorCopy(p, self->s.origin);
        RandomVector(self->velocity, 300);
        RandomVector(swap->velocity, 300);
        self->failcount = 0;
      } else
      if (self->failcount < 6) {
        self->failcount++;
        return;
      }
    }

    trace_t tr = gi.trace(self->s.origin, self->mins, self->maxs, self->s.origin, NULL, MASK_SHOT);
    if (tr.allsolid) {
      self->fly_sound_debounce_time = level.time+.35f*(2.5+crandom());
      self->failcount += 2;
      if (self->failcount == 20) {
  	temp_point(TE_EXPLOSION2, self->s.origin);
  	gi.multicast(self->s.origin, MULTICAST_PVS);
  	if (self->spawnflags & 8192)
          gi.bprintf(PRINT_HIGH, "The %s has imploded!\n", colortext(classlist[self->classtype].classname));
    	G_FreeEdict(self);
      }
      return;
    }
    self->fly_sound_debounce_time = 0;
    self->takedamage = true;
    if (self->maxs[2] < 20)
      self->s.event = EV_ITEM_RESPAWN;
    else
      self->s.event = EV_PLAYER_TELEPORT;
    self->solid = SOLID_BBOX;
    SetModel(self);
    SetSparks(self);
    randomize = true;
  }
  
  if (randomize) {
    self->speed = 300;
    self->velocity[0] = crandom();
    self->velocity[1] = crandom();
    self->velocity[2] = crandom();
    VectorNormalize(self->velocity);
    VectorScale(self->velocity, self->speed, self->velocity);
  }

}


static void target_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
  if (!attacker || attacker->solid == SOLID_BSP || attacker->client && !attacker->client->resp.team) {
    temp_point(TE_BFG_BIGEXPLOSION, self->s.origin);
    gi.multicast(self->s.origin, MULTICAST_PVS);
    gi.positioned_sound(self->s.origin, self, CHAN_AUTO, gi.soundindex("bugport/bidmix9.wav"), 1, ATTN_NORM, 0);
//    gi.positioned_sound(self->s.origin, self, CHAN_AUTO, gi.soundindex("weapons/xpld_wat.wav"), 1, ATTN_NORM, 0);

    if (self->spawnflags & 8192)
      gi.bprintf(PRINT_HIGH, "The %s has been exterminated!\n", colortext(classlist[self->classtype].classname));

    G_FreeEdict(self); //If killed by world or obs (normally admin), disappear
    return;
  }
  
  if (attacker->client) {
    if (self->maxs[0] > 8)
      attacker->client->resp.total_score++; //easy
    else
      attacker->client->resp.total_score += 2; //normal
  }

  practice_target_hits++;
  
  if (meansOfDeath == MOD_EXSHELL) {
    qboolean die = false;

    if (level.time > self->exshelltime)
      self->exshellcount = 0;

    self->exshellcount++;

    self->health = self->max_health;
    if (self->classtype == CLASS_DRONE) {
      die = self->exshellcount == 2;
      self->health = 50;
    } else
    if (self->classtype == CLASS_BREEDER) {
      die = self->exshellcount == 2;
      self->health = 10;
    } else
    if (self->classtype == CLASS_STINGER)
      die = self->exshellcount == 3;
    else
    if (self->classtype == CLASS_STINGER)
      die = self->exshellcount == 5;
    else
    if (self->classtype == CLASS_GUARDIAN)
      die = self->exshellcount == 10;

    self->exshelltime = level.time+3.5f;
    if (!die) return;
    self->exshellcount = 0;
  } else
  if (self->exshellcount > 0) {
    self->exshellcount -= damage*70;
    if (self->exshellcount < 0) self->exshellcount = 0;
  }

  temp_point(TE_EXPLOSION2, self->s.origin);
  gi.multicast(self->s.origin, MULTICAST_PVS);
  
  self->failcount = 0;

  self->speed = (self->speed+300)*0.5f;
  self->health = self->max_health;
  self->solid = SOLID_NOT;
  self->fly_sound_debounce_time = level.time+1.25f*(2.5+crandom());
  self->velocity[0] = crandom();
  self->velocity[1] = crandom();
  self->velocity[2] = crandom();
  VectorNormalize(self->velocity);
  VectorScale(self->velocity, self->speed, self->velocity);
  self->takedamage = false;
  SetModel(self);
  
  if (self->spawnflags & 8192) {
    self->fly_sound_debounce_time = level.time+1.25f+crandom()*0.5f;
    if (attacker->client) {
      gi.bprintf(PRINT_HIGH, "%s has slain the %s!!\n", colortext(attacker->client->pers.netname), colortext(classlist[self->classtype].classname));
      attacker->client->resp.total_score += 9; //extra
    }
  }
    
  if (self->classtype == CLASS_KAMIKAZE /* && random() < 0.5f*/)
    T_RadiusDamage(self, self, 125, self, 266, MOD_R_SPLASH, 0);
}

static void target_pain(edict_t*self, edict_t *other, float kick, int damage)
{
  int old = self->svflags;
  
  practice_target_hits++;
  SetSparks(self);

  if (!(old & SVF_MONSTER) && (self->svflags & SVF_MONSTER) && self->health < self->bloodlimit && other) {
    vec3_t dir, point;
    VectorSubtract(self->s.origin, other->s.origin, dir);
    VectorNormalize(dir);
    point[0] = self->s.origin[0] + self->maxs[0]*dir[0];
    point[1] = self->s.origin[1] + self->maxs[1]*dir[1];
    if (dir[2] > 0)
      point[0] = self->s.origin[2] + self->maxs[2]*dir[2];
    else
      point[0] = self->s.origin[2] - self->mins[2]*dir[2];
    SpawnDamage(TE_BLOOD, point, dir, self->bloodlimit-self->health);
  }
}




void SP_practice_target(edict_t*t)
{
  t->classname = "practice_target";
  t->enttype = ENT_PRACTICE_TARGET;

  t->speed = 300;
  t->velocity[0] = crandom();
  t->velocity[1] = crandom();
  t->velocity[2] = crandom();
  VectorNormalize(t->velocity);
  VectorScale(t->velocity, t->speed, t->velocity);
  if (!(t->spawnflags & 1)) {
    VectorSet(t->mins, -8, -8, -8);
    VectorSet(t->maxs, 8, 8, 8);
  } else {
    VectorSet(t->mins, -16, -16, -16);
    VectorSet(t->maxs, 16, 16, 16);
    t->s.skinnum = 1; //greenish tint for easy
  }
  t->movetype = MOVETYPE_NONE;
  t->clipmask = MASK_SHOT | MASK_WATER;
  t->solid = SOLID_BBOX;
  t->s.frame = 0;
  t->takedamage = true;
  t->max_health = 5;
  t->touch = target_touch;
  t->pain = target_pain;
  t->die = target_die;
  t->think = target_think;
  t->nextthink = level.time+0.1f;
  
  t->classtype = -1;
  if (t->message) {
    int i;
    for (i = 0; i < CLASS_OBSERVER; i++)
      if (!strcasecmp(classlist[i].classname, t->message)) {
        t->classtype = i;
        VectorCopy(classlist[i].mins, t->mins);
        VectorCopy(classlist[i].maxs, t->maxs);
        t->bloodlimit = t->max_health = classlist[i].health;
        if (classlist[i].armor == ARMOR_COMBAT) t->max_health += 2*classlist[i].armorcount;
        else
        if (classlist[i].armor == ARMOR_FULL) t->max_health += classlist[i].armorcount/0.6f;
        else
          t->max_health += classlist[i].armorcount;
        t->mass = classlist[i].mass;
        t->damage_absorb = classlist[i].absorb;
      }
  }
  
  SetModel(t);  
  SetSparks(t);
  t->health = t->max_health;
  gi.linkentity(t);
  
  if (t->classtype < 0) t->spawnflags &= ~8192;

  if (t->spawnflags & 8192)
    gi.bprintf(PRINT_HIGH, "The %s spawned!\n", colortext(classlist[t->classtype].classname));
  

}
