
#include "g_local.h"


void	Svcmd_Test_f (void)
{
	gi.cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

// SH
char *Green1(char *in);
char *Green2(char *in);

void clearbans ()
{
	int i=0;
	for (i=0;i<MAX_ACTIVE;i++)
	{
		game.act_ban[i] = 0;
	}
	for (i=0;i<MAX_PASSIVE;i++)
	{
		game.pas_ban[i] = 0;
	}
	for (i=0;i<MAX_SPECIAL;i++)
	{
		game.spe_ban[i] = 0;
	}
	for (i=0;i<MAX_COMBO;i++)
	{
		game.com_ban[i] = 0;
	}
}

void readadmin()
{
	FILE	*f;
//	char * filename;
	char rb[500];
	char c=0;
	int n=0,q=0,i=0;

	if (f = fopen("admin.sh2", "r"))
	{
		fgets(rb, 500, f);

		for (i=0;i<20;i++)
		{
			admin.password[i] = rb[i];
			if (admin.password[i] < 33)
				admin.password[i] = 0;
		}

		fclose(f);
	}
	else
		gi.bprintf (PRINT_HIGH, "No \"admin.sh2\" found.\n");
}

void readqueue()
{
	FILE	*f;
//	char * filename;
	char rb[500];
	char c=0;
	int n=0,q=0,i=0;

	if (f = fopen("queue.sh2", "r"))
	{
		while (fgets(rb, 20, f) && c < MAX_LEVEL_QUEUE)
		{
			while (rb[i] >= 33 && i < 20)
			{
				admin.queue[c][i] = rb[i];
				i++;
			}
			admin.queue[c][i+1] = 0;
			c++;
			i=0;
		}
		admin.curlevel = 0;
		fclose(f);
	}
	else
		gi.bprintf (PRINT_HIGH, "No \"queue.sh2\" found.\n");
}

void readbans()
{
	FILE	*f;
//	char * filename;
	char bans[500];
	char c=0;
	int n=0,q=0,i=0;

	clearbans();
	
	if (f = fopen("banned.sh2", "r"))
	{
		while (fgets(bans, 500, f))
		{
			while (bans[c] != 0)
			{
				if (bans[c] == '/' && bans[c+1] == '/')
				{
					break;
				}
				
				if (bans[c] == 'a' || bans[c] == 'A')
				{
					n = 1;
					q = 0;
				}
				else if (bans[c] == 'p' || bans[c] == 'P')
				{
					n = 2;
					q = 0;
				}
				else if (bans[c] == 's' || bans[c] == 'S')
				{
					n = 3;
					q = 0;
				}
				else if (bans[c] == 'c' || bans[c] == 'C')
				{
					n = 4;
					q = 0;
				}
				else if (bans[c] >= '0' && bans[c] <= '9')
				{
					q *= 10;
					q += bans[c] - '0';
				}
				else// if (bans[c] == ',')
				{
					if (q > 0)
					{
						if (n == 1)
							game.act_ban[q] = 1;
						else if (n == 2)
							game.pas_ban[q] = 1;
						else if (n == 3)
							game.spe_ban[q] = 1;
						else if (n == 4)
							game.com_ban[q] = 1;
						q = 0;
					}
				}
				c++;
			}
			c = 0;
			q = 0;
			gi.bprintf (PRINT_HIGH, "%s", bans);
		}

		fclose(f);
	}
}

void showbans ()
{
	int i=0;

	gi.dprintf ("Banned:");

	gi.dprintf ("\nActive : ");
	for (i=0;i<MAX_ACTIVE;i++)
	{
		if (game.act_ban[i] == 1)
		gi.dprintf ("%i ", i);
	}
	
	gi.dprintf ("\nPassive: ");
	for (i=0;i<MAX_PASSIVE;i++)
	{
		if (game.pas_ban[i] == 1)
		gi.dprintf ("%i ", i);
	}
	
	gi.dprintf ("\nSpecial: ");
	for (i=0;i<MAX_SPECIAL;i++)
	{
		if (game.spe_ban[i] == 1)
		gi.dprintf ("%i ", i);
	}
	
	gi.dprintf ("\nCombo  : ");
	for (i=0;i<MAX_COMBO;i++)
	{
		if (game.com_ban[i] == 1)
		gi.dprintf ("%i ", i);
	}
}

void MakeAllObs ()
{
	int i=0;
	edict_t	*player;


	for (i = 1; i <= maxclients->value; i++) 
	{
		player = &g_edicts[i];
		if (!player->inuse || player->flags & FL_OBSERVER)
			continue;

		MakeObserver (player);
	}

}

// \SH

/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
void	ServerCommand (void)
{
	char	*cmd1, *cmd2, *cmd3;
	int i = 0;

	cmd1 = gi.argv(1);
	cmd2 = gi.argv(2);
	cmd3 = gi.argv(3);

	if (Q_stricmp (cmd1, "test") == 0)
		Svcmd_Test_f ();
// SH	
	else if (Q_stricmp (cmd1, "ban") == 0)
	{
		if (Q_stricmp (cmd2, "clear") == 0)
		{
			clearbans();
		}
		else if (Q_stricmp (cmd2, "reload") == 0)
		{
			readbans();
		}
		else if (Q_stricmp (cmd2, "show") == 0)
		{
			showbans();
		}
		else if (Q_stricmp (cmd2, "a") == 0)
		{
			i = atoi (cmd3);

			if (i > 0 && i <= MAX_ACTIVE)
				game.act_ban[i] = !game.act_ban[i];
		}
		else if (Q_stricmp (cmd2, "p") == 0)
		{
			i = atoi (cmd3);

			if (i > 0 && i <= MAX_PASSIVE)
				game.pas_ban[i] = !game.pas_ban[i];
		}
		else if (Q_stricmp (cmd2, "s") == 0)
		{
			i = atoi (cmd3);

			if (i > 0 && i <= MAX_SPECIAL)
				game.spe_ban[i] = !game.spe_ban[i];
		}
		else if (Q_stricmp (cmd2, "c") == 0)
		{
			i = atoi (cmd3);

			if (i > 0 && i <= MAX_COMBO)
				game.com_ban[i] = !game.com_ban[i];
		}
	}
	else if (Q_stricmp (cmd1, "forceobs") == 0)
	{
		MakeAllObs();
	}
// \SH
	else
		gi.cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd1);
}

////////////////////////////////////////////////////// SH CTF
int CTFTeamCount (int team, edict_t *ignore)
{
	int i=0,count=0;
	edict_t *player = NULL;
	
	for (i = 1; i <= maxclients->value; i++) 
	{
		player = &g_edicts[i];

		if (!player->inuse || player == ignore)
			continue;

		if (player->flags & team)
			count++;
	}

	return count;

}

void CTFTripSkin (edict_t * ent)
{
	if (ent->flags & FL_REDTEAM)
	{
		ent->s.skinnum = 1;
	}
	else if (ent->flags & FL_BLUETEAM)
	{
		ent->s.skinnum = 2;
	}
	else
	{
		ent->s.skinnum = 0;
	}
}

void CTFAssignSkin(edict_t *ent, char *s)
{
	int playernum = ent-g_edicts-1;
	char *p;
	char t[64];

	Com_sprintf(t, sizeof(t), "%s", s);

	if ((p = strrchr(t, '/')) != NULL)
		p[1] = 0;
	else
		strcpy(t, "male/");

	if (ent->flags & FL_REDTEAM)
	{
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", 
			ent->client->pers.netname, t, CTF_TEAM1_SKIN) );
	}
	else if (ent->flags & FL_BLUETEAM)
	{
		gi.configstring (CS_PLAYERSKINS+playernum,
			va("%s\\%s%s", ent->client->pers.netname, t, CTF_TEAM2_SKIN) );
	}
	else
	{
		gi.configstring (CS_PLAYERSKINS+playernum, 
			va("%s\\%s", ent->client->pers.netname, s) );
	}

	if (ent->client->pers.combo == C_ROBOT || ent->client->pers.combo == C_CRIP)
	{
		CTFTripSkin (ent);
	}
//	gi.cprintf(ent, PRINT_HIGH, "You have been assigned to %s team.\n", ent->client->pers.netname);
}

void CTFSetTeam (edict_t *ent)
{
	int team1count=0, team2count=0;
//	edict_t *player;
//	int i=0;

/*
	for (i = 1; i <= maxclients->value; i++) {
		player = &g_edicts[i];

		if (!player->inuse || player->client == who)
			continue;

		if (player->flags & FL_REDTEAM)
			team1count++;
		else if (player->flags & FL_BLUETEAM)
			team2count++;
	}
*/
	team1count = CTFTeamCount (FL_REDTEAM, NULL);
	team2count = CTFTeamCount (FL_BLUETEAM, NULL);

	if (team1count < team2count)
		ent->flags |= FL_REDTEAM;
	else if (team2count < team1count)
		ent->flags |= FL_BLUETEAM;
	else if (rand() & 1)
		ent->flags |= FL_REDTEAM;
	else
		ent->flags |= FL_BLUETEAM;

	gi.cprintf (ent, PRINT_HIGH, "*** You have been assigned to the %s Team!\n", (ent->flags & FL_REDTEAM?Green1("Good\0"):Green1("Evil\0")));
}

int CTFTeam_Flag (edict_t *flag, edict_t *ent)
{
	if (ent->flags & flag->style)
	{		
//gi.cprintf (ent, PRINT_HIGH, "same team\n");
		return true;
	}

//gi.cprintf (ent, PRINT_HIGH, "diff team\n");
	return false;
}		

void CTFResetFlag(int ctf_team)
{
	char *c;
	edict_t *ent;

	switch (ctf_team) {
	case FL_REDTEAM:
		c = "item_flag_team1";
		break;
	case FL_BLUETEAM:
		c = "item_flag_team2";
		break;
	default:
		return;
	}

//gi.bprintf (PRINT_HIGH, "trying to reset %s\n", c);

	ent = NULL;
	while ((ent = G_Find (ent, FOFS(classname), c)) != NULL) {
		if (ent->spawnflags & DROPPED_ITEM)
			G_FreeEdict(ent);
		else {
			ent->svflags &= ~SVF_NOCLIENT;
			ent->solid = SOLID_TRIGGER;
			gi.linkentity(ent);
			ent->s.event = EV_ITEM_RESPAWN;
		}
//gi.bprintf (PRINT_HIGH, "flag reset\n");
	}
}

void CTFResetFlags ()
{
	CTFResetFlag (FL_REDTEAM);
	CTFResetFlag (FL_BLUETEAM);
}

int CTFSameTeam (edict_t *ent1, edict_t *ent2)
{
	if (ctf->value == 0)
		return false;

	if (ent1->flags & FL_REDTEAM && ent2->flags & FL_REDTEAM)
		return true;
	if (ent1->flags & FL_BLUETEAM && ent2->flags & FL_BLUETEAM)
		return true;

	return false;
}

int CTFTeam (edict_t *ent)
{
	if (ent->flags & FL_REDTEAM)
		return FL_REDTEAM;
	else if (ent->flags & FL_BLUETEAM)
		return FL_BLUETEAM;
	else if (!(ent->flags & FL_OBSERVER))
	{
		gi.bprintf (PRINT_HIGH, "Player %s without a team\n", Green1(ent->client->pers.netname));
		return 0;
	}
}

qboolean CTFPickup_Flag (edict_t *ent, edict_t *other)
{
	int teamflag, oppflag;
	char teamname[10]={0};
	char oppname[10]={0};

	if (other->flags & FL_REDTEAM)
	{
		teamflag = FL_REDTEAM;
		oppflag = FL_BLUETEAM;
		strcpy(teamname,"Good");
		strcpy(oppname,"Evil");
	}
	else
	{
		teamflag = FL_BLUETEAM;
		oppflag = FL_REDTEAM;
		strcpy(teamname,"Evil");
		strcpy(oppname,"Good");
	}

	if (ent->spawnflags & DROPPED_ITEM)
	{
		if (CTFTeam_Flag (ent, other)) 
		{
			// returning own flag
			other->client->resp.score += CTF_RECOVERY_BONUS;
			gi.bprintf (PRINT_HIGH, "*** %s flag returned by %s\n", Green1(teamname), Green2(other->client->pers.netname));
			
			if (teamflag == FL_REDTEAM)
				gi.sound(other, CHAN_ITEM, gi.soundindex("ctf/goodret.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(other, CHAN_ITEM, gi.soundindex("ctf/evilret.wav"), 1, ATTN_NORM, 0);

			CTFResetFlag(teamflag);
			return false;
		}
		else
		{
			// picking up enemy flag
			other->flags |= FL_HASFLAG;
//			G_FreeEdict (ent);
			gi.bprintf (PRINT_HIGH, "*** %s flag picked up by %s\n", Green1(oppname), Green2(other->client->pers.netname));

			if (teamflag == FL_REDTEAM)
				gi.sound(other, CHAN_ITEM, gi.soundindex("ctf/goodpick.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound(other, CHAN_ITEM, gi.soundindex("ctf/evilpick.wav"), 1, ATTN_NORM, 0);
			
			return true;
		}
	}
	else
	{
		if (CTFTeam_Flag (ent, other))
		{
			if (other->flags & FL_HASFLAG)
			{
				// flag capture!
				edict_t *player;
				int i = 0;
				int teamcount = CTFTeamCount (CTFTeam (other), NULL);
				int totalplayers = CTFTeamCount (FL_REDTEAM, NULL) + CTFTeamCount (FL_BLUETEAM, NULL);

//				if (teamcount == 0)
//					teamcount = 1;

				other->client->resp.score += CTF_CAPTURE_BONUS*(ceil(totalplayers/2));

				for (i = 1; i <= maxclients->value; i++) {
					player = &g_edicts[i];
					if (!player->inuse || player->flags & FL_OBSERVER)
						continue;

					if (CTFSameTeam (player, other))
					{
//						if (player != other)
							player->client->resp.score += (CTF_TEAM_BONUS)*(ceil((totalplayers/teamcount)));
					}
				}
				other->flags &= ~FL_HASFLAG;
				CTFResetFlag (oppflag);

				if (teamflag == FL_REDTEAM)
					gi.sound(other, CHAN_ITEM, gi.soundindex("ctf/goodcap.wav"), 1, ATTN_NONE, 0);
				else
					gi.sound(other, CHAN_ITEM, gi.soundindex("ctf/evilcap.wav"), 1, ATTN_NONE, 0);
	
				gi.bprintf (PRINT_HIGH, "*** %s flag captured by %s\n", Green1(oppname), Green2(other->client->pers.netname));

			}
			return false;
		}
		else
		{
			// stealing flag
			ent->svflags |= SVF_NOCLIENT;
			ent->solid = SOLID_NOT;
			ent->flags |= FL_RESPAWN;
			gi.linkentity (ent);
			other->flags |= FL_HASFLAG;
			gi.bprintf (PRINT_HIGH, "*** %s flag stolen by %s\n", Green1(oppname), Green2(other->client->pers.netname));

			if (teamflag == FL_REDTEAM)
				gi.sound(other, CHAN_ITEM, gi.soundindex("ctf/goodstel.wav"), 1, ATTN_NONE, 0);
			else
				gi.sound(other, CHAN_ITEM, gi.soundindex("ctf/evilstel.wav"), 1, ATTN_NONE, 0);
			
			return true;
		}
	}
}

void CTFDropFlagTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	//owner (who dropped us) can't touch for two secs
	if (other == ent->owner && 
		ent->nextthink - level.time > CTF_FLAG_RETURN_TIME - 2)
		return;

	Touch_Item (ent, other, plane, surf);
}

void CTFDropFlagThink(edict_t *ent)
{
	// auto return the flag
	// reset flag will remove ourselves
	if (strcmp(ent->classname, "item_flag_team1") == 0) {
		CTFResetFlag(FL_REDTEAM);
		gi.bprintf (PRINT_HIGH, "*** %s Flag returned by timer.\n", Green1("Good\0"));
//		gi.bprintf(PRINT_HIGH, "The %s flag has returned!\n",
//			CTFTeamName(CTF_TEAM1));
	} else if (strcmp(ent->classname, "item_flag_team2") == 0) {
		CTFResetFlag(FL_BLUETEAM);
		gi.bprintf (PRINT_HIGH, "*** %s Flag returned by timer.\n", Green1("Evil\0"));
//		gi.bprintf(PRINT_HIGH, "The %s flag has returned!\n",
//			CTFTeamName(CTF_TEAM2));
	}
}

void CTFDrop_Flag (edict_t *ent)
{
	edict_t *flag;
	char *name;
	char *model;
	vec3_t forward, right, offset;

	if (!(ent->flags & FL_HASFLAG))
		return;

	flag = G_Spawn();

	if (ent->flags & FL_REDTEAM)
		flag->item = FindItemByClassname("item_flag_team1");
	else if (ent->flags & FL_BLUETEAM)
		flag->item = FindItemByClassname("item_flag_team2");
	else
		return;

	if (ent->flags & FL_REDTEAM)
		flag->style = FL_BLUETEAM;
	else
		flag->style = FL_REDTEAM;

	flag->classname = name;
	flag->spawnflags = DROPPED_ITEM;
	flag->s.frame = 198;
//	flag->s.renderfx = RF_GLOW;
//	flag->s.effects = flag->style;
	VectorSet (flag->mins, -15, -15, -15);
	VectorSet (flag->maxs, 15, 15, 15);
	gi.setmodel (flag, model);
	flag->solid = SOLID_TRIGGER;
	flag->movetype = MOVETYPE_TOSS;  
	flag->touch = CTFDropFlagTouch;
	flag->owner = ent;

	if (ent->client)
	{
		trace_t	trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right, flag->s.origin);
		trace = gi.trace (ent->s.origin, flag->mins, flag->maxs,
			flag->s.origin, ent, CONTENTS_SOLID);
		VectorCopy (trace.endpos, flag->s.origin);
	}
	else
	{
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, flag->s.origin);
	}

	VectorScale (forward, 100, flag->velocity);
	flag->velocity[2] = 300;

	flag->think = CTFDropFlagThink;
	flag->nextthink = level.time + CTF_FLAG_RETURN_TIME;

	gi.linkentity (flag);

	ent->flags &= ~FL_HASFLAG;
	ent->s.modelindex3 = 0;	// remove linked ctf flag
}

void CTFFlagThink (edict_t *ent)
{
	ent->nextthink = level.time + FRAMETIME;
}

void CTFFlagSetup (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15,-15,-15);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else
		gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;
	ent->s.frame = 198;

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		gi.dprintf ("CTFFlagSetup: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (tr.endpos, ent->s.origin);

	if (ent->s.effects & EF_FLAG1)
	{
		ent->style = FL_REDTEAM;
	}
	else if (ent->s.effects & EF_FLAG2)
	{
		ent->style = FL_BLUETEAM;
	}

	gi.linkentity (ent);

	ent->nextthink = level.time + FRAMETIME;
	ent->think = CTFFlagThink;
}

int CTFTeamScore (int team)
{
	int i=0, c=0;
	edict_t *player = NULL;

	for (i = 1; i <= maxclients->value; i++) 
	{
		player = &g_edicts[i];
		if (!player->inuse)
			continue;

		if (player->flags & team) 
		{
			c += player->client->resp.score;
//gi.bprintf (PRINT_HIGH, "%i.%i\n", i, c);
		}
	}
	return c;
}

int CTFFlagStat (int team)
{
	int p1=0, oppteam, base, drop, taken;
	char *flagname;
	edict_t *e;

	if (team == FL_REDTEAM)
	{
		oppteam = FL_BLUETEAM;
		base = gi.imageindex ("ctf/i_ctf1");
		drop = gi.imageindex ("ctf/i_ctf1d"); // must be dropped
		taken = p1 = gi.imageindex ("ctf/i_ctf1t");
		flagname = "item_flag_team1";
	}
	else if (team == FL_BLUETEAM)
	{
		oppteam = FL_REDTEAM;
		base = gi.imageindex ("ctf/i_ctf2");
		drop = gi.imageindex ("ctf/i_ctf2d"); // must be dropped
		taken = p1 = gi.imageindex ("ctf/i_ctf2t");
		flagname = "item_flag_team2";
	}
	else
		return 0;

	p1 = base;
	e = G_Find(NULL, FOFS(classname), flagname);
	if (e != NULL) 
	{
		if (e->solid == SOLID_NOT) 
		{
			int i;

			// not at base
			// check if on player
			p1 = drop; // default to dropped
			for (i = 1; i <= maxclients->value; i++)
			{
				if (g_edicts[i].inuse &&
					g_edicts[i].flags & FL_HASFLAG && 
					g_edicts[i].flags & oppteam) 
				{
					// enemy has it
					p1 = taken;
					break;
				}
			}
		} 
		else if (e->spawnflags & DROPPED_ITEM)
		{
			p1 = drop; // must be dropped
		}
	}

	return p1;
}

void CTFAnnounceScore (int done)
{
	int redscore=0,bluescore=0,winscore=0,losescore=0;
	char good[20] = {0};
	char evil[20] = {0};

	strcpy(good,"Forces of Good");
	strcpy(evil,"Minions of Evil");

	winscore = redscore = CTFTeamScore (FL_REDTEAM);
	losescore = bluescore = CTFTeamScore (FL_BLUETEAM);

	gi.bprintf (PRINT_HIGH, "*****\n");
	if (redscore > bluescore && !done)
	{
		gi.bprintf (PRINT_HIGH, "The %s are crushing the %s\n", Green1(good), Green2(evil));
		winscore = redscore;
		losescore = bluescore;
	}
	else if (bluescore > redscore && !done)
	{
		gi.bprintf (PRINT_HIGH, "The %s are crushing the %s\n", Green1(evil),Green2(good));
		winscore = bluescore;
		losescore = redscore;
	}
	else if (bluescore == redscore && !done)
	{
		gi.bprintf (PRINT_HIGH, "The battle is %s!\n", Green1("tied\0"));
	}
	else if (redscore > bluescore && done)
	{
		gi.bprintf (PRINT_HIGH, "%s\n", Green1("The Forces of Good have defeated the Minions of Evil"));
		winscore = redscore;
		losescore = bluescore;
	}
	else if (bluescore > redscore && done)
	{
		gi.bprintf (PRINT_HIGH, "%s\n", Green1("The Minions of Evil have defeated the Forces of Good"));
		winscore = bluescore;
		losescore = redscore;
	}
	else if (bluescore == redscore && done)
	{
		gi.bprintf (PRINT_HIGH, "%s\n", Green1("The battle results in a draw!\0"));
	}
	gi.bprintf (PRINT_HIGH, "Score: %i to %i\n*****\n", winscore, losescore);
}

/*QUAKED info_player_team1 (1 0 0) (-16 -16 -24) (16 16 32)
potential team1 spawning position for ctf games
*/
void SP_info_player_team1(edict_t *self)
{
}

/*QUAKED info_player_team2 (0 0 1) (-16 -16 -24) (16 16 32)
potential team2 spawning position for ctf games
*/
void SP_info_player_team2(edict_t *self)
{
}

edict_t *SelectRandomDeathmatchSpawnPoint (void);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);
float	PlayersRangeFromSpot (edict_t *spot);

/*
================
SelectCTFSpawnPoint

go to a ctf point, but NOT the two points closest
to other players
================
*/
edict_t *SelectCTFSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	char	*cname;

	switch (CTFTeam (ent)) {
	case FL_REDTEAM:
		cname = "info_player_team1";
		break;
	case FL_BLUETEAM:
		cname = "info_player_team2";
		break;
	default:
		return SelectRandomDeathmatchSpawnPoint();
	}

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), cname)) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return SelectRandomDeathmatchSpawnPoint();

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), cname);
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

int CTFCheck_Flag (int ctf_team)
{
	char *c;
	edict_t *ent, *player;
	int opp_team, i;

	switch (ctf_team) {
	case FL_REDTEAM:
		c = "item_flag_team1";
		opp_team = FL_BLUETEAM;
		break;
	case FL_BLUETEAM:
		c = "item_flag_team2";
		opp_team = FL_REDTEAM;
		break;
	default:
		gi.bprintf (PRINT_HIGH, "+++ Illegal flag check\n");
		return false;
	}

	for (i = 1; i <= maxclients->value; i++) 
	{
		player = &g_edicts[i];
		if (!player->inuse || player->flags & FL_OBSERVER)
			continue;

		if (player->flags & opp_team && player->flags & FL_HASFLAG)
		{
			gi.bprintf (PRINT_HIGH, "*** %s has the %s flag!\n", player->client->pers.netname, (ctf_team==FL_REDTEAM?"Good":"Evil"));
			return true;
		}
	}

	ent = NULL;
	while ((ent = G_Find (ent, FOFS(classname), c)) != NULL) 
	{
		if (ent->spawnflags & DROPPED_ITEM)
		{
			gi.bprintf (PRINT_HIGH, "*** %s flag is lying free!\n",(ctf_team==FL_REDTEAM?"Good":"Evil"));
			return true;
		}
		else if (!(ent->svflags & SVF_NOCLIENT))
		{
//			gi.bprintf (PRINT_HIGH, "*** %s flag is at base.\n",(ctf_team==FL_REDTEAM?"Good":"Evil"));
			return true;
		}
	}

	CTFResetFlag (ctf_team);
	gi.bprintf (PRINT_HIGH, "+++ Unexpected reset of %s flag.\n", (ctf_team==FL_REDTEAM?"Good":"Evil"));
	return false;
}