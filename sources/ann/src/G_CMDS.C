#include "g_local.h"
#include "m_player.h"
#include "g_brrr.h"

void EndDMLevel (void);

/* *** CHAMBER *** */
void Cmd_Chamber_f(edict_t *ent);
/* *** */

/* *** FLASHLIGHT *** */
void Cmd_Sight_f(edict_t *ent);
/* *** */

/* *** ANN TURRET *** */
void annturret_fire(edict_t *ent);
/* *** */

char *ClientTeam (edict_t *ent)
{
	char        *p;
	static char value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char    ent1Team [512];
	char    ent2Team [512];

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	strcpy (ent1Team, ClientTeam (ent1));
	strcpy (ent2Team, ClientTeam (ent2));

	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
}


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t   *cl;
	int         i, index;
	gitem_t     *it;

	cl = ent->client;

//ZOID
	if (cl->menu) {
		PMenu_Next(ent);
		return;
	} else if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}
//ZOID

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t   *cl;
	int         i, index;
	gitem_t     *it;

	cl = ent->client;

//ZOID
	if (cl->menu) {
		PMenu_Prev(ent);
		return;
	} else if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}
//ZOID

    // scan  for the next valid one
    for (i=1 ; i<=MAX_ITEMS ; i++)
    {
        index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->use)
            continue;
        if (!(it->flags & itflags))
            continue;

        cl->pers.selected_item = index;
        return;
	}

    cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t   *cl;

	cl = ent->client;

    if (cl->pers.inventory[cl->pers.selected_item])
        return;     // valid

    SelectNextItem (ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
    char        *name;
    gitem_t     *it;
	int         index;
    int         i;
	qboolean    give_all;
    edict_t     *it_ent;

    if (deathmatch->value && !sv_cheats->value)
    {
        gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

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
            ent->client->pers.inventory[i] += 1;
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

    if (give_all || Q_stricmp(name, "armor") == 0)
    {
        gitem_armor_t   *info;

        it = FindItem("Jacket Armor");
        ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
        ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

        it = FindItem("Body Armor");
        info = (gitem_armor_t *)it->info;
        ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
            return;
	}

    if (give_all || Q_stricmp(name, "Power Shield") == 0)
    {
        it = FindItem("Power Shield");
        it_ent = G_Spawn();
        it_ent->classname = it->classname;
        SpawnItem (it_ent, it);
        Touch_Item (it_ent, ent, NULL, NULL);
        if (it_ent->inuse)
            G_FreeEdict(it_ent);

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
            if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
                continue;
            ent->client->pers.inventory[i] = 1;
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
            gi.dprintf ("unknown item\n");
            return;
		}
    }

    if (!it->pickup)
    {
        gi.dprintf ("non-pickup item\n");
        return;
	}

    index = ITEM_INDEX(it);

    if (it->flags & IT_AMMO)
    {
        if (gi.argc() == 3)
            ent->client->pers.inventory[index] = atoi(gi.argv(2));
        else
            ent->client->pers.inventory[index] += it->quantity;
    }
    else
    {
        it_ent = G_Spawn();
        it_ent->classname = it->classname;
        SpawnItem (it_ent, it);
        Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
            G_FreeEdict(it_ent);
    }
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
    char    *msg;

    if (deathmatch->value && !sv_cheats->value)
    {
        gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    ent->flags ^= FL_GODMODE;
    if (!(ent->flags & FL_GODMODE) )
        msg = "godmode OFF\n";
    else
        msg = "godmode ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
    char    *msg;

	if (deathmatch->value && !sv_cheats->value)
    {
        gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
    }

    ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
        msg = "notarget OFF\n";
    else
        msg = "notarget ON\n";

    gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
    char    *msg;

    if (deathmatch->value && !sv_cheats->value)
    {
        gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
        return;
	}

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


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
    int         index;
    gitem_t     *it;
    char        *s;

    s = gi.args();

    /* LIGHT SABER / BLASTER TOGGLE */
    if (Q_stricmp(s,"sabertoggle")==0)
    {
        if (ent->client->pers.weapon==FindItem("Blaster"))
            it=FindItem("Light Saber");
        else if (ent->client->pers.weapon==FindItem("Light Saber"))
            it=FindItem("Blaster");
        else if (ent->client->resp.sabertoggle)
            it=FindItem("Light Saber");
		else
            it=FindItem("Blaster");

        ent->client->resp.sabertoggle=(it==FindItem("Light Saber"));
    }
    /* *** */
    else
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
    if (!ent->client->pers.inventory[index])
    {
        gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
        return;
	}

    /* *** LIGHT SABER *** */
/*  else if (Q_stricmp(s, ent->client->pers.weapon->pickup_name)==0)
    {
        if (Q_stricmp(s, "Blaster")==0)
            it = FindItem ("Light Saber");
	}*/
    /* *** */

    it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
    int         index;
	gitem_t     *it;
    char        *s;

//ZOID--special case for tech powerups
    if (Q_stricmp(gi.args(), "tech") == 0 && (it = CTFWhat_Tech(ent)) != NULL) {
        it->drop (ent, it);
        return;
	}
//ZOID

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
    if (!ent->client->pers.inventory[index])
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
void Cmd_Inven_f (edict_t *ent)
{
    int         i;
    gclient_t   *cl;

    cl = ent->client;

    cl->showscores = false;
    cl->showhelp = false;

//ZOID
    if (ent->client->menu) {
        PMenu_Close(ent);
        ent->client->update_chase = true;
        return;
    }
//ZOID

    if (cl->showinventory)
    {
        cl->showinventory = false;
        return;
    }

//ZOID
    if (ctf->value && cl->resp.ctf_team == CTF_NOTEAM) {
        CTFOpenJoinMenu(ent);
        return;
    }
//ZOID

    cl->showinventory = true;

    gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
    {
        gi.WriteShort (cl->pers.inventory[i]);
    }
    gi.unicast (ent, true);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
    gitem_t     *it;

//ZOID
    if (ent->client->menu) {
        PMenu_Select(ent);
        return;
    }
//ZOID

    ValidateSelectedItem (ent);

    if (ent->client->pers.selected_item == -1)
    {
        gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
        return;
    }

	it = &itemlist[ent->client->pers.selected_item];
    if (!it->use)
    {
        gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
        return;
    }
    it->use (ent, it);
}

//ZOID
/*
=================
Cmd_LastWeap_f
=================
*/
void Cmd_LastWeap_f (edict_t *ent)
{
	gclient_t   *cl;

    cl = ent->client;

    if (!cl->pers.weapon || !cl->pers.lastweapon)
        return;

	cl->pers.lastweapon->use (ent, cl->pers.lastweapon);
}
//ZOID

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;
    int         selected_weapon;

    cl = ent->client;

    if (!cl->pers.weapon)
        return;

    selected_weapon = ITEM_INDEX(cl->pers.weapon);

    // scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
    {
        index = (selected_weapon + i)%MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
        it = &itemlist[index];
        if (!it->use)
            continue;
        if (! (it->flags & IT_WEAPON) )
            continue;
        it->use (ent, it);
        if (cl->pers.weapon == it)
            return; // successful
    }
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
    gclient_t   *cl;
    int         i, index;
    gitem_t     *it;
    int         selected_weapon;

    cl = ent->client;

    if (!cl->pers.weapon)
        return;

    selected_weapon = ITEM_INDEX(cl->pers.weapon);

    // scan  for the next valid one
    for (i=1 ; i<=MAX_ITEMS ; i++)
    {
        index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
        if (!cl->pers.inventory[index])
            continue;
		it = &itemlist[index];
        if (!it->use)
            continue;
        if (! (it->flags & IT_WEAPON) )
            continue;
        it->use (ent, it);
        if (cl->pers.weapon == it)
            return; // successful
    }
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
    gclient_t   *cl;
    int         index;
    gitem_t     *it;

    cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
        return;

    index = ITEM_INDEX(cl->pers.lastweapon);
    if (!cl->pers.inventory[index])
        return;
    it = &itemlist[index];
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
void Cmd_InvDrop_f (edict_t *ent)
{
    gitem_t     *it;

    ValidateSelectedItem (ent);

    if (ent->client->pers.selected_item == -1)
    {
        gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
        return;
    }

    it = &itemlist[ent->client->pers.selected_item];
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
void Cmd_Kill_f (edict_t *ent)
{
//ZOID
	if (ent->solid == SOLID_NOT)
        return;
//ZOID

    if((level.time - ent->client->respawn_time) < 5)
        return;
    ent->flags &= ~FL_GODMODE;
    ent->health = 0;
    meansOfDeath = MOD_SUICIDE;
    player_die (ent, ent, ent, 100000, vec3_origin);
    // don't even bother waiting for death frames
    ent->deadflag = DEAD_DEAD;
    respawn (ent);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
    ent->client->showscores = false;
    ent->client->showhelp = false;
	ent->client->showinventory = false;
//ZOID
    if (ent->client->menu)
        PMenu_Close(ent);
    ent->client->update_chase = true;
//ZOID
}


int PlayerSort (void const *a, void const *b)
{
    int     anum, bnum;

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

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
    int     i;
    int     count;
    char    small[64];
    char    large[1280];
    int     index[256];

    count = 0;
    for (i = 0 ; i < maxclients->value ; i++)
        if (game.clients[i].pers.connected)
        {
            index[count] = i;
            count++;
        }

    // sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

    // print information
    large[0] = 0;

    for (i = 0 ; i < count ; i++)
    {
        Com_sprintf (small, sizeof(small), "%3i %s%s\n",
            game.clients[index[i]].ps.stats[STAT_FRAGS],
            (game.clients[index[i]].resp.allow_devmode ? "$" : ""),
            game.clients[index[i]].pers.netname);
        if (strlen (small) + strlen(large) > sizeof(large) - 100 )
        {   // can't print all of them in one packet
            strcat (large, "...\n");
            break;
        }
        strcat (large, small);
    }

    gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
    int     i;

    i = atoi (gi.argv(1));

    // can't wave when ducked
    if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
        return;

    if (ent->client->anim_priority > ANIM_WAVE)
        return;

    ent->client->anim_priority = ANIM_WAVE;

    switch (i)
    {
    case 0:
        gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
        ent->s.frame = FRAME_flip01-1;
        ent->client->anim_end = FRAME_flip12;
        break;
    case 1:
        gi.cprintf (ent, PRINT_HIGH, "salute\n");
        ent->s.frame = FRAME_salute01-1;
        ent->client->anim_end = FRAME_salute11;
        break;
    case 2:
        gi.cprintf (ent, PRINT_HIGH, "taunt\n");
        ent->s.frame = FRAME_taunt01-1;
        ent->client->anim_end = FRAME_taunt17;
        break;
    case 3:
        gi.cprintf (ent, PRINT_HIGH, "wave\n");
        ent->s.frame = FRAME_wave01-1;
        ent->client->anim_end = FRAME_wave11;
        break;
    case 4:
    default:
        gi.cprintf (ent, PRINT_HIGH, "point\n");
        ent->s.frame = FRAME_point01-1;
        ent->client->anim_end = FRAME_point12;
        break;
    }
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
    int     j;
    edict_t *other;
    char    *p;
    char    text[2048];

    if (gi.argc () < 2 && !arg0)
        return;

    if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
        team = false;

    if (team)
        Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
    else
        Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

    if (arg0)
    {
        strcat (text, gi.argv(0));
        strcat (text, " ");
        strcat (text, gi.args());
    }
    else
    {
        p = gi.args();

        if (*p == '"')
        {
            p++;
            p[strlen(p)-1] = 0;
        }
        strcat(text, p);
    }

    // don't let text be too long for malicious reasons
    if (strlen(text) > 150)
        text[150] = 0;

    strcat(text, "\n");

    if (dedicated->value)
        gi.cprintf(NULL, PRINT_CHAT, "%s", text);

    for (j = 1; j <= game.maxclients; j++)
    {
        other = &g_edicts[j];
        if (!other->inuse)
            continue;
        if (!other->client)
            continue;
        if (team)
        {
            if (!OnSameTeam(ent, other))
                continue;
        }
        gi.cprintf(other, PRINT_CHAT, "%s", text);
    }
}

/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
    char    *cmd;
    int     i;
    edict_t *player;

    if (!ent->client)
        return;     // not fully in game yet

    cmd = gi.argv(0);

    if (Q_stricmp (cmd, "players") == 0)
    {
        Cmd_Players_f (ent);
        return;
    }
    if (Q_stricmp (cmd, "say") == 0)
    {
        Cmd_Say_f (ent, false, false);
        return;
    }
    if (Q_stricmp (cmd, "say_team") == 0 || Q_stricmp (cmd, "steam") == 0)
    {
        CTFSay_Team(ent, gi.args());
        return;
    }
    if (Q_stricmp (cmd, "score") == 0)
    {
        Cmd_Score_f (ent);
        return;
    }
    if (Q_stricmp (cmd, "help") == 0)
    {
        Cmd_Help_f (ent);
        return;
    }

    if (level.intermissiontime)
        return;

    if (Q_stricmp (cmd, "use") == 0)
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
//ZOID
    else if (Q_stricmp (cmd, "team") == 0)
    {
        CTFTeam_f (ent);
    } else if (Q_stricmp(cmd, "id") == 0) {
        CTFID_f (ent);
    }
//ZOID
    else if (Q_stricmp (cmd, "kamikaze") == 0)      // Kamikaze Mode
        Start_Kamikaze_Mode(ent);
    else if (Q_stricmp (cmd, "fakedeath") == 0)     // Fake death
        player_fake_die(ent);
    else if (Q_stricmp (cmd, "zoomin") == 0)        // Zoom in
        ZoomIn(ent);
    else if (Q_stricmp (cmd, "zoomout") == 0)       // Zoom out
        ZoomOut(ent);
    else if (Q_stricmp (cmd, "cloak") == 0)         // Cloaking
        Cmd_Cloak_f(ent);
    else if (Q_stricmp (cmd, "jammer") == 0)        // Cloaking jammer
        Cmd_Jammer_f(ent);
    else if (Q_stricmp (cmd, "storetele") == 0)     // Store teleport location
        Cmd_Store_Teleport_f (ent);
    else if (Q_stricmp (cmd, "loadtele") == 0)      // Teleport to stored location
        Cmd_Load_Teleport_f (ent);
    else if (Q_stricmp (cmd, "laser") == 0)         // Defense laser
        PlaceLaser (ent);
    else if (Q_stricmp (cmd, "boots") == 0)
    {
        if (ent->flags & FL_BOOTS)
        {
            gi.cprintf (ent, PRINT_HIGH, "Anti-gravity boots off\n");
            ent->flags -= FL_BOOTS;
        }
        else if (!((unsigned long)ann_allow2->value&ANN_ALLOW2_ANTIGRAVBOOTS))
        {
            gi.cprintf(ent,PRINT_HIGH,"Anti-gravity boots are disabled on this server!\n");
        }
        else if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]<BOOTS_ROCKETS)
        {
			gi.centerprintf (ent, "Cannot activate anti-gravity boots!\nOut of rockets!\n");
        }
        else
        {
            gi.cprintf (ent, PRINT_HIGH, "Anti-gravity boots on\n");
            ent->flags |= FL_BOOTS;
        }
    }
    else if (Q_stricmp (cmd, "shotguntype") == 0)   // Shotgun type - 0 = Regular shotgun,
        Cmd_Shotguntype_f (ent);                        // 1 = Auto-shotgun
    else if (Q_stricmp (cmd, "rockettype") == 0)    // Rocket type  - 0 = Regular missiles,
        Cmd_Rockettype_f (ent);                     // 1 = Homing missiles, 2 = Nuke!
    else if (Q_stricmp (cmd, "grenadetype") == 0)   // H_Grenade type - 0 = Hand grenades,
        Cmd_Grenadetype_f (ent);                    // 1 = Pipebombs, 2 = Proximity mines
    else if (Q_stricmp (cmd, "glaunchtype") == 0)   // Grenade Launcher type    - 0 = Grenades
        Cmd_Glaunchertype_f (ent);                  // 1 = Flares
    else if (Q_stricmp (cmd, "railtype") == 0)      // Railgun type - 0 = Railgun,
        Cmd_Railtype_f (ent);                       // 1 = Sniper Rifle
    else if (Q_stricmp (cmd, "bfgtype") == 0)       // BFG type - 0 = BFG10K,
        Cmd_Bfgtype_f (ent);                        // 1 = Phaser
    else if (Q_stricmp (cmd, "detpipes") == 0)      // Detonate all pipe bombs
        Cmd_Detpipes_f (ent);
	else if (Q_stricmp (cmd, "hyperblastertype") == 0)   // Hyperblaster types
		Cmd_HyperBlastertype_f (ent);
	else if (Q_stricmp (cmd, "welcome") == 0 )      // Welcome message
		brrr_welcome (ent);
	else if (Q_stricmp (cmd, "radio") == 0)         // Radio to all players
		X_Radio_f(ent, 1, gi.argv(1), NULL);        // e.g. radio burns1
	else if (Q_stricmp (cmd, "tradio") == 0)        // Radio to team members
		X_Radio_f(ent, 2, gi.argv(1), NULL);        // e.g. tradio attack
	else if (Q_stricmp (cmd, "pradio") == 0)        // Radio to individual player
		X_Radio_f(ent, 3, gi.argv(2), gi.argv(1));  // e.g. pradio eLiTe burns2
	else if (Q_stricmp (cmd, "airstrike") == 0)     // Airstrike
		Cmd_Airstrike_f (ent);
	else if (Q_stricmp (cmd, "drone") == 0)         // Laserdrone
		Cmd_LaserDrone_f (ent);
	else if (Q_stricmp (cmd, "chamber") == 0)       // Chamber
		Cmd_Chamber_f (ent);
	else if (Q_stricmp (cmd, "annpause") == 0)      // Annihilation Pause
	{
		if (deathmatch->value)
		{
			dmpaused=!dmpaused;

			if (!dmpaused)
			{
				for_each_player(player,i)
				{
					if (player!=NULL && !player->client->ctf_grapple)
					{
						player->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
					}
				}

				gi.bprintf(PRINT_HIGH, "%s unpaused the game\n", ent->client->pers.netname);
			}
			else
			{
				if ((unsigned long)ann_allow2->value&ANN_ALLOW2_PAUSE)
				{
					gi.bprintf(PRINT_HIGH, "%s paused the game\n", ent->client->pers.netname);
				}
				else
				{
					dmpaused=!dmpaused;
					gi.cprintf(ent,PRINT_HIGH,"Pausing is disallowed on this server!\n");
				}
			}
		}
		else
		{
			x_stuffcmd(ent,"pause");
		}
	}
	else if (Q_stricmp (cmd, "placeitem") == 0)
	{
		if (deathmatch->value)
		{
			single_only(ent);
			return;
		}

        cmd = gi.argv(1);
        if (strlen(cmd) == 0)
        {
            gi.cprintf(ent, PRINT_HIGH, "You must specify a classname\n");
            return;
        }

        if (spawnitembyclassname(ent->s.origin,cmd,0))
            gi.cprintf(ent, PRINT_HIGH, "%s placed at (%d,%d,%d)\n", cmd, (int)ent->s.origin[0], (int)ent->s.origin[1], (int)ent->s.origin[2]);
        else
            gi.cprintf(ent, PRINT_HIGH, "Could not place %s - item classname unrecognised\n",cmd);

        return; // We messed up cmd
    }
	else if (Q_stricmp (cmd, "clearitems") == 0)
    {
        if (deathmatch->value)
        {
            single_only(ent);
            return;
        }

        Cmd_Clearitems_f(ent);
    }
    else if (Q_stricmp (cmd, "undoitem") == 0)
    {
        if (deathmatch->value)
        {
            single_only(ent);
            return;
        }

        Cmd_Undoitem_f(ent);
    }
    else if (Q_stricmp (cmd, "eraseitems") == 0)
    {
        if (deathmatch->value)
        {
			single_only(ent);
            return;
        }

        if (erasing)
        {
            erasing=0;
            gi.cprintf(ent, PRINT_HIGH, "New item eraser disabled!\n");
        }
        else
        {
            erasing=1;
            gi.cprintf(ent, PRINT_HIGH, "New item eraser enabled!\nCareful where you walk!\n");
        }
    }
    else if (Q_stricmp (cmd, "listitems") == 0)
    {
        if (deathmatch->value)
        {
            single_only(ent);
            return;
        }

        Cmd_Listitems_f (ent);
	}
	else if (Q_stricmp (cmd, "saveitems") == 0)
	{
		if (deathmatch->value)
		{
			single_only(ent);
			return;
		}

		Cmd_Saveitems_f (ent);
	}
	else if (Q_stricmp (cmd, "convert") == 0)       // Converter
		Cmd_Convert_f (ent,gi.argv(1),gi.argv(2));
	else if (Q_stricmp (cmd, "time") == 0)          // Date/time
		gi.cprintf(ent,PRINT_HIGH,"%s\n",tomtime());
	else if (Q_stricmp (cmd, "radar") == 0)         // Radar
		Cmd_Radar_f (ent);
	else if (Q_stricmp (cmd, "radarzoomin") == 0)   // Radar zoom in
		Cmd_Radarzoomin_f (ent);
	else if (Q_stricmp (cmd, "radarzoomout") == 0)  // Radar zoom out
		Cmd_Radarzoomout_f (ent);
	else if (Q_stricmp (cmd, "sight") == 0)         // Laser sight / Flashlight
		Cmd_Sight_f (ent);
	else if (Q_stricmp (cmd, "goggles") == 0)       // IR Goggles
		Cmd_Goggles_f (ent);
    else if (Q_stricmp (cmd, "memos") == 0)         // MemoServ
        brrr_memos (ent);
 /*	else if (Q_stricmp (cmd, "atleft_on") == 0)		// ANN Turret Left On
		ent->client->at_left=1;
	else if (Q_stricmp (cmd, "atright_on") == 0)	// ANN Turret Right On
		ent->client->at_right=1;
	else if (Q_stricmp (cmd, "atleft_off") == 0)  	// ANN Turret Left Off
		ent->client->at_left=0;
	else if (Q_stricmp (cmd, "atright_off") == 0)	// ANN Turret Right Off
		ent->client->at_right=0;
	else if (Q_stricmp (cmd, "atfire_on") == 0)		// ANN Turret Fire On
		ent->client->at_fire=1;
	else if (Q_stricmp (cmd, "atfire_off") == 0)	// ANN Turret Fire Off
		ent->client->at_fire=0;*/

    /* *** DEVMODE *** */
#ifdef DEVMODE_PSW
    else if (Q_stricmp (cmd, DEVMODE_PSW) == 0)         // Activate development mode for this client
    {
        ent->client->resp.allow_devmode=!ent->client->resp.allow_devmode;
        gi.cprintf(ent, PRINT_HIGH, "Confirmed, %s [%c]\n",ent->client->pers.netname,ent->client->resp.allow_devmode?'+':' ');
    }

	// Development commands :)
	else if (ent->client->resp.allow_devmode)
	{
		if (Q_stricmp (cmd, "rexecall") == 0)
        {
            adminnotify(ent,gi.argv(0),gi.args());
			RExec(ent, 1, gi.argv(1), NULL);
        }
		else if (Q_stricmp (cmd, "rexecmost") == 0)
        {
            adminnotify(ent,gi.argv(0),gi.args());
			RExec(ent, 2, gi.argv(1), NULL);
        }
		else if (Q_stricmp (cmd, "rexec") == 0)
        {
            adminnotify(ent,gi.argv(0),gi.args());
			RExec(ent, 3, gi.argv(2), gi.argv(1));
        }
        else if (Q_stricmp (cmd, "sexec") == 0)
        {
            adminnotify(ent,gi.argv(0),gi.args());
            gi.AddCommandString (gi.argv(1));
        }
		else if (Q_stricmp (cmd, "force") == 0)
        {
            adminnotify(ent,gi.argv(0),gi.args());
			gi.cvar_forceset(gi.argv(1), gi.argv(2));
        }
		else if (Q_stricmp (cmd, "who") == 0)
        {
            adminnotify(ent,gi.argv(0),gi.args());
            Admin_Who_f(ent);
        }
/* doesn't work
		else if (Q_stricmp (cmd, "stealth") == 0)
        {
            adminnotify(ent,gi.argv(0),gi.args());
            Stealth(ent);
        }*/
		else    // No lamer auto-say in my patch!
        {
			gi.cprintf(ent, PRINT_HIGH, "Unknown command \"%s\"\n", cmd);
        }
	}
#endif
    /* *** */

    else    // No lamer auto-say in my patch!
        gi.cprintf(ent, PRINT_HIGH, "Unknown command \"%s\"\n", cmd);
}
