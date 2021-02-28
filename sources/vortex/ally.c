#include "g_local.h"

#define ADD_ALLY				1
#define REMOVE_ALLY				2
#define ALLY_RANGE				0
#define ALLY_MAX_LEVEL_DELTA	5

#define	CENTERPRINT				1

int	team_numbers[100];
int	team_colors[] = 
{
		0xf2f2f0f0,		//0 red
		0xf3f3f1f1,		//1 blue
		0xdcdddedf,		//2 yellow
		0x70717273,		//3 light blue
		0xd0d1d2d3,     //4 green
		0xb0b1b2b3,		//5 purple
		0x40414243,		//6 light red
		0xe2e5e3e6,		//7 orange
		0xd0f1d3f3,		//8 blue green
		0xf2f3f0f1,		//9 red blue
};

int GetTeamColor (int teamnum)
{
	int index = teamnum - 100;

	if (index < 10)
		return team_colors[index];
	else
		return 0; // no colors left
}

qboolean ValidAllyMode (void)
{
	if (domination->value || ctf->value || invasion->value || hw->value || tbi->value)
		return false;
	return true;
}

void InitializeTeamNumbers (void)
{
	int i;

	if (allies->value < 1)
		return;
	if (!ValidAllyMode())
		return;

	for (i=0; i<100; i++) 
	{
		team_numbers[i] = i + 100; // value == index + 100
	}
}

int AssignTeamNumber (void)
{
	int i, j;

	for (i=0; i<100; i++)
	{
		if (!team_numbers[i])
			continue;
		j = team_numbers[i]; // temp storage, we will return this value
		team_numbers[i] = 0; // no longer available
		return j;
	}
	return 0;
}

qboolean ValidAlly (edict_t *ent)
{
	// FIXME: possibly add a level check here as well (used for validation of current allies)
	// can't ally in domination or pvm mode
	if (!ValidAllyMode())
		return false;
	// spectators can't ally
	if (ent->client->resp.spectator || ent->client->pers.spectator)
		return false;
	// mini-bosses can't ally
	if (IsNewbieBasher(ent))
		return false;
	// you can't ally if you are on a war
	if (SPREE_DUDE && (SPREE_DUDE == ent))
		return false;
	return true;
}

qboolean CanAlly (edict_t *ent, edict_t *other, int range)
{
	// you can't ally with yourself
	if (ent == other)
		return false;

	if (!ValidAlly(ent) || !ValidAlly(other))
	{
		//gi.dprintf("no4\n");
		return false;
	}
	// check for max allies
	// alianza con la mitad del server activo (vrxchile v1.3)
	// si uno de los dos tiene una alianza con mas de la mitad de los jugadores activos, no se pueden aliar.
	if (((numAllies(ent) + 1) > JoinedPlayers() / 2) ||
		((numAllies(other) + 1) > JoinedPlayers() / 2))
		return false;
	// are we already allied?
	if (IsAlly(ent, other))
		return false;
	
	// vrxchile v1.3 encourages alliances ASAP.
	// are we out of range?
	/*if (range && (entdist(ent, other) > range))
		return false;*/
	// can we see each other?
	/*if (range && !visible(ent, other))
		return false;*/

	// only allow allies that are close in level
	// FIXME: we should get an average or median player level if there are >1 allies
	if (abs(ent->myskills.level - other->myskills.level) > ALLY_MAX_LEVEL_DELTA)
	{
		//gi.dprintf("no3\n");
		return false;
	}
	// only allow allies when their level isn't too high, and server has enough players
	/*if ((ent->myskills.level > AveragePlayerLevel()*1.5) || (other->myskills.level > AveragePlayerLevel()*1.5) || (ActivePlayers() < 6))
	{
		//gi.dprintf("no1\n");
		return false;
	}*/
	//4.5 only allow allies if they have the same combat preferences
	//if (ent->myskills.respawns != other->myskills.respawns)
	if (!invasion->value && !pvm->value && !ffa->value) // pvp, always allow no matter the preferences.
		return true; 
	return true;
}

qboolean IsAlly (edict_t *ent, edict_t *other)
{
	if (!ent || !other)
		return false;

	if (!ent->teamnum || !other->teamnum)
		return false;

	if (ent->teamnum == other->teamnum)
		return true;
	else
		return false;
}

int numAllies (edict_t *ent)
{
	int		i, players=0;
	edict_t *cl_ent;

	// allies not enabled
	if (allies->value < 1)
		return 0;
	// no team assigned
	if (!ent->teamnum)
		return 0;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts+1+i;

		if (!cl_ent->inuse)
			continue;
		// don't count self toward total number of allies
		if (cl_ent == ent)
			continue;
		if (cl_ent->teamnum == ent->teamnum)
			players++;
	}

	return players;
}

void ResetAlliances (edict_t *ent)
{
	int		i;
	edict_t *cl_ent;

	// allies not enabled
	if (allies->value < 1)
		return;
	// no team assigned
	if (!ent->teamnum)
		return;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts+1+i;

		if (!cl_ent->inuse)
			continue;
		if (cl_ent == ent)
			continue;

		// remove allies
		if (cl_ent->teamnum == ent->teamnum)
			RemoveAlly(ent, cl_ent);
	}
}

edict_t *getPlayerByName (const char *name)
{
	int		i;
	edict_t *player, *found = NULL;

	for (i = 1; i <= maxclients->value; i++)
	{
		player = &g_edicts[i];

		if (!player->inuse)
			continue;
		if (strcmp(name, player->client->pers.netname) != 0)
			continue;

		found = player;
	}

	return found;
}

void UpdateAlliedTeamnum (int old_teamnum, int new_teamnum)
{
	int		i;
	edict_t *cl_ent;

	// if we didn't have a team, then there is nobody to update
	if (!old_teamnum)
		return;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts+1+i;

		if (!cl_ent->inuse)
			continue;
		if (cl_ent->teamnum == old_teamnum)
			cl_ent->teamnum = new_teamnum;
	}
}

void allymarker_think (edict_t *self)
{
	vec3_t start, right;

	// must have an owner
	if (!self->owner || !self->owner->inuse || !self->owner->teamnum)
	{
		G_FreeEdict(self);
		return;
	}

	// beam diameter goes to 0 if we're dead or cloaked (to hide it)
	if ((self->owner->health < 1) || (self->owner->client->cloaking 
		&& self->owner->svflags & SVF_NOCLIENT) || (self->owner->flags & FL_WORMHOLE))
		self->s.frame = 0;
	else 
		self->s.frame = 4;

	self->s.skinnum = GetTeamColor(self->owner->teamnum);

	self->s.angles[YAW]+=18;
	AngleCheck(&self->s.angles[YAW]);
	AngleVectors(self->s.angles, NULL, right, NULL);

	if (PM_PlayerHasMonster(self->owner))
	{
		VectorCopy(self->owner->owner->s.origin, start);
		start[2] = self->owner->owner->absmax[2]+16;
	}
	else
	{
		VectorCopy(self->owner->s.origin, start);
		start[2] = self->owner->absmax[2]+16;
	}
	
	VectorMA(start, 12, right, self->s.origin);
	VectorMA(start, -12, right, self->s.old_origin);
	gi.linkentity(self);

	self->nextthink = level.time + FRAMETIME;
}

void RemoveAllyMarker (edict_t *ent)
{
	edict_t *scan = NULL;

	while((scan = G_Find(scan, FOFS(classname), "allylaser")) != NULL)
	{
		// remove all markers owned by this entity
		if (scan && scan->inuse && scan->owner 
			&& scan->owner->client && (scan->owner == ent))
			G_FreeEdict(scan);
	}
}

void CreateAllyMarker (edict_t *ent)
{
	edict_t *laser;

	// no team colors to assign
	if (!GetTeamColor(ent->teamnum))
		return;

	laser = G_Spawn();
	laser->movetype	= MOVETYPE_NONE;
	laser->solid = SOLID_NOT;
	laser->s.renderfx = RF_BEAM|RF_TRANSLUCENT;
	laser->s.modelindex = 1; // must be non-zero
	laser->classname = "allylaser";
	laser->s.frame = 4; // beam diameter
    laser->owner = ent;
	
    laser->think = allymarker_think;
	gi.linkentity(laser);
	laser->nextthink = level.time + FRAMETIME;
}
	
void AddAlly (edict_t *ent, edict_t *other)
{
	// if the player doesn't have a team number, assign one
	if (!ent->teamnum)
	{
		ent->teamnum = AssignTeamNumber();
		CreateAllyMarker(ent);
	}

	// join invitee and his allies to player's team
	if (other->teamnum)
	{
		UpdateAlliedTeamnum(other->teamnum, ent->teamnum);
	}
	else
	{
		other->teamnum = ent->teamnum;
		CreateAllyMarker(other);
	}
}

void RemoveAlly (edict_t *ent, edict_t *other)
{
	// will removing a player invalidate the team? (no more allies)
	if (numAllies(ent) - 1 < 1)
	{
		// relinquish the team number for others to use
		team_numbers[ent->teamnum - 100] = ent->teamnum;

		// we may have removed ourself, so we need to update our ally as well
		UpdateAlliedTeamnum(ent->teamnum, 0);

		ent->teamnum = 0;
		return;
	}

	// this player is ousted from the team, so reset his team number to zero (no team)
	if (other)
		other->teamnum = 0;
}

void NotifyAllies (edict_t *ent, int msgtype, char *s)
{
	int		i;
	edict_t *cl_ent;

	// no team assigned!
	if (!ent->teamnum)
		return;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts+1+i;

		if (!cl_ent->inuse)
			continue;
		if (cl_ent->teamnum != ent->teamnum)
			continue;
		if (cl_ent == ent)
			continue;
		if (msgtype)
			gi.centerprintf(cl_ent, "%s", s);
		else
			safe_cprintf(cl_ent, PRINT_HIGH, "%s", s);
	}
}

int AddAllyExp (edict_t *ent, int exp)
{
	int		i, allies=0;
	edict_t	*cl_ent;

	// get number of players allied with us
	allies = numAllies(ent);
	if (!allies)
		return 0;

	// divide experience evenly among allies
	exp /= allies+1;

	// award points to allies
	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts+1+i;

		if (!cl_ent->inuse)
			continue;
		if (cl_ent->teamnum != ent->teamnum)
			continue;
		if (cl_ent->health < 0)
			continue;
		if (G_IsSpectator(cl_ent))
			continue;
		if (cl_ent->flags & FL_CHATPROTECT)
			continue;

		V_AddFinalExp(cl_ent, exp);
	}

	//V_AddFinalExp(ent, exp);
	return exp;
}

void AllyID (edict_t *ent)
{
	vec3_t	forward, right, offset, start, end;
	trace_t tr;
	edict_t *e=NULL;

	if (!allies->value)
		return;
	if (!numAllies(ent))
		return;

	// find entity near crosshair
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	if (tr.ent)
		e = tr.ent;

	if (e && e->inuse && (e->health > 0) && (level.time > ent->msg_time))
	{
		if (PM_MonsterHasPilot(e))
			e = e->owner;
		else if (!e->client)
			return;

		if (IsAlly(ent, e))
		{
			gi.centerprintf(ent, "Ally: %s\n", e->client->pers.netname);
			ent->msg_time = level.time + 5;
		}
	}
}

void AbortAllyWait (edict_t *ent)
{
	if (!ent)
		return;

	ent->client->allytarget = NULL;
	ent->client->ally_accept = false;
	ent->client->allying = false;
	ent->client->ally_time = 0;
}

void ShowAllyInviteMenu_handler (edict_t *ent, int option)
{
	edict_t *e = ent->client->allytarget;

	if (!e || !e->inuse)
		return;

	// make sure they can still ally
	if (!CanAlly(ent, e, ALLY_RANGE))
		return;

	if (option == 1)
	{
		// notify inviter and his previous allies, if any
		gi.centerprintf(e, "Now allied with %s\n", ent->client->pers.netname);
		NotifyAllies(e, CENTERPRINT, va("Now allied with %s\n", ent->client->pers.netname));

		// notify invitee's allies
		NotifyAllies(ent, CENTERPRINT, va("Now allied with %s\n", e->client->pers.netname));

		// join allies
		AddAlly(ent, e);

		// notify invitee
		safe_cprintf(ent, PRINT_HIGH, "Ally added.\n");

		// reset wait menu variables
		AbortAllyWait(ent);
		AbortAllyWait(e);

		// close the invitation menu
		closemenu(ent);

		if (InMenu(e, 0, ShowAllyWaitMenu_handler))
			closemenu(e);
	}
	else if (option == 2)
	{
		// reset wait menu variables
		AbortAllyWait(ent);
		AbortAllyWait(e);

		// close the invitation menu
		closemenu(ent);

		if (InMenu(e, 0, ShowAllyWaitMenu_handler))
			closemenu(e);

		safe_cprintf(e, PRINT_HIGH, "%s declined your offer to ally.\n", ent->client->pers.netname);
	}
}

void ShowAllyInviteMenu (edict_t *ent)
{
	edict_t *e = ent->client->allytarget;

	 if (!ShowMenu(ent))
        return;

	clearmenu(ent);
		//				xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, va("%s", e->client->pers.netname), MENU_GREEN_CENTERED);
	addlinetomenu(ent, "would like to ally.", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	//Menu footer
	addlinetomenu(ent, "Accept", 1);
	addlinetomenu(ent, "Decline", 2);

	//Set handler
	setmenuhandler(ent, ShowAllyInviteMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = 5;

	//Display the menu
	showmenu(ent);
}

void ShowAllyWaitMenu_handler (edict_t *ent, int option)
{
	edict_t *e = ent->client->allytarget;

	if (option == 666)	//ent closed the menu
	{
		closemenu(ent);
		return;
	}

	if (e)
	{
		AbortAllyWait(e);
		if (InMenu(e, 0, ShowAllyInviteMenu_handler))
			closemenu(e);
	}

	AbortAllyWait(ent);
	closemenu(ent);
}

void ShowAllyWaitMenu (edict_t *ent)
{
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);
		//				xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Please wait for player to ", MENU_GREEN_CENTERED);
	addlinetomenu(ent, "accept your invitation. ", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	//Menu footer
	addlinetomenu(ent, "Close Window", 666);
	addlinetomenu(ent, "Cancel Invitation", 1);

	//Set handler
	setmenuhandler(ent, ShowAllyWaitMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = 4;

	//Display the menu
	showmenu(ent);
}

void ShowAddAllyMenu_handler (edict_t *ent, int option)
{
	edict_t *e;

	if (option == 666)	//ent closed the menu
	{
		closemenu(ent);
		return;
	}

	// get ally target
	e = V_getClientByNumber(option-1);

	// make sure we're still allowed to ally
	if (!CanAlly(ent, e, ALLY_RANGE))
	{
		closemenu(ent);
		return;
	}

	// open wait menu
	ShowAllyWaitMenu(ent);

	// set up ally parameters
	ent->client->allytarget = e;
	ent->client->ally_accept = true;
	ent->client->allying = true;
	ent->client->ally_time = level.time;

	// set up ally target parameters
	e->client->allytarget = ent;
	e->client->ally_accept = false;
	e->client->allying = true;
	e->client->ally_time = level.time;

	// open invitation menu
	ShowAllyInviteMenu(e);
}

void ShowAddAllyMenu (edict_t *ent)
{
	int i;
	int j = 0;
	edict_t *temp;

	 if (!ShowMenu(ent))
		return;
	clearmenu(ent);

	addlinetomenu(ent, "Select a player:", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	for_each_player(temp, i)
	{
		if (CanAlly(ent, temp, ALLY_RANGE))
		{
			//Add player to the list
			addlinetomenu(ent, va(" %s (%s)", temp->myskills.player_name, 
				GetClassString(temp->myskills.class_num)), GetClientNumber(temp));
			++j;

			// 3.65 can only display up to 10 possible allies
			// any more than this will cause the game to crash
			// FIXME: split this up into multiple menus?
			if (j >= 10)
				break;
		}
		
	}

	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Exit", 666);

	//Set handler
	setmenuhandler(ent, ShowAddAllyMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = 4 + j;

	//Display the menu
	showmenu(ent);
}

void ShowRemoveAllyMenu_handler (edict_t *ent, int option)
{
	edict_t *e;

	if (option == 666)	//ent closed the menu
	{
		closemenu(ent);
		return;
	}

	e = V_getClientByNumber(option-1);
	RemoveAlly(ent, e);
	closemenu(ent);

	// notify player's allies
	NotifyAllies(ent, 0, va("%s has removed %s from your team\n", ent->client->pers.netname, e->client->pers.netname));
	NotifyAllies(ent, CENTERPRINT, va("Ally removed: %s\n", e->client->pers.netname));

	// notify removed player
	gi.centerprintf(e, "No longer allied with: %s\n", ent->client->pers.netname);

	// notify player
	safe_cprintf(ent, PRINT_HIGH, "Ally removed.\n");
}

void ShowRemoveAllyMenu (edict_t *ent)
{
	int i;
	int j = 0;
	edict_t *temp;

	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	addlinetomenu(ent, "Select a player:", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	for_each_player(temp, i)
	{
		if (IsAlly(ent, temp))
		{
			//Add player to the list
			addlinetomenu(ent, va(" %s", temp->myskills.player_name), GetClientNumber(temp));
			++j;
		}
		
	}

	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Exit", 666);

	//Set handler
	setmenuhandler(ent, ShowRemoveAllyMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = 4 + j;

	//Display the menu
	showmenu(ent);
}

void ShowAllyMenu_handler (edict_t *ent, int option)
{
	if (option == 1)
		ShowAddAllyMenu(ent);
	else if (option == 2)
		ShowRemoveAllyMenu(ent);
	else
		closemenu(ent);
}

void ShowAllyMenu (edict_t *ent)
{
	int i;
	int j = 0;
	edict_t *temp;

	//Don't bother displaying the menu if alliances are disabled
	if (!allies->value)
	{
		safe_cprintf(ent, PRINT_HIGH, "Alliances are disabled.\n");
		return;
	}

	// alliance only work in pvp mode
	if (!ValidAllyMode())
	{
		safe_cprintf(ent, PRINT_HIGH, "Alliances are disabled.\n");
		return;
	}

	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	addlinetomenu(ent, "Currently allied with:", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	for_each_player(temp, i)
	{
		if (IsAlly(ent, temp))
		{
			//Add player to the list
			addlinetomenu(ent, va(" %s", temp->myskills.player_name), 0);
			++j;
		}
	}

	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Add Ally", 1);
	addlinetomenu(ent, "Remove Ally", 2);
	addlinetomenu(ent, "Exit", 666);

	//Set handler
	setmenuhandler(ent, ShowAllyMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = 6 + j;

	//Display the menu
	showmenu(ent);
}

int AlliedTeamComp (const void *v1, const void *v2)
{
	edict_t *p1 = (edict_t *)v1, *p2 = (edict_t *)v2;
	return p1->teamnum-p2->teamnum;
}

void PrintAlliedTeams (edict_t *ent)
{
	int i, j;
	edict_t *player[MAX_CLIENTS], *e;

	// create an array of allied players
	for (i=0, j=0; i<game.maxclients ; i++)
	{
		e = g_edicts+1+i;

		if (!e->inuse || !e->teamnum)
			continue;
		player[j++] = e;
		//gi.dprintf("populated %d with %s\n", j-1, e->client->pers.netname);
	}

	qsort(player, j, sizeof(edict_t), AlliedTeamComp);

	for (i=0 ; i<j ; i++)
	{
		safe_cprintf(ent, PRINT_HIGH, "%d: %s\n", player[i]->teamnum, player[i]->client->pers.netname);
	}

}
