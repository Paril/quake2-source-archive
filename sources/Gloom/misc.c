#ifndef WIN32
#include <unistd.h>
#include <sys/time.h>
#else
#include <windows.h>
#endif

#include "g_local.h"

#ifndef LENGTH
#define LENGTH(a) (sizeof(a)/sizeof((a)[0]))
#endif

static char buf[2048];

typedef struct {
  char*   dest;
  int     avail; //bytes available for writing (\0 not included). Set to negative to force cancelation of entire text
  char*   src;
  int     len;   //Length available for reading
  edict_t*ent;
  int16_t flags;
  uint8_t playertracemask;
  uint8_t miscmask;
  edict_t*playertrace[MAXTEAMS];
  edict_t*item[MAXTEAMS];
} ExpandLocal_t;


static qboolean Put(ExpandLocal_t*l, char*text, int firstcase)
{
  if (!text) text = "(NULL)"; //Shouldn't happen, but just in case, its better to see some (NULL) around than server crashing

  int n = strlen(text);
  int orig = n;
  if (n > l->avail) n = l->avail;
  if (n <= 0) return false;
  memmove(l->dest, text, n);
  if (firstcase) {
    if (firstcase > 0) {
      if (*l->dest >= 'a' && *l->dest <= 'z') *l->dest &= ~32;
    } else
      if (*l->dest >= 'A' && *l->dest <= 'Z') *l->dest |= 32;
  }
  l->dest += n;
  l->avail -= n;
  return orig == n;
}

static edict_t* PlayerTrace(ExpandLocal_t*l, int team)
{
  if (!(l->playertracemask & (1 << team))) {
    edict_t*t = l->ent->client->target_ent;
    l->playertracemask |= 1 << team;
    if (!t || !t->inuse || !t->client || team && l->ent->client->resp.team != team) {
      vec3_t start = {l->ent->client->target_point[0], l->ent->client->target_point[1], l->ent->client->target_point[2] + l->ent->viewheight};
      vec3_t forward, end;
      trace_t tr;
      vec3_t mins = {-4,-4,-4};
      vec3_t maxs = {4,4,4};
      int n = 5;

      AngleVectors(l->ent->client->v_angle, forward, NULL, NULL);

      while (--n) {
        VectorMA(start, 4096, forward, end);

        tr = gi.trace(start, mins, maxs, end, t, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER); //same box and contents as from CTFSetIDViev
        if (!tr.ent || !tr.ent->takedamage || tr.ent->solid == SOLID_BSP) { //world, door, etc
          t = NULL;
          break;
        }
        t = tr.ent;
        if (t->client && (!team || t->client->resp.team == team) && t != l->ent) { //found what was searching for?
          qboolean found = true;
          if (t->client->resp.class_type == CLASS_GUARDIAN) //Extra check for guard, only if can be seen/detected
            found = ((uint8_t)t->s.modelindex != 255 || l->ent->client->resp.team != TEAM_HUMAN || l->ent->client->resp.class_type == CLASS_BIO);
          if (found) break;
        }
        VectorCopy(tr.endpos, start);
      }
      if (!n) t = NULL;
    }
    l->playertrace[team] = t;
  }
  return l->playertrace[team];
}


static char* GetItemName(edict_t*ent)
{
  int t = ent->enttype;
  if (t == ENT_TURRETBASE) t = ent->target_ent->enttype;
  if (t == ENT_TURRET) return "turret";
  if (t == ENT_MGTURRET) return "mgturret";
  if (t == ENT_DETECTOR) return "DETECTOR";
  if (t == ENT_TRIPWIRE_LASER || t == ENT_TRIPWIRE_BOMB) return "mine";
  if (t == ENT_AMMO_DEPOT) return "depot";
  if (t == ENT_TELEPORTER || t == ENT_TELEPORTER_D) {
    if (ent->spawnflags & 1024) return "unbuilt teleporter";
    return "teleporter";
  }
  if (t == ENT_COCOON || t == ENT_COCOON_D) return "egg";
  if (t == ENT_SPIKER) return "spiker";
  if (t == ENT_OBSTACLE) return "obstacle";
  if (t == ENT_GASSER || t == ENT_GASSER_GAS) return "gasser";
  if (t == ENT_HEALER || t == ENT_EVIL_HEALER) return "healer";
  if (t == ENT_ROCKET) return "rocket";
  if (t == ENT_GRENADE || t == ENT_FLASH_GRENADE) return "grenade";
  if (t == ENT_SMOKE_GRENADE) return "smoke grenade";
  if (t == ENT_C4) return "C4";
  if (t == ENT_GAS_SPORE) return "gas spore";
  if (t == ENT_SPIKE_SPORE) return "spike spore";
  if (t == ENT_PROXY_SPORE) return "guard spore";
  if (t == ENT_FLARE) return "flare";
  if (t == ENT_INFEST) return "infested body";
  if (t == ENT_CORPSE || t == ENT_HUMAN_BODY || t == ENT_FEMALE_BODY || t == ENT_NOBLOOD_BODY) return "corpse"; //TODO: "dead <class>"
  return NULL;
}

static int GetItemDistance(edict_t*ent)
{
  int t;
  
  if (!ent) return 96; //max of the values used below
  
  t = ent->enttype;

  if (t == ENT_TURRETBASE || t == ENT_TURRET || t == ENT_MGTURRET || t == ENT_TRIPWIRE_LASER || t == t == ENT_TRIPWIRE_BOMB) return 72;
  if (t == ENT_DETECTOR) return 40;
  if (t == ENT_AMMO_DEPOT || t == ENT_TELEPORTER || t == ENT_TELEPORTER_D) return 80;

  if (t == ENT_COCOON || t == ENT_COCOON_D) return 80;
  if (t == ENT_SPIKER) return 72;
  if (t == ENT_OBSTACLE) return 64;
  if (t == ENT_GASSER || t == ENT_GASSER_GAS) return 48;
  if (t == ENT_HEALER || t == ENT_EVIL_HEALER) return 72;
  if (t == ENT_ROCKET) return 72;
  if (t == ENT_GRENADE || t == ENT_FLASH_GRENADE) return 80;
  if (t == ENT_SMOKE_GRENADE) return 72;
  if (t == ENT_C4) return 96;
  if (t == ENT_GAS_SPORE) return 56;
  if (t == ENT_SPIKE_SPORE) return 56;
  if (t == ENT_PROXY_SPORE) return 40;
  if (t == ENT_FLARE) return 96;
  if (t == ENT_INFEST) return 72;
  if (t == ENT_CORPSE || t == ENT_HUMAN_BODY || t == ENT_FEMALE_BODY || t == ENT_NOBLOOD_BODY) 72;

  return 0; //unknown item
}

static int GetItemTeam(edict_t*ent)
{
  int t;

  if (!ent) return TEAM_NONE;

  t  = ent->enttype;

  if (t == ENT_TURRETBASE || t == ENT_TURRET || t == ENT_MGTURRET || t == ENT_DETECTOR || t == ENT_TRIPWIRE_LASER ||
      t == ENT_TRIPWIRE_BOMB || t == ENT_AMMO_DEPOT || t == ENT_TELEPORTER || t == ENT_TELEPORTER_D || t == ENT_ROCKET ||
      t == ENT_GRENADE || t == ENT_FLASH_GRENADE || t == ENT_SMOKE_GRENADE || t == ENT_C4 || t  == ENT_FLARE)
    return TEAM_HUMAN;

  if (t == ENT_COCOON || t == ENT_COCOON_D || t == ENT_SPIKER || t == ENT_OBSTACLE || t == ENT_GASSER || t == ENT_GASSER_GAS ||
     t == ENT_HEALER || t == ENT_EVIL_HEALER || t == ENT_GAS_SPORE ||t == ENT_SPIKE_SPORE || t == ENT_PROXY_SPORE || t == ENT_INFEST)
    return TEAM_ALIEN;

  return TEAM_NONE;
}


static edict_t* GetItem(ExpandLocal_t*l)
{
  //TODO: Weights: A grenade/C4 near an obstacle should report grenade/C4 even if aiming at obstacle if player is alien.
  if (!(l->miscmask & 1)) {
    edict_t*item = l->ent->client->target_ent;
    if (!item || !GetItemName(item)) {
      float distance = GetItemDistance(NULL)+0.1f;
      edict_t*ent = NULL;
      while (ent = findradius(ent, l->ent->client->target_point, distance+0.1f)) {
        float dist;
        vec3_t v;
        VectorSubtract(ent->s.origin, l->ent->client->target_point, v);
        dist = VectorLength(v);
        
        if (dist < distance && dist < GetItemDistance(ent) && gi.inPVS(l->ent->s.origin, ent->s.origin)) {
          distance = dist;
          item = ent;
        }
      }
    }
    if (item && !GetItemName(item)) item = NULL;
    l->miscmask |= 1;
    l->item[TEAM_NONE] = item;
  }
  return l->item[TEAM_NONE];
}

static edict_t* GetTeamItem(ExpandLocal_t*l, int team)
{
  int mask = 1 << team;
  
  if (!team) return GetItem(l);

  if (!(l->miscmask & mask)) {
    edict_t*item = NULL;
    float distance = GetItemDistance(NULL)+0.1f;
    edict_t*ent = NULL;
    
    //FIXME: Ignore obstacles from the opposing team
//gi.trace
gi.dprintf("FIXME: Ignore obstacles from the opposing team\n");
    while (ent = findradius(ent, l->ent->client->target_point, distance+0.1f)) {
      float dist;
      vec3_t v;
      VectorSubtract(ent->s.origin, l->ent->client->target_point, v);
      dist = VectorLength(v);
        
      if (dist < distance && dist < GetItemDistance(ent) && gi.inPVS(l->ent->s.origin, ent->s.origin) && GetItemTeam(ent) == team) {
        distance = dist;
        item = ent;
      }
    }

    l->miscmask |= mask;

    l->item[team] = item;
  }

  return l->item[team];
}


static int FindClosingBracket(ExpandLocal_t*l)
{
  char*c = l->src;
  int  r = l->len;
  int  n = 1;
  while (n) {
    if (r <= 0) return 0;
    if (*c == '{') n++;
    else
    if (*c == '}') n--;
    c++;
    r--;
  }
  return c - l->src;
}

static void SubExpand(ExpandLocal_t*l)
{
  char c;
  qboolean expanded, prevexp;
  int fcase;
  int n;

  if (l->len <= 0) return;

  prevexp = false;  
  while (l->avail > 0) {
    while (*l->src != '%') {
      if (l->avail <= 0 || l->len <= 0) return;
      *(l->dest++) = *(l->src++);
      l->avail--;
      l->len--;
    }
    if ((l->len -= 2) < 0) return;
    l->src++;
    c = *(l->src++);
    fcase = (c&32)?-1:+1;
    c |= 32;
    expanded = true;
    if (c == '%') {
      Put(l, "%", 0);
      continue;
    } else
    if (c == 'c')
      Put(l, classlist[l->ent->client->resp.class_type].classname, fcase);
    else
    if (c == 'b') {
      float t = l->ent->client->build_timer - level.time;
      if (t > 0) {
        Put(l, (fcase > 0)?va("%.1f", t):va("%d", (int)t), 0);
      } else
        expanded = false;
    } else
    if (c == 'f') {
      if (fcase > 0)
        n = l->ent->client->resp.total_score;
      else
        n = l->ent->client->resp.score;
      Put(l, va("%d", n), 0);
    } else
    if (c == 'i') {
      edict_t*item = GetItem(l);
      if (item)
        Put(l, GetItemName(item), fcase);
      else
        expanded = false;
    } else
    if (c == 'p') {
      if (fcase > 0)
        n = l->ent->client->armor?l->ent->client->resp.inventory[ITEM_INDEX(l->ent->client->armor)]:0;
      else
        n = l->ent->health;
      Put(l, va("%d", n), 0);
    } else
    if (c == 'q') {
      int t = l->ent->client->resp.team;
      if (t) t = 3-t;
      Put(l, team_info.teamnames[t], fcase);
    } else
    if (c == 'r')
      Put(l, team_info.teamnames[l->ent->client->resp.team], fcase);
    else
    if (c == 't' || c == 's' || c == 'h') {
      edict_t*t = PlayerTrace(l, (c!='t')?(c=='s')?TEAM_ALIEN:TEAM_HUMAN:TEAM_NONE);
      if (t) {
        if (fcase < 0) {
          if (!l->ent->client->resp.team || l->ent->client->resp.team == t->client->resp.team)
            Put(l, t->client->pers.netname, 0);
          else
            expanded = false;
        } else
          Put(l, classlist[t->client->resp.class_type].classname, 0);
      } else
        expanded = false;
    } else
    if (c == 'u') {
      if (l->ent->client->resp.team == TEAM_ALIEN)
        Put(l, "healer", fcase);
      else
      if (l->ent->client->resp.team == TEAM_HUMAN)
        Put(l, "depot", fcase);
      else
        Put(l, "life", fcase);
    } else
    if (c >= '0' && c <= '9') {
       char num[2] = {c, 0};
       char*name = num;
       if (l->ent->client->resp.team == TEAM_ALIEN) {
         static char*hteam[10] = {"0", "Engineer", "Grunt", "Shock trooper", "Biotech", "Heavy trooper", "Commando", "Exterminator", "Mech", "9"};
         name = hteam[c-48];
       } else
       if (l->ent->client->resp.team == TEAM_HUMAN) {
         static char*ateam[10] = {"0", "Breeder", "Hatchling", "Drone", "Wraith", "Kamikaze", "Stinger", "Guardian", "Stalker", "9"};
         name = ateam[c-48];
       }
       Put(l, name, 0);
    } else
    if (c == 'o' || c == 'e') {
      int team = l->ent->client->resp.team;
      c = (l->len>0)?*(l->len--,l->src++):0;

      if (team) {
        if (c == 'b') c = '7';
        else
        if (c == 's') c = '6';
        else
        if (c > '5' && c <= '9') c = '?';

        if (c == 'e') team = (team==TEAM_ALIEN)?TEAM_HUMAN:TEAM_ALIEN;

        if (c >= '0' && c < '8') {
          static uint8_t hcls[8] = {CLASS_GRUNT,     CLASS_SHOCK, CLASS_HEAVY,    CLASS_COMMANDO, CLASS_EXTERM,   CLASS_MECH,    CLASS_BIO,    CLASS_ENGINEER};
          static uint8_t acls[8] = {CLASS_HATCHLING, CLASS_DRONE, CLASS_KAMIKAZE, CLASS_STINGER,  CLASS_GUARDIAN, CLASS_STALKER, CLASS_WRAITH, CLASS_BREEDER};
          Put(l, classlist[(team==TEAM_ALIEN)?acls[c-48]:hcls[c-48]].classname, fcase);
        } else
        if (c == 'h')
          Put(l, (team==TEAM_ALIEN)?"healer":"depot", fcase);
        else
        if (c == 'd')
          Put(l, (team==TEAM_ALIEN)?"spiker":"turret", fcase);
        else
        if (c == 'D')
          Put(l, (team==TEAM_ALIEN)?"gasser":"detector", fcase);
        else
        if (c == 'B')
          Put(l, (team==TEAM_ALIEN)?"obstacle":"mine", fcase);
        else
        if (c == 'h')
          Put(l, (team==TEAM_ALIEN)?"healer":"depot", fcase);
        else
        if (c == 'S')
          Put(l, (team==TEAM_ALIEN)?"egg":"tele", fcase);
        else
        if (c == 'i') {
          edict_t*item = GetTeamItem(l, team);
          if (item)
            Put(l, GetItemName(item), fcase);
          else
            expanded = false;
        } else
          expanded = false;
      } else
        expanded = false;
    } else
    if (c == 'x') {
      int team = l->ent->client->resp.team;
      c = (l->len>0)?*(l->len--,l->src++):0;
      
      if (c == 'a') {
        if (l->ent->health > 0)
          Put(l, "alive", fcase);
        else
          expanded = false;
      } else
      if (c == 'd') {
        if (l->ent->health <= 0)
          Put(l, "dead", fcase);
        else
          expanded = false;
      } else
      if (c == 'h') {
        if (l->ent->health < 0.9f*l->ent->max_health)
          Put(l, "health", fcase);
        else
          expanded = false;
      } else
      if (c == 'r') {
        if (l->ent->client->armor) {
          int index = ITEM_INDEX(l->ent->client->armor);
          
//          if (l->ent->client->resp.inventory[index] < 0.9f*GetInventoryWatermark(l->ent, index)) Put(l, (team == TEAM_ALIEN)?"healer":"repairs", fcase);
        } else
          expanded = false;
      } else
      if (c == 'A' || c == 'R') {
        int msg = false;
        int index;
/*        for (index = 0; !msg && index < LENGTH(l->ent->client->resp.invwatermark) && l->ent->client->resp.invwatermark[index].item; index++) {
          int item = l->ent->client->resp.invwatermark[index].item;
          
          if (itemlist[item].flags & IT_CLIP || itemlist[item].tag == AMMO_GRENADE)
            msg = l->ent->client->resp.inventory[item] < l->ent->client->resp.invwatermark[index].count;
          else
          if (c == 'R' && itemlist[item].flags & IT_ARMOR)
            msg = l->ent->client->resp.inventory[item] < l->ent->client->resp.invwatermark[index].count*0.9f;
        }
*/
#warning FIXME: watermark gone
        
        if (msg) {
          if (c == 'A')
            Put(l, (team == TEAM_ALIEN)?"healer":"ammo", fcase); //FIXME: "ammo" for alien looks odd
          else
            Put(l, (team == TEAM_ALIEN)?"healer":"repairs", fcase);
        } else
          expanded = false;
      } else
      if (c == 'q') {
#ifdef OLD_TEST_QUEUES
        int n = PlayerQueueDataDefault(&queue_spawn_humans, l->ent, CLASS_OBSERVER);
        if (n == CLASS_OBSERVER)
          n = PlayerQueueDataDefault(&queue_spawn_aliens, l->ent, CLASS_OBSERVER);
        if (n != CLASS_OBSERVER)
          Put(l, classlist[n].classname, fcase);
        else
          expanded = false;
#else
        expanded = false;
#endif
      } else
      if (c == 'Q' || c == 'O') {
#ifdef OLD_TEST_QUEUES
        int pos = PlayerQueuePosition(NULL, l->ent);
        if (pos > 0) {
          if (c == 'Q')
            Put(l, va("%d", pos), fcase);
          else {
            char*ordinal[3] = {"1st", "2nd", "3rd"};
            if (pos < LENGTH(ordinal))
              Put(l, ordinal[pos], fcase);
            else
              Put(l, va("%dth", pos), fcase);
          }
        } else
          expanded = false;
#else
        expanded = false;
#endif
      } else
        expanded = false;
    }
    if (c == '?') {
      expanded = !prevexp;
    } else
    if (c == '{') {
      n = FindClosingBracket(l);
      l->src += n;
      l->len -= n;
      //This is reserved for future use. Just ignores contents atm.
    } else
    if ((c < '0' || c > '9') && c < 'a' || c > 'z') {
      Put(l, va("%%%c", l->src[-1]), 0);
      continue;
    } else
      expanded = false;
    
    prevexp = expanded;
    
    if (l->flags & EF_CANCEL_IF_FAIL && !expanded) {
      l->avail = -1; //cancel all text
      return;
    }

    if (l->len > 0 && *l->src == '{') {
      char*nsrc = ++l->src;
      int  nlen = --l->len;
      n = FindClosingBracket(l);
      if (!expanded) {
        if (n == 2 && *nsrc == '-') {
          l->avail = -1; //cancel all text
          return;
        }
        l->len = n-1;
        SubExpand(l);
      }
      l->src = nsrc+n;
      l->len = nlen-n;
    }
  }
}

char* ExpandTextMacros(edict_t*ent, char*text, int flags)
{
  ExpandLocal_t l;
  l.ent = ent;
  l.dest = buf;
  l.avail = sizeof(buf)-1;
  l.src = text;
  l.len = strlen(text);
  l.flags = flags;
  l.playertracemask = l.miscmask = 0;
  
  SubExpand(&l);

  if (l.avail >= 0)
    *l.dest = 0;
  else
    buf[0] = 0;
  return buf;
}















void teamprintf(int team, int mode, char*fmt, ...)
{
  edict_t *ent;
  va_list list;
  
  char buf[1024];

  va_start(list, fmt);
  vsnprintf(buf, sizeof(buf), fmt, list);
  va_end(list);

  for (ent= g_edicts+1; ent <= g_edicts+game.maxclients; ent++)
    if (ent->inuse && ent->client->resp.team == team)
      gi.cprintf(ent, mode, "%s", buf);
}


char*customformat(char*args, char*text, char*append)
{
  static char buf[258];
  char*dst = buf;
  int max = sizeof(buf)-2;
  if (!args) args = "";
  if (append) max -= strlen(append);
  while (*text && dst < buf+max) {
    if (*text != '@')
      *(dst++) = *text;
    else {
      if (text[1] != '@' && *args) {
        *(dst++) = '%';
        *(dst++) = *(args++);
      } else {
        *(dst++) = '@';
        if (text[1]) text++;
      }
    }
    text++;
  }
  if (append)
    strcpy(dst, append);
  else
    *dst = 0;
  return buf;
}















void DebugTrail(vec3_t a, vec3_t b)
{
  gi.WriteByte(svc_temp_entity);
  gi.WriteByte(TE_DEBUGTRAIL);
  gi.WritePosition(a);
  gi.WritePosition(b);
  gi.multicast(a, MULTICAST_PVS);
}

void DebugTrailDir(vec3_t a, vec3_t dir)
{
  vec3_t b;
  VectorAdd(a, dir, b);
  gi.WriteByte(svc_temp_entity);
  gi.WriteByte(TE_DEBUGTRAIL);
  gi.WritePosition(a);
  gi.WritePosition(b);
  gi.multicast(a, MULTICAST_PVS);
}

static void debugtrail(float x0, float y0, float z0, float x1, float y1, float z1)
{
  vec3_t a = {x0, y0, z0};
  vec3_t b = {x1, y1, z1};
  gi.WriteByte(svc_temp_entity);
  gi.WriteByte(TE_DEBUGTRAIL);
  gi.WritePosition(a);
  gi.WritePosition(b);
}

void DebugBox(vec3_t a, vec3_t b)
{
  debugtrail(b[0], b[1], a[2],  a[0], b[1], a[2]);
  debugtrail(b[0], b[1], a[2],  b[0], a[1], a[2]);
  debugtrail(a[0], a[1], a[2],  a[0], b[1], a[2]);
  debugtrail(a[0], a[1], a[2],  b[0], a[1], a[2]);

  debugtrail(b[0], b[1], b[2],  a[0], b[1], b[2]);
  debugtrail(b[0], b[1], b[2],  b[0], a[1], b[2]);
  debugtrail(a[0], a[1], b[2],  a[0], b[1], b[2]);
  debugtrail(a[0], a[1], b[2],  b[0], a[1], b[2]);

  debugtrail(a[0], a[1], a[2],  a[0], a[1], b[2]);
  debugtrail(b[0], b[1], a[2],  b[0], b[1], b[2]);
  debugtrail(b[0], a[1], a[2],  b[0], a[1], b[2]);
  debugtrail(a[0], b[1], a[2],  a[0], b[1], b[2]);

  gi.multicast(a, MULTICAST_PVS);
}

void DebugDir(vec3_t a, vec3_t dir)
{
  vec3_t b;
  VectorMA(a, 40, dir, b);
  gi.WriteByte(svc_temp_entity);
  gi.WriteByte(TE_BFG_LASER);
  gi.WritePosition(a);
  gi.WritePosition(b);
  gi.multicast(a, MULTICAST_PVS);
}




int debug_n = 0;
char Debug(debug_t item)
{
  if (item->n != debug_n) {
    char*s = va(",%s,",g_debug->string);
    item->enabled = (strstr(s, item->name) != NULL) || (strstr(s, ",all,") != NULL);
    item->n = debug_n;
  }
  return item->enabled || item->parent && Debug(item->parent);
}








float Distance1(vec3_t a, vec3_t b)
{
  return sqrt((a[0]-b[0])*(a[0]-b[0]) + (a[1]-b[1])*(a[1]-b[1]) + (a[2]-b[2])*(a[2]-b[2]));
}

float Distance2(vec3_t a, vec3_t b)
{
  return (a[0]-b[0])*(a[0]-b[0]) + (a[1]-b[1])*(a[1]-b[1]) + (a[2]-b[2])*(a[2]-b[2]);
}

void RandomVector(vec3_t v, float len)
{
  float l;
  do {
    v[0] = crandom();
    v[1] = crandom();
    v[2] = crandom();
    l = VectorLength(v);
  } while (l == 0);
  VectorScale(v, len/l, v);
}


trace_t EXPORT PM_trace_gloom (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end); //from p_client.c

gloom_pmove_t*PlayerGloomPMove(edict_t*player, pmove_t*pm)
{
  static gloom_pmove_t gpm;
  static pmove_t tmp_pm;
  
  if (pm) {
    gpm.pmove = pm;
  } else {
    gpm.pmove = &tmp_pm;

    tmp_pm.s.origin[0] = player->s.origin[0]*8;
    tmp_pm.s.origin[1] = player->s.origin[1]*8;
    tmp_pm.s.origin[2] = player->s.origin[2]*8;

    tmp_pm.s.velocity[0] = player->velocity[0]*8;
    tmp_pm.s.velocity[1] = player->velocity[1]*8;
    tmp_pm.s.velocity[2] = player->velocity[2]*8;

    tmp_pm.s = player->client->ps.pmove;
    
    tmp_pm.trace = PM_trace_gloom;
    tmp_pm.pointcontents = gi.pointcontents;
  }

  gpm.self = player;
  gpm.ent_origin = player->s.origin;
  gpm.ent_velocity = player->velocity;
  gpm.state = &player->client->gpms;
  
  gpm.trace = (void*)gi.trace;
  
  return &gpm;
}



static int nft_msec;
#ifndef WIN32
static struct timeval nft;

void ClearNextMiddleFrame()
{
  nft_msec = 0;
}

void SetNextMiddleFrame(int msec)
{
  gettimeofday(&nft, NULL);
  nft.tv_usec += msec*1000;
  if (nft.tv_usec >= 1000000) {
    nft.tv_sec++;
    nft.tv_usec -= 1000000;
  }
  nft_msec = msec;
}

int CheckNextMiddleFrame()
{
  struct timeval t;
  int msec;

  if (!nft_msec) return 0;

  gettimeofday(&t, NULL);
  
  if (t.tv_sec < nft.tv_sec || t.tv_usec < nft.tv_usec) return 0;

  msec = nft_msec;
  nft_msec = 0;

  return msec;
}

#else
static unsigned int nft;
void ClearNextMiddleFrame()
{
  nft_msec = 0;
}

void SetNextMiddleFrame(int msec)
{
  nft_msec = msec;
  nft = GetTickCount();
}

int CheckNextMiddleFrame()
{
  unsigned int msec, tc;

  if (!nft_msec) return 0;

  tc = GetTickCount()-nft_msec; //overflows every 49.71 days
  if (tc < nft) return;
  
  msec = nft_msec;
  nft_msec = 0;

  return msec;
}
#endif





void Use_Health (edict_t *ent, const gitem_t *item);
void Cmd_Flashlight_f (edict_t *ent);
void Cmd_Grenade_f (edict_t *ent);
void Cmd_WeapSwitch_f(edict_t *ent);

void ImpulsePreview (edict_t *ent, int code)
{
    int selected = code - 28; // menu selection number
    if (ent->client->menu.entries == engineer_menu && ent->client->menu.cur == selected) {
            switch (selected) {
                case 2:
                    create_teleporter(ent);
                    break;
                case 3:
                    create_turret(ent);
                    break;
                case 4:
                    create_mgturret(ent);
                    break;
                case 5:
                    create_detector(ent);
                    break;
                case 6:
                    create_tripwire(ent);
                    break;
                case 7:
                    create_depot(ent);
                    break;
            }
    }
    else
        PMenu_Open(ent, engineer_menu, selected, 3);
}

void Impulse(edict_t*ent, int code)
{
  if (code == 'g') Cmd_Grenade_f (ent);
  else
  if (code == 'f') Cmd_Flashlight_f (ent);
  else
  if (code == 'r') Reload(ent);
  else
  if (code == 'h') Use_Health (ent, &itemlist[ITEM_HEALTH]);
  else
  if (code == 's') Cmd_Use_f(ent, "special");
  else
  if (code == 'W') Cmd_Use_f(ent, "primary");
  else
  if (code == 'w') Cmd_Use_f(ent, "secondary");
  else
  if (code == 'S') Cmd_WeapSwitch_f(ent);
  else
  if (code >= '0' && code < '9') {
    if (ent->health > 0 && ent->client->last_voice_framenum + (int)(2.0f/FRAMETIME) <= level.framenum && classlist[ent->client->resp.class_type].voicefunc) {
      classlist[ent->client->resp.class_type].voicefunc(ent, code-48);
      ent->client->last_voice_framenum = level.framenum;
    }
  } else
  if (code == 8) RemovePlayerFromQueues(ent);
  else
#ifdef GLOOM_MENU
  if (code >= 200 && code < 210) {
    if (ent->client->gmenu)
      GMenu_ExecuteNumber(ent, code-200);
  } else
  if (code == 'y' || code == 'n' || code == '-') {
    int n = 0; //abstain
    if (code == 'y') n = +1;
    else
    if (code == 'n') n = -1;

    if (vote_active)
      Player_CastVote(n);
  } else
#endif
  if (ent->client->resp.team == TEAM_HUMAN) {
    if (code == 20) TeamStart(ent, CLASS_ENGINEER);
    else
    if (code == 21) TeamStart(ent, CLASS_GRUNT);
    else
    if (code == 22) TeamStart(ent, CLASS_SHOCK);
    else
    if (code == 23) TeamStart(ent, CLASS_BIO);
    else
    if (code == 24) TeamStart(ent, CLASS_HEAVY);
    else
    if (code == 25) TeamStart(ent, CLASS_COMMANDO);
    else
    if (code == 26) TeamStart(ent, CLASS_EXTERM);
    else
    if (code == 27) TeamStart(ent, CLASS_MECH);
    else
    if (ent->client->resp.class_type == CLASS_ENGINEER && ent->health > 0) {
      if (code == 10) create_teleporter(ent);
      else
      if (code == 11) create_turret(ent);
      else
      if (code == 12) create_mgturret(ent);
      else
      if (code == 13) create_detector(ent);
      else
      if (code == 14) create_tripwire(ent);
      else
      if (code == 15) create_depot(ent);
      else
      if (code >= 30 && code <= 35) ImpulsePreview(ent, code);
    }
  } else
  if (ent->client->resp.team == TEAM_ALIEN) {
    if (code == 20) TeamStart(ent, CLASS_BREEDER);
    else
    if (code == 21) TeamStart(ent, CLASS_HATCHLING);
    else
    if (code == 22) TeamStart(ent, CLASS_DRONE);
    else
    if (code == 23) TeamStart(ent, CLASS_WRAITH);
    else
    if (code == 24) TeamStart(ent, CLASS_KAMIKAZE);
    else
    if (code == 25) TeamStart(ent, CLASS_STINGER);
    else
    if (code == 26) TeamStart(ent, CLASS_GUARDIAN);
    else
    if (code == 27) TeamStart(ent, CLASS_STALKER);
    else
    if (ent->client->resp.class_type == CLASS_BREEDER && ent->health > 0) {
      if (code == 10) lay_egg(ent);
      else
      if (code == 11) lay_healer(ent);
      else
      if (code == 12) lay_obstacle(ent);
      else
      if (code == 13) lay_spiker(ent);
      else
      if (code == 14) lay_gasser(ent);
    }
  }
}

void cvar_range(cvar_t*cvar, float*out_min, float*out_max)
{
  float discard;
  char*s = strchr(cvar->string, '-');
  
  if (!out_min) out_min = &discard;
  if (!out_max) out_max = &discard;
  
  *out_min = *out_max = cvar->value;
  if (s) {
    s++;
    while (*s < 32) s++;
  }
  if (s && *s) {
    char*e;
    float f = strtof(s, &e);
    if (e > s)
      *out_max = f;
  }
  
  if (*out_max < *out_min) {
    float f = *out_max;
    *out_max = *out_min;
    *out_min = f;
  }
}

