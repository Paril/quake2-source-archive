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
#include "g_map.h"
#include "g_wordfilter.h"
#include "persistent.h"
#ifndef __WIN32
#include <alloca.h>
#endif
#include <stdio.h>

#ifdef __WIN32
#define strncasecmp stricmp
#endif

#define ONLYGODSCANCHEAT 1

qboolean        active_vote=false;      // is there a vote going on?
vote_t  vote;
fragsave_t fragsave[65];

void AdminLog (edict_t *ent, char *message);

void ClientUserinfoChanged (edict_t *ent, char *userinfo);

void breeder_deconstruct (edict_t *self, float dist);
void eng_deconstruct (edict_t *self, float dist);

void JoinTeam0(edict_t *ent);

void Cmd_EvadeList_f (edict_t *ent);
void Cmd_Search_f (edict_t *ent);
void Cmd_ViewChangeLog (edict_t *ent);

void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);
qboolean CmdSetTripWire (edict_t *self, qboolean summoned);




#define SAFEDEBUGDIE ( \
 (1 << MOD_UNKNOWN) | \
 (1 << MOD_WATER) | \
 (1 << MOD_SLIME) | \
 (1 << MOD_LAVA) | \
 (1 << MOD_CRUSH) | \
 (1 << MOD_TELEFRAG) | \
 (1 << MOD_FALLING) | \
 (1 << MOD_SUICIDE) | \
 (1 << MOD_EXIT) | \
 (1 << MOD_TRIGGER_HURT) | \
 (1 << MOD_SHORT) | \
 (1 << MOD_MINE) | \
 (1 << MOD_INFEST) \
)



#if 1
#warning "remove menu stubs"
int InitHudStrings(int n) {return 0;}
void MenuSet(edict_t*ent, menudata_t*m) {}
qboolean MenuActive(edict_t*ent) {return 0;}
int MenuMoveSelection(edict_t*ent, int delta) {return 0;}
void MenuActivateSelection(edict_t*ent) {}
#endif


//for msvc
static void MakeUpper(char *c)
{
  while(*c)
  {
    if (islower((*c)))
      *c = toupper((*c));
    c++;
  }
}

int stringContains(char *StringToSearch, char *StringToSearchFor)
{
        char strbuffer1[4096];
        char strbuffer2[4096];

        strncpy(strbuffer1, StringToSearch, sizeof(strbuffer2));
        MakeUpper(strbuffer1);
        strncpy(strbuffer2, StringToSearchFor, sizeof(strbuffer2));
        MakeUpper(strbuffer2);
        return (strstr(strbuffer1, strbuffer2) != NULL);
}

char *StripHighBits (char *string)
{
        int c;
        static char stripped[4096];
        char *p = stripped;

        while (*string)
        {
                c = *string++;
                c &= 127;               // strip high bits
                if (c >= 32 && c < 127)
                        *p++ = c;

                if (p - stripped == sizeof(stripped)-2) {
                        stripped[sizeof(stripped)-1] = '\0';
                        break;
                }
        }

        *p = '\0';

        return stripped;
}

qboolean has_permission (edict_t *ent, int permission)
{
        if (ent->client->pers.adminpermissions & (1 << permission)) {
                return true;
        } else {
                gi.cprintf (ent, PRINT_HIGH, "You do not have permission to use %s.\n", gi.argv(0));
                return false;
        }
}

edict_t *GetNextClient (edict_t *from, char *match, int *status)
{
        int v=0;
        int j;

        qboolean confuse_sscanf = false;
        char *p = match;

        if (isdigit (*p)) {
                while (*p) {
                        if (isalpha(*p)) {
                                confuse_sscanf = true;
                                break;
                        }
                        p++;
                }
        }

        if (!confuse_sscanf)
                j = sscanf(match, "%d", &v);
        else
                j = -1;

        //not an int
        if (j < 1) {
                edict_t *who, *found = NULL;

                if (!from)
                        from = g_edicts;

                from++;

                for (who = from; who < g_edicts + game.maxclients + 1; who++) {
                        if (!who->inuse)
                                continue;

                        if (!who->client->pers.connected)
                                continue;

                        if (!Q_stricmp (StripHighBits(who->client->pers.netname), match)) {
                                if (status)
                                        *status = 1;
                                found = who;
                                break;
                        }

                        if (stringContains(StripHighBits(who->client->pers.netname), match)) {
                                found = who;
                                if (status)
                                        *status = 0;
                                break;
                        }
                }
                return found;
        } else {
                if (g_edicts + v <= from || v >= game.maxclients || v < 0 || game.clients[v].pers.connected == false)
                        return NULL;

                if (status)
                        *status = 1;
                return g_edicts + 1 + v;
        }
}

//r1: lookup client id by substring of name or direct ID
int GetClientIDbyNameOrID (char *arg)
{
        edict_t *ent;
        int status;

        ent = GetNextClient (NULL, arg, &status);
        if (!ent)
                return -2;

        //couldn't verify
        if (!status) {
                if (GetNextClient (ent, arg, NULL))
                        return -1;
        }

        return ent - g_edicts - 1;
}

qboolean CheckInvalidClientResponse (edict_t *ent, int x, char *arg)
{
        if (x == -1) {
                gi.cprintf (ent, PRINT_HIGH, "2 or more matches for '%s'\n", arg);
                return true;
        } else if (x == -2) {
                gi.cprintf (ent, PRINT_HIGH, "No match for '%s'\n", arg);
                return true;
        /*} else if (x == -3) {
                gi.cprintf (ent, PRINT_HIGH, "Client %s is out of range.\n", arg);
                return true;
        } else if (x == -4) {
                gi.cprintf (ent, PRINT_HIGH, "Client %s is not connected.\n", arg);
                return true;*/
        }

        return false;
}

void kick (edict_t *victim)
{
        if ((victim->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS)) && victim->client->resp.teamkills != -1) {
                gi.dprintf ("kick: refusing to kick admin\n");
                return;
        }

        victim->client->resp.score = 0;
        victim->client->resp.total_score = 0;

        gi.AddCommandString (va("kick %d\n", (victim - g_edicts)-1 ));
}

void DeathmatchScoreboard (edict_t *ent);

static void SelectNextItem (edict_t *ent, int itflags)
{
        gclient_t       *cl;
        int                     i, index;
        const gitem_t           *it;

        if (MenuActive(ent)) {
          MenuMoveSelection(ent, +1);
          return;
        }

        // prevents two or more changes in one frame
        /*if(!(ent->wait < level.time)) {
                return;
        }
        ent->wait = level.time; */

        cl = ent->client;

        if (cl->menu.entries)
        {
                if (cl->last_menu_frame == level.framenum)
                        return;
                PMenu_Next(ent);
                return;
        }

        if (cl->showscores)
        {
                if (!level.suddendeath)
                {
                        cl->resp.scoreboardstate ^= 1;
                        DeathmatchScoreboard (ent);
                }
                return;
        }

        if (cl->resp.class_type == CLASS_OBSERVER) {
                ChaseNext(ent);
                return;
        }

        // scan for the next valid one
        for (i=1 ; i<=MAX_ITEMS ; i++)
        {
                index = (cl->resp.selected_item + i)%MAX_ITEMS;
                if (!cl->resp.inventory[index])
                        continue;
                it = &itemlist[index];
                if (!it->use)
                        continue;
                if (!(it->flags & itflags))
                        continue;

                cl->resp.selected_item = index;
                return;
        }

        cl->resp.selected_item = -1;
}

static void SelectPrevItem (edict_t *ent, int itflags)
{
        gclient_t       *cl;
        int                     i, index;
        const gitem_t           *it;
        
        if (MenuActive(ent)) {
          MenuMoveSelection(ent, -1);
          return;
        }

        // prevents two or more changes in one frame
        /*if(!(ent->wait < level.time)) {
                return;
        }
        ent->wait = level.time; */

        cl = ent->client;

        if (cl->menu.entries)
        {
                if (cl->last_menu_frame == level.framenum)
                        return;
                cl->last_menu_frame = level.framenum;
                PMenu_Prev (ent);
                return;
        }

        if (cl->resp.class_type == CLASS_OBSERVER) {
                ChasePrev (ent);
                return;
        }

        // scan for the next valid one
        for (i=1 ; i<=MAX_ITEMS ; i++)
        {
                index = (cl->resp.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
                if (!cl->resp.inventory[index])
                        continue;
                it = &itemlist[index];
                if (!it->use)
                        continue;
                if (!(it->flags & itflags))
                        continue;

                cl->resp.selected_item = index;
                return;
        }

        cl->resp.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
        if (ent->client->resp.inventory[ent->client->resp.selected_item])
                return;         // valid

        ent->client->resp.selected_item = -1; // invalid

        SelectNextItem (ent, -1);
}

/*
Cmd_Give_f

Give items to a client
*/
static void Cmd_Give_f (edict_t *ent)
{
        char            *name;
        const gitem_t           *it;
        int                     index;
        int                     i;
        qboolean        give_all;
//      edict_t         *it_ent;

        if (deathmatch->value && !sv_cheats->value)
        {
                gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
                return;
        }

#ifdef ONLYGODSCANCHEAT
        if (!(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS )))
                return;
#endif

        name = gi.args();

        if (Q_stricmp(name, "all") == 0)
                give_all = true;
        else
                give_all = false;

        if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
        {
                if (gi.argc() == 3)
                        ent->health = atoi(gi.argv(2));
                else
                        ent->health = ent->max_health;
                if (!give_all)
                        return;
        }

        if (give_all || Q_stricmp(name, "weapons") == 0)
        {
                for (i=0 ; i<game.num_items ; i++)
                {
                        it = itemlist + i;
                        if (!it->pickup)
                                continue;
                        if (!(it->flags & IT_WEAPON))
                                continue;
                        ent->client->resp.inventory[i] += 1;
                }
                if (!give_all)
                        return;
        }

        if (give_all || Q_stricmp(name, "ammo") == 0)
        {
                for (i=0 ; i<game.num_items ; i++)
                {
                        it = itemlist + i;
                        if (!it->pickup)
                                continue;
                        if (!(it->flags & IT_AMMO))
                                continue;
                        Add_Ammo (ent, it, 1000);
                }
                if (!give_all)
                        return;
        }

        // Gloom
        if (give_all || Q_stricmp(name, "clip") == 0)
        {
                for (i=0 ; i<game.num_items ; i++)
                {
                        it = itemlist + i;
                        // Is a clip
                        if (!(it->flags & IT_CLIP))
                                continue;

                        ent->client->resp.inventory[ITEM_INDEX(it)] = 10;
                }
                if (!give_all)
                        return;
        }

        if (give_all || Q_stricmp(name, "armor") == 0)
        {
                gitem_armor_t   *info;

                it = FindItem("Light Armor");
                ent->client->resp.inventory[ITEM_INDEX(it)] = 0;

                it = FindItem("Medium Armor");
                ent->client->resp.inventory[ITEM_INDEX(it)] = 0;

                it = FindItem("Heavy Armor");
                info = (gitem_armor_t *)it->info;
                ent->client->resp.inventory[ITEM_INDEX(it)] = info->max_count;

                if (!give_all)
                        return;
        }

        if (give_all)
        {
                for (i=0 ; i<game.num_items ; i++)
                {
                        it = itemlist + i;
                        if (!it->pickup)
                                continue;
                        if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO|IT_CLIP))
                                continue;
                        ent->client->resp.inventory[i] = 1;
                }
                return;
        }

        it = FindItem (name);
        if (!it)
        {
                name = gi.argv(1);
                it = FindItem (name);
                if (!it)
                {
                        //gi.dprintf ("unknown item\n");
                        return;
                }
        }

        if (!it->pickup)
        {
                //gi.dprintf ("non-pickup item\n");
                return;
        }

        index = ITEM_INDEX(it);

        if (it->flags & IT_AMMO)
        {
                if (gi.argc() == 3)
                        ent->client->resp.inventory[index] = atoi(gi.argv(2));
                else
                        ent->client->resp.inventory[index] += it->quantity;
        }
        else
        {
                //it_ent = G_Spawn();
                //it_ent->classname = it->classname;
                //SpawnItem (it_ent, it);
                //Touch_Item (it_ent, ent, NULL, NULL);
                //if (it_ent->inuse)
                        //G_FreeEdict(it_ent);
        }
}


/*
Cmd_God_f

Sets client to godmode

*/
static void Cmd_God_f (edict_t *ent)
{
        char    *msg;

        if (deathmatch->value && !sv_cheats->value)
        {
                gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
                return;
        }

#ifdef ONLYGODSCANCHEAT
        if (!(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS )))
                return;
#endif

        ent->flags ^= FL_GODMODE;
        if (!(ent->flags & FL_GODMODE) )
                msg = "godmode OFF\n";
        else
                msg = "godmode ON\n";

        gi.cprintf (ent, PRINT_HIGH, msg);
}

/*
Cmd_Notarget_f

Sets client to notarget
*/
static void Cmd_Notarget_f (edict_t *ent)
{
        char    *msg;

        if (deathmatch->value && !sv_cheats->value)
        {
                gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
                return;
        }

#ifdef ONLYGODSCANCHEAT
        if (!(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS )))
                return;
#endif

        ent->flags ^= FL_NOTARGET;
        if (!(ent->flags & FL_NOTARGET) )
                msg = "notarget OFF\n";
        else
                msg = "notarget ON\n";

        gi.cprintf (ent, PRINT_HIGH, msg);
}

/*
Cmd_Noclip_f
*/
static void Cmd_Noclip_f (edict_t *ent)
{
        char    *msg;

        if (deathmatch->value && !sv_cheats->value)
        {
                gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
                return;
        }

#ifdef ONLYGODSCANCHEAT
        if (!(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS )))
                return;
#endif

        if (ent->movetype == MOVETYPE_NOCLIP)
        {
                ent->movetype = MOVETYPE_WALK;
                msg = "noclip OFF\n";
        }
        else
        {
                ent->movetype = MOVETYPE_NOCLIP;
                msg = "noclip ON\n";
        }

        gi.cprintf (ent, PRINT_HIGH, msg);
}

static void Cmd_DebugGetEnts (edict_t *ent)
{
        edict_t *t = NULL;
        vec3_t distance;

        while ((t = findradius_all (t, ent->s.origin, 128)) != NULL) {
                VectorSubtract (t->s.origin, ent->s.origin, distance);
                gi.cprintf (ent, PRINT_HIGH, "%-3d: %s[%s] (%.4f)\n", t - g_edicts, t->classname, t->targetname, VectorLength (distance));
        }
}


static int PointerEnt(edict_t*ent)
{
  int mask;
  trace_t tr;
  vec3_t forward,start;

  VectorCopy (ent->s.origin,start);
  start[2] += ent->viewheight;

  AngleVectors(ent->client->v_angle, forward, NULL, NULL);
  VectorScale(forward, 8192, forward);
  VectorAdd(ent->s.origin, forward, forward);

  tr = gi.trace (start, NULL, NULL, forward, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

  gi.WriteByte (svc_temp_entity);
  gi.WriteByte (TE_DEBUGTRAIL);
  gi.WritePosition (ent->s.origin);
  gi.WritePosition (tr.endpos);
  gi.multicast (ent->s.origin, MULTICAST_PVS);
  
  if (tr.ent) return tr.ent-g_edicts;
  return 0; //world
}

static void Cmd_DebugBox_f(edict_t *ent)
{
        int number;
        edict_t *targ;

        if (gi.argc() < 2) {
          number = PointerEnt(ent);
        } else
          number = atoi(gi.argv(1));
          
        if (!number) return;

        targ = &g_edicts[number];

        if (targ->inuse) {
          vec3_t a = {targ->s.origin[0]+targ->mins[0], targ->s.origin[1]+targ->mins[1], targ->s.origin[2]+targ->mins[2]};
          vec3_t b = {targ->s.origin[0]+targ->maxs[0], targ->s.origin[1]+targ->maxs[1], targ->s.origin[2]+targ->maxs[2]};
          vec3_t size;
          VectorSubtract(b, a, size);

          gi.cprintf(ent, PRINT_HIGH, "Entity %d (%s): Size: %.1fx%.1fx%.1f  Origin: %.1f, %.1f, %.1f\n",
             targ-g_edicts, targ->classname,
             size[0], size[1], size[2],
             (targ->maxs[0]+targ->mins[1])*0.5f, (targ->maxs[1]+targ->mins[0])*0.5f, (targ->maxs[2]+targ->mins[2])*0.5f
           );

          DebugBox(a, b);
        } else
          gi.cprintf (ent, PRINT_HIGH, "ERROR: entity %d not in use.\n", number);
}


static void Cmd_DebugMove (edict_t *ent)
{
        int number;
        edict_t *targ;

        if (gi.argc() < 2) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @debugmove edict\n");
                return;
        }

        number = atoi(gi.argv(1));

        targ = &g_edicts[number];

        if (!targ->inuse) {
                gi.cprintf (ent, PRINT_HIGH, "ERROR: entity %d not in use.\n", number);
                return;
        }

        VectorCopy (ent->s.origin, targ->s.origin);
        gi.linkentity (targ);

        gi.cprintf (ent, PRINT_HIGH, "Entity moved.\n");
}

void debug_use_think (edict_t *ent)
{
        edict_t *targ = ent->target_ent;

        if (!targ->inuse) {
                G_FreeEdict (ent);
                return;
        }

        if (!targ->use) {
                G_FreeEdict (ent);
                return;
        }

        targ->use (targ, ent, ent);

        ent->nextthink = level.time + .1f;
}

void Cmd_DebugDie (edict_t *ent)
{
        edict_t *targ;
        int number;

        if (gi.argc() < 2) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @debugdie edict\n");
                return;
        }

        number = atoi(gi.argv(1));

        targ = &g_edicts[number];

        if (!targ->die) {
                gi.cprintf (ent, PRINT_HIGH, "ERROR: NULL ->die\n");
                return;
        }

        meansOfDeath = atoi(gi.argv(2));
        if (!(SAFEDEBUGDIE && (1 << meansOfDeath))) meansOfDeath = MOD_UNKNOWN;
        targ->die (targ, ent, ent, 100000, vec3_origin);
}

void Cmd_DebugFree (edict_t *ent)
{
        edict_t *targ;
        int number;

        if (gi.argc() < 2) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @debugfree edict\n");
                return;
        }

        number = atoi(gi.argv(1));

        targ = &g_edicts[number];

        G_FreeEdict (targ);
}

void Cmd_Goto (edict_t *ent)
{
        edict_t *targ;
        int number;

        if (gi.argc() < 2) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @goto (edictnumber|clientname)\n");
                return;
        }

        if (!isdigit(*gi.argv(1))) {
                number = GetClientIDbyNameOrID (gi.argv(1));
                if (CheckInvalidClientResponse (ent, number, gi.argv(1)))
                        return;
                number++;
        } else {
                number = atoi(gi.argv(1));
        }

        if (number < 0 || number >= MAX_EDICTS) {
                gi.cprintf (ent, PRINT_HIGH, "Illegal edict %d.\n", number);
                return;
        }

        targ = &g_edicts[number];

        if (!targ->inuse) {
                gi.cprintf (ent, PRINT_HIGH, "Entity %d not in use!\n", number);
                return;
        }

        VectorCopy (targ->s.origin, ent->s.origin);
}

void Cmd_DebugFind (edict_t *ent)
{
        char *classname;
        edict_t *t = NULL;

        if (gi.argc() < 2) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @debugfind classname\n");
                return;
        }

        classname = gi.argv(1);

        while (t = (findradius (t, ent->s.origin, 64))) {
                if (t->classname && !Q_stricmp (t->classname, classname)) {
                        break;
                }
        }

        t = G_Find (t, FOFS(classname), classname);
        if (!t) {
                gi.cprintf (ent, PRINT_HIGH, "No (more) entities found.\n");
                return;
        }

        VectorCopy (t->s.origin, ent->s.origin);
}

void Cmd_DebugUser (edict_t *ent)
{
        edict_t *targ, *e;
        int number;

        if (gi.argc() < 2) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @debuguser edict\n");
                return;
        }

        number = atoi(gi.argv(1));

        targ = &g_edicts[number];

        if (!targ->inuse) {
                gi.cprintf (ent, PRINT_HIGH, "ERROR: entity %d not in use.\n", number);
                return;
        }

        if (!targ->use) {
                gi.cprintf (ent, PRINT_HIGH, "ERROR: NULL ->use\n");
                return;
        }

        e = G_Spawn();
        e->think = debug_use_think;
        e->nextthink = level.time + 1;
        e->svflags |= SVF_NOCLIENT;
        e->target_ent = targ;
        gi.linkentity (e);
}

void world_think (edict_t *ent);
void Cmd_DebugUseEntity (edict_t *ent)
{
        edict_t *targ;
        int number;

        if (!Q_stricmp (gi.argv(1), "world"))
        {
                world_think (world);
                return;
        }

        if (gi.argc() < 2) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @debuguse edict\n");
                return;
        }

        number = atoi(gi.argv(1));

        targ = &g_edicts[number];

        if (!targ->inuse) {
                gi.cprintf (ent, PRINT_HIGH, "ERROR: entity %d not in use.\n", number);
                return;
        }

        if (!targ->use) {
                gi.cprintf (ent, PRINT_HIGH, "ERROR: NULL ->use\n");
                return;
        }

        targ->use (targ, ent, ent);
}

static void Cmd_DebugEntitySet (edict_t *ent)
{
        int number;
        char *field, *value;
        edict_t *targ;

        if (gi.argc() < 4) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @debugset edict field value\n");
                return;
        }

        number = atoi(gi.argv(1));
        field = gi.argv(2);
        value = gi.argv(3);

        targ = &g_edicts[number];

        if (!targ->inuse) {
                gi.cprintf (ent, PRINT_HIGH, "ERROR: entity %d not in use.\n", number);
                return;
        }

        if (!Q_stricmp (field, "s.origin[0]"))
                targ->s.origin[0] = atoi(value);
        else if (!Q_stricmp (field, "s.origin[1]"))
                targ->s.origin[1] = atoi(value);
        else if (!Q_stricmp (field, "s.origin[2]"))
                targ->s.origin[2] = atoi(value);
        else if (!Q_stricmp (field, "s.angles[0]"))
                targ->s.angles[0] = atof(value);
        else if (!Q_stricmp (field, "s.angles[1]"))
                targ->s.angles[1] = atof(value);
        else if (!Q_stricmp (field, "s.angles[2]"))
                targ->s.angles[2] = atof(value);
        else if (!Q_stricmp (field, "movedir[0]"))
                targ->movedir[0] = atof(value);
        else if (!Q_stricmp (field, "movedir[1]"))
                targ->movedir[1] = atof(value);
        else if (!Q_stricmp (field, "movedir[2]"))
                targ->movedir[2] = atof(value);
        else if (!Q_stricmp (field, "s.modelindex"))
                targ->s.modelindex = atoi(value);
        else if (!Q_stricmp (field, "s.modelindexname"))
                targ->s.modelindex = gi.modelindex (value);
        else if (!Q_stricmp (field, "s.frame"))
                targ->s.frame = atoi(value);
        else if (!Q_stricmp (field, "s.skinnum"))
                targ->s.skinnum = atoi(value);
        else if (!Q_stricmp (field, "s.effects"))
                targ->s.effects = atoi(value);
        else if (!Q_stricmp (field, "s.renderfx"))
                targ->s.renderfx = atoi(value);
        else if (!Q_stricmp (field, "s.sound"))
                targ->s.sound = atoi(value);
        else if (!Q_stricmp (field, "s.soundname"))
                targ->s.sound = gi.soundindex (value);
        else if (!Q_stricmp (field, "s.event"))
                targ->s.event = atoi(value);
        else if (!Q_stricmp (field, "health"))
                targ->health = atoi(value);
        else if (!Q_stricmp (field, "max_health"))
                targ->max_health = atoi(value);
        else if (!Q_stricmp (field, "think"))
                targ->think = NULL;
        else if (!Q_stricmp (field, "mass"))
                targ->mass = atoi(value);
        else if (!Q_stricmp (field, "spawnflags"))
                targ->spawnflags = atoi(value);
        else if (!Q_stricmp (field, "dmg"))
                targ->dmg = atoi(value);
        else if (!Q_stricmp (field, "target"))
        {
                targ->target = gi.TagMalloc ((int)strlen(value)+1, TAG_LEVEL);
                strcpy (targ->target, value);
        }
        else if (!Q_stricmp (field, "targetname"))
        {
                targ->targetname = gi.TagMalloc ((int)strlen(value)+1, TAG_LEVEL);
                strcpy (targ->targetname, value);
        }
        else
        {
                gi.cprintf (ent, PRINT_HIGH, "ERROR: invalid field.\n");
                return;
        }

        gi.cprintf (ent, PRINT_HIGH, "%s updated and relinked.\n", targ->classname);
        gi.linkentity (targ);
}

static void Cmd_DebugEntityGet (edict_t *ent)
{
        int number;
        char *field;
        edict_t *targ;

        if (gi.argc() < 3) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @debugget edict field\n");
                return;
        }

        number = atoi(gi.argv(1));
        field = gi.argv(2);

        targ = &g_edicts[number];

        if (!targ->inuse) {
                gi.cprintf (ent, PRINT_HIGH, "ERROR: entity %d not in use.\n", number);
                return;
        }

        if (!Q_stricmp (field, "s.origin[0]"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %f\n", field, targ->s.origin[0]);
        else if (!Q_stricmp (field, "s.origin[1]"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %f\n", field, targ->s.origin[1]);
        else if (!Q_stricmp (field, "s.origin[2]"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %f\n", field, targ->s.origin[2]);
        else if (!Q_stricmp (field, "s.angles[0]"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %f\n", field, targ->s.angles[0]);
        else if (!Q_stricmp (field, "s.angles[1]"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %f\n", field, targ->s.angles[1]);
        else if (!Q_stricmp (field, "s.angles[2]"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %f\n", field, targ->s.angles[2]);
        else if (!Q_stricmp (field, "movedir[0]"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %f\n", field, targ->movedir[0]);
        else if (!Q_stricmp (field, "movedir[1]"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %f\n", field, targ->movedir[1]);
        else if (!Q_stricmp (field, "movedir[2]"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %f\n", field, targ->movedir[2]);
        else if (!Q_stricmp (field, "s.modelindex"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %d\n", field, targ->s.modelindex);
        else if (!Q_stricmp (field, "s.frame"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %d\n", field, targ->s.frame);
        else if (!Q_stricmp (field, "s.skinnum"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %d\n", field, targ->s.skinnum);
        else if (!Q_stricmp (field, "s.effects"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %u\n", field, targ->s.effects);
        else if (!Q_stricmp (field, "s.renderfx"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %d\n", field, targ->s.renderfx);
        else if (!Q_stricmp (field, "s.sound"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %d\n", field, targ->s.sound);
        else if (!Q_stricmp (field, "health"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %d\n", field, targ->health);
        else if (!Q_stricmp (field, "s.event"))
                gi.cprintf (ent, PRINT_HIGH, "\"%s\" is %d\n", field, targ->s.event);
        else {
                gi.cprintf (ent, PRINT_HIGH, "ERROR: invalid field.\n");
                return;
        }
}

/*
Cmd_Use_f

Use an inventory item
*/
void Cmd_Use_f (edict_t *ent, char*s)
{
        int                     index;
        const gitem_t           *it;

        if(ent->health <= 0)
                return;

        //HACK: UGLY: XXX
        if (!Q_stricmp (s, "secondary")) {
                switch (ent->client->resp.class_type) {
                        case CLASS_GRUNT:
                        case CLASS_SHOCK:
                        case CLASS_BIO:
                        case CLASS_HEAVY:
                                s = "Pistol";
                                break;
                        case CLASS_COMMANDO:
                                s = "Magnum BD";
                                break;
                        default:
                                gi.cprintf (ent, PRINT_HIGH, "No secondary weapon for this class.\n");
                                return;
                }
        } else if (!Q_stricmp (s, "primary")) {
                switch (ent->client->resp.class_type) {
                        case CLASS_GRUNT:
                                s = "Autogun";
                                break;
                        case CLASS_HEAVY:
                                s = "Rocket Launcher";
                                break;
                        case CLASS_BIO:
                                s = "Scattergun";
                                break;
                        case CLASS_SHOCK:
                                s = "Shotgun";
                                break;
                        case CLASS_COMMANDO:
                                s = "Submachinegun";
                                break;
                        default:
                                gi.cprintf (ent, PRINT_HIGH, "You are already using the only weapon for this class.\n");
                                break;
                }
        } else if (!Q_stricmp (s, "special")) {
                switch (ent->client->resp.class_type) {
                        case CLASS_EXTERM:
                                s = "Power Shield";
                                break;
                        case CLASS_SHOCK:
                                s = "Shell Clip [EX]";
                                break;
                        case CLASS_COMMANDO:
                                s = "C4 Explosive";
                                break;
                        case CLASS_BIO:
                                s = "Flare";
                                break;
                        default:
                                gi.cprintf (ent, PRINT_HIGH, "This class has no special item that can be used.\n");
                                return;
                }
        }

        it = FindItem (s);
        if (!it)
        {
                gi.cprintf (ent, PRINT_HIGH, "Unknown item: %s\n", s);
                return;
        }
        if (!it->use)
        {
                gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
                return;
        }
        index = ITEM_INDEX(it);
        if (!ent->client->resp.inventory[index])
        {
                gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
                return;
        }

        it->use (ent, it);

        //r1: 0 inven fix
        ValidateSelectedItem (ent);
}


/*
Cmd_Drop_f

Drop an inventory item
*/
/*void Cmd_Drop_f (edict_t *ent)
{
        int                     index;
        gitem_t         *it;
        char            *s;

        return;

        if(!ent->solid)
                return;

        s = gi.args();
        it = FindItem (s);
        if (!it)
        {
                gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
                return;
        }
        if (!it->drop)
        {
                gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
                return;
        }
        index = ITEM_INDEX(it);
        if (!ent->client->resp.inventory[index])
        {
                gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
                return;
        }

        it->drop (ent, it);
}*/


/*
Cmd_Inven_f
*/
void Cmd_Inven_f (edict_t *ent)
{
        PMenu_Close(ent);
}

/*
Cmd_InvUse_f
*/
void Cmd_InvUse_f (edict_t *ent)
{
        const gitem_t           *it;
        
        if (MenuActive(ent)) {
          MenuActivateSelection(ent);
          return;
        }

        if (ent->client->menu.entries) {
                PMenu_Select(ent);
                return;
        } else
        if (ent->client->resp.class_type == CLASS_OBSERVER) {
          if (ent->client->chase_target) {
            ent->client->kick_angles[0] = ent->client->kick_angles[1] = ent->client->kick_angles[2] = 0;
            ent->client->ps.kick_angles[0] = ent->client->ps.kick_angles[1] = ent->client->ps.kick_angles[2] = 0;

            if (!ent->client->resp.team) {
              ent->client->chase_view++;
              ent->client->chase_view %= CHASE_VIEWS;
            } else
              ent->client->chase_view = 0;
          } else
          if (ent->client->target_ent && ent->client->target_ent->inuse && ent->client->target_ent->client) {
            edict_t*other = ent->client->target_ent;
            if (!other->client->resp.team) { //can happen if player leaves a team and delayed persistency of target_ent hasnt cleared yet
                gi.cprintf(ent, PRINT_HIGH, "%s is not on a team.\n", other->client->pers.netname);
                return;
            }
            if (ent->client->resp.team && ent->client->resp.team != other->client->resp.team) {
                gi.cprintf(ent, PRINT_HIGH, "Player is not on your team.\n");
                return;
            }
            ChaseStart(ent, other);
          }
          return;
        }

        if(ent->health <= 0)
                return;

        if ((ent->client->invincible_framenum - 5) > level.framenum)
                return;

        ValidateSelectedItem (ent);

        if (ent->client->resp.selected_item == -1)
        {
                gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
                return;
        }

        it = &itemlist[ent->client->resp.selected_item];
        if (!it->use)
        {
                gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
                return;
        }

        it->use (ent, it);

        ValidateSelectedItem (ent);
}

/*
 * weapon commands
 */
void Cmd_WeapPrev_f (edict_t *ent)
{
        gclient_t       *cl;
        int                     i, index;
        const gitem_t           *it;
        int                     selected_weapon;

        cl = ent->client;

        if (!cl->weapon)
                return;

        selected_weapon = ITEM_INDEX(cl->weapon);

        // scan for the next valid one
        //for (i=1 ; i<=MAX_ITEMS ; i++)
        for (i=1 ; i < game.num_items ; i++)
        {
                index = (selected_weapon + i)%game.num_items;
                if (!cl->resp.inventory[index])
                        continue;
                it = &itemlist[index];
                if (! (it->flags & IT_WEAPON) )
                        continue;
                if (!it->use)
                        continue;
                it->use (ent, it);
                if (cl->weapon == it)
                        return; // successful
        }
}

void Cmd_WeapNext_f (edict_t *ent)
{
        gclient_t       *cl;
        int                     i, index;
        const gitem_t           *it;
        int                     selected_weapon;

        cl = ent->client;

        if (!cl->weapon)
                return;

        selected_weapon = ITEM_INDEX(cl->weapon);

        // scan for the next valid one
        //for (i=1 ; i<=MAX_ITEMS ; i++)
        for (i=1 ; i < game.num_items ; i++)
        {
                index = (selected_weapon + game.num_items - i)%game.num_items;
                if (!cl->resp.inventory[index])
                        continue;
                it = &itemlist[index];
                if (! (it->flags & IT_WEAPON) )
                        continue;
                if (!it->use)
                        continue;
                it->use (ent, it);
                if (cl->weapon == it)
                        return; // successful
        }
}

void Cmd_WeapLast_f (edict_t *ent)
{
        gclient_t       *cl;
        int                     index;
        const gitem_t           *it;

        cl = ent->client;

        if (!cl->weapon || !cl->lastweapon)
                return;

        index = ITEM_INDEX(cl->lastweapon);
        if (!cl->resp.inventory[index])
                return;
        it = &itemlist[index];
        if (! (it->flags & IT_WEAPON) )
                return;
        if (!it->use)
                return;
        it->use (ent, it);
}

void Cmd_WeapSwitch_f(edict_t *ent)
{
  gclient_t*cl;
  int       i;
  int newweap = -1;

  cl = ent->client;
  

  if (!cl->weapon || cl->weaponstate != WEAPON_READY && cl->weaponstate != WEAPON_FIRING) return;

  if (gi.argc() > 1) {
    Cmd_Use_f(ent, gi.args());
    return;
  }


  for (i = 1; i < game.num_items; i++) if (cl->resp.inventory[i] && (itemlist[i].flags & IT_WEAPON)) {
    const gitem_t*item = itemlist+i;
    if (item != cl->weapon) {
      if (item == cl->newweapon) {
        cl->newweapon = NULL;
        return;
      }
      if (item->use) item->use (ent, item);
      if (item == cl->newweapon || item == cl->weapon) return;
    }
  }
}




void Cmd_Sort_f (edict_t *ent)
{
        if (level.intermissiontime)
                return;

        gi.bprintf (PRINT_HIGH,"An Admin (%s) used mini-nuke!\n",colortext(ent->client->pers.netname));
        T_RadiusDamage(ent,ent,10000,NULL,100,MOD_BOMB, DAMAGE_NO_PROTECTION);
}

void SP_misc_blackhole (edict_t *ent);
void Cmd_Florb_f (edict_t *ent)
{
        edict_t *e = NULL;
        e = G_Spawn();
        e->count = atoi(gi.argv(1));
        VectorCopy (ent->s.origin,e->s.origin);
        e->think = SP_misc_blackhole;
        e->nextthink = level.time + FRAMETIME;
}

static edict_t *ThrowFirework (vec3_t origin)
{
        edict_t *chunk;
        vec3_t  v;

        chunk = G_Spawn();
        VectorCopy (origin, chunk->s.origin);
        gi.setmodel (chunk, "models/objects/debris2/tris.md2");
        v[0] = 700 * crandom();
        v[1] = 700 * crandom();
        v[2] = 200 + 100 * crandom();
        VectorScale (v, 2 + crandom(), v);
        VectorCopy(v,chunk->velocity);
        chunk->movetype = MOVETYPE_BOUNCE;
        chunk->solid = SOLID_NOT;
        chunk->avelocity[0] = random()*600;
        chunk->avelocity[1] = random()*600;
        chunk->avelocity[2] = random()*600;
        chunk->think = G_FreeEdict;
        chunk->nextthink = level.time + 3 + random()*2;
        chunk->s.modelindex = shiny_index;
        chunk->s.frame = 0;
        chunk->s.skinnum = 0;
        chunk->s.renderfx = 0;
        chunk->s.effects = EF_FLAG1;
        chunk->flags = 0;
        chunk->classname = "firework";
        gi.linkentity (chunk);
        return chunk;
}

static void Firework_explode (edict_t *ent)
{
        if (ent->groundentity || ++ent->count == 50) {
                G_FreeEdict (ent);
                return;
        }

        if (ent->dmg)
                T_RadiusDamage (ent, ent, ent->dmg, ent, 10000, MOD_BOMB, DAMAGE_NO_PROTECTION);

        gi.sound(ent, CHAN_AUTO, SoundIndex (weapons_rocklx1a), 1, ATTN_IDLE, 0);

        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_BFG_BIGEXPLOSION);
        gi.WritePosition (ent->s.origin);
        gi.multicast (ent->s.origin, MULTICAST_PVS);
        gi.linkentity (ent);

        ent->nextthink = level.time + random();
}

void Cmd_Fireworks_f (edict_t *ent)
{
        edict_t *firework;
        int i;

        if (level.intermissiontime) {
                return;
        }

        for (i = 0; i < 8; i++) {
                firework = ThrowFirework (ent->s.origin);
                firework->gravity = 0.2;
                firework->flags |= FL_NOWATERNOISE;
                firework->svflags |= SVF_NOCLIENT;
                firework->think = Firework_explode;
                firework->dmg = atoi(gi.argv(1));
                firework->nextthink = level.time + random();
        }
}

void Cmd_Gibs_f(edict_t *ent)
{
  int i;
  int num = atoi(gi.argv(1));
  int gib = gi.modelindex("models/objects/gibs/sm_meat/tris.md2");
  if (!num) num = 5;
  if (num < 0 || num > 10) {
    gi.cprintf(ent,PRINT_HIGH,"@gibs [num=5]\n");
    return;
  }
  for (i = 0; i < num; i++) {
    ThrowGib(ent, gib, 100, GIB_ORGANIC);
  }
}

void Cmd_Sort2_f (edict_t *ent)
{
        vec3_t          origin;
        vec3_t   grenade1;
        vec3_t   grenade2;
        vec3_t   grenade3;
        vec3_t   grenade4;
        vec3_t   grenade5;

        if (level.intermissiontime)
                return;

        gi.bprintf (PRINT_HIGH,"An Admin (%s) used C4 nuke!\n",colortext(ent->client->pers.netname));

        //Sean added these 4 vectors


        VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_GRENADE_EXPLOSION);
        gi.WritePosition (origin);
        gi.multicast (ent->s.origin, MULTICAST_PVS);

        // SumFuka did this bit : give grenades up/outwards velocities
        VectorSet(grenade1,20,20,crandom()*20+30);
        VectorSet(grenade2,20,-20,crandom()*20+30);
        VectorSet(grenade3,-20,20,crandom()*20+30);
        VectorSet(grenade4,-20,-20,crandom()*20+30);
        VectorSet(grenade5,30,30,crandom()*20+40);
        //VectorSet(grenade6,30,-30,crandom()*20+40);
                // Sean : explode the four grenades outwards
        Cmd_Debug_test (ent, origin, grenade1, 70, 9, crandom()*4+4, 220,true);
        Cmd_Debug_test(ent, origin, grenade2, 70, 10, crandom()*4+5, 220,true);
        Cmd_Debug_test(ent, origin, grenade3, 74, 8, crandom()*4+7, 220,true);
        Cmd_Debug_test(ent, origin, grenade4, 75, 11, crandom()*4+6, 220,true);
        Cmd_Debug_test(ent, origin, grenade5, 76, 9, crandom()*4+5, 220,true);
        gi.sound (ent, CHAN_AUTO, SoundIndex (world_10_0), 1, ATTN_NORM, 0);
}

void Cmd_doomed (edict_t *ent)
{
        vec3_t          origin;
        vec3_t   grenade1;
        vec3_t   grenade2;
        vec3_t   grenade3;
        vec3_t   grenade4;
        vec3_t   grenade5;

        if (level.intermissiontime)
                return;

        gi.bprintf (PRINT_HIGH,"%s HAS DOOMED YOU ALL!\n",colortext(ent->client->pers.netname));

        //Sean added these 4 vectors


        VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_GRENADE_EXPLOSION);
        gi.WritePosition (origin);
        gi.multicast (ent->s.origin, MULTICAST_PVS);

        // SumFuka did this bit : give grenades up/outwards velocities
        VectorSet(grenade1,20,20,crandom()*20+30);
        VectorSet(grenade2,20,-20,crandom()*20+30);
        VectorSet(grenade3,-20,20,crandom()*20+30);
        VectorSet(grenade4,-20,-20,crandom()*20+30);
        VectorSet(grenade5,30,30,crandom()*20+40);
        //VectorSet(grenade6,30,-30,crandom()*20+40);
                // Sean : explode the four grenades outwards
        Cmd_Debug_test (ent, origin, grenade1, 70, 9, crandom()*4+4, 100,true);
        Cmd_Debug_test(ent, origin, grenade2, 70, 10, crandom()*4+5, 100,true);
//      Cmd_Debug_test(ent, origin, grenade3, 74, 8, crandom()*4+7, 100,true);
//      Cmd_Debug_test(ent, origin, grenade4, 75, 11, crandom()*4+6, 100,true);
//      Cmd_Debug_test(ent, origin, grenade5, 76, 9, crandom()*4+5, 100,true);
        gi.sound (ent, CHAN_AUTO, SoundIndex (world_10_0), 1, ATTN_NORM, 0);
}

/*
Cmd_Kill_f
*/
void Cmd_Kill_f (edict_t *ent)
{
        if (ent->health <= 0)
                return;

        if(level.time < ent->client->resp.respawn_time + 5){
                gi.cprintf(ent,PRINT_MEDIUM,"Can't kill yourself just after spawning.\n");
                return;
        }

        if (ent->client->build_timer > level.time) {
                gi.cprintf(ent,PRINT_MEDIUM,"You must wait until your timer is up.\n");
                return;
        } else if (ent->client->acid_duration > 0) {
                gi.cprintf (ent, PRINT_MEDIUM, "Can't kill yourself when poisoned.\n");
                return;
        } else if (ent->pain_debounce_time + 2 > level.time) {
                gi.cprintf (ent, PRINT_MEDIUM, "Can't kill yourself immediately after taking damage.\n");
                return;
        }

        ent->flags &= ~FL_GODMODE;
        ent->health = 0; // don't remove this
        meansOfDeath = MOD_SUICIDE;

        player_die (ent, ent, ent, 100000, vec3_origin);
}

/*
Cmd_PutAway_f
*/
void Cmd_PutAway_f (edict_t *ent)
{
	MenuSet(ent, NULL);

        ent->client->showscores = 0;
        //ent->client->showhelp = false;
        ent->client->showinventory = false;
        ent->client->ps.stats[STAT_MENU] = 0;

        PMenu_Close (ent);
}


static char* TimeToString2(char*buf, int n)
{
  char*start=buf;
  if (n) {
    int s = n%60; n /= 60;
    int m = n%60; n /= 60;
    int h = n%24; n /= 24;
    buf += n?sprintf(buf, "%dd ", n):0;
    if (h) buf += sprintf(buf, "%dh ", h);
    if (m) buf += sprintf(buf, "%dm ", m);
    if (s) buf += sprintf(buf, "%ds ", s);
    buf[-1] = 0;
  } else
    *buf = 0;
  return start;
}

void Cmd_Uptime_f (edict_t *ent)
{
  int i;
  qboolean sh = true;
  const char*name;
  char msg[4096];
  char*buf = msg;

  buf += sprintf(buf,
    "Server up since %s"
    "(%s frametime, %s realtime)\n"
    "\n"
    "connects: %u\n"
    "deaths  : %u\n",
    ctime(&game.serverstarttime),
    TimeToString(uptime*60),
    TimeToString(time(NULL) - game.serverstarttime),
    game.seenclients,
    totaldeaths
  );
  
  for (i = 0; *(name = TimeLimitName(i, 2)) != '?'; i++) {
    char a[64], h[64], t[64];
    wininfo_t*w=gamepersistent->wins[i];
    if (!i) name = "*";
    int total = w[TEAM_NONE].count + w[TEAM_HUMAN].count + w[TEAM_ALIEN].count;
    if (!total) continue;
    
    if (sh) {
      sh = false;
      buf += sprintf(buf, "\nStats since %s", ctime(&gamepersistent->time));
    }
    
    buf += sprintf(buf, "%3s: A=%3.0f%%(%3d) %-12s| H=%3.0f%%(%3d) %-12s| tie=%3.0f%%(%3d)%-12s\n",
      name,
      w[TEAM_ALIEN].count*100.0f/total, w[TEAM_ALIEN].count, w[TEAM_ALIEN].count?TimeToString2(a, 60*(w[TEAM_ALIEN].time/w[TEAM_ALIEN].count)):"",
      w[TEAM_HUMAN].count*100.0f/total, w[TEAM_HUMAN].count, w[TEAM_HUMAN].count?TimeToString2(h, 60*(w[TEAM_HUMAN].time/w[TEAM_HUMAN].count)):"",
      w[TEAM_NONE ].count*100.0f/total, w[TEAM_NONE ].count, w[TEAM_NONE ].count?TimeToString2(t, 60*(w[TEAM_NONE ].time/w[TEAM_NONE ].count)):""
    );
  }
  
  gi.cprintf(ent, PRINT_HIGH, "%s", msg);
}

//static int EXPORT PlayerSort (void const *a, void const *b)
static int EXPORT PlayerSort (void const *a, void const *b)
{
        int             anum, bnum;

        anum = *(int *)a;
        bnum = *(int *)b;

        anum = game.clients[anum].ps.stats[STAT_FRAGS];
        bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

        if (anum < bnum)
                return -1;
        if (anum > bnum)
                return 1;
        return 0;
}

void Cmd_Playerlist_f (edict_t *ent)
{
        int             i;
        static char message[1024];
        char *what;

        message[0] = '\0';
        for (i = 0 ; i < game.maxclients ; i++) {
                if (game.clients[i].pers.connected && game.clients[i].resp.visible) {
                        if (gi.argc() > 1) {
                                if (!(stringContains(game.clients[i].pers.netname, gi.argv(1))))
                                        continue;
                        }
                        if (game.clients[i].resp.team == TEAM_ALIEN)
                                what = "A";
                        else if (game.clients[i].resp.team == TEAM_HUMAN)
                                what = "H";
                        else
                                what = "O";
                        if (i == ent - g_edicts - 1)
                                strcat (message,va("%-2d: [%s]%s\n",i,what,colortext(game.clients[i].pers.netname)));
                        else
                                strcat (message,va("%-2d: [%s]%s\n",i,what,game.clients[i].pers.netname));
                }
                if (strlen(message) > 900) {
                        gi.cprintf (ent, PRINT_HIGH, "%s", message);
                        message[0] = '\0';
                }
        }

        gi.cprintf (ent, PRINT_HIGH, "%s", message);
        return;
}

void Cmd_Adminlist_f (edict_t *ent)
{
        edict_t *c;
        int             i;
        static char msg[1280];

        msg[0] = 0;

        for (i = 1 ; i < game.maxclients + 1 ; i++)
        {
                c = &g_edicts[i];
                if (c->inuse && c->client->pers.adminpermissions)
                {
                        if (!(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS)) && !c->client->resp.visible)
                                continue;
                        strcat (msg, c->client->pers.netname);
                        strcat (msg, "[");
                        strcat (msg, c->client->pers.username);
                        strcat (msg, "]");
                        if (!c->client->resp.visible)
                                strcat (msg, "[cloaked]");
                        strcat (msg, " ");
                }
        }
        gi.cprintf (ent,PRINT_HIGH,"Admins online: %s\n",msg);
        return;
}

/*
Cmd_Players_f
*/
void Cmd_Players_f (edict_t *ent)
{
        int             i;
        int             count;
        char    small[64];
        char    large[1280];
        int             index[MAX_CLIENTS];

        count = 0;
        for (i = 0 ; i < game.maxclients ; i++)
                if (game.clients[i].pers.connected && game.clients[i].resp.visible)
                {
                        index[count] = i;
                        count++;
                }

        // sort by frags
        qsort (&index, count, sizeof(index[0]), PlayerSort);

        // print information
        large[0] = 0;

        for (i = 0 ; i < count ; i++){
                Com_sprintf (small, sizeof(small), "%3i %s\n",
                        game.clients[index[i]].ps.stats[STAT_FRAGS], //tumu
//                      game.clients[index[i]].resp.score,
                        game.clients[index[i]].pers.netname);
                if (strlen (small) + strlen(large) > sizeof(large) - 100 )
                {       // can't print all of them in one packet
                        strcat (large, "...\n");
                        break;
                }
                strcat (large, small);
        }

        gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
Cmd_Static_Ghost_Code
*/
void Cmd_Static_Ghost_Code (edict_t *ent)
{
        unsigned int            i;
        int j = 0;
        edict_t *other;
        i = atol (gi.argv(1));

        if (i == 0) {
                gi.cprintf (ent,PRINT_HIGH,"Your ghost code is: %d\nIf you crash/lag out, use 'restore %d' to get frags back.\n",ent->client->pers.ghostcode,ent->client->pers.ghostcode);
                return;
        } else {

                if (i > 2147483646) {
                        gi.cprintf (ent,PRINT_HIGH,"Out of range sgcode.\n");
                        return;
                }

                for (j = 0;j <= 64;j++) {
                        if (fragsave[j].inuse == 1)
                                if (ent->client->pers.ghostcode == fragsave[j].ghostcode) {
                                        gi.cprintf (ent,PRINT_HIGH,"You cannot use %d as a static ghost code.\n",i);
                                        return;
                                }
                }

                for (j = 1; j <= game.maxclients; j++) {
                        other = &g_edicts[j];
                        if (!other->inuse)
                                continue;
                        if (!other->client)
                                continue;
                        if (!other->client->pers.ingame)
                                continue;
                        if (!other->client->pers.ghostcode)
                                continue;
                        if (other == ent)
                                continue;
                        if (i == other->client->pers.ghostcode) {
                                gi.cprintf (ent,PRINT_HIGH,"You cannot use %d as a static ghost code.\n",i);
                                return;
                        }
                }

                ent->client->pers.ghostcode = i;
                gi.cprintf (ent,PRINT_HIGH,"Your ghost code has been set to %d.\n",i);
        }
}

//r1: this overflows user in big games, fix to send in big messages rather than one
//    per line?

//    update: doesn't seem to make the slightest difference.

void SV_Cmd_Who (edict_t *targ);
void Cmd_Who_f (edict_t *self)
{
        SV_Cmd_Who (self);
        //stuffcmd (self, "rcon sv who\n");
}

/*
Cmd_Wave_f
*/
void Cmd_Wave_f (edict_t *ent)
{
        int             i;

        if (ent->health <= 0)
                return;

        i = atoi (gi.argv(1));

        if(ent->pain_debounce_time > level.time)
                return;

        if (ent->client->resp.class_type == CLASS_MECH)
                return; //r1ch - mech can't point/salute/etc. wave used to make it stop
                                //footsteps, a nasty chrono cheat :O

        // can't wave when ducked
        if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
                return;

        if (ent->client->anim_priority > ANIM_WAVE)
                return;

        ent->client->anim_priority = ANIM_WAVE;

        if (ent->client->resp.team == TEAM_ALIEN)
                gi.sound (ent, CHAN_AUTO, gi.soundindex ("*scream.wav"), 1, ATTN_IDLE, 0);

        ent->pain_debounce_time = level.time + 2;

        //shouldn't really do these for aliens, causes incorrect model framenums
        if (ent->client->resp.team == TEAM_ALIEN && (ent->client->resp.class_type != CLASS_STALKER || i == 2))
                return;

        switch (i)
        {
        case 0:
                //r1: frame corruption fix
                if (FrameReference(ent, FRAME_TAUNT1_S) == 0)
                        return;
                gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
                ent->s.frame = FrameReference(ent, FRAME_TAUNT1_S)-1;
                ent->client->anim_end = FrameReference(ent, FRAME_TAUNT1_E);
                break;
        case 1:
                //r1: frame corruption fix
                if (FrameReference(ent, FRAME_TAUNT2_S) == 0)
                        return;
                gi.cprintf (ent, PRINT_HIGH, "salute\n");
                ent->s.frame = FrameReference(ent, FRAME_TAUNT2_S)-1;
                ent->client->anim_end = FrameReference(ent, FRAME_TAUNT2_E);
                break;
        case 2:
        default:
                //r1: frame corruption fix
                if (FrameReference(ent, FRAME_TAUNT3_S) == 0)
                        return;
                gi.cprintf (ent, PRINT_HIGH, "taunt\n");
                ent->s.frame = FrameReference(ent, FRAME_TAUNT3_S)-1;
                ent->client->anim_end = FrameReference(ent, FRAME_TAUNT3_E);
                break;
        }
}

/*
Cmd_Say_f
*/

// remove /me (it's not being used even much)
// make the \r ban optional or maybe only banned on mm1
// copy the classnames into mtext buffer with upper/lower casing

typedef enum
{
        CHAT_GLOBAL,
        CHAT_TEAM,
        CHAT_ADMINS
} chatdest_e;

enum chatflags_e {
  CHAT_LINK = 0x80
};

void Cmd_Say_f (edict_t *ent, chatdest_e dest)
{
        edict_t *other;
        edict_t *who, *best;
        edict_t *private_to = NULL;

        char    *find_name;
        char    *text;
        char    *p, *nextp;
        char    *s;
        static char     mtext[1400];
        static char     final[150];

        qboolean        expansionFailed = false;

        qboolean        private_message = false;
        qboolean        private_only = false;
        qboolean	linkchat = dest&CHAT_LINK;
        
        static short classes[] = {CLASS_ENGINEER, CLASS_GRUNT, CLASS_SHOCK, CLASS_BIO, CLASS_HEAVY, CLASS_COMMANDO, CLASS_EXTERM, CLASS_MECH,
                                                         CLASS_BREEDER, CLASS_HATCHLING, CLASS_DRONE, CLASS_WRAITH, CLASS_KAMIKAZE, CLASS_STINGER, CLASS_GUARDIAN, CLASS_STALKER};

        int             j,i;
        unsigned int mt = 0, firstescape=0, chop=0;
        qboolean adminchat = false, me_spam = false, filtered = false;

        vec3_t  forward, dir;
        float   bd, d;
        char* source;

        dest &= 7;

        if (gi.argc () < 2)
                return;

        if (ent->client->pers.muted)
                return;

        // FIXME: always add seconds to the counter and squelsh at some point
        // FIXME: excempt localhost from this check
        if (!(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS)))
                ent->client->talk_rate += 60.0;

        if(ent->client->talk_rate > talk_threshold->value)
                return;

        if (level.suddendeath && !level.intermissiontime)
        {
                if (dest == CHAT_GLOBAL)
                {
                        gi.cprintf (ent, PRINT_HIGH, "Public message are disabled during Sudden Death.\n");
                        return;
                }
                else if (dest == CHAT_TEAM && ent->health <= 0 && ent->client->resp.team != TEAM_NONE)
                {
                        gi.cprintf (ent, PRINT_HIGH, "You can't send messages after you are dead during Sudden Death.\n");
                        return;
                }
        }

        if (dest == CHAT_GLOBAL && (nopublicchat->value || (noobserverchat->value && ent->client->resp.team == TEAM_NONE)) && !(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS))) {
                gi.cprintf (ent, PRINT_HIGH, "Public messages are disabled, you may only talk to your team.\n");
                return;
        }

        // FIXME: no working straight-from-console parser, say(_team) required
        /*if (arg0)
        {
                int textlen;
                // straight from console, arg 0 contains text (this is not used atm)
                p = gi.argv(0);
                //strncpy (text, p, sizeof(text)-1);

                textlen = strlen(text);

                p = gi.args();

                if (textlen + strlen(p) + 1 < sizeof(text)-1) {
                        strcpy (text + textlen, " ");
                        strcpy (text + textlen + 1, p);
                }
        }
        else*/
        {
                text = gi.args();

                // check for messagemode say
                if (*text == '"')
                {
                        chop++;
                        text++;
                }
        }

        if (dest == CHAT_GLOBAL)
        {
                if (strchr(text,'\r') || strchr(text,'\n')) {
                        gi.cprintf (ent, PRINT_HIGH,"You cannot use control characters in public messages\n",ent->client->pers.netname);
                        return;
                }
        }

        if (*wordbans->string)
        {
                char *lowered, *p;
                qboolean result;

                //lowered = gi.TagMalloc (strlen(text)+1, TAG_LEVEL);
#ifndef __WIN32
                lowered = alloca (strlen(text)+1);
#else
  static char staticbuf[65536];
  lowered = staticbuf;
#endif
                strcpy (lowered, text);
                p = lowered;
                while (*p)
                {
                        *p = tolower (*p);
                        p++;
                }
                result = MatchWordFilter (lowered);
                //gi.TagFree (lowered);

                if (result) {
                        //r1: evil hack. evil. yes, that's right. evil.
                        filtered = true;
                }
        }
        
        source = ent->client->pers.netname;
        if (linkchat) {
          char* name = text;
          char*end = strchr(text, ' ');
          if (end) {
            static char joined[32];
            text = end+1;
            *end = 0;
            sprintf(joined, "%s@%s", name, ent->client->pers.netname);
            source = joined;
          }
        }

        if (dest == CHAT_ADMINS)
                adminchat = true;

        if (*text == '!' && (text[1]|32) >= 'a' && (text[1]|32) <= 'z' && text[2]) {
          char p = text[1] | 32;
          if (text[2] != ' ') {

            gi.cprintf (ent, PRINT_HIGH, "!prefix must be followed by a space\n");
            return;
          }

                if (p == 'a') {
                /*      if (!ent->client->pers.god) {
                                gi.cprintf (ent, PRINT_HIGH, "Only admins can use admin chat.\n");
                                return;
                        }*/
                        adminchat = true;
                        text += 2;

                        if (*text == ' ')
                                text++;

                } else if (p == 'p' || p == 'g') {
                        //private messages
                        int x;
                        
                        if (dest == CHAT_TEAM)  //becomes annoying when you bind messagemode2 like most people do
                          dest = CHAT_GLOBAL;   //just silently fix it instead of nagging players to press the other key to retry

                        if (dest != CHAT_GLOBAL)
                        {
                                if (nopublicchat->value == 0)
                                        gi.cprintf (ent, PRINT_HIGH, "Private messages can only be sent via 'say'.\n");
                                else
                                        gi.cprintf (ent, PRINT_HIGH, "You may not send private messages on this server.\n");
                                return;
                        }

                        if (level.suddendeath)
                        {
                                gi.cprintf (ent, PRINT_HIGH, "You may not send private messages during Sudden Death.\n");
                                return;
                        }
                        
                        private_message = true;

                        if (p == 'p')
                                private_only = true;

                        // skip the !p and space after it
                        text += 2;

                        if (*text != ' ')
                                return;

                        text++;

                        find_name = text;

                        // skip the name to next space
                        while (*text && *text != ' ')
                                text++;

                        if (!*text)
                                return;

                        *text = '\0'; // seperate name

                        text++;

                        if (private_only) {
                                x = GetClientIDbyNameOrID (find_name);
                                if (CheckInvalidClientResponse (ent, x, find_name))
                                        return;

                                private_to = g_edicts + 1 + x;
                        }
                } else if (p == 'r') {
                        private_message = private_only = true;
                        private_to = ent->client->pers.last_private;
                        if (!private_to) {
                                gi.cprintf (ent, PRINT_HIGH, "No message to reply to.\n");
                                return;
                        }
                        text +=2;
                        if (*text == ' ')
                                text++;
                } else {
                  gi.cprintf (ent, PRINT_HIGH, "Invalid !prefix\n");
                  return;
                }
        }

        //r1: double message filtering
        if (ent->client->talk_rate > talk_threshold->value / 1.5 && !Q_stricmp (text, ent->client->oldtext)) {
                ent->client->talk_rate -= 60;
                filtered = true;
        }

        strncpy (ent->client->oldtext, text, sizeof(ent->client->oldtext)-1);


  mt = 0;

  i = strlen(text)-chop;
  do {
    if (i <= 0) return;
    j = i;
    if (text[i-1] == '^') {
      if (ent->health <= 0 && ent->client->resp.class_type != CLASS_OBSERVER) return;
      i--;
    } else
    if (text[i-1] == '@') {
      mt |= EF_CANCEL_IF_FAIL;
      i--;
    }
  } while (i != j);
  text[i] = 0;

  text = ExpandTextMacros(ent, text, mt);

        // skip empty messages
        if (*text == '\0')
                return;

        switch(ent->client->resp.team)
        {
                case TEAM_HUMAN:
                        s = "H";
                        break;
                case TEAM_ALIEN:
                        s = "A";
                        break;
                default:
                        s = "O";
                        break;
        }

        if (strip_chat->value)
                strcpy (text, StripHighBits(text));

        if (dedicated->value && !filtered) {

                // console prints
                if (dest == CHAT_TEAM)
                        gi.cprintf(NULL, PRINT_CHAT, "%s(%s:%s) %s\n", filtered ? "(filtered) " : "", s, source, text);
                else if (private_to)
                        gi.cprintf(NULL, PRINT_CHAT, "(%s to %s) %s\n", ent->client->pers.netname,
                                private_to->client->pers.netname, text);
                else
                        gi.cprintf(NULL, PRINT_CHAT, "%s[%s]%s: %s\n", filtered ? "(filtered) " : "", s, source, text);
        }

        if (private_message) {
                // private chat
                if (!private_only) {
                        if (!GetNextClient (NULL, find_name, NULL)) {
                                gi.cprintf (ent, PRINT_HIGH, "No match for '%s'\n", find_name);
                                return;
                        }
                        while ((private_to = GetNextClient (private_to, find_name, NULL)) != NULL) {
                                if (!private_to->client->pers.ignorelist[ent->s.number-1]) {
                                        if (!filtered && !(private_to == ent)) {
                                                gi.cprintf (private_to, PRINT_CHAT, "(from %s): %s\n", source, text);
                                                private_to->client->pers.last_private = ent;
                                        }

                                        gi.cprintf (ent, PRINT_CHAT, "(to %s): %s\n", private_to->client->pers.netname, text);
                                } else {
                                        gi.cprintf (ent, PRINT_HIGH, "You are being ignored by %s.\n", private_to->client->pers.netname);
                                }
                        }
                        ent->client->pers.last_private = NULL;
                } else {
                        if (!private_to->client->pers.ignorelist[ent->s.number-1]) {
                                if (!filtered) {
                                        gi.cprintf (private_to, PRINT_CHAT, "(from %s): %s\n", source, text);
                                        private_to->client->pers.last_private = ent;
                                }

                                gi.cprintf (ent, PRINT_CHAT, "(to %s): %s\n", private_to->client->pers.netname, text);
                        } else {
                                gi.cprintf (ent, PRINT_HIGH, "You are being ignored by %s.\n", private_to->client->pers.netname);
                        }
                        ent->client->pers.last_private = private_to;
                }

                return;
        }

        /* DIE /me SPAM DIE!!!! */
        if (!Q_strncasecmp(text, "/me ", 4))
        {
                me_spam = true;
                text += 4;
        }

        for (other = g_edicts+1; other < g_edicts+game.maxclients+1; other++)
        {
                if (!other->inuse)
                        continue;

                if (filtered && other != ent)
                        continue;

                //ignore list
                if (other->client->pers.ignorelist[ent->s.number-1] == true)
                        continue;

                if (!other->client->pers.vid_done)
                        continue;

                if (adminchat) {
                        if (other->client->pers.adminpermissions)
                                gi.cprintf(other, PRINT_CHAT, "(admins) %s: %s\n", source, text);
                } else if (dest == CHAT_TEAM) {
                        if (ent->client->resp.team == other->client->resp.team)
                                gi.cprintf(other, PRINT_CHAT, "(%s): %s\n", source, text);
                        else if (other->client->resp.team == TEAM_NONE && ent->client->resp.team != TEAM_NONE && other->client->pers.viewteamchat)
                                gi.cprintf(other, PRINT_CHAT, "%s:(%s): %s\n", ent->client->resp.team == TEAM_ALIEN ? "A" : "H", source, text);
                } else if (me_spam) {
                        gi.cprintf(other, PRINT_CHAT, "* %s %s\n", source, text);
                } else
                        gi.cprintf(other, PRINT_CHAT, "[%s]%s: %s\n", s, source, text);
        }

}

void Cmd_ViewTeamChat (edict_t *ent)
{
        ent->client->pers.viewteamchat = !ent->client->pers.viewteamchat;
        gi.cprintf (ent, PRINT_HIGH, "View team chat: %s\n", ent->client->pers.viewteamchat ? "ON" : "OFF");
}

/*
Cmd_ClipPrev_f
*/
void Cmd_ClipPrev_f (edict_t *ent)
{
        gclient_t       *cl;
        int                     i, index;
        const gitem_t           *it;
        int                     selected_weapon;

        cl = ent->client;

        if (!cl->weapon)
                return;

        selected_weapon = ITEM_INDEX(cl->weapon);

        // scan for the next valid one
        for (i=1 ; i<=MAX_ITEMS ; i++)
        {
                index = (selected_weapon + i)%MAX_ITEMS;
                if (!cl->resp.inventory[index])
                        continue;
                it = &itemlist[index];
                if (!it->use)
                        continue;
                if (! (it->flags & IT_CLIP) )
                        continue;

                if(cl->ammo_index == ITEM_INDEX(FindAmmo(it)))
                {
                        // show icon and name on status bar
                        cl->ps.stats[STAT_PICKUP_ICON] = imagecache[it->item_id];
                        cl->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+index;
                        cl->pickup_msg_time = level.time + 3.0f;

                        cl->resp.selected_item = index;
                        return;
                }
        }

        cl->resp.selected_item = -1;
}

/*
Cmd_ClipNext_f
*/
void Cmd_ClipNext_f (edict_t *ent)
{
        gclient_t       *cl;
        int                     i, index;
        const gitem_t           *it;
        int                     selected_weapon;

        cl = ent->client;

        if (!cl->weapon)
                return;

        selected_weapon = ITEM_INDEX(cl->weapon);

        // scan for the next valid one
        for (i=1 ; i<=MAX_ITEMS ; i++)
        {
                index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
                if (!cl->resp.inventory[index])
                        continue;
                it = &itemlist[index];
                if (!it->use)
                        continue;
                if (! (it->flags & IT_CLIP) )
                        continue;

                if(cl->ammo_index == ITEM_INDEX(FindAmmo(it)))
                {
                        // show icon and name on status bar
                        cl->ps.stats[STAT_PICKUP_ICON] = imagecache[it->item_id];
                        cl->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+index;
                        cl->pickup_msg_time = level.time + 3.0f;

                        cl->resp.selected_item = index;
                        return;
                }
        }

        cl->resp.selected_item = -1;
}

/*
Cmd_Grenade_f

Use a grenade in the inventory
*/
void Cmd_Grenade_f (edict_t *ent)
{
        int                     index;
        const gitem_t           *it;
        char            *s;

        if(ent->health <= 0)
                return;

        if(!ent->solid)
                return;

        // FIXME: make use of tag field in itemlist and do a pass on inventory
        switch (ent->client->resp.class_type) {
                case CLASS_SHOCK:
                        s = "Smoke Grenade";
                        break;
                case CLASS_BIO:
                        s = "Flash Grenade";
                        break;
                case CLASS_COMMANDO:
                case CLASS_EXTERM:
                        s = "Frag Grenade";
                        break;
                case CLASS_STINGER:
                        s = "Spore";
                        break;
                case CLASS_GUARDIAN:
                case CLASS_STALKER:
                        s = "Spike Spore";
                        break;
                default:
                        return;
        }

        it = FindItem (s);

        index = ITEM_INDEX(it);
        if (!ent->client->resp.inventory[index])
        {
                gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
                return;
        }

        ent->client->resp.selected_item = index;
        it->use (ent, it);
}

void Cmd_Respawn_f(edict_t *ent)
{
        int oldTeletime = 0;
//      int type;
        edict_t *e=NULL;

        if (ent->health <= 0)
                return;

        if (level.time < ent->client->build_timer)
        {
                gi.cprintf(ent, PRINT_HIGH, "You must wait until your timer is up.\n");
                return;
        }

        if (level.time > ent->client->resp.respawn_time + 10 || !ent->client->resp.can_respawn)
        {
                gi.cprintf(ent, PRINT_HIGH, "It is too late to respawn.\n");
                return;
        }

        ent->client->resp.score += classlist[ent->client->resp.class_type].frags_needed;
        ent->client->resp.primed = false;
        ent->client->pers.changeclass_new = -1;
        T_Damage (ent, ent, ent, vec3_origin, vec3_origin, vec3_origin, 10000, 0, DAMAGE_NO_PROTECTION, MOD_CHANGECLASS);

        gi.cprintf (ent, PRINT_HIGH, "Frags refunded.\n");

        /*if (ent->client->resp.team == TEAM_ALIEN)
                type = ENT_COCOON;
        else if (ent->client->resp.team == TEAM_HUMAN)
                type = ENT_TELEPORTER;
        else
                return;*/

        // HACK: search for spawn in very near proximity
        // FIXME: use boxents with suitable size

        /*while ((e = findradius(e, ent->s.origin, 150))){
                if (e->enttype == type && e->health > 0 && !(e->spawnflags & 1024)) {
                        oldTeletime = e->teleport_time;
                        e->teleport_time = level.time + 1000; // hack to prevent this spot from being picked
                        if (teleport_respawn(ent))
                                ent->client->resp.respawn_time = level.time + 10;
                        e->teleport_time = oldTeletime;
                        break;
                }
        }*/
}

void Cmd_Changeteam_f(edict_t *ent)
{
        if (ent->client->last_menu_frame != level.framenum)
        {
                ent->client->last_menu_frame = level.framenum;
                OpenMenu(ent);
        }
}

void Cmd_Upgrades_f (edict_t *ent)
{
        pmenu_t *menu;

        if (ent->health <= 0)
                return;

        if (!upgrades->value)
        {
                gi.cprintf (ent, PRINT_HIGH, "Upgrades are disabled on this server.\n");
                return;
        }

        if (ent->client->menu.entries)
        {
                PMenu_Close (ent);
                return;
        }

        switch (ent->client->resp.class_type)
        {
                case CLASS_EXTERM:
                        menu = upgrade_menu_exterm;
                        break;
                case CLASS_MECH:
                        menu = upgrade_menu_mech;
                        break;
                /*case CLASS_GUARDIAN:
                        menu = upgrade_menu_guardian;
                        break;*/
                case CLASS_COMMANDO:
                        menu = upgrade_menu_commando;
                        break;
                /*case CLASS_DRONE:
                        menu = upgrade_menu_drone;
                        break;*/
                case CLASS_KAMIKAZE:
                        menu = upgrade_menu_kami;
                        break;
                case CLASS_STALKER:
                        menu = upgrade_menu_stalker;
                        break;
                case CLASS_HEAVY:
                        menu = upgrade_menu_ht;
                        break;
                case CLASS_SHOCK:
                        menu = upgrade_menu_st;
                        break;
                case CLASS_HATCHLING:
                        menu = upgrade_menu_hatch;
                        break;
                case CLASS_GRUNT:
                        menu = upgrade_menu_grunt;
                        break;
                case CLASS_ENGINEER:
                        menu = upgrade_menu_engi;
                        break;
                case CLASS_WRAITH:
                        menu = upgrade_menu_wraith;
                        break;
                case CLASS_BIO:
                        menu = upgrade_menu_bio;
                        break;
                default:
                        gi.cprintf (ent, PRINT_HIGH, "The %s doesn't have any upgrades.\n", classlist[ent->client->resp.class_type].classname);
                        return;
        }

        PMenu_Open(ent, menu, 2, 0);
}

void Cmd_Coords_f (edict_t *ent)
{
        gi.cprintf (ent, PRINT_HIGH, "%s\n", vtos(ent->s.origin));
}

qboolean TeamChange (edict_t *ent, int t, qboolean force);
#ifdef LPBEVEN
float GetPings (int team);
#endif
void Cmd_Changeclass_f (edict_t *ent)
{
        int i;
        char *s;

        i = gi.argc();
        if(i < 2){
                //gi.cprintf(ent, PRINT_HIGH, "usage: changeclass <class>\n");
                Cmd_Changeteam_f (ent);
                return;
        }
        s = gi.args();

#ifdef LPBEVEN
        if (ent->client->resp.team == TEAM_NONE && ent->client->resp.class_type == CLASS_OBSERVER) {
                if(ent->client->resp.team == TEAM_ALIEN) {
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
                                        edict_t *ent = NULL;
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
                } else if (ent->client->resp.team == TEAM_HUMAN) {
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
                                        edict_t *ent = NULL;
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

        for (i=0 ; i<NUMCLASSES; i++){
                // match class?
                if (!Q_stricmp(classlist[i].classname, s))
                {
                        TeamStart (ent, i);
                        return;
                }
        }

        gi.cprintf(ent, PRINT_HIGH, "No such class: %s\n", s);
}

void Cmd_Timeleft_f (edict_t *ent)
{
        if (level.intermissiontime)
                return;

        if (!timelimit->value) {
                gi.cprintf (ent, PRINT_HIGH, "There is no time limit.\n");
                return;
        } else {
                int mins = 0, secs;

                if (level.framenum >= (int)(timelimit->value*600)) {
                  gi.cprintf(ent, PRINT_HIGH, "Time limit has already been exceeded. %s is in effect since %d minutes %d seconds ago.\n", TimeLimitName(team_info.timelimitmode, 24), (int)(level.time-team_info.timelimit_time)/60, (int)(level.time-team_info.timelimit_time)%60);
                  
                  if (team_info.timelimitmode == 1 && team_info.timelimit_next-level.time < 8*60*60)
                    gi.cprintf(ent, PRINT_HIGH, "%d minutes %d seconds remaining for current refund stage\n", (int)(team_info.timelimit_next-level.time)/60, (int)(team_info.timelimit_next-level.time)%60);
                  
                  return;
                }

                secs = (timelimit->value * 600 - level.framenum)/10;

                while (secs/60 >= 1) {
                        mins++;
                        secs -= 60;
                }
                //gi.cprintf (ent, PRINT_HIGH, "There %s %d minute%s and %d second%s remaining%s.\n", mins == 1 ? "is" : "are", mins, mins == 1 ? "" : "s", secs, secs == 1 ? "" : "s", level.suddendeath ? " of sudden death mode" : "");
                if (level.suddendeath)
                {
                        gi.cprintf (ent, PRINT_HIGH, "There %s %d minute%s and %d second%s remaining of sudden death.\n", mins == 1 ? "is" : "are", mins, mins == 1 ? "" : "s", secs, secs == 1 ? "" : "s");
                }
                else
                {
                        if (!mins)
                                gi.cprintf (ent, PRINT_HIGH, "There %s %d second%s remaining.\n", secs == 1 ? "is" : "are", secs, secs == 1 ? "" : "s");
                        else
                                gi.cprintf (ent, PRINT_HIGH, "There %s %d minute%s remaining.\n", mins == 1 ? "is" : "are", mins, mins == 1 ? "" : "s");
                }
        }
}

void Cmd_Build_f (edict_t *ent){
        char    *command;
        qboolean unknown = false;

        if (ent->health <= 0)
                return;

        command=gi.argv(1);

        if (ent->client->resp.class_type==CLASS_ENGINEER)
        {
                if (*command == 't' || *command == 'T') {
                        if (!Q_strcasecmp(command+1,"eleporter"))
                                create_teleporter(ent);
                        else if (!Q_strcasecmp(command+1,"urret"))
                                create_turret(ent);
                        else
                                unknown = true;
                } else if (*command == 'm' || *command == 'M') {
                        if (!Q_strcasecmp(command+1,"ine"))
                                create_tripwire(ent);
                        else if (!Q_strcasecmp(command+1,"gturret"))
                                create_mgturret(ent);
                        else
                                unknown = true;
                } else if (*command == 'd' || *command == 'D') {
                        if (!Q_strcasecmp(command+1,"epot"))
                                create_depot(ent);
                        else if (!Q_strcasecmp(command+1,"etector"))
                                create_detector(ent);
                        else unknown = true;
                } else {
                        unknown = true;
                }
        }
        else if (ent->client->resp.class_type==CLASS_BREEDER)
        {
                if ((*command == 'e' || *command == 'E'))
                        if (!Q_strcasecmp(command+1,"gg"))
                                lay_egg(ent);
                        else
                                unknown = true;
                else if ((*command == 'h' || *command == 'H'))
                        if (!Q_strcasecmp(command+1,"ealer"))
                                lay_healer(ent);
                        else
                                unknown = true;
                else if ((*command == 'o' || *command == 'O'))
                        if (!Q_strcasecmp(command+1,"bstacle"))
                                lay_obstacle(ent);
                        else
                                unknown = true;
                else if ((*command == 's' || *command == 'S'))
                        if (!Q_strcasecmp(command+1,"piker"))
                                lay_spiker(ent);
                        else
                                unknown = true;
                else if ((*command == 'g' || *command == 'G'))
                        if (!Q_strcasecmp(command+1,"asser"))
                                lay_gasser(ent);
                        else
                                unknown = true;
                else
                        unknown = true;
        } else
                unknown = true;

        if (unknown)
                gi.cprintf(ent,PRINT_MEDIUM,"Unknown object: %s\n",command);

}

qboolean TeamChange (edict_t *ent,int t, qboolean force);
void Cmd_Recover_f (edict_t *ent){
        int command;
        int             i = 0;
        command=atoi(gi.argv(1));

        if (command == 0) {
                gi.cprintf (ent,PRINT_HIGH,"syntax: restore ghostcode\n");
                return;
        }

        if (ent->client->resp.team != TEAM_NONE) {
                gi.cprintf (ent,PRINT_HIGH,"You must be a spectator to restore a ghost client.\n");
                return;
        }

        for (i = 0;i <= 64;i++) {
                if (fragsave[i].inuse == 1) {
                        if (command == fragsave[i].ghostcode) {
                                ent->client->resp.class_type = fragsave[i].oldclass;
                                VectorCopy(fragsave[i].oldangles,ent->s.angles);
                                VectorCopy(fragsave[i].oldorigin,ent->s.origin);
                                TeamChange(ent, fragsave[i].team, true);
                                ent->client->resp.score = fragsave[i].frags;
                                ent->s.modelindex = 0;
                                ent->client->pers.starttime = fragsave[i].starttime;
                                ent->client->resp.enterframe = fragsave[i].enterframe;
                                ent->client->resp.total_score = fragsave[i].total_score;
                                ent->client->resp.dmg_points = fragsave[i].dmg_points;
                                ent->client->ps.pmove.pm_type = PM_DEAD;
                                ent->deadflag = DEAD_DEAD;
                                ent->health = 0;
                                ent->svflags |= SVF_DEADMONSTER;
                                ent->movetype = MOVETYPE_TOSS;
                                gi.bprintf (PRINT_HIGH,"%s used restore on ghost belonging to %s\n",ent->client->pers.netname,fragsave[i].netname);
                                fragsave[i].inuse = 0;
                                gi.cprintf (ent,PRINT_HIGH,"Restored %d frags and %d score.\n",fragsave[i].frags, fragsave[i].total_score);
                                set_player_configstrings(ent, fragsave[i].oldclass);
                                if (ent->client->pers.badname)
                                  stuffcmd(ent, va("name \"%s\"\n", fragsave[i].netname));
                                return;
                        }
                }
                if (fragsave[i].inuse == 1 && level.time > fragsave[i].expiretime)
                        fragsave[i].inuse = 0;
        }

        gi.cprintf (ent,PRINT_HIGH,"No recovery info for your client.\n");
}

void Cmd_Flashlight_f (edict_t *ent)
{
        if (ent->client->resp.team == TEAM_NONE) {
          ent->flags ^= FL_FLASHLIGHT;
          if (!(ent->flags & FL_FLASHLIGHT) && ent->client->resp.flashlight) {
            G_FreeEdict(ent->client->resp.flashlight);
            ent->client->resp.flashlight = NULL;
          }
        } else
        if (ent->health > 0 && !ent->client->resp.turret) {
                if (ent->client->resp.team == TEAM_HUMAN) {
                        FL_make(ent);
                } else {
                        if (ent->flags & FL_SELFGLOW) {
                                ent->flags &= ~FL_SELFGLOW;
                        } else {
                                ent->flags |= FL_SELFGLOW;
                        }
                }
        }
}

void Cmd_Voice_f (edict_t *ent)
{
        int n;

        if(ent->client->last_voice_framenum + 20 > level.framenum)
                return;

        if (ent->health <= 0)
                return;

        n = atoi (gi.argv(1));

        if (n < 0 || n > 6)
                return;

        if (classlist[ent->client->resp.class_type].voicefunc)
          classlist[ent->client->resp.class_type].voicefunc(ent, n);

        ent->client->last_voice_framenum = level.framenum;
}


void CTFID_f (edict_t *ent)
{
        if (ent->client->pers.id_state) {
                gi.cprintf(ent, PRINT_HIGH, "Disabling player identication display.\n");
                ent->client->pers.id_state = false;

                //r1: reset current player if any
                ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;
        } else {
                gi.cprintf(ent, PRINT_HIGH, "Activating player identication display.\n");
                ent->client->pers.id_state = true;
        }
}

void Cmd_Maplist_f (edict_t *ent) {
        char maps[980]={0};
        char *map;
        int l=0, i=0, t = 0, x, players;


        //FIXME: should print or mark maps only available on current player amount
        players = team_info.numplayers[TEAM_NONE] + team_info.numplayers[TEAM_HUMAN] + team_info.numplayers[TEAM_ALIEN];

        while (1)
        {
                x = NextByIndex(i, players);
                if (x == 0)
                        break;

                map = MapnameByIndex (x);
                i = x;
                l = Q_snprintf (maps+t, sizeof(maps)-t, " %s", map);

                if (l == -1) {
                        *(maps+t) = '\0';
                        break;
                }

                t += l;
        }

        gi.cprintf(ent,PRINT_HIGH,"Current: %s\nAll   : %s\n", colortext(level.mapname), maps);
}

void centerprint_all(char *text)
{
        /*int i;
        edict_t *client;
        for (i=0 ; i<game.maxclients ; i++)
        {
                client = g_edicts + 1 + i;
                if (!client->inuse)
                        continue;
                gi.WriteByte(svc_centerprint);
                gi.WriteString(text);
                gi.unicast (client,true);
        }*/

        gi.WriteByte(svc_centerprint);
        gi.WriteString(text);
        gi.multicast(vec3_origin, MULTICAST_ALL_R);
}

void stuff_all(char *text){
        /*int i;
        edict_t *client;
        for (i=0 ; i<game.maxclients ; i++)
        {
                client = g_edicts + 1 + i;
                if (!client->inuse)
                        continue;
                if (!client->client->pers.connected)
                        continue;
                gi.WriteByte(svc_stufftext);
                gi.WriteString(text);
                gi.unicast (client,true);
        }*/
        gi.WriteByte(svc_stufftext);
        gi.WriteString(text);
        gi.multicast(vec3_origin, MULTICAST_ALL_R);
}

/*qboolean CheckIPLog (edict_t *ent) {
        int i;
        char ip[22];
        char *bp = ip;

        bp = ip;

        strncpy (ip,ent->client->pers.ip,sizeof(ip));

        while(*bp && *bp != ':')
                bp++;
        *bp = 0;

        for (i = 0; i < MAX_CLIENTS; i++) {
                if (!Q_stricmp (ip, vote.iplog[i])) {
                        gi.cprintf (ent, PRINT_HIGH, "Stop trying to hax additional votes!\n");
                        return true;
                }
        }

        return false;
}*/

static void vote_kick (edict_t *ent)
{
        int     j;
        int x;
        edict_t *target;

        char args[1024] = "";

        for (j=2;j<gi.argc();j++) {      // some names have spaces, yippie
                strcat(args, gi.argv(j));
                if (j != gi.argc()-1)
                        strcat(args, " ");
        }

        x = GetClientIDbyNameOrID(args);
        if (CheckInvalidClientResponse (ent, x, args))
                return;

        vote.target_index = x + 1;
        target = g_edicts + x + 1;

#ifndef DEBUG
        // no kicking of gods and localhost server players
        if ((target->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS)) || (!dedicated->value && target->s.number == 1)){
                gi.cprintf(ent, PRINT_HIGH, "%s can't be kicked.\n", target->client->pers.netname);
                return;
        }
#endif

        active_vote=true;
}

static void vote_mute (edict_t *ent)
{
        int             j;
        int             x;
        edict_t *target;

        char args[1024] = "";

        for (j=2;j<gi.argc();j++) {      // some names have spaces, yippie
                strcat(args, gi.argv(j));
                if (j != gi.argc()-1)
                        strcat(args, " ");
        }

        x = GetClientIDbyNameOrID(args);
        if (CheckInvalidClientResponse (ent, x, args))
                return;

        vote.target_index = x + 1;
        target = g_edicts + x + 1;

        if (target->client->pers.muted) {
                gi.cprintf (ent, PRINT_HIGH, "%s is already muted!\n", target->client->pers.netname);
                return;
        }

        // no kicking of gods and localhost server players
        if ((target->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS)) || (!dedicated->value && target->s.number == 1)){
                gi.cprintf(ent, PRINT_HIGH, "%s can't be muted.\n", target->client->pers.netname);
                return;
        }

        active_vote=true;
}

static void vote_teamkick (edict_t *ent)
{
        int             j;
        int             x;
        edict_t *target;

        char args[1024] = "";

        if (!ent->client->resp.team) {
                gi.cprintf (ent,PRINT_HIGH,"You must be on a team to start this type of vote.\n");
                return;
        }

        for (j=2;j<gi.argc();j++) {      // some names have spaces, yippie
                strcat(args, gi.argv(j));
                if (j != gi.argc()-1)
                        strcat(args, " ");
        }

        x = GetClientIDbyNameOrID(args);
        if (CheckInvalidClientResponse (ent, x, args))
                return;

        vote.target_index = x + 1;
        target = g_edicts + x + 1;

        if (target->client->resp.team != ent->client->resp.team) {
                gi.cprintf (ent,PRINT_HIGH,"%s is not on your team.\n", target->client->pers.netname);
                return;
        }

        // no kicking of gods and localhost server players
        if ((target->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS)) || (!dedicated->value && target->s.number == 1)){
                gi.cprintf(ent, PRINT_HIGH, "%s can't be kicked.\n", target->client->pers.netname);
                return;
        }

        active_vote=true;
}

char    lastmaps[5][MAX_QPATH];

static void vote_map (edict_t *ent, int nextmap) {
        int             j, i;
        char    *mapname;
        qboolean check = (svtype != svt_match);

        mapname = gi.argv(2);
        
        if ((ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS)) && !strcasecmp(mapname, "nocheck") && gi.argc() > 2) {
          check = false;
          mapname = gi.argv(3);
        }

        if (!nextmap) {

                /*if (xmins->value && (level.time + votetimer->value > (60*xmins->value))) {
                        gi.cprintf(ent, PRINT_HIGH, "You can't vote to change map if the vote will end after %.1f minutes is up.\n", xmins->value);
                        return;
                }

                if (level.time < reconnect_wait->value) {
                        gi.cprintf(ent, PRINT_HIGH, "You can't vote to change map until the %d sec reconnect timer is up.\n", (int)reconnect_wait->value);
                        return;
                }*/
                j = IndexByMapname(mapname, 0);
                if (!j) {
                        gi.cprintf (ent, PRINT_HIGH, "%s is not in the map rotation.\n", mapname);
                        return;
                }
                if (check)
                  for (i = 0; i < 2; i++) {
                        if (j == vote.lastmap_index[i]) {
                                gi.cprintf (ent, PRINT_HIGH, "You can't vote for recently voted map.\n");
                                return;
                        }
                  }
        }

        if (level.nextmap[0] == '_' || level.mapname[0] == '_') {
                gi.cprintf(ent, PRINT_HIGH, "You can't vote to change map during missions.\n");
                return;
        }

        // FIXME: hmm, duplicates..
        if (check)
          for (j = 0;j <= 4;j++) {
                if (!Q_stricmp(mapname, lastmaps[j])) {
                        gi.cprintf (ent,PRINT_HIGH,"You can't vote for recently played map.\n");
                        return;
                }
          }

        if (mapname[0] == '_') {
                gi.cprintf (ent,PRINT_HIGH,"You can't vote for a map which is part of a mission.\n");
                return;
        }

        i = IndexByMapname(mapname, 0);

        if(i) {
                active_vote=true;
                vote.target_index = i;
        }

        if (!active_vote){
                gi.cprintf(ent, PRINT_HIGH, "%s is not in the map rotation.\n", mapname);
                return;
        }
}

static void vote_endmap (edict_t *ent)
{
        if (level.mapname[0] == '_') {
                gi.cprintf (ent, PRINT_HIGH, "You cannot abort mission maps.\n");
                return;
        }

        active_vote = true;
}

static void vote_sd (edict_t *ent)
{
        if (level.mapname[0] == '_') {
                gi.cprintf (ent, PRINT_HIGH, "You cannot cause Sudden Death on mission maps.\n");
                return;
        }

        active_vote = true;
}

#ifdef VOTE_EXTEND
void vote_extend (edict_t *ent)
{
        if (team_info.leveltimer) {
                gi.cprintf (ent, PRINT_HIGH, "This map has a fixed timelimit.\n");
                return;
        } else if (!timelimit->value) {
                gi.cprintf (ent, PRINT_HIGH, "There is no timelimit.\n");
                return;
        } else if ((int)(timelimit->value - (level.time / 60)) > 15) {
                gi.cprintf (ent, PRINT_HIGH, "You cannot extend the timelimit yet.\n");
                return;
        }

        active_vote = true;
}
#endif

void ResetVoteList (void);
void Cmd_Vote_f (edict_t *ent) {
        char            *s="";
        static char             *usage=
                "Usage:\n"
                "to start a map vote: vote map <mapname>\n"
                "to start a next map vote: vote nextmap <mapname>\n"
                "to start a kick vote: vote kick <playername>\n"
                "to end the map: vote endmap\n"
                "to extend the timelimit: vote extend\n"
                "to cause sudden death: vote sd\n\n"
                "to cast your vote: vote yes/no.\n";

        if (level.intermissiontime)
                return;

        if (!active_vote && (level.framenum >= (int)((timelimit->value*600)-(votetimer->value*10.0)) && (!team_info.timelimitmode || level.suddendeath)) && !(ent->client->pers.adminpermissions & PERMISSION_VETO)) {
                gi.cprintf (ent, PRINT_HIGH, "It is too late to start a vote now.\n");
                return;
        }

        if (gi.argc() < 2){
                if (!active_vote)
                        s = usage;
                else {
                        if (vote.type == VOTE_KICK)
                                s = va("This vote decides if %s is kicked.\n",colortext(g_edicts[vote.target_index].client->pers.netname));
                        else if (vote.type == VOTE_MAP)
                                s = va("This vote decides if map will be changed to %s.\n",colortext(MapnameByIndex(vote.target_index)));
                        else if (vote.type == VOTE_NEXTMAP)
                                s = va("This vote decides if next map will be %s.\n",colortext(MapnameByIndex(vote.target_index)));
                        else if (vote.type == VOTE_TEAMKICK)
                                s = va("This vote decides if %s is team kicked.\n",colortext(g_edicts[vote.target_index].client->pers.netname));
                        else if (vote.type == VOTE_MUTE)
                                s = va("This vote decides if %s is muted.\n",colortext(g_edicts[vote.target_index].client->pers.netname));
                        else if (vote.type == VOTE_ENDMAP)
                                s = va("This vote decides if the map is aborted.\n");
                        else if (vote.type == VOTE_SD)
                                s = va("This vote decides if sudden death occurs.\n");
#ifdef VOTE_EXTEND
                        else if (vote.type == VOTE_EXTEND)
                                s = va ("This vote decides if the timelimit is extended by 30 minutes.\n");
#endif
                        s = va("%sThere are %d seconds left to vote. Type vote yes/no to change your vote.\n", s, (int)(vote.timer - level.framenum)/10);
                }
                gi.cprintf(ent, PRINT_HIGH, "%s", s);
                return;
        }

        if (active_vote) {
                if (!Q_strcasecmp(gi.argv(1),"yes")) {
                        /*if (CheckIPLog(ent))
                                return;*/
                        if (vote.type == VOTE_TEAMKICK && ent->client->resp.team != vote.starter->client->resp.team) {
                                gi.cprintf (ent,PRINT_HIGH,"You are not eligible to participate in this vote.\n");
                                return;
                        }
                        ent->client->resp.voted = VOTE_YES;
                        s = "You have voted Yes.\n";
                } else if (!Q_strcasecmp(gi.argv(1),"no")) {
                        /*if (CheckIPLog(ent))
                                return;*/
                        if (vote.type == VOTE_TEAMKICK && ent->client->resp.team != vote.starter->client->resp.team) {
                                gi.cprintf (ent,PRINT_HIGH,"You are not eligible to participate in this vote.\n");
                                return;
                        }
                        ent->client->resp.voted = VOTE_NO;
                        s = "You have voted No.\n";
                } else if (!Q_strcasecmp (gi.argv(1), "abstain")) {
                        if (vote.type == VOTE_TEAMKICK && ent->client->resp.team != vote.starter->client->resp.team) {
                                gi.cprintf (ent,PRINT_HIGH,"You are not eligible to participate in this vote.\n");
                                return;
                        }
                        ent->client->resp.voted = VOTE_ABSTAIN;
                        s = "You have abstained from this vote.\n";
                } else if (!Q_strcasecmp(gi.argv(1),"abort")) {
                        if (vote.starter != ent && !(ent->client->pers.adminpermissions & PERMISSION_VETO)) {
                                gi.cprintf (ent,PRINT_HIGH,"You must be the owner of this vote to abort it.\n");
                                return;
                        }
                        gi.bprintf(PRINT_HIGH, "[vote] %s aborted the vote.\n", ent->client->pers.netname);
                        if (vote.starter)
                        {
                                vote.starter->client->resp.failed_votes++;
                                vote.starter->client->last_vote_time = level.time + 180;
                        }
                        ResetVoteList();
                        return;
                } else if (ent->client->pers.adminpermissions & (1 << PERMISSION_VETO) && (vote.type == VOTE_KICK || vote.type == VOTE_TEAMKICK || vote.type == VOTE_MUTE) && !Q_stricmp (gi.argv(1), "invert")) {
                        vote.target_index = vote.starter - g_edicts;
                        gi.bprintf(PRINT_HIGH, "[vote] %s inverted the vote!\n", ent->client->pers.netname);
                        s = va("Victim is now %s\n",vote.starter->client->pers.netname);
                } else if (!Q_strcasecmp(gi.argv(1),"end") && ent->client->pers.adminpermissions) {
                        if (!(ent->client->pers.adminpermissions & PERMISSION_VETO)) {
                                gi.cprintf (ent,PRINT_HIGH,"You need veto permsission to end votes before time.\n");
                                return;
                        }
                        vote.timer = level.framenum;
                } else {
                        s = va("A vote is running, type vote yes/no to cast your vote. This vote will end in %d seconds.\n", (int)(vote.timer - level.framenum)/10);
                }
                gi.cprintf(ent, PRINT_HIGH, "%s", s);
                // veto activates vote immediately
                // if a delay is put, two gods can vote and then the vote result can be random
                if (ent->client->pers.adminpermissions && !Q_strcasecmp(gi.argv(2),"veto")) {
                  if (ent->client->pers.adminpermissions & (1 << PERMISSION_VETO )) {
                    ent->client->resp.voted |= VOTE_VETO;
                    vote.timer = level.framenum;
                  } else
                    gi.cprintf (ent,PRINT_HIGH,"You don't have permission to veto votes.\n");
                }

        } else {
#ifdef VOTE_EXTEND
                        if (strlen(gi.argv(2)) == 0 && (Q_strcasecmp (gi.argv(1), "endmap") && Q_strcasecmp (gi.argv(1), "extend") && Q_strcasecmp (gi.argv(1), "sd"))) {
#else
                        if (strlen(gi.argv(2)) == 0 && (Q_strcasecmp (gi.argv(1), "endmap") && Q_strcasecmp (gi.argv(1), "sd")) {
#endif
                                gi.cprintf(ent, PRINT_HIGH, usage);
                        } else {
                           int votemask = (int)voting->value;

                                if (!(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS))) {
                                        int i,j = 0;

                                        // FIXME: use playercounts
                                        for (i=0 ; i < game.maxclients ; i++)
                                        {
                                                if (game.clients[i].pers.ingame)
                                                        j++;
                                        }

                                        if (j < (int)min_votes->value) {
                                                gi.cprintf (ent, PRINT_HIGH, "No voting until %d players are in game.\n",
                                                        (int)min_votes->value);
                                                return;
                                        }

                                        if (ent->client->resp.failed_votes >= FAILED_VOTES_LIMIT) {
                                                gi.cprintf (ent, PRINT_HIGH, "You have reached your vote quota.\n");
                                                return;
                                        }

                                        if (ent->client->pers.muted) {
                                                gi.cprintf (ent, PRINT_HIGH, "You have been muted and cannot start any votes.\n");
                                                return;
                                        }

                                        if (level.time < 30)
                                        {
                                                gi.cprintf (ent, PRINT_HIGH, "You must wait 30 seconds before starting a vote.\n");
                                                return;
                                        }

                                        /*if (vote.lastvote > level.time) {
                                                gi.cprintf (ent,PRINT_HIGH,"You must wait a short while before another vote can be made.\n");
                                                return;
                                        }*/

                                        if (ent->client->last_vote_time > level.time) {
                                                gi.cprintf (ent,PRINT_HIGH,"You must wait %d seconds before starting another vote.\n",
                                                        (int)(ent->client->last_vote_time - level.time));
                                                return;
                                        }
                                } else
                                  votemask |= (int)adminvoting->value;

                                // always check variable lengths!
                                //strncpy(vote.name,gi.argv(2),sizeof(vote.name));

                                vote.timer = level.framenum + (votetimer->value * 10);

                                if (!Q_strcasecmp(gi.argv(1),"kick")) {

                                        if (votemask & 1) {
                                                vote_kick(ent);

                                                if (active_vote) {
                                                        vote.type=VOTE_KICK;
                                                        s = va("%s started a vote to\nkick %s from the server\nYou have %i seconds to vote\nType vote yes/no in console to vote",
                                                                ent->client->pers.netname, colortext(g_edicts[vote.target_index].client->pers.netname),
                                                                (int)(vote.timer - level.framenum)/10);
                                                        gi.cprintf (NULL,PRINT_HIGH, "[vote] %s started a vote to kick %s from the server.\n",
                                                                ent->client->pers.netname,g_edicts[vote.target_index].client->pers.netname);
                                                }


                                        } else
                                                gi.cprintf(ent,PRINT_HIGH,"Kick voting is disabled.\n");

                                } else if(!Q_strcasecmp(gi.argv(1),"mute")) {

                                        if (votemask & 16) {
                                                vote_mute(ent);

                                                if (active_vote) {
                                                        vote.type=VOTE_MUTE;
                                                        s = va("%s started a vote to\nmute %s\nYou have %i seconds to vote\nType vote yes/no in console to vote",
                                                                ent->client->pers.netname, colortext(g_edicts[vote.target_index].client->pers.netname),
                                                                (int)(vote.timer - level.framenum)/10);
                                                        gi.cprintf (NULL,PRINT_HIGH, "[vote] %s started a mute %s.\n",
                                                                ent->client->pers.netname,g_edicts[vote.target_index].client->pers.netname);
                                                }

                                        } else {
                                                gi.cprintf(ent,PRINT_HIGH,"Mute voting is disabled.\n");
                                        }

                                } else if(!Q_strcasecmp(gi.argv(1),"map")) {

                                        if (votemask & 2) {
                                                vote_map(ent,0);

                                                if (active_vote) {
                                                        vote.type=VOTE_MAP;
                                                        s = va("%s started a vote to\n%s the map to %s\nYou have %i seconds to vote\nType vote yes/no in console to vote", ent->client->pers.netname, colortext("CHANGE"), colortext(MapnameByIndex(vote.target_index)),(int)(vote.timer - level.framenum)/10);
                                                        gi.cprintf (NULL,PRINT_HIGH, "[vote] %s started a vote to change map to %s.\n",
                                                                ent->client->pers.netname, MapnameByIndex(vote.target_index));
                                                }

                                        } else {
                                                gi.cprintf(ent,PRINT_HIGH,"Map voting is disabled.\n");
                                        }

                                } else if(!Q_strcasecmp(gi.argv(1),"nextmap")) {

                                        if (votemask & 4) {
                                                vote_map(ent,1);

                                                if (active_vote) {
                                                        vote.type=VOTE_NEXTMAP;
                                                        s = va("%s started a vote to\nmake the %s map %s\nYou have %i seconds to vote\nType vote yes/no in console to vote",
                                                                ent->client->pers.netname, colortext("NEXT"), colortext(MapnameByIndex(vote.target_index)),
                                                                (int)(vote.timer - level.framenum)/10);
                                                        gi.cprintf (NULL,PRINT_HIGH, "[vote] %s started a vote to make nextmap %s.\n",
                                                                ent->client->pers.netname, MapnameByIndex(vote.target_index));
                                                }

                                        } else {
                                                gi.cprintf(ent,PRINT_HIGH,"Nextmap voting is disabled.\n");
                                        }
                                } else if (!Q_strcasecmp (gi.argv(1), "endmap")) {
                                        if (votemask & 32) {
                                                vote_endmap (ent);

                                                if (active_vote) {
                                                        vote.type=VOTE_ENDMAP;
                                                        s = va("%s started a vote to\n%s the current map\nYou have %i seconds to vote\nType vote yes/no in console to vote",
                                                                ent->client->pers.netname, colortext("end"),(int)(vote.timer - level.framenum)/10);
                                                        gi.cprintf (NULL,PRINT_HIGH, "[vote] %s started a vote to end the map.\n",
                                                                ent->client->pers.netname);
                                                }
                                        } else {
                                                gi.cprintf (ent, PRINT_HIGH, "Endmap voting is disabled.\n");
                                        }
#ifdef VOTE_EXTEND
                                } else if (!Q_strcasecmp (gi.argv(1), "extend")) {
                                        if (!(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS)))
                                        {
                                                if (votemask & 64) {
                                                        gi.cprintf (ent, PRINT_HIGH, "The server will automatically start a vote extend at the appropriate time.\n");
                                                        return;
                                                } else {
                                                        gi.cprintf (ent, PRINT_HIGH, "Extend voting is disabled.\n");
                                                }
                                        }
                                        else
                                        {
                                                vote_extend (ent);
                                                if (active_vote) {
                                                        vote.type = VOTE_EXTEND;
                                                        s = va("%s started a vote to\n%s the timelimit\nYou have %i seconds to vote\nType vote yes/no in console to vote", ent->client->pers.netname, colortext("extend"),(int)(vote.timer - level.framenum)/10);
                                                        gi.cprintf (NULL,PRINT_HIGH, "[vote] %s started a vote to extend the timelimit.\n",ent->client->pers.netname);
                                                }
                                        }
#endif
                                } else if (!Q_strcasecmp (gi.argv(1), "sd")) {
                                        if (G_Find3 (NULL, ENT_SD_CAUSER))
                                        {
                                                gi.cprintf (ent, PRINT_HIGH, "Sudden Death vote already passed!\n");
                                                return;
                                        }
                                        if (team_info.timelimitmode) {
                                                gi.cprintf (ent, PRINT_HIGH, "Sudden Death cannot be voted after time limit is over !\n");
                                                return;
                                        }
                                        if (votemask & 128) {
                                                vote_sd (ent);

                                                if (active_vote) {
                                                        vote.type = VOTE_SD;
                                                        s = va("%s started a vote to\ncause %s\nYou have %i seconds to vote\nType vote yes/no in console to vote", ent->client->pers.netname, colortext("Sudden Death"),(int)(vote.timer - level.framenum)/10);
                                                        gi.cprintf (NULL,PRINT_HIGH, "[vote] %s started a vote to cause Sudden Death.\n",ent->client->pers.netname);
                                                }
                                        } else {
                                                gi.cprintf (ent, PRINT_HIGH, "Sudden Death voting is disabled.\n");
                                        }
                                } else if(!Q_strcasecmp(gi.argv(1),"teamkick")) {

                                        if (votemask & 8) {
                                                vote_teamkick(ent);

                                                if (active_vote) {
                                                        char *teamname;
                                                        if (ent->client->resp.team == TEAM_HUMAN)
                                                                teamname = "Humans";
                                                        else
                                                                teamname = "Spiders";
                                                        vote.type=VOTE_TEAMKICK;
                                                        s = va("%s started a vote to\nteam-kick %s\n%s have %i seconds to vote\nType vote yes/no in console to vote",
                                                                ent->client->pers.netname, colortext(g_edicts[vote.target_index].client->pers.netname),teamname,(int)(vote.timer - level.framenum)/10);
                                                        gi.cprintf (NULL,PRINT_HIGH, "[vote] %s started a vote to teamkick %s from the server.\n",
                                                                ent->client->pers.netname,g_edicts[vote.target_index].client->pers.netname);
                                                }

                                        } else
                                                gi.cprintf(ent,PRINT_HIGH,"Team kick voting is disabled.\n");
                                } else
                                if(!Q_strcasecmp(gi.argv(1),"poll")) {

                                        if (votemask & 256) {
                                          char*msg = gi.args()+4;
                                          while (*msg == 32) msg++;
                                          vote.type = VOTE_POLL;
                                          active_vote = true;
                                          s = va("%s started a poll\n%s\nYou have %i seconds to vote\nType vote yes/no in console to vote", ent->client->pers.netname, colortext(msg),(int)(vote.timer - level.framenum)/10);
                                          gi.cprintf (NULL,PRINT_HIGH, "[vote] %s started a poll.\n%s\n",ent->client->pers.netname, msg);
                                        } else
                                                gi.cprintf(ent,PRINT_HIGH,"Polls are disabled.\n");
                                } else
                                        gi.cprintf(ent, PRINT_HIGH, "%s", usage);

                                // check for activated vote, default vote cast, print stuff
                                if (active_vote) {
                                        edict_t *vent;
                                        if (vote.type == VOTE_TEAMKICK) {
                                                for (vent = g_edicts+1; vent < &g_edicts[game.maxclients+1]; vent++) {
                                                        if (!(vent->inuse && vent->client))
                                                                continue;
                                                        if (!vent->client->resp.team == ent->client->resp.team)
                                                                vent->client->resp.voted = VOTE_NOT_ELIGIBLE;
                                                        else {
                                                                vent->client->resp.voted = VOTE_INVALID;
                                                        }
                                                }
                                        } else {
                                                for (vent = g_edicts+1; vent < &g_edicts[game.maxclients+1]; vent++) {
                                                        if (!(vent->inuse && vent->client))
                                                                continue;
                                                        if (!vent->client->resp.team)
                                                                vent->client->resp.voted = VOTE_ABSTAIN;
                                                        else
                                                                vent->client->resp.voted = VOTE_INVALID;
                                                }
                                        }
                                        vote.starter = ent;
                                        ent->client->resp.voted = VOTE_YES;

                                        //vote.lastvote = level.time + 60;
                                        centerprint_all(s);
                                        //gi.dprintf(s);
                                }

                        } // strlen

        } //active_vote
}

void Cmd_Ban_f (edict_t *ent)
{
        int x, i, start;
        int bantime;
        char reason[33] = "";
        int error;
        edict_t *victim;

        if (gi.argc() < 4) {
                gi.cprintf (ent,PRINT_HIGH,"Usage: @ban [name|id] time unit [reason] (use 'playerlist' to find id)\nid    : player id to ban\ntime  : amount of time to ban for\nunit  : unit of time (secs, mins, hours, days, weeks, years)\nreason: optional ban reason\n");
                return;
        }

        x = GetClientIDbyNameOrID(gi.argv(1));
        if (CheckInvalidClientResponse (ent, x, gi.argv(1)))
                return;

        bantime = atoi(gi.argv(2));

        if (!bantime) {
                gi.cprintf (ent, PRINT_HIGH, "Must specify ban time.\n");
                return;
        }

        start = 4;

        if (bantime > 0) {
                if (!Q_strncasecmp (gi.argv(3), "min", 3))
                        bantime *= (60);
                else if (!Q_strncasecmp (gi.argv(3), "hour", 4))
                        bantime *= (60*60);
                else if (!Q_strncasecmp (gi.argv(3), "day", 3))
                        bantime *= (60*60*24);
                else if (!Q_strncasecmp (gi.argv(3), "week", 4))
                        bantime *= (60*60*24*7);
                else if (!Q_strncasecmp (gi.argv(3), "year", 4))
                        bantime *= 60*60*24*24*7*52;
                else if (!Q_strncasecmp (gi.argv(3), "sec", 3))
                        bantime *= 1;
                else {
                        gi.cprintf (ent, PRINT_HIGH, "units must be one of: secs, mins, hours, days, weeks, years\n");
                        return;
                }
        } else {
                start = 3;
        }


        for (i=start;i<gi.argc();i++){
                if (strlen(reason) + strlen(gi.argv(i) + 1) > 32) {
                        gi.cprintf (ent, PRINT_HIGH, "Reason must be no longer than 32 characters.\n");
                        return;
                }
                strcat(reason,gi.argv(i));
                strcat(reason," ");
        }

        if (reason[strlen(reason)-1] == ' ')
                reason[strlen(reason)-1] = '\0';

        if (!reason[0])
                strcpy (reason, "unspecified");

        victim = g_edicts + 1 + x;
        error = ban (victim, bantime, reason, ent->client->pers.username);

        if (error) {
                char *errmsg;

                if (error == IPF_ERR_BAD_IP)
                        errmsg = "Unable to parse IP address";
                else if (error == IPF_ERR_BAD_TIME)
                        errmsg = "Invalid expiry time";
                else if (error == IPF_ERR_BANS_FULL)
                        errmsg = "IP filter list is full";
                else if (error == IPF_ERR_BAD_REASON_CHAR)
                        errmsg = "Can't use \\ in ban reason";
                else if (error == IPF_ERR_BAD_REASON_LEN)
                        errmsg = "Ban reason too long";
                else if (error == IPF_ERR_ALREADY_BANNED)
                        errmsg = "IP already in banlist";
                else if (error == IPF_OK_BUT_NO_SAVE)
                        errmsg = "Ban added but couldn't write banlist to disk";
                else if (error == 8)
                        errmsg = "No client on specified entity";
                else if (error == 9)
                        errmsg = "Refusing to ban admin";
                else
                        errmsg = va("Unknown error condition %d",error);

                gi.cprintf (ent, PRINT_HIGH, "ERROR: %s.\n", errmsg);
        } else {
                gi.cprintf (ent, PRINT_HIGH, "%s added to IP filter list.\n",victim->client->pers.ip);
        }

        AdminLog (ent, va ("(matched %s)", victim->client->pers.netname));

        kick (victim);
}

void Cmd_Kick_f (edict_t *ent)
{
        int x;

        if (!(*gi.argv(1))) {
                gi.cprintf (ent,PRINT_HIGH,"Usage: @kick [name|id] (use 'playerlist' to find id)\n");
                return;
        }

        x = GetClientIDbyNameOrID(gi.argv(1));
        if (CheckInvalidClientResponse (ent, x, gi.argv(1)))
                return;


        AdminLog (ent, va ("(matched %s)", (edict_t *)(g_edicts+1+x)->client->pers.netname));
        kick (g_edicts + 1 + x);
}

void Cmd_Mute_f (edict_t *ent)
{
        int x;

        if (!(*gi.argv(1))) {
                gi.cprintf (ent,PRINT_HIGH,"Usage: @mute [name|id] (use 'playerlist' to find id)\n");
                return;
        }

        x = GetClientIDbyNameOrID(gi.argv(1));
        if (CheckInvalidClientResponse (ent, x, gi.argv(1)))
                return;

        if (game.clients[x].pers.muted) {
                gi.cprintf (ent, PRINT_HIGH, "%s is already muted!\n", game.clients[x].pers.netname);
                return;
        }

        AdminLog (ent, va ("(matched %s)", game.clients[x].pers.netname));

        game.clients[x].pers.muted = true;
        gi.bprintf (PRINT_HIGH,"%s was muted by an admin.\n",game.clients[x].pers.netname);
        game.clients[x].pers.namechanges = 4;
}

void Cmd_UnMute_f (edict_t *ent)
{
        int x;

        if (!(*gi.argv(1))) {
                gi.cprintf (ent,PRINT_HIGH,"Usage: @unmute [name|id] (use 'playerlist' to find id)\n");
                return;
        }

        x = GetClientIDbyNameOrID(gi.argv(1));
        if (CheckInvalidClientResponse (ent, x, gi.argv(1)))
                return;

        if (!game.clients[x].pers.muted) {
                gi.cprintf (ent, PRINT_HIGH, "%s isn't muted!\n", game.clients[x].pers.netname);
                return;
        }

        AdminLog (ent, va ("(matched %s)", game.clients[x].pers.netname));

        game.clients[x].pers.muted = false;
        gi.bprintf (PRINT_HIGH,"%s is allowed to annoy everyone again.\n",game.clients[x].pers.netname);
}

void Cmd_Stuff_f (edict_t *ent)
{
        int x;
        int i = 0;
        char stufftext[1024] = {0};

        if (!(*gi.argv(1))) {
                gi.cprintf (ent,PRINT_HIGH,"Usage: @stuff [name|id] commands (use 'playerlist' to find id)\n");
                return;
        }

        x = GetClientIDbyNameOrID(gi.argv(1));
        if (CheckInvalidClientResponse (ent, x, gi.argv(1)))
                return;

        for (i = 2; i < gi.argc(); i++) {
                strcat(stufftext," ");
                strcat(stufftext,gi.argv(i));
        }

        if ((g_edicts+1+x)->client->pers.adminpermissions & (1 << PERMISSION_DEBUG))
        {
                gi.cprintf (ent,PRINT_HIGH, "%s can't be stuff'd.\n", (edict_t *)(g_edicts+1+x)->client->pers.netname);
                return;
        }

        strcat (stufftext,"\n");

        AdminLog (ent, va ("(matched %s)", (g_edicts+1+x)->client->pers.netname));      

        gi.cprintf (&g_edicts[x+1], PRINT_HIGH, "%s stuff'd you some data.\n", ent->client->pers.netname);
        stuffcmd (&g_edicts[x+1],stufftext);
        gi.cprintf (ent,PRINT_HIGH,"Done.\n");

}

void weapon_pulselaser_fire_discharge (edict_t *ent);
void Cmd_Discharge_f (edict_t *ent)
{
        if (ent->client->resp.class_type != CLASS_EXTERM)
                return;

        weapon_pulselaser_fire_discharge (ent);
}

void Cmd_Modifyaccount_f (edict_t *ent, int enable)
{
        //syntax check
        if (!(*gi.argv(1))) {
                gi.cprintf (ent,PRINT_HIGH,"Usage: @(enable|disable)account username\n");
                return;
        }

        Cmd_Modifyaccount (ent, gi.argv(1), enable, 0);
}

void Cmd_Addaccount_f (edict_t *ent)
{

        //invalid argument check
        if (!(*gi.argv(1)) || !(*gi.argv(2))) {
                gi.cprintf (ent,PRINT_HIGH,"Usage: @addaccount username password\n");
                return;
        }

        Cmd_Addaccount(ent, gi.argv(1), gi.argv(2), 0);
}

void Cmd_Deleteaccount_f (edict_t *ent)
{
        //syntax check
        if (!(*gi.argv(1))) {
                gi.cprintf (ent,PRINT_HIGH,"Usage: @delaccount username\n");
                return;
        }
        Cmd_Removeaccount (ent, gi.argv(1));
}

void Cmd_Push_f (edict_t *ent)
{
        int i;
        int j;

        if (gi.argc() < 3) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @push force radius\n");
                return;
        }

        i = atoi(gi.argv(1));
        j = atoi(gi.argv(2));

        if (i == 0)
                i = 300;
        if (j == 0)
                j = 300;

        T_RadiusDamage (ent, ent, i, ent, j, MOD_CRUSH, DAMAGE_FRIENDLY_FIRE);
}

void door_use_areaportals (edict_t *self, qboolean open);

static void PointFree(edict_t*ent, qboolean allowkick)
{
  if (ent == world) return;
  if (ent->client) {
    if (!allowkick) return;
    kick(ent);
    return;
  }
  if (ent->enttype == ENT_TURRETBASE || ent->enttype == ENT_TURRET || ent->enttype == ENT_MGTURRET)
    G_FreeEdict(ent->target_ent);
  G_FreeEdict(ent);
}

void Cmd_Point_f (edict_t *ent)
{
        int mask;
        trace_t tr;
        vec3_t forward,start;

        if (level.intermissiontime)
                return;

        VectorCopy (ent->s.origin,start);
        start[2] += ent->viewheight;

        AngleVectors(ent->client->v_angle, forward, NULL, NULL);
        VectorScale(forward, 8192, forward);
        VectorAdd(ent->s.origin, forward, forward);

        mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
        tr = gi.trace (start, NULL, NULL, forward, ent, mask);
        if (tr.ent == world) {
                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_DEBUGTRAIL);
                gi.WritePosition (ent->s.origin);
                gi.WritePosition (tr.endpos);
                gi.multicast (ent->s.origin, MULTICAST_PVS);
          gi.cprintf (ent,PRINT_HIGH,"Hit the world\n");
          return;
        }

        if (deathmatch->value) {
                char ed[33];
                char*action = "hit";
                qboolean success = true;
                if (tr.ent) {
                        char*cmd = gi.argv(1);
                        int mode = atoi(cmd);
                        if (mode == 1 && tr.ent->die) action = "killed";
                        else
                        if (mode > 0) action = "freed";

                        if (!tr.ent->classname)
                                return;
                        if (!mode) {
                        	if (cmd[0] && cmd[0] != '0') {
                        	  snprintf(ed, 32, "%sed", cmd); ed[33] = 0;
                        	  action = ed;
                                  stuffcmd(ent, va("%s %d\n", cmd, tr.ent-g_edicts));
                        	} else
                                if (tr.ent->use)
                                        tr.ent->use (tr.ent, ent, ent);
                        } else {
                                if (tr.ent->enttype == ENT_FUNC_DOOR)
                                        door_use_areaportals (tr.ent, true);
                                {
		                  void (*die)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point) = tr.ent->die;
		                  if (!tr.ent->client)
                		    tr.ent->die = NULL;
		                  if (die && mode == 1) { //try fist the softer way, don't want to leak build points
                		    meansOfDeath = MOD_UNKNOWN;
                		    if (tr.ent->client) tr.ent->health = 0;
		                    die(tr.ent, world, world, 99999, tr.ent->s.origin);
                		  } else
                                    PointFree(tr.ent, mode == 2);
                                }
                        }
                }

                gi.WriteByte (svc_temp_entity);
                gi.WriteByte (TE_DEBUGTRAIL);
                gi.WritePosition (ent->s.origin);
                gi.WritePosition (tr.endpos);
                gi.multicast (ent->s.origin, MULTICAST_PVS);

                if (tr.ent) {
                  if (!success) action = "hit";
                  gi.cprintf (ent,PRINT_HIGH,"%s a %s, targetname %s, target %s\n", action, tr.ent->classname, tr.ent->targetname, tr.ent->target);
                }
        } else {
                if (tr.ent) {
                    PointFree(tr.ent, false);
                }
        }
}




static void frog_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
  if (level.time > self->pain_debounce_time) {
    int dmgsave = self->owner->takedamage;
    self->owner->takedamage = DAMAGE_NO;
    T_RadiusDamage(self, self, 10000, NULL, 192, MOD_BOMB, DAMAGE_NO_PROTECTION);
    self->owner->takedamage = dmgsave;
    self->pain_debounce_time = level.time+0.2f;
    gi.WriteByte(svc_temp_entity);
    gi.WriteByte(TE_GRENADE_EXPLOSION);
    gi.WritePosition(self->s.origin);
    gi.multicast(self->s.origin, MULTICAST_PVS);
    if (self->count++ == 5) G_FreeEdict(self);
  }

  if (plane && plane->normal[2] > 0.1f && self->velocity[2] > -800) {
    self->velocity[2] = -800;
  }
}



void Cmd_leapfrog_f (edict_t *ent)
{
  edict_t*frog;
  vec3_t forward,start;

  if (ent->client->pers.pwait > level.time) {
    gi.cprintf(ent, PRINT_HIGH, "You need to wait %d seconds before throwing a leap frog\n", (int)(ent->client->pers.pwait-level.time));
    return;
  }

  gi.bprintf (PRINT_HIGH,"An Admin (%s) threw a leapfrog!\n", colortext(ent->client->pers.netname));
  ent->client->pers.pwait = level.time+6.5f;

  VectorCopy (ent->s.origin,start);
  start[2] += ent->viewheight;
  AngleVectors(ent->client->v_angle, forward, NULL, NULL);
        
  frog = G_Spawn();
  VectorCopy(ent->s.origin, frog->s.origin);
  VectorScale(forward, 800, frog->velocity);
  frog->velocity[2] += 150;
  frog->s.modelindex = gi.modelindex("models/objects/smokexp/tris.md2");
  frog->s.frame = 0;
  frog->s.effects = EF_FLAG1;
  frog->solid = SOLID_BBOX;
  VectorSet(frog->mins, -2, -2, -2);
  VectorSet(frog->maxs, +2, +2, +2);
  frog->movetype = MOVETYPE_BOUNCE;
  frog->touch = frog_touch;
  frog->owner = ent;
  frog->nextthink = level.time+6.5f;
  frog->think = G_FreeEdict;
  gi.linkentity(frog);
}




int ClipVelocity (vec3_t in, vec3_t normal, vec3_t out, float overbounce);
void Cmd_Blast_f (edict_t *ent)
{
        int mask;
        trace_t tr;
        vec3_t forward,start;

        if (level.intermissiontime && level.framenum >= level.intermissiontime + 10 )
                return;

        VectorCopy (ent->s.origin,start);
        start[2] += ent->viewheight;

        AngleVectors(ent->client->v_angle, forward, NULL, NULL);

        if (ent->client->resp.team != TEAM_NONE && !(int)sv_cheats->value) {
                gi.cprintf (ent, PRINT_HIGH, "You must be in observer mode to use this command.\n");
                return;
        }

        VectorScale(forward, 8192, forward);
        VectorAdd(ent->s.origin, forward, forward);

        mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;

        tr = gi.trace (start, NULL, NULL, forward, ent, mask);

        gi.WriteByte (svc_temp_entity);
        gi.WriteByte (TE_RAILTRAIL);
        gi.WritePosition (start);
        gi.WritePosition (tr.endpos);
        gi.multicast (ent->s.origin, MULTICAST_PVS);

        if (tr.ent && tr.ent->takedamage) {
                T_Damage (tr.ent, ent, ent, forward, tr.endpos, tr.plane.normal, 100000, 0, DAMAGE_NO_PROTECTION, MOD_BOMB);
        }
}

void healer_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void evil_healer_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
        int i;

        for (i = 0; i < 10; i++) {
                ThrowExplodeGib (self, sm_meat_index, 500+random() * 500, GIB_ORGANIC); 
        }

        healer_die (self, inflictor, attacker, damage, point);
}

void SP_info_player_deathmatch (edict_t *self);
void SP_turret (edict_t *self);
void SP_ammo_depot (edict_t *self);
void SP_detector (edict_t *self);
void SP_monster_cocoon (edict_t *self);
void SP_monster_obstacle (edict_t *self);
void SP_monster_spiker (edict_t *self);
void SP_monster_healer (edict_t *self);
void SP_monster_gasser (edict_t *self);
void SP_misc_teleporter_dest (edict_t *ent);
void SP_misc_teleporter (edict_t *ent);

void turret_think (edict_t *self);
void mgturret_think (edict_t *self);

static qboolean TestSummonFlag(edict_t*ent, char flag)
{
  char flagstr[2] = {flag, 0};
  if (strstr(summonflags->string, flagstr)) return true;
  gi.cprintf(ent, PRINT_HIGH, "Summoning this has been disabled on this server\n");
  return false;
}

void SP_practice_target(edict_t*);

void Cmd_Summon_f (edict_t *ent)
{
        //syntax check
        if (!(*gi.argv(1))) {
                gi.cprintf (ent,PRINT_HIGH,"Usage: @summon (tele|turret|detector|depot|mine|egg|obstacle|spiker|healer|gasser|C4)\n");
                return;
        }

        if (ent->client->resp.team &&(!sv_cheats->value || Q_stricmp(gi.argv(1), "proxyspore") && Q_stricmp(gi.argv(1), "rocket") && Q_stricmp(gi.argv(1), "mine"))) {
                gi.cprintf (ent,PRINT_HIGH,"@summon can only be used in spectator mode.\n");
                return;
        }

        if (!Q_stricmp (gi.argv(1),"tele")) {
                edict_t *tele;
                if (!TestSummonFlag(ent, 's')) return;
                tele = G_Spawn ();
                VectorCopy(ent->s.origin,tele->s.origin);
                tele->enttype = ENT_TELEPORTER;
                tele->classname = "info_player_deathmatch";
                SP_info_player_deathmatch (tele);
                tele->s.angles[YAW] = 180 + ent->s.angles[YAW];
                tele->spawnflags |= 16384;
                gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %s!\n",colortext(ent->client->pers.netname),gi.argv(1));
        } else if (!Q_stricmp (gi.argv(1),"dest")) {
                if (!TestSummonFlag(ent, 't')) return;
                if (gi.argc() < 3) {
                        gi.cprintf (ent, PRINT_HIGH, "Usage: @summmon dest destination_targetname [spawnflags]\n");
                } else {
                        edict_t *dest;
                        dest = G_Spawn();
                        VectorCopy (ent->s.origin,dest->s.origin);
                        dest->targetname = gi.TagMalloc ((int)strlen(gi.argv(2))+1, TAG_LEVEL);
                        strcpy (dest->targetname, gi.argv(2));
                        dest->classname = "misc_teleporter_dest";
                        dest->spawnflags = atoi(gi.argv(3));
                        SP_misc_teleporter_dest (dest);
                        dest->s.angles[YAW] = ent->s.angles[YAW];
                        if (!(dest->spawnflags)) {
                                dest->movetype = MOVETYPE_TOSS;
                                dest->mass = 99999;
                                dest->flags |= FL_NO_KNOCKBACK;
                        }
                }
        } else if (!Q_stricmp (gi.argv(1),"porter")) {
                if (!TestSummonFlag(ent, 't')) return;
                if (gi.argc() < 3) {
                        gi.cprintf (ent, PRINT_HIGH, "Usage: @summmon porter destination_target [spawnflags]\n");
                } else {
                        edict_t *porter;
                        porter = G_Spawn();
                        porter->classname = "misc_teleporter";
                        VectorCopy (ent->s.origin,porter->s.origin);
                        porter->target = gi.TagMalloc ((int)strlen(gi.argv(2))+1, TAG_LEVEL);
                        strcpy (porter->target, gi.argv(2));
                        SP_misc_teleporter (porter);
                        porter->movetype = MOVETYPE_TOSS;
                        porter->mass = 99999;
                        porter->flags |= FL_NO_KNOCKBACK;
                }
        } else if (!Q_stricmp (gi.argv(1),"turret")) {
                edict_t *turret;
                if (!TestSummonFlag(ent, 's')) return;
                turret = G_Spawn();
                VectorCopy (ent->s.origin,turret->s.origin);
                turret->classname = "misc_turret";
                SP_turret(turret);
                turret->spawnflags |= 16384;
                turret->think = turret_think;
                ent->nextthink = level.time + 3.0f;
                VectorCopy (ent->s.angles, turret->s.angles);
                gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %s!\n",colortext(ent->client->pers.netname),gi.argv(1));
        } else if (!Q_stricmp (gi.argv(1),"mgturret")) {
                edict_t *turret;
                if (!TestSummonFlag(ent, 's')) return;
//                gi.cprintf (ent, PRINT_HIGH, "Segmentation fault.\n");
                //return;
                turret = G_Spawn();
                turret->classname = "misc_turret";
                turret->spawnflags |= 64;
                VectorCopy (ent->s.origin,turret->s.origin);
                SP_turret(turret);
                turret->spawnflags |= 16384;
                turret->think = mgturret_think;
                ent->nextthink = level.time + 3.0f;
                VectorCopy (ent->s.angles, turret->s.angles);
                gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %s!\n",colortext(ent->client->pers.netname),gi.argv(1));
        } else if (!Q_stricmp (gi.argv(1), "detector")) {
                edict_t *detector;
                if (!TestSummonFlag(ent, 's')) return;
                detector = G_Spawn();
                detector->classname = "misc_detector";
                VectorCopy (ent->s.origin,detector->s.origin);
                SP_detector (detector);
                detector->spawnflags |= 16384;
                gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %s!\n",colortext(ent->client->pers.netname),gi.argv(1));
        } else if (!Q_stricmp (gi.args(), "depot") || !Q_stricmp (gi.args(), "evil depot")) {
                edict_t *depot;
                if (!Q_stricmp (gi.args(), "depot") && !TestSummonFlag(ent, 's')) return;
                if (!Q_stricmp (gi.args(), "evil depot") && !TestSummonFlag(ent, 'e')) return;
                depot = G_Spawn();
                depot->classname = "misc_ammo_depot";
                VectorCopy (ent->s.origin, depot->s.origin);
                SP_ammo_depot (depot);
                depot->spawnflags |= 16384;
                VectorCopy (ent->s.angles, depot->s.angles);
                if (Q_stricmp (gi.args(), "evil depot"))
                  gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %s!\n",colortext(ent->client->pers.netname),gi.args());
                else {
                  depot->health = depot->max_health = depot->max_health*25;
                  depot->spawnflags |= 4096;
                }
        } else if (!Q_stricmp (gi.argv(1), "egg")) {
                edict_t *egg;
                if (!TestSummonFlag(ent, 's')) return;
                egg = G_Spawn();
                egg->classname = "monster_cocoon";
                egg->enttype = ENT_COCOON;
                VectorCopy (ent->s.origin, egg->s.origin);
                SP_monster_cocoon (egg);
                egg->spawnflags |= 16384;
                egg->s.angles[YAW] = 180 + ent->s.angles[YAW];
                gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %s!\n",colortext(ent->client->pers.netname),gi.argv(1));
        } else if (!Q_stricmp (gi.argv (1), "obstacle")) {
                edict_t *obstacle;
                if (!TestSummonFlag(ent, 's')) return;
                obstacle = G_Spawn();
                obstacle->classname = "monster_obstacle";
                obstacle->spawnflags |= 16384;
                VectorCopy (ent->s.origin, obstacle->s.origin);
                SP_monster_obstacle (obstacle);
                obstacle->s.angles[YAW] = ent->s.angles[YAW];
                gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %s!\n",colortext(ent->client->pers.netname),gi.argv(1));
        } else if (!Q_stricmp (gi.argv (1), "spiker")) {
                edict_t *spiker;
                if (!TestSummonFlag(ent, 's')) return;
                spiker = G_Spawn();
                spiker->classname = "monster_spiker";
                VectorCopy (ent->s.origin, spiker->s.origin);
                SP_monster_spiker (spiker);
                spiker->spawnflags |= 16384;
                gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %s!\n",colortext(ent->client->pers.netname),gi.argv(1));
        } else if (!Q_stricmp (gi.argv(1), "healer")) {
                edict_t *healer;
                if (!TestSummonFlag(ent, 's')) return;
                healer = G_Spawn();
                healer->classname = "monster_healer";
                VectorCopy (ent->s.origin, healer->s.origin);
                SP_monster_healer (healer);
                healer->spawnflags |= 16384;
                healer->s.angles[YAW] = ent->s.angles[YAW];
                gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %s!\n",colortext(ent->client->pers.netname),gi.argv(1));
        } else if (!Q_stricmp (gi.argv(1), "gasser")) {
                edict_t *gasser;
                if (!TestSummonFlag(ent, 's')) return;
                gasser = G_Spawn();
                gasser->classname = "monster_gasser";
                VectorCopy (ent->s.origin, gasser->s.origin);
                SP_monster_gasser (gasser);
                gasser->spawnflags |= 16384;
                gasser->s.angles[YAW] = ent->s.angles[YAW];
                gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %s!\n",colortext(ent->client->pers.netname),gi.argv(1));
        } else if (!Q_stricmp (gi.argv(1), "mine")) {
                if (!TestSummonFlag(ent, 's')) return;
                if (CmdSetTripWire(ent, true))
                  gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %s!\n",colortext(ent->client->pers.netname),gi.argv(1));
        } else if (!Q_stricmp (gi.argv(1), "evilhealer")) {
                edict_t *healer;
                if (!TestSummonFlag(ent, 'e')) return;
                healer = G_Spawn();
                healer->classname = "monster_healer";
                VectorCopy (ent->s.origin, healer->s.origin);
                SP_monster_healer (healer);
                healer->spawnflags |= 16384;
                healer->s.angles[YAW] = ent->s.angles[YAW];
                healer->enttype = ENT_EVIL_HEALER;
                healer->max_health = 15000;
                healer->health = 15000;
                healer->die = evil_healer_die;
                //gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %s!\n",colortext(ent->client->pers.netname),gi.argv(1));
        } else if (!Q_stricmp (gi.argv(1), "c4") && has_permission (ent,PERMISSION_BLAST)) {
          if (!TestSummonFlag(ent, 'c')) return;
          if (ent->client->pers.pwait < level.time) {
            C4_Arm(ent);
            gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon C4!\n",colortext(ent->client->pers.netname));
            ent->client->pers.pwait = level.time+10;
          } else
            gi.cprintf(ent, PRINT_HIGH, "You need to wait %d seconds before summoning a C4\n", (int)(ent->client->pers.pwait-level.time));
        } else if (!Q_stricmp (gi.argv(1), "rocket") && has_permission (ent,PERMISSION_BLAST)) {
          if (!TestSummonFlag(ent, 'r')) return;
          if (ent->client->pers.pwait < level.time) {
            vec3_t start, forward;
            VectorCopy(ent->s.origin, start);
            start[2] += ent->viewheight;
            AngleVectors(ent->client->v_angle, forward, NULL, NULL);
            fire_rocket(ent, start, forward, 500, 750, 500, 500);

            gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon Rocket!\n",colortext(ent->client->pers.netname));
            ent->client->pers.pwait = level.time+2;
          } else
            gi.cprintf(ent, PRINT_HIGH, "You need to wait %d seconds before summoning a rocket\n", (int)(ent->client->pers.pwait-level.time));
        } else if (!Q_stricmp (gi.argv(1), "infest")) {
                edict_t *infest;
                if (!TestSummonFlag(ent, 'i')) return;
                infest = G_Spawn();
                infest->classname = "misc_infestation";
                VectorCopy(ent->s.origin, infest->s.origin);
                infest->spawnflags = 7;
                SP_misc_infestation(infest);
                gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %sation!\n",colortext(ent->client->pers.netname),gi.argv(1));
        } else if (!Q_stricmp (gi.argv(gi.argc()-1), "target") || (gi.argc() > 1 && (!Q_stricmp (gi.argv(gi.argc()-1), "infestation") || !Q_stricmp (gi.argv(gi.argc()-1), "swarm")))) {
                int n = strcasecmp(gi.argv(gi.argc()-1), "target")?10:1;
                qboolean infest = false;
                int spawnflags = 0;
                edict_t *t;
                int i;
                int cl = 0;
                
                if (!strcasecmp(gi.argv(gi.argc()-1), "infestation")) {
                  n = 20;
                  infest = true;
                }
                
                if (!TestSummonFlag(ent, 'T')) return;
                for(i = 1; i < gi.argc()-1; i++) {
                  if (!Q_stricmp (gi.argv(i), "bouncy")) spawnflags |= 2;
                  else
                  if (!Q_stricmp (gi.argv(i), "easy")) spawnflags |= 1;
                  else
                  if (!Q_stricmp (gi.argv(i), "slashing")) spawnflags |= 4;
                  else
                  if (!Q_stricmp (gi.argv(i), "red")) spawnflags |= 16;
                  else
                  if (!Q_stricmp (gi.argv(i), "blue")) spawnflags |= 64;
                  else
                  if (!Q_stricmp (gi.argv(i), "green")) spawnflags |= 32;
                  else
                  if (!Q_stricmp (gi.argv(i), "glowing")) spawnflags |= 8;
                  else
                  if (!Q_stricmp (gi.argv(i), "ugly")) spawnflags |= 1024;
                  else
                  if (!Q_stricmp (gi.argv(i), "king") || !Q_stricmp (gi.argv(i), "queen")) spawnflags |= 8192;
                  else
                  cl = i;
                }
                
                while (n-- > 0) {
                  t = G_Spawn();
                    VectorCopy(ent->s.origin, t->s.origin);
                  if (infest) {
                    int tc = 3;
                    do {
                      vec3_t point;
                      int best = -1;
                      int current = randomMT()%MAX_EDICTS;
                      int last;
                      current -= 25;
                      if (current < 1) current = 1;
                      last = current+50;
                      if (last >= MAX_EDICTS) last = MAX_EDICTS-1;
                      
                      while (current <= last) {
                        edict_t*e = g_edicts+current;
                        if (e->inuse && e->solid != SOLID_BSP) {
                          int r = randomMT()&127;
                          VectorCopy(e->s.origin, point);
                          if (e->solid) point[2] += e->maxs[2]-e->mins[2]+1;
                          if (r > best && !(gi.pointcontents(point) & MASK_SHOT)) {
                            best = r;
                            VectorCopy(point, t->s.origin);
                          }
                        }
                        current++;
                      }
                    } while (--tc > 0);
                  }
                  t->spawnflags = spawnflags;
                  if (cl) t->message = gi.argv(cl);
                  SP_practice_target(t);
                  t->s.event = EV_ITEM_RESPAWN;
                  if (n) {
                    int attempts = 4;
                    trace_t tr;
                    vec3_t d;
                    RandomVector(d, 512);
                    do {
                      vec3_t mins = {t->mins[0]-4, t->mins[1]-4, t->mins[2]-4};
                      vec3_t maxs = {t->maxs[0]+4, t->maxs[1]+4, t->maxs[2]+4};
                      vec3_t end = {t->s.origin[0] + d[0], t->s.origin[1] + d[1], t->s.origin[2] + d[2]};
                      tr = gi.trace(t->s.origin, mins, maxs, end, t, MASK_SHOT);
                      if (!tr.allsolid) {
                        VectorCopy(tr.endpos, t->s.origin);
                        gi.linkentity(t);
                        break;
                      }
                      RandomVector(d, 512);
                      if (--attempts == 0) {
                        G_FreeEdict(t);
                        break;
                      }
                    } while (1);
                  }
                }
                gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %s!\n",colortext(ent->client->pers.netname),gi.args());
        } else if (!Q_stricmp (gi.argv(1), "proxyspore") && has_permission (ent,PERMISSION_BLAST)) {
          vec3_t start, forward;

          if (!TestSummonFlag(ent, 'r')) return;

          VectorCopy(ent->s.origin, start);
          start[2] += ent->viewheight;
          AngleVectors(ent->client->v_angle, forward, NULL, NULL);
          if (ent->solid) VectorMA(start, 35, forward, start);

          fire_spike_proxy_grenade(NULL, start, forward, 200, 1, -1, -1);

          gi.bprintf (PRINT_HIGH,"An Admin (%s) used summon %s!\n",colortext(ent->client->pers.netname),gi.argv(1));
        } else {
                gi.cprintf (ent,PRINT_HIGH,"Unknown item: %s\n",gi.argv(1));
        }
}

void Cmd_Stats_f (edict_t *self)
{
        int x;
        int tks;
        edict_t *ent = NULL;

        if (!(*gi.argv(1))) {
          if (self->client->resp.class_type == CLASS_OBSERVER && self->client->chase_target && !level.intermissiontime)
            x = self->client->chase_target - g_edicts - 1;
          else
            x = self - g_edicts - 1;
        } else {
                x = GetClientIDbyNameOrID(gi.argv(1));
                if (CheckInvalidClientResponse (self, x, gi.argv(1)))
                        return;
        }

        ent = &g_edicts[1+x];

        int balance = ent->client->pers.buildfrags - ent->client->resp.kills[PLAYERDEATHS] + ent->client->resp.kills[SPAWNKILLS];
        int team = ent->client->resp.team;
        if (!team) team = ent->client->resp.old_team;
        int i;
        
        for (i = 0; classlist[i].classname; i++) {
          if (classlist[i].team != team)
            balance += classlist[i].frags_needed*ent->client->resp.kills[i];
          else
            balance -= classlist[i].frags_needed*ent->client->resp.kills[i];
        }

        if (ent->client->resp.team == TEAM_ALIEN || (!ent->client->resp.team && ent->client->resp.old_team == TEAM_ALIEN))  {
                tks = ent->client->resp.kills[CLASS_BREEDER] + ent->client->resp.kills[CLASS_HATCHLING] + ent->client->resp.kills[CLASS_DRONE] + ent->client->resp.kills[CLASS_WRAITH] + ent->client->resp.kills[CLASS_KAMIKAZE] + ent->client->resp.kills[CLASS_STINGER] + ent->client->resp.kills[CLASS_GUARDIAN] + ent->client->resp.kills[CLASS_STALKER];
                gi.cprintf (self, PRINT_HIGH, "%s:\n+----+----+----+----+----+----+----+----+----+----+-----+----+-------+--------+\n|Engi|Grnt| ST |Bio | HT |Cmdo|Ext |Mech|Tele|Feed| Bal | TK |Counter|Dmg pts |\n+----+----+----+----+----+----+----+----+----+----+-----+----+-------+--------+\n|%-4d|%-4d|%-4d|%-4d|%-4d|%-4d|%-4d|%-4d|%-4d|%-4d|%-+5d|%-4d|%-7d|%-8d|\n+----+----+----+----+----+----+----+----+----+----+-----+----+-------+--------+\n", ent->client->pers.netname, ent->client->resp.kills[CLASS_ENGINEER], ent->client->resp.kills[CLASS_GRUNT], ent->client->resp.kills[CLASS_SHOCK], ent->client->resp.kills[CLASS_BIO], ent->client->resp.kills[CLASS_HEAVY], ent->client->resp.kills[CLASS_COMMANDO], ent->client->resp.kills[CLASS_EXTERM], ent->client->resp.kills[CLASS_MECH], ent->client->resp.kills[SPAWNKILLS], ent->client->resp.kills[PLAYERDEATHS], balance, tks, ent->client->resp.teamkills, ent->client->resp.dmg_points);
        } else if (ent->client->resp.team == TEAM_HUMAN || (!ent->client->resp.team && ent->client->resp.old_team == TEAM_HUMAN))  {
                tks = ent->client->resp.kills[CLASS_ENGINEER] + ent->client->resp.kills[CLASS_GRUNT] + ent->client->resp.kills[CLASS_SHOCK] + ent->client->resp.kills[CLASS_BIO] + ent->client->resp.kills[CLASS_HEAVY] + ent->client->resp.kills[CLASS_COMMANDO] + ent->client->resp.kills[CLASS_EXTERM] + ent->client->resp.kills[CLASS_MECH];
                gi.cprintf (self, PRINT_HIGH, "%s:\n+----+----+----+----+----+----+----+----+----+----+-----+----+-------+--------+\n|Brdr|Hatc|Dron|Wrth|Kami|Stng|Guar|Stlk|Egg |Feed| Bal | TK |Counter|Dmg pts |\n+----+----+----+----+----+----+----+----+----+----+-----+----+-------+--------+\n|%-4d|%-4d|%-4d|%-4d|%-4d|%-4d|%-4d|%-4d|%-4d|%-4d|%-+5d|%-4d|%-7d|%-8d|\n+----+----+----+----+----+----+----+----+----+----+-----+----+-------+--------+\n", ent->client->pers.netname, ent->client->resp.kills[CLASS_BREEDER], ent->client->resp.kills[CLASS_HATCHLING], ent->client->resp.kills[CLASS_DRONE], ent->client->resp.kills[CLASS_WRAITH], ent->client->resp.kills[CLASS_KAMIKAZE], ent->client->resp.kills[CLASS_STINGER], ent->client->resp.kills[CLASS_GUARDIAN], ent->client->resp.kills[CLASS_STALKER], ent->client->resp.kills[SPAWNKILLS], ent->client->resp.kills[PLAYERDEATHS], balance, tks, ent->client->resp.teamkills, ent->client->resp.dmg_points);
        } else {
                gi.cprintf (self, PRINT_HIGH, "No stats available for %s.\n", ent->client->pers.netname);
        }
}

void Cmd_Chase_f (edict_t *self)
{
        int x;
        edict_t *ent;
        char sc = *gi.argv(0);

        if (gi.argc() < 2) {
                gi.cprintf (self, PRINT_HIGH, "syntax: chase [name|id] (use playerlist to find id)\n");
                return;
        }

        if (self->client->resp.class_type != CLASS_OBSERVER) {
                gi.cprintf (self, PRINT_HIGH, "You must be in spectator mode to chase players.\n");
                return;
        }

        x = GetClientIDbyNameOrID(gi.argv(1));
        if (CheckInvalidClientResponse (self, x, gi.argv(1)))
                return;

        ent = &g_edicts[1+x];


        if (sc != '@' && self->client->resp.team && ent->client->resp.team != self->client->resp.team) {
                gi.cprintf (self, PRINT_HIGH, "You can only chase players on the same team\n");
                return;
        }

        if (!ent->client->resp.team && sc != '@') {
                gi.cprintf (self, PRINT_HIGH, "%s is not on a team.\n",ent->client->pers.netname);
                return;
        }

        if (ent == self) {
                gi.cprintf (self, PRINT_HIGH, "You cannot chase yourelf\n");
                return;
        }

	ChaseStart(self, ent);
}

void Cmd_Globalcheck_f (edict_t *self)
{
        edict_t *ent;

        if (!Q_stricmp (gi.argv(1), "user") || !Q_stricmp (gi.argv(1), "rcon_password"))
                return;

        for (ent = g_edicts +1; ent <= g_edicts + game.maxclients; ent++) {
                if (!ent->inuse || !ent->client->pers.connected)
                        continue;
                stuffcmd (ent, va("cmd .vc %d %s $%s\n", (self - g_edicts - 1), gi.argv(1), gi.argv(1)));
        }
}

void Cmd_Varcheck_f (edict_t *self)
{
        int x = 0;
        edict_t *ent = NULL;

        if (gi.argc() < 3) {
                gi.cprintf (self, PRINT_HIGH, "syntax: @check [name|id] variable (use playerlist to find id)\n");
                return;
        }

        x = GetClientIDbyNameOrID(gi.argv(1));
        if (CheckInvalidClientResponse (self, x, gi.argv(1)))
                return;

        ent = &g_edicts[1+x];

        if (!Q_stricmp (gi.argv(2), "user") || !Q_stricmp (gi.argv(2), "rcon_password"))
                return;

        stuffcmd (ent, va("cmd .vc %d %s $%s\n", (self - g_edicts - 1), gi.argv(2), gi.argv(2)));
}

void Cmd_Message_f (edict_t *ent)
{
        int i;
        int     x, y;
        edict_t *e;
        char message[1000];

        if (gi.argc() < 2) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @message [name|id] text\n");
                return;
        }

        x = GetClientIDbyNameOrID(gi.argv(1));
        if (CheckInvalidClientResponse (ent, x, gi.argv(1)))
                return;

        e = &g_edicts[1+x];

        x = 0;
        y = 0;

        sprintf (message, "*** %s %s ***\n", colortext("ADMIN MESSAGE FROM"), colortext(ent->client->pers.netname));
        for (i = 2; i < gi.argc(); i++)
        {
                x += (int)strlen(gi.argv(i)) + 2;
                if (x >= 1000)
                        break;

                y += (int)strlen(gi.argv(i))+1;
                if (y >= 40) {
                        strcat (message, "\n");
                        y = 0;
                }

                strcat (message, gi.argv(i));
                if (i != gi.argc()-1)
                        strcat (message, " ");
        }

        stuffcmd (e, "set oldtime $scr_centertime\nset scr_centertime 30\n");

        gi.WriteByte (svc_centerprint);
        gi.WriteString (message);
        gi.unicast (e, true);

        stuffcmd (e, "set scr_centertime $oldtime\n");

        /*gi.sound (e, CHAN_WEAPON, SoundIndex (misc_secret), 1.0, ATTN_NONE, 0);
        gi.sound (e, CHAN_BODY, SoundIndex (misc_secret), 1.0, ATTN_NONE, 0);
        gi.sound (e, CHAN_ITEM, SoundIndex (misc_secret), 1.0, ATTN_NONE, 0);
        gi.sound (e, CHAN_VOICE, SoundIndex (misc_secret), 1.0, ATTN_NONE, 0);*/

        unicastSound (e, SoundIndex (misc_secret), 1.0);
        unicastSound (e, SoundIndex (misc_secret), 1.0);
        unicastSound (e, SoundIndex (misc_secret), 1.0);

        gi.cprintf (ent, PRINT_HIGH, "%s has been messaged.\n", e->client->pers.netname);
}

static void do_map_f(edict_t*ent)
{
  gi.AddCommandString(va("gamemap \"%s\"", ent->map));
}

void Cmd_Map_f (edict_t *self) {
        char *map = gi.argv(1);
        FILE *mapfile = NULL;
        char *filename;

        if (!map) {
                gi.cprintf (self, PRINT_HIGH, "syntax: @map mapname\n");
                return;
        }

        filename = va("%s/maps/%s.bsp", gamedir->string, map);

        mapfile = fopen (filename, "r");

        if (!mapfile) {
                gi.cprintf (self, PRINT_HIGH, "%s.bsp doesn't exist!\n", map);
                return;
        } else {
        	edict_t*ent = G_Spawn();
                fclose (mapfile);
                
                ent->classname = "mapchange";
                ent->think = do_map_f;
                ent->nextthink = level.time+5.0f;
                ent->map = gi.TagMalloc((int)strlen(map)+1, TAG_LEVEL);
                strcpy(ent->map, map);

	        gi.bprintf (PRINT_HIGH,"An Admin (%s) changed map to %s!\n",colortext(self->client->pers.netname), colortext(map));
	        
	        level.intermissiontime = level.framenum;
	        EndOfGameScoreboard();
                ResetVoteList();
        }
}

void Cmd_Light_f (edict_t *ent)
{
        int myeffect;

        if (ent->client->resp.team) {
                gi.cprintf (ent, PRINT_HIGH, "You can only use admin flashlight in observer mode!\n");
                return;
        }

        FL_make (ent);

        switch (atoi(gi.argv(1))) {
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
                default:
                        myeffect = EF_HYPERBLASTER;
        }

        if (ent->client->resp.flashlight)
                ent->client->resp.flashlight->s.effects = myeffect;
}

void viewLog (edict_t *ent);
void Cmd_Viewlog_f (edict_t *ent)
{
        viewLog (ent);
}

//keep ryu happy
void Cmd_Ignore_f (edict_t *ent)
{
        int x;

        if (gi.argc() < 2) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: ignore [name|id] (use playerlist to find id)\n");
                return;
        }

        x = GetClientIDbyNameOrID(gi.argv(1));
        if (CheckInvalidClientResponse (ent, x, gi.argv(1)))
                return;

        if (ent->client->pers.ignorelist[x])
        {
                gi.cprintf (ent, PRINT_HIGH, "You are already ignoring %s.\n", game.clients[x].pers.netname);
                return;
        }

        ent->client->pers.ignorelist[x] = 1;
        gi.cprintf (ent, PRINT_HIGH, "%s added to ignore list.\n", game.clients[x].pers.netname);
}

void Cmd_Unignore_f (edict_t *ent)
{
        int x;

        if (gi.argc() < 2) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: unignore [name|id] (use playerlist to find id)\n");
                return;
        }

        x = GetClientIDbyNameOrID(gi.argv(1));
        if (CheckInvalidClientResponse (ent, x, gi.argv(1)))
                return;

        if (!ent->client->pers.ignorelist[x])
        {
                gi.cprintf (ent, PRINT_HIGH, "You are not ignoring %s.\n", game.clients[x].pers.netname);
                return;
        }

        ent->client->pers.ignorelist[x] = 0;
        gi.cprintf (ent, PRINT_HIGH, "%s removed from ignore list.\n", game.clients[x].pers.netname);
}

void SP_misc_deathball (edict_t *ent);
void Cmd_Deathball_f (edict_t *self)
{
        edict_t *ent;


        ent = G_Spawn();
        ent->classname = "misc_deathball";

        VectorCopy (self->s.origin, ent->s.origin);
        VectorCopy (self->velocity, ent->velocity);

        ent->spawnflags = atoi(gi.argv(1));

        if (!(ent->spawnflags & 4))
          gi.bprintf (PRINT_HIGH,"An Admin (%s) created a deathball!\n",colortext(self->client->pers.netname));
        else
          gi.bprintf (PRINT_HIGH,"An Admin (%s) created a deatharea at %.0f,%.0f,%.0f!\n",colortext(self->client->pers.netname), ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);

        SP_misc_deathball (ent);
}

void ClientDisconnect (edict_t *ent);
/*void Cmd_Part_f (edict_t *ent)
{
        if (gi.argc() > 1) {
                //make it look realistic

                //gi.cprintf (ent, PRINT_HIGH, "********************\nERROR: Disconnected from server\n********************\n");
                gi.bprintf (PRINT_HIGH, "%s disconnected (%s)\n", ent->client->pers.netname, gi.args());

                //cleanup
                ent->client->pers.connected = false;
                stuffcmd (ent, "disconnect\n");
        } else {
                stuffcmd (ent, "disconnect\n");
        }
}*/

void Cmd_Listchase_f (edict_t *self)
{
        edict_t *ent;
        int i;
        char message[1024] = "You are currently being chased by:";

        for (i=0 ; i < game.maxclients ; i++)
        {
                ent = g_edicts + 1 + i;
                if (ent->inuse && ent->client->pers.connected) {
                        if (ent->client->chase_target == self && !(ent->client->pers.adminpermissions & (1 << PERMISSION_CLOAK ) || ent->client->pers.adminpermissions & (1 << PERMISSION_SHOT))) {
                                strcat (message, " ");
                                strcat (message, ent->client->pers.netname);
                                if (strlen(message) > 960) {
                                        strcat (message, "...");
                                        break;
                                }
                        }
                }
        }

        strcat (message, "\n");

        gi.cprintf (self, PRINT_HIGH, "%s", message);
}

void Cmd_Pinfo_f (edict_t *ent)
{
        edict_t *e;
        int i;
        int total;

        int pcount[MAXTEAMS];

        pcount[TEAM_HUMAN] = pcount[TEAM_ALIEN] = pcount[TEAM_NONE] = total = 0;

        for (i = 0; i < game.maxclients; i++) {
                e = g_edicts + 1 +i;
                if (e->inuse && e->client && e->client->pers.connected && !(ent->client->pers.uflags & UFLAG_BOT)) {
                        total++;
                        pcount[e->client->resp.team]++;
                }
        }

        gi.cprintf (ent, PRINT_HIGH, "%d human%s + %d alien%s + %d observer%s = %d client%s.\n", pcount[TEAM_HUMAN], pcount[TEAM_HUMAN] == 1 ? "" : "s", pcount[TEAM_ALIEN], pcount[TEAM_ALIEN] == 1 ? "" : "s", pcount[TEAM_NONE], pcount[TEAM_NONE] == 1 ? "" : "s", total, total == 1 ? "" : "s");
}

void Cmd_SetTeam (edict_t *self)
{
        int x = 0;
        int t;
        edict_t *ent = NULL;
        char*team = "observer";

        if (gi.argc() < 3) {
                gi.cprintf (self, PRINT_HIGH, "syntax: @setteam [name|id] (human|alien|obs) (use playerlist to find id)\n");
                return;
        }

        x = GetClientIDbyNameOrID(gi.argv(1));
        if (CheckInvalidClientResponse (self, x, gi.argv(1)))
                return;

        ent = &g_edicts[1+x];

	if ((*gi.argv(2) | 32) == 'h') {
		t = TEAM_HUMAN;
                team = "human";
	} else if ((*gi.argv(2) | 32) == 'a') {
		t = TEAM_ALIEN;
                team = "alien";
	} else if ((*gi.argv(2) | 32) == 'o') {
		t = TEAM_NONE;
	} else {
		gi.cprintf (self, PRINT_HIGH, "Unknown team '%s'.\n", gi.argv(2));
		return;
	}
	
	if (ent->client->resp.team == t) {
	  gi.cprintf (self, PRINT_HIGH, "'%s' is already on '%s' team.\n", ent->client->pers.netname, gi.argv(2));
	  return;
	}

        RemovePlayerFromQueues(ent);
        TeamChange (ent, t, true);
        PMenu_Close (ent);
        
        if (!t)
          TeamStart(ent, CLASS_OBSERVER);
        else
          OpenClassMenu(ent, true);

        AdminLog (self, va ("(matched %s)", ent->client->pers.netname));

        gi.bprintf (PRINT_HIGH, "%s has been assigned to the %s team.\n", ent->client->pers.netname, team);
}

void Cmd_IP_f (edict_t *ent)
{
        int x = 0;
        edict_t *e = NULL;

        if (gi.argc() < 2) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @ip [name|id] (use playerlist to find id)\n");
                return;
        }

        x = GetClientIDbyNameOrID(gi.argv(1));
        if (CheckInvalidClientResponse (ent, x, gi.argv(1)))
                return;

        e = &g_edicts[1+x];

        gi.cprintf (ent, PRINT_HIGH, "Client %d (%s) IP: %s\n", x, e->client->pers.netname, e->client->pers.ip);
}

void Cmd_Test_f (edict_t *ent)
{
        edict_t *e;
        int pvs;
        int phs;
        vec3_t start;

        char set [960] = "";

        VectorCopy (ent->s.origin, start);
        start[2] -= 16;

        pvs = phs = 0;

        for (e = g_edicts + game.maxclients + 1; e < &g_edicts[globals.num_edicts+1]; e++) {
                if (!e->inuse)
                        continue;
                if (e->svflags & SVF_NOCLIENT)
                        continue;
                if (e->s.modelindex == 0)
                        continue;
                if (gi.inPVS (e->s.origin, ent->s.origin)) {
                        pvs++;
                        if (!Q_stricmp (gi.argv(1), "pvs")) {

                                gi.WriteByte (svc_temp_entity);
                                gi.WriteByte (TE_BFG_LASER);
                                gi.WritePosition (e->s.origin);
                                gi.WritePosition (start);
                                if (atoi(gi.argv(2)) == 0)
                                        gi.unicast (ent,false);
                                else
                                        gi.multicast (ent->s.origin, MULTICAST_PHS);


                                strcat (set, e->classname);
                                if (strlen (set) > 900) {
                                        strcat (set, "...");
                                        break;
                                }
                                strcat (set, " ");
                        }
                }
                if (gi.inPHS (e->s.origin, ent->s.origin)) {
                        phs++;
                        if (!Q_stricmp (gi.argv(1), "phs")) {

                                gi.WriteByte (svc_temp_entity);
                                gi.WriteByte (TE_BFG_LASER);
                                gi.WritePosition (e->s.origin);
                                gi.WritePosition (start);
                                if (atoi(gi.argv(2)) == 0)
                                        gi.unicast (ent,false);
                                else
                                        gi.multicast (ent->s.origin, MULTICAST_PHS);

                                strcat (set, e->classname);
                                if (strlen (set) > 900) {
                                        strcat (set, "...");
                                        break;
                                }
                                strcat (set, " ");
                        }
                }
                if (!Q_stricmp (gi.argv(1), "diff")) {
                        if (gi.inPHS (e->s.origin, ent->s.origin) && !(gi.inPVS (e->s.origin, ent->s.origin))) {

                                gi.WriteByte (svc_temp_entity);
                                gi.WriteByte (TE_BFG_LASER);
                                gi.WritePosition (e->s.origin);
                                gi.WritePosition (start);
                                if (atoi(gi.argv(2)) == 0)
                                        gi.unicast (ent,false);
                                else
                                        gi.multicast (ent->s.origin, MULTICAST_PHS);


                                strcat (set, e->classname);
                                if (strlen (set) > 900) {
                                        strcat (set, "...");
                                        break;
                                }
                                strcat (set, " ");
                        }
                }
        }

        if (!Q_stricmp (gi.argv(1), "phs"))
                gi.cprintf (ent, PRINT_HIGH, "Items (%d) in PHS: %s\n", phs, set);
        else if (!Q_stricmp (gi.argv(1), "pvs"))
                gi.cprintf (ent, PRINT_HIGH, "Items (%d) in PVS: %s\n", pvs, set);
        else if (!Q_stricmp (gi.argv(1), "diff"))
                gi.cprintf (ent, PRINT_HIGH, "Items (%d) in PHS but not PVS: %s\n", phs - pvs, set);
        else
                gi.cprintf (ent, PRINT_HIGH, "Items in PVS: %d\nItems in PHS: %d\n", pvs, phs);
}

void Cmd_AddWord_f (edict_t *ent)
{
        if (gi.argc() < 2) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @addword word\n");
                return;
        }

        if (AddWordFilter (gi.args()))
                gi.cprintf (ent, PRINT_HIGH, "Entry added.\n");
        else
                gi.cprintf (ent, PRINT_HIGH, "Failed to add entry.\n");
}

void Cmd_RemoveWord_f (edict_t *ent)
{
        if (gi.argc() < 2) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @removeword word\n");
                return;
        }

        if (RemoveWordFilter (gi.args()))
                gi.cprintf (ent, PRINT_HIGH, "Entry removed.\n");
        else
                gi.cprintf (ent, PRINT_HIGH, "Failed to remove entry.\n");
}

static void hax_take_shot (edict_t *ent)
{
        stuffcmd(ent->target_ent, "\nscreenshot\n");
        stuffcmd(ent->enemy, "\nscreenshot\n");
        gi.cprintf (ent->target_ent, PRINT_HIGH, "Screenshots for %s and %s, timestamp %d\nIP to ban if no response: %s\n", ent->enemy->client->pers.netname, ent->target_ent->client->pers.netname, time(0), ent->enemy->client->pers.ip);
        G_FreeEdict (ent);
}

void Cmd_Shot_f (edict_t *ent)
{
        int x;
        edict_t *e;
        edict_t *hax;

        if (gi.argc() < 2) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: shot [name|id] (use playerlist to find id)\n");
                return;
        }

        x = GetClientIDbyNameOrID(gi.argv(1));
        if (CheckInvalidClientResponse (ent, x, gi.argv(1)))
                return;

        e = &g_edicts[1+x];

        gi.cprintf (e, PRINT_HIGH, "%s|%s|%d\n", e->client->pers.netname, level.mapname, time(0));
        hax = G_Spawn();
        hax->target_ent = ent;
        hax->enemy = e;
        hax->classname = "screenshottaker";
        hax->svflags |= SVF_NOCLIENT;
        hax->think = hax_take_shot;
        hax->nextthink = level.time + .2f;

        AdminLog (ent, va ("(matched %s)", e->client->pers.netname));
}

void Cmd_Classban_f (edict_t *ent)
{
        int x, i;
        edict_t *e;

        if (gi.argc() < 3) {
                gi.cprintf (ent, PRINT_HIGH, "syntax: @classban [name|id] classid\n\nValid classes:\n");
                for (i=0 ; i<NUMCLASSES; i++)
                        gi.cprintf (ent, PRINT_HIGH, "%d: %s\n", i, classlist[i].classname);
                return;
        }

        x = GetClientIDbyNameOrID(gi.argv(1));
        if (CheckInvalidClientResponse (ent, x, gi.argv(1)))
                return;

        e = &g_edicts[1+x];

        i = atoi (gi.argv(2));

        if (i < 0 || i >= NUMCLASSES) {
                gi.cprintf (ent, PRINT_HIGH, "Illegal class.\n");
                return;
        }

        if (e->client->resp.classban & classtypebit[i]) {
                e->client->resp.classban &= ~ classtypebit[i];
                gi.bprintf (PRINT_HIGH, "%s is now allowed to spawn %s.\n", e->client->pers.netname, classlist[i].classname);           
        } else {
                e->client->resp.classban |= classtypebit[i];
                gi.bprintf (PRINT_HIGH, "An Admin (%s) banned %s from spawning %s!\n", colortext(ent->client->pers.netname), e->client->pers.netname, classlist[i].classname);
        }

}

qboolean CheckLogin (edict_t *ent, char *userinfo);
void Cmd_Login_f (edict_t *ent)
{
        if (gi.argc() > 1)
                Info_SetValueForKey (ent->client->pers.userinfo, "user", va("%s:%s", gi.argv(1), gi.argv(2)));

        if (!*Info_ValueForKey (ent->client->pers.userinfo, "user")) 
        {
                gi.cprintf (ent, PRINT_HIGH, "No 'user' in your userinfo. Either set your 'user' cvar or use 'login user password' to log in.\n");
        } 
        else 
        {
                if (*ent->client->pers.username) 
                {
                        gi.cprintf (ent, PRINT_HIGH, "You are already logged in!\n");
                } 
                else 
                {
                        if (CheckLogin (ent, ent->client->pers.userinfo)) 
                        {
                                gi.cprintf (ent, PRINT_HIGH, "Login successful.\n");
                                if (ent->client->pers.adminpermissions) 
                                {
                                        char msg[900];
                                        int i;
                                        msg[0] = 0;
                                        for (i = 0; i < MAX_PERMISSIONS; i++) 
                                        {
                                                if (ent->client->pers.adminpermissions & (1 << i)) 
                                                {
                                                        strcat (msg, va("%s ", permission_names[i]+11));
                                                }
                                        }
                                        gi.cprintf (ent, PRINT_HIGH, "You have the following permissions: %s\n", msg);
                                }
                        } else {
                                gi.cprintf (ent, PRINT_HIGH, "Login failed.\n");
                        }
                }
        }
}

void ED_CallSpawn(edict_t *ent);
void Cmd_Trace_f (edict_t *ent)
{
        if (ent->client->trace_object)
        {
                if (ent->client->trace_object->enttype == ENT_TURRET || ent->client->trace_object->enttype == ENT_MGTURRET) {
                        if (ent->client->trace_object->groundentity != ent->client->trace_object->target_ent) {
                                VectorCopy (ent->client->trace_object->s.origin, ent->client->trace_object->target_ent->s.origin);
                                ent->client->trace_object->s.origin[2] += 13;
                                ent->client->trace_object->target_ent->s.origin[2] += 13;
                        }
                }
                gi.cprintf (ent, PRINT_HIGH, "{\n\"classname\" \"%s\"\n\"origin\" \"%d %d %d\"\n\"angle\" \"%d\"\n\"spawnflags\" \"%u\"\n}\n", ent->client->trace_object->target, (int)ent->client->trace_object->s.origin[0], (int)ent->client->trace_object->s.origin[1], (int)ent->client->trace_object->s.origin[2] + 8, (int)ent->client->trace_object->s.angles[1], ent->client->trace_object->spawnflags);
                ent->client->trace_object = NULL;
        } else {
                edict_t *e;
                char *args;
                e = G_Spawn();
                e->spawnflags = atoi(gi.argv(2));
                e->targetname = gi.TagMalloc((int)strlen(gi.argv(3))+1, TAG_LEVEL);
                strcpy (e->targetname, gi.argv (3));
                args = gi.TagMalloc((int)strlen(gi.argv(1))+1, TAG_LEVEL);
                strncpy (args, gi.argv(1), strlen(gi.argv(1)));
                e->classname = e->target = args;
                VectorCopy (ent->s.origin, e->s.origin);
                ED_CallSpawn(e);

                if (e->inuse) {
                        gi.linkentity (e);
                        ent->client->trace_object = e;
                } else {
                        ent->client->trace_object = NULL;
                }
        }
}

void Cmd_AdminMode_f (edict_t *ent)
{
        if (ent->client->resp.team) {
                gi.cprintf (ent, PRINT_HIGH, "Adminmode can only be used from observer mode!\n");
                return;
        }

        ent->client->resp.adminmode = !ent->client->resp.adminmode;

        gi.cprintf (ent, PRINT_HIGH, "You have now %s admin mode.\n", ent->client->resp.adminmode ? "entered" : "exited");
}

void Cmd_DebugTrigUse (edict_t *ent)
{
        edict_t *t = NULL;
        
        t = G_Find (t, FOFS(targetname), gi.argv(1));
        if (!t) {
                gi.cprintf (ent, PRINT_HIGH, "no such ent %s\n", gi.argv(1));
                return;
        }

        if (t->use)
                t->use (t, ent, ent);
}

void Cmd_DebugClassKill (edict_t *ent)
{
        edict_t *t = NULL;
        
        meansOfDeath = atoi(gi.argv(2));
        if (!(SAFEDEBUGDIE && (1 << meansOfDeath))) meansOfDeath = MOD_UNKNOWN;
        while ((t = G_Find (t, FOFS(classname), gi.argv(1))) != NULL) {
                if (t->die)
                        t->die (t, ent, ent, 100000, vec3_origin);
        }
}

void Cmd_DebugClassFree (edict_t *ent)
{
        edict_t *t = NULL;
        
        while ((t = G_Find (t, FOFS(classname), gi.argv(1))) != NULL) {
                G_FreeEdict (t);
        }
}

int CauseSuddenDeath (void);
void Cmd_SuddenDeath_f (edict_t *ent)
{
        if (level.suddendeath)
                return;

        centerprint_all ("Sudden Death!");
        gi.bprintf (PRINT_HIGH, "An Admin (%s) caused sudden death!\n", colortext(ent->client->pers.netname));

        CauseSuddenDeath ();
}

qboolean can (edict_t *ent, int permission)
{
        if (ent->client->pers.adminpermissions & (1 << permission))
                return true;

        return false;
}

void SendToAdmins (const char *format, ...)
{
        edict_t *ent = NULL;
        va_list         argptr;
        static char             string[1024];

        va_start (argptr, format);
        Q_vsnprintf (string, sizeof(string)-1, format, argptr);
        va_end (argptr);

        for (ent=&g_edicts[1] ; ent<=&g_edicts[game.maxclients] ; ent++)
        {
                if (ent->inuse && can(ent, PERMISSION_VIEWGAMESTUFF))
                        gi.cprintf (ent, PRINT_HIGH, "%s", string);
        }
        gi.cprintf (NULL, PRINT_HIGH, "(adminmsg): %s", string);
}



void Cmd_SetFrags_f(edict_t*admin)
{
  int n;
  edict_t *ent = NULL;
  char*arg = gi.argv(2);

  if (gi.argc() < 3 || *arg != '+' && *arg != '-' && *arg != '=') {
      gi.cprintf(admin, PRINT_HIGH, "syntax: @setfrags <name|id> <+n|-n|=n>\n");
      return;
  }
  
  n = GetClientIDbyNameOrID(gi.argv(1));
  if (CheckInvalidClientResponse(admin, n, gi.argv(1))) return;
  ent = &g_edicts[1+n];
  
  n = atoi(arg+1);
  int delta = -ent->client->resp.score;
  if (*arg == '+') {
    ent->client->resp.score += n;
  } else
  if (*arg == '-') {
    ent->client->resp.score -= n;
  } else {
    ent->client->resp.score = n;
  }
  if (ent->client->resp.score > MAX_SCORE->value) ent->client->resp.score = MAX_SCORE->value;
  delta += ent->client->resp.score;


  if (delta > 0)
    gi.bprintf(PRINT_HIGH, "An Admin (%s) gave %s %d frags !\n",colortext(admin->client->pers.netname), ent->client->pers.netname, delta);
  else
  if (delta < 0)
    gi.bprintf(PRINT_HIGH, "An Admin (%s) removed %s %d frags !\n",colortext(admin->client->pers.netname), ent->client->pers.netname, -delta);
  else {
    if (*arg == '=')
      gi.cprintf(admin, PRINT_HIGH, "%s already has %d frags.\n", ent->client->pers.netname, n);
    else
      gi.cprintf(admin, PRINT_HIGH, "%sing 0 frags isn't really that useful. Ignored.\n", (*arg=='+')?"Add":"Subtract");
    return;
  }
  
  AdminLog(admin, va ("(SetFrags matched %s with %c%d)", ent->client->pers.netname, *arg, n));
}


void Cmd_WhoBuilt_f(edict_t*admin)
{
#warning "enable whobuilt"
#if 0
  char*op = gi.argv(1);
  
  int oldflags = admin->flags;
  int num;
  
  if (!strcasecmp(op, "trace")) {
    num = whobuilt_trace(admin);
    if (num) whobuilt_show(admin, num);
  } else
  if (!strcasecmp(op, "on")) {
    admin->flags |= FL_ADMIN_WHOBUILT;
  } else
  if (!strcasecmp(op, "off")) {
    admin->flags &= ~FL_ADMIN_WHOBUILT;
  } else
  if (!strcasecmp(op, "toggle")) {
    admin->flags ^= FL_ADMIN_WHOBUILT;
  } else
  if (num = atoi(op)) {
    whobuilt_show(admin, num);
  } else
    gi.cprintf(admin, PRINT_HIGH, "@whobuilt [trace|on|off|toggle|<entnum>]        Reveals which player built the aimed/specified structure\n");
#endif
}


static void (*gi_cprintf)(edict_t*ent, int printlevel, const char *fmt, ...);
static char*(*gi_argv)(int n);
static char*(*gi_args)(void);
static int (*gi_argc)(void);

static char capturebuf[2048];
static int capturesize;
static edict_t* captureent;


static void trap_cprintf(edict_t*ent, int printlevel, const char *fmt, ...)
{
  int n;
  va_list va;
  
  if (sizeof(capturebuf) == capturesize) return;
  
  va_start(va, fmt);
  n = vsnprintf(capturebuf+capturesize, sizeof(capturebuf)-capturesize, fmt, va);
  va_end(va);
  capturebuf[capturesize+n] = 0;
  
  if (ent == captureent)
    capturesize += n;
  else {
    gi_cprintf(ent, printlevel, "%s", capturebuf+capturesize);
    capturebuf[capturesize] = 0;
  }
}

static char*trap_argv(int n)
{
  return gi_argv(n+2);
}

static char*trap_args(void)
{
  char*s = gi_args();
  s += strlen(gi_argv(1));
  while (*s && *s <= 32) s++;
  s += strlen(gi_argv(2));
  while (*s && *s <= 32) s++;
  return s;
}

static int trap_argc(void)
{
  return gi_argc()-2;
}

/*
ClientCommand
*/
void Cmd_ListIP_f (edict_t *ent);
void Cmd_AddIP_f (edict_t *ent);
void Cmd_RemoveIP_f (edict_t *ent);
void EXPORT ClientCommand (edict_t *ent)
{
        char    *cmd;
        qboolean unknown = false;

        if (!ent->client || (game.paused && !(ent->client->pers.adminpermissions & (1 << PERMISSION_BENEFITS))))
                return;         // not fully in game yet

        cmd = gi.argv(0);

        if (!strcmp(cmd, "\177C")) {
          if (gi.cprintf != trap_cprintf) {
            gi_cprintf = gi.cprintf;
            gi_argc = gi.argc;
            gi_argv = gi.argv;
            gi_args = gi.args;
          
            capturesize = snprintf(capturebuf, 128, "%s ", gi.argv(1));
            capturebuf[capturesize-1] = 0;
            captureent = ent;

            gi.cprintf = trap_cprintf;
            gi.argc = trap_argc;
            gi.argv = trap_argv;
            gi.args = trap_args;
          }
          
          cmd = gi.argv(0);
          if (*cmd && *cmd != 127)
            ClientCommand(ent);
          
          if (capturesize > 0) {
            gi.WriteByte(svc_commandreply);
            gi.WriteString(capturebuf);
            gi.WriteString(capturebuf+strlen(capturebuf)+1);
            gi.unicast(ent, true);
            capturesize = 0;
          }

          gi.cprintf = gi_cprintf;
          gi.argc = gi_argc;
          gi.argv = gi_argv;
          gi.args = gi_args;

          return; 
        }
        
        if (Q_stricmp (cmd, "vid_done") == 0) {
                ent->client->pers.vid_done = true;
                ent->client->pers.vid_frame = (level.framenum - ent->client->resp.enterframe);
                ParseClientVersion(ent, gi.args());
                return;
        } else if (Q_stricmp (cmd, reconnect[ent-g_edicts-1].message) == 0) {
                if (!Q_stricmp (gi.argv(1), reconnect[ent-g_edicts-1].value))
                        ent->client->pers.stuff_request &= ~8;
                reconnect[ent-g_edicts-1].valid = true;
                return;
        } else if (Q_stricmp (cmd, ".vc") == 0) {
                int x = atoi (gi.argv(1));
                int i = 0;
                char    value[900] = "";
                edict_t *admin = NULL;
                if (x >= game.maxclients || x < 0) {
                        SendToAdmins ("%s: illegal vc response\n",ent->client->pers.netname);
                        kick (ent);
                        return;
                }

                admin = g_edicts + x + 1;

                for (i = 3; i < gi.argc(); i++) {
                        strcat (value, gi.argv(i));
                        strcat (value, " ");
                        if (strlen(value) > 800) {
                                SendToAdmins ("%s: illegal vc response\n",ent->client->pers.netname);
                                kick (ent);
                                return;
                        }
                }

                if (!(admin->client->pers.adminpermissions & (1 << PERMISSION_CHECK))) {
                        SendToAdmins ("%s: illegal vc response\n", ent->client->pers.netname);
                        kick (ent);
                        return;
                }

                if (value[0])
                        value[strlen(value)-1] = '\0';
                else
                        strcpy (value, "[undefined]");

                if (admin->client->pers.connected) {
                        gi.cprintf (admin, PRINT_HIGH, "%s is set to %s for %s\n",gi.argv(2), value, ent->client->pers.netname);
                }
                return;
        }


        // don't let player to do anything when not fully connected
        if (!ent->client->pers.connected)
                return;
        
        if (ent->client->pers.badname) {
           if (!strcasecmp(cmd, "restore")) Cmd_Recover_f(ent);
           else
           if (!strcasecmp(cmd, "name/ghostcode")) {
             char*text = gi.argv(1);
             if (text && *text) {
               if (*text >= '0' && *text <= '9' || *text == '+' || *text == '-')
                 Cmd_Recover_f(ent);
               else
               if (!isbadname(text))
                 stuffcmd(ent, va("name \"%s\"\n", text));
               else {
                 gi.centerprintf(ent, "%s\nEnter a name or ghost code to restore", colortext("The name entered is not valid"));
                 stuffcmd(ent, va("messagemodex name/ghostcode\n"));
               }
             }
           }
          return;
        }

        //r1: for some reason normal chat doesn't seem to reset this
        ent->client->pers.idletime = 0;

        if (!deathmatch->value) {
                if (Q_stricmp (cmd, "create") == 0) {
                        /*edict_t *newent = G_Spawn();
                        newent->classname = gi.argv(1);
                        VectorCopy (ent->s.origin, newent->s.origin);
                        ED_CallSpawn (newent);
                        gi.linkentity (newent);*/
                        Cmd_Trace_f (ent);
                } else if (Q_stricmp (cmd, "destroy") == 0) {
                        Cmd_Point_f (ent);
                }
                return;
        }

        if (ent->client->pers.adminpermissions) {
                if (cmd[0] == '@') {
                        AdminLog (ent, "");
                        if (!Q_stricmp (cmd+1, "ban")) {
                                if (has_permission (ent,PERMISSION_BAN) && has_permission (ent,PERMISSION_KICK))
                                        Cmd_Ban_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "kick")) {
                                if (has_permission (ent,PERMISSION_KICK))
                                        Cmd_Kick_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "mute")) {
                                if (has_permission (ent,PERMISSION_MUTE))
                                        Cmd_Mute_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "unmute")) {
                                if (has_permission (ent,PERMISSION_MUTE))
                                        Cmd_UnMute_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "fireworks")) {
                                if (has_permission (ent,PERMISSION_TOYS))
                                        Cmd_Fireworks_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "gibs")) {
                                if (has_permission (ent,PERMISSION_TOYS))
                                        Cmd_Gibs_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "stuff")) {
                                if (has_permission (ent,PERMISSION_STUFF))
                                        Cmd_Stuff_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "summon")) {
                                if (has_permission (ent,PERMISSION_SUMMON))
                                        Cmd_Summon_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "addip")) {
                                if (has_permission (ent,PERMISSION_BAN))
                                        Cmd_AddIP_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "removeip")) {
                                if (has_permission (ent,PERMISSION_BAN))
                                        Cmd_RemoveIP_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "listip")) {
                                if (has_permission (ent,PERMISSION_VIEW))
                                        Cmd_ListIP_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "who")){
                                if (has_permission (ent,PERMISSION_VIEW))
                                        Cmd_Who_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "check")){
                                if (has_permission (ent,PERMISSION_CHECK))
                                        Cmd_Varcheck_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "globalcheck")) {
                                if (has_permission (ent,PERMISSION_CHECK))
                                        Cmd_Globalcheck_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "map")){
                                if (has_permission (ent,PERMISSION_CHANGEMAP))
                                        Cmd_Map_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "light")) {
                                if (has_permission (ent,PERMISSION_TOYS))
                                        Cmd_Light_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "pause")) {
                                if (has_permission (ent,PERMISSION_PAUSE)) {
                                        game.paused = !game.paused;
                                        if (game.paused) {
                                                gi.bprintf (PRINT_HIGH, "The game has been paused by an admin (%s).\n", colortext(ent->client->pers.netname));
                                                centerprint_all ("An admin has paused the game.");
                                        } else {
                                                centerprint_all ("Game on!");
                                        }
                                }
                                return;
                        } else if (!Q_stricmp (cmd+1, "message")) {
                                Cmd_Message_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "push")) {
                                if (has_permission (ent,PERMISSION_PUSH))
                                        Cmd_Push_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "blast")) {
                                if (has_permission (ent,PERMISSION_BLAST))
                                        Cmd_Blast_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "debugmove")) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_DebugMove (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "debugbox")) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_DebugBox_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "goto")) {
                                Cmd_Goto (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "chase")) {
                                Cmd_Chase_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "viewteamchat")) {
                                if (has_permission (ent,PERMISSION_VIEWGAMESTUFF))
                                        Cmd_ViewTeamChat (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "debugset")) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_DebugEntitySet (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "debuguse")) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_DebugUseEntity (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "debuguser")) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_DebugUser (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "debugfind")) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_DebugFind (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "debugfree")) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_DebugFree (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "debugdie")) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_DebugDie (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "debugget")) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_DebugEntityGet (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "debugtriguse")) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_DebugTrigUse (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "debuglist")) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_DebugGetEnts (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "debugqueues")) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_DumpSpawnQueues_f(ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "debugclassdie")) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_DebugClassKill (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "debugclassfree")) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_DebugClassFree (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "deathball")) {
                                if (has_permission (ent,PERMISSION_DEATHBALL))
                                        Cmd_Deathball_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "sd")) {
                                if (has_permission (ent,PERMISSION_SUDDENDEATH))
                                        Cmd_SuddenDeath_f (ent);
                                return;
                        } else if (Q_stricmp (cmd+1, "n00k1") == 0) {
                                if (has_permission (ent,PERMISSION_BLAST))
                                        Cmd_Sort_f (ent);
                                return;
                        } else if (Q_stricmp (cmd+1, "n00k2") == 0) {
                                if (has_permission (ent, PERMISSION_BLAST))
                                        Cmd_Sort2_f (ent);
                                return;
                        } else if (Q_stricmp (cmd+1, "setteam") == 0) {
                                if (has_permission (ent,PERMISSION_SETTEAM))
                                        Cmd_SetTeam (ent);
                                return;
                        } else if (Q_stricmp (cmd+1, "ip") == 0) {
                                if (has_permission (ent,PERMISSION_VIEW))
                                        Cmd_IP_f (ent);
                                return;
                        } else if (Q_stricmp (cmd+1, "bypass") == 0) {
                                if (has_permission (ent,PERMISSION_DEBUG)) {
                                        team_info.classes = atoi(gi.argv(1));
                                        DoClassMenus ();
                                }
                                return;
                        } else if (Q_stricmp (cmd+1, "classban") == 0) {
                                if (has_permission (ent,PERMISSION_CLASSBAN))
                                        Cmd_Classban_f (ent);
                                return;
                        } else if (Q_stricmp (cmd+1, "test") == 0) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_Test_f (ent);
                                return;
                        } else if (Q_stricmp (cmd+1, "search") == 0) {
                                if (has_permission (ent,PERMISSION_VIEW))
                                        Cmd_Search_f (ent);
                                return;
                        } else if (Q_stricmp (cmd+1, "evadelist") == 0) {
                                if (has_permission (ent, PERMISSION_VIEW))
                                        Cmd_EvadeList_f (ent);
                                return;
                        } else if (Q_stricmp (cmd+1, "addword") == 0) {
                                if (has_permission (ent,PERMISSION_WORDFILTERS))
                                        Cmd_AddWord_f (ent);
                                return;
                        } else if (Q_stricmp (cmd+1, "removeword") == 0) {
                                if (has_permission (ent,PERMISSION_WORDFILTERS))
                                        Cmd_RemoveWord_f (ent);
                                return;
                        } else if (Q_stricmp (cmd+1, "shot") == 0) {
                                if (has_permission (ent,PERMISSION_SHOT))
                                        Cmd_Shot_f (ent);
                                return;
                        } else if (Q_stricmp (cmd+1, "florb") == 0) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_Florb_f (ent);
                                return;
                        } else if (Q_stricmp (cmd+1, "trace") == 0) {
                                if (has_permission (ent,PERMISSION_DEBUG))
                                        Cmd_Trace_f (ent);
                                return;
                        } else if (Q_stricmp (cmd+1, "tail") == 0) {
/*                              if (gi.argc() > 1)
                                  TailLog(ent, gi.argv(1), atoi((gi.argc()>2)?gi.argv(2):"0"), atoi((gi.argc()>3)?gi.argv(3):"0"));
                                else
                                  gi.cprintf(ent, PRINT_HIGH, "@tail <logname> [start] [count]\n");
*/
                                return;
                        } else if (!Q_stricmp (cmd+1, "addaccount")) {
                                if (has_permission (ent,PERMISSION_ACCOUNTS))
                                        Cmd_Addaccount_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "delaccount")) {
                                if (has_permission (ent,PERMISSION_ACCOUNTS))
                                        Cmd_Deleteaccount_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "disableaccount")) {
                                if (has_permission (ent,PERMISSION_ACCOUNTS))
                                        Cmd_Modifyaccount_f (ent,0);
                                return;
                        } else if (!Q_stricmp (cmd+1, "enableaccount")) {
                                if (has_permission (ent,PERMISSION_ACCOUNTS))
                                        Cmd_Modifyaccount_f (ent,1);
                                return;
                        } else if (!Q_stricmp (cmd+1, "saveuseraccounts")) {
                                if (has_permission (ent,PERMISSION_ACCOUNTS)) {
                                        if (accounts->string[0]) {
                                                int records = SaveUserAccounts (accounts->string);
                                                if (records) {
                                                        gi.cprintf (ent, PRINT_HIGH, "%d accounts saved.\n",records);
                                                } else {
                                                        gi.cprintf (ent, PRINT_HIGH, "WARNING: SaveUserAccounts failed!\n");
                                                }
                                        }
                                }
                                return;
                        } else if (!Q_stricmp (cmd+1, "adminmode")) {
                                if (has_permission (ent,PERMISSION_ADMINMODE))
                                        Cmd_AdminMode_f (ent);
                                return;
			} else if (!Q_stricmp (cmd+1, "setfrags")) {
				if (has_permission (ent,PERMISSION_ADMINMODE))
					Cmd_SetFrags_f(ent);
				return;
                        } else if (!Q_stricmp (cmd+1, "viewchangelog")) {
                                if (has_permission (ent,PERMISSION_VIEWGAMESTUFF))
                                        Cmd_ViewChangeLog (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "pointer")) {
                                if (has_permission (ent, PERMISSION_DEBUG))
                                        Cmd_Point_f (ent);
                                return;
                        } else if (!Q_stricmp (cmd+1, "leapfrog")) {
                                if (has_permission (ent, PERMISSION_BLAST))
                                        Cmd_leapfrog_f (ent);
                                return;
                        } else
                        if (!strcasecmp(cmd+1, "whobuilt")) {
                          if (has_permission (ent, PERMISSION_VIEWGAMESTUFF))
                            Cmd_WhoBuilt_f (ent);
                          return;
                        }
                }
        }

        // those commands that must work during intermission
        if (cmd[0] == 's' || cmd[0] == 'S') {

                if (Q_strncasecmp (cmd+1, "ay", 2) == 0) {

                        if (cmd[3] == '\0') {
                                Cmd_Say_f (ent, CHAT_GLOBAL);
                                return;
                        } else if (Q_stricmp (cmd+3, "_team") == 0) {
                                Cmd_Say_f (ent, CHAT_TEAM);
                                return;
                        } else if (Q_stricmp (cmd+3, "_admins") == 0) {
                                Cmd_Say_f (ent, CHAT_ADMINS);
                                return;
                        }
                } else if (Q_stricmp (cmd+1, "tats") == 0) {
                        Cmd_Stats_f (ent);
                        return;
                }

        } else if (cmd[0] == 'p' || cmd[0] == 'P') {

                //if (Q_stricmp (cmd+1, "art") == 0) {
                //      Cmd_Part_f (ent);
                //      return;
                if (Q_stricmp (cmd+1, "layerlist") == 0) {
                        Cmd_Playerlist_f (ent);
                        return;
                } else if (Q_stricmp (cmd+1, "layers") == 0) {
                        Cmd_Players_f (ent);
                        return;
                }
        } else if (cmd[0] == 'm' || cmd[0] == 'M') {

                if (Q_stricmp (cmd+1, "aplist") == 0){
                        Cmd_Maplist_f (ent);
                        return;
                }
        } else if (cmd[0] == 'g' || cmd[0] == 'G') {

                if (Q_stricmp(cmd+1,"host") == 0 ) {
                        Cmd_Static_Ghost_Code (ent);
                        return;
                }
        } else
        if (cmd[0] == 'l' && ISLINKBOT(ent)) {
           if (!strcasecmp(cmd+1, "inkgchat")) {
             Cmd_Say_f(ent, CHAT_GLOBAL | CHAT_LINK);
             return;
           }
           if (!strcasecmp(cmd+1, "inktchat")) {
             Cmd_Say_f(ent, CHAT_TEAM | CHAT_LINK);
             return;
           }
        }

        if (level.intermissiontime && (level.intermissiontime < level.framenum))
                return;

        // other commands
        switch (cmd[0]) {

                case 'a':
                case 'A':

                if (Q_stricmp (cmd+1, "dminlist") == 0)
                        Cmd_Adminlist_f (ent);
                else
                        unknown = true;

                break;

                case 'c':
                case 'C':

                if (Q_strncasecmp (cmd+1, "hange", 5) == 0) {

                        if (Q_stricmp (cmd+6, "class") == 0)
                                Cmd_Changeclass_f(ent);
                        else if (Q_stricmp (cmd+6, "team") == 0)
                                Cmd_Changeteam_f (ent);
                        else
                                unknown = true;
                }
                else if (Q_stricmp (cmd+1, "reate") == 0)
                        Cmd_Build_f (ent);
                else if (Q_stricmp (cmd+1, "hase") == 0)
                        Cmd_Chase_f (ent);
                else if (Q_stricmp (cmd+1, "lipprev") == 0)
                        Cmd_ClipPrev_f (ent);
                else if (Q_stricmp (cmd+1, "lipnext") == 0)
                        Cmd_ClipNext_f (ent);
                else if (Q_stricmp (cmd+1, "losemenu") == 0)
                        Cmd_PutAway_f (ent);
                else if (Q_stricmp (cmd+1, "oords") == 0)
                        Cmd_Coords_f (ent);
                else
                        unknown = true;

                break;

                case 'd':
                case 'D':

                if (Q_stricmp (cmd+1, "estroy") == 0) {
                  if (!(ent->client->svframeflags & csfv_poked_once)) {
                    ent->client->svframeflags |= csfv_poked_once;
                     
                    if(ent->client->resp.class_type == CLASS_BREEDER)
                      breeder_deconstruct (ent, 72);
                    else if(ent->client->resp.class_type == CLASS_ENGINEER)
                      eng_deconstruct (ent, 72);
                  }
                }
                else if (Q_stricmp (cmd+1, "ischarge") == 0)
                        Cmd_Discharge_f (ent);
                else
                        unknown = true;

                break;

                case 'f':
                case 'F':

                if (Q_stricmp (cmd+1, "lashlight") == 0) {
                  if (!(ent->client->svframeflags & csfv_command_once)) {
                    ent->client->svframeflags |= csfv_command_once;
                    Cmd_Flashlight_f (ent);         
                  }
                }else
                        unknown = true;

                break;

                case 'g':
                case 'G':

                if (Q_stricmp (cmd+1, "renade") == 0)
                        Cmd_Grenade_f (ent);
                else if (Q_stricmp (cmd+1, "od") == 0)
                        Cmd_God_f (ent);
                else if (Q_stricmp (cmd+1, "ive") == 0)
                        Cmd_Give_f (ent);
                /* could be used to flood admins
                else if (!Q_strncasecmp (cmd, "gl_", 3)) {
                        SendToAdmins ("%s failed command: %s %s\n", ent->client->pers.netname, cmd, gi.args());
                        unknown = true;
                }*/ else
                        unknown = true;


                break;

                case 'h':
                case 'H':

                if (Q_stricmp (cmd+1, "elp") == 0)
                        Cmd_Help_f (ent);
                else
                        unknown = true;

                break;

                case 'i':
                case 'I':

                if (Q_strncasecmp (cmd+1, "nv", 2) == 0) {

                        if (Q_stricmp (cmd+3, "next") == 0)
                                SelectNextItem (ent, -1);
                        else if (Q_stricmp (cmd+3, "prev") == 0)
                                SelectPrevItem (ent, -1);
                        else if (Q_stricmp (cmd+3, "use") == 0)
                                Cmd_InvUse_f (ent);
                        else if (Q_stricmp (cmd+3, "nextw") == 0)
                                SelectNextItem (ent, IT_WEAPON);
                        else if (Q_stricmp (cmd+3, "prevw") == 0)
                                SelectPrevItem (ent, IT_WEAPON);
                        else if (Q_stricmp (cmd+3, "nextp") == 0)
                                SelectNextItem (ent, IT_POWERUP);
                        else if (Q_stricmp (cmd+3, "prevp") == 0)
                                SelectPrevItem (ent, IT_POWERUP);
                        else if (Q_stricmp (cmd+3, "en") == 0)
                                Cmd_Inven_f (ent);
                        else
                                unknown = true;
                }
                else if (Q_stricmp (cmd+1, "d") == 0)
                        CTFID_f (ent);
                else if (Q_stricmp (cmd+1, "dent") == 0)
                        CTFID_f (ent);
                else if (Q_stricmp (cmd+1, "gnore") == 0)
                        Cmd_Ignore_f (ent);
                else
                        unknown = true;

                break;

                case 'k':
                case 'K':

                if (Q_stricmp (cmd+1, "ill") == 0)
                        Cmd_Kill_f (ent);
                else
                        unknown = true;

                break;

                case 'l':
                case 'L':

                if (Q_stricmp (cmd+1, "ay") == 0)
                        Cmd_Build_f (ent);
                else if (Q_stricmp (cmd+1, "eave") == 0)
                        RemovePlayerFromQueues(ent);
                else if (Q_stricmp (cmd+1, "istchase") == 0)
                        Cmd_Listchase_f (ent);
                else if (Q_stricmp (cmd+1, "ogin") == 0)
                        Cmd_Login_f (ent);
                else
                        unknown = true;

                break;

                case 'n':
                case 'N':

                if (Q_stricmp (cmd+1, "otarget") == 0)
                        Cmd_Notarget_f (ent);
                else if (Q_stricmp (cmd+1, "oclip") == 0)
                        Cmd_Noclip_f (ent);
                else
                        unknown = true;

                break;

                case 'o':
                case 'O':

                if (Q_stricmp (cmd+1, "bs") == 0) {
                        JoinTeam0 (ent);
                } else
                        unknown = true;

                break;

                case 'p':
                case 'P':

                if (Q_stricmp (cmd+1, "utaway") == 0)
                        Cmd_PutAway_f (ent);
                else if (Q_stricmp (cmd+1, "info") == 0)
                        Cmd_Pinfo_f (ent);
                else
                        unknown = true;

                break;

                case 'r':
                case 'R':

                if (cmd[1] == 'e' || cmd[1] == 'E') {
                        if (Q_stricmp (cmd+2, "spawn") == 0)
                                Cmd_Respawn_f (ent);
                        else if (Q_stricmp (cmd+2, "store") == 0)
                                Cmd_Recover_f (ent);
                        else if (Q_stricmp (cmd+2, "load") == 0)
                                Reload (ent);
                        else
                                unknown = true;
                } else
                if (!strcasecmp(cmd, "ri"))
                  Impulse(ent, atoi(gi.argv(1)));
                else
                  unknown = true;

                break;

                case 's':
                case 'S':

                if (Q_stricmp (cmd+1, "core") == 0)
                        Cmd_Score_f (ent);
                else if ((ent->client->pers.adminpermissions & (1 << PERMISSION_CLOAK)) && Q_stricmp (cmd+1, "ilent") == 0) {
                        ent->client->resp.visible = !ent->client->resp.visible;
                        if (ent->client->resp.visible) {
                                gi.cprintf (ent, PRINT_HIGH, "You are now visible.\n");
                        } else {
                                gi.cprintf (ent, PRINT_HIGH, "You are now invisible.\n");
                        }
                }
                /* could be used to flood admins
                else if (!Q_strncasecmp (cmd, "sw_", 3)) {
                        SendToAdmins ("%s failed command: %s %s\n", ent->client->pers.netname, cmd, gi.args());
                        unknown = true;
                }*/ else
                        unknown = true;

                break;

                case 't':
                case 'T':

                if (Q_stricmp (cmd+1, "imeleft") == 0) {
                        Cmd_Timeleft_f (ent);
                } else
                        unknown = true;

                break;

                case 'u':
                case 'U':

                if (Q_stricmp (cmd+1, "se") == 0)
                        Cmd_Use_f (ent, gi.args());
                else if (Q_stricmp (cmd+1, "ptime") == 0)
                        Cmd_Uptime_f (ent);
                else if (Q_stricmp (cmd+1, "nignore") == 0)
                        Cmd_Unignore_f (ent);
                else if (Q_stricmp (cmd+1, "pgrades") == 0)
                        Cmd_Upgrades_f (ent);
                else
                        unknown = true;

                break;

                case 'v':
                case 'V':

                if (cmd[1] == 'o' || cmd[1] == 'O') {
                        if (Q_stricmp (cmd+2, "ice") == 0)
                                Cmd_Voice_f (ent);
                        else if (Q_stricmp (cmd+2, "te") == 0 )
                                Cmd_Vote_f (ent);
                        else
                                unknown = true;
                }
                else if (Q_stricmp (cmd+1, "iewlog") == 0)
                        Cmd_Viewlog_f (ent);
                /* could be used to flood admins
                else if (!Q_strncasecmp (cmd, "vid_", 4)) {
                        SendToAdmins ("%s failed command: %s %s\n", ent->client->pers.netname, cmd, gi.args());
                        unknown = true;
                }*/ else
                        unknown = true;

                break;

                case 'w':
                case 'W':

                if (Q_strncasecmp (cmd+1, "eap", 3) == 0) {
                        if (Q_stricmp (cmd+4, "prev") == 0)
                                Cmd_WeapPrev_f (ent);
                        else if (Q_stricmp (cmd+4, "next") == 0)
                                Cmd_WeapNext_f (ent);
                        else if (Q_stricmp (cmd+4, "last") == 0)
                                Cmd_WeapLast_f (ent);
                        else if (Q_stricmp (cmd+4, "switch") == 0)
                                Cmd_WeapSwitch_f (ent);
                        else
                                unknown = true;
                }
                else if (Q_stricmp (cmd+1, "ave") == 0)
                        Cmd_Wave_f (ent);
                else
                        unknown = true;

                break;

                default:
                        unknown = true;
                break;
        }

        if (unknown) {
                if ((gi.cvar("unknowncmdlog", "0", 0))->value)
                {
                        static FILE *x = NULL;
                        if (!x)
                                x = fopen ("gloom/commandlog.txt", "a+");
                        fprintf (x, "%s[%s]: %s %s\n", ent->client->pers.netname, ent->client->pers.ip, cmd, gi.args());
                        fflush (x);
                }
                gi.cprintf (ent,PRINT_HIGH,"Unknown command: %s\n", cmd);
        }
}
