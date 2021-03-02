#include "g_local.h"

int empty_slot (void)
{
	int i;
    int kill = 14;
    float oldest = 9999999999.0;
    for (i = 0; i < 15; i++)
    {
	    if (level.time > saved_client[i].timeout)
	        return i;
        if (saved_client[i].timeout < oldest)
        {
            oldest = saved_client[i].timeout;
            kill = i;
        }
    }
    return kill;
}

int find_saved (char *ip)
{
    int i;
    for (i = 0; i < 15; i++)
	    if (level.time < saved_client[i].timeout)
        {
	        if (Q_strcasecmp(saved_client[i].ip, ip) == 0)
	            return i;
        }
    return -1;
}

void make_empty (int i)
{
    saved_client[i].score = 0;
    saved_client[i].class = 0;
    saved_client[i].team = 0;
    saved_client[i].ctf_team = 0;
    Com_sprintf(saved_client[i].ip, 30, " ");
    saved_client[i].timeout = 0;
}

int saveinfo (edict_t *ent)
{
    int     i;
    i = empty_slot();
	saved_client[i].score = ent->client->resp.score;
    saved_client[i].class = ent->client->resp.class;
    saved_client[i].team = ent->client->resp.team;
    saved_client[i].ctf_team = ent->client->resp.ctf_team;
    Com_sprintf(saved_client[i].ip, 30, Info_ValueForKey(ent->client->pers.userinfo, "ip"));
    saved_client[i].timeout = level.time + 300;
    return i;
}

void getinfo (edict_t *ent, int i)
{
	ent->client->resp.score = saved_client[i].score;
    ent->client->resp.class = saved_client[i].class;
    ent->client->resp.team = saved_client[i].team;
    ent->client->resp.ctf_team = saved_client[i].ctf_team;
}
