//============================================================================
// g_ctf.c
// 
// CTF chase camera
// 
// Ripped from ctf source code, with changes.
//
// Copyright (c), 1999 The BatCave. All Rights Reserved. 
//============================================================================

#include "g_local.h"

void CTFChaseCam(edict_t *ent)
{
	int i;
	edict_t *e;

  // Can't chase if predator.
  if (ent->isPredator) {
    gi.centerprintf(ent, "Cannot enter chasecam mode as Predator\n") ;
    return ;
  }

  // Observer mode stuff.
  ent->movetype = MOVETYPE_NOCLIP;
  ent->solid = SOLID_NOT;
  ent->svflags |= SVF_NOCLIENT;
  ent->client->ps.gunindex = 0;
  gi.linkentity (ent);
		
  // Don't ask me why this is necesasry, as observer mode
  // doesn't use anything similar.
  ent->deadflag = DEAD_NO;
	
  // If we are chasing someone, stop.
  if (ent->client->chase_target) {
    ent->client->chase_target = NULL;
    PMenu_Close(ent);
    Cmd_Observe_f(ent);	//switch to observer 
    return;
  }

  // For all clients.
  for (i = 1; i <= maxclients->value; i++) {
    e = g_edicts + i;

    // Only consider client in use, or who are not also in chase mode,
    // or who are not observing.
    if (e->inuse && e->solid != SOLID_NOT) {
      ent->client->chase_target = e;
      PMenu_Close(ent);
      ent->client->update_chase = true;
      break;
    }
  }
}

static void loc_buildboxpoints(vec3_t p[8], vec3_t org,
			       vec3_t mins, vec3_t maxs)
{
  VectorAdd(org, mins, p[0]);
  VectorCopy(p[0], p[1]);
  p[1][0] -= mins[0];
  VectorCopy(p[0], p[2]);
  p[2][1] -= mins[1];
  VectorCopy(p[0], p[3]);
  p[3][0] -= mins[0];                                                    
  p[3][1] -= mins[1];
  VectorAdd(org, maxs, p[4]);
  VectorCopy(p[4], p[5]);
  p[5][0] -= maxs[0];
  VectorCopy(p[0], p[6]);
  p[6][1] -= maxs[1];
  VectorCopy(p[0], p[7]);
  p[7][0] -= maxs[0];
  p[7][1] -= maxs[1];
}                                                                              

static qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
{
  trace_t trace;
  vec3_t  targpoints[8];
  int i;
  vec3_t viewpoint;

  // bmodels need special checking because their origin is 0,0,0
  if (targ->movetype == MOVETYPE_PUSH)
    return false; // bmodels not supported                         
    
  loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);

  VectorCopy(inflictor->s.origin, viewpoint);
  viewpoint[2] += inflictor->viewheight;

  for (i = 0; i < 8; i++) {
    trace = gi.trace (viewpoint, vec3_origin, vec3_origin,
		      targpoints[i], inflictor, MASK_SOLID);
    if (trace.fraction == 1.0)
      return true;
  }                                                                      
	
  return false ;
}

/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/

static edict_t *loc_findradius (edict_t *from, vec3_t org, float rad)
{
  vec3_t  eorg;
  int             j;

  if (!from)
    from = g_edicts;
  else
    from++;

  for ( ; from < &g_edicts[globals.num_edicts]; from++)
    {
      if (!from->inuse)
	continue;
#if 0
      if (from->solid == SOLID_NOT)
	continue;
#endif
      for (j=0 ; j<3 ; j++)
	eorg[j] = org[j] - (from->s.origin[j] +
			    (from->mins[j] + from->maxs[j])*0.5);
      if (VectorLength(eorg) > rad)
	continue;
      return from;
    }
  
  return NULL;
}


/*
======================================================================

SAY_TEAM             

======================================================================
*/

// This array is in 'importance order', it indicates what items are
// more important when reporting their names.
struct {
        char *classname;
        int priority;
} loc_names[] =
{
  {       "item_flag_team1",                      1 },
  {       "item_flag_team2",                      1 },
  {       "item_quad",                            2 },
  {       "item_invulnerability",         2 },
  {       "weapon_bfg",                           3 },
  {       "weapon_railgun",                       4 },
  {       "weapon_rocketlauncher",        4 },
  {       "weapon_hyperblaster",          4 },
  {       "weapon_chaingun",                      4 },
  {       "weapon_grenadelauncher",       4 },
  {       "weapon_machinegun",            4 },
  {       "weapon_supershotgun",          4 },
  {       "weapon_shotgun",                       4 },
#if XATRIX
  {       "weapon_boomer",                       4 },
  {       "weapon_phalanx",                       4 },
  {       "ammo_trap",                       4 },
#endif
  {       "item_power_screen",            5 },
  {       "item_power_shield",            5 },
  {       "item_armor_body",                      6 },
  {       "item_armor_combat",            6 },
  {       "item_armor_jacket",            6 },
  {       "item_silencer",                        7 },
  {       "item_breather",                        7 },
  {       "item_enviro",                          7 },
  {       "item_adrenaline",                      7 },
  {       "item_bandolier",                       8 },
  {       "item_pack",                            8 },
  { NULL, 0 }
};


static void CTFSay_Team_Location(edict_t *who, char *buf)
{
  edict_t *what = NULL;
  edict_t *hot = NULL;
  float hotdist = 999999, newdist;
  vec3_t v;
  int hotindex = 999;
  int i;
  gitem_t *item;
  int nearteam = -1;
  edict_t *flag1, *flag2;
  qboolean hotsee = false;
  qboolean cansee;
  
  while ((what = loc_findradius(what, who->s.origin, 1024)) != NULL) {

    // find what in loc_classnames
    for (i = 0; loc_names[i].classname; i++)
      if (strcmp(what->classname, loc_names[i].classname) == 0)
	break;

    if (!loc_names[i].classname)
      continue;

    // something we can see get priority over something we can't
    cansee = loc_CanSee(what, who);
    if (cansee && !hotsee) {
      hotsee = true;
      hotindex = loc_names[i].priority;
      hot = what;
      VectorSubtract(what->s.origin, who->s.origin, v);
      hotdist = VectorLength(v);
      continue;
    }

    // if we can't see this, but we have something we can see, skip it
    if (hotsee && !cansee)
      continue;

    if (hotsee && hotindex < loc_names[i].priority)
      continue;

    VectorSubtract(what->s.origin, who->s.origin, v);
    newdist = VectorLength(v);
    if (newdist < hotdist ||
	(cansee && loc_names[i].priority < hotindex)) {
      hot = what;
      hotdist = newdist;
      hotindex = i;
      hotsee = loc_CanSee(hot, who);
    }
  }

  if (!hot) {
    strcpy(buf, "nowhere");
    return;
  }

  if ((item = FindItemByClassname(hot->classname)) == NULL) {
    strcpy(buf, "nowhere");
    return;
  }
  
  // in water?
  if (who->waterlevel)
    strcpy(buf, "in the water ");
  else
    *buf = 0;

  // near or above
  VectorSubtract(who->s.origin, hot->s.origin, v);
  if (fabs(v[2]) > fabs(v[0]) && fabs(v[2]) > fabs(v[1]))
    if (v[2] > 0)
      strcat(buf, "above ");
    else
      strcat(buf, "below ");
  else
    strcat(buf, "near ");

    strcat(buf, "the ");

    // ***** Start of NH Changes *****
    // If IR disabled and it found the IR goggles, change to Combat Armor
    if ( !strcmp(item->pickup_name, "IR goggles") && !getIREffectTime())
    	strcat(buf, "Combat Armor");
    else
    // ***** End of NH Changes *****
    strcat(buf, item->pickup_name);
}

static void CTFSay_Team_Sight(edict_t *who, char *buf)
{
  int i;
  edict_t *targ;
  int n = 0;
  char s[1024];
  char s2[1024];


  *s = *s2 = 0;
  for (i = 1; i <= maxclients->value; i++) {
    targ = g_edicts + i;
    if (!targ->inuse ||
	targ == who ||
	!loc_CanSee(targ, who))
      continue;
    if (*s2) {
      if (strlen(s) + strlen(s2) + 3 < sizeof(s)) {
	if (n)
	  strcat(s, ", ");
	strcat(s, s2);
	*s2 = 0;
      }
      n++;
    }
    strcpy(s2, targ->client->pers.netname);
  }
  if (*s2) {
    if (strlen(s) + strlen(s2) + 6 < sizeof(s)) {
      if (n)
	strcat(s, " and ");
      strcat(s, s2);
    }
    strcpy(buf, s);
  } else
    strcpy(buf, "no one");
}

void CTFSay_Team(edict_t *who, char *msg)
{
  char outmsg[1024];
  char buf[1024];
  int i;
  char *p;
  edict_t *cl_ent;
  gclient_t *cl; // ***** NH Change ***** for flood protection
  
  outmsg[0] = 0;

  // Can't do this if pred, observing, or dead.
  
  if (*msg == '\"') {
    msg[strlen(msg) - 1] = 0;
    msg++;
  }
  
  for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 1; msg++) {
    if (*msg == '%') {
      switch (*++msg) {
      case 'l' :
      case 'L' :
	CTFSay_Team_Location(who, buf);
	strcpy(p, buf);
	p += strlen(buf);
	break;
      case 'n' :
      case 'N' :
	CTFSay_Team_Sight(who, buf);
	strcpy(p, buf);
	p += strlen(buf);
	break;
	
      default :
	*p++ = *msg;
      }
    } else
      *p++ = *msg;
  }
  *p = 0;

// ***** Start of NH Changes ***** Added in flood control from g_cmds.c

	if (flood_msgs->value) {
		cl = who->client;

        if (level.time < cl->flood_locktill) {
			gi.cprintf(who, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(who, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

// ***** End of NH Changes *****

  
  for (i = 0; i < maxclients->value; i++) {
    cl_ent = g_edicts + 1 + i;
    if (!cl_ent->inuse)
      continue;

      gi.cprintf(cl_ent, PRINT_CHAT, "(%s): %s\n",
		 who->client->pers.netname, outmsg);
  }
}

    
