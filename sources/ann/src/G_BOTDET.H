#define BOT_DETECTION_FNAME	"botdetect.log"

#define BOT_DETECTION_TIMER           0.1
#define BOT_DETECTION_COUNT           1
#define BOT_DETECTION_ATTACK_RATE     40000.0
#define BOT_DETECTION_RESTORE_RATE    0.0

// sv_botdetection flags
#define BOT_LOG       1
#define BOT_KICK      2
#define BOT_NOTIFY    4
#define BOT_IMPULSE   8

extern int botdetection;

void BotDetection(edict_t *ent, usercmd_t *ucmd);
void OnBotDetection(edict_t *ent, char *msg);
void InitBotDetection(void);
void EndBotDetection(void);
