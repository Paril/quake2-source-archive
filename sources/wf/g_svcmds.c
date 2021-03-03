
#include "g_local.h"
#include "bot_procs.h"//ERASER
char *getClassName (gclient_t *c);
//WF34 S
void Cmd_Classdef_f (); 
//int SaveClassInfo();
void Adm_Test(char *cmd);
void CTFResetFlags(void);
//WF34 E

//Show all the players
void sv_ShowPlayers()
{
//	edict_t		*cl_ent;
//	gclient_t	*cl;
	edict_t	*e;

	int i;

	//Show red players first
	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		//getClassName(e->client, classname);
		if (e->client->resp.ctf_team == CTF_TEAM1)
		{
			gi.dprintf ("Red  %3d %8s %s\n",
				e->client->resp.score,
				getClassName(e->client),
				e->client->pers.netname);
		}
	}

	//Next blue
	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		//getClassName(e->client, classname);
		if (e->client->resp.ctf_team == CTF_TEAM2)
		{
			gi.dprintf ("Blue %3d %8s %s\n",
				e->client->resp.score,
				getClassName(e->client),
				e->client->pers.netname);
		}
	}

	//Then observers
	for (i=1, e=g_edicts+i; i < globals.num_edicts; i++,e++)
	{
		if (!e->inuse)
			continue;
		if (!e->client)
			continue;
		//getClassName(e->client, classname);
		if (e->client->resp.ctf_team <= 0)
		{
			gi.dprintf ("Obsv %3d %8s %s\n",
				e->client->resp.score,
				getClassName(e->client),
				e->client->pers.netname);
		}
	}
}

//ERASER START
void	Svcmd_Test_f (void)
{
	safe_cprintf (NULL, PRINT_HIGH, "Svcmd_Test_f()\n");
}

void	Svcmd_Bots_f (void)
{
	int i=2, j, len;
	char	name[128];

	while (i < gi.argc())
	{
		strcpy(name, gi.argv(i));
		len = strlen(name);

		// convert '~' to ' '
		for (j=0; j<len; j++)
			if (name[j] == '~')
				name[j] = ' ';

		spawn_bot(name);

		i++;
	}
}

void	Svcmd_Teams_f (void)
{
	int arg=2, i;
	char	team[128];

	while (arg < gi.argc())
	{
		strcpy(team, gi.argv(arg));

		i=0;
		while (i<MAX_TEAMS)
		{
			if (!bot_teams[i])
				break;

#ifdef _WIN32
			if (!_stricmp(bot_teams[i]->teamname, team) || !_stricmp(bot_teams[i]->abbrev, team))
#else
			if (!strcasecmp(bot_teams[i]->teamname, team) || !strcasecmp(bot_teams[i]->abbrev, team))
#endif
			{	// found the team, so add the bots
				bot_teams[i]->ingame = true;	// bots will be added automatically (below)
				break;
			}

			i++;
		}

		arg++;
	}
}

int	force_team = CTF_NOTEAM;

void	Svcmd_Blueteam_f (void)
{
	int i=2;

	force_team = CTF_TEAM2;
	while (i < gi.argc())
	{
//gi.dprintf("Spawning: \"%s\"\n", gi.argv(i));
		spawn_bot(gi.argv(i));

		i++;
	}

	force_team = CTF_NOTEAM;
}

void	Svcmd_Redteam_f (void)
{
	int i=2;

	force_team = CTF_TEAM1;
	while (i < gi.argc())
	{
		spawn_bot(gi.argv(i));

		i++;
	}

	force_team = CTF_NOTEAM;
}
//ERASER END
/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
void SV_WFFlags_f ();//WF24

void	ServerCommand (void)
{
	char	*cmd;
	char	*arg1;

	cmd = gi.argv(1);
	arg1 = gi.argv(2);//WF34
//WF24
	if (Q_stricmp (cmd, "showplayers") == 0)
	{
		sv_ShowPlayers ();
	}

    else if (Q_stricmp (cmd, "maplist") == 0) 
    { 
        Cmd_Maplist_f (NULL); 
	}
	
    else if (Q_stricmp (cmd, "classdef") == 0)//WF34 
    { 
        Cmd_Classdef_f (NULL); 
	}

    else if (Q_stricmp (cmd, "wfflags") == 0) 
    { 
        SV_WFFlags_f (); 
	}  

    else if (Q_stricmp (cmd, "resetflags") == 0)//WF34 
    { 
        CTFResetFlags(); 
	}  

    else if (Q_stricmp (cmd, "addip") == 0) //WF34
    { 
		Adm_Ban(arg1);
		//	Adm_KickBan(edict_t *ent);
	}  

    else if (Q_stricmp (cmd, "testip") == 0)//WF34 
    { 
		Adm_Test(arg1);
		//	Adm_KickBan(edict_t *ent);
	}  
    else if (Q_stricmp (cmd, "listip") == 0)//WF34 
    { 
		Adm_Bans(arg1);
	}  

    else if (Q_stricmp (cmd, "writeip") == 0)//WF34 
    { 
		WriteBans();
	}  

    else if (Q_stricmp (cmd, "removeip") == 0)//WF34 
    { 
		Adm_Unban(arg1);
	}  

    else if (Q_stricmp (cmd, "debug") == 0)//WF34 
    { 
		wfdebug = 1;
		gi.dprintf("WF Debug On\n");
	}  

    else if (Q_stricmp (cmd, "nodebug") == 0)//WF34 
    { 
                wfdebug = 0;
		gi.dprintf("WF Debug Off\n");
	}  
//ERASER
    else if (Q_stricmp (cmd, "test") == 0)
		Svcmd_Test_f ();
	else if (Q_stricmp (cmd, "bots") == 0)
		Svcmd_Bots_f ();
	else if (Q_stricmp (cmd, "teams") == 0)
		Svcmd_Teams_f ();
	else if (Q_stricmp (cmd, "bluebots") == 0)
		Svcmd_Blueteam_f ();
	else if (Q_stricmp (cmd, "redbots") == 0)
		Svcmd_Redteam_f ();
//ERASER END
	else
		gi.dprintf ("Unknown server command \"%s\"\n", cmd);
}

