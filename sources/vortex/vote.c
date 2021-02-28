#include "g_local.h"

void ShowVoteMapMenu(edict_t *ent, int pagenum, int mapmode);

//************************************************************************************************
//		**VOTING UTILS**
//************************************************************************************************

#ifdef OLD_VOTE_SYSTEM // Paril
static votes_t votes[MAX_VOTERS];
int numVotes = 0;

void CountVotes();
#else
votes_t currentVote = {false, 0, 0, "", ""};
int numVotes = 0, numVoteNo;
char* text1 = NULL, *text2 = NULL, *smode = NULL;
float voteTimeLeft = 0;
edict_t *voter;
static char strBuffer[1024];
#endif

//************************************************************************************************

#ifndef OLD_VOTE_SYSTEM
qboolean V_VoteInProgress()
{
	return voteTimeLeft > 0;
}
#endif

void V_ChangeMap(v_maplist_t *maplist, int mapindex, int gamemode)
{
	//char buf[8];
	int timelimit;

	//Change the mode
	switch(gamemode)
	{
		case MAPMODE_PVP:
		{
			// player versus player
			gi.cvar_set("ffa", "0");
			gi.cvar_set("domination", "0");
			gi.cvar_set("ctf", "0");
			gi.cvar_set("pvm", "0");
			gi.cvar_set("invasion", "0");
			gi.cvar_set("fraglimit", "50"); // vrxchile 2.5: less time per round (more dynamic pvp)
			gi.cvar_set("timelimit", "0");
			gi.cvar_set("trading", "0");
			//gi.cvar_set("hw", "0");
			gi.cvar_set("tbi", "0");
			//gi.cvar_set("dm_monsters", "0");				
		}
		break;
		case MAPMODE_PVM:
		{
			// player versus monsters
			timelimit = Lua_GetVariable("pvm_timelimit", 10)+pregame_time->value/60;
			gi.cvar_set("ffa", "0");
			gi.cvar_set("domination", "0");
			gi.cvar_set("ctf", "0");
			gi.cvar_set("pvm", "1");
			gi.cvar_set("invasion", "0");
			gi.cvar_set("fraglimit", "0");
			gi.cvar_set("timelimit", va("%d", timelimit));
			gi.cvar_set("trading", "0");
			//gi.cvar_set("hw", "0");
			gi.cvar_set("tbi", "0");
			//if (dm_monsters->value < 8)
			//	gi.cvar_set("dm_monsters", "8");
			//else
			//	gi.cvar_set("dm_monsters", itoa(ActivePlayers(), buf, 10));
		}
		break;
		case MAPMODE_DOM:
		{
			timelimit = Lua_GetVariable("dom_timelimit", 10)+pregame_time->value/60;
			// domination mode
			gi.cvar_set("ffa", "0");
			gi.cvar_set("domination", "1");
			gi.cvar_set("ctf", "0");
			gi.cvar_set("pvm", "0");
			gi.cvar_set("invasion", "0");
			gi.cvar_set("fraglimit", "0");
			gi.cvar_set("timelimit", va("%d", timelimit));
			gi.cvar_set("trading", "0");
			gi.cvar_set("tbi", "0");
		
		}
		break;
		case MAPMODE_CTF:
		{
			timelimit = Lua_GetVariable("ctf_timelimit", 10)+pregame_time->value/60;
			// ctf mode
			gi.cvar_set("ffa", "0");
			gi.cvar_set("domination", "0");
			gi.cvar_set("ctf", "1");
			gi.cvar_set("pvm", "0");
			gi.cvar_set("invasion", "0");
			gi.cvar_set("fraglimit", "0");
			gi.cvar_set("timelimit", va("%d", timelimit));
			gi.cvar_set("trading", "0");
			//gi.cvar_set("hw", "0");
			gi.cvar_set("tbi", "0");
			//gi.cvar_set("dm_monsters", "0");
		}
		break;
		case MAPMODE_FFA:
		{
			timelimit = Lua_GetVariable("ffa_timelimit", 15)+pregame_time->value/60;
			// free for all mode
			gi.cvar_set("ffa", "1");
			gi.cvar_set("domination", "0");
			gi.cvar_set("ctf", "0");
			gi.cvar_set("pvm", "0");
			gi.cvar_set("invasion", "0");
			gi.cvar_set("fraglimit", "100");
			gi.cvar_set("timelimit", va("%d", timelimit));
			gi.cvar_set("trading", "0");
			
			//gi.cvar_set("hw", "0");
			gi.cvar_set("tbi", "0");
			//if (dm_monsters->value < 4)
			//	gi.cvar_set("dm_monsters", "4");
			//else
			//	gi.cvar_set("dm_monsters", itoa(ActivePlayers(), buf, 10));
		}
		break;
		case MAPMODE_INV:
		{
			timelimit = Lua_GetVariable("inv_timelimit", 20)+pregame_time->value/60;
			// invasion mode
			gi.cvar_set("ffa", "0");
			gi.cvar_set("domination", "0");
			gi.cvar_set("ctf", "0");
			gi.cvar_set("pvm", "1");
			gi.cvar_set("invasion", "1");
			gi.cvar_set("fraglimit", "0");
			gi.cvar_set("timelimit", va("%d", timelimit));
			gi.cvar_set("trading", "0");
			//gi.cvar_set("hw", "0");
			gi.cvar_set("tbi", "0");
			//gi.cvar_set("dm_monsters", "4");
		}
		break;
		case MAPMODE_INH:
		{
			timelimit = Lua_GetVariable("inh_timelimit", 25)+pregame_time->value/60;
			// invasion mode - hard
			gi.cvar_set("ffa", "0");
			gi.cvar_set("domination", "0");
			gi.cvar_set("ctf", "0");
			gi.cvar_set("pvm", "1");
			gi.cvar_set("invasion", "2");
			gi.cvar_set("fraglimit", "0");
			gi.cvar_set("timelimit", va("%d", timelimit));
			gi.cvar_set("trading", "0");
			//gi.cvar_set("hw", "0");
			gi.cvar_set("tbi", "0");
			//gi.cvar_set("dm_monsters", "4");
		}
		break;
		case MAPMODE_TRA:
		{
			gi.cvar_set("ffa", "0");
			gi.cvar_set("domination", "0");
			gi.cvar_set("ctf", "0");
			gi.cvar_set("pvm", "0");
			gi.cvar_set("invasion", "0");
			gi.cvar_set("fraglimit", "0");
			gi.cvar_set("trading", "1");
			gi.cvar_set("timelimit", "0");
			//gi.cvar_set("hw", "0");
			gi.cvar_set("tbi", "0");
			break;
		}
		case MAPMODE_VHW: // vortex holy wars
			{
				timelimit = Lua_GetVariable("vhw_timelimit", 10)+pregame_time->value/60;
				gi.cvar_set("ffa", "0");
				gi.cvar_set("domination", "0");
				gi.cvar_set("ctf", "0");
				gi.cvar_set("pvm", "0");
				gi.cvar_set("invasion", "0");
				gi.cvar_set("fraglimit", "0");
				gi.cvar_set("trading", "0");
				gi.cvar_set("timelimit", va("%d", timelimit));
				//gi.cvar_set("hw", "1");
				gi.cvar_set("tbi", "0");
				break;
			}
		case MAPMODE_TBI:
			{
				timelimit = Lua_GetVariable("dts_timelimit", 15)+pregame_time->value/60;
				gi.cvar_set("ffa", "0");
				gi.cvar_set("domination", "0");
				gi.cvar_set("ctf", "0");
				gi.cvar_set("pvm", "0");
				gi.cvar_set("invasion", "0");
				gi.cvar_set("fraglimit", "0");
				gi.cvar_set("trading", "0");
				gi.cvar_set("timelimit", va("%d", timelimit));
			//	gi.cvar_set("hw", "0");
				gi.cvar_set("tbi", "1");
			}
		break;
	}

	//Reset player votes
#ifdef OLD_VOTE_SYSTEM // Paril
	memset(votes, 0, sizeof(votes_t) * MAX_VOTERS);
#else
	numVotes = 0;
	numVoteNo = 0;
	voteTimeLeft = 0;
	memset (&currentVote, 0, sizeof(currentVote));
#endif

	gi.dprintf("Votes have been reset.\n");

	//Change the map
	if (mapindex < maplist->nummaps) // Assume everyone has proper maplists? Nope -az
	{
		strcpy(level.nextmap, maplist->maps[mapindex].name);
		level.r_monsters = maplist->maps[mapindex].monsters;//4.5
		gi.dprintf("Next map is: %s\n", level.nextmap);
	}else
	{
		gi.dprintf("Invalid map index (%d) defaulting to current map.\n", mapindex);
		strcpy(level.nextmap, level.mapname);
	}
	if (JoinedPlayers() == 0)
		ExitLevel();
}

//************************************************************************************************

v_maplist_t *GetMapList(int mode)
{
	if(Lua_GetIntVariable("UseLuaMaplists", 0))
		v_LoadMapList(mode); // reload the map list (lua conditional maplists)

	//Change the map
	switch(mode)
	{
	case MAPMODE_PVP:	return &maplist_PVP;
	case MAPMODE_DOM:	return &maplist_DOM;
	case MAPMODE_PVM:	return &maplist_PVM;
	case MAPMODE_CTF:	return &maplist_CTF;
	case MAPMODE_FFA:	return &maplist_FFA;
	case MAPMODE_INV:	return &maplist_INV;
	case MAPMODE_TRA:   return &maplist_TRA; // vrxchile 2.5: trading mode maplist
	case MAPMODE_INH:	return &maplist_INH; // vrxchile 2.6: invasion hard mode
	//case MAPMODE_VHW:	return &maplist_VHW; // vrxchile 3.0: vortex holy wars mode
//	case MAPMODE_TBI:	return &maplist_TBI; // vrxchile 3.4: destroy the spawn mode
	default:
		gi.dprintf("ERROR in GetMapList(). Incorrect map mode. (%d)\n", mode);
		return 0;
	}
}

//************************************************************************************************

//Removes ip addresses not on the server at this time
#ifdef OLD_VOTE_SYSTEM // Paril
void CheckPlayerVotes (void)
{
	int i, j;
    edict_t *player;
	qboolean found;

	//Check all the votes
    for(i = 0; i < MAX_VOTERS; ++i)
	{
		if (!votes[i].used)
			continue;

		found = false;

		//Check all the players and look for a matching ip
		for (j = 1; j <= game.maxclients; j++)
		{
			player = &g_edicts[j];

			if (!player->inuse || !player->client)
				continue;
		
			//If ip was found on the server
    		if (Q_stricmp(votes[i].ip, player->client->pers.current_ip) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			//Delete the vote
			gi.dprintf("Removing vote for: %s [%s]\n", votes[i].name, votes[i].ip);
			memset(&votes[i], 0, sizeof(votes_t));
		}
	}
}
#endif

//************************************************************************************************

#ifdef OLD_VOTE_SYSTEM // Paril
votes_t *FindVote(edict_t *ent)
{
	int i;

	//Try to find the player's vote
	for (i = 0; i < MAX_VOTERS; ++i)
	{
		if (!votes[i].used)
			continue;
		
		if (Q_stricmp(votes[i].ip, ent->client->pers.current_ip) == 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "Your vote has been changed.\n");
			return &votes[i];
		}
		
		if (Q_strcasecmp(votes[i].name, ent->client->pers.netname) == 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "Your vote has been changed.\n");
			return &votes[i];
		}

	}

	//Can't find a previous vote, so find an empty slot
	for (i = 0; i < MAX_VOTERS; ++i)
	{
		if(votes[i].used)
			continue;
		return &votes[i];
	}

	return NULL;
}
#endif

//************************************************************************************************

#ifdef OLD_VOTE_SYSTEM // Paril
qboolean AddVote(edict_t *ent, int mode, int mapnum)
{
	v_maplist_t *maplist = GetMapList(mode);
	votes_t *myvote;

	if (!maplist) return false;

	//Find the player's vote
	myvote = FindVote(ent);
  
	//check for valid choice
	if (myvote && mode && (maplist->nummaps > mapnum))
	{
		//Add the vote
		strcpy(myvote->ip, ent->client->pers.current_ip);
		strcpy(myvote->name, ent->myskills.player_name);
		myvote->mapindex = mapnum;
		myvote->mode = mode;
		myvote->used = true;

		//Notify everyone
		gi.bprintf(PRINT_HIGH, "%s voted for %s.\n", ent->myskills.player_name, maplist->maps[mapnum].name);
		switch(mode)
		{
			case MAPMODE_PVP:	gi.bprintf(PRINT_HIGH, "%s voted for Player Vs. Player (PvP).\n", ent->myskills.player_name);	break;
			case MAPMODE_PVM:	gi.bprintf(PRINT_HIGH, "%s voted for Player Vs. Monster (PvM).\n", ent->myskills.player_name);	break;
			case MAPMODE_DOM:	gi.bprintf(PRINT_HIGH, "%s voted for Domination.\n", ent->myskills.player_name);			break;
			case MAPMODE_CTF:	gi.bprintf(PRINT_HIGH, "%s voted for Capture The Flag (CTF).\n", ent->myskills.player_name);					break;
			case MAPMODE_FFA:	gi.bprintf(PRINT_HIGH, "%s voted for Free For All (FFA).\n", ent->myskills.player_name);	break;
			case MAPMODE_INV:	gi.bprintf(PRINT_HIGH, "%s voted for Invasion (INV).\n", ent->myskills.player_name); break;
			case MAPMODE_TRA:	gi.bprintf(PRINT_HIGH, "%s voted for  Trading ", ent->myskills.player_name); break;
			case MAPMODE_INH:	gi.bprintf(PRINT_HIGH, "%s voted for Invasion (Hard mode) ", ent->myskills.player_name); break;
			case MAPMODE_VHW:	gi.bprintf(PRINT_HIGH, "%s voted for Vortex HolyWars ", ent->myskills.player_name); break;
			case MAPMODE_TBI:   gi.bprintf(PRINT_HIGH, "%s voted for Destroy the Spawn ", ent->myskills.player_name); break;
		}
		return true;
	}
	else 
	{
		gi.dprintf("Error in AddVote(): map number = %d, mode number = %d, address of myvote = %d\n", mapnum, mode, myvote);
		return false;
	}
}
#else
void AddVote(edict_t *ent, int mode, int mapnum)
{
	v_maplist_t *maplist = GetMapList(mode);

	if (!maplist)
		return;

	if(V_VoteInProgress())
		return;

#ifdef OLD_VOTE_SYSTEM
	CountVotes();
#endif
  
	//check for valid choice
	if (mode && (maplist->nummaps > mapnum))
	{
		char tempBuffer[1024];
		//Add the vote
		voter = ent;
		numVotes = 1;
		numVoteNo = 0;
		ent->client->resp.HasVoted = true;
		strcpy(currentVote.ip, ent->client->pers.current_ip);
		strcpy(currentVote.name, ent->myskills.player_name);
		currentVote.mapindex = mapnum;
		currentVote.mode = mode;
		currentVote.used = true;
		voteTimeLeft = level.time + 90;


		Com_sprintf (tempBuffer, 1024, "%s started a vote for ", ent->myskills.player_name);
		switch(mode)
		{
			case MAPMODE_PVP:	smode =  "Player vs. Player (PvP) ";	break;
			case MAPMODE_PVM:	smode =  "Player vs. Monster (PvM) ";	break;
			case MAPMODE_DOM:	smode =  "Domination (DOM) ";			break;
			case MAPMODE_CTF:	smode =  "Capture The Flag (CTF) ";	break;
			case MAPMODE_FFA:	smode =  "Free For All (FFA) ";		break;
			case MAPMODE_INV:	smode =  "Invasion ";				break;
			case MAPMODE_TRA:	smode =  "Trading ";					break;
			case MAPMODE_INH:	smode =  "Invasion (Hard mode) ";	break;
			case MAPMODE_VHW:	smode =  "Vortex HolyWars ";			break;
			case MAPMODE_TBI:   smode =  "Destroy the Spawn ";		break;
		}
		Com_sprintf (tempBuffer, 1024, "%s%son %s\n", tempBuffer, smode, maplist->maps[mapnum].name);

		text1 = HiPrint(va("%s", smode));
		text2 = HiPrint(va("%s", maplist->maps[mapnum].name));
		Com_sprintf (strBuffer, 1024, "vote in progress: %son %s\n", text1, text2);
		V_Free(text1); V_Free(text2);
		
		
		gi.configstring(CS_GENERAL+MAX_CLIENTS+1, strBuffer);

		G_PrintGreenText(tempBuffer);
		gi.sound(&g_edicts[0], CHAN_VOICE, gi.soundindex("misc/comp_up.wav"), 1, ATTN_NONE, 0);


		gi.bprintf (PRINT_HIGH, "Please place your vote by typing 'vote yes' or 'vote no' within the next %d seconds.\n", floattoint(voteTimeLeft-level.time));

	}
	else 
		gi.dprintf("Error in AddVote(): map number = %d, mode number = %d\n", mapnum, mode);
}
#endif

//************************************************************************************************

#ifdef OLD_VOTE_SYSTEM // Paril
int CountTotalModeVotes(int mode)
{
	int count = 0;
	int i;

	for (i = 0; i < MAX_VOTERS; ++i)
		if (votes[i].used && votes[i].mode == mode)
			++count;
	return count;
}
#endif

//************************************************************************************************

#ifdef OLD_VOTE_SYSTEM // Paril
int CountMapVotes(int mapIndex, int mode)
{
	int count = 0;
	int i;
		
	//count the number of votes for this map
	for (i = 0; i < MAX_VOTERS; ++i)
	{
		if ((votes[i].used) && (votes[i].mode == mode) && (votes[i].mapindex == mapIndex))
			++count;
	}
	return count;
}
#endif

#define CHANGE_NOW		1	// change the map/mode immediately
#define CHANGE_LATER	2	// change the map/mode at the completion of vote duration

int V_VoteDone ()
{
	int players = JoinedPlayers();

	if (players < 1)
		return 0;

#ifdef OLD_VOTE_SYSTEM
	CountVotes();
#endif

	if (numVotes >= 0.75*players)
		return CHANGE_NOW;

	if (numVotes > 0.5*players)
		return CHANGE_LATER;

	return 0;
}

//************************************************************************************************

//Finds the index of the map with the most votes.
//Returns: -1 if there is no winning map (a tie)
#ifdef OLD_VOTE_SYSTEM // Paril
int FindBestMap(int mode)
{
	v_maplist_t *maplist = GetMapList(mode);
	int i;
	int count;
	int bestIndex = -1;
	int bestCount = 0;

	//iterate through each map
	for (i = 0; i < maplist->nummaps; ++i)
	{
		//count the number of votes for this map
		count = CountMapVotes(i, mode);
		
		//Check to see if this map has the most votes
		if (count > bestCount)
		{
			bestCount = count;
			bestIndex = i;
		}
		//Ties don't count
		else if ((count > 0) && (count == bestCount))
		{
			bestIndex = -1;
		}
	}

	//gi.dprintf("bestcount=%d, index=%d\n", bestCount, bestIndex);
	//return map with the most votes
	return bestIndex;
}
#else
int FindBestMap(int mode)
{
	//if (numVotes > 0.5*ActivePlayers())
	//	return currentVote.mapindex;
	if (V_VoteDone())
		return currentVote.mapindex;
	return -1;
}
#endif

//************************************************************************************************
#ifdef OLD_VOTE_SYSTEM
void CountVotes()
{
	int pvp, pvm, dom, ctf, ffa, inv, players, total_votes;
	int inh, vhw, tra, dts;
	// count the votes for each mode
	pvp = CountTotalModeVotes(MAPMODE_PVP);
	pvm = CountTotalModeVotes(MAPMODE_PVM);
	dom = CountTotalModeVotes(MAPMODE_DOM);
	ctf = CountTotalModeVotes(MAPMODE_CTF);
	ffa = CountTotalModeVotes(MAPMODE_FFA);
	inv = CountTotalModeVotes(MAPMODE_INV);
	inh = CountTotalModeVotes(MAPMODE_INH);
	dts = CountTotalModeVotes(MAPMODE_TBI);
	vhw = CountTotalModeVotes(MAPMODE_VHW);
	tra = CountTotalModeVotes(MAPMODE_TRA);


	total_votes = pvp + pvm + dom + ctf + ffa + inv + inh + dts + vhw + tra;
	numVotes = total_votes;
}
#endif

int V_AttemptModeChange(qboolean endlevel)
{
#ifdef OLD_VOTE_SYSTEM // Paril
	int pvp, pvm, dom, ctf, ffa, inv, players, total_votes;
	int inh, vhw, tra, dts;
	players = ActivePlayers();

	if (players < 1)
		return 0; // not enough players

	// count the votes for each mode
	pvp = CountTotalModeVotes(MAPMODE_PVP);
	pvm = CountTotalModeVotes(MAPMODE_PVM);
	dom = CountTotalModeVotes(MAPMODE_DOM);
	ctf = CountTotalModeVotes(MAPMODE_CTF);
	ffa = CountTotalModeVotes(MAPMODE_FFA);
	inv = CountTotalModeVotes(MAPMODE_INV);
	inh = CountTotalModeVotes(MAPMODE_INH);
	dts = CountTotalModeVotes(MAPMODE_TBI);
	vhw = CountTotalModeVotes(MAPMODE_VHW);
	tra = CountTotalModeVotes(MAPMODE_TRA);


	total_votes = pvp + pvm + dom + ctf + ffa + inv + inh + dts + vhw + tra;
	numVotes = total_votes;

	//gi.dprintf("total_votes=%d\n", total_votes);
	//gi.dprintf("%d,%d,%d,%d,%d\n", pvp,pvm,dom,ctf,ffa);

	if (total_votes < 0.33*players)
		return 0; // need at least 1/3rd of server to change mode

	// change mode immediately if the majority
	// of players are voting
	if (!endlevel && (total_votes < 0.8*players))
		return 0;

	if (pvp && (pvp>pvm) && (pvp>dom) && (pvp>ctf) && (pvp>ffa) && (pvp>inv) && (pvp>inh) && (pvp>dts) && (pvp>vhw) && (pvp>tra))
		return MAPMODE_PVP;
	else if (pvm && (pvm>pvp) && (pvm>dom) && (pvm>ctf) && (pvm>ffa) && (pvm>inv) && (pvm>inh) && (pvm>dts) && (pvm>vhw) && (pvm>tra))
		return MAPMODE_PVM;
	else if (dom && (dom>pvp) && (dom>pvm) && (dom>ctf) && (dom>ffa) && (dom>inv) && (dom>inh) && (dom>dts) && (dom>vhw) && (dom>tra))
		return MAPMODE_DOM;
	else if (ctf && (ctf>pvp) && (ctf>pvm) && (ctf>dom) && (ctf>ffa) && (ctf>inv) && (ctf>inh) && (ctf>dts) && (ctf>vhw) && (ctf>tra))
		return MAPMODE_CTF;
	else if (ffa && (ffa>pvp) && (ffa>pvm) && (ffa>dom) && (ffa>ctf) && (ffa>inv) && (ffa>inh) && (ffa>dts) && (ffa>vhw) && (ffa>tra))
		return MAPMODE_FFA;
	else if (inv && (inv>pvp) && (inv>pvm) && (inv>dom) && (inv>ctf) && (inv>ffa) && (inv>inh) && (inv>dts) && (inv>vhw) && (inv>tra))
		return MAPMODE_INV;
	else if (inh && (inh>pvp) && (inh>pvm) && (inh>dom) && (inh>ctf) && (inh>ffa) && (inh>inv) && (inh>dts) && (inh>vhw) && (inh>tra))
		return MAPMODE_INH;
	else if (tra && (tra>pvp) && (tra>pvm) && (tra>dom) && (tra>ctf) && (tra>ffa) && (tra>inh) && (tra>dts) && (tra>vhw) && (tra>inv))
		return MAPMODE_TRA;
	else if (dts && (dts>pvp) && (dts>pvm) && (dts>dom) && (dts>ctf) && (dts>ffa) && (dts>inh) && (dts>inv) && (dts>vhw) && (dts>tra))
		return MAPMODE_TBI;
	else if (vhw && (vhw>pvp) && (vhw>pvm) && (vhw>dom) && (vhw>ctf) && (vhw>ffa) && (vhw>inh) && (vhw>inv) && (vhw>dts) && (vhw>tra))
		return MAPMODE_VHW;


	return 0;
#else

#if FORCE_PVP_WITH_A_LOT_OF_PLAYERS
	int max_players, players = ActivePlayers();

	//4.4 forcibly switch to PvP if we are in Invasion/PvM and there are many players
	max_players = 0.25 * maxclients->value;
	if (max_players < 4)
		max_players = 4;
	if (players >= max_players && (pvm->value || invasion->value) 
		&& (currentVote.mode == MAPMODE_PVM || currentVote.mode == MAPMODE_INV))
	{
		gi.bprintf(PRINT_HIGH, "Forcing switch to FFA because there are too many players!\n");
		return MAPMODE_FFA;
	}
#endif
	// did the vote pass?
	if (V_VoteDone())
		return currentVote.mode;

	// vote failed
	return 0;
#endif
}

#ifndef OLD_VOTE_SYSTEM // Paril


void RunVotes ()
{
	if (voteTimeLeft == -1)
		return;

	if (voteTimeLeft == level.time+60.0f)
		gi.bprintf (PRINT_CHAT, "One minute left to place your vote.\n");
	else if (voteTimeLeft == level.time+30.0f)
		gi.bprintf (PRINT_CHAT, "Thirty seconds left to place your vote.\n");
	else if (voteTimeLeft == level.time+10)
		gi.bprintf (PRINT_CHAT, "Ten seconds left to place your vote.\n");
	else if (voteTimeLeft == level.time)
	{
		// Finish vote
		// Did we reach a majority?
		if (V_VoteDone())
		{
			// Tell everyone
			G_PrintGreenText("A majority was reached! Vote passed!\n");
			voteTimeLeft = -1;
			//Change the map
			EndDMLevel();
		}
		else
		{
			edict_t *e;
			int i;

			gi.bprintf (PRINT_CHAT, "Vote failed.\n");
			numVotes = 0;
			voteTimeLeft = 0;
			memset (&currentVote, 0, sizeof(currentVote));
			
			for_each_player(e, i)
			{
				e->client->resp.HasVoted = false;
			}

			voter->client->resp.VoteTimeout = level.time + 20;
			voter = NULL;
		}
	}
	else
	{	
		if (V_VoteDone() == CHANGE_NOW)
		{
			// Tell everyone
			gi.sound(&g_edicts[0], CHAN_AUTO, gi.soundindex("misc/keyuse.wav"), 1, ATTN_NONE, 0);
			G_PrintGreenText("A majority was reached! Vote passed!\n");
			//Change the map
			EndDMLevel();
		}
	}

}
#else

void RunVotes()
{
	if (V_VoteDone() == CHANGE_NOW)
	{
		// Tell everyone
		gi.sound(&g_edicts[0], CHAN_AUTO, gi.soundindex("misc/keyuse.wav"), 1, ATTN_NONE, 0);
		G_PrintGreenText("A majority was reached! Vote passed!\n");
		//Change the map
		EndDMLevel();
	}
}

#endif


//************************************************************************************************
//		**VOTE MAP SELECT MENU**
//************************************************************************************************

extern cvar_t *adminctrl;

void ShowVoteMapMenu_handler(edict_t *ent, int option)
{
	if (option == 66666)
	{
		closemenu(ent);
		return;
	}
	//Multi-page navigation
	else if (option > 20000)
	{
		int mode = (option / 1000) - 20;
		int nextpage = option % 1000;		//page number we will end up in (page 0 = main menu)
		if (nextpage != 0)
			ShowVoteMapMenu(ent, nextpage, mode);
		else
			ShowVoteModeMenu(ent);
	}
	else
	{
		int mode = option / 1000;
		int mapnum = option % 1000;
		v_maplist_t *maplist = GetMapList(mode);

		if (!maplist) return;

		//Admins directly control the map change
		if (ent->myskills.administrator && adminctrl->value) // IF the cvar is enabled.
		{
			VortexEndLevel();
            V_ChangeMap(maplist, mapnum-1, mode);
			ExitLevel();
			return;
		}
		else if (!strcmp(maplist->maps[mapnum-1].name, level.mapname))
		{
			safe_cprintf(ent, PRINT_HIGH, "Can't vote for current map!\n");
			return;
		}
		else
		{
			//Add the player's vote
			AddVote(ent, mode, mapnum-1);
		}
		
		//Are we changing the map?
		//if (V_AttemptModeChange(false))
		//{
			//Change the map
		//	EndDMLevel();
		//}
	}
}

//************************************************************************************************

void ShowVoteMapMenu(edict_t *ent, int pagenum, int mapmode)
{
	int i;
	v_maplist_t *maplist = GetMapList(mapmode);
	qboolean EndOfList = false;
	
	if (!maplist) return;

	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//Print header
	addlinetomenu(ent, "Vote for map:", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	for (i = 10 * (pagenum-1); i < 10 * pagenum; ++i)
	{
		char buf[20];
		if (i >= maplist->nummaps)
		{
			EndOfList = true;
			break;
		}
		strcpy(buf, maplist->maps[i].name);

		padRight(buf, 15);
#ifdef OLD_VOTE_SYSTEM // Paril
		addlinetomenu(ent, va(" %s (%d)", buf, CountMapVotes(i, mapmode)), i+1+(mapmode * 1000));
#else
		addlinetomenu(ent, va(" %s", buf), i+1+(mapmode * 1000));
#endif
	}

	//Menu footer
	addlinetomenu(ent, " ", 0);
#ifdef OLD_VOTE_SYSTEM // Paril
	addlinetomenu(ent, va("    %d votes total.", CountTotalModeVotes(mapmode)), 0);
#endif
	addlinetomenu(ent, " ", 0);

	//Set current line
	if (i % 10 != 0) ent->client->menustorage.currentline = 6 + (i % 10);
	else ent->client->menustorage.currentline = 16;

	if ((!EndOfList) && (i != maplist->nummaps))
	{
		addlinetomenu(ent, " Next", (20000 + pagenum + 1) + (mapmode * 1000)); //ex: pvp, page 3 = 11004
	}
	else
	{
		addlinetomenu(ent, " ", 0);
		++ent->client->menustorage.currentline;
	}
	
	addlinetomenu(ent, " Previous", (20000 + pagenum - 1) + (mapmode * 1000));	//ex: pvm, page 3 = 12002
	addlinetomenu(ent, " Exit", 66666);

	//Set handler
	setmenuhandler(ent, ShowVoteMapMenu_handler);

	//Display the menu
	showmenu(ent);
}

//************************************************************************************************
//		**VOTE MODE SELECT MENU**
//************************************************************************************************

void ShowVoteModeMenu_handler(edict_t *ent, int option)
{
	if (option == 66666)
	{
		closemenu(ent);
		return;
	}

	//Select a mode, and pick a map
	ShowVoteMapMenu(ent, 1, option);	//page = 1, gamemode = option
}

//************************************************************************************************

void KillMyVote (edict_t *ent)
{
	if (ent->client->resp.HasVoted) // voted?
	{
		ent->client->resp.HasVoted = false;
		if (ent->client->resp.voteType == 1) // voted yes?
			numVotes--;
		else
			numVoteNo--; // voted no..
	}
}

qboolean ThereIsOneLevelTen()
{
	int i;
	for (i = 1; i <= maxclients->value; i++)
	{
		edict_t* cl = g_edicts+i;
		if (cl->client && !G_IsSpectator(cl) && cl->inuse)
			if (cl->myskills.level >= 10)
				return true;
	}
	return false;
}

void ShowVoteModeMenu(edict_t *ent)
{
#ifdef OLD_VOTE_SYSTEM // Paril
	int pvp, pvmon, dom, ctf, ffa, total, inv, inh, vhw, tra, dts;
	int	players, lastline=8, min_players;

	//Voting enabled?
	if (!voting->value)
		return;
	// don't allow non-admin voting during pre-game to allow players time to connect
	if (!ent->myskills.administrator && (level.time < 30.0)) // allow 30 seconds for players to connect
		return;

	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	pvp = CountTotalModeVotes(MAPMODE_PVP);
	pvmon = CountTotalModeVotes(MAPMODE_PVM);
	dom = CountTotalModeVotes(MAPMODE_DOM);
	ctf = CountTotalModeVotes(MAPMODE_CTF);
	ffa = CountTotalModeVotes(MAPMODE_FFA);
	inv = CountTotalModeVotes(MAPMODE_INV);
	inh = CountTotalModeVotes(MAPMODE_INH);
	dts = CountTotalModeVotes(MAPMODE_TBI);
	vhw = CountTotalModeVotes(MAPMODE_VHW);
	tra = CountTotalModeVotes(MAPMODE_TRA);

	total = pvp + pvmon + dom + ctf + ffa + inv;
	
	//Print header
	addlinetomenu(ent, "Vote for game mode:", MENU_GREEN_LEFT);
	addlinetomenu(ent, " ", 0);

	addlinetomenu(ent, va(" Deathmatch         (%d)", pvp), MAPMODE_PVP);
	addlinetomenu(ent, va(" Free For All       (%d)", ffa), MAPMODE_FFA);
//GHz START
	players = ActivePlayers();
	// pvm and invasion are only available when there are few players on the server
	if (0.33 * maxclients->value < 4)
		min_players = 4;
	else
		min_players = 0.25 * maxclients->value;
	if (/*pvm->value ||*/ (players < min_players))
	{
		addlinetomenu(ent, va(" Player Vs. Monster (%d)", pvmon), MAPMODE_PVM);
		lastline++;
	}
	// invasion mode
	if (invasion_enabled->value && (players < min_players))
	{
		addlinetomenu(ent, va(" Invasion           (%d)", inv), MAPMODE_INV);
		lastline++;
	}else
	{
		if (invasion_enabled->value && AveragePlayerLevel() > 10)
		{
			addlinetomenu(ent, " Invasion (Hard mode)  (%d)", MAPMODE_INH);
			lastline++;
		}
	}
	
	// domination available when there are at least 4 players
	if (players >= 4)
	{
		addlinetomenu(ent, va(" Domination         (%d)", dom), MAPMODE_DOM);
		lastline++;
	}
	// CTF available when there are at least 4 players
	if (players >= 4)
	{
		addlinetomenu(ent, va(" CTF                (%d)", ctf), MAPMODE_CTF);
		lastline++;
	
		addlinetomenu(ent, va(" Vortex Holywars    (%d)", vhw), MAPMODE_VHW);
		lastline++;

		addlinetomenu(ent, va(" Destroy The Spawn  (%d)", dts), MAPMODE_TBI);
		lastline++;
	}
//GHz END

	// az begin
	// vrx chile 2.5: trading mode
	if (tradingmode_enabled->value)
	{
		addlinetomenu(ent, " Trading", MAPMODE_TRA);
		lastline++;
	}

	// az end
	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, va("    %d votes total.", total), 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " Exit", 66666);

	//Set handler
	setmenuhandler(ent, ShowVoteModeMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = lastline;

	//Display the menu
	showmenu(ent);
#else
	int	players, lastline=6, min_players;

	char *cmd2 = gi.argv(1);

	if (voteTimeLeft == -1)
		return;

	//Voting enabled?
	if (!voting->value)
		return;
	// don't allow non-admin voting during pre-game to allow players time to connect
	if (!ent->myskills.administrator && (level.time < 5.0)) // allow 5 seconds for players to connect
	{
		safe_cprintf(ent, PRINT_HIGH, "Please allow time for other players to connect.\n");
		return;
	}
	if (ent->client->resp.VoteTimeout > level.time)
		return;

	if (cmd2[0] == 'y' || cmd2[0] == 'Y')
	{
		if (!currentVote.mode)
			safe_cprintf(ent, PRINT_HIGH, "There is no vote taking place at this time.\n");
		else
		{
			// Did we already vote?
			if (ent->client->resp.HasVoted)
			{
				safe_cprintf(ent, PRINT_HIGH, "You already placed your vote.\n");
				return; // GTFO PLZ
			}
			ent->client->resp.HasVoted = true;
			numVotes++;
			
			G_PrintGreenText(va("%s voted Yes.\n", ent->client->pers.netname));
		}
		return;
	}
	else if (cmd2[0] == 'n' || cmd2[0] == 'N')
	{
		if (!currentVote.mode)
			safe_cprintf(ent, PRINT_HIGH, "There is no vote taking place at this time.\n");
		else
		{
			// Did we already vote?
			if (ent->client->resp.HasVoted)
			{
				safe_cprintf(ent, PRINT_HIGH, "You already placed your vote.\n");
				return; // GTFO PLZ
			}
			numVoteNo++;
			ent->client->resp.HasVoted = true;
			gi.bprintf (PRINT_CHAT, "%s voted No.\n", ent->client->pers.netname);
		}
		return;
	}
	// Bring open the menu
	else if (currentVote.mode)
	{
		// TO DO
		return;
	}

	//Usual menu stuff
	 if (!ShowMenu(ent))
        return;
	clearmenu(ent);
	
	//Print header
	addlinetomenu(ent, "Vote for game mode:", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

//GHz START
	players = ActivePlayers();
	// pvm and invasion are only available when there are few players on the server
#ifdef FORCE_PVP_WITH_A_LOT_OF_PLAYERS
	if (0.33 * maxclients->value < 4)
		min_players = 4;
	else
		min_players = 0.25 * maxclients->value;
#else
		min_players = 6;
#endif

	if (players > 1) // how could you ever play pvp alone? -az (bots not functional yet)
		addlinetomenu(ent, " Player vs. Player", MAPMODE_PVP);

	addlinetomenu(ent, " Free For All", MAPMODE_FFA);

	if (players < min_players)
	{
		addlinetomenu(ent, " Player vs. Monster", MAPMODE_PVM);
		lastline++;
	}

		// vrx chile 2.5: trading mode
		if (tradingmode_enabled->value)
		{
			addlinetomenu(ent, " Trading", MAPMODE_TRA);
			lastline++;
		}

	// invasion mode
	if (players < min_players)
	{
		if (invasion_enabled->value)
		{
			addlinetomenu(ent, " Invasion", MAPMODE_INV);
			lastline++;
		}
	}

	if (invasion_enabled->value && (ThereIsOneLevelTen() || ActivePlayers() >= min_players) )
	{
		addlinetomenu(ent, " Invasion (Hard mode)", MAPMODE_INH);
		lastline++;
	}
	
	// domination available when there are at least 4 players
	if (players >= 8)
	{
		addlinetomenu(ent, " Domination", MAPMODE_DOM);
		lastline++;
	}
	// CTF and vhw and tbi available when there are at least 4 players
	if (players >= 4)
	{
		addlinetomenu(ent, " CTF", MAPMODE_CTF);
		lastline++;

		if (gi.cvar("vhw_enabled", "1", 0)->value)
		{
			addlinetomenu(ent, " Vortex Holywars", MAPMODE_VHW);
			lastline++;
		}

		addlinetomenu(ent, " Destroy The Spawn", MAPMODE_TBI);
		lastline++;
	}

//GHz END
	//Menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " Exit", 66666);

	//Set handler
	setmenuhandler(ent, ShowVoteModeMenu_handler);

	//Set current line
	ent->client->menustorage.currentline = lastline;

	//Display the menu
	showmenu(ent);
#endif
}

