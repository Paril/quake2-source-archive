#include "g_local.h"
#include "laser.h"
#include <sys/types.h>
//#include <sys/timeb.h>

void Grenade_Explode (edict_t *ent);
void sentry_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
#define CTF_FLAG_BONUS			level.FLAG_TAKE_BONUS	// what you get for picking up enemy flag

void Grenade_Die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict (self);
}

void PlaceLaser (edict_t *ent, vec3_t plane)
{
	edict_t		*self;
	int		laser_colour[] =
	{
		0xd0d1d2d3,		// green
		0xf2f2f0f0,		// red							
		0xf3f3f1f1,		// blue	
		0xdcdddedf,		// yellow
		0xe0e1e2e3		// bitty yellow strobe
	};	// valid ent ?

	if ((!G_ClientExists(ent->owner)) || (ent->owner->health <= 0))
		return;	// cells for laser ?

	// Ok, lets stick one on then ...
	gi.cprintf (ent->owner, PRINT_HIGH, "Laser attached.\n");

	// -----------
	// Setup laser
	// -----------

	self = G_Spawn();
	self -> movetype		= MOVETYPE_NONE;
	self -> solid			= SOLID_BBOX;
	self -> s.renderfx		= RF_BEAM|RF_TRANSLUCENT;
	self -> s.modelindex	= 1;			// must be non-zero
	self -> s.sound			= gi.soundindex ("world/laser.wav");
	self -> classname		= "laser_yaya";
	self -> activator       = ent->owner;
	self -> s.frame			= 2;	// beam diameter
  	self -> owner			= ent->owner;
	self -> s.skinnum		= laser_colour[self->owner->client->resp.s_team];
  	self -> dmg				= LASER_DAMAGE;
	self -> think			= pre_target_laser_think;
	self -> delay			= level.time + 20;
	self -> flags           = ~SVF_DEADMONSTER; 
    self -> ripstate        = ent->owner->client->resp.s_team;

	// Set orgin of laser to point of contact with wall
	VectorCopy(ent->s.origin, self->s.origin);

	// convert normal at point of contact to laser angles
	vectoangles(plane, self->s.angles);

	// setup laser movedir (projection of laser)
	G_SetMovedir (self->s.angles, self->movedir);

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);

	// link to world
	gi.linkentity (self);

	// start off ...
	target_laser_off (self);

	// ... but make automatically come on
	self -> nextthink = level.time + 2;
}

void	pre_target_laser_think (edict_t *self)
{
	target_laser_on (self);

	self->think = target_laser_think;
}

void SP_trigger_laser (entity *self)
{
	int		laser_colour[] =
	{
		0xe0e1e2e3,		// bitty yellow strobe
	 	0xf2f2f0f0,		// red
		0xf3f3f1f1,		// blue	
		0xd0d1d2d3,		// green
		0xdcdddedf		// yellow
	};	// valid ent ?

	self -> movetype		= MOVETYPE_NONE;
	self -> solid			= SOLID_BBOX;
	self -> s.renderfx		= RF_BEAM|RF_TRANSLUCENT;
	self -> s.modelindex	= 1;			// must be non-zero
	self -> s.sound			= gi.soundindex ("world/laser.wav");
	self -> s.frame			= 2;	// beam diameter
  	self -> owner			= self;

	if (!self->client->resp.s_team)
		self->ripstate = 0;

	if (!self->s.skinnum)
		self -> s.skinnum		= laser_colour[self->ripstate];

  	self -> dmg				= LASER_DAMAGE;
	self -> think			= pre_target_laser_think;
	self -> delay			= level.time + LASER_TIME;
	self -> flags           = ~SVF_DEADMONSTER; 

	gi.linkentity (self);
}

/*----------------------------------------
  SP_LaserSight

  Create/remove the laser sight entity
-----------------------------------------*/

#define lss self->lasersight

void	pre_target_laser_think (edict_t *self);

void SP_LaserSight(edict_t *self, char *cmd1)
{
   vec3_t  start,forward,right,end;

   if (self->playerclass != 4 && self->client->resp.it != 4)
	   return;

   if (Q_stricmp (cmd1, "off") == 0)
   {
      G_FreeEdict(lss);
      return;
   }
   else if (Q_stricmp (cmd1, "on") == 0)
   {
	   AngleVectors (self->client->v_angle, forward, right, NULL);

       VectorSet(end,100 , 0, 0);
       G_ProjectSource (self->s.origin, end, forward, right, start);

       lss = G_Spawn ();
       lss->owner = self;
       lss->movetype = MOVETYPE_NOCLIP;
       lss->solid = SOLID_NOT;
       lss->classname = "lasersight";
       lss->owner = self;

       if (self->client->resp.s_team == 1)
		   lss->s.modelindex = gi.modelindex ("models/objects/spots/spotr.md2");
       else
	       lss->s.modelindex = gi.modelindex ("models/objects/spots/spotb.md2");

	   lss->prethink = LaserSightThink;
   }
}


/*
===============================
LaserSightThink

Updates the sights position, angle, and shape
is the lasersight entity
==============================
*/

void LaserSightThink (edict_t *self)
{
   vec3_t start,end,endp,offset;
   vec3_t forward,right,up;
   trace_t tr;

   AngleVectors (self->owner->client->v_angle, forward, right, up);

   VectorSet(offset,24 , 6, self->owner->viewheight-7);
   G_ProjectSource (self->owner->s.origin, offset, forward, right, start);
   VectorMA(start,8192,forward,end);

   tr = gi.trace (start,NULL,NULL, end,self->owner, MASK_SHOT);

   if (tr.fraction != 1) {
      VectorMA(tr.endpos,-4,forward,endp);
      VectorCopy(endp,tr.endpos);
   }

   vectoangles(tr.plane.normal,self->s.angles);
   VectorCopy(tr.endpos,self->s.origin);

   gi.linkentity (self);
   self->nextthink = level.time + 0.1;
}

void SP_misc_teleporter_dest (edict_t *ent);

/*
===========================
=### UTILITY FUNCTIONS ###=
===========================
*/

void LessAmmo (edict_t *ent, float x, char * t)
{
	if (((int)dmflags->value & DF_INFINITE_AMMO ) )
		return;

 	if (x > ent->client->pers.inventory[ITEM_INDEX(FindItem(t))])
		return;
 
	ent->client->pers.inventory[ITEM_INDEX(FindItem(t))] -= x;
}

void LessHealth (edict_t *ent, float x)
{
	if (x > ent->health)
		return;

	ent->health -= x;
}

void CheckX (edict_t *self, float x, char *r)
{
	if (self->client->pers.inventory[ITEM_INDEX(FindItem(r))] < x)
	{
		gi.cprintf(self, PRINT_HIGH, "Not enough %s\n", r);
		return;
	}
}

/*
===========
Eject_shell
===========
*/

void info_player_team1 (edict_t *ent)
{
	ent->classname = "info_player_team1";
}

void info_player_team2 (edict_t *ent)
{
	ent->classname = "info_player_team2";

}

void Spawn_TeamPoint (edict_t *ent)
{
	edict_t *spot;
    vec3_t forward;

	spot = G_Spawn();
	spot->owner = ent;

    AngleVectors(ent->client->v_angle, forward, NULL, NULL);
    VectorMA(ent->s.origin, 100, forward, spot->s.origin); 

	if (ent->client->resp.s_team == 2)
		info_player_team2 (spot);
	else if (ent->client->resp.s_team == 1)
		info_player_team1 (spot);
	else
		return;

	spot->health = 30;
	spot->gib_health = -30;
	spot->takedamage = DAMAGE_YES;
	spot->monsterinfo.aiflags = AI_NOSTEP;
	spot->think = M_droptofloor;
	spot->nextthink = level.time + 2 * FRAMETIME;
    SP_misc_teleporter_dest(spot);
}

/*
======================================
This modification allows use of Quake I -style teleports in Quake II
created by Toni Wilen <twilen@sci.fi> (26.12.1997)
======================================
*/

void teleporter_touch2(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*dest;
	int			i,client;
	vec3_t		destdir,tempvec;

	// is a player? (There are probably better ways to do this..)
	if(other->client)
		client = 1;
	else
	{
		client = 0;
		if(!other->velocity)
			return;
	}

	dest = G_Find (NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf ("Couldn't find destination\n");
		return;
	}

    if (self->ripstate && (other->client->resp.s_team != self->ripstate))
		return;

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	VectorCopy (dest->s.origin, other->s.origin);
	other->s.origin[2] += 9;

	// clear the velocity and hold them in place briefly (clients only)
	if(client)
	{
		VectorClear (other->velocity);
		other->teleport_time = 50;
	}

	// draw the teleport splash at the destination
	other->s.event = EV_PLAYER_TELEPORT;

	if(client)
	{
		// set players' angles
		for (i=0 ; i<3 ; i++)
			other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);
		other->s.angles[PITCH] = 0;
		other->s.angles[YAW] = self->s.angles[YAW];
		other->s.angles[ROLL] = 0;
		VectorCopy (self->s.angles, other->client->ps.viewangles);
		VectorCopy (self->s.angles, other->client->v_angle);
	}
	else
	{
		// change velocity of projectiles
		VectorCopy(dest->s.angles,tempvec); // use temp because
		G_SetMovedir(tempvec,destdir); // G_SetMovedir clears source vector
		VectorScale(destdir,VectorLength(other->velocity),other->velocity);
	}

	// only clients can telefrag
	if(client)
	{
		if (!KillBox (other))
		{
		}
	}

	gi.linkentity (other);

}

extern void InitTrigger(edict_t*);
extern void SP_func_wall(edict_t*);

// FUNC_WALL teleport

void SP_func_wall2 (edict_t *self)
{
	// is teleport?
    if(self->target)
	{
		// convert to trigger
		InitTrigger(self);
		self->touch = teleporter_touch2;
		gi.linkentity (self);
		return;
	}

// put visible if not already visible
    if((self->spawnflags & 128) && (self->spawnflags & 1))
	    self->spawnflags &= ~1;
// normal func_wall continues
    SP_func_wall(self);
}

// TRIGGER_TELEPORT

void tr_think (edict_t *self)
{
	 if (strcmp (level.mapname, "ctd3"))
		 self->s.origin[2] -= 20;
}

void SP_trigger_teleport(edict_t *self)
{
     if (!self->target)
	 {
	    gi.dprintf ("trigger_teleport without a target.\n");
	    G_FreeEdict (self);
	    return;
	 }

     InitTrigger(self);
     self->touch = teleporter_touch2;
	 // this is an ugly hack!
     gi.linkentity(self);
	 self->think = tr_think;
	 self->nextthink = level.time + FRAMETIME * 2;
}

void SP_info_teleport_destination(edict_t *ent)
{
}


extern void teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);

// remove misc_teleporter and misc_teleporter_destination -entities and triggers

void remove_teleporter_pads(int *inhibit)
{
     edict_t *from,*trig,*dest;

// remove teleporter pads only if new teleports are detected
    for (from=g_edicts;from < &g_edicts[globals.num_edicts]; from++)
	{
	   if(!from->inuse||from->solid==SOLID_NOT)
	     	continue;
	   if(!strcmp(from->classname,"trigger_teleport")||(!strcmp(from->classname,"func_wall")&&from->target))
	    	break;
	}

    if(from==&g_edicts[globals.num_edicts])
            return;

// remove pads
    for (from=g_edicts;from < &g_edicts[globals.num_edicts]; from++)
	{
	       if(!from->inuse)
			   continue;
	       if(from->solid==SOLID_NOT)
		       continue;
	// misc_teleporter or misc_teleporter_destination without spawnflags&1?
	if(!(from->spawnflags&1)&&!strcmp(from->classname,"misc_teleporter"))
	{
		dest=G_Find(NULL,FOFS(targetname),from->target);
		for(trig=g_edicts;trig<&g_edicts[globals.num_edicts];trig++)
		{
			if(trig->touch==teleporter_touch&&trig->target==from->target)
			{
				G_FreeEdict(trig); // remove teleporter trigger
				(*inhibit)++;
				break;
			}
		}
		G_FreeEdict(dest); // remove misc_teleporter_destination
		G_FreeEdict(from); // remove misc_teleporter
		(*inhibit) += 2;
	}
}
}

// ### TEAMPLAY FUNCTIONS ### //

//========================================
// Ent joins Blue Team

char *CTFTeamName(int team);

void JoinTeam (edict_t *ent, int s_team)
{
	static int num, i;
	edict_t *other;

	if (ent->client->resp.s_team > 0)
		return;

	num = 0;

	for (i = 0; i <= maxclients->value; i++)
	{
		other = &g_edicts[i];

		if (!G_ClientExists(other))
			continue;

		if (other->client->resp.s_team == s_team)
			num++;
	}

	if (num == 0)
	{
        if ((int)ripflags->value & RF_NO_MASTERS)
		{
			ent->team_master = false;
     		gi.bprintf (PRINT_MEDIUM,"%s joined %s team\n", ent->client->pers.netname, CTFTeamName (s_team));
		}
		else
		{
			gi.bprintf (PRINT_MEDIUM,"%s joined %s team as a team master!\n", ent->client->pers.netname, CTFTeamName (s_team));
			ent->team_master = true;
		}
	}
	else
		ent->team_master = false;

	ent->client->resp.s_team = s_team;
}

void Self_Origin (edict_t *ent)
{
     gi.cprintf (ent, PRINT_HIGH, "You're at %s, %s.\n", vtos(ent->s.origin), vtos(ent->s.angles));
}

//========================================
// Prints Teammates classes
void PrintOtherClass (entity *ent)
{
	edict_t *players;
	int found = 0;
	int i;

    for (i = 0; i <= maxclients->value; i ++)
	{
		players = &g_edicts[i];

		if (!players->client)
			continue;

		if (!players->inuse)
			continue;
		
		if (players->client->resp.s_team == ent->client->resp.s_team && ent != players && players->client->resp.s_team != 0)
		{
			if (players->playerclass != 0)
			{
				if (!found)
					found = 1;

				gi.cprintf (ent, PRINT_HIGH, "%s's class is %s\n", players->client->pers.netname, players->playerclasss);
			}
		}
	}

	if (!found)
		gi.cprintf (ent, PRINT_HIGH, "There is nobody on your team\n");
}

//============================================
// Prints if team needs attackers or defenders
void PrintTeamState (entity *ent)
{
	entity *players;
	int num1, num2;
	int i;

	num1 = num2 = 0;

	for_each_player (players, i)
	{
		if (players->client->resp.s_team == ent->client->resp.s_team)
		{
			if (players->teamstate == 1)
                num1++;
			else if (players->teamstate == 2)
                num2++;
		}
	}

	if (num1 > num2)
		gi.cprintf (ent, PRINT_HIGH, "Your team needs defenders!\n");
	else if (num1 < num2)
		gi.cprintf (ent, PRINT_HIGH, "Your team needs attackers!\n");
	else if (num1 == num2)
		gi.cprintf (ent, PRINT_HIGH, "Your team needs attackers!\n");
}

//========================================
// Prints fmt to self enemy's team
void eprintf (edict_t *self, edict_t *ignore, int mod, int printlevel, char *fmt, ...) // not really used yet
{
    va_list ap;
    static char st[100];
	int i;
	edict_t *players;

    va_start (ap, fmt);
    vsprintf (st, fmt, ap);
    va_end (ap);

	for_each_player (players, i)
	{
	   if (players->client->resp.s_team == self->client->resp.s_team)
	        continue;

	   if (players == ignore)
	        continue;

	   if (mod == MOD_NORMAL)
		   gi.cprintf (players, printlevel, "%s", st);
	   else if (mod == MOD_CENTRAL)
		   gi.centerprintf (players,"%s", st);
	}

}

//========================================
// Prints fmt to ALL
void centerprint_all (char *fmt, ...)
{
    va_list ap;
    static char st[100];
    edict_t *ent;
	int i;

    va_start (ap, fmt);
    vsprintf (st, fmt, ap);
    va_end (ap);

    
	for_each_player (ent, i)
            gi.centerprintf(ent, "%s", st);
}

//========================================
// Prints fmt to self's team
void tprintf (edict_t *self, edict_t *ignore, int mod, int printlevel, char *fmt, ...)
{
    va_list ap;
    static char st[100];
	int i;
	edict_t *players;

    va_start (ap, fmt);
    vsprintf (st, fmt, ap);
    va_end (ap);

    for (i = 0; i <= maxclients->value; i ++)
	{
		players = &g_edicts[i];

		if (!players->client)
		   continue;

		if (!players->inuse)
		   continue;

		if (!OnSameTeam (players, self))
		   continue;

	    if (players == ignore)
	   	   continue;

	    if (mod == MOD_NORMAL)
		   gi.cprintf (players, printlevel, "%s", st);
	    else if (mod == MOD_CENTRAL)
		   gi.centerprintf (players, "%s", st);
	}
}

////////////////////////////////////////////////
// Maps stuff                                //
//////////////////////////////////////////////

void target_kill (edict_t *self)
{
	int i;
	edict_t *players;
	vec3_t vel;

	VectorSet (vel, 0, 0, 0);

	for_each_player (players, i)
	{
		if (self->ripstate)
		{
			if (players->client->resp.s_team == self->ripstate)
      			T_Damage (players, self, self, vel, players->s.origin, vec3_origin, 10000, 10000, 0, 39);
		}
		else
  			T_Damage (players, self, self, vel, players->s.origin, vec3_origin, 10000, 10000, 0, 39);
	}

	self->speed = 0;
}

void kill_think (edict_t *self)
{
	if (self->speed)
	{
		self->think = target_kill;
		self->nextthink = level.time + self->delay;
		self->prethink = NULL;
	}
}

void trigger_kill (edict_t *self)
{
	self->prethink = kill_think;
	self->speed = 0;

	gi.linkentity(self);
}


/*
=======================
Middle Flag
=======================
*/

qboolean Pickup_Flag(edict_t *ent, edict_t *other)
{
	// hey, its not our flag, pick it up
	if (level.taker_message)
		tprintf (other, NULL, MOD_CENTRAL, false, "%s %s\n",other->client->pers.netname, level.taker_message);
	else
	    tprintf (other, NULL, MOD_CENTRAL, false, "%s got the enemy's flag!\n",other->client->pers.netname);
	
	if (level.take_message)
	    eprintf (other, NULL, MOD_CENTRAL, false, "%s %s\n",other->client->pers.netname, level.take_message);
    else
	    eprintf (other, NULL, MOD_CENTRAL, false, "%s got your flag!\n",other->client->pers.netname);

	gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("world/x_alarm.wav"), 1, ATTN_NONE, 0);

	other->client->resp.score += CTF_FLAG_BONUS;
    other->mode = 1;
	other->client->resp.ctf_flagsince = level.time;

	// pick up the flag
	// if it's not a dropped flag, we just make is disappear
	// if it's dropped, it will be removed by the pickup caller
	if (!(ent->spawnflags & DROPPED_ITEM)) {
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	return true;
}
