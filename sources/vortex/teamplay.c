#include "g_local.h"
#include "m_insane.h"

#define RETARD_POINTS			25
#define RETARD_CREDITS			10
#define RETARD_POINTS_DELAY		10
#define RETARD_HEALTH			5000
#define RETARD_TARGET_RANGE		256
#define RETARD_RETARGET_TIME	3
#define RETARD_MSG_DELAY		3
#define RETARD_MIN_PLAYERS		4
#define RETARD_PLAYER_TIMEOUT	30
#define RETARD_FAIRTEAMS_VALUE	1.2

void AssignTeamSkin (edict_t *ent, char *s)
{
	int		playernum = ent-g_edicts-1;
	char	*p;
	char	t[64];

//	gi.dprintf("AssignTeamSkin()\n");

	Com_sprintf(t, sizeof(t), "%s", s);

	if ((p = strrchr(t, '/')) != NULL)
		p[1] = 0;
	else
		strcpy(t, "male/");

	if (ent->teamnum == 1)
	{
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", 
			ent->client->pers.netname, team1_skin->string) );
	}
	else if (ent->teamnum == 2)
	{
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", 
			ent->client->pers.netname, team2_skin->string) );
	}
	else
	{
		if (G_EntExists(ent) && (level.time > pregame_time->value))
			gi.dprintf("ERROR: AssignTeamSkin() can't determine team %d!\n", ent->teamnum);
		gi.configstring (CS_PLAYERSKINS+playernum, 
			va("%s\\%s", ent->client->pers.netname, s) );
	}
}

char *TeamName (edict_t *ent)
{
	if (ent->teamnum == 1)
		return "red";
	else if (ent->teamnum == 2)
		return "blue";
	else if (!ent->teamnum)
		return "no team";
	else
		return "unknown";
}

static edict_t *LowPlayer (void)
{
	int		i, j=0;
	edict_t	*cl_ent, *found=NULL;

	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (!cl_ent->inuse)
			continue;
		if (G_IsSpectator(cl_ent))
			continue;
		if (cl_ent->teamnum)
			continue; // already assigned a team
		if (!j || (cl_ent->myskills.level < j))
			found = cl_ent;
	}
	return found;
}

static edict_t *HiPlayer (void)
{
	int		i, j=0;
	edict_t	*cl_ent, *found=NULL;

	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (!cl_ent->inuse)
			continue;
		if (G_IsSpectator(cl_ent))
			continue;
		if (cl_ent->teamnum)
			continue; // already assigned a team
		if (!j || (cl_ent->myskills.level > j))
			found = cl_ent;
	}
	return found;
}

int TeamValue (int teamnum)
{
	int		i, value=0;
	edict_t *cl_ent;

	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (!cl_ent->inuse)
			continue;
		if (G_IsSpectator(cl_ent))
			continue;
		if (cl_ent->teamnum != teamnum)
			continue;
		value += cl_ent->myskills.level+1;
	//	gi.dprintf("value %d: %d\n", teamnum, value);
	}
	return value;
}

qboolean FairTeams (void)
{
	float	value;

	value = (float)TeamValue(1) / TeamValue(2);
	if ((value > RETARD_FAIRTEAMS_VALUE) || (value < 1/RETARD_FAIRTEAMS_VALUE))
		return false;
	return true;
}

void ResetTeams (void)
{
	int		i;
	edict_t	*cl_ent;

	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (cl_ent && cl_ent->inuse)
		{
			// drop the flag
			if (domination->value)
				dom_dropflag(cl_ent, FindItem("Flag"));
			// reset team value
			cl_ent->teamnum = 0;
		}
	}
}

int GetLevelValue (edict_t *player)
{
	int final = player->myskills.level + player->myskills.inuse;

	if (final < 1)
		final = 1;

	return final;
}

// DOOMIE START
void sortTeamsByLevel()  
{  
     edict_t	*player;  
     int		activeplayers = 0;//, total = 0;  
     edict_t	*playerlist[MAX_CLIENTS];  
     int		i, index, j;
	 char *s;
  
	 gi.bprintf(PRINT_HIGH, "Teams have been re-organized.\n");

     
     for (i=0 ; i<game.maxclients ; i++)  
     {
          player = g_edicts+1+i;  

          if (player && player->inuse)  
          {
			  // reset their team value
			  player->teamnum = 0;
			  
			  // ignore spectators
			  if (player->client->resp.spectator)  
                    continue;

			  // add player to the list
			  playerlist[activeplayers++] = player;

			  // calculate total player levels
			  //total += player->myskills.level;
          } 
     }
  
     if(activeplayers > 0)  
     {  
          int		team1score=0;  
          int		team2score=0; 
		  int		team1mod=0, team2mod=0;//, delta;
          edict_t	*max;  
          edict_t	*temp;  

		  // check for unfair team captures
		  /*
		  if (ctf->value)
		  {
			delta = abs(red_flag_caps-blue_flag_caps);
			if (delta > 1) // need at least 2 extra caps
			{
				// don't award more than 20% (70/30 split)
				if (delta > 4)
					delta = 4;

				// red team has the advantage, award blue with more levels
				if (red_flag_caps > blue_flag_caps)
					team2mod = 0.05*total*delta; // 5% per extra capture
				// blue team has the advantage, award red with more levels
				else
					team1mod = 0.05*total*delta;
			}
		  }
		  */

          // sort players by level  
          for (i = 0; i < activeplayers; i++)  
          {  
               max = playerlist[i];  
               index = i;  
               for(j = i+1; j < activeplayers; j++)  
                    if(GetLevelValue(playerlist[j])/*playerlist[j]->myskills.level*/ > GetLevelValue(max)/*max->myskills.level*/)  
                    {  
                         max = playerlist[j];  
                         index = j;  
                    }  
               // swap maximum with current position in array (noted as 'i')  
               temp = playerlist[i];  
               playerlist[i] = max;  
               playerlist[index] = temp;  
          }
  
          // organize teams by placing them on alternate sides, in order of level  
          for (i = 0; i < activeplayers; i++)  
          {  
               if (team1score/*-team1mod*/ < team2score/*-team2mod*/)  
               {  
                    // put on team 1  
                    playerlist[i]->teamnum = 1;  
                    team1score += GetLevelValue(playerlist[i]);//playerlist[i]->myskills.level + 1;
               }  
               else  
               {  
                    // put on team 2  
                    playerlist[i]->teamnum = 2;  
                    team2score += GetLevelValue(playerlist[i]);//playerlist[i]->myskills.level + 1;
               }
			   
			   gi.centerprintf(playerlist[i], "You are on the %s team\n", TeamName(playerlist[i]));

			   // set team skin
			   s = Info_ValueForKey (playerlist[i]->client->pers.userinfo, "skin");
			   V_AssignClassSkin(playerlist[i], s);
          }	
     }
}
// DOOMIE END

void OrganizeTeams (qboolean remove_summonables)
{
	sortTeamsByLevel();
	if (remove_summonables)
		G_ResetPlayerState(NULL);
}

int InJoinedQueue (edict_t *ent)
{
	int	i;
	for (i=0; i<MAX_CLIENTS; i++) {
		if (!strcmp(players[i].name, ent->client->pers.netname))
			return ent->teamnum;
	}
	return 0;
}

joined_t *GetJoinedSlot (edict_t *ent)
{
	int i;

	for (i=0; i<MAX_CLIENTS; i++) {
		if (!strcmp(players[i].name, ent->client->pers.netname))
			return &players[i];
	}
	return NULL;
}

joined_t *EmptyJoinedSlot (void)
{
	int	i;

//	gi.dprintf("EmptyJoinedSlot()\n");

	for (i=0; i<MAX_CLIENTS; i++) {
		if (players[i].team)
			continue;
		return &players[i];
	}
	return NULL;
}

void CopyToJoinedQueue (edict_t *ent, joined_t *slot)
{
//	gi.dprintf("CopyToJoinedQueue()\n");

	strcpy(slot->name, ent->client->pers.netname);
	slot->team = ent->teamnum;
	slot->time = level.time + RETARD_PLAYER_TIMEOUT;
}

void ClearJoinedSlot (joined_t *slot)
{
//	gi.dprintf("ClearJoinedSlot()\n");

	strcpy(slot->name, "");
	slot->team = 0;
	slot->time = 0;
}

void InitJoinedQueue (void)
{
	int	i;

//	gi.dprintf("InitJoinedQueue()\n");

	for (i=0; i<MAX_CLIENTS; i++) {
		ClearJoinedSlot(&players[i]);
	}
}

void AddJoinedQueue (edict_t *ent)
{
	joined_t *slot;

//	gi.dprintf("AddJoinedQueue()\n");

	if (InJoinedQueue(ent))
		return;
	if ((slot = EmptyJoinedSlot()) != NULL)
		CopyToJoinedQueue(ent, slot);
}

float HighestJoinedTime (void)
{
	int		i;
	float	time=0;

	for (i=0; i<MAX_CLIENTS; i++) {
		if (!time || (players[i].time > time))
			time = players[i].time;
	}
	return time;
}

int NumInJoinedQue (void)
{
	int i, num=0;

	for (i=0; i<MAX_CLIENTS; i++) {
		if (!players[i].team)
			continue;
		num++;
	}
//	gi.dprintf("%d in que\n", num);
	return num;
}
	
void PTRCheckJoinedQue (void)
{
	if (!ptr->value && !domination->value && !ctf->value)
		return;
	if (level.time < pregame_time->value)
		return;

	// check every second
	if (!(level.framenum%10))
	{
		if (JoinedPlayers() < 1)
			return;
		if (NumInJoinedQue() < 1)
			return;

		//gi.dprintf("There are %d players in the join que. Teams will re-balance in %.1f seconds\n",
		//	NumInJoinedQue(), HighestJoinedTime()-level.time);

		// if the last player in the que hasnt rejoined yet
		if (!FairTeams() && (level.time > HighestJoinedTime()))
		{
			gi.bprintf(PRINT_HIGH, "Teams are being re-organized because dropped players did not re-join!\n");
			OrganizeTeams(true);
			InitJoinedQueue();

			if (ctf->value)
				CTF_Init();
		}
	}
}

// spawns all players that are waiting to join the game
qboolean SpawnWaitingPlayers (void)
{
	int			i;
	edict_t		*player;
	qboolean	foundPlayers=false;

	//gi.dprintf("SpawnWaitingPlayers()\n");

	for (i=0 ; i<game.maxclients ; i++) 
	{
		player = g_edicts+1+i;

		if (!player || !player->inuse || !G_IsSpectator(player))
			continue;

#ifndef NO_GDS
#ifndef GDS_NOMULTITHREADING
		if (player->ThreadStatus != 1) // Not loaded?
			continue; // Can't play
#endif
#endif

		if (!player->client->waiting_to_join)
			continue;

		StartGame(player);
		foundPlayers = true;
	}

	return foundPlayers;
}

void teamselect_handler (edict_t *ent, int option)
{
	int	num=0;
	joined_t *slot;

	if (option == 3)
		return;
	if (option == 2)
	{
		OpenJoinMenu(ent);
		return;
	}

	// new character
	if (!ent->myskills.class_num)
	{
		OpenClassMenu(ent, 1);
		return;
	}

	// let them join if they were already here before
	// and were accidentally disconnected, or if we're
	// still in pre-game
	slot = GetJoinedSlot(ent);
	if (slot || (level.time < pregame_time->value))
	{
		if (slot)
		{
			ent->teamnum = slot->team;
			// remove player from the queue
			ClearJoinedSlot(slot);
		}
		StartGame(ent);
	}
	
}

void OpenPTRJoinMenu (edict_t *ent)
{
	int			time_left;


	if (debuginfo->value)
		gi.dprintf("DEBUG: OpenPTRJoinMenu()\n");

	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	if (!InJoinedQueue(ent) && (level.time > pregame_time->value))
	{
		if (timelimit->value)
			time_left = timelimit->value - level.time/60;
		else
			time_left = 999;


		addlinetomenu(ent, "Vortex PTR:", MENU_GREEN_CENTERED);
		addlinetomenu(ent, "Protect The Retard!", MENU_GREEN_CENTERED);
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, "The match has already", 0);
		addlinetomenu(ent, va("begun. %d minutes until", time_left), 0);
		addlinetomenu(ent, "next round.", 0);
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, "Back", 2);
		addlinetomenu(ent, "Exit", 3);

		setmenuhandler(ent, teamselect_handler);
		ent->client->menustorage.currentline = 8;
		showmenu(ent);
		return;
	}

	addlinetomenu(ent, "Vortex PTR:", MENU_GREEN_CENTERED);
	addlinetomenu(ent, "Protect The Retard!", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Your goal is to gain", 0);
	addlinetomenu(ent, "control of the retard. As", 0);
	addlinetomenu(ent, "long as he stays alive,", 0);
	addlinetomenu(ent, "you gain points. If he", 0);
	addlinetomenu(ent, "is killed, the other team", 0);
	addlinetomenu(ent, "gains control.", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Join", 1);
	addlinetomenu(ent, "Back", 2);
	addlinetomenu(ent, "Exit", 3);

	setmenuhandler(ent, teamselect_handler);
	ent->client->menustorage.currentline = 11;
	showmenu(ent);
}

void retard_shake (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("insane/insane5.wav"), 1, ATTN_IDLE, 0);
}

void retard_fist (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("insane/insane11.wav"), 1, ATTN_IDLE, 0);
}

void retard_moan (edict_t *self)
{
	gi.sound (self, CHAN_VOICE, gi.soundindex ("insane/insane7.wav"), 1, ATTN_IDLE, 0);
}

void retard_stand (edict_t *self);
void retard_refist (edict_t *self);

mframe_t retard_frames_punchground [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	retard_fist,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	retard_fist
};
mmove_t retard_move_punchground = {FRAME_stand25, FRAME_stand34, retard_frames_punchground, retard_refist};

mframe_t retard_frames_uptodown [] =
{
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	retard_moan,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,

	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,

	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	retard_fist,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,

	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	retard_fist,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL,
	ai_move,	0,	NULL
};
mmove_t retard_move_uptodown = {FRAME_stand1, FRAME_stand40, retard_frames_uptodown, retard_refist};

mframe_t retard_frames_downtoup [] =
{
	ai_move,	0,	NULL,		
	ai_move,	0,	NULL,		
	ai_move,	0,	NULL,	
	ai_move,	0,	NULL,		
	ai_move,	0,	NULL,		
	ai_move,	0,	NULL,		
	ai_move,	0,	NULL,		
	ai_move,	0,	NULL,		
	ai_move,	0,	NULL,				
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,				
	ai_move,	0,	NULL,				
	ai_move,	0,	NULL,				
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL,		
	ai_move,	0,	NULL,		
	ai_move,	0,	NULL,				
	ai_move,	0,	NULL,			
	ai_move,	0,	NULL
};
mmove_t retard_move_downtoup = {FRAME_stand41, FRAME_stand59, retard_frames_downtoup, retard_stand};

mframe_t retard_frames_stand_insane [] =
{
	ai_charge,	0,	retard_shake,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL,
	ai_charge,	0,	NULL
};
mmove_t retard_move_stand_insane = {FRAME_stand65, FRAME_stand94, retard_frames_stand_insane, retard_stand};

void retard_refist (edict_t *self)
{
	if (random() <= 0.5)
		self->monsterinfo.currentmove = &retard_move_punchground;
	else
		self->monsterinfo.currentmove = &retard_move_downtoup;
}

void retard_stand (edict_t *self)
{
	if (random() > 0.5)
		self->monsterinfo.currentmove = &retard_move_uptodown;
	else
		self->monsterinfo.currentmove = &retard_move_stand_insane;
}

void retard_findtarget (edict_t *self)
{
	edict_t *e=NULL;

	while ((e = findclosestradius(e, self->s.origin, 
		RETARD_TARGET_RANGE)) != NULL)
	{
		if (!G_EntIsAlive(e))
			continue;
		if (!OnSameTeam(self, e))
			continue;
		self->enemy = e;
		return;
	}
	self->enemy = NULL;
}

void retard_awardteampoints (edict_t *self)
{
	int		i;
	edict_t *cl_ent;

	if (!self->teamnum)
		return;

	if (ActivePlayers() < RETARD_MIN_PLAYERS)
	{
		gi.bprintf(PRINT_HIGH, "Not enough players!\n");
		// switch back to pvp default
		ptr->value = 0;
		pvm->value = 0;
		dm_monsters->value = 0;
		fraglimit->value = 100;
		timelimit->value = 0;
		EndDMLevel();
		return;
	}

	gi.dprintf("INFO: Team 1: %d levels Team 2: %d levels.\n", TeamValue(1), TeamValue(2));

	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (!G_EntIsAlive(cl_ent))
			continue;
		if (!OnSameTeam(self, cl_ent))
			continue;
		/*
		cl_ent->client->resp.score += RETARD_POINTS;
		cl_ent->myskills.experience += RETARD_POINTS;
		check_for_levelup(cl_ent);
		*/
		V_AddFinalExp(cl_ent, RETARD_POINTS);
		VortexAddCredits(cl_ent, 0, RETARD_CREDITS, false);
	}
}

void retard_seteffects (edict_t *self)
{
	self->s.effects &= ~(EF_COLOR_SHELL|EF_POWERSCREEN|EF_PLASMA);
	self->s.renderfx &= ~(RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE|RF_SHELL_YELLOW);

	if (!self->teamnum)
	{
		if (level.framenum & 6)
		{
			self->s.effects |= EF_COLOR_SHELL;
			self->s.renderfx |= RF_SHELL_RED;
		}
		else
		{
			self->s.effects |= EF_COLOR_SHELL;
			self->s.renderfx |= RF_SHELL_BLUE;
		}
	}
	else
	{
		if (self->teamnum == 1)
		{
			self->s.effects |= EF_COLOR_SHELL;
			self->s.renderfx |= RF_SHELL_RED;
		}
		else if (self->teamnum == 2)
		{
			self->s.effects |= EF_COLOR_SHELL;
			self->s.renderfx |= RF_SHELL_BLUE;
		}
	}
}

void SpawnRetard (int teamnum);

void retard_checkposition (edict_t *self)
{
	if (gi.pointcontents(self->s.origin) & MASK_SOLID)
	{
		gi.dprintf("WARNING: Retard stuck in solid. Attempting to respawn...");
		if (FindValidSpawnPoint(self, false))
		{
			gi.dprintf("success!\n");
		}
		else
		{
			// hopefully this doesn't happen too often
			// or the server could be locked up
			// most likely to happen on small maps
			gi.dprintf("failed.\n");
			gi.dprintf("WARNING: Possible infinite loop in-progress!\n");
			gi.bprintf(PRINT_HIGH, "Retard has respawned! Go find him!\n");
			SpawnRetard(0);
		}
	}
}

void retard_think (edict_t *self)
{
	vec3_t	v;

//	if (!(level.framenum%10))
//		gi.dprintf("defense_team %d retard %d\n", DEFENSE_TEAM, self->teamnum);

	retard_checkposition(self);
	if (level.time > self->wait)
	{
		retard_awardteampoints(self);
		self->wait = level.time + RETARD_POINTS_DELAY;
	}

	if (level.time > self->delay)
	{
		retard_findtarget(self);
		self->delay = level.time + RETARD_RETARGET_TIME;
		if (self->enemy)
		{
			VectorSubtract(self->enemy->s.origin, self->s.origin, v);
			self->ideal_yaw = vectoyaw(v);
			M_ChangeYaw(self);
		}
	}

	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	// don't slide
	if (self->groundentity)
	{
		self->velocity[0] *= 0.6;
		self->velocity[1] *= 0.6;
	}
	
	retard_seteffects(self);
	M_MoveFrame (self);
	M_CatagorizePosition (self);
	M_WorldEffects (self);
	self->nextthink = level.time + FRAMETIME;
}

void retard_teleport (edict_t *self)
{
	edict_t *e;

	if (!FairTeams())
		OrganizeTeams(true);
	for (e=g_edicts ; e < &g_edicts[globals.num_edicts]; e++)
	{
		if (e == self)
			continue;
		if (!G_EntIsAlive(e))
			continue;
		if (!OnSameTeam(self, e))
		{
			e->s.event = EV_PLAYER_TELEPORT;
			FindValidSpawnPoint(e, false);
			continue;
		}
		if (e->movetype == MOVETYPE_NONE)
			continue;
		TeleportNearArea(e, self->s.origin, 512, false);
	}
}

void retard_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	forward, right, start, offset;

	if (!self->teamnum && other && other->client && other->teamnum)
	{
		self->teamnum = other->teamnum;
		gi.bprintf(PRINT_HIGH, "%s found the retard! %s team is in control!\n",
			other->client->pers.netname, TeamName(other));
		DEFENSE_TEAM = other->teamnum; // used for hud
		gi.sound(self, CHAN_ITEM, gi.soundindex("world/xianbeats.wav"), 1, ATTN_NORM, 0);
		self->takedamage = DAMAGE_YES;
		retard_teleport(self);
	}

	if (other && other->client && OnSameTeam(self, other))
	{
		AngleVectors (other->client->v_angle, forward, right, NULL);
		VectorScale (forward, -3, other->client->kick_origin);
		VectorSet(offset, 0, 7,  other->viewheight-8);
		P_ProjectSource (other->client, other->s.origin, offset, forward, right, start);

		self->velocity[0] += forward[0] * 50;
		self->velocity[1] += forward[1] * 50;
		self->velocity[2] += forward[2] * 50;
	}
}

void retard_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	attacker = G_GetClient(attacker);
	if (attacker)
	{
		gi.bprintf(PRINT_HIGH, "%s killed the retard! %s team is now in control!\n",
				attacker->client->pers.netname, TeamName(attacker));
		SpawnRetard(attacker->teamnum);
	}
	else
	{
		gi.bprintf(PRINT_HIGH, "The retard was killed! Go find the new one!\n");
		SpawnRetard(0);
	}
	BecomeTE(self);
}

void retard_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	int		i;
	edict_t	*cl_ent;

	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (!G_EntIsAlive(cl_ent))
			continue;
		if (!OnSameTeam(self, cl_ent))
			continue;
		if (level.time > cl_ent->msg_time)
		{
			gi.centerprintf(cl_ent, "Retard under attack!\n%d health left.", self->health);
			cl_ent->msg_time = level.time + RETARD_MSG_DELAY;
		}
	}
}

void SpawnRetard (int teamnum)
{
	edict_t *retard;

	retard = G_Spawn();
	if (teamnum)
	{
		retard->teamnum = teamnum;
		DEFENSE_TEAM = teamnum;
		retard->takedamage = DAMAGE_YES;
	}
	else
	{
		retard->takedamage = DAMAGE_NO; 
	}
	retard->svflags |= SVF_MONSTER;
	retard->classname = "retard";
	retard->pain = retard_pain;
	retard->yaw_speed = 20;
	retard->health = RETARD_HEALTH;
	retard->max_health = retard->health;
	retard->gib_health = -150;
	retard->mass = 200;
	retard->monsterinfo.power_armor_power = 0;
	retard->monsterinfo.power_armor_type = POWER_ARMOR_NONE;
	retard->clipmask = MASK_MONSTERSOLID;
	retard->movetype = MOVETYPE_STEP;
	retard->s.effects |= EF_PLASMA;
	retard->s.renderfx |= RF_IR_VISIBLE;
	retard->monsterinfo.stand = retard_stand;
	retard->solid = SOLID_BBOX;
	retard->think = retard_think;
	retard->touch = retard_touch;
	retard->die = retard_die;
	retard->mtype = M_RETARD;
	VectorSet(retard->mins, -16, -16, -24);
	VectorSet(retard->maxs, 16, 16, 32);
	retard->s.modelindex = gi.modelindex("models/monsters/insane/tris.md2");
	gi.linkentity(retard);
	retard->monsterinfo.currentmove = &retard_move_stand_insane;
	FindValidSpawnPoint(retard, false);
	retard->nextthink = level.time + 1;
//	gi.dprintf("retard created\n");
	if (teamnum)
		retard_teleport(retard);
}

void PTRInit (void)
{
	int		i;
	char	*s;
	edict_t *cl_ent;

	OrganizeTeams(true);
	gi.bprintf(PRINT_HIGH, "Find the retard!\n");
	SpawnRetard(0);
	

	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (G_EntExists(cl_ent))
		{
			s = Info_ValueForKey (cl_ent->client->pers.userinfo, "skin");
			AssignTeamSkin(cl_ent, s);
		}
	}
	
}

int	TeamScore (int teamnum)
{
	int i, score=0;
	edict_t *cl_ent;

	for (i=0 ; i<game.maxclients ; i++) {
		cl_ent = g_edicts+1+i;
		if (!G_EntExists(cl_ent))
			continue;
		if (cl_ent->teamnum != teamnum)
			continue;
		score += cl_ent->client->pers.score;
	}
	return score;
}

qboolean HasFlag (edict_t *ent)
{
	if (!ent->client)
		return false;

	if (ent->client->pers.inventory[flag_index] > 0)
		return true;
	if (ent->client->pers.inventory[red_flag_index] > 0)
		return true;
	if (ent->client->pers.inventory[blue_flag_index] > 0)
		return true;
	if (ent->client->pers.inventory[halo_index] > 0)
		return true;
	return false;
}


