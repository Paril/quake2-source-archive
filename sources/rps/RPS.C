#include "g_local.h"
#include "q_devels.h"

// This file contains all the altered functions used for RPS and a list of changes to other functions.

// I made LOTS of small changes throughout the code, taking out anything with single player or coop code.
// As a result, many functions will be missing or changed, but this is not important to RPS itself.

// When looking for following changes, search for "MIKE"

// Added some things at the top of g_local.h
// Changed client_respawn_t in g_local.h
// Added some STAT_* in q_shared.h
// Changed gclient_s in g_local.h
// Added "autoteams" cvar in InitGame() in g_save.c and in g_main.c
// Changed G_SetStats() in p_hud.c
// Changed DeathmatchScoreboardMessage() in p_hud.c
// Changed CheckTeamDamage() in g_combat.c
// Changed T_Damage() in g_combat.c
// Changed itemlist [] in g_items.c
// Made rps_statusbar in g_spawn.c (and all referances to the SP statusbar to rps_statusbar)
// Changed SP_worldspawn() in g_spawn.c
// Changed several things in menu.c
// Changed PutClientInServer() in p_client.c
// Changed ClientBeginDeathmatch() in p_client.c
// Changed G_SetClientEffects() in p_view.c
// Changed ClientDisconnect() in p_client.c

extern void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

char *ClientTeam (edict_t *ent)
{
	if(ent->client)
	{
		if(ent->client->resp.team==ROCK)
			return "ROCK";
		else if(ent->client->resp.team==PAPER)
			return "PAPER";
		else if(ent->client->resp.team==SCISSORS)
			return "SCISSORS";
	}

	return "NONE";
}

char *ClientOpposingTeam(edict_t *ent)
{
	if(ent->client)
	{
		if(ent->client->resp.team==ROCK)
			return "PAPER";
		else if(ent->client->resp.team==PAPER)
			return "SCISSORS";
		else if(ent->client->resp.team==SCISSORS)
			return "ROCK";
	}

	return "NONE";
}

char *ClientVictimTeam(edict_t *ent)
{
	if(ent->client)
	{
		if(ent->client->resp.team==ROCK)
			return "SCISSORS";
		else if(ent->client->resp.team==PAPER)
			return "ROCK";
		else if(ent->client->resp.team==SCISSORS)
			return "PAPER";
	}

	return "NONE";
}

char *Color(team_t Team)
{
	switch(Team)
	{
	case ROCK: return "RED";
	case PAPER: return "BLUE";
	case SCISSORS: return "GREEN";
	default: return "NONE";
	}
}

void Cmd_Team_f (edict_t *ent)
{
	char *s;
	int Index;

	s=gi.args();

	for(Index=0;Index<strlen(s);++Index) /* Make all uppercase. */
		if(s[Index]>='a')
			s[Index]=s[Index]-32;

	if(strcmp(s,ClientTeam(ent))==0) //same team
	{
		gi.cprintf (ent, PRINT_HIGH, "You are already on the %s team, silly goose!\n", s);
		return;
	}
	else if(!*s) /* if no args. */
	{
		gi.cprintf (ent, PRINT_HIGH, "You are on the %s team.\n", ClientTeam(ent));
		return;
	}
	else if(strcmp(s,"ROCK")!=0 && strcmp(s,"PAPER")!=0 && strcmp(s,"SCISSORS")!=0) //it isn't good.
	{
		gi.cprintf(ent,PRINT_HIGH, "Unknown team %s.  Now enter it right, or I'll smack you around like a red-headed stepchild.\n",s);
		return;
	}

	if(strcmp(s,"ROCK")==0)
		PutClientOnTeam(ent,ROCK);
	else if(strcmp(s,"PAPER")==0)
		PutClientOnTeam(ent,PAPER);
	else if(strcmp(s,"SCISSORS")==0)
		PutClientOnTeam(ent,SCISSORS);

	gi.bprintf(PRINT_HIGH, "%s defected to the %s team.\n",ent->client->pers.netname,s);
}

int FlagsOnTeam(team_t Team)
{
	edict_t *player;
	int i,count=0;

	for_each_player(player,i)
	{
		if(player->client->resp.team==Team)
			count+=player->client->pers.inventory[ITEMLIST_FLAG];
	}

	return count;
}

qboolean Add_Flag(edict_t *other)
{
	const int FLAG_CARRIER_REWARD=2;
	const int MAX_LIFE=150;
	edict_t *player;
	int i;

	if (!other->client)
		return false;

	//Give the cube via inventory.
	other->client->pers.inventory[ITEMLIST_FLAG]++;

	if(FlagsOnTeam(other->client->resp.team) >= 3) //Capture.
	{
		switch(other->client->resp.team) /* Increment team scores. */
		{
		case ROCK: RockScore++;
					break;
		case PAPER: PaperScore++;
					break;
		case SCISSORS: ScissorsScore++;
					break;
		default: break;
		}

		for_each_player(player,i) //Clear flags, give rewards
		{
			gi.centerprintf(player,"The %s team got all three cubes!",ClientTeam(other));
			gi.sound(player, CHAN_RELIABLE, gi.soundindex("world/klaxon2.wav"), 1, ATTN_NORM, 0); //Gotta let them know.

			//Let everyone see that player got the cube.
//			gi.cprintf(player, PRINT_HIGH, "%s got the last cube for the %s team!\n",other->client->pers.netname,ClientTeam(other));

			if(player->client->pers.inventory[ITEMLIST_FLAG] && player->max_health+FLAG_CARRIER_REWARD<=MAX_LIFE && player->client->resp.team==other->client->resp.team /* so "give cube" will work better.*/)
			{
				// Why do I need to do it to both max_health counters?
				player->client->pers.max_health+=FLAG_CARRIER_REWARD;
				player->max_health+=FLAG_CARRIER_REWARD;

				gi.cprintf(player,PRINT_HIGH,"Your maximum health total is now %i.  (reward for holding flag)\n",player->client->pers.max_health);
			}

			player->client->pers.inventory[ITEMLIST_FLAG]=0; /* Flush all flags. */
		}

		// Give a flag to each team that exists.  (Although, they all should.)

		if(PlayersOnTeam(ROCK))
		{
			player=random_player_on_team(ROCK,NULL);

			gi.cprintf(player,PRINT_HIGH,"You have been awarded a flag.\n");
			player->client->pers.inventory[ITEMLIST_FLAG]=1;
		}

		if(PlayersOnTeam(PAPER))
		{
			player=random_player_on_team(PAPER,NULL);

			gi.cprintf(player,PRINT_HIGH,"You have been awarded a flag.\n");
			player->client->pers.inventory[ITEMLIST_FLAG]=1;
		}

		if(PlayersOnTeam(SCISSORS))
		{
			player=random_player_on_team(SCISSORS,NULL);

			gi.cprintf(player,PRINT_HIGH,"You have been awarded a flag.\n");
			player->client->pers.inventory[ITEMLIST_FLAG]=1;
		}
	}
//	else
		//Let everyone see that player got the cube.
//		gi.bprintf(PRINT_HIGH, "%s got a cube for the %s team!\n",other->client->pers.netname,ClientTeam(other));

	return true;
}

int FlagsOnGround()
{
	int num=0;
	edict_t *current=NULL;

	while(current=G_Find(current,FOFS(classname),"cube"))
		num++;

	return num;
}

int flagsInExistance()
{
	return FlagsOnGround()+FlagsOnTeam(ROCK)+FlagsOnTeam(PAPER)+FlagsOnTeam(SCISSORS);
}

int teamsInExistance()
{
	int howmany=0;

	if(PlayersOnTeam(ROCK)!=0)
		++howmany;
	if(PlayersOnTeam(SCISSORS)!=0)
		++howmany;
	if(PlayersOnTeam(PAPER)!=0)
		++howmany;

	return howmany;
}

qboolean Pickup_Flag (edict_t *ent, edict_t *other)
{
//	if(other->client && ent->spawnflags & DROPPED_PLAYER_ITEM)
//		FlagsAlive--;

	if(Add_Flag(other))
	{
		gi.bprintf(PRINT_HIGH, "%s got a cube for the %s team!\n",other->client->pers.netname,ClientTeam(other));
		return true;
	}
	else
		return false;
}

void ReturnFlag(edict_t *ent)
{
	edict_t *Player=NULL;
	int flags=flagsInExistance();
	int teams=teamsInExistance();

	if(flags<=teams)
	{
		if(PlayersOnTeam(OpposingTeam(ent->owner->client->resp.team)))
			Player=random_player_on_team(OpposingTeam(ent->owner->client->resp.team),NULL);
		else if(PlayersOnTeam(VictimTeam(ent->owner->client->resp.team)))
			Player=random_player_on_team(VictimTeam(ent->owner->client->resp.team),NULL);
		else if(ent->owner)
			Player=ent->owner;
		else if(PlayersOnTeam(ent->owner->client->resp.team))
			Player=random_player_on_team(ent->owner->client->resp.team,NULL);

		if(Player)
		{
			gi.bprintf(PRINT_HIGH,"A cube has been returned to %s of the %s team.\n",Player->client->pers.netname,ClientTeam(Player));
			Add_Flag(Player);
		}
	}

//	FlagsAlive--;
	G_FreeEdict(ent);
}

static void drop_make_touchable_flag (edict_t *ent)
{
	ent->touch = Touch_Item;
	ent->nextthink=level.time + 29;
	ent->think = ReturnFlag;
}

void Drop_Flag (edict_t *ent, gitem_t *item)
{
	float spread=rndnum(0,360); /* a random number  for where flag goes. */

	edict_t	*dropped;
	vec3_t	forward, right;
	vec3_t offset;

//	FlagsAlive++;

	ent->client->v_angle[YAW] -= spread; /* to alter where it get's tossed. */

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	gi.setmodel (dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;  
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	if (ent->client)
	{
		trace_t	trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right, dropped->s.origin);
		trace = gi.trace (ent->s.origin, dropped->mins, dropped->maxs,
			dropped->s.origin, ent, CONTENTS_SOLID);
		VectorCopy (trace.endpos, dropped->s.origin);
	}
	else
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
	}

	VectorScale (forward, 100, dropped->velocity);
	dropped->velocity[2] = 300;

	dropped->think = drop_make_touchable_flag;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

	ent->client->pers.inventory[ITEMLIST_FLAG]--;
	ValidateSelectedItem (ent);

	ent->client->v_angle[YAW] += spread; /* to alter back to start. */

	gi.bprintf(PRINT_HIGH, "%s dropped a cube!\n",ent->client->pers.netname);
}

team_t OpposingTeam(team_t Team)
{
	switch(Team)
	{
	case ROCK: return PAPER;
	case PAPER: return SCISSORS;
	case SCISSORS: return ROCK;
	default: return NONE;
	}
}

team_t VictimTeam(team_t Team)
{
	switch(Team)
	{
	case ROCK: return SCISSORS;
	case PAPER: return ROCK;
	case SCISSORS: return PAPER;
	default: return NONE;
	}
}

int PlayersOnTeam(team_t Team)
{
	edict_t *player;
	int i,count=0;

	for_each_player(player,i)
	{
		if(player->client->resp.team==Team)
			count++;
	}

	return count;
}

void DisplayGoal(edict_t *ent)
{
	if(teamsInExistance()>=3)
		gi.centerprintf(ent,"You are on the %s team (%s).\n\nAttack the %s team (%s).\nAvoid the %s team (%s).",
						ClientTeam(ent),Color(ent->client->resp.team),
						ClientVictimTeam(ent),Color(VictimTeam(ent->client->resp.team)),
						ClientOpposingTeam(ent),Color(OpposingTeam(ent->client->resp.team)));
	else
		gi.centerprintf(ent,"You are on the %s team (%s).\n\nLESS THAN THREE TEAMS!\nNormal teamplay deathmatch rules exist.",
						ClientTeam(ent),Color(ent->client->resp.team));
}

int PutClientOnTeam(edict_t *ent,team_t Team)
{
	int i,teams;
	edict_t *player;

	if(ent->health>0 && ent->client->resp.team!=NONE) // if alive,
	{
		while(ent->client->pers.inventory[ITEMLIST_FLAG])
			Drop_Flag(ent,GetItemByIndex(ITEMLIST_FLAG));

		TossClientWeapon(ent);

		ent->client->resp.score--; //Don't want to let people do this to avoid dying...

		ent->health = 0;
		ent->deadflag = DEAD_DEAD;
	}

	// check if # teams now is 2 and
	teams=teamsInExistance();

	ent->client->resp.team=Team;

	if(teams==2 && teamsInExistance()==3) // there are 3 teams now.
		for_each_player(player,i)
			if(player!=ent) // he/she will get goal when he is put in server.
				DisplayGoal(player);

	PutClientInServer(ent);

	return 0;
}

void RPSJoinMenu(edict_t *ent)
{
	if(ent->client->showmenu)
	{
		arena_link_t *menulink;

		ent->client->arenaID=0;

		menulink = CreateMenu(ent, "Join which team?");

		AddMenuItem(menulink, "Rock (red)      :", " people: ", PlayersOnTeam(ROCK), &JoinMenuResponse);
		AddMenuItem(menulink, "Paper (blue)    :", " people: ", PlayersOnTeam(PAPER), &JoinMenuResponse);
		AddMenuItem(menulink, "Scissors (green):", " people: ", PlayersOnTeam(SCISSORS), &JoinMenuResponse);

		FinishMenu(ent,menulink);
	}
	else
		SendStatusBar(ent, rps_statusbar);
}

int JoinMenuResponse(edict_t *ent, arena_link_t *menulink, arena_link_t *selected, int key)
{
	team_t team;

	switch(ent->client->arenaID)
	{
	case 0: team=ROCK;
			break;
	case 1: team=PAPER;
			break;
	case 2: team=SCISSORS;
			break;
	default: break;
	}

	gi.bprintf(PRINT_HIGH, "%s has joined the %s team!\n",ent->client->pers.netname,ClientTeam(ent));

	PutClientOnTeam(ent,team);

	return 0;
}

// blatently stolen from the qdevels code.  
edict_t *random_player_on_team (team_t Team, edict_t *notme)
{
	int i;
	int count;
	int random_player;
	edict_t *joe_bloggs;

	// count the number of players
	count = 0;
	for_each_player(joe_bloggs,i)
	{
		if (joe_bloggs != notme && joe_bloggs->client->resp.team==Team) // here's my big change.
			count++;
	}

	// no players ?
	if (count == 0)
	{
	//	gi.dprintf("ERROR: tried to select a random player when none are available.\n");
		return NULL;
	}
	
	// select a random player
	random_player = rand() % count;

	// find the randomly selected player
	count = 0;
	for_each_player(joe_bloggs,i)
	{
		if (joe_bloggs != notme && joe_bloggs->client->resp.team==Team) // here's my big change.
		{
			if (count == random_player)
				return joe_bloggs;
			else
				count++;
		}
	}
}

// Used for autoteams.  (Could be semi-easily adapted to beyond just the worst, but I'm tired.)
team_t LowestTeam()
{
	int rplayers=PlayersOnTeam(ROCK),pplayers=PlayersOnTeam(PAPER),splayers=PlayersOnTeam(SCISSORS);

	if(rplayers==pplayers && rplayers==splayers) // if all teams have same number of players.
	{
		// points advantage
		if(RockScore<PaperScore && RockScore<ScissorsScore)
			return ROCK;
		else if(PaperScore<RockScore && PaperScore<ScissorsScore)
			return PAPER;
		else if(ScissorsScore<RockScore && ScissorsScore<PaperScore)
			return SCISSORS;
	}
	else
	{
		if(rplayers<=pplayers && rplayers<=splayers)
			return ROCK;
		else if(pplayers<=rplayers && pplayers<=splayers)
			return PAPER;
		else if(splayers<=rplayers && splayers<=pplayers)
			return SCISSORS;
	}

	//even players, and no score advantage, so just put them on rock.
	return ROCK;  
}
