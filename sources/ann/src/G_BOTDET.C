#include "g_local.h"
#include "g_brrr.h"

static FILE *fpBot;

int botdetection;


static void LogBotDetection(gclient_t *cl, char *msg);


void
BotDetection(edict_t *ent, usercmd_t *ucmd)
{
	static int i;
	static vec3_t change;
	static byte attack;
	static float rate;
	static gclient_t *cl;

	cl = ent->client;

	if (cl->resp.is_bot)
		return;

	if (ucmd->impulse>=169 && ucmd->impulse<= 175) {
		if (botdetection & BOT_IMPULSE) {
			OnBotDetection(ent, va("impulse %d", ucmd->impulse));
			return;
		}
	}

	attack = ucmd->buttons & BUTTON_ATTACK;
	if (attack ^ cl->pAttack) {
		cl->pAttack = attack;

		// if the first criteria doesn't hold then skip calculations
		if (!attack && (cl->angle_rate < BOT_DETECTION_ATTACK_RATE))
			goto noattack;

		if (!ucmd->msec) {
			 cl->angle_rate = 0.0;
			 goto noattack;
		}

		for (i = 0; i < 2; i++) {
			// angular change since last update
			change[i] = ucmd->angles[i] - cl->pAngles[i];
			// compensate for -180 <-> 180 rollover
			if (change[i] > 32768.0)
				change[i] -= 65536.0;
			else if (change[i] < -32768.0)
				change[i] += 65536.0;
		}

		// calculate a combined (pitch and yaw) angular rate of change
		rate = (change[0]*change[0] + change[1]*change[1])/ucmd->msec;

		if (attack) {
			cl->angle_rate = rate;
			return;
		}

		if (rate <= BOT_DETECTION_RESTORE_RATE) { // a bot
			if (level.time > cl->bot_timer) {
				cl->bot_timer = level.time + BOT_DETECTION_TIMER;
				cl->bot_count = 1;
			}
			else
				cl->bot_count++;

			// having a BOT_DETECTION_COUNT > 1 is just extra precaution
			if (cl->bot_count == BOT_DETECTION_COUNT) {
				OnBotDetection(ent, NULL);
				return;
			}
		}
	}

noattack:
	// only remember angles if it's not an attack
	if (!attack) {
		for (i = 0; i < 2; i++)
			cl->pAngles[i] = ucmd->angles[i];
	}
}

void
OnBotDetection(edict_t *ent, char *msg)
{
	ent->client->resp.is_bot = 1;

	if (botdetection & BOT_LOG)
		LogBotDetection(ent->client, msg);

	if (botdetection & BOT_NOTIFY) {
		gi.bprintf(PRINT_HIGH, "%s was caught using a BOT!\n",
			ent->client->pers.netname);
	}

	if (botdetection & BOT_KICK) {
//		ent->movetype = MOVETYPE_NOCLIP;
//		gi.WriteByte(svc_disconnect);
//		gi.unicast(ent, true);
//		ClientDisconnect(ent);
		x_stuffcmd(ent,"clear;echo You were caught using a BOT!\n");
		x_stuffcmd(ent,"echo You were kicked from the game;echo \"\";disconnect\n");
	}
}

void
InitBotDetection(void) {
	cvar_t	*temp1;
	char fname[FILENAME_MAX + 1];

	temp1=gi.cvar("gamedir","",0);
	if (strlen(temp1->string)>0)
		sprintf(fname,"./%s/%s",temp1->string,BOT_DETECTION_FNAME);
	else
		sprintf(fname,"./%s",BOT_DETECTION_FNAME);

//	sprintf(fname, "%s/%s/%s",
//		basedir->string, gamedir->string, BOT_DETECTION_FNAME);

	fpBot = fopen(fname, "a");
}

void
EndBotDetection(void)
{
	if (fpBot)
		fclose(fpBot);
}

static void
LogBotDetection(gclient_t *cl, char *msg)
{
	struct tm *tm;
	time_t sec;

	if (!fpBot)
		return;

	time(&sec);
	tm = localtime(&sec);

	if (msg && msg[0]) {
		fprintf(fpBot,"%s@%s on %.24s (%s)\n",
			cl->pers.netname,
			Info_ValueForKey(cl->pers.userinfo, "ip"),
			asctime(tm),
			msg);
	}
	else {
		fprintf(fpBot,"%s@%s on %s",
			cl->pers.netname,
			Info_ValueForKey(cl->pers.userinfo, "ip"),
			asctime(tm));
	}
}

