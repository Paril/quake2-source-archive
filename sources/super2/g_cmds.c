#include "g_local.h"
#include "m_player.h"

// SH
#include "s_defs.h"

extern	admin_t admin;
extern int body_armor_index;
vec3_t v_forward;
vec3_t v_right;
vec3_t v_up;
int	invis_index, cripple_index, robot_index, sun_index;
// \SH

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
	
	if (ctf->value)
	{
		return CTFSameTeam(ent1, ent2);
	}

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
// SH
	if (SuperCommand(ent, s))
	{
		return;
	}
// \SH

	if (ent->flags & FL_OBSERVER)
	{
		return;
	}

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
// SH
	if (ent->flags & FL_OBSERVER || ent->drop_wait > level.time)
		return;
// \SH
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
	ent->drop_wait = level.time + 5; // SH
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
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

	if (ent->flags & FL_OBSERVER)
		return;

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

	if (ent->flags & FL_OBSERVER)
		return;

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

	if (ent->flags & FL_OBSERVER)
		return;

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

	if (ent->flags & FL_OBSERVER)
		return;

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

// SH
	if (ent->flags & FL_OBSERVER || ent->drop_wait > level.time)
		return;
// \SH

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
	ent->drop_wait = level.time + 5; // SH
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
	if (ent->flags & FL_OBSERVER)
		return;

	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
	// don't even bother waiting for death frames
//	ent->deadflag = DEAD_DEAD;
//	respawn (ent);
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

	if (ent->flags & FL_OBSERVER)
		return;

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

void target_laser_off (edict_t *);

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		j;
	edict_t	*other;
	char	*p;
	char	text[2048];

	if (gi.argc () < 2 && !arg0)
		return;

	if ( !( (int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS) ) && !(ctf->value) )
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

// SH
//gi.bprintf (PRINT_HIGH, p);

	if (Q_strncasecmp(p, "the sun is a mass of incandescent gas", 37) == 0)
	{
	
		if (ent->client->pers.combo == C_SUN)
		{
			gi.cprintf (ent, PRINT_HIGH, "You already are The Sun!\n");
		}
		else if (ent->client->resp.score >= 10)
		{
		
			gi.cprintf (ent, PRINT_HIGH, "You are the Sun!\n");
			ent->client->pers.active = SUN;
			ent->client->pers.passive = SUN;
			ent->client->pers.special = SUN;
			ent->client->pers.combo = C_SUN;
			ent->client->resp.score = 0;
			ent->flags &= ~(FL_RANDOM_ACT|FL_RANDOM_PAS|FL_RANDOM_SPE);
		
			sun_index = gi.modelindex ("models/super2/sun/sun.md2");
	
			if (ent->teamchain)
			{
				target_laser_off (ent->teamchain);
				G_FreeEdict (ent->teamchain);
				ent->flags &= ~FL_OPTIC;
				ent->teamchain = NULL;
			}
			
			if (ent->angel != NULL)
			{
				G_FreeEdict (ent->angel);
				ent->angel = NULL;
			}
	
	
			PutClientInServer (ent);
	
			gi.sound(ent, CHAN_AUTO, gi.soundindex("powers/sun3.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.cprintf (ent, PRINT_HIGH, "You're not hot enough to be The Sun!\n");
		}
	}
// \SH
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
	else if (Q_stricmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	else if (Q_stricmp (cmd, "gameversion") == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "%s : %s\n", GAMEVERSION, __DATE__);
	}
	else if (Q_stricmp (cmd, "fov") == 0)
	{
		ent->client->ps.fov = atoi(gi.argv(1));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}

////////////////////////////////// SH

char * AName (int i)
{
	if (i == 0)
		return "None";
	else if (i == MORTAL)
		return "Mere Mortal";
	else if (i == ROBOT)
		return "Killer Robot";
	else if (i == CRIP)
		return "Cripple w/ a Big Gun";
	else if (i == MAGE)
		return "Archmage";
	else if (i == PUNISH)
		return "Punisher";
	else if (i == ASS)
		return "Assassin";
	else if (i == TAFT)
		return "Taft";
	else if (i == FLAME)
		return "Flame Ball";
	else if (i == THROW)
		return "Kinetic Throw";
	else if (i == FREEZE)
		return "Freeze Grenades";
	else if (i == BLOW)
		return "Death Blow";
	else if (i == TELEPORT)
		return "Teleportation";
	else if (i == BEACON)
		return "Teleport Beacon";
	else if (i == CARD)
		return "Kinetic Cards";
	else if (i == WAIL)
		return "Banshee Wail";
	else if (i == GRAV)
		return "Reverse Gravity";
	else if (i == ANTS)
		return "Ants in the Pants";
	else if (i == BLAST)
		return "Psionic Blast";
	else if (i == OPTIC)
		return "Optic Blast";
	else if (i == SABER)
		return "Lightsaber";
	else if (i == GRAPPLE)
		return "Bionic Claw";
	else if (i == SUCK)
		return "Black Hole";
	else if (i == ANCHOR)
		return "Power Anchor";
	else if (i == GREN)
		return "Grenade Swarm";
	else if (i == CASCADE)
		return "Flame Cascade";
	else if (i == I9)
		return "Impulse 9";
	else if (i == PROX)
		return "Proximity Mines";
	else if (i == BLIND)
		return "Power Word, Blind";
	else if (i == WEIRD)
		return "Weird Bombs";
	else if (i == SUN)
		return "The Sun";
	else if (i == JEDI)
		return "Jedi Knight";
	else if (i == RAND_ACT)
		return "Random Active";
	else
		return "???";
}

char * PName (int i)
{
	if (i == 0)
		return "None";
	else if (i == MORTAL)
		return "Mere Mortal";
	else if (i == ROBOT)
		return "Killer Robot";
	else if (i == CRIP)
		return "Cripple w/ a Big Gun";
	else if (i == MAGE)
		return "Archmage";
	else if (i == PUNISH)
		return "Punisher";
	else if (i == ASS)
		return "Assassin";
	else if (i == TAFT)
		return "Taft";
	else if (i == BOOT)
		return "Boot to the Head";
	else if (i == ELASTIC)
		return "Elastic";
	else if (i == REPULSE)
		return "Repulsion Field";
	else if (i == IRAD)
		return "Immune to Radius";
	else if (i == REGEN)
		return "Regeneration";
	else if (i == RADIO)
		return "Radioactive";
	else if (i == FORCE)
		return "Force Field";
	else if (i == INVIS)
		return "Invisibility";
	else if (i == ABSORB)
		return "Energy Absorption";
	else if (i == BULLET)
		return "Bullet Proof";
	else if (i == PRISMATIC)
		return "Prismatic Shell";
	else if (i == LIFE)
		return "Life Well";
	else if (i == ARMOR)
		return "Metallic Form";
	else if (i == SPEED)
		return "Super Speed";
	else if (i == FLY)
		return "Flight";
	else if (i == LFORM)
		return "Liquid Form";
	else if (i == DENSE)
		return "Hyper Density";
	else if (i == SHIN)
		return "Shining Radiance";
	else if (i == CONNECT)
		return "Heightened Reflexes";
	else if (i == SUN)
		return "The Sun";
	else if (i == JEDI)
		return "Jedi Knight";
	else if (i == RAND_PAS)
		return "Random Passive";
	else
		return "???";
}
char * SName (int i)
{
	if (i == 0)
		return "None";
	else if (i == MORTAL)
		return "Mere Mortal";
	else if (i == ROBOT)
		return "Killer Robot";
	else if (i == CRIP)
		return "Cripple w/ a Big Gun";
	else if (i == MAGE)
		return "Archmage";
	else if (i == PUNISH)
		return "Punisher";
	else if (i == ASS)
		return "Assassin";
	else if (i == TAFT)
		return "Taft";
	else if (i == HASTE)
		return "Hasted Attacks";
	else if (i == JUMP)
		return "Super Jump";
	else if (i == ISHOTS)
		return "Invisible Shots";
	else if (i == FSHOTS)
		return "Fast Projectiles";
	else if (i == VAMP)
		return "Vampiric Attacks";
	else if (i == AMMO)
		return "Reduced Ammo Use";
	else if (i == AP)
		return "Armor Piercing";
	else if (i == ELEC)
		return "Electric Attacks";
	else if (i == SNIPER)
		return "Sniper Shots";
	else if (i == AGG)
		return "Aggravated Attacks";
	else if (i == BERSERK)
		return "Berserker Rage";
	else if (i == AODEATH)
		return "Angel of Death";
	else if (i == AOLIFE)
		return "Angel of Life";
	else if (i == AOMERCY)
		return "Angel of Mercy";
	else if (i == BLOSSOM)
		return "Happy Fun Balls";
	else if (i == LUCK)
		return "Dark One's Luck";
	else if (i == STR)
		return "Super Strength";
	else if (i == FUNK)
		return "Funkagroovitalizer";
	else if (i == BOPGUN)
		return "Bop Gun";
	else if (i == SUN)
		return "The Sun";
	else if (i == JEDI)
		return "Jedi Knight";
	else if (i == RAND_SPE)
		return "Random Special";
	else
		return "???";
}

char * CName (int i)
{
	if (i == 0)
		return "Powers =========";
	else if (i == C_MORTAL)
		return "Mere Mortal ====";
	else if (i == C_ROBOT)
		return "Killer Robot ===";
	else if (i == C_CRIP)
		return "Cripple ========";
	else if (i == C_MAGE)
		return "Archmage =======";
	else if (i == C_PUNISH)
		return "Punisher =======";
	else if (i == C_ASS)
		return "Assassin =======";
	else if (i == C_TAFT)
		return "Taft ===========";
	else if (i == C_JEDI2)
		return "Jedi Knight ====";
	else if (i == C_BIONIC)
		return "Bionic Commando ";
	else if (i == C_BOMB)
		return "E.M.B.W.B.M. ===";
	else if (i == C_JEDI)
		return "Jedi Knight ====";
	else if (i == C_SUN)
		return "The Sun ========";
	else
		return "???";
}

///////////////////////////////////////// passive power functs

void boot (edict_t * ent, edict_t * other, float damage)
{
	vec3_t	aim = {MELEE_DISTANCE, 0, -40};
	int mod, kick;

	if (ent->client && ent->client->pers.passive == BOOT)
	{
		mod = MOD_BOOT;
		kick = 500;
	}
	else if (ent->client && ent->client->pers.passive == CRIP)
	{
		mod = MOD_CRIP;
		kick = 50;
	}

	ent->enemy = other;
	fire_hit (ent, aim, damage, kick, mod);
}

///////////////////////////////////////// special power functs

int needitem (edict_t *ent, edict_t *it)
{
	int index = ITEM_INDEX(it->item);

	if (it->svflags & SVF_NOCLIENT)
		return false;

	if (it->item->pickup == Pickup_Health && (ent->health < ent->max_health || it->style & 1))
		return true;
	if (it->item->pickup == Pickup_Weapon)
	{
		if (((int)(dmflags->value) & DF_WEAPONS_STAY) && ent->client->pers.inventory[index])
		{
//			if (!(it->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
				return false;	// leave the weapon for others to pickup
		}
		if (it->touch == drop_temp_touch)
		{
			return false;
		}
		if ((it->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
		{
			return false;
		}

		return true;
	}
	if (it->item->pickup == Pickup_Powerup)
//		return true;
		return false;
	if (it->item->pickup == Pickup_Adrenaline)
		return true;
	if (it->item->pickup == Pickup_AncientHead)
		return true;
	if (it->item->pickup == Pickup_Bandolier)
		return true;
	if (it->item->pickup == Pickup_Pack)
		return true;
	if (it->item->pickup == Pickup_Key)
		return true;
	if (it->item->pickup == Pickup_Ammo)
	{
		int max = 0, index = 0;

		if (it->item->tag == AMMO_BULLETS)
			max = ent->client->pers.max_bullets;
		else if (it->item->tag == AMMO_SHELLS)
			max = ent->client->pers.max_shells;
		else if (it->item->tag == AMMO_ROCKETS)
			max = ent->client->pers.max_rockets;
		else if (it->item->tag == AMMO_GRENADES)
			max = ent->client->pers.max_grenades;
		else if (it->item->tag == AMMO_CELLS)
			max = ent->client->pers.max_cells;
		else if (it->item->tag == AMMO_SLUGS)
			max = ent->client->pers.max_slugs;
		else
			return false;
	
		index = ITEM_INDEX(it->item);
	
		if (ent->client->pers.inventory[index] >= max)
			return false;

		if (it->spawnflags & DROPPED_ITEM)
			return false;
		
		return true;
	}
	if (it->item->pickup == Pickup_Armor)
	{
		int	jacket_armor_index;
		int	combat_armor_index;
		int	body_armor_index;
		int	power_screen_index;
		int	power_shield_index;
		int old_armor_index = ArmorIndex(ent);

		jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
		combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
		body_armor_index   = ITEM_INDEX(FindItem("Body Armor"));
		power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
		power_shield_index = ITEM_INDEX(FindItem("Power Shield"));

		if (it->item->tag == ARMOR_SHARD)
			return true;
		else if (old_armor_index == 0)
			return true;
		else
		{
			gitem_armor_t	*oldinfo;
			gitem_armor_t	*newinfo;
			int salvage = 0, newcount = 0, salvagecount = 0;

			newinfo = (gitem_armor_t *)it->item->info;
			if (old_armor_index == jacket_armor_index)
				oldinfo = &jacketarmor_info;
			else if (old_armor_index == combat_armor_index)
				oldinfo = &combatarmor_info;
			else // (old_armor_index == body_armor_index)
				oldinfo = &bodyarmor_info;

			if (newinfo->normal_protection > oldinfo->normal_protection)
				return true;

//			salvage = newinfo->normal_protection / oldinfo->normal_protection;
//			salvagecount = salvage * (newinfo->base_count / (ent->client->pers.passive == ARMOR ? 1.25 : 1)); // SH
//			newcount = ent->client->pers.inventory[old_armor_index] + salvagecount;
//			if (newcount > oldinfo->max_count && ent->client->pers.passive != ARMOR) // SH
//				newcount = oldinfo->max_count;

			// if we're already maxed out then we don't need the new armor
			if (ent->client->pers.inventory[old_armor_index] >= oldinfo->max_count)
				return false;

			return true;
		}
	}
	return false;
}

int aom_valid (edict_t *cur)
{
	if (!(strcmp(cur->classname, "grenade")))
		return true;
	if (!(strcmp(cur->classname, "rocket")))
		return true;
	if (!(strcmp(cur->classname, "robo rocket")))
		return true;
	if (!(strcmp(cur->classname, "bfg blast")))
		return true;
	if (!(strcmp(cur->classname, "bolt")))
		return true;
	if (!(strcmp(cur->classname, "super shot")))
		return true;
	if (cur->think == target_laser_think)
		return true;

	return false;
}

void aol_finditem (edict_t *ent)
{
	edict_t	*cur;
	trace_t	tr;

	cur = findradius (world, ent->owner->s.origin, 300);

	while (cur != NULL)
	{
		if (cur->item && cur != ent->enemy && needitem (ent->owner, cur))
		{
//gi.cprintf (ent->owner, PRINT_HIGH, "aol found an item, %s\n", cur->classname);
			tr = gi.trace (cur->s.origin, ent->mins, ent->maxs, ent->s.origin, ent, MASK_SOLID);
			if (tr.fraction == 1.0)
			{
//gi.cprintf (ent->owner, PRINT_HIGH, "ent: %s, targ: %s\n", tr.ent->classname, cur->classname);
				ent->goalentity = cur;
				return;
			}
		}
		cur = findradius (cur, ent->owner->s.origin, 300);
	}

	return;
}

void aom_findtarg (edict_t *ent)
{
	edict_t	*cur;
	trace_t	tr;

	cur = findradius (world, ent->owner->s.origin, 100);

	while (cur != NULL)
	{
		if (aom_valid (cur) && cur->owner != ent->owner && cur->s.modelindex)
		{
//gi.cprintf (ent->owner, PRINT_HIGH, "aom found an item, %s\n", cur->classname);
			tr = gi.trace (cur->s.origin, ent->mins, ent->maxs, ent->s.origin, ent, MASK_SOLID);
			if (tr.fraction == 1.0)
			{
//gi.cprintf (ent->owner, PRINT_HIGH, "ent: %s, targ: %s\n", tr.ent->classname, cur->classname);
				ent->goalentity = cur;
				return;
			}
		}
		cur = findradius (cur, ent->owner->s.origin, 100);
	}
	return;
}

void angel_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == world)
	{
		ent->enemy = ent->goalentity;
		ent->goalentity = NULL;
	}
}

void aol_think (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	vel;
	trace_t tr;

	ent->groundentity = NULL;

	if (ent->owner->angel != ent || ent->owner->client->pers.special != AOLIFE)
	{
		G_FreeEdict (ent);
		return;
	}

	vectoangles (ent->velocity, ent->s.angles);

	tr = gi.trace (ent->s.origin, NULL, NULL, ent->owner->s.origin, NULL, MASK_SHOT);
	if (tr.ent != ent->owner)
	{
		VectorAdd (ent->owner->s.origin, ent->move_origin, offset);
		VectorCopy (offset, ent->s.origin);
	}

// new offset?
	if (rand()%10 < 1 || ent->move_origin[0] == 0)
	{
		VectorSet (offset, rand()%100 - 50, rand()%100 - 50, rand()%50);
		VectorCopy (offset, ent->move_origin);
	}
// set velocity
	if (!(ent->goalentity) && ent->owner->health > 0)
		aol_finditem(ent);

//	if (ent->goalentity)
//		gi.cprintf (ent->owner, PRINT_HIGH, "target: %s\n", ent->goalentity->classname);

	if (ent->goalentity && ent->owner->health > 0)
	{
		VectorSubtract (ent->owner->s.origin, ent->goalentity->s.origin, offset);

		if (!ent->goalentity->inuse)
		{
			ent->goalentity = NULL;
		}
		else if (VectorLength (offset) > 300)
		{
			ent->goalentity = NULL;
		}
		else if (ent->goalentity->svflags & SVF_NOCLIENT)
		{
			ent->goalentity = NULL;
		}
		else
		{
			vec3_t	len;
			VectorSubtract (ent->s.origin, ent->goalentity->s.origin, len);
			if (VectorLength (len) < 10)
			{
				int			i, num;
				edict_t		*touch[MAX_EDICTS], *hit;
			
				num = gi.BoxEdicts (ent->absmin, ent->absmax, touch, MAX_EDICTS, AREA_TRIGGERS);

//				ent->goalentity = NULL;
//				ent->enemy = NULL;

				for (i=0 ; i<num ; i++)
				{
					hit = touch[i];
					if (hit != ent->goalentity)
						continue;
					if (!hit->inuse)
						continue;
					if (!hit->touch)
						continue;
					if (ent->owner->health <= 0)
						continue;

					hit->touch (hit, ent->owner, NULL, NULL);
				}
				ent->goalentity = NULL;
				ent->enemy = NULL;
			}

			VectorSubtract (ent->goalentity->s.origin, ent->s.origin, vel);
			VectorScale (vel, 7, vel);
			VectorCopy (vel, ent->velocity);
		}
	}
	
	if (!(ent->goalentity) || ent->owner->health <= 0)
	{
		VectorAdd (ent->owner->s.origin, ent->move_origin, offset);
		VectorSubtract (offset, ent->s.origin, vel);
		VectorScale (vel, 2, vel);
		VectorAdd (vel, ent->velocity, ent->velocity);
	}

	ent->nextthink = level.time + FRAMETIME;
}

void aom_think (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	vel;
	trace_t tr;

	ent->groundentity = NULL;

	if (ent->owner->angel != ent || (ent->owner->client->pers.special != AOMERCY && ent->owner->client->pers.special != MAGE))
	{
		G_FreeEdict (ent);
		return;
	}

//	VectorCopy (ent->owner->s.angles, ent->s.angles);
	vectoangles (ent->velocity, ent->s.angles);

//	gi.cprintf (ent->owner, PRINT_HIGH, "angel of mercy think\n");
// check if master in view
	tr = gi.trace (ent->s.origin, NULL, NULL, ent->owner->s.origin, NULL, MASK_SHOT);
	if (tr.ent != ent->owner)
	{
		VectorAdd (ent->owner->s.origin, ent->move_origin, offset);
		VectorCopy (offset, ent->s.origin);
	}

// new offset?
	if (rand()%10 < 1 || ent->move_origin[0] == 0)
	{
		VectorSet (offset, rand()%70 - 35, rand()%70 - 35, rand()%35);
		VectorCopy (offset, ent->move_origin);
	}
// set velocity
	if (!(ent->goalentity))
		aom_findtarg(ent);

//	if (ent->goalentity)
//		gi.cprintf (ent->owner, PRINT_HIGH, "target: %s\n", ent->goalentity->classname);

	if (ent->goalentity)
	{
		VectorSubtract (ent->owner->s.origin, ent->goalentity->s.origin, offset);

		if (VectorLength (offset) > 100)
		{
			ent->goalentity = NULL;
		}
		else
		{
			vec3_t	len;
			{
				VectorSet (len, 1, 1, 1);
				VectorAdd (len, ent->goalentity->s.origin, len);
				VectorCopy (len, ent->s.origin);
			}

			VectorSubtract (ent->s.origin, ent->goalentity->s.origin, len);
			if (VectorLength (len) < 10)
			{
//gi.cprintf (ent->owner, PRINT_HIGH, "blocked: %s\n", ent->goalentity->classname);				
				SpawnDamage (TE_SPARKS, ent->goalentity->s.origin, ent->goalentity->s.origin, 100);
				G_FreeEdict (ent->goalentity);
				ent->goalentity = NULL;
			}
		}
	}
	
	else if (!(ent->goalentity))
	{
		VectorAdd (ent->owner->s.origin, ent->move_origin, offset);
		VectorSubtract (offset, ent->s.origin, vel);
		VectorScale (vel, 3.5, vel);
		VectorAdd (vel, ent->velocity, ent->velocity);
	}

	ent->nextthink = level.time + FRAMETIME;
}

void aod_think (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	vel;
	trace_t tr;

	ent->groundentity = NULL;

	if (ent->owner->angel != ent || ent->owner->client->pers.special != AODEATH)
	{
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (ent->owner->s.angles, ent->s.angles);

//	gi.cprintf (ent->owner, PRINT_HIGH, "angel of death think\n");
// check if master in view
	tr = gi.trace (ent->s.origin, NULL, NULL, ent->owner->s.origin, NULL, MASK_SHOT);
	if (tr.ent != ent->owner)
	{
		VectorAdd (ent->owner->s.origin, ent->move_origin, offset);
		VectorCopy (offset, ent->s.origin);
	}

// new offset?
	if (rand()%10 < 1 || ent->move_origin[0] == 0)
	{
		VectorSet (offset, rand()%50 - 25, rand()%50 - 25, rand()%25);
		VectorCopy (offset, ent->move_origin);
	}
// set velocity
	VectorAdd (ent->owner->s.origin, ent->move_origin, offset);
	VectorSubtract (offset, ent->s.origin, vel);
	VectorScale (vel, 2, vel);
	VectorAdd (vel, ent->velocity, ent->velocity);

	ent->nextthink = level.time + FRAMETIME;
}
void spawn_angel(edict_t *ent)
{
	edict_t	*angel;
	
	angel = G_Spawn();
	angel->owner = ent;
	ent->angel = angel;

	VectorCopy (ent->s.origin, angel->s.origin);
	angel->movetype = MOVETYPE_FLY;
	angel->solid = SOLID_BBOX;
//	angel->s.effects |= EF_BLASTER;
	angel->touch = angel_touch;
	VectorClear (angel->mins);
	VectorClear (angel->maxs);
//	angel->s.modelindex = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");
	angel->nextthink = level.time + FRAMETIME;
	
	if (ent->client->pers.special == AODEATH)
	{
		angel->think = aod_think;
		angel->classname = "angel of death";
		angel->s.sound = gi.soundindex ("world/amb19.wav");
		angel->s.modelindex = gi.modelindex ("models/super2/aodeath/tris.md2");
	}
	else if (ent->client->pers.special == AOLIFE)
	{
		angel->think = aol_think;
		angel->classname = "angel of life";
		angel->s.sound = gi.soundindex ("world/comp_hum2.wav");
		angel->s.modelindex = gi.modelindex ("models/super2/aolife/tris.md2");
	}
	else if (ent->client->pers.special == AOMERCY)
	{
		angel->think = aom_think;
		angel->classname = "angel of mercy";
		angel->s.sound = gi.soundindex ("world/comp_hum2.wav");
		angel->s.modelindex = gi.modelindex ("models/super2/aomercy/tris.md2");
	}
	else if (ent->client->pers.special == MAGE)
	{
		angel->think = aom_think;
		angel->classname = "familiar";
		angel->s.sound = gi.soundindex ("world/amb19.wav");
		angel->s.modelindex = gi.modelindex ("models/super2/aomercy/tris.md2");
	}
	else
	{
		ent->angel = NULL;
		G_FreeEdict (angel);
		return;
	}

	gi.linkentity (angel);

}

void blossom_think (edict_t *self)
{
	float	speed;
	edict_t *cur = world;

	if (self->waterlevel)
	{
		G_FreeEdict (self);
		return;
	}

	if (self->touch == NULL)
	{
		self->touch = rocket_touch;
	}

	if (self->enemy && self->enemy->health <= 0)
	{
		self->enemy = NULL;
	}

	if (self->enemy == NULL)
	{
		while (cur != NULL)//findradius (cur, self->s.origin, 500) != NULL)
		{
			if (((cur != self->owner && cur->client && !(cur->flags & FL_OBSERVER)) || cur->svflags & SVF_MONSTER) && cur->health > 0)
			{
				if (rand()%(clientcount()) == 0)
					self->enemy = cur;				
			}

			cur = findradius (cur, self->s.origin, 500);
		}
	}

	if (self->enemy)
	{
		VectorSubtract (self->enemy->s.origin, self->s.origin, self->velocity);
	}

	speed = VectorLength (self->velocity);

	if (speed != 200)
	{
		VectorNormalize2 (self->velocity, self->velocity);
		VectorScale (self->velocity, 200, self->velocity);
	}
	
	vectoangles (self->velocity, self->s.angles);
	self->nextthink = level.time + 0.1;
}

void BecomeExplosion1_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

void death_blossom (edict_t *ent, edict_t *other)
{
	edict_t		*blossom;
	int			i=0;
	vec3_t		dir, start;
	int			speed = 200;

	if (ent->movetype == MOVETYPE_TOSS)
		return;

	if (other == ent)
		return;

	gi.sound (ent, CHAN_AUTO, gi.soundindex("powers/deathblossom.wav"), 1, ATTN_NORM, 0);

	VectorCopy (ent->s.origin, start);
	start[2] += 25;

	for (i=0;i<4;i++)
	{
		avrandom(dir);
				
		blossom = G_Spawn();
		VectorCopy (start, blossom->s.origin);
		VectorCopy (dir, blossom->movedir);
		vectoangles (dir, blossom->s.angles);
		VectorScale (dir, speed, blossom->velocity);
		blossom->movetype = MOVETYPE_FLYMISSILE;
		blossom->clipmask = MASK_SHOT;
		blossom->solid = SOLID_BBOX;
		blossom->classname = "super shot";
//		blossom->s.effects |= EF_ROCKET;
		VectorClear (blossom->mins);
		VectorClear (blossom->maxs);

		VectorSet (blossom->mins, -3, -3, -3);
		VectorSet (blossom->maxs, 3, 3, 3);
/*
		blossom->takedamage = DAMAGE_AIM;
		blossom->health = 20;
		blossom->die = BecomeExplosion1_die;
*/		
//		blossom->s.modelindex = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");
		blossom->s.modelindex = gi.modelindex ("models/super2/blossom/db.md2");
//		blossom->s.effects |= EF_COLOR_SHELL;
//		blossom->s.renderfx |= RF_SHELL_GREEN|RF_SHELL_BLUE|RF_SHELL_RED;
		blossom->owner = ent;
		blossom->touch = NULL;
		blossom->nextthink = level.time + 0.4;
		blossom->think = blossom_think;
		blossom->dmg = 0;
		blossom->radius_dmg = 100;
		blossom->dmg_radius = 100;
		blossom->s.sound = gi.soundindex ("weapons/rockfly.wav");

		if (i < 1 && other != ent)
			blossom->enemy = other;
		
		gi.linkentity (blossom);
		blossom = NULL;
	}
}

///////////////////////////////////////// active power functs

void fball_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage && other != ent->enemy)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_FLAMEBALL);
		ent->enemy = other;
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
		ent->enemy = NULL;
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, 0, MOD_FLAMEBALL);
/*
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
*/
//	G_FreeEdict (ent);
}

void fball_fire (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_BOUNCE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->style = 23;
	rocket->classname = "super shot";
	rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");
	rocket->owner = self;
	rocket->touch = fball_touch;
	rocket->nextthink = level.time + 5;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");

//	if (self->client)
//		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

int kin_throw (edict_t * self)
{
	trace_t tr;
	vec3_t vel, end, start, min, max;
	int damage;
	
//	AngleVectors(ent->client->v_angle, v_forward, v_right, v_up);
	MV(self);
	VectorMA(self->s.origin, 2000, v_forward, end);

	VectorCopy (self->s.origin, start);
	start[2] += self->viewheight-8;
	end[2] += self->viewheight-8;

	VectorSet (min, -10, -10, -10);
	VectorSet (max, 10, 10, 10);

	tr = gi.trace (start, min, max, end, self, MASK_SHOT);

	if (!strcmp (tr.ent->classname, "func_button"))
	{
		button_use (tr.ent, NULL, self);
		return 1;
	}

	if (OnSameTeam (tr.ent, self) || tr.ent == self || !tr.ent->takedamage || tr.ent == world || (tr.ent->client && tr.ent->client->pers.passive == DENSE))//((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client)))
		return 0;

	vel[0] = rand()%2000 - 1000;
	vel[1] = rand()%2000 - 1000;
	vel[2] = rand()%500 + 100;

	VectorMA(tr.ent->velocity, 1, vel, tr.ent->velocity);
	damage = VectorLength(vel)/30;
	T_Damage(tr.ent, self, self, vel, tr.ent->s.origin, tr.plane.normal, damage, 0, DAMAGE_NO_PROTECTION|DAMAGE_NO_ARMOR, MOD_KINTHROW); 

//gi.cprintf(self, PRINT_HIGH, "kin_throw, %s, %i\n", tr.ent->classname, damage);	

	return 1;
}

void card_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, DAMAGE_NO_PROTECTION|DAMAGE_NO_ARMOR, MOD_KINCARD);
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}

//	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, 0, MOD_KINCARD);
/*
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
*/
	G_FreeEdict (ent);
}

void card_fire (edict_t *self, int j, int damage, int speed, float damage_radius, int radius_damage)
{
	edict_t	*rocket;
	vec3_t avel;

			float fan = ((float)j - 3) / 10;
			vec3_t start;
			vec3_t dir;

//			fan = (j - 3) / 10;
			MV (self);
			VectorScale(v_forward, 20, start);
//			VectorMA(v_forward, fan, v_right, v_forward);
			VectorMA(v_forward, fan, v_right, dir);
			VectorMA(start, 1, self->s.origin, start);
			start[2] += self->viewheight - 8;

//gi.cprintf (self, PRINT_HIGH, "j:%f, f:%f, s:%i\n", (float)(j), fan, speed);

	rocket = G_Spawn();
	vrandom (avel);
	VectorScale (avel, 200, rocket->avelocity);
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLY;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	rocket->classname = "kcard";
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/super2/cards/tris.md2");
	rocket->s.skinnum = rand()%5;
	rocket->owner = self;
	rocket->touch = card_touch;
	rocket->nextthink = level.time + 5;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = radius_damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("misc/lasfly.wav");

	gi.linkentity (rocket);
}

void grav_think (edict_t *ent)
{
	edict_t *cur;
	vec3_t	vel;

	if (ent->health < level.time)
	{
		G_FreeEdict (ent);
		return;
	}
	
	ent->nextthink = level.time + FRAMETIME;

	cur = findradius (world, ent->s.origin, 300);			
	while (cur != NULL)
	{
		if (cur == world)
			break;

		if (cur != ent && cur->takedamage && cur != world && cur != ent->owner && !OnSameTeam (ent->owner, cur) && !(cur->client && cur->client->pers.passive == DENSE))
		{
			vel[0] = 0;
			vel[1] = 0;
			vel[2] = 1;

			if (cur->groundentity)
			{
				cur->groundentity = NULL;
// ONE			cur->s.origin[2] += 1;
			}
			VectorMA (cur->velocity, 2 * sv_gravity->value * FRAMETIME, vel, cur->velocity);
		}

		cur = findradius (cur, ent->s.origin, 300);
	}

}

void grav_place (edict_t *ent)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (ent->s.origin, rocket->s.origin);
	rocket->movetype = MOVETYPE_NONE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_NOT;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = 0;
	rocket->owner = ent;
	rocket->nextthink = level.time + FRAMETIME;
	rocket->think = grav_think;
	rocket->health = level.time + 8;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");

//	if (self->client)
//		check_dodge (self, rocket->s.origin, dir, speed);

	gi.linkentity (rocket);
}

void blast_fire (edict_t *ent, vec3_t loc, int damage, int mod)
{
	edict_t *rocket;
	rocket = G_Spawn();
	VectorCopy (loc, rocket->s.origin);
	rocket->movetype = MOVETYPE_NONE;
	rocket->clipmask = MASK_SHOT;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->owner = ent;
	rocket->nextthink = level.time + FRAMETIME;
	rocket->think = G_FreeEdict;
	gi.linkentity (rocket);

	if (ent->client)
		PlayerNoise(ent, loc, PNOISE_IMPACT);

	T_RadiusDamage(rocket, ent, damage, ent, 80, DAMAGE_RADIO, mod);


	if (mod == MOD_PSIBLAST)
	{
		gi.WriteByte (svc_temp_entity);
		if (ent->waterlevel)
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
		gi.WritePosition (rocket->s.origin);
		gi.multicast (rocket->s.origin, MULTICAST_PVS);
	}
}

void grap_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (other != world)
	{
		ent->owner->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		
		if (other->takedamage)
		{
			T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 1000, 0, MOD_GRAPPLE);
			ent->owner->client->pers.active_finished = level.time + 0.5;
		}
		ent->owner->teamchain = NULL;
		G_FreeEdict(ent);
		return;		
	}
	gi.sound (ent, CHAN_AUTO, gi.soundindex("powers/bionichit.wav"), 1, ATTN_NORM, 0); // "berserk/death.wav"
	ent->s.sound = 0;
	ent->s.frame = 1;
	ent->flags |= FL_OPTIC;
	VectorClear (ent->avelocity);
	VectorScale (plane->normal, -1, ent->movedir);
	VectorMA (ent->s.origin, 3, plane->normal, ent->s.origin);
	ent->solid = SOLID_NOT;
	gi.linkentity (ent);
}

void grap_think (edict_t * ent)
{
	vec3_t dir;
//	trace_t tr;

	if ((ent->owner->deadflag & DEAD_DEAD) || !ent->owner->inuse || ent->owner->flags & FL_OBSERVER)
	{
//gi.cprintf (ent->owner, PRINT_HIGH, "removing grapple\n");
		if (ent->owner->teamchain == ent)
			ent->owner->teamchain = NULL;

		G_FreeEdict (ent);
		return;
	}

//	tr = gi.trace (ent->s.origin, NULL, NULL, ent->owner->s.origin, NULL, MASK_SHOT);

//	if (tr.ent != ent->owner)
//	{
//		ent->owner->teamchain = NULL;
//		G_FreeEdict (ent);
//		return;
//	}

	vectoangles (ent->movedir, ent->s.angles);

	if (ent->flags & FL_OPTIC)
	{
			
			VectorSubtract (ent->s.origin, ent->owner->s.origin, dir);
			VectorNormalize2 (dir, dir);

//			VectorMA (dir, ent->wait, dir, ent->owner->velocity);
			VectorMA (ent->owner->velocity, ent->wait, dir, ent->owner->velocity);
//			if (ent->wait <= 1000)
//				ent->wait += 50;

			VectorClear (ent->velocity);
	}
	else
	{
		ent->groundentity = NULL;
		G_TouchTriggers (ent);
		VectorMA (ent->velocity, 0.1, ent->velocity, ent->velocity);
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (ent - g_edicts);
	gi.WritePosition (ent->owner->s.origin);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	
	ent->nextthink = level.time + FRAMETIME;
}

void anchor_think (edict_t *self)
{
	vec3_t	dist;
	float	len;

	if (self->max_health < level.time || self->enemy->health <= 0 || self->enemy->flags & FL_OBSERVER)
	{
		G_FreeEdict (self);
		return;
	}

	VectorSubtract (self->s.origin, self->enemy->s.origin, dist);
	len = VectorLength (dist);
	if (len > 200)
	{
		VectorMA (self->enemy->velocity, 2, dist, self->enemy->velocity);
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
	gi.WriteShort (self - g_edicts);
	gi.WritePosition (self->enemy->s.origin);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->nextthink = level.time + FRAMETIME;
}

int anchor (edict_t * self)
{
	trace_t tr;
	vec3_t	end;
	edict_t	*anchor;
	vec3_t	start;
	
//	AngleVectors(ent->client->v_angle, v_forward, v_right, v_up);
	MV(self);
	VectorMA(self->s.origin, 1000, v_forward, end);

	VectorCopy (self->s.origin, start);
	start[2] += self->viewheight-8;
	end[2] += self->viewheight-8;

	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if (tr.ent == self || !tr.ent->takedamage || tr.ent == world || OnSameTeam (tr.ent, self) || !((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client)) || (tr.ent->client && tr.ent->client->pers.passive == DENSE))
		return 0;

//	tr.ent = self;

	anchor = G_Spawn();
	VectorCopy (tr.ent->s.origin, anchor->s.origin);
	anchor->movetype = MOVETYPE_NONE;
	anchor->clipmask = MASK_SHOT;
	anchor->solid = SOLID_NOT;
	anchor->classname = "anchor";
	VectorClear (anchor->mins);
	VectorClear (anchor->maxs);
	anchor->s.modelindex = gi.modelindex ("models/super2/anchor/tris.md2");
	anchor->owner = self;
	anchor->enemy = tr.ent;
	anchor->dmg_radius = 200;
	anchor->nextthink = level.time + FRAMETIME;
	anchor->max_health = level.time + (ctf->value?3:10);
	anchor->think = anchor_think;

//	if (self->client)
//		check_dodge (self, anchor->s.origin, dir, speed);

	gi.linkentity (anchor);
		
//gi.cprintf(self, PRINT_HIGH, "kin_throw, %s, %i\n", tr.ent->classname, damage);	
	gi.sound(self, CHAN_AUTO, gi.soundindex("powers/poweranchor.wav"), 1, ATTN_NORM, 0);

	return 1;
}

void cascade_think (edict_t *self)
{
	trace_t tr;
	vec3_t	end;
	edict_t	*hit;

	if (VectorLength (self->velocity) < 50 || level.time > self->max_health)
	{
		G_FreeEdict (self);
		return;
	}

	VectorNormalize2 (self->velocity, self->velocity);
	VectorScale (self->velocity, 500, self->velocity);

	VectorCopy (self->s.origin, end);
	end[2] -= 1000;

	tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SHOT);

	hit = G_Spawn();
	VectorCopy (tr.endpos, hit->s.origin);
	gi.linkentity (hit);

	T_RadiusDamage(hit, self->owner, 100, self->owner, 120, 0, MOD_CASCADE);

	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_ROCKET_EXPLOSION);
		gi.WritePosition (hit->s.origin);
		gi.multicast (hit->s.origin, MULTICAST_PVS);
	}

	G_FreeEdict (hit);

	self->nextthink = level.time + 0.2;
}

static void touch_free(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	G_FreeEdict(self);
}

void fire_cascade (edict_t *ent)
{
	edict_t		*cascade;
	vec3_t		start;
	VectorCopy (ent->s.origin, start);

	MV (ent);

	start[2] += ent->viewheight;
	cascade = G_Spawn();
	VectorCopy (start, cascade->s.origin);
	VectorCopy (v_forward, cascade->movedir);
	vectoangles (v_forward, cascade->s.angles);
	VectorScale (v_forward, 500, cascade->velocity);
	cascade->movetype = MOVETYPE_FLYMISSILE;
	cascade->clipmask = MASK_SOLID;
	cascade->solid = SOLID_NOT;
	VectorClear (cascade->mins);
	VectorClear (cascade->maxs);
	cascade->dmg = 100;
	cascade->max_health = level.time + 5;
//	cascade->s.modelindex = gi.modelindex ("models/objects/gibs/sm_meat/tris.md2");
	cascade->owner = ent;
	cascade->touch = touch_free;
	cascade->nextthink = level.time + 0.4;
	cascade->think = cascade_think;
	cascade->classname = "cascade";
	cascade->s.sound = gi.soundindex ("weapons/rockfly.wav");
	gi.linkentity (cascade);
}

void give_random (edict_t *ent)
{
	int ran;
	char *pks;

	ran = rand()%10;

	if (ran == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "You get full life!\n");
		if (ent->health < 100)
			ent->health = 100;
		else
			ent->health += 25;

		pks = "items/pkup.wav";
	}
	else if (ran == 1)
	{
		gi.cprintf (ent, PRINT_HIGH, "You get +50 life!\n");
		ent->health += 50;
		pks = "items/l_health.wav";
	}
	else if (ran == 2)
	{
		gi.cprintf (ent, PRINT_HIGH, "You get +25 life!\n");
		ent->health += 25;
		pks = "items/l_health.wav";
	}
	else if (ran == 3)
	{
		gi.cprintf (ent, PRINT_HIGH, "You get the rocket launcher!\n");
		ent->client->pers.inventory[ITEM_INDEX(FindItem("rocket launcher"))] += 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] += 15;
		ent->client->newweapon = FindItem ("rocket launcher");
		ChangeWeapon (ent);
		pks = "misc/w_pkup.wav";
	}
	else if (ran == 4)
	{
		gi.cprintf (ent, PRINT_HIGH, "You get the hyperblaster!\n");
		ent->client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))] += 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] += 50;
		ent->client->newweapon = FindItem ("hyperblaster");
		ChangeWeapon (ent);
		pks = "misc/w_pkup.wav";
	}
	else if (ran == 5)
	{
		gi.cprintf (ent, PRINT_HIGH, "You get the chaingun!\n");
		ent->client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))] += 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] += 100;
		ent->client->newweapon = FindItem ("chaingun");
		ChangeWeapon (ent);
		pks = "misc/w_pkup.wav";
	}
	else if (ran == 6)
	{
		gi.cprintf (ent, PRINT_HIGH, "You get the railgun!\n");
		ent->client->pers.inventory[ITEM_INDEX(FindItem("railgun"))] += 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] += 20;
		ent->client->newweapon = FindItem ("railgun");
		ChangeWeapon (ent);
		pks = "misc/w_pkup.wav";
	}
	else if (ran == 7)
	{
		gi.cprintf (ent, PRINT_HIGH, "You get the BFG!\n");
		ent->client->pers.inventory[ITEM_INDEX(FindItem("bfg10k"))] += 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] += 50;
		ent->client->newweapon = FindItem ("bfg10k");
		ChangeWeapon (ent);
		pks = "misc/w_pkup.wav";
	}
	else if (ran >= 8)
	{
		gi.cprintf (ent, PRINT_HIGH, "You get body armor!\n");
		
		if (ArmorIndex (ent) != body_armor_index)
			ent->client->pers.inventory[ArmorIndex(ent)] = 0;

		ent->client->pers.inventory[ITEM_INDEX(FindItem("Body Armor"))] += 100;

		if (ent->client->pers.inventory[ITEM_INDEX(FindItem("Body Armor"))] > 200 && ent->client->pers.passive != ARMOR)
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Body Armor"))] = 200;

		pks = "misc/ar1_pkup.wav";
	}

	BoundAmmo (ent);
	if (ent->health > ent->max_health && ent->health > 200)
	{
		ent->health = (ent->max_health > 200 ? ent->max_health : 200);
	}

	gi.sound(ent, CHAN_ITEM, gi.soundindex(pks), 1, ATTN_NORM, 0);

}

void weird_think (edict_t * ent)
{
	if (ent->wait < level.time)
	{
		if (ent->teamchain != NULL)
			G_FreeEdict (ent->teamchain);

		G_FreeEdict (ent);

		return;
	}

	ent->s.frame++;
	if (ent->s.frame > 31)
		ent->s.frame = 0;

	if (ent->style == 0)
	{
		edict_t *cur;
		vec3_t	vel;

		cur = findradius (world, ent->s.origin, 200);			
		while (cur != NULL)
		{
			if (cur == world || cur == ent)
			{
				cur = findradius (cur, ent->s.origin, 200);
				continue;
			}

			if (cur != ent && cur != ent->owner && (cur->takedamage || cur->movetype == MOVETYPE_BOUNCE || cur->movetype == MOVETYPE_FLYMISSILE) && cur != world && (cur->solid != SOLID_BSP) && !(cur->flags & FL_OBSERVER) && !CTFSameTeam(cur, ent->owner))
			{
				VectorSubtract (cur->s.origin, ent->s.origin, vel);
				VectorNormalize2 (vel, vel);
				if (cur->groundentity)
				{
					cur->groundentity = NULL;
// ONE				cur->s.origin[2] += 1;
				}
				VectorMA (cur->velocity, 1500 * FRAMETIME, vel, cur->velocity);
			}

			cur = findradius (cur, ent->s.origin, 200);
		}
	}
	else if (ent->style == 1)
	{
		edict_t *cur;

		cur = findradius (world, ent->s.origin, 200);			
		while (cur != NULL)
		{
			if (cur == world || cur == ent)
			{
				cur = findradius (cur, ent->s.origin, 200);
				continue;
			}

			if (cur->client && cur != ent && cur != ent->owner && (cur->takedamage || cur->movetype == MOVETYPE_BOUNCE || cur->movetype == MOVETYPE_FLYMISSILE) && cur != world && (cur->solid != SOLID_BSP) && !(cur->flags & FL_OBSERVER) && !CTFSameTeam(cur, ent->owner))
			{
				if (cur->flash_time < level.time)
					cur->flash_time = level.time + 0.5;

				cur->flash_time += 0.3;
			}

			cur = findradius (cur, ent->s.origin, 200);
		}
	}
	else if (ent->style == 2)
	{
		T_RadiusDamage (ent, ent->owner, 2, ent->owner, 200, DAMAGE_RADIO, MOD_WEIRD);
	}
	else if (ent->style == 3)
	{
		edict_t *cur;

		cur = findradius (world, ent->s.origin, 200);			
		while (cur != NULL)
		{
			if (cur == world || cur == ent)
			{
				cur = findradius (cur, ent->s.origin, 200);
				continue;
			}

			if (cur->client && cur != ent && cur != ent->owner && (cur->takedamage || cur->movetype == MOVETYPE_BOUNCE || cur->movetype == MOVETYPE_FLYMISSILE) && cur != world && (cur->solid != SOLID_BSP) && !(cur->flags & FL_OBSERVER) && !CTFSameTeam(cur, ent->owner))
			{
				if (cur->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] > 0)
				{
					ent->owner->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] += 1;
					cur->client->pers.inventory[ITEM_INDEX(FindItem("bullets"))] -= 1;
				}
				if (cur->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] > 0)
				{
					ent->owner->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] += 1;
					cur->client->pers.inventory[ITEM_INDEX(FindItem("slugs"))] -= 1;
				}
				if (cur->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] > 0)
				{
					ent->owner->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] += 1;
					cur->client->pers.inventory[ITEM_INDEX(FindItem("shells"))] -= 1;
				}
				if (cur->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] > 0)
				{
					ent->owner->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] += 1;
					cur->client->pers.inventory[ITEM_INDEX(FindItem("cells"))] -= 1;
				}
				if (cur->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))] > 0)
				{
					ent->owner->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))] += 1;
					cur->client->pers.inventory[ITEM_INDEX(FindItem("grenades"))] -= 1;
				}
				if (cur->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] > 0)
				{
					ent->owner->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] += 1;
					cur->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))] -= 1;
				}
				BoundAmmo (ent->owner);
				
			}

			cur = findradius (cur, ent->s.origin, 200);
		}
	}
//	else if (ent->style == 4)
//	{
//	}

	ent->nextthink = level.time + 0.1;
}

///////////////////////////////////// active power use

void CallActive (edict_t * ent)
{
	int i = ent->client->pers.active;
	vec3_t forward;
	vec3_t right;
	vec3_t up;
	int vis = 0;

	if (ent->flags & FL_BERSERK)
	{
		return;
	}

	if (ent->freezetime > level.time)
	{
		return;
	}
	
	if (ent->client->ps.pmove.pm_type == PM_FREEZE)
	{
		return;
	}

	if (ent->health <= 0)
	{
		return;
	}

//	if (ent->client->pers.passive == INVIS)
//	{
//		ent->client->pers.passive_finished = level.time + 3;
//	}

	AngleVectors (ent->client->v_angle, forward, right, up);
	
	VectorNormalize2 (forward, forward);
	VectorNormalize2 (right, right);
	VectorNormalize2 (up, up);

	if (ent->flags & FL_OBSERVER)
	{
		relocate (ent);
		ent->client->pers.active_finished = level.time + 1;
		return;
	}

	if (i == NONE)
	{
		gi.cprintf (ent, PRINT_HIGH, "No active power!\n");
	}
	else if (i == ROBOT)
	{
		ent->flags |= FL_OPTIC;
		ent->client->pers.active_finished = level.time + 1;
	}
	else if (i == CRIP)
	{
		gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);	//FIXME powering down sound
		ent->client->invincible_framenum = level.framenum + 40;
		ent->client->pers.active_finished = level.time + 9;
	}
	else if (i == MAGE)
	{
		edict_t	*cur;
		edict_t	*best;
		trace_t	tr;
		vec3_t	loc;
		vec3_t	end;
		float	len;
		int		i = 0;

		cur = G_Find (world, FOFS(classname), "player");
		best = ent;

		while (cur != NULL)
		{
			if (cur != ent && !(cur->flags & FL_OBSERVER) && !OnSameTeam (ent, cur))
			{
				VectorSubtract (ent->s.origin, cur->s.origin, end);
				VectorSubtract (ent->s.origin, best->s.origin, loc);
				if (VectorLength (end) < VectorLength (loc) || best == ent)
				{
					MV (cur);
					v_forward[2] = 0;
					VectorMA (cur->s.origin, -150, v_forward, loc);
					tr = gi.trace (cur->s.origin, NULL, NULL, loc, cur, MASK_SHOT);
					
					VectorSubtract (tr.endpos, cur->s.origin, end);
					len = VectorLength (end);
					if (len > 50)
					{
						best = cur;
					}
				}
			}
			cur = G_Find (cur, FOFS(classname), "player");
		}

		if (best == NULL || (best->flags & FL_OBSERVER) || best == ent)
			return;

		MV (best);
		v_forward[2] = 0;
		
		if (rand()%10 == 0)
		{
			VectorMA (best->s.origin, 150, v_forward, loc);
		}
		else
		{
			VectorMA (best->s.origin, -150, v_forward, loc);
		}
		tr = gi.trace (best->s.origin, NULL, NULL, loc, best, MASK_SHOT);
				
		VectorSubtract (tr.endpos, best->s.origin, end);
		len = VectorLength (end);
		if (len > 50)
		{
			if (ent->flags & FL_HASFLAG)
				return;

			CTFDrop_Flag (ent);
	
			len -= 25;
			VectorNormalize2 (end, end);
			VectorScale (end, len, end);
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_EXPLOSION);
			gi.WritePosition (ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
			gi.sound (ent, CHAN_AUTO, gi.soundindex("misc/tele1.wav"), 1, ATTN_NORM, 0);
					
			VectorMA (best->s.origin, 1, end, ent->s.origin);

			for (i=0 ; i<2 ; i++)
				ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(best->s.angles[i] - ent->client->resp.cmd_angles[i]);

			ent->s.angles[PITCH] = 0;
			ent->s.angles[YAW] = 0;//best->s.angles[YAW];
			ent->s.angles[ROLL] = 0;
			VectorCopy (ent->s.angles, ent->client->ps.viewangles);
			VectorCopy (ent->s.angles, ent->client->v_angle);

			gi.linkentity (ent);
			ent->client->pers.active_finished = level.time + 5;
			return;
		}
	}
	else if (i == PUNISH)
	{
		give_random(ent);
		ent->client->pers.active_finished = level.time + 10;
/*
		if (ent->health > ent->max_health)
			return;

		ent->health += 30;

		gi.sound(ent, CHAN_ITEM, gi.soundindex("items/l_health.wav"), 1, ATTN_NORM, 0);
		ent->client->pers.active_finished = level.time + 5;
*/
	}
	else if (i == ASS)
	{
		int index;

		if (ent->flags & FL_HASFLAG)
			return;

		CTFDrop_Flag (ent);

		index = ArmorIndex (ent);
		ent->client->pers.inventory[index] = ceil(ent->client->pers.inventory[index]/2);

		make_tele (ent);
		relocate (ent);
		make_tele (ent);

		ent->client->pers.active_finished = level.time + 15;
	
	}
	else if (i == TAFT)
	{
		edict_t	*cur;
		vec3_t	dir;
		cur = world;
		cur = findradius (cur, ent->s.origin, 500);
		while (cur != NULL)
		{
			if ((cur->client && !(cur->flags & FL_OBSERVER) && cur != ent && cur->client->pers.passive != DENSE && !OnSameTeam (ent, cur)) || cur->svflags & SVF_MONSTER)
			{
				VectorSubtract (ent->s.origin, cur->s.origin, dir);
				VectorNormalize2 (dir, dir);
				VectorScale (dir, -1000, dir);
				dir[2] += 200;
				VectorCopy (dir, cur->velocity);
// ONE				cur->s.origin[2] += 1;
				cur->groundentity = NULL;
			}

			cur = findradius (cur, ent->s.origin, 500);
		}

		gi.sound (ent, CHAN_AUTO, gi.soundindex("powers/taft.wav"), 1, ATTN_NORM, 0);
		ent->client->pers.active_finished = level.time + 5;
	}
	else if (i == JEDI)
	{
		if (kin_throw(ent) == 1)
		{
			ent->client->pers.active_finished = level.time + 1.5;
			vis = 1;
		}
	}
	else if (i == SUN)
	{
		edict_t	*cur;
		cur = world;
		cur = findradius (cur, ent->s.origin, 500);
		while (cur != NULL)
		{													
			if ((cur->client && !(cur->flags & FL_OBSERVER) && cur != ent && !OnSameTeam (ent, cur)) || cur->svflags & SVF_MONSTER)
			{
				cur->blind_time = level.time + 3;
			}

			cur = findradius (cur, ent->s.origin, 500);
		}

		T_RadiusDamage (ent, ent, 40, ent, 1000, DAMAGE_RADIO, MOD_SOLAR);
		gi.sound(ent, CHAN_AUTO, gi.soundindex("powers/flare.wav"), 1, ATTN_NORM, 0);
		ent->client->pers.active_finished = level.time + 8;

		vis = 1;
	}
	else if (i == FLAME)
	{
		vec3_t start;
		VectorScale(forward, 15, start);
		VectorMA(forward, 0.2, up, forward);
		VectorMA(start, 1, ent->s.origin, start);
		start[2] += ent->viewheight - 8;
		fball_fire (ent, start, forward, 160, 1000, 0, 0);
		ent->client->pers.active_finished = level.time + 3.5;
		vis = 1;
	}
	else if (i == THROW)
	{
		if (kin_throw(ent) == 1)
		{
			ent->client->pers.active_finished = level.time + 2;
			vis = 1;
		}
	}
	else if (i == FREEZE)
	{
		vec3_t	offset;
		vec3_t	forward, right;
		vec3_t	start;
		int		damage = 120;
		float	radius;
	
		radius = 150;
	
		VectorSet(offset, 8, 8, ent->viewheight-8);
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	
		VectorScale (forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;
	
		fire_grenade (ent, start, forward, damage, 600, 2.5, radius, 1);
	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_GRENADE);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	
		PlayerNoise(ent, start, PNOISE_WEAPON);

		ent->client->pers.active_finished = level.time + 2.5;

		vis = 1;
	}
	else if (i == BLOW)
	{
		if (ent->flags & FL_DEATHBLOW)
		{
			ent->flags &= ~FL_DEATHBLOW;
			gi.cprintf(ent, PRINT_HIGH, "Death Blow deactivated\n");
			gi.sound (ent, CHAN_AUTO, gi.soundindex("powers/deathblow2.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			ent->flags |= FL_DEATHBLOW;
			gi.cprintf(ent, PRINT_HIGH, "Death Blow activated\n");
			gi.sound (ent, CHAN_AUTO, gi.soundindex("powers/deathblow.wav"), 1, ATTN_NORM, 0);
		}
		ent->client->pers.active_finished = level.time + 4;
	}
	else if (i == TELEPORT)
	{
		trace_t	tr;
		vec3_t	end;
		int		len;

		if (ent->flags & FL_HASFLAG)
			return;

		CTFDrop_Flag (ent);

		make_tele (ent);

		VectorMA (ent->s.origin, 2000, forward, end);
		tr = gi.trace (ent->s.origin, NULL, NULL, end, ent, MASK_SHOT);
		VectorSubtract (tr.endpos, ent->s.origin, end);
		len = VectorLength (end);

		if (len < 50)
			return;
		else 
			len -= 25;

		VectorNormalize2 (end, end);
		VectorScale (end, len, end);
		VectorMA (ent->s.origin, 1, end, ent->s.origin);

		make_tele (ent);

		ent->client->pers.active_finished = level.time + 2;
	}
	else if (i == BEACON)
	{
		if (ent->goalentity)
		{
			if (gi.pointcontents (ent->goalentity->s.origin) == CONTENTS_PLAYERCLIP)
			{
				gi.cprintf(ent, PRINT_HIGH, "Beacon occupied!\n");
			}
			else
			{
				if (ent->flags & FL_HASFLAG)
					return;

				CTFDrop_Flag (ent);

				make_tele (ent);
				VectorCopy(ent->goalentity->s.origin, ent->s.origin);
				make_tele (ent);

				G_FreeEdict(ent->goalentity);
				ent->goalentity = NULL;
				KillBox(ent);
				gi.cprintf(ent, PRINT_HIGH, "Beacon removed\n");
			}
		}
		else
		{
			edict_t *beacon;
			beacon = G_Spawn();
			VectorCopy(ent->s.origin, beacon->s.origin);
			gi.linkentity(beacon);
			ent->goalentity = beacon;
			gi.cprintf(ent, PRINT_HIGH, "Beacon set\n");
			gi.sound (ent, CHAN_AUTO, gi.soundindex("misc/spawn1.wav"), 1, ATTN_NORM, 0);

		}
		ent->client->pers.active_finished = level.time + 1;
	}
	else if (i == CARD)
	{
//		vec3_t start;
//		float fan;
//		float j;

		ent->cards = 5;

//		for (j=0;j<5;j++)
//		{
//			card_fire (ent, j, 24, 1000, 0, 0);
//		}

		ent->client->pers.active_finished = level.time + 0.8;

		vis = 1;
	}
	else if (i == WAIL)
	{
		gi.sound (ent, CHAN_AUTO, gi.soundindex("powers/banshee.wav"), 1, ATTN_NORM, 0); // "berserk/death.wav"
		T_RadiusDamage (ent, ent, 150, ent, 200, 0, MOD_WAIL);

		ent->client->pers.active_finished = level.time + 5;

		vis = 1;
	}
	else if (i == GRAV)
	{
		grav_place (ent);
		gi.sound(ent, CHAN_AUTO, gi.soundindex("powers/reversegrav.wav"), 1, ATTN_NORM, 0);
		ent->client->pers.active_finished = level.time + 8;
	}
	else if (i == ANTS)
	{
		trace_t	tr;
		vec3_t	end, start;

		VectorMA (ent->s.origin, 2000, forward, end);
	
		VectorCopy (ent->s.origin, start);
		start[2] += ent->viewheight-8;
		end[2] += ent->viewheight-8;

		tr = gi.trace (start, NULL, NULL, end, ent, MASK_SHOT);
//		tr.ent = ent;

		if (tr.ent->health > 0 && tr.ent->client && !OnSameTeam (tr.ent, ent) && tr.ent->client->pers.passive != DENSE)// || tr.ent->svflags & SVF_MONSTER))
		{
			tr.ent->ants = level.time + 10;
			ent->client->pers.active_finished = level.time + 5;
		}

	}
	else if (i == BLAST)
	{
		trace_t	tr;
		vec3_t	end;
		vec3_t	start;

		VectorCopy (ent->s.origin, start);
		start[2] += ent->viewheight-8;
		
		VectorMA (start, 2000, forward, end);
//		end[2] += ent->viewheight-8;

		tr = gi.trace (start, NULL, NULL, end, ent, MASK_SHOT);
		
		blast_fire (ent, tr.endpos, 75, MOD_PSIBLAST);
		gi.sound (ent, CHAN_AUTO, gi.soundindex("powers/psionic.wav"), 1, ATTN_NORM, 0); // "berserk/death.wav"
		ent->client->pers.active_finished = level.time + 1.5;

		vis = 1;
	}
	else if (i == OPTIC)
	{
		ent->flags |= FL_OPTIC;
		ent->client->pers.active_finished = level.time + 3;

		vis = 1;
	}
	else if (i == SABER)
	{
		ent->flags |= FL_OPTIC;
		ent->client->pers.active_finished = level.time + 0.6;
	}
	else if (i == GRAPPLE)
	{
		if (ent->teamchain == NULL)
		{
			edict_t	*rocket;
			vec3_t start;
			int speed = 1000;

			VectorScale(forward, 15, start);
//			VectorMA(forward, 0.2, up, forward);
			VectorMA(start, 1, ent->s.origin, start);
			start[2] += ent->viewheight - 8;

			rocket = G_Spawn();
			VectorSet (rocket->mins, -5, -5, -5);
			VectorSet (rocket->maxs, 5, 5, 5);

			VectorCopy (start, rocket->s.origin);
			VectorCopy (forward, rocket->movedir);
			vectoangles (forward, rocket->s.angles);
			VectorScale (forward, speed, rocket->velocity);
			VectorSet (rocket->avelocity, 0, 0, 300);
			rocket->movetype = MOVETYPE_FLYMISSILE;
			rocket->clipmask = MASK_PLAYERSOLID;
			rocket->solid = SOLID_BBOX;
			rocket->dmg = 40;
			rocket->wait = 230;
			rocket->s.modelindex = gi.modelindex ("models/super2/claw/tris.md2");
			rocket->owner = ent;
			rocket->teammaster = ent;
			ent->teamchain = rocket;
			rocket->delay = 1;
			rocket->touch = grap_touch;
			rocket->nextthink = level.time + FRAMETIME;
			rocket->think = grap_think;
			rocket->classname = "grapple";
			rocket->s.sound = gi.soundindex ("misc/ar2_pkup.wav");
			gi.linkentity (rocket);

			ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
			gi.sound (ent, CHAN_AUTO, gi.soundindex("powers/bionicfire.wav"), 1, ATTN_NORM, 0); // "berserk/death.wav"
		}
		else
		{
//			team_remove (ent->teamchain);
			G_FreeEdict(ent->teamchain);
			ent->teamchain = NULL;
			ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
		}
	}
	else if (i == SUCK)
	{
		edict_t	*cur;
		vec3_t	dir;
		cur = world;
		cur = findradius (cur, ent->s.origin, 500);
		while (cur != NULL)
		{
			if ((cur->client && !(cur->flags & FL_OBSERVER) && cur != ent && !OnSameTeam (ent, cur) && cur->client->pers.passive != DENSE) || cur->svflags & SVF_MONSTER)
			{
//				gi.cprintf (ent, PRINT_HIGH, "%s\n", cur->classname);

				VectorSubtract (ent->s.origin, cur->s.origin, dir);
				VectorNormalize2 (dir, dir);
				VectorScale (dir, 1300, dir);
				dir[2] += 200;
				VectorCopy (dir, cur->velocity);
// ONE			cur->s.origin[2] += 1;
				cur->groundentity = NULL;
			}

			cur = findradius (cur, ent->s.origin, 500);
		}

		gi.sound(ent, CHAN_AUTO, gi.soundindex("powers/blackhole.wav"), 1, ATTN_NORM, 0);
		make_ball (B_BLACKHOLE, ent->s.origin, NULL);
		ent->client->pers.active_finished = level.time + 2;
	}
	else if (i == ANCHOR)
	{
		if (anchor (ent) == 1)
			ent->client->pers.active_finished = level.time + 7;
	}
	else if (i == GREN)
	{
		int i = 0;
		vec3_t	dir;
		for (i=0;i<10;i++)
		{
			vrandom(dir);
			fire_grenade (ent, ent->s.origin, dir, 100, 300+(rand()%300), 1+((float)i*0.2), 140, 0)			;
		}

		ent->client->pers.active_finished = level.time + 6;

		vis = 1;
	}
	else if (i == CASCADE)
	{
		fire_cascade (ent);
		ent->client->pers.active_finished = level.time + 4;

		vis = 1;
	}
	else if (i == I9)
	{
		give_random(ent);
		ent->client->pers.active_finished = level.time + 10;
	}
	else if (i == PROX)
	{
		vec3_t	offset;
		vec3_t	forward, right;
		vec3_t	start;
		int		damage = 180;
		float	radius;
	
		radius = 150;
	
		VectorSet(offset, 8, 8, ent->viewheight-8);
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	
		VectorScale (forward, -2, ent->client->kick_origin);
		ent->client->kick_angles[0] = -1;
	
		fire_grenade (ent, start, forward, damage, 600, 10, radius, 2);
	
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_GRENADE);
		gi.multicast (ent->s.origin, MULTICAST_PVS);
	
		PlayerNoise(ent, start, PNOISE_WEAPON);

		ent->client->pers.active_finished = level.time + 2.5;
	}
	else if (i == BLIND)
	{
		edict_t	*cur;
		cur = world;
		cur = findradius (cur, ent->s.origin, 200);
		while (cur != NULL)
		{													
			if ((cur->client && !(cur->flags & FL_OBSERVER) && cur != ent && !OnSameTeam (ent, cur)) || cur->svflags & SVF_MONSTER)
			{
				cur->blind_time = level.time + 5;
			}

			cur = findradius (cur, ent->s.origin, 200);
		}

		gi.sound(ent, CHAN_AUTO, gi.soundindex("powers/blind.wav"), 1, ATTN_NORM, 0);
		ent->client->pers.active_finished = level.time + 8;

		vis = 1;
	}
	else if (i == WEIRD)
	{
		edict_t *weird;
		weird = G_Spawn();
		VectorCopy(ent->s.origin, weird->s.origin);
		weird->movetype = MOVETYPE_NONE;
		weird->solid = SOLID_BBOX;
		weird->think = weird_think;
		weird->nextthink = level.time + 0.1;
		weird->wait = level.time + 15;
		weird->s.modelindex = gi.modelindex ("models/super2/weird/tris.md2");
		weird->style = rand()%4;
		weird->owner = ent;

		VectorSet (weird->mins, -3, -3, -3);
		VectorSet (weird->maxs, 3, 3, 3);
		weird->takedamage = DAMAGE_AIM;
		weird->health = 100;
		weird->die = BecomeExplosion1_die;

		gi.linkentity(weird);

		if (weird->style == 0)
			gi.cprintf (ent, PRINT_HIGH, "Repulsion\n");
		else if (weird->style == 1)
			gi.cprintf (ent, PRINT_HIGH, "Hallucinogen\n");
		else if (weird->style == 2)
			gi.cprintf (ent, PRINT_HIGH, "Radioactive\n");
		else if (weird->style == 3)
			gi.cprintf (ent, PRINT_HIGH, "Ammo Vacuum\n");

		gi.sound (ent, CHAN_AUTO, gi.soundindex("misc/spawn1.wav"), 1, ATTN_NORM, 0);
		ent->client->pers.active_finished = level.time + 4;
	}
	else
	{
		gi.cprintf (ent, PRINT_HIGH, "NO ACTIVE POWER %i\n", i);
	}


//	if (ent->client->pers.passive == INVIS && vis)
	if (ent->s.modelindex == invis_index && vis)
	{
		if (ent->client->pers.combo == C_JEDI)
			ent->client->pers.passive_finished = level.time + 5;
		else
			ent->client->pers.passive_finished = level.time + 3;
	}

}
///////////////////////////////////////// commands

void ComboPowers (edict_t *ent)
{
	int i = ent->client->pers.combo;
	int	a = 0, p = 0, s = 0;

	if (i == 0)
	{
		a = 0;
		p = 0;
		s = 0;
	}
	else if (i == C_MORTAL)
	{
		a = MORTAL;
		p = MORTAL;
		s = MORTAL;
	}
	else if (i == C_ROBOT)
	{
		a = ROBOT;
		p = ROBOT;
		s = ROBOT;
	}
	else if (i == C_CRIP)
	{
		a = CRIP;
		p = CRIP;
		s = CRIP;
	}
	else if (i == C_MAGE)
	{
		a = MAGE;
		p = MAGE;
		s = MAGE;
	}
	else if (i == C_PUNISH)
	{
		a = PUNISH;
		p = PUNISH;
		s = PUNISH;
	}
	else if (i == C_ASS)
	{
		a = ASS;
		p = ASS;
		s = ASS;
	}
	else if (i == C_TAFT)
	{
		a = TAFT;
		p = TAFT;
		s = TAFT;
	}
	else if (i == C_JEDI2)
	{
		a = SABER;
		p = INVIS;
		s = JUMP;
	}
	else if (i == C_BIONIC)
	{
		a = GRAPPLE;
		p = ARMOR;
		s = AP;
	}
	else if (i == C_BOMB)
	{
		a = GREN;
		p = IRAD;
		s = ISHOTS;
	}
	else if (i == C_SUN)
	{
		a = SUN;
		p = SUN;
		s = SUN;
	}
	else if (i == C_JEDI)
	{
		a = JEDI;
		p = JEDI;
		s = JEDI;
	}
	else
	{
		a = 0;
		p = 0;
		s = 0;
	}

	ent->client->pers.active = a;
	ent->client->pers.passive = p;
	ent->client->pers.special = s;
}

int SuperCommand (edict_t * ent, char * s)
{
	int type = 0;
	char	*cmd1, *cmd2, *cmd3, cb[20]={0};
	int i = 0;

	cmd1 = gi.argv(1);
	cmd2 = gi.argv(2);
	cmd3 = gi.argv(3);

	if (Q_strncasecmp(s, "admin", 5) == 0)
	{
//		gi.cprintf (ent, PRINT_HIGH, "The admin password is: %s, not %s\n", admin.password, cmd2);

		if (!strcmp(cmd2, admin.password))
		{
			if (ent->client->pers.admin == 5)
			{
				ent->client->pers.admin = 0;
				gi.bprintf (PRINT_HIGH, "%s is no longer an admin.\n", Green1(ent->client->pers.netname));
			}
			else// (ent->client->pers.admin)
			{
				ent->client->pers.admin = 5;
				gi.bprintf (PRINT_HIGH, "%s is an admin.\n", Green1(ent->client->pers.netname));
			}
		}
		else
		{
			gi.bprintf (PRINT_HIGH, "%s entered an incorrect admin password.\n", Green1(ent->client->pers.netname));
			ent->client->pers.admin++;
			if (ent->client->pers.admin > 2)
			{
				stuffcmd (ent, "disconnect\n");
			}
		}
	}
	else if (Q_strncasecmp(s, "warp", 4) == 0)
	{
		if (ent->client->pers.admin == 5)
		{
			strcpy (cb, "map ");
			strcat (cb, cmd2);
			gi.AddCommandString (cb);
		}
		else
		{
			gi.cprintf (ent, PRINT_HIGH, "Only admins can change levels.\n");
		}
	}
	else if (Q_strncasecmp(s, "lock", 4) == 0)
	{
		if (ent->client->pers.admin == 5)
		{
			strcpy (cb, "playerlock ");
			strcat (cb, cmd2);
			gi.AddCommandString (cb);
		}
		else
		{
			gi.cprintf (ent, PRINT_HIGH, "Only admins can lock players.\n");
		}
	}
	else if (Q_strncasecmp(s, "allobs", 4) == 0)
	{
		if (ent->client->pers.admin == 5)
		{
			gi.AddCommandString ("sv forceobs\n");
		}
		else
		{
			gi.cprintf (ent, PRINT_HIGH, "Only admins can force observers.\n");
		}
	}
	else if (Q_strncasecmp(s, "loc", 3) == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "location: %f %f %f\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2]);
	}
	else if (Q_strncasecmp(s, "a_inc", 5) == 0)
	{
		if ((ent->flags & FL_OBSERVER))
		{
			ent->client->pers.combo = 0;
			type = 1;
			ent->client->pers.active += 1;
			if (ent->client->pers.active > MAX_ACTIVE)
				ent->client->pers.active = 0;
		}
		else
		{
			type = 2;
		}
	}
	else if (Q_strncasecmp(s, "a_dec", 5) == 0)
	{
		if ((ent->flags & FL_OBSERVER))
		{
			ent->client->pers.combo = 0;
			type = 1;
			ent->client->pers.active -= 1;
			if (ent->client->pers.active < 0)
				ent->client->pers.active = MAX_ACTIVE;
		}
		else
		{
			type = 2;
		}
	}
	else if (Q_strncasecmp(s, "p_inc", 5) == 0)
	{
		if ((ent->flags & FL_OBSERVER))
		{
			ent->client->pers.combo = 0;
			type = 1;
			ent->client->pers.passive += 1;
			if (ent->client->pers.passive > MAX_PASSIVE)
				ent->client->pers.passive = 0;
		}
		else
		{
			type = 2;
		}
	}
	else if (Q_strncasecmp(s, "p_dec", 5) == 0)
	{
		if ((ent->flags & FL_OBSERVER))
		{
			ent->client->pers.combo = 0;
			type = 1;
			ent->client->pers.passive -= 1;
			if (ent->client->pers.passive < 0)
				ent->client->pers.passive = MAX_PASSIVE;;
		}
		else
		{
			type = 2;
		}
	}
	else if (Q_strncasecmp(s, "s_inc", 5) == 0)
	{
		if ((ent->flags & FL_OBSERVER))
		{
			ent->client->pers.combo = 0;
			type = 1;
			ent->client->pers.special += 1;
			if (ent->client->pers.special > MAX_SPECIAL)
				ent->client->pers.special = 0;
		}
		else
		{
			type = 2;
		}
	}
	else if (Q_strncasecmp(s, "s_dec", 5) == 0)
	{
		if ((ent->flags & FL_OBSERVER))
		{
			ent->client->pers.combo = 0;
			type = 1;
			ent->client->pers.special -= 1;
			if (ent->client->pers.special < 0)
				ent->client->pers.special = MAX_SPECIAL;
		}
		else
		{
			type = 2;
		}
	}
	else if (Q_strncasecmp(s, "c_inc", 5) == 0)
	{
		if ((ent->flags & FL_OBSERVER))
		{
			type = 1;
			ent->client->pers.combo += 1;
			if (ent->client->pers.combo > MAX_COMBO)
				ent->client->pers.combo = 0;

			ComboPowers (ent);
		}
		else
		{
			type = 2;
		}
	}
	else if (Q_strncasecmp(s, "c_dec", 5) == 0)
	{
		if ((ent->flags & FL_OBSERVER))
		{
			type = 1;
			ent->client->pers.combo -= 1;
			if (ent->client->pers.combo < 0)
				ent->client->pers.combo = MAX_COMBO;

			ComboPowers (ent);
		}
		else
		{
			type = 2;
		}
	}
	else if (Q_strncasecmp(s, "s_obs", 5) == 0)
	{
		if (!(ent->flags & FL_OBSERVER) && !(ent->flags & FL_HASFLAG))
		{
			if (ent->health > 0)
				ent->client->resp.score -= 1;

			MakeObserver (ent);
		}
		else if (ent->flags & FL_HASFLAG)
		{
			gi.centerprintf (ent, "You have the flag!\n");
		}
		else
		{
			gi.centerprintf (ent, "You already are an observer!\n");
		}
	}
	else if (Q_strncasecmp(s, "s_play", 6) == 0)
	{
		MakePlayer (ent);
	}
	else if (Q_strncasecmp(s, "s_pwr", 5) == 0)
	{
		type = 3;
	}
	else if (Q_strncasecmp(s, "pnum", 4) == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "pnums: %i, %i, %i\n", ent->client->pers.active, ent->client->pers.passive, ent->client->pers.special);
	}
	else if (Q_strncasecmp(s, "setp", 4) == 0)
	{
		char	*stuff;
		int		num;
		int		a=0, p=0, s=0;

		if (ent->flags & FL_OBSERVER)
		{
			stuff = gi.args();
			num = gi.argc();
	
			a = atoi(gi.argv(2));
			p = atoi(gi.argv(3));
			s = atoi(gi.argv(4));
	
			if (a > MAX_ACTIVE)
				a = 0;
			if (p > MAX_PASSIVE)
				p = 0;
			if (s > MAX_SPECIAL)
				s = 0;
	
			if (a < 0 || p < 0 || s < 0)
			{
				a = 0;
				p = 0;
				s = 0;
			}
	
			ent->client->pers.active = a;
			ent->client->pers.passive = p;
			ent->client->pers.special = s;
			ent->client->pers.combo = 0;
			type = 1;
		}
		else
		{
			type = 2;
		}
//		gi.cprintf (ent, PRINT_HIGH, "%i, %i, %i\n", a, p, s);
	}
	else if (Q_strncasecmp(s, "active", 6) == 0)
	{
//		if (ent->flags & FL_OBSERVER)
//		{
//			gi.centerprintf (ent, "Must be a player!\n");
//			return 0;
//		}

		if (ent->client->pers.active_finished < level.time)
//		{
//			gi.cprintf (ent, PRINT_HIGH, "Active recharging: %f seconds\n", ent->client->pers.active_finished - level.time);
//		}
//		else
		{
			CallActive (ent);
		}
	}
	else if (Q_strncasecmp(s, "rebind", 6) == 0)
	{
		stuffcmd (ent, "bind ins  \"use a_inc\"\n");
		stuffcmd (ent, "bind del  \"use a_dec\"\n");
		stuffcmd (ent, "bind home \"use p_inc\"\n");
		stuffcmd (ent, "bind end  \"use p_dec\"\n");
		stuffcmd (ent, "bind pgup \"use s_inc\"\n");
		stuffcmd (ent, "bind pgdn \"use s_dec\"\n");
		stuffcmd (ent, "bind ] \"use c_inc\"\n");
		stuffcmd (ent, "bind [ \"use c_dec\"\n");
		stuffcmd (ent, "bind o \"use s_obs\"\n");
		stuffcmd (ent, "bind p \"use s_pwr\"\n");
		gi.cprintf (ent, PRINT_HIGH, "Keys rebound.\n");
	}
	else if (Q_strncasecmp(s, "team", 4) == 0)
	{
		if (ctf->value == 0)
			gi.cprintf (ent, PRINT_HIGH, "You're not playing CTF!\n");
		else if (ent->flags & FL_REDTEAM)
			gi.cprintf (ent, PRINT_HIGH, "You are on the Good Team!\n");
		else if (ent->flags & FL_BLUETEAM)
			gi.cprintf (ent, PRINT_HIGH, "You are on the Evil Team!\n");
		else
			gi.cprintf (ent, PRINT_HIGH, "You're not on a team yet!\n");
	}
	else if (Q_strncasecmp(s, "score", 5) == 0)
	{
		int score, num;

		score = CTFTeamScore (FL_REDTEAM);
		num = CTFTeamCount (FL_REDTEAM, NULL);
		gi.cprintf (ent, PRINT_HIGH, "Good: %i, Players: %i\n", score, num);

		score = CTFTeamScore (FL_BLUETEAM);
		num = CTFTeamCount (FL_BLUETEAM, NULL);
		gi.cprintf (ent, PRINT_HIGH, "Evil: %i, Players: %i\n", score, num);
	}
	else if (Q_strncasecmp(s, "good", 4) == 0)
	{
		if (ctf->value == 0)
		{
			gi.cprintf (ent, PRINT_HIGH, "You're not playing CTF!\n");
		}
		else if (ctf_force->value == 1)
		{
			gi.cprintf (ent, PRINT_HIGH, "*** ctf_force is 1, cannot change teams\n");
		}
		else if (ent->flags & FL_REDTEAM)
		{
			gi.cprintf (ent, PRINT_HIGH, "*** Already on the Good team!\n");
		}
		else
		{
			MakeObserver(ent);
			ent->client->resp.score = 0;
			ent->flags &= ~FL_BLUETEAM;
			ent->flags |= FL_REDTEAM;
			return 1;
		}
	}
	else if (Q_strncasecmp(s, "evil", 4) == 0)
	{
		if (ctf->value == 0)
			gi.cprintf (ent, PRINT_HIGH, "You're not playing CTF!\n");
		else if (ctf_force->value == 1)
		{
			gi.cprintf (ent, PRINT_HIGH, "*** ctf_force is 1, cannot change teams\n");
		}
		else if (ent->flags & FL_BLUETEAM)
		{
			gi.cprintf (ent, PRINT_HIGH, "*** Already on the Evil team!\n");
		}
		else
		{
			MakeObserver(ent);
			ent->client->resp.score = 0;
			ent->flags &= ~FL_REDTEAM;
			ent->flags |= FL_BLUETEAM;
			return 1;
		}
	}
	else if (Q_strncasecmp(s, "luck", 4) == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "Your lucky number is: %i\n", ent->client->pers.lucky);
	}
	else if (Q_strncasecmp(s, "health", 6) == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "Your max health is: %i, pers->%i\n", ent->max_health, ent->client->pers.max_health);
	}
//	else if (Q_strncasecmp(s, "flag_drop", 9) == 0)
//	{
//		ent->flags &= ~(FL_HASFLAG);
//	}
	else
	{
		return 0;
	}

	if (type == 3)  // used to be "1" not "3"
	{
		if (ent->client->pers.combo == 0)
		{
			if (ent->client->pers.active < 0)
				ent->client->pers.active = 0;
			if (ent->client->pers.passive < 0)
				ent->client->pers.passive = 0;
			if (ent->client->pers.special < 0)
				ent->client->pers.special = 0;
		}

		gi.cprintf (ent, PRINT_HIGH, "%c%.16s====================\n", (game.com_ban[ent->client->pers.combo]?'*':' '),CName(ent->client->pers.combo));
		gi.cprintf (ent, PRINT_HIGH, "Active :%c%s\n", (game.act_ban[ent->client->pers.active]&&ent->client->pers.active>0?'*':' '), AName(ent->client->pers.active));
		gi.cprintf (ent, PRINT_HIGH, "Passive:%c%s\n", (game.pas_ban[ent->client->pers.passive]&&ent->client->pers.passive>0?'*':' '),PName(ent->client->pers.passive));
		gi.cprintf (ent, PRINT_HIGH, "Special:%c%s\n", (game.spe_ban[ent->client->pers.special]&&ent->client->pers.special>0?'*':' '),SName(ent->client->pers.special));
	}
	else if (type == 2)
	{
		gi.centerprintf (ent, "You can only change powers\nWhile in observer mode!\nHit 'o' to become an observer.");
	}
	else if (type == 1)
	{
		if (ent->client->pers.combo == 0)
		{
			if (ent->client->pers.active < 0)
				ent->client->pers.active = 0;
			if (ent->client->pers.passive < 0)
				ent->client->pers.passive = 0;
			if (ent->client->pers.special < 0)
				ent->client->pers.special = 0;
		}
	}

	return 1;
}


/////////////////////////////////////// observer
void MakeObserver (edict_t * ent)
{
	ent->flags |= FL_OBSERVER;
	ent->flags |= FL_NOTARGET;
	gi.bprintf (PRINT_HIGH, "%s becomes an observer\n", ent->client->pers.netname);
//	ent->client->pers.oldhand = ent->client->pers.hand;
//	ent->client->pers.hand = CENTER_HANDED;

//	Info_SetValueForKey (ent->client->pers.userinfo, "hand", "2");
//	ClientUserinfoChanged (ent, ent->client->pers.userinfo);

	CTFDrop_Flag(ent);

	if (ent->teamchain)
	{
		G_FreeEdict(ent->teamchain);
		ent->teamchain = NULL;
	}

	if (ent->angel)
	{
		G_FreeEdict(ent->angel);
		ent->angel = NULL;
	}

	if (ent->flags & FL_RANDOM_ACT)
	{
		ent->client->pers.active = RAND_ACT;
		ent->flags &= ~FL_RANDOM_ACT;
	}
	if (ent->flags & FL_RANDOM_PAS)
	{
		ent->client->pers.passive = RAND_PAS;
		ent->flags &= ~FL_RANDOM_PAS;
	}
	if (ent->flags & FL_RANDOM_SPE)
	{
		ent->client->pers.special = RAND_SPE;
		ent->flags &= ~FL_RANDOM_SPE;
	}
	
	PutClientInServer(ent);
}

void MakePlayer (edict_t * ent)
{
	if (playerlock->value == 1)
	{
		gi.cprintf (ent, PRINT_HIGH, "Player lock is on!\n");
		return;
	}
	else if (ent->client->pers.active && ent->client->pers.passive && ent->client->pers.special && ent->flags & FL_OBSERVER)
	{
		if ((ent->client->pers.active == MORTAL || ent->client->pers.passive == MORTAL || ent->client->pers.special == MORTAL) 
			&& !(ent->client->pers.active == MORTAL && ent->client->pers.passive == MORTAL && ent->client->pers.special == MORTAL))
		{
			gi.centerprintf (ent, "To be a Mere Mortal,\nYou must select Mere Mortal for\nall of your powers!");
			return;
		}
		if ((ent->client->pers.active == ROBOT || ent->client->pers.passive == ROBOT || ent->client->pers.special == ROBOT) 
			&& !(ent->client->pers.active == ROBOT && ent->client->pers.passive == ROBOT && ent->client->pers.special == ROBOT))
		{
			gi.centerprintf (ent, "To be a Killer Robot,\nYou must select Killer Robot for\nall of your powers!");
			return;
		}
		if ((ent->client->pers.active == CRIP || ent->client->pers.passive == CRIP || ent->client->pers.special == CRIP) 
			&& !(ent->client->pers.active == CRIP && ent->client->pers.passive == CRIP && ent->client->pers.special == CRIP))
		{
			gi.centerprintf (ent, "To be a Cripple,\nYou must select Cripple for\nall of your powers!");
			return;
		}
		if ((ent->client->pers.active == MAGE || ent->client->pers.passive == MAGE || ent->client->pers.special == MAGE) 
			&& !(ent->client->pers.active == MAGE && ent->client->pers.passive == MAGE && ent->client->pers.special == MAGE))
		{
			gi.centerprintf (ent, "To be an Archmage,\nYou must select Archmage for\nall of your powers!");
			return;
		}
		if ((ent->client->pers.active == PUNISH || ent->client->pers.passive == PUNISH || ent->client->pers.special == PUNISH) 
			&& !(ent->client->pers.active == PUNISH && ent->client->pers.passive == PUNISH && ent->client->pers.special == PUNISH))
		{
			gi.centerprintf (ent, "To be a Punisher,\nYou must select Punisher for\nall of your powers!");
			return;
		}
		if ((ent->client->pers.active == ASS || ent->client->pers.passive == ASS || ent->client->pers.special == ASS) 
			&& !(ent->client->pers.active == ASS && ent->client->pers.passive == ASS && ent->client->pers.special == ASS))
		{
			gi.centerprintf (ent, "To be an Assassin,\nYou must select Assassin for\nall of your powers!");
			return;
		}

		if (((ent->client->pers.active < 0) || (ent->client->pers.passive < 0) || (ent->client->pers.special < 0)) && (ent->client->pers.combo == 0))
		{
			gi.centerprintf (ent, "To use a triple power,\nYou must select that power for\nall of your powers!");
			return;
		}

		if (ent->client->pers.combo == 0)
		{
			if (game.act_ban[ent->client->pers.active] == 1 || game.pas_ban[ent->client->pers.passive] == 1 || game.spe_ban[ent->client->pers.special] == 1)
			{
				gi.centerprintf (ent, "You have selected powers\nThat have been banned on\nThis server!\nThey are marked with a *\n");
				return;
			}
		}
		else
		{
			if (game.com_ban[ent->client->pers.combo] == 1)
			{
				gi.centerprintf (ent, "You have selected a combo\nThat has been banned on\nThis server!\nIt is marked with a *\n");
				return;
			}
		}

		if (ent->client->pers.active == RAND_ACT)
		{
			ent->flags |= FL_RANDOM_ACT;
		}
		else
		{
			ent->flags &= ~FL_RANDOM_ACT;
		}

		if (ent->client->pers.passive == RAND_PAS)
		{
			ent->flags |= FL_RANDOM_PAS;
		}
		else
		{
			ent->flags &= ~FL_RANDOM_PAS;
		}
		
		if (ent->client->pers.special == RAND_SPE)
		{
			ent->flags |= FL_RANDOM_SPE;
		}
		else
		{
			ent->flags &= ~FL_RANDOM_SPE;
		}

		if (!(ent->flags & (FL_REDTEAM|FL_BLUETEAM)) && ctf->value)
		{
			CTFSetTeam(ent);
		}
		
		ent->flags &= ~FL_OBSERVER;
		ent->flags &= ~FL_NOTARGET;
		gi.bprintf (PRINT_HIGH, "%s becomes a player\n", ent->client->pers.netname);
//		ent->client->pers.hand = ent->client->pers.oldhand;

//		itoa (ent->client->pers.hand, duh, 10);
//		Info_SetValueForKey (ent->client->pers.userinfo, "hand", duh);
//		ClientUserinfoChanged (ent, ent->client->pers.userinfo);
		
		PutClientInServer(ent);

		if (ent->client->pers.special == AODEATH || ent->client->pers.special == AOLIFE || ent->client->pers.passive == MAGE || ent->client->pers.special == AOMERCY)
		{
			spawn_angel(ent);
		}

		return;
	}
	else if (!(ent->flags & FL_OBSERVER))
	{
		gi.centerprintf(ent, "You are already a player!\n");
	}
	else
	{
		gi.centerprintf(ent, "You must select your powers first!\nUse INS DEL HOME END PGUP PGDN\nto cycle through the powers.\n");
	}
}

char * GetSub ()
{
	int r=0;

	srand((unsigned)time(NULL));
	r = rand()%23;
	
	if (r == 0)
		return "Judgment Day";
	else if (r == 1)
		return "Terror from the Deep";
	else if (r == 2)
		return "Tides of Darkness";
	else if (r == 3)
		return "First Blood Part II";
	else if (r == 4)
		return "Battle at Antares";
	else if (r == 5)
		return "The Wrath of Khan";
	else if (r == 6)
		return "Freddy's Revenge";
	else if (r == 7)
		return "The Second Story";
	else if (r == 8)
		return "The Empire Strikes Back";
	else if (r == 9)
		return "On The Rocks";
	else if (r == 10)
		return "Electric Boogaloo";
	else if (r == 11)
		return "Die Harder";
	else if (r == 12)
		return "The New Batch";
	else if (r == 13)
		return "Lost in New York";
	else if (r == 14)
		return "The Road Warrior";
	else if (r == 15)
		return "The Smell of Fear";
	else if (r == 16)
		return "Back in the Habit";
	else if (r == 17)
		return "Their First Assignment";
	else if (r == 18)
		return "When Nature Calls";
	else if (r == 19)
		return "Dead By Dawn";
	else if (r == 20)
		return "Vengeance of the Kilrathi";
	else if (r == 21)
		return "Jews in Space";
	else if (r == 22)
		return "The Search for More Money";
}

// Icon subcode - goes at end of g_cmds.c

int active_icon (edict_t *ent)
{
	int i = ent->client->pers.active;
//	int i = 0;

	if (i == 0)
		return gi.imageindex ("i_fixme");
	else if (i == MORTAL)
		return gi.imageindex ("tp_mortal");
	else if (i == ROBOT)
		return gi.imageindex ("tp_robot");
	else if (i == CRIP)
		return gi.imageindex ("tp_cripple");
	else if (i == MAGE)
		return gi.imageindex ("tp_archmage");
	else if (i == PUNISH)
		return gi.imageindex ("tp_punisher");
	else if (i == ASS)
		return gi.imageindex ("tp_assassin");
	else if (i == TAFT)
		return gi.imageindex ("tp_taft");

	else if (i == JEDI && ent->wait == 3)
		return gi.imageindex ("tp_jedi2");
	else if (i == JEDI)
		return gi.imageindex ("tp_jedi1");

	else if (i == SUN)
		return gi.imageindex ("tp_sun");

	else if (i == FLAME)
		return gi.imageindex ("ap_flameball");
	else if (i == THROW)
		return gi.imageindex ("ap_kineticthrow");
	else if (i == FREEZE)
		return gi.imageindex ("ap_freeze");
	else if (i == BLOW)
		return gi.imageindex ("ap_deathblow");
	else if (i == TELEPORT)
		return gi.imageindex ("ap_teleport");
	else if (i == BEACON)
		return gi.imageindex ("ap_beacon");
	else if (i == CARD)
		return gi.imageindex ("ap_cards");
	else if (i == WAIL)
		return gi.imageindex ("ap_banshee");
	else if (i == GRAV)
		return gi.imageindex ("ap_gravity");
	else if (i == ANTS)
		return gi.imageindex ("ap_ants");
	else if (i == BLAST)
		return gi.imageindex ("ap_psionic");
	else if (i == OPTIC)
		return gi.imageindex ("ap_optic");
	else if (i == SABER)
		return gi.imageindex ("ap_lightsaber");
	else if (i == GRAPPLE)
		return gi.imageindex ("ap_claw");
	else if (i == ANCHOR)
		return gi.imageindex ("ap_poweranchor");
	else if (i == SUCK)
		return gi.imageindex ("ap_blackhole");
	else if (i == CASCADE)
		return gi.imageindex ("ap_flamecascade");
	else if (i == GREN)
		return gi.imageindex ("ap_grenswarm");
	else if (i == I9)
		return gi.imageindex ("ap_impulse9");
	else if (i == PROX)
		return gi.imageindex ("ap_prox");
	else if (i == BLIND)
		return gi.imageindex ("ap_blind");
	else if (i == WEIRD)
		return gi.imageindex ("ap_weird");
	else
		return gi.imageindex ("i_fixme");
}
int passive_icon (edict_t *ent)
{
	int i = ent->client->pers.passive;
//	int i = 0;

	if (i == 0)
		return gi.imageindex ("i_fixme");
	else if (i == MORTAL)
		return gi.imageindex ("tiny");
	else if (i == ROBOT)
		return gi.imageindex ("tiny");
	else if (i == CRIP)
		return gi.imageindex ("tiny");
	else if (i == MAGE)
		return gi.imageindex ("tiny");
	else if (i == PUNISH)
		return gi.imageindex ("tiny");
	else if (i == ASS)
		return gi.imageindex ("tiny");
	else if (i == TAFT)
		return gi.imageindex ("tiny");
	else if (i == JEDI)
		return gi.imageindex ("tiny");
	else if (i == SUN)
		return gi.imageindex ("tiny");

	else if (i == BOOT)
		return gi.imageindex ("pp_boot");
	else if (i == ELASTIC)
		return gi.imageindex ("pp_elastic");
	else if (i == REPULSE)
		return gi.imageindex ("pp_repulsion");
	else if (i == IRAD)
		return gi.imageindex ("pp_immune");
	else if (i == REGEN)
		return gi.imageindex ("pp_regen");
	else if (i == RADIO)
		return gi.imageindex ("pp_radio");
	else if (i == FORCE)
		return gi.imageindex ("pp_forcefield");
	else if (i == INVIS)
		return gi.imageindex ("pp_invis");
	else if (i == ABSORB)
		return gi.imageindex ("pp_energy");
	else if (i == BULLET)
		return gi.imageindex ("pp_bullet");
	else if (i == PRISMATIC)
		return gi.imageindex ("pp_prism");
	else if (i == LIFE)
		return gi.imageindex ("pp_lifewell");
	else if (i == ARMOR)
		return gi.imageindex ("pp_metalform");
	else if (i == SPEED)
		return gi.imageindex ("pp_superspeed");
	else if (i == FLY)
		return gi.imageindex ("pp_flying");
	else if (i == LFORM)
		return gi.imageindex ("pp_liquid");
	else if (i == DENSE)
		return gi.imageindex ("pp_hdense");
	else if (i == SHIN)
		return gi.imageindex ("pp_radiance");
	else if (i == CONNECT)
		return gi.imageindex ("pp_connect");
	else
		return gi.imageindex ("i_fixme");
}

int special_icon (edict_t *ent)
{
	int i = ent->client->pers.special;
//	int i = 0;

	if (i == 0)
		return gi.imageindex ("i_fixme");
	else if (i == MORTAL)
		return gi.imageindex ("tiny");
	else if (i == ROBOT)
		return gi.imageindex ("tiny");
	else if (i == CRIP)
		return gi.imageindex ("tiny");
	else if (i == MAGE)
		return gi.imageindex ("tiny");
	else if (i == PUNISH)
		return gi.imageindex ("tiny");
	else if (i == ASS)
		return gi.imageindex ("tiny");
	else if (i == TAFT)
		return gi.imageindex ("tiny");
	else if (i == JEDI)
		return gi.imageindex ("tiny");
	else if (i == SUN)
		return gi.imageindex ("tiny");
	
	else if (i == HASTE)
		return gi.imageindex ("sp_haste");
	else if (i == JUMP)
		return gi.imageindex ("sp_superjump");
	else if (i == ISHOTS)
		return gi.imageindex ("sp_invisshots");
	else if (i == FSHOTS)
		return gi.imageindex ("sp_fastproj");
	else if (i == VAMP)
		return gi.imageindex ("sp_vampiric");
	else if (i == AMMO)
		return gi.imageindex ("sp_ammouse");
	else if (i == AP)
		return gi.imageindex ("sp_piercing");
	else if (i == ELEC)
		return gi.imageindex ("sp_electric");
	else if (i == SNIPER)
		return gi.imageindex ("sp_sniper");
	else if (i == AGG)
		return gi.imageindex ("sp_aggravated");
	else if (i == BERSERK)
		return gi.imageindex ("sp_rage");
	else if (i == AODEATH)
		return gi.imageindex ("sp_angeldeath");
	else if (i == AOLIFE)
		return gi.imageindex ("sp_angellife");
	else if (i == AOMERCY)
		return gi.imageindex ("sp_angelmercy");
	else if (i == BLOSSOM)
		return gi.imageindex ("sp_deathblossom");
	else if (i == LUCK)
		return gi.imageindex ("sp_luck");
	else if (i == STR)
		return gi.imageindex ("sp_strength");
	else if (i == FUNK)
		return gi.imageindex ("sp_funk");
	else
		return gi.imageindex ("i_fixme");
}

//////////////////////////////////////////////// \SH

