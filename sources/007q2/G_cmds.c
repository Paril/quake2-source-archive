#include "g_local.h"
#include "m_player.h"
//ExtraMod: include q_extra.h (1 line)
//#include "q_extra.h"
//#include "scanner.h"

void	Cmd_PlayMidi_f (edict_t *ent);
void	MidiPause (edict_t *ent);
void	MidiResume (edict_t *ent);
void	Cmd_StopMidi_f ();
void	(*Cmd_PauseMidi_f) (edict_t *ent);
void	(*Cmd_ResumeMidi_f) (edict_t *ent);
void	MidiNotPlaying (edict_t *ent);
//extern cvar_t	*muzzleon;

	//+BD local declaration of our new command function
	void Cmd_Reload_f (edict_t *ent);
	//+BD end add

char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

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
	char	ent1Team [512];
	char	ent2Team [512];

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
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

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

	if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}

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
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

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

	if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}

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
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

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
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

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
		gitem_armor_t	*info;

//		it = FindItem("Jacket Armor");
//		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

/*		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;
*/
		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}

/*	if (give_all || Q_stricmp(name, "Power Shield") == 0)
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
*/
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
	char	*msg;

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
	char	*msg;

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
	char	*msg;

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
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
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
    //added 1-13-98 by Dan Eisner
        //to allow multiple weapons from one keypress

    else if (!Q_stricmp(s, ent->client->pers.weapon->pickup_name)) 
    {    
        if (!Q_stricmp(s, "PP7"))
		{
            it = FindItem ("Knife");
		}
		else if (!Q_stricmp(s, "Knife"))
		{
			it = FindItem ("Slapper");
		}
		else if (!Q_stricmp(s, "Slapper"))
		{
			it = FindItem ("PP7");
		}
		else
		{
			it = FindItem ("Slapper");
		}
		
// IoN
//     }  else if (!Q_stricmp(s, "Grenade Launcher"))    {
//            it = FindItem ("Proximity Mines");
// }
// IoN
//       }  else if (!Q_stricmp(s, "Shotgun"))    {
//            it = FindItem ("SuperShotgun");
//       }

    }

        //end added portion

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
	int			index;
	gitem_t		*it;
	char		*s;

/*ZOID--special case for tech powerups
	if (ctf->value)
	{
		if (Q_stricmp(gi.args(), "tech") == 0 && (it = CTFWhat_Tech(ent)) != NULL)
		{
		it->drop (ent, it);
		return;
		}
	}
*///ZOID


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
	int			i;
	gclient_t	*cl;

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

/*ZOID
	if (ctf->value && cl->resp.ctf_team == CTF_NOTEAM) {
		CTFOpenJoinMenu(ent);
		return;
	}
//ZOID*/

//ION
	if (teamdm->value && cl->resp.teamdm_team == TEAMDM_NOTEAM && !ent->isabot) {
		TEAMDMOpenJoinMenu(ent);
		return;
	}
	else if (teamdm->value && cl->resp.teamdm_team == TEAMDM_NOTEAM && ent->isabot)
	{
		TEAMDMAssignBotTeam(ent->client);
		return;
	}
//ION

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);

// scanner start
//	if (cl->pers.scanner_active & 1)
//		cl->pers.scanner_active = 2;
// scanner end


}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

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
	gclient_t	*cl;

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
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

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
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

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
			return;	// successful
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

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
	gitem_t		*it;

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
	int		anum, bnum;

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
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

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
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
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
	int		i;

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
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;

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

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
//   bot bugfox
			gi_cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
// changed . to _ :)
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->value)
//    bot bugfix
		gi_cprintf(NULL, PRINT_CHAT, "%s", text);

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
//  bot bugfix
	gi_cprintf(other, PRINT_CHAT, "%s", text);
	}
}

void Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;

	// connect time, ping, score, name
	*text = 0;
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
		if (!e2->inuse)
			continue;

		Com_sprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			e2->client->pers.netname,
			e2->client->resp.spectator ? " (spectator)" : "");
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}

/*
=================
Cmd_ThirdpersonToggle

=================

void Cmd_ThirdpersonToggle (edict_t *ent)
{
//	if(cl_3dcam->value)
//		cl_3dcam = 0;
//	else
		cl_3dcam->value = true;
}
*/
/*
=================
Cmd_Detonate_f

=================
*/
void Cmd_Detonate_f (edict_t *ent)
{
	edict_t	*blip = NULL;

	while ((blip = findradius(blip, ent->s.origin, 20000)) != NULL)
	{
		if (!strcmp(blip->classname, "remote") && blip->owner == ent)
		{
			blip->think = Remote_Explode;
			blip->nextthink = level.time + .1;
		}
	}
}

// +DL - Reload Ammo

void Cmd_ReloadAmmo(edict_t *ent)
{
    // delay
	int ClipSize;

//+ ION FIX
	if ((ent->client->pers.weapon == FindItem ("Slapper")) ||
		(ent->client->pers.weapon == FindItem ("Grenades")) ||
		(ent->client->pers.weapon == FindItem ("Proximity Mines")) ||
		(ent->client->pers.weapon == FindItem ("Laser")) ||
 		(ent->client->pers.weapon == FindItem ("Remote Mines")))
		return;
//-ION FIX

 	if(ent->deadflag == DEAD_DEAD)
	return;

	if (ent->client->pers.weapon == FindItem ("PP7"))
		ClipSize = 7;

//	if (ent->client->pers.weapon == FindItem ("PP7 Silenced"))
//		ClipSize = 7;

/*	if (ent->client->pers.weapon == FindItem ("Grenades"))
		ClipSize = 1;
*/
	if (ent->client->pers.weapon == FindItem ("ZMG 9mm"))
		ClipSize = 32;

	if (ent->client->pers.weapon == FindItem ("Dual ZMG 9mm's"))
		ClipSize = 64;

	if (ent->client->pers.weapon == FindItem ("KF7 Soviet"))
		ClipSize = 30;

	if (ent->client->pers.weapon == FindItem ("AR33 Assault Rifle"))
		ClipSize = 30;

	if (ent->client->pers.weapon == FindItem ("Rocket Launcher"))
		ClipSize = 1;
/*
	if (ent->client->pers.weapon == FindItem ("Remote Mines"))
		ClipSize = 1;

	if (ent->client->pers.weapon == FindItem ("Proximity Mines"))
		ClipSize = 1;
*/
	if (ent->client->pers.weapon == FindItem ("RCP-90"))
		ClipSize = 80;	
	
	if (ent->client->ammo_clip == ClipSize)
	return;

	ent->client->weaponstate = WEAPON_RELOADING;

	// Unload Ammo
	ent->client->pers.inventory[ent->client->ammo_index] += ent->client->ammo_clip;
	ent->client->ammo_clip = 0;

	// Load Ammo
	LoadAmmo(ent);

	// Redisplay Weapon
//	if (!ent->client->chasecam){
	if (!(ent->client->weaponstate == WEAPON_RELOADING))
	{
		ent->client->ps.gunframe = 0;
		ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
	}
//}

	ent->client->anim_priority = ANIM_PAIN;
	if(ent->client->ps.pmove.pm_flags & PMF_DUCKED)
	{
			ent->s.frame = FRAME_crpain1;
			ent->client->anim_end = FRAME_crpain4;
	}
	else
	{
			ent->s.frame = FRAME_pain301;
			ent->client->anim_end = FRAME_pain304;
			
	}
}

/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;		// not fully in game yet

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
	if (Q_stricmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
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
// scanner start
//	else if (Q_stricmp (cmd, "scanner") == 0)
//		Toggle_Scanner (ent);
// scanner end
//ZOID
	else if (Q_stricmp (cmd, "team") == 0)
	{
//		CTFTeam_f (ent);
		TEAMDMTeam_f (ent);
	} else if (Q_stricmp(cmd, "id") == 0) {
//		CTFID_f (ent);
		TEAMDMID_f (ent);
	}
//ZOID

	// ion new detonate
	else if (Q_stricmp (cmd, "detonate") == 0)
		Cmd_Detonate_f (ent);
	//+BD - for handling reload commands
//  	else if (Q_stricmp (cmd, "reload") == 0)
//     		 Cmd_Reload_f (ent);
  	else if (Q_stricmp (cmd, "reload") == 0)
    		 Cmd_ReloadAmmo (ent);
	else if (Q_stricmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	else if (Q_stricmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);

/* spaz
	else if (Q_stricmp(cmd, "thirdperson") == 0)
		Cmd_ThirdpersonToggle(ent);
// spaz */

//	else if (Q_stricmp (cmd, "cloak") == 0)
//		Cmd_Cloak_f (ent);
 
//        else if (Q_stricmp (cmd, "hud") == 0)
//              Cmd_ToggleHud();
        //END                  */
//ExtraMod: add the commands (8 lines)
//	else if (Q_stricmp (cmd, "playmidi") == 0)
//		Cmd_PlayMidi_f (ent);
//	else if (Q_stricmp(cmd, "pausemidi") == 0)
//		Cmd_PauseMidi_f (ent);
//	else if (Q_stricmp(cmd, "resumemidi") == 0)
//		Cmd_ResumeMidi_f (ent);
//	else if (Q_stricmp(cmd, "stopmidi") == 0)
//		Cmd_StopMidi_f ();
//	else if (Q_stricmp(cmd, "stopmp3") == 0)
//		StopMP3 (ent);
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}

	//+BD ENTIRE CODE BLOCK NEW
	// Cmd_Reload_f()
	// Handles weapon reload requests
/*	void Cmd_Reload_f (edict_t *ent)
	{
   		int rds_left;           //+BD - Variable to handle rounds left

  		 //+BD - If the player is dead, don't bother
  	 	if(ent->deadflag == DEAD_DEAD)
   		{
     		 	gi.centerprintf(ent, "I know you're a hard ass,\nBUT YOU'RE DEAD!!\n");
      			return;
  		 }

   		//First, grab the current magazine max count...
  		if(stricmp(ent->client->pers.weapon->pickup_name, "ZMG 9mm") == 0)
      			rds_left = ent->client->ZMG_max;
		else    //We should never get here, but...
  			 //BD 5/26 - Actually we get here quite often right now. Just exit for weaps that we
   			//          don't want reloaded or that never reload (grenades)
  	 	{
      			gi.centerprintf(ent,"Where'd you train?\nYou can't reload that!\n");
      			return;
   		}

   		if(ent->client->pers.inventory[ent->client->ammo_index])
   		{       
      			if((ent->client->weaponstate != WEAPON_END_MAG) && (ent->client->pers.inventory[ent->client->ammo_index] < rds_left))
      			{
            				gi.centerprintf(ent,"Buy a clue-\nYou're on your last magazine!\n");
      			}
   			else
   				//Set the weaponstate...
      				ent->client->weaponstate = WEAPON_RELOADING;
  	 	}
   		else
      			gi.centerprintf(ent,"Pull your head out-\nYou've got NO AMMO!\n");
	}
	//+BD END CODE BLOCK
*/