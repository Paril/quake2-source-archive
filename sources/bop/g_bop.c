// Everything in here was written by Matt Ownby
// for the Balance of Power (aka High-Ping Heaven) mod

// Other files modified are:

// p_client.c
// g_combat.c
// p_view.c
// p_weapon.c
// g_local.h
// g_svcmds.c
// g_cmds.c
// g_save.c

// To find lines modified thruout source, search for "MATT"

#include "g_local.h"
#include "bop_fnc.h"
#include "bop_var.h"
#include "bop_mod.h"

#ifdef BOP_FREEZE
  extern qboolean just_won;	// freeze tag specific code...
#endif

int anti_lpb(edict_t *attacker, edict_t *target, int damage) {

// This function adjusts the amount of damage that a HPB does to an LPB
// It is called when a client takes damage

  int delta_ping;
  float x, adjust;
  gclient_t *aclient, *tclient;

  if (laghelp_str == 0.0)
	return(damage);
  if (!attacker->client)	// if attacker is not a player
	return(damage);		// don't adjust at all...
  if (OnSameTeam(target, attacker))	// if they're on the same team..
	return(damage);
  if (!target->client)		// if target is not a player
	return(damage);		// (if that's even possible)

  aclient = attacker->client;
  tclient = target->client;

  delta_ping = aclient->ping - tclient->ping; // difference in pings of attacker & target

  if (delta_ping<min_delta_ping)
	return(damage);

  if (delta_ping>max_delta_ping)
	delta_ping = max_delta_ping;

  x = ((float)delta_ping/max_delta_ping)*(NINETYDEGS);
  adjust = (laghelp_str*sin(x))+1;

  if (bop_verbose)
  {
    gi.cprintf(attacker, PRINT_HIGH, "Your damage increased from %d ", damage);
    gi.cprintf(target, PRINT_HIGH, "Damage to you increased from %d ", damage);
  }
  damage *= adjust;
  if (bop_verbose)
  {
    gi.cprintf(attacker, PRINT_HIGH, " to %d [1:%f]\n", damage, adjust);
    gi.cprintf(target, PRINT_HIGH, " to %d [1:%f]\n", damage, adjust);
  }

  aclient->strength_framenum = level.framenum + bop_flareup;
    // have them flareup a nice blue...
  
  return(damage);

}


int pro_hpb(edict_t *attacker, edict_t *target, int damage) {

// This function adjusts the amount of resistance that an HPB gains against
// an LPB.  It is called when a client takes damage

  int delta_ping, dmg;
  float x, adjust;
  gclient_t *aclient, *tclient;
  char s[81];

  if ((laghelp_rst == 0) || (!attacker->client)
     || (!target->client) || (OnSameTeam(target, attacker)) )
	return(damage);

  aclient = attacker->client;
  tclient = target->client;

  delta_ping = tclient->ping - aclient->ping; // difference in pings of attacker & target

  if (delta_ping<min_delta_ping)
	return(damage);

  if (delta_ping>max_delta_ping)
	delta_ping = max_delta_ping;

  x = ((float) delta_ping/max_delta_ping)*(NINETYDEGS);
  adjust = (laghelp_rst*sin(x))+1;

  if (bop_verbose)
  {
    gi.cprintf(target, PRINT_HIGH, "Damage to you reduced from %d ",damage);
    gi.cprintf(attacker, PRINT_HIGH, "Damage to your target reduced from %d ", damage);
  }
  damage /= adjust;
  sprintf(s, " to %d [%f:1]\n", damage, adjust);
  if (bop_verbose)
  {
    gi.cprintf(target, PRINT_HIGH, "%s", s);
    gi.cprintf(attacker, PRINT_HIGH, "%s", s);
  } 

  if ( (!adjust_resistance(attacker, target, &dmg)) && (damage<dmg) ) {
	if (bop_verbose)
	  gi.cprintf(attacker, PRINT_HIGH, "You hit for %d damage\n", dmg);
	return(dmg);
  }
  // The above lines do the following as of 6/4/98:
  // - Check to see if enemy is using a railgun
  // - Check to see whether railgun damage is under 100
  // - Make sure railgun does AT LEAST 100 points of damage
  // (Note, no flareup with the railgun under these circumstances)

  tclient->resistence_framenum = level.framenum + bop_flareup;
  // Make them flash red to indicate lag help...

  return(damage);

}


void protecthpb(edict_t *attacker, edict_t *ent) {

// Function: Check to see if they are lagged out and need to get lag sickness

  gclient_t *client;

  if (!ent->client)
	return;

  if ((ent == attacker) && (no_suicide_protect))
	return;	// no protection if they shoot themselves...

  if ((!attacker->client) && (no_slag_protect))
	return;	// no protection if they fall in slag...

  client = ent->client;

  if (client->invincible_framenum > level.framenum)
	return;	// if they already have pent, don't give it to them

  if (client->protecthpb_framenum > level.framenum)
	return;	// they've already got lag sickness!

  if (lagged_out(ent)) {	// if they're lagged out, give them L.S.
    client->protecthpb_framenum = level.framenum + bop_deadtime;
    client->invincible_framenum = level.framenum + bop_getawaytime;
#ifdef BOP_CTF
    CTFDeadDropFlag(ent);	// lagged out players drop the flag
//    CTFDeadDropTech(ent);	// let them keep their tech hehe...
#endif
#ifdef BOP_FREEZE
	just_won = false;	// no lagged players holding up victory
#endif
    if (bopsickremove) {
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent-g_edicts);
	gi.WriteByte(MZ_LOGOUT);
	gi.multicast(ent->s.origin, MULTICAST_PVS);
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
//	gi.unlinkentity(ent);	// take us out of the scan algorithms
	if (bop_invisible)
	  gi.bprintf(PRINT_HIGH, "%s overflowed\n", ent->client->pers.netname);
	else
	  gi.bprintf(PRINT_HIGH, "%s lagged out\n", ent->client->pers.netname);
	ent->client->bop_i[0] = 1;	// mark them as lagged out...
    }
  }	

}

qboolean lagged_out(edict_t *ent) {

  int lod;
  gclient_t *client;

// Pre-Condition: ent->client exists (otherwise it will crash)

// Here is the pre-Quake 2 v3.15 code which only looks at ping

// Unfortunately with the introduction of v3.15, when a person lags out
// their ping become frozen instead of rising above 999.  This makes
// determining whether a client is lagged out more difficult and more
// error prone.

  client = ent->client;

  if (ent->health < 1)	// Check to see if they're dead...
	return(0);	// If so, we don't do any lag sickness stuff...

	// they don't get lag sickness in their 1st second of respawn...
  if (level.time - ent->client->respawn_time < 1.0) {
	client->last_recorded_ping = ent->client->ping;
        client->lag_detect_framenum = level.framenum; // reset-it
	return(0);
  }

  if (client->ping == 0)	// If their ping is 0, they have cl_nodelta
				// enabled, which sucks for them =)
				// we'll give them 3 nice warnings...
				// into the game...
  {
	switch ((level.framenum - client->resp.enterframe) / 10) {
	  case 30:	// 30 seconds...
	  case 60:	// 1 minute.. you get the idea...
	  case 90:
	    if (lag_msg(ent))
		gi.centerprintf(ent, "It is advised that you type\ncl_nodelta 0\nat the console!  You will be\nnotified three times");
	    break;
	}
    return(0);
  }

  if (client->ping > lag_threshold) {
        if (bop_verbose)
                gi.cprintf(NULL, PRINT_HIGH, "Client's ping has exceeded threshold\n");
	return(1);
  }

// If their ping does happen to be over 999, we will still give them
// lag sickness but we will also perform some additional checks..

// The new code will watch a client's ping and will declare them lagged
// if their ping is frozen for a administrator-defined length of time.

  lod = lagged_out_detect;
  if (client->ping<100)    // ie, if they're an LPB
        lod *=3;                // perform a check 3 times as long
                                // the lower the ping the more likely
                                //  it is that the ping won't fluctuate

	// LPB's can get phone jack at a LAN party is some idiot unplugs
	// their RJ-45 cable from the hub <grin>.. ok, so it's a long shot

  if (client->last_recorded_ping == ent->client->ping) {
        if (level.framenum - ent->client->lag_detect_framenum > lod) {
	  return(1);	// They have been at same ping for too long
				// They're lagged out...
	}
  }

  else {	// their ping has fluctuated...
	client->last_recorded_ping = ent->client->ping;
        client->lag_detect_framenum = level.framenum; // reset-it
  }

  return(0);

}

qboolean pent_from_lag(edict_t *ent) {

// Pre-Condition: ent->client must exist

  if ((ent->client->invincible_framenum > level.framenum) &&
	(ent->client->protecthpb_framenum > level.framenum))
	return(1);
  return(0);

// Post-Condition: Returns 1 if player got the pent due to lag

}

int lag_sickness(edict_t *ent) {

// Pre-Condition: ent->client must exist

  float difference, f;

  difference = ent->client->protecthpb_framenum - level.framenum;
  if (difference > lagsicktime) {
        f = (difference - lagsicktime) / 10;
        if (f<1 && f)
          return(1);    // just to make sure we return 1 second instead of 0
        else
          return(f);
  }

//  if (bop_verbose)
//    if (lag_msg(ent)) {
//      gi.cprintf(NULL, PRINT_HIGH, "Difference is %d\nPhpb is %d\nFn is %d\nLagsicktime is %d\n",
//      (int) difference, (int) ent->client->protecthpb_framenum,
//      (int) level.framenum, (int) lagsicktime);
//    }

  return(0);	// no lag sickness! Frag away! >8)

// Post-Condition: Returns # of seconds remaining of lag sickness

}

qboolean lag_msg(edict_t *ent) {

// Pre-Condition: ent->client must exist or we cra$h

// Purpose: To make sure client can't flood the server
// returns true if it's okay to send client a message
// returns false if they've had a message too recently

  if (fabs(ent->client->lagmsg_framenum - level.framenum) > msg_interval) {
	ent->client->lagmsg_framenum = level.framenum;
	return(1);
  }

  return(0);

// Post-Condition: Returns 1 if it's been long enough since the
// last message...

// (ie, don't flood the poor fools)

}

qboolean short_lag_msg(edict_t *ent) {

// Pre-Condition: ent->client must exist or we cra$h

// Purpose: To make sure client can't flood the server
// returns true if it's okay to send client a message
// returns false if they've had a message too recently

  if (fabs(ent->client->lagmsg_framenum - level.framenum) > short_msg_interval) {
	ent->client->lagmsg_framenum = level.framenum;
	return(1);
  }

  return(0);

// Post-Condition: Returns 1 if it's been long enough since the
// last message...

// (ie, don't flood the poor fools)

}

qboolean lag_sick_notice(edict_t *ent) {

// called from p_weapon.c when client fires or throws a grenade

// returns true if they have lag sickness (ie they can't fire!)
// returns false if it's okay for them to fire

	int i;

	i = lag_sickness(ent);
	if (i || pent_from_lag(ent)) {
          if (lag_msg(ent)) {
            gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/secret.wav"), 1, ATTN_NORM, 0);
	    if (pent_from_lag(ent)) {
	      if (ent->client->bop_i[0])
		gi.centerprintf(ent, "You've got lag sickness!\n\nYou are frozen until you\n\nstop lagging out...");
	      else
                gi.centerprintf(ent,"You've got lag sickness!\n\nWeapons are jammed...\n\nFleeing would be a good option...\n");
	    }
	    else
              gi.centerprintf(ent,"You've got lag sickness\nfor %d more seconds\n", i);
	  }
	  return(true);
        }

	return(false);

}

void do_bop_frame(edict_t *ent) {

// called from ClientBeginServerFrame

	gclient_t *client;
	edict_t *other;
	qboolean blocked;
	int i;

	client=ent->client;

	if (pent_from_lag(ent)) {
	  if (lagged_out(ent)) {	// if they're still lagging...
		client->invincible_framenum++;
		client->protecthpb_framenum++;
	  }
	  else if (bopsickremove) {
	    if (short_lag_msg(ent)) {
		i = (client->invincible_framenum - level.framenum)/10;
		gi.centerprintf(ent, "You lagged out but were saved\n\nGet ready to re-enter the game\n\n%i", i);
		// give them the countdown hehe...
	    }
	  }
	  return;
	}

	if (ent->client->bop_i[0]) // ie, if they're an observer
	{
	  other = NULL;
	  blocked = 0;
	  while ((other = findradius(other, ent->s.origin, 80)) != NULL) {
	    if (!strcmp(other->classname, "player") && other->client) {
			blocked = 1;
			break;
	    }
	  }
	  if (!blocked) {
	    KillBox(ent);	// telefrag just in case =)
//		  gi.linkentity(ent);	// get us back in the mix!
	    ent->svflags &= ~SVF_NOCLIENT;
	    ent->movetype = MOVETYPE_WALK;
	    ent->solid = SOLID_BBOX;
	    gi.WriteByte(svc_muzzleflash);
	    gi.WriteShort(ent-g_edicts);
	    gi.WriteByte(MZ_LOGIN);
	    gi.multicast(ent->s.origin, MULTICAST_PVS);
	    gi.bprintf(PRINT_HIGH, "%s re-entered the game\n", ent->client->pers.netname);
 	    client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	    client->ps.pmove.pm_time = 14;	// little pause..
	    client->bop_i[0] = 0;	// back in the fray...
	  }
	  else {
	    if (lag_msg(ent))
		gi.centerprintf(ent, "%s is in your way!\n", other->client->pers.netname);
	    client->invincible_framenum++;
	    client->protecthpb_framenum++;
	  }
	  return;
	}	// end if they're detected to be an observer w/ sickness

	if (bopmanageobservers && (ent->svflags & SVF_NOCLIENT) 
	    && (ent->movetype != MOVETYPE_NOCLIP)) {
		ent->svflags &= ~SVF_NOCLIENT;
		// invisible men over from last connection?
		// fix that...
	}

}

void do_bop_client_frame(edict_t *ent, usercmd_t *ucmd) {

// called from ClientThink...

  if ((pent_from_lag(ent) && lagged_out(ent)) || ent->client->bop_i[0]) {
	ucmd->forwardmove = 0;	// don't let them move (stop cheaters)
	ucmd->sidemove = 0;
	ucmd->upmove = 0;
  }

}

void do_greeting(edict_t *ent) {

// Pre-Conditions: ent->client must exist

  char stage[81], gmstr[81];

  if (!BOP_STAGE)
	strcpy(stage, "Alpha Release (Bugs may exist)");
  else if (BOP_STAGE==1)
	strcpy(stage, "Beta Release");
  else
	strcpy(stage, "Public Release");

  get_mode_name (gmstr);

  gi.centerprintf(ent, "Server running Balance of Power %.2f\n\n%s\n\nBOP set to %s mode\n\nFor help type 'cmd bophelp'\n\nLatest stable version at:\nwww.planetquake.com/bop/", BOP_VERSION, stage, gmstr);

}

void get_mode_name (char *gmstr) {

// Puts the game mode name in gmstr

  switch(game_mode) {
	case 0:	// normal
		if ((laghelp_str == n_laghelp_str) && (laghelp_rst == n_laghelp_str)
		   && (max_delta_ping == n_max_delta_ping)
		   && (min_delta_ping == n_min_delta_ping))
		  strcpy(gmstr, "NORMAL");
		else
		  strcpy(gmstr, "Customized NORMAL");
		break;
	case 1:	// expert
		if ((laghelp_str == x_laghelp_str) && (laghelp_rst == x_laghelp_rst)
		    && (max_delta_ping == x_max_delta_ping)
		    && (min_delta_ping == x_min_delta_ping))
		  strcpy(gmstr, "EXPERT");
		else
		  strcpy(gmstr, "Customized EXPERT");
		break;
	case 2: // Tournament
		if ((!laghelp_str) && (!laghelp_rst))
		  strcpy(gmstr, "ANTI PHONE-JACK");
		else
		  strcpy(gmstr, "Cust. PHONE-JACK");
		break;
	case 3: // Unknown
		strcpy(gmstr, "UNKNOWN (Bug)");
		break;
  }

}

void bopinit() {

// Pre-Conditions: None, really
// Function called from g_save.c
// Purpose: To initialize the bop cvar so that Gamespy will detect BOP
//	servers
//	Also, to read the bop.cfg file and setup any parameters found

#define BI_LENGTH 81
#define BI_FILENAMES 4

  cvar_t *gamedircv;
  char *gamedir;
  char s2[10], s1[BI_LENGTH], cmd2[BI_LENGTH], path[81];
  char *filename[BI_FILENAMES] = { "bop.ini", "bop.cfg",
#ifndef AMIGA
			"./bop/bop.ini", "./bop/bop.cfg" };
#else
			"bop/bop.ini","bop/bop.cfg"};
#endif

  char *s;
  int ch, i, j, n;
  FILE *F;	// old file handling, but it's simple and we only do it
		// once so speed is not a concern

  sprintf(s2, "v%.2f", BOP_VERSION);
  gi.cvar("bop", s2, CVAR_SERVERINFO);	// set us up for gamespy tabs!

  gamedircv = gi.cvar("gamedir", "bop", CVAR_SERVERINFO);	// get our dir
  gamedir = gamedircv->string;

  strcpy(path, filename[0]);
  s = path;
  n = 0;
  while (s != NULL) {
   F = fopen(s, "r");	// open for read only...
   if (F) {
    gi.cprintf(0, PRINT_HIGH, "%s file found, processing...\n", s);
    ch = 0;
    i = 0;
    while (ch !=EOF) {	// ch = EOF at end of file OR an error...
      ch = getc(F);	// get a character...
      if ((ch == 10) || (ch == 13) || (i>BI_LENGTH-2)) {
	s1[i] = 0; // terminate the string and get ready to process...
	if (i) {	// if the line isn't blank...
	  for (i=0; i<strlen(s1); i++)
	    if (s1[i]==' ')
		break;
	  if (s1[0] != '/' && s1[1] != '/') {	// check for, and ignore comments
	    if (i == strlen(s1))
		process_bop_cmds(s1, NULL);	// no second arg...
            else {
		j = 0;
		s1[i++] = 0;	// terminate the s1 string...
		while (s1[i])
		  cmd2[j++] = s1[i++];
		cmd2[j] = 0;	// terminate the cmd2 string...
		process_bop_cmds(s1, cmd2);
	    }
	  }
        }
	i = 0;
      }
      else
        s1[i++] = ch;	// throw it in our string...
    };
    fclose(F);	// end of file, we're done...
    s = NULL;
   }
   else {
    gi.cprintf(0, PRINT_HIGH, "%s file not found\n", s);
    if (strncmp(path, filename[n], strlen(filename[n]))==0) {
	// ie, if we tried without the path...
	strcpy(path, gamedir);
	strcat(path, "/");
	strcat(path, filename[n]);
	// then try with the path this time...
    }
    else if (strncmp(path, gamedir, strlen(gamedir))==0) {
	// ie, if we tried it WITH the path...
	n++;
	if (n<=BI_FILENAMES-1) {
	  strcpy(path, filename[n]);	// start over with a new file
	}
	else {
	  s = NULL;
	  gi.cprintf(0, PRINT_HIGH, "No bop.ini or bop.cfg file found, using defaults\n");
	}
    }
    else {
	s = NULL;
	gi.cprintf(0, PRINT_HIGH, "BUG IN bopinit() PLEASE REPORT TO bop@planetquake.com\n");
    }
   }
  };

// Post-Conditions: BOP cvar set so our Gamespy tab will work nicely

}

void check_effects(edict_t *ent) {

  if (bop_invisible)
	return;	// no fx if the mod is invisible
  if (ent->client->strength_framenum > level.framenum)
	ent->s.effects |= EF_QUAD;
  if (ent->client->resistence_framenum > level.framenum)
	ent->s.effects |= EF_PENT;
//  if (pent_from_lag(ent)) {
//	if (ent->s.effects & EF_PENT)	// if the player is red from pent
//	  ent->s.renderfx |= RF_TRANSLUCENT;	// make him a ghost too
//  }

	// the ghosting feature only works in GL and doesn't show up very
	// well; I'm still deciding what I want to do with it...

}

qboolean bop_showicon(edict_t *ent) {

// Display the quad/pent in the corner if they are getting lag help

  gclient_t *client;

  client = ent->client;

  if (client->strength_framenum > level.framenum)
    client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex("p_quad");
  else if (client->resistence_framenum > level.framenum)
    client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex("p_invulnerability");
  else
    return(false);

  return(true);

}

void check_4_lagpent(edict_t *attacker, edict_t *target) {

// Purpose of function: To alert people of a lag sick player that
//	they may be attacking

// Pre-Conditions: None that aren't obvious

  if (!attacker->client)        // if attacker is not another player
        return;

  if (bop_invisible)
        return;                 // don't tell them if bop_invis is enabled

  if (pent_from_lag(target) && lag_msg(attacker)) {
    gi.sound(attacker, CHAN_VOICE, gi.soundindex("misc/secret.wav"), 1, ATTN_NORM, 0);
    if (!target->client->bop_i[0]) {  // if target didn't become an observer
      if (lagged_out(target))
        gi.centerprintf(attacker, "%s is frozen!\n", target->client->pers.netname);
      else
        gi.centerprintf(attacker, "%s has lag sickness\n\n(type 'bophelp' for more info)", target->client->pers.netname);
    }
  }

// Post-Conditions: If target has lag pent, informs attacker of such

}

qboolean adjust_resistance(edict_t *attacker, edict_t *target, int *dmg) {

// Returns 1 if the user should get some resistance help
// Returns 0 if user is on his/her own
// d returns the normal suggested damage value (100 for railgun)

// Pre-Conditions: attacker and target MUST exist

// NOTE: This only takes into account the weapon of the attacker
// If the attacker is using a railgun, no resistance is granted

  char *weap;

  if (attacker->client->pers.weapon)
	weap = attacker->client->pers.weapon->classname;
  else
	weap = "";

  *dmg = 100;	// just in case there is a bug, give it a default 100

  if (strcmp(weap,"weapon_railgun")==0) {
	*dmg = 100;	// normal railgun damage is 100...
	if (bop_verbose) {
	  gi.cprintf(target, PRINT_HIGH, "Damage to you will be at least %d (railgun)\n", *dmg);
	  gi.cprintf(attacker, PRINT_HIGH, "Damage to your target at least %d (railgun)\n", *dmg);
	}
	return(0);
  }
  return(1);	// if they're not getting railed, they get lag help

}

void reset_bop_counters(edict_t *ent) {

// called from player_die() inside p_client.c

  ent->client->strength_framenum = 0;
  ent->client->resistence_framenum = 0;
  ent->client->protecthpb_framenum = 0;	// no lag sickness when dead
//  ent->client->lag_detect_framenum = level.framenum;
  if (bopsickremove) {
	ent->svflags &= ~SVF_NOCLIENT;	// make sure they're put back to normal
//	ent->movetype = MOVETYPE_WALK;
//	ent->solid = SOLID_BBOX;
  }

}

void bophelp(edict_t *ent) {

  if (!ent->client->bopmenu) {
    ent->client->bopmenu = 1;	// start them with help screen #1
    ent->client->showscores = true;
    refresh_bop_menu(ent);
    gi.unicast(ent, true);
  }
  else
    adv_bop_menu(ent);

}

void refresh_bop_menu(edict_t *ent) {

  char string[1024];
  char s[81], s1[81];

  if ((bop_invisible) && (ent->client->bopmenu==1))
	ent->client->bopmenu = 2;	// skip first menu if it's invis...
  strcpy(string, "xv 32 yv 8 picn inventory yv 32 xv 64 string2 \"Balance of Power Help\" ");
  if (ent->client->bopmenu==1) {
   strcat(string, "yv 48 xv 64 string2 \"The Rules:\" ");
   strcat(string, "yv 64 xv 64 string \"High ping players may\" ");
   strcat(string, "yv 72 xv 64 string \"inflict more damage and\" ");
   strcat(string, "yv 80 xv 64 string \"absorb extra damage when\" ");
   strcat(string, "yv 88 xv 64 string \"they fight LPB's.\" ");
   strcat(string, "yv 104 xv 64 string \"HPB's flash red and blue\" ");
   strcat(string, "yv 112 xv 64 string \"to indicate resistance\" ");
   strcat(string, "yv 120 xv 64 string \"and strength adjustments\" ");
   strcat(string, "yv 128 xv 64 string \"respectively.\" ");
   strcat(string, "yv 144 xv 104 string2 \"--Press Enter--\" ");
  }
  else if (ent->client->bopmenu==2) {
    strcat(string, "yv 48 xv 64 string2 \"Lag Sickness:\" ");
    strcat(string, "yv 64 xv 64 string \"If you experience unusual\" ");
    strcat(string, "yv 72 xv 64 string \"lag, BoP will protect you\" ");
    strcat(string, "yv 80 xv 64 string \"from enemy attacks for a\" ");
    strcat(string, "yv 88 xv 64 string \"certain period of time.\" ");
    strcat(string, "yv 104 xv 64 string \"The price of this shield\" ");
    strcat(string, "yv 112 xv 64 string \"is a condition called lag\" ");
    strcat(string, "yv 120 xv 64 string \"sickness which prevents\" ");
    strcat(string, "yv 128 xv 64 string \"weapon usage temporarily.\" ");
    strcat(string, "yv 144 xv 104 string2 \"--Press Enter--\" ");
  }
  else {
    strcat(string, "yv 48 xv 64 string2 \"Server Specific Settings:\" ");
    strcat(string, "yv 64 xv 64 string \"This server is set to\" ");
    get_mode_name(s);
    sprintf(s1, "yv 72 xv 64 string2 \"BOP %s mode\" ", s);
    strcat(string, s1);
    if (!laghelp_rst)
      strcat(string, "yv 88 xv 64 string \"HPB resistance is DISABLED\" ");
    else {
      sprintf(s, "yv 88 xv 64 string \"HPB resistance is %0.2f:1\" ", laghelp_rst+1);
      strcat(string, s);
//      if (lag_rst_cap) {
//        sprintf(s, "yv 96 xv 64 string \"  (Capped at %0.2f:1)\" ", lag_rst_cap);
//        strcat(string, s);
//      }
    }
    if (!laghelp_str)
      strcat(string, "yv 104 xv 64 string \"HPB strength is DISABLED\" ");
    else {
      sprintf(s, "yv 104 xv 64 string \"HPB strength is %0.2f:1\" ", laghelp_str+1);
      strcat(string, s);
//      if (lag_str_cap) {
//        sprintf(s, "yv 112 xv 64 string \"  (Capped at %0.2f:1)\" ", lag_str_cap);
//        strcat(string, s);
//      }
    }
    sprintf(s, "yv 128 xv 64 string \"After %0.1f seconds of\" ", (float) lagged_out_detect/10);
    strcat(string, s);
    strcat(string, "yv 136 xv 64 string \"phone-jack, you're\" ");
    strcat(string, "yv 144 xv 64 string \"lag sickness prone.\" ");
  }
  gi.WriteByte(svc_layout);
  gi.WriteString(string);

}

void adv_bop_menu(edict_t *ent) {

// advances the bop menu to the next section...

  ent->client->bopmenu++;
  if (ent->client->bopmenu > 3) {
	ent->client->bopmenu = 0;
	ent->client->showscores = 0;
  }
  else {
	refresh_bop_menu(ent);
	gi.unicast(ent, true);
  }

}

void bopreport(edict_t *ent) {

// gives the user the BOP report screens...

  ent->client->bopmenu = 3;	// start them on the report screens...
  ent->client->showscores = true;
  refresh_bop_menu(ent);
  gi.unicast(ent, true);

}

qboolean bop_client_cmds(edict_t *ent, char *cmd, char *sec_arg) {

// called by g_cmds.c to process client bop-specific commands

	if (Q_stricmp (cmd, "lagstats") == 0)
		Cmd_lagstats(ent);
	else if (Q_stricmp (cmd, "bophelp") == 0) {
	  if (!nobophelp)
	    bophelp(ent);
	  else {
	    if (lag_msg(ent))
	      gi.cprintf(ent, PRINT_HIGH, "Bop help menu disabled\n");
	  }
	}
	else
		return(false);	// unknown command...

	return(true);	// we found a command to execute..

}

qboolean process_bop_cmds(char *cmd, char *sec_arg) {

// called by g_svcmds.c to process bop specific commands
// also called by bopinit()

	strncpy(bop_twoarg, sec_arg, 80);	// copy the second arg into our
					// awaiting array...

	if (Q_stricmp (cmd, "laghelp_str") == 0)
		sv_laghelp_str();
	else if (Q_stricmp (cmd, "laghelp_rst") == 0)
		sv_laghelp_rst();
	else if (Q_stricmp (cmd, "nobophelp") == 0)
		sv_nobophelp();
	else if (Q_stricmp (cmd, "bopsickremove") == 0)
		sv_bopsickremove();
	else if (Q_stricmp (cmd, "laghelp") == 0)
		sv_laghelp_old();
	else if (Q_stricmp (cmd, "lagcap_str") == 0)
		sv_lagcap_str();
	else if (Q_stricmp (cmd, "lagcap_rst") == 0)
		sv_lagcap_rst();
	else if (Q_stricmp (cmd, "gamemode") == 0)
		sv_gamemode();
        else if (Q_stricmp (cmd, "pingmax") == 0)
                sv_pingmax();
        else if (Q_stricmp (cmd, "pingmin") == 0)
                sv_pingmin();
	else if (Q_stricmp (cmd, "lagbare") == 0)
		sv_lagbare();
	else if (Q_stricmp (cmd, "lagpent") == 0)
		sv_lagpent();
	else if (Q_stricmp (cmd, "lagsick") == 0)
		sv_lagsick();
	else if (Q_stricmp (cmd, "lagpeak") == 0)
		sv_lagpeak();
	else if (Q_stricmp (cmd, "lagtimeout") == 0)
		sv_lagtimeout();
	else if (Q_stricmp (cmd, "lagsuicide") == 0)
		sv_lagsuicide();
	else if (Q_stricmp (cmd, "lagslag") == 0)
		sv_lagslag();
	else if (Q_stricmp (cmd, "lagverbose") == 0)
		sv_verbose();
	else if (Q_stricmp (cmd, "laginvisible") == 0)
		sv_invisible();
	else if (Q_stricmp (cmd, "bopmanageobservers") == 0)
		sv_bopmanageobservers();
	else if (Q_stricmp (cmd, "sv") == 0)
		gi.cprintf(0, PRINT_HIGH, "An 'sv' was received that should not be there, try again!\n");
	else
	  return(false);	// unknown command...

	return(true);	// otherwise, return true...

}

void sv_gamemode() {

  int i;
  char s[20];

  i = getargi();

  if ((i>2) || (i<0))
	i = 0;	// default to normal mode...

  game_mode = i;

  switch(i) {
	case 0:	// normal mode
		max_delta_ping = n_max_delta_ping;
		min_delta_ping = n_min_delta_ping;
		laghelp_str = n_laghelp_str;
		laghelp_rst = n_laghelp_rst;
		strcpy(s, "NORMAL");
		break;
	case 1: // expert mode
		max_delta_ping = x_max_delta_ping;
		min_delta_ping = x_min_delta_ping;
		laghelp_str = x_laghelp_str;
		laghelp_rst = x_laghelp_rst;
		strcpy(s, "EXPERT");
		break;
	case 2:	// tournament mode
		max_delta_ping = t_max_delta_ping;
		min_delta_ping = t_min_delta_ping;
		laghelp_str = t_laghelp_str;
		laghelp_rst = t_laghelp_rst;
		strcpy(s, "TOURNAMENT");
		break;
	default:
		gi.cprintf(0, PRINT_HIGH, "Bug in Gamemode func!\n");
		break;
  }
  gi.cprintf(0, PRINT_HIGH, "BOP is now set to %s mode.\n", s);

}

void sv_laghelp_str() {

  float f;

  f = getargf();
  if (f>=0)
	laghelp_str = f;

  gi.cprintf(NULL, PRINT_HIGH, "Strength bonus cannot exceed %f:1\n", laghelp_str+1);

  if (laghelp_str > 5)
	gi.cprintf(NULL, PRINT_HIGH,"Caution: High numbers may crash the server!\n");
  else if (laghelp_str > 1)
	gi.cprintf(NULL, PRINT_HIGH,"Caution: Much higher and lagged players can gib LPB's with a blaster\n");
  else if (!laghelp_str)
        gi.cprintf(NULL, PRINT_HIGH,"Notice: You have DISABLED HPB strength adjustment!\n");

}

void sv_laghelp_rst() {

  float f;

  f = getargf();
  if (f>=0)
	laghelp_rst = f;

  gi.cprintf(NULL, PRINT_HIGH, "Resistance bonus cannot exceed %f:1\n", laghelp_rst+1);

  if (laghelp_rst > 5)
	gi.cprintf(NULL, PRINT_HIGH,"Caution: High numbers may crash the server!\n");
  else if (laghelp_rst > 1)
	gi.cprintf(NULL, PRINT_HIGH,"Caution: Much higher and lagged players will be impossible to frag\n");
  else if (!laghelp_rst)
        gi.cprintf(NULL, PRINT_HIGH,"Notice: You have DISABLED HPB resistance adjustment!\n");

}

void sv_nobophelp() {

  int i;

  i = getargi();
  nobophelp = i;

  if (!i)
	gi.cprintf(NULL, PRINT_HIGH, "Bop Help Menu ENABLED\n");
  else
	gi.cprintf(NULL, PRINT_HIGH, "Bop Help Menu DISABLED\n");

}

void sv_bopsickremove() {

  int i;

  i = getargi();
  bopsickremove = i;

  if (i)
	gi.cprintf(NULL, PRINT_HIGH, "Lag sick players will become observers temporarily\n");
  else
	gi.cprintf(NULL, PRINT_HIGH, "Lag sick players will get stay visible\n");

}

void sv_laghelp_old() {

// Old laghelp, obsolete

  gi.cprintf(NULL, PRINT_HIGH, "Notice: sv laghelp is now an obsolete command.\n");
  gi.cprintf(NULL, PRINT_HIGH, "Try sv laghelp_str and sv laghelp_rst!\n");

}

void sv_lagcap_str() {

// Purpose: Sets the lag cap for strength


  gi.cprintf(NULL, PRINT_HIGH, "Lag strength cap is obsolete!  Use laghelp_str as your cap\n");

}

void sv_lagcap_rst() {

// Purpose: Sets the lag cap for resistance

  gi.cprintf(NULL, PRINT_HIGH, "Lag resistance cap is obsolete!  Use laghelp_rst as your cap\n");

}

void sv_pingmax() {

  int i;

  i = getargi();
  if (i>0)
    max_delta_ping = i;

  gi.cprintf(NULL, PRINT_HIGH, "Max Delta Ping is %d\n", max_delta_ping);

}

void sv_pingmin() {

  int i;

  i = getargi();
  if (i>0)
    min_delta_ping = i;

  gi.cprintf(NULL, PRINT_HIGH, "Min Delta Ping is %d\n", min_delta_ping);

}

void sv_lagbare() {

  int i;

  i = getargi();

  if (i)
	bop_deadtime = i*10;	// convert it to frames

  gi.cprintf(NULL, PRINT_HIGH, "LagBare time is %d seconds\n",((int)bop_deadtime/10));
  gi.cprintf(NULL, PRINT_HIGH, "Make sure you check the value of lagsick!\n");

}

void sv_lagpent() {

  int i;

  i = getargi();

  if (i)
	bop_getawaytime = i*10;

  gi.cprintf(NULL, PRINT_HIGH, "Lagpent will now last for %d seconds\n",(int)bop_getawaytime/10);
  gi.cprintf(NULL, PRINT_HIGH, "Make sure you check the value for lagsick!\n");

}

void sv_lagsick() {

  int i = getargi();

  if ((i) && (i<((bop_deadtime-bop_getawaytime)/10)))
	lagsicktime = bop_deadtime-bop_getawaytime-(i*10);
  else
	i = (bop_deadtime-bop_getawaytime-lagsicktime)/10;
  gi.cprintf(NULL, PRINT_HIGH, "Lagsick set for %d seconds (%f actual)\n",i, lagsicktime);

}

void sv_lagpeak() {

  int i = getargi();

  if (i)
	lag_threshold = i;
  gi.cprintf(NULL, PRINT_HIGH, "You'll get lag pent if your ping exceeds %d\n", lag_threshold);
  gi.cprintf(NULL, PRINT_HIGH, "See also 'sv lagtimeout'\n");

}

void sv_lagtimeout() {

  int i = getargi();

  if (i)
	lagged_out_detect = i;
  gi.cprintf(NULL, PRINT_HIGH, "Users will get lagpent after %d frames of ping-freeze (10 frames = 1 second)\n", lagged_out_detect);

}

void sv_lagsuicide() {

  char *s = "Suicide lag protection ";

  gi.cprintf(NULL, PRINT_HIGH, "%s", s);
  no_suicide_protect = !getargi();
  if (no_suicide_protect)
	gi.cprintf(NULL, PRINT_HIGH, "DISABLED\n");
  else
	gi.cprintf(NULL, PRINT_HIGH, "ENABLED\n");

}

void sv_lagslag() {

  char *s = "Lag non-player protection ";

  gi.cprintf(NULL, PRINT_HIGH, "%s", s);
  no_slag_protect = !getargi();
  if (no_slag_protect)
	gi.cprintf(NULL, PRINT_HIGH, "DISABLED\n");
  else
	gi.cprintf(NULL, PRINT_HIGH, "ENABLED\n");

}

void sv_verbose() {

  char *s = "Verbose messaging ";

  gi.cprintf(NULL, PRINT_HIGH, "%s", s);
  bop_verbose = getargi();
  if (bop_verbose)
    gi.cprintf(NULL, PRINT_HIGH, "ENABLED\n");
  else
    gi.cprintf(NULL, PRINT_HIGH, "DISABLED\n");

}

void sv_invisible() {

  char *s = "BOP Quake2 mod is now ";
  gi.cprintf(NULL, PRINT_HIGH, "%s", s);
  bop_invisible = getargi();
  if (bop_invisible)
    gi.cprintf(NULL, PRINT_HIGH, "INVISIBLE\n");
  else
    gi.cprintf(NULL, PRINT_HIGH, "VISIBLE\n"); 

}

void sv_bopmanageobservers() {

  int i;

  i = getargi();

  bopmanageobservers = i;
  gi.cprintf(0, PRINT_HIGH, "BOP Observer Managing is %i\n", i);

}

void Cmd_lagstats(edict_t *ent) {

// Used to get a cryptic dump of all bop config variables

  char s[250];

  if (lag_msg(ent)) {
    sprintf(s, "BoP running:\nLHS=%f\nLHR=%f\nLB=%f\nLPt=%f\nLPk=%d\nMiDP=%d MxDP=%d\nLS=%f\nNSuP=%d\nNSlP=%d\nLV=%d\nMI=%d\n",
    laghelp_str, laghelp_rst, bop_deadtime, bop_getawaytime, lag_threshold, min_delta_ping,
    max_delta_ping, lagsicktime, no_suicide_protect, no_slag_protect,
    bop_verbose, bop_invisible);

    gi.cprintf(ent, PRINT_HIGH, "%s",s); 
  }

}

float getargf() {

  char *s;

  s = bop_twoarg;

  return(atof(s));

}

int getargi() {

  char *s;

  s = bop_twoarg;

  return(atoi(s));

}




void LagObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
        qboolean	s_is_quad, a_is_quad=0;	// matt's addition
	qboolean	ff;


	if (self->client)
	  s_is_quad = (self->client->quad_framenum > level.framenum);
	if (attacker->client)
	  a_is_quad = (attacker->client->quad_framenum > level.framenum);
	// Matt's stuff here...

	if (coop->value && attacker->client)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	if (deathmatch->value || coop->value)
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = "";

		switch (mod)
		{
		case MOD_SUICIDE:
			message = "dies the lonely death";
			break;
		case MOD_FALLING:
			if (IsFemale(self))
			  message = "fell into her boots";
			else
			  message = "fell into his boots";
			break;
		case MOD_CRUSH:
			message = "was crushed like a grapefruit";
			break;
		case MOD_WATER:
			message = "forgot to grab a rebreather";
			break;
		case MOD_SLIME:
			message = "melted";
			break;
		case MOD_LAVA:
			message = "can't exist on slag alone";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "was blown up";
			break;
		case MOD_EXIT:
			message = "found a way out";
			break;
		case MOD_TARGET_LASER:
			message = "got abdominal surgery";
			break;
		case MOD_TARGET_BLASTER:
			message = "got electrocuted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = "died the death";
			break;
		}
		if (attacker == self)
		{
			switch (mod)
			{
			case MOD_HELD_GRENADE:
                                if (IsFemale(self))
                                  message = "got stuck to her grenade";
                                else
                                  message = "got stuck to his grenade";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				message = "tried to put the pin back in";
				break;
			case MOD_R_SPLASH:
                                if (s_is_quad)
                                  message = "got too quad happy";
                                else
                                  message = "becomes bored with life";
				break;
			case MOD_BFG_BLAST:
                                message = "pointed the BFG the wrong way";
				break;
			default:
				if (IsFemale(self))
					message = "cracked her own pate";
				else
					message = "cracked his own pate";
				break;
			}
		}
		if (message)
		{
			gi.bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message);
			if (deathmatch->value)
                                self->client->resp.score--;     // lose a frag if you kill yourself
			self->enemy = NULL;
			return;
		}

		self->enemy = attacker;
		if (attacker && attacker->client)
		{
		// ie, if they don't have pent...
                  if (attacker->client->invincible_framenum<=level.framenum) {
			switch (mod)
			{
			case MOD_BLASTER:
				message = "was humiliated by";
                                message2 = "'s blaster";
				break;
			case MOD_SHOTGUN:
				message = "was picked-off by";
				break;
			case MOD_SSHOTGUN:
				message = "was blown away by";
				message2 = "'s super shotgun";
				break;
			case MOD_MACHINEGUN:
                                message = "was machine-gunned by";
				break;
			case MOD_CHAINGUN:
				message = "was sawn in half by";
				message2 = "'s chaingun";
				break;
			case MOD_GRENADE:
				message = "ate";
				message2 = "'s pineapple";
				break;
			case MOD_G_SPLASH:
                                message = "was snagged by";
				message2 = "'s grenade";
				break;
			case MOD_ROCKET:
                                if (a_is_quad) {
                                    if (IsFemale(self))
                                      message = "got her world rocked by";
                                    else
                                      message = "got his world rocked by";
                                    message2 = "'s quad rocket";
                                }
                                else if (self->health<-40) {
                                  message = "was gibbed by";
                                  message2 = "'s rocket";
				}
				else {
                                  message = "ate";
                                  message2 = "'s rocket";
				}
				break;
			case MOD_R_SPLASH:
                                if (a_is_quad) {
                                  message = "was obliterated by";
                                  message2 = "'s quad rocket";
                                }
                                else if (self->health<-40) {
				  message = "was gibbed into meat chunks by";
				  message2 = "'s rocket";
				}
				else {
				  message = "ate";
				  message2 = "'s rocket";
				}
				break;
			case MOD_HYPERBLASTER:
                                message = "was melted by";
				message2 = "'s hyperblaster";
				break;
			case MOD_RAILGUN:
				message = "was railed by";
				break;
			case MOD_BFG_LASER:
				message = "saw the pretty lights from";
				message2 = "'s BFG";
				break;
			case MOD_BFG_BLAST:
				if (a_is_quad) {
				  message = "didn't feel a thing as";
				  if (IsFemale(self))
				    message2 = "'s quad BFG ripped her body apart";
				  else
				    message2 = "'s quad BFG ripped his body apart";
				}
				else {
				  message = "intercepted";
				  message2 = "'s big green ball";
				}
				break;
			case MOD_BFG_EFFECT:
				if (a_is_quad) {
				  if (IsFemale(self)) {
				    message = "sprinted like a madwoman from";
				    message2 = "'s quad BFG and got vaporized for her trouble";
				  }
				  else {
				    message = "sprinted like a madman from";
				    message2 = "'s quad BFG and got vaporized for his trouble";
				  }
				}
				else {
				  message = "couldn't hide from";
				  message2 = "'s BFG";
				}
				break;
			case MOD_HANDGRENADE:
                                message = "played catch with";
                                message2 = "'s handgrenade";
				break;
			case MOD_HG_SPLASH:
				message = "cartwheeled over";
				message2 = "'s handgrenade";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_TELEFRAG:
				message = "was caught trespassing on";
				message2 = "'s pad";
				break;
			}
                  }
                  else {
                    if (IsFemale(attacker))
                      message = "forgot to run from pent-girl";
                    else
                      message = "forgot to run from pent-boy";
                  }
                  if (message)
                  {
				gi.bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message, attacker->client->pers.netname, message2);
				if (deathmatch->value)
				{
					if (ff)
						attacker->client->resp.score--;
					else
                                                attacker->client->resp.score++;  // gain a point if you frag someone
				}
				return;
                  }
		}
	}

	gi.bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if (deathmatch->value)
		self->client->resp.score--;
}
