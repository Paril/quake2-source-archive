#include "g_local.h"

edict_t *dom_flagcarrier (void)
{
	int		i;
	edict_t *cl_ent;

	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (cl_ent->client->pers.inventory[flag_index])
			return cl_ent;
	}
	return NULL;
}

void dom_awardpoints (void)
{
	int		i, points, credits;
	edict_t	*cl_ent;

	// flag has not been captured
	if (!DEFENSE_TEAM)
		return;
	FLAG_FRAMES++; // record frames flag has been captured
//	if (!(FLAG_FRAMES % 10))
//		gi.dprintf("frames %d\n", FLAG_FRAMES);

	if (level.framenum % DOMINATION_AWARD_FRAMES)
		return;
	// not enough players
	if (ActivePlayers() < DOMINATION_MINIMUM_PLAYERS)
		return;

	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		//if (G_EntIsAlive(cl_ent) && (cl_ent->teamnum == DEFENSE_TEAM))
		if (cl_ent && cl_ent->inuse && cl_ent->client && (cl_ent->health>0) 
			&& !G_IsSpectator(cl_ent) && (cl_ent->teamnum==DEFENSE_TEAM))
		{
			points = DOMINATION_POINTS;
			credits = DOMINATION_CREDITS;
			// flag carrier gets extra points
			if (cl_ent->client->pers.inventory[flag_index])
			{
				points *= 3;
				credits *= 3;
			}
			/*
			cl_ent->myskills.experience += points;
			cl_ent->client->resp.score += points;
			check_for_levelup(cl_ent);
			*/
			cl_ent->myskills.credits += credits;
			V_AddFinalExp(cl_ent, points);
		}
	}
}

void dom_fragaward (edict_t *attacker, edict_t *target)
{
	int		points = DOMINATION_FRAG_POINTS;
	float	dist;
	edict_t *carrier, *targ;

	targ = target; // this could possibly be a non-client entity
	attacker = G_GetClient(attacker);
	target = G_GetClient(target);

	if (!DEFENSE_TEAM)
		return;
	if (FLAG_FRAMES < 100)
		return; // flag must be captured for at least 10 seconds
	//if (!G_EntExists(attacker) || !G_EntExists(target))
	//	return;
	// basic sanity checks
	if (!attacker || !attacker->inuse || G_IsSpectator(attacker) 
		|| !target || !target->inuse || G_IsSpectator(target))
		return;
	if (attacker == target)
		return;
	if (attacker->health < 1)
		return;
	
	if (((carrier = dom_flagcarrier()) != NULL) && (carrier != attacker))
	{
		dist = entdist(carrier, targ);
		if (OnSameTeam(attacker, carrier))
		{
			// if we are on the same team as the flag carrier and the
			// enemy was close to the carrier, then award a bonus for
			// protecting the flag
			if (dist <= DOMINATION_DEFEND_RANGE)
			{
				gi.bprintf(PRINT_HIGH, "%s defends the flag carrier!\n", 
					attacker->client->pers.netname);
				points = DOMINATION_DEFEND_BONUS;
				gi.sound(carrier, CHAN_ITEM, gi.soundindex("speech/excelent.wav"), 1, ATTN_NORM, 0);
			}
		}
		else if (carrier == targ)
		{
			// award a bonus for killing the flag carrier
			gi.bprintf(PRINT_HIGH, "%s kills the flag carrier!\n",
				attacker->client->pers.netname);
			points = DOMINATION_CARRIER_BONUS;
			gi.sound(attacker, CHAN_ITEM, gi.soundindex("ctf/flagcap.wav"), 1, ATTN_NORM, 0);
		}
		else if (dist <= DOMINATION_DEFEND_RANGE)
		{
			// award a bonus for killing flag defense
			gi.bprintf(PRINT_HIGH, "%s kills a defender!\n", 
				attacker->client->pers.netname);
			points = DOMINATION_OFFENSE_BONUS;
			gi.sound(attacker, CHAN_ITEM, gi.soundindex("speech/idoasskk.wav"), 1, ATTN_NORM, 0);
		}
	}
	else if (attacker->teamnum == DEFENSE_TEAM)
		return; // no further bonuses available for defense team

	/*
	attacker->myskills.experience += points;
	attacker->client->resp.score += points;
	check_for_levelup(attacker);
	*/
	V_AddFinalExp(attacker, points);
}

qboolean dom_pickupflag (edict_t *ent, edict_t *other)
{
	int		i;
	edict_t *cl_ent;

	//if (!G_EntExists(other))
	//	return false;
	if (!other || !other->inuse || !other->client || G_IsSpectator(other))
		return false;

	//if ((other->myskills.class_num == CLASS_POLTERGEIST) || other->mtype)
	//	return false; // poltergeist and morphed players can't pick up flag

	// unmorph morphed players
	if (other->mtype)
	{
		other->mtype = 0;
		other->s.modelindex = 255;
		other->s.skinnum = ent-g_edicts-1;
		ShowGun(other);
	}
	
	// if this is a player-monster, remove the monster and restore the player
	if (PM_PlayerHasMonster(other))
		PM_RemoveMonster(other);

	// disable movement abilities
	if (other->client)
	{
		//jetpack
		other->client->thrusting = 0;
		//grapple hook
		other->client->hook_state = HOOK_READY;
	}
	// super speed
	other->superspeed = false;
	// antigrav
	other->antigrav = false;

	VortexRemovePlayerSummonables(other);

	// disable scanner
	if (other->client->pers.scanner_active & 1)
		other->client->pers.scanner_active = 0;

	// reset their velocity
	VectorClear(other->velocity);

	// alert everyone
	gi.bprintf(PRINT_HIGH, "%s got the flag!\n", other->client->pers.netname);
	gi.bprintf(PRINT_HIGH, "The %s team is now in control.\n", TeamName(other));

	// alert teammates
	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (G_EntExists(cl_ent) && (cl_ent->teamnum == other->teamnum) && (cl_ent != other))
			gi.centerprintf(cl_ent, "Protect the flag carrier!\n");
	}

	DEFENSE_TEAM = other->teamnum;

	// if a new team takes control of the flag, then reset the counter
	if (PREV_DEFENSE_TEAM != DEFENSE_TEAM)
		FLAG_FRAMES = 0;

	PREV_DEFENSE_TEAM = DEFENSE_TEAM;

	gi.sound(other, CHAN_ITEM, gi.soundindex("world/xianbeats.wav"), 1, ATTN_NORM, 0);
	other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
	return true;
}

void dom_laserthink (edict_t *self)
{
	// must have an owner
	if (!self->owner || !self->owner->inuse)
	{
		G_FreeEdict(self);
		return;
	}
	
	if (self->owner->style)
		self->s.skinnum = 0xf2f2f0f0;
	else
		self->s.skinnum = 0xf3f3f1f1;

	// update position
	VectorCopy(self->pos2, self->s.origin);
	VectorCopy(self->pos1, self->s.old_origin);

	self->nextthink = level.time + FRAMETIME;
}

edict_t *dom_spawnlaser (edict_t *ent, vec3_t v1, vec3_t v2)
{
	edict_t *laser;

	laser = G_Spawn();
	laser->movetype	= MOVETYPE_NONE;
	laser->solid = SOLID_NOT;
	laser->s.renderfx = RF_BEAM|RF_TRANSLUCENT;
	laser->s.modelindex = 1; // must be non-zero
	//laser->s.sound = gi.soundindex ("world/laser.wav");
	laser->classname = "laser";
	laser->s.frame = 8; // beam diameter
    laser->owner = ent;
	// set laser color
	laser->s.skinnum = 0xf2f2f0f0; // red
    laser->think = dom_laserthink;
	VectorCopy(v2, laser->s.origin);
	VectorCopy(v1, laser->s.old_origin);
	VectorCopy(v2, laser->pos2);
	VectorCopy(v1, laser->pos1);
	gi.linkentity(laser);
	laser->nextthink = level.time + FRAMETIME;
	return laser;
}

void dom_flagthink (edict_t *self)
{
	vec3_t	end;
	trace_t	tr;

	//3.0 allow anyone to pick up the flag
	if (self->owner) self->owner = NULL;

	//3.0 destroy idling flags so they can respawn somewhere else
	if (self->count >= 60)	//30 seconds of idling time;
	{
		BecomeTE(self);
		return;
	}
	self->count++;

	if (!self->other && !VectorLength(self->velocity))
	{
		VectorCopy(self->s.origin, end);
		end[2] += 8192;
		tr = gi.trace (self->s.origin, NULL, NULL, end, self, MASK_SOLID);
		VectorCopy(tr.endpos, end);
		self->other = dom_spawnlaser(self, self->s.origin, end);
	}
	self->s.effects = 0;
	if (self->style)
		self->style = 0;
	else
		self->style = 1;
	self->s.effects |= (EF_ROTATE|EF_COLOR_SHELL);
	if (self->style)
		self->s.renderfx = RF_SHELL_RED;
	else
		self->s.renderfx = RF_SHELL_BLUE;
	self->nextthink = level.time + 0.5;
}

void dom_dropflag (edict_t *ent, gitem_t *item)
{
	edict_t *flag;

	//if (!G_EntExists(ent))
	//	return;
	if (!ent || !ent->inuse || G_IsSpectator(ent))
		return;
	if (!domination->value || !ent->client->pers.inventory[flag_index])
		return;
	gi.bprintf(PRINT_HIGH, "%s dropped the flag.\n", ent->client->pers.netname);
	flag = Drop_Item (ent, item);
	flag->think = dom_flagthink;
	flag->count = 0;
	//Wait a second before starting to think
	flag->nextthink = level.time + 1.0;
	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
	ValidateSelectedItem (ent);
	DEFENSE_TEAM = 0;
	//FLAG_FRAMES = 0;

	if (SpawnWaitingPlayers())
		OrganizeTeams(true);
}

void dom_spawnflag (void)
{
	edict_t *flag;

	flag = Spawn_Item(FindItemByClassname("item_flag"));
	flag->think = dom_flagthink;
	flag->nextthink = level.time + FRAMETIME;
	if (FindValidSpawnPoint(flag, false))
		gi.dprintf("INFO: Flag spawned successfully.\n");
	else
		gi.dprintf("WARNING: Flag failed to spawn!\n");
	DEFENSE_TEAM = 0;
	FLAG_FRAMES = 0;
}

void dom_checkforflag (edict_t *ent)
{
	if (!domination->value)
		return;
	if (DEFENSE_TEAM)
		return; // someone has the flag
	if (!ent || !ent->item)
		return;
	if (strcmp(ent->classname, "item_flag"))
		return;

	gi.bprintf(PRINT_HIGH, "Flag was re-spawned. Go find it!\n");
	// flag was destroyed, so re-spawn it
	dom_spawnflag();
}

void dom_resetflag (void)
{
	int		i;
	edict_t *cl_ent;

	// make sure nobody else has a flag!
	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (cl_ent && cl_ent->inuse)
			cl_ent->client->pers.inventory[flag_index] = 0;
	}
}

void dom_init (void)
{
	int		i;
	char	*s;
	edict_t *cl_ent;

	dom_resetflag();
	OrganizeTeams(true);
	gi.bprintf(PRINT_HIGH, "Find the flag!\n");
	dom_spawnflag();
	
	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (G_EntExists(cl_ent))
		{
			s = Info_ValueForKey (cl_ent->client->pers.userinfo, "skin");
			V_AssignClassSkin(cl_ent, s);
		}
	}
	
}

void jointeam_handler (edict_t *ent, int option)
{
	int	num=0;
	joined_t	*slot=NULL;

	// exit menu
	if (option == 3)
		return;

	// let them join if we are still in pre-game or were here before
	if (level.time < pregame_time->value || ((slot = GetJoinedSlot(ent)) != NULL))
	{
		StartGame(ent);

		// set their team value and then clear the slot
		if (slot)
		{
			ent->teamnum = slot->team;
			ClearJoinedSlot(slot);
		}

		return;
	}

	// always allow administrators into the game
	if (ent->myskills.administrator)
	{
		StartGame(ent);
		OrganizeTeams(true);
		return;
	}
}

void OpenDOMJoinMenu (edict_t *ent)
{
	if (debuginfo->value)
		gi.dprintf("DEBUG: OpenDOMJoinMenu()\n");

	if (!ShowMenu(ent))
        return;

	// new character
	if (!ent->myskills.class_num)
	{
		OpenClassMenu(ent, 1);
		return;
	}

	clearmenu(ent);

	if (!ent->myskills.administrator && !InJoinedQueue(ent) && (level.time > pregame_time->value))
	{
//							xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
		addlinetomenu(ent, "Vortex Domination:", MENU_GREEN_CENTERED);
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, "The match has already", 0);
		addlinetomenu(ent, "begun. You will be joined", 0);
		addlinetomenu(ent, "automatically when the", 0);
		addlinetomenu(ent, "flag is dropped.", 0);
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, "Exit", 3);

		setmenuhandler(ent, jointeam_handler);
		ent->client->menustorage.currentline = 8;
		showmenu(ent);

		// indicate that the player wants to join
		ent->client->waiting_to_join = true;
		ent->client->waiting_time = level.time;
		return;
	}

	addlinetomenu(ent, "Vortex Domination:", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	//					xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Your goal is to help your", 0);
	addlinetomenu(ent, "team capture the flag.", 0);
	addlinetomenu(ent, "While your team is in", 0);
	addlinetomenu(ent, "control of the flag, you", 0);
	addlinetomenu(ent, "gain points. Protect your", 0);
	addlinetomenu(ent, "flag carrier! Don't let", 0);
	addlinetomenu(ent, "the flag into enemy hands.", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Join", 1);
	addlinetomenu(ent, "Back", 2);
	addlinetomenu(ent, "Exit", 3);

	setmenuhandler(ent, jointeam_handler);
	ent->client->menustorage.currentline = 11;
	showmenu(ent);
}
