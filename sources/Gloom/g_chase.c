/*
    Quake II Glooom, a total conversion mod for Quake II
    Copyright (C) 1999-2007  Gloom Developers

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
*/

#include "g_local.h"

static void UpdateChaseIR(edict_t*ent)
{
  if (!ent->client->resp.team) return; //real obses are unlocked

  if (ent->client->chase_target) {
    edict_t*other = ent->client->chase_target;
    if (other->client) {
      ent->client->ps.rdflags = ent->client->ps.rdflags & ~RDF_IRGOGGLES | (other->client->ps.rdflags & RDF_IRGOGGLES);
      return;
    }
  }

  if (ent->client->resp.team == TEAM_ALIEN)
    ent->client->ps.rdflags |= RDF_IRGOGGLES;
  else
    ent->client->ps.rdflags &= ~RDF_IRGOGGLES;
}

static qboolean StartChaseFromSpawn(edict_t*ent)
{
  edict_t*other;
  int team = ent->client->resp.team;

  if (ent->client->resp.class_type != CLASS_OBSERVER || ent->client->chase_target || !team || ent->movetype != MOVETYPE_NONE) return false;

  //search alive teammate  
  for (other = g_edicts+1; other->client; other++) if (other->inuse && other->client->resp.class_type != CLASS_OBSERVER && ent->client->resp.team == team && other->health > 0) {
    ent->movetype = MOVETYPE_NOCLIP;
    ent->client->chase_target = other;
    UpdateChaseIR(ent);
    VectorCopy(other->s.origin, ent->s.origin);
    other->client->resp.chased = true;
    ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
    ent->client->ps.pmove.pm_type = PM_FREEZE;
    return true;
  }

  return false;  //bad luck, nobody alive on team. Stay looking from the spawn
}

void ChaseTeamStart(edict_t*player)
{
  int team = player->client->resp.team;
  if (!team) return;
  
  if (player->client->resp.class_type != CLASS_OBSERVER) {
    pmenuhnd_t m = player->client->menu;
    int scores = player->client->showscores;
    int pic = player->client->ps.stats[STAT_MENU];

    PutClientInServer(player, CLASS_OBSERVER);

    player->client->menu = m;
    player->client->showscores = scores;
    player->client->ps.stats[STAT_MENU] = pic;
    if (m.entries) PMenu_Update(player);
    
    if (player->client->resp.team) {
      player->flags |= FL_DEAD_OBSERVER;
      player->client->chase_view = 0;
    }
  }
  
  player->client->chase_target = player;
  player->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
  player->client->ps.pmove.pm_type = PM_FREEZE;
  StopChasingCurrentTarget(player);
}



qboolean ChaseStart(edict_t*player, edict_t*other)
{
  edict_t*loop = other;
  while (other->client->chase_target) {
    other = other->client->chase_target;
    if (other == player) return false;
  }

  other->client->resp.chased = true;
  player->client->chase_target = other;
  player->client->chase_view = 0;
  player->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
  player->client->ps.pmove.pm_type = PM_FREEZE;

  return true;
}

void ChaseStop(edict_t*player)
{
  player->client->chase_target = NULL;
  player->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
  player->client->ps.fov = 90;
}

edict_t*GetChasedTargetPlayer(edict_t*ent)
{
  if (ent->client->resp.class_type == CLASS_OBSERVER) {
    edict_t*loop = ent;
    if (!ent->client->chase_target) return NULL;

    while (ent->client->chase_target) {
      ent = ent->client->chase_target;
      if (ent == loop) return loop;
    }
  
    return ent;
  }
  
  return NULL;
}




edict_t *SelectNearestDeathmatchSpawnPoint (int team, edict_t *ent);

void StopChasingCurrentTarget(edict_t*player)
{
  int team = player->client->resp.team;

  edict_t* old = player->client->chase_target;
  ChaseNext(player);
  if (player->client->chase_target != old) return;
  
  if (!team) {
    JoinTeam0(player);
    return;
  }

//Couldn't chase a teammate, drop on a friendly spawn
  edict_t*tele = SelectNearestDeathmatchSpawnPoint(team, player);
  if (!tele) {
    edict_t*r = NULL;
    int n = -1;
    char*name = (team==TEAM_HUMAN)?"info_player_deathmatch":"monster_cocoon";
    while (r = G_Find(r, FOFS(classname), name)) {
      int v = randomMT()&31;
      if (v > n) {
        n = v;
        tele = r;
      }
    }
  }
  if (tele)
    VectorCopy(tele->s.origin, player->s.origin);
  player->movetype = MOVETYPE_NONE;
  player->client->chase_target = NULL;
  UpdateChaseIR(player);
}


void ChaseNext(edict_t *ent)
{
        edict_t *e;
        int team = ent->client->resp.team;
        
        if (StartChaseFromSpawn(ent)) return;

        if (!ent->client->chase_target)
                return;

        e = ent->client->chase_target;
        do {
                e++;
                if (!e->client)
                        e = g_edicts+1;
                if (!e->inuse)
                        continue;
                if (team && e->client->resp.team != team)
                        continue;
                if (e->solid != SOLID_NOT)
                        break;
        } while (e != ent->client->chase_target);
        
        ent->client->chase_target = e;
        e->client->resp.chased = true;
        UpdateChaseIR(ent);
}

//FIXME: when in chase overhead mode, if target is looking down and jumps, view
//       clips out of the map if their head touches the ceiling.

//FIXME: when chasing someone who disconnects chasecam should find another suitable
//       target and only drop to observer if one isn't found.

void WIP_UpdateChaseCam(edict_t *obs)
{
  int mode = obs->client->chase_view;
  edict_t*ent = obs->client->chase_target;
  int entclass = ent->client?ent->client->resp.class_type:-1;
  if (ent != obs->client->last_chase_target || entclass != obs->client->last_chase_class) {
    edict_t*old = obs->client->last_chase_target;
    obs->client->last_chase_target = ent;
    obs->client->last_chase_class = entclass;
    
    if (old && old->inuse && old->client && old != ent)
      playerconfigstring(obs, CS_PLAYERSKINS + old->s.number-1, va("%s\\%s", ent->client->pers.netname, classlist[old->client->resp.class_type].skin));

    if (ent && ent->inuse && ent->client)
      playerconfigstring(obs, CS_PLAYERSKINS + ent->s.number-1, va("%s\\none/none", ent->client->pers.netname));
  }
  
  if (!ent) return;

  int i;  
  vec3_t eye, target, angles;
  int viewheight = 0;
  
  VectorCopy(ent->s.origin, eye);
  viewheight = ent->viewheight;
  if (ent->client) {
    ent->client->resp.chased = true;
    VectorCopy(ent->client->v_angle, angles);
  } else
    VectorCopy(ent->s.angles, angles);

  VectorCopy(eye, obs->s.origin);
  VectorCopy(angles, obs->client->ps.viewangles);
  VectorCopy(angles, obs->client->v_angle);
  for (i=0 ; i<3 ; i++)
     obs->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(angles[i] - obs->client->resp.cmd_angles[i]);

}

void UpdateChaseCam(edict_t *ent)
{
        vec3_t  targorigin, angles, goal, goaldir, temp;
        vec3_t  forward, right;
        edict_t *targ;
        trace_t trace;
        int     i;

        targ = ent->client->chase_target;
        if (targ != ent->client->last_chase_target) {
                ent->client->last_chase_target = targ;

                // vary looking position depending on some classes
                switch (targ->client->resp.class_type) {
                        case CLASS_BREEDER:
                                ent->client->chase_height = 60;
                                ent->client->chase_dist = 100;
                                break;
                        case CLASS_STALKER:
                                ent->client->chase_height = 25;
                                ent->client->chase_dist = 80;
                                break;
                        case CLASS_ENGINEER:
                                ent->client->chase_height = 30;
                                ent->client->chase_dist = 80;
                                break;
                        case CLASS_MECH:
                                ent->client->chase_height = 50;
                                ent->client->chase_dist = 105;
                                break;
                        default:
                                ent->client->chase_height = 20;
                                ent->client->chase_dist = 75;
                                break;
                }

        }

        targ->client->resp.chased = true;

        VectorCopy(targ->s.origin, targorigin);

        targorigin[2] += targ->viewheight;

        // copy target view angles to temp variable
        VectorCopy(targ->client->v_angle, angles);

        ent->client->ps.fov = 90;

        switch (ent->client->chase_view){
        case 1:   //    chase
                // do a smooth movement feedback on pitch
                angles[PITCH] /= 2;

                // convert angles to vectors
                AngleVectors (angles, forward, right, NULL);

                // calc view pos
                VectorMA(targorigin, -ent->client->chase_dist, forward, goal);

                // up it
                goal[2] += ent->client->chase_height;

                // sub o from targorigin, creating a dir vector
                VectorSubtract(targorigin, goal, goaldir);

                break;
        case 2:  //     right side
                AngleVectors (angles, forward, right, NULL);
                VectorMA(targorigin, ent->client->chase_dist, right, goal);
                goal[2] += ent->client->chase_height;
                VectorSubtract(targorigin,goal,goaldir);
                break;
        case 3:  //     front side
                angles[PITCH] = 0;
                AngleVectors (angles, forward, right, NULL);
                VectorMA(targorigin, ent->client->chase_dist, forward, goal);
                goal[2] += ent->client->chase_height;
                VectorSubtract(targorigin,goal,goaldir);
                break;
        case 4:  //     left side
                AngleVectors (angles, forward, right, NULL);
                VectorMA(targorigin, -ent->client->chase_dist, right, goal);
                goal[2] += ent->client->chase_height;
                VectorSubtract(targorigin,goal,goaldir);
                break;
        default:
                // eyes
                AngleVectors (angles, forward, right, NULL);
                VectorMA(targorigin, 30, forward, goal);
                VectorSubtract (goal, targorigin, goaldir); // swapped

                ent->client->ps.fov = targ->client->ps.fov;
                break;
        }

        // trace from targorigin to final chase origin goal
        trace = gi.trace(targorigin, vec3_origin, vec3_origin, goal, targ, MASK_SOLID);

        // test for hit
        if (trace.fraction < 1) {
                // we hit something, need to do a bit of avoidance

                // take real end point
                VectorCopy (trace.endpos, goal);
                // real dir vector
                VectorSubtract(goal, targorigin, temp);
                // scale it back bit more
                VectorMA(targorigin, 0.9, temp, goal);
        }

        // someone set us up the target origin :)
        VectorCopy(goal, ent->s.origin);

        // move view
        vectoangles(goaldir, ent->client->ps.viewangles);
        vectoangles(goaldir, ent->client->v_angle);

        // deltas
        for (i=0 ; i<3 ; i++)
                ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);
}

void ChasePrev(edict_t *ent)
{
        edict_t *e;
        int team = ent->client->resp.team;

        if (StartChaseFromSpawn(ent)) return;

        if (!ent->client->chase_target)
                return;

        e = ent->client->chase_target;
        do {
                e--;
                if (!e->client)
                        e = g_edicts+game.maxclients;
                if (!e->inuse)
                        continue;
                if (team && e->client->resp.team != team)
                        continue;
                if (e->solid != SOLID_NOT)
                        break;
        } while (e != ent->client->chase_target);

        ent->client->chase_target = e;
        e->client->resp.chased = true;
	UpdateChaseIR(ent);
}

