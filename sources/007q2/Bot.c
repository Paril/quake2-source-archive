#include "g_local.h"
#include "m_player.h"
#include "b_local.h"

// Special thanks to Ponpoko for his brilliant work upon
// which this work is based.

#define MaxOf(x,y) ((x)>(y)?(x):(y))

#define DEG2RAD(a) ((a)*(M_PI/180.0F))

//skill[]
#define AIMACCURACY    0 // 0..9 aiming accuracy
#define AGGRESSION     1 // 0..9
#define COMBATSKILL    2 // 0..9
#define VRANGEVIEW     3 // 60-120 deg - Vertical view range
#define HRANGEVIEW     4 // 60-120 deg - Horizontal view range
#define PRIMARYWEAP    5 // 1..11  - 1=Blaster, 11=BFG

//=====================================================
//=====================================================

vec3_t zvec = {0,0,0};

float myrandom=0.5F;

int TotalRouteNodes=0;
route_t Route[MAXNODES];

botinfo_t Bot[MAXBOTS+1];

qboolean pickup_priority=false;
int trace_priority=0;
float JumpMax=0.0F;

int NumBotsInGame=1; // [1..MAXBOTS]

int SkillLevel[10] = {
  0x00001000|0x10000000|0x00000002|0x00000004,
  0x00001000|0x10000000|0x00000002|0x00000004,
  0x00001000|0x10000000|0x00000002,
  0x00001000|0x10000000|0x00000002,
  0x00001000|0x10000000,
  0x00001000|0x10000000,
  0x00001000|0x10000000,
  0x00001000|0x00000010,
  0x10000000|0x00000010,
  0x10000000|0x00000010
};

typedef char cfg[64];
typedef cfg cfg_t[2];
cfg_t gbot[] = { // ENTRIES MUST == MAXBOTS, else kabooom!
{  "007",      "bond/actionbond"  },
{  "Soldier",    "gijoe/soldier"      },
{  "Henchman",   "gijoe/pvt'rollins"   },
{  "Russian Soldier",    "gijoe/soldier"      },
{  "Jaws",   "gijoe/soldier"   },
{  "Dr.Doaks",  "civilian/scientist"  },
{  "Alec Trevelyan",    "007/actionbond"      },
{  "0DDJ0B",   "oddjob/oddjob"   },
{  "Terrorist","bond/actionbond"  },
{  "Scientist", "civilian/scientist"   },
{  "Xenia Onnatopp", "female/armypants" },
{  "Innocent Civilian",  "civilian/scientist"  },
{  "Innocent Civilian2",  "civilian/scientist"  },
{  "Scientist",  "civilian/scientist"  },
{  "Sergeant",    "gijoe/soldier"      },
{  "Army girl", "female/armypants" },
{  "Female Civilian", "female/buffy" },
{  "Hiker Girl", "female/jacket" },
{  "Business Girl", "female/jackettest" },
{  "Unnamed", "gijoe/lt'dan" },
//{  "Viper",    "male/viper"      },
//{  "Flak",     "male/flak"       },
//{  "Venus",    "female/venus"    },
//{  "NightOps", "male/nightops"   },
//{  "Psycho",   "male/psycho"     },
//{  "Voodoo",   "female/voodoo"   },
/*{  "007",      "007/actionbond"  },
{  "Razor",    "male/razor"      },
{  "Cobalt",   "female/cobalt"   },
{  "Scout",    "male/scout"      },
{  "ps9000",   "cyborg/ps9000"   },
{  "Brianna",  "female/brianna"  },
{  "Recon",    "male/recon"      },
{  "Viper",    "male/viper"      },
{  "oni911",   "cyborg/oni911"   },
{  "Flak",     "male/flak"       },
{  "Venus",    "female/venus"    },
{  "Terrorist","male/terrorist"  },
{  "Pointman", "male/pointman"   },
{  "Stiletto", "female/stiletto" },
{  "Claymore", "male/claymore"   },
{  "Jezebel",  "female/jezebel"  },
{  "Cypher",   "male/cypher"     },
{  "Athena",   "female/athena"   },
{  "Major",    "male/major"      },
{  "Jungle",   "female/jungle"   },
{  "Howitzer", "male/howitzer"   },
{  "Ensign",   "female/ensign"   },
{  "NightOps", "male/nightops"   },
{  "Psycho",   "male/psycho"     },
{  "Voodoo",   "female/voodoo"   },*/
/*{  "Razor",    "male/razor"      },
{  "Cobalt",   "female/cobalt"   },
{  "Scout",    "male/scout"      },
{  "ps9000",   "cyborg/ps9000"   },
{  "Brianna",  "female/brianna"  },
{  "Recon",    "male/recon"      },
{  "Viper",    "male/viper"      },
{  "oni911",   "cyborg/oni911"   },
{  "Flak",     "male/flak"       },
{  "Venus",    "female/venus"    },
{  "Terrorist","male/terrorist"  },
{  "Pointman", "male/pointman"   },
{  "Stiletto", "female/stiletto" },
{  "Claymore", "male/claymore"   },
{  "Jezebel",  "female/jezebel"  },
{  "Cypher",   "male/cypher"     },
{  "Athena",   "female/athena"   },
{  "Major",    "male/major"      },
{  "Jungle",   "female/jungle"   },
{  "Howitzer", "male/howitzer"   },

{  "Ensign",   "female/ensign"   },
{  "NightOps", "male/nightops"   },
{  "Psycho",   "male/psycho"     },
{  "Voodoo",   "female/voodoo"   },*/
/*{  "Rampage",  "male/rampage"    },
{  "Brazen",   "cyborg/tyr574"   },
{  "Zeroid",   "male/razor"      },
{  "Busted",   "female/cobalt"   },
{  "Lotus",    "female/lotus"    },
{  "Grunt",    "male/grunt"      },

{  "Flyte",    "female/venus"    },
{  "Avenger",  "male/pointman"   },
{  "WetSpot",  "female/stiletto" },
{  "Bomber",   "male/claymore"   },
{  "Shaved",   "female/jezebel"  },
{  "BadBoy",   "male/cypher"     },
{  "Nailz",    "female/athena"   },
{  "Sarge",    "male/major"      },
{  "Hairless", "female/jungle"   },
{  "PlowBoy",  "male/howitzer"   },

{  "T1000",    "cyborg/tyr574"   },
{  "Copper",   "male/razor"      },
{  "Queen",    "female/cobalt"   },
{  "Private",  "male/scout"      },
{  "z111x",    "cyborg/ps9000"   },
{  "Virgin",   "female/brianna"  },
{  "Sniper",   "male/recon"      },
{  "Violator", "male/viper"      },
{  "rs3434",   "cyborg/oni911"   },
{  "Metal",    "male/flak"       },

{  "Killer",   "male/grunt"      }, */// Count=51
};
//===================================
//===================================

//======================================================
//========== BASIC BOT UTILITY FUNCTIONS ===============
//======================================================

//======================================================
qboolean G_EntExists(edict_t *ent) {
  return ((ent) && (ent->client) && (ent->inuse));
}

//======================================================
qboolean G_ClientNotDead(edict_t *ent) {
qboolean buried=true;
qboolean b1=(ent->client->ps.pmove.pm_type!=PM_DEAD);
qboolean b2=(ent->deadflag==DEAD_NO);
qboolean b3=(ent->health > 0);
  return ((b3 || b2 || b1) && buried);
}

//======================================================
qboolean G_ClientInGame(edict_t *ent) {
  if (!G_EntExists(ent)) return false;
  if (!G_ClientNotDead(ent)) return false;
  if (ent->client->pers.spectator) return false;
  return (ent->client->respawn_time + 5.0 < level.time);
}

//==============================================
float Get_yaw(vec3_t vec) {
vec3_t out;
double yaw;
  VectorCopy(vec,out);
  out[2] = 0;
  VectorNormalize(out);
  yaw = (double)(acos((double)out[0]))/M_PI*180;
  if (asin((double)out[1])<0) yaw *= -1;
  return (float)yaw;
}

//==============================================
float Get_pitch(vec3_t vec) {
vec3_t out;
float pitch;
  VectorCopy(vec,out);
  VectorNormalize(out);
  pitch = (((float)(acos((double)out[2])))/M_PI*180)-90;
  return (float)((pitch<-180)?(pitch+360):pitch);
}

//======================================================
void AdjustAngle(edict_t *ent, vec3_t targaim, float aim) {

  VectorSet(ent->s.angles,(Get_pitch(targaim)),(Get_yaw(targaim)),0.0F);

  ent->s.angles[1] += aim*0.70*(myrandom-0.5);
  if (ent->s.angles[1] > 180)
    ent->s.angles[1] -= 360;
  else
  if (ent->s.angles[1] < -180)
    ent->s.angles[1] += 360;

  ent->s.angles[0] += aim*0.70*(myrandom-0.5);
  if (ent->s.angles[0] > 90)
    ent->s.angles[0] = 90;
  else
  if (ent->s.angles[0] < -90)
    ent->s.angles[0] = -90;
}

//=============================================
qboolean BankCheck(edict_t *ent, vec3_t pos) {
trace_t tr;
vec3_t end;
  VectorCopy(pos,end);
  end[2] -= 4096;
  tr = gi.trace(pos, ent->mins, ent->maxs, end, ent, MASK_BOTSOLIDX);
  return !(tr.startsolid || tr.allsolid || tr.plane.normal[2] < 0.8);
}

//=============================================
qboolean HazardCheck(edict_t *ent, vec3_t pos) {
trace_t tr;
vec3_t end;
int contents;
  VectorCopy(pos,end);
  end[2] -= 8192;
  contents = (ent->client->enviro_framenum>level.framenum)?CONTENTS_LAVA:(CONTENTS_LAVA|CONTENTS_SLIME);
  tr = gi.trace(pos, ent->mins, ent->maxs, end, ent, MASK_OPAQUE);
// ion idea
//  tr = gi.trace(pos, ent->mins, ent->maxs, end, ent, MASK_SOLID);
  return !(tr.contents & contents);
}

//==============================================
void SetBotAnim(edict_t *ent) {
  gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
  if (ent->client->anim_priority >= ANIM_JUMP) return;
  ent->s.frame = FRAME_jump1-1;
  ent->client->anim_end = FRAME_jump6;
}

//============================================================
qboolean Get_FlyingSpeed(float bottom,float block,float dist,float *speed) {
float tdist;

  if (bottom >= 40) {
    if (block > 4) return false;
    tdist = (dist*block)*.250; }
  else if (bottom >= 35) {
    if (block > 5) return false;
    tdist = (dist*block)*.200; }
  else if (bottom >= 30) {
    if (block > 6) return false;
    tdist = (dist*block)*.167; }
  else if (bottom >= 20) {
    if (block > 7) return false;
    tdist = (dist*block)*.143; }
  else if (bottom >= -5) {
    if (block > 8) return false;
    tdist = (dist*block)*.125; }
  else if (bottom >= -20) {
    if (block > 9) return false;
    tdist = (dist*block)*.143; }
  else if (bottom >= -35) {
    if (block > 10) return false;
    tdist = (dist*block)*.167; }
  else if (bottom >= -52) {
    if (block > 11) return false;
    tdist = (dist*block)*.200; }
  else if (bottom >= -75) {
    if (block > 12) return false;
    tdist = (dist*block)*.250; }
  else if (bottom >= -95) {
    if (block > 13) return false;
    tdist = (dist*block)*.333; }
  else if (bottom >=-125) {
    if (block > 14) return false;
    tdist = (dist*block)*.500; }
  else {
    if (block > 15) return false;
    tdist = (dist*block)*.500; }

  *speed = tdist/30;

  return true;
}

//==========================================
float SetBotXYSpeed(edict_t *ent, float *xyspeed) {

  if (!ent->isabot) return *xyspeed;

  if (ent->groundentity && ent->client->movestate & (0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800)) {
    *xyspeed = (VectorLength(ent->groundentity->velocity)<1)?300:0;
    if (*xyspeed)
      ent->client->movestate |= 0x00000010; } // Don't move
  else
    *xyspeed = (ent->client->camptime > level.time)?0:300;

  return *xyspeed;
}

//==========================================
void SetBotThink(edict_t *ent) {

  if (!ent->isabot) return;

  ent->client->chattime = level.time+60+(20*rand()%6);
  ent->client->ping = atoi(Info_ValueForKey(ent->client->pers.userinfo,"ping"));
  ent->think = BotThink;
  ent->nextthink = level.time+0.1;
}

//==========================================
void ForceRouteReset(edict_t *other) {

  if (!other->isabot) return;

  if (!other->client->routetrace) return;

  if (other->client->pers.routeindex < TotalRouteNodes) {
    if (Route[other->client->pers.routeindex].state == 2)
      other->client->pers.routeindex++;
    if (other->client->pers.routeindex < TotalRouteNodes)
      if (Route[other->client->pers.routeindex].state == 22)
        other->client->pers.routeindex++; }
}

//==========================================
void G_FindTrainTeam(void) {
edict_t *e;
int i;

  e = &g_edicts[(int)maxclients->value+1];
  for (i=maxclients->value+1; i<globals.num_edicts; i++, e++) {
    if (e->inuse && e->classname) {
      if (e->touch == path_corner_touch && e->targetname && e->target) {
        qboolean findteam = false;
        char *currtarget = e->target;         //target
        char *currtargetname = e->targetname; //targetname
        int k,lc=0,loopindex=0;
        edict_t *teamlist[1025];
        char *targethist[1024];
        memset(&teamlist,0,sizeof(teamlist));
        memset(&targethist,0,sizeof(targethist));
        targethist[0] = e->targetname;
        while (lc < MAX_EDICTS) {
          int j;
          edict_t *p,*t=&g_edicts[(int)maxclients->value+1];
          for (j=maxclients->value+1; j<globals.num_edicts; j++, t++)
            if (t->inuse && t->classname && !t->trainteam)
              if (t->use == train_use)
                if (!stricmp(t->target,currtargetname))
                  for (k=0;k<lc;k++)
                    if (teamlist[k] == t)
                      break;
          if (k == lc) {
            teamlist[lc] = t;
            lc++; }
          p = G_PickTarget(currtarget);
          if (!p) break; // exit while
          currtarget = p->target;
          currtargetname = p->targetname;
          if (!p->target) break; // exit while
          for (k=0;k<loopindex;k++)
            if (!stricmp(targethist[k],currtargetname))
              break;
          if (k < loopindex) {
            findteam = true;
            break; }
          targethist[loopindex] = currtargetname;
          loopindex++; }
        if (findteam && lc > 0)
          for (k=0;k<lc;k++) {
            if (!teamlist[k+1]) {
              teamlist[k]->trainteam = teamlist[0];
              break; }
            teamlist[k]->trainteam = teamlist[k+1]; } } } }
}

//==============================================
void droptofloor2(edict_t *ent) {
vec3_t trmin,trmax,min,mins,maxs;
float i,j=0,yaw;
trace_t tr;
vec3_t v,dest;

  VectorSet(ent->mins,-15,-15,-15);
  VectorSet(ent->maxs,8,8,15);

  if (ent->union_ent && ent->item == item_navi2) {
    dest[0] = (ent->union_ent->s.origin[0]+ent->union_ent->mins[0]+ent->union_ent->s.origin[0]+ent->union_ent->maxs[0])*0.5;
    dest[1] = (ent->union_ent->s.origin[1]+ent->union_ent->mins[1]+ent->union_ent->s.origin[1]+ent->union_ent->maxs[1])*0.5;
    for (i=ent->union_ent->s.origin[2]+ent->union_ent->mins[2]; i <= ent->union_ent->s.origin[2]+ent->union_ent->maxs[2] +16; i++) {
      dest[2] = i;
      tr = gi.trace(dest,ent->mins,ent->maxs,dest,ent,MASK_SOLID);
      if ((!tr.startsolid && !tr.allsolid) && j==1) {
        j = 2; break; }
      else
      if ((tr.startsolid || tr.allsolid) && (!j) && tr.ent == ent->union_ent)
        j = 1; }
    VectorCopy(dest,ent->s.origin);
    VectorSubtract(ent->s.origin,ent->union_ent->s.origin,ent->moveinfo.dir); }

  ent->s.modelindex = 0;
  ent->solid = (ent->item == item_navi3)?SOLID_NOT:SOLID_TRIGGER;
  ent->movetype = MOVETYPE_TOSS;
  ent->touch = Touch_Item;
  ent->use = NULL;

  VectorSet(v,0,0,-128);
  VectorAdd(ent->s.origin, v, dest);

  tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
  if (tr.startsolid && ent->item == item_navi1) {
    G_FreeEdict(ent);
    return; }

  VectorCopy(tr.endpos, ent->s.origin);

  if (ent->team) {
    ent->flags &= ~FL_TEAMSLAVE;
    ent->chain = ent->teamchain;
    ent->teamchain = NULL;
    ent->svflags |= SVF_NOCLIENT;
    ent->solid = SOLID_NOT;
    if (ent == ent->teammaster) {
      ent->nextthink = level.time+FRAMETIME;
      ent->think = DoRespawn; } }

  if (ent->spawnflags & 2) {
    ent->solid = SOLID_BBOX;
    ent->touch = NULL;
    ent->s.effects &= ~EF_ROTATE;
    ent->s.renderfx &= ~RF_GLOW; }

  if (ent->spawnflags & 1) {
    ent->svflags |= SVF_NOCLIENT;
    ent->solid = SOLID_NOT;
    ent->use = Use_Item; }

  gi.linkentity(ent);

  VectorCopy(ent->s.origin,min);
  VectorSet(mins, -15, -15, -15);
  VectorSet(maxs, 8, 8, 0);
  min[2] -= 128;

  for (i=0;i<8;i++) {
    if (i<4) {
      yaw = DEG2RAD(90*i-180);
      for (j=32;j<80;j+=2) {
        trmin[0] = ent->s.origin[0]+cos(yaw)*j;
        trmin[1] = ent->s.origin[1]+sin(yaw)*j;
        trmin[2] = ent->s.origin[2];
        VectorCopy(trmin,trmax);
        trmax[2] -= 128;
        tr = gi.trace(trmin, mins, maxs, trmax, ent, MASK_PLAYERSOLID);
        if (tr.endpos[2] < ent->s.origin[2]-16 && tr.endpos[2] > min[2])
          if (!tr.allsolid && !tr.startsolid) {
            min[2] = tr.endpos[2];
            min[0] = ent->s.origin[0]+cos(yaw)*(j+16);
            min[1] = ent->s.origin[1]+sin(yaw)*(j+16);
            break; } } }
    else {
      yaw = DEG2RAD(90*(i-4)-135);
      for (j=32;j<80;j+=2) {
        trmin[0] = ent->s.origin[0]+cos(yaw)*46;
        trmin[1] = ent->s.origin[1]+sin(yaw)*46;
        trmin[2] = ent->s.origin[2];
        VectorCopy(trmin,trmax);
        trmax[2] -= 128;
        tr = gi.trace(trmin, NULL, NULL, trmax, ent, MASK_PLAYERSOLID);
        if (tr.endpos[2] < ent->s.origin[2]-16 && tr.endpos[2] > min[2])
          if (!tr.allsolid && !tr.startsolid) {
            VectorCopy(tr.endpos,min);
            break; } } } }

  VectorCopy(min,ent->moveinfo.start_origin);
}

//==============================================
void TraceAllSolid(edict_t *ent, vec3_t point, trace_t tr) {

  if (tr.allsolid) {
    vec3_t stp,v1,v2;
    trace_t tracep;
    VectorSet(v1,-16,-16,-24);
    VectorSet(v2,16,16,4);
    VectorCopy(ent->s.origin,stp);
    stp[2] += 24;
    tracep = gi.trace(stp, v1, v2, point, ent, MASK_BOTSOLID);
    if (tracep.ent && !tracep.allsolid)
      if (tracep.ent->classname[0] == 'f') {
        VectorCopy(tracep.endpos,ent->s.origin);
        ent->groundentity = tracep.ent;
        ent->groundentity_linkcount = tracep.ent->linkcount;
        gi.linkentity(ent);
        return; } }
  else {
    if (ent->client) {
      ent->client->ground_contents = tr.contents;
      ent->client->ground_slope = tr.plane.normal[2]; }
    VectorCopy(tr.endpos, ent->s.origin);
    ent->groundentity = tr.ent;
    ent->groundentity_linkcount = tr.ent->linkcount; }

  gi.linkentity(ent);
}

//==============================================
void ResetGroundSlope(edict_t *ent) {
  if (!ent->isabot) return;
  ent->client->ground_slope = 1.0;
}

//==============================================
void SpawnItem3(edict_t *it_ent, gitem_t *item) {
  it_ent->item = item;
  it_ent->s.effects = 0;
  it_ent->s.renderfx = 0;
  it_ent->s.modelindex = 0;
  it_ent->nextthink = level.time+0.2;
  it_ent->think = droptofloor2;
}

//===============================================
void bFuncTrain(edict_t *self) {
gitem_t *it = item_navi1;
edict_t *it_ent = G_Spawn();

  VectorAdd(self->s.origin,self->mins,self->monsterinfo.last_sighting);
  it_ent->classname = it->classname;
  it_ent->union_ent = self;
  self->union_ent = it_ent;
  SpawnItem3(it_ent,it);
}

// ion func_explosive implementation
/*===============================================
void bFuncExplosive(edict_t *self) {
int i;
  for (i=1; i<=maxclients->value; i++)
  {
    edict_t *ent = &g_edicts[i];
    if (!ent->isabot) continue;
//    if (ent->client->waiting_obj != self) continue;
//    if (!ent->client->movestate) continue;

    ent->client->moveyaw = ent->s.angles[YAW]+180; // Turn around!
    if (ent->client->moveyaw > 180)
      ent->client->moveyaw -= 360;

	ent->client->battlemode |= 0x00000008;
    ent->client->movestate |= 0x00000080;
  }
}

*/// ion end
//===============================================
void bFuncDoor(edict_t *ent) {

  VectorAdd(ent->s.origin,ent->mins,ent->monsterinfo.last_sighting);
  if (fabs(ent->moveinfo.start_origin[2]-ent->moveinfo.end_origin[2]) > 20) {
    gitem_t *it=item_navi3;
    edict_t *it_ent=G_Spawn();
    it_ent->classname = it->classname;
    it_ent->union_ent = ent;
    ent->union_ent = it_ent;
    SpawnItem3(it_ent,it); }
}

//===============================================
void bDoorBlocked(edict_t *self) {
int i;
  for (i=1; i<=maxclients->value; i++) {
    edict_t *ent = &g_edicts[i];
    if (!ent->isabot) continue;
    if (ent->client->waiting_obj != self) continue;
    if (!ent->client->movestate) continue;
    ent->client->movestate |= 0x00000010; }
}

//===============================================
void bFuncButton(edict_t *ent) {
vec3_t tdir,tdir2;
vec3_t abs_movedir;
float dist=1;
gitem_t *it=item_navi2;
edict_t *it_ent=G_Spawn();

  VectorAdd(ent->s.origin,ent->mins,ent->monsterinfo.last_sighting);
  it_ent->classname = it->classname;
  VectorCopy(ent->s.origin,it_ent->s.origin);
  it_ent->s.origin[0] = (ent->absmin[0]+ent->absmax[0])*0.5;
  it_ent->s.origin[1] = (ent->absmin[1]+ent->absmax[1])*0.5;
  it_ent->s.origin[2] = (ent->absmin[2]+ent->absmax[2])*0.5;
  it_ent->union_ent = ent;
  ent->union_ent = it_ent;
  VectorSubtract(ent->moveinfo.start_origin, ent->moveinfo.end_origin, abs_movedir);
  VectorNormalize(abs_movedir);
  while (dist < 500) {
    VectorScale(abs_movedir, dist, tdir);
    VectorAdd(it_ent->s.origin,tdir,tdir2);
    if (!(gi.pointcontents(tdir2) & CONTENTS_SOLID)) break;
    dist++; }
  VectorScale(abs_movedir,(dist+20), tdir);
  VectorAdd(it_ent->s.origin,tdir,tdir2);
  VectorCopy(tdir2,it_ent->s.origin);
  it_ent->item = it;
  it_ent->s.effects = 0;
  it_ent->s.renderfx = 0;
  it_ent->s.modelindex  = 0;
  it_ent->solid = SOLID_TRIGGER;
  it_ent->movetype = MOVETYPE_NONE;
  it_ent->touch = Touch_Item;
  gi.linkentity(it_ent);
}

//===============================================
void bFuncPlat(edict_t *ent) {
gitem_t *it=item_navi1;
edict_t *it_ent=G_Spawn();

  VectorAdd(ent->s.origin,ent->mins,ent->monsterinfo.last_sighting);
  it_ent->classname = it->classname;
  it_ent->union_ent = ent;
  ent->union_ent = it_ent;
  SpawnItem3(it_ent,it);
}

//===============================================
void CheckBotCrushed(edict_t *targ,edict_t *inflictor,int mod) {

  if (!targ->isabot || mod != MOD_CRUSH) return;

  if ((targ->client->waiting_obj == inflictor && targ->client->movestate) || targ->groundentity == inflictor)
    targ->client->movestate |= 0x00000010;
}

//===============================================
void CheckPrimaryWeapon(edict_t *ent, edict_t *other) {

  if (!other->isabot) return;

  if (ent->item->use) {
    // Switch weapon if picked up primary
    int k = GetKindWeapon(ent->item);
    if (k != WEAP_BLASTER) {
      if (Bot[other->client->pers.botindex].skill[6] == k)
        ent->item->use(other,ent->item); } }
}

//===============================================
void BotCheckEnemy(gclient_t *client, edict_t *attacker, edict_t *targ, int mod) {

  if (client && targ->isabot && attacker) {
    if (client->battlemode & 0x00000008)
      client->battlemode &= ~0x00000008;
//    if (mod & MOD_RAILGUN|MOD_ROCKET|MOD_BLASTER|MOD_HYPERBLASTER) {
    if (mod & MOD_ROCKET|MOD_BLASTER|MOD_HYPERBLASTER) {
      if (attacker->client && !client->current_enemy)
        if (!OnSameTeam(targ, attacker))
          client->current_enemy = attacker; } }
}

//===============================================
void CheckCampSite(edict_t *ent, edict_t *other) {

  if (!other->isabot) return;

//  if (ent->item != item_health_mega && ent->item != item_railgun) return;
// IoN
/*  if (ent->item != item_health_mega) return;

  if (ent->item == item_health_mega && other->client->pers.weapon == item_blaster)
    return;
*/
  if (other->client->pers.weapon == item_slapper)
	return;

  if (other->client->quad_framenum > level.framenum) return;

  if (other->client->camptime >= level.time) return;

//  if (random() > 0.60) return; // camp 60% of time
// IoN LESS CAMPING DAMNIT
  if (random() > 0.20) return; // camp 60% of time

  other->client->camptime = level.time+20+rand()%10; // 20..30
  other->client->chattime = level.time+10+rand()%15; // 10..25
  other->client->taunttime = other->client->camptime+10; // turn OFF!
  VectorCopy(ent->s.origin, other->client->lastorigin);
  other->client->lastorigin[2] += 16;
  other->client->campitem = ent->item; //camping near this item
}

//======================================================
//======== ROUTE FILE AND BOT CONFIGURATION ============
//======================================================

//===============================================
void RandomizeParameters(int i) {
  Bot[i].skill[AIMACCURACY] = 5+rand()%4;     // [5..9]
//  Bot[i].skill[AIMACCURACY] = 1+rand()%4;     // [5..9]
  Bot[i].skill[AGGRESSION]  = 6+rand()%3;     // [6..9]
  Bot[i].skill[COMBATSKILL] = 3+rand()%6;     // [3..9]
  Bot[i].skill[PRIMARYWEAP] = 3+rand()%8;     // [3..11]
  Bot[i].skill[VRANGEVIEW]  = 60+10*rand()%6; // [60..120]
  Bot[i].skill[HRANGEVIEW]  = 60+10*rand()%6; // [60..120]
}

//==============================================
void LoadBotNames(void) {
int i;
  for (i=0;i<MAXBOTS;i++) {
    memset(&Bot[i],0,sizeof(botinfo_t));
    sprintf(Bot[i].netname,"%s",gbot[i][0]);
    sprintf(Bot[i].skin,"%s",gbot[i][1]);
    RandomizeParameters(i); }
}

//==============================================
//==============================================

int CurrentIndex=0;

//==================================================
qboolean RTJump_Chk(vec3_t apos,vec3_t tpos) {
float x,l,vel,yori;
vec3_t v,vv;
int mf = 0;

  vel = 300;
  yori = apos[2];

  VectorSubtract(tpos,apos,v);

  for(x = 1;x <= 60;++x ) {
    vel -= sv_gravity->value * 0.1;
    yori += vel * 0.1;
    if(vel > 0) {
      if(mf == 0) {
        if(tpos[2] < yori) mf = 2; } }
    else if(x > 1) {
      if(mf == 0) {
        if(tpos[2] < yori) mf = 2; }
      else if(mf == 2) {
        if(tpos[2] >= yori) {
          mf = 1;
          break; } } } }

  VectorCopy(v,vv);
  vv[2] = 0;
  l = VectorLength(vv);
  if(x > 1) l /= (x - 1);

  return (l < 32 && mf==1);
}

//==============================================
void G_FindRouteLink(edict_t *ent) {
trace_t  rs_trace;
qboolean tpbool;
int i,j,k,l,total = 0;
vec3_t v;
float x;

  if (JumpMax == 0) {
    x = 300-ent->gravity*sv_gravity->value*0.1;
    JumpMax = 0;
    while (1) {
      JumpMax += x*0.1;
      x -= ent->gravity*sv_gravity->value*0.1;
      if (x < 0) break; } }

  for(i = 0;i < CurrentIndex;i++) {
    if(Route[i].state == 0) {
      for(j=0;j<CurrentIndex;j++) {
        if(abs(i-j) <= 50 || j==i || Route[j].state != 0) continue;
        VectorSubtract(Route[j].Pt,Route[i].Pt,v);
        if(v[2] > JumpMax || v[2] < -500) continue;
        v[2] = 0;
        if(VectorLength(v) > 200) continue;
        if(fabs(v[2]) > 20 || VectorLength(v) > 64)
          if(!RTJump_Chk(Route[i].Pt,Route[j].Pt))
            continue;
        tpbool = false;
        for(l = -5;l < 6;l++) {
          if((i+l)<0 || (i+l)>=CurrentIndex) continue;
          for(k = 0;k < 5;k++) {
            if(!Route[i + l].linkpod[k]) break;
            if(abs(Route[i + l].linkpod[k] - j) < 50) {
              tpbool = true;
              break; } }
          if(tpbool) break; }
      if(tpbool) continue;
      rs_trace = gi.trace(Route[j].Pt,NULL,NULL,Route[i].Pt,ent,MASK_SOLID);
      if(!rs_trace.startsolid && !rs_trace.allsolid && rs_trace.fraction == 1.0) {
        for(k = 0;k < 5;k++) {
          if(!Route[i].linkpod[k]) {
            Route[i].linkpod[k] = j;
            total++;
            break; } } } } } }

  G_FreeEdict (ent);
}

//==================================================
//void ReadRouteFile(void) {
void ReadRouteFile(void) {

int i,j;
vec3_t v;
edict_t *e;
FILE *fp;
char name[256];

  cvar_t* gamedir;
  cvar_t* basedir;

	gamedir = gi.cvar("game", "", 0);
	basedir = gi.cvar("basedir", ".", 0);

  TotalRouteNodes = 0;

//level.mapname = s
//  sprintf(name,"c:/quake2/3zb2/chdtm/%s.chn",level.mapname);
//  sprintf(name,"chdtm/%s.chn",level.mapname);
//    sprintf(name,"%s%s%s.chn","chdtm\\",level.mapname);
//    gi.dprintf (name);
    strcpy(name, basedir->string);
    strcat(name, "\\");
    strcat(name, "base007");
    strcat(name, "\\chdtm\\");
    strncat(name, level.mapname, 24);
    strcat(name, ".chn");
//    gi.dprintf (name);

   fp = fopen(name,"rb");

  if (fp=fopen(name,"rb")) {
    char code[8];
    unsigned int size;
    CurrentIndex=0;
    memset(Route,0,sizeof(route_t));
    memset(code,0,8);
    fread(code,sizeof(char),8,fp);
    fread(&CurrentIndex,sizeof(int),1,fp);
    size = (unsigned int)CurrentIndex*sizeof(route_t);
    fread(Route,size,1,fp);
    fclose(fp);
    TotalRouteNodes = CurrentIndex;
    gi.dprintf("%d Total Route Nodes\n",TotalRouteNodes); }

  if (TotalRouteNodes==0) {
    gi.dprintf("NO ROUTE FILE LOADED\n");
    return; }

  for(i = 0;i < TotalRouteNodes;i++) {
    if((Route[i].state > 2 && Route[i].state <= 7)
   || Route[i].state == -10 || Route[i].state == -11) {
    edict_t *other = &g_edicts[(int)maxclients->value+1];
    for (j=maxclients->value+1 ; j<globals.num_edicts ; j++, other++) {
      if (other && other->inuse) {
        if(Route[i].state == 4
         || Route[i].state == 5
         || Route[i].state == 7
         || Route[i].state == 6) {
         VectorAdd(other->s.origin,other->mins,v);
         if(VectorCompare (Route[i].Pt,v)) {
           if(Route[i].state == 4 && !Q_stricmp(other->classname, "func_plat")) {
             Route[i].ent = other;
             break; }
           else if(Route[i].state == 5 && !Q_stricmp(other->classname, "func_train")) {
             Route[i].ent = other;
             break; }
// IoN func_explosive idea start
/*
           else if(Route[i].state == 6 && !Q_stricmp(other->classname, "func_explosive")) {
             Route[i].ent = other;
             break; }
*/
// IoN func_explosive idea end
           
		   else if(Route[i].state == 7 && !Q_stricmp(other->classname, "func_button")) {
             Route[i].ent = other;
             break; }
           else if(Route[i].state == 6 && !Q_stricmp(other->classname, "func_door")) {
             Route[i].ent = other;
             break; } } }
      else
        if(Route[i].state == 3 || Route[i].state == -10 || Route[i].state == -11) {
          if(VectorCompare(Route[i].Pt,other->monsterinfo.last_sighting)) {
            Route[i].ent = other;
            break; } } } }
  if(j >= globals.num_edicts)
    Route[i].state = 0; } }

  e = G_Spawn();
  e->nextthink = level.time + FRAMETIME * 2;
  e->think = G_FindRouteLink;
}

//======================================================
//============= SPAWNING BOTS INTO THE GAME ============
//======================================================

//======================================================
char *Random_IP(void) {
static char ipstr[16];
int ip1;

  do {
    ip1 = 128+(rand()%128);
  } while (ip1 == 192 || ip1 == 172);

  sprintf(ipstr, "%d.%d.%d.%d", ip1, (int)(rand()%256), (int)(rand()%256), (int)(rand()%256));

  return ipstr;
}

//=============================================
int GetFreeEdict(void) {
int i;

  for (i=(int)(game.maxclients-1); i>=0; i--) {
    edict_t *ent = g_edicts+i+1;
    if (!ent->inuse) {
      G_InitEdict(ent);
      return i; } }

  return -1; // refuse connection
}

//======================================================
void G_MuzzleFlash(short rec_no, vec3_t origin, int flashnum) {
  gi.WriteByte(svc_muzzleflash);
  gi.WriteShort(rec_no);
  gi.WriteByte(flashnum);
  gi.multicast(origin, MULTICAST_PVS);
}

//=============================================
qboolean SpawnBot(int i) {
edict_t *ent;
int clientnum;
char userinfo[512];

  if (i<0 || i>(MAXBOTS-1)) return false;

  clientnum = GetFreeEdict();
  if (clientnum < 0) {
    gi.dprintf("Server is full. Increase maxclients!!\n");
    return false; }

  ent = g_edicts+clientnum+1;
  ent->client = &game.clients[clientnum];

  ent->isabot = true;

  InitClientResp(ent->client);
  InitClientPersistant(ent->client);

  ent->client->pers.botindex = i;
  ent->client->pers.routeindex = 0;

  sprintf(userinfo,"\\name\\%s\\skin\\%s\\fov\\90\\hand\\2\\ip\\%s\\ping\\%3d", Bot[i].netname, Bot[i].skin, Random_IP(), (int)(100+(rand()%128)));
  ClientUserinfoChanged(ent,userinfo);

  PutClientInServer(ent);

  G_MuzzleFlash((short)(ent-g_edicts), ent->s.origin, (int)(MZ_LOGIN));

  gi_bprintf(2,"%s entered the game\n",Bot[i].netname);

	if (teamdm->value && ent->client->resp.teamdm_team < TEAMDM_TEAM1)
		TEAMDMAssignBotTeam(ent->client);

  ClientEndServerFrame(ent);

  return true;
}

//==================================================
void RemoveAllBots(void) {
int i;

//  for (i=1;i<=game.maxclients;i++)
  for (i=0;i < MAXBOTS;i++)
  {
    edict_t *ent = &g_edicts[i];
    if (!ent || !ent->isabot)
		continue;
    Bot[ent->client->pers.botindex].ingame = 0;
    ent->isabot=false;
    ClientDisconnect(ent);
	G_FreeEdict (ent);
  }

  NumBotsInGame = 0; // Must Reset
}

//==================================================
void RespawnAllBots(void) {
int i;
  for (i=0;i<MAXBOTS;i++)
    if (Bot[i].ingame)
      SpawnBot(i);
}

//==================================================
void SpawnNumBots(int numbots) {
int j,k;
qboolean foundASpot;

  for (k=1;k<=numbots;k++)
  {
	  foundASpot = false;
/*    do
	{
      j=(int)(rand()%(MAXBOTS-1));
    } while (Bot[j].ingame);
*/
	for (j = 0; j < MAXBOTS; j++)
	{
		if (Bot[j].ingame == 0)
		{
			foundASpot = true;
			break;
		}
		else
		{
			foundASpot = false;
		}
	}
//	if (!foundASpot)
    
	if (!SpawnBot(j)) return; // No free edicts!!
    
	Bot[j].ingame = 1;
    NumBotsInGame++;
  }

  if (NumBotsInGame >= MAXBOTS)
    gi_cprintf(NULL,2,"Max Bots Spawned.\n");
}

//=======================================================
//============ TAUNTING/CHATTING/INSULTING ==============
//=======================================================

//==============================================
void InsultVictim(edict_t *ent, edict_t *victim) {

  if (!ent->isabot) return;

  if (!victim || !victim->client) return;

  if (ent->client->insulttime > level.time) return;

  // if bot just killed self then...
  if (victim == ent) {
    if (myrandom < 0.40)
      switch (rand()%7) {
        case 0: gi_bprintf(3,"%s: OUCH!!!\n", ent->client->pers.netname); break;
        case 1: gi_bprintf(3,"%s: I hate that!\n", ent->client->pers.netname); break;
        case 2: gi_bprintf(3,"%s: shit!\n", ent->client->pers.netname); break;
        case 3: gi_bprintf(3,"%s: not again!\n", ent->client->pers.netname); break;
        case 4: gi_bprintf(3,"%s: Ugghhhh!\n", ent->client->pers.netname); break;
        case 5: gi_bprintf(3,"%s: wtf!\n", ent->client->pers.netname); break;
        case 6: gi_bprintf(3,"%s: Ohhhhh!!!!\n", ent->client->pers.netname); } }
  else
  // Otherwise, chat
  if (myrandom < 0.80)
    switch (rand()%19) {
      case 0: gi_bprintf(3,"%s: You REALLY suck!\n", ent->client->pers.netname); break;
      case 1: gi_bprintf(3,"%s: YOU SUCK!\n", ent->client->pers.netname); break;
      case 2: gi_bprintf(3,"%s: Suck THIS!\n", ent->client->pers.netname); break;
      case 3: gi_bprintf(3,"%s: This sucks!\n", ent->client->pers.netname); break;
      case 4: gi_bprintf(3,"%s: Eat Me!\n", ent->client->pers.netname); break;
      case 5: gi_bprintf(3,"%s: You ALL suck!\n", ent->client->pers.netname); break;
      case 6: gi_bprintf(3,"%s: Suck THAT\n", ent->client->pers.netname); break;
      case 7: gi_bprintf(3,"%s: Muhhhhaahhhaaa\n", ent->client->pers.netname); break;
      case 8: gi_bprintf(3,"%s: Muhaaaaaaaaa!!\n", ent->client->pers.netname); break;
      case 9: gi_bprintf(3,"%s: Huuuhhhaaaaaa!\n", ent->client->pers.netname); break;
      case 10:gi_bprintf(3,"%s: Muhhhhhhaaaaa!!!\n", ent->client->pers.netname); break;
      case 11:gi_bprintf(3,"%s: Whoooooaaaaa!\n", ent->client->pers.netname); break;
      case 12:gi_bprintf(3,"%s: Your sister!!\n", ent->client->pers.netname); break;
      case 13:gi_bprintf(3,"%s: Your daughter!!!!\n", ent->client->pers.netname); break;
      case 14:gi_bprintf(3,"%s: Yo mama!\n", ent->client->pers.netname); break;
      case 15:gi_bprintf(3,"%s: Arggggghhhh!\n", ent->client->pers.netname); break;
      case 16:gi_bprintf(3,"%s: Your daddy!!!\n", ent->client->pers.netname); break;
      case 17:gi_bprintf(3,"%s: Bite Me!\n", ent->client->pers.netname); break;
      case 18:gi_bprintf(3,"%s: HeeeeHaaaaa\n", ent->client->pers.netname); }

  ent->client->insulttime = level.time+60+(10*(rand()%6));
}

//==============================================
void TauntVictim(edict_t *ent, edict_t *victim) {
vec3_t vtmp;

  if (!ent->isabot || ent == victim) return;

  if (!victim || !victim->client) return;

  if (ent->client->taunttime > level.time) return;

  // Taunt only if near victim (don't reset timer)
  VectorSubtract(ent->s.origin, victim->s.origin, vtmp);
  if (VectorLength(vtmp) > 250) return;

  switch (rand()%3) {
    case 0:ent->s.frame = FRAME_flip01-1;
           ent->client->anim_end = FRAME_flip12; break;
    case 1:ent->s.frame = FRAME_salute01-1;
           ent->client->anim_end = FRAME_salute11; break;
    case 2:ent->s.frame = FRAME_taunt01-1;
           ent->client->anim_end = FRAME_taunt17; }

  ent->client->taunttime = level.time+30+(10*(rand()%6));
}

//==============================================
void RandomChat(edict_t *ent) {

  if (ent->client->chattime > level.time) return;

  if (ent->client->camptime > level.time) {
    if (myrandom < 0.50) { // Camp and Chat about 50% of the time
/*      if (ent->client->campitem == item_railgun)
        switch (rand()%6) {
        case 0: gi_bprintf(3,"%s: Bring firewood nexttime!!\n", ent->client->pers.netname); break;
        case 1: gi_bprintf(3,"%s: Want a roasted weener?\n", ent->client->pers.netname); break;
        case 2: gi_bprintf(3,"%s: Want a beer with that??\n", ent->client->pers.netname); break;
        case 3: gi_bprintf(3,"%s: Don't ya just love it?\n", ent->client->pers.netname); break;
        case 4: gi_bprintf(3,"%s: Get the camper at railgun!!\n", ent->client->pers.netname); break;
        case 5: gi_bprintf(3,"%s: There's a camper at railgun!\n", ent->client->pers.netname); break; }
      else
*/      if (ent->client->campitem == item_health_mega)
        switch (rand()%6) {
        case 0: gi_bprintf(3,"%s: Kill the megahealth camper!!!\n", ent->client->pers.netname); break;
        case 1: gi_bprintf(3,"%s: Want marshmellows?\n", ent->client->pers.netname); break;
        case 2: gi_bprintf(3,"%s: Got hotdogs?\n", ent->client->pers.netname); break;
        case 3: gi_bprintf(3,"%s: fuckin campers\n", ent->client->pers.netname); break;
        case 4: gi_bprintf(3,"%s: Get the camper by megahealth!!\n", ent->client->pers.netname); break;
        case 5: gi_bprintf(3,"%s: Camper at megahealth!\n", ent->client->pers.netname); break; }
      ent->client->chattime = level.time+20;
      return; } }
  else
  if (myrandom < 0.10) // Do this random chatting very rarely
    switch (rand()%6) {
      case 0: gi_bprintf(3,"%s: Bunch of Chicken Shits!\n", ent->client->pers.netname); break;
      case 1: gi_bprintf(3,"%s: Come and get it!!!\n", ent->client->pers.netname); break;
      case 2: gi_bprintf(3,"%s: Who wants a piece of me?\n", ent->client->pers.netname); break;
      case 3: gi_bprintf(3,"%s: Where'd everybody go?\n", ent->client->pers.netname); break;
      case 4: gi_bprintf(3,"%s: This server sucks!\n", ent->client->pers.netname); break;
      case 5: gi_bprintf(3,"%s: Only pussies on this server!\n", ent->client->pers.netname); break; }

  ent->client->chattime = level.time+60+(20*(rand()%6));
}

//=====================================================
//=========== BASIC TRACING ALGORITHMS ================
//=====================================================

//==============================================
qboolean InSight(edict_t *ent, edict_t *other) {
vec3_t start,end;
trace_t tr;

  if (other->client && !G_ClientInGame(other))
    return false;

  VectorCopy(ent->s.origin,start);
  start[2] += ent->viewheight-8;

  VectorCopy(other->s.origin,end);
  end[2] += other->viewheight-8;

  if ((gi.pointcontents(start) & CONTENTS_WATER) && !other->waterlevel) {
    tr = gi.trace(end, NULL, NULL, start, ent, CONTENTS_WINDOW|MASK_OPAQUE|CONTENTS_WATER);
    if (tr.surface && tr.surface->flags & SURF_WARP) return false;
    tr = gi.trace(start, NULL, NULL, end, ent, CONTENTS_WINDOW|MASK_OPAQUE);
    return (tr.fraction == 1.0); }

  if ((gi.pointcontents(start) & CONTENTS_WATER) && other->waterlevel) {
    VectorCopy(other->s.origin,end);
    end[2] -= 16;
    tr = gi.trace(start, NULL, NULL, end, ent, CONTENTS_SOLID|CONTENTS_WINDOW);
    return (tr.fraction == 1.0); }

  if (other->waterlevel) {
    VectorCopy(other->s.origin,end);
    end[2] += 32;
    tr = gi.trace(start, NULL, NULL, end, ent, CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_WATER);
    if (tr.surface && tr.surface->flags & SURF_WARP) return false; }

  return (gi.trace(start, NULL, NULL, end, ent, CONTENTS_WINDOW|MASK_OPAQUE).fraction == 1.0);
}

//==============================================
qboolean Bot_trace2(edict_t *ent,vec3_t ttz) {
vec3_t ttx;
  VectorCopy(ent->s.origin,ttx);
  ttx[2] += (ent->maxs[2]>=32)?24:-12;
  return (gi.trace(ttx, NULL, NULL, ttz ,ent, MASK_OPAQUE).fraction == 1.0);
}

//==============================================
qboolean Bot_trace(edict_t *ent, edict_t *other) {
trace_t tr;
vec3_t ttx,tty;

  VectorCopy(ent->s.origin,ttx);
  VectorCopy(other->s.origin,tty);
  if (ent->maxs[2] >=32) {
    if (tty[2] > ttx[2])
      tty[2] += 16;
    ttx[2] += 30; }
  else
    ttx[2] -= 12;

  tr = gi.trace(ttx,NULL,NULL,tty,ent,CONTENTS_WINDOW|MASK_OPAQUE);
  if (tr.fraction == 1.0 && !tr.allsolid && !tr.startsolid) return true;

  if (ent->maxs[2] < 32) return false;

  if (tr.ent && tr.ent->use == door_use)
    if (!tr.ent->targetname)
      return true;

  if (ent->s.origin[2] < other->s.origin[2] || ent->s.origin[2]-24 > other->s.origin[2])
    return false;

  ttx[2] -= 36;
  tr = gi.trace(ttx, NULL, NULL, other->s.origin, ent, CONTENTS_WINDOW|MASK_OPAQUE);
  return (tr.fraction == 1.0 && !tr.allsolid && !tr.startsolid);
}

//==================================================
qboolean TraceX(edict_t *ent,vec3_t p2) {
trace_t tr;
vec3_t v1,v2;
int contents;

  contents = (CONTENTS_SOLID|CONTENTS_WINDOW);

  if (ent->svflags & ~SVF_MONSTER) {
    if (ent->client->waterstate) {
      VectorCopy(ent->mins,v1);
      VectorCopy(ent->maxs,v2); }
    else
    if (ent->client->ps.pmove.pm_flags & ~PMF_DUCKED) {
      VectorSet(v1,-16,-16,-4);
      VectorSet(v2,16,16,32); }
    else {
      VectorSet(v1,-4,-4,-4);
      VectorSet(v2,4,4,4); } }
  else {
    VectorClear(v1);
    VectorClear(v2);
    contents |= (CONTENTS_LAVA|CONTENTS_SLIME); }

  tr = gi.trace(ent->s.origin, v1, v2, p2, ent, contents);
  if (tr.fraction == 1.0 && !tr.allsolid && !tr.startsolid)
    return true;

  if (ent->client->routetrace)
    if (ent->svflags & SVF_MONSTER)
      if (tr.ent && tr.ent->use == door_use)
        return (tr.ent->moveinfo.state == 2);

  return false;
}

//============================================================
void Get_RouteOrigin(int index,vec3_t pos) {
edict_t *e;

  if (Route[index].state <= 3 || Route[index].state >= 20)
    if (Route[index].state == 3) {
      VectorCopy(Route[index].ent->s.origin,pos);
      pos[2] += 8; }
    else
      VectorCopy(Route[index].Pt,pos);

  switch (Route[index].state) {
    case 4:
      VectorCopy(Route[index].ent->union_ent->s.origin,pos);
      pos[2] += 8;
      return;
    case 5:
      if (!Route[index].ent->trainteam) {
        VectorCopy(Route[index].ent->union_ent->s.origin,pos);
        pos[2] += 8;
        return; }
      if (Route[index].ent->target_ent)
        if (VectorCompare(Route[index].Tcourner, Route[index].ent->target_ent->s.origin)) {
          VectorCopy(Route[index].ent->union_ent->s.origin,pos);
          pos[2] += 8;
          return; }
      e = Route[index].ent->trainteam;
      while (1) {
        if (e == Route[index].ent)
          break;
        if (e->target_ent)
          if (VectorCompare(Route[index].Tcourner, e->target_ent->s.origin)) {
            VectorCopy(e->union_ent->s.origin,pos);
            pos[2] += 8;
            Route[index].ent = e;
            return; }
      e = e->trainteam; }
      VectorCopy(Route[index].ent->union_ent->s.origin,pos);
      pos[2] += 8;
      return;
    case 6:
      if (Route[index].ent->union_ent) {
        VectorCopy(Route[index].ent->union_ent->s.origin,pos);
        pos[2] += 8; }
      else
      if (index+1 < TotalRouteNodes) {
        if (Route[index+1].state <= 3) {
          VectorCopy(Route[index+1].Pt,pos);
          pos[2] += 8; }
        else
        if (Route[index+1].state <= 5) {
          VectorCopy(Route[index+1].ent->union_ent->s.origin,pos);
          pos[2] += 8; }
        else
        if (Route[index+1].state == 7) {
          VectorCopy(Route[index+1].ent->union_ent->s.origin,pos);
          pos[2] += 8; }
        else
          VectorCopy(Route[index+1].Pt,pos); }
      else {
        pos[0] = (Route[index].ent->absmin[0]+Route[index].ent->absmax[0])*0.5;
        pos[1] = (Route[index].ent->absmin[1]+Route[index].ent->absmax[1])*0.5;
        pos[2] =  Route[index].ent->absmax[2]; }
      return;
    case 7:
      VectorCopy(Route[index].ent->union_ent->s.origin,pos); }
}

//==============================================
void GetAimAngle(edict_t *ent, float aim, float dist) {
vec3_t targaim;
trace_t tr;
int weapon;

  weapon = GetKindWeapon(ent->client->pers.weapon);

  switch (weapon) {
// ion slap
    case WEAP_SWORD:
    case WEAP_BLASTER:
    case WEAP_RCP90:
//	case WEAP_SHOTGUN:
//  case WEAP_SUPERSHOTGUN:
    case WEAP_LASER:
//be careful here
    case WEAP_MACHINEGUN:
/*    case WEAP_CHAINGUN:
    case WEAP_HYPERBLASTER:
*/
    case WEAP_ZMG:
    case WEAP_ASSRIFLE:
/*    case WEAP_RAILGUN:
      if (ent->client->current_enemy != ent->client->prev_enemy) {
        if (ent->client->current_enemy->isabot)
          VectorSubtract(ent->client->current_enemy->s.old_origin, ent->client->current_enemy->s.origin, targaim);
        else {
          VectorCopy(ent->client->current_enemy->velocity, targaim);
          VectorInverse(targaim); }
        VectorNormalize(targaim);
        VectorMA(ent->client->current_enemy->s.origin, 5*aim*myrandom, targaim, targaim); }
      else {
        VectorSubtract(ent->client->targ_old_origin, ent->client->current_enemy->s.origin, targaim);
        VectorMA(ent->client->current_enemy->s.origin, aim*myrandom, targaim, targaim); }
      VectorSubtract(targaim, ent->s.origin, targaim);
      AdjustAngle(ent, targaim, aim);
      return;
*/
    case WEAP_GRENADES:
// ion mine support
    case WEAP_PROXMINE:
    case WEAP_GRENADELAUNCHER:
    case WEAP_ROCKETLAUNCHER:
      if (ent->client->current_enemy != ent->client->prev_enemy) {
        if (ent->client->current_enemy->isabot)
          VectorSubtract(ent->client->current_enemy->s.origin, ent->client->current_enemy->s.old_origin, targaim);
        else {
          VectorCopy(ent->client->current_enemy->velocity, targaim);
          VectorScale(targaim, 32, targaim); }
        VectorNormalize(targaim);
        VectorMA(ent->client->current_enemy->s.origin, (11-aim)*dist/25, targaim, targaim); }
      else {
        VectorSubtract(ent->client->current_enemy->s.origin, ent->client->targ_old_origin, targaim);
        targaim[2] *= 0.5;
        VectorMA(ent->client->current_enemy->s.origin, -aim*myrandom+dist/75, targaim, targaim); }
      tr = gi.trace(ent->client->current_enemy->s.origin, NULL, NULL, targaim, ent->client->current_enemy, MASK_SHOT);
      VectorCopy(tr.endpos, targaim);
      if (weapon & WEAP_GRENADELAUNCHER|WEAP_ROCKETLAUNCHER) {
        if (targaim[2] < (ent->s.origin[2]+JumpMax)) {
          vec3_t vtmp;
          targaim[2] -= 24;
          VectorCopy(ent->s.origin, vtmp);
          vtmp[2] += ent->viewheight-8;
          tr = gi.trace(vtmp, NULL, NULL, targaim, ent, MASK_SHOT);
          if (tr.fraction != 1.0)
            targaim[2] += 24; }
        else
          if (targaim[2] >(ent->s.origin[2]+JumpMax))
            targaim[2] += 5; }
      VectorSubtract(targaim, ent->s.origin, targaim);
      AdjustAngle(ent, targaim, aim);
      return;

    default: // BFG
      VectorCopy(ent->client->vtemp, targaim);
      VectorSubtract(targaim, ent->s.origin, targaim);
      VectorSet(ent->s.angles,(Get_pitch(targaim)),(Get_yaw(targaim)),0.0F); }
}

//=================================================
qboolean HasAmmoForWeapon(edict_t *self, gitem_t *weapon) {
//  if (weapon==item_blaster) return true;
//  if (weapon==item_supershotgun) return true;
  if (weapon==item_slapper) return true;
  if (weapon==item_supershotgun) return true;
  if ((int)dmflags->value & DF_INFINITE_AMMO) return true;
  return (self->client->pers.inventory[ITEM_INDEX(FindItem(weapon->ammo))] >= weapon->quantity);
}

//========================================================
gitem_t *GetWeaponType(int weapnum) {

  switch (weapnum) {
//    case WEAP_SHOTGUN:         return item_shotgun;
    case WEAP_RCP90:           return item_shotgun;
//    case WEAP_SUPERSHOTGUN:    return item_supershotgun;
    case WEAP_LASER       :    return item_supershotgun;
// be careful
    case WEAP_MACHINEGUN:      return item_machinegun;
//    case WEAP_CHAINGUN:        return item_chaingun;
    case WEAP_ZMG:             return item_chaingun;
    case WEAP_GRENADES:        return item_grenades;
// ion mine support
    case WEAP_PROXMINE:        return item_proxmine;
    case WEAP_GRENADELAUNCHER: return item_grenadelauncher;
    case WEAP_ROCKETLAUNCHER:  return item_rocketlauncher;
//    case WEAP_HYPERBLASTER:    return item_hyperblaster;
    case WEAP_ASSRIFLE       :    return item_hyperblaster;
//    case WEAP_RAILGUN:         return item_railgun;
//    case WEAP_BFG:             return item_bfg; //}
// ion slap
    case WEAP_SWORD:         return item_slapper;
  
	case WEAP_BFG:				return item_rocketlauncher;
  }

	if (g_gametype->value != 5)
	{
		return item_blaster;
	}
	else
	{
		return item_slapper;
	}
}

//==============================================
qboolean CanUseWeapon(edict_t *ent, int weapnum) {
gitem_t *weapon;

  weapon = GetWeaponType(weapnum);

  if (ent->client->pers.inventory[ITEM_INDEX(weapon)])
    if (HasAmmoForWeapon(ent,weapon))
      if (ent->client->weaponstate & WEAPON_READY|WEAPON_FIRING) {
        if (ent->client->pers.weapon != weapon) {
          ent->client->newweapon = weapon;
          ChangeWeapon(ent); }
        return true; } // true whether switched or not

  return false;
}

//==============================================
qboolean Pickup_Navi(edict_t *ent, edict_t *other) {
int i;

  if (!(ent->spawnflags & DROPPED_ITEM))
    if (ent->item->quantity)
      SetRespawn(ent, ent->item->quantity);

  //on door (up & down)
  if (ent->item == item_navi3 && ent->union_ent) {
    int j,k;
    qboolean flg=false;
    if (ent->target_ent == other) {
      other->client->movestate &= ~(0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800);
      other->client->waiting_obj = ent->union_ent;
      if (ent->union_ent->spawnflags & 32) {
        if (ent->union_ent->moveinfo.state & 3|1)
          other->client->movestate |= 0x00000100;
        else
          other->client->movestate |= 0x00000080; }
      else {
        if (ent->union_ent->moveinfo.state & 3|0)
          other->client->movestate |= 0x00000100;
        else
        if (ent->union_ent->moveinfo.state & 2|1)
          other->client->movestate |= 0x00000080; }
      for (i=-10;i<10;i++) {
        if (i <= 0)
          j = other->client->pers.routeindex-(10-i);
        else
          j = other->client->pers.routeindex+i;
        if (j < 0) continue;
        if (j >= TotalRouteNodes) continue;
        if ((Route[j].state == 6 && Route[j].ent == ent->union_ent) || Route[j].state == 7) {
          vec3_t v;
          k = 1;
          flg = false;
          while (1) {
            if ((j+k) >= TotalRouteNodes) break;
            if ((j+k) >= other->client->pers.routeindex) {
              Get_RouteOrigin(j+k,v);
              if (fabs(v[2]-other->s.origin[2]) > JumpMax) {
                flg = true;
                break; } }
            k++; }
          if ((j+k) < TotalRouteNodes && flg) {
            other->client->pers.routeindex = j+k;
            break; } } }
      if (!flg)
        other->client->movestate |= 0x00000010;
      ent->target_ent = NULL; }
    SetRespawn(ent, 1000000);
    ent->solid = SOLID_NOT; }
  else
  if (ent->item == item_navi2)
    for (i=0;i<10;i++) {
      if ((other->client->pers.routeindex+i) >= TotalRouteNodes) break;
      if (!Route[other->client->pers.routeindex+i].index) break;
      if (Route[other->client->pers.routeindex+i].state != 7) continue;
      if (Route[other->client->pers.routeindex+i].ent == ent->union_ent) {
        other->client->pers.routeindex += i+1;
        break; } }

  return true;
}

//==============================================
qboolean B_UseWeapon(edict_t *ent, float aim, float distance, int weap) {

  if (!CanUseWeapon(ent,weap)) return false;

//  if (weap != WEAP_GRENADES && weap != WEAP_GRENADELAUNCHER)
    if (weap != WEAP_GRENADES && weap != WEAP_GRENADELAUNCHER && weap !=WEAP_PROXMINE)
    if (!InSight(ent,ent->client->current_enemy))
      return false;

  GetAimAngle(ent,aim,distance);
  ent->client->buttons |= BUTTON_ATTACK;
  trace_priority = MaxOf(trace_priority,2);

  return true;
}

//======================================================
//=========== BOT FIGHTING/COMBAT FUNCTIONS ============
//======================================================

//==============================================
void Combat_LevelX(edict_t *ent,float distance) {
vec3_t vdir;

  if (ent->client->battlemode & 0x00000040) {

    int k=0;
    float aim = 10.0 - Bot[ent->client->pers.botindex].skill[AIMACCURACY];

    //Rocket Launcher
    if (distance > 200 && distance < 1000)
      if (GetKindWeapon(ent->client->pers.weapon) == WEAP_ROCKETLAUNCHER)
        if (B_UseWeapon(ent,aim,distance,WEAP_ROCKETLAUNCHER))
          k = 1;

    //Grenade Launcher
    if (!k && distance > 100 && distance < 400)
      if (GetKindWeapon(ent->client->pers.weapon) == WEAP_GRENADELAUNCHER)
        if ((ent->client->current_enemy->s.origin[2]-ent->s.origin[2]) < 150)
          if (B_UseWeapon(ent,aim,distance,WEAP_GRENADELAUNCHER))
            k = 1;

    //Hand Grenade
    if (!k && distance > 200 && distance < 800)
      if (GetKindWeapon(ent->client->pers.weapon) == WEAP_GRENADES)
        if (B_UseWeapon(ent,aim,distance,WEAP_GRENADES))
          k = 1;

// ion mine support

    //Proximity Mine
    if (!k && distance > 200 && distance < 800)
      if (GetKindWeapon(ent->client->pers.weapon) == WEAP_PROXMINE)
        if (B_UseWeapon(ent,aim,distance,WEAP_PROXMINE))
          k = 1;

    VectorSubtract(ent->client->vtemp,ent->s.origin,vdir);
    ent->s.angles[YAW] = Get_yaw(vdir);
    ent->s.angles[PITCH] = Get_pitch(vdir);
    trace_priority = (k)?4:2;
    return; }

  VectorSubtract(ent->client->current_enemy->s.origin,ent->s.origin,vdir);
  ent->s.angles[YAW] = Get_yaw(vdir);
  ent->s.angles[PITCH] = Get_pitch(vdir);
  trace_priority = 2; // Use this angle
}

qboolean Bot_Fall(edict_t *ent,vec3_t pos,float dist);

//==============================================
void Combat_Normal(edict_t *ent,float distance) {
edict_t *target;
int i,j,k;
vec3_t v,vv,v1,v2;
trace_t tr;
int enewep;
float f,aim;

  target = ent->client->current_enemy;
  aim = 10.0 - Bot[ent->client->pers.botindex].skill[AIMACCURACY];

  // NOTE:  skill = Bot[ent->client->pers.botindex].skill[COMBATSKILL];

  //================================================

  // Fire_Chicken -------------------------
  if (ent->client->battlemode == 0x00000008)
    aim *= 0.7;

  //================================================

  // Fire_Shift -------------------------
  if (ent->client->battlemode & (0x00020000|0x00040000)) {
    GetAimAngle(ent,aim,distance);
    if (--ent->client->battlesubcnt > 0) {
      if (ent->groundentity) {
        if (ent->client->battlemode & 0x00020000) {
          ent->client->moveyaw = ent->s.angles[YAW]+90;
          if (ent->client->moveyaw > 180)
            ent->client->moveyaw -= 360; }
        else {
          ent->client->moveyaw = ent->s.angles[YAW]-90;
          if (ent->client->moveyaw < -180)
            ent->client->moveyaw += 360; }
        trace_priority = 3; } }
    else
      ent->client->battlemode &= ~(0x00020000|0x00040000); }

  //================================================

  // always try to dodge ---------------------------
  if (ent->groundentity && !ent->waterlevel) {
    AngleVectors(target->client->v_angle, v, NULL, NULL);
    VectorScale(v,300,v);
    VectorSet(vv,0,0,target->viewheight-8);
    VectorAdd(target->s.origin,vv,vv);
    VectorAdd(vv,v,v);
    VectorSet(v1, -4, -4,-4);
    VectorSet(v2, 4, 4, 4);
    tr = gi.trace(vv,v1,v2,v,target,MASK_SHOT);
    if (tr.ent == ent)
      if ((tr.endpos[2] > (ent->s.origin[2]+4)) && (random() < 0.4)) { // BUG!
        ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
        ent->client->battleduckcnt = 2+8*random(); }
      else
      if (tr.endpos[2] < (ent->s.origin[2]+JumpMax-24))
        if (ent->client->routetrace) {
          if (Bot_Fall(ent,ent->s.origin,0))
            trace_priority = 3;; }
        else {
          ent->moveinfo.speed = 0.5;
          ent->velocity[2] = 300;
          ent->client->anim_priority = ANIM_JUMP; } }

  //================================================

  // Fire_Ignore -------------------------
  if (ent->client->battlemode & 0x10000000) {
    if (--ent->client->battlecount > 0)
      if (ent->client->current_enemy == ent->client->prev_enemy)
        return;
    ent->client->battlemode = 0x00000000; }

  //================================================

  // Fire_PreStayFire -------------------------
  if (ent->client->battlemode & 0x00000002) {
    if (--ent->client->battlecount > 0) {
      GetAimAngle(ent,aim,distance);
      if (ent->groundentity)
        if (target->client->weaponstate == WEAPON_FIRING)
          ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
      trace_priority = 4;
      return; }
    if (!(ent->client->battlemode & (0x00020000|0x00040000)))
      ent->client->battlemode = 0x00000004;
    ent->client->battlecount = 5+(int)(20*random()); }

  //================================================

  // Fire_StayFire -------------------------
  if (ent->client->battlemode & 0x00000004) {
    if (--ent->client->battlecount > 0) {
      CanUseWeapon(ent,WEAP_BFG);
      aim *= 0.95;
      GetAimAngle(ent,aim,distance);
      if (ent->groundentity)
        if (target->client->weaponstate == WEAPON_FIRING)
          ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
      if (!(ent->client->battlemode & (0x00020000|0x00040000)))
        trace_priority = 4;
//      if ((GetKindWeapon(ent->client->pers.weapon) & WEAP_BFG|WEAP_GRENADELAUNCHER) || InSight(ent,target))
      if ((GetKindWeapon(ent->client->pers.weapon) & WEAP_BFG) || InSight(ent,target))
        ent->client->buttons |= BUTTON_ATTACK;
      return; }
    ent->client->battlemode = 0x00000000; }

  //================================================

  if (ent->client->battlemode & 0x00000010) {
    if (--ent->client->battlecount > 0) {
     CanUseWeapon(ent,WEAP_BFG);
      aim *= 0.95;
      GetAimAngle(ent,aim,distance);
      if (ent->groundentity)
        if (target->client->weaponstate == WEAPON_FIRING) {
          if (GetKindWeapon(ent->client->pers.weapon) == WEAP_BFG) {
            if (target->s.origin[2] > ent->s.origin[2])
              ent->client->ps.pmove.pm_flags |= PMF_DUCKED; }
          else
            ent->client->ps.pmove.pm_flags |= PMF_DUCKED; }
      trace_priority = 3;
      ent->client->moveyaw = ent->s.angles[YAW];
//      if ((GetKindWeapon(ent->client->pers.weapon) & WEAP_BFG|WEAP_GRENADELAUNCHER) || InSight(ent,target))
      if ((GetKindWeapon(ent->client->pers.weapon) & WEAP_BFG) || InSight(ent,target))
        ent->client->buttons |= BUTTON_ATTACK;
      return; }
    ent->client->battlemode = 0x00000000; }

  //================================================

  // Always avoid explosions -------------------------
  if (--ent->client->battlecount > 0) {
    CanUseWeapon(ent,WEAP_BFG);
    aim *= 0.95;
    GetAimAngle(ent,aim,distance);
    if (ent->groundentity)
      if (target->client->weaponstate == WEAPON_FIRING) {
        if (GetKindWeapon(ent->client->pers.weapon) == WEAP_BFG) {
          if (target->s.origin[2] > ent->s.origin[2])
            ent->client->ps.pmove.pm_flags |= PMF_DUCKED; }
        else
          ent->client->ps.pmove.pm_flags |= PMF_DUCKED; }
    trace_priority = 3;
    ent->client->moveyaw = ent->s.angles[YAW]+180;
    if (ent->client->moveyaw > 180)
      ent->client->moveyaw -= 360;
//      if ((GetKindWeapon(ent->client->pers.weapon) & WEAP_BFG|WEAP_GRENADELAUNCHER) || InSight(ent,target))
      if ((GetKindWeapon(ent->client->pers.weapon) & WEAP_BFG) || InSight(ent,target))
        ent->client->buttons |= BUTTON_ATTACK;
    return; }

  //================================================

  // Fire_BFG -------------------------
  if (ent->client->battlemode & 0x00010000) {
    if (--ent->client->battlecount > 0) {
      CanUseWeapon(ent,WEAP_BFG);
      aim *= 0.95;
      GetAimAngle(ent,aim,distance);
      if (ent->groundentity)
        if (target->client->weaponstate == WEAPON_FIRING)
          ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
      trace_priority = 2;
//      if ((GetKindWeapon(ent->client->pers.weapon) & WEAP_BFG|WEAP_GRENADELAUNCHER) || InSight(ent,target))
      if ((GetKindWeapon(ent->client->pers.weapon) & WEAP_BFG) || InSight(ent,target))
        ent->client->buttons |= BUTTON_ATTACK;
      return; }
    ent->client->battlemode = 0x00000000; }

  //================================================

  // Fire_SeekRefuge -------------------------
  if (ent->client->battlemode & 0x00001000) {
    if (--ent->client->battlecount > 0) {
      aim *= 0.95;
      GetAimAngle(ent,aim,distance);
      if (ent->groundentity)
        if (target->client->weaponstate == WEAPON_FIRING)
          if (GetKindWeapon(ent->client->pers.weapon) == WEAP_BFG)
            ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
      trace_priority = 2;
//      if ((GetKindWeapon(ent->client->pers.weapon) & WEAP_BFG|WEAP_GRENADELAUNCHER) || InSight(ent,target))
      if ((GetKindWeapon(ent->client->pers.weapon) & WEAP_BFG) || InSight(ent,target))
        ent->client->buttons |= BUTTON_ATTACK;
      return; }
    ent->client->battlemode = 0x00000000;
    ent->client->pers.routeindex -= 2; }

  //================================================

  if (ent->client->combatstate & ~0x00000001)
    if (ent->client->movestate & (0x00000100|0x00000080|0x00000040|0x00000200))
      if (abs(target->s.origin[2]-ent->s.origin[2]) < 300) {
        // GrenadeLauncher
        if (CanUseWeapon(ent,WEAP_GRENADELAUNCHER)) {
          GetAimAngle(ent,aim,distance);
          if (ent->groundentity)
            if ((target->client->weaponstate == WEAPON_FIRING) || (ent->client->movestate & (0x00000100|0x00000080|0x00000040|0x00000200)))
              ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
          ent->client->buttons |= BUTTON_ATTACK;
          trace_priority = 2;
          return; }
        // Handgrenades
        if (CanUseWeapon(ent,WEAP_GRENADES)) {
          GetAimAngle(ent,aim,distance);
          if (ent->groundentity)
            if (target->client->weaponstate == WEAPON_FIRING)
              ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
          if (ent->client->weaponstate == WEAPON_READY)
            ent->client->buttons |= BUTTON_ATTACK;
          trace_priority = 2;
          return; } //}

        // Mines
        if (CanUseWeapon(ent,WEAP_PROXMINE)) {
          GetAimAngle(ent,aim,distance);
          if (ent->groundentity)
            if (target->client->weaponstate == WEAPON_FIRING)
              ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
          if (ent->client->weaponstate == WEAPON_READY)
            ent->client->buttons |= BUTTON_ATTACK;
          trace_priority = 2;
          return; } }


  //================================================

  k=0;
  if (ent->client->battlemode & ~(0x00020000|0x00040000))
    if (Bot[ent->client->pers.botindex].skill[COMBATSKILL] > (random()*Bot[ent->client->pers.botindex].skill[COMBATSKILL]))
      if ((30*random()) < Bot[ent->client->pers.botindex].skill[AGGRESSION]) {
        enewep = GetKindWeapon(target->client->pers.weapon);
        if (ent->client->routetrace)
		{
			// && enewep != WEAP_RAILGUN) {
          for (i=ent->client->pers.routeindex; i<(ent->client->pers.routeindex+10); i++) {
            if (i >= TotalRouteNodes) break;
            if (Route[i].state == 3)
              if (Route[i].ent->solid == SOLID_TRIGGER) {
                k = 1; break; } }
          if (!k) {
            GetAimAngle(ent,aim,distance);
            f =target->s.angles[YAW]-ent->s.angles[YAW];
            if (f > 180)  f = -(360-f);
            if (f < -180) f = -(f+360);
            if (f <= -160) {
              ent->client->battlemode |= 0x00040000; // strafe left
              ent->client->battlesubcnt = 5+(int)(16*random()); }
            else
            if (f >= 160) {
              ent->client->battlemode |= 0x00020000; // strafe right
              ent->client->battlesubcnt = 5+(int)(16*random()); } } } }

  //================================================

  // Always avoid invincible enemy!
  if (target->client->invincible_framenum > level.framenum) {
    GetAimAngle(ent,aim,distance);
    trace_priority = 3;
    ent->client->moveyaw = ent->s.angles[YAW]+180; // Turn around!
    if (ent->client->moveyaw > 180)
      ent->client->moveyaw -= 360;
    return; }

  //================================================

  // Always use the quad
  if (ent->client->quad_framenum > level.framenum)
    //SuperShotgun, HyperBlaster, Chaingun
//    if (CanUseWeapon(ent,WEAP_SUPERSHOTGUN)
    if (CanUseWeapon(ent,WEAP_LASER)
/*    || (CanUseWeapon(ent,WEAP_HYPERBLASTER))
    || (CanUseWeapon(ent,WEAP_CHAINGUN))) {*/
    || (CanUseWeapon(ent,WEAP_ASSRIFLE))
    || (CanUseWeapon(ent,WEAP_ZMG))) {
      GetAimAngle(ent,aim,distance);
//      if ((GetKindWeapon(ent->client->pers.weapon) & WEAP_BFG|WEAP_GRENADELAUNCHER) || InSight(ent,target))
//        ent->client->buttons |= BUTTON_ATTACK;
      if (InSight(ent,target))
        ent->client->buttons |= BUTTON_ATTACK;
      trace_priority = 2;
      enewep = GetKindWeapon(target->client->pers.weapon);
//      if (enewep < WEAP_MACHINEGUN || enewep == WEAP_GRENADES) {
      if (enewep < WEAP_MACHINEGUN || enewep == WEAP_GRENADES || enewep == WEAP_PROXMINE) {
        ent->client->battlemode |= 0x00000010;
        ent->client->battlecount = 8+(int)(10*random()); }
      return; }

  //================================================

  // Fire Refuge
  if (SkillLevel[Bot[ent->client->pers.botindex].skill[COMBATSKILL]] & 0x00001000)
    if (ent->client->battlemode == 0x00000000)
      if (ent->client->routetrace && ent->client->pers.routeindex > 1) {
        enewep = GetKindWeapon(target->client->pers.weapon);
//        j = (enewep >= WEAP_CHAINGUN && enewep != WEAP_GRENADES)?1:0;
        j = (enewep >= WEAP_ZMG && enewep != WEAP_GRENADES && enewep != WEAP_PROXMINE)?1:0;
        Get_RouteOrigin(ent->client->pers.routeindex-2,v);
        if (fabs(v[2]-ent->s.origin[2]) < JumpMax && j==1) {
          if (GetKindWeapon(ent->client->pers.weapon) & WEAP_GRENADELAUNCHER|WEAP_ROCKETLAUNCHER) {
            ent->client->battlemode |= 0x00001000;
            ent->client->battlecount = 8+(int)(10*random());
            trace_priority = 4;
            return; } } }

  if (!ent->client->routetrace && distance < 100) {
    ent->client->battlecount = 4+(int)(8*random());
    trace_priority = 4; }

/*  //BFG
  if (distance > 400)
    if (B_UseWeapon(ent,aim,distance,WEAP_BFG))
      goto FIRED;
*/
  //Hyper Blaster
  if (distance < 1200)
//    if (B_UseWeapon(ent,aim,distance,WEAP_HYPERBLASTER))
    if (B_UseWeapon(ent,aim,distance,WEAP_ASSRIFLE))
      goto FIRED;

  //Rocket
  if ((distance > 100 && distance < 1200))
    if (B_UseWeapon(ent,aim,distance,WEAP_ROCKETLAUNCHER))
      goto FIRED;

/*  //Railgun
  if (distance < 1200)
    if (B_UseWeapon(ent,aim,distance,WEAP_RAILGUN))
      goto FIRED;
*/
  //Grenade Launcher
  if (distance > 100 && distance < 400)
    if ((target->s.origin[2]-ent->s.origin[2]) < 200)
      if (B_UseWeapon(ent,aim,distance,WEAP_GRENADELAUNCHER))
        goto FIRED;

  //Chain Gun
  if (distance < 1200)
//    if (B_UseWeapon(ent,aim,distance,WEAP_CHAINGUN))
    if (B_UseWeapon(ent,aim,distance,WEAP_ZMG))
      goto FIRED;

  //Machine Gun
  if (distance < 600)
    if (B_UseWeapon(ent,aim,distance,WEAP_MACHINEGUN))
      goto FIRED;

  //SuperShotgun
  if (distance < 800)
//    if (B_UseWeapon(ent,aim,distance,WEAP_SUPERSHOTGUN))
    if (B_UseWeapon(ent,aim,distance,WEAP_LASER))
      goto FIRED;

  // Should be firing?
  if (ent->groundentity && distance >= 400)
    if (SkillLevel[Bot[ent->client->pers.botindex].skill[COMBATSKILL]] & 0x10000000)
      if (ent->client->movestate & ~(0x00000020|0x00000040|0x00000080|0x00000400|0x00000100|0x00000200)) {
        ent->client->battlemode = 0x10000000;
        ent->client->battlecount = 5+(int)(10*random()); }

  //Shotgun
  if (distance < 800)
//    if (B_UseWeapon(ent,aim,distance,WEAP_SHOTGUN))
    if (B_UseWeapon(ent,aim,distance,WEAP_RCP90))
      goto FIRED;

  //Hand Grenade
  if (distance < 400)
    if (B_UseWeapon(ent,aim,distance,WEAP_GRENADES))
      goto FIRED;

  //Mines
  if (distance < 400)
    if (B_UseWeapon(ent,aim,distance,WEAP_PROXMINE))
      goto FIRED;

//if (g_gametype->value != 5)
//{
  //Blaster
//  if (distance < 50)

	if (B_UseWeapon(ent,aim,distance,WEAP_BLASTER))
		goto FIRED;

	if (distance < 1)
	  if (B_UseWeapon(ent,aim,distance,WEAP_SWORD))
		goto FIRED;

//}

  //Slapper
    

  return;

FIRED: // Shoot the weapon

  if (ent->client->battlemode == 0x00000008) {
    if (--ent->client->battlesubcnt > 0)
      if (ent->groundentity && ent->waterlevel < 2) {
        f =target->s.angles[YAW]-ent->s.angles[YAW];
        if (f > 180)  f = -(360-f);
        if (f < -180) f = -(f+360);
        if (fabs(f) >= 150)
          ent->client->battlemode = 0x00000000;
        else {
          if (ent->client->weaponstate != WEAPON_READY)
            if (target->s.origin[2] < ent->s.origin[2]) {
//              if (GetKindWeapon(ent->client->pers.weapon) & WEAP_ROCKETLAUNCHER|WEAP_GRENADELAUNCHER|WEAP_RAILGUN)
              if (GetKindWeapon(ent->client->pers.weapon) & WEAP_ROCKETLAUNCHER|WEAP_GRENADELAUNCHER)
                ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
              else
              if (Bot[ent->client->pers.botindex].skill[COMBATSKILL] >= 7) {
                if (GetKindWeapon(ent->client->pers.weapon) & WEAP_RCP90|WEAP_LASER|WEAP_BLASTER)
//                if (GetKindWeapon(ent->client->pers.weapon) & WEAP_SHOTGUN|WEAP_SUPERSHOTGUN|WEAP_BLASTER)
//                if (GetKindWeapon(ent->client->pers.weapon) & WEAP_SHOTGUN|WEAP_SUPERSHOTGUN|WEAP_BLASTER|WEAP_SWORD)

                  ent->client->ps.pmove.pm_flags |= PMF_DUCKED; } }
          trace_priority = 4; }
        return; }
    else
      ent->client->battlemode = 0x00000000;
  else
  if (ent->client->battlemode == 0x00000000 && distance > 200)
    if (ent->groundentity && ent->waterlevel < 2)
      if (9*random() > Bot[ent->client->pers.botindex].skill[AGGRESSION]) {
        if (GetKindWeapon(ent->client->pers.weapon) > WEAP_BLASTER && target->client->current_enemy != ent) {
          f =target->s.angles[YAW]-ent->s.angles[YAW];
          if (f > 180)  f = -(360-f);
          if (f < -180) f = -(f+360);
          if (fabs(f) < 150) {
            ent->client->battlemode = 0x00000008;
            ent->client->battlesubcnt = 5+(int)(random()*8);
            trace_priority = 4; } } } }
}

//============================================================
void Set_Combatstate(edict_t *ent) {
vec3_t vtmp;
float distance;

  if (ent->client->movestate & 0x00000001) return;

  if (!G_ClientInGame(ent->client->current_enemy)) {
    ent->client->battleduckcnt = 0;
    ent->client->current_enemy = NULL;
    ent->client->combatstate &= ~0x00000001;
    return; }

  if (!Bot_trace(ent,ent->client->current_enemy)) {
    if (ent->client->targetlock <= level.time) {
      ent->client->current_enemy = NULL;
      return; }
    ent->client->combatstate |= 0x00000001; }
  else {
    ent->client->targetlock = level.time+1.2;
    ent->client->combatstate &= ~0x00000001;
    ent->client->battlemode &= ~0x00000040; }

  VectorSubtract(ent->client->current_enemy->s.origin,ent->s.origin,vtmp);
  distance = VectorLength(vtmp);

  if (!(ent->client->combatstate & 0x00000001))
    Combat_Normal(ent,distance);
  else
  if (ent->client->combatstate & 0x00001000)
    Combat_Normal(ent,distance);
  else
    Combat_LevelX(ent,distance);

  if (ent->client->current_enemy) {
    ent->client->prev_enemy = ent->client->current_enemy;
    VectorCopy(ent->client->current_enemy->s.origin, ent->client->targ_old_origin); }
}

//====================================================
//============ BOT ENEMY SEARCHING ROUTINES ==========
//====================================================

//====================================================
void Bot_SearchEnemy(edict_t *ent) {
qboolean tmpflg=false;
edict_t *target=NULL;
edict_t *trent;
int i,j;
vec3_t vdir;

  if (ent->client->current_enemy)
    if (Bot_trace(ent,ent->client->current_enemy))
      tmpflg = true;

  j = (random() < 0.5)?0:-1;

  for (i=1; i <= maxclients->value && !target; i++) {
    if (j)
      trent = &g_edicts[i];
    else
      trent = &g_edicts[(int)(maxclients->value)-i+1];
    if (!trent->inuse || ent == trent || trent->deadflag) continue;
	if (teamdm->value)
		if (trent->client->resp.teamdm_team == ent->client->resp.teamdm_team)
			continue;
    if (ent->client->current_enemy == trent) continue;
    if (ent->client->current_enemy && ent->client->current_enemy->health < 1) continue; // raven - added deadflag check
    if (trent->movetype != MOVETYPE_NOCLIP) {
      if (InSight(ent,trent)) {
        VectorSubtract(trent->s.origin, ent->s.origin, vdir);
        if (!tmpflg && !target) {
          float vr = (float)Bot[ent->client->pers.botindex].skill[VRANGEVIEW];
          float hr = (float)Bot[ent->client->pers.botindex].skill[HRANGEVIEW];
          float pitch = fabs(Get_pitch(vdir)-ent->s.angles[PITCH]);
          if (pitch > 180) pitch = 360-pitch;
          if (pitch <= vr) {
            float yaw = Get_yaw(vdir);
            yaw = fabs(yaw-ent->s.angles[YAW]);
            if (yaw > 180) yaw = 360-yaw;
            if (yaw <= hr || (ent->client->movestate & (0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800)))
              target = trent; } }
        if (!tmpflg && !target && trent->mynoise && trent->mynoise2) {
          if (trent->mynoise->teleport_time >= (level.time-FRAMETIME)) {
            VectorSubtract(trent->mynoise->s.origin, ent->s.origin, vdir);
            if (VectorLength(vdir) < 300) {
              if ((9*random()) < 1+rand()%8)
                target = trent; } }
            if (!target && trent->mynoise2->teleport_time >= (level.time-FRAMETIME)) {
              VectorSubtract(trent->mynoise->s.origin, ent->s.origin, vdir);
              if (VectorLength(vdir) < 100) {
                if ((9*random()) < 1+rand()%8)
                  target = trent; } } } }
      else
      if (!tmpflg && trent->mynoise)
        if (trent->mynoise->teleport_time >= (level.time-FRAMETIME)) {
          trace_t tr;
          AngleVectors(trent->client->v_angle, vdir, NULL, NULL);
          VectorScale(vdir,200,vdir);
          VectorAdd(trent->s.origin,vdir,vdir);
          tr = gi.trace(trent->s.origin,NULL,NULL,vdir,trent,MASK_SHOT);
          VectorSubtract(ent->s.origin, tr.endpos, vdir);
          if (VectorLength(vdir) < 500) {
            VectorCopy(tr.endpos,vdir);
            tr = gi.trace(ent->s.origin,NULL,NULL,vdir,ent,MASK_SHOT);
            if (tr.fraction == 1.0 && (9*random()) < 1+rand()%8) {
              target = trent;
              ent->client->battlemode |= 0x00000040;
              VectorCopy(vdir,ent->client->vtemp); } } } } }

  if (target && !tmpflg)
    ent->client->current_enemy = target;
  else
  if (target && ent->client->current_enemy)
    if (GetKindWeapon(target->client->pers.weapon) > GetKindWeapon(ent->client->current_enemy->client->pers.weapon))
      ent->client->current_enemy = target;
}

//==============================================
//============ ITEM HANDLING ROUTINES ==========
//==============================================

//==============================================
void InitAllItems(void) {

  // set IT_ARMOR lookups
  item_jacketarmor = FindItem("Jacket Armor");
/*  item_combatarmor = FindItem("Combat Armor");
  item_bodyarmor   = FindItem("Body Armor");
  item_armorshard  = FindItem("Armor Shard");
  item_powerscreen = FindItem("Power Screen");
  item_powershield = FindItem("Power Shield");
*/
  // set IT_AMMO lookups
//  item_shells = FindItem("shells");
//  item_cells = FindItem("cells");
  item_bullets = FindItem("bullets");
  item_rockets = FindItem("rockets");
//  item_slugs = FindItem("slugs");
  item_grenades = FindItem("grenades");

    if (g_gametype->value == 2)
    {
        item_proxmine = FindItem("Remote Mines");
    } else
  item_proxmine = FindItem("Proximity Mines");

  // set IT_WEAPON lookups
  item_slapper = FindItem("Slapper");
  item_blaster = FindItem("PP7");
  item_shotgun = FindItem("RCP-90");
//  item_supershotgun = FindItem("super shotgun");
  item_supershotgun = FindItem("Laser");

  item_handgrenade = FindItem("grenades");
  item_machinegun = FindItem("KF7 Soviet");
  item_chaingun = FindItem("ZMG 9mm");
  item_grenadelauncher = FindItem("grenades");
  item_rocketlauncher = FindItem("rocket launcher");
//  item_railgun = FindItem("railgun");
  item_hyperblaster = FindItem("AR33 Assault Rifle");
//  item_bfg = FindItem("grenades");

  // set IT_HEALTH lookups
  item_adrenaline = FindItem("Adrenaline");
  item_health = FindItem("Health");
  item_stimpak = FindItem("Health");
  item_health_large = FindItem("Health");
  item_health_mega = FindItem("Health");

  // set IT_POWERUP lookups
  item_quad = FindItem("Quad Damage");
  item_invulnerability = FindItem("Invulnerability");
  item_silencer = FindItem("Silencer");
  item_breather = FindItem("Rebreather");
  item_enviro = FindItem("Environment Suit");

  // set IT_PACK lookups
  item_pack = FindItem("Ammo Pack");
  item_bandolier = FindItem("Bandolier");

  // set IT_NODE lookups
  item_navi1 = FindItem("Roam Navi1");
  item_navi2 = FindItem("Roam Navi2");
  item_navi3 = FindItem("Roam Navi3");
}

//==============================================
int GetKindWeapon(gitem_t *it) {

  if (it->weaponthink == Weapon_Shotgun)         return WEAP_RCP90;
//  if (it->weaponthink == Weapon_Shotgun)         return WEAP_SHOTGUN;
//  if (it->weaponthink == Weapon_SuperShotgun)    return WEAP_SUPERSHOTGUN;
  if (it->weaponthink == Weapon_Laser		)    return WEAP_LASER;

//  if (it->weaponthink == Weapon_Machinegun)      return WEAP_MACHINEGUN;
  if (it->weaponthink == Weapon_Machinegun)      return WEAP_MACHINEGUN;
//  if (it->weaponthink == Weapon_Chaingun)        return WEAP_CHAINGUN;
  if (it->weaponthink == Weapon_Chaingun)        return WEAP_ZMG;
  if (it->weaponthink == Weapon_Grenade)         return WEAP_GRENADES;
// ion mine support
  if (it->weaponthink == Weapon_Proxmine)        return WEAP_PROXMINE;
  if (it->weaponthink == Weapon_GrenadeLauncher) return WEAP_GRENADELAUNCHER;
  if (it->weaponthink == Weapon_RocketLauncher)  return WEAP_ROCKETLAUNCHER;
//  if (it->weaponthink == Weapon_AssRifle)        return WEAP_HYPERBLASTER;
  if (it->weaponthink == Weapon_AssRifle)        return WEAP_ASSRIFLE;
//  if (it->weaponthink == Weapon_Railgun)         return WEAP_RAILGUN;
//  if (it->weaponthink == Weapon_BFG)             return WEAP_BFG;
// ion slap
  if (it->weaponthink == Weapon_Sword)           return WEAP_SWORD;
  return WEAP_BLASTER;
}


//====================================================
//============ BOT MOVEMENT TESTING ROUTINES =========
//====================================================

//============================================================
int Bot_TestMove(edict_t *ent,float ryaw,vec3_t pos,float dist,float *bottom) {
float yaw;
vec3_t trstart,trend;
vec3_t trmin,trmax,v,vv;
trace_t tr;
float tracelimit;
int contents;

  if (ent->waterlevel >= 1)
    tracelimit = 75;
  else
  if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
    tracelimit = 26;
  else
    tracelimit = JumpMax+5;

  VectorSet(trmin,-16,-16,-24);
  VectorSet(trmax,16,16,3);

  if (ent->client->routetrace)
    VectorSet(vv,16,16,0);
  else
    VectorSet(vv,16,16,3);

  if (ent->client->routetrace)
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
      if (ent->waterlevel < 2) {
        Get_RouteOrigin(ent->client->pers.routeindex,v);
        if ((v[2]-ent->s.origin[2]) > 20)
          trmax[2] = 31; }

  yaw = DEG2RAD(ryaw);
  trend[0] = cos(yaw)*dist;
  trend[1] = sin(yaw)*dist;
  trend[2] = 0;
  VectorAdd(trend, ent->s.origin, trstart);

  VectorCopy(trstart,trend);
  trend[2] += 1;
  tr = gi.trace(trstart, trmin, trmax, trend, ent, MASK_BOTSOLIDX);
  trmax[2] += 1;
  if (tr.allsolid || tr.startsolid || tr.fraction != 1.0) {
    float i;
    qboolean moveok = false;
    VectorCopy(trstart, trend);
    for (i = 4; i <(tracelimit+4); i += 4) {
      trstart[2] = ent->s.origin[2]+i;
      tr = gi.trace(trstart, trmin, vv, trend, ent, MASK_BOTSOLIDX);
      if (!tr.allsolid && !tr.startsolid && tr.fraction > 0) {
        moveok = true;
        break; } }
    if (!moveok) return 0;
    *bottom = tr.endpos[2]-ent->s.origin[2];
    if (!ent->client->routetrace) {
      if (tr.plane.normal[2] < 0.7 && (!ent->client->waterstate && ent->groundentity))
        return 0; }
    else {
      Get_RouteOrigin(ent->client->pers.routeindex,v);
      if (tr.plane.normal[2] < 0.7 && v[2] < ent->s.origin[2])
        return 0; }
    if (*bottom >tracelimit-5) return 0;
    VectorCopy(tr.endpos,pos);
    if (trmax[2] == 32) return 1;
    VectorCopy(pos,trend);
    trend[2] += 28;
    tr = gi.trace(pos, trmin, trmax, trend, ent, MASK_BOTSOLIDX);
    return (!tr.allsolid && !tr.startsolid && tr.fraction == 1.0)?1:2; }
  else {
    VectorCopy(trstart,pos);
    VectorCopy(trstart, trend);
    trstart[2] = trend[2]-8190;
    tr = gi.trace(trend, trmin, trmax, trstart, ent, MASK_BOTSOLIDX|MASK_OPAQUE);
    *bottom = tr.endpos[2]-ent->s.origin[2];
    contents = 0;
    if (!ent->waterlevel) {
      if (ent->client->enviro_framenum > level.framenum)
        contents = CONTENTS_LAVA;
      else
        contents = (CONTENTS_LAVA|CONTENTS_SLIME); }
    if (tr.contents & contents)
      *bottom = -9999;
    else
    if (tr.surface->flags & SURF_SKY)
      *bottom = -9999;
    if (!ent->waterlevel && !ent->groundentity)
      if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
        if (ent->velocity[2] > 10 && trmax[2] == 4)
          return 2;
    if (trmax[2] == 32) return 1;
    VectorCopy(pos,trend);
    trend[2] += 28;
    tr = gi.trace(pos, trmin, trmax, trend, ent, MASK_BOTSOLIDX);
    return (!tr.allsolid && !tr.startsolid && tr.fraction == 1.0)?1:2; }
}

//============================================================
qboolean Bot_Watermove(edict_t *ent, vec3_t pos, float dist, float upd) {
trace_t tr;
vec3_t trmin,trmax,touchmin;
float i,j,max,vec;

  VectorCopy(ent->s.origin,trmax);

  trmax[2] += upd;
  tr = gi.trace(ent->s.origin, ent->mins, ent->maxs, trmax, ent, MASK_BOTSOLIDX);
  if (!tr.allsolid && !tr.startsolid && tr.fraction > 0) {
    VectorCopy(tr.endpos,pos);
    return true; }

  VectorCopy(ent->s.origin,trmin);
  trmin[2] += upd;
  vec = -1;
  max = 0;
  for (i=0; i<360; i+=10) {
    if (i && upd > -13 && upd < 0) break;
    if (i > 60 && i < 300) continue;
    j = ent->client->moveyaw+i;
    if (j > 180) j -= 360;
    else if (j < -180) j += 360;
    else j = i;
    touchmin[0] = cos(j)*24;
    touchmin[1] = sin(j)*24;
    touchmin[2] = 0;
    VectorAdd(trmin,touchmin,trmax);
    tr = gi.trace(trmax, ent->mins, ent->maxs, trmin, ent, MASK_BOTSOLIDX);
    if (!tr.allsolid && !tr.startsolid) {
      VectorAdd(tr.endpos,touchmin,trmax);
      tr = gi.trace(trmax, ent->mins, ent->maxs, trmax, ent, MASK_BOTSOLIDX);
      if (!tr.allsolid && !tr.startsolid) {
        vec = i; break; } } }

  if (vec == -1) return false;

  VectorCopy(trmax,pos);
  if (upd < 0)
    ent->velocity[2] = 0;

  return true;
}

//============================================================
qboolean Bot_moveW(edict_t *ent, float ryaw, vec3_t pos, float dist, float *bottom) {
float yaw;
vec3_t trstart,trend;
trace_t tr;
int contents;

  if (ent->client->enviro_framenum > level.framenum)
    contents = CONTENTS_LAVA;
  else
    contents = (CONTENTS_LAVA|CONTENTS_SLIME);

  yaw = DEG2RAD(ryaw);
  trend[0] = cos(yaw)*dist;
  trend[1] = sin(yaw)*dist;
  trend[2] = 0;
  VectorAdd(trend, ent->s.origin, trstart);

  VectorCopy(trstart,pos);
  VectorCopy(trstart,trend);

  trstart[2] = trend[2]-8190;
  tr = gi.trace(trend, ent->mins, ent->maxs, trstart,ent, MASK_BOTSOLIDX|CONTENTS_WATER);
  if ((trend[2]-tr.endpos[2]) >= 95) return false;
  if (tr.contents & contents) return false;
  if (!(tr.contents & CONTENTS_WATER)) return false;

  *bottom = tr.endpos[2]-ent->s.origin[2];

  return true;
}

//============================================================
qboolean Bot_Jump(edict_t *ent, vec3_t pos, float dist) {
float x,yaw,tdist,bottom,speed;
vec3_t temppos;

  yaw = ent->client->moveyaw;

  Bot_TestMove(ent,yaw,temppos,dist,&bottom);
  if (bottom > -JumpMax) return false;

  for (x=2; x<=16; x++) {
    tdist = dist*x;
    if (Bot_TestMove(ent,yaw,temppos,tdist,&bottom)) {
      if (bottom <= JumpMax && bottom > -JumpMax)
        if (Get_FlyingSpeed(bottom,x,dist,&speed)) {
          speed *= 1.5;
          if (speed > 1.2) speed = 1.2;
          ent->moveinfo.speed = speed;
          ent->velocity[2] = 300;
          SetBotAnim(ent);
          return true; }
      continue; }
    else
      return false; }

  return false;
}

//============================================================
qboolean Bot_Fall(edict_t *ent,vec3_t pos,float dist) {
float x,n,speed,vel,yori,ypos;
vec3_t vdir,vv;
int mf = 0;
int mode = 0;

  if (ent->client->routetrace) {
    mode = 2;
    Get_RouteOrigin(ent->client->pers.routeindex,vv);
    ypos = vv[2];
    if (!HazardCheck(ent,vv)) {
      if (++ent->client->pers.routeindex >= TotalRouteNodes)
        ent->client->pers.routeindex = 0;
      return false; }
    yori = pos[2];
    VectorSubtract(vv,pos,vdir);
    if (vdir[2] >= 0) goto JUMPCATCH;
    vel = ent->velocity[2];
    n = 1.0;
    for (x=1; x<=30; ++x,n+=x) {
      vel -= (ent->gravity*sv_gravity->value*FRAMETIME);
      yori += vel*0.1;
      if (ypos >= yori) {
        mf = 1;
        break; } }
    VectorCopy(vdir,vv);
    vv[2] = 0;
    if (Route[ent->client->pers.routeindex].state == 5) {
      vv[0] += 0.1*Route[ent->client->pers.routeindex].ent->velocity[0]*x;
      vv[1] += 0.1*Route[ent->client->pers.routeindex].ent->velocity[1]*x; }
    speed = VectorLength(vv)/x;
    if (speed <= 30 && mf) {
      ent->moveinfo.speed = speed/30;
      VectorCopy(pos,ent->s.origin);
      return true; }
    goto JUMPCATCH; }

  goto JMPCHK;

JUMPCATCH:

  vel = 300;
  yori = pos[2];
  mf = 0;

  for (x=1; x<=30; ++x) {
    vel -= (ent->gravity*sv_gravity->value*FRAMETIME);
    yori += vel*0.1;
    if (vel > 0) {
      if (mf == 0) {
        if (ypos < yori)
          mf = 2; } }
    else if (x > 1) {
      if (mf == 0) {
        if (ypos < yori)
          mf = 2; }
      else if (mf == 2) {
        if (ypos >= yori) {
          mf = 1;
          break; } } } }

  VectorCopy(vdir,vv);
  vv[2] = 0;
  if (mode == 2)
    if (Route[ent->client->pers.routeindex].state == 5) {
      vv[0] += 0.1*Route[ent->client->pers.routeindex].ent->velocity[0]*x;
      vv[1] += 0.1*Route[ent->client->pers.routeindex].ent->velocity[1]*x; }

  n = VectorLength(vv);
  if (x > 1) n /= (x-1);

  if (n < 30 && mf) {
    ent->moveinfo.speed = n/30;
    VectorCopy(pos,ent->s.origin);
    ent->velocity[2] = 300;
    SetBotAnim(ent);
    return true; }

JMPCHK:

  if (Bot_Jump(ent,pos,dist)) return true;

  return false;
}

//============================================================
qboolean TargetJump(edict_t *ent,vec3_t tpos) {
float x,n,jvel,vel,yori;
vec3_t vdir,vv;
int mf=0;

  jvel = vel = 300;
  yori = ent->s.origin[2];

  if (!HazardCheck(ent,tpos)) return false;

  VectorSubtract(tpos,ent->s.origin,vdir);

  for (x=1; x<=60; ++x) {
    vel -= (ent->gravity*sv_gravity->value*0.1);
    yori += vel*0.1;
    if (vel > 0) {
      if (mf == 0) {
        if (tpos[2] < yori)
          mf = 2; } }
    else if (x > 1) {
      if (mf == 0) {
        if (tpos[2] < yori)
          mf = 2; }
      else if (mf == 2) {
        if (tpos[2] >= yori) {
          mf = 1;
          break; } } } }

  VectorCopy(vdir,vv);
  vv[2] = 0;

  n = VectorLength(vv);
  if (x > 1) n /= (x-1);

  if (n < 30 && mf) {
    ent->moveinfo.speed = n/30;
    ent->velocity[2] = jvel;
    SetBotAnim(ent);
    return true; }

  return false;
}

//============================================================
qboolean TargetJump_Chk(edict_t *ent,vec3_t tpos,float defvel) {
float x,n,vel,yori;
vec3_t vdir,vv;
int mf = 0;

  vel = defvel+300;
  yori = ent->s.origin[2];

  if (!HazardCheck(ent,tpos)) return false;

  VectorSubtract(tpos,ent->s.origin,vdir);

  for (x=1; x<=60; ++x) {
    vel -= (ent->gravity*sv_gravity->value*0.1);
    yori += vel*0.1;
    if (vel > 0) {
      if (mf == 0) {
        if (tpos[2] < yori)
          mf = 2; } }
    else if (x > 1) {
      if (mf == 0) {
        if (tpos[2] < yori)
          mf = 2; }
      else if (mf == 2) {
        if (tpos[2] >= yori) {
          mf = 1;
          break; } } } }

  VectorCopy(vdir,vv);
  vv[2] = 0;

  n = VectorLength(vv);
  if (x > 1) n /= (x-1);

  return (n<30 && mf!=0);
}

//============================================================
void Get_WaterState(edict_t *ent) {

  ent->watertype = gi.pointcontents(ent->s.origin);
  ent->waterlevel = (ent->watertype & MASK_WATER)?1:0;

  if (ent->waterlevel) {
    float x;
    trace_t tr;
    vec3_t trmin,trmax;
    VectorCopy(ent->s.origin,trmax);
    VectorCopy(ent->s.origin,trmin);
    trmax[2] -= 24;
    trmin[2] += 8;
    tr = gi.trace(trmin, NULL, NULL, trmax, ent, MASK_WATER);
    x = trmin[2]-tr.endpos[2];
    if (tr.allsolid || tr.startsolid || (x < 4.0))
      ent->client->waterstate = 2;
    else
      ent->client->waterstate = (x >= 4.0 && x <= 12.0)?1:0; }
  else
    ent->client->waterstate = 0;
}

//============================================================
void Search_NearbyPod(edict_t *ent) {

  if (Route[ent->client->pers.routeindex].state >= 3) return;

  if ((ent->client->pers.routeindex+1) < TotalRouteNodes)
    if (Route[ent->client->pers.routeindex+1].state < 3) {
      vec3_t v;
      Get_RouteOrigin(ent->client->pers.routeindex+1,v);
      if (TraceX(ent,v)) {
        vec3_t v1,v2;
        float x;
        VectorSubtract(v,ent->s.origin,v1);
        Get_RouteOrigin(ent->client->pers.routeindex,v);
        VectorSubtract(v,ent->s.origin,v2);
        x = fabs(v1[2]);
        if (VectorLength(v1) < VectorLength(v2) && x <= JumpMax && Route[ent->client->pers.routeindex].state <= 1)
          ent->client->pers.routeindex++;
        else {
          if (ent->client->waterstate==0)
            if (v2[2] > JumpMax)
              if (fabs(v1[2]) < JumpMax)
                ent->client->pers.routeindex++; } } }
}

//========================================================
void BotAI(edict_t *ent) {
float dist,x,yaw,iyaw,f1,f2,f3,bottom;
int tempflag,i,j,k;
edict_t *it_ent,*touch[1024],*trent;
vec3_t touchmin,touchmax,v,vv;
vec3_t temppos,trmin,trmax;
qboolean ladderdrop,canrocj,waterjumped;
gitem_t *it;
edict_t *front,*left,*right,*e;
char *str;
cplane_t plane;
trace_t tr;

  myrandom=random();

  if (ent->client->changetime < level.time) {
    RandomizeParameters(ent->client->pers.botindex);
    ent->client->changetime = level.time+30+10*(int)rand()%9; }

  trace_priority = 1;
  ent->client->objshot = false;
  ent->client->buttons &= ~BUTTON_ATTACK;

  if (VectorCompare(ent->s.origin,ent->s.old_origin))
    if (!ent->groundentity && !ent->waterlevel) {
      VectorCopy(ent->s.origin,v);
      v[2] -= 1.0;
      tr = gi.trace(ent->s.origin,ent->mins,ent->maxs,v,ent,MASK_BOTSOLIDX);
      if (!tr.allsolid && !tr.startsolid)
        ent->groundentity = tr.ent; }

  if (JumpMax == 0) {
    x = 300-ent->gravity*sv_gravity->value*0.1;
    JumpMax = 0;
    while (1) {
      JumpMax += x*0.1;
      x -= ent->gravity*sv_gravity->value*0.1;
      if (x < 0) break; } }

  if (!ent->client->havetarget && ent->client->routetrace) {
    j = Bot[ent->client->pers.botindex].skill[PRIMARYWEAP];
    if (j && !ent->client->pers.inventory[j]) {
      it = &itemlist[j];
      if (ent->client->enemy_routeindex < ent->client->pers.routeindex || ent->client->enemy_routeindex >= TotalRouteNodes)
        ent->client->enemy_routeindex = ent->client->pers.routeindex;
      for (i = ent->client->enemy_routeindex+1;i<(ent->client->enemy_routeindex+50);i++) {
        if (i > TotalRouteNodes) break;
        if (Route[i].state == 3) {
          if (Route[i].ent->item == it) {
            ent->client->havetarget = true;
            break; }
          else
          if (Route[i].ent->solid == SOLID_TRIGGER) {
            if (Route[i].ent->item == &itemlist[j]) {
              ent->client->havetarget = true;
              break; } } } }
      ent->client->enemy_routeindex = i; }
    else
      if (j=ITEM_INDEX(item_quad)) {
        it = &itemlist[j];
        if (ent->client->enemy_routeindex < ent->client->pers.routeindex || ent->client->enemy_routeindex >= TotalRouteNodes)
          ent->client->enemy_routeindex = ent->client->pers.routeindex;
        for (i = ent->client->enemy_routeindex+1;i<(ent->client->enemy_routeindex+25);i++) {
          if (i > TotalRouteNodes) break;
          if (Route[i].state == 3) {
            if (Route[i].ent->item == it) {
              if (Route[i].ent->solid == SOLID_TRIGGER) {
                ent->client->havetarget = true;
                break; } } } }
        ent->client->enemy_routeindex = i; } }
  else
  if (ent->client->havetarget)
    if (ent->client->enemy_routeindex < ent->client->pers.routeindex) {
      ent->client->havetarget = false;
      ent->client->enemy_routeindex = ent->client->pers.routeindex; }

  canrocj = (ent->client->pers.inventory[ITEM_INDEX(item_rocketlauncher)] && ent->client->pers.inventory[ITEM_INDEX(item_rockets)] > 0);

  if (ent->client->ps.pmove.pm_flags & PMF_DUCKED) {
    if (ent->client->battleduckcnt > 0)
      if (ent->groundentity)
        goto DCHCANC;
    VectorSet(v,16,16,32);
    VectorCopy(ent->s.origin,v);
    v[2] += 28;
    tr = gi.trace(ent->s.origin,ent->mins,ent->maxs,v,ent,MASK_BOTSOLIDX);
    if (!tr.startsolid && !tr.allsolid && tr.fraction == 1.0) {
      ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
      ent->maxs[2] = 32; } }
  else
  if (ent->velocity[2] > 10 && !ent->groundentity)
    if (!(ent->client->movestate & (0x00000008|0x00000002|0x00000004))) {
      VectorSet(v,16,16,40);
      tr = gi.trace(ent->s.origin,ent->mins,v,ent->s.origin,ent,MASK_BOTSOLIDX);
      if (tr.startsolid || tr.allsolid) {
        ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
        ent->maxs[2] = 4; } }

DCHCANC:

  if (ent->groundentity || ent->waterlevel) {
    if (ent->waterlevel) {
      if (!(ent->client->movestate & 0x00000004))
        ent->client->movestate &= ~(0x00000008|0x00000002|0x00000004); }
    else
      ent->client->movestate &= ~(0x00000008|0x00000002|0x00000004);
    if (ent->groundentity && !ent->waterlevel)
      ent->moveinfo.speed = 1.0;
    else
    if (ent->waterlevel && ent->velocity[2] <= 1)
      ent->moveinfo.speed = 1.0; }

  if ((ent->client->ps.pmove.pm_flags & PMF_DUCKED) && ent->groundentity)
    dist = 10*ent->moveinfo.speed;
  else {
    dist = 30*ent->moveinfo.speed;
    if (ent->groundentity)
      dist *= ent->client->ground_slope; }

  Get_WaterState(ent);

  ent->client->enemysearchcnt += 2;
  if (ent->client->enemysearchcnt >= 10) {
    Bot_SearchEnemy(ent);
    ent->client->enemysearchcnt = 1+rand()%8;
    if (ent->client->enemysearchcnt > 10)
      ent->client->enemysearchcnt = 10;
    if (ent->client->enemysearchcnt < 0)
      ent->client->enemysearchcnt = 0; }

  Set_Combatstate(ent);

  if (trace_priority == 4)
    goto VCHCANSEL;

  if (ent->client->routetrace) {
    if (Route[ent->client->pers.routeindex].state >= 0)
      Search_NearbyPod(ent);
    Get_RouteOrigin(ent->client->pers.routeindex,v);
    if (ent->client->movestate & (0x00000020|0x00000040|0x00000080|0x00000400|0x00000100|0x00000200))
      ent->client->routelocktime = level.time+1.5;
    else
    if (Route[ent->client->pers.routeindex].state <= 3 && ((v[2]-ent->s.origin[2]) > JumpMax && !ent->client->waterstate) && !(ent->client->movestate & 0x00000001)) {
      if (ent->client->routelocktime <= level.time) {
        ent->client->routetrace = false;
        ent->client->routereleasetime = level.time+2.0; } }
    else
    if (!TraceX(ent,v)) {
      k=0;
      if (ent->groundentity) {
        if (ent->groundentity->use == train_use) {
          ent->client->routelocktime = level.time+1.5;
          k = 1; } }
      if (ent->client->routelocktime <= level.time && !k) {
        ent->client->routetrace = false;
        ent->client->routereleasetime = level.time+2.0; } }
    else
      ent->client->routelocktime = level.time+1.5; }

  if (trace_priority == 4)
    goto VCHCANSEL;

  if (ent->client->movestate & 0x00000001) {
    ent->velocity[2] = 200;
    VectorCopy(ent->mins,trmin);
    trmin[2] += 20;
    yaw = DEG2RAD(ent->client->moveyaw);
    touchmin[0] = cos(yaw)*32;
    touchmin[1] = sin(yaw)*32;
    touchmin[2] = 0;
    VectorAdd(ent->s.origin,touchmin,touchmax);
    tr = gi.trace(ent->s.origin, trmin,ent->maxs, touchmax,ent, MASK_BOTSOLID);
    plane = tr.plane;
    if (!(tr.contents & CONTENTS_LADDER) && !tr.allsolid) {
      if (ent->velocity[2] <= 200)
        if (!ent->waterlevel)
          ent->velocity[2] = 200;
      ent->client->movestate &= ~0x00000001;
      ent->moveinfo.speed = 0.25;
      if (ent->client->routetrace) {
        Get_RouteOrigin(ent->client->pers.routeindex,v);
        if (VectorLength(v) > 32) {
          VectorSubtract(v,ent->s.origin,v);
          ent->client->moveyaw = Get_yaw(v);
          if (trace_priority < 2)
            ent->s.angles[YAW] = ent->client->moveyaw; }
        else
          ent->client->pers.routeindex++; } }
    else {
      if (!tr.allsolid)
        VectorCopy(tr.endpos,ent->s.origin);
      VectorCopy(ent->s.origin,touchmin);
      touchmin[2] += 8;
      tr = gi.trace(ent->s.origin, ent->mins,ent->maxs, touchmin,ent, MASK_BOTSOLID);
      x = tr.endpos[2]-ent->s.origin[2];
      ent->s.origin[2] += x;
      e = tr.ent;
      if (x == 0) {
        x = Get_yaw(plane.normal);
        VectorCopy(ent->s.origin,v);
        yaw = x+90;
        if (yaw > 180) yaw -= 360;
        yaw = DEG2RAD(yaw);
        touchmin[0] = cos(yaw)*48;
        touchmin[1] = sin(yaw)*48;
        touchmin[2] = 0;
        VectorAdd(ent->s.origin,touchmin,trmin);
        VectorCopy(trmin,trmax);
        trmin[2] += 32;
        trmax[2] += 64;
        tr = gi.trace(trmin,NULL,NULL,trmax,ent,MASK_BOTSOLID);
        f1 = tr.fraction;
        VectorCopy(ent->s.origin,v);
        iyaw = x -90;
        if (iyaw < 180) iyaw += 360;
        iyaw = DEG2RAD(iyaw);
        touchmin[0] = cos(iyaw)*48;
        touchmin[1] = sin(iyaw)*48;
        touchmin[2] = 0;
        VectorAdd(ent->s.origin,touchmin,trmin);
        VectorCopy(trmin,trmax);
        trmin[2] += 32;
        trmax[2] += 64;
        tr = gi.trace(trmin,NULL,NULL,trmax,ent,MASK_BOTSOLID);
        f2 = tr.fraction;
        x = 0.0;
        if (f1 == 1.0 && f2 != 1.0)
          x = yaw;
        else
          if (f1 != 1.0 && f2 == 1.0)
            x = iyaw;
        if (x != 0.0) {
          touchmin[0] = cos(x)*4;
          touchmin[1] = sin(x)*4;
          touchmin[2] = 0;
          VectorAdd(ent->s.origin,touchmin,trmin);
          tr = gi.trace(ent->s.origin,ent->mins,ent->maxs,trmin,ent,MASK_BOTSOLID);
          if (tr.startsolid || tr.allsolid)
            x = 0;
          else
            VectorCopy(tr.endpos,ent->s.origin); }
        if (x == 0.0) {
          k = 0;
          if (e) {
            if (e->use == door_use) {
              if (e->moveinfo.state == 2)
                k = 1; } }
          if (!k) {
            ent->client->moveyaw += 180;
            if (ent->client->moveyaw > 180)
              ent->client->moveyaw -= 360;
            ent->client->movestate &= ~0x00000001;
            ent->moveinfo.speed = 0.25; } } } }

    if (ent->client->movestate & 0x00000001) {
      if (ent->client->routetrace) {
        Get_RouteOrigin(ent->client->pers.routeindex,v);
        if (v[2] < ent->s.origin[2]) {
          VectorSubtract(ent->s.origin,v,vv);
          vv[2] = 0;
          if (VectorLength(vv) < 32)
            ent->client->pers.routeindex++; } }
      ent->velocity[0] = 0;
      ent->velocity[1] = 0;
      goto VCHCANSEL_L; } }

  if (ent->groundentity && ent->waterlevel <= 1)
    if (trace_priority < 2)
      ent->s.angles[PITCH] = 0;

  if (ent->groundentity && !ent->client->routetrace) {
    if (trace_priority < 3)
      ent->client->moveyaw = ent->s.angles[YAW]; }
  else
  if (trace_priority < 2)
    ent->s.angles[YAW] = ent->client->moveyaw;

  if (!ent->client->routetrace && ent->client->routereleasetime <= level.time) {
    if (ent->client->pers.routeindex >= TotalRouteNodes)
      ent->client->pers.routeindex = 0;
    for (i=0; i<TotalRouteNodes && i<12;i++) {
      if (Route[ent->client->pers.routeindex].state == 21) {
        while (1) {
            ++ent->client->pers.routeindex;
            if (ent->client->pers.routeindex >= TotalRouteNodes) {
              i = TotalRouteNodes;
              break; }
            if (Route[ent->client->pers.routeindex].state == 22) {
              ++ent->client->pers.routeindex;
              break; } }
          continue; }
        else
        if (Route[ent->client->pers.routeindex].state == 22) {
          ++ent->client->pers.routeindex; continue; }
        Get_RouteOrigin(ent->client->pers.routeindex,v);
        if (Route[ent->client->pers.routeindex].state <= 3 && TraceX(ent,v)) {
          if (fabs(v[2]-ent->s.origin[2]) <= JumpMax || ent->client->waterstate == 2) {
            ent->client->routetrace = true;
            ent->client->routelocktime = level.time+1.5;
            break; } }
        if (++ent->client->pers.routeindex >= TotalRouteNodes)
          ent->client->pers.routeindex = 0; } }
    else
    if (ent->client->routetrace) {
      if (Route[ent->client->pers.routeindex].state == 6) {
          it_ent = Route[ent->client->pers.routeindex].ent;
          if (ent->client->pers.routeindex+1 < TotalRouteNodes) {
            Get_RouteOrigin(ent->client->pers.routeindex+1,v);
            ent->client->routetrace = false;
            j = TraceX(ent,v);
            ent->client->routetrace = true;
            if ((!j ||(v[2]-ent->s.origin[2]) > JumpMax)&& it_ent->union_ent) {
              k = ((it_ent->union_ent->s.origin[2]-ent->s.origin[2]) > JumpMax)?1:0;
              VectorSubtract(it_ent->union_ent->s.origin,ent->s.origin,temppos);
              yaw = Get_yaw(temppos);
              if (trace_priority < 2) {
                ent->s.angles[PITCH] = Get_pitch(temppos);
                ent->s.angles[YAW] = yaw; }
              temppos[2] = 0;
              x = VectorLength(temppos);
              if (x == 0 || k) {
                if (it_ent->nextthink >= level.time)
                  ent->client->routelocktime = level.time+1.5;
                goto VCHCANSEL; }
              if (x < dist)
                dist = x;
              if (it_ent->nextthink > level.time)
                ent->client->routelocktime = it_ent->nextthink+1.5;
              else
                ent->client->routelocktime = level.time+1.5;
              if (trace_priority < 3)
                ent->client->moveyaw = yaw;
              goto GOMOVE; } }
        ent->client->pers.routeindex++; }

      if (ent->client->pers.routeindex < TotalRouteNodes) {
        Get_RouteOrigin(ent->client->pers.routeindex,v);
        k = 0;
        if (Route[ent->client->pers.routeindex].state == 7) {
          it_ent = Route[ent->client->pers.routeindex].ent;
          if (it_ent->health && (it_ent->takedamage || it_ent->moveinfo.state != 0))
            k = 2;
          else
          if (it_ent->health) {
            ent->client->pers.routeindex++;
            if (ent->client->pers.routeindex < TotalRouteNodes)
              Get_RouteOrigin(ent->client->pers.routeindex,v); } }
        else {
          VectorSet(touchmax,16,16,4);
          VectorSet(touchmin,-16,-16,0);
          tr = gi.trace(ent->s.origin,touchmin,touchmax,v,ent,MASK_SHOT);
          if (tr.fraction != 1.0 && tr.ent) {
            if (tr.ent->health || tr.ent->takedamage)
              if (tr.ent->classname[0] != 'p' && tr.ent->classname[0] != 'b') {
                ent->client->routelocktime = level.time+1.5;
                it_ent = tr.ent;
                k = 1; } } }
        if (k && !(ent->client->buttons & BUTTON_ATTACK)) {
          trmin[0] = (it_ent->absmin[0]+it_ent->absmax[0])*0.5;
          trmin[1] = (it_ent->absmin[1]+it_ent->absmax[1])*0.5;
          trmin[2] = (it_ent->absmin[2]+it_ent->absmax[2])*0.5;
          if (k == 2) {
            VectorSet(touchmin, 0, 0, ent->viewheight-8);
            VectorAdd(ent->s.origin,touchmin,touchmin);
            tr = gi.trace(it_ent->union_ent->s.origin,NULL,NULL,trmin,it_ent->union_ent,MASK_SHOT);
            VectorSubtract(tr.endpos,ent->s.origin,trmax); }
          else
            VectorSubtract(v,ent->s.origin,trmax);
          if (!ent->client->current_enemy && it_ent->takedamage) {

			  if (g_gametype->value != 5)
			  {
					ent->client->newweapon = item_blaster;
					ChangeWeapon(ent);
					ent->client->pers.weapon->use(ent,item_blaster); 
			  }
			  else
			  {
					ent->client->newweapon = item_supershotgun;
					ChangeWeapon(ent);
					ent->client->pers.weapon->use(ent,item_supershotgun); 
			  }		  
		  }
          if (!ent->client->current_enemy || it_ent->takedamage) {
            ent->s.angles[YAW] = Get_yaw(trmax);
            ent->s.angles[PITCH] = Get_pitch(trmax); }
          if (it_ent->takedamage)
            ent->client->buttons |= BUTTON_ATTACK;
          if (k == 2) {
            if (it_ent->moveinfo.state != 0)
              goto VCHCANSEL; }
          else {
            if (!TraceX(ent,v))
              goto VCHCANSEL; } }

        if (Route[ent->client->pers.routeindex].state == 5 && !ent->client->waterstate) {
          Get_RouteOrigin(ent->client->pers.routeindex -1 ,trmin);
          if ((trmin[2]-ent->s.origin[2]) > JumpMax && (v[2]-ent->s.origin[2]) > JumpMax && ent->waterlevel < 3) {
            ent->client->routetrace = false; } }
        f2=(ent->client->waterstate == 2)?20:(ent->groundentity)?-8:0;
        if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
          f1 = -16;
        else {
          if (ent->client->waterstate == 2)
            f1 = 24;
          else
          if (ent->waterlevel && ent->waterlevel < 3) {
            f1 = ((v[0] == ent->s.origin[0] && v[1] == ent->s.origin[1])?-300:(-(JumpMax+64))); }
          else
            f1 = -(JumpMax+64); }
        yaw = (Route[ent->client->pers.routeindex].state == 1)?-48:12;
        if (v[0] <= (ent->absmax[0]-yaw) && v[0] >= (ent->absmin[0]+yaw)) {
          if (v[1] <= (ent->absmax[1]-yaw) && v[1] >= (ent->absmin[1]+yaw)) {
            if ((v[2] <= (ent->absmax[2]-f1) && v[2] >= (ent->absmin[2]+f2))
              || Route[ent->client->pers.routeindex].state == 1) {
              if (ent->client->pers.routeindex < TotalRouteNodes) {
                if (Route[ent->client->pers.routeindex].state <= 3) {
                  if (ent->client->havetarget) {
                    for (i = 0;i <(6);i++) {
                      if (!(k=Route[ent->client->pers.routeindex].linkpod[i])) break;
                      if (k > ent->client->pers.routeindex && k < ent->client->enemy_routeindex) {
                        ent->client->pers.routeindex = k;
                        break; } } }
                  else
                  if (random() < 0.2) {
                    for (i = 0;i <(6);i++) {
                      if (!(k=Route[ent->client->pers.routeindex].linkpod[i])) break;
                      if (k > ent->client->pers.routeindex && k < ent->client->enemy_routeindex) {
                        if (random() < 0.5) {
                          ent->client->pers.routeindex = k;
                          break; } } } } }
                ent->client->pers.routeindex++;
                if (!(ent->client->pers.routeindex < TotalRouteNodes))
                 ent->client->pers.routeindex = 0; } } } }

        if (ent->client->pers.routeindex < TotalRouteNodes && trace_priority) {
          if (1) {
            Get_RouteOrigin(ent->client->pers.routeindex,v);
            VectorSubtract(v,ent->s.origin,temppos);
            if (trace_priority < 2)
              ent->s.angles[PITCH] = Get_pitch(temppos);
            k = 0;
            if (ent->groundentity  || ent->waterlevel) {
              yaw = temppos[2];
              temppos[2] = 0;
              x = VectorLength(temppos);
              if (1) {
                k = 0;
                if (trace_priority < 3)
                  ent->client->moveyaw = Get_yaw(temppos);
                if ((ent->groundentity || ent->waterlevel) && trace_priority < 2) {
                  ent->s.angles[YAW] = ent->client->moveyaw;
                  k = 1; }
                if (x < dist && fabs(yaw) < 20 && k) {
                  iyaw = Get_yaw(temppos);
                  i = Bot_TestMove(ent,iyaw,temppos,x,&bottom);
                  tr = gi.trace(v,ent->mins,ent->maxs,v,ent,MASK_BOTSOLIDX);
                  if (Route[ent->client->pers.routeindex].state == 3 && !i) {
                    if (x < 30) ent->client->pers.routeindex++; }
                  else
                  if ((Route[ent->client->pers.routeindex].state == 3
                    || Route[ent->client->pers.routeindex].state == 0)
                    && !tr.allsolid && !tr.startsolid
                    && HazardCheck(ent,v)
                    && fabs(bottom) < 20 && i && !ent->waterlevel) {
                    if ((v[2] < ent->s.origin[2] && bottom < 0) || (v[2] >= ent->s.origin[2] && bottom >= 0)) {
                      VectorCopy(temppos,ent->s.origin);
                      VectorCopy(v,trmin);
                      dist -= x;
                      if (Route[ent->client->pers.routeindex].state <= 3) {
                        if (ent->client->havetarget) {
                          for (i = 0;i <(6);i++) {
                            if (!(j=Route[ent->client->pers.routeindex].linkpod[i])) break;
                            if (j > ent->client->pers.routeindex && j < ent->client->enemy_routeindex) {
                              ent->client->pers.routeindex = j;
                              break; } } } }
                      ent->client->pers.routeindex++;
                      if (i == 2)
                        ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
                      Get_RouteOrigin(ent->client->pers.routeindex,v);
                      VectorSubtract(v,ent->s.origin,temppos);
                      if (trace_priority < 2)
                        ent->s.angles[PITCH] = Get_pitch(temppos);
                      if (trace_priority < 3)
                        ent->client->moveyaw = Get_yaw(temppos);
                      if (k && trace_priority < 2)
                        ent->s.angles[YAW] = ent->client->moveyaw; } }
                  else
                  if ((Route[ent->client->pers.routeindex].state == 3
                    || Route[ent->client->pers.routeindex].state == 0)
                    && fabs(bottom) < 20 && ent->waterlevel) {
                    if ((v[2] < ent->s.origin[2] && bottom < 0) || (v[2] >= ent->s.origin[2] && bottom >= 0)) {
                      VectorCopy(temppos,ent->s.origin);
                      VectorCopy(v,trmin);
                      dist -= x;
                      ent->client->pers.routeindex++;
                      Get_RouteOrigin(ent->client->pers.routeindex,v);
                      VectorSubtract(v,ent->s.origin,temppos);
                      if (trace_priority < 2)
                        ent->s.angles[PITCH] = Get_pitch(temppos);
                      if (trace_priority < 3)
                        ent->client->moveyaw = Get_yaw(temppos);
                      if (k && trace_priority < 2)
                        ent->s.angles[YAW] = ent->client->moveyaw; }
                    else dist = x; }
                  else dist = x; }
                else
                  if (x < dist) dist = x; }

              k = 0;
              if ((ent->client->pers.routeindex-1) >= 0 &&
                (Route[ent->client->pers.routeindex].state == 4
                || Route[ent->client->pers.routeindex].state == 5)) {
                Get_RouteOrigin(ent->client->pers.routeindex-1,v);
                if (fabs(v[2]-ent->s.origin[2]) <= JumpMax) {
                  if (ent->client->waterstate < 2 && Route[ent->client->pers.routeindex].ent->nextthink > level.time)
                    k = 1; } }
              if (k && !(ent->client->movestate & (0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800))) {
                if ((ent->client->pers.routeindex+1) < TotalRouteNodes) {
                  Get_RouteOrigin(ent->client->pers.routeindex+1,v);
                  if ((v[2]-ent->s.origin[2]) > JumpMax) {
                    if ((Route[ent->client->pers.routeindex].ent->union_ent->s.origin[2] - ent->s.origin[2]) > JumpMax) {
                      ent->client->waiting_obj = Route[ent->client->pers.routeindex].ent;
                      ent->client->movestate |= 0x00000400;
                      k = 0;
                      for (i = 1;i <=3;i++) {
                        if (ent->client->pers.routeindex-i >= 0) {
                          Get_RouteOrigin(ent->client->pers.routeindex-i,v);
                          if (ent->client->waiting_obj->absmax[0] < (v[0]+ent->mins[0])) k = 1;
                          else if (ent->client->waiting_obj->absmax[1] < (v[1]+ent->mins[1])) k = 1;
                          else if (ent->client->waiting_obj->absmin[0] > (v[0]+ent->maxs[0])) k = 1;
                          else if (ent->client->waiting_obj->absmin[1] > (v[1]+ent->maxs[1])) k = 1;
                          if (k) break; } }
                      if (k)
                        VectorCopy(v,ent->client->movtarget_pt);
                      else
                        Get_RouteOrigin(ent->client->pers.routeindex-1,ent->client->movtarget_pt);
                      goto VCHCANSEL; } } } } } } }
        else
        if (ent->client->pers.routeindex >= TotalRouteNodes) {
          ent->client->pers.routeindex = 0;
          ent->client->routetrace = false; } }
      else {
        ent->client->pers.routeindex = 0;
        ent->client->routetrace = false; } }

  if (!(ent->client->movestate & 0x00000020) && (!ent->groundentity || ent->groundentity != ent->client->waiting_obj))
    if (!(ent->client->waiting_obj && ent->client->waiting_obj->use == door_use)) {
      ent->client->movestate &= ~(0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800);
      ent->client->waiting_obj = NULL; }

  if (ent->groundentity && !(ent->client->movestate & (0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800))) {
    it_ent = ent->groundentity;
    if (it_ent->classname[0] == 'f') {
      if (it_ent->use == Use_Plat) {
        if (it_ent->pos1[2] > it_ent->pos2[2]
          && ((it_ent->moveinfo.state == 2 && it_ent->velocity[2] > 0) || it_ent->moveinfo.state == 1)) {
          ent->client->waiting_obj = it_ent;
          ent->client->movestate |= 0x00000040;
          if (ent->client->routetrace) {
            if (Route[ent->client->pers.routeindex].ent == ent->client->waiting_obj)
              if (Route[ent->client->pers.routeindex].state == 4) {
                if (ent->client->waiting_obj->union_ent->s.origin[2] >(ent->s.origin[2]+32)) {
                  ent->client->movestate &= ~0x00000040;
                  ent->client->movestate |= 0x00000400; }
                else
                  ent->client->pers.routeindex++; } } } }
      else
      if (it_ent->use == train_use
        && it_ent->nextthink >= level.time
        && ((it_ent->s.origin[2]-it_ent->s.old_origin[2]) > 0 || ent->client->routetrace)) {
        if (ent->client->routetrace && ent->client->pers.routeindex > 0) {
          j = 0;
          k = ent->client->pers.routeindex-1;
          for (i=0;i<3;i++) {
            if ((k+i) < TotalRouteNodes) {
              if (Route[k+i].state == 5) {
                if (Route[k+i].ent == it_ent)
                  j = 1;
                else
                if (it_ent->trainteam) {
                  e = it_ent->trainteam;
                  while (1) {
                    if (e == it_ent) break;
                    if (e == Route[k+i].ent) {
                      j = 1;
                      it_ent = e;
                      Route[k+i].ent = e;
                      break; }
                    e = e->trainteam; } }
                else
                if (it_ent->target_ent) {
                  if (VectorCompare(Route[k+i].Tcourner,it_ent->target_ent->s.origin)) {
                    j = 1;
                    break; } }
                if (j) break; } }
            else break; }
          if (j) {
            ent->client->movestate |= 0x00000200;
            ent->client->waiting_obj = it_ent;
            ent->client->pers.routeindex = k+i+1; } }
        else {
          if ((it_ent->s.origin[2]-it_ent->s.old_origin[2]) > 0) {
            ent->client->movestate |= 0x00000200;
            ent->client->waiting_obj = it_ent; }
          else
          if ((it_ent->s.origin[2]-it_ent->s.old_origin[2]) > -2
            && trace_priority) {
            ent->client->movestate |= 0x00000200;
            ent->client->waiting_obj = it_ent; }
          else
            ent->client->movestate |= 0x00000010; } } } }

  if ((ent->client->movestate & 0x00000010) && ent->groundentity) {
    if (ent->client->movestate & 0x00000040) {
      if (ent->groundentity->use == Use_Plat) {
        ent->client->movestate &= ~(0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800);
        ent->client->waiting_obj = NULL; } }
    else
    if (ent->client->movestate & 0x00000200) {
      if (ent->groundentity->use == train_use) {
        ent->client->movestate &= ~(0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800);
        ent->client->waiting_obj = NULL; } }
    else
    if (ent->client->movestate & (0x00000080|0x00000100)) {
      if (ent->groundentity->use == door_use) {
        ent->client->movestate &= ~(0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800);
        ent->client->waiting_obj = NULL; } }
    else {
      ent->client->movestate &= ~(0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800);
      ent->client->waiting_obj = NULL; } }
  else
  if ((ent->client->movestate & (0x00000040|0x00000400|0x00000080|0x00000100))
   && !(ent->client->movestate & 0x00000010)) {
    k = 0;
    if (ent->client->movestate & (0x00000080|0x00000100)) {
      if (ent->client->movestate & 0x00000080) {
        if (ent->client->waiting_obj->moveinfo.state & 2|1)
          k = 1; }
      else {
        if (ent->client->waiting_obj->moveinfo.state & 0|3)
          k = 1; } }
    else
    if (ent->client->movestate & 0x00000400) {
      if (Route[ent->client->pers.routeindex].state == 5) {
        if (!TraceX(ent,Route[ent->client->pers.routeindex].ent->union_ent->s.origin))
          k = 1;
        if ((Route[ent->client->pers.routeindex].ent->union_ent->s.origin[2]+8-ent->s.origin[2]) > JumpMax)
          k = 1; }
      else {
        if ((ent->client->waiting_obj->union_ent->s.origin[2] - ent->s.origin[2]) > JumpMax)
          k = 1; }
      if (ent->client->pers.routeindex-1 > 0 && ent->client->waterstate < 2) {
        Get_RouteOrigin(ent->client->pers.routeindex -1 ,trmin);
        if ((trmin[2]-ent->s.origin[2]) > JumpMax && (v[2]-ent->s.origin[2]) > JumpMax)
          k = 0; } }
    else {
      if (ent->client->waiting_obj->moveinfo.state & 2|1)
        k = 1;
      if (ent->client->waiting_obj->moveinfo.state == 1)
        plat_go_up(ent->client->waiting_obj);
      if (ent->client->routetrace) {
        Get_RouteOrigin(ent->client->pers.routeindex,v);
        if (ent->s.origin[2] > v[2])
          k = 2; } }

    if (k != 1) {
      if (k == 2)
        ent->client->movestate |= 0x00000010;
      else {
        ent->client->movestate &= ~(0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800);
        ent->client->waiting_obj = NULL; } }
    else {
      if (ent->client->movestate & 0x00000400) {
        k = 0;
        if (ent->client->pers.routeindex-1 > 0) {
          VectorCopy(ent->client->movtarget_pt,trmax);
          trmax[2] = 0;
          k = 1; }
        if (!k)
          goto VCHCANSEL; }
      else {
        trmax[0] = (ent->client->waiting_obj->absmin[0]+ent->client->waiting_obj->absmax[0])*0.5;
        trmax[1] = (ent->client->waiting_obj->absmin[1]+ent->client->waiting_obj->absmax[1])*0.5;
        trmax[2] = 0; }
      VectorSubtract(trmax,ent->s.origin,temppos);
      yaw = temppos[2];
      temppos[2] = 0;
      x = VectorLength(temppos);
      if (x == 0)
        goto VCHCANSEL;
      if (x < dist)
        dist = x;
      if (trace_priority < 3)
        ent->client->moveyaw = Get_yaw(temppos); } }

  else
  if (ent->client->movestate & 0x00000200) {
    i = 0;
    if (ent->client->routetrace) {
      Get_RouteOrigin(ent->client->pers.routeindex,v);
      if ((ent->client->pers.routeindex-1) >= 0) {
        if (Route[ent->client->pers.routeindex-1].state != 5)
          i = 1; }
      else
        i = 1;
      if (TraceX(ent,v)) {
        if ((v[2]-ent->s.origin[2]) <= JumpMax)
          i = 1;
        else
          ent->client->routelocktime = level.time+1.5; }
      else
        ent->client->routelocktime = level.time+1.5; }
    else
    if (j ||(ent->client->waiting_obj->s.origin[2]-ent->client->waiting_obj->s.old_origin[2]) <= 0) {
      ent->client->movestate |= 0x00000010;
      ent->client->movestate &= ~(0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800); }
    else {
      k = 0;
      if (ent->client->routetrace) {
        tr = gi.trace(ent->s.origin,NULL,NULL,v,ent,MASK_BOTSOLIDX);
        if (tr.ent == ent->client->waiting_obj) {
          tr = gi.trace(v,NULL,NULL,ent->s.origin,ent,MASK_BOTSOLIDX);
          if (tr.ent == ent->client->waiting_obj) {
            VectorSubtract(v,ent->s.origin,temppos);
            k = 1; } } }
      if (!k) {
        VectorCopy(ent->client->waiting_obj->union_ent->s.origin,trmax);
        trmax[2] += 8;
        VectorSubtract(trmax,ent->s.origin,temppos);
        yaw = temppos[2];
        temppos[2] = 0;
        x = VectorLength(temppos);
        if (x < dist)
          dist = x; }
      if (trace_priority < 3)
        ent->client->moveyaw = Get_yaw(temppos); }
    goto GOMOVE; }

  else
  if (ent->client->movestate & 0x00000020) {
    if (!trace_priority || ent->client->waiting_obj->moveinfo.state == 0) {
      ent->client->movestate &= ~(0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800);
      ent->client->waiting_obj = NULL; }
    else
    if (ent->client->waiting_obj->moveinfo.state & 1|2) {
      VectorSubtract(ent->client->movtarget_pt,ent->s.origin,temppos);
      temppos[2] = 0;
      dist *= 0.25;
      if (VectorLength(temppos) < 10 || VectorCompare(ent->s.origin,ent->client->movtarget_pt)) {
        if (!ent->client->waiting_obj->union_ent) {
          trmin[0] = (ent->client->waiting_obj->absmin[0]+ent->client->waiting_obj->absmax[0])*0.5;
          trmin[1] = (ent->client->waiting_obj->absmin[1]+ent->client->waiting_obj->absmax[1])*0.5;
          trmin[2] = (ent->client->waiting_obj->absmin[2]+ent->client->waiting_obj->absmax[2])*0.5; }
        else
          VectorCopy(ent->client->waiting_obj->union_ent->s.origin,trmin);
        trmin[2] += 8;
        VectorSubtract(trmin,ent->s.origin,temppos);
        if (trace_priority < 3)
          ent->client->moveyaw = Get_yaw(temppos);
        if (trace_priority < 2) {
          ent->s.angles[YAW] = ent->client->moveyaw;
          ent->s.angles[PITCH] = Get_pitch(temppos); }
        goto VCHCANSEL; }
      else {
        if (trace_priority < 3)
          ent->client->moveyaw = Get_yaw(temppos);
        if (!ent->client->waiting_obj->union_ent) {
          trmin[0] = (ent->client->waiting_obj->absmin[0]+ent->client->waiting_obj->absmax[0])*0.5;
          trmin[1] = (ent->client->waiting_obj->absmin[1]+ent->client->waiting_obj->absmax[1])*0.5;
          trmin[2] = (ent->client->waiting_obj->absmin[2]+ent->client->waiting_obj->absmax[2])*0.5; }
        else
          VectorCopy(ent->client->waiting_obj->union_ent->s.origin,trmin);
        trmin[2] += 8;
        VectorSubtract(trmin,ent->s.origin,temppos);
        if (trace_priority < 2) {
          ent->s.angles[YAW] = Get_yaw(temppos);
          ent->s.angles[PITCH] = Get_pitch(temppos); } } } }


GOMOVE:

  if (!ent->groundentity && !ent->waterlevel) {
    if (ent->velocity[2] > 300 && !(ent->client->movestate & (0x00000008|0x00000002|0x00000004)))
      ent->velocity[2] = 300;
    k = (ent->client->ps.pmove.pm_flags & PMF_DUCKED)?1:0;
    for (x = 0; x < 90; x += 10) {
      dist = 30*ent->moveinfo.speed;
      yaw = ent->client->moveyaw+x;
      if (yaw > 180) yaw -= 360;
      if (Bot_TestMove(ent,yaw,temppos,dist,&bottom)) {
        if (bottom <= 24 && bottom > 0)
          if (ent->velocity[2] <= 10) {
            VectorCopy(temppos,ent->s.origin);
            break; }
        if (!ent->waterlevel && ent->s.origin[2] > ent->s.old_origin[2]
          && ent->client->routetrace
          && !(ent->client->movestate & 0x00000001|(0x00000008|0x00000002|0x00000004))
          && (ent->client->pers.routeindex+1) < TotalRouteNodes
          && ent->velocity[2] >= 100
          && ent->velocity[2] < (100+ent->gravity*sv_gravity->value*0.1)) {
          Get_RouteOrigin(ent->client->pers.routeindex,v);
          Get_RouteOrigin(ent->client->pers.routeindex+1,vv);
          k = 0;
          j = Bot_TestMove(ent,yaw,trmin,16,&f1);
          VectorSubtract(v,ent->s.origin,trmin);
          if ((vv[2]-v[2]) > JumpMax)
            k = 1;
          else
          if ((v[2]-ent->s.origin[2]) > JumpMax)
            k = 2;
          else
          if (!TargetJump_Chk(ent,vv,0) && VectorLength(trmin) < 64) {
            if (TargetJump_Chk(ent,vv,ent->velocity[2]))
              k = 1; }
          if (!j)
            k = 0;
          else
            if (f1 > 10 && f1 < -10)
              k = 0;
          if (k) {
            if (k == 2)
              VectorCopy(v,vv);
            if (TargetJump(ent,vv)) {
              VectorSubtract(vv,ent->s.origin,v);
              ent->client->moveyaw = Get_yaw(v);
              if (ent->velocity[2] > 300)
                ent->client->movestate |= 0x00000008;
              if (k == 1)
                ent->client->pers.routeindex++;
              break; } } }
        if (bottom <= 0) {
          VectorCopy(temppos,ent->s.origin);
          if (i == 2)
            ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
          else
            ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
          break; }
        else
          ent->moveinfo.speed = 0.3; }
      else {
        ent->moveinfo.speed = 0.3; }
      if (x == 0) continue;
      yaw = ent->client->moveyaw-x;
      if (yaw < -180) yaw += 360;
      if (Bot_TestMove(ent,yaw,temppos,dist,&bottom)) {
        if (bottom <= 24 && bottom >0  && ent->velocity[2] <= 10) {
          VectorCopy(temppos,ent->s.origin);
          break; }
        if (!ent->waterlevel && ent->s.origin[2] > ent->s.old_origin[2]
          && ent->client->routetrace
          && !(ent->client->movestate & 0x00000001|(0x00000008|0x00000002|0x00000004))
          && (ent->client->pers.routeindex+1) < TotalRouteNodes
          && ent->velocity[2] >= 100
          && ent->velocity[2] <(100+ent->gravity*sv_gravity->value*0.1)) {
          Get_RouteOrigin(ent->client->pers.routeindex ,v);
          Get_RouteOrigin(ent->client->pers.routeindex+1,vv);
          k = 0;
          j = Bot_TestMove(ent,yaw,trmin,16,&f1);
          VectorSubtract(v,ent->s.origin,trmin);
          if ((vv[2]-v[2]) > JumpMax)
            k = 1;
          else
            if ((v[2]-ent->s.origin[2]) > JumpMax)
              k = 2;
          else
            if (!TargetJump_Chk(ent,vv,0) && VectorLength(trmin) < 64) {
              if (TargetJump_Chk(ent,vv,ent->velocity[2]))
                k = 1; }
          if (!j)
            k = 0;
          else
            if (f1 > 10 && f1 < -10)
              k = 0;
          if (k) {
            if (k == 2)
              VectorCopy(v,vv);
            if (TargetJump(ent,vv)) {
              VectorSubtract(vv,ent->s.origin,v);
              ent->client->moveyaw = Get_yaw(v);
              if (ent->velocity[2] > 300)
                ent->client->movestate |= 0x00000008;
              if (k == 1)
                ent->client->pers.routeindex++;
              break; } } }
        if (bottom <= 0) {
          VectorCopy(temppos,ent->s.origin);
          if (i == 2)
            ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
          else
            ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
          break; }
        else
          ent->moveinfo.speed = 0.3; }
      else
        ent->moveinfo.speed = 0.3; }
    if (x >= 90) {
      if (trace_priority < 2)
        ent->s.angles[YAW] += ((random()-0.5)*360);
      if (ent->s.angles[YAW]>180)
        ent->s.angles[YAW] -= 360;
      else
      if (ent->s.angles[YAW]< -180)
        ent->s.angles[YAW] += 360; }
    goto VCHCANSEL; }

  waterjumped = false;
  if (ent->groundentity || ent->waterlevel) {
    if (ent->groundentity && ent->waterlevel <= 0)
      k = 1;
    else
    if (ent->waterlevel) {
      k = 2;
      if (ent->client->routetrace) {
        Get_RouteOrigin(ent->client->pers.routeindex,v);
        VectorSubtract(v,ent->s.origin,vv);
        vv[2] = 0;
        if (v[2] < ent->s.origin[2] && VectorLength(vv) < 24)
          k = 0; }
      if (ent->waterlevel == 3)
        k = 0; }
    else
      if (ent->waterlevel)
        k = 0;
    else
      k = 1;
    if (k)
      if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
        k = 0;
    f1 = (ent->client->waterstate)?-8192:-JumpMax;
    if (ent->client->nextcheck < (level.time+1.0)) {
      VectorSubtract(ent->client->my_old_origin,ent->s.origin,temppos);
      if (VectorLength(temppos) < 64) {
        if (ent->client->routetrace) {
          ent->client->routetrace = false;
          ent->client->pers.routeindex++; }
        else
          f1 = -300; }
      if (ent->client->nextcheck < level.time) {
        VectorCopy(ent->s.origin,ent->client->my_old_origin);
        ent->client->nextcheck = level.time+4.0; } }
    f3 = 20;
    if (ent->client->routetrace)
      Get_RouteOrigin(ent->client->pers.routeindex,v);
    if (ent->waterlevel && ent->client->routetrace) {
      if (v[2]+20 <= ent->s.origin[2]) {
        f2 = 20; f3 = 0; }
      else
        f2 = JumpMax; }
    else
      f2 = JumpMax;

    ladderdrop = true;
    for (x = 0; x <= 180 && dist != 0; x += 10) {
      yaw = ent->client->moveyaw+x;
      if (yaw > 180) yaw -= 360;
      if (j = Bot_TestMove(ent,yaw,temppos,dist,&bottom)) {
        if (x == 0 && !ent->waterlevel && !(ent->client->ps.pmove.pm_flags & PMF_DUCKED)) {
          if (ent->client->routetrace) {
            if ((v[2] -(ent->s.origin[2]+bottom)) > f2 || (bottom > 20 && v[2] > ent->s.origin[2])) {
              ladderdrop = false;
              if (Bot_Fall(ent,temppos,dist) && !ent->client->waterstate) {
                ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
                break; }
              if ((v[2]-ent->s.origin[2]) <= JumpMax) {
                if (Route[ent->client->pers.routeindex].state == 5 && ent->client->waterstate < 2) break;
                if (ent->client->pers.routeindex > 0)
                  if (Route[ent->client->pers.routeindex-1].state == 5
                    && Route[ent->client->pers.routeindex-1].ent == ent->groundentity) break; } }
            else
            if (ent->groundentity) {
              if (ent->groundentity->use == rotating_use) {
                if (Bot_Fall(ent,temppos,dist)) {
                  ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
                  break; } }
              else
              if (Route[ent->client->pers.routeindex].state == 1) {
                if (!TraceX(ent,v)) break;
                if (!HazardCheck(ent,v)) break;
                if (!BankCheck(ent,v)) break;
                if (Bot_Fall(ent,temppos,dist)) {
                  ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
                  break; } } } } }
        if (bottom > 20 && bottom <= f2 && j == 1 && k && !(ent->client->ps.pmove.pm_flags & PMF_DUCKED)) {
          ent->moveinfo.speed = 0.15;
          if (k == 1)
            ent->velocity[2] = 300;
          else {
            ent->moveinfo.speed = 0.1;
            if (ent->velocity[2] < 300 || VectorCompare(ent->s.origin,ent->s.old_origin)) {
              ent->velocity[2] = 300;
              ent->client->movestate |= 0x00000004; }
            goto VCHCANSEL; }
          SetBotAnim(ent);
          ent->client->moveyaw = yaw;
          ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
          break; }
        else
        if (bottom <= f3 && (bottom >= f1 || ent->waterlevel)) {
          if (bottom < 0 && !ent->client->waterstate) {
            f2 = 0.1*(ent->velocity[2]-ent->gravity*sv_gravity->value*0.1);
            if (bottom >= f2 && ent->velocity[2] < 0)
              temppos[2] += bottom;
            else
              temppos[2] += f2; }
          VectorCopy(temppos,ent->s.origin);
          if (f1 > -52)
            ent->moveinfo.speed = 0.25;
          if (j != 1)
            ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
          else
            ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
          if (x > 30 || !ent->client->routetrace) {
            f2 = ent->client->moveyaw;
            ent->client->moveyaw = yaw;
            if (f2 == ent->s.angles[YAW] && trace_priority < 2)
              ent->s.angles[YAW] = yaw; }
          break; }
        else
        if (bottom < f1 && !ent->client->waterstate && x <= 30) {
          if (ladderdrop && bottom != -9999)
            if (ent->client->ground_contents & CONTENTS_LADDER) {
              VectorCopy(temppos,ent->s.origin);
              ent->client->moveyaw = yaw;
              ent->moveinfo.speed = 0.2;
              goto VCHCANSEL; }
          if (ladderdrop &&  bottom < 0)
            if (!ent->client->waterstate) {
              if (Bot_moveW(ent,yaw,temppos,dist,&bottom)) {
                iyaw = -41;
                if (bottom > -20 && iyaw < -40) {
                  VectorCopy(temppos,ent->s.origin);
                  break; } } }
          if (Bot_Fall(ent,temppos,dist))
            break; } }

      if (x == 0 && (ent->client->battlemode & (0x00020000|0x00040000)))
        ent->client->battlemode &= ~(0x00020000|0x00040000);

      if (x == 0 || x == 180) continue;

      yaw = ent->client->moveyaw-x;
      if (yaw < -180) yaw += 360;
      if (j = Bot_TestMove(ent,yaw,temppos,dist,&bottom)) {
        f2 = (ent->client->waterstate == 1)?100:JumpMax;
        if (bottom > 20 && bottom <= f2 && j == 1 && k && !(ent->client->ps.pmove.pm_flags & PMF_DUCKED)) {
          ent->moveinfo.speed = 0.15;
          if (k == 1)
            ent->velocity[2] = 300;
          else {
            ent->moveinfo.speed = 0.1;
            if (ent->velocity[2] < 300 || VectorCompare(ent->s.origin,ent->s.old_origin)) {
              ent->velocity[2] = 300;
              ent->client->movestate |= 0x00000004; }
            goto VCHCANSEL; }
          SetBotAnim(ent);
          ent->client->moveyaw = yaw;
          ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
          break; }
        else
        if (bottom <= f3 && (bottom >= f1 || ent->waterlevel)) {
          if (bottom < 0 && !ent->client->waterstate) {
            f2 = 0.1*(ent->velocity[2]-ent->gravity*sv_gravity->value*0.1);
            if (bottom >= f2 && ent->velocity[2] < 0)
              temppos[2] += bottom;
            else
              temppos[2] += f2; }
          VectorCopy(temppos,ent->s.origin);
          if (f1 > -52)
            ent->moveinfo.speed = 0.25;
          if (j != 1)
            ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
          else
            ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;
          if (x > 30 || !ent->client->routetrace) {
            f2 = ent->client->moveyaw;
            ent->client->moveyaw = yaw;
            if (f2 == ent->s.angles[YAW] && trace_priority < 2)
              ent->s.angles[YAW] = yaw; }
          break; }
        else
        if (bottom < f1 && !ent->client->waterstate && x <= 30) {
          if (ladderdrop && ent->client->ground_contents & CONTENTS_LADDER && bottom != -9999) {
            VectorCopy(temppos,ent->s.origin);
            ent->client->moveyaw = yaw;
            ent->moveinfo.speed = 0.2;
            goto VCHCANSEL; }
          if (ladderdrop && bottom < 0 && !ent->client->waterstate) {
            if (Bot_moveW(ent,yaw,temppos,dist,&bottom)) {
              iyaw = -41;
              if (bottom > -54 && iyaw < -40) {
                VectorCopy(temppos,ent->s.origin);
                break; } } }
          if (Bot_Fall(ent,temppos,dist))
            break; } } }

    if (!ent->client->routetrace && !ent->client->current_enemy)
      if (trace_priority < 2)
        ent->s.angles[YAW] = yaw;

    if (ent->waterlevel && !waterjumped) {
      k = 0;
      VectorCopy(ent->s.origin,temppos);
      if (ent->client->routetrace) {
        Get_RouteOrigin(ent->client->pers.routeindex,v);
        k = 2;
        x = v[2]-ent->s.origin[2];
        if (x > 13) x = 13;
        else if (x < -13) x = -13;
        if (x < 0) {
          if (Bot_Watermove(ent,temppos,dist,x)) {
            VectorCopy(temppos,ent->s.origin);
            k = 1; } }
        else
        if (x > 0 && ent->client->waterstate == 2)
          if (!(ent->client->ps.pmove.pm_flags & PMF_DUCKED)) {
            if (ent->velocity[2] < -10)
              ent->velocity[2] = 0;
            if (Bot_Watermove(ent,temppos,dist,x)) {
              VectorCopy(temppos,ent->s.origin);
              k = 1; } } }
      else
      if (ent->air_finished-2.0 < level.time)
        if (ent->client->waterstate == 2) {
          if (Bot_Watermove(ent,temppos,dist,13)) {
            VectorCopy(temppos,ent->s.origin);
            k = 1; }
          else
            k = 2; }
      if (k == 1)
        Get_WaterState(ent);
      if (ent->client->routetrace)
        if (v[2] == ent->s.origin[2])
          k = 3;
      if ((!ent->groundentity && !ent->client->waterstate && k && ent->velocity[2] < 1)
        ||(ent->client->waterstate == 2 && (ent->client->ps.pmove.pm_flags & PMF_DUCKED))) {
        if (Bot_Watermove(ent,temppos,dist,-7) && k != 3)
          VectorCopy(temppos,ent->s.origin); }
      if (ent->client->waterstate == 2)
        ent->moveinfo.decel = level.time;
      else
      if (!k) {
        if ((level.time-ent->moveinfo.decel) > 4.0)
          if (!ent->client->routetrace) {
            ent->velocity[2] = -200;
            ent->moveinfo.decel = level.time; } }
      if (ent->groundentity && ent->waterlevel == 1) {
        VectorSubtract(ent->s.origin,ent->s.old_origin,temppos);
        if (!temppos[0] && !temppos[1] && !temppos[2])
          ent->velocity[2] += 80; } }
    else
    if (ent->client->routetrace && !dist) {
      Get_RouteOrigin(ent->client->pers.routeindex,v);
      if (v[2] <(ent->s.origin[2]-20))
        if (Bot_Watermove(ent,temppos,dist,-20))
          VectorCopy(temppos,ent->s.origin); } }

  if (!ent->client->routetrace && trace_priority && random() < 0.2) {
    VectorCopy(ent->s.origin,v);
    VectorCopy(ent->mins,touchmin);
    touchmin[2] += 16;
    VectorCopy(ent->maxs,touchmax);
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
      touchmax[2] = 0;
    else
      v[2] += 20;
    if (random() < 0.5) {
      f1 = ent->client->moveyaw+90;
      if (f1 > 180) iyaw -= 360;
      f2 = ent->client->moveyaw+135;
      if (f2 > 180) iyaw -= 360; }
    else {
      f1 = ent->client->moveyaw-90;
      if (f1 < 180) iyaw += 360;
      f2 = ent->client->moveyaw-135;
      if (f2 < 180) iyaw += 360; }
    yaw = DEG2RAD(f1);
    trmin[0] = cos(yaw)*128;
    trmin[1] = sin(yaw)*128;
    trmin[2] = 0;
    VectorAdd(v,trmin,trmax);
    tr = gi.trace(v, NULL, NULL, trmax, ent, MASK_BOTSOLIDX);
    x = tr.fraction;

    yaw = DEG2RAD(f2);
    trmin[0] = cos(yaw)*128;
    trmin[1] = sin(yaw)*128;
    trmin[2] = 0;
    VectorAdd(v,trmin,trmax);
    tr = gi.trace(v, NULL, NULL, trmax, ent, MASK_BOTSOLIDX);
    if (x > tr.fraction && x > 0.5)
      ent->client->moveyaw = f1; }

  it_ent = NULL;
  k = 0;
  VectorCopy(ent->absmin, touchmin);
  VectorCopy(ent->absmax, touchmax);
  touchmin[0] -= 48;
  touchmin[1] -= 48;
  touchmin[2] -= 5;
  touchmax[0] += 48;
  touchmax[1] += 48;
  if (i=gi.BoxEdicts(touchmin ,touchmax,touch,1024,1)) {
    for (j=i-1;j>=0;j--) {
      trent = touch[j];
      if (trent->classname) {
        if (trent->use == button_use) {
          k = 1;
          it_ent = trent;
          break; }
        else
        if (trent->use == door_use || trent->use == rotating_use) {
          if (!trent->targetname && !trent->takedamage)
            if (ent->groundentity != trent) {
              k = 2;
              it_ent = trent;
              break; } } } } }

  if (it_ent && k == 1) {
    if (it_ent->use && it_ent->moveinfo.state == 1 && !it_ent->health) {
      k = 0;
      if (ent->client->routetrace && ent->client->pers.routeindex-1 > 0) {
        k = 1;
        i = ent->client->pers.routeindex;
        if (Route[i].state == 7)
          k = 0;
        else
          if (Route[--i].state == 7)
            k = 0;
        if (!k && Route[i].ent == it_ent)
          ent->client->pers.routeindex = i+1;
        else
          k = 1; }
      if (!k && it_ent->target) {
        str = it_ent->target;
        e = &g_edicts[(int)maxclients->value+1];
        for (i=maxclients->value+1; i<globals.num_edicts; i++, e++) {
          if (!e->inuse || !e->targetname) continue;
          if (!stricmp(str, e->targetname)) {
            if (e->classname[0] == 't') {
              if (e->use == trigger_relay_use) {
                if (e->target) {
                  str = e->target;
                  e = &g_edicts[(int)maxclients->value];
                  i=maxclients->value;
                  continue; } } }
            else
            if (e->classname[0] == 'f') {
              it_ent->use(it_ent,ent,it_ent);
              if (e->use == door_use || e->use == rotating_use) {
                k = 0;
                if (!ent->client->routetrace) {
                  v[0] = (it_ent->absmin[0]+it_ent->absmax[0])*0.5;
                  v[1] = (it_ent->absmin[1]+it_ent->absmax[1])*0.5;
                  v[2] = (it_ent->absmin[2]+it_ent->absmax[2])*0.5;
                  VectorSubtract(it_ent->union_ent->s.origin,v,temppos);
                  VectorScale(temppos, 3, v);
                  VectorAdd(ent->s.origin,v,ent->client->movtarget_pt); }
                else
                  VectorCopy(ent->s.origin,ent->client->movtarget_pt);
                if (fabs(e->moveinfo.start_origin[2]-e->moveinfo.end_origin[2]) > JumpMax) {
                  if (!e->union_ent) {
                    it = item_navi3;
                    trent = G_Spawn();
                    trent->classname = it->classname;
                    trent->s.origin[0] = (e->absmin[0]+e->absmax[0])*0.5;
                    trent->s.origin[1] = (e->absmin[1]+e->absmax[1])*0.5;
                    trent->s.origin[2] = e->absmax[2]+16;
                    trent->union_ent = e;
                    e->union_ent = trent;
                    SpawnItem3(trent, it); }
                  else {
                    trent = e->union_ent;
                    trent->solid = SOLID_TRIGGER;
                    trent->svflags &= ~SVF_NOCLIENT; }
                  trent->target_ent = ent;
                  if (e->spawnflags & 32) {
                    f1 = e->moveinfo.start_origin[2]-e->moveinfo.end_origin[2];
                    k = 1; }
                  else {
                    f1 = e->moveinfo.start_origin[2]-e->moveinfo.end_origin[2];
                    if (f1 > 0) {
                      if (e->moveinfo.state & 1|2) {
                        if (fabs(trent->s.origin[2]-ent->s.origin[2]) < JumpMax)
                          k = 1; } }
                    else {
                      if (e->moveinfo.state & 1|2) {
                        if (fabs(trent->s.origin[2]-ent->s.origin[2]) < JumpMax)
                          k = 1; } } } }
                if (!k) {
                  ent->client->waiting_obj = e;
                  ent->client->movestate &= ~(0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800);
                  ent->client->movestate |= 0x00000020; }
                else {
                  if ((e->union_ent->s.origin[2]+8-ent->s.origin[2]) > JumpMax) {
                    ent->client->routetrace = false;
                    ent->client->movestate &= ~(0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800); } }
                break; } } } } }
      else
        if (!k)
          it_ent->use(it_ent,ent,it_ent); } }
  else
  if (it_ent && k == 2) {
    if (it_ent->moveinfo.state == 1) {
      if (it_ent->flags & FL_TEAMSLAVE)
        it_ent->teammaster->use(it_ent->teammaster,ent,it_ent->teammaster);
      else
        it_ent->use(it_ent,ent,it_ent); }
    if (it_ent->moveinfo.state == 1) {
      VectorCopy(ent->s.origin,ent->client->movtarget_pt);
      ent->client->waiting_obj = it_ent;
      ent->client->movestate &= ~(0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800);
      ent->client->movestate |= 0x00000020;
      if (it_ent->flags & FL_TEAMSLAVE) {
        trmin[0] = (it_ent->teammaster->absmin[0]+it_ent->teammaster->absmax[0])*0.5;
        trmin[1] = (it_ent->teammaster->absmin[1]+it_ent->teammaster->absmax[1])*0.5;
        trmax[0] = (it_ent->absmin[0]+it_ent->absmax[0])*0.5;
        trmax[1] = (it_ent->absmin[1]+it_ent->absmax[1])*0.5;
        temppos[0] = (trmin[0]+trmax[0])*0.5;
        temppos[1] = (trmin[1]+trmax[1])*0.5;
        if (trace_priority < 2)
          ent->s.angles[YAW] = Get_yaw(temppos); }
      else {
        trmax[0] = (it_ent->absmin[0]+it_ent->absmax[0])*0.5;
        trmax[1] = (it_ent->absmin[1]+it_ent->absmax[1])*0.5;
        VectorSubtract(trmax,ent->s.origin,temppos);
        if (trace_priority < 2)
          ent->s.angles[YAW] = Get_yaw(temppos); } }
    else
    if (it_ent->moveinfo.state == 2) {
      VectorCopy(ent->s.origin,ent->client->movtarget_pt);
      ent->client->waiting_obj = it_ent;
      ent->client->movestate &= ~(0x00000010|0x00000020|0x00000400|0x00000040|0x00000080|0x00000100|0x00000200|0x00000800);
      ent->client->movestate |= 0x00000020; } }

VCHCANSEL:

  front = left = right = NULL;
  k = 0;
  if (ent->client->routetrace)
    if (ent->client->pers.routeindex+1 < TotalRouteNodes) {
      Get_RouteOrigin(ent->client->pers.routeindex+1,v);
      if (v[2]-ent->s.origin[2] >= 32)
        k = 1; }
  if (k && trace_priority && !(ent->client->ps.pmove.pm_flags & PMF_DUCKED)) {
    tempflag = 0;
    VectorCopy(ent->mins,trmin);
    VectorCopy(ent->maxs,trmax);
    trmin[2] += 20;
    iyaw = ent->client->moveyaw;
    yaw = DEG2RAD(iyaw);
    touchmin[0] = cos(yaw)*32;
    touchmin[1] = sin(yaw)*32;
    touchmin[2] = 0;
    VectorAdd(ent->s.origin,touchmin,touchmax);
    tr = gi.trace(ent->s.origin, trmin,ent->maxs, touchmax,ent, MASK_BOTSOLID);
    front = tr.ent;
    if (tr.contents & CONTENTS_LADDER)
      tempflag = 1;
    if (!tempflag && !ent->client->waterstate) {
      trmax[2] += 32;
      tr = gi.trace(ent->s.origin, trmin,trmax, touchmax,ent, MASK_BOTSOLID);
      if (tr.contents & CONTENTS_LADDER)
        tempflag = 2; }
    if (!tempflag && ent->groundentity) {
      Get_RouteOrigin(ent->client->pers.routeindex,v);
      v[2] = ent->s.origin[2];
      tr = gi.trace(ent->s.origin, trmin,ent->maxs, v,ent, MASK_BOTSOLID);
      if (tr.contents & CONTENTS_LADDER)
        tempflag = 3; }
    if (tempflag==0) {
      iyaw = ent->client->moveyaw+90;
      if (iyaw > 180) iyaw -= 360;
      yaw = DEG2RAD(iyaw);
      touchmin[0] = cos(yaw)*32;
      touchmin[1] = sin(yaw)*32;
      touchmin[2] = 0;
      VectorAdd(ent->s.origin,touchmin,touchmax);
      tr = gi.trace(ent->s.origin, trmin,ent->maxs, touchmax,ent,  MASK_BOTSOLID);
      right = tr.ent;
      if (tr.contents & CONTENTS_LADDER)
        tempflag = 1; }
    if (tempflag==0) {
      iyaw = ent->client->moveyaw-90;
      if (iyaw < -180) iyaw += 360;
      yaw = DEG2RAD(iyaw);
      touchmin[0] = cos(yaw)*32;
      touchmin[1] = sin(yaw)*32;
      touchmin[2] = 0;
      VectorAdd(ent->s.origin,touchmin,touchmax);
      tr = gi.trace(ent->s.origin, trmin,ent->maxs, touchmax,ent, MASK_BOTSOLID);
      left = tr.ent;
      if (tr.contents & CONTENTS_LADDER)
        tempflag = 1; }
    if (tempflag) {
      VectorCopy(tr.endpos,trmax);
      VectorCopy(trmax,touchmax);
      touchmax[2] += 8192;
      tr = gi.trace(trmax, trmin,ent->maxs, touchmax,ent, MASK_SOLID);
      e = tr.ent;
      k = 0;
      VectorCopy(tr.endpos,temppos);
      VectorAdd(tr.endpos,touchmin,touchmax);
      tr = gi.trace(temppos, trmin,ent->maxs, touchmax,ent, MASK_BOTSOLID);
      if (e && e->use == door_use) k = 1;
      if ((!(tr.contents & CONTENTS_LADDER) || k)) {
        ent->velocity[0] = 0;
        ent->velocity[1] = 0;
        if (ent->client->moveyaw == iyaw || ent->client->routetrace) {
          if (ent->client->moveyaw != iyaw)
            ent->client->moveyaw = iyaw;
          ent->s.angles[YAW] = ent->client->moveyaw;
          if (tempflag != 3)
            VectorCopy(trmax,ent->s.origin);
          ent->client->movestate |= 0x00000001;
          ent->s.angles[YAW] = ent->client->moveyaw;
          ent->s.angles[PITCH] = -29;
          if (tempflag == 2) {
            ent->velocity[2] = 300;
            SetBotAnim(ent);
            ent->client->movestate |= (0x00000008|0x00000002|0x00000004);
            ent->moveinfo.speed = 0; }
          else
          if (tempflag == 3) {
            ent->velocity[2] = 300;
            SetBotAnim(ent);
            ent->client->movestate |= (0x00000008|0x00000002|0x00000004);
            ent->moveinfo.speed = 30; }
          ent->velocity[2] = 200; }
        else {
          ent->client->moveyaw = iyaw;
          ent->s.angles[YAW] = ent->client->moveyaw; } } } }

VCHCANSEL_L:

  if (ent->client->battleduckcnt > 0)
    if (ent->groundentity)
      if (ent->velocity[2] < 10) {
        ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
        ent->client->battleduckcnt--; }

  if (ent->client->ps.pmove.pm_flags & PMF_DUCKED) {
    ent->client->duckedtime = 0;
    ent->maxs[2] = 4;
    ent->viewheight = -2; }
  else {
    if (ent->client->duckedtime < 1)
      ent->client->duckedtime += 0.1;
    ent->maxs[2] = 32;
    ent->viewheight = 22; }

  VectorCopy(ent->s.angles,ent->client->v_angle);
  if (ent->s.angles[PITCH] < -29)
    ent->s.angles[PITCH] = -29;
  else
  if (ent->s.angles[PITCH] > 29)
    ent->s.angles[PITCH] = 29;

  gi.linkentity(ent);
  G_TouchTriggers(ent);

  RandomChat(ent);
}

//==============================================
void BotThink(edict_t *ent) {

  // Reset bot's enemy info after enemy died
  if (ent->client->current_enemy)
    if (!G_ClientInGame(ent->client->current_enemy)) {
      ent->client->battleduckcnt = 0;
      ent->client->current_enemy = NULL;
      ent->client->combatstate &= ~0x00000001;
      ent->client->battlemode = 0x00000000; }

  // bot dead? put'em back into game
  if (!G_ClientNotDead(ent)) {
    ent->s.modelindex2 = 0;
    ent->client->routetrace = false;
    if (ent->client->respawn_time <= level.time) {
      ent->client->respawn_time = level.time;
      PutClientInServer(ent); }
    ent->nextthink = level.time+FRAMETIME;
    return; }

  BotAI(ent);

  // Quit camping if health below 10%
  if (ent->health < 10)
    ent->client->camptime = level.time;

  if (ent->client->camptime > level.time)
  {
    VectorCopy(ent->client->lastorigin,ent->s.origin);
//    if (ent->client->campitem == item_railgun)
//	  {
      // Force railgun use if camping at rail else stop camping here
//		if (ent->client->pers.weapon != item_railgun && HasAmmoForWeapon(ent,item_railgun))
//	    {
//        ent->client->newweapon = item_railgun;
//        ChangeWeapon(ent);
//      }
/*	  }
	  else*/
        ent->client->camptime = level.time;
//    }
//    
  }
  else
  {
    if (ent->client->campitem == item_health_mega)
	{
      if (random() < 0.85)
        ent->client->ps.pmove.pm_flags |= PMF_DUCKED;
      // Prefer using grenadelauncher if camping at megahealth
      if (ent->client->pers.weapon != item_grenadelauncher && HasAmmoForWeapon(ent,item_grenadelauncher))
	  {
        ent->client->newweapon = item_grenadelauncher;
        ChangeWeapon(ent);
	  } 
	} //}
//    else
//  if (ent->client->quad_framenum > level.framenum)
    // Force SuperShotgun use if quad enabled!
//    if (ent->client->pers.weapon != item_supershotgun)
//      if (HasAmmoForWeapon(ent,item_supershotgun)) {
//        ent->client->newweapon = item_supershotgun;
//        ChangeWeapon(ent); }
  }
  ent->nextthink = level.time+FRAMETIME;
}


