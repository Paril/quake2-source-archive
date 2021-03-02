#include "g_local.h"

teamplay_t *teamplay_main;

void StripLine (char *wholestring, int lineno, char *output)
{
	int i = 0, n = 0;
	int curline = 0;
	char *retval = gi.TagMalloc(strlen(wholestring), TAG_TEMP);
	int saved_time = 0;

	// Now, we need to find a number of lines the same as lineno.
	// If it's 0, then we take that line.
//	for (; i < strlen(wholestring); i++)
//	{
up:
		if (curline == lineno)
		{
			if (saved_time)
			{
				// Copy into retval;
				for (n = saved_time; n < strlen(wholestring); n++, i++)
				{
					char curch = wholestring[n];
					if (curch != '\n')
						retval[i] = curch;
					else
						break;
				}
			}
			else
			{
				// Copy into retval;
				for (; n < strlen(wholestring); n++)
				{
					char curch = wholestring[n];
					if (curch != '\n')
						retval[n] = curch;
					else
						break;
				}
			}
		}
		else
		{
			// Copy into retval;
			for (; n < strlen(wholestring); n++)
			{
				char curch = wholestring[n];

				if (curch == '\n')
					curline++;
				// Copy string location
				else if (curline == lineno)
				{
					saved_time = n;
					goto up;
				}
			}
		}
//	}

	strcpy (output, retval);
}

void CopyFileToString (FILE *fp, char *pass)
{
	unsigned int fileSize;
	int l = 0;
//	char pass32[200];

	// get file size
	fseek(fp,0,SEEK_END);
	fileSize = ftell(fp);
	fseek(fp,0,SEEK_SET);

	for (; l < fileSize; l++)
	{
		char lcha = fgetc(fp);

		if (lcha == -1 || lcha == 0)
			break;

		pass[l] = lcha;
	}
	pass[l] = 0;

	// Strip the first line, put it in pass32.
	//gi.dprintf ("%s\n", pass);

	//gi.dprintf ("%s\n", pass32);
}

#define COMMAND_NAME			1
#define COMMAND_NEXT			2
#define COMMAND_BAN				3

int TypeOfCommand (char *cmd)
{
	if (Q_stricmp(cmd, "name") == 0)
		return COMMAND_NAME;
	else if (Q_stricmp(cmd, "\0") == 0)
		return COMMAND_NEXT;
	else if (Q_stricmp(cmd, "blasterban") == 0 || Q_stricmp(cmd, "shotgunban") == 0 || Q_stricmp(cmd, "machinegunban") == 0 || Q_stricmp(cmd, "chaingunban") == 0 || Q_stricmp(cmd, "grenadelauncherban") == 0 || Q_stricmp(cmd, "rocketlauncherban") == 0 || Q_stricmp(cmd, "hyperblasterban") == 0 || Q_stricmp(cmd, "railgunban") == 0 || Q_stricmp(cmd, "bfgban") == 0 || Q_stricmp(cmd, "grenadeban") == 0)
		return COMMAND_BAN;
	else
		return -1;
}

int GetBanType (char *command)
{
	char *str[] = {"blasterban", "shotgunban", "supershotgunban", "machinegunban", "chaingunban", "grenadelauncherban", "rocketlauncherban", "hyperblasterban", "railgunban", "bfgban", "grenadeban"};
	int i = 0;
	
	for (; i < 11; i++)
	{
		if (Q_stricmp(command, str[i]) == 0)
			return i+1;
	}
	return 0;
}

// Type: 0 if gamemode description
//       1 if team. Add teamnumber to type.
//       Example: ParseLine (line, 2); is team 1.
int EndOfFile = 0;

int curteam = -1;
// Function gets the command from the line and uses it to fill in the teams struct.
int GetCommandFromLine (char *line, int type)
{
	int i = 0;
	char command[24];
	int toc = -1;

	for (; i < strlen(line); i++)
	{
		if (line[i] == ' ')
		{
			command[i] = '\0';
			break; 
		}
		else if (line[i] == '\0')
			return -1;
		else
			command[i] = line[i];
	}

	toc = TypeOfCommand (command);
	if (toc == COMMAND_NAME)
	{
		if (curteam >= 0)
			strcpy(teamplay_main->teams[curteam]->teamname, line+5);
		else
			strcpy(teamplay_main->gamename, line+5);
	}
	else if (toc == COMMAND_BAN)
	{
		int weap = 0;
		char tmp[68];
		int cpy = 0;

		weap = GetBanType (command);

		strcpy (tmp, line+strlen(command)+1);
		cpy = atoi (tmp);

		if (weap == 1)
			teamplay_main->teams[curteam]->blaster_ban = cpy;
		if (weap == 2)
			teamplay_main->teams[curteam]->shotgun_ban = cpy;
		if (weap == 3)
			teamplay_main->teams[curteam]->supershotgun_ban = cpy;
		if (weap == 4)
			teamplay_main->teams[curteam]->machinegun_ban = cpy;
		if (weap == 5)
			teamplay_main->teams[curteam]->chaingun_ban = cpy;
		if (weap == 6)
			teamplay_main->teams[curteam]->grenadelauncher_ban = cpy;
		if (weap == 7)
			teamplay_main->teams[curteam]->rocketlauncher_ban = cpy;
		if (weap == 8)
			teamplay_main->teams[curteam]->hyperblaster_ban = cpy;
		if (weap == 9)
			teamplay_main->teams[curteam]->railgun_ban = cpy;
		if (weap == 10)
			teamplay_main->teams[curteam]->bfg_ban = cpy;
		if (weap == 11)
			teamplay_main->teams[curteam]->grenade_ban = cpy;
	}
	else if (toc == COMMAND_NEXT)
	{
		if (line[strlen(line)] == '\0')
			EndOfFile = 1;
		else
			curteam++;
	}

	return 1;
}

void InitTeams ()
{
	FILE *fp;
	char lch[12000];
	char pass32[120];
	int curline = 0;
	int l = 0;
	teamplay_t *temp = (teamplay_t *)gi.TagMalloc (sizeof(teamplay_t), TAG_GAME);
	memset (temp, 0, sizeof(temp));
	teamplay_main = temp;

	// Set up each team.
	for (; l < 8; l++)
	{
		team_t *tmp = (team_t *)gi.TagMalloc (sizeof(team_t), TAG_GAME);
		memset (tmp, 0, sizeof(tmp));
		teamplay_main->teams[l] = tmp;
	}

	fp = fopen ("iwm/teams.ini", "r");
	if (!fp)
		return;

	CopyFileToString (fp, lch);

	fclose(fp);

	// Go through each line, process their commands.
	while (!EndOfFile)
	{
		int j;
		StripLine(lch, curline, pass32);
		j = GetCommandFromLine(pass32, 0);
		if (pass32[0] == '\0')
			break;
		if (pass32[0] == '#')
			curteam++;

		curline++;
	}
	gi.FreeTags (TAG_TEMP);
}

void JoinATeam (edict_t *ent, pmenuhnd_t *p);

pmenu_t teammenu[] = {
	{ "*Quake II",						PMENU_ALIGN_CENTER, NULL },
	{ "*Insane Weapons Mod",	PMENU_ALIGN_CENTER, NULL },
	{ "*Teamplay",								PMENU_ALIGN_CENTER, NULL },
	{ NULL,								PMENU_ALIGN_CENTER,  },
	{ NULL,						PMENU_ALIGN_LEFT, NULL },
	{ "Text for the team goes here.",					PMENU_ALIGN_LEFT, JoinATeam },
	{ "Text for the team goes here.",								PMENU_ALIGN_LEFT, JoinATeam },
	{ "Text for the team goes here.",								PMENU_ALIGN_LEFT, JoinATeam },
	{ "Text for the team goes here.",								PMENU_ALIGN_LEFT, JoinATeam },
	{ "Text for the team goes here.",								PMENU_ALIGN_LEFT, JoinATeam },
	{ "Text for the team goes here.",								PMENU_ALIGN_LEFT, JoinATeam },
	{ "Text for the team goes here.",								PMENU_ALIGN_LEFT, JoinATeam },
	{ "Text for the team goes here.",								PMENU_ALIGN_LEFT, JoinATeam },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ NULL,								PMENU_ALIGN_LEFT, NULL },
	{ "Use [ and ] by default",					PMENU_ALIGN_LEFT, NULL },
	{ "to navigate, and enter to",					PMENU_ALIGN_LEFT, NULL },
	{ "select an option.",					PMENU_ALIGN_LEFT, NULL },
};

int HowManyOnTeam (int teamnum)
{
	int i, count = 0;
	edict_t *fnd;

	for (i = 0; i < game.maxclients; i++)
	{
		fnd = &g_edicts[i];

		if (fnd->inuse && fnd->client)
		{
			if (fnd->client->resp.team != -1 && fnd->client->resp.team == teamnum)
				count++;
		}
	}

	return count;
}

int OnSameIWMTeam (edict_t *ent, edict_t *othah)
{
	if (iwm_gamemode->value != 1)
		return 0;

	if (!ent->client || !othah->client)
		return 0;

	if (ent->client->resp.team == othah->client->resp.team)
		return 1;
	return 0;
}

void JoinATeam (edict_t *ent, pmenuhnd_t *p)
{
	int joined_team = p->cur-5;

	ent->client->resp.team = joined_team;

	ent->movetype = MOVETYPE_WALK;
	ent->solid = SOLID_BBOX;
	ent->svflags &= ~SVF_NOCLIENT;
	gi.linkentity (ent);
	PutClientInServer(ent);
	safe_bprintf (PRINT_HIGH, "%s joined team %s!\n", ent->client->pers.netname, teamplay_main->teams[joined_team]->teamname);
}

typedef char chr50[50];

void OpenTeamMenu (edict_t *ent, pmenuhnd_t *p)
{
	chr50 string[8];
	int i = 5;

	memcpy (ent->client->menuc, teammenu, sizeof(teammenu));

	for (; i < MAX_TEAMS+5; i++)
	{
		if (Q_stricmp(teamplay_main->teams[i-5]->teamname, ""))
		{
			sprintf (string[i-5], "%s - Players: %i", teamplay_main->teams[i-5]->teamname, HowManyOnTeam(i-5));
			ent->client->menuc[i].text = string[i-5];
		}
		else
		{
			sprintf (string[i-5], "");
			ent->client->menuc[i].SelectFunc = NULL;
			ent->client->menuc[i].text = string[i-5];
		}
	}

	PMenu_Close(ent);
	PMenu_Open(ent, ent->client->menuc, -1, sizeof(ent->client->menuc) / sizeof(pmenu_t), NULL);
}