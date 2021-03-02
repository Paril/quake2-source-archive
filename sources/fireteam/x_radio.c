#include "g_local.h"
#include "q_devels.h"
#include "x_radio.h"

void stuffcmd(edict_t *ent, char *s);

void X_Radio_Power_f(edict_t *self, char *state)
{
	if (!deathmatch->value)
		return;

	if (Q_strcasecmp (state, "1") == 0)
    {
	    self->client->pers.radio_power = 1;
        gi.cprintf(self, PRINT_HIGH, "Radio on.\n");
    }
    else
    {
        self->client->pers.radio_power = 0;
        gi.cprintf(self, PRINT_HIGH, "Radio off.\n");
    }
}

void X_Radio_f(edict_t *self, char *channel, char *msg)
{
    edict_t *player;
    int i;
    char *cmd, *pos;

	if (!CVAR_DEATHMATCH)
		return;

    cmd = "\0";
    if(pos = strstr(msg,";"))
	    pos[0]=0;
    sprintf( cmd, "play radio/%s\n", msg);

    if (Q_strcasecmp (channel, "ALL") == 0)
    {
        for_each_player(player, i)
        {
	        if (player->client->pers.radio_power)
	            stuffcmd(player, cmd);
        }
    }
    else if (Q_strcasecmp
		(channel, "TEAM") == 0)
    {
        for_each_player(player, i)
        {
			if (ctf->value)
			{
		        if (player->client->resp.ctf_team == self->client->resp.ctf_team)
		            if (player->client->pers.radio_power)
		                stuffcmd(player, cmd);
			}
			else if (team_dm->value)
			{
		        if (player->client->resp.team == self->client->resp.team)
		            if (player->client->pers.radio_power)
		                stuffcmd(player, cmd);
			}
			else // not in a teamplay mode, so just say it
			{
	            if (player->client->pers.radio_power)
	                stuffcmd(player, cmd);
			}
        }
    }
}
