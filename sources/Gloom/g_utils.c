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

// g_utils.c -- misc utility functions for game module

#include "g_local.h"
#include <stdio.h>

void G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
        result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1];
        result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1];
        result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] + distance[2];
}

void debugline (vec3_t start, vec3_t end)
{
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_DEBUGTRAIL);
        gi.WritePosition (start);
        gi.WritePosition (end);
        gi.multicast (NULL, MULTICAST_ALL);
}

float Distance (vec3_t o1, vec3_t o2)
{
        vec3_t  tmp;
        VectorSubtract (o1, o2, tmp);
        return VectorLength (tmp);
}

/*
G_Find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

*/
edict_t *G_Find (edict_t *from, int fieldofs, char *match)
{
        char    *s;

        if (!from)
                from = g_edicts;
        else
                from++;

        for ( ; from < &g_edicts[globals.num_edicts] ; from++)
        {
                if (!from->inuse)
                        continue;
                s = *(char **) ((byte *)from + fieldofs);
                if (!s)
                        continue;
                if (!Q_strcasecmp (s, match))
                        return from;
        }

        return NULL;
}

edict_t *G_Find2 (edict_t *from, int fieldofs, char *match)
{
        char    *s;

        if (!from)
                from = g_edicts;
        else
                from++;

        for ( ; from < &g_edicts[globals.num_edicts] ; from++)
        {
                if (!from->inuse)
                        continue;
                s = *(char **) ((byte *)from + fieldofs);
                if (!s)
                        continue;
                if (!Q_strncasecmp (s, match, strlen(match)))
                        return from;
        }

        return NULL;
}

edict_t *G_Find3 (edict_t *from, int match)
{
        if (!from)
                from = g_edicts;
        else
                from++;

        for ( ; from < &g_edicts[globals.num_edicts] ; from++)
        {
                if (!from->inuse)
                        continue;
                if (from->enttype == match)
                        return from;
        }

        return NULL;
}

/*
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
*/
edict_t *findradius (edict_t *from, vec3_t org, float rad)
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
                if (from->solid == SOLID_NOT)
                        continue;
                for (j=0 ; j<3 ; j++)
                        eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5f);
                if (VectorLength(eorg) > rad)
                        continue;
                return from;
        }

        return NULL;
}

/*
findradius_all

Returns entities that have origins within a spherical area. Includes none-solid ents.

findradius (origin, radius)
*/
edict_t *findradius_all (edict_t *from, vec3_t org, float rad)
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
                for (j=0 ; j<3 ; j++)
                        eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5f);
                if (VectorLength(eorg) > rad)
                        continue;
                return from;
        }

        return NULL;
}

/*
findradius_c

Returns clients that have origins within a spherical area

findradius_c (origin, radius)
*/
edict_t *findradius_c (edict_t *from_ent, edict_t *from_who, float rad)
{
        vec3_t  eorg;

        if (!from_ent)
                from_ent = g_edicts+1;
        else
                from_ent++;

        for ( ; from_ent->client ; from_ent++)
        {
                if (!from_ent->inuse)
                        continue;
                if (from_ent->health <= 0)
                        continue;
                VectorSubtract (from_ent->s.origin,from_who->s.origin,eorg);

                if (VectorLength(eorg) > rad)
                        continue;
                return from_ent;
        }

        return NULL;
}

/*
G_PickTarget

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

*/
#define MAXCHOICES      8

edict_t *G_PickTarget (char *targetname)
{
        edict_t *ent = NULL;
        int             num_choices = 0;
        edict_t *choice[MAXCHOICES];

        if (!targetname)
        {
                gi.dprintf("G_PickTarget called with NULL targetname\n");
                return NULL;
        }

        while(1)
        {
                ent = G_Find (ent, FOFS(targetname), targetname);
                if (!ent)
                        break;
                choice[num_choices++] = ent;
                if (num_choices == MAXCHOICES)
                        break;
        }

        if (!num_choices)
        {
                gi.dprintf("G_PickTarget: target %s not found\n", targetname);
                return NULL;
        }

        return choice[randomMT() % num_choices];
}



void Think_Delay (edict_t *ent)
{
        G_UseTargets (ent, ent->activator);
        G_FreeEdict (ent);
}

/*
G_UseTargets

the global "activator" should be set to the entity that initiated the firing.

If self.delay is set, a DelayedUse entity will be created that will actually
do the SUB_UseTargets after that many seconds have passed.

Centerprints any self.message to the activator.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function

*/
void G_UseTargets (edict_t *ent, edict_t *activator)
{
        edict_t         *t;

//
// check for a delay
//
        if (ent->delay)
        {
                // create a temp object to fire at a later time
                t = G_Spawn();
                t->classname = "DelayedUse";
                t->nextthink = level.time + ent->delay;
                t->think = Think_Delay;
                t->activator = activator;
                if (!activator)
                        gi.dprintf ("WARNING: Think_Delay with no activator: %s (%s) at %s\n", ent->classname, ent->targetname, vtos2(ent));
                t->message = ent->message;
                t->target = ent->target;
                t->svflags |= SVF_NOCLIENT;
                t->killtarget = ent->killtarget;
                return;
        }


//
// print the message
//
        if ((ent->message) && !(activator->svflags & SVF_MONSTER))
        {
                if (activator->client)
                        gi.centerprintf (activator, "%s", ent->message);
                if (ent->noise_index)
                        gi.sound (activator, CHAN_AUTO, ent->noise_index, 1, ATTN_NORM, 0);
                else
                        gi.sound (activator, CHAN_AUTO, SoundIndex (misc_talk1), 1, ATTN_NORM, 0);
        }

//
// kill killtargets
//
        if (ent->killtarget)
        {
                t = NULL;
                while ((t = G_Find (t, FOFS(targetname), ent->killtarget)))
                {
                        if (mapdebugmode->value)
                                gi.dprintf ("[killtar] %s {%s} was killtargetted by %s {%s}\n",t->classname, t->targetname, ent->classname, ent->targetname);
                        G_FreeEdict (t);
                        if (!ent->inuse)
                        {
                                gi.dprintf("entity was removed while using killtargets\n");
                                return;
                        }
                }
        }

//
// fire targets
//
        if (ent->target)
        {
                t = NULL;
                while ((t = G_Find (t, FOFS(targetname), ent->target)))
                {
                        // doors fire area portals in a specific way
                        if (t->enttype == ENT_AREAPORTAL &&
                                (ent->enttype == ENT_FUNC_DOOR || !Q_stricmp(ent->classname, "func_door_rotating")))
                                continue;

                        if (t == ent)
                        {
                                gi.dprintf ("WARNING: Entity used itself.\n%s - %s\n",t->classname,t->targetname);
                        }
                        else
                        {
                                if (mapdebugmode->value >= 2)
                                        gi.dprintf ("[trigger] %s (targetname:%s target:%s) was triggered by %s (targetname:%s target:%s)\n", t->classname, t->targetname, t->target, ent->classname, ent->targetname, ent->target);

                                if (t->use)
                                        t->use (t, ent, activator);
                        }
                        if (!ent->inuse)
                        {
                                gi.dprintf("entity was removed while using targets\n");
                                return;
                        }
                }
        }
}


/*
TempVector

This is just a convenience function
for making temporary vectors for function calls
*/
float   *tv (float x, float y, float z)
{
        static  int             index;
        static  vec3_t  vecs[8];
        float   *v;

        // use an array so that multiple tempvectors won't collide
        // for a while
        v = vecs[index];
        index = (index + 1)&7;

        v[0] = x;
        v[1] = y;
        v[2] = z;

        return v;
}


/*
VectorToString

This is just a convenience function
for printing vectors
*/
char    *vtos (vec3_t v)
{
        static  int             index;
        static  char    str[8][32];
        char    *s;

        // use an array so that multiple vtos won't collide
        s = str[index];
        index = (index + 1)&7;

        Com_sprintf (s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

        return s;
}

char    *vtos2 (edict_t *self)
{
        vec3_t v;
        static  int             index;
        static  char    str[8][32];
        char    *s;

        // use an array so that multiple vtos won't collide
        s = str[index];
        index = (index + 1)&7;

        VectorCopy (self->s.origin, v);

        if (v[0] == 0 && v[1] == 0 && v[2] == 0) {
                vec3_t size;
                VectorScale (self->size, 0.5f, size);
                VectorAdd (self->absmin, size, v);
        }

        Com_sprintf (s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

        return s;
}

const vec3_t VEC_UP             = {0, -1, 0};
const vec3_t MOVEDIR_UP = {0, 0, 1};
const vec3_t VEC_DOWN           = {0, -2, 0};
const vec3_t MOVEDIR_DOWN       = {0, 0, -1};

void G_SetMovedir (vec3_t angles, vec3_t movedir)
{
        if (VectorCompare (angles, VEC_UP))
        {
                VectorCopy (MOVEDIR_UP, movedir);
        }
        else if (VectorCompare (angles, VEC_DOWN))
        {
                VectorCopy (MOVEDIR_DOWN, movedir);
        }
        else
        {
                AngleVectors (angles, movedir, NULL, NULL);
        }

        VectorClear (angles);
}


float vectoyaw (vec3_t vec)
{
        float   yaw;

        if (vec[YAW] == 0 && vec[PITCH] == 0)
                yaw = 0;
        else
        {
                yaw = (int) (atan2(vec[YAW], vec[PITCH]) * 180 / M_PI);
                if (yaw < 0)
                        yaw += 360;
        }

        return yaw;
}


void vectoangles (vec3_t value1, vec3_t angles)
{
        float   forward;
        float   yaw, pitch;

        if (value1[1] == 0 && value1[0] == 0)
        {
                yaw = 0;
                if (value1[2] > 0)
                        pitch = 90;
                else
                        pitch = 270;
        }
        else
        {
                yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
                if (yaw < 0)
                        yaw += 360;

                forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
                pitch = (int) (atan2(value1[2], forward) * 180 / M_PI);
                if (pitch < 0)
                        pitch += 360;
        }

        angles[PITCH] = -pitch;
        angles[YAW] = yaw;
        angles[ROLL] = 0;
}

char *G_CopyString (char *in)
{
        char    *out;

        out = gi.TagMalloc ((int)strlen(in)+1, TAG_LEVEL);
        strcpy (out, in);
        return out;
}


void G_InitEdict (edict_t *e)
{
        e->s.number = e - g_edicts;
        e->inuse = true;
        e->gravity = 1.0;
        e->spawnframenum = level.framenum;
        if (sv_extended) ext.SetEntityMask(e->s.number, 1);
}

void GrappleCount(void){
        edict_t *from = g_edicts;
//      int grapples = 0;
        int temp = 0;

        for ( ; from < &g_edicts[globals.num_edicts] ; from++)
        {
                if (!from->inuse)
                        continue;
                temp++;
        }

        gi.dprintf ("There are %d/1024 entities in use.\n",temp);
}
/*
void DumpClasses(void){
        edict_t *from = g_edicts;
        FILE *log;
        int temp = 0;

        log = fopen("entity.log", "w");
        fprintf(log, "No Free Edicts!\nEntities in use:\n");
        for ( ; from < &g_edicts[globals.num_edicts] ; from++)
        {
                if (!from->inuse)
                        continue;
                fprintf(log, "%i: %s\n",temp, from->classname);
                        temp++;
        }
        fclose(log);
}*/

/*
G_Spawn

Either finds a free edict, or allocates a new one.
Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.

Find a free ent starting from high mark and ignore ents that haven't got enough freetime.

*/
edict_t *g_spawn_ent;
edict_t *G_Spawn (void)
{
        while (!g_spawn_ent->client) {

                if (!g_spawn_ent->inuse && (g_spawn_ent->freeframe == 0 ||
                        level.framenum - g_spawn_ent->freeframe > 50))
                {
                        // free ent found, take it
                        G_InitEdict(g_spawn_ent);
                        return g_spawn_ent;
                }

                g_spawn_ent--;
        }

        // alloc new ent
        g_spawn_ent = g_edicts+globals.num_edicts; // top is always free
        globals.num_edicts++;

        if (globals.num_edicts < MAX_EDICTS) {
                G_InitEdict (g_spawn_ent);
                return g_spawn_ent;
        }

        gi.error ("G_Spawn: overflowed entity limit!");
        return NULL;
}

/*
G_FreeEdict

Marks the edict as free
*/
void G_FreeEdict (edict_t *ed)
{
        if (!ed) {
                DEBUGBREAKPOINT;
                gi.error ("G_FreeEdict: NULL edict");
        }

        if (ed->client) {
                gi.dprintf ("WARNING: Tried to free a client!\n");
                return;
        } else if (ed == world) {
                gi.bprintf (PRINT_HIGH, "WARNING: ATTEMPTED TO FREE WORLDSPAWN!!\n");
                return;
        }
        
        gi.unlinkentity (ed);           // unlink from world
        memset (ed, 0, sizeof(*ed));
        ed->freeframe = level.framenum;

        // fixme: do a quick search on map spawned ents if any of them has been freed
        // so we can update the low mark set in g_spawn/spawnentities

        // update high mark
        //gi.dprintf("mark at %d, ", globals.num_edicts);
        g_spawn_ent = g_edicts+globals.num_edicts-1; // point to last inuse ent
        while (!g_spawn_ent->inuse && !g_spawn_ent->client)
        {
                g_spawn_ent--;
                globals.num_edicts--;
        }
}


/*
G_TouchTriggers

*/
void    G_TouchTriggers (edict_t *ent)
{
        int                     i, num;
        edict_t         *touch[MAX_EDICTS], *hit;

        // dead things don't activate triggers!
        if (ent->client && ent->health <= 0)
                return;

        num = gi.BoxEdicts (ent->absmin, ent->absmax, touch
                , MAX_EDICTS, AREA_TRIGGERS);

        // be careful, it is possible to have an entity in this
        // list removed before we get to it (killtriggered)
        for (i=0 ; i<num ; i++)
        {
                hit = touch[i];
                if (!hit->inuse)
                        continue;
                if (!hit->touch)
                        continue;
                hit->touch (hit, ent, NULL, NULL);
        }
}

/*
G_TouchSolids

Call after linking a new trigger in during gameplay
to force all entities it covers to immediately touch it
*/
void    G_TouchSolids (edict_t *ent)
{
        int                     i, num;
        edict_t         *touch[MAX_EDICTS], *hit;

        num = gi.BoxEdicts (ent->absmin, ent->absmax, touch
                , MAX_EDICTS, AREA_SOLID);

        // be careful, it is possible to have an entity in this
        // list removed before we get to it (killtriggered)
        for (i=0 ; i<num ; i++)
        {
                hit = touch[i];
                if (!hit->inuse)
                        continue;
                if (ent->touch)
                        ent->touch (hit, ent, NULL, NULL);
                if (!ent->inuse)
                        break;
        }
}

/*
KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
*/
qboolean KillBox (edict_t *ent){
        trace_t         tr;

        while (1)
        {
                tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, ent->s.origin, NULL, MASK_PLAYERSOLID);
                if (!tr.ent)
                        break;

                //gi.bprintf (PRINT_HIGH,"attempting to kill a %s\n",tr.ent->classname);

                // nail it
                T_Damage (tr.ent, ent, ent, vec3_origin, ent->s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);

        //gi.bprintf (PRINT_HIGH,"failed, breaking\n");

                // if we didn't kill it, fail
                if (tr.ent->solid)
                        return false;
        }

        return true;            // all clear
}

void stuffcmd(edict_t * ent, const char * texttostuff){

        if (!ent->inuse || !ent->client || !ent->client->pers.ingame)
                return;

        gi.WriteByte(svc_stufftext);
        gi.WriteString(texttostuff);
        gi.unicast(ent,true);
}

const char *colortext(const char *text)
{
        static char ctext[2][80];
        static int c=0;
        const char *p;
        char *cp;
        c^=1;

        if (!*text)
                return text;

        for (p=text, cp=ctext[c];*p!=0;p++, cp++){
                *cp=*p|128;
        }
        *cp=0;
        return ctext[c];
}

void SetClientVelocity (edict_t *ent, int speed)
{
        stuffcmd (ent, va("set cl_forwardspeed %d\nset cl_sidespeed %d\n", speed, speed));
}
