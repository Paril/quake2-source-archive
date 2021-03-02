void SP_trigger_teleport(edict_t *self);
void SP_info_teleport_destination(edict_t *self);

extern char *ctf_statusbar;

void ctf_frag_bonuses(edict_t *targ, edict_t *inflictor, edict_t *attacker);

void stuffcmd(edict_t *e, char *s);

void CTFScoreboardMessage (edict_t *ent, edict_t *killer);

void ctf_update_sbar(edict_t *ent);
void Cmd_CTFHelp_f (edict_t *ent);

void CTF_Disconnect(edict_t *ent);
qboolean CTF_Reconnect(edict_t *ent);
void CTF_EndLevel(void);
qboolean CTF_CheckRules(void);



#define CTF_FRAG_CARRIER_BONUS			2
#define CTF_TARGET_PROTECT_RADIUS		400	
#define CTF_FLAG_DEFENSE_BONUS			1
#define CTF_ATTACKER_PROTECT_RADIUS		400
#define CTF_CARRIER_PROTECT_BONUS		1

static void CTFSay_Team_Location(edict_t *who, char *buf);
static void CTFSay_Team_Armor(edict_t *who, char *buf);
static void CTFSay_Team_Health(edict_t *who, char *buf);
static void CTFSay_Team_Tech(edict_t *who, char *buf);
static void CTFSay_Team_Weapon(edict_t *who, char *buf);
static void CTFSay_Team_Sight(edict_t *who, char *buf);
void Cmd_CTFSay_f(edict_t *who);
