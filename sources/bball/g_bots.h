#define MAX_BOTS	64	//absolute limit

typedef enum{
	BC_CUT_NONE,
	BC_CUT_TO,
	BC_CUT_BACK
} botcut_t;

typedef enum{
	BD_ZONE,
	BD_MAN2MAN,
	BD_POINT	//will initially guard ball handler
} botdefense_t;

typedef enum{
	BT_NONE,
	BT_CLIENT,
	BT_BOT,
	BT_BALL
} bottarget_t;

typedef enum{
	BOT_MOVE_NORMAL,
	BOT_MOVE_GUARD,
	BOT_MOVE_DRIBBLE
} botmove_t;

typedef struct gbot_s gbot_t;

void InitBotAI(void);
void BotEndServerFrames (void);
void SVCmd_SpawnBot_f(void);
void SVCmd_KillBot_f(void);
void Cmd_BotList_f(edict_t *ent);
void BotEvenTeams(void);
