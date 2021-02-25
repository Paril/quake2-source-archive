#define BOT_DETECTION_FNAME		"bots/BotDetection.log"

#define BOT_DETECTION_TIMER		0.1
#define BOT_DETECTION_COUNT		2

// botdetect flags
#define BOT_LOG         1
#define BOT_KICK        2
#define BOT_NOTIFY      4

extern int botdetection;

void BotDetection(edict_t *ent, usercmd_t *ucmd);
void InitBotDetection(void);
void EndBotDetection(void);
