#include "g_local.h"

typedef struct local_s { //local client/player information
  pqueue_t* q_ptr;
  edict_t*  q_next;
  int       q_data;    //team to join, class to spawn, etc
  int8_t    q_pos;     //Position to show to player in layout.
  int8_t    hudstate;  //For updating layout
  int16_t   unused16;
  int32_t   unused32;
  float     time;
} local_t;



void Updateteam_menu(void);
void PMenu_Update(edict_t *ent);
qboolean RealTeamStart (edict_t *ent, int class_type); //g_breaking.c


static local_t local[96];

static float dweights[CLASS_OBSERVER];
static float last_respawn[4];

static float sweights[CLASS_OBSERVER+1] = {
  /*CLASS_GRUNT     */ 0.0f,
  /*CLASS_HATCHLING */ 0.0f,
  /*CLASS_HEAVY     */ 1.0f,
  /*CLASS_COMMANDO  */ 3.0f,
  /*CLASS_DRONE     */ 2.0f,
  /*CLASS_MECH      */ 5.0f,
  /*CLASS_SHOCK     */ 2.0f,
  /*CLASS_STALKER   */ 5.0f,
  /*CLASS_BREEDER   */ 2.8f,
  /*CLASS_ENGINEER  */ 2.8f,
  /*CLASS_GUARDIAN  */ 4.0f,
  /*CLASS_KAMIKAZE  */ 1.0f,
  /*CLASS_EXTERM    */ 4.0f,
  /*CLASS_STINGER   */ 3.0f,
  /*CLASS_WRAITH    */ 1.5f,
  /*CLASS_BIO       */ 0.5f,
  /*CLASS_OBSERVER  */ -65536,
};

void InitializePlayerQueues(pqueue_t*q, ...)
{
  va_list va;
  memset(local, 0, sizeof(local));
  memset(dweights, 0, sizeof(dweights));
  va_start(va, q);
  do {
    q->first = q->last = NULL;
    q->count = 0;
  } while (q = va_arg(va, pqueue_t*));
  va_end(va);
}

qboolean PlayerQueueAdd(pqueue_t*q, edict_t*player, int userdata)
{
  local_t*l = local + (player-g_edicts)-1;
  if (!player->client || l->q_ptr) return false;
  l->q_ptr = q;
  l->q_next = NULL;
  if (q->last)
    local[q->last-g_edicts-1].q_next = player;
  q->last = player;
  if (!q->first) q->first = player;
  q->count++;
  l->q_pos = q->fifo?q->count:-1;
  l->q_data = userdata;
  l->hudstate = 0;
  l->time = level.time;
  return true;
}

qboolean PlayerQueueRemove(pqueue_t*q, edict_t*player)
{
  local_t*l = local + (player-g_edicts)-1;
  if (!player->client || !l->q_ptr || (q && l->q_ptr != q)) return false;
  
  q = l->q_ptr;
  
  edict_t**p = &q->first;
  edict_t*prev = NULL;
  while (*p) {
    if (*p == player) {
      *p = l->q_next;
      if (q->last == player) q->last = prev;
      q->count--;
      
      if (q->fifo) {
        edict_t*u = l->q_next;
        while (u) {
          local[u-g_edicts-1].q_pos--;
          u = local[u-g_edicts-1].q_next;
        }
      }
      break;
    }
    prev = *p;
    p = &local[*p-g_edicts-1].q_next;
  }
  
  l->q_ptr = NULL;
  
  return true;
}

edict_t* PlayerQueueNext(pqueue_t*q, edict_t*current)
{
  local_t*l = local + (current-g_edicts)-1;
  if (!current) return q->first;
  if (!current->client || l->q_ptr != q) return NULL;
  return l->q_next;
}

qboolean PlayerQueueTest(pqueue_t*q, edict_t*player)
{
  local_t*l = local + (player-g_edicts)-1;
  if (!player->client) return false;
  if (!q) return l->q_ptr != NULL;
  return l->q_ptr == q;
}

int*PlayerQueueData(pqueue_t*q, edict_t*player)
{
  local_t*l = local + (player-g_edicts)-1;
  if (!player->client || l->q_ptr != q) return NULL;
  return &l->q_data;
}

int PlayerQueueDataDefault(pqueue_t*q, edict_t*player, int defaultret)
{
  int*data = PlayerQueueData(q, player);
  if (data) return *data;
  return defaultret;
}

int PlayerQueuePosition(pqueue_t*q, edict_t*player)
{
  local_t*l = local + (player-g_edicts)-1;
  if (!player->client || (l->q_ptr != q && (q != NULL || l->q_ptr == NULL))) return 0;
  return l->q_pos;
}

void PlayerQueueSetPosition(pqueue_t*q, edict_t*player, int pos)
{
  local_t*l = local + (player-g_edicts)-1;
  if (!player->client || l->q_ptr != q) return;
  l->q_pos = pos;
}

float PlayerQueueTimeStamp(edict_t*player)
{
  local_t*l = local + (player-g_edicts)-1;
  if (!player->client || !l->q_ptr) return -1;
  return l->time;
}


void UpdatePlayerQueueHUD(edict_t*player)
{/* hud states:
 0 = new, needs enable layout
 1 = outdated, something changed layout text
 2 = up to date, do nothing
*/
  local_t*l = local + (player-g_edicts)-1;
  pqueue_t*q;
  
  if (!player->client || !(q = l->q_ptr)) return;
  
  player->client->ps.stats[30] = l->q_pos;
  player->client->ps.stats[31] = q->count;

  if (player->client->menu.entries || player->client->showscores) { //menu/scores overwrites layout, need to restore once menu is gone
    l->hudstate = 0;
    return;
  }
  
  if (l->hudstate) {
    player->client->ps.stats[STAT_LAYOUTS] |= 1;
  }

  if (!l->hudstate) {
    l->hudstate = 1;
  }

  if (l->hudstate == 1) {
    char buf[1024];
    char* layout = NULL;

    if (q->GetLayout) {
      layout = q->GetLayout(q, player, buf, sizeof(buf)-1);
      buf[sizeof(buf)-1] = 0;
    }
    
    if (!layout) {
      layout = buf;
      strcpy(buf, "xv 50 yv 50 string \"TODO: Default layout for queue\"");
    }

    gi.WriteByte(svc_layout);
    gi.WriteString(layout);
    gi.unicast(player, true);

    l->hudstate = 2;
  }
}

void InvalidatePlayerQueueHUD(edict_t*player)
{
  local_t*l = local + (player-g_edicts)-1;
  pqueue_t*q;
  if (!player->client || !(q = l->q_ptr)) return;
  
  if (l->hudstate == 2) l->hudstate = 1;
  else
  if (player->client->menu.entries) PMenu_Update(player);
  else
  if (player->client->showscores) DeathmatchScoreboard(player);
}


qboolean AppendPlayerQueueLayout(edict_t*player, char*start, int avail)
{
  local_t*l = local + (player-g_edicts)-1;
  pqueue_t*q;
  if (avail < 16 || !player->client || !(q = l->q_ptr) || !q->GetLayout) return false; //TODO: default layout
  
  *(start++) = 32; avail--;

  char*res = q->GetLayout(q, player, start, avail);
  if (res < start || res >= start+avail) strncpy(start, res, avail);
  res[avail-1] = 0;
  return true;
}










static char * GetJoinLayout(struct pqueue_s*, edict_t*, char*buf, int bufsize);
static char * GetSpawnLayout(struct pqueue_s*, edict_t*, char*buf, int bufsize);



pqueue_t queue_join_humans = {
  /*Name     */ "Join humans",
  /*GetLayout*/ GetJoinLayout,
  /*fifo     */ true
};

pqueue_t queue_join_aliens = {
  /*Name     */ "Join aliens",
  /*GetLayout*/ GetJoinLayout,
  /*fifo     */ true
};

pqueue_t queue_spawn_humans = {
  /*Name     */ "Spawn humans",
  /*GetLayout*/ GetSpawnLayout,
  /*fifo     */ false
};

pqueue_t queue_spawn_aliens = {
  /*Name     */ "Spawn aliens",
  /*GetLayout*/ GetSpawnLayout,
  /*fifo     */ false
};



static char * GetJoinLayout(struct pqueue_s*q, edict_t*ent, char*buf, int bufsize)
{
  snprintf(buf, bufsize, "xl 88 yb -32 string2 \"%s\" xl 16 string \"Joining:\" xl 32 yb -24 num 2 30 xl 72 yb -16 string \"of\" xl 88 yb -24 num 2 31", (q==&queue_join_humans)?"Humans":"Aliens");
  return buf;
}

static char * GetSpawnLayout(struct pqueue_s*q, edict_t*ent, char*buf, int bufsize)
{
  int ct = PlayerQueueDataDefault(q, ent, CLASS_OBSERVER);

  snprintf(buf, bufsize, "xl 96 yb -32 string2 \"%s\" xl 16 string \"Spawning:\" xl 32 yb -24 num 2 30 xl 72 yb -16 string \"of\" xl 88 yb -24 num 2 31", classlist[ct].classname);
  return buf;
}



edict_t *SelectNearestDeathmatchSpawnPoint (int team, edict_t *ent);

static void PJQ_team(pqueue_t*q, int max, int team)
{
  while (q->count && (max-- > 0)) {
    edict_t*player = q->first;
    PlayerQueueRemove(q, player);
    TeamChange(player, team, false);
    OpenClassMenu(player, false);

    if (level.time-1 > PlayerQueueTimeStamp(player)) { //If player had to wait on queue for more than 1 second, send also notification sound
      if (team == TEAM_HUMAN)
        unicastSound(player, SoundIndex(misc_activate), 1.0);
      else
        unicastSound(player, SoundIndex(weapons_webshot2), 1.0);
    }
    
    if (!player->client->chase_target || player->client->chase_target->client->resp.team != team) {
      player->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
      player->client->ps.pmove.pm_type = PM_FREEZE;
      if (player->target_ent && player->target_ent->inuse && player->target_ent->client && player->target_ent->client->resp.team == team) {
        ChaseStart(player, player->target_ent);
        player->target_ent = NULL;
      } else
        ChaseTeamStart(player);
    }
  }
}


void ProcessJoinQueues()
{
  int maxdiff = 100;
  int jh, ja, i;
  int balance = 0;

  if (team_info.starttime > level.time) return; //Wait until the turtle is gone

  if (teameven->value) {
    if ((int)teameven->value == 1)
      maxdiff = 1;
    else
      maxdiff = 0;
  }
  
  jh = team_info.numplayers[TEAM_HUMAN]+queue_join_humans.count;
  ja = team_info.numplayers[TEAM_ALIEN]+queue_join_aliens.count;

  //Players going directly from a team to the other
  for (i = 0; i < game.maxclients; i++) {
    if (game.clients[i].resp.team == TEAM_ALIEN && local[i].q_ptr == &queue_join_humans) balance++;
    if (game.clients[i].resp.team == TEAM_HUMAN && local[i].q_ptr == &queue_join_aliens) balance--;
  }
  ja -= balance;
  jh += balance;

  if (jh+maxdiff > ja) jh = ja+maxdiff;
  if (ja+maxdiff > jh) ja = jh+maxdiff;

  PJQ_team(&queue_join_humans, jh-team_info.numplayers[TEAM_HUMAN], TEAM_HUMAN);
  PJQ_team(&queue_join_aliens, ja-team_info.numplayers[TEAM_ALIEN], TEAM_ALIEN);
}



qboolean validatejoin(edict_t*ent, qboolean msg)
{
  if (forceteams->value && !(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS))) {
    if (msg) gi.cprintf (ent, PRINT_HIGH, "Enforced teams are active. Send a message to an admin to join a team.\n");
    return false;
  }

  if(maxswitches->value && ent->client->resp.teamchanges > maxswitches->value) {
    if (msg) gi.cprintf (ent, PRINT_HIGH, "You have used up your team changes.\n");
    return false;
  }

  if (ent->client->resp.team == TEAM_NONE) {
     if (team_info.numplayers[TEAM_ALIEN] + team_info.numplayers[TEAM_HUMAN] >= maxplayers->value) {
       if (msg) gi.cprintf (ent, PRINT_HIGH, "All %d player slots are currently full, you may only observe.\n", (int)maxplayers->value);
       return false;
     }
   }
 
  return true;
}



void JoinHumanQueueMenu(edict_t*ent)
{
  if (ent->client->resp.team == TEAM_HUMAN) {
    OpenMenu(ent);
    return;
  }
  if (!validatejoin(ent, true)) return;

  PMenu_Close(ent);
  if (ent->client->resp.team != TEAM_NONE) {
    TeamChange(ent, TEAM_NONE, true);
    RealTeamStart(ent, CLASS_OBSERVER);
  }

  int c = queue_join_humans.count;
  PlayerQueueAdd(&queue_join_humans, ent, 0);
  ProcessJoinQueues();
  if (c != queue_join_humans.count) Updateteam_menu();
}

void JoinAlienQueueMenu(edict_t*ent)
{
  if (ent->client->resp.team == TEAM_ALIEN) {
    OpenMenu(ent);
    return;
  }
  if (!validatejoin(ent, true)) return;

  PMenu_Close(ent);
  if (ent->client->resp.team != TEAM_NONE) {
    TeamChange(ent, TEAM_NONE, true);
    RealTeamStart(ent, CLASS_OBSERVER);
  }

  int c = queue_join_aliens.count;
  PlayerQueueAdd(&queue_join_aliens, ent, 0);
  ProcessJoinQueues();
  if (c != queue_join_aliens.count) Updateteam_menu();
}


static int ssq_cmp(float**a, float**b)
{
  if (**a > **b) return 1;
  if (**a < **b) return -1;
  return 0;
}

static void SortSpawnQueue(pqueue_t*q, edict_t*debugent)
{
  float*t[64];
  if (svtype != svt_match) {
    int n = 0;
    int i;
    float w[64];
    edict_t*ent = NULL;
    i = 0;
    while (ent = PlayerQueueNext(q, ent)) {
      int c = PlayerQueueDataDefault(q, ent, CLASS_OBSERVER);

      w[n] = dweights[c] + sweights[c] + i--*0.0001f;

      t[n] = w+n;
      n++;
    }

    qsort(t, n, sizeof(float*), (void*)ssq_cmp);

    ent = NULL; i = 0;
    while (ent = PlayerQueueNext(q, ent)) {
      int order = n - (t[i++]-w);

      PlayerQueueSetPosition(q, ent, order);
    }
  } else {
    edict_t*ent = NULL;
    int i = 0;
    while (ent = PlayerQueueNext(q, ent)) {
      t[i] = i;
      PlayerQueueSetPosition(q, ent, i++);
    }
  }
  
  if (debugent) {
    char buf[4096];
    char*dst = buf;
    edict_t*list[96];
    int i, n;
    edict_t*ent = NULL;

    while (ent = PlayerQueueNext(q, ent))
      list[PlayerQueuePosition(q, ent)-1] = ent;

    for (n = 0; n < i; n++)
      dst += sprintf(dst, "%10s: %.3s %d(%.2f) | ",
        list[n]->client->pers.netname,
        classlist[PlayerQueueDataDefault(q, list[n], CLASS_OBSERVER)].classname,
        n+1,
        *(t[i-n-1])
      );

    if (dst != buf)
      gi.cprintf(debugent, PRINT_HIGH, "Queue: %s\n", buf);
    else
      gi.cprintf(debugent, PRINT_HIGH, "Queue is empty\n");
  }
}

void Cmd_DumpSpawnQueues_f(edict_t*ent)
{
  int i;
  char buf[4096];
  char*dst = buf;
  
  int q = 3;

  if (*gi.argv(1) | 32 == 'h') q = 5;
  else
  if (*gi.argv(1) | 32 == 'a') q = 10;

  if (q & 1) {
    dst += sprintf(dst, "Humans: ");
    for (i = 0; i < CLASS_OBSERVER; i++) if (classlist[i].team == TEAM_HUMAN)
      dst += sprintf(dst, "%.3s: %5.2f; ", classlist[i].classname, dweights[i] + sweights[i]);
    dst += sprintf(dst, "\n");
  }
  if (q & 2) {
    dst += sprintf(dst, "Aliens: ");
    for (i = 0; i < CLASS_OBSERVER; i++) if (classlist[i].team == TEAM_ALIEN)
      dst += sprintf(dst, "%.3s: %5.2f; ", classlist[i].classname, dweights[i] + sweights[i]);
    dst += sprintf(dst, "\n");
  }
  gi.cprintf(ent, PRINT_HIGH, "%s", buf);

  if (q & 8) SortSpawnQueue(&queue_spawn_aliens, ent);
  else
  if (q & 4) SortSpawnQueue(&queue_spawn_humans, ent);
}


static void PSQ_team(pqueue_t*q)
{
//TODO: Delay players by means of ent->client->resp.respawn_time if respawns twice in 2 seconds
  int i, n;
  edict_t*list[64];
  edict_t*player = NULL;
  while (player = PlayerQueueNext(q, player)) {
    unsigned int n = PlayerQueuePosition(q, player)-1;
    if (n >= 64) gi.error("Overflowed(%d) spawn sorted list \n", n);
    list[n] = player;
  }

  n = q->count;
  i = 0;
  while (i < n && SelectNearestDeathmatchSpawnPoint(q->first->client->resp.team, q->first)) {
    int classtype = PlayerQueueDataDefault(q, player = list[i], CLASS_OBSERVER);
    PlayerQueueRemove(q, player);
    if (RealTeamStart(player, classtype)) {
      int c;
      int t = classlist[classtype].team;
      float f = (level.time - last_respawn[t])/10;
      last_respawn[t] = level.time;
      if (f < 1) f = 1;
      f = 1.0f/f;
      for (c = 0; c < CLASS_OBSERVER; c++) if (classlist[c].team == t) {
        if (c == classtype) {
          dweights[c] -= 0.25f;
          if (dweights[c] < -1.5f) dweights[c] = -1.5f;
        } else
          dweights[c] *= 0.75f;
        dweights[c] *= f;
      }
    }
    i++;
  }
  
  SortSpawnQueue(q, NULL);
}

void ProcessSpawnQueues(int team)
{
  if (!team || team == TEAM_ALIEN)
    PSQ_team(&queue_spawn_aliens);

  if (!team || team == TEAM_HUMAN)
    PSQ_team(&queue_spawn_humans);
}



void SpawnProcessQueue(edict_t*ent)
{
  ProcessJoinQueues();
  ProcessSpawnQueues((ent->enttype == ENT_COCOON)?TEAM_ALIEN:TEAM_HUMAN);

  if (ent->enttype == ENT_TELEPORTER && ent->think == SpawnProcessQueue && ent->teleport_time > level.time)
    ent->nextthink = ent->teleport_time;
}

void QueuePlayerRespawn(edict_t*player, int class_type)
{
  int t = classlist[class_type].team;

  if (player->client->resp.team != t) return; //Shouldn't happen

  if (t == TEAM_ALIEN) {
    if (PlayerQueueAdd(&queue_spawn_aliens, player, class_type)) {
      SortSpawnQueue(&queue_spawn_aliens, NULL);
    } else {
      int*ct = PlayerQueueData(&queue_spawn_aliens, player);
      if (ct && *ct != class_type) {
        *ct = class_type;
        SortSpawnQueue(&queue_spawn_aliens, NULL);
        InvalidatePlayerQueueHUD(player);
      }
    }
  } else
  if (t == TEAM_HUMAN) {
    if (PlayerQueueAdd(&queue_spawn_humans, player, class_type)) {
      SortSpawnQueue(&queue_spawn_humans, NULL);
    } else {
      int*ct = PlayerQueueData(&queue_spawn_humans, player);
      if (ct && *ct != class_type) {
        *ct = class_type;
        SortSpawnQueue(&queue_spawn_humans, NULL);
        InvalidatePlayerQueueHUD(player);
      }
    }
  } else {
    RemovePlayerFromQueues(player);
    RealTeamStart(player, CLASS_OBSERVER); //Obses change team directly
    return;
  }

  ProcessJoinQueues();
  ProcessSpawnQueues(t);
}

void RemovePlayerFromQueues(edict_t*player)
{
  if (PlayerQueueRemove(&queue_spawn_humans, player))
    SortSpawnQueue(&queue_spawn_humans, NULL);
  else
  if (PlayerQueueRemove(&queue_spawn_aliens, player))
    SortSpawnQueue(&queue_spawn_aliens, NULL);
  else
    PlayerQueueRemove(NULL, player);
}




void SkipJoinQueueMenu(edict_t*ent)
{
  PMenu_Close(ent);
  int t = 0;
  if (PlayerQueueRemove(&queue_join_humans, ent)) t = TEAM_HUMAN;
  else
  if (PlayerQueueRemove(&queue_join_aliens, ent)) t = TEAM_ALIEN;
  else
  return;

  TeamChange(ent, t, false);
  OpenClassMenu(ent, false);

  ChaseTeamStart(ent);
}
