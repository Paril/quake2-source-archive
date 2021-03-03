/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "g_local.h"
#include "m_player.h"

static void Cmd_Menu_f( edict_t *ent );

static void SelectNextItem (edict_t *ent, int itflags) {
    gclient_t   *cl;
    int         i, index;
    const gitem_t       *it;

    cl = ent->client;

    if( cl->layout == LAYOUT_MENU ) {
        PMenu_Next( ent );
        return;
    }
    if (cl->chase_target) {
        ChaseNext(ent);
        cl->chase_mode = CHASE_NONE;
        return;
    }

    // scan  for the next valid one
    for (i=1 ; i<=ITEM_TOTAL ; i++)
    {
        index = (cl->selected_item + i)%ITEM_TOTAL;
        if (!cl->inventory[index])
            continue;
        it = INDEX_ITEM(index);
        if (!it->use)
            continue;
        if (!(it->flags & itflags))
            continue;

        cl->selected_item = index;
        return;
    }

    cl->selected_item = -1;
}

static void SelectPrevItem (edict_t *ent, int itflags) {
    gclient_t   *cl;
    int         i, index;
    const gitem_t       *it;

    cl = ent->client;

    if( cl->layout == LAYOUT_MENU ) {
        PMenu_Prev( ent );
        return;
    }
    if (cl->chase_target) {
        ChasePrev(ent);
        cl->chase_mode = CHASE_NONE;
        return;
    }

    // scan  for the next valid one
    for (i=1 ; i<=ITEM_TOTAL ; i++)
    {
        index = (cl->selected_item + ITEM_TOTAL - i)%ITEM_TOTAL;
        if (!cl->inventory[index])
            continue;
        it = INDEX_ITEM(index);
        if (!it->use)
            continue;
        if (!(it->flags & itflags))
            continue;

        cl->selected_item = index;
        return;
    }

    cl->selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
    gclient_t   *cl;

    cl = ent->client;

    if (cl->inventory[cl->selected_item])
        return;     // valid

    SelectNextItem (ent, -1);
}


//=================================================================================

static qboolean CheckCheats( edict_t *ent ) {
    if( !PLAYER_SPAWNED( ent ) ) {
        return qfalse;
    }
    if( (int)sv_cheats->value == 0 ) {
        gi.cprintf( ent, PRINT_HIGH, "Cheats are disabled on this server.\n" );
        return qfalse;
    }

    return qtrue;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
static void Cmd_Give_f (edict_t *ent)
{
    char        *name;
    gitem_t     *it;
    int         index;
    int         i;
    qboolean    give_all;
    edict_t     *it_ent;

    if( !CheckCheats( ent ) ) {
        return;
    }

    name = gi.args();

    if (Q_stricmp(name, "all") == 0)
        give_all = qtrue;
    else
        give_all = qfalse;

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
        for (i=0 ; i<ITEM_TOTAL ; i++)
        {
            it = INDEX_ITEM(i);
            if (!it->pickup)
                continue;
            if (!(it->flags & IT_WEAPON))
                continue;
            ent->client->inventory[i] += 1;
        }
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "ammo") == 0)
    {
        for (i=0 ; i<ITEM_TOTAL ; i++)
        {
            it = INDEX_ITEM(i);
            if (!it->pickup)
                continue;
            if (!(it->flags & IT_AMMO))
                continue;
            Add_Ammo (ent, it, 1000);
        }
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "armor") == 0)
    {
        gitem_armor_t   *info;

        ent->client->inventory[ITEM_ARMOR_JACKET] = 0;

        ent->client->inventory[ITEM_ARMOR_COMBAT] = 0;

        it = INDEX_ITEM( ITEM_ARMOR_BODY );
        info = (gitem_armor_t *)it->info;
        ent->client->inventory[ITEM_ARMOR_BODY] = info->max_count;

        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "Power Shield") == 0)
    {
        it = INDEX_ITEM( ITEM_POWER_SHIELD );
        it_ent = G_Spawn();
        it_ent->classname = it->classname;
        SpawnItem (it_ent, it);
        if( it_ent->inuse ) {
            Touch_Item (it_ent, ent, NULL, NULL);
            if (it_ent->inuse)
                G_FreeEdict(it_ent);
        }

        if (!give_all)
            return;
    }

    if (give_all)
    {
        for (i=0 ; i<ITEM_TOTAL ; i++)
        {
            it = INDEX_ITEM(i);
            if (!it->pickup)
                continue;
            if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
                continue;
            ent->client->inventory[i] = 1;
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
            gi.cprintf (ent, PRINT_HIGH, "unknown item\n");
            return;
        }
    }

    if (!it->pickup)
    {
        gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
        return;
    }

    index = ITEM_INDEX(it);

    if (it->flags & IT_AMMO)
    {
        if (gi.argc() == 3)
            ent->client->inventory[index] = atoi(gi.argv(2));
        else
            ent->client->inventory[index] += it->quantity;
    }
    else
    {
        it_ent = G_Spawn();
        it_ent->classname = it->classname;
        SpawnItem (it_ent, it);
        if( it_ent->inuse ) {
            Touch_Item (it_ent, ent, NULL, NULL);
            if (it_ent->inuse)
                G_FreeEdict(it_ent);
        }
    }
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
static void Cmd_God_f (edict_t *ent)
{
    if( CheckCheats( ent ) ) {
        ent->flags ^= FL_GODMODE;
        gi.cprintf( ent, PRINT_HIGH, "godmode %s\n",
            ( ent->flags & FL_GODMODE ) ? "ON" : "OFF" );
    }
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
static void Cmd_Notarget_f (edict_t *ent)
{
    if( CheckCheats( ent ) ) {
        ent->flags ^= FL_NOTARGET;
        gi.cprintf( ent, PRINT_HIGH, "notarget %s\n",
            ( ent->flags & FL_NOTARGET ) ? "ON" : "OFF" );
    }
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
static void Cmd_Noclip_f (edict_t *ent)
{
    if( !CheckCheats( ent ) ) {
        return;
    }

    if( ent->movetype == MOVETYPE_NOCLIP ) {
        ent->movetype = MOVETYPE_WALK;
    } else {
        ent->movetype = MOVETYPE_NOCLIP;
    }

    gi.cprintf( ent, PRINT_HIGH, "noclip %s\n",
        ent->movetype == MOVETYPE_NOCLIP ? "ON" : "OFF" );
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
static void Cmd_Use_f (edict_t *ent)
{
    int         index;
    gitem_t     *it;
    char        *s;

    s = gi.args();
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
    if (!ent->client->inventory[index])
    {
        gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
        return;
    }

    it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
static void Cmd_Drop_f (edict_t *ent)
{
    int         index;
    gitem_t     *it;
    char        *s;

    s = gi.args();
    it = FindItem (s);
    if (!it)
    {
        gi.cprintf (ent, PRINT_HIGH, "Unknown item: %s\n", s);
        return;
    }
    if (!it->drop)
    {
        gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
        return;
    }
    index = ITEM_INDEX(it);
    if (!ent->client->inventory[index])
    {
        gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
        return;
    }

    it->drop (ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
static void Cmd_Inven_f (edict_t *ent) {
    // this one is left for backwards compatibility
    Cmd_Menu_f( ent );
}

/*
=================
Cmd_InvUse_f
=================
*/
static void Cmd_InvUse_f (edict_t *ent)
{
    gitem_t     *it;

    if( ent->client->layout == LAYOUT_MENU ) {
        PMenu_Select( ent );
        return;
    }

    ValidateSelectedItem (ent);

    if (ent->client->selected_item == -1)
    {
        gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
        return;
    }

    it = INDEX_ITEM( ent->client->selected_item );
    if (!it->use)
    {
        gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
        return;
    }
    it->use (ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
static void Cmd_WeapPrev_f (edict_t *ent)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;
    int         selected_weapon;

    cl = ent->client;

    if (!cl->weapon)
        return;

    selected_weapon = ITEM_INDEX(cl->weapon);

    // scan  for the next valid one
    for (i=1 ; i<=ITEM_TOTAL ; i++)
    {
        index = (selected_weapon + i)%ITEM_TOTAL;
        if (!cl->inventory[index])
            continue;
        it = INDEX_ITEM( index );
        if (!it->use)
            continue;
        if (! (it->flags & IT_WEAPON) )
            continue;
        it->use (ent, it);
        if (cl->weapon == it)
            return; // successful
    }
}

/*
=================
Cmd_WeapNext_f
=================
*/
static void Cmd_WeapNext_f (edict_t *ent)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;
    int         selected_weapon;

    cl = ent->client;

    if (!cl->weapon)
        return;

    selected_weapon = ITEM_INDEX(cl->weapon);

    // scan  for the next valid one
    for (i=1 ; i<=ITEM_TOTAL ; i++)
    {
        index = (selected_weapon + ITEM_TOTAL - i)%ITEM_TOTAL;
        if (!cl->inventory[index])
            continue;
        it = INDEX_ITEM( index );
        if (!it->use)
            continue;
        if (! (it->flags & IT_WEAPON) )
            continue;
        it->use (ent, it);
        if (cl->weapon == it)
            return; // successful
    }
}

/*
=================
Cmd_WeapLast_f
=================
*/
static void Cmd_WeapLast_f (edict_t *ent)
{
    gclient_t   *cl;
    int         index;
    gitem_t     *it;

    cl = ent->client;

    if (!cl->weapon || !cl->lastweapon)
        return;

    index = ITEM_INDEX(cl->lastweapon);
    if (!cl->inventory[index])
        return;
    it = INDEX_ITEM( index );
    if (!it->use)
        return;
    if (! (it->flags & IT_WEAPON) )
        return;
    it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
static void Cmd_InvDrop_f (edict_t *ent)
{
    gitem_t     *it;

    if( ent->client->layout == LAYOUT_MENU ) {
        PMenu_Select( ent );
        return;
    }

    ValidateSelectedItem (ent);

    if (ent->client->selected_item == -1)
    {
        gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
        return;
    }

    it = INDEX_ITEM( ent->client->selected_item );
    if (!it->drop)
    {
        gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
        return;
    }
    it->drop (ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
static void Cmd_Kill_f (edict_t *ent)
{
    if (!PLAYER_SPAWNED (ent))
        return;
    if(level.framenum - ent->client->respawn_framenum < 5*HZ)
        return;
    ent->flags &= ~FL_GODMODE;
    ent->health = 0;
    meansOfDeath = MOD_SUICIDE;
    player_die (ent, ent, ent, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
static void Cmd_PutAway_f (edict_t *ent) {
    ent->client->layout = 0;
}

qboolean G_FloodProtect( edict_t *ent, flood_t *flood,
    const char *what, int msgs, float persecond, float delay )
{
    int i, sec;

    if( msgs < 1 ) {
        return qfalse;
    }

    if( level.framenum < flood->locktill ) {
        sec = ( flood->locktill - level.framenum ) / HZ;
        if( sec < 1 ) {
            sec = 1;
        }
        gi.cprintf( ent, PRINT_HIGH, "You can't %s for %d more second%s.\n",
            what, sec, sec == 1 ? "" : "s" );
        return qtrue;
    }

    i = flood->whenhead - msgs + 1;
    if( i >= 0 && level.framenum - flood->when[i % FLOOD_MSGS] < persecond*HZ ) {
        flood->locktill = level.framenum + delay*HZ;
        gi.cprintf( ent, PRINT_CHAT,
            "Flood protection: You can't %s for %d seconds.\n", what, (int)delay );
        gi.dprintf( "%s can't %s for %d seconds\n",
            ent->client->pers.netname, what, (int)delay );
        return qtrue;
    }

    flood->when[++flood->whenhead % FLOOD_MSGS] = level.framenum;
    return qfalse;
}


/*
=================
Cmd_Wave_f
=================
*/
static void Cmd_Wave_f (edict_t *ent)
{
    int     i;

    // spectators can't wave!
    if( !PLAYER_SPAWNED( ent ) ) {
        return;
    }

    // can't wave when ducked
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
        return;

    if (ent->client->anim_priority > ANIM_WAVE)
        return;

    if( G_FloodProtect( ent, &ent->client->level.wave_flood, "use waves",
        (int)flood_waves->value, flood_perwave->value, flood_wavedelay->value ) )
    {
        return;
    }

    i = atoi (gi.argv(1));

    ent->client->anim_priority = ANIM_WAVE;

    switch (i)
    {
    case 0:
        gi.cprintf (ent, PRINT_LOW, "flipoff\n");
        ent->s.frame = FRAME_flip01-1;
        ent->client->anim_end = FRAME_flip12;
        break;
    case 1:
        gi.cprintf (ent, PRINT_LOW, "salute\n");
        ent->s.frame = FRAME_salute01-1;
        ent->client->anim_end = FRAME_salute11;
        break;
    case 2:
        gi.cprintf (ent, PRINT_LOW, "taunt\n");
        ent->s.frame = FRAME_taunt01-1;
        ent->client->anim_end = FRAME_taunt17;
        break;
    case 3:
        gi.cprintf (ent, PRINT_LOW, "wave\n");
        ent->s.frame = FRAME_wave01-1;
        ent->client->anim_end = FRAME_wave11;
        break;
    case 4:
    default:
        gi.cprintf (ent, PRINT_LOW, "point\n");
        ent->s.frame = FRAME_point01-1;
        ent->client->anim_end = FRAME_point12;
        break;
    }
}

#define MAX_CHAT   150

typedef enum {
    CHAT_MISC,
    CHAT_ALL,
    CHAT_TEAM
} chat_t;

static size_t build_chat( const char *name, chat_t chat, int start, char *buffer ) {
    size_t len, total;
    int i;
    char *p;

    total = Q_scnprintf( buffer, MAX_CHAT,
        ( chat == CHAT_TEAM ) ? "(%s): " : "%s: ", name );

    for (i = start; i < gi.argc(); i++) {
        p = gi.argv (i);
        len = strlen (p);
        if (!len)
            continue;
        if (total + len + 1 >= MAX_CHAT)
            break;
        while (*p) {
            int c = *p++;
            c &= 127;   // strip high bits
            // don't allow carriage returns, etc
            if (!Q_isspecial(c))
                buffer[total++] = c;
        }
        buffer[total++] = ' ';
    }
    buffer[total] = 0;

    return total;
}

/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f (edict_t *ent, chat_t chat)
{
    int     i, start;
    edict_t *other;
    char    text[MAX_CHAT];
    gclient_t *cl = ent->client;

    start = ( chat == CHAT_MISC ) ? 0 : 1;
    if (gi.argc () <= start)
        return;

    // don't flood protect team chat to self
    if( chat == CHAT_TEAM && (int)g_team_chat->value == 0 && PLAYER_SPAWNED( ent ) ) {
        build_chat( cl->pers.netname, chat, start, text );
        gi.cprintf(ent, PRINT_CHAT, "%s\n", text);
        return;
    }

    // stop flood during the match
    if( !cl->pers.admin && !level.intermission_framenum ) {
        if( cl->pers.muted ) {
            gi.cprintf( ent, PRINT_HIGH, "You are not allowed to talk.\n" );
            return;
        }
        if( (int)g_mute_chat->value ) {
            if( PLAYER_SPAWNED( ent ) ) {
                gi.cprintf( ent, PRINT_HIGH, "Players can't talk during the match.\n" );
                return;
            }
            if( (int)g_mute_chat->value > 1 ) {
                gi.cprintf( ent, PRINT_HIGH, "Spectators can't talk during the match.\n" );
                return;
            }
            chat = CHAT_TEAM;
        }
    }

    if( G_FloodProtect( ent, &cl->level.chat_flood, "talk",
        (int)flood_msgs->value, flood_persecond->value, flood_waitdelay->value ) )
    {
        return;
    }

    build_chat( cl->pers.netname, chat, start, text );

    if ((int)dedicated->value)
        gi.cprintf(NULL, PRINT_CHAT, "%s\n", text);

    for (i = 1; i <= game.maxclients; i++)
    {
        other = &g_edicts[i];
        if (!other->inuse)
            continue;
        if (!other->client)
            continue;
        if (chat == CHAT_TEAM && PLAYER_SPAWNED( ent ) != PLAYER_SPAWNED( other ) ) {
            continue;
        }
        gi.cprintf(other, PRINT_CHAT, "%s\n", text);
    }
}

void Cmd_Players_f( edict_t *ent ) {
    gclient_t *c;
    int i, sec;
    char score[16], idle[16], time[16];
    qboolean show_ips = !ent || ent->client->pers.admin;

    gi.cprintf( ent, PRINT_HIGH,
        "id score ping time name            idle %s\n"
        "-- ----- ---- ---- --------------- ---- %s\n",
        show_ips ? "address" : "",
        show_ips ? "-------" : "" );

    for( i = 0; i < game.maxclients; i++ ) {
        c = &game.clients[i];
        if( c->pers.connected <= CONN_CONNECTED ) {
            continue;
        }
        if( c->pers.connected == CONN_SPAWNED ) {
            sprintf( score, "%d", c->resp.score );
            sec = ( level.framenum - c->resp.activity_framenum ) / HZ;
            if( level.framenum < 10 * 60 * HZ ) {
                sprintf( idle, "%d:%02d", sec / 60, sec % 60 );
            } else {
                sprintf( idle, "%d", sec / 60 );
            }
        } else {
            strcpy( score, "SPECT" );
            strcpy( idle, "-" );
        }
        sec = ( level.framenum - c->resp.enter_framenum ) / HZ;
        if( level.framenum < 10 * 60 * HZ ) {
            sprintf( time, "%d:%02d", sec / 60, sec % 60 );
        } else {
            sprintf( time, "%d", sec / 60 );
        }
        gi.cprintf( ent, PRINT_HIGH, "%2d %5s %4d %4s %-15s %4s %s\n",
            i, score, c->ping, time, c->pers.netname, idle,
            show_ips ? c->pers.ip : "" );
    }
}

void Cmd_HighScores_f( edict_t *ent ) {
    int i;
    char date[MAX_QPATH];
    struct tm *tm;
    score_t *s;

    if( !level.numscores ) {
        gi.cprintf( ent, PRINT_HIGH, "No high scores available.\n" );
        return;
    }

    gi.cprintf( ent, PRINT_HIGH,
        " # Name            FPH  Date\n"
        "-- --------------- ---- ----------------\n" );
    for( i = 0; i < level.numscores; i++ ) {
        s = &level.scores[i];

        tm = localtime( &s->time );
        strftime( date, sizeof( date ), "%Y-%m-%d %H:%M", tm );
        gi.cprintf( ent, PRINT_HIGH, "%2d %-15.15s %4d %s\n",
            i + 1, s->name, s->score, date );
    }
}

edict_t *G_SetPlayer( edict_t *ent, int arg ) {
    edict_t     *other, *match;
    int         i, count;
    char        *s;

    s = gi.argv(arg);

    // numeric values are just slot numbers
    if( COM_IsUint( s ) ) {
        i = atoi(s);
        if (i < 0 || i >= game.maxclients) {
            gi.cprintf (ent, PRINT_HIGH, "Bad client slot number: %d\n", i);
            return NULL;
        }

        other = &g_edicts[ i + 1 ];
        if (!other->client || other->client->pers.connected <= CONN_CONNECTED) {
            gi.cprintf (ent, PRINT_HIGH, "Client %d is not active.\n", i);
            return NULL;
        }
        return other;
    }

    // check for a name match
    match = NULL;
    count = 0;
    for( i = 0; i < game.maxclients; i++ ) {
        other = &g_edicts[ i + 1 ];
        if (!other->client ) {
            continue;
        }
        if( other->client->pers.connected <= CONN_CONNECTED) {
            continue;
        }
        if (!Q_stricmp(other->client->pers.netname, s)) {
            return other; // exact match
        }
        if (Q_stristr(other->client->pers.netname, s)) {
            match = other; // partial match
            count++;
        }
    }

    if( !match ) {
        gi.cprintf( ent, PRINT_HIGH, "No clients matching '%s' found.\n", s );
        return NULL;
    }

    if( count > 1 ) {
        gi.cprintf( ent, PRINT_HIGH, "'%s' matches multiple clients.\n", s );
        return NULL;
    }

    return match;
}

edict_t *G_SetVictim( edict_t *ent, int start ) {
    edict_t *other = G_SetPlayer( ent, start + 1 );
    if( !other ) {
        return NULL;
    }

    if( other == ent ) {
        gi.cprintf( ent, PRINT_HIGH, "You can't %s yourself.\n", gi.argv( start ) );
        return NULL;
    }

    if( other->client->pers.loopback || other->client->pers.mvdspec ) {
        gi.cprintf( ent, PRINT_HIGH, "You can't %s local client.\n", gi.argv( start ) );
        return NULL;
    }
    if( other->client->pers.admin ) {
        gi.cprintf( ent, PRINT_HIGH, "You can't %s an admin.\n", gi.argv( start ) );
        return NULL;
    }

    return other;
}


static qboolean G_SpecRateLimited( edict_t *ent ) {
    if( level.framenum - ent->client->resp.enter_framenum < 5*HZ ) {
        gi.cprintf( ent, PRINT_HIGH, "You may not change modes too soon.\n" );
        return qtrue;
    }
    return qfalse;
}

static void Cmd_Observe_f( edict_t *ent ) {
    if( ent->client->pers.connected == CONN_PREGAME ) {
        ent->client->pers.connected = CONN_SPECTATOR;
        gi.cprintf( ent, PRINT_HIGH, "Changed to spectator mode.\n" );
        return;
    }
    if( G_SpecRateLimited( ent ) ) {
        return;
    }
    if( ent->client->pers.connected == CONN_SPECTATOR ) {
        spectator_respawn( ent, CONN_SPAWNED );
    } else {
        spectator_respawn( ent, CONN_SPECTATOR );
    }
}

static void Cmd_Chase_f( edict_t *ent ) {
    edict_t *target = NULL;
    chase_mode_t mode = CHASE_NONE;

    if( gi.argc() == 2 ) {
        char *who = gi.argv( 1 );

        if( !Q_stricmp( who, "quad" ) ) {
            mode = CHASE_QUAD;
        } else if( !Q_stricmp( who, "inv" ) ||
                   !Q_stricmp( who, "pent" ) )
        {
            mode = CHASE_INVU;
        } else if( !Q_stricmp( who, "top" ) || 
                   !Q_stricmp( who, "topfragger" ) ||
                   !Q_stricmp( who, "leader" ) )
        {
            mode = CHASE_LEADER;
        } else {
            target = G_SetPlayer( ent, 1 );
            if( !target ) {
                return;
            }
            if( !PLAYER_SPAWNED( target ) ) {
                gi.cprintf( ent, PRINT_HIGH,
                    "Player '%s' is not in the game.\n",
                    target->client->pers.netname);
                return;
            }
        }
    }

    // changing from pregame mode into spectator
    if( ent->client->pers.connected == CONN_PREGAME ) {
        ent->client->pers.connected = CONN_SPECTATOR;
        gi.cprintf( ent, PRINT_HIGH, "Changed to spectator mode.\n" );
        if( target ) {
            SetChaseTarget( ent, target );
            ent->client->chase_mode = CHASE_NONE;
        } else {
            GetChaseTarget( ent, mode );
        }
        return;
    }

    // respawn the spectator
    if( ent->client->pers.connected != CONN_SPECTATOR ) {
        if( G_SpecRateLimited( ent ) ) {
            return;
        }
        spectator_respawn( ent, CONN_SPECTATOR );
    }

    if( target ) {
        if( target == ent->client->chase_target ) {
            gi.cprintf( ent, PRINT_HIGH,
                "You are already chasing this player.\n");
            return;
        }
        SetChaseTarget( ent, target );
        ent->client->chase_mode = CHASE_NONE;
    } else {
        if( !ent->client->chase_target || mode != CHASE_NONE ) {
            GetChaseTarget( ent, mode );
        } else {
            SetChaseTarget( ent, NULL );
        }
    }
}

static void Cmd_Join_f( edict_t *ent ) {
    switch( ent->client->pers.connected ) {
    case CONN_PREGAME:
    case CONN_SPECTATOR:
        if( G_SpecRateLimited( ent ) ) {
            return;
        }
        spectator_respawn( ent, CONN_SPAWNED );
        break;
    case CONN_SPAWNED:
        gi.cprintf( ent, PRINT_HIGH, "You are already in the game.\n" );
        break;
    default:
        break;
    }
}

static const char weapnames[WEAP_TOTAL][12] = {
    "None",         "Blaster",      "Shotgun",      "S.Shotgun",
    "Machinegun",   "Chaingun",     "Grenades",     "G.Launcher",
    "R.Launcher",   "H.Blaster",    "Railgun",      "BFG10K"
};

void Cmd_Stats_f( edict_t *ent, qboolean check_other ) {
    int i;
    fragstat_t *s;
    char acc[16];
    char hits[16];
    char frgs[16];
    char dths[16];
    edict_t *other;

    if( !ent ) {
        if( gi.argc() < 3 ) {
            gi.cprintf( ent, PRINT_HIGH, "Usage: %s <playerID>\n", gi.argv( 1 ) );
            return;
        }
        other = G_SetPlayer( ent, 2 );
        if( !other ) {
            return;
        }
    } else if( check_other && gi.argc() > 1 ) {
        other = G_SetPlayer( ent, 1 );
        if( !other ) {
            return;
        }
    } else if( ent->client->chase_target ) {
        other = ent->client->chase_target;
    } else {
        other = ent;
    }

    for( i = FRAG_BLASTER; i <= FRAG_BFG; i++ ) {
        s = &other->client->resp.frags[i];
        if( s->atts || s->deaths ) {
            break;
        }
    }
    if( i > FRAG_BFG ) {
        gi.cprintf( ent, PRINT_HIGH, "No accuracy stats available for %s.\n",
            other->client->pers.netname );
        return;
    }

    gi.cprintf( ent, PRINT_HIGH,
        "Accuracy stats for %s:\n"
        "Weapon     Acc%% Hits/Atts Frgs Dths\n"
        "---------- ---- --------- ---- ----\n",
        other->client->pers.netname );

    for( i = FRAG_BLASTER; i <= FRAG_BFG; i++ ) {
        s = &other->client->resp.frags[i];
        if( !s->atts && !s->deaths ) {
            continue;
        }
        if( s->atts && i != FRAG_BFG ) {
            sprintf( acc, "%3i%%", s->hits * 100 / s->atts );
            sprintf( hits, "%4d/%-4d", s->hits, s->atts );
        } else {
            strcpy( acc, "    " );
            strcpy( hits, "         " );
        }
        if( s->kills ) {
            sprintf( frgs, "%4d", s->kills );
        } else {
            strcpy( frgs, "    " );
        }
        if( s->deaths ) {
            sprintf( dths, "%4d", s->deaths );
        } else {
            strcpy( dths, "    " );
        }

        gi.cprintf( ent, PRINT_HIGH,
            "%-10s %s %s %s %s\n",
            weapnames[i], acc, hits, frgs, dths );
    }

    gi.cprintf( ent, PRINT_HIGH,
        "Total damage given/recvd: %d/%d\n",
        other->client->resp.damage_given,
        other->client->resp.damage_recvd );
}

static void Cmd_Id_f( edict_t *ent ) {
    ent->client->pers.noviewid ^= 1;

    gi.cprintf( ent, PRINT_HIGH,
        "Player identification display is now %sabled.\n",
        ent->client->pers.noviewid ? "dis" : "en" );
}

void Cmd_Settings_f( edict_t *ent ) {
    char buffer[MAX_QPATH], *s;
    int v;

    if( timelimit->value > 0 ) {
        v = (int)timelimit->value;
        sprintf( buffer, "%d minute%s", v, v == 1 ? "" : "s" );
        s = buffer;
    } else {
        s = "none";
    }
    gi.cprintf( ent, PRINT_HIGH, "Timelimit:          %s\n", s );

    v = (int)fraglimit->value;
    if( v > 0 ) {
        sprintf( buffer, "%d frag%s", v, v == 1 ? "" : "s" );
        s = buffer;
    } else {
        s = "none";
    }
    gi.cprintf( ent, PRINT_HIGH, "Fraglimit:          %s\n", s );

    v = (int)g_item_ban->value;
    if( v & (ITB_QUAD|ITB_INVUL|ITB_BFG) ) {
        buffer[0] = 0;
        if( v & ITB_QUAD ) {
            strcat( buffer, "quad " );
        }
        if( v & ITB_INVUL ) {
            strcat( buffer, "invul " );
        }
        if( v & ITB_BFG ) {
            strcat( buffer, "bfg " );
        }
        if( v & ITB_PS ) {
            strcat( buffer, "ps " );
        }
        s = buffer;
    } else {
        s = "none";
    }
    gi.cprintf( ent, PRINT_HIGH, "Removed items:      %s\n", s );

    gi.cprintf( ent, PRINT_HIGH, "Weapons stay:       %s\n",
        DF( WEAPONS_STAY ) ? "on" : "off" );

    if( g_protection_time->value > 0 ) {
        if( g_protection_time->value == 1 ) {
            sprintf( buffer, "1 second" );
        } else {
            sprintf( buffer, "%.1f seconds", g_protection_time->value );
        }
        s = buffer;
    } else {
        s = "off";
    }
    gi.cprintf( ent, PRINT_HIGH, "Respawn protection: %s\n", s );

    s = (int)g_teleporter_nofreeze->value ? "no freeze" : "normal";
    gi.cprintf( ent, PRINT_HIGH, "Teleporter mode:    %s\n", s );

    if( (int)g_bugs->value < 1 ) {
        s = "all bugs fixed";
    } else if( (int)g_bugs->value < 2 ) {
        s = "serious bugs fixed";
    } else {
        s = "default Q2 behaviour";
    }
    gi.cprintf( ent, PRINT_HIGH, "Gameplay bugs:      %s\n", s );

    if( DF( SPAWN_FARTHEST ) ) {
        s = "farthest";
    } else if( (int)g_spawn_mode->value == 0 ) {
        s = "avoid closest (bugged)";
    } else if( (int)g_spawn_mode->value == 1 ) {
        s = "avoid closest";
    } else {
        s = "random";
    }
    gi.cprintf( ent, PRINT_HIGH, "Respawn mode:       %s\n", s );
}

static void Cmd_Admin_f( edict_t *ent ) {
    char *p;

    if( ent->client->pers.admin ) {
        gi.bprintf( PRINT_HIGH, "%s is no longer an admin.\n",
            ent->client->pers.netname );
        ent->client->pers.admin = qfalse;
        return;
    }
    if( gi.argc() < 2 ) {
        gi.cprintf( ent, PRINT_HIGH, "Usage: %s <password>\n", gi.argv( 0 ) );
        return;
    }
    p = gi.argv( 1 );
    if( !g_admin_password->string[0] || strcmp( g_admin_password->string, p ) ) {
        gi.cprintf( ent, PRINT_HIGH, "Bad admin password.\n" );
        if( (int)dedicated->value ) {
            gi.dprintf( "%s[%s] failed to become an admin.\n",
                ent->client->pers.netname, ent->client->pers.ip );
        }
        return;
    }

    ent->client->pers.admin = qtrue;
    gi.bprintf( PRINT_HIGH, "%s became an admin.\n",
        ent->client->pers.netname );

    G_CheckVote();
}

static void Cmd_Mute_f( edict_t *ent, qboolean muted ) {
    edict_t *other;

    if( gi.argc() < 2 ) {
        gi.cprintf( ent, PRINT_HIGH, "Usage: %s <playerID>\n", gi.argv( 0 ) );
        return;
    }
    
    other = G_SetVictim( ent, 0 );
    if( !other ) {
        return;
    }

    if( other->client->pers.muted == muted ) {
        gi.cprintf( ent, PRINT_HIGH, "%s is already %smuted\n",
            other->client->pers.netname, muted ? "" : "un" );
        return;
    }

    other->client->pers.muted = muted;
    gi.bprintf( PRINT_HIGH, "%s has been %smuted.\n",
        other->client->pers.netname, muted ? "" : "un" );
}

static void Cmd_MuteAll_f( edict_t *ent, qboolean muted ) {
    if( !!(int)g_mute_chat->value == muted ) {
        gi.cprintf( ent, PRINT_HIGH, "Players are already %smuted\n",
            muted ? "" : "un" );
        return;
    }

    gi.cvar_set( "g_mute_chat", muted ? "1" : "0" );
    gi.bprintf( PRINT_HIGH, "Players may %s talk during the match.\n",
        muted ? "no longer" : "now" );
}

static void Cmd_Kick_f( edict_t *ent, qboolean ban ) {
    edict_t *other;

    if( gi.argc() < 2 ) {
        gi.cprintf( ent, PRINT_HIGH, "Usage: %s <playerID>\n", gi.argv( 0 ) );
        return;
    }
    
    other = G_SetVictim( ent, 0 );
    if( !other ) {
        return;
    }

    gi.AddCommandString( va( "kick %d\n", ( int )( other->client - game.clients ) ) );

    if( ban ) {
        G_BanEdict( other, ent );
    }
}

static void Cmd_AdminCommands_f( edict_t *ent ) {
    gi.cprintf( ent, PRINT_HIGH,
        "(un)mute    Allow/disallow specific player to talk\n"
        "(un)muteall Allow/disallow everyone to talk\n"
        "(un)ban     Add/remove temporary bans\n"
        "bans        List bans\n"
        "kick        Kick a player\n"
        "kickban     Kick a player and ban him for 1 hour\n"
        );
}

static void Cmd_Commands_f( edict_t *ent ) {
    gi.cprintf( ent, PRINT_HIGH,
        "menu       Show OpenFFA menu\n"
        "join       Enter the game\n"
        "observe    Leave the game\n"
        "chase      Enter chasecam mode\n"
        "settings   Show match settings\n"
        "oldscore   Show previous scoreboard\n"
        "vote       Propose new settings\n"
        "stats      Show accuracy stats\n"
        "players    Show players on server\n"
        "highscores Show the best results on map\n"
        "id         Toggle player ID display\n"
        );
}

static qboolean become_spectator( edict_t *ent ) {
    switch( ent->client->pers.connected ) {
    case CONN_PREGAME:
        ent->client->pers.connected = CONN_SPECTATOR;
        break;
    case CONN_SPAWNED:
        if( G_SpecRateLimited( ent ) ) {
            return qfalse;
        }
        spectator_respawn( ent, CONN_SPECTATOR );
        break;
    case CONN_SPECTATOR:
        return qtrue;
    default:
        return qfalse;
    }

    gi.cprintf( ent, PRINT_HIGH, "Changed to spectator mode.\n" );
    return qtrue;
}

static void select_test( edict_t *ent ) {
    switch( ent->client->menu.cur ) {
    case 3:
        if( ent->client->pers.connected == CONN_SPAWNED ) {
            if( G_SpecRateLimited( ent ) ) {
                break;
            }
            spectator_respawn( ent, CONN_SPECTATOR );
            break;
        }
        if( ent->client->pers.connected != CONN_PREGAME ) {
            if( G_SpecRateLimited( ent ) ) {
                break;
            }
        }
        spectator_respawn( ent, CONN_SPAWNED );
        break;
    case 5:
        if( become_spectator( ent ) ) {
            if( ent->client->chase_target ) {
                SetChaseTarget( ent, NULL );
            }
            PMenu_Close( ent );
        }
        break;
    case 6:
        if( become_spectator( ent ) ) {
            if( !ent->client->chase_target ) {
                GetChaseTarget( ent, CHASE_NONE );
            }
            PMenu_Close( ent );
        }
        break;
    case 7:
        if( become_spectator( ent ) ) {
            GetChaseTarget( ent, CHASE_LEADER );
            PMenu_Close( ent );
        }
        break;
    case 8:
        if( become_spectator( ent ) ) {
            GetChaseTarget( ent, CHASE_QUAD );
            PMenu_Close( ent );
        }
        break;
    case 9:
        if( become_spectator( ent ) ) {
            GetChaseTarget( ent, CHASE_INVU );
            PMenu_Close( ent );
        }
        break;
    case 11:
        PMenu_Close( ent );
        break;
    }
}

static const pmenu_entry_t main_menu[MAX_MENU_ENTRIES] = {
    { "OpenFFA - Main", PMENU_ALIGN_CENTER },
    { NULL },
    { NULL },
    { NULL, PMENU_ALIGN_LEFT, select_test },
    { NULL },
    { "*Enter freefloat mode", PMENU_ALIGN_LEFT, select_test },
    { "*Enter chasecam mode", PMENU_ALIGN_LEFT, select_test },
    { "*Autocam - Frag Leader", PMENU_ALIGN_LEFT, select_test },
    { "*Autocam - Quad Runner", PMENU_ALIGN_LEFT, select_test },
    { "*Autocam - Pent Runner", PMENU_ALIGN_LEFT, select_test },
    { NULL },
    { "*Exit menu", PMENU_ALIGN_LEFT, select_test },
//    { "*Voting menu", PMENU_ALIGN_LEFT, select_test },
    { NULL },
    { NULL },
    { NULL },
    { "Use [ and ] to move cursor", PMENU_ALIGN_CENTER },
    { "Press Enter to select", PMENU_ALIGN_CENTER },
    { "*"VERSION, PMENU_ALIGN_RIGHT }
};

void Cmd_Menu_f( edict_t *ent ) {
    if( ent->client->layout == LAYOUT_MENU ) {
        PMenu_Close( ent );
        return;
    }

    PMenu_Open( ent, main_menu );

    switch( ent->client->pers.connected ) {
    case CONN_PREGAME:
    case CONN_SPECTATOR:
        ent->client->menu.entries[3].text = "*Enter the game";
        break;
    case CONN_SPAWNED:
        ent->client->menu.entries[3].text = "*Leave the game";
        break;
    default:
        return;
    }
}

/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
static void Cmd_Score_f (edict_t *ent) {
    if (ent->client->layout == LAYOUT_SCORES) {
        ent->client->layout = 0;
        return;
    }

    ent->client->layout = LAYOUT_SCORES;
    DeathmatchScoreboardMessage (ent, qtrue);
}

static void Cmd_OldScore_f (edict_t *ent) {
    if (ent->client->layout == LAYOUT_OLDSCORES) {
        ent->client->layout = 0;
        return;
    }

    if( !game.oldscores[0] ) {
        gi.cprintf( ent, PRINT_HIGH, "There is no old scoreboard yet.\n" );
        return;
    }

    ent->client->layout = LAYOUT_OLDSCORES;

    gi.WriteByte (svc_layout);
    gi.WriteString (game.oldscores);
    gi.unicast (ent, qtrue);
}

/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
    char    *cmd;

    if (!ent->client)
        return;     // not fully in game yet

    if( ent->client->pers.connected <= CONN_CONNECTED ) {
        return;
    }

    //ent->client->resp.activity_framenum = level.framenum;

    cmd = gi.argv(0);

    if( ent->client->pers.admin ) {
        if (Q_stricmp (cmd, "mute") == 0) {
            Cmd_Mute_f (ent, qtrue);
            return;
        }
        if (Q_stricmp (cmd, "unmute") == 0) {
            Cmd_Mute_f (ent, qfalse);
            return;
        }
        if (Q_stricmp (cmd, "muteall") == 0) {
            Cmd_MuteAll_f (ent, qtrue);
            return;
        }
        if (Q_stricmp (cmd, "unmuteall") == 0) {
            Cmd_MuteAll_f (ent, qfalse);
            return;
        }
        if (Q_stricmp (cmd, "ban") == 0) {
            G_AddIP_f( ent );
            return;
        }
        if (Q_stricmp (cmd, "unban") == 0) {
            G_RemoveIP_f( ent );
            return;
        }
        if (Q_stricmp (cmd, "bans") == 0) {
            G_ListIP_f( ent );
            return;
        }
        if (Q_stricmp (cmd, "kick") == 0 || Q_stricmp (cmd, "boot") == 0) {
            Cmd_Kick_f( ent, qfalse );
            return;
        }
        if (Q_stricmp (cmd, "kickban") == 0) {
            Cmd_Kick_f( ent, qtrue );
            return;
        }
        if (Q_stricmp(cmd, "acommands") == 0) {
            Cmd_AdminCommands_f(ent);
            return;
        }
    }

    if (Q_stricmp (cmd, "say") == 0) {
        Cmd_Say_f (ent, CHAT_ALL);
        return;
    }
    if (Q_stricmp (cmd, "say_team") == 0) {
        Cmd_Say_f (ent, CHAT_TEAM);
        return;
    }
    if (Q_stricmp (cmd, "players") == 0 || Q_stricmp (cmd, "playerlist") == 0) {
        Cmd_Players_f (ent);
        return;
    }
    if (Q_stricmp (cmd, "highscore") == 0 || Q_stricmp (cmd, "highscores") == 0) {
        Cmd_HighScores_f (ent);
        return;
    }
    if (Q_stricmp(cmd, "stats") == 0 || Q_stricmp(cmd, "accuracy") == 0) {
        Cmd_Stats_f(ent, qtrue);
        return;
    }
    if (Q_stricmp(cmd, "settings") == 0 || Q_stricmp(cmd, "matchinfo") == 0) {
        Cmd_Settings_f (ent);
        return;
    }
    if (Q_stricmp(cmd, "admin") == 0 || Q_stricmp(cmd, "referee") == 0) {
        Cmd_Admin_f(ent);
        return;
    }
    if (Q_stricmp(cmd, "commands") == 0) {
        Cmd_Commands_f(ent);
        return;
    }
    if (Q_stricmp(cmd, "id") == 0) {
        Cmd_Id_f(ent);
        return;
    }

    if (level.intermission_framenum)
        return;

    if (Q_stricmp (cmd, "score") == 0 || Q_stricmp (cmd, "help") == 0)
        Cmd_Score_f (ent);
    else if (Q_stricmp (cmd, "oldscore") == 0 || Q_stricmp (cmd, "oldscores") == 0 ||
             Q_stricmp (cmd, "lastscore") == 0 || Q_stricmp (cmd, "lastscores") == 0)
        Cmd_OldScore_f (ent);
    else if (Q_stricmp (cmd, "use") == 0)
        Cmd_Use_f (ent);
    else if (Q_stricmp (cmd, "drop") == 0)
        Cmd_Drop_f (ent);
    else if (Q_stricmp (cmd, "give") == 0)
        Cmd_Give_f (ent);
    else if (Q_stricmp (cmd, "god") == 0)
        Cmd_God_f (ent);
    else if (Q_stricmp (cmd, "notarget") == 0)
        Cmd_Notarget_f (ent);
    else if (Q_stricmp (cmd, "noclip") == 0)
        Cmd_Noclip_f (ent);
    else if (Q_stricmp (cmd, "inven") == 0)
        Cmd_Inven_f (ent);
    else if (Q_stricmp (cmd, "invnext") == 0)
        SelectNextItem (ent, -1);
    else if (Q_stricmp (cmd, "invprev") == 0)
        SelectPrevItem (ent, -1);
    else if (Q_stricmp (cmd, "invnextw") == 0)
        SelectNextItem (ent, IT_WEAPON);
    else if (Q_stricmp (cmd, "invprevw") == 0)
        SelectPrevItem (ent, IT_WEAPON);
    else if (Q_stricmp (cmd, "invnextp") == 0)
        SelectNextItem (ent, IT_POWERUP);
    else if (Q_stricmp (cmd, "invprevp") == 0)
        SelectPrevItem (ent, IT_POWERUP);
    else if (Q_stricmp (cmd, "invuse") == 0)
        Cmd_InvUse_f (ent);
    else if (Q_stricmp (cmd, "invdrop") == 0)
        Cmd_InvDrop_f (ent);
    else if (Q_stricmp (cmd, "weapprev") == 0)
        Cmd_WeapPrev_f (ent);
    else if (Q_stricmp (cmd, "weapnext") == 0)
        Cmd_WeapNext_f (ent);
    else if (Q_stricmp (cmd, "weaplast") == 0)
        Cmd_WeapLast_f (ent);
    else if (Q_stricmp (cmd, "kill") == 0)
        Cmd_Kill_f (ent);
    else if (Q_stricmp (cmd, "putaway") == 0)
        Cmd_PutAway_f (ent);
    else if (Q_stricmp (cmd, "wave") == 0)
        Cmd_Wave_f (ent);
    else if (Q_stricmp(cmd, "observe") == 0 || Q_stricmp(cmd, "spectate") == 0 ||
             Q_stricmp(cmd, "spec") == 0 || Q_stricmp(cmd, "obs") == 0 ||
             Q_stricmp(cmd, "observer") == 0 || Q_stricmp(cmd, "spectator") == 0 )
        Cmd_Observe_f(ent);
    else if (Q_stricmp(cmd, "chase") == 0)
        Cmd_Chase_f(ent);
    else if (Q_stricmp(cmd, "join") == 0)
        Cmd_Join_f(ent);
    else if (Q_stricmp(cmd, "vote") == 0 || Q_stricmp(cmd, "callvote") == 0)
        Cmd_Vote_f(ent);
    else if (Q_stricmp(cmd, "yes") == 0 && level.vote.proposal)
        Cmd_CastVote_f(ent, qtrue);
    else if (Q_stricmp(cmd, "no") == 0 && level.vote.proposal)
        Cmd_CastVote_f(ent, qfalse);
    else if (Q_stricmp(cmd, "menu") == 0)
        Cmd_Menu_f(ent);
    else    // anything that doesn't match a command will be a chat
        Cmd_Say_f (ent, CHAT_MISC);
}

