#include "g_local.h"
#include "x_radio.h"

void X_Radio_Power_f(edict_t *self, char *state)
{
	if (self->teamstate & STATE_VICTIM)
	{
		gi.cprintf (self, PRINT_HIGH, "Access to radio has been denided by team master\n");
		return;
	}

	if (strlen(state) == 0)
	{
		gi.cprintf (self, PRINT_HIGH, "radio_power is %i\n", self->client->pers.radio_power);
		return;
	}

	if (Q_stricmp (state, "0") != 0 || Q_stricmp (state, "off") != 0)
	{
      if (!self->client->pers.radio_power)
		gi.cprintf(self, PRINT_HIGH, "Radio ON\n");

	  self->client->pers.radio_power = 1;
	}
	else
	{
		if (self->client->pers.radio_power)
        	gi.cprintf(self, PRINT_HIGH, "Radio OFF\n");
		self->client->pers.radio_power = 0;
	}
}

void X_Radio_f(edict_t *self, char *channel, char *msg)
{
	entity *player;
	int i;
	char *cmd, *pos;
    char file[MAX_QPATH]; // Initialize cmd variable

	cmd = "\0";

	if(pos = strstr(msg,";"))			// Check if msg contains ';'
		pos[0] = 0;			//	 If so, terminate string there

	if (self->teamstate & STATE_VICTIM)
	{
		gi.cprintf (self, PRINT_HIGH, "Access to radio has been denided by team master\n");
		return;
	}

	if (strlen (msg) == 0)
	{
		gi.cprintf (self, PRINT_HIGH, "Name the file to play!\n");
		return;
	}

	sprintf (file, "radio/%s.wav", msg);

    if (Q_stricmp (channel, "TEAM") == 0)	// To Team Members
	{
		for_each_player(player, i)
		{
			if (player->deadflag == DEAD_DEAD)
				break;

			if (player->client->resp.s_team == self->client->resp.s_team || player->playerclass == 3)
			{
				if (player->client->pers.radio_power)
				{
					gi.sound (player, CHAN_VOICE, gi.soundindex(file), 1, ATTN_NONE, 0);

					if (player != self)
						gi.cprintf (player,PRINT_HIGH, "Incoming message from %s\n", self->client->pers.netname); 
				}
			}
		}
	}
}

void Radio_Player (entity *ent, entity *other, char *msg)
{
	if (strlen(msg) == 0)
	{
		gi.cprintf (ent, PRINT_HIGH, "Name the .wav file to play!\n");
		return;
	}

	if (ent->teamstate & STATE_VICTIM)
	{
		gi.cprintf (ent, PRINT_HIGH, "Access to radio has been denided by team master\n");
		return;
	}

	if (other != NULL)
	{
		if (ent->client->resp.s_team == other->client->resp.s_team)
		{
			if (other->client->pers.radio_power)
			{
			    char *s;

				if (other->deadflag == DEAD_DEAD)
				{
					gi.cprintf (ent, PRINT_HIGH, "%s is dead\n", other->client->pers.netname);
					return;
				}

				s = "\0";
			    sprintf (s, "radio/%s.wav", msg);
				gi.sound (other, CHAN_VOICE, gi.soundindex(s), 1, ATTN_NONE, 0);
			}
			else
				gi.cprintf (ent, PRINT_HIGH, "%s's radio is off\n", other->client->pers.netname);
		}
		else
			gi.cprintf (ent, PRINT_HIGH, "%s is playing in another team!\n", other->client->pers.netname);
	}
	else
		gi.cprintf (ent, PRINT_HIGH, "No such player : %s\n", gi.argv(1));
}

void TeamMasterCanOffPlayersRadio (entity *master, char *cmd)
{
	entity *victim = ent_by_name (cmd);

	if (!victim->client || !victim->inuse)
	{
		gi.cprintf (master, PRINT_HIGH, "Victim is not fully in the game\n");
		return;
	}

	if (master->team_master == false)
	{
		gi.cprintf (master, PRINT_HIGH, "You are not a team master!\n");
		return;
	}

	if (strlen(cmd) == 0)
	{
		gi.cprintf (master, PRINT_HIGH, "Name the player!\n");
		return;
	}

	if (victim != NULL)
	{
		if (victim->client->resp.s_team == master->client->resp.s_team)
		{
			victim->client->pers.radio_power = 0;
			victim->teamstate |= STATE_VICTIM;
			gi.cprintf (victim, PRINT_HIGH, "Your radio has been disabled by team master!\n");
		}
		else
			gi.cprintf (master, PRINT_HIGH, "%s is playing in another team!\n", victim->client->pers.netname);
	}
	else
		gi.cprintf (master, PRINT_HIGH, "No such player: %s\n", cmd);
}

void TeamMasterCanKickPlayer (entity *master, char *cmd)
{
	entity *victim = ent_by_name (cmd);

	if (!victim->client || !victim->inuse)
	{
		gi.cprintf (master, PRINT_HIGH, "Victim is not fully in the game\n");
		return;
	}

	if (master->team_master == false)
	{
		gi.cprintf (master, PRINT_HIGH, "You are not a team master!\n");
		return;
	}

	if (strlen(cmd) == 0)
	{
		gi.cprintf (master, PRINT_HIGH, "Name the player!\n");
		return;
	}

	if (victim == master)
	{
		gi.cprintf (master, PRINT_HIGH, "You can't kick yourself!\n");
		return;
	}

	if (victim != NULL)
	{
		if (victim->client->resp.s_team == master->client->resp.s_team)
		{
			gi.cprintf (victim, PRINT_HIGH, "You have been kicked by teammaster\n");
			gi.bprintf (PRINT_HIGH, "%s was kicked by team master\n", victim->client->pers.netname);
			stuffcmd  (victim, "disconnect\n");
		}
		else
			gi.cprintf (master, PRINT_HIGH, "%s is playing in another team!\n", victim->client->pers.netname);
	}
	else
		gi.cprintf (master, PRINT_HIGH, "No such player: %s\n", cmd);
}

void TeamMasterCanShutUpPlayer (entity *master, char *cmd)
{
	entity *victim = ent_by_name (cmd);

	if (!victim->client || !victim->inuse)
	{
		gi.cprintf (master, PRINT_HIGH, "Victim is not fully in the game\n");
		return;
	}

	if (master->team_master == false)
	{
		gi.cprintf (master, PRINT_HIGH, "You are not a team master!\n");
		return;
	}

	if (strlen(cmd) == 0)
	{
		gi.cprintf (master, PRINT_HIGH, "Name the player!\n");
		return;
	}

	if (victim != NULL)
	{
		if (victim->client->resp.s_team == master->client->resp.s_team)
		{
			gi.cprintf (victim, PRINT_HIGH, "You can't talk now!\n");
            victim->teamstate |= STATE_NOTSAY;
		}
		else
			gi.cprintf (master, PRINT_HIGH, "%s is playing in another team!\n", victim->client->pers.netname);
	}
	else
		gi.cprintf (master, PRINT_HIGH, "No such player: %s\n", cmd);
}

