#include "g_local.h"

static FILE *fpBot;

static void LogBotDetection(gclient_t *cl);

void BotDetection(edict_t *ent, usercmd_t *ucmd)
{
    static int i;
    static vec3_t change;
    static byte attack;
    static float rate;
    static gclient_t *cl;

	return;

    cl = ent->client;

    if (cl->resp.is_bot)
		return;

    attack = ucmd->buttons & BUTTON_ATTACK;
    if (attack ^ cl->pAttack) 
	{
		cl->pAttack = attack;

        // if the first criteria doesn't hold then skip calculations
        if (!attack && (cl->angle_rate < 400.0))
			goto noattack;

		if (!ucmd->msec) 
		{
			cl->angle_rate = 0.0;
            goto noattack;
		}

		for (i = 0; i < 3; i++) 
		{
			// angular change since last update
			change[i] = (ucmd->angles[i] - cl->pAngles[i]) * 180.0/32768.0;

            // compensate for -180 <-> 180 rollover
            if (change[i] > 180.0)
					change[i] -= 360.0;
			else if (change[i] < -180.0)
				change[i] += 360.0;
		}

        // calculate a combined (pitch and yaw) angular rate of change
        rate = sqrt(change[0]*change[0] + change[1]*change[1])*1000.0/ucmd->msec;

        if (attack) 
		{
            cl->angle_rate = rate;
            return;
        }

        if (rate == 0.0) 
		{ // a bot
            if (level.time > cl->bot_timer) 
			{
                cl->bot_timer = level.time + BOT_DETECTION_TIMER;
                cl->bot_count = 1;
            }
            else
				cl->bot_count++;

            // having a BOT_DETECTION_COUNT > 1 is just extra precaution
            if (cl->bot_count == BOT_DETECTION_COUNT) 
			{
                cl->resp.is_bot = 1;

                if ((int)(botdetect->value) & BOT_LOG)
					LogBotDetection(cl);

                if ((int)(botdetect->value) & BOT_KICK) 
				{
					gi.WriteByte(svc_disconnect);
					gi.unicast(ent, true);
					ClientDisconnect(ent);
                }

                if ((int)(botdetect->value) & BOT_NOTIFY) 
					gi.bprintf(PRINT_HIGH, "%s was caught using a BOT!\n", cl->pers.netname);
            }
        }
    }

noattack:
    // only remember angles if it's not an attack
    if (!attack) 
	{
		for (i = 0; i < 3; i++)
			cl->pAngles[i] = ucmd->angles[i];
    }   
}

void InitBotDetection(void) 
{
    char fname[FILENAME_MAX + 1];

	return;

    sprintf(fname, "bots/%s", BOT_DETECTION_FNAME);

    fpBot = fopen(fname, "a");
}

void EndBotDetection(void)
{
	return;

    if (fpBot)
		fclose(fpBot);
}

static void LogBotDetection(gclient_t *cl)
{
    struct tm *tm;
    time_t sec;

	return;

	if (!fpBot)
		return;

    time(&sec);
    tm = localtime(&sec);

    fprintf(fpBot,"%s@%s on %s", cl->pers.netname,
		Info_ValueForKey(cl->pers.userinfo, "ip"), asctime(tm));
}

