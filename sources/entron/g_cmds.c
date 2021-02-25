#include "g_local.h"
#include "b_player.h"

void SP_monster_centurion (edict_t *self);
void SP_monster_drone (edict_t *self);
void SP_monster_guard (edict_t *self);
void SP_monster_pod (edict_t *self);
void SP_monster_podmedic (edict_t *self);
void SP_monster_priest (edict_t *self);
void SP_monster_sdriver (edict_t *self);
void SP_monster_smaster (edict_t *self);
void SP_monster_sten (edict_t *self);
void SP_monster_boss1 (edict_t *self);
void SP_monster_boss2 (edict_t *self);
void SP_monster_zombie (edict_t *self);
void SP_monster_pkf (edict_t *self);
void SP_monster_bactn (edict_t *self);
void SP_slave (edict_t *self);
void SP_sbarrow (edict_t *self);
void SP_shammer (edict_t *self);
void SP_sore (edict_t *self);
void SP_misc_book (edict_t *self);
void SpawnItem (edict_t *ent, gitem_t *item);
void camera_enemy (edict_t *self, edict_t *user);
void Cmd_Weapon_Mode(edict_t *ent);
void Show_camera_f(edict_t *self, int cam_number, float fade);
//extern spawn_t	spawns;

void tracegravity (map_node_t *node1, map_node_t *node2);
void GoScript(edict_t *ent);


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
	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	gi.cprintf (ent, PRINT_HIGH, "wave\n");
	ent->s.frame = FRAME_Wave_start - 1;
	ent->client->anim_end = FRAME_Wave_end;
}

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
void Cmd_Plant_f (edict_t *ent);

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

	else if (Q_stricmp (cmd, "node") == 0)
		draw_nodes(ent);

	else if (Q_stricmp (cmd, "trace") == 0)
      tracegravity (&g_nodes[4], &g_nodes[5]);

	else if (Q_stricmp (cmd, "plant") == 0)
      Cmd_Plant_f(ent);

   else if (Q_stricmp (cmd, "3rd") == 0)
      camera_3rd(ent);

   else if (Q_stricmp (cmd, "fire") == 0)
      Cmd_Weapon_Mode(ent);

   else if (Q_stricmp (cmd, "camera") == 0)
      Show_camera_f(ent, -1, .1);

   else if (Q_stricmp (cmd, "chase") == 0)
      camera_enemy(ent, ent);
   else if (Q_stricmp (cmd, "script") == 0)
      GoScript(ent);
   else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}



void Cmd_Plant_f (edict_t *ent)
{
	char		*name;
   vec3_t   start, right, forward, offset;
   edict_t *spawn;
   gitem_t *item;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 82, 0, ent->viewheight+14);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

   name = gi.args();
	if (gi.argc() < 2)
		{
		gi.dprintf ("DEBUG: Plant <monstername> [<spawnflags>]\n");
		return;
		}
	spawn = G_Spawn();
	if (gi.argc() > 2)
		spawn->spawnflags = atoi(gi.argv(2));
	name = gi.argv(1);
	VectorCopy(start, spawn->s.origin);
	if (!Q_stricmp(name, "pod"))
      {          
      SP_monster_pod(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);   
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
	if (!Q_stricmp(name, "sten"))
      {
      SP_monster_sten(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);   
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
	if (!Q_stricmp(name, "priest"))
      {
      SP_monster_priest(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);   
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
	if (!Q_stricmp(name, "centurion"))
      {
      SP_monster_centurion(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);   
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
	if (!Q_stricmp(name, "podmedic"))
      {
      SP_monster_podmedic(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);   
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
	if (!Q_stricmp(name, "drone"))
      {
      SP_monster_drone(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);   
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
	if (!Q_stricmp(name, "guard"))
      {
      SP_monster_guard(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);   
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
   if (!Q_stricmp(name, "slave_driver"))
      {
      SP_monster_sdriver(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);   
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
   if (!Q_stricmp(name, "slave_master"))
      {
      SP_monster_smaster(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);   
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
   if (!Q_stricmp(name, "pkf"))
      {
      SP_monster_pkf(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);   
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
   if (!Q_stricmp(name, "zombie_priest"))
      {
      SP_monster_zombie (spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);   
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
   if (!Q_stricmp(name, "guardian"))
      {
      SP_monster_boss1(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
   if (!Q_stricmp(name, "overseer"))
      {
      SP_monster_boss2(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
   if (!Q_stricmp(name, "bactn"))
      {
      SP_monster_bactn(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
   if (!Q_stricmp(name, "slave"))
      {
      SP_slave(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
   if (!Q_stricmp(name, "slave_barrow"))
      {
      SP_sbarrow(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
   if (!Q_stricmp(name, "slave_hammer"))
      {
      SP_shammer(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
   if (!Q_stricmp(name, "slave_ore"))
      {
      SP_sore(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
   if (!Q_stricmp(name, "book"))
      {
      VectorCopy(start, spawn->s.origin);
      SP_misc_book(spawn);
      VectorSubtract (ent->s.origin, spawn->s.origin, forward);
	   VectorNormalize (forward);   
      spawn->s.angles[YAW] = vectoyaw(forward);
      return;
      }
   item = FindItem (name);
   if (!item)
		{
		G_FreeEdict(spawn);
      return;
		}
   VectorSubtract (ent->s.origin, spawn->s.origin, forward);
   VectorNormalize (forward);   
   spawn->s.angles[YAW] = vectoyaw(forward);
   SpawnItem (spawn, item);
}


void Cmd_Weapon_Mode(edict_t *ent)
{
   if (!ent->client->pers.weapon)
      return;
   if (!strcmp(ent->client->pers.weapon->pickup_name, "u23k"))
      {
		ent->client->newweapon = FindItem ("u23k2");
		ChangeWeapon (ent);
		return;
		}
   if (!strcmp(ent->client->pers.weapon->pickup_name, "u23k2"))
      {
		ent->client->newweapon = FindItem ("u23k");
		ChangeWeapon (ent);
		/*
      if (!ent->client->weapon_mode)
         {
         ent->client->weapon_mode = 1;
         gi.cprintf (ent, PRINT_MEDIUM, "-> WEAPON: Panic Sniper Mode\n");
         }
		*/
		return;
		}

   if (!strcmp(ent->client->pers.weapon->pickup_name, "Rifle"))
      {
      if (!ent->client->weapon_mode)
         {
         ent->client->weapon_mode = 1;
         gi.cprintf (ent, PRINT_MEDIUM, "-> WEAPON: Panic Sniper Mode\n");
         }
      else if (ent->client->weapon_mode == 1)
         {
         ent->client->weapon_mode = 2;
         gi.cprintf (ent, PRINT_MEDIUM, "-> WEAPON: Automatic Rifle\n");
         }
      else 
         {  
         ent->client->weapon_mode = 0;
         gi.cprintf (ent, PRINT_MEDIUM, "-> WEAPON: Normal Rifle Mode\n");
         }
      if (ent->client->weapon_mode == 2)
         {
         ent->client->pers.weapon->ammo = "Clip";
	      ent->client->ammo_index = ITEM_INDEX(FindItem("Clip"));
         }
      else
         {
         ent->client->pers.weapon->ammo = "Sniper Rounds";
	      ent->client->ammo_index = ITEM_INDEX(FindItem("Sniper Rounds"));
         }
      return;
     }
   if (!strcmp(ent->client->pers.weapon->pickup_name, "Assimilator"))
      {
      if (!ent->client->weapon_mode)
         {
         gi.cprintf (ent, PRINT_MEDIUM, "-> WEAPON: Panic 6-pack Assimilator Mode\n");
         ent->client->weapon_mode = 1;
         }  
      else if (ent->client->weapon_mode == 1)
         {
         gi.cprintf (ent, PRINT_MEDIUM, "-> WEAPON: Heat Seek Rocket\n");
         ent->client->weapon_mode = 2;
         }  
      else 
         {
         ent->client->weapon_mode = 0;
         gi.cprintf (ent, PRINT_MEDIUM, "-> WEAPON: Normal Assimilator Mode\n");
         }  
     }

   if (!strcmp(ent->client->pers.weapon->pickup_name, "Staff"))
      {
      if (!ent->client->weapon_mode)
         {
         gi.cprintf (ent, PRINT_MEDIUM, "-> WEAPON: Lightning mode\n");
         ent->client->weapon_mode = 1;
         }  
      else 
         {
         gi.cprintf (ent, PRINT_MEDIUM, "-> WEAPON: Fire ball mode\n");
         ent->client->weapon_mode = 0;
         }  
     }
}