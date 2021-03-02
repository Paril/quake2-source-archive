#include "g_local.h"
#include "m_player.h"
#include "laser.h"
#include "x_radio.h"

void ChasecamRemove                (edict_t *ent, char *opt);
void PrintOtherClass               (entity *ent);
void PrintClass_ToOther            (entity *ent);
void Print_ClassProperties         (entity *ent);
void SelectRandom                  (edict_t *ent);
void Print_Msg                     (edict_t *who, char *msg);
void TeamMasterCanShutUpPlayer     (entity *master, char *cmd);
void CTFSay_Team                   (edict_t *who, char *msg);
void TeamMasterCanKickPlayer       (entity *master, char *cmd);
void Levitation_Spell              (entity *ent);
void TeamMasterCanOffPlayersRadio  (entity *master, char *cmd);
void Radio_Player                  (entity *ent, entity *other, char *msg);
void X_Radio_Power_f               (edict_t *self, char *state);
void ClassFunction                 (edict_t *ent, int i);
void CTFDeadDropFlag               (edict_t *self);
void Cmd_Spell_f                   (edict_t *ent);
void Flag_StatusReport             (edict_t *self);
void ClientUserinfoChanged         (edict_t *ent, char *userinfo);
void Self_Origin                   (edict_t *ent);
void Cmd_Custom_f                  (edict_t *ent);
void Cmd_Speed_f                   (edict_t *ent);
void Cmd_Armor_f                   (edict_t *ent);
void Cmd_Done_f                    (edict_t *ent);
void CTFDeadDropFlag               (edict_t *self);
void MageJump1                     (edict_t *ent);
void MageJump2                     (edict_t *ent);
void Cmd_Class_f                   (edict_t *ent);
void Cmd_Build_f                   (edict_t *ent);

char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!G_ClientExists(ent))
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

   // ++QMENU++
   //
   // See if the menu is visible, and if it is, handle it accordingly.

  // ++QMENU++
   //
   // See if the menu is visible, and if it is, handle it accordingly.

   if (ent->client->showmenu)
   {
     Menu_Dn(ent);
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

   // ++QMENU++
   //
   // See if the menu is visible, and if it is, handle it accordingly.


   // ++QMENU++
   //
   // See if the menu is visible, and if it is, handle it accordingly.

   if (ent->client->showmenu)
   {
     Menu_Up(ent);
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

	if (!sv_cheats->value)
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

	if (!sv_cheats->value)
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

	if (!sv_cheats->value)
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

	if (!sv_cheats->value)
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

	if (ent->client->pers.weapon == FindItem("Grenades"))
	{
    	if (Q_stricmp (s, "grenades") == 0  && !ent->client->grenade_time && ent->playerclass && ent->deadflag != DEAD_DEAD)
		{
			if (ent->client->gr_type == 1)
			{
				ent->client->gr_type = 0;
				gi.cprintf (ent, PRINT_HIGH, "Normal grenades\n");
			}
   	    	else
			{
				ent->client->gr_type = 1;
				gi.cprintf (ent, PRINT_HIGH, "Special grenades\n");
			}
			return;
		}
	}

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

	if (cl->resp.s_team == 0)
	{
		Cmd_Team_f (ent);
		return;
	}
	else if (ent->playerclass == 0 && cl->resp.s_team)
	{
		Cmd_Class_f (ent);
	    return;
	}

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

	// ++QMENU++
//
// if we are in menu mode, call the menu handler.

// ++QMENU++
//
// if we are in menu mode, call the menu handler.

   if (ent->client->showmenu)
   {
      Menu_Sel(ent);
      return;
   }


	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
        ent->client->pers.selected_item = 0; // Vic
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

    if (ent->playerclass == 0)
        return;

	if ((int)(ripflags->value) & RF_SUICIDE_KICK)
	{
		gi.bprintf (PRINT_MEDIUM, "%s kicked for suiciding\n", ent->client->pers.netname);
 		stuffcmd (ent, "disconnect\n");
		return;
    }

	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;

	player_die (ent, ent, ent, 500, vec3_origin);

    ent->deadflag = DEAD_DEAD;
	respawn(ent);
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
	if (ent->client->showmenu)
		Menu_Close(ent);
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
	int		i, count;
	char	small[64],	large[1280];
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
=================
Cmd_id_f
New function to print player name and dist
By Reven
=================
*/

void Cmd_id_f (edict_t *ent)
{
    char stats[500];
    vec3_t  start, forward, end;
	trace_t tr;

    VectorCopy(ent->s.origin, start);
    start[2] += ent->viewheight;
    AngleVectors(ent->client->v_angle, forward, NULL, NULL);
    VectorMA(start, 8192, forward, end);
    tr = gi.trace(start, NULL, NULL, end, ent,MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

    if (tr.ent->client)
	{
		strcpy (stats, "xv 32 yv 8 picn inventory ");
		strcat_(stats, "xv 52 yv 36 string2 Name:\n");
		strcat_(stats, "xv 52 yv 54 %16s\n",tr.ent->client->pers.netname);
		strcat_(stats, "xv 52 yv 66 string2 Class:\n",tr.ent->client->pers.netname);

		strcat_(stats, "xv 52 yv 75 friendly %16s\n",tr.ent->playerclasss);
	}
	
	ent->client->showmsg = true;
	ent->client->showinventory = false;

	gi.WriteByte (svc_layout);
	gi.WriteString (stats);
	gi.unicast (ent, true);
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
	//flood protection code
    int time_left=0; 

	//end flood protection code
	if (gi.argc () < 2 && !arg0)
		return;

	if (ent->teamstate & STATE_NOTSAY)
		return;

	if (ent->deadflag == DEAD_DEAD)
		return; // corpses can't talk

// Begin - Message Flooding Protection Code. 

// if not already in a timer situation then.. 
	if (!(ent->client->flood_timer > PRESENT_TIME)) 
// if attempt to post more than 3 msgs in 1 unit of time. 
    if ((ent->client->flood_num_msgs >= 3) && (ent->client->flood_post_time+1 > PRESENT_TIME))
	{
		ent->client->flood_timer = PRESENT_TIME + 30; // Start 30 sec Timer. 
		ent->client->flood_post_time = PRESENT_TIME; // Reset First Post Time. 
		ent->client->flood_num_msgs = 0;
	} // Reset Num Messages posted 


	// Timer is running so NO POSTS ALLOWED. Just printf and exit.. 
	if (ent->client->flood_timer > PRESENT_TIME)
	{
		time_left = (int)(ent->client->flood_timer - PRESENT_TIME); 
		gi.cprintf(ent, PRINT_HIGH, "You can't talk for %i more seconds..\n",time_left); 
		return;
	} 

// End - Message Flood Protection 

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

		if (!G_ClientExists(other))
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
    // Increment Flood Msg Count 
    ent->client->flood_num_msgs += 1; 

    // Capture first post time 
     if (ent->client->flood_post_time + 1 < PRESENT_TIME) 
      ent->client->flood_post_time = PRESENT_TIME; 
}

void MyTeam_Sel (edict_t *ent, int choice) 
{
	switch (choice)
	{
	case 1:
	   JoinTeam (ent, 1);
       stuffcmd (ent, "menu_class\n");
	break;
	case 2:
	   JoinTeam (ent, 2);
       stuffcmd (ent, "menu_class\n");
	break;
	default:
	   stuffcmd (ent, "spectator 1\n");
	break;
	}
} // MyWave_Sel

//========================================
// Prints fmt to self's team
void strcpy_ (char *s, char *fmt, ...)
{
    va_list ap;
    static char st[100];

    va_start (ap, fmt);
    vsprintf (st, fmt, ap);
    va_end (ap);

    strcpy (s, st);
}

void Cmd_Team_f (edict_t *ent)
{
    static int num1, num2, i;
	static char team1[32], team2[32];
	edict_t *other;

	num1 = num2 = 0;

	if (ent->client->resp.s_team > 0)
		return;

	// Check to see if the menu is already open

	if (ent->client->showscores || ent->client->showinventory || ent->client->showmenu || ent->client->showmsg)
        return;

	for (i = 0; i <= maxclients->value; i++)
	{
		other = &g_edicts[i];

		if (!G_ClientExists(other))
			continue;

		if (other->client->resp.s_team == 1)
			num1++;
		else if (other->client->resp.s_team == 2)
			num2++;
	}

	if ((int)ripflags->value & RF_FORCE_JOIN)
	{
		if (num1 > num2)
		{
			JoinTeam (ent, 2);
       		stuffcmd (ent, "menu_class\n");
	     	return;
		}
		else if (num2 > num1)
		{
        	JoinTeam (ent, 1);
		    stuffcmd (ent, "menu_class\n");
		    return;
		}
	}

   // send the layout
   Menu_Title(ent,"Select your team");

   strcpy_ (team1, "Red Team  (%d players)", num1);
   strcpy_ (team2, "Blue Team (%d players)", num2);

   Menu_Add(ent, team1);
   Menu_Add(ent, team2);
   Menu_Add(ent, "Chase camera");

   // Setup the User Selection Handler

   ent->client->usr_menu_sel = MyTeam_Sel;
   Menu_Open(ent);

} // Cmd_MyWave_f

void nothing (entity *ent)
{
}

void MyClass_Sel (edict_t *ent, int choice) 
{
    if (choice == 1)
	{
			if (ent->playerclass == 0)
				ClassFunction(ent, 1);
		    else
			    stuffcmd (ent, "warrior\n");
	}  
	else if (choice == 2)
	{
			if (ent->playerclass == 0)
				ClassFunction(ent, 2);
		    else
			    stuffcmd (ent, "mage\n");
    }
	else if (choice == 3)
	{
    		if (ent->playerclass == 0)
				ClassFunction(ent, 3);
		    else
			    stuffcmd (ent, "thief\n");
    }
	else if (choice == 4)
	{
			if (ent->playerclass == 0)
				ClassFunction(ent, 4);
		    else
			    stuffcmd (ent, "infantry\n");
    }
    else if (choice == 5)
	{
			if (ent->playerclass == 0)
				ClassFunction(ent, 5);
		    else
			    stuffcmd (ent, "miner\n");
    }
	else if (choice == 6)
	{
			if (ent->playerclass == 0)
				ClassFunction(ent,6);
		    else
			    stuffcmd (ent, "scientist\n");
    }
	else if (choice == 7)
	{
			if (ent->playerclass == 0)
				ClassFunction(ent, 7);
		    else
			    stuffcmd (ent, "flamer\n");
    }
	else if (choice == 8)
	{
			if (ent->playerclass == 0)
				ClassFunction(ent, 8);
		    else
			    stuffcmd (ent, "trooper\n");
    }
	else if (choice == 9)
	{
			if (ent->playerclass == 0)
				ClassFunction(ent, 9);
		    else
			    stuffcmd (ent, "ghost\n");
    }
	else if (choice == 0)
	{
		nothing (ent);
		stuffcmd (ent, "custom\n");
	}

} // MyWave_Sel

void Cmd_Class_f (edict_t *ent)
{
	// Check to see if the menu is already open

   if (ent->client->showscores || ent->client->showinventory ||
        ent->client->showmenu || ent->client->showmsg)
        return;

   // send the layout
   
   Menu_Title(ent,"Select your class");
   Menu_Add(ent,"Warrior      ");
   Menu_Add(ent,"Necromancer  ");
   Menu_Add(ent,"Thief        ");
   Menu_Add(ent,"Infantry     ");
   Menu_Add(ent,"Miner        ");
   Menu_Add(ent,"Scientist    ");
   Menu_Add(ent,"Flamer       ");
   Menu_Add(ent,"Trooper      ");
   Menu_Add(ent,"Ghost        ");
   Menu_Add(ent,"Build        ");

    // Setup the User Selection Handler

   ent->client->usr_menu_sel = MyClass_Sel;
   Menu_Open(ent);

} // Cmd_MyWave_f

void Cmd_DetPipes_f (edict_t *ent, char *cmd);

void Cmd_Main_f (entity *ent, char *cmd1)
{
	if (!ent->playerclass)
		return;

	if (ent->playerclass == 4 || ent->client->resp.it == 4)
        SP_LaserSight (ent, cmd1);
	else if (ent->playerclass == 5 || ent->client->resp.it == 5)
		Cmd_DetPipes_f (ent, cmd1);
    else if (ent->playerclass == 2 || ent->client->resp.it == 2)
		Cmd_Spell_f (ent);
	else if (ent->playerclass == 6 || ent->client->resp.it == 6)
		Cmd_Build_f (ent);
}

qboolean check_stand (edict_t *ent)
{
	vec3_t v1, v2;
	trace_t tr;

	if (ent->client->chasetoggle)
	{
		gi.cprintf (ent, PRINT_HIGH, "You can't feign while chasing yourself.\n");
		return false;
	}

	if (ent->deadflag == DEAD_DEAD)
		return false;

	if (!(ent->ripstate & STATE_FEINING))
	{
		edict_t *ent1;

		ent1 = NULL;

		if ((ent1 = findradius (ent1, ent->s.origin, 64)) != NULL && ent1 != ent)
		{
			gi.cprintf (ent, PRINT_HIGH, "You can feign on the top of another spy\n");
			return false;
		}
		else
			return true;
	}

	VectorSet (v1, ent->s.origin[0] + 16, ent->s.origin[1], ent->s.origin[2] + 20);
	VectorSet (v2, ent->s.origin[0] - 16, ent->s.origin[1], ent->s.origin[2] + 20);

    tr = gi.trace (ent->s.origin, NULL, NULL, v1, ent, MASK_SHOT);

	if (tr.fraction < 0)
	{
		 if (tr.ent && tr.ent->groundentity)
		 {
			 if (tr.ent->mass && tr.ent->deadflag != DEAD_DEAD)
			 {
				  gi.cprintf (ent, PRINT_HIGH, "You can't get up while someone\nis standing on you.\n\n");
			 }

			return false;
		 }
	}

    tr = gi.trace (ent->s.origin, NULL, NULL, v2, ent, MASK_SHOT);

	if (tr.fraction < 0)
	{
		 if (tr.ent && tr.ent->groundentity)
		 {
			 if (tr.ent->mass)
			 {
				 if (!tr.ent->client)
					  gi.cprintf (ent, PRINT_HIGH, "Something stands on you.\n");
				 else
					  gi.cprintf (ent, PRINT_HIGH, "Somebody stands on you.\n");
			 }

			return false;
		 }
	}

	return true;
}

void Feign (edict_t *self)
{
	static int i;

	if (!check_stand(self))
		return;

	i = (i+1)%3;
 
	if (!(self->ripstate & STATE_FEINING))
	{
		self->client->anim_priority = ANIM_DEATH;

        switch (i)
		{
		case 0:
			self->s.frame = FRAME_death101-1;
			self->client->anim_end = FRAME_death106;
			break;
		case 1:
			self->s.frame = FRAME_death201-1;
			self->client->anim_end = FRAME_death206;
			break;
		case 2:
			self->s.frame = FRAME_death301-1;
			self->client->anim_end = FRAME_death308;
			break;
		}

		gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
        CTFDeadDropFlag(self);
		self->s.modelindex2 = 0;
		self->movetype = MOVETYPE_TOSS;
		self->client->ps.pmove.pm_type = PM_DEAD;
		self->ripstate &= ~STATE_FEINING;
		gi.linkentity (self);
	}
	else
	{
		self->client->anim_priority = ANIM_BASIC;
		self->client->ps.pmove.pm_type = PM_NORMAL;
    	self->client->ps.gunindex = gi.modelindex(self->client->pers.weapon->view_model);
		self->ripstate |= STATE_FEINING;
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

void Cmd_Cloak_f (edict_t *ent)
{
	if (ent->playerclass != 9)
		return;

	if (ent->ripstate & STATE_CLOAKING)
	{
    	ent->ripstate &= ~STATE_CLOAKING;
      	ent->svflags &= ~SVF_NOCLIENT;

		if (ent->client->chasetoggle)
			ent->client->oldplayer->svflags &= ~SVF_NOCLIENT;

		gi.cprintf (ent, PRINT_HIGH, "Cloaking field deactivated.\n");
		return;
	}

	if (ent->client->pers.inventory[ITEM_INDEX(FindItem ("Cells"))] < 25){
		gi.cprintf (ent, PRINT_HIGH, "Not enough energy.\n");
		return;
	}

	ent->client->pers.inventory[ITEM_INDEX(FindItem ("Cells"))] -= 25;
	ent->ripstate |= STATE_CLOAKING;
	ent->svflags |= SVF_NOCLIENT;

	if (ent->client->chasetoggle)
		ent->client->oldplayer->svflags |= SVF_NOCLIENT;

	gi.cprintf (ent, PRINT_HIGH, "Cloaking field activated.\n");
}

/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!G_ClientExists(ent))
		return;		// not fully in game yet

	if (level.intermissiontime)
		return;

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
	else if (Q_stricmp (cmd, "feign") == 0)
		Feign (ent);
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
	else if (Q_stricmp (cmd, "maphelp") == 0)
	{
		if (level.goal != NULL)
			if (strlen (level.goal))
                Print_Msg (ent, level.goal);
	}  
	else if (Q_stricmp (cmd, "flaginfo") == 0)
        Flag_StatusReport (ent);
	else if (Q_stricmp (cmd, "radio_power") == 0)
		X_Radio_Power_f (ent, gi.argv(1));
	else if (Q_stricmp(cmd, "radio_team") == 0)		//Radio to Team
		X_Radio_f(ent, "TEAM", gi.argv(1));
	else if (Q_stricmp (cmd, "radio_player") == 0)		//Radio to Team
     	Radio_Player(ent, ent_by_name (gi.argv(1)), gi.argv(2));
    else if (Q_stricmp(cmd, "custom") == 0)
       	Cmd_Custom_f (ent);
    else if (Q_stricmp(cmd, "main") == 0)
       	Cmd_Main_f (ent, gi.argv(1));
    else if (Q_stricmp(cmd, "menu_speed") == 0)
		Cmd_Speed_f (ent);
    else if (Q_stricmp(cmd, "menu_class") == 0)
	{
		if (ent->client->resp.s_team == 0)
			stuffcmd (ent, "menu_team\n");
		else
			Cmd_Class_f (ent);
	}
    else if (Q_stricmp(cmd, "menu_team") == 0)
		Cmd_Team_f(ent);
	else if (Q_stricmp(cmd, "menu_armor") == 0)
		Cmd_Armor_f (ent);
	else if (Q_stricmp(cmd, "menu_done") == 0)
		Cmd_Done_f (ent);
	else if (Q_stricmp(cmd, "id") == 0)
		Cmd_id_f (ent);
	else if (Q_stricmp(cmd, "build") == 0)
		Cmd_Build_f (ent);
	else if (Q_stricmp(cmd, "origin") == 0)
		Self_Origin (ent);
	else if (Q_stricmp(cmd, "cloak") == 0)
		Cmd_Cloak_f (ent);
	else if (Q_stricmp(cmd, "jumpf") == 0)
		MageJump2 (ent);
	else if (Q_stricmp(cmd, "jumpb") == 0)
		MageJump1 (ent);
	else if (Q_stricmp(cmd, "reset") == 0)
		stuffcmd (ent, "\n");
	else if (Q_stricmp(cmd, "radio_off") == 0)
		TeamMasterCanOffPlayersRadio (ent, gi.argv(1));
	else if (Q_stricmp(cmd, "kick_team") == 0)
        TeamMasterCanKickPlayer (ent, gi.argv(1));
	else if (Q_stricmp(cmd, "shut_up") == 0)
		TeamMasterCanShutUpPlayer (ent, gi.argv(1));
	else if (Q_stricmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f (ent);
    else if (Q_stricmp (cmd, "chasecam") == 0)
        Cmd_Chasecam_Toggle (ent);
	else if (Q_stricmp(cmd, "riplist") == 0)
	    gi.cprintf (ent, PRINT_HIGH, "feign\nmaphelp\ncam\nflaginfo\nradio_power\nradio_team\nradio_player\nmain\nid\nbuild\norigin\njumpf\njumpb\nreset\nradio_off\nkick_team\nshut_up\nplayerlist\nwarrior\nnecromancer\nthief\ninfantry\nminer\nscientist\nflamer\ntrooper\nchasecam\nriplist\n");
	else if (!Q_stricmp(cmd, "warrior") || !Q_stricmp(cmd, "necromancer") || !Q_stricmp(cmd, "thief") || !Q_stricmp(cmd, "infantry") || !Q_stricmp(cmd, "miner")
		|| !Q_stricmp(cmd, "scientist") || !Q_stricmp(cmd, "flamer") || !Q_stricmp(cmd, "trooper") || !Q_stricmp(cmd, "ghost"))
		if (!ent->client->pers.spectator)
	{
		int i;

		for (i = 1; cla_names[i]; i++)
		{
	         if (stricmp (cmd, cla_names[i]) == 0)
			 {
				 if (ent->lastclass != i)
				 {
					 if (!ent->deadflag)
						 gi.cprintf (ent, PRINT_HIGH, "After death you will respawn as a %s\n", cla_names[i]);
					 else
						 gi.cprintf (ent, PRINT_HIGH, "You will respawn as a %s\n", cla_names[i]);
				 }

				 ent->lastclass = i;
				 break;
			 }
		}
	}
	else
		Cmd_Say_f (ent, false, true);
}