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

// Hatchling no damage
// skin in dir
// Gravity specific to class

//FIXME: spawnque?

#include "g_local.h"
#include "m_breeder.h"
#include "m_engineer.h"
#include "m_player.h"
#include <stdio.h>
#include "g_log.h"

long classtypebit[16]={BC_GRUNT, BC_HATCHLING, BC_HEAVY, BC_COMMANDO, BC_DRONE, BC_MECH, BC_SHOCK, BC_STALKER, BC_BREEDER, BC_ENGINEER, BC_GUARDIAN, BC_KAMI, BC_EXTERM, BC_STINGER, BC_WRAITH, BC_BIO };

void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t /*@out@*/result);
void SP_monster_cocoon (edict_t *self);
void SP_monster_obstacle (edict_t *self);
void SP_monster_gasser (edict_t *self);
void SP_monster_spiker (edict_t *self);
void SP_monster_healer (edict_t *self);

void SP_ammo_depot (edict_t *self);
void CmdSetTripWire (edict_t *self, qboolean summoned);

void SP_misc_teleporter_dest (edict_t *self);
void turret_think (edict_t *self);
void mgturret_think (edict_t *self);

void SP_turret (edict_t *self);
void SP_detector (edict_t *self);

void AddToChangeLog (edict_t *ent, int newclass);

void cocoon_die_large (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

#define BUILD_LOG_SIZE 10

char buildlog[MAXTEAMS][BUILD_LOG_SIZE][256];

typedef struct spawnqueue_s
{
        struct spawnqueue_s     *next;
        edict_t                         *client;
        int                                     class_type;
} spawnqueue_t;

spawnqueue_t    spawnqueue[MAXTEAMS];

static const int spawn_priorities[NUMCLASSES] =
{
        0,      // CLASS_GRUNT          
        0,      // CLASS_HATCHLING      
        2,      // CLASS_HEAVY          
        3,      // CLASS_COMMANDO       
        1,      // CLASS_DRONE          
        5,      // CLASS_MECH           
        1,      // CLASS_SHOCK          
        5,      // CLASS_STALKER        
        4,      // CLASS_BREEDER        
        4,      // CLASS_ENGINEER       
        4,      // CLASS_GUARDIAN       
        2,      // CLASS_KAMIKAZE       
        4,      // CLASS_EXTERM 
        3,      // CLASS_STINGER        
        1,      // CLASS_WRAITH 
        1,      // CLASS_BIO            
};

void PostSpawnSetup (edict_t *ent, int class_type)
{
        // got spawn, go for it!
        log_changeclass (ent, class_type);

        // check for open menu before clearing client
        PMenu_Close(ent);

        // pcis clears just about everything (not .resp and parts of .pers)
        PutClientInServer (ent, class_type);

        //WARNING: configstrings won't have been sent to the client yet (unless on localhost)
        //so it might be a good idea to delay the spawning voice (or you will get the
        //voice of the previous class which can be dodgy)

        //the configstrings (sent above) should arrive before the stuff (but then again
        //this is UDP and anything can happen). either way, lets see what happens.

        //interesting side note, this works fine for humans (the old method). but bugs for aliens
        //hmmm.
        //n/m, i discovered why. aliens have *spawn, humans have /voice/class/spawn.wav

        if (classlist[class_type].voicefunc)
                stuffcmd (ent,"voice 0\n");
                //classlist[ent->client->resp.class_type].voicefunc (ent, VOICE_SPAWN);

        if (!(sv_cheats->value))
                ent->client->resp.score -= classlist[class_type].frags_needed;

        // entity setup code here moved to PutClientInServer

        ent->client->resp.respawn_time = level.time;
        ent->client->resp.can_respawn = true;
        VectorCopy(ent->s.origin, ent->client->resp.respawn_point);
}

int AddToSpawnQueue (edict_t *ent, int class_type)
{
        spawnqueue_t    *q, *last;
        int                             priority;
        int                             position;

        priority = spawn_priorities[class_type];

        q = last = &spawnqueue[ent->client->resp.team];

        position = 0;

        while (q->next)
        {
                q = q->next;
                position++;
                if (spawn_priorities[q->class_type] < priority)
                {
                        int                             my_position;
                        spawnqueue_t    *me;
                        edict_t                 *other;

                        last->next = gi.TagMalloc (sizeof(spawnqueue_t), TAG_LEVEL);

                        me = last->next;
                        me->next = q;
                        me->client = ent;
                        me->class_type = class_type;

                        q = me;

                        my_position = position;

                        //notify everyone we bumped of their new position
                        while (q->next)
                        {
                                q = q->next;
                                position++;

                                gi.centerprintf (q->client, "You are in position %s of the spawn queue\nYou will spawn as a%s %s", colortext (va("%d",position)), classlist[q->class_type].classname[0] == 'E' ? "n" : "", classlist[q->class_type].classname);

                                //share with observers so they don't wonder wtf
                                if (q->client->client->resp.chased)
                                {
                                        for (other = g_edicts + 1; other->client; other++)
                                        {
                                                if (other->client->chase_target == q->client)
                                                        gi.centerprintf (other, "You are in position %s of the spawn queue\nYou will spawn as a%s %s", colortext (va("%d",position)), classlist[q->class_type].classname[0] == 'E' ? "n" : "", classlist[q->class_type].classname);
                                        }
                                }
                        }

                        return my_position;
                }
                last = q;
        }

        position++;
        q->next = gi.TagMalloc (sizeof(spawnqueue_t), TAG_LEVEL);
        q = q->next;
        q->client = ent;
        q->class_type = class_type;

        return position;
}

void RemoveFromSpawnQueue (edict_t *ent)
{
        spawnqueue_t    *q, *last;

        q = last = &spawnqueue[ent->client->resp.team];

        while (q->next)
        {
                q = q->next;

                if (q->client == ent)
                {
                        last->next = q->next;
                        gi.TagFree (q);
                        return;
                }
        }
}

void ClearSpawnQueue (void)
{
        int                             i;
        spawnqueue_t    *q, *last;

        for (i = 0; i < MAXTEAMS; i++)
        {
                q = &spawnqueue[i];
                last = NULL;

                while (q->next)
                {
                        q = q->next;
                        if (last)
                        {
                                gi.TagFree (last);
                        }
                        last = q;
                }

                if (last)
                        gi.TagFree (last);

                spawnqueue[i].next = NULL;
        }
}

void ProcessSpawnQueue (void)
{
        spawnqueue_t    *q, *last, *r;
        int                             i, position;

        for (i = 0; i < MAXTEAMS; i++)
        {
                q = last = &spawnqueue[i];

                if (q->next)
                {
                        q = q->next;

                        if (teleport_respawn (q->client))
                        {
                                edict_t         *other;

                                PostSpawnSetup (q->client, q->class_type);
                                last->next = q->next;
                                gi.TagFree (q);
                                q = last;

                                r = &spawnqueue[i];
                                position = 0;
                                while (r->next)
                                {
                                        position++;
                                        r = r->next;

                                        gi.centerprintf (r->client, "You are in position %s of the spawn queue\nYou will spawn as a%s %s", colortext (va("%d",position)), classlist[r->class_type].classname[0] == 'E' ? "n" : "", classlist[r->class_type].classname);

                                        //share with observers so they don't wonder wtf
                                        if (r->client->client->resp.chased)
                                        {
                                                for (other = g_edicts + 1; other->client; other++)
                                                {
                                                        if (other->client->chase_target == r->client)
                                                                gi.centerprintf (other, "You are in position %s of the spawn queue\nYou will spawn as a%s %s", colortext (va("%d",position)), classlist[r->class_type].classname[0] == 'E' ? "n" : "", classlist[r->class_type].classname);
                                                }
                                        }
                                }
                        }
                        else
                        {
                                continue;
                        }
                }
        }
}

spawnqueue_t *IsInSpawnQueue (edict_t *ent, int *out_position)
{
        spawnqueue_t    *q, *last;
        int                             position;

        q = last = &spawnqueue[ent->client->resp.team];

        position = 0;

        while (q->next)
        {
                q = q->next;
                position++;
                if (q->client == ent)
                {
                        *out_position = position;
                        return q;
                }
        }

        *out_position = position;
        return NULL;
}

void resetLog (void)
{
        int i;

        for (i = 0; i < BUILD_LOG_SIZE; i++)
        {
                buildlog[TEAM_ALIEN][i][0] = '\0';
                buildlog[TEAM_HUMAN][i][0] = '\0';
        }
}

void BuildLog (edict_t *self, char *object, qboolean created)
{
        int i;

        if (!self->client->resp.visible)
                return;

        for (i = 0; i < BUILD_LOG_SIZE-1; i++) {
                strcpy(buildlog[self->client->resp.team][i], buildlog[self->client->resp.team][i+1]);
        }

        if (created) {
                strncpy (buildlog[self->client->resp.team][i], va("%s created %s\n",self->client->pers.netname, object), sizeof(buildlog[self->client->resp.team][i])-1);
                log_makestructure (self, object, 0);
        } else {
                strncpy (buildlog[self->client->resp.team][i], va("%s destroyed %s\n",self->client->pers.netname, object), sizeof(buildlog[self->client->resp.team][i])-1);
        }
}

void BuildLogEnt(edict_t*player, edict_t*built)
{
  built->bclient = player-g_edicts-1;
  built->bport = player->client->pers.port;
  built->bip.l = player->client->pers.ip4.l;
}

void ShowLog (edict_t *ent, int team)
{
        char message[1024] = "";
        int i;

        if (team == TEAM_HUMAN)
                strcpy (message, "Human build log:\n");
        else if (team == TEAM_ALIEN)
                strcpy (message, "Alien build log:\n");

        for (i = 0; i < BUILD_LOG_SIZE; i++) {
                if (buildlog[team][i][0])
                        strcat (message, buildlog[team][i]);
                if (strlen(message) > 1000)
                        break;
        }

        gi.cprintf (ent, PRINT_HIGH, "%s", message);
}

void viewLog (edict_t *ent)
{
        //gods see all
        if (ent->client->pers.adminpermissions & (1 << PERMISSION_VIEWGAMESTUFF)) {
                ShowLog (ent, TEAM_ALIEN);
                ShowLog (ent, TEAM_HUMAN);
        } else {
                ShowLog (ent, ent->client->resp.team);
        }
}


static void turtle_expire(edict_t*ent)
{
  G_FreeEdict(ent);
  ProcessJoinQueues();
  ProcessSpawnQueues(0);
}

void breakinSetDefaults(void)
{
        edict_t*ent;
        int i;

        team_info.teamnames[TEAM_NONE] = "Observer";
        team_info.teamnames[TEAM_ALIEN] = "Aliens";
        team_info.teamnames[TEAM_HUMAN] = "Humans";

        team_info.starttime = level.time + reconnect_wait->value;
        team_info.spawnkills = 0;

        team_info.buildflags = 0;

        for(i=0; i<MAXTEAMS; i++)
        {
                team_info.lost[i] = false;
                team_info.numplayers[i] = 0;
                team_info.points[i] = 0;
                team_info.bodycount[i] = 0;
                team_info.spawns[i] = 0;
                team_info.maxpoints[i] = (int)teambonus->value;
        }

  ent = G_Spawn();
  ent->classname = "turtletimer";
  ent->nextthink = team_info.starttime;
  ent->think = turtle_expire;
}

void ClipBBoxToEights (edict_t *ent)
{
        int i;
        float   temp;

        for (i=0 ; i<3 ; i++)
        {
                temp = ent->s.origin[i]*8;
                if (temp > 0.0)
                        temp += 0.5;
                else
                        temp -= 0.5;
                ent->s.origin[i] = 0.125f * (int)temp;
        }
}

int countPlayers(int t)         // gets actual alive players on a team
{
        int count=0;
        edict_t *ent;

        for (ent=g_edicts+1 ; ent<g_edicts+game.maxclients+1 ; ent++)
        {
                if(ent->inuse && ent->client->resp.team == t && ent->health > 0)
                        count++;
        }

        return count;
}

spawnlist_t     spawnlist[MAXTEAMS] = {
        {0},
        {0},
        {0}};

void AddToSpawnlist (edict_t *spawn, int team)
{
        spawnlist_t     *temp;

        temp = &spawnlist[team];

        while (temp) {
                // take any no-spawn spot
                if (temp->spawn)
                        if (temp->spawn->inuse == false || (temp->spawn->enttype != ENT_TELEPORTER && temp->spawn->enttype != ENT_COCOON)) {
                        temp->spawn = spawn;
                        return;
                }

                if (!temp->spawn) {
                        temp->spawn = spawn;
                        return;
                }
                if (!temp->next) {
                        temp->next = gi.TagMalloc(sizeof(spawnlist_t), TAG_LEVEL);
                }

                temp = temp->next;
        }
}

edict_t *SelectNearestDeathmatchSpawnPoint (int team, edict_t *ent)
{
        edict_t *bestspot=NULL;
        float   bestdistance=4096*2, bestplayerdistance;
        vec3_t  v;

        spawnlist_t *temp;
        int type;

        temp = &spawnlist[team];

        if (team == TEAM_HUMAN)
                type = ENT_TELEPORTER;
        else
                type = ENT_COCOON;

        while(temp) {

                // null any invalid spawns (died ones)
                if (temp->spawn && (temp->spawn->inuse == false || temp->spawn->enttype != type))
                        temp->spawn = NULL;

                // process any found spawns
                if (temp->spawn && temp->spawn->enttype == type)
                {
                        if (temp->spawn->teleport_time < level.time)
                        {
                                if (type == ENT_COCOON && temp->spawn->solid == SOLID_NOT)
                                {
                                        temp = temp->next;
                                        continue;
                                }

                                // try to choose the nearest spawn
                                VectorSubtract (temp->spawn->s.origin, ent->s.origin, v);
                                bestplayerdistance = VectorLength (v);

                                if (bestplayerdistance < bestdistance)
                                {
                                        bestspot = temp->spawn;
                                        bestdistance = bestplayerdistance;
                                }                       
                        } else
                        if (team == TEAM_HUMAN && !temp->spawn->nextthink) { //ugly hack because initial teles dont worked with queue for some reason
                                temp->spawn->nextthink = temp->spawn->teleport_time;
                                temp->spawn->think = SpawnProcessQueue;
                        }
                }

                temp = temp->next;
        }

        return bestspot;
}

int teleport_respawn(edict_t *ent)
{
        vec3_t accel;
        edict_t *dest = NULL;

        if (ent->client->resp.team == TEAM_NONE)
                return 1;

        dest = SelectNearestDeathmatchSpawnPoint (ent->client->resp.team, ent);

        if (dest == NULL)
        {
                return 0;
        }

        // clear the velocity and hold them in place briefly
        VectorClear (ent->velocity);

        ent->s.angles[PITCH] = 0;
        if (dest->enttype != ENT_COCOON)
          ent->s.angles[YAW] = dest->s.angles[YAW];
        else
          ent->s.angles[YAW] = dest->ideal_yaw;
        ent->s.angles[ROLL] = 0;

        if(ent->client->resp.team == TEAM_ALIEN)
        {
                vec3_t origin, up;
                trace_t tr;

                // Explode egg
                dest->monsterinfo.melee(dest);

                //thanks to timbo for making this work :)
                AngleVectors (dest->s.angles, NULL, NULL, up);
                VectorNormalize (up);
                VectorMA (dest->s.origin, 48, up, origin);
                VectorScale (up, 288, accel);

                //head room
                tr = gi.trace (dest->s.origin, ent->mins, ent->maxs, origin, dest, MASK_SOLID);
                if (tr.fraction < 1)
                {
                        if (origin[2] - tr.endpos[2] < 48)
                        {
                                dest->solid = SOLID_NOT;
                                gi.linkentity (dest);
                        }
                        VectorCopy (tr.endpos, origin);
                }
                //set their origin to "above" the egg
                VectorCopy (origin, ent->s.origin);

                tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, ent->s.origin, dest, MASK_SOLID);
                if (tr.startsolid)
                {
                        stuffcmd (ent, "+movedown ; wait ; -movedown\n");
                }

                //set their speed to "jump" out the egg if its "upwards"
                if (accel[2] > 0)
                        VectorCopy (accel, ent->velocity);

                //egg wait
                dest->teleport_time = level.time + 20 - (float)(((float)dest->health / (float)dest->max_health)*10.0);
        }
        else if(ent->client->resp.team == TEAM_HUMAN)
        {
                trace_t tr;

                VectorCopy (dest->s.origin, ent->s.origin);
                // push the player a bit upwards, so he doesn't get stuck on tele
                ent->s.origin[2] += 16;
                //ent->velocity[2] = 256;

                tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, ent->s.origin, dest, MASK_SOLID);
                if (tr.startsolid)
                {
                        stuffcmd (ent, "+movedown ; wait ; -movedown\n");
                }

                //tele wait
                dest->teleport_time = level.time + 20 - (float)(((float)dest->health / (float)dest->max_health)*10.0);
                if (dest->health != dest->max_health)
                        dest->teleport_time += 5;

                dest->nextthink = dest->teleport_time;
                dest->think = SpawnProcessQueue;
        }
        
        
        

        // kill anything at the destination
        if (ent->client->resp.team != TEAM_NONE)
        {
                edict_t *touch[1024], *e;
                trace_t tr;
                int             num;
                int             i;

                num = gi.BoxEdicts (ent->absmin, ent->absmax, touch, MAX_EDICTS, AREA_SOLID);

                for (i = 0; i < num; i++)
                {
                        e = touch[i];

                        if (e == dest)
                                continue;

                        if (!e->inuse)
                                continue;

                        if (!(e->svflags & SVF_MONSTER) || !(e->flags & FL_CLIPPING) || !e->client)
                                continue;

                        if (e->health > 0)
                                T_Damage (e, dest, dest, vec3_origin, vec3_origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
                }
                
                do
                {
                        tr = gi.trace (ent->s.origin, tv (-16, -16, -24), tv (16, 16, 32), ent->s.origin, dest, MASK_PLAYERSOLID);
                        if (tr.ent && tr.ent->client)
                        {
                                T_Damage (tr.ent, dest, dest, vec3_origin, vec3_origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
                        }
                } while (tr.ent && tr.ent->client);
        }

        return 1;
}


#if 0
qboolean TeamStart (edict_t *ent, int class_type)
{
        spawnqueue_t    *q;
        int                             i;
        int                             position;

        i = classlist[class_type].frags_needed;

        if (ent->client->resp.team != classlist[class_type].team) {
                gi.cprintf(ent, PRINT_HIGH, "That class belongs to the other team.\n");
                return false;
        } else if (ent->client->resp.key) {
                gi.cprintf (ent, PRINT_HIGH, "You can't change class when holding onto a key.\n");
                return false;
        } else if (!ent->client->resp.visible) {
                gi.cprintf (ent, PRINT_HIGH, "You can't play when you're invisible, idiot.\n");
                return false;
        } else if (team_info.classes & classtypebit[class_type] && class_type != CLASS_OBSERVER) {
                gi.cprintf (ent, PRINT_HIGH, "That class is currently unavailable.\n");
                return false;
        } else if (ent->client->resp.classban & classtypebit[class_type]) {
                gi.cprintf (ent, PRINT_HIGH, "You are not allowed to spawn that class!\n");
                return false;
        } else
        if (ent->health > 0) {
           if (ent->client->acid_duration > 0) {
                gi.cprintf (ent, PRINT_MEDIUM, "Can't respawn when poisoned.\n");
                return false;
          } else
          if (ent->client->slow_timer > 0) {
                gi.cprintf (ent, PRINT_MEDIUM, "Can't respawn while slowed.\n");
                return false;
          } else
          if (ent->pain_debounce_time + 2 > level.time) {
                gi.cprintf (ent, PRINT_MEDIUM, "Can't respawn immediately after taking damage.\n");
                return false;
          }
        }
        
        if(!sv_cheats->value)
        if(i != 0)
        if(ent->client->resp.score < i)
        {
                int diff = i - ent->client->resp.score ;
                gi.cprintf(ent, PRINT_HIGH, "You need %d more frag%s to go %s.\n", diff, (diff != 1) ? "s" : "", classlist[class_type].classname);
                return false;
        }

        if (buildercap->value > 0 && (class_type == CLASS_ENGINEER || class_type == CLASS_BREEDER)) {
                edict_t *e;
                int max_builders;

                max_builders = buildercap->value;

                for (e = g_edicts +1 ; e <= g_edicts + game.maxclients; e++) {
                        if (e->inuse && e->health > 0)
                        if (e->client->resp.class_type == class_type) {
                                if (max_builders-- == 0) {
                                        gi.cprintf (ent, PRINT_HIGH, "Your team already has enough builder classes.\n");
                                        return false;
                                }
                        }
                }

        }

        // borks when team loses players
        /*
        numhumans = countPlayers(TEAM_HUMAN);
        numaliens = countPlayers(TEAM_ALIEN);
        if (teameven->value > 1  && ((numhumans - numaliens > 2 && ent->client->resp.team == TEAM_HUMAN) || (numaliens - numhumans > 2 && ent->client->resp.team == TEAM_ALIEN))) {
                gi.cprintf (ent, PRINT_HIGH, "You cannot respawn whilst your team has too many players.\n");
                return false;
        }*/

        // do some pre-spawning checks

        if (class_type != CLASS_OBSERVER)
        {
                if(ent->health > 0 && ent->health < ent->max_health/2)
                {
                        gi.cprintf(ent, PRINT_HIGH, "Not enough health\n");
                        return false;
                }

                if(ent->client->build_timer > level.time && class_type != CLASS_OBSERVER)
                {
                        gi.cprintf(ent, PRINT_HIGH, "You must wait until your timer is up.\n");
                        return false;
                }

                // don't allow spawning during death anims
                if (ent->deadflag == DEAD_DYING)
                        return false;

                // respawn spamming prevention
                if (ent->client->resp.respawn_time + 2 > level.time)
                {
                        return false;
                }
        }

        q = IsInSpawnQueue (ent, &position);
        if (q)
        {
                if (q->class_type == class_type)
                {
                        gi.centerprintf (ent, "You are in position %s of the spawn queue\nYou will spawn as a%s %s", colortext (va("%d",position)), classlist[class_type].classname[0] == 'E' ? "n" : "", classlist[class_type].classname);
                        return false;
                }

                RemoveFromSpawnQueue (ent);
        }

        /* I hate this, prevents you to spawn into bigger class if you happen to get frags while under attack -ankka
        if (ent->health > 0 && ent->client && ent->client->resp.team != TEAM_NONE) {
                edict_t *e = NULL;
                while ((e=findradius_c(e,ent,750))) {
                        if (e->client && e->client->resp.team != ent->client->resp.team && e->client->resp.team != TEAM_NONE && e->health > 0){
                                gi.cprintf(ent, PRINT_HIGH, "Cannot changeteam when near enemies.\n");
                                return false;
                        }
                }
        }*/

        if (ent->health > 0)
        {
                AddToChangeLog (ent, class_type);

                ent->health = -99; // must gib player, otherwise TeamStart doesn't complete
                // prevents obit
                meansOfDeath = MOD_CHANGECLASS;
                player_die (ent, ent, ent, 100000, vec3_origin);
        }

        //we do this here so teleport_respawn can do proper bbox size checking.
        VectorCopy (classlist[class_type].mins, ent->mins);
        VectorCopy (classlist[class_type].maxs, ent->maxs);

        // try to find free tele/egg and move player to it
        // safe to call with TEAM_NONE's
        if (!teleport_respawn(ent))
        {
                int     position;

                //we couldn't spawn instantly

                position = AddToSpawnQueue (ent, class_type);

                //gi.cprintf(ent, PRINT_HIGH, "No spawns available!\n");
                gi.centerprintf (ent, "You are in position %s of the spawn queue\nYou will spawn as a%s %s", colortext (va("%d",position)), classlist[class_type].classname[0] == 'E' ? "n" : "", classlist[class_type].classname);

                //share with observers so they don't wonder wtf
                if (ent->client->resp.chased)
                {
                        edict_t *other;

                        for (other = g_edicts + 1; other->client; other++)
                        {
                                if (other->client->chase_target == ent)
                                        gi.centerprintf (other, "You are in position %s of the spawn queue\nYou will spawn as a%s %s", colortext (va("%d",position)), classlist[class_type].classname[0] == 'E' ? "n" : "", classlist[class_type].classname);
                        }
                }

                ent->client->resp.respawn_time = level.time;
                PMenu_Close (ent);

                return false;
        }
        
        PostSpawnSetup (ent, class_type);

        return true;
}
#endif










qboolean TeamStartCheck(edict_t *ent, int class_type)
{
        spawnqueue_t    *q;
        int                             i;
        int                             position;
        
        if (ISLINKBOT(ent)) class_type = CLASS_OBSERVER;

        i = classlist[class_type].frags_needed;

        if (ent->client->resp.team != classlist[class_type].team) {
                gi.cprintf(ent, PRINT_HIGH, "That class belongs to the other team.\n");
                return false;
        } else if (ent->client->resp.key) {
                gi.cprintf (ent, PRINT_HIGH, "You can't change class when holding onto a key.\n");
                return false;
        } else if (!ent->client->resp.visible) {
                gi.cprintf (ent, PRINT_HIGH, "You can't play when you're invisible, idiot.\n");
                return false;
        } else if (team_info.classes & classtypebit[class_type] && class_type != CLASS_OBSERVER) {
                gi.cprintf (ent, PRINT_HIGH, "That class is currently unavailable.\n");
                return false;
        } else if (ent->client->resp.classban & classtypebit[class_type]) {
                gi.cprintf (ent, PRINT_HIGH, "You are not allowed to spawn that class!\n");
                return false;
        } else
        if (ent->health > 0) {
           if (ent->client->acid_duration > 0) {
                gi.cprintf (ent, PRINT_MEDIUM, "Can't respawn when poisoned.\n");
                return false;
          } else
          if (ent->client->slow_timer > 0) {
                gi.cprintf (ent, PRINT_MEDIUM, "Can't respawn while slowed.\n");
                return false;
          } else
          if (ent->pain_debounce_time + 2 > level.time) {
                gi.cprintf (ent, PRINT_MEDIUM, "Can't respawn immediately after taking damage.\n");
                return false;
          }
        }
        
        if (class_type != CLASS_OBSERVER) {
          // respawn spamming prevention
          if (ent->client->resp.respawn_time + 2 > level.time)  return false;
        }
        
        fun_prespawn(ent, &class_type);

        if(!sv_cheats->value)
        if(i != 0)
        if(ent->client->resp.score < i)
        {
                int diff = i - ent->client->resp.score ;
                gi.cprintf(ent, PRINT_HIGH, "You need %d more frag%s to go %s.\n", diff, (diff != 1) ? "s" : "", classlist[class_type].classname);
                return false;
        }

        if (buildercap->value > 0 && (class_type == CLASS_ENGINEER || class_type == CLASS_BREEDER)) {
                edict_t *e;
                int max_builders;

                max_builders = buildercap->value;

                for (e = g_edicts +1 ; e <= g_edicts + game.maxclients; e++) {
                        if (e->inuse && e->health > 0)
                        if (e->client->resp.class_type == class_type) {
                                if (max_builders-- == 0) {
                                        gi.cprintf (ent, PRINT_HIGH, "Your team already has enough builder classes.\n");
                                        return false;
                                }
                        }
                }

        }

        // do some pre-spawning checks

        if (class_type != CLASS_OBSERVER)
        {
                if(ent->health > 0 && ent->health < ent->max_health/2)
                {
                        gi.cprintf(ent, PRINT_HIGH, "Not enough health\n");
                        return false;
                }

                if(ent->client->build_timer > level.time && class_type != CLASS_OBSERVER)
                {
                        gi.cprintf(ent, PRINT_HIGH, "You must wait until your timer is up.\n");
                        return false;
                }

                // don't allow spawning during death anims
                if (ent->deadflag == DEAD_DYING)
                        return false;

                // respawn spamming prevention
                /* which causes more key press spam and makes menu look irresponsive... better delay player in queue
                if (ent->client->resp.respawn_time + 2 > level.time)
                {
                        return false;
                }
                */
        }

        /* I hate this, prevents you to spawn into bigger class if you happen to get frags while under attack -ankka
        if (ent->health > 0 && ent->client && ent->client->resp.team != TEAM_NONE) {
                edict_t *e = NULL;
                while ((e=findradius_c(e,ent,750))) {
                        if (e->client && e->client->resp.team != ent->client->resp.team && e->client->resp.team != TEAM_NONE && e->health > 0){
                                gi.cprintf(ent, PRINT_HIGH, "Cannot changeteam when near enemies.\n");
                                return false;
                        }
                }
        }*/
        
        return true;
}


qboolean TeamStart (edict_t *ent, int class_type)
{
        if (!TeamStartCheck(ent, class_type)) return false;
        QueuePlayerRespawn(ent, class_type);
        return true;
}


qboolean RealTeamStart (edict_t *ent, int class_type)
{
        if (!TeamStartCheck(ent, class_type)) return false;

        if (ISLINKBOT(ent)) class_type = CLASS_OBSERVER;

        fun_prespawn(ent, &class_type);

        if (ent->health > 0)
        {
                AddToChangeLog (ent, class_type);

                ent->health = -1; // must gib player, otherwise TeamStart doesn't complete
                // prevents obit
                meansOfDeath = MOD_CHANGECLASS;
                if (class_type == CLASS_ENGINEER || class_type == CLASS_BREEDER)
                  ent->client->invincible_framenum = level.framenum+1; //Silence "%s changed class" message when going builder class
                ent->client->pers.changeclass_new = class_type;
                player_die (ent, ent, ent, 100000, vec3_origin);
                ent->client->pers.changeclass_new = -1;
        }

        //we do this here so teleport_respawn can do proper bbox size checking.
        VectorCopy (classlist[class_type].mins, ent->mins);
        VectorCopy (classlist[class_type].maxs, ent->maxs);

        // try to find free tele/egg and move player to it
        // safe to call with TEAM_NONE's
        if (!teleport_respawn(ent))
        {
                int     position;

                //we couldn't spawn instantly

                gi.cprintf(ent, PRINT_HIGH, "No spawns available!\n");

                ent->client->resp.respawn_time = level.time;
                PMenu_Close (ent);

                return false;
        }
        
        PostSpawnSetup (ent, class_type);
        if (sv_extended) ext.SetEntityMask(ent->s.number, 1);

        return true;
}

void JoinHumanQueueMenu(edict_t*ent);
void JoinAlienQueueMenu(edict_t*ent);


#ifdef LPBEVEN
float GetPings (int team)
{
        int i;
        float pings;
        edict_t *ent;

        pings = 0;

        for (i = 0; i < game.maxclients; i++) {
                ent = g_edicts + i + 1;
                if (!ent->inuse)
                        continue;
                if (ent->client->resp.team == team)
                        pings += ent->client->ping;
        }

        return pings;
}

/*void UpdatePlayerCounts (void)
{
        team_info.averageping[TEAM_ALIEN] = team_info.averageping[TEAM_HUMAN] = 0;

        team_info.averageping[TEAM_ALIEN] = (GetPings(TEAM_ALIEN) / team_info.numplayers[TEAM_ALIEN]);
        team_info.averageping[TEAM_HUMAN] = (GetPings(TEAM_HUMAN) / team_info.numplayers[TEAM_HUMAN]);
}*/
#endif

//FIXME: this should be used instead of all the icky if (!ent->owner->inuse) crap on every
//owner specific item... this is event based, the owner stuff runs every frame and is liable
//to not work if the ent doesn't think every frame.
void CleanUpOwnerStuff (edict_t *ent)
{
        edict_t *e;

        // used for kami kill credit in copytobodyque and body_explode_die
        e = NULL;
        while ((e = G_Find3 (e, ENT_CORPSE)) != NULL)
        {
                if (e->target_ent == ent) {
                        e->target_ent = NULL;
                }
        }

        // remove possible kami spikes or guardian spikes
        e = NULL;
        while ((e = G_Find3 (e, ENT_SPIKE)) != NULL)
        {
                if (e->owner == ent)
                        G_FreeEdict (e);
        }

        if (ent->client->mirror_ent != NULL) {
              G_FreeEdict(ent->client->mirror_ent);
              ent->client->mirror_ent = NULL;
        }

        /*e = NULL;
        while ((e = G_Find (e, FOFS(classname), "nukething")) != NULL) {
                if (e->target_ent == ent) {
                        G_FreeEdict (e);
                }
        }*/
}

void Updateteam_menu (void);
qboolean TeamChange(edict_t *ent, int t, qboolean force)
{ /*
use one of
void JoinHumanQueueMenu(edict_t*ent)
void JoinAlienQueueMenu(edict_t*ent)
void JoinTeam0(edict_t*ent)
to join a team, never call this directly, unless @setteam
*/

  uint8_t leader_update = false;

  PlayerQueueRemove(&queue_join_humans, ent);
  PlayerQueueRemove(&queue_join_aliens, ent);

        //FIXME: check if all the following code needs to check for force
        if(ent->client->resp.team != t)
        {
                edict_t *other;

                //if((!force && (ent->client->resp.teamchanges > maxswitches->value || forceteams->value)) || !deathmatch->value)
                if (forceteams->value && !(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS)) && !force)
                {
                        //if (!(ent->client->resp.team == TEAM_NONE && t == TEAM_NONE && force))
                        //if (forceteams->value)
                        gi.cprintf (ent, PRINT_HIGH, "Enforced teams are active. Send a message to an admin to join a team.\n");
                        return false;
                }

                if(maxswitches->value && ent->client->resp.teamchanges > maxswitches->value && !force)
                {
                        //gi.bprintf (PRINT_MEDIUM, "%s is a team changing lamer.\n", ent->client->pers.netname);
                        gi.cprintf (ent, PRINT_HIGH, "You have used up your team changes.\n");

                        return false;
                }

                if(team_info.starttime > level.time && !(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS)) && !force)
                {
                        gi.cprintf(ent, PRINT_HIGH, "Waiting for other players to connect, %d seconds left.\n", (int)(team_info.starttime - level.time));
                        return false;
                }

                if (ent->client->resp.team == TEAM_NONE && t != TEAM_NONE && !force) {

                        if (team_info.numplayers[TEAM_ALIEN] + team_info.numplayers[TEAM_HUMAN] >= maxplayers->value) {
                                gi.cprintf (ent, PRINT_HIGH, "All %d player slots are currently full, you may only observe.\n", (int)maxplayers->value);
                                return false;
                        }
                }

                /*if(ent->client->build_timer > level.time && !force)
                {
                        gi.cprintf(ent, PRINT_HIGH, "You must wait until your timer is up.\n");
                        return false;
                }*/

/*
                if(teameven->value && !force && !(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS))) {
                        // from obs -> alien
                        if (t == TEAM_ALIEN && team_info.numplayers[TEAM_ALIEN] > team_info.numplayers[TEAM_HUMAN]) {
                                gi.cprintf (ent, PRINT_HIGH, "You may not unbalance the teams.\n");
                                return false;
                        }
                        // from obs -> human
                        else if (t == TEAM_HUMAN && team_info.numplayers[TEAM_HUMAN] > team_info.numplayers[TEAM_ALIEN]) {
                                gi.cprintf (ent, PRINT_HIGH, "You may not unbalance the teams.\n");
                                return false;
                        }
                        // from team -> team
                        else if (ent->client->resp.team != TEAM_NONE && team_info.numplayers[ent->client->resp.team] >= team_info.numplayers[t]) {
                                gi.cprintf (ent, PRINT_HIGH, "You may not unbalance the teams.\n");
                                return false;
                        }
                }
*/

                LogRemoveDamages(ent);

                RemoveFromSpawnQueue (ent);
                
                if (t) {
                        //reset their frags if positive
                        if (!(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS))) {
                           if (ent->client->resp.score > 0) ent->client->resp.score = 0;
                           if (ent->client->resp.total_score > 0) ent->client->resp.total_score = 0;
                        }
                
                        if (!ent->client->resp.teamchanges++)
                          ent->client->resp.enterframe = level.framenum; //Update scoreboard time to show real time in team, not since conncted to server

                        memset (ent->client->resp.kills, 0, sizeof(ent->client->resp.kills));
                  if (!level.suddendeath && (ent->client->resp.team || ent->client->resp.old_team && ent->client->resp.old_team != t))
                    gi.bprintf (PRINT_HIGH, "%s switched to the %s team\n", ent->client->pers.netname, (t==TEAM_ALIEN)?"alien":"human");
                    
                  if (/*TODO: optimize*/ 1) leader_update = true;
                } else
                if (!level.suddendeath && !(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS)))
                  gi.bprintf (PRINT_HIGH, "%s left the %s team\n", ent->client->pers.netname, (ent->client->resp.team==TEAM_ALIEN)?"alien":"human");
 
                // observers only for teamed players
                if (t == TEAM_NONE && ent->client->resp.chased) {
                        ent->client->resp.chased = false;

                        for (other = g_edicts + 1; other->client; other++) {
                                if (other->client->chase_target == ent) ChaseStop(other);
                        }
                }

                // changing teams kills the player

                if (ent->health > 0 && ent->client->resp.team != TEAM_NONE) {
                        ent->flags &= ~FL_GODMODE;
                        ent->health = -1; // must gib player, otherwise TeamStart doesn't complete
                        // prevents obit
                        meansOfDeath = MOD_CHANGECLASS;
                        player_die (ent, ent, ent, 100000, vec3_origin);

                        //FIXME: don't do this next line (messes up the body que)
                        //note, not 100% necessary, just fixes bug with someone changing teams
                        //then doing %c before picking new class (shows stalker on humans for eg)
                }

                CleanUpOwnerStuff (ent);

                team_info.numplayers[ent->client->resp.team]--;

                if (ent->client->resp.team != t) {
                  ent->client->resp.old_team = ent->client->resp.team;
                  ent->client->resp.team = t;
                }
                
                team_info.numplayers[t]++;

                configplayerlights(ent);

                Updateteam_menu();

                // Set client mask for extended entity visibility settings
                if (sv_extended) {
                    ext.SetClientMask((ent - g_edicts - 1), 16 << ent->client->resp.team);
                }

                if (level.time < team_info.startfragtimer && team_info.startfragtimer != -1)
                        ent->client->resp.score = team_info.startfrags[t];
        }

        if (leader_update) {
                    int i;
                    ent->client->pers.tflags &= ~TFLAG_LEADER;
                    for (i = 1; i <= game.maxclients; i++) if (g_edicts[i].inuse && g_edicts[i].client && g_edicts[i].client->pers.connected && !g_edicts[i].client->resp.team) {
                      char* leader = Info_ValueForKey(g_edicts[i].client->pers.userinfo, "leader");
                      if (leader && *leader && GetClientIDbyNameOrID(leader)+1 == ent-g_edicts) {
                        ent->client->pers.tflags |= TFLAG_LEADER;
                        RemovePlayerFromQueues(g_edicts+i);
                        if (t == TEAM_ALIEN)
                          JoinAlienQueueMenu(g_edicts+i);
                        else
                          JoinHumanQueueMenu(g_edicts+i);
                      }
                    }
        }
        return true;
}

void JoinTeam0(edict_t *ent)
{
        if (TeamChange (ent, TEAM_NONE, true))
        {
                //VectorClear (ent->s.angles);
                ent->s.angles[2] = 0;
                ent->s.angles[0] = 0;
                TeamStart (ent, CLASS_OBSERVER);
        }

        //fix dodgy view shifting bug
        //ent->client->kick_angles[0] = ent->client->kick_angles[1] = ent->client->kick_angles[2] = 0;
        //ent->client->ps.kick_angles[0] = ent->client->ps.kick_angles[1] = ent->client->ps.kick_angles[2] = 0;
        //VectorClear (ent->client->kick_angles);
        //VectorClear (ent->s.angles);
}

void SpawnClass(edict_t *ent)
{
        int     class_type=-1;

        if (!ent->client->menu.entries)
                return;

        if (ent->client->menu.entries == alien_menu || ent->client->menu.entries == alien_menu_exit) {

                switch (ent->client->menu.cur) {
                case 2:
                        class_type = CLASS_BREEDER;
                        break;
                case 3:
                        class_type = CLASS_HATCHLING;
                        break;
                case 4:
                        class_type = CLASS_DRONE;
                        break;
                case 5:
                        class_type = CLASS_WRAITH; //WraithStart does needed inits
                        break;
                case 6:
                        class_type = CLASS_KAMIKAZE;
                        break;
                case 7:
                        class_type = CLASS_STINGER;
                        break;
                case 8:
                        class_type = CLASS_GUARDIAN;
                        break;
                case 9:
                        class_type = CLASS_STALKER;
                        break;
                default:
                        return;
                }
        } else if (ent->client->menu.entries == human_menu || ent->client->menu.entries == human_menu_exit) {
                switch (ent->client->menu.cur) {
                case 2:
                        class_type = CLASS_ENGINEER;
                        break;
                case 3:
                        class_type = CLASS_GRUNT;
                        break;
                case 4:
                        class_type = CLASS_SHOCK;
                        break;
                case 5:
                        class_type = CLASS_BIO;
                        break;
                case 6:
                        class_type = CLASS_HEAVY;
                        break;
                case 7:
                        class_type = CLASS_COMMANDO;
                        break;
                case 8:
                        class_type = CLASS_EXTERM;
                        break;
                case 9:
                        class_type = CLASS_MECH; //MechStart does needed inits
                        break;
                default:
                        return;
                }
        }
        
        if (class_type < 0) return;
        
        if (classlist[class_type].team == TEAM_ALIEN || classlist[class_type].team == TEAM_HUMAN) {
          if (TeamStart (ent, class_type)) PMenu_Close(ent);
        } else
          PMenu_Close(ent);

}

//////////////////////////////////////////////////
// Functions for breeder and engineer construction
//////////////////////////////////////////////////

/*void checkcontents (edict_t *ent)
{
        int pc = gi.pointcontents (ent->s.origin);
        trace_t tr;
        vec3_t end;

        G_TouchTriggers (ent);

        VectorCopy (ent->s.origin, end);

        tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, CONTENTS_LAVA|CONTENTS_SLIME);

        if (tr.contents & CONTENTS_LAVA || pc & CONTENTS_LAVA)
                T_Damage (ent, world, world, vec3_origin, vec3_origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION | DAMAGE_IGNORE_RESISTANCES, MOD_LAVA);
        else if (pc & CONTENTS_SLIME || tr.contents & CONTENTS_SLIME) {
                if (ent->flags & FL_CLIPPING) {
                        T_Damage (ent, world, world, vec3_origin, vec3_origin, vec3_origin, 10, 0, DAMAGE_NO_PROTECTION | DAMAGE_IGNORE_RESISTANCES, MOD_SLIME);
                } else if (ent->svflags & SVF_MONSTER) {
                        if (ent->health < ent->max_health && ent->pain_debounce_time < level.time)
                                ent->health++;
                }
        }
}*/


static void turret_boom_die_explode (edict_t *ent)
{
	T_Damage (ent,ent,ent,vec3_origin,vec3_origin,vec3_origin,10000,0,DAMAGE_NO_PROTECTION,0);
	if (ent->target_ent) {
		ent = ent->target_ent;
		T_Damage (ent,ent,ent,vec3_origin,vec3_origin,vec3_origin,10000,0,DAMAGE_NO_PROTECTION,0);
	}
}

void depotcheck(edict_t *self)
{
        edict_t *ent;
        trace_t tr;
        vec3_t up;//, mins={-16,-16,0}, maxs={16,16,0};

        if(!self->groundentity) {
                self->die (self, self, self, 5, vec3_origin);
                return;
        } else {
                if(self->groundentity->takedamage && self->groundentity->flags & FL_CLIPPING)
                {
                        T_Damage (self->groundentity, self, self, self->velocity, self->s.origin, vec3_origin, 200, 1, 0, MOD_UNKNOWN);
                }
        }

        // atm, works only for teleporters and depots
        if ((self->flags & FL_CLIPPING) && (gi.pointcontents(self->s.origin) & (CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME))) {
                self->die (self, self, self, 5, vec3_origin);
                return;
        }

        ent = NULL;

        while((ent = findradius(ent, self->s.origin, 72)) != NULL)
        {
                if (ent == self)
                        continue;

                if (ent->inuse && ent->enttype == ENT_TELEPORTER) {
                        self->health = 0;
                        self->die (self, self, self, 5, vec3_origin);
                        return;
                }

        }

        while((ent = findradius(ent, self->s.origin, 32)) != NULL)
        {

                if (ent == self)
                        continue;
                if (ent->inuse && (ent->enttype == ENT_TURRET || ent->enttype == ENT_MGTURRET)) {
                        self->health = 0;
                        self->die (self, self, self, 5, vec3_origin);
                        return;
                }
        }

/*  makes sure there is enough clearance for spawns so you don't spawn with your head in the void  */

        VectorCopy(self->s.origin, up);
        up[2]+=40;

        if (self->enttype == ENT_TELEPORTER) {
                tr=gi.trace(self->s.origin, NULL, NULL, up, self, MASK_SHOT);
                if (tr.fraction < 1.0){
                        self->health = 0;
                        self->die (self, self, self, 5, vec3_origin);
                }
        }

        if (self->spawnflags & 1024) {
                self->think = turret_boom_die_explode;
                self->nextthink = level.time + 30;
        } else {
                self->nextthink = self->teleport_time;
                if (!self->nextthink) self->nextthink = level.time+FRAMETIME;
                self->think = SpawnProcessQueue;
        }
}

static void CheckBreederDeath(edict_t*ent)
{
  gclient_t*cl;
  if (!ent->owner || !ent->owner->inuse || !ent->owner->client) {
    ent->prethink = NULL;
    return;
  }

  cl = ent->owner->client;
  if (!(cl->ps.pmove.pm_flags & PMF_TIME_TELEPORT) || !cl->ps.pmove.pm_time) {
    ent->prethink = NULL;
    return;
  }

  if (ent->owner->health <= 0) {
    cl->ps.pmove.pm_time = 1; //dont freeze dead breeder
    switch (ent->enttype) {
        case ENT_OBSTACLE:
            team_info.points[TEAM_ALIEN] -= COST_OBSTACLE;
            break;
        case ENT_COCOON:
            team_info.points[TEAM_ALIEN] -= COST_EGG;
            break;
        case ENT_SPIKER:
            team_info.points[TEAM_ALIEN] -= COST_SPIKER;
            break;
        case ENT_GASSER:
            team_info.points[TEAM_ALIEN] -= COST_GASSER;
            break;
        case ENT_HEALER:
            team_info.points[TEAM_ALIEN] -= COST_HEALER;
    }
    G_FreeEdict(ent); //and kill structure
  }
}


void lay_egg(edict_t *self)
{
        vec3_t forward,up;
        edict_t *baby;
        trace_t tr;
        vec3_t mins = {-16, -16, -16}, maxs = {16, 16, 16};
        qboolean ceiling = false;

        if(self->client->build_timer > level.time)
        {
                gi.cprintf(self, PRINT_HIGH, "You must rest %d seconds.\n", (int)(self->client->build_timer - level.time));
                return;
        }

        if (team_info.buildflags & 64) {
                gi.cprintf(self, PRINT_HIGH, "Can't lay an egg here!\n");
                return;
        }

        if(self->client->invincible_framenum > level.framenum)
        {
                gi.cprintf(self, PRINT_HIGH, "Can\'t breed while invulnerable.\n");
                return;
        }

        if(team_info.points[TEAM_ALIEN] + COST_EGG > team_info.maxpoints[1])
        {
                gi.cprintf(self, PRINT_HIGH, "Not enough build points!\n");
                return;
        }

        // test from -64 forward to -64 forward and +32 up
        AngleVectors (self->s.angles, forward, NULL, NULL);
        forward[2] = 0;
        VectorScale(forward, -64, forward);

        VectorAdd(self->s.origin, forward, forward);
        VectorCopy(forward,up);

//        up[2] += 32;
        up[2] += 8;

        tr = gi.trace (forward, mins, maxs, up, self, MASK_SHOT);

        if (tr.fraction < 1.0f || tr.ent != world)
        {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }
        
        if (ceiling_eggs->value >= 1) {
          if (ceiling_eggs->value < 2) {
            vec3_t upwards = {forward[0], forward[1], forward[2]+32};
            vec3_t downwards = {forward[0], forward[1], forward[2]-40};
            trace_t uptrace = gi.trace(forward, mins, maxs, upwards, self, MASK_SHOT);
            if (uptrace.fraction < 1) {
              trace_t downtrace;
              if (self->groundentity) gi.unlinkentity(self->groundentity);
              downtrace = gi.trace(forward, mins, maxs, downwards, self, MASK_SHOT);
              if (self->groundentity) gi.linkentity(self->groundentity);
            
              ceiling = uptrace.fraction+0.1f < downtrace.fraction;
            }
          } else
            ceiling = true;
        }

        if (!ceiling) {
            if (!self->groundentity) {
                vec3_t up2;

                VectorCopy(self->s.origin,up2);

                up2[2] -= 64;

                tr = gi.trace (self->s.origin, mins, maxs, up2, self, MASK_SHOT);

                if (tr.fraction == 1.0f) {
                        gi.cprintf(self, PRINT_HIGH, "You must be on the ground to breed!\n");
                        return;
               }
            }
        } else {
                up[2]+= 1024;

                tr = gi.trace (forward, mins, maxs, up, self, MASK_SHOT);
                if ((tr.fraction == 1.0f)  || (tr.surface && tr.surface->flags & SURF_SKY)) {
                        gi.cprintf (self,PRINT_HIGH,"Need a solid ceiling (which ain't too high)!\n");
                        return;
                }

                up[2]-= 4096;

                tr = gi.trace (forward, mins, maxs, up, self, MASK_WATER | MASK_SHOT);
                if (tr.contents & MASK_WATER) {
                        gi.cprintf (self,PRINT_HIGH,"Need a solid ground!\n");
                        return;
                }
                
        }


        /*gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_DEBUGTRAIL);
        gi.WritePosition (self->s.origin);
        gi.WritePosition (forward);
        gi.multicast (self->s.origin, MULTICAST_PHS);*/

        // test from origin to -64 forward
        tr = gi.trace (self->s.origin, mins, maxs, forward, self, MASK_SHOT);

        if (tr.fraction < 1.0 || tr.ent != world)
        {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }

        if (gi.pointcontents(tr.endpos)&CONTENTS_NOBUILD){
                gi.cprintf(self, PRINT_HIGH, "Can't lay an egg here!\n");
                return;
        }

        PMenu_Close(self);

        BuildLog (self, "egg", true);
        self->client->pers.idletime = 0;

        baby = G_Spawn();
        BuildLogEnt(self, baby);

        baby->spawnflags = ceiling?32:0;

        VectorCopy(tr.endpos, baby->s.origin);
        VectorCopy(self->s.angles, baby->s.angles);

        ClipBBoxToEights (baby);

        baby->pain_debounce_time = level.time + 32;
        baby->teleport_time = level.time + 15;

        baby->classname = "monster_cocoon";
        baby->enttype = ENT_COCOON;     // layed

        // delay appearance
        baby->think = SP_monster_cocoon;
        baby->nextthink = level.time + 2;

	baby->prethink = CheckBreederDeath;
        baby->owner = self;

        self->s.frame = BREED_COVER_S;
        self->client->anim_end = BREED_COVER_E;
        self->client->anim_priority = ANIM_ATTACK;
#ifndef DEBUG
        self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
        self->client->ps.pmove.pm_time = 180;//(unsigned char)


        team_info.points[TEAM_ALIEN] += COST_EGG;

        self->client->build_timer = level.time + 20;
#endif
        //KillBox (self);
}

void lay_obstacle(edict_t *self)
{
        vec3_t forward;
        edict_t *baby;
        trace_t tr;
        vec3_t mins = {-16, -16, -16}, maxs = {16, 16, 16};

        if(self->client->build_timer > level.time)
        {
                gi.cprintf(self, PRINT_HIGH, "You must rest %d seconds.\n", (int)(self->client->build_timer - level.time));
                return;
        }

        if (team_info.buildflags & 256) {
                gi.cprintf(self, PRINT_HIGH, "Can't build an obstacle here!\n");
                return;
        }

        if (!self->groundentity)
        {
                vec3_t up2;

                VectorCopy(self->s.origin,up2);

                up2[2] -= 64;

                tr = gi.trace (self->s.origin, mins, maxs, up2, self, MASK_SHOT);

                if (tr.fraction == 1.0 && tr.ent != world) {
                        gi.cprintf(self, PRINT_HIGH, "You must be on the ground to breed!\n");
                        return;
                }
        }

        if(team_info.points[1] + COST_OBSTACLE > team_info.maxpoints[1])
        {
                gi.cprintf(self, PRINT_HIGH, "Not enough build points!\n");
                return;
        }

        AngleVectors (self->s.angles, forward, NULL, NULL);
        forward[2] = 0;
        VectorScale(forward, -60.0, forward);

        VectorAdd(self->s.origin, forward, forward);
        tr = gi.trace (self->s.origin, mins, maxs, forward, self, MASK_SHOT);
        if (tr.fraction < 1.0 || tr.ent != world)
        {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }

        if (gi.pointcontents(tr.endpos)&CONTENTS_NOBUILD){
                gi.cprintf(self, PRINT_HIGH, "Can\'t lay an obstacle here!\n");
                return;
        }

        PMenu_Close(self);

        BuildLog (self, "obstacle", true);
        self->client->pers.idletime = 0;

        baby = G_Spawn();
        BuildLogEnt(self, baby);

        baby->spawnflags = 0;

        VectorCopy(forward, baby->s.origin);

        ClipBBoxToEights (baby);

        VectorCopy(self->s.angles, baby->s.angles);

        baby->classname = "monster_obstacle";

        baby->enttype = ENT_OBSTACLE;

        // delay appearance
        baby->think = SP_monster_obstacle;
        baby->nextthink = level.time + 0.9f;


	baby->prethink = CheckBreederDeath;
        baby->owner = self;
        
        baby->target_ent = self;

        self->s.frame = BREED_COVER_S;
        self->client->anim_end = BREED_COVER_E;
        self->client->anim_priority = ANIM_ATTACK;

#ifndef DEBUG
        self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
        self->client->ps.pmove.pm_time = 120;


        team_info.points[1] += COST_OBSTACLE;

        self->client->build_timer = level.time + 7;
#endif

        //KillBox (self);
}

void lay_gasser(edict_t *self)
{
        vec3_t forward;
        edict_t *baby;
        trace_t tr;
        vec3_t mins = {-8, -8, -16}, maxs = {16, 16, 16};

        if(self->client->build_timer > level.time)
        {
                gi.cprintf(self, PRINT_HIGH, "You must rest %d seconds.\n", (int)(self->client->build_timer - level.time));
                return;
        }

        if (team_info.buildflags & 1024) {
                gi.cprintf(self, PRINT_HIGH, "Can't lay a gasser here!\n");
                return;
        }

        if (!self->groundentity)
        {
                vec3_t up2;

                VectorCopy(self->s.origin,up2);

                up2[2] -= 64;

                tr = gi.trace (self->s.origin, mins, maxs, up2, self, MASK_SHOT);

                if (tr.fraction == 1.0 && tr.ent != world) {
                        gi.cprintf(self, PRINT_HIGH, "You must be on the ground to breed!\n");
                        return;
                }
        }

        if(team_info.points[1] + COST_GASSER > team_info.maxpoints[1])
        {
                gi.cprintf(self, PRINT_HIGH, "Not enough build points!\n");
                return;
        }

        AngleVectors (self->s.angles, forward, NULL, NULL);
        forward[2] = 0;
        VectorScale(forward, -60.0, forward);

        VectorAdd(self->s.origin, forward, forward);
        tr = gi.trace (self->s.origin, mins, maxs, forward, self, MASK_SHOT);
        if (tr.fraction < 1.0 || tr.ent != world)
        {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }
        if (gi.pointcontents(tr.endpos)&CONTENTS_NOBUILD){
                gi.cprintf(self, PRINT_HIGH, "Can\'t lay a gasser here!\n");
                return;
        }

        PMenu_Close(self);

        BuildLog (self, "gasser", true);
        self->client->pers.idletime = 0;

        baby = G_Spawn();
        BuildLogEnt(self, baby);

        baby->spawnflags = 0;

        VectorCopy(forward, baby->s.origin);

        ClipBBoxToEights (baby);

        baby->classname = "monster_gasser";


        // delay appearance
        baby->enttype = ENT_GASSER;
        baby->think = SP_monster_gasser;
        baby->nextthink = level.time + 0.9f;
        baby->owner = self;
	baby->prethink = CheckBreederDeath;

        self->s.frame = BREED_COVER_S;
        self->client->anim_end = BREED_COVER_E;
        self->client->anim_priority = ANIM_ATTACK;

#ifndef DEBUG
        self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
        self->client->ps.pmove.pm_time = 120;


        team_info.points[1] += COST_GASSER;
        self->client->build_timer = level.time + 8;
#endif

        //KillBox (self);
}

void lay_spiker(edict_t *self)
{
        vec3_t forward;
        edict_t *baby;
        trace_t tr;
        vec3_t mins = {-16, -16, -16}, maxs = {16, 16, 16};

        if(self->client->build_timer > level.time)
        {
                gi.cprintf(self, PRINT_HIGH, "You must rest %d seconds.\n", (int)(self->client->build_timer - level.time));
                return;
        }

        if (team_info.buildflags & 128) {
                gi.cprintf(self, PRINT_HIGH, "Can't lay a spiker here!\n");
                return;
        }

        if (!self->groundentity)
        {
                vec3_t up2;

                VectorCopy(self->s.origin,up2);

                up2[2] -= 48;

                tr = gi.trace (self->s.origin, mins, maxs, up2, self, MASK_SHOT);

                if (tr.fraction == 1.0 && tr.ent != world) {
                        gi.cprintf(self, PRINT_HIGH, "You must be on the ground to breed!\n");
                        return;
                }
        }

        if(team_info.points[1] + COST_SPIKER > team_info.maxpoints[1])
        {
                gi.cprintf(self, PRINT_HIGH, "Not enough build points!\n");
                return;
        }

        AngleVectors (self->s.angles, forward, NULL, NULL);
        forward[2] = 0;
        VectorScale(forward, -60.0, forward);

        VectorAdd(self->s.origin, forward, forward);
        tr = gi.trace (self->s.origin, mins, maxs, forward, self, MASK_MONSTERSOLID);
        if (tr.fraction < 1.0 || tr.ent != world)
        {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }
        if (gi.pointcontents(tr.endpos)&CONTENTS_NOBUILD){
                gi.cprintf(self, PRINT_HIGH, "Can\'t lay a spiker here!\n");
                return;
        }

        PMenu_Close(self);

        BuildLog (self, "spiker", true);
        self->client->pers.idletime = 0;

        baby = G_Spawn();
        BuildLogEnt(self, baby);

        baby->spawnflags = 0;

        VectorCopy(forward, baby->s.origin);

        ClipBBoxToEights (baby);

        VectorCopy(self->s.angles, baby->s.angles);

        baby->pain_debounce_time = level.time + 32;
        baby->teleport_time = level.time + 10;

        baby->classname = "monster_spiker";

        baby->enttype = ENT_SPIKER;

        // delay appearance
        baby->think = SP_monster_spiker;
        baby->nextthink = level.time + 1.4f;
        baby->owner = self;
	baby->prethink = CheckBreederDeath;

        self->s.frame = BREED_COVER_S;
        self->client->anim_end = BREED_COVER_E;
        self->client->anim_priority = ANIM_ATTACK;

#ifndef DEBUG
        self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
        self->client->ps.pmove.pm_time = 140;

        team_info.points[1] += COST_SPIKER;
        self->client->build_timer = level.time + 10;
#endif

        //KillBox (self);
}

void lay_healer(edict_t *self)
{
        vec3_t forward;
        edict_t *baby;
        trace_t tr;
        vec3_t mins = {-16, -16, 0}, maxs = {16, 16, 32};

        if (self->client->build_timer > level.time)
        {
                gi.cprintf(self, PRINT_HIGH, "You must rest %d seconds.\n", (int)(self->client->build_timer - level.time));
                return;
        }

        if (team_info.buildflags & 512) {
                gi.cprintf(self, PRINT_HIGH, "Can't lay a healer here!\n");
                return;
        }


        if (!self->groundentity)
        {
                vec3_t up2;

                VectorCopy(self->s.origin,up2);

                up2[2] -= 64;

                tr = gi.trace (self->s.origin, mins, maxs, up2, self, MASK_SHOT);

                if (tr.fraction == 1.0 && tr.ent != world) {
                        gi.cprintf(self, PRINT_HIGH, "You must be on the ground to breed!\n");
                        return;
                }
        }

        if(team_info.points[1] + COST_HEALER > team_info.maxpoints[1])
        {
                gi.cprintf(self, PRINT_HIGH, "Not enough build points!\n");
                return;
        }

        AngleVectors (self->s.angles, forward, NULL, NULL);
        forward[2] = 0;
        VectorScale(forward, -50.0, forward);

        VectorAdd(self->s.origin, forward, forward);
        tr = gi.trace (self->s.origin, mins, maxs, forward, self, MASK_SHOT);
        if (tr.fraction < 1.0 || tr.ent != world)
        {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }
        if (gi.pointcontents(tr.endpos)&CONTENTS_NOBUILD){
                gi.cprintf(self, PRINT_HIGH, "Can\'t lay a healer here!\n");
                return;
        }

        PMenu_Close(self);

        BuildLog (self, "healer", true);
        self->client->pers.idletime = 0;

        baby = G_Spawn();
        BuildLogEnt(self, baby);

        baby->spawnflags = 0;

        VectorCopy(forward, baby->s.origin);

        ClipBBoxToEights (baby);

        VectorCopy(self->s.angles, baby->s.angles);
        baby->s.angles[1] += 180;

        baby->pain_debounce_time = level.time + 32;
        baby->teleport_time = level.time + 10;
        baby->owner = self;

        baby->classname = "monster_healer";

        baby->enttype = ENT_HEALER;

        // delay appearance
        baby->think = SP_monster_healer;
        baby->nextthink = level.time + 1.5f;
	baby->prethink = CheckBreederDeath;

        self->s.frame = BREED_COVER_S;
        self->client->anim_end = BREED_COVER_E;
        self->client->anim_priority = ANIM_ATTACK;

#ifndef DEBUG
        self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
        self->client->ps.pmove.pm_time = 180;

        team_info.points[1] += COST_HEALER;

        self->client->build_timer = level.time + 10;
#endif
        //KillBox (self);
}


void create_depot(edict_t *self)
{
        vec3_t forward;
        edict_t *baby;
        trace_t tr;
        vec3_t mins = {-16, -16, -24}, maxs = {16, 16, 0};

        if (team_info.buildflags & 32) {
                gi.cprintf(self, PRINT_HIGH, "Can't build a depot here!\n");
                return;
        }

        if(self->client->build_timer > level.time)
        {
                gi.cprintf(self, PRINT_HIGH, "You must wait %d seconds.\n", (int)(self->client->build_timer - level.time));
                return;
        }

        if(team_info.points[2] + COST_DEPOT > team_info.maxpoints[2])
        {
                gi.cprintf(self, PRINT_HIGH, "Not enough build points!\n");
                return;
        }

        AngleVectors (self->s.angles, forward, NULL, NULL);
        forward[2] = 0;
        VectorScale(forward, 50.0, forward);

        VectorAdd(self->s.origin, forward, forward);
        forward[2] += 16;       // move off ground a little.
        tr = gi.trace (self->s.origin, mins, maxs, forward, self, MASK_SHOT);
        if (gi.pointcontents (forward) == CONTENTS_SOLID || tr.fraction < 1.0 || tr.ent != world)
        {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }
        if (gi.pointcontents(tr.endpos)&CONTENTS_NOBUILD)
        {
                gi.cprintf(self, PRINT_HIGH, "Can\'t build a depot here!\n");
                return;
        }

        tr = gi.trace (forward, NULL, NULL, self->s.origin, self, MASK_SHOT);
        if (tr.fraction != 1.0) {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }

        PMenu_Close(self);

        BuildLog (self, "depot", true);
        self->client->pers.idletime = 0;

        baby = G_Spawn();
        BuildLogEnt(self, baby);

        baby->spawnflags = 0;

        VectorCopy(forward, baby->s.origin);

        ClipBBoxToEights (baby);

        baby->s.angles[1] = self->s.angles[1];

        baby->pain_debounce_time = level.time + 32;
        //baby->owner = self;
        baby->enttype = ENT_AMMO_DEPOT;

        // delay appearance
        SP_ammo_depot(baby);

        baby->think = depotcheck;
        baby->nextthink = level.time + 2;

#ifndef DEBUG
        team_info.points[2] += COST_DEPOT;
        self->client->build_timer=level.time+5;
#endif
        //KillBox (self);
}

void spawnspot_die(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void spawn_pain (edict_t *self, edict_t *other, float kick, int damage);

//void test_touch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
//{
        //gi.dprintf ("Boo.\n");
//}

void create_teleporter(edict_t *self)
{
        vec3_t forward;
        vec3_t head;
        edict_t *baby;
        trace_t tr;
        vec3_t mins = {-24, -24, -24}, maxs = {24, 24, -8};

        if (team_info.buildflags & 1) {
                gi.cprintf(self, PRINT_HIGH, "Can't build a teleporter here!\n");
                return;
        }

        if(team_info.points[2] + COST_TELEPORT > team_info.maxpoints[2])
        {
                gi.cprintf(self, PRINT_HIGH, "Not enough build points!\n");
                return;
        }

        if(self->client->build_timer > level.time){
                gi.cprintf(self, PRINT_HIGH, "You must wait %d seconds.\n", (int)(self->client->build_timer - level.time));
                return;
        }

        VectorCopy(self->s.origin, head);
        //head[2] += self->viewheight;
        AngleVectors (self->s.angles, forward, NULL, NULL);
        forward[2] = 0;
        VectorScale(forward, 70, forward);

        VectorAdd(head, forward, forward);
        forward[2] += self->viewheight/2;       // move off ground a little.
        tr = gi.trace (self->s.origin, mins, maxs, forward, self, MASK_SHOT);
        if (gi.pointcontents (forward) == CONTENTS_SOLID || tr.fraction < 1.0 || tr.ent != world)
        {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }

        tr = gi.trace (forward, NULL, NULL, self->s.origin, self, MASK_SHOT);
        if (tr.fraction != 1.0) {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }

        if (gi.pointcontents(tr.endpos)&CONTENTS_NOBUILD) {
                gi.cprintf(self, PRINT_HIGH, "Can\'t build a teleporter here!\n");
                return;
        }

        PMenu_Close(self);

        BuildLog (self, "tele", true);
        self->client->pers.idletime = 0;

        baby = G_Spawn();
        BuildLogEnt(self, baby);

        baby->spawnflags = 0;
        VectorCopy(forward,baby->s.origin);
        baby->s.angles[YAW] = 180 + self->s.angles[YAW];

        baby->pain_debounce_time = level.time + 32;
        baby->classname = "info_player_deathmatch";

        gi.setmodel (baby, "models/objects/dmspot/tris.md2");
        VectorSet (baby->mins, -24, -24, -24);
        VectorSet (baby->maxs, 24, 24, -16);

        baby->solid = SOLID_BBOX;

        baby->flags |= FL_CLIPPING;
        baby->teleport_time = level.time + 5;
        baby->max_health = 215;
        baby->health = 1;

        baby->spawnflags |= 1024; // for killing unfinished teles after a time

        baby->think = depotcheck;
        baby->nextthink = level.time + 2;
        baby->damage_absorb = 30;
        baby->mass = 800;
        baby->die = spawnspot_die;

        baby->enttype = ENT_TELEPORTER;

        baby->takedamage = DAMAGE_YES;
        baby->movetype = MOVETYPE_STEP;
        baby->s.renderfx |= RF_IR_VISIBLE;

        team_info.points[2] += COST_TELEPORT;

        gi.linkentity(baby);
}

static void fix_bbox (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (other == ent->target_ent) {
                VectorSet (ent->mins, -24, -24, 0);
                VectorSet (ent->maxs, 24, 24, 24);

                ent->s.origin[0] = ent->target_ent->s.origin[0];
                ent->s.origin[1] = ent->target_ent->s.origin[1];

                VectorClear (ent->target_ent->velocity);
                VectorClear (ent->velocity);

                gi.linkentity (ent);
                ent->touch = NULL;
        }
}

void create_turret(edict_t *self)
{
        vec3_t forward, start;
        edict_t *baby;
        trace_t tr, tr2;
        vec3_t mins = {-24, -24, 0 };
        vec3_t maxs = {24 , 24 , 24};

        if(self->client->build_timer > level.time)
        {
                gi.cprintf(self, PRINT_HIGH, "You must wait %d seconds.\n", (int)(self->client->build_timer - level.time));
                return;
        }

        if (team_info.buildflags & 2) {
                gi.cprintf(self, PRINT_HIGH, "Can't build a turret here!\n");
                return;
        }

        if(team_info.points[2] + COST_TURRET > team_info.maxpoints[2])
        {
                gi.cprintf(self, PRINT_HIGH, "Not enough build points!\n");
                return;
        }

        AngleVectors (self->s.angles, forward, NULL, NULL);
        forward[2] = 0;
        VectorScale(forward, 128.0, forward);

        VectorAdd(self->s.origin, forward, forward);
        //forward[2] += ;       // move off ground a little.
        tr = gi.trace (self->s.origin, mins, maxs, forward, self, MASK_SHOT);
        if (gi.pointcontents (forward) == CONTENTS_SOLID || tr.fraction < 1.0 || tr.ent != world) {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }
        if (gi.pointcontents(tr.endpos)&CONTENTS_NOBUILD) {
                gi.cprintf(self, PRINT_HIGH, "Can\'t build a turret here!\n");
                return;
        }

        VectorCopy (forward,start);
        forward[2] -= 64;

        tr = gi.trace (start, mins, maxs, forward, self, MASK_SHOT);
        if (tr.allsolid || tr.startsolid) {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }

        if (tr.fraction !=  1.0)
                VectorCopy (tr.endpos, forward);

        tr2 = gi.trace (forward, NULL, NULL, self->s.origin, self, MASK_SHOT);
        if (tr2.fraction != 1.0) {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }

        forward[2] += 24;

        PMenu_Close(self);

        BuildLog (self, "turret", true);
        self->client->pers.idletime = 0;

        baby = G_Spawn();
        BuildLogEnt(self, baby);

        baby->spawnflags = 128;

        VectorCopy(forward, baby->s.origin);

        ClipBBoxToEights (baby);

        baby->s.angles[1] = self->s.angles[1];

        //baby->pain_debounce_time = level.time + 32.0;
        baby->classname = "turret";
        baby->enttype = ENT_TURRET;

        // delay appearance
        SP_turret(baby);
        baby->s.sound = 0;
        baby->random = 0;
        baby->spawnflags &= ~32768;
        //VectorSet (baby->mins, -8, -8, 0);
        //VectorSet (baby->maxs, 8, 8, 24);

        VectorClear (baby->mins);
        VectorClear (baby->maxs);

        baby->touch = fix_bbox;

        baby->radius_dmg = 0;
        baby->s.skinnum = 0;    // unconstructed
        baby->health = 1;

	baby->think = turret_boom_die_explode;
	baby->nextthink = level.time + 60;

        gi.linkentity (baby);
#ifndef DEBUG
        self->client->build_timer = level.time + 5;
        team_info.points[2] += COST_TURRET;
#endif
        //KillBox (self);
}

//kept for menu layout compatibility
void create_mgturret(edict_t *self)
{
        vec3_t forward, start;
        edict_t *baby;
        trace_t tr, tr2;
        vec3_t mins = {-24, -24, 0 };
        vec3_t maxs = {24 , 24 , 24};

        if (!turrettest->value)
        {
                gi.cprintf (self, PRINT_HIGH, "Machine gun turrets do not exist in this DLL, building regular turret.\n");
                create_turret (self);
                return;
        }

        if(self->client->build_timer > level.time)
        {
                gi.cprintf(self, PRINT_HIGH, "You must wait %d seconds.\n", (int)(self->client->build_timer - level.time));
                return;
        }

        if (team_info.buildflags & 4) {
                gi.cprintf(self, PRINT_HIGH, "Can't build a macheinegun turret here!\n");
                return;
        }

        if(team_info.points[2] + COST_MGTURRET > team_info.maxpoints[2])
        {
                gi.cprintf(self, PRINT_HIGH, "Not enough build points!\n");
                return;
        }

        AngleVectors (self->s.angles, forward, NULL, NULL);
        forward[2] = 0;
        VectorScale(forward, 128.0, forward);

        VectorAdd(self->s.origin, forward, forward);
        //forward[2] += ;       // move off ground a little.
        tr = gi.trace (self->s.origin, mins, maxs, forward, self, MASK_SHOT);
        if (gi.pointcontents (forward) == CONTENTS_SOLID || tr.fraction < 1.0 || tr.ent != world) {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }
        if (gi.pointcontents(tr.endpos)&CONTENTS_NOBUILD) {
                gi.cprintf(self, PRINT_HIGH, "Can\'t build a machinegun turret here!\n");
                return;
        }

        VectorCopy (forward,start);
        forward[2] -= 64;

        tr = gi.trace (start, mins, maxs, forward, self, MASK_SHOT);
        if (tr.allsolid || tr.startsolid) {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }

        if (tr.fraction !=  1.0)
                VectorCopy (tr.endpos, forward);

        tr2 = gi.trace (forward, NULL, NULL, self->s.origin, self, MASK_SHOT);
        if (tr2.fraction != 1.0) {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }

        forward[2] += 24;

        PMenu_Close(self);

        BuildLog (self, "mgturret", true);
        self->client->pers.idletime = 0;

        baby = G_Spawn();
        BuildLogEnt(self, baby);

        baby->spawnflags = 128;

        VectorCopy(forward, baby->s.origin);

        ClipBBoxToEights (baby);

        baby->s.angles[1] = self->s.angles[1];

        //baby->pain_debounce_time = level.time + 32.0;
        baby->classname = "mgturret";
        baby->spawnflags |= 64;
        baby->enttype = ENT_MGTURRET;

        // delay appearance
        SP_turret(baby);
        baby->s.sound = 0;
        baby->random = 0;
        //VectorSet (baby->mins, -8, -8, 0);
        //VectorSet (baby->maxs, 8, 8, 24);

        VectorClear (baby->mins);
        VectorClear (baby->maxs);

        baby->touch = fix_bbox;

        baby->radius_dmg = 0;
        baby->s.frame = 0;      // unconstructed
        baby->health = 1;

	baby->think = turret_boom_die_explode;
	baby->nextthink = level.time + 60;


        gi.linkentity (baby);
#ifndef DEBUG
        self->client->build_timer = level.time + 5;
        team_info.points[2] += COST_MGTURRET;
#endif
        //KillBox (self);
}

static void detector_fix_bbox (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
        if (other == world || other->flags & FL_CLIPPING) {
                VectorSet (ent->mins, -8, -8, 0);
                VectorSet (ent->maxs, 8, 8, 8);
                ent->solid = SOLID_BBOX;
                ent->touch = NULL;
                gi.linkentity (ent);
        }
}
void create_detector(edict_t *self)
{
        vec3_t forward;
        edict_t *baby;
        trace_t tr, tr2;
        vec3_t mins = {-8, -8, 0}, maxs = {8, 8, 8};//, end;

        if(self->client->build_timer > level.time){
                gi.cprintf(self, PRINT_HIGH, "You must wait %d seconds.\n", (int)(self->client->build_timer - level.time));
                return;
        }

        if (team_info.buildflags & 8) {
                gi.cprintf(self, PRINT_HIGH, "Can't build a detector here!\n");
                return;
        }

        if(team_info.points[2] + COST_DETECTOR > team_info.maxpoints[2])
        {
                gi.cprintf(self, PRINT_HIGH, "Not enough build points!\n");
                return;
        }

        AngleVectors (self->s.angles, forward, NULL, NULL);
        VectorScale(forward, 60, forward);

        VectorAdd(self->s.origin, forward, forward);

        tr = gi.trace (self->s.origin, mins, maxs, forward, self, MASK_SHOT);
        if (gi.pointcontents(tr.endpos)&CONTENTS_NOBUILD){
                gi.cprintf(self, PRINT_HIGH, "Can\'t lay a detector here!\n");
                return;
        }

        tr2 = gi.trace (forward, NULL, NULL, self->s.origin, self, MASK_SHOT);
        if (tr2.fraction != 1.0) {
                gi.cprintf(self, PRINT_HIGH, "No room!\n");
                return;
        }

        if (tr.fraction != 1.0) {
                VectorCopy (tr.endpos, forward);
        }

        // Don't allow on other items?

        PMenu_Close(self);

        BuildLog (self, "detector", true);
        self->client->pers.idletime = 0;

        baby = G_Spawn();
        BuildLogEnt(self, baby);

        VectorCopy(forward, baby->s.origin);
        ClipBBoxToEights (baby);
        baby->enttype = ENT_DETECTOR;

        baby->spawnflags = 0;
        baby->owner = self;

        baby->s.angles[1] = self->s.angles[1];

#ifndef DEBUG
        self->client->build_timer = level.time + 1;
        team_info.points[2] += COST_DETECTOR;
#endif
        // delay appearance
        SP_detector(baby);

        VectorClear (baby->mins);
        VectorClear (baby->maxs);
        baby->solid = SOLID_BBOX;
        baby->clipmask = MASK_SOLID | CONTENTS_MONSTER;
        baby->touch = detector_fix_bbox;
        gi.linkentity (baby);
}

void create_tripwire(edict_t *self)
{

        if(self->client->build_timer > level.time){
                gi.cprintf(self, PRINT_HIGH, "You must wait %d seconds.\n", (int)(self->client->build_timer - level.time));
                return;
        }

        if (team_info.buildflags & 16) {
                gi.cprintf(self, PRINT_HIGH, "Can't build a laser tripwire here!\n");
                return;
        }

        if(team_info.points[2] + COST_MINE > team_info.maxpoints[2])
        {
                gi.cprintf(self, PRINT_HIGH, "Not enough build points!\n");
                return;
        }

        PMenu_Close(self);

        CmdSetTripWire (self, false);
}

// Breeder & Engineer menus are opened at Think_Weapon

// Team mode menus

void DoClassMenus(void)
{
        int c;

        for (c=2;c<10;c++) {
                alien_menu[c].SelectFunc=SpawnClass;
                human_menu[c].SelectFunc=SpawnClass;
        }

        for (c=0;c<16;c++){
                switch (c){
                case CLASS_ENGINEER:
                case CLASS_GRUNT:
                case CLASS_SHOCK:
                case CLASS_BIO:
                case CLASS_HEAVY:
                case CLASS_COMMANDO:
                case CLASS_EXTERM:
                case CLASS_MECH:
                        if ((team_info.classes & classtypebit[c])){
                                human_menu[classlist[c].menu_number+1].text="X";
                                human_menu[classlist[c].menu_number+1].SelectFunc=NULL;
                        }else{
                                human_menu[classlist[c].menu_number+1].text= "";
                        }
                        break;
                default:
                        if ((team_info.classes & classtypebit[c])){
                                alien_menu[classlist[c].menu_number+1].text="X";
                                alien_menu[classlist[c].menu_number+1].SelectFunc=NULL;
                        }else{
                                alien_menu[classlist[c].menu_number+1].text= "" ;
                        }
                }
        }
}

void Activate_Upgrade (edict_t *ent)
{
        int             cost, found_ent, required;
        char    *upgrade_name;
        edict_t *other;

        if (ent->health <= 0)
        {
                PMenu_Close (ent);
                return;
        }

        upgrade_name = strstr (ent->client->menu.entries[ent->client->menu.cur].text, "- ");
        upgrade_name += 2;

        //this is HACK!!
        cost = atoi (ent->client->menu.entries[ent->client->menu.cur].text);

        if (ent->client->resp.score < cost && !sv_cheats->value)
        {
                gi.cprintf (ent, PRINT_HIGH, "You need %d frags for the %s.\n", cost, upgrade_name);
                return;
        }

        if (ent->client->resp.upgrades & (int)pow (2, ent->client->menu.cur - 3))
        {
                gi.cprintf (ent, PRINT_HIGH, "You already have %s.\n", upgrade_name);
                return;
        }

        found_ent = 0;
        other = NULL;

        if (ent->client->resp.team == TEAM_HUMAN)
                required = ENT_AMMO_DEPOT;
        else if (ent->client->resp.team == TEAM_ALIEN)
                required = ENT_HEALER;
        else
                return;

        while ((other = findradius (other, ent->s.origin, 150)))
        {
                if (other->enttype != required)
                        continue;

                if (!visible (ent, other))
                        continue;

                found_ent = 1;
                break;
        }

        if (!found_ent)
        {
                gi.cprintf (ent, PRINT_HIGH, "You must be near a %s to buy upgrades.\n", required == ENT_AMMO_DEPOT ? "depot" : "healer");
                return;
        }

        switch (ent->client->resp.class_type)
        {
                case CLASS_HEAVY:
                        switch (ent->client->menu.cur)
                        {
                                //stealth
                                case 3:
                                        break;

                                //shrap
                                case 4:
                                        break;
				//mortar
				case 5:
					break;

                                default:
                                        return;
                        }
                        break;
                case CLASS_COMMANDO:
                        switch (ent->client->menu.cur)
                        {
                                //ligaments
                                case 3:
                                        break;

                                //shrap
                                case 4:
                                        break;
                                //c4 refill
                                case 5:
                                        break;
                                default:
                                        return;
                        }
                        break;
                case CLASS_KAMIKAZE:
                        switch (ent->client->menu.cur)
                        {
                                //pouch
                                case 3:
                                        break;

                                default:
                                        return;
                        }
                        break;
                case CLASS_DRONE:
                        switch (ent->client->menu.cur)
                        {
                                //mucus
                                case 3:
                                        break;

                                default:
                                        return;
                        }
                        break;
                case CLASS_ENGINEER:
                        switch (ent->client->menu.cur)
                        {
                                //stealth
                                case 3:
                                        break;

                                default:
                                        return;
                        }
                        break;
                case CLASS_GRUNT:
                        switch (ent->client->menu.cur)
                        {
                                //stimpacks
                                case 3:
                                        ent->max_health -= STIMPACK_HEALTH_AMOUNT;
                                        break;

                                //smartgun
                                case 4:
                                        break;

                                default:
                                        return;
                        }
                        break;
                case CLASS_HATCHLING:
                        switch (ent->client->menu.cur)
                        {
                                //membrane
                                case 3:
                                        break;

                                default:
                                        return;
                        }
                        break;
                case CLASS_WRAITH:
                        switch (ent->client->menu.cur)
                        {
                                //acid
                                case 3:
                                        break;

                                default:
                                        return;
                        }
                        break;
                case CLASS_EXTERM:
                        switch (ent->client->menu.cur)
                        {
                                //charon boosters
                                case 3:
                                        break;

                                //supercharger
                                case 4:
                                        ent->client->resp.shot_type += 30;
                                        break;

                                //coolant
                                case 5:
                                        break;

                                default:
                                        return;
                        }
                        break;
                case CLASS_GUARDIAN:
                        switch (ent->client->menu.cur)
                        {
                                //cell wall
                                case 3:
                                        break;
                                default:
                                        return;
                        }
                        break;
                case CLASS_BIO:
                        switch (ent->client->menu.cur)
                        {
                                //glow grenade
                                case 3:
                                        break;
                                default:
                                        return;
                        }
                case CLASS_SHOCK:
                        switch (ent->client->menu.cur)
                        {
                                //equipment kit
                                case 3:
                                        break;
                                default:
                                        return;
                        }
                        break;
                case CLASS_MECH:
                        switch (ent->client->menu.cur)
                        {
                                //depot interface
                                case 3:
                                        break;

                                //doomsday
                                case 4:
                                        break;

                                default:
                                        return;
                        }
                        break;
                case CLASS_STALKER:
                        switch (ent->client->menu.cur)
                        {
                                //sac
                                case 3:

                                //synthesis
                                case 4:
                                        break;

                                default:
                                        return;
                        }
                        break;
                default:
                        gi.cprintf (ent, PRINT_HIGH, "You shouldn't be here!\n");
                        return;
        }

        ent->client->resp.upgrades |= (int)pow (2, ent->client->menu.cur - 3);

        // Special repurchaseable upgrades
        if ((ent->client->resp.class_type == CLASS_SHOCK && ent->client->resp.upgrades & UPGRADE_EQPT_KIT) ||
                (ent->client->resp.class_type == CLASS_COMMANDO &&  ent->client->resp.upgrades & UPGRADE_C4_KIT))
        {   
            // Shock trooper refill
            if (ent->upgrade_time - level.time < 0 && ent->client->resp.class_type == CLASS_SHOCK)
            {
                // Don't let the player buy this upgrade if he's not missing EX shells
                if (ent->client->resp.inventory[ITEM_INDEX(FindItem("Shell Clip [EX]"))] > 0) {
                    gi.cprintf(ent, PRINT_MEDIUM, "You can't carry any more EX shell clips.\n");
                    ent->client->resp.upgrades &= ~UPGRADE_EQPT_KIT;
                    return;
                }
                // Don't let the player buy this upgrade before reloading is done
                if (ent->client->weaponstate == WEAPON_RELOADING) {
                    ent->client->resp.upgrades &= ~UPGRADE_EQPT_KIT;
                    return;
                }
                ShotgunStart(ent);
                
                ent->health = classlist[ent->client->resp.class_type].health;
                //RepairArmor (ent, NULL, classlist[ent->client->resp.class_type].armorcount, 0);
                gi.sound (ent, CHAN_AUTO, SoundIndex (world_laser), 1, ATTN_IDLE, 0);
                ent->client->resp.upgrades &= ~UPGRADE_EQPT_KIT;
                ent->upgrade_time = level.time + 45;
            }
            // C4 Refill
            else if (ent->upgrade_time - level.time < 0 && ent->client->resp.class_type == CLASS_COMMANDO)
            {
                // Allow purchase only if player doesn't have C4
                if (ent->client->resp.inventory[ITEM_INDEX(FindItem("C4 Explosive"))] == 1) {
                    gi.cprintf(ent, PRINT_MEDIUM, "You already have a C4 explosive.\n");
                    ent->client->resp.upgrades &= ~UPGRADE_C4_KIT;
                    return;
                }
                ent->client->resp.inventory[ITEM_INDEX(FindItem("C4 Explosive"))] = 1;
                ent->client->resp.upgrades &= ~UPGRADE_C4_KIT;
                ent->upgrade_time = level.time + 120;
            }
            else {
                if (other->touch_debounce_time < level.time) {
                    gi.sound(other, CHAN_AUTO, SoundIndex (misc_keytry), 1, ATTN_NORM, 0);
                    other->touch_debounce_time = level.time +3;
                }
                gi.cprintf(ent, PRINT_MEDIUM, "You must wait %.1f seconds for another refill!\n", (ent->upgrade_time - level.time));
                if (ent->client->resp.class_type == CLASS_COMMANDO) ent->client->resp.upgrades &= ~UPGRADE_C4_KIT;
                if (ent->client->resp.class_type == CLASS_SHOCK) ent->client->resp.upgrades &= ~UPGRADE_EQPT_KIT;
                return;
            }
        }


        
        if (!sv_cheats->value)
                ent->client->resp.score -= cost;

        gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_refill), 1, ATTN_NORM, 0);

        gi.cprintf (ent, PRINT_HIGH, "You bought %s for %d frags!\n", upgrade_name, cost);
        PMenu_Close (ent);
}

void OpenMenu(edict_t *ent)
{
        if(ent->client->resp.team == TEAM_ALIEN || ent->client->resp.team == TEAM_HUMAN)
                OpenClassMenu (ent, false);
        else
        if (!PlayerQueueTest(&queue_join_aliens, ent) && !PlayerQueueTest(&queue_join_humans, ent))
                PMenu_Open(ent, team_menu, 2, 0);
        else
        if (!(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS)))
          PMenu_Open(ent, menu_exitqueue, 8, 0);
        else
          PMenu_Open(ent, menu_join_admin, 8, 0);
}

void OpenClassMenu (edict_t *ent, qboolean force)
{
        int menu_number;
        int chosen_team = 0;
        pmenuhnd_t *hnd=NULL;

        if (ISLINKBOT(ent)) return;

        if (ent->client->menu.entries == team_menu)
        {
                hnd = &ent->client->menu;

                if (hnd) {
                        if (hnd->cur == 2)
                                chosen_team = TEAM_ALIEN;
                        else if (hnd->cur == 4)
                                chosen_team = TEAM_HUMAN;
                }

        } else
                chosen_team = ent->client->resp.team;

        if (!hnd && forceteams->value && chosen_team)
                force = true;

        /*if (!ent->client->resp.team && chosen_team && chosen_team != ent->client->resp.team) {
                int currentplayers = team_info.numplayers[TEAM_ALIEN] + team_info.numplayers[TEAM_HUMAN];

                if (currentplayers >= maxplayers->value) {
                        gi.cprintf (ent, PRINT_HIGH, "All %d player slots are currently full, you may only observe.\n", (int)maxplayers->value);
                        return;
                }
        }*/

#ifdef LPBEVEN
        if(p && teameven->value) {
                if(ent->client->resp.team != TEAM_ALIEN && chosen_team == TEAM_ALIEN) {
                        if (teamlpbeven->value) {
                                int new_average;
                                if (teamlpbeven->value == 1) {
                                        if (team_info.numplayers[TEAM_ALIEN] && team_info.averageping[TEAM_ALIEN] < team_info.averageping[TEAM_HUMAN]) {
                                                new_average = GetPings(TEAM_ALIEN) + ent->client->ping;
                                                new_average /= (team_info.numplayers[TEAM_ALIEN] + 1);
                                                if (new_average < team_info.averageping[TEAM_ALIEN]) {
                                                        gi.cprintf(ent, PRINT_HIGH, "Too many LPBs on that team!\n");
                                                        return;
                                                }
                                        }
                                } else if (teamlpbeven->value == 2) {
                                        edict_t *ent=NULL;
                                        int i;
                                        int num_a_lpbs = 0;
                                        int num_h_lpbs = 0;

                                        for (i = 0; i < game.maxclients; i++) {
                                                ent = g_edicts + i + 1;
                                                if (!ent->inuse)
                                                        continue;
                                                if (ent->client->resp.team == TEAM_HUMAN && ent->client->ping < 100)
                                                        num_h_lpbs++;
                                                else if (ent->client->resp.team == TEAM_ALIEN && ent->client->ping < 100)
                                                        num_a_lpbs++;
                                        }

                                        if (num_a_lpbs > num_h_lpbs) {
                                                gi.cprintf(ent, PRINT_HIGH, "Too many LPBs on that team!\n");
                                                return;
                                        }
                                }
                        }
                } else if (ent->client->resp.team != TEAM_HUMAN && chosen_team == TEAM_HUMAN) {

                        if (teamlpbeven->value) {
                                if (teamlpbeven->value == 1) {
                                        int new_average;
                                        if (team_info.numplayers[TEAM_HUMAN] && team_info.averageping[TEAM_HUMAN] < team_info.averageping[TEAM_ALIEN]) {
                                                new_average = GetPings(TEAM_HUMAN) + ent->client->ping;
                                                new_average /= (team_info.numplayers[TEAM_HUMAN] + 1);
                                                if (new_average < team_info.averageping[TEAM_HUMAN]) {
                                                        gi.cprintf(ent, PRINT_HIGH, "Too many LPBs on that team!\n");
                                                        return;
                                                }
                                        }
                                } else if (teamlpbeven->value == 2) {
                                        edict_t *ent=NULL;
                                        int i;
                                        int num_a_lpbs = 0;
                                        int num_h_lpbs = 0;

                                        for (i = 0; i < game.maxclients; i++) {
                                                ent = g_edicts + i + 1;
                                                if (!ent->inuse)
                                                        continue;
                                                if (ent->client->resp.team == TEAM_HUMAN && ent->client->ping < 100)
                                                        num_h_lpbs++;
                                                else if (ent->client->resp.team == TEAM_ALIEN && ent->client->ping < 100)
                                                        num_a_lpbs++;
                                        }

                                        if (num_h_lpbs > num_a_lpbs) {
                                                gi.cprintf(ent, PRINT_HIGH, "Too many LPBs on that team!\n");
                                                return;
                                        }
                                }
                        }
                }
        }
#endif

        if (!TeamChange (ent, chosen_team, force))
                return;

        if (chosen_team == TEAM_ALIEN) {
                int c = PlayerQueueDataDefault(&queue_spawn_aliens, ent, CLASS_OBSERVER);
                if (c == CLASS_OBSERVER) c = ent->client->resp.class_type;

                        switch (c) {
                        case CLASS_BREEDER:
                                menu_number = 2;
                                break;
                        case CLASS_HATCHLING:
                                menu_number = 3;
                                break;
                        case CLASS_DRONE:
                                menu_number = 4;
                                break;
                        case CLASS_WRAITH:
                                menu_number = 5;
                                break;
                        case CLASS_KAMIKAZE:
                                menu_number = 6;
                                break;
                        case CLASS_STINGER:
                                menu_number = 7;
                                break;
                        case CLASS_GUARDIAN:
                                menu_number = 8;
                                break;
                        case CLASS_STALKER:
                                menu_number = 9;
                                break;
                        default:
                                menu_number = 3;
                                break;
                        }

                        if (!PlayerQueueTest(&queue_spawn_aliens, ent))
                          PMenu_Open(ent, alien_menu, menu_number, 0);
                        else
                          PMenu_Open(ent, alien_menu_exit, menu_number, 0);

        } else if (chosen_team == TEAM_HUMAN) {
                int c = PlayerQueueDataDefault(&queue_spawn_humans, ent, CLASS_OBSERVER);
                if (c == CLASS_OBSERVER) c = ent->client->resp.class_type;

                        switch (c) {
                        case CLASS_ENGINEER:
                                menu_number = 2;
                                break;
                        case CLASS_GRUNT:
                                menu_number = 3;
                                break;
                        case CLASS_SHOCK:
                                menu_number = 4;
                                break;
                        case CLASS_BIO:
                                menu_number = 5;
                                break;
                        case CLASS_HEAVY:
                                menu_number = 6;
                                break;
                        case CLASS_COMMANDO:
                                menu_number = 7;
                                break;
                        case CLASS_EXTERM:
                                menu_number = 8;
                                break;
                        case CLASS_MECH:
                                menu_number = 9;
                                break;
                        default:
                                menu_number = 3;
                                break;
                        }

                        if (!PlayerQueueTest(&queue_spawn_humans, ent))
                          PMenu_Open(ent, human_menu, menu_number, 0);
                        else
                          PMenu_Open(ent, human_menu_exit, menu_number, 0);

        }
}

void OpenClassMenuWrapper (edict_t *ent)
{
        OpenClassMenu (ent, false);
}

void CTFChaseCam(edict_t *ent)
{
        int i;
        edict_t *e;

        PMenu_Close(ent);

        // check if chasing
        if (ent->client->chase_target) {

                // reset chasing variables and return
                ent->client->chase_target = NULL;
                ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
                ent->client->ps.pmove.pm_type = PM_NORMAL;
                return;
        }

        // check if observer
        if (ent->client->resp.team != TEAM_NONE) {

                // change team to observer team, return if not allowed for some reason
                /*if (!TeamChange (ent, TEAM_NONE, false)) {
                        return;
                }*/

                // change our team
                JoinTeam0 (ent);
        } else {

        // choose first available player to chase
        for (i = 1; i <= game.maxclients; i++) {
                e = g_edicts + i;
                if (e->inuse && e->solid != SOLID_NOT) {
                        ent->client->chase_target = e;
                        e->client->resp.chased = true;
                        ent->client->chase_view = 0;
                        ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
                        ent->client->ps.pmove.pm_type = PM_FREEZE;
                        break;
                }
        }
        }
}

// Regular mode menus ///////////////////////////////////////////////

void Updateteam_menu(void)
{
        edict_t *ent;

        static char team1players[32];
        static char team2players[32];
        static char team3players[32];
        
        static char alienjoinstatus[32];
        static char humanjoinstatus[32];

        gi.AddCommandString (va("set curplayers \"%d\" s\n", team_info.numplayers[TEAM_ALIEN] + team_info.numplayers[TEAM_HUMAN]));

        sprintf(team1players, "  %d player%s", team_info.numplayers[TEAM_ALIEN], (team_info.numplayers[TEAM_ALIEN] != 1) ? "s" : "");
        sprintf(team2players, "  %d player%s", team_info.numplayers[TEAM_HUMAN], (team_info.numplayers[TEAM_HUMAN] != 1) ? "s" : "");
        sprintf(team3players, "  %d observer%s", team_info.numplayers[TEAM_NONE], (team_info.numplayers[TEAM_NONE] != 1) ? "s" : "");

        team_menu[3].text = team1players;
        team_menu[5].text = team2players;
        team_menu[7].text = team3players;

        sprintf(alienjoinstatus, "  %d alien%s (%d queued)", team_info.numplayers[TEAM_ALIEN], (team_info.numplayers[TEAM_ALIEN] != 1) ? "s" : "", queue_join_aliens.count);
        sprintf(humanjoinstatus, "  %d human%s (%d queued)", team_info.numplayers[TEAM_HUMAN], (team_info.numplayers[TEAM_HUMAN] != 1) ? "s" : "", queue_join_humans.count);

        menu_join_admin[4].text = menu_exitqueue[4].text = alienjoinstatus;
        menu_join_admin[5].text = menu_exitqueue[5].text = humanjoinstatus;
        menu_join_admin[6].text = menu_exitqueue[6].text = team3players;

        for (ent = g_edicts + 1; ent <= g_edicts + game.maxclients; ent++)
        {
                if (!ent->inuse)
                        continue;

                if (ent->client->menu.entries == team_menu)
                        PMenu_Update(ent);
        }
}

void Back(edict_t *ent)
{
        PMenu_Open(ent, team_menu, 2, 0);
}

/////////////////////////////////////////////////////////////////
// Engineer Functions
//void SpawnDamage (int type,vec3_t origin,vec3_t normal, int damage);
void tripwire_activate (edict_t *mine);
void tripwire_think (edict_t *self);
void CorruptedMineThink (edict_t *self);
void SP_info_player_deathmatch_use (edict_t *self, edict_t *other, edict_t *activator);
void engineer_fix (edict_t *self, float dist)
{
        vec3_t          end;
        trace_t         tr;
        edict_t         *ignore, *ent;
        int                     damage, frameBoost;
        vec3_t          angles, forward, start,mins,maxs;

        VectorAdd (self->client->v_angle, self->client->kick_angles, angles);
        VectorCopy (self->s.origin, start);
        start[2] += self->viewheight;
        AngleVectors (angles, forward, NULL, NULL);

        VectorMA (start, dist, forward, end);
        ignore = self;

        VectorSet(mins,-16,-16,-16);
        VectorSet(maxs,16,16,16);

        if(randomMT() & 1)
        {
                self->s.frame = ENG_WORKA_S;
                self->client->anim_end = ENG_WORKA_E;
                frameBoost = 3;
        }
        else
        {
                self->s.frame = ENG_WORKB_S;
                self->client->anim_end = ENG_WORKB_E;
                frameBoost = 6;
        }

        self->client->anim_priority = ANIM_ATTACK;

        tr = gi.trace (start, mins, maxs, end, ignore, CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

        //if (self->client)
//              PlayerNoise(self, tr.endpos, PNOISE_IMPACT);

        if (tr.fraction == 1.0 || !CanDamage (tr.ent, self))
        {
                tr = gi.trace (start, NULL, NULL, end, ignore, MASK_SHOT);
        }

        if (tr.fraction == 1.0)
        {
                if (self->client->resp.upgrades & UPGRADE_PHOBOS_REACTOR)
                        self->s.frame += frameBoost;
                return;
        }
        else
        {
                ent = tr.ent;

                if(ent->enttype == ENT_TURRETBASE && tr.plane.normal[2] > -0.5f)
                  ent = ent->target_ent;
                
                
                //Extend time after a poke if needed so doesn't blow up while being poked
                if (ent->think == turret_boom_die_explode && ent->nextthink > level.time && ent->nextthink < level.time+30)
        	  ent->nextthink = level.time + 30;


                if(ent->enttype == ENT_TELEPORTER)
                {
                	self->spawnflags |= 32768; //If poked at least once, refund on death is subject to reduced refund

                        if (ent->health < ent->max_health) {

                                // heal the tele
                                ent->health += 30;

                                // cap it and add particle effect back
                                if (ent->health >= ent->max_health) {
                                        ent->health = ent->max_health;
                                        ent->s.effects = EF_TELEPORTER;

                                        ent->nextthink = 0;
                                        ent->think = NULL;
                                }

                                if(ent->spawnflags & 1024 && ent->health >= (ent->max_health/2))
                                {
                                        // healed built tele over 50%, activate tele

                                        ent->teleport_time = level.time + 5;
                                        ent->pain = spawn_pain;
                                        ent->think = NULL;
                                        ent->nextthink = 0;
                                        //ent->clipmask = MASK_SHOT;
                                        ent->s.skinnum = 1;
                                        ent->damage_absorb = 30;
                                        ent->use = SP_info_player_deathmatch_use;

                                        team_info.spawns[TEAM_HUMAN]++;

                                        ent->spawnflags &= ~1024;

                                        AddToSpawnlist(ent, TEAM_HUMAN);
                                }

                                if (!(ent->spawnflags & 1024)) {
                                  ent->nextthink = ent->teleport_time;
                                  if (ent->nextthink < level.time+FRAMETIME) ent->nextthink = level.time+FRAMETIME;
                                  ent->think = SpawnProcessQueue;
                                }
                        }

                        // print status info
                        damage = 100 - (int)(((float)tr.ent->health / (float)tr.ent->max_health) * 100.0f);

                        gi.cprintf(self, PRINT_HIGH, "Repairs left: %d%%\n", damage);
                }
                else if (ent->enttype == ENT_TURRET || ent->enttype == ENT_MGTURRET) {
                	self->spawnflags |= 32768; //If poked at least once, refund on death is subject to reduced refund

                        if (ent->health > 0)     {
                                ent->newcount++;
                                if (ent->enttype == ENT_TURRET) {
                                        ent->health += 52;
                                        if (!ent->random && ++ent->dmg % 2 == 0)
                                                ent->s.frame++;
                                        //if (ent->dmg > 25)
                                                //ent->dmg = 25;
                                } else {
                                        ent->health += 15;
                                        /*ent->dmg += 20;
                                        if (ent->dmg > 200)
                                                ent->dmg = 200;*/
                                }

                                if(ent->health >= ent->max_health)
                                        ent->health = ent->max_health;

                                //r1: upddate health on base also
                                ent->target_ent->health = ent->health;

                                damage = ((ent->max_health - ent->health)*100)/ent->max_health;

                                gi.cprintf(self, PRINT_HIGH, "Repairs left: %d%%\n", damage);

                                if(!damage)
                                {
                                        if (!ent->random) {
                                                gi.sound (ent, CHAN_AUTO, SoundIndex (misc_activate), 1.0, ATTN_IDLE, 0);
                                                ent->random = ent->target_ent->random = 1;
                                        }
                                        //ent->s.sound = SoundIndex (misc_turrethum);
                                        //ent->radius_dmg = 0;
                                        if (ent->spawnflags & 64)
                                                ent->think = mgturret_think;
                                        else
                                                ent->think = turret_think;
                                        ent->nextthink = level.time + 3;
                                }
                        }
                } else if (ent->enttype == ENT_AMMO_DEPOT) {
                        ent->health += 150;
                        if (ent->health > ent->max_health) ent->health = ent->max_health;
                        gi.cprintf(self, PRINT_HIGH, "Repairs left: %d%%\n", 100-(ent->health*100)/ent->max_health);
                } else {
                        edict_t *sent = NULL;
                        while ((sent = findradius(sent, tr.endpos, 30)) != NULL) {
                                if (tripwire_repair_count->value || (sent->spawnflags & SPAWNFLAG_CORRUPTED_MINE)) {
                                    
                                        if (sent->enttype == ENT_TRIPWIRE_BOMB && !sent->owner && !(sent->spawnflags & SPAWNFLAG_CORRUPTED_MINE)) {
                                                sent->mass++;
                                                if (sent->mass == (int)tripwire_repair_count->value)
                                                        tripwire_activate (sent);
                                                else
                                                        sent->nextthink = level.time + 60;
                                                break;
                                        }
                                        // Corrupted mine/tripwire repair
                                        else if (sent->spawnflags & SPAWNFLAG_CORRUPTED_MINE) {
                                            sent->health += 45;
                                            if (sent->health > sent->max_health) sent->health = sent->max_health;
                                            gi.cprintf(self, PRINT_HIGH, "Repairs left: %d%%\n", 100-(ent->health*100)/ent->max_health);
                                            if (sent->health == sent->max_health) {
                                                sent->s.renderfx &= ~RF_IR_VISIBLE;
                                                sent->spawnflags &= ~SPAWNFLAG_CORRUPTED_MINE;
                                                sent->think = tripwire_think;
                                                tripwire_think(sent);
                                                sent->s.frame = 4;
                                                gi.linkentity(sent);
                                                gi.sound (ent, CHAN_AUTO, SoundIndex (misc_activate), 1.0, ATTN_IDLE, 0);
                                                break;
                                            }
                                       }
                                }
                        }
    
                }

                if (ent->enttype == ENT_CORPSE || ent->enttype == ENT_HUMAN_BODY || ent->enttype == ENT_NOBLOOD_BODY) {
                        T_Damage (ent, self, ent, forward, tr.endpos, tr.plane.normal, 50, 30, 0, MOD_POKED);
                } else if ((ent->solid == SOLID_BSP || ent->enttype == ENT_PRACTICE_TARGET) && ent->takedamage) {
                        T_Damage (ent, self, ent, forward, tr.endpos, tr.plane.normal, 10, 30, 0, MOD_POKED);
                }

                if(ent->client)
                {
                        if (ent->client->resp.team==TEAM_HUMAN){
                                if(classlist[ent->client->resp.class_type].healfunc && ent->health > 0)
                                        classlist[ent->client->resp.class_type].healfunc(ent, self);
                                // r1: anti spam test
                        }else{
                                /*gi.WriteByte (svc_temp_entity);
                                gi.WriteByte (TE_SPARKS);
                                gi.WritePosition (ent->s.origin);
                                gi.WriteDir (ent->velocity);
                                gi.multicast (ent->s.origin, MULTICAST_PVS);*/
                                //ent->client->frozentime += 1;
                                //ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
                                T_Damage (ent, self, self, forward, tr.endpos, tr.plane.normal, 10, 25, 0, MOD_POKED);
                        }

                        if (self->client->resp.upgrades & UPGRADE_PHOBOS_REACTOR)
                                self->s.frame += frameBoost;
                }

                if(randomMT() & 1)
                        gi.sound (self, CHAN_AUTO, SoundIndex (world_spark1), 1, ATTN_NORM, 0);
                else
                        gi.sound (self, CHAN_AUTO, SoundIndex (world_spark3), 1, ATTN_NORM, 0);

                tr = gi.trace (start, NULL, NULL, end, ignore, MASK_SHOT);

                gi.WriteByte(svc_temp_entity);
                gi.WriteByte(TE_LASER_SPARKS);
                gi.WriteByte(24);
                gi.WritePosition(tr.endpos);
                gi.WriteDir(tr.plane.normal);
                gi.WriteByte(226);
                gi.multicast(tr.endpos, MULTICAST_PVS);
        }
}

void eng_deconstruct (edict_t *self, float dist)
{
        vec3_t          end;
        trace_t         tr;
        edict_t         *ignore, *ent;
        int                     mask;
        vec3_t          angles, forward, start, mins, maxs;

        if (self->health < 1)
                return;

        if(randomMT() & 1)
        {
                self->s.frame = ENG_WORKA_S;
                self->client->anim_end = ENG_WORKA_E;
        }
        else
        {
                self->s.frame = ENG_WORKB_S;
                self->client->anim_end = ENG_WORKB_E;
        }
        self->client->anim_priority = ANIM_ATTACK;

        VectorAdd (self->client->v_angle, self->client->kick_angles, angles);
        VectorCopy (self->s.origin, start);
        start[2] += self->viewheight;
        AngleVectors (angles, forward, NULL, NULL);

        VectorMA (start, dist, forward, end);
        ignore = self;
        mask = MASK_SHOT;

        VectorSet(mins,-4,-4,-4);
        VectorSet(maxs,4,4,4);

        tr = gi.trace (start, mins, maxs, end, ignore, mask);

        if (tr.fraction >= 1.0)
        {
                //r1: detector finder!(TM)
                edict_t *detector = NULL;
                while ((detector = findradius(detector,self->s.origin,500)) != NULL) {
                        if (detector->enttype == ENT_DETECTOR) {
                                gi.WriteByte (svc_temp_entity);
                                gi.WriteByte (TE_BFG_LASER);
                                gi.WritePosition (self->s.origin);
                                gi.WritePosition (detector->s.origin);
                                gi.unicast (self,false);
                        }
                }
                return;
        }
        else
        {
                ent = tr.ent;

                if(ent->enttype == ENT_TURRETBASE && tr.plane.normal[2] > -0.5f)
                  ent = ent->target_ent;

                if(ent->enttype == ENT_TURRET ||
                        ent->enttype == ENT_MGTURRET ||
                        ent->enttype == ENT_DETECTOR ||
                        (ent->enttype == ENT_AMMO_DEPOT && !(ent->spawnflags & 4096)) ||
                        (ent->enttype == ENT_TELEPORTER && ent->spawnflags & 1024))
                {
                        if (!(ent->style && ent->style & classtypebit[self->client->resp.class_type])) {
                                if (ent->enttype == ENT_AMMO_DEPOT)
                                        BuildLog (self, "depot", false);
                                else if (ent->enttype == ENT_TELEPORTER)
                                        BuildLog (self, "teleporter", false);
                                ent->health = 0;
                                ent->die(ent, ent, ent, 0, ent->s.origin);
                        }

                }else{
                        edict_t *sent = NULL;
                        while ((sent = findradius(sent, tr.endpos, 30)) != NULL)
                        {
                                if (sent->enttype == ENT_DETECTOR || sent->enttype == ENT_TRIPWIRE_BOMB) {
                                        ent = sent;
                                        ent->radius_dmg/=2;
                                        if (!(ent->style && ent->style & classtypebit[self->client->resp.class_type])) {
                                                BuildLog (self, ent->classname, false);
                                                ent->health=0;
                                                ent->die(ent, ent, ent, 0, ent->s.origin);
                                        }
                                }
                        }
                }

                if (ent == world)
                        return;

                if(randomMT() & 1)
                        gi.sound (self, CHAN_AUTO, SoundIndex (world_spark1), 1, ATTN_NORM, 0);
                else
                        gi.sound (self, CHAN_AUTO, SoundIndex (world_spark3), 1, ATTN_NORM, 0);

                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_SPARKS);
                gi.WritePosition (tr.endpos);
                gi.WriteDir (tr.plane.normal);
                gi.multicast (tr.endpos, MULTICAST_PVS);
        }
}

/*void biotech_magic_thingy (edict_t *self)
{
        health_multiplier
}*/

void biotech_heal (edict_t *self, float dist)
{
        vec3_t          end;
        trace_t         tr;
        edict_t         *ignore, *ent = NULL;
        int                     mask;
        vec3_t          angles, forward, start, mins, maxs;

        self->s.frame = FRAME_point01;
        self->client->anim_end = FRAME_point12;

        self->client->anim_priority = ANIM_ATTACK;

        VectorAdd (self->client->v_angle, self->client->kick_angles, angles);
        VectorCopy (self->s.origin, start);
        start[2] += self->viewheight;
        AngleVectors (angles, forward, NULL, NULL);

        VectorMA (start, dist, forward, end);
        ignore = self;

        mask = MASK_SHOT;

        VectorSet(mins,-16,-16,-16);
        VectorSet(maxs,16,16,16);

        tr = gi.trace (start, mins, maxs, end, ignore, mask);

        //if (self->client)
        //      PlayerNoise(self, tr.endpos, PNOISE_IMPACT);

        if (tr.fraction == 1.0)
        {
                int donestuff;
                int rounderror = 0;

                if (self->wait > level.time)
                        return;

                if (self->client->resp.inventory[ITEM_AMMO_CELLS] < 5) {
                        gi.cprintf (self, PRINT_HIGH, "Not enough cells for area heal!\n");
                        self->wait = level.time + 1;
                        return;
                } else {
                        self->wait = level.time + .1f;
                }

                donestuff = 0;

                while ((ent = findradius_c (ent, self, 500)) != NULL) {

                        if (ent == self)
                                continue;

                        if (ent->client->resp.team == TEAM_HUMAN && CanDamage(self, ent)) {
                                if ((int)dmflags->value & DF_SLOW_HEALTH_PACK) {
                                  vec3_t vdist = {ent->s.origin[0]-self->s.origin[0], ent->s.origin[1]-self->s.origin[1], ent->s.origin[2]-self->s.origin[2]};
                                  float  dist = VectorLength(vdist);
                                  int    max = (15*250)/(250+dist);
                                  int    heal = ent->max_health - ent->health - ent->client->resp.health_regen;

                                  if (ent->client->resp.health_regen > max/2 || ent->client->resp.health_regen_time >= level.time) heal = 0;
                                  
                                  if (heal > 0) {
                                    if (heal > max) heal = max;

                                    if (heal > self->client->resp.inventory[ITEM_AMMO_CELLS]*2) {
                                      if (!self->client->resp.inventory[ITEM_AMMO_CELLS]) {
                                        gi.cprintf (self, PRINT_HIGH, "Not enough cells for area heal!\n");
                                        self->wait = level.time + 1;
                                        break;
                                      }
                                      heal = self->client->resp.inventory[ITEM_AMMO_CELLS]*2;
                                    }


                                    self->client->resp.inventory[ITEM_AMMO_CELLS] -= (heal+rounderror)/2;
                                    rounderror ^= heal&1;
 
                                    ent->client->resp.health_regen += heal;
                                    if (ent->client->resp.health_regen_time < level.time+0.09f)
                                      ent->client->resp.health_regen_time = level.time+0.09f;
                                  }
                                } else
                                if (ent->health < ent->max_health) {
                                        if (self->client->resp.inventory[ITEM_AMMO_CELLS] < 1) {
                                                gi.cprintf (self, PRINT_HIGH, "Not enough cells for area heal!\n");
                                                self->wait = level.time + 1;
                                                break;
                                        }

                                        ent->health += 10;

                                        if (ent->pain_debounce_time < level.time) {
                                                self->client->resp.bioScore += 10;
                                                donestuff += 10;
                                        }

                                        if (ent->health > ent->max_health)
                                                ent->health = ent->max_health;

                                        self->client->resp.inventory[ITEM_AMMO_CELLS] --;
                                }

                                if (ent->client->resp.parasited) {
                                        if (self->client->resp.inventory[ITEM_AMMO_CELLS] < 3) {
                                                gi.cprintf (self, PRINT_HIGH, "Not enough cells for parasite cure!\n");
                                                self->wait = level.time + 1;
                                                break;
                                        }
                                        
                                        ent->client->resp.parasited = 0;

                                        donestuff += 15;
                                        gi.cprintf (ent, PRINT_HIGH, "%s cured you of a parasite!\n", self->client->pers.netname);
                                        gi.cprintf (self, PRINT_HIGH, "Cured %s of a parasite!\n", ent->client->pers.netname);

                                        self->client->resp.total_score++;
                                        self->client->resp.bioScore += 20;
                                        self->client->resp.inventory[ITEM_AMMO_CELLS] -= 3;

                                }

                                if (ent->client->acid_duration)
                                {

                                        if (self->client->resp.inventory[ITEM_AMMO_CELLS] < 5) {
                                                gi.cprintf (self, PRINT_HIGH, "Not enough cells for poison cure!\n");
                                                self->wait = level.time + 1;
                                                break;
                                        }

                                        ent->client->acid_damage = ent->client->acid_duration = 0;
                                        ent->client->acid_attacker = 0;

                                        donestuff += 15;
                                        gi.cprintf (ent, PRINT_HIGH, "%s cured you of poison!\n", self->client->pers.netname);
                                        gi.cprintf (self, PRINT_HIGH, "Cured %s of poison!\n", ent->client->pers.netname);

                                        self->client->resp.total_score++;
                                        self->client->resp.bioScore += 25;
                                        self->client->resp.inventory[ITEM_AMMO_CELLS] -= 5;
                                }
                        }
                }
                if (donestuff) {
                        gi.sound (self, CHAN_AUTO, SoundIndex (world_laser), 1, ATTN_IDLE, 0);
                        self->random += donestuff;
                        if (self->random > 100) {
                                self->client->resp.total_score++;
                                self->random -= 100;
                        }
                }
        }
        else
        {
                ent = tr.ent;
                if(ent->client && ent->client->resp.team==TEAM_HUMAN)
                {
                        if(classlist[ent->client->resp.class_type].healfunc)
                                classlist[ent->client->resp.class_type].healfunc(ent, self);
                        gi.sound (self, CHAN_AUTO, SoundIndex (world_laser), 1, ATTN_IDLE, 0);
                }
        }
}


void breeder_deconstruct (edict_t *self, float dist)
{
        vec3_t          end;
        trace_t         tr;
        edict_t         *ignore, *ent;
        int                     mask;
        vec3_t          angles, forward, start;

        if (self->health < 1)
                return;

        VectorAdd (self->client->v_angle, self->client->kick_angles, angles);
        VectorCopy (self->s.origin, start);
        start[2] += self->viewheight;
        AngleVectors (angles, forward, NULL, NULL);

        VectorMA (start, dist, forward, end);
        ignore = self;
        mask = MASK_SHOT;

        tr = gi.trace (start, NULL, NULL, end, ignore, mask);

//      if (self->client)
                //PlayerNoise(self, tr.endpos, PNOISE_IMPACT);

        if (tr.fraction >= 1.0 && !tr.startsolid)
        {
                return;
        }
        else
        {
                ent = tr.ent;
                if (ent->enttype == ENT_HEALER || ent->enttype == ENT_SPIKER ||
                        ent->enttype == ENT_OBSTACLE || ent->enttype == ENT_GASSER) {
                        if (!(ent->style && ent->style & classtypebit[self->client->resp.class_type])) {
                                char *p;
                                switch (ent->enttype) {
                                case ENT_HEALER:
                                        p = "healer";
                                        break;
                                case ENT_SPIKER:
                                        p = "spiker";
                                        break;
                                case ENT_OBSTACLE:
                                        p = "obstacle";
                                        break;
                                case ENT_GASSER:
                                        p = "gasser";
                                        break;
                                default:
                                        p = "something";
                                        break;
                                }
                                BuildLog (self, p, false);
                                ent->health = 0;
                                ent->die(ent, ent, ent, 0, ent->s.origin);
                        }
                }
        }
}

// Ident
static void loc_buildboxpoints(vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
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

qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
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
                trace = gi.trace (viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
                if (trace.fraction == 1.0)
                        return true;
        }

        return false;
}

#define MASK_VOLUME                     1
#define MASK_ATTENUATION        2
#define MASK_POSITION           4
#define MASK_ENTITY_CHANNEL     8
#define MASK_TIMEOFS            16

void unicastSound(edict_t *player, int soundIndex, float volume)
{
        int mask = MASK_ENTITY_CHANNEL;

        if (volume != 1.0)
                mask |= MASK_VOLUME;

        gi.WriteByte (svc_sound);
        gi.WriteByte ((byte)mask);
        gi.WriteByte ((byte)soundIndex);

        if (mask & MASK_VOLUME)
                gi.WriteByte ((byte)(volume * 255));

        gi.WriteShort(((player - g_edicts - 1) << 3) + CHAN_NO_PHS_ADD);
        gi.unicast (player, true);
}

void teamcastSound(int t, int soundIndex, float volume)
{
        edict_t *ent;

        for (ent = &g_edicts[1] ; ent<=&g_edicts[game.maxclients] ; ent++)
        {
                if (ent->inuse && ent->client->resp.team == t && ent->health > 0)
                {
                        unicastSound(ent, soundIndex, volume);
                }
        }
}




void unicastEntitySound(edict_t *player, edict_t*ent, int soundIndex, int atten, float volume)
{
  int mask = 0;
  int attenbyte = atten*64;
  int volbyte = volume*255;
  if (attenbyte > 255) attenbyte = 255, atten = 4;
  if (volbyte > 255) volbyte = 255, volume = 1;

  if (!player->client || !player->client->pers.connected || volbyte <= 0 || attenbyte < 0) return;
        
  if (attenbyte) {
    float distance;
    vec3_t dist;

    VectorSubtract(player->s.origin, ent->s.origin, dist);
    distance = (VectorLength(dist)-80)*atten;

    if (distance > volume+0.5f) return;
  }

  if (attenbyte != 64)
    mask |= MASK_ATTENUATION;

  if (!gi.inPHS(player->s.origin, ent->s.origin)) return;

  if (volbyte != 255)
    mask |= MASK_VOLUME;
        
  if (gi.inPVS(player->s.origin, ent->s.origin))
    mask |= MASK_ENTITY_CHANNEL;
  else
    mask |= MASK_POSITION;

  gi.WriteByte(svc_sound);
  gi.WriteByte((uint8_t)mask);
  gi.WriteByte((uint8_t)soundIndex);

  if (mask & MASK_VOLUME)
    gi.WriteByte(volbyte);

  if (mask & MASK_ATTENUATION)
    gi.WriteByte(attenbyte);

  if (mask & MASK_ENTITY_CHANNEL)
    gi.WriteShort(((ent - g_edicts -1) << 3) | CHAN_NO_PHS_ADD);
  else
    gi.WritePosition(ent->s.origin);

  gi.unicast(player, false);
}


void teamcastUSound(edict_t*player, int t, int soundIndex, float volume)
{
  edict_t *ent;

  if (player) {
    if (!player->inuse) return;
    if (!player->client) player = NULL;
  }

  for(ent = &g_edicts[1]; ent<=&g_edicts[game.maxclients]; ent++)
    if (ent->inuse && ent->client->resp.team == t && ent->health > 0 && (!player || !ent->client->pers.ignorelist[player->s.number-1]))
      unicastSound(ent, soundIndex, volume);
}

void broadcastUSound(edict_t*speaker, int soundIndex, float volume, int atten)
{
  gi.sound(speaker, CHAN_VOICE, soundIndex, volume, atten, 0);
  /*FIXME

  edict_t *ent;
  
  if (!speaker) return;
  
  for(ent = &g_edicts[1]; ent<=&g_edicts[game.maxclients]; ent++)
    if (ent->inuse && (!speaker->client || !ent->client->pers.ignorelist[speaker->s.number-1]))
      unicastEntitySound(ent, speaker, soundIndex, atten, volume);
  */
}


// flashlight plug
// think routine at end of p_hud.c
void FL_make(edict_t *self)
{
        int myeffect;

        if ( self->client->resp.flashlight )
        {
                // turn it on/off (optimized)
                if (self->client->resp.flashlight->svflags ^= SVF_NOCLIENT) {
                        gi.sound(self, CHAN_AUTO, SoundIndex (weapons_noammo), 1, ATTN_STATIC, 0);
                }

                return;
        }

        self->client->resp.flashlight = G_Spawn ();
        self->client->resp.flashlight->owner = self;
        self->client->resp.flashlight->movetype = MOVETYPE_NOCLIP;
        self->client->resp.flashlight->solid = SOLID_NOT;
        self->client->resp.flashlight->classname = "flashlight";
        self->client->resp.flashlight->s.modelindex = shiny_index;

        // set the color
        switch ((int)flashlight_mode->value){
                default:
                case 0:
                        myeffect = EF_HYPERBLASTER;
                        break;
                case 1:
                        myeffect = EF_BFG;
                        break;
                case 2:
                        myeffect = EF_BLUEHYPERBLASTER;
                        break;
                case 3:
                        myeffect = EF_PLASMA;
                        break;
                case 4:
                        myeffect = 0x04000000; //dynamic darkness
                        break;
                case 5:
                        myeffect = 0x84000000;
                        break;
        }

        self->client->resp.flashlight->s.effects = myeffect;
}
