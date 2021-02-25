/*  funball mod for gloom by tachikoma

*****

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*****

Patching gloom:

g_local.h:
        add globals of the mod:

void fun_breeder_bite(edict_t*breeder, trace_t*tr, vec3_t forward);
void fun_prespawn(edict_t*ent, int*ct);
void fun_postclient(edict_t*ent);
void SP_fun_ball(edict_t*ent);
void SP_fun_goal(edict_t*ent);
void SP_fun_digit(edict_t*ent);


g_breakin.c:
        add fun_prespawn:

        } else if (ent->pain_debounce_time + 2 > level.time && ent->health > 0) {
                gi.cprintf (ent, PRINT_MEDIUM, "Can't respawn immediately after taking damage.\n");
                return false;
        }
        
        fun_prespawn(ent, &class_type);

        if(!sv_cheats->value)
        if(i != 0)
        if(ent->client->resp.score < i)


g_spawn.c:
        add to spawn list:
        
        {"fun_ball", SP_fun_ball},
        {"fun_goal", SP_fun_goal},
        {"fun_digit", SP_fun_digit},

P_weapon.c:
        add fun_breeder_bite:

                        ent->s.frame = 77;
                        client->anim_end = 85;
                        fun_breeder_bite(ent, &tr, forward);
                        if (tr.ent) {
                                if (tr.ent->client)

        allow breeder on h team to work:
        
        if(ent->client->resp.team == TEAM_ALIEN || ent->client->resp.class_type == CLASS_BREEDER)
        {
                // aliens don't have normal weapons
                AlienWeaponThink(ent);

p_view.c:
        add fun_postclient:

        G_SetClientFrame (ent);
        fun_postclient(ent);
#ifdef R1Q2_BUILD
        if (basetest->value) {
                VectorCopy (ent->mins, ent->client->ps.mins);


m_depot.c:
        fix crash if human breeder touches depot:

                        int response;
                        if (!(self->spawnflags & 4) && classlist[other->client->resp.class_type].healfunc) {
                                response = classlist[other->client->resp.class_type].healfunc(other, NULL);
                                if (response == 1) {
                                        if (!(self->spawnflags & 16)) {

        replacing
                if (!(self->spawnflags & 4)) {
        with                            
                if (!(self->spawnflags & 4) && classlist[other->client->resp.class_type].healfunc) {
        avoids calling NULL on breeder

Makefile:
        just add this patch to be compiled too at the end of the list
p_menu.o \
p_shotgun.o \
q_shared.o \
turret.o \
fun.o


TODO/FIXME:
 - h needs ent->client->ps.rdflags |= RDF_IRGOGGLES;
 - h has flashlight instead of alien light

*/








#include "g_local.h"
#include "m_player.h"
#undef MODEL_SCALE
#include "m_cocoon.h"

#define ENT_FUNBALL 64

static int sound_hit;
static int sound_taunt;
static int sound_hurts;

static int scores[2];
static int digits[10];

static float restarttime;
static char  healthrestart[68];
static unsigned char stickcounter;  //weird bug of MOVE_STEP sometimes sticks on walls

static vec3_t up = {0,0,1};

static void globalsound(int s, int teammask)
{
  edict_t *ent;
  for (ent = g_edicts+1; ent <= g_edicts+game.maxclients; ent++)
    if (ent->inuse && (1 <<ent->client->resp.team) & teammask)
      unicastSound(ent, s, 1);
}

static void updatescores()
{
  char scoretext[8];
  edict_t*ent = NULL;
  snprintf(scoretext, 8, "%03d%03d", scores[0] & 255, scores[1] & 255);
  while (ent = G_Find(ent, FOFS(classname), "fun_digit")) {
    int n = scoretext[ent->count & 7]-48;
    if (n >= 0 && n < 10) {
      ent->s.modelindex = digits[n];
      gi.linkentity(ent);
    }
  }
}

static void getdigitmodels()
{
  int i;
  for (i = 0; i < 10; i++) {
    edict_t*ent = G_Find(NULL, FOFS(targetname), va("%d", i));
    if (!ent) gi.dprintf("WARNING: digit %d missing\n", i);
    digits[i] = ent?ent->s.modelindex?ent->s.modelindex:gi.modelindex(ent->model):0;
  }
}

void fun_breeder_bite(edict_t*breeder, trace_t*tr, vec3_t forward)
{
  if (tr->ent->enttype == ENT_FUNBALL) {
    VectorMA(tr->ent->velocity, 800.0/VectorLength(forward), forward, tr->ent->velocity);
    VectorMA(tr->ent->velocity, 0.75f, breeder->velocity, tr->ent->velocity);
    gi.sound(tr->ent, CHAN_AUTO, sound_hit, 1, ATTN_NORM, 0);
    tr->ent->enemy = breeder;
    tr->ent = NULL;
  }
}



void fun_prespawn(edict_t*ent, int*ct)
{
  if (world->teammaster && world->teammaster->enttype == ENT_FUNBALL) {
    if (*ct != CLASS_OBSERVER) {
      *ct = CLASS_BREEDER;
    }
  }
}

void fun_postclient(edict_t*ent)
{
  if (world->teammaster && world->teammaster->enttype == ENT_FUNBALL) {
    ent->s.effects |= (ent->client->resp.team == TEAM_ALIEN)?EF_FLAG1:EF_FLAG2;
  }
}


static void ball_regen(edict_t*ent, edict_t*inflictor, edict_t*attacker, int damage, vec3_t point)
{
  ent->health = ent->max_health*(0.75+0.05*crandom());
  gi.sound(ent, CHAN_AUTO, SoundIndex(items_l_health), 1.0, ATTN_NORM,0);
  ent->dmg *= 0.5;
  if (ent->dmg < 50) ent->dmg = 50;

  ent->s.frame = (randomMT()&1)?FRAME_pain101:FRAME_pain201;

  if (ent->pain_debounce_time < level.time+1) {
    ent->pain_debounce_time = level.time+5;
    gi.sound(ent, CHAN_AUTO, (randomMT()&7 < 5)?sound_hurts:sound_taunt, 1, ATTN_NORM, 0);
  }
}

static void ball_pain(edict_t*ent, edict_t*other, float kick, int damage)
{
  while (other->owner && !other->client) other = other->owner;
  
  if (other->client) ent->enemy = other;

  if (ent->s.frame <= FRAME_stand40) ent->s.frame = (randomMT()&1)?FRAME_pain101:FRAME_pain201;

  if (damage > 10 && ent->pain_debounce_time < level.time && randomMT()&7>=5) {
    gi.sound(ent, CHAN_AUTO, sound_hurts, 1, ATTN_NORM, 0);
    ent->pain_debounce_time = level.time+5;
  }

  if (ent->health < ent->max_health - 80) {
    ent->health += 80;
    gi.sound(ent, CHAN_AUTO, SoundIndex(items_l_health), 1, ATTN_NORM, 0);
    ent->dmg *= 0.8;
  }
}

static void ball_animate(edict_t*ent)
{
  vec3_t dir;
  float speed;
  
  if (!ent->s.modelindex) {
    if (restarttime) {
      edict_t*p;
      float f = (restarttime-level.time)*0.2f;
      for (p = g_edicts+1; p <= g_edicts+game.maxclients; p++)
         if (p->inuse && p->health > 0 && p->client && p->client->resp.class_type != CLASS_OBSERVER) {
           int h = healthrestart[p-g_edicts-1]*f + p->max_health*(1-f);
           if (p->health < h) p->health = h;
         }
    }

    ent->speed = 0;
    VectorClear(ent->pos2);
    return;
  }
  
  ent->gravity = 1;

  ent->s.frame++;
  if (ent->s.frame == FRAME_stand40+1 || ent->s.frame == FRAME_pain104+1 || ent->s.frame == FRAME_pain204+1 || ent->s.frame == FRAME_taunt17+1)
    ent->s.frame = FRAME_stand01;
  
  VectorSubtract(ent->s.origin, ent->pos1, dir);
  dir[2] = 0;
  if (VectorLength(dir) > 64) {
    dir[1] *= 2;
    vectoangles(dir, ent->s.angles);
  }
  
  speed = VectorLength(ent->velocity);
  if (fabs(speed-ent->speed) > 192) {
    float diff = fabs(speed-ent->speed);
    vec3_t tmp;
    VectorNormalize2(ent->pos2, tmp);
    if (DotProduct(tmp, up) < -0.3 && ent->pos2[2] < -160) {
      trace_t tr;
      vec3_t end;
      VectorNormalize2(ent->pos2, tmp);
      VectorMA(ent->s.origin, 8, tmp, end);
      tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_SHOT);
      if (tr.fraction < 1.0 && tr.ent && !tr.ent->takedamage) {
        vec3_t dir;
        VectorNormalize2(ent->velocity, dir);
        T_Damage(ent, tr.ent, world, dir, tr.endpos, tr.plane.normal, diff*0.05-5, 0, DAMAGE_IGNORE_RESISTANCES | DAMAGE_NO_ARMOR, MOD_FALLING);
        if (diff>500 && ent-g_edicts < 768) ent->s.event = EV_FALLFAR; //make human crunching sound, but only if ent happens to have a valid pointer to configstring
      }
    }
  }
  ent->speed = speed;
  VectorCopy(ent->velocity, ent->pos2);
  if (ent->groundentity) ent->pos2[2] = 0; //don't allow make crunch sound pushing downwards when already on floor

  if (!speed) { //bugfix: sometimes gets stuck into walls when pushed by players. Havent managed to reproduce that one yet, but may be this can fix it
    if (++stickcounter == 25) {  //check after 2.5 seconds since last movement in case it got stuck
      trace_t tr;
      vec3_t end = {ent->s.origin[0], ent->s.origin[0], ent->s.origin[2]-1};
      tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_SHOT);
      if (tr.startsolid || tr.fraction == 1.0f) { //stuck in solid, or floating stuck somehow
        VectorCopy(ent->pos1, ent->s.origin);
        ent->s.event = EV_PLAYER_TELEPORT;
        ent->groundentity = NULL;
        ent->velocity[2] = -8;
      }
    } else
    if (stickcounter >= 49) //after 7.5 sec check again just in case
      stickcounter = 0;
  } else
    stickcounter = 0;

  if (!ent->groundentity || speed > 8) {
    ent->dmg = 80+(randomMT()&255);
  } else
  if (ent->dmg > 0) {
    if (!--ent->dmg) {
      if (ent->groundentity && ent->s.frame <= FRAME_stand40) {
        gi.sound(ent, CHAN_AUTO, sound_taunt, 1, ATTN_NORM, 0);
        ent->s.frame = FRAME_taunt01;
        ent->dmg = 120+(randomMT()&511);
      } else
       ent->dmg = 7+(randomMT()&7);
    }
  }

}

static void ball_respawn(edict_t*ent)
{
  edict_t*p;

  if (ent->count) {
    if (ent->count <= 5) {
      if (ent->count == 5) {
        updatescores();

        restarttime = level.time+5;
        for (p = g_edicts+1; p <= g_edicts+game.maxclients; p++)
           if (p->inuse && p->health > 0 && p->client && p->client->resp.class_type != CLASS_OBSERVER)
             healthrestart[p-g_edicts-1] = p->health;
           else
             healthrestart[p-g_edicts-1] = 0;
      }
      centerprint_all(va("%sstart in %d", (scores[0]|scores[1])?"Re":"Match ", ent->count));
      globalsound(SoundIndex(weapons_targbeep), -1);
    }
    ent->count--;
    ent->nextthink = level.time+1;
  } else {
    trace_t tr;
    vec3_t down = {ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]-1024};
    gi.trace(ent->s.origin, ent->mins, ent->maxs, down, ent, MASK_SHOT);
    if (!tr.startsolid) VectorCopy(tr.endpos, ent->s.origin);
    KillBox(ent);
    centerprint_all(va("GO"));
    ent->solid = SOLID_BBOX;
    ent->movetype = MOVETYPE_STEP;
    ent->clipmask = MASK_SHOT;
    ent->svflags |= SVF_MONSTER;
    ent->s.event = EV_PLAYER_TELEPORT;
    ent->takedamage = DAMAGE_YES;
    ent->health = ent->max_health = 100;
    ent->s.modelindex = gi.modelindex("players/male/tris.md2");
    ent->s.modelindex2 = gi.modelindex("players/male/autogun.md2");
    ent->s.angles[1] = crandom()*180;
    ent->enemy = NULL;
    ent->dmg = 50+(randomMT()&255);
    stickcounter = 0;
    gi.linkentity(ent);
    globalsound(SoundIndex(weapons_magshot), -1);

    for (p = g_edicts+1; p <= g_edicts+game.maxclients; p++)
      if (p->inuse && p->health > 0 && p->client && p->client->resp.class_type != CLASS_OBSERVER)
        p->health = p->max_health;

    restarttime = 0;
  }
}

static void tele_animate(edict_t*ent)
{
  if (++ent->s.frame >= LARGE_E)
    ent->s.frame = LARGE_S;
}

static void spawn_new_ball(edict_t*owner);

static void check_unexplained_free(edict_t*ent)
{
  //something on the game frees entities for no apparent reason, need to check and respawn it when it disappears

  if (!ent->enemy->inuse || ent->enemy->enttype != ENT_FUNBALL)
    spawn_new_ball(ent);

  ent->nextthink = level.time+3;
}

static void spawn_new_ball(edict_t*owner)
{
  edict_t*ent;

  owner->enemy = ent = G_Spawn();
  VectorCopy(owner->s.origin, ent->s.origin);

  ent->classname = "ball";
  ent->enttype = ENT_FUNBALL;

  ent->solid = SOLID_NOT;
  ent->s.renderfx = RF_IR_VISIBLE;
  VectorSet(ent->mins, -16, -16, -24);
  VectorSet(ent->maxs, 16, 16, 32);

  ent->prethink = ball_animate;

  ent->pain = ball_pain;
  ent->die = ball_regen;

  VectorCopy(ent->s.origin, ent->pos1);

  ent->count = 5;
  ent->think = ball_respawn;
  ent->nextthink = level.time+1;

  owner->think = check_unexplained_free;
  owner->nextthink = level.time+10;

  gi.linkentity(ent);
}

static void funball_post(edict_t*ent)
{
  int egg = gi.modelindex("models/objects/cocoon/tris.md2");

  world->teammaster = ent;
  
  getdigitmodels();

  //buildpoints in this map make no sense, humans can't build own structures, but alien ones instead  
  team_info.points[TEAM_ALIEN] = team_info.maxpoints[TEAM_ALIEN] = 0;
  team_info.points[TEAM_HUMAN] = team_info.maxpoints[TEAM_HUMAN] = 0;
  
  scores[0] = scores[1] = 0;

  ent->think = spawn_new_ball;
  ent->nextthink = team_info.starttime-1;

  ent = NULL; 
  while (ent = G_Find(ent, FOFS(classname), "info_player_deathmatch")) {
    ent->prethink = tele_animate;
    ent->s.modelindex = egg;
    ent->s.frame = SEED_S;
    ent->s.effects = 0;
    ent->s.origin[2] -= 8;
    ent->mins[2] += 8;
    ent->maxs[2] += 8;
  }
}


void SP_fun_ball(edict_t*ent)
{
  ent->enttype = ENT_FUNBALL;
  ent->classname = "ball safeguard";
  
  sound_hit = gi.soundindex("insane/insane11.wav");
  sound_taunt = gi.soundindex("insane/insane5.wav");
  sound_hurts = gi.soundindex("insane/insane1.wav");
  
  VectorSet(ent->mins, -16, -16, -24);
  VectorSet(ent->maxs, 16, 16, 32);

  ent->think = funball_post;
  ent->nextthink = level.time+1;

  CheckSolid(ent);
  
}

static void goal_touch(edict_t*ent, edict_t*other, cplane_t*plane, csurface_t*surf)
{
  int i;
  qboolean owngoal = false;
  int team = 2-(ent->spawnflags & 1); //flag off: alien side goal alrea, on: humans side goal area
  
  if (other->client && other->health > 0 && other->health < other->max_health && other->client->resp.team != team && other->pain_debounce_time < level.time-3) {
    other->health++;
    other->pain_debounce_time = level.time-3;
  }

  if (other->enttype != ENT_FUNBALL || !other->s.modelindex) return;

  other->solid = SOLID_NOT;
  gi.unlinkentity(other);
  VectorCopy(other->pos1, other->s.origin);
  VectorClear(other->velocity); other->velocity[2] = -64;
  other->s.event = EV_OTHER_TELEPORT;
  other->s.modelindex = 0;
  other->s.modelindex2 = 0;

  centerprint_all(va("%s\nSCORED A GOAL FOR THE\n%s TEAM!",
    (other->enemy && other->enemy->client)?other->enemy->client->pers.netname:"SOMETHING",
    (team==1)?"RED":"BLUE"
  ));
  scores[team-1]++;
  updatescores();
  
  globalsound(SoundIndex(voice_toastie), (team==1)?4:2);
  globalsound(SoundIndex(misc_keytry), (team==1)?3:5);

  if (other->enemy && other->enemy->client) {
    edict_t*player = other->enemy;
    if (player->client->resp.team == team) {
      player->client->resp.score++;
      player->client->resp.total_score += 5;
    } else {
      player->client->resp.score--;
      player->client->resp.total_score -= 5;
      owngoal = true;
    }
  }


  for (i = 0; i < 4; i++)    
    ThrowGib(other, gi.modelindex("models/objects/gibs/sm_meat/tris.md2"), 350, GIB_ORGANIC);
  gi.positioned_sound(ent->s.origin, ent, CHAN_AUTO, SoundIndex(misc_ssdie), 1, ATTN_NORM, 0);
  if (randomMT()&7 == 7)
    gi.positioned_sound(ent->s.origin, ent, CHAN_AUTO, sound_hurts, 1, ATTN_NORM, 0);

  other->enemy = NULL;

  other->count = 7;
  other->think = ball_respawn;
  other->nextthink = level.time+1;
  
  for (ent = g_edicts+1; ent <= g_edicts+game.maxclients; ent++) if (ent->inuse && ent->client && ent->client->resp.team != TEAM_NONE) {
    if (ent->client->resp.team == team)
      ent->client->resp.total_score += owngoal?5:10;
    else
    if (owngoal) {
      if (ent->client->resp.total_score > 0)
        ent->client->resp.total_score -= 5;
      else
        ent->client->resp.total_score = 0;
    }
  }
}


void SP_fun_goal(edict_t*ent)
{
  ent->solid = SOLID_TRIGGER;
  ent->touch = goal_touch;
  ent->movetype = MOVETYPE_NONE;
  ent->svflags |= SVF_NOCLIENT;

  gi.setmodel(ent, ent->model);

  gi.linkentity (ent);
}

void SP_fun_digit(edict_t*ent)
{
  //entities to be modified by updatescores()
}
